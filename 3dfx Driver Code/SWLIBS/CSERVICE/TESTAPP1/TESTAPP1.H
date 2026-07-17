// testapp1.h : main header file for the TESTAPP1 application
//

#if !defined(AFX_TESTAPP1_H__CB273825_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_)
#define AFX_TESTAPP1_H__CB273825_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "clientdll.h"

/////////////////////////////////////////////////////////////////////////////
// CTestapp1App:
// See testapp1.cpp for the implementation of this class
//

class CTestapp1App : public CWinApp
{
public:
	CTestapp1App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestapp1App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTestapp1App)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTAPP1_H__CB273825_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_)
