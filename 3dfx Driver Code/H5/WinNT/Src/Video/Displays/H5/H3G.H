/*==========================================================================;
 *
 *
 *  File:       h3g.h
 *  Content:    header for a Windows NT display driver
 *
 *
 *
 ***************************************************************************/

extern volatile ULONG   p6fenceLoc;
extern LONG             prologEpilogPairing;

#define FIELDOFFSET(s,f)     (unsigned long)(& (((s *)0)->f))

#define H3_2DREG_OFFSET(reg) ( FIELDOFFSET(SstGRegs, reg) )

#define P6_FENCE 	{ __asm xchg eax, p6fenceLoc }
#define P6FENCE   P6_FENCE

#ifdef A0_SILICON
   #define START_DIRECT_ACCESS_H3(ppdev,pjH3Base) P6_FENCE; H3_GP_WAIT(ppdev,ppdev->pjBase)
#else
   #define START_DIRECT_ACCESS_H3(ppdev,pjH3Base) P6_FENCE; H3_GP_WAIT(ppdev,pjH3Base)
#endif

#define END_DIRECT_ACCESS_H3(ppdev, pjH3Base)

#ifdef CSIM
#define CP_BEGIN(startOffset) \
	(0x10000000L + SST_RAW_LFB_OFFSET + (startOffset))
#endif

#ifdef CSIM

// write reg.
extern void halStore32( volatile void *addr, FxU32 DATA );

// read from reg
extern FxU32 halLoad32( volatile void *addr );

// numByte:  size of framebuf
// *memory:  address of framebuf
// *hw    :  address of register space.
//		 	 hard wired to 0x10000000 for simulator
//extern void (*lpCsimInitDriver)(FxU32 numBytes,
//					volatile FxU32 *memory,
//					volatile FxU32 *hw);
#endif


#ifdef CSIM                 // CSIM macros
        #define GET2(s)          halLoad32(&(s))
        #define SET2(d,s)        halStore32(&(d),s)
#else                           // REAL hw
        #define GET2(s)     s
// since GWH_BEGIN_PKT1_PACKET & GWH_BEGIN_PKT5_PACKET use a *fifoPtr++ as the
// d arg to SET2, I can't put an H3PRINTF in the SET2 macro otherwise fifoPtr might
// be incremented more than once
//#if ENABLE_LOG_FILE
//        #define SET2(d,s) d = s; H3PRINTF((ppdev, "  %08lX = %08lX\r\n", d, s))
//#else
        #define SET2(d,s)   d = s
//#endif
#endif

#ifdef H3_FIFO

#define PKT_DBG_LVL         7

// packet 1 defines
#define SSTCP_PKT1_NOINC    0
#define SSTCP_PKT1_INC      BIT(15)
#define SSTCP_PKT1_CLIP0MIN        (SSTCP_PKT1_2D | (0x02 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_CLIP0MAX        (SSTCP_PKT1_2D | (0x03 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_DSTBASEADDR     (SSTCP_PKT1_2D | (0x04 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_DSTFORMAT       (SSTCP_PKT1_2D | (0x05 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_SRCCOLORKEYMIN  (SSTCP_PKT1_2D | (0x06 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_SRCCOLORKEYMAX  (SSTCP_PKT1_2D | (0x07 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_DSTCOLORKEYMIN  (SSTCP_PKT1_2D | (0x08 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_DSTCOLORKEYMAX  (SSTCP_PKT1_2D | (0x09 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_BRESERROR0      (SSTCP_PKT1_2D | (0x0a << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_BRESERROR1      (SSTCP_PKT1_2D | (0x0b << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_ROP             (SSTCP_PKT1_2D | (0x0c << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_SRCBASEADDR     (SSTCP_PKT1_2D | (0x0d << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_COMMANDEXTRA    (SSTCP_PKT1_2D | (0x0e << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_LINESTIPPLE     (SSTCP_PKT1_2D | (0x0f << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_LINESTYLE       (SSTCP_PKT1_2D | (0x10 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_PATTERN0ALIAS   (SSTCP_PKT1_2D | (0x11 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_PATTERN1ALIAS   (SSTCP_PKT1_2D | (0x12 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_CLIP1MIN        (SSTCP_PKT1_2D | (0x13 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_CLIP1MAX        (SSTCP_PKT1_2D | (0x14 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_SRCFORMAT       (SSTCP_PKT1_2D | (0x15 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_SRCSIZE         (SSTCP_PKT1_2D | (0x16 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_SRCXY           (SSTCP_PKT1_2D | (0x17 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_COLORBACK       (SSTCP_PKT1_2D | (0x18 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_COLORFORE       (SSTCP_PKT1_2D | (0x19 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_DSTSIZE         (SSTCP_PKT1_2D | (0x1a << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_DSTXY           (SSTCP_PKT1_2D | (0x1b << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_COMMAND         (SSTCP_PKT1_2D | (0x1c << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_LAUNCH          (SSTCP_PKT1_2D | (0x20 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_COLORPATTERN    (SSTCP_PKT1_2D | (0x40 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT1_OFFSETTOREG(offset) \
    (SSTCP_PKT1_2D | (((offset) >> 2) << SSTCP_REGBASE_SHIFT))


// packet 2 defines
#define SSTCP_PKT2_CLIP0MIN         0x00000001L
#define SSTCP_PKT2_CLIP0MAX         0x00000002L
#define SSTCP_PKT2_DSTBASEADDR      0x00000004L
#define SSTCP_PKT2_DSTFORMAT        0x00000008L
#define SSTCP_PKT2_SRCCOLORKEYMIN   0x00000010L
#define SSTCP_PKT2_SRCCOLORKEYMAX   0x00000020L
#define SSTCP_PKT2_DSTCOLORKEYMIN   0x00000040L
#define SSTCP_PKT2_DSTCOLORKEYMAX   0x00000080L
#define SSTCP_PKT2_BRESERROR0       0x00000100L
#define SSTCP_PKT2_BRESERROR1       0x00000200L
#define SSTCP_PKT2_ROP              0x00000400L
#define SSTCP_PKT2_SRCBASEADDR      0x00000800L
#define SSTCP_PKT2_COMMANDEXTRA     0x00001000L
#define SSTCP_PKT2_LINESTIPPLE      0x00002000L
#define SSTCP_PKT2_LINESTYLE        0x00004000L
#define SSTCP_PKT2_PATTERN0ALIAS    0x00008000L
#define SSTCP_PKT2_PATTERN1ALIAS    0x00010000L
#define SSTCP_PKT2_CLIP1MIN         0x00020000L
#define SSTCP_PKT2_CLIP1MAX         0x00040000L
#define SSTCP_PKT2_SRCFORMAT        0x00080000L
#define SSTCP_PKT2_SRCSIZE          0x00100000L
#define SSTCP_PKT2_SRCXY            0x00200000L
#define SSTCP_PKT2_COLORBACK        0x00400000L
#define SSTCP_PKT2_COLORFORE        0x00800000L
#define SSTCP_PKT2_DSTSIZE          0x01000000L
#define SSTCP_PKT2_DSTXY            0x02000000L
#define SSTCP_PKT2_COMMAND          0x04000000L


// 2d packet 4 defines

#define SSTCP_PKT4_CLIP0MIN        (SSTCP_PKT4_2D | (0x02 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_CLIP0MAX        (SSTCP_PKT4_2D | (0x03 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_DSTBASEADDR     (SSTCP_PKT4_2D | (0x04 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_DSTFORMAT       (SSTCP_PKT4_2D | (0x05 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_SRCCOLORKEYMIN  (SSTCP_PKT4_2D | (0x06 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_SRCCOLORKEYMAX  (SSTCP_PKT4_2D | (0x07 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_DSTCOLORKEYMIN  (SSTCP_PKT4_2D | (0x08 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_DSTCOLORKEYMAX  (SSTCP_PKT4_2D | (0x09 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_BRESERROR0      (SSTCP_PKT4_2D | (0x0a << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_BRESERROR1      (SSTCP_PKT4_2D | (0x0b << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_ROP             (SSTCP_PKT4_2D | (0x0c << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_SRCBASEADDR     (SSTCP_PKT4_2D | (0x0d << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_COMMANDEXTRA    (SSTCP_PKT4_2D | (0x0e << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_LINESTIPPLE     (SSTCP_PKT4_2D | (0x0f << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_LINESTYLE       (SSTCP_PKT4_2D | (0x10 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_PATTERN0ALIAS   (SSTCP_PKT4_2D | (0x11 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_PATTERN1ALIAS   (SSTCP_PKT4_2D | (0x12 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_CLIP1MIN        (SSTCP_PKT4_2D | (0x13 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_CLIP1MAX        (SSTCP_PKT4_2D | (0x14 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_SRCFORMAT       (SSTCP_PKT4_2D | (0x15 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_SRCSIZE         (SSTCP_PKT4_2D | (0x16 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_SRCXY           (SSTCP_PKT4_2D | (0x17 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_COLORBACK       (SSTCP_PKT4_2D | (0x18 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_COLORFORE       (SSTCP_PKT4_2D | (0x19 << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_DSTSIZE         (SSTCP_PKT4_2D | (0x1a << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_DSTXY           (SSTCP_PKT4_2D | (0x1b << SSTCP_REGBASE_SHIFT))
#define SSTCP_PKT4_COMMAND         (SSTCP_PKT4_2D | (0x1c << SSTCP_REGBASE_SHIFT))


// 3d packet 4 defines

#define SSTCP_PKT4_NOPCMD          ((0x48 << SSTCP_REGBASE_SHIFT))

   // Use GWH_DECL at top of functions that will use GWH packets.

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
# if DBG
   #define GWH_DECL      \
      ULONG*   fifoPtr;  \
      CmdFifo *fifo;     \
      ULONG    pktType;  \
      ULONG    regMask;  \
      ULONG    roomCount;
# else
   #define GWH_DECL      \
      ULONG*   fifoPtr;  \
      CmdFifo *fifo;
# endif  // DBG
#else
# if DBG
   #define GWH_DECL      \
      ULONG*   fifoPtr;  \
      ULONG    pktType;  \
      ULONG    regMask;  \
      ULONG    roomCount;
# else
   #define GWH_DECL      \
      ULONG*   fifoPtr;
# endif  // DBG
#endif

   /* Use GWH_PROLOG at top of functions that will use GWH packets,
    * but after ppdev has been initialized.  Also use it after return
    * from a subroutine call that uses GWH_PACKETS.
    */

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
   #define GWH_PROLOG \
   { \
      fifoPtr = ppdev->fifoData.fifoPtr;   \
      fifo = (CmdFifo *) ppdev->pjFifoBase; \
      ASSERTDD( prologEpilogPairing++ == 0, "PROLOG/EPILOG mismatch\n"); \
      H3PRINTF((ppdev, "  GWH_Prolog fifoPtr = %08lX\r\n", fifoPtr)); \
   }
#else
   #define GWH_PROLOG \
   { \
      fifoPtr = ppdev->fifoData.fifoPtr;   \
      ASSERTDD( prologEpilogPairing++ == 0, "PROLOG/EPILOG mismatch\n"); \
      H3PRINTF((ppdev, "  GWH_Prolog fifoPtr = %08lX\r\n", fifoPtr)); \
   }
#endif

   /* Use GWH_EPILOG before exit from functions using GWH packets
    * ( including immediately before a subroutine call that uses
    * GWH packets).
    */

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
   #define GWH_EPILOG \
   { \
      ppdev->fifoData.fifoPtr = fifoPtr; \
      if ( ppdev->doAGPFifo == TRUE ) \
      { \
         NTAGPEPILOG(ppdev, (DWORD) fifoPtr); \
      } \
      ASSERTDD( prologEpilogPairing-- == 1, "PROLOG/EPILOG mismatch\n"); \
      H3PRINTF((ppdev, "  GWH_Epilog fifoPtr = %08lX\r\n", fifoPtr)); \
   }
#else
   #define GWH_EPILOG \
   { \
      ppdev->fifoData.fifoPtr = fifoPtr;   \
      ASSERTDD( prologEpilogPairing-- == 1, "PROLOG/EPILOG mismatch\n"); \
      H3PRINTF((ppdev, "  GWH_Epilog fifoPtr = %08lX\r\n", fifoPtr)); \
   }
#endif


#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
   #define GWH_FIFO_JMP_PLUS4 \
   { \
      if ( ppdev->doAGPFifo == FALSE ) \
      { \
          H3MakeRoom( ppdev, &fifoPtr, 1 ); \
          H3PRINTF((ppdev, "  GWH_fifo_jmp_plus4, fifoPtr = %08lX, count = %ld\r\n", fifoPtr, 1)); \
          H3PRINTF((ppdev, "    %08lX = %08lX\r\n", fifoPtr, (SSTCP_PKT0_JMP_LOCAL | ( ((((BYTE *) fifoPtr) - ppdev->pjScreenBase) + 4) << (SSTCP_PKT0_ADDR_SHIFT - 2))))); \
          *fifoPtr = (SSTCP_PKT0_JMP_LOCAL | \
                     ( ((((BYTE *) fifoPtr) - ppdev->pjScreenBase) + 4) << (SSTCP_PKT0_ADDR_SHIFT - 2))); \
          fifoPtr++; \
        } \
      else \
      { \
          ULONG     newPhysReadPtr; \
                                    \
          H3MakeRoom( ppdev, &fifoPtr, 2 ); \
          H3PRINTF((ppdev, "  GWH_fifo_jmp_plus4, fifoPtr = %08lX, count = %ld\r\n", fifoPtr, 2)); \
          newPhysReadPtr = ppdev->fifoData.physBaseL + 8 + ( ((BYTE *) fifoPtr) - ppdev->fifoData.fifoStart); \
          *fifoPtr = (SSTCP_PKT0_JMP_AGP | \
                     ((newPhysReadPtr << (SSTCP_PKT0_ADDR_SHIFT - 2)) & SSTCP_PKT0_ADDR)); \
          *(fifoPtr + 1) = newPhysReadPtr >> 25; \
          fifoPtr += 2; \
      } \
   }
#else
   #define GWH_FIFO_JMP_PLUS4 \
   { \
          H3MakeRoom( ppdev, &fifoPtr, 1 ); \
          H3PRINTF((ppdev, "  GWH_fifo_jmp_plus4, fifoPtr = %08lX, count = %ld\r\n", fifoPtr, 1)); \
          H3PRINTF((ppdev, "    %08lX = %08lX\r\n", fifoPtr, (SSTCP_PKT0_JMP_LOCAL | ( ((((BYTE *) fifoPtr) - ppdev->pjScreenBase) + 4) << (SSTCP_PKT0_ADDR_SHIFT - 2))))); \
          *fifoPtr = (SSTCP_PKT0_JMP_LOCAL | \
                     ( ((((BYTE *) fifoPtr) - ppdev->pjScreenBase) + 4) << (SSTCP_PKT0_ADDR_SHIFT - 2))); \
          fifoPtr++; \
   }
#endif

    // Add one for header word.  Wrap reserves extra space.

   #define CHECK_FIFO_ROOM( ppdev, n ) \
   { \
          H3MakeRoom( ppdev, &fifoPtr, n+1 ); \
          H3PRINTF((ppdev, "  CheckFifoRoom, fifoPtr = %08lX, count = %ld\r\n", fifoPtr, n+1)); \
          SET_ROOM_COUNT(n); \
   }

   // For retail build everything falls out but the write to fifo memory.

   #define GWH_BEGIN_2D_PACKET(numregs, regmask) \
   {\
      ASSERTDD( (roomCount == numregs), "CHECK_FIFO_ROOM value does not match packet header" ); \
      GWH_RESET_WSH;\
      ASSERTDD(  !((FxU32)(fifoPtr) & 0x3), "Fifo pointer misaligned" ); \
      DISPDBG((PKT_DBG_LVL,"Beginning 2D Packet"));\
      DISPDBG((PKT_DBG_LVL,"fifoPtr = 0x%x roomToReadPtr = 0x%x", fifoPtr, ppdev->fifoData.roomToReadPtr));\
      DISPDBG((PKT_DBG_LVL,"Mask = 0x%x", regmask));\
      GWH_2D_REG_COUNT( regmask, ppdev->fifoData.numRegsInPacket );\
      ASSERTDD( numregs == ppdev->fifoData.numRegsInPacket, "GWH - User specified register count not equal to actual count" );\
      GWH_SAVE_DEBUG_INFO( 2, regmask );\
      ASSERTDD( (((regmask) << SSTCP_PKT2_MASK_SHIFT) & SSTCP_PKT2_MASK), "  Error - Register mask == 0" );\
      H3PRINTF((ppdev, "  GWH_begin_2d_packet\r\n")); \
      H3PRINTF((ppdev, "    %08lX = %08lX\r\n", fifoPtr, ((regmask) << SSTCP_PKT2_MASK_SHIFT) | SSTCP_PKT2)); \
      SET2( *fifoPtr, ((regmask) << SSTCP_PKT2_MASK_SHIFT) | SSTCP_PKT2 );\
   }

   #define GWH_BEGIN_PKT1_PACKET(num, base, inctype) \
   {\
      ASSERTDD( (roomCount == (ULONG)(num)), "CHECK_FIFO_ROOM value does not match packet header" ); \
      GWH_RESET_WSH;\
      ASSERTDD(  !((FxU32)(fifoPtr) & 0x3), "Fifo pointer misaligned" ); \
      DISPDBG((PKT_DBG_LVL,"Beginning Type 1 Packet")); \
      DISPDBG((PKT_DBG_LVL," fifoPtr = 0x%x roomToReadPtr = 0x%x count = 0x%x", fifoPtr, ppdev->fifoData.roomToReadPtr, (num)));\
      ASSERTDD( (num) < (2L<<16), "Too many words in packet" ); \
      GWH_PKT1_COUNT( num, ppdev->fifoData.numRegsInPacket ); \
      H3PRINTF((ppdev, "  GWH_begin_pkt1_packet\r\n")); \
      H3PRINTF((ppdev, "    %08lX = %08lX\r\n", fifoPtr, ((num) << SSTCP_PKT1_NWORDS_SHIFT) | (base | inctype | SSTCP_PKT1))); \
      SET2( *fifoPtr++, ((num) << SSTCP_PKT1_NWORDS_SHIFT) | (base | inctype | SSTCP_PKT1) );\
   }

   #define GWH_BEGIN_PKT4_PACKET(num, base, mask) \
   {\
      ASSERTDD( (roomCount == num), "CHECK_FIFO_ROOM value does not match packet header" ); \
      GWH_RESET_WSH;\
      ASSERTDD( !((FxU32)(fifoPtr) & 0x3), "Fifo pointer misaligned" ); \
      DISPDBG((PKT_DBG_LVL,"Beginning Type 4 Packet")); \
      DISPDBG((PKT_DBG_LVL," fifoPtr = 0x%x roomToReadPtr = 0x%x count = 0x%x", fifoPtr, ppdev->fifoData.roomToReadPtr, (num)));\
      ASSERTDD( ((num) > 0) && ((num) < 15), " *** Packet 4 data count mismatch" ); \
      GWH_PKT4_COUNT( num, ppdev->fifoData.numRegsInPacket ); \
      GWH_SAVE_DEBUG_INFO( 4, (mask) );\
      H3PRINTF((ppdev, "  GWH_begin_pkt4_packet\r\n")); \
      H3PRINTF((ppdev, "    %08lX = %08lX\r\n", fifoPtr, (base) | ((mask) << SSTCP_PKT4_MASK_SHIFT) | SSTCP_PKT4)); \
      SET2( *fifoPtr, (base) | ((mask) << SSTCP_PKT4_MASK_SHIFT) | SSTCP_PKT4 );\
   }

   // This packet 5 macro only handles full byte enables for performance.
   // Add another macro if we need enables.
   // Packet 5 takes 2 header words instead of 1 so adjust room check.

   #define GWH_BEGIN_PKT5_PACKET(num, base) \
   {\
      ASSERTDD( (roomCount == (ULONG)(num)+1), "CHECK_FIFO_ROOM value does not match packet header" ); \
      GWH_RESET_WSH;\
      ASSERTDD(  !((FxU32)(fifoPtr) & 0x3), "Fifo pointer misaligned" ); \
      DISPDBG((PKT_DBG_LVL,"Beginning Type 5 Packet")); \
      DISPDBG((PKT_DBG_LVL," fifoPtr = 0x%x roomToReadPtr = 0x%x count = 0x%x", fifoPtr, ppdev->fifoData.roomToReadPtr, (num)));\
      ASSERTDD( (num) < (HW_CMDFIFO_TOTAL_SIZE/4-H3_FIFO_END_ADJUST), "Too many words in packet" ); \
      GWH_PKT5_COUNT( num, ppdev->fifoData.numRegsInPacket ); \
      H3PRINTF((ppdev, "  GWH_begin_pkt5_packet\r\n")); \
      H3PRINTF((ppdev, "    %08lX = %08lX\r\n", fifoPtr, ((num << SSTCP_PKT5_NWORDS_SHIFT) & SSTCP_PKT5_NWORDS ) | SSTCP_PKT5)); \
      SET2( *fifoPtr++, ((num << SSTCP_PKT5_NWORDS_SHIFT) & SSTCP_PKT5_NWORDS ) | SSTCP_PKT5 );\
      H3PRINTF((ppdev, "    %08lX = %08lX\r\n", fifoPtr, (base))); \
      SET2( *fifoPtr++, (base) );\
   }

#else

   #define GWH_DECL
   #define GWH_PROLOG
   #define GWH_EPILOG
   #define GWH_BEGIN_2D_PACKET(numregs, regmask)
   #define GWH_BEGIN_PKT1_PACKET(num, base, inctype)
   #define GWH_BEGIN_PKT4_PACKET(num, base, mask)
   #define GWH_BEGIN_PKT5_PACKET(num, base)
   #define CHECK_FIFO_ROOM( ppdev, n )

#endif

#if ( DBG == 1 ) && defined ( H3_FIFO )
      #define GWH_RESET_WSH ppdev->fifoData.writesSinceHeader = 0
      #define GWH_INC_WSH ppdev->fifoData.writesSinceHeader++
      #define GWH_2D_REG_COUNT(mask, count)   \
      {                                       \
         LONG	i, localcount;                \
         LONG   localmask = mask;             \
                                              \
         localcount = 0;                      \
         for( i = 28; i >= 0 ; i-- )          \
         {                                    \
            if( localmask & 1 )               \
               localcount++;                  \
            localmask >>= 1;                  \
         }                                    \
         count = localcount;                  \
      }

      #define GWH_PKT1_COUNT(num, loc)        \
      {                                       \
         loc = num;                           \
      }

      #define GWH_PKT5_COUNT(num, loc) GWH_PKT1_COUNT(num, loc)

      #define GWH_PKT4_COUNT(num, loc)  GWH_PKT1_COUNT(num, loc)

      #define GWH_SAVE_DEBUG_INFO( localType, mask ) \
      { \
         pktType = (localType); \
         regMask = (mask); \
      }

      #define GWH_RESET_DEBUG_INFO \
      { \
         pktType = 0; \
         regMask = 0; \
      }


      #define GWH_CHECK_ORDER(reg, slot)                   \
      {                                                    \
         ULONG   regnum;                                   \
         ULONG    i, m;                                    \
                                                           \
         if( (slot) != ppdev->fifoData.writesSinceHeader ) \
            RIP( "Banshee fifo written out of order." );   \
                                                           \
         if( pktType == 2 )                                \
         {                                                 \
            regnum = (FIELDOFFSET(SstGRegs,reg) >> 2) - 2; \
                                                           \
            for( i = 0, m = 1 ; i < 29; i++, m <<= 1 )     \
               if( regMask & m )                           \
                  break;                                   \
                                                           \
            if( i < 29 )                                   \
            {                                              \
               if( i != regnum )                           \
                   RIP( "Register written out of order." );\
               regMask &= ~BIT(i);                         \
            }                                              \
            else                                           \
               RIP( "Unexpected register SET." );          \
                                                           \
         }                                                 \
      }


      #define SET_ROOM_COUNT(n) \
      { \
          roomCount = n; \
      }


#  else
      #define GWH_RESET_WSH
      #define GWH_INC_WSH
      #define GWH_2D_REG_COUNT(mask, count)
      #define GWH_PKT1_COUNT(num, loc)
      #define GWH_PKT4_COUNT(num, loc)
      #define GWH_PKT5_COUNT(num, loc)
      #define GWH_SAVE_DEBUG_INFO(localType, mask)
      #define GWH_RESET_DEBUG_INFO
      #define GWH_CHECK_ORDER(reg, slot)
      #define SET_ROOM_COUNT(n)
#  endif

#ifdef DBG
    #define DBG_ZERO_ROOMCOUNT roomCount = 0;
#else
    #define DBG_ZERO_ROOMCOUNT
#endif

#ifdef H3_FIFO
      // Slots here should match slots on begin packet.

      #define GWH_END_2D_PACKET( slots ) \
      { \
         ASSERTDD( ppdev->fifoData.writesSinceHeader == ppdev->fifoData.numRegsInPacket, \
                   " *** Passed packet size != actual" ); \
         GWH_RESET_DEBUG_INFO; \
         fifoPtr += (slots + 1); \
         DISPDBG((PKT_DBG_LVL,"End 2D Packet"));\
         H3PRINTF((ppdev, "  GWH_end_2d_packet\r\n")); \
         DBG_ZERO_ROOMCOUNT; \
      }

      // fifoPtr has already been bumped in SET_XXX
      #define GWH_END_PKT1_PACKET( num ) \
      { \
         ASSERTDD( ppdev->fifoData.writesSinceHeader == ppdev->fifoData.numRegsInPacket, \
                   " *** Passed packet size != actual" ); \
         GWH_RESET_DEBUG_INFO; \
         DISPDBG((PKT_DBG_LVL,"End type 1 Packet"));\
         H3PRINTF((ppdev, "  GWH_end_pkt1_packet\r\n")); \
         DBG_ZERO_ROOMCOUNT; \
      }

      #define GWH_END_PKT4_PACKET(slots) \
      { \
         ASSERTDD( ppdev->fifoData.writesSinceHeader == ppdev->fifoData.numRegsInPacket, \
                   " *** Passed packet size != actual" ); \
         GWH_RESET_DEBUG_INFO; \
         fifoPtr += (slots + 1); \
         DISPDBG((PKT_DBG_LVL,"End type 4 Packet"));\
         H3PRINTF((ppdev, "  GWH_end_pkt4_packet\r\n")); \
         DBG_ZERO_ROOMCOUNT; \
      }

      // fifoPtr has already been bumped in SET_XXX
      #define GWH_END_PKT5_PACKET( num ) \
      { \
         ASSERTDD( ppdev->fifoData.writesSinceHeader == ppdev->fifoData.numRegsInPacket, \
                   " *** Passed packet size != actual" ); \
         GWH_RESET_DEBUG_INFO; \
         DISPDBG((PKT_DBG_LVL,"End type 5 Packet"));\
         H3PRINTF((ppdev, "  GWH_end_pkt5_packet\r\n")); \
         DBG_ZERO_ROOMCOUNT; \
      }

#else
      #define GWH_END_2D_PACKET(slots)
      #define GWH_END_PKT1_PACKET(num)
      #define GWH_END_PKT4_PACKET(slots)
      #define GWH_END_PKT5_PACKET(num)
#endif

#undef SET
#undef GET


#ifndef CSIM

	// DW = Direct Writes - not through command fifo.

   #define SET_DW(phw,reg,val) \
   { \
        *(volatile ULONG *)((ULONG)(phw)+FIELDOFFSET(SstGRegs, reg)) = val;  \
   }
   //H3PRINTF((ppdev, "  SET_DW  %s = %08lX\r\n", #reg, val)); \

   #define SET_ABSOLUTE_DW( addr, val ) \
   { \
        *(volatile ULONG *)(addr) = val; \
   }
   //H3PRINTF((ppdev, "  SET_ABS_DW  %08lX = %08lX\r\n", addr, val)); \

// moved GET define to ddfxnt95.h
// this style of GET will no longer work
//   #define GET(phw, offset)                                       \
//      ( *(volatile ULONG *) ((ULONG)phw + (offset)) )

   #define GET_ABSOLUTE( addr )                                   \
     ( *(volatile ULONG *) (addr) )

#  ifdef H3_FIFO

     /* Register doesn't matter for command fifo because we are shoving values
      * incrementally into the command fifo space, not the register space.
      * You better have used the GWH macros to declare and set fifoPtr or
		* you will be hosed.  You also need to have called CHECK_FIFO_ROOM
		* to verify there is enough space in the fifo.
		*
		* We have unused parameters that will be used if compiled without
		* command fifo support.
		*/

     # define SET(slot, addr, reg, val)                             \
     {                                                              \
        GWH_INC_WSH;                                                \
        GWH_CHECK_ORDER(reg, slot);                                 \
        fifoPtr[slot] = val;                                        \
     }
     //H3PRINTF((ppdev, "    slot %ld = %08lX (%s)\r\n", slot, val, #reg)); \

     # define SET3D(slot, reg, val)                                 \
     {                                                              \
        GWH_INC_WSH;                                                \
        fifoPtr[slot] = val;                                        \
     }

     #define SET_ABSOLUTE( addr, val )                              \
     {                                                              \
        GWH_INC_WSH;                                                \
        *fifoPtr++ = val;                                           \
     }
     //H3PRINTF((ppdev, "    SET_ABS  %08lX = %08lX\r\n", fifoPtr, val)); \

     #define SET_PKT1_REG( phw, addr, val )                         \
     {                                                              \
        GWH_INC_WSH;                                                \
        *fifoPtr++ = val;                                           \
     }
     //H3PRINTF((ppdev, "    SET_PKT1 %08lX = %08lX\r\n", fifoPtr, val)); \

#  else

     # define SET(slot, phw, reg, val)                              \
     {                                                              \
        GWH_INC_WSH;                                                \
        *(volatile ULONG *)((ULONG)(phw)+FIELDOFFSET(SstGRegs, reg)) = val;  \
     }

     # define SET3D(slot, reg, val)                                 \
     {                                                              \
        GWH_INC_WSH;                                                \
        (volatile ULONG)(reg) = val;                                \
     }

     #define SET_ABSOLUTE(addr, val )                               \
     {                                                              \
        GWH_INC_WSH;                                                \
        *(volatile ULONG *)addr = val;                              \
     }

     # define SET_PKT1_REG(phw, reg, val)                           \
     {                                                              \
        *(volatile ULONG *)((ULONG)(phw)+FIELDOFFSET(SstGRegs, reg)) = val;  \
     }

#  endif

#else   // CSIM

	// DW = Direct Writes - not through command fifo.

   #define SET_DW(phw,reg,val)                                     \
      halStore32((void *) ((ULONG)(phw) + FIELDOFFSET(SstGRegs, reg)), (val))

   #define SET_ABSOLUTE_DW( addr, val )                            \
      halStore32( (void *) (addr), (val) )

// moved GET define to ddfxnt95.h
// this style of GET will no longer work
//   #define GET(phw,reg)                                         \
//      halLoad32( (void *) ((ULONG)(phw) + (reg)) )

   #define GET_ABSOLUTE( addr )                                 \
      halLoad32( (void *) (addr) )


#  ifdef H3_FIFO

   #define SET(slot,phw,reg,val)                                   \
   {                                                               \
      GWH_INC_WSH;                                                 \
      GWH_CHECK_ORDER(reg, slot);                                  \
      halStore32((void *)(fifoPtr + (slot)), (val)); \
   }

   #define SET3D(slot, reg, val)                                   \
   {                                                               \
      GWH_INC_WSH;                                                 \
      halStore32((void *)(fifoPtr + (slot)), (val)); \
   }

   // Pass address of register.  Used for SimDb and pattern load.
   #define SET_ABSOLUTE( addr, val )                               \
   {                                                               \
      GWH_INC_WSH;                                                 \
      halStore32((void *)(fifoPtr++), (val)); \
   }

   #define SET_PKT1_REG(phw, reg, val)                             \
   {                                                               \
      GWH_INC_WSH;                                                 \
      halStore32((void *)(fifoPtr++), (val)); \
   }

#  else
	// reg is field  of SstGRegs structure

   #define SET(slot,phw,reg,val)                                   \
   {                                                               \
      GWH_INC_WSH;                                                 \
      halStore32((void *) ((ULONG)(phw) + FIELDOFFSET(SstGRegs, reg)), (val)); \
   }

   #define SET3D(slot, reg, val)                                   \
   {                                                               \
      GWH_INC_WSH;                                                 \
      halStore32( (void *) (reg), (val) );                         \
   }

   // Pass address of register.  Used for SimDb and pattern load.
   #define SET_ABSOLUTE( addr, val )                               \
   {                                                               \
      GWH_INC_WSH;                                                 \
      halStore32( (void *) (addr), (val) );                        \
   }

   #define SET_PKT1_REG(phw,reg,val)                               \
   {                                                               \
      GWH_INC_WSH;                                                 \
      halStore32((void *) ((ULONG)(phw) + FIELDOFFSET(SstGRegs, reg)), (val)); \
   }

#  endif    // H3_FIFO
#endif  // CSIM
