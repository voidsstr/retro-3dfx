/******************************Module*Header*******************************\
* Module Name: debug.c
*
* Debug helper routines.
*
* Copyright (c) 1992-1996 Microsoft Corporation
*
\**************************************************************************/

#include "precomp.h"

LONG UseCSIM = 1;    //  jdw - 0 selects GDI punt, 1 selects CSIM/H3 hardware

volatile ULONG	p6fenceLoc;		//  Dummy location used to fence P6 processors

#if DBG

// Helper variable to make sure PROLOG/EPILOGS match

LONG	prologEpilogPairing = 0;

////////////////////////////////////////////////////////////////////////////
// DEBUGGING INITIALIZATION CODE
//
// When you're bringing up your display for the first time, you can
// recompile with 'DebugLevel' set to 100.  That will cause absolutely
// all DISPDBG messages to be displayed on the kernel debugger (this
// is known as the "PrintF Approach to Debugging" and is about the only
// viable method for debugging driver initialization code).

LONG DebugLevel = 0;            // Set to '100' to debug initialization code
                                //   (the default is '0')

////////////////////////////////////////////////////////////////////////////

#define LARGE_LOOP_COUNT  10000000

////////////////////////////////////////////////////////////////////////////
// Miscellaneous Driver Debug Routines
////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 *
 *   Routine Description:
 *
 *      This function is variable-argument, level-sensitive debug print
 *      routine.
 *      If the specified debug level for the print statement is lower or equal
 *      to the current debug level, the message will be printed.
 *
 *   Arguments:
 *
 *      DebugPrintLevel - Specifies at which debugging level the string should
 *          be printed
 *
 *      DebugMessage - Variable argument ascii c string
 *
 *   Return Value:
 *
 *      None.
 *
 ***************************************************************************/

VOID
DebugPrint(
    LONG  DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    )
{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= DebugLevel)
    {
        EngDebugPrint(STANDARD_DEBUG_PREFIX, DebugMessage, ap);
        EngDebugPrint("", "\n", ap);
    }

    va_end(ap);

} // DebugPrint()

////////////////////////////////////////////////////////////////////////////

/******************************Public*Routine******************************\
* VOID vH3GpWait
\**************************************************************************/

VOID vH3GpWait(
PDEV*   ppdev,
BYTE*   pjH3Base)
{
    LONG    i;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!H3_GP_BUSY(ppdev, pjH3Base))
            return;         // It isn't busy
    }

    FLUSH_LOG_FILE_BUFFER(ppdev);

    RIP("vH3GpWait timeout -- The hardware is in a funky state.");
}

#define INPB(a)     _inp((a))
#define INPD(a)     _inpd((a))
#define OUTPB(a,b)  _outp((a),(b))
#define OUTPD(a,b)  _outpd((a),(b))

/*----------------------------------------------------------------------
Function name: 

Description:   

Return:        
----------------------------------------------------------------------*/

static VOID
MyDumpPCIConfigSpace(PDEV *ppdev, ULONG DebugLevel, PUCHAR pIO, SstIORegs *RegisterMap)
{
  ULONG         vgaInit0;
  unsigned int  regNum;
  UCHAR         PCIBuffer[256];


  DISPDBG((DebugLevel, "PCI Config Space using special access thru CR1C"));

  vgaInit0 = (int)RegisterMap->vgaInit0;
  RegisterMap->vgaInit0 &= 0xFFFFFF3F;
  for (regNum = 0; regNum < 64*4; regNum++)
  {
    OUTPB(pIO + 0xD4, 0x1C);
    OUTPB(pIO + 0xD5, (UCHAR)regNum);
    *(UCHAR *)((UCHAR *)&PCIBuffer + regNum) = INPB(pIO + 0xD5);
  }
  RegisterMap->vgaInit0 = (ULONG)vgaInit0;


  DISPDBG((DebugLevel, "  %02x: %04lx    ;VendorID=%04x",    0x00, *(USHORT *)&PCIBuffer[0x00], *(USHORT *)&PCIBuffer[0x00]));
  DISPDBG((DebugLevel, "  %02x: %04lx    ;DeviceID=%04x",    0x02, *(USHORT *)&PCIBuffer[0x02], *(USHORT *)&PCIBuffer[0x02]));
  DISPDBG((DebugLevel, "  %02x: %04lx    ;Command=",         0x04, *(USHORT *)&PCIBuffer[0x04]));
                                                                   //((*(USHORT *)&PCIBuffer[0x04] & 0x0002) : "MemSpaceEnable" ? ""),
                                                                   //((*(USHORT *)&PCIBuffer[0x04] & 0x0001) : "IOSpaceEnable"  ? "")));
  DISPDBG((DebugLevel, "  %02x: %04lx    ;Status=",          0x06, *(USHORT *)&PCIBuffer[0x06]));

  DISPDBG((DebugLevel, "  %02x: %02lx      ;RevisionID=%02x",  0x08, *(UCHAR  *)&PCIBuffer[0x08], *(UCHAR  *)&PCIBuffer[0x08]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;ProgIF=%02x",      0x09, *(UCHAR  *)&PCIBuffer[0x09], *(UCHAR  *)&PCIBuffer[0x09]));
                                                                   //"VGA compatible controller"));

  DISPDBG((DebugLevel, "  %02x: %02lx      ;SubClass=%02x",    0x0A, *(UCHAR  *)&PCIBuffer[0x0A], *(UCHAR  *)&PCIBuffer[0x0A]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;BaseClass=%02x",   0x0B, *(UCHAR  *)&PCIBuffer[0x0B], *(UCHAR  *)&PCIBuffer[0x0B]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;CacheLineSize=",   0x0C, *(UCHAR  *)&PCIBuffer[0x0C]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;LatencyTimer=%02x",0x0D, *(UCHAR  *)&PCIBuffer[0x0D], *(UCHAR  *)&PCIBuffer[0x0D]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;HeaderType=%02x",  0x0E, *(UCHAR  *)&PCIBuffer[0x0E], *(UCHAR  *)&PCIBuffer[0x0E]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;BIST=%02x",        0x0F, *(UCHAR  *)&PCIBuffer[0x0F], *(UCHAR  *)&PCIBuffer[0x0F]));

  DISPDBG((DebugLevel, "  %02x: %08lx;BAR0=%08x",        0x10, *(ULONG  *)&PCIBuffer[0x10], *(ULONG  *)&PCIBuffer[0x10]));
  DISPDBG((DebugLevel, "  %02x: %08lx;BAR1=%08x",        0x14, *(ULONG  *)&PCIBuffer[0x14], *(ULONG  *)&PCIBuffer[0x14]));
  DISPDBG((DebugLevel, "  %02x: %08lx;BAR2=%08x",        0x18, *(ULONG  *)&PCIBuffer[0x18], *(ULONG  *)&PCIBuffer[0x18]));
  DISPDBG((DebugLevel, "  %02x: %08lx;BAR3=%08x",        0x1C, *(ULONG  *)&PCIBuffer[0x1C], *(ULONG  *)&PCIBuffer[0x1C]));
  DISPDBG((DebugLevel, "  %02x: %08lx;BAR4=%08x",        0x20, *(ULONG  *)&PCIBuffer[0x20], *(ULONG  *)&PCIBuffer[0x20]));
  DISPDBG((DebugLevel, "  %02x: %08lx;BAR5=%08x",        0x24, *(ULONG  *)&PCIBuffer[0x24], *(ULONG  *)&PCIBuffer[0x24]));
  DISPDBG((DebugLevel, "  %02x: %08lx;CBCISPtr=%08x",    0x28, *(ULONG  *)&PCIBuffer[0x28], *(ULONG  *)&PCIBuffer[0x28]));

  DISPDBG((DebugLevel, "  %02x: %04lx    ;SubSysVenID=%04x", 0x2C, *(USHORT *)&PCIBuffer[0x2C], *(USHORT *)&PCIBuffer[0x2C]));
  DISPDBG((DebugLevel, "  %02x: %04lx    ;SusSysID=%04x",    0x2E, *(USHORT *)&PCIBuffer[0x2E], *(USHORT *)&PCIBuffer[0x2E]));

  DISPDBG((DebugLevel, "  %02x: %08lx;ROMBAR=%08x",      0x30, *(ULONG  *)&PCIBuffer[0x30], *(ULONG  *)&PCIBuffer[0x30]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;CapPtr=%02x",      0x34, *(UCHAR  *)&PCIBuffer[0x34], *(UCHAR  *)&PCIBuffer[0x34]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;Reserved=%02x",    0x35, *(UCHAR  *)&PCIBuffer[0x35], *(UCHAR  *)&PCIBuffer[0x35]));
  DISPDBG((DebugLevel, "  %02x: %04lx    ;Reserved=%04x",    0x36, *(USHORT *)&PCIBuffer[0x36], *(USHORT *)&PCIBuffer[0x36]));
  DISPDBG((DebugLevel, "  %02x: %08lx;Reserved=%08x",    0x38, *(ULONG  *)&PCIBuffer[0x38], *(ULONG  *)&PCIBuffer[0x38]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;IntLine=%02x",     0x3C, *(UCHAR  *)&PCIBuffer[0x3C], *(UCHAR  *)&PCIBuffer[0x3C]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;IntPin=%02x",      0x3D, *(UCHAR  *)&PCIBuffer[0x3D], *(UCHAR  *)&PCIBuffer[0x3D]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;MinGnt=%02x",      0x3E, *(UCHAR  *)&PCIBuffer[0x3E], *(UCHAR  *)&PCIBuffer[0x3E]));
  DISPDBG((DebugLevel, "  %02x: %02lx      ;MaxLat=%02x",      0x3F, *(UCHAR  *)&PCIBuffer[0x3F], *(UCHAR  *)&PCIBuffer[0x3F]));

  for (regNum = 0x40; regNum < 0x100; regNum += 0x20)
  {
    DISPDBG((DebugLevel, "  %02x: %08lx,%08lx,%08lx,%08lx,%08lx,%08lx,%08lx,%08lx",
             regNum,
             *(ULONG  *)&PCIBuffer[regNum],
             *(ULONG  *)&PCIBuffer[regNum+0x04],
             *(ULONG  *)&PCIBuffer[regNum+0x08],
             *(ULONG  *)&PCIBuffer[regNum+0x0C],
             *(ULONG  *)&PCIBuffer[regNum+0x10],
             *(ULONG  *)&PCIBuffer[regNum+0x14],
             *(ULONG  *)&PCIBuffer[regNum+0x18],
             *(ULONG  *)&PCIBuffer[regNum+0x1C]));
  }
}

/*----------------------------------------------------------------------
Function name: 

Description:   

Return:        
----------------------------------------------------------------------*/

static VOID
DumpH3RegsForChip(PDEV *ppdev, ULONG DebugLevel,
                  PUCHAR pIO, SstIORegs *RegisterMap,
                  SstGRegs *RegisterMap2D, SstRegs *RegisterMap3D)
{
  int i;


  //MyDumpPCIConfigSpace(ppdev, DebugLevel, pIO, RegisterMap);



#if 0
  DISPDBG((DebugLevel, "IO Base0 Regs"));

  DISPDBG((DebugLevel, "  STATUS                       = %08lXh", INPD(pIO + STATUS                      )));
  DISPDBG((DebugLevel, "  PCIINIT0                     = %08lXh", INPD(pIO + PCIINIT0                    )));
  DISPDBG((DebugLevel, "  SIPMONITOR                   = %08lXh", INPD(pIO + SIPMONITOR                  )));
  DISPDBG((DebugLevel, "  LFBMEMORYCONFIG              = %08lXh", INPD(pIO + LFBMEMORYCONFIG             )));
  DISPDBG((DebugLevel, "  MISCINIT0                    = %08lXh", INPD(pIO + MISCINIT0                   )));
  DISPDBG((DebugLevel, "  MISCINIT1                    = %08lXh", INPD(pIO + MISCINIT1                   )));
  DISPDBG((DebugLevel, "  DRAMINIT0                    = %08lXh", INPD(pIO + DRAMINIT0                   )));
  DISPDBG((DebugLevel, "  DRAMINIT1                    = %08lXh", INPD(pIO + DRAMINIT1                   )));
  DISPDBG((DebugLevel, "  AGPINIT                      = %08lXh", INPD(pIO + AGPINIT                     )));
  DISPDBG((DebugLevel, "  TMUGBEINIT                   = %08lXh", INPD(pIO + TMUGBEINIT                  )));
  DISPDBG((DebugLevel, "  VGAINIT0                     = %08lXh", INPD(pIO + VGAINIT0                    )));
  DISPDBG((DebugLevel, "  VGAINIT1                     = %08lXh", INPD(pIO + VGAINIT1                    )));
  DISPDBG((DebugLevel, "  DRAMCOMMAND                  = %08lXh", INPD(pIO + DRAMCOMMAND                 )));
  DISPDBG((DebugLevel, "  DRAMDATA                     = %08lXh", INPD(pIO + DRAMDATA                    )));
  DISPDBG((DebugLevel, "  RESERVEDZ_0                  = %08lXh", INPD(pIO + RESERVEDZ_0                 )));
  DISPDBG((DebugLevel, "  RESERVEDZ_1                  = %08lXh", INPD(pIO + RESERVEDZ_1                 )));
  DISPDBG((DebugLevel, "  PLLCTRL0                     = %08lXh", INPD(pIO + PLLCTRL0                    )));
  DISPDBG((DebugLevel, "  PLLCTRL1                     = %08lXh", INPD(pIO + PLLCTRL1                    )));
  DISPDBG((DebugLevel, "  PLLCTRL2                     = %08lXh", INPD(pIO + PLLCTRL2                    )));
  DISPDBG((DebugLevel, "  DACMODE                      = %08lXh", INPD(pIO + DACMODE                     )));
  DISPDBG((DebugLevel, "  DACADDR                      = %08lXh", INPD(pIO + DACADDR                     )));
  DISPDBG((DebugLevel, "  DACDATA                      = %08lXh", INPD(pIO + DACDATA                     )));
  DISPDBG((DebugLevel, "  VIDMAXRGBDELTA               = %08lXh", INPD(pIO + VIDMAXRGBDELTA              )));
  DISPDBG((DebugLevel, "  VIDPROCCFG                   = %08lXh", INPD(pIO + VIDPROCCFG                  )));
  DISPDBG((DebugLevel, "  HWCURPATADDR                 = %08lXh", INPD(pIO + HWCURPATADDR                )));
  DISPDBG((DebugLevel, "  HWCURLOC                     = %08lXh", INPD(pIO + HWCURLOC                    )));
  DISPDBG((DebugLevel, "  HWCURC0                      = %08lXh", INPD(pIO + HWCURC0                     )));
  DISPDBG((DebugLevel, "  HWCURC1                      = %08lXh", INPD(pIO + HWCURC1                     )));
  DISPDBG((DebugLevel, "  VIDINFORMAT                  = %08lXh", INPD(pIO + VIDINFORMAT                 )));
  DISPDBG((DebugLevel, "  VIDINSTATUS                  = %08lXh", INPD(pIO + VIDINSTATUS                 )));
  DISPDBG((DebugLevel, "  VIDSERIALPARALLELPORT        = %08lXh", INPD(pIO + VIDSERIALPARALLELPORT       )));
  DISPDBG((DebugLevel, "  VIDINXDECIMDELTAS            = %08lXh", INPD(pIO + VIDINXDECIMDELTAS           )));
  DISPDBG((DebugLevel, "  VIDINDECIMINITERRS           = %08lXh", INPD(pIO + VIDINDECIMINITERRS          )));
  DISPDBG((DebugLevel, "  VIDINYDECIMDELTAS            = %08lXh", INPD(pIO + VIDINYDECIMDELTAS           )));
  DISPDBG((DebugLevel, "  VIDPIXELBUFTHOLD             = %08lXh", INPD(pIO + VIDPIXELBUFTHOLD            )));
  DISPDBG((DebugLevel, "  VIDCHROMAMIN                 = %08lXh", INPD(pIO + VIDCHROMAMIN                )));
  DISPDBG((DebugLevel, "  VIDCHROMAMAX                 = %08lXh", INPD(pIO + VIDCHROMAMAX                )));
  DISPDBG((DebugLevel, "  VIDCURRENTLINE               = %08lXh", INPD(pIO + VIDCURRENTLINE              )));
  DISPDBG((DebugLevel, "  VIDSCREENSIZE                = %08lXh", INPD(pIO + VIDSCREENSIZE               )));
  DISPDBG((DebugLevel, "  VIDOVERLAYSTARTCOORDS        = %08lXh", INPD(pIO + VIDOVERLAYSTARTCOORDS       )));
  DISPDBG((DebugLevel, "  VIDOVERLAYENDCOORDS          = %08lXh", INPD(pIO + VIDOVERLAYENDCOORDS         )));
  DISPDBG((DebugLevel, "  VIDOVERLAYDUDX               = %08lXh", INPD(pIO + VIDOVERLAYDUDX              )));
  DISPDBG((DebugLevel, "  VIDOVERLAYDUDXOFFSETSRCWIDTH = %08lXh", INPD(pIO + VIDOVERLAYDUDXOFFSETSRCWIDTH)));
  DISPDBG((DebugLevel, "  VIDOVERLAYDVDY               = %08lXh", INPD(pIO + VIDOVERLAYDVDY              )));

  DISPDBG((DebugLevel, "  VIDOVERLAYDVDYOFFSET         = %08lXh", INPD(pIO + VIDOVERLAYDVDYOFFSET        )));
  DISPDBG((DebugLevel, "  VIDDESKTOPSTARTADDR          = %08lXh", INPD(pIO + VIDDESKTOPSTARTADDR         )));
  DISPDBG((DebugLevel, "  VIDDESKTOPOVERLAYSTRIDE      = %08lXh", INPD(pIO + VIDDESKTOPOVERLAYSTRIDE     )));
  DISPDBG((DebugLevel, "  VIDINADDR0                   = %08lXh", INPD(pIO + VIDINADDR0                  )));
  DISPDBG((DebugLevel, "  VIDINADDR1                   = %08lXh", INPD(pIO + VIDINADDR1                  )));
  DISPDBG((DebugLevel, "  VIDINADDR2                   = %08lXh", INPD(pIO + VIDINADDR2                  )));
  DISPDBG((DebugLevel, "  VIDINSTRIDE                  = %08lXh", INPD(pIO + VIDINSTRIDE                 )));
  DISPDBG((DebugLevel, "  VIDCURROVERLAYSTARTADDR      = %08lXh", INPD(pIO + VIDCURROVERLAYSTARTADDR     )));
#endif



  DISPDBG((DebugLevel, "General Regs"));

  DISPDBG((DebugLevel, "  MiscOutput         = %02Xh", INPB(pIO + 0xCC)));
  DISPDBG((DebugLevel, "  Input Status 0     = %02Xh", INPB(pIO + 0xC2)));
  DISPDBG((DebugLevel, "  Input Status 1     = %02Xh", INPB(pIO + 0xDA)));
  DISPDBG((DebugLevel, "  Feature Control    = %02Xh", INPB(pIO + 0xCA)));
  DISPDBG((DebugLevel, "  Motherboard Enable = %02Xh", INPB(pIO + 0xC3)));
  //DISPDBG((DebugLevel, "  Adapter Enable     = %02Xh", INPB(pIO + ??)));
  DISPDBG((DebugLevel, "  Subsystem Enable   = %02Xh", INPB(pIO + 0xCE)));



  DISPDBG((DebugLevel, "CRTC Regs"));

  for (i = 0; i <= 0x18; i++)
  {
    OUTPB(pIO + 0xD4, i);
    DISPDBG((DebugLevel, "  Index %02Xh = %02Xh", i, INPB(pIO + 0xD5)));
  }
  for (i = 0x1A; i <= 0x1F; i++)
  {
    OUTPB(pIO + 0xD4, i);
    DISPDBG((DebugLevel, "  Index %02Xh = %02Xh", i, INPB(pIO + 0xD5)));
  }
  OUTPB(pIO + 0xD4, 0x22);
  DISPDBG((DebugLevel, "  Index 22h = %02Xh", i, INPB(pIO + 0xD5)));
  OUTPB(pIO + 0xD4, 0x24);
  DISPDBG((DebugLevel, "  Index 24h = %02Xh", i, INPB(pIO + 0xD5)));
  OUTPB(pIO + 0xD4, 0x26);
  DISPDBG((DebugLevel, "  Index 26h = %02Xh", i, INPB(pIO + 0xD5)));



  DISPDBG((DebugLevel, "Sequencer Regs"));

  for (i = 0; i <= 0x4; i++)
  {
    OUTPB(pIO + 0xC4, i);
    DISPDBG((DebugLevel, "  Index %02Xh = %02Xh", i, INPB(pIO + 0xC5)));
  }



  DISPDBG((DebugLevel, "Graphics Controller Regs"));

  for (i = 0; i <= 0x8; i++)
  {
    OUTPB(pIO + 0xCE, i);
    DISPDBG((DebugLevel, "  Index %02Xh = %02Xh", i, INPB(pIO + 0xCF)));
  }



#if 0
  DISPDBG((DebugLevel, "Attribute Controller Regs"));

  for (i = 0; i <= 0x14; i++)
  {
    OUTPB(pIO + 0xC0, i);
    DISPDBG((DebugLevel, "  Index %02Xh = %02Xh", i, INPB(pIO + 0xC0)));
  }
#endif



  DISPDBG((DebugLevel, "MemBase0 Regs"));

  DISPDBG((DebugLevel, "  status                       = %08lXh", RegisterMap->status                      ));
  DISPDBG((DebugLevel, "  pciInit0                     = %08lXh", RegisterMap->pciInit0                    ));
  DISPDBG((DebugLevel, "  sipMonitor                   = %08lXh", RegisterMap->sipMonitor                  ));
  DISPDBG((DebugLevel, "  lfbMemoryConfig              = %08lXh", RegisterMap->lfbMemoryConfig             ));
  DISPDBG((DebugLevel, "  miscInit0                    = %08lXh", RegisterMap->miscInit0                   ));
  DISPDBG((DebugLevel, "  miscInit1                    = %08lXh", RegisterMap->miscInit1                   ));
  DISPDBG((DebugLevel, "  dramInit0                    = %08lXh", RegisterMap->dramInit0                   ));
  DISPDBG((DebugLevel, "  dramInit1                    = %08lXh", RegisterMap->dramInit1                   ));
  DISPDBG((DebugLevel, "  agpInit                      = %08lXh", RegisterMap->agpInit                     ));
  DISPDBG((DebugLevel, "  tmuGbeInit                   = %08lXh", RegisterMap->tmuGbeInit                  ));
  DISPDBG((DebugLevel, "  vgaInit0                     = %08lXh", RegisterMap->vgaInit0                    ));
  DISPDBG((DebugLevel, "  vgaInit1                     = %08lXh", RegisterMap->vgaInit1                    ));
  DISPDBG((DebugLevel, "  dramCommand                  = %08lXh", RegisterMap->dramCommand                 ));
  DISPDBG((DebugLevel, "  dramData                     = %08lXh", RegisterMap->dramData                    ));
  DISPDBG((DebugLevel, "  strapInfo                    = %08lXh", RegisterMap->strapInfo                   ));
#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
  DISPDBG((DebugLevel, "  vidTvOutBlankVCount          = %08lXh", RegisterMap->vidTvOutBlankVCount         ));
#else
  DISPDBG((DebugLevel, "  _reserved01                  = %08lXh", RegisterMap->_reserved01                 ));
#endif
  DISPDBG((DebugLevel, "  pllCtrl0                     = %08lXh", RegisterMap->pllCtrl0                    ));
  DISPDBG((DebugLevel, "  pllCtrl1                     = %08lXh", RegisterMap->pllCtrl1                    ));
  DISPDBG((DebugLevel, "  pllCtrl2                     = %08lXh", RegisterMap->pllCtrl2                    ));
  DISPDBG((DebugLevel, "  dacMode                      = %08lXh", RegisterMap->dacMode                     ));
  DISPDBG((DebugLevel, "  dacAddr                      = %08lXh", RegisterMap->dacAddr                     ));
  DISPDBG((DebugLevel, "  dacData                      = %08lXh", RegisterMap->dacData                     ));
  DISPDBG((DebugLevel, "  vidMaxRGBDelta               = %08lXh", RegisterMap->vidMaxRGBDelta              ));
  DISPDBG((DebugLevel, "  vidProcCfg                   = %08lXh", RegisterMap->vidProcCfg                  ));
  DISPDBG((DebugLevel, "  hwCurPatAddr                 = %08lXh", RegisterMap->hwCurPatAddr                ));
  DISPDBG((DebugLevel, "  hwCurLoc                     = %08lXh", RegisterMap->hwCurLoc                    ));
  DISPDBG((DebugLevel, "  hwCurC0                      = %08lXh", RegisterMap->hwCurC0                     ));
  DISPDBG((DebugLevel, "  hwCurC1                      = %08lXh", RegisterMap->hwCurC1                     ));
  DISPDBG((DebugLevel, "  vidInFormat                  = %08lXh", RegisterMap->vidInFormat                 ));
#ifdef TVOUT_SUPPORTED
  DISPDBG((DebugLevel, "  vidTvOutBlankHCount          = %08lXh", RegisterMap->vidTvOutBlankHCount         ));
#else
  DISPDBG((DebugLevel, "  vidInStatus                  = %08lXh", RegisterMap->vidInStatus                 ));
#endif
  DISPDBG((DebugLevel, "  vidSerialParallelPort        = %08lXh", RegisterMap->vidSerialParallelPort       ));
  DISPDBG((DebugLevel, "  vidInXDecimDeltas            = %08lXh", RegisterMap->vidInXDecimDeltas           ));
  DISPDBG((DebugLevel, "  vidInDecimInitErrs           = %08lXh", RegisterMap->vidInDecimInitErrs          ));
  DISPDBG((DebugLevel, "  vidInYDecimDeltas            = %08lXh", RegisterMap->vidInYDecimDeltas           ));
  DISPDBG((DebugLevel, "  vidPixelBufThold             = %08lXh", RegisterMap->vidPixelBufThold            ));
  DISPDBG((DebugLevel, "  vidChromaMin                 = %08lXh", RegisterMap->vidChromaMin                ));
  DISPDBG((DebugLevel, "  vidChromaMax                 = %08lXh", RegisterMap->vidChromaMax                ));
  DISPDBG((DebugLevel, "  vidCurrentLine               = %08lXh", RegisterMap->vidCurrentLine              ));
  DISPDBG((DebugLevel, "  vidScreenSize                = %08lXh", RegisterMap->vidScreenSize               ));
  DISPDBG((DebugLevel, "  vidOverlayStartCoords        = %08lXh", RegisterMap->vidOverlayStartCoords       ));
  DISPDBG((DebugLevel, "  vidOverlayEndScreenCoord     = %08lXh", RegisterMap->vidOverlayEndScreenCoord    ));
  DISPDBG((DebugLevel, "  vidOverlayDudx               = %08lXh", RegisterMap->vidOverlayDudx              ));
  DISPDBG((DebugLevel, "  vidOverlayDudxOffsetSrcWidth = %08lXh", RegisterMap->vidOverlayDudxOffsetSrcWidth));
  DISPDBG((DebugLevel, "  vidOverlayDvdy               = %08lXh", RegisterMap->vidOverlayDvdy              ));
  DISPDBG((DebugLevel, "  vidOverlayDvdyOffset         = %08lXh", RegisterMap->vidOverlayDvdyOffset        ));
  DISPDBG((DebugLevel, "  vidDesktopStartAddr          = %08lXh", RegisterMap->vidDesktopStartAddr         ));
  DISPDBG((DebugLevel, "  vidDesktopOverlayStride      = %08lXh", RegisterMap->vidDesktopOverlayStride     ));
  DISPDBG((DebugLevel, "  vidInAddr0                   = %08lXh", RegisterMap->vidInAddr0                  ));
  DISPDBG((DebugLevel, "  vidInAddr1                   = %08lXh", RegisterMap->vidInAddr1                  ));
  DISPDBG((DebugLevel, "  vidInAddr2                   = %08lXh", RegisterMap->vidInAddr2                  ));
  DISPDBG((DebugLevel, "  vidInStride                  = %08lXh", RegisterMap->vidInStride                 ));
  DISPDBG((DebugLevel, "  vidCurrOverlayStartAddr      = %08lXh", RegisterMap->vidCurrOverlayStartAddr     ));



  DISPDBG((DebugLevel, "2D Regs"));

  DISPDBG((DebugLevel, "  status           = %08lXh", RegisterMap2D->status          ));
  DISPDBG((DebugLevel, "  unused           = %08lXh", RegisterMap2D->unused0         ));
  DISPDBG((DebugLevel, "  clip0Min         = %08lXh", RegisterMap2D->clip0min        ));
  DISPDBG((DebugLevel, "  clip0Max         = %08lXh", RegisterMap2D->clip0max        ));
  DISPDBG((DebugLevel, "  dstBaseAddr      = %08lXh", RegisterMap2D->dstBaseAddr     ));
  DISPDBG((DebugLevel, "  dstFormat        = %08lXh", RegisterMap2D->dstFormat       ));
  DISPDBG((DebugLevel, "  srcColorkeyMin   = %08lXh", RegisterMap2D->srcColorkeyMin  ));
  DISPDBG((DebugLevel, "  srcColorkeyMax   = %08lXh", RegisterMap2D->srcColorkeyMax  ));
  DISPDBG((DebugLevel, "  dstColorkeyMin   = %08lXh", RegisterMap2D->dstColorkeyMin  ));
  DISPDBG((DebugLevel, "  dstColorkeyMax   = %08lXh", RegisterMap2D->dstColorkeyMax  ));
  DISPDBG((DebugLevel, "  bresError0       = %08lXh", RegisterMap2D->bresError0      ));
  DISPDBG((DebugLevel, "  bresError1       = %08lXh", RegisterMap2D->bresError1      ));
  DISPDBG((DebugLevel, "  rop              = %08lXh", RegisterMap2D->rop             ));
  DISPDBG((DebugLevel, "  srcBaseAddr      = %08lXh", RegisterMap2D->srcBaseAddr     ));
  DISPDBG((DebugLevel, "  commandEx        = %08lXh", RegisterMap2D->commandEx       ));
  DISPDBG((DebugLevel, "  lineStipple      = %08lXh", RegisterMap2D->lineStipple     ));
  DISPDBG((DebugLevel, "  lineStyle        = %08lXh", RegisterMap2D->lineStyle       ));
  DISPDBG((DebugLevel, "  pattern0alias    = %08lXh", RegisterMap2D->pattern0alias   ));
  DISPDBG((DebugLevel, "  pattern1alias    = %08lXh", RegisterMap2D->pattern1alias   ));
  DISPDBG((DebugLevel, "  clip1min         = %08lXh", RegisterMap2D->clip1min        ));
  DISPDBG((DebugLevel, "  clip1max         = %08lXh", RegisterMap2D->clip1max        ));
  DISPDBG((DebugLevel, "  srcFormat        = %08lXh", RegisterMap2D->srcFormat       ));
  DISPDBG((DebugLevel, "  srcSize          = %08lXh", RegisterMap2D->srcSize         ));
  DISPDBG((DebugLevel, "  srcXY            = %08lXh", RegisterMap2D->srcXY           ));
  DISPDBG((DebugLevel, "  colorBack        = %08lXh", RegisterMap2D->colorBack       ));
  DISPDBG((DebugLevel, "  colorFore        = %08lXh", RegisterMap2D->colorFore       ));
  DISPDBG((DebugLevel, "  dstSize          = %08lXh", RegisterMap2D->dstSize         ));
  DISPDBG((DebugLevel, "  dstXY            = %08lXh", RegisterMap2D->dstXY           ));
  DISPDBG((DebugLevel, "  command          = %08lXh", RegisterMap2D->command         ));
#if 0
  for (i = 0; i < 16; i++)
  {
    DISPDBG((DebugLevel, "  launchArea[%02ld]   = %08lXh", i, RegisterMap2D->launchArea[16]  ));
  }
#endif
  for (i = 0; i < 32; i++)
  {
    DISPDBG((DebugLevel, "  colorPattern[%02ld] = %08lXh", i, RegisterMap2D->colorPattern[32]));
  }



#if 0
  DISPDBG((DebugLevel, "3D regs"));

  DISPDBG((DebugLevel, "  status             = %08lXh", RegisterMap3D->status              ));
  DISPDBG((DebugLevel, "  intrCtrl           = %08lXh", RegisterMap3D->intrCtrl            ));
  DISPDBG((DebugLevel, "  vA.x               = %08lXh", RegisterMap3D->vA.x                ));
  DISPDBG((DebugLevel, "  vA.y               = %08lXh", RegisterMap3D->vA.y                ));
  DISPDBG((DebugLevel, "  vB.x               = %08lXh", RegisterMap3D->vB.x                ));
  DISPDBG((DebugLevel, "  vB.y               = %08lXh", RegisterMap3D->vB.y                ));
  DISPDBG((DebugLevel, "  vC.x               = %08lXh", RegisterMap3D->vC.x                ));
  DISPDBG((DebugLevel, "  vC.y               = %08lXh", RegisterMap3D->vC.y                ));
  DISPDBG((DebugLevel, "  r                  = %08lXh", RegisterMap3D->r                   ));
  DISPDBG((DebugLevel, "  g                  = %08lXh", RegisterMap3D->g                   ));
  DISPDBG((DebugLevel, "  b                  = %08lXh", RegisterMap3D->b                   ));
  DISPDBG((DebugLevel, "  z                  = %08lXh", RegisterMap3D->z                   ));
  DISPDBG((DebugLevel, "  s                  = %08lXh", RegisterMap3D->s                   ));
  DISPDBG((DebugLevel, "  t                  = %08lXh", RegisterMap3D->t                   ));
  DISPDBG((DebugLevel, "  a                  = %08lXh", RegisterMap3D->a                   ));
  DISPDBG((DebugLevel, "  w                  = %08lXh", RegisterMap3D->w                   ));
  DISPDBG((DebugLevel, "  drdx               = %08lXh", RegisterMap3D->drdx                ));
  DISPDBG((DebugLevel, "  dgdx               = %08lXh", RegisterMap3D->dgdx                ));
  DISPDBG((DebugLevel, "  dbdx               = %08lXh", RegisterMap3D->dbdx                ));
  DISPDBG((DebugLevel, "  dzdx               = %08lXh", RegisterMap3D->dzdx                ));
  DISPDBG((DebugLevel, "  dadx               = %08lXh", RegisterMap3D->dadx                ));
  DISPDBG((DebugLevel, "  dsdx               = %08lXh", RegisterMap3D->dsdx                ));
  DISPDBG((DebugLevel, "  dtdx               = %08lXh", RegisterMap3D->dtdx                ));
  DISPDBG((DebugLevel, "  dwdx               = %08lXh", RegisterMap3D->dwdx                ));
  DISPDBG((DebugLevel, "  drdy               = %08lXh", RegisterMap3D->drdy                ));
  DISPDBG((DebugLevel, "  dgdy               = %08lXh", RegisterMap3D->dgdy                ));
  DISPDBG((DebugLevel, "  dbdy               = %08lXh", RegisterMap3D->dbdy                ));
  DISPDBG((DebugLevel, "  dzdy               = %08lXh", RegisterMap3D->dzdy                ));
  DISPDBG((DebugLevel, "  dady               = %08lXh", RegisterMap3D->dady                ));
  DISPDBG((DebugLevel, "  dsdy               = %08lXh", RegisterMap3D->dsdy                ));
  DISPDBG((DebugLevel, "  dtdy               = %08lXh", RegisterMap3D->dtdy                ));
  DISPDBG((DebugLevel, "  dwdy               = %08lXh", RegisterMap3D->dwdy                ));
  DISPDBG((DebugLevel, "  triangleCMD        = %08lXh", RegisterMap3D->triangleCMD         ));
  DISPDBG((DebugLevel, "  FvA.x              = %08lXh", RegisterMap3D->FvA.x               ));
  DISPDBG((DebugLevel, "  FvA.y              = %08lXh", RegisterMap3D->FvA.y               ));
  DISPDBG((DebugLevel, "  FvB.x              = %08lXh", RegisterMap3D->FvB.x               ));
  DISPDBG((DebugLevel, "  FvB.y              = %08lXh", RegisterMap3D->FvB.y               ));
  DISPDBG((DebugLevel, "  FvC.x              = %08lXh", RegisterMap3D->FvC.x               ));
  DISPDBG((DebugLevel, "  FvC.y              = %08lXh", RegisterMap3D->FvC.y               ));
  DISPDBG((DebugLevel, "  Fr                 = %08lXh", RegisterMap3D->Fr                  ));
  DISPDBG((DebugLevel, "  Fg                 = %08lXh", RegisterMap3D->Fg                  ));
  DISPDBG((DebugLevel, "  Fb                 = %08lXh", RegisterMap3D->Fb                  ));
  DISPDBG((DebugLevel, "  Fz                 = %08lXh", RegisterMap3D->Fz                  ));
  DISPDBG((DebugLevel, "  Fs                 = %08lXh", RegisterMap3D->Fs                  ));
  DISPDBG((DebugLevel, "  Ft                 = %08lXh", RegisterMap3D->Ft                  ));
  DISPDBG((DebugLevel, "  Fa                 = %08lXh", RegisterMap3D->Fa                  ));
  DISPDBG((DebugLevel, "  Fw                 = %08lXh", RegisterMap3D->Fw                  ));
  DISPDBG((DebugLevel, "  Fdrdx              = %08lXh", RegisterMap3D->Fdrdx               ));
  DISPDBG((DebugLevel, "  Fdgdx              = %08lXh", RegisterMap3D->Fdgdx               ));
  DISPDBG((DebugLevel, "  Fdbdx              = %08lXh", RegisterMap3D->Fdbdx               ));
  DISPDBG((DebugLevel, "  Fdzdx              = %08lXh", RegisterMap3D->Fdzdx               ));
  DISPDBG((DebugLevel, "  Fdadx              = %08lXh", RegisterMap3D->Fdadx               ));
  DISPDBG((DebugLevel, "  Fdsdx              = %08lXh", RegisterMap3D->Fdsdx               ));
  DISPDBG((DebugLevel, "  Fdtdx              = %08lXh", RegisterMap3D->Fdtdx               ));
  DISPDBG((DebugLevel, "  Fdwdx              = %08lXh", RegisterMap3D->Fdwdx               ));
  DISPDBG((DebugLevel, "  Fdrdy              = %08lXh", RegisterMap3D->Fdrdy               ));
  DISPDBG((DebugLevel, "  Fdgdy              = %08lXh", RegisterMap3D->Fdgdy               ));
  DISPDBG((DebugLevel, "  Fdbdy              = %08lXh", RegisterMap3D->Fdbdy               ));
  DISPDBG((DebugLevel, "  Fdzdy              = %08lXh", RegisterMap3D->Fdzdy               ));
  DISPDBG((DebugLevel, "  Fdady              = %08lXh", RegisterMap3D->Fdady               ));
  DISPDBG((DebugLevel, "  Fdsdy              = %08lXh", RegisterMap3D->Fdsdy               ));
  DISPDBG((DebugLevel, "  Fdtdy              = %08lXh", RegisterMap3D->Fdtdy               ));
  DISPDBG((DebugLevel, "  Fdwdy              = %08lXh", RegisterMap3D->Fdwdy               ));
  DISPDBG((DebugLevel, "  FtriangleCMD       = %08lXh", RegisterMap3D->FtriangleCMD        ));
  DISPDBG((DebugLevel, "  fbzColorPath       = %08lXh", RegisterMap3D->fbzColorPath        ));
  DISPDBG((DebugLevel, "  fogMode            = %08lXh", RegisterMap3D->fogMode             ));
  DISPDBG((DebugLevel, "  alphaMode          = %08lXh", RegisterMap3D->alphaMode           ));
  DISPDBG((DebugLevel, "  fbzMode            = %08lXh", RegisterMap3D->fbzMode             ));
  DISPDBG((DebugLevel, "  lfbMode            = %08lXh", RegisterMap3D->lfbMode             ));
  DISPDBG((DebugLevel, "  clipLeftRight      = %08lXh", RegisterMap3D->clipLeftRight       ));
  DISPDBG((DebugLevel, "  clipTopBottom      = %08lXh", RegisterMap3D->clipTopBottom       ));
  DISPDBG((DebugLevel, "  nopCMD             = %08lXh", RegisterMap3D->nopCMD              ));
  DISPDBG((DebugLevel, "  fastfillCMD        = %08lXh", RegisterMap3D->fastfillCMD         ));
  DISPDBG((DebugLevel, "  swapbufferCMD      = %08lXh", RegisterMap3D->swapbufferCMD       ));
  DISPDBG((DebugLevel, "  fogColor           = %08lXh", RegisterMap3D->fogColor            ));
  DISPDBG((DebugLevel, "  zaColor            = %08lXh", RegisterMap3D->zaColor             ));
  DISPDBG((DebugLevel, "  chromaKey          = %08lXh", RegisterMap3D->chromaKey           ));
  DISPDBG((DebugLevel, "  chromaRange        = %08lXh", RegisterMap3D->chromaRange         ));
  DISPDBG((DebugLevel, "  userIntrCmd        = %08lXh", RegisterMap3D->userIntrCmd         ));
  DISPDBG((DebugLevel, "  stipple            = %08lXh", RegisterMap3D->stipple             ));
  DISPDBG((DebugLevel, "  c0                 = %08lXh", RegisterMap3D->c0                  ));
  DISPDBG((DebugLevel, "  c1                 = %08lXh", RegisterMap3D->c1                  ));
  DISPDBG((DebugLevel, "  stats.fbiPixelsIn  = %08lXh", RegisterMap3D->stats.fbiPixelsIn   ));
  DISPDBG((DebugLevel, "  stats.fbiChromaFail= %08lXh", RegisterMap3D->stats.fbiChromaFail ));
  DISPDBG((DebugLevel, "  stats.fbiZfuncFail = %08lXh", RegisterMap3D->stats.fbiZfuncFail  ));
  DISPDBG((DebugLevel, "  stats.fbiAfuncFail = %08lXh", RegisterMap3D->stats.fbiAfuncFail  ));
  DISPDBG((DebugLevel, "  stats.fbiPixelsOut = %08lXh", RegisterMap3D->stats.fbiPixelsOut  ));
  for (i = 0; i < 32; i++)
  {
    DISPDBG((DebugLevel, "  fogTable[%02ld]       = %08lXh", i, RegisterMap3D->fogTable[32]));
  }
  DISPDBG((DebugLevel, "  renderMode         = %08lXh", RegisterMap3D->renderMode          ));
  DISPDBG((DebugLevel, "  stencilMode        = %08lXh", RegisterMap3D->stencilMode         ));
  DISPDBG((DebugLevel, "  stencilOp          = %08lXh", RegisterMap3D->stencilOp           ));
  DISPDBG((DebugLevel, "  colBufferAddr      = %08lXh", RegisterMap3D->colBufferAddr       ));
  DISPDBG((DebugLevel, "  colBufferStride    = %08lXh", RegisterMap3D->colBufferStride     ));
  DISPDBG((DebugLevel, "  auxBufferAddr      = %08lXh", RegisterMap3D->auxBufferAddr       ));
  DISPDBG((DebugLevel, "  auxBufferStride    = %08lXh", RegisterMap3D->auxBufferStride     ));
  DISPDBG((DebugLevel, "  clipLeftRight1     = %08lXh", RegisterMap3D->clipLeftRight1      ));
  DISPDBG((DebugLevel, "  clipTopBottom1     = %08lXh", RegisterMap3D->clipTopBottom1      ));
  DISPDBG((DebugLevel, "  combineMode        = %08lXh", RegisterMap3D->combineMode         ));
  DISPDBG((DebugLevel, "  sliCtrl            = %08lXh", RegisterMap3D->sliCtrl             ));
  DISPDBG((DebugLevel, "  aaCtrl             = %08lXh", RegisterMap3D->aaCtrl              ));
  DISPDBG((DebugLevel, "  chipMask           = %08lXh", RegisterMap3D->chipMask            ));
  DISPDBG((DebugLevel, "  leftDesktopBuf     = %08lXh", RegisterMap3D->leftDesktopBuf      ));
  DISPDBG((DebugLevel, "  swapBufferPend     = %08lXh", RegisterMap3D->swapBufferPend      ));
  DISPDBG((DebugLevel, "  leftOverlayBuf     = %08lXh", RegisterMap3D->leftOverlayBuf      ));
  DISPDBG((DebugLevel, "  rightOverlayBuf    = %08lXh", RegisterMap3D->rightOverlayBuf     ));
  DISPDBG((DebugLevel, "  fbiSwapHistory     = %08lXh", RegisterMap3D->fbiSwapHistory      ));
  DISPDBG((DebugLevel, "  fbiTrianglesOut    = %08lXh", RegisterMap3D->fbiTrianglesOut     ));
  DISPDBG((DebugLevel, "  sSetupMode         = %08lXh", RegisterMap3D->sSetupMode          ));
  DISPDBG((DebugLevel, "  sVx                = %08lXh", RegisterMap3D->sVx                 ));
  DISPDBG((DebugLevel, "  sVy                = %08lXh", RegisterMap3D->sVy                 ));
  DISPDBG((DebugLevel, "  sARGB              = %08lXh", RegisterMap3D->sARGB               ));
  DISPDBG((DebugLevel, "  sRed               = %08lXh", RegisterMap3D->sRed                ));
  DISPDBG((DebugLevel, "  sGreen             = %08lXh", RegisterMap3D->sGreen              ));
  DISPDBG((DebugLevel, "  sBlue              = %08lXh", RegisterMap3D->sBlue               ));
  DISPDBG((DebugLevel, "  sAlpha             = %08lXh", RegisterMap3D->sAlpha              ));
  DISPDBG((DebugLevel, "  sVz                = %08lXh", RegisterMap3D->sVz                 ));
  DISPDBG((DebugLevel, "  sOowfbi            = %08lXh", RegisterMap3D->sOowfbi             ));
  DISPDBG((DebugLevel, "  sOow0              = %08lXh", RegisterMap3D->sOow0               ));
  DISPDBG((DebugLevel, "  sSow0              = %08lXh", RegisterMap3D->sSow0               ));
  DISPDBG((DebugLevel, "  sTow0              = %08lXh", RegisterMap3D->sTow0               ));
  DISPDBG((DebugLevel, "  sOow1              = %08lXh", RegisterMap3D->sOow1               ));
  DISPDBG((DebugLevel, "  sSow1              = %08lXh", RegisterMap3D->sSow1               ));
  DISPDBG((DebugLevel, "  sTow1              = %08lXh", RegisterMap3D->sTow1               ));
  DISPDBG((DebugLevel, "  sDrawTriCMD        = %08lXh", RegisterMap3D->sDrawTriCMD         ));
  DISPDBG((DebugLevel, "  sBeginTriCMD       = %08lXh", RegisterMap3D->sBeginTriCMD        ));
  DISPDBG((DebugLevel, "  textureMode        = %08lXh", RegisterMap3D->textureMode         ));
  DISPDBG((DebugLevel, "  tLOD               = %08lXh", RegisterMap3D->tLOD                ));
  DISPDBG((DebugLevel, "  tDetail            = %08lXh", RegisterMap3D->tDetail             ));
  DISPDBG((DebugLevel, "  texBaseAddr        = %08lXh", RegisterMap3D->texBaseAddr         ));
  DISPDBG((DebugLevel, "  texBaseAddr1       = %08lXh", RegisterMap3D->texBaseAddr1        ));
  DISPDBG((DebugLevel, "  texBaseAddr2       = %08lXh", RegisterMap3D->texBaseAddr2        ));
  DISPDBG((DebugLevel, "  texBaseAddr38      = %08lXh", RegisterMap3D->texBaseAddr38       ));
  DISPDBG((DebugLevel, "  trexInit0          = %08lXh", RegisterMap3D->trexInit0           ));
  DISPDBG((DebugLevel, "  trexInit1          = %08lXh", RegisterMap3D->trexInit1           ));
  for (i = 0; i < 12; i++)
  {
    DISPDBG((DebugLevel, "  nccTable0[%02ld]      = %08lXh", i, RegisterMap3D->nccTable0[12]));
  }
  for (i = 0; i < 12; i++)
  {
    DISPDBG((DebugLevel, "  nccTable1[%02ld]      = %08lXh", i, RegisterMap3D->nccTable1[12]));
  }
#endif
}

#ifdef SLI_AA

#ifndef PCI_ENABLE_IO_SPACE
#define PCI_ENABLE_IO_SPACE   0x0001
#endif

#ifndef PCICOMMAND
#define PCICOMMAND            4
#endif

/*----------------------------------------------------------------------
Function name:  UpdatePCICommandReg

Description:

Information:

Return:
----------------------------------------------------------------------*/

void
UpdatePCICommandReg(PDEV    *ppdev,
                    ULONG   chipNumber,
                    ULONG   andMask,
                    ULONG   orMask)
{
  HWPCIOP HwPCIOp;
  DWORD   numBytes;


  // read the pci command register
  HwPCIOp.dwOp = H3G_PCI_READ;
  HwPCIOp.dwFunc = chipNumber;
  HwPCIOp.dwOffset = PCICOMMAND;

  // call the miniport
  if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_3DFX_PCI_OP,
                         &HwPCIOp,
                         sizeof(HwPCIOp),
                         &HwPCIOp,
                         sizeof(HwPCIOp),
                         &numBytes))
  {
    DISPDBG((0, "Failed IOCTL_3DFX_PCI_OP read"));
    return;
  }

  // apply masks
  HwPCIOp.dwValue &= andMask;
  HwPCIOp.dwValue |= orMask;

  // write the pci command register
  HwPCIOp.dwOp = H3G_PCI_WRITE;
  HwPCIOp.dwFunc = chipNumber;
  HwPCIOp.dwOffset = PCICOMMAND;
  
  // call the miniport
  if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_3DFX_PCI_OP,
                         &HwPCIOp,
                         sizeof(HwPCIOp),
                         &HwPCIOp,
                         sizeof(HwPCIOp),
                         &numBytes))
  {
    DISPDBG((0, "Failed IOCTL_3DFX_PCI_OP write"));
  }
}
#endif

VOID
DumpH3Regs(PDEV *ppdev, ULONG DebugLevel)
{
  DISPDBG((DebugLevel, "Chip0"));
  DumpH3RegsForChip(ppdev, DebugLevel,
                    (PUCHAR)     _FF(regBase[HWINFO_SST_IO_INDEX]),
                    (SstIORegs *)_FF(regBase[HWINFO_SST_IOREGS_INDEX]),
                    (SstGRegs *) _FF(regBase[HWINFO_SST_2DREGS_INDEX]),
                    (SstRegs *)  _FF(regBase[HWINFO_SST_3DREGS_INDEX]));

#ifdef SLI_AA
  if ((IS_NAPALM) && (1 < _FF(dwNumUnits)))
  {
    DWORD i;

    // disable i/o on master
    UpdatePCICommandReg(ppdev, 0, ~PCI_ENABLE_IO_SPACE, 0);
  
    for (i = 1; i < _FF(dwNumUnits); i++)
    {
      // enable i/o on slave
      UpdatePCICommandReg(ppdev, i, (LONG)-1, PCI_ENABLE_IO_SPACE);
  
      DISPDBG((DebugLevel, "Chip%ld", i));
      DumpH3RegsForChip(ppdev, DebugLevel,
                        (PUCHAR)     _FF(regBase[i * HWINFO_SST_MAX_CHIP_INDEX + HWINFO_SST_IO_INDEX]),
                        (SstIORegs *)_FF(regBase[i * HWINFO_SST_MAX_CHIP_INDEX + HWINFO_SST_IOREGS_INDEX]),
                        (SstGRegs *) _FF(regBase[i * HWINFO_SST_MAX_CHIP_INDEX + HWINFO_SST_2DREGS_INDEX]),
                        (SstRegs *)  _FF(regBase[i * HWINFO_SST_MAX_CHIP_INDEX + HWINFO_SST_3DREGS_INDEX]));
  
      // disable i/o on slave
      UpdatePCICommandReg(ppdev, i, ~PCI_ENABLE_IO_SPACE, 0);
    }
  
    // enable i/o on master
    UpdatePCICommandReg(ppdev, 0, (LONG)-1, PCI_ENABLE_IO_SPACE);
  }
#endif
}

#endif // DBG
