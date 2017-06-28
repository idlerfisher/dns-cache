#include "StdAfx.h"
#include "DnsParser.h"
#include "Public.h"

CDnsParser::CDnsParser()
{
	Clear();
}

CDnsParser::CDnsParser(const char *ptr, int len)
{
	Clear();
	m_strDnsBuf = string(ptr, len);
}

CDnsParser::~CDnsParser()
{
	Clear();
}

void CDnsParser::SetDnsBuf(const char *ptr, int len)
{
	Clear();
	m_strDnsBuf = string(ptr, len);
}

//成功：内存搜索、仿真、转发
//失败：转发
bool CDnsParser::StartParse()
{
	int dnsLen = m_strDnsBuf.size();
	m_pDnsHdr = (DNS_HDR *)m_strDnsBuf.c_str();

	//获取dns头信息
	if (!StartParseDnsHeader(m_pDnsHdr))
	{
		return false;
	}

	//获取dns请求信息:Question
	StartParseDnsQuestion(m_pDnsHdr);
	
	//遍历获取所有dns响应:Answer
	StartParseDnsAnswer(m_pDnsHdr);

	return true;
}

bool CDnsParser::IsRequest()
{
	return (0 == (m_DnsHdr.flags & DNS_FLAG_QR));
}

bool CDnsParser::IsResponce()
{
	return (1 == (m_DnsHdr.flags & DNS_FLAG_QR));
}

USHORT CDnsParser::GetDnsId()
{
	return m_DnsHdr.id;
}

USHORT CDnsParser::GetDnsFlag()
{
	return m_DnsHdr.flags;
}

USHORT CDnsParser::GetQuestions()
{
	return m_DnsHdr.questions;
}

USHORT CDnsParser::GetAnswers()
{
	return m_DnsHdr.answers;
}

USHORT CDnsParser::GetAuthority()
{
	return m_DnsHdr.authority;
}

USHORT CDnsParser::GetAdditional()
{
	return m_DnsHdr.additional;
}

USHORT CDnsParser::GetFlagQR()
{
	 return m_DnsHdr.flags & DNS_FLAG_QR;
}

USHORT CDnsParser::GetFlagOPCODE()
{
	return m_DnsHdr.flags & DNS_FLAG_OPCODE;
}

USHORT CDnsParser::GetFlagAA()
{
	return m_DnsHdr.flags & DNS_FLAG_AA;
}

USHORT CDnsParser::GetFlagTC()
{
	return m_DnsHdr.flags & DNS_FLAG_TC;
}

USHORT CDnsParser::GetFlagRD()
{
	return m_DnsHdr.flags & DNS_FLAG_RD;
}

USHORT CDnsParser::GetFlagRA()
{
	return m_DnsHdr.flags & DNS_FLAG_RA;
}

USHORT CDnsParser::GetFlagZ()
{
	return m_DnsHdr.flags & DNS_FLAG_Z;
}

USHORT CDnsParser::GetFlagRCODE()
{
	return m_DnsHdr.flags & DNS_FLAG_RCODE;
}

bool CDnsParser::IsIpv4()
{
	return (A == m_Question.question.type);
}

bool CDnsParser::IsIpv6()
{
	return (AAAA == m_Question.question.type);
}

string CDnsParser::QuestionDomain()
{
	return m_Question.domain;
}

QType CDnsParser::QuestionType()
{
	return (QType)m_Question.question.type;
}

QClass CDnsParser::QuestionClass()
{
	return (QClass)m_Question.question.cls;
}

StringList CDnsParser::GetDomainList()
{
	StringList strList;

	SAnswer *pAnswer;
	for (SAnswerList::iterator it = m_AnswerList.begin(); it != m_AnswerList.end(); ++it)
	{
		pAnswer = *it;
		if (CNAME == pAnswer->resource.type)
		{
			strList.push_back(pAnswer->result);
		}
	}

	return strList;
}

StringList CDnsParser::GetIpList()
{
	StringList strList;

	SAnswer *pAnswer;
	for (SAnswerList::iterator it = m_AnswerList.begin(); it != m_AnswerList.end(); ++it)
	{
		pAnswer = *it;
		if (A == pAnswer->resource.type)
		{
			strList.push_back(pAnswer->result);
		}
	}

	return strList;
}

void CDnsParser::PrintLog()
{
	string strLog;
	char buf[256];
	SAnswer *pAnswer;

	//网址
	sprintf_s(buf, sizeof(buf), "[%s]\n\n", m_Question.domain.c_str());
	strLog += buf;

	//dns头信息
	sprintf_s(buf, sizeof(buf), "[DNS]:\nIdentification:\t0x%04x\nDns Flags:\t0x%04x\nQuestions:\t%d\nAnswers:\t\t%d\nAuthority:\t\t%d\nAdditional:\t%d\n\n", 
		m_DnsHdr.id, m_DnsHdr.flags, m_DnsHdr.questions, m_DnsHdr.answers, m_DnsHdr.authority, m_DnsHdr.additional);
	strLog += buf;

	//请求报文信息Question
	sprintf_s(buf, sizeof(buf), "[Question]:\nDomain Name:\t%s\nType:\t\t%d  %s\nClass:\t\t%d  %s\n\n", m_Question.domain.c_str(), m_Question.question.type, ConvertDnsType(m_Question.question.type), m_Question.question.cls, ConvertDnsClass(m_Question.question.cls));
	strLog += buf;

	//响应报文信息Answer
	for (SAnswerList::iterator it = m_AnswerList.begin(); it != m_AnswerList.end(); ++it)
	{
		pAnswer = *it;
		sprintf_s(buf, sizeof(buf), "[Answer]:\nDomain Name:\t%s\nType:\t\t%d  %s\nClass:\t\t%d  %s\nTime to Live:\t%d\nData Length:\t%d\nDomain Name:\t%s\n\n",
			pAnswer->domain.c_str(), pAnswer->resource.type, ConvertDnsType(pAnswer->resource.type), pAnswer->resource.cls, ConvertDnsClass(pAnswer->resource.cls), pAnswer->resource.ttl, pAnswer->resource.len, pAnswer->result.c_str());
		strLog += buf;
	}

	CString cstrLog = CPublic::MbToWc(strLog);
	AfxMessageBox(cstrLog);
}

string CDnsParser::EmulateDnsHostResponce(const string &host, const string &req)
{
	DNS_HDR *pDnsHdr;
	string strDomain;
	char res[1024];
	char *pRes = res;
	int resLen;
	USHORT ptrSeek;//压缩内容的指针位移:1100000000001100
	
	//生成dns格式的域名串
	strDomain = CreateDnsFormatDomain(host);

	/******开始制作自定义dns响应包*******/
	memset(res, 0, sizeof(res));
	memcpy(pRes, req.c_str(), req.size());//拷贝请求报文

	//修改dns头信息
	pDnsHdr = (DNS_HDR *)pRes;
	pDnsHdr->flags = htons(0x8180);//标志: 0x8180
	pDnsHdr->answers = htons(1);//回答个数: 1

	//压缩名称指针：0xc00c 2字节

	//增加并填充一个资源记录(回答)
	Resource resource;
	memset(&resource, 0, sizeof(Resource));
	resource.type = htons(PTR);
	resource.cls = htons(INTERNET);
	resource.ttl = htonl(10000);
	resource.len = htons(strDomain.size());

	//[DNS协议]
	//操作Answer资源记录
	pRes += req.size();//指向存储Answer资源的首地址(Domain Name:这里使用压缩指针)

	ptrSeek = htons(0xc00c);//转换成网络字节序
	memcpy(pRes, &ptrSeek, sizeof(USHORT));//填充压缩指针
	pRes += sizeof(USHORT);//指向Answer实际定义的结构地址

	memcpy(pRes, &resource, sizeof(Resource));//填充回答资源记录
	pRes += sizeof(Resource);

	memcpy(pRes, strDomain.c_str(), strDomain.size());

	//DNS协议头_问题资源头 + 压缩指针 + 回答资源头 + 域名数组
	resLen = req.size() + 2 + sizeof(Resource) + strDomain.size();//响应报文长度

	return string(res, resLen);
}

string CDnsParser::EmulateIpv6Responce(const string &req)
{
	DNS_HDR *pDnsHdr;
	char res[1024];
	char *pRes = res;
	int resLen;
	USHORT ptrSeek;//压缩内容的指针位移:1100000000001100

	/******开始制作自定义dns响应包*******/
	memset(res, 0, sizeof(res));
	memcpy(pRes, req.c_str(), req.size());//拷贝请求报文

	//修改dns头信息
	pDnsHdr = (DNS_HDR *)pRes;
	pDnsHdr->flags = htons(0x8180);//标志: 0x8180
	pDnsHdr->answers = htons(1);//回答个数: 1

	//压缩名称指针：0xc00c 2字节

	//增加并填充一个资源记录(回答)
	Resource resource;
	resource.type = htons(CNAME);
	resource.cls = htons(INTERNET);
	resource.ttl = htonl(10000);
	resource.len = htons(2);

	//压缩名称指针：0xc00c 2字节

	//[DNS协议]
	//操作Answer资源记录
	pRes += req.size();//指向存储Answer资源的首地址(Domain Name:这里使用压缩指针)

	ptrSeek = htons(0xc00c);//转换成网络字节序
	memcpy(pRes, &ptrSeek, sizeof(USHORT));//填充压缩指针
	pRes += sizeof(USHORT);//指向Answer实际定义的结构地址

	memcpy(pRes, &resource, sizeof(Resource));//填充回答资源记录
	pRes += sizeof(Resource);

	ptrSeek = htons(0xc00c);//转换成网络字节序
	memcpy(pRes, &ptrSeek, sizeof(USHORT));//填充压缩指针

	//DNS协议头_问题资源头 + 压缩指针 + 回答资源头 + 域名数组
	resLen = req.size() + 2 + sizeof(Resource) + 2;//响应报文长度

	return string(res, resLen);
}

string CDnsParser::CreateDnsFormatDomain(const string &name)
{
	//生成dns协议格式的字符串
	//example:
	//www.baidu.com
	//03 www 05 baidu 03 com  占用15
	//存储格式:03 www 09 hywebtech 03 com 00
	string strDomain;
	StringVector strVec;

	strVec = CPublic::Split(name, '.');
	for (UINT i = 0; i < strVec.size(); i++)
	{
		string &r = strVec[i];
		strDomain += r.size();//分割后的字符串长度
		strDomain += r;
	}
	strDomain += '\0';//最后必须要加0

	return strDomain;
}

char *CDnsParser::ConvertDnsType(USHORT type)
{
	char *res = "NOTEXISTS";

	switch ((QType)type)
	{
	case A:
		res = "A";
		break;
	case NS:
		res = "NS";
		break;
	case MD:
		res = "MD";
		break;
	case MF:
		res = "MF";
		break;
	case CNAME:
		res = "CNAME";
		break;
	case SOA:
		res = "SOA";
		break;
	case MB:
		res = "MB";
		break;
	case MG:
		res = "MG";
		break;
	case MR:
		res = "MR";
		break;
	case Empty:
		res = "Empty";
		break;
	case WKS:
		res = "WKS";
		break;
	case PTR:
		res = "PTR";
		break;
	case HINFO:
		res = "HINFO";
		break;
	case MINFO:
		res = "MINFO";
		break;
	case MX:
		res = "MX";
		break;
	case TXT:
		res = "TXT";
		break;
	case AAAA:
		res = "AAAA";
		break;
	case AXFR:
		res = "AXFR";
		break;
	case MAILB:
		res = "MAILB";
		break;
	case MAILA:
		res = "MAILA";
		break;
	case ANY_Type:
		res = "ANY_Type";
		break;
	}

	return res;
}

char *CDnsParser::ConvertDnsClass(USHORT cls)
{
	char *res = "NOTEXISTS";

	switch ((QClass)cls)
	{
	case INTERNET:
		res = "INTERNET";
		break;
	case CSNET:
		res = "CSNET";
		break;
	case CHAOS:
		res = "CHAOS";
		break;
	case HESIOD:
		res = "HESIOD";
		break;
	case ANY_Class:
		res = "ANY_Class";
		break;
	}

	return res;
}

bool CDnsParser::StartParseDnsHeader(const DNS_HDR *dns)
{
	m_DnsHdr.id			= ntohs(dns->id);
	m_DnsHdr.flags		= ntohs(dns->flags);
	m_DnsHdr.questions	= ntohs(dns->questions);
	m_DnsHdr.answers	= ntohs(dns->answers);
	m_DnsHdr.authority	= ntohs(dns->authority);
	m_DnsHdr.additional	= ntohs(dns->additional);

	return (1 == m_DnsHdr.questions);
}

void CDnsParser::StartParseDnsQuestion(const DNS_HDR *dns)
{
	char *pDns = (char *)dns;
	string strDomain;
	int iSeek;
	Question *pQue;

	strDomain = GetDomain(pDns, pDns+sizeof(DNS_HDR), &iSeek);
	m_Question.domain = strDomain;

	pDns += sizeof(DNS_HDR) + iSeek;//跳过dns头部和域名，指向Question
	pQue = (Question *)pDns;
	m_Question.question.type = ntohs(pQue->type);
	m_Question.question.cls = ntohs(pQue->cls);
}

void CDnsParser::StartParseDnsAnswer(const DNS_HDR *dns)
{
	//若是dns响应，才继续解析下去
	if (IsRequest() || !IsIpv4())
	{
		return;
	}

	Resource *pResource;
	SAnswer *pAnswer;
	char *pDns = (char *)dns;
	int iSeek;

	//指针移动到Answer首地址
	pDns += sizeof(DNS_HDR);
	while (*pDns++);
	pDns += sizeof(Question);

	//pDns指向Answer首地址
	//循环解析
	for (int i = 0; i < m_DnsHdr.answers; i++)
	{
		pAnswer =  new SAnswer;

		pAnswer->domain = GetDomain((char *)dns, pDns, &iSeek);
		pDns += iSeek;

		pResource = (Resource *)pDns;
		pDns += sizeof(Resource);//跳过Resource空间

		pAnswer->resource.type = ntohs(pResource->type);
		pAnswer->resource.cls = ntohs(pResource->cls);
		pAnswer->resource.ttl = ntohl(pResource->ttl);
		pAnswer->resource.len = ntohs(pResource->len);

		//ipv4地址
		if (A == pAnswer->resource.type)
		{
			pAnswer->result = inet_ntoa(*(struct in_addr *)pDns);//将ip地址转换为字符串型
			pDns += 4;
		}
		else if (CNAME == pAnswer->resource.type)//主机名
		{
			pAnswer->result = GetDomain((char *)dns, pDns, &iSeek);
			pDns += iSeek;
		}

		//添加到链表
		m_AnswerList.push_back(pAnswer);
	}
}

bool CDnsParser::IsCompressedName(const char *ptr, int *seek)
{
	USHORT flag = ntohs(*(USHORT *)ptr);
	//0xc00c
	if (flag & 0xc000)
	{
		if (seek)
		{
			*seek = flag & 0x3fff;//剩下的14位就是压缩名称指针位移
		}
		return true;
	}
	return false;
}

//first:dns首地址
//start:读取域名的首地址
string CDnsParser::GetDomain(const char *first, const char *start, int *pLen)
{
	char *p = (char *)start;
	char *tmp;
	int iLen = 0;
	int iSeek = 0;
	string strDomain;

	//获取非压缩名称指针的内容
	if (!IsCompressedName(p, NULL))
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
			if (IsCompressedName(p, NULL))
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
		if (IsCompressedName(tmp, &iSeek))
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
				if (!*tmp || IsCompressedName(tmp, NULL))
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

void CDnsParser::Clear()
{
	memset(&m_DnsHdr, 0, sizeof(DNS_HDR));
	m_pDnsHdr = NULL;

	for (SAnswerList::iterator it = m_AnswerList.begin(); it != m_AnswerList.end(); ++it)
	{
		delete *it;
	}
	m_AnswerList.clear();
}

