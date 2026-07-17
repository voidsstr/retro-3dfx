/**************************************************************************
 *									  *
 * 		 Copyright (C) 1994, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#ifdef WIN32

void ogEnvCtxswHook(void)
{
}

#else /* WIN32 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


#include "ogtst.h"
#include "env.h"

#define SEMKEY 443443L		/* Semaphore Key */
#define PERMISSIONS 0666

static struct sembuf semop_op[2] = {
  {0, 1, 0},
  {1, 1, 0}		/* Initialize the operations structure */
  };


int CtxSwitchOK;		/* flag to indicate when to actually */
				/* execute the code in the */
				/* hook. This hook */ 
				/* is currently called from */
				/* ogEnvLog(), which is used even */
				/* before each test is actually run.. */


/* Desired attributes for the visual */
static int attributeList[] = { GLX_RGBA, 
				 GLX_DEPTH_SIZE, 16,
				 GLX_STENCIL_SIZE, 4,
				 None };

static Bool WaitForNotify(Display *, XEvent *, char *);
static int ctxsw_process(int);
static int gtst_semid;		/* Semaphore id */
static int gtst_childpid;	/* Child IPC process */
extern opts_t opts;

/****************************************************************************
* WaitForNotify() -
****************************************************************************/
/*ARGSUSED*/
static Bool WaitForNotify(Display *d, XEvent *e, char *arg)
{
  return (e->type == MapNotify) && (e->xmap.window == (Window) arg);
}


/* Semaphore Operations */

static int setup(int semkey, int *m_semid)
{
  int semid;
  union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
  } semctl_arg;

  
  /* Create 2 semaphores */

  if ((semid = semget(semkey, 2, IPC_CREAT | PERMISSIONS)) < 0)
    {
      perror("semget");
      return -1;
    }

  *m_semid = semid;
  
  /* Initialize both semaphore values to 0 */
  
  semctl_arg.val = (int) 0;
  if (semctl(semid, 0, SETVAL, semctl_arg) < 0 || 
      semctl(semid, 1, SETVAL, semctl_arg) < 0)
    {
      perror("semctl");
      return -1;
    }
  
  return 0;
}

static int cleanup(int semid)
{
  if (semid >= 0)
    if (semctl(semid, 0, IPC_RMID, 0) < 0 || 
	semctl(semid, 1, IPC_RMID, 0) < 0)
      {
        perror("semctl");
        return -1;
      }
  
  return 0;
}


/* Cleanup function called from the parent ogtst() process when it */
/* terminates */
static void clean_IPC(void)
{
  kill(gtst_childpid, SIGKILL);

  semctl(gtst_semid, 0, IPC_RMID, 0); /* remove semaphore */
  semctl(gtst_semid, 1, IPC_RMID, 0);
}


static int sem_signal(int semid, int semnum)
{
  semop_op[semnum].sem_op = 1;
  if (semop(semid, &semop_op[semnum], 1) < 0)
    {
      perror("semop");
      fprintf(stderr, "sem-sig: %d: %d\n", semnum,
	      semctl(semid, semnum, GETVAL, 0));
      return (-1);
    }
  return 0;
}


static int sem_wait(int semid, int semnum)
{
  semop_op[semnum].sem_op = -1;
  if (semop(semid, &semop_op[semnum], 1) < 0)
    {
      perror("semop");
      fprintf(stderr, "sem-wait: %d: %d\n", semnum,
	      semctl(semid, semnum, GETVAL, 0));
      return (-1);
    }
  return 0;
}

  
static void sigint_handler(void)
{
  exit(1);
}

/* ogEnvInitDummyContext(): Initialize a dummy window and the graphics */
/* context to switch to later */

int ogEnvInitDummyContext(void)
{
  int childpid;
  int semid, semkey;

  semkey = (unsigned int) getpid() + SEMKEY;

  CtxSwitchOK = GL_FALSE;
  if (setup(semkey, &semid) < 0) 
    {
      ogEnvLog(OG_LINTERNALERROR, 
	       "InitDummyContext: Cannot setup semaphores\n");
      return -1;
    }
  gtst_semid = semid;
  
  if ((childpid = fork()) < 0)
    {
      perror("fork");
      cleanup(semid);
      ogEnvLog(OG_LINTERNALERROR,
	       "InitDummyContext: Cannot fork() child process\n");
      return -1;
    }
  
  if (childpid == 0)            /* Child process */
    {                           /* Initialise window, and execute GL */
				/* calls */
      ctxsw_process(semkey);
      exit(0);
    }
  
  /* Parent Continues. Register cleanup function for removing */
  /* semaphores at exit */

  gtst_childpid = childpid;
  if (signal(SIGINT, sigint_handler) == SIG_ERR) {
    perror("signal");
    cleanup(semid);
    ogEnvLog(OG_LINTERNALERROR, 
	     "InitDunnyContext: Cannot install signal handler\n");
    return -1;
  }
  atexit(clean_IPC);
  return 0;
}

static int ctxsw_process(int semkey)
{
  Display *dpy;
  GLXContext dumcxt;
  Window dumwin;
  XVisualInfo *vi;
  Colormap cmap;
  XSetWindowAttributes swa;
  XEvent event;
  XSizeHints sizeHints;
  const int xsize = 50;
  const int ysize = 50;
  int semid;
  GLfloat mat_spec[] = {1.0, 0.8, 0.7, 1.0};
  GLfloat light_pos[] = {1.0, 1.0, 1.0, 0.0};

  /* Check whether display is valid */
  if ((dpy = (Display *) XOpenDisplay(0)) == NULL) {
    fprintf(stderr, "child: Cannot connect to display\n");
    return -1;
  }

  /* Obtain a visual */
  if ((vi = glXChooseVisual(dpy, DefaultScreen(dpy), attributeList))
      == NULL) {
    fprintf(stderr, "child: Cannot obtain desired visual\n");
    return -1;
  }

  if ((dumcxt = glXCreateContext(dpy, vi, 0, GL_TRUE)) == NULL) {
    fprintf(stderr, "child: Could not create context\n");
    return -1;
  }
  
  cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual,
			 AllocNone);
  swa.colormap = cmap;
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask;
  
  dumwin = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0,
			 xsize, ysize, 0, vi->depth, InputOutput,
			 vi->visual, 
			 CWBorderPixel | CWColormap | CWEventMask, 
			 &swa); 
  
  sizeHints.flags = USPosition;
  sizeHints.x = 0;
  sizeHints.y = 0;
  sizeHints.width = xsize;
  sizeHints.height = ysize;
  
  XSetStandardProperties(dpy, dumwin, "Dummy", "Dummy",
			 None, NULL, 0, &sizeHints);
  XMapWindow(dpy, dumwin);
  XIfEvent(dpy, &event, WaitForNotify, (char *)dumwin);
  glXMakeCurrent(dpy, dumwin, dumcxt);
  fprintf(stderr, "Child (ctxsw_process) running\n");
  
  /* Setup semaphores */
  
  if ((semid = semget(semkey, 2, 0)) < 0)
    {
      perror("child semget");
      return -1;
    }

  /* Set lot of state-variables in the context so the a context-switch */
  /* flips a lot of bits */
  
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_FLAT);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glMatrixMode(GL_TEXTURE);
  glRotatef(10.0, 0.0, 0.0, 1.0); /* set texture matrix */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glPushMatrix();
  glPushMatrix();

  /* */


  /* Execute GL code in a loop after getting a valid semaphore signal */

  while (1) 
    {
      if (sem_wait(semid, 0) < 0) /* Wait for the semaphore to be */
        return -1;                /* greater than  0; Then decrement */
				  /* it to 0 */

      glBegin(GL_POINTS);
      glVertex3f(1.0, 0.5, -1.0);
      glEnd();
      glFlush();

      if (sem_signal(semid, 1) < 0)
	return -1;
    }

  /*NOTREACHED*/
  return 1;
}

/* ogEnvCtxswHook(): Switch context to the dummy context 
 */
void ogEnvCtxswHook(void)
{
  static int count = 0;
  
  if (!(opts.flags & FLAG_CTXSW) || !CtxSwitchOK)
    return;
  
  /* Switch context every n'th call */
  if ((count = (count + 1)%5))
    return;
  
  /* Signal sem0 to force the child process to run in its own */
  /* context. Wait for ack. */

  sem_signal(gtst_semid, 0);
  sem_wait(gtst_semid, 1);
  return;
}
#endif /* WIN32 */
