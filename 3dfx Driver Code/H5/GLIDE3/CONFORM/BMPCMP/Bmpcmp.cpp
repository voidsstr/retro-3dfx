/*
 *      Windows bitmap compare utility
 *
 *      Copyright (c) 1998 by Goodin & Associates, Inc.
 *      All Rights Reserved.
 *
 */

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "resource.h"

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam,
			  LPARAM lParam);
BYTE * ReadBmpFile(char * szFileName);
BYTE * ReadFxFile(char * szFileName);
LONG GetDibWidth (BYTE * bits);
LONG GetDibHeight (BYTE * bits);
WORD GetDibNumColors (BYTE * lpDib);
WORD GetDibDepth (BYTE * lpDib);
BYTE * GetDibColorAddr (BYTE * lpDib);
BYTE * GetDibBitsAddr (BYTE * bits);
void GetDibColor(BYTE * dib, WORD depth,
		 signed short mousex, signed short mousey,
		 BYTE *index, BYTE *red, BYTE *green, BYTE *blue);
HPALETTE CreateBmpPalette (BYTE * lpDib);
WORD CompareDibCMAP(BYTE * testbits, BYTE * refbits);
BYTE * CreateDiffBits(WORD * diff, 
			   BYTE * testbits,
			   WORD testdepth,
			   BYTE * refbits,
			   WORD refdepth,
			   WORD dpytype);

WORD magnify = 1;
WORD posx=0;
WORD posy=0;
WORD diff=0;
WORD dpytype = CM_DISPTESTREF;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdParam, int nCmdShow)
{
  static char szAppName[] = "bmpcmp" ;
  HWND        hwnd ;
  MSG         msg ;
  WNDCLASS    wndclass ;
  
  if (!hPrevInstance)
    {
      wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
      wndclass.lpfnWndProc   = WndProc ;
      wndclass.cbClsExtra    = 0 ;
      wndclass.cbWndExtra    = 0 ;
      wndclass.hInstance     = hInstance ;
      wndclass.hIcon         = LoadIcon (hInstance, "MAIN_ICON") ;
      wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
      wndclass.hbrBackground = CreateSolidBrush(
				  GetSysColor(COLOR_BACKGROUND));
      wndclass.lpszMenuName  = MAKEINTRESOURCE(BMPCMP_MENU) ;
      wndclass.lpszClassName = szAppName ;
      
      RegisterClass (&wndclass) ;
    }
  
  hwnd = CreateWindow (szAppName,               // window class name
		       "Bitmap Comparison",     // window caption
		       WS_OVERLAPPEDWINDOW |
		       WS_VSCROLL | WS_HSCROLL, // window style
		       CW_USEDEFAULT,           // initial x position
		       CW_USEDEFAULT,           // initial y position
		       640,                     // initial x size
		       480,                     // initial y size
		       NULL,                    // parent window handle
		       NULL,                    // window menu handle
		       hInstance,               // program instance handle
		       NULL) ;		        // creation parameters

  ShowWindow (hwnd, nCmdShow) ;
  UpdateWindow (hwnd) ;

  while (GetMessage (&msg, NULL, 0, 0))
    {
      TranslateMessage (&msg) ;
      DispatchMessage (&msg) ;
    }
  return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam,
				 LPARAM lParam)
{
  HDC         hdc ;
  PAINTSTRUCT ps;
  
  static OPENFILENAME fname;
  static char szOpenTest[] = "Select Test File";
  static char szOpenRef[] = "Select Reference File";
  static char szFileName[_MAX_PATH];
  static char szTitleName[_MAX_FNAME+_MAX_EXT];
  
  static BYTE * refbits = NULL;
  static HPALETTE refpal;
  static WORD refdepth;
  static BYTE * testbits = NULL;
  static HPALETTE testpal;
  static WORD testdepth;
  static BYTE * diffbits = NULL;
  static BYTE * overlaydiffbits = NULL;

  static BYTE *dpybits1;
  static BYTE *dpybits2;
  static LONG cxBmp,cyBmp;
  
  static HMENU hMenu;
  static WORD cxClient,cyClient;
  static WORD cyMenu;

  static HBRUSH Brush;

  static RECT Msgbar;
  static RECT Sepbar;

  switch (message)
    {
    case WM_SIZE:
      cxClient = LOWORD(lParam);
      cyClient = HIWORD(lParam);
      
      cyMenu = GetSystemMetrics(SM_CYMENU);

      cyClient -= cyMenu;

      SetRect(&Msgbar,0,0,cxClient,cyMenu);
      SetRect(&Sepbar,(cxClient/2)-2,cyMenu,(cxClient/2)+2,cyClient+cyMenu);

      SetScrollRange(hwnd,SB_VERT,0,cyClient,FALSE);
      SetScrollRange(hwnd,SB_HORZ,0,cxClient,FALSE);
      return 0;
    case WM_CREATE:
      fname.lStructSize = sizeof(OPENFILENAME);
      fname.hwndOwner = hwnd;
      fname.hInstance = NULL;
      fname.lpstrFilter = 
	"Portable Bitmap Files (*.PPM)\0*.ppm\0Bitmap Files (*.BMP)\0*.bmp\0All Files\0*.*\0";
      fname.lpstrCustomFilter = NULL;
      fname.nMaxCustFilter = 0;
      fname.nFilterIndex = 1;
      fname.lpstrFile = szFileName;
      fname.nMaxFile = _MAX_PATH;
      fname.lpstrFileTitle = szTitleName;
      fname.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
      fname.lpstrInitialDir = NULL;
      fname.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY ;
      fname.nFileOffset = 0;
      fname.nFileExtension = 0;
      fname.lpstrDefExt = "ppm";
      fname.lCustData = 0l;
      fname.lpfnHook = NULL;
      fname.lpTemplateName = NULL;

      Brush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

      return 0;
    case WM_PAINT:
      hdc = BeginPaint (hwnd, &ps) ;
      
      if((refbits != NULL) && (testbits != NULL)) {
	{
	  if(refdepth == 8) {
	    SelectPalette(hdc,refpal,FALSE);
	    RealizePalette(hdc);
	  } else if(testdepth == 8) {
	    SelectPalette(hdc,testpal,FALSE);
	    RealizePalette(hdc);
	  }

	  cxBmp = GetDibWidth(testbits);
	  cyBmp = GetDibHeight(testbits);
	  
	  switch(dpytype) {
	  case CM_DISPTESTREF:
	    dpybits1 = GetDibBitsAddr(testbits);
	    dpybits2 = GetDibBitsAddr(refbits);
	    StretchDIBits(hdc,0,cyMenu,
			  (cxClient/2)-2,cyClient,
			  posx,posy,
			  cxClient/(magnify*2),cyClient/magnify,
			  (LPSTR)dpybits1,(LPBITMAPINFO)testbits,
			  DIB_RGB_COLORS,SRCCOPY);
	    
	    StretchDIBits(hdc,(cxClient/2)+2,cyMenu,
			  (cxClient/2)-1,cyClient,
			  posx,posy,
			  cxClient/(magnify*2),cyClient/magnify,
			  (LPSTR)dpybits2,(LPBITMAPINFO)refbits,
			  DIB_RGB_COLORS,SRCCOPY);

	    FillRect(hdc,&Sepbar,Brush);
	    break;
	  case CM_DISPTESTDIFF:
	    dpybits1 = GetDibBitsAddr(testbits);
	    dpybits2 = GetDibBitsAddr(diffbits);
	    StretchDIBits(hdc,0,cyMenu,
			  (cxClient/2)-2,cyClient,
			  posx,posy,
			  cxClient/(magnify*2),cyClient/magnify,
			  (LPSTR)dpybits1,(LPBITMAPINFO)testbits,
			  DIB_RGB_COLORS,SRCCOPY);
	    
	    StretchDIBits(hdc,(cxClient/2)+2,cyMenu,
			  (cxClient/2)-1,cyClient,
			  posx,posy,
			  cxClient/(magnify*2),cyClient/magnify,
			  (LPSTR)dpybits2,(LPBITMAPINFO)diffbits,
			  DIB_RGB_COLORS,SRCCOPY);

	    FillRect(hdc,&Sepbar,Brush);
	    break;
	  case CM_DISPREFDIFF:
	    dpybits1 = GetDibBitsAddr(refbits);
	    dpybits2 = GetDibBitsAddr(diffbits);
	    StretchDIBits(hdc,0,cyMenu,
			  (cxClient/2)-2,cyClient,
			  posx,posy,
			  cxClient/(magnify*2),cyClient/magnify,
			  (LPSTR)dpybits1,(LPBITMAPINFO)refbits,
			  DIB_RGB_COLORS,SRCCOPY);
	    
	    StretchDIBits(hdc,(cxClient/2)+2,cyMenu,
			  (cxClient/2)-1,cyClient,
			  posx,posy,
			  cxClient/(magnify*2),cyClient/magnify,
			  (LPSTR)dpybits2,(LPBITMAPINFO)diffbits,
			  DIB_RGB_COLORS,SRCCOPY);

	    FillRect(hdc,&Sepbar,Brush);
	    break;
	  case CM_DISPREFOVERLAY24DIFF:
            if (!overlaydiffbits) {
              MessageBox(hwnd,"No 24 bit overlay diff available!",
			   "Display Overlay Diff",
			   MB_ICONEXCLAMATION | MB_OK);
            } else {
              dpybits1 = GetDibBitsAddr(refbits);
              dpybits2 = GetDibBitsAddr(overlaydiffbits);
              StretchDIBits(hdc,0,cyMenu,
                            (cxClient/2)-2,cyClient,
                            posx,posy,
                            cxClient/(magnify*2),cyClient/magnify,
                            (LPSTR)dpybits1,(LPBITMAPINFO)refbits,
                            DIB_RGB_COLORS,SRCCOPY);
	    
              StretchDIBits(hdc,(cxClient/2)+2,cyMenu,
                            (cxClient/2)-1,cyClient,
                            posx,posy,
                            cxClient/(magnify*2),cyClient/magnify,
                            (LPSTR)dpybits2,(LPBITMAPINFO)overlaydiffbits,
                            DIB_RGB_COLORS,SRCCOPY);
              FillRect(hdc,&Sepbar,Brush);
            }
            break;
	  case CM_DISPTESTOVERLAY24DIFF:
            if (!overlaydiffbits) {
              MessageBox(hwnd,"No 24 bit overlay diff available!",
			   "Display Overlay Diff",
			   MB_ICONEXCLAMATION | MB_OK);
            } else {
              dpybits1 = GetDibBitsAddr(testbits);
              dpybits2 = GetDibBitsAddr(overlaydiffbits);
              StretchDIBits(hdc,0,cyMenu,
                            (cxClient/2)-2,cyClient,
                            posx,posy,
                            cxClient/(magnify*2),cyClient/magnify,
                            (LPSTR)dpybits1,(LPBITMAPINFO)testbits,
                            DIB_RGB_COLORS,SRCCOPY);
	    
              StretchDIBits(hdc,(cxClient/2)+2,cyMenu,
                            (cxClient/2)-1,cyClient,
                            posx,posy,
                            cxClient/(magnify*2),cyClient/magnify,
                            (LPSTR)dpybits2,(LPBITMAPINFO)overlaydiffbits,
                            DIB_RGB_COLORS,SRCCOPY);
              FillRect(hdc,&Sepbar,Brush);
            }
            break;
            }
          }
	
	FillRect(hdc,&Sepbar,Brush);
	FillRect(hdc,&Msgbar,Brush);
      } else {
	FillRect(hdc,&Sepbar,Brush);
	FillRect(hdc,&Msgbar,Brush);
      }
      
      EndPaint (hwnd, &ps) ;
      
      SetScrollPos(hwnd,SB_VERT,cyClient-posy,TRUE);
      SetScrollPos(hwnd,SB_HORZ,posx,TRUE);
      return 0 ;
    case WM_LBUTTONDOWN:
      if(refbits != NULL) {
	if(MK_SHIFT & wParam) {
	  signed short mousey;
	  signed short mousex;
	  
	  mousey = HIWORD(lParam);
	  mousex = LOWORD(lParam);
	  
	  if(mousex > cxClient/2)
	    mousex -= (cxClient/2)+2;
	  
	  if(cyClient>(cyBmp*magnify)) {
	    if(mousey<(cyClient-(cyBmp*magnify)))
	      return 0;
	    mousey -= cyClient-(cyBmp*magnify);
	    mousey = (cyBmp*magnify) - mousey;
	  } else {
	    mousey = cyClient-mousey;
	  }
	  
	  /* mouse is now in screen lower left */
	  mousey /=magnify;
	  mousex /=magnify;
	  
	  /* mouse is now in pixel lower left */
	  mousex += posx;
	  mousey += posy;
	  mousey = GetDibHeight(testbits) - mousey;
	  
	  if(mousey >= ((cyMenu/magnify)+1)) {
	    BYTE index, red, green, blue;
	    char szTextOut[80], szTemp[80];
	    
	    mousey -= (cyMenu/magnify)+1;
	    
	    GetDibColor(testbits,testdepth,
			mousex, mousey,
			&index, &red, &green, &blue);
	    
	    if(testdepth == 8) {
	      sprintf(szTextOut,"[%02x] [%02x,%02x,%02x]        ",
		      index,red,green,blue);
	    } else {
	      sprintf(szTextOut,"     [%02x,%02x,%02x]        ",
		      red,green,blue);
	    }
	    
	    sprintf(szTemp,"(%04d,%04d)",mousex,mousey);
	    strcat(szTextOut,szTemp);
	    
	    GetDibColor(refbits,refdepth,
			mousex, mousey,
			&index, &red, &green, &blue);
	    if(refdepth == 8) {
	      sprintf(szTemp,"        [%02x] [%02x,%02x,%02x]",
		      index,red,green,blue);
	    } else {
	      sprintf(szTemp,"             [%02x,%02x,%02x]",
		      red,green,blue);
	    }
	    
	    strcat(szTextOut,szTemp);
	    
	    hdc = GetDC(hwnd) ;
	    FillRect(hdc,&Msgbar,Brush);
	    
	    SetTextAlign(hdc, TA_TOP | TA_CENTER);
	    SetBkMode(hdc,TRANSPARENT);
	    TextOut(hdc, cxClient/2, 0, szTextOut, lstrlen(szTextOut));
	    
	    ReleaseDC (hwnd, hdc) ;
	  }
	} else if(MK_CONTROL & wParam) {
	  POINT point;
	  int x,y;
	  
	  GetCursorPos(&point);
	  x = point.x;
	  y = point.y;
	  ScreenToClient(hwnd,&point);
	  if(point.x < (cxClient/2)) {
	    x += (cxClient/2)+2;
	  } else {
	    x -= (cxClient/2)+2;
	  }
	  SetCursorPos(x,y);
	} else {
	  if(magnify != 32) {
	    signed short mousey;
	    signed short mousex;
	    
	    mousey = HIWORD(lParam);
	    mousex = LOWORD(lParam);
	    
	    if(mousex > cxClient/2)
	      mousex -= (cxClient/2)+2;
	    
	    if(cyClient>(cyBmp*magnify)) {
	      if(mousey<(cyClient-(cyBmp*magnify)))
		return 0;
	      mousey -= cyClient-(cyBmp*magnify);
	      mousey = (cyBmp*magnify) - mousey;
	    } else {
	      mousey = cyClient-mousey;
	    }
	    
	    /* mouse is now in screen lower left */
	    mousey /=magnify;
	    mousex /=magnify;
	    
	    /* mouse is now in pixel lower left */
	    mousex += posx;
	    mousey += posy;
	    
	    /* mouse is now in image pixel lower left */
	    /* create lower left corner of zoom window */
	    mousex -= cxClient/(8*magnify);
	    mousey -= cyClient/(4*magnify);
	    
	    magnify *= 2;
	    
	    /* fix edge problems */
	    if(((cxClient/(2*magnify))+mousex)>GetDibWidth(testbits)) {
	      mousex = GetDibWidth(testbits)-(cxClient/(2*magnify));
	    }
	    if(((cyClient/magnify)+mousey)>GetDibHeight(testbits)) {
	      mousey = GetDibHeight(testbits)-(cyClient/magnify);
	    }
	    if(mousex < 0)
	      mousex = 0;
	    if(mousey < 0)
	      mousey = 0;
	    
	    posy = mousey;
	    posx = mousex;
	    
	  }
	  InvalidateRect(hwnd,NULL,TRUE);
	}
      }
      return 0;
    case WM_RBUTTONDOWN:
      if(magnify != 1) { 
	signed short mousey;
	signed short mousex;
	
	/* reset to center */
	mousex = posx + cxClient/(4*magnify);
	mousey = posy + cyClient/(2*magnify);
	
	magnify /= 2;

	/* mouse is now in image pixel lower left */
	/* create lower left corner of zoom window */
	mousex -= cxClient/(4*magnify);
	mousey -= cyClient/(2*magnify);

	/* fix edge problems */
	if(((cxClient/(2*magnify))+mousex)>GetDibWidth(testbits)) {
	  mousex = GetDibWidth(testbits)-(cxClient/(2*magnify));
	}
	if(((cyClient/magnify)+mousey)>GetDibHeight(testbits)) {
	  mousey = GetDibHeight(testbits)-(cyClient/magnify);
	}
	if(mousex < 0)
	  mousex = 0;
	if(mousey < 0)
	  mousey = 0;

	posy = mousey;
	posx = mousex;

      }
      InvalidateRect(hwnd,NULL,TRUE);
      return 0;
    case WM_VSCROLL:
      {
	signed short mousey;

	mousey = posy;

	switch(wParam) {
	case SB_LINEUP:
	  mousey++;
	  
	  /* fix edge problems */
	  if(((cyClient/magnify)+mousey)>GetDibHeight(testbits)) {
	    mousey = GetDibHeight(testbits)-(cyClient/magnify);
	  }
	  if(mousey < 0)
	    mousey = 0;
	  
	  posy = mousey;
	  
	  InvalidateRect(hwnd,NULL,TRUE);
	  break;
	case SB_LINEDOWN:
	  mousey--;
	  
	  /* fix edge problems */
	  if(((cyClient/magnify)+mousey)>GetDibHeight(testbits)) {
	    mousey = GetDibHeight(testbits)-(cyClient/magnify);
	  }
	  if(mousey < 0)
	    mousey = 0;
	  
	  posy = mousey;
	  
	  InvalidateRect(hwnd,NULL,TRUE);
	  break;
	case SB_PAGEUP:
	  mousey += cyClient/magnify;
	  
	  /* fix edge problems */
	  if(((cyClient/magnify)+mousey)>GetDibHeight(testbits)) {
	    mousey = GetDibHeight(testbits)-(cyClient/magnify);
	  }
	  if(mousey < 0)
	    mousey = 0;
	  
	  posy = mousey;
	  
	  InvalidateRect(hwnd,NULL,TRUE);
	  break;
	case SB_PAGEDOWN:
	  mousey -= cyClient/magnify;
	  
	  /* fix edge problems */
	  if(((cyClient/magnify)+mousey)>GetDibHeight(testbits)) {
	    mousey = GetDibHeight(testbits)-(cyClient/magnify);
	  }
	  if(mousey < 0)
	    mousey = 0;
	  
	  posy = mousey;
	  
	  InvalidateRect(hwnd,NULL,TRUE);
	  break;
	case SB_THUMBPOSITION:
	  mousey = cyClient - LOWORD(lParam);
	  
	  /* fix edge problems */
	  if(((cyClient/magnify)+mousey)>GetDibHeight(testbits)) {
	    mousey = GetDibHeight(testbits)-(cyClient/magnify);
	  }
	  if(mousey < 0)
	    mousey = 0;
	  
	  posy = mousey;
	  
	  InvalidateRect(hwnd,NULL,TRUE);
	  break;
	default:
	  break;
	}
      }
      return 0;
    case WM_HSCROLL:
      {
	signed short mousex;

	mousex = posx;
	
	switch(wParam) {
	case SB_LINEUP:
	  mousex++;

	  /* fix edge problems */
	  if(((cxClient/(2*magnify))+mousex)>GetDibWidth(testbits)) {
	    mousex = GetDibWidth(testbits)-(cxClient/(2*magnify));
	  }
	  if(mousex < 0)
	    mousex = 0;
	  
	  posx = mousex;
	  InvalidateRect(hwnd,NULL,TRUE);
	  break;
	case SB_LINEDOWN:
	  mousex--;

	  /* fix edge problems */
	  if(((cxClient/(2*magnify))+mousex)>GetDibWidth(testbits)) {
	    mousex = GetDibWidth(testbits)-(cxClient/(2*magnify));
	  }
	  if(mousex < 0)
	    mousex = 0;
	  
	  posx = mousex;
	  InvalidateRect(hwnd,NULL,TRUE);
	  break;
	case SB_PAGEUP:
	  mousex -= cxClient/(2*magnify);

	  /* fix edge problems */
	  if(((cxClient/(2*magnify))+mousex)>GetDibWidth(testbits)) {
	    mousex = GetDibWidth(testbits)-(cxClient/(2*magnify));
	  }
	  if(mousex < 0)
	    mousex = 0;
	  
	  posx = mousex;
	  InvalidateRect(hwnd,NULL,TRUE);
	  break;
	case SB_PAGEDOWN:
	  mousex += cxClient/(2*magnify);

	  /* fix edge problems */
	  if(((cxClient/(2*magnify))+mousex)>GetDibWidth(testbits)) {
	    mousex = GetDibWidth(testbits)-(cxClient/(2*magnify));
	  }
	  if(mousex < 0)
	    mousex = 0;
	  
	  posx = mousex;
	  InvalidateRect(hwnd,NULL,TRUE);
	  break;
	case SB_THUMBPOSITION:
	  mousex = LOWORD(lParam);

	  /* fix edge problems */
	  if(((cxClient/(2*magnify))+mousex)>GetDibWidth(testbits)) {
	    mousex = GetDibWidth(testbits)-(cxClient/(2*magnify));
	  }
	  if(mousex < 0)
	    mousex = 0;
	  
	  posx = mousex;
	  InvalidateRect(hwnd,NULL,TRUE);
	  break;
	default:
	  break;
	}
      }
      return 0;
    case WM_COMMAND:
      hMenu = GetMenu(hwnd);

      switch(wParam) {
      case CM_EXIT:
	SendMessage(hwnd,WM_CLOSE,0,0L);
	return 0;
      case CM_DISPTESTREF:
	dpytype = CM_DISPTESTREF;
	hMenu = GetMenu(hwnd);
	CheckMenuItem(hMenu,CM_DISPTESTREF,MF_CHECKED);
	CheckMenuItem(hMenu,CM_DISPTESTDIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPREFDIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPTESTOVERLAY24DIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPREFOVERLAY24DIFF,MF_UNCHECKED);
	InvalidateRect(hwnd,NULL,TRUE);
	return 0;
      case CM_DISPTESTDIFF:
	dpytype = CM_DISPTESTDIFF;
	hMenu = GetMenu(hwnd);
	CheckMenuItem(hMenu,CM_DISPTESTREF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPTESTDIFF,MF_CHECKED);
	CheckMenuItem(hMenu,CM_DISPREFDIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPTESTOVERLAY24DIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPREFOVERLAY24DIFF,MF_UNCHECKED);
	InvalidateRect(hwnd,NULL,TRUE);
	return 0;
      case CM_DISPREFDIFF:
	dpytype = CM_DISPREFDIFF;
	hMenu = GetMenu(hwnd);
	CheckMenuItem(hMenu,CM_DISPTESTREF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPTESTDIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPREFDIFF,MF_CHECKED);
	CheckMenuItem(hMenu,CM_DISPTESTOVERLAY24DIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPREFOVERLAY24DIFF,MF_UNCHECKED);
	InvalidateRect(hwnd,NULL,TRUE);
	return 0;
      case CM_DISPTESTOVERLAY24DIFF:
	dpytype = CM_DISPTESTOVERLAY24DIFF;
	hMenu = GetMenu(hwnd);
	CheckMenuItem(hMenu,CM_DISPTESTREF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPTESTDIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPREFDIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPTESTOVERLAY24DIFF,MF_CHECKED);
	CheckMenuItem(hMenu,CM_DISPREFOVERLAY24DIFF,MF_UNCHECKED);
	InvalidateRect(hwnd,NULL,TRUE);
	return 0;
      case CM_DISPREFOVERLAY24DIFF:
	dpytype = CM_DISPREFOVERLAY24DIFF;
	hMenu = GetMenu(hwnd);
	CheckMenuItem(hMenu,CM_DISPTESTREF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPTESTDIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPREFDIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPTESTOVERLAY24DIFF,MF_UNCHECKED);
	CheckMenuItem(hMenu,CM_DISPREFOVERLAY24DIFF,MF_CHECKED);
	InvalidateRect(hwnd,NULL,TRUE);
	return 0;
      case CM_LOAD:
	fname.lpstrTitle = szOpenRef;
	if(GetOpenFileName(&fname)) {
	  if(refbits != NULL) {
	    GlobalFreePtr(refbits);
	    refbits = NULL;
	    if(refpal != NULL)
	      DeleteObject(refpal);
	    refpal = NULL;
	  }
	  if(testbits != NULL) {
	    GlobalFreePtr(testbits);
	    testbits = NULL;
	    if(testpal != NULL)
	      DeleteObject(testpal);
	    testpal = NULL;
	  }
	  if(diffbits != NULL) {
	    GlobalFreePtr(diffbits);
	    diffbits = NULL;
	  }
	  if(overlaydiffbits != NULL) {
	    GlobalFreePtr(overlaydiffbits);
	    overlaydiffbits = NULL;
	  }
#ifdef RMG
	  if((refbits = ReadBmpFile(szFileName)) == NULL) {
#else
	  if((refbits = ReadFxFile(szFileName)) == NULL) {
#endif
	    MessageBox(hwnd,"Could not open file",
		       "Test Reference File",
		       MB_ICONEXCLAMATION | MB_OK);
	  } else {
	    refdepth = GetDibDepth(refbits);
	    if(refdepth == 8) {
	      refpal = CreateBmpPalette (refbits);
	    } else {
	      refpal = NULL;
	    }
	    fname.lpstrTitle = szOpenTest;
	    if(GetOpenFileName(&fname)) {
#ifdef RMG
	      if((testbits = ReadBmpFile(szFileName)) == NULL) {
#else
	      if((testbits = ReadFxFile(szFileName)) == NULL) {
#endif
		GlobalFreePtr(refbits);
		refbits = NULL;
		if(refpal != NULL)
		  DeleteObject(refpal);
		refpal = NULL;
		MessageBox(hwnd,"Could not open file",
			   "Test Result File",
			   MB_ICONEXCLAMATION | MB_OK);
	      } else {
		testdepth = GetDibDepth(testbits);
		if(testdepth == 8) {
		  testpal = CreateBmpPalette (testbits);
		} else {
		  testpal = NULL;
		}

		/* Test size and colormap */
		if((GetDibWidth(testbits) != GetDibWidth(refbits)) ||
		   (GetDibHeight(testbits) != GetDibHeight(refbits))) {
		  MessageBox(hwnd,"File size different",
			     "Test and Reference File",
			     MB_ICONEXCLAMATION | MB_OK);
		}
		if((testdepth == 8) && (refdepth == 8)) {
		  if(CompareDibCMAP(testbits,refbits)) {
		    MessageBox(hwnd,"Map colors different",
			       "Colormap",
			       MB_ICONEXCLAMATION | MB_OK);
		  }
		}

		if((diffbits = CreateDiffBits(&diff,testbits,testdepth,
					      refbits,refdepth, NULL))==NULL) {
		  GlobalFreePtr(refbits);
		  refbits = NULL;
		  if(refpal != NULL)
		    DeleteObject(refpal);
		  refpal = NULL;
		  GlobalFreePtr(testbits);
		  testbits = NULL;
		  if(testpal != NULL)
		    DeleteObject(testpal);
		  testpal = NULL;
		  MessageBox(hwnd,"Could not create",
			     "Difference Image",
			     MB_ICONEXCLAMATION | MB_OK);
		} else if ((testdepth == 24) && (refdepth == 24)){ 
                  // if applicable, also create the overlay diff
		  overlaydiffbits = CreateDiffBits(&diff, testbits, testdepth,
                                                   refbits, refdepth, 
                                                   CM_DISPTESTOVERLAY24DIFF);
                }
 
		if(diff) {
		  MessageBox(hwnd,"Images DO NOT match",
			     "Difference Image",
			     MB_ICONEXCLAMATION | MB_OK);
		} else {
		  MessageBox(hwnd,"Images match",
			     "Difference Image",
			     MB_ICONINFORMATION | MB_OK);
		}
	      }
	    }
	  }
	}
      }
      
      posx = 0;
      posy = 0;
      magnify = 1;
      InvalidateRect(hwnd,NULL,TRUE);
      return 0;
    case WM_DESTROY:
      if(refbits != NULL) {
	GlobalFreePtr(refbits);
	DeleteObject(refpal);
      }
      if(testbits != NULL) {
	GlobalFreePtr(testbits);
      }
      if(diffbits != NULL) {
	GlobalFreePtr(diffbits);
      }
      if(overlaydiffbits != NULL) {
	GlobalFreePtr(overlaydiffbits);
      }
      DeleteObject(Brush);
      PostQuitMessage (0) ;
      return 0 ;
    }
  
  return DefWindowProc (hwnd, message, wParam, lParam) ;
}
