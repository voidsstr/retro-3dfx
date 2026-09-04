/* fxscan2.c - per-chip scanout-register instrument for the DEPLOYED vintage
 * 3dfx H5 driver (3dfxv5d.dll) on a 4-chip Voodoo5 6000.  NO DRIVER REBUILD.
 *
 * Uses only escapes the SHIPPING 3dfxv5d.dll already answers:
 *   HWCEXT_GETDEVICECONFIG 0x02, HWCEXT_ALLOCCONTEXT 0x01,
 *   HWCEXT_GETLINEARADDR   0x03, HWCEXT_GET_SLAVE_REGS 0x19,
 *   HWCEXT_RELEASECONTEXT  0x07
 * verified present in optimized/deployed-133-v56k-20260811/3dfxv5d.dll.
 *
 * GET_SLAVE_REGS returns, per chip unit 0..3, the VAs of four register
 * windows mapped into THIS process by VideoPortMapMemory:
 *   [0]=SstIORegs (video/CRTC)  [1]=SstCRegs  [2]=SstGRegs(2D)  [3]=SstRegs(3D)
 * (Miniport SLIAA.C:1621-1713 GlideMapSlaveChips; H3.C:3394-3511.)
 * They are READ/WRITE mappings, which is what makes `poke` and `fixtest`
 * possible with no rebuild.
 *
 *   fxscan2 dump  [--json]        per-chip register table
 *   fxscan2 diff                  exit 1 if the must-match set diverges
 *   fxscan2 phase [secs]          per-chip CRTC frequency + relative phase
 *   fxscan2 poke  <chip> <off> <val> [holdms]
 *   fxscan2 fixtest [holdms]      copy master vidScreenSize/dudx to slaves,
 *                                 pulse each slave's video processor
 *
 * Build: i686-w64-mingw32-gcc -O2 -Wall -o fxscan2.exe fxscan2.c -lgdi32
 */
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define EXT_HWC       0x3df3
#define EXT_HWC_OLD   0x0fd3

#define HWCEXT_ALLOCCONTEXT     0x01
#define HWCEXT_GETDEVICECONFIG  0x02
#define HWCEXT_GETLINEARADDR    0x03
#define HWCEXT_RELEASECONTEXT   0x07
#define HWCEXT_GET_SLAVE_REGS   0x19
#define HWCEXT_PROTOCOLREV      0x01
#define HWCEXT_MAX_BASEADDR     0x09
#define HWCEXT_MAX_SLAVE_REGS   4
#define MAXCHIP                 4

typedef unsigned int u32;

/* hwcExtRequest_t: 2 leading dwords + a union whose largest member is
 * hwcExtSliAAReq_t (14 dwords). 16 dwords of pad covers it. HWCEXT.H:640-668.
 * DrvEscape does NOT size-check EXT_HWC (ESCAPE.C:395-397), so an oversized
 * buffer is safe; an undersized one is not. */
typedef struct { u32 contextID, which; u32 opt[16]; } hwcReq_t;
typedef struct { int resStatus;        u32 opt[16]; } hwcRes_t;

/* ---- SstIORegs byte offsets, derived by counting FxU32 members of
 * H5/INCLUDE/H3REGS.H:121-194 (verified: vidProcCfg 0x5C, vidCurrentLine
 * 0x94, vidScreenSize 0x98, vgaRegister[0] 0xB0, vidDesktopStartAddr 0xE4). */
#define IO_STATUS            0x000
#define IO_LFBMEMORYCONFIG   0x00C
#define IO_MISCINIT0         0x010
#define IO_MISCINIT1         0x014
#define IO_VGAINIT0          0x028
#define IO_VGAINIT1          0x02C
#define IO_PLLCTRL0          0x040
#define IO_PLLCTRL1          0x044
#define IO_PLLCTRL2          0x048
#define IO_DACMODE           0x04C
#define IO_VIDMAXRGBDELTA    0x058
#define IO_VIDPROCCFG        0x05C
#define IO_VIDCURRENTLINE    0x094
#define IO_VIDSCREENSIZE     0x098
#define IO_VIDOVLSTARTCOORD  0x09C
#define IO_VIDOVLENDCOORD    0x0A0
#define IO_VIDOVLDUDX        0x0A4
#define IO_VIDOVLDUDXOFF     0x0A8
#define IO_VGAREGISTER0      0x0B0
#define IO_VIDDESKTOPSTART   0x0E4
#define IO_VIDDESKTOPSTRIDE  0x0E8

#define SST_VIDEO_PROCESSOR_EN 0x00000001u
#define SSTG_IS_TILED          0x80000000u

typedef struct { u32 off; const char *name; int mustmatch; const char *note; } REGDEF;

/* mustmatch: 1 = the miniport copies it master->slave, or SLI correctness
 *                demands the four chips agree -> a divergence is a finding
 *            0 = informational / legitimately per-chip                     */
static const REGDEF g_regs[] = {
  { IO_STATUS,           "status",            0, "" },
  { IO_LFBMEMORYCONFIG,  "lfbMemoryConfig",   0, "MUX reg - read reflects bits29:30 select" },
  { IO_MISCINIT0,        "miscInit0",         1, "Y-origin; NOT in the miniport 5-reg copy" },
  { IO_MISCINIT1,        "miscInit1",         0, "bit8 = POWERDOWN_DAC" },
  { IO_VGAINIT0,         "vgaInit0",          1, "" },
  { IO_VGAINIT1,         "vgaInit1",          1, "" },
  { IO_PLLCTRL0,         "pllCtrl0",          1, "" },
  { IO_PLLCTRL1,         "pllCtrl1",          1, "video PLL" },
  { IO_PLLCTRL2,         "pllCtrl2",          1, "video PLL" },
  { IO_DACMODE,          "dacMode",           0, "" },
  { IO_VIDMAXRGBDELTA,   "vidMaxRGBDelta",    1, "DOS copies it, W2K does not" },
  { IO_VIDPROCCFG,       "vidProcCfg",        1, "COPIED by miniport SLIAA.C:3558" },
  { IO_VIDSCREENSIZE,    "vidScreenSize",     1, "CANDIDATE #1: NOT copied by W2K" },
  { IO_VIDOVLSTARTCOORD, "vidOvlStartCoords", 1, "DOS copies it, W2K does not" },
  { IO_VIDOVLENDCOORD,   "vidOvlEndCoord",    1, "DOS copies it, W2K does not" },
  { IO_VIDOVLDUDX,       "vidOverlayDudx",    1, "CANDIDATE: hsync-mid-line, master-only" },
  { IO_VIDOVLDUDXOFF,    "vidOvlDudxOffSrcW", 1, "COPIED by miniport" },
  { IO_VIDDESKTOPSTART,  "vidDesktopStart",   1, "COPIED by miniport (DOS refuses to)" },
  { IO_VIDDESKTOPSTRIDE, "vidDesktopStride",  1, "COPIED by miniport" },
};
#define NREGS ((int)(sizeof(g_regs)/sizeof(g_regs[0])))

static HDC   g_dc;
static int   g_esc = EXT_HWC;
static u32   g_numChips = 1;
static BYTE *g_io[MAXCHIP];        /* per-chip SstIORegs base */
static BYTE *g_r3d[MAXCHIP];
static int   g_json = 0;

static u32 RD(BYTE *b, u32 off) { return *(volatile u32 *)(b + off); }
static void WR(BYTE *b, u32 off, u32 v) { *(volatile u32 *)(b + off) = v; }

static int esc(hwcReq_t *rq, hwcRes_t *rs)
{
    memset(rs, 0, sizeof(*rs));
    return ExtEscape(g_dc, g_esc, sizeof(*rq), (LPCSTR)rq, sizeof(*rs), (LPSTR)rs);
}

static int open_card(void)
{
    hwcReq_t rq; hwcRes_t rs; u32 i, j;

    if (!(g_dc = CreateDCA("DISPLAY", NULL, NULL, NULL))) {
        fprintf(stderr, "ERR CreateDC(DISPLAY) failed %lu\n", GetLastError());
        return 0;
    }
    memset(&rq, 0, sizeof(rq)); rq.which = HWCEXT_GETDEVICECONFIG;
    g_esc = EXT_HWC;
    if (esc(&rq, &rs) <= 0) {
        g_esc = EXT_HWC_OLD;
        if (esc(&rq, &rs) <= 0) {
            fprintf(stderr, "ERR no 3dfx HWCEXT escape (is 3dfxv5d.dll the active driver?)\n");
            return 0;
        }
    }
    /* deviceConfigRes: devNum,vendorID,deviceID,fbRam,chipRev,pciStride,
     *                  hwStride,tileMark,isMaster,numChips  (HWCEXT.H:227-242) */
    printf("escape       0x%04x\n", g_esc);
    printf("vendor/dev   %04X:%04X rev %u\n", rs.opt[1], rs.opt[2], rs.opt[4]);
    printf("fbRam        %u (%u MB)\n", rs.opt[3], rs.opt[3] >> 20);
    printf("numChips     %u\n", rs.opt[9]);
    printf("ddTilePitch  %u   ddTileStride %u   ddTileMark %08Xh\n",
           rs.opt[5], rs.opt[6], rs.opt[7]);
    g_numChips = rs.opt[9];
    if (g_numChips == 0 || g_numChips > MAXCHIP) g_numChips = 1;

    /* ORDER MATTERS.  Do NOT send ALLOCCONTEXT first: hwcAllocContext
     * (HWCEXT.C:676-711) allocates the per-PID GLIDESTATE, and
     * hwcGetLinearAddr (HWCEXT.C:814-825) then takes its "a GLIDESTATE
     * already exists" branch and returns the OLD (never-mapped, all-zero)
     * bases with resStatus=1 - without ever calling miscGlideMapMemoryBases,
     * which is the only thing that fills glideSlaveRegBase[].  GET_SLAVE_REGS
     * would then hand back four NULLs.  The shipping Glide gets this right:
     * MINIHWC.C:1543 GETLINEARADDR, :1578 GET_SLAVE_REGS, and ALLOCCONTEXT
     * only later at :2659.  (This is also the exact symptom recorded in
     * retro-agent/tests/native/test_glide_linaddr_guard.c: status 1, bases 0.)
     */
    memset(&rq, 0, sizeof(rq));
    rq.which  = HWCEXT_GETLINEARADDR;
    rq.opt[0] = 0;                                  /* devNum  */
    rq.opt[1] = (u32)GetCurrentProcessId();         /* pHandle */
    /* guard per retro-agent tests/native/test_glide_linaddr_guard.c: the
     * escape can report success with all-zero bases. */
    if (esc(&rq, &rs) <= 0 || rs.resStatus != 1 || rs.opt[1] == 0) {
        fprintf(stderr, "ERR GETLINEARADDR failed (status %d base0 %08Xh)\n",
                rs.resStatus, rs.opt[1]);
        if (rs.resStatus == 1 && rs.opt[1] == 0)
            fprintf(stderr, "     status 1 with a zero base means a stale GLIDESTATE is\n"
                            "     already bound to this PID - re-run in a fresh process.\n");
        return 0;
    }
    printf("BAR0 regs    %08Xh\nBAR1 lfb     %08Xh\nIO base      %08Xh\n",
           rs.opt[1], rs.opt[2], rs.opt[3]);

    for (i = 0; i < g_numChips; i++) {
        memset(&rq, 0, sizeof(rq));
        rq.which  = HWCEXT_GET_SLAVE_REGS;
        rq.opt[0] = i;                              /* DeviceId */
        if (esc(&rq, &rs) <= 0 || rs.resStatus != 1) {
            fprintf(stderr, "ERR GET_SLAVE_REGS(%u) failed (status %d)\n", i, rs.resStatus);
            return 0;
        }
        g_io[i]  = (BYTE *)(UINT_PTR)rs.opt[0];
        g_r3d[i] = (BYTE *)(UINT_PTR)rs.opt[3];
        for (j = 0; j < HWCEXT_MAX_SLAVE_REGS; j++)
            if (rs.opt[j] == 0)
                fprintf(stderr, "WARN chip %u window %u is NULL\n", i, j);
    }
    return 1;
}

static void close_card(void)
{
    hwcReq_t rq; hwcRes_t rs;
    memset(&rq, 0, sizeof(rq)); rq.which = HWCEXT_RELEASECONTEXT;
    esc(&rq, &rs);
    if (g_dc) DeleteDC(g_dc);
}

static void decode_line(const REGDEF *r, u32 v, char *out, int n)
{
    out[0] = 0;
    if (r->off == IO_VIDSCREENSIZE)
        _snprintf(out, n, "%ux%u", v & 0xFFF, (v >> 12) & 0xFFF);
    else if (r->off == IO_VIDPROCCFG)
        _snprintf(out, n, "vpEn=%u desktopEn=%u tiled=%u fmt=%u refOpt28=%u refOpt29=%u",
                  v & 1, (v >> 7) & 1, (v >> 24) & 1, (v >> 18) & 7,
                  (v >> 28) & 1, (v >> 29) & 1);
    else if (r->off == IO_MISCINIT1)
        _snprintf(out, n, "powerdownDac=%u", (v >> 8) & 1);
    else if (r->off == IO_VIDDESKTOPSTART)
        _snprintf(out, n, "hw=%08Xh tiledFlag=%u", v & ~SSTG_IS_TILED, v >> 31);
    else if (r->off == IO_VIDDESKTOPSTRIDE)
        _snprintf(out, n, "desktop=%u overlay=%u", v & 0x3FFF, (v >> 16) & 0x3FFF);
}

static int cmd_dump(int diffmode)
{
    u32 v[MAXCHIP]; int i, k, bad = 0;
    char dec[128];

    if (g_json) printf("{\"numChips\":%u,\"chips\":[", g_numChips);
    for (i = 0; !g_json && i < 1; i++) { }

    if (!g_json) {
        printf("\n%-20s", "register");
        for (k = 0; k < (int)g_numChips; k++) printf("  chip%-8d", k);
        printf("  verdict\n");
    }
    for (i = 0; i < NREGS; i++) {
        int diverge = 0;
        for (k = 0; k < (int)g_numChips; k++) {
            v[k] = g_io[k] ? RD(g_io[k], g_regs[i].off) : 0;
            if (k && v[k] != v[0]) diverge = 1;
        }
        if (!g_json) {
            printf("%-20s", g_regs[i].name);
            for (k = 0; k < (int)g_numChips; k++) printf("  %08X    ", v[k]);
            if (diverge && g_regs[i].mustmatch) { printf("  *** DIVERGES ***  %s", g_regs[i].note); bad++; }
            else if (diverge)                    printf("  differs (informational)");
            else                                 printf("  ok");
            printf("\n");
            decode_line(&g_regs[i], v[0], dec, sizeof(dec));
            if (dec[0]) {
                printf("%-20s  master: %s", "", dec);
                if (diverge) {
                    decode_line(&g_regs[i], v[1], dec, sizeof(dec));
                    printf("   | chip1: %s", dec);
                }
                printf("\n");
            }
        } else {
            printf("%s{\"reg\":\"%s\",\"off\":%u,\"vals\":[", i ? "," : "", g_regs[i].name, g_regs[i].off);
            for (k = 0; k < (int)g_numChips; k++) printf("%s%u", k ? "," : "", v[k]);
            printf("],\"diverges\":%d,\"mustmatch\":%d}", diverge, g_regs[i].mustmatch);
            if (diverge && g_regs[i].mustmatch) bad++;
        }
    }
    if (!g_json) {
        printf("\n%-20s", "vgaRegister[0..11]");
        printf("  (CRTC timing shadows - H3SetMode wrote the DESKTOP mode to slaves)\n");
        for (k = 0; k < (int)g_numChips; k++) {
            int j, d = 0;
            printf("  chip%d ", k);
            for (j = 0; j < 12; j++) {
                u32 a = RD(g_io[k], IO_VGAREGISTER0 + 4u * j);
                u32 b = RD(g_io[0], IO_VGAREGISTER0 + 4u * j);
                if (k && a != b) d = 1;
                printf(" %08X", a);
            }
            printf("%s\n", d ? "   *** DIFFERS FROM MASTER ***" : "");
            if (k && d) bad++;
        }
    } else {
        printf("]}\n");
    }
    if (!g_json) printf("\nmust-match divergences: %d\n", bad);
    return diffmode ? (bad ? 1 : 0) : 0;
}

/* ---------------- phase / frequency ------------------------------------ */

static double qpc_now(LARGE_INTEGER f)
{
    LARGE_INTEGER c; QueryPerformanceCounter(&c);
    return (double)c.QuadPart / (double)f.QuadPart;
}

static int cmd_phase(double secs)
{
    LARGE_INTEGER f;
    double t0, per[MAXCHIP] = {0,0,0,0};
    u32 maxline[MAXCHIP] = {0,0,0,0};
    int k, n = (int)g_numChips;

    QueryPerformanceFrequency(&f);
    if (secs <= 0) secs = 4.0;

    printf("\n-- per-chip CRTC frame period (%.1fs each) ------------------\n", secs);
    for (k = 0; k < n; k++) {
        u32 prev = RD(g_io[k], IO_VIDCURRENTLINE) & 0x7FF, cur, mx = prev;
        int wraps = 0;
        double tfirst = 0, tlast = 0;
        t0 = qpc_now(f);
        while (qpc_now(f) - t0 < secs) {
            cur = RD(g_io[k], IO_VIDCURRENTLINE) & 0x7FF;
            if (cur > mx) mx = cur;
            if (cur + 64 < prev) {                 /* wrap to top of frame */
                double t = qpc_now(f);
                if (!wraps) tfirst = t; else tlast = t;
                wraps++;
            }
            prev = cur;
        }
        maxline[k] = mx;
        per[k] = (wraps > 1) ? (tlast - tfirst) / (double)(wraps - 1) : 0.0;
        if (per[k] > 0)
            printf("chip%d  maxLine=%4u  frames=%d  period=%.9fs  refresh=%.6f Hz\n",
                   k, mx, wraps, per[k], 1.0 / per[k]);
        else
            printf("chip%d  maxLine=%4u  NO WRAPS SEEN - CRTC stopped or vidCurrentLine dead\n", k, mx);
    }
    if (per[0] > 0) {
        printf("\nfrequency deltas vs chip0 (ppm):\n");
        for (k = 1; k < n; k++)
            if (per[k] > 0)
                printf("  chip%d  %+.3f ppm%s\n", k,
                       (per[0] / per[k] - 1.0) * 1e6,
                       (fabs(per[0] / per[k] - 1.0) * 1e6 > 2.0)
                           ? "   *** FREE-RUNNING (not locked to a shared clock) ***"
                           : "   locked");
    }

    /* relative phase: sandwich chip0 reads around each slave read */
    printf("\n-- relative line phase vs chip0 (sandwich, 200 samples) ------\n");
    for (k = 1; k < n; k++) {
        double sum = 0, sum2 = 0; int i, got = 0;
        u32 mx = maxline[0] ? maxline[0] + 1 : 525;
        for (i = 0; i < 200; i++) {
            double ta, tb, tc, l0;
            u32 a, b, c;
            ta = qpc_now(f); a = RD(g_io[0], IO_VIDCURRENTLINE) & 0x7FF;
            tb = qpc_now(f); b = RD(g_io[k], IO_VIDCURRENTLINE) & 0x7FF;
            tc = qpc_now(f); c = RD(g_io[0], IO_VIDCURRENTLINE) & 0x7FF;
            if (c < a) continue;                   /* wrapped mid-sandwich  */
            if (tc - ta <= 0) continue;
            l0 = a + (c - a) * (tb - ta) / (tc - ta);
            {
                double d = (double)b - l0;
                while (d >  (double)mx / 2) d -= mx;
                while (d < -(double)mx / 2) d += mx;
                sum += d; sum2 += d * d; got++;
            }
            Sleep(1);
        }
        if (got > 4) {
            double m = sum / got, sd = sqrt(sum2 / got - m * m);
            printf("chip%d  offset = %+.2f lines (sd %.2f, n=%d)%s\n", k, m, sd, got,
                   (fabs(m) > 1.0) ? "   *** OFFSET ***" : "");
        } else {
            printf("chip%d  insufficient samples\n", k);
        }
    }
    printf("\nNOTE vidCurrentLine is a LINE counter: it cannot see a sub-line\n"
           "     (horizontal/pixel) phase error.  A clean result here means the\n"
           "     chips are frequency-locked and vertically aligned - the skew is\n"
           "     then a HORIZONTAL phase/timing error (vga_vsync_offset, CRTC\n"
           "     shadows, vidScreenSize), which the dump/diff table addresses.\n");
    return 0;
}

/* ---------------- write experiments ------------------------------------ */

static int cmd_poke(int chip, u32 off, u32 val, int holdms)
{
    u32 old;
    if (chip < 0 || chip >= (int)g_numChips) { fprintf(stderr, "ERR bad chip\n"); return 2; }
    old = RD(g_io[chip], off);
    printf("chip%d off %03Xh: %08X -> %08X\n", chip, off, old, val);
    WR(g_io[chip], off, val);
    printf("readback %08X\n", RD(g_io[chip], off));
    if (holdms > 0) {
        printf("holding %d ms then restoring...\n", holdms);
        Sleep(holdms);
        WR(g_io[chip], off, old);
        printf("restored %08X\n", RD(g_io[chip], off));
    } else {
        printf("NOT restored (holdms=0) - value stays until the next mode set\n");
    }
    return 0;
}

/* The candidate-#1 live trial with no rebuild: give every slave the master's
 * vidScreenSize and vidOverlayDudx, then pulse its video processor, which the
 * Napalm spec (11.1.18) requires after any screen-size change and which the
 * DOS reference does (MINIHWC/DOS_MODE.C:645) but W2K's EnableSLIAA does not. */
static int cmd_fixtest(int holdms)
{
    u32 ss, dudx, sc, ec, mrd, vpc, k;
    u32 old_ss[MAXCHIP], old_dudx[MAXCHIP], old_sc[MAXCHIP], old_ec[MAXCHIP], old_mrd[MAXCHIP];

    ss   = RD(g_io[0], IO_VIDSCREENSIZE);
    dudx = RD(g_io[0], IO_VIDOVLDUDX);
    sc   = RD(g_io[0], IO_VIDOVLSTARTCOORD);
    ec   = RD(g_io[0], IO_VIDOVLENDCOORD);
    mrd  = RD(g_io[0], IO_VIDMAXRGBDELTA);
    printf("master vidScreenSize=%08X (%ux%u) vidOverlayDudx=%08X\n",
           ss, ss & 0xFFF, (ss >> 12) & 0xFFF, dudx);

    for (k = 1; k < g_numChips; k++) {
        old_ss[k]   = RD(g_io[k], IO_VIDSCREENSIZE);
        old_dudx[k] = RD(g_io[k], IO_VIDOVLDUDX);
        old_sc[k]   = RD(g_io[k], IO_VIDOVLSTARTCOORD);
        old_ec[k]   = RD(g_io[k], IO_VIDOVLENDCOORD);
        old_mrd[k]  = RD(g_io[k], IO_VIDMAXRGBDELTA);
        printf("chip%u before: scrSize=%08X (%ux%u) dudx=%08X\n", k, old_ss[k],
               old_ss[k] & 0xFFF, (old_ss[k] >> 12) & 0xFFF, old_dudx[k]);
    }
    for (k = 1; k < g_numChips; k++) {
        WR(g_io[k], IO_VIDSCREENSIZE,    ss);
        WR(g_io[k], IO_VIDOVLSTARTCOORD, sc);
        WR(g_io[k], IO_VIDOVLENDCOORD,   ec);
        WR(g_io[k], IO_VIDOVLDUDX,       dudx);
        WR(g_io[k], IO_VIDMAXRGBDELTA,   mrd);
        vpc = RD(g_io[k], IO_VIDPROCCFG);          /* spec-mandated reset */
        WR(g_io[k], IO_VIDPROCCFG, vpc & ~SST_VIDEO_PROCESSOR_EN);
        WR(g_io[k], IO_VIDPROCCFG, vpc |  SST_VIDEO_PROCESSOR_EN);
        printf("chip%u after:  scrSize=%08X dudx=%08X vidProcCfg pulsed\n",
               k, RD(g_io[k], IO_VIDSCREENSIZE), RD(g_io[k], IO_VIDOVLDUDX));
    }
    if (holdms > 0) {
        printf("holding %d ms then restoring...\n", holdms);
        Sleep(holdms);
        for (k = 1; k < g_numChips; k++) {
            WR(g_io[k], IO_VIDSCREENSIZE,    old_ss[k]);
            WR(g_io[k], IO_VIDOVLSTARTCOORD, old_sc[k]);
            WR(g_io[k], IO_VIDOVLENDCOORD,   old_ec[k]);
            WR(g_io[k], IO_VIDOVLDUDX,       old_dudx[k]);
            WR(g_io[k], IO_VIDMAXRGBDELTA,   old_mrd[k]);
            vpc = RD(g_io[k], IO_VIDPROCCFG);
            WR(g_io[k], IO_VIDPROCCFG, vpc & ~SST_VIDEO_PROCESSOR_EN);
            WR(g_io[k], IO_VIDPROCCFG, vpc |  SST_VIDEO_PROCESSOR_EN);
        }
        printf("restored\n");
    }
    return 0;
}

int main(int argc, char **argv)
{
    const char *cmd = (argc > 1) ? argv[1] : "dump";
    int rc = 0, i;

    for (i = 1; i < argc; i++) if (!strcmp(argv[i], "--json")) g_json = 1;
    if (!open_card()) { close_card(); return 3; }

    if      (!strcmp(cmd, "dump"))    rc = cmd_dump(0);
    else if (!strcmp(cmd, "diff"))    rc = cmd_dump(1);
    else if (!strcmp(cmd, "phase"))   rc = cmd_phase(argc > 2 ? atof(argv[2]) : 4.0);
    else if (!strcmp(cmd, "poke") && argc >= 5)
        rc = cmd_poke(atoi(argv[2]), (u32)strtoul(argv[3], 0, 0),
                      (u32)strtoul(argv[4], 0, 0), argc > 5 ? atoi(argv[5]) : 8000);
    else if (!strcmp(cmd, "fixtest")) rc = cmd_fixtest(argc > 2 ? atoi(argv[2]) : 0);
    else { fprintf(stderr, "usage: fxscan2 dump|diff|phase [s]|poke <chip> <off> <val> [ms]|fixtest [ms]\n"); rc = 2; }

    close_card();
    return rc;
}
