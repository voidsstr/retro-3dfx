/******************************************************************************
  
  DIALOG_H - Dialog library prototypes, constants and typedefs
  Written for Autodesk 3D Studio 3.0
   
  (C) Copyright 1994 by Autodesk, Inc.

  This program is copyrighted by Autodesk, Inc. and is licensed to you under
  the following conditions.  You may not distribute or publish the source
  code of this program in any form.  You may incorporate this code in object
  form in derivative works provided such derivative works are (i.) are de-
  signed and intended to work solely with Autodesk, Inc. products, and (ii.)
  contain Autodesk's copyright notice "(C) Copyright 1994 by Autodesk, Inc."

  AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.  AUTODESK SPE-
  CIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
  A PARTICULAR USE.  AUTODESK, INC.  DOES NOT WARRANT THAT THE OPERATION OF
  THE PROGRAM WILL BE UNINTERRUPTED OR ERROR FREE.

*******************************************************************************/

#ifndef DIALOG_H
#define DIALOG_H

#ifdef __cplusplus
extern "C" {
#endif   

typedef struct DL
{
   short parent, child, prev, next;
   short type;
   short x, y, width, height;
   void *text;
   int *radio_var;
   short sx, sy;
   short key, radio;
   void (*see)(struct DL *), (*feel)(struct DL *, int);
   unsigned short flags;
} Dialog;

/* Flag bits */


/* Standard dialog part types */

#define DLG_BOX 0
#define DLG_BUTTON 1
#define DLG_FRAME 2
#define DLG_BOXCHAR 3
#define DLG_STRING 4
#define DLG_EDIT 5
#define DLG_ICON 6

/* Feel substitution list */

typedef struct
{
   int index;
   void (*feel)(Dialog *, int);
} FeelSub;

/* Pre-defined Feel functions */

/* Default feel funtions for radio buttons */
extern void feel_radio(Dialog *d, int mouse);

/* Default feel function for a button */
extern void feel_button(Dialog *d, int mouse);

/* Default feel function for editable text fields */
extern void feel_edit(Dialog *d, int mouse);

/* See substitution list */

typedef struct
{
   int index;
   void (*see)(Dialog *);
} SeeSub;

/* Pre-defined See functions */

extern void see_radio(Dialog *d);
extern void see_button(Dialog *d);
extern void see_string(Dialog *d);
extern void see_boxchar(Dialog *d);
extern void see_edit(Dialog *d);

/* Radio button substitution list */

typedef struct
{
   short index;
   void (*feel)(Dialog *, int);
   int *variable;
   short value;
} RadSub;

/* Scroller substitution list */

typedef struct
{
   int up;
   int down;
   int slider;
   int box;
   void (*boxfeel)(Dialog *, int);
   void *scrollstruct;
} ScrSub;

#define FNULL ((void (*)()) 0 )
#define SEENULL ((void (*)(Dialog *)) 0)
#define FEELNULL ((void (*)(Dialog *, int)) 0)


/* Editables */

typedef struct
{
   unsigned short dcount;		/* Displayed characters */
   unsigned short bcount;		/* Total characters in string */
   unsigned short ccount;		/* Current string length */
   char *string;		/* Edit string */
   unsigned short type;		/* See below */
   unsigned short cpos;		/* Cursor position */
   unsigned short dpos;		/* First character displayed */
   Dialog *up;			/* Pointer to previous editable */
   Dialog *down;		/* Pointer to next editable */
} Editable;

/* Defines for editable item type */

#define ANY_CHAR 0		/* All chars */
#define INT_CHAR 1		/* Integer -- 0-9, - */
#define FLT_CHAR 2		/* Float -- 0-9, -, . */
#define POSINT_CHAR 3	/* Integer -- 0-9 */
#define POSFLT_CHAR 4	/* Float -- 0-9, . */
#define FORCE_UPCASE 5	/* Any char but force lowercase to upper */
#define PRI_FNAME 6		/* Force lowercase to upper, restrict to pri fname */
#define UNIV_CHAR 7		/* Universe units, appropriate for current mode */
#define POS_UNIV_CHAR 8	/* Positive Universe units */
#define DOS_FN_CHAR 9	/* DOS 8-char filename & 3-char ext */

/* Editable edit_phase settings */

#define EP_IDLE 0		/* Nothing happening edit-wise */
#define EP_STARTED 1	/* One editable is fully hilighted */
#define EP_WORKING 2	/* Editable has been clicked or typed */

extern void tog_edit(Dialog *d);

/* The following define, if activated, turns off an editable if it is	*/
/* active and the user clicks on another item.  This allows key equates	*/
/* to operate properly.		*/

#define EDIT_TURNOFF

extern Dialog *cur_edit;
extern int edit_phase, dialog_done;

/* Scrolling name box structures */

typedef struct
{
   char *name;
   void *next;
} Name_list;

typedef struct
{
   short yoff;
   short ycount;
   short name_count;
   short knob_height;
   short top_name;
   Name_list *name_list;
   Dialog *slider, *box;
} Scroller;

/* Slider structures */

typedef struct
{
   short min;
   short max;
   short value;
} Slider;

/* Slider substitution list */

typedef struct
{
   short slider;
   short down;
   short up;
   short type;
   short action;
   Slider *sdata;
} SlidSub;

/* Slider types */

#define SLID_NUM 0	/* Numbered slider */
#define SLID_REG 1	/* Plain slider */

/* General functions in dialog library */

extern int dlg_item_count(Dialog *root, int index);
extern void init_dialog(Dialog *root, Editable *ed, unsigned char *ic);
extern void ready_dialog(Dialog *root, Editable *ed, 
			 SeeSub *s, FeelSub *f, RadSub *r, ScrSub *c, SlidSub *l);
extern void center_dialog(Dialog *root);
extern void save_under_dialog(Dialog *root);
extern void draw_dialog(Dialog *root);
extern void draw_dialog_entry(Dialog *root, int index);
extern int in_item(Dialog *d, int x, int y);
extern int feel_dialog_entry(Dialog *root, int index, int x, int y);
extern int key_dialog_entry(Dialog *root, int index, int key);
extern void dialog_frame(int x1, int y1, int x2, int y2, int c1, int c2);
extern void out_frame(Dialog *d);
extern void box_around(Dialog *d, int color);
extern void box_around(Dialog *d, int color);
extern void in_frame(Dialog *d);
extern void draw_outbox(Dialog *d, int color);
extern void draw_inbox(Dialog *d, int color);
extern void button_out(Dialog *d, int textcolor, int boxcolor);
extern void button_in(Dialog *d, int textcolor, int boxcolor);
extern void centered_text(Dialog *d, int textcolor, int bgcolor, int erasefirst);
extern void do_dialog(Dialog *root, int initial_edit);
extern void restore_under_dialog(void);
extern void un_edit(Dialog *d);
extern void tog_edit(Dialog *d);
extern void draw_edit_chars(Dialog *d, int start);
/* Given an entry in a dialog array (*d), find_root searchs backwards through the array
   looking for the root dialog entry and returns a pointer to that entry */
extern Dialog * find_root(Dialog *d);
extern void draw_item(Dialog *d);
extern void draw_item_number(Dialog *root, int index);
extern void draw_radio_group(Dialog *root, int *var, int index);
extern int press_button(Dialog *d);
extern void init_editable(Dialog *d, char *string);
extern void update_edit(Dialog *d, int startpos);

extern int dialog_done;

/* Name-list / Scroller box functions in dialog library */

extern char * clone_string(char *s);
extern int add_to_name_list(Name_list **list, char *string);
extern int add_to_end_of_name_list(Name_list **list, char *string);
extern int insert_before_name_list(Name_list **n, int index, char *string);
extern int els_in_list(Name_list *n);
extern void sort_array(char **array, int count, int (*cmp)(char *, char *));
extern void sort_name_list(Name_list *list);
extern void reverse_name_list(Name_list **n);
extern void free_name_list(Name_list **n);
extern void init_scroller(Scroller *s, Name_list *n);
extern void calc_scroll_pos(Scroller *s, Dialog *slider);
extern void prt_list(Dialog *d);
extern void redraw_scroller(Dialog *slider, Dialog *box);
extern void fredraw_scroller(void);
extern void fpagedown(void);
extern void fpageup(void);
extern int pdn_timeout(int timeout);
extern void repeat_on_pdn(void(*v)());
extern void feel_scroll_slider(Dialog *d,int mouse);
extern void see_scroll_slider(Dialog *d);
extern void wait_penup();
extern void * which_sel(Dialog *d);
extern char * sel_name(Dialog *d);
extern void feel_scroll_up(Dialog *d, int mouse);
extern void feel_scroll_down(Dialog *d, int mouse);
extern void see_num_slider(Dialog *d);
extern void feel_num_slider(Dialog *d, int mouse);
extern void inc_num_slider(Dialog *d, int mouse);
extern void dec_num_slider(Dialog *d, int mouse);
extern void see_reg_slider(Dialog *d);
extern void feel_reg_slider(Dialog *d, int mouse);
extern void inc_reg_slider(Dialog *d, int mouse);
extern void dec_reg_slider(Dialog *d, int mouse);

#ifdef __cplusplus
} /* close the c linkage specifier */
#endif

#endif

