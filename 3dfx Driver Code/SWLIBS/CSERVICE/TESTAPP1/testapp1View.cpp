// testapp1View.cpp : implementation of the CTestapp1View class
//

#include "stdafx.h"
#include "testapp1.h"

#include "testapp1Doc.h"
#include "testapp1View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestapp1View

IMPLEMENT_DYNCREATE(CTestapp1View, CView)

BEGIN_MESSAGE_MAP(CTestapp1View, CView)
	//{{AFX_MSG_MAP(CTestapp1View)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestapp1View construction/destruction

CTestapp1View::CTestapp1View()
{
	// TODO: add construction code here

}

CTestapp1View::~CTestapp1View()
{
}

BOOL CTestapp1View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTestapp1View drawing

void CTestapp1View::OnDraw(CDC* pDC)
{
	CTestapp1Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CTestapp1View diagnostics

#ifdef _DEBUG
void CTestapp1View::AssertValid() const
{
	CView::AssertValid();
}

void CTestapp1View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTestapp1Doc* CTestapp1View::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTestapp1Doc)));
	return (CTestapp1Doc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTestapp1View message handlers
