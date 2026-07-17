/********************************************************************************
	 hrm_patches.cp
	 	
	This file contains the functions the HRM needs to install OS Patches, and 
	to call registered HRM patch clients when a patched trap is called.
	
	 To patch new functions, search for ADD_PATCH and follow the comments.
	 
	 Chall Fry
	 Critical Path Software
*/

// Includes
#include "DynamicPatches.h"
#include <Traps.h>
#include <Patches.h>


// Types

template <class Patch>
class ApplyPatch
{
public:
	static RoutineDescriptor patchRD;

	static Patch::PatchFnType Apply()
	{
		Patch::PatchFnType patchFnPtr;

		patchFnPtr = (Patch::PatchFnType) NGetTrapAddress(Patch::kTrapNum, ToolTrap); 
		NSetTrapAddress(&patchRD, Patch::kTrapNum, ToolTrap);
		return patchFnPtr;
	};
};	


template <class Patch>
class ApplyD0SelectorPatch : public ApplyPatch<Patch>
{
public:
	static UInt16 selectorDispatch[];

	static Patch::PatchFnType Apply()
	{
		Patch::PatchFnType patchFnPtr;
		
		patchFnPtr = (Patch::PatchFnType) NGetTrapAddress(Patch::kTrapNum, ToolTrap); 

		// Insert the original selector address and the address of our implementation
		// into the 68k code for decoding the selector
		((UInt32*)(selectorDispatch + 5))[0] = (UInt32)&patchRD;
		((UInt32*)(selectorDispatch + 8))[0] = (UInt32)patchFnPtr;

		// MBW -- XXX -- I don't think we need to flush caches here, but we might.
		
		NSetTrapAddress((UniversalProcPtr)&selectorDispatch, Patch::kTrapNum, ToolTrap);
		return patchFnPtr;
	};
};	

template <class Patch>
UInt16 ApplyD0SelectorPatch<Patch>::selectorDispatch[] =
{
	// This is the 68k code for decoding the selector.
	0x0C80, Patch::kTrapSelector >> 16, Patch::kTrapSelector & 0x0000FFFF,		
								// cmpi.l    Patch::kTrapSelector, d0
	0x6606,						// bne.s     *+8 
	0x4EF9, 0xDEAD, 0xBEEF,		// jmp       0xdeadbeef -> change DEADBEEF to our implementation
	0x4EF9, 0xDEAD, 0xC0DE,		// jmp       0xdeadbeef -> change DEADC0DE to original dispatcher
};

// Routine Descriptors

template <class Patch>
RoutineDescriptor ApplyPatch<Patch>::patchRD = BUILD_ROUTINE_DESCRIPTOR(Patch::kProcInfo,
		Patch::PatchFn);
	
// Globals
UInt32	gPatchManagerInitialized = false;

// Function Definitions

/********************************************************************************
	hrm_InitPatchManager
		
	Initializes the patch manager. Applies the OS patches, which will initially 
	just call through to the OS. 
*/
void hrm_InitPatchManager()
{
	// Set up our initial patch group.
	gPatchGroup.Init();
	gPatchGroup.nextPatchGroup = &gPatchGroup;
	gPatchGroup.isToolbox = true;
	
	// Apply the Patches
	gPatchGroup.stdBitsPatch = ApplyPatch<PatchStdBits>::Apply();
	gPatchGroup.stdTextPatch = ApplyPatch<PatchStdText>::Apply();
	gPatchGroup.stdTextMeasPatch = ApplyPatch<PatchStdTextMeas>::Apply();
	gPatchGroup.drawPicturePatch = ApplyPatch<PatchDrawPicture>::Apply();
	gPatchGroup.newGWorldPatch = ApplyD0SelectorPatch<PatchNewGWorld>::Apply();
	gPatchGroup.menuSelectPatch = ApplyPatch<PatchMenuSelect>::Apply();

	// ADD_PATCH: Add a line here to apply the patch for the new trap
	
	gPatchManagerInitialized = true;
}

/********************************************************************************
	hrm_RegisterPatches
		PatchGroup *patches
		
	HRM clients call this function to register their patch group with the HRM.
	After registration, the functions passed in in the patches argument will be
	called in response to an application callnig the associated trap. 
*/
void hrm_RegisterPatches(PatchGroup *patches)
{
	// Check that we're initialized
	if (!gPatchManagerInitialized)
		return;

	// Check that the new patch group is valid
	if (!patches || patches->structSize != sizeof(PatchGroup))
		return;

	patches->nextPatchGroup = gPatchGroup.nextPatchGroup;
	gPatchGroup.nextPatchGroup = patches;
}

/********************************************************************************
	hrm_DeregisterPatches
		PatchGroup *patches
		
	HRM clients call this function to deregister their patch group. It is necessary
	for clients to call this function before unloading, or else a system crash 
	will occur very soon.
*/
void hrm_DeregisterPatches(PatchGroup *patches)
{
	PatchGroup		*listIter = &gPatchGroup;

	// Check that we're initialized
	if (!gPatchManagerInitialized)
		return;

	// Check the passed in patch group
	if (!patches)
		return;
		
	// Search for the given patch group, and unlink it from the list
	while (listIter->nextPatchGroup != &gPatchGroup)
	{
		if (listIter->nextPatchGroup == patches)
		{
			listIter->nextPatchGroup = patches->nextPatchGroup;
			break;
		}
		listIter = listIter->nextPatchGroup;
	}
}
