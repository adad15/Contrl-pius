#pragma once
#pragma warning(disable:4267)//�����ض��ı���������
#include "ClientSocket.h"
#include "ClientController.h"
#include "MQueue.h"
#include "MThread.h"
#include <mutex>
#include <atomic>
/*�Զ������Ϣ*/
#define WM_RE_SEND_PACK		(WM_USER + 101)
#define WM_RE_SEND_PACK_ACK (WM_USER + 102)

/*
 * CRequest�ࣺ��ʹ���̡߳����к��¼���ʵ���첽���ݰ����ͣ�ȷ���ڶ��̻߳��������ݰ��ܹ�����ȫ��Ч�ط���
 * 
 *	1.˽�г�Ա��һ���̰߳�ȫ�Ķ���  һ���̶߳���  �¼�����  ������  ԭ�ӱ���
 * 
 *	2.�߳�������ThreadMain��
 *		2.1 ѭ�������ϴ��̰߳�ȫ���б�����ȡ����
 *		2.2 �����ݴ����Ȼ�����_SendPacket���������ݰ����ͳ�ȥ
 * 
 *  3._SendPacket�������������ݰ�����
 *		3.1 �����׽��֣����������ַ��ip
 *		3.2 ���û�������С
 *		3.3 �������ݰ�
 *		3.4 �������յ������ݰ����������������
 *		3.5 �ر��׽���
 * 
 *	4.���캯��������ʵ�ʹ�������
 * 
 *	5.������������m_stop����Ϊfalse,ʵ�ʹ��������˳�ѭ��
 * 
 *	6.SendPacket���������� ������� ���� ���ݳ��ȣ����ݰ����͸�����
 *		6.1 ����
 *		6.2 �������е����ݰ�����10��ʱ��������ǰ������ݰ�
 *		6.3 ��������������ݰ������뵽������
 *		6.4 ����
 *		6.5 ���¼���������Ϊ���ź�״̬
 *		
 *	7.SendPacket���������� ����õ�����
 *		7.1 �����ݰ����뵽������
 *		7.2 ���¼���������Ϊ���ź�״̬
 * 
 */

/*��ʹ���̡߳����к��¼���ʵ���첽���ݰ����ͣ�ȷ���ڶ��̻߳��������ݰ��ܹ�����ȫ��Ч�ط���*/
class CRequest : public CMFuncBase
{
private:
	CMQueue<CPacket>	m_quePackets;
	CMThread			m_thread;
	HANDLE				m_event;
	std::mutex			m_mutex;
	std::atomic<bool>	m_stop;
public:
	/*���̺߳����������ϴ��б��з������ݰ�*/
	int ThreadMain()
	{
		while (m_stop)
		{
			//�������գ���ô����ȴ�
			if (m_quePackets.size() <= 0)
			{
				WaitForSingleObject(m_event, INFINITE);
			}
			ResetEvent(m_event);			//���¼�����Ϊ���ź�״̬
			CPacket pack;					//�������ݰ�����
			m_quePackets.pop_front(pack);   //���б��������ݣ���ֵ�����ݰ�����
			_SendPacket(pack);              //���÷�װ�õ�_SendPacket�����������ݰ����ͳ�ȥ
		}
		return -1;
	}
	/*�������ݰ�����*/
	void _SendPacket(CPacket& pack)
	{
		
		CClientSocket clientSock;
		/*�����׽��֣����������ַ��ip*/
		clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
		/*���û�������С*/
		clientSock.SetBufferSize(1024 * 1024 * 2);
		/*�������ݰ�*/
		clientSock.Send(pack);
		/*�������յ������ݰ����������������*/
		clientSock.DealCommand();
		/*�ر��׽���*/
		clientSock.CloseSocket();
	}
	/*����ʵ�ʹ�������*/
	CRequest()
	{
		/*��ʵ�ʵĹ����������ø��̶߳����work����*/
		m_thread.Work(CMWork(this,(MT_FUNC)&CRequest::ThreadMain));
		/*�����߳�*/
		m_thread.Start();
		/*�����¼�����Ϊfalse*/
		m_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_stop = true;
	}

	~CRequest()
	{
		m_stop = false; //�˳�ѭ��
	}
	/*���ݰ����͸�����*/
	void SendPacket(WORD nCmd, BYTE* data, DWORD len)
	{
		
		TRACE(" push back ->  %d  size  %d\r\n", GetTickCount64(), m_quePackets.size());
		m_mutex.lock();
		/*�������е����ݰ�����10��ʱ��������ǰ������ݰ�*/
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
	/*���ݰ����͸�����*/
	void SendPacket(CPacket& packet)
	{
		TRACE(" push back ->  %d  size  %d\r\n", GetTickCount64(), m_quePackets.size());
		m_quePackets.push_back(packet);
		SetEvent(m_event);
	}
};