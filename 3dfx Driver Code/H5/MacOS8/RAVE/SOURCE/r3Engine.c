//: r3Engine.c
//: alt.drivers inc.
//: Glenn Nissen
//: Vadim Kochubievski
//: Rohit Gundecha

#include <Processes.h>
#include "r3Core.h"

// global vars at the engine level
extern 	UInt32 		rvDeviceListCount;

Boolean bNanosaurRunning 	= false;
Boolean bBugdomRunning		= false;
Boolean bQuakeRunning		= false;
Boolean bATVRunning			= false;
Boolean bWaterRaceRunning 	= false;
Boolean bNightfallRunning 	= false;
Boolean bCombatRunning		= false;
Boolean bDVRunning			= false;
Boolean bFlyRunning			= false;
Boolean bUTRunning			= false;

float	gSTWfactor;	// applied to s, t, w in vertex macros; depends on QD3D vs RAVE

static const long tnslVendorID = kQAVendor_3Dfx;
static const long tnslEngineID = kQAEngine_3DfxVoodooVSA100; // was kQAEngine_3DfxSST1;

static const long tnslRevisionID = 0x00000111;
static const char atiCardEngineName[] = "ATI 3DRage QD3D Rave Engine";
static char driverName[20] = "";

extern TRvInfo gRvEngInfo;


/////////////////////////////
//                         //
//  RvEngineDeviceCheck  //
//                         //
/////////////////////////////

TQAError 
RvEngineDeviceCheck(const TQADevice* device)
{
	GDHandle	dev;
	TQAError err = kQANoErr;

	DebugStr("--> RvEngineDeviceCheck - ");

	rvDeviceListCount = ValidateGlideEnvironment();

	switch(device->deviceType) {
	case kQADeviceMemory:
		DebugStr("Device Memory kQANotSupported");
		err = kQANotSupported;
		break;

	case kQADeviceGDevice:
		DebugStr("kQADeviceGDevice");
		
        {
        	int i;
        	long	cDepth = (*(*device->device.gDevice)->gdPMap)->pixelSize;
			err = (cDepth == 16 || cDepth == 32) ? kQANoErr : kQADisplayModeUnsupported;
			if (err == kQANoErr)
			{
				dev = device->device.gDevice;
				for ( i = 0;  i < rvDeviceListCount; i++)
				{
					if ( (GLint)rvDeviceList[i].systemDeviceId == (GLint)(**dev).gdRefNum)
					{
						DebugStr("\nTQADevice OK in list\n");
						return kQANoErr;
					}
				}
				err = kQANotSupported;
			}
			else
			{
				DebugStr("\ndevice not in 16 or 32 bit color \n");
			}
		}
		
		break;

	default:
		DebugStr("*** unexpected device type");
		err = kQAError;
		break;
	}

bail:
	DebugStr("\n");
	return err;
}

static void SetAllAppFlagsFalse(void)
{
	bWaterRaceRunning	= false;
	bQuakeRunning		= false;
	bBugdomRunning 		= false;
	bNanosaurRunning 	= false;
	bCombatRunning		= false;
	bATVRunning			= false;
	bNightfallRunning	= false;
	bDVRunning			= false;
	bFlyRunning			= false;
	bUTRunning			= false;
}

/////////////////////////
//                     //
//  RvEngineGestalt  //
//                     //
/////////////////////////

TQAError 
RvEngineGestalt(TQAGestaltSelector selector,
                  void*              response)
{
	TQAError err = kQANoErr;

	unsigned long ii;
	OSErr	stat;
	long tmp, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9;
	ProcessSerialNumber	psn;
	ProcessInfoRec	pir;
	const int kATIGestalt_CurrentContext = 1000;
	Str255	procname;
	FourCharCode	kNanosaurSig 	= 'NanO';
	FourCharCode	kBugdomSig	 	= 'BalZ';
	FourCharCode	kQuakeSig	 	= 'Quak';
	FourCharCode	kATVKawasakiSig	= 'MBka';
	FourCharCode	kWaterRaceSig	= 'WrRc';
	FourCharCode	kNightfallSig	= 'NgtF';
	FourCharCode	kCombatSig		= 'CtM0';	// Combat Mission demo
	FourCharCode	kDVSig			= 'DaVe';	// Dark Vengeance
	FourCharCode	kFlySig			= 'MFly';	// Fly!2K
	FourCharCode	kUTSig			= 'UnTn';	// Unreal Tournament
	
	const float		kRaveSTW		= 1.0;
	const float		kQD3DSTW		= 0.01;
    FSSpec appFSSpec;

	DebugStr("--> RvEngineGestalt - ");

	/*
	if(selector >= kQAGestalt_NumSelectors) 
	{
		DebugStr("*** unexpected selector");
		err = kQAGestaltUnknown;
		goto bail;
	}
	*/
	if(selector > kATIGestalt_CurrentContext) 
	{
		DebugStr("*** unexpected selector");
		DebugNum((UInt32)selector);
		err = kQAGestaltUnknown;
		goto bail;
	}

	tmp1 = bBugdomRunning;
	tmp2 = bNanosaurRunning;
	tmp3 = bQuakeRunning;
	tmp4 = bATVRunning;
	tmp5 = bWaterRaceRunning;
	tmp6 = bNightfallRunning;
	tmp7 = bCombatRunning;
	tmp8 = bDVRunning;
	tmp9 = bFlyRunning;
	
	// Some QD3D/Rave games are limited if driver doesn't report itself as "ATI".
	if ( (tmp1 == 0) && (tmp2 == 0 ) && (tmp3 == 0) && (tmp4 == 0) && (tmp5 == 0) && (tmp6 == 0) &&
		 (tmp7 == 0) && (tmp8 == 0) && (tmp9 == 0) &&
		(selector == kQAGestalt_VendorID) ||
	    (selector == kQAGestalt_EngineID) ||
	    (selector == kQAGestalt_Revision) ||
	    (selector == kQAGestalt_ASCIINameLength) ||
	    (selector == kQAGestalt_ASCIIName))
	{
		psn.highLongOfPSN = 0;
		psn.lowLongOfPSN = kNoProcess;
		pir.processInfoLength	        = sizeof(ProcessInfoRec);
		pir.processName		        	= procname;
		pir.processAppSpec             = &appFSSpec;
		stat = GetCurrentProcess( &psn );
		if (stat == noErr)
			if (noErr == GetProcessInformation( &psn, &pir ) )
				if ( (unsigned long)kNanosaurSig == *(unsigned long *)&pir.processSignature ) 
				{
					SetAllAppFlagsFalse();
					bNanosaurRunning 	= true;
					gSTWfactor			= kQD3DSTW;
				}					
				else if ( (unsigned long)kBugdomSig == *(unsigned long *)&pir.processSignature )
				{
					SetAllAppFlagsFalse();
					bBugdomRunning 		= true;
					gSTWfactor			= kQD3DSTW;
				}
				else if ( (unsigned long)kWaterRaceSig == *(unsigned long *)&pir.processSignature )
				{
					SetAllAppFlagsFalse();
					bWaterRaceRunning	= true;
					gSTWfactor			= kRaveSTW;
				}
				else if ( (unsigned long)kNightfallSig == *(unsigned long *)&pir.processSignature )
				{
					SetAllAppFlagsFalse();
					bNightfallRunning	= true;
					// neither of two defaults seem right for Nightfall
					gSTWfactor			= 0.0001;	// trial-and-error
				}
				else if ( (unsigned long)kDVSig == *(unsigned long *)&pir.processSignature )
				{
					SetAllAppFlagsFalse();
					bDVRunning			= true;
					gSTWfactor			= kRaveSTW;
				}
				else if ( (unsigned long)kCombatSig == *(unsigned long *)&pir.processSignature )
				{
					SetAllAppFlagsFalse();
					bCombatRunning		= true;
					gSTWfactor			= kRaveSTW;
				}
				else if ( (unsigned long)kQuakeSig == *(unsigned long *)&pir.processSignature )
				{
					SetAllAppFlagsFalse();
					bQuakeRunning		= true;
					gSTWfactor			= kRaveSTW;
				}
				else if ( (unsigned long)kATVKawasakiSig == *(unsigned long *)&pir.processSignature )
				{
					SetAllAppFlagsFalse();
					bATVRunning			= true;
					gSTWfactor			= kRaveSTW;
				}
				else if ( (unsigned long)kFlySig == *(unsigned long *)&pir.processSignature )
				{
					SetAllAppFlagsFalse();
					bFlyRunning			= true;
					gSTWfactor			= kRaveSTW;
				}
				else if ( (unsigned long)kUTSig == *(unsigned long *)&pir.processSignature )
				{
					SetAllAppFlagsFalse();
					bUTRunning			= true;
					gSTWfactor			= kRaveSTW;
				}
				else
				{
					SetAllAppFlagsFalse();
					gSTWfactor			= kQD3DSTW;
				}
	}

	switch(selector) {
	case kQAGestalt_OptionalFeatures:
		DebugStr("OptionalFeatures ");
		*(unsigned long*) response = (	kQAOptional_None
									| kQAOptional_DeepZ
                                    | kQAOptional_Texture
                                    | kQAOptional_TextureHQ
                                    | kQAOptional_TextureColor
		                            | kQAOptional_Blend
                                    | kQAOptional_BlendAlpha
									| kQAOptional_Antialias
                                    | kQAOptional_ZSorted
									| kQAOptional_BufferComposite
									// | kQAOptional_MultiTextures  // try Fly
									// | kQAOptional_MipmapBias // try Fly
									| kQAOptional_FogDepth
                                    | kQAOptional_OpenGL
                                    | kQAOptional_NoDither
                                    | kQAOptional_AlphaTest
                                    // | kQAOptional_AccessTexture	// try Fly
                                    // | kQAOptional_AccessBitmap	// try Fly
                                    | kQAOptional_AccessDrawBuffer
									| kQAOptional_FogAlpha	// try Fly
                                    | kQAOptional_AccessZBuffer
                                    | kQAOptional_ClearDrawBuffer
                                    | kQAOptional_ClearZBuffer
									| kQAOptional_CL4	
									| kQAOptional_CL8	
                                    | kQAOptional_BufferComposite
                                    // | kQAOptional_ChannelMask	// try Fly
                                    | kQAOptional_ZBufferMask
                                    // | kQAOptional_OffscreenDrawContexts	// try Fly
#if defined(AA_SUPPORT)
                                    | kQAOptional_Antialias
#endif /* defined(AA_SUPPORT) */
									// | kQAOptional_PerspectiveZ	// try Fly
                                    | kQAOptional_CL4
                                    | kQAOptional_CL8 );
			DebugHex( *(UInt32*) response );
		break;

	case kQAGestalt_OptionalFeatures2:	
		DebugStr("OptionalFeatures2 ");
			*(long *) response = 	kQAOptional2_Busy
                              		| kQAOptional2_Chromakey  		
									| kQAOptional2_BitmapScale
                               		| kQAOptional2_SwapBuffers
									// | kQAOptional2_NonRelocatable	// try Fly
									// | kQAOptional2_DrawContextScale // try Fly
									// | kQAOptional2_DrawContextNonRelocatable // try Fly
									// | kQAOptional2_NoCopy	// try Fly
									| kQAOptional2_FlipOrigin;
		 							// | kQAOptional2_TextureDrawContexts        // try Fly 
                                	// | kQAOptional2_BitmapDrawContexts;  // try Fly 
									// | kQAOptional2_PriorityBits
									
			break;					
	case kQAGestalt_FastFeatures:
		DebugStr("FastFeatures ");
		*(unsigned long*) response = (	  kQAFast_None
		                                | kQAFast_Line
		                                | kQAFast_Gouraud
		                                | kQAFast_Texture
		                                | kQAFast_TextureHQ
		                                | kQAFast_Blend
										| kQAFast_ZSorted	
										| kQAFast_CL4	
										| kQAFast_CL8
										| kQAFast_Antialiasing
										| kQAFast_FogDepth
										| kQAFast_FogAlpha
		                                | kQAFast_CL4
		                                | kQAFast_CL8
										| kQAFast_BitmapScale	
		                                );
		break;

	case kQAGestalt_VendorID:
		DebugStr("VendorID");
		if (bNanosaurRunning || bBugdomRunning ) 	// || bQuakeRunning)
		{
			*(long*) response = 1; 		// kQAVendor_ATI;
			DebugStr(" reporting kQAVendor_ATI ");
		}
		else
			*(long*) response = tnslVendorID;	// 17
		break;

	case kQAGestalt_EngineID:
		DebugStr("EngineID");
		if (bNanosaurRunning || bBugdomRunning ) 	// || bQuakeRunning)
			*(long*) response = 4;		// Rage Pro
		else
			*(long*) response = tnslEngineID;
		break;

	case kQAGestalt_Revision:
		DebugStr("Revision ");
		if (bNanosaurRunning || bBugdomRunning ) 	// || bQuakeRunning)
			*(long*) response = 37;		// Rage Pro v3.7
		else
			*(long*) response = tnslRevisionID;
		DebugHex(tnslRevisionID);
		break;

	case kQAGestalt_ASCIINameLength:
		DebugStr("ASCIINameLength ");
		if (bNanosaurRunning || bBugdomRunning ) 		// || bQuakeRunning)
			*(long*) response = sizeof( atiCardEngineName ) - 1;
		else
		{
			/*
			if (strcmp(driverName, "") != 0)
			{
				*(long*) response = strlen( driverName );
			}
			else
			{
				// seems that you can only make this call once
				const char *hw = grGetString( GR_HARDWARE );
				*(long*) response = strlen( hw );
			}
			*/
			*(long *)response = 12;
		}
		break;

	case kQAGestalt_ASCIIName:
		DebugStr("ASCIIName ");
		if (bNanosaurRunning || bBugdomRunning ) 	// || bQuakeRunning)
			for (ii = 0; ii < sizeof( atiCardEngineName ); ++ii)
			{
				((char*) response)[ ii ] = atiCardEngineName[ ii ];
			}
		else
		{
			char *ptm;
			if (strcmp(driverName, "") != 0)
			{
				strcpy( (char *)response, driverName );
				DebugStr("not first call, already set to "); DebugStr(driverName);
			}
			else
			{
				// const char *hw = grGetString( GR_HARDWARE );
				// DebugStr( hw );
				strcpy( (char *)response, grGetString( GR_HARDWARE ) );
				// need to filter bad chars, like "("
				
				// replace "(tm)" with "RAVE" and add \0 (same strlen)
				ptm = strstr( (char *)response, "(tm)" );
				if (ptm)
					strcpy( ptm, "RAVE\0");
				strcpy( driverName, (char *)response );
				DebugStr(" munged ASCIIName "); DebugStr( (char *)response );
			}
			/*
			// hack, because the glide call is returning "ERROR".
			strcpy( (char *)response, "Voodoo5 RAVE");
			*/
							
		}
		break;

	/*
	 * Since we don't allocate a texture surface until the first time a tex is uploaded to VRAM,
	 * the surface size may very well be zero at the time of this call. So, we report a useful, but
	 * somewhat bogus value.
	 */
	case kQAGestalt_TextureMemory:
		DebugStr("TextureMemory ");
		tmp = 0;
		grGet( GR_MEMORY_UMA, 4, &tmp );
		if (gRvEngInfo.surface_size > 0)
		{
			*(long *) response =  max(0,  gRvEngInfo.surface_size - gRvEngInfo.totalTextureSize);
			DebugNum( max(0,  gRvEngInfo.surface_size - gRvEngInfo.totalTextureSize));
		}
		else
		{
			*(long *) response =  max(0,  MAX_TEXTUREBYTES_PER_ENGINE - gRvEngInfo.totalTextureSize);
			DebugNum( max(0,  MAX_TEXTUREBYTES_PER_ENGINE - gRvEngInfo.totalTextureSize));
		}
		DebugStr("\n");
		break;
		
	case kQAGestalt_FastTextureMemory:
		DebugStr("FastTextureMemory ");
		tmp = 0;
		grGet( GR_MEMORY_UMA, 4, &tmp );
		if (gRvEngInfo.surface_size > 0)
		{
			*(long *) response =  max(0,  gRvEngInfo.surface_size - gRvEngInfo.totalTextureSize);
			DebugNum( max(0,  gRvEngInfo.surface_size - gRvEngInfo.totalTextureSize));
		}
		else
		{
			*(long *) response =  max(0,  MAX_TEXTUREBYTES_PER_ENGINE - gRvEngInfo.totalTextureSize);
			DebugNum( max(0,  MAX_TEXTUREBYTES_PER_ENGINE - gRvEngInfo.totalTextureSize));
		}
		break;

	/* returns all the draw context pixel types supported by the RAVE engine */
	case kQAGestalt_DrawContextPixelTypesAllowed:				
		DebugStr( "	kQAGestalt_DrawContextPixelTypesAllowed ");
			*((long *) response) = 	(1 << kQAPixel_ARGB16) | 
									(1 << kQAPixel_ARGB32);
		break;
		
	/* returns all the draw context pixel types that are preferred by the RAVE engine. */
	case kQAGestalt_DrawContextPixelTypesPreferred:				
		DebugStr( "	kQAGestalt_DrawContextPixelTypesPreferred ");
			*((long *) response) = 	(1 << kQAPixel_ARGB16) | 
									(1 << kQAPixel_ARGB32);
		break;
			
	//+ TBFL
	/* returns all the texture pixel types that are supported by the RAVE engine */
	case kQAGestalt_TexturePixelTypesAllowed:
		DebugStr( "	kQAGestalt_TexturePixelTypesAllowed ");
		*(long *) response =		(1 << kQAPixel_Alpha1) 		|
									(1 << kQAPixel_RGB16)		|
									(1 << kQAPixel_ARGB16)		|
									(1 << kQAPixel_RGB32)		|
									(1 << kQAPixel_ARGB32)		|
									(1 << kQAPixel_CL4)			|
									(1 << kQAPixel_CL8);
		break;
		
	case kQAGestalt_TexturePixelTypesPreferred:					/* returns all the texture pixel types that are preferred by the RAVE engine.*/
		DebugStr( "	kQAGestalt_TexturePixelTypesPreferred ");
		*(long *) response =		(1 << kQAPixel_Alpha1) 		|
									(1 << kQAPixel_RGB16)		|
									(1 << kQAPixel_ARGB16)		|
									(1 << kQAPixel_RGB32)		|
									(1 << kQAPixel_ARGB32)		|
									(1 << kQAPixel_CL4)			|
									(1 << kQAPixel_CL8);
		break;
	case kQAGestalt_BitmapPixelTypesAllowed:					/* returns all the bitmap pixel types that are supported by the RAVE engine. */
		DebugStr( "	kQAGestalt_BitmapPixelTypesAllowed ");
			*(long *) response = 	(1 << kQAPixel_Alpha1) 		|
									(1 << kQAPixel_RGB16)		|
									(1 << kQAPixel_ARGB16)		|
									(1 << kQAPixel_RGB32)		|
									(1 << kQAPixel_ARGB32)		|
									(1 << kQAPixel_CL4)			|
									(1 << kQAPixel_CL8);
			break;
	case kQAGestalt_BitmapPixelTypesPreferred:					/* returns all the bitmap pixel types that are preferred by the RAVE engine. */
		DebugStr( "	kQAGestalt_BitmapPixelTypesPreferred ");
			*(long *) response = 	(1 << kQAPixel_Alpha1) 		|
									(1 << kQAPixel_RGB16)		|
									(1 << kQAPixel_ARGB16)		|
									(1 << kQAPixel_RGB32)		|
									(1 << kQAPixel_ARGB32)		|
									(1 << kQAPixel_CL4)			|
									(1 << kQAPixel_CL8);
			break;
	case kQAGestalt_MultiTextureMax:		/* max number of multi textures supported by this engine */
		DebugStr( "	kQAGestalt_MultiTextureMax ");
        	*(long *)response = 0;			/* does not count the primary texture */
        	break;
	
	// Nanosaur is crippled without support for some ATI-specific features.
	case kATIGestalt_CurrentContext:
		DebugStr( "	kATIGestalt_CurrentContext ");
		*(long *)response = (UInt32)gRvEngInfo.currentDrawContext;
		break;
		
	default:
		DebugStr("*** unsupported selector");
		err = kQAGestaltUnknown;
		goto bail;
	}

bail:
	DebugStr("\n");
	return err;
}
