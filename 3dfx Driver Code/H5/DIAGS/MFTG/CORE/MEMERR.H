/*
 * memerr.h
 *
 */


typedef unsigned long int MAXERR;
typedef MAXERR ****MAXERR_ARRAY;

typedef struct {
  int addr_bits;      /* address bits in testing address space */
  int addr_cs_bits;   /* number of chip select bits used */
  
  MAXERR_ARRAY fail_map;
  MAXERR_ARRAY test_map;
  unsigned long int total_failures;
  unsigned long int total_tests;

} MEMERR_INFO;

/* test_bit: returns 1 if the bit position is 1, 0 if the bit position is 0
 *
 * pull_bits: returns the bit positions "start_bit" through "end_bit", shifted down to the LSBs
 */

#define test_bit(data,bit) ( (data & (1 << bit)) ? 1 : 0 )
#define pull_bits(data,start_bit,end_bit) ( (data & ((0xFFFFFFFF >> (31 - (end_bit-start_bit))) << start_bit)) >> start_bit)

MEMERR_INFO *allocMemErrInfo(int addr_bit_count,int addr_chip_select_bits);
void freeMemErrInfo(MEMERR_INFO *errmap);
void reportResultU8(MEMERR_INFO *info, FxU32 addr, FxU8 expect, FxU8 receive);
void reportResultU16(MEMERR_INFO *info, FxU32 addr, FxU16 expect, FxU8 receive);
void reportResultU32(MEMERR_INFO *info, FxU32 addr, FxU32 expect, FxU32 receive);
void printMemErrSummary(MEMERR_INFO *info);
void clearMemErrInfo(MEMERR_INFO *info);
