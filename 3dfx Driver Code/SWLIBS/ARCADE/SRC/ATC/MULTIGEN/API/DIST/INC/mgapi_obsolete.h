/*******************************************************************************
 * 
 * $Header: mgapi_obsolete.h, 4, 10/11/00 7:32:07 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:07 PM$
 *
 ******************************************************************************/

/*============================================================================*\

   PROPRIETARY RIGHTS NOTICE: All rights reserved.  This software contains 
   proprietary information and trade secrets of MultiGen Inc. of San Jose, 
   California, and embodies substantial creative efforts as well as 
   confidential information, ideas, and expressions.  No part or all of this 
   software may be reproduced in any form, or by any means of electronic, 
   mechanical, or otherwise, without the written permission of MultiGen Inc.

   COPYRIGHT NOTICE: Copyright (C) 1986-1996 MultiGen Inc., San Jose, California.

\*============================================================================*/

/*----------------------------------------------------------------------------*/

#ifndef MGAPI_OBSOLETE_H_
#define MGAPI_OBSOLETE_H_

/*############################################################################*/
/*############################################################################*/
/*### 
/*###   The function definitions and other information defined in this header file
/*###   are unsupported and may be removed in future releases.
/*### 
/*############################################################################*/
/*############################################################################*/


#define fltVbead fltVertex 
#define fltLodFloat fltLod

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
extern APIFUNC(int) mgCheckVal ( mgrec* rec0, mgcode fcode );

extern APIFUNC(char) mgGetValC ( mgrec* rec );
extern APIFUNC(short) mgGetValS ( mgrec* rec );
extern APIFUNC(int) mgGetValI ( mgrec* rec );
extern APIFUNC(float) mgGetValF ( mgrec* rec );
extern APIFUNC(double) mgGetValD ( mgrec* rec );
extern APIFUNC(void) *mgGetValP ( mgrec* rec );
extern APIFUNC(unsigned char) mgGetValUC ( mgrec* rec );
extern APIFUNC(unsigned short) mgGetValUS ( mgrec* rec );
extern APIFUNC(unsigned int) mgGetValUI ( mgrec* rec );
extern APIFUNC(char) mgGetChar ( mgrec* rec );
extern APIFUNC(char) *mgGetText ( mgrec* rec );
extern APIFUNC(unsigned int) mgGetValBits ( mgrec* rec );
extern APIFUNC(mgbool) mgGetFlag ( mgrec* rec );

extern APIFUNC(mgbool) mgGetAttText ( mgrec* rec0, mgcode fcode, char** cptr );
extern APIFUNC(mgbool) mgGetAttBits ( mgrec* rec0, mgcode fcode, unsigned int* ival );
extern APIFUNC(mgbool) mgGetAttFlag ( mgrec* rec0, mgcode fcode, mgbool* flag );
extern APIFUNC(mgbool) mgGetAttValC ( mgrec* rec0, mgcode fcode, char* cval );
extern APIFUNC(mgbool) mgGetAttValS ( mgrec* rec0, mgcode fcode, short* sval );
extern APIFUNC(mgbool) mgGetAttValI ( mgrec* rec0, mgcode fcode, int* ival );
extern APIFUNC(mgbool) mgGetAttValF ( mgrec* rec0, mgcode fcode, float* fval );
extern APIFUNC(mgbool) mgGetAttValD ( mgrec* rec0, mgcode fcode, double* dval );
extern APIFUNC(mgbool) mgGetAttValP ( mgrec* rec0, mgcode fcode, void** ptr );
extern APIFUNC(mgbool) mgGetAttValUC ( mgrec* rec0, mgcode fcode, unsigned char* cval );
extern APIFUNC(mgbool) mgGetAttValUS ( mgrec* rec0, mgcode fcode, unsigned short* sval );
extern APIFUNC(mgbool) mgGetAttValUI ( mgrec* rec0, mgcode fcode, unsigned int* ival );
extern APIFUNC(mgrec) *mgGetAttAddress ( mgrec* rec0, mgcode fcode );	/* for SetBuf use only */
extern APIFUNC(mgrec) *mgGetAttNth ( mgrec* rec0, int nth );
/* return pointer to temporary record storage space containing the "Nth" record
   data from "rec0".  The temporary storage will be reused by the next call 
   to any mgGetAtt* function.  Do not free return pointer.  */


extern APIFUNC(char) *mgCode2Buf ( mgrec* rec0, mgcode fcode, attr_def** aptr_out );


extern APIFUNC(void) mgSetValC ( mgrec* rec, char cval );
extern APIFUNC(void) mgSetValS ( mgrec* rec, short sval );
extern APIFUNC(void) mgSetValI ( mgrec* rec, int ival );
extern APIFUNC(void) mgSetValF ( mgrec* rec, float fval );
extern APIFUNC(void) mgSetValD ( mgrec* rec, double dval );
extern APIFUNC(void) mgSetValP ( mgrec* rec, void* ptr );
extern APIFUNC(void) mgSetValIP ( mgrec* rec, int* ptr );
extern APIFUNC(void) mgSetValUC ( mgrec* rec, unsigned char cval );
extern APIFUNC(void) mgSetValUS ( mgrec* rec, unsigned short sval );
extern APIFUNC(void) mgSetValUI ( mgrec* rec, int ival );
extern APIFUNC(void) mgSetChar ( mgrec* rec, char* text );
extern APIFUNC(void) mgSetText ( mgrec* rec, char* text );
extern APIFUNC(void) mgSetValBits ( mgrec* rec, unsigned int ival );
extern APIFUNC(void) mgSetFlag ( mgrec* rec, mgbool flag );

extern APIFUNC(mgbool) mgSetAttRec ( mgrec* dest_rec, mgrec* src_rec );


extern APIFUNC(mgbool) mgSetAttFlag ( mgrec* rec0, mgcode fcode, mgbool flag );
/* output value is promoted to mgbool in the passed-in argument; 
	return value shows whether the operation was successful, eg. mgFALSE if not found */
extern APIFUNC(mgbool) mgSetAttValC ( mgrec* rec0, mgcode fcode, char cval );
extern APIFUNC(mgbool) mgSetAttValS ( mgrec* rec0, mgcode fcode, short sval );
extern APIFUNC(mgbool) mgSetAttValI ( mgrec* rec0, mgcode fcode, int ival );
extern APIFUNC(mgbool) mgSetAttValF ( mgrec* rec0, mgcode fcode, float fval );
extern APIFUNC(mgbool) mgSetAttValD ( mgrec* rec0, mgcode fcode, double dval );
extern APIFUNC(mgbool) mgSetAttValP ( mgrec* rec0, mgcode fcode, void* ptr );
extern APIFUNC(mgbool) mgSetAttValUC ( mgrec* rec0, mgcode fcode, unsigned char cval );
extern APIFUNC(mgbool) mgSetAttValUS ( mgrec* rec0, mgcode fcode, unsigned short sval );
extern APIFUNC(mgbool) mgSetAttValUI ( mgrec* rec0, mgcode fcode, unsigned int ival );
extern APIFUNC(mgbool) mgSetAttText ( mgrec* rec0, mgcode fcode, char* text );
extern APIFUNC(mgbool) mgSetAttBits ( mgrec* rec0, mgcode fcode, unsigned int ival );
	/* return value shows whether the operation was successful, eg. mgFALSE if not found */


extern APIFUNC(mgrec) *mgTempLightSourceTableEntry ( void );
extern APIFUNC(int) mgAddLightSourceTableEntry ( mgrec* db, mgrec* ent_rec, int match_flag );
	/* given a filled table entry, search for match and insert it to the table; 
		return entry index */
extern APIFUNC(int) mgIndexOfLtsEntry ( mgrec* db, char* name );
extern APIFUNC(char) *mgNameOfLtsEntry ( mgrec* db, int index );

extern APIFUNC(int) mgGetLtsCount ( mgrec *db );

extern APIFUNC(mgbool) mgDelMaterialTableEntry ( mgrec* db, int index );
extern APIFUNC(mgbool) mgDelMaterialTableEntryByName ( mgrec* db, char *name );

extern APIFUNC(mgbool) mgGetFirstTextureInPalette ( mgrec* db, int *index, char *textureName );
extern APIFUNC(mgbool) mgGetNextTextureInPalette ( mgrec* db, int *index, char *textureName );

extern APIFUNC(mgbool) mgAddMatrix ( mgrec* rec, mgMatrix* matrix );

extern APIFUNC(void) mgSetReadExtFlag ( int flag );
extern APIFUNC(int) mgGetRepCount ( mgrec* rec );
extern APIFUNC(mgbool) mgGetAttXmBuf ( mgrec* xrec, mgcode fcode, void* buf );

extern APIFUNC(mgbool) mgIsBsp ( mgrec* rec );
extern APIFUNC(mgbool) mgIsDof ( mgrec* rec );
extern APIFUNC(mgbool) mgIsGroup ( mgrec* rec );
extern APIFUNC(mgbool) mgIsHeader ( mgrec* rec );
extern APIFUNC(mgbool) mgIsLightSource ( mgrec* rec );
extern APIFUNC(mgbool) mgIsLod ( mgrec* rec );
extern APIFUNC(mgbool) mgIsObject ( mgrec* rec );
extern APIFUNC(mgbool) mgIsPolygon ( mgrec* rec );
extern APIFUNC(mgbool) mgIsSound ( mgrec* rec );
extern APIFUNC(mgbool) mgIsSwitch ( mgrec* rec );
extern APIFUNC(mgbool) mgIsPath ( mgrec* rec );
extern APIFUNC(mgbool) mgIsVertex ( mgrec* rec );
extern APIFUNC(mgbool) mgIsXref ( mgrec* rec );

extern APIFUNC(tagtype) mgGetType ( mgrec* rec );
extern APIFUNC(void) *mgGetUser ( mgrec* rec );
extern APIFUNC(void) mgSetUser ( mgrec* rec, void* user );
extern APIFUNC(mgrec) *mgGetAtt ( mgrec* rec0, mgcode fcode );

#define ILNEXT 1		/* traverse next bead */
#define ILON 2			/* traverse only "on" (currently displayed LOD) beads */
#define ILMASTER 4		/* traverse master bead */
#define ILNORDONLY 8	/* don't traverse read-only beads */
#define ILVERTEX 16		/* traverse vertex beads */
#define ILMASTERALL 32	/* traverse all instances */
#define ILATTR 64		/* traverse attribute beads */
#define ILXFORM 128		/* traverse xforms */
/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */
