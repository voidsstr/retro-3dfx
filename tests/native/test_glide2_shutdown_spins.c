/* test_glide2_shutdown_spins.c
 *
 * Guards the UT99-exit-hang fix (H5/GLIDE/SRC GSST.C grSstIdle +
 * FIFO.C _grCommandTransportMakeRoom): when the accelerator wedges
 * (SST_BUSY stuck / fifo read pointer frozen), both user-mode spins on
 * the grSstWinClose path originally looped FOREVER at 100% CPU, so
 * hwcRestoreVideo never ran and the desktop was never restored (screen
 * left garbled, UnrealTournament.exe pinned at 98% CPU on exit).
 *
 * The fix bounds both loops at ~4M no-progress polls (seconds of wall
 * time on real hw) and then proceeds — same philosophy as the display
 * driver's H3MakeRoom / DdFlip WEDGE-BREAK@50M.
 *
 * This test executes the exact loop logic with a simulated status /
 * read-pointer source and asserts:
 *   1. healthy hw: loops exit via the normal path (no wedge break).
 *   2. wedged hw:  loops terminate within the bound (would previously hang).
 *   3. slow-but-alive hw: progress resets the wedge counter (no false break).
 */
#include "../munit.h"

#define SST_BUSY 1u
#define WEDGE_POLLS 4000000UL

/* ---- grSstIdle poll loop (GSST.C), status supplied by callback ---------- */
typedef uint32_t (*status_fn)(uint64_t poll);

/* Returns number of polls executed; *wedged set if the bound tripped. */
static uint64_t idle_loop(status_fn status, int *wedged) {
    uint32_t i = 0, busyPolls = 0;
    uint64_t polls = 0;
    *wedged = 0;
    do {
        if (status(polls++) & SST_BUSY) {
            i = 0; /* Reset counter */
            if (++busyPolls > WEDGE_POLLS) { *wedged = 1; break; }
        } else
            i++;
    } while (i < 3);
    return polls;
}

static uint32_t st_idle_immediately(uint64_t p) { (void)p; return 0; }
static uint32_t st_busy_forever(uint64_t p)     { (void)p; return SST_BUSY; }
static uint32_t st_busy_then_idle(uint64_t p)   { return p < 1000 ? SST_BUSY : 0; }

/* ---- makeroom stall loop (FIFO.C), read-ptr supplied by callback -------- */
typedef uint32_t (*readptr_fn)(uint64_t poll);

static uint64_t makeroom_loop(readptr_fn rp, int32_t roomToReadPtr,
                              int32_t blockSize, int *wedged) {
    uint32_t lastHwRead = rp(0);
    uint32_t stuckPolls = 0;
    uint64_t polls = 0;
    *wedged = 0;
    while (roomToReadPtr < blockSize) {
        uint32_t curReadPtr = rp(++polls);
        uint32_t curReadDist = curReadPtr - lastHwRead;
        if (curReadDist == 0) {
            if (++stuckPolls > WEDGE_POLLS) {
                roomToReadPtr = blockSize; /* WEDGE-BREAK: force room */
                *wedged = 1;
                break;
            }
        } else
            stuckPolls = 0;
        roomToReadPtr += (int32_t)curReadDist;
        lastHwRead = curReadPtr;
    }
    return polls;
}

static uint32_t rp_frozen(uint64_t p) { (void)p; return 0x1000; }
static uint32_t rp_advancing(uint64_t p) { return 0x1000 + (uint32_t)(p * 8); }
/* advances 8 bytes only every 1M polls: slow but alive */
static uint32_t rp_glacial(uint64_t p) { return 0x1000 + (uint32_t)(p / 1000000) * 8; }

TEST(idle_hw_exits_normally) {
    int wedged; uint64_t polls = idle_loop(st_idle_immediately, &wedged);
    CHECK(!wedged, "idle hw must not trip wedge break");
    CHECK_EQ_U(polls, 3); /* 3 consecutive idle reads */
}

TEST(transient_busy_exits_normally) {
    int wedged; uint64_t polls = idle_loop(st_busy_then_idle, &wedged);
    CHECK(!wedged, "transiently-busy hw must not trip wedge break");
    CHECK_EQ_U(polls, 1003); /* exits 3 polls after going idle */
}

TEST(stuck_busy_trips_wedge_break) {
    int wedged; uint64_t polls = idle_loop(st_busy_forever, &wedged);
    CHECK(wedged, "stuck-busy hw must trip the wedge break (was: infinite spin)");
    CHECK(polls <= WEDGE_POLLS + 1, "bounded at ~4M polls");
}

TEST(makeroom_advancing_drains_normally) {
    int wedged; makeroom_loop(rp_advancing, 0, 64, &wedged);
    CHECK(!wedged, "advancing read ptr must not trip wedge break");
}

TEST(makeroom_frozen_trips_wedge_break) {
    int wedged; uint64_t polls = makeroom_loop(rp_frozen, 0, 64, &wedged);
    CHECK(wedged, "frozen read ptr must trip the wedge break (was: infinite spin)");
    CHECK(polls <= WEDGE_POLLS + 1, "bounded at ~4M polls");
}

TEST(makeroom_glacial_progress_no_false_break) {
    /* each advance resets the stuck counter, so a slow-but-alive fifo
     * drains without a false wedge break. 8 advances of 8 bytes for 64
     * bytes of room = ~8M polls total, MORE than the bound — only legal
     * because progress resets the counter. */
    int wedged; uint64_t polls = makeroom_loop(rp_glacial, 0, 64, &wedged);
    CHECK(!wedged, "slow-but-alive fifo must not trip wedge break");
    CHECK(polls > WEDGE_POLLS, "glacial case really did outlast the bound");
}

MUNIT_MAIN("glide2-shutdown-spins (UT99 exit hang)", {
    RUN(idle_hw_exits_normally);
    RUN(transient_busy_exits_normally);
    RUN(stuck_busy_trips_wedge_break);
    RUN(makeroom_advancing_drains_normally);
    RUN(makeroom_frozen_trips_wedge_break);
    RUN(makeroom_glacial_progress_no_false_break);
})
