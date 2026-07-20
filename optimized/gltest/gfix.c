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
  fprintf(lg,"done\n"); fclose(lg);
  wglMakeCurrent(0,0); wglDeleteContext(rc); ReleaseDC(hwnd,dc); DestroyWindow(hwnd);
  return 0;
}
