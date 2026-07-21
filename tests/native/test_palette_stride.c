/* test_palette_stride.c
 *
 * Guards the 0.3.2 "CS palette colors fixed" fix (retro3dfx-gl MesaFX ICD,
 * sst_ctable.c __glSSTColorTableEXT; see retro-3dfx optimized/CHANGELOG.md 0.3.2).
 *
 * Root cause: __glSSTColorTableEXT read a GL_RGBA color table (EXT_paletted_
 * texture) with a stride of 3 bytes/entry (as if GL_RGB) instead of 4, so every
 * GR_TEXTABLE_PALETTE entry after index 0 was shifted 1 byte further per index
 * -> GoldSrc (CS 1.6, the only stack that uses paletted textures; Q3 doesn't)
 * rendered wrong colors. Fix: read the source palette with stride 4 (RGBA).
 *
 * Invariant: entry i's R/G/B must come from src[4*i + {0,1,2}].
 */
#include "../munit.h"

/* Read entry `i` R,G,B out of an RGBA source palette using a given stride. */
static void read_entry(const uint8_t *src, int stride, int i,
                       uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = src[stride * i + 0];
    *g = src[stride * i + 1];
    *b = src[stride * i + 2];
}

TEST(stride4_reads_each_rgba_entry_correctly) {
    /* palette[i] = (R=i, G=i+100, B=i+200, A=255) */
    uint8_t pal[256 * 4];
    for (int i = 0; i < 256; i++) {
        pal[4*i+0] = (uint8_t)i;
        pal[4*i+1] = (uint8_t)(i + 100);
        pal[4*i+2] = (uint8_t)(i + 200);
        pal[4*i+3] = 255;
    }
    for (int i = 0; i < 256; i++) {
        uint8_t r, g, b;
        read_entry(pal, 4, i, &r, &g, &b);   /* the FIX: stride 4 */
        CHECK_EQ_U(r, (uint8_t)i);
        CHECK_EQ_U(g, (uint8_t)(i + 100));
        CHECK_EQ_U(b, (uint8_t)(i + 200));
    }
}

TEST(stride3_shifts_entries_the_bug) {
    /* Demonstrate the regression: stride 3 on RGBA data mis-reads entry 1+. */
    uint8_t pal[256 * 4];
    for (int i = 0; i < 256; i++) {
        pal[4*i+0] = (uint8_t)i;
        pal[4*i+1] = (uint8_t)(i + 100);
        pal[4*i+2] = (uint8_t)(i + 200);
        pal[4*i+3] = 255;
    }
    uint8_t r, g, b;
    read_entry(pal, 3, 1, &r, &g, &b);       /* BUG: stride 3 */
    /* entry 1 with stride 3 starts at byte 3 = entry-0's ALPHA (255), not R=1 */
    CHECK_EQ_U(r, 255);                      /* wrong on purpose - documents bug */
    CHECK(r != (uint8_t)1, "stride-3 must NOT read the correct entry-1 red");
}

MUNIT_MAIN("palette-stride (CS palette fix 0.3.2)", {
    RUN(stride4_reads_each_rgba_entry_correctly);
    RUN(stride3_shifts_entries_the_bug);
})
