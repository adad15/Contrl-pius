#pragma once
#include "MToolbar.h"
#include "Request.h"
// CScreenWatch 远控屏幕界面

/*
 * 
 * 
 *
 * 
 */

class CScreenWatch : public CDialogEx
{
	DECLARE_DYNAMIC(CScreenWatch)

private:
	CRequest req;         // 用于处理请求
	CToolBar m_toolbar;   // 工具栏对象
	CImageList img;       // 图像列表
	bool isControlMouse;  // 标识是否控制鼠标
	bool isControlKey;    // 标识是否控制键盘
	bool showOver;
	CImage image;         // 图像对象
	int		imageWidth;   // 图像的宽度
	int		imageHeight;  // 图像的高度
	CStatic m_picScreen;  // 静态控件，用于显示屏幕图像
	void CreateToolBar(); // 创建工具栏
	HANDLE hThread;
	bool threadIsRunning;
	static void ThreadEntryScreenWatch(void* arg); // 线程入口函数
	void ThreadScreenWatch();					   // 线程函数
	CPoint ClientPt2GlobalPt(CPoint& clientPt);    // 将客户端坐标转换为全局坐标
	void ShowDesktop(DWORD code);                  // 显示桌面

public:
	CScreenWatch(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CScreenWatch();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCREEN_WATCH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();              // 对话框初始化
	afx_msg void OnTimer(UINT_PTR nIDEvent);  // 定时器处理
	virtual void OnCancel();                  // 取消操作
	// 鼠标和键盘控制命令处理
	afx_msg void CmdControlMouse();
	afx_msg void CmdControlKey();
	// 锁定和解锁机器
	afx_msg void CmdLockMachine();
	afx_msg void CmdUnLockMachine();
	// 鼠标左键双击、按下和释放事件
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	// 鼠标右键双击、按下和释放事件
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	// 窗口移动
	afx_msg void OnMove(int x, int y);
	// 鼠标移动
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	// 按钮点击
	afx_msg void OnBnClickedButton1();
	// 键盘按下
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	// 消息预翻译
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CEdit m_edit;
};