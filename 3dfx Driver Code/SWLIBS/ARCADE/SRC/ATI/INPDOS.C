/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 7:33:34 PM$ 
**
*/

#include <dos.h>
#include <conio.h>
#include <3dfx.h>
#include "atinput.h"

extern AtiState atiState;

#define KEYBOARD_INT 0x09

typedef void (interrupt far *KeyISR)(void);
void interrupt far KeyboardISR( void );

static KeyISR oldKeyISR;

FxBool InitDos(void) {
    SetupKeyboardISR();

    return FXTRUE;

}

void interrupt far KeyboardISR( void ) {
    static FxU32    event;
    static FxU8     scan_code;
    static FxBool   ext_flag=FXFALSE;

    scan_code = inp(0x60);

    event = atiState.lastEvent + 1;
    if ( event > ATI_MAX_NUM_EVENTS ) event = 0;
    
    if (scan_code==0xE0)
        ext_flag=FXTRUE;
    else {
        atiState.queue[event].device       = ATI_DEV_KEYBOARD;
        if (ext_flag==FXTRUE) {
            atiState.queue[event].ev.key.code  = scan_code|0x80;
            atiState.queue[event].ev.key.state = 
                (scan_code&0x80)?ATI_KEY_RELEASE:ATI_KEY_PRESS;
        } else {
            atiState.queue[event].ev.key.code  = scan_code&0x7F;
            atiState.queue[event].ev.key.state = 
                (scan_code&0x80)?ATI_KEY_RELEASE:ATI_KEY_PRESS;
        }            
        ext_flag=FXFALSE;
    }
    atiState.lastEvent = event;
    outp(0x20,0x20);     
}

void SetupKeyboardISR(void) {
    /* Need to clean buffer */
    /* Setup Keyboard ISR */

    oldKeyISR=_dos_getvect(KEYBOARD_INT);
    _dos_setvect(KEYBOARD_INT,KeyboardISR);     
}

void CloseKeyboardISR(void) {
    _dos_setvect(KEYBOARD_INT,oldKeyISR);
}
