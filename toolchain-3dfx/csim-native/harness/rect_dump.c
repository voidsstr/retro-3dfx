/* CSIM harness (2D isolation): replicate DIAGS/CSIMTEST/DRIVER.C's proven 2D
 * rectangle fill EXACTLY, then read the destination back via csimReadPixel2d and
 * dump to PPM. Purpose: prove the sim renders + reads back AT ALL, before the 3D
 * triangle path. DRIVER.C does NOT call fxHalInitRegisters/Video — just
 * csimInitDriver + GUI-register writes + a cmdfifo + fxHalIdleNoNop. */
#include <stdio.h>
#include <stdlib.h>
#include <h3.h>
#include <csim.h>

#define W 640
#define H 480
#define CMDFIFO_START 0x03ff000

int main(int argc, char **argv)
{
    const char *out = (argc > 1) ? argv[1] : "/tmp/csim_rect.ppm";
    long x = 100, y = 80, w = 300, h = 200;
    SstRegs  *sst;
    SstGRegs *sstg;
    FxU32 *cp;
    FILE *f;
    int px, py;

    sst = (SstRegs *)(0x10000000 + SST_3D_OFFSET);
    csimInitDriver(4096 * 1024, malloc(4096 * 1024),
                   (volatile FxU32 *)SST_BASE_ADDRESS(sst));

    /* 2D GUI setup (exactly DRIVER.C): 32bpp dest, stride 640*4, fore color */
    sstg = (SstGRegs *)SST_GUI_ADDRESS(sst);
    SET(sstg->dstFormat, SSTG_PIXFMT_32BPP | (W * 4));
    SET(sstg->colorFore, 0x00c86432);          /* a distinctive RGB (200,100,50) */
    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max, 0xFFFFFFFF);

    /* draw the rect directly via the GUI command register */
    SET(sstg->dstXY,   (y << 16) | x);
    SET(sstg->dstSize, (h << 16) | w);
    SET(sstg->command, SSTG_RECTFILL | SSTG_GO | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT));

    /* also via a cmdfifo (DRIVER.C does both), then execute everything */
    fxHalInitCmdFifo(sst, 0, CMDFIFO_START, 0x1000, 0, 0, 0);
    cp = (FxU32 *)(SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + CMDFIFO_START);
    SET(cp[0], (0x7000000 << SSTCP_PKT2_MASK_SHIFT) | SSTCP_PKT2);
    SET(cp[1], (h << 16) | w);
    SET(cp[2], (y << 16) | x);
    SET(cp[3], SSTG_RECTFILL | SSTG_GO | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT));
    fxHalIdleNoNop(sst);

    /* read back the 2D destination (32bpp ARGB) -> PPM */
    f = fopen(out, "wb");
    if (!f) { printf("cannot open %s\n", out); return 1; }
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (py = 0; py < H; py++)
        for (px = 0; px < W; px++) {
            FxU32 p = csimReadPixel(sst, CSIM_BUF_2D_DST, px, py);
            unsigned char rgb[3];
            rgb[0] = (unsigned char)((p >> 16) & 0xFF);
            rgb[1] = (unsigned char)((p >>  8) & 0xFF);
            rgb[2] = (unsigned char)( p        & 0xFF);
            fwrite(rgb, 1, 3, f);
        }
    fclose(f);
    printf("wrote %s (%dx%d)\n", out, W, H);
    return 0;
}
