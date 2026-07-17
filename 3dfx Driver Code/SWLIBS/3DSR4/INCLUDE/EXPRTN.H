/******************************************************************************
  
  Header file for the IPAS interface

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

#ifndef EXPRTNS
#define EXPRTNS

#ifdef __cplusplus
extern "C" {
#endif   

#define LARGEFLOAT (1.0e20)
   
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;

typedef struct
{
   uchar r, g, b;
} Color_24;

typedef struct
{
   uint r, g, b;
} Col24;

typedef struct
{
   uchar r, g, b, a;
} Color_32, Col32;

typedef struct
{
   ushort r, g, b;
} Color_48, Col48;

typedef struct
{
   short id;
   char *s;
} DlgEntry;

typedef struct
{
   char title[41];
   int max;
   int current;
   int recall_state;
} Report;

typedef union
{
   int i;
   float f;
   long l;
   ulong ul;
   Color_32 c;
   char *s;
} OVL;

typedef struct
{
   float r, g, b;
} FColor;


/* Used to interface with 3D Studio */
extern long xpmagic, xpmainseg, xpbuffer;

/* Macro to compute the segment selector */
#ifndef FP_SEL 
#define FP_SEL(fp) ((ushort)(*(((ushort *)&(fp))+sizeof(uint)/sizeof(ushort))))
#endif

/* Macro to compute the offset */
#ifndef FP_OFF
#define FP_OFF(fp) ((unsigned int)(*(unsigned char **)&(fp)))
#endif

/* Macro to set a far pointer using a segment selector and an offset */
#ifndef FP_SET
#define FP_SET(fp, off, sel) ((*(uint *)&fp=(uint)(off)), (*(((ushort *)&fp) + sizeof(uint)/sizeof(ushort))=sel))
#endif

/* Image-processing (.IXP) data */

typedef struct
{
   int width;
   int height;
   int aspect;
   void *imagein;
   void *imageout; /* Not used if image_format == IMG_HICOLOR */
   void *alphin;
   void *alphout;  /* Not used if image_format == IMG_HICOLOR */
   int curfield;
   int totfields;
   /* The following fields are present in V3.0 and above */
   int image_format; /* Either IMG_LOCOLOR or IMG_HICOLOR */
   int start_frame, end_frame;      /* First and last frame used */
   /* The following fields are present in V3.01 and above */
   float *zbufin;  /* 3DS.SET must have a VPOST-ZBUFFER = YES setting to activate this */
   float zmin,zmax;
} Ixpdata;


/* image_format values */

#define IMG_LOCOLOR 0   /* 8 bits per component */
#define IMG_HICOLOR 1   /* 16 bits per component */

typedef struct
{
   float p[3];             /* 3D position in object's space */
   int curfield;           /* current field number being rendered */
   int totfields;          /* total number of fields */
   Col24 col;              /* color for texture mapping, opacity */
   float shininess;        /* shininess of material, texture mapping only */
   float norm_pert[3];     /* normal perturbation for bump mapping */
   float dp[3];            /* variation in 3D position for anti-aliasing */
   float mins[3], maxs[3]; /* bounding box of object for auto-scaling*/
} Sxpdata;

/* Progress data I/O structure */

typedef struct
{
   char title[41];
   int max;
   int current;
} ProgIO;

/* Vertex data I/O structure */

typedef struct
{
   int vertex;
   float x, y, z;    /* XYZ coordinates */
   float u, v;       /* Texture coords, if used */
} VertIO;

typedef struct
{
   float x, y, z;    /* XYZ coordinates */
   float u, v;       /* Texture coords, if used */
} VData;

typedef struct
{
   int start, count;
   VData _far *data;
} VListIO;

typedef struct
{
   int face;
   uint a, b, c;     /* Vertex numbers for triangular face */
   ulong flags;      /* See below for meanings */
   ulong sm_group;   /* Smoothing group bits (32) */
   uint material;    /* 0-255 */
} FaceIO;

typedef struct
{
   ushort a, b, c;   /* Vertex numbers for triangular face */
   ulong sm_group; /* Smoothing group bits (32) */
   uchar flags;    /* See below for meanings */
   uchar material; /* 0-255 */
} FData;

typedef struct
{
   int start, count;
   FData _far *data;
} FListIO;

/* Face flags bit assignments for FData structure */
#define FC_ABLINE 1
#define FC_BCLINE 2
#define FC_CALINE 4
#define KF_ABLINE 4
#define KF_BCLINE 2
#define KF_CALINE 1
#define FC_UWRAP 8
#define FC_VWRAP 16

typedef struct
{
   char name[11];
   uchar flags;
   int verts;
   int faces;
   int tverts;
   int curfield;   /* For Procedurals */
   int totfields;  /* For Procedurals */
/* The following fields are present in 3DS 4.0 and later */
   float matrix[4][3];	/* Current transformation matrix */
} ObjectIO;

/* ObjectIO flags bit assignments */

#define EXP_OBJ_HIDDEN 0x01     /* Used by PXPs only */

typedef struct
{
   char name[11];
   char filler;
   float minx;
   float maxx;
   float centerx;
   float miny;
   float maxy;
   float centery;
   float minz;
   float maxz;
   float centerz;
} ExtInfo;

typedef struct
{
   int defbut;
   char string[256];
} Alert;

typedef struct
{
   char title[40];
   char tag[30];
   int min, max, step, value;
} Slider1;

typedef struct
{
   char title[40];
   char tag1[30], tag2[30];
   int min1, max1, step1, value1;
   int min2, max2, step2, value2;
} Slider2;

typedef struct
{
   ulong size;                     /* Number of bytes in config data */
   uchar data[2048];               /* Up to 2K of data */
} Cfigdata;

typedef struct
{
   float x, y, z, hangle, vangle, tilt, width;
} Ortho;

typedef struct
{
   float x, y, z, tx, ty, tz, bank, focal;
} Cam;

typedef struct
{
   int viewtype;
   Cam camera;
   Ortho ortho;
   int fnum;
   int dev_width, dev_height;
   float dev_aspect;
   float ov_zoom;
   int animlength, start, end;
   char homedir[66], scene[13];
   /* The following fields are present in V3.0 and above */
   Cam prevcam, nextcam;
   char evenloc[9], oddloc[9];      /* .LOC file names */
} FrameInfo;

typedef struct
{
   int viewtype;
   Cam camera;
   Ortho ortho;
   int dev_width, dev_height;
   float dev_aspect;
   float ov_zoom;
   /* The following fields are present in V3.5 and above */
   int using_fields;
} AXPViewInfo;

/* Values in 'viewtype' field: */

#define VIEW_TOP 0x3010
#define VIEW_BOTTOM 0x3020
#define VIEW_LEFT 0x3030
#define VIEW_RIGHT 0x3040
#define VIEW_FRONT 0x3050
#define VIEW_BACK 0x3060
#define VIEW_USER 0x3070
#define VIEW_CAMERA 0x3080

typedef struct
{
   int using_fields;
   int field_order;        
   Cam cam1, cam2;
   /* The following fields are present in V3.0 and above */
   Cam prevcam, nextcam;
} FieldInfo;

typedef struct
{
   float master_scale;
   int type;
} Mscale;

/* The master scale types */
/* One universe unit = (master_scale)(type) */
/* Example: master_scale=7, type=2: 1 unit=7 Centimeters */

#define MS_IN 0 /* Inches */
#define MS_FT 1 /* Feet */
#define MS_CM 2 /* Centimeters */
#define MS_MT 3 /* Meters */

typedef struct
{
   long magic;     /* If 3DS >2.0, this will contain VER_MAGIC */
   int version;    /* Two-decimal version #, example: 231 = V2.31 */
} VersInfo;

/* Matrix packet */

typedef struct
{
   float matrix[4][3];
} Matrix;

typedef struct
{
   long magic;
} GammaCapable;

/* .BXP packets */

typedef struct
{
   unsigned char r, g, b;
} BXPColor;

typedef struct
{
   char filename[81];
} BXPLoadStart;

typedef struct
{
   BXPColor _far *image;
   uchar _far *alpha;
} BXPLoadFinish;

typedef struct
{
   char filename[81];
   BXPColor _far *image;
   uchar _far *alpha;
   int width;
   int height;
   int frame;
} BXPAnimate;

typedef struct
{
   char filetype[21];
   int width;
   int height;
   int has_alpha;
   float aspect;
   float gamma;
   int frames;
   char desc1[77];
   char desc2[77];
   char desc3[77];
   char desc4[77];
} BXPInfo;

typedef struct
{
   char filename[81];
   BXPColor _far *image;
   uchar _far *alpha;
   int width;
   int height;
   int has_alpha;
   float aspect;
   float gamma;
   char desc1[77];
   char desc2[77];
   char desc3[77];
   char desc4[77]; 
} BXPSaveImage;

/* IPAS Direct Communication Structures */

typedef struct
{
   char title[41];
   char path[66];
   char file[13];
   char exts[11][4];
   char selection[81];
} FSel;

typedef struct
{
   char mtlname[17];
   uchar mtlnum;
} MtlIO;

typedef struct
{
   int x, y, w, h, r, g, b;
} TCParm;

typedef struct
{
   int opcode;
   int params[5];
   union
   {
      char string1[256];
      FSel fs;
      MtlIO mtl;
      TCParm tc;
   } s1;
   union
   {
      char string2[256];
   } s2;
   int mouse_x, mouse_y, umouse_x, umouse_y, mouse_button, mouse_moved;
   int key_hit, key, kstate;
   void _far *ptr1;
   void _far *ptr2;
   void _far *ptr3;
   float f1, f2;
} GFXComm;

/* IPAS Graphics routine information */

typedef struct
{
   void _far *regs;
   ulong entry;
   GFXComm _far *buf;
   int sc_width, sc_height;
   int ch_width, ch_height;
   int colors;
   int paletted;
} GFXInfo;

/* KXP Direct-call communication structure */

typedef struct
{
   float x, y, z;
} Pivot;

typedef struct
{
   int opcode;
   int params[5];
   union
   {
      char string1[256];
      struct
      {
	 float mins[3];  
	 float maxs[3];
      } box;
      float tm[4][3];
      VData _far *vdata;  /* far pointer to vertex data */
      FData _far *fdata;  /* far pointer to face data */
      float f;  
   } s1;
   union
   {
      char string2[256];
   } s2;
   Pivot pivot;
   void _far *ptr1;
   void _far *ptr2;
   void _far *ptr3;
} KXPComm;

typedef struct
{
   void _far *regs;
   ulong entry;
   KXPComm _far *buf;
} KXPInfo;

/* PXP Direct-call communication structure */

typedef struct
{
   int opcode;
   int params[5];
   union
   {
      char string1[256];
      VData _far *vdata;  /* far pointer to vertex data */
      FData _far *fdata;  /* far pointer to face data */
   } s1;
   union
   {
      char string2[256];
   } s2;
   void _far *ptr1;
   void _far *ptr2;
   void _far *ptr3;
} PXPComm;

typedef struct
{
   void _far *regs;
   ulong entry;
   PXPComm _far *buf;
} PXPInfo;

typedef struct
{
   int hitflags;
   char prompt[61];
} KXPPick;


typedef struct
{
   void _far *ptr;
   int nbytes;
} AppDataInfo;

typedef struct
{
   ulong magic;    /* Magic number */
   int opcode;     /* What .EXP wants to do in 3D Studio */
   int usercode;   /* What .EXP wants to do when it gets back */
   int status;     /* How it all came out */
   union           /* Parameters for the opcodes & returned values */
   {
      int integer;
      char string[256];
      VertIO vert;
      VListIO vlist;
      FaceIO face;
      FListIO flist;
      ObjectIO object;
      MtlIO mtl;
      ProgIO progress;
      Alert alert;
      Slider1 s1;
      Slider2 s2;
      Cfigdata cfig;
      Ixpdata ixp;
      Sxpdata sxp;
      FrameInfo finfo;
      AXPViewInfo vinfo;
      ExtInfo exts;
      FieldInfo fldinfo;
      Mscale msc;
      Matrix matrix;
      char nodename[32];
      GammaCapable gamma;
      BXPLoadStart iload1;
      BXPLoadFinish iload2;
      BXPAnimate ianim;
      BXPInfo iinfo;
      BXPSaveImage isave;
      GFXInfo ginfo;
      KXPInfo kinfo;
      KXPPick kpick;
      PXPInfo pinfo;
      AppDataInfo apinfo;
   } data;
   VersInfo ver;   /* Version info added for 3DS >2.0 */
} EXPbuf;

/* Magic numbers for 3D Studio communication */

#define IXP_MAGIC       0x18273645 
#define PXP_MAGIC       0x10293847
#define PXP_MAGIC_2     0xBC51AFFE      /* Used when PXP is called from other module */
#define AXP_MAGIC       0x29384756
#define SXP_MAGIC       0x57483920
#define BXP_MAGIC       0x7C8AE40A  
#define KXP_MAGIC       0x02F22E53  
#define VER_MAGIC       0x3D25FE1D
#define GAMMA_MAGIC     0xF55CBD07
#define PXP_UNIVERSAL_MAGIC 0x7FCDEE45

/* Op codes for 3D Studio communication */

#define EXP_TERMINATE   0x0000
/*
   Codes 0x0001-0x01FF Reserved for internal interface codes
   */
/*
   Codes 0x0200-0x0FFF Reserved for user codes
   */
#define EXP_GET_VERTEX  0x1000
#define EXP_PUT_VERTEX  0x1001
#define EXP_GET_VLIST   0x1002
#define EXP_PUT_VLIST   0x1003
#define EXP_GET_FACE    0x1010
#define EXP_PUT_FACE    0x1011
#define EXP_GET_FLIST   0x1012
#define EXP_PUT_FLIST   0x1013
#define EXP_MATERIAL    0x1015
#define EXP_PICK_OBJECT 0x1020

/* IPAS 3 expands the usefulness of EXP_READY_OBJ.  If you set the verts or     */
/* faces field to -1, and place a valid object name in the name field, 3DS      */
/* will automatically copy that object's geometry for you into the output       */
/* object!                                                                                                                                      */
#define EXP_READY_OBJ   0x1021

#define EXP_CREATE_OBJ  0x1022
#define EXP_OBJ_EXIST   0x1023
#define EXP_OBJ_EXTENTS 0x1024
#define EXP_HOLD_MATRIX 0x1025
#define EXP_SLIDER1     0x1030
#define EXP_SLIDER2     0x1031
#define EXP_ALERT       0x1040
#define EXP_CONTINUE    0x1050
#define EXP_YES_NO      0x1060
#define EXP_PROMPT      0x1070
#define EXP_PROGRESS    0x1080

#define EXP_INIT_DIALOG 0x1100
#define EXP_ADD_DIALOG  0x1110
#define EXP_ADD_VAR     0x1120
#define EXP_DO_DIALOG   0x1130

#define EXP_FRAME_INF   0x1200
#define EXP_FIELD_INF   0x1210

#define EXP_V16_GRAB    0x1220  /* For testing */

/* IXP - Image processing process */
#define IXP_PROC_IMAGE  0x1300

/* SXP - Solid texture process */
#define SXP_3D_TEXT     0x1400
#define SXP_3D_OPAC     0x1401
#define SXP_3D_BUMP     0x1402

/* New functions for IPAS 3 */

/* The following command, passed in an AXP after EXP_READY_OBJ, 3DS to use the
   supplied coordinates as absolute, pre- values.  This allows an IXP to
   create objects at predetermined location and size, independent of the
   position or orientation of the master object.  It also allows AXP to create
   one object from a group of other objects, the coordinates without worrying
   about how all source objects are oriented.  May be done any time after
   EXP_READY_OBJ but before EXP_CREATE_OBJ.  */

#define EXP_ABS_INPUT_COORDS    0x2000  /* The AXP input coords are absolute */
#define EXP_ABS_OUTPUT_COORDS   0x2010  /* The AXP output coords are absolute */

/* The following command allows an AXP or PXP to select any named mesh object
   as the one it will be getting vertex and face information from.  Otherwise,
   on AXPs, the input object is the AXP's master object.  May be undone via
   EXP_DEFAULT_INPUT_OBJ. Sets input coord mode to relative.  */

#define EXP_SELECT_INPUT_OBJ    0x2020  /* Use specified named obj as input */

/* The following command causes an AXP to get further vertex/face info from
   its default master object.  Resets to relative input coords.  */

#define EXP_DEFAULT_INPUT_OBJ   0x2030  /* Use default master obj as input */

/* This command hides the mesh object.  Uses ObjectIO packet */

#define EXP_HIDE_OBJ    0x2070  /* Hide object */

/* This command prevents an IPAS process from being unloaded.  Recommended for
   those processes which want to keep information around while the process
   isn't being used.  (i.e. processes with text editor documents) */

#define EXP_DONT_UNLOAD 0x2080  /* Don't unload this process */

/* The following AXP commands allow an AXP to traverse the object hierarchy. */

#define EXP_GET_NUM_CHILD_OBJS 0x2090   /* Get number of children of curobj */
#define EXP_SELECT_CHILD_OBJ 0x20A0     /* Select nth child obj of curobj as
					   input obj*/
#define EXP_SELECT_OBJ_BY_NODENAME 0x20B0 /* select object by its full node
					     name*/

/* The following AXP command allows the AXP to access the view information for
   the current frame or field being rendered.  Information is placed in the
   AXPViewInfo structure.  */

#define EXP_AXP_VIEW    0x20C0

/* The following AXP command allowsthe AXP to read the AXP_DATA chunk
   associated with an object.  */

#define EXP_GET_APP_DATA 0x20D0

/* The following AXP commands allow the AXP to read, write, 
  or delete a particular named subchunk of the APP_DATA chunk 
  associated with an object */

#define EXP_GET_APPDATA_CHUNK 0x20E0  	/* 3DS 3.01 */
#define EXP_SET_APPDATA_CHUNK 0x20E2  	/* 3DS 3.01 */
#define EXP_DELETE_APPDATA_CHUNK 0x20E4  	/* 3DS 3.01 */

/* This function returns the current master scale from 3DS.  It is expressed
   as a float with a units indicator.  See above Mscale structure for more
   information.  */

#define EXP_MASTER_SCALE        0x5000  /* Get master scale from 3DS */

/* Get pointers to graphics function entry point, parameter buffer */

#define EXP_GFX_INFO    0x5100

/* .BXP Opcodes */

#define BXP_INITIALIZE  0x6000
#define BXP_INIT_DONE   0x6001
#define BXP_LOAD_START  0x6010
#define BXP_LOAD_FINISH 0x6020
#define BXP_IMAGE_INFO  0x6030
#define BXP_SAVE_IMAGE  0x6040
#define BXP_GET_FRAME   0x6050
#define BXP_RESPONSE    0x6060

/* Get pointers to KXP function entry point, parameter buffer */

#define EXP_KXP_INFO    0x7000
#define KXP_PICK_NODE   0x7010

/* Get pointers to PXP function entry point, parameter buffer */

#define EXP_PXP_INFO    0x8000

/* KXP_PICK_NODE hit type flags */

#define HT_OBJECT       (1)
#define HT_CAMERA       (1<<1)
#define HT_LIGHT        (1<<2)
#define HT_CAMTARG      (1<<3)
#define HT_SPOTLT       (1<<4)
#define HT_LITTARG      (1<<5)
#define HT_ANY (HT_OBJECT|HT_CAMERA|HT_LIGHT|HT_SPOTLT|HT_CAMTARG|HT_LITTARG)

#define EXP_GET_CONFIG  0xA000  /* 3DS->EXP, to put up config dialog */
#define EXP_PROC_DATA   0xA020  /* EXP->3DS, to return config data */

#define EXP_PUT_CONFIG  0xA100  /* 3DS->EXP, to send config data to exp */
#define EXP_CONFIGURED  0xA110  /* EXP->3DS, comes back after successful
				   PUT_CONFIG */

/* The following packet is sent from 3DS to a PXP if the PXP is in the
  "Program" dropdown and is called from a non-Editor module.  If the PXP can
  operate outside the Editor, it should respond by placing the
  PXP_UNIVERSAL_MAGIC code into the status field of the packet.  */

#define EXP_UNIVERSAL_CHECK     0xA200  /* Is this PXP universal? (PXP only) */

#define EXP_NOCODE      0xFFFD  /* Use to clear out opcode */
#define EXP_NOP         0xFFFE  /* No operation */
#define EXP_STARTUP     0xFFFF    

/* Types of executable processes supported */

#define EXP_IXP 0       /* Image */
#define EXP_PXP 1       /* Procedural Object */
#define EXP_AXP 2       /* Animated Procedural Object */
#define EXP_SXP 3       /* Solid Texture */
#define EXP_EXP 4       /* Plain ol' Vanilla EXP */
#define EXP_EXE 5       /* Regular executable */
#define EXP_BAT 6       /* Batch file */
#define EXP_BXP 7       /* Bitmap I/O */
#define EXP_KXP 8       /* Keyframer key maker */

/* .BXP status codes */

#define BXP_SUCCESS 1
#define BXP_CANT_OPEN 0
#define BXP_INVALID_FORMAT -1
#define BXP_NO_RAM -2
#define BXP_CORRUPTED_MAP -3
#define BXP_WRITE_ERROR -4
#define BXP_NOT_WRITABLE -5

/* Code to handle .EXP processes */

extern void nfstrcpy(char _far *to, char *from);
extern void fnstrcpy(char *to, char _far *from);
extern void far_to_near(void *to, void _far *from, int count);
extern void near_to_far(void _far *to, void *from, int count);
extern void far_to_far(void _far *to, void _far *from, int count);
extern void near_to_far(void _far *to, void *from, int size);
extern void do_GFX(void);
extern void do_PXP(void);
extern void do_KXP(void);

/* IPAS Graphics Interface Function Codes */

#define GFX_CLEARSCREEN 0x1000
#define GFX_SCRN_INFO	0x1002		/* (INFO) */	/* 3DS 4.0 */
#define GFX_REDRAW	0x1005
#define GFX_INIT_REDRAW	0x1008						/* 3DS 4.0 */
#define GFX_GET_HOLE	0x1010	/* X, Y, W, H */
#define GFX_PUT_HOLE	0x1020
#define GFX_LINE	0x1030  /* X1, Y1, X2, Y2, COLOR */
#define GFX_XORLINE	0x1040	/* X1, Y1, X2, Y2, COLOR */
#define GFX_HLINE	0x1050	/* Y, X1, X2, COLOR */
#define GFX_XORHLINE	0x1060	/* Y, X1, X2, COLOR */
#define GFX_VLINE	0x1070	/* X, Y1, Y2, COLOR */
#define GFX_XORVLINE	0x1080	/* X, Y1, Y2, COLOR */
#define GFX_SET_CLIP	0x1090	/* X, Y, W, H */
#define GFX_CLIPFRAME	0x10A0	/* X, Y, W, H, COLOR */
#define GFX_CLIP_LINE	0x10C0	/* X1, Y1, X2, Y2, COLOR */
#define GFX_CLIP_DLINE	0x10D0	/* X1, Y1, X2, Y2, COLOR */
#define GFX_CBLOCK	0x10E0	/* X, Y, W, H, COLOR */
#define GFX_XOR_CBLOCK	0x10E2	/* X, Y, W, H, COLOR */
#define GFX_FRAME	0x10F0	/* X, Y, W, H, COLOR */
#define GFX_XORFRAME	0x1100	/* X, Y, W, H, COLOR */
#define GFX_PUTDOT	0x1110	/* X, Y, COLOR */
#define GFX_TEXT	0x1200	/* STRING/X, Y, COLOR */
#define GFX_2TEXT	0x1210	/* STRING/X, Y, FCOLOR, BCOLOR */
#define GFX_XORTBLK	0x1220	/* X, Y, CHARS, COLOR */
#define GFX_8X16_TEXT	0x1230	/* STRING/X, Y, FCOLOR, BCOLOR */
#define GFX_BLIT	0x1240	/* IMG, X, Y, W, H, COLOR */
#define GFX_C_BLITGET	0x1250	/* IMG, X, Y, W, H */
#define GFX_C_BLITGET2	0x1251	/* IMG,X,Y,W,H */	/* 3DS 3.01 */
#define GFX_C_BLITPUT	0x1260	/* IMG, X, Y, W, H */
#define GFX_C_BLITPUT2	0x1261	/* IMG,X,Y,W,H */	/* 3DS 3.01 */
#define GFX_PAL_GET	0x1270	/* COLOR, COUNT, (DATA) */
#define GFX_PAL_SET	0x1280	/* COLOR, COUNT, DATA */
#define GFX_TC_BLITGET	0x1290	/* IMG, X, Y, W, H */
#define GFX_TC_BLITPUT	0x12A0	/* IMG, X, Y, W, H */
#define GFX_TC_CBLOCK	0x12B0	/* X, Y, W, H, R, G, B */
#define GFX_TC_PUTDOT	0x12C0	/* X, Y, R, G, B */
#define GFX_VIBRANT		0x12D0	/* (STATUS) */		/* 3DS 3.01 */
#define GFX_VLM_VERSION	0x12E0	/* (VERSION) */		/* 3DS 3.01 */

#define GFX_KEY_WAIT	0x2000
#define GFX_KEY_HIT	0x2001
#define GFX_GET_KEY	0x2002
#define GFX_KSTATE	0x2003
#define GFX_BUTUP	0x2010
#define GFX_LBUTUP	0x2020
#define GFX_RBUTUP	0x2030
#define GFX_WAIT_CLICK	0x2040
#define GFX_WAIT_INPUT	0x2050
#define GFX_POLL_MOUSE	0x2060
#define GFX_SET_MFORM	0x2070	/* FORM, DRAW */
#define GFX_GET_MFORM	0x2080	/* (FORM) */
#define GFX_MOUSE_XY	0x2090	/* X, Y */
#define GFX_MOUSE_SCALE	0x20A0						/* 3DS 3.5 */
#define GFX_MOUSE_SCRN	0x20B0						/* 3DS 3.5 */
#define GFX_USER_MFORM	0x20C0  /* FORM */			/* 3DS 4.0 */

#define GFX_CONTINU_LINE	0x3000	/* STRING */
#define GFX_CONTINU_2LINE	0x3010	/* STRING, STRING */
#define GFX_YES_NO_LINE		0x3012	/* STRING, (RESULT) */
#define GFX_PROMPT		0x3014	/* STRING */
#define GFX_REPORT			0x3015	/* STRING */	/* 3DS 4.0 */
#define GFX_PLEASE_WAIT		0x3016	/* STRING */	/* 3DS 4.0 */
#define GFX_ALERT		0x3020	/* BUTTON, STRING, (RESULT) */
#define GFX_FILE_SELECTOR	0x3030	/* TITLE, PATH, FILE, EXTS, (SELECTION) */
#define GFX_GET_PATHS		0x3040	/* TYPE, INDEX, (PATH), (FILE) */
#define GFX_MTL_SELECTOR	0x3050	/* (NUMBER), (NAME) */
#define GFX_CURRENT_MTL		0x3060	/* (NUMBER), (NAME) */
#define GFX_MTL_NAME		0x3062	/* NUMBER, (NAME) */
#define GFX_MTL_NUMBER		0x3064	/* NAME, (NUMBER) */
#define GFX_COLOR_SELECTOR	0x3070	/* COLORIN, (COLOROUT) */
#define GFX_TEXT_EDITOR		0x3080	/* DOCIN, (DOCOUT), CODEIN, (CODEOUT) */
#define GFX_SET_TEX_EXTS	0x3081	/* EXTS */					/* 3DS 3.01 */
#define GFX_SET_TEX_PF		0x3082	/* PATH,FILE */				/* 3DS 3.01 */
#define GFX_GET_TEX_PF		0x3083	/* (PATH,FILE) */			/* 3DS 3.01 */
#define GFX_GET_TEX_DIRTY	0x3084	/* (DIRTY) */				/* 3DS 4.0 */
#define GFX_SET_TEX_DIRTY	0x3085	/* DIRTY */					/* 3DS 4.0 */
#define GFX_STAND_BY		0x3090	/* STRING */
#define GFX_FINDFIRST		0x30A0	/* PATTERN, ATTRIB, DATA, STATUS */
#define GFX_FINDNEXT		0x30B0	/* DATA, STATUS */
#define GFX_SHOW_GASGAUGE	0x30C0	/* TITLE */
#define GFX_SET_GASGAUGE	0x30D0	/* TITLE, CURRENT, TOTAL */
#define GFX_PROGRESS		0x30E0	/* TITLE,CURRENT,TOTAL */	/* 3DS 4.0 */

#define GFX_BITMAP_INFO		0x3100	/* NAME, (INFO), (STATUS) */
#define GFX_LOAD_BITMAP		0x3110	/* NAME, W, H, PTR, (STATUS) */
#define GFX_SAVE_BITMAP		0x3111	/* NAME, W, H, ASPECT, PTR, GAMMA, (STATUS) */
#define GFX_RESIZE_BITMAP	0x3120	/* PTRIN, WIN, HIN, PTROUT, WOUT, HOUT, (STATUS) */
#define GFX_CALC_PALETTE	0x3130	/* PTR, W, H, COLORS, (PALETTE) */
#define GFX_24_TO_8_BIT		0x3140	/* PTRIN, W, H, COLORS, PALETTE, PTROUT, (STATUS) */

#define GFX_DEBUG_PRINT		0x3150	/* STRING */

#define GFX_GET_80HZ		0x3160	/* (TIME) */

#define GFX_GAMMA_INFO		0x3170	/* GAMMADATA */
#define GFX_DEGAMMA		0x3180	/* COLORIN, COUNT, GAMMA, (COLOROUT) */
#define GFX_GAMMA		0x3190	/* COLORIN, COUNT, GAMMA, (COLOROUT) */

#define GET_AXP_CFGNAME		0x31A0	/* (NAME) */ 		/* 3DS 3.01 */

#define GET_PROGMODE		0x31B0	/* (MODE) */ 		/* 3DS 3.01 */

#define HIDDEN_STATUS		0x31C0	/* NAME,(HIDDEN) */ /* 3DS 3.01 */
#define HIDDEN_IN_EDITOR	0x31C2	/* NAME,(HIDDEN) */	/* 3DS 4.0 */
#define HIDDEN_IN_KEYFRAMER	0x31C4	/* NAME,(HIDDEN) */	/* 3DS 4.0 */
#define SET_EDITOR_DIRTY	0x31C8	/* DIRTY */			/* 3DS 4.0 */
#define SET_KEYFRAMER_DIRTY	0x31CA	/* DIRTY */			/* 3DS 4.0 */

#define IN_SLAVE_MODE		0x31D0	/* (STATUS) */		/* 3DS 3.01 */
#define SLAVE_RENDER_TYPE	0x31E0	/* (TYPE) */		/* 3DS 3.01 */

#define GET_SETFILE_NAME	0x31F0	/* (NAME) */		/* 3DS 4.0 */

#define GET_ATMOSPHERE		0x3200	/* (DATA) */		/* 3DS 3.01 */
#define SET_ATMOSPHERE		0x3210	/* DATA,(STATUS) */	/* 3DS 3.01 */

#define DO_3DS_HOLD			0x3300	/* (STATUS) */		/* 3DS 3.01 */
#define DO_3DS_FETCH		0x3310	/* (STATUS) */		/* 3DS 3.01 */

#define DO_3DS_LOAD			0x3400	/* (NAME,STATUS) */		/* 3DS 3.01 */
#define DO_3DS_SAVE			0x3410	/* (NAME,STATUS) */		/* 3DS 3.01 */

#define GFX_VIEWPORTS		0x4000	/* PTR */				/* 3DS 4.0 */
#define GFX_SET_VIEWPORT	0x4010	/* INDEX */				/* 3DS 4.0 */
#define GFX_DRAW_VIEWPORT	0x4015	/* INDEX */				/* 3DS 4.0 */
#define GFX_MESH_AT			0x4020	/* X,Y,(NAME,STATUS) */	/* 3DS 4.0 */
#define GFX_CAMERA_AT		0x4030	/* X,Y,(NAME,TARGET,STATUS) */	/* 3DS 4.0 */
#define GFX_LIGHT_AT		0x4040	/* X,Y,(NAME,STATUS) */	/* 3DS 4.0 */
#define GFX_SPOT_AT			0x4050	/* X,Y,(NAME,TARGET,STATUS) */	/* 3DS 4.0 */

#define RENDER_SCREEN_TYPE	0x4100	/* (TYPE) */			/* 3DS 4.0 */
#define GET_SCREEN_MODE		0x4110	/* (MODE) */			/* 3DS 4.0 */
#define SET_SCREEN_MODE		0x4120	/* MODE,(STATUS) */		/* 3DS 4.0 */

#define GET_SERIAL_NUMBER	0x4200	/* (STRING) */			/* 3DS 4.0 */

/* GFX_SCRN_INFO data structure (3DS 4.0) */

typedef struct
	{
	int sc_width,sc_height;
	int ch_width,ch_height;
	int colors;
	int paletted;
	} ScrnInfo;
  
/* GFX_GET_PATHS path ID's */

#define GFX_3DS_PATH 0
#define GFX_SHP_PATH 1
#define GFX_LFT_PATH 2
#define GFX_MLI_PATH 3
#define GFX_FNT_PATH 4
#define GFX_IMG_PATH 5
#define GFX_MAP_PATH 6  /* Call with secondary parameter for map path # */
#define GFX_HOME_PATH 7			/* 3DS 3.01 */
#define GFX_PROCESS_PATH 8		/* 3DS 3.01 */
#define GFX_VP_PATH 9			/* 3DS 3.01 */
#define GFX_PRJ_PATH 10			/* 3DS 3.01 */
#define GFX_DRIVER_PATH 11		/* 3DS 3.01 */
#define GFX_TEMP_PATH 12		/* 3DS 3.01 */
#define GFX_FLI_PATH 13			/* 3DS 3.01 */
#define GFX_PREVIEW_PATH 14		/* 3DS 3.01 */

/* Standard 3DS Colors */

#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define LTGRAY 5
#define DKGRAY 6
#define MDGRAY 7
#define GRAY 8
#define LBLUE 9
#define LGREEN 10
#define LCYAN 11
#define LRED 12
#define ORANGE 13
#define YELLOW 14
#define WHITE 15

/* Mouse form definitions */

#define C_ARROW 0
#define C_CROSS 1
#define C_55BOX 2
#define C_OMNIBOX 3
#define C_HORIZBOX 4
#define C_VERTBOX 5
#define C_UPBOX 6
#define C_DOWNBOX 7
#define C_RIGHTBOX 8
#define C_LEFTBOX 9
#define C_PENCIL 10
#define C_QMARROW 11
#define C_MARROW 12
#define C_CW 13
#define C_CCW 14
#define C_USER 15		/* 3DS 4.0 */

/* User-defined mouse data structure (3DS 4.0) */

typedef struct
	{
	int hotx;
	int hoty;
	char mask[32];
	char data[32];
	} UserMouse;

/* Text editor document structure */

typedef struct
{
   char _far *text;        /* Pointer to text */
   int length;             /* Length of text */
   int bufsize;            /* Length of text buffer */
   int cursor;             /* Offset to cursor */
   int dropct;
   void _far *drop;        /* Additional dropdown menu */
   int toolct;
   void _far *tools;       /* Tool bar */
} TextEd;

/* Text Editor call codes */

#define TEX_STARTUP 0
#define TEX_REENTER 1
#define TEX_REENTER_REDRAW 2
#define TEX_CLOSE 99

/* Text Editor return codes */

#define TEX_FAILED -1
#define TEX_CLOSED 0

/* Text Editor tool icon data */

typedef struct
{
   uchar out_icondark[128];        /* All icons 32 X 32 X 1BIT */
   uchar out_iconlight[128];
   uchar in_icondark[128];
   uchar in_iconlight[128];
   int retval;
} EdTool;

/* Text Editor additional dropdown data (Version 4.0) */

typedef struct
{
char string[32];
int retval;		/* 0 for heading, -1 for divider, nonzero for entries */
} EdDrop;

/* Dropdown return value presets */

#define DROP_SEPARATOR 0
#define DROP_HEADER 1

/* Dropdown/Toolbar Button accelerator prefix (r4)  */
/* OR this with the accelerator's keyboard scancode */
/* in the lower word to get keyboard accelerators   */
/* for text editor dropdowns and toolbar buttons!   */

#define ACCELERATOR 0x7F4C0000

/* Bitmap Information data structure */

typedef struct
{
   char filetype[21];
   int width, height;
   float aspect;
   float gamma;
   int frames;
   char desc1[77];
   char desc2[77];
   char desc3[77];
   char desc4[77];
} BitmapInfo;

typedef struct
{
   char reserved[21];
   char attribute;
   short time, date;
   long size;
   char name[13];
} FNdata;

typedef struct
{
   float disp_gamma;               /* Main display gamma */
   float file_in_gamma;            /* Input file default gamma */
   float file_out_gamma;           /* Output file gamma */
   int gamma_switch;               /* 1=gamma on, 0=gamma off */
} GammaData;

/* Program modes */

#define MODE_SHAPER 1
#define MODE_LOFTER 2
#define MODE_EDITOR 3
#define MODE_KEYFRAMER 4
#define MODE_MATERIALS 5

/* Slave render types */

#define SLAVE_TYPE_EDITOR		0
#define SLAVE_TYPE_KEYFRAMER	1

/* Atmosphere Data Structures */

typedef struct
	{
	float neardens;
	float fardens;
	FColor color;
	int fog_bg;
	} IPASFog;

typedef struct
	{
	float zmin,zmax;
	float density;
	int type;
	FColor color;
	int fog_bg;
	} IPASLFog;

typedef struct
	{
	float neardim;
	float fardim;
	int dim_bg;
	} IPASDistCue;

typedef struct
	{
	int type;
	union
		{
		IPASFog fog;
		IPASLFog lfog;
		IPASDistCue dcue;
		} data;
	} IPASAtmosphere;

/* Atmosphere types */

#define ATMO_NONE 0		/* No atmosphere in use */
#define ATMO_FOG 1		/* Simple fog */
#define ATMO_LFOG 2		/* Layered fog */
#define ATMO_DCUE 3		/* Distance-cueing */

/* Layered fog types */

#define LFOG_UNIFORM 0	/* Uniform */
#define LFOG_BOTTOM 1	/* Falls off at bottom */
#define LFOG_TOP 2		/* Falls off at top */

/* Viewport data structure (3DS 4.0) */

typedef struct
	{
	int view;				/* See below */
	int flags;				/* See below */
	int x,y,width,height;	/* Viewport area */
	float zoom;				/* Pixels per universe unit */
	float cx,cy,cz;			/* Window center in world coords */
	float hangle,vangle;	/* User view angles */
	char camera[11];		/* Camera or spotlight name */
	} Viewport;

/* Viewport view values */

#define VP_CAMERA -1
#define VP_NOVIEW 0
#define VP_TOP 1
#define VP_BOTTOM 2
#define VP_LEFT 3
#define VP_RIGHT 4
#define VP_FRONT 5
#define VP_BACK 6
#define VP_SPECIAL 7		/* AKA User view */
#define VP_MODSHAPE 8
#define VP_MODSCALEXZ 9
#define VP_MODSCALEYZ 10
#define VP_MODBEVEL 11
#define VP_MODTEETERXZ 13
#define VP_MODTEETERYZ 14
#define VP_MODFITXZ 15
#define VP_MODFITYZ 16
#define VP_MODTWIST 17
#define VP_SPOTLIGHT 18

/* Viewport flags */

#define WIND_ENABLE 0x0001		/* Enabled */
#define WIND_SNAP 0x0002		/* Snap is in effect */
#define WIND_GRID 0x0004		/* Grid is in effect */
#define WIND_VIEWLOCK 0x0008	/* View type is locked */
#define WIND_FAST 0x0010		/* Fastdraw view */
#define WIND_USER 0x0020		/* User axis display in effect */
#define WIND_SAFETY 0x0040		/* Safe frame in effect */
#define WIND_BGND 0x0080		/* Background shown */
#define WIND_SCRLOCK 0x0100		/* Scroll locked */

typedef struct
	{
	int viewports;		/* Current number of viewports */
	int active;			/* Index of active viewport */
	int fastdraw;		/* Global fastdraw flag */
	int faststep;		/* Fastdraw face step */
	Viewport port[4];	/* Viewport data */	
	} ViewportInfo;

/* Rendering screen types */

#define SCREEN_ERROR -1			/* Something wrong! */
#define SCREEN_SINGLE 0			/* Single screen -- Space bar toggle */
#define SCREEN_DUAL 1			/* Dual screen -- No toggle */

/* Dual rendering screen modes */

#define SCREEN_MODE_ERROR -1	/* You did something wrong! */
#define SCREEN_MODE_MAIN 0		/* On user interface display */
#define SCREEN_MODE_RENDER 1	/* On rendering screen display */

/* Handy graphics macros */

extern GFXInfo *GI;
extern GFXComm _far *GC;

#define gfx_clearscreen() \
        {GC->opcode=GFX_CLEARSCREEN; do_GFX();}
#define gfx_scrn_info(ptr) \
		{GC->ptr1=(void _far *)ptr; GC->opcode=GFX_SCRN_INFO; do_GFX();}
#define gfx_redraw() \
        {GC->opcode=GFX_REDRAW; do_GFX();}
#define gfx_init_redraw() \
		{GC->opcode=GFX_INIT_REDRAW; do_GFX();}
#define gfx_get_hole(x, y, w, h) \
        {GC->opcode=GFX_GET_HOLE; GC->params[0]=x; GC->params[1]=y; \
        GC->params[2]=w; GC->params[3]=h; do_GFX();}
#define gfx_put_hole() \
	{GC->opcode=GFX_PUT_HOLE; do_GFX();}
#define gfx_line(x1,y1,x2,y2,color) \
	{GC->opcode=GFX_LINE; GC->params[0]=x1; GC->params[1]=y1; \
	GC->params[2]=x2; GC->params[3]=y2; GC->params[4]=color; do_GFX();}
#define gfx_xorline(x1,y1,x2,y2,color) \
	{GC->opcode=GFX_XORLINE; GC->params[0]=x1; GC->params[1]=y1; \
	GC->params[2]=x2; GC->params[3]=y2; GC->params[4]=color; do_GFX();}
#define gfx_hline(y,x1,x2,color) \
	{GC->opcode=GFX_HLINE; GC->params[0]=y; GC->params[1]=x1; \
	GC->params[2]=x2; GC->params[3]=color; do_GFX();}
#define gfx_xorhline(y,x1,x2,color) \
	{GC->opcode=GFX_XORHLINE; GC->params[0]=y; GC->params[1]=x1; \
	GC->params[2]=x2; GC->params[3]=color; do_GFX();}
#define gfx_vline(x,y1,y2,color) \
	{GC->opcode=GFX_VLINE; GC->params[0]=x; GC->params[1]=y1; \
	GC->params[2]=y2; GC->params[3]=color; do_GFX();}
#define gfx_xorvline(x,y1,y2,color) \
	{GC->opcode=GFX_XORVLINE; GC->params[0]=x; GC->params[1]=y1; \
	GC->params[2]=y2; GC->params[3]=color; do_GFX();}
#define gfx_set_clip(x,y,w,h) \
	{GC->opcode=GFX_SET_CLIP; GC->params[0]=x; GC->params[1]=y; \
	GC->params[2]=w; GC->params[3]=h; do_GFX();}
#define gfx_clipframe(x1,y1,x2,y2,color) \
	{GC->opcode=GFX_CLIPFRAME; GC->params[0]=x1; GC->params[1]=y1; \
	GC->params[2]=x2; GC->params[3]=y2; GC->params[4]=color; do_GFX();}
#define gfx_clip_line(x1,y1,x2,y2,color) \
	{GC->opcode=GFX_CLIP_LINE; GC->params[0]=x1; GC->params[1]=y1; \
	GC->params[2]=x2; GC->params[3]=y2; GC->params[4]=color; do_GFX();}
#define gfx_clip_dline(x1,y1,x2,y2,color) \
	{GC->opcode=GFX_CLIP_DLINE; GC->params[0]=x1; GC->params[1]=y1; \
	GC->params[2]=x2; GC->params[3]=y2; GC->params[4]=color; do_GFX();}
#define gfx_cblock(x,y,w,h,color) \
	{GC->opcode=GFX_CBLOCK; GC->params[0]=x; GC->params[1]=y; \
	GC->params[2]=w; GC->params[3]=h; GC->params[4]=color; do_GFX();}
#define gfx_xor_cblock(x,y,w,h,color) \
	{GC->opcode=GFX_XOR_CBLOCK; GC->params[0]=x; GC->params[1]=y; \
	GC->params[2]=w; GC->params[3]=h; GC->params[4]=color; do_GFX();}
#define gfx_frame(x,y,w,h,color) \
	{GC->opcode=GFX_FRAME; GC->params[0]=x; GC->params[1]=y; \
	GC->params[2]=w; GC->params[3]=h; GC->params[4]=color; do_GFX();}
#define gfx_xorframe(x,y,w,h,color) \
	{GC->opcode=GFX_XORFRAME; GC->params[0]=x; GC->params[1]=y; \
	GC->params[2]=w; GC->params[3]=h; GC->params[4]=color; do_GFX();}
#ifdef IPAS_R3_ONLY
/* BOGUS transformation of put dot to gfx_cblock because of bug in 3D Studio */
#define gfx_putdot(x,y,color) \
	{GC->opcode=GFX_CBLOCK; GC->params[0]=x; GC->params[1]=y; \
	GC->params[2]=1; GC->params[3]=1; GC->params[4]=color; do_GFX();}
#else
#define gfx_putdot(x,y,color) \
	{GC->opcode=GFX_PUTDOT; GC->params[0]=x; GC->params[1]=y; \
	GC->params[2]=color; do_GFX();}
#endif
#define gfx_text(s,x,y,color) \
	{GC->opcode=GFX_TEXT; nfstrcpy(GC->s1.string1,s); GC->params[0]=(x); \
	GC->params[1]=(y); GC->params[2]=color; do_GFX();}
#define gfx_2text(s,x,y,fc,bc) \
	{GC->opcode=GFX_2TEXT; nfstrcpy(GC->s1.string1,s); GC->params[0]=(x); \
	GC->params[1]=(y); GC->params[2]=fc; GC->params[3]=bc; do_GFX();}
#define gfx_xortblk(x,y,chars,color) \
	{GC->opcode=GFX_XORTBLK; GC->params[0]=x; GC->params[1]=y; \
	GC->params[2]=chars; GC->params[3]=color; do_GFX();}
#define gfx_8x16_text(s,x,y,fc,bc) \
	{GC->opcode=GFX_8X16_TEXT; nfstrcpy(GC->s1.string1,s); GC->params[0]=x; \
	GC->params[1]=y; GC->params[2]=fc; GC->params[3]=bc; do_GFX();}
#define gfx_blit(i,x,y,w,h,c) \
	{GC->opcode=GFX_BLIT; GC->ptr1=(void _far *)i; GC->params[0]=x; \
	GC->params[1]=y; GC->params[2]=w; GC->params[3]=h; GC->params[4]=c; \
	do_GFX();}
#define gfx_c_blitget(i,x,y,w,h) \
	{GC->opcode=GFX_C_BLITGET; GC->ptr1=(void _far *)i; GC->params[0]=x; \
	GC->params[1]=y; GC->params[2]=w; GC->params[3]=h; do_GFX();}
#define gfx_c_blitget2(i,x,y,w,h) \
	{GC->opcode=GFX_C_BLITGET2; GC->ptr1=(void _far *)i; GC->params[0]=x; \
	GC->params[1]=y; GC->params[2]=w; GC->params[3]=h; do_GFX();}
#define gfx_c_blitput(i,x,y,w,h) \
	{GC->opcode=GFX_C_BLITPUT; GC->ptr1=(void _far *)i; GC->params[0]=x; \
	GC->params[1]=y; GC->params[2]=w; GC->params[3]=h; do_GFX();}
#define gfx_c_blitput2(i,x,y,w,h) \
	{GC->opcode=GFX_C_BLITPUT2; GC->ptr1=(void _far *)i; GC->params[0]=x; \
	GC->params[1]=y; GC->params[2]=w; GC->params[3]=h; do_GFX();}
#define gfx_pal_get(c,n,d) \
	{GC->opcode=GFX_PAL_GET; GC->ptr1=(void _far *)d; GC->params[0]=c; \
	GC->params[1]=n; do_GFX();}
#define gfx_pal_set(c,n,d) \
	{GC->opcode=GFX_PAL_SET; GC->ptr1=(void _far *)d; GC->params[0]=c; \
	GC->params[1]=n; do_GFX();}
#define gfx_tc_blitget(i,xx,yy,ww,hh) \
	{ GC->ptr1=(void _far *)i; GC->s1.tc.x=xx; GC->s1.tc.y=yy; \
	GC->s1.tc.w=ww; GC->s1.tc.h=hh; GC->opcode=GFX_TC_BLITGET; do_GFX(); }
#define gfx_tc_blitput(i,xx,yy,ww,hh) \
	{ GC->ptr1=(void _far *)i; GC->s1.tc.x=xx; GC->s1.tc.y=yy; \
	GC->s1.tc.w=ww; GC->s1.tc.h=hh; GC->opcode=GFX_TC_BLITPUT; do_GFX(); }
#define gfx_tc_cblock(xx,yy,ww,hh,rr,gg,bb) \
	{ GC->s1.tc.x=xx; GC->s1.tc.y=yy; GC->s1.tc.w=ww; GC->s1.tc.h=hh; \
	GC->s1.tc.r=rr; GC->s1.tc.g=gg; GC->s1.tc.b=bb; GC->opcode=GFX_TC_CBLOCK; \
	do_GFX(); }
#ifdef IPAS_R3_ONLY
#define gfx_tc_putdot(xx,yy,rr,gg,bb) \
	{ GC->s1.tc.x=xx; GC->s1.tc.y=yy; GC->s1.tc.w=1; GC->s1.tc.h=1; \
	GC->s1.tc.r=rr; GC->s1.tc.g=gg; GC->s1.tc.b=bb; GC->opcode=GFX_TC_CBLOCK; \
	do_GFX(); }
#else
#define gfx_tc_putdot(xx,yy,rr,gg,bb) \
	{ GC->s1.tc.x=xx; GC->s1.tc.y=yy; GC->s1.tc.r=rr; GC->s1.tc.g=gg; \
	GC->s1.tc.b=bb; GC->opcode=GFX_TC_PUTDOT; do_GFX(); }
#endif
#define gfx_vibrant(s) \
	{ GC->opcode=GFX_VIBRANT; do_GFX(); s=GC->params[0]; }
#define gfx_vlm_version(v) \
	{ GC->opcode=GFX_VLM_VERSION; do_GFX(); v=GC->params[0]; }

#define gfx_key_wait(x) \
        {GC->opcode=GFX_KEY_WAIT; do_GFX(); x=GC->params[0];}
#define gfx_key_hit(x) \
        {GC->opcode=GFX_KEY_HIT; do_GFX(); x=GC->params[0];}
#define gfx_get_key(x) \
        {GC->opcode=GFX_GET_KEY; do_GFX(); x=GC->params[0];}
#define gfx_kstate(x) \
        {GC->opcode=GFX_KSTATE; do_GFX(); x=GC->params[0];}
#define gfx_butup() \
        {GC->opcode=GFX_BUTUP; do_GFX();}
#define gfx_lbutup() \
        {GC->opcode=GFX_LBUTUP; do_GFX();}
#define gfx_rbutup() \
        {GC->opcode=GFX_RBUTUP; do_GFX();}
#define gfx_wait_click() \
        {GC->opcode=GFX_WAIT_CLICK; do_GFX();}
#define gfx_wait_input() \
        {GC->opcode=GFX_WAIT_INPUT; do_GFX();}
#define gfx_poll_mouse() \
        {GC->opcode=GFX_POLL_MOUSE; do_GFX();}
#define gfx_set_mform(form, draw) \
        {GC->opcode=GFX_SET_MFORM; GC->params[0]=form; \
        GC->params[1]=draw; do_GFX();}
#define gfx_get_mform(form) \
        {GC->opcode=GFX_GET_MFORM; do_GFX(); form=GC->params[0];}
#define gfx_mouse_xy(x, y) \
        {GC->opcode=GFX_MOUSE_XY; GC->params[0]=x; GC->params[1]=y; do_GFX();}
#define gfx_mouse_scale(s) \
        {GC->opcode=GFX_MOUSE_SCALE; do_GFX(); s=GC->params[0];}
#define gfx_mouse_scrn() \
        {GC->opcode=GFX_MOUSE_SCRN; do_GFX();}
#define gfx_user_mform(d) \
        {GC->opcode=GFX_USER_MFORM; GC->ptr1=(void _far *)d; do_GFX();}

#define gfx_continu_line(s) \
        {GC->opcode=GFX_CONTINU_LINE; nfstrcpy(GC->s1.string1, s); do_GFX();}
#define gfx_continu_2line(sa, sb) \
        {GC->opcode=GFX_CONTINU_2LINE; nfstrcpy(GC->s1.string1, sa); \
        nfstrcpy(GC->s2.string2, sb); do_GFX();}
#define gfx_yes_no_line(s, r) \
        {GC->opcode=GFX_YES_NO_LINE; nfstrcpy(GC->s1.string1, s); do_GFX(); \
        r=GC->params[0]; }
#define gfx_prompt(s) \
	{GC->opcode=GFX_PROMPT; nfstrcpy(GC->s1.string1,s); do_GFX();}
#define gfx_report(s) \
	{GC->opcode=GFX_REPORT; nfstrcpy(GC->s1.string1,s); do_GFX();}
#define gfx_please_wait(s) \
	{GC->opcode=GFX_PLEASE_WAIT; nfstrcpy(GC->s1.string1,s); do_GFX();}
#define gfx_alert(b,s,x) \
	{GC->opcode=GFX_ALERT; nfstrcpy(GC->s1.string1,s); GC->params[0]=b; \
	do_GFX(); x=GC->params[0];}
#define gfx_file_selector(t,p,f,e,s) \
	{GC->opcode=GFX_FILE_SELECTOR; nfstrcpy(GC->s1.fs.title,t); \
	nfstrcpy(GC->s1.fs.path,p); nfstrcpy(GC->s1.fs.file,f); \
	near_to_far(GC->s1.fs.exts,e,sizeof(GC->s1.fs.exts)); \
	do_GFX(); fnstrcpy(s,GC->s1.fs.selection); }
#define gfx_get_paths(type,num,p,f) \
	{GC->opcode=GFX_GET_PATHS; GC->params[0]=type; GC->params[1]=num; \
	do_GFX(); fnstrcpy(p,GC->s1.fs.path); fnstrcpy(f,GC->s1.fs.file); }
#define gfx_mtl_selector(index,name) \
	{GC->opcode=GFX_MTL_SELECTOR; do_GFX(); index=GC->s1.mtl.mtlnum; \
	fnstrcpy(name,GC->s1.mtl.mtlname); }
#define gfx_current_mtl(index,name) \
	{GC->opcode=GFX_CURRENT_MTL; do_GFX(); index=GC->s1.mtl.mtlnum; \
	fnstrcpy(name,GC->s1.mtl.mtlname); }
#define gfx_mtl_name(index,name) \
	{GC->params[0]=index; GC->opcode=GFX_MTL_NAME; do_GFX(); \
	fnstrcpy(name,GC->s1.mtl.mtlname); }
#define gfx_mtl_number(name,index) \
	{nfstrcpy(GC->s1.mtl.mtlname,name); GC->opcode=GFX_MTL_NUMBER; do_GFX(); \
	index=GC->s1.mtl.mtlnum; }
#define gfx_color_selector(in,out,changed) \
	{GC->opcode=GFX_COLOR_SELECTOR; \
	GC->params[0]=in.r; GC->params[1]=in.g; GC->params[2]=in.b; do_GFX(); \
	out.r=GC->params[0]; out.g=GC->params[1]; out.b=GC->params[2]; \
	changed=GC->params[3]; }
#define gfx_text_editor(d1,d2,cin,cout) \
	{ GC->ptr1=(void _far *)d1; GC->params[0]=cin; \
	GC->opcode=GFX_TEXT_EDITOR; do_GFX(); d2=GC->ptr1; cout=GC->params[0]; }
#define gfx_set_tex_exts(e) \
	{ near_to_far(GC->s1.fs.exts,e,sizeof(GC->s1.fs.exts)); \
	GC->opcode=GFX_SET_TEX_EXTS; do_GFX(); }
#define gfx_set_tex_pf(p,f) \
	{ nfstrcpy(GC->s1.fs.path,p); nfstrcpy(GC->s1.fs.file,f); \
	GC->opcode=GFX_SET_TEX_PF; do_GFX(); }
#define gfx_get_tex_pf(p,f) \
	{ GC->opcode=GFX_GET_TEX_PF; do_GFX(); fnstrcpy(p,GC->s1.fs.path); \
	fnstrcpy(f,GC->s1.fs.file); }
#define gfx_get_tex_dirty(d) \
	{ GC->opcode=GFX_GET_TEX_DIRTY; do_GFX(); d=GC->params[0]; }
#define gfx_set_tex_dirty(d) \
	{ GC->params[0]=d; GC->opcode=GFX_SET_TEX_DIRTY; do_GFX(); }
#define gfx_stand_by(s) \
        { nfstrcpy(GC->s1.string1, s); GC->opcode=GFX_STAND_BY; do_GFX(); }
#define gfx_findfirst(p, a, d, s) \
        { nfstrcpy(GC->s1.string1, p); GC->params[0]=a; GC->ptr1=(void _far *)d; \
        GC->opcode=GFX_FINDFIRST; do_GFX(); s=GC->params[0]; }
#define gfx_findnext(d, s) \
        { GC->ptr1=(void _far *)d; GC->opcode=GFX_FINDNEXT; do_GFX(); \
        s=GC->params[0]; }
#define gfx_show_gasgauge(t) \
        { nfstrcpy(GC->s1.string1, t); GC->opcode=GFX_SHOW_GASGAUGE; do_GFX(); }
#define gfx_set_gasgauge(t, c, n) \
        { nfstrcpy(GC->s1.string1, t); GC->params[0]=c; GC->params[1]=n; \
        GC->opcode=GFX_SET_GASGAUGE; do_GFX(); }
#define gfx_progress(t,c,n) \
		{ nfstrcpy(GC->s1.string1,t); GC->params[0]=c; GC->params[1]=n; \
		GC->opcode=GFX_PROGRESS; do_GFX(); }

#define gfx_bitmap_info(n, i, s) \
        { nfstrcpy(GC->s1.string1, n); GC->ptr1=(void _far *)i; \
        GC->opcode=GFX_BITMAP_INFO; do_GFX(); s=GC->params[0]; }
#define gfx_load_bitmap(n, w, h, p, s) \
        { nfstrcpy(GC->s1.string1, n); GC->params[0]=w; GC->params[1]=h; \
        GC->ptr1=(void _far *)p; GC->opcode=GFX_LOAD_BITMAP; do_GFX(); \
        s=GC->params[0]; }
#define gfx_save_bitmap(n, w, h, a, p, g, s) \
        { nfstrcpy(GC->s1.string1, n); GC->params[0]=w; GC->params[1]=h; \
        GC->f1=a; GC->ptr1=(void _far *)p; GC->f2=g; GC->opcode=GFX_SAVE_BITMAP; \
        do_GFX(); s=GC->params[0]; }
#define gfx_resize_bitmap(pi, wi, hi, po, wo, ho, s) \
        { GC->ptr1=(void _far *)pi; GC->params[0]=wi; GC->params[1]=hi; \
        GC->ptr2=(void _far *)po; GC->params[2]=wo; GC->params[3]=ho; \
        GC->opcode=GFX_RESIZE_BITMAP; do_GFX(); s=GC->params[0]; }
#define gfx_calc_palette(p, w, h, c, pal) \
        { GC->ptr1=(void _far *)p; GC->params[0]=w; GC->params[1]=h; \
        GC->params[2]=c; GC->ptr2=(void _far *)pal; \
        GC->opcode=GFX_CALC_PALETTE; do_GFX(); }
#define gfx_24_to_8_bit(pi, w, h, c, pal, po, s) \
        { GC->ptr1=(void _far *)pi; GC->params[0]=w; GC->params[1]=h; \
        GC->params[2]=c; GC->ptr2=(void _far *)pal; GC->ptr3=(void _far *)po; \
        GC->opcode=GFX_24_TO_8_BIT; do_GFX(); s=GC->params[0]; }
#define gfx_debug_print(s) \
        { nfstrcpy(GC->s1.string1, s); GC->opcode=GFX_DEBUG_PRINT; do_GFX(); }
#define gfx_get_80hz(time) \
        { GC->opcode=GFX_GET_80HZ; do_GFX(); time=GC->params[0]; }
#define gfx_gamma_info(g) \
        { GC->ptr1=(void _far *)g; GC->opcode=GFX_GAMMA_INFO; do_GFX(); }
#define gfx_degamma(i, c, g, o) \
        { GC->ptr1=(void _far *)i; GC->ptr2=(void _far *)o; GC->params[0]=c; \
        GC->f1=g; GC->opcode=GFX_DEGAMMA; do_GFX(); }
#define gfx_gamma(i, c, g, o) \
        { GC->ptr1=(void _far *)i; GC->ptr2=(void _far *)o; GC->params[0]=c; \
        GC->f1=g; GC->opcode=GFX_GAMMA; do_GFX(); }
#define get_axp_cfgname(name) \
	{ GC->opcode=GET_AXP_CFGNAME; do_GFX(); fnstrcpy(name,GC->s1.string1); }
#define get_progmode(mode) \
	{ GC->opcode=GET_PROGMODE; do_GFX(); mode=GC->params[0]; }
#define hidden_status(n,s) \
	{ nfstrcpy(GC->s1.string1,n); \
	GC->opcode=HIDDEN_STATUS; do_GFX(); s=GC->params[0]; }
#define hidden_in_editor(n,s) \
	{ nfstrcpy(GC->s1.string1,n); \
	GC->opcode=HIDDEN_IN_EDITOR; do_GFX(); s=GC->params[0]; }
#define hidden_in_keyframer(n,s) \
	{ nfstrcpy(GC->s1.string1,n); \
	GC->opcode=HIDDEN_IN_KEYFRAMER; do_GFX(); s=GC->params[0]; }
#define set_editor_dirty(flag) \
	{ GC->params[0]=flag; GC->opcode=SET_EDITOR_DIRTY; do_GFX(); }
#define set_keyframer_dirty(flag) \
	{ GC->params[0]=flag; GC->opcode=SET_KEYFRAMER_DIRTY; do_GFX(); }
#define in_slave_mode(s) \
	{ GC->opcode=IN_SLAVE_MODE; do_GFX(); s=GC->params[0]; }
#define slave_render_type(t) \
	{ GC->opcode=SLAVE_RENDER_TYPE; do_GFX(); t=GC->params[0]; }
#define get_setfile_name(n) \
	{ GC->opcode=GET_SETFILE_NAME; do_GFX(); fnstrcpy(n,GC->s1.string1); }

#define get_atmosphere(d) \
	{GC->opcode=GET_ATMOSPHERE; GC->ptr1=(void _far *)d; do_GFX();}
#define set_atmosphere(d,s) \
	{GC->opcode=SET_ATMOSPHERE; GC->ptr1=(void _far *)d; do_GFX(); \
	s=GC->params[0]; }

#define do_3ds_hold(s) \
	{ GC->opcode=DO_3DS_HOLD; do_GFX(); s=GC->params[0]; }
#define do_3ds_fetch(s) \
	{ GC->opcode=DO_3DS_FETCH; do_GFX(); s=GC->params[0]; }

#define do_3ds_load(n,s) \
	{ nfstrcpy(GC->s1.string1,n); GC->opcode=DO_3DS_LOAD; \
	do_GFX(); s=GC->params[0]; }
#define do_3ds_save(n,s) \
	{ nfstrcpy(GC->s1.string1,n); GC->opcode=DO_3DS_SAVE; \
	do_GFX(); s=GC->params[0]; }

#define gfx_viewports(ptr) \
	{GC->opcode=GFX_VIEWPORTS; GC->ptr1=(void _far *)ptr; do_GFX();}
#define gfx_set_viewport(i) \
	{GC->opcode=GFX_SET_VIEWPORT; GC->params[0]=i; do_GFX();}
#define gfx_draw_viewport(i) \
	{GC->opcode=GFX_DRAW_VIEWPORT; GC->params[0]=i; do_GFX();}
#define gfx_mesh_at(x,y,n,s) \
	{GC->opcode=GFX_MESH_AT; GC->params[0]=x; GC->params[1]=y; do_GFX(); \
	fnstrcpy(n,GC->s1.string1); s=GC->params[0];}
#define gfx_camera_at(x,y,n,t,s) \
	{GC->opcode=GFX_CAMERA_AT; GC->params[0]=x; GC->params[1]=y; do_GFX(); \
	fnstrcpy(n,GC->s1.string1); t=GC->params[0]; s=GC->params[1];}
#define gfx_light_at(x,y,n,s) \
	{GC->opcode=GFX_LIGHT_AT; GC->params[0]=x; GC->params[1]=y; do_GFX(); \
	fnstrcpy(n,GC->s1.string1); s=GC->params[0];}
#define gfx_spot_at(x,y,n,t,s) \
	{GC->opcode=GFX_SPOT_AT; GC->params[0]=x; GC->params[1]=y; do_GFX(); \
	fnstrcpy(n,GC->s1.string1); t=GC->params[0]; s=GC->params[1];}

#define render_screen_type(t) \
	{ GC->opcode=RENDER_SCREEN_TYPE; do_GFX(); t=GC->params[0]; }
#define get_screen_mode(m) \
	{ GC->opcode=GET_SCREEN_MODE; do_GFX(); m=GC->params[0]; }
#define set_screen_mode(m,s) \
	{ GC->params[0]=m; GC->opcode=SET_SCREEN_MODE; do_GFX(); \
	s=GC->params[0]; }

#define get_serial_number(s) \
	{ GC->opcode=GET_SERIAL_NUMBER; do_GFX(); fnstrcpy(s,GC->s1.string1); }

/* IPAS KXP Interface Function Codes */

#define KXP_GET_NODE_COUNT	0x1000	/* (COUNT) */
#define KXP_GET_CUR_FRAME	0x1005	/* (CUR FRAME) */
#define KXP_SET_CUR_FRAME	0x1007	/* CUR FRAME,(STATUS) */	/* 3DS 4.0 */
#define KXP_GET_NODE_NAME	0x1010	/* NODE, (NODENAME), (TYPE) */
#define KXP_GET_NODE_NUMBER	0x1020	/* NODENAME, (NODE), (TYPE) */
#define KXP_GET_NUM_CHILDREN	0x1030	/* NODE, (COUNT) */
#define KXP_GET_CHILD		0x1040	/* NODE, CHILDNUM, (CHILDINDEX) */
#define KXP_GET_PARENT		0x1045	/* NODE,(PARENTINDEX) */ 	/* 3DS 3.01 */
#define KXP_SET_PARENT		0x1046	/* NODE,PARENTINDEX,FIXUP,(STATUS) */	/* 3DS 4.0 */
#define KXP_GET_KEY_COUNT	0x1050	/* NODE, TRACK, (COUNT), (STATUS) */
#define KXP_FIND_PREV_KEY	0x1060	/* NODE, TRACK, TIME, (INDEX), (STATUS) */
#define KXP_FIND_NEXT_KEY	0x1070	/* NODE, TRACK, TIME, (INDEX), (STATUS) */
#define KXP_FIND_KEY_AT_TIME	0x1080	/* NODE, TRACK, TIME, (INDEX), (STATUS) */
#define KXP_GET_KEY_BY_NUMBER	0x1090	/* NODE, TRACK, INDEX, (KEY), (STATUS) */
#define KXP_INTERPOLATE_KEY	0x10A0	/* NODE, TRACK, TIME, (KEY), (STATUS) */
#define KXP_FRAC_INTERPOLATE_KEY	0x10A5	/* NODE,TRACK,TIME,FRAC(float),(KEY),(STATUS) */	/* 3DS 3.01 */
#define KXP_CREATE_KEY		0x10B0	/* NODE, TRACK, KEY, (NUMBER), (STATUS) */
#define KXP_ALLOCATE_KEYS	0x10B5	/* NODE,TRACK,NUMBER,(STATUS) */	/* 3DS 3.01 */
#define KXP_DELETE_KEY		0x10C0	/* NODE, TRACK, INDEX, (STATUS) */
#define KXP_DELETE_TRACK	0x10E0	/* NODE, TRACK, (STATUS) */
#define KXP_DELETE_ALL_TRACKS	0x10F0	/* NODE, (STATUS) */
#define KXP_GET_PIVOT		0x1100	/* NODE, (PIVOT), (STATUS) */
#define KXP_SET_PIVOT		0x1110	/* NODE, PIVOT, (STATUS) */
#define KXP_PREP_NODE		0x1120	/* NODE, (STATUS) */
#define KXP_NUM_FRAMES		0x1130	/* (FRAMES) */
#define KXP_SET_NUM_FRAMES	0x1132	/* FRAMES,(STATUS) */	/* 3DS 4.0 */
#define KXP_GET_SEGMENT		0x1135	/* (SEGSTART,SEGEND) */			/* 3DS 4.0 */
#define KXP_SET_SEGMENT		0x1136	/* SEGSTART,SEGEND,(STATUS) */	/* 3DS 4.0 */
#define KXP_FINISHED		0x1140
#define KXP_GET_TRACK_FLAGS	0x1150	/* NODE, TRACK, (FLAGS), (STATUS) */
#define KXP_SET_TRACK_FLAGS	0x1160	/* NODE, TRACK, FLAGS, (STATUS) */
#define KXP_GET_BBOX		0x1170	/* NODE, (minx, miny, minz, maxx, maxy, maxz)(STATUS) */
#define KXP_GET_XFORM		0x1180	/* NODE, tm, (MATRIX) */
#define KXP_GET_VLIST		0x1190	/* NODE, vstart, vcount, frame (verts) (STATUS) */
#define KXP_GET_FLIST		0x11A0	/* NODE, vstart, vcount (faces) (STATUS) */
#define KXP_GET_FVCOUNT		0x11B0	/* NODE, (nfaces, nverts) (STATUS) */
#define KXP_CHOOSE_NODE		0x11C0	/* TITLE, FLAGS, (node) */
#define KXP_UPDATE_PATHS	0x11D0	/* NODE */
#define KXP_GET_APPDATA		0x11E0	/* NODE (nbytes, farptr) */
#define KXP_SET_APPDATA		0x11F0	/* NODE, nbytes, farptr */
#define KXP_GET_APPDATA_CHUNK	  	0x11F4	/* NODE, idstring ( farptr)(STATUS) */ 	/* 3DS 3.01 */
#define KXP_SET_APPDATA_CHUNK	  	0x11F6	/* NODE  farptr  (STATUS) */	/* 3DS 3.01 */
#define KXP_DELETE_APPDATA_CHUNK	0x11F8	/* NODE, idstring (STATUS) */	/* 3DS 3.01 */
#define KXP_INIT_KEYFRAMER		0x1200

/* Handy KXP macros */

extern KXPInfo *KI;
extern KXPComm _far *KC;

#define kxp_get_node_count(c) \
	{ KC->opcode=KXP_GET_NODE_COUNT; do_KXP(); c=KC->params[0]; }
#define kxp_get_node_name(i,n,t) \
	{ KC->params[0]=i; KC->opcode=KXP_GET_NODE_NAME; do_KXP(); \
	fnstrcpy(n,KC->s1.string1); t=KC->params[0]; }
#define kxp_get_node_number(n,i,t) \
	{ nfstrcpy(KC->s1.string1,n); KC->opcode=KXP_GET_NODE_NUMBER; do_KXP(); \
	i=KC->params[0]; t=KC->params[1]; }

/* When 3D Studio is asked for the number of the children and there are none
 * it incorrectly returns an error message instead of setting the number to 0.
 * To correct for this we do our own error checking here (the only error that
 * can occur is if the node number is greater than the total number of nodes).
 * Thus once we determine no error can occur, if 3DS reports an error we set
 * the number to 0 instead.
 */
#define kxp_get_num_children(i,c) \
	{ c=0; \
          if (i>=0) { kxp_get_node_count(c); if (i>=c) c=-1; else c=0;} \
	  if (! c) { KC->params[0]=i; KC->opcode=KXP_GET_NUM_CHILDREN; \
	  do_KXP(); c=KC->params[0]>=0 ? KC->params[0] : 0; } }
#define kxp_get_child(i,c,ci) \
	{ KC->params[0]=i; KC->params[1]=c; KC->opcode=KXP_GET_CHILD; do_KXP(); \
	ci=KC->params[0]; }
#define kxp_get_parent(nodenum,par) \
	{ KC->params[0]=nodenum; KC->opcode=KXP_GET_PARENT; do_KXP(); \
	par=KC->params[0]; }
#define kxp_set_parent(nodenum,par,fix,s) \
	{ KC->params[0]=nodenum; KC->params[1]=par; KC->params[2]=fix; \
	KC->opcode=KXP_SET_PARENT; do_KXP(); s=KC->params[0]; }
#define kxp_get_key_count(i,t,c,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->opcode=KXP_GET_KEY_COUNT; do_KXP(); \
	c=KC->params[0]; s=KC->params[1]; }
#define kxp_find_prev_key(i,t,f,ix,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->params[2]=f; \
	KC->opcode=KXP_FIND_PREV_KEY; do_KXP(); ix=KC->params[0]; s=KC->params[1]; }
#define kxp_find_next_key(i,t,f,ix,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->params[2]=f; \
	KC->opcode=KXP_FIND_NEXT_KEY; do_KXP(); ix=KC->params[0]; \
	s=KC->params[1]; }
#define kxp_find_key_at_time(i,t,f,ix,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->params[2]=f; \
	KC->opcode=KXP_FIND_KEY_AT_TIME; do_KXP(); ix=KC->params[0]; \
	s=KC->params[1]; }
#define kxp_get_key_by_number(i,t,ix,k,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->params[2]=ix; \
	KC->ptr1=(void _far *)k; KC->opcode=KXP_GET_KEY_BY_NUMBER; do_KXP(); \
	s=KC->params[0]; }
#define kxp_interpolate_key(i,t,f,r,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->params[2]=f; \
	KC->ptr1=(void _far *)r; KC->opcode=KXP_INTERPOLATE_KEY; do_KXP(); \
	s=KC->params[0]; }
#define kxp_frac_interpolate_key(i,t,frame,frac,r,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->params[2]=frame; \
	KC->s1.f = frac; KC->ptr1=(void _far *)r; KC->opcode=KXP_FRAC_INTERPOLATE_KEY; do_KXP(); \
	s=KC->params[0]; }
#define kxp_create_key(i,t,k,n,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->ptr1=(void _far *)k; \
	KC->opcode=KXP_CREATE_KEY; do_KXP(); \
	n=KC->params[0]; s=KC->params[1]; }
#define kxp_allocate_keys(i,t,n,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->params[2]=n; \
	KC->opcode=KXP_ALLOCATE_KEYS; do_KXP(); \
	s=KC->params[1]; }
#define kxp_delete_key(i,t,n,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->params[2]=n; \
	KC->opcode=KXP_DELETE_KEY; do_KXP(); s=KC->params[0]; }
#define kxp_delete_track(i,t,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->opcode=KXP_DELETE_TRACK; do_KXP(); \
	s=KC->params[0]; }
#define kxp_delete_all_tracks(i,s) \
	{ KC->params[0]=i; KC->opcode=KXP_DELETE_ALL_TRACKS; do_KXP(); \
	s=KC->params[0]; }
#define kxp_get_pivot(i,p,s) \
	{ KC->params[0]=i; KC->opcode=KXP_GET_PIVOT; do_KXP(); p=KC->pivot; \
	s=KC->params[0]; }
#define kxp_set_pivot(i,p,s) \
	{ KC->params[0]=i; KC->pivot=p; KC->opcode=KXP_SET_PIVOT; do_KXP(); \
	s=KC->params[0]; }
#define kxp_get_track_flags(i,t,f,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->opcode=KXP_GET_TRACK_FLAGS; do_KXP(); \
	f = KC->params[0]; s=KC->params[1]; }
#define kxp_set_track_flags(i,t,f,s) \
	{ KC->params[0]=i; KC->params[1]=t; KC->params[2]=f; KC->opcode=KXP_SET_TRACK_FLAGS; do_KXP();\
	s=KC->params[0]; }
#define kxp_get_bound_box(i,time,absolute,boxmin,boxmax,s) \
	{ KC->params[0]=i; KC->params[1]=time; KC->params[2]=absolute; \
	KC->opcode=KXP_GET_BBOX; do_KXP(); \
	boxmin[0] = KC->s1.box.mins[0]; boxmin[1] = KC->s1.box.mins[1]; boxmin[2] = KC->s1.box.mins[2];\
	boxmax[0] = KC->s1.box.maxs[0]; boxmax[1] = KC->s1.box.maxs[1]; boxmax[2] = KC->s1.box.maxs[2];\
	s=KC->params[0]; }
#define kxp_prep_node(i,s) \
	{ KC->params[0]=i; KC->opcode=KXP_PREP_NODE; do_KXP(); s=KC->params[0]; }
#define kxp_num_frames(c) \
        { KC->opcode=KXP_NUM_FRAMES; do_KXP(); c=KC->params[0]; }
#define kxp_set_num_frames(n,s) \
	{ KC->params[0]=n; KC->opcode=KXP_SET_NUM_FRAMES; do_KXP(); \
	s=KC->params[0]; }
#define kxp_get_segment(s,e) \
	{ KC->opcode=KXP_GET_SEGMENT; do_KXP(); s=KC->params[0]; e=KC->params[1]; }
#define kxp_set_segment(s,e,status) \
	{ KC->params[0]=s; KC->params[1]=e; KC->opcode=KXP_SET_SEGMENT; do_KXP(); \
	status=KC->params[0]; }
#define kxp_get_cur_frame(f) \
	{ KC->opcode=KXP_GET_CUR_FRAME; do_KXP(); f=KC->params[0]; }
#define kxp_set_cur_frame(f,s) \
	{ KC->params[0]=f; KC->opcode=KXP_SET_CUR_FRAME; do_KXP(); \
	s=KC->params[0]; }
#define kxp_finished() \
	{ KC->opcode=KXP_FINISHED; do_KXP(); }
#define kxp_get_xform(inode,time,tmat) \
	{ KC->params[0]=inode; KC->params[1]=time;  \
	KC->opcode=KXP_GET_XFORM; do_KXP(); \
	far_to_near(tmat,KC->s1.tm,12*sizeof(float)); }
#define kxp_get_fvcount(i,nfaces,nverts) \
	{ KC->params[0]=i; KC->opcode=KXP_GET_FVCOUNT; do_KXP(); \
	nfaces =  KC->params[0]; nverts =  KC->params[1];}

/* 3D Studio always returns with KC->params[0] equal to -1 for kxp_get_verts
 * so we have to determine if there was an error some other way.
 */
#define kxp_get_verts(inode,start,count,time,absolute,verts,status) \
	{ KC->params[0]=inode; KC->params[1]=start;  \
	KC->params[2]=count; KC->params[3]=time; KC->params[4]=absolute; \
	KC->s1.vdata = verts; KC->opcode=KXP_GET_VLIST; \
	set_to_NaN (&verts[0].x); do_KXP(); \
	status=is_NaN(verts[0].x) ? -1 : 1; }

/* 3D Studio always returns with KC->params[0] equal to -1 for kxp_get_faces
 * so we have to determine if there was an error some other way.
 */
#define kxp_get_faces(inode,start,count,time,faces,status) \
	{ KC->params[0]=inode; KC->params[1]=start;  \
	KC->params[2]=count; KC->s1.fdata = faces;	\
	KC->opcode=KXP_GET_FLIST; \
	faces[0].flags = 255; do_KXP(); \
	status = faces[0].flags == 255 ? -1 : 1; } 
#define kxp_choose_node(title,flags,node) \
	{ nfstrcpy(KC->s1.string1,title); KC->params[0]=flags; \
	KC->opcode=KXP_CHOOSE_NODE; do_KXP(); node=KC->params[0]; } 
#define kxp_update_paths(node,s) \
	{ KC->params[0]=node; KC->opcode=KXP_UPDATE_PATHS; do_KXP(); \
	s=KC->params[0]; } 
#define kxp_get_appdata(node,ptr,nbytes) \
	{ KC->params[0]=node; KC->opcode=KXP_GET_APPDATA; do_KXP(); \
	ptr=KC->ptr1; nbytes = KC->params[0]; } 
#define kxp_set_appdata(node,ptr,nbytes,status) \
	{ KC->params[0]=node; KC->opcode=KXP_SET_APPDATA;  \
	KC->ptr1=(void _far *)ptr; KC->params[1] = nbytes; \
	do_KXP(); status = KC->params[0]; } 
#define kxp_get_appdata_chunk(node,name,ptr) \
	{ KC->params[0]=node; KC->opcode=KXP_GET_APPDATA_CHUNK; \
	nfstrcpy(KC->s1.string1,name); do_KXP(); ptr=KC->ptr1; } 
#define kxp_set_appdata_chunk(node,ptr,status) \
	{ KC->params[0]=node; KC->opcode=KXP_SET_APPDATA_CHUNK; \
	KC->ptr1 = ptr; do_KXP();	status = KC->params[0]; } 
#define kxp_delete_appdata_chunk(node,name) \
	{ KC->params[0]=node; KC->opcode=KXP_DELETE_APPDATA_CHUNK; \
	nfstrcpy(KC->s1.string1,name); do_KXP(); status = KC->params[0]; } 
#define kxp_init_keyframer() \
	{ KC->opcode=KXP_INIT_KEYFRAMER; do_KXP(); }
 
/* Node type values */

#define OBJECT_NODE 1
#define CAMERA_NODE 2
#define TARGET_NODE 3
#define LIGHT_NODE 4
#define SPOTLIGHT_NODE 5
#define AMBIENT_NODE 6
#define DUMMY_NODE 7

/* Track Type Values */

#define KT_POSITION     0
#define KT_SCALE        1
#define KT_ROTATE       2
#define KT_MORPH        3
#define KT_HIDE         4
#define KT_ROLL         5
#define KT_FOV          6
#define KT_COLOR        7
#define KT_HOTSPOT      8
#define KT_FALLOFF      9

/* Track Flag Values */
#define ANIM_CYCLIC	1
#define ANIM_LOOP 	2
#define ANIM_NEGWRAP	4
#define X_LOCKED	8
#define Y_LOCKED	16
#define Z_LOCKED	32
#define ALL_LOCKED 	(X_LOCKED|Y_LOCKED|Z_LOCKED)
#define TRACK_ATKEY	64
#define NO_LNK_X	128
#define NO_LNK_Y	256
#define NO_LNK_Z	512
#define LAST_AXIS_MASK	1024

/* Key data structures */

#define KEYHEAD  \
        int time;  \
        float tens, cont, bias; \
        float easeTo, easeFrom

/* The additional nu keys are provided to conveniently make all the Key structures
   the same size in memory.  */
	
typedef struct
{
   KEYHEAD;
      float e[4]; /* enough to be bigger than the biggest key */
} KXPKey;

typedef struct
{
   KEYHEAD;
   float x;
   float y;
   float z;
   float nu; /* not used */
} KXPScalarKey;

typedef struct
{
   KEYHEAD;
   float x, y, z;
   float nu; /* not used */
} KXPVecKey;    

typedef struct
{
   KEYHEAD;
   float angle;    /* angle of rotation in radians (always >0) */
   float axis[3];  /* axis of rotation (unit vector) */
} KXPRotKey;

typedef struct
{
   KEYHEAD;
   char object[11];
   char nu[5]; /* not used */
} KXPMorphKey;  

typedef struct
{
   KEYHEAD;
   float nu[4];  /* not used */
} KXPHideKey;

typedef struct
{
   KEYHEAD;
   float r, g, b;
   float nu; /* not used */
} KXPColorKey;

/* Interpolated key structures */

typedef struct
{
   float a, b, c;
   float nu;  /* not used */
} KXPVecData;

typedef struct
{
   float angle, axis[3];
} KXPRotData;

typedef struct
{
   union
   {
      int i;
      float f;
      KXPVecData v;
      KXPRotData r;
   } u;
} KXPInterp;

/* KXP_CHOOSE_NODE node-selection flags */

#define CN_OBJECT 1
#define CN_CAMERA 2
#define CN_TARGET 4
#define CN_LIGHT 8
#define CN_SPOTLIGHT 16
#define CN_AMBIENT 32
#define CN_DUMMY 64

/* IPAS 3 PXP Direct-Call Extensions */

#define PXP_GET_ITEM_COUNT              0x1000  /* (COUNT) */
#define PXP_GET_ITEM                    0x1010  /* INDEX, (DATA, STATUS) */
#define PXP_DELETE_ITEM                 0x1020  /* NAME, (STATUS) */
#define PXP_CREATE_ITEM                 0x1030  /* DATA, (STATUS) */
#define PXP_CHANGE_ITEM                 0x1032  /* DATA, (STATUS) */
#define PXP_MAKE_OB_BOX					0x1033	/* NAME,(STATUS) */	/* 3DS 3.01 */
#define PXP_UPDATE_OB_BOX				0x1034	/* NAME,(STATUS) */	/* 3DS 3.01 */

#define PXP_SET_VERTS                   0x1040  /* NAME, COUNT, TVCOUNT, (STATUS) */
#define PXP_SET_FACES                   0x1050  /* NAME, COUNT, (STATUS) */
#define PXP_GET_VERTS                   0x1060  /* NAME, START, COUNT, (VLIST, STATUS) */
#define PXP_GET_FACES                   0x1070  /* NAME, START, COUNT, (FLIST, STATUS) */
#define PXP_GET_FACES2					0x1071	/* NAME, START, COUNT, (FLIST, STATUS) */ /* IPAS 3.01 */
#define PXP_PUT_VERTS                   0x1080  /* NAME, START, COUNT, (VLIST, STATUS) */
#define PXP_PUT_FACES                   0x1090  /* NAME, START, COUNT, (FLIST, STATUS) */
#define PXP_PUT_FACES2					0x1091	/* NAME, START, COUNT, (FLIST, STATUS) */ /* IPAS 3.01 */

#define PXP_GET_EXCLUDE_COUNT   0x1100  /* NAME, (COUNT) */
#define PXP_GET_EXCLUDED                0x1110  /* NAME, INDEX, (ENAME), (STATUS) */
#define PXP_CLEAR_EXCLUDED              0x1120  /* NAME, (STATUS) */
#define PXP_ADD_EXCLUDED                0x1130  /* NAME, (ENAME), (STATUS) */

#define PXP_GET_APPDATA                 0x1200  /* NAME, (NBYTES, FARPTR) */
#define PXP_SET_APPDATA                 0x1210  /* NAME, NBYTES, FARPTR, (STATUS) */
#define PXP_GET_APPDATA_CHUNK	  		0x1230	/* NAME,IDSTRING ( farptr)(STATUS) */	/* 3DS 3.01 */
#define PXP_SET_APPDATA_CHUNK	  		0x1240	/* NAME, FARPTR  (STATUS) */	/* 3DS 3.01 */
#define PXP_DELETE_APPDATA_CHUNK		0x1250	/* NAME, IDSTRING (STATUS) */	/* 3DS 3.01 */

#define PXP_GET_AXPINFO					0x1280	/* NAME,(SWITCH,AXP,NBYTES,FARPTR,STATUS) */
#define PXP_SET_AXPINFO					0x1290	/* NAME,SWITCH,AXP,NBYTES,FARPTR,(STATUS) */

#define PXP_ERASE_ITEM                  0x1300  /* NAME */
#define PXP_DRAW_ITEM                   0x1310  /* NAME */
#define PXP_VIEW_REDRAW                 0x1320  /* NAME */

#define PXP_MATERIAL					0x1400	/* MATERIALNAME, (INDEX) */	/* 3DS 3.01 */
#define PXP_LOAD_MLI					0x1410	/* FILENAME, (STATUS) */	/* 3DS 3.01 */
#define PXP_MATERIAL_COUNT				0x1420	/* (COUNT) */	/* 3DS 3.01 */
#define PXP_GET_MATERIAL				0x1430	/* INDEX, PTR, (STATUS) */ /* 3DS 3.01 */

/* Handy PXP macros */

extern PXPInfo *PXP_I;
extern PXPComm _far *PC;

#define pxp_get_item_count(count) \
        { PC->opcode=PXP_GET_ITEM_COUNT; do_PXP(); count=PC->params[0]; }
#define pxp_get_item(index, data, status) \
        { PC->params[0]=index; PC->ptr1=(void _far *)data; \
        PC->opcode=PXP_GET_ITEM; do_PXP(); status=PC->params[0]; }
#define pxp_delete_item(name, status) \
        { nfstrcpy(PC->s1.string1, name); PC->opcode=PXP_DELETE_ITEM; do_PXP(); \
        status=PC->params[0]; }
#define pxp_create_item(data, status) \
        { PC->ptr1=(void _far *)data; PC->opcode=PXP_CREATE_ITEM; do_PXP(); \
        status=PC->params[0]; }
#define pxp_change_item(data, status) \
        { PC->ptr1=(void _far *)data; PC->opcode=PXP_CHANGE_ITEM; do_PXP(); \
        status=PC->params[0]; }
#define pxp_make_ob_box(name,status) \
		{ nfstrcpy(PC->s1.string1,name); PC->opcode=PXP_MAKE_OB_BOX; do_PXP(); \
		status=PC->params[0]; }
#define pxp_update_ob_box(name,status) \
		{ nfstrcpy(PC->s1.string1,name); PC->opcode=PXP_UPDATE_OB_BOX; do_PXP(); \
		status=PC->params[0]; }

#define pxp_set_verts(name, count, tvcount, status) \
        { nfstrcpy(PC->s1.string1, name); PC->params[0]=count; \
        PC->params[1]=tvcount; PC->opcode=PXP_SET_VERTS; do_PXP(); \
        status=PC->params[0]; }
#define pxp_set_faces(name, count, status) \
        { nfstrcpy(PC->s1.string1, name); PC->params[0]=count; \
        PC->opcode=PXP_SET_FACES; do_PXP(); status=PC->params[0]; }
#define pxp_get_verts(name, start, count, ptr, status) \
        { nfstrcpy(PC->s1.string1, name); PC->params[0]=start; PC->params[1]=count; \
        PC->ptr1=(void _far *)ptr; PC->opcode=PXP_GET_VERTS; do_PXP(); \
        status=PC->params[0]; }
#define pxp_get_faces(name, start, count, ptr, status) \
        { nfstrcpy(PC->s1.string1, name); PC->params[0]=start; PC->params[1]=count; \
        PC->ptr1=(void _far *)ptr; PC->opcode=PXP_GET_FACES; do_PXP(); \
        status=PC->params[0]; }
#define pxp_get_faces2(name,start,count,ptr,status) \
		{ nfstrcpy(PC->s1.string1,name); PC->params[0]=start; PC->params[1]=count; \
		PC->ptr1=(void _far *)ptr; PC->opcode=PXP_GET_FACES2; do_PXP(); \
		status=PC->params[0]; }
#define pxp_put_verts(name, start, count, ptr, status) \
        { nfstrcpy(PC->s1.string1, name); PC->params[0]=start; PC->params[1]=count; \
        PC->ptr1=(void _far *)ptr; PC->opcode=PXP_PUT_VERTS; do_PXP(); \
        status=PC->params[0]; }
#define pxp_put_faces(name, start, count, ptr, status) \
        { nfstrcpy(PC->s1.string1, name); PC->params[0]=start; PC->params[1]=count; \
        PC->ptr1=(void _far *)ptr; PC->opcode=PXP_PUT_FACES; do_PXP(); \
        status=PC->params[0]; }
#define pxp_put_faces2(name,start,count,ptr,status) \
		{ nfstrcpy(PC->s1.string1,name); PC->params[0]=start; PC->params[1]=count; \
		PC->ptr1=(void _far *)ptr; PC->opcode=PXP_PUT_FACES2; do_PXP(); \
		status=PC->params[0]; }

#define pxp_get_exclude_count(name, count) \
        { nfstrcpy(PC->s1.string1, name); PC->opcode=PXP_GET_EXCLUDE_COUNT; \
        do_PXP(); count=PC->params[0]; }
#define pxp_get_excluded(name, index, ename, status) \
        { nfstrcpy(PC->s1.string1, name); PC->params[0]=index; \
        PC->opcode=PXP_GET_EXCLUDED; do_PXP(); fnstrcpy(ename, PC->s1.string1); \
        status=PC->params[0]; }
#define pxp_clear_excluded(name, status) \
        { nfstrcpy(PC->s1.string1, name); PC->opcode=PXP_CLEAR_EXCLUDED; do_PXP(); \
        status=PC->params[0]; }
#define pxp_add_excluded(name, ename, status) \
        { nfstrcpy(PC->s1.string1, name); nfstrcpy(PC->s2.string2, ename); \
        PC->opcode=PXP_ADD_EXCLUDED; do_PXP(); status=PC->params[0]; }

#define pxp_get_appdata(name, nbytes, ptr) \
        { nfstrcpy(PC->s1.string1, name); PC->opcode=PXP_GET_APPDATA; do_PXP(); \
        nbytes=PC->params[0]; ptr=PC->ptr1; }
#define pxp_set_appdata(name, nbytes, ptr, status) \
        { nfstrcpy(PC->s1.string1, name); PC->params[0]=nbytes; \
        PC->ptr1=(void _far *)ptr; PC->opcode=PXP_SET_APPDATA; do_PXP(); \
        status=PC->params[0]; }
#define pxp_get_appdata_chunk(name,idstring,chunkptr) \
	{ nfstrcpy(PC->s1.string1,name); nfstrcpy(PC->s2.string2,idstring); \
	PC->opcode=PXP_GET_APPDATA_CHUNK; do_PXP(); chunkptr=PC->ptr1; }
#define pxp_set_appdata_chunk(name,chunkptr,status) \
	{ nfstrcpy(PC->s1.string1,name); PC->ptr1 = chunkptr; \
	PC->opcode=PXP_SET_APPDATA_CHUNK; do_PXP(); status = PC->params[0]; }
#define pxp_delete_appdata_chunk(name,idstring,status) \
	{ nfstrcpy(PC->s1.string1,name); nfstrcpy(PC->s2.string2,idstring); \
	PC->opcode=PXP_DELETE_APPDATA_CHUNK; do_PXP(); status = PC->params[0]; }
#define pxp_get_axpinfo(name,sw,axp,nbytes,ptr,status) \
	{ nfstrcpy(PC->s1.string1,name); PC->opcode=PXP_GET_AXPINFO; do_PXP(); \
	status=PC->params[0]; sw=PC->params[1]; nbytes=PC->params[2]; \
	fnstrcpy(axp,PC->s1.string1); ptr=PC->ptr1; }
#define pxp_set_axpinfo(name,sw,axp,nbytes,ptr,status) \
	{ nfstrcpy(PC->s1.string1,name); PC->params[1]=sw; PC->params[2]=nbytes; \
	nfstrcpy(PC->s2.string2,axp); PC->ptr1=(void _far *)ptr; \
	PC->opcode=PXP_SET_AXPINFO; do_PXP(); status=PC->params[0]; }

#define pxp_erase_item(name) \
        { nfstrcpy(PC->s1.string1, name); PC->opcode=PXP_ERASE_ITEM; do_PXP(); }
#define pxp_draw_item(name) \
        { nfstrcpy(PC->s1.string1, name); PC->opcode=PXP_DRAW_ITEM; do_PXP(); }
#define pxp_view_redraw(name) \
        { nfstrcpy(PC->s1.string1, name); PC->opcode=PXP_VIEW_REDRAW; do_PXP(); }

#define pxp_material(name,status) \
		{ nfstrcpy(PC->s1.string1,name); PC->opcode=PXP_MATERIAL; do_PXP(); \
		status=PC->params[0]; }
#define pxp_load_mli(fname,status) \
		{ nfstrcpy(PC->s1.string1,fname); PC->opcode=PXP_LOAD_MLI; do_PXP(); \
		status=PC->params[0]; }
#define pxp_material_count(count) \
	{ PC->opcode=PXP_MATERIAL_COUNT; do_PXP(); count=PC->params[0]; }
#define pxp_get_material(index,ptr,status) \
	{ PC->params[0]=index; PC->ptr1=(void _far *)ptr; \
	PC->opcode=PXP_GET_MATERIAL; do_PXP(); status=PC->params[0]; }

/* Data structures for information on editor items */

typedef struct
{
   ulong flags;
   ushort verts, tverts, faces;
   float matrix[4][3];
   float mins[3], maxs[3];
   uchar color;
} ItemMesh;

/* Mesh item flags */

#define MESH_DONT_RCVSHADOW	0x08000000L	/* Dont receive shadows */
#define MESH_FROZEN		0x00080000L	/* Object is frozen */
#define MESH_SELECTED		0x00001000L	/* Object selected */
#define MESH_MATTE		0x00000800L	/* Object is a matte object */
#define MESH_DOESNT_CAST	0x00000400L	/* Object doesn't cast shadows */
#define MESH_FAST		0x00000200L	/* Object is fastdraw */
#define MESH_HIDDEN		0x00000001L	/* Object hidden */

typedef struct
{
   FColor color;
} ItemAmbient;

typedef struct
{
   ushort flags;
   float x, y, z;
   float tx, ty, tz;
   float hotsize, fallsize;
   float bank, aspect;
   float in_range, out_range;
   float bias, shadfilter;
   int shadsize;
   float ray_bias;
   FColor color;
   float mult;
   char imgfile[13];
} ItemLight;

/* Light item flags */

#define LIGHT_ON        0x0001  /* Light turned on */
#define LIGHT_SHAD      0x0002  /* Light casts shadows */
#define LIGHT_LOCAL		0x0004	/* Light uses local shadow controls */
#define LIGHT_CONE      0x0008  /* Light's cone visible */
#define LIGHT_RECT      0x0010  /* Light is rectangular */
#define LIGHT_PROJ      0x0020  /* Light is a projector */
#define LIGHT_OVER      0x0040  /* Light overshoots */
#define LIGHT_ATTEN     0x0080  /* Light is attenuated */
#define LIGHT_RAYTR     0x0100  /* Light is raytraced */

typedef struct
{
   ushort flags;
   float x, y, z;
   float tx, ty, tz;
   float focal, bank;
   float nearplane, farplane;
} ItemCamera;

/* Camera flags */

#define CAMERA_CONE 0x0001      /* Camera's cone visible */

typedef struct
{
   char name[11];
   int type;
   union
   {
      ItemMesh m;
      ItemAmbient a;
      ItemLight l;
      ItemCamera c;
   } item;
} ItemData;

/* Item types */

#define PXPMESH 0
#define PXPAMBIENT 1
#define PXPLIGHT 2
#define PXPCAMERA 3

/* Extended vertex data (Direct-call PXP interface only!) */

typedef struct
{
   float x, y, z;                    /* XYZ coordinates */
   float u, v;                              /* Texture coords, if used */
   ushort flags;   /* See flags below */
} XVData;

/* Extended vertex flags bit assignments */

#define XV_SELECTED 1
#define XV_SEL_A 2
#define XV_SEL_B 4
#define XV_SEL_C 8

/* Extended face data (Direct-call PXP interface only!) */

typedef struct
{
   ushort a, b, c;   /* Vertex numbers for triangular face */
   ulong sm_group; /* Smoothing group bits (32) */
   uchar flags;    /* See below for meanings */
   uchar material; /* 0-255 */
} XFData;

typedef struct
{
unsigned short a,b,c;	/* Vertex numbers for triangular face */
unsigned long sm_group;	/* Smoothing group bits (32) */
unsigned short flags;	/* See below for meanings */
unsigned char material;	/* 0-255 */
} XFData2;

/* Extended face flags bit assignments */

#define XF_ABLINE 1
#define XF_BCLINE 2
#define XF_CALINE 4
#define XF_UWRAP 8
#define XF_VWRAP 16
#define XF_SELECTED 32
#define XF_SEL_A 64
#define XF_SEL_B 128
#define XF_SEL_C 256

#define XDATA_SECTION 0x8000 /* ID for entire section of appdata */
#define XDATA_ENTRY   0x8001 /* ID for chunks in appdata chunk */
#define XDATA_APPNAME 0x8002 /* ID for required first chunk in XDATA_ENTRY chunks */
#define XDATA_STRING  0x8003 /* ID for null terminated strings */
#define XDATA_FLOAT   0x8004 /* ID for floats */
#define XDATA_DOUBLE  0x8005 /* ID for doubles */
#define XDATA_SHORT   0x8006 /* ID for shorts */
#define XDATA_LONG    0x8007 /* ID for longs */
#define XDATA_VOID    0x8008 /* ID for voids */
#define XDATA_GROUP   0x8009 /* ID for collections of the above 5 types */


/* Material Data Structure (mapping omitted) */

typedef struct
{
   char name[17];	/* Material's 16-char name */
   BXPColor amb;	/* 0-255 triplets */
   BXPColor diff;	/* 0-255 triplets */
   BXPColor spec;	/* 0-255 triplets */
   short transparency;	/* 0-100 */
   short shading;	/* 0=WIRE 1=FLAT 2=GOURAUD 3=PHONG 4=METAL */
   unsigned long flags; /* Material flags */
   unsigned short use;	/* Use flags */
   short shininess;	/* 0-100 */
   short shin2pct;	/* 0-100 */
   short shin3pct;	/* 0-100 - Not Used  */
   short xpfall;	/* 0-100 */
   short refblur;	/* 0-100 */
   short selfipct;	/* 0-100 */
   float wiresize;	/* size of wire frame */
} MtlData;

/* Material flags field bit definitions	*/

#define MTL_TWOSIDE	(1<<0)	/* Material seen from both sides */
#define MTL_SELF	(1<<1)	/* Material self-illuminated */
#define MTL_DECAL	(1<<2)	/* Material maps act as decals (transparent color) */
#define MTL_ADDITIVE	(1<<3)	/* Material uses additive transparency */
#define MTL_WIRE	(1<<4)	/* Material renders as wire frame */
#define MTL_NEEDUV	(1<<5)	/* Material has some UV type maps */
#define MTL_NEED3D	(1<<6)	/* Material needs 3D coords for SXP */
#define MTL_XPFALLIN	(1<<7)	/* Transparency fall-off to inside */
#define MTL_MATTE	(1<<8)	/* Material used as a matte */
#define MTL_CUBICMAP	(1<<9)	/* Reflection map is cubic */
#define MTL_XPFALL	(1<<10)	/* Do Transparency fall-off */
#define MTL_SUPERSMP	(1<<11)	/* Super sample material */
#define MTL_FACEMAP	(1<<12)	/* Face-texture-coords */
#define MTL_PHONGSOFT	(1<<13)	/* Soften phong hilites */
#define MTL_WIREABS	(1<<14)	/* Absolute wire size */
#define MTL_REFBLUR	(1<<15)	/* blurred reflection map */

/* Material use field bit definitions	*/

#define MATUSE_XPFALL (1<<0)
#define MATUSE_REFBLUR (1<<1)

#ifdef __cplusplus
} /* close the c linkage specifier */
#endif

#endif /* EXPRTNS */
