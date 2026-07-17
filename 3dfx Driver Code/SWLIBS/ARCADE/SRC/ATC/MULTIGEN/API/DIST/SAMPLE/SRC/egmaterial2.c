/*******************************************************************************

  Sample file: MGMATERIAL2.C

  Objective:Shows how to add and modify materials in the material palette.
				Shows how to assign materials to polygons.  

  Program functions:	Create new database with name from command line.
							Create a database hierarchy with 2 polygons.
							Add a material to the material palette.
							Modify the first material in the palette.
							Assign the new material to one of the polygons and
							the modified material to the other.
	
  API functions used:	mgSetNormColor(), mgGetFirstMaterial(), mgSetAttList(),
								mgAttach(), mgSetIcoord(), mgNewDb(), mgCloseDb()
								mgAppend(), mgWriteDb().

 ******************************************************************************/

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

#include "mgapiall.h"

/* forward declarations */
mgrec *makeMaterial(mgrec *db, int *index, char *name);
mgrec *makeStructure(mgrec *db_rec);
void addVertex(mgrec *db_rec, mgrec *prec, double x, double y, double z); 
mgrec *makePoly(mgrec *db, int mat, double offsetx, double offsety, double offsetz);

void main ( int argc, char* argv[] )
{
	mgrec *db;
	mgrec *orec;
	mgrec *prec1, *prec2;
	int newmatindex, firstmatindex;
	mgrec	*newmat, *firstmat;
	char *siteid, *message;

	/* check for proper arguments */
	if (argc < 2) {
		printf("Usage: %s file_name\n", argv[0]);
		exit(1);
	}
	
	/* always call mgInit before any other API calls */
	mgInit(&argc,argv);

	/* start a new OpenFlight database, overwrite if exists */
	mgSetNewOverwriteFlag(mgTRUE);
	if (!(db = mgNewDb(argv[1]))) {
		mgGetError(&siteid, &message);
		printf("\nError from %s: %s\n", siteid, message);
		exit(1);
	}

	/* get the first material in the palette and modify it */
	firstmat = mgGetFirstMaterial(db, &firstmatindex);
	mgSetNormColor(firstmat, fltDiffuse, .8f, 0.f, .8f); /* change to purple */

	/* make a new material in the palette */
	newmat = makeMaterial (db, &newmatindex, "New Mat");

	/* create simple hierarchy with 2 polygons */
	/* one has the new material, the other has the modified material */
	orec = makeStructure(db);
	prec1 = makePoly(db, newmatindex, 0., 0., 0.);
	prec2 = makePoly(db, firstmatindex, 100., 0., 0.);
	mgAttach(orec, prec1);
	mgAttach(orec, prec2);
	
	mgWriteDb(db);
	mgCloseDb(db);
}

mgrec *makeMaterial(mgrec *db, int *index, char *name)
/* adds a material to the palette, returns the record and index */
{
	mgrec *mat;

	mat = mgNewMaterial(db, name, index);
	mgSetNormColor(mat, fltAmbient, .2f, .2f, .2f);
	mgSetNormColor(mat, fltDiffuse, 0.f, 0.f, .8f);
	mgSetNormColor(mat, fltSpecular, .5f, .5f, .5f);
	mgSetAttList(mat, fltShininess, 100.f,
							fltMatAlpha, 1.0f, mgNULL);
	return mat;
}

mgrec *makeStructure(mgrec *db)
/* make structure containing group/object, attach under db */
/* return the object rec ptr */
{
	mgrec *grec, *orec;

  	/* make group and object, attach object to group */
	grec = mgNewRec(fltGroup);
	mgAttach(db, grec);
	orec = mgNewRec(fltObject);
	mgAttach(grec, orec);

	return orec;
}

void addVertex(mgrec *db_rec, mgrec *prec, double x, double y, double z) 
/* add a vertex to a polygon */
{
	mgrec *vrec;

	vrec = mgNewRec(fltVertex);
	mgAppend(prec, vrec);
	mgSetIcoord(vrec, fltIcoord, x, y, z);
}

mgrec *makePoly(mgrec *db, int mat, double offsetx, double offsety, double offsetz)
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

	/* set material and color of polygon*/
	mgSetAttList(prec, fltPolyMaterial, mat, 
							fltPolyPrimeColor, 0, mgNULL);
	return prec;
}
