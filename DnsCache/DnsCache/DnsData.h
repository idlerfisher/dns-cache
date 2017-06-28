#pragma once

class CDnsData
{
public:
	CDnsData(void);
	~CDnsData(void);

	//dns����
	void		AddToSeekMem(DNSINFO *pDnsInfo);
	void		UpdateSeekMem(DNSINFO *pDnsInfo);
	void		UpdateSeekMem(DNSINFOVECTOR &vec);
	DNSINFO		*GetDnsFromMem(const CString &domain);
	void		LockMemDns();
	void		UnlockMemDns();

	//���ݿ⻺��
	void		AddToSqlList(DNSINFO *pDnsInfo);
	void		AddToSqlList(DNSINFOVECTOR &vec);
	bool		PopSqlList(DNSINFOLIST &lst);

	//��������
	void		AddToUpdateVec(DNSINFO *pDnsInfo);
	void		ModifyUpdateInfo(DNSINFO *pDnsInfo);
	bool		GetTimeoutDns(DNSINFOVECTOR &vec);

	void			ReleaseMem();
	static DNSINFO *CreateDnsInfo(const CString &domain, const char *reqbuf, int reqlen, const char *resbuf, int reslen);
	static void		CopyDnsInfo(const DNSINFO *pSrc, DNSINFO *pDst);

private:
	DNSINFOMAP		m_DnsSeekMem;//�ڴ�Ѱ����
	DNSINFOLIST		m_DnsSqlList;//���ݿ⻺��
	DNSINFOVECTOR	m_DnsUpdateVec;//��������
	CMutex			m_DnsListMutex;//���ݿ⻺����
	CMutex			m_DnsMapMutex;//�ڴ�Ѱ������
	CMutex			m_DnsUpdateMutex;//����������
	CMutex			m_MemDnsValueMutex;//�ڴ�DNS������
};
