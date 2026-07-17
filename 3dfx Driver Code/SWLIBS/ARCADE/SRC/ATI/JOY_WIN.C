#include <windows.h>
#include <stdio.h>
#include <3dfx.h>
#include <math.h>
#include "joy.h"
#include "resource.h"

void     UpdateJoystickPicture(HWND hCtrl,float x,float y);
FxBool   DoNotCenteredDlog(void);
BOOL     CALLBACK NotCenteredProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
FxBool   ConfigureDlog(void);
BOOL     CALLBACK ConfigDlogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
void     PaintJoyWindow(HWND hWnd,float x,float y);
LRESULT  CALLBACK JoyCustomCtrlProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam);
void     CenterJoystick(void);
void     GetRange(int *min_x,int *min_y,int *max_x, int *max_y);
void     GetDeadSpot();

#define TIMER_JOY_CONFIG         101
#define TEXT_NO_MODE             NULL
#define TEXT_CENTER_MODE         "Center joystick and pull trigger"
#define TEXT_CENTER_DONE         "Joystick centered"
#define TEXT_RANGE_MODE          "Move joystick about range and pull trigger"
#define TEXT_RANGE_DONE          "Joystick range adjusted"
#define TEXT_DEAD_SPOT_MODE      "Move joystick in area for no activity and pull trigger"

extern int     minx;
extern int     miny;
extern int     maxx;
extern int     maxy;
extern int     centerx;
extern int     centery;
extern float   dead_spot;


static float   pict_x=0.0f;
static float   pict_y=0.0f;

FxBool DoNotCenteredDlog(void)
{
   HINSTANCE      hInst;

   hInst=GetModuleHandle(NULL);
   return DialogBox(hInst,"JOY_NOT_CENTERED",NULL,NotCenteredProc);      
}

BOOL CALLBACK NotCenteredProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
   switch (uMsg)
   {
      case WM_INITDIALOG:
         return FXTRUE;
      case WM_COMMAND:
         switch(wParam)
         {
            case IDOK:
               EndDialog(hwndDlg,FXTRUE);
               return FXTRUE;
            case IDCANCEL:
               EndDialog(hwndDlg,FXFALSE);
               return FXTRUE;
         }
         break;
   }
   return FXFALSE;
}

LRESULT CALLBACK JoyCustomCtrlProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
   switch(msg)
   {
      case WM_PAINT:
         printf("Should not be here\n");
         PaintJoyWindow(hWnd,pict_x,pict_y);
         break;
      default:
         return(DefWindowProc(hWnd,msg,wParam,lParam));
         break;
   }
}

FxBool ConfigureDlog(void)
{
   HINSTANCE      hInst;
   WNDCLASS       wc;
   static FxBool  notRegistered=FXTRUE;

   hInst=GetModuleHandle(NULL);
   
   wc.style          =CS_BYTEALIGNCLIENT|CS_BYTEALIGNWINDOW|CS_OWNDC;
   wc.lpfnWndProc    =JoyCustomCtrlProc;
   wc.cbClsExtra     =0;
   wc.cbWndExtra     =0;
   wc.hInstance      =hInst;
   wc.hIcon          =LoadIcon(NULL,IDI_APPLICATION);
   wc.hCursor        =LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground  =GetStockObject(GRAY_BRUSH);
   wc.lpszMenuName   =NULL;
   wc.lpszClassName  ="3dfxJoystickDisplayWin";

   if ((!RegisterClass(&wc))&&(notRegistered)) 
   {                                             
      MessageBox(NULL,"Unable to Register 3dfxJoystickDisplayWin Class",
                  "Error",MB_APPLMODAL|MB_ICONSTOP|MB_OK);
      exit(1);                                                      
   }
   
   notRegistered=FXFALSE;                                                                      

   return DialogBox(hInst,"CONFIGURE_JOY",NULL,ConfigDlogProc);
}

BOOL CALLBACK ConfigDlogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
   static UINT       theTimer;
   float             x,y;
   int               a,b,c,d;
   static HWND       hCtrl;
   static HWND       textCtrl;
   
   switch (uMsg)
   {
      case WM_INITDIALOG:
         theTimer=SetTimer(hwndDlg,TIMER_JOY_CONFIG,25,NULL);
         hCtrl=GetDlgItem(hwndDlg,IDC_PICTURE_JOY);
         textCtrl=GetDlgItem(hwndDlg,IDC_CONFIG_TEXT);
         SetWindowText(textCtrl,TEXT_NO_MODE);
         return FXTRUE;
      case WM_TIMER:
         joystick_read(&x,&y,&a,&b,&c,&d);
         UpdateJoystickPicture(hCtrl,x,y);
         break;
      case WM_COMMAND:
         switch(wParam)
         {
            case IDOK:
               KillTimer(hwndDlg,theTimer);
               EndDialog(hwndDlg,FXTRUE);
               return FXTRUE;
            case IDCANCEL:
               KillTimer(hwndDlg,theTimer);
               EndDialog(hwndDlg,FXFALSE);
               return FXTRUE;
            case IDC_CENTER_JOY:
               SetWindowText(textCtrl,TEXT_CENTER_MODE);
               CenterJoystick();
               SetWindowText(textCtrl,TEXT_CENTER_DONE);
               break;
            case IDC_RANGE_JOY:
               SetWindowText(textCtrl,TEXT_RANGE_MODE);
               GetRange(&minx,&miny,&maxx,&maxy);
               SetWindowText(textCtrl,TEXT_RANGE_DONE);
               break;
            case WM_PAINT:
               UpdateJoystickPicture(hCtrl,x,y);
               break;
         }
         break;
   }
   return FXFALSE;
}

void UpdateJoystickPicture(HWND hCtrl,float x,float y)
{
   pict_x=x;
   pict_y=y;
   InvalidateRect(hCtrl,NULL,FALSE);
   UpdateWindow(hCtrl);
}

void PaintJoyWindow(HWND hWnd,float x,float y)
{
   HBRUSH      hBrush;
   HDC         hdc;
   RECT        windowRect,joy_rect;
   PAINTSTRUCT ps;
   FxU16       center_x,center_y;
   HPEN        hPen;

   hdc=BeginPaint(hWnd,&ps);
   hdc=GetDC(hWnd);
   GetClientRect(hWnd,&windowRect);
   
   center_x=(FxU16)(windowRect.right>>1);
   center_y=(FxU16)(windowRect.bottom>>1);

   joy_rect.right=center_x;
   joy_rect.left=(long)(center_x+(x*center_x));
   joy_rect.top=center_y;
   joy_rect.bottom=(long)(center_y-(y*center_y));
   
   hPen=(HPEN) SelectObject(hdc,GetStockObject(NULL_PEN));
   hBrush=CreateSolidBrush(RGB(0,255,0));
   
   FillRect(hdc,&windowRect,hBrush);
   DeleteObject(SelectObject(hdc,hBrush));
   
   hBrush=CreateSolidBrush(RGB(255,0,0));
   
   FillRect(hdc,&joy_rect,hBrush);
   DeleteObject(SelectObject(hdc,hBrush));
   
   EndPaint(hWnd,&ps);
}

void joystick_calibrate_win( FxBool force )
{
  FILE *fp;
  float xf,yf;
  int   a,b,c,d;
  float  dead_spot_old;

   if (!force)
   {
      fp = fopen( "c:\\joy.dat", "r" );
      if( fp )
      {
         fscanf( fp, "%u %u %u %u %u %u", &minx, &miny, &maxx, &maxy, &centerx, &centery );
         fclose( fp );
         
         dead_spot_old=dead_spot;
         dead_spot=0.25f;
         joystick_read(&xf,&yf,&a,&b,&c,&d);
         dead_spot=dead_spot_old;
         if ((xf==0.0f)&&(yf==0.0f))
            return;
         if (!DoNotCenteredDlog())
            return;
      }
   }
   if (!ConfigureDlog())
      return;
   fp = fopen( "c:\\joy.dat", "w" );
   if( !fp )
      return;
   fprintf( fp, "%u %u %u %u %u %u\n", minx, miny, maxx, maxy, centerx, centery );
   fclose( fp );
}

void CenterJoystick(void)
{
   int      x,y;
   FxU8     buttons;
   
   while( 1 )
   {
      joystick_read_raw( &x, &y, &buttons );
      if( !( buttons & 0x10 ) )
         break;
   }

   joystick_read_raw( &centerx, &centery, &buttons );

   /*
   * Wait for the button to come back up.
   */
   while( 1 )
   {
      joystick_read_raw( &x, &y, &buttons );
      if( buttons & 0x10 )
         break;
   }
}

void GetRange(int *min_x,int *min_y,int *max_x, int *max_y)
{
   int      x,y;
   FxU8     buttons;

   joystick_read_raw( &x, &y, &buttons );
   *min_x = *max_x = x;
   *min_y = *max_y = y;

   while( 1 )
   {
      joystick_read_raw( &x, &y, &buttons );
      if( !( buttons & 0x10 ) )
         break;
      if( x < *min_x )
         *min_x = x;
      if( x > *max_x )
         *max_x = x;
      if( y < *min_y )
         *min_y = y;
      if( y > *max_y )
         *max_y = y;
   }

   /*
   * Wait for the button to come back up.
   */
   while( 1 )
   {
      joystick_read_raw( &x, &y, &buttons );
      if( buttons & 0x10 )
         break;
   }
}

void GetDeadSpot()
{
   int      x_min,y_min,x_max,y_max;
   float    temp,maxPercentage=0.0f;

   GetRange(&x_min,&y_min,&x_max,&y_max);
   temp=(x_min-centerx)/(float)centerx;
   temp=(float)fabs(temp);
   if (temp>maxPercentage)
      maxPercentage=temp;
   temp=(x_max-centerx)/(float)centerx;
   temp=(float)fabs(temp);
   if (temp>maxPercentage)
      maxPercentage=temp;
   temp=(y_min-centery)/(float)centery;
   temp=(float)fabs(temp);
   if (temp>maxPercentage)
      maxPercentage=temp;
   temp=(y_max-centery)/(float)centery;
   temp=(float)fabs(temp);
   if (temp>maxPercentage)
      maxPercentage=temp;
   joystick_set_deadspot(maxPercentage);
}

FxBool BadVersion(void)
{
   FxU32    version;

   version=GetVersion();

   if (version < 0x80000000)                
      return FXTRUE;
   return FXFALSE;
}