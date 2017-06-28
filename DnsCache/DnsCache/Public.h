#pragma once

class CPublic
{
public:
	CPublic();
	~CPublic();

	static CString			MbToWc(const string &str);
	static CString			MbToWc(LPCSTR multiByte);
	static string			WcToMb(LPCTSTR wideChar);
	static string			ToHex(const string &str);
	static string			FromHex(const string &hex);
	static CString			SizeToFriendlyName(DWORD size);
	static CString			GetDomainFromDnsBuf(const char *buf);
	static CStringVector	GetIpFromDnsBuf(const char *buf);
	static CStringVector	GetIpStrFromDnsBuf(const char *buf);
	static int				GetTtlFromDnsBuf(const char *buf);
	static USHORT			GetQuestionType(const char *buf);
	static CString			DwToIpAddr(DWORD dwIp);
	static BOOL				IsCompressedFlag(const char *buf, int *seek);//c0 xx
	static CStringVector	Split(const CString &str, TCHAR ch);
	static StringVector		Split(const string &str, char ch);
	static CString			MergeVec(const CStringVector &strVec, TCHAR sep);
	static CString			IntToStr(int n);
	static void				ProcessEvent();
	static CString			GetAppAbsoluteFileName();
	static CString			GetAppCurrentDirectory();
	static string			GetDomain(const char *first, const char *start, int *pLen=NULL);
};