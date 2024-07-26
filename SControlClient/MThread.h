#pragma once

#include "pch.h"
#include "framework.h"
#include <atomic>
#include <vector>
#include <mutex>

/*
 * 线程 + 事件对象
 *	1.唤起线程：_beginthread
 *  2.等待线程：WaitForSingleObject
 * 
 * 线程同步——事件对象
 *	1.调用CreateEvent创建或打开事件对象
 *  2.调用SetEvent把指定的事件对象设置为有信号状态
 *  3.调用ResetEvent把指定的事件对象设置为无信号状态
 *  4.调用WaitForSingleObject请求事件对象
 * 
 */

/*
 * CMFuncBase:基类
 * 
 * MT_FUNC：指向CMFuncBase类的成员函数的指针
 		class CMFuncBase {
 		public:
			int someFunction() {}
		};
		MT_FUNC funcPtr = &CMFuncBase::someFunction;
		funcPtr 是一个 MT_FUNC 类型的变量，指向CMFuncBase类的成员函数someFunction。

 * CMWork类：封装了一个成员函数指针和一个对象指针
 *		1.构造函数：初始化对象指针和成员函数指针。
 *		2.重载函数调用操作符：使用括号调用成员函数。
 * 
 * CMThread类：线程类，用来处理CMWork对象
 *		1.私有成员变量：原子类型指针->CMWork*  线程句柄 事件句柄m_event线程运行标志
 *																	
 *																						2.1 通过传入的参数运行真正的线程函数
 *		2.静态成员函数->线程入口函数:void ThreadEntry(void* arg),传入CMThread线程类对象指针
 *																						2.2 结束当前线程
 * 
 *										  3.1 通过WaitForSingleObject等待事件对象m_event至有信号状态，线程同步操作
 *		3.线程主函数->ThreadMain：循环 =》
 *										  3.2 调用CMWork中的成员函数（工作函数），并将该类中的m_event设为有信号状态
 * 
 * 
 *		4.Start函数：启动线程 => 调用_beginthread将CMThread对象传入ThreadEntry中，并运行。
 * 
 *		5.Work函数：分配实际的工作函数，并执行 =》创建CMWork对象；并将事件对象m_event设置为有信号状态
 * 
 *								6.1 将m_run设置为false，跳出线程函数中的循环。
 *		6.Stop函数：关闭线程 =》
 *								6.2 调用WaitForSingleObject检查线程是否结束
 * 
 *		7.IsFree函数：检查线程是否空闲
 * 
 * 
 * CMThreadPool类：线程池类，管理多个CMThread线程
 *		1.私有成员变量：vector容器：储存线程类CMThread   互斥锁
 * 
 *										  2.1 加锁
 *		2.构造函数 =》创建指定数量的线程  2.2 vector容器中加入new出来的线程类
 *										  2.3 解锁
 * 
 *												3.1 调用stop函数停止所有线程
 *		3.析构函数 => 停止所有线程并释放资源	3.2 delete掉new出来的内存
 *												3.3 加解锁 =》清楚vector容器
 *												
 *		4.Invoke函数：启动所有线程，调用各个线程的Start函数 =》调用_beginthread
 * 
 *		5.Stop函数：关闭所有线程
 *									
 *														6.1 检查线程类是否空闲
 *		6.DispatchWork函数：将工作函数分配给空闲线程	
 *														6.2 调用线程类中的Work函数，传入工作函数类，以备后序调用工作类中的实际工作函数
 * 
 */

class CMFuncBase{};
typedef int (CMFuncBase::* MT_FUNC)();

/*封装了一个成员函数指针和一个对象指针*/
class CMWork
{
public:
	CMFuncBase* thiz; //对象指针
	MT_FUNC     func; //成员函数指针
	CMWork(CMFuncBase* objThis, MT_FUNC objFunc)
	{
		thiz	= objThis;
		func	= objFunc;
	}
	/*使用括号调用成员函数*/
	int operator()()
	{
		return (thiz->*func)();
	}
};

/*线程类，用来处理CMWork对象*/
class CMThread
{
private:
	std::atomic<CMWork*> m_work;   //原子类型指针，存储当前要执行的工作
	HANDLE               m_thread; //线程句柄
	HANDLE				 m_event;  //事件句柄，用来控制线程等待和唤醒
	bool				 m_run;    //线程运行标志
	/*线程入口函数，调用 ThreadMain*/
	static void ThreadEntry(void* arg)
	{
		CMThread* thiz = (CMThread*)arg;
		thiz->ThreadMain();
		_endthread();    //调用_endthread 函数，结束当前线程。
	}
	/*线程主函数，循环等待事件触发并执行工作*/
	void ThreadMain()
	{
		/*循环*/
		while (m_run)
		{
			/* 等待一个事件对象m_event，第二个参数INFINITE表示无限期等待。
			 * m_event是一个同步事件对象，通常用于线程间的同步。
			 * 让当前线程无限期地等待，直到 m_event 事件对象被设置为有信号状态
			 */
			WaitForSingleObject(m_event, INFINITE/*无限期等待*/);

			int ret = (*m_work)();  //调用成员变量m_work指向的成员函数
			if (ret != 0)
			{
				delete m_work;
				m_work = NULL;
			}
			else
			{
				Notify(true);  //将事件对象m_event设置为有信号状态
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
	/*控制事件的设置和重置*/
	void Notify(bool b)
	{
		if (b)
		{
			SetEvent(m_event);   //将事件对象m_event设置为有信号状态，唤醒等待该事件的线程。
		}
		else
		{
			ResetEvent(m_event); //将事件对象m_event重置为无信号状态，让等待该事件的线程继续等待。
		}
	}
	/*分配工作并通知线程执行*/
	void Work(const CMWork& work)
	{
		if (m_work != NULL)
		{
			delete m_work;
		}
		m_work = new CMWork(work);
		Notify(true);    //将事件对象m_event设置为有信号状态
	}
	/*启动线程*/
	bool Start()
	{
		/*this 是传递给ThreadEntry函数的参数，指向当前CMThread对象*/
		m_thread = (HANDLE)_beginthread(&CMThread::ThreadEntry, 0, this);
		if (m_thread == NULL)
		{
			TRACE("线程启动失败\r\n");
			return false;
		}
		return true;
	}
	/*关闭线程*/
	bool Stop()
	{
		if (m_run == false) return true;
		m_run = false;  //设置成false，跳出循环
		/*让当前线程等待最多 100 毫秒，以检查m_thread线程是否已经结束*/
		int ret = WaitForSingleObject(m_thread, 100);
		printf("stop ret : %d\r\n", ret);
		/*WAIT_OBJECT_0: 表示等待的对象（线程）已经变为有信号状态，即线程已经结束。*/
		if (ret == WAIT_OBJECT_0)
		{
			return true;
		}
		return false;
	}
	/*检查线程是否空闲*/
	bool IsFree()
	{
		if (m_run && (m_work == NULL))
		{
			return true;
		}
		return false;
	}
};

/*是一个线程池类，管理多个CMThread线程*/
class CMThreadPool
{
private:
	std::vector<CMThread*>	m_vecThreads;  //存储线程对象的容器
	std::mutex				m_mutex;       //用于线程安全的互斥锁
public:
	/*创建指定数量的线程*/
	CMThreadPool(int count)
	{
		m_mutex.lock();
		for (int i = 0; i < count; i++)
		{
			m_vecThreads.push_back(new CMThread());
		}
		m_mutex.unlock();
	}
	/*停止所有线程并释放资源*/
	~CMThreadPool()
	{
		Stop();
		/*释放资源*/
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			delete m_vecThreads.at(i);
		}
		m_mutex.lock();
		m_vecThreads.clear();
		m_mutex.unlock();
	}
	/*启动所有线程*/
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
	/*关闭所有线程*/
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
			printf("线程关闭失败");
		}
		//assert(isOk);
		return isOk;
	}
	/*将工作函数分配给空闲线程*/
	void DispatchWork(const CMWork& work/*工作函数*/)
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