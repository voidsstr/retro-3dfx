/*******************************************************************************

  Sample file: EGTEXTURE2.C

  Objective:	Show how to access information from textures in the texture
					palette.

  Program functions:	Open a database from file name given on command line.
							Steps through all the textures in the database's 
							texture palette and prints the texture attributes of
							each.
	
  API functions used:mgGetFirstTexture(), mgGetTextureAttributes(), 
							mgGetAttList(), mgGetNextTexture().

 ******************************************************************************/
/*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mgapiall.h"

void main (int argc, char **argv)
{
	int type, width, height;
	int patindex;
	mgrec *attr_rec, *db_rec;
	char	patname[256];
	char *siteid, *errortxt;

	/* check for proper arguments */
	if (argc < 2) {
		printf("Usage: %s file_name\n", argv[0]);
		exit(1);
	}

		/* Initialize the API */
	mgInit(&argc, argv);

		/* Load the database */
	if (!(db_rec = mgOpenDb(argv[1]))) {
		mgGetError(&siteid, &errortxt);
		printf("%s", errortxt);
		exit(EXIT_FAILURE);
	}

		/* Walk the texture palette & print the width, height & type of each texture */
	if (mgGetFirstTexture(db_rec, &patindex, patname)) {
		do	{
			if (attr_rec = mgGetTextureAttributes(db_rec, patindex)) {
				mgGetAttList ( attr_rec, fltImgWidth, &width,
								fltImgHeight, &height,
								fltImgType, &type,
								mgNULL);

				printf("Texture %d: %s: width = %d, height = %d, numChannels = %d\n", 
														patindex, patname, width, height, (type-1));
			}
			else {
				printf("Texture %d: %s: Error: cannot get attributes\n", 
														patindex, patname);
			}
		} while (mgGetNextTexture(db_rec, &patindex, patname));
	}

		/* Close the database and exit*/
	mgCloseDb(db_rec);
	mgExit();
	exit(0);
}
