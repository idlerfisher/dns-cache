
// DnsCacheDlg.cpp : ʵ���ļ�
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CDnsCacheDlg �Ի���
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


// CDnsCacheDlg ��Ϣ�������

BOOL CDnsCacheDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//���ô�С����
	m_Resize.SetOwner(this);
	m_Resize.SetResize(IDC_LIST_DOMAIN, PK_TOP_LEFT, PK_BOTTOM_RIGHT);
	m_Resize.SetResize(IDC_STATIC_VISIT, PK_BOTTOM_LEFT, PK_BOTTOM_LEFT);
	m_Resize.SetResize(IDC_STATIC_SQL, PK_BOTTOM_LEFT, PK_BOTTOM_LEFT);
	m_Resize.SetResize(IDC_STATIC_DNS, PK_BOTTOM_LEFT, PK_BOTTOM_LEFT);

	//��ʼ������ͼ��
	InitSystrayIcon();

	m_SystrayMenu.LoadMenu(IDR_MENU_SYSTRAY);

	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_STOP, MF_DISABLED|MF_BYCOMMAND);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);

	//��ȡ������Ϣ
	RefreshNicInfo();

	//����ListCtrl�ؼ�����
	InitListCtrl();

	//��ʱ����
	//SetTimer(23, 1000, NULL);
	
	//�޸���ʽ
	//m_TreeCtrlIp.ModifyStyle(NULL, TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT);
	//���ı䴰�ڴ�С
	//AfxGetMainWnd()->ModifyStyle(WS_MAXIMIZE|WS_MAXIMIZEBOX|WS_THICKFRAME, 0);
	//AfxGetMainWnd()->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME); 

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CDnsCacheDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CDnsCacheDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDnsCacheDlg::OnBnClickedBtnStart()
{
	// TODO: Add your control notification handler code here	
	static BOOL bFirst = TRUE;

	//ǰ������
	if (m_nicCtrl.IsEmpty())
	{
		if (!RefreshNicInfo())
		{
			MessageBox(_T("����������ڿ��õ������������ԣ�"));
		}
		return;
	}

	//�߳��˳���־
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
	//�����ݿ�
	if (!InitSql())
	{
		goto deal_failed;
	}

	//�״�������������
	if (bFirst)
	{
		bFirst = FALSE;
		LoadSqlToMem();
	}

	//�����߳�
	if (!InitThread())
	{
		goto deal_failed;
	}

	//����dns
	//���ֻ����"127.0.0.1"һ����ַ������ĳЩ�������Ӧ�û�������һ��"127.0.0.1"�ĵ�ַ����ʹ�ñ���dns
	if (!m_nicCtrl.SetAllNicDnsAddr(_T("127.0.0.1"), _T("")))
	//if (!m_nicCtrl.SetAllNicDnsAddr(_T("192.168.1.66"), _T("")))
	{
		goto deal_failed;
	}

	//������ʱ��
	SetTimer(22, 1000, NULL);

	ShowSystrayMsg(_T("DNS�Ż�����������"), _T("��ʾ"), _T("DNS�Ż����߿�ʼ���У�"));

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
	
	//�˳���Ϊ���Ա�����������dns
	m_nicCtrl.SetAllNicDnsAddr(_T("8.8.8.8"), _T("8.8.4.4"));

	MessageBox(_T("����ʧ�ܣ�������!"));
}


void CDnsCacheDlg::OnBnClickedBtnStop()
{
	// TODO: Add your control notification handler code here
	m_OpFlag.SetFlag(FALSE);
	
	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_START, MF_DISABLED|MF_BYCOMMAND);
	m_SystrayMenu.GetSubMenu(0)->EnableMenuItem(IDM_SYSTRAY_STOP, MF_DISABLED|MF_BYCOMMAND);

	//�˳���Ϊ���Ա�����������dns
	m_nicCtrl.SetAllNicDnsAddr(_T("8.8.8.8"), _T("8.8.4.4"));
	UnInitThread();
	UnInitSql();

	//�رն�ʱ��
	KillTimer(22);

	//�޸�����
	m_notifyData.hIcon = AfxGetApp()->LoadIcon(IDI_ICON_LOGO);
	ShowSystrayMsg(_T("DNS�Ż����� v1.0"), _T("��ʾ"), _T("DNS�Ż�����ֹͣ���У�"));

	//ʹ�ܰ�ť
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

	//���ó�ʱʱ��
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

	//�����ݿ�
	if (!m_SqlDb.Open(strDbPath))
	{
		return FALSE;
	}

	//�ж��Ƿ����dns_table��
	stmt = m_SqlDb.Statement(_T("select count(*) from sqlite_master where type='table' and name='dns_table'"));
	if (!stmt->NextRow())
	{
		delete stmt;
		m_SqlDb.Close();
		return FALSE;
	}
	//�������ڸñ������²Ŵ�����
	result = stmt->ValueInt(0);//�õ��ñ����������Ϊ1������ڸñ�
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
	//��ʼ���̲߳���
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
	//�˳�ת��dns�߳�
	if (m_hTransmitDnsThread)
	{
		//���ý������
		m_ThrTranFlag.SetFlag(FALSE);

		//�߳�������������󲿷��ǻ���recvfrom�����У����Թر��׽��֣��ͻ����recvfrom���󷵻ض��˳��߳�
		UnInitSocket();
		//�ȴ��߳��˳�
		WaitForThreadFinished(&m_hTransmitDnsThread);
		//���ȴ�Ҳ�ɹ���˵����û��
		CloseHandle(m_hTransmitDnsThread);
		m_hTransmitDnsThread = NULL;
	}
	//�˳�����dns���߳�
	if (m_hUpdateDnsThread)
	{
		m_ThrUpdateFlag.SetFlag(FALSE);
		WaitForThreadFinished(&m_hUpdateDnsThread);
		CloseHandle(m_hUpdateDnsThread);
		m_hUpdateDnsThread = NULL;
	}
	//�˳��洢���ݿ���߳�
	if (m_hSaveDnsThread)
	{
		//���ý������
		m_ThrSaveFlag.SetFlag(FALSE);
		//�ȴ��߳��˳�
		WaitForThreadFinished(&m_hSaveDnsThread);
		CloseHandle(m_hSaveDnsThread);
		m_hSaveDnsThread = NULL;
	}
}

//�Ľ�:����ʹ�ö������dns��ַ���л�
BOOL CDnsCacheDlg::TransmitDns(const char *req, int reqlen, char *res, int reslen, int *recvlen)
{
	//�������ʹ���dns��������sock
	SOCKET dnsServer = 0;
	SOCKADDR_IN addrServer, addrClient;
	int nSend, nRecv;
	int len = sizeof(SOCKADDR_IN);

	dnsServer = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == dnsServer)
	{
		return FALSE;
	}
	//���ó�ʱʱ��
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

//ģ��������������ʽ��dns����
//��������̫��
BOOL CDnsCacheDlg::MakeHostInfoDnsBuf(const string &strDomain, const char *req, int reqlen, char *res, int reslen, int *packlen)
{
	if ((unsigned int)reslen < reqlen + sizeof(Resource) + 22)//��С����
	{
		return FALSE;
	}

	DNS_HDR *pDnsHdr;
	char *ptr = res;
	string strDomainDst;
	StringVector strVec;

	//����dnsЭ���ʽ���ַ���
	//example:
	//www.hywebtech.com
	//03 www 09 hywebtech 03 com  ռ��19
	//�洢:03 www 09 hywebtech 03 com 00
	strDomainDst.clear();
	strVec = CPublic::Split(strDomain, '.');
	for (UINT i = 0; i < strVec.size(); i++)
	{
		string &r = strVec[i];
		strDomainDst += r.size();//�ָ����ַ�������
		strDomainDst += r;
	}
	strDomainDst += '\0';//������Ҫ��0

	/******��ʼ�����Զ���dns��Ӧ��*******/
	memset(ptr, 0, reslen);
	memcpy(ptr, req, reqlen);//������յ���Դ�ַ���
	pDnsHdr = (DNS_HDR *)ptr;
	pDnsHdr->flags = htons(0x8180);//��־: 0x8180
	pDnsHdr->answers = htons(1);//�ش����: 1

	//���Ӳ����һ����Դ��¼(�ش�)
	Resource resource;
	memset(&resource, 0, sizeof(Resource));
	//0xc00c
	resource.type = htons(PTR);
	resource.cls = htons(INTERNET);
	resource.ttl = htonl(1000);
	resource.len = htons(strDomainDst.size());

	//[DNSЭ��]
	//����Answer��Դ��¼
	ptr += reqlen;//ָ��洢Answer��Դ���׵�ַ(Domain Name:����ʹ��ѹ��ָ��)

	unsigned short ptrSeek = 0xc00c;//ѹ�����ݵ�ָ��λ��:1100000000001100
	ptrSeek = htons(0xc00c);//ת���������ֽ���
	memcpy(ptr, &ptrSeek, sizeof(unsigned short));//���ѹ��ָ��

	ptr += sizeof(unsigned short);//ָ��Answerʵ�ʶ���Ľṹ��ַ
	memcpy(ptr, &resource, sizeof(Resource));//���ش���Դ��¼

	ptr += sizeof(Resource);
	
	memcpy(ptr, strDomainDst.c_str(), strDomainDst.size());

	//DNSЭ��ͷ + ������Դͷ + ѹ��ָ�� + �ش���Դͷ + ��������
	*packlen = reqlen + 2 + sizeof(Resource) + strDomainDst.size();//��Ӧ���ĳ���

	return TRUE;
}

BOOL CDnsCacheDlg::MakeIpv6DnsBuf(const string &ipv6, const char *req, int reqlen, char *res, int reslen, int *packlen)
{
	DNS_HDR *pDnsHdr;
	char *ptr = res;
	string strDomainDst;
	StringVector strVec;

	/******��ʼ�����Զ���dns��Ӧ��*******/
	memset(ptr, 0, reslen);
	memcpy(ptr, req, reqlen);//������յ���Դ�ַ���
	pDnsHdr = (DNS_HDR *)ptr;
	pDnsHdr->flags = htons(0x8180);//��־: 0x8180
	pDnsHdr->answers = htons(1);//�ش����: 1

	//���Ӳ����һ����Դ��¼(�ش�)
	Resource resource;
	memset(&resource, 0, sizeof(Resource));
	//0xc00c
	//resource.type = htons(AAAA);
	//resource.cls = htons(INTERNET);
	//resource.ttl = htonl(1000);
	//resource.len = htons(16);//ipv6��ַ����
	resource.type = htons(CNAME);
	resource.cls = htons(INTERNET);
	resource.ttl = htonl(1000);
	resource.len = htons(2);

	//[DNSЭ��]
	//����Answer��Դ��¼
	ptr += reqlen;//ָ��洢Answer��Դ���׵�ַ(Domain Name:����ʹ��ѹ��ָ��)

	unsigned short ptrSeek = 0xc00c;//ѹ�����ݵ�ָ��λ��:1100000000001100
	ptrSeek = htons(0xc00c);//ת���������ֽ���
	memcpy(ptr, &ptrSeek, sizeof(unsigned short));//���ѹ��ָ��

	ptr += sizeof(unsigned short);//ָ��Answerʵ�ʶ���Ľṹ��ַ
	memcpy(ptr, &resource, sizeof(Resource));//���ش���Դ��¼

	ptr += sizeof(Resource);

	ptrSeek = htons(0xc00c);//ת���������ֽ���
	memcpy(ptr, &ptrSeek, sizeof(unsigned short));//���ѹ��ָ��

	//���ipv6��ַ
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

	//DNSЭ��ͷ + ������Դͷ + ѹ��ָ�� + �ش���Դͷ + ��������
	*packlen = reqlen + 2 + sizeof(Resource) + 2;//sizeof(buf);//��Ӧ���ĳ���

	return TRUE;
}

//�����ݿ����뵽�ڴ�
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
		strDomain		= pStmt->ValueString(0);//����
		strDnsReq		= pStmt->ValueString(1);//dns����
		strDnsRes		= pStmt->ValueString(2);//dns��Ӧ
		strIpVec		= pStmt->ValueString(3);//ip��ַ�б�
		nVisit			= pStmt->ValueInt(4);//������
		nTtl			= pStmt->ValueInt(5);//ttl
		nLastVisitTime	= pStmt->ValueLong(6);//������ʱ��
		nLastUpdateTime	= pStmt->ValueLong(7);//������ʱ��
		strVisitTm		= pStmt->ValueString(8);//������ʱ��
		strUpdateTm		= pStmt->ValueString(9);//������ʱ��

		pDnsInfo = new DNSINFO;
		pDnsInfo->strDomain			= strDomain;
		pDnsInfo->nVisit			= nVisit;
		pDnsInfo->nTtl				= nTtl;
		pDnsInfo->nLastVisitTime	= nLastVisitTime;
		pDnsInfo->nLastUpdateTime	= nLastUpdateTime;
		pDnsInfo->strVisitTime		= strVisitTm;
		pDnsInfo->strUpdateTime		= strUpdateTm;
		pDnsInfo->op				= SQLOP_UPDATE;//����ȥ���Ǹ�����
		//ת��dns����
		hex = CPublic::WcToMb(strDnsReq);
		tmp = CPublic::FromHex(hex);
		pDnsInfo->dnsReq.len = tmp.size();
		memcpy(pDnsInfo->dnsReq.buf, tmp.c_str(), tmp.size());
		//ת��dns��Ӧ
		hex = CPublic::WcToMb(strDnsRes);
		tmp = CPublic::FromHex(hex);
		pDnsInfo->dnsRes.len = tmp.size();
		memcpy(pDnsInfo->dnsRes.buf, tmp.c_str(), tmp.size());
		//�ֽ�ip�б�
		pDnsInfo->ipVec = CPublic::Split(strIpVec, _T('|'));

		//��ӵ��ڴ�Ѱ����
		m_DnsData.AddToSeekMem(pDnsInfo);
		//��ӵ�UI
		AddToList(pDnsInfo);
		//::SendMessage(hwnd, WM_UPDATETOUI, 0, (LPARAM)pDnsInfo);

		//��ӵ�������������һ������
		//��֮�䲻��ʹ��memcpy
		pDnsInfoBak = new DNSINFO;
		CDnsData::CopyDnsInfo(pDnsInfo, pDnsInfoBak);
		m_DnsData.AddToUpdateVec(pDnsInfoBak);

		//��Ӧ������
		CPublic::ProcessEvent();
	}
	//����Ҫɾ������
	delete pStmt;
	m_OpFlag.SetFlag(TRUE);
}

//���ÿ�������
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
		strStatus.Format(_T("������: %d��/��"), v1);
		break;
	case  IDC_STATIC_SQL:
		strStatus.Format(_T("�������ݿ�(%d/%d): %s"), v1, v2, str);
		break;
	case IDC_STATIC_DNS:
		strStatus.Format(_T("����DNS����(%d/%d): %s"), v1, v2, str);
		break;
	default:
		break;
	}

	return strStatus;
}

void CDnsCacheDlg::InitSystrayIcon()
{
	//��ͬ�汾����ͬ��С
	m_notifyData.cbSize = NOTIFYICONDATA_V2_SIZE;//sizeof(NOTIFYICONDATA);
	m_notifyData.hWnd = m_hWnd;
	m_notifyData.uID = IDI_ICON_LOGO;
	m_notifyData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_notifyData.dwInfoFlags = NIIF_INFO;
	m_notifyData.uTimeout = 5000;
	m_notifyData.uCallbackMessage = WM_NOTIFYICON;
	m_notifyData.hIcon = m_hIcon;
	_tcscpy_s(m_notifyData.szTip, _T("DNS�Ż����� v1.0"));
	Shell_NotifyIcon(NIM_ADD, &m_notifyData);
}

void CDnsCacheDlg::InitListCtrl()
{
	DWORD dwStyle = m_ListCtrlDomain.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP ;//��ѡ��Ч��|����
	m_ListCtrlDomain.SetExtendedStyle(dwStyle);

	LPCTSTR colArr[] = {_T("ID"), _T("����"), _T("IP��ַ"), _T("TTL"), _T("������"), _T("������ʱ��"), _T("������ʱ��")};
	int size = sizeof(colArr)/sizeof(LPCTSTR);
	for (int i = 0; i < size; i++)
	{
		m_ListCtrlDomain.InsertColumn(i, colArr[i]);
	}
	//�����п�
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
	//������ʾ
	m_notifyData.uFlags |= NIF_INFO;

	if (!tip.IsEmpty())
	{
		_tcscpy_s(m_notifyData.szTip, tip);
	}
	
	_tcscpy_s(m_notifyData.szInfoTitle, title);
	_tcscpy_s(m_notifyData.szInfo, info);
	Shell_NotifyIcon(NIM_MODIFY, &m_notifyData);

	//�ָ�
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
	//����µ��������б�
	int iListRow = m_ListCtrlDomain.GetItemCount();
	m_ListCtrlDomain.InsertItem(iListRow, CPublic::IntToStr(iListRow+1));
	m_ListCtrlDomain.SetItemText(iListRow, 1, pDnsInfo->strDomain);
	m_ListCtrlDomain.SetItemText(iListRow, 2, CPublic::MergeVec(pDnsInfo->ipVec, _T('|')));
	m_ListCtrlDomain.SetItemText(iListRow, 3, CPublic::IntToStr(pDnsInfo->nTtl));
	m_ListCtrlDomain.SetItemText(iListRow, 4, CPublic::IntToStr(pDnsInfo->nVisit));
	m_ListCtrlDomain.SetItemText(iListRow, 5, pDnsInfo->strVisitTime);
	m_ListCtrlDomain.SetItemText(iListRow, 6, pDnsInfo->strUpdateTime);
	m_ListCtrlDomain.EnsureVisible(iListRow, FALSE);//��ʾ��ǰ��
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
	//�̲߳���
	ThrParam *pThrParam = (ThrParam *)lpParameter;
	if (!pThrParam)
	{
		return 0;
	}

	SOCKADDR_IN		addrClient;
	int				len;
	int				reqlen;
	int				reslen;
	char			requestBuf[512];//�����
	char			responceBuf[512];//��Ӧ��
	CString			strDomain;
	DNS_HDR			*pDnsHdr;
	DNSINFO			*pDnsInfo;
	USHORT			nlDnsId;
	//USHORT			questionType;
	time_t			iLastTime;
	int				iCount;
	CString			strStatus;

	CDnsParser		dnsParser;
	
	iLastTime = 0;//������ʱ��
	iCount = 0;//n���ڷ�����
	while (m_ThrTranFlag.GetFlag())
	{
		//ͳ�Ʒ�����1s
		if (time(NULL) - iLastTime >= 1)
		{
			//ͳ�ƽ���
			strStatus = MakeStatusStr(IDC_STATIC_VISIT, iCount);
			::SendMessage(pThrParam->hwnd, WM_UPDATESTATUS, IDC_STATIC_VISIT, (LPARAM)(LPCTSTR)strStatus);
			iCount = 0;
			iLastTime = time(NULL);
		}
		else
		{
			iCount++;
		}

		//����dns����
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
				//����A, AAAA, PTR���ͣ��������ͣ�ֻ����ת��
				if (!TransmitDns(requestBuf, reqlen, responceBuf, sizeof(responceBuf), &reslen))
				{
					continue;
				}
				//���͸��ͻ���
				sendto(pThrParam->sock, responceBuf, reslen, 0, (SOCKADDR *)&addrClient, sizeof(SOCKADDR));
				continue;
			}
		}
		else
		{
			//����A, AAAA, PTR���ͣ��������ͣ�ֻ����ת��
			if (!TransmitDns(requestBuf, reqlen, responceBuf, sizeof(responceBuf), &reslen))
			{
				continue;
			}
			//���͸��ͻ���
			if (SOCKET_ERROR == sendto(pThrParam->sock, responceBuf, reslen, 0, (SOCKADDR *)&addrClient, sizeof(SOCKADDR)))
			{
				continue;
			}
			continue;
		}

		/****************A����****************/
		pDnsHdr		= (DNS_HDR *)requestBuf;
		nlDnsId		= pDnsHdr->id;//���浱ǰdns����id�������ֽ���
		strDomain	= CPublic::GetDomainFromDnsBuf(requestBuf);//����
		
		//���ڴ�Ѱ��������
		pDnsInfo = m_DnsData.GetDnsFromMem(strDomain);
		if (pDnsInfo)
		{
			//ȡ��dns����
			reslen = pDnsInfo->dnsRes.len;
			memcpy(responceBuf, pDnsInfo->dnsRes.buf, pDnsInfo->dnsRes.len);
			pDnsHdr = (DNS_HDR *)responceBuf;
			pDnsHdr->id = nlDnsId;
			sendto(pThrParam->sock, responceBuf, reslen, 0, (SOCKADDR *)&addrClient, sizeof(SOCKADDR));

			//�����ֶ�
			m_DnsData.LockMemDns();
			pDnsInfo->nVisit++;
			pDnsInfo->strVisitTime = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			pDnsInfo->nLastVisitTime = time(NULL);
			pDnsInfo->op = SQLOP_UPDATE;
			m_DnsData.UnlockMemDns();

			m_DnsData.AddToSqlList(pDnsInfo);//��ӵ����ݿ������Ա����
			//�޸ĵ���������
			m_DnsData.ModifyUpdateInfo(pDnsInfo);
		}
		else
		{
			//ת��dns
			if (!TransmitDns(requestBuf, reqlen, responceBuf, sizeof(responceBuf), &reslen))
			{
				continue;
			}
			//���͸��ͻ���
			if (SOCKET_ERROR == sendto(pThrParam->sock, responceBuf, reslen, 0, (SOCKADDR *)&addrClient, sizeof(SOCKADDR)))
			{
				continue;
			}

			//�ж��Ƿ���answers
			pDnsHdr = (DNS_HDR *)responceBuf;//dns����ͷ
			if (0 != ntohs(pDnsHdr->answers))
			{
				//��ӵ��ڴ�Ѱ���������ݿ�, ����ָ��
				pDnsInfo = CDnsData::CreateDnsInfo(strDomain, requestBuf, reqlen, responceBuf, reslen);
				m_DnsData.AddToSeekMem(pDnsInfo);//��ӵ��ڴ�Ѱ����
				m_DnsData.AddToSqlList(pDnsInfo);//��ӵ����ݿ�����

				pDnsInfo = CDnsData::CreateDnsInfo(strDomain, requestBuf, reqlen, responceBuf, reslen);
				m_DnsData.AddToUpdateVec(pDnsInfo);//��ӵ���������
			}
		}
	}

	//���÷�����״̬
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
	int iSleep = 0;//���ڼ�¼Sleep����
	static const int iPeriod = 2;//iPeriod���д�����ݿ�һ�Σ��ڴ��޸�

	while (1)
	{
		//�жϽ������
		if (!m_ThrSaveFlag.GetFlag())
		{
			//����������д����У�dnsResList���ܻ���������һ��ʧ�ܵ����ݣ����Բ������
			//ȡ�����ݿ�����
			if (m_DnsData.PopSqlList(dnsInfoList))
			{
				//���µ�UI
				WriteDnsToUi(hwnd, dnsInfoList);
				//���µ�sql
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

		//����һ��д��ʧ�ܣ�������δɾ������������д�����
		if (dnsInfoList.empty())
		{
			m_DnsData.PopSqlList(dnsInfoList);
		}

		if (!dnsInfoList.empty())
		{
			//���µ�UI
			WriteDnsToUi(hwnd, dnsInfoList);
			//���µ�SQL
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
	int iSleep = 0;//���ڼ�¼Sleep����
	static const int iPeriod = 2;//iPeriod���д�����ݿ�һ�Σ��ڴ��޸�
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

		//����
		dnsInfoVec.clear();
		if (m_DnsData.GetTimeoutDns(dnsInfoVec))
		{
			if (!m_ThrUpdateFlag.GetFlag())
			{
				strStatus = MakeStatusStr(IDC_STATIC_DNS, 0, dnsInfoVec.size(), _T("ֹͣ����"));
				::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_DNS, (LPARAM)(LPCTSTR)strStatus);
				Sleep(200);
				break;
			}

			iValue = 0;
			iTotal = dnsInfoVec.size();
			for (DNSINFOVECTOR::iterator it = dnsInfoVec.begin(); it != dnsInfoVec.end(); ++it)
			{
				pDnsInfo = *it;

				//״̬��Ϣ
				strStatus = MakeStatusStr(IDC_STATIC_DNS, ++iValue, iTotal, pDnsInfo->strDomain);
				::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_DNS, (LPARAM)(LPCTSTR)strStatus);
								
				UpdateDnsInfo(pDnsInfo);
				//������׼���˳������ٸ���
				if (!m_ThrUpdateFlag.GetFlag())
				{
					//ɾ��û�и��µ���dns��Ϣ
					dnsInfoVec.erase(it+1, dnsInfoVec.end());
					break;
				}
			}

			strStatus = MakeStatusStr(IDC_STATIC_DNS, iValue, iTotal, _T("׼������"));
			::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_DNS, (LPARAM)(LPCTSTR)strStatus);

			m_DnsData.UpdateSeekMem(dnsInfoVec);//��������

			strStatus = MakeStatusStr(IDC_STATIC_DNS, iValue, iTotal, _T("�������"));
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
	
	//��ʼ����
	m_SqlDb.Begin();
	iValue = 0;
	iTotal = lst.size();

	strStatus = MakeStatusStr(IDC_STATIC_SQL, 0, iTotal, _T("׼������"));
	::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);
	Sleep(200);

	for (DNSINFOLIST::iterator it = lst.begin(); it != lst.end(); ++it)
	{
		pDnsInfo = *it;
		m_DnsData.LockMemDns();
		
		//����״̬��Ϣ
		strStatus = MakeStatusStr(IDC_STATIC_SQL, ++iValue, iTotal, pDnsInfo->strDomain);
		::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);
		Sleep(10);

		//dns����
		hex = CPublic::ToHex(string(pDnsInfo->dnsReq.buf, pDnsInfo->dnsReq.len));
		reqHex = CPublic::MbToWc(hex.c_str());

		//dns��Ӧ
		hex = CPublic::ToHex(string(pDnsInfo->dnsRes.buf, pDnsInfo->dnsRes.len));
		resHex = CPublic::MbToWc(hex.c_str());
		strIp = CPublic::MergeVec(pDnsInfo->ipVec, _T('|'));
		
		strCmd.Empty();
		if (SQLOP_INSERT == pDnsInfo->op)
		{
			//��������
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
	strStatus = MakeStatusStr(IDC_STATIC_SQL, iTotal, iTotal, _T("׼���ύ"));
	::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);
	Sleep(200);
	
	//�ύ����
	if (!m_SqlDb.Commit())
	{
		strStatus = MakeStatusStr(IDC_STATIC_SQL, iTotal, iTotal, _T("׼���ع�"));
		::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);
		Sleep(200);

		m_SqlDb.Rollback();

		strStatus = MakeStatusStr(IDC_STATIC_SQL, iTotal, iTotal, _T("�ع��ɹ�"));
		::SendMessage(hwnd, WM_UPDATESTATUS, IDC_STATIC_SQL, (LPARAM)(LPCTSTR)strStatus);
		return FALSE;
	}
	strStatus = MakeStatusStr(IDC_STATIC_SQL, iTotal, iTotal, _T("�������"));
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
	//���� ���Թ��̣��Ȳ����øù���
	m_bAppQuit = true;//test
	if (m_bAppQuit)
	{
		if (!m_OpFlag.GetFlag())
		{
			MessageBox(_T("���ڴ������ݣ�����ر�!"), _T("����"));
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
		//�ͷ�DNSռ�ÿռ�
		m_DnsData.ReleaseMem();

		m_OpFlag.SetFlag(TRUE);
	}
	else
	{
		ShowWindow(SW_HIDE);
		ShowSystrayMsg(_T(""), _T("��ʾ"), _T("DNS�Ż����������ں�̨���У�"));
		return;
	}

	CDialogEx::OnClose();
}

int CDnsCacheDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	// ����Ѱ�ұ��
	//uuid(8c3d6f06-4427-49f4-a508-787c2c4d4bfc)
	::SetProp(m_hWnd, APP_GLOBAL_PROP, APP_GLOBAL_DATA);

	return 0;
}

LRESULT	CDnsCacheDlg::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_LBUTTONDOWN:
		//��ʾ
		if (IsWindowVisible())
		{
			if (IsIconic())
			{
				ShowWindow(SW_RESTORE);//��ԭ
			}
			else
			{
				SetForegroundWindow();//����
			}
		}//����
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
		//���������������������
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

//��С�ߴ�
void CDnsCacheDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: Add your message handler code here and/or call default
	lpMMI->ptMinTrackSize.x = 811;
	lpMMI->ptMinTrackSize.y = 490;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CDnsCacheDlg::OnSystrayStart()
{
	// TODO: �ڴ���������������
	OnBnClickedBtnStart();
}


void CDnsCacheDlg::OnSystrayStop()
{
	// TODO: �ڴ���������������
	OnBnClickedBtnStop();
}


void CDnsCacheDlg::OnSystrayOptions()
{
	// TODO: �ڴ���������������
}

//����esc��enter������dialog�˳�
BOOL CDnsCacheDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
