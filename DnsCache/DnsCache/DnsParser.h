#pragma once

typedef unsigned short USHORT;
typedef vector<SAnswer *> SAnswerList;
typedef vector<string> StringList;

#define DNS_HOSTINFO	"secure-fast-dns"

class CDnsParser
{
public:
	CDnsParser();
	CDnsParser(const char *ptr, int len);
	virtual ~CDnsParser();

	void	SetDnsBuf(const char *ptr, int len);
	bool	StartParse();
	
	//dnsͷ��Ϣ
	bool	IsRequest();
	bool	IsResponce();
	USHORT	GetDnsId();
	USHORT	GetDnsFlag();
	USHORT	GetQuestions();
	USHORT	GetAnswers();
	USHORT	GetAuthority();
	USHORT	GetAdditional();

	//dns��־
	USHORT	GetFlagQR();//��ѯ(0) | ��Ӧ(1)
	USHORT	GetFlagOPCODE();//��׼��ѯ(0) | �����ѯ(1) | ������״̬����(2) | ����(3-15)
	USHORT	GetFlagAA();//Ȩ���ش�(1) | ��Ȩ���ش�(0), ��Ӧ����Ч
	USHORT	GetFlagTC();//�ض�(1), ����λΪ1, ����ʹ��TCP��������
	USHORT	GetFlagRD();//�����ݹ�
	USHORT	GetFlagRA();//�ݹ����
	USHORT	GetFlagZ();//����
	USHORT	GetFlagRCODE();//û�д���(0) | ��ʽ����(1) | ����������(2) | ���ƴ���(3) | δʵ��(4) | �ܾ�(5) | ����(6-15)

	//dns����/��Ӧ��Ϣ
	bool	IsIpv4();
	bool	IsIpv6();
	string  QuestionDomain();//��������
	QType	QuestionType();//��������
	QClass	QuestionClass();//������
	StringList GetDomainList();//������������
	StringList GetIpList();//ip��������
	
	//log
	void	PrintLog();

	//dns����
	static string	EmulateDnsHostResponce(const string &host, const string &req);//����dns������,ptrָ���ѯ
	static string	EmulateIpv6Responce(const string &req);//����ipv6����Ӧ
	static string	CreateDnsFormatDomain(const string &name);//����dns��ʽ������
	static char		*ConvertDnsType(USHORT type);
	static char		*ConvertDnsClass(USHORT cls);

private:
	bool	StartParseDnsHeader(const DNS_HDR *dns);//��ȡdnsͷ��Ϣ
	void	StartParseDnsQuestion(const DNS_HDR *dns);//��ȡdns������Ϣ:Question
	void	StartParseDnsAnswer(const DNS_HDR *dns);//������ȡ����dns��Ӧ:Answer
	bool	IsCompressedName(const char *ptr, int *seek);//�ж��Ƿ���ѹ������ָ��
	string	GetDomain(const char *first, const char *start, int *pLen=NULL);//��ȡ����,firstΪdns�׵�ַ��firstΪ������ȡ���׵�ַ
	void	Clear();

private:
	DNS_HDR			m_DnsHdr;//dnsͷ��
	SQuestion		m_Question;//������
	SAnswerList		m_AnswerList;//Ӧ������
	string			m_strDnsBuf;//dns����/��Ӧ����
	DNS_HDR			*m_pDnsHdr;//dnsͷ�ṹ
};

