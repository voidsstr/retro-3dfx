/*******************************************************************************

  Sample file: EGXREF1.C

  Objective:Shows how to walk a database that has external references.
				Shows how to deal with the palette override flags.
				
  Program functions:	Read database given on command line.
							Traverses the database and any external references in it.
							Keeps track of the correct palettes to use by checking
							the palette override flags in the external reference
							record. 
							Prints the name and code of each record visited.
							Announces when an external reference is being
							stepped into.
							Print the color RGB of all polygons using the correct
							color palette.
	
  API functions used:	mgGetAttList(), mgIndex2RGB(), mgGetName()
								mgWalk(), mgIsPolygon(), mgIsCode(),
								mgOpenDb(), mgCloseDb().

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "mgapiall.h"

typedef struct currentpalette {
		mgrec *clr; /* which db to get color palette from */
		mgrec *mat; /* which db to get material palette from */
		mgrec *txt; /* which db to get texture palette from */
} currentpalettetype;


static mgbool PrintPolyRGB ( mgrec* db_rec, mgrec* par_rec, mgrec* rec, void *info ) 
{	
	char* id;
	mgcode code = mgGetCode (rec);
	char *xfname;
	currentpalettetype *pal = info;

	if ( id = mgGetName ( rec ) ) {
		printf ( "%s(%d)\t", id, code );

		if (mgIsCode(rec, fltXref)) {
			mgrec *xdb;
			int	colpalovrd = 0;
			int	matpalovrd = 0;
			int	txtpalovrd = 0;

			mgGetAttList (rec, 
					fltXrefFilename, &xfname, 
					fltGcColPal, &colpalovrd,
					fltGcMatPal, &matpalovrd,
					fltGcTxtPal, &txtpalovrd,
					mgNULL);

			printf ("Begin %s ------------------\n", xfname);
			if (xdb = mgOpenDb (xfname)) {
				mgrec *savePalDb = pal->clr;
				if (colpalovrd) {
					pal->clr = xdb;
					printf ("Using external color palette\n");
				}
				mgWalk(xdb, PrintPolyRGB, mgNULL, pal, MGWALKMASTER + MGWALKMASTERALL);
				printf("\n");
				mgCloseDb (xdb);
				pal->clr = savePalDb;
			}
			else
				printf ("Open file failure for xref <%s>\n", xfname);

			printf ("end %s ------------------\n", xfname);
			mgFree(xfname);
		}
	}
	else
		printf ( "No name (code=%d) ", code);

	if (mgIsCode(rec, fltPolygon)) {
		int col;
		float intens;
		unsigned short	r, g, b;

		mgGetAttList (rec, 
				fltPolyPrimeColor, &col,
				fltPolyPrimeIntensity, &intens,
				mgNULL);
		mgIndex2RGB (pal->clr, col, intens, &r, &g, &b);
		printf ("Poly color (%d/%3.2f) = (%d, %d, %d)\n", col, intens, r, g, b);
	}
	return ( mgTRUE );
}

void main ( int argc, char* argv[] )
{
	mgrec* db;
	int	status=0, patindex=0;
	char	*siteid, *message;
	currentpalettetype pal;
	
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
	
	/* get color palette from top db */
	pal.clr = db;
	mgWalk(db, PrintPolyRGB, mgNULL, &pal, MGWALKMASTER + MGWALKMASTERALL);
	printf("\n");

   mgCloseDb(db);

   mgExit();
}


