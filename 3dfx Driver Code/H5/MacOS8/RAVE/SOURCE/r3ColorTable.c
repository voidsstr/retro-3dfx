//: r3ColorTable.c
//: alt.drivers inc.
//: Glenn Nissen
//: Vadim Kochubievski
//: Rohit Gundecha


#include "r3Core.h"
extern TRvInfo 	gRvEngInfo;
extern long		gEngID;
extern Boolean	gRave16;

/////////////////////////
//                     //
//  RvColorTableNew  //
//                     //
/////////////////////////

TQAError 
RvColorTableNew(TQAColorTableType pixelType,
                  void*             pixelData,
                  long              transparentIndexFlag,
                  TQAColorTable**   newTable)
{
	int ii;
	
	// UInt32 r,g,b;
	// UInt32		rgb;
	UInt32 *pu32 = pixelData;

	DebugStr("--> RvColorTableNew "); 
	DebugStr(" pixelType=(0=kQAColorTable_CL8_RGB32)=");
	DebugNum(pixelType); 
	DebugStr(" transparentIndexFlag="); DebugNum(transparentIndexFlag);

	*newTable = (struct TQAColorTable*) AllocPtr(sizeof (**newTable));
	if(*newTable == NULL) goto fail_0;
	DebugHex((UInt32 *)(*newTable)); DebugStr("\n");

	if (gRave16)
		gEngID = QAGetCurrentEngineRefCon();
	
	HLock((Handle)newTable);
	(**newTable).transparentIndexFlag = transparentIndexFlag;
	DebugStr( "entry[0]="); DebugHex(pixelData);
	(**newTable).pixelType        = pixelType;
	if ((pixelType != kQAColorTable_CL8_RGB32) && (pixelType != kQAColorTable_CL4_RGB32))
		goto fail_1;
		
	switch(pixelType) {
	case kQAColorTable_CL8_RGB32: 
		memcpy((**newTable).pixelData, pixelData, 1024);		// 1024 = 4 bytes/entry * 256 entries
		break;
		
	case kQAColorTable_CL4_RGB32:
		for (ii = 0; ii < 16; ii++) 
		{
			// memcpy(&((UInt32*) (**newTable).pixelData)[ii], pixelData, 64); // 64 = 4 bytes/entry * 16 entries
			(**newTable).pixelData[ii] = *pu32++;
			
		}
		break;
		
	}
	HUnlock((Handle)newTable);
	DebugStr("\n");
	
	return kQANoErr;
	
fail_1:
	FreePtr((void*) *newTable);
	
fail_0:
	return kQAError;
}


////////////////////////////
//                        //
//  RvColorTableDelete  //
//                        //
////////////////////////////

void 
RvColorTableDelete(TQAColorTable* colorTable)
{
	DebugStr("--> RvColorTableDelete\n");

	FreePtr((void *) colorTable);
}


/////////////////////////////////
//                             //
//  RvTextureBindColorTable  //
//                             //
/////////////////////////////////

TQAError 
RvTextureBindColorTable(TQATexture*    texture,
                          TQAColorTable* colorTable)
{
	DebugStr("--> RvTextureBindColorTable, texture ");

	if (gRave16)
		gEngID = QAGetCurrentEngineRefCon();

	switch(colorTable->pixelType) {
	case kQAColorTable_CL8_RGB32:
		if(texture->glideTextureInfo.format != GR_TEXFMT_P_8) goto fail_0;
		break;
		
	case kQAColorTable_CL4_RGB32:
		if(texture->glideTextureInfo.format != GR_TEXFMT_P_8) goto fail_0;
		break;
		
	default:
		goto fail_0;	
	}
	
	texture->colorTable = colorTable;
	DebugHex(texture); DebugStr("&"); DebugHex(colorTable); DebugStr("\n");
		
	return kQANoErr;
	
fail_0:
	assert(0);
	return kQAError;
}


////////////////////////////////
//                            //
//  RvBitmapBindColorTable  //
//                            //
////////////////////////////////

TQAError 
RvBitmapBindColorTable(TQABitmap*     bitmap,
                         TQAColorTable* colorTable)
{
	int		p, numPieces;
	DebugStr("--> RvBitmapBindColorTable, bitmap ");

	if (gRave16)
		gEngID = QAGetCurrentEngineRefCon();

	switch(colorTable->pixelType) {
		case kQAColorTable_CL4_RGB32:
		case kQAColorTable_CL8_RGB32:
			if (bitmap->pieces[0].texPiece->glideTextureInfo.format != GR_TEXFMT_P_8) goto fail_0;
			break;
			
		default:
			goto fail_0;	
	}
	
	numPieces = bitmap->numPieces;
	for ( p = 0; p < bitmap->numPieces; p++)
		bitmap->pieces[p].texPiece->colorTable = colorTable;
		
	DebugHex(bitmap); DebugStr("&"); DebugHex(colorTable); DebugStr("\n");
		
	return kQANoErr;
	
fail_0:
	assert(0);
	return kQAError;
}
