
#include <CodeFragments.h>
#include "r3Core.h"
#include "hdwr_res_mgr.h"
#include <Profiler.h>
#if TNSL_SPOTLIGHT
#include "SpotlightAPI.h"
#endif

int blockCount = 0;
int maxBlockCount = 0;


extern 	GrDeviceInfo_t 	tnslDeviceList[MAX_GLIDE_DEVICES];

Boolean	gRave16;
long gtnslInitialized = 0;
TRvInfo gRvEngInfo;

//////////////////////
//                  //
//  RvInitialize  //
//                  //
//////////////////////
OSErr 
RvInitialize(void)
{
	OSErr err = noErr;
	TQAError rErr;
	static UInt32	numTimesCalled = 0;
	static long		gEngRefCon	= 0;
	FxI32	numBoards = 0, sizeOfState = 0;
	FxU32	numBytes;
	SInt32  texMemSize = 0;
	UInt32	sizeOfGlideState = 0;
    long	tmp = 0;
	
#if TNSL_SPOTLIGHT
	SLInit();		// Spotlight
#endif

	//+ GrHwConfiguration hwconfig;
	++numTimesCalled;
	
	memset(&gRvEngInfo, 0, sizeof (gRvEngInfo));
	
#ifdef TNSL_LOG
	gRvEngInfo.pFileLog = fopen("3DfxLog", "w");
#endif

	DebugStr("--> RvInitialize - ");
	
	// first check for presence of hardware
#if TNSL_ALLOW_HRM_LINK	
	numBoards = hrmGetNumTargets();
	if (numBoards == 0)
	{
		DebugStr("\nAbort -- no 3dfx hardware\n");
#ifdef TNSL_LOG
		fclose( (FILE*)gRvEngInfo.pFileLog);
#endif
		return (OSErr)1;
	}
#endif
	
 #ifdef TNSL_LOG
	{
		long tempMem, sysMem, mem;
		tempMem = TempFreeMem();
		mem = FreeMem();
		sysMem = FreeMemSys();
		DebugStr(" freeMem="); DebugNum(mem);
		DebugStr(" freeMemSys="); DebugNum(sysMem);
		DebugStr(" tempMem="); DebugNum(tempMem);
	}
#endif
	if ( !InitializeGlide() )
    {
		grSstSelect(0);
	
		DebugStr("\n");
		
		if ((UInt32)QARegisterEngineWithRefCon == (UInt32)kUnresolvedCFragSymbolAddress)
		{
			gRave16 = false;
			if ( (rErr = QARegisterEngine(RvEngineGetMethod)) != kQANoErr) 
		    	return( (OSErr)paramErr );
		}
		else
		{
			gRave16 = true;
			if ( (rErr = QARegisterEngineWithRefCon(RvEngineGetMethod, gEngRefCon)) != kQANoErr) 
		    	return( (OSErr)paramErr );
		
		}
	    ++gEngRefCon;

		grGet( GR_MEMORY_UMA, 4, &tmp );
		gRvEngInfo.totalVRAMcard = tmp;
		DebugStr(" gRvEngInfo.totalVRAMcard ="); DebugNum(gRvEngInfo.totalVRAMcard);
		
	    
		gRvEngInfo.textureList = NULL;
		gRvEngInfo.texture_surface = NULL;
		gRvEngInfo.numDrawContexts = 0;
		
		numBytes = grGet( GR_GLIDE_STATE_SIZE, 4, &sizeOfState );
		if (numBytes) gRvEngInfo.sizeOfGlideState = sizeOfState;
	
		gtnslInitialized = 1;
	}
	#if TNSL_PROFILING
		// Profiler
		ProfilerInit(collectDetailed, bestTimeBase, 30000, 8);
	#endif
	
bail:
	if(err) 
			grGlideShutdown();
	
	return err;
}



/////////////////////
//                 //
//  RvTerminate  //
//                 //
/////////////////////

OSErr 
RvTerminate(void)
{
	DebugStr("--> RvTerminate - ");

#if TNSL_PROFILING
	ProfilerDump("\p3dfxDriverDump.dat");
	ProfilerTerm();
#endif

    if ( gtnslInitialized )
    {

		// Textures are owned at the engine level.
		// This does a Force Delete on all textures, including bitmap components. Frees VRAM, then cache.
		EradicateTextures();
	
		grSurfaceSetTextureSurfaceExt(GR_TMU0,0);
		grSurfaceSetTextureSurfaceExt(GR_TMU1,0);
		
		if(gRvEngInfo.texture_surface) 
		{
			grSurfaceReleaseExt(gRvEngInfo.texture_surface);
			gRvEngInfo.texture_surface = 0;
			gRvEngInfo.surface_size = 0;
		}
		
		grGlideShutdown();
		DebugStr("\ngrGlideShutdown\n");
    }

#ifdef TNSL_LOG
	fclose(gRvEngInfo.pFileLog);
#endif

	return noErr;
}


///////////////////////////
//                       //
//  RvEngineGetMethod  //
//                       //
///////////////////////////

TQAError 
RvEngineGetMethod(TQAEngineMethodTag methodTag,
                    TQAEngineMethod*   method)
{
	TQAError err = kQANoErr;

	DebugStr("--> RvEngineGetMethod - \n");
	
	switch(methodTag) {
	case kQADrawPrivateNew: 
    method->drawPrivateNew        = RvDrawPrivateNew;
    DebugStr("drawPrivateNew");        
    break;

	case kQADrawPrivateDelete:
    method->drawPrivateDelete     = RvDrawPrivateDelete; 
    DebugStr("drawPrivateDelete"); 
    break;

	case kQAEngineCheckDevice: 
    method->engineCheckDevice     = RvEngineDeviceCheck; 
    DebugStr("engineCheckDevice"); 
    break;

	case kQAEngineGestalt: 
    method->engineGestalt         = RvEngineGestalt; 
    DebugStr("engineGestalt"); 
    break;

	case kQATextureNew: 
    method->textureNew            = RvTextureNew; 
    DebugStr("textureNew"); 
    break;

	case kQATextureDetach: 
    method->textureDetach         = RvTextureDetach; 
    DebugStr("textureDetach"); 
    break;

	case kQATextureDelete: 
    method->textureDelete         = RvTextureDelete; 
    DebugStr("textureDelete"); 
    break;

	case kQABitmapNew: 
    method->bitmapNew             = RvBitmapNew; 
    DebugStr("bitmapNew"); 
    break;

	case kQABitmapDetach: 
    method->bitmapDetach          = RvBitmapDetach; 
    DebugStr("bitmapDetach"); 
    break;

	case kQABitmapDelete: 
    method->bitmapDelete          = RvBitmapDelete; 
    DebugStr("bitmapDelete"); 
    break;

	case kQAColorTableNew: 
    method->colorTableNew         = RvColorTableNew; 
    DebugStr("colorTableNew"); 
    break;

	case kQAColorTableDelete: 
    method->colorTableDelete      = RvColorTableDelete; 
    DebugStr("colorTableDelete"); 
    break;

	case kQATextureBindColorTable: 
    method->textureBindColorTable = RvTextureBindColorTable; 
    DebugStr("textureBindColorTable"); 
    break;

	case kQABitmapBindColorTable: 
    method->bitmapBindColorTable  = RvBitmapBindColorTable; 
    DebugStr("bitmapBindColorTable"); 
    break;

	case kQAAccessTexture:
		method->accessTexture = NULL;
		DebugStr("accessTexture");
		break;
		
	case kQAAccessTextureEnd:
		method->accessTextureEnd = NULL;
		DebugStr("accessTextureEnd");
		break;
		
	case kQAAccessBitmap:
		method->accessBitmap = NULL;
		DebugStr("accessBitmap");
		break;
		
	case kQAAccessBitmapEnd:
		method->accessBitmapEnd = NULL;
		DebugStr("accessBitmapEnd");
		break;

	default:
		DebugStr("*** unsupported selector");
		err = kQAParamErr;
		goto bail;
	}

bail:
	DebugStr("\n");
	return err;
}


#if 0

static Ptr AllocPtr2(Size s)
{
	Handle h;
	OSErr err;
	
	h = TempNewHandle(s, &err);
	
	if(!err && *h != NULL){
		HLock(h);
		return *h;
	}
	
	return NULL;
}


static void FreePtr2(Ptr p)
{
	Handle h;
	OSErr err;
	
	if(p == NULL) return;
	
	h = RecoverHandle(p);
	err = MemError();
	
	if(err == noErr){
		DisposeHandle(h);
	}
}

#else

static Ptr AllocPtr2(Size s)
{
	return NewPtr(s);
}


static void FreePtr2(Ptr p)
{
	DisposePtr(p);
}
#endif

#if 0

static Ptr AllocPtr2(Size s)
{
	Handle h;
	OSErr err;
	
#if 0
	h = NewHandle(s + 8);
	
	err = MemError();
#else
	h = TempNewHandle(s + 8, &err);
#endif
	if(!err && *h != NULL){
		Ptr p;
		HLock(h);
		p = *h;
		*((Handle *)p) = h;
		*(unsigned long *)(p + 4) = 'r2d2';
		return p + 8;
	}
	
	return NULL;
}


static void FreePtr2(Ptr p)
{
	Handle h;
	
	if(p == NULL) return;
	
	if(*(unsigned long *)(p - 4) != 'r2d2'){
		char s[300];
		sprintf(s, "bad FreePtr2, p - 8 = %x", (unsigned long)p - 8);
		debugstr(s);
	}
	*(unsigned long *)(p - 4) = 'c3po';
	
	h = *(Handle *)(p - 8);

	HUnlock(h);
	DisposeHandle(h);

}
#endif

#if TNSL_DEBUG_MEM
typedef struct MemHeader{
	unsigned long safetyCheck;
	unsigned long size;
	struct MemHeader * next;
} MemHeader;

MemHeader * headerList;
#define memHeaderSize sizeof (MemHeader)
#define buf 8

static void SetBlock(Ptr p)
{
	int i;
	for (i = 0; i < buf; i++){
		p[i] = i;
	}
}

static void CheckBlock(Ptr p)
{
	int i;
	for (i = 0; i < buf; i++){
		if(p[i] != (char)i){
			debugstr("bad CheckBlock");
		}
	}
}


#else

#define memHeaderSize 0
#define buf 0

#endif


#if TNSL_DEBUG_MEM

void MemoryCheck(void)
{
	MemHeader * hd;
	hd = headerList;
	while(hd){
		if(hd->safetyCheck != 'memh'){
			debugstr("bad mem header");
		}
		
		CheckBlock(((Ptr)hd) - buf);
		CheckBlock(((Ptr)hd) + sizeof (MemHeader) + hd->size);
		
		hd = hd->next;
	}
}

static void RemoveHeader(Ptr p)
{
	MemHeader * hd;
	MemHeader ** pp;
	
	pp = &headerList;
	
	hd = *pp;
	while(hd){
		if(((Ptr)hd) + sizeof (MemHeader) == p) {
			*pp = hd->next;
			blockCount--;
			return;
		}
		
		pp = &hd->next;
		hd = *pp;
	}

	debugstr("bad RemoveHeader");
}

#endif


Ptr AllocPtr(Size s)
{
	Ptr p;
	
	MemoryCheck();
	
	p =  AllocPtr2(s + buf * 2 + memHeaderSize);
	if(p == NULL){
		return p;
	}
		
#if TNSL_DEBUG_MEM
	{
		MemHeader * hd;

		SetBlock(p);
		SetBlock(p + buf + memHeaderSize + s);
	
		hd = (MemHeader *)(p + buf);
		hd->safetyCheck = 'memh';
		hd->size = s;
		hd->next = headerList;	
		headerList = hd;

		blockCount++;
		
		
		if(blockCount > maxBlockCount){
			char s[300];
			maxBlockCount += 100;
			sprintf(
				s, 
				"blockCount = %d, textureCount = %d, totalTextureSize = %d", 
				blockCount, 
				gRvEngInfo.textureCount, 
				gRvEngInfo.totalTextureSize);
			//debugstr(s);
		}
	}
#endif
		
	return p + buf + memHeaderSize;
}


void FreePtr(Ptr p)
{
	MemoryCheck();

	if(p == NULL){
		debugstr("bad ptr");
	}
#if TNSL_DEBUG_MEM
	
	RemoveHeader(p);
#endif

	FreePtr2(p - (buf + memHeaderSize));
}





