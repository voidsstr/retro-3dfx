/* -*-c++-*- */
/* $Header: h3plltable.h, 2, 10/11/00 8:58:16 PM, Brent$ */
/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
**
** $Revision: 2$
** $Date: 10/11/00 8:58:16 PM$
**
** $History: h3plltable.h $
** 
** *****************  Version 1  *****************
** User: Bob          Date: 11/17/98   Time: 2:34p
** Created in $/devel/h3/WinNT/src/Video/Miniport/h3
** Restructured clock control to separate the clocks for Banshee and
** Avenger.  This addition must be used with the checkin occuring on the
** same day.
**
*/

//
// Banshee only!
//
// Constants dealing with the PLL clocks
//
// Values of M,N and K must be selected so that M and N are nonzero,
// and vco must fall in the range 50-200 MHz or so
//

//
// Format hereafter cribbed from plltable.h in h4
//

ULONG H3pllTable[] = {
	NO_CLOCK,	 //  0 MHz -- Invalid clock setting
	NO_CLOCK,	 //  1 MHz -- Invalid clock setting
	NO_CLOCK,	 //  2 MHz -- Invalid clock setting
	NO_CLOCK,	 //  3 MHz -- Invalid clock setting
	NO_CLOCK,	 //  4 MHz -- Invalid clock setting
	NO_CLOCK,	 //  5 MHz -- Invalid clock setting
	NO_CLOCK,	 //  6 MHz -- Invalid clock setting
	NO_CLOCK,	 //  7 MHz -- Invalid clock setting
	NO_CLOCK,	 //  8 MHz -- Invalid clock setting
	NO_CLOCK,	 //  9 MHz -- Invalid clock setting
	NO_CLOCK,	 // 10 MHz -- Invalid clock setting
	NO_CLOCK,	 // 11 MHz -- Invalid clock setting
	NO_CLOCK,	 // 12 MHz -- Invalid clock setting
	NO_CLOCK,	 // 13 MHz -- Invalid clock setting
	NO_CLOCK,	 // 14 MHz -- Invalid clock setting
	NO_CLOCK,	 // 15 MHz -- Invalid clock setting
	NO_CLOCK,	 // 16 MHz -- Invalid clock setting
	NO_CLOCK,	 // 17 MHz -- Invalid clock setting
	NO_CLOCK,	 // 18 MHz -- Invalid clock setting
	NO_CLOCK,	 // 19 MHz -- Invalid clock setting
	NO_CLOCK,	 // 20 MHz -- Invalid clock setting
	NO_CLOCK,	 // 21 MHz -- Invalid clock setting
	NO_CLOCK,	 // 22 MHz -- Invalid clock setting
	NO_CLOCK,	 // 23 MHz -- Invalid clock setting
	NO_CLOCK,	 // 24 MHz -- Invalid clock setting
	NO_CLOCK,	 // 25 MHz -- Invalid clock setting
	NO_CLOCK,	 // 26 MHz -- Invalid clock setting
	NO_CLOCK,	 // 27 MHz -- Invalid clock setting
	NO_CLOCK,	 // 28 MHz -- Invalid clock setting
	NO_CLOCK,	 // 29 MHz -- Invalid clock setting
	NO_CLOCK,	 // 30 MHz -- Invalid clock setting
	NO_CLOCK,	 // 31 MHz -- Invalid clock setting
	NO_CLOCK,	 // 32 MHz -- Invalid clock setting
	NO_CLOCK,	 // 33 MHz -- Invalid clock setting
	NO_CLOCK,	 // 34 MHz -- Invalid clock setting
	NO_CLOCK,	 // 35 MHz -- Invalid clock setting
	NO_CLOCK,	 // 36 MHz -- Invalid clock setting
	NO_CLOCK,	 // 37 MHz -- Invalid clock setting
	NO_CLOCK,	 // 38 MHz -- Invalid clock setting
	NO_CLOCK,	 // 39 MHz -- Invalid clock setting
	NO_CLOCK,	 // 40 MHz -- Invalid clock setting
	NO_CLOCK,	 // 41 MHz -- Invalid clock setting
	NO_CLOCK,	 // 42 MHz -- Invalid clock setting
	NO_CLOCK,	 // 43 MHz -- Invalid clock setting
	NO_CLOCK,	 // 44 MHz -- Invalid clock setting
	NO_CLOCK,	 // 45 MHz -- Invalid clock setting
	NO_CLOCK,	 // 46 MHz -- Invalid clock setting
	NO_CLOCK,	 // 47 MHz -- Invalid clock setting
	NO_CLOCK,	 // 48 MHz -- Invalid clock setting
	NO_CLOCK,	 // 49 MHz -- Invalid clock setting
	0x00001305,  // 050.1136MHz  m:001	n:019  k:1	vco:100.2273MHz
	0x00007039,  // 051.0085MHz  m:014	n:112  k:1	vco:102.0170MHz
	0x0000f53e,  // 052.0087MHz  m:015	n:245  k:2	vco:208.0347MHz
	0x0000eb79,  // 053.0220MHz  m:030	n:235  k:1	vco:106.0440MHz
	0x0000a451,  // 054.0186MHz  m:020	n:164  k:1	vco:108.0372MHz
	0x00009045,  // 055.0120MHz  m:017	n:144  k:1	vco:110.0239MHz
	0x0000833d,  // 056.0094MHz  m:015	n:131  k:1	vco:112.0187MHz
	0x0000d565,  // 057.0076MHz  m:025	n:213  k:1	vco:114.0151MHz
	0x0000e96d,  // 058.0133MHz  m:027	n:233  k:1	vco:116.0266MHz
	0x0000ed6d,  // 059.0008MHz  m:027	n:237  k:1	vco:118.0016MHz
	0x0000ae4d,  // 060.0000MHz  m:019	n:174  k:1	vco:120.0000MHz
	0x0000c255,  // 061.0079MHz  m:021	n:194  k:1	vco:122.0158MHz
	0x00000b04,  // 062.0454MHz  m:001	n:011  k:0	vco:062.0454MHz
	0x00008235,  // 063.0000MHz  m:013	n:130  k:1	vco:126.0000MHz
	0x0000963d,  // 064.0107MHz  m:015	n:150  k:1	vco:128.0214MHz
	0x0000e15d,  // 065.0045MHz  m:023	n:225  k:1	vco:130.0091MHz
	0x0000a441,  // 066.0227MHz  m:016	n:164  k:1	vco:132.0454MHz
	0x0000e85d,  // 067.0091MHz  m:023	n:232  k:1	vco:134.0182MHz
	0x00007029,  // 068.0114MHz  m:010	n:112  k:1	vco:136.0227MHz
	0x0000ef5d,  // 069.0136MHz  m:023	n:239  k:1	vco:138.0272MHz
	0x0000ae41,  // 070.0000MHz  m:016	n:174  k:1	vco:140.0000MHz
	0x0000f65d,  // 071.0182MHz  m:023	n:246  k:1	vco:142.0364MHz
	0x0000a93d,  // 072.0120MHz  m:015	n:169  k:1	vco:144.0240MHz
	0x00009735,  // 073.0227MHz  m:013	n:151  k:1	vco:146.0454MHz
	0x0000ec55,  // 074.0810MHz  m:021	n:236  k:1	vco:148.1620MHz
	0x0000da4d,  // 075.0000MHz  m:019	n:218  k:1	vco:150.0000MHz
	0x0000dd4d,  // 076.0227MHz  m:019	n:221  k:1	vco:152.0454MHz
	0x00009a6c,  // 077.0219MHz  m:027	n:154  k:0	vco:077.0219MHz
	0x00009c6c,  // 078.0094MHz  m:027	n:156  k:0	vco:078.0094MHz
	0x00009364,  // 079.0151MHz  m:025	n:147  k:0	vco:079.0151MHz
	0x0000bc3d,  // 080.0134MHz  m:015	n:188  k:1	vco:160.0267MHz
	0x0000d545,  // 081.0108MHz  m:017	n:213  k:1	vco:162.0215MHz
	0x00007c50,  // 082.0041MHz  m:020	n:124  k:0	vco:082.0041MHz
	0x0000ac35,  // 083.0454MHz  m:013	n:172  k:1	vco:166.0909MHz
	0x00005634,  // 084.0000MHz  m:013	n:086  k:0	vco:084.0000MHz
	0x00005d38,  // 085.0142MHz  m:014	n:093  k:0	vco:085.0142MHz
	0x0000fb4d,  // 086.2500MHz  m:019	n:251  k:1	vco:172.5000MHz
	0x00009c2d,  // 087.0105MHz  m:011	n:156  k:1	vco:174.0210MHz
	0x0000cf3d,  // 088.0147MHz  m:015	n:207  k:1	vco:176.0294MHz
	0x00008d54,  // 089.0217MHz  m:021	n:141  k:0	vco:089.0217MHz
	0x00005630,  // 090.0000MHz  m:012	n:086  k:0	vco:090.0000MHz
	0x00005730,  // 091.0227MHz  m:012	n:087  k:0	vco:091.0227MHz
	0x00005830,  // 092.0454MHz  m:012	n:088  k:0	vco:092.0454MHz
	0x00004c28,  // 093.0682MHz  m:010	n:076  k:0	vco:093.0682MHz
	0x00009554,  // 094.0020MHz  m:021	n:149  k:0	vco:094.0020MHz
	0x00004724,  // 095.0206MHz  m:009	n:071  k:0	vco:095.0206MHz
	0x0000703c,  // 096.0160MHz  m:015	n:112  k:0	vco:096.0160MHz
	0x00007840,  // 097.0454MHz  m:016	n:120  k:0	vco:097.0454MHz
	0x0000572c,  // 098.0245MHz  m:011	n:087  k:0	vco:098.0245MHz
	0x0000582c,  // 099.1259MHz  m:011	n:088  k:0	vco:099.1259MHz
	0x00002805,  // 100.2273MHz  m:001	n:040  k:1	vco:100.2273MHz
	0x00007d1d,  // 101.0227MHz  m:007	n:125  k:1	vco:101.0227MHz
	0x00003709,  // 102.0170MHz  m:002	n:055  k:1	vco:102.0170MHz
	0x00007119,  // 102.9119MHz  m:006	n:113  k:1	vco:103.9119MHz
	0x00003809,  // 103.8068MHz  m:002	n:056  k:1	vco:103.8068MHz
	0x00002a05,  // 105.0000MHz  m:001	n:042  k:1	vco:105.0000MHz
	0x0000480d,  // 105.9545MHz  m:003	n:072  k:1	vco:105.9545MHz
	0x00002b05,  // 107.3864MHz  m:001	n:043  k:1	vco:107.3864MHz
	0x00007719,  // 108.2812MHz  m:006	n:119  k:1	vco:108.2812MHz
	0x00003b09,  // 109.1761MHz  m:002	n:059  k:1	vco:109.1761MHz
	0x00007919,  // 110.0710MHz  m:006	n:121  k:1	vco:110.0710MHz
	0x00003c09,  // 110.9659MHz  m:002	n:060  k:1	vco:110.9659MHz
	0x00007b19,  // 111.8608MHz  m:006	n:123  k:1	vco:111.8608MHz
	0x00004d0d,  // 113.1136MHz  m:003	n:077  k:1	vco:113.1136MHz
	0x00007d19,  // 113.6506MHz  m:006	n:125  k:1	vco:113.6506MHz
	0x00002e05,  // 114.5454MHz  m:001	n:046  k:1	vco:114.5454MHz
	0x0000e96c,  // 116.0266MHz  m:027	n:233  k:0	vco:116.0266MHz
	0x0000eb6c,  // 117.0141MHz  m:027	n:235  k:0	vco:117.0141MHz
	0x0000ed6c,  // 118.0016MHz  m:027	n:237  k:0	vco:118.0016MHz
	0x00008338,  // 119.0199MHz  m:014	n:131  k:0	vco:119.0199MHz
	0x0000ae4c,  // 120.0000MHz  m:019	n:174  k:0	vco:120.0000MHz
	0x0000b850,  // 121.0537MHz  m:020	n:184  k:0	vco:121.0537MHz
	0x0000c254,  // 122.0158MHz  m:021	n:194  k:0	vco:122.0158MHz
	0x0000bb50,  // 123.0062MHz  m:020	n:187  k:0	vco:123.0062MHz
	0x00001804,  // 124.0909MHz  m:001	n:024  k:0	vco:124.0909MHz
	0x0000e160,  // 125.0087MHz  m:024	n:225  k:0	vco:125.0087MHz
	0x00002a0c,  // 126.0000MHz  m:003	n:042  k:0	vco:126.0000MHz
	0x00008c38,  // 127.0738MHz  m:014	n:140  k:0	vco:127.0738MHz
	0x0000963c,  // 128.0214MHz  m:015	n:150  k:0	vco:128.0214MHz
	0x0000fb68,  // 129.3750MHz  m:026	n:251  k:0	vco:129.3750MHz
	0x0000e15c,  // 130.0091MHz  m:023	n:225  k:0	vco:130.0091MHz
	0x0000b548,  // 131.0113MHz  m:018	n:181  k:0	vco:131.0113MHz
	0x0000a440,  // 132.0454MHz  m:016	n:164  k:0	vco:132.0454MHz
	0x0000dd58,  // 133.0398MHz  m:022	n:221  k:0	vco:133.0398MHz
	0x0000e85c,  // 134.0182MHz  m:023	n:232  k:0	vco:134.0182MHz
	0x00008230,  // 135.0000MHz  m:012	n:130  k:0	vco:135.0000MHz
	0x00007028,  // 136.0227MHz  m:010	n:112  k:0	vco:136.0227MHz
	0x00008430,  // 137.0454MHz  m:012	n:132  k:0	vco:137.0454MHz
	0x0000ef5c,  // 138.0272MHz  m:023	n:239  k:0	vco:138.0272MHz
	0x0000e758,  // 139.0057MHz  m:022	n:231  k:0	vco:139.0057MHz
	0x0000ae40,  // 140.0000MHz  m:016	n:174  k:0	vco:140.0000MHz
	0x0000c348,  // 141.0341MHz  m:018	n:195  k:0	vco:141.0341MHz
	0x0000f65c,  // 142.0364MHz  m:023	n:246  k:0	vco:142.0364MHz
	0x00001c04,  // 143.1818MHz  m:001	n:028  k:0	vco:143.1818MHz
	0x0000a93c,  // 144.0240MHz  m:015	n:169  k:0	vco:144.0240MHz
	0x0000e754,  // 145.0494MHz  m:021	n:231  k:0	vco:145.0494MHz
	0x00009734,  // 146.0454MHz  m:013	n:151  k:0	vco:146.0454MHz
	0x00009834,  // 147.0000MHz  m:013	n:152  k:0	vco:147.0000MHz
	0x0000ec54,  // 148.1620MHz  m:021	n:236  k:0	vco:148.1620MHz
	0x0000e350,  // 149.0392MHz  m:020	n:227  k:0	vco:149.0392MHz
	0x0000da4c,  // 150.0000MHz  m:019	n:218  k:0	vco:150.0000MHz
	0x0000d148,  // 151.0568MHz  m:018	n:209  k:0	vco:151.0568MHz
	0x0000dd4c,  // 152.0454MHz  m:019	n:221  k:0	vco:152.0454MHz
	0x0000a938,  // 153.0256MHz  m:014	n:169  k:0	vco:153.0256MHz
	0x0000e04c,  // 154.0909MHz  m:019	n:224  k:0	vco:154.0909MHz
	0x0000f754,  // 155.0099MHz  m:021	n:247  k:0	vco:155.0099MHz
	0x0000d848,  // 156.0682MHz  m:018	n:216  k:0	vco:156.0682MHz
	0x00001f04,  // 157.5000MHz  m:001	n:031  k:0	vco:157.5000MHz
	0x0000fc54,  // 158.1225MHz  m:021	n:252  k:0	vco:158.1225MHz
	0x0000d144,  // 159.0072MHz  m:017	n:209  k:0	vco:159.0072MHz
	0x0000bc3c,  // 160.0267MHz  m:015	n:188  k:0	vco:160.0267MHz
	0x00008528,  // 161.0795MHz  m:010	n:133  k:0	vco:161.0795MHz
	0x0000d544,  // 162.0215MHz  m:017	n:213  k:0	vco:162.0215MHz
	0x0000922c,  // 163.0070MHz  m:011	n:146  k:0	vco:163.0070MHz
	0x0000fa50,  // 164.0082MHz  m:020	n:250  k:0	vco:164.0082MHz
	0x0000f04c,  // 165.0000MHz  m:019	n:240  k:0	vco:165.0000MHz
	0x0000ac34,  // 166.0909MHz  m:013	n:172  k:0	vco:166.0909MHz
	0x00002104,  // 167.0454MHz  m:001	n:033  k:0	vco:167.0454MHz
	0x0000ae34,  // 168.0000MHz  m:013	n:174  k:0	vco:168.0000MHz
	0x0000f64c,  // 169.0909MHz  m:019	n:246  k:0	vco:169.0909MHz
	0x0000bc38	 // 170.0284MHz  m:014	n:188  k:0	vco:170.0284MHz
};
