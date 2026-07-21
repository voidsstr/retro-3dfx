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

    if(!strcmp(mode,"mippoint")||!strcmp(mode,"miplinear")||!strcmp(mode,"mipfar")){
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
