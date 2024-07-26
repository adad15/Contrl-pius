#pragma once

#include <Windows.h>
#include <list>

/* ？？？队列的安全问题？？？ */

/*
 * CMQueue模板类:使用Windows I/O完成端口（IOCP）机制实现的模板类CMQueue，它实现了一个线程安全的队列
 *	1.私有成员变量：储存队列数据的list链表   完成端口句柄  线程句柄
 * 
 *	2.枚举变量：
 *		入队、出队、获取队列大小、清空队列
 * 
 *	3.M结构体：使用模板参数T使其能够处理不同类型的数据，并通过事件对象进行同步操作
 *		3.1 hEvent：事件对象
 *		3.2 T：这个结构体可以用于任何类型 T，用来存放列表中的数据。
 *		3.3 构造函数：调用CreateEvent创建一个 Windows 事件对象，并将其句柄赋值给 hEvent。
 *		3.4 析构函数：删除事件句柄
 * 
 *	4.静态成员函数 =》void ThreadEntry(void* arg)：线程入口函数
 *		4.1 调用真正的主线程函数ThreadMain
 *		4.2 结束线程
 * 
 *	5.主线程函数ThreadMain：任务处理函数，用于处理完成端口接收的命令
 *		5.1 私有成员变量：GetQueuedCompletionStatus参数
 *		5.2 循环：调用GetQueuedCompletionStatus，端口接收到命令就进入while循环
 *			5.2.1 退出条件检查：transferred=-1 and completionKey不为-1 and lpOverlapped空 =》 将completionKey强制转换为M类型指针pM  设置事件为有信号状态
 *			5.2.2 处理不同的完成状态 -> transferred
 *					PUSH：将completionKey指向的对象加入到lstData列表的末尾
 *					POP：设置空的M类型的指针pM
 *						 取出lstData列表的第一个元素赋值给pM->t并移除该元素
 *						 设置事件对象
 *					SIZE：设置空的M类型的指针pM
 *						  将列表中的大小赋值给pM->wParam
 *						  设置事件对象
 *					CLEAR：设置空的M类型的指针pM
 *						   清空列表数据
 *						   设置事件对象
 * 
 *	6.构造函数：创建线程和I/O完成端口
 *		6.1 封装_beginthread创建线程
 *		6.2 封装CreateIoCompletionPort创建完成端口
 * 
 *  7.析构函数：清空队列、关闭线程以及处理可能的错误情况
 *		7.1 调用封装好的clear函数向完成端口发送清空命令
 *		7.2 创建M对象m
 *		7.3 通过PostQueuedCompletionStatus给完成端口发送命令，表示线程退出
 *		7.4 调用WaitForSingleObject，接收ThreadMain线程中设置的有消息的事件对象
 * 
 *	8.push_back函数：将一个元素添加到队列中，并通过I/O完成端口通知工作线程进行处理
 *		8.1 new一个T类型的指针，用来存放数据
 *		8.2 调用PostQueuedCompletionStatus给端口发送命令和要插入的数据（const T& t）
 * 
 *	9.pop_front函数：
 *		9.1 创建空的M类型的对象m，用作传出参数，用来接收数据
 *		9.2 调用PostQueuedCompletionStatus，向完成端口发送命令
 *		9.3 调用WaitForSingleObject，接收ThreadMain线程中设置的有消息的事件对象
 * 
 *  10.size函数、clear函数：原理和pop_front一致
 * 
*/

template<typename T>
class CMQueue
{
private:
	enum
	{
		PUSH = 1001,  //入队
		POP,		  //出队
		SIZE,         //获取大小
		CLEAR,        //清空队列
	};
	/*使用模板参数T使其能够处理不同类型的数据，并通过事件对象进行同步操作。*/
	struct M
	{
		HANDLE hEvent;
		T t;           //存放列表中的数据
		int wParam;    //存放列表大小和状态
		M(T _t){
			hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			t = _t;
			wParam = -1;
		}
		~M()
		{
			CloseHandle(hEvent);
		}
	};
private:
	std::list<T>	lstData;     //存储队列数据的标准库链表
	HANDLE			m_hIocp;     //I/O完成端口的句柄
	HANDLE			m_hThread;   //工作线程的句柄
private:
	static void ThreadEntry(void* arg)
	{
		CMQueue* thiz = (CMQueue*)arg;
		thiz->ThreadMain();
		_endthread();
	}
	/*用于处理 I/O 完成端口 (IOCP) 的线程主函数*/
	void ThreadMain()
	{
		DWORD transferred;          //返回完成的字节数
		ULONG_PTR completionKey;    //返回与此 I/O 关联的完成键，整型变量
		LPOVERLAPPED lpOverlapped;  //返回与此 I/O 关联的重叠结构
		/*GetQueuedCompletionStatus用于从IOCP获取完成状态，填充transferred、completionKey和lpOverlapped变量*/
		/*GetQueuedCompletionStatus是一个阻塞函数，它从完成端口m_hIocp中获取一个完成的I/O请求*/

		/*lpCompletionKey被称为完成键，传递的数据被称为单句柄数据，数据应该是与每个socket句柄对应
		  lpOverlapped被称为重叠结构体，传递的数据被称为单IO数据，数据应该与每次的操作WSARecv、WSASend等相对应*/
		while (GetQueuedCompletionStatus(m_hIocp/*完成端口句柄*/, &transferred, &completionKey, &lpOverlapped, INFINITE/*无限等待直到有一个I/O完成*/))
		{
			/*退出条件检查*/
			if ((transferred == -1) && (completionKey != -1) && (lpOverlapped == NULL))
			{
				/*将completionKey强制转换为M类型指针pM*/
				M* pM = (M*)completionKey;
				/*触发pM的事件hEvent,通知操作完成*/
				SetEvent(pM->hEvent);
				break;
			}
			/*处理不同的完成状态*/
			switch (transferred)
			{
			case PUSH:
			{
				/*将completionKey指向的对象加入到lstData列表的末尾*/
				lstData.push_back(*(T*)completionKey);
				delete (T*)completionKey;
				break;
			}
			case POP:
			{
				M* pM = (M*)completionKey;/*PostQueuedCompletionStatus传入的空的 M m(t)*/
				if (lstData.size() > 0)
				{
					/*取出lstData列表的第一个元素赋值给pM->t并移除该元素*/
					pM->t = lstData.front();
					lstData.pop_front();
				}
				else
				{
					pM->wParam = 0;
				}
				/*触发pM的事件 hEvent,通知操作完成*/
				SetEvent(pM->hEvent);
				break;
			}
			case SIZE:
			{
				M* pM = (M*)completionKey;
				pM->wParam = lstData.size();
				SetEvent(pM->hEvent);
				break;
			}
			case CLEAR:
			{
				M* pM = (M*)completionKey;
				while (lstData.size() > 0)
				{
					lstData.pop_front();
				}
				SetEvent(pM->hEvent);
				break;
			}
			}
		}
	}
public:
	/*创建线程和I/O完成端口*/
	CMQueue()
	{
		m_hThread = (HANDLE) _beginthread(ThreadEntry, 0, this);
		m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE/*创建一个新的I/O完成端口*/, NULL, 0, 1);
	}
	/*在CMQueue对象销毁时进行清理工作，具体包括清空队列、关闭线程以及处理可能的错误情况*/
	~CMQueue()
	{
		clear();   //清空内部的数据队列
		T t;       
		M m(t);    //创建M对象m，hEvent为false wParam为-1 t为空
		/*通知线程退出*/
		/*PostQueuedCompletionStatus函数向I/O完成端口m_hIocp投递一个特殊的完成状态，通知工作线程退出*/
		if (!PostQueuedCompletionStatus(m_hIocp, -1/*特殊的transferred值，用于指示线程退出*/, (ULONG_PTR)&m/*将m的地址作为完成键*/, NULL))
		{
			::AfxMessageBox(L"~CMQueue Error1");
			return;
		}
		/*等待线程退出，等待100毫秒*/
		if (WaitForSingleObject(m.hEvent, 100) != WAIT_OBJECT_0)
		{
			::AfxMessageBox(L"~CMQueue Error2");
			return;
		}
	}
	/*将一个元素添加到队列中，并通过I/O完成端口通知工作线程进行处理*/
	bool push_back(const T& t)
	{
		/*通过new操作符创建一个新的T类型对象pT，并使用传入的参数t初始化它*/
		T* pT = new T(t);
		/*向I/O完成端口m_hIocp投递一个完成状态，以通知工作线程有新的元素需要处理*/
		if (!PostQueuedCompletionStatus(m_hIocp, PUSH, (ULONG_PTR)pT, NULL))
		{
			delete pT;
			return false;
		}
		return true;
	}

	bool pop_front(T& t)
	{
		M m(t);//空的 包含：事件句柄、参数T（存放数据）、wParam参数
		if (!PostQueuedCompletionStatus(m_hIocp, POP, (ULONG_PTR)&m/*传出参数*/, NULL))
		{
			return false;
		}
		if (WaitForSingleObject(m.hEvent, 100) != WAIT_OBJECT_0)
		{
			return false;
		}
		if (m.wParam == 0)
		{
			return false;
		}
		t = m.t;
		return true;
	}

	size_t size()
	{
		T t;
		M m(t);
		if (!PostQueuedCompletionStatus(m_hIocp, SIZE, (ULONG_PTR)&m, NULL))
		{
			return 0;
		}
		if (WaitForSingleObject(m.hEvent, 100) != WAIT_OBJECT_0)
		{
			return 0;
		}
		if (m.wParam != -1)
		{
			return m.wParam;
		}
		return 0;
	}

	bool clear()
	{
		T t;
		M m(t);
		if (!PostQueuedCompletionStatus(m_hIocp, CLEAR, (ULONG_PTR)&m, NULL))
		{
			return false;
		}
		if (WaitForSingleObject(m.hEvent, 100) != WAIT_OBJECT_0)
		{
			return false;
		}
		return true;
	}
};