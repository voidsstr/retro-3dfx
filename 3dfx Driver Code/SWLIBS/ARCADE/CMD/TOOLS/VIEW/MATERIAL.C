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
** $Date: 10/11/00 7:30:25 PM$ 
**
*/

#include <atscene.h>
#define  _WIN32_LEAN_AND_MEAN_
#include <windows.h>
#include <atinput.h>
#include <atdemo.h>
#include <glide.h>
#include <texus.h>
#include "view.h"
#include "resource.h"

#define OFFSET 2

/* number of pages in the material property sheet */

#define LIGHTING_PAGE   0
#define TEXATTRIB_PAGE  1
#define TEXTURE0_PAGE   2
#define TEXTURE1_PAGE   3
#define CHROMAKEY_PAGE  4
#define PAGENUM 5

static EnumEntry textureFormats[] = { 
                          { ATR_IMGFMT_RGB_332, "RGB 332" },
                          { ATR_IMGFMT_YIQ_422, "YIQ 422" },
                          { ATR_IMGFMT_A_8, "A 8" },
                          { ATR_IMGFMT_I_8, "I 8", },
                          { ATR_IMGFMT_AI_44, "AI 44" },
                          { ATR_IMGFMT_P_8, "P 8" },
                          { ATR_IMGFMT_ARGB_8332, "ARGB 8332" },
                          { ATR_IMGFMT_AYIQ_8422, "AYIQ 8422" },
                          { ATR_IMGFMT_RGB_565, "RGB 565" },
                          { ATR_IMGFMT_ARGB_1555, "ARGB 1555" },
                          { ATR_IMGFMT_ARGB_4444, "ARGB 4444" },
                          { ATR_IMGFMT_AI_88, "AI 88" },
                          { ATR_IMGFMT_AP_88, "AP 88" },
                          { 0, NULL }};

static EnumEntry lightingModes[] = { 
                          { ATR_MAT_LIGHTSRC_NONE,  "None" },
                          { ATR_MAT_LIGHTSRC_LIGHT, "Dynamic" },
                          { ATR_MAT_LIGHTSRC_STATIC, "Precomputed" },
                          { ATR_MAT_LIGHTSRC_CONSTANT, "Constant" },
                          { 0, NULL }};

static EnumEntry colorCombineModes[] = { 
                          { ATR_MAT_LIGHTING_NONE,  "None" },
                          { ATR_MAT_LIGHTING_MULTIPLY,  "Multiply" },
                          { ATR_MAT_LIGHTING_ADD,  "Add" },
                          { ATR_MAT_LIGHTING_BLEND_ON_TEXALPHA,  "Blend on Texture Alpha" },
                          { 0, NULL }};

static EnumEntry filterModes[] = { 
                          { ATR_TEXFILTER_POINT_SAMPLED, "Point Sampled" },
                          { ATR_TEXFILTER_BILINEAR, "Bilinear" }, 
                          { 0, NULL }};

static EnumEntry clampModes[] = { 
                          { ATR_TEXCLAMP_WRAP, "Wrap" },
                          { ATR_TEXCLAMP_CLAMP, "Clamp" }, 
                          { 0, NULL }};

static EnumEntry mipmapModes[] = { 
                          { ATR_TEXMIPMAP_DISABLE, "Disable" },
                          { ATR_TEXMIPMAP_NEAREST, "Nearest" }, 
                          { ATR_TEXMIPMAP_NEAREST_DITHER, "Nearest Dither" }, 
                          { 0, NULL }};

static EnumEntry texSrcModes[] = { 
                          { ATR_TEXSRC_NONE, "None" },
                          { ATR_TEXSRC_DECAL, "Decal" },
                          { ATR_TEXSRC_DETAIL, "Detail" },
                          { ATR_TEXSRC_EMAP, "Environment Map" },
                          { ATR_TEXSRC_LMAP, "Lighting Map" },
                          { ATR_TEXSRC_PROJECTED, "Projected" },
                          { 0, NULL }};

static EnumEntry tcSrcModes[] = { 
                          { ATR_TCSRC_NONE, "None" },
                          { ATR_TCSRC_TC0, "TCO" },
                          { ATR_TCSRC_TC1, "TC1" },
                          { ATR_TCSRC_EMAP, "Environment Map" },
                          { ATR_TCSRC_LMAP, "Lighting Map" },
                          { ATR_TCSRC_PROJECTED, "Projected" },
                          { ATR_TCSRC_PLANAR, "Planar" },
                          { 0, NULL }};

static EnumEntry textureModes[] = { 
                          { ATR_MAT_TEX_NONE, "None" },
                          { ATR_MAT_TEX_DECAL, "Decal" },
                          { ATR_MAT_TEX_DECAL1, "Decal 1" },
                          { ATR_MAT_TEX_LODBLEND_PASS0, "LOD Blend Pass 0" },
                          { ATR_MAT_TEX_LODBLEND_PASS1, "LOD Blend Pass 1" },
                          { ATR_MAT_TEX_LODBLEND_SINGLEPASS, "LOD Blend Single Pass" },
                          { ATR_MAT_TEX_DETAIL_PASS0, "Detail Pass 0" },
                          { ATR_MAT_TEX_DETAIL_PASS1, "Detail Pass 1" },
                          { ATR_MAT_TEX_DETAIL_SINGLEPASS, "Detail Single Pass" },
                          { ATR_MAT_TEX_EMAP, "Environment Map" },
                          { ATR_MAT_TEX_LMAP, "Lighting Map" },
                          { ATR_MAT_TEX_PROJECTED, "Projection Map" },
                          { ATR_MAT_TEX_DECAL_X_PROJECTED, "Decal x Projection Map" },
                          { ATR_MAT_TEX_PLANAR, "Planar Map" },
                          { ATR_MAT_TEX_DECAL_X_PLANAR, "Decal x Planar Map" },
                          { ATR_MAT_TEX_DECAL_ADD_PLANAR, "Decal + Planar Map" },
                          { 0, NULL }};

/*---------------------------------------------------------------------
  Control Data
  ---------------------------------------------------------------------*/
static HBITMAP     texture0;

void setEnum( HWND dlg, FxU32 id, EnumEntry *edef, FxU32 cur) {
    HWND  hComboBox = GetDlgItem( dlg, id);
    int i = 0;
    int curIdx ;

    ComboBox_ResetContent(hComboBox);

    while ( edef->name != NULL ) {
        ComboBox_InsertString(hComboBox, i, edef->name);
        if ( edef->val == cur )
            curIdx = i;
        edef++;
        i++;
    }

    ComboBox_SetCurSel(hComboBox, curIdx);
}

void setBool( HWND dlg, FxU32 id, FxBool cur) {
    HWND  hButton = GetDlgItem( dlg, id);

    Button_SetCheck(hButton, cur);
}

void setText( HWND dlg, FxU32 id, const char *txt ) {
    HWND  handle;

    handle = GetDlgItem( dlg, id );
    SendMessage( handle, WM_SETTEXT, (WPARAM)0, (LPARAM)txt );
}

void setFloat( HWND dlg, FxU32 id, float val ) {
    HWND  handle;
    char buffer[64];

    handle = GetDlgItem( dlg, id );
    sprintf( buffer, "%1.3f", val);
    SendMessage( handle, WM_SETTEXT, (WPARAM)0, (LPARAM)buffer );
}

void setFxU32( HWND dlg, FxU32 id, FxU32 val ) {
    HWND  handle;
    char buffer[64];

    handle = GetDlgItem( dlg, id );
    sprintf( buffer, "0x%08x", val);
    SendMessage( handle, WM_SETTEXT, (WPARAM)0, (LPARAM)buffer );
}

void setInt( HWND dlg, FxU32 id, int val ) {
    HWND  handle;
    char buffer[64];

    handle = GetDlgItem( dlg, id );
    sprintf( buffer, "%4d", val);
    SendMessage( handle, WM_SETTEXT, (WPARAM)0, (LPARAM)buffer );
}

char *enumFindString(FxU32 val, EnumEntry *edef) {
    while ( edef->name != NULL ) {
        if ( edef->val == val )
            return edef->name;
        edef++;
    }

    return "Unknown!!";
}

/*---------------------------------------------------------------------
  Control Setup Function
  ---------------------------------------------------------------------*/
static void setupTextureAttributes( HWND dlg ) {
    AtsMaterial *material = (AtsMaterial *)GetWindowLong(dlg, GWL_USERDATA);
    AtrMaterial *mat = (AtrMaterial *)material;

    setEnum( dlg, IDC_TEXTURE_MODE, textureModes, 
               mat->typeFlag & ATR_MAT_TEX_MASK);

    setEnum( dlg, IDC_SCLAMP0, clampModes, mat->texSClamp[0]);
    setEnum( dlg, IDC_SCLAMP1, clampModes, mat->texSClamp[1]);
    setEnum( dlg, IDC_TCLAMP0, clampModes, mat->texTClamp[0]);
    setEnum( dlg, IDC_TCLAMP1, clampModes, mat->texTClamp[1]);

    setEnum( dlg, IDC_MIPMAP0, mipmapModes, mat->texMMMode[0]);
    setEnum( dlg, IDC_MIPMAP1, mipmapModes, mat->texMMMode[1]);

    setEnum( dlg, IDC_MIN_FILTER0, filterModes, mat->texMinFilter[0]);
    setEnum( dlg, IDC_MIN_FILTER1, filterModes, mat->texMinFilter[1]);
    setEnum( dlg, IDC_MAG_FILTER0, filterModes, mat->texMagFilter[0]);
    setEnum( dlg, IDC_MAG_FILTER1, filterModes, mat->texMagFilter[1]);

    setBool( dlg, IDC_LODBLEND0, mat->texLODBlend[0]);
    setBool( dlg, IDC_LODBLEND1, mat->texLODBlend[1]);

    setEnum( dlg, IDC_TEXSRC0, texSrcModes, mat->texSrc[0]);
    setEnum( dlg, IDC_TEXSRC1, texSrcModes, mat->texSrc[1]);

    setEnum( dlg, IDC_TCSRC0, tcSrcModes, mat->tcSrc[0]);
    setEnum( dlg, IDC_TCSRC1, tcSrcModes, mat->tcSrc[1]);
}

static void setupTexture( HWND dlg ) {
    AtsTexture *tex = (AtsTexture*)GetWindowLong(dlg, GWL_USERDATA);
    AtrImg     *img = tex->img[0];
    int        nCmdShow = SW_HIDE ;

    if ( tex == NULL )
        return;

    setText( dlg, IDC_TEXTURE_NAME, img->name );
    setInt( dlg, IDC_TEXTURE_WIDTH, img->width );
    setInt( dlg, IDC_TEXTURE_HEIGHT, img->height );
    setInt( dlg, IDC_TEXTURE_NLOD, img->nLevels );
    setText( dlg, IDC_TEXTURE_FORMAT, enumFindString(img->format, 
                                                    textureFormats ));
    setInt( dlg, IDC_TEXNUMFRAMES, tex->numImages );

    /* setup controls for texture player */

    if ( tex->numImages > 0 ) {
        SendMessage(CTRL(dlg, IDC_TMOVIE_PLAY), BM_SETIMAGE, 0,
           (LPARAM)LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_PLAY)));
        SendMessage(CTRL(dlg, IDC_TMOVIE_STOP), BM_SETIMAGE, 0,
           (LPARAM)LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_STOP)));
        SendMessage(CTRL(dlg, IDC_TMOVIE_PAUSE), BM_SETIMAGE, 0,
           (LPARAM)LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_PAUSE)));
        SendMessage(CTRL(dlg, IDC_TMOVIE_PREV), BM_SETIMAGE, 0,
           (LPARAM)LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_PREV)));
        SendMessage(CTRL(dlg, IDC_TMOVIE_NEXT), BM_SETIMAGE, 0,
           (LPARAM)LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_NEXT)));
        setInt( dlg, IDC_TEXCURFRAME, 0 );
        nCmdShow = SW_SHOWNORMAL;
    } 

    ShowWindow( CTRL(dlg, IDC_TMOVIE_PLAY), nCmdShow);
    ShowWindow( CTRL(dlg, IDC_TMOVIE_STOP), nCmdShow);
    ShowWindow( CTRL(dlg, IDC_TMOVIE_PAUSE), nCmdShow);
    ShowWindow( CTRL(dlg, IDC_TMOVIE_PREV), nCmdShow);
    ShowWindow( CTRL(dlg, IDC_TMOVIE_NEXT), nCmdShow);
    ShowWindow( CTRL(dlg, IDC_TEXCURFRAME_L), nCmdShow);
    ShowWindow( CTRL(dlg, IDC_TEXCURFRAME), nCmdShow);
    ShowWindow( CTRL(dlg, IDC_TEXNUMFRAMES_L), nCmdShow);
    ShowWindow( CTRL(dlg, IDC_TEXNUMFRAMES), nCmdShow);
 
    texture0 = imageToDIB(dlg, img);

    // Display Bitmap
    SendMessage( GetDlgItem( dlg, IDC_DISPLAY ),
                 STM_SETIMAGE,
                 (WPARAM)IMAGE_BITMAP,
                 (LPARAM)texture0 );
}

static void setupLightingAttributes( HWND dlg ) {
    AtsMaterial *material = (AtsMaterial *)GetWindowLong(dlg, GWL_USERDATA);

    setEnum( dlg, IDC_LIGHT_MODE, lightingModes, 
               material->material.typeFlag &  ATR_MAT_LIGHTSRC_MASK);
    setEnum( dlg, IDC_COLOR_COMBINE, colorCombineModes,
               material->material.typeFlag &  ATR_MAT_LIGHTING_MASK);
    setFloat( dlg, IDC_DIFFUSE_R, material->material.diffuse.r);
    setFloat( dlg, IDC_DIFFUSE_G, material->material.diffuse.g);
    setFloat( dlg, IDC_DIFFUSE_B, material->material.diffuse.b);
    setFloat( dlg, IDC_SPECULAR_R, material->material.specular.r);
    setFloat( dlg, IDC_SPECULAR_G, material->material.specular.g);
    setFloat( dlg, IDC_SPECULAR_B, material->material.specular.b);
    setFloat( dlg, IDC_EMISSIVE_R, material->material.emissive.r);
    setFloat( dlg, IDC_EMISSIVE_G, material->material.emissive.g);
    setFloat( dlg, IDC_EMISSIVE_B, material->material.emissive.b);
    setInt( dlg, IDC_SPECEXP, material->material.specExponent);
    setFxU32( dlg, IDC_CCRGB, material->material.constant);
    setFxU32( dlg, IDC_PRJTAG, material->material.projectedTag);
}

static void setupChromakey( HWND dlg ) {
    AtsMaterial *material = (AtsMaterial *)GetWindowLong(dlg, GWL_USERDATA);
    setBool( dlg, IDC_CHROMAKEY_ENABLED, material->material.chromaKeyEnable);
    setFxU32( dlg, IDC_CHROMAKEY_COLOR, material->material.chromaKeyValue);
}

static BOOL WINAPI LightingProc( HWND dlg, UINT msg,
                                WPARAM wParam, LPARAM lParam) {
    switch( msg) {
        case WM_INITDIALOG:
        {
			LPPROPSHEETPAGE pPropPage = ( LPPROPSHEETPAGE )lParam;
            SetWindowLong(dlg, GWL_USERDATA, pPropPage->lParam);
            setupLightingAttributes( dlg );
            break;
        }

        case WM_COMMAND:
        break;
    }
    return FALSE ;
}

static HBITMAP getBitmap( WPARAM wParam ) {
    switch ( wParam ) {
    case IDC_TMOVIE_PLAY:
        return LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_PLAY));
    case IDC_TMOVIE_STOP:
        return LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_STOP));
    case IDC_TMOVIE_PAUSE:
        return LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_PAUSE));
    case IDC_TMOVIE_PREV:
        return LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_PREV));
    case IDC_TMOVIE_NEXT:
        return LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_NEXT));
    default:
        atuError(FXFALSE, "Unknown item %d", wParam);
        return LoadBitmap( _atGlobals.hInstApp, MAKEINTRESOURCE(IDB_PLAY));
    }
}

BOOL DrawBitmap( HDC hdc, int x, int y, HBITMAP hbm) {
    HBITMAP hbmpOld ;
    BITMAP bm ;
    HDC hdcMem ;

    /* dc compatibile */

    hdcMem = CreateCompatibleDC( hdc) ;
    GetObject( hbm, sizeof( BITMAP), (LPSTR)&bm) ;

    hbmpOld = SelectObject( hdcMem, hbm) ;

    BitBlt( hdc, x, y, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY) ;
    SelectObject( hdc, hbmpOld) ;
    return DeleteDC( hdcMem) ;
}

static BOOL WINAPI TextureProc(    HWND dlg, UINT msg,
                                WPARAM wParam, LPARAM lParam) {
    switch( msg) {
        case WM_INITDIALOG: {
			LPPROPSHEETPAGE pPropPage = ( LPPROPSHEETPAGE )lParam;
            SetWindowLong(dlg, GWL_USERDATA, pPropPage->lParam);
            setupTexture( dlg);
        }
        break;

        case WM_COMMAND:
        {
            AtsTexture *tex = (AtsTexture*)GetWindowLong(dlg, GWL_USERDATA);
        }
        break;

        case WM_DRAWITEM:
        {
            AtsTexture *tex = (AtsTexture*)GetWindowLong(dlg, GWL_USERDATA);
            LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam ;
            HBITMAP bm = getBitmap( wParam );

            if (( lpdis -> itemAction & ODA_DRAWENTIRE )
                || (lpdis -> itemAction & ODA_SELECT )
                    && (! (lpdis -> itemState & ODS_SELECTED))) {
                    RoundRect(  lpdis -> hDC,
                                lpdis -> rcItem.left,
                                lpdis -> rcItem.top,
                                lpdis -> rcItem.right,
                                lpdis -> rcItem.bottom,
                                2, 2) ;

                    /* show the bitmap */

                    DrawBitmap( lpdis -> hDC,
                                lpdis -> rcItem.left + OFFSET / 2,
                                lpdis -> rcItem.top + OFFSET / 2,
                                bm) ;
            }

            if (( lpdis -> itemAction & ODA_SELECT) && 
                (lpdis -> itemState & ODS_SELECTED) ) {
                    RoundRect(  lpdis -> hDC,
                                lpdis -> rcItem.left,
                                lpdis -> rcItem.top,
                                lpdis -> rcItem.right,
                                lpdis -> rcItem.bottom,
                                2, 2) ;

                    /* show the bitmap */

                    DrawBitmap( lpdis -> hDC,
                                lpdis -> rcItem.left + OFFSET / 2,
                                lpdis -> rcItem.top + OFFSET / 2,
                                bm) ;
            }
        }
        break ;
    }

    return FALSE ;
}

static BOOL WINAPI ChromakeyProc(    HWND dlg, UINT msg,
                                WPARAM wParam, LPARAM lParam) {
    switch( msg) {
        case WM_INITDIALOG: {
			LPPROPSHEETPAGE pPropPage = ( LPPROPSHEETPAGE )lParam;
            SetWindowLong(dlg, GWL_USERDATA, pPropPage->lParam);
            setupChromakey( dlg );
        }
        break;

        case WM_COMMAND:
        break;
    }
    return FALSE ;
}

static BOOL WINAPI TexAttribProc(    HWND dlg, UINT msg,
                                WPARAM wParam, LPARAM lParam) {
    switch( msg) {
        case WM_INITDIALOG: {
			LPPROPSHEETPAGE pPropPage = ( LPPROPSHEETPAGE )lParam;
            SetWindowLong(dlg, GWL_USERDATA, pPropPage->lParam);
            setupTextureAttributes( dlg );
        }
        break;

        case WM_COMMAND:
        break;
    }
    return FALSE ;
}

BOOL WINAPI PropSheetDlgProc( HWND hwnd, UINT msg, LPARAM lParam) {
    switch( msg) {
        case PSCB_INITIALIZED:
        {
        }
            return FALSE ;

        case PSCB_PRECREATE:
        {
        }
            break ;
    }
    return TRUE ;
}

static UINT CALLBACK
propPageCallback(HWND hWnd, UINT message, LPPROPSHEETPAGE pPropPage) {
    switch (message) {
    case PSPCB_CREATE:
        return TRUE;

    case PSPCB_RELEASE:
        break;
    }

    return 0;
}

/*---------------------------------------------------------------------
  User Entry Point
  ---------------------------------------------------------------------*/
void *
materialEdit( void *object, HWND parent ) {
    AtsMaterial *material;
    FxU32     done = FXFALSE;
    PROPSHEETPAGE psp[ PAGENUM] ;
    PROPSHEETHEADER psh ;
    int i = 0;

    material = (AtsMaterial*)object;

    /* material lighting properties */

    psp[i].dwSize = sizeof( PROPSHEETPAGE) ;
    psp[i].dwFlags = PSP_USETITLE ;
    psp[i].hInstance = _atGlobals.hInstApp;
    psp[i].pszTemplate = MAKEINTRESOURCE(IDD_LIGHTING);
    psp[i].pfnDlgProc = LightingProc ;
    psp[i].pszTitle = "Lighting" ;
    psp[i].lParam = (LPARAM)material;
    i++;

    /* texture attribute properties */

    if (( material->textures[0] ) || ( material->textures[1] )) {
        psp[i].dwSize = sizeof( PROPSHEETPAGE) ;
        psp[i].dwFlags = PSP_USETITLE ;
        psp[i].hInstance = _atGlobals.hInstApp;
        psp[i].pszTemplate = MAKEINTRESOURCE(IDD_TEXATTRIB);
        psp[i].pfnDlgProc = TexAttribProc ;
        psp[i].pszTitle = "Texture Attributes" ;
        psp[i].lParam = (LPARAM)material;
        i++;
    }

    /* texture 0 properties */

    if ( material->textures[0] ) {
        psp[i].dwSize = sizeof( PROPSHEETPAGE) ;
        psp[i].dwFlags = PSP_USETITLE|PSP_USECALLBACK;
        psp[i].hInstance = _atGlobals.hInstApp;
        psp[i].pszTemplate = MAKEINTRESOURCE(IDD_TEXTURE0);
        psp[i].pfnDlgProc = TextureProc ;
        psp[i].pszTitle = "Texture 0" ;
        psp[i].lParam = (LPARAM)material->textures[0];
		psp[i].pfnCallback = propPageCallback;
        i++;
    }

    /* texture 1 properties */

    if ( material->textures[1] ) {
        psp[i].dwSize = sizeof( PROPSHEETPAGE) ;
        psp[i].dwFlags = PSP_USETITLE ;
        psp[i].hInstance = _atGlobals.hInstApp;
        psp[i].pszTemplate = MAKEINTRESOURCE(IDD_TEXTURE1);
        psp[i].pfnDlgProc = TextureProc ;
        psp[i].pszTitle = "Texture 1" ;
        psp[i].lParam = (LPARAM)material->textures[1];
        i++;
    }

    /* chromakey properties */

    psp[i].dwSize = sizeof( PROPSHEETPAGE) ;
    psp[i].dwFlags = PSP_USETITLE ;
    psp[i].hInstance = _atGlobals.hInstApp;
    psp[i].pszTemplate = MAKEINTRESOURCE(IDD_CHROMAKEY);
    psp[i].pfnDlgProc = ChromakeyProc ;
    psp[i].pszTitle = "Chromakey" ;
    psp[i].lParam = (LPARAM)material;
    i++;

    psh.dwSize = sizeof( PROPSHEETHEADER) ;
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USECALLBACK ;
    psh.hwndParent = parent ;
    psh.hInstance = _atGlobals.hInstApp;
    psh.nStartPage = 0 ;
    psh.pfnCallback = PropSheetDlgProc ;
    psh.pszCaption = (LPSTR)"Material" ;
    psh.nPages = i;
    psh.ppsp = (LPCPROPSHEETPAGE)&psp ;

    // creating a property sheet
    if( PropertySheet( &psh ) == -1)
        MessageBeep( 0) ;
    if ( texture0 ) DeleteObject( texture0 );
    return object;
}
