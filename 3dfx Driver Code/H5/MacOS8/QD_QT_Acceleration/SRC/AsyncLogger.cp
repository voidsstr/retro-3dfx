/******************************************************************************
	File:		CAsyncLogger.cp
	Copyright:	© 1995-99 Critical Path Software Inc.  All Rights Reserved.
	Author:		Andrew Mellinger
	Purpose:
	History:	
 ******************************************************************************/
	
	/* Headers */
/******************************************************************************/
#include "AsyncLogger.h"

	/* Constants */
/******************************************************************************/
#define 	kNumberQElems	10
#define		kMaxWriteSize	2048

const char		kTestString[] = "TestString.\n";

	/* Typedefs */
/******************************************************************************/
typedef	struct
{
	QElemPtr		qLink;
	IOParam			pb;
	char			buffer[kMaxWriteSize];
} AsyncLoggerPBTy, *AsyncLoggerPBPt;

	/* Globals */
	/* File Globals */
/******************************************************************************/
bool			gInited = false;
QHdr			gEmptyQ;
Ptr				gBlocksPtr;
SInt16			gRefNum;
IOCompletionUPP	gWriteCompUPP;
UInt32			gOutstandingCalls;

	/* Prototypes */
/******************************************************************************/
void WriteComp(ParmBlkPtr pbPtr);

	/* Methods */
/******************************************************************************/
OSErr InitAsyncLogger(StringPtr fileName)
{
	gBlocksPtr			= 0;
	gRefNum				= 0;
	gOutstandingCalls	= 0;

	OSErr	err;
	
	gEmptyQ.qFlags	= 0;
	gEmptyQ.qHead	= 0;
	gEmptyQ.qTail	= 0;

	// Setup completion routines
	gWriteCompUPP = NewIOCompletionProc(WriteComp);

	// Setup our param block queues
	gBlocksPtr = ::NewPtrClear(sizeof(AsyncLoggerPBTy) * kNumberQElems);
	if (gBlocksPtr == 0)
		return memFullErr;

	AsyncLoggerPBPt logger = (AsyncLoggerPBPt) gBlocksPtr;
	for (UInt16 shep = 0; shep < kNumberQElems; ++shep)
	{
		::Enqueue((QElemPtr)&(logger[shep]), &gEmptyQ);
	}

	// Find the file
	FSSpec				spec;
	ProcessSerialNumber psn;
	ProcessInfoRec		info;
	err = ::GetCurrentProcess(&psn);
	if (err != noErr)
		return err;
		
	info.processInfoLength	= sizeof(ProcessInfoRec);
	info.processName		= 0;
	info.processAppSpec		= &spec;

	err = ::GetProcessInformation(&psn, &info);
	if (err != noErr)
		return err;

//	err = ::FSMakeFSSpec(spec.vRefNum, spec.parID,
//			"\pLogFile", &spec);
	err = ::FSMakeFSSpec(spec.vRefNum, spec.parID,
			fileName, &spec);
	if (err == noErr)
	{
		// Delete
		err = ::FSpDelete(&spec);
		if (err != noErr)
			return err;
	}
	else if (err != fnfErr)
	{
		// Some other odd error.  Don't make a refNum
		return err;
	}	
	
	// Create the file
	err = ::FSpCreate(&spec, 'R*ch', 'TEXT', smSystemScript);
	if (err != noErr)
		return err;

	// Open the file for writing
	err = ::FSpOpenDF(&spec, fsRdWrPerm, &gRefNum);
	if (err != noErr)
		return err;

	gInited = true;

	return err;
}

/*----------------------------------------------------------------------------*/
void TerminateAsyncLogger()
{
	if (gInited)
	{
		// If we have pending writes, wait them out.
		while (gOutstandingCalls != 0)
			SystemTask();
			
		if (gBlocksPtr != 0)
			DisposePtr(gBlocksPtr);
	}
}

/*----------------------------------------------------------------------------*/
OSErr WriteToAsyncLog(char *data, UInt32 size)
{
	OSErr			err = noErr;
	AsyncLoggerPBPt entryPt = (AsyncLoggerPBPt) gEmptyQ.qHead;

	if (gInited == false)
		return -1;

	if (entryPt == 0)
		return -1;
	
	if (gRefNum == -1)
		return -1;

	if(size > kMaxWriteSize)
		return -1;

	// Pull off an element
	Dequeue((QElemPtr)entryPt, &gEmptyQ);
	
	BlockMoveData(data, entryPt->buffer, size);

	entryPt->pb.ioCompletion	= gWriteCompUPP;
	entryPt->pb.ioResult		= 0;
	entryPt->pb.ioNamePtr		= 0;
	entryPt->pb.ioVRefNum		= 0;
	entryPt->pb.ioRefNum		= gRefNum;
	entryPt->pb.ioVersNum		= 0;
	entryPt->pb.ioPermssn		= 0;
	entryPt->pb.ioMisc			= 0;
	entryPt->pb.ioBuffer		= entryPt->buffer;
	entryPt->pb.ioReqCount		= size;

	entryPt->pb.ioActCount		= 0;
	entryPt->pb.ioPosMode		= fsAtMark;
	entryPt->pb.ioPosOffset		= 0;
	
	++gOutstandingCalls;

	err = PBWriteAsync((ParmBlkPtr)&entryPt->pb);
	if (err)
	{
		// There was an error, the completion routine won't move it.
		Enqueue((QElemPtr)entryPt, &gEmptyQ);
	}
	
	return err;
}

/*----------------------------------------------------------------------------*/
void WriteComp(ParmBlkPtr pbPtr)
{
	AsyncLoggerPBPt		entryPt;
	entryPt = (AsyncLoggerPBPt) (((char *)pbPtr) - sizeof(QElemPtr));

	--gOutstandingCalls;

	// Put in the empty Q for later
	Enqueue((QElemPtr)entryPt, &gEmptyQ);
}

	/* Functions */
/******************************************************************************/
/*----------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
