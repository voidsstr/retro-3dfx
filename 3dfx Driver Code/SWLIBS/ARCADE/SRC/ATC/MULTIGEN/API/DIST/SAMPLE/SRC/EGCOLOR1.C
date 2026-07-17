/*******************************************************************************

  Sample file: EGCOLOR1.C

  Objective:Shows how to access color values from a color palette and
				from polygon and vertex node records.
				Shows how to convert color index values to red, green and
				blue values.
				Shows how to set color values in a color palette and
				polygon and vertex node records.

  Program functions:	Read database given on command line.
							Prints the index, RGB values, and names of each
							color in the database's color palette.
							Search the database and count how many polygons
							and vertices are using each color index.
							Increases the red component of all polygon colors.
							Increases the blue component of all vertex colors.
							Makes all palette colors max intensity.
	
  API functions used:	mgGetAttList(), mgIndex2RGB(), mgGetNextColorName(),
								mgWalk(), mgIsPolygon(), mgIsVertex(),
								mgOpenDb(), mgCloseDb().

 ******************************************************************************/
/*******************************************************************************/

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

#include "mgapiall.h"

int pcolorcount[1024];
int vcolorcount[1024];

void PrintPolyColors()
{
	int i;

	printf("\n\nNumber of Polygons with Each Color\n");
	for (i=0 ; i<1024 ; i++) {
		if (pcolorcount[i]) 
			printf("\n%d: %d", i, pcolorcount[i]);
	}
	printf("\n");
}

void PrintVtxColors()
{
	int i;

	printf("\n\nNumber of Vertices with Each Color\n");
	for (i=0 ; i<1024 ; i++) {
		if (vcolorcount[i]) 
			printf("\n%d: %d", i, vcolorcount[i]);
	}
	printf("\n");
}

mgbool CountPolyColor(mgrec *db, mgrec *par, mgrec *rec, void *info)
{
	int status, pcolor;
	float pintens;

	if (mgIsCode(rec, fltPolygon)) { /* only count polygons */
		status = mgGetAttList(rec, fltPolyPrimeColor, &pcolor, 
									fltPolyPrimeIntensity, &pintens, mgNULL);
		if (status)
			pcolorcount[pcolor]++;
		else
			printf ("Error trying to get color/intensity from polygon\n");
	}
	return (mgTRUE);
}

mgbool CountVtxColor(mgrec *db, mgrec *par, mgrec *rec, void *info)
{
	int status, vcolor;

	if (mgIsCode(rec, fltVertex)) { /* only count vertices */
		status = mgGetAttList(rec, fltVColor, &vcolor, mgNULL);
		if (status)
			vcolorcount[vcolor]++;
		else
			printf ("Error trying to get color from vertex\n");	
	}
	return (mgTRUE);
}

void main(int argc, char* argv[])

/* This program must be given an OpenFlight database file 
   as input. The database must contain at least one polygon */
{
	mgrec*			db;
	unsigned int	i; 
	short				r, g, b;
	char				*namelistptr=mgNULL, *name=mgNULL;
	char				*siteid, *message;

	/* check for correct number of arguments */
	if (argc < 2) {
		printf ("Usage: %s file_name\n", argv[0]);
		exit(1);
	}

	/* always call mgInit before any other API calls */
	mgInit(&argc, argv);

	/* open database */
	if (!(db = mgOpenDb(argv[1]))) {
		mgGetError(&siteid, &message);
		printf("\nError from %s: %s\n", siteid, message);
		exit(1);
	}

	/* print the index, RGB, and color names */
	/* for each entry in the color palette */
	printf("\nColor Palette Values\n");
	for (i = 0; i < 1024; i ++)
	{
		mgIndex2RGB (db, i, 1.f, &r, &g, &b);
		printf ("\n%d: %d, %d, %d", i, r, g, b);
		while (name = mgGetNextColorName(db, i, &namelistptr))
			printf(", %s", name);
		pcolorcount[i] = 0;
		vcolorcount[i] = 0;
		mgFree(name);
	}

	/* count how many polygons are using each palette color */
	mgWalk(db, CountPolyColor, mgNULL, mgNULL, MGWALKNORDONLY + MGWALKMASTER);
	PrintPolyColors();

	/* count how many vertices are using each palette color */
	mgWalk(db, CountVtxColor, mgNULL, mgNULL, MGWALKNORDONLY 
																+ MGWALKMASTER + MGWALKVERTEX);
	PrintVtxColors();

	/* close the database */
	mgCloseDb(db);

	/* always call mgExit() after all OpenFlight API calls */
	mgExit();

	exit(0);
}
