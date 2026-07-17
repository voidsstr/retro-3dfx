/*******************************************************************************
 * 
 * $Header: egextend1.c, 4, 10/11/00 7:32:58 PM, Brent$
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
	mgrec *grec;
	mgrec *mrec;
	mgrec *crec;
	mgrec *lrec;
	mgrec* orec;
	mgrec* frec;
	mgrec* vrec;
	mgrec* vrec1;
	mgrec prec;
	char *str;

	if ( argc < 2 ) {
		printf ( "Usage: %s new_file_name\n", argv[0] );
		exit (1);
	}

	mgInit ( &argc, argv );
	/* if ( !ddInit ( EAS_DDL, &siteno ) )		already done by FLT_init() TEMP...
		exit(1);		/* error msg already sent */

	if ( !( db = mgNewDb ( argv[1] ) ) ) {
		printf ( "Open failure - %s\n", argv[1] );
		exit(1);
	}

	grec = mgNewRec ( fltGroup );
	mgAttach ( db, grec );
	if ( mrec = mgNewRec ( easMesh ) ) {
		/* mgSetAttList ( mrec, easPriorityEnum, 234, mgNULL ); */
		mgAttach ( grec, mrec );
		mgSetName ( mrec, "EasMesh_1" );
		mgSetComment ( mrec, "This is a mesh");
		if ( str = mgGetName ( mrec ) ) {
			printf ( "%s\n", str );
			mgFree ( str ); /* mgGetName allocs, user must dealloc */
		}
		if ( crec = mgNewRec ( easCell ) ) {
			mgAttach ( mrec, crec );
			mgSetAttList ( crec, easSelectLevel, 456, mgNULL );
			mgSetName ( crec, "EasCell_1" );
			mgSetComment ( crec, "This is a cell");
		}
	}
	mgPrintRec ( mrec );
	mgPrintRec ( mgGetChild ( mrec ) );

	lrec = mgNewRec ( fltLodFloat );
	mgAttach ( crec, lrec );

	orec = mgNewRec ( fltObject );
	mgAttach ( lrec, orec );

	frec = mgNewRec ( fltPolygon );
	mgAttach ( orec, frec );

	vrec = mgNewRec ( fltVertex );
	mgAttach ( frec, vrec );
	mgSetAttBuf ( vrec, fltIcoord, (void*) ic0 );
	mgSetComment ( vrec, "This is the first vertex comment");

	vrec1 = mgNewRec ( fltVertex );
	mgAppend ( frec, vrec1 );
	mgSetAttBuf ( vrec1, fltIcoord, (void*) ic1 );

	vrec = mgNewRec ( fltVertex );
	mgInsert ( vrec1, vrec );
	mgSetAttBuf ( vrec, fltIcoord, (void*) ic2 );

	vrec = mgNewRec ( fltVertex );
	mgAppend ( frec, vrec );
	mgGetAttRec ( vrec, fltIcoord, &prec );
	mgSetAttValD( &prec, fltIcoordX, ic3[0] );
	mgSetAttValD( &prec, fltIcoordY, ic3[1] );
	mgSetAttValD( &prec, fltIcoordZ, ic3[2] );

	mgPrintRec ( grec );

	mgWriteDb ( db );
	mgCloseDb ( db );
}
