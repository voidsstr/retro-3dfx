/* d3dlab - minimal windowed D3D8 texture-stage lab for the 3dfx XP driver.
 * Usage: d3dlab.exe <mode>
 *   sel1     stage0 SELECTARG1 (texture only)
 *   mod      stage0 MODULATE texture x diffuse (diffuse=white)
 *   modgray  stage0 MODULATE texture x diffuse (diffuse=gray)
 *   mod2x    stage0 MODULATE2X
 *   spec     FVF includes SPECULAR; stage0 MODULATE
 *   tex2     two stages: 0 MODULATE(tex,diffuse), 1 MODULATE(tex2,current)
 *   tex2sel  two stages: 0 SELECTARG1, 1 SELECTARG2(current) passthrough
 * Renders 5 seconds then exits. Title bar shows mode. */
#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <string.h>

#pragma pack(push,1)
typedef struct { float x,y,z,rhw; DWORD dif; float u,v; } VTX1;
typedef struct { float x,y,z,rhw; DWORD dif; DWORD spec; float u,v; } VTXS;
typedef struct { float x,y,z,rhw; DWORD dif; float u,v,u2,v2; } VTX2;
#pragma pack(pop)

static LRESULT CALLBACK wndproc(HWND h,UINT m,WPARAM w,LPARAM l)
{ if(m==WM_DESTROY){PostQuitMessage(0);return 0;} return DefWindowProc(h,m,w,l); }

static IDirect3DTexture8 *mktex2(IDirect3DDevice8 *dev, int size, int levels, DWORD c0, DWORD c1)
{
    IDirect3DTexture8 *t=NULL; D3DLOCKED_RECT lr; int x,y;
    DWORD nlev,lev,sz;
    if(FAILED(IDirect3DDevice8_CreateTexture(dev,size,size,levels,0,D3DFMT_R5G6B5,D3DPOOL_MANAGED,&t)))
        return NULL;
    nlev=IDirect3DTexture8_GetLevelCount(t);
    sz=size;
    for(lev=0;lev<nlev;lev++){
        if(SUCCEEDED(IDirect3DTexture8_LockRect(t,lev,&lr,NULL,0))){
            for(y=0;y<(int)sz;y++){ unsigned short *row=(unsigned short*)((char*)lr.pBits+y*lr.Pitch);
                for(x=0;x<(int)sz;x++){ DWORD c=(((x>>3)+(y>>3))&1)?c0:c1;
                    if(lev==1)c=0x00FFFF00; if(lev==2)c=0x0000FFFF; if(lev>=3)c=0x00FF00FF;
                    row[x]=(unsigned short)((((c>>16&0xFF)>>3)<<11)|(((c>>8&0xFF)>>2)<<5)|((c&0xFF)>>3)); } }
            IDirect3DTexture8_UnlockRect(t,lev);
        }
        if(sz>1)sz>>=1;
    }
    return t;
}
static IDirect3DTexture8 *mktex(IDirect3DDevice8 *dev, DWORD c0, DWORD c1)
{ return mktex2(dev,64,1,c0,c1); }

/* DXT1 checker texture: 4x4-pixel blocks, each block a solid color (both
 * endpoints equal, indices 0). Alternating blocks = c0/c1 checker. This
 * exercises the Napalm-only compressed-texture download path (the historical
 * hard-freeze suspect; only advertised when IS_NAPALM). size must be mult of 4. */
static unsigned short to565(DWORD c)
{ return (unsigned short)((((c>>16&0xFF)>>3)<<11)|(((c>>8&0xFF)>>2)<<5)|((c&0xFF)>>3)); }
static IDirect3DTexture8 *mkdxt1(IDirect3DDevice8 *dev, int size, DWORD c0, DWORD c1)
{
    IDirect3DTexture8 *t=NULL; D3DLOCKED_RECT lr; int bx,by; int blocks=size/4;
    if(FAILED(IDirect3DDevice8_CreateTexture(dev,size,size,1,0,D3DFMT_DXT1,D3DPOOL_MANAGED,&t)))
        return NULL;
    if(SUCCEEDED(IDirect3DTexture8_LockRect(t,0,&lr,NULL,0))){
        /* DXT1 block = 8 bytes: [c0:16][c1:16][indices:32]. Pitch is bytes/row-of-blocks. */
        for(by=0;by<blocks;by++){
            unsigned char *row=(unsigned char*)lr.pBits + by*lr.Pitch;
            for(bx=0;bx<blocks;bx++){
                unsigned char *blk=row+bx*8;
                DWORD c=((bx+by)&1)?c1:c0;
                unsigned short e=to565(c);
                /* both endpoints equal + all-zero indices -> solid color e.
                 * Keep e0>=e1 (equal is fine) so it's opaque 4-color mode. */
                blk[0]=e&0xFF; blk[1]=e>>8; blk[2]=e&0xFF; blk[3]=e>>8;
                blk[4]=blk[5]=blk[6]=blk[7]=0;
            }
        }
        IDirect3DTexture8_UnlockRect(t,0);
    }
    return t;
}

/* Mip-capable DXT1 builder in an arbitrary pool. levels=0 -> full chain.
 * Lockable pools get every level filled: level 0 = c0/c1 checker, deeper
 * levels solid (yellow/cyan/magenta) like mktex2. POOL_DEFAULT is left
 * unfilled (not lockable) - fill a SYSTEMMEM twin and UpdateTexture it. */
static IDirect3DTexture8 *mkdxt1p(IDirect3DDevice8 *dev,int size,int levels,D3DPOOL pool,DWORD c0,DWORD c1)
{
    IDirect3DTexture8 *t=NULL; D3DLOCKED_RECT lr; int bx,by,lev; DWORD nlev; int sz;
    if(FAILED(IDirect3DDevice8_CreateTexture(dev,size,size,levels,0,D3DFMT_DXT1,pool,&t)))
        return NULL;
    if(pool==D3DPOOL_DEFAULT) return t;
    nlev=IDirect3DTexture8_GetLevelCount(t);
    sz=size;
    for(lev=0;lev<(int)nlev;lev++){
        int blocks=(sz+3)/4;
        if(SUCCEEDED(IDirect3DTexture8_LockRect(t,lev,&lr,NULL,0))){
            for(by=0;by<blocks;by++){
                unsigned char *row=(unsigned char*)lr.pBits+by*lr.Pitch;
                for(bx=0;bx<blocks;bx++){
                    unsigned char *blk=row+bx*8;
                    DWORD c=((bx+by)&1)?c1:c0;
                    unsigned short e;
                    if(lev==1)c=0x00FFFF00; if(lev==2)c=0x0000FFFF; if(lev>=3)c=0x00FF00FF;
                    e=to565(c);
                    blk[0]=e&0xFF; blk[1]=e>>8; blk[2]=e&0xFF; blk[3]=e>>8;
                    blk[4]=blk[5]=blk[6]=blk[7]=0;
                }
            }
            IDirect3DTexture8_UnlockRect(t,lev);
        }
        if(sz>1)sz>>=1;
    }
    return t;
}

int main(int argc,char**argv)
{
    const char *mode = argc>1?argv[1]:"sel1";
    char title[128];
    WNDCLASSA wc; HWND hwnd; MSG msg; DWORD t0;
    IDirect3D8 *d3d; IDirect3DDevice8 *dev=NULL;
    D3DPRESENT_PARAMETERS pp; D3DDISPLAYMODE dm;
    IDirect3DTexture8 *texA,*texB;
    HRESULT hr;

    _snprintf(title,sizeof(title),"D3DLAB-%s",mode);
    memset(&wc,0,sizeof(wc));
    wc.lpfnWndProc=wndproc; wc.hInstance=GetModuleHandle(NULL);
    wc.lpszClassName="d3dlab"; wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    RegisterClassA(&wc);
    hwnd=CreateWindowA("d3dlab",title,WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                       60,60,400,340,NULL,NULL,wc.hInstance,NULL);

    d3d=Direct3DCreate8(D3D_SDK_VERSION);
    if(!d3d){ printf("no d3d8\n"); return 1; }
    IDirect3D8_GetAdapterDisplayMode(d3d,0,&dm);
    memset(&pp,0,sizeof(pp));
    pp.Windowed=TRUE; pp.SwapEffect=D3DSWAPEFFECT_COPY; pp.BackBufferFormat=dm.Format;
    pp.EnableAutoDepthStencil=FALSE;
    hr=IDirect3D8_CreateDevice(d3d,0,D3DDEVTYPE_HAL,hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,&dev);
    if(FAILED(hr)){ printf("CreateDevice failed %08lx\n",(unsigned long)hr); return 2; }

    texA=mktex(dev,0x00FF4040,0x0040FF40);   /* red/green checker */
    texB=mktex(dev,0x004040FF,0x00FFFFFF);   /* blue/white checker */

    IDirect3DDevice8_SetRenderState(dev,D3DRS_LIGHTING,FALSE);
    IDirect3DDevice8_SetRenderState(dev,D3DRS_CULLMODE,D3DCULL_NONE);
    IDirect3DDevice8_SetRenderState(dev,D3DRS_ZENABLE,FALSE);
    IDirect3DDevice8_SetTexture(dev,0,(IDirect3DBaseTexture8*)texA);
    IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_MINFILTER,D3DTEXF_POINT);
    IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_MAGFILTER,D3DTEXF_POINT);

    if(!strcmp(mode,"dxt1up")){
        /* Regression for the D3DDP2OP_TEXBLT FourCC crash (bugcheck 8E in
         * Blt32_CopyFourCC called through the 7-arg PTEXBLTFUNC cast):
         * UpdateTexture from a SYSTEMMEM mipped DXT1 chain to a
         * POOL_DEFAULT one forces the runtime to emit TEXBLT per level,
         * including the tiny-LOD DXT1 paths. Pre-fix this bugchecks the
         * box; post-fix it renders the same checker as dxt1. */
        IDirect3DTexture8 *ts=mkdxt1p(dev,64,0,D3DPOOL_SYSTEMMEM,0x00FF4040,0x0040FF40);
        IDirect3DTexture8 *td=mkdxt1p(dev,64,0,D3DPOOL_DEFAULT,0,0);
        if(ts&&td&&SUCCEEDED(IDirect3DDevice8_UpdateTexture(dev,
                (IDirect3DBaseTexture8*)ts,(IDirect3DBaseTexture8*)td))){
            IDirect3DDevice8_SetTexture(dev,0,(IDirect3DBaseTexture8*)td);
            IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
            IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
        }
        /* on any failure the quad keeps the clear color -> golden mismatch */
    } else if(!strcmp(mode,"dxt1")||!strcmp(mode,"dxt1big")){
        IDirect3DTexture8 *td=mkdxt1(dev,!strcmp(mode,"dxt1big")?256:64,0x00FF4040,0x0040FF40);
        if(td){
            IDirect3DDevice8_SetTexture(dev,0,(IDirect3DBaseTexture8*)td);
            IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
            IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
        } else {
            /* DXT1 unsupported/creation failed: leave stage default so the quad
             * shows the clear color (distinct from a correct red/green checker). */
        }
    } else if(!strcmp(mode,"mippoint")||!strcmp(mode,"miplinear")||!strcmp(mode,"mipfar")){
        IDirect3DTexture8 *tm=mktex2(dev,64,0,0x00FF4040,0x0040FF40);
        IDirect3DDevice8_SetTexture(dev,0,(IDirect3DBaseTexture8*)tm);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_MIPFILTER,
            !strcmp(mode,"miplinear")?D3DTEXF_LINEAR:D3DTEXF_POINT);
    } else if(!strcmp(mode,"big512")||!strcmp(mode,"big512mip")){
        IDirect3DTexture8 *tb=mktex2(dev,512,!strcmp(mode,"big512mip")?0:1,0x00FF4040,0x0040FF40);
        IDirect3DDevice8_SetTexture(dev,0,(IDirect3DBaseTexture8*)tb);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
        if(!strcmp(mode,"big512mip"))
            IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_MIPFILTER,D3DTEXF_POINT);
    } else if(!strcmp(mode,"sel1")){
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
    } else if(!strcmp(mode,"mod")||!strcmp(mode,"modgray")||!strcmp(mode,"spec")){
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLOROP,D3DTOP_MODULATE);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
    } else if(!strcmp(mode,"mod2x")){
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLOROP,D3DTOP_MODULATE2X);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
        IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
    } else if(!strcmp(mode,"tex2")||!strcmp(mode,"tex2sel")){
        IDirect3DDevice8_SetTexture(dev,1,(IDirect3DBaseTexture8*)texB);
        IDirect3DDevice8_SetTextureStageState(dev,1,D3DTSS_MINFILTER,D3DTEXF_POINT);
        IDirect3DDevice8_SetTextureStageState(dev,1,D3DTSS_MAGFILTER,D3DTEXF_POINT);
        IDirect3DDevice8_SetTextureStageState(dev,1,D3DTSS_TEXCOORDINDEX,1);
        if(!strcmp(mode,"tex2")){
            IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLOROP,D3DTOP_MODULATE);
            IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
            IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
            IDirect3DDevice8_SetTextureStageState(dev,1,D3DTSS_COLOROP,D3DTOP_MODULATE);
            IDirect3DDevice8_SetTextureStageState(dev,1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
            IDirect3DDevice8_SetTextureStageState(dev,1,D3DTSS_COLORARG2,D3DTA_CURRENT);
        } else {
            IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
            IDirect3DDevice8_SetTextureStageState(dev,0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
            IDirect3DDevice8_SetTextureStageState(dev,1,D3DTSS_COLOROP,D3DTOP_SELECTARG2);
            IDirect3DDevice8_SetTextureStageState(dev,1,D3DTSS_COLORARG2,D3DTA_CURRENT);
        }
    }

    t0=GetTickCount();
    while(GetTickCount()-t0<5000){
        while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessage(&msg);}
        IDirect3DDevice8_Clear(dev,0,NULL,D3DCLEAR_TARGET,0x00202060,1.0f,0);
        if(SUCCEEDED(IDirect3DDevice8_BeginScene(dev))){
            DWORD dif = !strcmp(mode,"modgray")?0xFF808080:0xFFFFFFFF;
            if(!strcmp(mode,"spec")){
                VTXS q[4]={{20,20,0,1,dif,0xFF000000,0,0},{360,20,0,1,dif,0xFF000000,2,0},
                           {20,260,0,1,dif,0xFF000000,0,2},{360,260,0,1,dif,0xFF000000,2,2}};
                IDirect3DDevice8_SetVertexShader(dev,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_SPECULAR|D3DFVF_TEX1);
                IDirect3DDevice8_DrawPrimitiveUP(dev,D3DPT_TRIANGLESTRIP,2,q,sizeof(VTXS));
            } else if(!strncmp(mode,"tex2",4)){
                VTX2 q[4]={{20,20,0,1,dif,0,0,0,0},{360,20,0,1,dif,2,0,2,0},
                           {20,260,0,1,dif,0,2,0,2},{360,260,0,1,dif,2,2,2,2}};
                IDirect3DDevice8_SetVertexShader(dev,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX2);
                IDirect3DDevice8_DrawPrimitiveUP(dev,D3DPT_TRIANGLESTRIP,2,q,sizeof(VTX2));
            } else {
                float uv = strcmp(mode,"mipfar") ? 2.0f : 40.0f;
                VTX1 q[4]={{20,20,0,1,dif,0,0},{360,20,0,1,dif,uv,0},
                           {20,260,0,1,dif,0,uv},{360,260,0,1,dif,uv,uv}};
                IDirect3DDevice8_SetVertexShader(dev,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
                IDirect3DDevice8_DrawPrimitiveUP(dev,D3DPT_TRIANGLESTRIP,2,q,sizeof(VTX1));
            }
            IDirect3DDevice8_EndScene(dev);
        }
        IDirect3DDevice8_Present(dev,NULL,NULL,NULL,NULL);
        Sleep(30);
    }
    printf("d3dlab %s done\n",mode);
    return 0;
}
