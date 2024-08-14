#pragma once

// 用户信息表对话框 -> 添加客户按钮弹出

/*继承自CDialogEx，表示这是一个对话框类*/
class CUserInfoDlg : public CDialogEx
{
	/*声明动态创建这个类的宏*/
	DECLARE_DYNAMIC(CUserInfoDlg)
private:
public:
	/*定义了标准构造函数，接受一个父窗口指针参数，默认为空*/
	CUserInfoDlg(CWnd* pParent = nullptr);   
	virtual ~CUserInfoDlg();

// 在设计时被使用，定义了对话框的资源ID IDD
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_USER_INFO };
#endif

protected:
	/*重载了数据交换和验证的虚函数，用于将控件和变量之间进行数据交换*/
	virtual void DoDataExchange(CDataExchange* pDX);    
	/*声明消息映射宏*/
	DECLARE_MESSAGE_MAP()
public:
	// 标识
	CString m_alias;
	// 用户名
	CString m_username;
	// 操作系统
	CString m_os;
	// 运行时间
	CString m_runtime;
	// ip地址
	CString m_ip;
	// 端口
	CString m_port;
	//添加/修改
	bool isAdd;
	/*设置对话框的数据*/
	void SetData(bool _isAdd = true, const CString& _m_alias = nullptr, const CString& _m_username = NULL, const CString& _m_os = NULL, const CString& _m_runtime = NULL, const CString& _m_ip = NULL, const CString& _m_port = NULL);
	/*处理“确定”按钮点击事件的函数*/
	afx_msg void OnBnClickedBtnOk();
	/*处理“取消”按钮点击事件的函数*/
	afx_msg void OnBnClickedBtnCancel();
	/*重载了对话框初始化函*/
	virtual BOOL OnInitDialog();
};