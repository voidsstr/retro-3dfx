// testapp1Doc.h : interface of the CTestapp1Doc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTAPP1DOC_H__CB27382B_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_)
#define AFX_TESTAPP1DOC_H__CB27382B_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CTestapp1Doc : public CDocument
{
protected: // create from serialization only
	CTestapp1Doc();
	DECLARE_DYNCREATE(CTestapp1Doc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestapp1Doc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTestapp1Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTestapp1Doc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTAPP1DOC_H__CB27382B_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_)
