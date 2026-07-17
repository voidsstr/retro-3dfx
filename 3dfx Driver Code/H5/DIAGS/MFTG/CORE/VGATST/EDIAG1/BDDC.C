#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
// #include <i86.h>

#include "ediag.h"

#define VIDEO 0x10

/*
typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned long ULONG;
*/


BYTE buffer[128];
char *toManufactID = &buffer[8];
char DDCManufact[4];

#define DEFAULT_STR "NEC"

int bddc_check(int iter, void (*cbfn)(int count,int data)) {
  //  struct SREGS sregs;
  union REGS regs;
  REALMODECALL rmc;
  WORD wSeg, wSel;
  int i,loop;
  unsigned long ctr0 =0; /* call 0 fail */
  unsigned long ctr1 =0; /* call 1 fail */
  unsigned long ctr2 =0;
  BYTE bSum;
  int cb_count = 0;

#ifdef DEBUG_OUTPUT  
  unsigned long ctr  =0;
  unsigned long ctr3 =0;
#endif

  for (loop=0;loop<iter;loop++)
  {
    
    // Allocate DOS Memory Block
    regs.w.ax = 0x0100;
    regs.w.bx = ((sizeof(BYTE) * 128)/16) + 1;
    int386(0x31,&regs,&regs);
    wSeg = regs.w.ax;
    wSel = regs.w.dx;
    
    // Do Call!
    rmc.eax=0x4f15;
    rmc.ebx = 0;		// Report VBE/DDC Capabilities
    rmc.ecx = 0;
    rmc.es = 0;
    rmc.edi = 0;
    RealModeINT(&rmc,VIDEO);
    
    if ((rmc.eax & 0xFFFF) != 0x004f)	{
      printf("Call 0x00--Report VBE/DDC Capabilities failed %x\n", rmc.eax);
      // fputc('*', stderr);
      ctr0++;
      continue;
    }
    
    rmc.eax = 0x4f15;
    rmc.ebx = 0x01;		// Read EDID
    rmc.ecx = 0x00;
    rmc.edx = 0x00;
    memset(buffer, 0xA5, sizeof(buffer) * sizeof(BYTE));
    rmc.es = wSeg;
    rmc.edi = 0;
    RealModeINT(&rmc,0x10);
    // copy the info out of the real mode segment
    _fmemcpy(buffer,MK_FP(wSel,0), sizeof(buffer) * sizeof(BYTE));
    rmc.eax &= 0x0000FFFF;
    if ((rmc.eax & 0xFFFF) != 0x004f) {
      printf("call 0x01--Read EDID failed 0x%X\n",rmc.eax);
      // fputc('*',stderr);
      ctr1++;
      continue;
    } 
    
    DDCManufact[0] = '@'+  (( toManufactID[0] & (0x1F << 2)) >> 2);
    DDCManufact[1] = '@'+ ((( toManufactID[0] & 3) << 3) | 
                           (( toManufactID[1] & ( 7 << 5)) >> 5 ));
    DDCManufact[2] = '@'+ ( toManufactID[1] & 0x1F);
    DDCManufact[3] = '\0';
    // do checksum
    
    for (bSum=i=0; i<128; i++) {
      if (cbfn) {
        cbfn(cb_count++,buffer[i]); // inform the checksum callback
      }
      bSum += buffer[i];
    }
    
    if (bSum != 0) {
      printf("EDID Checksum Error %x\n", bSum);
      // fputc('*', stderr);
      ctr2++;
      continue;
    }
    
  }

#ifdef DEBUG_OUTPUT  
  for (i=0; i<128; i++) {
    printf("%d %02x\n", i, buffer[i]);
  }
  
  printf("Manufacturer: %s \n", DDCManufact);
  
  printf("(Total %ld) (Success %ld)\n",ctr+ctr0+ctr1+ctr2+ctr3, ctr);
  printf("Failures: (Call 00=%ld) (Call 01=%ld) (ChkSum=%ld) (Compares=%ld)\n",
          ctr0, ctr1, ctr2, ctr3);
  
#endif

  if (ctr0) {
    return 1; /* call 0 fail */
  } else if (ctr1) {
    return 2; /* call 1 fail */
  } else {
    return 0; /* pass!! */
  }
}

