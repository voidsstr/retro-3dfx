#ifndef __HWCPIO_H__
#define __HWCPIO_H__

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
** $Revision: 4$
** $Date: 10/11/00 7:40:15 PM$
*/

/* -------------------------------------------------- */
/* Direct port I/O macros: XXX fix NT case            */

#   if defined( __WATCOMC__ )
#       include <conio.h>
        /* xxx how do you find the version # ? */
#       if 1
#           define _pioInByte(port) inp(port)
#           define _pioInWord(port) inpw(port)
#           define _pioInLong(port) inpd(port)
#           define _pioOutByte(port,data) outp(port, data)
#           define _pioOutWord(port,data) outpw(port, data)
#           define _pioOutLong(port,data) outpd(port, data)
#       else
#           error Unknown Watcom C version
#       endif /* WATCOM versions */
#   elif defined( __MSC__ )
#       include <conio.h>
#       if _MSC_VER == 900         /* MS VC 2.0 */
#           define _pioInByte(port) inp(port)
#           define _pioInWord(port) inpw(port)
#           define _pioInLong(port) _inpd(port)
#           define _pioOutByte(port,data) _outp(port, data)
#           define _pioOutWord(port,data) outpw(port, data)
#           define _pioOutLong(port,data) _outpd(port, data)
#       elif _MSC_VER == 1000      /* MS VC 4.0 */
            /* xxx check this (the underscores) out to make sure */
#           define _pioInByte(port) _inp(port)
#           define _pioInWord(port) _inpw(port)
#           define _pioInLong(port) _inpd(port)
#           define _pioOutByte(port,data) _outp(port, data)
#           define _pioOutWord(port,data) _outpw(port, data)
#           define _pioOutLong(port,data) _outpd(port, data)
#       elif _MSC_VER == 1010      /* MS VC 4.1 */
            /* xxx check this (the underscores) out to make sure */
#           define _pioInByte(port) _inp(port)
#           define _pioInWord(port) _inpw(port)
#           define _pioInLong(port) _inpd(port)
#           define _pioOutByte(port,data) _outp(port, data)
#           define _pioOutWord(port,data) _outpw(port, data)
#           define _pioOutLong(port,data) _outpd(port, data)
#       elif _MSC_VER == 1020      /* MS VC 4.2 */
            /* xxx check this (the underscores) out to make sure */
#           define _pioInByte(port) _inp(port)
#           define _pioInWord(port) _inpw(port)
#           define _pioInLong(port) _inpd(port)
#           define _pioOutByte(port,data) _outp(port, data)
#           define _pioOutWord(port,data) _outpw(port, data)
#           define _pioOutLong(port,data) _outpd(port, data)
#       elif _MSC_VER == 1100      /* MS VC 5.0 */
            /* xxx check this (the underscores) out to make sure */
#           define _pioInByte(port) _inp(port)
#           define _pioInWord(port) _inpw(port)
#           define _pioInLong(port) _inpd(port)
#           define _pioOutByte(port,data) _outp(port, data)
#           define _pioOutWord(port,data) _outpw(port, data)
#           define _pioOutLong(port,data) _outpd(port, data)
#       elif _MSC_VER == 1200      /* MS VC 6.0 */
            /* xxx check this (the underscores) out to make sure */
#           define _pioInByte(port) _inp(port)
#           define _pioInWord(port) _inpw(port)
#           define _pioInLong(port) _inpd(port)
#           define _pioOutByte(port,data) _outp(port, data)
#           define _pioOutWord(port,data) _outpw(port, data)
#           define _pioOutLong(port,data) _outpd(port, data)
#       else
#           error Unknown Microsoft VC version
#       endif /* MSC versions */
#   elif defined( __DJGPP__ )
#       if ( __DJGPP__ == 2 ) && ( __DJGPP_MINOR__ == 0 )
            /* DJGPP 2.0 beta */
#           define _pioInByte(port) inportb(port)
#           define _pioInWord(port) inportw(port)
#           define _pioInLong(port) inportl(port)
#           define _pioOutByte(port,data) outportb(port, data)
#           define _pioOutWord(port,data) outportw(port, data)
#           define _pioOutLong(port,data) outportl(port, data)
#       else
#           error Unknown DJGPP version
#       endif /* DJGPP versions */
#   elif defined( __unix__ )
#       define _pioInByte(port) unixError()
#       define _pioInWord(port) unixError()
#       define _pioInLong(port) unixError()
#       define _pioOutByte(port,data) unixError()
#       define _pioOutWord(port,data) unixError()
#       define _pioOutLong(port,data) unixError()
#   else
#       error Unknown compiler
#   endif /* compilers */
#endif __HWCPIO_H__
