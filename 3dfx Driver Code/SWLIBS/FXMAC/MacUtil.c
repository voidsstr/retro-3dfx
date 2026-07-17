#include <MacUtil.h>
#include <ctype.h>
#include <3dfx.h>
//#include <cvgregs.h>
//#include <cvgdefs.h>
#include <fxpci.h>
//#include <sst1init.h>
#include <Gestalt.h>
#include "glide.h"
#include "fxglide.h"

#include <retrace.h>
#include <Notification.h>


	void					InstallVBLTask(void);
	void					RemoveVBLTask(void);
	void					CheckSwapPassthru(void);
	short					IsVirtualMemoryAvailable(void);
	static pascal void		PreProcessVBLTask(VBLTaskPtr theVBLTask);

	static VBLTaskPtr		sVBLtaskPtr;
	static short			sSlot;
	static short			sKeyDown;

void strupr
(
	char* cptr
)
{
	while( *cptr )
	{
#if 0
		*cptr = toupper( *cptr );
#else
		if( (*cptr >= 'a') && (*cptr <= 'z') )
			*cptr -= 0x20;
#endif
		cptr++;
	}
}

#if 0
void MacCheckBoardsInSystem(void)
{
    FxU32 vendorID = _3DFX_PCI_ID;     /* 3Dfx Vendor ID */
    FxU32 deviceID = 0xFFFF;           /* Find any 3Dfx board */
    FxU32 numBoards, j, n;

    numBoards = 0;
    for(j=0; j<SST1INIT_MAX_BOARDS; j++) 
    	pciFindCardMulti(vendorID, deviceID, &n, j);    
}
#endif

FxI32 _cpu_detect_asm(void)
{
	long gestaltVal;
	
	return ((Gestalt(gestaltNativeCPUtype, &gestaltVal) == noErr) && (gestaltVal >= gestaltCPU601))
		   ? 6
		   : 5;
}

void ErrorMacCallback( char* inMessage )
{
	NMRecPtr	theNotificationData;
	char *		theMessagePtr;
	OSErr		theSuccess;

	theNotificationData = (NMRecPtr) NewPtrSys( sizeof( NMRec ) + 128 );
	theMessagePtr = (Ptr) theNotificationData + sizeof( NMRec );

	theNotificationData->nmResp		= 0;
	theNotificationData->qType		= nmType;
	theNotificationData->nmRefCon	= 0;
	theNotificationData->nmSound	= (Handle) -1;
	theNotificationData->nmStr		= (unsigned char*) theMessagePtr;
	theNotificationData->nmMark		= 0;
	theNotificationData->nmIcon		= 0;

	
	{
		long		i = 1;
		while ( i < 127 && *inMessage != 0 ) {
			theMessagePtr[i] = *inMessage++;
			
			if ( theMessagePtr[i] == '\n' )
			  theMessagePtr[i] = ' ';
			
			i++;
		}
		theMessagePtr[0] = i - 1;
	}

	theSuccess = NMInstall( theNotificationData );
}

#if !HWC_GDX_INIT_HRM

void SstSetupMac(void)
{
	InstallVBLTask();
}

void SstCleanupMac(void)
{
	RemoveVBLTask();
}

void
InstallVBLTask(void)
{
	OSErr			theSuccess;
	KeyMap			theKeyMap;
	GDHandle		theGDevice;
	short			theRefNum;
	AuxDCEHandle	theDCEHandle;


	theSuccess = noErr;
	sKeyDown = false;
	theGDevice = GetDeviceList();
	theRefNum = (*theGDevice)->gdRefNum;
	theDCEHandle = (AuxDCEHandle)GetDCtlEntry( theRefNum );
	sSlot = (*theDCEHandle)->dCtlSlot;
	
	sVBLtaskPtr = (VBLTask *)NewPtrSysClear((long)sizeof(VBLTask));
	if (sVBLtaskPtr)
	{
		if ( IsVirtualMemoryAvailable() ) {
			LockMemory( sVBLtaskPtr, (long)sizeof(VBLTask));
		}
		sVBLtaskPtr->vblAddr = NewVBLProc( PreProcessVBLTask );
		sVBLtaskPtr->vblCount = 1;		// Every 1/60th of a second
		sVBLtaskPtr->qType = vType;
		sVBLtaskPtr->qLink = NULL;
		sVBLtaskPtr->vblPhase = 0;

		theSuccess = SlotVInstall( (QElemPtr) sVBLtaskPtr, sSlot);
	}

	GetKeys( theKeyMap );
	GetKeys( theKeyMap );
	GetKeys( theKeyMap );

}


void
RemoveVBLTask(void)
{
	if ( sVBLtaskPtr )
	{
		SlotVRemove( (QElemPtr) sVBLtaskPtr, sSlot );

		if ( sVBLtaskPtr->vblAddr ) {
			DisposeRoutineDescriptor( (VBLUPP) sVBLtaskPtr->vblAddr );
			sVBLtaskPtr->vblAddr = NULL;
		}

		if ( IsVirtualMemoryAvailable() ) {
			UnlockMemory( sVBLtaskPtr, (long)sizeof(VBLTask) );
		}

		DisposePtr( (Ptr) sVBLtaskPtr );
		sVBLtaskPtr = NULL;
	}
}


static pascal void
PreProcessVBLTask(VBLTaskPtr theVBLTask) 
{
	sVBLtaskPtr->vblCount = 1;		/* tell it to contiue */

	CheckSwapPassthru();
}

static void
CheckSwapPassthru(void)
{
	KeyMap			theKeyMap;
	SstRegs *		theSst;

	GetKeys( theKeyMap );
	if ( sKeyDown ) {
		/* we are waiting for the key to be released! */
		if ( (theKeyMap[0] == 0) ) {
			sKeyDown = false;
		}
		
	} else {
		if ( (theKeyMap[0] == 0x02000000) && (theKeyMap[1] == 0x00008004) ) {

			theSst = (SstRegs *) _GlideRoot.curGC->base_ptr;
			SET(theSst->fbiInit0, (GET(theSst->fbiInit0) ^ SST_EN_VGA_PASSTHRU) );

			sKeyDown = true;
		} else if ( theKeyMap[1] == 0x0000A004 ) {
			
			theSst = (SstRegs *) _GlideRoot.curGC->base_ptr;	
			SET(theSst->fbiInit0, (GET(theSst->fbiInit0) & ~SST_EN_VGA_PASSTHRU) );
		}
	}

}

short
IsVirtualMemoryAvailable(void)
{
	long	theFeature;
	short	theResult = false;

	theFeature = 0;
	if ( Gestalt( gestaltVMAttr, &theFeature) == noErr) {
		theResult = theFeature & (1<<gestaltVMPresent);
	}

	return theResult;
}
#endif

void single_precision_asm(void)
{

}

void double_precision_asm(void)
{

}

#if 0
void putenv(void)
{

}

void _outp(void)
{

}

void _outpw(void)
{

}

void _inp(void)
{

}
#endif /* Don't need for now */
