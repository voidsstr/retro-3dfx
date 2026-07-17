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
extern void __fastcall __glStoreSpanCINDEX(unsigned int mask, __GLtri *tr);
extern void __fastcall __glStoreSpanRGB332(unsigned int mask, __GLtri *tr);
extern void __fastcall __glStoreSpanRGB5(unsigned int mask, __GLtri *tr);
extern void __fastcall __glStoreSpanRGB565(unsigned int mask, __GLtri *tr);
extern void __fastcall __glStoreSpanRGB8(unsigned int mask, __GLtri *tr);
extern void __fastcall __glStoreSpanXRGB8(unsigned int mask, __GLtri *tr);

extern __GLspanlet __glStoreTab[];
