/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
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
*/
#include "context.h"
#include "global.h"
#include "xform.h"
#include "string.h"
#include "fr_modes.h"
#include "fr_tri.h"

#undef __GL_USE_INTEL_ASM

#ifdef __GL_USE_INTEL_ASM
#pragma warning(disable : 4035)
#define SAVE_EBP dword ptr [esp-4]
#define SAVE_EBX dword ptr [esp-8]
#define SAVE_ECX dword ptr [esp-12]
#define SAVE_ESI dword ptr [esp-16]
#define SAVE_EDI dword ptr [esp-20]
#endif
