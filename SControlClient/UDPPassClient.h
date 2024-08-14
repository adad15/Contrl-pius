#pragma once
#include "framework.h"
#include "Common.h"
#include "MThread.h"
#include <string>
#include <vector>
#include <atomic>
#include <map>

/*
 * UDPPassClient类：
 *	1.私有成员变量：
 *		m_currentUser：储存当前用户的套接字、ID、地址信息等
 *		m_mapAddrs：map容器，存储在线其他用户的信息
 *		UDP、TCP：套接字和地址信息
 *		m_thpool：线程池，用于管理多线程任务
 *		m_stop：原子布尔值，用于控制线程停止
 *		m_hWnd：窗口句柄，用于与 GUI 进行交互
 *		m_udpConectPack：UDP连接包
 * 
 *	2.ThreadTcpProc函数： 
 * 
 * 
 */

class UDPPassClient : public CMFuncBase
{
private:
	MUserInfo						m_currentUser;	//存储当前用户的信息
	std::map<long long, MUserInfo>	m_mapAddrs;		//存储在线其他用户的信息，使用 std::map 进行管理
	// 分别存储UDP和TCP的地址信息
	sockaddr_in						m_udpAddr;
	sockaddr_in						m_tcpAddr;
	// 分别是UDP和TCP的套接字
	SOCKET							m_udpSock;
	SOCKET							m_tcpSock;
	CMThreadPool					m_thpool;       // 线程池，用于管理多线程任务
	std::atomic<bool>				m_stop;			// 原子布尔值，用于控制线程停止
	HWND							m_hWnd;			// 窗口句柄，用于与 GUI 进行交互
	CPacket							m_udpConectPack;// UDP连接包
private:
	// 别处理TCP、UDP 和UDP穿透的线程函数
	int ThreadTcpProc();
	int ThreadUdpProc();
	int ThreadUDPPass();
	
	// 初始化网络环境，加载网络库
	void InitSockEnv()
	{
		//初始化网络库
		WSADATA wsaData;
		int ret = WSAStartup(MAKEWORD(1, 1), &wsaData);
		if (ret != 0)
		{
			TRACE("初始化网络库失败(错误码:%d 错误:%s)\r\n", ret, GetErrInfo(ret));
			AfxMessageBox(L"初始化网络库失败");
			exit(-1);
		}
	}
	
	// 释放网络环境，卸载网络库
	void DesSockEnv()
	{
		WSACleanup();
	}
	// 保持在线状态的函数
	int KeepOnline();
	
public:
	// 构造函数：初始化 UDP 客户端
	UDPPassClient(const std::string& ip, short tcpPort, short udpPort);
	// 析构函数：销毁 UDP 客户端
	~UDPPassClient();
	// 启动客户端的主要函数
	int Invoke(HWND hWnd);
	// 处理接收到的 UDP 数据包
	void DealUdp(CPacket& pack, sockaddr_in& addr);
	// 处理接收到的 TCP 数据包
	void DealTcp(CPacket& pack);
	// 获取当前在线的用户信息
	std::map<long long, MUserInfo>& GetMapAddrs();
	// 请求与指定 ID 的用户建立连接
	void RequestConnect(long long id);
	// 发送控制请求
	void SentToBeCtrl();
};