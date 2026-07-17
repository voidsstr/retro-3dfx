/******************************************************************************
	File:		CAsyncLogger.h
	Copyright:	© 1995-99 Critical Path Software Inc.  All Rights Reserved.
	Author:		Andrew Mellinger
	Purpose:	
	History:	
 ******************************************************************************/
#pragma once

	/* Headers */
/******************************************************************************/
#include <Files.h>
#include <OSUtils.h>

	/* Constants */
	/* Typedefs */
	/* Globals */
	/* Classes */
	/* Prototypes */
/******************************************************************************/
OSErr	InitAsyncLogger(StringPtr fileName);
void	TerminateAsyncLogger();
OSErr	WriteToAsyncLog(char *data, UInt32 size);


	/* Macros */
/******************************************************************************/
/*----------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
