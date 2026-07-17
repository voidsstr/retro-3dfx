#ifndef MIPMAP_H
#define MIPMAP_H

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ddraw.h>
#include <d3d.h>
#include "atd3d.h"

#define MAX_CHANNELS 10

/* macro to get color out of Image structure (c=>channel, nc=>num channels) */

#define ColorOf(img, x, y, nc, c) ((img)->data[((y)*((img)->width)+(x))*nc+c])

FxBool _atrD3DPutImg( LPDIRECTDRAWSURFACE lpDDS, const AtrImg *img);
int CalculateMipMapLevels(int width, int height);
int DetermineDepth(int n);
void DeterminePackingShift(unsigned int mask, unsigned int *truncate_shift,
			   unsigned int *up_shift);
FxBool LoadMipMap(AtrImg *atrimg, _AtrD3DDriver *ctx, LPCSTR lpName);
void PackAndCopy(unsigned char *dest, unsigned char *src, int size, int Bpp,
		 int src_format, int r_mask, int g_mask, int b_mask, int a_mask);
#endif

