#include "GraphicsPriv.h"

#define SETREFRESH(_x) ((FxU32)(_x * 65536))
#define GETREFRESH(_x) (_x >> 16)

enum
{
  valid = (1 << kModeValid),
  validAndSafe = (1 << kModeValid | 1 << kModeSafe),
  validAndSafeAndDefault = ( (1 << kModeValid) | (1 << kModeSafe) | (1 << kModeDefault) )
};

DisplayModeIDData displayModeIDMap[kMaxDisplayModeIDs] =
{
  {kDisplay640x480At60Hz,     SETREFRESH(60),   640,  480, validAndSafeAndDefault,  timingVESA_640x480_60hz},
  {kDisplay640x480At67Hz,     SETREFRESH(67),   640,  480, validAndSafe,            timingApple_640x480_67hz},
  {kDisplay640x480At72Hz,     SETREFRESH(72),   640,  480, validAndSafe,            timingVESA_640x480_72hz},
  {kDisplay640x480At75Hz,     SETREFRESH(75),   640,  480, validAndSafe,            timingVESA_640x480_75hz},
  {kDisplay640x480At85Hz,     SETREFRESH(85),   640,  480, valid,                   timingVESA_640x480_85hz},
  {kDisplay640x480At100Hz,    SETREFRESH(100),  640,  480, valid,                   timingInvalid},
  {kDisplay640x480At120Hz,    SETREFRESH(120),  640,  480, valid,                   timingInvalid},
  {kDisplay640x480At140Hz,    SETREFRESH(140),  640,  480, valid,                   timingInvalid},
  {kDisplay640x480At160Hz,    SETREFRESH(160),  640,  480, valid,                   timingVESA_640x480_85hz
  },
  
  {kDisplay800x600At56Hz,     SETREFRESH(56),   800,  600, valid,                   timingVESA_800x600_56hz},
  {kDisplay800x600At60Hz,     SETREFRESH(60),   800,  600, valid,                   timingVESA_800x600_60hz},
  {kDisplay800x600At72Hz,     SETREFRESH(72),   800,  600, valid,                   timingVESA_800x600_72hz},
  {kDisplay800x600At75Hz,     SETREFRESH(75),   800,  600, validAndSafe,            timingVESA_800x600_75hz},
  {kDisplay800x600At85Hz,     SETREFRESH(85),   800,  600, valid,                   timingVESA_800x600_85hz},
  {kDisplay800x600At100Hz,    SETREFRESH(100),  800,  600, valid,                   timingInvalid},
  {kDisplay800x600At120Hz,    SETREFRESH(120),  800,  600, valid,                   timingInvalid},
  {kDisplay800x600At140Hz,    SETREFRESH(140),  800,  600, valid,                   timingInvalid},
  {kDisplay800x600At160Hz,    SETREFRESH(160),  800,  600, valid,                   timingInvalid},
  
  {kDisplay800x500At60Hz,     SETREFRESH(60),   800,  500, 0,                       timingInvalid},
  {kDisplay800x512At60Hz,     SETREFRESH(60),   800,  512, 0,                       timingInvalid},

  {kDisplay832x624At75Hz,     SETREFRESH(75),   832,  624, validAndSafe,            timingApple_832x624_75hz},

  {kDisplay864x480At60Hz,     SETREFRESH(60),   864,  480, 0,                       timingInvalid},
  
  {kDisplay1024x768At60Hz,    SETREFRESH(60),  1024,  768, 0,                       timingVESA_1024x768_60hz},
  {kDisplay1024x768At70Hz,    SETREFRESH(70),  1024,  768, 0,                       timingVESA_1024x768_70hz},
  {kDisplay1024x768At72Hz,    SETREFRESH(72),  1024,  768, 0,                       timingVESA_1024x768_70hz},
  {kDisplay1024x768At74_9Hz,  SETREFRESH(74.9),1024,  768, validAndSafe,            timingApple_1024x768_75hz},
  {kDisplay1024x768At75Hz,    SETREFRESH(75),  1024,  768, valid,                   timingVESA_1024x768_75hz},
  {kDisplay1024x768At85Hz,    SETREFRESH(85),  1024,  768, valid,                   timingVESA_1024x768_85hz},
  {kDisplay1024x768At100Hz,   SETREFRESH(100), 1024,  768, valid,                   timingInvalid},
  {kDisplay1024x768At120Hz,   SETREFRESH(120), 1024,  768, valid,                   timingInvalid},

  {kDisplay1072x600At60Hz,    SETREFRESH(60),  1072,  600, 0,                       timingInvalid},
  
  {kDisplay1152x864At60Hz,    SETREFRESH(60),  1152,  864, 0,                       timingInvalid},
  {kDisplay1152x864At75Hz,    SETREFRESH(75),  1152,  864, 0,                       timingInvalid},
  {kDisplay1152x864At85Hz,    SETREFRESH(85),  1152,  864, 0,                       timingInvalid},
  {kDisplay1152x864At100Hz,   SETREFRESH(100), 1152,  864, 0,                       timingInvalid},
  
  {kDisplay1152x870At75Hz,    SETREFRESH(75),  1152,  870, valid,                   timingApple_1152x870_75hz},
  
  {kDisplay1280x960At60Hz,    SETREFRESH(60),  1280,  960, valid,                   timingVESA_1280x960_60hz},
  {kDisplay1280x960At75Hz,    SETREFRESH(75),  1280,  960, valid,                   timingVESA_1280x960_75hz},
  {kDisplay1280x960At85Hz,    SETREFRESH(85),  1280,  960, valid,                   timingVESA_1280x960_85hz},
  
  {kDisplay1280x1024At60Hz,   SETREFRESH(60),  1280, 1024, valid,                   timingVESA_1280x1024_60hz},
  {kDisplay1280x1024At75Hz,   SETREFRESH(75),  1280, 1024, valid,                   timingVESA_1280x1024_75hz},
  {kDisplay1280x1024At85Hz,   SETREFRESH(85),  1280, 1024, valid,                   timingVESA_1280x1024_85hz},
  {kDisplay1280x1024At100Hz,  SETREFRESH(100), 1280, 1024, valid,                   timingInvalid},
  
  {kDisplay1376x768At60Hz,    SETREFRESH(60),  1376,  768, valid,                   timingInvalid},
  
  {kDisplay1600x1024At60Hz,   SETREFRESH(60),  1600, 1024, valid,                   timingInvalid},
  {kDisplay1600x1024At76Hz,   SETREFRESH(76),  1600, 1024, valid,                   timingSony_1600x1024_76hz},
  {kDisplay1600x1024At85Hz,   SETREFRESH(85),  1600, 1024, valid,                   timingInvalid},
  
  {kDisplay1600x1200At60Hz,   SETREFRESH(60),  1600, 1200, valid,                   timingVESA_1600x1200_60hz},
  {kDisplay1600x1200At65Hz,   SETREFRESH(65),  1600, 1200, valid,                   timingVESA_1600x1200_65hz},
  {kDisplay1600x1200At70Hz,   SETREFRESH(70),  1600, 1200, valid,                   timingVESA_1600x1200_70hz},
  {kDisplay1600x1200At75Hz,   SETREFRESH(75),  1600, 1200, valid,                   timingVESA_1600x1200_75hz},
  {kDisplay1600x1200At80Hz,   SETREFRESH(80),  1600, 1200, valid,                   timingVESA_1600x1200_80hz},
  {kDisplay1600x1200At85Hz,   SETREFRESH(85),  1600, 1200, valid,                   timingVESA_1600x1200_85hz},
  {kDisplay1600x1200At100Hz,  SETREFRESH(100), 1600, 1200, valid,                   timingInvalid},
 
  {kDisplay1792x1344At60Hz,   SETREFRESH(60),  1792, 1344, valid,                   timingInvalid},
  {kDisplay1792x1344At75Hz,   SETREFRESH(75),  1792, 1344, valid,                   timingInvalid},
 
  {kDisplay1856x1392At60Hz,   SETREFRESH(60),  1856, 1392, valid,                   timingInvalid},
  {kDisplay1856x1392At75Hz,   SETREFRESH(75),  1856, 1392, valid,                   timingInvalid},
 
  {kDisplay1920x1080At60Hz,   SETREFRESH(60),  1920, 1080, valid,                   timingSony_1920x1080_60hz},
  {kDisplay1920x1080At72Hz,   SETREFRESH(72),  1920, 1080, valid,                   timingSony_1920x1080_72hz},
  {kDisplay1920x1080At75Hz,   SETREFRESH(75),  1920, 1080, valid,                   timingInvalid},
  {kDisplay1920x1080At85Hz,   SETREFRESH(85),  1920, 1080, valid,                   timingInvalid},
 
  {kDisplay1920x1200At60Hz,   SETREFRESH(60),  1920, 1200, valid,                   timingInvalid},
  {kDisplay1920x1200At76Hz,   SETREFRESH(76),  1920, 1200, valid,                   timingInvalid},
  {kDisplay1920x1200At85Hz,   SETREFRESH(85),  1920, 1200, valid,                   timingInvalid},
 
  {kDisplay1920x1440At60Hz,   SETREFRESH(60),  1920, 1440, valid,                   timingVESA_1920x1440_60hz},
  {kDisplay1920x1440At75Hz,   SETREFRESH(75),  1920, 1440, valid,                   timingVESA_1920x1440_75hz},
 
  {kDisplay2048x1536At60Hz,   SETREFRESH(60),  2048, 1536, valid,                   timingInvalid},
  {kDisplay2048x1536At75Hz,   SETREFRESH(75),  2048, 1536, valid,                   timingInvalid},
  {kDisplay2048x1536At85Hz,   SETREFRESH(85),  2048, 1536, valid,                   timingInvalid},  
};
