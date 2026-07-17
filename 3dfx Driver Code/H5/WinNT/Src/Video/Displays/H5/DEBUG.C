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

#if 0
/*----------------------------------------------------------------------
Function name: 

Description:   

Return:        
----------------------------------------------------------------------*/

static VOID
MyDumpPCIConfigSpace(PDEV *ppdev, ULONG DebugLevel)
{
	PUCHAR     pIO = (PUCHAR)ppdev->pjIoBase;
  SstIORegs *pIORegs = (SstIORegs *)ppdev->pjBase;
  ULONG      vgaInit0;


  DISPDBG((DebugLevel, "PCI Config Space using special access thru CR1C\n"));

  vgaInit0 = (int)RegisterMap->vgaInit0;
  pIORegs->vgaInit0 &= 0xFFFFFF3F;
  for (regNum = 0; regNum < 64*4; regNum++)
  {
    OUTPB(pIO + 0xD4, 0x1C);
    OUTPB(pIO + 0xD5, (UCHAR)regNum);
    *(UCHAR *)((UCHAR *)&PCIBuffer + regNum) = INPB(pIO + 0xD5);
  }
  pIORegs->vgaInit0 = (ULONG)vgaInit0;


  DISPDBG((DebugLevel, "  VendorID         = %04Xh\n",  PCIBuffer.VendorID                ));
  DISPDBG((DebugLevel, "  DeviceID         = %04Xh\n",  PCIBuffer.DeviceID                ));
  DISPDBG((DebugLevel, "  Command          = %04Xh\n",  PCIBuffer.Command                 ));
  DISPDBG((DebugLevel, "  Status           = %04Xh\n",  PCIBuffer.Status                  ));
  DISPDBG((DebugLevel, "  RevisionID       = %02Xh\n",  PCIBuffer.RevisionID              ));
  DISPDBG((DebugLevel, "  ProgIf           = %02Xh\n",  PCIBuffer.ProgIf                  ));
  DISPDBG((DebugLevel, "  SubClass         = %02Xh\n",  PCIBuffer.SubClass                ));
  DISPDBG((DebugLevel, "  BaseClass        = %02Xh\n",  PCIBuffer.BaseClass               ));
  DISPDBG((DebugLevel, "  CacheLineSize    = %02Xh\n",  PCIBuffer.CacheLineSize           ));
  DISPDBG((DebugLevel, "  LatencyTimer     = %02Xh\n",  PCIBuffer.LatencyTimer            ));
  DISPDBG((DebugLevel, "  HeaderType       = %02Xh\n",  PCIBuffer.HeaderType              ));
  DISPDBG((DebugLevel, "  BIST             = %02Xh\n",  PCIBuffer.BIST                    ));
  DISPDBG((DebugLevel, "  BaseAddresses[0] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[0]));
  DISPDBG((DebugLevel, "  BaseAddresses[1] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[1]));
  DISPDBG((DebugLevel, "  BaseAddresses[2] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[2]));
  DISPDBG((DebugLevel, "  BaseAddresses[3] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[3]));
  DISPDBG((DebugLevel, "  BaseAddresses[4] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[4]));
  DISPDBG((DebugLevel, "  BaseAddresses[5] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[5]));
  DISPDBG((DebugLevel, "  CIS              = %08lXh\n", PCIBuffer.u.type0.CIS             ));
  DISPDBG((DebugLevel, "  SubVendorID      = %04Xh\n",  PCIBuffer.u.type0.SubVendorID     ));
  DISPDBG((DebugLevel, "  SubSystemID      = %04Xh\n",  PCIBuffer.u.type0.SubSystemID     ));
  DISPDBG((DebugLevel, "  ROMBaseAddress   = %08lXh\n", PCIBuffer.u.type0.ROMBaseAddress  ));
#if (_WIN32_WINNT >= 0x0500)
  DISPDBG((DebugLevel, "  CapabilitiesPtr  = %02Xh\n",  PCIBuffer.u.type0.CapabilitiesPtr ));
#endif
  DISPDBG((DebugLevel, "  InterruptLine    = %02Xh\n",  PCIBuffer.u.type0.InterruptLine   ));
  DISPDBG((DebugLevel, "  InterruptPin     = %02Xh\n",  PCIBuffer.u.type0.InterruptPin    ));
  DISPDBG((DebugLevel, "  MinimumGrant     = %02Xh\n",  PCIBuffer.u.type0.MinimumGrant    ));
  DISPDBG((DebugLevel, "  MaximumLatency   = %02Xh\n",  PCIBuffer.u.type0.MaximumLatency  ));
}
#endif

/*----------------------------------------------------------------------
Function name: 

Description:   

Return:        
----------------------------------------------------------------------*/

VOID
DumpH3Regs(PDEV *ppdev, ULONG DebugLevel)
{
	PUCHAR     pIO = (PUCHAR)ppdev->pjIoBase;
  SstIORegs *RegisterMap = (SstIORegs *)ppdev->pjBase;
  SstGRegs  *RegisterMap2D = (SstGRegs *)ppdev->pjH3Base;
  SstRegs   *RegisterMap3D = (SstRegs *)ppdev->p3DBase;
  int i;


#if 0
  MyDumpPCIConfigSpace(HwDeviceExtension, DebugLevel);
#endif



#if 0
  DISPDBG((DebugLevel, "IO Base0 Regs\n"));

  DISPDBG((DebugLevel, "  STATUS                       = %08lXh\n", INPD(pIO + STATUS                      )));
  DISPDBG((DebugLevel, "  PCIINIT0                     = %08lXh\n", INPD(pIO + PCIINIT0                    )));
  DISPDBG((DebugLevel, "  SIPMONITOR                   = %08lXh\n", INPD(pIO + SIPMONITOR                  )));
  DISPDBG((DebugLevel, "  LFBMEMORYCONFIG              = %08lXh\n", INPD(pIO + LFBMEMORYCONFIG             )));
  if (IS_NAPALM)
  {
    OUTPD(pIO + MISCINIT0, INPD(pIO + MISCINIT0) & ~BIT(30));
    DISPDBG((DebugLevel, "  MISCINIT0                    = %08lXh\n", INPD(pIO + MISCINIT0                   )));
    DISPDBG((DebugLevel, "  MISCINIT1                    = %08lXh\n", INPD(pIO + MISCINIT1                   )));
    OUTPD(pIO + MISCINIT0, INPD(pIO + MISCINIT0) | BIT(30));
    DISPDBG((DebugLevel, "  MISCINIT0                    = %08lXh\n", INPD(pIO + MISCINIT0                   )));
    DISPDBG((DebugLevel, "  MISCINIT1                    = %08lXh\n", INPD(pIO + MISCINIT1                   )));
  }
  else
  {
    DISPDBG((DebugLevel, "  MISCINIT0                    = %08lXh\n", INPD(pIO + MISCINIT0                   )));
    DISPDBG((DebugLevel, "  MISCINIT1                    = %08lXh\n", INPD(pIO + MISCINIT1                   )));
  }
  DISPDBG((DebugLevel, "  DRAMINIT0                    = %08lXh\n", INPD(pIO + DRAMINIT0                   )));
  DISPDBG((DebugLevel, "  DRAMINIT1                    = %08lXh\n", INPD(pIO + DRAMINIT1                   )));
  DISPDBG((DebugLevel, "  AGPINIT                      = %08lXh\n", INPD(pIO + AGPINIT                     )));
  DISPDBG((DebugLevel, "  TMUGBEINIT                   = %08lXh\n", INPD(pIO + TMUGBEINIT                  )));
  DISPDBG((DebugLevel, "  VGAINIT0                     = %08lXh\n", INPD(pIO + VGAINIT0                    )));
  DISPDBG((DebugLevel, "  VGAINIT1                     = %08lXh\n", INPD(pIO + VGAINIT1                    )));
  DISPDBG((DebugLevel, "  DRAMCOMMAND                  = %08lXh\n", INPD(pIO + DRAMCOMMAND                 )));
  DISPDBG((DebugLevel, "  DRAMDATA                     = %08lXh\n", INPD(pIO + DRAMDATA                    )));
  DISPDBG((DebugLevel, "  RESERVEDZ_0                  = %08lXh\n", INPD(pIO + RESERVEDZ_0                 )));
  DISPDBG((DebugLevel, "  RESERVEDZ_1                  = %08lXh\n", INPD(pIO + RESERVEDZ_1                 )));
  DISPDBG((DebugLevel, "  PLLCTRL0                     = %08lXh\n", INPD(pIO + PLLCTRL0                    )));
  DISPDBG((DebugLevel, "  PLLCTRL1                     = %08lXh\n", INPD(pIO + PLLCTRL1                    )));
  DISPDBG((DebugLevel, "  PLLCTRL2                     = %08lXh\n", INPD(pIO + PLLCTRL2                    )));
  DISPDBG((DebugLevel, "  DACMODE                      = %08lXh\n", INPD(pIO + DACMODE                     )));
  DISPDBG((DebugLevel, "  DACADDR                      = %08lXh\n", INPD(pIO + DACADDR                     )));
  DISPDBG((DebugLevel, "  DACDATA                      = %08lXh\n", INPD(pIO + DACDATA                     )));
  DISPDBG((DebugLevel, "  VIDMAXRGBDELTA               = %08lXh\n", INPD(pIO + VIDMAXRGBDELTA              )));
  DISPDBG((DebugLevel, "  VIDPROCCFG                   = %08lXh\n", INPD(pIO + VIDPROCCFG                  )));
  DISPDBG((DebugLevel, "  HWCURPATADDR                 = %08lXh\n", INPD(pIO + HWCURPATADDR                )));
  DISPDBG((DebugLevel, "  HWCURLOC                     = %08lXh\n", INPD(pIO + HWCURLOC                    )));
  DISPDBG((DebugLevel, "  HWCURC0                      = %08lXh\n", INPD(pIO + HWCURC0                     )));
  DISPDBG((DebugLevel, "  HWCURC1                      = %08lXh\n", INPD(pIO + HWCURC1                     )));
  DISPDBG((DebugLevel, "  VIDINFORMAT                  = %08lXh\n", INPD(pIO + VIDINFORMAT                 )));
  DISPDBG((DebugLevel, "  VIDINSTATUS                  = %08lXh\n", INPD(pIO + VIDINSTATUS                 )));
  DISPDBG((DebugLevel, "  VIDSERIALPARALLELPORT        = %08lXh\n", INPD(pIO + VIDSERIALPARALLELPORT       )));
  DISPDBG((DebugLevel, "  VIDINXDECIMDELTAS            = %08lXh\n", INPD(pIO + VIDINXDECIMDELTAS           )));
  DISPDBG((DebugLevel, "  VIDINDECIMINITERRS           = %08lXh\n", INPD(pIO + VIDINDECIMINITERRS          )));
  DISPDBG((DebugLevel, "  VIDINYDECIMDELTAS            = %08lXh\n", INPD(pIO + VIDINYDECIMDELTAS           )));
  DISPDBG((DebugLevel, "  VIDPIXELBUFTHOLD             = %08lXh\n", INPD(pIO + VIDPIXELBUFTHOLD            )));
  DISPDBG((DebugLevel, "  VIDCHROMAMIN                 = %08lXh\n", INPD(pIO + VIDCHROMAMIN                )));
  DISPDBG((DebugLevel, "  VIDCHROMAMAX                 = %08lXh\n", INPD(pIO + VIDCHROMAMAX                )));
  DISPDBG((DebugLevel, "  VIDCURRENTLINE               = %08lXh\n", INPD(pIO + VIDCURRENTLINE              )));
  DISPDBG((DebugLevel, "  VIDSCREENSIZE                = %08lXh\n", INPD(pIO + VIDSCREENSIZE               )));
  DISPDBG((DebugLevel, "  VIDOVERLAYSTARTCOORDS        = %08lXh\n", INPD(pIO + VIDOVERLAYSTARTCOORDS       )));
  DISPDBG((DebugLevel, "  VIDOVERLAYENDCOORDS          = %08lXh\n", INPD(pIO + VIDOVERLAYENDCOORDS         )));
  DISPDBG((DebugLevel, "  VIDOVERLAYDUDX               = %08lXh\n", INPD(pIO + VIDOVERLAYDUDX              )));
  DISPDBG((DebugLevel, "  VIDOVERLAYDUDXOFFSETSRCWIDTH = %08lXh\n", INPD(pIO + VIDOVERLAYDUDXOFFSETSRCWIDTH)));
  DISPDBG((DebugLevel, "  VIDOVERLAYDVDY               = %08lXh\n", INPD(pIO + VIDOVERLAYDVDY              )));

  DISPDBG((DebugLevel, "  VIDOVERLAYDVDYOFFSET         = %08lXh\n", INPD(pIO + VIDOVERLAYDVDYOFFSET        )));
  DISPDBG((DebugLevel, "  VIDDESKTOPSTARTADDR          = %08lXh\n", INPD(pIO + VIDDESKTOPSTARTADDR         )));
  DISPDBG((DebugLevel, "  VIDDESKTOPOVERLAYSTRIDE      = %08lXh\n", INPD(pIO + VIDDESKTOPOVERLAYSTRIDE     )));
  DISPDBG((DebugLevel, "  VIDINADDR0                   = %08lXh\n", INPD(pIO + VIDINADDR0                  )));
  DISPDBG((DebugLevel, "  VIDINADDR1                   = %08lXh\n", INPD(pIO + VIDINADDR1                  )));
  DISPDBG((DebugLevel, "  VIDINADDR2                   = %08lXh\n", INPD(pIO + VIDINADDR2                  )));
  DISPDBG((DebugLevel, "  VIDINSTRIDE                  = %08lXh\n", INPD(pIO + VIDINSTRIDE                 )));
  DISPDBG((DebugLevel, "  VIDCURROVERLAYSTARTADDR      = %08lXh\n", INPD(pIO + VIDCURROVERLAYSTARTADDR     )));
#endif



  DISPDBG((DebugLevel, "General Regs\n"));

  DISPDBG((DebugLevel, "  MiscOutput         = %02Xh\n", INPB(pIO + 0xCC)));
  DISPDBG((DebugLevel, "  Input Status 0     = %02Xh\n", INPB(pIO + 0xC2)));
  DISPDBG((DebugLevel, "  Input Status 1     = %02Xh\n", INPB(pIO + 0xDA)));
  DISPDBG((DebugLevel, "  Feature Control    = %02Xh\n", INPB(pIO + 0xCA)));
  DISPDBG((DebugLevel, "  Motherboard Enable = %02Xh\n", INPB(pIO + 0xC3)));
  //DISPDBG((DebugLevel, "  Adapter Enable     = %02Xh\n", INPB(pIO + ??)));
  DISPDBG((DebugLevel, "  Subsystem Enable   = %02Xh\n", INPB(pIO + 0xCE)));



  DISPDBG((DebugLevel, "CRTC Regs\n"));

  for (i = 0; i <= 0x18; i++)
  {
    OUTPB(pIO + 0xD4, i);
    DISPDBG((DebugLevel, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xD5)));
  }
  for (i = 0x1A; i <= 0x1F; i++)
  {
    OUTPB(pIO + 0xD4, i);
    DISPDBG((DebugLevel, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xD5)));
  }
  OUTPB(pIO + 0xD4, 0x22);
  DISPDBG((DebugLevel, "  Index 22h = %02Xh\n", i, INPB(pIO + 0xD5)));
  OUTPB(pIO + 0xD4, 0x24);
  DISPDBG((DebugLevel, "  Index 24h = %02Xh\n", i, INPB(pIO + 0xD5)));
  OUTPB(pIO + 0xD4, 0x26);
  DISPDBG((DebugLevel, "  Index 26h = %02Xh\n", i, INPB(pIO + 0xD5)));



  DISPDBG((DebugLevel, "Sequencer Regs\n"));

  for (i = 0; i <= 0x4; i++)
  {
    OUTPB(pIO + 0xC4, i);
    DISPDBG((DebugLevel, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xC5)));
  }



  DISPDBG((DebugLevel, "Graphics Controller Regs\n"));

  for (i = 0; i <= 0x8; i++)
  {
    OUTPB(pIO + 0xCE, i);
    DISPDBG((DebugLevel, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xCF)));
  }



#if 0
  DISPDBG((DebugLevel, "Attribute Controller Regs\n"));

  for (i = 0; i <= 0x14; i++)
  {
    OUTPB(pIO + 0xC0, i);
    DISPDBG((DebugLevel, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xC0)));
  }
#endif



  DISPDBG((DebugLevel, "MemBase0 Regs\n"));

  DISPDBG((DebugLevel, "  status                       = %08lXh\n", RegisterMap->status                      ));
  DISPDBG((DebugLevel, "  pciInit0                     = %08lXh\n", RegisterMap->pciInit0                    ));
  DISPDBG((DebugLevel, "  sipMonitor                   = %08lXh\n", RegisterMap->sipMonitor                  ));
  DISPDBG((DebugLevel, "  lfbMemoryConfig              = %08lXh\n", RegisterMap->lfbMemoryConfig             ));
  if (IS_NAPALM)
  {
    RegisterMap->miscInit0 &= ~BIT(30);
    DISPDBG((DebugLevel, "  miscInit0                    = %08lXh\n", RegisterMap->miscInit0                   ));
    DISPDBG((DebugLevel, "  miscInit1                    = %08lXh\n", RegisterMap->miscInit1                   ));
    RegisterMap->miscInit0 |= BIT(30);
    DISPDBG((DebugLevel, "  miscInit0                    = %08lXh\n", RegisterMap->miscInit0                   ));
    DISPDBG((DebugLevel, "  miscInit1                    = %08lXh\n", RegisterMap->miscInit1                   ));
  }
  else
  {
    DISPDBG((DebugLevel, "  miscInit0                    = %08lXh\n", RegisterMap->miscInit0                   ));
    DISPDBG((DebugLevel, "  miscInit1                    = %08lXh\n", RegisterMap->miscInit1                   ));
  }
  DISPDBG((DebugLevel, "  dramInit0                    = %08lXh\n", RegisterMap->dramInit0                   ));
  DISPDBG((DebugLevel, "  dramInit1                    = %08lXh\n", RegisterMap->dramInit1                   ));
  DISPDBG((DebugLevel, "  agpInit                      = %08lXh\n", RegisterMap->agpInit                     ));
  DISPDBG((DebugLevel, "  tmuGbeInit                   = %08lXh\n", RegisterMap->tmuGbeInit                  ));
  DISPDBG((DebugLevel, "  vgaInit0                     = %08lXh\n", RegisterMap->vgaInit0                    ));
  DISPDBG((DebugLevel, "  vgaInit1                     = %08lXh\n", RegisterMap->vgaInit1                    ));
  DISPDBG((DebugLevel, "  dramCommand                  = %08lXh\n", RegisterMap->dramCommand                 ));
  DISPDBG((DebugLevel, "  dramData                     = %08lXh\n", RegisterMap->dramData                    ));
  DISPDBG((DebugLevel, "  strapInfo                    = %08lXh\n", RegisterMap->strapInfo                   ));
#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
  DISPDBG((DebugLevel, "  vidTvOutBlankVCount          = %08lXh\n", RegisterMap->vidTvOutBlankVCount         ));
#else
  DISPDBG((DebugLevel, "  _reserved01                  = %08lXh\n", RegisterMap->_reserved01                 ));
#endif
  DISPDBG((DebugLevel, "  pllCtrl0                     = %08lXh\n", RegisterMap->pllCtrl0                    ));
  DISPDBG((DebugLevel, "  pllCtrl1                     = %08lXh\n", RegisterMap->pllCtrl1                    ));
  DISPDBG((DebugLevel, "  pllCtrl2                     = %08lXh\n", RegisterMap->pllCtrl2                    ));
  DISPDBG((DebugLevel, "  dacMode                      = %08lXh\n", RegisterMap->dacMode                     ));
  DISPDBG((DebugLevel, "  dacAddr                      = %08lXh\n", RegisterMap->dacAddr                     ));
  DISPDBG((DebugLevel, "  dacData                      = %08lXh\n", RegisterMap->dacData                     ));
  DISPDBG((DebugLevel, "  vidMaxRGBDelta               = %08lXh\n", RegisterMap->vidMaxRGBDelta              ));
  DISPDBG((DebugLevel, "  vidProcCfg                   = %08lXh\n", RegisterMap->vidProcCfg                  ));
  DISPDBG((DebugLevel, "  hwCurPatAddr                 = %08lXh\n", RegisterMap->hwCurPatAddr                ));
  DISPDBG((DebugLevel, "  hwCurLoc                     = %08lXh\n", RegisterMap->hwCurLoc                    ));
  DISPDBG((DebugLevel, "  hwCurC0                      = %08lXh\n", RegisterMap->hwCurC0                     ));
  DISPDBG((DebugLevel, "  hwCurC1                      = %08lXh\n", RegisterMap->hwCurC1                     ));
  DISPDBG((DebugLevel, "  vidInFormat                  = %08lXh\n", RegisterMap->vidInFormat                 ));
#ifdef TVOUT_SUPPORTED
  DISPDBG((DebugLevel, "  vidTvOutBlankHCount          = %08lXh\n", RegisterMap->vidTvOutBlankHCount         ));
#else
  DISPDBG((DebugLevel, "  vidInStatus                  = %08lXh\n", RegisterMap->vidInStatus                 ));
#endif
  DISPDBG((DebugLevel, "  vidSerialParallelPort        = %08lXh\n", RegisterMap->vidSerialParallelPort       ));
  DISPDBG((DebugLevel, "  vidInXDecimDeltas            = %08lXh\n", RegisterMap->vidInXDecimDeltas           ));
  DISPDBG((DebugLevel, "  vidInDecimInitErrs           = %08lXh\n", RegisterMap->vidInDecimInitErrs          ));
  DISPDBG((DebugLevel, "  vidInYDecimDeltas            = %08lXh\n", RegisterMap->vidInYDecimDeltas           ));
  DISPDBG((DebugLevel, "  vidPixelBufThold             = %08lXh\n", RegisterMap->vidPixelBufThold            ));
  DISPDBG((DebugLevel, "  vidChromaMin                 = %08lXh\n", RegisterMap->vidChromaMin                ));
  DISPDBG((DebugLevel, "  vidChromaMax                 = %08lXh\n", RegisterMap->vidChromaMax                ));
  DISPDBG((DebugLevel, "  vidCurrentLine               = %08lXh\n", RegisterMap->vidCurrentLine              ));
  DISPDBG((DebugLevel, "  vidScreenSize                = %08lXh\n", RegisterMap->vidScreenSize               ));
  DISPDBG((DebugLevel, "  vidOverlayStartCoords        = %08lXh\n", RegisterMap->vidOverlayStartCoords       ));
  DISPDBG((DebugLevel, "  vidOverlayEndScreenCoord     = %08lXh\n", RegisterMap->vidOverlayEndScreenCoord    ));
  DISPDBG((DebugLevel, "  vidOverlayDudx               = %08lXh\n", RegisterMap->vidOverlayDudx              ));
  DISPDBG((DebugLevel, "  vidOverlayDudxOffsetSrcWidth = %08lXh\n", RegisterMap->vidOverlayDudxOffsetSrcWidth));
  DISPDBG((DebugLevel, "  vidOverlayDvdy               = %08lXh\n", RegisterMap->vidOverlayDvdy              ));
  DISPDBG((DebugLevel, "  vidOverlayDvdyOffset         = %08lXh\n", RegisterMap->vidOverlayDvdyOffset        ));
  DISPDBG((DebugLevel, "  vidDesktopStartAddr          = %08lXh\n", RegisterMap->vidDesktopStartAddr         ));
  DISPDBG((DebugLevel, "  vidDesktopOverlayStride      = %08lXh\n", RegisterMap->vidDesktopOverlayStride     ));
  DISPDBG((DebugLevel, "  vidInAddr0                   = %08lXh\n", RegisterMap->vidInAddr0                  ));
  DISPDBG((DebugLevel, "  vidInAddr1                   = %08lXh\n", RegisterMap->vidInAddr1                  ));
  DISPDBG((DebugLevel, "  vidInAddr2                   = %08lXh\n", RegisterMap->vidInAddr2                  ));
  DISPDBG((DebugLevel, "  vidInStride                  = %08lXh\n", RegisterMap->vidInStride                 ));
  DISPDBG((DebugLevel, "  vidCurrOverlayStartAddr      = %08lXh\n", RegisterMap->vidCurrOverlayStartAddr     ));



  DISPDBG((DebugLevel, "2D Regs\n"));

  DISPDBG((DebugLevel, "  status           = %08lXh\n", RegisterMap2D->status          ));
  DISPDBG((DebugLevel, "  unused           = %08lXh\n", RegisterMap2D->unused0         ));
  DISPDBG((DebugLevel, "  clip0Min         = %08lXh\n", RegisterMap2D->clip0min        ));
  DISPDBG((DebugLevel, "  clip0Max         = %08lXh\n", RegisterMap2D->clip0max        ));
  DISPDBG((DebugLevel, "  dstBaseAddr      = %08lXh\n", RegisterMap2D->dstBaseAddr     ));
  DISPDBG((DebugLevel, "  dstFormat        = %08lXh\n", RegisterMap2D->dstFormat       ));
  DISPDBG((DebugLevel, "  srcColorkeyMin   = %08lXh\n", RegisterMap2D->srcColorkeyMin  ));
  DISPDBG((DebugLevel, "  srcColorkeyMax   = %08lXh\n", RegisterMap2D->srcColorkeyMax  ));
  DISPDBG((DebugLevel, "  dstColorkeyMin   = %08lXh\n", RegisterMap2D->dstColorkeyMin  ));
  DISPDBG((DebugLevel, "  dstColorkeyMax   = %08lXh\n", RegisterMap2D->dstColorkeyMax  ));
  DISPDBG((DebugLevel, "  bresError0       = %08lXh\n", RegisterMap2D->bresError0      ));
  DISPDBG((DebugLevel, "  bresError1       = %08lXh\n", RegisterMap2D->bresError1      ));
  DISPDBG((DebugLevel, "  rop              = %08lXh\n", RegisterMap2D->rop             ));
  DISPDBG((DebugLevel, "  srcBaseAddr      = %08lXh\n", RegisterMap2D->srcBaseAddr     ));
  DISPDBG((DebugLevel, "  commandEx        = %08lXh\n", RegisterMap2D->commandEx       ));
  DISPDBG((DebugLevel, "  lineStipple      = %08lXh\n", RegisterMap2D->lineStipple     ));
  DISPDBG((DebugLevel, "  lineStyle        = %08lXh\n", RegisterMap2D->lineStyle       ));
  DISPDBG((DebugLevel, "  pattern0alias    = %08lXh\n", RegisterMap2D->pattern0alias   ));
  DISPDBG((DebugLevel, "  pattern1alias    = %08lXh\n", RegisterMap2D->pattern1alias   ));
  DISPDBG((DebugLevel, "  clip1min         = %08lXh\n", RegisterMap2D->clip1min        ));
  DISPDBG((DebugLevel, "  clip1max         = %08lXh\n", RegisterMap2D->clip1max        ));
  DISPDBG((DebugLevel, "  srcFormat        = %08lXh\n", RegisterMap2D->srcFormat       ));
  DISPDBG((DebugLevel, "  srcSize          = %08lXh\n", RegisterMap2D->srcSize         ));
  DISPDBG((DebugLevel, "  srcXY            = %08lXh\n", RegisterMap2D->srcXY           ));
  DISPDBG((DebugLevel, "  colorBack        = %08lXh\n", RegisterMap2D->colorBack       ));
  DISPDBG((DebugLevel, "  colorFore        = %08lXh\n", RegisterMap2D->colorFore       ));
  DISPDBG((DebugLevel, "  dstSize          = %08lXh\n", RegisterMap2D->dstSize         ));
  DISPDBG((DebugLevel, "  dstXY            = %08lXh\n", RegisterMap2D->dstXY           ));
  DISPDBG((DebugLevel, "  command          = %08lXh\n", RegisterMap2D->command         ));
#if 0
  for (i = 0; i < 16; i++)
  {
    DISPDBG((DebugLevel, "  launchArea[%02ld]   = %08lXh\n", i, RegisterMap2D->launchArea[16]  ));
  }
#endif
  for (i = 0; i < 32; i++)
  {
    DISPDBG((DebugLevel, "  colorPattern[%02ld] = %08lXh\n", i, RegisterMap2D->colorPattern[32]));
  }



#if 0
  DISPDBG((DebugLevel, "3D regs\n"));

  DISPDBG((DebugLevel, "  status             = %08lXh\n", RegisterMap3D->status              ));
  DISPDBG((DebugLevel, "  intrCtrl           = %08lXh\n", RegisterMap3D->intrCtrl            ));
  DISPDBG((DebugLevel, "  vA.x               = %08lXh\n", RegisterMap3D->vA.x                ));
  DISPDBG((DebugLevel, "  vA.y               = %08lXh\n", RegisterMap3D->vA.y                ));
  DISPDBG((DebugLevel, "  vB.x               = %08lXh\n", RegisterMap3D->vB.x                ));
  DISPDBG((DebugLevel, "  vB.y               = %08lXh\n", RegisterMap3D->vB.y                ));
  DISPDBG((DebugLevel, "  vC.x               = %08lXh\n", RegisterMap3D->vC.x                ));
  DISPDBG((DebugLevel, "  vC.y               = %08lXh\n", RegisterMap3D->vC.y                ));
  DISPDBG((DebugLevel, "  r                  = %08lXh\n", RegisterMap3D->r                   ));
  DISPDBG((DebugLevel, "  g                  = %08lXh\n", RegisterMap3D->g                   ));
  DISPDBG((DebugLevel, "  b                  = %08lXh\n", RegisterMap3D->b                   ));
  DISPDBG((DebugLevel, "  z                  = %08lXh\n", RegisterMap3D->z                   ));
  DISPDBG((DebugLevel, "  s                  = %08lXh\n", RegisterMap3D->s                   ));
  DISPDBG((DebugLevel, "  t                  = %08lXh\n", RegisterMap3D->t                   ));
  DISPDBG((DebugLevel, "  a                  = %08lXh\n", RegisterMap3D->a                   ));
  DISPDBG((DebugLevel, "  w                  = %08lXh\n", RegisterMap3D->w                   ));
  DISPDBG((DebugLevel, "  drdx               = %08lXh\n", RegisterMap3D->drdx                ));
  DISPDBG((DebugLevel, "  dgdx               = %08lXh\n", RegisterMap3D->dgdx                ));
  DISPDBG((DebugLevel, "  dbdx               = %08lXh\n", RegisterMap3D->dbdx                ));
  DISPDBG((DebugLevel, "  dzdx               = %08lXh\n", RegisterMap3D->dzdx                ));
  DISPDBG((DebugLevel, "  dadx               = %08lXh\n", RegisterMap3D->dadx                ));
  DISPDBG((DebugLevel, "  dsdx               = %08lXh\n", RegisterMap3D->dsdx                ));
  DISPDBG((DebugLevel, "  dtdx               = %08lXh\n", RegisterMap3D->dtdx                ));
  DISPDBG((DebugLevel, "  dwdx               = %08lXh\n", RegisterMap3D->dwdx                ));
  DISPDBG((DebugLevel, "  drdy               = %08lXh\n", RegisterMap3D->drdy                ));
  DISPDBG((DebugLevel, "  dgdy               = %08lXh\n", RegisterMap3D->dgdy                ));
  DISPDBG((DebugLevel, "  dbdy               = %08lXh\n", RegisterMap3D->dbdy                ));
  DISPDBG((DebugLevel, "  dzdy               = %08lXh\n", RegisterMap3D->dzdy                ));
  DISPDBG((DebugLevel, "  dady               = %08lXh\n", RegisterMap3D->dady                ));
  DISPDBG((DebugLevel, "  dsdy               = %08lXh\n", RegisterMap3D->dsdy                ));
  DISPDBG((DebugLevel, "  dtdy               = %08lXh\n", RegisterMap3D->dtdy                ));
  DISPDBG((DebugLevel, "  dwdy               = %08lXh\n", RegisterMap3D->dwdy                ));
  DISPDBG((DebugLevel, "  triangleCMD        = %08lXh\n", RegisterMap3D->triangleCMD         ));
  DISPDBG((DebugLevel, "  FvA.x              = %08lXh\n", RegisterMap3D->FvA.x               ));
  DISPDBG((DebugLevel, "  FvA.y              = %08lXh\n", RegisterMap3D->FvA.y               ));
  DISPDBG((DebugLevel, "  FvB.x              = %08lXh\n", RegisterMap3D->FvB.x               ));
  DISPDBG((DebugLevel, "  FvB.y              = %08lXh\n", RegisterMap3D->FvB.y               ));
  DISPDBG((DebugLevel, "  FvC.x              = %08lXh\n", RegisterMap3D->FvC.x               ));
  DISPDBG((DebugLevel, "  FvC.y              = %08lXh\n", RegisterMap3D->FvC.y               ));
  DISPDBG((DebugLevel, "  Fr                 = %08lXh\n", RegisterMap3D->Fr                  ));
  DISPDBG((DebugLevel, "  Fg                 = %08lXh\n", RegisterMap3D->Fg                  ));
  DISPDBG((DebugLevel, "  Fb                 = %08lXh\n", RegisterMap3D->Fb                  ));
  DISPDBG((DebugLevel, "  Fz                 = %08lXh\n", RegisterMap3D->Fz                  ));
  DISPDBG((DebugLevel, "  Fs                 = %08lXh\n", RegisterMap3D->Fs                  ));
  DISPDBG((DebugLevel, "  Ft                 = %08lXh\n", RegisterMap3D->Ft                  ));
  DISPDBG((DebugLevel, "  Fa                 = %08lXh\n", RegisterMap3D->Fa                  ));
  DISPDBG((DebugLevel, "  Fw                 = %08lXh\n", RegisterMap3D->Fw                  ));
  DISPDBG((DebugLevel, "  Fdrdx              = %08lXh\n", RegisterMap3D->Fdrdx               ));
  DISPDBG((DebugLevel, "  Fdgdx              = %08lXh\n", RegisterMap3D->Fdgdx               ));
  DISPDBG((DebugLevel, "  Fdbdx              = %08lXh\n", RegisterMap3D->Fdbdx               ));
  DISPDBG((DebugLevel, "  Fdzdx              = %08lXh\n", RegisterMap3D->Fdzdx               ));
  DISPDBG((DebugLevel, "  Fdadx              = %08lXh\n", RegisterMap3D->Fdadx               ));
  DISPDBG((DebugLevel, "  Fdsdx              = %08lXh\n", RegisterMap3D->Fdsdx               ));
  DISPDBG((DebugLevel, "  Fdtdx              = %08lXh\n", RegisterMap3D->Fdtdx               ));
  DISPDBG((DebugLevel, "  Fdwdx              = %08lXh\n", RegisterMap3D->Fdwdx               ));
  DISPDBG((DebugLevel, "  Fdrdy              = %08lXh\n", RegisterMap3D->Fdrdy               ));
  DISPDBG((DebugLevel, "  Fdgdy              = %08lXh\n", RegisterMap3D->Fdgdy               ));
  DISPDBG((DebugLevel, "  Fdbdy              = %08lXh\n", RegisterMap3D->Fdbdy               ));
  DISPDBG((DebugLevel, "  Fdzdy              = %08lXh\n", RegisterMap3D->Fdzdy               ));
  DISPDBG((DebugLevel, "  Fdady              = %08lXh\n", RegisterMap3D->Fdady               ));
  DISPDBG((DebugLevel, "  Fdsdy              = %08lXh\n", RegisterMap3D->Fdsdy               ));
  DISPDBG((DebugLevel, "  Fdtdy              = %08lXh\n", RegisterMap3D->Fdtdy               ));
  DISPDBG((DebugLevel, "  Fdwdy              = %08lXh\n", RegisterMap3D->Fdwdy               ));
  DISPDBG((DebugLevel, "  FtriangleCMD       = %08lXh\n", RegisterMap3D->FtriangleCMD        ));
  DISPDBG((DebugLevel, "  fbzColorPath       = %08lXh\n", RegisterMap3D->fbzColorPath        ));
  DISPDBG((DebugLevel, "  fogMode            = %08lXh\n", RegisterMap3D->fogMode             ));
  DISPDBG((DebugLevel, "  alphaMode          = %08lXh\n", RegisterMap3D->alphaMode           ));
  DISPDBG((DebugLevel, "  fbzMode            = %08lXh\n", RegisterMap3D->fbzMode             ));
  DISPDBG((DebugLevel, "  lfbMode            = %08lXh\n", RegisterMap3D->lfbMode             ));
  DISPDBG((DebugLevel, "  clipLeftRight      = %08lXh\n", RegisterMap3D->clipLeftRight       ));
  DISPDBG((DebugLevel, "  clipTopBottom      = %08lXh\n", RegisterMap3D->clipTopBottom       ));
  DISPDBG((DebugLevel, "  nopCMD             = %08lXh\n", RegisterMap3D->nopCMD              ));
  DISPDBG((DebugLevel, "  fastfillCMD        = %08lXh\n", RegisterMap3D->fastfillCMD         ));
  DISPDBG((DebugLevel, "  swapbufferCMD      = %08lXh\n", RegisterMap3D->swapbufferCMD       ));
  DISPDBG((DebugLevel, "  fogColor           = %08lXh\n", RegisterMap3D->fogColor            ));
  DISPDBG((DebugLevel, "  zaColor            = %08lXh\n", RegisterMap3D->zaColor             ));
  DISPDBG((DebugLevel, "  chromaKey          = %08lXh\n", RegisterMap3D->chromaKey           ));
  DISPDBG((DebugLevel, "  chromaRange        = %08lXh\n", RegisterMap3D->chromaRange         ));
  DISPDBG((DebugLevel, "  userIntrCmd        = %08lXh\n", RegisterMap3D->userIntrCmd         ));
  DISPDBG((DebugLevel, "  stipple            = %08lXh\n", RegisterMap3D->stipple             ));
  DISPDBG((DebugLevel, "  c0                 = %08lXh\n", RegisterMap3D->c0                  ));
  DISPDBG((DebugLevel, "  c1                 = %08lXh\n", RegisterMap3D->c1                  ));
  DISPDBG((DebugLevel, "  stats.fbiPixelsIn  = %08lXh\n", RegisterMap3D->stats.fbiPixelsIn   ));
  DISPDBG((DebugLevel, "  stats.fbiChromaFail= %08lXh\n", RegisterMap3D->stats.fbiChromaFail ));
  DISPDBG((DebugLevel, "  stats.fbiZfuncFail = %08lXh\n", RegisterMap3D->stats.fbiZfuncFail  ));
  DISPDBG((DebugLevel, "  stats.fbiAfuncFail = %08lXh\n", RegisterMap3D->stats.fbiAfuncFail  ));
  DISPDBG((DebugLevel, "  stats.fbiPixelsOut = %08lXh\n", RegisterMap3D->stats.fbiPixelsOut  ));
  for (i = 0; i < 32; i++)
  {
    DISPDBG((DebugLevel, "  fogTable[%02ld]       = %08lXh\n", i, RegisterMap3D->fogTable[32]));
  }
  DISPDBG((DebugLevel, "  renderMode         = %08lXh\n", RegisterMap3D->renderMode          ));
  DISPDBG((DebugLevel, "  stencilMode        = %08lXh\n", RegisterMap3D->stencilMode         ));
  DISPDBG((DebugLevel, "  stencilOp          = %08lXh\n", RegisterMap3D->stencilOp           ));
  DISPDBG((DebugLevel, "  colBufferAddr      = %08lXh\n", RegisterMap3D->colBufferAddr       ));
  DISPDBG((DebugLevel, "  colBufferStride    = %08lXh\n", RegisterMap3D->colBufferStride     ));
  DISPDBG((DebugLevel, "  auxBufferAddr      = %08lXh\n", RegisterMap3D->auxBufferAddr       ));
  DISPDBG((DebugLevel, "  auxBufferStride    = %08lXh\n", RegisterMap3D->auxBufferStride     ));
  DISPDBG((DebugLevel, "  clipLeftRight1     = %08lXh\n", RegisterMap3D->clipLeftRight1      ));
  DISPDBG((DebugLevel, "  clipTopBottom1     = %08lXh\n", RegisterMap3D->clipTopBottom1      ));
  DISPDBG((DebugLevel, "  combineMode        = %08lXh\n", RegisterMap3D->combineMode         ));
  DISPDBG((DebugLevel, "  sliCtrl            = %08lXh\n", RegisterMap3D->sliCtrl             ));
  DISPDBG((DebugLevel, "  aaCtrl             = %08lXh\n", RegisterMap3D->aaCtrl              ));
  DISPDBG((DebugLevel, "  chipMask           = %08lXh\n", RegisterMap3D->chipMask            ));
  DISPDBG((DebugLevel, "  leftDesktopBuf     = %08lXh\n", RegisterMap3D->leftDesktopBuf      ));
  DISPDBG((DebugLevel, "  swapBufferPend     = %08lXh\n", RegisterMap3D->swapBufferPend      ));
  DISPDBG((DebugLevel, "  leftOverlayBuf     = %08lXh\n", RegisterMap3D->leftOverlayBuf      ));
  DISPDBG((DebugLevel, "  rightOverlayBuf    = %08lXh\n", RegisterMap3D->rightOverlayBuf     ));
  DISPDBG((DebugLevel, "  fbiSwapHistory     = %08lXh\n", RegisterMap3D->fbiSwapHistory      ));
  DISPDBG((DebugLevel, "  fbiTrianglesOut    = %08lXh\n", RegisterMap3D->fbiTrianglesOut     ));
  DISPDBG((DebugLevel, "  sSetupMode         = %08lXh\n", RegisterMap3D->sSetupMode          ));
  DISPDBG((DebugLevel, "  sVx                = %08lXh\n", RegisterMap3D->sVx                 ));
  DISPDBG((DebugLevel, "  sVy                = %08lXh\n", RegisterMap3D->sVy                 ));
  DISPDBG((DebugLevel, "  sARGB              = %08lXh\n", RegisterMap3D->sARGB               ));
  DISPDBG((DebugLevel, "  sRed               = %08lXh\n", RegisterMap3D->sRed                ));
  DISPDBG((DebugLevel, "  sGreen             = %08lXh\n", RegisterMap3D->sGreen              ));
  DISPDBG((DebugLevel, "  sBlue              = %08lXh\n", RegisterMap3D->sBlue               ));
  DISPDBG((DebugLevel, "  sAlpha             = %08lXh\n", RegisterMap3D->sAlpha              ));
  DISPDBG((DebugLevel, "  sVz                = %08lXh\n", RegisterMap3D->sVz                 ));
  DISPDBG((DebugLevel, "  sOowfbi            = %08lXh\n", RegisterMap3D->sOowfbi             ));
  DISPDBG((DebugLevel, "  sOow0              = %08lXh\n", RegisterMap3D->sOow0               ));
  DISPDBG((DebugLevel, "  sSow0              = %08lXh\n", RegisterMap3D->sSow0               ));
  DISPDBG((DebugLevel, "  sTow0              = %08lXh\n", RegisterMap3D->sTow0               ));
  DISPDBG((DebugLevel, "  sOow1              = %08lXh\n", RegisterMap3D->sOow1               ));
  DISPDBG((DebugLevel, "  sSow1              = %08lXh\n", RegisterMap3D->sSow1               ));
  DISPDBG((DebugLevel, "  sTow1              = %08lXh\n", RegisterMap3D->sTow1               ));
  DISPDBG((DebugLevel, "  sDrawTriCMD        = %08lXh\n", RegisterMap3D->sDrawTriCMD         ));
  DISPDBG((DebugLevel, "  sBeginTriCMD       = %08lXh\n", RegisterMap3D->sBeginTriCMD        ));
  DISPDBG((DebugLevel, "  textureMode        = %08lXh\n", RegisterMap3D->textureMode         ));
  DISPDBG((DebugLevel, "  tLOD               = %08lXh\n", RegisterMap3D->tLOD                ));
  DISPDBG((DebugLevel, "  tDetail            = %08lXh\n", RegisterMap3D->tDetail             ));
  DISPDBG((DebugLevel, "  texBaseAddr        = %08lXh\n", RegisterMap3D->texBaseAddr         ));
  DISPDBG((DebugLevel, "  texBaseAddr1       = %08lXh\n", RegisterMap3D->texBaseAddr1        ));
  DISPDBG((DebugLevel, "  texBaseAddr2       = %08lXh\n", RegisterMap3D->texBaseAddr2        ));
  DISPDBG((DebugLevel, "  texBaseAddr38      = %08lXh\n", RegisterMap3D->texBaseAddr38       ));
  DISPDBG((DebugLevel, "  trexInit0          = %08lXh\n", RegisterMap3D->trexInit0           ));
  DISPDBG((DebugLevel, "  trexInit1          = %08lXh\n", RegisterMap3D->trexInit1           ));
  for (i = 0; i < 12; i++)
  {
    DISPDBG((DebugLevel, "  nccTable0[%02ld]      = %08lXh\n", i, RegisterMap3D->nccTable0[12]));
  }
  for (i = 0; i < 12; i++)
  {
    DISPDBG((DebugLevel, "  nccTable1[%02ld]      = %08lXh\n", i, RegisterMap3D->nccTable1[12]));
  }
#endif
}

#endif // DBG
