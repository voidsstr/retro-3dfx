/* sligrid.c - fullscreen Glide TEST-PATTERN generator + in-process scanout probe
 * for the Voodoo 5 6000 SLI skew.  NO DRIVER REBUILD.
 *
 *  build:
 *    i686-w64-mingw32-gcc -O2 -Wall -o sligrid.exe sligrid.c \
 *      -I<retro-agent>/scripts/3dfx/glide-sdk/include \
 *      <retro-agent>/scripts/3dfx/glide-sdk/lib/libglide3x_retail.dll.a \
 *      -lgdi32 -luser32
 *
 *  WHY THIS AND NOT QUAKE III: the pattern must be bit-exact and static, and
 *  -- decisively -- HWCEXT_PCI_OP is gated on the caller holding
 *  HWC_EXCLUSIVE (HWCEXT.C:2409).  Only the process that owns the card in
 *  fullscreen Glide holds it.  So the probe has to live INSIDE the Glide app.
 *
 *  usage:
 *    sligrid.exe --mode 640x480 --hold 45 [--readback C:\rb.bmp] [--regs] [--pci]
 *    sligrid.exe --emit C:\ref.bin --mode 640x480      (no card touched)
 *    sligrid.exe --mode 640x480 --hold 20 --poke 0:AC=0x000   (live PCI write)
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glide.h>
#include "pattern_gen.h"

/* ---------------- HWCEXT ABI (H5/W2K/.../Displays/H5/HWCEXT.H) ------------ */
#define EXT_HWC 0x3df3
#define EXT_HWC_OLD 0x0fd3
#define HWCEXT_ALLOCCONTEXT    0x01
#define HWCEXT_GETDEVICECONFIG 0x02
#define HWCEXT_RELEASECONTEXT  0x07
#define HWCEXT_PCI_OP          0x18
#define HWCEXT_PCI_READ        0x00
#define HWCEXT_PCI_WRITE       0x01
#define HWCEXT_GET_SLAVE_REGS  0x19
#define HWCEXT_MAX_SLAVE_REGS  4
#define HWCEXT_PROTOCOLREV     0x1
typedef unsigned long U32;
typedef struct { U32 contextID, which; union {
    struct { U32 protocolRev, appType; } alloc;
    struct { void *dc; U32 devNo; }      devcfg;
    struct { U32 devNum, pHandle; }      linaddr;
    struct { U32 DeviceId, Operation, Offset, Value; } pciOp;   /* HWCEXT.H:554 */
    struct { U32 DeviceId; }             slaveReg;              /* HWCEXT.H:564 */
    U32 pad[16]; } o; } req_t;
typedef struct { long resStatus; union {
    struct { U32 contextID; } alloc;
    struct { U32 devNum, vendorID, deviceID, fbRam, chipRev,
                 pciStride, hwStride, tileMark, isMaster, numChips; } devcfg;
    struct { U32 Value; }  pciOp;
    struct { U32 Regs[HWCEXT_MAX_SLAVE_REGS]; } slaveReg;
    U32 pad[16]; } o; } res_t;

/* SstIORegs byte offsets (H3REGS.H / Miniport H3.H:1120-1175) */
#define IO_LFBMEMORYCONFIG 0x00C
#define IO_MISCINIT0       0x010
#define IO_MISCINIT1       0x014
#define IO_VGAINIT0        0x028
#define IO_DACMODE         0x04C
#define IO_VIDPROCCFG      0x05C
#define IO_VIDCURRENTLINE  0x094
#define IO_VIDSCREENSIZE   0x098
#define IO_VIDOVLDUDX      0x0A4
#define IO_VGAREGISTER0    0x0B0        /* MMIO alias of VGA I/O 0x3B0..0x3DF */
#define IO_VIDDESKTOPSTART 0x0E4
#define IO_VIDDESKTOPSTRIDE 0x0E8
#define VGA_CRTC_INDEX (IO_VGAREGISTER0 + 0x24)   /* legacy 0x3D4 */
#define VGA_CRTC_DATA  (IO_VGAREGISTER0 + 0x25)   /* legacy 0x3D5 */
/* PCI config offsets (Miniport SLIAA.H:273-279) */
#define CFG_VIDEO_CTRL0 0x80
#define CFG_VIDEO_CTRL1 0x84
#define CFG_VIDEO_CTRL2 0x88
#define CFG_SLI_LFB_CTRL 0x8C
#define CFG_AA_LFB_CTRL  0x94
#define CFG_SLI_AA_MISC  0xAC
#define CFG_VIDPLL_SEL   (1UL << 11)

static HDC   g_dc;  static int g_esc = EXT_HWC;
static U32   g_slave[4][HWCEXT_MAX_SLAVE_REGS];
static U32   g_chips = 1;

static int esc(req_t *q, res_t *r)
{ memset(r, 0, sizeof(*r));
  return ExtEscape(g_dc, g_esc, sizeof(*q), (LPCSTR)q, sizeof(*r), (LPSTR)r); }
static U32 RD(volatile unsigned char *b, U32 o){ return *(volatile U32 *)(b + o); }
static unsigned char RDB(volatile unsigned char *b, U32 o){ return *(b + o); }
static void WRB(volatile unsigned char *b, U32 o, unsigned char v){ *(b + o) = v; }

static int hwc_open(void)
{
    req_t q; res_t r;
    if (!(g_dc = CreateDCA("DISPLAY", NULL, NULL, NULL))) return 0;
    memset(&q,0,sizeof q); q.which = HWCEXT_GETDEVICECONFIG;
    if (esc(&q,&r) <= 0) { g_esc = EXT_HWC_OLD; if (esc(&q,&r) <= 0) return 0; }
    g_chips = r.o.devcfg.numChips ? r.o.devcfg.numChips : 1;
    printf("device   %04lX:%04lX rev %lu  fbRam %luMB  chips %lu\n"
           "tiling   pitch %lu  stride %lu  mark %08lXh\n",
           r.o.devcfg.vendorID, r.o.devcfg.deviceID, r.o.devcfg.chipRev,
           r.o.devcfg.fbRam >> 20, g_chips,
           r.o.devcfg.pciStride, r.o.devcfg.hwStride, r.o.devcfg.tileMark);
    memset(&q,0,sizeof q); q.which = HWCEXT_ALLOCCONTEXT;
    q.o.alloc.protocolRev = HWCEXT_PROTOCOLREV;
    if (esc(&q,&r) <= 0 || r.resStatus != 1) { printf("ALLOCCONTEXT failed\n"); return 0; }
    { U32 i, j; for (i = 0; i < g_chips; i++) {
        memset(&q,0,sizeof q); q.which = HWCEXT_GET_SLAVE_REGS; q.o.slaveReg.DeviceId = i;
        if (esc(&q,&r) <= 0 || r.resStatus != 1) { printf("GET_SLAVE_REGS(%lu) failed\n", i); return 0; }
        for (j = 0; j < HWCEXT_MAX_SLAVE_REGS; j++) g_slave[i][j] = r.o.slaveReg.Regs[j]; } }
    return 1;
}
/* NOTE: do NOT issue HWCEXT_RELEASECONTEXT here.  Glide already holds a
 * GLIDESTATE for THIS pid (it is the same per-process state that carries
 * GLDATA_GDIFLAGS_HWC_EXCLUSIVE, which is what unlocks PCI_OP), and
 * hwcReleaseContext unmaps the ranges when the refcount hits 1
 * (HWCEXT.C:1348-1403).  Releasing from under a live Glide context would
 * pull the framebuffer mapping out from under the running app.  The driver
 * cleans the state up at process exit. */
static void hwc_close(void) { DeleteDC(g_dc); }

static int pci_rd(U32 dev, U32 off, U32 *v)
{ req_t q; res_t r; memset(&q,0,sizeof q); q.which = HWCEXT_PCI_OP;
  q.o.pciOp.DeviceId = dev; q.o.pciOp.Operation = HWCEXT_PCI_READ; q.o.pciOp.Offset = off;
  if (esc(&q,&r) <= 0) return 0;
  *v = r.o.pciOp.Value; return 1; }
static int pci_wr(U32 dev, U32 off, U32 v)
{ req_t q; res_t r; memset(&q,0,sizeof q); q.which = HWCEXT_PCI_OP;
  q.o.pciOp.DeviceId = dev; q.o.pciOp.Operation = HWCEXT_PCI_WRITE;
  q.o.pciOp.Offset = off; q.o.pciOp.Value = v; return esc(&q,&r) > 0; }

/* ------- per-chip scanout dump: this is the whole point of the exercise ---- */
static void dump_regs(void)
{
    U32 i, k;
    printf("\nchip vidProcCfg vidScrSize dtStart  dtStride lfbMemCfg miscInit0 miscInit1 dacMode  curLine ovlDudx\n");
    for (i = 0; i < g_chips; i++) {
        volatile unsigned char *io = (volatile unsigned char *)(UINT_PTR)g_slave[i][0];
        if (!io) { printf("%3lu  <unmapped>\n", i); continue; }
        printf("%3lu  %08lX  %08lX  %08lX %08lX %08lX  %08lX  %08lX  %08lX %5lu  %08lX\n", i,
            RD(io,IO_VIDPROCCFG), RD(io,IO_VIDSCREENSIZE), RD(io,IO_VIDDESKTOPSTART),
            RD(io,IO_VIDDESKTOPSTRIDE), RD(io,IO_LFBMEMORYCONFIG), RD(io,IO_MISCINIT0),
            RD(io,IO_MISCINIT1), RD(io,IO_DACMODE), RD(io,IO_VIDCURRENTLINE)&0x7FF,
            RD(io,IO_VIDOVLDUDX));
    }
    /* VGA CRTC through each chip's own MMIO alias -> horizontal timing in PIXELS */
    printf("\nchip  hTotal hDispEnd hBlankSt hRetrSt hRetrEnd  vTotal vDispEnd  (CRTC 0x00-0x18)\n");
    for (i = 0; i < g_chips; i++) {
        volatile unsigned char *io = (volatile unsigned char *)(UINT_PTR)g_slave[i][0];
        unsigned char c[0x19];
        if (!io) continue;
        for (k = 0; k <= 0x18; k++) { WRB(io, VGA_CRTC_INDEX, (unsigned char)k);
                                      c[k] = RDB(io, VGA_CRTC_DATA); }
        printf("%3lu  %6d %8d %8d %7d %8d  %6d %8d   ", i,
               (c[0]+5)*8, (c[1]+1)*8, c[2]*8, c[4]*8, (c[5]&0x1F)*8,
               c[6] | ((c[7]&1)<<8) | ((c[7]&0x20)<<4),
               c[0x12] | ((c[7]&2)<<7) | ((c[7]&0x40)<<3));
        for (k = 0; k <= 0x18; k++) printf("%02X", c[k]);
        printf("\n");
    }
}
static void dump_pci(void)
{
    U32 i, v = 0, misc = 0, ctl0 = 0;
    printf("\nchip cfgVideoCtrl0 [vidpll_sel] cfgVideoCtrl1 cfgVideoCtrl2 cfgSliLfbCtrl cfgSliAaMisc [px chr hx = px]\n");
    for (i = 0; i < g_chips; i++) {
        if (!pci_rd(i, CFG_VIDEO_CTRL0, &ctl0)) { printf("%3lu  PCI_OP refused (need HWC_EXCLUSIVE)\n", i); return; }
        printf("%3lu  %08lX   %s  ", i, ctl0, (ctl0 & CFG_VIDPLL_SEL) ? "LOCKED " : "FREERUN");
        pci_rd(i, CFG_VIDEO_CTRL1, &v); printf("%08lX      ", v);
        pci_rd(i, CFG_VIDEO_CTRL2, &v); printf("%08lX      ", v);
        pci_rd(i, CFG_SLI_LFB_CTRL, &v); printf("%08lX      ", v);
        pci_rd(i, CFG_SLI_AA_MISC, &misc);
        printf("%08lX   [%lu %lu %lu = %lu px]\n", misc, misc & 7, (misc >> 3) & 7,
               (misc >> 6) & 7, (misc & 7) + 8 * ((misc >> 3) & 7));
    }
}
/* cross-chip CRTC phase-lock test: are the four line counters frequency locked? */
static void phase_test(int nsamp)
{
    LARGE_INTEGER f, t0, t; U32 i, k; int lo[4], hi[4], first[4], last[4];
    volatile unsigned char *io[4];
    for (i = 0; i < g_chips; i++) io[i] = (volatile unsigned char *)(UINT_PTR)g_slave[i][0];
    QueryPerformanceFrequency(&f); QueryPerformanceCounter(&t0);
    for (i = 0; i < g_chips; i++) { lo[i] = 1 << 20; hi[i] = -(1 << 20); }
    for (k = 0; k < (U32)nsamp; k++) {
        int base = (int)(RD(io[0], IO_VIDCURRENTLINE) & 0x7FF);
        for (i = 1; i < g_chips; i++) {
            int d = (int)(RD(io[i], IO_VIDCURRENTLINE) & 0x7FF) - base;
            if (k == 0) first[i] = d;
            last[i] = d;
            if (d < lo[i]) lo[i] = d;
            if (d > hi[i]) hi[i] = d;
        }
    }
    QueryPerformanceCounter(&t);
    printf("\nphase-lock: %d samples over %.2f s\n", nsamp,
           (double)(t.QuadPart - t0.QuadPart) / (double)f.QuadPart);
    for (i = 1; i < g_chips; i++)
        printf("  chip%lu - chip0 line delta: min %d max %d spread %d  first %d last %d  -> %s\n",
               i, lo[i], hi[i], hi[i] - lo[i], first[i], last[i],
               (hi[i] - lo[i]) <= 4 ? "FREQUENCY LOCKED" : "*** NOT LOCKED (free-running PLL) ***");
}

/* ------------------------------------------------------------------ output */
static int write_bmp565(const char *p, const unsigned short *px, int W, int H)
{
    FILE *f = fopen(p, "wb"); int x, y, rb = ((W*3)+3)&~3; unsigned char h[54], *row;
    unsigned long img = (unsigned long)rb*H, fs = 54+img;
    if (!f) return 0;
    memset(h,0,54); h[0]='B'; h[1]='M'; *(unsigned long*)(h+2)=fs; h[10]=54; h[14]=40;
    *(long*)(h+18)=W; *(long*)(h+22)=H; h[26]=1; h[28]=24; *(unsigned long*)(h+34)=img;
    fwrite(h,1,54,f); row = (unsigned char*)malloc(rb);
    for (y = H-1; y >= 0; y--) { const unsigned short *s = px + (size_t)y*W;
        memset(row,0,rb);
        for (x = 0; x < W; x++) { unsigned q=s[x], r5=(q>>11)&31, g6=(q>>5)&63, b5=q&31;
            row[x*3+0]=(unsigned char)((b5<<3)|(b5>>2));
            row[x*3+1]=(unsigned char)((g6<<2)|(g6>>4));
            row[x*3+2]=(unsigned char)((r5<<3)|(r5>>2)); }
        fwrite(row,1,rb,f); }
    free(row); fclose(f); return 1;
}
static DWORD WINAPI watchdog(LPVOID p){ Sleep((DWORD)(UINT_PTR)p); ExitProcess(9); return 0; }

int main(int argc, char **argv)
{
    int W = 640, H = 480, hold = 30, i, res = GR_RESOLUTION_640x480;
    const char *readback = NULL, *emit = NULL;
    int do_regs = 0, do_pci = 0, nphase = 0;
    U32 pdev[8], poff[8], pval[8]; int npoke = 0;
    unsigned short *pat, *back;
    GrContext_t ctx; HWND hw;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--mode") && i+1 < argc) sscanf(argv[++i], "%dx%d", &W, &H);
        else if (!strcmp(argv[i], "--hold") && i+1 < argc) hold = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--readback") && i+1 < argc) readback = argv[++i];
        else if (!strcmp(argv[i], "--emit") && i+1 < argc) emit = argv[++i];
        else if (!strcmp(argv[i], "--regs")) do_regs = 1;
        else if (!strcmp(argv[i], "--pci")) do_pci = 1;
        else if (!strcmp(argv[i], "--phase") && i+1 < argc) nphase = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--poke") && i+1 < argc) {
            if (npoke < 8 && sscanf(argv[++i], "%lu:%lx=%lx",
                                    &pdev[npoke], &poff[npoke], &pval[npoke]) == 3)
                npoke++;
        }
    }
    if      (W == 640  && H == 480) res = GR_RESOLUTION_640x480;
    else if (W == 800  && H == 600) res = GR_RESOLUTION_800x600;
    else if (W == 1024 && H == 768) res = GR_RESOLUTION_1024x768;
    else { printf("unsupported mode\n"); return 2; }

    pat = (unsigned short *)malloc((size_t)W*H*2);
    sligrid_pattern(pat, W, H);
    if (emit) { FILE *f = fopen(emit, "wb"); fwrite(pat, 2, (size_t)W*H, f); fclose(f);
                printf("emitted %s (%dx%d)\n", emit, W, H); return 0; }

    CreateThread(NULL, 0, watchdog, (LPVOID)(UINT_PTR)((hold + 20) * 1000u), 0, NULL);
    hw = CreateWindowExA(WS_EX_TOPMOST, "STATIC", "sligrid", WS_POPUP | WS_VISIBLE,
                         0, 0, W, H, NULL, NULL, GetModuleHandle(NULL), NULL);

    grGlideInit();
    grSstSelect(0);
    ctx = grSstWinOpen((FxU32)(UINT_PTR)hw, res, GR_REFRESH_60Hz, GR_COLORFORMAT_ABGR,
                       GR_ORIGIN_UPPER_LEFT, 2, 1);
    if (!ctx) { printf("grSstWinOpen failed\n"); grGlideShutdown(); return 3; }
    printf("glide    %s\n", grGetString(GR_RENDERER) ? (char*)grGetString(GR_RENDERER) : "?");

    /* put the pattern in BOTH colour buffers so nothing the driver does can
       show a stale frame, then stop swapping - a static scanout is required. */
    for (i = 0; i < 2; i++) {
        grLfbWriteRegion(GR_BUFFER_BACKBUFFER, 0, 0, GR_LFB_SRC_FMT_565,
                         (FxU32)W, (FxU32)H, FXFALSE, (FxI32)(W*2), pat);
        grBufferSwap(1);
    }
    grLfbWriteRegion(GR_BUFFER_FRONTBUFFER, 0, 0, GR_LFB_SRC_FMT_565,
                     (FxU32)W, (FxU32)H, FXFALSE, (FxI32)(W*2), pat);
    grFinish();

    /* everything below runs while WE own HWC_EXCLUSIVE - that is what unlocks
       HWCEXT_PCI_OP (HWCEXT.C:2409). */
    if (hwc_open()) {
        if (do_regs) dump_regs();
        if (do_pci)  dump_pci();
        if (nphase)  phase_test(nphase);
        { int q; for (q = 0; q < npoke; q++) { U32 old = 0, rb = 0;
            pci_rd(pdev[q], poff[q], &old);
            pci_wr(pdev[q], poff[q], pval[q]);
            pci_rd(pdev[q], poff[q], &rb);
            printf("\npoke chip%lu cfg[0x%02lX] %08lX -> %08lX (readback %08lX)\n",
                   pdev[q], poff[q], old, pval[q], rb);
            fflush(stdout); } }
        if (readback) {
            back = (unsigned short *)malloc((size_t)W*H*2);
            grLfbReadRegion(GR_BUFFER_FRONTBUFFER, 0, 0, (FxU32)W, (FxU32)H,
                            (FxU32)(W*2), back);
            { size_t n = (size_t)W*H, bad = 0, k;
              for (k = 0; k < n; k++) if (back[k] != pat[k]) bad++;
              printf("\nreadback: %lu of %lu pixels differ (%.4f%%)\n",
                     (unsigned long)bad, (unsigned long)n, 100.0*bad/n); }
            write_bmp565(readback, back, W, H);
            printf("wrote %s\n", readback);
            free(back);
        }
        hwc_close();
    } else printf("HWCEXT probe unavailable\n");

    fflush(stdout);
    Sleep((DWORD)hold * 1000u);
    grSstWinClose(ctx);
    grGlideShutdown();
    return 0;
}
