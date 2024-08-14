#pragma once
#include "Common.h"
#include <Windows.h>
#include <direct.h>
#include <vector>

/*
 * 
 * CClientSocket�ࣺ
 *		1.���캯������ʼ���ͻ����׽��֣���ַ����ַ���ȣ���ʶ��
 *					���û��������ȣ�����ջ�����
 *					����InitSockEnv��������ʼ�������
 *		
 *		2.����������ɾ�������
 * 
 *		3.SetBufferSize����:�û����û������Ĵ�С
 * 
 *		4.InitSockEnv��������ʼ�������
 * 
 *		5.DesSockEnv������ɾ�������
 * 		
 *							6.1 ʹ��socket�����ͻ���TCP�ļ�������
 *		6.InitSocket������  6.2 ʹ��serv_addr���õ�ַ�Ͷ˿�
 *							6.3 ʹ��connect�������ӷ�����
 *					
 *		7.CloseSocket�������ر��׽���
 * 
 *		8.DealCommand�������ú������׽��ֽ������ݣ��������յ������ݴ���Ϊһ��CPacket����
 *			8.1 ��ʼ��������ָ��
 *								
 *                             	��������
 *          8.2 ѭ����������	����index���������е�����λ��
 *								����CPacket�������յ������ݰ�
 *								����������δ���������ݷ���ǰ��
 * 
 *		9.Send����������ͨ��CPacket��Data�������Ϻõ�������
 * 
 *		10.GetPacket����������CPacket���������
 * 
 *
*/

/*��װ���׽��֣������ڿͻ���ͨ���׽��֣�socket�������������ͨ��*/
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
	//��ʼ�������
	void InitSockEnv()
	{
		WSADATA wsaData;
		int ret = WSAStartup(MAKEWORD(1, 1), &wsaData);
		if (ret != 0)
		{
			TRACE("��ʼ�������ʧ��(������:%d ����:%s)\r\n", ret, GetErrInfo(ret));
			AfxMessageBox(L"��ʼ�������ʧ��");
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
		//�����׽���
		clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (clnt_sock == INVALID_SOCKET)
		{
			TRACE("�����׽���ʧ��(������:%d ����:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"�����׽���ʧ��");
			return;
		}
		char strPort[256]{};
		/*�� CString ���͵� IP ��ַ�ӿ��ַ�ת��Ϊ���ֽ��ַ�*/
		if (WideCharToMultiByte(CP_ACP, 0, ip, -1, strPort, 256, NULL, NULL) == 0)
		{
			TRACE("���ֽ�ת���ֽ�ʧ��(������:%d ����:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"���ֽ�ת���ֽ�ʧ��");
			return;
		}
		//���õ�ַ���˿�
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(strPort);
		serv_addr.sin_port = htons(_wtoi(port));
		//���ӱ��ض�
		if (connect(clnt_sock, (SOCKADDR*)&serv_addr, serv_addr_len) == SOCKET_ERROR)
		{
			closesocket(clnt_sock);
			TRACE("���ӷ�����ʧ��(������:%d ����:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"���ӷ�����ʧ��");
			return;
		}
	}

	void CloseSocket()
	{
		isReadOver = false;
		closesocket(clnt_sock);
	}
	/*���ڴ�����׽��ֽ��յ����ݣ������յ������ݴ���Ϊһ�� CPacket ���󣬷��ظ����ݰ�����������*/
	int DealCommand()
	{
		/*��ʼ��������ָ��*/
		char* buf = buffer.data();
		if (clnt_sock == INVALID_SOCKET)
		{
			TRACE("��������ʱ��δ���� (%d)\r\n", clnt_sock);
			return -1;
		}
		/*ѭ����������*/
		while (true)
		{
			int readLen = 0;	

			TRACE("1------------------tick = %lld\r\n", GetTickCount64());
			readLen = recv(clnt_sock, buf + index/*�������ݵĻ�����λ��*/, BUFFER_SIZE - index/*ʣ�໺�����Ĵ�С*/, 0);
			TRACE("2------------------tick = %lld\r\n", GetTickCount64());

			TRACE("[threadId: %d] -------------------------------------------DealCommand----------------------------------------\r\n",GetThreadId(GetCurrentThread()));
			TRACE("[threadId: %d] readLen = %d\r\n", GetThreadId(GetCurrentThread()), readLen);

			if (readLen <= 0 && index <= 0)
			{
				TRACE("[threadId: %d] ��ȡ�������������ˣ����һ����������� \r\n", GetThreadId(GetCurrentThread()));
				return -2;
			}
			TRACE("[threadId: %d] index 1 = %d\r\n", GetThreadId(GetCurrentThread()) , index);
			/*���� index ��ָʾ�������е�����λ��*/
			index += readLen;
			TRACE("[threadId: %d] index 2 = %d\r\n", GetThreadId(GetCurrentThread()), index);
			/*�����������ݵĳ���*/
			int len = index;
			TRACE("[threadId: %d] len 1 = %d\r\n", GetThreadId(GetCurrentThread()), len);
			/*�����յ������ݽ���Ϊ���ݰ���lenΪ����ʹ�������*/
			pack = CPacket((BYTE*)buf, len);
			/*��ʱlenΪ����������len��ʾ�������ĳ���*/
			TRACE("[threadId: %d] len 2 = %d\r\n", GetThreadId(GetCurrentThread()), len);
			TRACE("[threadId: %d] pack => %s \r\n", GetThreadId(GetCurrentThread()), pack.ToString2().c_str());
			/*��ȡ��ǰ�̵߳��̱߳�ʶ��*/
			DWORD thid = GetThreadId(GetCurrentThread());
			/*��δ���������ݷ��ڻ�������ǰ��*/
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
		TRACE("[threadId: %d] ��ȡ�������� \r\n", GetThreadId(GetCurrentThread()));
		return 0;
	}
	/*ͨ��clnt_sock�������ݣ�(char*)_pack.Data()���͵����ݵ�ָ��*/
	int Send(CPacket& _pack)
	{	
		/*����CPacket�е�Data�����������õ��������ϳ�һ��char���͵�����*/
		return send(clnt_sock, (char*)_pack.Data(), _pack.Size(), 0);
	}
	/*���ض�CPacket���������*/
	CPacket& GetPacket()
	{
		return pack;
	}
};