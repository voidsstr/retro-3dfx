/*________________________________________________________________________________________
** 
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
**________________________________________________________________________________________
**
**
** 
**
*/

#ifndef _GLR_CMDFIFO_H_
#define _GLR_CMDFIFO_H_

#include <h3.h>

enum {
   kSetupStrip           = 0x00,
   kSetupFan             = 0x01,
   kSetupCullDisable     = 0x00,
   kSetupCullEnable      = 0x02,
   kSetupCullPositive    = 0x00,
   kSetupCullNegative    = 0x04,
   kSetupPingPongNorm    = 0x00,
   kSetupPingPongDisable = 0x08
};

#define SETUP_SIGNMODE signMode = (__cullStripHdr & (SST_SETUP_CULL_NEGATIVE << SSTCP_PKT3_PMASK_SHIFT)) ? 0x80000000 : 0x0
#define CULLING_DISABLED  !(__cullStripHdr & (SST_SETUP_EN_CULLING << SSTCP_PKT3_PMASK_SHIFT))
#define CULLING_ENABLED    (__cullStripHdr & (SST_SETUP_EN_CULLING << SSTCP_PKT3_PMASK_SHIFT))

#define PARAM_SETUP(__paramMask,__numParams) \
{ \
  FxU32 __cullStripHdr = inContext->cmdTransportInfo->cullStripHdr & ~SSTCP_PKT3_PMASK, signMode; \
  __cullStripHdr |= SSTCP_PKT3_PACKEDCOLOR | ((__paramMask) << SSTCP_PKT3_PMASK_SHIFT); \
  vSize = __numParams * sizeof(FxU32); \
  SETUP_SIGNMODE;

#define PARAM_END \
}

#define P6FENCE __sync()

#if __POWERPC__ && PCI_BUMP_N_GRIND
#define FIFO_CACHE_FLUSH(d)  __dcbf(d,-4)
#define GR_BUMP_N_GRIND \
do { \
  FIFO_CACHE_FLUSH(inContext->cmdTransportInfo->fifoPtr);\
  P6FENCE; \
  \
} while(0)

#define CHECK_FOR_BUMP \
if (!inContext->cmdTransportInfo->autoBump && \
    (inContext->cmdTransportInfo->fifoPtr > inContext->cmdTransportInfo->bumpPos)) { \
  grCommandTransportInfoExt(); \
  }
#else
#define GR_BUMP_N_GRIND
#define CHECK_FOR_BUMP
#define FIFO_CACHE_FLUSH(d)
#endif

#define CHECK_FOR_ROOM(__n) \
  do { \
    const FxU32 writeSize = (__n);            /* Adjust for size of hdrs */ \
    if (inContext->cmdTransportInfo->fifoRoom < (FxI32)writeSize) { \
       grCommandTransportMakeRoomExt(writeSize, __FILE__, __LINE__); \
    } \
  } while(0)

#if CMDFIFO_DEBUG
#define SET_EXPECTED_SIZE(__n) \
{ \
  FxU32 __expectedSize = __n; \
  FxU32 __realSize = 0; \
  CHECK_FOR_ROOM(__n);

#define TRI_SET(_val) \
  do { __realSize += 4; \
  SET_FIFO(*tPackPtr++, _val); \
  } while(0)

#define TRI_SETF_FAST(_val) \
  do { __realSize += 4; \
  SET_FIFO(*tPackPtr++, *(FxU32 *)&(_val)); \
  } while(0)

#define TRI_SETF(_val) \
  do {__realSize += 4; \
  SETF_FIFO(*tPackPtr++, (_val)); \
  } while(0)

#if STORE_64
#define TRI_SETPAIR_II(_val1, _val2) \
  do {__realSize += 8; \
  SET_FIFO(_buff.i[0], _val1); \
  SET_FIFO(_buff.i[1], _val2); \
  if((FxU32)tPackPtr & 7) { \
    glr_debug_printf("64-bit misalignment, file: %s line: %d\n",__FILE__,__LINE__); \
  } \
  *((double *)tPackPtr)++ = _buff.d; \
  } while(0)
  
#define TRI_SETPAIR_IF(_val1, _val2) \
  do {__realSize += 8; \
  SET_FIFO(_buff.i[0], _val1); \
  SET_FIFO(_buff.i[1], *(FxU32 *)&(_val2)); \
  if((FxU32)tPackPtr & 7) { \
    glr_debug_printf("64-bit misalignment, file: %s line: %d\n",__FILE__,__LINE__); \
  } \
  *((double *)tPackPtr)++ = _buff.d; \
  } while(0)

#define TRI_SETPAIR_FI(_val1, _val2) \
  do {__realSize += 8; \
  SET_FIFO(_buff.i[0], *(FxU32 *)&(_val1)); \
  SET_FIFO(_buff.i[1], _val2); \
  if((FxU32)tPackPtr & 7) { \
    glr_debug_printf("64-bit misalignment, file: %s line: %d\n",__FILE__,__LINE__); \
  } \
  *((double *)tPackPtr)++ = _buff.d; \
  } while(0)

#define TRI_SETPAIR_FF(_val1, _val2) \
  do {__realSize += 8; \
  SET_FIFO(_buff.i[0], *(FxU32 *)&(_val1)); \
  SET_FIFO(_buff.i[1], *(FxU32 *)&(_val2)); \
  if((FxU32)tPackPtr & 7) { \
    glr_debug_printf("64-bit misalignment, file: %s line: %d\n",__FILE__,__LINE__); \
  } \
  *((double *)tPackPtr)++ = _buff.d; \
  } while(0)

#else /* !STORE_64 */
#define TRI_SETPAIR_II(_val1, _val2) \
  TRI_SET(_val1); \
  TRI_SET(_val2);

#define TRI_SETPAIR_IF(_val1, _val2) \
  TRI_SET(_val1); \
  TRI_SETF_FAST(_val2);

#define TRI_SETPAIR_FI(_val1, _val2) \
  TRI_SETF_FAST(_val1); \
  TRI_SET(_val2);

#define TRI_SETPAIR_FF(_val1, _val2) \
  TRI_SETF_FAST(_val1); \
  TRI_SETF_FAST(_val2);
#endif /* !STORE_64 */

#define CHECK_SIZE \
  if(__expectedSize != __realSize) \
    glr_debug_printf("HEY!  Size mismatch %s line %d  Expected: %d, sent %d\n",__FILE__,__LINE__,__expectedSize,__realSize); \
  CHECK_FOR_BUMP; \
}

#else
#define SET_EXPECTED_SIZE(__n) \
  CHECK_FOR_ROOM(__n);

#define TRI_SET(_val) \
  SET_FIFO(*tPackPtr++, _val);

#define TRI_SETF_FAST(_val) \
  SET_FIFO(*tPackPtr++, *(FxU32 *)&(_val));

#define TRI_SETF(_val) \
  SETF_FIFO(*tPackPtr++, (_val));

#if STORE_64
#define TRI_SETPAIR_II(_val1, _val2) \
  SET_FIFO(_buff.i[0], _val1); \
  SET_FIFO(_buff.i[1], _val2); \
  *((double *)tPackPtr)++ = _buff.d;

#define TRI_SETPAIR_IF(_val1, _val2) \
  SET_FIFO(_buff.i[0], _val1); \
  SET_FIFO(_buff.i[1], *(FxU32 *)&(_val2)); \
  *((double *)tPackPtr)++ = _buff.d;

#define TRI_SETPAIR_FI(_val1, _val2) \
  SET_FIFO(_buff.i[0], *(FxU32 *)&(_val1)); \
  SET_FIFO(_buff.i[1], _val2); \
  *((double *)tPackPtr)++ = _buff.d;

#define TRI_SETPAIR_FF(_val1, _val2) \
  SET_FIFO(_buff.i[0], *(FxU32 *)&(_val1)); \
  SET_FIFO(_buff.i[1], *(FxU32 *)&(_val2)); \
  *((double *)tPackPtr)++ = _buff.d;
#else
#define TRI_SETPAIR_II(_val1, _val2) \
  TRI_SET(_val1); \
  TRI_SET(_val2);

#define TRI_SETPAIR_IF(_val1, _val2) \
  TRI_SET(_val1); \
  TRI_SETF_FAST(_val2);

#define TRI_SETPAIR_FI(_val1, _val2) \
  TRI_SETF_FAST(_val1); \
  TRI_SET(_val2);

#define TRI_SETPAIR_FF(_val1, _val2) \
  TRI_SETF_FAST(_val1); \
  TRI_SETF_FAST(_val2);
#endif  

#define CHECK_SIZE CHECK_FOR_BUMP

#endif

#define ALIGN_FIFO_16 \
    while((FxU32)inContext->cmdTransportInfo->fifoPtr & 15) {  \
      CHECK_FOR_ROOM(sizeof(FxU32)); \
      *inContext->cmdTransportInfo->fifoPtr++ = 0; \
      inContext->cmdTransportInfo->fifoRoom -= sizeof(FxU32); \
    }

#if STORE_64
#define ALIGN_FIFO \
    if((FxU32)inContext->cmdTransportInfo->fifoPtr & 7) {  \
      CHECK_FOR_ROOM(sizeof(FxU32)); \
      *inContext->cmdTransportInfo->fifoPtr++ = 0; \
      inContext->cmdTransportInfo->fifoRoom -= sizeof(FxU32); \
    }
#define STORE_BUFFER \
  union { \
    FxU32 i[2]; \
    double d; \
  } _buff;
       
#else
#define ALIGN_FIFO
#define STORE_BUFFER
#endif

#define TRI_STRIP_BEGIN(__setupMode, __nVertex, __vertexSize, __cmd) \
{ \
  FxU32* tPackPtr = inContext->cmdTransportInfo->fifoPtr; \
  const FxU32 packetVal = (((__setupMode) << SSTCP_PKT3_SMODE_SHIFT) |   /* [27:22] */ \
                           ((__nVertex) << SSTCP_PKT3_NUMVERTEX_SHIFT) | /* [9:6] */ \
                           (__cmd) |                                     /* command [5:3] */ \
                           __cullStripHdr); \
  TRI_SET(packetVal);

#define TRI_STRIP_BEGIN_NOHEADER(__setupMode, __nVertex, __vertexSize) \
{ \
  STORE_BUFFER \
  FxU32* tPackPtr = inContext->cmdTransportInfo->fifoPtr; \
  FxU32 packetVal = (((__setupMode) << SSTCP_PKT3_SMODE_SHIFT) |   /* [27:22] */ \
                           ((__nVertex) << SSTCP_PKT3_NUMVERTEX_SHIFT) | /* [9:6] */ \
                           __cullStripHdr);

#define TRI_STRIP_BEGIN_NOHEADER_G4(__setupMode, __nVertex, __vertexSize) \
{ \
  FxU32* tPackPtr = inContext->cmdTransportInfo->fifoPtr; \
  FxU32 packetVal = (((__setupMode) << SSTCP_PKT3_SMODE_SHIFT) |   /* [27:22] */ \
                           ((__nVertex) << SSTCP_PKT3_NUMVERTEX_SHIFT) | /* [9:6] */ \
                           __cullStripHdr);
  
#define TRI_END \
  inContext->cmdTransportInfo->fifoRoom -= ((FxU32)tPackPtr - (FxU32)inContext->cmdTransportInfo->fifoPtr); \
  inContext->cmdTransportInfo->fifoPtr = tPackPtr; \
}

#endif
