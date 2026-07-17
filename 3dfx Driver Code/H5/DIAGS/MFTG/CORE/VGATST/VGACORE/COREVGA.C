

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "3dfx.h"
#include "fxpci.h"

#include "h3regs.h"
#include "h3defs.h"
#include "h3cinit.h"
#include "fxvid.h"
#include "fxhal.h"

#include "pcibrd.h"
#include "memtst.h"

#include "glide.h"

#include "banshee.h"
#include "crc.h"
#include "ediag_ex.h"
#include "errrpt.h"
#include "misc.h"
#include "vgasim.h"
#include "vgacore.h"





//
//		UndocLightpenTest - Verify that CRTC[3].7 sets/clears access to CRTC[10] and CRTC[11]
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
//		Note:	Later versions of the Model 70 do not support this feature and
//				therefore diminishes the relevance of this test.
//






BOOL UndocLightpenTest (LPCARDINFO card)
{
	int	nErr;
	BYTE	temp;
	WORD	wSimType;
	BOOL	bFullVGA;
	FxU32	dummy;
	SstIORegs *ioregs = (SstIORegs *)(card->NatMem0.MappedAddr);
int	REF_PART  =	1;
int	REF_TEST  =	1;

 
	nErr = ERROR_NONE;
  //	wSimType = SimGetType ();
    bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

  // 	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
   	if (!bFullVGA){
  // 	 SimSetFrameSize (FALSE);	// Use small frame
  	 printf("use small frame \n");
   	}
 

   SetMode (0x03);

	// Clear write protect
	IOByteWrite (CRTC_CINDEX, 0x11);
	IOByteWrite (CRTC_CDATA, (BYTE) (IOByteRead (CRTC_CDATA) & 0x7F));

	// Verify read/write-ability (don't mess with interrupt bits)
	if ((temp = IsIObitFunctional (CRTC_CDATA, CRTC_CDATA, 0x30)) != 0)
	{
	   printf("lightpentest failure0 \n");
	   //	nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST,
		 //						CRTC_CDATA, 0x11, 0x00, temp);
		goto UndocLightpenTest_exit;
	}
 
	IOByteWrite (CRTC_CINDEX, 0x10);
	if ((temp = IsIObitFunctional (CRTC_CDATA, CRTC_CDATA, 0x00)) != 0)
	{
	 printf("lightpentest failure 1 \n");
	  
	  //	nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST,
		//						CRTC_CDATA, 0x10, 0x00, temp);
		goto UndocLightpenTest_exit;
	}
	IOByteWrite (CRTC_CINDEX, 0x03);
	if ((temp = IsIObitFunctional (CRTC_CDATA, CRTC_CDATA, 0x00)) != 0)
	{
	 printf("lightpentest failure 2 \n");
	  
	  //	nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST,
		//						CRTC_CDATA, 0x03, 0x00, temp);
		goto UndocLightpenTest_exit;
	}
 
	// Clear CRTC[3].7
	temp = (BYTE) IOByteRead (CRTC_CDATA);
	IOByteWrite (CRTC_CDATA, (BYTE) (temp & 0x7F));

	// Verify read/write-ability is lost
   	IOByteWrite (CRTC_CINDEX, 0x11);
  	if ((temp = IsIObitFunctional (CRTC_CDATA, CRTC_CDATA, 0x30)) != 0xCF)
  	{
  		 printf("lightpentest failure 3\n");
	  
		
	  //	nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST,
		//						CRTC_CDATA, 0x11, 0xFF, temp);
 		goto UndocLightpenTest_exit;
 	}
 	IOByteWrite (CRTC_CINDEX, 0x10);
	if ((temp = IsIObitFunctional (CRTC_CDATA, CRTC_CDATA, 0x00)) != 0xFF)
	{
	 printf("lightpentest failure 4 \n");
	  
	  //	nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST,
		//						CRTC_CDATA, 0x10, 0xFF, temp);
		goto UndocLightpenTest_exit;
	}

	// Restore CRTC[3].7
	IOByteWrite (CRTC_CINDEX, 0x03);
	IOByteWrite (CRTC_CDATA, temp);

UndocLightpenTest_exit:
	//SystemCleanUp ();
	return (nErr);
}
//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//


 //
//		UndocLatchTest - Load various values into memory, load latches, and verify contents.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int UndocLatchTest (void)
{
	int		nErr, i;
	SEGOFF	lpVideo;
	BYTE		pattern, temp;
	WORD		offset, wSimType;
	BOOL		bFullVGA;
int	REF_PART = 1;
int	REF_TEST = 2;


	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) (0xA0000000);
 	wSimType = SimGetType ();
 	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	
  	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
  	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame

	if (InitMaps ())
	{
   	printf ( "Could not allocate map memory\n");
		
	}
	// Initialize fake bios data area
	MemByteWrite ((SEGOFF) 0x00000410, 0x20);
	MemWordWrite ((SEGOFF) 0x00000463, 0x03D4);
	MemByteWrite ((SEGOFF) 0x00000487, 0x60);
	MemByteWrite ((SEGOFF) 0x00000488, 0x09);
	MemByteWrite ((SEGOFF) 0x00000489, 0x11);
	MemByteWrite ((SEGOFF) 0x0000048a, 0x0C);


	IOByteWrite (MB_ENABLE, 1);
	IOByteWrite (ADAPTER_ENABLE, 0x0E);



	SetMode (0x12);

	// Fill memory with a pattern
	pattern = 0x15;
	for (i = 0; i < 4; i++)
	{
		IOByteWrite (SEQ_INDEX, 0x02);
		IOByteWrite (SEQ_DATA, (BYTE) (1 << i));
		MemoryFill (lpVideo, pattern, (WORD) 80*480);
	   	pattern = RotateByteLeft (pattern, 1);
	}
 //SimDumpMemory ("T1002.VGA");

	for (offset = 0; offset < (WORD) 80*480; offset++)
	{
		temp = MemByteRead (lpVideo + offset);		// Load the latches
		pattern = 0x15;
		for (i = 0; i < 4; i++)
		{
			IOByteWrite (GDC_INDEX, 0x04);
			IOByteWrite (GDC_DATA, (BYTE) i);
			IOByteWrite (CRTC_CINDEX, 0x22);
			temp = (BYTE) IOByteRead (CRTC_CDATA);
			if (temp != pattern)
			{
			   //	printf("undoc latch test failure #1 temp = %x pattern = %x\n ", temp , pattern);
			   //	nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
			   //					HIWORD (lpVideo + offset),
			   //	   					LOWORD (lpVideo + offset), pattern, temp);
				goto UndocLatchTest_exit;
			}
			pattern = RotateByteLeft (pattern, 1);
		}
	}

UndocLatchTest_exit:
   	SystemCleanUp ();
	return (nErr);
}
//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

//
//		UndocATCToggleTest - Set the ATC toggle to a known state and verify it
//								can be read from an undocumented register (CRTC[24])
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int UndocATCToggleTest (void)
{
	int	nErr;
	BYTE	temp;
	WORD	wSimType;
	BOOL	bFullVGA;
int	REF_PART = 1;
int	REF_TEST = 3;

	nErr = ERROR_NONE;
   //	wSimType = SimGetType ();
   //	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

  //	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
 //	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x03);

	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (CRTC_CINDEX, 0x24);
	temp = (BYTE) IOByteRead (CRTC_CDATA);
	if ((temp & 0x80) == 0x80)
	{
		printf("undoc ATCToggle test failure");
		//nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST,
		  //						CRTC_CDATA, 0x24, 0x00, temp & 0x80);
		goto UndocATCToggleTest_exit;
	}
	IOByteWrite (ATC_INDEX, 0x20);
	temp = (BYTE) IOByteRead (CRTC_CDATA);
	if ((temp & 0x80) == 0x00)
	{
	   printf("undoc ATCToggle test failure");

		//nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST,
		  //						CRTC_CDATA, 0x24, 0x80, temp & 0x80);
		goto UndocATCToggleTest_exit;
	}

UndocATCToggleTest_exit:
  //	SystemCleanUp ();
	return (nErr);
}


//
//		UndocATCIndexTest - Set the ATC index to a known state and verify it
//								can be read from an undocumented register (CRTC[26])
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int UndocATCIndexTest (void)
{
	int	i, nErr;
	BYTE	temp, byOrgIndex;
	WORD	wSimType;
	BOOL	bFullVGA;
int	REF_PART = 1;
int	REF_TEST = 4;

	nErr = ERROR_NONE;
   //	wSimType = SimGetType ();
   //	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

	//SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
   //	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x03);

	IOByteWrite (CRTC_CINDEX, 0x26);
	byOrgIndex = IOByteRead (ATC_INDEX);
	for (i = 0; i < 32; i++)
	{
		// Reset toggle to index state and write the ATC index
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, i);

		// Attribute controller index should be read back as the low order
		// five bits of CRTC[26].
		temp = IOByteRead (CRTC_CDATA);
		if ((temp & 0x3F) != i)
		{
		
				printf("undoc ATCIndex test failure ");
				nErr= 0xFF;  // pass back failure
		  //	nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, CRTC_CDATA, 0x26, i, temp & 0x3F);
			goto UndocATCIndexTest_exit;
		}
	}

	// Reset toggle to index state and test palette address bit. This bit
	// should also be able to be read back from CRTC[26].
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x20);
	temp = IOByteRead (CRTC_CDATA);
	if ((temp & 0x3F) != 0x20)
	{
		printf("undoc ATCIndex test failure ");
		nErr= 0xFF;  // pass back failure
		//nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, CRTC_CDATA, 0x26, 0x20, temp & 0x3F);
		goto UndocATCIndexTest_exit;
	}

UndocATCIndexTest_exit:
 //	SystemCleanUp ();
	return (nErr);
}







//
//		RandomAccessTest - Verify that the VGA space can be accessed in any sequence
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int RandomAccessTest (void)
{
	static DWORD	dwBurstData[] = {
		0x33221100, 0x77665544, 0xBBAA9988, 0xFFEEDDCC,
		0x76543210, 0xFEDCBA98, 0x89ABCDEF, 0x01234567
	};
	static BYTE		byCRTC[] = {
		0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80,
		0x0B, 0x3E, 0x00, 0x40, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xEA, 0x8C,
		0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
		0xFF
	};
	static PARMENTRY	parmX = {					// 320x400x8
		0x50, 0x1D, 0x10,
		0xFF, 0xFF,
		{0x01, 0x0F, 0x00, 0x06},					// SEQ 1..4
		0x63,												// Misc
		{0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80,	// CRTC 0..18h
		0xBF, 0x1F, 0x20, 0x40, 0x00, 0x00,
		0xFF, 0xFF, 0x00, 0x00, 0x9C, 0x0E,
		0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
		0xFF},
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	// ATC 0..13h
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00,
		0x0F, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x40,	// GDC 0..8
		0x05, 0x0F, 0xFF}
	};
	static PARMENTRY	parmSmallX = {
		0x28, 0x1D, 0x10,
		0xFF, 0xFF,
		{0x01, 0x0F, 0x00, 0x06},					// SEQ 1..4
		0x63,												// Misc
		{0x2D, 0x27, 0x28, 0x90, 0x2B, 0x80,	// CRTC 0..18h
		0x17, 0x10, 0x20, 0x40, 0x00, 0x00,
		0xFF, 0xFF, 0x00, 0x00, 0x15, 0x06,
		0x13, 0x14, 0x00, 0x14, 0x17, 0xE3,
		0xFF},
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	// ATC 0..13h
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00,
		0x0F, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x40,	// GDC 0..8
		0x05, 0x0F, 0xFF}
	};
	static DWORD	tblPattern1[] = {0x0C080400, 0x0D090501, 0x0E0A0602, 0x0F0B0703};
	static DWORD	tblPattern2[] = {0x1C181410, 0x1D191511, 0x1E1A1612, 0x1F1B1713};
	int		nErr, nState, nScans, nRowOffset, nFrames;
	BOOL		bROM, bIndexed, bFullVGA, bEnd;
	SEGOFF	lpVideoA0, lpVideoB8, lpROM;
	WORD		i, wROMSize, wWPort, wRPort, wCRTC, wSimType;
	BYTE		bySum, byIdx, byMask, byAct, byExp;
	DWORD		dw, dwAct, dwExp;
	FILE		*hIO;
	LPBYTE	lpBurstData;
	int		arg1, arg2, arg3, arg4, arg5, arg6;
	DWORD		dwErrAct = 0;
	DWORD		dwErrExp = 0;
	LPBYTE	lpErrAddr = NULL;
	BOOL VerifyPatternStrip (LPBYTE, int, int, DWORD *, BOOL);
	void BurstMemoryWrites (SEGOFF, LPDWORD, int);
	int	REF_PART = 1;
	int	REF_TEST = 9;

	nErr = ERROR_NONE;
	//wSimType = SimGetType ();
   //	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

	lpVideoA0 = (SEGOFF) 0xA0000000;
	lpVideoB8 = (SEGOFF) 0xB8000000;
	lpROM = (SEGOFF) 0xC0000000;
	lpBurstData = (LPBYTE) &dwBurstData;		// For BYTE accesses

   //	SimSetState (TRUE, FALSE, FALSE);
   //	if (bFullVGA)
   //		SimSetFrameSize (TRUE);						// Use standard frame
   //	else
   //		SimSetFrameSize (FALSE);					// Use small frame
	SetMode (0x12);
   //	SimSetState (TRUE, TRUE, TRUE);

	// If a ROM exists (first two bytes are 055h and 0AAh) then, do a
	// POST-like sequence that includes a checksum verification and a
	// ROM copy into system memory. On a PCI-bus this should involve
	// burst memory reads.
	bROM = FALSE;
	if (MemWordRead (lpROM) == 0xAA55)
	{
		bROM = TRUE;
		wROMSize = (MemByteRead (lpROM + 2)) * 512;
		bySum = 0;
		for (i = 0; i < wROMSize; i++)
			bySum += MemByteRead (lpROM + i);

		if (bySum != 0)
		{
		   //	printf("random access test fail checksum error\n");

			nErr = FlagError (ERROR_CHECKSUM, REF_PART, REF_TEST, 0, 0, 0x00, bySum);
			goto RandomAccessTest_exit;
		}

		// Now do a PCI-like ROM image transfer
		bySum = 0;
		for (i = 0; i < wROMSize; i += 4)
		{
			dw = MemDwordRead (lpROM + i);
			bySum += LOBYTE (LOWORD (dw));
			bySum += HIBYTE (LOWORD (dw));
			bySum += LOBYTE (HIWORD (dw));
			bySum += HIBYTE (HIWORD (dw));
		}

		if (bySum != 0)
		{
			//printf("random access test fail checksum error #2\n");

			nErr = FlagError (ERROR_CHECKSUM, REF_PART, REF_TEST, 0, 0, 0x00, bySum);
			goto RandomAccessTest_exit;
		}
	}

	// Perform a random I/O read/write sequence based on a script file
	// that includes whether to just read, just write or do a full test.
	// Read the pseudo-random I/O sequence from a file called "RAND.TXT"
	hIO = fopen ("RAND.TXT", "r");
	if (hIO == NULL)
	{
		//printf("random access test fail error file\n ");

		nErr = FlagError (ERROR_FILE, REF_PART, REF_TEST, 0, 0, 0, 0);
		goto RandomAccessTest_exit;
	}

	while (!feof (hIO))
	{
		// For compatibility with most C compilers and 16-bit vs. 32-bit
		// integers, the arguments passed to "fscanf" must all be "int".
		fscanf (hIO, "%02X %04X %04X %02X %02X %02X", &arg1, &arg2, &arg3, &arg4, &arg5, &arg6);
		nState = arg1;
		wWPort = arg2;
		wRPort = arg3;
		bIndexed = arg4;
		byIdx = arg5;
		byMask = arg6;

		if (!IOTest (nState, wWPort, wRPort, bIndexed, byIdx, byMask))
		{
			fclose (hIO);
			goto RandomAccessTest_exit;
		}
	}
	fclose (hIO);

	// At this point the VGA is in an unknown state due to the random
	// I/O writes that have occurred. Set it back to a known state.
   //	SimSetState (TRUE, FALSE, FALSE);
	SetMode (0x92);							// Don't write to memory
   //	SimSetState (TRUE, TRUE, TRUE);

	// Perform a variety of operations involving different mixes of memory
	// and I/O accesses.
	// First, do a burst memory write with a couple of "random" memory reads.
	BurstMemoryWrites (lpVideoA0, dwBurstData, 8);
	byAct = MemByteRead (lpVideoA0 + 3);
	byExp = *(lpBurstData + 3);
	if (byAct != byExp);
	{
		//printf("random access test fail error memory\n");
	

		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, HIWORD (lpVideoA0), 3, byExp, byAct);
		goto RandomAccessTest_exit;
	}
	byAct = MemByteRead (lpVideoA0 + 21);
	byExp = *(lpBurstData + 21);
	if (byAct != byExp)
	{
		printf("random access test fail error memory #2\n");

		//nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, HIWORD (lpVideoA0), 21, byExp, byAct);
		goto RandomAccessTest_exit;
	}

	// Now do a burst of memory writes with an I/O read operation in the middle
	// followed by a complete verification of memory
	BurstMemoryWrites (lpVideoA0, dwBurstData, 4);
	IOByteRead (0x3DA);
	BurstMemoryWrites (lpVideoA0 + 16, &dwBurstData[4], 4);
	for (i = 0; i < 32; i++)
	{
		byAct = MemByteRead (lpVideoA0 + i);
		byExp = *(lpBurstData + i);
		if (byAct != byExp)
		{
		   //printf("random access test fail error memory #3\n");

		   	nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, HIWORD (lpVideoA0), i, byExp, byAct);
			goto RandomAccessTest_exit;
		}
	}

	// Now do a burst of memory writes with an I/O write operation in the
	// middle that switches the memory address from A0000h to B8000h. Switch
	// back to A0000h before reading memory back as DWORD's.
	BurstMemoryWrites (lpVideoA0, dwBurstData, 4);
	IOWordWrite (0x3CE, 0x0D06);
	BurstMemoryWrites (lpVideoB8 + 16, &dwBurstData[4], 4);
	IOWordWrite (0x3CE, 0x0506);
	for (i = 0; i < 8; i++)
	{
		dwAct = MemDwordRead (lpVideoA0 + (i*4));
		dwExp = *(lpBurstData + i);
		if (dwAct != dwExp)
		{
			// Note: error handler can only handle BYTE's, not DWORD's
		   //	 printf("random access test fail error memory #4\n");

			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, HIWORD (lpVideoA0), i*4, (BYTE) dwExp, (BYTE) dwAct);
			goto RandomAccessTest_exit;
		}
	}

	// Now write 00h to all the CRTC registers, switching the I/O address
	// between 3D4h and 3B4h. When complete, verify that it can be read
	// from both locations and then restore it to "normal" values.
	wCRTC = 0x3D4;
	IOWordWrite (wCRTC, 0x0011);			// Unwrite-protect the CRTC
	for (i = 0; i < sizeof (byCRTC); i++)
	{
		// Switch the CRTC address
		if (wCRTC == 0x3D4)
		{
			IOByteWrite (0x3C2, 0x66);
			wCRTC = 0x3B4;
		}
		else
		{
			IOByteWrite (0x3C2, 0x67);
			wCRTC = 0x3D4;
		}

		if (i % 3)								// On every 3rd access, do a WORD write
			IOWordWrite (wCRTC, i);
		else
		{
			IOByteWrite (wCRTC, (BYTE) i);
			IOByteWrite (wCRTC + 1, 0x00);
		}
	}
	// Verify all is 00h
	IOByteWrite (0x3C2, 0x67);				// Set back to 3D4h address
	for (i = 0; i < sizeof (byCRTC); i++)
	{
		IOByteWrite (wCRTC, (BYTE) i);
		byAct = IOByteRead (wCRTC + 1);
		if (byAct != 0x00)
		{
			//printf("random access test fail error io failure \n");

			nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, wCRTC, i, 0, byAct);
			goto RandomAccessTest_exit;
		}
	}
	// Restore the CRTC
	for (i = 0; i < sizeof (byCRTC); i++)
		IOWordWrite (0x3D4, (((WORD) byCRTC[i]) << 8) | i);

	// Some Mode X type games will perform DWORD accesses to video memory
	// and switch planes four times to write 16-pixel "strips". Write a
	// strip of data two-DWORDs wide and verify that the video memory
	// is correct. Perform this for a given number of frames.
	// First, set mode X (assume the VGA is already in Mode 12h)
	if (bFullVGA)
	{
		SetRegs (&parmX);
		nScans = 400;
		nRowOffset = 320/4;
	}
	else
	{
		SetRegs (&parmSmallX);
		nScans = 20;
		nRowOffset = 160/4;
	}

	// Loop thru a number of frames, while writing strips and reading them
	// back for accuracy.
	nFrames = 5;
	bEnd = FALSE;
	while (!bEnd)
	{
		// Stick this condition inside the loop so that - for debugging
		// purposes - a wait for "kbhit" can be inserted.
		if (--nFrames <= 0) bEnd = TRUE;

		WaitVerticalRetrace ();
		ClearModeX (bFullVGA);
		WritePatternStrip ((LPBYTE) 0xA0000028, nScans, nRowOffset, tblPattern1, bFullVGA);
		WritePatternStrip ((LPBYTE) 0xA000002C, nScans, nRowOffset, tblPattern2, bFullVGA);
		if (!VerifyPatternStrip ((LPBYTE) 0xA0000028, nScans, nRowOffset, tblPattern1, bFullVGA))
		{
		   //printf("random access test fail error memory #3\n ");

		   	nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, LOWORD (lpErrAddr), HIWORD (lpErrAddr), dwErrExp, dwErrAct);
			break;
		}
		if (!VerifyPatternStrip ((LPBYTE) 0xA000002C, nScans, nRowOffset, tblPattern2, bFullVGA))
		{
		  //printf("random access test fail error memory #4\n");

			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, LOWORD (lpErrAddr), HIWORD (lpErrAddr), dwErrExp, dwErrAct);
			break;
		}
	}

RandomAccessTest_exit:
//	SystemCleanUp ();
	return (nErr);
}

//
//		IOTest - Test a specific read/write I/O port
//
//		Entry:	state		I/O action (0: Read, 1: Write, 2: Test)
//					wport		WRITE address of port
//					rport		READ address of port
//					bIndexed	Index flag (TRUE = Indexed register, FALSE = Not)
//					idx		Index register
//					mask		Mask of unused bits
//		Exit:		<int>		DOS ERRORLEVEL value
//
int IOTest (BYTE state, WORD wport, WORD rport, BOOL bIndexed, BYTE idx, BYTE mask)
{
	int		nErr;
	BYTE		temp;
	static	data = 0x55;
int	REF_PART = 1;
int	REF_TEST = 5;

	nErr = ERROR_NONE;

	switch (state)
	{
		case 0:											// Just read (ignore index)

			IOByteRead (rport);
			break;

		case 1:											// Just write

			if (wport == ATC_INDEX)
			{
				ClearIObitDataBus ();
				if (bIndexed)
				{
					IOByteWrite (wport, idx);
					ClearIObitDataBus ();
				}
			}
			else if (bIndexed)
				IOByteWrite (wport - 1, idx);

			IOByteWrite (wport, data & mask);
			break;

		case 2:											// Test the port

			if (wport == ATC_INDEX)
			{
				ClearIObitDataBus ();				// Set index state
				if (bIndexed)
				{
					IOByteWrite (wport, idx);
					ClearIObitDataBus ();			// Set back to index state
				}
			}
			else if (bIndexed)
				IOByteWrite (wport - 1, idx);

			if ((temp = IsIObitFunctional (rport, wport, (BYTE) ~mask)) != 0x00)
				printf("random access test fail");

				//nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, rport, idx, 0, temp);
			break;
	}

	return (nErr);
}

//
//		BurstMemoryWrites - Write a block of DWORD data to memory
//
//		Entry:	lpVideo		Pointer to memory
//					lpdwData		Pointer to data table
//					nCount		Number of DWORD's in the table
//		Exit:		None
//
void BurstMemoryWrites (SEGOFF lpVideo, LPDWORD lpdwData, int nCount)
{
	int	i;

	for (i = 0; i < nCount; i++)
	{
		MemDwordWrite (lpVideo, *lpdwData);
		lpdwData++;
		lpVideo += 4;
	}
}

//
//		ClearModeX - Clear Mode X memory
//
//		Entry:	bPhysical	Physical usage flag (TRUE = Use physical, FALSE = Use library)
//		Exit:		None
//
void ClearModeX (BOOL bPhysical)
{
	LPDWORD	lpVideo;
	long int	i;

	lpVideo = (LPDWORD) 0xA0000000;

	IOWordWrite (0x3C4, 0x0F02);
	for (i = 0; i < 0x4000; i++)
	{
		if (bPhysical)
		{
			*lpVideo++ = 0;
		}
		else
		{
			MemDwordWrite ((SEGOFF) lpVideo, 0);
			lpVideo++;
		}
	}
}

//
//		WritePatternStrip - Write a strip of DWORD patterns
//
//		Entry:	lpStart		Start address on first scan
//					nScans		Number of scan lines
//					nRowOffset	Row offset between scans
//					lpPattern	Array of DWORD patterns for plane 0,1,2,3
//					bPhysical	Physical usage flag (TRUE = Use physical, FALSE = Use library)
//		Exit:		None
//
void WritePatternStrip (LPBYTE lpStart, int nScans, int nRowOffset, DWORD *lpPattern, BOOL bPhysical)
{
	int		i;
	LPDWORD	lpVideo;

	lpVideo = (LPDWORD) lpStart;
	IOByteWrite (0x3C4, 0x02);
	for (i = 0; i < nScans; i++)
	{
#ifdef __X86_16__
		if (bPhysical)
		{
			_outp (0x3C5, 0x01);
			*lpVideo = *lpPattern;
			_outp (0x3C5, 0x02);
			*lpVideo = *(lpPattern + 1);
			_outp (0x3C5, 0x04);
			*lpVideo = *(lpPattern + 2);
			_outp (0x3C5, 0x08);
			*lpVideo = *(lpPattern + 3);
		}
		else
		{
			IOByteWrite (0x3C5, 0x01);
			MemDwordWrite ((SEGOFF) lpVideo, *lpPattern);
			IOByteWrite (0x3C5, 0x02);
			MemDwordWrite ((SEGOFF) lpVideo, *(lpPattern + 1));
			IOByteWrite (0x3C5, 0x04);
			MemDwordWrite ((SEGOFF) lpVideo, *(lpPattern + 2));
			IOByteWrite (0x3C5, 0x08);
			MemDwordWrite ((SEGOFF) lpVideo, *(lpPattern + 3));
		}
#else
		bPhysical = bPhysical;				// Prevent compiler warning
		IOByteWrite (0x3C5, 0x01);
		MemDwordWrite ((SEGOFF) lpVideo, *lpPattern);
		IOByteWrite (0x3C5, 0x02);
		MemDwordWrite ((SEGOFF) lpVideo, *(lpPattern + 1));
		IOByteWrite (0x3C5, 0x04);
		MemDwordWrite ((SEGOFF) lpVideo, *(lpPattern + 2));
		IOByteWrite (0x3C5, 0x08);
		MemDwordWrite ((SEGOFF) lpVideo, *(lpPattern + 3));
#endif
		lpVideo += nRowOffset / 4;
	}
}

//
//		VerifyPatternStrip - Verify a strip of DWORD patterns
//
//		Entry:	lpStart		Start address on first scan
//					nScans		Number of scan lines
//					nRowOffset	Row offset between scans
//					lpPattern	Array of DWORD patterns for plane 0,1,2,3
//					bPhysical	Physical usage flag (TRUE = Use physical, FALSE = Use library)
//		Exit:		<BOOL>		Error flag (TRUE = Passed, FALSE = Error)
//
BOOL VerifyPatternStrip (LPBYTE lpStart, int nScans, int nRowOffset, DWORD *lpPattern, BOOL bPhysical)
{
	int		i, nPlane;
	LPDWORD	lpVideo;
	DWORD		dwActual;
	BOOL		bErr;

	int		nErrPlane = 0;
	DWORD		dwErrAct = 0;
	DWORD		dwErrExp = 0;
 LPBYTE	lpErrAddr = NULL;


	bErr = FALSE;
	lpVideo = (LPDWORD) lpStart;
	IOByteWrite (0x3CE, 0x04);
	for (i = 0; i < nScans; i++)
	{
#ifdef __X86_16__
		if (bPhysical)
		{
			for (nPlane = 0; nPlane < 4; nPlane++)
			{
				_outp (0x3CF, nPlane);
				dwActual = *lpVideo;
				if (*lpVideo != *(lpPattern + nPlane)) goto VerifyPatternStrip_error;
			}
		}
		else
		{
			for (nPlane = 0; nPlane < 4; nPlane++)
			{
				IOByteWrite (0x3CF, nPlane);
				dwActual = MemDwordRead ((SEGOFF) lpVideo);
				if (dwActual != *(lpPattern + nPlane)) goto VerifyPatternStrip_error;
			}
		}
#else
		bPhysical = bPhysical;							// Prevent compiler warning
		for (nPlane = 0; nPlane < 4; nPlane++)
		{
			IOByteWrite (0x3CF, nPlane);
			dwActual = MemDwordRead ((SEGOFF) lpVideo);
			if (dwActual != *(lpPattern + nPlane)) goto VerifyPatternStrip_error;
		}
#endif
		lpVideo += nRowOffset / 4;
	}

VerifyPatternStrip_exit:
	return (!bErr);

VerifyPatternStrip_error:
	nErrPlane = nPlane;
	lpErrAddr = (LPBYTE) lpVideo;
	bErr = TRUE;
	dwErrAct = dwActual;
	dwErrExp = *(lpPattern + nPlane);
	goto VerifyPatternStrip_exit;
}

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

//
//		MemoryAccessTest - Verify memory can be accessed as BYTE, WORD, and DWORD
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//

int MemoryAccessTest (void);
int MemoryAccessSubTest (void);

int MemoryAccessTest (void)
{
	#define	ALLOW_UNALIGNED	1

	//	Register definition for VGA Sequencer
	#define SEQ_INDEX					0x3C4
	#define SEQ_DATA					0x3C5
	#define SEQ_RESET					0x00
	#define BIT_ERAM					0x02


	
	int	nErr;
	WORD	wSimType;
	BOOL	bFullVGA;
	BYTE	tmpvar;

	nErr = ERROR_NONE;
  //	wSimType = SimGetType ();
 //	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

//	SimSetState (TRUE, FALSE, FALSE);
//	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	
	SetMode (0x12);
//	SimSetState (TRUE, TRUE, TRUE);

//	SimDumpMemory ("T0919.VGA");

	// Do test first in planar mode
	nErr = MemoryAccessSubTest ();
	if (nErr != ERROR_NONE){
	 printf("memaccess subtest failed");
	 goto MemoryAccessTest_exit;
	}
	// Set chain/4 and repeat test
	IOWordWrite (SEQ_INDEX, 0x0E04);
	nErr = MemoryAccessSubTest ();
   if (nErr != ERROR_NONE){
	 printf("memaccess chain4 test failed");
	 goto MemoryAccessTest_exit;
	}
	// Set chain/2 and odd/even modes and repeat test
	IOWordWrite (SEQ_INDEX, 0x0204);			// Clear Chain/4, Set odd/even
	IOWordWrite (GDC_INDEX, 0x1005);			// Set odd/even
	IOWordWrite (GDC_INDEX, 0x0706);			// Set chain/2
	nErr = MemoryAccessSubTest ();
	if (nErr != ERROR_NONE){
	 printf("memaccess chain2 test failed");

	}
MemoryAccessTest_exit:
	SystemCleanUp ();
	return (nErr);
}

//
//		MemoryAccessSubTest - Test various BYTE, WORD, and DWORD accesses
//
//		Entry:	None
//
int MemoryAccessSubTest (void)
{

	int		nErr, i, j;
	SEGOFF	lpVideo;
	BYTE		byData0, byData1, byData2, byData3;
	WORD		wData;
	DWORD		dwData;
	BYTE		tblBytePattern[16] = {
					0x00, 0x55, 0xAA, 0xFF, 0x66, 0x99, 0x69, 0x96,
					0xC3, 0x3C, 0x5A, 0xA5, 0xDB, 0xBD, 0x81, 0x18
				};
	WORD		tblWordPattern[8] = {
					0x0FF0, 0x5AA5, 0xA55A, 0xF00F, 0x9669, 0x6996, 0xC33C, 0xDBBD
				};
	DWORD		tblDWordPattern[4] = {
					0xC00F0CF0, 0xE77E5AA5, 0x6996C33C, 0xBDDB1881
				};
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	
	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;

	// Write BYTEs to 16 locations and verify as BYTE, WORD, and DWORD
	for (i = 0; i < 16; i++)
	{
		MemByteWrite (lpVideo + i, tblBytePattern[i]);
	}
	for (i = 0; i < 16; i++)
	{
		byData0 = MemByteRead (lpVideo + i);
		if (byData0 != tblBytePattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, i, HIWORD (lpVideo), tblBytePattern[i], byData0);
			goto MemoryAccessSubTest_exit;
		}
	}
	for (i = 0; i < 8; i+=2)
	{
		wData = MemWordRead (lpVideo + i);
		byData0 = LOBYTE (wData);
		byData1 = HIBYTE (wData);
		if (byData0 != tblBytePattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, i, HIWORD (lpVideo), tblBytePattern[i], byData0);
			goto MemoryAccessSubTest_exit;
		}
		if (byData1 != tblBytePattern[i + 1])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, i, HIWORD (lpVideo), tblBytePattern[i+1], byData1);
			goto MemoryAccessSubTest_exit;
		}
	}
	for (i = 0; i < 4; i+=4)
	{
		dwData = MemDwordRead (lpVideo + i);
		byData0 = LOBYTE (LOWORD (dwData));
		byData1 = HIBYTE (LOWORD (dwData));
		byData2 = LOBYTE (HIWORD (dwData));
		byData3 = HIBYTE (HIWORD (dwData));
		if (byData0 != tblBytePattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, i, HIWORD (lpVideo), tblBytePattern[i], byData0);
			goto MemoryAccessSubTest_exit;
		}
		if (byData1 != tblBytePattern[i + 1])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, i, HIWORD (lpVideo), tblBytePattern[i+1], byData1);
			goto MemoryAccessSubTest_exit;
		}
		if (byData2 != tblBytePattern[i + 2])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, i, HIWORD (lpVideo), tblBytePattern[i+2], byData2);
			goto MemoryAccessSubTest_exit;
		}
		if (byData3 != tblBytePattern[i + 3])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, i, HIWORD (lpVideo), tblBytePattern[i+3], byData3);
			goto MemoryAccessSubTest_exit;
		}
	}

	// Write WORDs to 16 locations as WORD aligned and verify (as BYTES)
	for (i = 0, j = 0; i < 8; i++, j+=2)
	{
		MemWordWrite (lpVideo + j, tblWordPattern[i]);
	}
	for (i = 0, j = 0; i < 8; i++, j+=2)
	{
		byData0 = MemByteRead (lpVideo + j);
		byData1 = MemByteRead (lpVideo + j + 1);
		wData = ((WORD) byData1 << 8) | (WORD) byData0;
		if (wData != tblWordPattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), tblWordPattern[i], wData);
			goto MemoryAccessSubTest_exit;
		}
	}

#ifdef ALLOW_UNALIGNED
	// Write WORDs to 16 locations as non-aligned
	for (i = 0, j = 1; i < 8; i++, j+=2)
	{
		MemWordWrite (lpVideo + j, tblWordPattern[i]);
	}
	for (i = 0, j = 1; i < 8; i++, j+=2)
	{
		// First verify that an unaligned WORD read can take place and matches
		// the data written.
		wData = MemWordRead (lpVideo + j);
		if (wData != tblWordPattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), tblWordPattern[i], wData);
			goto MemoryAccessSubTest_exit;
		}
		// Next verify that the WORD write worked correctly and that the data
		// written is as expected.
		byData0 = MemByteRead (lpVideo + j);
		byData1 = MemByteRead (lpVideo + j + 1);
		wData = ((WORD) byData1 << 8) | (WORD) byData0;
		if (wData != tblWordPattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), tblWordPattern[i], wData);
			goto MemoryAccessSubTest_exit;
		}
	}
#endif

	// Write DWORDs to 16 locations as DWORD aligned (starting on BYTE 0)
	for (i = 0, j = 0; i < 4; i++, j+=4)
	{
		MemDwordWrite (lpVideo + j, tblDWordPattern[i]);
	}
	for (i = 0, j = 0; i < 4; i++, j+=4)
	{
		byData0 = MemByteRead (lpVideo + j);
		byData1 = MemByteRead (lpVideo + j + 1);
		byData2 = MemByteRead (lpVideo + j + 2);
		byData3 = MemByteRead (lpVideo + j + 3);
		dwData = ((DWORD) byData3 << 24) | ((DWORD) byData2 << 16) | ((DWORD) byData1 << 8) | (DWORD) byData0;
		if (dwData != tblDWordPattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), tblDWordPattern[i], dwData);
			goto MemoryAccessSubTest_exit;
		}
	}

#ifdef ALLOW_UNALIGNED
	// Write DWORDs to 16 locations non-aligned starting on BYTE 1
	for (i = 0, j = 1; i < 4; i++, j+=4)
	{
		MemDwordWrite (lpVideo + j, tblDWordPattern[i]);
	}
	for (i = 0, j = 1; i < 4; i++, j+=4)
	{
		// First verify that an unaligned DWORD read can take place and
		// matches the data written.
		dwData = MemDwordRead (lpVideo + j);
		if (dwData != tblDWordPattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), tblDWordPattern[i], dwData);
			goto MemoryAccessSubTest_exit;
		}
		// Next verify that the DWORD write worked correctly and that the data
		// written is as expected.
		byData0 = MemByteRead (lpVideo + j);
		byData1 = MemByteRead (lpVideo + j + 1);
		byData2 = MemByteRead (lpVideo + j + 2);
		byData3 = MemByteRead (lpVideo + j + 3);
		dwData = ((DWORD) byData3 << 24) | ((DWORD) byData2 << 16) | ((DWORD) byData1 << 8) | (DWORD) byData0;
		if (dwData != tblDWordPattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), tblDWordPattern[i], dwData);
			goto MemoryAccessSubTest_exit;
		}
	}

	// Write DWORDs to 16 locations non-aligned starting on BYTE 2
	for (i = 0, j = 2; i < 4; i++, j+=4)
	{
		MemDwordWrite (lpVideo + j, tblDWordPattern[i]);
	}
	for (i = 0, j = 2; i < 4; i++, j+=4)
	{
		// First verify that an unaligned DWORD read can take place and
		// matches the data written.
		dwData = MemDwordRead (lpVideo + j);
		if (dwData != tblDWordPattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), tblDWordPattern[i], dwData);
			goto MemoryAccessSubTest_exit;
		}
		// Next verify that the DWORD write worked correctly and that the data
		// written is as expected.
		byData0 = MemByteRead (lpVideo + j);
		byData1 = MemByteRead (lpVideo + j + 1);
		byData2 = MemByteRead (lpVideo + j + 2);
		byData3 = MemByteRead (lpVideo + j + 3);
		dwData = ((DWORD) byData3 << 24) | ((DWORD) byData2 << 16) | ((DWORD) byData1 << 8) | (DWORD) byData0;
		if (dwData != tblDWordPattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), tblDWordPattern[i], dwData);
			goto MemoryAccessSubTest_exit;
		}
	}

	// Write DWORDs to 16 locations non-aligned starting on BYTE 3
	for (i = 0, j = 3; i < 4; i++, j+=4)
	{
		MemDwordWrite (lpVideo + j, tblDWordPattern[i]);
	}
	for (i = 0, j = 3; i < 4; i++, j+=4)
	{
		// First verify that an unaligned DWORD read can take place and
		// matches the data written.
		dwData = MemDwordRead (lpVideo + j);
		if (dwData != tblDWordPattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), tblDWordPattern[i], dwData);
			goto MemoryAccessSubTest_exit;
		}
		// Next verify that the DWORD write worked correctly and that the data
		// written is as expected.
		byData0 = MemByteRead (lpVideo + j);
		byData1 = MemByteRead (lpVideo + j + 1);
		byData2 = MemByteRead (lpVideo + j + 2);
		byData3 = MemByteRead (lpVideo + j + 3);
		dwData = ((DWORD) byData3 << 24) | ((DWORD) byData2 << 16) | ((DWORD) byData1 << 8) | (DWORD) byData0;
		if (dwData != tblDWordPattern[i])
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), tblDWordPattern[i], dwData);
			goto MemoryAccessSubTest_exit;
		}
	}
#endif

MemoryAccessSubTest_exit:
	return (nErr);
}
//




 //
//		ModeXTest - Verify mode 'X' style test
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int ModeXTest (void)
{
	static BYTE	dac[256][3] = {
		// Vary blue by +2
		{0x0, 0x0, 0x0},		// Index 0
		{0x0, 0x0, 0x2},		// Index 1
		{0x0, 0x0, 0x4},		// Index 2
		{0x0, 0x0, 0x6},		// Index 3
		{0x0, 0x0, 0x8},		// Index 4
		{0x0, 0x0, 0x0A},		// Index 5
		{0x0, 0x0, 0x0C},		// Index 6
		{0x0, 0x0, 0x0E},		// Index 7
		{0x0, 0x0, 0x10},		// Index 8
		{0x0, 0x0, 0x12},		// Index 9
		{0x0, 0x0, 0x14},		// Index 10
		{0x0, 0x0, 0x16},		// Index 11
		{0x0, 0x0, 0x18},		// Index 12
		{0x0, 0x0, 0x1A},		// Index 13
		{0x0, 0x0, 0x1C},		// Index 14
		{0x0, 0x0, 0x1E},		// Index 15
		{0x0, 0x0, 0x20},		// Index 16
		{0x0, 0x0, 0x22},		// Index 17
		{0x0, 0x0, 0x24},		// Index 18
		{0x0, 0x0, 0x26},		// Index 19
		{0x0, 0x0, 0x28},		// Index 20
		{0x0, 0x0, 0x2A},		// Index 21
		{0x0, 0x0, 0x2C},		// Index 22
		{0x0, 0x0, 0x2E},		// Index 23
		{0x0, 0x0, 0x30},		// Index 24
		{0x0, 0x0, 0x32},		// Index 25
		{0x0, 0x0, 0x34},		// Index 26
		{0x0, 0x0, 0x36},		// Index 27
		{0x0, 0x0, 0x38},		// Index 28
		{0x0, 0x0, 0x3A},		// Index 29
		{0x0, 0x0, 0x3C},		// Index 30
		{0x0, 0x0, 0x3F},		// Index 31
		// Vary green by +2
		{0x0, 0x02, 0x03F},		// Index 32
		{0x0, 0x04, 0x03F},		// Index 33
		{0x0, 0x06, 0x03F},		// Index 34
		{0x0, 0x08, 0x03F},		// Index 35
		{0x0, 0x0A, 0x03F},		// Index 36
		{0x0, 0x0C, 0x03F},		// Index 37
		{0x0, 0x0E, 0x03F},		// Index 38
		{0x0, 0x10, 0x03F},		// Index 39
		{0x0, 0x12, 0x03F},		// Index 40
		{0x0, 0x14, 0x03F},		// Index 41
		{0x0, 0x16, 0x03F},		// Index 42
		{0x0, 0x18, 0x03F},		// Index 43
		{0x0, 0x1A, 0x03F},		// Index 44
		{0x0, 0x1C, 0x03F},		// Index 45
		{0x0, 0x1E, 0x03F},		// Index 46
		{0x0, 0x20, 0x03F},		// Index 47
		{0x0, 0x22, 0x03F},		// Index 48
		{0x0, 0x24, 0x03F},		// Index 49
		{0x0, 0x26, 0x03F},		// Index 50
		{0x0, 0x28, 0x03F},		// Index 51
		{0x0, 0x2A, 0x03F},		// Index 52
		{0x0, 0x2C, 0x03F},		// Index 53
		{0x0, 0x2E, 0x03F},		// Index 54
		{0x0, 0x30, 0x03F},		// Index 55
		{0x0, 0x32, 0x03F},		// Index 56
		{0x0, 0x34, 0x03F},		// Index 57
		{0x0, 0x36, 0x03F},		// Index 58
		{0x0, 0x38, 0x03F},		// Index 59
		{0x0, 0x3A, 0x03F},		// Index 60
		{0x0, 0x3C, 0x03F},		// Index 61
		{0x0, 0x3F, 0x03F},		// Index 62
		// Vary blue by -2
		{0x0, 0x3F, 0x3C},		// Index 63
		{0x0, 0x3F, 0x3A},		// Index 64
		{0x0, 0x3F, 0x38},		// Index 65
		{0x0, 0x3F, 0x36},		// Index 66
		{0x0, 0x3F, 0x34},		// Index 67
		{0x0, 0x3F, 0x32},		// Index 68
		{0x0, 0x3F, 0x30},		// Index 69
		{0x0, 0x3F, 0x2E},		// Index 70
		{0x0, 0x3F, 0x2C},		// Index 71
		{0x0, 0x3F, 0x2A},		// Index 72
		{0x0, 0x3F, 0x28},		// Index 73
		{0x0, 0x3F, 0x26},		// Index 74
		{0x0, 0x3F, 0x24},		// Index 75
		{0x0, 0x3F, 0x22},		// Index 76
		{0x0, 0x3F, 0x20},		// Index 77
		{0x0, 0x3F, 0x1E},		// Index 78
		{0x0, 0x3F, 0x1C},		// Index 79
		{0x0, 0x3F, 0x1A},		// Index 80
		{0x0, 0x3F, 0x18},		// Index 81
		{0x0, 0x3F, 0x16},		// Index 82
		{0x0, 0x3F, 0x14},		// Index 83
		{0x0, 0x3F, 0x12},		// Index 84
		{0x0, 0x3F, 0x10},		// Index 85
		{0x0, 0x3F, 0x0E},		// Index 86
		{0x0, 0x3F, 0x0C},		// Index 87
		{0x0, 0x3F, 0x0A},		// Index 88
		{0x0, 0x3F, 0x08},		// Index 89
		{0x0, 0x3F, 0x06},		// Index 90
		{0x0, 0x3F, 0x04},		// Index 91
		{0x0, 0x3F, 0x02},		// Index 92
		{0x0, 0x3F, 0x00},		// Index 93
		// Vary red by +2
		{0x02, 0x03F, 0x00},	// Index 94
		{0x04, 0x03F, 0x00},	// Index 95
		{0x06, 0x03F, 0x00},	// Index 96
		{0x08, 0x03F, 0x00},	// Index 97
		{0x0A, 0x03F, 0x00},	// Index 98
		{0x0C, 0x03F, 0x00},	// Index 99
		{0x0E, 0x03F, 0x00},	// Index 100
		{0x10, 0x03F, 0x00},	// Index 101
		{0x12, 0x03F, 0x00},	// Index 102
		{0x14, 0x03F, 0x00},	// Index 103
		{0x16, 0x03F, 0x00},	// Index 104
		{0x18, 0x03F, 0x00},	// Index 105
		{0x1A, 0x03F, 0x00},	// Index 106
		{0x1C, 0x03F, 0x00},	// Index 107
		{0x1E, 0x03F, 0x00},	// Index 108
		{0x20, 0x03F, 0x00},	// Index 109
		{0x22, 0x03F, 0x00},	// Index 110
		{0x24, 0x03F, 0x00},	// Index 111
		{0x26, 0x03F, 0x00},	// Index 112
		{0x28, 0x03F, 0x00},	// Index 113
		{0x2A, 0x03F, 0x00},	// Index 114
		{0x2C, 0x03F, 0x00},	// Index 115
		{0x2E, 0x03F, 0x00},	// Index 116
		{0x30, 0x03F, 0x00},	// Index 117
		{0x32, 0x03F, 0x00},	// Index 118
		{0x34, 0x03F, 0x00},	// Index 119
		{0x36, 0x03F, 0x00},	// Index 120
		{0x38, 0x03F, 0x00},	// Index 121
		{0x3A, 0x03F, 0x00},	// Index 122
		{0x3C, 0x03F, 0x00},	// Index 123
		{0x3F, 0x03F, 0x00},	// Index 124
		// Vary green by -2
		{0x3F, 0x03C, 0x00},	// Index 125
		{0x3F, 0x03A, 0x00},	// Index 126
		{0x3F, 0x038, 0x00},	// Index 127
		{0x3F, 0x036, 0x00},	// Index 128
		{0x3F, 0x034, 0x00},	// Index 129
		{0x3F, 0x032, 0x00},	// Index 130
		{0x3F, 0x030, 0x00},	// Index 131
		{0x3F, 0x02E, 0x00},	// Index 132
		{0x3F, 0x02C, 0x00},	// Index 133
		{0x3F, 0x02A, 0x00},	// Index 134
		{0x3F, 0x028, 0x00},	// Index 135
		{0x3F, 0x026, 0x00},	// Index 136
		{0x3F, 0x024, 0x00},	// Index 137
		{0x3F, 0x022, 0x00},	// Index 138
		{0x3F, 0x020, 0x00},	// Index 139
		{0x3F, 0x01E, 0x00},	// Index 140
		{0x3F, 0x01C, 0x00},	// Index 141
		{0x3F, 0x01A, 0x00},	// Index 142
		{0x3F, 0x018, 0x00},	// Index 143
		{0x3F, 0x016, 0x00},	// Index 144
		{0x3F, 0x014, 0x00},	// Index 145
		{0x3F, 0x012, 0x00},	// Index 146
		{0x3F, 0x010, 0x00},	// Index 147
		{0x3F, 0x00E, 0x00},	// Index 148
		{0x3F, 0x00C, 0x00},	// Index 149
		{0x3F, 0x00A, 0x00},	// Index 150
		{0x3F, 0x008, 0x00},	// Index 151
		{0x3F, 0x006, 0x00},	// Index 152
		{0x3F, 0x004, 0x00},	// Index 153
		{0x3F, 0x002, 0x00},	// Index 154
		{0x3F, 0x000, 0x00},	// Index 155
		// Vary blue by +2
		{0x3F, 0x000, 0x02},	// Index 156
		{0x3F, 0x000, 0x04},	// Index 157
		{0x3F, 0x000, 0x06},	// Index 158
		{0x3F, 0x000, 0x08},	// Index 159
		{0x3F, 0x000, 0x0A},	// Index 160
		{0x3F, 0x000, 0x0C},	// Index 161
		{0x3F, 0x000, 0x0E},	// Index 162
		{0x3F, 0x000, 0x10},	// Index 163
		{0x3F, 0x000, 0x12},	// Index 164
		{0x3F, 0x000, 0x14},	// Index 165
		{0x3F, 0x000, 0x16},	// Index 166
		{0x3F, 0x000, 0x18},	// Index 167
		{0x3F, 0x000, 0x1A},	// Index 168
		{0x3F, 0x000, 0x1C},	// Index 169
		{0x3F, 0x000, 0x1E},	// Index 170
		{0x3F, 0x000, 0x20},	// Index 171
		{0x3F, 0x000, 0x22},	// Index 172
		{0x3F, 0x000, 0x24},	// Index 173
		{0x3F, 0x000, 0x26},	// Index 174
		{0x3F, 0x000, 0x28},	// Index 175
		{0x3F, 0x000, 0x2A},	// Index 176
		{0x3F, 0x000, 0x2C},	// Index 177
		{0x3F, 0x000, 0x2E},	// Index 178
		{0x3F, 0x000, 0x30},	// Index 179
		{0x3F, 0x000, 0x32},	// Index 180
		{0x3F, 0x000, 0x34},	// Index 181
		{0x3F, 0x000, 0x36},	// Index 182
		{0x3F, 0x000, 0x38},	// Index 183
		{0x3F, 0x000, 0x3A},	// Index 184
		{0x3F, 0x000, 0x3C},	// Index 185
		{0x3F, 0x000, 0x3F},	// Index 186
		// Vary green by +2
		{0x3F, 0x002, 0x3F},	// Index 187
		{0x3F, 0x004, 0x3F},	// Index 188
		{0x3F, 0x006, 0x3F},	// Index 189
		{0x3F, 0x008, 0x3F},	// Index 190
		{0x3F, 0x00A, 0x3F},	// Index 191
		{0x3F, 0x00C, 0x3F},	// Index 192
		{0x3F, 0x00E, 0x3F},	// Index 193
		{0x3F, 0x010, 0x3F},	// Index 194
		{0x3F, 0x012, 0x3F},	// Index 195
		{0x3F, 0x014, 0x3F},	// Index 196
		{0x3F, 0x016, 0x3F},	// Index 197
		{0x3F, 0x018, 0x3F},	// Index 198
		{0x3F, 0x01A, 0x3F},	// Index 199
		{0x3F, 0x01C, 0x3F},	// Index 200
		{0x3F, 0x01E, 0x3F},	// Index 201
		{0x3F, 0x020, 0x3F},	// Index 202
		{0x3F, 0x022, 0x3F},	// Index 203
		{0x3F, 0x024, 0x3F},	// Index 204
		{0x3F, 0x026, 0x3F},	// Index 205
		{0x3F, 0x028, 0x3F},	// Index 206
		{0x3F, 0x02A, 0x3F},	// Index 207
		{0x3F, 0x02C, 0x3F},	// Index 208
		{0x3F, 0x02E, 0x3F},	// Index 209
		{0x3F, 0x030, 0x3F},	// Index 210
		{0x3F, 0x032, 0x3F},	// Index 211
		{0x3F, 0x034, 0x3F},	// Index 212
		{0x3F, 0x036, 0x3F},	// Index 213
		{0x3F, 0x038, 0x3F},	// Index 214
		{0x3F, 0x03A, 0x3F},	// Index 215
		{0x3F, 0x03C, 0x3F},	// Index 216
		{0x3F, 0x03E, 0x3F},	// Index 217
		// Vary (approximately) red by -2, green by -2, blue by -2
		{0x3F, 0x03F, 0x3F},	// Index 218
		{0x3E, 0x03E, 0x3E},	// Index 219
		{0x3D, 0x03D, 0x3D},	// Index 220
		{0x3C, 0x03C, 0x3C},	// Index 221
		{0x3B, 0x03B, 0x3B},	// Index 222
		{0x3A, 0x03A, 0x3A},	// Index 223
		{0x39, 0x039, 0x39},	// Index 224
		{0x38, 0x038, 0x38},	// Index 225
		{0x37, 0x037, 0x37},	// Index 226
		{0x36, 0x036, 0x36},	// Index 227
		{0x34, 0x034, 0x34},	// Index 228
		{0x32, 0x032, 0x32},	// Index 229
		{0x30, 0x030, 0x30},	// Index 230
		{0x2E, 0x02E, 0x2E},	// Index 231
		{0x2C, 0x02C, 0x2C},	// Index 232
		{0x2A, 0x02A, 0x2A},	// Index 233
		{0x28, 0x028, 0x28},	// Index 234
		{0x26, 0x026, 0x26},	// Index 235
		{0x24, 0x024, 0x24},	// Index 236
		{0x22, 0x022, 0x22},	// Index 237
		{0x20, 0x020, 0x20},	// Index 238
		{0x1E, 0x01E, 0x1E},	// Index 239
		{0x1C, 0x01C, 0x1C},	// Index 240
		{0x1A, 0x01A, 0x1A},	// Index 241
		{0x18, 0x018, 0x18},	// Index 241
		{0x16, 0x016, 0x16},	// Index 243
		{0x14, 0x014, 0x14},	// Index 244
		{0x12, 0x012, 0x12},	// Index 245
		{0x10, 0x010, 0x10},	// Index 246
		{0x0E, 0x00E, 0x0E},	// Index 247
		{0x0C, 0x00C, 0x0C},	// Index 248
		{0x0A, 0x00A, 0x0A},	// Index 249
		{0x08, 0x008, 0x08},	// Index 250
		{0x06, 0x006, 0x06},	// Index 251
		{0x04, 0x004, 0x04},	// Index 252
		{0x02, 0x002, 0x02},	// Index 253
		{0x01, 0x001, 0x01},	// Index 254
		{0x00, 0x000, 0x00}	// Index 255
	};
	static PARMENTRY	parmX = {					// 320x400x8
		0x50, 0x1D, 0x10,
		0xFF, 0xFF,
		{0x01, 0x0F, 0x00, 0x06},					// SEQ 1..4
		0x63,												// Misc
		{0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80,	// CRTC 0..18h
		0xBF, 0x1F, 0x20, 0x40, 0x00, 0x00,
		0xFF, 0xFF, 0x00, 0x00, 0x9C, 0x0E,
		0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
		0xFF},
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	// ATC 0..13h
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00,
		0x0F, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x40,	// GDC 0..8
		0x05, 0x0F, 0xFF}
	};
	static PARMENTRY	parmSmallX = {
		0x28, 0x1D, 0x10,
		0xFF, 0xFF,
		{0x01, 0x0F, 0x00, 0x06},					// SEQ 1..4
		0x63,												// Misc
		{0x2D, 0x27, 0x28, 0x90, 0x2B, 0x80,	// CRTC 0..18h
		0x17, 0x10, 0x20, 0x40, 0x00, 0x00,
		0xFF, 0xFF, 0x00, 0x00, 0x15, 0x06,
		0x13, 0x14, 0x00, 0x14, 0x17, 0xE3,
		0xFF},
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	// ATC 0..13h
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00,
		0x0F, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x40,	// GDC 0..8
		0x05, 0x0F, 0xFF}
	};
	int		nErr, i, j;
	int	REF_PART = 1;
	int REF_TEST = 33;
	
	SEGOFF	lpVideo;
	BYTE		clr, clrOrg;
	WORD		wSimType, wXRes, wYRes;
	BOOL		bFullVGA;

	nErr = ERROR_NONE;
    wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	//if (!DisplayInspectionMessage ())
	  //	goto ModeXTest_exit;

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);

	SetDacBlock ((LPBYTE) dac, 0, 256);

	// Load the registers
	if (bFullVGA)
	{
		SetRegs (&parmX);
		wXRes = 320;
		wYRes = 400;
	}
	else
	{
		SetRegs (&parmSmallX);
		wXRes = 160;
		wYRes = 20;
	}

	// Draw a "cool" pattern into memory
	clr = clrOrg = 0;
	for (i = 0; i < (int) wXRes; i++)
	{
		lpVideo = (SEGOFF) (0xA0000000) + (i / 4);
		j = 1 << (i & 3);
		IOByteWrite (SEQ_INDEX, 0x02);
		IOByteWrite (SEQ_DATA, (BYTE) j);
		clr = clrOrg++;
		for (j = 0; j < (int) (wYRes - 1); j++)
		{
			MemByteWrite (lpVideo, clr++);
			lpVideo += wXRes/4;
		}
	}

	SimDumpMemory ("T0918.VGA");

	if (!FrameCapture (REF_PART, REF_TEST))
	{ 
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto ModeXTest_exit;
		}
	}

ModeXTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//


//
//		HiResColor1Test - Verify C&T Test #4 compatible operation
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int HiResColor1Test (void)
{
	static PARMENTRY	parmHiResColor1 = {		// 1280x400x2
		0xA0, 0x31, 0x08,
		0xFF, 0xFF,
		{0x01, 0x0F, 0x00, 0x0E},					// SEQ 1..4
		0x63,												// Misc
		{0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80,	// CRTC 0..18h
		0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
		0xFF, 0xFF, 0x00, 0x00, 0x9C, 0x0E,
		0x8F, 0x28, 0x40, 0x96, 0xB9, 0xE3,
		0xFF},
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	// ATC 0..13h
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00,
		0x0F, 0x07},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x40,	// GDC 0..8
		0x05, 0x0F, 0xFF}
	};
	static PARMENTRY	parmSmallHiResColor1 = {// 1280x400x2
		0x50, 0x31, 0x08,
		0xFF, 0xFF,
		{0x01, 0x0F, 0x00, 0x0E},					// SEQ 1..4
		0x63,												// Misc
		{0x2D, 0x27, 0x28, 0x90, 0x2B, 0x80,	// CRTC 0..18h
		0x17, 0x10, 0x00, 0x41, 0x00, 0x00,
		0xFF, 0xFF, 0x00, 0x00, 0x15, 0x06,
		0x13, 0x14, 0x40, 0x14, 0x17, 0xE3,
		0xFF},
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	// ATC 0..13h
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00,
		0x0F, 0x07},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x40,	// GDC 0..8
		0x05, 0x0F, 0xFF}
	};
	BYTE	dacTable[] = {
		0x00, 0x00, 0x00,
		0x20, 0x00, 0x00,
		0x00, 0x20, 0x00,
		0x2A, 0x2A, 0x2A,
		0x20, 0x00, 0x00,
		0x2A, 0x00, 0x00,
		0x2A, 0x2A, 0x00,
		0x3F, 0x20, 0x20,
		0x00, 0x20, 0x00,
		0x2A, 0x2A, 0x00,
		0x00, 0x2A, 0x00,
		0x20, 0x3F, 0x20,
		0x2A, 0x2A, 0x2A,
		0x3F, 0x20, 0x20,
		0x20, 0x3F, 0x20,
		0x3F, 0x3F, 0x3F
	};
	int		nErr;
	SEGOFF	lpVideo, lpVideoLeft, lpVideoRight;
	WORD		i, j, k, n;
	BYTE		chr;
	BYTE		table[] = {0xFF, 0xAA, 0x55};
	WORD		wSimType;
	BOOL		bFullVGA;
	int		nRowOffset, nLineCount, nLastLine, nLineSpace;
	int REF_PART = 1;
	int REF_TEST = 17;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;
 //	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	//if (!DisplayInspectionMessage ())
	//	goto HiResColor1Test_exit;

 //	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
 //	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x13);
   //	SimSetState (TRUE, TRUE, TRUE);

	// Load the registers
	if (bFullVGA)
	{
		SetRegs (&parmHiResColor1);
		nRowOffset = 320;
		nLastLine = 199;
		nLineCount = 21;
		nLineSpace = 3;
	}
	else
	{
		SetRegs (&parmSmallHiResColor1);
		nRowOffset = 160;
		nLastLine = 9;
		nLineCount = 3;
		nLineSpace = 2;
	}

	IOByteWrite (DAC_MASK, 0x0F);
	SetDacBlock (dacTable, 0, 16);

	for (i = 0; i < (WORD) nLineCount; i++)
	{
		chr = table[i%3];
		lpVideoLeft = lpVideo + (nRowOffset + 1)*i*nLineSpace;
		lpVideoRight = lpVideoLeft + (nRowOffset - i*nLineSpace*2) - 1;
		k = nRowOffset - i*nLineSpace*2;
		n = i*nLineSpace + nRowOffset*(nLastLine - i*nLineSpace);
		for (j = 0; j < k; j++)
		{
			MemByteWrite (lpVideoLeft + j, chr);
			MemByteWrite (lpVideo + n + j, chr);
		}
		for (j = 0; j < (nLastLine - i*nLineSpace*2); j++)
		{
			MemByteWrite (lpVideoLeft, (BYTE) (MemByteRead (lpVideoLeft) | (chr & 0xF0)));
			MemByteWrite (lpVideoRight, (BYTE) (MemByteRead (lpVideoRight) | (chr & 0x0F)));
			lpVideoLeft += nRowOffset;
			lpVideoRight += nRowOffset;
		}
	}

	SimDumpMemory ("T0917.VGA");

   //	if (!FrameCapture (REF_PART, REF_TEST))
   //	{
   //		if (GetKey () == KEY_ESCAPE)
   //		{
   //			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
   //			goto HiResColor1Test_exit;
   //		}
   //	}

HiResColor1Test_exit:
   	SystemCleanUp ();
	return (nErr);
}
//
//	
//
//		HiResMono2Test - Verify C&T Test #1 compatible operation
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int HiResMono2Test (void)
{
	static PARMENTRY	parmHiResMono1 = {
		0xA0, 0x31, 0x08,
		0xFF, 0xFF,
		{0x11, 0x0F, 0x00, 0x0E},					// SEQ 1..4
		0x63,												// Misc
		{0x5F, 0x50, 0x52, 0xE2, 0x54, 0xE0,	// CRTC 0..18h
		0xBF, 0x1F, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E,
		0x8F, 0x14, 0x60, 0x96, 0xB9, 0xE3,
		0xFF},
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	// ATC 0..13h
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00,
		0x01, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// GDC 0..8
		0x05, 0x0F, 0xFF}
	};
	static PARMENTRY	parmSmallHiResMono1 = {
		0x50, 0x31, 0x08,
		0xFF, 0xFF,
		{0x11, 0x0F, 0x00, 0x0E},					// SEQ 1..4
		0x63,												// Misc
		{0x2D, 0x27, 0x28, 0xF0, 0x2B, 0xE0,	// CRTC 0..18h
		0x17, 0x10, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x15, 0x06,
		0x13, 0x0A, 0x60, 0x14, 0x17, 0xE3,
		0xFF},
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	// ATC 0..13h
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00,
		0x01, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// GDC 0..8
		0x05, 0x0F, 0xFF}
	};
	int		nErr, i, j, nPans, nRowCount, nColCount;
	SEGOFF	lpVideo;
	BYTE		chr;
	WORD		wSimType;
	BOOL		bFullVGA, bCapture;
	int	REF_PART = 1;
	int	REF_TEST = 14;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto HiResMono2Test_exit;

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);

	// Load the registers
	if (bFullVGA)
	{
		SetRegs (&parmHiResMono1);
		nRowCount = 50;
		nColCount = 160;
	}
	else
	{
		SetRegs (&parmSmallHiResMono1);
		nRowCount = 3;
		nColCount = 80;
	}

	IOByteWrite (0x3C6, 0x03);
	SetDac (0x01, 0x20, 0x20, 0x20);
	SetDac (0x02, 0x20, 0x20, 0x20);
	SetDac (0x03, 0x2A, 0x2A, 0x2A);

	chr = 1;
	for (i = 0; i < nRowCount; i++)
		for (j = 0; j < nColCount; j++)
			DrawMonoChar (j, i, chr++, tblFont8x8, 8, nColCount);

	SimDumpMemory ("T0916.VGA");

	StartCapture (1);
	bCapture = FrameCapture (REF_PART, REF_TEST);
	if (!bCapture)
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto HiResMono2Test_exit;
		}
	}

	// Now pan the beast
	nPans = 640;
	if (bCapture) nPans = 34;
	for (i = 0; i < nPans; i++)
	{
		WaitNotVerticalRetrace ();
	   // stub out we cant have kbb wait for diagnostics
	   //	if (!FrameCapture (REF_PART, REF_TEST))
	   //	{
	   //		if (_kbhit ())
	   //		{
	   //			if (GetKey () == KEY_ESCAPE)
	   //			{
	   //				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	   //				goto HiResMono2Test_exit;
	   //			}
	   //		}
	   //	}
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x33);
		IOByteWrite (ATC_INDEX, (BYTE) (i & 0x7));					// Pixel panning
		IOByteWrite (CRTC_CINDEX, 0x08);
		IOByteWrite (CRTC_CDATA, (BYTE) ((i & 0x18) << 2));		// Byte panning
		IOByteWrite (CRTC_CINDEX, 0x0C);
		IOByteWrite (CRTC_CDATA, (BYTE) (i >> 13));					// Display start high
		IOByteWrite (CRTC_CINDEX, 0x0D);
		IOByteWrite (CRTC_CDATA, (BYTE) ((i >> 5) & 0xFF));		// Display start low
	}

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

HiResMono2Test_exit:
	EndCapture ();
	SystemCleanUp ();
	return (nErr);
}





//
//		HiResMono1Test - Verify C&T Test #2 compatible operation
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int HiResMono1Test (void)
{
	static PARMENTRY	parmHiResMono1 = {
		0xA0, 0x31, 0x08,
		0xFF, 0xFF,
		{0x05, 0x0F, 0x00, 0x0E},					// SEQ 1..4
		0x63,												// Misc
		{0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80,	// CRTC 0..18h
		0xBF, 0x1F, 0x20, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E,
		0x8F, 0x14, 0x60, 0x96, 0xB9, 0xEB,
		0xFF},
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	// ATC 0..13h
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00,
		0x03, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x20,	// GDC 0..8
		0x05, 0x0F, 0xFF}
	};
	static PARMENTRY	parmSmallHiResMono1 = {
		0x50, 0x31, 0x08,
		0xFF, 0xFF,
		{0x05, 0x0F, 0x00, 0x0E},					// SEQ 1..4
		0x63,												// Misc
		{0x2D, 0x27, 0x28, 0x90, 0x2B, 0x80,	// CRTC 0..18h
		0x17, 0x10, 0x20, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x15, 0x06,
		0x13, 0x0A, 0x60, 0x14, 0x17, 0xEB,
		0xFF},
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	// ATC 0..13h
		0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00,
		0x03, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x20,	// GDC 0..8
		0x05, 0x0F, 0xFF}
	};
	int		nErr, i, j;
	SEGOFF	lpVideo;
	BYTE		chr;
	WORD		wSimType;
	BOOL		bFullVGA;
	int		nRowCount, 	nColCount;
  	int	REF_PART = 1;
  	int	REF_TEST = 15;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto HiResMono1Test_exit;

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);

	// Load the registers
	if (bFullVGA)
	{
		SetRegs (&parmHiResMono1);
		nRowCount = 50;
		nColCount = 160;
	}
	else
	{
		SetRegs (&parmSmallHiResMono1);
		nRowCount = 3;
		nColCount = 80;
	}
	IOByteWrite (DAC_MASK, 0x03);
	SetDac (0x01, 0x20, 0x20, 0x20);
	SetDac (0x02, 0x20, 0x20, 0x20);
	SetDac (0x03, 0x2A, 0x2A, 0x2A);

	chr = 1;
	for (i = 0; i < nRowCount; i++)
		for (j = 0; j < nColCount; j++)
			DrawMonoChar (j, i, chr++, tblFont8x8, 8, nColCount);

	SimDumpMemory ("T0915.VGA");

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto HiResMono1Test_exit;
		}
	}

HiResMono1Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//


//
//		Mode13Test - Verify memory is read/writable and test mode 13.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Mode13Test (void)
{
	int		nErr, i, j, nLength;
	SEGOFF	lpVideo, lpTemp;
	BYTE		tblTest[] = {0x00, 0x55, 0xAA, 0xFF};
	BYTE		chr, attr;
	WORD		wSimType;
	int		nRow, nRowCount, nCol, nColCount;
	BOOL		bFullVGA;
   	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;
	nLength = 0xFFFF;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Mode13Test_exit;

	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use large frame
		nRow = 6;
		nRowCount = 8;
		nCol = 4;
		nColCount = 32;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		nRow = 0;
		nRowCount = 1;
		nCol = 0;
		nColCount = 20;
	}

	// Do the memory test only for physical devices
	if (!((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION)))
	{
		SetMode (0x13);
		SimSetState (TRUE, TRUE, TRUE);

		for (i = 0; i < sizeof tblTest; i++)
		{
			for (j = 0; (WORD) j < (WORD) nLength; j++)
				MemByteWrite (lpVideo + j, tblTest[i]);

			lpTemp = VerifyBytes (lpVideo, tblTest[i], nLength);
			if (lpTemp != NULL)
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpTemp), HIWORD (lpTemp), tblTest[i],
									MemByteRead (lpTemp));
				goto Mode13Test_exit;
			}
		}
	}

	SimSetCaptureMode (CAP_COMPOSITE);
	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	SetMode (0x13);
	SimDumpMemory ("T0914.VGA");
	SimSetState (TRUE, TRUE, TRUE);

	chr = attr = 0;
	for (i = nRow; i < (nRow + nRowCount); i++)
	{
		for (j = nCol; j < (nCol + nColCount); j++)
		{
			VGACharOut (chr, attr, (BYTE) j, (BYTE) i, 0);
			chr++; attr++;
		}
	}

	// Bright white overscan
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x31);
	IOByteWrite (ATC_INDEX, 0x0F);

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

Mode13Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//
//		ModeFTest - Verify memory is read/writable and test mode F.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int ModeFTest (void)
{
	int		nErr, i, j, nLength;
	SEGOFF	lpVideo, lpTemp;
	BYTE		tblTest[] = {0x00, 0x55, 0xAA, 0xFF};
	BYTE		chr, attr;
	WORD		wSimType;
	int		nRow, nRowCount, nCol, nColCount;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;
	nLength = 0xFFFF;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto ModeFTest_exit;

	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use large frame
		nRow = 10;
		nRowCount = 4;
		nCol = 8;
		nColCount = 64;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		nRow = 0;
		nRowCount = 1;
		nCol = 0;
		nColCount = 40;
	}

	// Do the memory test only for physical devices
	if (!((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION)))
	{
		SetMode (0x0F);
		SimSetState (TRUE, TRUE, TRUE);

		for (i = 0; i < sizeof tblTest; i++)
		{
			for (j = 0; (WORD) j < (WORD) nLength; j++)
				MemByteWrite (lpVideo + j, tblTest[i]);

			for (j = 0; j < 4; j++)
			{
				IOByteWrite (GDC_INDEX, 0x04);
				IOByteWrite (GDC_DATA, (BYTE) j);
				lpTemp = VerifyBytes (lpVideo, tblTest[i], nLength);
				if (lpTemp != NULL)
				{
					nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
										LOWORD (lpTemp), HIWORD (lpTemp), tblTest[i],
										MemByteRead (lpTemp));
					goto ModeFTest_exit;
				}
			}
		}
	}

	SimSetCaptureMode (CAP_COMPOSITE);
	SimSetState (TRUE, TRUE, FALSE);
	SetMode (0x0F);
	SimDumpMemory ("T0913.VGA");
	SimSetState (TRUE, TRUE, TRUE);

	chr = attr = 0;
	for (i = nRow; i < (nRow + nRowCount); i++)
	{
		for (j = nCol; j < (nCol + nColCount); j++)
		{
			PlanarCharOut (chr, attr, (BYTE) j, (BYTE) i, 0);
			chr++; attr++;
		}
	}

	// Bright white overscan
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x31);
	IOByteWrite (ATC_INDEX, 0x3F);

	WaitAttrBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

ModeFTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		Mode12Test - Verify memory is read/writable and test mode E/10/11/12.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Mode12Test (void)
{
	int		nErr, i, j, k, nLength;
	SEGOFF	lpVideo, lpTemp;
	BYTE		tblTest[] = {0x00, 0x55, 0xAA, 0xFF};
	BYTE		tblModes[] = {0x0E, 0x10, 0x11, 0x12};
	BYTE		chr, attr;
	WORD		wSimType;
	int		nRow, nRowCount, nCol, nColCount;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;
	nLength = 0xFFFF;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Mode12Test_exit;

	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use large frame
		nRow = 10;
		nRowCount = 4;
		nCol = 8;
		nColCount = 64;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		nRow = 0;
		nRowCount = 1;
		nCol = 0;
		nColCount = 40;
	}

	// Do the memory test only for physical devices
	if (!((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION)))
	{
		SetMode (0x12);
		SimSetState (TRUE, TRUE, TRUE);

		for (i = 0; i < sizeof tblTest; i++)
		{
			for (j = 0; (WORD) j < (WORD) nLength; j++)
				MemByteWrite (lpVideo + j, tblTest[i]);

			for (j = 0; j < 4; j++)
			{
				IOByteWrite (GDC_INDEX, 0x04);
				IOByteWrite (GDC_DATA, (BYTE) j);
				lpTemp = VerifyBytes (lpVideo, tblTest[i], nLength);
				if (lpTemp != NULL)
				{
					nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
										LOWORD (lpTemp), HIWORD (lpTemp), tblTest[i],
										MemByteRead (lpTemp));
					goto Mode12Test_exit;
				}
			}
		}
	}

	SimSetCaptureMode (CAP_COMPOSITE);
	for (k = 0; k < sizeof (tblModes); k++)
	{
		SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
		SetMode (tblModes[k]);
		if (k == 0) SimDumpMemory ("T0912.VGA");
		SimSetState (TRUE, TRUE, TRUE);

		chr = attr = 0;
		for (i = nRow; i < (nRow + nRowCount); i++)
		{
			for (j = nCol; j < (nCol + nColCount); j++)
			{
				PlanarCharOut (chr, attr, (BYTE) j, (BYTE) i, 0);
				chr++; attr++;
			}
		}

		// Bright white overscan
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x31);
		IOByteWrite (ATC_INDEX, 0x3F);

		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto Mode12Test_exit;
			}
		}
	}

Mode12Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		ModeDTest - Verify memory is read/writable and test mode D.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int ModeDTest (void)
{
	int		nErr, i, j, nLength;
	SEGOFF	lpVideo, lpTemp;
	BYTE		tblTest[] = {0x00, 0x55, 0xAA, 0xFF};
	BYTE		chr, attr;
	WORD		wSimType;
	int		nRow, nRowCount, nCol, nColCount;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;
	nLength = 0xFFFF;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto ModeDTest_exit;

	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use large frame
		nRow = 6;
		nRowCount = 8;
		nCol = 4;
		nColCount = 32;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		nRow = 0;
		nRowCount = 1;
		nCol = 0;
		nColCount = 40;
	}

	// Do the memory test only for physical devices
	if (!((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION)))
	{
		SetMode (0x0D);
		SimSetState (TRUE, TRUE, TRUE);

		for (i = 0; i < sizeof tblTest; i++)
		{
			for (j = 0; (WORD) j < (WORD) nLength; j++)
				MemByteWrite (lpVideo + j, tblTest[i]);

			for (j = 0; j < 4; j++)
			{
				IOByteWrite (GDC_INDEX, 0x04);
				IOByteWrite (GDC_DATA, (BYTE) j);
				lpTemp = VerifyBytes (lpVideo, tblTest[i], nLength);
				if (lpTemp != NULL)
				{
					nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
										LOWORD (lpTemp), HIWORD (lpTemp), tblTest[i],
										MemByteRead (lpTemp));
					goto ModeDTest_exit;
				}
			}
		}
	}

	SimSetCaptureMode (CAP_COMPOSITE);
	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x0D);
	SimDumpMemory ("T0911.VGA");
	SimSetState (TRUE, TRUE, TRUE);

	chr = attr = 0;
	for (i = nRow; i < (nRow + nRowCount); i++)
	{
		for (j = nCol; j < (nCol + nColCount); j++)
		{
			PlanarCharOut (chr, attr, (BYTE) j, (BYTE) i, 0);
			chr++; attr++;
		}
	}

	// Bright white overscan
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x31);
	IOByteWrite (ATC_INDEX, 0x3F);

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

ModeDTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		Mode7Test - Verify memory is read/writable and test each mode 7+, and 7*.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Mode7Test (void)
{
	int		nErr, i, j, k, nLength;
	SEGOFF	lpVideo, lpTemp;
	BYTE		tblTest[] = {0x00, 0x55, 0xAA, 0xFF};
	WORD		tblScan[] = {350, 400};
	BYTE		chr, attr;
	WORD		wSimType;
	int		nRow, nRowCount, nCol, nColCount;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xB0000000;
	nLength = 0x8000;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Mode7Test_exit;

	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use large frame
		nRow = 10;
		nRowCount = 4;
		nCol = 8;
		nColCount = 64;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		nRow = 0;
		nRowCount = 1;
		nCol = 0;
		nColCount = 40;
	}

	// Do the memory test only for physical devices
	if (!((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION)))
	{
		SetMode (0x07);
		SimSetState (TRUE, TRUE, TRUE);

		for (i = 0; i < sizeof tblTest; i++)
		{
			for (j = 0; (WORD) j < (WORD) nLength; j++)
				MemByteWrite (lpVideo + j, tblTest[i]);

			lpTemp = VerifyBytes (lpVideo, tblTest[i], nLength);
			if (lpTemp != NULL)
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpTemp), HIWORD (lpTemp), tblTest[i],
									MemByteRead (lpTemp));
				goto Mode7Test_exit;
			}
		}
	}

	SimSetCaptureMode (CAP_COMPOSITE);
	for (k = 0; k < sizeof (tblScan) / sizeof (WORD); k++)
	{
		SetScans (tblScan[k]);
		SimSetState (TRUE, TRUE, FALSE);			// Ignore DAC writes
		SetMode (0x07);
		if (k == 0) SimDumpMemory ("T0910.VGA");
		SimSetState (TRUE, TRUE, TRUE);

		chr = attr = 0;
		for (i = nRow; i < (nRow + nRowCount); i++)
		{
			for (j = nCol; j < (nCol + nColCount); j++)
			{
				TextCharOut (chr, attr, (BYTE) j, (BYTE) i, 0);
				chr++; attr++;
			}
		}

		// Bright white overscan
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x31);
		IOByteWrite (ATC_INDEX, 0x3F);

		WaitAttrBlink (BLINK_OFF);
		WaitCursorBlink (BLINK_ON);
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto Mode7Test_exit;
			}
		}
	}

Mode7Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		Mode6Test - Verify memory is read/writable and test mode 6.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Mode6Test (void)
{
	int		nErr, i, j, nLength;
	SEGOFF	lpVideo, lpTemp;
	BYTE		tblTest[] = {0x00, 0x55, 0xAA, 0xFF};
	BYTE		chr, attr;
	WORD		wSimType;
	int		nRow, nRowCount, nCol, nColCount;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xB8000000;
	nLength = 0x8000;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Mode6Test_exit;

	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use large frame
		nRow = 10;
		nRowCount = 4;
		nCol = 8;
		nColCount = 64;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		nRow = 0;
		nRowCount = 1;
		nCol = 0;
		nColCount = 40;
	}
		
	// Do the memory test only for physical devices
	if (!((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION)))
	{
		SetMode (0x06);
		SimSetState (TRUE, TRUE, TRUE);

		for (i = 0; i < sizeof tblTest; i++)
		{
			for (j = 0; (WORD) j < (WORD) nLength; j++)
				MemByteWrite (lpVideo + j, tblTest[i]);

			lpTemp = VerifyBytes (lpVideo, tblTest[i], nLength);
			if (lpTemp != NULL)
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpTemp), HIWORD (lpTemp), tblTest[i],
									MemByteRead (lpTemp));
				goto Mode6Test_exit;
			}
		}
	}

	SimSetCaptureMode (CAP_COMPOSITE);
	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	SetMode (0x06);
	SimDumpMemory ("T0909.VGA");
	SimSetState (TRUE, TRUE, TRUE);

	chr = attr = 0;
	for (i = nRow; i < (nRow + nRowCount); i++)
	{
		for (j = nCol; j < (nCol + nColCount); j++)
		{
			CGA2CharOut (chr, attr, (BYTE) j, (BYTE) i, 0);
			chr++; attr++;
		}
	}

	// Bright white overscan
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x31);
	IOByteWrite (ATC_INDEX, 0x17);

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

Mode6Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		Mode5Test - Verify memory is read/writable and test mode 5.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Mode5Test (void)
{
	int		nErr, i, j, nLength;
	SEGOFF	lpVideo, lpTemp;
	BYTE		tblTest[] = {0x00, 0x55, 0xAA, 0xFF};
	BYTE		chr, attr;
	WORD		wSimType;
	int		nRow, nRowCount, nCol, nColCount;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xB8000000;
	nLength = 0x8000;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Mode5Test_exit;

	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use large frame
		nRow = 6;
		nRowCount = 8;
		nCol = 4;
		nColCount = 32;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		nRow = 0;
		nRowCount = 1;
		nCol = 0;
		nColCount = 40;
	}

	// Do the memory test only for physical devices
	if (!((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION)))
	{
		SetMode (0x05);
		SimSetState (TRUE, TRUE, TRUE);

		for (i = 0; i < sizeof tblTest; i++)
		{
			for (j = 0; (WORD) j < (WORD) nLength; j++)
				MemByteWrite (lpVideo + j, tblTest[i]);

			lpTemp = VerifyBytes (lpVideo, tblTest[i], nLength);
			if (lpTemp != NULL)
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpTemp), HIWORD (lpTemp), tblTest[i],
									MemByteRead (lpTemp));
				goto Mode5Test_exit;
			}
		}
	}

	SimSetCaptureMode (CAP_COMPOSITE);
	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	SetMode (0x05);
	SimSetState (TRUE, TRUE, TRUE);
	SimDumpMemory ("T0908.VGA");

	chr = attr = 0;
	for (i = nRow; i < (nRow + nRowCount); i++)
	{
		for (j = nCol; j < (nCol + nColCount); j++)
		{
			CGA4CharOut (chr, attr, (BYTE) j, (BYTE) i, 0);
			chr++; attr++;
		}
	}

	// Bright white overscan
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x31);
	IOByteWrite (ATC_INDEX, 0x17);

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

Mode5Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		Mode3Test - Verify memory is read/writable and test each mode 3, 3+, and 3*.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Mode3Test (void)
{
	int		nErr, i, j, k, nLength;
	SEGOFF	lpVideo, lpTemp;
	BYTE		tblTest[] = {0x00, 0x55, 0xAA, 0xFF};
	WORD		tblScan[] = {200, 350, 400};
	BYTE		chr, attr;
	WORD		wSimType;
	int		nRow, nRowCount, nCol, nColCount;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xB8000000;
	nLength = 0x8000;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Mode3Test_exit;

	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use large frame
		nRow = 10;
		nRowCount = 4;
		nCol = 8;
		nColCount = 64;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		nRow = 0;
		nRowCount = 1;
		nCol = 0;
		nColCount = 40;
	}

	// Do the memory test only for physical devices
	if (!((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION)))
	{
		SetMode (0x03);
		SimSetState (TRUE, TRUE, TRUE);

		for (i = 0; i < sizeof tblTest; i++)
		{
			for (j = 0; (WORD) j < (WORD) nLength; j++)
				MemByteWrite (lpVideo + j, tblTest[i]);

			lpTemp = VerifyBytes (lpVideo, tblTest[i], nLength);
			if (lpTemp != NULL)
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpTemp), HIWORD (lpTemp), tblTest[i],
									MemByteRead (lpTemp));
				goto Mode3Test_exit;
			}
		}
	}

	SimSetCaptureMode (CAP_COMPOSITE);

	for (k = 0; k < sizeof (tblScan) / sizeof (WORD); k++)
	{
		SetScans (tblScan[k]);
		SimSetState (TRUE, TRUE, FALSE);
		SetMode (0x03);
		if (k == 0) SimDumpMemory ("T0907.VGA");
		SimSetState (TRUE, TRUE, TRUE);
		chr = attr = 0;
		for (i = nRow; i < (nRow + nRowCount); i++)
		{
			for (j = nCol; j < (nCol + nColCount); j++)
			{
				TextCharOut (chr, attr, (BYTE) j, (BYTE) i, 0);
				chr++; attr++;
			}
		}

		// Bright white overscan
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x31);
		IOByteWrite (ATC_INDEX, 0x3F);

		WaitAttrBlink (BLINK_OFF);
		WaitCursorBlink (BLINK_ON);
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto Mode3Test_exit;
			}
		}
	}

Mode3Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		Mode0Test - Verify memory is read/writable and test each mode 0, 0+, and 0*.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Mode0Test (void)
{
	int		nErr, i, j, k, nLength;
	SEGOFF	lpVideo, lpTemp;
	BYTE		tblTest[] = {0x00, 0x55, 0xAA, 0xFF};
	WORD		tblScan[] = {200, 350, 400};
	BYTE		chr, attr;
	WORD		wSimType;
	int		nRow, nRowCount, nCol, nColCount;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xB8000000;
	nLength = 0x8000;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Mode0Test_exit;

	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use large frame
		nRow = 6;
		nRowCount = 8;
		nCol = 4;
		nColCount = 32;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		nRow = 0;
		nRowCount = 1;
		nCol = 0;
		nColCount = 40;
	}

	// Do the memory test only for physical devices
	if (!((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION)))
	{
		SetMode (0x00);
		SimSetState (TRUE, TRUE, TRUE);

		for (i = 0; i < sizeof tblTest; i++)
		{
			for (j = 0; (WORD) j < (WORD) nLength; j++)
				MemByteWrite (lpVideo + j, tblTest[i]);

			lpTemp = VerifyBytes (lpVideo, tblTest[i], nLength);
			if (lpTemp != NULL)
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpTemp), HIWORD (lpTemp), tblTest[i],
									MemByteRead (lpTemp));
				goto Mode0Test_exit;
			}
		}
	}

	SimSetCaptureMode (CAP_COMPOSITE);

	for (k = 0; k < sizeof (tblScan) / sizeof (WORD); k++)
	{
		SetScans (tblScan[k]);
		SimSetState (TRUE, TRUE, FALSE);
		SetMode (0x00);
		if (k == 0) SimDumpMemory ("T0906.VGA");
		SimSetState (TRUE, TRUE, TRUE);

		chr = attr = 0;
		for (i = nRow; i < (nRow + nRowCount); i++)
		{
			for (j = nCol; j < (nCol + nColCount); j++)
			{
				TextCharOut (chr, attr, (BYTE) j, (BYTE) i, 0);
				chr++; attr++;
			}
		}

		// Bright white overscan
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x31);
		IOByteWrite (ATC_INDEX, 0x3F);

		WaitAttrBlink (BLINK_OFF);
		WaitCursorBlink (BLINK_ON);
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto Mode0Test_exit;
			}
		}
	}

Mode0Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		LatchTest - Write all possible data value and copy each to another
//						video memory location using the latches
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int LatchTest (void)
{
	static BYTE		byPattern[4][4] = {
						{0x5A, 0x23, 0xA1, 0xFF},
						{0x15, 0x51, 0x8A, 0xA8},
						{0x69, 0xF0, 0xCA, 0x11},
						{0x01, 0xEF, 0x90, 0x77}
				};
	int		nErr;
	SEGOFF	lpVideo;
	BYTE		temp;
	WORD		i, j, wXRes, wYRes, wSimType;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	lpVideo = (SEGOFF) 0xA0000000;

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x12);
	GetResolution (&wXRes, &wYRes);

	SimDumpMemory ("T0905.VGA");

	// Fill memory with a pattern
	for (i = 0; i < 4; i++)
	{
		IOByteWrite (SEQ_INDEX, 0x02);
		IOByteWrite (SEQ_DATA, (BYTE) (1 << i));
		for (j = 0; j < (WORD) (wXRes/8)*(wYRes); j++)
			MemByteWrite (lpVideo + j, byPattern[j % 4][i]);
	}

	IOWordWrite (SEQ_INDEX, 0x0F02);					// Enable all planes
	IOWordWrite (GDC_INDEX, 0x0105);					// Write mode one
	for (j = 0; j < (WORD) (wXRes/8)*(wYRes); j++)
	{
		MemByteRead (lpVideo + j);						// Load latches
		MemByteWrite (lpVideo + 0xFFFF, 0);			// Write latches
		for (i = 0; i < 4; i++)
		{
			IOByteWrite (GDC_INDEX, 0x04);
			IOByteWrite (GDC_DATA, (BYTE) i);		// Read map select
			temp = MemByteRead (lpVideo + 0xFFFF);
			if (temp != byPattern[j % 4][i])
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, j, HIWORD (lpVideo), byPattern[j % 4][i], temp);
				goto LatchTest_exit;
			}
		}
	}

LatchTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		CGAHerculesTest - Test compatibility address line bits in CRTC[17]
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int CGAHerculesTest (void)
{
	int		nErr, i;
	SEGOFF	lpVideo;
	DWORD		offset, dwLength;
	WORD		wSimType;
	BOOL		bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 12;

	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto CGAHerculesTest_exit;

	if (bFullVGA)
	{
		dwLength = 0x10000;
	}
	else
	{
		SimSetFrameSize (FALSE);					// Use small frame
		dwLength = 0x2000;
	}

	SimSetState (TRUE, TRUE, FALSE);			// Ignore DAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);

	SimDumpMemory ("T0904.VGA");

	// Fill memory with a known pattern
	for (offset = 0; offset < dwLength; offset += 4)
	{
		IOWordWrite (SEQ_INDEX, 0x0102);
		MemByteWrite (lpVideo++, 0xFF);
		IOWordWrite (SEQ_INDEX, 0x0202);
		MemByteWrite (lpVideo++, 0xFF);
		IOWordWrite (SEQ_INDEX, 0x0402);
		MemByteWrite (lpVideo++, 0xFF);
		IOWordWrite (SEQ_INDEX, 0x0F02);
		MemByteWrite (lpVideo++, 0xFF);
	}

	// Verify bit 5 has no effect in BYTE mode comparing this screen to the
	// next one. They should be the same.
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CGAHerculesTest_exit;
		}
	}

	IOWordWrite (CRTC_CINDEX, 0xC317);			// Clear CRTC[17].5
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CGAHerculesTest_exit;
		}
	}

	// Verify bit 5 has no effect in DWORD mode comparing this screen to the
	// next one. They should be the same.
	IOWordWrite (CRTC_CINDEX, 0x4014);
	IOWordWrite (CRTC_CINDEX, 0xE317);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CGAHerculesTest_exit;
		}
	}

	IOWordWrite (CRTC_CINDEX, 0xC317);			// Clear CRTC[17].5
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CGAHerculesTest_exit;
		}
	}

	// Set to WORD mode, "normal" addressing
	IOWordWrite (CRTC_CINDEX, 0x0014);
	IOWordWrite (CRTC_CINDEX, 0xA317);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CGAHerculesTest_exit;
		}
	}

	// Now set bit 5 again in WORD mode. MA13 replaces LA0 instead of MA15.
	IOWordWrite (CRTC_CINDEX, 0x8317);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CGAHerculesTest_exit;
		}
	}

	// Verify CGA compatible addressing. Fill memory with a block of data
	// for the first 2000h bytes.
	lpVideo = (SEGOFF) 0xA0000000;
	SimSetState (TRUE, TRUE, FALSE);
	SetMode (0x12);
	for (i = 0; i < 0x2000; i++)
		MemByteWrite (lpVideo++, 0xFF);

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CGAHerculesTest_exit;
		}
	}

	// Clear CRTC[0] and verify that the first 2000h bytes are "duplicated"
	// on the display.
	IOWordWrite (CRTC_CINDEX, 0xE217);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CGAHerculesTest_exit;
		}
	}

	// Set DWORD mode, normal addressing
	IOWordWrite (CRTC_CINDEX, 0x4014);
	IOWordWrite (CRTC_CINDEX, 0xE317);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CGAHerculesTest_exit;
		}
	}

	// Set DWORD mode, CGA addressing
	IOWordWrite (CRTC_CINDEX, 0xE217);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

CGAHerculesTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		Chain2Chain4Test - Fill memory in each of chain/2 and chain/4 modes
//									and read back specific locations
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Chain2Chain4Test (void)
{
	int		nErr, i;
	SEGOFF	lpVideo;
	WORD		count, wOffset, wSimType;
	BYTE		temp;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	lpVideo = (SEGOFF) 0xA0000000;

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);

	SimDumpMemory ("T0903.VGA");

	// Set chain/2, odd/even mode and fill some of memory
	IOWordWrite (GDC_INDEX, 0x0706);				// Chain/2
	IOWordWrite (SEQ_INDEX, 0x0204);				// Odd/even mode
	IOWordWrite (SEQ_INDEX, 0x0302);				// Enable plane 0 / plane 1
//	for (count = 0; count < 0x8000; count++)
	for (count = 0; count < 0x800; count++)
	{
		MemByteWrite (lpVideo++, '0');
		MemByteWrite (lpVideo++, '1');
	}

	// Set back to planar mode and verify memory
	lpVideo = (SEGOFF) 0xA0000000;
	IOWordWrite (GDC_INDEX, 0x0506);				// Not chain/2
	IOWordWrite (SEQ_INDEX, 0x0604);				// Not odd/even
	IOWordWrite (GDC_INDEX, 0x0004);				// Read map 0
//	for (count = 0; count < 0x8000; count++)
	for (count = 0; count < 0x800; count++)
	{
		temp = MemByteRead (lpVideo);
		if (temp != '0')
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, LOWORD (lpVideo), HIWORD (lpVideo), '0', temp);
			goto Chain2Chain4Test_exit;
		}
		lpVideo += 2;
	}
	lpVideo = (SEGOFF) 0xA0000000;
	IOWordWrite (GDC_INDEX, 0x0104);				// Read map 1
//	for (count = 0; count < 0x8000; count++)
	for (count = 0; count < 0x800; count++)
	{
		temp = MemByteRead (lpVideo);
		if (temp != '1')
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, LOWORD (lpVideo), HIWORD (lpVideo), '1', temp);
			goto Chain2Chain4Test_exit;
		}
		lpVideo += 2;
	}

	// Clear memory
	SimSetState (TRUE, TRUE, FALSE);
	SetMode (0x12);

	// Set chain/4 mode and fill memory
	lpVideo = (SEGOFF) 0xA0000000;
	IOWordWrite (SEQ_INDEX, 0x0E04);				// Chain/4 mode
	IOWordWrite (SEQ_INDEX, 0x0F02);				// Enable planes 0..3
//	for (count = 0; count < 0x4000; count++)
	for (count = 0; count < 0x400; count++)
	{
		MemByteWrite (lpVideo++, '0');
		MemByteWrite (lpVideo++, '1');
		MemByteWrite (lpVideo++, '2');
		MemByteWrite (lpVideo++, '3');
	}
	// Set back to planar mode and verify memory
	lpVideo = (SEGOFF) 0xA0000000;
	wOffset = 0;
	IOWordWrite (SEQ_INDEX, 0x0604);				// Not odd/even
	for (i = 0; i < 4; i++)
	{
		IOByteWrite (GDC_INDEX, 0x04);			// Read map
		IOByteWrite (GDC_DATA, (BYTE) i);
//		for (count = 0; count < 0x4000; count++)
		for (count = 0; count < 0x400; count++)
		{
			wOffset = count*4 + ((count & 0x3000) >> 12);
			temp = MemByteRead (lpVideo + wOffset);
			if (temp != (BYTE) ('0' + i))
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, wOffset, HIWORD (lpVideo), '0' + i, temp);
				goto Chain2Chain4Test_exit;
			}
		}
	}

Chain2Chain4Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		ByteModeTest - Load a pattern and set each of BYTE mode, WORD mode,
//							and DWORD mode.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int ByteModeTest (void)
{
	int		nErr;
	SEGOFF	lpVideo;
	DWORD		offset, dwLength;
	WORD		wSimType;
	BOOL		bFullVGA;
   	int	REF_PART = 1;
   	int	REF_TEST = 13;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto ByteModeTest_exit;

	if (bFullVGA)
	{
		dwLength = 0x10000;
	}
	else
	{
		SimSetFrameSize (FALSE);					// Use small frame
		dwLength = 0x2000;
	}

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);

	lpVideo = (SEGOFF) 0xA0000000;
	SimDumpMemory ("T0902.VGA");

	// Fill memory with an RGB pattern
	for (offset = 0; offset < dwLength; offset += 4)
	{
		IOWordWrite (SEQ_INDEX, 0x0102);
		MemByteWrite (lpVideo++, 0xFF);
		IOWordWrite (SEQ_INDEX, 0x0202);
		MemByteWrite (lpVideo++, 0xFF);
		IOWordWrite (SEQ_INDEX, 0x0402);
		MemByteWrite (lpVideo++, 0xFF);
		IOWordWrite (SEQ_INDEX, 0x0F02);
		MemByteWrite (lpVideo++, 0xFF);
	}

	// Verify BYTE mode
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto ByteModeTest_exit;
		}
	}

	// Set WORD mode and observe every other byte is skipped
	IOWordWrite (CRTC_CINDEX, 0xA317);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto ByteModeTest_exit;
		}
	}

	// Set DWORD mode and observe every fourth byte is skipped (note that
	// WORD mode is still set)
	IOWordWrite (CRTC_CINDEX, 0x4014);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto ByteModeTest_exit;
		}
	}

	// Verify that BYTE/WORD mode bit has no effect on DWORD mode bit by
	// setting back to BYTE mode without changing from DWORD mode.
	IOWordWrite (CRTC_CINDEX, 0xE317);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

ByteModeTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		IORWTest - Write a pattern to each I/O register and verify that there
//						are no errors.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
typedef struct tagIOTABLE {
	WORD	wport;
	WORD	rport;
	BOOL	fIndexed;
	BYTE	idx;
	BYTE	mask;
	BYTE	rsvdmask;
	BYTE	rsvddata;
	BOOL	bError;
	BYTE	byExp;
	BYTE	byAct;
} IOTABLE;
int IORWTest (void)
{
	static IOTABLE iot[] = {
		{DAC_MASK, DAC_MASK, FALSE, 0x00, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{DAC_WINDEX, DAC_WINDEX, FALSE, 0x00, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{SEQ_INDEX, SEQ_INDEX, FALSE, 0x00, 0x07, 0xF8, 0x00, FALSE, 0x00, 0x00},
		{SEQ_DATA, SEQ_DATA, TRUE, 0x00, 0x03, 0xFC, 0x00, FALSE, 0x00, 0x00},
		{SEQ_DATA, SEQ_DATA, TRUE, 0x01, 0x3D, 0xC2, 0x00, FALSE, 0x00, 0x00},
		{SEQ_DATA, SEQ_DATA, TRUE, 0x02, 0x0F, 0xF0, 0x00, FALSE, 0x00, 0x00},
		{SEQ_DATA, SEQ_DATA, TRUE, 0x03, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{SEQ_DATA, SEQ_DATA, TRUE, 0x04, 0x0E, 0xF1, 0x00, FALSE, 0x00, 0x00},
//		{CRTC_CINDEX, CRTC_CINDEX, FALSE, 0x00, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CINDEX, CRTC_CINDEX, FALSE, 0x00, 0xBF, 0x40, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x00, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x01, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x02, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x03, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x04, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x05, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x06, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x07, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x08, 0x7F, 0x80, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x09, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x0A, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x0B, 0x7F, 0x80, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x0C, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x0D, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x0E, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x0F, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x10, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x11, 0xCF, 0x00, 0x00, FALSE, 0x00, 0x00},		// Don't test interrupt bits here
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x12, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x13, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x14, 0x7F, 0x80, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x15, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x16, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x17, 0xEF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{CRTC_CDATA, CRTC_CDATA, TRUE, 0x18, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{GDC_INDEX, GDC_INDEX, FALSE, 0x00, 0x0F, 0xF0, 0x00, FALSE, 0x00, 0x00},
		{GDC_DATA, GDC_DATA, TRUE, 0x00, 0x0F, 0xF0, 0x00, FALSE, 0x00, 0x00},
		{GDC_DATA, GDC_DATA, TRUE, 0x01, 0x0F, 0xF0, 0x00, FALSE, 0x00, 0x00},
		{GDC_DATA, GDC_DATA, TRUE, 0x02, 0x0F, 0xF0, 0x00, FALSE, 0x00, 0x00},
		{GDC_DATA, GDC_DATA, TRUE, 0x03, 0x1F, 0xE0, 0x00, FALSE, 0x00, 0x00},
//		{GDC_DATA, GDC_DATA, TRUE, 0x04, 0x03, 0xFC, 0x00, FALSE, 0x00, 0x00},
		{GDC_DATA, GDC_DATA, TRUE, 0x04, 0xF3, 0x0C, 0x00, FALSE, 0x00, 0x00},
//		{GDC_DATA, GDC_DATA, TRUE, 0x05, 0x7B, 0x84, 0x00, FALSE, 0x00, 0x00},
		{GDC_DATA, GDC_DATA, TRUE, 0x05, 0xFB, 0x04, 0x00, FALSE, 0x00, 0x00},
		{GDC_DATA, GDC_DATA, TRUE, 0x06, 0x0F, 0xF0, 0x00, FALSE, 0x00, 0x00},
		{GDC_DATA, GDC_DATA, TRUE, 0x07, 0x0F, 0xF0, 0x00, FALSE, 0x00, 0x00},
		{GDC_DATA, GDC_DATA, TRUE, 0x08, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_INDEX, FALSE, 0x00, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x00, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x01, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x02, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x03, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x04, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x05, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x06, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x07, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x08, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x09, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x0A, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x0B, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x0C, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x0D, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x0E, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x0F, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x10, 0xEF, 0x10, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x11, 0xFF, 0x00, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x12, 0x3F, 0xC0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x13, 0x0F, 0xF0, 0x00, FALSE, 0x00, 0x00},
		{ATC_INDEX, ATC_RDATA, TRUE, 0x34, 0x0F, 0xF0, 0x00, FALSE, 0x00, 0x00},
		{MISC_OUTPUT, MISC_INPUT, FALSE, 0x00, 0xEF, 0x10, 0x00, FALSE, 0x00, 0x00},
		{FEAT_CWCONTROL, FEAT_RCONTROL, FALSE, 0x00, 0x3B, 0xC4, 0x00, FALSE, 0x00, 0x00}
	};
	static char	szErrorMsg[48];
	int			nErr, i, j, idx, nTableSize;
	WORD			wIOWrt, wIORd, wSimType;
	BOOL			bFirst, bFullVGA;
	BYTE			temp;
   	int	REF_PART = 1;
   	int	REF_TEST = 7;

	nTableSize = sizeof (iot) / sizeof (IOTABLE);
	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
  SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);
	SimSetCaptureMode (CAP_COMPOSITE);

	// Unlock the CRTC and blank the screen
	IOByteWrite (CRTC_CINDEX, 0x11);
	IOByteWrite (CRTC_CDATA, (BYTE) (IOByteRead (CRTC_CDATA) & 0x7F));
	IOByteWrite (SEQ_INDEX, 0x01);
	IOByteWrite (SEQ_DATA, (BYTE) (IOByteRead (SEQ_DATA) | 0x20));

	SimDumpMemory ("T0901.VGA");

	for (i = 0; i < nTableSize; i++)
	{
		wIOWrt = iot[i].wport;
		wIORd = iot[i].rport;

		if (wIOWrt == ATC_INDEX)
		{
			ClearIObitDataBus ();				// Set index state
			if (iot[i].fIndexed)
			{
				IOByteWrite (wIOWrt, iot[i].idx);
				ClearIObitDataBus ();			// Set back to index state
			}
		}
		else if (iot[i].fIndexed)
		{
			IOByteWrite (wIOWrt - 1, iot[i].idx);
		}

		if ((temp = IsIObitFunctional (wIORd, wIOWrt, (BYTE) ~iot[i].mask)) != 0x00)
		{
			iot[i].bError = TRUE;
			iot[i].byExp = 0x00;
			iot[i].byAct = temp;
			nErr = ERROR_IOFAILURE;
		}

		if (iot[i].rsvdmask)
		{
			if (wIOWrt == ATC_INDEX) ClearIObitDataBus ();
			if ((temp = IsIObitFunctional (wIORd, wIOWrt, (BYTE) ~(iot[i].rsvdmask | iot[i].mask))) != iot[i].rsvdmask)
			{
				iot[i].bError = TRUE;
				iot[i].byExp = iot[i].rsvdmask;
				iot[i].byAct = temp;
				nErr = ERROR_IOFAILURE;
			}
			else
			{
				temp = IOByteRead (wIORd) & iot[i].rsvdmask;
				if (temp != iot[i].rsvddata)
				{
					iot[i].bError = TRUE;
					iot[i].byExp = iot[i].rsvdmask;
					iot[i].byAct = temp;
					nErr = ERROR_IOFAILURE;
				}
			}
		}
	}

	SystemCleanUp ();

	// If there were any errors display them now.
	if (nErr != ERROR_NONE)
	{
		for (i = 0; i < nTableSize; i++)
		{
			if (iot[i].bError)
			{
				if (iot[i].fIndexed)
					printf ("%04X[%02X] failed (exp=%02Xh, act=%02Xh)\n", iot[i].wport, iot[i].idx, iot[i].byExp, iot[i].byAct);
				else
					printf ("Write Address = %04X; Read Address = %04X failed (exp=%02Xh, act=%02Xh)\n", iot[i].wport, iot[i].rport, iot[i].byExp, iot[i].byAct);

				// Display a nice "user friendly" message that lists the
				// non-standard bits for the user
				temp = iot[i].byExp ^ iot[i].byAct;
				szErrorMsg[0] = '\0';
				bFirst = TRUE;
				idx = 0;
				for (j = 0; j < 8; j++)
				{
					if ((temp & 0x01) == 0x01)
					{
						if (!bFirst)
						{
							strcat (szErrorMsg, ", ");
							idx += 2;
						}
						szErrorMsg[idx++] = j | 0x30;
						szErrorMsg[idx] = '\0';
						bFirst = FALSE;
					}
					temp = temp >> 1;
				}
				printf ("   (Non-standard bit(s): %s)\n", szErrorMsg);
			}
		}
	}

	return (nErr);
}
//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//
//		DACSparkleTest - Fill the screen with a pattern and shift the palette
//								around without waiting for vertical retrace.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int DACSparkleTest (void)
{
	static BYTE	dac[256][3] = {
		// Vary blue by +2
		{0x0, 0x0, 0x0},		// Index 0
		{0x0, 0x0, 0x2},		// Index 1
		{0x0, 0x0, 0x4},		// Index 2
		{0x0, 0x0, 0x6},		// Index 3
		{0x0, 0x0, 0x8},		// Index 4
		{0x0, 0x0, 0x0A},		// Index 5
		{0x0, 0x0, 0x0C},		// Index 6
		{0x0, 0x0, 0x0E},		// Index 7
		{0x0, 0x0, 0x10},		// Index 8
		{0x0, 0x0, 0x12},		// Index 9
		{0x0, 0x0, 0x14},		// Index 10
		{0x0, 0x0, 0x16},		// Index 11
		{0x0, 0x0, 0x18},		// Index 12
		{0x0, 0x0, 0x1A},		// Index 13
		{0x0, 0x0, 0x1C},		// Index 14
		{0x0, 0x0, 0x1E},		// Index 15
		{0x0, 0x0, 0x20},		// Index 16
		{0x0, 0x0, 0x22},		// Index 17
		{0x0, 0x0, 0x24},		// Index 18
		{0x0, 0x0, 0x26},		// Index 19
		{0x0, 0x0, 0x28},		// Index 20
		{0x0, 0x0, 0x2A},		// Index 21
		{0x0, 0x0, 0x2C},		// Index 22
		{0x0, 0x0, 0x2E},		// Index 23
		{0x0, 0x0, 0x30},		// Index 24
		{0x0, 0x0, 0x32},		// Index 25
		{0x0, 0x0, 0x34},		// Index 26
		{0x0, 0x0, 0x36},		// Index 27
		{0x0, 0x0, 0x38},		// Index 28
		{0x0, 0x0, 0x3A},		// Index 29
		{0x0, 0x0, 0x3C},		// Index 30
		{0x0, 0x0, 0x3F},		// Index 31
		// Vary green by +2
		{0x0, 0x02, 0x03F},		// Index 32
		{0x0, 0x04, 0x03F},		// Index 33
		{0x0, 0x06, 0x03F},		// Index 34
		{0x0, 0x08, 0x03F},		// Index 35
		{0x0, 0x0A, 0x03F},		// Index 36
		{0x0, 0x0C, 0x03F},		// Index 37
		{0x0, 0x0E, 0x03F},		// Index 38
		{0x0, 0x10, 0x03F},		// Index 39
		{0x0, 0x12, 0x03F},		// Index 40
		{0x0, 0x14, 0x03F},		// Index 41
		{0x0, 0x16, 0x03F},		// Index 42
		{0x0, 0x18, 0x03F},		// Index 43
		{0x0, 0x1A, 0x03F},		// Index 44
		{0x0, 0x1C, 0x03F},		// Index 45
		{0x0, 0x1E, 0x03F},		// Index 46
		{0x0, 0x20, 0x03F},		// Index 47
		{0x0, 0x22, 0x03F},		// Index 48
		{0x0, 0x24, 0x03F},		// Index 49
		{0x0, 0x26, 0x03F},		// Index 50
		{0x0, 0x28, 0x03F},		// Index 51
		{0x0, 0x2A, 0x03F},		// Index 52
		{0x0, 0x2C, 0x03F},		// Index 53
		{0x0, 0x2E, 0x03F},		// Index 54
		{0x0, 0x30, 0x03F},		// Index 55
		{0x0, 0x32, 0x03F},		// Index 56
		{0x0, 0x34, 0x03F},		// Index 57
		{0x0, 0x36, 0x03F},		// Index 58
		{0x0, 0x38, 0x03F},		// Index 59
		{0x0, 0x3A, 0x03F},		// Index 60
		{0x0, 0x3C, 0x03F},		// Index 61
		{0x0, 0x3F, 0x03F},		// Index 62
		// Vary blue by -2
		{0x0, 0x3F, 0x3C},		// Index 63
		{0x0, 0x3F, 0x3A},		// Index 64
		{0x0, 0x3F, 0x38},		// Index 65
		{0x0, 0x3F, 0x36},		// Index 66
		{0x0, 0x3F, 0x34},		// Index 67
		{0x0, 0x3F, 0x32},		// Index 68
		{0x0, 0x3F, 0x30},		// Index 69
		{0x0, 0x3F, 0x2E},		// Index 70
		{0x0, 0x3F, 0x2C},		// Index 71
		{0x0, 0x3F, 0x2A},		// Index 72
		{0x0, 0x3F, 0x28},		// Index 73
		{0x0, 0x3F, 0x26},		// Index 74
		{0x0, 0x3F, 0x24},		// Index 75
		{0x0, 0x3F, 0x22},		// Index 76
		{0x0, 0x3F, 0x20},		// Index 77
		{0x0, 0x3F, 0x1E},		// Index 78
		{0x0, 0x3F, 0x1C},		// Index 79
		{0x0, 0x3F, 0x1A},		// Index 80
		{0x0, 0x3F, 0x18},		// Index 81
		{0x0, 0x3F, 0x16},		// Index 82
		{0x0, 0x3F, 0x14},		// Index 83
		{0x0, 0x3F, 0x12},		// Index 84
		{0x0, 0x3F, 0x10},		// Index 85
		{0x0, 0x3F, 0x0E},		// Index 86
		{0x0, 0x3F, 0x0C},		// Index 87
		{0x0, 0x3F, 0x0A},		// Index 88
		{0x0, 0x3F, 0x08},		// Index 89
		{0x0, 0x3F, 0x06},		// Index 90
		{0x0, 0x3F, 0x04},		// Index 91
		{0x0, 0x3F, 0x02},		// Index 92
		{0x0, 0x3F, 0x00},		// Index 93
		// Vary red by +2
		{0x02, 0x03F, 0x00},	// Index 94
		{0x04, 0x03F, 0x00},	// Index 95
		{0x06, 0x03F, 0x00},	// Index 96
		{0x08, 0x03F, 0x00},	// Index 97
		{0x0A, 0x03F, 0x00},	// Index 98
		{0x0C, 0x03F, 0x00},	// Index 99
		{0x0E, 0x03F, 0x00},	// Index 100
		{0x10, 0x03F, 0x00},	// Index 101
		{0x12, 0x03F, 0x00},	// Index 102
		{0x14, 0x03F, 0x00},	// Index 103
		{0x16, 0x03F, 0x00},	// Index 104
		{0x18, 0x03F, 0x00},	// Index 105
		{0x1A, 0x03F, 0x00},	// Index 106
		{0x1C, 0x03F, 0x00},	// Index 107
		{0x1E, 0x03F, 0x00},	// Index 108
		{0x20, 0x03F, 0x00},	// Index 109
		{0x22, 0x03F, 0x00},	// Index 110
		{0x24, 0x03F, 0x00},	// Index 111
		{0x26, 0x03F, 0x00},	// Index 112
		{0x28, 0x03F, 0x00},	// Index 113
		{0x2A, 0x03F, 0x00},	// Index 114
		{0x2C, 0x03F, 0x00},	// Index 115
		{0x2E, 0x03F, 0x00},	// Index 116
		{0x30, 0x03F, 0x00},	// Index 117
		{0x32, 0x03F, 0x00},	// Index 118
		{0x34, 0x03F, 0x00},	// Index 119
		{0x36, 0x03F, 0x00},	// Index 120
		{0x38, 0x03F, 0x00},	// Index 121
		{0x3A, 0x03F, 0x00},	// Index 122
		{0x3C, 0x03F, 0x00},	// Index 123
		{0x3F, 0x03F, 0x00},	// Index 124
		// Vary green by -2
		{0x3F, 0x03C, 0x00},	// Index 125
		{0x3F, 0x03A, 0x00},	// Index 126
		{0x3F, 0x038, 0x00},	// Index 127
		{0x3F, 0x036, 0x00},	// Index 128
		{0x3F, 0x034, 0x00},	// Index 129
		{0x3F, 0x032, 0x00},	// Index 130
		{0x3F, 0x030, 0x00},	// Index 131
		{0x3F, 0x02E, 0x00},	// Index 132
		{0x3F, 0x02C, 0x00},	// Index 133
		{0x3F, 0x02A, 0x00},	// Index 134
		{0x3F, 0x028, 0x00},	// Index 135
		{0x3F, 0x026, 0x00},	// Index 136
		{0x3F, 0x024, 0x00},	// Index 137
		{0x3F, 0x022, 0x00},	// Index 138
		{0x3F, 0x020, 0x00},	// Index 139
		{0x3F, 0x01E, 0x00},	// Index 140
		{0x3F, 0x01C, 0x00},	// Index 141
		{0x3F, 0x01A, 0x00},	// Index 142
		{0x3F, 0x018, 0x00},	// Index 143
		{0x3F, 0x016, 0x00},	// Index 144
		{0x3F, 0x014, 0x00},	// Index 145
		{0x3F, 0x012, 0x00},	// Index 146
		{0x3F, 0x010, 0x00},	// Index 147
		{0x3F, 0x00E, 0x00},	// Index 148
		{0x3F, 0x00C, 0x00},	// Index 149
		{0x3F, 0x00A, 0x00},	// Index 150
		{0x3F, 0x008, 0x00},	// Index 151
		{0x3F, 0x006, 0x00},	// Index 152
		{0x3F, 0x004, 0x00},	// Index 153
		{0x3F, 0x002, 0x00},	// Index 154
		{0x3F, 0x000, 0x00},	// Index 155
		// Vary blue by +2
		{0x3F, 0x000, 0x02},	// Index 156
		{0x3F, 0x000, 0x04},	// Index 157
		{0x3F, 0x000, 0x06},	// Index 158
		{0x3F, 0x000, 0x08},	// Index 159
		{0x3F, 0x000, 0x0A},	// Index 160
		{0x3F, 0x000, 0x0C},	// Index 161
		{0x3F, 0x000, 0x0E},	// Index 162
		{0x3F, 0x000, 0x10},	// Index 163
		{0x3F, 0x000, 0x12},	// Index 164
		{0x3F, 0x000, 0x14},	// Index 165
		{0x3F, 0x000, 0x16},	// Index 166
		{0x3F, 0x000, 0x18},	// Index 167
		{0x3F, 0x000, 0x1A},	// Index 168
		{0x3F, 0x000, 0x1C},	// Index 169
		{0x3F, 0x000, 0x1E},	// Index 170
		{0x3F, 0x000, 0x20},	// Index 171
		{0x3F, 0x000, 0x22},	// Index 172
		{0x3F, 0x000, 0x24},	// Index 173
		{0x3F, 0x000, 0x26},	// Index 174
		{0x3F, 0x000, 0x28},	// Index 175
		{0x3F, 0x000, 0x2A},	// Index 176
		{0x3F, 0x000, 0x2C},	// Index 177
		{0x3F, 0x000, 0x2E},	// Index 178
		{0x3F, 0x000, 0x30},	// Index 179
		{0x3F, 0x000, 0x32},	// Index 180
		{0x3F, 0x000, 0x34},	// Index 181
		{0x3F, 0x000, 0x36},	// Index 182
		{0x3F, 0x000, 0x38},	// Index 183
		{0x3F, 0x000, 0x3A},	// Index 184
		{0x3F, 0x000, 0x3C},	// Index 185
		{0x3F, 0x000, 0x3F},	// Index 186
		// Vary green by +2
		{0x3F, 0x002, 0x3F},	// Index 187
		{0x3F, 0x004, 0x3F},	// Index 188
		{0x3F, 0x006, 0x3F},	// Index 189
		{0x3F, 0x008, 0x3F},	// Index 190
		{0x3F, 0x00A, 0x3F},	// Index 191
		{0x3F, 0x00C, 0x3F},	// Index 192
		{0x3F, 0x00E, 0x3F},	// Index 193
		{0x3F, 0x010, 0x3F},	// Index 194
		{0x3F, 0x012, 0x3F},	// Index 195
		{0x3F, 0x014, 0x3F},	// Index 196
		{0x3F, 0x016, 0x3F},	// Index 197
		{0x3F, 0x018, 0x3F},	// Index 198
		{0x3F, 0x01A, 0x3F},	// Index 199
		{0x3F, 0x01C, 0x3F},	// Index 200
		{0x3F, 0x01E, 0x3F},	// Index 201
		{0x3F, 0x020, 0x3F},	// Index 202
		{0x3F, 0x022, 0x3F},	// Index 203
		{0x3F, 0x024, 0x3F},	// Index 204
		{0x3F, 0x026, 0x3F},	// Index 205
		{0x3F, 0x028, 0x3F},	// Index 206
		{0x3F, 0x02A, 0x3F},	// Index 207
		{0x3F, 0x02C, 0x3F},	// Index 208
		{0x3F, 0x02E, 0x3F},	// Index 209
		{0x3F, 0x030, 0x3F},	// Index 210
		{0x3F, 0x032, 0x3F},	// Index 211
		{0x3F, 0x034, 0x3F},	// Index 212
		{0x3F, 0x036, 0x3F},	// Index 213
		{0x3F, 0x038, 0x3F},	// Index 214
		{0x3F, 0x03A, 0x3F},	// Index 215
		{0x3F, 0x03C, 0x3F},	// Index 216
		{0x3F, 0x03E, 0x3F},	// Index 217
		// Vary (approximately) red by -2, green by -2, blue by -2
		{0x3F, 0x03F, 0x3F},	// Index 218
		{0x3E, 0x03E, 0x3E},	// Index 219
		{0x3D, 0x03D, 0x3D},	// Index 220
		{0x3C, 0x03C, 0x3C},	// Index 221
		{0x3B, 0x03B, 0x3B},	// Index 222
		{0x3A, 0x03A, 0x3A},	// Index 223
		{0x39, 0x039, 0x39},	// Index 224
		{0x38, 0x038, 0x38},	// Index 225
		{0x37, 0x037, 0x37},	// Index 226
		{0x36, 0x036, 0x36},	// Index 227
		{0x34, 0x034, 0x34},	// Index 228
		{0x32, 0x032, 0x32},	// Index 229
		{0x30, 0x030, 0x30},	// Index 230
		{0x2E, 0x02E, 0x2E},	// Index 231
		{0x2C, 0x02C, 0x2C},	// Index 232
		{0x2A, 0x02A, 0x2A},	// Index 233
		{0x28, 0x028, 0x28},	// Index 234
		{0x26, 0x026, 0x26},	// Index 235
		{0x24, 0x024, 0x24},	// Index 236
		{0x22, 0x022, 0x22},	// Index 237
		{0x20, 0x020, 0x20},	// Index 238
		{0x1E, 0x01E, 0x1E},	// Index 239
		{0x1C, 0x01C, 0x1C},	// Index 240
		{0x1A, 0x01A, 0x1A},	// Index 241
		{0x18, 0x018, 0x18},	// Index 241
		{0x16, 0x016, 0x16},	// Index 243
		{0x14, 0x014, 0x14},	// Index 244
		{0x12, 0x012, 0x12},	// Index 245
		{0x10, 0x010, 0x10},	// Index 246
		{0x0E, 0x00E, 0x0E},	// Index 247
		{0x0C, 0x00C, 0x0C},	// Index 248
		{0x0A, 0x00A, 0x0A},	// Index 249
		{0x08, 0x008, 0x08},	// Index 250
		{0x06, 0x006, 0x06},	// Index 251
		{0x04, 0x004, 0x04},	// Index 252
		{0x02, 0x002, 0x02},	// Index 253
		{0x01, 0x001, 0x01},	// Index 254
		{0x00, 0x000, 0x00}	// Index 255
	};
	static BYTE	dacShadow[256][3];
	int			nErr, i, j;
	SEGOFF		lpVideo;
	BYTE			red, green, blue, ridx, widx;
	BOOL			bCapture;
	WORD			wXRes, wYRes;
	WORD			wSimType;
	BOOL			bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto DACSparkleTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);		// Use small frame
  	SimSetState (TRUE, TRUE, TRUE);
    SetMode (0x13);
	GetResolution (&wXRes, &wYRes);
 	SimSetState (TRUE, TRUE, TRUE);
 //	SimSetCaptureMode (CAP_COMPOSITE);
								 
	lpVideo = (SEGOFF) 0xA0000000;

//	SimDumpMemory ("T0804.VGA");

	for (i = 0; i < (int) wYRes; i++)
	{
		for (j = 0; j < (int) wXRes; j++)
			MemByteWrite (lpVideo++, (BYTE) i);
	}

	// Load DAC and initialize DAC shadow
	SetDacBlock ((LPBYTE) dac, 0, 256);
	for (i = 0; i < 256; i++)
	{
		dacShadow[i][0] = dac[i][0];
		dacShadow[i][1] = dac[i][1];
		dacShadow[i][2] = dac[i][2];
	}
   
	bCapture = FrameCapture (8, 4);

	
	ridx = 2;
	widx = 1;
	if (bCapture)
	{
		for (i = 0; i < 64; i++)
		{
			for (j = 0; j < 256; j++)
			{
				
				// Get DAC values at this index
				GetDac (ridx, &red, &green, &blue);
				if (dacShadow[ridx][0] != red)
				{
					printf("iofailure\n");
					nErr = FlagError (ERROR_IOFAILURE, 8,4,
											ridx, 0, dacShadow[ridx][0], red);
					goto DACSparkleTest_exit;
				}
				if (dacShadow[ridx][1] != green)
				{
							printf("iofailure\n");
					nErr = FlagError (ERROR_IOFAILURE, 8,4,
											ridx, 1, dacShadow[ridx][1], green);
					goto DACSparkleTest_exit;
				}
				if (dacShadow[ridx][2] != blue)
				{
							printf("iofailure\n");
					nErr = FlagError (ERROR_IOFAILURE,8, 4,
											ridx, 2, dacShadow[ridx][2], blue);
					goto DACSparkleTest_exit;
				}

				// Set next DAC location to new values and update DAC shadow
				SetDac (widx, red, green, blue);
				dacShadow[widx][0] = red;
				dacShadow[widx][1] = green;
				dacShadow[widx][2] = blue;

				// Next location(s)
				if (++ridx == 0) ridx++;
				if (++widx == 0) widx++;
			}
		   FrameCapture (8, 4);

		}
	}
	else
	{
	  // stub out we cant wait during diags ... just do once
	  	while (1)
	  	{
			GetDac (ridx, &red, &green, &blue);
			SetDac (widx, red, green, blue);
			if (++ridx == 0) ridx++;
			if (++widx == 0) widx++;
	  	}
	}

	if (GetKey () == KEY_ESCAPE)
		nErr = FlagError (ERROR_USERABORT, 8, 4, 0, 0, 0, 0);


DACSparkleTest_exit:
	SetMode (3);
	
	SystemCleanUp ();
	return (nErr);
}
//

//
//		DACAutoIncrementTest - Three I/O writes should increment to the next index.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int DACAutoIncrementTest (void)
{
	int	nErr;
	BYTE	temp, red, green, blue;
	WORD	wSimType;
	BOOL	bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	
	SimSetState (TRUE, TRUE, TRUE);
	if (!bFullVGA) SimSetFrameSize (FALSE);		// Use small frame
	SetMode (0x03);
	SimSetCaptureMode (CAP_COMPOSITE);

	// Write index and read it back
	_disable ();
	IOByteWrite (DAC_WINDEX, 0x08);
	temp = IOByteRead (DAC_WINDEX);
	_enable ();
	if (temp != 8)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_WINDEX, 0, 8, temp);
		goto DACAutoIncrementTest_exit;
	}

	SimDumpMemory ("T0803.VGA");

	// Write data and read back the index again
	_disable ();
	IOByteWrite (DAC_DATA, 0x15);
	IOByteWrite (DAC_DATA, 0x2A);
	IOByteWrite (DAC_DATA, 0x3F);
	temp = IOByteRead (DAC_WINDEX);
	_enable ();
	if (temp != 9)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_WINDEX, 0, 9, temp);
		goto DACAutoIncrementTest_exit;
	}

	// Write "read index" and read it back (should be one past written value)
	_disable ();
	IOByteWrite (DAC_RINDEX, 0x08);
	temp = IOByteRead (DAC_WINDEX);
	_enable ();
	if (temp != 9)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_WINDEX, 0, 9, temp);
		goto DACAutoIncrementTest_exit;
	}

	// Read back and verify the previously written data and index
	_disable ();
	red = IOByteRead (DAC_DATA);
	green = IOByteRead (DAC_DATA);
	blue = IOByteRead (DAC_DATA);
	temp = IOByteRead (DAC_WINDEX);
	_enable ();
	if (temp != 0x0A)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_WINDEX, 0, 0x0A, temp);
		goto DACAutoIncrementTest_exit;
	}
	if (red != 0x15)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_DATA, 0, 0x15, red);
		goto DACAutoIncrementTest_exit;
	}
	if (green != 0x2A)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_DATA, 1, 0x2A, green);
		goto DACAutoIncrementTest_exit;
	}
	if (blue != 0x3F)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_DATA, 2, 0x3F, blue);
		goto DACAutoIncrementTest_exit;
	}

DACAutoIncrementTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		DACReadWriteStatusTest - Write to the DAC write index and the DAC read
//											index and read back the status.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int DACReadWriteStatusTest (void)
{
	int	nErr;
	BYTE	temp;
	WORD	wSimType;
	BOOL	bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);
	SimSetCaptureMode (CAP_COMPOSITE);

	// Verify write status returns a "0"
	_disable ();
	IOByteWrite (DAC_WINDEX, 0x01);
	temp = IOByteRead (DAC_RINDEX);
	_enable ();
	if (temp != 0)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_RINDEX, 0, 0, temp);
		goto DACReadWriteStatusTest_exit;
	}

	SimDumpMemory ("T0802.VGA");
	SimSetState (TRUE, TRUE, TRUE);

	// Verify read status returns a "3"
	_disable ();
	IOByteWrite (DAC_RINDEX, 0x01);
	temp = IOByteRead (DAC_RINDEX);
	_enable ();
	if (temp != 3)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_RINDEX, 0, 3, temp);
		goto DACReadWriteStatusTest_exit;
	}

	// Verify status is unchanged by write to mask
	_disable ();
	IOByteWrite (DAC_MASK, 0xFF);
	temp = IOByteRead (DAC_RINDEX);
	if (temp != 3)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_RINDEX, 0, 3, temp);
		goto DACReadWriteStatusTest_exit;
	}
	_enable ();

	// Verify status is unchanged by write to data
	_disable ();
	IOByteWrite (DAC_DATA, 0x3F);
	temp = IOByteRead (DAC_RINDEX);
	if (temp != 3)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, DAC_RINDEX, 0, 3, temp);
		goto DACReadWriteStatusTest_exit;
	}
	_enable ();

DACReadWriteStatusTest_exit:

	SystemCleanUp ();
	return (nErr);
}
//

//
//		DACMaskTest - Step through various values of the DAC mask, verifying
//							each step through visual inspection.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int DACMaskTest (void)
{
	int		nErr, i;
	SEGOFF	lpVideo;
	WORD		wXRes, wYRes;
	WORD		wSimType;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto DACMaskTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);			// Use small frame
	SimSetState (TRUE, TRUE, TRUE);
	SetMode (0x12);
	GetResolution (&wXRes, &wYRes);
	SimSetCaptureMode (CAP_COMPOSITE);
	lpVideo = (SEGOFF) 0xA0000000;

	SimDumpMemory ("T0801.VGA");

	// Load the DAC with a known set
	FillDAC (0, 0, 0);						// Clear the DAC
	SetDac (1, 0, 0, 0x3F);					// 1 = Blue
	SetDac (2, 0, 0x3F, 0);					// 2 = Green
	SetDac (4, 0, 0x3F, 0x3F);				// 4 = Cyan
	SetDac (8, 0x3F, 0, 0);					// 8 = Red
	SetDac (0x10, 0x3F, 0, 0x3F);			// 10h = Magenta
	SetDac (0x20, 0x3F, 0x3F, 0);			// 20h = Yellow
	SetDac (0x40, 0x3F, 0x2A, 0);			// 40h = Orange
	SetDac (0x80, 0x2A, 0x2A, 0x2A);		// 80h = Gray
	SetDac (0xFF, 0x3F, 0x3F, 0x3F);		// FFh = Bright White

	// Fill memory with 0FFh
	for (i = 0; (WORD) i < (WORD) (wXRes/8)*wYRes; i++)
		MemByteWrite (lpVideo++, 0xFF);

	// Set attribute controller to send 0FFh to the RAMDAC
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x0F);
	IOByteWrite (ATC_INDEX, 0x3F);
	IOByteWrite (ATC_INDEX, 0x34);
	IOByteWrite (ATC_INDEX, 0x0C);

	for (i = 0; i < 8; i++)
	{
		IOByteWrite (DAC_MASK, (BYTE) (1 << i));
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto DACMaskTest_exit;
			}
		}
	}

	// Verify last location
	IOByteWrite (DAC_MASK, 0xFF);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

DACMaskTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

 //
//		VSyncOrVDispTest - Visually inspect the sync pulse widened from vertical
//									display end to display start.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int VSyncOrVDispTest (void)
{
	int	nErr;
	WORD	wSimType;
	BOOL	bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto VSyncOrVDispTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Small frame
	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);
	SimSetCaptureMode (CAP_COMPOSITE);

	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x31);
	IOByteWrite (ATC_INDEX, 0x01);			// Blue overscan

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto VSyncOrVDispTest_exit;
		}
	}

	IOByteWrite (FEAT_CWCONTROL, 0x08);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}
	IOByteWrite (FEAT_CWCONTROL, 0x00);

VSyncOrVDispTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		SyncStatusTest - Verify that the sync signal changes and that the
//								rate is accurate.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int SyncStatusTest (void)
{
	int			nErr;
	DWORD			time0, time1;
	BOOL			bChanged;
	BYTE			temp;
	int			nLines, nFrames;
	WORD			wSimType;

 	int	REF_PART = 99;
  	int	REF_TEST = 99;
	nErr = ERROR_NONE;

	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}

	SetMode (0x03);
	TextStringOut ("The following test is self-running and will take about 5 seconds.", ATTR_NORMAL, 1, 1, 0);
	TextStringOut ("Press any key to continue, <ESC> to exit.", ATTR_NORMAL, 1, 3, 0);
	if (SimGetKey () == KEY_ESCAPE)
		goto SyncStatusTest_exit;
	SetMode (0x12);

	// Wait for not vertical retrace
	time0 = GetSystemTicks ();
	time1 = time0 + FIVE_SECONDS;
	bChanged = FALSE;
	while (!bChanged && (time0 < time1))
	{
		if ((IOByteRead (INPUT_CSTATUS_1) & 0x08) == 0x00) bChanged = TRUE;
		time0 = GetSystemTicks ();
	}

	// Did we time out?
	if (!bChanged)
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
				LOWORD (60000l), HIWORD (60000l), 0, 0);
		goto SyncStatusTest_exit;
	}

	// Wait for vertical retrace
	time0 = GetSystemTicks ();
	time1 = time0 + FIVE_SECONDS;
	bChanged = FALSE;
	while (!bChanged && (time0 < time1))
	{
		if (((temp = IOByteRead (INPUT_CSTATUS_1)) & 0x08) == 0x08) bChanged = TRUE;
		time0 = GetSystemTicks ();
	}

	// Did we time out?
	if (!bChanged)
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
				LOWORD (60000l), HIWORD (60000l), 0, 0);
		goto SyncStatusTest_exit;
	}
	// Is display disabled (it should be)?
	if ((temp & 1) == 0)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, INPUT_CSTATUS_1, 0, 1, 0);
		goto SyncStatusTest_exit;
	}

	// Wait for display enable
	time0 = GetSystemTicks ();
	time1 = time0 + FIVE_SECONDS;
	bChanged = FALSE;
	while (!bChanged && (time0 < time1))
	{
		if (((temp = IOByteRead (INPUT_CSTATUS_1)) & 0x01) == 0x00) bChanged = TRUE;
		time0 = GetSystemTicks ();
	}

	// Did we time out?
	if (!bChanged)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, INPUT_CSTATUS_1, 0, 0, 1);
		goto SyncStatusTest_exit;
	}

	// If everything is working properly, only one (or at most two) scan lines
	//	have gone by. Now count the number of display enables until vertical
	//	retrace. This number should be about 481.
	_disable ();
	nLines = 1;
	while ((temp & 0x08) == 0)
	{
		// Wait while the display is active
		while ((temp & 0x01) == 0)
		{
#ifdef __MSVC16__
			// Does not work when linked to simulation library
			_asm {
				mov	dx,INPUT_CSTATUS_1
				in		al,dx
				mov	[temp],al
			}
#else
			temp = IOByteRead (INPUT_CSTATUS_1);
#endif
		}
		// Wait while the display is inactive
		while (((temp & 0x01) == 0x01) && ((temp & 0x08) == 0x00))
		{
#ifdef __MSVC16__
			// Does not work when linked to simulation library
			_asm {
				mov	dx,INPUT_CSTATUS_1
				in		al,dx
				mov	[temp],al
			}
#else
			temp = IOByteRead (INPUT_CSTATUS_1);
#endif
		}
		nLines++;
	}
	_enable ();

	if ((nLines < 479) || (nLines > 482))
	{
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		goto SyncStatusTest_exit;
	}

	// Now time vertical sync. There should be approximately 60 of them
	// per second (or 300 vertical syncs during a five second period).
	// Wait for vertical sync before starting.
	nFrames = 0;
	WaitVerticalRetrace ();
	time0 = GetSystemTicks ();
	time1 = time0 + FIVE_SECONDS;
	while (time0 < time1)
	{
		WaitVerticalRetrace ();
		nFrames++;
		time0 = GetSystemTicks ();
	}

	if ((nFrames < 290) || (nFrames > 310))
	{
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		goto SyncStatusTest_exit;
	}

SyncStatusTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		SwitchReadbackTest - Attempt to do an EGA-style switch readback. On a
//									VGA this yields 0Fh. Then attempt to do a monitor
//									detect using the switch sense bit.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int SwitchReadbackTest (void)
{
	int	nErr, i;
	BYTE	misc, switches, temp, orgmisc;
	WORD	trigger, tmp, level, wSimType;
	BYTE	tblDAC[] = {0x00, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20,
							0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C, 0x3F};
	int	REF_PART = 1;
	int	REF_TEST = 41;

	nErr = ERROR_NONE;

	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\n\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}

	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);
	SimDumpMemory ("T0706.VGA");

	// Attempt to read EGA-style switches (meaningless on the VGA)
	switches = 0;
	orgmisc = IOByteRead (MISC_INPUT);
	misc = orgmisc & 0xF3;
	for (i = 0; i < 4; i++)
	{
		IOByteWrite (MISC_OUTPUT, (BYTE) (misc | (i << 2)));
		temp = IOByteRead (INPUT_STATUS_0);
		switches = switches | ((temp & 0x10) >> 4) << (3 - i);
	}
	IOByteWrite (MISC_OUTPUT, orgmisc);

	// Now read it as the monitor sense (analog comparator)
	trigger = 0;
	WaitNotVerticalRetrace ();
	for (i = 0; i < sizeof (tblDAC); i++)
	{
		FillDAC (tblDAC[i], tblDAC[i], tblDAC[i]);
		_disable ();
		while ((IOByteRead (INPUT_CSTATUS_1) & 0x01) == 0x00);
		while ((IOByteRead (INPUT_CSTATUS_1) & 0x01) == 0x01);
		temp = IOByteRead (INPUT_STATUS_0);
		_enable ();
		trigger |= (BYTE) (((temp & 0x10) >> 4) << i);
	}

	// Find first trigger
	tmp = trigger;
	i = 0;
	while ((tmp & 0x0001) != 0)
	{
		tmp = tmp >> 1;
		i++;
	}
	level = (WORD) ((((long) tblDAC[i]) * 100l) / 64);

	// If the level never triggered or triggered on a "0" value, then the
	// comparator isn't working properly.
	if ((i == 0) || (i >= sizeof (tblDAC)))
	{
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

	SystemCleanUp ();
   //	printf ("\nEGA-style switches = %02Xh"
	 //			"\nVGA trigger = %04Xh; DAC = (%02Xh, %02Xh, %02Xh); Level = %d%%",
	   //			switches, trigger, tblDAC[i], tblDAC[i], tblDAC[i], level);
	return (nErr);
}
//
//
//		CRTCAddressTest - Set the CRTC I/O address to each location and verify
//								that it is addressable.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int CRTCAddressTest (void)
{
	static WORD	wtblCRTC[] = {CRTC_MINDEX, CRTC_CINDEX};
	int			nErr, i;
	BYTE			temp;
	WORD			wCRTC, wSimType;
	BOOL			bFullVGA;
int	REF_PART = 1;
int	REF_TEST = 6;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x03);
	SimSetState (TRUE, TRUE, FALSE);
	SimDumpMemory ("T0321.VGA");

	for (i = 0; i < 2; i++)
	{
		temp = (BYTE) IOByteRead (MISC_INPUT);
		temp = (BYTE) ((temp & 0xFE) | i);
		IOByteWrite (MISC_OUTPUT, temp);
		wCRTC = wtblCRTC[i];
		if ((temp = IsIObitFunctional (wCRTC, wCRTC, 0xC0)) != 0)
		{
		//	nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, wCRTC, 0, 0, temp);
			goto CRTCAddressTest_exit;
		}
	}

CRTCAddressTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		RAMEnableTest - Test a byte of memory, disable memory and test it again.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int RAMEnableTest (void)
{
	int		nErr;
	SEGOFF	lpVideo;
	WORD		wSimType;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);
	lpVideo = (SEGOFF) 0xA0000000;
	SimDumpMemory ("T0704.VGA");

	if (!RWMemoryTest (HIWORD (lpVideo), LOWORD (lpVideo)))
	{
		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								LOWORD (lpVideo), HIWORD (lpVideo), 0, 0xFF);
		goto RAMEnableTest_exit;
	}

	IOByteWrite (MISC_OUTPUT, (BYTE) (IOByteRead (MISC_INPUT) & 0xFD));	// Disable memory
	if (RWMemoryTest (HIWORD (lpVideo), LOWORD (lpVideo)))
		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								LOWORD (lpVideo), HIWORD (lpVideo), 0xFF, 0);

RAMEnableTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		ClockSelectsTest - Set the standard clocks and measure the results.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int ClockSelectsTest (void)
{
	int		nErr;
	BYTE		temp;
	DWORD		hz, khz, mhz, expected, lower, upper;
	WORD		wSimType;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;

	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}

	SetMode (0x03);
	TextStringOut ("The following test is self-running and will take about 10 seconds.", ATTR_NORMAL, 1, 1, 0);
	TextStringOut ("Press any key to continue, <ESC> to exit.", ATTR_NORMAL, 1, 3, 0);
	if (SimGetKey () == KEY_ESCAPE)
		goto ClockSelectsTest_exit;

	IOWordWrite (SEQ_INDEX, 0x0100);		// Sync reset
	temp = IsIObitFunctional (MISC_INPUT, MISC_OUTPUT, 0xF3);
	IOWordWrite (SEQ_INDEX, 0x0300);		// End sync reset
	if (temp != 0)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST,
					MISC_OUTPUT, 0, 0, temp);
		goto ClockSelectsTest_exit;
	}

	SetMode (0x12);					// Set 25.175 MHz clock
	hz = GetFrameRate ();
	khz = (hz * 525) / 1000;		// Vertical total is 525 lines
	mhz = (khz * 800) / 1000;		// Horizontal total is 800 pixels

	// Calculate range of "okay" values (within 5%)
	expected = 25175;
	lower = expected - expected/20;
	upper = expected + expected/20;
	if ((mhz < lower) || (mhz > upper))
	{
		nErr = FlagError (ERROR_INVALIDCLOCK, REF_PART, REF_TEST,
					LOWORD (expected), HIWORD (expected), LOWORD (mhz), HIWORD (mhz));
		goto ClockSelectsTest_exit;
	}

	SetMode (0x03);					// Set 28.321 MHz clock
	hz = GetFrameRate ();
	khz = (hz * 449) / 1000;		// Vertical total is 449 lines
	mhz = (khz * 900) / 1000;		// Horizontal total is 900 pixels

	// Calculate range of "okay" values (within 5%)
	expected = 28321;
	lower = expected - expected/20;
	upper = expected + expected/20;
	if ((mhz < lower) || (mhz > upper))
	{
		nErr = FlagError (ERROR_INVALIDCLOCK, REF_PART, REF_TEST,
					LOWORD (expected), HIWORD (expected), LOWORD (mhz), HIWORD (mhz));
		goto ClockSelectsTest_exit;
	}

ClockSelectsTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		PageSelectTest - Place values into memory in different page bit
//								configurations and verify that those values ended
//								up where they were supposed to go.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int PageSelectTest (void)
{
	int		nErr, i;
	SEGOFF	lpVideo;
	BYTE		byExpected, byActual;
	WORD		wSimType;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto PageSelectTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);		// Small frame
	SimSetState (TRUE, TRUE, FALSE);					// Ignore DAC writes
	SetMode (0x03);
	lpVideo = (SEGOFF) 0xB8000000;
	SimDumpMemory ("T0702.VGA");

	// Clear page select bit, the inverse of which replaces LA0 in chain/2 mode
	IOByteWrite (MISC_OUTPUT, (BYTE) (IOByteRead (MISC_INPUT) & 0xDF));
	MemByteWrite (lpVideo++, '0');
	MemByteWrite (lpVideo++, '1');
	MemByteWrite (lpVideo++, '2');
	MemByteWrite (lpVideo, '3');
	IOByteWrite (MISC_OUTPUT, (BYTE) (IOByteRead (MISC_INPUT) | 0x20));

	// Force into "linear non-chained" mode
	IOWordWrite (GDC_INDEX, 0x0005);
	IOWordWrite (GDC_INDEX, 0x0506);
	IOWordWrite (SEQ_INDEX, 0x0604);

	// Verify memory is as it should be
	lpVideo = (SEGOFF) 0xA0000001;
	byExpected = '0';
	if (!MemByteTest (lpVideo, byExpected, &byActual))
	{
		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								LOWORD (lpVideo), HIWORD (lpVideo), byExpected, byActual);
		goto PageSelectTest_exit;
	}
	byExpected = '2';
	if (!MemByteTest (lpVideo + 2, byExpected, &byActual))
	{
		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								LOWORD (lpVideo + 2), HIWORD (lpVideo), byExpected, byActual);
		goto PageSelectTest_exit;
	}
	IOWordWrite (GDC_INDEX, 0x0104);
	byExpected = '1';
	if (!MemByteTest (lpVideo, byExpected, &byActual))
	{
		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								LOWORD (lpVideo), HIWORD (lpVideo), byExpected, byActual);
		goto PageSelectTest_exit;
	}
	byExpected = '3';
	if (!MemByteTest (lpVideo + 2, byExpected, &byActual))
	{
		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								LOWORD (lpVideo + 2), HIWORD (lpVideo), byExpected, byActual);
		goto PageSelectTest_exit;
	}

	// Verify page select can be inverted via SEQ[4].1
	// (This should make a blinking purple screen)
	SetMode (0x03);
	lpVideo = (SEGOFF) 0xB8000000;
	IOByteWrite (MISC_OUTPUT, (BYTE) (IOByteRead (MISC_INPUT) & 0xDF));
	IOWordWrite (SEQ_INDEX, 0x0004);

	for (i = 0; i < 0x1000; i++)
		MemByteWrite (lpVideo + i, 0xDB);

	WaitAttrBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

PageSelectTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		SyncPolarityTest - Set each of the four possible sync polarity settings.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int SyncPolarityTest (void)
{
	int	nErr, i;
	BYTE	misc;
	WORD	wXRes, wYRes;
	WORD	wSimType;
	BOOL	bFullVGA;
  	int	REF_PART = 1;
  	int	REF_TEST = 23;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto SyncPolarityTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);
	SimSetCaptureMode (CAP_COMPOSITE);
	GetResolution (&wXRes, &wYRes);

	SetLine4Columns (wXRes / 8);
	Line4 (0, 0, wXRes - 1, 0, 0x0F);
	Line4 (wXRes - 1, 0, wXRes - 1, wYRes - 1, 0x0F);
	Line4 (0, wYRes - 1, wXRes - 1, wYRes - 1, 0x0F);
	Line4 (0, 0, 0, wYRes - 1, 0x0F);
	Line4 (0, 0, wXRes - 1, wYRes - 1, 0x0F);
	Line4 (0, wYRes - 1, wXRes - 1, 0, 0x0F);

	SimDumpMemory ("T0701.VGA");

	misc = IOByteRead (MISC_INPUT);
	for (i = 0; i < 4; i++)
	{
		IOByteWrite (MISC_OUTPUT, (BYTE) ((misc & 0x3F) | i << 6));

		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto SyncPolarityTest_exit;
			}
		}
	}

SyncPolarityTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		GraphicsModeBlinkTest - Verify that the blink enable bit works in
//										graphics mode.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int GraphicsModeBlinkTest (void)
{
	int		nErr, i, j;
	SEGOFF	lpVideo;
	WORD		wXRes, wYRes;
	WORD	wSimType;
	BOOL	bFullVGA;
 	int	REF_PART = 1;
 	int	REF_TEST = 20;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto GraphicsModeBlinkTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	GetResolution (&wXRes, &wYRes);
	SimSetState (TRUE, TRUE, TRUE);

	// Draw large stripes into memory
	lpVideo = (SEGOFF) (0xA0000000);
	IOByteWrite (GDC_INDEX, 0x05);
	IOByteWrite (GDC_DATA, (BYTE) (IOByteRead (GDC_DATA) | 2));	// Write mode 2
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < (int) (wXRes/8)*((BYTE)wYRes/16); j++)
			MemByteWrite (lpVideo++, (BYTE) i);
	}

	SimDumpMemory ("T0610.VGA");

	// Verify starting frame
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto GraphicsModeBlinkTest_exit;
		}
	}

	// Enable blinking (color graphics mode)
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x30);
	IOByteWrite (ATC_INDEX, (BYTE) (IOByteRead (ATC_RDATA) | 0x08));
	WaitAttrBlink (BLINK_OFF);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto GraphicsModeBlinkTest_exit;
		}
	}
	else
	{
		WaitAttrBlink (BLINK_ON);
		FrameCapture (REF_PART, REF_TEST);
	}

	// Enable monochrome attributes
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x30);
	IOByteWrite (ATC_INDEX, (BYTE) (IOByteRead (ATC_RDATA) | 0x02));
	WaitAttrBlink (BLINK_OFF);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto GraphicsModeBlinkTest_exit;
		}
	}
	else
	{
		WaitAttrBlink (BLINK_ON);
		FrameCapture (REF_PART, REF_TEST);
	}

GraphicsModeBlinkTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		ColorPlaneEnableTest - Fill video memory and cycle through all possible
//										color plane enable values (16).
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int ColorPlaneEnableTest (void)
{
	int		nErr, i;
	SEGOFF	lpVideo;
	WORD		wXRes, wYRes;
	WORD		wSimType;
	BOOL		bFullVGA;
   	int	REF_PART = 1;
   	int	REF_TEST = 10;

  	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto ColorPlaneEnableTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	GetResolution (&wXRes, &wYRes);
	SimSetState (TRUE, TRUE, TRUE);

	lpVideo = (SEGOFF) (0xA0000000);
	for (i = 0; (WORD) i < (WORD) (wXRes/8)*wYRes; i++)
		MemByteWrite (lpVideo++, 0xFF);

	SimDumpMemory ("T0609.VGA");

	for (i = 0; i < 16; i++)
	{
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x32);
		IOByteWrite (ATC_INDEX, (BYTE) i);

		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto ColorPlaneEnableTest_exit;
			}
		}
	}

ColorPlaneEnableTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		PixelWidthTest - Draw a sequence of vertical lines and double the line
//								width by setting the pixel data to change on
//								every other dot clock.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int PixelWidthTest (void)
{
	int	nErr, i;
	WORD	x, wXRes, wYRes;
	WORD	wSimType;
	BOOL	bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 11;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto PixelWidthTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	GetResolution (&wXRes, &wYRes);
	SimSetState (TRUE, TRUE, TRUE);

	for (x = 0, i = 0; x < wXRes; x+=2, i++)
		Line4 (x, 0, x, wYRes - 1, (BYTE) (i%16));

	SimDumpMemory ("T0608.VGA");

	SimGetKey ();

	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x30);
	IOByteWrite (ATC_INDEX, (BYTE) (IOByteRead (ATC_RDATA) | 0x40));

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

PixelWidthTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		V67Test - Draw a pattern into memory and change the palette by
//						changing the "color page".
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int V67Test (void)
{
	int			nErr, i;
	static BYTE	dac[256][3];
	WORD			wXRes, wYRes, x;
	WORD			wSimType;
	BOOL			bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 21;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto V67Test_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	GetResolution (&wXRes, &wYRes);
	SimSetState (TRUE, TRUE, TRUE);

	// Load the DAC with a known pattern
	for (i = 0; i < 64; i++)
	{
		// First 64 locations are "normal"
		dac[i][0] = tbl16ColorDAC[i*3];
		dac[i][1] = tbl16ColorDAC[i*3 + 1];
		dac[i][2] = tbl16ColorDAC[i*3 + 2];

		// Next 64 are filled with red
		dac[i+64][0] = 0x3F;							// Red
		dac[i+64][1] = 0;
		dac[i+64][2] = 0;

		// Next 64 are filled with blue
		dac[i+128][0] = 0;							// Blue
		dac[i+128][1] = 0x3F;
		dac[i+128][2] = 0;

		// Last 64 are filled with green
		dac[i+192][0] = 0;							// Green
		dac[i+192][1] = 0;
		dac[i+192][2] = 0x3F;
	}
	dac[64][0] = 0;			// First location in each block needs to
	dac[128][1] = 0;			//  be 0 for background color to be black
	dac[192][2] = 0;
	SetDacBlock ((LPBYTE) dac, 0, 256);

	SimDumpMemory ("T0607.VGA");

	// Draw lines
	SetLine4Columns (wXRes / 8);
	for (x = 0, i = 0; i < (int) (wXRes / 10); i++, x+=10)
		Line4 (x, 0, x, 479, (BYTE) (i%16));

	// Switch pages (first time through should have no effect)
	for (i = 0; i < 4; i++)
	{
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x34);
		IOByteWrite (ATC_INDEX, (BYTE) (i << 2));
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto V67Test_exit;
			}
		}
	}

V67Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		V54Test - Draw a pattern into memory and change the palette by
//						enabling the V54 select bit and switching through
//						four "sub-pages" of color data.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int V54Test (void)
{
	int		nErr, i, j;
	static 	BYTE	dac[64][3];
	WORD		wXRes, wYRes, x;
	WORD		wSimType;
	BOOL		bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 22;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto V54Test_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	GetResolution (&wXRes, &wYRes);
	SimSetState (TRUE, TRUE, TRUE);

	// Load the DAC with a known pattern
	for (i = 0; i < 16; i++)
	{
		dac[i][0] = 0x3F;
		dac[i][1] = 0;
		dac[i][2] = 0;
		dac[i+16][0] = 0;
		dac[i+16][1] = 0x3F;
		dac[i+16][2] = 0;
		dac[i+32][0] = 0;
		dac[i+32][1] = 0;
		dac[i+32][2] = 0x3F;
		dac[i+48][0] = 0x3F;
		dac[i+48][1] = 0x3F;
		dac[i+48][2] = 0x3F;
	}
	dac[0][0] = 0;				// First location in each block needs to
	dac[16][1] = 0;			//  be 0 for background color to be black
	dac[32][2] = 0;
	dac[48][0] = 0;
	dac[48][1] = 0;
	dac[48][2] = 0;

	SetDacBlock ((LPBYTE) dac, 0, 64);

	SimDumpMemory ("T0606.VGA");

	// Load the attribute controller with "linear" values
	for (i = 0; i < 16; i++)
	{
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, (BYTE) i);
		IOByteWrite (ATC_INDEX, (BYTE) i);
	}
	IOByteWrite (ATC_INDEX, 0x20);			// Re-enable video

	// Draw lines
	SetLine4Columns (wXRes / 8);
	for (x = 0, i = 0; i < (int) (wXRes / 10); i++, x+=10)
		Line4 (x, 0, x, wYRes - 1, (BYTE) (i%16));

	// Switch pages (first time through should have no effect)
	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < 4; i++)
		{
			IOByteRead (INPUT_CSTATUS_1);
			IOByteWrite (ATC_INDEX, 0x34);
			IOByteWrite (ATC_INDEX, (BYTE) i);
			if (!FrameCapture (REF_PART, REF_TEST))
			{
				if (GetKey () == KEY_ESCAPE)
				{
					nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
					goto V54Test_exit;
				}
			}
		}
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x30);
		IOByteWrite (ATC_INDEX, (BYTE) (IOByteRead (ATC_RDATA) | 0x80));
	}

V54Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		OverscanTest - Place varying values in the overscan register and verify
//							that the correct DAC location is addressed.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int OverscanTest (void)
{
	int		nErr, i;
	BYTE		byActual;
	WORD		wSimType;
	int	REF_PART = 1;
	int	REF_TEST = 28;

	nErr = ERROR_NONE;

	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\n\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);

	for (i = 0; i < 64; i++)
	{
		SetDac ((BYTE) i, 0, 0, (BYTE) i);
		SetDac ((BYTE) (i + 64), 0, (BYTE) i, 0);
		SetDac ((BYTE) (i + 128), (BYTE) i, 0, 0);
		SetDac ((BYTE) (i + 192), (BYTE) i, (BYTE) i, (BYTE) i);
	}

	SimDumpMemory ("T0605.VGA");

	for (i = 0; i < 256; i++)
	{
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x11);				// Allow overscan to fill the screen
		IOByteWrite (ATC_INDEX, (BYTE) i);
		byActual = GetNonVideoData ();
		if (byActual != (BYTE) i)
		{
			nErr = FlagError (ERROR_VIDEO, REF_PART, REF_TEST, 0, 0, i, byActual);
			goto OverscanTest_exit;
		}
	}

OverscanTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		InternalPaletteTest - Set all possible internal palette values and
//										verify that the proper index is sent to
//										the RAMDAC.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int InternalPaletteTest (void)
{
	int		nErr, i, j;
	SEGOFF	lpVideo;
	BYTE		byActual;
	WORD 		wSimType;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;

	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\n\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}


	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);

	for (i = 0; i < 16; i++)
	{
		// Fill memory with one value
		lpVideo = (SEGOFF) 0xA0000000;
		IOWordWrite (GDC_INDEX, 0x0205);			// Write mode 2
		for (j = 0; (WORD) j < (WORD) (640/8) * 480; j++)
			MemByteWrite (lpVideo++, (BYTE) i);

		IOWordWrite (GDC_INDEX, 0x0005);			// Write mode 0
		for (j = 0; j < 64; j++)
		{
			IOByteRead (INPUT_CSTATUS_1);
			IOByteWrite (ATC_INDEX, (BYTE) i);
			IOByteWrite (ATC_INDEX, (BYTE) j);	// Video data
			IOByteWrite (ATC_INDEX, 0x31);
			IOByteWrite (ATC_INDEX, (BYTE) j);	// Overscan = video data
			byActual = GetVideoData ();
			if (byActual != (BYTE) j)
			{
				nErr = FlagError (ERROR_VIDEO, REF_PART, REF_TEST, 0, 0, j, byActual);
				goto InternalPaletteTest_exit;
			}
		}
	}

	SimDumpMemory ("T0604.VGA");

InternalPaletteTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		VideoStatusTest - Set various palette data and read it back from the
//								status register.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
typedef struct tagMUXDATA {
	BYTE	bit0;
	BYTE	shl0;
	BYTE	shr0;
	BYTE	bit1;
	BYTE	shl1;
	BYTE	shr1;
} MUXDATA;



int VideoStatusTest (void)
{
	static MUXDATA mx[] = {
		{0x01, 4, 0, 0x04, 3, 0},
		{0x10, 0, 0, 0x20, 0, 0},
		{0x02, 3, 0, 0x08, 2, 0},
		{0x40, 0, 2, 0x80, 0, 2}
	};
	BYTE		tblData[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
								0x00, 0x55, 0xAA, 0xFF, 0xCC, 0x33, 0x66, 0x99};
	BYTE		temp, byExpected, byActual;
	int		nErr, i, j;
	WORD 		wSimType;
	int	REF_PART = 1;
	int	REF_TEST = 29;

	nErr = ERROR_NONE;

	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\n\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);

	for (i = 0; i < 4; i++)
	{
		// Set video status mux
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x32);
		temp = IOByteRead (ATC_RDATA);
		IOByteWrite (ATC_INDEX, (BYTE) ((temp & 0x0F) | (i << 4)));
		for (j = 0; j < sizeof (tblData); j++)
		{
			IOByteRead (INPUT_CSTATUS_1);
			IOByteWrite (ATC_INDEX, 0x00);				// Set palette bits 0-5
			IOByteWrite (ATC_INDEX, (BYTE) (tblData[j] & 0x3F));
			IOByteWrite (ATC_INDEX, 0x34);				// Set palette bits 6-7
			IOByteWrite (ATC_INDEX, (BYTE) ((tblData[j] & 0xC0) >> 4));
			IOByteWrite (ATC_INDEX, 0x31);				// Must include overscan
			IOByteWrite (ATC_INDEX, tblData[j]);

			while (((byActual = IOByteRead (INPUT_CSTATUS_1)) & 1) == 1);
			byActual &= 0x30;
			byExpected = (((tblData[j] & mx[i].bit0) << mx[i].shl0) >> mx[i].shr0) |
								(((tblData[j] & mx[i].bit1) << mx[i].shl1) >> mx[i].shr1);
			if (byActual != byExpected)
			{
				nErr = FlagError (ERROR_VIDEO, REF_PART, REF_TEST, 0, 0, byExpected, byActual);
				goto VideoStatusTest_exit;
			}
		}
	}

	SimDumpMemory ("T0603.VGA");

VideoStatusTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		BlinkVsIntensityTest - Enable and disable the text attribute blink and
//										intensity bit with a known pattern in video
//										memory.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int BlinkVsIntensityTest (void)
{
	int		nErr, i, j;
	SEGOFF	lpVideo;
	BYTE		chr;
	WORD		wSimType;
	BOOL		bFullVGA;
	int		nRowCount, nColCount, nNextRow;
	BYTE		byCharStart;
	int	REF_PART = 1;
	int	REF_TEST = 43;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto BlinkVsIntensityTest_exit;

	// If the VGA simulation library is being used to generate tests
	// on a system where full VGA access is too slow, then use a small
	// frame with only one character row.
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);						// Use full frame
		lpVideo = (SEGOFF) 0xB8000650;			// 10 rows down, 8th column
		nRowCount = 4;
		nColCount = 64;
		nNextRow = 32;
		byCharStart = 0;
	}
	else
	{
		SimSetFrameSize (FALSE);					// Use small frame
		lpVideo = (SEGOFF) 0xB8000000;			// 0 rows down, 0th column
		nRowCount = 1;
		nColCount = 40;
		nNextRow = 0;
		byCharStart = 108;
	}

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);
	DisableCursor ();

	chr = byCharStart;
	for (i = 0; i < nRowCount; i++)				// 4 rows
	{
		for (j = 0; j < nColCount; j++)			// 64 words
		{
			MemByteWrite (lpVideo++, chr);		// ASCII character code
			MemByteWrite (lpVideo++, chr);		// Use it as the attribute as well
			chr++;
		}
		lpVideo += nNextRow;
	}

	SimDumpMemory ("T0602.VGA");

	WaitAttrBlink (BLINK_ON);
   //	if (!FrameCapture (REF_PART, REF_TEST))
   //	{
   //		if (GetKey () == KEY_ESCAPE)
   //		{
   //			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
   //			goto BlinkVsIntensityTest_exit;
   //		}
   //	}

	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x30);
	IOByteWrite (ATC_INDEX, (BYTE) (IOByteRead (ATC_RDATA) & 0xF7));

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

BlinkVsIntensityTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		PaletteAddressSourceTest - Determine that the attribute controller index
//											contains a bit (bit 5) that disables and
//											enables the CRTC access to the internal
//											(EGA-style) palette.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int PaletteAddressSourceTest (void)
{
	int	nErr;
	WORD	wSimType;
	BOOL	bFullVGA;
	int	nRow, nCol, nRowInc;
	char	szMsg0[] = "Now you see it.                      ";
	char	szMsg1[] = "Now you don't.                       ";
	char	szMsg2[] = "Now you see it again (with overscan).";
	int	REF_PART = 1;
	int	REF_TEST = 16;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto PAST_exit;

	// If the VGA simulation library is being used to generate tests
	// on a system where full VGA access is too slow, then use a small
	// frame with only one character row.
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);					// Use full frame
		nRow = 1;
		nCol = 1;
		nRowInc = 1;
	}
	else
	{
		SimSetFrameSize (FALSE);				// Use small frame
		nRow = 0;
		nCol = 0;
		nRowInc = 0;
	}

	SimSetState (TRUE, TRUE, FALSE);			// Ignore DAC writes
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);
	SimSetCaptureMode (CAP_COMPOSITE);

	// Enabled
	TextStringOut (szMsg0, ATTR_NORMAL, nCol, nRow, 0);
	WaitCursorBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto PAST_exit;
		}
	}
	nRow += nRowInc;

	SimDumpMemory ("T0601.VGA");

	// Disabled
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x00);
	TextStringOut (szMsg1, ATTR_NORMAL, nCol, nRow, 0);
	WaitCursorBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto PAST_exit;
		}
	}
	nRow += nRowInc;

	// Blue screen with overscan
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x11);
	IOByteWrite (ATC_INDEX, 0x01);
	WaitCursorBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto PAST_exit;
		}
	}

	// Re-enabled
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x20);
	TextStringOut (szMsg2, ATTR_NORMAL, nCol, nRow, 0);
	WaitCursorBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

PAST_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		ShiftRegisterModeTest - Fill memory with a pattern and read back for
//										an expected value in both mode 13h and mode 05h.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int ShiftRegisterModeTest (void)
{
	int		nErr, n;
	SEGOFF	lpVideo;
	DWORD		offset;
	BYTE		pattern[] = {0x82, 0x22, 0x0A, 0x2};
	BYTE		red[] = {0x00, 0x00, 0x2A, 0x00};
	BYTE		green[] = {0x00, 0x2A, 0x00, 0x2A};
	BYTE		blue[] = {0x2A, 0x00, 0x00, 0x00};
	BYTE		pattern2[] = {0x55, 0x33};
	WORD		wSimType;
	BOOL		bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 8;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto ShiftRegisterModeTest;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// I/O, memory, no DAC
	SetMode (0x13);
	SimSetState (FALSE, FALSE, FALSE);
	lpVideo = (SEGOFF) 0xA0000000;
	offset = 0;
	n = 0;
	SimSetState (TRUE, TRUE, TRUE);

	for (offset = 0; offset < 0x10000; offset++)
	{
		MemByteWrite (lpVideo++, pattern[n]);
		if (++n >= sizeof (pattern)) n = 0;
	}

	SimDumpMemory ("T0513.VGA");
	SimSetState (TRUE, TRUE, FALSE);

	// Make the screen somewhat aesthetically pleasing (Note: don't mess
	// with any DAC index that is used cross-plane. For example, DAC[02h]
	// should always be 00h,2Ah,00h). Screen should be B,G,R,G at this point.
	for (n = 0; n < sizeof (pattern); n++)
	{
		SetDac (pattern[n], red[n], green[n], blue[n]);
	}

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto ShiftRegisterModeTest;
		}
	}

	// Set cross planar shifting. Screen should be B,G,R,W at this point.
	IOWordWrite (GDC_INDEX, 0x0005);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto ShiftRegisterModeTest;
		}
	}

	SetMode (0x05);
	lpVideo = (SEGOFF) 0xB8000000;
	offset = 0;

	// Fill the screen with a pattern. Screen should be C,C,C,C,K,W,K,W.
	n = 0;
	for (offset = 0; offset < 0x8000; offset++)
	{
		MemByteWrite (lpVideo++, pattern2[n]);
		if (++n >= sizeof (pattern2)) n = 0;
	}
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto ShiftRegisterModeTest;
		}
	}

	// Set cross planar shifting. Screen should be K,C,M,W at this point.
	IOWordWrite (GDC_INDEX, 0x1005);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto ShiftRegisterModeTest;
		}
	}

ShiftRegisterModeTest:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		NonPlanarWriteMode3Test - Fill memory with a pattern and read back for
//										an expected value in both mode 13h and mode 05h.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int NonPlanarWriteMode3Test (void)
{
	int		nErr, i;
	SEGOFF	lpVideo;
	DWORD		offset;
	BYTE		bySetReset, byMask, temp, byMemData, plane, byData;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	SimSetState (TRUE, TRUE, FALSE);			// I/O, memory, no DAC
	SetMode (0x13);
	lpVideo = (SEGOFF) 0xA0000000;

	// Preload memory
	for (offset = 0; offset < 0x10000; offset += 256)
	{
		temp = (BYTE) (offset / 256);
		for (i = 0; i < 256; i++)
			MemByteWrite (lpVideo + offset + i, temp);
	}

	SimDumpMemory ("T0512.VGA");
	SimSetState (TRUE, TRUE, FALSE);

	IOWordWrite (GDC_INDEX, 0x4305);						// Set write mode 3

	// Fill with a pattern
	byMask = 0;
	bySetReset = 0;
	for (offset = 0; offset < 0x10000; offset++)
	{
		IOByteWrite (GDC_INDEX, 0x00);
		IOByteWrite (GDC_DATA, bySetReset);
		temp = MemByteRead (lpVideo + offset);			// Load latches
		MemByteWrite (lpVideo + offset, byMask);
		byMask += 3;
		bySetReset++;
	}

	// Verify memory
	byMask = 0;
	bySetReset = 0;
	for (offset = 0; offset < 0x10000; offset++)
	{
		byMemData = (BYTE) (offset / 256);
		plane = (BYTE) (offset % 4);
		byData = (bySetReset & (1 << plane)) ? 0xFF : 0x00;
		byData = byData & byMask;
		byMemData = byMemData & ~byMask;
		byData = byData | byMemData;
		if (MemByteRead (lpVideo + offset) != byData)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								LOWORD (lpVideo + offset),
								HIWORD (lpVideo), byData, MemByteRead (lpVideo + offset));
			goto NonPlanarWriteMode3Test;
		}
		byMask += 3;
		bySetReset++;
	}

	// Now chain/2 mode
	SetMode (0x05);
	lpVideo = (SEGOFF) 0xB8000000;

	// Preload memory
	for (offset = 0; offset < 0x8000; offset += 128)
	{
		temp = (BYTE) (offset / 128);
		for (i = 0; i < 128; i++)
			MemByteWrite (lpVideo + offset + i, temp);
	}

	IOWordWrite (GDC_INDEX, 0x3305);						// Set write mode 3

	// Fill with a pattern
	byMask = 0;
	bySetReset = 0;
	for (offset = 0; offset < 0x8000; offset++)
	{
		IOByteWrite (GDC_INDEX, 0x00);
		IOByteWrite (GDC_DATA, bySetReset);
		temp = MemByteRead (lpVideo + offset);			// Load latches
		MemByteWrite (lpVideo + offset, byMask);
		byMask += 3;
		bySetReset++;
	}

	// Verify memory
	byMask = 0;
	bySetReset = 0;
	for (offset = 0; offset < 0x8000; offset++)
	{
		byMemData = (BYTE) (offset / 128);
		plane = (BYTE) (offset % 2);
		byData = (bySetReset & (1 << plane)) ? 0xFF : 0x00;
		byData = byData & byMask;
		byMemData = byMemData & ~byMask;
		byData = byData | byMemData;
		if (MemByteRead (lpVideo + offset) != byData)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								LOWORD (lpVideo + offset),
								HIWORD (lpVideo), byData, MemByteRead (lpVideo + offset));
			goto NonPlanarWriteMode3Test;
		}
		byMask += 3;
		bySetReset++;
	}

NonPlanarWriteMode3Test:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		NonPlanarWriteMode2Test - Fill memory with a pattern and read back for
//										an expected value in both mode 13h and mode 05h.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int NonPlanarWriteMode2Test (void)
{
	int		nErr, i, j;
	SEGOFF	lpVideo;
	DWORD		offset, l;
	BYTE		byData;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	SimSetState (TRUE, TRUE, FALSE);				// I/O, memory, no DAC
	SetMode (0x13);
	lpVideo = (SEGOFF) 0xA0000000;

	IOWordWrite (GDC_INDEX, 0x4205);				// Set write mode 2
	for (i = 0; i < 16; i++)
	{
		for (l = 0; l < 0x10000; l++)
			MemByteWrite (lpVideo + l, (BYTE) i);
		for (j = 0; j < 4; j++)
		{
			// Chain/4 blocks writes to planes based on address bits MA0..1
			byData = (i & (1 << j)) ? 0xFF : 0x00;
			for (offset = 0; offset < 0x10000; offset += 4)
			{
				if (MemByteRead (lpVideo + offset + j) != byData)
				{
					nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
										LOWORD (lpVideo + offset + j),
										HIWORD (lpVideo), byData, MemByteRead (lpVideo + offset + j));
					goto NonPlanarWriteMode2Test;
				}
			}
		}
	}

	SimDumpMemory ("T0511.VGA");

	SetMode (0x05);
	lpVideo = (SEGOFF) 0xB8000000;
	IOWordWrite (GDC_INDEX, 0x3205);			// Set write mode 2
	for (i = 0; i < 16; i++)
	{
		for (l = 0; l < 0x8000; l++)
			MemByteWrite (lpVideo + l, (BYTE) i);

		for (j = 0; j < 2; j++)
		{
			// Chain/2 blocks writes to planes based on address bit MA0
			byData = (i & (1 << j)) ? 0xFF : 0x00;
			for (offset = 0; offset < 0x8000; offset += 2)
			{
				if (MemByteRead (lpVideo + offset + j) != byData)
				{
					nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
										LOWORD (lpVideo + offset + j),
										HIWORD (lpVideo), byData, MemByteRead (lpVideo + offset + j));
					goto NonPlanarWriteMode2Test;
				}
			}
		}
	}

NonPlanarWriteMode2Test:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		NonPlanarWriteMode1Test - Fill memory with a pattern and read back for
//										an expected value in both mode 13h and mode 05h.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int NonPlanarWriteMode1Test (void)
{
	int		nErr, i;
	SEGOFF	lpVideo;
	BYTE		temp;
	WORD		wSimType;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

	SimSetState (TRUE, TRUE, FALSE);				// I/O, memory, no DAC
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x12);									// To clear all planes of planar memory
	SimSetState (TRUE, FALSE, FALSE);			// I/O, no memory, no DAC
	SetMode (0x13);
	SimSetState (TRUE, TRUE, TRUE);
	lpVideo = (SEGOFF) 0xA0000000;

	// Fill first 4K of memory with a known pattern
	for (i = 0; i < 4*1024; i++)
	{
		MemByteWrite (lpVideo++, 0x00);
		MemByteWrite (lpVideo++, 0x55);
		MemByteWrite (lpVideo++, 0xAA);
		MemByteWrite (lpVideo++, 0xFF);
	}

	SimDumpMemory ("T0510.VGA");

	IOWordWrite (GDC_INDEX, 0x4105);		// Write mode 1
	lpVideo = (SEGOFF) 0xA0000000;
	for (i = 0; i < 0x4000; i += 4)
	{
		temp = MemByteRead (lpVideo + i);
		MemByteWrite (lpVideo + 0x4001 + i, 0);
		MemByteWrite (lpVideo + 0x8002 + i, 0);
		MemByteWrite (lpVideo + 0xC003 + i, 0);
	}

	// Even though the map mask enables all planes, the chain/4 only allows
	// the write to go to one plane (based on memory addressing).
	// Verify memory
	for (i = 0; i < 0x4000; i += 4)
	{
		// 000h, 055h, 000h, 000h
		if (MemByteRead (lpVideo + 0x4000 + i) != 0x00)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0x4000 + i),
									HIWORD (lpVideo), 0x00, MemByteRead (lpVideo + 0x4000 + i));
			goto NonPlanarWriteMode1Test;
		}
		if (MemByteRead (lpVideo + 0x4001 + i) != 0x55)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0x4001 + i),
									HIWORD (lpVideo), 0x55, MemByteRead (lpVideo + 0x4001 + i));
			goto NonPlanarWriteMode1Test;
		}
		if (MemByteRead (lpVideo + 0x4002 + i) != 0x00)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0x4002 + i),
									HIWORD (lpVideo), 0x00, MemByteRead (lpVideo + 0x4002 + i));
			goto NonPlanarWriteMode1Test;
		}
		if (MemByteRead (lpVideo + 0x4003 + i) != 0x00)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0x4003 + i),
									HIWORD (lpVideo), 0x00, MemByteRead (lpVideo + 0x4003 + i));
			goto NonPlanarWriteMode1Test;
		}

		// 000h, 000h, 0AAh, 000h
		if (MemByteRead (lpVideo + 0x8000 + i) != 0x00)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0x8000 + i),
									HIWORD (lpVideo), 0x00, MemByteRead (lpVideo + 0x8000 + i));
			goto NonPlanarWriteMode1Test;
		}
		if (MemByteRead (lpVideo + 0x8001 + i) != 0x00)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0x8001 + i),
									HIWORD (lpVideo), 0x00, MemByteRead (lpVideo + 0x8001 + i));
			goto NonPlanarWriteMode1Test;
		}
		if (MemByteRead (lpVideo + 0x8002 + i) != 0xAA)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0x8002 + i),
									HIWORD (lpVideo), 0xAA, MemByteRead (lpVideo + 0x8002 + i));
			goto NonPlanarWriteMode1Test;
		}
		if (MemByteRead (lpVideo + 0x8003 + i) != 0x00)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0x8003 + i),
									HIWORD (lpVideo), 0x00, MemByteRead (lpVideo + 0x8003 + i));
			goto NonPlanarWriteMode1Test;
		}

		// 000h, 000h, 000h, 0FFh
		if (MemByteRead (lpVideo + 0xC000 + i) != 0x00)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0xC000 + i),
									HIWORD (lpVideo), 0x00, MemByteRead (lpVideo + 0xC000 + i));
			goto NonPlanarWriteMode1Test;
		}
		if (MemByteRead (lpVideo + 0xC001 + i) != 0x00)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0xC001 + i),
									HIWORD (lpVideo), 0x00, MemByteRead (lpVideo + 0xC001 + i));
			goto NonPlanarWriteMode1Test;
		}
		if (MemByteRead (lpVideo + 0xC002 + i) != 0x00)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0xC002 + i),
									HIWORD (lpVideo), 0x00, MemByteRead (lpVideo + 0xC002 + i));
			goto NonPlanarWriteMode1Test;
		}
		if (MemByteRead (lpVideo + 0xC003 + i) != 0xFF)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0xC003 + i),
									HIWORD (lpVideo), 0xFF, MemByteRead (lpVideo + 0xC003 + i));
			goto NonPlanarWriteMode1Test;
		}
	}

	// Now test chain/2 stuff
	SetMode (0x05);
	lpVideo = (SEGOFF) 0xB8000000;
	for (i = 0; i < 0x2000; i++)
		MemByteWrite (lpVideo + i, 0x1B);

	IOWordWrite (GDC_INDEX, 0x3105);		// Write mode 1
	for (i = 0; i < 0x2000; i += 2)
	{
		temp = MemByteRead (lpVideo + i);
		MemByteWrite (lpVideo + 0x2001 + i, 0);
	}

	// Even though the map mask enables plane 0 & 1, the chain/2 only allows
	// the write to go to one plane (based on memory addressing).
	// Verify memory
	for (i = 0; i < 0x2000; i += 2)
	{
		// 000h, 01Bh
		if (MemByteRead (lpVideo + 0x2000 + i) != 0x00)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0x2000 + i),
									HIWORD (lpVideo), 0x00, MemByteRead (lpVideo + 0x2000 + i));
			goto NonPlanarWriteMode1Test;
		}
		if (MemByteRead (lpVideo + 0x2001 + i) != 0x1B)
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									LOWORD (lpVideo + 0x2001 + i),
									HIWORD (lpVideo), 0x1B, MemByteRead (lpVideo + 0x2001 + i));
			goto NonPlanarWriteMode1Test;
		}
	}

NonPlanarWriteMode1Test:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		NonPlanarReadModeTest - Fill memory with a pattern and read back for
//										an expected value in both mode 13h and mode 05h.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int NonPlanarReadModeTest (void)
{
	int		nErr, i;
	SEGOFF	lpVideo, lpTemp;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	SimSetState (TRUE, FALSE, FALSE);		// I/O, no memory, no DAC
	SetMode (0x13);
	lpVideo = (SEGOFF) 0xA0000000;
	SimSetState (TRUE, TRUE, FALSE);			// I/O, memory, no DAC

	// Fill memory with a known pattern
	for (i = 0; i < 16*1024; i++)
	{
		MemByteWrite (lpVideo++, 0x00);
		MemByteWrite (lpVideo++, 0x55);
		MemByteWrite (lpVideo++, 0xAA);
		MemByteWrite (lpVideo++, 0xFF);
	}

	SimDumpMemory ("T0509.VGA");

	IOWordWrite (GDC_INDEX, 0x4805);		// Read mode 1
	lpVideo = (SEGOFF) 0xA0000000;
	lpTemp = VerifyBytes (lpVideo, 0, 0xFFFF);
	if (lpTemp)
	{
		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, LOWORD (lpTemp),
								HIWORD (lpTemp), 0, MemByteRead (lpTemp));
		goto NonPlanarReadModeTest;
	}

	IOWordWrite (GDC_INDEX, 0x0A02);		// Color compare for "0101" in planes 0..3
	lpTemp = VerifyBytes (lpVideo, 0x55, 0xFFFF);
	if (lpTemp)
	{
		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, LOWORD (lpTemp),
								HIWORD (lpTemp), 0x55, MemByteRead (lpTemp));
		goto NonPlanarReadModeTest;
	}

	IOWordWrite (GDC_INDEX, 0x0C02);		// Color compare for "0011" in planes 0..3
	lpTemp = VerifyBytes (lpVideo, 0xAA, 0xFFFF);
	if (lpTemp)
	{
		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, LOWORD (lpTemp),
								HIWORD (lpTemp), 0xAA, MemByteRead (lpTemp));
		goto NonPlanarReadModeTest;
	}

	// Now test chain/2 stuff
	SetMode (0x05);
	lpVideo = (SEGOFF) 0xB8000000;
	for (i = 0; i < 0x4000; i++)
		MemByteWrite (lpVideo + i, 0x1B);
	IOWordWrite (GDC_INDEX, 0x3805);		// Read mode 1
	IOWordWrite (GDC_INDEX, 0x0307);		// Color "don't" care about non-visible planes
	lpTemp = VerifyBytes (lpVideo, 0xE4, 0x4000);
	if (lpTemp)
	{
		nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, LOWORD (lpTemp),
								HIWORD (lpTemp), 0xE4, MemByteRead (lpTemp));
		goto NonPlanarReadModeTest;
	}

NonPlanarReadModeTest:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		BitMaskTest - Fill video memory with at pattern and set all possible
//							bit mask combinations.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int BitMaskTest (void)
{
	int		nErr, i, j, nBlockSize, nColSize;
	SEGOFF	lpVideo;
	BYTE		byMask, byChr, byData, byExpected, byActual, bySR;
	WORD		wOffset;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	SimSetState (TRUE, TRUE, FALSE);			// I/O, memory, no DAC
	SetMode (0x12);
	lpVideo = (SEGOFF) 0xA0000000;

	if (SimGetType () & SIM_VECTORS)
	{
		nBlockSize = 20;
		nColSize = 5;
	}
	else
	{
		nBlockSize = (480/16);
		nColSize = 80;
	}

	// Fill memory with a pattern
	IOWordWrite (GDC_INDEX, 0x0F01);			// Enable set/reset
	for (i = 0; i < 16; i++)
	{
		IOByteWrite (GDC_INDEX, 0x00);
		IOByteWrite (GDC_DATA, (BYTE) i);
		wOffset = i*nColSize*nBlockSize;
		for (j = 0; j < nColSize*nBlockSize; j++)
			MemByteWrite (lpVideo + wOffset++, 0);
	}

	SimDumpMemory ("T0508.VGA");

	IOWordWrite (GDC_INDEX, 0x0001);			// Disable set/reset

	// Cycle through all combinations of bit masks
	byMask = 0;
	byChr = 0xFF;
	while (LOWORD (lpVideo) < (WORD) (nColSize*nBlockSize*16))
	{
		IOByteWrite (GDC_INDEX, 0x08);
		IOByteWrite (GDC_DATA, byMask);
		byMask++;
		MemByteRead (lpVideo);			// Load latches
		MemByteWrite (lpVideo++, byChr);
	}

	// Read all of memory and verify that it is correct
	lpVideo = (SEGOFF) 0xA0000000;
	byMask = 0;
	while (LOWORD (lpVideo) < (WORD) (nColSize*nBlockSize*16))
	{
		// Calculate the set/reset data
		bySR = (BYTE) ((LOWORD (lpVideo)) / ((WORD) (nColSize*nBlockSize)));

		// Read each plane
		for (i = 0; i < 4; i++)
		{
			// Calculate the video data on this plane
			byData = bySR & (1 << i);
			if (byData != 0) byData = 0xFF;

			// Calculate the new video data after the CPU write
			byExpected = (byData & ~byMask) | (byChr & byMask);

			// Set read map select and get the actual data
			IOByteWrite (GDC_INDEX, 0x04);
			IOByteWrite (GDC_DATA, (BYTE) i);

			if (!MemByteTest (lpVideo, byExpected, &byActual))
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
							LOWORD (lpVideo), HIWORD (lpVideo), byExpected, byActual);
				goto BitMaskTest_exit;
			}
		}
		byMask++;
		lpVideo++;
	}

BitMaskTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		VideoSegmentTest - Set the video segment to various locations and
//									verify that memory exists.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int VideoSegmentTest (void)
{
	static SEGOFF	tblAddress[] = {
		(SEGOFF) 0xB0000000, (SEGOFF) 0xA000FFFF,
		(SEGOFF) 0xB0007FFF, (SEGOFF) 0xB8007FFF
	};
	int	nErr, i;
	BYTE	temp;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	SimSetState (TRUE, FALSE, FALSE);		// I/O, no memory, no DAC
	SetMode (0x12);
	SimSetState (TRUE, TRUE, FALSE);			// I/O, no memory, no DAC

	for (i = 1; i < 4; i++)
	{
		IOByteWrite (GDC_INDEX, 0x06);
		temp = (BYTE) IOByteRead (GDC_DATA);
		temp = (BYTE) ((temp & 0xF3) | (i << 2));
		IOByteWrite (GDC_DATA, temp);
		if (!RWMemoryTest (HIWORD (tblAddress[i]), LOWORD (tblAddress[i])))
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
						LOWORD (tblAddress[i]), HIWORD (tblAddress[i]),
						0, 0xFF);
			break;
		}
	}

	SimDumpMemory ("T0507.VGA");

	SystemCleanUp ();
	return (nErr);
}
//
//
//		WriteMode3Test - Draw font images over a colored background without
//								affecting the background.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//

#define		COUNT_BITMASKS			2
#define		CHR_INCER_1				3
#define		CHR_INCER_2				7

int WriteMode3Test (void)
{
	int		nErr, i, j, k, l, nCountChars;
	SEGOFF	lpVideo;
	BYTE		byMemData, byCPUData, byExpected, byActual, byBitMask;
	BYTE		tblBitMasks[COUNT_BITMASKS] = {0x55, 0xFF};

 	int	REF_PART = 99;
  	int	REF_TEST = 99;
	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;
	SimSetState (TRUE, TRUE, FALSE);		// I/O, no memory, no DAC
	SetMode (0x12);

	// If test vectors, use only a quarter of the combinations
	if (SimGetType () & SIM_VECTORS)
		nCountChars = 64;
	else
		nCountChars = 256;

	// Fill memory with various data combinations
	byMemData = 0x00;
	IOByteWrite (SEQ_INDEX, 0x02);
	for (i = 0; i < 16*nCountChars*COUNT_BITMASKS; i++)
	{
		for (j = 0; j < 4; j++)
		{
			IOByteWrite (SEQ_DATA, (BYTE) (0x01 << j));
			MemByteWrite (lpVideo, byMemData);
			byMemData += CHR_INCER_1;
		}
		lpVideo++;
	}

	IOWordWrite (SEQ_INDEX, 0x0F02);
	SimDumpMemory ("T0506.VGA");

	// Set write mode 3 and draw a combination of data into memory
	lpVideo = (SEGOFF) 0xA0000000;
	IOWordWrite (GDC_INDEX, 0x0305);
	byCPUData = 0x00;
	for (i = 0; i < nCountChars; i++)
	{
		for (j = 0; j < COUNT_BITMASKS; j++)
		{
			IOWordWrite (GDC_INDEX, (tblBitMasks[j] << 8) | 0x08);
			IOByteWrite (GDC_INDEX, 0);
			for (k = 0; k < 16; k++)
			{
				IOByteWrite (GDC_DATA, (BYTE) k);
				MemByteRead (lpVideo);				// Load latches
				MemByteWrite (lpVideo, byCPUData);
				lpVideo++;
			}
		}
		byCPUData += CHR_INCER_2;
	}

	// Verify video data
	lpVideo = (SEGOFF) 0xA0000000;
	byMemData = 0x00;
	byCPUData = 0x00;
	IOByteWrite (GDC_INDEX, 0x04);
	for (i = 0; i < nCountChars; i++)			// Cycle through CPU data
	{
		for (j = 0; j < COUNT_BITMASKS; j++)	// Cycle through bitmasks
		{
			for (k = 0; k < 16; k++)				// Cycle through Set/Reset data
			{
				for (l = 0; l < 4; l++)				// Cycle through each plane
				{
					IOByteWrite (GDC_DATA, (BYTE) l);
					// Calculate Set/Reset data
					byExpected = ((k & (1 << l)) == 0) ? 0x00 : 0xFF;
					byBitMask = byCPUData & tblBitMasks[j];
					byExpected = (byExpected & byBitMask) | (byMemData & ~byBitMask);
					if (!MemByteTest (lpVideo, byExpected, &byActual))
					{
						nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
												LOWORD (lpVideo), HIWORD (lpVideo),
												byExpected, byActual);
						goto WriteMode3Test_exit;
					}
					byMemData += CHR_INCER_1;
				}
				lpVideo++;
			}
		}
		byCPUData += CHR_INCER_2;
	}

WriteMode3Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		WriteMode2Test - Draw a series of rectangles without doing any I/O
//								to change colors.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int WriteMode2Test (void)
{
	int			nErr;
	SEGOFF		lpVideo;
	int			i, j;
	BYTE			byROP, byRotate, byData, byCPU, byExpected, byActual;
	BYTE			byMem;
	int			nBlockSize, nColSize;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	SimSetState (TRUE, FALSE, FALSE);		// I/O, no memory, no DAC
	SetMode (0x12);
	lpVideo = (SEGOFF) 0xA0000000;
	SimDumpMemory ("T0505.VGA");
	SimSetState (TRUE, TRUE, FALSE);			// I/O, memory, no DAC

	// For test vectors, only use part of the screen
	if (SimGetType () & SIM_VECTORS)
	{
		nBlockSize = 5;
		nColSize = 10;
	}
	else
	{
		nBlockSize = 80;
		nColSize = (480/16);
	}

	IOByteWrite (GDC_INDEX, 0x05);
	IOByteWrite (GDC_DATA, (BYTE) (IOByteRead (GDC_DATA) | 2));	// Write mode 2

	for (i = 0; i < 16; i++)								// Cycle through each color
	{
		for (j = 0; j < nColSize*nBlockSize; j++)
			MemByteWrite (lpVideo++, (BYTE) i);
	}

	// Might have to break this function into two files.
	SimDumpMemory ("T0505.VGA");

	lpVideo = (SEGOFF) 0xA0000000;
	byData = byROP = byRotate = 0;
	for (i = 0; i < nBlockSize*16; i++)
	{
		IOByteWrite (GDC_INDEX, 0x03);
		IOByteWrite (GDC_DATA, (BYTE) (byRotate | (byROP << 3)));
		SetLatchedBytes (lpVideo, byData, nColSize);

		for (j = 0; j < 4; j++)							// Cycle through each plane
		{
			IOWordWrite (GDC_INDEX, (j << 8) | 0x04);

			// Calculate video memory byte
			byMem = 0;
			if ((i / nBlockSize) & (1 << j))
				byMem = 0xFF;

			// Calculate CPU byte
			byCPU = 0;
			if (byData & (1 << j))
				byCPU = 0xFF;

			byExpected = byCPU;
			switch (byROP)
			{
				case 1:
					byExpected &= byMem;
					break;
				case 2:
					byExpected |= byMem;
					break;
				case 3:
					byExpected ^= byMem;
					break;
			}
			for (j = 0; j < nColSize; j++)
			{
				if (!MemByteTest (lpVideo + j, byExpected, &byActual))
				{
					nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								LOWORD (lpVideo + j), HIWORD (lpVideo), byExpected, byActual);
					goto WriteMode2Test_exit;
				}
			}
		}

		lpVideo += nColSize;
		byData++;
		byROP = ((byROP + 1) & 0x03);
		byRotate = ((byRotate + 1) & 0x07);
	}

WriteMode2Test_exit:
	SystemCleanUp ();
	return (nErr);
}

//
//		SetLatchedBytes - Set a block of video memory after latching a data value
//
//		Entry:	lpDest	Pointer to destination
//					byData	CPU data value
//					nLength	Number of bytes to move
//		Exit:		None
//
//		Note:	Due to the way VGA latches work, only one BYTE at at time can
//				be moved into video memory. Therefore, the "C" routine, "_fmemset"
//				is useless since it doesn't do the latch load step and is also
//				optimized for speed which may use WORD or DWORD moves.
//
//void SetLatchedBytes (SEGOFF lpDest, BYTE byData, int nLength)
//{
//	int	i;

//	for (i = 0; i < nLength; i++)
//	{
//		MemByteRead (lpDest);
//		MemByteWrite (lpDest++, byData);
//	}
//}
//
//
//		WriteMode1Test - Draw pattern into memory and duplicate it using write
//								mode 1
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int WriteMode1Test (void)
{
	int		nErr, i;
	SEGOFF	lpVideo;
	WORD		dest, nLength, j, wScanSize;
	BYTE		byExpected, byActual;
	BYTE		byData, byIncr;

 	int	REF_PART = 99;
  	int	REF_TEST = 99;
	nErr = ERROR_NONE;
	SimSetState (TRUE, FALSE, FALSE);		// I/O, no memory, no DAC
	SetMode (0x12);
	SimSetState (TRUE, TRUE, FALSE);

	lpVideo = (SEGOFF) 0xA0000000;

	// For test vectors only use two scan lines
	if (SimGetType () & SIM_VECTORS)
		wScanSize = 2;
	else
		wScanSize = 120;

	// Draw a pattern into memory
	byData = 0;
	byIncr = 1;
	for (i = 0; i < 640; i++)
	{
		Line4 (i, 0, i, wScanSize - 1, byData);
		// Don't use a regular pattern
		if ((++byData & 0x0F) == 0)
		{
			byData += byIncr;
			byIncr++;
		}
	}

	SimDumpMemory ("T0504.VGA");

	IOWordWrite (GDC_INDEX, 0x0105);		// Write mode 1
	IOWordWrite (SEQ_INDEX, 0x0F02);		// Enable all planes

	// Move "wScanSize" scan lines
	dest = wScanSize*80;						// Nth scan line
	nLength = wScanSize*80;					// Move N scan lines
	MoveBytes (lpVideo + dest, lpVideo, nLength);

	// Set bit mask and do it again
	IOWordWrite (GDC_INDEX, 0x5508);		// Bit mask
	dest = (wScanSize*2)*80;
	MoveBytes (lpVideo + dest, lpVideo, nLength);

	// Set map mask and do it again
	IOWordWrite (GDC_INDEX, 0xFF08);		// Bit mask
	IOWordWrite (SEQ_INDEX, 0x0302);		// Map mask
	dest = (wScanSize*3)*80;
	MoveBytes (lpVideo + dest, lpVideo, nLength);

	// Read back memory and verify that it is what is expected
	for (i = 0; i < 4; i++)
	{
		IOByteWrite (GDC_INDEX, 0x04);
		IOByteWrite (GDC_DATA, (BYTE) i);	// Read map select
		dest = wScanSize*80;				// Nth scan line
		for (j = 0; j < nLength; j++)
		{
			byExpected = MemByteRead (lpVideo + j);
			if (!MemByteTest (lpVideo + dest + j, byExpected, &byActual))
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
							dest + j, HIWORD (lpVideo), byExpected, byActual);
				goto WriteMode1Test_exit;
			}
		}

		dest = (wScanSize*2)*80;			// (N*2)th scan line
		for (j = 0; j < nLength; j++)
		{
			byExpected = MemByteRead (lpVideo + j);
			if (!MemByteTest (lpVideo + dest + j, byExpected, &byActual))
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
							dest + j, HIWORD (lpVideo), byExpected, byActual);
				goto WriteMode1Test_exit;
			}
		}

		// Plane 0 & 1 only should compare
		dest = (wScanSize*3)*80;		// (N*3)th scan line
		if ((i == 0) || (i == 1))
		{
			for (j = 0; j < nLength; j++)
			{
				byExpected = MemByteRead (lpVideo + j);
				if (!MemByteTest (lpVideo + dest + j, byExpected, &byActual))
				{
					nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								dest + j, HIWORD (lpVideo), byExpected, byActual);
					goto WriteMode1Test_exit;
				}
			}
		}
		else
		{
			byExpected = 0;
			for (j = 0; j < nLength; j++)
			{
				if (!MemByteTest (lpVideo + dest + j, byExpected, &byActual))
				{
					nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
								dest + j, HIWORD (lpVideo), byExpected, byActual);
					goto WriteMode1Test_exit;
				}
			}
		}
	}

WriteMode1Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		ROPTest - Cycle through all possible combinations and match expected
//						data with actual data
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int ROPTest (void)
{
	int		nErr, i, j;
	SEGOFF	lpVideo;
	WORD		wOffset;
	BYTE		byRop, byRotate, byData, byExpected, byActual;
	BYTE		byBefore[4];
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	SimSetState (TRUE, FALSE, FALSE);		// I/O, no memory, no DAC
	SetMode (0x12);
	SimSetState (TRUE, TRUE, FALSE);

	lpVideo = (SEGOFF) 0xA0000000;

	IOWordWrite (GDC_INDEX, 0x0F01);			// Enable set/reset
	wOffset = 0;
	for (i = 0; i < 16; i++)
	{
		IOByteWrite (GDC_INDEX, 0x00);
		IOByteWrite (GDC_DATA, (BYTE) i);
		for (j = 0; j < 256; j++)
			MemByteWrite (lpVideo + wOffset++, 0);
	}
	IOWordWrite (GDC_INDEX, 0x0001);			// Disable set/reset

	SimDumpMemory ("T0503.VGA");

	// Go through every byte on the display
	wOffset = 0;
	byRop = byRotate = byData = 0;
	while (wOffset < (WORD) (16*256))
	{
		// stub out for diags.... 
		//if (_kbhit ())
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto ROPTest_exit;
			}

		IOByteWrite (GDC_INDEX, 0x03);
		IOByteWrite (GDC_DATA, (BYTE) ((byRop << 3) | byRotate));	// Set ROP and rotate value'

		// Get video memory needed for diagnostic (latches data as well)
		IOByteWrite (GDC_INDEX, 0x04);
		for (i = 0; i < 4; i++)
		{
			IOByteWrite (GDC_DATA, (BYTE) i);
			byBefore[i] = MemByteRead (lpVideo + wOffset);
		}

		// Write to memory
		MemByteWrite (lpVideo + wOffset, byData);

		// Get the results of the operation
		for (i = 0; i < 4; i++)
		{
			IOByteWrite (GDC_DATA, (BYTE) i);
			byExpected = RotateByteRight (byData, byRotate);
			switch (byRop)
			{
				case 1:
					byExpected &= byBefore[i];
					break;
				case 2:
					byExpected |= byBefore[i];
					break;
				case 3:
					byExpected ^= byBefore[i];
					break;
			}
			if (!MemByteTest (lpVideo + wOffset, byExpected, &byActual))
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
							wOffset, HIWORD (lpVideo), byExpected, byActual);
				goto ROPTest_exit;
			}
		}

		wOffset++;
		byRop = (BYTE) ((byRop + 1) & 0x03);
		byRotate = (BYTE) ((byRotate + 1) & 0x7);
		byData++;								// Let byte wrap around
	}

ROPTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		ColorCompareTest - Draw a pattern into memory and read it back with
//								various color compare values.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int ColorCompareTest (void)
{
	static BYTE abyExpected[80] = {
		0xFF, 0xFF, 0xFF, 0x55, 0xAA, 0x55, 0x33, 0x33,
		0xCC, 0x44, 0x22, 0x11, 0x0F, 0x0F, 0x0F, 0x05,
		0xA0, 0x50, 0x30, 0x30, 0x0C, 0x04, 0x02, 0x01,
		0x00, 0xFF, 0x00, 0x55, 0x00, 0x55, 0x00, 0x33,
		0xCC, 0x00, 0x22, 0x00, 0x0F, 0x00, 0x0F, 0x00,
		0x00, 0x50, 0x00, 0x30, 0x00, 0x04, 0x00, 0x01,
		0xFF, 0xFF, 0xFF, 0x55, 0xAA, 0x55, 0x33, 0x33,
		0xCC, 0x44, 0x22, 0x11, 0x0F, 0x0F, 0x0F, 0x05,
		0xA0, 0x50, 0x30, 0x30, 0x0C, 0x04, 0x02, 0x01,
		0x00, 0xFF, 0x00, 0x55, 0x00, 0x55, 0x00, 0x33
	};
	WORD		wOffset;
	int		i, j;
	BYTE		byColor, byActual, byMask, tmp;
	int		nErr;
	SEGOFF	lpVideo;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	SimSetState (TRUE, FALSE, FALSE);		// I/O, no memory, no DAC
	SetMode (0x12);

	SimSetState (TRUE, TRUE, FALSE);
	lpVideo = (SEGOFF) 0xA0000000;
	wOffset = (640/8)*(480/2);				// Half way down the screen
	byColor = 0;
	for (i = 0; i < 80; i++)
	{
		byMask = 0x01;
		for (j = 0; j < 8; j++)
		{
			byMask = RotateByteRight (byMask, 1);
			SetPixelAt (lpVideo + wOffset + i, byMask, byColor);
			byColor = (byColor + 1) % 16;
		}
	}

	SimDumpMemory ("T0502.VGA");

	IOByteWrite (GDC_INDEX, 0x05);
	tmp = IOByteRead (GDC_DATA);
	IOByteWrite (GDC_DATA, (BYTE) (tmp | 0x08));		// Set read mode 1
	for (i = 0; i < 80; i++)
	{
		IOByteWrite (GDC_INDEX, 0x07);					// Color don't care
		IOByteWrite (GDC_DATA, (BYTE) ((i/3) % 16));	// Cycle colors on every third pass
		IOByteWrite (GDC_INDEX, 0x02);					// Color compare
		IOByteWrite (GDC_DATA, (BYTE) (i % 16));		// Cycle through colors
		if (!MemByteTest (lpVideo + wOffset + i, abyExpected[i], &byActual))
		{
			nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
									wOffset + i, HIWORD (lpVideo),
									abyExpected[i], byActual);
			goto ColorCompareTest_exit;
		}
	}

ColorCompareTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		SetResetTest - Cycle through each combination of Set/Reset and Enable
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//

#define	COUNT_SETRESET				16
#define	COUNT_SETRESETENABLE		16


int SetResetTest (void)
{
	int		nErr;
	int		i, j, k;
	SEGOFF	lpVideo;
	BYTE		byExpected, byActual, byMemData;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	SimSetState (TRUE, FALSE, FALSE);		// I/O, no memory, no DAC
	SetMode (0x12);

	// Fill memory with a non-zero, non-FFh value
	SimSetState (TRUE, TRUE, TRUE);
	byMemData = 0x55;
	lpVideo = (SEGOFF) 0xA0000000;
	for (i = 0; i < COUNT_SETRESET*COUNT_SETRESETENABLE; i++)
	{
		MemByteWrite (lpVideo++, byMemData);
	}

	SimDumpMemory ("T0501.VGA");

	// Write each possible combination of Set/Reset and Set/Reset Enable
	lpVideo = (SEGOFF) 0xA0000000;
	for (i = 0; i < COUNT_SETRESET; i++)				// Cycle through each Set/Reset
	{
		IOByteWrite (GDC_INDEX, 0x00);
		IOByteWrite (GDC_DATA, (BYTE) i);
		for (j = 0; j < COUNT_SETRESETENABLE; j++)	// Cycle through each Enable
		{
			IOByteWrite (GDC_INDEX, 0x01);
			IOByteWrite (GDC_DATA, (BYTE) j);
			MemByteRead (lpVideo);							// Load latches
			MemByteWrite (lpVideo, 0);
			lpVideo++;
		}
	}

	// Verify the data
	lpVideo = (SEGOFF) 0xA0000000;
	IOByteWrite (GDC_INDEX, 0x04);
	for (i = 0; i < COUNT_SETRESET; i++)				// Cycle through each Set/Reset
	{
		for (j = 0; j < COUNT_SETRESETENABLE; j++)	// Cycle through each enable
		{
			for (k = 0; k < 4; k++)							// Cycle through each plane
			{
				IOByteWrite (GDC_DATA, (BYTE) k);
				// Calculate enable
				byExpected = (j & (0x01 << k)) == 0 ? 0x00 : 0xFF;
				// Calculate memory byte
				byExpected = ((i & (0x01 << k)) == 0 ? 0x00 : 0xFF) & byExpected;
				if (!MemByteTest (lpVideo, byExpected, &byActual))
				{
					nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST,
											LOWORD (lpVideo), HIWORD (lpVideo),
											byExpected, byActual);
					goto SetResetTest_exit;
				}
			}
			lpVideo++;
		}
	}

SetResetTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

int LargeCharTest (void);
typedef struct tagLARGECHAR {
	BYTE		chr;				// ASCII code
	BYTE		glyphL[32];		// Left font glyph
	BYTE		glyphR[32];		// Right font glyph
} LARGECHAR;
//
//		LargeCharTest - Display a large character set (16x32)
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int LargeCharTest (void)
{
	static LARGECHAR	lc[] = {
		{0x30,
			{0x00, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x3F, 0x78,
			 0xF0, 0xF0, 0xF0, 0xF0, 0xF3, 0xF3, 0xF3, 0xF3,
			 0xF0, 0xF0, 0xF0, 0xF0, 0x78, 0x3F, 0x1F, 0x07,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
			{0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xF0, 0x78,
			 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
			 0x3C, 0x3C, 0x3C, 0x3C, 0x78, 0xF0, 0xE0, 0x80,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
		},
		{0x31,
			{0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x0F, 0x1F,
			 0x3F, 0x3B, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
			 0x03, 0x03, 0x03, 0x03, 0x03, 0x07, 0x3F, 0x3F,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
			{0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xC0,
			 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
			 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE0, 0xFC, 0xFC,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
		},
		{0x32,
			{0x00, 0x00, 0x00, 0x00, 0x3F, 0x7F, 0xF0, 0xF0,
			 0xF0, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0F,
			 0x1F, 0x3E, 0x7C, 0xF8, 0xF8, 0xF8, 0xFF, 0xFF,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
			{0x00, 0x00, 0x00, 0x00, 0xF0, 0xF8, 0x3C, 0x3C,
			 0x3C, 0x3C, 0x78, 0xF0, 0xF0, 0xE0, 0xC0, 0x80,
			 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0xFC, 0xFC,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
		},
		{0x33,
			{0x00, 0x00, 0x00, 0x00, 0x3F, 0x7F, 0xFF, 0xF0,
			 0xF0, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x00,
			 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xFF, 0x7F, 0x3F,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
			{0x00, 0x00, 0x00, 0x00, 0xF8, 0xFC, 0xFE, 0x3E,
			 0x3E, 0x3E, 0x3E, 0x7C, 0xF8, 0xF8, 0xFC, 0x3E,
			 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0xFE, 0xFC, 0xF8,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
		}
	};
	BYTE		cscans[] = {
			0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
			0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
			0x1D, 0x1B, 0x19, 0x17, 0x15, 0x13, 0x11, 0x0F,
			0x0D, 0x0B, 0x09, 0x07, 0x05, 0x03, 0x01, 0x20
	};
	BYTE		cscane[] = {
			0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
			0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
			0x1D, 0x1B, 0x19, 0x17, 0x15, 0x13, 0x11, 0x0F,
			0x0D, 0x0B, 0x09, 0x07, 0x05, 0x03, 0x01, 0x1F
	};
	int		nErr, nLC, n, i, j;
	SEGOFF	lpVideo;
	BYTE		attr;
	WORD		wSimType;
	BOOL		bFullVGA;
	int		nRowCount, nColCount;
	int	REF_PART = 1;
	int	REF_TEST = 18;

	nLC = sizeof (lc) / sizeof (LARGECHAR);
	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto LargeCharTest_exit;

	if (bFullVGA)
	{
		nRowCount = 12;
		nColCount = 40;
	}
	else
	{
		SimSetFrameSize (FALSE);			// Use small frame
		nRowCount = 1;
		nColCount = 20;
	}

	SimSetState (TRUE, TRUE, FALSE);		// Don't load RAMDAC
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);

	// Set CRTC values
	IOByteWrite (MISC_OUTPUT, 0x63);		// 25 MHz clock
	IOWordWrite (SEQ_INDEX, 0x0101);		// 8-dot characters
	IOWordWrite (SEQ_INDEX, 0x0103);		// Block 0 & 1
	IOWordWrite (CRTC_CINDEX, 0x5F09);	// Character height = 32
	IOWordWrite (CRTC_CINDEX, 0x1D0A);	// Cursor scan start at 29
	IOWordWrite (CRTC_CINDEX, 0x1E0B);	// Cursor scan stop at 30
	IOWordWrite (CRTC_CINDEX, 0x0E0F);	// Cursor position
	if (bFullVGA)
		IOWordWrite (CRTC_CINDEX, 0x7F12);	// Display end after 12 rows
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x33);
	IOByteWrite (ATC_INDEX, 0x00);		// Pixel panning to 0
	IOByteWrite (ATC_INDEX, 0x32);
	IOByteWrite (ATC_INDEX, 0x07);

	lpVideo = (SEGOFF) 0xB8000000;

	// Load the glyphs
	for (i = 0; i < nLC; i++)
	{
		LoadFontGlyph (lc[i].chr, 32, 0, &lc[i].glyphL[0]);
		LoadFontGlyph (lc[i].chr, 32, 1, &lc[i].glyphR[0]);
	}

	// Fill video memory with large characters
	n = 0;
	attr = 0x0;
	for (i = 0; i < nRowCount; i++)			// Rows
	{
		for (j = 0; j < nColCount; j++)		// Columns
		{
			MemByteWrite (lpVideo, lc[n].chr);
			MemByteWrite (lpVideo+1, (BYTE) (attr | 0x08));
			MemByteWrite (lpVideo+2, lc[n].chr);
			MemByteWrite (lpVideo+3, attr);
			if (++n >= nLC) n = 0;
			lpVideo += 4;
			if ((++attr) & 0x08) attr = (attr & 0xF7) + 0x10;
		}
	}

	SimDumpMemory ("T0409.VGA");

	WaitCursorBlink (BLINK_ON);
	WaitAttrBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto LargeCharTest_exit;
		}
	}

	// Walk through each underline location and set various cursor styles
	for (i = 0; i < 32; i++)
	{
		IOByteWrite (CRTC_CINDEX, 0x14);
		IOByteWrite (CRTC_CDATA, (BYTE) i);			// Underline location
		IOByteWrite (CRTC_CINDEX, 0x0A);
		IOByteWrite (CRTC_CDATA, cscans[i]);		// Cursor scan start
		IOByteWrite (CRTC_CINDEX, 0x0B);
		IOByteWrite (CRTC_CDATA, cscane[i]);		// Cursor scan end
		WaitCursorBlink (BLINK_ON);
		WaitAttrBlink (BLINK_ON);
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto LargeCharTest_exit;
			}
		}
	}

LargeCharTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		LineCharTest - Display the character set with the line graphics character
//							bit enabled and disabled.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int LineCharTest (void)
{
	int		nErr;
	SEGOFF	lpVideo;
	int		i, j;
	BYTE		chr, temp;
	WORD		wSimType;
	BOOL		bFullVGA;
	int		nRowCount, nColCount, nNextRow;
	BYTE		byStartChar;
	int	REF_PART = 1;
	int	REF_TEST = 19;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto LineCharTest_exit;

	// If the VGA simulation library is being used to generate tests
	// on a system where full VGA access is too slow, then use a small
	// frame with only one character row.
	if (bFullVGA)
	{
		lpVideo = (SEGOFF) 0xB8000510;			// 8 rows down, 8th column
		nRowCount = 4;
		nColCount = 64;
		nNextRow = 32;
		byStartChar = 0;
	}
	else
	{
		SimSetFrameSize (FALSE);					// Use small frame
		lpVideo = (SEGOFF) 0xB8000000;			// 0 rows down, 0th column
		nRowCount = 1;
		nColCount = 40;
		nNextRow = 0;
		byStartChar = 0xC0;
	}

	SimSetState (TRUE, TRUE, FALSE);				// Don't load RAMDAC
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);

	DisableCursor ();

	// Show character set using attributes from 00h to FFh
	chr = byStartChar;
	for (i = 0; i < nRowCount; i++)
	{
		for (j = 0; j < nColCount; j++)
		{
			MemByteWrite (lpVideo++, chr);
			MemByteWrite (lpVideo++, chr);
			chr++;
		}
		lpVideo += nNextRow;
	}

	SimDumpMemory ("T0408.VGA");

	if (bFullVGA)
	{
		// Show character set using attributes from 80h to FFh, 00h to 7Fh
		chr = byStartChar;
		for (i = 0; i < nRowCount; i++)
		{
			for (j = 0; j < nColCount; j++)
			{
				MemByteWrite (lpVideo++, chr);
				MemByteWrite (lpVideo++, (BYTE) (chr ^ 0x80));
				chr++;
			}
			lpVideo += nNextRow;
		}

		// Draw a box around the character set
		lpVideo = (SEGOFF) 0xB800046E;								// 7 rows down, 7th column
		for (i = 0; i < 128; i += 2)
		{
			MemByteWrite (lpVideo + i + 2, 205);					// Double horizontal bar character
			MemByteWrite (lpVideo + i + 160*9 + 2, 205);			// Double horizontal bar character
		}
		for (i = 0; i < 8; i++)
		{
			MemByteWrite (lpVideo + 160 + 160*i, 186);			// Double vertical bar character
			MemByteWrite (lpVideo + 160 + 65*2 + 160*i, 186);	// Double vertical bar character
		}
		MemByteWrite (lpVideo, 201);									// Double upper left bar character
		MemByteWrite (lpVideo + 65*2, 187);							// Double upper right bar character
		MemByteWrite (lpVideo + 160*9, 200);						// Double lower left bar character
		MemByteWrite (lpVideo + 160*9 + 65*2, 188);				// Double upper right bar character
	}

	WaitAttrBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto LineCharTest_exit;
		}
	}

	// Turn off 9-dot expansion of line characters
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x30);
	temp = (BYTE) IOByteRead (ATC_RDATA);
	IOByteWrite (ATC_INDEX, (BYTE) (temp & 0xFB));

	WaitAttrBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

LineCharTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		Text64KTest - Setup a text mode that addresses A000h for 64K and set
//							various display start values.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Text64KTest (void)
{
	int		nErr;
	SEGOFF	lpVideo;
	int		i, j;
	BYTE		chr;
	WORD		temp;
	WORD		wSimType;
	BOOL		bFullVGA;
	int		nRowArrow, nColArrow;
	int	REF_PART = 1;
	int	REF_TEST = 27;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Text64KTest_exit;

	// If the VGA simulation library is being used to generate tests
	// on a system where full VGA access is too slow, then use a small
	// frame with only one character row.
	if (bFullVGA)
	{
		nRowArrow = 11;
		nColArrow = 39;
	}
	else
	{
		SimSetFrameSize (FALSE);					// Use small frame
		nRowArrow = 0;
		nColArrow = 19;
	}

	SimSetState (TRUE, TRUE, FALSE);				// Don't load RAMDAC
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);

	// Set video memory address to A0000h for 64K
	IOWordWrite (GDC_INDEX, 0x0206);
	lpVideo = (SEGOFF) 0xA0000000;

	// Fill video memory with the page number
	chr = '0';
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 0x1000/2; j++)
		{
			MemByteWrite (lpVideo++, chr);
			MemByteWrite (lpVideo++, 0x07);
		}
		if (++chr == ('9' + 1)) chr = 'A';
	}

	SimDumpMemory ("T0407.VGA");

	// Cycle through each page
	lpVideo = (SEGOFF) 0xA0000000;
	for (i = 0; i < 16; i++)
	{
		// Set screen start address
		temp = (0x1000/2) * i;
		IOByteWrite (CRTC_CINDEX, 0x0C);
		IOByteWrite (CRTC_CDATA, HIBYTE (temp));
		IOByteWrite (CRTC_CINDEX, 0x0D);
		IOByteWrite (CRTC_CDATA, LOBYTE (temp));

		// Set cursor start address
		temp += (80*nRowArrow + nColArrow);
		IOByteWrite (CRTC_CINDEX, 0x0E);
		IOByteWrite (CRTC_CDATA, HIBYTE (temp));
		IOByteWrite (CRTC_CINDEX, 0x0F);
		IOByteWrite (CRTC_CDATA, LOBYTE (temp));

		// Highlight the cursor by drawing an arrow to it and changing
		// the attribute to intensity.
		temp = temp << 1;
		MemByteWrite (lpVideo + temp++, 0x19);
		MemByteWrite (lpVideo + temp, 0x0F);

		// Verify the screen
		WaitCursorBlink (BLINK_ON);
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto Text64KTest_exit;
			}
		}
	}

Text64KTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//


//
//		LoadUpsideDownFont - Load an entire font upside down into a specific block
//
//		Entry:	lpFont	Address of font table
//					height	Number of bytes per character
//					start		Starting character code
//					count		Number of font images to load
//					block		Font block
//		Exit:		None
//
void LoadUpsideDownFont (LPBYTE lpFont, BYTE height, BYTE start, WORD count, BYTE block)
{
	SEGOFF	lpVideo;
	BYTE		byClear, i;

	PreFontLoad ();

	lpVideo = (SEGOFF) 0xA0000000;
	lpVideo += tblFontBlock[block] + (((int) start) * 32);
	lpFont += ((int) start) * ((int) height);
	byClear = 32 - height;

	while (count-- > 0)
	{
		for (i = 0; i < height; i++)
		{
			MemByteWrite (lpVideo++, *(lpFont + (height - i) - 1));
		}
		lpFont += height;
		for (i = 0; i < byClear; i++)
			MemByteWrite (lpVideo++, 0);
	}

	PostFontLoad ();
}

//
//		LoadSidewaysFont - Load an entire font sideways into a specific block
//
//		Entry:	lpFont	Address of font table
//					height	Number of bytes per character
//					start		Starting character code
//					count		Number of font images to load
//					block		Font block
//					bLeft		Left to right flag (TRUE = left to right, FALSE = right to left)
//		Exit:		None
//
void LoadSidewaysFont (LPBYTE lpFont, BYTE height, BYTE start, WORD count, BYTE block, BOOL bLeft)
{
	SEGOFF	lpVideo;
	BYTE		byClear, i, j, byTemp, byMask;
	BYTE		byGlyph[8];

	PreFontLoad ();

	lpVideo = (SEGOFF) 0xA0000000;
	lpVideo += tblFontBlock[block] + (((int) start) * 32);
	lpFont += ((int) start) * ((int) height);
	byClear = 32 - 8;

	while (count-- > 0)
	{
		// Build the glyph
		for (i = 0; i < 8; i++)
		{
			if (bLeft)
				byMask = 0x01 << i;
			else
				byMask = 0x80 >> i;
			byTemp = 0;
			for (j = 0; j < 8; j++)
			{
				if (bLeft)
				{
					byTemp = byTemp << 1;
					byTemp |= ((*(lpFont + j)) & byMask) >> i;
				}
				else
				{
					byTemp = byTemp >> 1;
					byTemp |= ((*(lpFont + j)) & byMask) << i;
				}
			}
			byGlyph[i] = byTemp;
		}
		// Draw the glyph
		for (i = 0; i < height; i++)
			MemByteWrite (lpVideo++, byGlyph[i]);
		for (i = 0; i < byClear; i++)
			MemByteWrite (lpVideo++, 0);
		lpFont += height;
	}

	PostFontLoad ();
}
//

//
//		Ext512CharSetTest - Display every character in the font set
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
//		Note: Internal palette is left in the "default" state to show
//				that the pixel path still uses the intensity bit as an
//				index into the palette at the same time it is used as
//				a font select.
//
int Ext512CharSetTest (void)
{
	int		nErr;
	SEGOFF	lpVideo;
	int		i, j;
	BYTE		chr, attr;
	WORD		wSimType;
	BOOL		bFullVGA;
	int		nRowCount, nColCount, nNextRow, nNextBlock;
	int	REF_PART = 1;
	int	REF_TEST = 31;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Ext512CharSetTest_exit;

	// If the VGA simulation library is being used to generate tests
	// on a system where full VGA access is too slow, then use a small
	// frame with only one character row.
	if (bFullVGA)
	{
		lpVideo = (SEGOFF) 0xB8000330;			// 5 rows down, 8th column
		nRowCount = 4;
		nColCount = 64;
		nNextRow = 32;
		nNextBlock = 160;
	}
	else
	{
		SimSetFrameSize (FALSE);					// Use small frame
		lpVideo = (SEGOFF) 0xB8000000;			// 0 rows down, 0th column
		nRowCount = 1;
		nColCount = 13;
		nNextRow = 0;
		nNextBlock = 0;
	}

	SimSetState (TRUE, TRUE, FALSE);				// Don't load RAMDAC
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);
	DisableCursor ();

	TextStringOut ("9x16 Font: (Map A)", 0x0F, 8, 4, 0);
	TextStringOut ("8x8 Font: (Map B)", 0x0F, 8, 9, 0);
	TextStringOut ("Alternating 9x16 and 8x8 Font: (Map A & B)", 0x0F, 8, 14, 0);
	TextStringOut ("Text cells are 9x16 -- Map A characters have intensity attributes set",
					0x0F, 6, 20, 0);

	// Draw characters 00h - 0FFh with attributes 00h - 0FFh (setting bit 3).
	// Map A (9x16 font) is shown.
	chr = 0;
	for (i = 0; i < nRowCount; i++)
	{
		for (j = 0; j < nColCount; j++)
		{
			MemByteWrite (lpVideo++, chr);
			MemByteWrite (lpVideo++, (BYTE) (chr | 0x08));
			chr++;
		}
		lpVideo += nNextRow;
	}

	SimDumpMemory ("T0405.VGA");

	// Draw characters 00h - 0FFh with attributes 00h - 0FFh (clearing bit 3)
	// Map B (8x8 font) is shown.
	lpVideo += nNextBlock;						// Put a space between blocks of characters
	chr = 0;
	for (i = 0; i < nRowCount; i++)
	{
		for (j = 0; j < nColCount; j++)
		{
			MemByteWrite (lpVideo++, chr);
			MemByteWrite (lpVideo++, (BYTE) (chr & 0xF7));
			chr++;
		}
		lpVideo += nNextRow;
	}

	// Draw characters 00h - 0FFh with attributes 00h - 0FFh (alternating
	// setting and clearing bit 3). This will force every other character
	// to lookup a different font.
	lpVideo += nNextBlock;						// Put a space between blocks of characters
	chr = 0;
	attr = 0x08;
	for (i = 0; i < nRowCount; i++)
	{
		for (j = 0; j < nColCount; j++)
		{
			MemByteWrite (lpVideo++, chr);
			MemByteWrite (lpVideo++, (BYTE) ((chr & 0xF7) | attr));
			chr++;
			attr ^= 0x08;		// If clear, set. If set, clear.
		}
		lpVideo += nNextRow;
	}

	// Disable blinking
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x30);
	IOByteWrite (ATC_INDEX, (BYTE) (IOByteRead (ATC_RDATA) & 0xF7));

	// Load 8x8 font into block 1 and set font blocks 0 & 1. Therefore,
	// map A has the 9x16 font (intensity bit = 1), and map B has the
	// 8x8 font (intensity bit = 0).
	Load8x8 (1);
	IOWordWrite (SEQ_INDEX, 0x0103);

	WaitAttrBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}
	else
	{
		WaitAttrBlink (BLINK_OFF);
		FrameCapture (REF_PART, REF_TEST);
	}

Ext512CharSetTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		Std256CharSetTest - Display every character in the font set
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Std256CharSetTest (void)
{
	int		nErr;
	SEGOFF	lpVideo;
	int		i, j;
	BYTE		chr;
	WORD		wSimType;
	BOOL		bFullVGA;
	int		nRowCount, nColCount, nNextRow;
	int	REF_PART = 1;
	int	REF_TEST = 32;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Std256CharSetTest_exit;

	// If the VGA simulation library is being used to generate tests
	// on a system where full VGA access is too slow, then use a small
	// frame with only one character row.
	if (bFullVGA)
	{
		lpVideo = (SEGOFF) 0xB8000650;			// 10 rows down, 8th column
		nRowCount = 4;
		nColCount = 64;
		nNextRow = 32;
	}
	else
	{
		SimSetFrameSize (FALSE);					// Use small frame
		lpVideo = (SEGOFF) 0xB8000000;			// 0 rows down, 0th column
		nRowCount = 1;
		nColCount = 40;
		nNextRow = 0;
	}

	SimSetState (TRUE, TRUE, FALSE);				// Don't load RAMDAC
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);

	DisableCursor ();
	chr = 0;
	for (i = 0; i < nRowCount; i++)
	{
		for (j = 0; j < nColCount; j++)
		{
			MemByteWrite (lpVideo++, chr);
			MemByteWrite (lpVideo++, (BYTE) (255 - chr));
			chr++;
		}
		lpVideo += nNextRow;
	}
	
	SimDumpMemory ("T0404.VGA");

	if (bFullVGA)
		TextStringOut ("9x16 Font:", 0x0F, 8, 9, 0);

	WaitAttrBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}
	else
	{
		WaitAttrBlink (BLINK_OFF);
		FrameCapture (REF_PART, REF_TEST);
	}

Std256CharSetTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
int WriteMapReadPlaneTest (void);
typedef struct tagWRITETABLE {
	WORD		offset;
	BYTE		mask;
	BYTE		data;
} WRITETABLE;
//
//		WriteMapReadPlaneTest - Write various values at various memory locations
//										and compare the read value
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int WriteMapReadPlaneTest (void)
{
	static WRITETABLE	wrt[] = {
		{0x0000, 0x0F, 0xFF},
		{0x0002, 0x0A, 0x55},
		{0x0004, 0x05, 0xAA},
		{0x0008, 0x01, 0x5A},
		{0x0010, 0x02, 0xA5},
		{0x0020, 0x04, 0x99},
		{0x0040, 0x08, 0x66},
		{0x0080, 0x03, 0x69},
		{0x0100, 0x05, 0x96},
		{0x0200, 0x09, 0x01},
		{0x0400, 0x06, 0x02},
		{0x0800, 0x07, 0x04},
		{0x1000, 0x08, 0x08},
		{0x2000, 0x0B, 0x10},
		{0x4000, 0x0C, 0x20},
		{0x8000, 0x0D, 0x40},
		{0xC000, 0x0E, 0x80},
		{0xF000, 0x00, 0xFF},
		{0xFFFF, 0x0F, 0xFF},
	};
	int		nErr, i, n;
	WORD		wSimType;
	SEGOFF	lpVideo;
	BYTE		temp0, temp1;
	BOOL		bFullVGA;

 	int	REF_PART = 99;
  	int	REF_TEST = 99;
	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

	SimSetState (TRUE, TRUE, FALSE);				// Don't load RAMDAC
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);
	SimSetCaptureMode (CAP_COMPOSITE);

	lpVideo = (SEGOFF) 0xA0000000;

	// Load video memory
	IOByteWrite (SEQ_INDEX, 0x02);

	for (i = 0; i < (sizeof (wrt) / sizeof (WRITETABLE)); i++)
	{
		IOByteWrite (SEQ_DATA, wrt[i].mask);
		MemByteWrite (lpVideo + wrt[i].offset, wrt[i].data);
	}
	SimDumpMemory ("T0403.VGA");

	// Verify video memory
	for (n = 0; n < 4; n++)
	{
		lpVideo = (SEGOFF) 0xA0000000;
		IOByteWrite (GDC_INDEX, 0x04);
		IOByteWrite (GDC_DATA, (BYTE) n);
		for (i = 0; i < (sizeof (wrt) / sizeof (WRITETABLE)); i++)
		{
			while (LOWORD (lpVideo) < wrt[i].offset)
			{
				temp0 = MemByteRead (lpVideo++);
				if (temp0)				// Video data should be a 0
				{
					nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, LOWORD (lpVideo), HIWORD (lpVideo), 0, temp0);
					goto WriteMapReadPlaneTest_exit;
				}
			}
			temp0 = MemByteRead (lpVideo++);
			temp1 = wrt[i].data;
			if ((wrt[i].mask & (1 << n)) == 0) temp1 = 0;
			if (temp0 != temp1)
			{
				nErr = FlagError (ERROR_MEMORY, REF_PART, REF_TEST, LOWORD (lpVideo), HIWORD (lpVideo), temp1, temp0);
				goto WriteMapReadPlaneTest_exit;
			}
		}
	}

WriteMapReadPlaneTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		CPUMaxBandwidthTest - Time writes to video memory with CRTC enabled and
//										with CRTC disabled.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int CPUMaxBandwidthTest (void)
{
	int		nErr, nMemSize, i;
	SEGOFF	lpVideo;
	LPBYTE	lpBuffer;
	DWORD		actualtime, time0, time1, counter0, counter1;
	DWORD		timetest0, timetest1;
	WORD		wSimType;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	lpBuffer = NULL;				// Memory is not yet allocated

	// This test can not be simulated. 
	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}

	SetMode (0x03);
	TextStringOut ("The following test is self-running and will take about 10 seconds.", ATTR_NORMAL, 1, 1, 0);
	TextStringOut ("Press any key to continue, <ESC> to exit.", ATTR_NORMAL, 1, 3, 0);
	if (GetKey () == KEY_ESCAPE)
		goto CPUMaxBandwidthTest_exit;
	SetMode (0x03);

	// Allocate a buffer and fill it with the full block character
	nMemSize = 0x8000;
	lpVideo = (SEGOFF) 0xB8000000;
#ifdef __MSVC16__
	lpBuffer = (LPBYTE) _fmalloc (nMemSize);
#else
	lpBuffer = (LPBYTE) malloc (nMemSize);
#endif
	if (lpBuffer == NULL)
	{
		nErr = FlagError (ERROR_INTERNAL, REF_PART, REF_TEST, 0, 0, 0, 0);
		goto CPUMaxBandwidthTest_exit;
	}
#ifdef __MSVC16__
	_fmemset (lpBuffer, 0xDB, nMemSize);
#else
	memset (lpBuffer, 0xDB, nMemSize);
#endif

	// Count the number of bytes that can be written in a given period of time
	actualtime = time0 = GetSystemTicks ();
	time1 = time0 + FIVE_SECONDS;
	counter0 = 0;
	while (actualtime < time1)
	{
		for (i = 0; i < nMemSize; i++)
			*(LPBYTE) (lpVideo + i) = *(lpBuffer + i);
//			MemByteWrite (lpVideo + i, *(lpBuffer + i));
		actualtime = GetSystemTicks ();
		counter0++;
	}
	timetest0 = actualtime - time0;

	WaitAttrBlink (BLINK_ON);
	FrameCapture (REF_PART, REF_TEST);

	// Give the CPU maximum bandwidth
	IOByteWrite (SEQ_INDEX, 0x01);
	IOByteWrite (SEQ_DATA, (BYTE) (IOByteRead (SEQ_DATA) | 0x20));

	actualtime = time0 = GetSystemTicks ();
	time1 = time0 + FIVE_SECONDS;
	counter1 = 0;
	while (actualtime < time1)
	{
		for (i = 0; i < nMemSize; i++)
			*(LPBYTE) (lpVideo + i) = *(lpBuffer + i);
//			MemByteWrite (lpVideo + i, *(lpBuffer + i));
		actualtime = GetSystemTicks ();
		counter1++;
	}

	timetest1 = actualtime - time0;

	// There should be more updates with the screen disabled
	if (counter1 < counter0)
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);

	WaitAttrBlink (BLINK_ON);
	FrameCapture (REF_PART, REF_TEST);

CPUMaxBandwidthTest_exit:
#ifdef __MSVC16__
	if (lpBuffer) _ffree (lpBuffer);
#else
	if (lpBuffer) free (lpBuffer);
#endif
	SystemCleanUp ();
	return (nErr);
}
//
//
//		SyncResetTest - Set sync reset and verify system behavior (no retrace,
//								SEQ[3] cleared).
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int SyncResetTest (void)
{
	int	nErr;
	BYTE	temp;
	WORD	wSimType;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;

	// This test can not be simulated. 
	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}

	SetMode (0x03);
	TextStringOut ("The following test is self-running and will take about 5 seconds.", ATTR_NORMAL, 1, 1, 0);
	TextStringOut ("Press any key to continue, <ESC> to exit.", ATTR_NORMAL, 1, 3, 0);
	if (SimGetKey () == KEY_ESCAPE)
		goto SyncResetTest_exit;

	SimSetState (TRUE, TRUE, FALSE);				// Don't load RAMDAC
	SimSetFrameSize (FALSE);						// Use small frame
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);
	SimSetCaptureMode (CAP_COMPOSITE);

	// There should be retrace happening
	if (!WaitVerticalRetrace ())
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
				LOWORD (70000l), HIWORD (70000l), 0, 0);
		goto SyncResetTest_exit;
	}

	SimDumpMemory ("T0401.VGA");

	IOWordWrite (SEQ_INDEX, 0x3F03);		// Set SEQ[3] to a non-zero value
	IOWordWrite (SEQ_INDEX, 0x0100);		// Sync reset
	if (WaitVerticalRetrace ())			// Should be no sync pulses
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
				0, 0, LOWORD (70000l), HIWORD (70000l));
		goto SyncResetTest_exit;
	}
	IOByteWrite (SEQ_INDEX, 0x03);
	temp = (BYTE) IOByteRead (SEQ_DATA);
	if (temp != 0x3F)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, SEQ_INDEX, 0x03, 0x3F, temp);
		goto SyncResetTest_exit;
	}

	IOWordWrite (SEQ_INDEX, 0x0000);		// Async reset
	if (WaitVerticalRetrace ())			// Should still be no sync pulses
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
				0, 0, LOWORD (70000l), HIWORD (70000l));
		goto SyncResetTest_exit;
	}
	IOByteWrite (SEQ_INDEX, 0x03);
	temp = (BYTE) IOByteRead (SEQ_DATA);
	if (temp != 0x00)							// Should reset SEQ[3]
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, SEQ_INDEX, 0x03, 0x00, temp);
		goto SyncResetTest_exit;
	}

	IOWordWrite (SEQ_INDEX, 0x0300);		// End sync reset
	if (!WaitVerticalRetrace ())			// Should get sync pulses back
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
				0, 0, LOWORD (70000l), HIWORD (70000l));
		goto SyncResetTest_exit;
	}

SyncResetTest_exit:
	IOWordWrite (SEQ_INDEX, 0x0300);		// End sync reset
	SystemCleanUp ();
	return (nErr);
}
//

//
//		NonBIOSLineCompareTest - Pan a split-screen using the Renaissance CRTC values
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int NonBIOSLineCompareTest (void)
{
	static PARMENTRY	parmTest = {
		0x50, 0x1D, 0x10,
		0x00, 0xA0,
		{0x11, 0x0F, 0x00, 0x0E},
		0xE3,
		{0x5F, 0x4F, 0x53, 0xE1, 0x58, 0x00,
		0x0B, 0x2E, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xFF, 0x00, 0xEA, 0x8C,
		0xDF, 0x0A, 0x7F, 0xE7, 0x04, 0xE3,
		0xC8},
		{0x01, 0x3F, 0x00, 0x3F, 0x00, 0x3F,
		0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F,
		0x00, 0x3F, 0x00, 0x3F, 0x21, 0x00,
		0x01, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x05, 0x0F, 0xFF}
	};
	static PARMENTRY	parmSmallTest = {
		0x50, 0x1D, 0x10,
		0x00, 0xA0,
		{0x11, 0x0F, 0x00, 0x0E},
		0xE3,
		{0x2D, 0x27, 0x28, 0xF0, 0x2B, 0xE0,
		0x17, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xFF, 0x00, 0x15, 0x06,
		0x13, 0x05, 0x7F, 0x14, 0x17, 0xE3,
		0x0A},
		{0x01, 0x3F, 0x00, 0x3F, 0x00, 0x3F,
		0x00, 0x3F, 0x00, 0x3F, 0x00, 0x3F,
		0x00, 0x3F, 0x00, 0x3F, 0x21, 0x00,
		0x01, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x05, 0x0F, 0xFF}
	};
	int		nErr, i, j, nPans, nRowCount, nColCount;
	SEGOFF	lpVideo;
	BYTE		chr;
	WORD		wSimType;
	BOOL		bFullVGA, bCapture;

 	int	REF_PART = 99;
  	int	REF_TEST = 99;
	nErr = ERROR_NONE;
	lpVideo = (SEGOFF) 0xA0000000;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto NonBIOSLineCompareTest_exit;

	SimSetState (TRUE, TRUE, FALSE);				// Ignore DAC writes
	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);

	// Load the registers
	if (bFullVGA)
	{
		SetRegs (&parmTest);
		nRowCount = 30;
		nColCount = 80;
	}
	else
	{
		SetRegs (&parmSmallTest);
		nRowCount = 1;
		nColCount = 40;
	}

	chr = 1;
	for (i = 0; i < nRowCount; i++)
	{
		for (j = 0; j < nColCount; j++)
			DrawMonoChar (j, i, chr + j, tblFont8x16, 16, nColCount);
		chr++;
	}

	SimDumpMemory ("T0322.VGA");

	StartCapture (1);
	bCapture = FrameCapture (REF_PART, REF_TEST);
	if (!bCapture)
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto NonBIOSLineCompareTest_exit;
		}
	}

	// Now pan the beast
	nPans = 320;
	if (bCapture) nPans = 34;
	for (i = 0; i < nPans; i++)
	{
		WaitNotVerticalRetrace ();
		if (!FrameCapture (REF_PART, REF_TEST))
		{
		   // dont wait for kbb ... 
		   //	if (_kbhit ())
		   //	{
				if (GetKey () == KEY_ESCAPE)
				{
					nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
					goto NonBIOSLineCompareTest_exit;
				}
		   //	}
		}
		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x33);
		IOByteWrite (ATC_INDEX, (BYTE) (i & 0x7));					// Pixel panning
		IOByteWrite (CRTC_CINDEX, 0x08);
		IOByteWrite (CRTC_CDATA, (BYTE) ((i & 0x18) << 2));		// Byte panning
		IOByteWrite (CRTC_CINDEX, 0x0C);
		IOByteWrite (CRTC_CDATA, (BYTE) (i >> 13));					// Display start high
		IOByteWrite (CRTC_CINDEX, 0x0D);
		IOByteWrite (CRTC_CDATA, (BYTE) ((i >> 5) & 0xFF));		// Display start low
	}

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

NonBIOSLineCompareTest_exit:
	EndCapture ();
	SystemCleanUp ();
	return (nErr);
}
//
//
//		CursorSkewTest - In mode 03h, set each of the four cursor skew positions
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int CursorSkewTest (void)
{
	int	nErr, i;
	BYTE	tblSkew[] = {0x0E, 0x2E, 0x4E, 0x6E};
	WORD	wSimType;
	BOOL	bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 37;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto CursorSkewTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);		// Use small frame
	SimSetState (TRUE, TRUE, FALSE);
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);
	TextStringOut ("0123", 0x07, 0, 0, 0);

	// Cursor at position 0
	IOByteWrite (CRTC_CINDEX, 0x0E);
	IOByteWrite (CRTC_CDATA, 0x00);
	IOByteWrite (CRTC_CINDEX, 0x0F);
	IOByteWrite (CRTC_CDATA, 0x00);

	SimDumpMemory ("T0321.VGA");
	SimSetCaptureMode (CAP_COMPOSITE);

	// Verify screen is as expected (0 skew)
	WaitCursorBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CursorSkewTest_exit;
		}
	}

	for (i = 0; i < sizeof (tblSkew); i++)
	{
		IOByteWrite (CRTC_CINDEX, 0x0B);
		IOByteWrite (CRTC_CDATA, tblSkew[i]);

		// Verify screen is as expected (cursor skewed "i" times)
		WaitCursorBlink (BLINK_ON);
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto CursorSkewTest_exit;
			}
		}
	}

CursorSkewTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		PresetRowScanTest - In mode 3, smooth scroll a screen.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int PresetRowScanTest (void)
{
	BYTE		byTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	int		nErr, i, j, n, nFrames;
	SEGOFF	lpVideo;
	WORD		offset, wSimType;
	BOOL		bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto PresetRowScanTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);

	lpVideo = (SEGOFF) 0xB8000000;

	// Fill memory with a known pattern
	n = 0;
	for (i = 0; i < 70; i++)				// Enough to fill 70*16 scan lines
	{
		for (j = 0; j < 80; j++)
		{
			MemByteWrite (lpVideo++, byTable[n]);
			MemByteWrite (lpVideo++, 0x07);
		}
		if (++n >= sizeof (byTable)) n = 0;
	}
	SimDumpMemory ("T0320.VGA");

	// Capture the initial frame and set the scroll variable based on
	// whether this is a visual or an automated test. For the visual
	// test, 700 + 400 (original screen) scans eventually make their
	// appearance on screen.
	if ((wSimType & SIM_TURBOACCESS) || (wSimType & SIM_SIMULATION))
		nFrames = 34;
	else
		nFrames = 700;

	// The preset row seems to get loaded immediately, while the row address
	// takes effect on the next frame. Therefore, change the row address first,
	// then wait for the end of the frame before changing the preset row address.
	StartCapture (1);
	FrameCapture (REF_PART, REF_TEST);
	for (i = 0; i < nFrames; i++)
	{
		offset = (i / 16) * 80;
		if (!FrameCapture (REF_PART, REF_TEST))
		{
		   // stub out for diags
		   //	if (_kbhit ())
		  //	{
				if (GetKey () == KEY_ESCAPE)
				{
					nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
					goto PresetRowScanTest_exit;
				}
		 //	}
			WaitNotVerticalRetrace ();
		}

		IOByteWrite (CRTC_CINDEX, 0x0C);
		IOByteWrite (CRTC_CDATA, HIBYTE (offset));
		IOByteWrite (CRTC_CINDEX, 0x0D);
		IOByteWrite (CRTC_CDATA, LOBYTE (offset));
		WaitVerticalRetrace ();
		IOByteWrite (CRTC_CINDEX, 0x08);
		IOByteWrite (CRTC_CDATA, (BYTE) (i % 16));
	}

PresetRowScanTest_exit:
	EndCapture ();
	SystemCleanUp ();
	return (nErr);
}
//

//
//		SkewTest - In mode 12h, set a known pattern and visually inspect
//					  the sync skew and display skew.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int SkewTest (void)
{
	int	nErr, i;
	WORD	wXRes, wYRes;
	WORD	wSimType;
	BOOL	bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 38;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto SkewTest_exit;

	// Set 640x480 graphics, draw a rectangle around the screen, and
	// draw two diagonal lines across the center.
	if (!bFullVGA) SimSetFrameSize (FALSE);			// Use small frame
	SimSetState (TRUE, TRUE, FALSE);
	SetMode (0x12);

	SimSetCaptureMode (CAP_COMPOSITE);
	GetResolution (&wXRes, &wYRes);
	SimSetState (TRUE, TRUE, TRUE);

	SetLine4Columns (wXRes / 8);
	Line4 (0, 0, wXRes - 1, 0, 0x0F);
	Line4 (wXRes -1, 0, wXRes - 1, wYRes - 1, 0x0F);
	Line4 (0, wYRes -1, wXRes - 1, wYRes -1, 0x0F);
	Line4 (0, 0, 0, wYRes, 0x0F);
	Line4 (0, 0, wXRes - 1, wYRes - 1, 0x0F);
	Line4 (0, wYRes - 1, wXRes - 1, 0, 0x0F);

	SimDumpMemory ("T0319.VGA");

	// Verify screen is as expected
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto SkewTest_exit;
		}
	}

	// Unlock CRTC
	IOByteWrite (CRTC_CINDEX, 0x11);
	IOByteWrite (CRTC_CDATA, (BYTE) (IOByteRead (CRTC_CDATA) & 0x7F));

	for (i = 0; i < 4; i++)
	{
		IOByteWrite (CRTC_CINDEX, 0x03);
		IOByteWrite (CRTC_CDATA, IOByteRead (CRTC_CDATA) & 0x9F);
		IOByteWrite (CRTC_CINDEX, 0x05);
		IOByteWrite (CRTC_CDATA, (BYTE)((IOByteRead (CRTC_CDATA) & 0x9F) | (i << 5)));

		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto SkewTest_exit;
			}
		}
	}
	for (i = 0; i < 4; i++)
	{
		IOByteWrite (CRTC_CINDEX, 0x03);
		IOByteWrite (CRTC_CDATA, (BYTE)((IOByteRead (CRTC_CDATA) & 0x9F) | (i << 5)));
		IOByteWrite (CRTC_CINDEX, 0x05);
		IOByteWrite (CRTC_CDATA, IOByteRead (CRTC_CDATA) & 0x9F);

		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto SkewTest_exit;
			}
		}
	}

	for (i = 0; i < 4; i++)
	{
		IOByteWrite (CRTC_CINDEX, 0x03);
		IOByteWrite (CRTC_CDATA, (BYTE)((IOByteRead (CRTC_CDATA) & 0x9F) | (i << 5)));
		IOByteWrite (CRTC_CINDEX, 0x05);
		IOByteWrite (CRTC_CDATA, (BYTE)((IOByteRead (CRTC_CDATA) & 0x9F) | (i << 5)));

		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto SkewTest_exit;
			}
		}
	}

SkewTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		CountBy4Test - In mode 12h, set a known pattern and visually inspect
//							the count by 2, count by 4 functionality.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int CountBy4Test (void)
{
	int			nErr, i, j;
	SEGOFF		lpVideo;
	WORD			offset, wXRes, wYRes, wRowOffset;
	WORD			wSimType;
	BOOL			bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 35;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto CountBy4Test_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// No RAMDAC writes
	SetMode (0x12);
	GetResolution (&wXRes, &wYRes);
	lpVideo = (SEGOFF) 0xA0000000;
	SimSetState (TRUE, TRUE, TRUE);

	wRowOffset = wXRes / 8;
	for (i = 0; i < (int) wRowOffset; i++)
	{
		IOByteWrite (SEQ_INDEX, 0x02);
		IOByteWrite (SEQ_DATA, (BYTE) (16 - (i % 16)));
		offset = i;
		for (j = 0; j < (int) wYRes; j++)
		{
			MemByteWrite (lpVideo + offset, 0xAA);
			offset += wRowOffset;
		}
	}
	SimDumpMemory ("T0318.VGA");

	// Verify "normal" screen
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CountBy4Test_exit;
		}
	}

	// Set count by 2
	IOWordWrite (CRTC_CINDEX, 0xAB17);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CountBy4Test_exit;
		}
	}

	// Set count by 2 and WORD mode
	IOWordWrite (CRTC_CINDEX, 0xEB17);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CountBy4Test_exit;
		}
	}

	// Set count by 4, BYTE mode
	IOWordWrite (CRTC_CINDEX, 0xA317);
	IOWordWrite (CRTC_CINDEX, 0x3F14);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CountBy4Test_exit;
		}
	}

	// Set WORD mode
	IOWordWrite (CRTC_CINDEX, 0xE317);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CountBy4Test_exit;
		}
	}

	// Set count by 2 mode while in count by 4 (verify count by 2 has
	// priority over count by 4
	IOWordWrite (CRTC_CINDEX, 0xAB17);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

CountBy4Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//
typedef struct tagDACTABLE {
	BYTE	idx;
	BYTE	red;
	BYTE	green;
	BYTE	blue;
} DACTABLE;

//
//		Panning256Test - In mode 13h, pan by half characters
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Panning256Test (void)
{
	static DACTABLE	dac[] = {
		{0xF0, 0x3F, 0x3F, 0x00},	// This entry must match last two
		{0xF1, 0x00, 0x00, 0x2A},
		{0xF2, 0x00, 0x2A, 0x00},
		{0xF3, 0x00, 0x2A, 0x2A},
		{0xF4, 0x2A, 0x00, 0x00},
		{0xF5, 0x2A, 0x00, 0x2A},
		{0xF6, 0x2A, 0x2A, 0x00},
		{0xF7, 0x2A, 0x2A, 0x2A},
		{0xF8, 0x15, 0x15, 0x15},
		{0xF9, 0x00, 0x00, 0x3F},
		{0xFA, 0x00, 0x3F, 0x00},
		{0xFB, 0x00, 0x3F, 0x3F},
		{0xFC, 0x3F, 0x00, 0x00},
		{0xFD, 0x3F, 0x00, 0x3F},
		{0xFE, 0x3F, 0x3F, 0x00},	// This entry will be used by previous half-pixel (DFh)
		{0xFF, 0x3F, 0x3F, 0x00},	// This entry must match previous and first entries
		{0xF0, 0x3F, 0x3F, 0x00}	// Repeat first entry
	};
	int		nErr;
	SEGOFF	lpVideo;
	int		i, j;
	WORD		wXRes, wYRes;
	WORD		wSimType;
	BOOL		bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 36;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto Panning256Test_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// No RAMDAC writes
	SetMode (0x13);
	SimSetState (TRUE, TRUE, TRUE);				// Allow RAMDAC writes
	GetResolution (&wXRes, &wYRes);
	lpVideo = (SEGOFF) 0xA0000000;

	// Screw up the internal palette to show that the pixel pipeline goes
	// through the internal palette in alternating nibbles. The pixel is
	// actually constructed on the other side of the internal palette.
	// Turn all "E" nibbles into "F" nibbles:
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x0E);
	IOByteWrite (ATC_INDEX, 0x0F);
	IOByteWrite (ATC_INDEX, 0x20);

	// There should be no bright white pixels in the picture
	FillDAC (0x3F, 0x3F, 0x3F);
	SetDac (0, 0, 0, 0);					// So overscan will be 0

	// Set DAC to match altered pixels
	for (i = 0, j = 1; i < 16; i++, j++)
	{
		SetDac (dac[i].idx, dac[i].red, dac[i].green, dac[i].blue);
		SetDac ((BYTE) (((dac[i].idx & 0x0F) << 4) | (dac[j].idx >> 4)),
					dac[j].red, dac[j].green, dac[j].blue);
	}

	// Fill memory with a repeating pattern using pixel data that will
	// reveal the pipeline through the attribute controller. Also. fill
	// one extra scan line because the panning will scroll data off the
	// left and the last scan line needs to pull data from the "next"
	// scan line.
	for (i = 0; i < (int) (wYRes + 1); i++)
	{
		for (j = 0; j < (int) wXRes; j++)
		{
			MemByteWrite (lpVideo++, (BYTE) ((j & 0x0F) | 0xE0));
		}
	}
	SimDumpMemory ("T0317.VGA");

	// Pan from left to right setting pixel panning from 0 through 7,
	// byte panning from 0 through 3, and incrementing start address
	// by 4 when necessary. Note that an entire frame is generated
	// before the pixel panning, byte panning, and start address
	// values take effect due to the wait for "not" vertical retrace.
	StartCapture (1);
	FrameCapture (REF_PART, REF_TEST);
	for (i = 1; i < 33; i++)
	{
		if (!FrameCapture (REF_PART, REF_TEST))
		{
		   // stub out for diags
		   //	if (_kbhit ())
				if (GetKey () == KEY_ESCAPE) goto Panning256Test_exit;
			WaitNotVerticalRetrace ();
		}

		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x33);
		IOByteWrite (ATC_INDEX, (BYTE) (i & 0x07));
		IOByteWrite (CRTC_CINDEX, 0x08);
		IOByteWrite (CRTC_CDATA, (BYTE) (((i >> 3) & 0x03) << 5));
		IOByteWrite (CRTC_CINDEX, 0x0D);
		IOByteWrite (CRTC_CDATA, (BYTE) (((i >> 3) >> 2) << 2));
	}

	// Pan from right to left setting pixel panning from 7 through 0,
	// byte panning from 1 through 0, and decrementing start address
	// by 2 when necessary. Note that an entire frame is generated
	// before the pixel panning, byte panning, and start address
	// values take effect due to the wait for "not" vertical retrace.
	for (i = 32; i >= 0; i--)
	{
		if (!FrameCapture (REF_PART, REF_TEST))
		{
		   // stub out for diags
		   //	if (_kbhit ())
				if (GetKey () == KEY_ESCAPE) goto Panning256Test_exit;
			WaitNotVerticalRetrace ();
		}

		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x33);
		IOByteWrite (ATC_INDEX, (BYTE) (i & 0x07));
		IOByteWrite (CRTC_CINDEX, 0x08);
		IOByteWrite (CRTC_CDATA, (BYTE) (((i >> 3) & 0x01) << 5));
		IOByteWrite (CRTC_CINDEX, 0x0D);
		IOByteWrite (CRTC_CDATA, (BYTE) (((i >> 3) >> 1) << 1));
	}

	if (!FrameCapture (REF_PART, REF_TEST))
		GetKey ();

Panning256Test_exit:
	EndCapture ();
	SystemCleanUp ();
	return (nErr);
}
//
//
//		LineCompareTest - Smooth scroll a split screen window to the top of the display
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int LineCompareTest (void)
{
	static BYTE	pattern0[] = {
		'0', 0x0F, '1', 0x0F, '2', 0x0F, '3', 0x0F, '4', 0x0F,
		'5', 0x0F, '6', 0x0F, '7', 0x0F, '8', 0x0F, '9', 0x0F
	};
	static BYTE	pattern1[] = {
		'A', 0x70, 'B', 0x70, 'C', 0x70, 'D', 0x70, 'E', 0x70,
		'F', 0x70, 'G', 0x70, 'H', 0x70, 'I', 0x70, 'J', 0x70,
		'K', 0x70, 'L', 0x70, 'M', 0x70, 'N', 0x70, 'O', 0x70,
		'P', 0x70, 'Q', 0x70, 'R', 0x70, 'S', 0x70, 'T', 0x70,
		'U', 0x70, 'V', 0x70, 'W', 0x70, 'X', 0x70, 'Y', 0x70,
		'Z', 0x70
	};
	int			nErr, i, n, nMemSize, nRows, nColumns, nRowCount;
	SEGOFF		lpVideo;
	BYTE			temp, byPan;
	WORD			wSimType, wXRes, wYRes;
	BOOL			bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto LineCompareTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);		// Use small frame
	SimSetState (TRUE, TRUE, FALSE);					// No RAMDAC writes
	SetMode (0x03);
	GetResolution (&wXRes, &wYRes);
	nRows = wYRes / 16;
	nColumns = wXRes / 9;
	SimSetState (TRUE, TRUE, TRUE);

	// Fill page 0 memory with a pattern
	lpVideo = (SEGOFF) 0xB8000000;
	nMemSize = nColumns*nRows*2;
	i = n = 0;
	while (n <= nMemSize)
	{
		MemByteWrite (lpVideo++, pattern0[i]);
		if (++i >= sizeof (pattern0)) i = 0;
		n++;
	}

	// Fill page 1 memory with a pattern
	lpVideo = (SEGOFF) 0xB8001000;
	nMemSize = nColumns*nRows*2;
	i = n = 0;
	while (n <= nMemSize)
	{
		MemByteWrite (lpVideo++, pattern1[i]);
		if (++i >= sizeof (pattern1)) i = 0;
		n++;
	}

	SimDumpMemory ("T0316.VGA");

	// Disable pixel panning for bottom half of screen
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x30);
	IOByteWrite (ATC_INDEX, (BYTE) (IOByteRead (ATC_RDATA) | 0x20));

	// Set display start to 1000h
	IOWordWrite (CRTC_CINDEX, 0x080C);
	IOWordWrite (CRTC_CINDEX, 0x000D);

	// Set preset row scan to 7 to show that the internal row scan counter
	// is NOT set to the preset row scan count at line compare. It appears
	// that all relevant internal counters are reset to "0" at line compare.
	IOWordWrite (CRTC_CINDEX, 0x0708);

	// Capture the initial frame and set the number of rows based on
	// whether this is a visual test or whether it is an automated test.

   // stub out for diags
   //	if ((wSimType & SIM_TURBOACCESS) || (wSimType & SIM_SIMULATION))
   //		nRowCount = __min (wYRes - 1, 34);
   //	else
		nRowCount = 200;

	byPan = 0;
	StartCapture (2);
	FrameCapture (REF_PART, REF_TEST);
	for (i = nRowCount, n = 0; i >= 0; i--, n++)
	{
		// Do comparison one scan line before the end.
		//	Standard VGA is off by one.
		if (!FrameCapture (REF_PART, REF_TEST))
		{

			if (i == 0)
			{
				if (GetKey () == KEY_ESCAPE)
				{
					nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
					goto LineCompareTest_exit;
				}
			}
			else
			{
			   // stub out for diags
			   //	if (_kbhit ())
			   //	{
					if (GetKey () == KEY_ESCAPE)
					{
						nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
						goto LineCompareTest_exit;
					}
			   //	}
			}
		}

 		WaitNotVerticalRetrace ();

		// CRTC[9].6 is line compare bit 9
		IOByteWrite (CRTC_CINDEX, 0x09);
		temp = (BYTE) (IOByteRead (CRTC_CDATA) & 0xBF);
		IOByteWrite (CRTC_CDATA, (BYTE) (temp | ((i & 0x200) >> 3)));

		// CRTC[7].4 is line compare bit 8
		IOByteWrite (CRTC_CINDEX, 0x07);
		temp = (BYTE) (IOByteRead (CRTC_CDATA) & 0xEF);
		IOByteWrite (CRTC_CDATA, (BYTE) (temp | ((i & 0x100) >> 4)));

		// CRTC[18].0..7 is line compare bits 0-7
		IOByteWrite (CRTC_CINDEX, 0x18);
		IOByteWrite (CRTC_CDATA, (BYTE) (i & 0xFF));

		IOByteWrite (ATC_INDEX, 0x33);
		temp = (BYTE) IOByteRead (ATC_RDATA);
		temp++;
		if (temp == 8)
		{
			byPan++;
			if (byPan > 3)
			{
				byPan = 0;
				IOByteWrite (CRTC_CINDEX, 0x0D);
				IOByteWrite (CRTC_CDATA, (BYTE) (IOByteRead (CRTC_CDATA) + 4));
			}
			IOByteWrite (CRTC_CINDEX, 0x08);
			IOByteWrite (CRTC_CDATA, (BYTE) (IOByteRead (CRTC_CDATA) & 0x9F) | (byPan << 5));
		}
		else if (temp > 8)
		{
			temp = 0;
		}
		IOByteWrite (ATC_INDEX, temp);
	}

	FrameCapture (REF_PART, REF_TEST);

	WaitVerticalRetrace ();

	FrameCapture (REF_PART, REF_TEST);

	WaitVerticalRetrace ();

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

LineCompareTest_exit:
	EndCapture ();
	SystemCleanUp ();
	return (nErr);
}
//

//
//		SyncDisableTest - Disable syncs and verify that no pulses are generated.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int SyncDisableTest (void)
{
	int	nErr;
	WORD	wSimType;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\n\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}

	SetMode (0x03);
	TextStringOut ("The following test is self-running and will take about 5 seconds.", ATTR_NORMAL, 1, 1, 0);
	TextStringOut ("Press any key to continue, <ESC> to exit.", ATTR_NORMAL, 1, 3, 0);
	if (SimGetKey () == KEY_ESCAPE)
		goto SyncDisableTest_exit;
	SetMode (0x12);

	// There should be retrace happening
	if (!WaitVerticalRetrace ())
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
				LOWORD (60000l), HIWORD (60000l), 0, 0);
		goto SyncDisableTest_exit;
	}

	IOByteWrite (CRTC_CINDEX, 0x17);
	IOByteWrite (CRTC_CDATA, (BYTE) (IOByteRead (CRTC_CDATA) & 0x7F));	// Disable syncs

	// There should be no retrace now
	if (WaitVerticalRetrace ())
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);

SyncDisableTest_exit:
	SystemCleanUp ();
	return (nErr);
}

//
//		PanningTest - Scroll an image that is larger than visible memory.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//

#define	GRMODE		0x0D
int	nArguments = 0;

int PanningTest (void)
{
	int	nErr;
	int	i;
	WORD	offset, wXRes, wYRes, wXVirtRes, wYVirtRes;
	BYTE	temp;
	WORD	wSimType;
	BOOL	bFullVGA;

 	int	REF_PART = 99;
  	int	REF_TEST = 99;
	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto PanningTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);		// Use small frame
	SimSetState (TRUE, TRUE, FALSE);					// No RAMDAC writes
	SetMode (GRMODE);
	GetResolution (&wXRes, &wYRes);
	wXVirtRes = wXRes + 48;
	wYVirtRes = wYRes + 16;
	SimSetState (TRUE, TRUE, TRUE);

	SetLine4Columns (wXVirtRes / 8);
	IOByteWrite (CRTC_CINDEX, 0x13);
	IOByteWrite (CRTC_CDATA, (BYTE) (wXVirtRes / 16));

	// Draw a pattern
	Line4 (0, 0, wXVirtRes - 1, 0, 0xF);
	Line4 (wXVirtRes - 1, 0, wXVirtRes - 1, wYVirtRes - 1, 0xF);
	Line4 (0, wYVirtRes - 1, wXVirtRes - 1, wYVirtRes - 1, 0xF);
	Line4 (0, 0, 0, wYVirtRes - 1, 0xF);
	Line4 (0, 0, wXVirtRes - 1, wYVirtRes - 1, 0xF);
	Line4 (0, wYVirtRes - 1, wXVirtRes - 1, 0, 0xF);

	SimDumpMemory ("T0313.VGA");

	// For debugging, if anything is on the command line then wait
	// for a keystroke from the user before panning
	if (bFullVGA && (nArguments > 1))
		SimGetKey ();

// Pan right
	for (i = 0; i <= (int) (wXVirtRes - wXRes); i++)
	{
		if (!FrameCapture (REF_PART, REF_TEST))
		{
		   // stub out for diags
		   //	if (_kbhit ())
				if (GetKey () == KEY_ESCAPE) goto PanningTest_abort;
			WaitNotVerticalRetrace ();
		}

		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x33);
		IOByteWrite (ATC_INDEX, (BYTE) (i % 8));
		IOByteWrite (CRTC_CINDEX, 0x08);
		temp = (BYTE) IOByteRead (CRTC_CDATA);
		temp = (BYTE) ((temp & 0x9F) | (((i / 8) % 4) << 5));
		IOByteWrite (CRTC_CDATA, temp);
		temp = (BYTE) (i / 32);
		temp = (BYTE) (temp * 4);
		IOByteWrite (CRTC_CINDEX, 0x0D);
		IOByteWrite (CRTC_CDATA, temp);
	}

// Pan down
	for (i = 0; i < (int) (wYVirtRes - wYRes); i++)
	{
		if (!FrameCapture (REF_PART, REF_TEST))
		{
		   // stub out for diags
		   //	if (_kbhit ())
				if (GetKey () == KEY_ESCAPE) goto PanningTest_abort;
			WaitVerticalRetrace ();
		}

		IOByteWrite (CRTC_CINDEX, 0x0C);
		offset = IOByteRead (CRTC_CDATA) << 8;
		IOByteWrite (CRTC_CINDEX, 0x0D);
		offset += IOByteRead (CRTC_CDATA);
		offset += wXVirtRes / 8;					// Next row down
		IOByteWrite (CRTC_CDATA, LOBYTE (offset));
		IOByteWrite (CRTC_CINDEX, 0x0C);
		IOByteWrite (CRTC_CDATA, HIBYTE (offset));
	}

// Pan left
	for (i = wXVirtRes - wXRes; i >= 0; i--)
	{
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			// stub for diags
			//if (_kbhit ())
				if (GetKey () == KEY_ESCAPE) goto PanningTest_abort;
			WaitNotVerticalRetrace ();
		}

		IOByteRead (INPUT_CSTATUS_1);
		IOByteWrite (ATC_INDEX, 0x33);
		IOByteWrite (ATC_INDEX, (BYTE) (i % 8));

		if ((i % 8) == 7)
		{
			offset--;
			IOByteWrite (CRTC_CINDEX, 0x0D);
			IOByteWrite (CRTC_CDATA, LOBYTE (offset));
			IOByteWrite (CRTC_CINDEX, 0x0C);
			IOByteWrite (CRTC_CDATA, HIBYTE (offset));
		}
	}

// Pan Up
	for (i = wYVirtRes - wYRes; i >= 0; i--)
	{
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			// stub
			//if (_kbhit ())
				if (GetKey () == KEY_ESCAPE) goto PanningTest_abort;
			WaitVerticalRetrace ();
		}

		IOByteWrite (CRTC_CINDEX, 0x0C);
		offset = IOByteRead (CRTC_CDATA) << 8;
		IOByteWrite (CRTC_CINDEX, 0x0D);
		offset += IOByteRead (CRTC_CDATA);
		offset -= wXVirtRes / 8;					// Next row down
		IOByteWrite (CRTC_CDATA, LOBYTE (offset));
		IOByteWrite (CRTC_CINDEX, 0x0C);
		IOByteWrite (CRTC_CDATA, HIBYTE (offset));
	}

	if (SimGetKey () == KEY_ESCAPE) goto PanningTest_abort;

PanningTest_exit:
	SystemCleanUp ();
	return (nErr);

// User hit the <ESC> key during panning
PanningTest_abort:
	nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	goto PanningTest_exit;
}
//

// IRQ2 appears on vector A
#define	VECTOR			0x0A

int VerticalInterruptTest (void);

#ifdef __MSVC16__
// These pragmas are needed for the interrupt handler stuff
#pragma check_stack( off )
#pragma check_pointer( off )

void (_interrupt FAR *oldirq2) ();
void _interrupt FAR irq2handler ();
DWORD	counter = 0;
#endif


#ifndef __MSVC16__
//
//		VerticalInterruptTest - Set a known timing and count the number of
//										vertical periods in a given time.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int VerticalInterruptTest (void)
{
	int	REF_PART = 99;
	int	REF_TEST = 99;


	printf ("\n\nThis test involves system-specific functions that cannot be obtained"
				"\nusing ANSI standard \"C\" functions or in a simulated environment.\n");
	return (FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0));
}
#else				// __STDC__
//
//		VerticalInterruptTest - Set a known timing and count the number of
//										vertical periods in a given time.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int VerticalInterruptTest (void)
{
	int	nErr;
	WORD	wSimType;
	DWORD	time0, time1;
	BYTE	temp;

 	int	REF_PART = 99;
  	int	REF_TEST = 99;
	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\n\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}

	SetMode (0x03);
	TextStringOut ("The following test is self-running and will take about 5 seconds.", ATTR_NORMAL, 1, 1, 0);
	TextStringOut ("Press any key to continue, <ESC> to exit.", ATTR_NORMAL, 1, 3, 0);
	if (SimGetKey () == KEY_ESCAPE)
		goto VerticalInterruptTest_exit;
	SetMode (0x12);

	// Hook IRQ2 vector
	oldirq2 = _dos_getvect (VECTOR);
	_dos_setvect (VECTOR, irq2handler);

	// Enable vertical interrupts
	IOByteWrite (CRTC_CINDEX, 0x11);
	temp = (BYTE) (IOByteRead (CRTC_CDATA) & 0xCF);
	IOByteWrite (CRTC_CDATA, temp);							// Enable and clear
	IOByteWrite (CRTC_CDATA, (BYTE) (temp | 0x10));		// Release clear

	// Wait five seconds
	counter = 0;
	time0 = GetSystemTicks ();
	time1 = time0 + FIVE_SECONDS;
	while (time0 < time1)
		time0 = GetSystemTicks ();

	// Disable vertical interrupts
	IOByteWrite (CRTC_CINDEX, 0x11);
	IOByteWrite (CRTC_CDATA, (BYTE) (IOByteRead (CRTC_CDATA) | 0x20));

	// Restore original vector
	_dos_setvect (VECTOR, oldirq2);

	// Number of counts should be around 300 (60 Hz * 5 seconds)
	if ((counter < 290) || (counter > 310))
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);


VerticalInterruptTest_exit:
	SystemCleanUp ();
	return (nErr);
}

//
//		irq2handler - Handler for the IRQ2 vector
//
//		Entry:	None
//		Exit:		None
//
//		Note:	Be very! careful about stack usage since the stack is not
//				ours at this time. Also, we cannot use most DOS functions
//				here as well (especially disk I/O, memory management, and
//				display functions [printf included]).
//
void _interrupt FAR irq2handler ()
{
	static BYTE	temp0;		// Use our data segment instead of the stack
	static BYTE	temp1;

	if (IOByteRead (INPUT_STATUS_0) & 0x80)					// Is it ours?
	{
		_disable ();													// Disable interrupts
		temp0 = (BYTE) IOByteRead (CRTC_CINDEX);				// Save index register
		IOByteWrite (CRTC_CINDEX, 0x11);
		temp1 = (BYTE) IOByteRead (CRTC_CDATA);
		IOByteWrite (CRTC_CDATA, (BYTE) (temp1 & 0xEF));	// Clear the interrupt
		IOByteWrite (CRTC_CDATA, (BYTE) (temp1 | 0x10));	// Release clear
		IOByteWrite (CRTC_CINDEX, temp0);						// Restore index register
		IOByteWrite (0x20, 0x20);									// Clear 8259
		counter++;
		_enable ();										// Enable interrupts
	}
	else
		_chain_intr (oldirq2);
}
#endif		// __STDC__

//
//
//		DoubleScanTest - Fill the screen with a pattern of horizontal lines
//								and set the double scan bit.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int DoubleScanTest (void)
{
	int	nErr, i;
	WORD	wXRes, wYRes;
	WORD	wSimType;
	BOOL	bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 40;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto DoubleScanTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);		// Use small frame
	SimSetState (TRUE, TRUE, FALSE);					// No RAMDAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);
	GetResolution (&wXRes, &wYRes);

	// Draw a white line on every other scan line
	SetLine4Columns ((WORD) (wXRes / 8));
	for (i = 0; i < (int) wYRes; i += 2)
		Line4 (0, (WORD) i, (WORD) (wXRes - 1), (WORD) i, 0x0F);

	SimDumpMemory ("T0311.VGA");

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto DoubleScanTest_exit;
		}
	}

	IOByteWrite (CRTC_CINDEX, 0x09);
	IOByteWrite (CRTC_CDATA, (BYTE) (IOByteRead (CRTC_CDATA) | 0x80));

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto DoubleScanTest_exit;
		}
	}

DoubleScanTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		CRTCWriteProtectTest - Verify that the lower CRTC registers cannot
//										be written when write protect is enabled.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int CRTCWriteProtectTest (void)
{
	int	nErr, i;
	BYTE	vse, temp;
	int	REF_PART = 1;
	int	REF_TEST = 30;

	nErr = ERROR_NONE;
	SimSetState (TRUE, FALSE, FALSE);		// I/O, no memory, no RAMDAC writes
	SetMode (0x03);

	IOByteWrite (CRTC_CINDEX, 0x11);
	vse = (BYTE) IOByteRead (CRTC_CDATA);
	IOByteWrite (CRTC_CDATA, (BYTE) (vse & 0x7F));		// Unwrite protect CRTC 0-7

	// Verify that it's unprotected by doing an I/O operation on index 2
	IOByteWrite (CRTC_CINDEX, 0x02);
	if ((temp = IsIObitFunctional (CRTC_CDATA, CRTC_CDATA, 0x00)) != 0)
	{
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, CRTC_CINDEX, 0x02, 0, temp);
		goto CRTCWriteProtectTest_exit;
	}

	IOByteWrite (CRTC_CINDEX, 0x11);
	IOByteWrite (CRTC_CDATA, (BYTE) (vse | 0x80));		// Write protect 'em

	// Verify CRTC indexes 0 thru 6
	for (i = 0; i < 7; i++)
	{
		IOByteWrite (CRTC_CINDEX, (BYTE) i);
		if ((temp = IsIObitFunctional (CRTC_CDATA, CRTC_CDATA, 0x00)) != 0xFF)
		{
			nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, CRTC_CINDEX, (WORD) i, 0xFF, temp);
			goto CRTCWriteProtectTest_exit;
		}
	}

	// Verify CRTC index 7
	IOByteWrite (CRTC_CINDEX, 0x07);
	if ((temp = IsIObitFunctional (CRTC_CDATA, CRTC_CDATA, 0x00)) != 0xEF)
		nErr = FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, CRTC_CINDEX, 0x07, 0xEF, temp);

CRTCWriteProtectTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		CharWidthTest - Set a known 9-dot mode, change it to 8-dot mode and
//								measure the results. The line rate should increase
//								about 12.5% from 31.5 KHz to 35.4 Khz.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int CharWidthTest (void)
{
	int	nErr;
	DWORD	hz0, hz1, hz2;
	WORD	wSimType;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\n\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}
	SetMode (0x03);

	TextStringOut ("The following test is self-running and will take about 10 seconds.", ATTR_NORMAL, 1, 1, 0);
	TextStringOut ("During this test, the monitor may lose sync.", ATTR_NORMAL, 1, 2, 0);
	TextStringOut ("Press any key to continue, <ESC> to exit.", ATTR_NORMAL, 1, 4, 0);
	if (SimGetKey () == KEY_ESCAPE)
		goto CharWidthTest_exit;
	TextStringOut ("Please wait...", ATTR_BLINK, 1, 6, 0);

	hz0 = GetFrameRate ();			// Get frame rate*1000
	if (hz0 == 0)
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
				LOWORD (70000l), HIWORD (70000l), 0, 0);
		goto CharWidthTest_exit;
	}

	IOWordWrite (SEQ_INDEX, 0x0101);		// Set 8-dot mode

	// Verify that the line rate and therefore the frame rate increased by
	// about 12.5% (allow 5% error)
	hz1 = GetFrameRate ();
	hz2 = hz0 + (hz0 / 8);			// Add 12.5%
	if ((hz2 < (hz1 - (hz1 / 20))) || (hz2 > (hz1 + (hz1 / 20))))
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
					LOWORD (hz2), HIWORD (hz2), LOWORD (hz1), HIWORD (hz1));

CharWidthTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		Vload2Vload4Test - Verify the combination of VLoad/N and Count by N
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int Vload2Vload4Test (void)
{
	int	nErr;
	BYTE	temp;
	BYTE	byRow, byColumn, byChar;
	WORD	wXRes, wYRes;
	WORD	wSimType;
	BOOL	bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 39;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto VL2VL4_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// No RAMDAC writes
	SetMode (0x0D);
	SimSetState (TRUE, TRUE, TRUE);
	SimSetCaptureMode (CAP_COMPOSITE);
	GetResolution (&wXRes, &wYRes);

	// Fill memory with characters
	for (byRow = 0; byRow < ((wYRes / 16) + 1); byRow++)
	{
		byChar = '0';
		for (byColumn = 0; byColumn < (wXRes / 8); byColumn++)
		{
			PlanarCharOut (byChar++, 0x0F, byColumn, byRow, 0);
			if (byChar > '9') byChar = '0';
		}
	}

	SimDumpMemory ("T0308.VGA");

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto VL2VL4_exit;
		}
	}

	// Set display enable skew to 1
	IOByteWrite (CRTC_CINDEX, 0x11);
	IOByteWrite (CRTC_CDATA, IOByteRead (CRTC_CDATA) & 0x7F);
	IOByteWrite (CRTC_CINDEX, 0x03);
	IOByteWrite (CRTC_CDATA, IOByteRead (CRTC_CDATA) | 0x20);

	// Should be alternating characters of white & gray
	IOByteWrite (SEQ_INDEX, 0x01);
	temp = (BYTE) IOByteRead (SEQ_DATA);
	IOByteWrite (SEQ_DATA, (BYTE) (temp | 0x04));			// Set VLoad/2

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto VL2VL4_exit;
		}
	}

	// Count by two
	IOWordWrite (CRTC_CINDEX, 0xEB17);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto VL2VL4_exit;
		}
	}

	// Count by four
	IOWordWrite (CRTC_CINDEX, 0xE317);
	IOWordWrite (CRTC_CINDEX, 0x2014);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto VL2VL4_exit;
		}
	}

	// Clear Count By N
	IOWordWrite (CRTC_CINDEX, 0x0014);
	IOWordWrite (CRTC_CINDEX, 0xE317);

	// Set display enable skew to 3
	IOByteWrite (CRTC_CINDEX, 0x03);
	IOByteWrite (CRTC_CDATA, IOByteRead (CRTC_CDATA) | 0x60);

	// Should be alternating bars of white, gray, cyan, and blue
	IOByteWrite (SEQ_INDEX, 0x01);
	IOByteWrite (SEQ_DATA, (BYTE) (temp | 0x10));	// VLoad/4 has precedence over VLoad/2

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto VL2VL4_exit;
		}
	}

	// Count by two
	IOWordWrite (CRTC_CINDEX, 0xEB17);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto VL2VL4_exit;
		}
	}

	// Count by four
	IOWordWrite (CRTC_CINDEX, 0xE317);
	IOWordWrite (CRTC_CINDEX, 0x2014);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto VL2VL4_exit;
		}
	}

VL2VL4_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		DotClock2Test - Set a known video timing and verify that the dot clock
//								is divided by 2.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int DotClock2Test (void)
{
	int	nErr;
	DWORD	hz0, hz1;
	WORD	wSimType;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	nErr = ERROR_NONE;

	wSimType = SimGetType ();
	if ((wSimType & SIM_VECTORS) || (wSimType & SIM_SIMULATION))
	{
		printf ("\n\nThis test involves system timings and will not function accurately"
					"\nin a simulated environment.\n");
		nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		return (nErr);
	}

	SetMode (0x03);
	TextStringOut ("The following test is self-running and will take about 20 seconds.", ATTR_NORMAL, 1, 1, 0);
	TextStringOut ("During this test, the monitor may lose sync.", ATTR_NORMAL, 1, 2, 0);
	TextStringOut ("Press any key to continue, <ESC> to exit.", ATTR_NORMAL, 1, 4, 0);
	if (SimGetKey () == KEY_ESCAPE)
		goto DotClock2Test_exit;
	TextStringOut ("Please wait...", ATTR_BLINK, 1, 6, 0);

	// Verify clock is 28.322 MHz here
	hz0 = GetFrameRate ();				// Frame rate in hertz * 1000
	if (hz0 == 0)
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST, LOWORD (70000l), HIWORD (70000l), 0, 0);
		goto DotClock2Test_exit;
	}

	IOByteWrite (SEQ_INDEX, 0x01);
	IOByteWrite (SEQ_DATA, (BYTE) (IOByteRead (SEQ_DATA) | 0x08));		// Clock/2

	// Verify clock is 14.161 MHz here
	// (Actually determine if the frame rate is approximately half the original
	// frame rate plus/minus 5 percent)
	hz1 = GetFrameRate ();
	if ((hz1 < (hz0 / 2 - hz0 / 20)) || (hz1 > (hz0 / 2 + hz0 / 20)))
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
					LOWORD (hz0 / 2), HIWORD (hz0 / 2), LOWORD (hz1), HIWORD (hz1));
		goto DotClock2Test_exit;
	}

	// Set a mode that uses the 25.175 MHz dot clock
	SetMode (0x12);

	// Verify clock is 25.175 MHz here
	hz0 = GetFrameRate ();				// Frame rate in hertz * 1000
	if (hz0 == 0)
	{
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
					LOWORD (60000l), HIWORD (60000l), 0, 0);
		goto DotClock2Test_exit;
	}

	IOByteWrite (SEQ_INDEX, 0x01);
	IOByteWrite (SEQ_DATA, (BYTE) (IOByteRead (SEQ_DATA) | 0x08));		// Clock/2

	// Verify clock is 12.087 MHz here
	// (Actually determine if the frame rate is approximately half the original
	// frame rate plus/minus 5 percent)
	hz1 = GetFrameRate ();
	if ((hz1 < (hz0 / 2 - hz0 / 20)) || (hz1 > (hz0 / 2 + hz0 / 20)))
		nErr = FlagError (ERROR_INVALIDFRAMES, REF_PART, REF_TEST,
					LOWORD (hz0 / 2), HIWORD (hz0 / 2), LOWORD (hz1), HIWORD (hz1));

DotClock2Test_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		VerticalTimesTwoTest - Set a known mode using different sets of CRTC
//										values (one normal, the other vert times 2)
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int VerticalTimesTwoTest (void)
{
	static WORD	tblCRTCFull[] = {
		0x0011, 0x0506, 0x1107, 0xF510, 0x0611, 0xEF12, 0xF315, 0x0216, 0xE717
	};
	static WORD	tblCRTCSmall[] = {
		0x0011, 0x0B06, 0x1007, 0x0A10, 0x0B11, 0x0912, 0x0A15, 0x0B16, 0xE717
	};
	int	nErr, i, ntbl;
	WORD	wXRes, wYRes, wYStart, wLinear;
	WORD	wSimType;
	BOOL	bFullVGA;
 	int	REF_PART = 99;
  	int	REF_TEST = 99;

	ntbl = sizeof (tblCRTCFull) / sizeof (WORD);
	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto VerticalTimesTwoTest_exit;

	if (!bFullVGA) SimSetFrameSize (FALSE);		// Use small frame
	SimSetState (TRUE, TRUE, FALSE);					// No RAMDAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);
	SimSetCaptureMode (CAP_COMPOSITE);
	GetResolution (&wXRes, &wYRes);

	// Draw a pattern
	SetLine4Columns ((WORD) (wXRes / 8));
	Line4 (0, 0, (WORD) (wXRes - 1), 0, 0x0F);
	Line4 ((WORD) (wXRes - 1), 0, (WORD) (wXRes - 1), (WORD) (wYRes - 1), 0x0F);
	Line4 (0, (WORD) (wYRes - 1), (WORD) (wXRes - 1), (WORD) (wYRes - 1), 0x0F);
	Line4 (0, 0, 0, (WORD) (wYRes - 1), 0x0F);
	Line4 (0, 0, (WORD) (wXRes - 1), (WORD) (wYRes - 1), 0x0F);
	Line4 (0, (WORD) (wYRes - 1), (WORD) (wXRes - 1), 0, 0x0F);

	// Start the second image one scan line after the end of the first image
	wYStart = wYRes + 1;
	Line4 (0, wYStart, (WORD) (wXRes - 1), wYStart, 0x0F);
	Line4 ((WORD) (wXRes - 1), wYStart, (WORD) (wXRes - 1), (WORD) (wYStart + wYRes/2 - 1), 0x0F);
	Line4 (0, (WORD) (wYStart + wYRes/2 - 1), (WORD) (wXRes - 1), (WORD) (wYStart + wYRes/2 - 1), 0x0F);
	Line4 (0, wYStart, 0, (WORD) (wYStart + wYRes/2 - 1), 0x0F);
	Line4 (0, wYStart, (WORD) (wXRes - 1), (WORD) (wYStart + wYRes/2 - 1), 0x0F);
	Line4 (0, (WORD) (wYStart + wYRes/2 - 1), (WORD) (wXRes - 1), wYStart, 0x0F);

	SimDumpMemory ("T0306.VGA");

	SimComment ("Normal Image");
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto VerticalTimesTwoTest_exit;
		}
	}

	// Change the CRTC values
	SimComment ("Set Vertical X2 Values");
	for (i = 0; i < ntbl; i++)
	{
		if (bFullVGA)
			IOWordWrite (CRTC_CINDEX, tblCRTCFull[i]);
		else
			IOWordWrite (CRTC_CINDEX, tblCRTCSmall[i]);
	}

	SimComment ("Vertical X2 Image");
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto VerticalTimesTwoTest_exit;
		}
	}

	// Show interaction of vertical times two and double scan
	// Change the display start address to start at the second image
	wLinear = (wXRes / 8) * (wYStart);
	IOByteWrite (CRTC_CINDEX, 0x0C);
	IOByteWrite (CRTC_CDATA, HIBYTE (wLinear));
	IOByteWrite (CRTC_CINDEX, 0x0D);
	IOByteWrite (CRTC_CDATA, LOBYTE (wLinear));

	// Set double scan
	IOWordWrite (CRTC_CINDEX, 0xC009);
	SimComment ("Vertical X2 Image with Double Scan");
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
	}

VerticalTimesTwoTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		TextModeSkewTest - Set a known graphics mode video timing and compare
//									the image after setting the text mode bit.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int TextModeSkewTest (void)
{
	int	nErr;
	WORD	wSimType;
	BOOL	bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 44;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));

	if (!bFullVGA) SimSetFrameSize (FALSE);	// Use small frame
	SimSetState (TRUE, TRUE, FALSE);				// No RAMDAC writes
	SetMode (0x12);
	SimSetCaptureMode (CAP_COMPOSITE);

	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x31);
	IOByteWrite (ATC_INDEX, 0x3F);				// Turn on overscan

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto TextModeSkewTest_exit;
		}
	}

	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x30);
	IOByteWrite (ATC_INDEX, 0x00);			// Treat pixel pipeline as "text"

	if (!FrameCapture (REF_PART, REF_TEST))
	{
		if (GetKey () == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto TextModeSkewTest_exit;
		}
	}

TextModeSkewTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		SyncPulseTimingTest - Set a given video timing and then alter the sync position.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int SyncPulseTimingTest (void)
{
	int	nErr, i, nCount;
	WORD	wXRes, wYRes;
	BYTE	bySyncS, bySyncE;
	WORD	wSimType;
	BOOL	bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 42;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto SyncPulseTimingTest_exit;

	if (bFullVGA)
	{
		bySyncS = 0x50;
		bySyncE = 0x9C;
		nCount = 7;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		bySyncS = 0x28;
		bySyncE = 0x8D;
		nCount = 4;
	}

	SimSetState (TRUE, TRUE, FALSE);					// No RAMDAC writes
	SetMode (0x12);
	SimSetState (TRUE, TRUE, TRUE);
	SimSetCaptureMode (CAP_COMPOSITE);
	GetResolution (&wXRes, &wYRes);

	SetLine4Columns ((WORD) (wXRes / 8));
	Line4 (0, 0, (WORD) (wXRes - 1), 0, 0x0F);
	Line4 ((WORD) (wXRes - 1), 0, (WORD) (wXRes - 1), (WORD) (wYRes - 1), 0x0F);
	Line4 (0, (WORD) (wYRes - 1), (WORD) (wXRes - 1), (WORD) (wYRes - 1), 0x0F);
	Line4 (0, 0, 0, (WORD) (wYRes - 1), 0x0F);
	Line4 (0, 0, (WORD) (wXRes - 1), (WORD) (wYRes - 1), 0x0F);
	Line4 (0, (WORD) (wYRes - 1), (WORD) (wXRes - 1), 0, 0x0F);
	SimDumpMemory ("T0304.VGA");

	IOByteWrite (CRTC_CINDEX, 0x11);
	IOByteWrite (CRTC_CDATA, (BYTE) (IOByteRead (CRTC_CDATA) & 0x7F));	// Disable write protect
	for (i = 0; i < nCount; i++)
	{
		IOByteWrite (CRTC_CINDEX, 0x04);
		IOByteWrite (CRTC_CDATA, bySyncS);
		IOByteWrite (CRTC_CINDEX, 0x05);
		IOByteWrite (CRTC_CDATA, bySyncE);
		bySyncS++;
		bySyncE = (bySyncE & 0xE0) | ((bySyncE + 1) & 0x1F);
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				break;
			}
		}
	}

SyncPulseTimingTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		CursorDisableTest - Display the text mode cursor enable/disable functionality
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
int CursorDisableTest (void)
{
	int	nErr;
	BYTE	temp;
	WORD	key;
	WORD	wSimType;
	BOOL	bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 34;

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto CursorDisableTest_exit;

	if (!bFullVGA)
		SimSetFrameSize (FALSE);						// Set small frame
	SimSetState (TRUE, TRUE, FALSE);					// No RAMDAC writes
	SetMode (0x03);

	IOByteWrite (CRTC_CINDEX, 0x0A);
	if ((temp = IsIObitFunctional (CRTC_CDATA, CRTC_CDATA, 0xDF)) != 0)
		return (FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, CRTC_CINDEX, 0x0A, 0, temp));

	IOWordWrite (CRTC_CINDEX, 0x000A);
	IOWordWrite (CRTC_CINDEX, 0x1F0B);

	SimSetState (FALSE, FALSE, FALSE);
	if (bFullVGA)
		TextStringOut ("Does the cursor look like this block (Y/N)?: \xDB", ATTR_NORMAL, 0, 1, 0);
	else
		TextStringOut ("Block Cursor (\xDB) ", ATTR_NORMAL, 0, 0, 0);
	SimSetState (TRUE, TRUE, TRUE);

	WaitCursorBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		key = GetKey ();
		if (key == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CursorDisableTest_exit;
		}
		else if ((key == KEY_N) || (key == KEY_n))
		{
			nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
			goto CursorDisableTest_exit;
		}
	}

	IOWordWrite (CRTC_CINDEX, 0x200A);

	SimSetState (FALSE, FALSE, FALSE);
	if (bFullVGA)
	{
		TextStringOut ("Is the cursor disabled (Y/N)?", ATTR_NORMAL, 0, 3, 0);
		TextStringOut ("(Press \"N\" if there is an error)", ATTR_NORMAL, 0, 4, 0);
	}
	else
		TextStringOut ("Disabled: ", ATTR_NORMAL, 20, 0, 0);
	SimSetState (TRUE, TRUE, TRUE);

	WaitCursorBlink (BLINK_ON);
	if (!FrameCapture (REF_PART, REF_TEST))
	{
		key = GetKey ();
		if (key == KEY_ESCAPE)
		{
			nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
		}
		else if ((key == KEY_N) || (key == KEY_n))
		{
			nErr = FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0);
		}
	}
	SimDumpMemory ("T0303.VGA");

CursorDisableTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		CursorLocationTest - Display the cursor at various positions on the
//									screen and prompt the user for correct location.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//

typedef struct tagCURSORPOS {
	BYTE		page;				// Page # (0 - 7)
	BYTE		col;				// Left = 0, Right side = 1
	BYTE		row;				// Top = 0, Bottom = 1
} CURSORPOS;

int CursorLocationTest (void)
{
	static CURSORPOS	cp[] = {
		{0, 0, 0},
		{0, 1, 0},
		{0, 1, 1},
		{0, 0, 1},
		{4, 0, 0},
		{4, 1, 0},
		{4, 1, 1},
		{4, 0, 1}
	};
	BYTE		byChar;
	int		nErr, nCP, nPageSize, nRows, nColumns, nRowLength, i;
	WORD		offset;
	SEGOFF	lpVideo;
	WORD		wSimType;
	BOOL		bFullVGA;

	int	REF_PART = 1;
	int	REF_TEST = 24;

	nCP = sizeof (cp) / sizeof (CURSORPOS);		// Number of entries in table
	byChar = '0';

	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto CursorLocationTest_exit;

	// If the VGA simulation library is being used to generate tests
	// on a system where full VGA access is too slow, then use a small
	// frame with only one character row.
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use standard frame
		nRows = 25;
		nColumns = 80;
		nRowLength = nColumns*2;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Set small frame
		nRows = 2;
		nColumns = 40;
		nRowLength = nColumns*2;
	}
	lpVideo = (SEGOFF) 0xB8000000;
	nPageSize = 0x1000;

	SimSetState (TRUE, TRUE, FALSE);					// No RAMDAC writes
	SetMode (0x03);

	if (bFullVGA)
	{
		TextStringOut ("Cursor will be placed at 8 locations, press <ESC> if an error occurs (Page 0).",
							ATTR_NORMAL, 1, 1, 0);
		TextStringOut ("Cursor will be placed at 8 locations, press <ESC> if an error occurs (Page 4).",
							ATTR_NORMAL, 1, 1, 4);
		if (SimGetKey () == KEY_ESCAPE)
			return (FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0));
	}

	SimDumpMemory ("T0302.VGA");

	// Make a full block cursor
	IOWordWrite (CRTC_CINDEX, 0x000A);
	IOWordWrite (CRTC_CINDEX, 0x0F0B);

	for (i = 0; i < nCP; i++)
	{
		offset = cp[i].page * (nPageSize / 2);
		IOByteWrite (CRTC_CINDEX, 0x0C);
		IOByteWrite (CRTC_CDATA, HIBYTE (offset));
		IOByteWrite (CRTC_CINDEX, 0x0D);
		IOByteWrite (CRTC_CDATA, LOBYTE (offset));
		offset = (offset * 2) + (cp[i].row * (nRows - 1) * nRowLength) + (cp[i].col * (nColumns - 1) * 2);
		MemByteWrite (lpVideo + offset, byChar++);
		offset = offset >> 1;
		IOByteWrite (CRTC_CINDEX, 0x0E);
		IOByteWrite (CRTC_CDATA, HIBYTE (offset));
		IOByteWrite (CRTC_CINDEX, 0x0F);
		IOByteWrite (CRTC_CDATA, LOBYTE (offset));

		WaitCursorBlink (BLINK_ON);
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				break;
			}
		}
	}

CursorLocationTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//
//
//		CursorTypeTest - Display the cursor in various sizes next to
//								a character block the same expected size.
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
typedef struct tagCURSORIMAGE {
	BYTE		chr;
	BYTE		start;
	BYTE		stop;
	BYTE		offset;
	BYTE		glyph[16];
} CURSORIMAGE;

int CursorTypeTest (void)
{
	static CURSORIMAGE	ci[] = {
		{128, 0, 0, 1,
			{0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		{129, 0, 1, 1,
			{0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		{130, 0, 2, 1,
			{0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		{131, 1, 1, 1,
			{0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		{132, 6, 7, 1,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		{133, 13, 13, 1,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00}},
		{134, 13, 14, 1,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00}},
		{135, 13, 15, 1,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF}},
		{136, 14, 15, 1,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF}},
		{137, 15, 15, 1,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}},
		{138, 15, 0, 1,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		{139, 7, 6, 1,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		{140, 0, 15, 1,
			{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
		{141, 46, 15, 1,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		{191, 9, 10, 0,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		{192, 9, 10, 0,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
		{193, 4, 13, 0,
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}
	};
	int	nCI, i, nErr;
	BYTE	nColumn, nRow, nColumnInc, nRowInc;
	WORD	wSimType;
	BOOL	bFullVGA;
	int	REF_PART = 1;
	int	REF_TEST = 25;


	nCI = sizeof (ci) / sizeof (CURSORIMAGE);		// Number of entries in "ci"
 	nErr = ERROR_NONE;
	wSimType = SimGetType ();
	bFullVGA = !((wSimType & SIM_TURBOACCESS) && (wSimType & SIM_SIMULATION));
	if (!DisplayInspectionMessage ())
		goto CursorTypeTest_exit;

	// If the VGA simulation library is being used to generate tests
	// on a system where full VGA access is too slow, then use a small
	// frame with only one character row.
	if (bFullVGA)
	{
		SimSetFrameSize (TRUE);							// Use standard frame
		nColumn = 40;
		nRow = 4;
		nColumnInc = 0;
		nRowInc = 1;
	}
	else
	{
		SimSetFrameSize (FALSE);						// Use small frame
		SimSetCaptureMode (CAP_COMPOSITE);
		nColumn = 0;
		nRow = 0;
		nColumnInc = 2;
		nRowInc = 0;
	}

	SimSetState (TRUE, TRUE, FALSE);					// No RAMDAC writes
	SetMode (0x03);
	SimSetState (TRUE, TRUE, TRUE);

	if (bFullVGA)
		TextStringOut ("Compare cursor to image. Press <ENTER> if correct, <ESC> if error.", ATTR_NORMAL, 1, 1, 0);

	// Load the font images
	for (i = 0; i < nCI; i++)
	{
		LoadFontGlyph (ci[i].chr, 16, 0, &ci[i].glyph[0]);
		TextCharOut (ci[i].chr, ATTR_NORMAL, (BYTE) (nColumn + (i * nColumnInc)), (BYTE) (nRow + (i * nRowInc)), 0);
	}

	// Setup to visually show 8-wide vs. 9-wide cursor for inspection
	if (bFullVGA)
	{
		TextStringOut ("8-pixel wide cursor: \xBF", ATTR_NORMAL, 19, 18, 0);
		TextStringOut ("9-pixel wide cursor: \xC0", ATTR_NORMAL, 19, 19, 0);
		TextStringOut ("Overlapping cursor: \xC1", ATTR_NORMAL, 20, 20, 0);
	}

	SimDumpMemory ("T0301.VGA");

	// Show cursor at each step of cursor type tests
	for (i = 0; i < nCI; i++)
	{
		IOByteWrite (CRTC_CINDEX, 0x0A);
		IOByteWrite (CRTC_CDATA, ci[i].start);
		IOByteWrite (CRTC_CINDEX, 0x0B);
		IOByteWrite (CRTC_CDATA, ci[i].stop);
		SetCursorPosition ((BYTE) (nColumn + ci[i].offset), nRow, 0);
		WaitCursorBlink (BLINK_ON);
		if (!FrameCapture (REF_PART, REF_TEST))
		{
			if (GetKey () == KEY_ESCAPE)
			{
				nErr = FlagError (ERROR_USERABORT, REF_PART, REF_TEST, 0, 0, 0, 0);
				goto CursorTypeTest_exit;
			}
		}
		nRow += nRowInc;
		nColumn += nColumnInc;
	}

CursorTypeTest_exit:
	SystemCleanUp ();
	return (nErr);
}
//

//
//		LimitedSetupTest - Enable VGA access via setup register 3C3h
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
//		PCI & AGP subsystems already have a setup sequence necessary to
//		wake up any adapter (initialize addresses, enable I/O, etc.).
//		Therefore, the extensive "setup mode" and other things that the
//		original VGA had to do is not only outmoded, but causes
//		compatibility with initialization on the newer systems.
//		Furthermore, the AGP subsystem (namely, the "bridge") does NOT
//		pass through the I/O addresses of 46E8h and 102h -- thus making
//		a compatible VGA startup sequence IMPOSSIBLE!
//
//		This test will enable and disable the VGA via 3C3h and verify
//		that I/O has indeed been enabled/disabled in the proper manner.
//
int LimitedSetupTest (void)
{
	BYTE	byTemp;
	int	REF_PART = 1;
	int	REF_TEST = 26;

	if (SimGetType () & SIM_SIMULATION)
	{
		// The VGA simulator still does the original wakeup sequence,
		// so enable the VGA first (a mode set does this in the other
		// tests). Note that since this is simulation, the other setup
		// bits in the PS2_SETUP register are meaningless.
		IOByteWrite (PS2_SETUP, 0);					// VGA into setup mode
		IOByteWrite (VGA_SETUP, 0x01);				// Enable VGA
		IOByteWrite (PS2_SETUP, 0xFF);				// VGA out of setup mode
	}

	// Test read/writability of 3C3h
	byTemp = IsIObitFunctional (MB_ENABLE, MB_ENABLE, 0xFE);
	if (byTemp != 0)
	{
		IOByteWrite (MB_ENABLE, 0x01);							// Enable the VGA
		return (FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, MB_ENABLE, 0x00, 0, byTemp));
	}

	IOByteWrite (MB_ENABLE, 0x00);								// Disable the VGA
	if (IsVGAEnabled ())
		return (FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0));

	IOByteWrite (MB_ENABLE, 0x01);								// Enable the VGA
	if (!IsVGAEnabled ())
		return (FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0));

	return (ERROR_NONE);
}
//

//
//		AdapterSetupTest - Enable VGA access via setup register 3C3h
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
//		The IBM VGA adapter card is disabled upon hardware reset. The
//		wake-up procedure for the adapter card is:
//			1) Place the VGA into setup mode via 46E8h.4 (= 1). Note that
//				46E8h is write only on the standard VGA and on most clones.
//			2) Enable the subsystem via bit 102h.0 (= 1)
//			3) Take the VGA out of setup mode (46E8h.4 = 0) and enable the
//				VGA subsystem (46E8h.3 = 1). Note that when writing to
//				46E8h, the low order three bits are the ROM bank bits and
//				should be set to 110b (06h). Most clones do not implement
//				the ROM bank "feature".
//			4) All I/O is enabled at this point. Note that after a h/w reset
//				most registers are set to "0", including the Miscellaneous
//				Output Register (3C2h) which controls access to memory
//				and the I/O address for the CRTC.
//
int AdapterSetupTest (void)
{
	BYTE	temp;

	int	REF_PART = 99;
	int	REF_TEST = 99;

	// Put the adapter into setup mode and test reading and writing
	// the VGA enable bit at port 102h.
	IOByteWrite (ADAPTER_ENABLE, 0x16);						// Put VGA into setup mode
	IOByteWrite (VGA_SETUP, 0x01);							// Enable the VGA
	temp = IsIObitFunctional (VGA_SETUP, VGA_SETUP, 0xFE);
	if (temp != 0)
	{
		IOByteWrite (VGA_SETUP, 0x01);						// Enable VGA
		IOByteWrite (ADAPTER_ENABLE, 0x0E);					// Get out of setup mode
		return (FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, VGA_SETUP, 0x00, 0x00, temp));
	}

	// Take the board out of setup mode and test reading and writing port 102h.
	// Access to 102h should be disabled at this point.
	IOByteWrite (ADAPTER_ENABLE, 0x0E);						// Get out of setup mode
	temp = IsIObitFunctional (VGA_SETUP, VGA_SETUP, 0x00);
	if ((temp & 0xFF) == 0)
	{
		return (FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, VGA_SETUP, 0x00, 0xFF, temp));
	}

	IOByteWrite (ADAPTER_ENABLE, 0x06);						// Disable the VGA
	if (IsVGAEnabled ())
		return (FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0));

	IOByteWrite (ADAPTER_ENABLE, 0x0E);						// Enable the VGA
	if (!IsVGAEnabled ())
		return (FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0));

	return (ERROR_NONE);
}
//

//
//		MotherboardSetupTest - Enable VGA access via setup register 3C3h
//
//		Entry:	None
//		Exit:		<int>		DOS ERRORLEVEL value
//
//		An IBM VGA (PS/2 motherboard) is disabled upon reset. The
//		wake-up procedure for the motherboard VGA (PS/2) is:
//			1) Put the VGA subsystem into setup mode via bit 94h.5 (= 0)
//			2) Enable the subsystem via bit 102h.0 (= 1)
//			3) Take the VGA out of setup mode (94h.5 = 1). I/O decode for
//				102h MUST not occur when the VGA is not in setup mode.
//			4) Further enable the VGA subsystem via bit 3C3h.0 (= 1)
//			5) All I/O is enabled at this point. Note that after a h/w reset
//				most registers are set to "0", including the Miscellaneous
//				Output Register (3C2h) which controls access to memory
//				and the I/O address for the CRTC.
//
int MotherboardSetupTest (void)
{
	BYTE	byTemp;
	BYTE	byOrgSetup;

 	int	REF_PART = 99;
  	int	REF_TEST = 99;
	// Put the motherboard into setup mode and test reading and writing
	// the VGA enable bit at port 102h.
	byOrgSetup = (BYTE) IOByteRead (PS2_SETUP);
	IOByteWrite (PS2_SETUP, (BYTE) (byOrgSetup & 0xDF));	// Put PS/2 into setup mode
	byTemp = IsIObitFunctional (VGA_SETUP, VGA_SETUP, 0xFE);
	if (byTemp != 0)
	{
		IOByteWrite (VGA_SETUP, 0x01);							// Enable VGA
		IOByteWrite (PS2_SETUP, byOrgSetup);					// Get out of setup mode
		return (FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, VGA_SETUP, 0x00, 0x00, byTemp));
	}

	// Take the motherboard out of setup mode and test reading and writing
	// port 102h. Access to 102h should be disabled at this point.
	IOByteWrite (PS2_SETUP, (BYTE) (byOrgSetup | 0x28));
	byTemp = IsIObitFunctional (VGA_SETUP, VGA_SETUP, 0x00);
	if ((byTemp & 0xFF) == 0)
	{
		return (FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, VGA_SETUP, 0x00, 0xFF, byTemp));
	}

	// Test read/writability of 3C3h
	byTemp = IsIObitFunctional (MB_ENABLE, MB_ENABLE, 0xFE);
	if (byTemp != 0)
	{
		IOByteWrite (MB_ENABLE, 0x01);							// Enable the VGA
		return (FlagError (ERROR_IOFAILURE, REF_PART, REF_TEST, MB_ENABLE, 0x00, 0, byTemp));
	}

	IOByteWrite (MB_ENABLE, 0x00);								// Disable the VGA
	if (IsVGAEnabled ())
		return (FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0));

	IOByteWrite (MB_ENABLE, 0x01);								// Enable the VGA
	if (!IsVGAEnabled ())
		return (FlagError (ERROR_UNEXPECTED, REF_PART, REF_TEST, 0, 0, 0, 0));

	return (ERROR_NONE);
}
//
