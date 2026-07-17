/*******************************************************************************

  Sample file: MGMATERIAL1.C

  Objective:Show how to access entries in the material palette.

  Program functions:	Open a database file with a material palette.
						Get the first material in the palette.  Change some of
						the first material's properties. Print properties of all
						the materials in the palette.  

  API functions used:	mgNameOfMaterial(), mgGetMaterialCount(),
						mgGetFirstMaterial(), mgGetNextMaterial(),

 ******************************************************************************/

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

#include "mgapiall.h"


void PrintMaterial(mgrec *db, mgrec *mat, int index)
/* prints the attributes of a given material */
{
	float ambred, ambgreen, ambblue;
	float diffred, diffgreen, diffblue;
	float specred, specgreen, specblue;
	float matshin, matalpha;
	char *matname;

	matname = mgNameOfMaterial(db, index);
	mgGetNormColor(mat, fltAmbient, &ambred, &ambgreen, &ambblue);
	mgGetNormColor(mat, fltDiffuse, &diffred, &diffgreen, &diffblue);
	mgGetNormColor(mat, fltSpecular, &specred, &specgreen, &specblue);
	mgGetAttList(mat, fltShininess, &matshin,
							fltMatAlpha, &matalpha, mgNULL);
	printf("\nMaterial: %d\tName: %s\n", index, matname);
	printf("Ambient: %5.3f\t%5.3f\t%5.3f\t\n", ambred, ambgreen, ambblue); 
	printf("Diffuse: %5.3f\t%5.3f\t%5.3f\t\n", diffred, diffgreen, diffblue); 
	printf("Specular: %5.3f\t%5.3f\t%5.3f\t\n", specred, specgreen, specblue);
	printf("Shininess: %5.3f\tAlpha: %5.3f\n", matshin, matalpha);

	mgFree(matname);	/* mgNameOfMaterial allocs, must dealloc */
}

mgbool PrintMaterialPalette(mgrec *db)
/* print properties of all the database's materials */
{
	mgrec *mat;
	int index;

	if (mat = mgGetFirstMaterial(db, &index )) {
		printf("\nEntire Material Palette\n");
		PrintMaterial(db, mat, index);
		while (mat = mgGetNextMaterial(mat, &index)) {
			PrintMaterial(db, mat, index);
		}
		return mgTRUE;
	}
	return mgFALSE;
}


void main(int argc, char* argv[])
{
	mgrec* db;
	int index, matcount;
	mgrec	*firstmat;
	unsigned short newcolor = 5678;
	char				*siteid, *message;

	if (argc < 2) {
		printf ("Usage: %s file_name\n", argv[0]);
		exit (1);
	}

	/* always call mgInit() before any OpenFlight API calls */
	mgInit (&argc, argv);

	/* open database */
	if (!(db = mgOpenDb ( argv[1]))) {
		mgGetError(&siteid, &message);
		printf("\nError from %s: %s\n", siteid, message);
		exit(1);
	}

	/* check for light sources in this database */
	if (!(matcount = mgGetMaterialCount (db))) {
		printf ("No materials in database.\n");
		exit (1);
	}

	printf ("Total number of materials in database:  %d\n", matcount);

	/* get the first material */
	if (!(firstmat = mgGetFirstMaterial(db, &index))) {
		printf("\nCan't get first material.\n");
		exit(1);
	}

	/* get and print the attributes of the first material */
	PrintMaterial(db, firstmat, index);
	
	/* now change the specular color of the first material */
	mgSetNormColor(firstmat, fltSpecular, 0.5f, 0.5f, 0.5f);

	/* now print attributes of first material, notice that the */
	/* ambient value has changed */
	PrintMaterial(db, firstmat, index);

	/* now print the attributes of all the materials */
	PrintMaterialPalette(db);

	/* close the database */
	mgCloseDb(db);

	/* always call mgExit() after all OpenFlight API calls */
	mgExit();

	exit(0);
}
