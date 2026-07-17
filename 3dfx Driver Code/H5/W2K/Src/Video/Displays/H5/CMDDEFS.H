  #ifndef __CMDDEFS_H__
  #define __CMDDEFS_H__
 
  #define BIT(n)              (1UL<<(n))

  // These are used to select a register mask for use with
  // command fifo packet type 4.  The defines represent an
  // offset register from the base register.
  // e.g. R5 = baseReg + 5, R0 = baseReg etc..
  #define R0  (1 << 0)
  #define R1  (1 << 1)
  #define R2  (1 << 2)
  #define R3  (1 << 3)
  #define R4  (1 << 4)
  #define R5  (1 << 5)
  #define R6  (1 << 6)
  #define R7  (1 << 7)
  #define R8  (1 << 8)
  #define R9  (1 << 9)
  #define R10 (1 << 10)
  #define R11 (1 << 11)
  #define R12 (1 << 12)
  #define R13 (1 << 13)
  #define R14 (1 << 14)
  #define R15 (1 << 15)
  #define R16 (1 << 16)
  #define R17 (1 << 17)
  #define R18 (1 << 18)
  #define R19 (1 << 19)
  #define R20 (1 << 20)
  #define R21 (1 << 21)
  #define R22 (1 << 22)
  #define R23 (1 << 23)
  #define R24 (1 << 24)
  #define R25 (1 << 25)
  #define R26 (1 << 26)
  #define R27 (1 << 27)
  #define R28 (1 << 28)
  #define R29 (1 << 29)
  #define R30 (1 << 30)
  #define R31 (1 << 31)

  // These are used to select a register mask for use with
  // command fifo packet type 2.  The defines represent an
  // offset register from the base register.
  #define clip0minBit         BIT(  3 )
  #define clip0maxBit         BIT(  4 )
  #define dstBaseAddrBit      BIT(  5 )
  #define dstFormatBit        BIT(  6 )
  #define srcColorkeyMinBit   BIT(  7 )
  #define srcColorkeyMaxBit   BIT(  8 )
  #define dstColorkeyMinBit   BIT(  9 )
  #define dstColorkeyMaxBit   BIT( 10 )
  #define bresError0Bit       BIT( 11 )
  #define bresError1Bit       BIT( 12 )
  #define ropBit              BIT( 13 )
  #define srcBaseAddrBit      BIT( 14 )
  #define commandExBit        BIT( 15 )
  #define lineStippleBit      BIT( 16 )
  #define lineStyleBit        BIT( 17 )
  #define pattern0aliasBit    BIT( 18 )
  #define pattern1aliasBit    BIT( 19 )
  #define clip1minBit         BIT( 20 )
  #define clip1maxBit         BIT( 21 )
  #define srcFormatBit        BIT( 22 )
  #define srcSizeBit          BIT( 23 )
  #define srcXYBit            BIT( 24 )
  #define colorBackBit        BIT( 25 )
  #define colorForeBit        BIT( 26 )
  #define dstSizeBit          BIT( 27 )
  #define dstXYBit            BIT( 28 )
  #define commandBit          BIT( 29 )

#endif // __CMDDEFS_H_
