
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
	mgrec* orec;
	mgrec* frec;
	mgrec* vrec;
	mgrec* vrec1;
	char *comment;

	mgInit ( &argc, argv );
	db_rec = mgNewDb ( argv[1] );

	grec = mgNewRec ( fltGroup );
	mgAttach ( db_rec, grec );
	orec = mgNewRec ( fltObject );
	mgAttach ( grec, orec );

	mgSetComment ( grec, "This is the group comment");
	comment = mgGetComment ( grec );
	printf ("Group Comment:  <%s>\n", comment);
	mgFree ( comment ); /* mgGetComment allocs, user must dealloc */

	frec = mgNewRec ( fltPolygon );
	mgAttach ( orec, frec );
	mgSetComment ( frec, "This is the polygon comment");
	comment = mgGetComment ( frec );
	printf ("Face Comment:  <%s>\n", comment);
	mgFree ( comment ); /* mgGetComment allocs, user must dealloc */

	mgDeleteComment (frec);
	comment = mgGetComment ( frec );
	printf ("Face Comment after delete:  <%s>\n", comment);	
	mgFree ( comment ); /* mgGetComment allocs, user must dealloc */

	vrec = mgNewRec ( fltVertex );
	mgAttach ( frec, vrec );
	mgSetIcoord ( vrec, fltIcoord, ic0[0], ic0[1], ic0[2]);
	mgSetComment ( vrec, "This is the first vertex comment");

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
