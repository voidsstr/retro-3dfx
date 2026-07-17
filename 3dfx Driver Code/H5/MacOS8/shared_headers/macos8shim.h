/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   Mac9platform.h
**
** Description: h3 mode initialization code (derived from h3vdd in win9x)
**
**
*/

#ifndef __MAC9PLATFORM_H__
#define __MAC9PLATFORM_H__

#define FAR far

typedef FxU32 DWORD;
typedef FxU16 WORD;
typedef FxU8  BYTE;
typedef void  VOID;
typedef FxU8  BOOL;

#define Round round
#define FloatToInt (FxU32)
#define FPU_FUNCTION_RESTORE 1
#define FPU_State 0 && (unsigned long)

#define FPU_FUNCTION_SAVE 1


#define TRUE FXTRUE
#define FALSE FXFALSE

#endif /* __MAC9PLATFORM_H__ */
