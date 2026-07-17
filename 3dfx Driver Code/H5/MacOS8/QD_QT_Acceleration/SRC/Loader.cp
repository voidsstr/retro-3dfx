/********************************************************************************
	Loader.cp
		
	Chall Fry
	Critical Path Software
	
*/

// Includes
#include <Resources.h>
#include <CodeFragments.h>
#include <Displays.h>
#include <CodeFragments.h>

#include "RegisterFileLibs.h"
#include "HDWR_Res_Mgr.h"
#include "DynamicPatches.h"
#include "ShowInitIcon.h"

// Enums

enum
{
	kSuccessIcon = 128,
	kFailureIcon = 129
};

// Function Definitions
extern "C" void LoaderMain();

// Function Declarations

/********************************************************************************
	LoaderMain
		
	
*/
void LoaderMain()
{
	FSSpec	spec;
	Str255	fileName;
	Boolean	initIconID = kFailureIcon;
	OSErr	err;

// Debugger();

	// Switch to the System Zone
	THz curZone = GetZone();
	SetZone(SystemZone());
	
	// Get the location of our init file
	FCBPBRec	pb;
	pb.ioRefNum = CurResFile();
	pb.ioNamePtr = fileName;
	pb.ioFCBIndx = 0;
	pb.ioCompletion = 0;
	if (PBGetFCBInfoSync(&pb))
		goto error;
	FSMakeFSSpec(pb.ioFCBVRefNum, pb.ioFCBParID, fileName, &spec);

	// Set up this file as a shared library-containing file 
	RegisterFileLibs(&spec);
	
	// 
	CFragConnectionID	connID;
	hrmGetExtensionPtr	hrmExtAddr;
	Str255 errName;
	if (!GetSharedLibrary("\p3DfxHrdwResMgr", kPowerPCCFragArch, kLoadCFrag, &connID, 
			(char **) &hrmExtAddr, errName))
	{
		CFragSymbolClass	cl;
		long (*hrmGetNumTargets)(void);
		FindSymbol(connID, "\phrmGetNumTargets", (char **) &hrmGetNumTargets, &cl);
		
		if (hrmGetNumTargets())
		{
			// Since it's INIT time, we need to tell the HRM to load in patches.
			// If the HRM gets instantiated at application time, DO NOT call this function.
			hrmInitPatchManagerPtr	initPMPtr = 
					(hrmInitPatchManagerPtr) hrmExtAddr("hrmInitPatchManager");
			if (initPMPtr)
				initPMPtr();
	
			// The shared lib should open a connection to itself as soon as we open it; 
			// it will then never go away. Now, we need to tell the HRM to load all the
			// acceleration resources
			UInt32 (*loadAccel)(void) = (UInt32 (*)(void)) hrmExtAddr("hrmLoadAccel");
			if (loadAccel && loadAccel())
			{
				initIconID = kSuccessIcon;			
			}
		}		
	}

error:
	// Check for existance of a display, and the existance of our icon resource
	DisplayIDType displayID;
	GDHandle display = DMGetFirstScreenDevice(true);
	err = DMGetDisplayIDByGDevice(display, &displayID, false);
	if (!err && (displayID != kDummyDeviceID) && Get1Resource('icl8', initIconID))
		ShowInitIcon(initIconID, true);

	SetZone(curZone);
}