// SControlClientDlg.h: 头文件
#pragma once

#include "UserInfoDlg.h"
#include "FileMangerDlg.h"
#include "ScreenWatch.h"
#include "UDPPassClient.h"

#define WM_ONLINE (WM_USER + 10)

/*
 * 
 * 
 * 
 * 
 */

// CSControlClientDlg 对话框:远控主界面
class CSControlClientDlg : public CDialogEx
{

private:
	CToolBar m_toolbar;              // 工具栏对象
	CImageList img;                  // 图像列表对象
	CImageList img2;                
	CFileMangerDlg* fileManagerDlg;  // 文件管理对话框指针
	CScreenWatch* screenWatch;       // 屏幕监控对话框指针

private:
	void CreateToolBar();            // 创建工具栏
	void InitListBeControl();        // 初始化列表控件
	void InitListInfo();
	
private:

public:
	// 响应用户添加、修改、删除操作的消息处理函数
	afx_msg void CmdAddUser();
	afx_msg void CmdModUser();
	afx_msg void CmdDelUser();
	// 用于添加和修改被控列表项的方法
	void AddListBeControlItem(const CString& _m_alias = nullptr, const CString& _m_username = NULL, const CString& _m_os = NULL, const CString& _m_runtime = NULL, const CString& _m_ip = NULL, const CString& _m_port = NULL);
	void ModListBeControlItem(const CString& _m_alias = nullptr, const CString& _m_username = NULL, const CString& _m_os = NULL, const CString& _m_runtime = NULL, const CString& _m_ip = NULL, const CString& _m_port = NULL);

public:
	CSControlClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCONTROLCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;// 对话框的图标

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnOnLine(WPARAM wParam, LPARAM lParam);// 处理在线状态的消息
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);    // 处理系统命令
	afx_msg void OnPaint();								   // 处理绘画消息
	afx_msg HCURSOR OnQueryDragIcon();					   // 处理拖动图标的查询
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl					m_ListBeControl; // 被控列表控件
	CListCtrl					m_ListInfo;      // 信息列表控件
	UDPPassClient				m_udppass;       // UDP 通信客户端对象
	std::vector<long long>		m_ids;           // 存储 ID 的向量

	// 响应文件管理和屏幕监控命令的消息处理函数
	afx_msg void CmdFileManager();
	afx_msg void CmdScreenWatch();
	// 响应按钮点击事件的消息处理函数
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};