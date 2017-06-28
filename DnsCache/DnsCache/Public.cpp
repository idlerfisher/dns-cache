#include "StdAfx.h"
#include "Public.h"


CPublic::CPublic(void)
{
}


CPublic::~CPublic(void)
{
}

CString CPublic::MbToWc(const string &str)
{
	return MbToWc(str.c_str());
}

CString CPublic::MbToWc(LPCSTR multiByte)
{
	CString wideChar;
	LPWSTR lpWideCharStr = NULL;
	int len = MultiByteToWideChar(CP_ACP, 0, multiByte, -1, NULL, 0);
	lpWideCharStr = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, multiByte, -1, lpWideCharStr, len);
	wideChar = lpWideCharStr;
	HeapFree(GetProcessHeap(), 0, lpWideCharStr);
	return wideChar;
}

string CPublic::WcToMb(LPCTSTR wideChar)
{
	string multiByte;
	LPSTR lpMultiByteStr = NULL;
	int len = WideCharToMultiByte(CP_ACP, 0, wideChar, -1, lpMultiByteStr, 0, NULL, NULL);
	lpMultiByteStr = (LPSTR)HeapAlloc(GetProcessHeap(), 0, len);
	WideCharToMultiByte(CP_ACP, 0, wideChar, -1, lpMultiByteStr, len, NULL, NULL);
	multiByte = lpMultiByteStr;
	HeapFree(GetProcessHeap(), 0, lpMultiByteStr);
	return multiByte;
}

string CPublic::ToHex(const string &str)
{
	int j = 0;
	int size = str.size();
	string hex(size * 2, 0);
	char *hexData = (char *)(hex.c_str());
	const unsigned char *data = (const unsigned char *)str.c_str();
	for (int i = 0; i < size; ++i)
	{
		j = (data[i] >> 4) & 0xf;
		if (j <= 9)
			hexData[i*2] = (j + '0');
		else
			hexData[i*2] = (j + 'a' - 10);
		j = data[i] & 0xf;
		if (j <= 9)
			hexData[i*2+1] = (j + '0');
		else
			hexData[i*2+1] = (j + 'a' - 10);
	}
	return hex;
}

string CPublic::FromHex(const string &hex)
{
	string res((hex.size() + 1)/ 2, 0);
	unsigned char *result = (unsigned char *)res.c_str() + res.size();

	bool odd_digit = true;
	for (int i = hex.size() - 1; i >= 0; --i)
	{
		int ch = hex.at(i);
		int tmp;
		if (ch >= '0' && ch <= '9')
			tmp = ch - '0';
		else if (ch >= 'a' && ch <= 'f')
			tmp = ch - 'a' + 10;
		else if (ch >= 'A' && ch <= 'F')
			tmp = ch - 'A' + 10;
		else
			continue;
		if (odd_digit) {
			--result;
			*result = tmp;
			odd_digit = false;
		} else {
			*result |= tmp << 4;
			odd_digit = true;
		}
	}

	res.erase(0, result - (const unsigned char *)(res.c_str()));
	return res;
}

CString CPublic::SizeToFriendlyName(DWORD size)
{
	CString strTmp;
	if (size >= 0 && size < 1024)
	{
		strTmp.Format(_T("%lu B"), size);
	}
	else if (size >= 1024 && size < 1024*1024)
	{
		strTmp.Format(_T("%.2f KB"), (float)size/1024);
	}
	else if (size >= 1024*1024 && size < 1024*1024*1024)
	{
		strTmp.Format(_T("%.2f MB"), (float)size/1024/1024);
	}
	else if (size >= 1024*1024*1024 && size < 1024*1024*1024)
	{
		strTmp.Format(_T("%.2f GB"), (float)size/1024/1024/1024);
	}
	return strTmp;
}

//直接指向域名地址
CString CPublic::GetDomainFromDnsBuf(const char *buf)
{
	char name[65];
	char *p = (char *)buf;
	int len = 0;
	int offset = 0;

	p += sizeof(DNS_HDR);
	while (*p)
	{
		len = *p++;
		for (int i = 0; i < len; i++)
		{
			name[offset++] = *p++;
		}
		//Null-Terminated
		if (*p)
		{
			name[offset++] = '.';
		}
	}
	name[offset] = 0;

	return MbToWc(name);
}

CStringVector CPublic::GetIpFromDnsBuf(const char *buf)
{
	CStringVector strIpVec;
	DNS_HDR		*pDnsHdr;
	Resource	*pResource;
	char		*ptr;
	int			nQuestions, nAnswers;
	DWORD		dwIp;

	ptr = (char *)buf;
	pDnsHdr = (DNS_HDR *)buf;
	nQuestions = ntohs(pDnsHdr->questions);
	nAnswers = ntohs(pDnsHdr->answers);

	//目前只解析一个Question的情况
	if (1 != nQuestions || 0 == nAnswers)
	{
		return strIpVec;
	}

	//跳过DNS头与Question头
	ptr += sizeof(DNS_HDR);//到达Question头指针
	while (*ptr++);//移动到Question的域名尾,指向Type首地址
	ptr += nQuestions * sizeof(Question);
	
	//指向Answer的域名头指针
	for (int i = 0; i < nAnswers; i++)
	{
		//不一定是压缩指针，先判断是否是压缩指针
		if (IsCompressedFlag(ptr, NULL))
		{
			ptr += 2;
		}
		else
		{
			while (*ptr++);//跳过域名
		}

		pResource = (Resource *)ptr;
		ptr += sizeof(Resource);//跳过Resource空间

		//是ip地址
		if (A == ntohs(pResource->type))
		{
			dwIp = ntohl(*(DWORD *)ptr);
			strIpVec.push_back(CPublic::DwToIpAddr(dwIp));//将ip地址保存下来
			ptr += 4;//移动32位ip地址空间
		}//不是ip地址
		else
		{
			//跳过len长度，即可到达下一个Answers域名头指针
			ptr += ntohs(pResource->len);
		}
	}

	return strIpVec;
}

CStringVector CPublic::GetIpStrFromDnsBuf(const char *buf)
{
	CStringVector	strIpVec;
	DNS_HDR			*pDnsHdr;
	Resource		*pResource;
	char			*ptr;//动态移动
	USHORT			nQuestions, nAnswers;
	DWORD			dwIp;

	string strDomain;
	int iSeek;

	ptr = (char *)buf;
	pDnsHdr = (DNS_HDR *)buf;
	nQuestions = ntohs(pDnsHdr->questions);
	nAnswers = ntohs(pDnsHdr->answers);

	//目前只解析一个Question的情况
	if (1 != nQuestions)
	{
		return strIpVec;
	}

	//跳过DNS头与Question头
	ptr += sizeof(DNS_HDR);//到达Question头指针
	while (*ptr++);//移动到Question的域名尾,指向Type首地址
	ptr += sizeof(Question);
	
	//循环解析N个 指向Answer的域名头指针
	for (int i = 0; i < nAnswers; i++)
	{
		strDomain = GetDomain(buf, ptr, &iSeek);
		strIpVec.push_back(CPublic::MbToWc(strDomain));
		//AfxMessageBox(CPublic::MbToWc(strDomain));//test
		ptr += iSeek;

		pResource = (Resource *)ptr;
		ptr += sizeof(Resource);//跳过Resource空间

		//是ip地址
		if (A == ntohs(pResource->type))
		{
			dwIp = ntohl(*(DWORD *)ptr);
			ptr += 4;//移动
			strIpVec.push_back(DwToIpAddr(dwIp));
			//AfxMessageBox(DwToIpAddr(dwIp));//test
		}//cname
		else if (CNAME == ntohs(pResource->type))
		{
			strDomain = GetDomain(buf, ptr, &iSeek);
			strIpVec.push_back(CPublic::MbToWc(strDomain));
			//AfxMessageBox(CPublic::MbToWc(strDomain));//test
			ptr += iSeek;
		}
		strIpVec.push_back(_T(" "));
	}

	return strIpVec;
}

//first:dns首地址
//start:读取域名的首地址
string CPublic::GetDomain(const char *first, const char *start, int *pLen)
{
	char *p = (char *)start;
	char *tmp;
	int iLen = 0;
	int iSeek = 0;
	string strDomain;

	//获取非压缩名称指针的内容
	if (!IsCompressedFlag(p, NULL))
	{
		while (1)
		{
			iLen = *p++;
			for (int i = 0; i < iLen; i++)
			{
				strDomain += *p++;
			}
			if (*p)
			{
				strDomain += '.';
			}

			//若遇到压缩指针，则循环遍历压缩指针所指的名称
			if (IsCompressedFlag(p, NULL))
			{
				break;
			}
			//若遇到空字符，则说明域名已经结束，直接返回
			if (!*p)
			{
				p++;//因为非压缩名称后面还有一个空字符
				if (pLen)
				{
					*pLen = p-start;
				}
				return strDomain;//返回
			}
		}
	}

	//获取压缩名称指针的内容
	tmp = p;//p已经改变
	do
	{
		if (IsCompressedFlag(tmp, &iSeek))
		{
			tmp = (char *)first + iSeek;//指向压缩名称

			while (1)
			{
				iLen = *tmp++;
				for (int i = 0; i < iLen; i++)
				{
					strDomain += *tmp++;
				}
				//若tmp+1还有内容，则添加'.'
				if (*tmp)
				{
					strDomain += '.';
				}

				//若遇到压缩指针，则循环遍历压缩指针所指的名称
				if (!*tmp || IsCompressedFlag(tmp, NULL))
				{
					break;
				}
			}
		}
	} while (*tmp);
	p += 2;//跳过压缩指针

	if (pLen)
	{
		*pLen = p - start;
	}
	return strDomain;
}

int CPublic::GetTtlFromDnsBuf(const char *buf)
{
	DNS_HDR		*pDnsHdr;
	Resource	*pResource;
	USHORT		nQuestions;
	USHORT		nAnswers;
	char		*ptr;

	ptr = (char *)buf;
	pDnsHdr = (DNS_HDR *)ptr;
	nQuestions = ntohs(pDnsHdr->questions);
	nAnswers = ntohs(pDnsHdr->answers);

	//目前只解析一个Question的情况,并且至少有一个answer
	if (1 != nQuestions && 0 != nAnswers)
	{
		return -1;
	}

	//跳过DNS头与Question头
	ptr += sizeof(DNS_HDR);//到达Question头指针
	while (*ptr++);//移动到Question的域名尾,指向Type首地址
	ptr += sizeof(Question);

	//不一定是压缩指针，先判断是否是压缩指针
	if (IsCompressedFlag(ptr, NULL))
	{
		ptr += 2;
	}
	else
	{
		while (*ptr++);//跳过域名
	}

	pResource = (Resource *)ptr;

	return ntohl(pResource->ttl);
}

//判断dns请求
USHORT CPublic::GetQuestionType(const char *buf)
{
	DNS_HDR		*pDnsHdr;
	Question	*pQuestion;
	USHORT		nQuestions;
	USHORT		nAnswers;
	char		*ptr;

	ptr = (char *)buf;
	pDnsHdr = (DNS_HDR *)ptr;
	nQuestions = ntohs(pDnsHdr->questions);
	nAnswers = ntohs(pDnsHdr->answers);

	//目前只解析一个Question的情况
	if (1 != nQuestions)
	{
		return ANY_Type;//随便返回一个非A值
	}

	//跳过DNS头与Question头
	ptr += sizeof(DNS_HDR);//到达Question头指针
	while (*ptr++);//移动到Question的域名尾,指向Type首地址
	pQuestion = (Question *)ptr;

	return ntohs(pQuestion->type);
}

CString CPublic::DwToIpAddr(DWORD dwIp)
{
	CString str;
	str.Format(_T("%d.%d.%d.%d"), FIRST_IPADDRESS(dwIp), SECOND_IPADDRESS(dwIp), THIRD_IPADDRESS(dwIp), FOURTH_IPADDRESS(dwIp));
	return str;
}

//若为真，并且seek不为空，返回压缩指针位移
BOOL CPublic::IsCompressedFlag(const char *buf, int *seek)
{
	WORD flag = ntohs(*(WORD *)buf);
	//if (((flag>>8) & 0xff) == 0xc0)
	if (flag & 0xc000)
	{
		if (seek)
		{
			*seek = flag & 0x3fff;//剩下的14位就是压缩名称指针位移
		}
		return TRUE;
	}
	return FALSE;
}

CStringVector CPublic::Split(const CString &str, TCHAR ch)
{
	int index;
	CStringVector strVec;
	CString srcStr = str;
	CString tmpStr;

	while (1)
	{
		index = srcStr.Find(ch);
		if (-1 == index)
		{
			if (!srcStr.IsEmpty())
			{
				strVec.push_back(srcStr);
			}
			break;
		}
		else
		{
			tmpStr = srcStr.Left(index);
			if (!tmpStr.IsEmpty())
			{
				strVec.push_back(tmpStr);
			}
			srcStr = srcStr.Right(srcStr.GetLength() - index - 1);
		}
	}
	return strVec;
}

StringVector CPublic::Split(const string &str, char ch)
{
	int index;
	StringVector strVec;
	string strSrc = str;
	string strTmp;

	while (1)
	{
		index = strSrc.find(ch);
		if (string::npos == index)
		{
			if (!strSrc.empty())
			{
				strVec.push_back(strSrc);
			}
			break;
		}
		else
		{
			strTmp = strSrc.substr(0, index);
			if (!strTmp.empty())
			{
				strVec.push_back(strTmp);
			}
			if (index+1 == strSrc.size())
			{
				break;
			}
			strSrc = strSrc.substr(index+1, strSrc.size() - index - 1);
		}
	}
	return strVec;
}

CString CPublic::MergeVec(const CStringVector &strVec, TCHAR sep)
{
	int size = strVec.size();
	CString strIp;
	for (int i = 0; i < size; i++)
	{
		strIp += strVec.at(i);
		if (i != size - 1)
		{
			strIp += sep;
		}
	}
	return strIp;
}

CString CPublic::IntToStr(int n)
{
	CString strTmp;
	strTmp.Format(_T("%d"), n);
	return strTmp;
}

void CPublic::ProcessEvent()
{
	MSG msg;
	if (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

CString CPublic::GetAppAbsoluteFileName()
{
	TCHAR buf[MAX_PATH];
	if (0 != GetModuleFileName(NULL, buf, sizeof(buf)))
	{
		return CString(buf);
	}
	return _T("");
}

CString CPublic::GetAppCurrentDirectory()
{
	CString appFileName = GetAppAbsoluteFileName();
	int pos = appFileName.ReverseFind(_T('\\'));
	return appFileName.Left(pos);
}