/* -*-c++-*- */
/* $Header:
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File name:   pci.h
**
** Description: Routines to Read/Write PCI for Napalm SLI.  These are neccessary
** becauses the OS provided functions to not allow one to specific function number.
** Also if the device is hidden as is our case when <Multi-Function Bit is zero>
** these is no devnode.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:53 PM$
**
** $History: pci.h $
** 
** *****************  Version 1  *****************
** User: Andrew       Date: 6/25/99    Time: 10:04a
** Created in $/devel/h5/Win9x/dx/minivdd
** Defines for PCI
** 
**
*/
#ifndef _PCI_H_
#define _PCI_H_

#define CONFIG_ADDRESS_PORT (0xCF8)
#define CONFIG_DATA_PORT    (0xCFC)
#define CONFIG_ADDRESS_ENABLE_BIT (0x80000000l)

#define CONFIG_MAPPING_ENABLE_BYTE (0x80)
#define CONFIG_MAPPING_DISABLE_BYTE (0x00)

#define CONFIG_SPACE_ENABLE_PORT (0xCF8)
#define FORWARD_REGISTER_PORT    (0xCFA)
#define CONFIG_MAPPING_OFFSET    (0xC000)


#endif
