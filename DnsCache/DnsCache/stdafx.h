
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��

//#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <Mprapi.h>
#include <IPTypes.h>
#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Mprapi.lib")
//#pragma comment(lib, "WS2_32.Lib")

//user
#include "IpHeader.h"
#include "DnsHeader.h"
#include "DbSqlite.h"

#pragma comment(lib, "sqlite3.lib")
//#pragma comment(lib, "sqlite3d.lib")

typedef vector<CString> CStringVector;
typedef vector<string>  StringVector;

//������Ϣ
typedef struct tagNicInfo
{
	ULONG			Index;//���: 2
	CString			AdapterName;//����Ψһ����: {5DFDF241-1198-4371-8008-2E4D18C26DDB}
	CString			Description;//����: Realtek PCIe GBE Family Controller #2 - ���ݰ��ƻ�����΢�Ͷ˿�
	CString			FriendlyName;//����: ��������
	CString			DhcpServer;//dhcp��ַ
	CStringVector	IpAddrList;//IP��ַ�б�
	CStringVector	GatewayList;//���ص�ַ�б�
	CStringVector	DnsList;//DNS��ַ�б�,ǰ2��Ϊ��ѡ/����DNS
}NICINFO;
typedef vector<NICINFO *> NICINFOVector;

typedef vector<DWORD> DWORDVector;

//dns��Ϣ
typedef struct tagDnsBuf
{
	int		len;
	char	buf[512];//ʹ��UDPʱ��dns��������512�ֽ�
}DNSBUF;

typedef enum tagSqlOp
{
	SQLOP_INSERT,
	SQLOP_UPDATE
}SqlOp;

typedef struct tagDnsInfo
{
	CString			strDomain;//����
	DNSBUF			dnsReq;//dns����
	DNSBUF			dnsRes;//dns��Ӧ
	CStringVector	ipVec;//ip����
	int				nVisit;//������
	CString			strVisitTime;//������ʱ��
	CString			strUpdateTime;//������ʱ��
	time_t			nLastVisitTime;//������ʱ��(��)
	time_t			nLastUpdateTime;//������ʱ��(��)
	int				nTtl;//ttl
	SqlOp			op;//���ݿ����
}DNSINFO;
typedef map<CString, DNSINFO *> DNSINFOMAP;//CString��������
typedef list<DNSINFO *> DNSINFOLIST;//�������ݿ�����
typedef vector<DNSINFO *> DNSINFOVECTOR;

//����Ӧ�ó������ĺ�
#define APP_GLOBAL_PROP		_T("8c3d6f06-4427-49f4-a508-787c2c4d4bfc")
#define APP_GLOBAL_MUTEX	_T("b33c9c9c-8a8b-443a-b87f-641b0984998d")
#define APP_GLOBAL_DATA		(HANDLE)22

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


