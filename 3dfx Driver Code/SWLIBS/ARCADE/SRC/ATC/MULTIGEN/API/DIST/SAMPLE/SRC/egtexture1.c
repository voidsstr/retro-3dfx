/*******************************************************************************

  Sample file: EGTEXTURE1.C

  Objective:Shows how to access texture information from polygon records.

  Program functions:	Read database given on command line.
							Prints texture information ( name, index, type,
							height, and width ) for each textured polygon
							in the database.
							Prints texture mapping information for each
							polygon that has a texture mapping.
							Prints the UV information for each vertex of 
							textured polygons.
	
  API functions used:mgGetAttList(), mgIsCode(), mgGetTextureMappingType(),
							mgGetTextureName(), mgGetTextureAttributes(),
							mgGetTextureMappingName(), mgGetFirstTexture(), mgWalk(),
							mgOpenDb(), mgCloseDb().
	
 ******************************************************************************/
/*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "mgapiall.h" /* includes all API headers */

static mgbool Action(mgrec* db_rec, mgrec* par_rec, mgrec* rec, void *unused) 
{	
	unsigned char level = 0;
	short tindex = -1, tmindex = -1;
	char *tname, *tmname;
	mgrec *txtrec;
	int width, height, type;
	float u, v;
	static int hasTexture = mgFALSE;

	if (!rec)
		return mgFALSE;

		/* if rec is a polygon */
	if (mgIsCode(rec, fltPolygon))
	{
			/* Get the polygon's texture index */
		mgGetAttList(rec, fltPolyTexture, &tindex,
									fltPolyTexmap, &tmindex,
									mgNULL);

			/* If the polygon is textured */
		if (tindex > -1)
		{
			hasTexture = mgTRUE;

				/* Get some info about the texture */
			tname = mgGetTextureName(db_rec, tindex);
			txtrec = mgGetTextureAttributes(db_rec, tindex);
			mgGetAttList(txtrec, fltImgWidth, &width,
											fltImgHeight, &height,
											fltImgType, &type,
											mgNULL);

			printf("Texture index = %d name = %s, type = %d\n", tindex, tname, type);
			printf("width = %d height = %d\n", width, height);
		}
		else
			hasTexture = mgFALSE;

			/* If the polygon has a mapping */
		if (tmindex > -1)
		{
				/* Get some info about the mapping */
			tmname = mgGetTextureMappingName(db_rec, tmindex);
			switch (type = mgGetTextureMappingType(db_rec, tmindex))
			{
				case 1:
					if (tmname && *tmname)
						printf("Texture Mapping %d: %s: type = 3 Point Put\n", tmindex, tmname);
					else
						printf("Texture Mapping %d: (no name): type = 3 Point Put\n", tmindex);
					break;
				case 2:
					if (tmname && *tmname)
						printf("Texture Mapping %d: %s: type = 4 Point Put\n", tmindex, tmname);
					else
						printf("Texture Mapping %d: (no name): type = 4 Point Put\n", tmindex);
					break;
				case 4:
					if (tmname && *tmname)
						printf("Texture Mapping %d: %s: type = Spherical Project\n", tmindex, tmname);
					else
						printf("Texture Mapping %d: (no name): type = Spherical Project\n", tmindex);
					break;
				case 5:
					if (tmname && *tmname)
						printf("Texture Mapping %d: %s: type = Radial Project\n", tmindex, tmname);
					else
						printf("Texture Mapping %d: (no name): type = Radial Project\n", tmindex);
					break;
				default:
					if (tmname && *tmname)
						printf("Texture Mapping %d: %s: ERROR - Unknown Type\n", tmindex, tmname );
					else
						printf("Texture Mapping %d: (no name): ERROR - Unknown Type\n", tmindex );
					break;
			}
		}
	}

		/* if rec is a vertex and it's parent polygon has a texture
			assigned to it */
	else if ((mgIsCode(rec, fltVertex)) && hasTexture)
	{
			/* Get the texture u,v coordinates */
		mgGetAttList(rec, fltVU, &u,
									fltVV, &v,
									mgNULL);
		printf("u, v = %f, %f\n", u, v);
	}

	return(mgTRUE);   /* If a FALSE is returned the walk will terminate */
}

void main(int argc, char* argv[])
{
	mgrec* db;
	int	status=0, patindex=0;
	char	patname[256];
	char *siteid, *errortxt;

	/* check for the correct number of arguments */	
   if (argc == 1) {
		printf("Usage: %s fltfilename\n", argv[0]);
		printf("       fltfilename = input flt file\n");
		exit(0);
	}

		/* Initialize the API */
	mgInit(&argc, argv);

		/* Load the database */
	if (!(db =mgOpenDb(argv[1])))
	{
		mgGetError(&siteid, &errortxt);
		printf("%s", errortxt);
		exit(EXIT_FAILURE);
	}

		/* Check for textures in the palette */
	status = mgGetFirstTexture(db, &patindex, patname);

	if (status)
		printf("First pattern (%d) name is %s\n", patindex, patname);
	else
	{
		printf("No textures\n");
	   mgCloseDb(db);
	   exit(0);
	}

		/* Walk the database */
	mgWalk(db, Action, mgNULL, mgNULL, MGWALKVERTEX);

		/* Close the database */
   mgCloseDb(db);

   mgExit();

	exit(0);
}


