
//  This is the WinNT4 specific version of this file.
//  These macros allows the code in di_i2c.c and ds_i2c.c to be operating system independent.

//#include "3dfx.h"
#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"

#define I2C_DEVINFOPTR(pcontext)  ((PI2C_DEVINFO)&(((PHW_DEVICE_EXTENSION)(pcontext))->i2cInfo))

#define IsVoodoo3(pcontext) (((PHW_DEVICE_EXTENSION)(pcontext))->PCIDeviceID == TDFX_AVENGER_ID)

#define I2C_REGBASEPTR(pcontext)	((PH3_MEMBASE0) ((PHW_DEVICE_EXTENSION)(pcontext))->MappedAddress[SST_IO_REGS_INDEX])

#define I2C_GETBUSINFO(pcontext, busid) (PI2C_BUSINFO) &(((PHW_DEVICE_EXTENSION)(pcontext))->i2cInfo.BusList[(busid)-1])

#define I2C_DELAYTIME(usecs)       VideoPortStallExecution(usecs);

//the following macro is not implemented.
#ifdef DEBUG
#define DEBUGI2C(expr, args)  { if (expr)         VideoDebugPrint((0, ##args ));}
#else
#define DEBUGI2C(foo, bar)
#endif
