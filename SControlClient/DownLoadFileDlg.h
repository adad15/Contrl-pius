#pragma once
// CDownLoadFileDlg 文件下载进度界面

class CDownLoadFileDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDownLoadFileDlg)

public:
	CDownLoadFileDlg(CWnd* pParent = nullptr);   // 构造函数，初始化对话框
	virtual ~CDownLoadFileDlg();

// 数据交换，用于控件与变量之间的关联
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLFILE_STATUS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_Info;
	CProgressCtrl m_Pro;  // 进度条范围
	CStatic m_TxtPro;
	CButton m_BtnOk;
	long long fileLen;    // 文件长度
	int bfb;
	 
	void SetFileLength(long long _fileLen); // 设置文件总长度
	void SetDownLoadedLen(long long len);   // 设置已下载长度
	void SetInfo(CString& info);            // 设置下载信息

	afx_msg void OnBnClickedBtnDlOk();      // 按钮点击事件处理函数
	virtual void OnCancel();                // 取消事件处理函数
};