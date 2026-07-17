/* 
 * memtst.c
 *
 * generic memory test routines 
 */
#include <stdio.h>
#include <stdlib.h>

#include "3dfx.h"

#include "memerr.h"
#include "memtst.h"
#include "mdc.h"

unsigned long int getCfgNumber(char *module, char *name, unsigned long int  def);

/* simpler bank oriented results */
/*
 * Memory configuration on boards is like this:
 *
 * SDRAM 16mb = 8 - 16 bit wide (2megabit) chips = 128bits, 1 bank
 * SGRAM 16mb = 8 - 32 bit wide (2megabit) chips = 128bits, 2 banks
 * SGRAM  8mb = 4 - 32 bit wide (2megabit) chips = 128bits, 1 bank
 *
 * It is possible to use 16 bit wide SGRAM, but I don't think we do it.
 *
 */

#define N_MEGS  16
#define N_WORDS  8
#define N_BANKS  2
FxU32 error_count_by_meg[N_MEGS];    /* per 1mb region */
FxU32 error_count_by_word[N_WORDS];    /* per 16 bit word */
FxU32 error_count_by_bank[N_BANKS]; /* per BANK */


void error_clear(void) {
  int i;
  for (i = 0; i < N_MEGS; i++) {
    error_count_by_meg[i] = 0;
  } 

  for (i = 0; i < N_WORDS; i++) {
    error_count_by_word[i] = 0;
  }
  for (i=0;i<N_BANKS;i++) {
    error_count_by_bank[i] = 0;
  }
}

void error_report(FxU32 *_addr,FxU32 expected, FxU32 received) {
  int meg_number, word_number, bank_number;
  FxU32 addr = (FxU32) _addr;
  int print_all = 0;

  meg_number = pull_bits(addr,21,24);
  word_number = pull_bits(addr,1,3);
  bank_number = test_bit(addr,4);

  ASSERT(meg_number < N_MEGS,"error_report():invalid meg number");
  ASSERT(word_number < N_WORDS,"error_report():invalid word number");
#if 0
  if (!error_count_by_meg[meg_number]) {
    print_all = 1;
    
  }
#endif

  if (print_all) {
     printf("er: 0x%.08X  expect: 0x%.08X  receive: 0x%.08X  word:%d  meg:%d bank:%d\n",
		addr, expected, received,word_number,meg_number,bank_number);
   }

  error_count_by_meg[meg_number]++;
  error_count_by_bank[bank_number]++;

  if ((expected & 0xFFFF) != (received & 0xFFFF)) {
    error_count_by_word[word_number]++;
  } 

  if ((expected >> 16) != (received >> 16)) {
    error_count_by_word[word_number | 1]++;
  }

}

void error_print() {
  int i;
#ifdef ENSONIQ_NUMBERS
  int j;
  char *fault_location[2] [N_WORDS] =
  { 
    { "U1: MD0-MD15", "U1: MD16-MD31", "U2: MD32-MD47", "U2: MD48-MD63" ,
      "U8: MD64-MD79","U8: MD80-MD95","U12: MD96-MD111","U12: MD112-MD127"},
    { "U5: MD0-MD15", "U5: MD16-MD31", "U6: MD32-MD47", "U6: MD48-MD63",
      "U7: MD64-MD79", "U7: MD80-MD95", "U11: MD96-MD111", "U11: MD112-MD127"}};
#endif


#if 0
  printf("*** Errors by MB (SDRAM)***\n");
  for (i = 0; i < N_MEGS; i++) {
      printf("% 2d MB = % 10d errors\n",i, error_count_by_meg[i]);
  } 
#endif

  printf("*** Errors by 16bit Word (SGRAM/SDRAM)***\n");
  for (i = 0; i < N_WORDS; i++) {
      printf("Word % 2d (datalines MD%d thru MD%d) = % 10d errors\n",
	i,i*16,(i*16)+ 15,error_count_by_word[i]);
  }

#if 0
  printf("*** Errors by bank (SGRAm)***\n");
  for (i=0;i<2;i++) {
      printf("Bank % 2d = % 10d errors\n",i+1,error_count_by_bank[i]);
  }
#endif

#ifdef ENSONIQ_NUMBERS
  printf("*** Errors by chip (SGRAM) ***\n");
  for (j=0;j<N_WORDS;j++) {
    for (i=0;i<N_BANKS;i++) {
	if (error_count_by_word[j] /* && error_count_by_bank[i] */) {
		printf("(%d/%d)First Check datalines: %s, then check other pins.\n",i,j,
			fault_location[i][j]);
        }
	
    }
  }
#endif

}


FxU32 constFill(FxU32 *start, FxU32 len, FxU32 value) {
  FxU32 *walker = start;
  FxU32 *end = (FxU32 *)((FxU32)start + (FxU32)len);
  MEMERR_INFO *info = NULL;
  FxU32 received,err_count;
  int print_all = getCfgNumber("memerr","excessive",0);
  int count=0;

  err_count = 0;

  if (getCfgNumber("memerr","full_report",0) == 1) {
    info = allocMemErrInfo(6,2);
  }
  error_clear();


  walker = start;
  while (walker < end) {
    *walker = value;
    walker++;
  }
  walker = start;

  while (walker < end) {
    received = *walker;

//    if (!(value & 0x400))
//       received &= ~(0x400);
    reportResultU32(info,(FxU32)walker,value,received);
    if (received != value) {
      error_report(walker,value,received);
      err_count++;
      if ((print_all) && (err_count < 100)) {
         printf("er: 0x%.08X  expect: 0x%.08X  receive: 0x%.08X\n",
		walker, value, received);
      }
    }
    count++;
    walker++;
  }

  printf("const_fill(0x%.08X): %lu errors\n",value,err_count);
  printMemErrSummary(info);

  freeMemErrInfo(info);


  if (err_count) {
    error_print();
  }
  return err_count;
}

FxU32 alternateFill(FxU32 *start, FxU32 len, FxU32 value1, FxU32 value2) {
  FxU32 *walker = start;
  FxU32 *end = (FxU32 *)((char *)start + len);
  MEMERR_INFO *info = NULL;
  FxU32 received,err_count;
  FxU32 values[8];
  FxU32 expected;
  int which_value = 0;
  int num_values = 8;
  int print_all = getCfgNumber("memerr", "excessive",0);
  int debug_alt_fill  = 0;
  int count=0;

  err_count = 0;
  values[0] = value1;
  values[1] = value1;
  values[2] = value1;
  values[3] = value1;
  values[4] = value2;
  values[5] = value2;
  values[6] = value2;
  values[7] = value2;

  if (getCfgNumber("memerr","alternate_fill_report",0)) {
    debug_alt_fill = 1;
  }

  if (debug_alt_fill) {
    if (getCfgNumber("memerr","full_report",0) == 1) {
      info = allocMemErrInfo(6,2);
    }
    error_clear();
  }
  
  /* write out data */
  which_value = 0;
  walker = start;
  while (walker < end) {
    *walker = values[which_value];
    if (++which_value >= num_values) {
       which_value = 0;
    }
    walker++;
  }

  printf("alternate_fill(0x%.08X,0x%.08X) completed.\n",
		value1,value2);


  if (debug_alt_fill)  {
    /* read back */

    which_value = 0;
    walker = start;
    while (walker < end) {
      received = *walker;
      expected = values[which_value];
//      if (!(expected & 0x400))
//         received &= ~(0x400);
      if (++which_value >= num_values) {
        which_value = 0;
      }
      reportResultU32(info,(FxU32)walker,expected,received);
      if (received != expected) {
        error_report(walker,expected,received);
        err_count++;
        if ((print_all) && (err_count < 100)) {
           printf("er: 0x%.08X  expect: 0x%.08X  receive: 0x%.08X\n",
		walker, expected, received);
        }
      }
      count++;
      walker++;
    }

    printf("alternate_fill(0x%.08X,0x%.08X): %lu errors\n",
		value1,value2,err_count);
    printMemErrSummary(info);

    freeMemErrInfo(info);

    if (err_count) {
      error_print();
    }
  }
  return (err_count);
}

FxU32 rndmFill(FxU32 *start, FxU32 len, FxU32 seed) {
  FxU32 value;
  FxU32 *walker = start;
  FxU32 *end = (FxU32 *)((char *)start + len);
  MEMERR_INFO *info = NULL;
  FxU32 received,err_count;
  int print_all;

  if (getCfgNumber("memerr","full_report",0) == 1) {
    info = allocMemErrInfo(6,2);
  }
  error_clear();
  err_count = 0;

  print_all = getCfgNumber("memerr", "excessive",0);

  printf("start = 0x%.08X, end = 0x%.08X\n",start, end);

  // write
  value = seed;
  walker = start;
  while (walker < end) {
    *walker = value;
    walker++;
    value = (value * 1103515245) + 12345;
  }

  // readback
  walker = start;
  value = seed;
  while (walker < end) {
    received = *walker;
      reportResultU32(info,(FxU32)walker,value,received);
    if (received != value) {
      error_report(walker,value,received);
      err_count++;
      if ((print_all) && (err_count < 100)) {
        printf("er: 0x%.08X  expect: 0x%.08X  receive: 0x%.08X\n",walker, value, received);
      }
    }
    walker++;
    value = (value * 1103515245) + 12345;
  }

  printf("rndm_fill(): %lu errors\n",err_count);
  printMemErrSummary(info);

  freeMemErrInfo(info);

  if (err_count) {
    error_print();
  }

  return (err_count);
}

FxBool walkingFill(FxU32 *start, FxU32 len, FxU32 value) {
  FxU32 *walker = start;
  FxU32 *walker2;
  FxU32 *end = (FxU32 *)((char *)start + len);
  FxU32 err_count;

  error_clear();
  err_count = 0;

  while (walker < end) {
    *walker = 0;
    walker++;
  }

  walker = start;

  while (walker < end) {
    *walker = value;
    walker2 = start;
    while (walker2 < end) {
      if (((walker != walker) && (*walker != 0)) || (*walker != value)) {
        error_report(walker,value,*walker);
	err_count++;
      }
      walker2++;
    }

    *walker = 0;
    walker++;
  }

  printf("walking_fill(0x%.08X): %lu errors\n",value,err_count);

  if (err_count) {
    error_print();
    return FXFALSE;
  }

  return FXTRUE;
}

