#pragma once

#include <Windows.h>
#include <list>

/* ���������еİ�ȫ���⣿���� */

/*
 * CMQueueģ����:ʹ��Windows I/O��ɶ˿ڣ�IOCP������ʵ�ֵ�ģ����CMQueue����ʵ����һ���̰߳�ȫ�Ķ���
 *	1.˽�г�Ա����������������ݵ�list����   ��ɶ˿ھ��  �߳̾��
 * 
 *	2.ö�ٱ�����
 *		��ӡ����ӡ���ȡ���д�С����ն���
 * 
 *	3.M�ṹ�壺ʹ��ģ�����Tʹ���ܹ�����ͬ���͵����ݣ���ͨ���¼��������ͬ������
 *		3.1 hEvent���¼�����
 *		3.2 T������ṹ����������κ����� T����������б��е����ݡ�
 *		3.3 ���캯��������CreateEvent����һ�� Windows �¼����󣬲���������ֵ�� hEvent��
 *		3.4 ����������ɾ���¼����
 * 
 *	4.��̬��Ա���� =��void ThreadEntry(void* arg)���߳���ں���
 *		4.1 �������������̺߳���ThreadMain
 *		4.2 �����߳�
 * 
 *	5.���̺߳���ThreadMain���������������ڴ�����ɶ˿ڽ��յ�����
 *		5.1 ˽�г�Ա������GetQueuedCompletionStatus����
 *		5.2 ѭ��������GetQueuedCompletionStatus���˿ڽ��յ�����ͽ���whileѭ��
 *			5.2.1 �˳�������飺transferred=-1 and completionKey��Ϊ-1 and lpOverlapped�� =�� ��completionKeyǿ��ת��ΪM����ָ��pM  �����¼�Ϊ���ź�״̬
 *			5.2.2 ����ͬ�����״̬ -> transferred
 *					PUSH����completionKeyָ��Ķ�����뵽lstData�б��ĩβ
 *					POP�����ÿյ�M���͵�ָ��pM
 *						 ȡ��lstData�б�ĵ�һ��Ԫ�ظ�ֵ��pM->t���Ƴ���Ԫ��
 *						 �����¼�����
 *					SIZE�����ÿյ�M���͵�ָ��pM
 *						  ���б��еĴ�С��ֵ��pM->wParam
 *						  �����¼�����
 *					CLEAR�����ÿյ�M���͵�ָ��pM
 *						   ����б�����
 *						   �����¼�����
 * 
 *	6.���캯���������̺߳�I/O��ɶ˿�
 *		6.1 ��װ_beginthread�����߳�
 *		6.2 ��װCreateIoCompletionPort������ɶ˿�
 * 
 *  7.������������ն��С��ر��߳��Լ�������ܵĴ������
 *		7.1 ���÷�װ�õ�clear��������ɶ˿ڷ����������
 *		7.2 ����M����m
 *		7.3 ͨ��PostQueuedCompletionStatus����ɶ˿ڷ��������ʾ�߳��˳�
 *		7.4 ����WaitForSingleObject������ThreadMain�߳������õ�����Ϣ���¼�����
 * 
 *	8.push_back��������һ��Ԫ����ӵ������У���ͨ��I/O��ɶ˿�֪ͨ�����߳̽��д���
 *		8.1 newһ��T���͵�ָ�룬�����������
 *		8.2 ����PostQueuedCompletionStatus���˿ڷ��������Ҫ��������ݣ�const T& t��
 * 
 *	9.pop_front������
 *		9.1 �����յ�M���͵Ķ���m����������������������������
 *		9.2 ����PostQueuedCompletionStatus������ɶ˿ڷ�������
 *		9.3 ����WaitForSingleObject������ThreadMain�߳������õ�����Ϣ���¼�����
 * 
 *  10.size������clear������ԭ���pop_frontһ��
 * 
*/

template<typename T>
class CMQueue
{
private:
	enum
	{
		PUSH = 1001,  //���
		POP,		  //����
		SIZE,         //��ȡ��С
		CLEAR,        //��ն���
	};
	/*ʹ��ģ�����Tʹ���ܹ�����ͬ���͵����ݣ���ͨ���¼��������ͬ��������*/
	struct M
	{
		HANDLE hEvent;
		T t;           //����б��е�����
		int wParam;    //����б��С��״̬
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
	std::list<T>	lstData;     //�洢�������ݵı�׼������
	HANDLE			m_hIocp;     //I/O��ɶ˿ڵľ��
	HANDLE			m_hThread;   //�����̵߳ľ��
private:
	static void ThreadEntry(void* arg)
	{
		CMQueue* thiz = (CMQueue*)arg;
		thiz->ThreadMain();
		_endthread();
	}
	/*���ڴ��� I/O ��ɶ˿� (IOCP) ���߳�������*/
	void ThreadMain()
	{
		DWORD transferred;          //������ɵ��ֽ���
		ULONG_PTR completionKey;    //������� I/O ��������ɼ������ͱ���
		LPOVERLAPPED lpOverlapped;  //������� I/O �������ص��ṹ
		/*GetQueuedCompletionStatus���ڴ�IOCP��ȡ���״̬�����transferred��completionKey��lpOverlapped����*/
		/*GetQueuedCompletionStatus��һ������������������ɶ˿�m_hIocp�л�ȡһ����ɵ�I/O����*/

		/*lpCompletionKey����Ϊ��ɼ������ݵ����ݱ���Ϊ��������ݣ�����Ӧ������ÿ��socket�����Ӧ
		  lpOverlapped����Ϊ�ص��ṹ�壬���ݵ����ݱ���Ϊ��IO���ݣ�����Ӧ����ÿ�εĲ���WSARecv��WSASend�����Ӧ*/
		while (GetQueuedCompletionStatus(m_hIocp/*��ɶ˿ھ��*/, &transferred, &completionKey, &lpOverlapped, INFINITE/*���޵ȴ�ֱ����һ��I/O���*/))
		{
			/*�˳��������*/
			if ((transferred == -1) && (completionKey != -1) && (lpOverlapped == NULL))
			{
				/*��completionKeyǿ��ת��ΪM����ָ��pM*/
				M* pM = (M*)completionKey;
				/*����pM���¼�hEvent,֪ͨ�������*/
				SetEvent(pM->hEvent);
				break;
			}
			/*����ͬ�����״̬*/
			switch (transferred)
			{
			case PUSH:
			{
				/*��completionKeyָ��Ķ�����뵽lstData�б��ĩβ*/
				lstData.push_back(*(T*)completionKey);
				delete (T*)completionKey;
				break;
			}
			case POP:
			{
				M* pM = (M*)completionKey;/*PostQueuedCompletionStatus����Ŀյ� M m(t)*/
				if (lstData.size() > 0)
				{
					/*ȡ��lstData�б�ĵ�һ��Ԫ�ظ�ֵ��pM->t���Ƴ���Ԫ��*/
					pM->t = lstData.front();
					lstData.pop_front();
				}
				else
				{
					pM->wParam = 0;
				}
				/*����pM���¼� hEvent,֪ͨ�������*/
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
	/*�����̺߳�I/O��ɶ˿�*/
	CMQueue()
	{
		m_hThread = (HANDLE) _beginthread(ThreadEntry, 0, this);
		m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE/*����һ���µ�I/O��ɶ˿�*/, NULL, 0, 1);
	}
	/*��CMQueue��������ʱ���������������������ն��С��ر��߳��Լ�������ܵĴ������*/
	~CMQueue()
	{
		clear();   //����ڲ������ݶ���
		T t;       
		M m(t);    //����M����m��hEventΪfalse wParamΪ-1 tΪ��
		/*֪ͨ�߳��˳�*/
		/*PostQueuedCompletionStatus������I/O��ɶ˿�m_hIocpͶ��һ����������״̬��֪ͨ�����߳��˳�*/
		if (!PostQueuedCompletionStatus(m_hIocp, -1/*�����transferredֵ������ָʾ�߳��˳�*/, (ULONG_PTR)&m/*��m�ĵ�ַ��Ϊ��ɼ�*/, NULL))
		{
			::AfxMessageBox(L"~CMQueue Error1");
			return;
		}
		/*�ȴ��߳��˳����ȴ�100����*/
		if (WaitForSingleObject(m.hEvent, 100) != WAIT_OBJECT_0)
		{
			::AfxMessageBox(L"~CMQueue Error2");
			return;
		}
	}
	/*��һ��Ԫ����ӵ������У���ͨ��I/O��ɶ˿�֪ͨ�����߳̽��д���*/
	bool push_back(const T& t)
	{
		/*ͨ��new����������һ���µ�T���Ͷ���pT����ʹ�ô���Ĳ���t��ʼ����*/
		T* pT = new T(t);
		/*��I/O��ɶ˿�m_hIocpͶ��һ�����״̬����֪ͨ�����߳����µ�Ԫ����Ҫ����*/
		if (!PostQueuedCompletionStatus(m_hIocp, PUSH, (ULONG_PTR)pT, NULL))
		{
			delete pT;
			return false;
		}
		return true;
	}

	bool pop_front(T& t)
	{
		M m(t);//�յ� �������¼����������T��������ݣ���wParam����
		if (!PostQueuedCompletionStatus(m_hIocp, POP, (ULONG_PTR)&m/*��������*/, NULL))
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