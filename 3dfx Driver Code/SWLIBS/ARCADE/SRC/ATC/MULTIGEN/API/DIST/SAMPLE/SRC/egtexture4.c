/*******************************************************************************

  Sample file: EGTEXTURE4.C

  Objective:	Show how to get statistics about the texture palette.

  Program functions:	Steps through all the textures in the texture palette and
							print the height, width, type and memory usage of each 
							one.
							Prints the total number of textures and the total 
							size of the textures in the texture palette.
	
  API functions used:mgGetFirstTexture(), mgGetTextureAttributes(), 
							mgGetAttList(), mgGetNextTexture(), mgIsTextureDefault(),
							mgGetTextureCount(), mgGetTextureTotalSize().
 ******************************************************************************/
/*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mgapiall.h"

void main (int argc, char **argv)
{
	int type, width, height, mem_size, count;
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

		/* Walk the texture palette & print the width, height, type &
			memory usage of each texture */
	if (mgGetFirstTexture(db_rec, &patindex, patname)) {
		do {
			if (attr_rec = mgGetTextureAttributes(db_rec, patindex)) {
				mgGetAttList(attr_rec, fltImgWidth, &width,
								fltImgHeight, &height,
								fltImgType, &type,
								mgNULL);
				mem_size = mgGetTextureSize(db_rec, patindex);

				if (mgIsTextureDefault(db_rec, patindex))
					printf("Texture %d (DEFAULT TEXTURE): %s: width = %d, height = %d, numChannels = %d, size = %d bytes\n",
								patindex, patname, width, height, (type-1), mem_size);
				else
					printf("Texture %d: %s: width = %d, height = %d, numChannels = %d, size = %d bytes\n",
								patindex, patname, width, height, (type-1), mem_size);
			}
			else {
				printf ("Texture %d: %s: Error: cannot get attributes\n", patindex, patname);
			}
		} while (mgGetNextTexture(db_rec, &patindex, patname));
	}

		/* Get the total texture count & memory usage */
	count = mgGetTextureCount(db_rec);
	mem_size = mgGetTextureTotalSize(db_rec);

	printf("\n");
	printf("Total %d textures using %d bytes\n", count, mem_size);

		/* Close the database and exit */
	mgCloseDb(db_rec);
	mgExit();
	exit(0);
}
