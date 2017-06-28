
// DnsCache.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "DnsCache.h"
#include "DnsCacheDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDnsCacheApp

BEGIN_MESSAGE_MAP(CDnsCacheApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDnsCacheApp ����

CDnsCacheApp::CDnsCacheApp()
	: m_bInitWSAOk(FALSE)
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
	m_bInitWSAOk = FALSE;
}

CDnsCacheApp::~CDnsCacheApp()
{
	UninitWSASocket();
}

// Ψһ��һ�� CDnsCacheApp ����

CDnsCacheApp theApp;


// CDnsCacheApp ��ʼ��

BOOL CDnsCacheApp::InitInstance()
{
	//�ж�Ӧ�ó�����
	HANDLE hMutex = CreateMutex(NULL, FALSE, APP_GLOBAL_MUTEX);
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		// Ѱ����ǰʵ����������
		HWND hWndPrevious = ::GetWindow(::GetDesktopWindow(), GW_CHILD);
		while (::IsWindow(hWndPrevious))
		{
			// ��鴰���Ƿ���Ԥ��ı��?
			// �У���������Ѱ�ҵ�����
			if (APP_GLOBAL_DATA == ::GetProp(hWndPrevious, APP_GLOBAL_PROP))
			{
				//��ʾ
				if (::IsWindowVisible(hWndPrevious))
				{
					if (::IsIconic(hWndPrevious))
					{
						::ShowWindow(hWndPrevious, SW_RESTORE);//��ԭ
					}
					else
					{
						::SetForegroundWindow(hWndPrevious);//����
					}
				}//����
				else
				{
					::ShowWindow(hWndPrevious, SW_SHOW);
				}

				// �˳���ʵ��
				return FALSE;
			}

			// ����Ѱ����һ������
			hWndPrevious = ::GetWindow(hWndPrevious, GW_HWNDNEXT);
		}
	}

	if (!InitWSASocket(2, 2))
	{
		return FALSE;
	}
	m_bInitWSAOk = TRUE;

	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CDnsCacheDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	CloseHandle(hMutex);
	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}



BOOL CDnsCacheApp::InitWSASocket(BYTE major, BYTE minor)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(major, minor);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return FALSE;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE(wsaData.wVersion) != major || HIBYTE(wsaData.wVersion) != minor) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
		return FALSE;
	}
	else
		printf("The Winsock 2.2 dll was found okay\n");

	return TRUE;
}


void CDnsCacheApp::UninitWSASocket(void)
{
	if (m_bInitWSAOk)
	{
		WSACleanup();
	}
}
