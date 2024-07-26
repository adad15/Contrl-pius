#pragma once

#include "pch.h"
#include "framework.h"
#include <atomic>
#include <vector>
#include <mutex>

/*
 * �߳� + �¼�����
 *	1.�����̣߳�_beginthread
 *  2.�ȴ��̣߳�WaitForSingleObject
 * 
 * �߳�ͬ�������¼�����
 *	1.����CreateEvent��������¼�����
 *  2.����SetEvent��ָ�����¼���������Ϊ���ź�״̬
 *  3.����ResetEvent��ָ�����¼���������Ϊ���ź�״̬
 *  4.����WaitForSingleObject�����¼�����
 * 
 */

/*
 * CMFuncBase:����
 * 
 * MT_FUNC��ָ��CMFuncBase��ĳ�Ա������ָ��
 		class CMFuncBase {
 		public:
			int someFunction() {}
		};
		MT_FUNC funcPtr = &CMFuncBase::someFunction;
		funcPtr ��һ�� MT_FUNC ���͵ı�����ָ��CMFuncBase��ĳ�Ա����someFunction��

 * CMWork�ࣺ��װ��һ����Ա����ָ���һ������ָ��
 *		1.���캯������ʼ������ָ��ͳ�Ա����ָ�롣
 *		2.���غ������ò�������ʹ�����ŵ��ó�Ա������
 * 
 * CMThread�ࣺ�߳��࣬��������CMWork����
 *		1.˽�г�Ա������ԭ������ָ��->CMWork*  �߳̾�� �¼����m_event�߳����б�־
 *																	
 *																						2.1 ͨ������Ĳ��������������̺߳���
 *		2.��̬��Ա����->�߳���ں���:void ThreadEntry(void* arg),����CMThread�߳������ָ��
 *																						2.2 ������ǰ�߳�
 * 
 *										  3.1 ͨ��WaitForSingleObject�ȴ��¼�����m_event�����ź�״̬���߳�ͬ������
 *		3.�߳�������->ThreadMain��ѭ�� =��
 *										  3.2 ����CMWork�еĳ�Ա���������������������������е�m_event��Ϊ���ź�״̬
 * 
 * 
 *		4.Start�����������߳� => ����_beginthread��CMThread������ThreadEntry�У������С�
 * 
 *		5.Work����������ʵ�ʵĹ�����������ִ�� =������CMWork���󣻲����¼�����m_event����Ϊ���ź�״̬
 * 
 *								6.1 ��m_run����Ϊfalse�������̺߳����е�ѭ����
 *		6.Stop�������ر��߳� =��
 *								6.2 ����WaitForSingleObject����߳��Ƿ����
 * 
 *		7.IsFree����������߳��Ƿ����
 * 
 * 
 * CMThreadPool�ࣺ�̳߳��࣬������CMThread�߳�
 *		1.˽�г�Ա������vector�����������߳���CMThread   ������
 * 
 *										  2.1 ����
 *		2.���캯�� =������ָ���������߳�  2.2 vector�����м���new�������߳���
 *										  2.3 ����
 * 
 *												3.1 ����stop����ֹͣ�����߳�
 *		3.�������� => ֹͣ�����̲߳��ͷ���Դ	3.2 delete��new�������ڴ�
 *												3.3 �ӽ��� =�����vector����
 *												
 *		4.Invoke���������������̣߳����ø����̵߳�Start���� =������_beginthread
 * 
 *		5.Stop�������ر������߳�
 *									
 *														6.1 ����߳����Ƿ����
 *		6.DispatchWork��������������������������߳�	
 *														6.2 �����߳����е�Work���������빤�������࣬�Ա�������ù������е�ʵ�ʹ�������
 * 
 */

class CMFuncBase{};
typedef int (CMFuncBase::* MT_FUNC)();

/*��װ��һ����Ա����ָ���һ������ָ��*/
class CMWork
{
public:
	CMFuncBase* thiz; //����ָ��
	MT_FUNC     func; //��Ա����ָ��
	CMWork(CMFuncBase* objThis, MT_FUNC objFunc)
	{
		thiz	= objThis;
		func	= objFunc;
	}
	/*ʹ�����ŵ��ó�Ա����*/
	int operator()()
	{
		return (thiz->*func)();
	}
};

/*�߳��࣬��������CMWork����*/
class CMThread
{
private:
	std::atomic<CMWork*> m_work;   //ԭ������ָ�룬�洢��ǰҪִ�еĹ���
	HANDLE               m_thread; //�߳̾��
	HANDLE				 m_event;  //�¼���������������̵߳ȴ��ͻ���
	bool				 m_run;    //�߳����б�־
	/*�߳���ں��������� ThreadMain*/
	static void ThreadEntry(void* arg)
	{
		CMThread* thiz = (CMThread*)arg;
		thiz->ThreadMain();
		_endthread();    //����_endthread ������������ǰ�̡߳�
	}
	/*�߳���������ѭ���ȴ��¼�������ִ�й���*/
	void ThreadMain()
	{
		/*ѭ��*/
		while (m_run)
		{
			/* �ȴ�һ���¼�����m_event���ڶ�������INFINITE��ʾ�����ڵȴ���
			 * m_event��һ��ͬ���¼�����ͨ�������̼߳��ͬ����
			 * �õ�ǰ�߳������ڵصȴ���ֱ�� m_event �¼���������Ϊ���ź�״̬
			 */
			WaitForSingleObject(m_event, INFINITE/*�����ڵȴ�*/);

			int ret = (*m_work)();  //���ó�Ա����m_workָ��ĳ�Ա����
			if (ret != 0)
			{
				delete m_work;
				m_work = NULL;
			}
			else
			{
				Notify(true);  //���¼�����m_event����Ϊ���ź�״̬
			}
		}
	}
public:
	CMThread()
	{
		m_work		= NULL;
		m_thread	= INVALID_HANDLE_VALUE;
		m_event		= CreateEvent(NULL, FALSE, FALSE, NULL);
		m_run		= true;
	}
	~CMThread()
	{
		Stop();
	}
	/*�����¼������ú�����*/
	void Notify(bool b)
	{
		if (b)
		{
			SetEvent(m_event);   //���¼�����m_event����Ϊ���ź�״̬�����ѵȴ����¼����̡߳�
		}
		else
		{
			ResetEvent(m_event); //���¼�����m_event����Ϊ���ź�״̬���õȴ����¼����̼߳����ȴ���
		}
	}
	/*���乤����֪ͨ�߳�ִ��*/
	void Work(const CMWork& work)
	{
		if (m_work != NULL)
		{
			delete m_work;
		}
		m_work = new CMWork(work);
		Notify(true);    //���¼�����m_event����Ϊ���ź�״̬
	}
	/*�����߳�*/
	bool Start()
	{
		/*this �Ǵ��ݸ�ThreadEntry�����Ĳ�����ָ��ǰCMThread����*/
		m_thread = (HANDLE)_beginthread(&CMThread::ThreadEntry, 0, this);
		if (m_thread == NULL)
		{
			TRACE("�߳�����ʧ��\r\n");
			return false;
		}
		return true;
	}
	/*�ر��߳�*/
	bool Stop()
	{
		if (m_run == false) return true;
		m_run = false;  //���ó�false������ѭ��
		/*�õ�ǰ�̵߳ȴ���� 100 ���룬�Լ��m_thread�߳��Ƿ��Ѿ�����*/
		int ret = WaitForSingleObject(m_thread, 100);
		printf("stop ret : %d\r\n", ret);
		/*WAIT_OBJECT_0: ��ʾ�ȴ��Ķ����̣߳��Ѿ���Ϊ���ź�״̬�����߳��Ѿ�������*/
		if (ret == WAIT_OBJECT_0)
		{
			return true;
		}
		return false;
	}
	/*����߳��Ƿ����*/
	bool IsFree()
	{
		if (m_run && (m_work == NULL))
		{
			return true;
		}
		return false;
	}
};

/*��һ���̳߳��࣬������CMThread�߳�*/
class CMThreadPool
{
private:
	std::vector<CMThread*>	m_vecThreads;  //�洢�̶߳��������
	std::mutex				m_mutex;       //�����̰߳�ȫ�Ļ�����
public:
	/*����ָ���������߳�*/
	CMThreadPool(int count)
	{
		m_mutex.lock();
		for (int i = 0; i < count; i++)
		{
			m_vecThreads.push_back(new CMThread());
		}
		m_mutex.unlock();
	}
	/*ֹͣ�����̲߳��ͷ���Դ*/
	~CMThreadPool()
	{
		Stop();
		/*�ͷ���Դ*/
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			delete m_vecThreads.at(i);
		}
		m_mutex.lock();
		m_vecThreads.clear();
		m_mutex.unlock();
	}
	/*���������߳�*/
	bool Invoke()
	{
		bool isOk = true;
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			if (!m_vecThreads.at(i)->Start())
			{
				isOk = false;
				break;
			}
		}
		if (isOk)
		{
			return true;
		}
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			m_vecThreads.at(i)->Stop();
		}
		return false;
	}
	/*�ر������߳�*/
	bool Stop()
	{
		bool isOk = true;
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			if (!m_vecThreads.at(i)->Stop())
			{
				isOk = false;
				break;
			}
		}
		if (isOk == false)
		{
			printf("�̹߳ر�ʧ��");
		}
		//assert(isOk);
		return isOk;
	}
	/*��������������������߳�*/
	void DispatchWork(const CMWork& work/*��������*/)
	{
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			if (m_vecThreads.at(i)->IsFree())
			{
				m_vecThreads.at(i)->Work(work);
				break;
			}
		}
	}
};