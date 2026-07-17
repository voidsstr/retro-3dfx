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
** $Header:  $
** $Log: $
**
*/

#include "Types.r"
#include "MacTypes.r"

resource 'vers' (1) {
	0x1,
	0x0,
	beta,
	0x12,
	0,
	"1.0b12",
	"1.0b12 , ©1999 3dfx Interactive, Inc.\n"
	"All rights reserved."
};

resource 'vers' (2) {
	0x1,
	0x0,
	beta,
	0x12,
	0,
	"1.0b12",
	"3dfx Multimedia Component"
};

type 'sysz' {
  unsigned longint;
};

resource 'sysz' (0) {
    0x200000
};

resource 'BNDL' (128) {
	'3DFX',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'FREF',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 128
		},
		/* [2] */
		'ICN#',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 128
		}
	}
};

resource 'FREF' (128) {
	'INIT',
	0,
	""
};

data '3DFX' (0, "Owner resource") {
	"©1999 3dfx Interactive, Inc. All rights reserved\n\n"
};



resource 'icl8' (128) {
	$"00FF FFFF FFFF FFFF FFFF FF00 0000 0000"
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FF00"
	$"FF00 0000 F6F6 F6F6 F8FF FFFF FFFF FFFF"
	$"FFF8 0000 0000 FF00 0000 0000 0000 00FF"
	$"FF00 2BF8 F8FA FDF8 FF2B 2B2B 2B2B 2B2B"
	$"2BFF 0000 0000 00FF F6F6 F6F6 F6F6 F7FF"
	$"FFF6 F6F6 F6F6 F6FF 2BF8 FCFC FCFC FCF8"
	$"2BFF FFFF FFFF FF00 F6F6 F6F6 F6F6 F7FF"
	$"FFF6 2BF8 FAFD F8FF 2BFC 542A 2A2A 2A00"
	$"F8FF 0000 0000 0000 F6F6 F6F6 F6F6 F7FF"
	$"FFF8 F8FA FCFD F8FF 2BFC 2A2A 2A2A 2A00"
	$"F8FF F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"
	$"FFF6 F6F6 F6F6 FF2B FC54 2A2A 2A2A 002B"
	$"FFFA F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F7FF"
	$"FFF8 FAFC FDF8 FF2B FC2A 2A2A 2A2A 00F8"
	$"FFF8 0F0F 1016 100F F6F6 F6F6 F6F6 F7FF"
	$"FFF6 F6F6 F6FF 2BFC 542A 2A2A 2A00 2BFF"
	$"FAF6 F6F6 F607 0E17 1607 F6F6 F6F6 F7FF"
	$"FFF8 FAFC F6FF 2B2B 0000 0000 002B F8FF"
	$"F6F6 F6F6 F6F6 F607 1716 F6F6 F6F6 F7FF"
	$"FFF6 F6F6 F6FF 2B2B E3D8 2B2B 2B2B FFF8"
	$"F6F6 F6F6 F6F6 F6F6 0F17 0FF6 F6F6 F7FF"
	$"FFFA FAFC FDF8 FFFF FFFF FFFF FFFF F8F6"
	$"F6F6 F62B F6F6 F6F6 0717 16F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6"
	$"F6F6 81AC FBF6 F6F6 F616 17F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 ACAC"
	$"F6F8 FDAC 56F6 F6F6 F616 17F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 ACAC"
	$"F656 FD56 F6F6 F6F6 F616 17F6 F6F6 F7FF"
	$"FF00 F6F6 F6F6 F756 2BF6 F6F6 56F7 FCFC"
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

resource 'icl8' (129) {
	$"00FF FFFF FFFF FFFF FFFF FF00 0000 0000"            /* .яяяяяяяяяя..... */
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FF00"            /* .....яяяяяяяяяя. */
	$"FF00 0000 F6F6 F6F6 F8FF FFFF FFFF FFFF"            /* я...ццццшяяяяяяя */
	$"FFF8 0000 0000 FF00 0000 0000 0000 00FF"            /* яш....я........я */
	$"FF00 2BF8 F8D8 FDF8 FF2B 2B2B 2B2B 2B2B"            /* я.+шшШэшя+++++++ */
	$"2BFF 0000 0000 00FF F6F6 F6D8 F6F6 F7FF"            /* +я.....яцццШццчя */
	$"FFF6 F6F6 D8D8 D8FF 2BF8 FCFC FCFC FCF8"            /* яцццШШШя+шьььььш */
	$"2BFF FFFF FFFF FF00 F6F6 D8D8 D8F6 F7FF"            /* +яяяяяя.ццШШШцчя */
	$"FFF6 2BF8 FAD8 D8D8 2BFC 542A 2A2A 2A00"            /* яц+шъШШШ+ьT****. */
	$"F8FF 0000 0000 0000 F6D8 D8D8 F6F6 F7FF"            /* шя......цШШШццчя */
	$"FFF8 F8FA FCFD D8D8 D8FC 2A2A 2A2A 2A00"            /* яшшъьэШШШь*****. */
	$"F8FF F6F6 F6F6 F6F6 D8D8 D8F6 F6F6 F7FF"            /* шяццццццШШШцццчя */
	$"FFF6 F6F6 F6F6 FFD8 D8D8 2A2A 2A2A 002B"            /* яцццццяШШШ****.+ */
	$"FFFA F6F6 F6F6 F6D8 D8D8 F6F6 F6F6 F7FF"            /* яъцццццШШШццццчя */
	$"FFF8 FAFC FDF8 FF2B D8D8 D82A 2A2A 00F8"            /* яшъьэшя+ШШШ***.ш */
	$"FFF8 0F0F 1016 D8D8 D8F6 F6F6 F6F6 F7FF"            /* яш....ШШШцццццчя */
	$"FFF6 F6F6 F6FF 2BFC 54D8 D8D8 2A00 2BFF"            /* яцццця+ьTШШШ*.+я */
	$"FAF6 F6F6 F6D8 D8D8 1607 F6F6 F6F6 F7FF"            /* ъццццШШШ..ццццчя */
	$"FFF8 FAFC F6FF 2B2B 0000 D8D8 D82B F8FF"            /* яшъьця++..ШШШ+шя */
	$"F6F6 F6F6 D8D8 D807 1716 F6F6 F6F6 F7FF"            /* ццццШШШ...ццццчя */
	$"FFF6 F6F6 F6FF 2B2B E3D8 2BD8 D8D8 FFF8"            /* яцццця++гШ+ШШШяш */
	$"F6F6 F6D8 D8D8 F6F6 0F17 0FF6 F6F6 F7FF"            /* цццШШШцц...цццчя */
	$"FFFA FAFC FDF8 FFFF FFFF FFFF D8D8 D8F6"            /* яъъьэшяяяяяяШШШц */
	$"F6F6 D8D8 D8F6 F6F6 0717 16F6 F6F6 F7FF"            /* ццШШШццц...цццчя */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6D8 D8D8"            /* я.цццццццццццШШШ */
	$"F6D8 D8D8 FBF6 F6F6 F616 17F6 F6F6 F7FF"            /* цШШШыцццц..цццчя */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 D8D8"            /* я.ццццццццццццШШ */
	$"D8D8 D8AC 56F6 F6F6 F616 17F6 F6F6 F7FF"            /* ШШШ¬Vцццц..цццчя */
	$"FF00 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 ACD8"            /* я.цццццццццццц¬Ш */
	$"D8D8 FD56 F6F6 F6F6 F616 17F6 F6F6 F7FF"            /* ШШэVццццц..цццчя */
	$"FF00 F6F6 F6F6 F756 2BF6 F6F6 56F7 D8D8"            /* я.ццццчV+цццVчШШ */
	$"D8D8 D8FA 2BF7 F7F6 0717 16F6 F6F6 F7FF"            /* ШШШъ+ччц...цццчя */
	$"FF00 F6F6 F6F7 FDFD FC2B F681 FDD8 D8D8"            /* я.цццчээь+цЃэШШШ */
	$"FAD8 D8D8 8156 FDF8 0817 0FF6 F6F6 F7FF"            /* ъШШШЃVэш...цццчя */
	$"FF00 F6F6 F6FB FAF8 FDF9 F7FD D8D8 D8FC"            /* я.цццыъшэщчэШШШь */
	$"F7FA D8D8 D82B ACFC 1017 07F6 F6F6 F7FF"            /* чъШШШ+¬ь...цццчя */
	$"FF00 F6F6 F62B F62B FDF9 56D8 D8D8 ACAC"            /* я.ццц+ц+эщVШШШ¬¬ */
	$"F656 FDD8 D8D8 FB88 170F F6F6 F6F6 F7FF"            /* цVэШШШы€..ццццчя */
	$"FF00 F6F6 F6F6 F8FB ACF7 D8D8 D8F6 ACAC"            /* я.ццццшы¬чШШШц¬¬ */
	$"F656 FD56 D8D8 D85F 1708 F6F6 F6F6 F7FF"            /* цVэVШШШ_..ццццчя */
	$"FF00 F6F6 F6F6 F9FD ACD8 D8D8 F9F6 ACAC"            /* я.ццццщэ¬ШШШщц¬¬ */
	$"F656 FD56 F6D8 D8D8 17F6 F6F6 F6F6 F7FF"            /* цVэVцШШШ.цццццчя */
	$"FF00 F6F6 F6F6 F6F7 D8D8 D8FD F9F6 ACAC"            /* я.цццццчШШШэщц¬¬ */
	$"F656 FD56 F6F6 D8D8 D8F8 F6F6 F6F6 F7FF"            /* цVэVццШШШшццццчя */
	$"FF00 F6F6 F6FA F7D8 D8D8 56FD FAF6 ACAC"            /* я.цццъчШШШVэъц¬¬ */
	$"F656 FD56 F607 17D8 D8D8 F6F6 F6F6 F7FF"            /* цVэVц..ШШШццццчя */
	$"FF00 F6F6 F6FB D8D8 D856 2BAC AC81 FDAC"            /* я.цццыШШШV+¬¬Ѓэ¬ */
	$"F656 FD56 F610 172B D8D8 D8F6 F6F6 F7FF"            /* цVэVц..+ШШШцццчя */
	$"FF00 F6F6 F6D8 D8D8 FAF6 F656 ACF9 8181"            /* я.цццШШШъццV¬щЃЃ */
	$"F6F8 FBF8 0810 08F6 56D8 D8D8 F6F6 F7FF"            /* цшыш...цVШШШццчя */
	$"FF00 F6F6 D8D8 D8F6 F6F6 F6F6 F6F6 F6F6"            /* я.ццШШШццццццццц */
	$"F6F6 F6F6 F6F6 F6F6 F6F6 D8D8 D8F6 F7FF"            /* ццццццццццШШШцчя */
	$"FF00 F6F6 F6D8 F6F6 F6F6 F6F6 F6F6 F6F6"            /* я.цццШцццццццццц */
	$"F6F6 F6F6 F6F6 F6F6 F6F6 F6D8 F6F6 F7FF"            /* цццццццццццШццчя */
	$"FF00 F7F7 F7F7 F7F7 F7F7 F7F7 F6F6 F6F6"            /* я.ччччччччччцццц */
	$"F6F6 F6F6 F7F7 F7F7 F7F7 F7F7 F7F7 F7FF"            /* ццццчччччччччччя */
	$"00FF FFFF FFFF FFFF FFFF FFFF F6F6 F6F6"            /* .яяяяяяяяяяяцццц */
	$"F6F6 F6F6 FFFF FFFF FFFF FFFF FFFF FF00"            /* ццццяяяяяяяяяяя. */
	$"0000 0000 0000 0000 0000 FF00 F6F6 F6F6"            /* ..........я.цццц */
	$"F6F6 F6F6 F6FF 0000 0000 0000 0000 0000"            /* ццццця.......... */
	$"0000 0000 0000 0000 00FF 00F6 F7F7 F7F7"            /* .........я.цчччч */
	$"F7F7 F7F7 F7F7 FF00 0000 0000 0000 0000"            /* ччччччя......... */
	$"0000 0000 0000 0000 0000 FFFF FFFF FFFF"            /* ..........яяяяяя */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* яяяяяя.......... */
};

resource 'icl4' (128) {
	$"0FFF FFFF FFF0 0000 0000 0FFF FFFF FFF0"
	$"F000 CCCC CFFF FFFF FC00 00F0 0000 000F"
	$"F0CC CDAC FCCC CCCC CF00 000F CCCC CCCF"
	$"FCCC CCCF CCEE EEEC CFFF FFF0 CCCC CCCF"
	$"FCCC DACF CECC CCC0 CF00 0000 CCCC CCCF"
	$"FCCD EACF CECC CCC0 CFCC CCCC CCCC CCCF"
	$"FCCC CCFC ECCC CC0C FDCC CCCC CCCC CCCF"
	$"FCDE ACFC ECCC CC0C FC22 2222 CCCC CCCF"
	$"FCCC CFCE CCCC C0CF DCCC CCC2 2CCC CCCF"
	$"FCDE CFCC 0000 0CCF CCCC CCCC 22CC CCCF"
	$"FCCC CFCC 83CC CCFC CCCC CCCC 222C CCCF"
	$"FDDE ACFF FFFF FFCC CCCC CCCC C22C CCCF"
	$"F0CC CCCC CCCC CCCC CCDE ECCC C22C CCCF"
	$"F0CC CCCC CCCC CCEE CCAE DCCC C22C CCCF"
	$"F0CC CCCC CCCC CCEE CDAD CCCC C22C CCCF"
	$"F0CC CCCD CCCC DCEE CDAD CCCC C22C CCCF"
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

resource 'icl4' (129) {
	$"0FFF FFFF FFF0 0000 0000 0FFF FFFF FFF0"            /* .яяяяр.....яяяяр */
	$"F000 CCCC CFFF FFFF FC00 00F0 0000 000F"            /* р.ММПяяяь..р.... */
	$"F0CC C3AC FCCC CCCC CF00 000F CCC3 CCCF"            /* рМГ¬ьМММП...МГМП */
	$"FCCC 333F CCEE EEEC CFFF FFF0 CC33 3CCF"            /* ьМ3?МоомПяярМ3<П */
	$"FCCC D333 CECC CCC0 CF00 0000 C333 CCCF"            /* ьМУ3ОММАП...Г3МП */
	$"FCCD EA33 3ECC CCC0 CFCC CCCC 333C CCCF"            /* ьНк3>ММАПМММ3<МП */
	$"FCCC CCF3 33CC CC0C FDCC CCC3 33CC CCCF"            /* ьММу3ММ.эММГ3ММП */
	$"FCDE ACFC 333C CC0C FC22 2233 3CCC CCCF"            /* ьЮ¬ь3<М.ь""3<ММП */
	$"FCCC CFCE C333 C0CF DCCC C333 2CCC CCCF"            /* ьМПОГ3АПЬМГ3,ММП */
	$"FCDE CFCC 0033 3CCF CCCC 333C 22CC CCCF"            /* ьЮПМ.3<ПММ3<"ММП */
	$"FCCC CFCC 83C3 33FC CCC3 33CC 222C CCCF"            /* ьМПМѓГ3ьМГ3М",МП */
	$"FDDE ACFF FFFF 333C CC33 3CCC C22C CCCF"            /* эЮ¬яяя3<М3<МВ,МП */
	$"F0CC CCCC CCCC C333 C333 ECCC C22C CCCF"            /* рМММММГ3Г3мМВ,МП */
	$"F0CC CCCC CCCC CC33 333E DCCC C22C CCCF"            /* рММММММ33>ЬМВ,МП */
	$"F0CC CCCC CCCC CCE3 33AD CCCC C22C CCCF"            /* рММММММг3­ММВ,МП */
	$"F0CC CCCD CCCC DC33 3333 CCCC C22C CCCF"            /* рММНММЬ333ММВ,МП */
	$"F0CC CCAA ECCD A333 D333 DDAC C22C CCCF"            /* рММЄмНЈ3У3Э¬В,МП */
	$"F0CC CEDC ADCA 333E CD33 3CEE 22CC CCCF"            /* рМОЬ­К3>Н3<о"ММП */
	$"F0CC CCCC ADD3 33EE CDA3 33EE 22CC CCCF"            /* рМММ­У3оНЈ3о"ММП */
	$"F0CC CCCE EC33 3CEE CDAD 333B 2CCC CCCF"            /* рММОм3<оН­3;,ММП */
	$"F0CC CCDA E333 DCEE CDAD C333 2CCC CCCF"            /* рММЪг3ЬоН­Г3,ММП */
	$"F0CC CCCC 333A DCEE CDAD CC33 3CCC CCCF"            /* рМММ3:ЬоН­М3<ММП */
	$"F0CC CDC3 33DA DCEE CDAD CC23 33CC CCCF"            /* рМНГ3ЪЬоН­М#3ММП */
	$"F0CC CE33 3DCE EDAE CDAD C22C 333C CCCF"            /* рМО3=Он®Н­В,3<МП */
	$"F0CC C333 DCCD EDDD CCEC C2CC D333 CCCF"            /* рМГ3ЬНнЭМмВМУ3МП */
	$"F0CC 333C CCCC CCCC CCCC CCCC CC33 3CCF"            /* рМ3<МММММММММ3<П */
	$"F0CC C3CC CCCC CCCC CCCC CCCC CCC3 CCCF"            /* рМГММММММММММГМП */
	$"F0CC CCCC CCCC CCCC CCCC CCCC CCCC CCCF"            /* рММММММММММММММП */
	$"0FFF FFFF FFFF CCCC CCCC FFFF FFFF FFF0"            /* .яяяяяММММяяяяяр */
	$"0000 0000 00F0 CCCC CCCC CF00 0000 0000"            /* .....рММММП..... */
	$"0000 0000 0F0C CCCC CCCC CCF0 0000 0000"            /* ......МММММр.... */
	$"0000 0000 00FF FFFF FFFF FF00 0000 0000"            /* .....яяяяяя..... */
};

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"7FE0 07FE 807F 8201 8680 4101 813E 7E01"
		$"8D40 4001 9D40 4001 8280 C001 BA80 BF01"
		$"8501 E181 B401 00C1 84C2 0061 FBFC 0061"
		$"8000 1861 8003 3821 8003 3021 8003 3061"
		$"838F 7A61 86DF 7B61 80D3 3341 8193 31C1"
		$"8193 3181 80D3 3181 84D3 33C1 879F 32C1"
		$"838F 3241 8000 0001 8000 0001 8000 0001"
		$"7FF0 0FFE 0020 0400 0040 0200 003F FC",
		/* [2] */
		$"7FE0 07FE FFFF C3FF FFFF C1FF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"7FFF FFFE 003F FC00 007F FE00 003F FC"
	}
};

resource 'ICN#' (129) {
  {
	$"7FE0 07FE 807F 8201 8E80 4111 9D3E 7E39"            /* .а.юЂ.‚.ЋЂA.ќ>~9 */
	$"8F40 4071 9FC0 40E1 83C0 C1C1 BAE0 BF81"            /* Џ@@qџА@бѓАББєаїЃ */
	$"8571 E781 B439 0EC1 84DE 1C61 FBFE 3861"            /* …qзЃґ9.Б„Ю.aыю8a */
	$"8007 7861 8003 F821 8003 F021 8003 F061"            /* Ђ.xaЂ.ш!Ђ.р!Ђ.рa */
	$"838F 7A61 86DF 7B61 80DF 3F41 81BB 3FC1"            /* ѓЏza†Я{aЂЯ?AЃ»?Б */
	$"81F3 3781 80F3 3381 85D3 33C1 879F 32E1"            /* Ѓу7ЃЂу3Ѓ…У3Б‡џ2б */
	$"878F 3271 8E00 0039 8400 0011 8000 0001"            /* ‡Џ2qЋ..9„...Ђ... */
	$"7FF0 0FFE 0020 0400 0040 0200 003F FC00",           /* .р.ю. ...@...?ь. */
	
	$"7FE0 07FE FFFF C3FF FFFF C1FF FFFF FFFF"            /* .а.юяяГяяяБяяяяя */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* яяяяяяяяяяяяяяяя */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* яяяяяяяяяяяяяяяя */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* яяяяяяяяяяяяяяяя */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* яяяяяяяяяяяяяяяя */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* яяяяяяяяяяяяяяяя */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* яяяяяяяяяяяяяяяя */
	$"7FFF FFFE 003F FC00 007F FE00 003F FC00"            /* .яяю.?ь...ю..?ь. */
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
		$"FC3F 8FF1 8001 8001 8001 8091 80A1 B1B5"
		$"8AA5 B2A1 8AA9 B1A9 8001 8001 FC3F 0FF0",
		/* [2] */
		$"FC3F FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0FF0"
	}
};


