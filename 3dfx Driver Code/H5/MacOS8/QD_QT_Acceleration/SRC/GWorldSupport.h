/******************************************************************************
	File:		
	Copyright:	© 1999  3Dfx Interactive, Inc.  All Rights Reserved.
	Author:		Andrew Mellinger
	Purpose:	
	History:	
 ******************************************************************************/
#pragma once

	/* Headers */
/******************************************************************************/
#include "NQDAcceleration.h"
#include "Utilities.h"

	/* Constants */
	/* Typedefs */
/******************************************************************************/
typedef struct
{
	QElemPtr	qLink;
	h3Info *	h3InfoDst;
	mmBlock_t *	blockPt;
	UInt32		buffer;
	UInt32		rowBytes;
} GWorldTracker;

	/* Globals */
	/* Classes */
	/* Prototypes */
/******************************************************************************/
OSErr  InstallGWorldSupport(h3Info *h3InfoDst);
OSErr  UninstallGWorldSupport(h3Info *h3InfoDst);

	/* Macros */
/******************************************************************************/
/*----------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
