#include "pch.h"
#include "SControlClient.h"
#include "UserInfoDlg.h"
#include "afxdialogex.h"
#include "SControlClientDlg.h"

// CUserInfoDlg 对话框
IMPLEMENT_DYNAMIC(CUserInfoDlg, CDialogEx)
/*构造函数初始化成员变量*/
CUserInfoDlg::CUserInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_USER_INFO, pParent)
	, m_alias(_T(""))
	, m_username(_T(""))
	, m_os(_T(""))
	, m_runtime(_T(""))
	, m_ip(_T(""))
	, m_port(_T(""))
{
}

CUserInfoDlg::~CUserInfoDlg()
{
}
/*MFC 对话框类中用于数据交换的虚函数*/
void CUserInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	/*每个DDX_Text宏将对话框控件和成员变量进行绑定*/
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_alias);
	DDX_Text(pDX, IDC_EDIT2, m_username);
	DDX_Text(pDX, IDC_EDIT3, m_os);
	DDX_Text(pDX, IDC_EDIT4, m_runtime);
	DDX_Text(pDX, IDC_EDIT5, m_ip);
	DDX_Text(pDX, IDC_EDIT6, m_port);
}
/*设置数据*/
void CUserInfoDlg::SetData(bool _isAdd, const CString& _m_alias, const CString& _m_username, 
	const CString& _m_os, const CString& _m_runtime, const CString& _m_ip, const CString& _m_port)
{
	isAdd = _isAdd;
	if (!_isAdd)
	{
		m_alias			= _m_alias;
		m_username		= _m_username;
		m_os			= _m_os;
		m_runtime		= _m_runtime;
		m_ip			= _m_ip;
		m_port			= _m_port;
	}
}

/*消息映射定义了按钮点击事件的处理函数*/
BEGIN_MESSAGE_MAP(CUserInfoDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_OK, &CUserInfoDlg::OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CUserInfoDlg::OnBnClickedBtnCancel)
END_MESSAGE_MAP()

/*消息处理函数*/
void CUserInfoDlg::OnBnClickedBtnOk()
{
	/*参数true表示将控件中的数据更新到成员变量中（从控件到变量）*/
	UpdateData(true);
	/*这个条件判断isAdd变量的值。如果为true，表示这是一个添加操作；否则表示是一个修改操作*/
	if (isAdd)
	{
		/*返回父窗口的指针，获取远控主界面的指针*/
		CSControlClientDlg* pParent = (CSControlClientDlg*)GetParent();
		/*如果是添加操作，调用父窗口的AddListBeControlItem函数，将对话框中的用户信息传递给父窗口进行添加处理*/
		pParent->AddListBeControlItem(m_alias, m_username, m_os, m_runtime, m_ip, m_port);
	}
	else
	{
		CSControlClientDlg* pParent = (CSControlClientDlg*)GetParent();
		/**/
		pParent->ModListBeControlItem(m_alias, m_username, m_os, m_runtime, m_ip, m_port);
	}
	/*关闭对话框*/
	CDialogEx::OnOK();
}

void CUserInfoDlg::OnBnClickedBtnCancel()
{
	/*OnCancel函数的作用是关闭对话框并返回IDCANCEL*/
	CDialogEx::OnCancel();
}

/*重载了对话框初始化函*/
BOOL CUserInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}