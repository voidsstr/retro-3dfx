// Runner.h : main header file for the RUNNER application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#define IMAGE_READ_IN 0 // gray
#define IMAGE_PENDING 4
#define IMAGE_RUNNING 1
#define IMAGE_COMPLETED 2
#define IMAGE_STOPPED 3

/////////////////////////////////////////////////////////////////////////////
// CRunnerApp:
// See Runner.cpp for the implementation of this class
//

class CRunnerApp : public CWinApp
{
public:
	CDocument *m_pDoc;
	CSingleDocTemplate * m_pDocTemplate;
	CRunnerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRunnerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRunnerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
