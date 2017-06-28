#pragma once

//#define FLAG_QR(a)		(((unsigned short)a)>>15)&0x1	//��ѯ(0) | ��Ӧ(1)
//#define FLAG_OPCODE(a)	(((unsigned short)a)>>11)&0xf	//��׼��ѯ(0) | �����ѯ(1) | ������״̬����(2) | ����(3-15)
//#define FLAG_AA(a)		(((unsigned short)a)>>10)&0x1	//Ȩ���ش�, ��Ӧ����Ч, ָ������Ӧ��ķ������ǲ�ѯ��������Ȩ����������
//#define FLAG_TC(a)		(((unsigned short)a)>>9)&0x1	//�ض�(1), ����λΪ1, ����ʹ��TCP��������
//#define FLAG_RD(a)		(((unsigned short)a)>>8)&0x1	//�����ݹ�
//#define FLAG_RA(a)		(((unsigned short)a)>>7)&0x1	//�ݹ����
//#define FLAG_Z(a)		(((unsigned short)a)>>4)&0x111	//����
//#define FLAG_RCODE(a)	(((unsigned short)a))&0xf		//û�д���(0) | ��ʽ����(1) | ����������(2) | ���ƴ���(3) | δʵ��(4) | �ܾ�(5) | ����(6-15)

#pragma pack(2) //2�ֽ��ڴ����

#define DNS_FLAG_QR			0x8000	//��ѯ(0) | ��Ӧ(1)
#define DNS_FLAG_OPCODE		0x7000	//��׼��ѯ(0) | �����ѯ(1) | ������״̬����(2) | ����(3-15)
#define DNS_FLAG_AA			0x0400	//Ȩ���ش�(1) | ��Ȩ���ش�(0), ��Ӧ����Ч
#define DNS_FLAG_TC			0x0200	//�ض�(1), ����λΪ1, ����ʹ��TCP��������
#define DNS_FLAG_RD			0x0100	//�����ݹ�
#define DNS_FLAG_RA			0x0080	//�ݹ����
#define DNS_FLAG_Z			0x0070	//����
#define DNS_FLAG_RCODE		0x000f	//û�д���(0) | ��ʽ����(1) | ����������(2) | ���ƴ���(3) | δʵ��(4) | �ܾ�(5) | ����(6-15)

//DNS���ݱ�ͷ 12
typedef struct _dns_hdr
{
	unsigned short id; //��ʶ��ͨ�����ͻ��˿��Խ�DNS��������Ӧ����ƥ�䣻
	unsigned short flags; //��־��(��ѯ)0x0100 (Ӧ��)0x8180  ��Щ���ֶ���������
	unsigned short questions; //������Ŀ
	unsigned short answers; //��Դ��¼��Ŀ
	unsigned short authority; //��Ȩ��Դ��¼��Ŀ
	unsigned short additional; //������Դ��¼��Ŀ
}DNS_HDR;

//��ѯ����
typedef struct _tagQuestion
{
	unsigned short type;
	unsigned short cls;
}Question;

//��Դ��¼
typedef struct _tagResource
{
	unsigned short	type;
	unsigned short	cls;
	unsigned int	ttl;
	unsigned short	len;
}Resource;

//�û�ʹ�õ�Question
typedef struct _tagSQuestion
{
	string		domain;
	Question	question;
}SQuestion;

//�û�ʹ�õ�Answer
typedef struct _tagSAnswer
{
	string		domain;
	Resource	resource;
	string		result;//ip����������
}SAnswer;

typedef enum _tagQType
{
	A=1,		//ָ������� IP ��ַ�� 
	NS,			//ָ��������������� DNS ���Ʒ������� 
	MD,			//ָ���ʼ�����վ���������Ѿ���ʱ�ˣ�ʹ��MX���棩 
	MF,			//ָ���ʼ���תվ���������Ѿ���ʱ�ˣ�ʹ��MX���棩 
	CNAME,		//ָ�����ڱ����Ĺ淶���ơ� 
	SOA,		//ָ������ DNS ����ġ���ʼ��Ȩ�������� 
	MB,			//ָ������������ 
	MG,			//ָ���ʼ����Ա�� 
	MR,			//ָ���ʼ������������� 
	Empty,		//ָ���յ���Դ��¼ 
	WKS,		//������֪���� 
	PTR,		//�����ѯ�� IP ��ַ����ָ���������������ָ��ָ��������Ϣ��ָ�롣 
	HINFO,		//ָ������� CPU �Լ�����ϵͳ���͡� 
	MINFO,		//ָ��������ʼ��б���Ϣ�� 
	MX,			//ָ���ʼ��������� 
	TXT,		//ָ���ı���Ϣ�� 
	AAAA=28,	//ipv6
	AXFR=252,	//��������������
	MAILB,		//������������¼
	MAILA,		//�����ʼ�����RRs
	ANY_Type	//�������м�¼
}QType;

typedef enum _tagQClass
{
	INTERNET=1,	//ָ�� Internet ��� 
	CSNET,		//ָ�� CSNET ��𡣣��ѹ�ʱ�� 
	CHAOS,		//ָ�� Chaos ��� 
	HESIOD,		//ָ�� MIT Athena Hesiod ��� 
	ANY_Class			//ָ���κ���ǰ�г���ͨ����� 
}QClass;

#pragma pack() //2�ֽ��ڴ����