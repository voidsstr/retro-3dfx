/* -*-c++-*- */
/* $Header: edgedefs.h, 9, 10/12/00 4:45:17 AM, Reid Campbell$ */
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
** File name:   edgedefs.h
**
** Description: Basic definition of edge tools types, guids and macros.
**
** $Revision: 9$
** $Date: 10/12/00 4:45:17 AM$
**
** $Log: 
**  9    3dfx      1.6.1.1     10/12/00 Reid Campbell   Added API version for 3dfx
**       Tools to check
**  8    3dfx      1.6.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  7    3dfx      1.6         02/08/00 Reid Campbell   Driver information now
**       written to Default/Info/Adapter area of the registry
**  6    3dfx      1.5         12/01/99 Dan O'Connel    Add a result code that
**       communicates back to 3dfx Tools that a mode change is necessary.
**  5    3dfx      1.4         12/01/99 Reid Campbell   Updated with new Phase II
**       defines
**  4    3dfx      1.3         10/22/99 Scott Kephart   
**  3    3dfx      1.2         10/01/99 Reid Campbell   Adds extra colour API call
**       to determine the colour functionality from the driver.
**  2    3dfx      1.1         09/22/99 Reid Campbell   Updated for Monitor API
**       changes
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 13    19/09/00 15:04 Pmagee
** Added define for FX_GLIDE_USE_APP_GAMMA, to support this tweak for the
** color page
** 
** 12    13/09/00 10:20 Ssaulters
** Added a ColorPage/DirectX namespace clash hack.
** 
** 11    00-08-11 11:48 Pmagee
** Added define for LEDGE_REG_W2K_GRX_CLOCK, to take care of the different
** name to the registry element in Windows 2000 to overclock.
** 
** 10    7/03/00 10:27 Cweir
** #define's added for the refresh events sent to/from the Advanced Tweaks
** and Feature pages.
** 
** 9     00-02-22 12:33 Gclarke
** Added CLSID and string for the hub
** 
** 8     14/02/00 14:56 Imccarty
** Provide names for the kernal event object to close the TaskMan from
** 3dfxInst.
** 
** 7     00-01-28 10:47 Gclarke
** Advanced Tweak regsitry name changed to Advanced Features
** 
** 6     27/01/00 12:22 Imccarty
** Change the location in the registry were the info page looks for extra
** info written in by the (napalm) driver.
** 
** 5     17/12/99 9:40 Cweir
** Definitions added for the 'Load task manager on startup' feature
** 
** 4     99-12-02 16:02 Gclarke
** strange error with comments in the header for this file
** 
** 3     99-12-02 16:01 Gclarke
** Common library has been updated to be a 3dfx application and exports
** the DllRegisterServer and DllUnregisterServer functions
// 
// 77    11/11/99 9:55 Cweir
// New define added for weblinks key in registry.
// 
// 76    10/11/99 16:20 Cweir
// New defines added for task manager key in registry.

/* ************************************************************** */
/*							edgedefs.h							  */
/* Common header file for Edge Tools definitions and declarations */
/* ************************************************************** */

#ifndef _EDGEDEFS_H
#define _EDGEDEFS_H


/* ************************************************************** */
/*		Registry related declarations and definitions			  */
/* ************************************************************** */

/* Registry keys */
#define STB_REG_BASE_KEY	  HKEY_LOCAL_MACHINE
#define STB_REG_3DFX_KEY	  _TEXT("SOFTWARE\\3dfx Interactive")
#define STB_REG_EDGE_KEY	  _TEXT("EdgeTools")
#define STB_REG_MAIN_KEY	  _TEXT("SOFTWARE\\3dfx Interactive\\EdgeTools")
#define STB_REG_INSTALLED_KEY _TEXT("SOFTWARE\\3dfx Interactive\\EdgeTools\\Installed")
#define STB_REG_RESOURCES_KEY _TEXT("SOFTWARE\\3dfx Interactive\\EdgeTools\\Resources")
#define STB_REG_TASKMAN_KEY	  _TEXT("SOFTWARE\\3dfx Interactive\\EdgeTools\\Task Manager")
#define STB_REG_WEBLINKS_KEY  _TEXT("SOFTWARE\\3dfx Interactive\\EdgeTools\\Task Manager\\Weblinks")
#define STB_REG_DISPLAY98     _TEXT("System\\CurrentControlSet\\Services\\Class\\Display\\")
#define STB_REG_DISPLAY95	  _TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\Display\\shellex\\PropertySheetHandlers\\")
#define STB_REG_SHELLEX		  _TEXT("\\shellex\\PropertySheetHandlers\\")
#define STB_REG_LCD			  _TEXT("Edge Digital Out")
#define STB_REG_TVOUT		  _TEXT("Edge TV Out")
#define STB_REG_TWEAKS		  _TEXT("Edge Tweaks")
#define STB_REG_INFO		  _TEXT("Edge Information")	
#define STB_REG_RESOURCE 	  _TEXT("Edge Tools Resource Control")
#define STB_REG_ADVTWEAK 	  _TEXT("3dfx Advanced Features")
#define STB_REG_OVERCLOCK 	  _TEXT("3dfx Overclocking Utility")	
#define STB_REG_ENABLED		  _TEXT("Enabled")
#define STB_REG_DISABLED	  _TEXT("Disabled")
#define STB_REG_EFFECT		  _TEXT("Effect")
#define STB_REG_TV_GUID		  _TEXT("SOFTWARE\\3dfx Interactive\\EdgeTools\\Installed\\{AB040305-8AA1-11D2-8DD1-00104BB5EAD6}")
#define STB_REG_DFP_GUID	  _TEXT("SOFTWARE\\3dfx Interactive\\EdgeTools\\Installed\\{AB040307-8AA1-11D2-8DD1-00104BB5EAD6}")
#define STB_REG_TVOUT_SUPPORT _TEXT("TV Out")		/* Used by InfoAPI.c to set value in registry for support */
#define STB_REG_LCD_SUPPORT	  _TEXT("LCD")
#define STB_REG_SUPPORTED	  _TEXT("Supported")
#define STB_REG_NOT_SUPPORTED _TEXT("Not Supported")
#define STB_REG_GAMMA_TABLE	  _TEXT("Edge Tweaks\\GammaTable")
#define STB_REG_DEFAULT_CLOCK _TEXT("GrxDefault")
#define STB_REG_GRX_CLOCK	  _TEXT("GrxClock")
#define STB_REG_CONFIRM_CLOCK _TEXT("ConfirmClk")
#define STB_REG_CHECK_CLOCK	  _TEXT("CheckClk")
#define STB_REG_MONCTRL		  _TEXT("3dfx Monitor Control")
#define STB_REG_COMPLETE_REG  _TEXT("CompleteRegistration")
#define STB_REG_COLOR		  _TEXT("3dfx Color")
#define STB_REG_GAMMA_TABLE	  _TEXT("Edge Tweaks\\GammaTable")
#define STB_REG_LOAD_TASKMAN  _TEXT("Load on startup")

/* The following defines are possible values of CompleteRegistration */
#define STB_REGISTRATION_UNCOMPLETE 0				/* Inidcates registration should not be completed */
#define STB_REGISTRATION_COMPLETE   1				/* Indicates registration should be completed */

// The following defines are used in page intercommunication
#define EDGE_QS_DISABLE 0xF0
#define EDGE_QS_CONFIRM 0xF1
#define	EDGE_QS_CANCEL  0xF2

/* The following is used to generate a unique Windows message */
#define STB_ENABLE_MSG_STR    _TEXT("STB Enable state change message")

/* Enumerations */
/* Reflection of Enabled registry value */
typedef enum _tagEnabledStates {
	Enabled  		= 0,
#ifdef COLORPAGE_HACK
#undef COLORPAGE_HACK
    _Disabled       = 1,
#else
	Disabled 		= 1,
#endif

	NotSupported 	= 2
} STB_TOOL_ENABLED;

/* Reflection of Effect registry value */
typedef enum _tagEffectValues {
	Global		= 0,
	Local		= 1
} STB_TOOL_EFFECT;

/* The following is used to generate a unique Windows message */
#define STB_ENABLE_MSG_STR    _TEXT("STB Enable state change message")
#define STB_REG_EDGE_KEY	  _TEXT("EdgeTools")

/*====================End of Depreciated section===========================*/

/* ************************************************************** */
/*		Definitions for Edge Tools module properties			  */
/* ************************************************************** */

#define STB_MAX_PAGE_WIDTH  253
#define STB_MAX_PAGE_HEIGHT 217

/* ************************************************************** */
/*		Registry related declarations and definitions			  */
/*		Note that string keys are declared in 2 places,			  */
/*		once for MBCS and once for UNICODE.						  */
/* ************************************************************** */

/* Registry keys */
/* Components */
#define EDGE_REG_RESOURCE 			_T("3dfxTools Resource Control")

/* Applications */
#define EDGE_REG_3DFXCMN			_T("3dfxTools Common Library")
#define EDGE_REG_HUB				_T("3dfxTools Hub")
#define EDGE_REG_TASKMAN			_T("3dfxTools Task Manager")

/* Tools */
#define EDGE_REG_LCD				_T("3dfxTools Digital Out")
#define EDGE_REG_TVOUT				_T("3dfxTools TV Out")
#define EDGE_REG_INFO				_T("3dfxTools Information")
#define EDGE_REG_ADVTWEAK 			_T("3dfxTools Advanced Features")
#define EDGE_REG_OVERCLOCK 			_T("3dfxTools Overclocking")
#define EDGE_REG_MONCTRL			_T("3dfxTools Monitor Control")
#define EDGE_REG_COLOR				_T("3dfxTools Color")
#define EDGE_REG_FEATURE			_T("3dfxTools Features")

/* The following defines relate to the overclocking page */
#define EDGE_REG_DEFAULT_CLOCK		_T("GrxDefault")
#define EDGE_REG_CHECK_CLOCK		_T("CheckClk")
#define EDGE_REG_CONFIRM_CLOCK		_T("ConfirmClk")
#define EDGE_REG_GRX_CLOCK			_T("GrxClock")

/* ************************************************************** */
/*		UNICODE variations of the above strings.				  */
/* ************************************************************** */

/* Components */
#define LEDGE_REG_RESOURCE 			L"3dfxTools Resource Control"

/* Applications */
#define LEDGE_REG_3DFXCMN			L"3dfxTools Common Library"
#define LEDGE_REG_HUB				L"3dfxTools Hub"
#define LEDGE_REG_TASKMAN			L"3dfxTools Task Manager"

/* Tools */
#define LEDGE_REG_LCD				L"3dfxTools Digital Out"
#define LEDGE_REG_TVOUT				L"3dfxTools TV Out"
#define LEDGE_REG_INFO				L"3dfxTools Information"	
#define LEDGE_REG_ADVTWEAK 			L"3dfxTools Advanced Features"
#define LEDGE_REG_OVERCLOCK 		L"3dfxTools Overclocking"	
#define LEDGE_REG_MONCTRL			L"3dfxTools Monitor Control"
#define LEDGE_REG_COLOR				L"3dfxTools Color"
#define LEDGE_REG_FEATURE			L"3dfxTools Features"

/* The following defines relate to the overclocking page */
#define LEDGE_REG_DEFAULT_CLOCK		L"GrxDefault"
#define LEDGE_REG_CHECK_CLOCK		L"CheckClk"
#define LEDGE_REG_CONFIRM_CLOCK		L"ConfirmClk"
#define LEDGE_REG_GRX_CLOCK			L"GrxClock"
#define LEDGE_REG_W2K_GRX_CLOCK		L"GraphicsClocking"

/* The following defines are for the color page */
#define LEDGE_REG_COLOR_OVERRIDE	L"FX_GLIDE_USE_APP_GAMMA"



/* The following are possible values of the 'ConfirmClk' */
#define EDGE_OC_USER_MUST_CONFIRM	1
#define EDGE_OC_RESET_TO_DEFAULT	2


/* The following defines kernal object names */
#define EDGE_DESTROY_EVENT_TASKMAN		_T("3dfxTools TaskMan Destroy Event")
#define EDGE_WAIT_FOR_DESTROY_TASKMAN	_T("3dfxTools TaskMan Wait For Destroy Event")
#define EDGE_ADVTWEAK_REFRESH_EVENT		_T("3dfxTools Advanced Tweaks Page Refresh Event")	// Sent from the Feature page to the Advanced Tweaks page
#define EDGE_FEATURE_REFRESH_EVENT		_T("3dfxTools Feature Page Refresh Event")	// Sent from Advanced Tweaks page to the Feature page

/* ************************************************************** */
/*					structure definitions						  */
/* ************************************************************** */

#ifndef _WINDEF_
typedef unsigned long	DWORD;
typedef unsigned short	WORD;
typedef unsigned char	BYTE;
#endif	// !_WINDEF_


#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID 
{          
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
} GUID;
#endif	

typedef struct _STB_PROPERTY
{
	GUID Guid;
	DWORD ulResult;
	DWORD ulPropertyId;
	DWORD ulPropertyValue;
} STB_PROPERTY;

typedef struct _STB_INTERFACESUPPORTED
{
	GUID Guid;
	DWORD ulResult;
} STB_INTERFACESUPPORTED;

typedef struct _STB_GROUPPROPERTY
{
	GUID Guid;
	DWORD ulResult;
	DWORD ulPropertyId;
	DWORD ulPropertySize;
	BYTE  ulPropertyValue[1024];
} STB_GROUPPROPERTY;

typedef struct _STB_GAMMA_TABLE
{       
	DWORD GammaTable[256];
} STB_GAMMA_TABLE;


// RECT dimensions for coordinates and size of spinning edge logo avi
#define STB_COORD_SPINNINGLOGORECT		{25,23,117,99}
// offset difference should be ...    + {18, 4, 18, 4} =
#define STB_COORD_SPINNINGLOGORECT_LCD	{43,27,135,103}

/* ************************************************************** */

/*				definitions of registry keys					  */
/*				used in driver only								  */

/* ************************************************************** */


#define STB_GAMMA_REGPATH				"\\3dfx Tools\\3dfxTools Color\\"			
/* Stores in the registry where gamma data for edge tools is stored
** Previously under HKEY_CURRENT_USER \EdgeTools\\Colour\\GammaTable
*/
#define	DESKTOPGAMMAPATH                "GammaTable"		// The subkey under which the desktop gamma table is stored
#define	OVERLAYGAMMAPATH			    "VideoOverlay"		// The subkey under which the video overlay gamma table is stored

#define STB_OLD_GAMMA_REGPATH			"\\shellex\\PropertySheetHandlers\\Edge Tweaks\\GammaTable"
/* This is where the gamma data was originally kept. This value is 
** kept here for compatibility between different versions of the driver
** and edge tools.
*/
// stores where in the registry that the information api places its adapter information

#define STB_DRIVER_REG_EDGEINFO_ROOT		_TEXT("shellex\\PropertySheetHandlers\\Edge Information\\Data")
#define STB_DRIVER_REG_EDGEINFO_ADAPTER		_TEXT("shellex\\PropertySheetHandlers\\Edge Information\\Data\\Adapter")
#define STB_DRIVER_REG_EDGEINFO_DRIVER		_TEXT("shellex\\PropertySheetHandlers\\Edge Information\\Data\\Driver")
#define EDGE_DRIVER_REG_EDGEINFO_ROOT		_TEXT("DEFAULT\\Info")
#define EDGE_DRIVER_REG_EDGEINFO_ADAPTER	_TEXT("DEFAULT\\Info\\Adapter")
#define EDGE_DRIVER_REG_EDGEINFO_DRIVER		_TEXT("DEFAULT\\Info\\Driver")

/* 
	different from STB_REG_SHELLEX because the CM_Write_Registry_Value takes the key
	without the preceding "\\"
*/
	
#define STB_DRIVER_REG_SHELLEX				"shellex\\PropertySheetHandlers\\"

#define EDGE_API_VERSION_REG				"3dfxToolsAPI" 	// Registry entry where the driver stores the 3dfxTools API version
															// This allows 3dfxTools to determine if driver and Tools are compatible  

#define EDGE_API_VERSION                    "0000"			// This version number should be increased if there could be a compatible
															// issue between the driver and Tools. 



/* ************************************************************** */

/*				definitions of escape functions					  */

/* ************************************************************** */

#define STB_ESCAPE_START				5000
#define	STB_GETPROPERTY					STB_ESCAPE_START
#define STB_SETPROPERTY					STB_ESCAPE_START + 1
#define	STB_QUERYINTERFACE				STB_ESCAPE_START + 2
#define STB_GETGROUPPROPERTY			STB_ESCAPE_START + 3
#define STB_SETGROUPPROPERTY			STB_ESCAPE_START + 4

/* ************************************************************** */
/*			definitions of error codes for escape calls			  */
/* ************************************************************** */

#define STB_ESCAPE_ERROR_START			0
#define STB_ESCAPE_NOT_SUPPORTED		STB_ESCAPE_ERROR_START
#define STB_INTERFACE_NOT_SUPPORTED		STB_ESCAPE_ERROR_START -1
#define STB_FEATURE_NOT_SUPPORTED		STB_ESCAPE_ERROR_START -2
#define STB_ERROR_SETTING_VALUE			STB_ESCAPE_ERROR_START -3
#define STB_NON_3DFX_CARD				STB_ESCAPE_ERROR_START -4

#define STB_ESCAPE_HANDLED				STB_ESCAPE_ERROR_START +1
#define STB_INTERFACE_SUPPORTED			STB_ESCAPE_ERROR_START
#define STB_CHANGE_MODE                 STB_ESCAPE_ERROR_START +2




/* ************************************************************** */
/*				definitions of STB Guids						  */
/* ************************************************************** */

#define STB_NULL_GUID		0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	/* used to denote the end of a guid structure */
#define STB_DUMMMY_GUID		0x11111111, 0x1111, 0x1111, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11	/* used to pad out arrays and structures */


#define GUID_STB_FUNCTION_API	0xab040400, 0x8aa1, 0x11d2, 0x8d, 0xd1, 0x0, 0x10, 0x4b, 0xb5, 0xea, 0xd6
#define	GUID_STB_NVIDIA_D3D_API	0xab040401, 0x8aa1, 0x11d2, 0x8d, 0xd1, 0x0, 0x10, 0x4b, 0xb5, 0xea, 0xd6
#define	GUID_STB_NVIDIA_OGL_API	0xab040402, 0x8aa1, 0x11d2, 0x8d, 0xd1, 0x0, 0x10, 0x4b, 0xb5, 0xea, 0xd6
#define	GUID_STB_3DFX_D3D_API	0xab040403, 0x8aa1, 0x11d2, 0x8d, 0xd1, 0x0, 0x10, 0x4b, 0xb5, 0xea, 0xd6
#define	GUID_STB_3DFX_OGL_API	0xab040404, 0x8aa1, 0x11d2, 0x8d, 0xd1, 0x0, 0x10, 0x4b, 0xb5, 0xea, 0xd6
#define	GUID_STB_DFP_API		0xab040405, 0x8aa1, 0x11d2, 0x8d, 0xd1, 0x0, 0x10, 0x4b, 0xb5, 0xea, 0xd6
#define	GUID_STB_MEDIATOR_API	0xab040406, 0x8aa1, 0x11d2, 0x8d, 0xd1, 0x0, 0x10, 0x4b, 0xb5, 0xea, 0xd6
#define GUID_STB_COLOUR_API		0xab040407, 0x8aa1, 0x11d2, 0x8d, 0xd1, 0x0, 0x10, 0x4b, 0xb5, 0xea, 0xd6
#define GUID_STB_INFO_API		0xab040408, 0x8aa1, 0x11d2, 0x8d, 0xd1, 0x0, 0x10, 0x4b, 0xb5, 0xea, 0xd6
#define	GUID_STB_MONITOR_API	0xab040409, 0x8aa1, 0x11d2, 0x8d, 0xd1, 0x0, 0x10, 0x4b, 0xb5, 0xea, 0xd6

static const GUID ZeroGuid = {STB_NULL_GUID};



#endif /* EDGEDEFS_H */


