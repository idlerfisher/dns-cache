#include "StdAfx.h"
#include "Flag.h"


CFlag::CFlag()
{
	m_bFlag = TRUE;
}


CFlag::~CFlag(void)
{
}

void CFlag::SetFlag(BOOL ok)
{
	m_Mutex.Lock();
	m_bFlag = ok;
	m_Mutex.Unlock();
}

BOOL CFlag::GetFlag()
{
	m_Mutex.Lock();
	BOOL ok = m_bFlag;
	m_Mutex.Unlock();
	return ok;
}