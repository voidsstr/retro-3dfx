/*******************************************************************************
 * 
 * $Header: printtexturestats.c, 4, 10/11/00 7:33:14 PM, Brent$
 * $Revision: 4$
 * $Author: Brent$
 * $Date: 10/11/00 7:33:14 PM$
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

	Example program for Texture Palette Statistics functions.


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


void main (int argc, char **argv)
{
	int type, width, height, mem_size, count;
	int patindex;
	mgrec attr_rec, *db_rec;
	char	patname[256];
	char *siteid, *errortxt;

	if ( argc < 2 )
	{
		printf ( "%s databasefile\n", argv[0] );
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

		/* Walk the texture palette & print the width, height, type &
			memory usage of each texture */
	if (  mgGetFirstTextureInPalette (db_rec, &patindex, patname) )
	{
		do
		{
			if ( mgGetTextureAttributes ( db_rec, patindex, &attr_rec ) )
			{
				mgGetAttList ( &attr_rec, fltImgWidth, &width,
								fltImgHeight, &height,
								fltImgType, &type,
								mgNULL );
				mem_size = mgGetTextureSize ( db_rec, patindex );

				if ( mgIsTextureDefault ( db_rec, patindex ) )
					printf ( "Texture %d (DEFAULT TEXTURE): %s: width = %d, height = %d, numChannels = %d, size = %d bytes\n",
								patindex, patname, width, height, (type-1), mem_size );
				else
					printf ( "Texture %d: %s: width = %d, height = %d, numChannels = %d, size = %d bytes\n",
								patindex, patname, width, height, (type-1), mem_size );
			}
			else
			{
				printf ( "Texture %d: %s: Error: cannot get attributes\n", patindex, patname );
			}
		} while ( mgGetNextTextureInPalette (db_rec, &patindex, patname) );
	}

		/* Get the total texture count & memory usage */
	count = mgGetTextureCount ( db_rec );
	mem_size = mgGetTextureTotalSize ( db_rec );

	printf ( "\n" );
	printf ( "Total %d textures using %d bytes\n", count, mem_size );

		/* Close the database */
	mgCloseDb ( db_rec );
}
