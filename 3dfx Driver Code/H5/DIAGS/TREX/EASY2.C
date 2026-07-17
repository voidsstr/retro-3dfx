#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "udiag.h"
#include "sstdiag.h"

void downloadTexture(SstRegs *sst);
void drawTestQuad(SstRegs *sst, int xOffset, int yOffset);
void checkTestQuad(SstRegs *sst, int xOffset, int yOffset);

/*Stank global variables*/
FxU32 texture[256*256];

int main (int argc, char **argv)
{
  SstRegs *sst;
  CsimPrivate *cpriv;
  
  sst = SST_BEGIN(argc,argv);
  cpriv=CSIM_PRIVATE(diago.sstCSIM);

  while (DIAG_STARTPASS()) 
    {    
      SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT | SST_ENTEXTUREMAP);
      SET(sst->fbzMode, SST_RGBWRMASK);	   
      SET(sst->renderMode, SST_RM_32BPP | SST_RM_RGB_WMASK | SST_RM_ALPHA_WMASK);

      SET(sst->textureMode, SST_TC_REPLACE | SST_TCA_REPLACE | SST_ARGB8888);
      //SET(sst->alphaMode, SST_ENALPHABLEND | (SST_A_SRCALPHA<<SST_RGBSRCFACT_SHIFT) 
      //	  | (SST_A_ONE << SST_ASRCFACT_SHIFT));
   
      /*Make sure we use the 256x256 texture (LOD 0)*/
      SET(sst->tLOD, ((0<<SST_LOD_FRACBITS)<<SST_LODMIN_SHIFT) |
	  ((0<<SST_LOD_FRACBITS)<<SST_LODMAX_SHIFT));
      
      downloadTexture(sst);
      drawTestQuad(sst, 0, 0);
      checkTestQuad(sst, 0, 0);
    }

  DIAG_PASS(0);       

  return(1);
}

void checkTestQuad(SstRegs *sst, int xOffset, int yOffset)
{
  SstIORegs *sstIORegs;
  unsigned int baseAddress, address;
  int x, y;

  sstIORegs = (SstIORegs *)SST_IO_ADDRESS((int)sst);

  fxHalIdle(sst);

  //Push the tiled space high up into the RAW LFB space
  SET(sstIORegs->lfbMemoryConfig, SST_RAW_LFB_TILE_BEGIN_PAGE);

  for(y=0; y<256; y++)
    for(x=0; x<256; x++)
      {
	address = baseAddress + diagfb.colBufferAddr[0] + diagfb.colBufferStride[0]*(y+yOffset) + 4*(x+xOffset);

	DIAG_TEST_MEM(address, texture[256*y + x], 4);
      }
}

/*Download a 256x256 texture that is a scaled 128x128 texture*/
void downloadTexture(SstRegs *sst)
{
  unsigned int address;
  int x, y, i;

  //Need to switch texBaseAddr to flush texture cache
  SET(sst->texBaseAddr, 0); 
  SET(sst->texBaseAddr, 1024*1024);

  //Create Texture
  for(y=0; y<256; y++)
    for(x=0; x<256; x++)
      texture[256*y+x]= (((x*y) & 0xFF) << 24) | ((x&0xFF)<<16) | ((y&0xFF)<<8) | (((~y)&0xFF));

  //download texture
  GDBG_INFO(50, "downloadTexture\n");
  
  address = ((int )sst) - SST_3D_OFFSET + SST_TEX2_OFFSET;
  for(i=0; i<256*256; i++)
    {
      halStore32((void *)address, texture[i]);
      address+=4;
    }
}

void drawTestQuad(SstRegs *sst, int xOffset, int yOffset)
{
  SET(sst->sSetupMode, SST_SETUP_Wfbi | SST_SETUP_ST0 | SST_SETUP_FAN);

  //Vertex 0
  SETF(sst->sVx, (float)xOffset + .5f);
  SETF(sst->sVy, (float)yOffset + .5f);
  SETF(sst->sOowfbi, 1.f);
  SETF(sst->sSow0, 0.f);
  SETF(sst->sTow0, 0.f);
  SET(sst->sBeginTriCMD, 0);

  //Vertex 1
  SETF(sst->sVx, (float)xOffset+256.5f);
  SETF(sst->sVy, (float)yOffset+.5f);
  SETF(sst->sOowfbi, 1.f);
  SETF(sst->sSow0, 256.f);
  SETF(sst->sTow0, 0.f);
  SET(sst->sDrawTriCMD, 0);

  //Vertex 2
  SETF(sst->sVx, (float)xOffset+256.5f);
  SETF(sst->sVy, (float)yOffset+256.5f);
  SETF(sst->sOowfbi, 1.f);
  SETF(sst->sSow0, 256.f);
  SETF(sst->sTow0, 256.f);
  SET(sst->sDrawTriCMD, 0);

  //Vertex 3
  SETF(sst->sVx, (float)xOffset+0.5f);
  SETF(sst->sVy, (float)yOffset+256.5f);
  SETF(sst->sOowfbi, 1.f);
  SETF(sst->sSow0, 0.f);
  SETF(sst->sTow0, 256.f);
  SET(sst->sDrawTriCMD, 0);
}





