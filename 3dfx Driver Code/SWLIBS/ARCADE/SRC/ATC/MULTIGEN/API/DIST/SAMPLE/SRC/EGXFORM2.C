
#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

#include "mgapibase.h"
#include "mgapiio.h"
#include "mgapistruc.h"
#include "mgapiattr.h"
#include "fltcode.h"
#include "mgapipref.h"
#include "mgapicoord.h"



double ic0[3] = { 0., 0., 0. };
double ic1[3] = { 100., 0., 0. };
double ic2[3] = { 100., 100., 0. };
double ic3[3] = { 0., 100., 0. };

void main ( int argc, char* argv[] )
{
	mgrec		*db;
	mgrec		*grec, *orec, *frec, *vrec, *vrec1;
	mgrec		*xformrec;
	icoord	center;
	vector	axis;
	float		angle;

	if ( argc < 2 ) {
		printf ( "Usage: %s file_name\n", argv[0] );
		exit (1);
	}

	mgInit ( &argc, argv );
	mgSetNewOverwriteFlag ( mgTRUE );
	db = mgNewDb ( argv[1] );
	if ( !db ) 
		exit(1);

	grec = mgNewRec (fltGroup);
	mgAttach (db, grec);

	xformrec = mgNewRec (fltXmRotate);

	center.x = 1.0;
	center.y = 2.0;
	center.z = 3.0;
	
	axis.i = 4.0f;
	axis.j = 5.0f;
	axis.k = 6.0f;

	angle = 0.5f;

	if (!mgSetIcoord (xformrec, fltXmRotateCenter, center.x, 
			center.y, center.z))
		printf ("Couldn't set rotate center\n");

	if (!mgSetVector (xformrec, fltXmRotateAxis, axis.i, axis.j, axis.k))
		printf ("Couldn't set rotate axis\n");

	mgSetAttList (xformrec, fltXmRotateAngle, angle, mgNULL);

	mgAttach (grec, xformrec);

	orec = mgNewRec ( fltObject );
	mgAttach ( grec, orec );

	frec = mgNewRec ( fltPolygon );
	mgAttach ( orec, frec );

	vrec = mgNewRec ( fltVertex );
	mgAttach ( frec, vrec );
	mgSetIcoord ( vrec, fltIcoord, ic0[0], ic0[1], ic0[2]);

	vrec1 = vrec;
	vrec = mgNewRec ( fltVertex );
	mgInsert ( vrec1, vrec );
	mgSetIcoord ( vrec, fltIcoord, ic1[0], ic1[1], ic1[2]);

	vrec1 = vrec;
	vrec = mgNewRec ( fltVertex );
	mgInsert ( vrec1, vrec );
	mgSetIcoord ( vrec, fltIcoord, ic2[0], ic2[1], ic2[2]);

	vrec1 = vrec;
	vrec = mgNewRec ( fltVertex );
	mgInsert ( vrec1, vrec );
	mgSetIcoord ( vrec, fltIcoord, ic3[0], ic3[1], ic3[2]);


	mgWriteDb (db);

	mgCloseDb ( db );
}
