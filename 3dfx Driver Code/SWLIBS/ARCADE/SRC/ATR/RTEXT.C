    /*
    ** Copyright (c) 1996, 3Dfx Interactive, Inc.
    ** All Rights Reserved.
    **
    ** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
    ** the contents of this file may not be disclosed to third parties, copied or
    ** duplicated in any form, in whole or in part, without the prior written
    ** permission of 3Dfx Interactive, Inc.
    **
    ** RESTRICTED RIGHTS LEGEND:
    ** Use, duplication or disclosure by the Government is subject to restrictions
    ** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
    ** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
    ** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
    ** rights reserved under the Copyright Laws of the United States.
    **
    **
    ** $Revision: 4$ 
    ** $Date: 10/11/00 7:34:20 PM$ 
    **
    */
    
    
    #include <3dfx.h>
    #include <stdio.h>
    #include <string.h>
    #include <glide.h>
    #include "atrender.h"
    #include "fxatr.h"
    
    extern FxU32 _atrFontData[];
    extern FxU32 _atrFontInfo[];
    static char charMap[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,:=- ";
    static fontInitialized = FXFALSE;
    static AtrImg       textImage;
    static AtrTexHandle textTexture;
    static AtrMaterial  *textMaterial;
    
    static const float mapWidth   = 256.0f;
    static const float mapHeight  =  64.0f;
    static const float realWidth  = 128.0f;
    static const float realHeight =  32.0f;
    static float sScale, tScale;
    
    #define NUM_ROWS  3
    #define NUM_COLS 16
    
    #if macintosh
    static void strupr(char *s) {
        int len = strlen(s);
        int i;
    
        for(i = 0; i < len; i++) {
            if(s[i] >= 'a' && s[i] <= 'z') {
                s[i] += 'a' - 'A';
            }
        }
    }
    #endif
    
    static void
    atrInitFont(void) {
        Gu3dfInfo *info;
    
        if ( fontInitialized )
             return;
    
        if (( textTexture = atrTexNewHandle(ATR_TEXELFX_0)) == ATR_NULL_TEXHANDLE ) {
            atuError(FXTRUE, "Could not create ATB font\n");
        }
    
        info = ( Gu3dfInfo *) _atrFontInfo;
    #ifdef GLIDE3
        /*
        ** what a hack!! it looks like _atrFontInfo is binary encoding
        ** the Gu3dfInfor structure, and therefore contains the old
        ** small and large LOD values, and the old aspect ratio value too.
        */
    //    info->header.large_lod = 8 - info->header.large_lod;
    //    info->header.small_lod = 8 - info->header.small_lod;
    
      switch (info->header.small_lod) {
      case 1:
        info->header.small_lod = GR_LOD_LOG2_1;
        break;
      case 2:
        info->header.small_lod = GR_LOD_LOG2_2;
        break;
      case 4:
        info->header.small_lod = GR_LOD_LOG2_4;
        break;
      case 8:
        info->header.small_lod = GR_LOD_LOG2_8;
        break;
      case 16:
        info->header.small_lod = GR_LOD_LOG2_16;
        break;
      case 32:
        info->header.small_lod = GR_LOD_LOG2_32;
        break;
      case 64:
        info->header.small_lod = GR_LOD_LOG2_64;
        break;
      case 128:
        info->header.small_lod = GR_LOD_LOG2_128;
        break;
      case 256:
        info->header.small_lod = GR_LOD_LOG2_256;
        break;
      }
    
      switch (info->header.large_lod) {
      case 1:
        info->header.large_lod = GR_LOD_LOG2_1;
        break;
      case 2:
        info->header.large_lod = GR_LOD_LOG2_2;
        break;
      case 4:
        info->header.large_lod = GR_LOD_LOG2_4;
        break;
      case 8:
        info->header.large_lod = GR_LOD_LOG2_8;
        break;
      case 16:
        info->header.large_lod = GR_LOD_LOG2_16;
        break;
      case 32:
        info->header.large_lod = GR_LOD_LOG2_32;
        break;
      case 64:
        info->header.large_lod = GR_LOD_LOG2_64;
        break;
      case 128:
        info->header.large_lod = GR_LOD_LOG2_128;
        break;
      case 256:
        info->header.large_lod = GR_LOD_LOG2_256;
        break;
      }
    
        info->header.aspect_ratio = 3 - info->header.aspect_ratio;
              
    #else
    #endif
    
        textImage.format  = info->header.format;
        textImage.width   = info->header.width;
        textImage.height  = info->header.height;
    #ifdef GLIDE3
        textImage.nLevels = info->header.large_lod - info->header.small_lod + 1;
    #else
        textImage.nLevels = info->header.small_lod - info->header.large_lod + 1;
    #endif
        textImage.data    = _atrFontData;
    
        atrTexAssociate( textTexture, &textImage );
        
        textMaterial = atrMaterialAllocate( 1 );
        textMaterial->texture[0] = textTexture;
        textMaterial->texture[1] = ATR_NULL_TEXHANDLE;
        atrMaterialSetup( textMaterial, ATR_MAT_DECAL);
        textMaterial->texMinFilter[0] = GR_TEXTUREFILTER_POINT_SAMPLED;
        textMaterial->texMagFilter[0] = GR_TEXTUREFILTER_POINT_SAMPLED;
        textMaterial->texMMMode[0] =  ATR_TEXMIPMAP_DISABLE;
        textMaterial->chromaKeyEnable = FXTRUE;
        atrMaterialModify(textMaterial);
        fontInitialized = FXTRUE;
    }
    
    static void _mapIndexToTexCoords( float *s0, 
                                      float *t0, 
                                      float *s1, 
                                      float *t1, 
                                      FxU32 index ) {
        const float charWidth  =  8.0f /* mapWidth *// realWidth;
        const float charHeight = 10.0f /* mapHeight */ / realHeight;
        const FxU32 width  = (FxU32)(mapWidth / charWidth);
        const FxU32 height = (FxU32)(mapHeight / charHeight);
        FxU32 row, column;
    
        if ( index > NUM_ROWS*NUM_COLS ) {
            return;
        }
        
        row    = index / NUM_COLS;
        column = index % NUM_COLS;
    
        *s0 = column * charWidth*sScale;
        *s1 = *s0 + charWidth*sScale;
        *t0 = row * charHeight*tScale;
        *t1 = *t0 + charHeight*tScale;
        return;
    }
    
    void _atrDrawString( char *string, 
                     float char_width, float char_height,
                     float xmin, float ymin, FxBool opaque ) {
    #ifdef GLIDE3
    	AtrDstVertex a, b, c, d;
    #else
        GrVertex a, b, c, d;
    #endif
        float s0, s1, t0, t1;
    
        if ( !fontInitialized ) {
            atrInitFont();
        }
    
        if ( !string ) return;
    
        /* convert to upper case */
    
        strupr(string);
    
    /*
    **  A    B
    **
    **  
    **  C    D
    */
    
        a.x = xmin;
        a.y = ymin + char_height;
        b.x = xmin + char_width;
        b.y = ymin + char_height;
        c.x = xmin;
        c.y = ymin;
        d.x = xmin + char_width;
        d.y = ymin;
        a.oow = b.oow = c.oow = d.oow = 1.0f;
    #ifdef GLIDE3
    	a.oow0 = b.oow0 = 
        c.oow0 = d.oow0 = 1.0f;
    #else 
        a.tmuvtx[0].oow = b.tmuvtx[0].oow = 
        c.tmuvtx[0].oow = d.tmuvtx[0].oow = 1.0f;
    #endif
        a.ooz = b.ooz = c.ooz = d.ooz = 0.0f;
    
    	a.r = 255.0f; a.g = 255.0f; a.b = 255.0f;
    	b.r = 255.0f; b.g = 255.0f; b.b = 255.0f;
    	c.r = 255.0f; c.g = 255.0f; c.b = 255.0f;
    	d.r = 255.0f; d.g = 255.0f; d.b = 255.0f;
    
        if ( textMaterial->chromaKeyEnable != !opaque ) {
            textMaterial->chromaKeyEnable = !opaque;
            /* probably not necessary for now, but maybe in the future */
            atrMaterialModify(textMaterial);
        }
    
        atrPushMaterial( textMaterial );
    
        _atrTexGetScale( 0, &sScale, &tScale );
    
        while( *string ) {
            char *offset;
            offset = strchr( charMap, *string );
            if ( !offset ) offset = charMap;
            _mapIndexToTexCoords( &s0, &t0, &s1, &t1, offset - charMap );
    #ifdef GLIDE3
            a.s0 = c.s0 = s0;
            a.t0 = b.t0 = t0;
            b.s0 = d.s0 = s1;
            c.t0 = d.t0 = t1;
    #else
            a.tmuvtx[0].sow = c.tmuvtx[0].sow = s0;
            a.tmuvtx[0].tow = b.tmuvtx[0].tow = t0;
            b.tmuvtx[0].sow = d.tmuvtx[0].sow = s1;
            c.tmuvtx[0].tow = d.tmuvtx[0].tow = t1;
    #endif
            DRV_FUNC(DrawTriangle)( &a, &b, &c );
            DRV_FUNC(DrawTriangle)( &b, &d, &c );
            a.x += char_width, b.x += char_width; 
            c.x += char_width, d.x += char_width;
            string++;
        }
    
        atrPopMaterial( 0 );    
    }
    
    void atrDrawString( char *string, 
                     float char_width, 
                     float char_height,
                     float xmin,
                     float ymin ) {
    
        _atrDrawString( string, char_width, char_height, xmin, ymin, FXFALSE );
    }
    
    
    void atrDrawStringOpaque( char *string, 
                     float char_width, 
                     float char_height,
                     float xmin,
                     float ymin ) {
    
        _atrDrawString( string, char_width, char_height, xmin, ymin, FXTRUE );
    }
