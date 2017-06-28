
// DnsCache.cpp : 定义应用程序的类行为。
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


// CDnsCacheApp 构造

CDnsCacheApp::CDnsCacheApp()
	: m_bInitWSAOk(FALSE)
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
	m_bInitWSAOk = FALSE;
}

CDnsCacheApp::~CDnsCacheApp()
{
	UninitWSASocket();
}

// 唯一的一个 CDnsCacheApp 对象

CDnsCacheApp theApp;


// CDnsCacheApp 初始化

BOOL CDnsCacheApp::InitInstance()
{
	//判断应用程序单例
	HANDLE hMutex = CreateMutex(NULL, FALSE, APP_GLOBAL_MUTEX);
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		// 寻找先前实例的主窗口
		HWND hWndPrevious = ::GetWindow(::GetDesktopWindow(), GW_CHILD);
		while (::IsWindow(hWndPrevious))
		{
			// 检查窗口是否有预设的标记?
			// 有，则是我们寻找的主窗
			if (APP_GLOBAL_DATA == ::GetProp(hWndPrevious, APP_GLOBAL_PROP))
			{
				//显示
				if (::IsWindowVisible(hWndPrevious))
				{
					if (::IsIconic(hWndPrevious))
					{
						::ShowWindow(hWndPrevious, SW_RESTORE);//还原
					}
					else
					{
						::SetForegroundWindow(hWndPrevious);//激活
					}
				}//隐藏
				else
				{
					::ShowWindow(hWndPrevious, SW_SHOW);
				}

				// 退出本实例
				return FALSE;
			}

			// 继续寻找下一个窗口
			hWndPrevious = ::GetWindow(hWndPrevious, GW_HWNDNEXT);
		}
	}

	if (!InitWSASocket(2, 2))
	{
		return FALSE;
	}
	m_bInitWSAOk = TRUE;

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CDnsCacheDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	CloseHandle(hMutex);
	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
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
