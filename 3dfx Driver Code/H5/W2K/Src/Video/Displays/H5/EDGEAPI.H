/****************************************************************************/
/*
** Copyright© 1999 STB Systems Inc.  All rights reserved.
** Project			: Edge Tools
** Target Name		: 
** Author #1		: Paul Magee
** Author #2		: 
** Purpose			: defines guids and guid structures to be used at the driver level
** Uses Libraries	: 
** Date Started		: January 99
** Date Completed	: 
** Update History #1: 
**
** File Name              : edgeapi.h
** Source Safe Location   : \3dfx\voodoo3\dd16\; \edgetools\common\
** Checked in by          : pmagee
*/
/****************************************************************************/


#ifndef STB_GUIDDEFS_H
#define STB_GUIDDEFS_H

#include "edgedefs.h"
#include "edgecaps.h"

typedef struct _ENUM_GUID
{
	GUID	Guid;
	WORD	Enum;
} ENUM_GUID;




/* ************************************************************** */

/*				definitions of Guids & Enumeration				  */

/* ************************************************************** */


/* guids for interfaces */

#define STB_GUID_INTERFACE_ENUM_START	0
#define STB_INITIALISER					STB_GUID_INTERFACE_ENUM_START
#define STB_FUNCTION_API				STB_GUID_INTERFACE_ENUM_START + 1
#define	STB_NVIDIA_D3D_API				STB_GUID_INTERFACE_ENUM_START + 2
#define	STB_NVIDIA_OGL_API				STB_GUID_INTERFACE_ENUM_START + 3
#define STB_INFO_API					STB_GUID_INTERFACE_ENUM_START + 4
#define	STB_3DFX_D3D_API				STB_GUID_INTERFACE_ENUM_START + 5
#define	STB_3DFX_OGL_API				STB_GUID_INTERFACE_ENUM_START + 6
#define	STB_DFP_API						STB_GUID_INTERFACE_ENUM_START + 7
#define	STB_MEDIATOR_API				STB_GUID_INTERFACE_ENUM_START + 8
#define STB_COLOUR_API					STB_GUID_INTERFACE_ENUM_START + 9
#define	STB_MONITOR_API					STB_GUID_INTERFACE_ENUM_START + 10
#define STB_INTERFACE_DELIM				STB_GUID_INTERFACE_ENUM_START + 11

static const ENUM_GUID STB_INTERFACE_GUIDS[] = {


	{{STB_DUMMMY_GUID},
	STB_INITIALISER},
/* Initialiser placed here so when edgectrl searches the structure, the first element */
/* will never be matched so an enumeration of 0 will always mean that the guid */
/* has not been found */

/* was contained within an #ifdef, but removed because this will always be enabled */
	{{GUID_STB_FUNCTION_API},
	STB_FUNCTION_API},/*STB_FUNCTION_API*/

//#ifdef V4400TWEAK_ENABLED
//	{{GUID_STB_NVIDIA_D3D_API},
//	STB_NVIDIA_D3D_API},/*STB_NVIDIA_D3D_API*/
//	{{GUID_STB_NVIDIA_OGL_API},
//	STB_NVIDIA_OGL_API},/*STB_NVIDIA_OGL_API*/
//#endif

/* dummy values to pad out structure */

	{{STB_DUMMMY_GUID},
	STB_NVIDIA_D3D_API},
	{{STB_DUMMMY_GUID},
	STB_NVIDIA_OGL_API},
	{{GUID_STB_INFO_API},
	STB_INFO_API},	/* info api */

#ifdef STB_3DFX_D3D_API_ENABLED
	{{GUID_STB_3DFX_D3D_API},
	STB_3DFX_D3D_API},/*STB_3DFX_D3D_API*/
#else
	{{STB_DUMMMY_GUID},
	STB_3DFX_D3D_API},
#endif

#ifdef STB_3DFX_OGL_API_ENABLED
	{{GUID_STB_3DFX_OGL_API},
	STB_3DFX_OGL_API},/*STB_3DFX_OGL_API*/
#else
	{{STB_DUMMMY_GUID},
	STB_3DFX_OGL_API},/*STB_3DFX_OGL_API*/
#endif

#ifdef STB_DFP_ENABLED
	{{GUID_STB_DFP_API},
	STB_DFP_API},/*STB_DFP_API*/
#else
	{{STB_DUMMMY_GUID},
	STB_DFP_API},/*STB_DFP_API*/
#endif

#ifdef STB_MEDIATOR_ENABLED
	{{GUID_STB_MEDIATOR_API},
	STB_MEDIATOR_API},/*STB_MEDIATOR_API*/
#else
	{{STB_DUMMMY_GUID},
	STB_DFP_API},/*STB_DFP_API*/
#endif

#ifdef STB_COLOUR_ENABLED
	{{GUID_STB_COLOUR_API},
	STB_COLOUR_API},
#else
	{{STB_DUMMMY_GUID},
	STB_COLOUR_API},/*STB_COLOUR_API*/
#endif

#ifdef	STB_MONITOR_ENABLED
	{{GUID_STB_MONITOR_API},
		STB_MONITOR_API},
#else
	{{STB_DUMMMY_GUID},
	STB_MONITOR_API},
#endif

	{{STB_NULL_GUID},
	STB_INTERFACE_DELIM}/* End of Structure */

};

#endif STB_GUIDDEFS_H
