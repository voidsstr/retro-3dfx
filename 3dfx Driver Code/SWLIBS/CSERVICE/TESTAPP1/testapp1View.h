// testapp1View.h : interface of the CTestapp1View class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTAPP1VIEW_H__CB27382D_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_)
#define AFX_TESTAPP1VIEW_H__CB27382D_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CTestapp1View : public CView
{
protected: // create from serialization only
	CTestapp1View();
	DECLARE_DYNCREATE(CTestapp1View)

// Attributes
public:
	CTestapp1Doc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestapp1View)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTestapp1View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTestapp1View)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in testapp1View.cpp
inline CTestapp1Doc* CTestapp1View::GetDocument()
   { return (CTestapp1Doc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTAPP1VIEW_H__CB27382D_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_)
