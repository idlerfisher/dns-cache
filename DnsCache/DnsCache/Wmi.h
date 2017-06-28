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

	//释放COM对象
	void Release();

	//返回查询到得总数
	int GetObjCount();

	//获取索引指定对象的的属性
	bstr_t GetProp(int iObj, LPCTSTR lpszPropName );

	//获取对象的方法
	HRESULT GetMethod(LPCTSTR lpszMethodName, IWbemClassObject **ppIn);

	//SAFEARRAY数组转bstr_t
	static bstr_t SafeArrayToString(SAFEARRAY *psa);

public:
	IWbemLocator  *pLoc;
	IWbemServices *pSvc;

	IEnumWbemClassObject  *pclsEnum;

	IWbemClassObject *pclsObj;
	vector<IWbemClassObject *> clslist;
};
#endif //_WMI_H_