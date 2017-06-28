#pragma once

//#define FLAG_QR(a)		(((unsigned short)a)>>15)&0x1	//查询(0) | 响应(1)
//#define FLAG_OPCODE(a)	(((unsigned short)a)>>11)&0xf	//标准查询(0) | 反向查询(1) | 服务器状态请求(2) | 保留(3-15)
//#define FLAG_AA(a)		(((unsigned short)a)>>10)&0x1	//权威回答, 响应中有效, 指出给出应答的服务器是查询域名的授权解析服务器
//#define FLAG_TC(a)		(((unsigned short)a)>>9)&0x1	//截断(1), 若该位为1, 将会使用TCP重新请求
//#define FLAG_RD(a)		(((unsigned short)a)>>8)&0x1	//期望递归
//#define FLAG_RA(a)		(((unsigned short)a)>>7)&0x1	//递归可用
//#define FLAG_Z(a)		(((unsigned short)a)>>4)&0x111	//保留
//#define FLAG_RCODE(a)	(((unsigned short)a))&0xf		//没有错误(0) | 格式错误(1) | 服务器故障(2) | 名称错误(3) | 未实现(4) | 拒绝(5) | 保留(6-15)

#pragma pack(2) //2字节内存对齐

#define DNS_FLAG_QR			0x8000	//查询(0) | 响应(1)
#define DNS_FLAG_OPCODE		0x7000	//标准查询(0) | 反向查询(1) | 服务器状态请求(2) | 保留(3-15)
#define DNS_FLAG_AA			0x0400	//权威回答(1) | 非权威回答(0), 响应中有效
#define DNS_FLAG_TC			0x0200	//截断(1), 若该位为1, 将会使用TCP重新请求
#define DNS_FLAG_RD			0x0100	//期望递归
#define DNS_FLAG_RA			0x0080	//递归可用
#define DNS_FLAG_Z			0x0070	//保留
#define DNS_FLAG_RCODE		0x000f	//没有错误(0) | 格式错误(1) | 服务器故障(2) | 名称错误(3) | 未实现(4) | 拒绝(5) | 保留(6-15)

//DNS数据报头 12
typedef struct _dns_hdr
{
	unsigned short id; //标识，通过它客户端可以将DNS的请求与应答相匹配；
	unsigned short flags; //标志：(查询)0x0100 (应答)0x8180  这些数字都是主机序
	unsigned short questions; //问题数目
	unsigned short answers; //资源记录数目
	unsigned short authority; //授权资源记录数目
	unsigned short additional; //额外资源记录数目
}DNS_HDR;

//查询问题
typedef struct _tagQuestion
{
	unsigned short type;
	unsigned short cls;
}Question;

//资源记录
typedef struct _tagResource
{
	unsigned short	type;
	unsigned short	cls;
	unsigned int	ttl;
	unsigned short	len;
}Resource;

//用户使用的Question
typedef struct _tagSQuestion
{
	string		domain;
	Question	question;
}SQuestion;

//用户使用的Answer
typedef struct _tagSAnswer
{
	string		domain;
	Resource	resource;
	string		result;//ip或者主机名
}SAnswer;

typedef enum _tagQType
{
	A=1,		//指定计算机 IP 地址。 
	NS,			//指定用于命名区域的 DNS 名称服务器。 
	MD,			//指定邮件接收站（此类型已经过时了，使用MX代替） 
	MF,			//指定邮件中转站（此类型已经过时了，使用MX代替） 
	CNAME,		//指定用于别名的规范名称。 
	SOA,		//指定用于 DNS 区域的“起始授权机构”。 
	MB,			//指定邮箱域名。 
	MG,			//指定邮件组成员。 
	MR,			//指定邮件重命名域名。 
	Empty,		//指定空的资源记录 
	WKS,		//描述已知服务。 
	PTR,		//如果查询是 IP 地址，则指定计算机名；否则指定指向其它信息的指针。 
	HINFO,		//指定计算机 CPU 以及操作系统类型。 
	MINFO,		//指定邮箱或邮件列表信息。 
	MX,			//指定邮件交换器。 
	TXT,		//指定文本信息。 
	AAAA=28,	//ipv6
	AXFR=252,	//请求整个区域传送
	MAILB,		//请求相关邮箱记录
	MAILA,		//请求邮件代理RRs
	ANY_Type	//请求所有记录
}QType;

typedef enum _tagQClass
{
	INTERNET=1,	//指定 Internet 类别。 
	CSNET,		//指定 CSNET 类别。（已过时） 
	CHAOS,		//指定 Chaos 类别。 
	HESIOD,		//指定 MIT Athena Hesiod 类别。 
	ANY_Class			//指定任何以前列出的通配符。 
}QClass;

#pragma pack() //2字节内存对齐