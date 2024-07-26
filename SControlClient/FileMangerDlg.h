#pragma once
#include "DownLoadFileDlg.h"
// CFileMangerDlg 文件管理对话框

class CFileMangerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileMangerDlg)
private:
	CToolBar   m_toolbar;    //工具栏
	CImageList toolbarImg;   //工具栏图像列表
	CImageList treeImg;      //树控件图像列表
	CImageList listImg;      //列表控件图像列表
	CDownLoadFileDlg downLoadFileDlg;  //文件下载对话框
	
private:
	void CreateToolBar();      //创建工具栏
	void GetDriveInfo();       //获取驱动器信息
	void InitMTree();		   //初始化树控件
	void InitMList();          //初始化列表控件
	void GetCurTreeItemPath(CString& path);                //获取当前树项目路径
	void DelCurTreeItemChildItem(HTREEITEM hTree);         //删除当前树项目的子项
	void Time2CString(CString& strTime, __time64_t time);  //将时间转换为 CString
	static void ThreadEntryDownLoadFile(void* arg);        //下载文件的线程入口
	void ThreadDownLoadFile();							   //下载文件的线程
public:
	/*公有成员函数和对话框数据*/
	CFileMangerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CFileMangerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILE_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	/*消息映射和控件*/
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();  // 初始化对话框
	CTreeCtrl m_Tree;			  // 树控件
	CListCtrl m_List;             // 列表控件
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);    // 树控件单击事件
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);   // 树控件双击事件
	afx_msg void CmdDownLoadFile();									   // 下载文件命令
	afx_msg LRESULT OnShowDLDlg(WPARAM wParam,LPARAM lParam);          // 显示下载对话框事件
	afx_msg void CmdDeleteFile();								       // 删除文件命令	
	virtual void OnCancel();										   // 取消对话框
};