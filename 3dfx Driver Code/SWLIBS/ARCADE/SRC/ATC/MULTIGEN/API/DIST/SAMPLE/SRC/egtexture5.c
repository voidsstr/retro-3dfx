/*******************************************************************************

  Sample file: EGTEXTURE5.C

  Objective:	Show how to add a texture to a texture palette. 
					Show how to copy a texture from one database to another.

  Program functions:	Opens two databases from filenames given on command line.
							Reads in a texture from command line.
							Adds the texture to the first database's texture palette.
							Copies the texture to the second database's
							texture palette.
							Writes out both databases.
	
  API functions used:mgInsertTexture(), mgCopyTexture().

 ******************************************************************************/
/*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mgapiall.h"

static void print_error(char *msg) {
	if (msg)
		printf("%s\n", msg);
}

void main(int argc, char **argv)
{
	int newindex;
	mgrec *db_rec1, *db_rec2;
	char *siteid, *errortxt;

	/* check for proper arguments */
	if (argc < 4) {
		printf("Usage: %s databasefile1 databasefile2 texturefile\n", argv[0]);
		exit(0);
	}

		/* Initialize the API */
	mgInit(&argc, argv);

		/* Load the databases */
	if (!(db_rec1 = mgOpenDb(argv[1]))) {
		mgGetError(&siteid, &errortxt);
		printf("%s", errortxt);
		exit(EXIT_FAILURE);
	}
	if (!(db_rec2 = mgOpenDb(argv[2]))) {
		mgGetError(&siteid, &errortxt);
		printf("%s", errortxt);
		exit(EXIT_FAILURE);
	}

		/* Read a new texture into the first database's palette */
	newindex = mgInsertTexture(db_rec1, argv[3]);

	printf ("Texture %s added to Database %s at index %d\n",
					argv[3], argv[1], newindex);

		/* Copy the new texture into the second database's palette */
	newindex = mgCopyTexture(db_rec2, db_rec1, argv[3], newindex);

	printf ("Texture %s added to Database %s at index %d\n",
					argv[3], argv[2], newindex);

			/* write the databases */
	if (!mgWriteDb(db_rec1))
	{
		print_error("Database write failed");
		exit (EXIT_FAILURE);
	}
	if (!mgWriteDb(db_rec2))
	{
		print_error("Database write failed");
		exit(EXIT_FAILURE);
	}

		/* Close the databases and exit */
	mgCloseDb(db_rec1);
	mgCloseDb(db_rec2);
	mgExit();
	exit(0);
}
