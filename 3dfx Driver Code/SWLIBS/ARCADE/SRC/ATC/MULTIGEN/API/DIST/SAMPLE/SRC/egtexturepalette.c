/*******************************************************************************
 * 
 * $Header: egtexturepalette.c, 4, 10/11/00 7:33:09 PM, Brent$
 * $Revision: 4$
 * $Author: Brent$
 * $Date: 10/11/00 7:33:09 PM$
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

	Example program for Texture Palette functions.


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

void main (int argc, char **argv)
{
	int newindex;
	mgrec *db_rec1, *db_rec2;
	char *siteid, *errortxt;

	if ( argc < 4 )
	{
		printf ( "%s databasefile1 databasefile2 texturefile\n", argv[0] );
		exit ( 0 );
	}

		/* Initialize the API */
	mgInit ( &argc, argv );

		/* Load the databases */
	if ( !( db_rec1 = mgOpenDb ( argv[1] ) ) )
	{
		mgGetError ( &siteid, &errortxt );
		printf ( "%s", errortxt );
		exit( EXIT_FAILURE );
	}
	if ( !( db_rec2 = mgOpenDb ( argv[2] ) ) )
	{
		mgGetError ( &siteid, &errortxt );
		printf ( "%s", errortxt );
		exit( EXIT_FAILURE );
	}

		/* Read a new texture into the first database's palette */
	newindex = mgInsertTexture ( db_rec1, argv[3] );

	printf ( "Texture %s added to Database %s at index %d\n",
					argv[3], argv[1], newindex );

		/* Copy the new texture into the second database's palette */
	newindex = mgCopyTexture ( db_rec2, db_rec1, argv[3], newindex );

	printf ( "Texture %s added to Database %s at index %d\n",
					argv[3], argv[2], newindex );

			/* write the databases */
	if ( !mgWriteDb ( db_rec1 ) )
	{
		print_error ( "Database write failed" );
		exit ( EXIT_FAILURE );
	}
	if ( !mgWriteDb ( db_rec2 ) )
	{
		print_error ( "Database write failed" );
		exit ( EXIT_FAILURE );
	}

		/* Close the databases */
	mgCloseDb ( db_rec1 );
	mgCloseDb ( db_rec2 );
}
