/* -*-c++-*- */
/* $Header: infoapi.c, 10, 10/11/00 8:51:22 PM, Brent$ */
/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** File name:   infoapi.c
**
** Description: Implements calls to driver to retreive information.
**
** $Revision: 10$
** $Date: 10/11/00 8:51:22 PM$
**
** $Log: 
**  10   3dfx      1.8.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  9    3dfx      1.8         03/16/00 Reid Campbell   Exchanged board name for
**       chip name
**  8    3dfx      1.7         03/14/00 Kaymann Woo     Added the string variable
**       OemBoardName.
** 
**       In InfoAPIGetProperty, called the miniVDD to get the Oem Board Name and
**       returned the name in the InfoArray.
** 
**  7    3dfx      1.6         02/23/00 Dan O'Connel    Minor clean up of DFP code
**       to prepare for future work.
**  6    3dfx      1.5         02/11/00 Reid Campbell   Total memory size of board
**       now written out based on memory per chip and number of chips.
**  5    3dfx      1.4         02/08/00 Reid Campbell   Changed so it does not use
**       the Subsystem id to determine card details
**  4    3dfx      1.3         12/07/99 Reid Campbell   Corrected missing memory
**       size write to registry.
**  3    3dfx      1.2         12/01/99 Reid Campbell   Writes out the number chip
**       on a card for display by the info page
**  2    3dfx      1.1         09/22/99 Reid Campbell   Updated to use the
**       V3_Board_Description table correctly
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $ 
*/


#include "infoapi.h"

#define _TEXT(x) x

#include "edgedefs.h"



// driver specific includes below 
#include "header.h"

#define Not_VxD
#include "minivdd.h"

#define Not_VxD
#include <vmm.h>

#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)
#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)

/* was including the h4....h files but that cause too many errors */
extern FxU32 *h4oempllTable;
extern FxU32 *h4pllTable;
extern FxU32 *pllTable;
#define	FREF	143184   //Fref*10000 
#define MIN_PLL_FREQ	30


#define SST_VENDOR_DEVICE_ID_H3			(0x0003121aL)
#define SST_VENDOR_DEVICE_ID_H4_OEM		(0x0004121aL)
#define PCI_3DFX_ID						(0x0000121aL)
#define STB_SSVENDOR_ID					(0x000010b4L)
#define TDFX_SSVENDOR_ID				(0x0000121aL)

#include "qmodes.h"	/* for LPQIN Structure */
#include "tv.h"
#include <string.h>


#include <pci.h>

#include <stdlib.h>	// for definition of _ltoa(). the dead cafe society
#include "dfpapi.h"

extern DISPLAYINFO DisplayInfo;
extern HDC hdc;

extern DWORD PLL2MHz(DWORD clock ); // This function is now in funcapi.c which should always be available


// bunged this in to get structure working
typedef struct   // this needs to move, but not today
{
	FxU8 busTypeIsAGP;    // 'A' == AGP, \000 == PCI
	FxU8 ramTypeIsSGRAM;  // 'G' == SGRAM, \000 == SDRAM
	FxU16 speed;          // in motorcycles per second
	FxU8 hasTVOUT;        // BOOL
	FxU8 hasLCD;          // BOOL
	FxU8 biosSize;        // in KB
}  V3_BOARD_DESCRIPTION;

extern V3_BOARD_DESCRIPTION FAR V3_Board_Description[];
#define	MAX_STR_LEN	5

#define PCI_SSVID	0x2C	//Subsystem Vendor ID
#define PCI_VID		0x00	//Vendor ID
#define BUS_MASK	0x0001
#define	TVO_MASK	0x0002
#define LCD_MASK	0x0004
#define MEM_MASK	0x0008



typedef	struct {
	char	Bus[MAX_STR_LEN];
	WORD	TvOut;
	WORD	LCD;
	char	MemType[MAX_STR_LEN];
}	BIOSInfo;


int ProcessBios();
DWORD	PLL2MHz(DWORD  clock );

int InfoAPIGetProperty(void * pInData,void * pOutData)
{
	switch (((STB_PROPERTY *)pInData)->ulPropertyId)
	{
		case (STB_GETDRIVER_INFO):
		{
			/*
				Must get following information from Driver :
				Card Name :
				Bios Version :
				Driver Version :
				RAM Size :
				RAMDAC Type :
				Clock Speed
				Memory Clock Speed:
			*/
			DWORD   grxFreq, memFreq, grxClock, memClock, dwVendorID, dwDeviceID;
/*         DWORD   dwSubVendorID; */
			DWORD	DevNode = _FF(DevNode);			
			typedef struct _edgeinfo
			{
				char	InfoName[30];
				char	InfoValue[30];
			}edgeinfo;
			
			int i;
			int iStart = 4;
			edgeinfo InfoArray[11];
			QIN				Qin;			// used to access functions in qmodes.h


			QGETBIOSVERSION BiosVersion;
			//QGETOEMBOARDNAME OemBoardName;
           QGETCHIPNAME ChipName;
			char buff1[30];
			//BIOSInfo SubSysInfo;
			char BaseKey[] = STB_DRIVER_REG_SHELLEX;


			/* 
				TODO :

				get card info and fill array 
				Set info size to number of elements in array (no of pieces of information)
			*/

			/*
				Get Vendor ID to check that there is a 3Dfx chip set and its revision is 
				greater than or equal to 3. Ensures that we only get information for cards
				from the Banshee up.

				dwVendorID lower 2 bytes contain the VendorID, the upper 2 bytes contain the 
				device ID.
			*/

			CM_Call_Enumerator_Function( _FF(DevNode),
				     PCI_ENUM_FUNC_GET_DEVICE_INFO,
				     PCI_VID, &dwVendorID, sizeof(DWORD), 0 );

			dwDeviceID = dwVendorID>>16;

			if(dwDeviceID < 3)
			{
				((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_NON_3DFX_CARD;
				return (STB_NON_3DFX_CARD);
			}

			dwVendorID &= 0x0000ffff; 

			if (! ((dwVendorID == STB_SSVENDOR_ID) || (dwVendorID == PCI_3DFX_ID)) )
			{
				((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_NON_3DFX_CARD;
				return (STB_NON_3DFX_CARD);
			}

			/*
				Get SubVendor ID to ensure that it is a 3Dfx or STB board
			*/

			/* Remove this check as we may do different Subvendor IDs in the future 
			   for special orders */
              
 /*
	           
			CM_Call_Enumerator_Function( _FF(DevNode),
				     PCI_ENUM_FUNC_GET_DEVICE_INFO,
				     PCI_SSVID, &dwSubVendorID, sizeof(DWORD), 0 );


			dwSubVendorID &= 0x0000ffff; 
			if (!(	( dwSubVendorID == STB_SSVENDOR_ID ) || 
					( dwSubVendorID == TDFX_SSVENDOR_ID ) ||
					( dwSubVendorID == 0xFFFF )
				) )

			{
				// this is not one of our cards
				((STB_PROPERTY *)pOutData)->ulResult = (DWORD)STB_NON_3DFX_CARD; 
				return (STB_NON_3DFX_CARD);
			}

	*/

	//BIOS Version

	// Get BIOS version string.

			/* remember to the check devtable to see if this can be done from there */
	        
			iStart = 0;

//--------------------------------------------------------------

   		if (!IS_VOODOO3)
   		{
				/* Get Chip Name  - only Napalm */

		  		Qin.dwSubFunc = QUERYGETCHIPNAME;
		 		QueryMode((LPQIN)&Qin, (LPVOID)&ChipName);

		  		lstrcpy(InfoArray[iStart].InfoName, "Chip Series");
		  		lstrcpy(InfoArray[iStart].InfoValue, (LPCSTR)&ChipName);
	        
       		iStart++;
           }
		
//--------------------------------------------------------------
	         
			lstrcpy(InfoArray[iStart].InfoName, "Bus Type");
           
			if (_FF(AGPCaps & IS_AGP_CARD)) 
				lstrcpy(InfoArray[iStart].InfoValue,(LPCSTR)"AGP");
			else 
				lstrcpy(InfoArray[iStart].InfoValue,(LPCSTR)"PCI");
	        
	        iStart++;    
	        
	        /* TV support ? */    

			lstrcpy(InfoArray[iStart].InfoName, STB_REG_TVOUT_SUPPORT);
			
			if (_FF(dwTvoCapable))	/* Only write out if TV is supported */
			{
				lstrcpy(InfoArray[iStart].InfoValue, STB_REG_SUPPORTED);
               iStart++; 
			}

			/* LCD Capabilities ? */
		
			lstrcpy(InfoArray[iStart].InfoName, STB_REG_LCD_SUPPORT);

			if (DFPisPanelPresent()) /* Only write out if LCD is supported */
			{
				lstrcpy(InfoArray[iStart].InfoValue, STB_REG_SUPPORTED);
               iStart++;
           }
			
	        /* Memory Type ? */  
           
           /* Remove the memory type functionality - may be added back later when call available 
              to Mini VDD to get BiosBoardConfigInfo data 
				

			lstrcpy(InfoArray[iStart].InfoName,"Memory Type");
			if (V3_Board_Description[((_FF(SSID) >> 16L) & ~CARD_TABLESIZEMASK) % CARD_TABLEMODSIZE].ramTypeIsSGRAM)
				lstrcpy(InfoArray[iStart].InfoValue,(LPCSTR)"SGRAM");
			else
				lstrcpy(InfoArray[iStart].InfoValue,(LPCSTR)"SDRAM");
	        
	        iStart++;    
           
           /*


			/* Amount of ram on card ? */
			
			_ltoa((_FF(TotalVRAM)*_FF(dwNumUnits)/1048576),(char *)&buff1,10);

			lstrcat((LPSTR)&buff1," MB");
			lstrcpy((InfoArray[iStart].InfoName),"Total Video Memory");
			lstrcpy((InfoArray[iStart].InfoValue),(LPCSTR)&buff1);
           
           iStart++;
           
           /* Number of graphic chips ? */
			
			_ltoa((_FF(dwNumUnits)),(char *)&buff1,10);

			lstrcpy((InfoArray[iStart].InfoName),"Graphic Chips");
			lstrcpy((InfoArray[iStart].InfoValue),(LPCSTR)&buff1);
			
	        iStart++;

			/* Speed of gfx Clock ? */
						
			grxClock = GET(lph3IORegs->pllCtrl1);
	         
	        if(dwDeviceID < 4)
			{
			    memClock = GET(lph3IORegs->pllCtrl2);	
	        }
	        else
	        {
	            memClock = GET(lph3IORegs->pllCtrl1); 
	        }

			memFreq = PLL2MHz((DWORD)memClock) / 10000;
			grxFreq = PLL2MHz((DWORD)grxClock) / 10000;		
			
			_ltoa(grxFreq, (char *)&buff1, 10);
			lstrcat((LPSTR)&buff1, " MHz");

			lstrcpy(InfoArray[iStart].InfoName, "Graphics Clock Speed");
			lstrcpy(InfoArray[iStart].InfoValue, (LPCSTR)&buff1);
	        
	       iStart++;
			

			/* Get Speed of Memory Clock */
			
	/* Memory clock speed for Voodoo3 and Napalm is the same as core speed */

	/*			_ltoa(memFreq, (char *)&buff1, 10);
			lstrcat((LPSTR)&buff1, " MHz");

			lstrcpy(InfoArray[iStart].InfoName, "Memory Clock Speed");
			lstrcpy(InfoArray[iStart].InfoValue, (LPCSTR)&buff1);
	        
	       iStart++;
	*/

			/* BIOS version ? */
	        
		  	Qin.dwSubFunc = QUERYGETBIOSVERSION;
		 	QueryMode((LPQIN)&Qin, (LPVOID)&BiosVersion);

		  	lstrcpy(InfoArray[iStart].InfoName, "BIOS Version");
		  	lstrcpy(InfoArray[iStart].InfoValue, (LPCSTR)&BiosVersion);
	        
	       iStart++;
	    
				
			/* fill in registry path */

			for (i=0 ; i < iStart ; i++)
			{
				/* actually write structure to edgetools area in registry */

				CM_Write_Registry_Value(DevNode, EDGE_DRIVER_REG_EDGEINFO_ADAPTER, (PFARCHAR) &(InfoArray[i].InfoName), 
										REG_SZ, (PFARVOID) &(InfoArray[i].InfoValue), sizeof(InfoArray[i].InfoValue), 
										CM_REGISTRY_SOFTWARE);

			}


			((STB_GROUPPROPERTY *)pOutData)->Guid =  ((STB_GROUPPROPERTY *)pInData)->Guid;
			((STB_GROUPPROPERTY *)pOutData)->ulPropertyId =  ((STB_GROUPPROPERTY *)pInData)->ulPropertyId;
			((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_ESCAPE_HANDLED;
				return(STB_ESCAPE_HANDLED);

			/* end of case(STB_GETDRIVER_INFO) */
		}	
		default:
		{
			((STB_GROUPPROPERTY *)pOutData)->Guid =  ((STB_GROUPPROPERTY *)pInData)->Guid;
			((STB_GROUPPROPERTY *)pOutData)->ulPropertyId =  ((STB_GROUPPROPERTY *)pInData)->ulPropertyId;
			((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_ESCAPE_NOT_SUPPORTED;
			return(STB_ESCAPE_NOT_SUPPORTED);
		}
    }
}


int ProcessBios(BIOSInfo	*SubSysInfo)
{

	unsigned long	SubSysID;
	DWORD			dwDevID;

	CM_Call_Enumerator_Function( _FF(DevNode),
				     PCI_ENUM_FUNC_GET_DEVICE_INFO,
				     PCI_SSVID, &dwDevID, sizeof(DWORD), 0 );

	if (dwDevID & STB_SSVENDOR_ID)
		return 1;
 
	SubSysID=dwDevID>>16;


	if(SubSysID)	{

	
		//Defaults

		lstrcpy(SubSysInfo->Bus, "AGP");	
		SubSysInfo->TvOut=0;
		SubSysInfo->LCD=0;
		lstrcpy(SubSysInfo->MemType, "SDRAM");

		if(SubSysID & BUS_MASK)	lstrcpy(SubSysInfo->Bus, "PCI");
		if(SubSysID & TVO_MASK)	SubSysInfo->TvOut=1;
		if(SubSysID & LCD_MASK)	SubSysInfo->LCD=1;
		if(SubSysID & MEM_MASK)	lstrcpy(SubSysInfo->MemType, "SGRAM");

		return 0; // good
	}

	else	{
		return 1; //error
	}
	


}




