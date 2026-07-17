/* $Header: monitor.h, 3, 10/11/00 8:51:31 PM, Brent$ */
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
** File name:   monitor.h
**
** Description: Structures, macros, etc for monitor support.
**
** $Revision: 3$ 
** $Date: 10/11/00 8:51:31 PM$
**
** $History: monitor.h $
** 
** *****************  Version 4  *****************
** User: Michael      Date: 12/29/98   Time: 2:36p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** 3     8/26/98 12:26a Andrew
** Added MIN, MAX macros
** 
** 2     5/21/98 5:38p Andrew
** Added another string to check to see if monitor changes since sometimes
** HardwareID not updated <particular when monitor is set to Laptop>.
** 
** 1     4/22/98 2:46p Andrew
** Monitor structures
** 
** 
*/
#ifndef _MONITOR_H_
#define _MONITOR_H_

#define RANGE(val,lo,hi) ((lo) <= (val) && (val) <= (hi))
#define MIN(A,B) (((A) < (B)) ? (A) : (B))
#define MAX(A,B) (((A) > (B)) ? (A) : (B))

#define MONINFO_REGISTRY 0x80				/* Information was taken from Registry */
#define MONINFO_EDID		 0x81				/* Information was taken from EDID */

typedef struct range {
   WORD nMin;                          /* Minumum Value */
   WORD nMax;                          /* Maximum Value */
   } RANGE, * PRANGE, FAR * LPRANGE;

typedef struct  montype {
   struct montype FAR * lpNext;			   /* Pointer to Next Entry */
	WORD nType;						            /* Type of Entry */
   WORD IsValid;								   /* Is Mode Valid */	
   WORD nResX;                            /* Maximum suggested X resolution    */
   WORD nResY;                            /* Maximum suggested Y resolution    */
} MONTYPE, * PMONTYPE, FAR * LPMONTYPE;

typedef struct moninfo {
  struct     moninfo FAR * lpNext;			 /* Pointer to Next Entry */
  WORD	 	 nType;							    /* Type of Entry */
  WORD	    IsValid;						    /* Is Mode Valid */	
  WORD       nResX;                        /* Maximum suggested X resolution    */
  WORD       nResY;                        /* Maximum suggested Y resolution    */
  RANGE      rHort;                       /* Horizontal range values           */
  RANGE      rVert;                        /* Vertical range values             */
} MONINFO, * PMONINFO, FAR * LPMONINFO;

typedef struct monedid {         
  LPMONINFO lpNext;			            /* Pointer to Next Entry */
  WORD	 	nType;							/* Type of Entry */
  WORD	   IsValid;						   /* Is Mode Valid */	
  WORD      nHor;                      /* Horizontal Active Pixels */
  WORD      nVer;                      /* Vertical Active Lines */
  WORD		nVRef;							/* Vertical Refresh Rate */
} MONEDID, * PMONEDID, FAR * LPMONEDID;

#define MONITOR_UNKNOWN					0x00	/* Monitor Unknown */
#define MONITOR_FIXED_FREQUENCY		0x01	/* Fixed Frequency Monitor */
#define MONITOR_MULTI_SYNC				0x02	/* Multi-Sync Monitor */

#define DISPLAY_INFO_VALID   (0x0001)			/* Display Info is Valid */
#define MAX_REZ_IS_VALID     (0x0002)			/* Can Parse on Max Resolution */
#define REFRESH_IS_VALID     (0x0004)			 /* Can Parse on Min/Max Refresh */

#define ALL_VALID (DISPLAY_INFO_VALID | MAX_REZ_IS_VALID | REFRESH_IS_VALID)
#define REZ_VALID (DISPLAY_INFO_VALID | MAX_REZ_IS_VALID)

typedef struct monitor {
   LPMONTYPE lpNext;                      /* Pointer to First Entry */
   WORD nType;                           /* Type of Monitor <see above> */
   WORD IsValid;                         /* Valid Flags */
   char  szHardwareID[32];                /* ID to suppress parse of data */
   char  szDeviceID[64];                  /* Device Desciption ID */
   } MONITOR;                             /* Sometimes HardwareID is not updated <use Laptop> */

// ref_Data and MAX_REF_DATA moved into monitor.h for use in mode.c jmccartney 22/05/00
typedef struct ref_data {
	WORD nHor;			/* Horizontal Pixels */
	WORD nVer;			/* Vertical Lines */
	WORD nRef;			/* Max Vertical Refresh */
	} REF_DATA, * PREF_DATA, FAR * LPREF_DATA;

#define MAX_REF_DATA 49

#endif
