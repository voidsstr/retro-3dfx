//: raveTexture.h

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Voodoo3: 5 MB per context
// Napalm:	28 MB per context
#define MAX_TEXTUREBYTES_PER_ENGINE	((NAPALM_AND_BEYOND) ? 27912864 : 0x500000)


struct TQATexture {
	unsigned long safetyCheck;	// always set to 'txtr'
	TQAColorTable	*colorTable;	// used if format = P_8
	TQATexture 	*next;			// the next texture in the master texture list
	TQATexture 	*next_resident;	// the next texture on the card
	TQATexture 	*prev_resident;	// the previous texture on the card

	// RAVE params
	UInt32		flags;				// passed in from app
	
	TQAImagePixelType	pixelType;	// the rave pixel type
	SInt32		alphaBits;			// 0 means no alpha
									// 1 means 1 bit alpha
									// > 1 means multi bit alpha

	GrTexInfo	glideTextureInfo;	// 20 bytes
	Handle		storage;			
	GLsizei		size;		// this is the size of our converted texture data in system memory

	// needed for CONVERT_STW
	FxFloat		ratioX;
	FxFloat		ratioY;

	GLboolean	resident;	// set if texture is on the card
	GLint		offset;		// offset from the begining of texture VRAM

	FxBool		cached;			// set if texture used by a cached triangle
	FxBool		toBeDeleted;	// set if texture used by a cached triangle and needs to be deleted at the end fo the frame
	FxBool		belongsToBitmap;	// don't delete when deleting all textures
	
};

typedef struct bmpi {
	TQATexture		*texPiece;
	Rect			rectPow;
	UInt32			bitmapWidth;
	UInt32			bitmapHeight;
	float			bitmapUoverW;
	float			bitmapVoverW;
	float			bitmapBiasU;
	float			bitmapBiasV;
} BmPiece;



struct TQABitmap {
	UInt32		numPieces;
	BmPiece		*pieces;
};

// prototypes
FxBool LoadTextureAndTableToVRAM(TQADrawPrivate	*dp, TQATexture *theTexture );
GLuint LoadAsBaseTexture( TQADrawPrivate	*dp, TQATexture *theTexture);
void FreeAllNormalTextures( void );
void EradicateTextures( void  );
void FreeAllTexturesVRAM( void );
int IsValidTexture(TQATexture * theTexture);


#ifdef __cplusplus
}
#endif
