
#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

#include "mgapibase.h"
#include "mgapiio.h"
#include "mgapistruc.h"
#include "mgapiattr.h"
#include "fltcode.h"


void main ( int argc, char* argv[] )
{
	mgrec *db;
	mgrec *group;
	mgrec *xrec;
	int xtype;
	double maxlimit;

	if ( argc < 2 ) {
		printf ( "Usage: %s file_name\n", argv[0] );
		exit (1);
	}

	mgInit ( &argc, argv );
	db = mgOpenDb ( argv[1] );
	if ( !db ) 
		exit(1);

	group = mgGetChild (db);
	while (group && (mgGetCode (group) != fltGroup)) group = mgGetChild (group);

	if (mgHasXform (group)) {
		xrec = mgGetXform (group);
		xtype = mgGetXformType (xrec);

		mgGetAttList (xrec, fltXmLimitMax, &maxlimit, mgNULL);
		printf ("XFORM Limit Max = %f\n", maxlimit);

		switch (xtype){
		case XLL_TRANSLATE : {
			double x, y, z;

			mgGetIcoord (xrec, fltXmTranslateFrom, &x, &y, &z);
			printf ("TranslateFrom xyz:  %f, %f, %f\n", x, y, z);
			break;
		}
		case XLL_ROTPT : {
			float angle;
			double x, y, z;
			float i, j, k;
			
			mgGetIcoord (xrec, fltXmRotateCenter, &x, &y, &z);
			printf ("RotateCenter: %f, %f, %f\n", x, y, z);
			mgGetAttList (xrec, fltVectorI, &i, fltVectorJ, &j, fltVectorK, &k, mgNULL);
			printf ("RotateAxis: %f, %f, %f\n", i, j, k);
			mgGetAttList (xrec, fltXmRotateAngle, &angle, mgNULL);
			printf ("RotateAngle: %f\n", angle );
			break;
		}
		default :
			break;
		}
	}
/*	mgWriteDb (db);   */

	mgCloseDb ( db );
}
