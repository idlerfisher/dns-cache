#include "StdAfx.h"
#include "DnsData.h"
#include "Public.h"

CDnsData::CDnsData(void)
{
}

CDnsData::~CDnsData(void)
{
	ReleaseMem();
}

void CDnsData::AddToSeekMem(DNSINFO *pDnsInfo)
{
	m_DnsMapMutex.Lock();
	m_DnsSeekMem[pDnsInfo->strDomain] = pDnsInfo;
	m_DnsMapMutex.Unlock();
}

void CDnsData::UpdateSeekMem(DNSINFO *pDnsInfo)
{
	DNSINFO *pTmp;

	m_DnsMapMutex.Lock();
	pTmp = m_DnsSeekMem[pDnsInfo->strDomain];
	m_DnsMapMutex.Unlock();

	if (pTmp)
	{
		LockMemDns();
		if (pTmp->ipVec != pDnsInfo->ipVec)
		{
			pTmp->ipVec = pDnsInfo->ipVec;
		}
		if (pTmp->nTtl != pDnsInfo->nTtl)
		{
			pTmp->nTtl = pDnsInfo->nTtl;
		}
		if (pTmp->strUpdateTime != pDnsInfo->strUpdateTime)
		{
			pTmp->strUpdateTime = pDnsInfo->strUpdateTime;
		}
		if (pTmp->nLastVisitTime != pDnsInfo->nLastVisitTime)
		{
			pTmp->nLastVisitTime = pDnsInfo->nLastVisitTime;
		}
		if (pTmp->nLastUpdateTime != pDnsInfo->nLastUpdateTime)
		{
			pTmp->nLastUpdateTime = pDnsInfo->nLastUpdateTime;
		}
		if (pTmp->op != SQLOP_UPDATE)
		{
			pTmp->op = SQLOP_UPDATE;
		}
		UnlockMemDns();
		AddToSqlList(pTmp);//添加到数据库,以便更新
	}
}

void CDnsData::UpdateSeekMem(DNSINFOVECTOR &vec)
{
	DNSINFO *pTmp, *pDnsInfo;
	DNSINFOVECTOR dnsVec;

	m_DnsMapMutex.Lock();
	for (DNSINFOVECTOR::iterator it = vec.begin(); it != vec.end(); ++it)
	{
		pDnsInfo = *it;
		pTmp = m_DnsSeekMem[pDnsInfo->strDomain];
		dnsVec.push_back(pTmp);

		LockMemDns();
		if (pTmp->ipVec != pDnsInfo->ipVec)
		{
			pTmp->ipVec = pDnsInfo->ipVec;
		}
		if (pTmp->nTtl != pDnsInfo->nTtl)
		{
			pTmp->nTtl = pDnsInfo->nTtl;
		}
		if (pTmp->strUpdateTime != pDnsInfo->strUpdateTime)
		{
			pTmp->strUpdateTime = pDnsInfo->strUpdateTime;
		}
		if (pTmp->nLastVisitTime != pDnsInfo->nLastVisitTime)
		{
			pTmp->nLastVisitTime = pDnsInfo->nLastVisitTime;
		}
		if (pTmp->nLastUpdateTime != pDnsInfo->nLastUpdateTime)
		{
			pTmp->nLastUpdateTime = pDnsInfo->nLastUpdateTime;
		}
		if (pTmp->op != SQLOP_UPDATE)
		{
			pTmp->op = SQLOP_UPDATE;
		}
		UnlockMemDns();
	}
	AddToSqlList(dnsVec);
	m_DnsMapMutex.Unlock();
}

DNSINFO *CDnsData::GetDnsFromMem(const CString &domain)
{
	m_DnsMapMutex.Lock();
	DNSINFO *pDnsInfo = m_DnsSeekMem[domain];
	m_DnsMapMutex.Unlock();

	return pDnsInfo;
}

void CDnsData::LockMemDns()
{
	m_MemDnsValueMutex.Lock();
}

void CDnsData::UnlockMemDns()
{
	m_MemDnsValueMutex.Unlock();
}

void CDnsData::AddToSqlList(DNSINFO *pDnsInfo)
{
	m_DnsListMutex.Lock();
	if (find(m_DnsSqlList.begin(), m_DnsSqlList.end(), pDnsInfo) == m_DnsSqlList.end())
	{
		m_DnsSqlList.push_back(pDnsInfo);
	}
	m_DnsListMutex.Unlock();
}

void CDnsData::AddToSqlList(DNSINFOVECTOR &vec)
{
	m_DnsListMutex.Lock();
	for (DNSINFOVECTOR::iterator it = vec.begin(); it != vec.end(); ++it)
	{
		if (find(m_DnsSqlList.begin(), m_DnsSqlList.end(), *it) == m_DnsSqlList.end())
		{
			m_DnsSqlList.push_back(*it);
		}
	}
	m_DnsListMutex.Unlock();
}

bool CDnsData::PopSqlList(DNSINFOLIST &lst)
{
	m_DnsListMutex.Lock();
	if (!m_DnsSqlList.empty())
	{
		lst.merge(m_DnsSqlList);
	}
	m_DnsListMutex.Unlock();
	return !lst.empty();
}

void CDnsData::AddToUpdateVec(DNSINFO *pDnsInfo)
{
	m_DnsUpdateMutex.Lock();
	m_DnsUpdateVec.push_back(pDnsInfo);
	m_DnsUpdateMutex.Unlock();
}

void CDnsData::ModifyUpdateInfo(DNSINFO *pDnsInfo)
{
	m_DnsUpdateMutex.Lock();
	DNSINFO *pTmp = NULL;
	for (DNSINFOVECTOR::iterator it = m_DnsUpdateVec.begin(); it != m_DnsUpdateVec.end(); ++it)
	{
		pTmp = *it;
		if (pTmp->strDomain == pDnsInfo->strDomain)
		{
			break;
		}
	}
	if (pTmp)
	{
		//该接口只有从内存搜索到数据，才会调用，所以只更新访问信息
		pTmp->nVisit			= pDnsInfo->nVisit;
		pTmp->nLastVisitTime	= pDnsInfo->nLastVisitTime;
		pTmp->strVisitTime		= pDnsInfo->strVisitTime;
	}
	m_DnsUpdateMutex.Unlock();
}

bool CDnsData::GetTimeoutDns(DNSINFOVECTOR &vec)
{
	UINT size;
	time_t iTime;
	DNSINFO *pDnsInfo;
	const time_t newer = 20 * 60;//20分钟之内的访问才更新dns

	m_DnsUpdateMutex.Lock();
	size = m_DnsUpdateVec.size();//先得到当前的大小
	m_DnsUpdateMutex.Unlock();

	//每次lock并延迟一段时间,为了给转发线程留机会，避免阻塞太久
	for (UINT i = 0; i < size; i++)
	{
		m_DnsUpdateMutex.Lock();
		pDnsInfo = m_DnsUpdateVec.at(i);
		iTime = time(NULL);
		if (iTime - pDnsInfo->nLastVisitTime < newer
			&& iTime - pDnsInfo->nLastUpdateTime > pDnsInfo->nTtl)
		{
			vec.push_back(pDnsInfo);//超时
		}
		m_DnsUpdateMutex.Unlock();
	}

	return !vec.empty();
}

void CDnsData::ReleaseMem()
{
	for (DNSINFOMAP::iterator itMap = m_DnsSeekMem.begin(); itMap != m_DnsSeekMem.end(); ++itMap)
	{
		delete itMap->second;
	}
	m_DnsSeekMem.clear();

	for (DNSINFOVECTOR::iterator itVec = m_DnsUpdateVec.begin(); itVec != m_DnsUpdateVec.end(); ++itVec)
	{
		delete *itVec;
	}
	m_DnsUpdateVec.clear();
}

DNSINFO *CDnsData::CreateDnsInfo(const CString &domain, const char *reqbuf, int reqlen, const char *resbuf, int reslen)
{
	int nTtl = CPublic::GetTtlFromDnsBuf(resbuf);
	if (-1 == nTtl)
	{
		return NULL;
	}

	//dnsinfo
	DNSINFO *pDnsInfo			= new DNSINFO;
	pDnsInfo->strDomain			= domain;
	pDnsInfo->nVisit			= 1;
	pDnsInfo->strVisitTime		= CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
	pDnsInfo->strUpdateTime		= pDnsInfo->strVisitTime;
	pDnsInfo->nLastVisitTime	= time(NULL);
	pDnsInfo->nLastUpdateTime	= pDnsInfo->nLastVisitTime;
	pDnsInfo->nTtl				= nTtl;
	pDnsInfo->ipVec				= CPublic::GetIpFromDnsBuf(resbuf);
	pDnsInfo->op				= SQLOP_INSERT;//首次

	//dns请求
	pDnsInfo->dnsReq.len = reqlen;
	memcpy(pDnsInfo->dnsReq.buf, reqbuf, reqlen);

	//dns响应
	pDnsInfo->dnsRes.len = reslen;
	memcpy(pDnsInfo->dnsRes.buf, resbuf, reslen);

	return pDnsInfo;
}

void CDnsData::CopyDnsInfo(const DNSINFO *pSrc, DNSINFO *pDst)
{
	pDst->dnsReq			= pSrc->dnsReq;
	pDst->dnsRes			= pSrc->dnsRes;
	pDst->ipVec				= pSrc->ipVec;
	pDst->nTtl				= pSrc->nTtl;
	pDst->nVisit			= pSrc->nVisit;
	pDst->op				= pSrc->op;
	pDst->strDomain			= pSrc->strDomain;
	pDst->strUpdateTime		= pSrc->strUpdateTime;
	pDst->strVisitTime		= pSrc->strVisitTime;
	pDst->nLastVisitTime	= pSrc->nLastVisitTime;
	pDst->nLastUpdateTime	= pSrc->nLastUpdateTime;
}