
// DnsCache.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CDnsCacheApp:
// �йش����ʵ�֣������ DnsCache.cpp
//

class CDnsCacheApp : public CWinApp
{
public:
	CDnsCacheApp();
	~CDnsCacheApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
	BOOL InitWSASocket(BYTE major, BYTE minor);
	void UninitWSASocket(void);
private:
	BOOL m_bInitWSAOk;
};

extern CDnsCacheApp theApp;