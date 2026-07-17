#
# Copyright (c) 1998, 3Dfx Interactive, Inc.
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
# the contents of this file may not be disclosed to third parties, copied or
# duplicated in any form, in whole or in part, without the prior written
# permission of 3Dfx Interactive, Inc.
# 
# RESTRICTED RIGHTS LEGEND:
# Use, duplication or disclosure by the Government is subject to restrictions
# as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
# and Computer Software clause at DFARS 252.227-7013, and/or in similar or
# successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
# rights reserved under the Copyright Laws of the United States.
# 
# 
# $Revision: $ 
# $Date: $ 
# 
#

#----------------------------------------------------------------------
#                    Banshee register addresses
#----------------------------------------------------------------------

# package definition

package h3regs;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(%h3port %h3reg %h3cfg %h3portx %h3regx %h3cfgx);
@EXPORT_OK = qw();

# i/o port register addresses

%h3port = (
 	STATUS				=> 0x00000000,
	PCIINIT0			=> 0x00000004,
	SIPMONITOR			=> 0x00000008,
	LFBMEMORYCONFIG			=> 0x0000000c,
	MISCINIT0			=> 0x00000010,
	MISCINIT1			=> 0x00000014,
	DRAMINIT0			=> 0x00000018,
	DRAMINIT1			=> 0x0000001c,

	AGPINIT				=> 0x00000020,
	TMUGBEINIT			=> 0x00000024,
	VGAINIT0			=> 0x00000028,
	VGAINIT1			=> 0x0000002c,
	DRAMCOMMAND			=> 0x00000030,
	DRAMDATA			=> 0x00000034,
	RESERVEDZ_0			=> 0x00000038,
	VIDTVOUTBLANKVCOUNT		=> 0x0000003c,

	PLLCTRL0			=> 0x00000040,
	PLLCTRL1			=> 0x00000044,
	PLLCTRL2			=> 0x00000048,
	DACMODE				=> 0x0000004c,
	DACADDR				=> 0x00000050,
	DACDATA				=> 0x00000054,
	VIDMAXRGBDELTA			=> 0x00000058,
	VIDPROCCFG			=> 0x0000005c,

	HWCURPATADDR			=> 0x00000060,
	HWCURLOC			=> 0x00000064,
	HWCURC0				=> 0x00000068,
	HWCURC1				=> 0x0000006c,
	VIDINFORMAT			=> 0x00000070,
	VIDTVOUTBLANKHCOUNT		=> 0x00000074,
	VIDSERIALPARALLELPORT		=> 0x00000078,
	VIDINXDECIMDELTAS		=> 0x0000007c,

	VIDINDECIMINITERRS		=> 0x00000080,
	VIDINYDECIMDELTAS		=> 0x00000084,
	VIDPIXELBUFTHOLD		=> 0x00000088,
	VIDCHROMAMIN			=> 0x0000008c,
	VIDCHROMAMAX			=> 0x00000090,
	VIDINSTATUSCURRENTLINE		=> 0x00000094,
	VIDSCREENSIZE			=> 0x00000098,
	VIDOVERLAYSTARTCOORDS		=> 0x0000009c,

        VIDOVERLAYENDCOORDS             => 0x000000a0,
	VIDOVERLAYDUDX			=> 0x000000a4,
	VIDOVERLAYDUDXOFFSETSRCWIDTH	=> 0x000000a8,
	VIDOVERLAYDVDY			=> 0x000000ac,
	VGAREGISTER_0			=> 0x000000b0,
	VGAREGISTER_1			=> 0x000000b4,
	VGAREGISTER_2			=> 0x000000b8,
	VGAREGISTER_3			=> 0x000000bc,

	VGAREGISTER_4			=> 0x000000c0,
	VGAREGISTER_5			=> 0x000000c4,
	VGAREGISTER_6			=> 0x000000c8,
	VGAREGISTER_7			=> 0x000000cc,
	VGAREGISTER_8			=> 0x000000d0,
	VGAREGISTER_9			=> 0x000000d4,
	VGAREGISTER_10			=> 0x000000d8,
	VGAREGISTER_11			=> 0x000000dc,

	VIDOVERLAYDVDYOFFSET		=> 0x000000e0,
	VIDDESKTOPSTARTADDR		=> 0x000000e4,
	VIDDESKTOPOVERLAYSTRIDE		=> 0x000000e8,
	VIDINADDR0			=> 0x000000ec,
	VIDINADDR1			=> 0x000000f0,
	VIDINADDR2			=> 0x000000f4,
	VIDINSTRIDE			=> 0x000000f8,
	VIDCURROVERLAYSTARTADDR		=> 0x000000fc,

);

# memory-mapped register addresses

%h3reg = (

	AGPREQSIZE			=> 0x00080000,
	HOSTADDRLOW			=> 0x00080004,
	HOSTADDRHIGH			=> 0x00080008,
	GRAPHICSADDR			=> 0x0008000c,
	GRAPHICSSTRIDE			=> 0x00080010,
	MOVECMD				=> 0x00080014,
	RESERVEDL_0			=> 0x00080018,
	RESERVEDL_1			=> 0x0008001c,

	CMD0_BASEADDRL			=> 0x00080020,
	CMD0_BASESIZE			=> 0x00080024,
	CMD0_BUMP			=> 0x00080028,
        CMD0_READPTRL                   => 0x0008002c,
        CMD0_READPTRH                   => 0x00080030,
	CMD0_AMIN			=> 0x00080034,
	CMD0_AMAX			=> 0x0008003c,
	CMD0_STATUS0			=> 0x00080040,
	CMD0_DEPTH			=> 0x00080044,
	CMD0_HOLECOUNT			=> 0x00080048,

	CMD1_BASEADDRL			=> 0x00080050,
	CMD1_BASESIZE			=> 0x00080054,
	CMD1_BUMP			=> 0x00080058,
	CMD1_READPTRL			=> 0x0008005c,
	CMD1_READPTRH			=> 0x00080060,
	CMD1_AMIN			=> 0x00080064,
	CMD1_AMAX			=> 0x0008006c,
	CMD1_DEPTH			=> 0x00080074,
	CMD1_HOLECOUNT			=> 0x00080078,

	CMDFIFOTHRESH			=> 0x00080080,
	RESERVEDO_0			=> 0x00080084,
	RESERVEDO_1			=> 0x00080088,
	RESERVEDO_2			=> 0x0008008c,
	RESERVEDO_3			=> 0x00080090,
	RESERVEDO_4			=> 0x00080094,
	RESERVEDO_5			=> 0x00080098,
	RESERVEDO_6			=> 0x0008009c,

	RESERVEDP_0			=> 0x000800a0,
	RESERVEDP_1			=> 0x000800a4,
	RESERVEDP_2			=> 0x000800a8,
	RESERVEDP_3			=> 0x000800ac,
	RESERVEDP_4			=> 0x000800b0,
	RESERVEDP_5			=> 0x000800b4,
	RESERVEDP_6			=> 0x000800b8,
	RESERVEDP_7			=> 0x000800bc,

	RESERVEDQ_0			=> 0x000800c0,
	RESERVEDQ_1			=> 0x000800c4,
	RESERVEDQ_2			=> 0x000800c8,
	RESERVEDQ_3			=> 0x000800cc,
	RESERVEDQ_4			=> 0x000800d0,
	RESERVEDQ_5			=> 0x000800d4,
	RESERVEDQ_6			=> 0x000800d8,
	RESERVEDQ_7			=> 0x000800dc,

	RESERVEDR_0			=> 0x000800e0,
	RESERVEDR_1			=> 0x000800e4,
	RESERVEDR_2			=> 0x000800e8,
	RESERVEDR_3			=> 0x000800ec,
	RESERVEDR_4			=> 0x000800f0,
	RESERVEDR_5			=> 0x000800f4,
	RESERVEDR_6			=> 0x000800f8,
	RESERVEDR_7			=> 0x000800fc,

	YUVBASEADDR			=> 0x00080100,
	YUVSTRIDE			=> 0x00080104,

	STATUS2D			=> 0x00100000,
	CLIP0MIN			=> 0x00100008,
	CLIP0MAX			=> 0x0010000c,
	DSTBASEADDR			=> 0x00100010,
	DSTFORMAT			=> 0x00100014,
        SRCCOLORKEYMIN                  => 0x00100018,
        SRCCOLORKEYMAX                  => 0x0010001c,
        DSTCOLORKEYMIN                  => 0x00100020,
        DSTCOLORKEYMAX                  => 0x00100024,
	ROP				=> 0x00100030,
        SRCBASEADDR                     => 0x00100034,
	COMMANDEX			=> 0x00100038,
	LINESTIPPLE			=> 0x0010003c,
	LINESTYLE			=> 0x00100040,
        PATTERN0ALIAS                   => 0x00100044,
        PATTERN1ALIAS                   => 0x00100048,
	CLIP1MIN			=> 0x0010004c,
	CLIP1MAX			=> 0x00100050,
        SRCFORMAT                       => 0x00100054,
	SRCSIZE				=> 0x00100058,
	SRCXY				=> 0x0010005c,
	COLORBACK			=> 0x00100060,
	COLORFORE			=> 0x00100064,
	DSTSIZE				=> 0x00100068,
	DSTXY				=> 0x0010006c,
	COMMAND				=> 0x00100070,
	LAUNCH				=> 0x00100080,
	COLORPATTERN			=> 0x00100100,

	STATUS				=> 0x00200000,
	INTRCTRL			=> 0x00200004,
	VA_X				=> 0x00200008,
	VA_Y				=> 0x0020000c,
	VB_X				=> 0x00200010,
	VB_Y				=> 0x00200014,
	VC_X				=> 0x00200018,
	VC_Y				=> 0x0020001c,

	R				=> 0x00200020,
	G				=> 0x00200024,
	B				=> 0x00200028,
	Z				=> 0x0020002c,
	A				=> 0x00200030,
	S				=> 0x00200034,
	T				=> 0x00200038,
	W				=> 0x0020003c,

	DRDX				=> 0x00200040,
	DGDX				=> 0x00200044,
	DBDX				=> 0x00200048,
	DZDX				=> 0x0020004c,
	DADX				=> 0x00200050,
	DSDX				=> 0x00200054,
	DTDX				=> 0x00200058,
	DWDX				=> 0x0020005c,

	DRDY				=> 0x00200060,
	DGDY				=> 0x00200064,
	DBDY				=> 0x00200068,
	DZDY				=> 0x0020006c,
	DADY				=> 0x00200070,
	DSDY				=> 0x00200074,
	DTDY				=> 0x00200078,
	DWDY				=> 0x0020007c,

	TRIANGLECMD			=> 0x00200080,
	RESERVEDA			=> 0x00200084,
	FVA_X				=> 0x00200088,
	FVA_Y				=> 0x0020008c,
	FVB_X				=> 0x00200090,
	FVB_Y				=> 0x00200094,
	FVC_X				=> 0x00200098,
	FVC_Y				=> 0x0020009c,

	FR				=> 0x002000a0,
	FG				=> 0x002000a4,
	FB				=> 0x002000a8,
	FZ				=> 0x002000ac,
	FA				=> 0x002000b0,
	FS				=> 0x002000b4,
	FT				=> 0x002000b8,
	FW				=> 0x002000bc,

	FDRDX				=> 0x002000c0,
	FDGDX				=> 0x002000c4,
	FDBDX				=> 0x002000c8,
	FDZDX				=> 0x002000cc,
	FDADX				=> 0x002000d0,
	FDSDX				=> 0x002000d4,
	FDTDX				=> 0x002000d8,
	FDWDX				=> 0x002000dc,

	FDRDY				=> 0x002000e0,
	FDGDY				=> 0x002000e4,
	FDBDY				=> 0x002000e8,
	FDZDY				=> 0x002000ec,
	FDADY				=> 0x002000f0,
	FDSDY				=> 0x002000f4,
	FDTDY				=> 0x002000f8,
	FDWDY				=> 0x002000fc,

	R_ALT				=> 0x00200020,
	DRDX_ALT			=> 0x00200024,
	DRDY_ALT			=> 0x00200028,
	G_ALT				=> 0x0020002c,
	DGDX_ALT			=> 0x00200030,
	DGDY_ALT			=> 0x00200034,
	B_ALT				=> 0x00200038,
	DBDX_ALT			=> 0x0020003c,

	DBDY_ALT			=> 0x00200040,
	Z_ALT				=> 0x00200044,
	DZDX_ALT			=> 0x00200048,
	DZDY_ALT			=> 0x0020004c,
	A_ALT				=> 0x00200050,
	DADX_ALT			=> 0x00200054,
	DADY_ALT			=> 0x00200058,
	S_ALT				=> 0x0020005c,

	DSDX_ALT			=> 0x00200060,
	DSDY_ALT			=> 0x00200064,
	DT_ALT				=> 0x00200068,
	DTDX_ALT			=> 0x0020006c,
	DTDY_ALT			=> 0x00200070,
	W_ALT				=> 0x00200074,
	DWDX_ALT			=> 0x00200078,
	DWDY_ALT			=> 0x0020007c,

	FR_ALT				=> 0x002000a0,
	FDRDX_ALT			=> 0x002000a4,
	FDRDY_ALT			=> 0x002000a8,
	FG_ALT				=> 0x002000ac,
	FDGDX_ALT			=> 0x002000b0,
	FDGDY_ALT			=> 0x002000b4,
	FB_ALT				=> 0x002000b8,
	FDBDX_ALT			=> 0x002000bc,

	FDBDY_ALT			=> 0x002000c0,
	FZ_ALT				=> 0x002000c4,
	FDZDX_ALT			=> 0x002000c8,
	FDZDY_ALT			=> 0x002000cc,
	FA_ALT				=> 0x002000d0,
	FDADX_ALT			=> 0x002000d4,
	FDADY_ALT			=> 0x002000d8,
	FS_ALT				=> 0x002000dc,

	FDSDX_ALT			=> 0x002000e0,
	FDSDY_ALT			=> 0x002000e4,
	FT_ALT				=> 0x002000e8,
	FDTDX_ALT			=> 0x002000ec,
	FDTDY_ALT			=> 0x002000f0,
	FW_ALT				=> 0x002000f4,
	FDWDX_ALT			=> 0x002000f8,
	FDWDY_ALT			=> 0x002000fc,

	FTRIANGLECMD			=> 0x00200100,
	FBZCOLORPATH			=> 0x00200104,
	FOGMODE				=> 0x00200108,
	ALPHAMODE			=> 0x0020010c,
	FBZMODE				=> 0x00200110,
	LFBMODE				=> 0x00200114,
	CLIPLEFTRIGHT			=> 0x00200118,
	CLIPBOTTOMTOP			=> 0x0020011c,

	NOPCMD				=> 0x00200120,
	FASTFILLCMD			=> 0x00200124,
	SWAPBUFFERCMD			=> 0x00200128,
	FOGCOLOR			=> 0x0020012c,
	ZACOLOR				=> 0x00200130,
	CHROMAKEY			=> 0x00200134,
	CHROMARANGE			=> 0x00200138,
	USERINTRCMD			=> 0x0020013c,

	STIPPLE				=> 0x00200140,
	C0				=> 0x00200144,
	C1				=> 0x00200148,
	FBIPIXELSIN			=> 0x0020014c,
	FBICHROMAFAIL			=> 0x00200150,
	FBIZFUNCFAIL			=> 0x00200154,
	FBIAFUNCFAIL			=> 0x00200158,
	FBIPIXELSOUT			=> 0x0020015c,

	FOGTABLE			=> 0x00200160,

	RENDERMODE                      => 0x002001e0,
	STENCILMODE                     => 0x002001e4,
        STENCILOP                       => 0x002001e8,
	COLBUFFERADDR			=> 0x002001ec,
	COLBUFFERSTRIDE			=> 0x002001f0,
	AUXBUFFERADDR			=> 0x002001f4,
	AUXBUFFERSTRIDE			=> 0x002001f8,
        FBISTENCILFAIL                  => 0x000001fc,

	CLIPLEFTRIGHT1			=> 0x00200200,
	CLIPBOTTOMTOP1			=> 0x00200204,
        COMBINEMODE                     => 0x00200208,

	SWAPBUFFERPEND			=> 0x0020024c,
	LEFTOVERLAYBUF			=> 0x00200250,
	RIGHTOVERLAYBUF			=> 0x00200254,
	FBISWAPHISTORY			=> 0x00200258,
	FBITRIANGLESOUT			=> 0x0020025c,

	SSETUPMODE			=> 0x00200260,
	SVX				=> 0x00200264,
	SVY				=> 0x00200268,
	SARGB				=> 0x0020026c,
	SRED				=> 0x00200270,
	SGREEN				=> 0x00200274,
	SBLUE				=> 0x00200278,
	SALPHA				=> 0x0020027c,

	SVZ				=> 0x00200280,
	SOOWFBI				=> 0x00200284,
	SOOW0				=> 0x00200288,
	SSOW0				=> 0x0020028c,
	STOW0				=> 0x00200290,
	SOOW1				=> 0x00200294,
	SSOW1				=> 0x00200298,
	STOW1				=> 0x0020029c,
	SDRAWTRICMD			=> 0x002002a0,
	SBEGINTRICMD			=> 0x002002a4,

	TEXTUREMODE			=> 0x00200300,
	TLOD				=> 0x00200304,
	TDETAIL				=> 0x00200308,
	TEXBASEADDR			=> 0x0020030c,
	TEXBASEADDR1			=> 0x00200310,
	TEXBASEADDR2			=> 0x00200314,
	TEXBASEADDR38			=> 0x00200318,
	TREXINIT0			=> 0x0020031c,

	TREXINIT1			=> 0x00200320,
	NCCTABLE0			=> 0x00200324,
	NCCTABLE1			=> 0x00200354,

	TCHROMAKEYMIN			=> 0x00200384,
	TCHROMAKEYMAX			=> 0x00200388,

);

while (($key,$value) = each %h3port) {    # i/o registers also appear in memory-mapped space
    $h3reg{$key} = $value;
}

# configuration register addresses

%h3cfg = (

	VENDORID        		=> 0,
	DEVICEID        		=> 2,
	PCICOMMAND      		=> 4,
	PCISTATUS       		=> 6,
	REVISIONID      		=> 8,
	CLASSCODE       		=> 9,
	CACHELINESIZE   		=> 12,
	LATENCYTIMER    		=> 13,
	HEADERTYPE      		=> 14,
	BIST            		=> 15,
	MEMBASEADDR0    		=> 16,
	MEMBASEADDR1    		=> 20,
	IOBASEADDR      		=> 24,
	RESBASEADDR0    		=> 28,
	RESBASEADDR1    		=> 32,
	RESBASEADDR2    		=> 36,
	CISPOINTER      		=> 40,
	SUBVENDORID     		=> 44,
	SUBSYSTEMID     		=> 46,

	ROMBASEADDR     		=> 48,
	CAPPTR          		=> 52,
	MISCRES0          		=> 56,    

	INTERRUPTLINE   		=> 60,
	INTERRUPTPIN    		=> 61,
	MINGNT          		=> 62,
	MAXLAT          		=> 63,

	FABID           		=> 64,
	CFGSTATUS       		=> 76,
	CFGSCRATCH      		=> 80,

	AGPCAPID        		=> 84,
	AGPCAPIDNEXT    		=> 85,
	AGPCAPIDOTHER   		=> 86,
	AGPSTATUS       		=> 88,
	AGPCMD          		=> 92,

	ACPICAPID       		=> 96,
	ACPICAPIDNEXT   		=> 97,
	ACPICAPIDPMC    		=> 98,
	ACPISTATUS      		=> 100,
	
);

# cross-references keyed by address

%h3portx = reverse %h3port;
%h3regx = reverse %h3reg;
%h3cfgx = reverse %h3cfg;

1;
