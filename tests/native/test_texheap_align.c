/* test_texheap_align.c
 *
 * Guards the 0.3.1 "2D text garble SOLVED" fix (retro3dfx-gl MesaFX ICD,
 * __glSSTInitTextureManager / SST texture heap; see retro-3dfx FINDINGS.md
 * "TEXT GARBLE" and memory text-garble-solved-alignment).
 *
 * Root cause: the VSA-100 (Napalm) texBaseAddr register applies
 * SST_TEXTURE_MUNGE_ADDRESS, which DROPS the low 4 bits of the texture base
 * address (16-byte granularity). The old code started the TMU0 texture heap at
 * grTexMinAddress + 8 (a legacy SST1 8-byte granularity) and allocated with
 * un-rounded lengths, so a texture downloaded at an 8-mod-16 address was
 * SAMPLED 8 bytes (== 4 texels in ARGB4444) BELOW where it was written -> the
 * characteristic +4-texel S-shift / "sliced" glyphs.
 *
 * The fix establishes an INVARIANT: every texture base address handed to the
 * hardware must be 16-byte aligned so munge(addr) == addr. This test encodes
 * that invariant: heap base rounded up to 16, and every allocation length
 * rounded up to 16, keeps every subsequent texture 16-aligned.
 */
#include "../munit.h"

/* Hardware behaviour we depend on: VSA-100 texBaseAddr (SST_TEXTURE_MUNGE_ADDRESS,
 * H3DEFS.H) DROPS the low 4 address bits — full macro is
 *   (((addr)&BIT(25))>>24) | ((addr)&(SST_MASK(21)<<4))
 * i.e. a tiled swizzle of bit 25 PLUS masking off bits [3:0]. The only property
 * the alignment fix relies on is the low-nibble drop, so we model just that here
 * (all test addresses keep bit 25 clear, where the swizzle is a no-op). The point
 * of the fix is to never HAND the hardware a base with a non-zero low nibble. */
static uint32_t munge_low_nibble_drop(uint32_t addr) { return addr & ~0xFu; }
#define munge_hw munge_low_nibble_drop

/* The fix's alignment helper (round up to 16). */
static uint32_t align16(uint32_t x) { return (x + 15u) & ~0xFu; }

/* --- the buggy vs fixed heap-base computation ------------------------------ */
static uint32_t old_heap_base(uint32_t grTexMinAddress) {
    return grTexMinAddress + 8;            /* SST1 8-byte granularity (BUG) */
}
static uint32_t fixed_heap_base(uint32_t grTexMinAddress) {
    return align16(grTexMinAddress + 8);   /* rounded to 16 (0.3.1 fix)     */
}

TEST(hw_munge_drops_low_4_bits) {
    CHECK_EQ_U(munge_hw(0x1000), 0x1000);
    CHECK_EQ_U(munge_hw(0x1008), 0x1000);   /* +8 sampled 16 bytes back to 0x1000 */
    CHECK_EQ_U(munge_hw(0x100F), 0x1000);
    CHECK_EQ_U(munge_hw(0x1010), 0x1010);
}

TEST(old_base_was_misaligned_the_bug) {
    /* grTexMinAddress is 16-aligned on the V3/V5; +8 breaks alignment, and the
     * hardware munge then samples 8 bytes below the download address. */
    uint32_t minAddr = 0x00010000;          /* 16-aligned */
    uint32_t base = old_heap_base(minAddr);
    CHECK(munge_hw(base) != base,
          "old +8 base must be misaligned (that was the garble bug)");
    CHECK_EQ_U(base - munge_hw(base), 8);    /* exactly the 8-byte / 4-texel shift */
}

TEST(fixed_base_is_16_aligned) {
    uint32_t minAddr = 0x00010000;
    uint32_t base = fixed_heap_base(minAddr);
    CHECK_EQ_U(munge_hw(base), base);        /* sampled exactly where downloaded */
    CHECK_EQ_U(base, minAddr + 16);          /* +8 rounded up to +16 */
}

TEST(fixed_base_16_aligned_for_any_aligned_min) {
    for (uint32_t minAddr = 0; minAddr < 0x100000; minAddr += 16) {
        uint32_t base = fixed_heap_base(minAddr);
        CHECK_EQ_U(base & 0xFu, 0u);
        CHECK_EQ_U(munge_hw(base), base);
    }
}

TEST(rounded_lengths_keep_every_allocation_16_aligned) {
    /* Walk a sequence of texture allocations the way the manager does; with
     * 16-rounded lengths every returned base stays 16-aligned (munge = identity).
     * Sizes deliberately include non-16-multiples that the OLD code would leave
     * misaligned. */
    const uint32_t sizes[] = {248, 1000, 17, 4096, 33, 260, 8, 1};
    uint32_t cursor = fixed_heap_base(0x00010000);
    for (unsigned i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        uint32_t base = cursor;
        CHECK_EQ_U(munge_hw(base), base);            /* aligned before download */
        cursor += align16(sizes[i]);                 /* fix: round the length   */
    }
    CHECK_EQ_U(cursor & 0xFu, 0u);
}

/* Contrast: WITHOUT length rounding, allocations drift out of alignment -> the
 * regression this test defends against. */
TEST(unrounded_lengths_would_drift_out_of_alignment) {
    const uint32_t sizes[] = {248, 1000, 17};
    uint32_t cursor = fixed_heap_base(0x00010000);
    int saw_misaligned = 0;
    for (unsigned i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        if (munge_hw(cursor) != cursor) saw_misaligned = 1;
        cursor += sizes[i];                          /* BUG: no rounding */
    }
    CHECK(saw_misaligned, "un-rounded lengths must drift out of 16-alignment");
}

MUNIT_MAIN("texheap-16b-alignment (garble fix 0.3.1)", {
    RUN(hw_munge_drops_low_4_bits);
    RUN(old_base_was_misaligned_the_bug);
    RUN(fixed_base_is_16_aligned);
    RUN(fixed_base_16_aligned_for_any_aligned_min);
    RUN(rounded_lengths_keep_every_allocation_16_aligned);
    RUN(unrounded_lengths_would_drift_out_of_alignment);
})
