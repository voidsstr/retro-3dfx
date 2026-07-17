/*
	File:	wgllib.h		

	Defines common functions that facilitate the use of 
	wgl under Windows NT.

	Paul Mayfield, 7/2/96
*/

#ifndef __winlib_h
#define __winlib_h

#include <windows.h>

// HERE'S A CLASS THAT WILL CONTAIN INFORMATION FOR CREATING WINDOWS
class wlWindowInfo {
  public:
	// CONSTRUCTION / DESTRUCTION
	wlWindowInfo(HINSTANCE hinst=NULL);
	~wlWindowInfo();

	// QUERY INFORMATION
	WNDCLASS *	GetWC()		{return & wc;};
	char *		GetWCName() {return (char*)wc.lpszClassName;};
	char *		GetTitle()  {return Title;};
	DWORD		GetStyle()	{return dwStyle;};
	int			GetX()		{return x;};
	int			GetY()		{return y;};
	int			GetWidth()	{return nWidth;};
	int			GetHeight() {return nHeight;};
	HWND		GetParent() {return hWndParent;};
	HMENU		GetMenu()	{return hMenu;};
	HANDLE		GetHinst()	{return hinstance;};
	LPVOID		GetCreationData(){return lpParam;};
	int			GetCmdShow(){return nCmdShow;};

	// SET INFORMATION
	void SetWC(WNDCLASS & newwc)	{wc=newwc;};
	void SetWCName(char * newname);
	void SetTitle(char * newtitle);
	void SetStyle(DWORD newstyle)	{dwStyle=newstyle;};
	void SetX(int newx)				{x=newx;};
	void SetY(int newy)				{y=newy;};
	void SetWidth(int newwidth)		{nWidth=newwidth;};
	void SetHeight(int newheight)	{nHeight=newheight;};
	void SetParent(HWND newparent)	{hWndParent=newparent;};
	void SetMenu(HMENU newmenu)		{hMenu=newmenu;};
	void SetHinst(HANDLE newhinst)	{hinstance=newhinst; wc.hInstance=newhinst;};
	void SetCreationData(LPVOID newparam){lpParam=newparam;};
	void SetCmdShow(int newshow)	{nCmdShow=newshow;};

  protected:
	WNDCLASS wc;
	char * Title;
	DWORD dwStyle;
	int x; 
	int y; 
	int nWidth;
	int nHeight;
	HWND hWndParent;
	HMENU hMenu;
	HANDLE hinstance;
	LPVOID lpParam;
	WNDPROC WindowProc;
	int nCmdShow;
};

// GLOBALS
extern char * wlPropID;

// INITIALIZATION 
int  WINAPI wlInitialize	(HINSTANCE hinst);

// WINDOW CREATION FUNCTIONS
HWND   WINAPI	wlCreateWindow	(char * title, wlWindowInfo * wi=NULL);
LPVOID WINAPI	wlGetCreationData(LPARAM lParam);
LONG   WINAPI	wlWindowProc	(HWND, UINT, WPARAM, LPARAM);
LONG   WINAPI	wlCommandHandler(HWND, WPARAM, LPARAM);

// LIBRARY STATE SETTING / ETC.
HINSTANCE WINAPI wlGetLibraryModule();
void wlSetWindowCount(int newcount);
int  wlGetWindowCount();

// ERROR REPORTING, ETC.
char * wlGetLastError();
void   wlSetLastError(char * error);
void wlPrintMessage(char * message);
void wlPrintError(char * message);

// CONVEINENT WINDOW FUNCTIONS
void	wlAddWindowParam(HWND hwnd, DWORD data, char * id=wlPropID);
void	wlDelWindowParam(HWND hwnd, char * id=wlPropID);
DWORD	wlGetWindowParam(HWND hwnd, char * id=wlPropID);
void	wlCenterWindow(HWND child, HWND parent=NULL);

// THIS IS A HELPER STRUCTURE USED TO EXTRACT CREATION DATA
typedef struct _tagMyCreationStruct {
    SHORT    cbExtra; 
    LPVOID   myData; 
} MyCreationData, UNALIGNED *LPMyCreationData; 
 



#endif