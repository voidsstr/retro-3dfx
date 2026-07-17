// RunnerDoc.h : interface of the CRunnerDoc class
//
/////////////////////////////////////////////////////////////////////////////

class CRunnerDoc : public CDocument
{
protected: // create from serialization only
	CRunnerDoc();
	DECLARE_DYNCREATE(CRunnerDoc)

// Attributes
public:
	void QTests();
	void UpdateTimeoutMenu(int nSecs);
	int m_stopCurrentTest;
  int m_stopTests;
	time_t m_timeout;
  

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRunnerDoc)
	public:
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	int m_pauseTests;
	void OnPauseTests();
	void OnStopTests();
	void OnStopCurrentTest();
	void OnRunTests();
	void OnTimer(UINT timerID);
	UINT StartTimer(UINT duration);
	UINT m_timerID;
	time_t m_testStartTime;
	PROCESS_INFORMATION m_procInfo;
	int m_currentTest;
	int RunNextTest();
	const char * ExpandVariables(char *buf);
	void AddToVariables(char *buf);
	CMapStringToString *m_pVariables;
	virtual ~CRunnerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CRunnerDoc)
	afx_msg void OnFileOpen();
	afx_msg void OnLogtoFile();
	afx_msg void OnTimeoutNone();
	afx_msg void OnTimeout1();
	afx_msg void OnTimeout10();
	afx_msg void OnTimeout2();
	afx_msg void OnTimeout5();
  afx_msg void OnTimeout30();
  afx_msg void OnTimeout60();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
