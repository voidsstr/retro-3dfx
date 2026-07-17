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
**
** $Revision: 4$ 
** $Date: 10/11/00 7:40:04 PM$ 
**
*/

#include <windows.h>
#include <winioctl.h>
#include <WinRTctl.h>

extern HANDLE        hWinRT;
static unsigned long iWinRTlength;

unsigned int  pciOutp( int port, int value ) {
/*%%
  #OnError ERROR_HANDLER
  #SetAbsolute On
  	outpb( port, value );
  %%
*/
{
	WINRT_CONTROL_ITEM _WinRTpp01[] =
	{/*	command	param1	param2 */
		{OUTP_BA,	0,	0},
	};
		_WinRTpp01[ 0].port = port;
		_WinRTpp01[ 0].value = value;
	if (!WinRTProcessIoBuffer(hWinRT, _WinRTpp01,
				sizeof(_WinRTpp01), &iWinRTlength))
		 goto ERROR_HANDLER;
}
	
	return value;

ERROR_HANDLER:
	return 0;
}

unsigned int pciOutpw( int port, unsigned short value ) {
/*%%
  #OnError ERROR_HANDLER
  #SetAbsolute On
  	outpw( port, value );
  %%
*/
{
	WINRT_CONTROL_ITEM _WinRTpp02[] =
	{/*	command	param1	param2 */
		{OUTP_WA,	0,	0},
	};
		_WinRTpp02[ 0].port = port;
		_WinRTpp02[ 0].value = value;
	if (!WinRTProcessIoBuffer(hWinRT, _WinRTpp02,
				sizeof(_WinRTpp02), &iWinRTlength))
		 goto ERROR_HANDLER;
}
	
	return value;

ERROR_HANDLER:
	return 0;
}


unsigned long pciOutpd( int port, unsigned long value ) {
/*%%
  #OnError ERROR_HANDLER
  #SetAbsolute On
  	outpl( port, value );
  %%
*/
{
	WINRT_CONTROL_ITEM _WinRTpp03[] =
	{/*	command	param1	param2 */
		{OUTP_LA,	0,	0},
	};
		_WinRTpp03[ 0].port = port;
		_WinRTpp03[ 0].value = value;
	if (!WinRTProcessIoBuffer(hWinRT, _WinRTpp03,
				sizeof(_WinRTpp03), &iWinRTlength))
		 goto ERROR_HANDLER;
}
	
	return value;

ERROR_HANDLER:
	return 0;
}

unsigned int  pciInp( int port ) {
	unsigned int retval;
/*%%
  #OnError ERROR_HANDLER
  #SetAbsolute On
  	retval = inpb( port );
  %%
*/
{
	WINRT_CONTROL_ITEM _WinRTpp04[] =
	{/*	command	param1	param2 */
		{INP_BA,	0,	0},
	};
		_WinRTpp04[ 0].port = port;
	if (!WinRTProcessIoBuffer(hWinRT, _WinRTpp04,
				sizeof(_WinRTpp04), &iWinRTlength))
		 goto ERROR_HANDLER;
	retval = _WinRTpp04[ 0].value;
}
	
	return retval;

ERROR_HANDLER:
	return 0;
}

unsigned int  pciInpw( int port ) {
	unsigned int retval;
/*%%
  #OnError ERROR_HANDLER
  #SetAbsolute On
  	retval = inpw( port );
  %%
*/
{
	WINRT_CONTROL_ITEM _WinRTpp05[] =
	{/*	command	param1	param2 */
		{INP_WA,	0,	0},
	};
		_WinRTpp05[ 0].port = port;
	if (!WinRTProcessIoBuffer(hWinRT, _WinRTpp05,
				sizeof(_WinRTpp05), &iWinRTlength))
		 goto ERROR_HANDLER;
	retval = _WinRTpp05[ 0].value;
}
	
	return retval;

ERROR_HANDLER:
	return 0;
}

unsigned long pciInpd( int port ) {
	unsigned long retval;
/*%%
  #OnError ERROR_HANDLER
  #SetAbsolute On
  	retval = inpl( port );
  %%
*/
{
	WINRT_CONTROL_ITEM _WinRTpp06[] =
	{/*	command	param1	param2 */
		{INP_LA,	0,	0},
	};
		_WinRTpp06[ 0].port = port;
	if (!WinRTProcessIoBuffer(hWinRT, _WinRTpp06,
				sizeof(_WinRTpp06), &iWinRTlength))
		 goto ERROR_HANDLER;
	retval = _WinRTpp06[ 0].value;
}
	
	return retval;

ERROR_HANDLER:
	return 0;

}

