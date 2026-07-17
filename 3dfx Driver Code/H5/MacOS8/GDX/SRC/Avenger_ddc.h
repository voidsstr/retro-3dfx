#ifndef __AVENGER_DDC__
#define __AVENGER_DDC__

extern int AvengerReadDDC(FxU32 port, int nAddr, Byte *pBuffer, int nSize);
extern int AvengerReadSiI( int nAddr, Byte * pBuffer, int nSize );

extern Byte GetEDIDVersion( Byte * edid );
extern Byte GetEDIDRevision( Byte * edid );
extern void GetEDIDManufacturerName( Byte * edid, long man );
extern Byte GetEDIDVideoInputType( Byte * edid );
extern Byte GetEDIDHorImageSize( Byte * edid );
extern Byte GetEDIDVerImageSize( Byte * edid );
extern float GetEDIDGammaValue( Byte * edid );
extern Byte GetEDIDDPMS( Byte * edid );
extern Byte GetEDIDStdTimingHorPix( Byte * edid, short ID );
extern Byte GetEDIDStdTimingAspectRatio( Byte * edid, short ID );
extern Byte GetEDIDStdTimingRefresh( Byte * edid, short ID );
extern short GetEDIDDetailedTimingPixelClock( Byte * edid, short ID );
extern long GetEDIDDetailedTimingHActive( Byte * edid, short ID );
extern long GetEDIDDetailedTimingHBlank( Byte * edid, short ID );
extern long GetEDIDDetailedTimingVActive( Byte * edid, short ID );
extern long GetEDIDDetailedTimingVBlank( Byte * edid, short ID );
extern long GetEDIDDetailedTimingHSyncOffset( Byte * edid, short ID );
extern long GetEDIDDetailedTimingHSyncWidth( Byte * edid, short ID );
extern long GetEDIDDetailedTimingVSyncOffset( Byte * edid, short ID );
extern long GetEDIDDetailedTimingVSyncWidth( Byte * edid, short ID );
extern Byte GetEDIDDetailedTimingHImageSize( Byte * edid, short ID );
extern Byte GetEDIDDetailedTimingVImageSize( Byte * edid, short ID );
extern Byte GetEDIDDetailedTimingHBorder( Byte * edid, short ID );
extern Byte GetEDIDDetailedTimingVBorder( Byte * edid, short ID );
extern Byte GetEDIDDetailedTimingFlags( Byte * edid, short ID );

#endif	/* __AVENGER_DDC__ */


