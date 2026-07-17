// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "testapp1.h"
#include "sst2flds.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void Rendertosentinel(void);

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_COMMANDS_BUILDCOMMAND, OnCommandsBuildcommand)
	ON_COMMAND(ID_DEVICECONTEXT_CSGETPROTOCOLREVISION, OnDevicecontextCsgetprotocolrevision)
	ON_COMMAND(ID_GRAPHICALCONTEXT_CREATEWINDOW, OnGraphicalcontextCreatewindow)
	ON_COMMAND(ID_GRAPHICALCONTEXT_CSGETGRAPHICALCONTEXT, OnGraphicalcontextCsgetgraphicalcontext)
	ON_COMMAND(ID_GRAPHICALCONTEXT_CSRELEASEGRAPHICALCONTEXT, OnGraphicalcontextCsreleasegraphicalcontext)
	ON_COMMAND(ID_MEMORY_CSALLOC, OnMemoryCsalloc)
	ON_COMMAND(ID_MEMORY_CSFREE, OnMemoryCsfree)
	ON_COMMAND(ID_MEMORY_CSLOCK, OnMemoryCslock)
	ON_COMMAND(ID_MEMORY_CSSWAPBUFFERTODISPLAY, OnMemoryCsswapbuffertodisplay)
	ON_COMMAND(ID_MEMORY_CSUNLOCK, OnMemoryCsunlock)
	ON_COMMAND(ID_OVERLAY_CSACQUIREOVLERAY, OnOverlayCsacquireovleray)
	ON_COMMAND(ID_REGISTERS_CSREGISTERREAD, OnRegistersCsregisterread)
	ON_COMMAND(ID_REGISTERS_CSREGISTERWRITE, OnRegistersCsregisterwrite)
	ON_COMMAND(ID_VIDEOMODE_CSSETVIDEOMODE, OnVideomodeCssetvideomode)
	ON_COMMAND(ID_COMMANDS_EXECUTECOMMANDS, OnCommandsExecutecommands)
	ON_COMMAND(ID_RUN_COMPLETEINIT, OnRunCompleteinit)
	ON_COMMAND(ID_RUN_COMPLETEUNINIT, OnRunCompleteuninit)
	ON_COMMAND(ID_RUN_STARTRENDER, OnRunStartrender)
	ON_COMMAND(ID_RUN_STOPRENDERING, OnRunStoprendering)
	ON_COMMAND(ID_RUN_RENDERTOSENTINEL, OnRunRendertosentinel)
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
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

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
static CSGRAPHICALCONTEXT sGraphicalContext;
static CSALLOCATIONDESCRIPTOR sExecuteBufferDescriptor[ 3 ];
static CSALLOCATIONDESCRIPTOR sStateBufferDescriptor;
static CSALLOCATIONDESCRIPTOR sSentinelBufferDescriptor;
static CSALLOCATIONDESCRIPTOR sRenderBufferDescriptor[ 3 ];
static CSVIDEOMODE  sVideoMode;
static HWND hCSWindow = 0x00;
static WNDCLASS sCSClass;
static CSOVERLAY sOverlay;
static HANDLE hThread[ 5 ];
static HANDLE hThreadID[ 5 ];
static DWORD dwRender = FALSE;
static DWORD dwRenderThreadActive = FALSE;
static CSSWAPBUFFERTODISPLAY sSwapBufferToDisplay;
LRESULT CALLBACK csWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI fnThread0( LPVOID lpParameter );
void AllocateAndFillRenderSurfaces( void );
void ReleaseRenderSurfaces( void );

void CMainFrame::OnCommandsBuildcommand() 
{
	// TODO: Add your command handler code here
	
}

void CMainFrame::OnCommandsExecutecommands() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLExecuteCommands( &sGraphicalContext, 
        &sStateBufferDescriptor, &sExecuteBufferDescriptor[ 2 ], &sSentinelBufferDescriptor, 
        0x00, 0x00, 0x00, 0x00, 0xfe ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}


void CMainFrame::OnDevicecontextCsgetprotocolrevision() 
{
    CSRESULT idResult;
    FxU32 u32Minor;
    FxU32 u32Major;
    if( ( idResult = csDLLGetProtocolRevision( ( CSHDC )GetDC(), &u32Minor, 
        &u32Major ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnGraphicalcontextCreatewindow() 
{
    static bool bClassCreated = FALSE;
    
    if( !bClassCreated )
    {
        sCSClass.style         = CS_HREDRAW | CS_VREDRAW;
        sCSClass.lpfnWndProc   = csWndProc;
        sCSClass.cbClsExtra    = 0;
        sCSClass.cbWndExtra    = 0x00;
        sCSClass.hInstance     = AfxGetInstanceHandle();
        sCSClass.hIcon         = NULL;
        sCSClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        sCSClass.hbrBackground = ( HBRUSH )GetStockObject(BLACK_BRUSH);
        sCSClass.lpszMenuName  = NULL;
        sCSClass.lpszClassName = "csWindowClass";
        if( !::RegisterClass( &sCSClass ) ) 
        {   
            ::MessageBox( NULL, ( LPCTSTR )"Failed To Create The Window Class", 
                "TestApp1", MB_OK );
            return;
        };

        bClassCreated = TRUE;
    }

    hCSWindow = CreateWindow("csWindowClass",
                        "csWindow1",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        320,
                        240,
                        m_hWnd,
                        NULL,
                        AfxGetInstanceHandle(),
                        NULL);

    ::ShowWindow( hCSWindow, SW_NORMAL );
}

void CMainFrame::OnGraphicalcontextCsgetgraphicalcontext() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLGetGraphicalContext( ( CSHDC )GetDC(), &sGraphicalContext ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnGraphicalcontextCsreleasegraphicalcontext() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLReleaseGraphicalContext( &sGraphicalContext ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnMemoryCsalloc() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLAlloc( &sGraphicalContext, &sExecuteBufferDescriptor[ 2 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnMemoryCsfree() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLFree( &sGraphicalContext, &sExecuteBufferDescriptor[ 2  ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnMemoryCslock() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLLock( &sGraphicalContext, &sExecuteBufferDescriptor[ 2  ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnMemoryCsswapbuffertodisplay() 
{
    CSRESULT idResult;
    CSSWAPBUFFERTODISPLAY sSwapBufferToDisplay;
    memset( &sSwapBufferToDisplay, 0x00, sizeof( CSSWAPBUFFERTODISPLAY ) );
    sSwapBufferToDisplay.psSrcBufferAllocationDescriptor = &sRenderBufferDescriptor[ 2 ];

    if( ( idResult = csDLLSwapBufferToDisplay( &sGraphicalContext, &sSwapBufferToDisplay ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnMemoryCsunlock() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLUnlock( &sGraphicalContext, &sExecuteBufferDescriptor[ 2  ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnOverlayCsacquireovleray() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLAcquireOverlay( &sGraphicalContext, &sOverlay ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnRegistersCsregisterread() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLDeviceSpecificCommunication( &sGraphicalContext, CSSST2DEVICESPECIFIC_REGISTERREAD ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnRegistersCsregisterwrite() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLDeviceSpecificCommunication( &sGraphicalContext, CSSST2DEVICESPECIFIC_REGISTERWRITE ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}


void CMainFrame::OnVideomodeCssetvideomode() 
{
    CSRESULT idResult;
    if( ( idResult = csDLLSetVideoMode( &sGraphicalContext, &sVideoMode, ( CSWINDOWID) hCSWindow ) ) != CS_SUCCESS )
    {
        MessageBox( ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}


LRESULT CALLBACK csWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
             break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
            }  
            break;

        case WM_DESTROY:         

            // stop the thread from processing 
            dwRender = FALSE;

            // close the window
            ::DestroyWindow(hWnd);

            break;

        default:
            return(::DefWindowProc(hWnd, msg, wParam, lParam));
    }
    return(0);
}


void CMainFrame::OnRunCompleteinit() 
{
    static bool bClassCreated = FALSE;
    CSRESULT idResult;

 
    if( !bClassCreated )
    {
        sCSClass.style         = CS_HREDRAW | CS_VREDRAW;
        sCSClass.lpfnWndProc   = csWndProc;
        sCSClass.cbClsExtra    = 0;
        sCSClass.cbWndExtra    = 0x00;
        sCSClass.hInstance     = AfxGetInstanceHandle();
        sCSClass.hIcon         = NULL;
        sCSClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        sCSClass.hbrBackground = ( HBRUSH )GetStockObject(BLACK_BRUSH);
        sCSClass.lpszMenuName  = NULL;
        sCSClass.lpszClassName = "csWindowClass";
        if( !::RegisterClass( &sCSClass ) ) 
        {   
            ::MessageBox( NULL, ( LPCTSTR )"Failed To Create The Window Class", 
                "TestApp1", MB_OK );
            return;
        };

        bClassCreated = TRUE;
    }

    hCSWindow = CreateWindow("csWindowClass",
                        "csWindow1",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        640,
                        480,
                        m_hWnd,
                        NULL,
                        AfxGetInstanceHandle(),
                        NULL);

    ::ShowWindow( hCSWindow, SW_NORMAL );

    memset( &sGraphicalContext, 0x00, sizeof( sGraphicalContext ) );
    if( ( idResult = csDLLGetGraphicalContext( ( CSHDC )GetDC(), &sGraphicalContext ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    /* Allocate buffers required to render */
    memset( &sExecuteBufferDescriptor[ 0 ], 0x00, sizeof( CSALLOCATIONDESCRIPTOR ) );
    sExecuteBufferDescriptor[ 0 ].u32BufferType = CS_BUFFER_FIFO;
    sExecuteBufferDescriptor[ 0 ].u32MemType = CS_MEMORY_LINEAR;
    sExecuteBufferDescriptor[ 0 ].u32Locale = CS_LOCALE_FRAMEBUFFER;
    sExecuteBufferDescriptor[ 0 ].u32Size = 256 * sizeof( DWORD );
    if( ( idResult = csDLLAlloc( &sGraphicalContext, &sExecuteBufferDescriptor[ 0 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
    memset( &sExecuteBufferDescriptor[ 1 ], 0x00, sizeof( CSALLOCATIONDESCRIPTOR ) );
    sExecuteBufferDescriptor[ 1 ].u32BufferType = CS_BUFFER_FIFO;
    sExecuteBufferDescriptor[ 1 ].u32MemType = CS_MEMORY_LINEAR;
    sExecuteBufferDescriptor[ 1 ].u32Locale = CS_LOCALE_FRAMEBUFFER;
    sExecuteBufferDescriptor[ 1 ].u32Size = 256 * sizeof( DWORD );
    if( ( idResult = csDLLAlloc( &sGraphicalContext, &sExecuteBufferDescriptor[ 1 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
    memset( &sSentinelBufferDescriptor, 0x00, sizeof( CSALLOCATIONDESCRIPTOR ) );
    sSentinelBufferDescriptor.u32BufferType = CS_BUFFER_PERSISTENT;
    sSentinelBufferDescriptor.u32MemType = CS_MEMORY_LINEAR;
    sSentinelBufferDescriptor.u32Locale = CS_LOCALE_FRAMEBUFFER;
    sSentinelBufferDescriptor.u32Size = sizeof( DWORD );
    if( ( idResult = csDLLAlloc( &sGraphicalContext, &sSentinelBufferDescriptor ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
    memset( &sStateBufferDescriptor, 0x00, sizeof( CSALLOCATIONDESCRIPTOR ) );
    sStateBufferDescriptor.u32BufferType = CS_BUFFER_PERSISTENT;
    sStateBufferDescriptor.u32MemType = CS_MEMORY_LINEAR;
    sStateBufferDescriptor.u32Locale = CS_LOCALE_FRAMEBUFFER;
    sStateBufferDescriptor.u32Size = sizeof( DWORD );
    if( ( idResult = csDLLAlloc( &sGraphicalContext, &sStateBufferDescriptor ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    AllocateAndFillRenderSurfaces();
}

void CMainFrame::OnRunCompleteuninit() 
{
    CSRESULT idResult;

    if( ( idResult = csDLLFree( &sGraphicalContext, &sExecuteBufferDescriptor[ 0 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
    if( ( idResult = csDLLFree( &sGraphicalContext, &sExecuteBufferDescriptor[ 1 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    ReleaseRenderSurfaces();

    if( ( idResult = csDLLFree( &sGraphicalContext, &sSentinelBufferDescriptor ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
	
    if( ( idResult = csDLLReleaseGraphicalContext( &sGraphicalContext ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void CMainFrame::OnRunStartrender() 
{
    dwRender = TRUE; 
    hThread[ 0 ] = CreateThread( NULL, 0, fnThread0, 0, 0, ( LPDWORD  )&hThreadID[ 0 ] );
    dwRenderThreadActive = TRUE;

}

void CMainFrame::OnRunStoprendering() 
{
    dwRender = FALSE;
}


DWORD WINAPI fnThread0(  LPVOID lpParameter   )
{
    DWORD dwIndex = 0x00;
    CSRESULT idResult;

    /* Setup the current fifo with data to write to the sentinel and wait for that 
       action to happen */
    if( ( idResult = csDLLLock( &sGraphicalContext, &sExecuteBufferDescriptor[ 0 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
    if( ( idResult = csDLLLock( &sGraphicalContext, &sExecuteBufferDescriptor[ 1 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    if( ( idResult = csDLLLock( &sGraphicalContext, &sSentinelBufferDescriptor ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    while( dwRender )
    {
        sSwapBufferToDisplay.psSrcBufferAllocationDescriptor = &sRenderBufferDescriptor[ dwIndex ];
        sSwapBufferToDisplay.idDestWindow = ( CSWINDOWID )hCSWindow;
        if( ( idResult = csDLLSwapBufferToDisplay( &sGraphicalContext, &sSwapBufferToDisplay ) ) != CS_SUCCESS )
        {
            ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
                "Central Services Error", MB_OK );
        }


        dwIndex ^= 0x01;

        Rendertosentinel();
    }

    if( ( idResult = csDLLUnlock( &sGraphicalContext, &sExecuteBufferDescriptor[ 0 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    if( ( idResult = csDLLUnlock( &sGraphicalContext, &sExecuteBufferDescriptor[ 1 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

  
    if( ( idResult = csDLLUnlock( &sGraphicalContext, &sSentinelBufferDescriptor ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }


    dwRenderThreadActive = FALSE;
    return( 0 );
}

void CMainFrame::OnRunRendertosentinel() 
{
    Rendertosentinel();
}

void Rendertosentinel(void)
{
    CSRESULT idResult;
    DWORD * pdwTemp;
    static DWORD dwColor = 0x00;


    /* Setup the current fifo with data to write to the sentinel and wait for that 
       action to happen */
    if( ( idResult = csDLLLock( &sGraphicalContext, &sExecuteBufferDescriptor[ 0 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    if( ( idResult = csDLLLock( &sGraphicalContext, &sExecuteBufferDescriptor[ 1 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    if( ( idResult = csDLLLock( &sGraphicalContext, &sSentinelBufferDescriptor ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
	

    // put a NOP in the command stream, execute the buffer and wait for the sentinel
    *( DWORD * )sExecuteBufferDescriptor[ 0 ].pvLinearAddress = 0x00;


    pdwTemp = ( DWORD * )sExecuteBufferDescriptor[ 1 ].pvLinearAddress;
    
    *pdwTemp++ = 0x3c0c0002;
    *pdwTemp++ = 0xffffffff;
    *pdwTemp++ = 0xffffffff;
    *pdwTemp++ = dwColor++;
    *pdwTemp++ = 0x00640064;
    *pdwTemp++ = 0x00320032;
    *pdwTemp++ = 0xf0002105;
    *( DWORD * )sSentinelBufferDescriptor.pvLinearAddress = 0x00;


    if( ( idResult = csDLLExecuteCommands( &sGraphicalContext, 
        &sExecuteBufferDescriptor[ 0 ], &sExecuteBufferDescriptor[ 1 ], 
        &sSentinelBufferDescriptor, 0x00, 0x00, 0x01 * sizeof( DWORD ), 
        0x07 * sizeof( DWORD ), 0xffee ) ) != CS_SUCCESS )
    {
//        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
//            "Central Services Error", MB_OK );

        ReleaseRenderSurfaces();
        AllocateAndFillRenderSurfaces();
    }
    else
    {
        while( *( DWORD * )sSentinelBufferDescriptor.pvLinearAddress != 0xffee );
    }     

    if( ( idResult = csDLLUnlock( &sGraphicalContext, &sExecuteBufferDescriptor[ 1 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    if( ( idResult = csDLLUnlock( &sGraphicalContext, &sExecuteBufferDescriptor[ 0 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    if( ( idResult = csDLLUnlock( &sGraphicalContext, &sSentinelBufferDescriptor ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}

void AllocateAndFillRenderSurfaces( void )
{
    CSRESULT idResult;

    memset( &sRenderBufferDescriptor[ 0 ], 0x00, sizeof( CSALLOCATIONDESCRIPTOR ) );
    sRenderBufferDescriptor[ 0 ].u32BufferType = CS_BUFFER_RENDER;
    sRenderBufferDescriptor[ 0 ].u32MemType = CS_MEMORY_LINEAR;
    sRenderBufferDescriptor[ 0 ].u32Locale = CS_LOCALE_FRAMEBUFFER;
    sRenderBufferDescriptor[ 0 ].u32Size = 640 * 480 * 3;
    if( ( idResult = csDLLAlloc( &sGraphicalContext, &sRenderBufferDescriptor[ 0 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    sRenderBufferDescriptor[ 0 ].u32LFBDepth = 0x18;
    sRenderBufferDescriptor[ 0 ].u32PhysicalStride = 640*3;

    memset( &sRenderBufferDescriptor[ 1 ], 0x00, sizeof( CSALLOCATIONDESCRIPTOR ) );
    sRenderBufferDescriptor[ 1 ].u32BufferType = CS_BUFFER_RENDER;
    sRenderBufferDescriptor[ 1 ].u32MemType = CS_MEMORY_LINEAR;
    sRenderBufferDescriptor[ 1 ].u32Locale = CS_LOCALE_FRAMEBUFFER;
    sRenderBufferDescriptor[ 1 ].u32Size = 640 * 480 * 3;
    if( ( idResult = csDLLAlloc( &sGraphicalContext, &sRenderBufferDescriptor[ 1 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }

    sRenderBufferDescriptor[ 1 ].u32LFBDepth = 0x18;
    sRenderBufferDescriptor[ 1 ].u32PhysicalStride = 640*3;


    /* load bitmaps from disk and copy them into the render buffers for testing */
    {
        FILE * psFile;
        if( ( psFile = fopen( "test1.bmp", "r" ) ) )
        {
            if( csDLLLock( &sGraphicalContext, &sRenderBufferDescriptor[ 0 ] ) == CS_SUCCESS )
            {

                fread( ( char * )sRenderBufferDescriptor[ 0 ].pvLinearAddress, sizeof( BITMAPFILEHEADER )
                    + sizeof( BITMAPINFOHEADER ) , 1, psFile );
                fread( ( char * )sRenderBufferDescriptor[ 0 ].pvLinearAddress, 1, 640*480*3, psFile );
                csDLLUnlock( &sGraphicalContext, &sRenderBufferDescriptor[ 0 ] );
                fclose( psFile );
            }
        }

        if( ( psFile = fopen( "test2.bmp", "r" ) ) )
        {
            if( csDLLLock( &sGraphicalContext, &sRenderBufferDescriptor[ 1 ] ) == CS_SUCCESS )
            {
                fread( ( char * )sRenderBufferDescriptor[ 1 ].pvLinearAddress, sizeof( BITMAPFILEHEADER )
                    + sizeof( BITMAPINFOHEADER ), 1, psFile );
                fread( ( char * )sRenderBufferDescriptor[ 1 ].pvLinearAddress, 1, 640*480*3, psFile );
                csDLLUnlock( &sGraphicalContext, &sRenderBufferDescriptor[ 1 ] );
                fclose( psFile );
            }
        }
    }
}

void ReleaseRenderSurfaces( void )
{
    CSRESULT idResult;

    if( ( idResult = csDLLFree( &sGraphicalContext, &sRenderBufferDescriptor[ 0 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
    if( ( idResult = csDLLFree( &sGraphicalContext, &sRenderBufferDescriptor[ 1 ] ) ) != CS_SUCCESS )
    {
        ::MessageBox( NULL, ( LPCTSTR )csDLLDEBUGGetStringForError( idResult ), 
            "Central Services Error", MB_OK );
    }
}