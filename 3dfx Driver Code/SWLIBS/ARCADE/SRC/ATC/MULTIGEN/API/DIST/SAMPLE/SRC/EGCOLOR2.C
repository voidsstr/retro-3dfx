/*******************************************************************************

  Sample file: EGCOLOR2.C

  Objective:Shows how to set color values and names in a color palette.
				Shows how to set color and intensity values in polygon and
				vertex records.  
				Shows how to set a polygon to use color RGB mode instead of
				color index mode.
				Shows how to convert color index values to red, green and
				blue values.
				Shows how to set color values in a color palette and
				polygon and vertex node records.

  Program functions:	Create new database with name from command line.
							Builds a simple color palette from scratch and
							saves it to disk.
							Creates a color RGB mode polygon and a color index
							mode polygon ribbon, assigns color values to each.
	
  API functions used:	mgSetPolyRGBA(), mgRGB2index(), mgNewRec(),
								mgSetColorIndex(), mgSetCurrentColorName(),
								mgAttach(), mgSetIcoord(), mgNewDb(), mgCloseDb()
								mgWriteDb().

 ******************************************************************************/

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

#include "mgapiall.h"

/* forward declarations */
mgrec *makePoly(mgrec *db, double offsetx, double offsety, double offsetz);
void addVertex(mgrec *db, mgrec *prec, double x, double y, double z);
void makeColorPalette(mgrec *db);
mgrec *makeStructure(mgrec *db);
void addColor(mgrec *db, int i, short r, short g, short b);


void main(int argc, char* argv[])
{
	mgrec* db;
	mgrec *precrgb, *precindex, *orec, *vrec;
	int i;
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

	/* build the database's color palette */
	makeColorPalette(db);

	/* save the color palette to disk */
	mgWriteColorPalette(db, "palette.clr");

	/* make a simple database hierarchy */
	orec = makeStructure(db);

	/* make a red poly with color RGB mode set */
	precrgb = makePoly(db, 0., 0., 0.);
	mgAttach(orec, precrgb);
	mgSetAttList(precrgb, fltPolyFlagRgbMode, mgTRUE, mgNULL);
	mgSetPolyRGBA(precrgb, 255, 0, 0, 0);

	/* make polys with color index mode set (one per index) */
	for ( i=0 ; i<1024 ; i++) {
		precindex = makePoly(db, 5.*(float)i, 0., 0.);
		mgAttach(orec, precindex);
		mgSetAttList(precindex, 
								fltPolyPrimeColor, i, 
								fltPolyPrimeIntensity, 1.0f,
								mgNULL);

		/* get first vertex of color index face */
		if (vrec = mgGetChild(precindex)) {

			/* set vertex's color and intensity */
			mgSetAttList(vrec, fltVColor, 1023-i, fltVIntensity, .4, mgNULL);
		}
		else
			printf("ERROR:  No vertex child of polygon.\n");
	}

	/* write and close the database */
	mgWriteDb(db);
	mgCloseDb(db);
	
	/* always call mgExit() after all OpenFlight API calls */
	mgExit();

	exit(0);
}

mgrec *makeStructure(mgrec *db_rec)
/* make structure containing group/object, attach under db */
/* return the object rec ptr */
{
	mgrec *grec, *orec;

  	/* make group and object, attach object to group */
	grec = mgNewRec(fltGroup);
	mgAttach(db_rec, grec);
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


mgrec *makePoly(mgrec *db, double offsetx, double offsety, double offsetz)
/* creates a new polygon record with 4 vertices, */
/* returns ptr to new polygon record */
{
	mgrec* prec;
	double ic0[3] = {0., 0., 0.};
	double ic1[3] = {5., 0., 0.};
	double ic2[3] = {5., 1000., 0.};
	double ic3[3] = {0., 1000., 0.};

	/* make polygon, attach to object */
	prec = mgNewRec(fltPolygon);

	/* make vertices, attach to polygon */
	addVertex(db, prec, ic0[0]+offsetx, ic0[1]+offsety, ic0[2]+offsetz);
	addVertex(db, prec, ic1[0]+offsetx, ic1[1]+offsety, ic1[2]+offsetz);
	addVertex(db, prec, ic2[0]+offsetx, ic2[1]+offsety, ic2[2]+offsetz);
	addVertex(db, prec, ic3[0]+offsetx, ic3[1]+offsety, ic3[2]+offsetz);

	return prec;

}

void addColor(mgrec *db, int i, short r, short g, short b)
/* adds color to palette */
{
	char name[32];

	mgSetColorIndex(db, i, r, g, b);
	sprintf(name, "C%d_%d_%d_%d", i, r, g, b);
	mgNewColorName(db, i, name);
	mgSetCurrentColorName(db, i, name);
}

void makeColorPalette(mgrec *db) 
{
/* creates a color palette, gives each entry a name */
/* which is a string containing the RGB values: */
/* "C<index>_<red>_<green>_<blue> */

	short r, g, b;
	int i, j, index = 0;
	double f1, f2, ramp_length;

	typedef struct {
		short r, g, b;
	} rgb;

	rgb primary[] = {
		{0,	0,		0},	/* black */
		{128,	128,	128},	/* grey */
		{0,	0, 	255},	/* blue */
		{0,	255,	255},	/* cyan */
		{0,	255,	0},	/* green */
		{255,	255,	0},	/* yellow */
		{255,	0,		0},	/* red */
		{255,	0,		255},	/* magenta */
		{255,	255,	255}	/* white */
	};

	ramp_length = 1024./8.;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < (int)ramp_length; j++) {
			f2 = (float)j/ramp_length;
			f1 = 1. - f2;
			r = (short)(f1 * primary[i].r + f2 * primary[i+1].r);
			g = (short)(f1 * primary[i].g + f2 * primary[i+1].g);
			b = (short)(f1 * primary[i].b + f2 * primary[i+1].b);
			mgSetColorIndex ( db, index++, r, g, b );
		}
	}
}

