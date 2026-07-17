//
//		IO.CPP - Routines to read and write VGA registers
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	5/2/97
//
//		Routines in this file:
//		InternalIOByteWrite		Write VGA registers
//		InternalIOByteRead		Reads VGA registers
//		AdapterENA					Evaluate registers and mode and determine if adapter enabled
//
#include	"vgaint.h"

// Arrays to handle reserved bits
BYTE byCRTCAddrMask = 0xBF;
BYTE byCRTCRegMask[25] = {
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0x7F,
	0xFF,
	0x3F,
	0x7F,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0x7F,
	0xFF,
	0xFF,
	0xEF,
	0xFF
			};

BYTE byATCAddrMask = 0x3F;
BYTE byATCRegMask[21]= {
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0x3F,
	0xEF,
	0xFF,
	0x3F,
	0x0F,
	0x0F
			};

BYTE bySEQAddrMask = 0x7;
BYTE bySEQRegMask[5] = {
	0x03,
	0x3D,
	0x0F,
	0x3F,
	0x0E
	};

BYTE byGDCAddrMask = 0xF;
BYTE byGDCRegMask[9] = {
	0x0F,
	0x0F,
	0x0F,
	0x1F,
	0xF3,
	0xFB,
	0x0F,
	0x0F,
	0xFF
	};

BYTE byMiscRegMask = 0xEF;
BYTE byFeatControlMask = 0x3B; 

//
//		InternalIOByteWrite - Write VGA registers
//
//		Entry:	wAddress		I/O address
// 				bData			Register data to write
//		Exit:		None
//
//
void InternalIOByteWrite (WORD wAddress, BYTE bData)
{
	if (AdapterENA ())
	{
		// Adapter enabled
		switch (wAddress)
		{
			case CRTC_MINDEX:

				// Color addresses - ignore
				if (byMiscReg & BIT_IOS)
				{
				}
				// Monochrome addresses
				else
					byCRTCAddr = bData & byCRTCAddrMask;
				break;

			case CRTC_MDATA:

				// Color addresses - ignore
				if (byMiscReg & BIT_IOS)
				{
				}
				else
				{
					// Monochrome addresses
					if (byCRTCAddr <= 7)
					{
						// Protected ?
						if (byCRTCReg[CRTC_END_VRET] & BIT_PR)
						{
							// Only update line compare bit 8
							if (byCRTCAddr == 7)
								byCRTCReg[CRTC_OVERFLOW] = (byCRTCReg[CRTC_OVERFLOW] & 
																  (~BIT_LC8)) | (bData & BIT_LC8);
 						}
						// Not protected
						else
							byCRTCReg[byCRTCAddr] = bData & byCRTCRegMask[byCRTCAddr];
					}
					// In range - write it
					else if (byCRTCAddr <= 24)
						byCRTCReg[byCRTCAddr] = bData & byCRTCRegMask[byCRTCAddr];
				}
				break;

			case CRTC_CINDEX:

				// Color addresses
				if (byMiscReg & BIT_IOS)
					byCRTCAddr = bData & byCRTCAddrMask;
				// Monochrome addresses - ignore
				else
				{
				}
				break;

			case CRTC_CDATA:

				if (byMiscReg & BIT_IOS)
				{
					// Color addresses
					if (byCRTCAddr <= 7)
					{
						// Protected ?
						if (byCRTCReg[CRTC_END_VRET] & BIT_PR)
						{
							// Only update line compare bit 8
							if (byCRTCAddr == 7)
								byCRTCReg[CRTC_OVERFLOW] = (byCRTCReg[CRTC_OVERFLOW] & 
								                          (~BIT_LC8)) | (bData & BIT_LC8);
 						}
						// Not protected
						else
							byCRTCReg[byCRTCAddr] = bData & byCRTCRegMask[byCRTCAddr];
					}
					// In range - write it
					else if (byCRTCAddr <= 24)
						byCRTCReg[byCRTCAddr] = bData & byCRTCRegMask[byCRTCAddr];
					
				}
				// Monochrome addresses - ignore
				else
				{
				}
      		break;

			case ATC_INDEX:

			if (byATCState == ATC_ADDRESS)
			{
				// Get attribute register address and toggle state to data
				byATCState = ATC_DATA;
				byATCAddr = bData & byATCAddrMask;
			}
			else
			{
				// Get attribute register data and toggle state to address
				byATCState = ATC_ADDRESS;
				if ((byATCAddr & FLD_ATC_ADDR) < 16)
				{
					// IPAS bit affects palette
					if (!(byATCAddr & BIT_IPAS))
						byATCReg[byATCAddr & FLD_ATC_ADDR] = bData & byATCRegMask[byATCAddr & FLD_ATC_ADDR];	 
				}
				else if ((byATCAddr & FLD_ATC_ADDR) <= 20)
					byATCReg[byATCAddr & FLD_ATC_ADDR] = bData & byATCRegMask[byATCAddr & FLD_ATC_ADDR];
      	}
			break;

			case SEQ_INDEX:

      		// Save sequencer address
				bySEQAddr = bData & bySEQAddrMask;
				break;

			case SEQ_DATA:
		
				// Save sequencer data
				if (bySEQAddr <= 4)
					bySEQReg[bySEQAddr] = bData & bySEQRegMask[bySEQAddr];
				break;

			case DAC_MASK:

      		// Update dac mask - normally 0xFF
				byDACMask = bData;
				break;

			case DAC_RINDEX:
			
				// Point to dac register to read
				byDACIndex = bData;

				// Set into read mode
				byDACState = DAC_READ;

				// Start reading red channel
				byDACColor = DAC_RED;

				// Read loads r,g,b to accumulator register
				dwDACAccum = dwDACReg[byDACIndex++];
				break;

			case DAC_WINDEX:
		
				// Point to register to write
				byDACIndex = bData;
			
				// Set into write mode
				byDACState = DAC_WRITE;
			
				// Start writing red channel
				byDACColor = DAC_RED;
				break;

			case DAC_DATA:
			
				// Are we in write mode ?
				if (byDACState == DAC_WRITE)
				{
					switch (byDACColor)
					{
						case DAC_RED:

							// Load color to accumulator
							dwDACAccum = (dwDACAccum & 0x00FFFF) |
							(((DWORD)(bData & 0x3F))<<18);

							// Point to next color
							byDACColor = DAC_GRN;
							break;

						case DAC_GRN:
					
							// Load color to accumulator
							dwDACAccum = (dwDACAccum & 0xFF00FF) |
							(((DWORD)(bData & 0x3F))<<10);
						
							// Point to next color
							byDACColor = DAC_BLU;
							break;
					
						case DAC_BLU:
					
							// Load color to accumulator
							dwDACAccum = (dwDACAccum & 0xFFFF00) |
							(((DWORD)(bData & 0x3F)) << 2);

							// Point to next color
							byDACColor = DAC_RED;

							// Update dac from accumulator register
							dwDACReg[byDACIndex++] = dwDACAccum;
							break;

						default:

							// Undefined
							break;
					}
				}
				else
				{
					// Read encountered in write mode - undefined
					// but this appears to be how it really works
					dwDACAccum = (dwDACAccum & 0x00FFFF) |
					(((DWORD)(bData & 0x3F)) << 18);
					byDACColor = DAC_GRN;
	  				break;
      		}
      		break;

			case FEAT_CCONTROL:
			
				// Color addresses
				if(byMiscReg&BIT_IOS)
					byFeatControl = bData & byFeatControlMask;
				// Monochrome addresses - ignore
				else
				{
				}
				break;

			case FEAT_MCONTROL:
			
				// Color addresses - ignore
				if(byMiscReg&BIT_IOS)
				{
				}
				// Monochrome addresses
				else
					byFeatControl = bData & byFeatControlMask;
				break;
		
			case GDC_INDEX:

				// Save GDC address
				byGDCAddr = bData & byGDCAddrMask;
				break;

			case GDC_DATA:

				// Update GDC bData
				if (byGDCAddr <= 8)
					byGDCReg[byGDCAddr] = bData & byGDCRegMask[byGDCAddr];
				break;

			case MISC_OUTPUT:

      		byMiscReg = bData & byMiscRegMask;
				break;

			case PS2_SETUP: 

				// This is motherboard setup register with VGA enabled
				if (wBoardConfig == SIM_MOTHERBOARD)
					byPS2Setup = bData;
				break;

			case VGA_SETUP:

				// This is setup register with VGA enabled
				if (wBoardConfig == SIM_MOTHERBOARD)
				{
					// Not visable unless in setup
					if (byPS2Setup & BIT_SETUP)
					{
					}
					// We are in setup mode
					else
						byVGASetup = bData;
				}
				else
				{
					// We are in setup mode
					if (byAdapterEnable & BIT_ADAPT_SETUP)
						byVGASetup = bData;
					// Not visable unless in setup
					else
					{
					}
				}
				break;

			case MB_ENABLE:

				// This is motherboard enable register with VGA enabled
				if (wBoardConfig == SIM_MOTHERBOARD)
					byMBEnable = bData;
				break;

			case ADAPTER_ENABLE:
		
				// This is adapter enable register with VGA enabled
				if (wBoardConfig == SIM_ADAPTER)
					byAdapterEnable = bData;
				break;

			default:
				;
		}
	}
	else
	{
		// Adapter disabled - only check the setup registers
		switch(wAddress)
		{
    		case PS2_SETUP:
				// This is setup register for motherboard VGA
				if (wBoardConfig == SIM_MOTHERBOARD)
					byPS2Setup = bData;
				break;

			case VGA_SETUP:
			
				if (wBoardConfig == SIM_MOTHERBOARD)
				{
					// This is setup register for motherboard VGA
					// Not visable unless in setup
					if (byPS2Setup & BIT_SETUP)
					{
					}
					// We are in setup mode
					else
						byVGASetup = bData;
				}
				else
				{
					// This is setup register for adapter VGA
					// We are in setup mode
					if (byAdapterEnable & BIT_ADAPT_SETUP)
						byVGASetup = bData;
					// Not visable unless in setup
					else
					{
					}
				}
				break;
	
			case MB_ENABLE:

				// This is the enable register for motherboard VGA
				if (wBoardConfig == SIM_MOTHERBOARD)
					byMBEnable = bData;
				break;

			// This is the enable register for adapter VGA
			case ADAPTER_ENABLE:
			
				if(wBoardConfig == SIM_ADAPTER)
					byAdapterEnable = bData;
				break;

    		default:
      		;
    	}
	} 
}

//
//		InternalIOByteRead - Reads VGA registers
//
//		Entry:	wAddress		I/O address
//		Exit:		<BYTE>		Register data at address
//
BYTE InternalIOByteRead (WORD wAddress)
{

	if(AdapterENA())
	{
		// Adapter enabled
		switch(wAddress)
		{

			case CRTC_MINDEX:

				// Save CRTC address
				// Color addresses - ignore
				if (byMiscReg & BIT_IOS)
					return (OPEN_BUS);
				else
					return (byCRTCAddr);

			case CRTC_MDATA:
			
				// Save CRTC data
				// Color addresses - ignore
				if (byMiscReg & BIT_IOS)
					return (OPEN_BUS);
				else
				{
					// Monochrome addresses
					if (byCRTCAddr <= 24)
					{
						// In range? - read it
						// Fake lightpen undocumented functionality
						if (byCRTCAddr == CRTC_START_VRET)
						{
							if (byCRTCReg[CRTC_END_HBLANK] & BIT_RDCOMPAT)
								return (byCRTCReg[byCRTCAddr]);
							else
	      					// This would return MSB of memory address counter
								// since the simulator is not scanning - fake it with 0
								return(0);
						}
						else if (byCRTCAddr == CRTC_END_VRET)
						{
							if(byCRTCReg[CRTC_END_HBLANK] & BIT_RDCOMPAT)
								return (byCRTCReg[byCRTCAddr]);
							else
								// This would return LSB of memory address counter
								// since the simulator is not scanning - fake it with 0
								return (0);
						}
						else
							return (byCRTCReg[byCRTCAddr]);

					}
	  				// Undocumented registers
					else if (byCRTCAddr == CRTC_LATCH)
					{
						// Call selected read mode
						if (byGDCReg[GDC_MODE] & BIT_RM)
							return(ReadMode1(CRTC_LATCH,1));
						else
							return(ReadMode0(CRTC_LATCH,1));
				  	}
					else if (byCRTCAddr == CRTC_TOGGLE)
					{
						if(byATCState == ATC_ADDRESS)
							return (0x0);
						else
							return (BIT_ATC_TOGGLE);
					}
					else if (byCRTCAddr == CRTC_ATC)
					{
						return (byATCAddr);
					}
					else
					  return (OPEN_BUS);
      		}

			case CRTC_CINDEX:

				// Save CRTC address
				if (byMiscReg & BIT_IOS)
					// Color addresses
					return (byCRTCAddr);
				else
					// Monochrome addresses - ignore
					return (OPEN_BUS);
			
			case CRTC_CDATA:

				// Save CRTC data
				if (byMiscReg & BIT_IOS)
				{
					// Color addresses
					if (byCRTCAddr <= 24)
					{
						// In range? - read it
						// Fake lightpen undocumented functionality
						if (byCRTCAddr == CRTC_START_VRET)
						{
							if(byCRTCReg[CRTC_END_HBLANK] & BIT_RDCOMPAT)
								return(byCRTCReg[byCRTCAddr]);
							else
								// This would return MSB of memory address counter
								// since the simulator is not scanning - fake it with 0
								return(0);
						}
						else if (byCRTCAddr == CRTC_END_VRET)
						{
							if (byCRTCReg[CRTC_END_HBLANK] & BIT_RDCOMPAT)
								return(byCRTCReg[byCRTCAddr]);
							else
								// This would return LSB of memory address counter
								// since the simulator is not scanning - fake it with 0
								return (0);
						}
						else
							return (byCRTCReg[byCRTCAddr]);
					
					// Undocumented registers
					}
					else if (byCRTCAddr == CRTC_LATCH)
					{
						// Call selected read mode
						if (byGDCReg[GDC_MODE] & BIT_RM)
							return (ReadMode1 (CRTC_LATCH, 1));
						else
							return (ReadMode0 (CRTC_LATCH, 1));
					}
					else if (byCRTCAddr == CRTC_TOGGLE)
					{
						if(byATCState == ATC_ADDRESS)
							return (0x0);
						else
							return (BIT_ATC_TOGGLE);
					}
					else if (byCRTCAddr == CRTC_ATC)
					{
						return (byATCAddr);
					}
					else
						return (OPEN_BUS);
				}
				else
					// Monochrome addresses - ignore
					return (OPEN_BUS);

			case ATC_INDEX:

				// Read attribute address
				return (byATCAddr);

			case ATC_RDATA:

				// Read attribute data
				if ((byATCAddr & FLD_ATC_ADDR) < 16)
				{
					// IPAS bit affects palette
					if (!(byATCAddr & BIT_IPAS))
						return (byATCReg[byATCAddr & FLD_ATC_ADDR]);
					else
						return (OPEN_BUS);
				}
				else if((byATCAddr & FLD_ATC_ADDR) <= 20)
					return (byATCReg[byATCAddr & FLD_ATC_ADDR]);
				else
					return (OPEN_BUS);

			case SEQ_INDEX:

				// Read sequencer address
				return (bySEQAddr);

			case SEQ_DATA:

				// Read sequencer data
				if (bySEQAddr <= 4)
					return (bySEQReg[bySEQAddr]);
				else
					return (OPEN_BUS);
				
			case DAC_MASK:

				// Read dac mask - usually 0xFF
				return (byDACMask);

			case DAC_RINDEX:

				// This actually returns the DAC state
				// DAC state is read or write in progress
				return (byDACState);

			case DAC_WINDEX:

				// Read DAC write address
				return (byDACIndex);
			
			case DAC_DATA:
				
				if (byDACState == DAC_READ)
				{
					switch (byDACColor)
					{
						case DAC_RED:
							
							// Point to next color
							byDACColor = DAC_GRN;
							
							// Return color from accumulator register
							return ((unsigned char)((dwDACAccum >> 18) & 0x3F));
					
						case DAC_GRN:
							
							// Point to next color
							byDACColor = DAC_BLU;

							// Return color from accumulator register
							return ((unsigned char)((dwDACAccum >> 10) & 0x3F));
						
						case DAC_BLU:

							{
								unsigned char val;
								// Return color from accumulator register
								val = (unsigned char)((dwDACAccum >> 2) & 0x3F);
								// Point to next color
								byDACColor = DAC_RED;
								// Load accumulator register with next color
								dwDACAccum = dwDACReg[byDACIndex++];
								return (val);
							}

						default:
							return (OPEN_BUS);
					}
				}
				else
				{
					// Read in write mode - undefined
					// Although this is how it appears to work
					byDACColor = DAC_GRN;
					return ((unsigned char)((dwDACAccum >> 18) & 0x3F));
				}
				
			case FEAT_CONTROLR:
		 
				return (byFeatControl);
				
			case GDC_INDEX:
			
				// Get GDC address
				return (byGDCAddr);
			
			case GDC_DATA:

				// Get GDC data
				if (byGDCAddr <= 8)
					return (byGDCReg[byGDCAddr]);
				else
					return (OPEN_BUS);
			
			case INPUT_RSTATUS_0:
				
				// Neither interrupts nor switch sense are implemented -
				// always return 0
				return (0x00);
			
			case INPUT_MSTATUS_1:

				// Color addresses - ignore
				if(byMiscReg&BIT_IOS)
					return (OPEN_BUS);
				else
				{
					// Monochrome addresses - reset attribute controller addressing
					byATCState = ATC_ADDRESS;
					// Some applications monitor these bits for timing information
					// that the simulator cannot provide.  This is handled in the
					// following manner: 
					//			BIT_VR (vertical retrace) - this bit is toggled on every read
					// 		BIT_DE (display not enabled) - this bit is set to 1 which
					//			implies that the hardware is always in blanking
					byInputStatus1 ^= BIT_VR;
					return ((byInputStatus1 & BIT_VR) | BIT_DE);
				}

			case INPUT_CSTATUS_1:
				
				if (byMiscReg & BIT_IOS)
				{
					// Color addresses
					// Reset attribute controller addressing
					byATCState = ATC_ADDRESS;
					// Some applications monitor these bits for timing information
					// that the simulator cannot provide.  This is simulated in the
					// following manner:
					//				BIT_VR (vertical retrace) - this bit is toggled on every
					//				read
					//				BIT_DE (display not enabled) - this bit is set to 1 which
					//				implies that the hardware is always in blanking
					//
					byInputStatus1 ^= BIT_VR;
					return ((byInputStatus1 & BIT_VR) | BIT_DE);
				}
				else
					// Monochrome addresses - ignore
					return (OPEN_BUS);
				
			case MISC_INPUT:
			
				return (byMiscReg);

			case PS2_SETUP:

				// This is motherboard setup register with VGA enabled
				if (wBoardConfig == SIM_MOTHERBOARD)
					return (byPS2Setup);
				else
					return (OPEN_BUS);
				
			case VGA_SETUP:
				
				if (wBoardConfig == SIM_MOTHERBOARD)
				{
					// This is motherboard setup register with VGA enabled
					if (byPS2Setup & BIT_SETUP)
						// Not visable unless in setup
						return (OPEN_BUS);
					else
						// We are in setup mode
						return(byVGASetup);
				}
				else
				{
					// This is adapter setup register with VGA enabled
					if (byAdapterEnable & BIT_ADAPT_SETUP)
						// We are in setup mode
						return (byVGASetup);
					else
						// Not visable unless in setup
						return(OPEN_BUS);
				}
			
				
			case MB_ENABLE:

				// This is motherboard enable register with VGA enabled
				if (wBoardConfig == SIM_MOTHERBOARD)
					return (byMBEnable);
				else
					return (OPEN_BUS);
				
			case ADAPTER_ENABLE:

				// This is adapter enable register with VGA enabled
				if (wBoardConfig == SIM_ADAPTER)
					return (byAdapterEnable);
				else
					return(OPEN_BUS);


			default:

				// No VGA registers specified - return open bus
				return (OPEN_BUS);
		}
	}
	else
	{
		// Adapter disabled - only check the setup registers
		switch (wAddress)
		{
			case PS2_SETUP:

				// This is setup register for motherboard VGA
				if (wBoardConfig == SIM_MOTHERBOARD)
					return(byPS2Setup);
				else
					return(OPEN_BUS);

			case VGA_SETUP:

				if (wBoardConfig == SIM_MOTHERBOARD)
				{
					// This is setup register for motherboard VGA
					if (byPS2Setup & BIT_SETUP)
						// Not visable unless in setup
						return (OPEN_BUS);
					else
						// We are in setup mode
						return (byVGASetup);
				}
				else
				{
					// This is setup register for adapter VGA
					if(byAdapterEnable & BIT_ADAPT_SETUP)
						// We are in setup mode
						return(byVGASetup);
					else
						// Not visable unless in setup
						return(OPEN_BUS);
				}

			case MB_ENABLE:

				// This is the enable register for motherboard VGA
				if (wBoardConfig == SIM_MOTHERBOARD)
					return(byMBEnable);
				else
					return(OPEN_BUS);
				 
			default:
				// No VGA registers specified - return open bus
				return (OPEN_BUS);
		}
	}
}


//
//		AdapterENA - Evaluate registers and mode and determine if adapter enabled
//
//		Entry:	None
// 	Exit:		<BOOL>	FALSE = If not enabled, TRUE = Nonzero if enabled
//
//
BYTE AdapterENA ()
{
	switch (wBoardConfig)
	{
		case SIM_MOTHERBOARD:
		
			// Enable test for motherboard
			return ((byVGASetup & BIT_VGAENA) && (byMBEnable & BIT_MBENA) &&
					 (byPS2Setup & BIT_SETUP));

		case SIM_ADAPTER:

			// Enable test for adapter
			return ((byVGASetup & BIT_VGAENA) && (byAdapterEnable & BIT_ADAPT_ENA) &&
					 ((~byAdapterEnable) & BIT_ADAPT_SETUP));
		
		default:

			// Unknown configuration - return failure
			return (0);
	}
}

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

