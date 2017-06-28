
// DnsCacheDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DnsCache.h"
#include "DnsCacheDlg.h"
#include "afxdialogex.h"
#include "Public.h"
#include "DnsParser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDnsCacheDlg 对话框
CDbSQLite	CDnsCacheDlg::m_SqlDb;
CFlag		CDnsCacheDlg::m_ThrTranFlag;
CFlag		CDnsCacheDlg::m_ThrSaveFlag;
CFlag		CDnsCacheDlg::m_ThrUpdateFlag;
CDnsData	CDnsCacheDlg::m_DnsData;
CFlag		CDnsCacheDlg::m_OpFlag;

CDnsCacheDlg::CDnsCacheDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDnsCacheDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_LOGO);//IDR_MAINFRAME

	m_localDnsSock			= 0;
	m_dwDnsIp				= 0;
	m_strNicAdapterName		= "";
	m_hTransmitDnsThread	= NULL;
	m_hSaveDnsThread		= NULL;
	m_hUpdateDnsThread		= NULL;
	m_bAppQuit				= FALSE;
	ZeroMemory(&m_TranThrParam, sizeof(m_TranThrParam));
}

CDnsCacheDlg::~CDnsCacheDlg()
{
}

void CDnsCacheDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DOMAIN, m_ListCtrlDomain);
	//  DDX_Control(pDX, IDC_COMBO_NIC, m_ComboNicName);
	//  DDX_Control(pDX, IDC_TREE1, m_TreeCtrlIp);
	DDX_Control(pDX, IDC_CHK_AUTORUN, m_ChkAutoRun);
}

BEGIN_MESSAGE_MAP(CDnsCacheDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_START, &CDnsCacheDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CDnsCacheDlg::OnBnClickedBtnStop)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_NOTIFYICON, &CDnsCacheDlg::OnNotifyIcon)
	ON_MESSAGE(WM_UPDATETOUI, &CDnsCacheDlg::OnUpdateToUi)
	ON_MESSAGE(WM_UPDATESTATUS, &CDnsCacheDlg::OnUpdateStatus)
ON_WM_CREATE()
ON_COMMAND(IDM_SYSTRAY_QUIT, &CDnsCacheDlg::OnSystrayQuit)
ON_WM_TIMER()
ON_COMMAND(IDM_SYSTRAY_ABOUT, &CDnsCacheDlg::OnSystrayAbout)
ON_WM_SIZE()
ON_WM_GETMINMAXINFO()
ON_COMMAND(IDM_SYSTRAY_START, &CDnsCacheDlg::OnSystrayStart)
ON_COMMAND(IDM_SYSTRAY_STOP, &CDnsCacheDlg::OnSystrayStop)
ON_COMMAND(IDM_SYSTRAY_OPTIONS, &CDnsCacheDlg::OnSystrayOptions)
ON_BN_CLICKED(IDC_CHK_AUTORUN, &CDnsCacheDlg::OnBnClickedChkAutorun)
END_MESSAGE_MAP()


// CDnsCacheDlg 消息处理程序

BOOL CDnsCacheDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	
	//m_menu.LoadMenu(IDR_MENU_MAIN);
	//SetMenu(&m_menu);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//设置大小控制
	m_Resize.SetOwner(this);
	m_Resize.SetResize(IDC_LIST_DOMAIN, PK_TOP_LEFT, PK_BOTTOM_RIGHT);
	m_Resize.SetResize(IDC_STATIC_VISIT, PK_BOTTOM_LEFT, PK_BOTTOM_LEFT);
	m_Resize.SetResize(IDC_STATIC_SQL, PK_BOTTOM_LEFT, PK_BOTTOM_LEFT);
	m_Resize.SetResize(IDC_STATIC_DNS, PK_BOTTOM_LEFT, PK_BOTTOM_LEFT);

	//初始化托盘图标
	InitSystrayIcon();

	m_SystrayMenu.LoadMenu(IDR_MENU_SYSTRAY);

	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_STOP, MF_DISABLED|MF_BYCOMMAND);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);

	//获取网卡信息
	RefreshNicInfo();

	//设置ListCtrl控件属性
	InitListCtrl();

	//延时处理
	//SetTimer(23, 1000, NULL);
	
	//修改样式
	//m_TreeCtrlIp.ModifyStyle(NULL, TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT);
	//不改变窗口大小
	//AfxGetMainWnd()->ModifyStyle(WS_MAXIMIZE|WS_MAXIMIZEBOX|WS_THICKFRAME, 0);
	//AfxGetMainWnd()->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME); 

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDnsCacheDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDnsCacheDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDnsCacheDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDnsCacheDlg::OnBnClickedBtnStart()
{
	// TODO: Add your control notification handler code here	
	static BOOL bFirst = TRUE;

	//前提条件
	if (m_nicCtrl.IsEmpty())
	{
		if (!RefreshNicInfo())
		{
			MessageBox(_T("计算机不存在可用的网卡，请重试！"));
		}
		return;
	}

	//线程退出标志
	m_ThrTranFlag.SetFlag(TRUE);
	m_ThrSaveFlag.SetFlag(TRUE);
	m_ThrUpdateFlag.SetFlag(TRUE);

	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_START, MF_DISABLED|MF_BYCOMMAND);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_STOP, MF_DISABLED|MF_BYCOMMAND);

	if (!InitSocket())
	{
		goto deal_failed;
	}
	//打开数据库
	if (!InitSql())
	{
		goto deal_failed;
	}

	//首次启动加载数据
	if (bFirst)
	{
		bFirst = FALSE;
		LoadSqlToMem();
	}

	//创建线程
	if (!InitThread())
	{
		goto deal_failed;
	}

	//设置dns
	//最好只设置"127.0.0.1"一个地址，由于某些浏览器或应用会跳过第一个"127.0.0.1"的地址，而使用备用dns
	if (!m_nicCtrl.SetAllNicDnsAddr(_T("127.0.0.1"), _T("")))
	//if (!m_nicCtrl.SetAllNicDnsAddr(_T("192.168.1.66"), _T("")))
	{
		goto deal_failed;
	}

	//开启定时器
	SetTimer(22, 1000, NULL);

	ShowSystrayMsg(_T("DNS优化工具运行中"), _T("提示"), _T("DNS优化工具开始运行！"));

	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_START, MF_DISABLED|MF_BYCOMMAND);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_STOP, MF_ENABLED|MF_BYCOMMAND);
	
	return;

deal_failed:
	GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_START, MF_ENABLED|MF_BYCOMMAND);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_STOP, MF_DISABLED|MF_BYCOMMAND);

	UnInitThread();
	UnInitSql();
	
	//退出后，为电脑本地设置以下dns
	m_nicCtrl.SetAllNicDnsAddr(_T("8.8.8.8"), _T("8.8.4.4"));

	MessageBox(_T("操作失败，请重试!"));
}


void CDnsCacheDlg::OnBnClickedBtnStop()
{
	// TODO: Add your control notification handler code here
	m_OpFlag.SetFlag(FALSE);
	
	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_START, MF_DISABLED|MF_BYCOMMAND);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_STOP, MF_DISABLED|MF_BYCOMMAND);

	//退出后，为电脑本地设置以下dns
	m_nicCtrl.SetAllNicDnsAddr(_T("8.8.8.8"), _T("8.8.4.4"));
	UnInitThread();
	UnInitSql();

	//关闭定时器
	KillTimer(22);

	//修改托盘
	m_notifyData.hIcon = AfxGetApp()->LoadIcon(IDI_ICON_LOGO);
	ShowSystrayMsg(_T("DNS优化工具 v1.0"), _T("提示"), _T("DNS优化工具停止运行！"));

	//使能按钮
	GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_START, MF_ENABLED|MF_BYCOMMAND);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_STOP, MF_DISABLED|MF_BYCOMMAND);

	m_OpFlag.SetFlag(TRUE);
}

BOOL CDnsCacheDlg::InitSocket(void)
{
	int err;

	m_localDnsSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == m_localDnsSock)
	{
		return FALSE;
	}

	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(53);
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	
	BOOL optval = TRUE;
	setsockopt(m_localDnsSock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(BOOL));

	//设置超时时间
	int timeout = 1000;//1s
	setsockopt(m_localDnsSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	timeout = 1000;
	setsockopt(m_localDnsSock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));

	err = bind(m_localDnsSock, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR));
	if (SOCKET_ERROR == err)
	{
		goto run_err;
	}

	return TRUE;

run_err:
	UnInitSocket();
	return FALSE;
}

void CDnsCacheDlg::UnInitSocket()
{
	if (m_localDnsSock)
	{
		closesocket(m_localDnsSock);
		m_localDnsSock = 0;
	}
}

BOOL CDnsCacheDlg::InitSql()
{
	CSqlStatement *stmt = NULL;
	int result = 0;
	bool ok = false;
	CString strDbPath;

	strDbPath.Format(_T("%s\\%s"), CPublic::GetAppCurrentDirectory(), SQLITE_DNS_DB);

	//打开数据库
	if (!m_SqlDb.Open(strDbPath))
	{
		return FALSE;
	}

	//判断是否存在dns_table表
	stmt = m_SqlDb.Statement(_T("select count(*) from sqlite_master where type='table' and name='dns_table'"));
	if (!stmt->NextRow())
	{
		delete stmt;
		m_SqlDb.Close();
		return FALSE;
	}
	//当不存在该表的情况下才创建表
	result = stmt->ValueInt(0);//得到该表的数量，若为1，则存在该表
	if (0 == result)
	{
		ok = m_SqlDb.DirectStatement(_T("CREATE TABLE dns_table(domain TEXT primary key, reqhex TEXT, reshex TEXT, ipvec TEXT, visit INTEGER, ttl INTEGER, lastvisittime INTEGER, lastupdatetime INTEGER, visittime TEXT, updatetime TEXT)"));
		if (!ok)
		{
			delete stmt;
			m_SqlDb.Close();
			return FALSE;
		}
	}
	delete stmt;
	return TRUE;
}

void CDnsCacheDlg::UnInitSql()
{
	m_SqlDb.Close();
}

BOOL CDnsCacheDlg::InitThread()
{
	//初始化线程参数
	ZeroMemory(&m_TranThrParam, sizeof(m_hTransmitDnsThread));
	m_TranThrParam.hwnd = m_hWnd;
	m_TranThrParam.sock = m_localDnsSock;

	m_hTransmitDnsThread = CreateThread(NULL, 0, TransmitDnsThreadProc, (LPVOID)&m_TranThrParam, 0, NULL);
	if (!m_hTransmitDnsThread)
	{
		return FALSE;
	}
	m_hSaveDnsThread = CreateThread(NULL, 0, SaveDnsThreadProc, (LPVOID)&m_hWnd, 0, NULL);
	if (!m_hSaveDnsThread)
	{
		return FALSE;
	}

	m_hUpdateDnsThread = CreateThread(NULL, 0, UpdateDnsThreadProc, (LPVOID)&m_hWnd, 0, NULL);
	if (!m_hUpdateDnsThread)
	{
		return FALSE;
	}
	return TRUE;
}

void CDnsCacheDlg::UnInitThread()
{
	//退出转发dns线程
	if (m_hTransmitDnsThread)
	{
		//设置结束标记
		m_ThrTranFlag.SetFlag(FALSE);

		//线程阻塞的情况，大部分是花在recvfrom函数中，所以关闭套接字，就会造成recvfrom错误返回而退出线程
		UnInitSocket();
		//等待线程退出
		WaitForThreadFinished(&m_hTransmitDnsThread);
		//不等待也成功，说明这没用
		CloseHandle(m_hTransmitDnsThread);
		m_hTransmitDnsThread = NULL;
	}
	//退出更新dns的线程
	if (m_hUpdateDnsThread)
	{
		m_ThrUpdateFlag.SetFlag(FALSE);
		WaitForThreadFinished(&m_hUpdateDnsThread);
		CloseHandle(m_hUpdateDnsThread);
		m_hUpdateDnsThread = NULL;
	}
	//退出存储数据库的线程
	if (m_hSaveDnsThread)
	{
		//设置结束标记
		m_ThrSaveFlag.SetFlag(FALSE);
		//等待线程退出
		WaitForThreadFinished(&m_hSaveDnsThread);
		CloseHandle(m_hSaveDnsThread);
		m_hSaveDnsThread = NULL;
	}
}

//改进:可以使用多个开放dns地址来切换
BOOL CDnsCacheDlg::TransmitDns(const char *req, int reqlen, char *res, int reslen, int *recvlen)
{
	//创建访问公网dns服务器的sock
	SOCKET dnsServer = 0;
	SOCKADDR_IN addrServer, addrClient;
	int nSend, nRecv;
	int len = sizeof(SOCKADDR_IN);

	dnsServer = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == dnsServer)
	{
		return FALSE;
	}
	//设置超时时间
	int timeout = 1000;//1s
	setsockopt(dnsServer, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	timeout = 1000;
	setsockopt(dnsServer, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(53);
	addrServer.sin_addr.S_un.S_addr = inet_addr("8.8.8.8");

	nSend = sendto(dnsServer, req, reqlen, 0, (SOCKADDR *)&addrServer, len);
	if (SOCKET_ERROR == nSend)
	{
		goto run_err;
	}

	nRecv = recvfrom(dnsServer, res, reslen, 0, (SOCKADDR *)&addrClient, &len);
	if (SOCKET_ERROR == nRecv)
	{
		goto run_err;
	}
	*recvlen = nRecv;

	closesocket(dnsServer);
	return TRUE;
run_err:
	if (dnsServer)
	{
		closesocket(dnsServer);
	}
	return FALSE;
}

//模拟生成主机名格式的dns缓存
//域名不能太长
BOOL CDnsCacheDlg::MakeHostInfoDnsBuf(const string &strDomain, const char *req, int reqlen, char *res, int reslen, int *packlen)
{
	if ((unsigned int)reslen < reqlen + sizeof(Resource) + 22)//最小长度
	{
		return FALSE;
	}

	DNS_HDR *pDnsHdr;
	char *ptr = res;
	string strDomainDst;
	StringVector strVec;

	//生成dns协议格式的字符串
	//example:
	//www.hywebtech.com
	//03 www 09 hywebtech 03 com  占用19
	//存储:03 www 09 hywebtech 03 com 00
	strDomainDst.clear();
	strVec = CPublic::Split(strDomain, '.');
	for (UINT i = 0; i < strVec.size(); i++)
	{
		string &r = strVec[i];
		strDomainDst += r.size();//分割后的字符串长度
		strDomainDst += r;
	}
	strDomainDst += '\0';//最后必须要加0

	/******开始制作自定义dns响应包*******/
	memset(ptr, 0, reslen);
	memcpy(ptr, req, reqlen);//保存接收到的源字符串
	pDnsHdr = (DNS_HDR *)ptr;
	pDnsHdr->flags = htons(0x8180);//标志: 0x8180
	pDnsHdr->answers = htons(1);//回答个数: 1

	//增加并填充一个资源记录(回答)
	Resource resource;
	memset(&resource, 0, sizeof(Resource));
	//0xc00c
	resource.type = htons(PTR);
	resource.cls = htons(INTERNET);
	resource.ttl = htonl(1000);
	resource.len = htons(strDomainDst.size());

	//[DNS协议]
	//操作Answer资源记录
	ptr += reqlen;//指向存储Answer资源的首地址(Domain Name:这里使用压缩指针)

	unsigned short ptrSeek = 0xc00c;//压缩内容的指针位移:1100000000001100
	ptrSeek = htons(0xc00c);//转换成网络字节序
	memcpy(ptr, &ptrSeek, sizeof(unsigned short));//填充压缩指针

	ptr += sizeof(unsigned short);//指向Answer实际定义的结构地址
	memcpy(ptr, &resource, sizeof(Resource));//填充回答资源记录

	ptr += sizeof(Resource);
	
	memcpy(ptr, strDomainDst.c_str(), strDomainDst.size());

	//DNS协议头 + 问题资源头 + 压缩指针 + 回答资源头 + 域名数组
	*packlen = reqlen + 2 + sizeof(Resource) + strDomainDst.size();//响应报文长度

	return TRUE;
}

BOOL CDnsCacheDlg::MakeIpv6DnsBuf(const string &ipv6, const char *req, int reqlen, char *res, int reslen, int *packlen)
{
	DNS_HDR *pDnsHdr;
	char *ptr = res;
	string strDomainDst;
	StringVector strVec;

	/******开始制作自定义dns响应包*******/
	memset(ptr, 0, reslen);
	memcpy(ptr, req, reqlen);//保存接收到的源字符串
	pDnsHdr = (DNS_HDR *)ptr;
	pDnsHdr->flags = htons(0x8180);//标志: 0x8180
	pDnsHdr->answers = htons(1);//回答个数: 1

	//增加并填充一个资源记录(回答)
	Resource resource;
	memset(&resource, 0, sizeof(Resource));
	//0xc00c
	//resource.type = htons(AAAA);
	//resource.cls = htons(INTERNET);
	//resource.ttl = htonl(1000);
	//resource.len = htons(16);//ipv6地址长度
	resource.type = htons(CNAME);
	resource.cls = htons(INTERNET);
	resource.ttl = htonl(1000);
	resource.len = htons(2);

	//[DNS协议]
	//操作Answer资源记录
	ptr += reqlen;//指向存储Answer资源的首地址(Domain Name:这里使用压缩指针)

	unsigned short ptrSeek = 0xc00c;//压缩内容的指针位移:1100000000001100
	ptrSeek = htons(0xc00c);//转换成网络字节序
	memcpy(ptr, &ptrSeek, sizeof(unsigned short));//填充压缩指针

	ptr += sizeof(unsigned short);//指向Answer实际定义的结构地址
	memcpy(ptr, &resource, sizeof(Resource));//填充回答资源记录

	ptr += sizeof(Resource);

	ptrSeek = htons(0xc00c);//转换成网络字节序
	memcpy(ptr, &ptrSeek, sizeof(unsigned short));//填充压缩指针

	//填充ipv6地址
	//unsigned char buf[sizeof(IN6_ADDR)];
	//int ret = inet_pton(AF_INET6, ipv6.c_str(), buf);

	//char bbuf[256];
	//sprintf(bbuf, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
	//CString strtmp = CPublic::MbToWc(bbuf);
	//AfxMessageBox(strtmp);
	
	//char buf[16];
	//memset(buf, '2', sizeof(buf));
	//memcpy(ptr, buf, sizeof(buf));
	//memcpy(ptr, strDomainDst.c_str(), strDomainDst.size());

	//DNS协议头 + 问题资源头 + 压缩指针 + 回答资源头 + 域名数组
	*packlen = reqlen + 2 + sizeof(Resource) + 2;//sizeof(buf);//响应报文长度

	return TRUE;
}

//从数据库载入到内存
void CDnsCacheDlg::LoadSqlToMem()
{
	m_OpFlag.SetFlag(FALSE);
	CString strCmd;
	CSqlStatement *pStmt;
	CString strDomain, strDnsReq, strDnsRes, strIpVec, strVisitTm, strUpdateTm;
	int nVisit, nTtl;
	time_t nLastVisitTime, nLastUpdateTime;
	DNSINFO *pDnsInfo, *pDnsInfoBak;
	string hex, tmp;

	strCmd = _T("select domain,reqhex,reshex,ipvec,visit,ttl,lastvisittime,lastupdatetime,visittime,updatetime from dns_table");
	pStmt = m_SqlDb.Statement(strCmd);

	while (pStmt->NextRow())
	{
		strDomain		= pStmt->ValueString(0);//域名
		strDnsReq		= pStmt->ValueString(1);//dns请求
		strDnsRes		= pStmt->ValueString(2);//dns响应
		strIpVec		= pStmt->ValueString(3);//ip地址列表
		nVisit			= pStmt->ValueInt(4);//访问量
		nTtl			= pStmt->ValueInt(5);//ttl
		nLastVisitTime	= pStmt->ValueLong(6);//最后访问时间
		nLastUpdateTime	= pStmt->ValueLong(7);//最后更新时间
		strVisitTm		= pStmt->ValueString(8);//最后访问时间
		strUpdateTm		= pStmt->ValueString(9);//最后更新时间

		pDnsInfo = new DNSINFO;
		pDnsInfo->strDomain			= strDomain;
		pDnsInfo->nVisit			= nVisit;
		pDnsInfo->nTtl				= nTtl;
		pDnsInfo->nLastVisitTime	= nLastVisitTime;
		pDnsInfo->nLastUpdateTime	= nLastUpdateTime;
		pDnsInfo->strVisitTime		= strVisitTm;
		pDnsInfo->strUpdateTime		= strUpdateTm;
		pDnsInfo->op				= SQLOP_UPDATE;//接下去就是更新了
		//转换dns请求
		hex = CPublic::WcToMb(strDnsReq);
		tmp = CPublic::FromHex(hex);
		pDnsInfo->dnsReq.len = tmp.size();
		memcpy(pDnsInfo->dnsReq.buf, tmp.c_str(), tmp.size());
		//转换dns响应
		hex = CPublic::WcToMb(strDnsRes);
		tmp = CPublic::FromHex(hex);
		pDnsInfo->dnsRes.len = tmp.size();
		memcpy(pDnsInfo->dnsRes.buf, tmp.c_str(), tmp.size());
		//分解ip列表
		pDnsInfo->ipVec = CPublic::Split(strIpVec, _T('|'));

		//添加到内存寻找区
		m_DnsData.AddToSeekMem(pDnsInfo);
		//添加到UI
		AddToList(pDnsInfo);
		//::SendMessage(hwnd, WM_UPDATETOUI, 0, (LPARAM)pDnsInfo);

		//添加到更新链表，另外一个备份
		//类之间不能使用memcpy
		pDnsInfoBak = new DNSINFO;
		CDnsData::CopyDnsInfo(pDnsInfo, pDnsInfoBak);
		m_DnsData.AddToUpdateVec(pDnsInfoBak);

		//响应主界面
		CPublic::ProcessEvent();
	}
	//必须要删除对象
	delete pStmt;
	m_OpFlag.SetFlag(TRUE);
}

//设置开机启动
void CDnsCacheDlg::SetAutoRun(BOOL ok)
{
	CRegKey regKey;
	if (regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")) == ERROR_SUCCESS)
	{
		if (ok)
		{
			regKey.SetStringValue(_T("DnsCache"), CPublic::GetAppAbsoluteFileName());
		}
		else
		{
			regKey.DeleteValue(_T("DnsCache"));
		}
		regKey.Close();
	}
}

BOOL CDnsCacheDlg::IsAutoRun()
{
	CRegKey regKey;
	CString strValue;
	TCHAR buf[256];
	ULONG len;
	if (regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")) == ERROR_SUCCESS)
	{
		if (regKey.QueryStringValue(_T("DnsCache"), buf, &len) == ERROR_SUCCESS)
		{
			strValue = buf;
			regKey.Close();
			if (CPublic::GetAppAbsoluteFileName() == strValue)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CDnsCacheDlg::UpdateDnsInfo(DNSINFO *pDnsInfo)
{
	if (!TransmitDns(pDnsInfo->dnsReq.buf, pDnsInfo->dnsReq.len, pDnsInfo->dnsRes.buf, sizeof(pDnsInfo->dnsRes), &(pDnsInfo->dnsRes.len)))
	{
		return FALSE;
	}
	pDnsInfo->ipVec			= CPublic::GetIpFromDnsBuf(pDnsInfo->dnsRes.buf);
	pDnsInfo->nTtl			= CPublic::GetTtlFromDnsBuf(pDnsInfo->dnsRes.buf);
	pDnsInfo->strUpdateTime = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
	pDnsInfo->nLastUpdateTime		= time(NULL);

	return TRUE;
}

CString CDnsCacheDlg::MakeStatusStr(UINT id, int v1, int v2, LPCTSTR str)
{
	CString strStatus;

	switch (id)
	{
	case IDC_STATIC_VISIT:
		strStatus.Format(_T("访问量: %d个/秒"), v1);
		break;
	case  IDC_STATIC_SQL:
		strStatus.Format(_T("更新数据库(%d/%d): %s"), v1, v2, str);
		break;
	case IDC_STATIC_DNS:
		strStatus.Format(_T("更新DNS缓存(%d/%d): %s"), v1, v2, str);
		break;
	default:
		break;
	}

	return strStatus;
}

void CDnsCacheDlg::InitSystrayIcon()
{
	//不同版本，不同大小
	m_notifyData.cbSize = NOTIFYICONDATA_V2_SIZE;//sizeof(NOTIFYICONDATA);
	m_notifyData.hWnd = m_hWnd;
	m_notifyData.uID = IDI_ICON_LOGO;
	m_notifyData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_notifyData.dwInfoFlags = NIIF_INFO;
	m_notifyData.uTimeout = 5000;
	m_notifyData.uCallbackMessage = WM_NOTIFYICON;
	m_notifyData.hIcon = m_hIcon;
	_tcscpy_s(m_notifyData.szTip, _T("DNS优化工具 v1.0"));
	Shell_NotifyIcon(NIM_ADD, &m_notifyData);
}

void CDnsCacheDlg::InitListCtrl()
{
	DWORD dwStyle = m_ListCtrlDomain.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP ;//行选中效果|网格
	m_ListCtrlDomain.SetExtendedStyle(dwStyle);

	LPCTSTR colArr[] = {_T("ID"), _T("域名"), _T("IP地址"), _T("TTL"), _T("访问量"), _T("最后访问时间"), _T("最后更新时间")};
	int size = sizeof(colArr)/sizeof(LPCTSTR);
	for (int i = 0; i < size; i++)
	{
		m_ListCtrlDomain.InsertColumn(i, colArr[i]);
	}
	//设置列宽
	CRect rect;
	m_ListCtrlDomain.GetClientRect(rect);
	int colWidthArr[7];
	colWidthArr[0] = 60;
	colWidthArr[1] = 150;
	colWidthArr[2] = 150;
	colWidthArr[3] = 60;
	colWidthArr[4] = 60;
	colWidthArr[5] = 130;
	colWidthArr[6] = 130;
	for (int i = 0; i < sizeof(colWidthArr)/sizeof(int); i++)
	{
		m_ListCtrlDomain.SetColumnWidth(i, colWidthArr[i]);
	}
}

void CDnsCacheDlg::UnInitSystrayIcon()
{
	Shell_NotifyIcon(NIM_DELETE, &m_notifyData);
}

void CDnsCacheDlg::ShowSystrayMsg(const CString &tip, const CString &title, const CString &info)
{
	//气泡提示
	m_notifyData.uFlags |= NIF_INFO;

	if (!tip.IsEmpty())
	{
		_tcscpy_s(m_notifyData.szTip, tip);
	}
	
	_tcscpy_s(m_notifyData.szInfoTitle, title);
	_tcscpy_s(m_notifyData.szInfo, info);
	Shell_NotifyIcon(NIM_MODIFY, &m_notifyData);

	//恢复
	m_notifyData.uFlags &= ~NIF_INFO;
}

BOOL CDnsCacheDlg::RefreshNicInfo()
{
	if (!m_nicCtrl.Init() || m_nicCtrl.IsEmpty())
	{
		return FALSE;
	}
	return TRUE;
}

int CDnsCacheDlg::FindListItem(const CString &str, int col)
{
	int iRow = m_ListCtrlDomain.GetItemCount();
	for (int i = 0; i < iRow; i++)
	{
		if (m_ListCtrlDomain.GetItemText(i, col) == str)
		{
			return i;
		}
	}
	return -1;
}

void CDnsCacheDlg::AddToList(const DNSINFO *pDnsInfo)
{
	//添加新的域名到列表
	int iListRow = m_ListCtrlDomain.GetItemCount();
	m_ListCtrlDomain.InsertItem(iListRow, CPublic::IntToStr(iListRow+1));
	m_ListCtrlDomain.SetItemText(iListRow, 1, pDnsInfo->strDomain);
	m_ListCtrlDomain.SetItemText(iListRow, 2, CPublic::MergeVec(pDnsInfo->ipVec, _T('|')));
	m_ListCtrlDomain.SetItemText(iListRow, 3, CPublic::IntToStr(pDnsInfo->nTtl));
	m_ListCtrlDomain.SetItemText(iListRow, 4, CPublic::IntToStr(pDnsInfo->nVisit));
	m_ListCtrlDomain.SetItemText(iListRow, 5, pDnsInfo->strVisitTime);
	m_ListCtrlDomain.SetItemText(iListRow, 6, pDnsInfo->strUpdateTime);
	m_ListCtrlDomain.EnsureVisible(iListRow, FALSE);//显示当前行
}

void CDnsCacheDlg::UpdateToList(const int index, const DNSINFO *pDnsInfo)
{
	m_ListCtrlDomain.SetItemText(index, 1, pDnsInfo->strDomain);
	m_ListCtrlDomain.SetItemText(index, 2, CPublic::MergeVec(pDnsInfo->ipVec, _T('|')));
	m_ListCtrlDomain.SetItemText(index, 3, CPublic::IntToStr(pDnsInfo->nTtl));
	m_ListCtrlDomain.SetItemText(index, 4, CPublic::IntToStr(pDnsInfo->nVisit));
	m_ListCtrlDomain.SetItemText(index, 5, pDnsInfo->strVisitTime);
	m_ListCtrlDomain.SetItemText(index, 6, pDnsInfo->strUpdateTime);
}

DWORD WINAPI CDnsCacheDlg::TransmitDnsThreadProc(LPVOID lpParameter)
{
	//线程参数
	ThrParam *pThrParam = (ThrParam *)lpParameter;
	if (!pThrParam)
	{
		return 0;
	}

	SOCKADDR_IN		addrClient;
	int				len;
	int				reqlen;
	int				reslen;
	char			requestBuf[512];//请求包
	char			responceBuf[512];//响应包
	CString			strDomain;
	DNS_HDR			*pDnsHdr;
	DNSINFO			*pDnsInfo;
	USHORT			nlDnsId;
	//USHORT			questionType;
	time_t			iLastTime;
	int				iCount;
	CString			strStatus;

	CDnsParser		dnsParser;
	
	iLastTime = 0;//最后访问时间
	iCount = 0;//n秒内访问量
	while (m_ThrTranFlag.GetFlag())
	{
		//统计访问量1s
		if (time(NULL) - iLastTime >= 1)
		{
			//统计结束
			strStatus = MakeStatusStr(IDC_STATIC_VISIT, iCount);
			::SendMessage(pThrParam->hwnd, WM_UPDATESTATUS, IDC_STATIC_VISIT, (LPARAM)(LPCTSTR)strStatus);
			iCount = 0;
			iLastTime = time(NULL);
		}
		else
		{
			iCount++;
		}

		//接收dns请求
		len		= sizeof(SOCKADDR_IN);
		reqlen	= recvfrom(pThrParam->sock, requestBuf, sizeof(requestBuf), 0, (SOCKADDR *)&addrClient, &len);
		if (SOCKET_ERROR == reqlen)
		{
			continue;
		}

		dnsParser.SetDnsBuf(requestBuf, reqlen);
		if (dnsParser.StartParse())
		{
			string strRes;
			if (PTR == dnsParser.QuestionType())
			{
				strRes = CDnsParser::EmulateDnsHostResponce(DNS_HOSTINFO, string(requestBuf, reqlen));
				sendto(pThrParam->sock, strRes.c_str(), strRes.size(), 0, (SOCKADDR *)&addrClient, sizeof(SOCKADDR));
				continue;
			}
			else if (AAAA == dnsParser.QuestionType())
			{
				strRes = CDnsParser::EmulateIpv6Responce(string(requestBuf, reqlen));
				sendto(pThrParam->sock, strRes.c_str(), strRes.size(), 0, (SOCKADDR *)&addrClient, sizeof(SOCKADDR));
				continue;
			}
			else if (A != dnsParser.QuestionType())
			{
				//除了A, AAAA, PTR类型，其余类型，只负责转发
				if (!TransmitDns(requestBuf, reqlen, responceBuf, sizeof(responceBuf), &reslen))
				{
					continue;
				}
				//发送给客户端
				sendto(pThrParam->sock, responceBuf, reslen, 0, (SOCKADDR *)&addrClient, sizeof(SOCKADDR));
				continue;
			}
		}
		else
		{
			//除了A, AAAA, PTR类型，其余类型，只负责转发
			if (!TransmitDns(requestBuf, reqlen, responceBuf, sizeof(responceBuf), &reslen))
			{
				continue;
			}
			//发送给客户端
			if (SOCKET_ERROR == sendto(pThrParam->sock, responceBuf, reslen, 0, (SOCKADDR *)&addrClient, sizeof(SOCKADDR)))
			{
				continue;
			}
			continue;
		}

		/****************A类型****************/
		pDnsHdr		= (DNS_HDR *)requestBuf;
		nlDnsId		= pDnsHdr->id;//保存当前dns请求id，网络字节序
		strDomain	= CPublic::GetDomainFromDnsBuf(requestBuf);//域名
		
		//从内存寻找区搜索
		pDnsInfo = m_DnsData.GetDnsFromMem(strDomain);
		if (pDnsInfo)
		{
			//取出dns缓存
			reslen = pDnsInfo->dnsRes.len;
			memcpy(responceBuf, pDnsInfo->dnsRes.buf, pDnsInfo->dnsRes.len);
			pDnsHdr = (DNS_HDR *)responceBuf;
			pDnsHdr->id = nlDnsId;
			sendto(pThrParam->sock, responceBuf, reslen, 0, (SOCKADDR *)&addrClient, sizeof(SOCKADDR));

			//更新字段
			m_DnsData.LockMemDns();
			pDnsInfo->nVisit++;
			pDnsInfo->strVisitTime = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			pDnsInfo->nLastVisitTime = time(NULL);
			pDnsInfo->op = SQLOP_UPDATE;
			m_DnsData.UnlockMemDns();

			m_DnsData.AddToSqlList(pDnsInfo);//添加到数据库链表，以便更新
			//修改到更新链表
			m_DnsData.ModifyUpdateInfo(pDnsInfo);
		}
		else
		{
			//转发dns
			if (!TransmitDns(requestBuf, reqlen, responceBuf, sizeof(responceBuf), &reslen))
			{
				continue;
			}
			//发送给客户端
			if (SOCKET_ERROR == sendto(pThrParam->sock, responceBuf, reslen, 0, (SOCKADDR *)&addrClient, sizeof(SOCKADDR)))
			{
				continue;
			}

			//判断是否有answers
			pDnsHdr = (DNS_HDR *)responceBuf;//dns请求头
			if (0 != ntohs(pDnsHdr->answers))
			{
				//添加到内存寻找区与数据库, 共用指针
				pDnsInfo = CDnsData::CreateDnsInfo(strDomain, requestBuf, reqlen, responceBuf, reslen);
				m_DnsData.AddToSeekMem(pDnsInfo);//添加到内存寻找区
				m_DnsData.AddToSqlList(pDnsInfo);//添加到数据库链表

				pDnsInfo = CDnsData::CreateDnsInfo(strDomain, requestBuf, reqlen, responceBuf, reslen);
				m_DnsData.AddToUpdateVec(pDnsInfo);//添加到更新链表
			}
		}
	}

	//设置访问量状态
	strStatus = MakeStatusStr(IDC_STATIC_VISIT, 0);
	::SendMessage(pThrParam->hwnd, WM_UPDATESTATUS, IDC_STATIC_VISIT, (LPARAM)(LPCTSTR)strStatus);

	return 0;
}

DWORD WINAPI CDnsCacheDlg::SaveDnsThreadProc(LPVOID lpParameter)
{
	if (!lpParameter)
	{
		return 1;
	}
	HWND hwnd = *(HWND *)lpParameter;

	string dnsHex;
	CString strCmd, strDnsHex;
	DNSINFOLIST dnsInfoList;
	int iSleep = 0;//用于记录Sleep个数
	static const int iPeriod = 2;//iPeriod秒才写入数据库一次，在此修改

	while (1)
	{
		//判断结束标记
		if (!m_ThrSaveFlag.GetFlag())
		{
			//将公用链表写入库中，dnsResList可能还遗留着上一次失败的内容，所以不能清空
			//取出数据库链表
			if (m_DnsData.PopSqlList(dnsInfoList))
			{
				//更新到UI
				WriteDnsToUi(hwnd, dnsInfoList);
				//更新到sql
				WriteDnsToSql(hwnd, dnsInfoList);
				dnsInfoList.clear();
			}
			break;
		}

		Sleep(1000);
		if (++iSleep == iPeriod)
		{
			iSleep = 0;
		}
		else
		{
			continue;
		}

		//若上一次写入失败，该链表未删除，继续尝试写入操作
		if (dnsInfoList.empty())
		{
			m_DnsData.PopSqlList(dnsInfoList);
		}

		if (!dnsInfoList.empty())
		{
			//更新到UI
			WriteDnsToUi(hwnd, dnsInfoList);
			//更新到SQL
			if (WriteDnsToSql(hwnd, dnsInfoList))
			{
				dnsInfoList.clear();//test
			}
		}
		////test
		////CString strTest;
		////CStringVector strVec;
		//for (DNSINFOLIST::iterator it = dnsInfoList.begin(); it != dnsInfoList.end(); ++it)
		//{
		//	/*strVec = CPublic::GetIpStrFromDnsBuf((*it)->dnsRes.buf);
		//	strTest.Empty();
		//	for (int i = 0; i < strVec.size(); i++)
		//	{
		//		strTest += strVec[i] + _T("\n");
		//	}
		//	AfxMessageBox(strTest);*/
		//	//CDnsParser dnsParser((*it)->dnsRes.buf, (*it)->dnsRes.len);
		//	//dnsParser.StartParse();
		//	//dnsParser.PrintLog();
		//}
		//dnsInfoList.clear();
	}

	return 0;
}

DWORD WINAPI CDnsCacheDlg::UpdateDnsThreadProc(LPVOID lpParameter)
{
	if (!lpParameter)
	{
		return 1;
	}
	HWND hwnd = *(HWND *)lpParameter;
	int iSleep = 0;//用于记录Sleep个数
	static const int iPeriod = 2;//iPeriod秒才写入数据库一次，在此修改
	DNSINFOVECTOR dnsInfoVec;
	DNSINFO *pDnsInfo;
	CString strStatus;
	int iValue, iTotal;
	
	while (m_ThrUpdateFlag.GetFlag())
	{
		Sleep(1000);
		//test
		//continue;
		if (++iSleep == iPeriod)
		{
			iSleep = 0;
		}
		else
		{
			continue;
		}

		//更新
		dnsInfoVec.clear();
		if (m_DnsData.GetTimeoutDns(dnsInfoVec))
		{
			if (!m_ThrUpdateFlag.GetFlag())
			{
				strStatus = MakeStatusStr(IDC_STATIC_DNS, 0, dnsInfoVec.size(), _T("停止更新"));
				::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_DNS, (LPARAM)(LPCTSTR)strStatus);
				Sleep(200);
				break;
			}

			iValue = 0;
			iTotal = dnsInfoVec.size();
			for (DNSINFOVECTOR::iterator it = dnsInfoVec.begin(); it != dnsInfoVec.end(); ++it)
			{
				pDnsInfo = *it;

				//状态信息
				strStatus = MakeStatusStr(IDC_STATIC_DNS, ++iValue, iTotal, pDnsInfo->strDomain);
				::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_DNS, (LPARAM)(LPCTSTR)strStatus);
								
				UpdateDnsInfo(pDnsInfo);
				//若程序准备退出，不再更新
				if (!m_ThrUpdateFlag.GetFlag())
				{
					//删除没有更新到的dns信息
					dnsInfoVec.erase(it+1, dnsInfoVec.end());
					break;
				}
			}

			strStatus = MakeStatusStr(IDC_STATIC_DNS, iValue, iTotal, _T("准备更新"));
			::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_DNS, (LPARAM)(LPCTSTR)strStatus);

			m_DnsData.UpdateSeekMem(dnsInfoVec);//批量更新

			strStatus = MakeStatusStr(IDC_STATIC_DNS, iValue, iTotal, _T("更新完成"));
			::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_DNS, (LPARAM)(LPCTSTR)strStatus);
		}
	}
	return 0;
}

void CDnsCacheDlg::ShowErrorString(DWORD errId)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errId,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	::MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
	// Free the buffer.
	LocalFree( lpMsgBuf );

}

CString	CDnsCacheDlg::GetErrorString(DWORD errId)
{
	CString errStr;
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errId,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	errStr = (LPCTSTR)lpMsgBuf;
	// Free the buffer.
	LocalFree( lpMsgBuf );
	return errStr;
}

void CDnsCacheDlg::WaitForThreadFinished(HANDLE *pHandle)
{
	DWORD res;
	MSG msg;

	while (1)
	{
		res = MsgWaitForMultipleObjects(1, pHandle, FALSE, INFINITE, QS_ALLEVENTS);
		if (WAIT_OBJECT_0 == res)
		{
			break;
		}
		else
		{
			if (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}

BOOL CDnsCacheDlg::WriteDnsToSql(HWND hwnd, DNSINFOLIST &lst)
{
	DNSINFO		*pDnsInfo;
	CString		reqHex, resHex, strCmd, strIp;
	string		hex;
	CString		strStatus;
	int			iValue, iTotal;
	
	//开始事务
	m_SqlDb.Begin();
	iValue = 0;
	iTotal = lst.size();

	strStatus = MakeStatusStr(IDC_STATIC_SQL, 0, iTotal, _T("准备更新"));
	::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);
	Sleep(200);

	for (DNSINFOLIST::iterator it = lst.begin(); it != lst.end(); ++it)
	{
		pDnsInfo = *it;
		m_DnsData.LockMemDns();
		
		//更新状态信息
		strStatus = MakeStatusStr(IDC_STATIC_SQL, ++iValue, iTotal, pDnsInfo->strDomain);
		::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);
		Sleep(10);

		//dns请求
		hex = CPublic::ToHex(string(pDnsInfo->dnsReq.buf, pDnsInfo->dnsReq.len));
		reqHex = CPublic::MbToWc(hex.c_str());

		//dns响应
		hex = CPublic::ToHex(string(pDnsInfo->dnsRes.buf, pDnsInfo->dnsRes.len));
		resHex = CPublic::MbToWc(hex.c_str());
		strIp = CPublic::MergeVec(pDnsInfo->ipVec, _T('|'));
		
		strCmd.Empty();
		if (SQLOP_INSERT == pDnsInfo->op)
		{
			//存入事务
			strCmd.Format(_T("insert into dns_table(domain, reqhex, reshex, ipvec, visit, ttl, lastvisittime, lastupdatetime, visittime, updatetime) values('%s', '%s', '%s', '%s', %d, %d, %I64d, %I64d, '%s', '%s')"), 
				pDnsInfo->strDomain, reqHex, resHex, strIp, pDnsInfo->nVisit, pDnsInfo->nTtl, pDnsInfo->nLastVisitTime, pDnsInfo->nLastUpdateTime, pDnsInfo->strVisitTime, pDnsInfo->strUpdateTime);
		}
		else if (SQLOP_UPDATE == pDnsInfo->op)
		{
			strCmd.Format(_T("update dns_table set reshex='%s',ipvec='%s',visit=%d,ttl=%d,lastvisittime=%I64d,lastupdatetime=%I64d,visittime='%s',updatetime='%s' where domain='%s'"),
				resHex, strIp, pDnsInfo->nVisit, pDnsInfo->nTtl, pDnsInfo->nLastVisitTime, pDnsInfo->nLastUpdateTime, pDnsInfo->strVisitTime, pDnsInfo->strUpdateTime, pDnsInfo->strDomain);
		}
		m_SqlDb.DirectStatement(strCmd);
		m_DnsData.UnlockMemDns();
	}
	strStatus = MakeStatusStr(IDC_STATIC_SQL, iTotal, iTotal, _T("准备提交"));
	::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);
	Sleep(200);
	
	//提交事务
	if (!m_SqlDb.Commit())
	{
		strStatus = MakeStatusStr(IDC_STATIC_SQL, iTotal, iTotal, _T("准备回滚"));
		::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);
		Sleep(200);

		m_SqlDb.Rollback();

		strStatus = MakeStatusStr(IDC_STATIC_SQL, iTotal, iTotal, _T("回滚成功"));
		::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);
		return FALSE;
	}
	strStatus = MakeStatusStr(IDC_STATIC_SQL, iTotal, iTotal, _T("更新完成"));
	::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);

	return TRUE;
}

void CDnsCacheDlg::WriteDnsToUi(HWND hwnd, DNSINFOLIST &lst)
{
	for (DNSINFOLIST::iterator it = lst.begin(); it != lst.end(); ++it)
	{
		::SendMessage(hwnd, WM_UPDATETOUI, 0, (LPARAM)*it);
	}
}

void CDnsCacheDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	//隐藏 调试过程，先不启用该功能
	m_bAppQuit = true;//test
	if (m_bAppQuit)
	{
		if (!m_OpFlag.GetFlag())
		{
			MessageBox(_T("正在处理数据，请勿关闭!"), _T("警告"));
			return;
		}
		else
		{
			m_OpFlag.SetFlag(FALSE);
		}

		ShowWindow(SW_HIDE);
		OnBnClickedBtnStop();
		UnInitSystrayIcon();
		m_ListCtrlDomain.DeleteAllItems();
		//释放DNS占用空间
		m_DnsData.ReleaseMem();

		m_OpFlag.SetFlag(TRUE);
	}
	else
	{
		ShowWindow(SW_HIDE);
		ShowSystrayMsg(_T(""), _T("提示"), _T("DNS优化工具隐藏在后台运行！"));
		return;
	}

	CDialogEx::OnClose();
}

int CDnsCacheDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	// 设置寻找标记
	//uuid(8c3d6f06-4427-49f4-a508-787c2c4d4bfc)
	::SetProp(m_hWnd, APP_GLOBAL_PROP, APP_GLOBAL_DATA);

	return 0;
}

LRESULT	CDnsCacheDlg::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_LBUTTONDOWN:
		//显示
		if (IsWindowVisible())
		{
			if (IsIconic())
			{
				ShowWindow(SW_RESTORE);//还原
			}
			else
			{
				SetForegroundWindow();//激活
			}
		}//隐藏
		else
		{
			ShowWindow(SW_SHOW);
		}
		break;

	case WM_RBUTTONDOWN:
		CMenu *pMenu = m_SystrayMenu.GetSubMenu(0);
		//pMenu->EnableMenuItem(IDM_SYSTRAY_ABOUT, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
		CPoint pos;
		GetCursorPos(&pos);
		SetForegroundWindow();
		pMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTALIGN, pos.x, pos.y, this);
		break;
	}

	return 0;
}

LRESULT CDnsCacheDlg::OnUpdateToUi(WPARAM wParam, LPARAM lParam)
{
	int index;
	DNSINFO *pDnsInfo = (DNSINFO *)lParam;
	if (!pDnsInfo)
	{
		return 0;
	}

	m_DnsData.LockMemDns();
	index = FindListItem(pDnsInfo->strDomain, 1);
	if (-1 != index)
	{
		//find
		UpdateToList(index, pDnsInfo);
	}
	else
	{
		//not find
		AddToList(pDnsInfo);
	}
	m_DnsData.UnlockMemDns();

	return 0;
}

LRESULT CDnsCacheDlg::OnUpdateStatus(WPARAM wParam, LPARAM lParam)
{
	SetDlgItemText(wParam, (LPCTSTR)lParam);
	return 0;
}

void CDnsCacheDlg::OnSystrayQuit()
{
	// TODO: Add your command handler code here
	m_bAppQuit = TRUE;

	SendMessage(WM_CLOSE);
}


void CDnsCacheDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (22 == nIDEvent)
	{
		static int index = 0;
		static UINT iconArray[] = {IDI_ICON_LOGO1, IDI_ICON_LOGO2, IDI_ICON_LOGO3};
		m_notifyData.hIcon = AfxGetApp()->LoadIcon(iconArray[++index%3]);
		Shell_NotifyIcon(NIM_MODIFY, &m_notifyData);
	}
	else if (23 == nIDEvent)
	{
		KillTimer(23);
		if (IsAutoRun())
		{
			m_ChkAutoRun.SetCheck(BST_CHECKED);
		}
		else
		{
			m_ChkAutoRun.SetCheck(BST_UNCHECKED);
		}
		CPublic::ProcessEvent();
		//软件启动，马上启动服务
		PostMessage(WM_COMMAND, MAKEWPARAM(IDC_BTN_START, BN_CLICKED), NULL);
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CDnsCacheDlg::OnSystrayAbout()
{
	// TODO: Add your command handler code here
	CAboutDlg dlg;
	dlg.DoModal();
}

void CDnsCacheDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	m_Resize.OnSize(cx, cy);
}

//最小尺寸
void CDnsCacheDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: Add your message handler code here and/or call default
	lpMMI->ptMinTrackSize.x = 811;
	lpMMI->ptMinTrackSize.y = 490;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CDnsCacheDlg::OnSystrayStart()
{
	// TODO: 在此添加命令处理程序代码
	OnBnClickedBtnStart();
}


void CDnsCacheDlg::OnSystrayStop()
{
	// TODO: 在此添加命令处理程序代码
	OnBnClickedBtnStop();
}


void CDnsCacheDlg::OnSystrayOptions()
{
	// TODO: 在此添加命令处理程序代码
}

//屏蔽esc和enter键导致dialog退出
BOOL CDnsCacheDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (WM_KEYDOWN == pMsg->message)
	{
		if (VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam)
		{
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDnsCacheDlg::OnBnClickedChkAutorun()
{
	// TODO: 在此添加控件通知处理程序代码
	SetAutoRun(m_ChkAutoRun.GetCheck()==BST_CHECKED);
	//CString strPath;

	//strPath = CPublic::GetAppCurrentDirectory() + _T("\\set.ini");

	//if (m_ChkAutoRun.GetCheck() == BST_CHECKED)
	//{
	//	SetAutoRun(TRUE);
	//	WritePrivateProfileString(_T("Setting"), _T("autorun"), _T("1"), strPath);
	//}
	//else
	//{
	//	SetAutoRun(FALSE);
	//	WritePrivateProfileString(_T("Setting"), _T("autorun"), _T("0"), strPath);
	//}
}
