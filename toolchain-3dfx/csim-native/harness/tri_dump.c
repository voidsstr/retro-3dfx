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
    hw = (SstRegs *)(0x10000000 + SST_3D_OFFSET);
    csimInitDriver(16 * 1024 * 1024, (volatile FxU32 *)calloc(16 * 1024 * 1024, 1),
                   (volatile FxU32 *)SST_BASE_ADDRESS(hw));
    if (!fxHalInitRegisters(hw)) { printf("initRegisters FAILED\n"); return 1; }
    if (!fxHalInitGamma(hw, 1.0F)) { printf("initGamma FAILED\n"); return 1; }
    if (!fxHalInitVideo(hw, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, NULL)) {
        printf("initVideo FAILED\n"); return 1;
    }

    SET(hw->fbzMode, SST_RGBWRMASK);
    SET(hw->fbzColorPath, SST_PARMADJUST);
    SET(hw->sSetupMode, SST_SETUP_RGB);

    /* clear the back buffer to mid-blue first (fastfill), so a black readback
     * means "readback broken" and a blue field means "readback works, triangle
     * didn't draw" — disambiguates render vs readback. */
    SET(hw->c1, (0x0000ffUL));         /* fastfill color (fbiColor1) */
    SET(hw->clipLeftRight, (0UL<<16) | W);
    SET(hw->clipBottomTop, (0UL<<16) | H);
    SET(hw->fastfillCMD, 0);
    fxHalIdleNoNop(hw);

    /* one Gouraud triangle, big enough to sample many pixels */
    vtx(hw, 120.0F, 100.0F, 255.0F,   0.0F,   0.0F); SET(hw->sBeginTriCMD, 0);
    vtx(hw, 520.0F, 140.0F,   0.0F, 255.0F,   0.0F); SET(hw->sDrawTriCMD, 0);
    vtx(hw, 300.0F, 400.0F,   0.0F,   0.0F, 255.0F); SET(hw->sDrawTriCMD, 0);
    fxHalIdleNoNop(hw);    /* flush the command fifo so the triangle executes */

    /* dump the color buffer to PPM (read 565, expand to 888). Read the BACK
     * buffer (we drew there); if the sim ignores buffer id, front == back. */
    f = fopen(out, "wb");
    if (!f) { printf("cannot open %s\n", out); return 1; }
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++) {
            FxU32 p = csimReadPixel(hw, CSIM_BUF_3D_BACK, x, y);   /* 565 */
            unsigned char rgb[3];
            rgb[0] = (unsigned char)(((p >> 11) & 0x1F) << 3);
            rgb[1] = (unsigned char)(((p >>  5) & 0x3F) << 2);
            rgb[2] = (unsigned char)(( p        & 0x1F) << 3);
            fwrite(rgb, 1, 3, f);
        }
    fclose(f);
    printf("wrote %s (%dx%d)\n", out, W, H);
    return 0;
}
