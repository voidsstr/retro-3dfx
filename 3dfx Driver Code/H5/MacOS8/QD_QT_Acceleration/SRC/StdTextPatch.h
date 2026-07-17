/********************************************************************************
	StdTextPatch.h
		
	Header file for text cache functions.
	
	Chall Fry
	Critical Path Software
*/

// Includes
#pragma once

// Types

// A simple bit field, 256 bits long.
class BitField256
{
	UInt32	data[8];
public:
	
	inline bool CheckBit(UInt32 bitNum) { return (data[bitNum >> 5] >> (bitNum & 31)) & 1; };
	
	inline void	SetBit(UInt32 bitNum) { data[bitNum >> 5] |= 1 << (bitNum & 31); };
	inline void ClearBit(UInt32 bitNum) { data[bitNum >> 5] &= ~(1 << bitNum & 31); };
	inline void Clear() { for (UInt32 index = 0; index < 8; ++index) data[index] = 0; };
};

// Width information kept about each character in each font in the cache
class CharMetrics
{
	// FFFF.FFFF FFFF LLLL LLGG GGGG AAAA AAAA
	UInt32	data;
	
public:
//	inline Fixed 	GetAdvance() { return ((SInt32) ((data << 24) & 0xFF000000)) >> 8; };
	inline Fixed 	GetAdvance() { return (data & 0xFF) << 16; };
	inline UInt32 	GetGlyph() { return (data >> 8) & 0x3F; };
	inline SInt32	GetLeft() { return ((SInt32) ((data << 12 ) & 0xFC000000)) >> 26; };
	inline Fixed	GetFract() { return ((data & 0xFF) << 16) + 
							((((SInt32) data) >> 12) & 0xFFFFFF00); };
	
	inline void		SetGlyph(UInt32 glyph) { data |= (glyph & 0x3F) << 8; };
	inline void		SetAdvance(SInt32 advance) { data |= advance & 0xFF; };
	inline void		SetLeft(SInt32 left) { data |= (left & 0x3F) << 14; };
	inline void		SetFract(Fixed fractWidth) { fractWidth -= GetAdvance(); 
							data |= (fractWidth << 12) & 0xFFF00000; };
	inline void		Clear() { data = 0; };
};

class FontData;
class FontListItem
{
public:
	FontData *next;
	FontData *prev;	
};

// FontFlag bits
enum
{
	kFontCheckedOutlinePrefOff = 1,
	kFontCheckedOutlinePrefOn  = 2,
	kFontApprovedOutlinePrefOff = 4,
	kFontApprovedOutlinePrefOn = 8,
	kFontIsOutline			   = 16,
	kFontIsAntiAliased		   = 32
};

// Information kept about a font in the font cache
class StdTextInfo;
class FontData : public FontListItem
{

	Fixed 	MeasureTextWidthInternal(StdTextInfo &info, bool fractEnable, bool extraSpace, 
					bool isRendering, bool scaledText, Fixed spExtra = 0, Fixed chExtra = 0);
	Fixed 	MeasureTextWidth2nd(StdTextInfo &info, bool fractEnable, bool extraSpace, 
					Fixed spExtra = 0, Fixed chExtra = 0);

public:
	UInt16		fontIndex;			// The font cache index for this font
	
	short		mFont;
	short		mFace;
	short		mSize;
	Point		mNumer;
	Point		mDenom;
	Fixed		mHorizScale;		// Imprecise, used for font matching
	Fixed		mVertScale;
	UInt16		fontFlags;
	
	// This is what our StdTextMeas patch returns in response to requests for this font
	FontInfo	fInfo;
	
	// MBW -- XXX -- Used for debugging
	FMOutput	mSFOutput;
	
	// These fields are used for measuring and drawing text with our cached glyphs
	UInt32		ascent;
	UInt32		descent;
	UInt32		imageRowBytes;
	UInt32		widMax;
		
	// Bitfields for tracking which characters have been measured and rendered
	BitField256	measuredChars;
	BitField256	fractMeasuredChars;
	BitField256	renderedChars;
	
	// Metrics info (glyph width, advance width, left side bearing, fractional width) for each char
	CharMetrics	metrics[256];
	
	// Information about how this font is imaged on the boards

	void	Initialize();
	bool	SetupFontData(StdTextInfo &info);
	bool 	IsMatch(short inFont, short inSize, short inFace, Fixed horizScale, Fixed vertScale,
					bool inAntiAliased);
	Fixed	MeasureTextWidth(StdTextInfo &info);
	void 	SetCharWidth(StdTextInfo &info, UInt8 curChar);
	void	SetCharWidthBitmap(StdTextInfo &info, UInt8 curChar);
	void 	SetCharWidthOutline(StdTextInfo &info, UInt8 curChar);
	void	RenderChar(StdTextInfo &info, UInt8 curChar);
	
	inline void	CheckCharRendered(StdTextInfo &info, UInt8 curChar)
	{ 
		if (!renderedChars.CheckBit(curChar))
			RenderChar(info, curChar);
	};

};

// The font cache itself
class FontCache
{
public:
	static const UInt32	kNumFonts = 300;
		
	FontData		fonts[kNumFonts];
	FontListItem	usedList;
	FontListItem	freeList;
	

				FontCache();
	void 		Init();
	void 		Terminate();

	FontData	*GetFirst() { return usedList.next; };
	FontData	*GetLast() { return usedList.prev; };
	FontData	*GetNext(FontData *inData) { return inData->next; };
	FontData	*NewFontData();
	void		RemoveFontData(FontData *inData);

				
	FontData	*GetFont(StdTextInfo &info);
};

// Globals

extern FontCache	gFonts;

// Function Declarations

short StdTextMeasPatch(short byteCount, UInt8 *textBuf, Point *numer, 
					Point *denom, FontInfo *info);
void StdTextPatch(short byteCount, UInt8 *textBuf, Point numer, Point denom);

 