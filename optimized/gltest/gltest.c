/* RETRO3DFX ICD probe: isolates the 2D text-garble mechanism.
 *
 * Renders a battery of known patterns through the installed OpenGL ICD
 * (fullscreen 640x480, like Q3's UI path: ortho, textured quads) and writes
 * each case's glReadPixels result to C:\gltest_NN.raw (RGB24) plus a summary
 * C:\gltest.log. Cases:
 *   00 solid quad, integer coords            (raster fill baseline)
 *   01 solid quad, fractional x (+0.4375)    (fractional-edge fill rule)
 *   02 1px-column checker tex, 1:1 integer   (exact texel sampling/alignment)
 *   03 1px-column checker tex, 1:1 frac x    (sampling under fractional pos)
 *   04 1px-ROW checker tex, 1:1 integer      (t-axis sampling)
 *   05 checker 1:1 integer + GL_BLEND        (blend path, glyph-style)
 *   06 checker scaled x0.75                  (minification stepping)
 *   07 checker scaled x1.25                  (magnification stepping)
 *   08 glyph-sim: AA-edged bar tex + blend, fractional pos (full Q3 recipe)
 * Build (MSVC6): cl /nologo /O2 gltest.c /link opengl32.lib gdi32.lib user32.lib
 */
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include "fontrows.h"


#define W 640
#define H 480
#define TW 64
#define TH 64

static FILE *lg;

static void dump_case(int idx)
{
    static unsigned char buf[W * H * 3];
    char name[64];
    FILE *f;
    glReadPixels(0, 0, W, H, GL_RGB, GL_UNSIGNED_BYTE, buf);
    sprintf(name, "C:\\gltest_%02d.raw", idx);
    f = fopen(name, "wb");
    if (f) { fwrite(buf, 1, sizeof(buf), f); fclose(f); }
    fprintf(lg, "case %02d dumped err=0x%x\n", idx, (unsigned)glGetError());
    fflush(lg);
}

static void quad(float x0, float y0, float x1, float y1,
                 float s0, float t0, float s1, float t1, int textured)
{
    glBegin(GL_QUADS);
    if (textured) glTexCoord2f(s0, t0);
    glVertex2f(x0, y0);
    if (textured) glTexCoord2f(s1, t0);
    glVertex2f(x1, y0);
    if (textured) glTexCoord2f(s1, t1);
    glVertex2f(x1, y1);
    if (textured) glTexCoord2f(s0, t1);
    glVertex2f(x0, y1);
    glEnd();
}

int main(void)
{
    PIXELFORMATDESCRIPTOR pfd;
    HWND hwnd;
    HDC dc;
    HGLRC rc;
    int pf, i;
    static unsigned char colchk[TH][TW][4], rowchk[TH][TW][4], bar[TH][TW][4];
    GLuint tex[3];

    lg = fopen("C:\\gltest.log", "w");
    if (!lg) return 1;

    hwnd = CreateWindowA("STATIC", "gltest", WS_POPUP | WS_VISIBLE,
                         0, 0, W, H, 0, 0, 0, 0);
    dc = GetDC(hwnd);
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd); pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA; pfd.cColorBits = 16; pfd.cDepthBits = 16;
    pf = ChoosePixelFormat(dc, &pfd);
    SetPixelFormat(dc, pf, &pfd);
    rc = wglCreateContext(dc);
    wglMakeCurrent(dc, rc);
    fprintf(lg, "GL_RENDERER: %s\nGL_VERSION: %s\npf=%d\n",
            (const char *)glGetString(GL_RENDERER),
            (const char *)glGetString(GL_VERSION), pf);
    fflush(lg);

    /* textures: 1px vertical columns, 1px horizontal rows, AA-edged bar */
    for (i = 0; i < TH * TW; i++) {
        int x = i % TW, y = i / TW;
        unsigned char cv = (x & 1) ? 255 : 0;      /* odd cols white */
        unsigned char rv = (y & 1) ? 255 : 0;      /* odd rows white */
        colchk[y][x][0] = colchk[y][x][1] = colchk[y][x][2] = cv; colchk[y][x][3] = 255;
        rowchk[y][x][0] = rowchk[y][x][1] = rowchk[y][x][2] = rv; rowchk[y][x][3] = 255;
        /* bar: vertical red bar 8..55 with 2px alpha ramp edges (glyph-like) */
        {
            unsigned char a = 0;
            if (x >= 10 && x <= 53) a = 255;
            else if (x == 8 || x == 55) a = 64;
            else if (x == 9 || x == 54) a = 160;
            bar[y][x][0] = 200; bar[y][x][1] = 30; bar[y][x][2] = 30; bar[y][x][3] = a;
        }
    }
    glGenTextures(3, tex);
    {
        void *ptrs[3]; int t;
        ptrs[0] = colchk; ptrs[1] = rowchk; ptrs[2] = bar;
        for (t = 0; t < 3; t++) {
            glBindTexture(GL_TEXTURE_2D, tex[t]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TW, TH, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, ptrs[t]);
        }
    }

    /* Q3-style 2D state */
    glViewport(0, 0, W, H);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0, W, H, 0, 0, 1);                    /* y-down like Q3 2D */
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    /* 00: solid quad integer */
    glClearColor(0, 0, 0.25f, 1); glClear(GL_COLOR_BUFFER_BIT);
    glColor4f(0.2f, 0.9f, 0.2f, 1);
    quad(100, 100, 400, 200, 0, 0, 0, 0, 0);
    dump_case(0);
    /* 01: solid quad fractional x */
    glClear(GL_COLOR_BUFFER_BIT);
    quad(100.4375f, 100, 400.4375f, 200, 0, 0, 0, 0, 0);
    dump_case(1);

    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    /* 02: column checker 1:1 integer */
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glClear(GL_COLOR_BUFFER_BIT);
    quad(100, 100, 100 + TW, 100 + TH, 0, 0, 1, 1, 1);
    dump_case(2);
    /* 03: column checker 1:1 fractional x */
    glClear(GL_COLOR_BUFFER_BIT);
    quad(100.4375f, 100, 100.4375f + TW, 100 + TH, 0, 0, 1, 1, 1);
    dump_case(3);
    /* 04: row checker 1:1 integer */
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glClear(GL_COLOR_BUFFER_BIT);
    quad(100, 100, 100 + TW, 100 + TH, 0, 0, 1, 1, 1);
    dump_case(4);
    /* 05: column checker + blend */
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);
    quad(100, 100, 100 + TW, 100 + TH, 0, 0, 1, 1, 1);
    glDisable(GL_BLEND);
    dump_case(5);
    /* 06: column checker x0.75 */
    glClear(GL_COLOR_BUFFER_BIT);
    quad(100, 100, 100 + TW * 0.75f, 100 + TH * 0.75f, 0, 0, 1, 1, 1);
    dump_case(6);
    /* 07: column checker x1.25 */
    glClear(GL_COLOR_BUFFER_BIT);
    quad(100, 100, 100 + TW * 1.25f, 100 + TH * 1.25f, 0, 0, 1, 1, 1);
    dump_case(7);
    /* 08: glyph-sim AA bar + blend at fractional pos */
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);
    quad(100.4375f, 100.4375f, 100.4375f + TW, 100.4375f + TH, 0, 0, 1, 1, 1);
    glDisable(GL_BLEND);
    dump_case(8);

    /* ---- vertex-array cases (Q3's actual submit path: glDrawElements with
     * vertex(3f,stride16) + texcoord(2f) + color(4ub) arrays) ---- */
    {
        static float xyzw[4][4];
        static float st[4][2];
        static unsigned char rgba[4][4];
        static unsigned int idx[6] = { 0, 1, 2, 0, 2, 3 };
        int c;

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(3, GL_FLOAT, 16, xyzw);
        glTexCoordPointer(2, GL_FLOAT, 0, st);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, rgba);
        for (c = 0; c < 4; c++) { rgba[c][0]=rgba[c][1]=rgba[c][2]=rgba[c][3]=255; }

#define VAQUAD(x0,y0,x1,y1) do { \
        xyzw[0][0]=(x0); xyzw[0][1]=(y0); xyzw[0][2]=0; \
        xyzw[1][0]=(x1); xyzw[1][1]=(y0); xyzw[1][2]=0; \
        xyzw[2][0]=(x1); xyzw[2][1]=(y1); xyzw[2][2]=0; \
        xyzw[3][0]=(x0); xyzw[3][1]=(y1); xyzw[3][2]=0; \
        st[0][0]=0; st[0][1]=0; st[1][0]=1; st[1][1]=0; \
        st[2][0]=1; st[2][1]=1; st[3][0]=0; st[3][1]=1; \
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, idx); } while (0)

        /* 10: VA column checker 1:1 integer */
        glBindTexture(GL_TEXTURE_2D, tex[0]);
        glClear(GL_COLOR_BUFFER_BIT);
        VAQUAD(100, 100, 100 + TW, 100 + TH);
        dump_case(10);
        /* 11: VA column checker 1:1 fractional */
        glClear(GL_COLOR_BUFFER_BIT);
        VAQUAD(100.4375f, 100, 100.4375f + TW, 100 + TH);
        dump_case(11);
        /* 12: VA column checker x0.75 (the Q3 prop-font scale) */
        glClear(GL_COLOR_BUFFER_BIT);
        VAQUAD(100, 100, 100 + TW * 0.75f, 100 + TH * 0.75f);
        dump_case(12);
        /* 13: VA glyph-sim AA bar + blend, fractional */
        glBindTexture(GL_TEXTURE_2D, tex[2]);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClear(GL_COLOR_BUFFER_BIT);
        VAQUAD(100.4375f, 100.4375f, 100.4375f + TW, 100.4375f + TH);
        glDisable(GL_BLEND);
        dump_case(13);
        /* 14: VA row checker x0.75 (t-axis minification, the sliced-glyph axis) */
        glBindTexture(GL_TEXTURE_2D, tex[1]);
        glClear(GL_COLOR_BUFFER_BIT);
        VAQUAD(100, 100, 100 + TW * 0.75f, 100 + TH * 0.75f);
        dump_case(14);
    }

    /* ---- Q3 glyph-string reproduction: 256x256 atlas, sub-rect texcoords,
     * batched quads with fractional advances at 0.75 scale (UI_DrawProportional-
     * String exactly). Atlas: solid 3px-wide vertical strokes every 6th texel
     * column so any dropped/duplicated sample column is obvious. ---- */
    {
        static unsigned char atlas[256][256][4];
        static float xyzw[4 * 40][4];
        static float st[4 * 40][2];
        static unsigned char rgba[4 * 40][4];
        static unsigned int idx[6 * 40];
        GLuint atex;
        int x, y, g, nq;
        float pen;

        for (y = 0; y < 256; y++)
            for (x = 0; x < 256; x++) {
                unsigned char v = ((x % 6) < 3 && (y % 32) < 27) ? 255 : 0;
                atlas[y][x][0] = atlas[y][x][1] = atlas[y][x][2] = v;
                atlas[y][x][3] = v;
            }
        glGenTextures(1, &atex);
        glBindTexture(GL_TEXTURE_2D, atex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, atlas);

        /* build 20 "glyphs": sub-rects 18-27 texels wide from row band t=32..59,
         * advanced by width*0.75 (fractional pen positions), height 27*0.75 */
        nq = 0; pen = 100.0f;
        for (g = 0; g < 20; g++) {
            float gw = 18.0f + (float)(g % 7);           /* texel width  */
            float gx = (float)((g * 29) % 200);           /* texel x      */
            float gy = 32.0f;                             /* texel y band */
            float w = gw * 0.75f, h = 27.0f * 0.75f;
            float s0 = gx / 256.0f, s1 = (gx + gw) / 256.0f;
            float t0 = gy / 256.0f, t1 = (gy + 27.0f) / 256.0f;
            int b = nq * 4;
            xyzw[b+0][0]=pen;   xyzw[b+0][1]=300;   xyzw[b+0][2]=0;
            xyzw[b+1][0]=pen+w; xyzw[b+1][1]=300;   xyzw[b+1][2]=0;
            xyzw[b+2][0]=pen+w; xyzw[b+2][1]=300+h; xyzw[b+2][2]=0;
            xyzw[b+3][0]=pen;   xyzw[b+3][1]=300+h; xyzw[b+3][2]=0;
            st[b+0][0]=s0; st[b+0][1]=t0;  st[b+1][0]=s1; st[b+1][1]=t0;
            st[b+2][0]=s1; st[b+2][1]=t1;  st[b+3][0]=s0; st[b+3][1]=t1;
            for (x = 0; x < 4; x++) { rgba[b+x][0]=220; rgba[b+x][1]=40;
                                      rgba[b+x][2]=40;  rgba[b+x][3]=255; }
            idx[nq*6+0]=b; idx[nq*6+1]=b+1; idx[nq*6+2]=b+2;
            idx[nq*6+3]=b; idx[nq*6+4]=b+2; idx[nq*6+5]=b+3;
            pen += w + 1.2f;                              /* fractional advance */
            nq++;
        }
        glVertexPointer(3, GL_FLOAT, 16, xyzw);
        glTexCoordPointer(2, GL_FLOAT, 0, st);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, rgba);

        /* 15: batched glyph string, blend on (full Q3 recipe) */
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, nq * 6, GL_UNSIGNED_INT, idx);
        glDisable(GL_BLEND);
        dump_case(15);
        /* 17: DROPSHADOW ORDER TEST - Q3 menu recipe: black pass at +2,+2 then
         * color pass at base pos, depth test DISABLED, blend on. If the color
         * string comes out with black glyph-shaped holes, the driver is
         * z-rejecting (depth test not actually disabled). */
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        {
            int pass;
            for (pass = 0; pass < 2; pass++) {
                float ox = pass ? 0.0f : 2.0f, oy = pass ? 0.0f : 2.0f;
                unsigned char cr = pass ? 220 : 0, cg = pass ? 40 : 0, cb = pass ? 40 : 0;
                pen = 100.0f; nq = 0;
                for (g = 0; g < 20; g++) {
                    float gw = 18.0f + (float)(g % 7);
                    float w = gw, h = 27.0f;
                    int b = nq * 4;
                    xyzw[b+0][0]=pen+ox;   xyzw[b+0][1]=380+oy;
                    xyzw[b+1][0]=pen+w+ox; xyzw[b+1][1]=380+oy;
                    xyzw[b+2][0]=pen+w+ox; xyzw[b+2][1]=380+h+oy;
                    xyzw[b+3][0]=pen+ox;   xyzw[b+3][1]=380+h+oy;
                    for (x = 0; x < 4; x++) { rgba[b+x][0]=cr; rgba[b+x][1]=cg;
                                              rgba[b+x][2]=cb; rgba[b+x][3]=255; }
                    pen += w + 1.5f;
                    nq++;
                }
                glDrawElements(GL_TRIANGLES, nq * 6, GL_UNSIGNED_INT, idx);
            }
        }
        glDisable(GL_BLEND);
        dump_case(17);

        /* 18: SAME dropshadow recipe but as ONE glDrawElements batch (Q3's
         * actual submission: all shadow quads then all color quads in a single
         * indexed draw). Exercises the batched compile path + vertex cache. */
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        {
            int pass;
            nq = 0;
            for (pass = 0; pass < 2; pass++) {
                float ox = pass ? 0.0f : 2.0f, oy = pass ? 0.0f : 2.0f;
                unsigned char cr = pass ? 220 : 0, cg = pass ? 40 : 0, cb = pass ? 40 : 0;
                pen = 100.0f;
                for (g = 0; g < 20 && nq < 40; g++) {
                    float gw = 18.0f + (float)(g % 7);
                    float w = gw, h = 27.0f;
                    float gx = (float)((g * 29) % 200);
                    float s0 = gx / 256.0f, s1 = (gx + gw) / 256.0f;
                    float t0 = 32.0f / 256.0f, t1 = (32.0f + 27.0f) / 256.0f;
                    int b = nq * 4;
                    xyzw[b+0][0]=pen+ox;   xyzw[b+0][1]=380+oy;   xyzw[b+0][2]=0;
                    xyzw[b+1][0]=pen+w+ox; xyzw[b+1][1]=380+oy;   xyzw[b+1][2]=0;
                    xyzw[b+2][0]=pen+w+ox; xyzw[b+2][1]=380+h+oy; xyzw[b+2][2]=0;
                    xyzw[b+3][0]=pen+ox;   xyzw[b+3][1]=380+h+oy; xyzw[b+3][2]=0;
                    st[b+0][0]=s0; st[b+0][1]=t0;  st[b+1][0]=s1; st[b+1][1]=t0;
                    st[b+2][0]=s1; st[b+2][1]=t1;  st[b+3][0]=s0; st[b+3][1]=t1;
                    for (x = 0; x < 4; x++) { rgba[b+x][0]=cr; rgba[b+x][1]=cg;
                                              rgba[b+x][2]=cb; rgba[b+x][3]=255; }
                    idx[nq*6+0]=b; idx[nq*6+1]=b+1; idx[nq*6+2]=b+2;
                    idx[nq*6+3]=b; idx[nq*6+4]=b+2; idx[nq*6+5]=b+3;
                    pen += w + 1.5f;
                    nq++;
                }
            }
            glDrawElements(GL_TRIANGLES, nq * 6, GL_UNSIGNED_INT, idx);
        }
        glDisable(GL_BLEND);
        dump_case(18);

        /* 19: Q3's REAL no-CVA path: R_DrawStripElements — triangle strips
         * via glBegin(GL_TRIANGLE_STRIP)/glArrayElement with strip restarts.
         * Exact port of quake3-1.32 tr_shade.c logic over the same
         * shadow+color batch as case 18. */
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        {
            /* rebuild the same 40-quad batch as case 18 */
            int pass, numIndexes;
            nq = 0;
            for (pass = 0; pass < 2; pass++) {
                float ox = pass ? 0.0f : 2.0f, oy = pass ? 0.0f : 2.0f;
                unsigned char cr = pass ? 220 : 0, cg = pass ? 40 : 0, cb = pass ? 40 : 0;
                pen = 100.0f;
                for (g = 0; g < 20 && nq < 40; g++) {
                    float gw = 18.0f + (float)(g % 7);
                    float w = gw, h = 27.0f;
                    float gx = (float)((g * 29) % 200);
                    float s0 = gx / 256.0f, s1 = (gx + gw) / 256.0f;
                    float t0 = 32.0f / 256.0f, t1 = (32.0f + 27.0f) / 256.0f;
                    int b = nq * 4;
                    xyzw[b+0][0]=pen+ox;   xyzw[b+0][1]=430+oy;   xyzw[b+0][2]=0;
                    xyzw[b+1][0]=pen+w+ox; xyzw[b+1][1]=430+oy;   xyzw[b+1][2]=0;
                    xyzw[b+2][0]=pen+w+ox; xyzw[b+2][1]=430+h+oy; xyzw[b+2][2]=0;
                    xyzw[b+3][0]=pen+ox;   xyzw[b+3][1]=430+h+oy; xyzw[b+3][2]=0;
                    st[b+0][0]=s0; st[b+0][1]=t0;  st[b+1][0]=s1; st[b+1][1]=t0;
                    st[b+2][0]=s1; st[b+2][1]=t1;  st[b+3][0]=s0; st[b+3][1]=t1;
                    for (x = 0; x < 4; x++) { rgba[b+x][0]=cr; rgba[b+x][1]=cg;
                                              rgba[b+x][2]=cb; rgba[b+x][3]=255; }
                    idx[nq*6+0]=b+3; idx[nq*6+1]=b+0; idx[nq*6+2]=b+2;  /* Q3 quad stamp order */
                    idx[nq*6+3]=b+2; idx[nq*6+4]=b+0; idx[nq*6+5]=b+1;
                    pen += w + 1.5f;
                    nq++;
                }
            }
            numIndexes = nq * 6;
            /* ---- R_DrawStripElements( numIndexes, indexes, qglArrayElement ) ---- */
            {
                int i;
                unsigned int last[3];
                int even;
                if (numIndexes >= 3) {
                    glBegin(GL_TRIANGLE_STRIP);
                    glArrayElement(idx[0]);
                    glArrayElement(idx[1]);
                    glArrayElement(idx[2]);
                    last[0]=idx[0]; last[1]=idx[1]; last[2]=idx[2];
                    even = 0;
                    for (i = 3; i < numIndexes; i += 3) {
                        if (!even) {
                            if ((idx[i+0] == last[2]) && (idx[i+1] == last[1])) {
                                glArrayElement(idx[i+2]); even = 1;
                            } else {
                                glEnd();
                                glBegin(GL_TRIANGLE_STRIP);
                                glArrayElement(idx[i+0]);
                                glArrayElement(idx[i+1]);
                                glArrayElement(idx[i+2]);
                                even = 0;
                            }
                        } else {
                            if ((idx[i+0] == last[2]) && (idx[i+1] == last[0])) {
                                glArrayElement(idx[i+2]); even = 0;
                            } else {
                                glEnd();
                                glBegin(GL_TRIANGLE_STRIP);
                                glArrayElement(idx[i+0]);
                                glArrayElement(idx[i+1]);
                                glArrayElement(idx[i+2]);
                                even = 0;
                            }
                        }
                        last[0]=idx[i+0]; last[1]=idx[i+1]; last[2]=idx[i+2];
                    }
                    glEnd();
                }
            }
        }
        glDisable(GL_BLEND);
        dump_case(19);

        /* 20: THE FULL Q3 RECIPE with the REAL FONT: 256x256 atlas whose top
         * rows are the actual font1_prop data (soft alpha skirts), sub-rect
         * glyph coords, dropshadow-then-color, strips via glArrayElement,
         * modulate by menu red, blend. If this thins/slices, we have the
         * in-probe repro of the menu garble. */
        if (0) {

            static unsigned char atlas2[256][256][4];
            GLuint ftex;
            int yy, xx;
            for (yy = 0; yy < 256; yy++)
                for (xx = 0; xx < 256; xx++) {
                    if (yy < FR_H) {
                        const unsigned char *p = &fontrows[(yy*256+xx)*4];
                        atlas2[yy][xx][0]=p[0]; atlas2[yy][xx][1]=p[1];
                        atlas2[yy][xx][2]=p[2]; atlas2[yy][xx][3]=p[3];
                    } else {
                        atlas2[yy][xx][0]=atlas2[yy][xx][1]=atlas2[yy][xx][2]=0;
                        atlas2[yy][xx][3]=0;
                    }
                }
            glGenTextures(1, &ftex);
            glBindTexture(GL_TEXTURE_2D, ftex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, atlas2);

            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            {
                int pass, numIndexes, i;
                unsigned int last[3]; int even;
                /* glyph run: letters A..M (row 1: x = 2 + k*19-ish; use fixed
                 * 19-texel cells at x=2,21,40,... t=2..29), 1:1 scale */
                nq = 0;
                for (pass = 0; pass < 2; pass++) {
                    float ox = pass ? 0.0f : 2.0f, oy = pass ? 0.0f : 2.0f;
                    unsigned char cr = pass ? 220 : 0, cg = pass ? 40 : 0, cb = pass ? 40 : 0;
                    pen = 120.0f;
                    for (g = 0; g < 12 && nq < 40; g++) {
                        float gw = 19.0f, h = 27.0f;
                        float gx = 2.0f + g * 19.0f;
                        float s0 = gx / 256.0f, s1 = (gx + gw) / 256.0f;
                        float t0 = 2.0f / 256.0f, t1 = (2.0f + 27.0f) / 256.0f;
                        int b = nq * 4;
                        xyzw[b+0][0]=pen+ox;    xyzw[b+0][1]=200+oy;   xyzw[b+0][2]=0;
                        xyzw[b+1][0]=pen+gw+ox; xyzw[b+1][1]=200+oy;   xyzw[b+1][2]=0;
                        xyzw[b+2][0]=pen+gw+ox; xyzw[b+2][1]=200+h+oy; xyzw[b+2][2]=0;
                        xyzw[b+3][0]=pen+ox;    xyzw[b+3][1]=200+h+oy; xyzw[b+3][2]=0;
                        st[b+0][0]=s0; st[b+0][1]=t0;  st[b+1][0]=s1; st[b+1][1]=t0;
                        st[b+2][0]=s1; st[b+2][1]=t1;  st[b+3][0]=s0; st[b+3][1]=t1;
                        for (x = 0; x < 4; x++) { rgba[b+x][0]=cr; rgba[b+x][1]=cg;
                                                  rgba[b+x][2]=cb; rgba[b+x][3]=255; }
                        idx[nq*6+0]=b+3; idx[nq*6+1]=b+0; idx[nq*6+2]=b+2;
                        idx[nq*6+3]=b+2; idx[nq*6+4]=b+0; idx[nq*6+5]=b+1;
                        pen += gw + 2.3f;   /* fractional advance like prop text */
                        nq++;
                    }
                }
                numIndexes = nq * 6;
                glBegin(GL_TRIANGLE_STRIP);
                glArrayElement(idx[0]); glArrayElement(idx[1]); glArrayElement(idx[2]);
                last[0]=idx[0]; last[1]=idx[1]; last[2]=idx[2]; even = 0;
                for (i = 3; i < numIndexes; i += 3) {
                    if (!even && (idx[i+0]==last[2]) && (idx[i+1]==last[1])) {
                        glArrayElement(idx[i+2]); even = 1;
                    } else if (even && (idx[i+0]==last[2]) && (idx[i+1]==last[0])) {
                        glArrayElement(idx[i+2]); even = 0;
                    } else {
                        glEnd(); glBegin(GL_TRIANGLE_STRIP);
                        glArrayElement(idx[i+0]); glArrayElement(idx[i+1]); glArrayElement(idx[i+2]);
                        even = 0;
                    }
                    last[0]=idx[i+0]; last[1]=idx[i+1]; last[2]=idx[i+2];
                }
                glEnd();
            }
            glDisable(GL_BLEND);
            dump_case(20);
        }

        /* 21: MINIMAL real-font repro - plain textured quads (immediate mode),
         * modulate menu-red, blend, at Q3's ~0.74 minification. No strips, no
         * shadow, no VA. Uses the embedded real font atlas (top 40 rows). */
        {
            static unsigned char fa[256][256][4];
            GLuint ft;
            int yy2, xx2, k;
            const unsigned char *fr = fontrows_data();
            for (yy2 = 0; yy2 < 256; yy2++)
                for (xx2 = 0; xx2 < 256; xx2++) {
                    if (yy2 < 40) { const unsigned char *p = fr + (yy2*256+xx2)*4;
                        fa[yy2][xx2][0]=p[0]; fa[yy2][xx2][1]=p[1]; fa[yy2][xx2][2]=p[2]; fa[yy2][xx2][3]=p[3];
                    } else { fa[yy2][xx2][0]=fa[yy2][xx2][1]=fa[yy2][xx2][2]=fa[yy2][xx2][3]=0; }
                }
            glGenTextures(1, &ft);
            glBindTexture(GL_TEXTURE_2D, ft);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, fa);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glClear(GL_COLOR_BUFFER_BIT);
            glColor4ub(220, 40, 40, 255);
            {
                float px = 120.0f;
                for (k = 0; k < 12; k++) {
                    float gx = 2.0f + k * 19.0f;        /* atlas glyph cell */
                    float s0 = gx/256.0f, s1 = (gx+19.0f)/256.0f;
                    float t0 = 2.0f/256.0f, t1 = 29.0f/256.0f;
                    float w = 19.0f * 0.74f, h = 27.0f * 0.74f;   /* Q3 prop scale */
                    quad(px, 250.0f, px + w, 250.0f + h, s0, t0, s1, t1, 1);
                    px += w + 1.3f;                     /* fractional advance */
                }
            }
            glColor4f(1,1,1,1);
            glDisable(GL_BLEND);
            dump_case(21);

            /* 22: SAME real font, 1:1 scale (magnification) - isolates whether
             * the slicing is minification-only. */
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glClear(GL_COLOR_BUFFER_BIT);
            glColor4ub(220, 40, 40, 255);
            {
                float px = 120.0f;
                for (k = 0; k < 12; k++) {
                    float gx = 2.0f + k * 19.0f;
                    float s0 = gx/256.0f, s1 = (gx+19.0f)/256.0f;
                    float t0 = 2.0f/256.0f, t1 = 29.0f/256.0f;
                    quad(px, 250.0f, px + 19.0f, 250.0f + 27.0f, s0, t0, s1, t1, 1);
                    px += 19.0f + 2.0f;
                }
            }
            glColor4f(1,1,1,1);
            glDisable(GL_BLEND);
            dump_case(22);

            /* 23: minified 0.74x but with GL_NEAREST filter (point sample) -
             * if clean, the bug is in our BILINEAR minification path. */
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glClear(GL_COLOR_BUFFER_BIT);
            glColor4ub(220, 40, 40, 255);
            {
                float px = 120.0f;
                for (k = 0; k < 12; k++) {
                    float gx = 2.0f + k * 19.0f;
                    float s0 = gx/256.0f, s1 = (gx+19.0f)/256.0f;
                    float t0 = 2.0f/256.0f, t1 = 29.0f/256.0f;
                    float w = 19.0f * 0.74f, h = 27.0f * 0.74f;
                    quad(px, 250.0f, px + w, 250.0f + h, s0, t0, s1, t1, 1);
                    px += w + 1.3f;
                }
            }
            glColor4f(1,1,1,1);
            glDisable(GL_BLEND);
            dump_case(23);

            /* 24: EXACT Q3 menu recipe - real font, 1:1, dropshadow (black @ +2
             * then red @ base), immediate quads, blend. If THIS slices, the
             * garble is the shadow/color overdraw, not minification. */
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glClear(GL_COLOR_BUFFER_BIT);
            {
                int pass2;
                for (pass2 = 0; pass2 < 2; pass2++) {
                    float ox = pass2 ? 0.0f : 2.0f, oy = pass2 ? 0.0f : 2.0f;
                    float px = 120.0f;
                    if (pass2) glColor4ub(220, 40, 40, 255);
                    else       glColor4ub(0, 0, 0, 255);
                    for (k = 0; k < 12; k++) {
                        float gx = 2.0f + k * 19.0f;
                        float s0 = gx/256.0f, s1 = (gx+19.0f)/256.0f;
                        float t0 = 2.0f/256.0f, t1 = 29.0f/256.0f;
                        quad(px+ox, 250.0f+oy, px+19.0f+ox, 250.0f+27.0f+oy, s0, t0, s1, t1, 1);
                        px += 19.0f + 2.0f;
                    }
                }
            }
            glColor4f(1,1,1,1);
            glDisable(GL_BLEND);
            dump_case(24);

            /* 25: real font, minified 0.74x, but MIPMAPPED (LOD0 256 + LOD1 128
             * box-filtered) with GL_LINEAR_MIPMAP_NEAREST. If clean, driver-side
             * mip generation is THE fix for the minified-font garble. */
            {
                static unsigned char mip1[128][128][4];
                int mx, my, cc;
                for (my = 0; my < 128; my++)
                    for (mx = 0; mx < 128; mx++)
                        for (cc = 0; cc < 4; cc++)
                            mip1[my][mx][cc] = (unsigned char)((
                                fa[my*2][mx*2][cc] + fa[my*2][mx*2+1][cc] +
                                fa[my*2+1][mx*2][cc] + fa[my*2+1][mx*2+1][cc]) / 4);
                glBindTexture(GL_TEXTURE_2D, ft);
                glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glClear(GL_COLOR_BUFFER_BIT);
            glColor4ub(220, 40, 40, 255);
            {
                float px = 120.0f;
                for (k = 0; k < 12; k++) {
                    float gx = 2.0f + k * 19.0f;
                    float s0 = gx/256.0f, s1 = (gx+19.0f)/256.0f;
                    float t0 = 2.0f/256.0f, t1 = 29.0f/256.0f;
                    float w = 19.0f * 0.74f, h = 27.0f * 0.74f;
                    quad(px, 250.0f, px + w, 250.0f + h, s0, t0, s1, t1, 1);
                    px += w + 1.3f;
                }
            }
            glColor4f(1,1,1,1);
            glDisable(GL_BLEND);
            dump_case(25);
        }

        /* 16: same string, scale 1.0 (rebuild positions at 1:1) */
        pen = 100.0f; nq = 0;
        for (g = 0; g < 20; g++) {
            float gw = 18.0f + (float)(g % 7);
            float w = gw, h = 27.0f;
            int b = nq * 4;
            xyzw[b+0][0]=pen;   xyzw[b+0][1]=300;
            xyzw[b+1][0]=pen+w; xyzw[b+1][1]=300;
            xyzw[b+2][0]=pen+w; xyzw[b+2][1]=300+h;
            xyzw[b+3][0]=pen;   xyzw[b+3][1]=300+h;
            pen += w + 1.5f;
            nq++;
        }
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, nq * 6, GL_UNSIGNED_INT, idx);
        glDisable(GL_BLEND);
        dump_case(16);
    }

    fprintf(lg, "done\n");
    fclose(lg);
    wglMakeCurrent(0, 0);
    wglDeleteContext(rc);
    ReleaseDC(hwnd, dc);
    DestroyWindow(hwnd);
    return 0;
}
