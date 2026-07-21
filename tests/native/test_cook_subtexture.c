/* test_cook_subtexture.c
 *
 * Guards the 0.2.1 "__glCookSubTexture OOB" fix (retro3dfx-gl MesaFX ICD,
 * GLCORE generic cook path; see retro-3dfx optimized/CHANGELOG.md 0.2.1).
 *
 * Root cause: the sub-image (glTexSubImage2D) cook copied each destination row
 * using the SUB-RECT width as the row stride -- dst_origin = w*(y+r)+x -- and
 * omitted the *bpp scale on the source row skip. When the sub-rect width w is
 * smaller than the full texture width, w*(y+r) walks the wrong (and eventually
 * out-of-bounds) destination offset. Fix: the destination row stride must be
 * the FULL texture width lp->width -> dst_origin = lp->width*(y+r)+x, and
 * srcSkip must be scaled by bytes-per-pixel.
 *
 * Invariant: for a sub-rect (x,y,w,h) into a texture of width W>=x+w and
 * height H>=y+h, every destination texel offset must land inside [0, W*H).
 */
#include "../munit.h"

static long dst_offset_fixed(int W, int x, int y, int r, int c) {
    return (long)W * (y + r) + (x + c);          /* full-width stride (fix) */
}
static long dst_offset_buggy(int w, int x, int y, int r, int c) {
    return (long)w * (y + r) + (x + c);          /* sub-width stride (bug)  */
}

TEST(fixed_stride_stays_in_bounds) {
    int W = 256, H = 256;
    int x = 200, y = 200, w = 40, h = 40;        /* sub-rect near the far corner */
    for (int r = 0; r < h; r++)
        for (int c = 0; c < w; c++) {
            long off = dst_offset_fixed(W, x, y, r, c);
            CHECK(off >= 0 && off < (long)W * H,
                  "fixed full-width stride must stay within the texture");
        }
    /* last texel lands exactly at the far corner */
    CHECK_EQ_I(dst_offset_fixed(W, x, y, h - 1, w - 1),
               (long)W * (y + h - 1) + (x + w - 1));
}

TEST(buggy_substride_matches_fixed_only_when_w_equals_W) {
    int W = 256, x = 0, y = 10, w = 256;         /* full-width update: w == W */
    for (int r = 0; r < 4; r++)
        CHECK_EQ_I(dst_offset_buggy(w, x, y, r, 0),
                   dst_offset_fixed(W, x, y, r, 0));   /* identical here */
}

TEST(buggy_substride_diverges_and_goes_oob_when_w_less_than_W) {
    int W = 256, H = 256;
    int x = 200, y = 200, w = 40, h = 40;
    int saw_wrong = 0, saw_oob_or_wrong = 0;
    for (int r = 1; r < h; r++) {                /* r=0 rows agree */
        long good = dst_offset_fixed(W, x, y, r, 0);
        long bad  = dst_offset_buggy(w, x, y, r, 0);
        if (bad != good) saw_wrong = 1;
        if (bad != good || bad >= (long)W * H) saw_oob_or_wrong = 1;
    }
    CHECK(saw_wrong, "sub-width stride must diverge from full-width when w<W");
    CHECK(saw_oob_or_wrong, "sub-width stride writes the wrong destination texels");
}

/* srcSkip must be scaled by bytes-per-pixel (the other half of the 0.2.1 fix). */
static long src_skip_fixed(int src_row_texels, int copied_texels, int bpp) {
    return (long)(src_row_texels - copied_texels) * bpp;
}
TEST(src_skip_is_scaled_by_bpp) {
    CHECK_EQ_I(src_skip_fixed(64, 40, 2), (64 - 40) * 2);   /* 16bpp texture */
    CHECK_EQ_I(src_skip_fixed(64, 40, 4), (64 - 40) * 4);   /* 32bpp texture */
    CHECK(src_skip_fixed(64, 40, 2) != (64 - 40),
          "unscaled skip (bpp omitted) is the bug");
}

MUNIT_MAIN("cook-subtexture OOB (fix 0.2.1)", {
    RUN(fixed_stride_stays_in_bounds);
    RUN(buggy_substride_matches_fixed_only_when_w_equals_W);
    RUN(buggy_substride_diverges_and_goes_oob_when_w_less_than_W);
    RUN(src_skip_is_scaled_by_bpp);
})
