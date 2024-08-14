// SControlClient.h: PROJECT_NAME 应用程序的主头文件
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"		// 主符号

/*
 *	程序入口通过AfxWinMain函数调用该类
 *	程序的执行入口
 *	
 *	MVC模式：
 *  模型（Model）：负责存储系统的中心数据。
 *  视图（View）：将信息显示给用户（可以定义多个视图）。界面，按钮，图片。
 *  控制器（Controller）：处理用户输入的信息。负责从视图读取数据，控制用户输入，并向模型发送数据，
 *  是应用程序中处理用户交互的部分。负责管理与用户交互交互控制。
 * 
 *	CRemoteClientApp类：
 *		1.析构函数：启动程序
 * 
 *		2.InitInstance成员函数 -> 调用CSControlClientDlg类，创建窗口。
 * 
 */

class CSControlClientApp : public CWinApp
{
public:
	CSControlClientApp();
// 重写
public:
	virtual BOOL InitInstance();
// 实现
	DECLARE_MESSAGE_MAP()
};

/* 利用关键字extern，可以在一个文件中引用另一个文件中定义的变量或者函数 */
/* 这样在其他文件中如果需要调用theApp对象，不必拘泥于调用：AfxGetApp()，而只需#include “xxx.h”
直接使用 theApp 即可。 */
extern CSControlClientApp theApp;/* theApp代表应用程序实例 */