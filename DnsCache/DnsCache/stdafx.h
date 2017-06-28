
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持

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

//网卡信息
typedef struct tagNicInfo
{
	ULONG			Index;//序号: 2
	CString			AdapterName;//网卡唯一名称: {5DFDF241-1198-4371-8008-2E4D18C26DDB}
	CString			Description;//描述: Realtek PCIe GBE Family Controller #2 - 数据包计划程序微型端口
	CString			FriendlyName;//名称: 本地连接
	CString			DhcpServer;//dhcp地址
	CStringVector	IpAddrList;//IP地址列表
	CStringVector	GatewayList;//网关地址列表
	CStringVector	DnsList;//DNS地址列表,前2个为首选/备用DNS
}NICINFO;
typedef vector<NICINFO *> NICINFOVector;

typedef vector<DWORD> DWORDVector;

//dns信息
typedef struct tagDnsBuf
{
	int		len;
	char	buf[512];//使用UDP时，dns包不超过512字节
}DNSBUF;

typedef enum tagSqlOp
{
	SQLOP_INSERT,
	SQLOP_UPDATE
}SqlOp;

typedef struct tagDnsInfo
{
	CString			strDomain;//域名
	DNSBUF			dnsReq;//dns请求
	DNSBUF			dnsRes;//dns相应
	CStringVector	ipVec;//ip链表
	int				nVisit;//访问量
	CString			strVisitTime;//最后访问时间
	CString			strUpdateTime;//最后更新时间
	time_t			nLastVisitTime;//最后访问时间(秒)
	time_t			nLastUpdateTime;//最后更新时间(秒)
	int				nTtl;//ttl
	SqlOp			op;//数据库操作
}DNSINFO;
typedef map<CString, DNSINFO *> DNSINFOMAP;//CString保存域名
typedef list<DNSINFO *> DNSINFOLIST;//用于数据库链表
typedef vector<DNSINFO *> DNSINFOVECTOR;

//用于应用程序单例的宏
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


