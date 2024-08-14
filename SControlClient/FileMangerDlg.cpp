#include "pch.h"
#include "SControlClient.h"
#include "FileMangerDlg.h"
#include "afxdialogex.h"
#include "ClientSocket.h"
#include "ClientController.h"

/*自定义消息，用来显示窗口*/
#define WM_SHOWDLDLG (WM_USER + 1)

// CFileMangerDlg 对话框
IMPLEMENT_DYNAMIC(CFileMangerDlg, CDialogEx)

CFileMangerDlg::CFileMangerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILE_MANAGER, pParent)
{
}

CFileMangerDlg::~CFileMangerDlg()
{
}

/*创建工具栏*/
void CFileMangerDlg::CreateToolBar()
{
	//创建ToolBar工具条，创建一个带有多种风格和属性的工具栏
	if (!m_toolbar.CreateEx(this, TBSTYLE_FLAT/*扁平样式*/, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_toolbar.LoadToolBar(IDR_TOOLBAR_FM))
	{
		TRACE0("Failed to Create Dialog ToolBar");
		EndDialog(IDCANCEL);
	}
	//重新定位工具栏，使其显示在窗口中
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	//获取工具栏、树视图和客户区的矩形区域，然后设置工具栏的位置和大小
	CRect re;
	CRect mTreeRect;
	CRect clientRect;
	m_toolbar.GetWindowRect(&re);
	m_Tree.GetWindowRect(&mTreeRect);
	GetWindowRect(&clientRect);
	re.top = 7;
	re.left = mTreeRect.Width() + 15;
	re.right = clientRect.Width() - 35;
	re.bottom = 60;
	m_toolbar.MoveWindow(&re);
	/*创建图标列表*/
	if (toolbarImg.m_hImageList == NULL)
	{
		//创建图标列表  CImageList
		toolbarImg.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);//加载图片大小，图片格式，图片数量
		//加载图标
		HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_SHW), IMAGE_ICON, 64, 64, 0);
		toolbarImg.Add(hIcon);
		HICON hIcon2 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_MOD), IMAGE_ICON, 64, 64, 0);
		toolbarImg.Add(hIcon2);
		HICON hIcon3 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_SET), IMAGE_ICON, 64, 64, 0);
		toolbarImg.Add(hIcon3);
		HICON hIcon4 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_DEL), IMAGE_ICON, 64, 64, 0);
		toolbarImg.Add(hIcon4);
		HICON hIcon5 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_DOW), IMAGE_ICON, 64, 64, 0);
		toolbarImg.Add(hIcon5);
	}
	m_toolbar.GetToolBarCtrl().SetImageList(&toolbarImg);
	//文字
	m_toolbar.SetButtonText(0, L"查看文件");
	m_toolbar.SetButtonText(2, L"修改文件");
	m_toolbar.SetButtonText(4, L"设置文件");
	m_toolbar.SetButtonText(6, L"删除文件");
	m_toolbar.SetButtonText(8, L"下载文件");
}

// 获取驱动信息并将其加载到树控件中的功能
void CFileMangerDlg::GetDriveInfo()
{
	/*1.创建套接字，和服务器建立连接，发送命令解析服务器发送的数据包*/
	CClientSocket clientSock;
	/*使用InitSocket方法初始化套接字，连接到指定的IP地址和端口。这些信息从CClientController类的m_vecUserInfos向量中获取*/
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	/*发送请求包*/
	CPacket pack(1);
	clientSock.Send(pack);
	/*处理服务器的响应*/
	int nCmd = clientSock.DealCommand();
	/*tcp短连接*/
	clientSock.CloseSocket();
	if (nCmd <= 0)
	{
		TRACE("接受驱动信息错误(错误码:%d 错误:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
		AfxMessageBox(L"接受驱动信息错误");
	}

	/*2.获取并解析驱动信息*/
	DRIVEINFO driveInfo;  // 定义一个DRIVEINFO结构体对象driveInfo来存储驱动信息
	memcpy(&driveInfo, clientSock.GetPacket().sData.c_str(), clientSock.GetPacket().sData.size());
	/*3.将驱动信息加载到树控件中*/
	for (int i = 0; i < driveInfo.drive_count; i++) // 遍历driveInfo中的所有驱动器
	{
		CString str;
		str.Format(L"%c:", driveInfo.drive[i]);
		HTREEITEM hTree = m_Tree.InsertItem(str, 0, 0, TVI_ROOT); // 将驱动器名称插入到树控件的根节点下
	}
}

/*初始化树控件,创建图标列表并将其与树控件关联*/
void CFileMangerDlg::InitMTree()
{
	treeImg.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);// 加载图片大小，图片格式，图片数量
	// 加载图标
	HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_DRIVE), IMAGE_ICON, 64, 64, 0);
	treeImg.Add(hIcon);
	HICON hIcon2 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_DIR), IMAGE_ICON, 64, 64, 0);
	treeImg.Add(hIcon2);
	/*设置图标列表到树控件*/
	m_Tree.SetImageList(&treeImg, TVSIL_NORMAL);
}

/*创建图标列表并将其与列表控件关联，以及设置列表控件的列和扩展样式*/
void CFileMangerDlg::InitMList()
{
	listImg.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);// 加载图片大小，图片格式，图片数量
	//加载图标
	HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_FILE), IMAGE_ICON, 64, 64, 0);
	listImg.Add(hIcon);
	m_List.SetImageList(&listImg, 1); // 设置图标列表到列表控件
	/*插入列到列表控件*/
	m_List.InsertColumn(0, _T("文件名称"), 0, 150);
	m_List.InsertColumn(1, _T("创建时间"), 0, 170);
	m_List.InsertColumn(2, _T("上次访问"), 0, 170);
	m_List.InsertColumn(3, _T("上次修改"), 0, 170);
	m_List.InsertColumn(4, _T("文件长度"), 0, 150);
	/*设置列表控件的扩展样式*/
	DWORD extStyle = m_List.GetExtendedStyle();
	extStyle |= LVS_EX_GRIDLINES;
	extStyle |= LVS_EX_FULLROWSELECT;
	m_List.SetExtendedStyle(extStyle);
}

/*将控件与成员变量关联起来*/
void CFileMangerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

/*消息映射表，将消息和消息响应函数绑定在一起*/
BEGIN_MESSAGE_MAP(CFileMangerDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CFileMangerDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CFileMangerDlg::OnNMDblclkTreeDir)
	ON_COMMAND(ID_T_DOWN, &CFileMangerDlg::CmdDownLoadFile)
	ON_MESSAGE(WM_SHOWDLDLG/*自定义的消息*/, OnShowDLDlg)
	ON_COMMAND(ID_T_DEL, &CFileMangerDlg::CmdDeleteFile)
END_MESSAGE_MAP()

// CFileMangerDlg 消息处理程序
BOOL CFileMangerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CreateToolBar();  //创建工具栏
	InitMTree();      //初始化树控件
	InitMList();	  //初始化列表控件

	GetDriveInfo();   //获取驱动器信息

	// TODO:  在此添加额外的初始化
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

/*获取当前树项目路径*/
void CFileMangerDlg::GetCurTreeItemPath(CString& path)
{
	/*获取当前选中的树项*/
	HTREEITEM hTree = m_Tree.GetSelectedItem();
	/*通过这个循环，代码从当前选中的树项开始，一直向上遍历，直到树的根节点，构建出完整的路径。
	最终，path 中包含了从根节点到当前选中项的完整路径*/
	do
	{
		/*将当前项的文本加到路径前面*/
		path = m_Tree.GetItemText(hTree) + "\\" + path;
		TRACE("%S\r\n", path.GetBuffer());
	} while ((hTree = m_Tree.GetParentItem(hTree)) != NULL);
}

/*删除当前树项目的子项*/
void CFileMangerDlg::DelCurTreeItemChildItem(HTREEITEM hTree)
{
	HTREEITEM hSubTree;
	/*代码反复获取并删除指定树项的第一个子项，直到该树项不再有子项为止*/
	while ((hSubTree = m_Tree.GetChildItem(hTree)) != NULL)
	{
		m_Tree.DeleteItem(hSubTree);
	}
}

/*将时间转换为CString对象*/
void CFileMangerDlg::Time2CString(CString& strTime, __time64_t time)
{
	if (time == -1) {
		strTime = _T("??-??-??:??:??:??");
	}
	else {
		char str[0xFF]{};
		tm t;                      // 定义一个 tm 结构体用于存储分解的时间
		localtime_s(&t, &time);	   // 将时间转换为本地时间，并存储在 tm 结构体中
		int ret = strftime(str, 0xff, "%Y-%m-%d:%H:%M:%S", &t);  // 将时间格式化为指定格式的字符串，存储在 str 数组中
		TCHAR wideTime[MAX_PATH]{};                              // 定义一个宽字符数组用于存储转换后的宽字符时间字符串
		/*将多字节字符串转换为宽字符字符串*/
		int transRet = MultiByteToWideChar(CP_ACP, 0, str, strlen(str), wideTime, MAX_PATH);
		strTime = wideTime;
	}
}

/*启动文件下载线程*/
void CFileMangerDlg::ThreadEntryDownLoadFile(void* arg)
{
	CFileMangerDlg* thiz = (CFileMangerDlg*)arg;
	thiz->ThreadDownLoadFile();
	_endthread();
}

/*从服务器下载文件并保存到本地的功能*/
void CFileMangerDlg::ThreadDownLoadFile()
{
	// 获取要下载的文件路径
	CString path;
	GetCurTreeItemPath(path);// 获取当前选中的树项的路径
	CString name = m_List.GetItemText(m_List.GetSelectionMark(), 0); // 获取列表中选中文件的名称
	char multiPath[MAX_PATH]{};
	char multiName[MAX_PATH]{};
	/*将路径转换为多字节字符串*/
	int transRet = WideCharToMultiByte(CP_ACP, 0, path.GetBuffer(), -1, multiPath, MAX_PATH, NULL, NULL);
	/*将文件名转换为多字节字符串*/
	transRet = WideCharToMultiByte(CP_ACP, 0, name.GetBuffer(), -1, multiName, MAX_PATH, NULL, NULL);
	/*拼接路径和文件名*/
	strcat(multiPath, multiName);
	
	/*打开保存文件对话框，获取保存路径*/
	CFileDialog fileDlg(false, 0, name);
	if (fileDlg.DoModal() != IDOK)
	{
		AfxMessageBox(L"未选择要保存");
		return;
	}
	char multiSavePath[MAX_PATH]{};
	transRet = WideCharToMultiByte(CP_ACP, 0, fileDlg.GetPathName().GetBuffer(), -1, multiSavePath, MAX_PATH, NULL, NULL);
	/*以二进制写入方式打开文件*/
	FILE* pFile = fopen(multiSavePath, "wb+");
	if (pFile == NULL)
	{
		AfxMessageBox(L"文件无法创建，可能没有权限");
		return;
	}

	/*发送自定义的消息，打开文件下载进度界面*/
	SendMessage(WM_SHOWDLDLG);

	/*设置下载信息*/
	CString info;
	info.Format(L"[开始下载...]\r\n[从 %s 下载]\r\n[到 %s 保存]", (path + name).GetBuffer(), fileDlg.GetPathName().GetBuffer());
	downLoadFileDlg.SetInfo(info);

	//告诉服务器我要下载文件
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	CPacket pack(3, (BYTE*)multiPath, strlen(multiPath));
	clientSock.Send(pack);

	/*第一步：接收文件长度信息*/
	int nCmd = clientSock.DealCommand();  // 将服务器发送来的数据收集，并解析一个数据包
	if (nCmd <= 0)
	{
		TRACE("下载文件错误(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
		AfxMessageBox(L"下载文件错误");
		return;
	}
	long long fileLen = *(long long*)clientSock.GetPacket().sData.c_str(); // 获取文件长度
	if (fileLen <= 0)
	{
		TRACE("文件长度为零，或者没有权限(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
		AfxMessageBox(L"文件长度为零，或者没有权限");
		return;
	}
	downLoadFileDlg.SetFileLength(fileLen);	// 设置进度对话框中的文件长度

	/*第二步：接收文件内容*/
	long long readedLen = 0;
	while (readedLen < fileLen)
	{
		nCmd = clientSock.DealCommand();    // 将服务器发送来的数据收集，并解析一个数据包
		int readLen = clientSock.GetPacket().sData.size();  //解析好的数据包的大小
		if (nCmd <= 0)
		{
			TRACE("下载文件错误(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"下载文件错误");
			return;
		}
		int writeLen = fwrite(clientSock.GetPacket().sData.c_str(), 1, readLen, pFile);
		if (writeLen <= 0)
		{
			TRACE("文件写入错误(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"文件写入错误");
			return;
		}
		readedLen += readLen;
		downLoadFileDlg.SetDownLoadedLen(readedLen); // 更新进度对话框中的已下载长度
	}
	/*关闭套接字和文件*/
	clientSock.CloseSocket();
	fclose(pFile);
}

/*树控件单击事件*/
void CFileMangerDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
}

/*树控件双击事件*/
void CFileMangerDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	/*获取当前选中的树项路径*/
	CString path;
	GetCurTreeItemPath(path);
	char multiPath[MAX_PATH]{};
	/*将选中的树的路径转换成多字节字符串*/
	int transRet = WideCharToMultiByte(CP_ACP, 0, path.GetBuffer(), -1, multiPath, MAX_PATH, NULL, NULL);
	
	/*和服务器交互，请求目录内容*/
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	CPacket pack(2, (BYTE*)multiPath, strlen(multiPath));
	clientSock.Send(pack);

	/*处理服务器的响应*/
	int nCmd = clientSock.DealCommand();  // 将服务器发送来的数据收集，并解析一个数据包
	PFILEINFO pFileInfo = (PFILEINFO)clientSock.GetPacket().sData.c_str();
	HTREEITEM hTree = m_Tree.GetSelectedItem(); // 获取当前选中的树项
	DelCurTreeItemChildItem(hTree);             // 删除当前树项的所有子项
	m_List.DeleteAllItems();                    // 删除列表控件中的所有项目
	/*处理文件信息，直至文件为空*/
	while (!pFileInfo->isNull)
	{
		if (nCmd <= 0)
		{
			TRACE("接受文件信息错误(错误码:%d 错误:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"接受文件信息错误");
			return;
		}
		TRACE("nCmd = %d name = %s\r\n", nCmd, pFileInfo->data.name);
		TCHAR str[MAX_PATH]{};
		int transRet = MultiByteToWideChar(CP_ACP, 0, pFileInfo->data.name, strlen(pFileInfo->data.name), str, MAX_PATH);
		if ((strcmp(pFileInfo->data.name, ".") == 0) || (strcmp(pFileInfo->data.name, "..") == 0))
		{
		}
		else
		{
			/*如果是子目录，插入到树控件中*/
			if (pFileInfo->data.attrib & _A_SUBDIR)
			{
				m_Tree.InsertItem(str, 1, 1, hTree);
			}
			else
			{
				/*如果是文件，插入到列表控件中*/
				int index = m_List.GetItemCount();
				CString strTime;
				m_List.InsertItem(index, str, 0);
				/*插入创建时间*/
				Time2CString(strTime, pFileInfo->data.time_create);
				m_List.SetItemText(index, 1, strTime);
				/*插入访问时间*/
				Time2CString(strTime, pFileInfo->data.time_access);
				m_List.SetItemText(index, 2, strTime);
				/*插入修改时间*/
				Time2CString(strTime, pFileInfo->data.time_write);
				m_List.SetItemText(index, 3, strTime);
				/*插入文件大小*/
				if (pFileInfo->data.size < 1024)
				{
					strTime.Format(L"%dB", pFileInfo->data.size);
				}
				else
				{
					strTime.Format(L"%dK", (int)(1.0 * pFileInfo->data.size / 1024));
				}
				m_List.SetItemText(index, 4, strTime);
			}
		}
		/*处理下一个文件信息，进入循环*/
		nCmd = clientSock.DealCommand();
		pFileInfo = (PFILEINFO)clientSock.GetPacket().sData.c_str();
	}
	/*短连接关闭套接字*/
	clientSock.CloseSocket();
}

/*点击下载按钮，开启文件下载线程*/
void CFileMangerDlg::CmdDownLoadFile()
{
	if (m_Tree.GetSelectedItem() < 0 || m_List.GetSelectionMark() < 0) return;
	_beginthread(ThreadEntryDownLoadFile, 0, this);
}

/*接收自定义的消息，创建文件下载窗口*/
LRESULT CFileMangerDlg::OnShowDLDlg(WPARAM wParam, LPARAM lParam)
{
	/*创建下载文件状态对话框*/
	downLoadFileDlg.Create(IDD_DLFILE_STATUS);
	/*显示下载文件状态对话框*/
	downLoadFileDlg.ShowWindow(SW_SHOW);
	/*设置下载文件状态对话框为活动窗口*/
	downLoadFileDlg.SetActiveWindow();
	/*将下载文件状态对话框居中显示*/
	downLoadFileDlg.CenterWindow();

	return 0;
}

/*删除文件*/
void CFileMangerDlg::CmdDeleteFile()
{
	/*检查是否有选中的树项或列表项*/
	if (m_Tree.GetSelectedItem() < 0 || m_List.GetSelectionMark() < 0) return;
	/*获取要删除的文件路径和名称*/
	CString path;
	GetCurTreeItemPath(path);                                          // 获取当前选中的树项的路径
	CString name = m_List.GetItemText(m_List.GetSelectionMark(), 0);   // 获取列表中选中文件的名称
	char multiPath[MAX_PATH]{};
	char multiName[MAX_PATH]{};
	/*将路径和文件名从宽字符转换为多字节字符*/
	int transRet = WideCharToMultiByte(CP_ACP, 0, path.GetBuffer(), -1, multiPath, MAX_PATH, NULL, NULL);
	transRet = WideCharToMultiByte(CP_ACP, 0, name.GetBuffer(), -1, multiName, MAX_PATH, NULL, NULL);
	/*拼接路径和文件名*/
	strcat(multiPath, multiName);

	/*和服务器交互，删除文件*/
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	CPacket pack(4, (BYTE*)multiPath, strlen(multiPath));
	clientSock.Send(pack);
	/*处理服务器的响应*/
	int nCmd = clientSock.DealCommand();
	if (nCmd <= 0)
	{
		TRACE("删除文件错误(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
		AfxMessageBox(L"删除文件错误");
		return;
	}
	int success = *(int*)clientSock.GetPacket().sData.c_str();
	if (success)
	{
		m_List.DeleteItem(m_List.GetSelectionMark());
	}
}

/*取消对话框*/
void CFileMangerDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	CDialogEx::OnCancel();
}