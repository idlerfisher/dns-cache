// ResizeControl.h: interface for the CResizeControl class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

typedef struct _tagResize
{
	UINT ID;
	CRect rt;
	int type1;
	int type2;
}tagResize;

enum{PK_TOP_LEFT,PK_TOP_RIGHT,PK_BOTTOM_LEFT,PK_BOTTOM_RIGHT};

class CResizeControl
{
public:
	void SetResize(UINT ID,int type1,int type2);
	void OnSize(int cx,int cy);
	void SetOwner(CWnd *pWnd);
	CResizeControl();
	virtual ~CResizeControl();

private:
	CWnd *m_pWnd;
	CArray <tagResize, tagResize&>m_arrResize;
	CRect m_rtParent;

};