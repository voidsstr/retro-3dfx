/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:44:23 PM$
**
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 1     10/01/98 6:15p Russ
*/

#ifndef _LOGFILE_H_
#define _LOGFILE_H_

/***************************************************************************
* D E F I N E S
****************************************************************************/

#ifndef ENABLE_LOG_FILE
#define ENABLE_LOG_FILE       0
#endif

/***************************************************************************
* M A C R O S
****************************************************************************/

#if ENABLE_LOG_FILE
#define H3PRINTF(n)     h3printf n
#else
#define H3PRINTF(n)
#endif

#if ENABLE_LOG_FILE
#define FLUSH_LOG_FILE_BUFFER(ppdev)    FlushLogFileBuffer((ppdev))
#else
#define FLUSH_LOG_FILE_BUFFER(ppdev)
#endif

/***************************************************************************
* F U N C T I O N   P R O T O T Y P E S
****************************************************************************/

#if ENABLE_LOG_FILE
typedef struct _PDEV PDEV;      // Handy forward declaration

extern VOID FlushLogFileBuffer ( PDEV * );
extern VOID h3printf(PDEV *, LPSTR pszFormat, ...);
#endif

#endif /* _LOGFILE_H_ */
/* Don't write below this endif */

