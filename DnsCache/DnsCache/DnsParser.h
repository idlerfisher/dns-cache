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
	
	//dns头信息
	bool	IsRequest();
	bool	IsResponce();
	USHORT	GetDnsId();
	USHORT	GetDnsFlag();
	USHORT	GetQuestions();
	USHORT	GetAnswers();
	USHORT	GetAuthority();
	USHORT	GetAdditional();

	//dns标志
	USHORT	GetFlagQR();//查询(0) | 响应(1)
	USHORT	GetFlagOPCODE();//标准查询(0) | 反向查询(1) | 服务器状态请求(2) | 保留(3-15)
	USHORT	GetFlagAA();//权威回答(1) | 非权威回答(0), 响应中有效
	USHORT	GetFlagTC();//截断(1), 若该位为1, 将会使用TCP重新请求
	USHORT	GetFlagRD();//期望递归
	USHORT	GetFlagRA();//递归可用
	USHORT	GetFlagZ();//保留
	USHORT	GetFlagRCODE();//没有错误(0) | 格式错误(1) | 服务器故障(2) | 名称错误(3) | 未实现(4) | 拒绝(5) | 保留(6-15)

	//dns请求/响应信息
	bool	IsIpv4();
	bool	IsIpv6();
	string  QuestionDomain();//请求域名
	QType	QuestionType();//请求类型
	QClass	QuestionClass();//请求类
	StringList GetDomainList();//域名解析链表
	StringList GetIpList();//ip解析链表
	
	//log
	void	PrintLog();

	//dns仿真
	static string	EmulateDnsHostResponce(const string &host, const string &req);//仿真dns主机名,ptr指针查询
	static string	EmulateIpv6Responce(const string &req);//仿真ipv6的响应
	static string	CreateDnsFormatDomain(const string &name);//生成dns格式的域名
	static char		*ConvertDnsType(USHORT type);
	static char		*ConvertDnsClass(USHORT cls);

private:
	bool	StartParseDnsHeader(const DNS_HDR *dns);//获取dns头信息
	void	StartParseDnsQuestion(const DNS_HDR *dns);//获取dns请求信息:Question
	void	StartParseDnsAnswer(const DNS_HDR *dns);//遍历获取所有dns响应:Answer
	bool	IsCompressedName(const char *ptr, int *seek);//判断是否是压缩名称指针
	string	GetDomain(const char *first, const char *start, int *pLen=NULL);//获取域名,first为dns首地址，first为域名读取的首地址
	void	Clear();

private:
	DNS_HDR			m_DnsHdr;//dns头部
	SQuestion		m_Question;//请求报文
	SAnswerList		m_AnswerList;//应答链表
	string			m_strDnsBuf;//dns请求/响应缓存
	DNS_HDR			*m_pDnsHdr;//dns头结构
};

