// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__CB273829_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_)
#define AFX_MAINFRM_H__CB273829_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCommandsBuildcommand();
	afx_msg void OnCommandsRestorestateandexecutecommands();
	afx_msg void OnDevicecontextCsgetdeviceconfig();
	afx_msg void OnDevicecontextCsgetdevicecontext();
	afx_msg void OnDevicecontextCsgetprotocolrevision();
	afx_msg void OnGraphicalcontextCreatewindow();
	afx_msg void OnGraphicalcontextCsgetgraphicalcontext();
	afx_msg void OnGraphicalcontextCsreleasegraphicalcontext();
	afx_msg void OnMemoryCsalloc();
	afx_msg void OnMemoryCsfree();
	afx_msg void OnMemoryCslock();
	afx_msg void OnMemoryCsswapbuffertodisplay();
	afx_msg void OnMemoryCsunlock();
	afx_msg void OnOverlayCsacquireovleray();
	afx_msg void OnRegistersCsregisterread();
	afx_msg void OnRegistersCsregisterwrite();
	afx_msg void OnSystemCscleanupresourcesforprocess();
	afx_msg void OnSystemCsinit();
	afx_msg void OnSystemCsuninit();
	afx_msg void OnVideomodeCssetvideomode();
	afx_msg void OnDevicecontextCsreleasedevicecontext();
	afx_msg void OnCommandsExecutecommands();
	afx_msg void OnRunCompleteinit();
	afx_msg void OnRunCompleteuninit();
	afx_msg void OnRunStartrender();
	afx_msg void OnRunStoprendering();
	afx_msg void OnRunRendertosentinel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__CB273829_ED2E_11D3_96E0_00105A1D4E55__INCLUDED_)
