/*******************************************************************************

  Sample file: MGSTRUCT2.C

  Objective:Show how to create, delete, duplicate, attach, and move records 
			around in an OpenFlight database.

  Program functions:Create new database with filename from command line.  
					Create a group with 3 child groups.
					Create object, polygon, and vertex records with
					certain attributes.  Duplicate the object record and attach
					it under a group.  Share the object with the other group.
					Write the file.

  API functions used:	mgNewRec(), mgDuplicate(),
						mgAttach(), mgAppend(), mgInsert(), mgReference()
						

 ******************************************************************************/

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

#include "mgapiall.h"

/* forward declarations */
void addVertex(mgrec *db_rec, mgrec *prec, double x, double y, double z) ;
mgrec *makePoly(mgrec *db, unsigned int color, double offsetx, double offsety, double offsetz);

void main(int argc, char* argv[])
{
	mgrec *db;		/* top record of database file specified on command line */
	mgrec *grec1;		/* group record created for new database file */
	mgrec *grec2;		/* group record created for new database file */
	mgrec *grec3;		/* group record created for new database file */
	mgrec *grec4;		/* group record created for new database file */
	mgrec *orec1;		/* object record created for new database file */
	mgrec *orec2;		/* object record created for new database file */
	mgrec *prec1;		/* polygon record created for new database file */
	mgrec *prec2;		/* nested polygon record created for new database file */
	unsigned int blue, red;		/* color indices */
	float inten;		/* color intensity */
	char *siteid, *message;

	/* Always call mgInit() before any other OpenFlight API calls */
	mgInit(&argc, argv);

	/* open the database file with the name specified on the command line */
	/* store the top record ptr in db_rec */
	mgSetNewOverwriteFlag(mgTRUE);
	if (!(db = mgNewDb(argv[1]))){
		mgGetError(&siteid, &message);
		printf("\nError from %s: %s\n", siteid, message);
		exit(1);
	}

	/* create a group record and attach it to the */
	/* new database so it isn't empty */
	grec1 = mgNewRec(fltGroup);
	mgAttach(db, grec1);

	/* create 3 child groups under the first group */
	grec2 = mgNewRec(fltGroup);
	mgAttach(grec1, grec2);
	grec3 = mgNewRec(fltGroup);
	mgAttach(grec1, grec3);
	grec4 = mgNewRec(fltGroup);
	mgAttach(grec1, grec4);

	/* now create an object which is not attached to the database */
	orec1 = mgNewRec(fltObject);

	/* get color indices for blue and red */
	mgRGB2Index(db, 0, 0, 255, &blue, &inten);
	mgRGB2Index(db, 255, 0, 0, &red, &inten);

	/* create a polygon with nested polygon, attach to object */
	prec1 = makePoly(db, blue, 0., 0., 0.);
	prec2 = makePoly(db, red, 50., 50., 0.);
	mgAttach(prec1, prec2);
	mgAttach(orec1, prec1);

	/* now share the object between the second and fourth groups */
	mgReference(grec2, orec1);
	mgReference(grec4, orec1);

	/* now duplicate the object  */
	/* and attach under the third group */
	orec2 = mgDuplicate(orec1);
	mgAttach(grec3, orec2);

	/* write database file */
	mgWriteDb(db);

	/* close database */
	mgCloseDb(db);

	/* always call mgExit() after all OpenFlight API calls */
	mgExit();

	exit(0);
}

void addVertex(mgrec *db_rec, mgrec *prec, double x, double y, double z) 
/* add a vertex to a polygon */
{
	mgrec *vrec;

	vrec = mgNewRec(fltVertex);
	mgAppend(prec, vrec);
	mgSetIcoord(vrec, fltIcoord, x, y, z);
}

mgrec *makePoly(mgrec *db, unsigned int color, double offsetx, double offsety, double offsetz)
/* creates a new polygon record with 4 vertices, */
/* returns ptr to new polygon record */
{
	mgrec* prec;
	double ic0[3] = {0., 0., 0.};
	double ic1[3] = {100., 0., 0.};
	double ic2[3] = {100., 100., 0.};
	double ic3[3] = {0., 100., 0.};

	/* make polygon, attach to object */
	prec = mgNewRec(fltPolygon);

	/* make vertices, attach to polygon */
	addVertex(db, prec, ic0[0]+offsetx, ic0[1]+offsety, ic0[2]+offsetz);
	addVertex(db, prec, ic1[0]+offsetx, ic1[1]+offsety, ic1[2]+offsetz);
	addVertex(db, prec, ic2[0]+offsetx, ic2[1]+offsety, ic2[2]+offsetz);
	addVertex(db, prec, ic3[0]+offsetx, ic3[1]+offsety, ic3[2]+offsetz);

	/* set color */
	mgSetAttList(prec, fltPolyPrimeColor, color, mgNULL);

	return prec;

}
