//
//		TESTS.CPP - Test routines for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/11/98
//		Last Modified:	5/12/98
//
//		Routines in this file:
//		fnTestNatMem		Test a Linear Memory Block
//		fnTestVGAMem		Test VGA framebuffer memory
//		fnTestNatMemA		Test native mode framebuffer A memory
//		fnTestNatMemB		Test native mode framebuffer B memory
//		fnTestZBuffer		Test Z buffer memory
//		fnTestTexture		Test texture memory
//		fnTestVGAIO			Test VGA I/O registers
//		fnTestNatReg		Test native mode memory mapped registers
//		fnTestVGAMode		Test a specific VGA mode
//		fnTestNatMode		Test a specific native mode resolution
//		fnTestRAMDAC		Test RAMDAC
//		fnTestVGAROM		Test VGA ROM
//		fnTestAccel2D		Test 2D accelerator functions
//		fnTestBitBlt		Test accelerated bitblt functionality
//		fnTestAccel3D		Test 3D pipeline, fifo, alpha, fog, etc.
//		fnTestDMA			Test DMA channel
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<i86.h>
#include	"ediag.h"

//
//		fnTestVGAMem - Test VGA framebuffer memory
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestVGAMem (void)
{
	long	nLength;
	int	i;
	BOOL	bSuccess;

	LogWriteString ("\nBegin testing video memory (VGA)");
	nLength = 0x10000 / _nMemUnitSize;

	OEMSetVGAMode (&CardInfo);
	SetMode (0x12);
	for (i = 0; i < 4; i++)
	{
		LogComment ("\nVGA Plane %d", i);
		_outp (SEQ_INDEX, 0x02);
		_outp (SEQ_DATA, (BYTE) (1 << i));		// Set map mask
		_outp (GDC_INDEX, 0x04);
		_outp (GDC_DATA, (BYTE) i);				// Set read plane select
		bSuccess = FlatMemoryTests (0xA0000, _nMemUnitSize, nLength);
	}

	// Use this opportunity to test the primitive VGA engine
	if (bSuccess)
	{
		LogComment ("\nVGA Data Pipeline");
		// Test VGA set/reset
		bSuccess = VGAMemSetReset ();
		if (!bSuccess) goto fnTestVGAMem_exit;
		LogWriteString ("\n\tVGA Set/Reset circuitry (passed).");

		// Test VGA color compare
		bSuccess = VGAColorCompare ();
		if (!bSuccess) goto fnTestVGAMem_exit;
		LogWriteString ("\n\tVGA color compare circuitry (passed).");

		// Test VGA rasterops and data rotation
		bSuccess = VGAROPs ();
		if (!bSuccess) goto fnTestVGAMem_exit;
		LogWriteString ("\n\tVGA rasterops and data rotation (passed).");

		// Test VGA write mode 1
		bSuccess = VGAWriteModeOne ();
		if (!bSuccess) goto fnTestVGAMem_exit;
		LogWriteString ("\n\tVGA write mode one (passed).");

		// Test VGA write mode 2
		bSuccess = VGAWriteModeTwo ();
		if (!bSuccess) goto fnTestVGAMem_exit;
		LogWriteString ("\n\tVGA write mode two (passed).");

		// Test VGA write mode 3
		bSuccess = VGAWriteModeThree ();
		if (!bSuccess) goto fnTestVGAMem_exit;
		LogWriteString ("\n\tVGA write mode three (passed).");

		// Test VGA bit mask
		bSuccess = VGABitMask ();
		if (!bSuccess) goto fnTestVGAMem_exit;
		LogWriteString ("\n\tVGA bit mask (passed).");
	}

fnTestVGAMem_exit:
	LogWriteString ("\nTesting video memory complete (VGA)");
	return (bSuccess);
}

//
//		fnTestNatMemA - Test native mode framebuffer A memory
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestNatMemA (void)
{
	long	nLength;

	LogWriteString ("\nBegin testing video memory (Framebuffer A)");

	if (CardInfo.physAddr0 == 0)
	{
		LogWriteString ("\nERROR: Video memory test given video memory address of 0.");
		return (FALSE);
	}
	nLength = CardInfo.nSizeAddr0 / _nMemUnitSize;
	if (nLength <= 0)
	{
		LogWriteString ("\nERROR: Video memory test given video memory length of 0.");
		return (FALSE);
	}

	// Set native mode, allowing access to the linear framebuffer. Test
	// the memory and then restore it back to VGA mode.
	OEMSetNativeMode (&CardInfo);
	OEMPreMemoryTest (&CardInfo, 0);
	FlatMemoryTests (CardInfo.physAddr0, _nMemUnitSize, nLength);
	OEMSetVGAMode (&CardInfo);

	LogWriteString ("\nTesting video memory complete (Framebuffer A)");
	return (TRUE);
}

//
//		fnTestNatMemB - Test native mode framebuffer B memory
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestNatMemB (void)
{
	long	nLength;

	LogWriteString ("\nBegin testing video memory (Framebuffer B)");

	if (CardInfo.physAddr1 == 0)
	{
		LogWriteString ("\nERROR: Video memory test given video memory address of 0.");
		return (FALSE);
	}
	nLength = CardInfo.nSizeAddr1 / _nMemUnitSize;
	if (nLength <= 0)
	{
		LogWriteString ("\nERROR: Video memory test given video memory length of 0.");
		return (FALSE);
	}

	// Set native mode, allowing access to the linear framebuffer. Test
	// the memory and then restore it back to VGA mode.
	OEMSetNativeMode (&CardInfo);
	OEMPreMemoryTest (&CardInfo, 1);
	FlatMemoryTests (CardInfo.physAddr1, _nMemUnitSize, nLength);
	OEMSetVGAMode (&CardInfo);

	LogWriteString ("\nTesting video memory complete (Framebuffer B)");
	return (TRUE);
}

//
//		fnTestZBuffer - Test Z buffer memory
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestZBuffer (void)
{
	long	nLength;

	LogWriteString ("\nBegin testing video memory (Z-Buffer)");

	if (CardInfo.physAddr2 == 0)
	{
		LogWriteString ("\nERROR: Video memory test given video memory address of 0.");
		return (FALSE);
	}
	nLength = CardInfo.nSizeAddr2 / _nMemUnitSize;
	if (nLength <= 0)
	{
		LogWriteString ("\nERROR: Video memory test given video memory length of 0.");
		return (FALSE);
	}

	// Set native mode, allowing access to the linear framebuffer. Test
	// the memory and then restore it back to VGA mode.
	OEMSetNativeMode (&CardInfo);
	OEMPreMemoryTest (&CardInfo, 2);
	FlatMemoryTests (CardInfo.physAddr2, _nMemUnitSize, nLength);
	OEMSetVGAMode (&CardInfo);

	LogWriteString ("\nTesting video memory complete (Z-Buffer)");
	return (TRUE);
}

//
//		fnTestTexture - Test texture memory
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestTexture (void)
{
	long	nLength;

	LogWriteString ("\nBegin testing video memory (Texture)");

	if (CardInfo.physAddr3 == 0)
	{
		LogWriteString ("\nERROR: Video memory test given video memory address of 0.");
		return (FALSE);
	}
	nLength = CardInfo.nSizeAddr3 / _nMemUnitSize;
	if (nLength <= 0)
	{
		LogWriteString ("\nERROR: Video memory test given video memory length of 0.");
		return (FALSE);
	}

	// Set native mode, allowing access to the linear framebuffer. Test
	// the memory and then restore it back to VGA mode.
	OEMSetNativeMode (&CardInfo);
	OEMPreMemoryTest (&CardInfo, 3);
	FlatMemoryTests (CardInfo.physAddr3, _nMemUnitSize, nLength);
	OEMSetVGAMode (&CardInfo);

	LogWriteString ("\nTesting video memory complete (Texture)");
	return (TRUE);
}

//
//		fnTestVGAIO - Test VGA I/O registers
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestVGAIO (void)
{
	BOOL	bSuccess;

	LogWriteString ("\nBegin testing I/O space (VGA)");
	bSuccess = VGAIOTest ();
	LogWriteString ("\nTesting I/O space complete (VGA)");
	SetMode (0x03);

	return (bSuccess);
}

//
//		fnTestNatReg - Test native mode memory mapped registers
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestNatReg (void)
{
	LogWriteString ("\nBegin testing register space (Native)");
	LogWriteString ("\n\tUnimplemented.");
	LogWriteString ("\nTesting register space complete (Native)");
	return (TRUE);
}

//
//		fnTestVGAMode - Test a specific VGA mode
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestVGAMode (void)
{
	static WORD	sVGAModes[] = {0x00, 0x03, 0x05, 0x06, 0x0D, 0x12, 0x13};
	static WORD	wVGAModes[] = {0x00, 0x03, 0x05, 0x06, 0x0D, 0x12, 0x13};
	int			i;
	BOOL			bSuccess;

	LogWriteString ("\nBegin testing video modes (VGA)");

	bSuccess = FALSE;					// Assume failure
	OEMSetVGAMode (&CardInfo);
	if (_bSelectVGAModes)
	{
	  LogComment ("\n\tSelect Mode Set\n");
		for (i = 0; i < (sizeof (sVGAModes)) / (sizeof (WORD)); i++)
		{
			  LogComment ("\n\tSetting Mode %02Xh", sVGAModes[i]);
			  SetMode (sVGAModes[i]);
        		  EnableCursor(0);
			  DrawCommonScreen (sVGAModes[i], NULL,0);
			  bSuccess = VerifyScreen (sVGAModes[i]);
			  if (callback_fn) {
				callback_fn(sVGAModes[i]);
			  }
			  if (!bSuccess) break;
		}
	}
	else if (_bAllVGAModes)
	{
	  LogComment ("\n\tAll Mode Set\n");
		for (i = 0; i < (sizeof (wVGAModes)) / (sizeof (WORD)); i++)
		{
			  LogComment ("\n\tSetting Mode %02Xh", wVGAModes[i]);
			  SetMode (wVGAModes[i]);
        		  EnableCursor(0);
			  DrawCommonScreen (wVGAModes[i], NULL,0);
			  bSuccess = VerifyScreen (wVGAModes[i]);
			  if (callback_fn) {
				callback_fn(wVGAModes[i]);
			  }
			  if (!bSuccess) break;
		}
	}
	else
	{
	  LogComment ("\n\tSingle Mode Set\n");
		LogComment ("\n\tSetting Mode %02Xh", _nVGAMode);
		if (!SetMode ((WORD) _nVGAMode))
		{
			LogComment ("\n\tERROR: Mode %02Xh could not be set.", _nVGAMode);
			return (bSuccess);
		}
		DrawCommonScreen ((WORD) _nVGAMode, NULL,1);
		if (callback_fn) {
			callback_fn(wVGAModes[i]);
		}
		bSuccess = VerifyScreen ((WORD) _nVGAMode);
	}

        EnableCursor(1);
	LogWriteString ("\nTesting video mode complete (VGA)");
	return (bSuccess);
}

//
//		fnTestNatMode - Test a specific native mode resolution
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestNatMode (void)
{
	LPWORD			lpModes;
	WORD			wMode;
	LPVBEMODEINFOBLOCK	lpvbe;
	BOOL			bSuccess;

	LogWriteString ("\nBegin testing video modes (Native)");

	bSuccess = FALSE;						// Assume the worst
	OEMSetNativeMode (&CardInfo);
	lpModes = CardInfo.lpModeList;
	if (_bSelNatModes)
	{
	  LogComment ("\n\tSelect Mode Set\n");
		while (*lpModes != 0xFFFF)
		{
			LogComment ("\n\tSetting Mode %02Xh", *lpModes);
			lpvbe = OEMSetMode (&CardInfo, *lpModes);
        		EnableCursor(0);
			DrawCommonScreen (*lpModes, lpvbe,0);
			if (callback_fn) {
				callback_fn(*lpModes);
			}
			bSuccess = VerifyScreen (*lpModes);
			if (!bSuccess) break;
			lpModes++;
			if (*lpModes != 0xFFFF)	lpModes++;
			if (*lpModes != 0xFFFF)	lpModes++;		/* Only do every third mode */
		}
	}
	else if (_bAllNatModes)
	{
	  LogComment ("\n\tAll Mode Set\n");
		while (*lpModes != 0xFFFF)
		{
			LogComment ("\n\tSetting Mode %02Xh", *lpModes);
			lpvbe = OEMSetMode (&CardInfo, *lpModes);
        		EnableCursor(0);
			DrawCommonScreen (*lpModes, lpvbe,0);
			if (callback_fn) {
				callback_fn(*lpModes);
			}
			bSuccess = VerifyScreen (*lpModes);
			if (!bSuccess) break;
			lpModes++;
		}
	}
	else
	{
		LogComment ("\nSetting Resolution %dx%dx%d at %dHz", _nXRes, _nYRes, _nBPP, _nRefresh);
		lpvbe = OEMSetResolution (&CardInfo, (WORD) _nXRes, (WORD) _nYRes, (WORD) _nBPP, (WORD) _nRefresh, (LPWORD) &wMode);
		if (lpvbe == NULL)
		{
			LogComment ("\n\tERROR: Resolution %dx%dx%d at %dHz could not be set.", _nXRes, _nYRes, _nBPP, _nRefresh);
			return (bSuccess);
		}
		DrawCommonScreen (wMode, lpvbe,1);
		bSuccess = VerifyScreen (wMode);
	}

	OEMSetVGAMode (&CardInfo);
        EnableCursor(1);
	LogWriteString ("\nTesting video mode complete (Native)");
	return (bSuccess);
}

//
//		fnTestRAMDAC - Test RAMDAC
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestRAMDAC (void)
{
	BOOL	bSuccess;

	LogWriteString ("\nBegin testing RAMDAC");

	bSuccess = TRUE;				// Assume success
	if (_bRAMDACFull)
	{
		bSuccess = VGATestRAMDAC ();
		if (bSuccess) bSuccess = OEMTestRAMDAC (&CardInfo);
		if (bSuccess) bSuccess = OEMTestPLL (&CardInfo);
		if (bSuccess) bSuccess = OEMTestHWCursor (&CardInfo);
	}
	else
	{
		if (_bRAMDACVGA)
			bSuccess = VGATestRAMDAC ();
		if (_bRAMDACNative && bSuccess)
			bSuccess = OEMTestRAMDAC (&CardInfo);
		if (_bRAMDACPLL && bSuccess)
			bSuccess = OEMTestPLL (&CardInfo);
		if (_bRAMDACCursor && bSuccess)
			bSuccess = OEMTestHWCursor (&CardInfo);
	}
	LogWriteString ("\nTesting RAMDAC complete");
	return (bSuccess);
}

//
//		fnTestVGAROM - Test VGA ROM
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestVGAROM (void)
{
	BYTE	__far *lpROM;
	BYTE	by;
	WORD	wPCIOffset;
	DWORD	dw;
	int	nSize, i;
	BOOL	bSuccess;

	LogWriteString ("\nBegin testing VGA ROM");

	bSuccess = FALSE;						// Assume the worst
	lpROM = (BYTE __far *) MK_FP (selFlat, Phys2Linear (0xC0000, 32*1024));

	// The first two bytes should be a 55h, AAh.
	if ((*lpROM == 0x55) && (*(lpROM+1) == 0xAA))
		LogWriteString ("\n\tROM signature found (passed).");
	else
	{
		LogWriteString ("\n\tERROR: ROM signature (55AAh) not found.");
		goto fnTestVGAROM_exit;
	}

	// Do a checksum
	nSize = *(lpROM + 2);					// Retrive the size
	nSize *= 512;								// Convert to size in BYTEs
	by = 0;
	for (i = 0; i < nSize; i++)
		by += *(lpROM + i);
	if (by == 0)
		LogComment ("\n\tROM checksum of size %d (%04Xh) computes to zero (passed).", nSize, nSize);
	else
	{
		LogComment ("\n\tERROR: Non-zero checksum computed for ROM of size %d (%04Xh).", nSize, nSize);
		goto fnTestVGAROM_exit;
	}

	// Verify data structure integrity
	wPCIOffset = *(WORD *) (lpROM + 0x18);
	if (wPCIOffset > nSize)
	{
		LogWriteString ("\n\tERROR: PCI Header points beyond end of ROM.");
		goto fnTestVGAROM_exit;
	}
	if ((wPCIOffset & 0x03) != 0)
	{
		LogWriteString ("\n\tERROR: PCI Header is not DWORD aligned.");
		goto fnTestVGAROM_exit;
	}
	dw = *(DWORD *) (lpROM + wPCIOffset);
	if (dw == 0x52494350)
		LogWriteString ("\n\tPCI Header found and is valid (passed).");
	else
	{
		LogWriteString ("\n\tERROR: PCI Header not found (\"PCIR\").");
		goto fnTestVGAROM_exit;
	}

	bSuccess = TRUE;
fnTestVGAROM_exit:
	LogWriteString ("\nTesting VGA ROM complete");
	return (bSuccess);
}

//
//		fnTestAccel2D - Test 2D accelerator functions
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestAccel2D (void)
{
	BOOL	bSuccess;

	LogWriteString ("\nBegin testing 2D Graphics Engine");
	bSuccess = OEMTestAccel2D (&CardInfo);
	LogWriteString ("\nTesting 2D Graphics Engine complete");
	return (bSuccess);
}

//
//		fnTestBitBlt - Test accelerated bitblt functionality
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestBitBlt (void)
{
	BOOL	bSuccess;

	LogWriteString ("\nBegin testing BITBLT Engine");
	bSuccess = OEMTestBitBlt (&CardInfo);
	LogWriteString ("\nTesting BITBLT complete");
	return (bSuccess);
}

//
//		fnTestAccel3D - Test 3D pipeline, fifo, alpha, fog, etc.
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestAccel3D (void)
{
	BOOL	bSuccess;

	LogWriteString ("\nBegin testing 3D Graphics Engine");
	bSuccess = OEMTestAccel3D (&CardInfo);
	LogWriteString ("\nTesting 3D Graphics Engine complete");
	return (bSuccess);
}

//
//		fnTestDMA - Test DMA channel
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Test executed successfully, FALSE = Not)
//
BOOL fnTestDMA (void)
{
	BOOL	bSuccess;

	LogWriteString ("\nBegin testing DMA channel");
	bSuccess = OEMTestDMA (&CardInfo);
	LogWriteString ("\nTesting DMA channel complete");
	return (bSuccess);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
