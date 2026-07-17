/* Copyright (c) Mark J. Kilgard, 1994. */

/* This program is freely distributable without licensing fees
   and is provided without guarantee or warrantee expressed or
   implied. This program is -not- in the public domain. */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <GL/glut.h>
#include "glutint.h"

GLUTmenu *__glutCurrentMenu = NULL;
void (*__glutMenuStatusFunc) (int, int, int);
GLUTmenu *__glutMappedMenu;
GLUTwindow *__glutMenuWindow;
GLUTmenuItem *__glutItemSelected;
GLUTmenu *__glutSelectedMenu = NULL;

static GLUTmenu **menuList = NULL;
static int menuListSize = 0;

/* OBSOLETE - use glutMenuStatusFunc instead. */
void
glutMenuStateFunc(GLUTmenuStateCB menuStateFunc)
{
  __glutMenuStatusFunc = (GLUTmenuStatusCB) menuStateFunc;
}

void
glutMenuStatusFunc(GLUTmenuStatusCB menuStatusFunc)
{
  __glutMenuStatusFunc = menuStatusFunc;
}

void
__glutSetMenu(GLUTmenu * menu)
{
  __glutCurrentMenu = menu;
}

static void
unmapMenu(GLUTmenu * menu)
{
  if (menu->cascade) {
    unmapMenu(menu->cascade);
    menu->cascade = NULL;
  }
  menu->anchor = NULL;
  menu->highlighted = NULL;
}

void
__glutFinishMenu(int win, int x, int y)
{
  unmapMenu(__glutMappedMenu);

  if (__glutMenuStatusFunc) {
    __glutSetWindow(__glutMenuWindow);
    __glutSetMenu(__glutMappedMenu);

    /* Setting __glutMappedMenu to NULL permits operations that
       change menus or destroy the menu window again. */
    __glutMappedMenu = NULL;

    __glutMenuStatusFunc(GLUT_MENU_NOT_IN_USE, x, y);
  }
  /* Setting __glutMappedMenu to NULL permits operations that
     change menus or destroy the menu window again. */
  __glutMappedMenu = NULL;

  /* If an item is selected and it is not a submenu trigger,
     generate menu callback. */
  if (__glutItemSelected) {// && !__glutItemSelected->isTrigger) {
    __glutSetWindow(__glutMenuWindow);
    /* When menu callback is triggered, current menu should be
       set to the callback menu. */
    __glutSetMenu(__glutItemSelected->menu);
    __glutItemSelected->menu->select(
      __glutItemSelected->value);
  }
  __glutMenuWindow = NULL;
}

#define MENU_BORDER 1
#define MENU_GAP 2
#define MENU_ARROW_GAP 6
#define MENU_ARROW_WIDTH 8

static void
mapMenu(GLUTmenu* menu, int x, int y, HWND hWnd)
{
	TrackPopupMenu(menu->hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, x, y, 0, hWnd, NULL);
}

void
__glutStartMenu(GLUTmenu * menu, GLUTwindow * window,
  int x, int y, int x_win, int y_win)
{
  assert(__glutMappedMenu == NULL);
  __glutMappedMenu = menu;
  __glutMenuWindow = window;
  __glutItemSelected = NULL;
  if (__glutMenuStatusFunc) {
    __glutSetMenu(menu);
    __glutSetWindow(window);
    __glutMenuStatusFunc(GLUT_MENU_IN_USE, x_win, y_win);
  }
  mapMenu(menu, x, y, window->hwnd);
}

static int
getMenuItemIndex(GLUTmenuItem * item)
{
  int count = 0;

  while (item) {
    count++;
    item = item->next;
  }
  return count;
}

GLUTmenuItem *
__glutGetMenuItem(GLUTmenu* menu, HMENU hMenu, int *which)
{
	GLUTmenuItem* item;
	int i;

	item = menu->list;
	while(item) {
		if(item->value == *which)
			return item;
		item = item->next;
	}

	return NULL;
}

GLUTmenu *
__glutGetMenu(HMENU hMenu)
{
	for(int i = 0; i < menuListSize; i++) {
		if(hMenu == menuList[i]->hMenu)
			return menuList[i];
	}
	return NULL;
}

GLUTmenu *
__glutGetMenuByNum(int menunum)
{
  if (menunum < 1 || menunum > menuListSize) {
    return NULL;
  }
  return menuList[menunum - 1];
}

static int
getUnusedMenuSlot(void)
{
  int i;

  /* Look for allocated, unused slot. */
  for (i = 0; i < menuListSize; i++) {
    if (!menuList[i]) {
      return i;
    }
  }
  /* Allocate a new slot. */
  menuListSize++;
  if (menuList) {
    menuList = (GLUTmenu **)
      realloc(menuList, menuListSize * sizeof(GLUTmenu *));
  } else {
    /* XXX Some realloc's do not correctly perform a malloc
       when asked to perform a realloc on a NULL pointer,
       though the ANSI C library spec requires this. */
    menuList = (GLUTmenu **) malloc(sizeof(GLUTmenu *));
  }
  if (!menuList)
    __glutFatalError("out of memory.");
  menuList[menuListSize - 1] = NULL;
  return menuListSize - 1;
}

static void
menuModificationError(void)
{
  /* XXX Remove the warning after GLUT 3.0. */
  __glutWarning("The following is a new check for GLUT 3.0; update your code.");
  __glutFatalError("menu manipulation not allowed while menus in use");
}

int
glutCreateMenu(GLUTselectCB selectFunc)
{
  GLUTmenu *menu;
  int menuid;

  if (__glutMappedMenu)
    menuModificationError();
  menuid = getUnusedMenuSlot();
  menu = (GLUTmenu *) malloc(sizeof(GLUTmenu));
  if (!menu)
    __glutFatalError("out of memory.");
  menu->id = menuid;
  menu->num = 0;
  menu->submenus = 0;
  menu->managed = FALSE;
  menu->pixwidth = 0;
  menu->select = selectFunc;
  menu->list = NULL;
  menu->cascade = NULL;
  menu->highlighted = NULL;
  menu->anchor = NULL;
  menu->hMenu = CreatePopupMenu();
// __glutMappedMenu = menu;
// __glutItemSelected = NULL;
  menuList[menuid] = menu;
  __glutSetMenu(menu);
  return menuid + 1;
}

/* CENTRY */
void
glutDestroyMenu(int menunum)
{
  GLUTmenu *menu = __glutGetMenuByNum(menunum);
  GLUTmenuItem *item, *next;

  if (__glutMappedMenu)
    menuModificationError();
  assert(menu->id == menunum - 1);
  DestroyMenu(menu->hMenu);
  menuList[menunum - 1] = NULL;
  /* free all menu entries */
  item = menu->list;
  while (item) {
    assert(item->menu == menu);
    next = item->next;
    free(item->label);
    free(item);
    item = next;
  }
  if (__glutCurrentMenu == menu) {
    __glutCurrentMenu = NULL;
  }
  free(menu);
}

int
glutGetMenu(void)
{
  if (__glutCurrentMenu) {
    return __glutCurrentMenu->id + 1;
  } else {
    return 0;
  }
}

void
glutSetMenu(int menuid)
{
  GLUTmenu *menu;

  if (menuid < 1 || menuid > menuListSize) {
    __glutWarning("glutSetMenu attempted on bogus menu.");
    return;
  }
  menu = menuList[menuid - 1];
  if (!menu) {
	__glutWarning("glutSetMenu attempted on bogus menu.");
    return;
  }
  __glutSetMenu(menu);
}
/* ENDCENTRY */

static void
setMenuItem(GLUTmenuItem * item, char *label,
  int value, BOOL isTrigger)
{
  GLUTmenu *menu;

  menu = item->menu;
  item->label = strdup(label);
  if (!item->label)
    __glutFatalError("out of memory.");
  item->isTrigger = isTrigger;
  item->len = (int) strlen(label);
  item->value = value;
  menu->managed = FALSE;
}

/* CENTRY */
void
glutAddMenuEntry(char *label, int value)
{
  GLUTmenuItem *entry;

  if (__glutMappedMenu)
    menuModificationError();
  entry = (GLUTmenuItem *) malloc(sizeof(GLUTmenuItem));
  if (!entry)
    __glutFatalError("out of memory.");
  entry->menu = __glutCurrentMenu;
  entry->hMenu = __glutCurrentMenu->hMenu;
  setMenuItem(entry, label, value, FALSE);
  AppendMenu(__glutCurrentMenu->hMenu, MF_STRING, value, label);
  __glutCurrentMenu->num++;
  entry->next = __glutCurrentMenu->list;
  __glutCurrentMenu->list = entry;
}

void
glutAddSubMenu(char *label, int menu)
{
  GLUTmenu *submenu;
  GLUTmenuItem *submenuitem;

  if (__glutMappedMenu)
    menuModificationError();
  submenuitem = (GLUTmenuItem *) malloc(sizeof(GLUTmenuItem));
  if (!submenuitem)
    __glutFatalError("out of memory.");
  __glutCurrentMenu->submenus++;
  submenuitem->menu = __glutCurrentMenu;
  submenu = __glutGetMenuByNum(menu);
  submenuitem->hMenu = submenu->hMenu;
  setMenuItem(submenuitem, label, /* base 0 */ menu - 1, TRUE);
  AppendMenu(__glutCurrentMenu->hMenu, MF_POPUP | MF_STRING, 
	     (int)submenu->hMenu, label);
  __glutCurrentMenu->num++;
  submenuitem->next = __glutCurrentMenu->list;
  __glutCurrentMenu->list = submenuitem;
}

void
glutChangeToMenuEntry(int num, char *label, int value)
{
  GLUTmenuItem *item;
  int i;

  if (__glutMappedMenu)
    menuModificationError();
  i = __glutCurrentMenu->num;
  item = __glutCurrentMenu->list;
  while (item) {
    if (i == num) {
	ModifyMenu(__glutCurrentMenu->hMenu, i - 1, MF_BYPOSITION | MF_STRING, value, label);
      if (item->isTrigger) {
        /* If changing a submenu trigger to a menu entry, we
           need to account for submenus.  */
        item->menu->submenus--;
      }
      free(item->label);
      setMenuItem(item, label, value, FALSE);
      return;
    }
    i--;
    item = item->next;
  }
  __glutWarning("Current menu has no %d item.", num);
}

void
glutChangeToSubMenu(int num, char *label, int menu)
{
  GLUTmenu *submenu;
  GLUTmenuItem *item;
  int i;

  if (__glutMappedMenu)
    menuModificationError();
  i = __glutCurrentMenu->num;
  item = __glutCurrentMenu->list;
  while (item) {
    if (i == num) {
	submenu = __glutGetMenuByNum(menu);
	ModifyMenu(__glutCurrentMenu->hMenu, i - 1, MF_BYPOSITION | MF_STRING | MF_POPUP, (int)submenu->hMenu, label);
      if (!item->isTrigger) {
        /* If changing a menu entry to as submenu trigger, we
           need to account for submenus.  */
        item->menu->submenus++;
      }
      free(item->label);
      setMenuItem(item, label, /* base 0 */ menu - 1, TRUE);
      return;
    }
    i--;
    item = item->next;
  }
  __glutWarning("Current menu has no %d item.", num);
}

void
glutRemoveMenuItem(int num)
{
  GLUTmenuItem *item, **prev, *remaining;
  int pixwidth, i;

  if (__glutMappedMenu)
    menuModificationError();
  i = __glutCurrentMenu->num;
  prev = &__glutCurrentMenu->list;
  item = __glutCurrentMenu->list;
  /* If menu item is removed, the menu's pixwidth may need to
     be recomputed. */
  pixwidth = 0;
  while (item) {
    if (i == num) {
	RemoveMenu(__glutCurrentMenu->hMenu, i - 1, MF_BYPOSITION);
      __glutCurrentMenu->num--;
      __glutCurrentMenu->managed = FALSE;
      __glutCurrentMenu->pixwidth = pixwidth;

      /* Patch up menu's item list. */
      *prev = item->next;

      free(item->label);
      free(item);
      return;
    }
    i--;
    prev = &item->next;
    item = item->next;
  }
  __glutWarning("Current menu has no %d item.", num);
}

void
glutAttachMenu(int button)
{
  if (__glutMappedMenu)
    menuModificationError();
  if (__glutCurrentWindow->menu[button] < 1) {
    __glutCurrentWindow->buttonUses++;
  }
  __glutCurrentWindow->menu[button] = __glutCurrentMenu->id + 1;
}

void
glutDetachMenu(int button)
{
  if (__glutMappedMenu)
    menuModificationError();
  if (__glutCurrentWindow->menu[button] > 0) {
    __glutCurrentWindow->buttonUses--;
    __glutCurrentWindow->menu[button] = 0;
  }
}
/* ENDCENTRY */
