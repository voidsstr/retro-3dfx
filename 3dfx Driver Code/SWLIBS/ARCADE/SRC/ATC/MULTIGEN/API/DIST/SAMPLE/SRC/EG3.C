/*******************************************************************************
 * 
 * $Header: eg3.c, 4, 10/11/00 7:32:54 PM, Brent$
 * $Revision: 4$
 * $Author: Brent$
 * $Date: 10/11/00 7:32:54 PM$
 *
 ******************************************************************************/

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* exit */

/* it should never happen like this... */

#include "mgapibase.h"
#include "mgapiinfo.h"
#include "mgapiio.h"
#include "mgapicolor.h"
#include "mgapistruc.h"
#include "mgapiattr.h"
#include "mgapiutil.h"
#include "mgapidd.h"
#include "fltcode.h"

main ( int argc, char* argv[] )
{
	mgrec* db_rec;
	mgrec* rec;
	short r, g, b, a;
	unsigned int index;
	float inten;
	char *siteid, *errortxt;

	if ( argc < 2 ) {
		printf ( "Usage: eg1 fname\n" );
		exit (1);
	}

	mgInit ( &argc, argv );
	if ( !( db_rec = mgOpenDb ( argv[1] ) ) ) {
		mgGetError ( &siteid, &errortxt );
		printf ( "%s", errortxt );
		exit(1);
	}

	rec = db_rec;
	while ( rec = mgGetChild ( rec ) ) {
		if ( mgGetCode( rec ) == fltPolygon ) {		/* get face color */
			do {
				if ( ! mgGetAttList ( rec, fltPolyPrimeColor, &index,
													fltPolyPrimeIntensity, &inten, mgNULL ) ) {
					printf( "\nERROR: mgGetAttList() can't get index/intensity \n");
					exit (1);
				}
				
				mgIndex2RGB ( index, inten, &r, &g, &b );
				printf ( "\n\tmgIndex2RGB: index=%d, inten=%f, rgb %d,%d,%d \n",
							index, inten,r,g,b);
				
				mgRGB2Index ( mgNULL, r, g, b, &index, &inten );
				printf ( "\n\tmgRGB2Index: index=%d, inten=%f \n",index, inten);
				
			} while ( ( rec = mgGetNext ( rec ) ) && ( mgGetCode( rec ) == fltPolygon ) );
			break;
		}
	}

	mgCloseDb ( db_rec );
}
