/*
 ** Copyright (c) 1997, 3Dfx Interactive, Inc.
 ** All Rights Reserved.
 **
 ** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
 ** the contents of this file may not be disclosed to third parties, copied or
 ** duplicated in any form, in whole or in part, without the prior written
 ** permission of 3Dfx Interactive, Inc.
 **
 ** RESTRICTED RIGHTS LEGEND:
 ** Use, duplication or disclosure by the Government is subject to restrictions
 ** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
 ** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
 ** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
 ** rights reserved under the Copyright Laws of the United States.
 **
 ** $Revision: 9$
 ** $Date: 10/11/00 7:39:58 PM$
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <3dfx.h>
#include <assert.h>
#include "gdebug.h"
#include "fxhwc.h"
#include "hwcpio.h"
#include "init.h"
#include <fx64.h>

#ifdef HWC_HSIM
extern "C" {
#include "tstbench.h"
extern void PCI_CFG_WR(FxU32 reg, FxU32 data, FxU32 device);
extern FxU32 PCI_CFG_RD(FxU32 reg, FxU32 device);
}                                                                              
#endif

FX_EXPORT void FX_CSTYLE
hwcPrintRegisterFields(HwcContext *hwc, HwcRegInfo *ri, FxU32 data, const int level ) 
{
	char outString[2048];		/* Used to construct verbose output */
	int index;							/* Index into sst2FieldValue[] */
	unsigned int x;					/* for bit math on fields */
	int j;									/* keeps track of position in string */
    FxU32 offset;
    HwcFieldType *fieldValue = hwc->fieldDefs;

    if (!GDBG_GET_DEBUGLEVEL(level) || ( ri->fieldDesc == 0 ))
        return;

	/* Output individual field values */
	index=ri->fieldDesc;
    offset = fieldValue[index].addr; 
	j=0;
	while ((fieldValue[index].offset + fieldValue[index].size <= 32) &&
					(fieldValue[index].addr == offset)) {
		/* x becomes the value in the current field */
		x=(1 << fieldValue[index].size)-1;
		x=(x << fieldValue[index].offset) & data;
		x=x >> fieldValue[index].offset;
		if (fieldValue[index].longName == NULL && x == 0) {
			/* do nothing.  Reserved fields shouldn't have data written to them. 
			** What about the case where we read from a register and get data back
			** 		from a reserved field? */
		} else {
			j += sprintf(outString+j, "%s:", fieldValue[index].abbrName);
			if (fieldValue[index].value != NULL) {
				/* Values are defined for this field.  Use x as index to values */
				j += sprintf(outString+j, "%s ",fieldValue[index].value[x].abbrName);
			} else if (fieldValue[index].size > 1) {
				j += sprintf(outString+j, "%xh ",x);
			} else {
				/* Binary field. No desc. values defined */
				if (x<1) {
					j += sprintf(outString+j, "N ");
				} else {
					j += sprintf(outString+j, "Y ");
				}		/* Binary value is true */
			}			/* Field is binary */
		}				/* Field is not Reserved == 0x0 */
		index++;
	}					/* while there are Fields left to examine */
	assert(strlen(outString) < sizeof(outString));
	gdbg_info(level, "%s: %s\n",ri->name,outString);
}

/*----------------------------------------------------------------------
   The routines hwcStore* and hwcLoad* are the funnels through which all
   hardware accesses go.  These procs route the stores and loads to the
   appropriate simulators and hardware.
  
   NOTE: we make the simplifying assumption that on H3 the second memory
   segment (32 Mbyte of non-modal LFB space) that is physically mapped using
   MemoryBase1, is virtually mapped right after the the 1st 16 Mbyte region
   Since the entire HWC/csim works off of a phony virtual address, this is
   not a problem
  ----------------------------------------------------------------------*/

FX_EXPORT void FX_CSTYLE
hwcIoVector(HwcSimulator * hws, HwcIoTransaction transaction,
			FxU32 size, FxU32 addr, FxU32 data)
{
	char *func;
    HwcAddressMap addrMap;
    HwcRegInfo *ri = NULL;
	HwcContext *hwc = hws->hwc;

	switch (transaction) {
	case HWC_IO_READ:
		func = "IO_READ  ";
        addrMap = HWC_ADDR_IO;
		break;
	case HWC_IO_WRITE:
		func = "IO_WRITE ";
        addrMap = HWC_ADDR_IO;
		break;
	case HWC_MEM_READ:
		func = "MEM_READ ";
        addrMap = HWC_ADDR_MEM;
		break;
	case HWC_MEM_WRITE:
		func = "MEM_WRITE";
        addrMap = HWC_ADDR_MEM;
		break;
	case HWC_CFG_READ:
		func = "CFG_READ ";
        addrMap = HWC_ADDR_CONFIG;
		break;
	case HWC_CFG_WRITE:
		func = "CFG_WRITE";
        addrMap = HWC_ADDR_CONFIG;
		break;
	case HWC_AGP_READ:
		func = "AGP_READ ";
        addrMap = HWC_ADDR_AGP;
		break;
	case HWC_AGP_WRITE:
		func = "AGP_WRITE";
        addrMap = HWC_ADDR_AGP;
		break;
	default:
		return;
	}
    
    if (GDBG_GET_DEBUGLEVEL(120) || HWC_TRACE_LEVEL(hws, HWC_VEC_IO)) {
    	// capture trace vector
        // output read transcation on last sim, since this is the value returned
        // output write transcation on first sim, since we want this message first
        if (( HWC_READ_TRANSACTION(transaction) && ( hws->next == NULL )) ||
            ( HWC_WRITE_TRANSACTION(transaction) && (hws == hwc->hws   ))) {
            if ( ri = (hwc->RegisterInfo)(hwc, addrMap, addr)) {
	            if ((ri->attr & HWC_REG_TYPE_MASK) == HWC_REG_FMT_FLOAT ) {
		            /* Register is a floating point value */
                    GDBG_INFO(120,"%s %d(0x%x,%f(0x%08x)) %s\n", 
                              func, size, addr, *(float*)&data, data, ri ? ri->name : "");
	            } else {
                    GDBG_INFO(120,"%s %d(0x%x,%11d(0x%08x)) %s\n", 
                              func, size, addr, data, data, ri ? ri->name : "");
	            }

                hwcPrintRegisterFields(hwc, ri, data, 120);
            } else {
                GDBG_INFO(120,"%s %d(0x%x,%11d(0x%08x)) %s\n", 
                          func, size, addr, data, data, ri ? ri->name : "");
            }
        }
    }

  	HWC_VECTOR(hws, HWC_VEC_IO, 0, "%s %02d 0x%08lx 0x%08lx\n", func, size, addr, data);
}

/*----------------------------------------------------------------------
   execute a 8 bit store to SST, only LFB access is allowed
  ----------------------------------------------------------------------*/
FX_EXPORT FxU8 FX_CSTYLE
hwcStore8(volatile void *addr, FxU8 data)
{
	HwcContext *hwc;
	HwcSimulator *hws;

	/* do some sanity checks */
	if (HWC_BAD_ADDRESS(addr)) {
		GDBG_ERROR("SET8", "bad address=0x%x  data=%d(0x%08x)\n",
				   addr, data, data);
		return data;
	}
	hwc = HWC_FAKE_ADDRESS_GET_HWC(addr);

	for (hws = hwc->hws; hws; hws = hws->next) {
	    hwcIoVector(hws, HWC_MEM_WRITE, 8, (FxU32) addr, data);
		(*hws->Store8) (hws, addr, data);
	}

	return data;
}

/*----------------------------------------------------------------------
   execute a 16 bit store to SST, only LFB access is allowed
  ----------------------------------------------------------------------*/
FX_EXPORT FxU16 FX_CSTYLE
hwcStore16(volatile void *addr, FxU16 data)
{
	HwcContext *hwc;
	HwcSimulator *hws;

	/* do some sanity checks */
	if (HWC_BAD_ADDRESS(addr)) {
		GDBG_ERROR("SET16", "bad address=0x%x  data=%d(0x%08x)\n",
				   addr, data, data);
		return data;
	}
	if (1 & (FxU32) addr) {
		GDBG_ERROR("SET16", "unaligned address=0x%x  data=%d(0x%08x)\n",
				   addr, data, data);
		return data;
	}
	hwc = HWC_FAKE_ADDRESS_GET_HWC(addr);

	for (hws = hwc->hws; hws; hws = hws->next) {
	    hwcIoVector(hws, HWC_MEM_WRITE, 16, (FxU32) addr, data);
		(*hws->Store16) (hws, addr, data);
	}

	return data;
}

/*----------------------------------------------------------------------
   execute a 32 bit store
  ----------------------------------------------------------------------*/
FX_EXPORT FxU32 FX_CSTYLE
hwcStore32(volatile void *addr, FxU32 data)
{
	HwcContext *hwc;
	HwcSimulator *hws;

	/* do some sanity checks */
	if (HWC_BAD_ADDRESS(addr)) {
		GDBG_ERROR("SET", "bad address=0x%x  data=%d(0x%08x)\n",
				   addr, data, data);
		return data;
	}
	if (3 & (FxU32) addr) {
		GDBG_ERROR("SET", "unaligned address=0x%x  data=%d(0x%08x)\n",
				   addr, data, data);
		return data;
	}
	hwc = HWC_FAKE_ADDRESS_GET_HWC(addr);

	for (hws = hwc->hws; hws; hws = hws->next) {
		hwcIoVector(hws, HWC_MEM_WRITE, 32, (FxU32) addr, data);
		(*hws->Store32) (hws, addr, data);
	}

	return data;
}

/*----------------------------------------------------------------------
   same as hwcStore32 except data is a float to be shoved right into an
        integer register.  NOTE: this code may be machine dependent
  ----------------------------------------------------------------------*/
FX_EXPORT FxFloat FX_CSTYLE
hwcStore32f(volatile void *addr, FxFloat data)
{
	hwcStore32(addr, *(FxU32 *) & data);

	return data;
}

/*----------------------------------------------------------------------
   execute a 8 bit read
  ----------------------------------------------------------------------*/
FxU8 FX_EXPORT FX_CSTYLE
hwcLoad8(volatile void *addr)
{
	FxU8 data = 0xbd;
	HwcContext *hwc;
	HwcSimulator *hws;
    FxU32 busData = 0xbdbdbdbd;

	/* do some sanity checks */
	if (HWC_BAD_ADDRESS(addr)) {
		GDBG_ERROR("GET8", "unaligned address=0x%x  data=%d(0x%08x)\n",
				   addr, data, data);
		return data;
	}
	hwc = HWC_FAKE_ADDRESS_GET_HWC(addr);

    for (hws = hwc->hws; hws; hws = hws->next) {
	    hws->lastRead = data = (*hws->Load8) (hws, addr);
	    hwcIoVector(hws, HWC_MEM_READ, 8, (FxU32) addr, data);
	}


	return data;
}

/*----------------------------------------------------------------------
   execute a 16 bit read
  ----------------------------------------------------------------------*/
FxU16 FX_EXPORT FX_CSTYLE
hwcLoad16(volatile void *addr)
{
	FxU16 data = 0xbad0;
	HwcContext *hwc;
	HwcSimulator *hws;
    FxU32 busData = 0xbad0bad0;

	/* do some sanity checks */
	if (HWC_BAD_ADDRESS(addr)) {
		GDBG_ERROR("GET16", "bad address=0x%x  data=%d(0x%08x)\n",
				   addr, data, data);
		return data;
	}
	if (1 & (FxU32) addr) {
		GDBG_ERROR("GET16", "unaligned address=0x%x  data=%d(0x%08x)\n",
				   addr, data, data);
		return data;
	}
	hwc = HWC_FAKE_ADDRESS_GET_HWC(addr);

   	for (hws = hwc->hws; hws; hws = hws->next) {
   		hws->lastRead = data = (*hws->Load16) (hws, addr);
        hwcIoVector(hws, HWC_MEM_READ, 16, (FxU32) addr, data);
    }

    return data;
}

/*----------------------------------------------------------------------
   execute a 32 bit read from SST
  ----------------------------------------------------------------------*/
FxU32 FX_EXPORT FX_CSTYLE
hwcLoad32(volatile void *addr)
{
	HwcSimulator *hws;
	FxU32 data = 0xdeadbeef;
	HwcContext *hwc;
    FxU32 busData = 0xdeadbeef;

	/* do some sanity checks */
	if (HWC_BAD_ADDRESS(addr)) {
		GDBG_ERROR("GET", "bad address=0x%x  data=%d(0x%08x)\n",
				   addr, data, data);
		return data;
	}
	if (3 & (FxU32) addr) {
		GDBG_ERROR("GET", "unaligned address=0x%x  data=%d(0x%08x)\n",
				   addr, data, data);
		return data;
	}
	hwc = HWC_FAKE_ADDRESS_GET_HWC(addr);

   	for (hws = hwc->hws; hws; hws = hws->next) {
   		hws->lastRead = data = (*hws->Load32) (hws, addr);
	    hwcIoVector(hws, HWC_MEM_READ, 32, (FxU32) addr, data);
    }

	return data;
}

#ifdef HWC_CSIM
/*----------------------------------------------------------------------
   The hwcInPort* and hwcOutPort* routines handle all port i/o operations.
   These routines perform the i/o operations on all the active simulators
   and/or hardware.
  ----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
   execute an 32 bit config write operation
   XXX: need to fixup setting of base addr
  ----------------------------------------------------------------------*/
void FX_EXPORT FX_CSTYLE
hwcCfgStore32(FxU16 port, int mechanism,
			  FxU32 bus_number, FxU32 device_number,
			  FxU32 function_number, FxU32 register_offset,
			  FxU32 data)
{
	HwcContext *hwc = HWC_DEV_TO_HWC(device_number);

	GDBG_INFO(121, "     W PCI CFG(0x%x,%11d(0x%08x))\n", port, data, data);

	if (hwc && hwc->hws ) {
		hwcIoVector(hwc->hws, HWC_CFG_WRITE, 32, register_offset, data);

#ifdef HWC_HSIM
		if (hwc->dev->state & HWC_HAS_HSIM) {
			if (mechanism == 1)
				PCI_CFG_WR(register_offset, data, device_number);
			else
				PCI_IOW32(port, data);
		}
#endif
		GDBG_INFO(120, "       SET_IO(0x%x,%11d(0x%08x))\n", port, data, data);
		/* GMT: this might be a problem if the real HW has a different address */
		if ((mechanism == 1 && bus_number == 0 && function_number == 0) ||
			(mechanism == 2)) {
			switch (register_offset) {
			case 4:			/* status | command     */
				hwc->memEnabled = data & SST_PCIMEM_ACCESS_EN;
				hwc->ioEnabled = data & SST_PCIIO_ACCESS_EN;
				GDBG_INFO(4, "    memEnabled = %d\n", hwc->memEnabled != 0);
				GDBG_INFO(4, "    ioEnabled = %d\n", hwc->ioEnabled != 0);
				break;
			case 0x10:			/* base address 0 */
				GDBG_ERROR("hwcCfgStore32", "remapping base address 0\n");
				break;
			case 0x14:			/* base address 1 */
				GDBG_ERROR("hwcCfgStore32", "remapping base address 1\n");
				break;
			case 0x18:			/* i/o base address */
				GDBG_ERROR("hwcCfgStore32", "remapping i/o base address\n");
				break;
			default:
				GDBG_ERROR("hwcCfgStore32", "bad register offset 0x%x(%d)\n",
						   register_offset, register_offset);
			}
		}
	}
	if (hwc->dev->state & HWC_HAS_HW) {
	    _pioOutLong(port, data);
    }

	hwcInfo.lastIO = 0;

	return;
}

/*----------------------------------------------------------------------
   execute an 32 bit config read operation
  ----------------------------------------------------------------------*/
FxU32 FX_EXPORT FX_CSTYLE
hwcCfgLoad32(FxU16 port, int mechanism,
			 FxU32 bus_number, FxU32 device_number,
			 FxU32 function_number, FxU32 register_offset)
{
	FxU32 data = 0xFFFFFFFF;
	HwcDevice *dev = &hwcInfo.devices[device_number];
	HwcContext *hwc = HWC_DEV_TO_HWC(device_number);

	GDBG_INFO(121, "     R PCI CFG(0x%x,%11d(0x%08x))\n", port, data, data);

	if (hwc) {
#ifdef HWC_HSIM
		if (hwc->dev->state & HWC_HAS_HSIM) {
			if (mechanism == 1)	/* mechanism 1 */
				data = PCI_CFG_RD(register_offset, device_number);
			else
				data = (FxU32) PCI_IOR32(port);
		}
#endif
		if (mechanism == 1) {
			if (hwc) {
				if (function_number == 0) {
					switch (register_offset) {
					case 0:	/* device | vendor */
						if (hwc)
							data = (hwc->dev->deviceID << 16) | hwc->dev->vendorID;
						break;
					case 4:	/* status | command     */
						data = (0x30 << 16) | hwc->config.command;
						break;
					case 8:	/* class_code | revision */
						data = (hwc->config.classCode << 8) | hwc->fbiRevision;
						break;
					case 0xC:	/* latency | cache line size */
						data = 0;
						break;
					case 0x10:	/* base address 0 */
						data = 0;
						break;
					case 0x14:	/* base address 1 */
						data = 0x2000000;
						break;
					case 0x18:	/* i/o base address */
						data = 0xFFFFFF01;
						break;
					default:
						GDBG_ERROR("halInPort32", "bad register offset 0x%x(%d)\n",
								   register_offset, register_offset);
					}
				}
			}
		} else {
			/* config mechanism 2
			   just return FFFFFFFF (don't support this mode)
			   GMT: if real HW uses this mode, then value returned will be from
			   the real HW and not CSIM anyway
			 */
		}
		GDBG_INFO(120, "       GET_IO(0x%x,%11d(0x%08x))\n", port, data, data);
	}
	/* perform operation but don't overwrite data value if no HW */

        if ( hwcInfo.busDetected )  { 
	    if (dev->state & HWC_HAS_HW)
		data = _pioInLong(port);
	    else
		_pioInLong(port);
        }

	hwcInfo.lastIO = 0;

	if (hwc && hwc->hws ) {
		hwcIoVector(hwc->hws, HWC_CFG_READ, 32, register_offset, data);
	}
	return data;
}
#endif /* HWC_CSIM */

// XXX LOOOK temporary hack to go from port number to context
HwcContext *lastContext;
HwcContext *
hwcPortToContext(FxU16 port)
{
    HwcContext *hwc = lastContext;

    if ( hwc == NULL ) {
		GDBG_ERROR("hwcPortToContext", "bad port=0x%x\n", port);
        exit(1);
    }

    return hwc;
}

/*----------------------------------------------------------------------
   execute an 8 bit port input operation
  ----------------------------------------------------------------------*/
FX_EXPORT FxU8 FX_CSTYLE
hwcIOLoad8(FxU16 port)
{
#ifdef HWC_CSIM
	FxU8 data = 0xFF;
	HwcContext *hwc;
	HwcSimulator *hws;

	/* read from board, do some sanity checks */

	hwc = hwcPortToContext(port);

	for (hws = hwc->hws; hws; hws = hws->next) {
	    hws->lastRead = data = (*hws->IOLoad8) (hws, port);
	    hwcIoVector(hws, HWC_IO_READ, 8, port, data);
    }

	GDBG_INFO(121, "     CSIM R PCI IO8(0x%x,%11d(0x%08x))\n", port, data, data);

	hwcInfo.lastIO = 0;
#else
	data = _pioInByte(port);
	GDBG_INFO(121, "     HW R PCI IO8(0x%x,%11d(0x%08x))\n", port, data, data);
#endif

	return data;
}

/*----------------------------------------------------------------------
   execute an 16 bit port input operation
  ----------------------------------------------------------------------*/
FX_EXPORT FxU16 FX_CSTYLE
hwcIOLoad16(FxU16 port)
{
#ifdef HWC_CSIM
	FxU16 data = 0xFFFF;
	HwcContext *hwc;
	HwcSimulator *hws;

	/* read from board, do some sanity checks */

	if (port & 1) {
		GDBG_ERROR("GET_IO16", "unaligned port=0x%x  data=%d(0x%08x)\n",
				   port, data, data);
		return data;
	}

	hwc = hwcPortToContext(port);

    /* check other simulators are consistent */

	for (hws = hwc->hws; hws; hws = hws->next) {
	    hws->lastRead = data = (*hws->IOLoad16) (hws, port);
	    hwcIoVector(hws, HWC_IO_READ, 16, port, data);
    }

	hwcInfo.lastIO = 0;
#else
	data = _pioInWord(port);
	GDBG_INFO(121, "     HW R PCI IO16(0x%x,%11d(0x%08x))\n", port, data, data);
#endif

	return data;
}

/*----------------------------------------------------------------------
   execute an 32 bit port input operation
  ----------------------------------------------------------------------*/
FX_EXPORT FxU32 FX_CSTYLE
hwcIOLoad32(FxU16 port)
{
#ifdef HWC_CSIM
	FxU32 bus_number = 0, device_number = 0, function_number = 0, register_offset = 0;
	FxU32 data = 0xFFFFFFFF;
	HwcContext *hwc;
	HwcSimulator *hws;

	/* check for config cycles */
	if (port == CONFIG_DATA_PORT) {
		if (hwcInfo.lastIO & CONFIG_ADDRESS_ENABLE_BIT) {	/* config mechanism 1 */
			FxU32 bus_number = (hwcInfo.lastIO >> 16) & 0xFF;
			FxU32 device_number = (hwcInfo.lastIO >> 11) & 0x1F;
			FxU32 function_number = (hwcInfo.lastIO >> 8) & 0x7;
			FxU32 register_offset = (hwcInfo.lastIO >> 0) & 0xFC;
			data = hwcCfgLoad32(port, 1, bus_number, device_number, function_number, register_offset);
		}
		return data;
	} else if (hwcInfo.lastIO == CONFIG_MAPPING_ENABLE_BYTE) {	/* config mechanism 2 */
		data = hwcCfgLoad32(port, 2, bus_number, device_number, function_number, register_offset);
		return data;
	}
	/* read from board, do some sanity checks */

	hwcInfo.lastIO = 0;

	if (port & 3) {
		GDBG_ERROR("GETIO", "unaligned port=0x%x  data=%d(0x%08x)\n",
				   port, data, data);
		return data;
	}

	hwc = hwcPortToContext(port);

    for (hws = hwc->hws; hws; hws = hws->next) {
	    hws->lastRead = data = (*hws->IOLoad32) (hws, port);
	    hwcIoVector(hws, HWC_IO_READ, 32, port, data);
    }


#else
	data = _pioInLong(port);
	GDBG_INFO(121, "     HW R PCI IO32(0x%x,%11d(0x%08x))\n", port, data, data);
#endif

	return data;
}

/*----------------------------------------------------------------------
   execute an 8 bit port output operation
  ----------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcIOStore8(FxU16 port, FxU8 data)
{
	GDBG_INFO(121, "     W PCI IO8(0x%x,%11d(0x%08x))\n", port, data, data);

#ifdef HWC_CSIM
	hwcInfo.lastIO = 0;

	if (port == CONFIG_ADDRESS_PORT) {
		hwcInfo.lastIO = data;
            if ( hwcInfo.busDetected )  { 
		_pioOutByte(port, data);
            }
		return FXTRUE;
	} else {
		HwcContext *hwc;
		HwcSimulator *hws;

		/* write to board, do some sanity checks */

		hwc = hwcPortToContext(port);

		/* perform the i/o write operation */

		for (hws = hwc->hws; hws; hws = hws->next) {
			hwcIoVector(hws, HWC_IO_WRITE, 8, port, data);
			(*hws->IOStore8) (hws, port, data);
		}

	}
#else
	_pioOutByte(port, data);
#endif
	return FXTRUE;
}

/*----------------------------------------------------------------------
   execute an 16 bit port output operation
  ----------------------------------------------------------------------*/

FX_EXPORT FxBool FX_CSTYLE
hwcIOStore16(FxU16 port, FxU16 data)
{
#ifdef HWC_CSIM
	HwcContext *hwc;
	HwcSimulator *hws;

	GDBG_INFO(121, "     W PCI IO16(0x%x,%11d(0x%08x))\n", port, data, data);
	hwcInfo.lastIO = 0;

	/* write to board, do some sanity checks */

	if (port & 1) {
		GDBG_ERROR("SET_IO16", "unaligned port=0x%x  data=%d(0x%08x)\n",
				   port, data, data);
		return FXFALSE;
	}

	hwc = hwcPortToContext(port);

	/* perform the i/o write operation */

	for (hws = hwc->hws; hws; hws = hws->next) {
		hwcIoVector(hws, HWC_IO_WRITE, 16, port, data);
		(*hws->IOStore16) (hws, port, data);
	}

#else
	GDBG_INFO(121, "     W PCI IO16(0x%x,%11d(0x%08x))\n", port, data, data);
	_pioOutWord(port, data);
#endif
	return FXTRUE;
}

/*----------------------------------------------------------------------
   execute an 32 bit port output operation
  ----------------------------------------------------------------------*/

FX_EXPORT FxBool FX_CSTYLE
hwcIOStore32(FxU16 port, FxU32 data)
{
#ifdef HWC_CSIM
	HwcContext *hwc;
	HwcSimulator *hws;
	FxU32 bus_number =0, device_number =0, function_number =0, register_offset =0;

	GDBG_INFO(121, "     W PCI IO32(0x%x,%11d(0x%08x))\n", port, data, data);

	/* check for config cycles */

	if (port == CONFIG_ADDRESS_PORT) {
		if (data & CONFIG_ADDRESS_ENABLE_BIT)
			hwcInfo.lastIO = data;
		else
			hwcInfo.lastIO = 0;
		_pioOutLong(port, data);
		return FXTRUE;
	} else if (port == CONFIG_DATA_PORT &&
			   (hwcInfo.lastIO & CONFIG_ADDRESS_ENABLE_BIT)) {	/* config mechanism 1 */
		bus_number = (hwcInfo.lastIO >> 16) & 0xFF;
		device_number = (hwcInfo.lastIO >> 11) & 0x1F;
		function_number = (hwcInfo.lastIO >> 8) & 0x7;
		register_offset = (hwcInfo.lastIO >> 0) & 0xFC;
		hwcCfgStore32(port, 1, bus_number, device_number, function_number, register_offset, data);
		return FXTRUE;
	} else if (hwcInfo.lastIO == CONFIG_MAPPING_ENABLE_BYTE) {	/* config mechanism 2 */
		device_number = ((port - CONFIG_MAPPING_OFFSET) >> 8) & 0xFF;
		register_offset = (port >> 0) & 0xFC;
		hwcCfgStore32(port, 2, bus_number, device_number, function_number, register_offset, data);
		return FXTRUE;
	}
	/* write to board, do some sanity checks */

	if (port & 3) {
		GDBG_ERROR("SETIO", "unaligned port=0x%x  data=%d(0x%08x)\n",
				   port, data, data);
		return FXFALSE;
	}
	hwcInfo.lastIO = 0;

	hwc = hwcPortToContext(port);

	/* perform the i/o write operation */

	for (hws = hwc->hws; hws; hws = hws->next) {
		hwcIoVector(hws, HWC_IO_WRITE, 32, port, data);
		(*hws->IOStore32) (hws, port, data);
	}

#else
	GDBG_INFO(121, "     W PCI IO32(0x%x,%11d(0x%08x))\n", port, data, data);

	_pioOutWord(port, data);
#endif
	return FXTRUE;
}

/*----------------------------------------------------------------------
   backdoor memory accessors
  ----------------------------------------------------------------------*/
FX_EXPORT void FX_CSTYLE
hwcWriteMemory(HwcContext * hwc, FxU32 addr, FxU32 size, FxU32 memType, FxU32 mask, FxU32 data)
{
	HwcSimulator *hws;

	for (hws = hwc->hws; hws; hws = hws->next) {
		(*hws->WriteMemory) (hws, addr, size, memType, mask, data);
    }
}

FX_EXPORT FxU32 FX_CSTYLE
hwcReadMemory(HwcContext * hwc, FxU32 addr, FxU32 size, FxU32 memType,FxU32 chipID)
{
	return (* hwc->hws->ReadMemory) (hwc->hws, addr, size, memType, chipID);
}

FX_EXPORT HwcPixel FX_CSTYLE
hwcReadPixel(HwcSimulator * hws, HwcBuffer * pBuff, int x, int y)
{
    if ( pBuff && ( x < (int)pBuff->width ) && ( y < (int)pBuff->height )) {
	    return (*hws->ReadPixel) (hws, (HwcBufferSpec*)pBuff, x, y);
    } else return 0xdeadbeef;
}

FX_EXPORT FxBool FX_CSTYLE
hwcComparePixel(HwcBuffer * pBuff, int x, int y, HwcPixel * pPixel)
{
	HwcSimulator *hws;
	FxBool first = FXTRUE;
	HwcPixel ref, q;

	for (hws = pBuff->hwc->hws; hws; hws = hws->next) {
		q = (*hws->ReadPixel) (hws, (HwcBufferSpec*)pBuff, x, y);
		if (first) {
			ref = q;
			first = FXFALSE;
		} else {
			if (ref != q)
				return FXFALSE;
		}
	}

	*pPixel = ref;
	return !first;
}

FX_EXPORT FxBool FX_CSTYLE
hwcCompareRect(HwcBuffer * pBuff, int x, int y, int width, int height, HwcPixel * pPixel)
{
	int dx, dy;
	HwcPixel q;

	for (dy = 0; dy < height; dy++) {
		for (dx = 0; dx < width; dx++) {
			if (!hwcComparePixel(pBuff, x + dx, y + dy, &q))
				return FXFALSE;
			if (pPixel && (*pPixel != q))
				return FXFALSE;
		}
	}
	return FXTRUE;
}

FX_EXPORT void FX_CSTYLE
hwcPixelToRGB(HwcBuffer * pBuff, HwcPixel pix, FxU8 * r, FxU8 * g, FxU8 * b)
{
	switch (pBuff->format) {
	case HWC_PIXFMT_RGB_565:
	case HWC_PIXFMT_AA_16:
		*r = (FxU8) (((pix & 0xF800) << 8) >> 16);
		*g = (FxU8) (((pix & 0x07E0) << 5) >> 8);
		*b = (FxU8) (((pix & 0x001F) << 3));
		break;
	case HWC_PIXFMT_FBCMP_16:
		*r = (FxU8) ((( pix >> 12 ) & 0x3f) <<2);
		*g = (FxU8) ((( pix >> 6 ) & 0x3f) <<2);
		*b = (FxU8) (( pix & 0x3f) <<2);
		break;
	case HWC_PIXFMT_FBCMP_32:
	case HWC_PIXFMT_AA_32:
	case HWC_PIXFMT_ARGB_8888:
		*r = (FxU8) ((pix & 0xFF0000) >> 16);
		*g = (FxU8) ((pix & 0xFF00) >> 8);
		*b = (FxU8) ((pix & 0x00FF));
		break;
	default:
		GDBG_ERROR("hwcPixelToRGB", "invalid pixel format %d\n",
				   pBuff->format);
	}
}

FX_EXPORT void FX_CSTYLE
hwcReadColor(HwcSimulator * hws, HwcBuffer *pBuff, int x, int y, FxU8 *a, FxU8 *r, FxU8 *g, FxU8 *b)
{
    if ( pBuff && ( x < (int)pBuff->width ) && ( y < (int)pBuff->height )) {
        FxU32 pixel = (*hws->ReadColor) (hws, (HwcBufferSpec*)pBuff, x, y);

        *a = (FxU8)((pixel >> 24)&0xff);
        *r = (FxU8)((pixel >> 16)&0xff);
        *g = (FxU8)((pixel >> 8) &0xff);
        *b = (FxU8)(pixel & 0xff);
    } else {
        *a = *r = *g = *b = 0;
    }
}

FX_EXPORT FxBool FX_CSTYLE
hwcWritePixel(HwcSimulator * hws, HwcBuffer * pBuff, int x, int y, HwcPixel pixel)
{
	return (*hws->WritePixel) (hws, (HwcBufferSpec*)pBuff, x, y, pixel);
}

FX_EXPORT void FX_CSTYLE
hwcSetPixel(HwcBuffer * pBuff, int x, int y, HwcPixel pixel)
{
	HwcSimulator *hws;

	for (hws = pBuff->hwc->hws; hws; hws = hws->next) {
		(*hws->WritePixel) (hws, (HwcBufferSpec*)pBuff, x, y, pixel);
	}
}

FX_EXPORT void FX_CSTYLE
hwcSetRect(HwcBuffer * pBuff, int x, int y, int width, int height, HwcPixel pixel)
{
	int dx, dy;

	for (dy = 0; dy < height; dy++) {
		for (dx = 0; dx < width; dx++) {
			hwcSetPixel(pBuff, x + dx, y + dy, pixel);
		}
	}
}

FX_EXPORT FxBool FX_CSTYLE
hwcLockBuffer(HwcBuffer * buff)
{
	return FXTRUE;
}

FX_EXPORT FxBool FX_CSTYLE
hwcUnlockBuffer(HwcBuffer * buff)
{
	return FXTRUE;
}
