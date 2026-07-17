/*******************************************************************************

  Sample file: EGTEXTURE3.C

  Objective: Show how to access information from the texture mapping palette.

  Program functions:	Steps through all the texture mappings in the texture
							mapping palette and prints the name and type of each one.
	
  API functions used:mgGetFirstTextureMapping(), mgGetNextTextureMapping(),
							mgGetTextureMappingType(), mgGetTextureMappingMatrix()

 ******************************************************************************/
/*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mgapiall.h"

static void print_matrix(double matrix[4][4])
{
	printf("\tmatrix = {\n");
	printf("\t\t%f\t%f\t%f\t%f\n", matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3]);
	printf("\t\t%f\t%f\t%f\t%f\n", matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]);
	printf("\t\t%f\t%f\t%f\t%f\n", matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]);
	printf("\t\t%f\t%f\t%f\t%f\n", matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
	printf("\t}\n");
}

void main (int argc, char **argv)
{
	int type;
	int mapindex;
	mgrec *db_rec;
	char	mapname[256];
	char *siteid, *errortxt;
	double matrix[4][4];

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

		/* Walk the mapping palette & print the name & type of each mapping */
	if (mgGetFirstTextureMapping(db_rec, &mapindex, mapname)) {
		do {
			switch (type = mgGetTextureMappingType(db_rec, mapindex)) {
				case 1:
					if (*mapname)
						printf("Texture Mapping %d: %s: type = 3 Point Put\n", mapindex, mapname);
					else
						printf("Texture Mapping %d: (no name): type = 3 Point Put\n", mapindex);
					if (mgGetTextureMappingMatrix(db_rec, mapindex, matrix))
						print_matrix(matrix);
					else
						printf("ERROR - No Matrix\n");
					break;
				case 2:
					if (*mapname)
						printf("Texture Mapping %d: %s: type = 4 Point Put\n", mapindex, mapname);
					else
						printf("Texture Mapping %d: (no name): type = 4 Point Put\n", mapindex);
					if (mgGetTextureMappingMatrix(db_rec, mapindex, matrix))
						print_matrix(matrix);
					else
						printf("ERROR - No Matrix\n");
					break;
				case 4:
					if (*mapname)
						printf("Texture Mapping %d: %s: type = Spherical Project\n", mapindex, mapname);
					else
						printf("Texture Mapping %d: (no name): type = Spherical Project\n", mapindex);
					break;
				case 5:
					if (*mapname)
						printf("Texture Mapping %d: %s: type = Radial Project\n", mapindex, mapname);
					else
						printf("Texture Mapping %d: (no name): type = Radial Project\n", mapindex);
					break;
				default:
					if (*mapname)
						printf("Texture Mapping %d: %s: ERROR - Unknown Type\n", mapindex, mapname);
					else
						printf("Texture Mapping %d: (no name): ERROR - Unknown Type\n", mapindex);
					break;
			}
		} while (mgGetNextTextureMapping(db_rec, &mapindex, mapname));
	}

		/* Close the database and exit */
	mgCloseDb(db_rec);
	mgExit();
	exit(0);
}
