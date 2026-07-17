/*******************************************************************************

  Sample file: EGTEXTURE6.C

  Objective:	Show how to load and save textures from a texture palette file.

  Program functions:	Opens a database from the command line.
							Loads a texture palette file into the database's texture
							palette.
							Writes the new texture palette to a new texture palette
							file.
							Writes the database with the new texture palette.
							The "-r" option means that the database's palette will
							be completely replaced by the new one.
	
  API functions used:mgGetTextureCount(), mgGetFirstTexture(),
							mgGetNextTexture(), mgDeleteTexture(),
							mgReadTexturePalette(), mgWriteTexturePalette().
 ******************************************************************************/
/*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mgapiall.h"

static void print_error(char *msg)
{
	if (msg)
		printf("%s\n", msg);
}

static int clear_palette(mgrec *db_rec)
{
	int numtxtrs, i;
	int *txtrindices;		/* Texture index list */
	char	patname[256];

		/* Get a count of the textures in the palette */
	numtxtrs = mgGetTextureCount(db_rec);

		/* Allocate a texture index list */
	txtrindices = (int *) malloc(numtxtrs * sizeof(int));
	if (!txtrindices)
		return (mgFALSE);

		/* Collect the texture indices */
	i = 0;
	mgGetFirstTexture(db_rec, &txtrindices[i], patname);
	while (mgGetNextTexture(db_rec, &txtrindices[i], patname))
		i++;

		/* Delete each texture by index */
	for (i = 0; i < numtxtrs; i++) {
		mgDeleteTexture(db_rec, txtrindices[i]);
	}

		/* Free the index list */
	free(txtrindices);

	return ( mgTRUE );
}

void main (int argc, char **argv)
{
	mgrec *db_rec;
	char *siteid, *errortxt;

	if (argc < 4) {
		printf("Usage: %s databasefile inpalettefile outpalettefile [-r]\n", argv[0]);
		exit(0);
	}

		/* Initialize the API */
	mgInit(&argc, argv);

		/* Load the database */
	if (!(db_rec = mgOpenDb(argv[1]))) {
		mgGetError(&siteid, &errortxt);
		printf("%s", errortxt);
		exit(EXIT_FAILURE);
	}

		/* Check for the replace palette option */
	if ((argc > 4) && (strcmp(argv[4], "-r") == 0)) {
		if (!clear_palette(db_rec)) {
			print_error("Failed to clear Palette");
			exit(EXIT_FAILURE);
		}
	}

		/* Load the texture palette file */
	if (!mgReadTexturePalette(db_rec, argv[2])) {
		print_error("Texture palette file read failed");
		exit(EXIT_FAILURE);		
	}

		/* Write the database's texture palette to a file */
	if (!mgWriteTexturePalette(db_rec, argv[3])) {
		print_error("Texture palette file write failed");
		exit(EXIT_FAILURE);		
	}

		/* write the database */
	if (!mgWriteDb(db_rec)) {
		print_error("Database write failed");
		exit(EXIT_FAILURE);
	}

		/* close the database and exit */
	mgCloseDb(db_rec);
	mgExit();
	exit(0);
}
