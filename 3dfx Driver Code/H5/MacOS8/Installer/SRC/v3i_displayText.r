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


#include "InstallerTypes.r"
#include "Types.r"
#include "MacTypes.r"
#include "v3_installer.h"


// The ID indicating the DisplayTEXT text should not be displayed
#define doNotDisplay	-1

//These region codes are defined in Script.h
#define		verUS			0
#define		verFrance		1
#define		verBritain		2
#define		verGermany		3
#define		verItaly		4
#define		verNetherlands	5
#define		verFrBelgiumLux	6
#define		verSweden		7
#define		verSpain		8
#define		verDenmark		9
#define		verPortugal		10
#define		verFrCanada		11
#define		verNorway		12

#define		verJapan		14
#define		verFinland		17
#define		verFrSwiss		18
#define		verGrSwiss		19
#define		verTurkey		24

/* Should be in Script.h but aren't yet, as of ETO 21 */
#define		verCatalan		73
#define		verFrUniversal	91


/* Resource type definition used in the DisplayTEXT Startup function. */
type 'LPic'
{
		integer;								/* Default language ID */
		integer = $$Countof(LanguageArray);
		array LanguageArray
		{
				integer;						/* System language ID found in Script.h */
				integer;						/* unique local Res ID, offset from 5000 (kSetupCodeID) */
				integer 	oneByte, twoByte;	/* Is this language a 1 or 2 byte language */
		};
};


resource 'LPic' ( kSetupCodeID )
{
	1,					//	Default Language ID
	{					//	language, local resource ID offset from 5000 same as MENU ID, one or two byte language
		verUS,				1,				oneByte
	}
};

//		•••	Multilingual Support Details •••		//
//	If you would like to coerce one script language to another, for instance SwissFrench to French in the below
//	example, just add another line : "langSwisFrench, 4, oneByte,".  This will cause a SwissFrench OS to display
//	the text in language "4", French.
//	If the dialog is not to be diplayed at all for a particular language, say langArabic, add a line:
//	"langArabic, doNotDisplay, oneByte,".  The "doNotDisplay" flag will coerce the script to go directly to
//	the software installation screen.
//resource 'LPic' ( 5000 )
//{
//	2,					//	Default Language ID
//	{					//	language, local resource ID offset from 5000 same as MENU ID, one or two byte language
//		langFrench,			4,				oneByte,
//		langSwissFrench,	4,				oneByte,
//		langArabic,			doNotDisplay,	oneByte,
//	}
//};




resource 'DITL' (5000, "Legal DITL") {
	{	/* array DITLarray: 8 elements */
		/* [1] */
		{282, 388, 302, 462},
		Button {
			enabled,
			"Agree"
		},
		/* [2] */
		{282, 302, 302, 376},
		Button {
			enabled,
			"Disagree"
		},
		/* [3] */
		{282, 215, 302, 289},
		Button {
			enabled,
			"Print"
		},
		/* [4] */
		{282, 129, 302, 203},
		Button {
			enabled,
			"Save..."
		},
		/* [5] */
		{284, 11, 300, 117},
		UserItem {
			enabled
		},
		/* [6] */
		{36, 11, 278, 447},
		UserItem {
			disabled
		},
		/* [7] */
		{36, 446, 278, 462},
		Control {
			enabled,
			5000
		},
		/* [8] */
		{-1, 11, 35, 463},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (5001) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{72, 296, 92, 376},
		Button {
			enabled,
			"Cancel"
		},
		/* [2] */
		{12, 76, 60, 376},
		StaticText {
			disabled,
			"^0"
		},
		/* [3] */
		{12, 22, 44, 54},
		Icon {
			disabled,
			0
		}
	}
};

resource 'CNTL' (5000) {
	{0, 320, 242, 336},
	0,
	visible,
	128,
	0,
	scrollBarProc,
	0,
	""
};

resource 'CNTL' (5001) {
	{0, 0, 20, 150},
	0,
	visible,
	0,
	5001,
	1008,
	0,
	""
};

resource 'STR#' (5001, "English") {
	{	/* array StringArray: 9 elements */
		/* [1] */
		"English",
		/* [2] */
		"Agree",
		/* [3] */
		"Disagree",
		/* [4] */
		"Print",
		/* [5] */
		"Save...",
		/* [6] */
		"If you agree with the terms of this license, "
		"press \"Agree\" to install the software.  "
		"If you do not agree, press \"Disagree\".",
		/* [7] */
		"Software License Agreement",
		/* [8] */
		"This text cannot be saved. This disk may be "
		"full or locked, or the file may be locked.",
		/* [9] */
		"Unable to print. Make sure you’ve selected a printer."
	}
};







data 'infn' (kSetupCodeID, purgeable) {
	$"4E56 FFDC 48E7 1C38 266E 0008 554F 2F3C"            /* NVˇ‹HÁ.8&n..UO/< */
	$"7379 7376 486E FFE0 4EBA 1BE0 544F 0CAE"            /* sysvHnˇ‡N∫.‡TO.Æ */
	$"0000 0700 FFE0 6C06 7000 4EFA 01C2 594F"            /* ....ˇ‡l.p.N˙.¬YO */
	$"3F3C 1388 42A7 70FF 2F00 A97C 201F 2840"            /* ?<.àBßpˇ/.©| .(@ */
	$"4A80 6606 7000 4EFA 01A6 4EBA 01B8 2D40"            /* JÄf.p.N˙.¶N∫.∏-@ */
	$"FFE8 7012 A31E 2D48 FFEC 2F08 4EBA 01B0"            /* ˇËp.£.-HˇÏ/.N∫.∞ */
	$"584F 2F0C 7004 3F00 486E FFF0 486E FFF4"            /* XO/.p.?.HnˇHnˇÙ */
	$"486E FFF8 A98D 302E FFFE 906E FFFA 206E"            /* Hnˇ¯©ç0.ˇ˛ênˇ˙ n */
	$"FFEC 3140 0006 224C 3029 0016 48C0 322E"            /* ˇÏ1@.."L0)..H¿2. */
	$"FFFA 48C1 9081 6A02 5680 E480 5540 3140"            /* ˇ˙H¡êÅj.VÄ‰ÄU@1@ */
	$"0008 594F 2F3C 4C50 6963 3F3C 1388 A9A0"            /* ..YO/<LPic?<.à©† */
	$"201F 206E FFEC 2140 000E 554F A9AF 301F"            /*  . nˇÏ!@..UO©Ø0. */
	$"6704 6000 010C 246E FFEC 2F2A 000E A992"            /* g.`...$nˇÏ/*..©í */
	$"4240 3540 0004 2F0C 4EBA 0712 584F 3012"            /* B@5@../.N∫..XO0. */
	$"3540 0002 3012 72FF B041 660A 7001 3D40"            /* 5@..0.rˇ∞Af.p.=@ */
	$"FFDC 6000 00CE 206E FFEC 3F10 2F0C 4EBA"            /* ˇ‹`..Œ nˇÏ?./.N∫ */
	$"01AE 5C4F 2F0C A915 487A 0A50 486E FFDC"            /* .Æ\O/.©.Hz.PHnˇ‹ */
	$"A991 206E FFEC 3028 0004 7202 B041 6778"            /* ©ë nˇÏ0(..r.∞Agx */
	$"302E FFDC 48C0 7203 B081 670E 7204 B081"            /* 0.ˇ‹H¿r.∞Åg.r.∞Å */
	$"674A 7205 B081 6760 605E 594F 2F0C A917"            /* gJr.∞Åg``^YO/.©. */
	$"201F 2D40 FFE4 2040 A064 A029 703A 3F00"            /*  .-@ˇ‰ @†d†)p:?. */
	$"4EBA 05FC 544F 1F00 2F2E FFE4 4EBA 1348"            /* N∫.¸TO../.ˇ‰N∫.H */
	$"5C4F 3A00 206E FFE4 A02A 3005 672A 7009"            /* \O:. nˇ‰†*0.g*p∆ */
	$"3F00 3F05 4EBA 19CE 584F 601C 4EBA 17E4"            /* ?.?.N∫.ŒXO`.N∫.‰ */
	$"3A00 4A40 6712 7202 B041 670C 7208 3F01"            /* :.J@g.r.∞Ag.r.?. */
	$"3F00 4EBA 19B0 584F 362E FFDC 7001 B640"            /* ?.N∫.∞XO6.ˇ‹p.∂@ */
	$"6706 7002 B640 660E 206E FFEC 3028 0004"            /* g.p.∂@f. nˇÏ0(.. */
	$"7202 B041 6604 6000 FF50 2F0C 4EBA 05DA"            /* r.∞Af.`.ˇP/.N∫.⁄ */
	$"584F 2F0C A983 206E FFEC 2068 000E A023"            /* XO/.©É nˇÏ h..†# */
	$"206E FFEC A01F 2F2E FFE8 4EBA 0032 584F"            /*  nˇÏ†./.ˇËN∫.2XO */
	$"0C6E 0001 FFDC 6604 7000 6002 70FF 4CDF"            /* .n..ˇ‹f.p.`.pˇLﬂ */
	$"1C38 4E5E 4E75 8A4C 6567 616C 697A 6549"            /* .8N^NuäLegalizeI */
	$"7400 0000 207C 0000 0A78 2010 4E75 4E56"            /* t... |...x .NuNV */
	$"0000 48E7 1030 246E 0008 263C 0000 0A78"            /* ..HÁ.0$n..&<...x */
	$"2643 2043 208A 227C 0000 0A78 700C A22E"            /* &C C ä"|...xp.¢. */
	$"4CDF 0C08 4E5E 4E75 8C53 6574 476C 6F62"            /* Lﬂ..N^NuåSetGlob */
	$"616C 5074 7200 0000 48E7 1820 554F A994"            /* alPtr...HÁ. UO©î */
	$"301F 3600 4267 A998 594F 2F3C 7665 7273"            /* 0.6.Bg©òYO/<vers */
	$"7001 3F00 A81F 201F 2440 2040 2250 3229"            /* p.?.®. .$@ @"P2) */
	$"0004 3801 2F00 A9A3 3F03 A998 0C44 005B"            /* ..8./.©£?.©ò.D.[ */
	$"6624 594F 70FF 3F00 7010 3F00 2F3C 8404"            /* f$YOpˇ?.p.?./<Ñ. */
	$"000C A8B5 201F 720B B081 6606 3001 3800"            /* ..®µ .r.∞Åf.0.8. */
	$"6004 7001 3800 3004 4CDF 0418 4E75 4E56"            /* `.p.8.0.Lﬂ..NuNV */
	$"FFE0 48E7 1E38 266E 0008 362E 000C 3003"            /* ˇ‡HÁ.8&n..6...0. */
	$"0640 1388 3C00 594F 2F3C 5445 5854 3F00"            /* .@.à<.YO/<TEXT?. */
	$"A9A0 201F 2D40 FFF4 594F 2F3C 7374 796C"            /* ©† .-@ˇÙYO/<styl */
	$"3F06 A9A0 201F 2D40 FFF8 4EBA FF18 2D40"            /* ?.©† .-@ˇ¯N∫ˇ.-@ */
	$"FFFC 2F0B 4EBA 083E 584F 2840 4A80 6704"            /* ˇ¸/.N∫.>XO(@JÄg. */
	$"2F00 A9CD 246E FFFC 206A 000E 2050 3028"            /* /.©Õ$nˇ¸ j.. P0( */
	$"0002 7201 B041 6F18 2F2A 000A 3F12 7020"            /* ..r.∞Ao./*..?.p  */
	$"3F00 A944 2F2A 000A 3F03 7012 3F00 A944"            /* ?.©D/*..?.p.?.©D */
	$"2F0B 7006 3F00 486E FFE0 486E FFE4 486E"            /* /.p.?.Hnˇ‡Hnˇ‰Hn */
	$"FFE8 A98D 486E FFE8 7003 3F00 7003 3F00"            /* ˇË©çHnˇËp.?.p.?. */
	$"A8A9 594F 486E FFE8 486E FFE8 A83E 201F"            /* ®©YOHnˇËHnˇË®> . */
	$"2840 246E FFF4 2F12 594F 2F0A 4EBA 1A9A"            /* (@$nˇÙ/.YO/.N∫.ö */
	$"201F 2F00 2F2E FFF8 2F0C 3F3C 0007 A83D"            /*  ././.ˇ¯/.?<..®= */
	$"2F0B 7007 3F00 486E FFE0 486E FFF0 486E"            /* /.p.?.Hnˇ‡HnˇHn */
	$"FFE8 A98D 2F2E FFF0 4267 A964 204C 2050"            /* ˇË©ç/.ˇBg©d L P */
	$"3028 005E 3800 4240 3A00 302E FFEC 48C0"            /* 0(.^8.B@:.0.ˇÏH¿ */
	$"322E FFE8 48C1 9081 3205 48C1 B081 6F10"            /* 2.ˇËH¡êÅ2.H¡∞Åo. */
	$"3F04 2F0C 4EBA 0F7C 5C4F DA40 5344 60DA"            /* ?./.N∫.|\O⁄@SD`⁄ */
	$"2F2E FFF0 3004 5240 3F00 A965 2F2E FFF0"            /* /.ˇ0.R@?.©e/.ˇ */
	$"4267 A963 3F06 2F0B 4EBA 003A 5C4F 486E"            /* Bg©c?./.N∫.:\OHn */
	$"FFE8 2F0C A9D3 7001 1F00 2F0C A813 2F0B"            /* ˇË/.©”p.../.®./. */
	$"2F0C A918 200C 6704 4240 6002 70FF 4CDF"            /* /.©. .g.B@`.pˇLﬂ */
	$"1C78 4E5E 4E75 8B53 6574 4C61 6E67 7561"            /* .xN^NuãSetLangua */
	$"6765 0000 4E56 FFD0 48E7 1830 246E 0008"            /* ge..NVˇ–HÁ.0$n.. */
	$"362E 000C 4EBA FDCE 2640 3F03 4EBA 09D0"            /* 6...N∫˝Œ&@?.N∫∆– */
	$"544F 3800 2F0A 7201 3F01 486E FFF0 486E"            /* TO8./.r.?.HnˇHn */
	$"FFF4 486E FFF8 A98D 486E FFD0 3F03 7002"            /* ˇÙHnˇ¯©çHnˇ–?.p. */
	$"3F00 4EBA 1A5C 2F2E FFF4 486E FFD0 A95F"            /* ?.N∫.\/.ˇÙHnˇ–©_ */
	$"2F2E FFF4 3F04 302E FFFC 906E FFF8 3F00"            /* /.ˇÙ?.0.ˇ¸ênˇ¯?. */
	$"A95C 2F0A 7002 3F00 486E FFF0 486E FFF4"            /* ©\/.p.?.HnˇHnˇÙ */
	$"486E FFF8 A98D 486E FFD0 3F03 7003 3F00"            /* Hnˇ¯©çHnˇ–?.p.?. */
	$"4EBA 1A1E 2F2E FFF4 486E FFD0 A95F 2F2E"            /* N∫../.ˇÙHnˇ–©_/. */
	$"FFF4 3F04 302E FFFC 906E FFF8 3F00 A95C"            /* ˇÙ?.0.ˇ¸ênˇ¯?.©\ */
	$"2F0A 7003 3F00 486E FFF0 486E FFF4 486E"            /* /.p.?.HnˇHnˇÙHn */
	$"FFF8 A98D 486E FFD0 3F03 7004 3F00 4EBA"            /* ˇ¯©çHnˇ–?.p.?.N∫ */
	$"19E0 2F2E FFF4 486E FFD0 A95F 2F2E FFF4"            /* .‡/.ˇÙHnˇ–©_/.ˇÙ */
	$"3F04 302E FFFC 906E FFF8 3F00 A95C 2F0A"            /* ?.0.ˇ¸ênˇ¯?.©\/. */
	$"7004 3F00 486E FFF0 486E FFF4 486E FFF8"            /* p.?.HnˇHnˇÙHnˇ¯ */
	$"A98D 486E FFD0 3F03 7005 3F00 4EBA 19A2"            /* ©çHnˇ–?.p.?.N∫.¢ */
	$"2F2E FFF4 486E FFD0 A95F 2F2E FFF4 3F04"            /* /.ˇÙHnˇ–©_/.ˇÙ?. */
	$"302E FFFC 906E FFF8 3F00 A95C 4CDF 0C18"            /* 0.ˇ¸ênˇ¯?.©\Lﬂ.. */
	$"4E5E 4E75 8B44 7261 7742 7574 746F 6E73"            /* N^NuãDrawButtons */
	$"0000 4E56 FEE0 48E7 1830 246E 0008 382E"            /* ..NV˛‡HÁ.0$n..8. */
	$"000C 4EBA FCA0 2640 3D7C D800 FFF8 363C"            /* ..N∫¸†&@=|ÿ.ˇ¯6< */
	$"6000 3D43 FFFA 3D43 FFFC 2F0A 7008 3F00"            /* `.=Cˇ˙=Cˇ¸/.p.?. */
	$"486E FEE0 486E FEE4 486E FEE8 A98D 486E"            /* Hn˛‡Hn˛‰Hn˛Ë©çHn */
	$"FEF0 3F04 7006 3F00 4EBA 1926 3F04 4EBA"            /* ˛?.p.?.N∫.&?.N∫ */
	$"160C 544F 3F00 A887 700C 3F00 A88A 486E"            /* ..TO?.®áp.?.®äHn */
	$"FFF0 AA19 486E FFF8 AA14 486E FEF1 122E"            /* ˇ™.Hnˇ¯™.Hn˛Ò.. */
	$"FEF0 7000 1001 2F00 486E FEE8 70FE 3F00"            /* ˛p.../.Hn˛Ëp˛?. */
	$"A9CE 486E FFF0 AA14 4267 A887 4267 A88A"            /* ©ŒHnˇ™.Bg®áBg®ä */
	$"4CDF 0C18 4E5E 4E75 9044 7261 7749 6E73"            /* Lﬂ..N^NuêDrawIns */
	$"7472 7563 7469 6F6E 7300 0000 4E56 FFC8"            /* tructions...NVˇ» */
	$"48E7 1E30 382E 0008 246E 000A 4EBA FBF6"            /* HÁ.08...$n..N∫˚ˆ */
	$"2640 2F0A 7205 3F01 486E FFE8 486E FFFC"            /* &@/.r.?.HnˇËHnˇ¸ */
	$"486E FFEC A98D 204B 2F28 000A 3F10 486E"            /* HnˇÏ©ç K/(..?.Hn */
	$"FFC8 A946 486E FFEC 70FF 3F00 70FF 3F00"            /* ˇ»©FHnˇÏpˇ?.pˇ?. */
	$"A8A9 302E FFF2 906E FFEE 72E9 D041 3A00"            /* ®©0.ˇÚênˇÓrÈ–A:. */
	$"554F 486E FFC8 A88C 301F 3C00 B045 6F52"            /* UOHnˇ»®å0.<.∞EoR */
	$"554F 70C9 3F00 A88D 301F 9A40 554F 142E"            /* UOp…?.®ç0.ö@UO.. */
	$"FFC8 532E FFC8 7200 1202 41EE FFC8 1230"            /* ˇ»S.ˇ»r...AÓˇ».0 */
	$"1000 7000 1001 3F00 A88D 301F 9C40 3006"            /* ..p...?.®ç0.ú@0. */
	$"B045 6D08 102E FFC8 6702 60D0 522E FFC8"            /* ∞Em...ˇ»g.`–R.ˇ» */
	$"122E FFC8 7000 1001 41EE FFC8 11BC 00C9"            /* ..ˇ»p...AÓˇ».º.… */
	$"0000 486E FFF4 362E FFF2 70EE D640 3F03"            /* ..HnˇÙ6.ˇÚpÓ÷@?. */
	$"362E FFEC 5C43 3F03 302E FFF2 5540 3F00"            /* 6.ˇÏ\C?.0.ˇÚU@?. */
	$"302E FFEC 7216 D041 3F00 A8A7 554F 486E"            /* 0.ˇÏr.–A?.®ßUOHn */
	$"FFF4 4267 4267 3F3C 1388 303C 0500 ABC9"            /* ˇÙBgBg?<.à0<..´… */
	$"544F 486E FFEC A8A1 3F2E FFF2 302E FFEC"            /* TOHnˇÏ®°?.ˇÚ0.ˇÏ */
	$"5440 3F00 A893 3F2E FFF2 3F2E FFF0 A891"            /* T@?.®ì?.ˇÚ?.ˇ®ë */
	$"302E FFEE 5440 3F00 3F2E FFF0 A891 302E"            /* 0.ˇÓT@?.?.ˇ®ë0. */
	$"FFEE 5E40 3F00 302E FFF0 5B40 3F00 A893"            /* ˇÓ^@?.0.ˇ[@?.®ì */
	$"486E FFC8 A884 4CDF 0C78 4E5E 205F 5C4F"            /* Hnˇ»®ÑLﬂ.xN^ _\O */
	$"4ED0 8944 5241 5750 4F50 5550 0000 4E56"            /* N–âDRAWPOPUP..NV */
	$"FFEC 48E7 1C00 3A2E 0008 486E FFEC A976"            /* ˇÏHÁ..:...HnˇÏ©v */
	$"3805 7600 3604 2D43 FFFC EA83 E583 41EE"            /* 8.v.6.-Cˇ¸ÍÉÂÉAÓ */
	$"FFEC 2030 3000 222E FFFC 741F C282 E2A8"            /* ˇÏ 00.".ˇ¸t.¬Ç‚® */
	$"7201 C041 4CDF 0038 4E5E 4E75 8949 7350"            /* r.¿ALﬂ.8N^NuâIsP */
	$"7265 7373 6564 0000 4E56 0000 48E7 0038"            /* ressed..NV..HÁ.8 */
	$"246E 0008 4EBA FA5E 2840 2040 2268 000E"            /* $n..N∫˙^(@ @"h.. */
	$"2251 3229 0002 7401 B242 6F06 2F28 000A"            /* "Q2)..t.≤Bo./(.. */
	$"A932 2F0A 4EBA 036E 584F 2640 2F00 A9CD"            /* ©2/.N∫.nXO&@/.©Õ */
	$"4CDF 1C00 4E5E 4E75 9150 7265 7061 7265"            /* Lﬂ..N^NuëPrepare */
	$"4672 6565 4469 616C 6F67 0000 4E56 FFE0"            /* FreeDialog..NVˇ‡ */
	$"48E7 1030 246E 0008 4EBA FA0A 2640 2F0A"            /* HÁ.0$n..N∫˙.&@/. */
	$"A873 2F0A 42A7 A918 A912 A850 4EBA FA3A"            /* ®s/.Bß©.©.®PN∫˙: */
	$"3F00 2F0A 4EBA 003E 5C4F 204B 3610 70FF"            /* ?./.N∫.>\O K6.pˇ */
	$"B640 671A 486E FFE0 0643 1388 3F03 7007"            /* ∂@g.Hnˇ‡.C.à?.p. */
	$"3F00 4EBA 168C 2F0A 486E FFE0 A91A 4CDF"            /* ?.N∫.å/.Hnˇ‡©.Lﬂ */
	$"0C08 4E5E 4E75 8B53 6574 7570 4469 616C"            /* ..N^NuãSetupDial */
	$"6F67 0000 4E56 FFCC 48E7 1F38 266E 0008"            /* og..NVˇÃHÁ.8&n.. */
	$"382E 000C 4EBA F99E 2840 2040 2268 000E"            /* 8...N∫˘û(@ @"h.. */
	$"2251 3211 3081 2268 000E 2251 3229 0002"            /* "Q2.0Å"h.."Q2).. */
	$"3A01 7401 B242 6F00 0280 2F0B 7205 3F01"            /* :.t.≤Bo..Ä/.r.?. */
	$"486E FFCC 486E FFD0 486E FFD4 A98D 2F0B"            /* HnˇÃHnˇ–Hnˇ‘©ç/. */
	$"7005 3F00 3F2E FFCC 487A FD52 486E FFD4"            /* p.?.?.ˇÃHz˝RHnˇ‘ */
	$"A98E 594F 3F3C 1389 487A 0270 A931 201F"            /* ©éYO?<.âHz.p©1 . */
	$"204C 2140 000A 7001 3C00 3606 B645 6E42"            /*  L!@..p.<.6.∂EnB */
	$"486E FFDC 0643 1388 3F03 7001 3F00 4EBA"            /* Hnˇ‹.C.à?.p.?.N∫ */
	$"15E0 102E FFDC 6726 244C 2F2A 000A 487A"            /* .‡..ˇ‹g&$L/*..Hz */
	$"0240 3F3C 00FF A826 2F2A 000A 554F 2F2A"            /* .@?<.ˇ®&/*..UO/* */
	$"000A A950 301F 3F00 486E FFDC A947 5246"            /* ..©P0.?.Hnˇ‹©GRF */
	$"60B8 204C 2068 000A A029 594F 7012 3F00"            /* `∏ L h..†)YOp.?. */
	$"2F3C 8402 0008 A8B5 201F 3E00 4240 3C00"            /* /<Ñ...®µ .>.B@<. */
	$"3606 B645 6C00 01D2 3003 48C0 2200 C0FC"            /* 6.∂El..“0.H¿".¿¸ */
	$"0006 4841 C2FC 0006 4841 4241 D081 204C"            /* ..HA¬¸..HABA–Å L */
	$"2068 000E 2050 5848 D088 2040 3010 48C0"            /*  h.. PXH–à @0.H¿ */
	$"720E B081 6750 7235 B081 6760 7233 B081"            /* r.∞ÅgPr5∞Åg`r3∞Å */
	$"6770 7210 B081 6700 0080 7214 B081 6700"            /* gpr.∞Åg..Är.∞Åg. */
	$"008C 7236 B081 6700 0098 7234 B081 6700"            /* .år6∞Åg..òr4∞Åg. */
	$"00A4 7231 B081 6700 00B0 723E B081 6700"            /* .§r1∞Åg..∞r>∞Åg. */
	$"00A8 7248 B081 6700 00A0 720D B081 6700"            /* .®rH∞Åg..†r¬∞Åg. */
	$"00AC 6000 00BC 0C47 0001 6704 7000 6002"            /* .¨`..º.G..g.p.`. */
	$"7001 2000 1D40 FFDC 6000 00AC 0C47 0002"            /* p. ..@ˇ‹`..¨.G.. */
	$"6704 7000 6002 7001 2000 1D40 FFDC 6000"            /* g.p.`.p. ..@ˇ‹`. */
	$"0096 0C47 0003 6704 7000 6002 7001 2000"            /* .ñ.G..g.p.`.p. . */
	$"1D40 FFDC 6000 0080 0C47 0004 6704 7000"            /* .@ˇ‹`..Ä.G..g.p. */
	$"6002 7001 2000 1D40 FFDC 606A 0C47 0006"            /* `.p. ..@ˇ‹`j.G.. */
	$"6704 7000 6002 7001 2000 1D40 FFDC 6056"            /* g.p.`.p. ..@ˇ‹`V */
	$"0C47 0015 6704 7000 6002 7001 2000 1D40"            /* .G..g.p.`.p. ..@ */
	$"FFDC 6042 0C47 0019 6704 7000 6002 7001"            /* ˇ‹`B.G..g.p.`.p. */
	$"2000 1D40 FFDC 602E 0C47 0007 6704 7000"            /*  ..@ˇ‹`..G..g.p. */
	$"6002 7001 2000 1D40 FFDC 601A 0C47 0005"            /* `.p. ..@ˇ‹`..G.. */
	$"6704 7000 6002 7001 2000 1D40 FFDC 6006"            /* g.p.`.p. ..@ˇ‹`. */
	$"1D7C 0001 FFDC 102E FFDC 6758 3606 48C3"            /* .|..ˇ‹..ˇ‹gX6.H√ */
	$"2D43 FFFC 2003 C6FC 0006 4840 C0FC 0006"            /* -Cˇ¸ .∆¸..H@¿¸.. */
	$"4840 4240 D680 244C 206A 000E 2050 5848"            /* H@B@÷Ä$L j.. PXH */
	$"D688 2043 3010 B044 6658 202E FFFC 2200"            /* ÷à C0.∞DfX .ˇ¸". */
	$"C0FC 0006 4841 C2FC 0006 4841 4241 D081"            /* ¿¸..HA¬¸..HABA–Å */
	$"206A 000E 2050 5848 D088 2040 3028 0002"            /*  j.. PXH–à @0(.. */
	$"3480 602E 244C 2F2A 000A 3006 48C0 2200"            /* 4Ä`.$L/*..0.H¿". */
	$"C0FC 0006 4841 C2FC 0006 4841 4241 D081"            /* ¿¸..HA¬¸..HABA–Å */
	$"206A 000E 2050 5848 D088 2040 3F28 0002"            /*  j.. PXH–à @?(.. */
	$"A93A 5246 6000 FE2A 4CDF 1CF8 4E5E 4E75"            /* ©:RF`.˛*Lﬂ.¯N^Nu */
	$"9653 6574 7570 4C61 6E67 7561 6765 506F"            /* ñSetupLanguagePo */
	$"7075 704D 656E 7500 000A 0344 5348 0000"            /* pupMenu....DSH.. */
	$"0120 0000 4E56 0000 2F0A 246E 0008 594F"            /* . ..NV../.$n..YO */
	$"2F0A A917 201F 245F 4E5E 4E75 8B47 6574"            /* /.©. .$_N^NuãGet */
	$"5445 4861 6E64 6C65 0000 4E56 0000 48E7"            /* TEHandle..NV..HÁ */
	$"0038 266E 0008 286E 000C 204C 3210 7000"            /* .8&n..(n.. L2.p. */
	$"3001 7203 B081 6724 7205 B081 671E 7204"            /* 0.r.∞Åg$r.∞Åg.r. */
	$"B081 6730 4A80 6734 7206 B081 6740 7208"            /* ∞Åg0JÄg4r.∞Åg@r. */
	$"B081 674C 7201 B081 6758 6072 2F0B 2F0C"            /* ∞ÅgLr.∞ÅgX`r/./. */
	$"2F2E 0010 4EBA 0104 4FEF 000C 7001 1D40"            /* /...N∫..OÔ..p..@ */
	$"0014 6060 4200 1D40 0014 6058 2F2E 0010"            /* ..``B..@..`X/... */
	$"4EBA 031E 584F 4200 1D40 0014 6046 2F2E"            /* N∫..XOB..@..`F/. */
	$"0010 4EBA 0060 584F 4200 1D40 0014 6034"            /* ..N∫.`XOB..@..`4 */
	$"2F2E 0010 4EBA 03B2 584F 4200 1D40 0014"            /* /...N∫.≤XOB..@.. */
	$"6022 244C 3F2A 000E 2F2A 000A 2F2E 0010"            /* `"$L?*...*...... */
	$"4EBA 03E8 4FEF 000A 1D40 0014 6006 4200"            /* N∫.ËOÔ...@..`.B. */
	$"1D40 0014 4CDF 1C00 4E5E 205F 4FEF 000C"            /* .@..Lﬂ..N^ _OÔ.. */
	$"4ED0 8E4D 5944 4941 4C4F 4746 494C 5445"            /* N–éMYDIALOGFILTE */
	$"5200 0000 4E56 FFF0 48E7 0038 246E 0008"            /* R...NVˇHÁ.8$n.. */
	$"4EBA F5C2 2840 2F0A 7206 3F01 486E FFF0"            /* N∫ı¬(@/.r.?.Hnˇ */
	$"486E FFF4 486E FFF8 A98D 486E FFF8 A8A1"            /* HnˇÙHnˇ¯©çHnˇ¯®° */
	$"486E FFF8 7003 3F00 7003 3F00 A8A9 2F0A"            /* Hnˇ¯p.?.p.?.®©/. */
	$"4EBA FEC2 584F 2640 486E FFF8 2F00 A9D3"            /* N∫˛¬XO&@Hnˇ¯/.©” */
	$"204C 3010 0640 1388 3F00 2F0A 4EBA F8C4"            /*  L0..@.à?./.N∫¯ƒ */
	$"5C4F 4CDF 1C00 4E5E 4E75 8C48 616E 646C"            /* \OLﬂ..N^NuåHandl */
	$"6555 7064 6174 6500 0000 4E56 FFFC 48E7"            /* eUpdate...NVˇ¸HÁ */
	$"1038 246E 0008 266E 000C 286E 0010 204B"            /* .8$n..&n..(n.. K */
	$"2028 0002 0200 00FF 1D40 FFFC 162E FFFC"            /*  (.....ˇ.@ˇ¸..ˇ¸ */
	$"700D B600 6706 7003 B600 6604 6000 0084"            /* p¬∂.g.p.∂.f.`..Ñ */
	$"204B 3228 000E 7000 3001 0280 0000 0100"            /*  K2(..p.0..Ä.... */
	$"6770 162E FFFC 7071 B600 670C 7051 B600"            /* gp..ˇ¸pq∂.g.pQ∂. */
	$"6706 702E B600 6614 7002 3F00 2F0A 4EBA"            /* g.p.∂.f.p.?./.N∫ */
	$"0074 5C4F 7002 204C 3080 6046 162E FFFC"            /* .t\Op. L0Ä`F..ˇ¸ */
	$"7070 B600 6706 7050 B600 6614 7003 3F00"            /* pp∂.g.pP∂.f.p.?. */
	$"2F0A 4EBA 0050 5C4F 7003 204C 3080 6022"            /* /.N∫.P\Op. L0Ä`" */
	$"162E FFFC 7073 B600 6706 7053 B600 6612"            /* ..ˇ¸ps∂.g.pS∂.f. */
	$"7004 3F00 2F0A 4EBA 002C 5C4F 7004 204C"            /* p.?./.N∫.,\Op. L */
	$"3080 2F0A 4EBA 0618 584F 4CDF 1C08 4E5E"            /* 0Ä/.N∫..XOLﬂ..N^ */
	$"4E75 8E48 616E 646C 654B 6579 5072 6573"            /* NuéHandleKeyPres */
	$"7300 0000 4E56 FFFC 48E7 1030 246E 0008"            /* s...NVˇ¸HÁ.0$n.. */
	$"362E 000C 3F03 2F0A 4EBA 0036 5C4F 2640"            /* 6...?./.N∫.6\O&@ */
	$"2F00 720A 3F01 A95D 7008 2040 43EE FFFC"            /* /.r.?.©]p. @CÓˇ¸ */
	$"A03B 2280 2F0B 4267 A95D 4CDF 0C08 4E5E"            /* †;"Ä/.Bg©]Lﬂ..N^ */
	$"4E75 8B43 6C69 636B 4275 7474 6F6E 0000"            /* NuãClickButton.. */
	$"4E56 FFF0 48E7 1020 246E 0008 362E 000C"            /* NVˇHÁ. $n..6... */
	$"2F0A 3F03 486E FFF0 486E FFFC 486E FFF4"            /* /.?.HnˇHnˇ¸HnˇÙ */
	$"A98D 202E FFFC 4CDF 0408 4E5E 4E75 8C53"            /* ©ç .ˇ¸Lﬂ..N^NuåS */
	$"6E61 7463 6848 616E 646C 6500 0000 4E56"            /* natchHandle...NV */
	$"FFE0 48E7 1C30 362E 0008 4EBA F3E8 2640"            /* ˇ‡HÁ.06...N∫ÛË&@ */
	$"2040 3028 0006 5D40 3A00 486E FFE0 3F03"            /*  @0(..]@:.Hnˇ‡?. */
	$"7202 3F01 4EBA 108A 554F 486E FFE0 A88C"            /* r.?.N∫.äUOHnˇ‡®å */
	$"301F 3800 B045 6F02 3A00 486E FFE0 3F03"            /* 0.8.∞Eo.:.Hnˇ‡?. */
	$"7003 3F00 4EBA 106A 554F 486E FFE0 A88C"            /* p.?.N∫.jUOHnˇ‡®å */
	$"301F 3800 B045 6F02 3A00 486E FFE0 3F03"            /* 0.8.∞Eo.:.Hnˇ‡?. */
	$"7004 3F00 4EBA 104A 554F 486E FFE0 A88C"            /* p.?.N∫.JUOHnˇ‡®å */
	$"301F 3800 B045 6F02 3A00 486E FFE0 3F03"            /* 0.8.∞Eo.:.Hnˇ‡?. */
	$"7005 3F00 4EBA 102A 554F 486E FFE0 A88C"            /* p.?.N∫.*UOHnˇ‡®å */
	$"301F 3800 B045 6F02 3A00 3005 48C0 5C80"            /* 0.8.∞Eo.:.0.H¿\Ä */
	$"244B 322A 0008 48C1 B081 6F06 302A 0008"            /* $K2*..H¡∞Åo.0*.. */
	$"6004 3005 5C40 4CDF 0C38 4E5E 4E75 8F43"            /* `.0.\@Lﬂ.8N^NuèC */
	$"616C 6342 7574 746F 6E57 6964 7468 0000"            /* alcButtonWidth.. */
	$"4E56 FFD8 48E7 0038 266E 0008 4EBA F316"            /* NVˇÿHÁ.8&n..N∫Û. */
	$"2D40 FFFC 594F 3F3C 1388 A9BA 201F 2D40"            /* -@ˇ¸YO?<.à©∫ .-@ */
	$"FFF8 4A80 677C 2040 2250 4A11 6774 723A"            /* ˇ¯JÄg| @"PJ.gtr: */
	$"3F01 4EBA F83A 544F 4A40 6746 206E FFFC"            /* ?.N∫¯:TOJ@gF nˇ¸ */
	$"3028 0004 7201 B041 6758 3001 3140 0004"            /* 0(..r.∞AgX0.1@.. */
	$"7003 3F00 2F0B 4EBA FE98 5C4F 2840 2F00"            /* p.?./.N∫˛ò\O(@/. */
	$"486E FFD8 A95E 594F 486E FFD8 206E FFF8"            /* Hnˇÿ©^YOHnˇÿ nˇ¯ */
	$"2F10 4EBA 10BC 584F 2F0C 486E FFD8 A95F"            /* /.N∫.ºXO/.Hnˇÿ©_ */
	$"6020 246E FFFC 4A6A 0004 6716 4240 3540"            /* ` $nˇ¸Jj..g.B@5@ */
	$"0004 3012 0640 1388 3F00 2F0B 4EBA F4A6"            /* ..0..@.à?./.N∫Ù¶ */
	$"5C4F 4CDF 1C00 4E5E 4E75 8A48 616E 646C"            /* \OLﬂ..N^NuäHandl */
	$"6549 646C 6500 0000 4E56 FFF0 48E7 0030"            /* eIdle...NVˇHÁ.0 */
	$"246E 0008 4EBA F25E 2640 2040 3F10 2F0A"            /* $n..N∫Ú^&@ @?./. */
	$"4EBA F2FC 5C4F 2F0A 7006 3F00 486E FFF0"            /* N∫Ú¸\O/.p.?.Hnˇ */
	$"486E FFF4 486E FFF8 A98D 486E FFF8 A8A1"            /* HnˇÙHnˇ¯©çHnˇ¯®° */
	$"4CDF 0C00 4E5E 4E75 8E48 616E 646C 6541"            /* Lﬂ..N^NuéHandleA */
	$"6374 6976 6174 6500 0000 4E56 FFE4 48E7"            /* ctivate...NVˇ‰HÁ */
	$"1838 4EBA F210 2840 486E FFF4 A874 2F2E"            /* .8N∫Ú.(@HnˇÙ®t/. */
	$"0008 A873 1D7C 0000 FFF8 486E 000C A871"            /* ..®s.|..ˇ¯Hn..®q */
	$"554F 2F2E 0008 2F2E 000C A984 301F 48C0"            /* UO/.../...©Ñ0.H¿ */
	$"5280 7206 B081 6710 7207 B081 6720 7205"            /* RÄr.∞Åg.r.∞Åg r. */
	$"B081 6732 6000 00D2 2F2E 0008 4EBA FAF6"            /* ∞Åg2`..“/...N∫˙ˆ */
	$"584F 2640 1D7C 0001 FFF9 6000 00C0 2F2E"            /* XO&@.|..ˇ˘`..¿/. */
	$"000C 2F2E 0008 4EBA 00D4 504F 1D7C 0001"            /* ../...N∫.‘PO.|.. */
	$"FFF9 6000 00A8 244C 206A 000E 2050 3028"            /* ˇ˘`..®$L j.. P0( */
	$"0002 7201 B041 6F00 0088 2F2A 000A 70FF"            /* ..r.∞Ao..à/*..pˇ */
	$"3F00 A935 2F2E 0008 7005 3F00 486E FFE4"            /* ?.©5/...p.?.Hnˇ‰ */
	$"486E FFE8 486E FFEC A98D 3D6E FFEE FFFE"            /* HnˇËHnˇÏ©ç=nˇÓˇ˛ */
	$"3D6E FFEC FFFC 486E FFFC A870 2F2A 000A"            /* =nˇÏˇ¸Hnˇ¸®p/*.. */
	$"A948 594F 2F2A 000A 3F2E FFFC 3F2E FFFE"            /* ©HYO/*..?.ˇ¸?.ˇ˛ */
	$"3F12 A80B 201F 2600 3F3C 1389 A936 2003"            /* ?.®. .&.?<.â©6 . */
	$"672E 3003 3800 3212 B240 6724 3F00 2F2E"            /* g.0.8.2.≤@g$?./. */
	$"0008 4EBA F1CA 5C4F 4A40 6602 3484 486E"            /* ..N∫Ò \OJ@f.4ÑHn */
	$"FFEC A8A3 2F2E 0008 7005 3F00 4EBA F4FE"            /* ˇÏ®£/...p.?.N∫Ù˛ */
	$"1D7C 0001 FFF9 6004 422E FFF9 2F2E FFF4"            /* .|..ˇ˘`.B.ˇ˘/.ˇÙ */
	$"A873 102E FFF9 4CDF 1C18 4E5E 4E75 8B48"            /* ®s..ˇ˘Lﬂ..N^NuãH */
	$"616E 646C 654D 6F75 7365 0000 4E56 FFFC"            /* andleMouse..NVˇ¸ */
	$"48E7 1020 246E 0008 554F 2F2E 000C 2F0A"            /* HÁ. $n..UO/.../. */
	$"486E FFFC A96C 301F 3600 3200 48C1 7014"            /* Hnˇ¸©l0.6.2.H¡p. */
	$"B280 671C 7015 B280 6716 7016 B280 6710"            /* ≤Äg.p.≤Äg.p.≤Äg. */
	$"7017 B280 670A 0C81 0000 0081 6716 602C"            /* p.≤Äg..Å...Åg.`, */
	$"554F 2F2E FFFC 2F2E 000C 487A 003A A968"            /* UO/.ˇ¸/...Hz.:©h */
	$"544F 6018 554F 2F2E FFFC 2F2E 000C 42A7"            /* TO`.UO/.ˇ¸/...Bß */
	$"A968 544F 2F0A 4EBA 0102 584F 4CDF 0408"            /* ©hTO/.N∫..XOLﬂ.. */
	$"4E5E 4E75 8E48 616E 646C 6553 6372 6F6C"            /* N^NuéHandleScrol */
	$"6C65 7200 0000 4E56 FFFC 48E7 1F30 3A2E"            /* ler...NVˇ¸HÁ.0:. */
	$"0008 246E 000A 554F 2F0A A962 301F 3D40"            /* ..$n..UO/.©b0.=@ */
	$"FFFC 554F 2F0A A961 301F 3D40 FFFE 554F"            /* ˇ¸UO/.©a0.=@ˇ˛UO */
	$"2F0A A960 301F 3E00 204A 2050 2028 0004"            /* /.©`0.>. J P (.. */
	$"2640 3005 48C0 7214 B081 6714 7215 B081"            /* &@0.H¿r.∞Åg.r.∞Å */
	$"6714 7216 B081 6714 7217 B081 6714 6018"            /* g.r.∞Åg.r.∞Åg.`. */
	$"70FF 3C00 6014 7001 3C00 600E 70F1 3C00"            /* pˇ<.`.p.<.`.pÒ<. */
	$"6008 700F 3C00 6002 6052 3607 3003 48C0"            /* `.p.<.`.`R6.0.H¿ */
	$"3206 48C1 D081 382E FFFC 3204 48C1 B081"            /* 2.H¡–Å8.ˇ¸2.H¡∞Å */
	$"6F04 9843 3C04 3607 3003 48C0 3206 48C1"            /* o.òC<.6.0.H¿2.H¡ */
	$"D081 382E FFFE 3204 48C1 B081 6C04 9843"            /* –Å8.ˇ˛2.H¡∞Ål.òC */
	$"3C04 3006 6716 3F07 3F06 2F0B 4EBA 0192"            /* <.0.g.?.?./.N∫.í */
	$"504F 2F0A 3007 D046 3F00 A963 4CDF 0CF8"            /* PO/.0.–F?.©cLﬂ.¯ */
	$"4E5E 205F 5C4F 4ED0 8F53 4352 4F4C 4C42"            /* N^ _\ON–èSCROLLB */
	$"4152 4143 5449 4F4E 0000 4E56 FFFC 48E7"            /* ARACTION..NVˇ¸HÁ */
	$"1E38 266E 0008 2F0B 4EBA F88A 584F 2840"            /* .8&n../.N∫¯äXO(@ */
	$"7207 3F01 2F0B 4EBA FB18 5C4F 2D40 FFFC"            /* r.?./.N∫˚.\O-@ˇ¸ */
	$"554F 2F00 A960 301F 3600 204C 2450 302A"            /* UO/.©`0.6. L$P0* */
	$"0008 48C0 3212 48C1 9081 2440 554F 2F2E"            /* ..H¿2.H¡êÅ$@UO/. */
	$"FFFC A960 301F 3F00 2F0C 4EBA 0056 5C4F"            /* ˇ¸©`0.?./.N∫.V\O */
	$"3200 48C1 200A 4EBA 0C16 3800 9043 3A00"            /* 2.H¡ .N∫..8.êC:. */
	$"554F 2F2E FFFC A960 301F 3F00 2F0C 4EBA"            /* UO/.ˇ¸©`0.?./.N∫ */
	$"0032 5C4F C1C5 3C00 4267 3F00 2F0C A9DD"            /* .2\O¡≈<.Bg?./.©› */
	$"4CDF 1C78 4E5E 4E75 9652 6541 6C69 676E"            /* Lﬂ.xN^NuñReAlign */
	$"5465 7874 546F 5363 726F 6C6C 6261 7200"            /* TextToScrollbar. */
	$"0000 4E56 0000 48E7 1820 246E 0008 382E"            /* ..NV..HÁ. $n..8. */
	$"000C 594F 3604 48C3 2F03 2F03 2F0A 3F3C"            /* ..YO6.H√/././.?< */
	$"0009 A83D 201F 4CDF 0418 4E5E 4E75 8D47"            /* .∆®= .Lﬂ..N^NuçG */
	$"6574 4C69 6E65 4865 6967 6874 0000 4E56"            /* etLineHeight..NV */
	$"FFFC 48E7 1038 266E 0008 2F0B 4EBA F7A6"            /* ˇ¸HÁ.8&n../.N∫˜¶ */
	$"584F 2840 7207 3F01 2F0B 4EBA FA34 5C4F"            /* XO(@r.?./.N∫˙4\O */
	$"2D40 FFFC 204C 2450 302A 0008 48C0 3212"            /* -@ˇ¸ L$P0*..H¿2. */
	$"48C1 9081 2440 554F 2F2E FFFC A960 301F"            /* H¡êÅ$@UO/.ˇ¸©`0. */
	$"3F00 2F0C 4EBA FF7C 5C4F 3200 48C1 200A"            /* ?./.N∫ˇ|\O2.H¡ . */
	$"4EBA 0B3C 3600 2F2E FFFC 3F00 A963 4CDF"            /* N∫.<6./.ˇ¸?.©cLﬂ */
	$"1C08 4E5E 4E75 9652 6541 6C69 676E 5363"            /* ..N^NuñReAlignSc */
	$"726F 6C6C 6261 7254 6F54 6578 7400 0000"            /* rollbarToText... */
	$"4E56 0000 48E7 1F38 266E 0008 382E 000C"            /* NV..HÁ.8&n..8... */
	$"3A2E 000E 2F0B 4EBA F71C 584F 2840 2040"            /* :.../.N∫˜.XO(@ @ */
	$"2450 302A 0008 48C0 3212 48C1 9081 2440"            /* $P0*..H¿2.H¡êÅ$@ */
	$"3F05 2F08 4EBA FF0C 5C4F 3200 48C1 200A"            /* ?./.N∫ˇ.\O2.H¡ . */
	$"4EBA 0ACC 3E00 3200 48C1 3404 48C2 D282"            /* N∫.Ã>.2.H¡4.H¬“Ç */
	$"6A04 4440 3800 3607 3003 48C0 3204 48C1"            /* j.D@8.6.0.H¿2.H¡ */
	$"D081 204C 2450 322A 005E 48C1 B081 6F08"            /* –Å L$P2*.^H¡∞Åo. */
	$"302A 005E 9043 3800 3F05 2F0C 4EBA FEC4"            /* 0*.^êC8.?./.N∫˛ƒ */
	$"5C4F C1C4 3C00 4267 4440 3F00 2F0C A9DD"            /* \O¡ƒ<.BgD@?./.©› */
	$"4CDF 1CF8 4E5E 4E75 8A53 6372 6F6C 6C54"            /* Lﬂ.¯N^NuäScrollT */
	$"6578 7400 0000 4E56 FFD8 48E7 1E38 266E"            /* ext...NVˇÿHÁ.8&n */
	$"0008 486E FFD8 A874 2F3C C800 0000 A8FD"            /* ..Hnˇÿ®t/<»...®˝ */
	$"554F 2F3C BA00 0000 A8FD 301F 3800 4A40"            /* UO/<∫...®˝0.8.J@ */
	$"6600 01AE 7078 A322 2D48 FFDC 554F 3EB8"            /* f..Æpx£"-Hˇ‹UO>∏ */
	$"0220 301F 3800 4A40 6600 018E 2F2E FFDC"            /* . 0.8.J@f..é/.ˇ‹ */
	$"2F3C 2004 0480 A8FD 206E FFDC 2450 302A"            /* /< ..Ä®˝ nˇ‹$P0* */
	$"003E 3A00 302A 0040 3C00 102E 000C 6714"            /* .>:.0*.@<.....g. */
	$"554F 2F08 2F3C 2A04 0484 A8FD 101F 1D40"            /* UO/./<*..Ñ®˝...@ */
	$"FFE0 6022 554F 2F2E FFDC 2F3C 5204 0498"            /* ˇ‡`"UO/.ˇ‹/<R..ò */
	$"A8FD 544F 7001 206E FFDC 2050 3140 0042"            /* ®˝TOp. nˇ‹ P1@.B */
	$"1D7C 0001 FFE0 102E FFE0 6700 0126 594F"            /* .|..ˇ‡..ˇ‡g..&YO */
	$"2F2E FFDC 42A7 42A7 2F3C 0400 0C00 A8FD"            /* /.ˇ‹BßBß/<....®˝ */
	$"201F 2840 554F 2F3C BA00 0000 A8FD 301F"            /*  .(@UO/<∫...®˝0. */
	$"3800 4A40 6600 00A8 2F0C A873 204B 2050"            /* 8.J@f..®/.®s K P */
	$"214C 0052 206E FFDC 2050 5048 43EE FFE8"            /* !L.R nˇ‹ PPHCÓˇË */
	$"22D8 22D8 0C45 0001 6C04 7001 3A00 3006"            /* "ÿ"ÿ.E..l.p.:.0. */
	$"3605 B043 6C02 3C03 3005 48C0 2D40 FFE4"            /* 6.∞Cl.<.0.H¿-@ˇ‰ */
	$"3006 48C0 B0AE FFE4 6D64 2F0C 42A7 2F3C"            /* 0.H¿∞Æˇ‰md/.Bß/< */
	$"1000 0808 A8FD 554F 2F3C BA00 0000 A8FD"            /* ....®˝UO/<∫...®˝ */
	$"301F 3800 4A40 6614 486E FFE4 486E FFE8"            /* 0.8.J@f.Hnˇ‰HnˇË */
	$"2F0B 4EBA 00C4 4FEF 000C 3800 2F0C 2F3C"            /* /.N∫.ƒOÔ..8././< */
	$"1800 040C A8FD 3004 660E 554F 2F3C BA00"            /* ....®˝0.f.UO/<∫. */
	$"0000 A8FD 301F 3800 3004 660A 0CAE FFFF"            /* ..®˝0.8.0.f..Æˇˇ */
	$"FFFF FFE4 6602 6006 52AE FFE4 6092 2F0C"            /* ˇˇˇ‰f.`.RÆˇ‰`í/. */
	$"2F3C 0800 0484 A8FD 3004 660E 554F 2F3C"            /* /<...Ñ®˝0.f.UO/< */
	$"BA00 0000 A8FD 301F 3800 3004 6634 206E"            /* ∫...®˝0.8.0.f4 n */
	$"FFDC 2050 1028 0044 7201 B001 6624 2F2E"            /* ˇ‹ P.(.Dr.∞.f$/. */
	$"FFDC 42A7 42A7 42A7 486E FFE4 2F3C 6005"            /* ˇ‹BßBßBßHnˇ‰/<`. */
	$"1480 A8FD 554F 2F3C BA00 0000 A8FD 301F"            /* .Ä®˝UO/<∫...®˝0. */
	$"3800 206E FFDC A023 2F3C D000 0000 A8FD"            /* 8. nˇ‹†#/<–...®˝ */
	$"204B 2050 216E FFD8 0052 2F2E FFD8 A873"            /*  K P!nˇÿ.R/.ˇÿ®s */
	$"3004 4CDF 1C78 4E5E 4E75 8A50 7269 6E74"            /* 0.Lﬂ.xN^NuäPrint */
	$"5374 7566 6600 0000 4E56 FED4 48E7 1F38"            /* Stuff...NV˛‘HÁ.8 */
	$"4240 3800 7001 3C00 4267 A887 4267 A88A"            /* B@8.p.<.Bg®áBg®ä */
	$"4267 A888 486E FEE0 A88B 302E FEE0 D06E"            /* Bg®àHn˛‡®ã0.˛‡–n */
	$"FEE2 D06E FEE6 D040 3A00 202E 0008 2D40"            /* ˛‚–n˛Ê–@:. ...-@ */
	$"FED4 2F00 A9D9 554F 486E FED4 4EBA 07BC"            /* ˛‘/.©ŸUOHn˛‘N∫.º */
	$"301F 3800 4A40 6704 6000 020C 206E 000C"            /* 0.8.J@g.`... n.. */
	$"43EE FED8 22D8 22D8 3005 916E FEDC 486E"            /* CÓ˛ÿ"ÿ"ÿ0.ën˛‹Hn */
	$"FED8 7005 3F00 7005 3F00 A8A9 41EE FED8"            /* ˛ÿp.?.p.?.®©AÓ˛ÿ */
	$"226E FED4 2251 2449 22D8 22D8 206E FED4"            /* "n˛‘"Q$I"ÿ"ÿ n˛‘ */
	$"2050 5048 20DA 20DA 2F2E FED4 A9D0 206E"            /*  PPH ⁄ ⁄/.˛‘©– n */
	$"FED4 2050 5048 43EE FEE8 22D8 22D8 4240"            /* ˛‘ PPHCÓ˛Ë"ÿ"ÿB@ */
	$"3E00 7000 2D40 FEF0 3006 48C0 206E 0010"            /* >.p.-@˛0.H¿ n.. */
	$"2210 B081 6E00 0188 7000 2D40 FEF4 594F"            /* ".∞Ån..àp.-@˛ÙYO */
	$"3607 48C3 2D43 FFFC 5283 2F03 202E FFFC"            /* 6.H√-Cˇ¸RÉ/. .ˇ¸ */
	$"5280 2F00 2F2E FED4 3F3C 0009 A83D 201F"            /* RÄ/./.˛‘?<.∆®= . */
	$"2D40 FEF8 D0AE FEF4 322E FEDC 48C1 342E"            /* -@˛¯–Æ˛Ù2.˛‹H¡4. */
	$"FED8 48C2 9282 B081 6F02 601A 5247 202E"            /* ˛ÿH¬íÇ∞Åo.`.RG . */
	$"FEF8 D1AE FEF4 206E FED4 2050 3028 005E"            /* ˛¯—Æ˛Ù n˛‘ P0(.^ */
	$"B047 6F02 60A8 3006 48C0 246E 0010 2212"            /* ∞Go.`®0.H¿$n..". */
	$"B081 6600 010C 594F A8D8 201F 2D40 FFF8"            /* ∞Åf...YO®ÿ .-@ˇ¯ */
	$"486E FEE8 4267 322E FEF2 4441 3F01 A8A8"            /* Hn˛ËBg2.˛ÚDA?.®® */
	$"302E FEE8 D06E FEF6 3D40 FEEC 41EE FEE8"            /* 0.˛Ë–n˛ˆ=@˛ÏAÓ˛Ë */
	$"226E FED4 2251 22D8 22D8 302E FED8 D06E"            /* "n˛‘"Q"ÿ"ÿ0.˛ÿ–n */
	$"FEF6 3D40 FEDC 2F2E FFF8 A87A 486E FED8"            /* ˛ˆ=@˛‹/.ˇ¯®zHn˛ÿ */
	$"A87B 486E FED8 2F2E FED4 A9D3 2F2E FFF8"            /* ®{Hn˛ÿ/.˛‘©”/.ˇ¯ */
	$"A879 2F2E FFF8 A8D9 1D7C 0002 FEF8 1D7C"            /* ®y/.ˇ¯®Ÿ.|..˛¯.| */
	$"002D FEF9 2F12 486E FEFA 4EBA 064C 102E"            /* .-˛˘/.Hn˛˙N∫.L.. */
	$"FEFA D12E FEF8 1D7C 0020 FEFA 522E FEF8"            /* ˛˙—.˛¯.|. ˛˙R.˛¯ */
	$"122E FEF8 7000 1001 41EE FEF8 11BC 0020"            /* ..˛¯p...AÓ˛¯.º.  */
	$"0000 522E FEF8 122E FEF8 7000 1001 47EE"            /* ..R.˛¯..˛¯p...GÓ */
	$"FEF8 17BC 002D 0000 286E 000C 302C 0006"            /* ˛¯.º.-..(n..0,.. */
	$"48C0 322C 0002 48C1 9081 E280 D06C 0002"            /* H¿2,..H¡êÅ‚Ä–l.. */
	$"48E7 8000 554F 486E FEF8 A88C 301F 48C0"            /* HÁÄ.UOHn˛¯®å0.H¿ */
	$"E280 3200 4CDF 0001 9041 3F00 302C 0004"            /* ‚Ä2.Lﬂ..êA?.0,.. */
	$"5B40 3F00 A893 486E FEF8 A884 206E FED4"            /* [@?.®ìHn˛¯®Ñ n˛‘ */
	$"2050 3028 005E B047 6E14 70FF 2480 600E"            /*  P0(.^∞Gn.pˇ$Ä`. */
	$"5246 202E FEF4 D1AE FEF0 6000 FE6C 206E"            /* RF .˛Ù—Æ˛`.˛l n */
	$"0010 2010 72FF B081 6606 2F2E 0008 A9D0"            /* .. .rˇ∞Åf./...©– */
	$"206E FED4 A023 3004 4CDF 1CF8 4E5E 4E75"            /*  n˛‘†#0.Lﬂ.¯N^Nu */
	$"8D54 6578 7450 7269 6E74 5061 6765 0000"            /* çTextPrintPage.. */
	$"4E56 0000 48E7 1800 362E 0008 282E 000A"            /* NV..HÁ..6...(... */
	$"207C 0000 0B21 0210 00FB 3F03 31DF 0214"            /*  |...!...˚?.1ﬂ.. */
	$"2F04 21DF 0398 4CDF 0018 4E5E 4E75 9153"            /* /.!ﬂ.òLﬂ..N^NuëS */
	$"6574 4465 6661 756C 7453 6176 6544 6972"            /* etDefaultSaveDir */
	$"0000 4E56 FF6C 48E7 1C38 554F A994 301F"            /* ..NVˇlHÁ.8UO©î0. */
	$"3800 4EBA E890 2640 486E FF6C 2040 3010"            /* 8.N∫Ëê&@Hnˇl @0. */
	$"0640 1388 3F00 7007 3F00 4EBA 0534 7002"            /* .@.à?.p.?.N∫.4p. */
	$"2F00 70FF 3F00 4EBA FF88 5C4F 42A7 486E"            /* /.pˇ?.N∫ˇà\OBßHn */
	$"FF6C 486E FF8C 3F3C 0005 A9EA 102E FF8C"            /* ˇlHnˇå?<..©Í..ˇå */
	$"6700 0170 102E FF8D 6722 554F 486E FF92"            /* g..p..ˇçg"UOHnˇí */
	$"486E FFF0 7007 AA52 544F 0CAE 7474 7874"            /* Hnˇp.™RTO.Ættxt */
	$"FFF4 6708 363C 1388 6000 014C 554F 486E"            /* ˇÙg.6<.à`..LUOHn */
	$"FF92 7006 AA52 301F 3600 72D5 B041 6708"            /* ˇíp.™R0.6.r’∞Ag. */
	$"4A40 6704 6000 0130 554F 486E FF92 2F3C"            /* J@g.`..0UOHnˇí/< */
	$"7474 7874 2F3C 7474 726F 4267 7004 AA52"            /* ttxt/<ttroBgp.™R */
	$"301F 3600 486E FF92 2F3C 7474 7874 2F3C"            /* 0.6.Hnˇí/<ttxt/< */
	$"7474 726F 4267 700E AA52 3003 6704 6000"            /* ttroBgp.™R0.g.`. */
	$"00F6 554F 486E FF92 7003 1F00 486E FFE4"            /* .ˆUOHnˇíp...Hnˇ‰ */
	$"7002 AA52 301F 3600 4A40 6624 4227 A99B"            /* p.™R0.6.J@f$B'©õ */
	$"554F 486E FF92 7003 1F00 700D AA52 301F"            /* UOHnˇíp...p¬™R0. */
	$"3A00 554F A9AF 301F 3600 7001 1F00 A99B"            /* :.UO©Ø0.6.p...©õ */
	$"3003 670C 554F 486E FF92 7006 AA52 544F"            /* 0.g.UOHnˇíp.™RTO */
	$"594F 2F3C 7374 796C 244B 3012 0640 1388"            /* YO/<styl$K0..@.à */
	$"3F00 A9A0 201F 2840 594F 2F3C 5445 5854"            /* ?.©† .(@YO/<TEXT */
	$"3212 0641 1388 3F01 A9A0 201F 2D40 FFE8"            /* 2..A.à?.©† .-@ˇË */
	$"594F 2F00 4EBA 0352 201F 2D40 FFEC 554F"            /* YO/.N∫.R .-@ˇÏUO */
	$"3F2E FFE4 486E FFEC 206E FFE8 2F10 4EBA"            /* ?.ˇ‰HnˇÏ nˇË/.N∫ */
	$"0364 301F 3600 554F 3F2E FFE4 2F2E FFEC"            /* .d0.6.UO?.ˇ‰/.ˇÏ */
	$"4EBA 0396 3F2E FFE4 4EBA 032C 301F 3600"            /* N∫.ñ?.ˇ‰N∫.,0.6. */
	$"3F05 A998 2F0C A992 2F0C 2F3C 7374 796C"            /* ?.©ò/.©í/./<styl */
	$"3F3C 0080 487A 003C A9AB 554F A9AF 301F"            /* ?<.ÄHz.<©´UO©Ø0. */
	$"3600 3F05 A999 2F0C A9A3 2F2E FFE8 A9A3"            /* 6.?.©ô/.©£/.ˇË©£ */
	$"6004 7002 3600 3F04 A998 3003 4CDF 1C38"            /* `.p.6.?.©ò0.Lﬂ.8 */
	$"4E5E 4E75 8A53 6176 6541 7346 696C 6500"            /* N^NuäSaveAsFile. */
	$"0002 0000 4E56 FF00 48E7 1820 362E 0008"            /* ....NVˇ.HÁ. 6... */
	$"382E 000A 4EBA E6AE 2440 486E FF00 2040"            /* 8...N∫ÊÆ$@Hnˇ. @ */
	$"3010 0640 1388 3F00 3F04 4EBA 0354 486E"            /* 0..@.à?.?.N∫.THn */
	$"FF00 487A 0036 487A 0032 487A 002E A98B"            /* ˇ.Hz.6Hz.2Hz..©ã */
	$"554F 3F3C 1389 42A7 A985 544F 4CDF 0418"            /* UO?<.âBß©ÖTOLﬂ.. */
	$"4E5E 4E75 9243 6F6E 6475 6374 4572 726F"            /* N^NuíConductErro */
	$"7244 6961 6C6F 6700 0002 0000 4E56 0000"            /* rDialog.....NV.. */
	$"48E7 1020 362E 0008 594F 2F3C 7374 796C"            /* HÁ. 6...YO/<styl */
	$"3F03 A9A0 201F 2440 4A80 670A 2040 2050"            /* ?.©† .$@JÄg. @ P */
	$"3028 000A 6002 7001 4CDF 0408 4E5E 4E75"            /* 0(..`.p.Lﬂ..N^Nu */
	$"9747 6574 466F 6E74 4672 6F6D 5374 796C"            /* óGetFontFromStyl */
	$"5265 736F 7572 6365 0000 4E56 0000 203C"            /* Resource..NV.. < */
	$"0000 A89F A746 2F08 203C 0000 A0AD A346"            /* ..®üßF/. <..†≠£F */
	$"B1DF 670E 202E 000C A1AD 226E 0008 2288"            /* ±ﬂg. ...°≠"n.."à */
	$"6026 41FA 0036 303C EA51 222E 000C B298"            /* `&A˙.60<ÍQ"...≤ò */
	$"6706 4A98 6712 60F6 43FA 0020 D3D0 4ED1"            /* g.Jòg.`ˆC˙. ”–N— */
	$"226E 0008 2280 4240 3D40 0010 4E5E 205F"            /* "n.."ÄB@=@..N^ _ */
	$"508F 4ED0 303C EA52 60EE 7665 7273 0000"            /* PèN–0<ÍR`Óvers.. */
	$"0060 6D61 6368 0000 0064 7379 7376 0000"            /* .`mach...dsysv.. */
	$"0088 7072 6F63 0000 0092 6670 7520 0000"            /* .àproc...ífpu .. */
	$"009E 7164 2020 0000 00E8 6B62 6420 0000"            /* .ûqd  ...Ëkbd .. */
	$"011A 6174 6C6B 0000 0142 6D6D 7520 0000"            /* ..atlk...Bmmu .. */
	$"0164 7261 6D20 0000 0188 6C72 616D 0000"            /* .dram ...àlram.. */
	$"0188 0000 0000 0000 0000 7001 6082 2278"            /* .à........p.`Ç"x */
	$"02AE 7004 0C69 0075 0008 6712 0C69 0276"            /* .Æp..i.u..g..i.v */
	$"0008 6604 5240 6006 1038 0CB3 5C80 6000"            /* ..f.R@`..8.≥\Ä`. */
	$"FF60 7000 3038 015A 6000 FF56 7000 1038"            /* ˇ`p.08.Z`.ˇVp..8 */
	$"012F 5240 6000 FF4A 0C38 0004 012F 6738"            /* ./R@`.ˇJ.8.../g8 */
	$"0838 0004 0B22 6734 204F F280 0000 F327"            /* .8..."g4 OÚÄ..Û' */
	$"3017 2E48 0C40 1F18 6716 0C40 3F18 6710"            /* 0..H.@..g..@?.g. */
	$"0C40 3F38 670E 0C40 1F38 6708 7000 600E"            /* .@?8g..@.8g.p.`. */
	$"7001 600A 7002 6006 7003 6002 7000 6000"            /* p.`.p.`.p.`.p.`. */
	$"FF00 0C78 3FFF 028E 6E1C 303C A89F A746"            /* ˇ..x?ˇ.én.0<®üßF */
	$"2408 203C 0000 AB03 A746 203C 0000 0100"            /* $. <..´.ßF <.... */
	$"B488 6606 600A 7000 6006 203C 0000 0200"            /* ¥àf.`.p.`. <.... */
	$"6000 FECE 1038 021E 41FA 0016 2248 1218"            /* `.˛Œ.8..A˙.."H.. */
	$"6700 FED2 B200 66F6 91C9 2008 6000 FEB2"            /* g.˛“≤.fˆë… .`.˛≤ */
	$"0313 0B02 0106 0704 0508 0900 7000 4A38"            /* ..........∆.p.J8 */
	$"0291 6B16 1238 01FB 0201 000F 0C01 0001"            /* .ëk..8.˚........ */
	$"6608 2078 02DC 1028 0007 6000 FE84 0C38"            /* f. x.‹.(..`.˛Ñ.8 */
	$"0002 012F 6D16 7000 1038 0CB1 0C00 0001"            /* .../m.p..8.±.... */
	$"670C 0C00 0003 6D04 5340 6002 7000 6000"            /* g.....m.S@`.p.`. */
	$"FE60 303C A89F A746 2408 203C 0000 A88F"            /* ˛`0<®üßF$. <..®è */
	$"A746 2038 0108 B488 670A 598F 3F3C 0016"            /* ßF 8..¥àg.Yè?<.. */
	$"A88F 201F 6000 FE3A 225F 205F A025 2E80"            /* ®è .`.˛:"_ _†%.Ä */
	$"6A02 4297 4ED1 4E56 FFCE 204F 316E 0008"            /* j.BóN—NVˇŒ O1n.. */
	$"0018 A001 3D40 000A 4E5E 205F 548F 4ED0"            /* ..†.=@..N^ _TèN– */
	$"51C1 6002 50C1 4E56 FFCE 204F 216E 0008"            /* Q¡`.P¡NVˇŒ O!n.. */
	$"0020 316E 0010 0018 226E 000C 2151 0024"            /* . 1n...."n..!Q.$ */
	$"4268 002C 42A8 002E 4A01 6604 A002 6002"            /* Bh.,B®..J.f.†.`. */
	$"A003 3D40 0012 226E 000C 22A8 0028 4E5E"            /* †.=@.."n.."®.(N^ */
	$"225F 4FEF 000A 4ED1 4E56 FFCE 204F 316E"            /* "_OÔ..N—NVˇŒ O1n */
	$"000C 0018 216E 0008 001C A012 3D40 000E"            /* ....!n....†.=@.. */
	$"4E5E 225F 5C8F 4ED1 206F 0004 202F 0008"            /* N^"_\èN— o.. /.. */
	$"4267 A9EE 205F 504F 4ED0 206F 0004 2050"            /* Bg©Ó _PON– o.. P */
	$"A9E1 226F 0004 2288 3F40 0008 2E9F 4E75"            /* ©·"o.."à?@...üNu */
	$"4E56 0000 594F 2F3C 5354 5223 3F2E 000A"            /* NV..YO/<STR#?... */
	$"A9A0 226E 000C 4211 201F 6722 2040 2050"            /* ©†"n..B. .g" @ P */
	$"3018 322E 0008 6716 B240 6212 7000 5341"            /* 0.2...g.≤@b.p.SA */
	$"6706 1018 D1C0 60F6 1010 5240 A02E 4E5E"            /* g...—¿`ˆ..R@†.N^ */
	$"205F 508F 4ED0 222F 0004 202F 0008 41FA"            /*  _PèN–"/.. /..A˙ */
	$"000A 327C 0002 4EF0 92FE 6006 4C41 0801"            /* ..2|..Ní˛`.LA.. */
	$"4E75 4EBA 002E 2001 4E75 8505 4C44 4956"            /* NuN∫.. .NuÖ.LDIV */
	$"5400 0000 222F 0004 202F 0008 41FA 000A"            /* T..."/.. /..A˙.. */
	$"327C 0002 4EF0 92FE 6008 4C41 0801 C340"            /* 2|..Ní˛`.LA..√@ */
	$"4E75 4A80 6B14 4A81 6B06 4EBA 0050 4E75"            /* NuJÄk.JÅk.N∫.PNu */
	$"4481 4EBA 0048 4481 4E75 4480 4A81 6B0A"            /* DÅN∫.HDÅNuDÄJÅk. */
	$"4EBA 003A 4480 4481 4E75 4481 4EBA 002E"            /* N∫.:DÄDÅNuDÅN∫.. */
	$"4480 4E75 8505 4C4D 4F44 5400 0000 222F"            /* DÄNuÖ.LMODT..."/ */
	$"0004 202F 0008 41FA 000A 327C 0002 4EF0"            /* .. /..A˙..2|..N */
	$"92FE 6008 4C41 0001 C340 4E75 3F01 4841"            /* í˛`.LA..√@Nu?.HA */
	$"4A41 661C 2200 4241 4841 670A 82D7 4841"            /* JAf.".BAHAg.Ç◊HA */
	$"4840 3001 4840 80DF 3200 4240 4840 4E75"            /* H@0.H@Äﬂ2.B@H@Nu */
	$"4841 3E82 2F03 3400 2601 7201 4240 4840"            /* HA>Ç/.4.&.r.B@H@ */
	$"660C 4840 3002 7200 6016 D241 6512 D442"            /* f.H@0.r.`.“Ae.‘B */
	$"D180 B083 65F4 9083 D241 08C1 0000 64EE"            /* —Ä∞ÉeÙêÉ“A.¡..dÓ */
	$"261F 341F 4E75 8606 554C 4D4F 4454 0000"            /* &.4.NuÜ.ULMODT.. */
	$"201F 225F 205F 2E88 2F00 7000 7200 1010"            /*  ."_ _.à/.p.r... */
	$"1219 2401 6720 D240 0C41 00FF 6F0C 0441"            /* ..$.g “@.A.ˇo..A */
	$"00FF 9441 6710 123C 00FF 10C1 D1C0 5302"            /* .ˇîAg..<.ˇ.¡—¿S. */
	$"10D9 51CA FFFC 4E75 8850 4C53 7472 4361"            /* .ŸQ ˇ¸NuàPLStrCa */
	$"7400 0000"                                          /* t... */
};

resource 'DLOG' (5000, "Legal DLOG") {
	{29, 20, 333, 492},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	5000,
	"",
	centerMainScreen
};

resource 'STR ' (5000, "<Option>Print") {
	"…"
};

resource 'ALRT' (5001) {
	{93, 62, 197, 450},
	5001,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	},
	alertPositionMainScreen
};

resource 'ics#' (5000) {
	{	/* array: 2 elements */
		/* [1] */
		$"FFE0 7FC0 3F80 1F00 0E00 04",
		/* [2] */
		$"FFE0 7FC0 3F80 1F00 0E00 04"
	}
};
