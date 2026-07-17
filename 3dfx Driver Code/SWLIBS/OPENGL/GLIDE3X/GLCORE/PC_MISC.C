/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 7:55:43 PM$
*/
#ifdef __GL_PC_RAST



void __glpcSetFPUPrecision( int bits )

{

    unsigned short cw;



    __asm fnstcw cw

    __asm fwait



    switch ( bits )

    {

    case 23:

        cw &= ~0x300;

        break;

    case 53:

        cw = ( cw & ~0x300 ) | 0x200;

        break;

    case 64:

        cw |= 0x300;

        break;

    }



    __asm fldcw cw

}



#endif

