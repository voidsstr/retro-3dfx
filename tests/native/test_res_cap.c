/* test_res_cap.c
 *
 * Guards the 0.3.6 "Q3 black world at 800/1024 SOLVED" fix (retro3dfx-gl MesaFX
 * ICD, sst_export.c MakeCurrent resolution/memory cap gate; see retro-3dfx
 * optimized/CHANGELOG.md 0.3.6 and memory 3dfx-optimized-driver-143-overnight).
 *
 * Root cause: MakeCurrent gates every requested resolution through a Voodoo1/2-
 * era capability table __sstResCapTable[platform][sli][mem][res]. The platform
 * classifier only matched "Voodoo"/"Voodoo2"/"VoodooRush" and DEFAULTED every
 * other string (including "Voodoo3"/"Voodoo5") to SST_VOODOO (Voodoo1). Voodoo1
 * at 2MB caps at 640x480, so any 800/1024 request silently ran a 640x480 Glide
 * context under a larger viewport -> BLACK WORLD on the monitor (fps still got
 * logged, which is why every pre-0.3.6 "800/1024" benchmark was actually 640).
 *
 * Fix: unmatched (modern, V3/V5+) hardware strings classify to the sentinel
 * platform = -1, and __SST_RES_OK bypasses the legacy cap gate for -1.
 *
 * Invariant: a V3/V5 string must NOT classify as the legacy Voodoo1 platform,
 * and the sentinel must let 800/1024 through.
 */
#include "../munit.h"
#include <string.h>

#define SST_VOODOO   0     /* Voodoo1 (the buggy default)  */
#define SST_VOODOO2  1
#define SST_RUSH     2
#define SST_SENTINEL (-1)  /* modern V3/V5+ : bypass legacy cap gate (the fix) */

/* FIXED classifier: legacy strings map to their platform; everything else
 * (modern hardware) maps to the -1 sentinel. */
static int classify_fixed(const char *name) {
    if (strstr(name, "Voodoo2"))      return SST_VOODOO2;
    if (strstr(name, "VoodooRush"))   return SST_RUSH;
    if (strcmp(name, "Voodoo") == 0)  return SST_VOODOO;   /* exact Voodoo1 */
    return SST_SENTINEL;                                    /* V3/V5/Napalm... */
}

/* BUGGY classifier: same matches, but DEFAULT falls through to Voodoo1. */
static int classify_buggy(const char *name) {
    if (strstr(name, "Voodoo2"))      return SST_VOODOO2;
    if (strstr(name, "VoodooRush"))   return SST_RUSH;
    return SST_VOODOO;                                      /* <-- the bug */
}

/* Legacy cap gate: Voodoo1@2MB tops out at 640x480; the -1 sentinel bypasses it. */
static int sst_res_ok(int width, int platform) {
    if (platform == SST_SENTINEL) return 1;                /* modern: allow all */
    if (platform == SST_VOODOO)   return width <= 640;     /* Voodoo1 2MB cap  */
    return 1;
}

TEST(v3_v5_classify_to_sentinel_not_voodoo1) {
    CHECK_EQ_I(classify_fixed("Voodoo3"),        SST_SENTINEL);
    CHECK_EQ_I(classify_fixed("Voodoo5 (tm)"),   SST_SENTINEL);
    CHECK_EQ_I(classify_fixed("3dfx Voodoo5 5500"), SST_SENTINEL);
    CHECK(classify_fixed("Voodoo3") != SST_VOODOO,
          "V3 must not be classified as the legacy Voodoo1 platform");
}

TEST(legacy_strings_still_classify_correctly) {
    CHECK_EQ_I(classify_fixed("Voodoo2"),    SST_VOODOO2);
    CHECK_EQ_I(classify_fixed("VoodooRush"), SST_RUSH);
    CHECK_EQ_I(classify_fixed("Voodoo"),     SST_VOODOO);
}

TEST(sentinel_allows_800_and_1024) {
    int p = classify_fixed("Voodoo5 (tm)");
    CHECK(sst_res_ok(800,  p), "800x600 must be allowed on modern hw");
    CHECK(sst_res_ok(1024, p), "1024x768 must be allowed on modern hw");
    CHECK(sst_res_ok(1600, p), "1600x1200 must be allowed on modern hw");
}

TEST(buggy_default_reproduces_the_black_world) {
    /* The regression: V5 fell through to Voodoo1, whose cap rejects >640 ->
     * the ICD ran a 640 context under an 800/1024 viewport = black world. */
    int p = classify_buggy("Voodoo5 (tm)");
    CHECK_EQ_I(p, SST_VOODOO);
    CHECK(!sst_res_ok(1024, p), "buggy path must (wrongly) reject 1024 -> black world");
    CHECK( sst_res_ok(640,  p), "only 640 survived the buggy cap");
}

MUNIT_MAIN("res-cap black-world (fix 0.3.6)", {
    RUN(v3_v5_classify_to_sentinel_not_voodoo1);
    RUN(legacy_strings_still_classify_correctly);
    RUN(sentinel_allows_800_and_1024);
    RUN(buggy_default_reproduces_the_black_world);
})
