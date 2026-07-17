/*
 * banshee.h
 *
 */
#define TO_MB(x)  (x * 1024 * 1024)

enum blit2d_enum {
   nop = 0,
   screen_to_screen = 1,
   screen_to_screen_stretch = 2,
   host_to_screen = 3,
   host_to_screen_stretch = 4,
   rectangle_fill = 5,
   line = 6,
   polyline = 7,
   polygon_fill = 8
};

struct location_struct {
   int x,y;
};

struct blit2d_params_struct {
  struct location_struct dst;
  struct location_struct src;
  struct location_struct dst_size;
  struct location_struct src_size;
  FxU32 colorFore;
};

extern struct blit2d_params_struct current_2dblit;

int init_banshee(LPCARDINFO card);
int init_banshee_modules();
FxU32 bansheeFbiSize(LPCARDINFO);
FxU32 bansheeMemClock(LPCARDINFO);
void bansheeFbiTest(LPCARDINFO card,void (*cbfn)(int frame));
FxBool bansheeSliFbiTest(LPCARDINFO card);
FxBool bansheePciTest(LPCARDINFO card, FxU32 testcode, FxU32 SSTRegister, FxU32 mask);
FxBool bansheeAgpTest(LPCARDINFO card, FxU32 testcode);
FxU32 bansheeCrc(LPCARDINFO);
FxU32 bansheeMemCrc(LPCARDINFO);
FxU32 bansheeSliMemCrc(LPCARDINFO);
void b2d_setlocation(struct location_struct *l, int x, int y);
void b2d_setdstsize(int w, int h);
void b2d_setsrcsize(int w, int h);
void b2d_setdest(int x, int y);
void b2d_setsrc(int x, int y);
void b2d_setcolor(FxU32 color);

void bansheeTestVESAVideo(LPCARDINFO card, void (*cbfn)(int), char *v_parm, int testmode);
FxBool bansheeTestVGACore(LPCARDINFO card);
FxU32 bansheeHWCursor(LPCARDINFO card, int en,int xLoc, int yLoc);
FxU32 bansheeDesktopStartCheck(LPCARDINFO card, FxU32 StartAddr);
void bansheeWaitForVsync(LPCARDINFO card, int state);

FxBool bansheeChecksumRom(LPCARDINFO card, int mb_bios_cksm, char *version_str, int check_cksm, int *calc_mb_chksm);
FxBool bansheeCheckVPD(LPCARDINFO card, char *assembly_number, int assbly_size, char *serial_number, char *eco_level, int suppress_msg);
FxBool bansheeCheckSerialRom(LPCARDINFO card);

void bansheeStartGlide(LPCARDINFO card);
FxBool bansheeTestVGACore(LPCARDINFO card);
void bansheeTestVideo(LPCARDINFO card, void (*cbfn)(int),int mode);
FxU32 banshee2dBlit(LPCARDINFO card, enum blit2d_enum type);
FxU32 GetMemSize(FxU32 dramInit0, FxU32 dramInit1);
FxU32 MemoryPatternTests(FxU32 fbiMemory,FxU32 memSize, void (*cbfn)(int frame));
