/*******************************************************************************
 * 
 * $Header: texturepalettefile.c, 4, 10/11/00 7:33:15 PM, Brent$
 * $Revision: 4$
 * $Author: Brent$
 * $Date: 10/11/00 7:33:15 PM$
 *
 ******************************************************************************/


/*********************************************************************

	Best display requires a tab setting of 3

	PROPRIETARY RIGHTS NOTICE: All rights reserved. This program
	contains proprietary information and trade secrets of Software
	Systems of San Jose, CA., and embodies substantial creative
	efforts and confidential information, ideas, and expressions.
	No part of this program may be reproduced in any form, or by
	any means electronic, mechanical, or otherwise, without the
	written permission of Software Systems.

	COPYRIGHT NOTICE: Copyright (C) 1986 to 1991, Software Systems,
	1884 The Alameda, San Jose, CA 95126.

**********************************************************************/

/*********************************************************************

	Module Description:

	Example program for Texture Palette File functions.


*********************************************************************/

/***********************/
/* 	 RCS markers     */
/***********************/

/* $R_HEAD: $ */
/* $R_L: $
 *
 *  */

/***************************************************
*																	*
*	#includes and #defines									*
*																	*
***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mgapibase.h"
#include "mgapiinfo.h"
#include "mgapiio.h"
#include "mgapistruc.h"
#include "mgapiattr.h"
#include "mgapitexture.h"
#include "fltcode.h"

/***************************************************
*																	*
*	External References										*
*																	*
***************************************************/

/***************************************************
*																	*
*	Locals (Used Only in This Module)					*
*																	*
***************************************************/

/***************************************************
*																	*
*	Forward References in This Module					*
*																	*
***************************************************/


static void print_error ( char *msg )
{
	if ( msg )
		printf ( "%s\n", msg );
}

static int clear_palette ( mgrec *db_rec )
{
	int numtxtrs, i;
	int *txtrindices;		/* Texture index list */
	char	patname[256];

		/* Get a count of the textures in the palette */
	numtxtrs = mgGetTextureCount ( db_rec );

		/* Allocate a texture index list */
	txtrindices = ( int *) malloc ( numtxtrs * sizeof ( int ) );
	if ( !txtrindices )
		return ( mgFALSE );

		/* Collect the texture indices */
	i = 0;
	mgGetFirstTextureInPalette ( db_rec, &txtrindices[i], patname );
	while ( mgGetNextTextureInPalette ( db_rec, &txtrindices[i], patname ) )
		i++;

		/* Delete each texture by index */
	for ( i = 0; i < numtxtrs; i++ )
	{
		mgDeleteTexture ( db_rec, txtrindices[i] );
	}

		/* Free the index list */
	free ( txtrindices );

	return ( mgTRUE );
}

void main (int argc, char **argv)
{
	mgrec *db_rec;
	char *siteid, *errortxt;

	if ( argc < 4 )
	{
		printf ( "%s databasefile inpalettefile outpalettefile [-r]\n", argv[0] );
		exit ( 0 );
	}

		/* Initialize the API */
	mgInit ( &argc, argv );

		/* Load the database */
	if ( !( db_rec = mgOpenDb ( argv[1] ) ) )
	{
		mgGetError ( &siteid, &errortxt );
		printf ( "%s", errortxt );
		exit( EXIT_FAILURE );
	}

		/* Check for the replace palette option */
	if ( ( argc > 4 ) && ( strcmp( argv[4], "-r" ) == 0 ) )
	{
		if ( !clear_palette ( db_rec ) )
		{
			print_error ( "Failed to clear Palette" );
			exit ( EXIT_FAILURE );
		}
	}

		/* Load the texture palette file */
	if ( !mgReadTexturePalette ( db_rec, argv[2] ) )
	{
		print_error ( "Texture palette file read failed" );
		exit ( EXIT_FAILURE );		
	}

		/* Write the database's texture palette to a file */
	if ( !mgWriteTexturePalette ( db_rec, argv[3] ) )
	{
		print_error ( "Texture palette file write failed" );
		exit ( EXIT_FAILURE );		
	}

		/* write the database */
	if ( !mgWriteDb ( db_rec ) )
	{
		print_error ( "Database write failed" );
		exit ( EXIT_FAILURE );
	}

		/* close the database */
	mgCloseDb ( db_rec );
}
