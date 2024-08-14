#include "pch.h"
#include "SControlClient.h"
#include "DownLoadFileDlg.h"
#include "afxdialogex.h"

// CDownLoadFileDlg 对话框
IMPLEMENT_DYNAMIC(CDownLoadFileDlg, CDialogEx)

CDownLoadFileDlg::CDownLoadFileDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLFILE_STATUS, pParent)
{
}

CDownLoadFileDlg::~CDownLoadFileDlg()
{
}

void CDownLoadFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	// 将控件ID与成员变量关联
	DDX_Control(pDX, IDC_EDIT_DL_INFO, m_Info);
	DDX_Control(pDX, IDC_PROGRESS_DL, m_Pro);
	DDX_Control(pDX, IDC_TXT_DL_PRO, m_TxtPro);
	DDX_Control(pDX, IDC_BTN_DL_OK, m_BtnOk);
}

// 设置文件总长度
void CDownLoadFileDlg::SetFileLength(long long _fileLen)
{
	fileLen = _fileLen;
	// 设置进度条范围
	m_Pro.SetRange(0, 100);
}

// 设置已下载长度
void CDownLoadFileDlg::SetDownLoadedLen(long long _len)
{
	int _bfb = (int)(1.0 * _len / fileLen * 100);
	if (_bfb != bfb)
	{
		CString str;
		str.Format(L"%d", _bfb);     // 格式化进度百分比
		str.Append(L"%");
		m_TxtPro.SetWindowText(str); // 更新进度文本
		m_Pro.SetPos(_bfb);          // 更新进度条位置
	}	
	if (_len == fileLen)
	{
		m_BtnOk.ShowWindow(SW_SHOW); // 下载完成，显示“确定”按钮
	}
	bfb = _bfb;						 // 更新当前进度百分比
}

// 设置下载信息
void CDownLoadFileDlg::SetInfo(CString& info)
{
	m_Info.SetWindowText(info);// 更新信息显示
}

// 消息映射表
BEGIN_MESSAGE_MAP(CDownLoadFileDlg, CDialogEx)
	// 映射按钮点击事件到相应的处理函数
	ON_BN_CLICKED(IDC_BTN_DL_OK, &CDownLoadFileDlg::OnBnClickedBtnDlOk)
END_MESSAGE_MAP()


// 按钮点击事件处理函数
void CDownLoadFileDlg::OnBnClickedBtnDlOk()
{
	DestroyWindow();// 销毁对话框
}

// 取消事件处理函数
void CDownLoadFileDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	DestroyWindow();
}