#ifndef __BIOS_H__
#define __BIOS_H__

#ifdef WINNT
// NT builds already have this defined in h3.h
#else
#define MAX_BIOS_VERSION_LENGTH 32
#endif

#define BYTEBITRANGE(hi,lo) ((FxU8)(((((((FxU8)0xFF) >> (lo)) << (lo)) << (7-(hi))) & 0xFF) >> (7-(hi))))


#define BIOS_CRx1C_IOADDRESS



#define BIOS_CRx1D_LINEARMODE   BIT(7)

#define BIOS_CRx1D_MEMORYSIZE   BYTEBITRANGE(3,0)
        #define BIOS_CRx1D_8MEG         0x00
        #define BIOS_CRx1D_16MEG        0x01
        #define BIOS_CRx1D_32MEG        0x03
        #define BIOS_CRx1D_64MEG        0x04
        #define BIOS_CRx1D_128MEG       0x05



#define BIOS_CRx1E_CRTDETECTED  BIT(7)

#define BIOS_CRx1E_TVCONNECTED  BYTEBITRANGE(2,1)
        #define BIOS_CRx1E_SVIDEO       0x00
        #define BIOS_CRx1E_COMPOSITE    0x02
        #define BIOS_CRx1E_BOTH         0x04

#define BIOS_CRx1E_TVACTIVE     BIT(0)



#define BIOS_CRx1F_PANELSCALING BIT(3)

#define BIOS_CRx1F_DFPCONNECTED BYTEBITRANGE(2,1)
        #define BIOS_CRx1F_1024x768     0x00
        #define BIOS_CRx1F_1280x1024    0x02
        #define BIOS_CRx1F_1600x1200    0x04

#define BIOS_CRx1F_DFPACTIVE    BIT(0)



#define BIOS_NVx00_TVSTANDARDMASK (BIOS_NVx00_NTSC | BIOS_NVx00_PAL_BDGHI | BIOS_NVx00_PAL_N | BIOS_NVx00_PAL_M | BIOS_NVx00_PAL_Nc)
        #define BIOS_NVx00_NTSC   		  0x01   // 	00001
        #define BIOS_NVx00_PAL_BDGHI    0x02   //   00010
        #define BIOS_NVx00_PAL_N  		  0x0a   // 	01010
        #define BIOS_NVx00_PAL_M  		  0x12   // 	10010
        #define BIOS_NVx00_PAL_Nc       0x1a   //   11010


#define BIOS_NVx01_OVERRIDEMASK   0x03
        #define BIOS_NVx01_AUTODETECT   0x00
        #define BIOS_NVx01_SVIDEO       0x01
        #define BIOS_NVx01_COMPOSITE    0x02

#define BIOS_NVx7E_VERSIONCODE    0x05


// definitions for use with OEM config table in BIOS
#define ROM_CONFIG (0x50)
#define FIRSTOEMSTRUCTCOMPATVERSION 3
#pragma pack (push,2)
typedef struct oemtable {
    FxU32 pciInit0;
    FxU32 miscInit0;
    FxU32 miscInit1;
    FxU32 dramInit0;
    FxU32 dramInit1;
    FxU32 agpInit0;
    FxU32 pllCtrl1;
    FxU32 pllCtrl2;
    FxU32 sgramMode;
    FxU16 PanelTable_Common_Reg;
    FxU16 PanelTable_Common;
    FxU16 PanelTable_Mode_Reg;
    FxU16 PanelTable_Mode;
    FxU8  TvMode;
    FxU8  Reserved;
    FxU32 BoardConfig;
    FxU32 TmuGbeInit;
    FxU16 OEMBoardName;
    FxU16 OEMChipName;
    FxU16 OEMBiosVersion;
    } OEMTABLE, * POEMTABLE;
#pragma pack (pop)

#define BIOS_BOARDCONFIG_MEMORYTYPE			   0X0007
#define BIOS_BOARDCONFIG_BUSTYPE			   0x0018
#define BIOS_BOARDCONFIG_TVOUTSUPPORT		   0x0060
#define BIOS_BOARDCONFIG_DFPSUPPORT			   0x0080
#define BIOS_BOARDCONFIG_CRTSUPPORT			   0x0100
#define BIOS_BOARDCONFIG_I2CLINESSHORTED	   0x0200



/*********			OLD VOODOO3 NAMES
#define BIOS_NTSC   		0x01   // 	00001
#define BIOS_PAL    		0x02   //   (BGDHI)	00010
#define BIOS_PAL_N  		0x0a   // 	01010
#define BIOS_PAL_M  		0x12   // 	10010
#define BIOS_PAL_Nc             0x1a   //   11010

#define BIOS_TVSTD_MASK     (BIOS_NTSC | BIOS_PAL | BIOS_PAL_N | BIOS_PAL_M | BIOS_PAL_Nc)
#define BIOS_LCD_ACTIVE         0x04
#define BIOS_TVOUT_ACTIVE       0x20
#define BIOS_TVOUT_COMPOSITE    0x40
**********/


/*----------------------------------------------------------------------
Macro name:  ReadScratchRegister2

Description:    read a byte value from BIOS scratch register 2 (location 0x1E)

Information:    
----------------------------------------------------------------------*/
#ifdef WINNT
#define READSCRATCHREGISTER2(pContext,result)  \
    { \
    /* save index reg setting */ \
    FxU8 bSavedIndex = VideoPortReadPortUchar((PUCHAR)((PHW_DEVICE_EXTENSION)(pContext))->MappedAddress[IOBASE_ZERO] + 0xd4); \
    /* return value from register shared with BIOS */ \
    VideoPortWritePortUchar((PUCHAR)((PHW_DEVICE_EXTENSION)(pContext))->MappedAddress[IOBASE_ZERO] + 0xd4, 0x1e);  /* select the reg */ \
    result = VideoPortReadPortUchar((PUCHAR)((PHW_DEVICE_EXTENSION)(pContext))->MappedAddress[IOBASE_ZERO] + 0xd5);  /* read the reg */ \
    /* restore index reg setting */ \
    VideoPortWritePortUchar((PUCHAR)((PHW_DEVICE_EXTENSION)(pContext))->MappedAddress[IOBASE_ZERO] + 0xd4, bSavedIndex); \
    }
#else
#define READSCRATCHREGISTER2(pContext,result)  \
    { \
    /* save index reg setting*/ \
    FxU8 bSavedIndex = inp ((FxU16)(((PDEVTABLE)(pContext))->IoBase + 0xd4)); \
    /* return value from register shared with BIOS */ \
    outp ((FxU16)(((PDEVTABLE)(pContext))->IoBase + 0xd4), 0x1e);  /* select the reg */ \
    result = inp ((FxU16)(((PDEVTABLE)(pContext))->IoBase + 0xd5));  /* read the reg */ \
    /* restore index reg setting */ \
    outp ((FxU16)(((PDEVTABLE)(pContext))->IoBase + 0xd4), bSavedIndex); \
    }
#endif
/*----------------------------------------------------------------------
Macro name:  WriteScratchRegister2

Description:    write a byte value to BIOS scratch register 2  (location 0x1E)

Information:    
----------------------------------------------------------------------*/
#ifdef WINNT
#define WRITESCRATCHREGISTER2(pContext,value) \
{ \
    /* save index reg setting */ \
    FxU8 bSavedIndex = VideoPortReadPortUchar((PUCHAR)((PHW_DEVICE_EXTENSION)(pContext))->MappedAddress[IOBASE_ZERO] + 0xd4); \
    /* write value to register shared with BIOS */ \
    VideoPortWritePortUchar((PUCHAR)((PHW_DEVICE_EXTENSION)(pContext))->MappedAddress[IOBASE_ZERO] + 0xd4, 0x1e);  /* select the reg */ \
    VideoPortWritePortUchar((PUCHAR)((PHW_DEVICE_EXTENSION)(pContext))->MappedAddress[IOBASE_ZERO] + 0xd5,value);  /* write the reg */ \
    /* restore index reg setting */ \
    VideoPortWritePortUchar((PUCHAR)((PHW_DEVICE_EXTENSION)(pContext))->MappedAddress[IOBASE_ZERO] + 0xd4, bSavedIndex); \
}
#else
#define WRITESCRATCHREGISTER2(pContext,value) \
{ \
    /* save index reg setting */ \
    FxU8 bSavedIndex = inp ((FxU16)(((PDEVTABLE)(pContext))->IoBase + 0xd4)); \
    /* write value to register shared with BIOS */ \
    outp ((FxU16)(((PDEVTABLE)(pContext))->IoBase + 0xd4), 0x1e);  /* select the reg */ \
    outp ((FxU16)(((PDEVTABLE)(pContext))->IoBase + 0xd5), value);  /* write the reg */ \
    /* restore index register setting */ \
    outp ((FxU16)(((PDEVTABLE)(pContext))->IoBase + 0xd4), bSavedIndex); \
}
#endif

#endif // __BIOS_H__
