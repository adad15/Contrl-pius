#pragma once
#pragma warning(disable:4267)//禁用特定的编译器警告
#include "ClientSocket.h"
#include "ClientController.h"
#include "MQueue.h"
#include "MThread.h"
#include <mutex>
#include <atomic>
/*自定义的消息*/
#define WM_RE_SEND_PACK		(WM_USER + 101)
#define WM_RE_SEND_PACK_ACK (WM_USER + 102)

/*
 * CRequest类：过使用线程、队列和事件来实现异步数据包发送，确保在多线程环境下数据包能够被安全有效地发送
 * 
 *	1.私有成员：一个线程安全的队列  一个线程对象  事件对象  互斥锁  原子变量
 * 
 *	2.线程主函数ThreadMain：
 *		2.1 循环：不断从线程安全的列表中拿取数据
 *		2.2 将数据打包，然后调用_SendPacket函数将数据包发送出去
 * 
 *  3._SendPacket函数：发送数据包函数
 *		3.1 创建套接字，设置网络地址和ip
 *		3.2 设置缓冲区大小
 *		3.3 发送数据包
 *		3.4 解析接收到的数据包，并返回命令代号
 *		3.5 关闭套接字
 * 
 *	4.构造函数：设置实际工作函数
 * 
 *	5.析构函数：将m_stop设置为false,实际工作函数退出循环
 * 
 *	6.SendPacket函数：参数 命令代号 数据 数据长度，数据包发送给队列
 *		6.1 上锁
 *		6.2 当队列中的数据包超过10个时，弹出最前面的数据包
 *		6.3 将参数打包成数据包，加入到队列中
 *		6.4 解锁
 *		6.5 将事件对象设置为有信号状态
 *		
 *	7.SendPacket函数：参数 打包好的数据
 *		7.1 将数据包加入到队列中
 *		7.2 将事件对象设置为有信号状态
 * 
 */

/*过使用线程、队列和事件来实现异步数据包发送，确保在多线程环境下数据包能够被安全有效地发送*/
class CRequest : public CMFuncBase
{
private:
	CMQueue<CPacket>	m_quePackets;
	CMThread			m_thread;
	HANDLE				m_event;
	std::mutex			m_mutex;
	std::atomic<bool>	m_stop;
public:
	/*主线程函数用来不断从列表中发送数据包*/
	int ThreadMain()
	{
		while (m_stop)
		{
			//如果链表空，那么就需等待
			if (m_quePackets.size() <= 0)
			{
				WaitForSingleObject(m_event, INFINITE);
			}
			ResetEvent(m_event);			//将事件设置为无信号状态
			CPacket pack;					//设置数据包对象
			m_quePackets.pop_front(pack);   //从列表中拿数据，赋值给数据包对象
			_SendPacket(pack);              //调用封装好的_SendPacket函数，将数据包发送出去
		}
		return -1;
	}
	/*发送数据包函数*/
	void _SendPacket(CPacket& pack)
	{
		
		CClientSocket clientSock;
		/*创建套接字，设置网络地址和ip*/
		clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
		/*设置缓冲区大小*/
		clientSock.SetBufferSize(1024 * 1024 * 2);
		/*发送数据包*/
		clientSock.Send(pack);
		/*解析接收到的数据包，并返回命令代号*/
		clientSock.DealCommand();
		/*关闭套接字*/
		clientSock.CloseSocket();
	}
	/*设置实际工作函数*/
	CRequest()
	{
		/*将实际的工作函数设置给线程对象的work函数*/
		m_thread.Work(CMWork(this,(MT_FUNC)&CRequest::ThreadMain));
		/*启动线程*/
		m_thread.Start();
		/*设置事件对象为false*/
		m_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_stop = true;
	}

	~CRequest()
	{
		m_stop = false; //退出循环
	}
	/*数据包发送给队列*/
	void SendPacket(WORD nCmd, BYTE* data, DWORD len)
	{
		
		TRACE(" push back ->  %d  size  %d\r\n", GetTickCount64(), m_quePackets.size());
		m_mutex.lock();
		/*当队列中的数据包超过10个时，弹出最前面的数据包*/
		if (m_quePackets.size() > 10)
		{
			CPacket pack;
			m_quePackets.pop_front(pack);
		}
		m_quePackets.push_back(CPacket(nCmd,data,len));

		int a = m_quePackets.size();
		TRACE("SendPacket ->  %d  size  %d\r\n", GetTickCount64(), a);
		static int count = 0;
		TRACE("SendPacket ->  %d  count  %d\r\n", GetTickCount64(), ++count);
		m_mutex.unlock();
		SetEvent(m_event);
	}
	/*数据包发送给队列*/
	void SendPacket(CPacket& packet)
	{
		TRACE(" push back ->  %d  size  %d\r\n", GetTickCount64(), m_quePackets.size());
		m_quePackets.push_back(packet);
		SetEvent(m_event);
	}
};