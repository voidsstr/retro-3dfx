/*
** Copyright (c) 1996-2000, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   data.c
**
** Description: Holds the global data for CSCLIENT.LIB      
**
** $Log: 
**  12   Napalm R1.51.8.1.2     10/11/00 Brent           Forced check in to enforce
**       branch.
**  11   Napalm R1.51.8.1.1     07/20/00 Andrew  Bell    Rolling back all swlibs
**       files to 2nd latest version due to an incorrect checkin.  This should
**       bring things back to normal.
**  10   Napalm R1.51.8.1.0     07/19/00 Dinesh Raja Savari Amirtharaj CSIM for the
**       RTL release_3_2
**  9    3dfx      1.8         03/02/00 Don Fowler      Incremental development
**  8    3dfx      1.7         03/01/00 Don Fowler      Incremental development
**  7    3dfx      1.6         02/29/00 Don Fowler      Incremental development
**  6    3dfx      1.5         02/28/00 Don Fowler      Incremental development
**  5    3dfx      1.4         02/28/00 Don Fowler      Incremental development
**  4    3dfx      1.3         02/27/00 Don Fowler      Incremental development
** 
**  3    3dfx      1.2         02/27/00 Don Fowler      Incremental development
**  2    3dfx      1.1         02/27/00 Don Fowler      Incremental Development
** 
**  1    3dfx      1.0         02/27/00 Don Fowler      
** $
**
*/

#include "csclient.h"

/* Libary-specific data for CSCLIENT.LIB */
CSCLIENTLIBDATA _sCSClientLibData = 
{
    0x00,
#ifdef WIN32
    0x00,
    NULL
#endif /* WIN32 */
};


