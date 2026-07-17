// RunnerView.cpp : implementation of the CRunnerView class
//

#include "stdafx.h"
#include "Runner.h"

#include "RunnerDoc.h"
#include "RunnerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRunnerView

IMPLEMENT_DYNCREATE(CRunnerView, CListView)

BEGIN_MESSAGE_MAP(CRunnerView, CListView)
	//{{AFX_MSG_MAP(CRunnerView)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CListView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRunnerView construction/destruction

CRunnerView::CRunnerView()
{
	// TODO: add construction code here

}

CRunnerView::~CRunnerView()
{
}

BOOL CRunnerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CRunnerView drawing

void CRunnerView::OnDraw(CDC* pDC)
{
	CRunnerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

void CRunnerView::OnInitialUpdate()
{
    CListView::OnInitialUpdate();
    CListCtrl &lc = GetListCtrl();

    m_imageListSmall.Create(IDB_BITMAP1, 16, 10, RGB(0,0,0));
    m_imageList.Create(IDB_BITMAP1, 16, 10, RGB(0,0,0));

    lc.SetImageList(&m_imageListSmall, LVSIL_SMALL);
    lc.SetImageList(&m_imageList, LVSIL_NORMAL);

    lc.InsertColumn(0, "Test", LVCFMT_LEFT, 200, -1);
    lc.InsertColumn(1, "State", LVCFMT_LEFT, 100, -1);
    lc.InsertColumn(2, "Start Time", LVCFMT_LEFT, 100, -1);
    lc.InsertColumn(3, "End Time", LVCFMT_LEFT, 100, -1);
    lc.InsertColumn(4, "Time Left", LVCFMT_LEFT, 100, -1);

    DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
    if ((dwStyle&LVS_TYPEMASK)!=LVS_REPORT)
	  SetWindowLong(m_hWnd, GWL_STYLE, (dwStyle&~LVS_TYPEMASK)|LVS_REPORT);
}

/////////////////////////////////////////////////////////////////////////////
// CRunnerView printing

BOOL CRunnerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CRunnerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CRunnerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CRunnerView diagnostics

#ifdef _DEBUG
void CRunnerView::AssertValid() const
{
	CListView::AssertValid();
}

void CRunnerView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CRunnerDoc* CRunnerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRunnerDoc)));
	return (CRunnerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRunnerView message handlers



int CRunnerView::AddItem(const char* s)
{
    char buf[MAX_PATH];
    strcpy(buf, s);

    CListCtrl &ctrl = GetListCtrl();
    int n = ctrl.GetItemCount();
    LV_ITEM item;
    item.mask = LVIF_TEXT;
    item.iItem = n;
    item.iSubItem = 0;
    item.state = 0;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.pszText = buf;
    item.cchTextMax = sizeof(buf);
    item.iImage = 0;
    item.lParam = n;
    return ctrl.InsertItem(&item);
    
}

int CRunnerView::GetNumItems()
{
    CListCtrl &ctrl = GetListCtrl();
    return ctrl.GetItemCount();
}

int CRunnerView::GetItem(int n, char *buf)
{ 
    LV_ITEM item;

    CListCtrl &ctrl = GetListCtrl();

    item.mask = LVIF_TEXT;
    item.iItem = n;
    item.iSubItem = 0;
    item.state = 0;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.pszText = buf;
    item.cchTextMax = MAX_PATH;
    item.iImage = 0;
    item.lParam = n;

    return (ctrl.GetItem(&item));
}

void CRunnerView::SetCurrentItem(int i)
{
    LV_ITEM item;
    char buf[MAX_PATH];

    CListCtrl &ctrl = GetListCtrl();

    item.mask = LVIF_IMAGE;
    item.iItem = i;
    item.iSubItem = 0;
    item.state = 0;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.pszText = buf;
    item.cchTextMax = MAX_PATH;
    item.iImage = IMAGE_RUNNING;
    item.lParam = i;
    ctrl.SetItem(&item);

}

void CRunnerView::DeleteAllItems()
{
    CListCtrl &ctrl = GetListCtrl();
    ctrl.DeleteAllItems();
}



void CRunnerView::SetImage(int i, int whichImage)
{
    LV_ITEM item;
    char buf[MAX_PATH];

    CListCtrl &ctrl = GetListCtrl();

    item.mask = LVIF_IMAGE;
    item.iItem = i;
    item.iSubItem = 0;
    item.state = 0;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.pszText = buf;
    item.cchTextMax = MAX_PATH;
    item.iImage = whichImage;
    item.lParam = i;
    ctrl.SetItem(&item);
}


void CRunnerView::SetState(int i, char *state)
{
    LV_ITEM item;

    CListCtrl &ctrl = GetListCtrl();

    item.mask = LVIF_TEXT;
    item.iItem = i;
    item.iSubItem = 1;
    item.state = 0;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.pszText = state;
    item.cchTextMax = MAX_PATH;
    item.iImage = 0;
    item.lParam = i;
    ctrl.SetItem(&item);
}

void CRunnerView::SetStartTime(int i)
{
    LV_ITEM item;
    char buf[MAX_PATH];

    CTime theTime = CTime::GetCurrentTime();
    strcpy(buf, LPCTSTR(theTime.Format("%H:%M:%S")));

    CListCtrl &ctrl = GetListCtrl();

    item.mask = LVIF_TEXT;
    item.iItem = i;
    item.iSubItem = 2;
    item.state = 0;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.pszText = buf;
    item.cchTextMax = MAX_PATH;
    item.iImage = 0;
    item.lParam = i;
    ctrl.SetItem(&item);


}

void CRunnerView::SetEndTime(int i)
{
    LV_ITEM item;
    char buf[MAX_PATH];

    CTime theTime = CTime::GetCurrentTime();
    strcpy(buf, LPCTSTR(theTime.Format("%H:%M:%S")));

    CListCtrl &ctrl = GetListCtrl();

    item.mask = LVIF_TEXT;
    item.iItem = i;
    item.iSubItem = 3;
    item.state = 0;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.pszText = buf;
    item.cchTextMax = MAX_PATH;
    item.iImage = 0;
    item.lParam = i;
    ctrl.SetItem(&item);

}

void CRunnerView::GetFullItem(int iItem, char *outBuf)
{
  LV_ITEM item;  
  CString out;
  char buf[MAX_PATH];

  CListCtrl &ctrl = GetListCtrl();

  // Get Label
  item.mask = LVIF_TEXT;
  item.iItem = iItem;
  item.iSubItem = 0;
  item.state = 0;
  item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
  item.pszText = buf;
  item.cchTextMax = MAX_PATH;
  item.iImage = 0;
  item.lParam = iItem;

  for (int i = 0; i < 4; i++) {
    item.iSubItem = i;
    ctrl.GetItem(&item); 
    out += buf;
    out += '\t';
  }

  strcpy(outBuf, LPCTSTR(out));
}

void CRunnerView::SetTimeLeft(int iItem, char *time)
{
    LV_ITEM item;

    CListCtrl &ctrl = GetListCtrl();

    item.mask = LVIF_TEXT;
    item.iItem = iItem;
    item.iSubItem = 4;
    item.state = 0;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.pszText = time;
    item.cchTextMax = MAX_PATH;
    item.iImage = 0;
    item.lParam = iItem;
    ctrl.SetItem(&item);
}

void CRunnerView::ClearTimes(int iItem)
{
    LV_ITEM item;

    CListCtrl &ctrl = GetListCtrl();

    item.mask = LVIF_TEXT;
    item.iItem = iItem;
    item.state = 0;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.cchTextMax = MAX_PATH;
    item.iImage = 0;
    item.lParam = iItem;

    for (int i = 2; i <= 4; i++) {
      item.iSubItem = i;
      item.pszText = "";
      ctrl.SetItem(&item);
    }
}


UINT CRunnerView::GetItemState(int whichItem)
{
  LV_ITEM item;

  CListCtrl &ctrl = GetListCtrl();

  item.mask = LVIF_STATE;
  item.iItem = whichItem;
  item.iSubItem = 0;
  item.stateMask = 0xFFFF;

  ctrl.GetItem(&item);

  return item.state;
}

int CRunnerView::GetItemImage(int whichItem)
{
  LV_ITEM item;

  CListCtrl &ctrl = GetListCtrl();

  item.mask = LVIF_IMAGE;
  item.iItem = whichItem;
  item.iSubItem = 0;
  item.stateMask = 0xFFFF;

  ctrl.GetItem(&item);

  return item.iImage;
}
