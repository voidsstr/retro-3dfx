/*******************************************************************************

  Sample file: EGDOF1.C

  Objective:Shows how to create DOF records.
				Shows how to set local origins and limits in DOF records.  

  Program functions:	Create new database with name from command line.
							Builds 3 bars, chain them together under DOF records.
							Set the local origin and pitch limits of each bar.
	
  API functions used:	mgSetAttList(), mgAttach(), mgNewRec(),
								mgSetColorIndex(), mgSetCurrentColorName(),
								mgAttach(), mgSetIcoord(), mgNewDb(), mgCloseDb()
								mgWriteDb().

 ******************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include "mgapiall.h"

 /* defines */
#define XDIR 1
#define YDIR 2
#define ZDIR 3
#define NXDIR 4
#define NYDIR 5
#define NZDIR 6

/* prototypes */
void addVertex(mgrec *db, mgrec *prec, double x, double y, double z); 
mgrec *makePoly(mgrec *db, unsigned int color, double offsetx, double offsety, double offsetz,
					 double size1, double size2, int direction);
mgrec *makeMovingPart(mgrec *db, unsigned int color, double offsetx, double offsety, double offsetz);

void main ( int argc, char* argv[] )
{
	mgrec* db;
	mgrec *grec, *dofrec1, *dofrec2, *dofrec3, *orec;
	double doforgx=0., doforgy=0., doforgz=0.;
	char *siteid, *message;
	unsigned int red, green, blue, yellow;
	float inten;
	
	/* check for correct number of arguments */
	if (argc < 2) {
		printf ("Usage: %s file_name\n", argv[0]);
		exit(1);
	}

	/* always call mgInit before any other API calls */
	mgInit ( &argc, argv );

	/* set red, green, blue, and yellow color indices */
	mgRGB2Index (db, (short)255, (short)0,  (short)0,	&red, &inten);
	mgRGB2Index (db, (short)255, (short)255,  (short)0,	&yellow, &inten);
	mgRGB2Index (db, (short)0, (short)255,  (short)0,	&green, &inten);
	mgRGB2Index (db, (short)0, (short)0,  (short)255,	&blue, &inten);

	/* start a new OpenFlight database, overwrite if exists */
	mgSetNewOverwriteFlag(mgTRUE);
	if (!(db = mgNewDb(argv[1]))) {
		mgGetError(&siteid, &message);
		printf("\nError from %s: %s\n", siteid, message);
		exit(1);
	}

	/* create group, 3 DOFs, chain DOFs under group */
	grec = mgNewRec(fltGroup);
	mgAttach(db, grec);
	dofrec1 = mgNewRec(fltDof);
	dofrec2 = mgNewRec(fltDof);
	dofrec3 = mgNewRec(fltDof);
	mgAttach (grec, dofrec1);
	mgAttach (dofrec1, dofrec2);
	mgAttach (dofrec2, dofrec3);

	/* make red bar under DOF 1, green under DOF 2, yellow under DOF 3 */
	orec = makeMovingPart(db, red, 50., 0., 0.);
	mgAttach(dofrec1, orec);
	orec = makeMovingPart(db, green, 150., 0., 0.);
	mgAttach(dofrec2, orec);
	orec = makeMovingPart(db, yellow, 250., 0., 0.);
	mgAttach(dofrec3, orec);

	/* set local origins and limits of DOFs */
	mgSetAttList (dofrec1, fltDofPutAnchorX, doforgx, 
								fltDofPutAnchorY, doforgy, 
								fltDofPutAnchorZ, doforgz, 
								fltDofPutTrackX, doforgx, 
								fltDofPutTrackY, doforgy+1, 
								fltDofPutTrackZ, doforgz, 
								fltDofPutAlignX, doforgx+1, 
								fltDofPutAlignY, doforgy, 
								fltDofPutAlignZ, doforgz,
								fltDofMinIncl, -30., 
								fltDofMaxIncl, 30.,
								fltDofCurIncl, 0., 
								fltDofIncrementIncl, 1.0, mgNULL);
	mgSetAttList (dofrec2, fltDofPutAnchorX, doforgx+100, 
								fltDofPutAnchorY, doforgy, 
								fltDofPutAnchorZ, doforgz, 
								fltDofPutTrackX, doforgx, 
								fltDofPutTrackY, doforgy+1, 
								fltDofPutTrackZ, doforgz, 
								fltDofPutAlignX, doforgx+101, 
								fltDofPutAlignY, doforgy, 
								fltDofPutAlignZ, doforgz,
								fltDofMinIncl, -30., 
								fltDofMaxIncl, 30.,
								fltDofCurIncl, 0., 
								fltDofIncrementIncl, 1.0, mgNULL);
	mgSetAttList (dofrec3, fltDofPutAnchorX, doforgx+200, 
								fltDofPutAnchorY, doforgy, 
								fltDofPutAnchorZ, doforgz, 
								fltDofPutTrackX, doforgx, 
								fltDofPutTrackY, doforgy+1, 
								fltDofPutTrackZ, doforgz, 
								fltDofPutAlignX, doforgx+101, 
								fltDofPutAlignY, doforgy, 
								fltDofPutAlignZ, doforgz,
								fltDofMinIncl, -30., 
								fltDofMaxIncl, 30.,
								fltDofCurIncl, 0., 
								fltDofIncrementIncl, 1.0, mgNULL);

	mgWriteDb(db);
	mgCloseDb ( db );
}

void addVertex(mgrec *db, mgrec *prec, double x, double y, double z) 
/* add a vertex to a polygon */
{
	mgrec *vrec;

	vrec = mgNewRec(fltVertex);
	mgAppend(prec, vrec);
	mgSetIcoord(vrec, fltIcoord, x, y, z);
}

mgrec *makePoly(mgrec *db, unsigned int color, double offsetx, double offsety, double offsetz,
					 double size1, double size2, int direction)
/* creates a new polygon record with 4 vertices, */
/* returns ptr to new polygon record */
{
	mgrec* prec;
	double p1[3], p2[3], p3[3], p4[3];
	double move1, move2;

	move1 = size1/2.;
	move2 = size2/2.;

	/* set vertices based on offset, size, and direction */
	switch (direction) {
		case XDIR:
					p1[0] = offsetx; p1[1] = -move2+offsety; p1[2] = -move1+offsetz;
					p2[0] = offsetx; p2[1] = move2+offsety; p2[2] = -move1+offsetz;
					p3[0] = offsetx; p3[1] = move2+offsety; p3[2] = move1+offsetz;
					p4[0] = offsetx; p4[1] = -move2+offsety; p4[2] = move1+offsetz;
				break;
		case YDIR:
					p4[0] = -move1+offsetx; p4[1] = offsety; p4[2] = -move2+offsetz;
					p3[0] = move1+offsetx; p3[1] = offsety; p3[2] = -move2+offsetz;
					p2[0] = move1+offsetx; p2[1] = offsety; p2[2] = move2+offsetz;
					p1[0] = -move1+offsetx; p1[1] = offsety; p1[2] = move2+offsetz;
				break;
		case ZDIR:
					p1[0] = -move1+offsetx; p1[1] = -move2+offsety; p1[2] = offsetz;
					p2[0] = move1+offsetx; p2[1] = -move2+offsety; p2[2] = offsetz;
					p3[0] = move1+offsetx; p3[1] = move2+offsety; p3[2] = offsetz;
					p4[0] = -move1+offsetx; p4[1] = move2+offsety; p4[2] = offsetz;
				break;
		case NXDIR:
					p4[0] = offsetx; p4[1] = -move2+offsety; p4[2] = -move1+offsetz;
					p3[0] = offsetx; p3[1] = move2+offsety; p3[2] = -move1+offsetz;
					p2[0] = offsetx; p2[1] = move2+offsety; p2[2] = move1+offsetz;
					p1[0] = offsetx; p1[1] = -move2+offsety; p1[2] = move1+offsetz;
				break;
		case NYDIR:
					p1[0] = -move1+offsetx; p1[1] = offsety; p1[2] = -move2+offsetz;
					p2[0] = move1+offsetx; p2[1] = offsety; p2[2] = -move2+offsetz;
					p3[0] = move1+offsetx; p3[1] = offsety; p3[2] = move2+offsetz;
					p4[0] = -move1+offsetx; p4[1] = offsety; p4[2] = move2+offsetz;
				break;
		case NZDIR:
					p4[0] = -move1+offsetx; p4[1] = -move2+offsety; p4[2] = offsetz;
					p3[0] = move1+offsetx; p3[1] = -move2+offsety; p3[2] = offsetz;
					p2[0] = move1+offsetx; p2[1] = move2+offsety; p2[2] = offsetz;
					p1[0] = -move1+offsetx; p1[1] = move2+offsety; p1[2] = offsetz;
				break;
	}

	/* make polygon, attach to object */
	prec = mgNewRec(fltPolygon);

	/* make vertices, attach to polygon */
	addVertex(db, prec, p1[0], p1[1], p1[2]);
	addVertex(db, prec, p2[0], p2[1], p2[2]);
	addVertex(db, prec, p3[0], p3[1], p3[2]);
	addVertex(db, prec, p4[0], p4[1], p4[2]);

	/* set color */
	mgSetAttList(prec, fltPolyPrimeColor, color, mgNULL);

	return prec;

}

mgrec *makeMovingPart(mgrec *db, unsigned int color, double offsetx, double offsety, double offsetz)
/* makes articulated part 100X10, centered at given offset */
{
	mgrec *orec, *prec;
	double size1=100, size2=10;
	double move1, move2;

	move1 = size1/2.;
	move2 = size2/2.;
	
	orec = mgNewRec(fltObject);
	prec = makePoly(db, color, offsetx, offsety, offsetz+move2, size1, size2, ZDIR);
	mgAttach(orec, prec);
	prec = makePoly(db, color, offsetx, offsety+move2, offsetz, size1, size2, YDIR);
	mgAttach(orec, prec);
	prec = makePoly(db, color, offsetx, offsety, offsetz-move2, size1, size2, NZDIR);
	mgAttach(orec, prec);
	prec = makePoly(db, color, offsetx, offsety-move2, offsetz, size1, size2, NYDIR);
	mgAttach(orec, prec);
	prec = makePoly(db, color, offsetx+move1, offsety, offsetz, size2, size2, XDIR);
	mgAttach(orec, prec);
	prec = makePoly(db, color, offsetx-move1, offsety, offsetz, size2, size2, NXDIR);
	mgAttach(orec, prec);
	
	return orec;
}