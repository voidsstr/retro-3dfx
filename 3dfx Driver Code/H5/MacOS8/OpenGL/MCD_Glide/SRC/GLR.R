/*________________________________________________________________________________________
** 
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
**________________________________________________________________________________________
**
**  Description: resources
**
** 
**
*/

#include <MacTypes.r>

#include "DrvVersion.h"

resource 'vers' (1, purgeable) {
	kOPENGL_MAJOR_REV,
	kOPENGL_MINOR_REV,
	kOPENGL_STAGE_REV,
	kOPENGL_LEVEL_REV,
	kOPENGL_REGION_CODE,
	kOPENGL_VERSION,
	kOPENGL_VER_FULL
};

resource 'vers' (2) {
	kOPENGL_MAJOR_REV,
	kOPENGL_MINOR_REV,
	kOPENGL_STAGE_REV,
	kOPENGL_LEVEL_REV,
	kOPENGL_REGION_CODE,
	kOPENGL_VERSION,
	kOPENGL_DESCRIPTION
};

data 'CCIª' (128) {
	$"4F70 656E 474C 2066 6F72 2074 6865 204D"            /* OpenGL for the M */
	$"6163 4F53 0D0D 456E 6162 6C65 7320 796F"            /* acOSÂÂEnables yo */
	$"7520 746F 2063 7265 6174 652C 2076 6965"            /* u to create, vie */
	$"772C 2073 6176 652C 2061 6E64 2064 6972"            /* w, save, and dir */
	$"6563 746C 7920 696E 7465 7261 6374 2077"            /* ectly interact w */
	$"6974 6820 7468 7265 652D 6469 6D65 6E73"            /* ith three-dimens */
	$"696F 6E61 6C20 6772 6170 6869 6373 2069"            /* ional graphics i */
	$"6E20 6170 706C 6963 6174 696F 6E73 2074"            /* n applications t */
	$"6861 7420 7375 7070 6F72 7420 4F70 656E"            /* hat support Open */
	$"474C 2E"                                            /* GL. */
};

data 'ftag' (0) {
	$"0201 0080 0012 4F70 656E 474C 5265 6E64"            /* ...€..OpenGLRend */
	$"6572 6572 3364 6678 0000 0000 0000 0000"            /* erer3dfx........ */
	$"0000 0000 0000 0000"                                /* ........ */
};


data 'icl4' (-16455) {
	$"0FFF FFFF FFF0 0000 0000 0FFF FFFF FFF0"            /* .ÿÿÿÿð.....ÿÿÿÿð */
	$"F000 0000 0F00 0000 0000 00F0 0000 000F"            /* ð..........ð.... */
	$"F0CC CCC0 F000 0000 0000 000F CCCC CCCF"            /* ðÌÌÀð.......ÌÌÌÏ */
	$"F0CC CCCC 0FFF FFFF FFFF FFF0 CCCC CCCF"            /* ðÌÌÌ.ÿÿÿÿÿÿðÌÌÌÏ */
	$"F0CC CCCC C000 0000 0000 0000 CCCC CCCF"            /* ðÌÌÌÀ.......ÌÌÌÏ */
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"            /* ðÌÌÌÌÌÌÌÌÌÌÌÌÌÌÏ */
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"            /* ðÌÌÌÌÌÌÌÌÌÌÌÌÌÌÏ */
	$"F0CC CCCC CCCC CCCC C222 2222 CCCC CCCF"            /* ðÌÌÌÌÌÌÌÂ"""ÌÌÌÏ */
	$"F0CC CCCC CCCC CC22 2CCC CCC2 2CCC CCCF"            /* ðÌÌÌÌÌÌ",ÌÌÂ,ÌÌÏ */
	$"F0CC CCCC CCCC 22CC CCCC CCCC 22CC CCCF"            /* ðÌÌÌÌÌ"ÌÌÌÌÌ"ÌÌÏ */
	$"F0CC CCCC CC22 CCCC CCCC CCCC 222C CCCF"            /* ðÌÌÌÌ"ÌÌÌÌÌÌ",ÌÏ */
	$"F0CC CCCC C2CC CCCC CCCC CCCC C22C CCCF"            /* ðÌÌÌÂÌÌÌÌÌÌÌÂ,ÌÏ */
	$"F0CC CCCC CCCC CCEE CCDE ECCC C22C CCCF"            /* ðÌÌÌÌÌÌîÌÞìÌÂ,ÌÏ */
	$"F0CC CCCC CCCC CCEE CCAE DCCC C22C CCCF"            /* ðÌÌÌÌÌÌîÌ®ÜÌÂ,ÌÏ */
	$"F0CC CCCC CCCC CCEE CDAD CCCC C22C CCCF"            /* ðÌÌÌÌÌÌîÍ­ÌÌÂ,ÌÏ */
	$"F0CC CCCD CCCC DCEE CDAD CCCC C22C CCCF"            /* ðÌÌÍÌÌÜîÍ­ÌÌÂ,ÌÏ */
	$"F0CC CCAA ECCD AEEE DEEA DDAC C22C CCCF"            /* ðÌÌªìÍ®îÞêÝ¬Â,ÌÏ */
	$"F0CC CEDC ADCA DCEE CDAD CCEE 22CC CCCF"            /* ðÌÎÜ­ÊÜîÍ­Ìî"ÌÌÏ */
	$"F0CC CCCC ADDA DCEE CDAD CCEE 22CC CCCF"            /* ðÌÌÌ­ÚÜîÍ­Ìî"ÌÌÏ */
	$"F0CC CCCE ECDA DCEE CDAD CCCB 2CCC CCCF"            /* ðÌÌÎìÚÜîÍ­ÌË,ÌÌÏ */
	$"F0CC CCDA ECDA DCEE CDAD CCC2 2CCC CCCF"            /* ðÌÌÚìÚÜîÍ­ÌÂ,ÌÌÏ */
	$"F0CC CCCC EEDA DCEE CDAD CC22 ECCC CCCF"            /* ðÌÌÌîÚÜîÍ­Ì"ìÌÌÏ */
	$"F0CC CDCC EEDA DCEE CDAD CC22 EDCC CCCF"            /* ðÌÍÌîÚÜîÍ­Ì"íÌÌÏ */
	$"F0CC CEEE ADCE EDAE CDAD C22C EACC CCCF"            /* ðÌÎî­Îí®Í­Â,êÌÌÏ */
	$"F0CC CCEE DCCD EDDD CCEC C2CC DEDC CCCF"            /* ðÌÌîÜÍíÝÌìÂÌÞÜÌÏ */
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"            /* ðÌÌÌÌÌÌÌÌÌÌÌÌÌÌÏ */
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"            /* ðÌÌÌÌÌÌÌÌÌÌÌÌÌÌÏ */
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"            /* ðÌÌÌÌÌÌÌÌÌÌÌÌÌÌÏ */
	$"0FFF FFFF FFFF CCCC CCCC FFFF FFFF FFF0"            /* .ÿÿÿÿÿÌÌÌÌÿÿÿÿÿð */
	$"0000 0000 00F0 CCCC CCCC CF00 0000 0000"            /* .....ðÌÌÌÌÏ..... */
	$"0000 0000 0F0C CCCC CCCC CCF0 0000 0000"            /* ......ÌÌÌÌÌð.... */
	$"0000 0000 00FF FFFF FFFF FF00 0000 0000"            /* .....ÿÿÿÿÿÿ..... */
};

data 'icl8' (-16455) {
	$"00FF FFFF FFFF FFFF FFFF FF00 0000 0000"            /* .ÿÿÿÿÿÿÿÿÿÿ..... */
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FF00"            /* .....ÿÿÿÿÿÿÿÿÿÿ. */
	$"FF00 0000 0000 0000 00FF 0000 0000 0000"            /* ÿ........ÿ...... */
	$"0000 0000 0000 FF00 0000 0000 0000 00FF"            /* ......ÿ........ÿ */
	$"FF00 F6F6 F6F6 F600 FF00 0000 0000 0000"            /* ÿ.ööööö.ÿ....... */
	$"0000 0000 0000 00FF F6F6 F6F6 F6F6 F7FF"            /* .......ÿöööööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 00FF FFFF FFFF FFFF"            /* ÿ.öööööö.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 F6F6 F6F6 F6F6 F7FF"            /* ÿÿÿÿÿÿÿ.öööööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F600 0000 0000 0000"            /* ÿ.ööööööö....... */
	$"0000 0000 0000 0000 F6F6 F6F6 F6F6 F7FF"            /* ........öööööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"            /* ÿ.öööööööööööööö */
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"            /* öööööööööööööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"            /* ÿ.öööööööööööööö */
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"            /* öööööööööööööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"            /* ÿ.öööööööööööööö */
	$"0708 0F0F 1016 100F F6F6 F6F6 F6F6 F7FF"            /* ........öööööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F607 0708"            /* ÿ.ööööööööööö... */
	$"0707 F6F6 F607 0E17 1607 F6F6 F6F6 F7FF"            /* ..ööö.....öööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 0707 F6F6"            /* ÿ.öööööööööö..öö */
	$"F6F6 F6F6 F6F6 F607 1716 F6F6 F6F6 F7FF"            /* ööööööö...öööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 0707 F6F6 F6F6"            /* ÿ.öööööööö..öööö */
	$"F6F6 F6F6 F6F6 F6F6 0F17 0FF6 F6F6 F7FF"            /* öööööööö...ööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F607 F6F6 F6F6 F6F6"            /* ÿ.ööööööö.öööööö */
	$"F6F6 F62B F6F6 F6F6 0717 16F6 F6F6 F7FF"            /* ööö+öööö...ööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBFB"            /* ÿ.ööööööööööööûû */
	$"F6F6 81AC FBF6 F6F6 F616 17F6 F6F6 F7FF"            /* öö¬ûöööö..ööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 ACAC"            /* ÿ.öööööööööööö¬¬ */
	$"F6F8 FDAC 56F6 F6F6 F616 17F6 F6F6 F7FF"            /* öøý¬Vöööö..ööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 ACAC"            /* ÿ.öööööööööööö¬¬ */
	$"F656 FD56 F6F6 F6F6 F616 17F6 F6F6 F7FF"            /* öVýVööööö..ööö÷ÿ */
	$"FF00 F6F6 F6F6 F756 2BF6 F6F6 56F7 FCFC"            /* ÿ.öööö÷V+öööV÷üü */
	$"2BFA FDFA 2BF7 F7F6 0717 16F6 F6F6 F7FF"            /* +úýú+÷÷ö...ööö÷ÿ */
	$"FF00 F6F6 F6F7 FDFD FC2B F681 FDFC ACFC"            /* ÿ.ööö÷ýýü+öýü¬ü */
	$"FAAC ACFD 8156 FDF8 0817 0FF6 F6F6 F7FF"            /* ú¬¬ýVýø...ööö÷ÿ */
	$"FF00 F6F6 F6FB FAF8 FDF9 F7FD 812B ACFC"            /* ÿ.öööûúøýù÷ý+¬ü */
	$"F7FA FD81 F72B ACFC 1017 07F6 F6F6 F7FF"            /* ÷úý÷+¬ü...ööö÷ÿ */
	$"FF00 F6F6 F62B F62B FDF9 56FD F9F6 ACAC"            /* ÿ.ööö+ö+ýùVýùö¬¬ */
	$"F656 FD56 F6F6 FB88 170F F6F6 F6F6 F7FF"            /* öVýVööûˆ..öööö÷ÿ */
	$"FF00 F6F6 F6F6 F8FB ACF7 F9FD F9F6 ACAC"            /* ÿ.ööööøû¬÷ùýùö¬¬ */
	$"F656 FD56 F6F6 F85F 1708 F6F6 F6F6 F7FF"            /* öVýVööø_..öööö÷ÿ */
	$"FF00 F6F6 F6F6 F9FD ACF7 F9FD F9F6 ACAC"            /* ÿ.ööööùý¬÷ùýùö¬¬ */
	$"F656 FD56 F6F6 2B17 17F6 F6F6 F6F6 F7FF"            /* öVýVöö+..ööööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F7 ACFB F9FD F9F6 ACAC"            /* ÿ.ööööö÷¬ûùýùö¬¬ */
	$"F656 FD56 F6F6 0F17 88F8 F6F6 F6F6 F7FF"            /* öVýVöö..ˆøöööö÷ÿ */
	$"FF00 F6F6 F6FA F72B ACFB 56FD FAF6 ACAC"            /* ÿ.öööú÷+¬ûVýúö¬¬ */
	$"F656 FD56 F607 1716 AC81 F6F6 F6F6 F7FF"            /* öVýVö...¬öööö÷ÿ */
	$"FF00 F6F6 F6FB ACFB FD56 2BAC AC81 FDAC"            /* ÿ.öööû¬ûýV+¬¬ý¬ */
	$"F656 FD56 F610 172B FBFD F7F6 F6F6 F7FF"            /* öVýVö..+ûý÷ööö÷ÿ */
	$"FF00 F6F6 F62B FCAC FAF6 F656 ACF9 8181"            /* ÿ.ööö+ü¬úööV¬ù */
	$"F6F8 FBF8 0810 08F6 56FB 56F6 F6F6 F7FF"            /* öøûø...öVûVööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"            /* ÿ.öööööööööööööö */
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"            /* öööööööööööööö÷ÿ */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"            /* ÿ.öööööööööööööö */
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"            /* öööööööööööööö÷ÿ */
	$"FF00 F7F7 F7F7 F7F7 F7F7 F7F7 F6F6 F6F6"            /* ÿ.÷÷÷÷÷÷÷÷÷÷öööö */
	$"F6F6 F6F6 F7F7 F7F7 F7F7 F7F7 F7F7 F7FF"            /* öööö÷÷÷÷÷÷÷÷÷÷÷ÿ */
	$"00FF FFFF FFFF FFFF FFFF FFFF F6F6 F6F6"            /* .ÿÿÿÿÿÿÿÿÿÿÿöööö */
	$"F6F6 F6F6 FFFF FFFF FFFF FFFF FFFF FF00"            /* ööööÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0000 0000 0000 0000 FF00 F6F6 F6F6"            /* ..........ÿ.öööö */
	$"F6F6 F6F6 F6FF 0000 0000 0000 0000 0000"            /* öööööÿ.......... */
	$"0000 0000 0000 0000 00FF 00F6 F7F7 F7F7"            /* .........ÿ.ö÷÷÷÷ */
	$"F7F7 F7F7 F7F7 FF00 0000 0000 0000 0000"            /* ÷÷÷÷÷÷ÿ......... */
	$"0000 0000 0000 0000 0000 FFFF FFFF FFFF"            /* ..........ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ.......... */
};

data 'ICN#' (-16455) {
	$"7FE0 07FE 8040 0201 8080 0101 807F FE01"            /* .à.þ€@..€€..€.þ. */
	$"8000 0001 8000 0001 8000 0001 8000 7F01"            /* €...€...€...€... */
	$"8003 8181 800C 00C1 8030 0061 8040 0061"            /* €.€..Á€0.a€@.a */
	$"8000 1861 8003 3821 8003 3021 8003 3061"            /* €..a€.8!€.0!€.0a */
	$"838F 7A61 86DF 7B61 80D3 3341 8193 31C1"            /* ƒza†ß{a€Ó3A“1Á */
	$"8193 3181 80D3 3181 84D3 33C1 879F 32C1"            /* “1€Ó1„Ó3Á‡Ÿ2Á */
	$"838F 3241 8000 0001 8000 0001 8000 0001"            /* ƒ2A€...€...€... */
	$"7FF0 0FFE 0020 0400 0040 0200 003F FC00"            /* .ð.þ. ...@...?ü. */
	$"7FE0 07FE FFC0 03FF FF80 01FF FFFF FFFF"            /* .à.þÿÀ.ÿÿ€.ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"7FFF FFFE 003F FC00 007F FE00 003F FC00"            /* .ÿÿþ.?ü...þ..?ü. */
};

data 'ics#' (-16455) {
	$"FC3F 8FF1 8001 8001 8001 8091 80A1 B1BB"            /* ü?ñ€.€.€.€‘€¡±» */
	$"8AAB B2A5 8AAB B1AB 8001 8001 FC3F 0FF0"            /* Š«²¥Š«±«€.€.ü?.ð */
	$"FC3F FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ü?ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0FF0"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.ð */
};

data 'ics4' (-16455) {
	$"FFFF FF00 00FF FFFF FCCC FFFF FFFF CCCF"            /* ÿÿÿ..ÿÿÿüÌÿÿÿÿÌÏ */
	$"FCCC CCCC CCCC CCCF FCCC CCC2 2222 CCCF"            /* üÌÌÌÌÌÌÏüÌÌÂ""ÌÏ */
	$"FCCC C22C CCCC 2CCF FCCC 2CCC ECCE 2CCF"            /* üÌÂ,ÌÌ,ÏüÌ,ÌìÎ,Ï */
	$"FCCC CCCC ECEC 2CCF FCEE CCCE ECEE 2ECF"            /* üÌÌÌìì,ÏüîÌÎìî.Ï */
	$"FCCC ECEC ECEC 2ECF FCEE CCEC ECEC C2CF"            /* üÌìììì.ÏüîÌìììÂÏ */
	$"FCCC ECEC ECEC E2CF FCEE CCCE ECEC E2CF"            /* üÌììììâÏüîÌÎììâÏ */
	$"FCCC CCCC CCCC CCCF FCCC CCCC CCCC CCCF"            /* üÌÌÌÌÌÌÏüÌÌÌÌÌÌÏ */
	$"FFFF FFCC CCFF FFFF 0000 FFFF FFFF 0000"            /* ÿÿÿÌÌÿÿÿ..ÿÿÿÿ.. */
};

data 'ics8' (-16455) {
	$"FFFF FFFF FFFF 0000 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿ....ÿÿÿÿÿÿ */
	$"FFF6 F6F6 FFFF FFFF FFFF FFFF F6F6 F6FF"            /* ÿöööÿÿÿÿÿÿÿÿöööÿ */
	$"FFF6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6FF"            /* ÿööööööööööööööÿ */
	$"FFF6 F6F6 F6F6 F617 1717 1717 F6F6 F6FF"            /* ÿöööööö.....öööÿ */
	$"FFF6 F6F6 F617 17F6 F6F6 F6F6 17F6 F6FF"            /* ÿöööö..ööööö.ööÿ */
	$"FFF6 F6F6 17F6 F6F6 ACF6 F6AC 17F6 F6FF"            /* ÿööö.ööö¬öö¬.ööÿ */
	$"FFF6 F6F6 F6F6 F6F6 ACF6 ACF6 17F6 F6FF"            /* ÿööööööö¬ö¬ö.ööÿ */
	$"FFF6 ACAC F6F6 F6AC ACF6 ACAC 17AC F6FF"            /* ÿö¬¬ööö¬¬ö¬¬.¬öÿ */
	$"FFF6 F6F6 ACF6 ACF6 ACF6 ACF6 17AC F6FF"            /* ÿööö¬ö¬ö¬ö¬ö.¬öÿ */
	$"FFF6 ACAC F6F6 ACF6 ACF6 ACF6 F617 F6FF"            /* ÿö¬¬öö¬ö¬ö¬öö.öÿ */
	$"FFF6 F6F6 ACF6 ACF6 ACF6 ACF6 AC17 F6FF"            /* ÿööö¬ö¬ö¬ö¬ö¬.öÿ */
	$"FFF6 ACAC F6F6 F6AC ACF6 ACF6 AC17 F6FF"            /* ÿö¬¬ööö¬¬ö¬ö¬.öÿ */
	$"FFF6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6FF"            /* ÿööööööööööööööÿ */
	$"FFF7 F7F7 F7F7 F6F6 F6F6 F7F7 F7F7 F7FF"            /* ÿ÷÷÷÷÷öööö÷÷÷÷÷ÿ */
	$"FFFF FFFF FFFF F6F6 F6F6 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿööööÿÿÿÿÿÿ */
	$"0000 0000 FFFF FFFF FFFF FFFF 0000 0000"            /* ....ÿÿÿÿÿÿÿÿ.... */
};
