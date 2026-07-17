
#include "mgapibase.h"
#include "mgapiio.h"
#include "mgapistruc.h"
#include "mgapiattr.h"
#include "fltcode.h"

#include <stdio.h>

double ic0[3] = { 0., 0., 0. };
double ic1[3] = { 100., 0., 0. };
double ic2[3] = { 100., 100., 0. };
double ic3[3] = { 0., 100., 0. };

void main ( int argc, char* argv[] )
{
	mgrec* db_rec;
	mgrec* grec;
	mgrec* dofrec;
	mgrec* orec;
	mgrec* frec;
	mgrec* vrec;
	mgrec* vrec1;
	double newval = 1.0;

	mgInit ( &argc, argv );
	db_rec = mgNewDb ( argv[1] );

	grec = mgNewRec ( fltGroup );
	mgAttach ( db_rec, grec );

	dofrec = mgNewRec (fltDof);
	mgAttach (grec, dofrec);
	mgSetAttList (dofrec, fltDofMaxZ, newval, mgNULL);

	mgSetAttList (dofrec, fltDofPutAnchorX, 0.0, fltDofPutAnchorY, 10.0, fltDofPutAnchorZ, 0.0, mgNULL);
	mgSetAttList (dofrec, fltDofPutTrackX, 50.0, fltDofPutTrackY, 50.0, fltDofPutTrackZ, 0.0, mgNULL);
	mgSetAttList (dofrec, fltDofPutAlignX, 50.0, fltDofPutAlignY, 10.0, fltDofPutAlignZ, 0.0, mgNULL);

	grec = mgNewRec ( fltGroup );
	mgAttach ( dofrec, grec );

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

	mgWriteDb ( db_rec );
	mgCloseDb ( db_rec );
}
