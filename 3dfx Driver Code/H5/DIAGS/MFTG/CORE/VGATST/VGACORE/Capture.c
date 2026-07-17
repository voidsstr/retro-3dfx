//
//		CAPTURE.CPP - Routines for simulating the scan out process, and generating a frame in Windows .BMP format.
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	6/3/98
//
//		Routines in this file:
//		InternalCaptureFrame	Simulate a single frame scanout and save results to BMP file
//		GetVideoCharacter		Loads the 9 pixel shift register every character 
//		ScanAddressMUX			Modify the scanout address for chaining and CGA mode
//		PaletteMap				Map 4 bit pixel value thru the palette to an 8 bit RAMDAC address
//		NextScanAddr			Return the next address to fetch a character from memory given the current address
//		ShiftReg					Shift a register
//		InitFrameVars			Initialize all of the frame variables
//		FramePrescan			Handle any wrapped signals from previous frame
//		HorizWrap				Wrap Values around the end of the scan line
//
#include "vgaint.h"

// Local variables for frame capture logic
static	FILE *scanfile;				// File pointer for output file
static	int Height;						// Number of scanlines in output file
static	int Width;						// Number of pixels in scanline
static	int CharWidth;					// Number of characters DISPLAYED in scan line

// Vertical state
static	WORD wVCount;					// Vertical scanline counter
static	BYTE byEarlyVDisplayEnable;// Vertical display enable at VSYNC
static	BYTE byVDisplayEnable;		// Vertical display enable at scan end
static	BYTE byVBlank;					// Vertical blanking
static	BYTE byVRetrace;				// Vretrace
static	BYTE byRowCount;				// Row counter
static	BYTE byDoubleScan;			// State for 2T4
static	BYTE byHorizRetrace;			// Horizontal retrace select

//	Addressing
static	WORD wCurAddr;					// Current map address
static	WORD wSaveAddr;				// Map address of start of scanline

// Unpacked versions of > 8 bit registers
static	WORD wVDispEnable;			// Vertical display enable
static	WORD wVTotal;					// Vertical total
static	WORD wStartVBlank;			// Start vertical blanking
static	WORD wStartVRetrace;			// Start vertical retrace
static	WORD wLineCompare;			// Line compare

// Horizontal state
static	BYTE byHCount;					// Horizontal character counter
static	BYTE byHDispEnable;			// Horizontal display enable
static	BYTE byHBlank;					// Horizontal blanking
static	BYTE byHRetrace;				// Horizontal retrace
static	BYTE byHLoadCount;			// Horizontal character loaded counter

// Unpacked versions of horizontal registers
static	BYTE byEndHBlank;				// End horizontal blanking

// Pixel state
static	BYTE byShiftReg[9];			// Pixel shift register
static	BYTE byShiftPos;				// Current pixel position in shift register
static	char chCurPixPan;				// Current pixel pan value
static	char chCurBytePan;			// Current pixel pan value
static	BYTE byMaxShiftPos;			// 8 or 9 pixel characters
static	BYTE byPixelCount;			// Pixel Count
static	BYTE byTemp;					// Pixel accumulation for mode 13
static	BYTE byInhibitShift;			// Inhibit shifting shift register (for illegal panning values)

//
//		InternalCaptureFrame	- Simulate a single frame scanout and save results to BMP file
//
//		Entry:	szFilename	BMP filename
//		Exit:		<int>			Error code (0 if success, Non-zero = Error)
//
int InternalCaptureFrame (LPSTR szFilename)
{
   if (InitFrameVars (szFilename) != 0)
		return (TRUE);
  
	// Prescan first scanline to generate wrapped signals, if any 
	byHBlank = 0;
	byHRetrace = 0;
	FramePrescan ();

	// This handles an IBM special case of line compare == 0
	if (wLineCompare == 0) 
	{
  		// May have to reset pixel pan
  		if (byATCReg[ATC_MODE] & BIT_PP) 
		{
    		// Reset pixel pan register to 0 panning is different 9 bit 
			// characters graphics is always an 8 bit character
			chCurBytePan = 0;
    		if (!(bySEQReg[SEQ_CLOCKING] & BIT_D89)) 
				chCurPixPan = 1;
    		else 
				chCurPixPan = 0;
  		}
  		wSaveAddr = 0;
  		byRowCount = 0;
	}
  
 	// Repeat for all vertical scan lines
	while (wVCount < (wVTotal +2))
	{
		// SCANLINE INITIALIZATION
		byHCount = 0;
		byHDispEnable = 0;
		byHLoadCount = 0;

		chGlobalDispStartSkew = (byCRTCReg[CRTC_END_HBLANK] & FLD_DES) >> SHFT_DES;

		// If text mode, skew by 1
		if ((byATCReg[ATC_MODE] & BIT_G) == 0)
			chGlobalDispStartSkew++;

		// Initialization for count by 2/4
		chGlobalCbCounter = 0;
//		if (wVCount == 0) printf ("\nInternalCaptureFrame: chGlobalCbCounter = %02Xh", chGlobalCbCounter);

		// Initialization for VLoad 2/4
		if (bySEQReg[SEQ_CLOCKING] & BIT_SH4)
		{
  			// VLoad 4
  			chGlobalLoadCounter = 3 - chCurBytePan;
		}
		else if (bySEQReg[SEQ_CLOCKING] & BIT_SL)
		{
  			// VLoad 2
	  		chGlobalLoadCounter = (3 - chCurBytePan) & 0x1;
    	}

		// Start at the saved address
		wCurAddr = wSaveAddr;
    
		// Setup shift register empty
		chGlobalSREmpty = 1;

		// If display enabled - get first character
    	if (!(bySEQReg[SEQ_CLOCKING] & BIT_SO)) 
		{
      	// Special case for weird behavior in HiRES 4 bit mode
      	if ((byGDCReg[GDC_MODE]&BIT_C256) && (!(byATCReg[ATC_MODE] & BIT_PW)))
			{
				if (chCurPixPan == (byMaxShiftPos - 1))
				{

	  				// Skip first character
	  				wCurAddr = GetVideoCharacter (byShiftReg, wCurAddr, byRowCount,
					 										  1, chCurPixPan, &byShiftPos);
					// Start next character at 0
	  				wCurAddr = GetVideoCharacter (byShiftReg, wCurAddr, byRowCount,
					 										  1, 0, &byShiftPos);
				}
				else
				{
	  				// Normal prefetch for pixel pan
	  				wCurAddr = GetVideoCharacter (byShiftReg, wCurAddr, byRowCount,
					 										  1, chCurPixPan + 1, &byShiftPos);
				}
      	} 
			else 
			{
				// Normal prefetch for pixel pan
				wCurAddr = GetVideoCharacter (byShiftReg, wCurAddr, byRowCount,
														  1, chCurPixPan, &byShiftPos);
			}
		}
    
//    END SCANLINE INITIALIZATION
  
		// Repeat for all pixels in scanline
		while (byHCount < (byCRTCReg[CRTC_HORIZ_TOT] + 5))
		{
			if (chGlobalDispStartSkew >= 0)
			{
				if (chGlobalDispStartSkew-- == 0)
				{
	  				byHDispEnable = 1;
				}
				else
				{
	  				// Prefetch for pixel pan
					if (((byATCReg[ATC_MODE] & BIT_G) == BIT_G) || (chGlobalDispStartSkew != 0))
		  				wCurAddr = GetVideoCharacter (byShiftReg, wCurAddr, byRowCount,
															 1, chCurPixPan, &byShiftPos);
				}
      	}
      
      	if ((byHDispEnable && byVDisplayEnable) && (!byHBlank))
			{
				// If display enabled - write out a character
				byPixelCount = 0;
				while (byPixelCount < byMaxShiftPos) 
				{
	  				// If display enabled
	  				if (!(bySEQReg[SEQ_CLOCKING] & BIT_SO))
					{
	    				if (byATCReg[ATC_MODE] & BIT_PW)
						{
	      				// Due to loading 8 bit pixels in 4 bit shift 
		 					// register - mode 13 unpacking is different 
		 					// from all other modes
	      				if (byPixelCount & 1) 
								// Write out duplicate of already processed pixel
								WriteBMP8Color (scanfile, byTemp);
							else
							{
								if (byGDCReg[GDC_MODE]&BIT_C256)
								{
		  							// Assemble MS4 of pixel
		  							byTemp = (PaletteMap (byShiftReg[0]) << 4) & 0xF0;
		  							byShiftPos++;
		  							ShiftReg (byShiftReg);
		  							// If shift register exhausted - load it
		  							if (byShiftPos == byMaxShiftPos)
									{
		    							wCurAddr = GetVideoCharacter (byShiftReg,
						   						  wCurAddr, byRowCount, 0, 0, NULL);
										byShiftPos = 0;
		  							}
									// Assemble LS4 of pixel
									byTemp |= PaletteMap (byShiftReg[0]) & 0x0F;
									byShiftPos++;
		  							ShiftReg (byShiftReg);
		  							// If shift register exhausted - load it
									if (byShiftPos == byMaxShiftPos)
									{
		    							wCurAddr = GetVideoCharacter(byShiftReg, wCurAddr,
																			 byRowCount, 0, 0, NULL);
										byShiftPos = 0;
									}
		 							// Write out 8 bit pixel
									WriteBMP8Color(scanfile,byTemp);
								} 
								else 
								{
		  							byTemp = PaletteMap (byShiftReg[0]);
		  							WriteBMP8Color (scanfile,byTemp);
		  							byShiftPos++;
		  							ShiftReg (byShiftReg);
		  							byShiftPos++;
		  							ShiftReg (byShiftReg);
									// If shift register exhausted - load it
									if(byShiftPos == byMaxShiftPos)
									{
		    							wCurAddr = GetVideoCharacter (byShiftReg, wCurAddr,
						   												 	byRowCount, 0, 0, NULL);
										byShiftPos = 0;
		  							}
								}
	      				}
	    				}
						else
						{
	      				// All other modes - pixel unpack if double clocked
							// duplicate pixel
	      				if (bySEQReg[SEQ_CLOCKING]&BIT_DC)
								WriteBMP8Color (scanfile, PaletteMap(byShiftReg[0]));
	      				WriteBMP8Color (scanfile, PaletteMap(byShiftReg[0]));
	      				byShiftPos++;
	      				ShiftReg (byShiftReg);
	      				// If shift register exhausted - load it
							if (byShiftPos == byMaxShiftPos)
							{
								wCurAddr = GetVideoCharacter (byShiftReg, wCurAddr, 
																	  	byRowCount, 0, 0, NULL);
								byShiftPos = 0;
	      				}
	    				}
	  				}
					else
					{
						// Display shut off - display enable pixels are black
						// if double clocked - duplicate pixel
	    				if (bySEQReg[SEQ_CLOCKING]&BIT_DC)
	      				 WriteBMP8Color (scanfile,0x00);
	    				WriteBMP8Color (scanfile,0x00);
	  				}
	  				byPixelCount++;
				}
      }
		else
		{  // These are pixels outside video enable
			if (byGlobalCaptureMode != CAP_SCREEN)
			{
	  			// Display outside
				if (byGlobalCaptureMode == CAP_OVERSCAN)
				{
	    			 // Check for border
	    			if ((!byVBlank) && (!byHBlank))
	      			// Use border color
	      			byTemp = byATCReg[ATC_OVERSCAN];
					else
	      			// Use black
	      			byTemp = 0x00;
	  			}
				else
				{
	    			// Display timing signals VBLANK/VSYNC/HBLANK/HSYNC
	    			byTemp = 0xF0;
	    			// Handle vertical blanking
	    			if (byVBlank)
	      			byTemp |= 0x08;
	    			// Handle vertical retrace
	    			// Use correct polarity
	    			if(byMiscReg&BIT_VSP) 
					{
	      			if(!byVRetrace)
							byTemp |= 0x04;
	    			}
					else 
					{
	      			if(byVRetrace)
							byTemp |= 0x04;
	    			}
	    			// handle horizontal blanking
	    			if (byHBlank)
	      			byTemp |= 0x02;
	    			// Handle horizontal retrace  use correct polarity
	    			if (byMiscReg & BIT_HSP)
					{
	      			if (!byHRetrace)
							byTemp |= 0x01;
	    			}
					else
					{
	      			if (byHRetrace)
							byTemp |= 0x01;
	    			}
	    
	  			}
	  		// All timing and border color remain constant for  one character time
	  		// Do a complete character worth
	  		byPixelCount = 0;
	  		while (byPixelCount++ < byMaxShiftPos)
			{
	    		// If double clocked - duplicate pixel
	    		if (bySEQReg[SEQ_CLOCKING] & BIT_DC)
	      		WriteBMP8Color (scanfile, byTemp);
	    		WriteBMP8Color (scanfile, byTemp);
	  		}
		}
	}
      
//    START HORIZONTAL SIGNAL UPDATE

		// Update horizontal signals every character time this order implied 
		// from the byHDispEnable-1 in the mode table.
		// Horizontal display enable
		if (byHCount == byCRTCReg[CRTC_HDISP_ENA])
		{
			chGlobalDispEndSkew = (byCRTCReg[CRTC_END_HBLANK] & FLD_DES) >> SHFT_DES;
			if ((byATCReg[ATC_MODE] & BIT_G) == 0)
				chGlobalDispEndSkew++;
		}

      if (chGlobalDispEndSkew >= 0)
		{
			if (chGlobalDispEndSkew-- == 0)
	  			byHDispEnable = 0;
		}

      // Start horizontal blanking
      if (byHCount == byCRTCReg[CRTC_START_HBLANK])
		{
			if ((byATCReg[ATC_MODE] & BIT_G) == 0)
		      chGlobalHBStartSkew = 1;
			else
				byHBlank = 1;
		}
		if (chGlobalHBStartSkew >= 0)
		{
			if (chGlobalHBStartSkew-- == 0)
				byHBlank = 1;
		}

      // End horizontal blanking
		if (byHBlank)
		{
			if ((byEndHBlank & CMP_EB) == (byHCount & CMP_EB))
			{
 				if ((byATCReg[ATC_MODE] & BIT_G) == 0)
			      chGlobalHBEndSkew = 1;
				else
					byHBlank = 0;
			}
			if (chGlobalHBEndSkew >= 0)
			{
				if (chGlobalHBEndSkew-- == 0)
					byHBlank = 0;
			}
      }

      // Start horizontal retrace
      // Horizontal retrace happens	3 clock cycles early
		if(HorizWrap(byHCount+3) == byCRTCReg[CRTC_START_HRET])
		{
			chGlobalRetStartSkew = (byCRTCReg[CRTC_END_HRET] & FLD_HRD) >> SHFT_HRD;
      }

		if (chGlobalRetStartSkew >= 0)
		{
			if (chGlobalRetStartSkew-- == 0)
			{
	  			byHRetrace = 1;

				// START VERTICAL SIGNAL UPDATE

				// Horizontal retrace select
				if ((!(byCRTCReg[CRTC_MODE] & BIT_HRS)) ||
					 ((byHorizRetrace++) & 1))
				{
					if ((!(byCRTCReg[CRTC_MAX_SCAN_LINE] & BIT_DSC)) ||
					 ((byDoubleScan++) &1))
					{
						// If second scanline of by two mode or next scanline
						//    bump row counter
						if ((byRowCount++) ==
							 (byCRTCReg[CRTC_MAX_SCAN_LINE] & FLD_MSL))
						{
			  				byRowCount = 0;
			  				wSaveAddr += (byCRTCReg[CRTC_OFFSET]) * 2;
						} 
					}
					// Update vertical signals every scanline
					//    vertical display enable
					if(wVCount == wVDispEnable)
						byEarlyVDisplayEnable = 0;
		      	// Start vertical blanking
			      if (wVCount == wStartVBlank)
						byVBlank = 1;
			      // End vertical blanking
		   	   if (byVBlank)
					{
						if ((byCRTCReg[CRTC_END_VBLANK] & FLD_VBE) ==
							 ((BYTE)(wVCount & FLD_VBE)))
							byVBlank = 0;
		      	}
			      // Start vertical retrace
					if (wVCount == wStartVRetrace)
						byVRetrace = 1;
		   	   // End vertical retrace
		      	if (byVRetrace)
					{
						if ((byCRTCReg[CRTC_END_VRET] & FLD_VRE) ==
							 ((BYTE)(wVCount & FLD_VRE)))
			  				byVRetrace = 0;
			      }
			      // Do line compare
			      if ((wVCount == wLineCompare) && (wLineCompare != 0))
					{
						// May have to reset pixel pan
						if (byATCReg[ATC_MODE]&BIT_PP)
						{
				  			// Reset pixel pan register to 0, 
							//    panning is different 9 bit characters
							// graphics is always an 8 bit character
							chCurBytePan = 0;
							if (!(bySEQReg[SEQ_CLOCKING] & BIT_D89))
				    			chCurPixPan = 1;
							else
				    			chCurPixPan = 0;
						}
					wSaveAddr = 0;
					byRowCount = 0;
					}
			      wVCount++;
				}
				else
				{
					if ((!(byCRTCReg[CRTC_MAX_SCAN_LINE] & BIT_DSC)) ||
							 ((byDoubleScan++) & 1))
					{
						// if second scanline of by two mode or next scanline
						//    bump row counter
						if ((byRowCount++) ==
							 (byCRTCReg[CRTC_MAX_SCAN_LINE] & FLD_MSL))
						{
				  			byRowCount = 0;
				  			wSaveAddr += (byCRTCReg[CRTC_OFFSET]) * 2;
						}
					}
				}
				// END VERTICAL SIGNAL UPDATE
			}
		}
      // End horizontal retrace
      // Horizontal retrace ends	3 clock cycles early
		if (byHRetrace)
		{
			if((HorizWrap(byHCount+3)&FLD_EHR) == 
				(WORD)(byCRTCReg[CRTC_END_HRET]&FLD_EHR))
			{
				if(chGlobalRetEndSkew < 0)
		  			chGlobalRetEndSkew = (byCRTCReg[CRTC_END_HRET] & FLD_HRD)
												 >> SHFT_HRD;
			}
		}

       if (chGlobalRetEndSkew >= 0)
		{
			if (chGlobalRetEndSkew-- == 0)
	  			byHRetrace = 0;
		}

//    END HORIZONTAL SIGNAL UPDATE
      
		// Bump horizontal character counter
      byHCount++;
    }

//    END OF SCAN LINE

	// Update Vertical display enable
	byVDisplayEnable = byEarlyVDisplayEnable;


	// Setup shift register empty
	chGlobalSREmpty = 1;
    
	}

	CloseBMP8 (scanfile);

  return (0);
}

//
//		HorizWrap - Wraps horizontal character position around scan line
//
// 	Entry:	pos				Current horizontal character position
//
//		Exit:		WORD				Position wrapped around the end of scanline
//
WORD HorizWrap(WORD pos)
{
	if(pos >= (WORD)(byCRTCReg[CRTC_HORIZ_TOT] + 5))
		return(pos - (byCRTCReg[CRTC_HORIZ_TOT] + 5));
	else
		return(pos);
}

//
//		GetVideoCharacter -	Loads the 9 pixel shift register every character 
//										clock cycle.
// 	Entry:	byPtrShiftReg	Pointer to 9 entry output shift register
//					wAddress			Current scanout address in map
//					byRowCount		Current row counter value form CRTC row counter
//					byApplyPan		Pan shift register
//					chPan				Character for Pan
//					byPtrShift		Byte pointer to shift
//		Exit:		WORD				Address for next character
//
WORD GetVideoCharacter (BYTE *byPtrShiftReg, WORD wAddress, BYTE byRowCount,
			 BYTE byApplyPan, char chPan, BYTE *byPtrShift)
{
	int	i;
	WORD	wNextAddress, wAddr;

	// Handle VLoad 2/4
	if (bySEQReg[SEQ_CLOCKING] & BIT_SH4)
	{
		// VLoad 4
		if (chGlobalLoadCounter-- == 0)
			// Fall thru and reload shift register
			chGlobalLoadCounter = 3;
		else
		{
      	// Return - don't reload shift register
      	if (chGlobalSREmpty)
			{
//				if (wVCount == 0) printf ("\nGetVideoCharacter: Address = %04Xh", wAddress);
				return (wAddress);
			}
      	else
			{
				if (byHLoadCount > 1)
				{
					wAddr = NextScanAddr (wAddress);
//					if (wVCount == 0) printf ("\nGetVideoCharacter: Address = %04Xh", wAddr);
					return (wAddr);
				}
//				if (wVCount == 0) printf ("\nGetVideoCharacter: Address = %04Xh", wAddress);
				return (wAddress);
			}
		}
	}
	else if (bySEQReg[SEQ_CLOCKING] & BIT_SL)
	{
		// VLoad 2
		if (chGlobalLoadCounter-- == 0)
      	// Fall thru and reload shift register
			 chGlobalLoadCounter = 1;
		else
		{
      	// Return - don't reload shift register
			if (chGlobalSREmpty)
			{
//				if (wVCount == 0) printf ("\nGetVideoCharacter: Address = %04Xh", wAddress);
				return (wAddress);
			}
			else
			{
				if (byHLoadCount > 1)
				{
					wAddr = NextScanAddr (wAddress);
//					if (wVCount == 0) printf ("\nGetVideoCharacter: Address = %04Xh", wAddr);
					return (wAddr);
				}
//				if (wVCount == 0) printf ("\nGetVideoCharacter: Address = %04Xh", wAddress);
				return (wAddress);
			}
    	}
	}
  	byHLoadCount++;					// Bump VLoad/N counter kludge

	chGlobalSREmpty = 0;

  	// Decode map data depending on current mode
  	if (byGDCReg[GDC_MISC] & BIT_GM)
	{
   	// Graphics mode
      // Mode 13
		if (byGDCReg[GDC_MODE] & BIT_C256)
			wNextAddress = GetVGAChar (byPtrShiftReg, wAddress, byRowCount);
      // CGA mode
		else if (byGDCReg[GDC_MODE]&BIT_GDC_SR)
      	wNextAddress = GetCGAChar (byPtrShiftReg, wAddress, byRowCount);
      // Planar modes
		else
      	wNextAddress = GetEGAChar (byPtrShiftReg, wAddress, byRowCount);
  	}
	// Text mode
	else
		wNextAddress = GetTextChar (byPtrShiftReg, wAddress, byRowCount);

	if (byApplyPan)
  	{
		*byPtrShift = chPan;
		// Pan initialization for vload 2/4
      // VLoad 4
		if (bySEQReg[SEQ_CLOCKING] & BIT_SH4)
      	for (i = 0; i < (chCurBytePan * 8); i++)
				ShiftReg (byPtrShiftReg);
		// VLoad 2
		else if (bySEQReg[SEQ_CLOCKING] & BIT_SL)
      	for (i = 0; i < ((chCurBytePan>>1) * 8); i++)
				ShiftReg (byPtrShiftReg);

		if (chPan == -1)
		{
			byInhibitShift = 1;
		}
		else
		{
			byInhibitShift = 0;
	    	for (i = 0; i < chPan; i++)
				ShiftReg (byPtrShiftReg);
		}
  }
  
//	if (wVCount == 0) printf ("\nGetVideoCharacter: Address = %04Xh", wNextAddress);
	return (wNextAddress);
}

//
//		ScanAddressMUX	- Modify the scanout address for chaining and CGA mode
//
// 	Entry:	wAddress		0-64K map address
//					byRow			Row counter
//					WORD			0-64K modified address
//
WORD ScanAddressMUX (WORD wAddress, BYTE byRow)
{
	WORD wMuxAddr;

	// Double word mode
	if(byCRTCReg[CRTC_UNDERLINE] & BIT_DW)
		wMuxAddr = (wAddress << 2) | ((wAddress >> 12) & 0x3);
	// Byte mode
	else if (byCRTCReg[CRTC_MODE] & BIT_WB)
		wMuxAddr = wAddress;
	else
	{
		// Word mode
		// Wrap address 15
		if (byCRTCReg[CRTC_MODE] & BIT_ADW)
			wMuxAddr = ((wAddress & 0x8000) >> 15) | ((wAddress & 0x7FFF) << 1);
     	// Wrap address 13
		else
			wMuxAddr = ((wAddress & 0x2000) >> 13) | ((wAddress & 0x7FFF) << 1);
	}
	// Use row counter for bit 13
	if (!(byCRTCReg[CRTC_MODE] & BIT_CMS0))
		wMuxAddr = ((byRow << 13) & 0x2000) | (wMuxAddr & 0xDFFF);
	return (wMuxAddr);
}

//
//		PaletteMap	-	Map 4 bit pixel value thru the palette to an 8 bit RAMDAC
//							address.
//
// 	Entry:			byData		4 bit input pixel value
// 	Exit:				BYTE			8 bit mapped pixel
//
BYTE PaletteMap (BYTE byData)
{
	unsigned char chIndex;

	// If palette disabled - display overscan
	if (!(byATCAddr & BIT_IPAS))
		chIndex = byATCReg[ATC_OVERSCAN];
	else
	{
		// Get 4 bit data
		chIndex = byData & byATCReg[ATC_COLOR_PLANE] & FLD_ECP;
		// Graphics blink
		if ((byATCReg[ATC_MODE] & BIT_EB) && (byATCReg[ATC_MODE] & BIT_G))
		{
			// For monochrome graphics, if blink is enabled look at lower
			// three bits and force the blink bit as the fourth bit
			if (byATCReg[ATC_MODE] & BIT_ME)
			{
				chIndex = (chIndex & 0x7) | (((~byGlobalAttrBlink) << 3) & 0x08);
			}
			else
			{
				// For color graphics, if the most significant bit is set,
				// replace it with the blink bit
				if (chIndex & 0x08)
					chIndex = (chIndex & 0x7) | (((~byGlobalAttrBlink) << 3) & 0x08);
				else
					chIndex |= 0x08;
			}
		}
    
    	// Pass thru palette registers
		chIndex = byATCReg[chIndex];
    	// Get bits 4&5 from right place
		// Get bits 4567 from color select
		if (byATCReg[ATC_MODE] & BIT_PS)
			chIndex = (chIndex & 0x0F) | ((byATCReg[ATC_COLOR_SEL] & 0x0F) << 4);
		// Get bits 67 from color select
		else
			chIndex = (chIndex & 0x3F) | ((byATCReg[ATC_COLOR_SEL] & 0x0C) << 4);
	}
	return (chIndex & byDACMask);
}

//
//		NextScanAddr	-	Return the next address to fetch a character from memory given
//								the current address.
//
// 	Entry:	wAddress	0-64K VGA scanout address
//		Exit:		WORD		0-64K VGA memory address for next character
//
WORD NextScanAddr (WORD wAddress)
{
	WORD	wAddr;

	if (byCRTCReg[CRTC_MODE] & BIT_CB2)
	{
		// Count by 2 - increment address every 2 character clocks
//		if (wVCount == 0) printf ("\nNextScanAddr: chGlobalCbCounter = %02Xh", chGlobalCbCounter);
		if (chGlobalCbCounter-- == 0)
		{
			chGlobalCbCounter = 1;
			wAddr = wAddress + 1;
//			if (wVCount == 0) printf ("\nNextScanAddr: chGlobalCbCounter = %02Xh", chGlobalCbCounter);
		}
		else
			wAddr = wAddress;
	}
	else if (byCRTCReg[CRTC_UNDERLINE] & BIT_CB4)
	{
		// Count by 4 - increment address every 4 character clocks
//		if (wVCount == 0) printf ("\nNextScanAddr: chGlobalCbCounter = %02Xh", chGlobalCbCounter);
		if (chGlobalCbCounter-- == 0)
		{
			chGlobalCbCounter = 3;
			wAddr = wAddress + 1;
//			if (wVCount == 0) printf ("\nNextScanAddr: chGlobalCbCounter = %02Xh", chGlobalCbCounter);
		}
		else
			wAddr = wAddress;
  	}
	else
	{
    	// Count by 1 - increment address every character clock
		wAddr = wAddress + 1;
	}

//	if (wVCount == 0) printf ("\nNextScanAddr: Address = %04Xh", wAddr);
	return (wAddr);
}

//
// 	ShiftReg	-	Shift a register 
//
//		Entry:		pShiftReg		Register to shift
//		Exit:			None
//
void ShiftReg (BYTE *pShiftReg)
{
	BYTE byTmp;
	BYTE i;

	// For illegal pan values
	if (byInhibitShift)
	{
	 	byInhibitShift = 0;
		return;
	}

	byTmp = pShiftReg[0];

	for (i = 0; i < 8; i++)
		pShiftReg[i] = pShiftReg[i + 1];
	if (!(bySEQReg[SEQ_CLOCKING] & BIT_D89))
	{
		// 9 bits/char
		if (byGDCReg[GDC_MODE] & BIT_C256)
		{
			// Mode 13
			pShiftReg[8] = byTmp >> 1;
		}
		else if (byGDCReg[GDC_MODE] & BIT_GDC_SR)
		{
			// CGA mode
			pShiftReg[8] = byTmp >> 2;
		}
		else
		{
			// Planar modes
			pShiftReg[8] = byTmp >> 1;
		}
	}
	else
	{
		// 8 bits/char
		if (byGDCReg[GDC_MODE] & BIT_C256)
		{
			// Mode 13
			pShiftReg[7] = byTmp >> 1;
		}
		else if (byGDCReg[GDC_MODE] & BIT_GDC_SR)
		{
			// CGA mode
			pShiftReg[7] = byTmp >> 2;
		}
		else
		{
			// Planar modes
			pShiftReg[7] = byTmp >> 1;
		}
	}
}

//
//		InitFrameVars - Initialize all of the Frame variable.
//
//		Entry:	szFilname	Pointer to the filename we are to write.
//		Exit:		<int>			Error code (0 if success, Non-zero = Error)
//
int InitFrameVars (LPSTR szFilename)
{
	// To Satisfy compiler
	byTemp = 0;

	// 8 or 9 bits per character
	if (bySEQReg[SEQ_CLOCKING] & BIT_D89) 
  		byMaxShiftPos = 8;
	else
  		byMaxShiftPos = 9;

	// This section handles HBLANK during DISPLAY ENABLE
	{
  		int delta;
  		int skew;
  
  		skew = (byCRTCReg[CRTC_END_HBLANK] & FLD_DES) >> SHFT_DES;
  		CharWidth = byCRTCReg[CRTC_HDISP_ENA];

    	// Compensate for HBLANK occuring prior to the end of DISPLAY ENABLE
  		if ((delta = ((CharWidth + skew) - (byCRTCReg[CRTC_START_HBLANK]))) > 0) 
    		CharWidth -= delta;
	}

	// Write out file header
	if(byGlobalCaptureMode == CAP_SCREEN) 
	{
  		// Output visable pixels
  		Height = byCRTCReg[CRTC_VDISP_ENA];
  		Height |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VDE8)) << SHFT_VDE8;
  		Height |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VDE9)) << SHFT_VDE9;
  		Height++;
  
  		Width = (CharWidth + 1) * byMaxShiftPos;
	}
	else
	{
  		// output all pixels
  		Height = byCRTCReg[CRTC_VERT_TOT];
  		Height |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VT8)) << SHFT_VT8;
  		Height |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VT9)) << SHFT_VT9;
		Height += 2;
  
  		Width = (byCRTCReg[CRTC_HORIZ_TOT] + 5) * byMaxShiftPos;
	}

	// If horizontal retrace  - multiply height by 2
	if (byCRTCReg[CRTC_MODE] & BIT_HRS)
		Height *= 2;
  
	// If double clocked - multiply width by 2 */
	if(bySEQReg[SEQ_CLOCKING] & BIT_DC) 
   	Width *=2;

	// Write out file header including colormap derived from DAC
	if (WriteBMP8Header (&scanfile, szFilename, Width, Height))
		return (-1);

  	if ((!(byCRTCReg[CRTC_MODE] & BIT_RST)) ||
      (!(bySEQReg[SEQ_RESET] & BIT_SEQ_SR)) ||
      (!(bySEQReg[SEQ_RESET] & BIT_ASR))) 
	{
		// Screen disabled, scan the screen in black
		for (wVCount = 0; wVCount < (WORD)Height; wVCount++)
      	for (byHCount = 0; byHCount < (BYTE)Width; byHCount++)
				WriteBMP8Color (scanfile, 0x00);

    	// Close output file
    	CloseBMP8 (scanfile);
    
    	return (0);
	}

	// This can be reset in mid scan - initialize once per frame
	if(byGlobalContinous > 1)
	{
		// Use "previous" values to simulate signals being
		//   being updated at start of vertical retrace time
		switch(byGlobalFrameDelay) {
		case 1:
			chCurPixPan = chGlobalLastPixPan[0];
			chCurBytePan = chGlobalLastBytePan[0];
			wCurAddr = wGlobalLastCurAddr[0];
			byRowCount = byGlobalLastRowCount[0];

	 		wGlobalLastLineCompare = byCRTCReg[CRTC_LINE_CMP];
			wGlobalLastLineCompare |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_LC8)) << SHFT_LC8;
			wGlobalLastLineCompare |= ((int)(byCRTCReg[CRTC_MAX_SCAN_LINE] & BIT_LC9)) << SHFT_LC9;
			break;
		case 2:
			chCurPixPan = chGlobalLastPixPan[1];
			chCurBytePan = chGlobalLastBytePan[1];
			wCurAddr = wGlobalLastCurAddr[1];
			byRowCount = byGlobalLastRowCount[1];
         wLineCompare = wGlobalLastLineCompare;

			chGlobalLastPixPan[1] = chGlobalLastPixPan[0];
			chGlobalLastBytePan[1] = chGlobalLastBytePan[0];
			wGlobalLastCurAddr[1] = wGlobalLastCurAddr[0];
			byGlobalLastRowCount[1] = byGlobalLastRowCount[0];

			break;
      default:
			break;
		}

		chGlobalLastPixPan[0] = byATCReg[ATC_PEL_PAN] & FLD_HPP;
		chGlobalLastBytePan[0] = (byCRTCReg[CRTC_PRESET_ROW] & FLD_BP) >> SHFT_BP;
		wGlobalLastCurAddr[0] = ((WORD)byCRTCReg[CRTC_START_LOW])|
  									(((WORD)byCRTCReg[CRTC_START_HIGH]) << 8);
	   byGlobalLastRowCount[0] = byCRTCReg[CRTC_PRESET_ROW] & FLD_SRS; 
 		wGlobalLastLineCompare = byCRTCReg[CRTC_LINE_CMP];
		wGlobalLastLineCompare |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_LC8)) << SHFT_LC8;
		wGlobalLastLineCompare |= ((int)(byCRTCReg[CRTC_MAX_SCAN_LINE] & BIT_LC9)) << SHFT_LC9;
	}
	else
	{
		// Setup initial continous frame
		if(byGlobalContinous == 1)
			byGlobalContinous++;

		// Use current values 
		chCurPixPan = byATCReg[ATC_PEL_PAN] & FLD_HPP;
		chCurBytePan = (byCRTCReg[CRTC_PRESET_ROW] & FLD_BP) >> SHFT_BP;
		wCurAddr = ((WORD)byCRTCReg[CRTC_START_LOW])|
  					  (((WORD)byCRTCReg[CRTC_START_HIGH]) << 8);
	   byRowCount = byCRTCReg[CRTC_PRESET_ROW] & FLD_SRS; 

 		wLineCompare = byCRTCReg[CRTC_LINE_CMP];
		wLineCompare |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_LC8)) << SHFT_LC8;
		wLineCompare |= ((int)(byCRTCReg[CRTC_MAX_SCAN_LINE] & BIT_LC9)) << SHFT_LC9;

		chGlobalLastPixPan[0] = chCurPixPan;
		chGlobalLastPixPan[1] = chCurPixPan;
		chGlobalLastBytePan[0] = chCurBytePan;
		chGlobalLastBytePan[1] = chCurBytePan;
		wGlobalLastCurAddr[0] = wCurAddr;
		wGlobalLastCurAddr[1] = wCurAddr;
		byGlobalLastRowCount[0] = byRowCount;
		byGlobalLastRowCount[1] = byRowCount;
		wGlobalLastLineCompare = wLineCompare;
	}
	
	// Panning is different 9 bit characters - graphics is always 
	//	an 8 bit character.
	if (!(bySEQReg[SEQ_CLOCKING] & BIT_D89)) 
	{
		// 9-Bit
  		chCurPixPan += 1;
  		if (chCurPixPan > 8)
    		chCurPixPan = 0;
	}
	else
	{
		// 8-Bit
		if (chCurPixPan > 7)
			chCurPixPan = -1;
	}

	// Initialization for vload 2/4
  	// VLoad 4
	if(bySEQReg[SEQ_CLOCKING] & BIT_SH4) 
	{
	} 
  	// VLoad 2
	else if (bySEQReg[SEQ_CLOCKING] & BIT_SL) 
  		wCurAddr += chCurBytePan >> 1;
  	// Byte panning
	else
  		wCurAddr += chCurBytePan;

	// Save initial start of scan line
	wSaveAddr = wCurAddr;

	//	These can be changed on the fly but are updated only here for the simulation
	// unpack the values for values scattered about multiple 8 bit registers
	// Vertical display enable
	wVDispEnable = byCRTCReg[CRTC_VDISP_ENA];
	wVDispEnable |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VDE8)) << SHFT_VDE8;
	wVDispEnable |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VDE9)) << SHFT_VDE9;

	// Vertical total
	wVTotal = byCRTCReg[CRTC_VERT_TOT];
	wVTotal |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VT8)) << SHFT_VT8;
	wVTotal |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VT9)) << SHFT_VT9;

	// Start vertical blanking
	wStartVBlank = byCRTCReg[CRTC_START_VBLANK];
	wStartVBlank |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VBS8)) << SHFT_VBS8;
	wStartVBlank |= ((int)(byCRTCReg[CRTC_MAX_SCAN_LINE] & BIT_VBS9)) << SHFT_VBS9;

	// Start vertical retrace
	wStartVRetrace = byCRTCReg[CRTC_START_VRET];
	wStartVRetrace |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VRS8)) << SHFT_VRS8;
	wStartVRetrace |= ((int)(byCRTCReg[CRTC_OVERFLOW] & BIT_VRS9)) << SHFT_VRS9;

	// End horizontal blanking
	byEndHBlank = byCRTCReg[CRTC_END_HBLANK] & FLD_EB;
	byEndHBlank |= (byCRTCReg[CRTC_END_HRET] & BIT_EB5) >> SHFT_EB5;

	// Start scan in upper left corner of displayed pixels
	wVCount = 0;
	byVDisplayEnable = 1;
	byEarlyVDisplayEnable = 1;
	byVBlank = 0;
	byVRetrace = 0;

	// Initialize double scan counter
	byDoubleScan = 0;

	// Initialize horizontal retrace select counter
	byHorizRetrace = 0;

	// Initialize skew counters to skew not active
	chGlobalCursorSkew = -1;
	chGlobalDispStartSkew = -1;
	chGlobalDispEndSkew = -1;
	chGlobalRetStartSkew = -1;
	chGlobalRetEndSkew = -1;
	chGlobalHBStartSkew = -1;
	chGlobalHBEndSkew = -1;
	return (0);
}	

//
//		FramePrescan - Handle any wrapped signals from previous frame 
//
//		Entry:	None
//		Exit:		None
//
void FramePrescan ()
{
	byHCount = 0;

	// Repeat for all pixels in scanline
	while (byHCount < (byCRTCReg[CRTC_HORIZ_TOT] + 5))
	{
      // Start horizontal blanking
      if (byHCount == byCRTCReg[CRTC_START_HBLANK])
		{
			if ((byATCReg[ATC_MODE] & BIT_G) == 0)
		      chGlobalHBStartSkew = 1;
			else
				byHBlank = 1;
		}

		if (chGlobalHBStartSkew >= 0)
		{
			if (chGlobalHBStartSkew-- == 0)
				byHBlank = 1;
		}

      // End horizontal blanking
		if (byHBlank)
		{
			if ((byEndHBlank & CMP_EB) == (byHCount & CMP_EB))
			{
				if ((byATCReg[ATC_MODE] & BIT_G) == 0)
			      chGlobalHBEndSkew = 1;
				else
					byHBlank = 0;
			}

			if (chGlobalHBEndSkew >= 0)
			{
				if (chGlobalHBEndSkew-- == 0)
					byHBlank = 0;
			}
      }

      // Start horizontal retrace
      if(byHCount == byCRTCReg[CRTC_START_HRET])
			chGlobalRetStartSkew = (byCRTCReg[CRTC_END_HRET] & FLD_HRD) >> SHFT_HRD;

		if (chGlobalRetStartSkew >= 0)
		{
			if (chGlobalRetStartSkew-- == 0)
	  			byHRetrace = 1;
		}

      // End horizontal retrace
		if (byHRetrace)
		{
			if((byCRTCReg[CRTC_END_HRET] & FLD_EHR) == (byHCount & FLD_EHR))
	  			chGlobalRetEndSkew = (byCRTCReg[CRTC_END_HRET] & FLD_HRD) >> SHFT_HRD;
		}

		// RESET HRET AT END OF SCAN LINE
      if(byHCount == byCRTCReg[CRTC_HORIZ_TOT]+2)
		{
			if(chGlobalRetEndSkew < 0)
				chGlobalRetEndSkew = (byCRTCReg[CRTC_END_HRET] & FLD_HRD)
											 >> SHFT_HRD;
		}

      if (chGlobalRetEndSkew >= 0)
		{
			if (chGlobalRetEndSkew-- == 0)
	  			byHRetrace = 0;
		}

		// Bump horizontal character counter
      byHCount++;
    }
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
