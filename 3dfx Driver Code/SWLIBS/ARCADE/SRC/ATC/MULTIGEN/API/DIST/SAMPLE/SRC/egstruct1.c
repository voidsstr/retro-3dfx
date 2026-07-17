/*******************************************************************************

  Sample file: MGSTRUCT1.C

  Objective:Shows how to access node records in an OpenFlight database.  
				Shows the structure of an OpenFlight database.  
				Shows how to use various other OpenFlight API structure 
					library routines.

  Program functions:Open a database from command line.  
						Traverse the database from the top record down to the first
							object record.  
						Print the names of the object record's parent, child, 
							previous, and next records.
						Count the children of the object record.
						Traverse the database and print all the node records that
							have nested children.
						Get the file name from the object record.


  API functions used:	mgGetNext(), mgGetPrevious(), mgGetParent(), 
								mgGetNestedParent(), mgGetNestedChild(), 
								mgGetChild(), mgGetChildNth(), mgIsCode(),
								mgRec2Filename(), mgCountChild(), mgGetName().

 ******************************************************************************/
	
#include "mgapiall.h"

#include <stdlib.h> /* exit */
#include <stdio.h> /* printf */

mgbool Findnested(mgrec *db, mgrec *par, mgrec *rec, void *info)
{
	mgrec *nrec;
	char *name, *nname;

	/* if find record with nested child, print name */
	/* of record and nested child */
	if (nrec = mgGetNestedChild( rec)) {
		if (mgGetNestedParent(nrec) != rec) {
			printf("\nthere was a nested record problem");
			exit(1);
		}
		else {
			name = mgGetName(rec);
			nname = mgGetName(nrec);
			printf("\n%s has nested child %s", name, nname);
			mgFree(name);
			mgFree(nname); /* mgGetName allocs, user must dealloc */
		}
	}
	return(mgTRUE);
}

void printAdjacent(mgrec *rec)
/* prints the names of a node record's parent, 1st child, previous, next, etc. */
{
	char *childname, *objname, *parname, *prevname, *nextname, *childnthname;

	objname = mgGetName(rec);
	parname = mgGetName(mgGetParent(rec));
	childname = mgGetName(mgGetChild(rec));
	prevname = mgGetName(mgGetPrevious(rec));
	nextname = mgGetName(mgGetNext(rec));
	childnthname = mgGetName(mgGetChildNth(rec, 2));
	printf("\nBead name: %s", objname);
	printf("\nBead's parent: %s", parname ); 
	printf("\nBead's 1st child: %s", childname ); 
	printf("\nBead's previous: %s", prevname ); 
	printf("\nBead's next: %s", nextname ); 
	printf("\nBead's number of children: %d", mgCountChild  ( rec ) ); 
	printf("\nBead's 2nd child: %s", childnthname ); 

	mgFree(objname) ; mgFree(parname) ; mgFree(childname);
	mgFree(prevname) ; mgFree(nextname) ; mgFree(childnthname);
}

void main(int argc, char* argv[])
{
	mgrec *db_rec;
	mgrec *child_rec;
	mgrec *obj_rec;
	char *siteid, *message;
	char *filename, *childname;

	/* Always call mgInit() before any other OpenFlight API calls */
	mgInit(&argc, argv);

	/* open the database file with the name specified on the command line */
	/* store the top record ptr in db_rec, check for errors */
	if (!(db_rec = mgOpenDb(argv[1]))) {
		mgGetError(&siteid, &message);
		printf("\n%s", message);
		exit(1);
	}

	/* traverse one branch of tree until an object record is found, */
	/* print the name of each record */
	child_rec = mgGetChild(db_rec);
	while ((child_rec = mgGetChild(child_rec)) && !mgIsCode(child_rec, fltObject)) {
		childname = mgGetName(child_rec);
		printf("\n%s", childname);
		mgFree(childname);
	}

	/* print names of the object record's parent, child, and next record */
	/* print how many children it has */
	/* print the name of the 2nd child */
	obj_rec = child_rec;
	printAdjacent(obj_rec);

	/* traverse database, look for records with nested children */
	mgWalk(db_rec, Findnested, mgNULL, mgNULL, MGWALKNORDONLY 
																+ MGWALKMASTER);

	/* find the name of the database file from this object record */
	/* illustrates how you can get the database file name from any record */
	if (filename = mgRec2Filename(obj_rec)) {
		printf("\nDatabase file: %s", filename);
		mgFree(filename);
	}

	/* close the database file */
	mgCloseDb(db_rec);

	/* always call mgExit() after all API function calls */
	mgExit();

	exit (0);
}
