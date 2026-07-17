/*******************************************************************************
 * 
 * $Header: printtexturemappinginfo.c, 4, 10/11/00 7:33:13 PM, Brent$
 * $Revision: 4$
 * $Author: Brent$
 * $Date: 10/11/00 7:33:13 PM$
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

	Example program for Texture Mapping Palette functions.


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
#include "mgapitxtrmap.h"
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

static void print_matrix ( double matrix[4][4] )
{
	printf ( "\tmatrix = {\n" );
	printf ( "\t\t%f\t%f\t%f\t%f\n", matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3] );
	printf ( "\t\t%f\t%f\t%f\t%f\n", matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3] );
	printf ( "\t\t%f\t%f\t%f\t%f\n", matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3] );
	printf ( "\t\t%f\t%f\t%f\t%f\n", matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3] );
	printf ( "\t}\n" );
}

void main (int argc, char **argv)
{
	int type;
	int mapindex;
	mgrec *db_rec;
	char	mapname[256];
	char *siteid, *errortxt;
	double matrix[4][4];

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

		/* Walk the mapping palette & print the name & type of each mapping */
	if (  mgGetFirstTextureMappingInPalette (db_rec, &mapindex, mapname) )
	{
		do
		{
			switch ( type = mgGetTextureMappingType ( db_rec, mapindex ) )
			{
				case 1:
					if ( *mapname )
						printf ( "Texture Mapping %d: %s: type = 3 Point Put\n", mapindex, mapname );
					else
						printf ( "Texture Mapping %d: (no name): type = 3 Point Put\n", mapindex );
					if ( mgGetTextureMappingMatrix ( db_rec, mapindex, matrix ) )
						print_matrix ( matrix );
					else
						printf ( "ERROR - No Matrix\n" );
					break;
				case 2:
					if ( *mapname )
						printf ( "Texture Mapping %d: %s: type = 4 Point Put\n", mapindex, mapname );
					else
						printf ( "Texture Mapping %d: (no name): type = 4 Point Put\n", mapindex );
					if ( mgGetTextureMappingMatrix ( db_rec, mapindex, matrix ) )
						print_matrix ( matrix );
					else
						printf ( "ERROR - No Matrix\n" );
					break;
				case 4:
					if ( *mapname )
						printf ( "Texture Mapping %d: %s: type = Spherical Project\n", mapindex, mapname );
					else
						printf ( "Texture Mapping %d: (no name): type = Spherical Project\n", mapindex );
					break;
				case 5:
					if ( *mapname )
						printf ( "Texture Mapping %d: %s: type = Radial Project\n", mapindex, mapname );
					else
						printf ( "Texture Mapping %d: (no name): type = Radial Project\n", mapindex );
					break;
				default:
					if ( *mapname )
						printf ( "Texture Mapping %d: %s: ERROR - Unknown Type\n", mapindex, mapname );
					else
						printf ( "Texture Mapping %d: (no name): ERROR - Unknown Type\n", mapindex );
					break;
			}
		} while ( mgGetNextTextureMappingInPalette (db_rec, &mapindex, mapname) );
	}

		/* Close the database */
	mgCloseDb ( db_rec );
}
