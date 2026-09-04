/* fxpci.c - per-chip PCI CONFIG-SPACE reader/writer for the four VSA-100
 * functions of a Voodoo5 6000, from user mode on 32-bit Windows XP.
 * NO DRIVER of any kind is installed.
 *
 * Why config space: the registers that set inter-chip video timing are NOT
 * in the MMIO register block.  They are 3dfx-private PCI config registers
 * (Miniport SLIAA.H:273-279):
 *   0x80 cfgVideoCtrl0   bit11 CFG_VIDPLL_SEL  ("sync slave chip to Master's
 *                        clock"); bits14:12 divide_video; bits19:16 vsync_ref
 *   0x84 cfgVideoCtrl1 / 0x88 cfgVideoCtrl2   SLI render/compare masks
 *   0x8C cfgSliLfbCtrl   bit28 CFG_SLI_RD_EN
 *   0xAC cfgSliAaMisc    bits8:0 vga_vsync_offset = pixels[2:0] chars[5:3]
 *                        hxtra[8:6]  <- the ONLY horizontal alignment knob
 *
 * Access: mechanism #1 (0xCF8/0xCFC).  Port I/O from user mode is enabled
 * with NtSetInformationProcess(ProcessUserModeIOPL=34), which needs
 * SeTcbPrivilege - held by LocalSystem, which is what the retro agent's
 * EXEC children run as (agent/src/service.c:497-506 passes a NULL
 * lpServiceStartName).  If the agent is running interactively instead, this
 * tool reports NOT_PRIVILEGED and exits without touching anything.
 *
 *   fxpci scan                     enumerate 121A:0009 functions
 *   fxpci dump                     dump 0x00-0xFF of every function + decode
 *   fxpci get  <fn> <off>
 *   fxpci set  <fn> <off> <val> [restore_ms]
 *   fxpci vidpll <fn> <0|1> [restore_ms]     set/clear CFG_VIDPLL_SEL
 *   fxpci vsyncoff <fn> <pix> <chr> <hxtra> [restore_ms]
 *
 * Build: i686-w64-mingw32-gcc -O2 -Wall -o fxpci.exe fxpci.c
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;
typedef LONG NTSTATUS;

#define PCI_ADDR_PORT 0xCF8
#define PCI_DATA_PORT 0xCFC
#define VSA_VEN 0x121A
#define VSA_DEV 0x0009
#define HINT_ID 0x00213388u          /* HiNT HB1-SE66, SLIAA.C V56K_HINT_BRIDGE_ID */

#define CFG_VIDEO_CTRL0  0x80
#define CFG_VIDEO_CTRL1  0x84
#define CFG_VIDEO_CTRL2  0x88
#define CFG_SLI_LFB_CTRL 0x8C
#define CFG_AA_LFB_CTRL  0x94
#define CFG_SLI_AA_MISC  0xAC
#define CFG_VIDPLL_SEL   (1u << 11)

static void outl_(u32 port, u32 val)
{ __asm__ __volatile__("outl %0, %w1" : : "a"(val), "Nd"((unsigned short)port)); }
static u32 inl_(u32 port)
{ u32 v; __asm__ __volatile__("inl %w1, %0" : "=a"(v) : "Nd"((unsigned short)port)); return v; }

static u32 cfg_rd(int bus, int dev, int fn, int off)
{
    outl_(PCI_ADDR_PORT, 0x80000000u | (bus << 16) | (dev << 11) | (fn << 8) | (off & 0xFC));
    return inl_(PCI_DATA_PORT);
}
static void cfg_wr(int bus, int dev, int fn, int off, u32 v)
{
    outl_(PCI_ADDR_PORT, 0x80000000u | (bus << 16) | (dev << 11) | (fn << 8) | (off & 0xFC));
    outl_(PCI_DATA_PORT, v);
}

static int enable_iopl(void)
{
    HANDLE tok; TOKEN_PRIVILEGES tp; LUID luid;
    NTSTATUS (WINAPI *pNtSetInformationProcess)(HANDLE, ULONG, PVOID, ULONG);
    HMODULE nt = GetModuleHandleA("ntdll.dll");
    NTSTATUS st;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tok)) {
        fprintf(stderr, "ERR OpenProcessToken %lu\n", GetLastError()); return 0;
    }
    if (!LookupPrivilegeValueA(NULL, "SeTcbPrivilege", &luid)) {
        fprintf(stderr, "ERR LookupPrivilegeValue %lu\n", GetLastError()); return 0;
    }
    tp.PrivilegeCount = 1; tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(tok, FALSE, &tp, sizeof(tp), NULL, NULL);
    if (GetLastError() != ERROR_SUCCESS) {
        fprintf(stderr, "NOT_PRIVILEGED: SeTcbPrivilege not held (run via the agent,\n"
                        "                which executes children as LocalSystem)\n");
        return 0;
    }
    pNtSetInformationProcess = (NTSTATUS (WINAPI *)(HANDLE, ULONG, PVOID, ULONG))
        GetProcAddress(nt, "NtSetInformationProcess");
    if (!pNtSetInformationProcess) { fprintf(stderr, "ERR no NtSetInformationProcess\n"); return 0; }
    /* 34 = ProcessUserModeIOPL; takes no data on 32-bit NT */
    st = pNtSetInformationProcess(GetCurrentProcess(), 34, NULL, 0);
    if (st != 0) { fprintf(stderr, "ERR ProcessUserModeIOPL status %08lX\n", (unsigned long)st); return 0; }
    Sleep(1);                                     /* let the IOPL take effect */
    return 1;
}

static int g_bus[8], g_dev[8], g_fn[8], g_n;

static void scan(int verbose)
{
    int bus, dev, fn;
    g_n = 0;
    for (bus = 0; bus < 32 && g_n < 8; bus++)
      for (dev = 0; dev < 32; dev++)
        for (fn = 0; fn < 8; fn++) {
            u32 id = cfg_rd(bus, dev, fn, 0x00);
            if (id == 0xFFFFFFFFu || id == 0) { if (!fn) break; else continue; }
            if (verbose && id == HINT_ID)
                printf("HiNT bridge  %02x:%02x.%x  id=%08X  secondaryBus=%u\n",
                       bus, dev, fn, id, (cfg_rd(bus, dev, fn, 0x18) >> 8) & 0xFF);
            if ((id & 0xFFFF) == VSA_VEN && ((id >> 16) & 0xFFFF) == VSA_DEV) {
                if (g_n < 8) { g_bus[g_n] = bus; g_dev[g_n] = dev; g_fn[g_n] = fn; g_n++; }
                if (verbose)
                    printf("VSA-100 #%d   %02x:%02x.%x  id=%08X  rev=%02X\n",
                           g_n - 1, bus, dev, fn, id, cfg_rd(bus, dev, fn, 0x08) & 0xFF);
            }
        }
    if (verbose) printf("found %d VSA-100 function(s)\n", g_n);
}

static void decode(int i)
{
    u32 c0 = cfg_rd(g_bus[i], g_dev[i], g_fn[i], CFG_VIDEO_CTRL0);
    u32 c1 = cfg_rd(g_bus[i], g_dev[i], g_fn[i], CFG_VIDEO_CTRL1);
    u32 c2 = cfg_rd(g_bus[i], g_dev[i], g_fn[i], CFG_VIDEO_CTRL2);
    u32 sl = cfg_rd(g_bus[i], g_dev[i], g_fn[i], CFG_SLI_LFB_CTRL);
    u32 am = cfg_rd(g_bus[i], g_dev[i], g_fn[i], CFG_SLI_AA_MISC);
    printf("chip%d  ctrl0=%08X  vidpll_sel=%u  divide_video=%u  vsync_ref_del=%u\n",
           i, c0, (c0 >> 11) & 1, (c0 >> 12) & 7, (c0 >> 16) & 0xF);
    printf("       enhVidEn=%u enhVidSlv=%u dacVsyncTri=%u dacHsyncTri=%u\n",
           c0 & 1, (c0 >> 1) & 1, (c0 >> 24) & 1, (c0 >> 25) & 1);
    printf("       ctrl1=%08X  ctrl2=%08X  rendermask_crt=%02X comparemask_crt=%02X\n",
           c1, c2, (c2 >> 16) & 0xFF, (c2 >> 24) & 0xFF);
    printf("       sliLfbCtrl=%08X  sli_rd_en=%u\n", sl, (sl >> 28) & 1);
    printf("       sliAaMisc=%08X  vga_vsync_offset: pixels=%u chars=%u hxtra=%u"
           "  (=%u pixel-equivalents)\n",
           am, am & 7, (am >> 3) & 7, (am >> 6) & 7,
           (am & 7) + 8 * ((am >> 3) & 7));
}

int main(int argc, char **argv)
{
    const char *cmd = (argc > 1) ? argv[1] : "dump";
    int i, j;

    if (!enable_iopl()) return 3;
    scan(!strcmp(cmd, "scan") || !strcmp(cmd, "dump"));
    if (!g_n) { fprintf(stderr, "ERR no VSA-100 found\n"); return 4; }

    if (!strcmp(cmd, "scan")) return 0;

    if (!strcmp(cmd, "dump")) {
        for (i = 0; i < g_n; i++) {
            printf("\n-- chip%d  %02x:%02x.%x  raw config --------------------------\n",
                   i, g_bus[i], g_dev[i], g_fn[i]);
            for (j = 0; j < 256; j += 16) {
                int k; printf("%02X:", j);
                for (k = 0; k < 16; k += 4) printf(" %08X", cfg_rd(g_bus[i], g_dev[i], g_fn[i], j + k));
                printf("\n");
            }
        }
        printf("\n-- decoded SLI/video control ---------------------------------\n");
        for (i = 0; i < g_n; i++) decode(i);
        printf("\nVERDICT TEST (the missing Win9x branch, MINIVDD/SLIAA.C:1690-1694):\n"
               "  if vidpll_sel reads 1 on chips 1..3 and 0 on chip 0, the W2K\n"
               "  miniport's dropped 4-chip master branch is CONFIRMED on hardware.\n");
        return 0;
    }
    if (!strcmp(cmd, "get") && argc >= 4) {
        i = atoi(argv[2]);
        if (i < 0 || i >= g_n) return 2;
        printf("%08X\n", cfg_rd(g_bus[i], g_dev[i], g_fn[i], (int)strtoul(argv[3], 0, 0)));
        return 0;
    }
    if ((!strcmp(cmd, "set") || !strcmp(cmd, "vidpll") || !strcmp(cmd, "vsyncoff")) && argc >= 4) {
        int off, ms = 0; u32 nv, ov;
        i = atoi(argv[2]);
        if (i < 0 || i >= g_n) return 2;
        if (!strcmp(cmd, "set")) {
            off = (int)strtoul(argv[3], 0, 0);
            nv  = (u32)strtoul(argv[4], 0, 0);
            ms  = (argc > 5) ? atoi(argv[5]) : 0;
            ov  = cfg_rd(g_bus[i], g_dev[i], g_fn[i], off);
        } else if (!strcmp(cmd, "vidpll")) {
            off = CFG_VIDEO_CTRL0;
            ov  = cfg_rd(g_bus[i], g_dev[i], g_fn[i], off);
            nv  = atoi(argv[3]) ? (ov | CFG_VIDPLL_SEL) : (ov & ~CFG_VIDPLL_SEL);
            ms  = (argc > 4) ? atoi(argv[4]) : 0;
        } else {
            u32 p = (u32)atoi(argv[3]) & 7, c = (u32)atoi(argv[4]) & 7, h = (u32)atoi(argv[5]) & 7;
            off = CFG_SLI_AA_MISC;
            ov  = cfg_rd(g_bus[i], g_dev[i], g_fn[i], off);
            nv  = (ov & ~0x1FFu) | p | (c << 3) | (h << 6);
            ms  = (argc > 6) ? atoi(argv[6]) : 0;
        }
        printf("chip%d off %02X: %08X -> %08X\n", i, off, ov, nv);
        cfg_wr(g_bus[i], g_dev[i], g_fn[i], off, nv);
        printf("readback %08X\n", cfg_rd(g_bus[i], g_dev[i], g_fn[i], off));
        if (ms > 0) {
            printf("holding %d ms...\n", ms); Sleep(ms);
            cfg_wr(g_bus[i], g_dev[i], g_fn[i], off, ov);
            printf("restored %08X\n", cfg_rd(g_bus[i], g_dev[i], g_fn[i], off));
        }
        return 0;
    }
    fprintf(stderr, "usage: fxpci scan|dump|get <fn> <off>|set <fn> <off> <val> [ms]|"
                    "vidpll <fn> <0|1> [ms]|vsyncoff <fn> <pix> <chr> <hxtra> [ms]\n");
    return 2;
}
