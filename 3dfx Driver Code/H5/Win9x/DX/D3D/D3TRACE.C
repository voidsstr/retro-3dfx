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
** File name: D3trace.c
**
** Description: Misc debugging tools.
**
** $Revision: 21$
** $Date: 10/11/00 8:47:45 PM$
**
**
** $Log: 
**  21   3dfx      1.10.1.1.1.710/11/00 Brent           Forced check in to enforce
**       branching.
**  20   3dfx      1.10.1.1.1.609/26/00 Johnny Trainor  #ifdefed some Dx7 specific
**       code. The file now compiles under Dx8.
**  19   3dfx      1.10.1.1.1.509/13/00 Allen Hansen    made the D3DPRINT for
**       texgen more informitive
**  18   3dfx      1.10.1.1.1.409/02/00 Allen Hansen    added texgen support to
**       D3DTSS_TEXCOORDINDEX
**  17   3dfx      1.10.1.1.1.308/27/00 Allen Hansen    added comments to the
**       matrix stats code
**  16   3dfx      1.10.1.1.1.208/21/00 Allen Hansen    added T&L renderstate
**       support to printRenderState()
**  15   3dfx      1.10.1.1.1.106/25/00 Allen Hansen    updated a couple of
**       variable names in "printLightStates()
**  14   3dfx      1.10.1.1.1.006/23/00 Steve Rogers    Speeding up the debug
**       version of the driver as well as allowing Half Life and Shogo to work with
**       Debug drivers.
**  13   3dfx      1.10.1.1    05/16/00 Bob Johnston    Consolodating VERT_BUFF and
**       TnL_HAL defines to just use TnL_HAL
**  12   3dfx      1.10.1.0    05/10/00 Bob Seitsinger  Added D3DPRINTTEXCOORD
**       function.
**  11   3dfx      1.10        03/20/00 Christopher Wilcox Removed AGP texture
**       download support.
** 
**  10   3dfx      1.9         03/17/00 Bob Seitsinger  Added compile and run-time
**       informational messages for the #define's that are set.
**  9    3dfx      1.8         03/11/00 Bob Seitsinger  Debug statements to assist
**       in buffer allocation/deallocation debugging.
**  8    3dfx      1.7         03/01/00 Bob Seitsinger  Added one comment.
**  7    3dfx      1.6         02/29/00 Bob Seitsinger  Added static variable
**       ulDoit in D3DPRINT to allow for easy toggling of all debug messages from
**       within a debugger.
**  6    3dfx      1.5         01/28/00 Scott Kephart   Big T&L Merge: changes for
**       FVF handling
**  5    3dfx      1.4         12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  4    3dfx      1.3         11/11/99 Scott Kephart   More profiling code for
**       lights
**  3    3dfx      1.2         11/10/99 Scott Kephart   Added profiling changes for
**       T&L
**  2    3dfx      1.1         10/26/99 Scott Kephart   Added initial support for
**       software T&L HAL
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 3     7/28/99 11:59p Bseitsin
** Changes to enable DX7 for W9x.
** 
** 2     7/09/99 4:33p Bseitsin
** Backwards compatability changes.
** 
** 1     6/02/99 6:44a Michael
** Branch from H3
** 
** 26    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
** 
** 25    2/11/99 11:07a Mconrad
** Support for Banshee/Voodoo3 file names based on HP= H3 or H4
** environment variable. Fixes PRSs #4087, #4088, #4151.
** 
** 24    1/29/99 10:49a Cshaw
** Added unified headers.
** 
** 23    11/22/98 9:04p Andrew
** Changes to support multi-monitor
** 
** 22    11/13/98 2:52p Miriam
** Support for DX6 triangle flavor & texture flavor tracing. Just compile
** with debug & fp=1 or tp=1.
** 
** 21    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
** 
** 20    9/25/98 6:46p Adrians
** Change the Debug string start text for Voodoo2 and Banshee.
** 
** 19    9/13/98 12:20p Adrians
** Added FbzColorPath print trace.
** 
** 18    9/12/98 12:57a Adrians
** Clean up renderstate trace code and add new DX6 states.
** Add trace support for DX6 texture stage states.
** General DX6 code tidyup.
** 
** 17    9/02/98 3:16p Adrians
** Allocate a debug level to texture memory heap allocation.
** 
** 16    8/14/98 8:09p Adrians
** Remove C runtime dependency.
** 
** 1     8/10/98 3:20p Adrians
** 
** 15    8/04/98 10:15a Adrians
** Added new debug levels.
** 
** 14    7/24/98 1:37p Hohn
** 
** 13    5/26/98 7:24p Adrians
** Added renderstate debug level.
** 
** 12    5/18/98 1:43p Adrians
** Wbuffering now uses the full wbuffer range.
** 
** 11    5/06/98 6:09p Adrians
** Changes for DX6 into DX5 driver.
** 
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
** 
** 1     4/29/98 6:31p Adrians
** Created
** 
** 9     3/27/98 11:17a Adrians
** Code added for triangle profiling.
 * 
 * 8     11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 * 
 * 7     11/18/97 4:36p Adrians
 * Nows compiles with new DDK R3.
 * 
 * 6     10/10/97 10:42a Adrians
 * Removed all references to DIRECTX5.
 * 
 * 5     9/15/97 3:44p Adrians
 * Tidyup Fan and Strip code.
 * Removed some int to float routines
 * Changed lines and points to send wstz with all vertices
 * Converted tabs to spaces in some files
 * 
 * 4     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#include "precomp.h"
// Fix for building with Windows 98 DDK (Must include DDrawI first)
#include "ddrawi.h"
#include "d3dhal.h"
#include "d3global.h"
#include "ddglobal.h"

#if (FXTRACE || FXPERF)

// Here is a list of the debug levels
//
// 0 - Always on
// 1 - Entry points
// 2 - Exit points
// 4 - Texture heap memory allocation
// 15 - Stencil Renderstates (DLSTENCIL in d3global.h)
// 16 - Surface allocation/deallocation (DLSURFACE in d3global.h)
// 17 - Calls to memMgr_* (DLALLOC in d3global.h)
// 18 - #Define information messages (DLINFOMSGS in d3global.h)

// 24 - DX5 Renderstates
// 32 - DX6 DrawPrimtive2 Types
// 33 - DX6 DrawPrimitive2 RenderStates
#define DLRS 33
// 34 - DX6 DrawPrimitive2 Texture Stage States
#define DLTS 34
// 40 - AA Entry points
// 41 - AA Information messages
// 56 - Dx6 Multi-texture opcodes
// 58 - Dx6 WInfo data
// 60 - FbzColorPath register
#define DLFBZCP 60

#if defined(TNL_PROFILE) && defined(TnL_HAL)
// 64 - TnL Hal Profiling
#define DLTNL  64
#endif //TNL_PROFILE

BYTE debugLevel[256] = {
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

char dbgEntryStr[128] = "";

/*-------------------------------------------------------------------
Function Name:  InitDebugMsgs
Description:    Displays messages based on symbols defined.
Information:    void InitDebugMsgs()
Return:         None.
                
-------------------------------------------------------------------*/
#ifdef DEBUG

void _stdcall InitDebugMsgs()
{
#ifndef WINNT

# ifdef CMDFIFO
#   pragma message("CMDFIFO defined")
	D3DPRINT(DLINFOMSGS, "CMDFIFO defined...");
# endif

# ifdef NEW_CLIP_FOR_GB
#   pragma message("NEW_CLIP_FOR_GB defined")
	D3DPRINT(DLINFOMSGS, "NEW_CLIP_FOR_GB defined...");
# endif

# ifdef NEW_CCU
#   pragma message("NEW_CCU defined")
	D3DPRINT(DLINFOMSGS, "NEW_CCU defined...");
# endif

# ifdef AGP_CMDFIFO
#   pragma message("AGP_CMDFIFO defined")
	D3DPRINT(DLINFOMSGS, "AGP_CMDFIFO defined...");
# endif

# ifdef K6_2
#   pragma message("K6_2 defined")
	D3DPRINT(DLINFOMSGS, "K6_2 defined...");
# endif

# ifdef NOKNITEXTUREDOWNLOAD
#   pragma message("NOKNITEXTUREDOWNLOAD defined")
	D3DPRINT(DLINFOMSGS, "NOKNITEXTUREDOWNLOAD defined...");
# endif

# ifdef SM
#   pragma message("SM (special MultiTxtr modes) defined")
	D3DPRINT(DLINFOMSGS, "SM (special MultiTxtr modes) defined...");
# endif

# ifdef STBKNI
#   if (STBKNI==0)
#     pragma message("STBKNI==0 defined")
	  D3DPRINT(DLINFOMSGS, "STBKNI==0 defined...");
#   endif
#   if (STBKNI==1)
#     pragma message("STBKNI==1 defined")
	  D3DPRINT(DLINFOMSGS, "STBKNI==1 defined...");
#   endif
# endif

# ifdef RD_ABORT_ERROR
#   pragma message("RD_ABORT_ERROR defined")
	D3DPRINT(DLINFOMSGS, "RD_ABORT_ERROR defined...");
# endif

# ifdef TnL_HAL
#   if (TnL_HAL==1)
#     pragma message("TnL_HAL==1 defined")
	  D3DPRINT(DLINFOMSGS, "TnL_HAL==1 defined...");
#   endif
# endif

# ifdef TNL_PROFILE
#   pragma message("TNL_PROFILE defined")
	D3DPRINT(DLINFOMSGS, "TNL_PROFILE defined...");
# endif

# ifdef WIN_CSIM
#   pragma message("WIN_CSIM defined")
	D3DPRINT(DLINFOMSGS, "WIN_CSIM defined...");
# endif

# ifdef SLI_AA
#   pragma message("SLI_AA defined")
	D3DPRINT(DLINFOMSGS, "SLI_AA defined...");
# endif

# ifdef SLI_AA_2D
#   pragma message("SLI_AA_2D defined")
	D3DPRINT(DLINFOMSGS, "SLI_AA_2D defined...");
# endif

# ifdef C_AUTOSTRIP
#   pragma message("C_AUTOSTRIP defined")
	D3DPRINT(DLINFOMSGS, "C_AUTOSTRIP defined...");
//#   if (C_AUTOSTRIP==0)
//#     pragma message("C_AUTOSTRIP==0 defined")
//	  D3DPRINT(DLINFOMSGS, "C_AUTOSTRIP==0 defined...");
//#   endif
//#   if (C_AUTOSTRIP==1)
//#     pragma message("C_AUTOSTRIP==1 defined")
//	  D3DPRINT(DLINFOMSGS, "C_AUTOSTRIP==1 defined...");
//#   endif
# endif

# ifdef NEWASMTRI
#   if (NEWASMTRI==0)
#     pragma message("NEWASMTRI==0 defined")
	  D3DPRINT(DLINFOMSGS, "NEWASMTRI==0 defined...");
#   endif
#   if (NEWASMTRI==1)
#     pragma message("NEWASMTRI==1 defined")
	  D3DPRINT(DLINFOMSGS, "NEWASMTRI==1 defined...");
#   endif
# endif

#endif // !WINNT
}

#endif // DEBUG

/*-------------------------------------------------------------------
Function Name:  isSeperator
Description:    Returns TRUE if the character is a seperator
Information:    BOOL isSeperator( char a, char* seperators )
Return:         BOOL TRUE - if the character is contained within the seperators string
                
-------------------------------------------------------------------*/
BOOL isSeperator( char a, char* seperators )
{
  unsigned int i;
  
  for(i = 0; i < strlen(seperators); i++)
  {
    if( a == seperators[i] )
      return TRUE;
  }
  
  return FALSE;
}
 
/*-------------------------------------------------------------------
Function Name:  getToken
Description:    returns a token from the string. Tokens are separated by
                separators

Information:    char *getToken( char *string, char* seperators )

Return:         pointer to the token
                
-------------------------------------------------------------------*/
char *getToken( char *string, char* seperators )
{
  static char   startOffset = 0, endOffset = 0;
  static char   savedString[128];
  static char   token[16];
    
  if( string != NULL )
  {
    strcpy( savedString, string );
    endOffset = 0;
  }

  while( savedString[endOffset] != 0 && isSeperator(savedString[endOffset], seperators)  )
    endOffset++;

  startOffset = endOffset;
    
  while( savedString[endOffset] != 0 )
  {
    if( isSeperator(savedString[endOffset], seperators) )
    {
      savedString[endOffset] = 0;
      endOffset++;
      
      return &savedString[startOffset];
    }
    
    endOffset++;
  }
  
  if( startOffset == endOffset )
    return NULL;
  else
    return &savedString[startOffset];
}

//-------------------------------------------------------------------

/*-------------------------------------------------------------------
Function Name:  setDebugLevel
Description:    Sets the debug level
Information:    void setDebugLevel( char* str )
Return:         VOID
                
-------------------------------------------------------------------*/
void setDebugLevel( char* str )
{
  char  seps[] = " ,\t\n";
  char  *token;
  int   index;

  // Read first token from string
  token = getToken( str, seps );

  // If the token is NULL then we have finished
  while( token != NULL )
  {
    // Convert the token to an integer index
    index = atoi(token);

    // Check that the index is in range and if it is
    // set the relevant debug level to ON
    if( index >= 0  && index <= 255 )
      debugLevel[index] = 255;

    // Get next token
    token = getToken( NULL, seps );
  }
}

//----------------------------------------------------------
// 
//----------------------------------------------------------
/*-------------------------------------------------------------------
Function Name:  D3DPRINT
Description:    Print a string and automatically prefix string with D3D  
Information:    void __cdecl D3DPRINT(int debugPrintLevel, LPSTR szFormat, ...)
Return:         VOID
                
-------------------------------------------------------------------*/
void __cdecl D3DPRINT(int debugPrintLevel, LPSTR szFormat, ...)
{
  static unsigned long ulDoit = 0;   // Force the display of all messages.
                                     // This allows for easy toggling of
                                     // all D3D debug messages from a
                                     // debugger.

#define START_STR       "NP.d3d: "
#define END_STR         "\r\n"

  if( debugLevel[debugPrintLevel] || ulDoit)
  { 
    char    str[1024];
    lstrcpy( str, START_STR );
    wvsprintf( str + lstrlen(str), szFormat, (LPVOID)(&szFormat+1) );
    lstrcat(str, END_STR);

    OutputDebugString( str );
  }
}

void D3DPRINTTEXCOORD(unsigned long debuglevel, char *prefix, float s, float t)
{
  if(debugLevel[debuglevel])
  {
    char sstr[256];
    char tstr[256];
    char buff[1024];

    strcpy(sstr, float2String(s));
    strcpy(tstr, float2String(t));

    wsprintf(buff, "%s s %s, t %s", prefix, sstr, tstr);

    D3DPRINT(debuglevel, "%s", buff);
  }
}

/*-------------------------------------------------------------------
Function Name:  float2String
Description:    Changes a float value to a string
Information:    char* float2String( float value )
Return:         pointer to string
                
-------------------------------------------------------------------*/
char* float2String( float value )
{
  static char buffer[24];
  int b = float2int(value);
  float a = (float)fabs(value) - (float)b;
  int c = (int)(a * 1000000.f);
    
  wsprintf( buffer, "%s%ld.%06d", value < 0.f ? "-" : "", b, c );
  return buffer;
}

//----------------------
// Rendering state 
//----------------------
static char *printSetStateString[] = {
    "",
    "TextureHandle", 
    "Antialias",
    "TextureAddress",
    "TexturePerspective", 
    "WrapU", 
    "WrapV", 
    "ZEnable",  
    "FillMode",  
    "ShadeMode",  
    "LinePattern",  
    "MonoEnable",  
    "Rop2",  
    "PlaneMask",  
    "ZWriteEnable",  
    "AlphaTestEnable",  
    "LastPixel",  
    "TextureMag",  
    "TextureMin",  
    "SrcBlend",  
    "DestBlend",  
    "TextureMapBlend",  
    "CullMode",  
    "ZFunc",  
    "AlphaRef", 
    "AlphaFunc",  
    "DitherEnable",  
    "BlendEnable",  
    "FogEnable",  
    "SpecularEnable",  
    "ZVisible",  
    "SubPixel",  
    "SubPixelX",  
    "StippledAlpha",
    "FogColor",
    "FogtableMode",
    "FogtableStart",
    "FogTableEnd",
    "FogtableDensity",
    "StippleEnable",
    "EdgeAntialias",
    "ColorKeyEnable",
    "",
    "BorderColor",
    "TextureAddressU",
    "TextureAddressV",
    "MipMapLodBias",
    "ZBias",
    "RangeFogEnable",
    "Anisotropy",
    "FlushBatch",
    "TranslucentSortIndependant",
    "StencilEnable",
    "StencilFail",
    "StencilZFail",
    "StencilPass",
    "StencilFunc",
    "StencilRef",
    "StencilMask",
    "StencilWriteMask",
    "TextureFactor",
    "",
    "",
    "",
    "StipplePattern00",
    "StipplePattern01",
    "StipplePattern02",
    "StipplePattern03",
    "StipplePattern04",
    "StipplePattern05",
    "StipplePattern06",
    "StipplePattern07",
    "StipplePattern08",
    "StipplePattern09",
    "StipplePattern10",
    "StipplePattern11",
    "StipplePattern12",
    "StipplePattern13",
    "StipplePattern14",
    "StipplePattern15",
    "StipplePattern16",
    "StipplePattern17",
    "StipplePattern18",
    "StipplePattern19",
    "StipplePattern20",
    "StipplePattern21",
    "StipplePattern22",
    "StipplePattern23",
    "StipplePattern24",
    "StipplePattern25",
    "StipplePattern26",
    "StipplePattern27",
    "StipplePattern28",
    "StipplePattern29",
    "StipplePattern30",
    "StipplePattern31",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "Wrap0",
    "Wrap1",
    "Wrap2",
    "Wrap3",
    "Wrap4",
    "Wrap5",
    "Wrap6",
    "Wrap7",
#ifdef  TnL_HAL
    "Clipping",
    "Lighting",
    "",
    "Ambient",
    "FogVertexMode",
    "ColorVertex",
    "LocalViewer",
    "NormalizeNormals",
    "",
    "DiffuseMaterialSrc",
    "SpecularMaterialSrc",
    "AmbientMaterialSrc",
    "EmissiveMaterialSrc",
    "",
    "",
    "VertexBlends",
    "ClipPlaneEnable",
#endif
    };
/*-------------------------------------------------------------------
Function Name:  printRenderState
Description:    Prints a D3D Renderstate
Information:    void printRenderState( ULONG state, ULONG stateData )
Return:         VOID
                
-------------------------------------------------------------------*/
void printRenderState( ULONG state, ULONG stateData )
{
  switch(state)
  {
    case D3DRENDERSTATE_TEXTUREADDRESS:		// =  3
    case D3DRENDERSTATE_TEXTUREADDRESSU:	// = 44
    case D3DRENDERSTATE_TEXTUREADDRESSV:	// = 45
      switch (stateData)
      {
        case  D3DTADDRESS_WRAP:
          D3DPRINT( DLRS, "RenderState %s = Wrap", printSetStateString[state] );
          break;
        case  D3DTADDRESS_MIRROR:
          D3DPRINT( DLRS, "RenderState %s = Mirror", printSetStateString[state] );
          break;
        case  D3DTADDRESS_CLAMP:
          D3DPRINT( DLRS, "RenderState %s = Clamp", printSetStateString[state] );
          break;
        case  D3DTADDRESS_BORDER:
          D3DPRINT( DLRS, "RenderState %s = Border", printSetStateString[state] );
          break;
      }  
      break;

    case D3DRENDERSTATE_ANTIALIAS:			// =  2
      switch( stateData )
      {
        case D3DANTIALIAS_NONE:
          D3DPRINT( DLRS, "RenderState %s = None", printSetStateString[state] );
          break;
        case D3DANTIALIAS_SORTDEPENDENT:
          D3DPRINT( DLRS, "RenderState %s = SortDependent", printSetStateString[state] );
          break;
        case D3DANTIALIAS_SORTINDEPENDENT:
          D3DPRINT( DLRS, "RenderState %s = SortIndependent", printSetStateString[state] );
          break;
      }
      break;
      
    case D3DRENDERSTATE_TEXTUREPERSPECTIVE:	// =  4
    case D3DRENDERSTATE_WRAPU:				// =  5
    case D3DRENDERSTATE_WRAPV:				// =  6
    case D3DRENDERSTATE_ZENABLE:			// =  7
    case D3DRENDERSTATE_MONOENABLE:			// = 11
    case D3DRENDERSTATE_ZWRITEENABLE:		// = 14
    case D3DRENDERSTATE_ALPHATESTENABLE:	// = 15
    case D3DRENDERSTATE_LASTPIXEL:			// = 16
    case D3DRENDERSTATE_DITHERENABLE:		// = 26
    case D3DRENDERSTATE_BLENDENABLE:		// = 27
    case D3DRENDERSTATE_FOGENABLE:			// = 28
    case D3DRENDERSTATE_SPECULARENABLE:		// = 29
    case D3DRENDERSTATE_ZVISIBLE:			// = 30
    case D3DRENDERSTATE_SUBPIXEL:			// = 31
    case D3DRENDERSTATE_SUBPIXELX:			// = 32
    case D3DRENDERSTATE_STIPPLEENABLE:		// = 33
    case D3DRENDERSTATE_STIPPLEDALPHA:		// = 39
    case D3DRENDERSTATE_EDGEANTIALIAS:		// = 40
    case D3DRENDERSTATE_COLORKEYENABLE:		// = 41
    case D3DRENDERSTATE_RANGEFOGENABLE:		// = 48
  #if ( DX >= 6 )    
    case D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT: // = 51
    case D3DRENDERSTATE_STENCILENABLE:		// = 52
  #endif    
      if (stateData)
        D3DPRINT( DLRS,"RenderState %s = Enable", printSetStateString[state] );
      else
        D3DPRINT( DLRS, "RenderState %s = Disable", printSetStateString[state] );
      break;
        
    case D3DRENDERSTATE_FOGCOLOR:			// = 34
    case D3DRENDERSTATE_BORDERCOLOR:		// = 43
  #if ( DX >= 6 )    
    case D3DRENDERSTATE_TEXTUREFACTOR:		// = 60
  #endif
      D3DPRINT( DLRS, "RenderState %s, A = 0x%02x, R = 0x%02x, G = 0x%02x, B = 0x%02x", printSetStateString[state],
        RGBA_GETALPHA(stateData), RGBA_GETRED(stateData), RGBA_GETGREEN(stateData), RGBA_GETBLUE(stateData) );
      break;
    
    case D3DRENDERSTATE_FOGTABLEMODE:		// = 35
      switch (stateData)
      {
        case D3DFOG_NONE:
          D3DPRINT( DLRS, "RenderState %s = None", printSetStateString[state] );
          break;
        case D3DFOG_EXP:
          D3DPRINT( DLRS, "RenderState %s = Exp", printSetStateString[state] );
          break;
        case D3DFOG_EXP2:
          D3DPRINT( DLRS, "RenderState %s = Exp2", printSetStateString[state] );
          break;
        case D3DFOG_LINEAR:
          D3DPRINT( DLRS, "RenderState %s = Linear", printSetStateString[state] );
          break;
	    default:
          D3DPRINT( DLRS, "RenderState %s = UNKNOWN", printSetStateString[state] );
          break;
      } 
      break;
    
    case D3DRENDERSTATE_LINEPATTERN:		// = 10
    case D3DRENDERSTATE_ROP2:				// = 12
    case D3DRENDERSTATE_PLANEMASK:			// = 13
    case D3DRENDERSTATE_ALPHAREF:			// = 24
    case D3DRENDERSTATE_FOGTABLESTART:		// = 36
    case D3DRENDERSTATE_FOGTABLEEND:		// = 37
    case D3DRENDERSTATE_FOGTABLEDENSITY:	// = 38
    case D3DRENDERSTATE_MIPMAPLODBIAS:		// = 46
    case D3DRENDERSTATE_ZBIAS:				// = 47
    case D3DRENDERSTATE_ANISOTROPY:			// = 49
  #if ( DX >= 6 )    
    case D3DRENDERSTATE_STENCILREF:			// = 57
    case D3DRENDERSTATE_STENCILMASK:		// = 58
    case D3DRENDERSTATE_STENCILWRITEMASK:	// = 59
  #endif
    case D3DRENDERSTATE_STIPPLEPATTERN00:	// = 64
    case D3DRENDERSTATE_STIPPLEPATTERN01:	// = 65
    case D3DRENDERSTATE_STIPPLEPATTERN02:	// = 66
    case D3DRENDERSTATE_STIPPLEPATTERN03:	// = 67
    case D3DRENDERSTATE_STIPPLEPATTERN04:	// = 68
    case D3DRENDERSTATE_STIPPLEPATTERN05:	// = 69
    case D3DRENDERSTATE_STIPPLEPATTERN06:	// = 70
    case D3DRENDERSTATE_STIPPLEPATTERN07:	// = 71
    case D3DRENDERSTATE_STIPPLEPATTERN08:	// = 72
    case D3DRENDERSTATE_STIPPLEPATTERN09:	// = 73
    case D3DRENDERSTATE_STIPPLEPATTERN10:	// = 74
    case D3DRENDERSTATE_STIPPLEPATTERN11:	// = 75
    case D3DRENDERSTATE_STIPPLEPATTERN12:	// = 76
    case D3DRENDERSTATE_STIPPLEPATTERN13:	// = 77
    case D3DRENDERSTATE_STIPPLEPATTERN14:	// = 78
    case D3DRENDERSTATE_STIPPLEPATTERN15:	// = 79
    case D3DRENDERSTATE_STIPPLEPATTERN16:	// = 80
    case D3DRENDERSTATE_STIPPLEPATTERN17:	// = 81
    case D3DRENDERSTATE_STIPPLEPATTERN18:	// = 82
    case D3DRENDERSTATE_STIPPLEPATTERN19:	// = 83
    case D3DRENDERSTATE_STIPPLEPATTERN20:	// = 84
    case D3DRENDERSTATE_STIPPLEPATTERN21:	// = 85
    case D3DRENDERSTATE_STIPPLEPATTERN22:	// = 86
    case D3DRENDERSTATE_STIPPLEPATTERN23:	// = 87
    case D3DRENDERSTATE_STIPPLEPATTERN24:	// = 88
    case D3DRENDERSTATE_STIPPLEPATTERN25:	// = 89
    case D3DRENDERSTATE_STIPPLEPATTERN26:	// = 90
    case D3DRENDERSTATE_STIPPLEPATTERN27:	// = 91
    case D3DRENDERSTATE_STIPPLEPATTERN28:	// = 92
    case D3DRENDERSTATE_STIPPLEPATTERN29:	// = 93
    case D3DRENDERSTATE_STIPPLEPATTERN30:	// = 94
    case D3DRENDERSTATE_STIPPLEPATTERN31:	// = 95
      D3DPRINT( DLRS, "RenderState %s = 0x%08lx", printSetStateString[state], stateData );
      break;

    case D3DRENDERSTATE_ZFUNC:				// = 23
    case D3DRENDERSTATE_ALPHAFUNC:			// = 25
  #if ( DX >= 6 )    
    case D3DRENDERSTATE_STENCILFUNC:		// = 56
  #endif
      switch (stateData) 
      {
        case D3DCMP_NEVER : 
          D3DPRINT( DLRS, "RenderState %s = Never", printSetStateString[state] );
          break;  
        case D3DCMP_LESS  :   
          D3DPRINT( DLRS, "RenderState %s = Less", printSetStateString[state] );
          break;  
        case D3DCMP_EQUAL :   
          D3DPRINT( DLRS, "RenderState %s = Equal", printSetStateString[state] );
          break;  
        case D3DCMP_LESSEQUAL:   
          D3DPRINT( DLRS, "RenderState %s = LessEqual", printSetStateString[state] );
          break;  
        case D3DCMP_GREATER : 
          D3DPRINT( DLRS, "RenderState %s = Greater", printSetStateString[state] );
          break;  
        case D3DCMP_NOTEQUAL: 
          D3DPRINT( DLRS, "RenderState %s = NotEqual", printSetStateString[state] );
          break;  
        case D3DCMP_GREATEREQUAL :     
          D3DPRINT( DLRS, "RenderState %s = GreaterEqual", printSetStateString[state] );
          break;  
        case D3DCMP_ALWAYS :  
          D3DPRINT( DLRS, "RenderState %s = Always", printSetStateString[state] );
          break;  
      }   // z func
      break;
           
    case D3DRENDERSTATE_SRCBLEND:			// = 19
    case D3DRENDERSTATE_DESTBLEND:			// = 20
      switch ( stateData ) 
      {
        case D3DBLEND_ZERO :                     
          D3DPRINT( DLRS, "RenderState %s = Zero", printSetStateString[state] );
          break;  
        case D3DBLEND_ONE  :              
          D3DPRINT( DLRS, "RenderState %s = One", printSetStateString[state] );
          break;  
        case D3DBLEND_SRCCOLOR     :     
          D3DPRINT( DLRS, "RenderState %s = SrcColor", printSetStateString[state] );
          break;  
        case D3DBLEND_INVSRCCOLOR :     
          D3DPRINT( DLRS, "RenderState %s = InvSrcColor", printSetStateString[state] );
          break;  
        case D3DBLEND_SRCALPHA     :     
          D3DPRINT( DLRS, "RenderState %s = SrcAlpha", printSetStateString[state] );
          break;  
        case D3DBLEND_INVSRCALPHA :     
          D3DPRINT( DLRS, "RenderState %s = InvSrcAlpha", printSetStateString[state] );
          break;  
        case D3DBLEND_DESTALPHA     :     
          D3DPRINT( DLRS, "RenderState %s = DestAlpha", printSetStateString[state] );
          break;  
        case D3DBLEND_INVDESTALPHA :     
          D3DPRINT( DLRS, "RenderState %s = InvDestAlpha", printSetStateString[state] );
          break;  
        case D3DBLEND_DESTCOLOR     :     
          D3DPRINT( DLRS, "RenderState %s = DestColor", printSetStateString[state] );
          break;  
        case D3DBLEND_INVDESTCOLOR :     
          D3DPRINT( DLRS, "RenderState %s = InvDstColor", printSetStateString[state] );
          break;  
        case D3DBLEND_SRCALPHASAT :     
          D3DPRINT( DLRS, "RenderState %s = SrcAlphaSat", printSetStateString[state] );
          break;  
        case D3DBLEND_BOTHSRCALPHA:     
          D3DPRINT( DLRS, "RenderState %s = BothSrcAlpha", printSetStateString[state] );
          break;  
        case D3DBLEND_BOTHINVSRCALPHA :
          D3DPRINT( DLRS, "RenderState %s = BothInvSrcAlpha", printSetStateString[state] );
          break;  
      }
      break;

    case D3DRENDERSTATE_TEXTUREHANDLE:		// =  1
      D3DPRINT( DLRS, "RenderState %s = %ld", printSetStateString[state], stateData );
      break;  

    case D3DRENDERSTATE_FILLMODE:			// =  8
      if (stateData == D3DFILL_POINT)     
        D3DPRINT( DLRS, "RenderState %s = Point", printSetStateString[state] );
      else if (stateData == D3DFILL_WIREFRAME)
        D3DPRINT( DLRS, "RenderState %s = WireFrame", printSetStateString[state] );
      else if (stateData == D3DFILL_SOLID)  
        D3DPRINT( DLRS, "RenderState %s = Solid", printSetStateString[state] );
      break;  

    case D3DRENDERSTATE_SHADEMODE :			// =  9
      if (stateData == D3DSHADE_FLAT)     
        D3DPRINT( DLRS, "RenderState %s = Flat", printSetStateString[state] );
      else if (stateData == D3DSHADE_GOURAUD)
        D3DPRINT( DLRS, "RenderState %s = Gouraud", printSetStateString[state] );
      else if (stateData == D3DSHADE_PHONG)  
        D3DPRINT( DLRS, "RenderState %s = Phong", printSetStateString[state] );
      break;  

    case D3DRENDERSTATE_TEXTUREMAG:			// = 17
    case D3DRENDERSTATE_TEXTUREMIN:			// = 18
      switch(stateData)
      {
        case D3DFILTER_NEAREST:
          D3DPRINT( DLRS, "RenderState %s = Nearest", printSetStateString[state] );
          break;  
        case D3DFILTER_LINEAR:
          D3DPRINT( DLRS, "RenderState %s = Linear", printSetStateString[state] );
          break;  
        case D3DFILTER_MIPNEAREST:
          D3DPRINT( DLRS, "RenderState %s = MipNearest", printSetStateString[state] );
          break;  
        case D3DFILTER_MIPLINEAR:
          D3DPRINT( DLRS, "RenderState %s = MipLinear", printSetStateString[state] );
          break;  
        case D3DFILTER_LINEARMIPNEAREST:
          D3DPRINT( DLRS, "RenderState %s = LinearMipNearest", printSetStateString[state] );
          break;  
        case D3DFILTER_LINEARMIPLINEAR:
          D3DPRINT( DLRS, "RenderState %s = LinearMipLinear", printSetStateString[state] );
          break;  
      }
      break;

    case D3DRENDERSTATE_TEXTUREMAPBLEND:	// = 21
      switch(stateData)
      {
        case D3DTBLEND_DECAL:
          D3DPRINT( DLRS, "RenderState %s = Decal", printSetStateString[state] );
          break;  
        case D3DTBLEND_MODULATE:
          D3DPRINT( DLRS, "RenderState %s = Modulate", printSetStateString[state] );
          break;  
        case D3DTBLEND_DECALALPHA:
          D3DPRINT( DLRS, "RenderState %s = DecalAlpha", printSetStateString[state] );
          break;  
        case D3DTBLEND_MODULATEALPHA:
          D3DPRINT( DLRS, "RenderState %s = ModulateAlpha", printSetStateString[state] );
          break;  
        case D3DTBLEND_DECALMASK:
          D3DPRINT( DLRS, "RenderState %s = DecalMask", printSetStateString[state] );
          break;  
        case D3DTBLEND_MODULATEMASK:
          D3DPRINT( DLRS, "RenderState %s = ModulateMask", printSetStateString[state] );
          break;  
        case D3DTBLEND_COPY:
          D3DPRINT( DLRS, "RenderState %s = Copy", printSetStateString[state] );
          break;  
        default: 
          D3DPRINT( DLRS, "RenderState %s = UNKNOWN, %d", printSetStateString[state], stateData );
          break;
      }     
      break;  

    case D3DRENDERSTATE_CULLMODE:			// = 22
      if (stateData == D3DCULL_NONE)     
        D3DPRINT( DLRS, "RenderState %s = None", printSetStateString[state] );
      else if (stateData == D3DCULL_CW)
        D3DPRINT( DLRS, "RenderState %s = CW", printSetStateString[state] );
      else if (stateData == D3DCULL_CCW)  
        D3DPRINT( DLRS, "RenderState %s = CCW", printSetStateString[state] );
      break;

  #if ( DX >= 6 )    
    case D3DRENDERSTATE_WRAP0:				// = 128
    case D3DRENDERSTATE_WRAP1:				// = 129
    case D3DRENDERSTATE_WRAP2:				// = 130
    case D3DRENDERSTATE_WRAP3:				// = 131
    case D3DRENDERSTATE_WRAP4:				// = 132
    case D3DRENDERSTATE_WRAP5:				// = 133
    case D3DRENDERSTATE_WRAP6:				// = 134
    case D3DRENDERSTATE_WRAP7:				// = 135
      switch( stateData )
      {
        case 0:
          D3DPRINT( DLRS, "RenderState %s = None", printSetStateString[state] );
          break;
        case D3DWRAP_U:
          D3DPRINT( DLRS, "RenderState %s = WrapU", printSetStateString[state] );
          break;
        case D3DWRAP_V:
          D3DPRINT( DLRS, "RenderState %s = WrapV", printSetStateString[state] );
          break;
        case (D3DWRAP_U | D3DWRAP_V):
          D3DPRINT( DLRS, "RenderState %s = WrapU & WrapV", printSetStateString[state] );
          break;
      }
      break;
    
    case D3DRENDERSTATE_STENCILFAIL:		// = 53
    case D3DRENDERSTATE_STENCILZFAIL:		// = 54
    case D3DRENDERSTATE_STENCILPASS:		// = 55
      switch( stateData )
      {
        case D3DSTENCILOP_KEEP:
          D3DPRINT( DLRS, "RenderState %s = Keep", printSetStateString[state] );
          break;
        case D3DSTENCILOP_ZERO:
          D3DPRINT( DLRS, "RenderState %s = Zero", printSetStateString[state] );
          break;
        case D3DSTENCILOP_REPLACE:
          D3DPRINT( DLRS, "RenderState %s = Replace", printSetStateString[state] );
          break;
        case D3DSTENCILOP_INCRSAT:
          D3DPRINT( DLRS, "RenderState %s = IncrSat", printSetStateString[state] );
          break;
        case D3DSTENCILOP_DECRSAT:
          D3DPRINT( DLRS, "RenderState %s = DecrSat", printSetStateString[state] );
          break;
        case D3DSTENCILOP_INVERT:
          D3DPRINT( DLRS, "RenderState %s = Invert", printSetStateString[state] );
          break;
        case D3DSTENCILOP_INCR:
          D3DPRINT( DLRS, "RenderState %s = Incr", printSetStateString[state] );
          break;
        case D3DSTENCILOP_DECR:
          D3DPRINT( DLRS, "RenderState %s = Decr", printSetStateString[state] );
          break;
      }
      break;
  #endif

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    case D3DRENDERSTATE_SCENECAPTURE:		// = 62
      D3DPRINT( DLRS, "RenderState SceneCapture = %s", printSetStateString[state] ? "BEGIN" : "END" );
      break;

#ifdef TnL_HAL
    case D3DRENDERSTATE_CLIPPING:			// = 136
    case D3DRENDERSTATE_LIGHTING:			// = 137
    case D3DRENDERSTATE_COLORVERTEX:		// = 141
	case D3DRENDERSTATE_LOCALVIEWER:		// = 142
    case D3DRENDERSTATE_NORMALIZENORMALS:	// = 143
    case D3DRENDERSTATE_CLIPPLANEENABLE:	// = 152
      switch(stateData)
      {
        case 0:
          D3DPRINT(DLRS, "RenderState %s = Disable", printSetStateString[state] );
          break;
        case 1:
          D3DPRINT(DLRS, "RenderState %s = Enable", printSetStateString[state] );
          break;
      }
      break;
    case D3DRENDERSTATE_AMBIENT:			// = 139
      D3DPRINT(DLRS, "RenderState %s  a=0x%02lx  r=0x%02lx  g=0x%02lx  b=0x%02lx", 
      		printSetStateString[state], (state>>24)&0xff, (state>>16)&0xff, (state>>8)&0xff, (state>>0)&0xff );
      break;
	case D3DRENDERSTATE_FOGVERTEXMODE:		// = 140
      switch (stateData)
      {
        case D3DFOG_NONE:
          D3DPRINT( DLRS, "RenderState %s = None", printSetStateString[state] );
          break;
        case D3DFOG_EXP:
          D3DPRINT( DLRS, "RenderState %s = Exp", printSetStateString[state] );
          break;
        case D3DFOG_EXP2:
          D3DPRINT( DLRS, "RenderState %s = Exp2", printSetStateString[state] );
          break;
        case D3DFOG_LINEAR:
          D3DPRINT( DLRS, "RenderState %s = Linear", printSetStateString[state] );
          break;
	    default:
          D3DPRINT( DLRS, "RenderState %s = UNKNOWN", printSetStateString[state] );
          break;
	  }
    case D3DRENDERSTATE_VERTEXBLEND:		// = 151
      switch(stateData)
      {
        case 0:
          D3DPRINT(DLRS, "RenderState %s = 0", printSetStateString[state] );
          break;
        case 1:
          D3DPRINT(DLRS, "RenderState %s = 1", printSetStateString[state] );
          break;
        case 2:
          D3DPRINT(DLRS, "RenderState %s = 2", printSetStateString[state] );
          break;
        case 3:
          D3DPRINT(DLRS, "RenderState %s = 3", printSetStateString[state] );
          break;
        case 4:
          D3DPRINT(DLRS, "RenderState %s = 4", printSetStateString[state] );
          break;
      }
      break;
    case D3DRENDERSTATE_DIFFUSEMATERIALSOURCE: 	// = 145
    case D3DRENDERSTATE_SPECULARMATERIALSOURCE:	// = 146
    case D3DRENDERSTATE_AMBIENTMATERIALSOURCE:	// = 147
    case D3DRENDERSTATE_EMISSIVEMATERIALSOURCE:	// = 148
      switch(stateData)
      {
        case D3DMCS_MATERIAL:
          D3DPRINT(DLRS, "RenderState %s = D3DMCS_MATERIAL", printSetStateString[state] );
          break;        
        case D3DMCS_COLOR1:
          D3DPRINT(DLRS, "RenderState %s = D3DMCS_COLOR1", printSetStateString[state] );
          break;        
        case D3DMCS_COLOR2:
          D3DPRINT(DLRS, "RenderState %s = D3DMCS_COLOR2", printSetStateString[state] );
          break;        
	    default:
          D3DPRINT( DLRS, "RenderState %s = UNKNOWN Color source", printSetStateString[state] );
          break;
      }
      break;
#endif	// TnL_HAL
#endif	// DX7
          
    // Not catered for.
    case D3DRENDERSTATE_EXTENTS:				// = 138
	case D3DRENDERSTATE_COLORKEYBLENDENABLE:	// = 144
#if 0 // these renderstates disappeared in the 2082 DDK
	case D3DRENDERSTATE_ALPHASOURCE:			// = 149
	case D3DRENDERSTATE_FOGFACTORSOURCE:		// = 150
	case D3DRENDERSTATE_POINTSIZE:				// = 153
	case D3DRENDERSTATE_POINTATTENUATION_A:		// = 154
	case D3DRENDERSTATE_POINTATTENUATION_B:		// = 155
	case D3DRENDERSTATE_POINTATTENUATION_C:		// = 156
	case D3DRENDERSTATE_POINTSIZEMIN:			// = 157
	case D3DRENDERSTATE_POINTSPRITE_ENABLE:		// = 158
#endif
    default:
      D3DPRINT( DLRS, "RenderState %s (%d) = UNKNOWN (0x%08lx)", printSetStateString[state], state, stateData );
      break;
  }
}

//-------------------------------------------------------------------
/*-------------------------------------------------------------------
Function Name:  printContext
Description:    prints the entire D3D Renderstate
Information:    void printContext(RC *pRc)
Return:         void
                
-------------------------------------------------------------------*/
void printContext(RC *pRc)
{
    printRenderState(D3DRENDERSTATE_TEXTUREHANDLE,pRc->texture);
    //D3DRENDERSTATE_ANTIALIAS          
    printRenderState(D3DRENDERSTATE_TEXTUREADDRESS,pRc->textureAddress);
    printRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE,pRc->texturePerspective);
    printRenderState(D3DRENDERSTATE_WRAPU,pRc->wrapU);
    printRenderState(D3DRENDERSTATE_WRAPV,pRc->wrapV);
    printRenderState(D3DRENDERSTATE_ZENABLE,pRc->zEnable);
    //D3DRENDERSTATE_FILLMODE             
    printRenderState(D3DRENDERSTATE_SHADEMODE,pRc->shadeMode);
    //D3DRENDERSTATE_LINEPATTERN          
    //D3DRENDERSTATE_MONOENABLE           
    //D3DRENDERSTATE_ROP2                 
    //D3DRENDERSTATE_PLANEMASK            
    printRenderState(D3DRENDERSTATE_ZWRITEENABLE,pRc->zWriteEnable);
    printRenderState(D3DRENDERSTATE_ALPHATESTENABLE,pRc->alphaTestEnable);
    //D3DRENDERSTATE_LASTPIXEL            
    printRenderState(D3DRENDERSTATE_TEXTUREMAG,pRc->texMag);
    printRenderState(D3DRENDERSTATE_TEXTUREMIN,pRc->texMin);
    printRenderState(D3DRENDERSTATE_SRCBLEND,pRc->srcBlend);
    printRenderState(D3DRENDERSTATE_DESTBLEND,pRc->dstBlend);
    printRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,pRc->texMapBlend);
    printRenderState(D3DRENDERSTATE_CULLMODE,pRc->cullMode);
    printRenderState(D3DRENDERSTATE_ZFUNC,pRc->zFunc);
    printRenderState(D3DRENDERSTATE_ALPHAREF,pRc->alphaRef);
    printRenderState(D3DRENDERSTATE_ALPHAFUNC,pRc->alphaFunc);
    printRenderState(D3DRENDERSTATE_DITHERENABLE,pRc->ditherEnable);
    printRenderState(D3DRENDERSTATE_BLENDENABLE,pRc->blendEnable);
    printRenderState(D3DRENDERSTATE_FOGENABLE,pRc->fogEnable);
    printRenderState(D3DRENDERSTATE_SPECULARENABLE,pRc->specular);
    printRenderState(D3DRENDERSTATE_ZVISIBLE,pRc->zVisible);
    printRenderState(D3DRENDERSTATE_SUBPIXEL,pRc->subPixel);
    //D3DRENDERSTATE_SUBPIXELX            
    //D3DRENDERSTATE_STIPPLEDALPHA      
    printRenderState(D3DRENDERSTATE_FOGCOLOR,pRc->fogColor);
    printRenderState(D3DRENDERSTATE_FOGTABLEMODE,pRc->fogTableMode);
    printRenderState(D3DRENDERSTATE_FOGTABLESTART,*(unsigned long *)&pRc->fogTableStart);
    printRenderState(D3DRENDERSTATE_FOGTABLEEND,*(unsigned long *)&pRc->fogTableEnd);
    printRenderState(D3DRENDERSTATE_FOGTABLEDENSITY,*(unsigned long *)&pRc->fogDensity);
    //D3DRENDERSTATE_STIPPLEENABLE
    //D3DRENDERSTATE_EDGEANTIALIAS
    printRenderState(D3DRENDERSTATE_COLORKEYENABLE,pRc->colorKeyEnable);
    printRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,pRc->alphaBlendEnable);
    //D3DRENDERSTATE_BORDERCOLOR
    printRenderState(D3DRENDERSTATE_TEXTUREADDRESSU,pRc->textureAddressU);
    printRenderState(D3DRENDERSTATE_TEXTUREADDRESSV,pRc->textureAddressV);
    D3DPRINT( 255, "VertexColortype %s",
             (pRc->vertexColorType == RX_VERTEX_COLOR_RGB) ? "RGB" : "RGBA" );
 
    D3DPRINT( 255, "BytesPerPixel  %d", pRc->sst.bytesPerPixel);  
    D3DPRINT( 255, "fbzMode      0x%x", pRc->sst.fbzMode);
    D3DPRINT( 255, "alphaMode    0x%x", pRc->sst.alphaMode);
    D3DPRINT( 255, "fogMode      0x%x", pRc->sst.fogMode);
    D3DPRINT( 255, "fogColor     0x%x", pRc->sst.fogColor);         
    D3DPRINT( 255, "textureMode  0x%x", pRc->sst.textureMode);
    D3DPRINT( 255, "fbzColorPath 0x%x", pRc->sst.fbzColorPath);
    D3DPRINT( 255, "tLOD         0x%x", pRc->sst.tLOD);
    D3DPRINT( 255, "baseAddr     0x%x", pRc->sst.baseAddr);
    D3DPRINT( 255, "lfbMode      0x%x", pRc->sst.lfbMode);
    D3DPRINT( 255, "scaleS       0x%x", *(long *)&pRc->sst.scaleS);
    D3DPRINT( 255, "scaleT       0x%x", *(long *)&pRc->sst.scaleT);
} 

//-------------------
// Print D3DOP_xxx type
//-------------------
static char *commandString[] = {
   "D3DOP_POINT                 ",
   "D3DOP_LINE                  ",
   "D3DOP_TRIANGLE              ",
   "D3DOP_MATRIXLOAD            ",
   "D3DOP_MATRIXMULTIPLY        ",
   "D3DOP_STATETRANSFORM        ",
   "D3DOP_STATELIGHT            ",
   "D3DOP_STATERENDER           ",
   "D3DOP_PROCESSVERTICES       ",
   "D3DOP_TEXTURELOAD           ",
   "D3DOP_EXIT                  ",      
   "D3DOP_BRANCHFORWARD         ",
   "D3DOP_SPAN                      "
} ;                                                               
/*-------------------------------------------------------------------
Function Name:  printCommand
Description:    Prints the Renderstates
Information:    void __stdcall printCommand(LPD3DINSTRUCTION lpIns, LPBYTE prim)
Return:         VOID
                
-------------------------------------------------------------------*/
void __stdcall printCommand(LPD3DINSTRUCTION lpIns, LPBYTE prim)
{
BYTE  id = lpIns->bOpcode;

   if ((id < 1) || (id > D3DOP_EXIT))
     D3DPRINT( 255, "--->D3DOP_xxx invalid instruction %d", id );
   else 
   {
     D3DPRINT( 255, "--->%s ",commandString[id - 1] );
   
     switch(id)
     {
       case D3DOP_POINT:
         break;
       case D3DOP_TRIANGLE:
         break;
       case D3DOP_LINE:
         break;  
       case D3DOP_SPAN:
         break;  
       case D3DOP_STATERENDER:
         {
           int        cnt;
           LPD3DSTATE newState = (LPD3DSTATE) prim;
         
           for (cnt = lpIns->wCount; cnt > 0; --cnt)
           {
             printRenderState(newState->drstRenderStateType, newState->dwArg[0]);
             if (cnt > 0)
               D3DPRINT( 255, "                       " );
              ++newState;
           }
         }
         break;
       case D3DOP_TEXTURELOAD:
         break;
       case D3DOP_PROCESSVERTICES:
         break;  
       default:
         break;  
     } ; // switch

   }  ;
} ;


//-------------------------------------------------------------------
/*-------------------------------------------------------------------
Function Name:  printTextureMode
Description:    does nothing
Information:    void printTextureMode( DWORD value )
Return:         void
                
-------------------------------------------------------------------*/
void printTextureMode( DWORD value )
{
}

//-------------------------------------------------------------------
/*-------------------------------------------------------------------
Function Name:  printFbzColorPath
Description:    prints the FbzColor Path setup
Information:    void printFbzColorPath( DWORD value )
Return:         void
                
-------------------------------------------------------------------*/
void printFbzColorPath( DWORD value )
{
  D3DPRINT( DLFBZCP, "FbzColorPath = 0x%08lx", value );
  
  switch( value & SST_RGBSELECT )
  {
    case SST_RGBSEL_RGBA:
      D3DPRINT( DLFBZCP, "  rgbselect = Iterated RGB" );
      break;
    case SST_RGBSEL_TMUOUT:
      D3DPRINT( DLFBZCP, "  rgbselect = Texture RGB" );
      break;
    case SST_RGBSEL_C1:
      D3DPRINT( DLFBZCP, "  rgbselect = Color1 RGB" );
      break;
    case SST_RGBSEL_LFB:
      D3DPRINT( DLFBZCP, "  rgbselect = Linear frame buffer RGB" );
      break;
  }
  
  if( value & SST_LOCALSELECT )
    D3DPRINT( DLFBZCP, "  cc_localselect = 1" );
  else      
    D3DPRINT( DLFBZCP, "  cc_localselect = 0" );

  if( value & SST_LOCALSELECT_OVERRIDE_WITH_ATEX )
    D3DPRINT( DLFBZCP, "  cc_localselect_override = Texture Alpha bit 0" );
  else      
    D3DPRINT( DLFBZCP, "  cc_localselect_override = cc_localselect" );

  if( value & SST_CC_ZERO_OTHER )
    D3DPRINT( DLFBZCP, "  cc_zero_other = 1" );
  else      
    D3DPRINT( DLFBZCP, "  cc_zero_other = 0" );

  if( value & SST_CC_SUB_CLOCAL )
    D3DPRINT( DLFBZCP, "  cc_sub_clocal = 1" );
  else      
    D3DPRINT( DLFBZCP, "  cc_sub_clocal = 0" );

  switch( value & SST_CC_MSELECT )
  {
    case SST_CC_MONE:
      D3DPRINT( DLFBZCP, "  cc_mselect = zero" );
      break;
    case SST_CC_MCLOCAL:
      D3DPRINT( DLFBZCP, "  cc_mselect = c_local" );
      break;
    case SST_CC_MAOTHER:
      D3DPRINT( DLFBZCP, "  cc_mselect = a_other" );
      break;
    case SST_CC_MALOCAL:
      D3DPRINT( DLFBZCP, "  cc_mselect = a_local" );
      break;
    case SST_CC_MATMU:
      D3DPRINT( DLFBZCP, "  cc_mselect = texture alpha" );
      break;
    case SST_CC_MRGBTMU:
      D3DPRINT( DLFBZCP, "  cc_mselect = texture RGB" );
      break;
  }

  if( value & SST_CC_REVERSE_BLEND )
    D3DPRINT( DLFBZCP, "  cc_reverse_blend = 1" );
  else      
    D3DPRINT( DLFBZCP, "  cc_reverse_blend = 0" );

  switch( value & (SST_CC_ADD_CLOCAL | SST_CC_ADD_ALOCAL) )
  {
    case 0:
      D3DPRINT( DLFBZCP, "  cc_add_clocal, cc_add_alocal = None" );
      break;
    case SST_CC_ADD_CLOCAL:
      D3DPRINT( DLFBZCP, "  cc_add_clocal, cc_add_alocal = cc_add_clocal" );
      break;
    case SST_CC_ADD_ALOCAL:
      D3DPRINT( DLFBZCP, "  cc_add_clocal, cc_add_alocal = cc_add_alocal" );
      break;
    case (SST_CC_ADD_CLOCAL | SST_CC_ADD_ALOCAL):
      D3DPRINT( DLFBZCP, "  cc_add_clocal, cc_add_alocal = None" );
      break;
  }

  if( value & SST_CC_INVERT_OUTPUT )
    D3DPRINT( DLFBZCP, "  cc_invert_ouput = 1" );
  else      
    D3DPRINT( DLFBZCP, "  cc_invert_ouput = 0" );

  switch( value & SST_ASELECT )
  {
    case SST_ASEL_RGBA:
      D3DPRINT( DLFBZCP, "  aselect = Iterated Alpha" );
      break;
    case SST_ASEL_TMUOUT:
      D3DPRINT( DLFBZCP, "  aselect = Texture Alpha" );
      break;
    case SST_ASEL_C1:
      D3DPRINT( DLFBZCP, "  aselect = Color1 Alpha" );
      break;
    case SST_ASEL_LFB:
      D3DPRINT( DLFBZCP, "  aselect = Linear frame buffer Alpha" );
      break;
  }
  
  switch( value & SST_ALOCALSELECT )
  {
    case SST_ALOCAL_ITERATOR:
      D3DPRINT( DLFBZCP, "  cca_localselect = Interated Alhpa" );
      break;
    case SST_ALOCAL_C0:
      D3DPRINT( DLFBZCP, "  cca_localselect = C0 Alpha" );
      break;
    case SST_ALOCAL_Z:
      D3DPRINT( DLFBZCP, "  cca_localselect = Iterated Z" );
      break;
    case SST_ALOCAL_W:
      D3DPRINT( DLFBZCP, "  cca_localselect = Iterated W" );
      break;
  }

  if( value & SST_CCA_ZERO_OTHER )
    D3DPRINT( DLFBZCP, "  cca_zero_other = 1" );
  else      
    D3DPRINT( DLFBZCP, "  cca_zero_other = 0" );

  if( value & SST_CCA_SUB_CLOCAL )
    D3DPRINT( DLFBZCP, "  cca_sub_clocal = 1" );
  else      
    D3DPRINT( DLFBZCP, "  cca_sub_clocal = 0" );

  switch( value & SST_CCA_MSELECT )
  {
    case SST_CC_MONE:
      D3DPRINT( DLFBZCP, "  cca_mselect = zero" );
      break;
    case SST_CC_MCLOCAL:
      D3DPRINT( DLFBZCP, "  cca_mselect = a_local" );
      break;
    case SST_CC_MAOTHER:
      D3DPRINT( DLFBZCP, "  cca_mselect = a_other" );
      break;
    case SST_CC_MALOCAL:
      D3DPRINT( DLFBZCP, "  cca_mselect = a_local" );
      break;
    case SST_CC_MATMU:
      D3DPRINT( DLFBZCP, "  cca_mselect = texture alpha" );
      break;
  }

  if( value & SST_CCA_REVERSE_BLEND )
    D3DPRINT( DLFBZCP, "  cca_reverse_blend = 1" );
  else      
    D3DPRINT( DLFBZCP, "  cca_reverse_blend = 0" );

  switch( value & (SST_CCA_ADD_CLOCAL | SST_CCA_ADD_ALOCAL) )
  {
    case 0:
      D3DPRINT( DLFBZCP, "  cca_add_clocal, cca_add_alocal = None" );
      break;
    case SST_CCA_ADD_CLOCAL:
      D3DPRINT( DLFBZCP, "  cca_add_clocal, cca_add_alocal = cca_add_clocal" );
      break;
    case SST_CCA_ADD_ALOCAL:
      D3DPRINT( DLFBZCP, "  cca_add_clocal, cca_add_alocal = cca_add_alocal" );
      break;
    case (SST_CCA_ADD_CLOCAL | SST_CCA_ADD_ALOCAL):
      D3DPRINT( DLFBZCP, "  cca_add_clocal, cca_add_alocal = None" );
      break;
  }

  if( value & SST_CC_INVERT_OUTPUT )
    D3DPRINT( DLFBZCP, "  cc_invert_ouput = 1" );
  else      
    D3DPRINT( DLFBZCP, "  cc_invert_ouput = 0" );

  if( value & SST_PARMADJUST )
    D3DPRINT( DLFBZCP, "  SubPixel Correction = Enable" );
  else      
    D3DPRINT( DLFBZCP, "  SubPixel Correction = Disable" );

  if( value & SST_ENTEXTUREMAP )
    D3DPRINT( DLFBZCP, "  Texture mapping = Enable" );
  else      
    D3DPRINT( DLFBZCP, "  Texture mapping = Disable" );

  if( value & SST_RGBAZ_CLAMP )
    D3DPRINT( DLFBZCP, "  RGBAZ Clamp = Enable" );
  else      
    D3DPRINT( DLFBZCP, "  RGBAZ Clamp = Disable" );
}

//-------------------------------------------------------------------

#if ( DX >= 6 )
/*-------------------------------------------------------------------
Function Name:  printTextureStageState
Description:    prints the Texture Stage State setup
Information:    void printTextureStageState( DWORD stage, DWORD state, DWORD value )
Return:         void
                
-------------------------------------------------------------------*/

void printTextureStageState( DWORD stage, DWORD state, DWORD value )
{
  char cState[32] = "", cValue[32] = "";
  
  switch( state )
  {
    case 0:
      D3DPRINT( DLTS, "Stage %d, TextureHandle = %ld", stage, value );
      break;
      
    case D3DTSS_COLOROP:
    case D3DTSS_ALPHAOP:
      {
        char *op;
      
        if( state == D3DTSS_COLOROP )
          op = "ColorOp";
        else
          op = "AlphaOp";
        
        switch( value )
        {
          case D3DTOP_DISABLE:
            D3DPRINT( DLTS, "Stage %d, %s = Disable", stage, op );
            break;
          case D3DTOP_SELECTARG1:
            D3DPRINT( DLTS, "Stage %d, %s = SelectArg1", stage, op );
            break;
          case D3DTOP_SELECTARG2:
            D3DPRINT( DLTS, "Stage %d, %s = SelectArg2", stage, op );
            break;
          case D3DTOP_MODULATE:
            D3DPRINT( DLTS, "Stage %d, %s = Modulate", stage, op);
            break;
          case D3DTOP_MODULATE2X:
            D3DPRINT( DLTS, "Stage %d, %s = Modulate2x", stage, op);
            break;
          case D3DTOP_MODULATE4X:
            D3DPRINT( DLTS, "Stage %d, %s = Modulate4x", stage, op );
            break;
          case D3DTOP_ADD:
            D3DPRINT( DLTS, "Stage %d, %s = Add", stage, op);
            break;
          case D3DTOP_ADDSIGNED:
            D3DPRINT( DLTS, "Stage %d, %s = AddSigned", stage, op);
            break;
          case D3DTOP_ADDSIGNED2X:
            D3DPRINT( DLTS, "Stage %d, %s = AddSigned2x", stage, op);
            break;
          case D3DTOP_SUBTRACT:
            D3DPRINT( DLTS, "Stage %d, %s = Subtract", stage, op);
            break;
          case D3DTOP_ADDSMOOTH:
            D3DPRINT( DLTS, "Stage %d, %s = AddSmooth", stage, op);
            break;
          case D3DTOP_BLENDDIFFUSEALPHA:
            D3DPRINT( DLTS, "Stage %d, %s = BlendDiffuseAlpha", stage, op);
            break;
          case D3DTOP_BLENDTEXTUREALPHA:
            D3DPRINT( DLTS, "Stage %d, %s = BlendtextureAlpha", stage, op);
            break;
          case D3DTOP_BLENDFACTORALPHA:
            D3DPRINT( DLTS, "Stage %d, %s = BlendfactorAlpha", stage, op);
            break;
          case D3DTOP_BLENDTEXTUREALPHAPM:
            D3DPRINT( DLTS, "Stage %d, %s = BlendTextureAlphaPM", stage, op);
            break;
          case D3DTOP_BLENDCURRENTALPHA:
            D3DPRINT( DLTS, "Stage %d, %s = BlendCurrentAlpha", stage, op);
            break;
          case D3DTOP_PREMODULATE:
            D3DPRINT( DLTS, "Stage %d, %s = Premodulate", stage, op);
            break;
          case D3DTOP_MODULATEALPHA_ADDCOLOR:
            D3DPRINT( DLTS, "Stage %d, %s = ModulateAlphaAddColor", stage, op);
            break;
          case D3DTOP_MODULATECOLOR_ADDALPHA:
            D3DPRINT( DLTS, "Stage %d, %s = ModulateColorAddAlpha", stage, op);
            break;
          case D3DTOP_MODULATEINVALPHA_ADDCOLOR:
            D3DPRINT( DLTS, "Stage %d, %s = ModulateInvAlphaAddColor", stage, op);
            break;
          case D3DTOP_MODULATEINVCOLOR_ADDALPHA:
            D3DPRINT( DLTS, "Stage %d, %s = ModulateInvColorAddAlpha", stage, op);
            break;
          case D3DTOP_BUMPENVMAP:
            D3DPRINT( DLTS, "Stage %d, %s = BumpEnvMat", stage, op);
            break;
          case D3DTOP_BUMPENVMAPLUMINANCE:
            D3DPRINT( DLTS, "Stage %d, %s = BumpEnvMapLuminance", stage, op);
            break;
          case D3DTOP_DOTPRODUCT3:
            D3DPRINT( DLTS, "Stage %d, %s = DotProduct3", stage, op);
            break;
        }
      }
      break;
      
    case D3DTSS_COLORARG1:
    case D3DTSS_COLORARG2:
    case D3DTSS_ALPHAARG1:
    case D3DTSS_ALPHAARG2:
      {
        char *op, *inv, *rep;
      
        switch( state )
        {
          case D3DTSS_COLORARG1:
            op = "ColorArg1";
            break;
          case D3DTSS_COLORARG2:
            op = "ColorArg2";
            break;
          case D3DTSS_ALPHAARG1:
            op = "AlphaArg1";
            break;
          case D3DTSS_ALPHAARG2:
            op = "AlphaArg2";
            break;
        }
      
        if( value & D3DTA_COMPLEMENT )
          inv = "Inverse ";
        else
          inv = "";
        
        if( value & D3DTA_ALPHAREPLICATE )
          rep = "AlphaReplicate ";
        else
          rep = "";
        
        switch( value & 0x03 )
        {
          case D3DTA_DIFFUSE:
            D3DPRINT( DLTS, "Stage %d, %s = %s%sDiffuse", stage, op, inv, rep );
            break;
          case D3DTA_CURRENT:
            D3DPRINT( DLTS, "Stage %d, %s = %s%sCurrent", stage, op, inv, rep );
            break;
          case D3DTA_TEXTURE:
            D3DPRINT( DLTS, "Stage %d, %s = %s%sTexture", stage, op, inv, rep );
            break;
          case D3DTA_TFACTOR:
            D3DPRINT( DLTS, "Stage %d, %s = %s%sFactor", stage, op, inv, rep );
            break;
        }
      }
      break;
      
    case D3DTSS_BUMPENVMAT00:
      D3DPRINT( DLTS, "Stage %d, BumpEnvMat00 = 0x%08lx", stage, value );
      break;      
    case D3DTSS_BUMPENVMAT01:
      D3DPRINT( DLTS, "Stage %d, BumpEnvMat01 = 0x%08lx", stage, value );
      break;
    case D3DTSS_BUMPENVMAT10:
      D3DPRINT( DLTS, "Stage %d, BumpEnvMat10 = 0x%08lx", stage, value );
      break;
    case D3DTSS_BUMPENVMAT11:
      D3DPRINT( DLTS, "Stage %d, BumpEnvMat11 = 0x%08lx", stage, value );
      break;

    case D3DTSS_TEXCOORDINDEX:
	  {
        char *op;
 	    switch( value & 0xffff0000)
	    {
	      case D3DTSS_TCI_PASSTHRU:
		    op = "PASSTHRU (no texgen)";
		    break;
	      case D3DTSS_TCI_CAMERASPACENORMAL:
		    op = "CAMERASPACENORMAL";
		    break;
	      case D3DTSS_TCI_CAMERASPACEPOSITION:
		    op = "CAMERASPACEPOSITION";
		    break;
	      case D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR:
		    op = "CAMERASPACEREFLECTIONVECTOR";
		    break;
		  default:
		    op = "unknown texgen";
			break;
		}
        D3DPRINT( DLTS, "Stage %ld, TCI = %ld  TexGen = %s", stage, value & 0xffff, op );
	  }
      break;
      
#if ( DX < 8 )    
    case D3DTSS_ADDRESS:
#endif
    case D3DTSS_ADDRESSU:
    case D3DTSS_ADDRESSV:
      {
        char *op;
        
        switch( state )
        {
#if ( DX < 8 )    
          case D3DTSS_ADDRESS:
            op = "Address";
            break;
#endif
          case D3DTSS_ADDRESSU:
            op = "AddressU";
            break;
          case D3DTSS_ADDRESSV:
            op = "AddressV";
            break;
        }

        switch( value )
        {        
          case D3DTADDRESS_WRAP:
            D3DPRINT( DLTS, "Stage %d, %s = Wrap", stage, op );
            break;
          case D3DTADDRESS_MIRROR:
            D3DPRINT( DLTS, "Stage %d, %s = Mirror", stage, op );
            break;
          case D3DTADDRESS_CLAMP:
            D3DPRINT( DLTS, "Stage %d, %s = Clamp", stage, op );
            break;
          case D3DTADDRESS_BORDER:
            D3DPRINT( DLTS, "Stage %d, %s = Border", stage, op );
            break;
        }
      }
      break;
      
    case D3DTSS_BORDERCOLOR:
      D3DPRINT( DLTS, "Stage %d, BorderColor, A = 0x%02x, R = 0x%02x, G = 0x%02x, B = 0x%02x", stage,
        RGBA_GETALPHA(value), RGBA_GETRED(value), RGBA_GETGREEN(value), RGBA_GETBLUE(value) );
      break;
      
    case D3DTSS_MINFILTER:
      switch( value )
      {
        case D3DTFN_POINT:
          D3DPRINT( DLTS, "Stage %d, MinFilter = Point", stage );
          break;
        case D3DTFN_LINEAR:
          D3DPRINT( DLTS, "Stage %d, MinFilter = Linear", stage );
          break;
        case D3DTFN_ANISOTROPIC:
          D3DPRINT( DLTS, "Stage %d, MinFilter = Anisotropic", stage );
          break;
      }
      break;
      
    case D3DTSS_MAGFILTER:
      switch( value )
      {
        case D3DTFG_POINT:
          D3DPRINT( DLTS, "Stage %d, MagFilter = Point", stage );
          break;
        case D3DTFG_LINEAR:
          D3DPRINT( DLTS, "Stage %d, MagFilter = Linear", stage );
          break;
        case D3DTFG_FLATCUBIC:
          D3DPRINT( DLTS, "Stage %d, MagFilter = FlatCubic", stage );
          break;
        case D3DTFG_GAUSSIANCUBIC:
          D3DPRINT( DLTS, "Stage %d, MagFilter = GaussianCubic", stage );
          break;
        case D3DTFG_ANISOTROPIC:
          D3DPRINT( DLTS, "Stage %d, MagFilter = Anisotropic", stage );
          break;
      }
      break;
      
    case D3DTSS_MIPFILTER:
      switch( value )
      {
        case D3DTFP_NONE:
          D3DPRINT( DLTS, "Stage %d, MipFilter = None", stage );
          break;
        case D3DTFP_POINT:
          D3DPRINT( DLTS, "Stage %d, MipFilter = Point", stage );
          break;
        case D3DTFP_LINEAR:
          D3DPRINT( DLTS, "Stage %d, MipFilter = Linear", stage );
          break;
      }
      break;
      
    case D3DTSS_MIPMAPLODBIAS:
      D3DPRINT( DLTS, "Stage %d, MipMapLodBias = 0x%08lx", stage, value );
      break;
      
    case D3DTSS_MAXMIPLEVEL:
      D3DPRINT( DLTS, "Stage %d, MaxMipLevel = %ld", stage, value );
      break;
      
    case D3DTSS_MAXANISOTROPY:
      D3DPRINT( DLTS, "Stage %d, MaxAnisotropy = %ld", stage, value );
      break;
      
    case D3DTSS_BUMPENVLSCALE:
      D3DPRINT( DLTS, "Stage %d, BumpEnvScale = 0x%08lx", stage, value );
      break;
      
    case D3DTSS_BUMPENVLOFFSET:
      D3DPRINT( DLTS, "Stage %d, BumpEnvlOffset = 0x%08lx", stage, value );
      break;
      
#if ( DX >= 7 )
    case D3DTSS_TEXTURETRANSFORMFLAGS:
      {
        char *op;
        switch( value )
        {
		  case D3DTTFF_DISABLE:
            op = "DISABLE (pass through)";
            break;
		  case D3DTTFF_COUNT1:
            op = "COUNT1 (1-D tex coords)";
            break;
		  case D3DTTFF_COUNT2:
            op = "COUNT2 (2-D tex coords)";
            break;
		  case D3DTTFF_COUNT3:
            op = "COUNT3 (3-D tex coords)";
            break;
		  case D3DTTFF_COUNT4:
            op = "COUNT4 (4-D tex coords)";
            break;
		  case D3DTTFF_PROJECTED:
            op = "PROJECTED (tex coords/nth element)";
            break;
		  case D3DTTFF_FORCE_DWORD:
            op = "FORCE_DWORD";
            break;
		  default:
		    op = "unknown";
			break;
	    }
        D3DPRINT( DLTS, "Stage %d, TextureTransformFlags = 0x%08lx  %s", stage, state, op );
	  }
      break;
#endif

    default:
      D3DPRINT( DLTS, "Invalid TextureStageState = %d, 0x%08lx", state, value );
      break;
  }
}

#endif // ( DX >= 6 )
//-------------------------------------------------------------------

#endif

#ifdef TXTR_PROFILE

ULONG txtrStats[TEXFMT_MAX];
/*-------------------------------------------------------------------
Function Name:  txtrFlavour
Description:    inits,prints,or updates the texture type statistics
Information:    void __stdcall txtrFlavour( DWORD format )
                format = the texture format for which to update usage count.
                format = 0xfffffffA - Init the statistics 
                format = 0xfffffffB - print the statistics
Return:         Void
                
-------------------------------------------------------------------*/
void __stdcall txtrFlavour( DWORD format )
{
  int i;
  static 
  char   *Format_Name[] = {
         "rgb332  ",                            // GR_TEXFMT_RGB_332 
         "yiq442  ",                            // GR_TEXFMT_YIQ_422
         "a8      ",                            // GR_TEXFMT_ALPHA_8
         "i8      ",                            // GR_TEXFMT_INTENSITY_8
         "ai44    ",                            // GR_TEXFMT_ALPHA_INTENSITY_44
         "p8rgb   ",                            // GR_TEXFMT_P_8
         "p8rgba  ",                            // GR_TEXFMT_RSVD1
         "rsvd2   ",                            // GR_TEXFMT_RSVD2
         "argb8332",                            // GR_TEXFMT_ARGB_8332
         "ayiq8422",                            // GR_TEXFMT_AYIQ_8422
         "rgb565  ",                            // GR_TEXFMT_RGB_565
         "argb1555",                            // GR_TEXFMT_ARGB_1555
         "argb4444",                            // GR_TEXFMT_ARGB_4444
         "ai88    ",                            // GR_TEXFMT_ALPHA_INTENSITY_88
         "ap88    ",                            // GR_TEXFMT_AP_88
         "rsvd4   ",                            // GR_TEXFMT_RSVD4
         "argb8888"                             // GR_TEXFMT_ARGB_8888 
         };
    
  // initialize the txtrStats
  if( format == 0xFFFFFFFA )
  {
    for (i=0; i<TEXFMT_MAX; ++i)
      txtrStats[i]=0;
  }
  
  // print the txtrStats
  else if ( format == 0xFFFFFFFB )
  {
    int    total = 0; 
    float  percent;

    // total primitives drawn
    for ( i = 0; i < TEXFMT_MAX; i++ )
      total += txtrStats[i];

    D3DPRINT( 0," ");
    D3DPRINT( 0, "%s ", "Application Statistics" );
    D3DPRINT( 0," ");
    D3DPRINT( 0, "%s ", "Texture Format  #Txtrs   %");
    D3DPRINT( 0, "%s ", "--------------  ------   -");
    for ( i = 0; i < TEXFMT_MAX; i++ )
    {
      if ( txtrStats[i] != 0 )
      {
        percent = ( txtrStats[i] * 100.0F ) / total;
        D3DPRINT( 0, "%s - %10ld, %4ld ", Format_Name[i], txtrStats[i], float2int(percent+0.5f)  );
      }
    }
  }
  else
    txtrStats[(format >> SST_TFORMAT_SHIFT)] += 1;

}
#endif

#if defined( FLAVOR_PROFILE )
// These variables are used to determine the triangle flavours
// used by an application.
ULONG renderStats[24][3];
int renderTypes = 0;
BOOL renderPrint = FALSE;
/*-------------------------------------------------------------------
Function Name:  triangleFlavour
Description:    Updates or prints the triangle flavour statistics
Information:    void __stdcall triangleFlavour( RC *pRc, DWORD state , DWORD type, DWORD count )
                state=0xffffffff - print the statistics
Return:         void
                
-------------------------------------------------------------------*/
void __stdcall triangleFlavour( RC *pRc, DWORD state , DWORD type, DWORD count )
{
  // see D3DGLOBAL STATE_REQUIRE_XXX for mapping
  #define NUM_ATTRIBS 18
  static unsigned char bit2char[NUM_ATTRIBS] = { 0, 'F', 'G', 'A', 0, 0, 0, '0', 0, '1', 0, 'V', 'S', 'H', 'X', 'P', 'Z', 'W' };
  static unsigned char *typeTable[] =
  {
#if ( DX == 5 )
    // execute buffer & dx5 draw primitive
    "Execute buffer   ", "Point List       ", "Line List        ", "Line Strip       ",
    "Triangle         ", "Strip            ", "Fan              ", "Indexed Triangle ",
    "Indexed Strip    ", "Indexed Fan      ",
#else
    // draw primitive2
    "DP2 TriList       ", "DP2 IndxdTriList  ", "DP2 IndxdTriList2 ", "DP2 TriStrip      ",
    "DP2 IndxdTriStrip ", "DP2 TriFanImm     ", "DP2 TriFan        ", "DP2 IndxdTriFan   ",
    "DP2 Points        ", "DP2 LineList      ", "DP2 IndxdLineList ", "DP2 IndxdLineList2",
    "DP2 LineStrip     ", "DP2 IndxdLineStrip", "DP2 LineListImm   ",
#endif
  };
  unsigned char text[128];
  int i, j, k, l, first;

  if( state != 0xFFFFFFFF )
  {
    // the iterated alpha flag is no longer updated so we will do it here.
    if (pRc->alphaBlendEnable || pRc->alphaTestEnable)
      state |= STATE_REQUIRES_IT_ALPHA;
    else state &= ~STATE_REQUIRES_IT_ALPHA;
    
    // only interested in 20 bits of the state
    state &= 0xFFFFF;
    for( i = 0; i < renderTypes; i++ )
    {
      
      if( renderStats[i][0] == (state | (type << 21)) )
      {
        renderStats[i][1] += count;
        if( count > renderStats[i][2] )
          renderStats[i][2] = count;
        break;
      }
    }

    if( i == renderTypes )
    {
      renderStats[i][0] = (state | (type << 21));
      renderStats[i][1] = count;
      renderStats[i][2] = count;
      renderTypes++;
    }
  }

  if( renderPrint || state == 0xFFFFFFFF )
  {
    int    total = 0; 
    float  percent;

    // total primitives drawn
    for ( i = 0; i < renderTypes; i++ )
      total += renderStats[i][1];

    D3DPRINT( 0," ");
    D3DPRINT( 0, "%s ", "D3D Triangle Type      Tri. Attrib.    Total     Maximum     %");
    D3DPRINT( 0, "%s ", "-----------------      -----------     -----     --------    -");
    for(l = 0; l < 15; l++)
    {
      for(k = 0, first = TRUE; k < renderTypes; k++)
      {
        if( l == (int)(renderStats[k][0] >> 21) )
        {
          for(i = NUM_ATTRIBS-1, j = 0; i >= 0; --i)
          {
            if( (renderStats[k][0] & (1 << i)) && (bit2char[i] != 0 ) )
              text[j++] = bit2char[i];
          }
          
          text[j] = 0;          
          
          percent = ( renderStats[k][1] * 100.0F ) / total;
          
          if( first )
            D3DPRINT( 0, "%s - %12s, %10ld, %10ld, %4ld ", typeTable[l], text, renderStats[k][1], renderStats[k][2], float2int(percent+0.5f)  );
          else
            D3DPRINT( 0, "                   - %12s, %10ld, %10ld, %4ld ", text, renderStats[k][1], renderStats[k][2], float2int(percent+0.5f) );
            
          first = FALSE;
        }
      }
    }

    renderPrint = FALSE;
  }
}
#endif // defined( FLAVOUR_PROFILE )

#if defined(TNL_PROFILE) && defined(TnL_HAL)

/*-------------------------------------------------------------------
Function Name:  TnLFlavour
Description:    Updates or prints the tnl flavour statistics
Information:    void __stdcall TnLFlavour( RC *pRc, DWORD state , DWORD type, DWORD count )
                type=0xfffffffa - initializes the statistics
                type=0xfffffffb - prints the statistics
Return:         void
                
-------------------------------------------------------------------*/
void __stdcall TnLFlavour( RC *pRc, DWORD type, DWORD count )
{
  #define NUM_ATTRIBS 18
  static unsigned char bit2char[NUM_ATTRIBS] = { 0, 'F', 'G', 'A', 0, 0, 0, '0', 0, '1', 0, 'V', 'S', 'H', 'X', 'P', 'Z', 'W' };
  static unsigned char *typeTable[] =
  {
    // draw primitive2
    "DP2 TriList       ", "DP2 IndxdTriList  ", "DP2 IndxdTriList2 ", "DP2 TriStrip      ",
    "DP2 IndxdTriStrip ", "DP2 TriFanImm     ", "DP2 TriFan        ", "DP2 IndxdTriFan   ",
    "DP2 Points        ", "DP2 LineList      ", "DP2 IndxdLineList ", "DP2 IndxdLineList2",
    "DP2 LineStrip     ", "DP2 IndxdLineStrip", "DP2 LineListImm   ",
  };

  if(type == 0xFFFFFFFA)
  {
     //Do init code here
  }
  else if (type == 0xFFFFFFFB)
  {
     // Do print code 
  }
  else
  {
      // gather statistics
  }
}




#define DLTLST 0
/*-------------------------------------------------------------------
Function Name:  printTLState
Description:    prints the TL State as set in the rendering context
Information:    void printTLState( DWORD value )
Return:         void
                
-------------------------------------------------------------------*/
void __stdcall printTLState( DWORD value )
{
   D3DPRINT( DLTLST, "TLSTATE = 0x%08lx", value );
  
   if( value & TLPV_DOLIGHTING )
   {
      D3DPRINT(DLTLST, "Lighting is enabled");
   }
   
   if( value & TLPV_DOCLIPPING )
   {
      D3DPRINT(DLTLST, "Clipping is enabled");
   }
   
   if( value & TLPV_GUARDBAND )
   {
      D3DPRINT(DLTLST, "Guardband clipping is enabled");
   }
   
   if( value & TLPV_DOFOG )
   {
      D3DPRINT(DLTLST, "Fog is enabled");
   }
   
   if( value & TLPV_DOSPECULAR )
   {
      D3DPRINT(DLTLST, "Specular is enabled");
   }
   
   if( value & TLPV_RANGEFOG )
   {
      D3DPRINT(DLTLST, "Range Fog is enabled");
   }
   
   if( value & TLPV_NORMALIZENORMALS )
   {
      D3DPRINT(DLTLST, "Normalize Normals is enabled");
   }
   
   if( value & TLPV_LOCALVIEWER )
   {
      D3DPRINT(DLTLST, "Local Viewer is enabled");
   }
   
  
   // Color Vertex Flags
   
   if( value & TLPV_COLORVERTEXFLAGS )
   {
      D3DPRINT(DLTLST, "All Color Vertex Flags are enabled");
   }
   else
   {
      if( value & TLPV_VERTEXDIFFUSENEEDED )
      {
         D3DPRINT(DLTLST, "Color Vertex Diffuse Needed");
      }
      if( value & TLPV_VERTEXSPECULARNEEDED )
      {
         D3DPRINT(DLTLST, "Color Vertex Specular Needed");
      }
      if( value & TLPV_COLORVERTEXAMB )
      {
         D3DPRINT(DLTLST, "Color Vertex Ambient Needed");
      }
      if( value & TLPV_COLORVERTEXDIFF )   // JJP What is this ?
      {
         D3DPRINT(DLTLST, "Color Vertex Diff Needed");
      }
      if( value & TLPV_COLORVERTEXSPEC )   // JJP What is this ?
      {
         D3DPRINT(DLTLST, "Color Vertex Spec Needed");
      }
      if( value & TLPV_COLORVERTEXEMIS )
      {
         D3DPRINT(DLTLST, "Color Vertex Emmissive Needed");
      }
   }
   
   if( value & TLPV_COLORVERTEXNEEDED )
   {
      D3DPRINT(DLTLST, "Vertex Color Needed Set");
   }
   
   if( value & TLPV_VERTEXBLENDNEEDED )
   {
      D3DPRINT(DLTLST, "Vertex Blend Needed");
   }
  
   if( value & TLPV_VALIDCLIPBUFFER )
   {
      D3DPRINT(DLTLST, "Clip Buffer is Valid");
   }
   if( value & TLPV_TLNEEDED )
   {
      D3DPRINT(DLTLST, "Incoming Buffer requires Transformation");
   }
}

void printTLColor(TLCOLOR *ptlc, char *name)
{
   D3DPRINT(0,"TLCOLOR for %s is", name);
   D3DPRINT(0,"red = %s, blue = %s, green = %s", float2String(ptlc->r), 
                                                 float2String(ptlc->g), 
                                                 float2String(ptlc->b) );
}   


void printD3DVector(D3DVECTOR *pD3DV, char *name)
{
   D3DPRINT(0, "D3DVECTOR %s = ", name);
   D3DPRINT(0, "x = %s, y = %s, z = %s", float2String(pD3DV->x),
                                         float2String(pD3DV->y),
                                         float2String(pD3DV->z) );
}   

/*-------------------------------------------------------------------
Function Name:  printLightState
Description:    prints the Light State
Information:    void printLightState( DWORD stage, DWORD state, DWORD value )
Return:         void
                
-------------------------------------------------------------------*/

void __stdcall printLightStates( RC *pRC )
{
  TLLIGHTING *pLight;
  
  pLight = &(pRC->tl.lighting);
  
  printD3DVector(&pLight->eye_in_eye, "eye_in_eye");

  // Ma * La + Me (Ambient and Emissive) ------
  printTLColor(&pLight->ambEmiss, "ambEmiss");

  // ColorVertex stuff ------------------------
  printTLColor(pLight->pAmbientSrc,  "AmbientSrc");         
  printTLColor(pLight->pDiffuseSrc,  "DiffuseSrc");         
  printTLColor(pLight->pSpecularSrc, "SpecularSrc");         
  printTLColor(pLight->pEmissiveSrc, "EmissiveSrc");         

  // Diffuse ----------------------------------
  printTLColor(&pLight->vertexDiffuse, "vertexDiffuse");
  // COLORVERTEX is enabled and a diffuse
  // color is provided in the vertex
  
  printTLColor(&pLight->fDiffuse, "diffuse");
  D3DPRINT(0,"outDiffuse = 0x%08x",	pLight->dwDiffuse);    		// Diffuse color result of lighting


  // Specular --------------------------------
  
  printTLColor(&pLight->vertexSpecular, "vertexSpecular");
  // COLORVERTEX is enabled and a specular
  // color is provided in the vertex
  printTLColor(&pLight->fSpecular, "specular");
  D3DPRINT(0,"outSpecular = 0x%08x", pLight->dwSpecular);    		// Diffuse color result of lighting
  D3DPRINT(0,"specThreshold = %s", float2String(pLight->specThreshold));
  // value, specular factor is zero
  // End of temporary data

  // RENDERSTATEAMBIENT --------------------------------------

  // Ambient color set by D3DRENDERSTATE_AMBIENT
  // They are all scaled to 0 - 1
  D3DPRINT(0,"Ambient color");
  D3DPRINT(0,"red = %s, blue = %s, green = %s", float2String(pLight->ambient_red), 
                                                float2String(pLight->ambient_green), 
                                                float2String(pLight->ambient_blue) );
  D3DPRINT(0,"ambient_save = 0x%08x", pLight->ambient_save);    		// Diffuse color result of lighting
  
  // Fog -----------------------------------------------------

  D3DPRINT(0,"fog_mode = %d", pLight->fog_mode);
  D3DPRINT(0,"fog_range_enable = %d", pLight->fog_range_enable);
  
  D3DPRINT(0,"fog_color = 0x%08x", pLight->fog_color);
  
  D3DPRINT(0,"fog_density = %s", float2String(pLight->fog_density) );
  D3DPRINT(0,"fog_start = %s", float2String(pLight->fog_start) );
  D3DPRINT(0,"fog_end = %s", float2String(pLight->fog_end) );
  D3DPRINT(0,"fog_factor = %s", float2String(pLight->fog_factor) );

  D3DPRINT(0,"color_model = 0x%08x", pLight->color_model);

  // Material ------------------------------------------------

  // For color material
  D3DPRINT(0, "DiffuseAlphaSrc = 0x%08x", *(pLight->pDiffuseAlphaSrc));
  D3DPRINT(0, "DiffuseAlphaSrc = 0x%08x", *(pLight->pSpecularAlphaSrc));
  D3DPRINT(0, "materialDiffAlpha = 0x%08x", pLight->materialDiffAlpha);
  
  // alpha (0-255) shifted left
  // by 24 bits

  D3DPRINT(0, "materialSpecAlpha = 0x%08x", pLight->materialSpecAlpha);
  
  // alpha (0-255) shifted left
  // by 24 bits
  D3DPRINT(0, "vertexDiffAlpha = 0x%08x", pLight->vertexDiffAlpha);
  
  // alpha (0-255) shifted left
  // by 24 bits
  
  D3DPRINT(0, "vertexSpecAlpha = 0x%08x", pLight->vertexSpecAlpha);
  // alpha (0-255) shifted left
  // by 24 bits

  // JJP D3DMATERIAL7    material;           // Cached material data
  printTLColor(&pLight->matAmb, "matAmb");
  printTLColor(&pLight->matDiff, "matDiff");
  printTLColor(&pLight->matSpec, "matSpec");
  printTLColor(&pLight->matEmis, "matEmis");
}

#define MAX_LIGHT_TYPES  4
void __stdcall printLightTypes(RC *pRc)
{
   TLLIGHT *pLight = pRc->tl.lighting.pActiveLights;
   DWORD   LTCount[MAX_LIGHT_TYPES];
   
   ZeroMemory( LTCount, (MAX_LIGHT_TYPES * sizeof(DWORD)) );
   while (pLight)
   {
      LTCount[pLight->Light.dltType]++;
      pLight = pLight->Next;
   }
   
   D3DPRINT(0,"Light Type Statistics");
   D3DPRINT(0,"=====================");
   D3DPRINT(0,"Num D3DLIGHT_POINT = %d", LTCount[1]);
   D3DPRINT(0,"Num D3DLIGHT_SPOT = %d", LTCount[2]);
   D3DPRINT(0,"Num D3DLIGHT_DIRECTIONAL = %d", LTCount[3]);
}

#define XFORMMATS_MAX  24
DWORD MatrixStatData[XFORMMATS_MAX];
/*-------------------------------------------------------------------
Function Name:  MatrixStats
Description:    inits,prints,or updates the transform matrix statistics
Information:    void __stdcall MatrixStats( DWORD dwxfrmType )
                format = the texture format for which to update usage count.
                dwxfrmType = 0xfffffffA - Init the statistics 
                dwxfrmType = 0xfffffffB - print the statistics
                dwxfrmType = 0x0        - record an update operation
Return:         Void
                
-------------------------------------------------------------------*/

void __stdcall MatrixStats(DWORD dwxfrmType)
{
   int i;
   static 
   char   *Matrix_Name[] = {
          "0 (undefined)",
          "World ",                       //   D3DTRANSFORMSTATE_WORLD
          "View  ",                       //   D3DTRANSFORMSTATE_VIEW      
          "Proj  ",                       //   D3DTRANSFORMSTATE_PROJECTION
          "World1",                       //   D3DTRANSFORMSTATE_WORLD1    
          "World2",                       //   D3DTRANSFORMSTATE_WORLD2    
          "World3",                       //   D3DTRANSFORMSTATE_WORLD3
          "7 (undefined)",
          "8 (undefined)",
          "9 (undefined)",
          "10 (undefined)",
          "11 (undefined)",
          "12 (undefined)",
          "13 (undefined)",
          "14 (undefined)",
          "15 (undefined)",
          "Txtr0 ",                       //   D3DTRANSFORMSTATE_TEXTURE0  
          "Txtr1 ",                       //   D3DTRANSFORMSTATE_TEXTURE1  
          "Txtr2 ",                       //   D3DTRANSFORMSTATE_TEXTURE2  
          "Txtr3 ",                       //   D3DTRANSFORMSTATE_TEXTURE3  
          "Txtr4 ",                       //   D3DTRANSFORMSTATE_TEXTURE4  
          "Txtr5 ",                       //   D3DTRANSFORMSTATE_TEXTURE5  
          "Txtr6 ",                       //   D3DTRANSFORMSTATE_TEXTURE6  
          "Txtr7 "                        //   D3DTRANSFORMSTATE_TEXTURE7  
          };

   if( dwxfrmType == 0xFFFFFFFA )
   {
      for (i = 1; i < XFORMMATS_MAX; i++)
      {
         MatrixStatData[i]=0;
      }
   }
   else if ( dwxfrmType == 0xFFFFFFFB )
   {
      D3DPRINT( 0," ");
      D3DPRINT( 0, "%s ", "Application Statistics" );
      D3DPRINT( 0," ");
      D3DPRINT( 0, "%s ", "Matrix Name  #Changes");
      D3DPRINT( 0, "%s ", "-----------  -------- ");
      for ( i = 1; i < XFORMMATS_MAX; i++ )
      {
         // Since they are all set once, only track changes after 
         // they are first set
         if ( MatrixStatData[i] > 1 )  
         {
            D3DPRINT( 0, "%s    - %10ld", Matrix_Name[i], MatrixStatData[i]  );
         } 
      }
      
      D3DPRINT( 0, "");
      D3DPRINT( 0, "# Updts  -  %10ld", MatrixStatData[0]);
   }
   else
   {
      MatrixStatData[dwxfrmType]++;
   }
}   

#define CLIPPLANES_MAX  17
DWORD TriClipData[CLIPPLANES_MAX];
/*-------------------------------------------------------------------
Function Name:  MatrixStats
Description:    inits,prints,or updates the transform matrix statistics
Information:    void __stdcall MatrixStats( DWORD dwxfrmType )
                format = the texture format for which to update usage count.
                dwxClipCode = 0xfffffffA - Init the statistics 
                dwxClipCode = 0xfffffffB - print the statistics
Return:         Void
                
-------------------------------------------------------------------*/

void __stdcall TriClipStats(DWORD dwClipCode)
{
   int i;
   static 
   char   *ClipPlane_Name[] = {
          "",
          "Left     ",                      
          "Right    ",                    
          "Top      ",                    
          "Bottom   ",                    
          "Front    ",                    
          "Back     ",                    
          "User 0   ",
          "User 1   ",
          "User 2   ",
          "User 3   ",
          "User 4   ",
          "User 5   ",
          "GB Left  ",
          "GB Right ",
          "GB Top   ",    
          "GB Bottom"
          };

   if( dwClipCode == 0xFFFFFFFA )
   {
      for (i = 1; i < CLIPPLANES_MAX; i++)
      {
         TriClipData[i] = 0;
      }
   }
   else if ( dwClipCode == 0xFFFFFFFB )
   {
      D3DPRINT( 0," ");
      D3DPRINT( 0, "%s ", "Triangle Clip Statistics" );
      D3DPRINT( 0, "%s ", "Clip Plane     #Changes");
      D3DPRINT( 0, "%s ", "-----------    -------- ");
      for ( i = 1; i < CLIPPLANES_MAX; i++ )
      {
         // Since they are all set once, only track changes after 
         // they are first set
         if ( TriClipData[i] > 0 )  
         {
            D3DPRINT( 0, "%s  - %10ld", ClipPlane_Name[i], TriClipData[i] );
         } 
      }
   }
   else
   {
      if(dwClipCode & TLCLIP_LEFT )
      {
         TriClipData[TLCLIP_LEFTBIT]++;
      }
      
      if(dwClipCode & TLCLIP_RIGHT )
      {
         TriClipData[TLCLIP_RIGHTBIT]++;
      }
      
      if(dwClipCode & TLCLIP_TOP )
      {
         TriClipData[TLCLIP_TOPBIT]++;
      }
      
      if(dwClipCode & TLCLIP_BOTTOM )
      {
         TriClipData[TLCLIP_BOTTOMBIT]++;
      }
      
      if(dwClipCode & TLCLIP_FRONT )
      {
         TriClipData[TLCLIP_FRONTBIT]++;
      }
      
      if(dwClipCode & TLCLIP_BACK )
      {
         TriClipData[TLCLIP_BACKBIT]++;
      }
      
      if(dwClipCode & TLCLIP_USERCLIPPLANE0 )
      {
         TriClipData[TLCLIP_USERCLIPLANE0BIT]++;
      }
      
      if(dwClipCode & TLCLIP_USERCLIPPLANE1 )
      {
         TriClipData[TLCLIP_USERCLIPLANE1BIT]++;
      }
      
      if(dwClipCode & TLCLIP_USERCLIPPLANE2 )
      {
         TriClipData[TLCLIP_USERCLIPLANE2BIT]++;
      }
      
      if(dwClipCode & TLCLIP_USERCLIPPLANE3 )
      {
         TriClipData[TLCLIP_USERCLIPLANE3BIT]++;
      }
      
      if(dwClipCode & TLCLIP_USERCLIPPLANE4 )
      {
         TriClipData[TLCLIP_USERCLIPLANE4BIT]++;
      }
      
      if(dwClipCode & TLCLIP_USERCLIPPLANE5 )
      {
         TriClipData[TLCLIP_USERCLIPLANE5BIT]++;
      }
      
      if(dwClipCode & TLCLIPGB_LEFT )
      {
         TriClipData[TLCLIPGB_LEFTBIT]++;
      }
      
      if(dwClipCode & TLCLIPGB_RIGHT )
      {
         TriClipData[TLCLIPGB_RIGHTBIT]++;
      }
      
      if(dwClipCode & TLCLIPGB_TOP )
      {
         TriClipData[TLCLIPGB_TOPBIT]++;
      }
      
      if(dwClipCode & TLCLIPGB_BOTTOM )
      {
         TriClipData[TLCLIPGB_BOTTOMBIT]++;
      }
   }
}   



typedef struct _FVFSTATS 
{
   DWORD dwFVFFlags;
   DWORD dwCount;
} FVFSTATS;

#define NUM_FVF2TRACK 1000
FVFSTATS FVFStats[NUM_FVF2TRACK];

void __stdcall InitFVFStats(DWORD value)
{
   int i;
   
   for (i = 0;i < NUM_FVF2TRACK; i++)
   {
      FVFStats[i].dwFVFFlags = 0L;
      FVFStats[i].dwCount    = 0L;
   }
}   

void __stdcall UpdateFVFStats(DWORD dwFVFFlags)
{
   int i = 0;
   int bContinue = TRUE;
    
   while (bContinue == TRUE)
   {
      if( FVFStats[i].dwFVFFlags == 0)
      {
         // Reached an empty entry before finding a used FVF entry
         // so create one and set the count to 1.
         FVFStats[i].dwFVFFlags = dwFVFFlags;
         FVFStats[i].dwCount = 1;
         bContinue = FALSE;
      }
      else if(FVFStats[i].dwFVFFlags == dwFVFFlags)
      {
         // Found an existing FVF entry so increment the counter
         FVFStats[i].dwCount++;
         bContinue = FALSE;
      }
      else
      {
         // Move to the next entry
         i++;
      }
   }  
}   

void __stdcall PrintFVFStats(DWORD value)
{
   int i = 0;
   int bContinue = TRUE;
   
   D3DPRINT( 0," ");
   D3DPRINT( 0, "%s ", "FVF Statistics" );
   D3DPRINT( 0," ");
   D3DPRINT( 0, "%s ", "FVF Flags    #Uses");
   D3DPRINT( 0, "%s ", "-----------  -------- ");
   
   while(bContinue == TRUE)
   {
      if(FVFStats[i].dwCount == 0)
      {
         bContinue = FALSE;
      }
      else
      {
         D3DPRINT( 0, "0x%x    - %10ld", FVFStats[i].dwFVFFlags, FVFStats[i].dwCount );
         i++;
      }
   }
}  

 
#endif //TNL_PROFILE

#ifdef FILEIO

#if 0
//example on how to use file i/o from DirectX
{
extern void trcCreateFile();
extern void trcEnable(int flag);
extern void trcWriteFile(char *data, int len);
extern void trcCloseFile();

trcCreateFile();
trcEnable(1);
trcWriteFile("hello world", sizeof("hello world"));
trcCloseFile();

}
#endif



HANDLE  hTraceFile = INVALID_HANDLE_VALUE;
int     traceEnable = 0;
int     formatIndex;

void trcCreateFile()
{
  // open/create file
  if (hTraceFile == INVALID_HANDLE_VALUE)
  {
    hTraceFile = CreateFile("c:\\fxtrace",
                           GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_WRITE_THROUGH,
                           NULL);

  }
  if (INVALID_HANDLE_VALUE == hTraceFile)
  {
    hTraceFile = INVALID_HANDLE_VALUE;
  }
}

void trcEnable(int flag)
{
  traceEnable = flag;
}

void trcWriteFile(char *data, int len)
{
  DWORD   dwNumBytesWritten;

  if (    (hTraceFile != INVALID_HANDLE_VALUE) 
       && (traceEnable) )
  {
    // write something to file
    if (FALSE == WriteFile( hTraceFile,
                              data,
                              len,
                              &dwNumBytesWritten,
                              NULL))
      trcEnable(0);                          
    else FlushFileBuffers( hTraceFile );
  }
}

void trcCloseFile()
{
  if (hTraceFile != INVALID_HANDLE_VALUE)
    CloseHandle(hTraceFile);
  hTraceFile = INVALID_HANDLE_VALUE;
}


void __cdecl trcPrint(LPSTR szFormat, ...)
{
    char    buffer[256];

    wvsprintf(buffer, szFormat, (LPVOID)(&szFormat+1));
    trcWriteFile(buffer, lstrlen(buffer));
}


void trcWrite8(FxU8 data)
{
    FxU8 byte[1];

    byte[0] = (FxU8) ((data     ) & 0xFF);

    trcWriteFile(byte, 1);
}

void trcWrite16(FxU16 data)
{
    FxU8 byte[2];

    byte[0] = (FxU8) ((data >> 8) & 0xFF);
    byte[1] = (FxU8) ((data     ) & 0xFF);

    trcWriteFile(byte, 2);
}

/* Write long word, msb first */

void trcWrite32(FxU32 data)
{
    FxU8 byte[4];

    byte[0] = (FxU8) ((data >> 24) & 0xFF);
    byte[1] = (FxU8) ((data >> 16) & 0xFF);
    byte[2] = (FxU8) ((data >>  8) & 0xFF);
    byte[3] = (FxU8) ((data      ) & 0xFF);

    trcWriteFile(byte, 4);
}

void txWrite16(FxU16 data)
{
    FxU8 byte[2];

    byte[1] = (FxU8) ((data >> 8) & 0xFF);
    byte[0] = (FxU8) ((data     ) & 0xFF);
    trcWriteFile(byte, 2);
}

/* Write long word, msb first */

void txWrite32(FxU32 data)
{
    FxU8 byte[4];

    byte[3] = (FxU8) ((data >> 24) & 0xFF);
    byte[2] = (FxU8) ((data >> 16) & 0xFF);
    byte[1] = (FxU8) ((data >>  8) & 0xFF);
    byte[0] = (FxU8) ((data      ) & 0xFF);
    trcWriteFile(byte, 4);
}


void trcWrite3dfPalTable(LPPALETTEENTRY pal)
{
  int     i;
  ULONG   data;

  for (i=0; i<256; i++) 
  {
    data = (pal[i].peRed << 16) | (pal[i].peGreen << 8) | (pal[i].peBlue);
    trcWrite32(data);
  }
}

void trcWrite3DFhdr(DWORD slog, DWORD tlog, DWORD ar, DWORD format, 
                    DWORD numMipmaps, DWORD lTlod, LPDDRAWI_DDRAWSURFACE_LCL surfLCL)
{
  char    hdr[] = "3df v%s\n%s\nlod range: %d %d\naspect ratio: %s\n";
  static  char *Version = "1.1";
  static  char *aspect_names[]  = { "1 1", "2 1", "4 1", "8 1" };
  static  char *aspect_names2[] = { "1 1", "1 2", "1 4", "1 8" };
  int     smallLod, largeLod;
  char   *arName;
  char   *Format_Name[] = {
         "rgb332",                              // GR_TEXFMT_RGB_332 
         "yiq442",                              // GR_TEXFMT_YIQ_422
         "a8    ",                              // GR_TEXFMT_ALPHA_8
         "i8    ",                              // GR_TEXFMT_INTENSITY_8
         "ai44  ",                              // GR_TEXFMT_ALPHA_INTENSITY_44
         "p8rgb ",                              // GR_TEXFMT_P_8
         "p8rgba",                              // GR_TEXFMT_RSVD1
         "rsvd2 ",                              // GR_TEXFMT_RSVD2
         "argb8332",                            // GR_TEXFMT_ARGB_8332
         "ayiq8422",                            // GR_TEXFMT_AYIQ_8422
         "rgb565",                              // GR_TEXFMT_RGB_565
         "argb1555",                            // GR_TEXFMT_ARGB_1555
         "argb4444",                            // GR_TEXFMT_ARGB_4444
         "ai88",                                // GR_TEXFMT_ALPHA_INTENSITY_88
         "ap88",                                // GR_TEXFMT_AP_88
         "rsvd4",                               // GR_TEXFMT_RSVD4
         "argb8888"                             // GR_TEXFMT_ARGB_8888 
         };
  

  if (    (hTraceFile != INVALID_HANDLE_VALUE) 
       && (traceEnable) )
  {
  // printf("Writing header...\n");
  if (tlog > slog)
  {
    largeLod = 1 << tlog;
    smallLod = 1 << (tlog - (numMipmaps - 1));
  }
  else
  {
    largeLod = 1 << slog;
    smallLod = 1 << (slog - (numMipmaps - 1));
  }
  
  switch (format)
  {
     case TEXFMT_ARGB_4444 << SST_TFORMAT_SHIFT:
       formatIndex = 12;
       break; 
     case TEXFMT_ARGB_1555 << SST_TFORMAT_SHIFT:
       formatIndex = 11;
       break;
     case TEXFMT_RGB_565 << SST_TFORMAT_SHIFT: 
       formatIndex = 10;
       break;
     case TEXFMT_ARGB_8332 << SST_TFORMAT_SHIFT:
       formatIndex = 8;
       break;
     case TEXFMT_RGB_332 << SST_TFORMAT_SHIFT:
       formatIndex = 0;
       break;
     case TEXFMT_P8 << SST_TFORMAT_SHIFT:
       formatIndex = 5;
       break;
  }
 
  if (lTlod & SST_LOD_S_IS_WIDER)
       arName=aspect_names[ar];
  else arName=aspect_names2[ar];
  
  trcPrint((LPVOID)(&hdr),
            Version, 
            Format_Name[formatIndex],
            smallLod,
            largeLod,
            arName);
            
  if (formatIndex == 5)
    trcWrite3dfPalTable(surfLCL->lpDDPalette->lpLcl->lpGbl->lpColorTable);              
  }
}
#endif
