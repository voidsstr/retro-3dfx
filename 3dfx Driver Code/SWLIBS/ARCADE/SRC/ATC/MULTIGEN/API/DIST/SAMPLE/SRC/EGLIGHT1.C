/*******************************************************************************

  Sample file: MGLIGHT1.C

  Objective:Show how to access, modify, and create entries in the light	
				source palette.

  Program functions:	Read in a database file that has a light source palette 
							Get the first light source entry and the light
							source count.  Change some of the attributes of the light 
							source.  Step through all the light sources.
							Build and add a light source entry.  Write the light
							source palette as a file.

  API functions used:mgGetLightSource(), mgIndexOfLightSource()
							mgNameOfLightSource(),
							mgGetLightSourceCount(), mgGetFirstLightSource(),
							mgGetNextLightSource(), mgNewLightSource(),
							mgWriteLightSourceFile().

 ******************************************************************************/

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

#include "mgapiall.h"

void PrintLightSource(mgrec *lightsource)
{
	char	*ltsname;
	int	ltsindex, ltstype;
	float ambred, ambgreen, ambblue, ambalpha;
	float diffred, diffgreen, diffblue, diffalpha;
	float specred, specgreen, specblue, specalpha;

	mgGetAttList(lightsource, fltLtspPaletteId, &ltsindex, mgNULL);
	ltsname = mgNameOfLightSource(lightsource, ltsindex);
	mgGetColorRGBA(lightsource, fltLtspAmbient, &ambred, &ambgreen, 
															&ambblue, &ambalpha);
	mgGetColorRGBA(lightsource, fltLtspDiffuse, &diffred, &diffgreen, 
															&diffblue, &diffalpha);
	mgGetColorRGBA(lightsource, fltLtspSpecular, &specred, &specgreen, 
															&specblue, &specalpha);
	mgGetAttList(lightsource, fltLtspType, &ltstype,	mgNULL);
	printf("\nLight Source:\n");
	printf("Index: %d\tName: %s\n", ltsindex, ltsname);
	printf("Ambient: %5.3f\t%5.3f\t%5.3f\t\n", ambred, ambgreen, ambblue); 
	printf("Diffuse: %5.3f\t%5.3f\t%5.3f\t\n", diffred, diffgreen, diffblue); 
	printf("Specular: %5.3f\t%5.3f\t%5.3f\t\n", specred, specgreen, specblue);

	/* need to deallocate the name */
	mgFree(ltsname);

}

void main(int argc, char* argv[])
{
	mgrec* db;
	int index, ltscount;
	mgrec *lts;
	unsigned short newcolor = 5678;
	char				*siteid, *message;

	if (argc < 2) {
		printf("Usage: %s file_name\n", argv[0]);
		exit(1);
	}

	/* always call mgInit() before any OpenFlight API calls */
	mgInit(&argc, argv);

	/* open database */
	if (!(db = mgOpenDb(argv[1]))) {
		mgGetError(&siteid, &message);
		printf("\nError from %s: %s\n", siteid, message);
		exit(1);
	}

	/* check for light sources in this database */
	if (!(ltscount = mgGetLightSourceCount(db)))
	{
		printf("No light sources in database.\n");
		exit(1);
	}

	printf("Total number of light sources in database:  %d\n", ltscount);

	/* get the first light source */
	lts = mgGetFirstLightSource(db, &index);

	/* get and print the attributes of this light source */
	PrintLightSource(lts);
	
	/* now change the ambient color */
	mgSetColorRGBA(lts, fltLtspAmbient, 0.5f, 0.5f, 0.5f, 0.0f);

	/* now print attributes of first light source, notice that the */
	/* ambient value has changed */
	lts = mgGetFirstLightSource(db, &index);
	PrintLightSource(lts);

	/* now print the attributes of all the other light sources */
	while(lts = mgGetNextLightSource(lts, &index))
		PrintLightSource(lts);

	/* close the database */
	mgCloseDb(db);

	/* always call mgExit() after all OpenFlight API calls */
	mgExit();

	exit(0);
}
