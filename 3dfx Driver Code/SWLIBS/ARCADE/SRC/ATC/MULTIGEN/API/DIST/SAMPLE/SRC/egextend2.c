/*******************************************************************************
 * 
 * $Header: egextend2.c, 4, 10/11/00 7:32:58 PM, Brent$
 * $Revision: 4$
 * $Author: Brent$
 * $Date: 10/11/00 7:32:58 PM$
 *
 ******************************************************************************/

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

#define EAS_DDL "easdata.dll"

#include "mgapibase.h"
#include "mgapiinfo.h"
#include "mgapiio.h"
#include "mgapicolor.h"
#include "mgapistruc.h"
#include "mgapiattr.h"
#include "mgapiutil.h"
#include "mgapipref.h"
#include "mgapidd.h"

#include "fltcode.h"
#include "eascode.h"

double ic0[3] = { 0., 0., 0. };
double ic1[3] = { 100., 0., 0. };
double ic2[3] = { 100., 100., 0. };
double ic3[3] = { 0., 100., 0. };


void main ( int argc, char* argv[] )
{
	mgrec *db;
	mgrec *mrec;
	mgrec *crec;
	mgrec* orec;
	mgrec* frec;
	mgrec* vrec;
	mgrec* vrec1;
	mgrec prec;
	char *str;
	int val1, val2;
	mgbool ok;

	if ( argc < 2 ) {
		printf ( "Usage: %s new_file_name\n", argv[0] );
		exit (1);
	}

	mgInit ( &argc, argv );
	/* if ( !ddInit ( EAS_DDL, &siteno ) )		already done by FLT_init() TEMP...
		exit(1);		/* error msg already sent */

	mgSetNewOverwriteFlag ( mgTRUE );
	if ( !( db = mgNewDb ( argv[1] ) ) ) {
		printf ( "Open failure - %s\n", argv[1] );
		exit(1);
	}

	if ( !( mrec = mgNewRec ( easMesh ) ) )
		exit (1);
	mgSetAttList ( mrec, gdfRangeSorted, mgTRUE, mgNULL );
	mgSetComment ( mrec, "This is a mesh");
	mgAttach ( db, mrec );
	if ( str = mgGetName ( mrec ) ) {
		printf ( "%s\n", str );
		mgFree ( str ); /* mgGetName allocs, user must dealloc */
	}
	mgPrintRec ( mrec );

	if ( !( crec = mgNewRec ( easCell ) ) )
		exit (1);
	mgAttach ( mrec, crec );
	mgSetAttList ( crec, easSelectLevel, 456, mgNULL );
	mgSetAttList ( crec, easX, 100., mgNULL );
	mgSetComment ( crec, "This is a cell");

	mgPrintRec ( mgGetChild ( mrec ) );

	orec = mgNewRec ( easObject );
	mgAttach ( crec, orec );

	frec = mgNewRec ( fltPolygon );
	mgAttach ( orec, frec );

	mgSetAttList ( frec, easPaint, 123, mgNULL );
	mgGetAttList ( frec, easPaint, &val1, mgNULL );
	mgSetAttList ( frec, easFogFlag, mgTRUE, mgNULL );
	mgGetAttList ( frec, easFogFlag, &val2, mgNULL );

	vrec = mgNewRec ( fltVertex );
	mgPrintRec ( frec );

	mgAttach ( frec, vrec );
	mgSetAttBuf ( vrec, fltIcoord, (void*) ic0 );
	mgSetComment ( vrec, "This is the first vertex comment");
	mgPrintRec ( vrec );

	vrec1 = mgNewRec ( fltVertex );
	mgAppend ( frec, vrec1 );
	mgSetAttBuf ( vrec1, fltIcoord, (void*) ic1 );
	ok = mgSetAttList ( vrec1, gdfPointConformal, mgTRUE, mgNULL );
	ok = mgGetAttList ( vrec1, gdfPointConformal, &val1, mgNULL );
	mgPrintRec ( vrec1 );

	vrec = mgNewRec ( fltVertex );
	mgInsert ( vrec1, vrec );
	mgSetAttBuf ( vrec, fltIcoord, (void*) ic2 );

	vrec = mgNewRec ( fltVertex );
	mgAppend ( frec, vrec );
	mgGetAttRec ( vrec, fltIcoord, &prec );
	mgSetAttValD( &prec, fltIcoordX, ic3[0] );
	mgSetAttValD( &prec, fltIcoordY, ic3[1] );
	mgSetAttValD( &prec, fltIcoordZ, ic3[2] );

	mgWriteDb ( db );
	mgCloseDb ( db );
}
