#ifndef _WMI_H_
#define _WMI_H_

#define _WIN32_DCOM
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#include <vector>
#include <string>
using namespace std;

class CWmi
{
public:

	CWmi();
	~CWmi();

	bool Init(LPCTSTR lpszWMIObj, LPCTSTR lpszWhere=NULL);

	//�ͷ�COM����
	void Release();

	//���ز�ѯ��������
	int GetObjCount();

	//��ȡ����ָ������ĵ�����
	bstr_t GetProp(int iObj, LPCTSTR lpszPropName );

	//��ȡ����ķ���
	HRESULT GetMethod(LPCTSTR lpszMethodName, IWbemClassObject **ppIn);

	//SAFEARRAY����תbstr_t
	static bstr_t SafeArrayToString(SAFEARRAY *psa);

public:
	IWbemLocator  *pLoc;
	IWbemServices *pSvc;

	IEnumWbemClassObject  *pclsEnum;

	IWbemClassObject *pclsObj;
	vector<IWbemClassObject *> clslist;
};
#endif //_WMI_H_