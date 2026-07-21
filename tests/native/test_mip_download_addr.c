/* test_mip_download_addr.c
 *
 * Guards the mipmapped-texture download fix in the W2K/XP D3D HAL
 * (D3TXTR.C TEXTURELOAD, commit 08fd889, 2026-07-21; FINDINGS.md
 * "ROOT CAUSE FOUND+FIXED: black/garbage mipmapped textures in D3D").
 *
 * Root cause: 3dfx's final change to the file (rev 40, 10/25/00, "no longer
 * use surface local pointers") DELETED the per-LOD board-offset line in the
 * single-mip-level download path:
 *
 *     addr = psurfDst->mmData[nDstLOD].fpVidMem - _FX(textureHeapStart[tmuCnt]);
 *
 * leaving `addr` stale (whatever the previous non-mipped download set it to),
 * so every mip level's texels landed at the wrong board offset and the TMU
 * sampled unwritten memory -> black/garbage in all mipmapped D3D content
 * (Win9x rev 35 retains the line and works).
 *
 * Invariant encoded here: for every LOD of a mipmapped chain, the download
 * destination offset must equal the offset that surface creation assigned to
 * that LOD (initMipMapChain / txtrComputeVAddr chain layout: levels packed
 * consecutively, each advanced by the previous level's byte size). The buggy
 * path (addr not recomputed per level) violates this for every level of a
 * chain that follows any other allocation.
 */
#include "../munit.h"

/* --- chain layout model (initMipMapChain/txtrComputeVAddr, square 16bpp) --- */
#define HEAP_START 0x00200000u             /* _FX(textureHeapStart[tmu]) */

/* Byte size of a square level with edge (1<<lg) at 2 bytes/texel — the
 * txtMipMapSize[SST_AR_11][LOG2LOD(lg)] * bytesPerTexel term. */
static uint32_t level_bytes(int lg) { return (1u << lg) * (1u << lg) * 2u; }

/* fpVidMem of each level as surface creation lays the chain out:
 * chain start + sum of the byte sizes of all LARGER levels. */
static uint32_t chain_fpVidMem(uint32_t chain_start_vaddr, int top_lg, int lod) {
    uint32_t v = chain_start_vaddr;
    for (int l = 0; l < lod; l++)
        v += level_bytes(top_lg - l);
    return v;
}

/* --- download-side address computation ------------------------------------ */
/* FIXED (line restored): per-LOD board offset from the level's own fpVidMem. */
static uint32_t fixed_download_addr(uint32_t level_fpVidMem) {
    return level_fpVidMem - HEAP_START;
}
/* BUGGY (rev 40): addr keeps whatever the previous download left in it. */
static uint32_t buggy_download_addr(uint32_t stale_addr) {
    return stale_addr;
}

TEST(fixed_addr_matches_chain_layout_for_every_lod) {
    /* 256x256 16bpp chain allocated at board offset 0x40000 */
    const uint32_t chain_start = HEAP_START + 0x40000u;
    const int top_lg = 8;
    uint32_t expect = 0x40000u;
    for (int lod = 0; lod <= top_lg; lod++) {
        uint32_t fp = chain_fpVidMem(chain_start, top_lg, lod);
        CHECK_EQ_U(fixed_download_addr(fp), expect);
        expect += level_bytes(top_lg - lod);
    }
}

TEST(buggy_addr_hits_wrong_offset_for_every_lod) {
    /* Before the mipmapped download, a non-mipped texture at offset 0x10000
     * left addr = 0x10000 (desttxtr->start). The rev-40 code then downloads
     * every LOD of a chain at 0x40000 without recomputing addr. */
    const uint32_t stale = 0x10000u;
    const uint32_t chain_start = HEAP_START + 0x40000u;
    const int top_lg = 8;
    int any_correct = 0;
    for (int lod = 0; lod <= top_lg; lod++) {
        uint32_t want = fixed_download_addr(chain_fpVidMem(chain_start, top_lg, lod));
        if (buggy_download_addr(stale) == want) any_correct = 1;
    }
    CHECK(!any_correct,
          "rev-40 stale addr must never hit a correct per-LOD offset "
          "(this was the black-texture bug)");
}

TEST(big_texture_chain_offsets_are_strictly_increasing_and_disjoint) {
    /* 512x512 (TextureIsBig on Napalm): each level's region must start
     * exactly at the previous level's end — a packing error here corrupts
     * neighbouring levels (the 'garbage stripes' symptom). */
    const uint32_t chain_start = HEAP_START;
    const int top_lg = 9;
    uint32_t prev_end = 0;
    for (int lod = 0; lod <= top_lg; lod++) {
        uint32_t a = fixed_download_addr(chain_fpVidMem(chain_start, top_lg, lod));
        if (lod > 0)
            CHECK_EQ_U(a, prev_end);
        prev_end = a + level_bytes(top_lg - lod);
    }
}

MUNIT_MAIN("d3d-mip-download-addr (black-texture fix 08fd889)", {
    RUN(fixed_addr_matches_chain_layout_for_every_lod);
    RUN(buggy_addr_hits_wrong_offset_for_every_lod);
    RUN(big_texture_chain_offsets_are_strictly_increasing_and_disjoint);
})
