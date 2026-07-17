/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
**
** $Revision: 2$ 
** $Date: 10/11/00 7:55:03 PM$ 
**
*/

#define MANVERSION              2
#define MANREVISION             6

/* yes this is hoopti, will add automation later */
#define BUILD_NUMBER            0

#define DESCRIPTIONSTR "3Dfx Interactive, Inc. OpenGL DLL\0"
#ifndef DEBUG
#  define VERSIONSTR   "Beta 3.00 (RELEASE)\0"
#else
#  define VERSIONSTR   "Beta 3.00 (DEBUG)\0"
#endif
#define VERSIONNAME    "3dfxogl.dll\0"
#define PRODNAME       "OpenGL\0"
#define PRODVER        "1.1\0"
