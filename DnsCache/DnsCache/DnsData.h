#pragma once

class CDnsData
{
public:
	CDnsData(void);
	~CDnsData(void);

	//dns缓存
	void		AddToSeekMem(DNSINFO *pDnsInfo);
	void		UpdateSeekMem(DNSINFO *pDnsInfo);
	void		UpdateSeekMem(DNSINFOVECTOR &vec);
	DNSINFO		*GetDnsFromMem(const CString &domain);
	void		LockMemDns();
	void		UnlockMemDns();

	//数据库缓存
	void		AddToSqlList(DNSINFO *pDnsInfo);
	void		AddToSqlList(DNSINFOVECTOR &vec);
	bool		PopSqlList(DNSINFOLIST &lst);

	//更新链表
	void		AddToUpdateVec(DNSINFO *pDnsInfo);
	void		ModifyUpdateInfo(DNSINFO *pDnsInfo);
	bool		GetTimeoutDns(DNSINFOVECTOR &vec);

	void			ReleaseMem();
	static DNSINFO *CreateDnsInfo(const CString &domain, const char *reqbuf, int reqlen, const char *resbuf, int reslen);
	static void		CopyDnsInfo(const DNSINFO *pSrc, DNSINFO *pDst);

private:
	DNSINFOMAP		m_DnsSeekMem;//内存寻找区
	DNSINFOLIST		m_DnsSqlList;//数据库缓存
	DNSINFOVECTOR	m_DnsUpdateVec;//更新链表
	CMutex			m_DnsListMutex;//数据库缓存锁
	CMutex			m_DnsMapMutex;//内存寻找区锁
	CMutex			m_DnsUpdateMutex;//更新链表锁
	CMutex			m_MemDnsValueMutex;//内存DNS数据锁
};
