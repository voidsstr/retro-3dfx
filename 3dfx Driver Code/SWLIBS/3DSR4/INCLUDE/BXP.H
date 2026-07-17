/******************************************************************************
  
  Interface for Autodesk 3D Studio 3.0 .BXP Procedures

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


#ifndef BXP_H
#define BXP_H

#include <exprtn.h>

#ifdef __cplusplus
extern "C" {
#endif   

/*----------------------------------------------------------
  Client process must supply these procedures: 
  see exprtn.h for definition of xpdata 
  ----------------------------------------------------------*/

/* ClientLoadStart/Finish: Image load process */

extern void ClientLoadStart(EXPbuf *buf);
extern void ClientLoadFinish(EXPbuf *buf);

/* ClientGetFrame: Animate bitmap to the indicated frame */

extern void ClientGetFrame(EXPbuf *buf);

/* ClientSaveImage: Save image file */

extern void ClientSaveImage(EXPbuf *buf);

/* ClientImageInfo: Return Image Information */

extern void ClientImageInfo(EXPbuf *buf);

/* ClientTerminate: Free any allocated memory */

extern void ClientTerminate(void);

extern char gp_buffer[];
void error(char *s1, char *s2, char *s3, char *s4, char *s5);
int studio_version(void);

#ifdef __cplusplus
} /* close the c linkage specifier */
#endif

#endif


