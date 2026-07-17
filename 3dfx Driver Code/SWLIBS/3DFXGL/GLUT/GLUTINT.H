#ifndef __glutint_h__
#define __glutint_h__

/* Copyright (c) Mark J. Kilgard, 1994. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <GL/glut.h>

#include <sys/timeb.h>

#ifdef WIN32
#define GETTIMEOFDAY(_x)             \
{                                    \
  struct timeb _t;                  \
  ftime(&_t);                       \
  (_x)->tv_sec = _t.time;            \
  (_x)->tv_usec = _t.millitm * 1000; \
}
#else // BORLAND headers
#define GETTIMEOFDAY(_x)             \
{                                    \
  struct _timeb _t;                  \
  _ftime(&_t);                       \
  (_x)->tv_sec = _t.time;            \
  (_x)->tv_usec = _t.millitm * 1000; \
}
#endif

#define ADD_TIME(dest, src1, src2) { \
  if(((dest).tv_usec = \
    (src1).tv_usec + (src2).tv_usec) >= 1000000) { \
    (dest).tv_usec -= 1000000; \
    (dest).tv_sec = (src1).tv_sec + (src2).tv_sec + 1; \
  } else { \
    (dest).tv_sec = (src1).tv_sec + (src2).tv_sec; \
    if(((dest).tv_sec >= 1) && (((dest).tv_usec <0))) { \
      (dest).tv_sec --;(dest).tv_usec += 1000000; \
    } \
  } \
}
#define TIMEDELTA(dest, src1, src2) { \
  if(((dest).tv_usec = (src1).tv_usec - (src2).tv_usec) < 0) { \
    (dest).tv_usec += 1000000; \
    (dest).tv_sec = (src1).tv_sec - (src2).tv_sec - 1; \
  } else { \
     (dest).tv_sec = (src1).tv_sec - (src2).tv_sec; \
  } \
}
#define IS_AFTER(t1, t2) \
  (((t2).tv_sec > (t1).tv_sec) || \
  (((t2).tv_sec == (t1).tv_sec) && \
  ((t2).tv_usec > (t1).tv_usec)))
#define IS_AT_OR_AFTER(t1, t2) \
  (((t2).tv_sec > (t1).tv_sec) || \
  (((t2).tv_sec == (t1).tv_sec) && \
  ((t2).tv_usec >= (t1).tv_usec)))

#define GLUT_WIND_IS_RGB(x)         (((x) & GLUT_INDEX) == 0)
#define GLUT_WIND_IS_INDEX(x)       (((x) & GLUT_INDEX) != 0)
#define GLUT_WIND_IS_SINGLE(x)      (((x) & GLUT_DOUBLE) == 0)
#define GLUT_WIND_IS_DOUBLE(x)      (((x) & GLUT_DOUBLE) != 0)
#define GLUT_WIND_HAS_ACCUM(x)      (((x) & GLUT_ACCUM) != 0)
#define GLUT_WIND_HAS_ALPHA(x)      (((x) & GLUT_ALPHA) != 0)
#define GLUT_WIND_HAS_DEPTH(x)      (((x) & GLUT_DEPTH) != 0)
#define GLUT_WIND_HAS_STENCIL(x)    (((x) & GLUT_STENCIL) != 0)
#define GLUT_WIND_IS_MULTISAMPLE(x) (((x) & GLUT_MULTISAMPLE) != 0)
#define GLUT_WIND_IS_STEREO(x)      (((x) & GLUT_STEREO) != 0)
#define GLUT_WIND_IS_LUMINANCE(x)   (((x) & GLUT_LUMINANCE) != 0)
#define GLUT_MAP_WORK               (1 << 0)
#define GLUT_EVENT_MASK_WORK        (1 << 1)
#define GLUT_REDISPLAY_WORK         (1 << 2)
#define GLUT_CONFIGURE_WORK         (1 << 3)
#define GLUT_COLORMAP_WORK          (1 << 4)
#define GLUT_DEVICE_MASK_WORK	    (1 << 5)
#define GLUT_FINISH_WORK	    (1 << 6)
#define GLUT_DEBUG_WORK		    (1 << 7)
#define GLUT_DUMMY_WORK		    (1 << 8)
#define GLUT_FULL_SCREEN_WORK       (1 << 9)
#define GLUT_OVERLAY_REDISPLAY_WORK (1 << 10)

// for Win32 (X defines)
#define ShiftMask   1
#define LockMask    2
#define ControlMask 4
#define AltMask     8

/* GLUT CALLBACK FUNCTION TYPES */
typedef void (*GLUTdisplayCB) (void);
typedef void (*GLUTreshapeCB) (int, int);
typedef void (*GLUTkeyboardCB) (unsigned char, int, int);
typedef void (*GLUTmouseCB) (int, int, int, int);
typedef void (*GLUTmotionCB) (int, int);
typedef void (*GLUTpassiveCB) (int, int);
typedef void (*GLUTentryCB) (int);
typedef void (*GLUTvisibilityCB) (int);
typedef void (*GLUTidleCB) (void);
typedef void (*GLUTtimerCB) (int);
typedef void (*GLUTmenuStateCB) (int);  /* DEPRICATED. */
typedef void (*GLUTmenuStatusCB) (int, int, int);
typedef void (*GLUTselectCB) (int);
typedef void (*GLUTspecialCB) (int, int, int);
typedef void (*GLUTspaceMotionCB) (int, int, int);
typedef void (*GLUTspaceRotateCB) (int, int, int);
typedef void (*GLUTspaceButtonCB) (int, int);
typedef void (*GLUTdialsCB) (int, int);
typedef void (*GLUTbuttonBoxCB) (int, int);
typedef void (*GLUTtabletMotionCB) (int, int);
typedef void (*GLUTtabletButtonCB) (int, int, int, int);
typedef int (*GLUTpfdSetupCB) (HDC);

typedef struct _GLUTcolorcell GLUTcolorcell;
struct _GLUTcolorcell {
  /* GLUT_RED, GLUT_GREEN, GLUT_BLUE */
  GLfloat component[3];
};

typedef struct _GLUTcolormap GLUTcolormap;
struct _GLUTcolormap {
  int pfdIndex;			/* index into the pfd table */
  HPALETTE cmap;       /* windows pallette */
  int refcnt;           /* number of windows using colormap */
  int size;             /* number of cells in colormap */
  int transparent;      /* transparent pixel, or -1 if opaque */
  GLUTcolorcell *cells; /* array of cells */
  GLUTcolormap *next;   /* next colormap in list */
};

typedef struct _GLUTwindow GLUTwindow;
typedef struct _GLUToverlay GLUToverlay;
struct _GLUTwindow {
  int num;              /* small integer window id (0-based) */

  /* Window system related state. */
  HWND hwnd;				/* Handle to the GDI window */
  HGLRC hglrc;				/* OpenGL context GLUT glut window */
  int pfdIndex;				/* index to PFD table*/
  HPALETTE hPalette;			/* RGB colormap for window; None if CI */
  HDC hdc;                       /* Window device context */
  GLUTcolormap *colormap;	/* colormap; NULL if RGBA */
  GLUToverlay *overlay;		/* overlay; NULL if no overlay */
  HWND renderHwnd;			/* window for rendering (might be overlay) */
  HGLRC renderHglrc;		/* OpenGL context for rendering (might be overlay) */

  /* GLUT settable or visible window state. */
  int width;            /* window width in pixels */
  int height;           /* window height in pixels */
  HCURSOR cursor;       /* cursor name */
  UINT visState;        /* TRUE = visible, FALSE = not visible (visibility of the window) */
  int shownState;       /* if ShowWindow has been called for hwnd */
  int entryState;       /* entry state (-1 is unknown) */
  int damaged;          /* is layer damaged by expose? */
  int fullscreen;		/* is the window full screened? */

#define GLUT_MAX_MENUS              3

  int menu[GLUT_MAX_MENUS];  /* attatched menu nums */

  /* Window relationship state. */
  GLUTwindow *parent;   /* parent window */
  GLUTwindow *children; /* list of children */
  GLUTwindow *siblings; /* list of siblings */

  /* Misc. non-API visible (hidden) state. */
  BOOL fakeSingle;      /* faking single buffer with double */
  BOOL forceReshape;    /* force reshape before display */
  BOOL isDirect;        /* if direct context */

  int buttonUses;       /* number of button uses, ref cnt */

  /* Work list related state. */
  unsigned int workMask;  /* mask of window work to be done */
  GLUTwindow *prevWorkWin;  /* link list of windows to work on */
  BOOL desiredMapState;		/* how to mapped window if on map work list */
  int desiredConfMask;		/* mask of desired window configuration */

  int desiredX;         /* desired X location */
  int desiredY;         /* desired Y location */
  int desiredWidth;     /* desired window width */
  int desiredHeight;    /* desired window height */
  int desiredStack;     /* desired window stack */

  /* Callbacks */
  GLUTdisplayCB display;	/* redraw callback */
  GLUTreshapeCB reshape;	/* resize callback (width,height) */
  GLUTmouseCB mouse;		/* mouse callback (button,state,x,y) */
  GLUTmotionCB motion;		/* motion callback (x,y) */
  GLUTpassiveCB passive;	/* passive motion callback (x,y) */
  GLUTentryCB entry;		/* window entry/exit callback (state) */
  GLUTkeyboardCB keyboard;		/* keyboard callback (ASCII,x,y) */
  GLUTvisibilityCB visibility;  /* visibility callback */
  GLUTspecialCB special;		/* special key callback */
  GLUTbuttonBoxCB buttonBox;	/* button box callback */
  GLUTdialsCB dials;			/* dials callback */
  GLUTspaceMotionCB spaceMotion;  /* Spaceball motion callback */
  GLUTspaceRotateCB spaceRotate;  /* Spaceball rotate callback */
  GLUTspaceButtonCB spaceButton;  /* Spaceball button callback */
  GLUTtabletMotionCB tabletMotion;  /* tablet motion callback */
  GLUTtabletButtonCB tabletButton;  /* tablet button callback */
  GLUTpfdSetupCB pfdSetup;          // set up the pfd
};

struct _GLUToverlay {
  HWND hwnd;
  HGLRC hglrc;
  int pfdIndex;				/* index into the pfd table*/
  HPALETTE cmap;	        /* RGB colormap for window; None if CI */
  GLUTcolormap *colormap;	/* colormap; NULL if RGBA */
  int shownState;			/* if overlay window mapped */
  BOOL fakeSingle;			/* faking single buffer with double */
  BOOL isDirect;			/* if direct context */
  int damaged;				/* is layer damaged by expose? */
  int transparentPixel;		/* transparent pixel value */
  GLUTdisplayCB display;	/* redraw callback */
};

typedef struct _GLUTstale GLUTstale;
struct _GLUTstale {
  GLUTwindow *window;
  HWND hwnd;
  GLUTstale *next;
};

extern GLUTstale *__glutStaleWindowList;
/* 
#define GLUT_OVERLAY_EVENT_FILTER_MASK \
  (ExposureMask | \
  StructureNotifyMask | \
  EnterWindowMask | \
  LeaveWindowMask)
#define GLUT_DONT_PROPAGATE_FILTER_MASK \
  (ButtonReleaseMask | \
  ButtonPressMask | \
  KeyPressMask | \
  KeyReleaseMask | \
  PointerMotionMask | \
  Button1MotionMask | \
  Button2MotionMask | \
  Button3MotionMask)
#define GLUT_HACK_STOP_PROPAGATE_MASK \
  (KeyPressMask | \
  KeyReleaseMask)
*/ 
typedef struct _GLUTmenu GLUTmenu;
typedef struct _GLUTmenuItem GLUTmenuItem;

struct _GLUTmenu {
  int id;               /* small integer menu id (0-based) */
  HMENU hMenu;           /* X window for the menu */
  GLUTselectCB select;  /* callback function of menu */
  GLUTmenuItem *list;   /* list of menu entries */
  int num;              /* number of entries */
  BOOL managed;         /* are the InputOnly windows size
                           validated? */
  int pixwidth;         /* width of menu in pixels */
  int pixheight;        /* height of menu in pixels */
  int submenus;         /* number of submenu entries */
  GLUTmenuItem *highlighted;  /* pointer to highlighted menu
                                 entry, NULL not highlighted */
  GLUTmenu *cascade;    /* currently cascading this menu  */
  GLUTmenuItem *anchor; /* currently anchored to this entry */
  int x;                /* current x origin relative to the
                           root window */
  int y;                /* current y origin relative to the
                           root window */
};
struct _GLUTmenuItem {
  GLUTmenu *menu;       /* menu entry belongs to */
  HMENU hMenu;         /* menu this item is attached to */
  BOOL isTrigger;       /* is a submenu trigger? */
  int value;            /* value to return for selecting this
                           entry; doubles as submenu id
                           (0-base) if submenu trigger */
  char *label;          /* strdup'ed label string */
  int len;              /* length of label string */
  int pixwidth;         /* width of X window in pixels */
  GLUTmenuItem *next;   /* next menu entry on list for menu */
};

typedef struct _GLUTtimer GLUTtimer;
struct _GLUTtimer {
  GLUTtimer *next;			/* list of timers */
  struct timeval timeout;	/* time to be called */
  GLUTtimerCB func;			/* timer callback (value) */
  int value;				/* callback return value */
};

/* private variables from glut_event.c */
extern GLUTwindow *__glutWindowWorkList;
extern int __glutWindowDamaged;

/* private variables from glut_init.c */
extern unsigned int __glutDisplayMode;
extern GLboolean __glutDebug;
extern GLboolean __glutIconic;
extern HWND __glutRoot;
extern RECT __glutRect;
extern char **__glutArgv;
extern char *__glutProgramName;
extern int __glutArgc;
extern int __glutInitHeight;
extern int __glutInitWidth;
extern int __glutInitX;
extern int __glutInitY;
extern int __glutScreenHeight;
extern int __glutScreenWidth;

/* private variables from glut_menu.c */
extern GLUTmenu *__glutCurrentMenu;
extern GLUTmenu *__glutMappedMenu;
extern GLUTmenu *__glutSelectedMenu;
extern GLUTwindow *__glutMenuWindow;
extern GLUTmenuItem *__glutItemSelected;
extern void (*__glutMenuStatusFunc) (int, int, int);

/* private variables from glut_win.c */
extern GLUTwindow **__glutWindowList;
extern GLUTwindow *__glutCurrentWindow;
extern int __glutWindowListSize;
extern void (*__glutFreeOverlayFunc) (GLUToverlay *);

/* private routines from glut_cindex.c */
//extern GLUTcolormap *__glutAssociateColormap(int pfdIndex);
//extern void __glutFreeColormap(GLUTcolormap *);

/* private routines from glut_event.c */
extern unsigned int __glutModifierMask;
extern void (*__glutUpdateInputDeviceMaskFunc) (GLUTwindow *);
extern void __glutPutOnWorkList(GLUTwindow * window, int work_mask);
extern void __glutPostRedisplay(GLUTwindow * window, int layerMask);
extern LONG WINAPI glutWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* private routines from glut_init.c */
extern void __glutInitTime(struct timeval *beginning);

/* private routines for glut_menu.c */
extern GLUTmenu *__glutGetMenu(HMENU hMenu);
extern GLUTmenu *__glutGetMenuByNum(int menunum);
extern GLUTmenuItem *__glutGetMenuItem(GLUTmenu * menu, HMENU hMenu, int *which);
extern void __glutFinishMenu(int win, int x, int y);
extern void __glutSetMenu(GLUTmenu * menu);
extern void __glutStartMenu(GLUTmenu * menu, GLUTwindow * window, int x, int y, int x_win, int y_win);

/* private routines from glut_util.c */
extern void __glutWarning	(char *format,...);
extern void __glutFatalError(char *format,...);
extern void __glutFatalUsage(char *format,...);

/* private routines from glut_win.c */
extern GLUTwindow *__glutGetWindow(HWND hwnd);
extern GLUTwindow *__glutToplevelOf(GLUTwindow * window);
extern void __glutEstablishColormapsProperty(GLUTwindow * window);
extern int  __glutDetermineVisual(unsigned int mode, BOOL * fakeSingle, int*(getVisualInfo)(unsigned int));
extern int  __glutGetVisualInfo(unsigned int mode);
extern void __glutSetWindow(GLUTwindow * window);
extern void __glutReshapeFunc(GLUTreshapeCB reshapeFunc, int callingConvention);
extern void __glutDefaultReshape(int, int);
extern void __glutSetupColormap(int * pfdIndex, GLUTcolormap ** colormap, HPALETTE * cmap);

#endif /* __glutint_h__ */
