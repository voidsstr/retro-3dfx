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

#include "DrvVersion.h"

resource 'vers' (2) {
	kRAVE_MAJOR_REV,
	kRAVE_MINOR_REV,
	kRAVE_STAGE_REV,
	kRAVE_LEVEL_REV,
	kRAVE_REGION_CODE,
	kRAVE_VERSION,
	kRAVE_DESCRIPTION
};

resource 'vers' (1) {
	kRAVE_MAJOR_REV,
	kRAVE_MINOR_REV,
	kRAVE_STAGE_REV,
	kRAVE_LEVEL_REV,
	kRAVE_REGION_CODE,
	kRAVE_VERSION,
	kRAVE_VER_FULL
};

data 'tnsl' (0, "Owner resource") {
	kGLOBAL_OWNER
};


resource 'ics#' (-16455) {
	{	/* array: 2 elements */
		/* [1] */
		$"",
		/* [2] */
		$"FC3F FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0FF0"
	}
};


data 'icns' (-16455) {
	$"6963 6E73 0000 10FB 6963 7323 0000 0048"            /* icns...ûics#...H */
	$"FF3F 99F1 B501 E501 E501 EB01 BF61 FFC1"            /* ÿ?™ñµ.å.å.ë.¿aÿÁ */
	$"BFD1 9D51 9D41 BF49 9849 8001 FC3F 0FF0"            /* ¿ÑQA¿I˜I€.ü?.ð */
	$"FF3F FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ?ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0FF0"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.ð */
	$"6963 7338 0000 0108 FFFF FFFF FFFF FFFF"            /* ics8....ÿÿÿÿÿÿÿÿ */
	$"0000 FFFF FFFF FFFF FF2B 2BB0 B054 2BFF"            /* ..ÿÿÿÿÿÿÿ++°°T+ÿ */
	$"FFFF FFFF F6F6 F6FF FF2B B0B0 2BB0 2BFF"            /* ÿÿÿÿöööÿÿ+°°+°+ÿ */
	$"F6F6 F6F6 F6F6 F6FF FFB0 EC2B 2BB0 2BFF"            /* öööööööÿÿ°ì++°+ÿ */
	$"F6F6 F6F6 F6F6 F6FF FFB0 7F33 33EC 2BFF"            /* öööööööÿÿ°.33ì+ÿ */
	$"F6F6 F6F6 17F6 F6FF FFB0 B033 B0F5 FFFF"            /* öööö.ööÿÿ°°3°õÿÿ */
	$"F6F6 F6F6 17F6 F6FF FF2B B0B0 7FFF 5CFF"            /* öööö.ööÿÿ+°°.ÿ\ÿ */
	$"F8FD FBF6 F6F6 F6FF FFFF FFFF FFFF FFFF"            /* øýûööööÿÿÿÿÿÿÿÿÿ */
	$"FAFD F7F7 F6F6 F6FF FFF6 FBFD FDFD FDFC"            /* úý÷÷öööÿÿöûýýýýü */
	$"FAFD F7FD 17F6 F6FF FFF6 F6FB FDFD F9AC"            /* úý÷ý.ööÿÿööûýýù¬ */
	$"F6FD F6FB 17F6 F6FF FFF6 F6FD FBFD F9AC"            /* öýöû.ööÿÿööýûýù¬ */
	$"F6FD F62B F8F6 F6FF FFF6 FBFB FDFD FAFD"            /* öýö+øööÿÿöûûýýúý */
	$"F6FD F62B FDF7 F6FF FFF6 F6FC FAF6 F9F6"            /* öýö+ý÷öÿÿööüúöùö */
	$"F8FB F6F6 FBF6 F6FF FFF7 F7F7 F7F7 F6F6"            /* øûööûööÿÿ÷÷÷÷÷öö */
	$"F6F6 F7F7 F7F7 F7FF FFFF FFFF FFFF F6F6"            /* öö÷÷÷÷÷ÿÿÿÿÿÿÿöö */
	$"F6F6 FFFF FFFF FFFF 0000 0000 FFFF FFFF"            /* ööÿÿÿÿÿÿ....ÿÿÿÿ */
	$"FFFF FFFF 0000 0000 6973 3332 0000 0250"            /* ÿÿÿÿ....is32...P */
	$"8500 01FF FF84 0005 CCCC 3333 99CC 8200"            /* …..ÿÿ„..ÌÌ33™Ì‚. */
	$"80DD 0800 00CC 3333 CC33 CC00 84DD 0800"            /* €Ý...Ì33Ì3Ì.„Ý.. */
	$"0033 00CC CC33 CC00 84DD 0800 0033 66CC"            /* .3.ÌÌ3Ì.„Ý...3fÌ */
	$"CC00 CC00 81DD 0BFF DDDD 0000 3333 CC33"            /* Ì.Ì.Ý.ÿÝÝ..33Ì3 */
	$"EE00 0081 DD0E FFDD DD00 00CC 3333 6600"            /* î..Ý.ÿÝÝ..Ì33f. */
	$"9900 AA22 5581 DD86 0003 7722 BBBB 80DD"            /* ™.ª"UÝ†..w"»»€Ý */
	$"0300 00DD 5581 2249 4477 22BB 22FF DDDD"            /* ...ÝU"IDw"»"ÿÝÝ */
	$"0000 DDDD 5522 2288 33DD 22DD 55FF DDDD"            /* ..ÝÝU""ˆ3Ý"ÝUÿÝÝ */
	$"0000 DDDD 2255 2288 33DD 22DD CCAA DDDD"            /* ..ÝÝ"U"ˆ3Ý"ÝÌªÝÝ */
	$"0000 DD55 5522 2277 22DD 22DD CC22 BBDD"            /* ..ÝUU""w"Ý"ÝÌ"»Ý */
	$"0000 DDDD 4477 DD88 DDAA 55DD DD55 DDDD"            /* ..ÝÝDwÝˆÝªUÝÝUÝÝ */
	$"0000 82BB 81DD 82BB 8400 81DD 8300 81FF"            /* ..‚»Ý‚»„.Ýƒ.ÿ */
	$"8500 81FF 8500 01FF FF84 0005 CCCC 0000"            /* ….ÿ…..ÿÿ„..ÌÌ.. */
	$"99CC 8200 80DD 0800 00CC 0000 CC00 CC00"            /* ™Ì‚.€Ý...Ì..Ì.Ì. */
	$"84DD 8100 04CC CC00 CC00 84DD 8000 0566"            /* „Ý..ÌÌ.Ì.„Ý€..f */
	$"9999 00CC 0081 DD02 66DD DD81 0004 9900"            /* ™™.Ì.Ý.fÝÝ..™. */
	$"EE00 0081 DD0E 66DD DD00 00CC 0000 6600"            /* î..Ý.fÝÝ..Ì..f. */
	$"6600 AA22 5581 DD86 0003 7722 BBBB 80DD"            /* f.ª"UÝ†..w"»»€Ý */
	$"0300 00DD 5581 2249 4477 22BB 2266 DDDD"            /* ...ÝU"IDw"»"fÝÝ */
	$"0000 DDDD 5522 2288 33DD 22DD 5566 DDDD"            /* ..ÝÝU""ˆ3Ý"ÝUfÝÝ */
	$"0000 DDDD 2255 2288 33DD 22DD CCAA DDDD"            /* ..ÝÝ"U"ˆ3Ý"ÝÌªÝÝ */
	$"0000 DD55 5522 2277 22DD 22DD CC22 BBDD"            /* ..ÝUU""w"Ý"ÝÌ"»Ý */
	$"0000 DDDD 4477 DD88 DDAA 55DD DD55 DDDD"            /* ..ÝÝDwÝˆÝªUÝÝUÝÝ */
	$"0000 82BB 81DD 82BB 8400 81DD 8300 81FF"            /* ..‚»Ý‚»„.Ýƒ.ÿ */
	$"8500 81FF 8500 01FF FF84 0005 CCCC 9999"            /* ….ÿ…..ÿÿ„..ÌÌ™™ */
	$"FFCC 8200 80DD 0800 00CC 9999 CC99 CC00"            /* ÿÌ‚.€Ý...Ì™™Ì™Ì. */
	$"84DD 0800 0099 DDCC CC99 CC00 84DD 0800"            /* „Ý...™ÝÌÌ™Ì.„Ý.. */
	$"0099 CC66 66DD CC00 81DD 0B00 DDDD 0000"            /* .™ÌffÝÌ.Ý..ÝÝ.. */
	$"9999 6699 EE00 0081 DD0E 00DD DD00 00CC"            /* ™™f™î..Ý..ÝÝ..Ì */
	$"9999 CC00 9900 AA22 5581 DD86 0003 7722"            /* ™™Ì.™.ª"UÝ†..w" */
	$"BBBB 80DD 0300 00DD 5581 2249 4477 22BB"            /* »»€Ý...ÝU"IDw"» */
	$"2200 DDDD 0000 DDDD 5522 2288 33DD 22DD"            /* ".ÝÝ..ÝÝU""ˆ3Ý"Ý */
	$"5500 DDDD 0000 DDDD 2255 2288 33DD 22DD"            /* U.ÝÝ..ÝÝ"U"ˆ3Ý"Ý */
	$"CCAA DDDD 0000 DD55 5522 2277 22DD 22DD"            /* ÌªÝÝ..ÝUU""w"Ý"Ý */
	$"CC22 BBDD 0000 DDDD 4477 DD88 DDAA 55DD"            /* Ì"»Ý..ÝÝDwÝˆÝªUÝ */
	$"DD55 DDDD 0000 82BB 81DD 82BB 8400 81DD"            /* ÝUÝÝ..‚»Ý‚»„.Ý */
	$"8300 81FF 8500 81FF 4943 4E23 0000 0108"            /* ƒ.ÿ….ÿICN#.... */
	$"07E0 07FE 781E 0201 8001 0101 83C1 FE01"            /* .à.þx...€...ƒÁþ. */
	$"8621 0001 8C31 0001 8831 0001 9831 0001"            /* †!..Œ1..ˆ1..˜1.. */
	$"9831 0001 9821 0001 9809 0001 88D9 0001"            /* ˜1..˜!..˜Æ..ˆÙ.. */
	$"87B9 3801 8001 3001 87E1 2001 781F 7001"            /* ‡¹8.€.0.‡á .x.p. */
	$"839F FA01 869B 7301 8093 2301 8193 2101"            /* ƒŸú.†›s.€“#.“!. */
	$"8193 2001 80D3 2081 84DB 20C1 879F 20C1"            /* “ .€Ó „Û Á‡Ÿ Á */
	$"838B 2041 8000 0001 8000 0001 8000 0001"            /* ƒ‹ A€...€...€... */
	$"7FF0 0FFE 0020 0400 0040 0200 003F FC00"            /* .ð.þ. ...@...?ü. */
	$"07E0 07FE 7FFE 03FF FFFF 01FF FFFF FFFF"            /* .à.þ.þ.ÿÿÿ.ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 7FFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"7FFF FFFE 003F FC00 007F FE00 003F FC00"            /* .ÿÿþ.?ü...þ..?ü. */
	$"6963 6C38 0000 0408 0000 0000 00FF FFFF"            /* icl8.........ÿÿÿ */
	$"FFFF FF00 0000 0000 0000 0000 00FF FFFF"            /* ÿÿÿ..........ÿÿÿ */
	$"FFFF FFFF FFFF FF00 00FF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿ..ÿÿÿÿ... */
	$"0000 00FF FFFF FF00 0000 0000 0000 FF00"            /* ...ÿÿÿÿ.......ÿ. */
	$"0000 0000 0000 00FF FF00 0000 002B 2B2B"            /* .......ÿÿ....+++ */
	$"2B2B 2B00 0000 F7FF 0000 0000 0000 00FF"            /* +++...÷ÿ.......ÿ */
	$"F6F6 F6F6 F6F6 F7FF FF00 2B2B 2B2B 7FB0"            /* öööööö÷ÿÿ.++++.° */
	$"B0B0 542B 2B2B F8FF FFFF FFFF FFFF FF00"            /* °°T+++øÿÿÿÿÿÿÿÿ. */
	$"F6F6 F6F6 F6F6 F7FF FF00 2B2B 2BB0 B02B"            /* öööööö÷ÿÿ.+++°°+ */
	$"2B2B B054 2B2B F8FF 0000 0000 0000 0000"            /* ++°T++øÿ........ */
	$"F6F6 F6F6 F6F6 F7FF FF00 2B2B B0B0 2B2B"            /* öööööö÷ÿÿ.++°°++ */
	$"2B2B 7FB0 2B2B F8FF F6F6 F6F6 F6F6 F6F6"            /* ++.°++øÿöööööööö */
	$"F6F6 F6F6 F6F6 F7FF FF00 2B54 EC54 2B2B"            /* öööööö÷ÿÿ.+TìT++ */
	$"2B2B 7FB0 2B2B F8FF F6F6 F6F6 F6F6 F6F6"            /* ++.°++øÿöööööööö */
	$"F6F6 F6F6 F6F6 F7FF FF00 2BB0 7F2B 2B2B"            /* öööööö÷ÿÿ.+°.+++ */
	$"2B2B 7FB0 2B2B F8FF 0708 0F0F 1016 100F"            /* ++.°++øÿ........ */
	$"F6F6 F6F6 F6F6 F7FF FF00 2BB0 7F2B 3333"            /* öööööö÷ÿÿ.+°.+33 */
	$"332B ECB0 2B2B F8FF 0707 F6F6 F607 0E17"            /* 3+ì°++øÿ..ööö... */
	$"1607 F6F6 F6F6 F7FF FF00 2BB0 7F33 0808"            /* ..öööö÷ÿÿ.+°.3.. */
	$"0833 B054 2B2B F8FF F6F6 F6F6 F6F6 F607"            /* .3°T++øÿööööööö. */
	$"1716 F6F6 F6F6 F7FF FF00 2BB0 7F33 3333"            /* ..öööö÷ÿÿ.+°.333 */
	$"3333 F517 5C2B F8FF F6F6 F6F6 F6F6 F6F6"            /* 33õ.\+øÿöööööööö */
	$"0F17 0FF6 F6F6 F7FF FF00 2B54 B054 2B2B"            /* ...ööö÷ÿÿ.+T°T++ */
	$"7FB0 17D8 FF2B F8FF F6F6 F62B F6F6 F6F6"            /* .°.Øÿ+øÿööö+öööö */
	$"0717 16F6 F6F6 F7FF FF00 2B2B 2AB0 B0B0"            /* ...ööö÷ÿÿ.++*°°° */
	$"7F2B 69FF 5C2B F8FF F6F6 81AC FBF6 F6F6"            /* .+iÿ\+øÿöö¬ûööö */
	$"F616 17F6 F6F6 F7FF FF00 2B2B 2B2B 2B2B"            /* ö..ööö÷ÿÿ.++++++ */
	$"2B2B 2B2B 2B2B F8FF F6F8 FDAC 56F6 F6F6"            /* ++++++øÿöøý¬Vööö */
	$"F616 17F6 F6F6 F7FF FFF7 F8F8 F8FF FFFF"            /* ö..ööö÷ÿÿ÷øøøÿÿÿ */
	$"FFFF FFF8 F8F8 F8FF F656 FD56 F6F6 F6F6"            /* ÿÿÿøøøøÿöVýVöööö */
	$"F616 17F6 F6F6 F7FF 00FF FFFF FFF6 F756"            /* ö..ööö÷ÿ.ÿÿÿÿö÷V */
	$"2BF6 F6FF FFFF FFAC 2BFA FDFA 2BF7 F7F6"            /* +ööÿÿÿÿ¬+úýú+÷÷ö */
	$"0717 16F6 F6F6 F7FF FF00 F6F6 F6F7 FDFD"            /* ...ööö÷ÿÿ.ööö÷ýý */
	$"FC2B F681 FDFC ACFC FAAC ACFD 8156 FDF8"            /* ü+öýü¬üú¬¬ýVýø */
	$"0817 0FF6 F6F6 F7FF FF00 F6F6 F6FB FAF8"            /* ...ööö÷ÿÿ.öööûúø */
	$"FDF9 F7FD 812B ACFC F7FA FD81 F72B ACFC"            /* ýù÷ý+¬ü÷úý÷+¬ü */
	$"1017 07F6 F6F6 F7FF FF00 F6F6 F62B F62B"            /* ...ööö÷ÿÿ.ööö+ö+ */
	$"FDF9 56FD F9F6 ACAC F656 FD56 F6F6 FB88"            /* ýùVýùö¬¬öVýVööûˆ */
	$"170F F6F6 F6F6 F7FF FF00 F6F6 F6F6 F8FB"            /* ..öööö÷ÿÿ.ööööøû */
	$"ACF7 F9FD F9F6 ACAC F656 FD56 F6F6 F85F"            /* ¬÷ùýùö¬¬öVýVööø_ */
	$"1708 F6F6 F6F6 F7FF FF00 F6F6 F6F6 F9FD"            /* ..öööö÷ÿÿ.ööööùý */
	$"ACF7 F9FD F9F6 ACAC F656 FD56 F6F6 2B17"            /* ¬÷ùýùö¬¬öVýVöö+. */
	$"17F6 F6F6 F6F6 F7FF FF00 F6F6 F6F6 F6F7"            /* .ööööö÷ÿÿ.ööööö÷ */
	$"ACFB F9FD F9F6 ACAC F656 FD56 F6F6 0F17"            /* ¬ûùýùö¬¬öVýVöö.. */
	$"88F8 F6F6 F6F6 F7FF FF00 F6F6 F6FA F72B"            /* ˆøöööö÷ÿÿ.öööú÷+ */
	$"ACFB 56FD FAF6 ACAC F656 FD56 F607 1716"            /* ¬ûVýúö¬¬öVýVö... */
	$"AC81 F6F6 F6F6 F7FF FF00 F6F6 F6FB ACFB"            /* ¬öööö÷ÿÿ.öööû¬û */
	$"FD56 2BAC AC81 FDAC F656 FD56 F610 172B"            /* ýV+¬¬ý¬öVýVö..+ */
	$"FBFD F7F6 F6F6 F7FF FF00 F6F6 F62B FCAC"            /* ûý÷ööö÷ÿÿ.ööö+ü¬ */
	$"FAF6 F656 ACF9 8181 F6F8 FBF8 0810 08F6"            /* úööV¬ùöøûø...ö */
	$"56FB 56F6 F6F6 F7FF FF00 F6F6 F6F6 F6F6"            /* VûVööö÷ÿÿ.öööööö */
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"            /* öööööööööööööööö */
	$"F6F6 F6F6 F6F6 F7FF FF00 F6F6 F6F6 F6F6"            /* öööööö÷ÿÿ.öööööö */
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"            /* öööööööööööööööö */
	$"F6F6 F6F6 F6F6 F7FF FF00 F7F7 F7F7 F7F7"            /* öööööö÷ÿÿ.÷÷÷÷÷÷ */
	$"F7F7 F7F7 F6F6 F6F6 F6F6 F6F6 F7F7 F7F7"            /* ÷÷÷÷öööööööö÷÷÷÷ */
	$"F7F7 F7F7 F7F7 F7FF 00FF FFFF FFFF FFFF"            /* ÷÷÷÷÷÷÷ÿ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF F6F6 F6F6 F6F6 F6F6 FFFF FFFF"            /* ÿÿÿÿööööööööÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ......... */
	$"0000 FF00 F6F6 F6F6 F6F6 F6F6 F6FF 0000"            /* ..ÿ.öööööööööÿ.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"00FF 00F6 F7F7 F7F7 F7F7 F7F7 F7F7 FF00"            /* .ÿ.ö÷÷÷÷÷÷÷÷÷÷ÿ. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 0000 696C 3332 0000 0843"            /* ........il32...C */
	$"82FF 8300 87FF 8700 01FF FF81 0083 FF81"            /* ‚ÿƒ.‡ÿ‡..ÿÿ.ƒÿ */
	$"0084 FF00 0085 FF01 0000 81FF 83CC 80FF"            /* .„ÿ..…ÿ...ÿƒÌ€ÿ */
	$"01BB 0084 FF00 0083 DD03 BB00 00FF 81CC"            /* .».„ÿ..ƒÝ.»..ÿÌ */
	$"0066 8033 0099 80CC 00AA 8500 00FF 83DD"            /* .f€3.™€Ì.ª…..ÿƒÝ */
	$"03BB 0000 FF80 CC01 3333 80CC 0533 99CC"            /* .»..ÿ€Ì.33€Ì.3™Ì */
	$"CCAA 0085 FF83 DD07 BB00 00FF CCCC 3333"            /* Ìª.…ÿƒÝ.»..ÿÌÌ33 */
	$"81CC 0566 33CC CCAA 008B DD07 BB00 00FF"            /* Ì.f3ÌÌª.‹Ý.»..ÿ */
	$"CC99 0099 81CC 0566 33CC CCAA 008B DD06"            /* Ì™.™Ì.f3ÌÌª.‹Ý. */
	$"BB00 00FF CC33 6682 CC05 6633 CCCC AA00"            /* »..ÿÌ3f‚Ì.f3ÌÌª. */
	$"85FF 83DD 06BB 0000 FFCC 3366 82CC 0700"            /* …ÿƒÝ.»..ÿÌ3f‚Ì.. */
	$"33CC CCAA 00FF FF80 DD82 FF81 DD07 BB00"            /* 3ÌÌª.ÿÿ€Ý‚ÿÝ.». */
	$"00FF CC33 66CC 80FF 06CC 3399 CCCC AA00"            /* .ÿÌ3fÌ€ÿ.Ì3™ÌÌª. */
	$"84DD 80FF 81DD 06BB 0000 FFCC 3366 82CC"            /* „Ý€ÿÝ.»..ÿÌ3f‚Ì */
	$"05EE FF99 CCAA 0085 DD80 FF80 DD11 BB00"            /* .îÿ™Ìª.…Ý€ÿ€Ý.». */
	$"00FF CC99 3399 CCCC 6633 FFDD 00CC AA00"            /* .ÿÌ™3™ÌÌf3ÿÝ.Ìª. */
	$"80DD 00CC 81DD 80FF 80DD 03BB 0000 FF80"            /* €Ý.ÌÝ€ÿ€Ý.»..ÿ€ */
	$"CC80 330C 66CC 9900 99CC AA00 DDDD 6633"            /* Ì€3.fÌ™.™Ìª.ÝÝf3 */
	$"5581 DD01 FFFF 80DD 03BB 0000 FF89 CC06"            /* UÝ.ÿÿ€Ý.»..ÿ‰Ì. */
	$"AA00 DDAA 2233 9981 DD01 FFFF 80DD 03BB"            /* ª.Ýª"3™Ý.ÿÿ€Ý.» */
	$"0000 BB80 AA83 0081 AA04 00DD 9922 9982"            /* ..»€ªƒ.ª..Ý™"™‚ */
	$"DD01 FFFF 80DD 02BB 00FF 8100 05DD BB99"            /* Ý.ÿÿ€Ý.».ÿ..Ý»™ */
	$"CCDD DD81 0008 33CC 7722 77CC BBBB DD80"            /* ÌÝÝ..3Ìw"wÌ»»Ý€ */
	$"FF80 DD03 BB00 00FF 80DD 12BB 2222 44CC"            /* ÿ€Ý.»..ÿ€Ý.»""DÌ */
	$"DD66 2244 3344 7733 3322 6699 22AA 80FF"            /* Ýf"D3Dw33"f™"ª€ÿ */
	$"80DD 03BB 0000 FF80 DD12 5577 AA22 88BB"            /* €Ý.»..ÿ€Ý.Uwª"ˆ» */
	$"2266 CC33 44BB 7722 66BB CC33 4480 FF80"            /* "fÌ3D»w"f»Ì3D€ÿ€ */
	$"DD03 BB00 00FF 80DD 14CC DDCC 2288 9922"            /* Ý.»..ÿ€Ý.ÌÝÌ"ˆ™" */
	$"88DD 3333 DD99 2299 DDDD 5566 FFFF 81DD"            /* ˆÝ33Ý™"™ÝÝUfÿÿÝ */
	$"03BB 0000 FF81 DD13 AA55 33BB 8822 88DD"            /* .»..ÿÝ.ªU3»ˆ"ˆÝ */
	$"3333 DD99 2299 DDDD AA99 FFFF 81DD 03BB"            /* 33Ý™"™ÝÝª™ÿÿÝ.» */
	$"0000 FF81 DD12 8822 33BB 8822 88DD 3333"            /* ..ÿÝ.ˆ"3»ˆ"ˆÝ33 */
	$"DD99 2299 DDDD CCFF FF82 DD03 BB00 00FF"            /* Ý™"™ÝÝÌÿÿ‚Ý.»..ÿ */
	$"82DD 12BB 3355 8822 88DD 3333 DD99 2299"            /* ‚Ý.»3Uˆ"ˆÝ33Ý™"™ */
	$"DDDD FFFF 66AA 81DD 03BB 0000 FF80 DD0F"            /* ÝÝÿÿfªÝ.»..ÿ€Ý. */
	$"77BB CC33 5599 2277 DD33 33DD 9922 99DD"            /* w»Ì3U™"wÝ33Ý™"™Ý */
	$"80FF 0133 6681 DD03 BB00 00FF 80DD 1555"            /* €ÿ.3fÝ.»..ÿ€Ý.U */
	$"3355 2299 CC33 3366 2233 DD99 2299 DDFF"            /* 3U"™Ì33f"3Ý™"™Ýÿ */
	$"FFCC 5522 BB80 DD03 BB00 00FF 80DD 0ECC"            /* ÿÌU"»€Ý.»..ÿ€Ý.Ì */
	$"4433 77DD DD99 3388 6666 DDAA 55AA 80FF"            /* D3wÝÝ™3ˆffÝªUª€ÿ */
	$"03DD 9955 9980 DD03 BB00 00FF 99DD 03BB"            /* .Ý™U™€Ý.»..ÿ™Ý.» */
	$"0000 FF99 DD03 BB00 00FF 87BB 85DD 88BB"            /* ..ÿ™Ý.»..ÿ‡»…Ýˆ» */
	$"0100 FF88 0085 DD88 0088 FF01 00FF 86DD"            /* ..ÿˆ.…Ýˆ.ˆÿ..ÿ†Ý */
	$"0000 90FF 0200 FFDD 87BB 0000 90FF 8900"            /* ..ÿ..ÿÝ‡»..ÿ‰. */
	$"87FF 82FF 8300 87FF 8700 01FF FF81 0083"            /* ‡ÿ‚ÿƒ.‡ÿ‡..ÿÿ.ƒ */
	$"FF81 0084 FF00 0085 FF01 0000 81FF 83CC"            /* ÿ.„ÿ..…ÿ...ÿƒÌ */
	$"80FF 01BB 0084 FF00 0083 DD03 BB00 00FF"            /* €ÿ.».„ÿ..ƒÝ.»..ÿ */
	$"81CC 0066 8000 0099 80CC 00AA 8500 00FF"            /* Ì.f€..™€Ì.ª…..ÿ */
	$"83DD 03BB 0000 FF80 CC01 0000 80CC 0500"            /* ƒÝ.»..ÿ€Ì...€Ì.. */
	$"99CC CCAA 0085 FF83 DD07 BB00 00FF CCCC"            /* ™ÌÌª.…ÿƒÝ.»..ÿÌÌ */
	$"0000 81CC 0566 00CC CCAA 008B DD07 BB00"            /* ..Ì.f.ÌÌª.‹Ý.». */
	$"00FF CC99 0099 81CC 0566 00CC CCAA 008B"            /* .ÿÌ™.™Ì.f.ÌÌª.‹ */
	$"DD06 BB00 00FF CC00 6682 CC07 6600 CCCC"            /* Ý.»..ÿÌ.f‚Ì.f.ÌÌ */
	$"AA00 CCCC 8099 0266 9999 83DD 07BB 0000"            /* ª.ÌÌ€™.f™™ƒÝ.».. */
	$"FFCC 0066 CC80 9908 CC00 00CC CCAA 00CC"            /* ÿÌ.fÌ€™.Ì..ÌÌª.Ì */
	$"CC80 DD04 CC99 6666 CC81 DD07 BB00 00FF"            /* Ì€Ý.Ì™ffÌÝ.»..ÿ */
	$"CC00 6699 80CC 0699 0099 CCCC AA00 84DD"            /* Ì.f™€Ì.™.™ÌÌª.„Ý */
	$"02CC 6666 81DD 06BB 0000 FFCC 0066 8299"            /* .ÌffÝ.»..ÿÌ.f‚™ */
	$"05EE 6666 CCAA 0085 DD02 9966 9980 DD11"            /* .îffÌª.…Ý.™f™€Ý. */
	$"BB00 00FF CC99 0099 CCCC 6600 6600 00CC"            /* »..ÿÌ™.™ÌÌf.f..Ì */
	$"AA00 80DD 00CC 81DD 02CC 6666 80DD 03BB"            /* ª.€Ý.ÌÝ.Ìff€Ý.» */
	$"0000 FF80 CC80 000C 66CC 0000 66CC AA00"            /* ..ÿ€Ì€..fÌ..fÌª. */
	$"DDDD 6633 5581 DD01 6666 80DD 03BB 0000"            /* ÝÝf3UÝ.ff€Ý.».. */
	$"FF89 CC06 AA00 DDAA 2233 9981 DD01 6666"            /* ÿ‰Ì.ª.Ýª"3™Ý.ff */
	$"80DD 03BB 0000 BB80 AA83 0081 AA04 00DD"            /* €Ý.»..»€ªƒ.ª..Ý */
	$"9922 9982 DD01 6666 80DD 02BB 00FF 8100"            /* ™"™‚Ý.ff€Ý.».ÿ. */
	$"05DD BB99 CCDD DD81 000B 33CC 7722 77CC"            /* .Ý»™ÌÝÝ..3Ìw"wÌ */
	$"BBBB DDCC 6666 80DD 03BB 0000 FF80 DD15"            /* »»ÝÌff€Ý.»..ÿ€Ý. */
	$"BB22 2244 CCDD 6622 4433 4477 3333 2266"            /* »""DÌÝf"D3Dw33"f */
	$"9922 AACC 6699 80DD 03BB 0000 FF80 DD15"            /* ™"ªÌf™€Ý.»..ÿ€Ý. */
	$"5577 AA22 88BB 2266 CC33 44BB 7722 66BB"            /* Uwª"ˆ»"fÌ3D»w"f» */
	$"CC33 4499 66CC 80DD 03BB 0000 FF80 DD14"            /* Ì3D™fÌ€Ý.»..ÿ€Ý. */
	$"CCDD CC22 8899 2288 DD33 33DD 9922 99DD"            /* ÌÝÌ"ˆ™"ˆÝ33Ý™"™Ý */
	$"DD55 3366 9981 DD03 BB00 00FF 81DD 13AA"            /* ÝU3f™Ý.»..ÿÝ.ª */
	$"5533 BB88 2288 DD33 33DD 9922 99DD DDAA"            /* U3»ˆ"ˆÝ33Ý™"™ÝÝª */
	$"6666 CC81 DD03 BB00 00FF 81DD 1288 2233"            /* ffÌÝ.»..ÿÝ.ˆ"3 */
	$"BB88 2288 DD33 33DD 9922 99DD DDCC 6666"            /* »ˆ"ˆÝ33Ý™"™ÝÝÌff */
	$"82DD 03BB 0000 FF82 DD12 BB33 5588 2288"            /* ‚Ý.»..ÿ‚Ý.»3Uˆ"ˆ */
	$"DD33 33DD 9922 99DD DD99 6633 AA81 DD03"            /* Ý33Ý™"™ÝÝ™f3ªÝ. */
	$"BB00 00FF 80DD 1477 BBCC 3355 9922 77DD"            /* »..ÿ€Ý.w»Ì3U™"wÝ */
	$"3333 DD99 2299 DDCC 6666 3366 81DD 03BB"            /* 33Ý™"™ÝÌff3fÝ.» */
	$"0000 FF80 DD15 5533 5522 99CC 3333 6622"            /* ..ÿ€Ý.U3U"™Ì33f" */
	$"33DD 9922 99DD 9966 CC55 22BB 80DD 03BB"            /* 3Ý™"™Ý™fÌU"»€Ý.» */
	$"0000 FF80 DD15 CC44 3377 DDDD 9933 8866"            /* ..ÿ€Ý.ÌD3wÝÝ™3ˆf */
	$"66DD AA55 AACC 99CC DD99 5599 80DD 03BB"            /* fÝªUªÌ™ÌÝ™U™€Ý.» */
	$"0000 FF99 DD03 BB00 00FF 99DD 03BB 0000"            /* ..ÿ™Ý.»..ÿ™Ý.».. */
	$"FF87 BB85 DD88 BB01 00FF 8800 85DD 8800"            /* ÿ‡»…Ýˆ»..ÿˆ.…Ýˆ. */
	$"88FF 0100 FF86 DD00 0090 FF02 00FF DD87"            /* ˆÿ..ÿ†Ý..ÿ..ÿÝ‡ */
	$"BB00 0090 FF89 0087 FF82 FF83 0087 FF87"            /* »..ÿ‰.‡ÿ‚ÿƒ.‡ÿ‡ */
	$"0001 FFFF 8100 83FF 8100 84FF 0000 85FF"            /* ..ÿÿ.ƒÿ.„ÿ..…ÿ */
	$"0100 0081 FF83 CC80 FF01 BB00 84FF 0000"            /* ...ÿƒÌ€ÿ.».„ÿ.. */
	$"83DD 03BB 0000 FF82 CC80 9900 FF80 CC00"            /* ƒÝ.»..ÿ‚Ì€™.ÿ€Ì. */
	$"AA85 0000 FF83 DD03 BB00 00FF 80CC 0199"            /* ª…..ÿƒÝ.»..ÿ€Ì.™ */
	$"9980 CC05 99FF CCCC AA00 85FF 83DD 07BB"            /* ™€Ì.™ÿÌÌª.…ÿƒÝ.» */
	$"0000 FFCC CC99 9982 CC04 99CC CCAA 008B"            /* ..ÿÌÌ™™‚Ì.™ÌÌª.‹ */
	$"DD07 BB00 00FF CCFF DDFF 82CC 0499 CCCC"            /* Ý.»..ÿÌÿÝÿ‚Ì.™ÌÌ */
	$"AA00 8BDD 05BB 0000 FFCC 9984 CC08 99CC"            /* ª.‹Ý.»..ÿÌ™„Ì.™Ì */
	$"CCAA 00CC 9966 6680 3300 6683 DD07 BB00"            /* Ìª.Ì™ff€3.fƒÝ.». */
	$"00FF CC99 CCCC 8066 08CC DD99 CCCC AA00"            /* .ÿÌ™ÌÌ€f.ÌÝ™ÌÌª. */
	$"CCCC 80DD 04CC 9900 33CC 81DD 07BB 0000"            /* ÌÌ€Ý.Ì™.3ÌÝ.».. */
	$"FFCC 99CC 6680 9906 6699 FFCC CCAA 0084"            /* ÿÌ™Ìf€™.f™ÿÌÌª.„ */
	$"DD02 CC00 3381 DD06 BB00 00FF CC99 CC82"            /* Ý.Ì.3Ý.»..ÿÌ™Ì‚ */
	$"6605 EE00 99CC AA00 85DD 0266 0066 80DD"            /* f.î.™Ìª.…Ý.f.f€Ý */
	$"07BB 0000 FFCC FF99 FF80 CC00 9980 0002"            /* .»..ÿÌÿ™ÿ€Ì.™€.. */
	$"CCAA 0080 DD00 CC81 DD02 CC00 3380 DD06"            /* Ìª.€Ý.ÌÝ.Ì.3€Ý. */
	$"BB00 00FF CCCC FF80 990C CCCC 6600 99CC"            /* »..ÿÌÌÿ€™.ÌÌf.™Ì */
	$"AA00 DDDD 6633 5581 DD01 3300 80DD 03BB"            /* ª.ÝÝf3UÝ.3.€Ý.» */
	$"0000 FF89 CC06 AA00 DDAA 2233 9981 DD01"            /* ..ÿ‰Ì.ª.Ýª"3™Ý. */
	$"3300 80DD 03BB 0000 BB80 AA83 0081 AA04"            /* 3.€Ý.»..»€ªƒ.ª. */
	$"00DD 9922 9982 DD01 3300 80DD 02BB 00FF"            /* .Ý™"™‚Ý.3.€Ý.».ÿ */
	$"8100 05DD BB99 CCDD DD81 000B 33CC 7722"            /* ..Ý»™ÌÝÝ..3Ìw" */
	$"77CC BBBB DDCC 0033 80DD 03BB 0000 FF80"            /* wÌ»»ÝÌ.3€Ý.»..ÿ€ */
	$"DD15 BB22 2244 CCDD 6622 4433 4477 3333"            /* Ý.»""DÌÝf"D3Dw33 */
	$"2266 9922 AA99 0066 80DD 03BB 0000 FF80"            /* "f™"ª™.f€Ý.»..ÿ€ */
	$"DD15 5577 AA22 88BB 2266 CC33 44BB 7722"            /* Ý.Uwª"ˆ»"fÌ3D»w" */
	$"66BB CC33 4433 00CC 80DD 03BB 0000 FF80"            /* f»Ì3D3.Ì€Ý.»..ÿ€ */
	$"DD14 CCDD CC22 8899 2288 DD33 33DD 9922"            /* Ý.ÌÝÌ"ˆ™"ˆÝ33Ý™" */
	$"99DD DD55 3300 6681 DD03 BB00 00FF 81DD"            /* ™ÝÝU3.fÝ.»..ÿÝ */
	$"13AA 5533 BB88 2288 DD33 33DD 9922 99DD"            /* .ªU3»ˆ"ˆÝ33Ý™"™Ý */
	$"DDAA 0000 9981 DD03 BB00 00FF 81DD 1288"            /* Ýª..™Ý.»..ÿÝ.ˆ */
	$"2233 BB88 2288 DD33 33DD 9922 99DD DDCC"            /* "3»ˆ"ˆÝ33Ý™"™ÝÝÌ */
	$"0000 82DD 03BB 0000 FF82 DD12 BB33 5588"            /* ..‚Ý.»..ÿ‚Ý.»3Uˆ */
	$"2288 DD33 33DD 9922 99DD DD66 0033 AA81"            /* "ˆÝ33Ý™"™ÝÝf.3ª */
	$"DD03 BB00 00FF 80DD 1477 BBCC 3355 9922"            /* Ý.»..ÿ€Ý.w»Ì3U™" */
	$"77DD 3333 DD99 2299 DDCC 0033 3366 81DD"            /* wÝ33Ý™"™ÝÌ.33fÝ */
	$"03BB 0000 FF80 DD15 5533 5522 99CC 3333"            /* .»..ÿ€Ý.U3U"™Ì33 */
	$"6622 33DD 9922 99DD 3300 CC55 22BB 80DD"            /* f"3Ý™"™Ý3.ÌU"»€Ý */
	$"03BB 0000 FF80 DD15 CC44 3377 DDDD 9933"            /* .»..ÿ€Ý.ÌD3wÝÝ™3 */
	$"8866 66DD AA55 AA99 3399 DD99 5599 80DD"            /* ˆffÝªUª™3™Ý™U™€Ý */
	$"03BB 0000 FF99 DD03 BB00 00FF 99DD 03BB"            /* .»..ÿ™Ý.»..ÿ™Ý.» */
	$"0000 FF87 BB85 DD88 BB01 00FF 8800 85DD"            /* ..ÿ‡»…Ýˆ»..ÿˆ.…Ý */
	$"8800 88FF 0100 FF86 DD00 0090 FF02 00FF"            /* ˆ.ˆÿ..ÿ†Ý..ÿ..ÿ */
	$"DD87 BB00 0090 FF89 0087 FF"                        /* Ý‡»..ÿ‰.‡ÿ */
};

resource 'icl8' (-16455) {
	$"0000 0000 00FF FFFF FFFF FF00 0000 0000"
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FF00"
	$"00FF FFFF FF00 0000 0000 00FF FFFF FF00"
	$"0000 0000 0000 FF00 0000 0000 0000 00FF"
	$"FF00 0000 002B 2B2B 2B2B 2B00 0000 F7FF"
	$"0000 0000 0000 00FF F6F6 F6F6 F6F6 F7FF"
	$"FF00 2B2B 2B2B 7FB0 B0B0 542B 2B2B F8FF"
	$"FFFF FFFF FFFF FF00 F6F6 F6F6 F6F6 F7FF"
	$"FF00 2B2B 2BB0 B02B 2B2B B054 2B2B F8FF"
	$"0000 0000 0000 0000 F6F6 F6F6 F6F6 F7FF"
	$"FF00 2B2B B0B0 2B2B 2B2B 7FB0 2B2B F8FF"
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"
	$"FF00 2B54 EC54 2B2B 2B2B 7FB0 2B2B F8FF"
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"
	$"FF00 2BB0 7F2B 2B2B 2B2B 7FB0 2B2B F8FF"
	$"0708 0F0F 1016 100F F6F6 F6F6 F6F6 F7FF"
	$"FF00 2BB0 7F2B 3333 332B ECB0 2B2B F8FF"
	$"0707 F6F6 F607 0E17 1607 F6F6 F6F6 F7FF"
	$"FF00 2BB0 7F33 0808 0833 B054 2B2B F8FF"
	$"F6F6 F6F6 F6F6 F607 1716 F6F6 F6F6 F7FF"
	$"FF00 2BB0 7F33 3333 3333 F517 5C2B F8FF"
	$"F6F6 F6F6 F6F6 F6F6 0F17 0FF6 F6F6 F7FF"
	$"FF00 2B54 B054 2B2B 7FB0 17D8 FF2B F8FF"
	$"F6F6 F62B F6F6 F6F6 0717 16F6 F6F6 F7FF"
	$"FF00 2B2B 2AB0 B0B0 7F2B 69FF 5C2B F8FF"
	$"F6F6 81AC FBF6 F6F6 F616 17F6 F6F6 F7FF"
	$"FF00 2B2B 2B2B 2B2B 2B2B 2B2B 2B2B F8FF"
	$"F6F8 FDAC 56F6 F6F6 F616 17F6 F6F6 F7FF"
	$"FFF7 F8F8 F8FF FFFF FFFF FFF8 F8F8 F8FF"
	$"F656 FD56 F6F6 F6F6 F616 17F6 F6F6 F7FF"
	$"00FF FFFF FFF6 F756 2BF6 F6FF FFFF FFAC"
	$"2BFA FDFA 2BF7 F7F6 0717 16F6 F6F6 F7FF"
	$"FF00 F6F6 F6F7 FDFD FC2B F681 FDFC ACFC"
	$"FAAC ACFD 8156 FDF8 0817 0FF6 F6F6 F7FF"
	$"FF00 F6F6 F6FB FAF8 FDF9 F7FD 812B ACFC"
	$"F7FA FD81 F72B ACFC 1017 07F6 F6F6 F7FF"
	$"FF00 F6F6 F62B F62B FDF9 56FD F9F6 ACAC"
	$"F656 FD56 F6F6 FB88 170F F6F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F8FB ACF7 F9FD F9F6 ACAC"
	$"F656 FD56 F6F6 F85F 1708 F6F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F9FD ACF7 F9FD F9F6 ACAC"
	$"F656 FD56 F6F6 2B17 17F6 F6F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F7 ACFB F9FD F9F6 ACAC"
	$"F656 FD56 F6F6 0F17 88F8 F6F6 F6F6 F7FF"
	$"FF00 F6F6 F6FA F72B ACFB 56FD FAF6 ACAC"
	$"F656 FD56 F607 1716 AC81 F6F6 F6F6 F7FF"
	$"FF00 F6F6 F6FB ACFB FD56 2BAC AC81 FDAC"
	$"F656 FD56 F610 172B FBFD F7F6 F6F6 F7FF"
	$"FF00 F6F6 F62B FCAC FAF6 F656 ACF9 8181"
	$"F6F8 FBF8 0810 08F6 56FB 56F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"
	$"FF00 F7F7 F7F7 F7F7 F7F7 F7F7 F6F6 F6F6"
	$"F6F6 F6F6 F7F7 F7F7 F7F7 F7F7 F7F7 F7FF"
	$"00FF FFFF FFFF FFFF FFFF FFFF F6F6 F6F6"
	$"F6F6 F6F6 FFFF FFFF FFFF FFFF FFFF FF00"
	$"0000 0000 0000 0000 0000 FF00 F6F6 F6F6"
	$"F6F6 F6F6 F6FF 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 00FF 00F6 F7F7 F7F7"
	$"F7F7 F7F7 F7F7 FF00 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 FFFF FFFF FFFF"
	$"FFFF FFFF FFFF"
};

resource 'icl4' (-16455) {
	$"0000 0FFF FFF0 0000 0000 0FFF FFFF FFF0"
	$"0FFF F000 000F FFF0 0000 00F0 0000 000F"
	$"F000 0CCC CCC0 00CF 0000 000F CCCC CCCF"
	$"F0CC CCD5 55CC CCCF FFFF FFF0 CCCC CCCF"
	$"F0CC C55C CC5C CCCF 0000 0000 CCCC CCCF"
	$"F0CC 55CC CCD5 CCCF CCCC CCCC CCCC CCCF"
	$"F0CC 6CCC CCD5 CCCF CCCC CCCC CCCC CCCF"
	$"F0C5 DCCC CCD5 CCCF CC22 2222 CCCC CCCF"
	$"F0C5 DCDD DC65 CCCF CCCC CCC2 2CCC CCCF"
	$"F0C5 DDCC CD5C CCCF CCCC CCCC 22CC CCCF"
	$"F0C5 DDDD DD02 DCCF CCCC CCCC 222C CCCF"
	$"F0CC 5CCC D523 FCCF CCCC CCCC C22C CCCF"
	$"F0CC C555 DC4F DCCF CCDE ECCC C22C CCCF"
	$"F0CC CCCC CCCC CCCF CCAE DCCC C22C CCCF"
	$"FCCC CFFF FFFC CCCF CDAD CCCC C22C CCCF"
	$"0FFF FCCD CCCF FFFE CDAD CCCC C22C CCCF"
	$"F0CC CCAA ECCD AEEE DEEA DDAC C22C CCCF"
	$"F0CC CEDC ADCA DCEE CDAD CCEE 22CC CCCF"
	$"F0CC CCCC ADDA DCEE CDAD CCEE 22CC CCCF"
	$"F0CC CCCE ECDA DCEE CDAD CCCB 2CCC CCCF"
	$"F0CC CCDA ECDA DCEE CDAD CCC2 2CCC CCCF"
	$"F0CC CCCC EEDA DCEE CDAD CC22 ECCC CCCF"
	$"F0CC CDCC EEDA DCEE CDAD CC22 EDCC CCCF"
	$"F0CC CEEE ADCE EDAE CDAD C22C EACC CCCF"
	$"F0CC CCEE DCCD EDDD CCEC C2CC DEDC CCCF"
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"
	$"0FFF FFFF FFFF CCCC CCCC FFFF FFFF FFF0"
	$"0000 0000 00F0 CCCC CCCC CF00 0000 0000"
	$"0000 0000 0F0C CCCC CCCC CCF0 0000 0000"
	$"0000 0000 00FF FFFF FFFF FF"
};

resource 'ICN#' (-16455) {
	{	/* array: 2 elements */
		/* [1] */
		$"",
		/* [2] */
		$"07E0 07FE 7FFE 03FF FFFF 01FF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF 7FFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"7FFF FFFE 003F FC00 007F FE00 003F FC"
	}
};

resource 'ics8' (-16455) {
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

resource 'ics4' (-16455) {
	$"FFFF FF00 00FF FFFF FCCC FFFF FFFF CCCF"
	$"FCCC CCCC CCCC CCCF FCCC CCC2 2222 CCCF"
	$"FCCC C22C CCCC 2CCF FCCC 2CCC ECCE 2CCF"
	$"FCCC CCCC ECEC 2CCF FCEE CCCE ECEE 2ECF"
	$"FCCC ECEC ECEC 2ECF FCEE CCEC ECEC C2CF"
	$"FCCC ECEC ECEC E2CF FCEE CCCE ECEC E2CF"
	$"FCCC CCCC CCCC CCCF FCCC CCCC CCCC CCCF"
	$"FFFF FFCC CCFF FFFF 0000 FFFF FFFF"
};


