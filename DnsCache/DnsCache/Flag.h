#pragma once
class CFlag
{
public:
	CFlag();
	~CFlag(void);

	void SetFlag(BOOL ok);
	BOOL GetFlag();

private:
	CMutex  m_Mutex;
	BOOL	m_bFlag;
};

