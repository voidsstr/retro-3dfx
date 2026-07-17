/*
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
**
*/

#include "Types.r"
#include "MacTypes.r"

#include <DrvVersion.h>

data '3DFx' (0, "Owner resource") {
	kGLOBAL_OWNER
};

resource 'BNDL' (-4064) {
	'3DFx',
	0,
	{
		'FREF', {	0,	128 },
		'ICN#', {	0,	128 }
	}
};

resource 'FREF' (128) {
	'shlb',
	0,
	""
};

resource 'icl8' (128) {
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"
	$"FF00 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 00FF"
	$"FF00 F6F6 F57A 4FF6 F6F6 4F7A F57A 4FF6"
	$"2B4F 4FF6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"
	$"FF00 F555 C9C9 C9C9 4FF5 7AC9 F6C9 7AF6"
	$"A5C9 C9C9 A4F5 F6F6 F57A C99E F6F5 F7FF"
	$"FF00 4FC9 7AF6 F69E 2BF6 7AC9 F64F F6F6"
	$"A57A F64F C9A4 F6F5 A5C9 7AC9 C9F6 F7FF"
	$"FF00 9EA4 F6F6 F6F6 F6F6 7AC9 F6A4 79F6"
	$"C979 F6F5 55C9 F67A CF4F F6F6 C9A4 F7FF"
	$"FF00 C979 F679 A49E 9EF6 7AC9 F6A5 79F6"
	$"0710 0F0F 1016 100F C9C9 C9C9 C9C9 F7FF"
	$"FF00 A4A4 F54F 7AA4 CFF5 7AC9 F607 1010"
	$"1010 F6F6 2B07 0E17 164F 4F4F 2B4F F7FF"
	$"FF00 55CF 55F6 F69E C9F6 7AF6 1010 79F6"
	$"A57A F6F6 9EC9 F607 1716 F62B 4FF6 F7FF"
	$"FF00 F69E C99E A5C9 4FF6 1010 F6C9 7AF6"
	$"C9C9 A4C9 C94F F6F6 0F17 0FCF C9F5 F7FF"
	$"FF00 F6F6 4FA4 7AF6 F610 F655 F64F F6F6"
	$"4F7A 7955 F6F5 F6F6 0717 1655 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F5 F607 F6F6 F6F5 F6F6"
	$"F6F6 F6F6 F6F6 F6F6 0716 17F6 F6F6 F7FF"
	$"FF00 00F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBFB"
	$"F6F6 81FD FBF6 F6F6 F616 17F6 F6F6 F7FF"
	$"FF00 FF00 F6F6 F6F6 F6F6 F6F6 F6F5 FDAC"
	$"F6F8 FDAC 56F6 F6F6 F616 17F6 F6F6 F7FF"
	$"FFFF 00FF F6F6 F6F6 F5F6 F6F6 F6F6 ACAC"
	$"F656 FDF8 F6F6 F6F5 F616 17F6 F5F6 F7FF"
	$"FF00 00FF F6F5 F756 2BF6 F6F6 56F7 FCAC"
	$"F6FA FEFA 2BF7 F7F6 0717 16F6 F6F6 F7FF"
	$"0000 00FF F6F7 FDFD ACF6 F681 FEFC ACFC"
	$"81AC ACFD 8156 FDF8 0817 0FF6 F6F6 F7FF"
	$"0000 00FF F5FC FAF8 FDF9 2BFE 81F6 ACAC"
	$"2BFA FD81 F7F6 FDFC 1017 07F6 F6F5 F7FF"
	$"0000 00FF F6F6 F62B FDF9 56FD F9F6 ACAC"
	$"F656 FD56 F6F6 FB88 170F F6F6 F6F6 F7FF"
	$"0000 00FF F6F6 F8FB FD2B F9FD F9F6 ACAC"
	$"F656 FD56 F6F6 F85F 1708 F6F6 F6F6 F7FF"
	$"0000 00FF F6F6 F9FD ACF7 F9FD 56F6 FDAC"
	$"F556 FD56 F5F6 2B17 17F6 F6F6 F6F6 F7FF"
	$"FF00 00FF F6F6 F62B ACFC 56FD F9F6 ACAC"
	$"F656 FD56 F6F6 0F17 88F8 F6F6 F6F5 F7FF"
	$"FFFF 00FF F6FA F72B ACFB 56FD FAF6 ACFD"
	$"F556 FD56 F607 1716 AC81 F6F6 F6F6 F7FF"
	$"FF00 FF00 F6FB FDFB FD56 2BAC FD81 FDAC"
	$"F656 FD56 F610 172B FBFD F7F6 F6F6 F7FF"
	$"FF00 F6F6 F6F5 FCFD FAF5 F656 ACF9 8181"
	$"F6F8 FBF8 0810 08F6 56FB 56F5 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F5 F6F6 F6F6 F6F5 F6F6"
	$"F6F6 F6F6 F5F6 F6F5 F6F6 F6F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"
	$"F6F5 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 2BFF"
	$"FF00 2BF7 F7F7 F7F7 F7F7 F7F7 F6F6 F6F6"
	$"F6F6 F6F6 F7F7 F7F7 F7F7 F7F7 F7F7 F7FF"
	$"00FF FFFF FFFF FFFF FFFF FFFF F6F6 F6F6"
	$"F6F6 F6F6 FFFF FFFF FFFF FFFF FFFF FF00"
	$"0000 0000 0000 0000 0000 FF00 F6F6 F5F6"
	$"F6F6 F6F6 F6FF 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 00FF 00F7 F7F7 F7F7"
	$"F7F7 F7F7 F7F7 FF00 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 FFFF FFFF FFFF"
	$"FFFF FFFF FFFF"
};

resource 'icl4' (128) {
	$"0FFF FFFF FFFF FFFF FFFF FFFF FFFF FFF0"
	$"F000 0000 0000 0000 0000 0000 0000 000F"
	$"F0CC 0DCC CCCD CDCC CCCC CCCC CCCC CCCF"
	$"F0CC 9999 CCD9 C9DC E999 50CC CD9D CCCF"
	$"F0C9 DCCD CCD9 CCCC EDCC 95C0 E9D9 9CCF"
	$"F0D5 CCCC CCD9 C5DC 9DCC C9CD ECCC 95CF"
	$"F09D CD5D DCD9 CEDC 2222 2222 9999 99CF"
	$"F055 CCD5 ECD9 C222 CCCC CCC2 2CCC CCCF"
	$"F0CE CCCD 9CD2 2CDC EDCC D9CC 22CC CCCF"
	$"F0CD 9DE9 CC2C C9DC 9959 9CCC 222E 9CCF"
	$"F0CC C5DC CCCC CCCC CDDC CCCC C22C CCCF"
	$"F0CC CCCC CCCC CCCC CCCC CCCC C22C CCCF"
	$"F00C CCCC CCCC CCEE CCDE ECCC C22C CCCF"
	$"F0F0 CCCC CCCC CCEE CCAE DCCC C22C CCCF"
	$"FF0F CCCC CCCC CCEE CDAD CCCC C22C CCCF"
	$"F00F CCCD CCCC DCEE CDAD CCCC C22C CCCF"
	$"000F CCAA ECCD AEEE DEEA DDAC C22C CCCF"
	$"000F CEDC ADCA DCEE CDAD CCEE 22CC CCCF"
	$"000F CCCC ADDA DCEE CDAD CCEE 22CC CCCF"
	$"000F CCCE ECDA DCEE CDAD CCCB 2CCC CCCF"
	$"000F CCDA ECDA DCEE CDAD CCC2 2CCC CCCF"
	$"F00F CCCC EEDA DCEE CDAD CC22 ECCC CCCF"
	$"FF0F CDCC EEDA DCEE CDAD CC22 EDCC CCCF"
	$"F0F0 CEEE ADCE EDAE CDAD C22C EACC CCCF"
	$"F0CC CCEE DCCD EDDD CCEC C2CC DEDC CCCF"
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"
	$"0FFF FFFF FFFF CCCC CCCC FFFF FFFF FFF0"
	$"0000 0000 00F0 CCCC CCCC CF00 0000 0000"
	$"0000 0000 0F0C CCCC CCCC CCF0 0000 0000"
	$"0000 0000 00FF FFFF FFFF FF"
};

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"",
		/* [2] */
		$"7FFF FFFE FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF DFFF FFFF 9FFF FFFF"
		$"1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF"
		$"1FFF FFFF 9FFF FFFF DFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"7FFF FFFE 003F FC00 007F FE00 003F FC"
	}
};

resource 'ics8' (128) {
	$"FFFF FFFF FFFF 0000 0000 FFFF FFFF FFFF"
	$"FFF6 F6F6 FFFF FFFF FFFF FFFF F6F6 F6FF"
	$"FFF6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6FF"
	$"FFF6 F6F6 F6F6 F617 1717 1717 F6F6 F6FF"
	$"FFF6 F6F6 F617 17F6 F6F6 F6F6 17F6 F6FF"
	$"FFF6 F6F6 17F6 F6F6 ACF6 F6AC 17F6 F6FF"
	$"FFF6 F6F6 F6F6 F6F6 ACF6 ACF6 17F6 F6FF"
	$"FFF6 ACAC F6F6 F6AC ACF6 ACAC 17AC F6FF"
	$"FFF6 F6F6 ACF6 ACF6 ACF6 ACF6 17AC F6FF"
	$"FFF6 ACAC F6F6 ACF6 ACF6 ACF6 F617 F6FF"
	$"FFF6 F6F6 ACF6 ACF6 ACF6 ACF6 AC17 F6FF"
	$"FFF6 ACAC F6F6 F6AC ACF6 ACF6 AC17 F6FF"
	$"FFF6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6FF"
	$"FFF7 F7F7 F7F7 F6F6 F6F6 F7F7 F7F7 F7FF"
	$"FFFF FFFF FFFF F6F6 F6F6 FFFF FFFF FFFF"
	$"0000 0000 FFFF FFFF FFFF FFFF"
};

resource 'ics4' (128) {
	$"FFFF FF00 00FF FFFF FCCC FFFF FFFF CCCF"
	$"FCCC CCCC CCCC CCCF FCCC CCC2 2222 CCCF"
	$"FCCC C22C CCCC 2CCF FCCC 2CCC ECCE 2CCF"
	$"FCCC CCCC ECEC 2CCF FCEE CCCE ECEE 2ECF"
	$"FCCC ECEC ECEC 2ECF FCEE CCEC ECEC C2CF"
	$"FCCC ECEC ECEC E2CF FCEE CCCE ECEC E2CF"
	$"FCCC CCCC CCCC CCCF FCCC CCCC CCCC CCCF"
	$"FFFF FFCC CCFF FFFF 0000 FFFF FFFF"
};

resource 'ics#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"",
		/* [2] */
		$"FC3F FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0FF0"
	}
};

resource 'icl8' (-16455, "Item Icon ") {
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"
	$"FF00 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 00FF"
	$"FF00 F6F6 F57A 4FF6 F6F6 4F7A F57A 4FF6"
	$"2B4F 4FF6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"
	$"FF00 F555 C9C9 C9C9 4FF5 7AC9 F6C9 7AF6"
	$"A5C9 C9C9 A4F5 F6F6 F57A C99E F6F5 F7FF"
	$"FF00 4FC9 7AF6 F69E 2BF6 7AC9 F64F F6F6"
	$"A57A F64F C9A4 F6F5 A5C9 7AC9 C9F6 F7FF"
	$"FF00 9EA4 F6F6 F6F6 F6F6 7AC9 F6A4 79F6"
	$"C979 F6F5 55C9 F67A CF4F F6F6 C9A4 F7FF"
	$"FF00 C979 F679 A49E 9EF6 7AC9 F6A5 79F6"
	$"0710 0F0F 1016 100F C9C9 C9C9 C9C9 F7FF"
	$"FF00 A4A4 F54F 7AA4 CFF5 7AC9 F607 1010"
	$"1010 F6F6 2B07 0E17 164F 4F4F 2B4F F7FF"
	$"FF00 55CF 55F6 F69E C9F6 7AF6 1010 79F6"
	$"A57A F6F6 9EC9 F607 1716 F62B 4FF6 F7FF"
	$"FF00 F69E C99E A5C9 4FF6 1010 F6C9 7AF6"
	$"C9C9 A4C9 C94F F6F6 0F17 0FCF C9F5 F7FF"
	$"FF00 F6F6 4FA4 7AF6 F610 F655 F64F F6F6"
	$"4F7A 7955 F6F5 F6F6 0717 1655 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F5 F607 F6F6 F6F5 F6F6"
	$"F6F6 F6F6 F6F6 F6F6 0716 17F6 F6F6 F7FF"
	$"FF00 00F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBFB"
	$"F6F6 81FD FBF6 F6F6 F616 17F6 F6F6 F7FF"
	$"FF00 FF00 F6F6 F6F6 F6F6 F6F6 F6F5 FDAC"
	$"F6F8 FDAC 56F6 F6F6 F616 17F6 F6F6 F7FF"
	$"FFFF 00FF F6F6 F6F6 F5F6 F6F6 F6F6 ACAC"
	$"F656 FDF8 F6F6 F6F5 F616 17F6 F5F6 F7FF"
	$"FF00 00FF F6F5 F756 2BF6 F6F6 56F7 FCAC"
	$"F6FA FEFA 2BF7 F7F6 0717 16F6 F6F6 F7FF"
	$"0000 00FF F6F7 FDFD ACF6 F681 FEFC ACFC"
	$"81AC ACFD 8156 FDF8 0817 0FF6 F6F6 F7FF"
	$"0000 00FF F5FC FAF8 FDF9 2BFE 81F6 ACAC"
	$"2BFA FD81 F7F6 FDFC 1017 07F6 F6F5 F7FF"
	$"0000 00FF F6F6 F62B FDF9 56FD F9F6 ACAC"
	$"F656 FD56 F6F6 FB88 170F F6F6 F6F6 F7FF"
	$"0000 00FF F6F6 F8FB FD2B F9FD F9F6 ACAC"
	$"F656 FD56 F6F6 F85F 1708 F6F6 F6F6 F7FF"
	$"0000 00FF F6F6 F9FD ACF7 F9FD 56F6 FDAC"
	$"F556 FD56 F5F6 2B17 17F6 F6F6 F6F6 F7FF"
	$"FF00 00FF F6F6 F62B ACFC 56FD F9F6 ACAC"
	$"F656 FD56 F6F6 0F17 88F8 F6F6 F6F5 F7FF"
	$"FFFF 00FF F6FA F72B ACFB 56FD FAF6 ACFD"
	$"F556 FD56 F607 1716 AC81 F6F6 F6F6 F7FF"
	$"FF00 FF00 F6FB FDFB FD56 2BAC FD81 FDAC"
	$"F656 FD56 F610 172B FBFD F7F6 F6F6 F7FF"
	$"FF00 F6F6 F6F5 FCFD FAF5 F656 ACF9 8181"
	$"F6F8 FBF8 0810 08F6 56FB 56F5 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F5 F6F6 F6F6 F6F5 F6F6"
	$"F6F6 F6F6 F5F6 F6F5 F6F6 F6F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"
	$"F6F5 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 2BFF"
	$"FF00 2BF7 F7F7 F7F7 F7F7 F7F7 F6F6 F6F6"
	$"F6F6 F6F6 F7F7 F7F7 F7F7 F7F7 F7F7 F7FF"
	$"00FF FFFF FFFF FFFF FFFF FFFF F6F6 F6F6"
	$"F6F6 F6F6 FFFF FFFF FFFF FFFF FFFF FF00"
	$"0000 0000 0000 0000 0000 FF00 F6F6 F5F6"
	$"F6F6 F6F6 F6FF 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 00FF 00F7 F7F7 F7F7"
	$"F7F7 F7F7 F7F7 FF00 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 FFFF FFFF FFFF"
	$"FFFF FFFF FFFF"
};

resource 'icl4' (-16455, "Item Icon ") {
	$"0FFF FFFF FFFF FFFF FFFF FFFF FFFF FFF0"
	$"F000 0000 0000 0000 0000 0000 0000 000F"
	$"F0C0 0DCC 0CCD 0DCC CCCC 0C0C 0C0C 0CCF"
	$"F00D E979 C0D7 C970 5997 50C0 CD97 C0CF"
	$"F0C7 DCC7 CCD9 0CC0 870D 860C D97E 90CF"
	$"F07E C00C 0079 C7CC 6CC0 D8CD 6C0C 75CF"
	$"F097 0D6D 7CD7 CE70 02C2 C21C 9999 97DF"
	$"F06D CCD7 90D9 0022 12C0 C0C2 20CC CCCF"
	$"F0C6 C0CD 907C 2270 ECC0 DEC0 2B0C CCCF"
	$"F0C7 9D79 CC12 C7D0 9E79 9C0C D226 90CF"
	$"F00C C7B0 C207 0D0C C7DC CC0C 022C 0CCF"
	$"F0C0 0CCC 00C0 C00C 0C0C 00C0 C220 C0CF"
	$"F00C 0C0C 0C0C 0CEE 00EE EC0C 022C 0CCF"
	$"F0F0 C0C0 C0CC 0CEE CCFE D0C0 C420 CCCF"
	$"FF0F 0C0C 0C00 C0AE CDED 0C0C 022C 00CF"
	$"F00F CCCC CC0C DCEE 0DFD CCCC 022C 0CCF"
	$"000F 0CFF E0CD AEFE DEEF DDEC C2C0 C0CF"
	$"000F CEDC EDCF DCEE CDFD CCAE 22CC 0CCF"
	$"000F 0CCC ADDE DCEE CDED 00EE 220C 0CCF"
	$"000F 0CCD FCCF D0EF 0CFC C0D2 20C0 C0CF"
	$"000F C0DF ECDE DCEE CDED 0C02 2C0C 0CCF"
	$"F00F 0C0C EEDE DCEE CCFC C0D2 AC0C 0CCF"
	$"FF0F CDCC EECF D0FE 0DED 0C22 EEC0 C0CF"
	$"F0F0 0EEE FCCE EEEE CCFD 022C EECC 0CCF"
	$"F0C0 C0EE DC0D EDDE 0DDC C1C0 DECC 0CCF"
	$"F00C 0CC0 C0C0 C0C0 C0C0 C0C0 C0C0 C0CF"
	$"F0C0 C0CC 0C0C 0C0C 0C0C 0C0C 0C0C 0CCF"
	$"F0CC CCCC CCCC C0C0 C0CC CCCC CCCC CCCF"
	$"0FFF FFFF FFFF 0C0C 0C00 FFFF FFFF FFF0"
	$"0000 0000 00F0 C0C0 CC0C CF00 0000 0000"
	$"0000 0000 0F0C CCCC CCCC CCF0 0000 0000"
	$"0000 0000 00FF FFFF FFFF FF"
};

resource 'ICN#' (-16455, "Item Icon ") {
	{	/* array: 2 elements */
		/* [1] */
		$"7FFF FFFE 8000 0001 8414 0003 8F36 F873"
		$"9930 CCFB B036 C58F B7B6 7FFF B3B3 C183"
		$"91AE CCC3 9F36 F8FB 8640 6063 8000 0063"
		$"8003 3863 A003 7863 D003 7063 930F 7663"
		$"179F FF63 17FB FBC3 10FB 73C3 13FB 7383"
		$"13FB 7183 91FB 73C3 D6FB 73C3 A7DF 76E3"
		$"839F 74E3 8000 0003 8000 0003 BFF0 0FFF"
		$"7FF0 0FFE 0020 0400 005F FE00 003F FC",
		/* [2] */
		$"7FFF FFFE FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF DFFF FFFF 9FFF FFFF"
		$"1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF"
		$"1FFF FFFF 9FFF FFFF DFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"7FFF FFFE 003F FC00 007F FE00 003F FC"
	}
};

resource 'ics8' (-16455, "Item Icon ") {
	$"FFFF FFFF FFFF 0000 0000 FFFF FFFF FFFF"
	$"FFF6 F6F6 FFFF FFFF FFFF FFFF F6F6 F6FF"
	$"FFF6 F5F6 F6F6 F6F6 F6F6 F6F6 F6F5 F6FF"
	$"FFF6 F6F6 F6F6 F617 1717 1717 F6F6 F6FF"
	$"FFF6 F6F6 F617 17F6 F6F6 F6F6 17F6 F6FF"
	$"FFF6 F6F6 17F6 F6F6 ACF6 F6AC 17F6 F6FF"
	$"FFF6 F6F5 F6F6 F6F6 ACF6 ACF6 17F6 F6FF"
	$"FFF5 ACFD F6F6 F6AC ACF6 ACAC 17AC F6FF"
	$"FFF6 F6F5 FDF5 FDF5 FDF5 FDF5 17FD F5FF"
	$"FFF6 ACFD F6F6 ACF6 ACF6 ACF6 F617 F6FF"
	$"FFF6 F5F6 ACF6 ACF6 FDF5 FDF6 AC17 F6FF"
	$"FFF5 FDAC F6F6 F6AC ACF6 ACF6 AC17 F6FF"
	$"FFF6 F6F6 F6F6 F6F6 F5F6 F6F6 F6F6 F6FF"
	$"FFF7 2BF7 F7F7 F6F6 F6F6 F7F7 F7F7 F7FF"
	$"FFFF FFFF FFFF F6F6 F6F6 FFFF FFFF FFFF"
	$"0000 0000 FFFF FFFF FFFF FFFF"
};

resource 'ics4' (-16455, "Item Icon ") {
	$"FFFF FF00 00FF FFFF FC0C FFFF FFFF C0CF"
	$"F0C0 C0C0 C0C0 0C0F FC0C 0C02 2222 C0CF"
	$"F0C0 C22C 0C0C 2C0F FC0C 20C0 F0CE 20CF"
	$"F0C0 CC0C ECEC 20CF FCEF 00CE A0EE 2F0F"
	$"F0C0 F0EC ECEC 2ECF FCEE C0F0 F0F0 020F"
	$"F0C0 ECE0 ECEC E2CF FCEA C0CE F0E0 F20F"
	$"F0C0 C0C0 CC0C 0CCF FCCC CC0C 00CC CCCF"
	$"FFFF FFC0 CCFF FFFF 0000 FFFF FFFF"
};

resource 'ics#' (-16455, "Item Icon ") {
	{	/* array: 2 elements */
		/* [1] */
		$"FC3F 8FF1 8001 81F1 8609 8899 80A9 B1BD"
		$"8AAD B2A5 8AAD B1AD 8001 FC3F FC3F 0FF0",
		/* [2] */
		$"FC3F FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0FF0"
	}
};

data 'icns' (-16455, "Item Icon ") {
	$"6963 6E73 0000 1614 4943 4E23 0000 0108"            /* icns....ICN#.... */
	$"7FFF FFFE 8000 0001 8414 0003 8F36 F873"            /* .ÿÿþ€...„...6øs */
	$"9930 CCFB B036 C58F B7B6 7FFF B3B3 C183"            /* ™0Ìû°6Å·¶.ÿ³³Áƒ */
	$"91AE CCC3 9F36 F8FB 8640 6063 8000 0063"            /* ‘®ÌÃŸ6øû†@`c€..c */
	$"8003 3863 A003 7863 D003 7063 930F 7663"            /* €.8c .xcÐ.pc“.vc */
	$"179F FF63 17FB FBC3 10FB 73C3 13FB 7383"            /* .Ÿÿc.ûûÃ.ûsÃ.ûsƒ */
	$"13FB 7183 91FB 73C3 D6FB 73C3 A7DF 76E3"            /* .ûqƒ‘ûsÃÖûsÃ§ßvã */
	$"839F 74E3 8000 0003 8000 0003 BFF0 0FFF"            /* ƒŸtã€...€...¿ð.ÿ */
	$"7FF0 0FFE 0020 0400 005F FE00 003F FC00"            /* .ð.þ. ..._þ..?ü. */
	$"7FFF FFFE FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿþÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF DFFF FFFF 9FFF FFFF"            /* ÿÿÿÿÿÿÿÿßÿÿÿŸÿÿÿ */
	$"1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"1FFF FFFF 9FFF FFFF DFFF FFFF FFFF FFFF"            /* .ÿÿÿŸÿÿÿßÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"7FFF FFFE 003F FC00 007F FE00 003F FC00"            /* .ÿÿþ.?ü...þ..?ü. */
	$"6963 6C38 0000 0408 00FF FFFF FFFF FFFF"            /* icl8.....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿ.ÿ....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 00FF FF00 F6F6 F57A 4FF6"            /* .......ÿÿ.ööõzOö */
	$"F6F6 4F7A F57A 4FF6 2B4F 4FF6 F6F6 F6F6"            /* ööOzõzOö+OOööööö */
	$"F6F6 F6F6 F6F6 F7FF FF00 F555 C9C9 C9C9"            /* öööööö÷ÿÿ.õUÉÉÉÉ */
	$"4FF5 7AC9 F6C9 7AF6 A5C9 C9C9 A4F5 F6F6"            /* OõzÉöÉzö¥ÉÉÉ¤õöö */
	$"F57A C99E F6F5 F7FF FF00 4FC9 7AF6 F69E"            /* õzÉžöõ÷ÿÿ.OÉzööž */
	$"2BF6 7AC9 F64F F6F6 A57A F64F C9A4 F6F5"            /* +özÉöOöö¥zöOÉ¤öõ */
	$"A5C9 7AC9 C9F6 F7FF FF00 9EA4 F6F6 F6F6"            /* ¥ÉzÉÉö÷ÿÿ.ž¤öööö */
	$"F6F6 7AC9 F6A4 79F6 C979 F6F5 55C9 F67A"            /* öözÉö¤yöÉyöõUÉöz */
	$"CF4F F6F6 C9A4 F7FF FF00 C979 F679 A49E"            /* ÏOööÉ¤÷ÿÿ.Éyöy¤ž */
	$"9EF6 7AC9 F6A5 79F6 0710 0F0F 1016 100F"            /* žözÉö¥yö........ */
	$"C9C9 C9C9 C9C9 F7FF FF00 A4A4 F54F 7AA4"            /* ÉÉÉÉÉÉ÷ÿÿ.¤¤õOz¤ */
	$"CFF5 7AC9 F607 1010 1010 F6F6 2B07 0E17"            /* ÏõzÉö.....öö+... */
	$"164F 4F4F 2B4F F7FF FF00 55CF 55F6 F69E"            /* .OOO+O÷ÿÿ.UÏUööž */
	$"C9F6 7AF6 1010 79F6 A57A F6F6 9EC9 F607"            /* Éözö..yö¥zööžÉö. */
	$"1716 F62B 4FF6 F7FF FF00 F69E C99E A5C9"            /* ..ö+Oö÷ÿÿ.öžÉž¥É */
	$"4FF6 1010 F6C9 7AF6 C9C9 A4C9 C94F F6F6"            /* Oö..öÉzöÉÉ¤ÉÉOöö */
	$"0F17 0FCF C9F5 F7FF FF00 F6F6 4FA4 7AF6"            /* ...ÏÉõ÷ÿÿ.ööO¤zö */
	$"F610 F655 F64F F6F6 4F7A 7955 F6F5 F6F6"            /* ö.öUöOööOzyUöõöö */
	$"0717 1655 F6F6 F7FF FF00 F6F6 F6F6 F6F5"            /* ...Uöö÷ÿÿ.öööööõ */
	$"F607 F6F6 F6F5 F6F6 F6F6 F6F6 F6F6 F6F6"            /* ö.öööõöööööööööö */
	$"0716 17F6 F6F6 F7FF FF00 00F6 F6F6 F6F6"            /* ...ööö÷ÿÿ..ööööö */
	$"F6F6 F6F6 F6F6 FBFB F6F6 81FD FBF6 F6F6"            /* ööööööûûööýûööö */
	$"F616 17F6 F6F6 F7FF FF00 FF00 F6F6 F6F6"            /* ö..ööö÷ÿÿ.ÿ.öööö */
	$"F6F6 F6F6 F6F5 FDAC F6F8 FDAC 56F6 F6F6"            /* öööööõý¬öøý¬Vööö */
	$"F616 17F6 F6F6 F7FF FFFF 00FF F6F6 F6F6"            /* ö..ööö÷ÿÿÿ.ÿöööö */
	$"F5F6 F6F6 F6F6 ACAC F656 FDF8 F6F6 F6F5"            /* õööööö¬¬öVýøöööõ */
	$"F616 17F6 F5F6 F7FF FF00 00FF F6F5 F756"            /* ö..öõö÷ÿÿ..ÿöõ÷V */
	$"2BF6 F6F6 56F7 FCAC F6FA FEFA 2BF7 F7F6"            /* +öööV÷ü¬öúþú+÷÷ö */
	$"0717 16F6 F6F6 F7FF 0000 00FF F6F7 FDFD"            /* ...ööö÷ÿ...ÿö÷ýý */
	$"ACF6 F681 FEFC ACFC 81AC ACFD 8156 FDF8"            /* ¬ööþü¬ü¬¬ýVýø */
	$"0817 0FF6 F6F6 F7FF 0000 00FF F5FC FAF8"            /* ...ööö÷ÿ...ÿõüúø */
	$"FDF9 2BFE 81F6 ACAC 2BFA FD81 F7F6 FDFC"            /* ýù+þö¬¬+úý÷öýü */
	$"1017 07F6 F6F5 F7FF 0000 00FF F6F6 F62B"            /* ...ööõ÷ÿ...ÿööö+ */
	$"FDF9 56FD F9F6 ACAC F656 FD56 F6F6 FB88"            /* ýùVýùö¬¬öVýVööûˆ */
	$"170F F6F6 F6F6 F7FF 0000 00FF F6F6 F8FB"            /* ..öööö÷ÿ...ÿööøû */
	$"FD2B F9FD F9F6 ACAC F656 FD56 F6F6 F85F"            /* ý+ùýùö¬¬öVýVööø_ */
	$"1708 F6F6 F6F6 F7FF 0000 00FF F6F6 F9FD"            /* ..öööö÷ÿ...ÿööùý */
	$"ACF7 F9FD 56F6 FDAC F556 FD56 F5F6 2B17"            /* ¬÷ùýVöý¬õVýVõö+. */
	$"17F6 F6F6 F6F6 F7FF FF00 00FF F6F6 F62B"            /* .ööööö÷ÿÿ..ÿööö+ */
	$"ACFC 56FD F9F6 ACAC F656 FD56 F6F6 0F17"            /* ¬üVýùö¬¬öVýVöö.. */
	$"88F8 F6F6 F6F5 F7FF FFFF 00FF F6FA F72B"            /* ˆøöööõ÷ÿÿÿ.ÿöú÷+ */
	$"ACFB 56FD FAF6 ACFD F556 FD56 F607 1716"            /* ¬ûVýúö¬ýõVýVö... */
	$"AC81 F6F6 F6F6 F7FF FF00 FF00 F6FB FDFB"            /* ¬öööö÷ÿÿ.ÿ.öûýû */
	$"FD56 2BAC FD81 FDAC F656 FD56 F610 172B"            /* ýV+¬ýý¬öVýVö..+ */
	$"FBFD F7F6 F6F6 F7FF FF00 F6F6 F6F5 FCFD"            /* ûý÷ööö÷ÿÿ.öööõüý */
	$"FAF5 F656 ACF9 8181 F6F8 FBF8 0810 08F6"            /* úõöV¬ùöøûø...ö */
	$"56FB 56F5 F6F6 F7FF FF00 F6F6 F6F6 F6F5"            /* VûVõöö÷ÿÿ.öööööõ */
	$"F6F6 F6F6 F6F5 F6F6 F6F6 F6F6 F5F6 F6F5"            /* öööööõööööööõööõ */
	$"F6F6 F6F6 F6F6 F7FF FF00 F6F6 F6F6 F6F6"            /* öööööö÷ÿÿ.öööööö */
	$"F6F6 F6F6 F6F6 F6F6 F6F5 F6F6 F6F6 F6F6"            /* öööööööööõöööööö */
	$"F6F6 F6F6 F6F6 2BFF FF00 2BF7 F7F7 F7F7"            /* öööööö+ÿÿ.+÷÷÷÷÷ */
	$"F7F7 F7F7 F6F6 F6F6 F6F6 F6F6 F7F7 F7F7"            /* ÷÷÷÷öööööööö÷÷÷÷ */
	$"F7F7 F7F7 F7F7 F7FF 00FF FFFF FFFF FFFF"            /* ÷÷÷÷÷÷÷ÿ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF F6F6 F6F6 F6F6 F6F6 FFFF FFFF"            /* ÿÿÿÿööööööööÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ......... */
	$"0000 FF00 F6F6 F5F6 F6F6 F6F6 F6FF 0000"            /* ..ÿ.ööõööööööÿ.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"00FF 00F7 F7F7 F7F7 F7F7 F7F7 F7F7 FF00"            /* .ÿ.÷÷÷÷÷÷÷÷÷÷÷ÿ. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 0000 696C 3332 0000 088D"            /* ........il32... */
	$"00FF 9B00 01FF 009B FF07 0000 FFDE DEEF"            /* .ÿ›..ÿ.›ÿ...ÿÞÞï */
	$"639C 80DE 089C 63DE 639C DECE 9C9C 88DE"            /* cœ€Þ.œcÞcœÞÎœœˆÞ */
	$"05BD 0000 FFDE 9C81 0008 9CDE 6300 DE00"            /* .½..ÿÞœ..œÞc.Þ. */
	$"63DE 3180 0001 31EF 80DE 2A63 0031 DEDE"            /* cÞ1€..1ï€Þ*c.1ÞÞ */
	$"BD00 00FF 9C00 63DE DE31 CEDE 6300 DE9C"            /* ½..ÿœ.cÞÞ1ÎÞc.Þœ */
	$"DEDE 3163 DE9C 0031 DEEF 3100 6300 00DE"            /* ÞÞ1cÞœ.1Þï1.c..Þ */
	$"BD00 00FF 3131 83DE 1B63 00DE 3163 DE00"            /* ½..ÿ11ƒÞ.c.Þ1cÞ. */
	$"63DE DE9C 00DE 6300 9CDE DE00 31BD 0000"            /* cÞÞœ.Þc.œÞÞ.1½.. */
	$"FF00 63DE 6380 3106 DE63 00DE 3163 DE85"            /* ÿ.cÞc€1.Þc.Þ1cÞ… */
	$"FF83 000E BD00 00FF 3131 DE9C 6331 00DE"            /* ÿƒ..½..ÿ11Þœc1.Þ */
	$"6300 DE82 FF02 DEDE CE81 FF80 9C1A CE9C"            /* c.Þ‚ÿ.ÞÞÎÿ€œ.Îœ */
	$"BD00 00FF 9C00 9CDE DE31 00DE 63DE FFFF"            /* ½..ÿœ.œÞÞ1.ÞcÞÿÿ */
	$"63DE 3163 DEDE 3100 DE80 FF1D DECE 9CDE"            /* cÞ1cÞÞ1.Þ€ÿ.ÞÎœÞ */
	$"BD00 00FF DE31 0031 3100 9CDE FFFF DE00"            /* ½..ÿÞ1.11.œÞÿÿÞ. */
	$"63DE 0000 3100 009C DEDE 80FF 1800 00DE"            /* cÞ..1..œÞÞ€ÿ...Þ */
	$"BD00 00FF DEDE 9C31 63DE DEFF DE9C DE9C"            /* ½..ÿÞÞœ1cÞÞÿÞœÞœ */
	$"DEDE 9C63 639C 81DE 80FF 069C DEDE BD00"            /* ÞÞœccœÞ€ÿ.œÞÞ½. */
	$"00FF 84DE 00FF 8BDE 80FF 80DE 04BD 0000"            /* .ÿ„Þ.ÿ‹Þ€ÿ€Þ.½.. */
	$"FFFF 88DE 0652 52DE DE63 3152 81DE 01FF"            /* ÿÿˆÞ.RRÞÞc1RÞ.ÿ */
	$"FF80 DE05 BD00 00FF 00FF 87DE 0631 31DE"            /* ÿ€Þ.½..ÿ.ÿ‡Þ.11Þ */
	$"AD21 319C 81DE 01FF FF80 DE00 BD80 0001"            /* ­!1œÞ.ÿÿ€Þ.½€.. */
	$"FF00 87DE 0531 31DE 9C21 9C82 DE01 FFFF"            /* ÿ.‡Þ.11Þœ!œ‚Þ.ÿÿ */
	$"80DE 0ABD 0000 FFFF 00DE DEBD 9CCE 80DE"            /* €Þ.½..ÿÿ.ÞÞ½œÎ€Þ */
	$"0B9C BD42 42CE 7321 73CE BDBD DE80 FF80"            /* .œ½BBÎs!sÎ½½Þ€ÿ€ */
	$"DE01 BD00 80FF 1400 DEBD 2121 42CE DE63"            /* Þ.½.€ÿ..Þ½!!BÎÞc */
	$"2142 3142 7331 3121 639C 21AD 80FF 80DE"            /* !B1Bs11!cœ!­€ÿ€Þ */
	$"01BD 0080 FF14 00DE 5273 AD21 8CBD 2163"            /* .½.€ÿ..ÞRs­!Œ½!c */
	$"CE31 42BD 7321 63BD CE31 4280 FF80 DE01"            /* Î1B½s!c½Î1B€ÿ€Þ. */
	$"BD00 80FF 1600 DECE DECE 218C 9C21 8CDE"            /* ½.€ÿ..ÞÎÞÎ!Œœ!ŒÞ */
	$"3131 DE9C 219C DEDE 5263 FFFF 81DE 01BD"            /* 11Þœ!œÞÞRcÿÿÞ.½ */
	$"0080 FF16 00DE DEAD 5231 BD8C 218C DE31"            /* .€ÿ..ÞÞ­R1½Œ!ŒÞ1 */
	$"31DE 9C21 9CDE DEAD 9CFF FF81 DE01 BD00"            /* 1Þœ!œÞÞ­œÿÿÞ.½. */
	$"80FF 1500 DEDE 8C21 31BD 8C21 8CDE 3131"            /* €ÿ..ÞÞŒ!1½Œ!ŒÞ11 */
	$"DE9C 219C DEDE CEFF FF82 DE05 BD00 00FF"            /* Þœ!œÞÞÎÿÿ‚Þ.½..ÿ */
	$"FF00 80DE 12BD 3152 8C21 8CDE 3131 DE9C"            /* ÿ.€Þ.½1RŒ!ŒÞ11Þœ */
	$"219C DEDE FFFF 63AD 81DE 00BD 8000 12FF"            /* !œÞÞÿÿc­Þ.½€..ÿ */
	$"00DE 73BD CE31 529C 2173 DE31 31DE 9C21"            /* .Þs½Î1Rœ!sÞ11Þœ! */
	$"9CDE 80FF 0131 6381 DE1C BD00 00FF 00FF"            /* œÞ€ÿ.1cÞ.½..ÿ.ÿ */
	$"DE52 3152 219C CE31 3163 2131 DE9C 219C"            /* ÞR1R!œÎ11c!1Þœ!œ */
	$"DEFF FFCE 5221 BD80 DE03 BD00 00FF 81DE"            /* ÞÿÿÎR!½€Þ.½..ÿÞ */
	$"0D42 3173 DEDE 9C31 8C63 63DE AD52 AD80"            /* ÂB1sÞÞœ1ŒccÞ­R­€ */
	$"FF03 DE9C 529C 80DE 03BD 0000 FF99 DE03"            /* ÿ.ÞœRœ€Þ.½..ÿ™Þ. */
	$"BD00 00FF 99DE 03BD 0000 FF87 BD85 DE88"            /* ½..ÿ™Þ.½..ÿ‡½…Þˆ */
	$"BD01 00FF 8800 85DE 8800 88FF 0100 FF86"            /* ½..ÿˆ.…Þˆ.ˆÿ..ÿ† */
	$"DE00 0090 FF01 00FF 88BD 0000 90FF 8900"            /* Þ..ÿ..ÿˆ½..ÿ‰. */
	$"87FF 00FF 9B00 01FF 009B FF07 0000 FFDE"            /* ‡ÿ.ÿ›..ÿ.›ÿ...ÿÞ */
	$"DEEF 9CCE 80DE 05CE 9CDE 9CCE DE80 CE88"            /* ÞïœÎ€Þ.ÎœÞœÎÞ€Îˆ */
	$"DE05 BD00 00FF DE9C 8163 07CE DE9C 63DE"            /* Þ.½..ÿÞœc.ÎÞœcÞ */
	$"639C DE82 6300 EF80 DE2A 9C63 9CDE DEBD"            /* cœÞ‚c.ï€Þ*œcœÞÞ½ */
	$"0000 FFCE 639C DEDE 9CCE DE9C 63DE CEDE"            /* ..ÿÎcœÞÞœÎÞœcÞÎÞ */
	$"DE63 9CDE CE63 63DE EF63 639C 6363 DEBD"            /* ÞcœÞÎccÞïccœccÞ½ */
	$"0000 FF9C 6383 DE26 9C63 DE63 9CDE 639C"            /* ..ÿœcƒÞ&œcÞcœÞcœ */
	$"DEDE 9C63 DE9C 31CE DEDE 6363 BD00 00FF"            /* ÞÞœcÞœ1ÎÞÞcc½..ÿ */
	$"639C DE9C 639C 9CDE 9C63 DE63 9CDE CE81"            /* cœÞœcœœÞœcÞcœÞÎ */
	$"9C02 639C 9C83 630F BD00 00FF 6363 DECE"            /* œ.cœœƒc.½..ÿccÞÎ */
	$"9C63 31DE 9C63 DECE 819C 06DE DECE CE9C"            /* œc1ÞœcÞÎœ.ÞÞÎÎœ */
	$"6363 82CE 0DBD 0000 FF9C 319C DEDE 9C63"            /* cc‚ÎÂ½..ÿœ1œÞÞœc */
	$"DE9C DE80 9C20 DE63 9CDE DE9C 63DE CE63"            /* ÞœÞ€œ ÞcœÞÞœcÞÎc */
	$"63DE CECE DEBD 0000 FFDE 9C63 9C63 63CE"            /* cÞÎÎÞ½..ÿÞœcœccÎ */
	$"DE9C 9CDE 639C DE82 631B CEDE DE9C 639C"            /* ÞœœÞcœÞ‚c.ÎÞÞœcœ */
	$"3163 DEBD 0000 FFDE DECE 639C DEDE 9CDE"            /* 1cÞ½..ÿÞÞÎcœÞÞœÞ */
	$"9CDE CEDE DECE 809C 81DE 09CE 6363 9CDE"            /* œÞÎÞÞÎ€œÞÆÎccœÞ */
	$"DEBD 0000 FF84 DE00 CE8B DE02 CE63 6380"            /* Þ½..ÿ„Þ.Î‹Þ.Îcc€ */
	$"DE04 BD00 00FF FF88 DE06 5252 DEDE 6331"            /* Þ.½..ÿÿˆÞ.RRÞÞc1 */
	$"5281 DE01 6363 80DE 05BD 0000 FF00 FF87"            /* RÞ.cc€Þ.½..ÿ.ÿ‡ */
	$"DE06 3131 DEAD 2131 9C81 DE01 6363 80DE"            /* Þ.11Þ­!1œÞ.cc€Þ */
	$"00BD 8000 01FF 0087 DE05 3131 DE9C 219C"            /* .½€..ÿ.‡Þ.11Þœ!œ */
	$"82DE 0163 6380 DE0A BD00 00FF FF00 DEDE"            /* ‚Þ.cc€Þ.½..ÿÿ.ÞÞ */
	$"BD9C CE80 DE0E 9CBD 4242 CE73 2173 CEBD"            /* ½œÎ€Þ.œ½BBÎs!sÎ½ */
	$"BDDE CE63 6380 DE01 BD00 80FF 1700 DEBD"            /* ½ÞÎcc€Þ.½.€ÿ..Þ½ */
	$"2121 42CE DE63 2142 3142 7331 3121 639C"            /* !!BÎÞc!B1Bs11!cœ */
	$"21AD CE63 9C80 DE01 BD00 80FF 1700 DE52"            /* !­Îcœ€Þ.½.€ÿ..ÞR */
	$"73AD 218C BD21 63CE 3142 BD73 2163 BDCE"            /* s­!Œ½!cÎ1B½s!c½Î */
	$"3142 9C63 CE80 DE01 BD00 80FF 1600 DECE"            /* 1BœcÎ€Þ.½.€ÿ..ÞÎ */
	$"DECE 218C 9C21 8CDE 3131 DE9C 219C DEDE"            /* ÞÎ!Œœ!ŒÞ11Þœ!œÞÞ */
	$"5231 639C 81DE 01BD 0080 FF16 00DE DEAD"            /* R1cœÞ.½.€ÿ..ÞÞ­ */
	$"5231 BD8C 218C DE31 31DE 9C21 9CDE DEAD"            /* R1½Œ!ŒÞ11Þœ!œÞÞ­ */
	$"6363 CE81 DE01 BD00 80FF 1500 DEDE 8C21"            /* ccÎÞ.½.€ÿ..ÞÞŒ! */
	$"31BD 8C21 8CDE 3131 DE9C 219C DEDE CE63"            /* 1½Œ!ŒÞ11Þœ!œÞÞÎc */
	$"6382 DE05 BD00 00FF FF00 80DE 12BD 3152"            /* c‚Þ.½..ÿÿ.€Þ.½1R */
	$"8C21 8CDE 3131 DE9C 219C DEDE 9C63 31AD"            /* Œ!ŒÞ11Þœ!œÞÞœc1­ */
	$"81DE 00BD 8000 17FF 00DE 73BD CE31 529C"            /* Þ.½€..ÿ.Þs½Î1Rœ */
	$"2173 DE31 31DE 9C21 9CDE CE63 6331 6381"            /* !sÞ11Þœ!œÞÎcc1c */
	$"DE1C BD00 00FF 00FF DE52 3152 219C CE31"            /* Þ.½..ÿ.ÿÞR1R!œÎ1 */
	$"3163 2131 DE9C 219C DE9C 63CE 5221 BD80"            /* 1c!1Þœ!œÞœcÎR!½€ */
	$"DE03 BD00 00FF 81DE 1442 3173 DEDE 9C31"            /* Þ.½..ÿÞ.B1sÞÞœ1 */
	$"8C63 63DE AD52 ADCE 9CCE DE9C 529C 80DE"            /* ŒccÞ­R­ÎœÎÞœRœ€Þ */
	$"03BD 0000 FF99 DE03 BD00 00FF 99DE 03BD"            /* .½..ÿ™Þ.½..ÿ™Þ.½ */
	$"0000 FF87 BD85 DE88 BD01 00FF 8800 85DE"            /* ..ÿ‡½…Þˆ½..ÿˆ.…Þ */
	$"8800 88FF 0100 FF86 DE00 0090 FF01 00FF"            /* ˆ.ˆÿ..ÿ†Þ..ÿ..ÿ */
	$"88BD 0000 90FF 8900 87FF 00FF 9B00 01FF"            /* ˆ½..ÿ‰.‡ÿ.ÿ›..ÿ */
	$"009B FF07 0000 FFDE DEEF 9CCE 80DE 05CE"            /* .›ÿ...ÿÞÞïœÎ€Þ.Î */
	$"9CDE 9CCE DE80 CE88 DE05 BD00 00FF DECE"            /* œÞœÎÞ€ÎˆÞ.½..ÿÞÎ */
	$"8163 07CE DE9C 63DE 639C DE81 6301 9CEF"            /* c.ÎÞœcÞcœÞc.œï */
	$"80DE 2A9C 639C DEDE BD00 00FF CE63 9CDE"            /* €Þ*œcœÞÞ½..ÿÎcœÞ */
	$"DE9C CEDE 9C63 DECE DEDE 639C DECE 639C"            /* ÞœÎÞœcÞÎÞÞcœÞÎcœ */
	$"DEEF 6363 9C63 63DE BD00 00FF 9C9C 83DE"            /* ÞïccœccÞ½..ÿœœƒÞ */
	$"1B9C 63DE 9CCE DE63 CEDE DECE 63DE 9C63"            /* .œcÞœÎÞcÎÞÞÎcÞœc */
	$"CEDE DE63 9CBD 0000 FF63 CEDE CE80 9C0A"            /* ÎÞÞcœ½..ÿcÎÞÎ€œ. */
	$"DE9C 63DE 63CE DECE 3163 6380 3184 630F"            /* ÞœcÞcÎÞÎ1cc€1„c. */
	$"BD00 00FF 9C9C DECE 9C9C 63DE 9C63 DECE"            /* ½..ÿœœÞÎœœcÞœcÞÎ */
	$"8131 06DE DECE CE9C 0031 82CE 3BBD 0000"            /* 1.ÞÞÎÎœ.1‚Î;½.. */
	$"FFCE 63CE DEDE 9C63 DE9C DE31 31CE DE63"            /* ÿÎcÎÞÞœcÞœÞ11ÎÞc */
	$"9CDE DE9C 63DE CE00 31DE CECE DEBD 0000"            /* œÞÞœcÞÎ.1ÞÎÎÞ½.. */
	$"FFDE 9C63 9C63 63CE DE31 31DE 639C DE63"            /* ÿÞœcœccÎÞ11ÞcœÞc */
	$"639C 6363 CEDE DE63 0080 6316 DEBD 0000"            /* cœccÎÞÞc.€c.Þ½.. */
	$"FFDE DECE 9C9C DEDE 31DE CEDE CEDE DECE"            /* ÿÞÞÎœœÞÞ1ÞÎÞÎÞÞÎ */
	$"9CCE CE81 DE09 CE00 31CE DEDE BD00 00FF"            /* œÎÎÞÆÎ.1ÎÞÞ½..ÿ */
	$"84DE 00CE 8BDE 02CE 3100 80DE 04BD 0000"            /* „Þ.Î‹Þ.Î1.€Þ.½.. */
	$"FFFF 88DE 0652 52DE DE63 3152 81DE 0131"            /* ÿÿˆÞ.RRÞÞc1RÞ.1 */
	$"0080 DE05 BD00 00FF 00FF 87DE 0631 31DE"            /* .€Þ.½..ÿ.ÿ‡Þ.11Þ */
	$"AD21 319C 81DE 0131 0080 DE00 BD80 0001"            /* ­!1œÞ.1.€Þ.½€.. */
	$"FF00 87DE 0531 31DE 9C21 9C82 DE01 3100"            /* ÿ.‡Þ.11Þœ!œ‚Þ.1. */
	$"80DE 0ABD 0000 FFFF 00DE DEBD 9CCE 80DE"            /* €Þ.½..ÿÿ.ÞÞ½œÎ€Þ */
	$"0E9C BD42 42CE 7321 73CE BDBD DECE 0031"            /* .œ½BBÎs!sÎ½½ÞÎ.1 */
	$"80DE 01BD 0080 FF17 00DE BD21 2142 CEDE"            /* €Þ.½.€ÿ..Þ½!!BÎÞ */
	$"6321 4231 4273 3131 2163 9C21 AD9C 0063"            /* c!B1Bs11!cœ!­œ.c */
	$"80DE 01BD 0080 FF17 00DE 5273 AD21 8CBD"            /* €Þ.½.€ÿ..ÞRs­!Œ½ */
	$"2163 CE31 42BD 7321 63BD CE31 4231 00CE"            /* !cÎ1B½s!c½Î1B1.Î */
	$"80DE 01BD 0080 FF16 00DE CEDE CE21 8C9C"            /* €Þ.½.€ÿ..ÞÎÞÎ!Œœ */
	$"218C DE31 31DE 9C21 9CDE DE52 3100 6381"            /* !ŒÞ11Þœ!œÞÞR1.c */
	$"DE01 BD00 80FF 1600 DEDE AD52 31BD 8C21"            /* Þ.½.€ÿ..ÞÞ­R1½Œ! */
	$"8CDE 3131 DE9C 219C DEDE AD00 009C 81DE"            /* ŒÞ11Þœ!œÞÞ­..œÞ */
	$"01BD 0080 FF15 00DE DE8C 2131 BD8C 218C"            /* .½.€ÿ..ÞÞŒ!1½Œ!Œ */
	$"DE31 31DE 9C21 9CDE DECE 0000 82DE 05BD"            /* Þ11Þœ!œÞÞÎ..‚Þ.½ */
	$"0000 FFFF 0080 DE12 BD31 528C 218C DE31"            /* ..ÿÿ.€Þ.½1RŒ!ŒÞ1 */
	$"31DE 9C21 9CDE DE63 0031 AD81 DE00 BD80"            /* 1Þœ!œÞÞc.1­Þ.½€ */
	$"0017 FF00 DE73 BDCE 3152 9C21 73DE 3131"            /* ..ÿ.Þs½Î1Rœ!sÞ11 */
	$"DE9C 219C DECE 0031 3163 81DE 1CBD 0000"            /* Þœ!œÞÎ.11cÞ.½.. */
	$"FF00 FFDE 5231 5221 9CCE 3131 6321 31DE"            /* ÿ.ÿÞR1R!œÎ11c!1Þ */
	$"9C21 9CDE 3100 CE52 21BD 80DE 03BD 0000"            /* œ!œÞ1.ÎR!½€Þ.½.. */
	$"FF81 DE14 4231 73DE DE9C 318C 6363 DEAD"            /* ÿÞ.B1sÞÞœ1ŒccÞ­ */
	$"52AD 9C31 9CDE 9C52 9C80 DE03 BD00 00FF"            /* R­œ1œÞœRœ€Þ.½..ÿ */
	$"99DE 03BD 0000 FF99 DE03 BD00 00FF 87BD"            /* ™Þ.½..ÿ™Þ.½..ÿ‡½ */
	$"85DE 88BD 0100 FF88 0085 DE88 0088 FF01"            /* …Þˆ½..ÿˆ.…Þˆ.ˆÿ. */
	$"00FF 86DE 0000 90FF 0100 FF88 BD00 0090"            /* .ÿ†Þ..ÿ..ÿˆ½.. */
	$"FF89 0087 FF6C 386D 6B00 0004 0800 FFFF"            /* ÿ‰.‡ÿl8mk.....ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ.ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 0000 0000 0000 0000"            /* ÿÿÿ............. */
	$"0000 0000 0000 FFFF FFFF FFFF FFFF FFFF"            /* ......ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 0000 0000 0000 0000"            /* ÿÿÿÿ............ */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 0000 0000 0069 6373"            /* ÿÿÿ..........ics */
	$"2300 0000 48FC 3F8F F180 0181 F186 0988"            /* #...Hü?ñ€.ñ†Æˆ */
	$"9980 A9B1 BD8A ADB2 A58A ADB1 AD80 01FC"            /* ™€©±½Š­²¥Š­±­€.ü */
	$"3FFC 3F0F F0FC 3FFF FFFF FFFF FFFF FFFF"            /* ?ü?.ðü?ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF0F F069 6373 3800 0001 08FF FFFF"            /* ÿÿÿ.ðics8....ÿÿÿ */
	$"FFFF FF00 0000 00FF FFFF FFFF FFFF F6F6"            /* ÿÿÿ....ÿÿÿÿÿÿÿöö */
	$"F6FF FFFF FFFF FFFF FFF6 F6F6 FFFF F6F5"            /* öÿÿÿÿÿÿÿÿöööÿÿöõ */
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F5F6 FFFF F6F6"            /* ööööööööööõöÿÿöö */
	$"F6F6 F6F6 1717 1717 17F6 F6F6 FFFF F6F6"            /* öööö.....öööÿÿöö */
	$"F6F6 1717 F6F6 F6F6 F617 F6F6 FFFF F6F6"            /* öö..ööööö.ööÿÿöö */
	$"F617 F6F6 F6AC F6F6 AC17 F6F6 FFFF F6F6"            /* ö.ööö¬öö¬.ööÿÿöö */
	$"F5F6 F6F6 F6AC F6AC F617 F6F6 FFFF F5AC"            /* õöööö¬ö¬ö.ööÿÿõ¬ */
	$"FDF6 F6F6 ACAC F6AC AC17 ACF6 FFFF F6F6"            /* ýööö¬¬ö¬¬.¬öÿÿöö */
	$"F5FD F5FD F5FD F5FD F517 FDF5 FFFF F6AC"            /* õýõýõýõýõ.ýõÿÿö¬ */
	$"FDF6 F6AC F6AC F6AC F6F6 17F6 FFFF F6F5"            /* ýöö¬ö¬ö¬öö.öÿÿöõ */
	$"F6AC F6AC F6FD F5FD F6AC 17F6 FFFF F5FD"            /* ö¬ö¬öýõýö¬.öÿÿõý */
	$"ACF6 F6F6 ACAC F6AC F6AC 17F6 FFFF F6F6"            /* ¬ööö¬¬ö¬ö¬.öÿÿöö */
	$"F6F6 F6F6 F6F5 F6F6 F6F6 F6F6 FFFF F72B"            /* öööööõööööööÿÿ÷+ */
	$"F7F7 F7F6 F6F6 F6F7 F7F7 F7F7 FFFF FFFF"            /* ÷÷÷öööö÷÷÷÷÷ÿÿÿÿ */
	$"FFFF FFF6 F6F6 F6FF FFFF FFFF FF00 0000"            /* ÿÿÿööööÿÿÿÿÿÿ... */
	$"00FF FFFF FFFF FFFF FF00 0000 0069 7333"            /* .ÿÿÿÿÿÿÿÿ....is3 */
	$"3200 0002 0F83 0081 FF84 0080 DE85 0080"            /* 2....ƒ.ÿ„.€Þ….€ */
	$"DE01 0000 8BDE 0100 0083 DE82 FF80 DE01"            /* Þ...‹Þ...ƒÞ‚ÿ€Þ. */
	$"0000 81DE 01FF FF82 DE04 FFDE DE00 0080"            /* ..Þ.ÿÿ‚Þ.ÿÞÞ..€ */
	$"DE00 FF80 DE08 31DE DE31 FFDE DE00 0084"            /* Þ.ÿ€Þ.1ÞÞ1ÿÞÞ..„ */
	$"DE0B 31DE 31DE FFDE DE00 00DE 3131 80DE"            /* Þ.1Þ1ÞÿÞÞ..Þ11€Þ */
	$"0931 31DE 3131 FF31 DE00 0080 DE1C 31DE"            /* Æ11Þ11ÿ1Þ..€Þ.1Þ */
	$"31DE 31DE 31DE FF31 DE00 00DE 3131 DEDE"            /* 1Þ1Þ1Þÿ1Þ..Þ11ÞÞ */
	$"31DE 31DE 31DE DEFF DE00 0080 DE0F 31DE"            /* 1Þ1Þ1ÞÞÿÞ..€Þ.1Þ */
	$"31DE 31DE 31DE 31FF DE00 00DE 3131 80DE"            /* 1Þ1Þ1Þ1ÿÞ..Þ11€Þ */
	$"0931 31DE 31DE 31FF DE00 008B DE01 0000"            /* Æ11Þ1Þ1ÿÞ..‹Þ... */
	$"82BD 81DE 82BD 8400 81DE 8300 81FF 8500"            /* ‚½Þ‚½„.Þƒ.ÿ…. */
	$"81FF 8300 81FF 8400 80DE 8500 80DE 0100"            /* ÿƒ.ÿ„.€Þ….€Þ.. */
	$"008B DE01 0000 83DE 8263 80DE 0100 0081"            /* .‹Þ...ƒÞ‚c€Þ... */
	$"DE01 6363 82DE 0463 DEDE 0000 80DE 0063"            /* Þ.cc‚Þ.cÞÞ..€Þ.c */
	$"80DE 0831 DEDE 3163 DEDE 0000 84DE 0B31"            /* €Þ.1ÞÞ1cÞÞ..„Þ.1 */
	$"DE31 DE63 DEDE 0000 DE31 3180 DE09 3131"            /* Þ1ÞcÞÞ..Þ11€ÞÆ11 */
	$"DE31 3163 31DE 0000 80DE 1C31 DE31 DE31"            /* Þ11c1Þ..€Þ.1Þ1Þ1 */
	$"DE31 DE63 31DE 0000 DE31 31DE DE31 DE31"            /* Þ1Þc1Þ..Þ11ÞÞ1Þ1 */
	$"DE31 DEDE 63DE 0000 80DE 0F31 DE31 DE31"            /* Þ1ÞÞcÞ..€Þ.1Þ1Þ1 */
	$"DE31 DE31 63DE 0000 DE31 3180 DE09 3131"            /* Þ1Þ1cÞ..Þ11€ÞÆ11 */
	$"DE31 DE31 63DE 0000 8BDE 0100 0082 BD81"            /* Þ1Þ1cÞ..‹Þ...‚½ */
	$"DE82 BD84 0081 DE83 0081 FF85 0081 FF83"            /* Þ‚½„.Þƒ.ÿ….ÿƒ */
	$"0081 FF84 0080 DE85 0080 DE01 0000 8BDE"            /* .ÿ„.€Þ….€Þ...‹Þ */
	$"0100 0083 DE82 0080 DE01 0000 81DE 0100"            /* ...ƒÞ‚.€Þ...Þ.. */
	$"0082 DE04 00DE DE00 0080 DE00 0080 DE08"            /* .‚Þ..ÞÞ..€Þ..€Þ. */
	$"31DE DE31 00DE DE00 0084 DE0B 31DE 31DE"            /* 1ÞÞ1.ÞÞ..„Þ.1Þ1Þ */
	$"00DE DE00 00DE 3131 80DE 0931 31DE 3131"            /* .ÞÞ..Þ11€ÞÆ11Þ11 */
	$"0031 DE00 0080 DE1C 31DE 31DE 31DE 31DE"            /* .1Þ..€Þ.1Þ1Þ1Þ1Þ */
	$"0031 DE00 00DE 3131 DEDE 31DE 31DE 31DE"            /* .1Þ..Þ11ÞÞ1Þ1Þ1Þ */
	$"DE00 DE00 0080 DE0F 31DE 31DE 31DE 31DE"            /* Þ.Þ..€Þ.1Þ1Þ1Þ1Þ */
	$"3100 DE00 00DE 3131 80DE 0931 31DE 31DE"            /* 1.Þ..Þ11€ÞÆ11Þ1Þ */
	$"3100 DE00 008B DE01 0000 82BD 81DE 82BD"            /* 1.Þ..‹Þ...‚½Þ‚½ */
	$"8400 81DE 8300 81FF 8500 81FF 7338 6D6B"            /* „.Þƒ.ÿ….ÿs8mk */
	$"0000 0108 FFFF FFFF FFFF 0000 0000 FFFF"            /* ....ÿÿÿÿÿÿ....ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ....ÿÿÿÿÿÿÿÿ */
	$"0000 0000"                                          /* .... */
};

