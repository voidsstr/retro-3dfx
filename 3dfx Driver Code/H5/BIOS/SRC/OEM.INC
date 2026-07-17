;
;	OEM.INC - OEM Specific equates and BIOS conditional builds
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;
;	Written by:	Larry Coffey
;	Date:		3/28/97
;
;	$Revision$
;	$Date$
;	$Logfile: /devel/h5/BIOS/SRC/OEM.INC $
;	$Log: /devel/h5/BIOS/SRC/OEM.INC $
;; 
;; 21    9/09/99 3:39p Kaymann
;; Updated the Napalm BIOS verison number to 0.19.
;; 
;; 20    9/04/99 5:52p Kaymann
;; Updated the Napalm BIOS version 0.18.
;; 
;; 19    9/02/99 5:53p Kaymann
;; Updated the BIOS version to 0.17.
;; 
;; 18    9/02/99 5:41p Kaymann
;; Renamed the equate FLD_CRT to FLD_CRT_ACTIVE.
;
;
; The following equate defines whether there are extensions beyond
; the original IBM BIOS defined. If "EXTENDED_BIOS" is set to a
; "1", then extensions beyond the standard are used, if it is set
; to a "0", then a "pure" IBM-style BIOS is built. If extensions are
; included, then two further options may be set: support for character
; functions (EXTENDED_TTY) in extended modes and support for pages
; in planar modes. Disabling planar pages is a common setting for
; VBE implementations using a graphics chip's native mode. Setting
; the option to a "1" enables support, while setting it to a "0"
; disables it.
;
EXTENDED_BIOS		equ	1
EXTENDED_TTY		equ	1
EXTENDED_PLANARPAGES	equ	0
;
; The following equate defines whether VESA BIOS Extensions are
; to be used. If "VESA" is set to a "1", then the VBE is included,
; if it is set to a "0", then the VBE is not included. If the VBE
; is included, then there are five further options that may be set:
;   VESA_DPMS - Support for Display Power Management Signalling
;   VESA_DDC - Support for Display Data Channel
;   VESA_300 - Support for the VBE 3.0 specification
;   VESA_VGACOMPAT - Support for VGA-style banked framebuffer
;   VESA_STDMODES - Support for standard and extended text modes in functions
; Setting an option to a "1" enables the function, while setting it
; to a "0" disables it.
;
; Note: For maximum compatibility, the hardware SHOULD support the
; VGA banked framebuffer, but for those hardware vendors who prefer
; to limit their sales, setting this option to "0" is a last resort
; for providing some sort of VBE support.
;
VESA			equ	1
VESA_DPMS		equ	1
VESA_DDC		equ	1
VESA_300		equ	1
VESA_VGACOMPAT		equ	1
VESA_STDMODES		equ	0
;
; The following equate defines where the ROM is initially addressed.
; For most implementations, this will be at "0C000h", but some motherboard
; implementations may place the ROM at "0E000h".
;
ROM_SEGMENT		equ	0C000h
;
; The following equate defines whether the BIOS will be placed onto
; a PCI adapter and therefore would need the PCI header data. If "PCIBIOS"
; is set to a "1", then the header data is included, if it is set to
; a "0", then the header data is not included.
;
PCIBIOS			equ	1
PCI_ID			=	00000h
PCI_ID_DEVICE		=	00000h
;
; Variations - The following are OEM options that may or may not be
; set with regard to specific areas of the standard VGA BIOS. They
; usually are implemented as "work-arounds" for hardware bugs or
; specific system related implementation. The default setting for
; each one of these is a "0".
;
; Variation 1:	Monitor Detection - Setting this to a "1" will cause
;		the monitor detection code to not be assembled. This
;		will cause the BIOS to default to a color analog
;		monitor, even if a monochrome analog monitor is attached.
;		This is usually implemented in emulated systems (like
;		QuickTurn) since the analog comparator necessary for
;		this to work is usually not available.
;
VARIATION_001		=	0
;
;
; Variation 2:	INT 10h Redirection - On the original adapter version
;		of the IBM VGA, the INT 10h vector was redirected to
;		an INT 6Dh vector, which then did all the video BIOS
;		processing. Note that the PS/2 versions (Model 70, etc.)
;		did not have this redirection. Setting this to a "1"
;		will cause the INT 10h handler to NOT issue the INT 6Dh.
;
VARIATION_002		=	1
;
;
; Variation 3:	No 9-Dot Fonts - On most LCD controllers, the 9-dot fixup
;		in the fonts will run into each other and therefore the
;		8-Dot fonts must be left alone. Note that for compatibility
;		reasons, the pointers to the fixup table are pointed
;		at the table termination character (00h).
;
VARIATION_003		=	0
;
;
; Variation 4:	No 9x14 Fixup - Only used by the 350 line monochrome
;		text mode, this table eats up premium ROM space. Note
;		that setting this option may create compatibility problems.
;
VARIATION_004		=	0	; Set under TVOUT below
;
;
; Variation 5:	No reading from or writing to the RAMDAC. To OEM functions
;		are called instead of the standard I/O to/from the RAMDAC
;		registers. This option may be used by hardware vendors
;		that have special interface needs to the RAMDAC registers.
;
VARIATION_005		=	0
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 3Dfx specific definitions
;
BANSHEE_DOUBLESCAN	equ	1

;---------------------------------------------------------------------------
;The following are the default settings for the build option.
;Unless enabled in the makefile, the build option will be off or disabled state.

BoardI2CLineShorted	=		0h		;I2C lines ARE NOT shorted
BoardCRTSupport		=		100h	;Has CRT support

;SGRAM memory build option
SDRAM_BIOS				=		0
DDR_BIOS				=		0
SDR_BIOS				=		0
BoardMemType			=		0		;SGRAM memory

;Type of bus on adapter
BoardBusType			=		08h		;AGP Bus
ROM_64K        		=  		0
OPTIMIZE_ROM          	=     	0
FB_High_Priorty   		=   	0
								

;TV BIOS Options
BoardTVOutSupport		=		0	;No TV support
TVOUT_BIOS				=		0	;Not TV BIOS
PAL_BIOS				=		0	;Not PAL TV BIOS

;DVI BIOS options
BoardDFPSupport		=		0	;No DFP support
DVI_BIOS             	=     	0	;Not DFP BIOS


;Memory clock Options. Will default to 143MHz if not defined
  MCLK_210MHZ			=     	0  
  MCLK_200MHZ			=     	0  
  MCLK_183MHZ			=     	0  
  MCLK_170MHZ          =     	0  
  MCLK_166MHZ          =     	0  
  MCLK_155MHZ			=     	0  
  MCLK_150MHZ			=     	0  
  MCLK_143MHZ			=     	0  
  MCLK_125MHZ			=     	0  
  MCLK_100MHZ			=     	0  

;Flag for building V5P896
  Build_V5P896        =       0


;Number of chips on board
;Default is one chip on board
	NumberOfChips		=		1

;---------------------------------------------------------------------------

	DisableCopyrightMsg	=		0

IFDEF	TURN_OFF_COPYRIGHT_MSG
	DisableCopyrightMsg	=		1
ENDIF

;---------------------------------------------------------------------------

	Napalm2_BIOS		=		0		;We must be a Napalm1 BIOS

;---------------------------------------------------------------------------
IFDEF	BUILD_NAPALM2_BIOS
;***** Start of Napalm2 BIOSs *****
	Napalm2_BIOS		=		1		;We must be a Napalm2 BIOS

IFDEF	BOARD_V4P882_2
%out	Building BOARD V4P882-2	DDR
;PCI/DDR/143/No TV/With LCD | V4-4200
	BoardBusType		=		0		;PCI Bus
	DDR_BIOS			=		1		;DDR memory
  	MCLK_143MHZ         =     	1  
	DVI_BIOS           	=     	1		;DFP BIOS
	STR_OEMPRODUCT		equ		"Voodoo4-2 4200 LCD "

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V5P902_2
%out	Building BOARD V5P902-2 SDR
;AGP/DDR/143/No TV/No LCD | V5-5000
	BoardBusType		=		08h		;AGP Bus
	DDR_BIOS			=		1		;DDR memory
  	MCLK_143MHZ         =     	1  
	NumberOfChips		=		2
	STR_OEMPRODUCT		equ		"Voodoo5-2 5000 "
;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V4P868_2
%out	Building BOARD V4P868-2 DDR
;AGP/DDR/143/No TV/No LCD | V4-4000
	BoardBusType		=		08h		;AGP Bus
	DDR_BIOS			=		1		;SDR memory
  	MCLK_143MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo4-2 4000 "

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V4P869_2
%out	Building BOARD V4P869-2 SDR
;AGP/SDR/143/No TV/No LCD | V4-4000
	BoardBusType		=		08h		;AGP Bus
	SDR_BIOS			=		1		;SDR memory
  	MCLK_143MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo4-2 4000 "
ENDIF

;***** End of Napalm2 BIOSs *****
;---------------------------------------------------------------------------
;***** Start of Napalm BIOSs *****

ELSEIFDEF	BOARD_V5P848	
%out	Building BOARD V5P848
;AGP/SD/183/No TV/No LCD | V5-6000
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo5 6000 "
	NumberOfChips		=		4

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V5P846
%out	Building BOARD V5P846
;AGP/SD/166/No TV/No LCD | V5-5500
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo5 5500 "
	NumberOfChips		=		2

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V5P899
%out	Building BOARD V5P899
;AGP/SD/166/No TV/No LCD | V5-5500
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo5 5500 "
	NumberOfChips		=		2

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V5P896
%out	Building BOARD V5P896
;PCI/SD/166/No TV/No LCD | V5-5500
	BoardBusType		=		0		;PCI Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo5 5500 "
	NumberOfChips		=		2

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V5P902
%out	Building BOARD V5P902
;AGP/SG/150/No TV/No LCD | V5-5000
	BoardBusType		=		08h		;AGP Bus
	SGRAM_BIOS			=		0		;SGRAM memory
  	MCLK_166MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo5 5000 "
	NumberOfChips		=		2

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V4P850
%out	Building BOARD V4P851
;AGP/SD/166/No TV/With LCD | V4-4800 with DVI
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  
	DVI_BIOS           	=     	1		;DFP BIOS
	STR_OEMPRODUCT		equ		"Voodoo4 4800 LCD "

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V4P851
%out	Building BOARD V4P851
;AGP/SD/166/No TV/No LCD | V4-4800
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo4 4800 "

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V4P849
%out	Building BOARD V4P849
;AGP/SD/166/With TV/No LCD | V4-4800 with TV
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  

	TVOUT_BIOS         	=     	1		;TV BIOS
	;BoardTVOutSupport		=		20h	;Board supports Composite TV only
	BoardTVOutSupport		=		40h	;Board supports SVideo TV only
	;BoardTVOutSupport		=		60h	;Board supports SVideo and Composite TV 

	STR_OEMPRODUCT		equ		"Voodoo4 4800 TV "

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V4P845
%out	Building BOARD V4P845
;AGP/SD/166/With TV/No LCD | V4-4800 with TV and LCD
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  

	TVOUT_BIOS         	=     	1		;TV BIOS
	;BoardTVOutSupport		=		20h	;Board supports Composite TV only
	BoardTVOutSupport		=		40h	;Board supports SVideo TV only
	;BoardTVOutSupport		=		60h	;Board supports SVideo and Composite TV 

	DVI_BIOS           	=     	1		;DFP BIOS
	STR_OEMPRODUCT		equ		"Voodoo4 4800 TV/LCD "
;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V4P868
%out	Building BOARD V4P868
;AGP/SD/166/No TV/No LCD | V4-4500
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo4 4500 "
;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V4P872
%out	Building BOARD V4P872
;PCI/SD/166/No TV/No LCD | V4-4500
	BoardBusType		=		0		;PCI Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo4 4500 "
;---------------------------------------------------------------------------
ELSEIFDEF	BOARD_V4P879
%out	Building BOARD V4P879
;AGP/SD/166/No TV/With LCD | V4-4500 with LCD
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  

	DVI_BIOS           	=     	1		;DFP BIOS
	STR_OEMPRODUCT		equ		"Voodoo4 4500 LCD "
;---------------------------------------------------------------------------
ELSEIFDEF	BOARD_V4P880
%out	Building BOARD V4P880
;AGP/SD/166/With TV/No LCD | V4-4500 with TV 
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  

	TVOUT_BIOS         	=     	1		;TV BIOS
	;BoardTVOutSupport		=		20h	;Board supports Composite TV only
	BoardTVOutSupport		=		40h	;Board supports SVideo TV only
	;BoardTVOutSupport		=		60h	;Board supports SVideo and Composite TV 

	STR_OEMPRODUCT		equ		"Voodoo4 4500 TV "
;---------------------------------------------------------------------------
ELSEIFDEF	BOARD_V4P881
%out	Building BOARD V4P881
;AGP/SD/166/With TV/With LCD | V4-4500 with TV and LCD
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  

	TVOUT_BIOS         	=     	1		;TV BIOS
	;BoardTVOutSupport		=		20h	;Board supports Composite TV only
	BoardTVOutSupport		=		40h	;Board supports SVideo TV only
	;BoardTVOutSupport		=		60h	;Board supports SVideo and Composite TV 

	DVI_BIOS           	=     	1		;DFP BIOS
	STR_OEMPRODUCT		equ		"Voodoo4 4500 TV/LCD "
;---------------------------------------------------------------------------
ELSEIFDEF	BOARD_V4P882
%out	Building BOARD V4P882
;PCI/Sd/166/No TV/With LCD | V4-4500 with LCD
	BoardBusType		=		0		;PCI Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  

	DVI_BIOS           	=     	1		;DFP BIOS
	STR_OEMPRODUCT		equ		"Voodoo4 4500 LCD "
;---------------------------------------------------------------------------
ELSEIFDEF	BOARD_V4P883
%out	Building BOARD V4P883
;PCI/SD/166/With TV/No LCD | V4-4500 with TV 
	BoardBusType		=		0		;PCI Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  

	TVOUT_BIOS         	=     	1		;TV BIOS
	;BoardTVOutSupport		=		20h	;Board supports Composite TV only
	BoardTVOutSupport		=		40h	;Board supports SVideo TV only
	;BoardTVOutSupport		=		60h	;Board supports SVideo and Composite TV 

	STR_OEMPRODUCT		equ		"Voodoo4 4500 TV "
;---------------------------------------------------------------------------
;ELSEIFDEF	BOARD_V4P884
;%out	Building BOARD V4P884
;;PCI/SD/166/With TV/With LCD | V4-4500 with TV and LCD
;	BoardBusType		=		0		;PCI Bus
;	SDRAM_BIOS			=		1		;SDRAM memory
;  	MCLK_166MHZ         =     	1  
;
;	TVOUT_BIOS         	=     	1		;TV BIOS
;	;BoardTVOutSupport	=		20h	;Board supports Composite TV only
;	BoardTVOutSupport	=		40h	;Board supports SVideo TV only
;	;BoardTVOutSupport	=		60h	;Board supports SVideo and Composite TV 
;
;	DVI_BIOS           	=     	1		;DFP BIOS
;	STR_OEMPRODUCT		equ		"Voodoo4 4500 TV/LCD "
;---------------------------------------------------------------------------
ELSEIFDEF	BOARD_V5P896
%out	Building BOARD V5P896
;PCI/SD/166/No TV/No LCD | V5-5500
	BoardBusType		=		0		;PCI Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo5 5500 "
	NumberOfChips		=		2

;Flag for building V5P896
   Build_V5P896        =       1

;---------------------------------------------------------------------------
ELSEIFDEF	BOARD_V4P887
%out	Building BOARD V4P887
;PCI/SD/166/No TV/No LCD | V4-4500
	BoardBusType		=		0		;PCI Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_166MHZ         =     	1  

	TVOUT_BIOS         	=     	1		;TV BIOS
	;BoardTVOutSupport	=		20h	;Board supports Composite TV only
	BoardTVOutSupport	=		40h	;Board supports SVideo TV only
	;BoardTVOutSupport	=		60h	;Board supports SVideo and Composite TV 

	DVI_BIOS           	=     	1		;DFP BIOS
	STR_OEMPRODUCT		equ		"Voodoo4 4500 TV/LCD "

;---------------------------------------------------------------------------

ELSEIFDEF	BOARD_V4P906
%out	Building BOARD V4P906
;AGP/SD/143/No TV/No LCD | V4-4400
	BoardBusType		=		08h		;AGP Bus
	SDRAM_BIOS			=		1		;SDRAM memory
  	MCLK_143MHZ         =     	1  
	STR_OEMPRODUCT		equ		"Voodoo4 4400 "
;---------------------------------------------------------------------------
;***** End of Napalm2 BIOSs *****

ELSE
;We get here is a board build option isn't defined
;***************************************************************************
;---------------------------------------------------------------------------
BoardI2CLineShorted	=		0h		;I2C lines ARE NOT shorted
;BoardI2CLineShorted	=		200h	;I2C lines ARE shorted
;---------------------------------------------------------------------------
BoardCRTSupport		=		100h	;Has CRT support
;BoardCRTSupport		=		0		;Has no CRT support
;---------------------------------------------------------------------------

IFDEF BUILD_SDRAM_BIOS			; From MAKEFILE "VGABIOS_OPT" environment variable
  	SDRAM_BIOS		=		1
ELSEIF BUILD_DDR_BIOS
  	DDR_BIOS		=		1
ELSEIF BUILD_SDR_BIOS
  	SDR_BIOS		=		1
ENDIF

;---------------------------------------------------------------------------

IFDEF	BUILD_PCI_BIOS
	BoardBusType	=		0		;PCI Bus
ENDIF

;---------------------------------------------------------------------------

IFDEF BUILD_TVOUT_BIOS			; From MAKEFILE "VGABIOS_OPT" environment variable
;---------------------------------------------------------------------------
	IFDEF BUILD_TVOUT_COMP
		BoardTVOutSupport		=		20h	;Composite TV only
	ELSEIFDEF BUILD_TVOUT_COMP_AND_SVIDEO
		BoardTVOutSupport		=		60h	;SVideo and Composite TV 
	ELSE
		BoardTVOutSupport		=		40h	;SVideo TV only
	ENDIF
;---------------------------------------------------------------------------
  	TVOUT_BIOS		=	1

  	IFDEF BUILD_PAL				; From MAKEFILE, only valid with TVOUT
  		PAL_BIOS		=	1
	ENDIF

ENDIF

IFDEF BUILD_OPTIMIZE_ROM
  OPTIMIZE_ROM          =     1
ENDIF

IFDEF BUILD_DVI_BIOS
;---------------------------------------------------------------------------
BoardDFPSupport		=		80h	;DFP supported
;---------------------------------------------------------------------------
  DVI_BIOS             =     1
ENDIF

IFDEF BUILD_210MHZ_BIOS
  MCLK_210MHZ           =     1  
ENDIF

IFDEF BUILD_200MHZ_BIOS
  MCLK_200MHZ           =     1  
ENDIF

IFDEF BUILD_183MHZ_BIOS
  MCLK_183MHZ           =     1  
ENDIF

IFDEF BUILD_170MHZ_BIOS
  MCLK_170MHZ           =     1  
ENDIF

IFDEF BUILD_166MHZ_BIOS
  MCLK_166MHZ           =     1  
ENDIF

IFDEF BUILD_155MHZ_BIOS
  MCLK_155MHZ           =     1  
ENDIF

IFDEF BUILD_150MHZ_BIOS
  MCLK_150MHZ           =     1  
ENDIF

IFDEF BUILD_143MHZ_BIOS
  MCLK_143MHZ           =     1  
ENDIF

IFDEF BUILD_125MHZ_BIOS
  MCLK_125MHZ           =     1  
ENDIF

IFDEF BUILD_100MHZ_BIOS
  MCLK_100MHZ           =     1  
ENDIF

IFDEF   Disable_FB_High
  FB_High_Priorty   =   0
ENDIF

;---------------------------------------------------------------------------
;OEM Product Name
	STR_OEMPRODUCT			equ	"Voodoo4 4000 "
;***************************************************************************
ENDIF

;---------------------------------------------------------------------------
;If we're a DVI or TV BIOS, we must be a 64K BIOS
IF      (DVI_BIOS OR TVOUT_BIOS)
  	ROM_64K        		=  		1
   VARIATION_004		=		1
ENDIF

;---------------------------------------------------------------------------
IF      (DVI_BIOS)
	BoardDFPSupport		=		80h	;DFP supported
ENDIF
;---------------------------------------------------------------------------
  IF	SDRAM_BIOS
	BoardMemType		=		1		;SDRAM memory
  ELSEIF (DDR_BIOS)
	BoardMemType		=		2		;DDR memory
  ELSEIF (SDR_BIOS)
	BoardMemType		=		3		;SDR memory
  ENDIF
;---------------------------------------------------------------------------

;
IFDEF IDENTIFY_BIOSTYPE
  IF BANSHEE_DOUBLESCAN
	%out	!		BIOS Type: 3Dfx Interactive, Inc. (Doublescan modes included)
  ELSE
	%out	!		BIOS Type: 3Dfx Interactive, Inc. (No doublescan modes)
  ENDIF
  IF ROM_64K
  %out   !     64K ROM
  ELSE
  %out   !     32K ROM
  ENDIF

  IF TVOUT_BIOS
    IF PAL_BIOS
	%out	!				(TVOUT version - PAL)
    ELSE
	%out	!				(TVOUT version - NTSC)
    ENDIF
  ENDIF

  IF OPTIMIZE_ROM
  %out    !         Optimize BIOS
  ENDIF

  IF DVI_BIOS
        %out    !   DVI BIOS
  ENDIF

  IF	SDRAM_BIOS
  		%OUT	!	SDRAM BIOS
  ELSEIF DDR_BIOS
  		%OUT	!	DDR BIOS
  ELSEIF SDR_BIOS
  		%OUT	!	SDR BIOS
  ELSE
  		%OUT	!	SGRAM BIOS
  ENDIF

ENDIF

;OEM name string as part of the sign-on message and in the VBEInfoStruct
	STR_OEMNAME					equ	"3dfx Interactive, Inc."

;OEM Vendor name in the VBEInfoStruct structure
	STR_VBE_OEMVENDOR     		equ	"3dfx Interactive, Inc."

;OEM Vendor name in the VBEInfoStruct structure
	STR_VBE_OEMPRODUCTREV		equ	"Version 1.00"

;OEM chip name
	STR_OEMCHIPNAME			equ	"VSA 100"

IF		(Napalm2_BIOS)
;Napalm2 BIOS version
%OUT Napalm2 BIOS version
  STR_OEMVERSION		equ	"1.05 "
ELSE
;Napalm1 BIOS version
%OUT Napalm BIOS version
  STR_OEMVERSION		equ	"1.18 "
ENDIF

;

IF		Napalm2_BIOS

%OUT Napalm2 SSID
;Device ID can range from 0A-0D.  Pick one.
PCI_ID				=	0121Ah
PCI_ID_DEVICE		=	0000Bh          ;Napalm 3000 Device ID (Bring up ID)

ELSE

%OUT Napalm SSID
PCI_ID				=	0121Ah
PCI_ID_DEVICE		=	00009h          ;Napalm 3000 Device ID (Bring up ID)

ENDIF

;
; Register locations
;
CRTC_HEXT_OVER		equ	01Ah		; CRTC extension
CRTC_VEXT_OVER		equ	01Bh		; CRTC extension
;
; Scratch register 0 is used to store the PCI I/O base address. Only
; the upper nibble of the I/O base is stored here, the lower nibble
; is assumed to be zero.
;
SCRATCH_REG0		equ	01Ch		; Bits 0-7: I/O Base
;
; Scratch register 1 is used to store memory usage information. Bits
; 0-6 are unused for now, but bit 7 is used as the VBE flag indicating
; whether the video mode was set with the linear framebuffer access
; enabled or not.
;
SCRATCH_REG1		equ	01Dh		; Bits 0-6: Memory size (unused)
						; Bit 7: Linear mode flag
FL_LINEARMODE		equ	080h
						; Bit 6: 0 - 1024x768 panel attached; 1 - 1280x1024 panel attached
;
; Scratch register 2 is used to store TV information. Bits 0-1 indicate
; the type of monitor output to be generated. Possible values include
; VGA (0), NTSC (1), and PAL (2). Only VGA and NTSC are supported for now.
;
SCRATCH_REG2		equ	01Eh		; Bits 0-1: NTSC/PAL/VGA mode
FLD_TVMASK			equ	003h
FLD_VGAMODE		equ	000h
FLD_NTSCMODE		equ	001h
FLD_PALMODE		equ	002h
FLD_CRT_ACTIVE		equ	080h
FLD_COMPOSITE		equ	002h
FLD_COMPOSITE_SVIDEO	equ	004h

FLD_PALMODEMASK  equ   18h
FLD_PAL_BGDHI    equ   00h
FLD_PAL_N        equ   01h
FLD_PAL_M        equ   02h
FLD_PAL_Nc       equ   03h

FLD_TV_ACTIVE    equ   01h

FLD_SDRAMBOARD	equ	40000000h

CFG_TV_MASK		equ	FLD_PALMODEMASK + FLD_TVMASK
CFG_PAL_BGDHI    equ   02h
CFG_PAL_N        equ   0Ah
CFG_PAL_M        equ   12h
CFG_PAL_Nc       equ   1Ah
CFG_NTSC       	equ   01h

;Scratch register 3 is used to store DVI information
SCRATCH_REG3		equ	01Fh		

FLD_DVI		   equ	001h 

CFG_DFP_SIZE_MASK	equ	0F9h
CFG_DFP_SIZE_1024	equ	000h			;CR1F[2:1] = 0
CFG_DFP_SIZE_1280	equ	002h			;CR1F[2:1] = 1
CFG_DFP_SIZE_1600	equ	004h			;CR1F[2:1] = 2

;Established timing field defination
FLD_EDID_1280			equ	00100h			;Established timing field for 1280
FLD_EDID_1024			equ	01E00h			;Established timing field for 1024
FLD_EDID_800			equ	0C003h			;Established timing field for 800

FLD_PANELSCALING		equ	008h

;
REG_PCIINIT0		equ	004h
REG_LFBMEMORYCONFIG	equ	00Ch
REG_MISCINIT0		equ	010h
REG_MISCINIT1		equ	014h
REG_DRAMINIT0		equ	018h
REG_DRAMINIT1		equ	01Ch
REG_AGPINIT0		equ	020h
REG_TMUGBEINIT	EQU	024h
REG_VGAINIT0		equ	028h
REG_VGAINIT1		equ	02Ch
REG_DRAMSMODE		equ	034h
REG_DRAMMODE		equ	030h
REG_STRAPINFO		equ	038h
REG_VIDCLOCK		equ	040h
REG_PLLCTRL1		equ	044h		; Mem/Grx clock
REG_PLLCTRL2		equ	048h		; AGP clock
REG_DACMODE		equ	04Ch
REG_VIDPROCCFG          equ     05Ch
REG_VIDINFORMAT		equ	070h
REG_VIDSERIALPARALLEL	equ	078h
REG_VIDINDECIMXDELTA	equ	07Ch
REG_VIDINDECIMYDELTA	equ	084h
REG_VIDSCREENSIZE	equ	098h
REG_VIDDESKTOPSTART	equ	0E4h
REG_VIDDESKTOPSTRIDE	equ	0E8h
;

if     (SDR_BIOS)
   DEFAULT_TMUGBEINIT	equ	00C000FF0h
else
	DEFAULT_TMUGBEINIT	equ	000000FF0h
endif


DEFAULT_PCIINIT0	equ	001841320h	; Set default for PCI.  Or in WS later for AGP.

;IF FB_High_Priorty
;  DEFAULT_PCIINIT0	equ	00584FB20h	; Set default for PCI.  Or in WS later for AGP.
;ELSE
;  DEFAULT_PCIINIT0	equ	00184FB20h	; Set default for PCI.  Or in WS later for AGP.
;ENDIF

DEFAULT_MISCINIT0	equ	000000000h

DEFAULT_MISCINIT1	equ	000000001h

IF DDR_BIOS

;--------------------------------------------------------------------------------
IF	(MCLK_100MHZ OR MCLK_125MHZ)
%out	143MHz> Napalm2 DDR DramInit timings
;143MHz< Napalm2 DDR DramInit timings
	DEFAULT_DRAMINIT0	equ	0807FE9A9h
	DEFAULT_DRAMINIT1	equ	040034031h

ELSE
%out   143MHz<= Napalm2 DDR DramInit timings
	DEFAULT_DRAMINIT0	equ	0807FE9A9h
	DEFAULT_DRAMINIT1	equ	040034031h
;	DEFAULT_DRAMINIT1	equ	040030031h
ENDIF
;--------------------------------------------------------------------------------

ELSEIF (SDR_BIOS)

;--------------------------------------------------------------------------------
IF	(MCLK_100MHZ OR MCLK_125MHZ)
%out	143MHz> Napalm2 DDR DramInit timings
;143MHz< Napalm2 DDR DramInit timings
	DEFAULT_DRAMINIT0	equ	0001FE9A9h
	DEFAULT_DRAMINIT1	equ	041A00031h
ELSE
%out   143MHz<= Napalm2 DDR DramInit timings
	DEFAULT_DRAMINIT0	equ	0001FE9A9h
	DEFAULT_DRAMINIT1	equ	041A04031h
ENDIF
;--------------------------------------------------------------------------------

ELSE

%out   Napalm DramInit timings
;166MHz DramInit timings Test BIOS 1.03h
DEFAULT_DRAMINIT0	equ	0001EA9A9h
DEFAULT_DRAMINIT1	equ	000240031h

ENDIF

;143MHz DramInit timings
;;DEFAULT_DRAMINIT0	equ	00116A9A9h
;;DEFAULT_DRAMINIT1	equ	000240031h

;Old 100 MHz DramInit timings
;DEFAULT_DRAMINIT0	equ	000169D25h
;DEFAULT_DRAMINIT1	equ	00046C031h

DEFAULT_AGPINIT0	equ	00000049Eh

;**********************************************************************
;Memory clock PLL settings

IF MCLK_210MHZ
   DEFAULT_PLLCTRL1	equ	000005605h	; 210 MHz MemClock, GrxClock
ELSEIF MCLK_200MHZ
   DEFAULT_PLLCTRL1	equ	00000F91Dh	; 200 MHz MemClock, GrxClock
ELSEIF MCLK_183MHZ
   DEFAULT_PLLCTRL1	equ	000004B05h	; 183 MHz MemClock, GrxClock
ELSEIF MCLK_170MHZ
   DEFAULT_PLLCTRL1	equ	00000BC19h	; 170 MHz MemClock, GrxClock
ELSEIF MCLK_166MHZ
   DEFAULT_PLLCTRL1	equ	00000E721h	; 166 MHz MemClock, GrxClock (Passes FCC.  No pajama bug)
ELSEIF MCLK_155MHZ
   DEFAULT_PLLCTRL1	equ	000003F05h	; 155 MHz MemClock, GrxClock
ELSEIF MCLK_150MHZ
   DEFAULT_PLLCTRL1	equ	00000F929h	; 150 MHz MemClock, GrxClock
ELSEIF MCLK_143MHZ
   DEFAULT_PLLCTRL1	equ	000003A05h	; 143 MHz MemClock, GrxClock
ELSEIF MCLK_125MHZ
   DEFAULT_PLLCTRL1	equ	00000E12Dh	; 125 MHz MemClock, GrxClock
ELSEIF MCLK_100MHZ
   DEFAULT_PLLCTRL1	equ	000002805h	; 100 MHz MemClock, GrxClock
ELSE
   DEFAULT_PLLCTRL1	equ	000003A05h	; 143 MHz MemClock, GrxClock
ENDIF
DEFAULT_PLLCTRL2	equ	000000000h	; Placeholder for Avenger

;**********************************************************************

IF	Napalm2_BIOS
	IF DDR_BIOS
		DEFAULT_SGRAMMODE	equ	000002000h
	ELSE
		DEFAULT_SGRAMMODE	equ	000000030h
	ENDIF
ELSE
	DEFAULT_SGRAMMODE	equ	000000037h
ENDIF
;
; Extended register data structure for mode parameters
;
EXTREGS struc
  xrFlags		db	?	; Mode flags (Bit 0 maps to multiple bits):
  					;  Bit 0: IOBase[5Ch].0 (VIDPROCCFG - VGA/Native mode)
					;         IOBase[5Ch].7 (VIDPROCCFG - Desktop surface enable)
					;         IOBase[28h].12 (VGAINIT0 - Extended shift out)
					;         IOBase[2Ch].20 (VGAINIT1 - Sequential Chain/4)
					;  Bit 1: IOBase[5Ch].3 (VIDPROCCFG - Interlace)
					;  Bit 2: IOBase[5Ch].4 (VIDPROCCFG - Vertical X2)
					;  Bits 3-5: IOBase[5Ch].18..20 (VIDPROCCFG - Desktop pixel format)
					;  Bit 6-7: Reserved
  xrOverflow		db	?	; CRTC[1Bh]
if	(DVI_BIOS OR TVOUT_BIOS)
  xrTVOutIdx		db	?	; Index into the TV parameters array
ENDIF
  xrParmIdx		db	?	; If non-zero, index of parameter table
  xrClock		dw	?	; Clock M,N,P

IF	(DVI_BIOS OR TVOUT_BIOS)
  xrDVIIdx			db	?	; Index into the DVI parameters array
  xrDVICR04_1024	db	? 
  xrDVICR05_1024	db	? 
  xrDVICR10_1024	db	? 
  xrDVICR11_1024	db	? 
  xrDVICR02_1024	db	? 
  xrDVICR03_1024	db	? 
  xrDVICR15_1024	db	? 
  xrDVICR16_1024	db	? 
  xrDVICR04_1280	db	? 
  xrDVICR05_1280	db	? 
  xrDVICR10_1280	db	? 
  xrDVICR11_1280	db	? 
  xrDVICR02_1280	db	? 
  xrDVICR03_1280	db	? 
  xrDVICR15_1280	db	? 
  xrDVICR16_1280	db	? 
ENDIF
EXTREGS ends

LCDREGS struc
  xrDVICR04		db	? 
  xrDVICR05		db	? 
  xrDVICR10		db	? 
  xrDVICR11		db	? 
  xrDVICR02		db	? 
  xrDVICR03		db	? 
  xrDVICR15		db	? 
  xrDVICR16		db	? 
LCDREGS ends

LCD_CR	struc
  xrDVICR00		db	? 
  xrDVICR06		db	? 
  xrDVICR07		db	? 
LCD_CR	ends

;
TVDATA	struc
  tvClock		dw	?		; Clock word
  tvBT868Cfg		db	?		; BT868[B8]
  tvBT868Blank	db	?		; BT868[8C]
  tvCRTC0			db	?		; CRTC[0]
  tvCRTC3			db	?		; CRTC[3]
  tvCRTC4			db	?		; CRTC[4]
  tvCRTC5			db	?		; CRTC[5]
  tvCRTC6			db	?		; CRTC[6]
  tvCRTC7			db	?		; CRTC[7]
  tvCRTC10			db	?		; CRTC[10]
  tvBT868MSC0		db	?		; BT868[AE]
  tvBT868MSC1		db	?		; BT868[B0]
  tvBT868MSC2		db	?		; BT868[B2]
  tvBT868MSC3		db	?		; BT868[B4]
TVDATA	ends

TVPALPATCH struct
  PALRegA2    db ?     ;New Reg A2 value
  PALRegAE    db ?     ;New Reg AE value
  PALRegB0    db ?     ;New Reg B0 value
  PALRegB2    db ?     ;New Reg B2 value
  PALRegB4    db ?     ;New Reg B4 value
TVPALPATCH ends

;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


