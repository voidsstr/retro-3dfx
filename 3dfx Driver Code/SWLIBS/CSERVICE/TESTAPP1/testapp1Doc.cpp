// testapp1Doc.cpp : implementation of the CTestapp1Doc class
//

#include "stdafx.h"
#include "testapp1.h"

#include "testapp1Doc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestapp1Doc

IMPLEMENT_DYNCREATE(CTestapp1Doc, CDocument)

BEGIN_MESSAGE_MAP(CTestapp1Doc, CDocument)
	//{{AFX_MSG_MAP(CTestapp1Doc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestapp1Doc construction/destruction

CTestapp1Doc::CTestapp1Doc()
{
	// TODO: add one-time construction code here

}

CTestapp1Doc::~CTestapp1Doc()
{
}

BOOL CTestapp1Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CTestapp1Doc serialization

void CTestapp1Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTestapp1Doc diagnostics

#ifdef _DEBUG
void CTestapp1Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTestapp1Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTestapp1Doc commands
