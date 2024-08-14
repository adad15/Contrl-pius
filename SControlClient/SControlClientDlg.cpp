#include "pch.h"
#include "framework.h"
#include "SControlClient.h"
#include "SControlClientDlg.h"
#include "afxdialogex.h"
#include "ScreenWatch.h"
#include "ClientController.h"
#include "Tool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)

END_MESSAGE_MAP()

/* 创建工具栏 */
void CSControlClientDlg::CreateToolBar()
{
	//创建ToolBar工具条
	if (!m_toolbar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_toolbar.LoadToolBar(IDR_TOOLBAR_MAIN))
	{
		TRACE0("Failed to Create Dialog ToolBar");
		EndDialog(IDCANCEL);
	}
	//显示工具栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	//设置大小
	CRect re;
	m_toolbar.GetWindowRect(&re);
	re.top = 0;
	re.bottom = 80;
	m_toolbar.MoveWindow(&re);
	//创建图标列表  CImageList
	img.Create(45, 45, ILC_COLOR32 | ILC_MASK, 1, 1);//加载图片大小，图片格式，图片数量
	//加载图标
	HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_DIR), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon);
	HICON hIcon2 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SCREEN), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon2);
	HICON hIcon3 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_ADD), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon3);
	HICON hIcon4 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_MOD), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon4);
	HICON hIcon5 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_DEL), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon5);
	m_toolbar.GetToolBarCtrl().SetImageList(&img);
	//文字
	m_toolbar.SetButtonText(0, L"文件管理");
	m_toolbar.SetButtonText(2, L"屏幕监控");
	m_toolbar.SetButtonText(4, L"添加客户");
	m_toolbar.SetButtonText(6, L"修改客户");
	m_toolbar.SetButtonText(8, L"删除客户");
}

/* 初始化列表控件 */
void CSControlClientDlg::InitListBeControl()
{
	// 插入列标题
	m_ListBeControl.InsertColumn(0, _T("标识"), 0, 150);
	m_ListBeControl.InsertColumn(1, _T("用户名"), 0, 150);
	m_ListBeControl.InsertColumn(2, _T("操作系统"), 0, 150);
	m_ListBeControl.InsertColumn(3, _T("运行时间"), 0, 150);
	m_ListBeControl.InsertColumn(4, _T("IP"), 0, 150);
	m_ListBeControl.InsertColumn(5, _T("PORT"), 0, 150);

	// 设置扩展样式 
	DWORD extStyle = m_ListBeControl.GetExtendedStyle();
	extStyle |= LVS_EX_GRIDLINES;     // 网格线
	extStyle |= LVS_EX_FULLROWSELECT; // 选择整行
	m_ListBeControl.SetExtendedStyle(extStyle);

	// 插入初始项
	int i = 0;
	m_ListBeControl.InsertItem(i,	  _T("本机"));
	m_ListBeControl.SetItemText(i, 1, _T("Administrator"));
	m_ListBeControl.SetItemText(i, 2, _T("WIN10"));
	m_ListBeControl.SetItemText(i, 3, _T("10:59:43"));
	m_ListBeControl.SetItemText(i, 4, _T("127.0.0.1"));
	m_ListBeControl.SetItemText(i, 5, _T("6888"));
	i = 1;
	m_ListBeControl.InsertItem(i,     _T("被控机"));
	m_ListBeControl.SetItemText(i, 1, _T("Administrator"));
	m_ListBeControl.SetItemText(i, 2, _T("WIN10"));
	m_ListBeControl.SetItemText(i, 3, _T("10:59:43"));
	m_ListBeControl.SetItemText(i, 4, _T("192.168.1.104"));
	m_ListBeControl.SetItemText(i, 5, _T("6888"));
}

/* 下面列表控件初始化 */
void CSControlClientDlg::InitListInfo()
{
	// 创建图像列表，设置图像大小和格式
	img2.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);

	// 加载图标并添加到图像列表
	HICON hIcon3 = NULL;
	hIcon3 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_MSG), IMAGE_ICON, 64, 64, 0);
	img2.Add(hIcon3);

	// 设置列表控件的图像列表
	m_ListInfo.SetImageList(&img2, 1);

	// 插入列标题
	m_ListInfo.InsertColumn(0, _T(""), 0, 33);
	m_ListInfo.InsertColumn(1, _T("时间"), 0, 150);
	m_ListInfo.InsertColumn(2, _T("消息"), 0, 600);

	// 设置扩展样式
	DWORD extStyle = m_ListInfo.GetExtendedStyle();
	extStyle |= LVS_EX_GRIDLINES;
	extStyle |= LVS_EX_FULLROWSELECT;
	m_ListInfo.SetExtendedStyle(extStyle);
}

/* 用于在被控列表控件m_ListBeControl中添加一项新条目 */
void CSControlClientDlg::AddListBeControlItem(const CString& _m_alias, const CString& _m_username, const CString& _m_os,
	const CString& _m_runtime, const CString& _m_ip, const CString& _m_port)
{
	// 获取m_ListBeControl中当前项的数量，这个值用于确定新项的插入位置
	int i = m_ListBeControl.GetItemCount();
	// 在列表控件m_ListBeControl中的第i项插入_m_alias，即在列表的末尾添加一项，即标识列为_m_alias
	m_ListBeControl.InsertItem(i, _m_alias);
	// 为插入的新项设置子项文本
	m_ListBeControl.SetItemText(i, 1, _m_username);
	m_ListBeControl.SetItemText(i, 2, _m_os);
	m_ListBeControl.SetItemText(i, 3, _m_runtime);
	m_ListBeControl.SetItemText(i, 4, _m_ip);
	m_ListBeControl.SetItemText(i, 5, _m_port);
}

/* 修改被控列表控件m_ListBeControl中当前选中的项 */
void CSControlClientDlg::ModListBeControlItem(const CString& _m_alias, const CString& _m_username, const CString& _m_os,
	const CString& _m_runtime, const CString& _m_ip, const CString& _m_port)
{
	// 调用GetSelectionMark方法获取当前选中项的索引。
	int i = m_ListBeControl.GetSelectionMark();
	// 使用SetItemText方法修改当前选中项的各列文本
	m_ListBeControl.SetItemText(i, 0, _m_alias);
	m_ListBeControl.SetItemText(i, 1, _m_username);
	m_ListBeControl.SetItemText(i, 2, _m_os);
	m_ListBeControl.SetItemText(i, 3, _m_runtime);
	m_ListBeControl.SetItemText(i, 4, _m_ip);
	m_ListBeControl.SetItemText(i, 5, _m_port);
}

/* 响应添加用户的消息响应函数 */
void CSControlClientDlg::CmdAddUser()
{
	CUserInfoDlg userInfoDlg(this); // 创建添加用户界面对象
	userInfoDlg.SetData();  // 设置对话框数据
	userInfoDlg.DoModal();  // 显示对话框
}

/* 响应修改用户的消息响应函数 */
void CSControlClientDlg::CmdModUser()
{
	// 这段代码从m_ListBeControl列表控件中获取当前选中的项的索引，并将其存储在变量i中
	int i = m_ListBeControl.GetSelectionMark();
	if (i == -1) return;
	// 从m_ListBeControl列表控件中获取当前选中项的各个字段值，并将其存储在相应的CString变量中
	CString alias = m_ListBeControl.GetItemText(i, 0);
	CString username = m_ListBeControl.GetItemText(i, 1);
	CString os = m_ListBeControl.GetItemText(i, 2);
	CString runtime = m_ListBeControl.GetItemText(i, 3);
	CString ip = m_ListBeControl.GetItemText(i, 4);
	CString port = m_ListBeControl.GetItemText(i, 5);
	// 创建一个CUserInfoDlg类的对象，名为userInfoDlg，并将当前对话框CSControlClientDlg作为其父窗口
	CUserInfoDlg userInfoDlg(this);
	// 将获取到的用户信息传递给对话框进行初始化
	userInfoDlg.SetData(false, alias, username, os, runtime, ip, port);
	// 显示对话框
	userInfoDlg.DoModal();
}

/* 删除操作的消息处理函数 */
void CSControlClientDlg::CmdDelUser()
{
	int i = m_ListBeControl.GetSelectionMark();
	if (i == -1) return;
	m_ListBeControl.DeleteItem(i);
}

CSControlClientDlg::CSControlClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SCONTROLCLIENT_DIALOG, pParent)
	, m_udppass("192.168.1.100", 16888, 18888)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSControlClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_BECONTROL, m_ListBeControl);
	DDX_Control(pDX, IDC_LIST_INFO, m_ListInfo);
}

/* 消息映射表 */
BEGIN_MESSAGE_MAP(CSControlClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_T_ADDUSER, CSControlClientDlg::CmdAddUser)
	ON_COMMAND(ID_T_MODUSER, CSControlClientDlg::CmdModUser)
	ON_COMMAND(ID_T_DELUSER, CSControlClientDlg::CmdDelUser)
	ON_COMMAND(ID_T_FILEMANAGER, &CSControlClientDlg::CmdFileManager)
	ON_COMMAND(ID_T_SCREENWATCH, &CSControlClientDlg::CmdScreenWatch)
	ON_MESSAGE(WM_ONLINE, &CSControlClientDlg::OnOnLine)
	ON_BN_CLICKED(IDC_BUTTON1, &CSControlClientDlg::OnBnClickedButton1)//UDP连接
	ON_BN_CLICKED(IDC_BUTTON2, &CSControlClientDlg::OnBnClickedButton2)//UDP消息发送
END_MESSAGE_MAP()

// CSControlClientDlg 消息处理程序
BOOL CSControlClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CreateToolBar();      // 创建工具栏
	InitListBeControl();  // 创建上面的列表控件
	InitListInfo();		  // 创建下面的列表控件

	m_udppass.Invoke(GetSafeHwnd()); // 启动客户端的主要函数

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

/* 处理在线状态的消息，用于处理 Windows 消息的回调函数 */
LRESULT CSControlClientDlg::OnOnLine(WPARAM wParam, LPARAM lParam)
{
	// 清空了两个列表控件中的所有项
	m_ListBeControl.DeleteAllItems();
	m_ListInfo.DeleteAllItems();
	if (wParam == 1)
	{
		return LRESULT();
	}
	// 获取用户信息映射
	std::map<long long, MUserInfo> mapAddrs = m_udppass.GetMapAddrs();
	// 显示到(信息提示列表中)
	int count0 = m_ListInfo.GetItemCount();
	int count1 = m_ListBeControl.GetItemCount();
	int i = 0; // 初始化一个计数器 i，用于遍历 mapAddrs 映射
	// 遍历mapAddrs映射中的每个用户信息，并将这些信息插入到m_ListInfo列表控件中
	for (std::map<long long, MUserInfo>::iterator it = mapAddrs.begin();it != mapAddrs.end();it ++)
	{
		int idx = i + count0;
		m_ListInfo.InsertItem(LVIF_TEXT | LVIF_STATE, idx , L"", 0, LVIS_SELECTED, 0, 0);
		m_ListInfo.SetItemText(idx, 1, CTool::GetCurrentTime());
		MUserInfo& mInfo = it->second;
		WCHAR wideIp[32]{};
		MultiByteToWideChar(CP_ACP, 0, mInfo.ip, 16, wideIp, 32);
		CString ip = wideIp;
		ip += L" 已经上线";
		m_ListInfo.SetItemText(idx, 2, ip);

		i++;
	}
	i = 0;
	//清空ids
	m_ids.clear();

	//显示到(控制列表中)，从 mapAddrs 映射中获取用户信息，并将这些信息插入到 m_ListBeControl 列表控件中
	for (std::map<long long, MUserInfo>::iterator it = mapAddrs.begin(); it != mapAddrs.end(); it++)
	{	
		m_ids.push_back(it->first);
		MUserInfo& mInfo = it->second;
		WCHAR wideIp[32]{};
		MultiByteToWideChar(CP_ACP, 0, mInfo.ip, 16, wideIp, 32);

		CString port;
		port.Format(_T("%d"), mInfo.port);
		int idx = i + count1;
		m_ListBeControl.InsertItem(idx,     _T("___"));
		m_ListBeControl.SetItemText(idx, 1, _T("未知"));
		m_ListBeControl.SetItemText(idx, 2, _T("未知"));
		m_ListBeControl.SetItemText(idx, 3, _T("未知"));
		m_ListBeControl.SetItemText(idx, 4, wideIp);
		m_ListBeControl.SetItemText(idx, 5, port);

		i++;
	}
	return LRESULT();
}

/* 处理系统命令吗，响应系统消息WM_SYSCOMMAND（由窗口程序接收，以便处理各种系统命令） */
void CSControlClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// 这段代码检查 nID 是否与 IDM_ABOUTBOX 相匹配
	// 具体来说，nID & 0xFFF0 会屏蔽掉 nID 的低 4 位，以匹配 IDM_ABOUTBOX 命令
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		// 如果匹配，则创建一个 CAboutDlg 对话框对象 dlgAbout，
		// 并调用其 DoModal 方法，以模态对话框的形式显示关于对话框
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		// 如果 nID 不匹配 IDM_ABOUTBOX，则调用基类 CDialogEx 的 OnSysCommand 方法，继续处理其他系统命令
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

/* 用于处理窗口的绘制消息。当窗口处于最小化状态时，它会创建一个设备上下文，擦除窗口背景，
计算图标的居中位置，并在客户区绘制图标。如果窗口没有最小化，则调用基类的方法处理其他绘制操作。 */
void CSControlClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文
		// 发送 WM_ICONERASEBKGND 消息，以便窗口背景能够被正确地擦除。
		// 这里将设备上下文句柄（HDC）作为 WPARAM 参数传递
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		// 获取客户端区域的矩形
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CSControlClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/* 用于启动文件管理器对话框 */
void CSControlClientDlg::CmdFileManager()
{
	// 获取选中的列表项索引
	int i = m_ListBeControl.GetSelectionMark();
	if (i == -1) return;
	// 获取选中项的 IP 地址和端口号
	CString ip = m_ListBeControl.GetItemText(i, 4);
	CString port = m_ListBeControl.GetItemText(i, 5);
	// 更新 CClientController 中的用户信息
	if (CClientController::m_vecUserInfos.size() > 0)
	{
		CClientController::m_vecUserInfos.pop_back();
	}
	CClientController::m_vecUserInfos.push_back(USERINFO(ip, port));
	// 创建并显示文件管理器对话框
	fileManagerDlg = new CFileMangerDlg;
	// IDD_FILE_MANAGER 是文件管理器对话框的资源 ID
	fileManagerDlg->Create(IDD_FILE_MANAGER);
	fileManagerDlg->ShowWindow(SW_SHOW);
}

/* 用于启动屏幕监视对话框 */
void CSControlClientDlg::CmdScreenWatch()
{
	int i = m_ListBeControl.GetSelectionMark();
	if (i == -1) return;
	CString ip = m_ListBeControl.GetItemText(i, 4);
	CString port = m_ListBeControl.GetItemText(i, 5);
	if (CClientController::m_vecUserInfos.size() > 0)
	{
		CClientController::m_vecUserInfos.pop_back();
	}
	CClientController::m_vecUserInfos.push_back(USERINFO(ip, port));
	screenWatch = new CScreenWatch;
	screenWatch->Create(IDD_SCREEN_WATCH);
	screenWatch->ShowWindow(SW_SHOW);
}

/* 用于处理按钮点击事件，建立连接 */
void CSControlClientDlg::OnBnClickedButton1()
{
	int i = m_ListBeControl.GetSelectionMark();
	if (i == -1) return;
	if (m_ids.size() > 0) {
		// 请求与指定 ID 的用户建立连接
		m_udppass.RequestConnect(m_ids.at(i));
	}
}

/* 用于处理按钮点击事件，发送控制请求 */
void CSControlClientDlg::OnBnClickedButton2()
{
	m_udppass.SentToBeCtrl();
}