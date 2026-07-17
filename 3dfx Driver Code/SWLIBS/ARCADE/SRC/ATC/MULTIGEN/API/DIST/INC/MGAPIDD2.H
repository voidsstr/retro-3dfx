/*******************************************************************************
 * 
 * $Header: mgapidd2.h, 4, 10/11/00 7:32:16 PM, Brent$
 * $Revision: 4$
 * $Author: Brent$
 * $Date: 10/11/00 7:32:16 PM$
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


#ifndef _MGAPIDD2_H_
#define _MGAPIDD2_H_

#include "mgdddecl.h"
#include "mgapidd1.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/

DDFUNC(mgbool)		ddImportSchema(const char* filename, int* fillinSiteno);
						/* ddImportSchema imports the given schema file into in-memory
						structures.  If successful it returns mgTRUE, and fills in
						*fillinSiteno with the new siteno. */

DDFUNC(mgbool)			ddExportSchema(int siteno, const char* fileName);
							/* ddExportSchema exports the given siteno to the given fileName. */

DDFUNC(mgbool)			ddExportHeader(int siteno, const char* codeFileName, const char* tableFileName);
							/* ddExportHeader writes out two header files, one containing
							enums ('codeFileName') and one containing the data
							structures ('tableFileName').  The given site's data is exported. */

DDFUNC(mgbool)			ddExportDocumentation(int siteno, const char* fileName);
							/* ddExortDocumentation writes out a documentation file for
							the given site. */

/*============================================================================*/
/* Creation API */

DDFUNC(mgbool)			ddNewSite ( const char* siteName, const char* dllName, int* fillinSiteno );
							/* ddNewSite creates a new site with the given name and
							returns the associated siteno in fillinSiteno. */

DDFUNC(mgcode)			ddNewRecordDef (int siteno);
DDFUNC(mgcode)			ddNewDataDef (int siteno);
							/* ddNewXxxDef functions create a new record_def or data_def
							and return the associated mgcode value.  The caller can then
							use that code value to define other attributes via calls
							described elsewhere. */

/*============================================================================*/
/* Record/Data Common API */

DDFUNC(mgbool)			ddSetTagType ( mgcode code, tagtype type );
DDFUNC(mgbool)			ddSetKey ( mgcode code, int key );
DDFUNC(mgbool)			ddSetIsDisplay ( mgcode code, mgbool isDisplay );
DDFUNC(mgbool)			ddSetIsEdit ( mgcode code, mgbool isEdit );
DDFUNC(mgbool)			ddSetIsSearch ( mgcode code, mgbool isSearch );
DDFUNC(mgbool)			ddSetName ( mgcode code, const char* name );
DDFUNC(mgbool)			ddSetLabel ( mgcode code, const char* label );
DDFUNC(mgbool)			ddSetDesc ( mgcode code, const char* msg );
DDFUNC(mgbool)			ddSetXcode ( mgcode code, char *xstring );
DDFUNC(mgbool)			ddSetLength ( mgcode code, int len );	/* user's responsible */
							/* These methods access the following fields in the common
							header of data_def and record_def structures: type, key, display,
							edit, search, id (ddSetName), label, msg (ddSetDesc).
							NOTE - The char* parameters are copied, so the caller
							retains responsibility for their storage. */

/*============================================================================*/
/* Data Def API */

DDFUNC(mgbool)			ddSetRange ( mgcode dcode, float minVal, float maxVal );
DDFUNC(mgbool)			ddSetDefVal ( mgcode dcode, float defVal );
DDFUNC(mgbool)			ddUnsetRange ( mgcode dcode );
DDFUNC(mgbool)			ddUnsetDefVal ( mgcode dcode );
							/* ddSetRange and ddSetDefVal set the range and default
							value of a data_def struct.  The ddUnsetXxx calls set the
							def_set and limit_set booleans to FALSE. */

/*============================================================================*/
/* Record Def API */

DDFUNC(mgbool)			ddSetIlevel ( mgcode rcode, int ilevel );
DDFUNC(mgbool)			ddSetField ( mgcode rcode, attr_def*, int count );
DDFUNC(mgbool)			ddSetParent ( mgcode rcode, parent_def*, int count );
DDFUNC(mgbool)			ddSetChild ( mgcode rcode, child_def*, int count );
DDFUNC(mgbool)			ddSetAlias ( mgcode rcode, alias_def*, int count );
							/* ddSetIlevel sets the ilev of the given record_def.
							ddSetField, -Parent, -Child, and -Alias set the arrays
							and counts for the given record_def.  If the array is
							NULL, the 'count' value is ignored.
							NOTE: The arrays are copied, so the caller retains
							responsibility for their storage. */

DDFUNC(mgbool)		ddSetUnpackedField ( mgcode rcode, attr_def* atbl, int count, int length );
						/* DO NOT USE unless you know what you are doing.
						ddSetUnpackedField alters the field list for an UNPACKED
						record.  The caller must supply a completely filled-in
						attr_def (with all deltas computed) and the length of
						the resultant record.  The record will be marked unpacked
						and will not be readjusted automatically if fields change,
						so all fields should be unpacked.  If this is not the case,
						mgFALSE will be returned. */

/*============================================================================*/
/* Attr Def API */

DDFUNC(mgbool)			ddSetFieldIsPointer ( mgcode rcode, attr_def* aptr );
DDFUNC(mgbool)			ddSetFieldIsStructure ( mgcode rcode, attr_def* aptr );
DDFUNC(mgbool)			ddSetFieldIsDC ( mgcode rcode, attr_def* aptr );
							/* ddSetFieldIsXxx sets the 'ptr' field of the given
							attr_def. */

/*============================================================================*/
/* Length and offset fixup */

DDFUNC(mgbool)			ddCacheFixups ( mgbool cacheFixups );
							/* ddCacheFixups(mgTRUE) makes all ddSetXxx calls hold off
							recomputing record lengths until the next call to ddFixup.
							ddCacheFixups(mgFALSE) causes calls to ddSetXxx fixup lengths
							of records immediately.  Use caching when many changes are
							going to be made before any schema are read via ddGetXxx calls.
							DEFAULT is NO CACHING.  Returns previous value of cacheFixups. */

DDFUNC(void)			ddFixup ();
							/* ddFixup recalculates all lengths and all offsets.
							This is a slow operation, but is it O(n). */

/*----------------------------------------------------------------------------*/
/* END OF PUBLIC API */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/* Length and offset fixup */

mgbool					ddiForEachSchema(mgbool (*fn)(data_def*, code_type, void*),
												  mgbool visitRecords, mgbool visitData,
												  void* context );
							/* Call the given function for each record_def and data_def in the
							system.  Pass the context pointer through unchanged.  During
							each call to fn, code_type indicates if the record is a
							data_def or a record_def; if the latter.  If
							the function returns mgFALSE, iteration stops and mgFALSE
							is returned.  Otherwise mgTRUE is returned to indicate
							that all records were visited. */

mgbool					ddiAUXSetLengthToZero( data_def *def, code_type type, void* context );
mgbool					ddiAUXRecomputeLength( data_def *def, code_type type, void* context );
							/* Auxiliary functions passed to ddiForEachRecord. */

void						ddiRecomputeRecordLength ( mgcode rcode );
							/* Recompute the length of a record. */

void						ddiPropagateLengthChange ( mgcode code );
							/* Propagate a change in the length of the given schema
							to the rest of the schema. */

int						ddiGetAttrLength ( const attr_def* attr );
							/* Given an attr_def, return the length it will occupy
							in memory. */

int						ddiComputeRecordLength ( record_def *rec );
							/* Calculate the length of a record schema.  Set the
							attr_def.delta values according to computed
							lengths of fields.
							
							NOTE: Length is NOT multiplied by 'acnt'; if there is an array
							of items, the length of only 1 item is returned. */

int						ddiGetLength ( mgcode code );
							/* Calculate the length of a data or record schema.  If
							acnt is >1, the single length will be multiplied as appropriate to
							return the length of the entire array. */

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /*_MGAPIDD2_H_*/
/* DON'T ADD ANYTHING AFTER THIS #endif */

/*eof*/
