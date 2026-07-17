/******************************************************************************
  
  Interface for Autodesk 3D Studio 3.0 .SXP Procedures

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

#ifndef SXP_H
#define SXP_H

#include "exprtn.h"

#ifdef __cplusplus
extern "C" {
#endif   

/*----------------------------------------------------------
   Client process must supply these procedures:             
   see exprtn.h for definition of Sxpdata                  
-----------------------------------------------------------*/

/* ClientStartup: allocate data structures, compute lookup tables, etc */

extern void ClientStartup(void);

/* ClientTerminate: Free any allocated memory */

extern void ClientTerminate(void);

/* ClientGetState: return a pointer to client's "state" structure, and stuff
   size of the struct into "size" variable */

extern char *ClientGetState(int *size);

/* ClientResetState: reset client state to default values. */

extern void ClientResetState();

/* Client3DText: the function for texture mapping: based on the values in the
   fields in the Sxpdata record, the client computes an rgb value and stuffs
   it into d->col, and optionally stuffs a value into d->shininess. */

extern void Client3DText(Sxpdata *d);

/* Client3DOpac: the function for opacity mapping: based on the values in the
   fields in the Sxpdata record, the client computes an rgb value and stuffs
   it into d->col representing an opacity. */

extern void Client3DOpac(Sxpdata *d);

/* Client3DBump: the function for bump mapping.based on the values in the
   fields in the Sxpdata record, client computes a normal perterbation stuffs
   it into d->norm_pert */

extern void Client3DBump(Sxpdata *d);

/* ClientDialog: returns a pointer to the n'th DlgEntry struct in the
   user-defined DlgEntry array. This array describes a dialog window for
   setting parameters in the "state" struct.  (This dialog is brought up in
   3DStudio when the user clicks on the [S] "settings" button in the Material
   Editor) */

extern DlgEntry *ClientDialog(int n);

/* ClientSetStateVar: Assign a value (val) to the state variable
   corresponding to the DialogEntry whose id is "id" */

extern void ClientSetStateVar(int id, void *ptr);

/* ClientSetStateVar: Retrieve the value of the state variable
   corresponding to the Dialog Item whose id is "id" */

extern ulong ClientGetStateVar(int id);

/* ClientNewState: Called when a new state record has been received, 
   so client can compute derived tables, etc*/

void ClientNewState(void);

/* ClientVarSize: Return the length of the state variable in doublewords
   (multiples of four bytes).  All variables must be padded to a multiple of
   four bytes */

extern int ClientVarSize(int id);

/* ClientUsesInitDialog: Returns 1 if this process uses IPAS dialog handler */

extern int ClientUsesInitDialog(void);

/* ClientUsesCustomDialog: Returns 1 if this process uses custom dialog */

extern int ClientUsesCustomDialog(void);

/* ClientDoCustomDialog: Does custom dialog, returns 1 if OK'd, 0 if cancelled */

extern int ClientDoCustomDialog(void);

/* ClientFormatString: Formats a floating-point value to fit in editable */

extern void ClientFormatString(int id, float value, char *string);

/* General-Purpose buffer */

extern char gp_buffer[];
void error(char *s1, char *s2, char *s3, char *s4, char *s5);
int studio_version(void);

#ifdef __cplusplus
} /* close the c linkage specifier */
#endif

#endif


