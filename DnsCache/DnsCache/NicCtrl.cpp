#include "StdAfx.h"
#include "NicCtrl.h"
#include "Public.h"
#include "Wmi.h"

CNicCtrl::CNicCtrl(void)
{
}


CNicCtrl::~CNicCtrl(void)
{
	ReleaseNicInfo(m_nicInfoVec);
}

BOOL CNicCtrl::Init()
{
	return GetNicInfo(m_nicInfoVec);
}

CString CNicCtrl::GetAdapterName(int index)
{
	if (index > (int)m_nicInfoVec.size()-1)
	{
		return _T("");
	}
	return m_nicInfoVec.at(index)->AdapterName;
}

CStringVector CNicCtrl::GetAdapterNames()
{
	CStringVector strVec;
	for (NICINFOVector::iterator it = m_nicInfoVec.begin(); it != m_nicInfoVec.end(); ++it)
	{
		strVec.push_back((*it)->AdapterName);
	}
	return strVec;
}

CStringVector CNicCtrl::GetFriendlyNames()
{
	CStringVector strVec;
	for (NICINFOVector::iterator it = m_nicInfoVec.begin(); it != m_nicInfoVec.end(); ++it)
	{
		strVec.push_back((*it)->FriendlyName);
	}
	return strVec;
}

BOOL CNicCtrl::GetNicInfo(NICINFOVector &nicInfoVec)
{
	//清空链表
	ReleaseNicInfo(nicInfoVec);

	//获取网卡的基本信息
	if (!GetNicBaseInfo(nicInfoVec))
	{
		return FALSE;
	}

	//获取网卡名称
	if (!GetNicFriendlyName(nicInfoVec))
	{
		return FALSE;
	}
	return TRUE;
}

void CNicCtrl::ReleaseNicInfo(NICINFOVector &nicInfoVec)
{
	int size = nicInfoVec.size();
	for (int i = 0; i < size; i++)
	{
		delete nicInfoVec.at(i);
	}
	nicInfoVec.clear();
}

BOOL CNicCtrl::GetNicBaseInfo(NICINFOVector &nicInfoVec)
{
	DWORD dwRet;
	ULONG ulOutSize;
	IP_ADAPTER_INFO *pIpAdapterInfo;
	NICINFO *pNicInfo;

	//获取网卡信息
	dwRet = GetAdaptersInfo(NULL, &ulOutSize);
	if (ERROR_BUFFER_OVERFLOW != dwRet)
	{
		return FALSE;
	}
	pIpAdapterInfo = (IP_ADAPTER_INFO *)new char[ulOutSize];
	dwRet = GetAdaptersInfo(pIpAdapterInfo, &ulOutSize);
	if (ERROR_SUCCESS != dwRet)
	{
		delete [] pIpAdapterInfo;
		return FALSE;
	}

	//遍历
	while (pIpAdapterInfo)
	{
		//判断是否连接网线,如果ip为0.0.0.0，则说明没有连接网线
		if (strcmp(pIpAdapterInfo->IpAddressList.IpAddress.String, "0.0.0.0") != 0)
		{
			//71:无线网卡
			if (MIB_IF_TYPE_ETHERNET == pIpAdapterInfo->Type || 71 == pIpAdapterInfo->Type)
			{
				pNicInfo = new NICINFO;
				TransformNicInfo(pIpAdapterInfo, pNicInfo);
				///////添加到链表
				nicInfoVec.push_back(pNicInfo);
			}
		}
		pIpAdapterInfo = pIpAdapterInfo->Next;
	}

	delete [] pIpAdapterInfo;

	return TRUE;
}

BOOL CNicCtrl::GetNicFriendlyName(NICINFOVector &nicInfoVec)
{
	HANDLE hMprConfig;
	TCHAR szFriendlyName[256];
	CString nicName;
	NICINFO *pNic;
	int size = nicInfoVec.size();
	if (NO_ERROR != MprConfigServerConnect(NULL, &hMprConfig))
	{
		return FALSE;
	}
	for (int i = 0; i < size; i++)
	{
		pNic = nicInfoVec.at(i);
		nicName = _T("\\DEVICE\\TCPIP_") + pNic->AdapterName;//可由GetInterfaceInfo函数获取信息
		if (NO_ERROR == MprConfigGetFriendlyName(hMprConfig, (PWSTR)(LPCTSTR)nicName, szFriendlyName, sizeof(szFriendlyName)))
		{
			pNic->FriendlyName = szFriendlyName;
		}
	}
	MprConfigServerDisconnect(hMprConfig);

	return TRUE;
}

void CNicCtrl::TransformNicInfo(IP_ADAPTER_INFO *pIpAdapterInfo, NICINFO *pNicInfo)
{
	pNicInfo->Index = pIpAdapterInfo->Index;
	pNicInfo->AdapterName = CPublic::MbToWc(pIpAdapterInfo->AdapterName);
	pNicInfo->Description = CPublic::MbToWc(pIpAdapterInfo->Description);
	pNicInfo->DhcpServer = CPublic::MbToWc(pIpAdapterInfo->DhcpServer.IpAddress.String);

	////////ip 使用GetAdaptersInfo获取到的结构体，ip的顺序是相反(不一定)的，最后一个ip才是网卡的第一个ip
	IP_ADDR_STRING *pIpAddr = &(pIpAdapterInfo->IpAddressList);
	while (pIpAddr)
	{
		pNicInfo->IpAddrList.push_back(CPublic::MbToWc(pIpAddr->IpAddress.String));
		pIpAddr = pIpAddr->Next;
	}
	//倒序
	CStringVector::iterator itBegin = pNicInfo->IpAddrList.begin();
	CStringVector::iterator itEnd = pNicInfo->IpAddrList.end() - 1;
	CString tmpStr;
	while (itBegin < itEnd)
	{
		tmpStr = *itBegin;
		*itBegin = *itEnd;
		*itEnd = tmpStr;
		++itBegin;
		--itEnd;
	}

	///////gateway 顺序一致
	pIpAddr = &(pIpAdapterInfo->GatewayList);
	while (pIpAddr)
	{
		pNicInfo->GatewayList.push_back(CPublic::MbToWc(pIpAddr->IpAddress.String));
		pIpAddr = pIpAddr->Next;
	}

	///////dns
	DWORD dwRet;
	ULONG ulOutSize;
	IP_PER_ADAPTER_INFO *pPerAdapterInfo;
	dwRet = GetPerAdapterInfo(pIpAdapterInfo->Index, NULL, &ulOutSize);
	if (ERROR_BUFFER_OVERFLOW == dwRet)
	{
		pPerAdapterInfo = (IP_PER_ADAPTER_INFO *)new char[ulOutSize];
	}
	dwRet = GetPerAdapterInfo(pIpAdapterInfo->Index, pPerAdapterInfo, &ulOutSize);
	if (ERROR_SUCCESS == dwRet)
	{
		IP_ADDR_STRING *pIpAddr = &(pPerAdapterInfo->DnsServerList);
		while (pIpAddr)
		{
			pNicInfo->DnsList.push_back(CPublic::MbToWc(pIpAddr->IpAddress.String));
			pIpAddr = pIpAddr->Next;
		}
	}
	delete [] pPerAdapterInfo;
}

BOOL CNicCtrl::SetNicDnsAddr(LPCTSTR lpszAdapterName, LPCTSTR lpszDns1, LPCTSTR lpszDns2)
{
	CWmi wmi;
	CString param;

	param.Format(_T("SettingID=\"%s\""), lpszAdapterName);
	if (wmi.Init(_T("Win32_NetworkAdapterConfiguration"), param))
	{
		bstr_t szIndex;

		if (0 == wmi.GetObjCount())
		{
			return FALSE;
		}
		szIndex = wmi.GetProp(0, _T("Index"));

		bstr_t szObjPath=_T("Win32_NetworkAdapterConfiguration.Index=");
		szObjPath+=szIndex;

		IWbemClassObject *pSetIn=NULL;
		IWbemClassObject *pSetOut=NULL;

		// SetDNS
		if ( (lpszDns1 && _tcslen(lpszDns1)) && (lpszDns2 && _tcslen(lpszDns2)==0) )
		{
			if (SUCCEEDED(wmi.GetMethod(_T("SetDNSServerSearchOrder"), &pSetIn)))
			{
				SAFEARRAYBOUND Bound[1]={0};
				Bound[0].lLbound = 0;
				Bound[0].cElements = 1;

				//构造参数1

				bstr_t bsIP1=lpszDns1;
				BSTR DNS[1]={bsIP1};
				SAFEARRAY* pArrayDNS=SafeArrayCreate(VT_BSTR,1,Bound);
				pArrayDNS->pvData = DNS;

				VARIANT vtDNS;
				VariantInit(&vtDNS);
				vtDNS.vt=VT_BSTR | VT_ARRAY;
				vtDNS.parray=pArrayDNS;

				//设置参数值
				pSetIn->Put(bstr_t("DNSServerSearchOrder"),  0, &vtDNS,  0);

				//SetDNSServerSearchOrder, 执行函数
				HRESULT hr = wmi.pSvc->ExecMethod(szObjPath, bstr_t("SetDNSServerSearchOrder"), 0, NULL, pSetIn, &pSetOut, NULL);
				if (S_OK == hr)
				{
					return TRUE;
				}
			}
		}
		else if ( (lpszDns1 && _tcslen(lpszDns1)) && (lpszDns2 && _tcslen(lpszDns2)) )
		{
			if (SUCCEEDED(wmi.GetMethod(_T("SetDNSServerSearchOrder"), &pSetIn)))
			{
				SAFEARRAYBOUND Bound[2]={0};//2
				Bound[0].lLbound = 0;
				Bound[0].cElements = 2;//2

				//构造参数1

				bstr_t bsIP1=lpszDns1;
				bstr_t bsIP2=lpszDns2;
				BSTR DNS[2]={bsIP1, bsIP2};//2
				SAFEARRAY* pArrayDNS=SafeArrayCreate(VT_BSTR,1,Bound);
				pArrayDNS->pvData = DNS;

				VARIANT vtDNS;
				VariantInit(&vtDNS);
				vtDNS.vt=VT_BSTR | VT_ARRAY;
				vtDNS.parray=pArrayDNS;

				//设置参数值
				pSetIn->Put(bstr_t("DNSServerSearchOrder"),  0, &vtDNS,  0);

				//SetDNSServerSearchOrder, 执行函数
				HRESULT hr = wmi.pSvc->ExecMethod(szObjPath, bstr_t("SetDNSServerSearchOrder"), 0, NULL, pSetIn, &pSetOut, NULL);
				if (S_OK == hr)
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL CNicCtrl::SetAllNicDnsAddr(LPCTSTR lpszDns1, LPCTSTR lpszDns2)
{
	CStringVector strVec = GetAdapterNames();
	for (CStringVector::iterator it = strVec.begin(); it != strVec.end(); ++it)
	{
		if (!SetNicDnsAddr(*it, lpszDns1, lpszDns2))
		{
			return FALSE;
		}
	}
	return TRUE;
}