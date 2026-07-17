/* Copyright (c) Mark J. Kilgard, 1994, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <stdlib.h>
#include <GL/glut.h>
#include "glutint.h"
#ifdef TODO_FOR_WIN32
#include "layerutil.h"

GLUTcolormap *__glutColormapList = NULL;

static GLUTcolormap *
associateNewColormap(XVisualInfo * vis)
{
  GLUTcolormap *cmap;
  int transparentPixel, i;
  unsigned long pixels[255];

  cmap = (GLUTcolormap *) malloc(sizeof(GLUTcolormap));
  if (!cmap)
    __glutFatalError("out of memory.");
  cmap->visual = vis->visual;
  cmap->refcnt = 1;
  cmap->size = vis->visual->map_entries;
  cmap->cells = (GLUTcolorcell *)
    malloc(sizeof(GLUTcolorcell) * cmap->size);
  /* make all color cell entries be invalid */
  for (i = cmap->size - 1; i >= 0; i--) {
    cmap->cells[i].component[GLUT_RED] = -1.0;
    cmap->cells[i].component[GLUT_GREEN] = -1.0;
    cmap->cells[i].component[GLUT_BLUE] = -1.0;
  }
  if (!cmap->cells)
    __glutFatalError("out of memory.");
  transparentPixel = __glutGetTransparentPixel(__glutDisplay, vis);
  if (transparentPixel == -1 || transparentPixel >= vis->visual->map_entries) {

    /* If there is no transparent pixel or if the transparent
       pixel is outside the range of valid colormap cells (HP
       can implement their overlays this smart way since their
       transparent pixel is 255), we can AllocAll the colormap.
       See note below.  */

    cmap->cmap = XCreateColormap(__glutDisplay,
      __glutRoot, vis->visual, AllocAll);
  } else {

    /* On machines where zero (or some other value in the range
       of 0 through map_entries-1), BadAlloc may be generated
       when an AllocAll overlay colormap is allocated since the
       transparent pixel precludes all the cells in the colormap
       being allocated (the transparent pixel is pre-allocated).
       So in this case, use XAllocColorCells to allocate
       map_entries-1 pixels (that is, all but the transparent
       pixel.  */

    cmap->cmap = XCreateColormap(__glutDisplay,
      __glutRoot, vis->visual, AllocNone);
    XAllocColorCells(__glutDisplay, cmap->cmap, False, 0, 0,
      pixels, vis->visual->map_entries - 1);
  }
  cmap->next = __glutColormapList;
  __glutColormapList = cmap;
  return cmap;
}

GLUTcolormap *
__glutAssociateColormap(XVisualInfo * vis)
{
  GLUTcolormap *cmap = __glutColormapList;

  while (cmap != NULL) {
    /* Play safe: compare visual IDs, not Visual*'s */
    if (cmap->visual->visualid == vis->visual->visualid) {
      /* already have created colormap for the visual */
      cmap->refcnt++;
      return cmap;
    }
    cmap = cmap->next;
  }
  return associateNewColormap(vis);
}

#define CLAMP(i) ((i) > 1.0 ? 1.0 : ((i) < 0.0 ? 0.0 : (i)))

#endif

/* CENTRY */
void
glutSetColor(int ndx, GLfloat red, GLfloat green, GLfloat blue)
{
	PALETTEENTRY pe;

	pe.peRed = (BYTE)(red * 256.0);
	pe.peBlue = (BYTE)(blue * 256.0);
	pe.peGreen = (BYTE)(green * 256.0);
	pe.peFlags = NULL;
	
	UINT ret = SetPaletteEntries(__glutCurrentWindow->hPalette, ndx, 1, &pe);
}

GLfloat
glutGetColor(int ndx, int comp)
{
	PALETTEENTRY pe;
	
	if(!GetPaletteEntries(__glutCurrentWindow->hPalette, ndx, 1, &pe))
		return -1.0;
	switch(comp) {
	case GLUT_RED:
		return (float)pe.peRed / 256.0;
	case GLUT_BLUE:
		return (float)pe.peBlue / 256.0;
	case GLUT_GREEN:
		return (float)pe.peGreen / 256.0;
	default:
		__glutWarning("invalid component in glutGetColor()!");
		return -1.0;
	}
}
/* ENDCENTRY */

#ifdef TODO_FOR_WIN32
void
__glutFreeColormap(GLUTcolormap * cmap)
{
  GLUTcolormap *cur, **prev;

  cmap->refcnt--;
  if (cmap->refcnt == 0) {
    /* remove from colormap list */
    cur = __glutColormapList;
    prev = &__glutColormapList;
    while (cur) {
      if (cur == cmap) {
        *prev = cmap->next;
        break;
      }
      prev = &(cur->next);
      cur = cur->next;
    }
    /* actually free colormap */
    XFreeColormap(__glutDisplay, cmap->cmap);
    free(cmap->cells);
    free(cmap);
  }
}
#endif

/* CENTRY */
void
glutCopyColormap(int winnum)
{
#ifdef TODO_FOR_WIN32
  GLUTwindow *window = __glutWindowList[winnum - 1];
  GLUTcolormap *oldcmap, *newcmap, *copycmap;
  XVisualInfo *dstvis;
  XColor color;
  int i, last;

  if (__glutCurrentWindow->renderWin == __glutCurrentWindow->win) {
    oldcmap = __glutCurrentWindow->colormap;
    dstvis = __glutCurrentWindow->vis;
    newcmap = window->colormap;
  } else {
    oldcmap = __glutCurrentWindow->overlay->colormap;
    dstvis = __glutCurrentWindow->overlay->vis;
    if (!window->overlay) {
      __glutWarning("glutCopyColormap: window %d has no overlay", winnum);
      return;
    }
    newcmap = window->overlay->colormap;
  }

  if (!oldcmap) {
    __glutWarning("glutCopyColormap: destination colormap must be color index");
    return;
  }
  if (!newcmap) {
    __glutWarning(
      "glutCopyColormap: source colormap of window %d must be color index",
      winnum);
    return;
  }
  if (newcmap == oldcmap) {
    /* Source and destination are the same; now copy needed. */
    return;
  }
  /* Play safe: compare visual IDs, not Visual*'s */
  if (newcmap->visual->visualid == oldcmap->visual->visualid) {
    /* Visuals match!  "Copy" by reference...  */
    __glutFreeColormap(oldcmap);
    newcmap->refcnt++;
    if (__glutCurrentWindow->renderWin == __glutCurrentWindow->win) {
      __glutCurrentWindow->colormap = newcmap;
      __glutCurrentWindow->cmap = newcmap->cmap;
    } else {
      __glutCurrentWindow->overlay->colormap = newcmap;
      __glutCurrentWindow->overlay->cmap = newcmap->cmap;
    }
    XSetWindowColormap(__glutDisplay, __glutCurrentWindow->renderWin,
      newcmap->cmap);
    __glutPutOnWorkList(__glutToplevelOf(window), GLUT_COLORMAP_WORK);
  } else {
    /* Visuals different - need a distinct X colormap! */
    copycmap = associateNewColormap(dstvis);
    /* Wouldn't it be nice if XCopyColormapAndFree could be
       told not to free the old colormap's entries! */
    last = newcmap->size;
    if (last > copycmap->size) {
      last = copycmap->size;
    }
    for (i = last - 1; i >= 0; i--) {
      if (newcmap->cells[i].component[GLUT_RED] >= 0.0) {
        color.pixel = i;
        copycmap->cells[i].component[GLUT_RED] =
          newcmap->cells[i].component[GLUT_RED];
        color.red = (GLfloat) 0xffff *
          newcmap->cells[i].component[GLUT_RED];
        copycmap->cells[i].component[GLUT_GREEN] =
          newcmap->cells[i].component[GLUT_GREEN];
        color.green = (GLfloat) 0xffff *
          newcmap->cells[i].component[GLUT_GREEN];
        copycmap->cells[i].component[GLUT_BLUE] =
          newcmap->cells[i].component[GLUT_BLUE];
        color.blue = (GLfloat) 0xffff *
          newcmap->cells[i].component[GLUT_BLUE];
        color.flags = DoRed | DoGreen | DoBlue;
        XStoreColor(__glutDisplay, copycmap->cmap, &color);
      }
    }
  }
#endif
}
/* ENDCENTRY */
