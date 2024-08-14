#pragma once
#include "framework.h"
#include "Common.h"
#include "MThread.h"
#include <string>
#include <vector>
#include <atomic>
#include <map>

/*
 * UDPPassClient�ࣺ
 *	1.˽�г�Ա������
 *		m_currentUser�����浱ǰ�û����׽��֡�ID����ַ��Ϣ��
 *		m_mapAddrs��map�������洢���������û�����Ϣ
 *		UDP��TCP���׽��ֺ͵�ַ��Ϣ
 *		m_thpool���̳߳أ����ڹ�����߳�����
 *		m_stop��ԭ�Ӳ���ֵ�����ڿ����߳�ֹͣ
 *		m_hWnd�����ھ���������� GUI ���н���
 *		m_udpConectPack��UDP���Ӱ�
 * 
 *	2.ThreadTcpProc������ 
 * 
 * 
 */

class UDPPassClient : public CMFuncBase
{
private:
	MUserInfo						m_currentUser;	//�洢��ǰ�û�����Ϣ
	std::map<long long, MUserInfo>	m_mapAddrs;		//�洢���������û�����Ϣ��ʹ�� std::map ���й���
	// �ֱ�洢UDP��TCP�ĵ�ַ��Ϣ
	sockaddr_in						m_udpAddr;
	sockaddr_in						m_tcpAddr;
	// �ֱ���UDP��TCP���׽���
	SOCKET							m_udpSock;
	SOCKET							m_tcpSock;
	CMThreadPool					m_thpool;       // �̳߳أ����ڹ�����߳�����
	std::atomic<bool>				m_stop;			// ԭ�Ӳ���ֵ�����ڿ����߳�ֹͣ
	HWND							m_hWnd;			// ���ھ���������� GUI ���н���
	CPacket							m_udpConectPack;// UDP���Ӱ�
private:
	// ����TCP��UDP ��UDP��͸���̺߳���
	int ThreadTcpProc();
	int ThreadUdpProc();
	int ThreadUDPPass();
	
	// ��ʼ�����绷�������������
	void InitSockEnv()
	{
		//��ʼ�������
		WSADATA wsaData;
		int ret = WSAStartup(MAKEWORD(1, 1), &wsaData);
		if (ret != 0)
		{
			TRACE("��ʼ�������ʧ��(������:%d ����:%s)\r\n", ret, GetErrInfo(ret));
			AfxMessageBox(L"��ʼ�������ʧ��");
			exit(-1);
		}
	}
	
	// �ͷ����绷����ж�������
	void DesSockEnv()
	{
		WSACleanup();
	}
	// ��������״̬�ĺ���
	int KeepOnline();
	
public:
	// ���캯������ʼ�� UDP �ͻ���
	UDPPassClient(const std::string& ip, short tcpPort, short udpPort);
	// �������������� UDP �ͻ���
	~UDPPassClient();
	// �����ͻ��˵���Ҫ����
	int Invoke(HWND hWnd);
	// ������յ��� UDP ���ݰ�
	void DealUdp(CPacket& pack, sockaddr_in& addr);
	// ������յ��� TCP ���ݰ�
	void DealTcp(CPacket& pack);
	// ��ȡ��ǰ���ߵ��û���Ϣ
	std::map<long long, MUserInfo>& GetMapAddrs();
	// ������ָ�� ID ���û���������
	void RequestConnect(long long id);
	// ���Ϳ�������
	void SentToBeCtrl();
};