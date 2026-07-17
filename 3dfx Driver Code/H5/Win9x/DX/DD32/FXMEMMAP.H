/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 8:52:10 PM$
*/

/*
 * DeviceIoControl service table control codes for communication with
 * fxmemmap vxd. The codes here must match those in fxmemmap.asm.
 * ESPECIALLY fxmajor_ver and fx_minor_ver!!!
 */

#define   DIOC_CLOSEHANDLE     -1     
#define   DIOC_GETVERSIONDWORD  0   
#define   GETAPPVERSIONDWORD    1   
#define   GETLINEARADDR         2   
#define   GETLDTSELECTOR        3
#define   FREELDTSELECTOR       4
#define   GETLINEARADDR_AUTO    5 /* Watches for & handles PASSTHRU,
                                     Otherwise identical to 2 */
#define   GETMSR                6 /* Get the contents of an MSR */
#define   SETMSR                7
#define   DECREMENTMUTEX        8

#define   FX_MAJOR_VER          1
#define   FX_MINOR_VER          2


/* FxMemmap Int 2F API Codes */

#define   PROCMAPPHYS           1
#define   PROCGETMSR            2
#define   PROCSETMSR            3
#define   PROCGETVERSION        4
#define   PROCUNMAPPHYS         5
#define   PROCMAPPHYS16         6   /* NOT YET IMPLEMENTED */
#define   PROCGETMSR16          7   
#define   PROCSETMSR16          8   
#define   PROCGETVERSION16      9
#define   PROCUNMAPPHYS16       10  /* NOT YET IMPLEMENTED */
#define   PROCINCREMENTMUTEX16  11
#define   PROCDECREMENTMUTEX16  12
