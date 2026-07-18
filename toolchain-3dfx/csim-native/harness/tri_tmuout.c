/* RETRO3DFX CSIM harness: render a triangle in the VSA-100 C-model and dump the
 * framebuffer to a PPM. Proves the sim-side toolchain (build->render->readback)
 * and is the base for the sim-vs-hardware validation + the 2D column-drop repro.
 * Adapted from H5/DIAGS/CSIMTEST/TRIS.C (register-level triangle) + LFB readback.
 * Build: see ../build.sh (links harness against libcsim.a + HAL objects). */
#include <stdio.h>
#include <stdlib.h>
#include <h3.h>
#include <csim.h>

#define W 640
#define H 480

static void vtx(SstRegs *hw, float x, float y, float r, float g, float b)
{
    SETF(hw->sVx, x); SETF(hw->sVy, y);
    SETF(hw->sRed, r); SETF(hw->sGreen, g); SETF(hw->sBlue, b); SETF(hw->sAlpha, 255.0F);
}

int main(int argc, char **argv)
{
    SstRegs *hw;
    int x, y;
    FILE *f;
    const char *out = (argc > 1) ? argv[1] : "/tmp/csim_tri.ppm";

    /* CSIM init (per DIAGS/CSIMTEST/DRIVER.C): a fake hw base at 0x10000000, and
     * csimInitDriver() = allocate board memory (csimInitMemory) THEN fxHalInit +
     * fxHalMapBoard (csimInitHwAddress). Skipping the memory step leaves the sim's
     * RAM NULL and csimLoad32 segfaults, which is what the fxHalInit-only path did. */
    volatile unsigned char *board = (volatile unsigned char *)calloc(16 * 1024 * 1024, 1);
    hw = (SstRegs *)(0x10000000 + SST_3D_OFFSET);
    csimInitDriver(16 * 1024 * 1024, (volatile FxU32 *)board,
                   (volatile FxU32 *)SST_BASE_ADDRESS(hw));
    if (!fxHalInitRegisters(hw)) { printf("initRegisters FAILED\n"); return 1; }
    if (!fxHalInitGamma(hw, 1.0F)) { printf("initGamma FAILED\n"); return 1; }
    if (!fxHalInitVideo(hw, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, NULL)) {
        printf("initVideo FAILED\n"); return 1;
    }

    SET(hw->chipMask, 0x1);   /* single simulated chip */
    /* fxHalInitVideo skipped the video-register init (trace warning), so the 3D
     * color buffer stride was 0 -> every row collapsed onto y=0. Set it like
     * VIDEO.C:182-183: base 0, linear stride = width*2 bytes (16bpp). */
    SET(hw->colBufferAddr, 0);
    SET(hw->colBufferStride, W * 2);
    SET(hw->fbzMode, SST_RGBWRMASK);
    SET(hw->fbzColorPath, SST_PARMADJUST | SST_RGBSEL_TMUOUT);

    /* clear the buffer to mid-blue first (fastfill) so we can tell render from
     * readback: black = readback broken, blue = readback ok + triangle missed. */
    SET(hw->c1, 0x000000ffUL);         /* fastfill color */
    SET(hw->clipLeftRight, (0UL << 16) | W);   /* xmin=0 (hi), xmax=W (lo) */
    SET(hw->clipBottomTop, (H << 16) | 0UL);   /* ymax=H (hi), ymin=0 (lo) */
    SET(hw->fastfillCMD, 0);
    fxHalIdleNoNop(hw);

    /* Triangle via the FLOAT SETUP unit (sstTriangleSetup): write float vertices
     * to the s* setup registers with sSetupMode, then sBeginTriCMD / sDrawTriCMD
     * (cmdCodes SST_SBEGINTRICMD / SST_SDRAWTRICMD -> the setup unit computes the
     * gradients + edges, then rasterizes). This is the robust path (what glide /
     * the real driver use); hand-setting iterated gradients hung the span walker. */
    SET(hw->sSetupMode, SST_SETUP_RGB);
    vtx(hw, 120.0F, 100.0F, 255.0F,  40.0F,  40.0F); SET(hw->sBeginTriCMD, 0);
    vtx(hw, 520.0F, 140.0F, 255.0F,  40.0F,  40.0F); SET(hw->sDrawTriCMD, 0);
    vtx(hw, 300.0F, 400.0F, 255.0F,  40.0F,  40.0F); SET(hw->sDrawTriCMD, 0);
    /* NO idle: it spins (triangle leaves FBI_BUSY); triangle renders synchronously */

    /* DIRECT board-memory dump: the color buffer is at board offset 0 (16bpp 565,
     * stride W). csimReadPixel is pathologically slow post-render, so read the
     * malloc'd board RAM directly -> 640x480 PPM. Fast + reliable. */
    {
        volatile unsigned short *fb = (volatile unsigned short *)board;
        long nz = 0, i;
        for (i = 0; i < (long)W * H; i++) if (fb[i]) nz++;
        printf("framebuffer nonzero pixels: %ld / %d\n", nz, W * H);
        f = fopen(out, "wb");
        if (!f) { printf("cannot open %s\n", out); return 1; }
        fprintf(f, "P6\n%d %d\n255\n", W, H);
        for (y = 0; y < H; y++)
            for (x = 0; x < W; x++) {
                unsigned short p = fb[y * W + x];    /* 565 */
                unsigned char rgb[3];
                rgb[0] = (unsigned char)(((p >> 11) & 0x1F) << 3);
                rgb[1] = (unsigned char)(((p >>  5) & 0x3F) << 2);
                rgb[2] = (unsigned char)(( p        & 0x1F) << 3);
                fwrite(rgb, 1, 3, f);
            }
        fclose(f);
        printf("wrote %s (%dx%d)\n", out, W, H);
    }
    return 0;
}
