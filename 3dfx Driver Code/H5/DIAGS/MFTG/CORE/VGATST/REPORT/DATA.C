//
//		DATA.CPP - Reporting device specific data definitions for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/17/98
//		Last Modified:	5/4/98
//
#include	<stdio.h>
#define	_DATAFILE_
#include "ediag.h"
#include	"repdev.h"

WORD		_selFlatDescriptor = 0;
BOOL		_bMonoOpen = FALSE;
BOOL		_bSerialOpen = FALSE;

DWORD		_linMono = 0;
BYTE far	*_lpMono = NULL;
int		_nMonoCol = 0;
int		_nMonoRow = 0;

int		_nSerialPort = 0;
//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
