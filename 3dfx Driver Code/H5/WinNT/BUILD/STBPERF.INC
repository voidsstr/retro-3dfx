// stbperf.inc
// STB optimizations for Voodoo3\BlackMagic 6X00


#define PERF_COPY_BITS_OPT	1	// handle trivial 8bpp to 32bpp, 8bpp to 16bpp, and 8bpp to 24bpp copybits instead of punting
#define PERF_ASM_XLATE		1	// speed up the 8 to 16 and 8 to 32 translations

#if (_WIN32_WINNT < 0x0500)
#define PERF_DISABLE_CANBLTSYSMEM 1	// D3D HEL hangs when system to video memory blts are enabled
#endif


// end perf.inc
