#pragma once

#pragma pack(2) //2�ֽ��ڴ����

// i386 is little_endian.
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN   (1)   //BYTE ORDER
#else
#error Redefine LITTLE_ORDER
#endif
//Macͷ�����ܳ���14�ֽ�
typedef struct _eth_hdr
{
	unsigned char dstmac[6]; //Ŀ��mac��ַ
	unsigned char srcmac[6]; //Դmac��ַ
	unsigned short eth_type; //��̫������
}ETH_HEADER;

//IPͷ�����ܳ���20�ֽ�
typedef struct _ip_hdr
{
#if LITTLE_ENDIAN
	unsigned char ihl:4;     //�ײ�����
	unsigned char version:4; //�汾 
#else
	unsigned char version:4; //�汾
	unsigned char ihl:4;     //�ײ�����
#endif
	unsigned char tos;       //��������
	unsigned short tot_len;  //�ܳ���
	unsigned short id;       //��־
	unsigned short frag_off; //��Ƭƫ��
	unsigned char ttl;       //����ʱ��
	unsigned char protocol;  //Э��
	unsigned short chk_sum;  //�����
	struct in_addr srcaddr;  //ԴIP��ַ
	struct in_addr dstaddr;  //Ŀ��IP��ַ
}IP_HEADER;

//TCPͷ�����ܳ���20�ֽ�
typedef struct _tcp_hdr
{
	unsigned short src_port;    //Դ�˿ں�
	unsigned short dst_port;    //Ŀ�Ķ˿ں�
	unsigned int seq_no;        //���к�
	unsigned int ack_no;        //ȷ�Ϻ�
#if LITTLE_ENDIAN
	unsigned char reserved_1:4; //����6λ�е�4λ�ײ�����
	unsigned char thl:4;        //tcpͷ������
	unsigned char flag:6;       //6λ��־
	unsigned char reseverd_2:2; //����6λ�е�2λ
#else
	unsigned char thl:4;        //tcpͷ������
	unsigned char reserved_1:4; //����6λ�е�4λ�ײ�����
	unsigned char reseverd_2:2; //����6λ�е�2λ
	unsigned char flag:6;       //6λ��־ 
#endif
	unsigned short wnd_size;    //16λ���ڴ�С
	unsigned short chk_sum;     //16λTCP�����
	unsigned short urgt_p;      //16Ϊ����ָ��
}TCP_HEADER;

//UDPͷ�����ܳ���8�ֽ�
typedef struct _udp_hdr
{
	unsigned short src_port; //Զ�˿ں�
	unsigned short dst_port; //Ŀ�Ķ˿ں�
	unsigned short uhl;      //udpͷ������
	unsigned short chk_sum;  //16λudp�����
}UDP_HEADER;

//ICMPͷ�����ܳ���4�ֽ�
typedef struct _icmp_hdr
{
	unsigned char i_type;//8λ����
	unsigned char i_code; //8λ���� 
	unsigned short i_cksum; //16λУ��� 
	unsigned short i_id; //ʶ��ţ�һ���ý��̺���Ϊʶ��ţ� 
	unsigned short i_seq; //�������к� 
	unsigned int timestamp;//ʱ��� 
}ICMP_HEADER;

#pragma pack() //2�ֽ��ڴ����