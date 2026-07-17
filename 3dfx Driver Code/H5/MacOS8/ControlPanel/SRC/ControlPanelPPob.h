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

#ifndef _Control_Panel_PPob_
#define _Control_Panel_PPob_

#include <PP_Types.h>
#include <International_Lang.h>

/* !!!!!  Current Version Number !!!! to update for each new version !!!! */
#define				CURRENT_DRIVER_VERSION			kMACTOOLS_VERSION

#define				UPDATEDRV_HOST					"\pwww.3dfxgamers.com"
#define				UPDATEDRV_RESOURCE				"\p/drivers/voodoo5/mac/"
#define				USER_AGENT						"3dfx Update Drivers"

const	OSType		kDRVFILE_CREATOR				= FOUR_CHAR_CODE('BnHq');
const	OSType		kDRVFILE_TYPE					= FOUR_CHAR_CODE('TEXT');



// Special CHAR
#define				kOpenBracket					" [ "
#define				kCloseBracket					" ] "
#define				kCHAR_BY						" x "
#define				k8Bits							1
#define				k16Bits							2
#define				k32Bits							3

// RESTORE FACTORY DEFAULTS
#define				kDisabled						1
#define				kEnabled						2
#define				kBasic							2
#define				kSingleChip						1

#define				kDefault2DQDAcce				3
#define				kDefault2DFontCache				2
#define				kDefault2DPictCache				2
#define				kDefault2DQT					2
#define				kDefault2DMPEG					2
#define				kDefault3DAA					2
#define				kDefault3DAAKey					0
#define				kDefault3DGamma					15
#define				kDefault3DVSync					2
#define 			kDefault3D32Bit					1

// CCheckRom Class
#define				BeginDriverDescriptionStruct	"vers"
const	ResType		rom_ResourceType				= FOUR_CHAR_CODE('RomU');


// RESIDT
const 	ResIDT		ControlPanel_Window				= 128;
const 	ResIDT		About_Window					= 130;
const 	ResIDT		GameSettings_Window				= 131;
const	ResIDT		UpdateDrv_Window				= 132;
const	ResIDT		FlashRom_Window					= 133;
const	ResIDT		UpdateRom_Window				= 134;
const	ResIDT		EndUpdateRom_Window				= 135;
const	ResIDT		PP_AppInfoCell					= 4201;
const	ResIDT		display_ResolutionMENUID		= 131;
const	ResIDT		display_Depth_STR				= 2003;

// MacVoodooVersion File name
const 	ResIDT		UpdateFile_STR					= 2000;
#define				kVersionFileName				1

// The TEXTS UpdateDrv_STR
const	ResIDT		UpdateDrv_STR					= 2001;
#define				kChecking4Version				1
#define				kNewerVersionFound				2
#define				kNoNewerVersionFound			3
#define				kNowDownloading					4
#define				kAllGood						5
#define				kErrorCheckingVersion			6
#define				kErrorReceivingVersion			7
#define				kErrorDecompressing				8
#define				kErrorSaving					9
#define				kErrorDownloading				10
#define				kNowSaving						11
#define				kNowDecompressing				12
#define				kErrorConnecting				13


// The TEXTS UpdateDrvStatus_STR
const	ResIDT		UpdateDrvStatus_STR				= 2002;
#define				kIdle							1
#define				kFailedRetrieve					2
#define				kFinishRetrieve					3
#define				kOpenConnection					4
#define				kConnectedTo					5
#define				kConnectionTerminated			6
#define				kReceivingVersion				7
#define				kVersionReceived				8
#define				kReceivedK						9
#define				kKiloUnit						10
#define				kOf								11
#define				kSavingFile						12
#define				kDecompressingFile				13


// Divers
const	SInt16		index_2DPerfView				= 1;
const	SInt16		index_3DPerfView				= 2;
const	SInt16		index_DisplayView				= 3;
const	SInt16		index_ProfileView				= 4;
const	SInt16		index_GamesView					= 5;
const	SInt16		index_DebugView					= 6;


const	SInt16		index_3DPerf_OpenGL				= 1;
const	SInt16		index_3DPerf_Glide				= 2;
const	SInt16		index_3DPerf_Rave				= 3;
const	SInt16		index_AboutInfoView				= 1;
const	SInt16		index_UpdateDrvView				= 2;

// Main Window
const	PaneIDT		main_MultiPanelView				= FOUR_CHAR_CODE('MPV1');
const	PaneIDT		main_TabsControl				= FOUR_CHAR_CODE('TABS');

const 	PaneIDT		pane_BI_Quit					= FOUR_CHAR_CODE('QUIT');
const 	PaneIDT		pane_BI_FlashROM				= FOUR_CHAR_CODE('FLAS');

// FlashRom Window
const 	PaneIDT		flashRom_Cancel					= FOUR_CHAR_CODE('CNCL');
const 	PaneIDT		flashRom_Flash					= FOUR_CHAR_CODE('FLSH');
const 	PaneIDT		flashRom_AskAgain				= FOUR_CHAR_CODE('AGAN');
const	PaneIDT		flashRom_Close					= FOUR_CHAR_CODE('QUIT');
const	PaneIDT		flashRom_Restart				= FOUR_CHAR_CODE('RSTR');

// About View
const 	PaneIDT		about_Main						= FOUR_CHAR_CODE('ABTV');
const	PaneIDT		about_MPView					= FOUR_CHAR_CODE('MPV3');
const 	PaneIDT		about_OK						= FOUR_CHAR_CODE('OKOK');
const	PaneIDT		about_UpdateDriver				= FOUR_CHAR_CODE('UPDT');
const	PaneIDT		about_Picture					= FOUR_CHAR_CODE('LOGO');
const	PaneIDT		about_Name						= FOUR_CHAR_CODE('TXT1');
const	PaneIDT		about_Infos						= FOUR_CHAR_CODE('TXT2');
const	PaneIDT		about_Version					= FOUR_CHAR_CODE('TXT3');
const	PaneIDT		about_BuildVersion				= FOUR_CHAR_CODE('TXT4');
const	PaneIDT		about_tdfxURL					= FOUR_CHAR_CODE('mURL');

// Update Driver View
const 	PaneIDT		updatedrv_Main					= FOUR_CHAR_CODE('UPDT');
const 	PaneIDT		updatedrv_Status				= FOUR_CHAR_CODE('STAT');
const 	PaneIDT		updatedrv_Line1					= FOUR_CHAR_CODE('LIN1');
const 	PaneIDT		updatedrv_Cancel				= FOUR_CHAR_CODE('CNCL');
const 	PaneIDT		updatedrv_Download				= FOUR_CHAR_CODE('GOGO');



// 2DPerformance View
const	PaneIDT		perf2D_Main						= FOUR_CHAR_CODE('2DPE');
const	PaneIDT		perf2D_QDAccel					= FOUR_CHAR_CODE('QDAC');
const	PaneIDT		perf2D_FontCaching				= FOUR_CHAR_CODE('FTCA');
const	PaneIDT		perf2D_PictCaching				= FOUR_CHAR_CODE('PICA');
const	PaneIDT		perf2D_QTAccel					= FOUR_CHAR_CODE('QTAC');
const	PaneIDT		perf2D_MPEGAccel				= FOUR_CHAR_CODE('MPAC');

// 3DPerformance View
const	PaneIDT		perf3D_Main						= FOUR_CHAR_CODE('3DPE');
const	PaneIDT		perf3D_PopupGroupBox			= FOUR_CHAR_CODE('PGB1');
const	PaneIDT		perf3D_MultiPanelView			= FOUR_CHAR_CODE('MPV2');
const	PaneIDT		perf3D_OpenGLView				= FOUR_CHAR_CODE('OGLV');
const	PaneIDT		perf3D_GlideView				= FOUR_CHAR_CODE('GLIV');
const	PaneIDT		perf3D_RaveView					= FOUR_CHAR_CODE('RAVV');
const	PaneIDT		perf3D_TestButton				= FOUR_CHAR_CODE('TEST');

const	PaneIDT		perf3D_AAliasing				= FOUR_CHAR_CODE('AALI');
const	PaneIDT		perf3D_GammaTbl					= FOUR_CHAR_CODE('GAMA');
const	PaneIDT		perf3D_VSync					= FOUR_CHAR_CODE('VSYN');
const	PaneIDT		perf3D_AAliasingKey				= FOUR_CHAR_CODE('KyCF');
const	PaneIDT		perf3D_32Bit					= FOUR_CHAR_CODE('32BT');

// OpenGL View
const	PaneIDT		openGL_Main						= FOUR_CHAR_CODE('OGLV');

// Glide View
const	PaneIDT		glide_Main						= FOUR_CHAR_CODE('GLIV');

// Rave View
const	PaneIDT		rave_Main						= FOUR_CHAR_CODE('RAVV');

// AppInfoCell View
const	PaneIDT		game_MasterCell					= FOUR_CHAR_CODE('MCEL');
const	PaneIDT		game_Title						= FOUR_CHAR_CODE('gTIT');
const	PaneIDT		game_Infos						= FOUR_CHAR_CODE('gINF');
const	PaneIDT		game_Num						= FOUR_CHAR_CODE('gNUM');

// Game View
const	PaneIDT		game_Main						= FOUR_CHAR_CODE('GAME');
const	PaneIDT		game_ViewColumn					= FOUR_CHAR_CODE('VCOL');
const	PaneIDT		game_ButtEdit					= FOUR_CHAR_CODE('bEDI');
const	PaneIDT		game_ButtDelete					= FOUR_CHAR_CODE('bDEL');
const	PaneIDT		game_ButtAdd					= FOUR_CHAR_CODE('bADD');

// Display View
#define				kOptimalRefresh					"\pOptimal"
const	PaneIDT		display_Main					= FOUR_CHAR_CODE('DISP');
const	PaneIDT		display_PButtResolution			= FOUR_CHAR_CODE('pbRS');
const	PaneIDT		display_PButtRefreshRate		= FOUR_CHAR_CODE('pbRR');
const	PaneIDT		display_PButtDepth				= FOUR_CHAR_CODE('pbDE');
const	PaneIDT		display_ButtApply				= FOUR_CHAR_CODE('APLY');
const	PaneIDT		display_ButtCancel				= FOUR_CHAR_CODE('CNCL');

// Profile View
const	PaneIDT		profile_Main					= FOUR_CHAR_CODE('PROF');
const	PaneIDT		profile_CaptName				= FOUR_CHAR_CODE('pNAM');
const	PaneIDT		profile_CaptBus					= FOUR_CHAR_CODE('pBUS');
const	PaneIDT		profile_CaptCSeries				= FOUR_CHAR_CODE('pCSE');
const	PaneIDT		profile_CaptCNum				= FOUR_CHAR_CODE('pCNU');
const	PaneIDT		profile_CaptClock				= FOUR_CHAR_CODE('pCLK');
const	PaneIDT		profile_CaptMemory				= FOUR_CHAR_CODE('pMEM');
const	PaneIDT		profile_CaptVersion				= FOUR_CHAR_CODE('pVER');
const	PaneIDT		profile_RestoreDefault			= FOUR_CHAR_CODE('DFLT');

// Setting Game Window
const	PaneIDT		SetGame_ButtCancel				= FOUR_CHAR_CODE('CNCL');
const	PaneIDT		SetGame_ButtSave				= FOUR_CHAR_CODE('SAVE');
const	PaneIDT		SetGame_CaptTitle				= FOUR_CHAR_CODE('sTIT');

// Debug View
const 	PaneIDT		debug_Main						= FOUR_CHAR_CODE('DBUG');


// inMessage list
const	MessageT	msg_mainMPV						= main_MultiPanelView;
const	MessageT	msg_mainTabs					= main_TabsControl;
const	MessageT	msg_2DPerf_QD					= perf2D_QDAccel;
const	MessageT	msg_2DPerf_FontCach				= perf2D_FontCaching;
const	MessageT	msg_2DPerf_PictCach				= perf2D_PictCaching;
const	MessageT	msg_2DPerf_QT					= perf2D_QTAccel;
const	MessageT	msg_2DPerf_MPEG					= perf2D_MPEGAccel;
const	MessageT	msg_3DPerf_AAliasing			= perf3D_AAliasing;
const	MessageT	msg_3DPerf_GammaTbl				= perf3D_GammaTbl;
const	MessageT	msg_3DPerf_VSync				= perf3D_VSync;
const	MessageT	msg_3DPerf_PGB					= perf3D_PopupGroupBox;
const	MessageT	msg_3DPerf_TestButton			= perf3D_TestButton;
const	MessageT	msg_Games_bEdit					= game_ButtEdit;
const	MessageT	msg_Games_bAdd					= game_ButtAdd;
const	MessageT	msg_Games_bDelete				= game_ButtDelete;
const	MessageT	msg_Games_CellClicked					= 1000;
const	MessageT	msg_SetGameCancl				= SetGame_ButtCancel;
const	MessageT	msg_SetGameSave					= SetGame_ButtSave;
const	MessageT	msg_DispResolution				= display_PButtResolution;
const	MessageT	msg_DispRefreshRate				= display_PButtRefreshRate;
const	MessageT	msg_DispDepth					= display_PButtDepth;
const	MessageT	msg_DispApply					= display_ButtApply;
const	MessageT	msg_DispCancel					= display_ButtCancel;
const	MessageT	msg_ProfRestore					= profile_RestoreDefault;





// CPGamesListStruct

typedef struct _GameInfo {
	char		GameName[255]; 			// name of the game	
	char		GamePath[255];				// path of the game (we might add FSSpec too ?)
	char		GameInfos[255];				// info line under the name into the cell
	
	//GameSetting* GSettingStruct		// GameSetting save
	
	void *		GameCellPtr;
	int			GameNumber;				// theNumber of the game ??
	int			GameSelected;			// should be false by default
	
} GameInfo, * GameInfoP;


#endif /* _Control_Panel_PPob_ */
