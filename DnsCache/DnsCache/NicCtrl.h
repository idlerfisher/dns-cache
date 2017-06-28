#pragma once

class CNicCtrl
{
public:
	CNicCtrl();
	~CNicCtrl();
	BOOL			Init();
	CString			GetAdapterName(int index);
	CStringVector	GetAdapterNames();
	CStringVector	GetFriendlyNames();
	int				GetAdapterCount() {return m_nicInfoVec.size();}
	BOOL			IsEmpty() {return GetAdapterCount()==0 ? TRUE : FALSE;}
	BOOL			SetNicDnsAddr(LPCTSTR lpszAdapterName, LPCTSTR lpszDns1, LPCTSTR lpszDns2);
	BOOL			SetAllNicDnsAddr(LPCTSTR lpszDns1, LPCTSTR lpszDns2);

private:
	BOOL GetNicInfo(NICINFOVector &nicInfoVec);
	void ReleaseNicInfo(NICINFOVector &nicInfoVec);
	BOOL GetNicBaseInfo(NICINFOVector &nicInfoVec);
	BOOL GetNicFriendlyName(NICINFOVector &nicInfoVec);
	void TransformNicInfo(IP_ADAPTER_INFO *pIpAdapterInfo, NICINFO *pNicInfo);

private:
	NICINFOVector m_nicInfoVec;
};

