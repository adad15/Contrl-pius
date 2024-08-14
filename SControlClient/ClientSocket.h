#pragma once
#include "Common.h"
#include <Windows.h>
#include <direct.h>
#include <vector>

/*
 * 
 * CClientSocket类：
 *		1.构造函数：初始化客户端套接字，地址，地址长度，标识符
 *					设置缓冲区长度，并清空缓冲区
 *					调用InitSockEnv函数，初始化网络库
 *		
 *		2.析构函数：删除网络库
 * 
 *		3.SetBufferSize函数:用户设置缓冲区的大小
 * 
 *		4.InitSockEnv函数：初始化网络库
 * 
 *		5.DesSockEnv函数：删除网络库
 * 		
 *							6.1 使用socket创建客户端TCP文件描述符
 *		6.InitSocket函数：  6.2 使用serv_addr配置地址和端口
 *							6.3 使用connect函数连接服务器
 *					
 *		7.CloseSocket函数：关闭套接字
 * 
 *		8.DealCommand函数：该函数从套接字接收数据，并将接收到的数据处理为一个CPacket对象
 *			8.1 初始化缓冲区指针
 *								
 *                             	接收数据
 *          8.2 循环接收数据	更新index到缓冲区中的数据位置
 *								调用CPacket解析接收到的数据包
 *								将缓冲区中未解析的数据放在前面
 * 
 *		9.Send函数：发送通过CPacket中Data函数整合好的数据流
 * 
 *		10.GetPacket函数：返回CPacket对象的引用
 * 
 *
*/

/*封装了套接字，用于在客户端通过套接字（socket）与服务器进行通信*/
class CClientSocket {
public:
	CClientSocket() : clnt_sock(INVALID_SOCKET), serv_addr(), serv_addr_len(sizeof(SOCKADDR_IN)), isReadOver(false) //, index(0)
	{
		BUFFER_SIZE = 1024 * 1024 * 2;
		InitSockEnv();
		buffer.resize(BUFFER_SIZE);
		memset(buffer.data(), 0, BUFFER_SIZE);
	}
	~CClientSocket()
	{
		DesSockEnv();
	}

	void SetBufferSize(DWORD size)
	{
		BUFFER_SIZE = size;
		buffer.resize(BUFFER_SIZE);
		memset(buffer.data(), 0, BUFFER_SIZE);
	}
private:
	SOCKET				clnt_sock;
	SOCKADDR_IN			serv_addr;
	int					serv_addr_len;
	std::vector<char>	buffer;
	CPacket				pack;
	bool				isReadOver;
	DWORD				BUFFER_SIZE;
	int					index;
private:
	//初始化网络库
	void InitSockEnv()
	{
		WSADATA wsaData;
		int ret = WSAStartup(MAKEWORD(1, 1), &wsaData);
		if (ret != 0)
		{
			TRACE("初始化网络库失败(错误码:%d 错误:%s)\r\n", ret, GetErrInfo(ret));
			AfxMessageBox(L"初始化网络库失败");
			exit(-1);
		}
	}
	void DesSockEnv()
	{
		WSACleanup();
	}
public:
	void InitSocket(const CString& ip, const CString& port)
	{
		//创建套接字
		clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (clnt_sock == INVALID_SOCKET)
		{
			TRACE("创建套接字失败(错误码:%d 错误:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"创建套接字失败");
			return;
		}
		char strPort[256]{};
		/*将 CString 类型的 IP 地址从宽字符转换为多字节字符*/
		if (WideCharToMultiByte(CP_ACP, 0, ip, -1, strPort, 256, NULL, NULL) == 0)
		{
			TRACE("宽字节转多字节失败(错误码:%d 错误:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"宽字节转多字节失败");
			return;
		}
		//配置地址，端口
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(strPort);
		serv_addr.sin_port = htons(_wtoi(port));
		//连接被控端
		if (connect(clnt_sock, (SOCKADDR*)&serv_addr, serv_addr_len) == SOCKET_ERROR)
		{
			closesocket(clnt_sock);
			TRACE("连接服务器失败(错误码:%d 错误:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"连接服务器失败");
			return;
		}
	}

	void CloseSocket()
	{
		isReadOver = false;
		closesocket(clnt_sock);
	}
	/*用于处理从套接字接收的数据，将接收到的数据处理为一个 CPacket 对象，返回该数据包的命令类型*/
	int DealCommand()
	{
		/*初始化缓冲区指针*/
		char* buf = buffer.data();
		if (clnt_sock == INVALID_SOCKET)
		{
			TRACE("处理命令时，未连接 (%d)\r\n", clnt_sock);
			return -1;
		}
		/*循环接收数据*/
		while (true)
		{
			int readLen = 0;	

			TRACE("1------------------tick = %lld\r\n", GetTickCount64());
			readLen = recv(clnt_sock, buf + index/*接收数据的缓冲区位置*/, BUFFER_SIZE - index/*剩余缓冲区的大小*/, 0);
			TRACE("2------------------tick = %lld\r\n", GetTickCount64());

			TRACE("[threadId: %d] -------------------------------------------DealCommand----------------------------------------\r\n",GetThreadId(GetCurrentThread()));
			TRACE("[threadId: %d] readLen = %d\r\n", GetThreadId(GetCurrentThread()), readLen);

			if (readLen <= 0 && index <= 0)
			{
				TRACE("[threadId: %d] 读取错误，数据收完了，并且缓冲区读完了 \r\n", GetThreadId(GetCurrentThread()));
				return -2;
			}
			TRACE("[threadId: %d] index 1 = %d\r\n", GetThreadId(GetCurrentThread()) , index);
			/*更新 index 以指示缓冲区中的数据位置*/
			index += readLen;
			TRACE("[threadId: %d] index 2 = %d\r\n", GetThreadId(GetCurrentThread()), index);
			/*缓冲区中数据的长度*/
			int len = index;
			TRACE("[threadId: %d] len 1 = %d\r\n", GetThreadId(GetCurrentThread()), len);
			/*将接收到的数据解析为数据包，len为传入和传出参数*/
			pack = CPacket((BYTE*)buf, len);
			/*此时len为传出参数，len表示解析出的长度*/
			TRACE("[threadId: %d] len 2 = %d\r\n", GetThreadId(GetCurrentThread()), len);
			TRACE("[threadId: %d] pack => %s \r\n", GetThreadId(GetCurrentThread()), pack.ToString2().c_str());
			/*获取当前线程的线程标识符*/
			DWORD thid = GetThreadId(GetCurrentThread());
			/*将未解析的数据放在缓冲区的前面*/
			if (len > 0)
			{
				TRACE("[threadId: %d] index 3 = %d\r\n", GetThreadId(GetCurrentThread()), index);
				memmove(buf, buf + len, index - len);
				TRACE("[threadId: %d] index 4 = %d\r\n", GetThreadId(GetCurrentThread()), index);
				index -= len;
				TRACE("[threadId: %d] index 5 = %d\r\n", GetThreadId(GetCurrentThread()), index);
				return pack.nCmd;
			}
		}
		TRACE("[threadId: %d] 读取产生意外 \r\n", GetThreadId(GetCurrentThread()));
		return 0;
	}
	/*通过clnt_sock发送数据，(char*)_pack.Data()发送的数据的指针*/
	int Send(CPacket& _pack)
	{	
		/*调用CPacket中的Data函数将解析好的数据整合成一个char类型的数据*/
		return send(clnt_sock, (char*)_pack.Data(), _pack.Size(), 0);
	}
	/*返回对CPacket对象的引用*/
	CPacket& GetPacket()
	{
		return pack;
	}
};