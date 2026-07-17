/*******************************************************************************

  Sample file: MGIO.C

  Objective:Shows the structure of an OpenFlight program.  Shows how 
				to open, create, close, and write OpenFlight database files.

  Program functions:	Opens an OpenFlight database file specified on
							the command line.  Opens a new OpenFlight 
							database file named "new.flt".  Closes and writes
							both database files.  

  API functions used:	mgInit(), mgOpenDb(), mgNewDb(), mgCloseDb(), 
								mgWriteDb(), mgExit().

 ******************************************************************************/

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */
#include <string.h>		/* strcat */

#include "mgapiall.h"

void main(int argc, char* argv[])
{
	mgrec *db;		/* top record of database file specified on command line */
	mgrec *newdb;/* top record of new database file */
	mgrec *grec;		/* group record created for new database file */
	char new_fname[80];
	char *siteid, *message;

	/* Always call mgInit() before any other OpenFlight API calls */
	mgInit(&argc, argv);

	/* open the database file with the name specified on the command line */
	/* store the top record ptr in db_rec */
	if (!(db = mgOpenDb(argv[1]))) {
		mgGetError(&siteid, &message);
		printf("\nError from %s: %s\n", siteid, message);
		exit(1);
	}

	/* create a new database, store the top record ptr in new_db_rec */
	strcpy(new_fname, "newfile.flt");
	mgSetNewOverwriteFlag(mgTRUE);
	if (!(newdb = mgNewDb(new_fname))) {
		mgGetError(&siteid, &message);
		printf("\nError from %s: %s\n", siteid, message);
		exit(1);
	}
	/* create a group record and attach it to the */
	/* new database so it isn't empty */
	grec = mgNewRec(fltGroup);
	mgAttach(newdb, grec);

	/* write both database files */
	mgWriteDb(db);
	mgWriteDb(newdb);

	/* close both database files */
	mgCloseDb(db);
	mgCloseDb(newdb);

	/* always call mgExit() after all OpenFlight API calls */
	mgExit();

	exit(0);
}
