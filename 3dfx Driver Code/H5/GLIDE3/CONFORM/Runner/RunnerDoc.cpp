// RunnerDoc.cpp : implementation of the CRunnerDoc class
//

#include "stdafx.h"
#include "Runner.h"

#include "RunnerDoc.h"
#include "RunnerView.h"

#define NOTYETRUN "Not Yet Run"
#define PENDING   "Q'd - Not Yet Run"
#define RUNNING   "Running"
#define KILLED    "Killed"
#define STOPPED   "Stopped"
#define TIMED_OUT "Timed Out"
#define COMPLETED "Completed"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRunnerDoc

IMPLEMENT_DYNCREATE(CRunnerDoc, CDocument)

BEGIN_MESSAGE_MAP(CRunnerDoc, CDocument)
    //{{AFX_MSG_MAP(CRunnerDoc)
    ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
    ON_COMMAND(ID_LOGTOFILE, OnLogtoFile)
    ON_COMMAND(ID_TIMEOUT_NONE, OnTimeoutNone)
    ON_COMMAND(ID_TIMEOUT_1, OnTimeout1)
    ON_COMMAND(ID_TIMEOUT_10, OnTimeout10)
    ON_COMMAND(ID_TIMEOUT_2, OnTimeout2)
    ON_COMMAND(ID_TIMEOUT_5, OnTimeout5)
    ON_COMMAND(ID_TIMEOUT_30, OnTimeout30)
    ON_COMMAND(ID_TIMEOUT_60, OnTimeout60)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRunnerDoc construction/destruction

CRunnerDoc::CRunnerDoc()
{
  m_timeout = 5 * 60; // 5 minute default timeout.
  m_stopTests = 0;
  m_stopCurrentTest = 0;
  m_pVariables = 0;
  m_pauseTests = 0;
  m_currentTest = 0;
}

CRunnerDoc::~CRunnerDoc()
{
}



/////////////////////////////////////////////////////////////////////////////
// CRunnerDoc serialization

void CRunnerDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: add storing code here
    }
    else
    {
        // TODO: add loading code here
    }
}

/////////////////////////////////////////////////////////////////////////////
// CRunnerDoc diagnostics

#ifdef _DEBUG
void CRunnerDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CRunnerDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG


// Super primitive parser for 'set FOO=bar' type lines
void CRunnerDoc::AddToVariables(char *buf)
{
  //AfxMessageBox(buf);
  //parse the line into x = y
  char var[128], val[128];
  char *cptr = buf;
  char *dest = var;
  int state = 0;

  while (*cptr && state < 4) {
    switch (state) {
    case 0: // nothing
      if (*cptr == ' ') {
        while (*cptr == ' ') ++cptr;
        state = 1; 
        *dest++ = *cptr;
      }
      break;
    case 1: // variable name
      if (*cptr == '=') {
        state = 2;
        *dest = 0;
        dest = val;
      } else {
        *dest++ = *cptr;
      }
      break;
    case 2: // leading spaces before value
      if (*cptr == ' ') {        
      } else {
        *dest++ = *cptr;
        state = 3;
      }
      break;
    case 3: // value
      if (*cptr == '\n' || *cptr == '#') { // end of value
        *dest = 0;
        state = 4;
        break;
      } else {
        *dest++ = *cptr;
      }
      break;
    }
    ++cptr;
  }
  *dest = 0;

  m_pVariables->SetAt(var, val);

  //char msg[512];
  //sprintf(msg, "'%s' == '%s'", var, val);
  //AfxMessageBox(msg);
  
}
/////////////////////////////////////////////////////////////////////////////
// CRunnerDoc commands

void CRunnerDoc::OnFileOpen() 
{
  char buf[MAX_PATH];
  char *szFilter = 
  "Runner files (*.run)|*.run|Batch files (*.bat)|*.bat|Text files (*.txt)|*.txt";

    CFileDialog *fd = new CFileDialog(TRUE, ".run", NULL,
                                      OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                                      szFilter,
                                      NULL);
  if (fd->DoModal() == 2) {
    return; // cancel was pressed
  }

  if (m_pVariables) {
    delete m_pVariables;    
  }

  m_pVariables = new CMapStringToString(5);

  POSITION pos = GetFirstViewPosition();
  CRunnerView* pView = (CRunnerView *)GetNextView( pos );
  pView->DeleteAllItems();   

  FILE *fp = fopen(fd->GetPathName(), "r");
  if (fp) {
    while (!feof(fp) && !ferror(fp)) {
      fgets(buf, sizeof(buf), fp);

      if (!strncmp(buf, "REM", 3) ||!strncmp(buf, "#", 1)) {
        // Skip comments
        continue;
      }

      if (!strncmp(buf, "set", 3)) {
        // TODO: add to list of variable substitutions.
        // TODO: substitue variables if available
        // Parse
        AddToVariables(buf);

      } else  if (strlen(buf) > 1) {
          // Substitute variables, and insert into list
          pView->SetState(pView->AddItem(ExpandVariables(buf)), NOTYETRUN);
      }
    } 
    fclose(fp);
  } else {
    AfxMessageBox("Couldn't open file!");
  }
}

// http://www.codeguru.com/misc/MFCTimer.shtml

UINT  CRunnerDoc::StartTimer (UINT TimerDuration)
{
   UINT    TimerVal;

   TimerVal = AfxGetMainWnd()->SetTimer (IDT_TIMER_0, TimerDuration, NULL);
   if (TimerVal == 0) {
      AfxMessageBox ("Unable to obtain timer");
   }

   return TimerVal;

}// end StartTimer



void CRunnerDoc::OnLogtoFile() 
{
  char buf[MAX_PATH];
  CFileDialog *fd = new CFileDialog(TRUE);
  
  if (fd->DoModal() == 2) {
    return; // cancel was pressed
  }
   
  FILE *fp = fopen(fd->GetPathName(), "w+");
  delete *fd;

  if (fp) {
      POSITION pos = GetFirstViewPosition();
    CRunnerView* pView = (CRunnerView *)GetNextView( pos );
    int n = pView->GetNumItems();
    for (int i = 0; i < n; i++) {
      pView->GetFullItem(i, buf);
      fprintf(fp, "%s\n", buf);
      //AfxMessageBox(buf);
    }
    fclose(fp);
  } else {
    AfxMessageBox("Couldn't open file!");
  }    
}

// These are lame, I know.

void CRunnerDoc::OnTimeout1()    {UpdateTimeoutMenu(1);} // 1 min
void CRunnerDoc::OnTimeout2()    {UpdateTimeoutMenu(2);} // 2 min
void CRunnerDoc::OnTimeout5()    {UpdateTimeoutMenu(5);} // 5 min
void CRunnerDoc::OnTimeout10()   {UpdateTimeoutMenu(10);} // 10 min
void CRunnerDoc::OnTimeout30()   {UpdateTimeoutMenu(30);} // 30 min
void CRunnerDoc::OnTimeout60()   {UpdateTimeoutMenu(60);} // 60 min
void CRunnerDoc::OnTimeoutNone() {UpdateTimeoutMenu(24*60);} // large... 24 hours


void CRunnerDoc::UpdateTimeoutMenu(int which)
{

  // set/unset checkmarks in the menu items
  CMenu *menu = AfxGetMainWnd()->GetMenu();  
  menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_NONE, MF_UNCHECKED);
  menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_1, MF_UNCHECKED);
  menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_2, MF_UNCHECKED);
  menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_5, MF_UNCHECKED);
  menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_10, MF_UNCHECKED);
  menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_30, MF_UNCHECKED);
  menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_60, MF_UNCHECKED);

  m_timeout = which * 60; 

  switch(which) {
  case 1:
    menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_1, MF_CHECKED);
    break;
  case 2:
    menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_2, MF_CHECKED);
    break;
  case 5:
    menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_5, MF_CHECKED);
    break;
  case 10:
    menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_10, MF_CHECKED);
    break;
  case 30:
    menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_30, MF_CHECKED);
    break;
  case 60:
    menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_60, MF_CHECKED);
    break;
  case 99:
    menu->CheckMenuItem(MF_BYCOMMAND | ID_TIMEOUT_NONE, MF_CHECKED);
    break;    
  }


}

void CRunnerDoc::OnStopCurrentTest() 
{
  m_stopCurrentTest = 1;    
}

void CRunnerDoc::QTests()
{
  POSITION pos = GetFirstViewPosition();
  CRunnerView* pView = (CRunnerView *)GetNextView( pos );
  int numSelected = 0;

  int n = pView->GetNumItems();  

  // Loop through and set any selected ones to pending. If none are
  // selected, Q them all.
  for (int i = 0; i < n; i++) {
    if (pView->GetItemState(i) & LVIS_SELECTED) {
      //AfxMessageBox("Selected!");
      pView->SetState(i, PENDING);
      pView->SetImage(i, IMAGE_PENDING);
      pView->ClearTimes(i);
      numSelected++;
    } else {
      pView->SetImage(i, IMAGE_READ_IN);
      pView->SetState(i, NOTYETRUN);
      pView->ClearTimes(i);
    }
  }

  if (!numSelected) {  // none were selected, so Q them all...
    for (int i = 0; i < n; i++) {
      pView->SetState(i, PENDING);
      pView->SetImage(i, IMAGE_PENDING);
      pView->ClearTimes(i);
    }
  }
}


// Expand out any variables in 'buf'.
// Eg. 'test %ARGS' becomes 'test -res 800x600', if 'ARGS' is '-res 800x600'
// 'buf' is modified in place.
const char * CRunnerDoc::ExpandVariables(char *buf)
{
  char tmp_buf[512];
  char var_name[64];
  CString var, val;
  int numVars = m_pVariables->GetCount();
  POSITION pos = m_pVariables->GetStartPosition();

  
  // Sucky. VC4.2 doesn't support CString::Replace(). Sigh.
  // for (int i = 0; i < numVars; i++) {
  //   m_pVariables->GetNextAssoc(pos, var, val);      
  //   out.Replace(var, val);
  // }
  char *cptr = buf;
  char *outptr = tmp_buf;
  char *vptr = var_name;
  int state = 0;

  while (*cptr) {
    switch (state) {
    case 0: // scanning
      if (*cptr == '%') {
        state = 1;
        *outptr++ = 0;
        vptr = var_name;
      } else if (*cptr == '\n') {
        *outptr++ = 0;
      } else {
        *outptr++ = *cptr;
      }
      break;
    case 1: // variable name
      if (*cptr == ' ' || *cptr == '%' || *cptr == '\n') { // end of variable name
        state = 2; 
        *vptr = 0;
        CString val;
        if (m_pVariables->Lookup(var_name, val)) {
          // found it. put the substitution in.
          strcat(tmp_buf, val);
        } else {
          // couldn't find it, put the raw version in.
          strcat(tmp_buf, "%");
          strcat(tmp_buf, var_name);
        }

        strcat(tmp_buf, " ");

        outptr = &tmp_buf[strlen(tmp_buf)]; // put the pointer at the end of the string

        state = 0;
      } else {
        *vptr++ = *cptr;
      }
      break;
    }
    ++cptr;
  }
  *outptr = 0;

  strcpy(buf, tmp_buf);
  return buf;

}

// Run the next test that is Q'd.
int CRunnerDoc::RunNextTest()
{
    // Only attempt to run the tests that were queued. Ickily enough,
    // we're using the image as the state.

  int done = 0;
  int retval = 0;
  char buf[MAX_PATH];
  STARTUPINFO startInfo;
  POSITION pos = GetFirstViewPosition();

  CRunnerView* pView = (CRunnerView *)GetNextView( pos );

  int numItems = pView->GetNumItems();
  int test = m_currentTest;

  while (!done) {

    if (test >= numItems) {
      m_stopTests = 1; // end of tests
      return retval;
    }

    if (pView->GetItemImage(test) & IMAGE_PENDING) {
      pView->GetItem(test, buf);

      startInfo.cb = sizeof(startInfo);
      startInfo.lpReserved = NULL;
      startInfo.lpDesktop = NULL;
      startInfo.lpTitle = NULL;
      startInfo.dwX = 0;
      startInfo.dwY = 0;
      startInfo.dwXSize = 0; 
      startInfo.dwYSize = 0;
      startInfo.dwXCountChars = 0;
      startInfo.dwYCountChars = 0;
      startInfo.dwFillAttribute = 0;
      startInfo.dwFlags = 0;
      startInfo.wShowWindow = 0;
      startInfo.cbReserved2 = 0;
      startInfo.lpReserved2 = 0;
      startInfo.hStdInput = NULL;
      startInfo.hStdOutput = NULL;
      startInfo.hStdError = NULL;

      pView->SetStartTime(test);
      pView->SetState(test, "Running");
      pView->SetImage(test, 1);

      if (CreateProcess(NULL,
                        buf, // command line
                        NULL, NULL, // security
                        FALSE,      // don't inherit filehandles
                        NORMAL_PRIORITY_CLASS, // dwCreationFlags
                        NULL, // inherit environment
                        NULL, // same cwd
                        &startInfo,
                        &m_procInfo)) {
        time(&m_testStartTime); // get the start time
        m_currentTest = test;
        done = 1;
        retval = 1;
      } else {
        strcpy(buf, "Failed to start");        
        pView->SetEndTime(test);
        pView->SetState(test, "Failed to start");
        pView->SetImage(test, 2);        
        m_currentTest = -1;
      }
    }    
    test++; // try the next one.
  }

  return retval;
}

void CRunnerDoc::OnRunTests() 
{
  m_stopTests = 0;

  if (!m_pauseTests) {
    m_currentTest = 0; // beginning
    QTests();          // set selected ones to pending
  }

  m_pauseTests = 0;

  if (RunNextTest()) {
    StartTimer(2000);
  }
}

void CRunnerDoc::OnTimer(UINT timerVal)
{
  char tbuf[64];
  int testDone = 0;
  CTimeSpan *timeLeft;  
  POSITION pos = GetFirstViewPosition();
  time_t elapsed;
  DWORD exitCode = STILL_ACTIVE;

  AfxGetMainWnd()->KillTimer(timerVal); // kill it for the moment.

  if (m_currentTest < 0) {
    return;
  }

  CRunnerView* pView = (CRunnerView *)GetNextView( pos );
         
  GetExitCodeProcess(m_procInfo.hProcess, &exitCode);
  pView->SetEndTime(m_currentTest);

  elapsed = time(NULL) - m_testStartTime; // elapsed time
  timeLeft = new CTimeSpan(m_timeout - elapsed);
  strcpy(tbuf, LPCTSTR(timeLeft->Format("%H:%M:%S")));
  pView->SetTimeLeft(m_currentTest, tbuf);
  delete timeLeft;
           
  if (exitCode != STILL_ACTIVE) {
    pView->SetState(m_currentTest, COMPLETED);
    pView->SetImage(m_currentTest, IMAGE_COMPLETED);
    testDone = 1;
  } else if (elapsed > m_timeout) {
    // timed out, kill it
    TerminateProcess(m_procInfo.hProcess, 99);              
    pView->SetState(m_currentTest, TIMED_OUT);
    pView->SetImage(m_currentTest, IMAGE_STOPPED);
    testDone = 1;
  } else if (m_stopCurrentTest) { // stop all tests
    TerminateProcess(m_procInfo.hProcess, 99);
    pView->SetState(m_currentTest, KILLED);
    pView->SetImage(m_currentTest, IMAGE_STOPPED);
    m_stopCurrentTest = 0;
    testDone = 1;
  } else if (m_stopTests) {
    TerminateProcess(m_procInfo.hProcess, 99);
    pView->SetState(m_currentTest, STOPPED);
    pView->SetImage(m_currentTest, IMAGE_STOPPED);
    testDone = 1;
  }  

  int start_timer = 1; 
  if (testDone && !m_stopTests && !m_pauseTests) {
    if (!RunNextTest()) {
       start_timer = 0; // don't need a new timer if there's
                        // test running
    }
  }

  if (start_timer) {
    m_timerID = StartTimer(2000); //2 sec timer
    if (!m_timerID) {
      AfxMessageBox ("Unable to obtain timer");
    }       
  }
}

void CRunnerDoc::OnStopTests() 
{    
  m_stopTests = 1;    
  m_pauseTests = 0;
}

void CRunnerDoc::OnPauseTests()
{
  m_pauseTests = 1;
}
