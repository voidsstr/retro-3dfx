/*
 * memerr.c
 *
 * record and report about memory errors
 *
 */
#include <stdlib.h>

#include "mdc.h"


#include "3dfx.h"

#include "memerr.h"

#define BITS_PER_BYTE   8   /* number of bits in a byte */
#define NSTATES         2   /* number of possible digital states (just 0 and 1) */


/* 
 * we are going to address a fail_map like this:
 *
 * num_failures = xxxxx_map[addr_bit][addr_bit_value][data_bit][data_bit_value]
 *
 */

MAXERR_ARRAY alloc_map(int addr_bit_count) {
  int astates_array_size = (sizeof(MAXERR *) * NSTATES);
  int addr_array_size = (sizeof(MAXERR *) * addr_bit_count);
  int data_array_size = (sizeof(MAXERR *) * BITS_PER_BYTE);
  int dstates_array_size = (sizeof(MAXERR) * NSTATES);
  int a,d,states; /* address line, data line, address states */
  MAXERR_ARRAY map;
  
  ASSERT(map = (MAXERR ****)malloc(addr_array_size), "couldn't allocate addr_array for _map");
  
  for ( a=0 ; a<addr_bit_count ; a++ ) {
    /* for each address */
    ASSERT(map[a] = (MAXERR ***)malloc(astates_array_size), 
	   "couldn't allocate addr_states for _map");
    
    for ( states=0; states<NSTATES ; states++) {
      ASSERT(map[a][states] = (MAXERR **)malloc(data_array_size), 
	     "couldn't allocate data_array for _map");
      
      for ( d=0 ; d<BITS_PER_BYTE ; d++ ) {
	ASSERT(map[a][states][d] = (MAXERR *)malloc(dstates_array_size),
	       "couldn't allocate data_states for _map");
      }
    }
  }
  return (map);
}

void free_map(MAXERR_ARRAY map, int addr_bit_count) {
  int a,d,states;

  if (!map) { return; }

  for (a=0 ; a<addr_bit_count ; a++) {
    for (states = 0; states < NSTATES ; states++) {
      for ( d=0 ; d<BITS_PER_BYTE ; d++ ) {
	free( map[a][states][d] );
      }
      free( map[a][states] );
    }
    free( map[a] );
  }
}

MEMERR_INFO *allocMemErrInfo(int addr_bit_count,int addr_chip_select_bits) {
  MEMERR_INFO *errmap = (MEMERR_INFO *)malloc(sizeof(MEMERR_INFO));
  
  ASSERT((addr_bit_count <= 32) && (addr_bit_count > 0),"Invalid addr_bit_count");
  ASSERT(errmap, "Unable to allocate errmap top");

  errmap->addr_bits = addr_bit_count;
  errmap->addr_cs_bits = addr_chip_select_bits;
  ASSERT(errmap->fail_map = alloc_map(addr_bit_count), "failed to alloc fail_map");
  ASSERT(errmap->test_map = alloc_map(addr_bit_count), "failed to alloc test_map");

  clearMemErrInfo(errmap);
  return (errmap);
}

void freeMemErrInfo(MEMERR_INFO *errmap) {
  if (!errmap) { return; }

  free_map(errmap->fail_map,errmap->addr_bits); errmap->fail_map = NULL;
  free_map(errmap->test_map,errmap->addr_bits); errmap->test_map = NULL;
}

void clearMemErrInfo(MEMERR_INFO *info) {
  int a,d,as,ds; /* address, data, address_states, data_states */

  if (!info) { return; }

  for ( a=0 ; a < info->addr_bits ; a++) {
    for ( as=0 ; as < NSTATES ; as++) {
      for ( d=0 ; d < BITS_PER_BYTE ; d++) {
	for ( ds=0 ; ds < NSTATES ; ds++) {
	  info->test_map[a][as][d][ds] = 0;
	  info->fail_map[a][as][d][ds] = 0;
	}
      }
    }
  }
  info->total_tests = 0;
  info->total_failures = 0;
}


int print_failures = 50;

void reportResultU8(MEMERR_INFO *info, FxU32 addr, FxU8 expect, FxU8 receive) {
  int a,d;

  if (!info) { return; }

  /* record test */

  info->total_tests++;  
  for ( a=0 ; a < info->addr_bits ; a++) {
    for ( d=0 ; d < BITS_PER_BYTE ; d++) {
      info->test_map[a][test_bit(addr,a)][d][test_bit(expect,a)]++;
    }
  }
  
  /* record failures */
  
  if (expect != receive) {
    info->total_failures++;
    if (print_failures) {
      printf("failed addr(0x%X), ex(0x%X), rec(0x%X)\n",addr,expect,receive);
      print_failures--;
    }
    
    for ( a=0 ; a < info->addr_bits ; a++) {
      for ( d=0 ; d < BITS_PER_BYTE ; d++) {
	if (test_bit(expect,d) != test_bit(receive,d)) {
	  info->fail_map[a][test_bit(addr,a)][d][test_bit(expect,a)]++;

	}
      }
    }
  } 
}

void reportResultU16(MEMERR_INFO *info, FxU32 addr, FxU16 expect, FxU16 receive) {

  if (!info) { return; }

  ASSERT((test_bit(addr,0) == 0), "non-aligned U16 result");

  reportResultU8(info,addr,(FxU8)(expect & 0xff), (FxU8)(receive & 0xff));
  reportResultU8(info,addr+1,(FxU8) ((expect >> BITS_PER_BYTE) & 0xFF), 
		 (FxU8)((receive >> BITS_PER_BYTE) & 0xFF));
}

void reportResultU32(MEMERR_INFO *info, FxU32 addr, FxU32 expect, FxU32 receive) {
  if (!info) { return; }
  ASSERT((test_bit(addr,1) == 0), "non-aligned U32 result");

  reportResultU16(info,addr,(FxU16)(expect & 0xFFFF), (FxU16)(receive & 0xFFFF));
  reportResultU16(info,addr+2,(FxU16)((expect >> (2*BITS_PER_BYTE)) & 0xFFFF),
		  (FxU16)((receive >> (2*BITS_PER_BYTE)) & 0xFFFF));
}



/*
 * void memerrFullSummary(MEMERR_INFO *info);
 *
 * this prints the full summary chart. However, this data is a bit hard to read so,
 * someone likely should look at the more specific summary results
 */

void memerrFullSummary(MEMERR_INFO *info) {
  int a,d,states,dstates;

  if (!info) { return; }

  printf("Total Tests: %d   Failures: %d\n",info->total_tests,info->total_failures);

 if (info->total_failures) {

  for (dstates=0; dstates < NSTATES ; dstates++) {
    printf("                <---  DataBit = %d  --->\n",dstates);
    for (states=0 ; states < NSTATES ; states++) {
      /* print row banner */
      printf("\nAddr=%d  ",states);
      for ( d=0 ; d<BITS_PER_BYTE ; d++) {
	printf("(%02d)    ",d);
      }
      
      for ( a=0; a < info->addr_bits ; a++) {
	if (states) {
	  printf("\n %02d]    ",a);
	} else {
	  printf("\n[%02d     ",a);
	}
	for ( d=0 ; d<BITS_PER_BYTE ; d++) {
	  FxU32 err_cnt = (info->fail_map[a][states][d][dstates]);
	  if (err_cnt) {
	    printf("%06d  ",err_cnt);
	  } else {
	    printf("--      ");
	  }
	}
      }
    }
  }
  printf("\n");
 }
}

/*
 * void memerrDataBitSummary(MEMERR_INFO *info)
 *
 * This looks at databit failures and tries to find out unifying patterns. 
 *
 * databit stuck fiailure 
 *
 */

void memerrDataBitSummary(MEMERR_INFO *info) {
  int a,as,d,ds;
  int byte_failure;

  if (!info) { return; }

  ds = 0;

  for (d=0; d< BITS_PER_BYTE ; d++) {
    byte_failure = 1;
    for (a=0; a< info->addr_bits ; a++) {
      for (as=0; as < NSTATES ; as++) {
	byte_failure &= (info->fail_map[a][as][d][ds] == info->test_map[a][as][d][ds]);
      }
    }

    if (byte_failure) {
     // printf("global bit stuck (%d)\n",d);
    }
  }
}

void memerrAddrBitSummary(MEMERR_INFO *info) {
  int a,as,d,ds;
  int nfail[NSTATES];

  if (!info) { return; }

  /* first look at global pin stuck failures */

  for (a= info->addr_cs_bits ; a<info->addr_bits; a++) {
    
  }

  /* now look for "chip pin" no connects or "dead chip" failures */
  for ( a =0 ; a <info->addr_cs_bits ; a++) {
    nfail[0] = nfail[1] = 0;

    for (as = 0; as < NSTATES ; as++) {
      for (d = 0; d< BITS_PER_BYTE ; d++) {
	for (ds = 0 ; ds < NSTATES ; ds++) {
	  nfail[as] += info->fail_map[a][as][d][ds];
	}
      }
    }
  }
}

void printMemErrSummary(MEMERR_INFO *info) {
  if (!info) { return; }
  
  memerrFullSummary(info);
  memerrDataBitSummary(info);
}
