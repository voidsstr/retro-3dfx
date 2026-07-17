/*******************************************************************************

  Sample file: EGGETATTR1.C

  Objective:Shows how to access attributes from a database.

  Program functions:	Read database given on command line.
							Get and print some header attributes from the database node.
							Look for a group as the child of the database node.
							Get and print some of the group attributes.
	
  API functions used:	mgGetAttList(), mgGetChild(), mgIsCode(),
								mgOpenDb(), mgCloseDb().

 ******************************************************************************/

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

#include "mgapiall.h"

void main(int argc, char* argv[])
{
	mgrec* db;
	mgrec* rec;
	int status;
	int iv2, iv3, iv4;
	short sval;
	unsigned char units;
	char	*siteid, *message;
	
	/* check for correct number of arguments */
	if (argc < 2) {
		printf("Usage: %s file_name\n", argv[0]);
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

	/* get the units from the database header */
	mgGetAttList(db, fltHdrUnits, &units, mgNULL);
	printf("fltHdrUnits: %d\n", units);

	/* look for a group as the first child of the database node */
	if ((rec = mgGetChild(db)) && mgIsCode(rec, fltGroup)) {

		/* get and print some of the attributes of the group */
		status = mgGetAttList(rec, 
				fltGrpPrio, &sval, 
				fltGrpFlagTerrain, &iv2,
				fltGrpFlagBoxed, &iv3,
				fltGrpFlagAnimationFB, &iv4, mgNULL);

		printf("fltGrpPrio: %d\n"
					"fltGrpFlagTerrain: %d\n"
					"fltGrpFlagBoxed: %d\n"
					"fltGrpFlagAnimationFB: %d\n",
					sval, iv2, iv3, iv4);
	}

	/* close, exit */
	mgCloseDb (db);
	mgExit();
}
