
#include <stdio.h>
#include <stdlib.h>
#include "mgapibase.h"
#include "mgapiio.h"
#include "mgapistruc.h"
#include "mgapiattr.h"
#include "mgapiinfo.h"
#include "fltcode.h"

void main ( int argc, char* argv[] )
{
	mgrec* db_rec;
	mgrec* grec;
	mgrec* grec2;
	mgrec* xrec;
	double x, y, z;
	int xtype;
	char *siteid, *errortxt;

	if ( argc < 2 ) {
		printf ( "Usage: egsetattr fname\n" );
		exit (0);
	}

	mgInit ( &argc, argv );
	if ( !( db_rec = mgOpenDb ( argv[1] ) ) ) {
		mgGetError ( &siteid, &errortxt );
		printf ( "%s", errortxt );
		exit(-1);
	}

	if ( !( grec = mgGetRecByName ( db_rec, "g1" ) ) ) {
		mgGetError ( &siteid, &errortxt );
		printf ( "%s", errortxt );
		exit(-1);
	}
	printf ( "===> group info:\n" );
	mgPrintRec ( grec );

	/*** NOTE: must use box1.flt as test file... ***/

	if ( grec2 = mgGetRecByName ( db_rec, "g2" ) ) {
		/* get/set xformll... both methods work! */
		if (mgHasXform (grec2)) {
			xrec = mgGetXform (grec2);
			while (xrec) {
				mgPrintRec ( xrec );
				if ( xtype = mgGetXformType (xrec) ) {
					switch ( xtype ) {
					case XLL_TRANSLATE:
						if ( mgGetIcoord ( xrec, fltXmTranslateFrom, &x, &y, &z ) )
							printf ( "fltXmTranslateFrom: %lf, %lf, %lf\n", x, y, z );
						if ( mgGetIcoord ( xrec, fltXmTranslateDelta, &x, &y, &z ) )
							printf ( "fltXmTranslateDelta: %lf, %lf, %lf\n", x, y, z );
						break;
					case XLL_SCALE:
						if ( mgGetIcoord ( xrec, fltXmScaleCenter, &x, &y, &z ) )
							printf ( "fltXmScaleCenter: %lf, %lf, %lf\n", &x, &y, &z );
						x = 11.234;y=22.56; z=33.789;
						mgSetIcoord ( xrec, fltXmScaleCenter, x, y, z );
						if ( mgGetIcoord ( xrec, fltXmScaleCenter, &x, &y, &z ) )
							printf ( "fltXmScaleCenter: %lf, %lf, %lf\n", x, y, z );
						if ( mgGetAttList ( xrec, fltXmScaleX, &x, fltXmScaleY, &y, fltXmScaleZ, &z, mgNULL ) ) {
							printf ( "fltXmScaleX: %lf\n", x );
							printf ( "fltXmScaleY: %lf\n", y );
							printf ( "fltXmScaleZ: %lf\n", z );
						}
						break;
					}  /* switch */
				}  /* if  */

				xrec = mgGetNext (xrec);
			}  /* while */
		}  /* if  */
	}

#if 0
	/* get/set fltIDblInfo */
	mgGetAttValP ( grec, fltIDblInfo, &ptr1 );
	printf ( "old ptr1: %x\n", ptr1 );
	printf ( "db_rec: %x\n", db_rec );
	mgSetAttValP ( grec, fltIDblInfo, (void*) db_rec );
	mgGetAttValP ( grec, fltIDblInfo, &ptr1 );
	printf ( "new ptr1: %x\n", ptr1 );

	/* get/set fltILinkInfo */
	mgGetAttBuf ( grec, fltILinkInfo, &ptr2 );
	printf ( "old ptr2: %x\n", ptr2 );
	printf ( "db_rec: %x\n", grec );
	mgSetAttBuf ( grec, fltILinkInfo, &grec );
	mgGetAttBuf ( grec, fltILinkInfo, &ptr2 );
	printf ( "new ptr2: %x\n", ptr2 );


	rec = grec;
	while ( rec = mgGetChild ( rec ) ) {
		if ( mgGetCode( rec ) == fltPolygon ) {		/* first face */
			printf ( "===> face field info:\n" );
			if ( mgGetAttRec ( rec, fltPolyIrMaterial, &my_rec ) )
				mgPrintField ( &my_rec );
			if ( tmp_rec = mgGetAtt ( rec, fltPolyFlagTerrain ) )
				mgPrintField ( tmp_rec );
		}
	}
#endif

	mgWriteDb ( db_rec );
	mgCloseDb ( db_rec );
}
