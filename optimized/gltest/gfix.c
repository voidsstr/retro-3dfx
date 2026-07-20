/* RETRO3DFX minimal text-garble fix harness. Renders:
 *  case A (C:\gfix_A.raw): the EXACT live Q3 menu quads (real xy + real
 *    texel-space texcoords) with the real font atlas -> the garble repro.
 *  case B (C:\gfix_B.raw): 2px-period ALPHA-column pattern, RGB white, modulate
 *    red, SRC_ALPHA blend, NEAREST, 1:1 high-s -> isolates alpha-channel sampling.
 * Small, single-texture, always quits cleanly (no fullscreen-crash hang). */
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include "fontrows.h"
#define W 640
#define H 480
static FILE *lg;
static void dump(const char *nm){
  static unsigned char b[W*H*3]; FILE*f;
  glReadPixels(0,0,W,H,GL_RGB,GL_UNSIGNED_BYTE,b);
  f=fopen(nm,"wb"); if(f){fwrite(b,1,sizeof(b),f);fclose(f);}
  fprintf(lg,"%s err=0x%x\n",nm,(unsigned)glGetError()); fflush(lg);
}
static void quad(float x0,float y0,float x1,float y1,float s0,float t0,float s1,float t1){
  glBegin(GL_QUADS);
  glTexCoord2f(s0,t0); glVertex2f(x0,y0);
  glTexCoord2f(s1,t0); glVertex2f(x1,y0);
  glTexCoord2f(s1,t1); glVertex2f(x1,y1);
  glTexCoord2f(s0,t1); glVertex2f(x0,y1);
  glEnd();
}
int main(void){
  PIXELFORMATDESCRIPTOR pfd; HWND hwnd; HDC dc; HGLRC rc; int pf,qi,y,x;
  static unsigned char fa[256][256][4], chk[256][256][4];
  GLuint ft,ck;
  static const float Q[][8]={
    {222,169.937f,245,196.937f,230,4,253,31},{248,169.937f,266,196.937f,146,34,164,61},
    {269,169.937f,281,196.937f,216,4,228,31},{284,169.937f,298,196.937f,130,34,144,61},
    {301,169.937f,309,196.937f,164,4,172,31},{312,169.937f,330,196.937f,48,34,66,61},
    {333,169.937f,345,196.937f,216,4,228,31},{348,169.937f,366,196.937f,5,4,23,31},
    {369,169.937f,387,196.937f,234,34,252,61},{390,169.937f,403,196.937f,90,4,103,31},
    {406,169.937f,423,196.937f,90,34,107,61}};
  lg=fopen("C:\\gfix.log","w"); if(!lg)return 1;
  hwnd=CreateWindowA("STATIC","gfix",WS_POPUP|WS_VISIBLE,0,0,W,H,0,0,0,0);
  dc=GetDC(hwnd); ZeroMemory(&pfd,sizeof(pfd));
  pfd.nSize=sizeof(pfd); pfd.nVersion=1;
  pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
  pfd.iPixelType=PFD_TYPE_RGBA; pfd.cColorBits=16; pfd.cDepthBits=16;
  pf=ChoosePixelFormat(dc,&pfd); SetPixelFormat(dc,pf,&pfd);
  rc=wglCreateContext(dc); wglMakeCurrent(dc,rc);
  fprintf(lg,"RENDERER: %s\n",(const char*)glGetString(GL_RENDERER)); fflush(lg);
  for(y=0;y<256;y++)for(x=0;x<256;x++){
    if(y<FR_H){const unsigned char*p=&fontrows[(y*256+x)*4];
      fa[y][x][0]=p[0];fa[y][x][1]=p[1];fa[y][x][2]=p[2];fa[y][x][3]=p[3];}
    else{fa[y][x][0]=fa[y][x][1]=fa[y][x][2]=fa[y][x][3]=0;}
    chk[y][x][0]=chk[y][x][1]=chk[y][x][2]=255; chk[y][x][3]=((x&2)?255:0);
  }
  glGenTextures(1,&ft); glBindTexture(GL_TEXTURE_2D,ft);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,fa);
  glViewport(0,0,W,H); glMatrixMode(GL_PROJECTION); glLoadIdentity();
  glOrtho(0,W,H,0,0,1); glMatrixMode(GL_MODELVIEW); glLoadIdentity();
  glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE); glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  /* A: exact menu quads (repro) */
  glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0,0,0,1); glClear(GL_COLOR_BUFFER_BIT);
  glColor4ub(220,40,40,255);
  for(qi=0;qi<11;qi++){const float*q=Q[qi];
    quad(q[0],q[1],q[2],q[3],q[4]/256.f,q[5]/256.f,q[6]/256.f,q[7]/256.f);}
  glDisable(GL_BLEND); dump("C:\\gfix_A.raw");
  /* B: alpha-column probe, NEAREST, 1:1 high-s */
  glGenTextures(1,&ck); glBindTexture(GL_TEXTURE_2D,ck);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,chk);
  glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glClear(GL_COLOR_BUFFER_BIT); glColor4ub(230,40,40,255);
  quad(120,170,144,197,230.f/256,0,254.f/256,1);
  glDisable(GL_BLEND); dump("C:\\gfix_B.raw");
  /* C: real glyphs (like A) but NEAREST filter. If clean, the bug is LINEAR. */
  glBindTexture(GL_TEXTURE_2D,ft);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glClear(GL_COLOR_BUFFER_BIT); glColor4ub(220,40,40,255);
  for(qi=0;qi<11;qi++){const float*q=Q[qi];
    quad(q[0],q[1],q[2],q[3],q[4]/256.f,q[5]/256.f,q[6]/256.f,q[7]/256.f);}
  glDisable(GL_BLEND); dump("C:\\gfix_C.raw");
  /* D: alpha-checker (clean at x=120 in case B) but rendered at the GLYPH screen
   * region x=222..246. If it slices here -> screen-position dependent. */
  glBindTexture(GL_TEXTURE_2D,ck);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glEnable(GL_BLEND); glClear(GL_COLOR_BUFFER_BIT); glColor4ub(230,40,40,255);
  quad(222,170,246,197,230.f/256,0,254.f/256,1);
  glDisable(GL_BLEND); dump("C:\\gfix_D.raw");
  /* E: alpha-checker as 11 SEPARATE abutting quads (like the glyphs), x=222.. */
  glEnable(GL_BLEND); glClear(GL_COLOR_BUFFER_BIT); glColor4ub(230,40,40,255);
  {int k; float px=222.f;
   for(k=0;k<11;k++){ float s0=(230-k*4)/256.f, s1=(254-k*4)/256.f;
     quad(px,170,px+20,197,s0,0,s1,1); px+=20; } }
  glDisable(GL_BLEND); dump("C:\\gfix_E.raw");
  /* F: ONE real glyph (the M, Q[0]) ALONE. If clean -> multi-quad interaction;
   * if sliced -> the glyph content/texcoords. LINEAR (like the font). */
  glBindTexture(GL_TEXTURE_2D,ft);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glEnable(GL_BLEND); glClear(GL_COLOR_BUFFER_BIT); glColor4ub(220,40,40,255);
  { const float*q=Q[0]; quad(q[0],q[1],q[2],q[3],q[4]/256.f,q[5]/256.f,q[6]/256.f,q[7]/256.f); }
  glDisable(GL_BLEND); dump("C:\\gfix_F.raw");
  /* G: TEXEL RULER. 256-wide tex, white 1-texel lines at texels 32,96,160,224
   * on black, opaque, NEAREST, drawn 1:1 no-blend at screen x=64..320 (s=0..256)
   * and a SECOND 128-wide-region draw at y=240 (screen x=64..192, s=0..128) to
   * test if the S bias scales with texture width. Read back: white screen-x of
   * each line => exact S offset (screen_x - 64 - texel). Isolates the constant
   * horizontal texture offset seen on every glyph. */
  { static unsigned char rul[256][256][4]; int rx,ry;
    for(ry=0;ry<256;ry++)for(rx=0;rx<256;rx++){
      int on=(rx==32||rx==96||rx==160||rx==224);
      unsigned char r=on?255:0,g=on?255:0,b=on?255:0;
      /* edge decode: texels 0-3 GREEN, 252-255 MAGENTA. With the +4 shift:
       * left screen cols show GREEN (clamped S => S-DDA bug) or MAGENTA
       * (row-end wrap => texture base-address/memory offset bug). */
      if(rx<4){r=0;g=255;b=0;} else if(rx>=252){r=255;g=0;b=255;}
      rul[ry][rx][0]=r;rul[ry][rx][1]=g;rul[ry][rx][2]=b; rul[ry][rx][3]=255; }
    glBindTexture(GL_TEXTURE_2D,ck);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,rul);
    glDisable(GL_BLEND); glColor4ub(255,255,255,255);
    glClear(GL_COLOR_BUFFER_BIT);
    /* full 256 tex, 1:1 at screen x=64..320, y=100..140 */
    quad(64,100,320,140, 0.f,0.f, 256.f/256, 1.f);
    /* left 128 texels of same tex, 1:1 at screen x=64..192, y=240..280 */
    quad(64,240,192,280, 0.f,0.f, 128.f/256, 1.f);
    /* 2x MAGNIFIED: full 256 tex across screen x=64..576 (512px) y=340..380.
     * lines ideal at screen 64+2*texel = 128,256,384,512. If shift=+8 the bias
     * is texel-space (sow); if +4 it is screen-space (geometry). */
    quad(64,340,576,380, 0.f,0.f, 256.f/256, 1.f);
  }
  dump("C:\\gfix_G.raw");
  /* H: GoldSrc-style COLOR CHANNEL probe. HL passes legacy numeric
   * internalformat 3 (world tex) / 4 (alpha tex) with GL_RGBA source data.
   * Upload solid-color 8x8 textures - red, green, blue, tan - via
   * internalformat 3 (row y=60) and 4 (row y=120), draw 40px quads,
   * modulate white, no blend. Readback tells the exact channel transform
   * (CS de_dust renders tan world as GREEN => suspect R lost/remapped in
   * the ifmt-3 RGBA->565 conversion). */
  { static unsigned char sc[8][8][4]; GLuint ht; int ci,fi,sx,sy;
    static const unsigned char cols[4][3]={{255,0,0},{0,255,0},{0,0,255},{210,180,140}};
    glGenTextures(1,&ht); glBindTexture(GL_TEXTURE_2D,ht);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glDisable(GL_BLEND); glColor4ub(255,255,255,255);
    glClear(GL_COLOR_BUFFER_BIT);
    for(fi=0;fi<2;fi++){
      for(ci=0;ci<4;ci++){
        for(sy=0;sy<8;sy++)for(sx=0;sx<8;sx++){
          sc[sy][sx][0]=cols[ci][0];sc[sy][sx][1]=cols[ci][1];
          sc[sy][sx][2]=cols[ci][2];sc[sy][sx][3]=255;}
        glTexImage2D(GL_TEXTURE_2D,0,fi?4:3,8,8,0,GL_RGBA,GL_UNSIGNED_BYTE,sc);
        quad(40.f+ci*60,60.f+fi*60,80.f+ci*60,100.f+fi*60,0,0,1,1);
      }
    }
  }
  dump("C:\\gfix_H.raw");
  /* I: PALETTED TEXTURE probe (GL_EXT_paletted_texture, the path GoldSrc/HL
   * uses for world textures when advertised - GDI Generic lacks it, skip).
   * glColorTableEXT 256xRGBA palette, then COLOR_INDEX8 upload; indices
   * 1=red 2=green 3=blue 4=tan. If these swatches come out wrong while
   * case H (plain RGBA upload) was correct, the CS green-world bug is the
   * palette path. */
  { typedef void (APIENTRY *PFNCT)(GLenum,GLenum,GLsizei,GLenum,GLenum,const GLvoid*);
    PFNCT pCT=(PFNCT)wglGetProcAddress("glColorTableEXT");
    fprintf(lg,"glColorTableEXT=%p\n",(void*)pCT); fflush(lg);
    if(pCT){
      static unsigned char pal[256][4]; static unsigned char idx[8][8];
      static const unsigned char cols[4][3]={{255,0,0},{0,255,0},{0,0,255},{210,180,140}};
      GLuint pt; int ci,sx,sy;
      for(ci=0;ci<256;ci++){pal[ci][0]=pal[ci][1]=pal[ci][2]=0;pal[ci][3]=255;}
      for(ci=0;ci<4;ci++){pal[ci+1][0]=cols[ci][0];pal[ci+1][1]=cols[ci][1];pal[ci+1][2]=cols[ci][2];pal[ci+1][3]=255;}
      glGenTextures(1,&pt); glBindTexture(GL_TEXTURE_2D,pt);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
      glDisable(GL_BLEND); glColor4ub(255,255,255,255);
      glClear(GL_COLOR_BUFFER_BIT);
      for(ci=0;ci<4;ci++){
        for(sy=0;sy<8;sy++)for(sx=0;sx<8;sx++) idx[sy][sx]=(unsigned char)(ci+1);
        pCT(GL_TEXTURE_2D,GL_RGBA8,256,GL_RGBA,GL_UNSIGNED_BYTE,pal);
        glTexImage2D(GL_TEXTURE_2D,0,0x80E5/*COLOR_INDEX8_EXT*/,8,8,0,
                     0x1900/*GL_COLOR_INDEX*/,GL_UNSIGNED_BYTE,idx);
        quad(40.f+ci*60,200.f,80.f+ci*60,240.f,0,0,1,1);
      }
      dump("C:\\gfix_I.raw");
    }
  }
  /* K: GoldSrc-exact world draw replication (CS green-world hunt).
   * HL uploads ifmt=GL_RGBA4(0x8057) fmt=GL_RGBA ubyte and draws the world
   * as GL_POLYGON with ARB multitexture MODULATE(world) x MODULATE(lightmap).
   * K1 y=300: single-tex GL_POLYGON, tan tex          -> expect tan
   * K2 y=360: multitex GL_POLYGON, tan x 50%-gray LM  -> expect half-tan
   * K3 y=420: multitex GL_TRIANGLES (path compare)    -> expect half-tan */
  { typedef void (APIENTRY *PFNAT)(unsigned int);
    typedef void (APIENTRY *PFNMT)(unsigned int,float,float);
    PFNAT pAT=(PFNAT)wglGetProcAddress("glActiveTextureARB");
    PFNAT pCAT=(PFNAT)wglGetProcAddress("glClientActiveTextureARB");
    PFNMT pMT=(PFNMT)wglGetProcAddress("glMultiTexCoord2fARB");
    static unsigned char tanw[16][16][4], gray[16][16][4];
    GLuint tw,tl; int kx,ky;
    fprintf(lg,"ARB mt: AT=%p MT=%p\n",(void*)pAT,(void*)pMT); fflush(lg);
    for(ky=0;ky<16;ky++)for(kx=0;kx<16;kx++){
      tanw[ky][kx][0]=210;tanw[ky][kx][1]=180;tanw[ky][kx][2]=140;tanw[ky][kx][3]=255;
      gray[ky][kx][0]=gray[ky][kx][1]=gray[ky][kx][2]=128;gray[ky][kx][3]=255;}
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND); glColor4ub(255,255,255,255);
    glGenTextures(1,&tw); glBindTexture(GL_TEXTURE_2D,tw);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,0x8057/*GL_RGBA4*/,16,16,0,GL_RGBA,GL_UNSIGNED_BYTE,tanw);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    /* K1: single-tex GL_POLYGON */
    glBegin(GL_POLYGON);
    glTexCoord2f(0,0); glVertex2f(60,300);
    glTexCoord2f(1,0); glVertex2f(140,300);
    glTexCoord2f(1,1); glVertex2f(140,340);
    glTexCoord2f(0,1); glVertex2f(60,340);
    glEnd();
    if(pAT&&pMT){
      pAT(0x84C1/*TEXTURE1*/); glEnable(GL_TEXTURE_2D);
      glGenTextures(1,&tl); glBindTexture(GL_TEXTURE_2D,tl);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D,0,0x8057,16,16,0,GL_RGBA,GL_UNSIGNED_BYTE,gray);
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
      pAT(0x84C0/*TEXTURE0*/);
      /* K2: multitex GL_POLYGON */
      glBegin(GL_POLYGON);
      pMT(0x84C0,0,0); pMT(0x84C1,0,0); glVertex2f(60,360);
      pMT(0x84C0,1,0); pMT(0x84C1,1,0); glVertex2f(140,360);
      pMT(0x84C0,1,1); pMT(0x84C1,1,1); glVertex2f(140,400);
      pMT(0x84C0,0,1); pMT(0x84C1,0,1); glVertex2f(60,400);
      glEnd();
      /* K3: multitex GL_TRIANGLES */
      glBegin(GL_TRIANGLES);
      pMT(0x84C0,0,0); pMT(0x84C1,0,0); glVertex2f(60,420);
      pMT(0x84C0,1,0); pMT(0x84C1,1,0); glVertex2f(140,420);
      pMT(0x84C0,1,1); pMT(0x84C1,1,1); glVertex2f(140,460);
      pMT(0x84C0,0,0); pMT(0x84C1,0,0); glVertex2f(60,420);
      pMT(0x84C0,1,1); pMT(0x84C1,1,1); glVertex2f(140,460);
      pMT(0x84C0,0,1); pMT(0x84C1,0,1); glVertex2f(60,460);
      glEnd();
      pAT(0x84C1); glDisable(GL_TEXTURE_2D); pAT(0x84C0);
    }
    dump("C:\\gfix_K.raw");
  }
  /* L: CS-exact world recipe. From live capture: world tex = ifmt 3
   * (565) 128x128 WITH FULL MIP CHAIN (HL uploads lods itself),
   * LINEAR_MIPMAP_NEAREST; lightmap = ifmt GL_RGB5_A1(0x8056) 128x128,
   * gray 0x46 with ALPHA=0, LINEAR; multitex MODULATE x MODULATE,
   * GL_POLYGON. Expect tan*0x46/255 ~ (58,50,39). Green here = repro. */
  { typedef void (APIENTRY *PFNAT)(unsigned int);
    typedef void (APIENTRY *PFNMT)(unsigned int,float,float);
    PFNAT pAT=(PFNAT)wglGetProcAddress("glActiveTextureARB");
    PFNMT pMT=(PFNMT)wglGetProcAddress("glMultiTexCoord2fARB");
    static unsigned char wtx[128][128][4], ltx[128][128][4];
    GLuint tw,tl; int kx,ky,lv,sz;
    for(ky=0;ky<128;ky++)for(kx=0;kx<128;kx++){
      wtx[ky][kx][0]=210;wtx[ky][kx][1]=180;wtx[ky][kx][2]=140;wtx[ky][kx][3]=255;
      ltx[ky][kx][0]=ltx[ky][kx][1]=ltx[ky][kx][2]=0x46;ltx[ky][kx][3]=0;}
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND); glColor4ub(255,255,255,255);
    glGenTextures(1,&tw); glBindTexture(GL_TEXTURE_2D,tw);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,0x2701/*LINEAR_MIPMAP_NEAREST*/);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    for(lv=0,sz=128;sz>=1;lv++,sz>>=1)
      glTexImage2D(GL_TEXTURE_2D,lv,3,sz,sz,0,GL_RGBA,GL_UNSIGNED_BYTE,wtx);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    if(pAT&&pMT){
      pAT(0x84C1); glEnable(GL_TEXTURE_2D);
      glGenTextures(1,&tl); glBindTexture(GL_TEXTURE_2D,tl);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D,0,0x8056/*GL_RGB5_A1*/,128,128,0,GL_RGBA,GL_UNSIGNED_BYTE,ltx);
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
      pAT(0x84C0);
      glBegin(GL_POLYGON);
      pMT(0x84C0,0,0); pMT(0x84C1,0,0); glVertex2f(200,300);
      pMT(0x84C0,1,0); pMT(0x84C1,1,0); glVertex2f(328,300);
      pMT(0x84C0,1,1); pMT(0x84C1,1,1); glVertex2f(328,428);
      pMT(0x84C0,0,1); pMT(0x84C1,0,1); glVertex2f(200,428);
      glEnd();
      pAT(0x84C1); glDisable(GL_TEXTURE_2D); pAT(0x84C0);
    }
    /* also single-tex control of the mipped 565 world texture alone */
    glBegin(GL_POLYGON);
    glTexCoord2f(0,0); glVertex2f(40,300);
    glTexCoord2f(1,0); glVertex2f(168,300);
    glTexCoord2f(1,1); glVertex2f(168,428);
    glTexCoord2f(0,1); glVertex2f(40,428);
    glEnd();
    dump("C:\\gfix_L.raw");
  }
  /* M: two-pass lightmap BLEND probe (GoldSrc gl_texsort path).
   * M1 x=40 : tan base, then gray-0x46 tex with Blend(GL_ZERO,GL_SRC_COLOR)
   *           -> dst*src, expect ~(58,50,39)
   * M2 x=180: tan base, then gray tex with Blend(GL_DST_COLOR,GL_ZERO)
   *           -> src*dst, expect same
   * M3 x=320: tan base, then gray tex with Blend(GL_DST_COLOR,GL_SRC_COLOR)
   *           -> 2*src*dst (Q3 overbright), expect ~(115,99,77)
   * Wrong hues here = broken color-factor blend on Napalm. */
  { static unsigned char tanb[16][16][4], grayb[16][16][4];
    GLuint tb,gb; int kx,ky,mi;
    static const float MX[3]={40,180,320};
    static const unsigned int BF[3][2]={{0/*ZERO*/,0x0300/*SRC_COLOR*/},
                                        {0x0306/*DST_COLOR*/,0/*ZERO*/},
                                        {0x0306,0x0300}};
    for(ky=0;ky<16;ky++)for(kx=0;kx<16;kx++){
      tanb[ky][kx][0]=210;tanb[ky][kx][1]=180;tanb[ky][kx][2]=140;tanb[ky][kx][3]=255;
      grayb[ky][kx][0]=grayb[ky][kx][1]=grayb[ky][kx][2]=0x46;grayb[ky][kx][3]=255;}
    glClear(GL_COLOR_BUFFER_BIT);
    glColor4ub(255,255,255,255);
    glGenTextures(1,&tb); glBindTexture(GL_TEXTURE_2D,tb);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,3,16,16,0,GL_RGBA,GL_UNSIGNED_BYTE,tanb);
    glGenTextures(1,&gb); glBindTexture(GL_TEXTURE_2D,gb);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,0x8056,16,16,0,GL_RGBA,GL_UNSIGNED_BYTE,grayb);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    for(mi=0;mi<3;mi++){
      glDisable(GL_BLEND);
      glBindTexture(GL_TEXTURE_2D,tb);
      glBegin(GL_POLYGON);
      glTexCoord2f(0,0); glVertex2f(MX[mi],120);
      glTexCoord2f(1,0); glVertex2f(MX[mi]+96,120);
      glTexCoord2f(1,1); glVertex2f(MX[mi]+96,216);
      glTexCoord2f(0,1); glVertex2f(MX[mi],216);
      glEnd();
      glEnable(GL_BLEND); glBlendFunc(BF[mi][0]?BF[mi][0]:GL_ZERO, BF[mi][1]?BF[mi][1]:GL_ZERO);
      glBindTexture(GL_TEXTURE_2D,gb);
      glBegin(GL_POLYGON);
      glTexCoord2f(0,0); glVertex2f(MX[mi],120);
      glTexCoord2f(1,0); glVertex2f(MX[mi]+96,120);
      glTexCoord2f(1,1); glVertex2f(MX[mi]+96,216);
      glTexCoord2f(0,1); glVertex2f(MX[mi],216);
      glEnd();
      glDisable(GL_BLEND);
    }
    dump("C:\\gfix_M.raw");
  }
  /* N: LOD/aspect color probe (CS green-LOD0 hunt). Textures ifmt=3
   * (565) with a DISTINCT color per mip level: L0=tan L1=red L2=blue
   * L3+=magenta. N1: 128x128 square. N2: 128x32 non-square (HL wall
   * shape). Drawn 1:1 (shows L0) and quarter-size (shows ~L2).
   * LINEAR_MIPMAP_NEAREST like HL. Expected 1:1 = TAN; wrong hue or
   * pure-green here reproduces the live CS corruption. */
  { static unsigned char nbuf[128][128][4];
    static const unsigned char lc[4][3]={{210,180,140},{255,0,0},{0,0,255},{255,0,255}};
    GLuint tn; int kx,ky,lv,w2,h2,ci;
    int shapes[2][2]={{128,128},{128,32}};
    float bx;
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND); glColor4ub(255,255,255,255);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    for(ci=0;ci<2;ci++){
      glGenTextures(1,&tn); glBindTexture(GL_TEXTURE_2D,tn);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,0x2701);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      w2=shapes[ci][0]; h2=shapes[ci][1];
      for(lv=0; w2||h2; lv++){
        int lw=w2?w2:1, lh=h2?h2:1;
        const unsigned char *c=lc[lv<3?lv:3];
        for(ky=0;ky<lh;ky++)for(kx=0;kx<lw;kx++){
          nbuf[ky][kx][0]=c[0];nbuf[ky][kx][1]=c[1];nbuf[ky][kx][2]=c[2];nbuf[ky][kx][3]=255;}
        glTexImage2D(GL_TEXTURE_2D,lv,3,lw,lh,0,GL_RGBA,GL_UNSIGNED_BYTE,nbuf);
        w2>>=1; h2>>=1;
      }
      bx = 40.f + ci*300;
      /* 1:1 -> LOD0 (must be TAN) */
      quad(bx,60, bx+shapes[ci][0], 60.f+shapes[ci][1], 0,0,1,1);
      /* quarter-size -> ~LOD2 (must be BLUE) */
      quad(bx,240, bx+shapes[ci][0]/4.f, 240.f+shapes[ci][1]/4.f, 0,0,1,1);
    }
    dump("C:\\gfix_N.raw");
  }
  /* O: STALE-FORMAT REBIND probe. HL rebinds textures of different
   * grformats constantly; if the bind path only re-sources the TMU on
   * address change, the format goes stale. texA=565 tan, texB=4444
   * white/50%. Draw A (must be tan), draw B, REBIND A draw again (must
   * still be tan; green/wrong = stale-format repro). Multitex variant
   * with a 4444 lightmap on TMU1 mirrors the live CS state. */
  { typedef void (APIENTRY *PFNAT)(unsigned int);
    typedef void (APIENTRY *PFNMT)(unsigned int,float,float);
    PFNAT pAT=(PFNAT)wglGetProcAddress("glActiveTextureARB");
    PFNMT pMT=(PFNMT)wglGetProcAddress("glMultiTexCoord2fARB");
    static unsigned char ta[64][64][4], tb[64][64][4], tl2[128][128][4];
    GLuint hA,hB,hL; int kx,ky,lv,sz;
    for(ky=0;ky<64;ky++)for(kx=0;kx<64;kx++){
      ta[ky][kx][0]=210;ta[ky][kx][1]=180;ta[ky][kx][2]=140;ta[ky][kx][3]=255;
      tb[ky][kx][0]=tb[ky][kx][1]=tb[ky][kx][2]=200;tb[ky][kx][3]=128;}
    for(ky=0;ky<128;ky++)for(kx=0;kx<128;kx++){
      tl2[ky][kx][0]=tl2[ky][kx][1]=tl2[ky][kx][2]=0x80;tl2[ky][kx][3]=0;}
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND); glColor4ub(255,255,255,255);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glGenTextures(1,&hA); glBindTexture(GL_TEXTURE_2D,hA);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,0x2701);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    for(lv=0,sz=64;sz>=1;lv++,sz>>=1)
      glTexImage2D(GL_TEXTURE_2D,lv,3,sz,sz,0,GL_RGBA,GL_UNSIGNED_BYTE,ta);
    glGenTextures(1,&hB); glBindTexture(GL_TEXTURE_2D,hB);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,0x8057,64,64,0,GL_RGBA,GL_UNSIGNED_BYTE,tb);
    /* enable TMU1 lightmap (4444, alpha 0) like live CS */
    if(pAT&&pMT){
      pAT(0x84C1); glEnable(GL_TEXTURE_2D);
      glGenTextures(1,&hL); glBindTexture(GL_TEXTURE_2D,hL);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D,0,0x8056,128,128,0,GL_RGBA,GL_UNSIGNED_BYTE,tl2);
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
      pAT(0x84C0);
    }
    #define OQUAD(X) do{ glBegin(GL_POLYGON); \
      pMT(0x84C0,0,0); pMT(0x84C1,0,0); glVertex2f((float)(X),60); \
      pMT(0x84C0,1,0); pMT(0x84C1,1,0); glVertex2f((float)(X)+64,60); \
      pMT(0x84C0,1,1); pMT(0x84C1,1,1); glVertex2f((float)(X)+64,124); \
      pMT(0x84C0,0,1); pMT(0x84C1,0,1); glVertex2f((float)(X),124); glEnd(); }while(0)
    if(pAT&&pMT){
      glBindTexture(GL_TEXTURE_2D,hA); OQUAD(40);    /* A first: tan*0.5 */
      glBindTexture(GL_TEXTURE_2D,hB); OQUAD(140);   /* B: gray*0.5 */
      glBindTexture(GL_TEXTURE_2D,hA); OQUAD(240);   /* A REBOUND: must equal draw 1 */
      glBindTexture(GL_TEXTURE_2D,hB); OQUAD(340);
      glBindTexture(GL_TEXTURE_2D,hA); OQUAD(440);   /* third A */
    }
    dump("C:\\gfix_O.raw");
  }
  /* P: CROSS-ROUTED multitex coords probe (the CS green-world killer).
   * unit0 = 64x64 solid TAN (565). unit1 = 128x128 lightmap: GREEN
   * everywhere except a WHITE patch at texel [32..40]^2 (565 via ifmt 3).
   * Quad drawn with unit0 coords 0..1 but unit1 coords pinned INSIDE the
   * white patch (0.26..0.30). Correct = tan x white = TAN.
   * If unit coords/textures are cross-routed between TMUs, the lightmap
   * gets sampled over the full quad (mostly GREEN) -> GREEN = live CS bug. */
  { typedef void (APIENTRY *PFNAT)(unsigned int);
    typedef void (APIENTRY *PFNMT)(unsigned int,float,float);
    PFNAT pAT=(PFNAT)wglGetProcAddress("glActiveTextureARB");
    PFNMT pMT=(PFNMT)wglGetProcAddress("glMultiTexCoord2fARB");
    static unsigned char w0[64][64][4], l1[128][128][4];
    GLuint h0,h1; int kx,ky;
    for(ky=0;ky<64;ky++)for(kx=0;kx<64;kx++){
      w0[ky][kx][0]=210;w0[ky][kx][1]=180;w0[ky][kx][2]=140;w0[ky][kx][3]=255;}
    for(ky=0;ky<128;ky++)for(kx=0;kx<128;kx++){
      int inpatch=(kx>=32&&kx<40&&ky>=32&&ky<40);
      l1[ky][kx][0]=inpatch?255:0; l1[ky][kx][1]=255; l1[ky][kx][2]=inpatch?255:0;
      l1[ky][kx][3]=255;}
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND); glColor4ub(255,255,255,255);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glGenTextures(1,&h0); glBindTexture(GL_TEXTURE_2D,h0);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,3,64,64,0,GL_RGBA,GL_UNSIGNED_BYTE,w0);
    if(pAT&&pMT){
      pAT(0x84C1); glEnable(GL_TEXTURE_2D);
      glGenTextures(1,&h1); glBindTexture(GL_TEXTURE_2D,h1);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D,0,3,128,128,0,GL_RGBA,GL_UNSIGNED_BYTE,l1);
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
      pAT(0x84C0);
      glBegin(GL_POLYGON);
      pMT(0x84C0,0,0); pMT(0x84C1,0.26f,0.26f); glVertex2f(200,150);
      pMT(0x84C0,1,0); pMT(0x84C1,0.30f,0.26f); glVertex2f(360,150);
      pMT(0x84C0,1,1); pMT(0x84C1,0.30f,0.30f); glVertex2f(360,310);
      pMT(0x84C0,0,1); pMT(0x84C1,0.26f,0.30f); glVertex2f(200,310);
      glEnd();
      pAT(0x84C1); glDisable(GL_TEXTURE_2D); pAT(0x84C0);
    }
    dump("C:\\gfix_P.raw");
  }
  /* Q: EXACT live CS world formats - world 565 x lightmap 565 (both
   * gr=0xa per the begin-state capture), MODULATE x MODULATE, 128x128,
   * GL_POLYGON. Every prior multitex probe used a 4444 lightmap; the live
   * lightmap is 565. World=tan, lightmap=gray 0x46. Expect ~(58,50,39).
   * GREEN here = the live CS bug finally reproduced. */
  { typedef void (APIENTRY *PFNAT)(unsigned int);
    typedef void (APIENTRY *PFNMT)(unsigned int,float,float);
    PFNAT pAT=(PFNAT)wglGetProcAddress("glActiveTextureARB");
    PFNMT pMT=(PFNMT)wglGetProcAddress("glMultiTexCoord2fARB");
    static unsigned char wq[128][128][4], lq[128][128][4];
    GLuint hw,hl; int kx,ky;
    for(ky=0;ky<128;ky++)for(kx=0;kx<128;kx++){
      wq[ky][kx][0]=210;wq[ky][kx][1]=180;wq[ky][kx][2]=140;wq[ky][kx][3]=255;
      lq[ky][kx][0]=lq[ky][kx][1]=lq[ky][kx][2]=0x46;lq[ky][kx][3]=255;}
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND); glColor4ub(255,255,255,255);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    /* world on unit0: ifmt 3 => 565 */
    glGenTextures(1,&hw); glBindTexture(GL_TEXTURE_2D,hw);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,3,128,128,0,GL_RGBA,GL_UNSIGNED_BYTE,wq);
    if(pAT&&pMT){
      pAT(0x84C1); glEnable(GL_TEXTURE_2D);
      glGenTextures(1,&hl); glBindTexture(GL_TEXTURE_2D,hl);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D,0,3,128,128,0,GL_RGBA,GL_UNSIGNED_BYTE,lq); /* 565! */
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
      pAT(0x84C0);
      glBegin(GL_POLYGON);
      pMT(0x84C0,0,0); pMT(0x84C1,0,0); glVertex2f(200,150);
      pMT(0x84C0,1,0); pMT(0x84C1,1,0); glVertex2f(360,150);
      pMT(0x84C0,1,1); pMT(0x84C1,1,1); glVertex2f(360,310);
      pMT(0x84C0,0,1); pMT(0x84C1,0,1); glVertex2f(200,310);
      glEnd();
      pAT(0x84C1); glDisable(GL_TEXTURE_2D); pAT(0x84C0);
    }
    dump("C:\\gfix_Q.raw");
  }
  /* R: MEMORY-PRESSURE eviction repro (the missing axis). Upload 300
   * distinct 128x128 565 MIPMAPPED textures (~12MB > TMU RAM) to force
   * eviction, re-bind + draw texture #0 (tan), read its color. Live CS
   * has hundreds of textures -> eviction + re-download; my earlier probes
   * had 1-2 (no pressure). GREEN here = eviction/re-download bug repro. */
  { static unsigned char rt[128][128][4]; GLuint rid[300]; int ri,lv,sz,rk;
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND); glColor4ub(255,255,255,255);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glGenTextures(300,rid);
    for(ri=0;ri<300;ri++){
      /* tex 0 = pure tan; others = a per-index color so they occupy RAM */
      unsigned char cr=(ri==0)?210:(unsigned char)(ri*7);
      unsigned char cg=(ri==0)?180:(unsigned char)(ri*3);
      unsigned char cb=(ri==0)?140:(unsigned char)(ri*5);
      for(rk=0;rk<128*128;rk++){ rt[0][rk][0]=cr;rt[0][rk][1]=cg;rt[0][rk][2]=cb;rt[0][rk][3]=255; }
      glBindTexture(GL_TEXTURE_2D,rid[ri]);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,0x2701/*LINEAR_MIPMAP_NEAREST*/);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      for(lv=0,sz=128;sz>=1;lv++,sz>>=1)
        glTexImage2D(GL_TEXTURE_2D,lv,3,sz,sz,0,GL_RGBA,GL_UNSIGNED_BYTE,rt);
    }
    /* now re-bind + draw tex 0 (long-evicted) 1:1 -> forces re-download */
    glBindTexture(GL_TEXTURE_2D,rid[0]);
    quad(100,100,228,228,0,0,1,1);
    dump("C:\\gfix_R.raw");
    glDeleteTextures(300,rid);
  }
  /* case S (non-mip->mip transition crash) removed from harness; tracked separately */
  /* T: DUAL-TMU + PRESSURE repro (the untested combination). CS uses
   * simultaneous multitexture (world on TMU1, lightmap on TMU0) with
   * HUNDREDS of textures; Q3 uses single-texture only. My multitex probes
   * had no pressure; my pressure probe (R) was single-texture. Combine:
   * upload 120 world (565) + 120 lightmap (565) textures, do a multitex
   * draw with EACH pair (fills both TMU memories), then re-draw pair 0 and
   * check. tan pair 0 = OK; green = dual-TMU-under-pressure bug found. */
  { typedef void (APIENTRY *PFNAT)(unsigned int);
    typedef void (APIENTRY *PFNMT)(unsigned int,float,float);
    PFNAT pAT=(PFNAT)wglGetProcAddress("glActiveTextureARB");
    PFNMT pMT=(PFNMT)wglGetProcAddress("glMultiTexCoord2fARB");
    static unsigned char tw[128][128][4];
    GLuint wid[120], lid[120]; int ti,lv,sz,rk;
    if(pAT&&pMT){
      glClear(GL_COLOR_BUFFER_BIT);
      glDisable(GL_BLEND); glColor4ub(255,255,255,255);
      glGenTextures(120,wid); glGenTextures(120,lid);
      /* create all world (tan-ish, idx-varied) + lightmap (gray) textures */
      for(ti=0;ti<120;ti++){
        unsigned char c0=(ti==0)?210:(unsigned char)(80+ti);
        unsigned char c1=(ti==0)?180:(unsigned char)(60+ti);
        unsigned char c2=(ti==0)?140:(unsigned char)(40+ti);
        for(rk=0;rk<128*128;rk++){tw[0][rk][0]=c0;tw[0][rk][1]=c1;tw[0][rk][2]=c2;tw[0][rk][3]=255;}
        glBindTexture(GL_TEXTURE_2D,wid[ti]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,0x2701);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        for(lv=0,sz=128;sz>=1;lv++,sz>>=1)
          glTexImage2D(GL_TEXTURE_2D,lv,3,sz,sz,0,GL_RGBA,GL_UNSIGNED_BYTE,tw);
        for(rk=0;rk<128*128;rk++){tw[0][rk][0]=tw[0][rk][1]=tw[0][rk][2]=0x46;tw[0][rk][3]=255;}
        glBindTexture(GL_TEXTURE_2D,lid[ti]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        /* lightmap ifmt 0x8056 = GL_RGBA4 -> 4444 (matches live CS; a 565
        ** lightmap would hide any overwrite of the 565 world's LOD0). */
        glTexImage2D(GL_TEXTURE_2D,0,0x8056,128,128,0,GL_RGBA,GL_UNSIGNED_BYTE,tw);
      }
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
      pAT(0x84C1); glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
      pAT(0x84C0);
      /* multitex draw with EACH pair -> sources both TMUs 120 times */
      for(ti=0;ti<120;ti++){
        pAT(0x84C0); glBindTexture(GL_TEXTURE_2D,wid[ti]);
        pAT(0x84C1); glBindTexture(GL_TEXTURE_2D,lid[ti]);
        pAT(0x84C0);
        glBegin(GL_POLYGON);
        pMT(0x84C0,0,0); pMT(0x84C1,0,0); glVertex2f(500,20);
        pMT(0x84C0,1,0); pMT(0x84C1,1,0); glVertex2f(560,20);
        pMT(0x84C0,1,1); pMT(0x84C1,1,1); glVertex2f(560,60);
        pMT(0x84C0,0,1); pMT(0x84C1,0,1); glVertex2f(500,60);
        glEnd();
      }
      /* now re-draw PAIR 0 (long-evicted) large and check */
      pAT(0x84C0); glBindTexture(GL_TEXTURE_2D,wid[0]);
      pAT(0x84C1); glBindTexture(GL_TEXTURE_2D,lid[0]);
      pAT(0x84C0);
      glBegin(GL_POLYGON);
      pMT(0x84C0,0,0); pMT(0x84C1,0,0); glVertex2f(200,150);
      pMT(0x84C0,1,0); pMT(0x84C1,1,0); glVertex2f(360,150);
      pMT(0x84C0,1,1); pMT(0x84C1,1,1); glVertex2f(360,310);
      pMT(0x84C0,0,1); pMT(0x84C1,0,1); glVertex2f(200,310);
      glEnd();
      pAT(0x84C1); glDisable(GL_TEXTURE_2D); pAT(0x84C0);
      dump("C:\\gfix_T.raw");
      glDeleteTextures(120,wid); glDeleteTextures(120,lid);
    } else { dump("C:\\gfix_T.raw"); }
  }
  /* J: swap-loop meter validation. 220 SwapBuffers frames; with
   * RETRO3DFX_PERFLOG=1 the ICD must emit >=2 lines to C:\icd_perf.log
   * (db=1). Validates the perf meter under a known double-buffered
   * context - if CS then still logs nothing, CS's context is
   * single-buffered (wgl swap wrapper early-returns). */
  { int fr;
    for(fr=0;fr<220;fr++){
      glClearColor((fr&1)?0.2f:0.0f,0,0,1); glClear(GL_COLOR_BUFFER_BIT);
      SwapBuffers(dc);
    }
    fprintf(lg,"swaploop done\n"); fflush(lg);
  }
  fprintf(lg,"done\n"); fclose(lg);
  wglMakeCurrent(0,0); wglDeleteContext(rc); ReleaseDC(hwnd,dc); DestroyWindow(hwnd);
  return 0;
}
