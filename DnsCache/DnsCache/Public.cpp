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

//ֱ��ָ��������ַ
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

	//Ŀǰֻ����һ��Question�����
	if (1 != nQuestions || 0 == nAnswers)
	{
		return strIpVec;
	}

	//����DNSͷ��Questionͷ
	ptr += sizeof(DNS_HDR);//����Questionͷָ��
	while (*ptr++);//�ƶ���Question������β,ָ��Type�׵�ַ
	ptr += nQuestions * sizeof(Question);
	
	//ָ��Answer������ͷָ��
	for (int i = 0; i < nAnswers; i++)
	{
		//��һ����ѹ��ָ�룬���ж��Ƿ���ѹ��ָ��
		if (IsCompressedFlag(ptr, NULL))
		{
			ptr += 2;
		}
		else
		{
			while (*ptr++);//��������
		}

		pResource = (Resource *)ptr;
		ptr += sizeof(Resource);//����Resource�ռ�

		//��ip��ַ
		if (A == ntohs(pResource->type))
		{
			dwIp = ntohl(*(DWORD *)ptr);
			strIpVec.push_back(CPublic::DwToIpAddr(dwIp));//��ip��ַ��������
			ptr += 4;//�ƶ�32λip��ַ�ռ�
		}//����ip��ַ
		else
		{
			//����len���ȣ����ɵ�����һ��Answers����ͷָ��
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
	char			*ptr;//��̬�ƶ�
	USHORT			nQuestions, nAnswers;
	DWORD			dwIp;

	string strDomain;
	int iSeek;

	ptr = (char *)buf;
	pDnsHdr = (DNS_HDR *)buf;
	nQuestions = ntohs(pDnsHdr->questions);
	nAnswers = ntohs(pDnsHdr->answers);

	//Ŀǰֻ����һ��Question�����
	if (1 != nQuestions)
	{
		return strIpVec;
	}

	//����DNSͷ��Questionͷ
	ptr += sizeof(DNS_HDR);//����Questionͷָ��
	while (*ptr++);//�ƶ���Question������β,ָ��Type�׵�ַ
	ptr += sizeof(Question);
	
	//ѭ������N�� ָ��Answer������ͷָ��
	for (int i = 0; i < nAnswers; i++)
	{
		strDomain = GetDomain(buf, ptr, &iSeek);
		strIpVec.push_back(CPublic::MbToWc(strDomain));
		//AfxMessageBox(CPublic::MbToWc(strDomain));//test
		ptr += iSeek;

		pResource = (Resource *)ptr;
		ptr += sizeof(Resource);//����Resource�ռ�

		//��ip��ַ
		if (A == ntohs(pResource->type))
		{
			dwIp = ntohl(*(DWORD *)ptr);
			ptr += 4;//�ƶ�
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

//first:dns�׵�ַ
//start:��ȡ�������׵�ַ
string CPublic::GetDomain(const char *first, const char *start, int *pLen)
{
	char *p = (char *)start;
	char *tmp;
	int iLen = 0;
	int iSeek = 0;
	string strDomain;

	//��ȡ��ѹ������ָ�������
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

			//������ѹ��ָ�룬��ѭ������ѹ��ָ����ָ������
			if (IsCompressedFlag(p, NULL))
			{
				break;
			}
			//���������ַ�����˵�������Ѿ�������ֱ�ӷ���
			if (!*p)
			{
				p++;//��Ϊ��ѹ�����ƺ��滹��һ�����ַ�
				if (pLen)
				{
					*pLen = p-start;
				}
				return strDomain;//����
			}
		}
	}

	//��ȡѹ������ָ�������
	tmp = p;//p�Ѿ��ı�
	do
	{
		if (IsCompressedFlag(tmp, &iSeek))
		{
			tmp = (char *)first + iSeek;//ָ��ѹ������

			while (1)
			{
				iLen = *tmp++;
				for (int i = 0; i < iLen; i++)
				{
					strDomain += *tmp++;
				}
				//��tmp+1�������ݣ������'.'
				if (*tmp)
				{
					strDomain += '.';
				}

				//������ѹ��ָ�룬��ѭ������ѹ��ָ����ָ������
				if (!*tmp || IsCompressedFlag(tmp, NULL))
				{
					break;
				}
			}
		}
	} while (*tmp);
	p += 2;//����ѹ��ָ��

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

	//Ŀǰֻ����һ��Question�����,����������һ��answer
	if (1 != nQuestions && 0 != nAnswers)
	{
		return -1;
	}

	//����DNSͷ��Questionͷ
	ptr += sizeof(DNS_HDR);//����Questionͷָ��
	while (*ptr++);//�ƶ���Question������β,ָ��Type�׵�ַ
	ptr += sizeof(Question);

	//��һ����ѹ��ָ�룬���ж��Ƿ���ѹ��ָ��
	if (IsCompressedFlag(ptr, NULL))
	{
		ptr += 2;
	}
	else
	{
		while (*ptr++);//��������
	}

	pResource = (Resource *)ptr;

	return ntohl(pResource->ttl);
}

//�ж�dns����
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

	//Ŀǰֻ����һ��Question�����
	if (1 != nQuestions)
	{
		return ANY_Type;//��㷵��һ����Aֵ
	}

	//����DNSͷ��Questionͷ
	ptr += sizeof(DNS_HDR);//����Questionͷָ��
	while (*ptr++);//�ƶ���Question������β,ָ��Type�׵�ַ
	pQuestion = (Question *)ptr;

	return ntohs(pQuestion->type);
}

CString CPublic::DwToIpAddr(DWORD dwIp)
{
	CString str;
	str.Format(_T("%d.%d.%d.%d"), FIRST_IPADDRESS(dwIp), SECOND_IPADDRESS(dwIp), THIRD_IPADDRESS(dwIp), FOURTH_IPADDRESS(dwIp));
	return str;
}

//��Ϊ�棬����seek��Ϊ�գ�����ѹ��ָ��λ��
BOOL CPublic::IsCompressedFlag(const char *buf, int *seek)
{
	WORD flag = ntohs(*(WORD *)buf);
	//if (((flag>>8) & 0xff) == 0xc0)
	if (flag & 0xc000)
	{
		if (seek)
		{
			*seek = flag & 0x3fff;//ʣ�µ�14λ����ѹ������ָ��λ��
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