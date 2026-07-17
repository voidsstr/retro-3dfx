#ifndef __INIT_H__
#define __INIT_H__

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
** $Revision: 4$
** $Date: 10/11/00 7:40:16 PM$
*/

/* Useful defines */
#define PCICFG_WR(ADDR, DEVNUM, DATA) \
	if (hwcSetConfigData(ADDR,DEVNUM,&DATA) == FXFALSE) return(FXFALSE)

#define PCICFG_RD(ADDR, DEVNUM, DATA) \
	if (hwcGetConfigData(ADDR,DEVNUM,&DATA) == FXFALSE) return (FXFALSE)

#ifndef DIRECTX
  #undef GETENV
  #undef ATOI
  #undef ATOF
  #undef SSCANF
  #undef POW
  #define GETENV(A) getenv(A)
  #define ATOI(A) atoi(A)
  #define ATOF(A) atof(A)
  #define FTOL( X ) ((FxU32)(X))
  #define POW( A, B ) pow( A, B )
  #define SSCANF( A, B, C ) sscanf( A, B, C )

#else /* DIRECTX */
  #include "ddglobal.h"
  #pragma optimize ("",off)   /* ddglobal.h tuns this on for retail builds */
  #undef INIT_PRINTF
  #undef INIT_INFO
  #undef GETENV
  #undef ATOI
  #undef ATOF
  #undef FTOL
  #undef ITOF_INV
  #undef SSCANF
  #undef POW
  #define GETENV(A)  ddgetenv(A)
  #define ATOI(A) ddatoi(A)
  #define ATOF(A) ddatof(A)
  #define FTOL(A) ddftol(A)
  #define ITOF_INV(A) dd_itof_inv(A)
  #define POW( A, B ) ddpow( A, B )
  #define SSCANF( A, B, C ) ddsscanf( A, B, C )
#endif /* DIRECTX */


/* Following defines need to go in "sst.h" eventually */
/*--------- SST PCI Configuration Command bits --------------*/
#define SST_PCIMEM_ACCESS_EN               BIT(1)
#define SST_PCIIO_ACCESS_EN		   BIT(0)

/*------- SST PCI Configuration Register defaults -----------*/
#define SST_PCI_INIT_ENABLE_DEFAULT        0x0
#define SST_PCI_BUS_SNOOP_DEFAULT          0x0

/*--- SST PCI Init Enable Configuration Register defaults ---*/
#define SST_SLI_OWNPCI                     SST_SCANLINE_SLV_OWNPCI
#define SST_SLI_MASTER_OWNPCI              0x0
#define SST_SLI_SLAVE_OWNPCI               SST_SCANLINE_SLV_OWNPCI


#define SST_EN_TEX_MEM_REFRESH             BIT(0)
#define SST_TEX_MEM_REFRESH_SHIFT          1
#define SST_TEX_MEM_REFRESH                (0x1FF<<SST_TEX_MEM_REFRESH_SHIFT)
#define SST_TEX_MEM_PAGE_SIZE_SHIFT        10
#define SST_TEX_MEM_PAGE_SIZE_8BITS        (0x0<<SST_TEX_MEM_PAGE_SIZE_SHIFT)
#define SST_TEX_MEM_PAGE_SIZE_9BITS        (0x1<<SST_TEX_MEM_PAGE_SIZE_SHIFT)
#define SST_TEX_MEM_PAGE_SIZE_10BITS       (0x2<<SST_TEX_MEM_PAGE_SIZE_SHIFT)
#define SST_TEX_MEM_SECOND_RAS_BIT_SHIFT   12
#define SST_TEX_MEM_SECOND_RAS_BIT_BIT17   (0x0<<SST_TEX_MEM_SECOND_RAS_BIT_SHIFT)
#define SST_TEX_MEM_SECOND_RAS_BIT_BIT18   (0x1<<SST_TEX_MEM_SECOND_RAS_BIT_SHIFT)
#define SST_EN_TEX_MEM_SECOND_RAS          BIT(14)
#define SST_TEX_MEM_TYPE_SHIFT             15
#define SST_TEX_MEM_TYPE_EDO               (0x0<<SST_TEX_MEM_TYPE_SHIFT)
#define SST_TEX_MEM_TYPE_SYNC              (0x1<<SST_TEX_MEM_TYPE_SHIFT)
#define SST_TEX_MEM_DATA_SIZE_16BIT        0x0
#define SST_TEX_MEM_DATA_SIZE_8BIT         BIT(18)
#define SST_TEX_MEM_DO_EXTRA_CAS           BIT(19)
#define SST_TEX_MEM2                       BIT(20)

#define SST_TREXINIT0_DEFAULT \
  ( (SST_EN_TEX_MEM_REFRESH)  \
  | (0x020 << SST_TEX_MEM_REFRESH_SHIFT) \
  | (SST_TEX_MEM_PAGE_SIZE_9BITS) \
  | (SST_TEX_MEM_SECOND_RAS_BIT_BIT18) \
  | (SST_EN_TEX_MEM_SECOND_RAS) \
  | (SST_TEX_MEM_TYPE_EDO) \
  | (SST_TEX_MEM_DATA_SIZE_16BIT) \
  | (0 & SST_TEX_MEM_DO_EXTRA_CAS) \
  | (0 & SST_TEX_MEM2)  )

#define SST_TREX0INIT0_DEFAULT             SST_TREXINIT0_DEFAULT
#define SST_TREX1INIT0_DEFAULT             SST_TREXINIT0_DEFAULT
#define SST_TREX2INIT0_DEFAULT             SST_TREXINIT0_DEFAULT

/*----------------- SST trexInit1 bits -----------------------*/
#define SST_TEX_SCANLINE_INTERLEAVE_MASTER 0x0
#define SST_TEX_SCANLINE_INTERLEAVE_SLAVE  BIT(0)
#define SST_EN_TEX_SCANLINE_INTERLEAVE     BIT(1)
#define SST_TEX_FT_FIFO_SIL_SHIFT          2
#define SST_TEX_FT_FIFO_SIL                (0x1F<<SST_TEX_FT_FIFO_SIL_SHIFT)
#define SST_TEX_TT_FIFO_SIL_SHIFT          7
#define SST_TEX_TT_FIFO_SIL                (0xF<<SST_TEX_TT_FIFO_SIL_SHIFT)
#define SST_TEX_TF_CLK_DEL_ADJ_SHIFT       12
#define SST_TEX_TF_CLK_DEL_ADJ             (0xF<<SST_TEX_TF_CLK_DEL_ADJ_SHIFT)
#define SST_TEX_RG_TTCII_INH               BIT(16)
#define SST_TEX_USE_RG_TTCII_INH           BIT(17)
#define SST_TEX_SEND_CONFIG                BIT(18)
#define SST_TEX_RESET_FIFO                 BIT(19)
#define SST_TEX_RESET_GRX                  BIT(20)
#define SST_TEX_PALETTE_DEL_SHIFT          21
#define SST_TEX_PALETTE_DEL                (0x3<<SST_TEX_PALETTE_DEL_SHIFT)
#define SST_TEX_SEND_CONFIG_SEL_SHIFT      23
#define SST_TEX_SEND_CONFIG_SEL            (0x7<<SST_TEX_SEND_CONFIG_SEL_SHIFT)

/* After things stabilize, the fifo stall inputs levels should be backed off
   from the max. conservative values that are being used now for better
        performance.
   SST_TEX_FT_FIFO_SIL =  ??
   SST_TEX_TT_FIFO_SIL =  ??  (effects multi-trex only)
        */

/* for trex ver. 1 bringup, SST_TEX_PALETTE_DEL should be set to it's max
   (== 3) for <50 MHz bringup */

/* TF Clock delays for (FBI Revision, TMU Revision) */
#define SST_TEX_TF_CLK_DEL_ADJ_DEFAULT_R10  0xf
#define SST_TEX_TF_CLK_DEL_ADJ_DEFAULT_R11  0x6
#define SST_TEX_TF_CLK_DEL_ADJ_DEFAULT_R20  0xf
#define SST_TEX_TF_CLK_DEL_ADJ_DEFAULT_R21  0x5

/* FT Clock delays for (FBI Revision, TMU Revision) */
#define SST_FT_CLK_DEL_ADJ_DEFAULT_R10  0xa
#define SST_FT_CLK_DEL_ADJ_DEFAULT_R11  0xa
#define SST_FT_CLK_DEL_ADJ_DEFAULT_R20  0x5
#define SST_FT_CLK_DEL_ADJ_DEFAULT_R21  0x5

#define SST_TREXINIT1_DEFAULT \
  ( (SST_TEX_SCANLINE_INTERLEAVE_MASTER) \
  | (0 & SST_EN_TEX_SCANLINE_INTERLEAVE) \
  | (0x8 << SST_TEX_FT_FIFO_SIL_SHIFT) \
  | (0x8 << SST_TEX_TT_FIFO_SIL_SHIFT) \
  | (0xf << SST_TEX_TF_CLK_DEL_ADJ_SHIFT) \
  | (0 & SST_TEX_RG_TTCII_INH) \
  | (0 & SST_TEX_USE_RG_TTCII_INH) \
  | (0 & SST_TEX_SEND_CONFIG) \
  | (0 & SST_TEX_RESET_FIFO) \
  | (0 & SST_TEX_RESET_GRX) \
  | (0 << SST_TEX_PALETTE_DEL_SHIFT) \
  | (0 << SST_TEX_SEND_CONFIG_SEL_SHIFT) )

#define SST_TREX0INIT1_DEFAULT              SST_TREXINIT1_DEFAULT
#define SST_TREX1INIT1_DEFAULT              SST_TREXINIT1_DEFAULT
#define SST_TREX2INIT1_DEFAULT              SST_TREXINIT1_DEFAULT

#endif /* !__INIT_H__ */
