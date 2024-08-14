#pragma once

#include "resource.h"

/*
 * CMToolbar类：自定义工具类
 *	1.私用成员：工具条、图标
 *	2.Create函数：创建自定义的工具条
 * 
 */

class CMToolbar
{
	CToolBar m_toolbar;
	CImageList img;
public:
	void Create(CWnd* pParent,int toolbarId)
	{
		//创建ToolBar工具条
		if (!m_toolbar.CreateEx(pParent, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_toolbar.LoadToolBar(toolbarId))
			//IDR_TOOLBAR2 工具条资源id
		{
			TRACE0("Failed to Create Dialog ToolBar");
			return;
		}
		//重新定位工具条，使其显示在窗口的顶部
		pParent->RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
		//设置工具条大小和位置
		CRect re;
		m_toolbar.GetWindowRect(&re);//获取工具条的当前矩形位置
		re.left = 370;
		re.top = 7;
		re.bottom = 60;
		re.right -= 35;
		m_toolbar.MoveWindow(&re);// 调整工具条的位置和大小
		//创建图标列表  CImageList
		img.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);//加载图片大小，图片格式，图片数量
		//加载图标
		HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_MOUSE), IMAGE_ICON, 64, 64, 0);
		img.Add(hIcon);
		HICON hIcon2 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_KEY), IMAGE_ICON, 64, 64, 0);
		img.Add(hIcon2);
		HICON hIcon3 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_LOCK), IMAGE_ICON, 64, 64, 0);
		img.Add(hIcon3);
		HICON hIcon4 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_UNLOCK), IMAGE_ICON, 64, 64, 0);
		img.Add(hIcon4);
		m_toolbar.GetToolBarCtrl().SetImageList(&img);
		//文字
		m_toolbar.SetButtonText(0, L"鼠标控制");
		m_toolbar.SetButtonText(2, L"键盘控制");
		m_toolbar.SetButtonText(4, L"锁定用户");
		m_toolbar.SetButtonText(6, L"解锁用户");
	}
	//void AddItem(int imgId,)
};