/* RETRO3DFX CSIM harness: render a TEXTURED triangle to reproduce the 2D-text
 * column-drop bug at the chip-model level. Uploads a 64x64 vertical-stripe
 * texture (alternating red/green columns) and maps it magnified onto a quad; if
 * the rasterizer/sampler drops columns, the stripes reveal it. Builds on the
 * working flat-triangle harness (tri_dump.c). Debug with GDBG_LEVEL=170 (tex uv),
 * 172 (texel rgba), 150 (put_pixel). */
#include <stdio.h>
#include <stdlib.h>
#include <h3.h>
#include <csim.h>

#define W 640
#define H 480
#define TW 64          /* texture width  */
#define TH 64          /* texture height */

/* write float setup vertex with S/T/W (TMU0) */
static void tvtx(SstRegs *hw, float x, float y, float s, float t)
{
    SETF(hw->sVx, x);   SETF(hw->sVy, y);
    SETF(hw->sRed, 255.0F); SETF(hw->sGreen, 255.0F); SETF(hw->sBlue, 255.0F); SETF(hw->sAlpha, 255.0F);
    /* Glide s/t are pre-divided by w; with w=1, sSow0=s, sTow0=t, sOow0=1/w=1 */
    SETF(hw->sSow0, s);  SETF(hw->sTow0, t);  SETF(hw->sOow0, 1.0F);  SETF(hw->sOowfbi, 1.0F);
}

int main(int argc, char **argv)
{
    const char *out = (argc > 1) ? argv[1] : "/tmp/csim_tex.ppm";
    volatile unsigned char *board = (volatile unsigned char *)calloc(16 * 1024 * 1024, 1);
    volatile unsigned short *fb;
    SstRegs *hw;
    volatile unsigned short *texap;
    int x, y, u, v;
    long nz = 0, i;
    FILE *f;

    hw = (SstRegs *)(0x10000000 + SST_3D_OFFSET);
    csimInitDriver(16 * 1024 * 1024, (volatile FxU32 *)board,
                   (volatile FxU32 *)SST_BASE_ADDRESS(hw));
    if (!fxHalInitRegisters(hw)) { printf("initRegisters FAILED\n"); return 1; }
    if (!fxHalInitGamma(hw, 1.0F)) { printf("initGamma FAILED\n"); return 1; }
    if (!fxHalInitVideo(hw, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, NULL)) { printf("initVideo FAILED\n"); return 1; }

    /* --- upload the 64x64 vertical-stripe texture into the TMU0 aperture --- */
    /* aperture writes MUST go through the sim's store intercept (SET = halStore32),
     * NOT a direct pointer store. Write 32-bit words = 2 RGB565 texels each.
     * vertical stripes: even column red (0xF800), odd column green (0x07E0). */
    if(0){ /* upload OFF */
        volatile FxU32 *texap32 = (volatile FxU32 *)(SST_BASE_ADDRESS(hw) + SST_TEX0_OFFSET);
        (void)texap;
        for (v = 0; v < TH; v++)
            for (u = 0; u < TW; u += 2) {
                FxU32 lo = 0xF800;   /* even u  -> red   */
                FxU32 hi = 0x07E0;   /* odd  u+1-> green */
                SET(texap32[(v * TW + u) / 2], (hi << 16) | lo);
            }
    }

    SET(hw->chipMask, 0x1);
    SET(hw->colBufferAddr, 0x300000);
    SET(hw->colBufferStride, W * 2);


    /* --- texture registers: RGB565, point-sample, LOD for 64x64 (log2=6), 1x1 --- */
    SET(hw->texBaseAddr, 0);                        /* texture at tex-mem offset 0 (not tiled) */
    SET(hw->tLOD, SST_TLOD_MINMAX_INT(6, 6));       /* single LOD level 6 = 64 */
    SET(hw->textureMode, SST_RGB565 | SST_TC_ZERO_OTHER | SST_TC_ADD_CLOCAL); /* RGB565 + decal (out=texel) */
    SET(hw->tDetail, 0);

    /* FBI color path: take color from the TMU (texture) output */
    SET(hw->fbzMode, SST_RGBWRMASK);
    SET(hw->fbzColorPath, SST_PARMADJUST | SST_RGBSEL_TREXOUT | SST_ENTEXTUREMAP);  /* texture color path + ENABLE */

    /* --- two triangles = a magnified quad (64x64 tex -> 256x256 screen = 4x) --- */
    /* TODO texturing: SST_RGBSEL_TMUOUT above + SST_SETUP_ST0|W0 here + texcoords
     * currently prevents the triangle from rasterizing (0 px) -> needs TREX init
     * (trexInit0/1) and/or the texture color-combine bits in textureMode. The
     * texture UPLOADS correctly (2048 red + 2048 green stripes at board 0) and the
     * untextured quad RENDERS (69632 px). This is the next debug step. */
    SET(hw->sSetupMode, SST_SETUP_RGB | SST_SETUP_ST0 | SST_SETUP_W0);
    /* quad corners: screen (100,100)-(356,356), tex (0,0)-(64,64) */
    tvtx(hw, 100.0F, 100.0F,  0.0F,  0.0F); SET(hw->sBeginTriCMD, 0);
    tvtx(hw, 356.0F, 100.0F, 64.0F,  0.0F); SET(hw->sDrawTriCMD, 0);
    tvtx(hw, 356.0F, 356.0F, 64.0F, 64.0F); SET(hw->sDrawTriCMD, 0);
    tvtx(hw, 100.0F, 100.0F,  0.0F,  0.0F); SET(hw->sBeginTriCMD, 0);
    tvtx(hw, 356.0F, 356.0F, 64.0F, 64.0F); SET(hw->sDrawTriCMD, 0);
    tvtx(hw, 100.0F, 356.0F,  0.0F, 64.0F); SET(hw->sDrawTriCMD, 0);
    /* no idle: renders synchronously; idle spins on FBI_BUSY */

    /* dump framebuffer (board offset 0, 16bpp 565) */
    { long nred=0, ngrn=0, first_red=-1, first_grn=-1, N=(16*1024*1024)/2;
      volatile unsigned short *all=(volatile unsigned short*)board;
      for (i=0;i<N;i++){ unsigned short vv=all[i];
        if(vv==0xF800){nred++; if(first_red<0)first_red=i;}
        if(vv==0x07E0){ngrn++; if(first_grn<0)first_grn=i;} }
      printf("WHOLE BOARD: red=%ld (first word %ld @0x%lx)  green=%ld (first word %ld @0x%lx)\n",
             nred,first_red,first_red*2,ngrn,first_grn,first_grn*2); }
    fb = (volatile unsigned short *)(board + 0x300000);
    for (i = 0; i < (long)W * H; i++) if (fb[i]) nz++;
    printf("framebuffer@0x300000 nonzero: %ld / %d\n", nz, W * H);
    f = fopen(out, "wb");
    if (!f) return 1;
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++) {
            unsigned short p = fb[y * W + x];
            unsigned char rgb[3];
            rgb[0] = (unsigned char)(((p >> 11) & 0x1F) << 3);
            rgb[1] = (unsigned char)(((p >>  5) & 0x3F) << 2);
            rgb[2] = (unsigned char)(( p        & 0x1F) << 3);
            fwrite(rgb, 1, 3, f);
        }
    fclose(f);
    printf("wrote %s\n", out);
    return 0;
}
