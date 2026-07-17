/*	
	FILE:	wgllib.cpp

	Implementation of functions that facilitate the 
	use of the wgl API.

	Paul Mayfield, 7/2/96

*/

#include "winlib.h"
#include "stdio.h"

//============================================================
//============================================================
// GLOBALS AND FUNCTIONS USED INTERNALLY BY THE LIBRARY
//============================================================
//============================================================
HINSTANCE wlHinst = NULL;
int wlWindowCount=0;
char * wlPropID = "wl id property";

void wlError(char * error) {
	wlSetLastError(error);
}

void wlAddWindowParam(HWND hwnd, DWORD data, char * id) {
	SetProp(hwnd, id, (HANDLE)data);
}

void wlDelWindowParam(HWND hwnd, char * id) {
	RemoveProp(hwnd, id);
}

DWORD wlGetWindowParam(HWND hwnd, char * id) {
	return (DWORD)GetProp(hwnd, id);
}


//============================================================
//============================================================
//============================================================
//============================================================
wlWindowInfo::wlWindowInfo(HINSTANCE hinst) {
	// INITIALIZE THE CLASS NAME 
	char * name = "wlDefaultWindow";
	char * ClassName = new char [lstrlen(name)+1];
	lstrcpy(ClassName, name);

	// AND WINDOW TITLE
	char * title = "WGL Library Window";
	Title = new char [lstrlen(title)+1];
	lstrcpy(Title, title);

	// INITIALIZE THE WINDOW CLASS
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)wlWindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hinst;
    wc.hIcon         = LoadIcon(GetModuleHandle(NULL), "GLUT_ICON");
    if(!wc.hIcon) 
	wc.hIcon     = LoadIcon(NULL, IDI_APPLICATION);
    // must set the hCursor to NULL since we might change it
    // in the course of the program.  See the SetCursor() man
    // page in Visual C++
    wc.hCursor       = NULL;  // LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = ClassName;

	// INITIALIZE THE CREATE WINDOW PARAMETERS
	dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	x=CW_USEDEFAULT; 
	y=CW_USEDEFAULT;  
	nWidth=CW_USEDEFAULT; 
	nHeight=CW_USEDEFAULT; 
	hWndParent=NULL;
	hMenu=NULL;
	hinstance = hinst;
	lpParam=NULL;
	
	// FINALLY, INITIALIZE THE SHOW COMMAND
	nCmdShow = SW_SHOW;
}

wlWindowInfo::~wlWindowInfo() {
	if (Title) 
		delete [] Title;
//	if (wc.lpszClassName)
//		delete [] wc.lpszClassName;
}

void wlWindowInfo::SetWCName(char * newname) {
	//if (wc.lpszClassName)
	//	delete [] wc.lpszClassName;
	wc.lpszClassName=0;
	if (!newname)
		return;
	wc.lpszClassName = new char[lstrlen(newname)+1];
	lstrcpy((char*)wc.lpszClassName, newname);
}

void wlWindowInfo::SetTitle(char * newtitle) {
	if (Title)
		delete [] Title;
	Title=0;
	if (!newtitle)
		return;
	Title = new char[lstrlen(newtitle)+1];
	lstrcpy(Title, newtitle);
}

// INITIALIZATION 
int WINAPI wlInitialize	(HINSTANCE hinst) {
	wlHinst = hinst;
	return 1;
}

// WINDOW CREATION FUNCTIONS
HWND WINAPI wlCreateWindow	(char * title, wlWindowInfo * wi) {
	static wlWindowInfo DefaultInfo;
	WNDCLASS wc;

	// REGISTER THE WINDOW CLASS IF NEEDED
	if (!wi) {
		DefaultInfo.SetHinst(wlGetLibraryModule());
		// MAKE SURE THE WINDOW CLASS HAS NOT ALREADY BEEN REGISTERED
		if (!GetClassInfo(wlGetLibraryModule(), DefaultInfo.GetWC()->lpszClassName, &wc)) {
			if (!RegisterClass(DefaultInfo.GetWC())) {
				wlError("Default Class Registration Failure");
				return 0;
			}
		}
	}
	else {
		// MAKE SURE THE WINDOW CLASS HAS NOT ALREADY BEEN REGISTERED
		if (!GetClassInfo(wlGetLibraryModule(), wi->GetWC()->lpszClassName, &wc)) {
			if (!RegisterClass(wi->GetWC())) {
				wlError("User Supplied Class Registration Failure");
				return 0;
			}
		}
	}

	// GET A POINTER TO THE WINDOW CREATION INFORMATION STRUCTURE
	wlWindowInfo * ptr = (wi!=NULL) ? wi : &DefaultInfo;

	// SET THE WINDOW TITLE
	ptr->SetTitle(title);

	HWND ret = CreateWindow(ptr->GetWCName(),	
							ptr->GetTitle(),	
							ptr->GetStyle(),	
							ptr->GetX(),	
							ptr->GetY(),	
							ptr->GetWidth(),
							ptr->GetHeight(),
							ptr->GetParent(),
							ptr->GetMenu(),	
							ptr->GetHinst(),	
							ptr->GetCreationData() 
							);
	if (!ret) {
		wlError("Call to CreateWindow failed!");
		return NULL;
	}

	// SHOW THE WINDOW
	ShowWindow(ret, ptr->GetCmdShow());

	return ret;
}
 

LPVOID WINAPI wlGetCreationData(LPARAM lParam) {
	CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
	return cs->lpCreateParams;
}

// THIS WINDOW PROCEDURE WAS STOLEN FROM MICROSOFT'S GENGL
LONG WINAPI wlWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LONG    lRet = 1;

	switch (uMsg) {
		case WM_CREATE:
			wlSetWindowCount(wlGetWindowCount()+1);
			return 0;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) 
				DestroyWindow(hWnd);
			break;
		case WM_DESTROY:
		case WM_QUIT:
		case WM_QUERYENDSESSION:
			wlSetWindowCount(wlGetWindowCount()-1);
			if (wlGetWindowCount() <= 0)
				PostQuitMessage(0);
			break;
		default:
			lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
	}

    /* return 1 if handled message, 0 if not */
    return lRet;
}

// LIBRARY STATE SETTING / ETC.
HINSTANCE WINAPI wlGetLibraryModule() {
	return wlHinst;
}

void wlSetWindowCount(int newcount) {
	wlWindowCount = newcount;
}

int  wlGetWindowCount() {
	return wlWindowCount;
}

// ERROR REPORTING, ETC.
char * wlErrorString = NULL;
char * wlGetLastError() {
	return wlErrorString;
}

void wlSetLastError(char * error) {
	if (wlErrorString) delete [] wlErrorString;
	wlErrorString = new char [lstrlen(error)+1];
	lstrcpy(wlErrorString, error);
}

void wlPrintMessage(char * message) {
	MessageBox(GetFocus(), message, "Application Message", MB_OK);
}

void wlPrintError(char * message) {
	MessageBox(GetFocus(), message, "Application Error Message", MB_OK | MB_ICONEXCLAMATION);
}


void wlCenterWindow(HWND child, HWND parent) {
	if (!parent)
		parent = GetDesktopWindow();

	RECT ch, pa;
	GetWindowRect(child, &ch);
	GetWindowRect(parent, &pa);

	int x = pa.left + ((pa.right-pa.left)/2 - (ch.right-ch.left)/2);
	int y = pa.top +  ((pa.bottom-pa.top)/2 - (ch.bottom-ch.top)/2);
	
	MoveWindow(child,x,y,ch.right-ch.left,ch.bottom-ch.top,TRUE);
}
