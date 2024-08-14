#include "pch.h"
#include "SControlClient.h"
#include "ScreenWatch.h"
#include "afxdialogex.h"
#include "ClientSocket.h"
#include "ClientController.h"
#include "Request.h"
#include "MThread.h"

IMPLEMENT_DYNAMIC(CScreenWatch, CDialogEx)

/* 创建工具栏 */
void CScreenWatch::CreateToolBar()
{
	CMToolbar tol;
	// tol.Create(this, IDR_TOOLBAR2);
	// 创建工具栏，设置工具栏的样式和位置 -> LoadToolBar方法加载工具栏资源
	if (!m_toolbar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_toolbar.LoadToolBar(IDR_TOOLBAR_SW))
		//IDR_TOOLBAR2 工具条资源id
	{
		TRACE0("Failed to Create Dialog ToolBar");
		EndDialog(IDCANCEL);
	}
	// 显示工具栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	// 设置工具栏位置和大小
	CRect re;
	m_toolbar.GetWindowRect(&re); // 获取工具栏的矩形区域
	re.left = 370;
	re.top = 7;
	re.bottom = 60;
	re.right -= 35;
	m_toolbar.MoveWindow(&re);    // 设置工具栏的新位置
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
	// 将图标列表设置到工具栏控件中
	m_toolbar.GetToolBarCtrl().SetImageList(&img);
	// 设置按钮信息，可选中
	m_toolbar.SetButtonInfo(0/*索引值0表示是工具栏中的第一个按钮*/, ID_T_CMOUSE/*点击这个按钮时，将发送一个ID_T_CMOUSE命令*/, TBBS_CHECKBOX, 0/*索引值0表示这个按钮使用图标列表中的第一个图标*/);
	m_toolbar.SetButtonInfo(2/*索引值2表示是工具栏中的第二个按钮*/, ID_T_CKEY, TBBS_CHECKBOX, 1);
	//文字
	m_toolbar.SetButtonText(0, L"鼠标控制");
	m_toolbar.SetButtonText(2, L"键盘控制");
	m_toolbar.SetButtonText(4, L"锁定用户");
	m_toolbar.SetButtonText(6, L"解锁用户");
}

/* 线程入口函数 */
void CScreenWatch::ThreadEntryScreenWatch(void* arg)
{
	CScreenWatch* thiz = (CScreenWatch*)arg;
	thiz->ThreadScreenWatch();
	_endthread();
}

/* 线程函数 */
void CScreenWatch::ThreadScreenWatch()
{
	threadIsRunning = true;
	while (threadIsRunning)
	{
		//告诉服务器我要屏幕截图
		CClientSocket clientSock;
		clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
		clientSock.SetBufferSize(1024 * 1024 * 10);
		CPacket pack(5);
		clientSock.Send(pack);
		//解析数据包得到截图
		TRACE("1=======================tick = %lld\r\n", GetTickCount64());
		int nCmd = clientSock.DealCommand();
		TRACE("2=======================tick = %lld\r\n", GetTickCount64());
		if (nCmd <= 0)
		{
			TRACE("获取屏幕截图错误(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"获取屏幕截图错误");
			return;
		}
		if (showOver == true)
		{
			//截图数据写入pStream
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0); // 分配全局内存
			if (hMem == NULL) return;
			IStream* pStream = NULL;					  // 创建内存流
			HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream); // 将内存流和全局内存绑定
			ULONG written;
			// 将收到的截图数据写入pStream
			pStream->Write(clientSock.GetPacket().sData.c_str(), clientSock.GetPacket().sData.size(), &written);
			// 加载成图片
			image.Load(pStream);
			// 释放内存流和全局内存
			pStream->Release();
			GlobalFree(hMem);
			showOver = false;
			// 记录日志，输出当前线程 ID 和命令
			TRACE("[threadId: %d] 成功读取一张照片 nCmd = %d\r\n", GetThreadId(GetCurrentThread()), nCmd);
		}
		clientSock.CloseSocket();
	}
}

/* 将客户端坐标转换为全局坐标 */
CPoint CScreenWatch::ClientPt2GlobalPt(CPoint& clientPt)
{
	// 获取图片控件在屏幕上的位置和大小，并将其存储在picRect变量中
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);

	clientPt.x = 1.0 * clientPt.x * imageWidth / picRect.Width();
	clientPt.y = 1.0 * clientPt.y * imageHeight / picRect.Height();
	return clientPt;
}

// 构造函数
CScreenWatch::CScreenWatch(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SCREEN_WATCH, pParent)
{
	showOver = true;
	isControlMouse = false;
	isControlKey = false;
	imageWidth = 0;
	imageHeight = 0;
}

// 析构函数
CScreenWatch::~CScreenWatch()
{
}

void CScreenWatch::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_PIC_SCREEN, m_picScreen);
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
}

// 消息列表
BEGIN_MESSAGE_MAP(CScreenWatch, CDialogEx)
	ON_WM_TIMER()
	ON_COMMAND(ID_T_CMOUSE, &CScreenWatch::CmdControlMouse)
	ON_COMMAND(ID_T_CKEY, &CScreenWatch::CmdControlKey)
	ON_COMMAND(ID_T_LOCKM, &CScreenWatch::CmdLockMachine)
	ON_COMMAND(ID_T_UNLOCKM, &CScreenWatch::CmdUnLockMachine)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOVE()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON1, &CScreenWatch::OnBnClickedButton1)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CScreenWatch消息响应函数

// 初始化对话框
BOOL CScreenWatch::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  在此添加额外的初始化

	CreateToolBar();  // 创建工具栏
	// 启动一个新线程，参数为 this（当前对象指针）。hThread保存线程句柄。
	hThread = (HANDLE)_beginthread(ThreadEntryScreenWatch, 0, this);
	SetTimer(123, 70, NULL); // 设置定时器

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

/* 用于处理定时器事件，当定时器触发时，函数检查定时器 ID 和相关条件，如果满足条件，则执行屏幕截图的相关操作 */
void CScreenWatch::OnTimer(UINT_PTR nIDEvent)
{
	if ((nIDEvent == 123) && (showOver == false))
	{
		// 获取和设置图片宽度和高度
		if (imageWidth == 0)
		{
			imageWidth = image.GetWidth();
		}
		if (imageHeight == 0)
		{
			imageHeight = image.GetHeight();
		}
		// 获取图片控件的窗口矩形
		CRect picRect;
		m_picScreen.GetWindowRect(&picRect);
		// 创建显示矩形
		CRect showPicRect(0, 0, picRect.Width(), picRect.Height());
		// 绘制和保存图片
		image.StretchBlt(m_picScreen.GetDC()->GetSafeHdc(), showPicRect, SRCCOPY);
		image.Save(L"D:\\Desktop\\1.png", Gdiplus::ImageFormatPNG);
		image.Destroy();
		showOver = true;
	}
	if (nIDEvent == 123)
	{
	}
	// 调用基类CDialogEx的OnTimer方法，处理其他定时器事件
	CDialogEx::OnTimer(nIDEvent);
}

// 取消操作
void CScreenWatch::OnCancel()
{
	threadIsRunning = false;
	WaitForSingleObject(hThread, 500);
	delete this;
}

// 鼠标和键盘控制命令处理
void CScreenWatch::CmdControlMouse()
{
	isControlMouse = !isControlMouse;
}
void CScreenWatch::CmdControlKey()
{
	isControlKey = !isControlKey;
}

// 锁定和解锁机器 
void CScreenWatch::CmdLockMachine()
{
	//发送给被控端
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	clientSock.SetBufferSize(1024 * 1024 * 2);
	CPacket pack(7);
	clientSock.Send(pack);
	int ret = clientSock.DealCommand();
	clientSock.CloseSocket();
}
void CScreenWatch::CmdUnLockMachine()
{
	//发送给被控端
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	clientSock.SetBufferSize(1024 * 1024 * 2);
	CPacket pack(8);
	clientSock.Send(pack);
	int ret = clientSock.DealCommand();
	clientSock.CloseSocket();
}

// 左键双击，该函数主要功能是将双击事件的信息转换为全局坐标，并通过网络发送给远程端
void CScreenWatch::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	// 把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	// 将point的Y坐标减去picRect的顶部坐标，调整Y坐标到控件的原点
	point.y -= picRect.top;
	if (point.y < 0) return;
	// 组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::LEFT;
	mouseInfo.nEvent = MOUSEEVE::DBCLICK;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 左键将在(%d,%d)处双击\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	
	// 将鼠标信息发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));
	//***
	CDialogEx::OnLButtonDblClk(nFlags, point);
}

//左键按下
void CScreenWatch::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::LEFT;
	mouseInfo.nEvent = MOUSEEVE::DOWN;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 左键将在(%d,%d)处按下\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	
	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	//***
	CDialogEx::OnLButtonDown(nFlags, point);
}

//左键弹起
void CScreenWatch::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::LEFT;
	mouseInfo.nEvent = MOUSEEVE::UP;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 左键将在(%d,%d)处弹起\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	
	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	//***
	CDialogEx::OnLButtonUp(nFlags, point);
}

//右键双击
void CScreenWatch::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::RIGHT;
	mouseInfo.nEvent = MOUSEEVE::DBCLICK;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 右键将在(%d,%d)处双击\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	
	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	//***
	CDialogEx::OnRButtonDblClk(nFlags, point);
}

//右键按下
void CScreenWatch::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::RIGHT;
	mouseInfo.nEvent = MOUSEEVE::DOWN;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 右键将在(%d,%d)处按下\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	//***
	CDialogEx::OnRButtonDown(nFlags, point);
}

//右键弹起
void CScreenWatch::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::RIGHT;
	mouseInfo.nEvent = MOUSEEVE::UP;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 右键将在(%d,%d)处弹起\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	
	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	//***
	CDialogEx::OnRButtonUp(nFlags, point);
}

/* 窗口移动 */
void CScreenWatch::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
}

/* 鼠标移动 */
void CScreenWatch::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::NOTHING;
	mouseInfo.nEvent = MOUSEEVE::MOVE;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 鼠标将要移动到(%d,%d)\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	//发送给被控端
	req.SendPacket( 6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	TRACE("OnMouseMove %lld\r\n", GetTickCount64());

	//***
	CDialogEx::OnMouseMove(nFlags, point);
}

//**********************************************************************
//
// Sends Win + D to toggle to the desktop
//
//**********************************************************************
void CScreenWatch::ShowDesktop(DWORD code)
{
	// 这段代码将客户端坐标(1047, 30)转换为屏幕坐标，并存储在p变量中
	CPoint p(1047, 30);
	ClientToScreen(&p);

	// 初始化鼠标输入事件结构，模拟鼠标左键的按下和松开事件
	INPUT inputs1[2] = {};
	ZeroMemory(inputs1, sizeof(inputs1));

	inputs1[0].type = INPUT_MOUSE;
	inputs1[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	inputs1[0].mi.dwExtraInfo = GetMessageExtraInfo();

	inputs1[1].type = INPUT_MOUSE;
	inputs1[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
	inputs1[1].mi.dwExtraInfo = GetMessageExtraInfo();

	// 初始化键盘输入事件结构
	UINT uSent;
	INPUT inputs[2] = {};

	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki.wVk = code;

	inputs[1].type = INPUT_KEYBOARD;
	inputs[1].ki.wVk = code/*VK_SCROLL*/;
	inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

	// 发送输入事件
	int nSize = ARRAYSIZE(inputs);
	uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
	if (uSent != ARRAYSIZE(inputs)) {}
}

/* 按钮1点击响应函数 */
void CScreenWatch::OnBnClickedButton1()
{
	// 循环发送鼠标事件
	for (int i = 0; i < 1000; i++)
	{
		// 在循环的每一次迭代中，都会创建一个MOUSEINFO结构体，用于存储鼠标事件的信息
		MOUSEINFO mouseInfo;
		mouseInfo.nButton = MOUSEBTN::RIGHT;
		mouseInfo.nEvent = MOUSEEVE::UP;
		// 设置鼠标位置，每次循环时 Y 坐标增加 i
		mouseInfo.ptXY = CPoint(100,100 + i);
		req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));
	}
}

/* 键盘按下 */
void CScreenWatch::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialogEx::OnKeyDown(nChar, nRepCnt, nFlags);
}

/* 消息预翻译 */
BOOL CScreenWatch::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}