#include <3dfx.h>
#include <glidesys.h>

#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"
#include "fxcmd.h"


#include <stdio.h>



#define GEN_OFFSET(structName,fieldName,defName) \
  if(offsetof(structName,fieldName) > 32767) { \
    fprintf(out,"%s SET %d\n",defName,-(65536 - offsetof(structName,fieldName))); \
  } else { \
    fprintf(out,"%s SET %d\n",defName,offsetof(structName,fieldName)); \
  }

int main(int ac, char **av)
{
  FILE *out;
  size_t offset;
  static struct _GlideRoot_s gr;
  static GrGC gc;

  if(out = fopen(":src:PPCInline.inc","w")) {
  
    GEN_OFFSET(GrGC,lostContext,"lostContext");
    GEN_OFFSET(GrGC,triSetupProc,"triSetupProc");
    GEN_OFFSET(GrGC,curTriSize,"curTriSize");
    GEN_OFFSET(GrGC,stats.trisProcessed,"trisProcessed");
    GEN_OFFSET(GrGC,stats.trisDrawn,"trisDrawn");
    GEN_OFFSET(GrGC,state.invalid,"invalid");

    GEN_OFFSET(GrGC, cmdTransportInfo.fifoPtr, "fifoPtr");
    GEN_OFFSET(GrGC, cmdTransportInfo.fifoRead, "fifoRead");
    GEN_OFFSET(GrGC, cmdTransportInfo.fifoRoom, "fifoRoom");
    GEN_OFFSET(GrGC, cmdTransportInfo.autoBump, "autoBump");
    GEN_OFFSET(GrGC, cmdTransportInfo.bumpPos, "bumpPos");
    GEN_OFFSET(GrGC, cmdTransportInfo.bumpSize, "bumpSize");
    GEN_OFFSET(GrGC, cmdTransportInfo.lastBump, "lastBump");
    
    GEN_OFFSET(GrGC, cmdTransportInfo.triPacketHdr, "triPacketHdr");
    GEN_OFFSET(GrGC, tsuDataList,"tsuDataList");
    GEN_OFFSET(GrGC, state.cull_mode,"cull_mode");

    GEN_OFFSET(GrGC, cRegs, "cRegs");
    GEN_OFFSET(SstCRegs, cmdFifo0.bump, "cmdFifo0bump");
    	    
    fclose(out);
  }

  return 0;

}    

