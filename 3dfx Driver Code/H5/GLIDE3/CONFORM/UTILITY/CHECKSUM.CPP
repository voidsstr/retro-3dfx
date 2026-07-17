/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
*/

/*
 * Snarfed from ogtest source. Only the names have been changed
 * to protect the innoc^H^H^H^H^H coding convention.
 */

/*
 * A 32 bit CRC algorithm derived from the 16 bit version in
 * Numerical Recipes 2nd Ed., pp 900-901.  Uses the polynomial
 * x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 +
 * x^7 + x^5 + x^4 + x^2 + x^1 + 1.
 */

/*
 * mask corresponds to the polynomial (excludes 32nd bit)
 */
#define CRCMASK 0x04c11db7

/*----------------------------------------------------------------------------
@func crcinit
@date 10/07/98
@arg
@return 
@imp
@key
@sect
@html
Support function for compute_checksum()
@end
----------------------------------------------------------------------------*/
static unsigned int
crcinit(unsigned int crc)
{
    int i;
    unsigned int ans = crc;

    for (i=0; i < 8; i++) {
	if (ans & 0x80000000) {
	    ans = (ans << 1) ^ CRCMASK;
	} else {
	    ans <<= 1;
	}
    }
    return ans;
}

static unsigned int crctab[256];

/*----------------------------------------------------------------------------
@func compute_checksum
@date 10/07/98
@arg u_char *bufp - pointer to the thing you want to compute a checksum on
@arg int len - the size (in bytes) of what *bufp points to
@return the checksum
@imp
@key
@sect
@html
Compute a checksum on a buffer
@end
----------------------------------------------------------------------------*/
unsigned int
compute_checksum(unsigned char *bufp, int len)
{
    unsigned int cword = (unsigned int)~0;
    static int crcinited = 0;
    int i;
    
    if (!crcinited) {
	/* 
	 * Initialize a lookup table for the 8 most significant bits of the
	 * cumulative remainder.  This way we can do the division 8 bits at 
	 * a time, instead of 1 at a time.
	 */
	for (i=0; i < 256; i++) {
	    crctab[i] = crcinit(i << 24);
	}
	crcinited = 1;
    } 
    for (i=0; i < len; i++) {
	cword = crctab[ bufp[i] ^ (cword >> 24) ] ^ (cword << 8);
    }
    return cword;
}

