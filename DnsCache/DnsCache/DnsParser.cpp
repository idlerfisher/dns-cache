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

//�ɹ����ڴ����������桢ת��
//ʧ�ܣ�ת��
bool CDnsParser::StartParse()
{
	int dnsLen = m_strDnsBuf.size();
	m_pDnsHdr = (DNS_HDR *)m_strDnsBuf.c_str();

	//��ȡdnsͷ��Ϣ
	if (!StartParseDnsHeader(m_pDnsHdr))
	{
		return false;
	}

	//��ȡdns������Ϣ:Question
	StartParseDnsQuestion(m_pDnsHdr);
	
	//������ȡ����dns��Ӧ:Answer
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

	//��ַ
	sprintf_s(buf, sizeof(buf), "[%s]\n\n", m_Question.domain.c_str());
	strLog += buf;

	//dnsͷ��Ϣ
	sprintf_s(buf, sizeof(buf), "[DNS]:\nIdentification:\t0x%04x\nDns Flags:\t0x%04x\nQuestions:\t%d\nAnswers:\t\t%d\nAuthority:\t\t%d\nAdditional:\t%d\n\n", 
		m_DnsHdr.id, m_DnsHdr.flags, m_DnsHdr.questions, m_DnsHdr.answers, m_DnsHdr.authority, m_DnsHdr.additional);
	strLog += buf;

	//��������ϢQuestion
	sprintf_s(buf, sizeof(buf), "[Question]:\nDomain Name:\t%s\nType:\t\t%d  %s\nClass:\t\t%d  %s\n\n", m_Question.domain.c_str(), m_Question.question.type, ConvertDnsType(m_Question.question.type), m_Question.question.cls, ConvertDnsClass(m_Question.question.cls));
	strLog += buf;

	//��Ӧ������ϢAnswer
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
	USHORT ptrSeek;//ѹ�����ݵ�ָ��λ��:1100000000001100
	
	//����dns��ʽ��������
	strDomain = CreateDnsFormatDomain(host);

	/******��ʼ�����Զ���dns��Ӧ��*******/
	memset(res, 0, sizeof(res));
	memcpy(pRes, req.c_str(), req.size());//����������

	//�޸�dnsͷ��Ϣ
	pDnsHdr = (DNS_HDR *)pRes;
	pDnsHdr->flags = htons(0x8180);//��־: 0x8180
	pDnsHdr->answers = htons(1);//�ش����: 1

	//ѹ������ָ�룺0xc00c 2�ֽ�

	//���Ӳ����һ����Դ��¼(�ش�)
	Resource resource;
	memset(&resource, 0, sizeof(Resource));
	resource.type = htons(PTR);
	resource.cls = htons(INTERNET);
	resource.ttl = htonl(10000);
	resource.len = htons(strDomain.size());

	//[DNSЭ��]
	//����Answer��Դ��¼
	pRes += req.size();//ָ��洢Answer��Դ���׵�ַ(Domain Name:����ʹ��ѹ��ָ��)

	ptrSeek = htons(0xc00c);//ת���������ֽ���
	memcpy(pRes, &ptrSeek, sizeof(USHORT));//���ѹ��ָ��
	pRes += sizeof(USHORT);//ָ��Answerʵ�ʶ���Ľṹ��ַ

	memcpy(pRes, &resource, sizeof(Resource));//���ش���Դ��¼
	pRes += sizeof(Resource);

	memcpy(pRes, strDomain.c_str(), strDomain.size());

	//DNSЭ��ͷ_������Դͷ + ѹ��ָ�� + �ش���Դͷ + ��������
	resLen = req.size() + 2 + sizeof(Resource) + strDomain.size();//��Ӧ���ĳ���

	return string(res, resLen);
}

string CDnsParser::EmulateIpv6Responce(const string &req)
{
	DNS_HDR *pDnsHdr;
	char res[1024];
	char *pRes = res;
	int resLen;
	USHORT ptrSeek;//ѹ�����ݵ�ָ��λ��:1100000000001100

	/******��ʼ�����Զ���dns��Ӧ��*******/
	memset(res, 0, sizeof(res));
	memcpy(pRes, req.c_str(), req.size());//����������

	//�޸�dnsͷ��Ϣ
	pDnsHdr = (DNS_HDR *)pRes;
	pDnsHdr->flags = htons(0x8180);//��־: 0x8180
	pDnsHdr->answers = htons(1);//�ش����: 1

	//ѹ������ָ�룺0xc00c 2�ֽ�

	//���Ӳ����һ����Դ��¼(�ش�)
	Resource resource;
	resource.type = htons(CNAME);
	resource.cls = htons(INTERNET);
	resource.ttl = htonl(10000);
	resource.len = htons(2);

	//ѹ������ָ�룺0xc00c 2�ֽ�

	//[DNSЭ��]
	//����Answer��Դ��¼
	pRes += req.size();//ָ��洢Answer��Դ���׵�ַ(Domain Name:����ʹ��ѹ��ָ��)

	ptrSeek = htons(0xc00c);//ת���������ֽ���
	memcpy(pRes, &ptrSeek, sizeof(USHORT));//���ѹ��ָ��
	pRes += sizeof(USHORT);//ָ��Answerʵ�ʶ���Ľṹ��ַ

	memcpy(pRes, &resource, sizeof(Resource));//���ش���Դ��¼
	pRes += sizeof(Resource);

	ptrSeek = htons(0xc00c);//ת���������ֽ���
	memcpy(pRes, &ptrSeek, sizeof(USHORT));//���ѹ��ָ��

	//DNSЭ��ͷ_������Դͷ + ѹ��ָ�� + �ش���Դͷ + ��������
	resLen = req.size() + 2 + sizeof(Resource) + 2;//��Ӧ���ĳ���

	return string(res, resLen);
}

string CDnsParser::CreateDnsFormatDomain(const string &name)
{
	//����dnsЭ���ʽ���ַ���
	//example:
	//www.baidu.com
	//03 www 05 baidu 03 com  ռ��15
	//�洢��ʽ:03 www 09 hywebtech 03 com 00
	string strDomain;
	StringVector strVec;

	strVec = CPublic::Split(name, '.');
	for (UINT i = 0; i < strVec.size(); i++)
	{
		string &r = strVec[i];
		strDomain += r.size();//�ָ����ַ�������
		strDomain += r;
	}
	strDomain += '\0';//������Ҫ��0

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

	pDns += sizeof(DNS_HDR) + iSeek;//����dnsͷ����������ָ��Question
	pQue = (Question *)pDns;
	m_Question.question.type = ntohs(pQue->type);
	m_Question.question.cls = ntohs(pQue->cls);
}

void CDnsParser::StartParseDnsAnswer(const DNS_HDR *dns)
{
	//����dns��Ӧ���ż���������ȥ
	if (IsRequest() || !IsIpv4())
	{
		return;
	}

	Resource *pResource;
	SAnswer *pAnswer;
	char *pDns = (char *)dns;
	int iSeek;

	//ָ���ƶ���Answer�׵�ַ
	pDns += sizeof(DNS_HDR);
	while (*pDns++);
	pDns += sizeof(Question);

	//pDnsָ��Answer�׵�ַ
	//ѭ������
	for (int i = 0; i < m_DnsHdr.answers; i++)
	{
		pAnswer =  new SAnswer;

		pAnswer->domain = GetDomain((char *)dns, pDns, &iSeek);
		pDns += iSeek;

		pResource = (Resource *)pDns;
		pDns += sizeof(Resource);//����Resource�ռ�

		pAnswer->resource.type = ntohs(pResource->type);
		pAnswer->resource.cls = ntohs(pResource->cls);
		pAnswer->resource.ttl = ntohl(pResource->ttl);
		pAnswer->resource.len = ntohs(pResource->len);

		//ipv4��ַ
		if (A == pAnswer->resource.type)
		{
			pAnswer->result = inet_ntoa(*(struct in_addr *)pDns);//��ip��ַת��Ϊ�ַ�����
			pDns += 4;
		}
		else if (CNAME == pAnswer->resource.type)//������
		{
			pAnswer->result = GetDomain((char *)dns, pDns, &iSeek);
			pDns += iSeek;
		}

		//��ӵ�����
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
			*seek = flag & 0x3fff;//ʣ�µ�14λ����ѹ������ָ��λ��
		}
		return true;
	}
	return false;
}

//first:dns�׵�ַ
//start:��ȡ�������׵�ַ
string CDnsParser::GetDomain(const char *first, const char *start, int *pLen)
{
	char *p = (char *)start;
	char *tmp;
	int iLen = 0;
	int iSeek = 0;
	string strDomain;

	//��ȡ��ѹ������ָ�������
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

			//������ѹ��ָ�룬��ѭ������ѹ��ָ����ָ������
			if (IsCompressedName(p, NULL))
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
		if (IsCompressedName(tmp, &iSeek))
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
				if (!*tmp || IsCompressedName(tmp, NULL))
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

