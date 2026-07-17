/*******************************************************************************

  Sample file: MGWALK1.C

  Objective:Shows how to traverse an OpenFlight database in several ways.

  Program functions:	Open a database from command line.  
							Traverse the database without visiting vertices, or 
								references.  
							Traverse the database, visiting vertices and the first
								instance of each referenced node.  
							Traverse the database, visiting vertices and every instance
								of each referenced node.  
							Cycle through the database's levels of detail, traverse 
								and visit only the records associated with the current
								level of detail.  

  API functions used:	mgWalk(), mgMoreDetail().

 ******************************************************************************/

#include "mgapiall.h"

#include <stdlib.h> /* exit */
#include <string.h> /* strcpy */
#include <stdio.h>

static mgbool Printrecname (mgrec* db_rec, mgrec* par_rec, mgrec* rec, char *id) 
{
	char *name;

	if (!mgGetPrevious(rec))
		printf("\n");
	if (name = mgGetName(rec)) {
		printf("%s\t", name);
		strcpy (id, name); /* save name in user data... for whatever reason */
		mgFree(name); /* mgGetName allocs, user must dealloc */
	}
	return (mgTRUE);
} 

void main(int argc, char* argv[])
{
	mgrec* db;
	char *siteid;
	char *message;
	char idname[80];
	char *id = idname;

	/* check for the correct number of command line arguments */
   if (argc == 1) {
		printf("Usage: %s fltfilename\n");
		printf("       fltfilename = input flt file\n", argv[0]);
		exit(0);
	}

	/* Always call mgInit() before any other OpenFlight API calls */
	mgInit(&argc, argv);

	/* open database */
	if (!(db = mgOpenDb(argv[1]))) {
		mgGetError(&siteid, &message);
		printf("\nError from %s: %s\n", siteid, message);
		exit(1);
	}

	/* traverse, don't visit read-only, vertices, references, xforms */
	/* "id" shows use of the user data pointer */
	printf("\nMGWALKNORDONLY\n");
	mgWalk(db, Printrecname, mgNULL, id, MGWALKNORDONLY); 
	printf("\n");

	/* traverse, don't visit read only, xforms, only visit one instance */
	/* of each reference node */
	printf("\nMGWALKNORDONLY + MGWALKVERTEX + MGWALKMASTER\n");
	mgWalk(db, mgNULL, Printrecname, id, MGWALKNORDONLY 
															+ MGWALKVERTEX 
															+ MGWALKMASTER);
	printf("\n");

	/* traverse, don't visit read-only, vertices, xforms, visit all instances */
	printf("\nMGWALKNORDONLY + MGWALKMASTER + MGWALKMASTERALL\n");
	mgWalk(db, Printrecname, mgNULL, id, MGWALKNORDONLY 
															+ MGWALKVERTEX 
															+ MGWALKMASTER
															+ MGWALKMASTERALL);
	printf("\n");

	/* cycle through each level of detail, traverse and visit only */
	/* those nodes associated with the current level of detail */
	while (mgMoreDetail(db)) {
		printf("\n New Level Of Detail: MGWALKNORDONLY + MGWALKON\n");
		mgWalk(db, Printrecname, mgNULL, id, MGWALKNORDONLY 
															+ MGWALKON); 
		printf("\n");
	}

	/* close the database */
   mgCloseDb(db);

	/* always call mgExit() after all OpenFlight API calls */
   mgExit();

	exit(0);
}


