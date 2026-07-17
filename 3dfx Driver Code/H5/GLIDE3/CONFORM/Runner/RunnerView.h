// RunnerView.h : interface of the CRunnerView class
//
/////////////////////////////////////////////////////////////////////////////

class CRunnerView : public CListView
{
protected: // create from serialization only
	CRunnerView();
	DECLARE_DYNCREATE(CRunnerView)

// Attributes
public:
	int GetItemImage(int whichItem);
	UINT GetItemState(int whichItem);
	void SetTimeLeft(int iItem, char *time);
	void GetFullItem(int i, char *buf);
	void SetEndTime(int i);
	void SetStartTime(int i);
	void SetState(int i, char *state);
        void SetImage(int i, int whichImage);
	void DeleteAllItems();
	void SetCurrentItem(int i);
        void ClearTimes(int iItem);
	int GetItem(int n, char *buf);
	int GetNumItems();
	int AddItem(const char* s);
	CImageList m_imageListSmall;
    CImageList m_imageList;
	CRunnerDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRunnerView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRunnerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CRunnerView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in RunnerView.cpp
inline CRunnerDoc* CRunnerView::GetDocument()
   { return (CRunnerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
