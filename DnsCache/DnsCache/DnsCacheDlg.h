
// DnsCacheDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "NicCtrl.h"
#include "ResizeControl.h"
#include "DnsData.h"
#include "Flag.h"

using namespace std;

#define SQLITE_DNS_DB	_T("dns.db")

//线程参数结构体
typedef struct tagThrParam
{
	HWND		hwnd;//窗口句柄
	SOCKET		sock;//本地dns服务器
}ThrParam;

typedef vector<UINT> UINTVector;

#define	WM_NOTIFYICON	WM_USER+1
#define WM_UPDATETOUI	WM_USER+2
#define WM_UPDATESTATUS	WM_USER+3
#define WM_SETAUTORUN	WM_USER+4

// CDnsCacheDlg 对话框
class CDnsCacheDlg : public CDialogEx
{
// 构造
public:
	CDnsCacheDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CDnsCacheDlg();

// 对话框数据
	enum { IDD = IDD_DNSCACHE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void		OnBnClickedBtnStart();
	afx_msg void		OnBnClickedBtnStop();
	afx_msg void		OnClose();
	afx_msg int			OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT		OnNotifyIcon(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT		OnUpdateToUi(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT		OnUpdateStatus(WPARAM wParam, LPARAM lParam);
	BOOL				InitSocket();
	void				UnInitSocket();
	BOOL				InitSql();
	void				UnInitSql();
	BOOL				InitThread();
	void				UnInitThread();
	void				InitSystrayIcon();
	void				InitListCtrl();
	void				UnInitSystrayIcon();
	void				ShowSystrayMsg(const CString &tip, const CString &title, const CString &info);
	BOOL				RefreshNicInfo();
	int					FindListItem(const CString &str, int col);
	void				AddToList(const DNSINFO *pDnsInfo);
	void				UpdateToList(const int index, const DNSINFO *pDnsInfo);
	void				LoadSqlToMem();
	void				SetAutoRun(BOOL ok);
	BOOL				IsAutoRun();

	static BOOL			TransmitDns(const char *req, int reqlen, char *res, int reslen, int *recvlen);
	static BOOL			MakeHostInfoDnsBuf(const string &strDomain, const char *req, int reqlen, char *res, int reslen, int *packlen);
	static BOOL			MakeIpv6DnsBuf(const string &ipv6, const char *req, int reqlen, char *res, int reslen, int *packlen);
	static DWORD WINAPI TransmitDnsThreadProc(LPVOID lpParameter);
	static DWORD WINAPI SaveDnsThreadProc(LPVOID lpParameter);
	static DWORD WINAPI UpdateDnsThreadProc(LPVOID lpParameter);
	static void			ShowErrorString(DWORD errId);
	static CString		GetErrorString(DWORD errId);
	static void			WaitForThreadFinished(HANDLE *pHandle);
	static BOOL			WriteDnsToSql(HWND hwnd, DNSINFOLIST &lst);
	static void			WriteDnsToUi(HWND hwnd, DNSINFOLIST &lst);
	static BOOL			UpdateDnsInfo(DNSINFO *pDnsInfo);
	static CString		MakeStatusStr(UINT id, int v1, int v2=0, LPCTSTR str=_T(""));

private:
	SOCKET				m_localDnsSock;
	DWORD				m_dwDnsIp;
	HANDLE				m_hTransmitDnsThread;//转发线程
	HANDLE				m_hSaveDnsThread;//更新数据库与ui的线程
	HANDLE				m_hUpdateDnsThread;//更新dns的线程
	CNicCtrl			m_nicCtrl;
	CString				m_strNicAdapterName;
	ThrParam			m_TranThrParam;
	NOTIFYICONDATA		m_notifyData;
	BOOL				m_bAppQuit;
	CMenu				m_menu;
	CResizeControl		m_Resize;
	CStringVector		m_strAdapterNameVec;
	CMenu				m_SystrayMenu;

	static CDnsData		m_DnsData;
	static CDbSQLite	m_SqlDb;
	static CFlag		m_ThrTranFlag;
	static CFlag		m_ThrSaveFlag;
	static CFlag		m_ThrUpdateFlag;
	static CFlag		m_OpFlag;//true:可以操作 false:不可操作
public:
	CListCtrl m_ListCtrlDomain;
	afx_msg void OnSystrayQuit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSystrayAbout();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSystrayStart();
	afx_msg void OnSystrayStop();
	afx_msg void OnSystrayOptions();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedChkAutorun();
	CButton m_ChkAutoRun;
};
