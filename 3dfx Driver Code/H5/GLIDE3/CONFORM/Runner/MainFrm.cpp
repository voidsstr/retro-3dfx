// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Runner.h"

#include "MainFrm.h"
#include "RunnerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_ALWAYS_ON_TOP, OnAlwaysOnTop)
	ON_WM_TIMER()
	ON_COMMAND(ID_RUN_TEST, OnRunTest)
	ON_COMMAND(ID_RUNTESTS, OnRuntests)
	ON_COMMAND(ID_STOPCURRENTTEST, OnStopcurrenttest)
	ON_COMMAND(ID_STOPTESTS, OnStoptests)
	ON_COMMAND(ID_PAUSETESTS, OnPausetests)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_alwaysOnTop = 0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);


	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

  cs.cx = 600;
  cs.cy = 500;

	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnAlwaysOnTop() 
{
	CMenu *menu = AfxGetMainWnd()->GetMenu();  
  RECT winRect;

  GetWindowRect(&winRect);

  if (m_alwaysOnTop) {
    menu->CheckMenuItem(MF_BYCOMMAND | ID_ALWAYS_ON_TOP, MF_UNCHECKED);
    SetWindowPos(&wndNoTopMost, 
                 winRect.left, 
                 winRect.top,
                 winRect.right - winRect.left,
                 winRect.bottom - winRect.top,
                 SWP_NOMOVE);
    m_alwaysOnTop = 0;
  } else {
    menu->CheckMenuItem(MF_BYCOMMAND | ID_ALWAYS_ON_TOP, MF_CHECKED);
    SetWindowPos(&wndTopMost, 
                 winRect.left, 
                 winRect.top,
                 winRect.right - winRect.left,
                 winRect.bottom - winRect.top,
                 SWP_NOMOVE);
    m_alwaysOnTop = 1;
  }
}


// Propogate ON_TIMER events to document
void CMainFrame::OnTimer(UINT nIDEvent) 
{
  CRunnerDoc *doc = (CRunnerDoc *)GetActiveDocument();
  doc->OnTimer(nIDEvent);
}

void CMainFrame::OnRunTest() 
{
	CRunnerDoc *doc = (CRunnerDoc *)GetActiveDocument();
  doc->OnRunTests();
}

void CMainFrame::OnRuntests() 
{
		CRunnerDoc *doc = (CRunnerDoc *)GetActiveDocument();
  doc->OnRunTests();
}

void CMainFrame::OnStopcurrenttest() 
{
	CRunnerDoc *doc = (CRunnerDoc *)GetActiveDocument();
  doc->OnStopCurrentTest();
}

void CMainFrame::OnStoptests() 
{
	CRunnerDoc *doc = (CRunnerDoc *)GetActiveDocument();
  doc->OnStopTests();
}

void CMainFrame::OnPausetests() 
{
	CRunnerDoc *doc = (CRunnerDoc *)GetActiveDocument();
  doc->OnPauseTests();
}
