/*-*-c++-*-*/

#include <assert.h>
#include <stdio.h>

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

#define COLOR_BUFFER_BASE 0x1000
#define COLOR_BUFFER_STRIDE 8192

#define MAX_LOD_DESCRIPTIONS 2048

//This is an ad hoc diag designed to test the location
//of all the LODs for compressed texture formats.
//The diag doesn't support any options; it just
//tests every possibility. In fact, it would be
//bad to use options
//The only useable option is --compressedTextures

//This diag is dedicated to Eazy E.

void downloadTexture(SstRegs *sst);
void lodAddressCheck(SstRegs *sst);
void lodAddressDrawTriangle(SstRegs *sst, FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel, FxBool tiled,
			    FxBool compressed, FxBool bigAssTexture, FxBool wideTexture, FxBool splitTextures);

//Global variable
FxU32 *rawLFB;
FxI32 lodCounter=0;
char lodDescription[MAX_LOD_DESCRIPTIONS][128];

int main (int argc, char **argv)
{
  SstRegs *sst;
  SstIORegs *sstio;

  FxU32 lod, logAspectRatio, bitsPerTexel, maximumLOD;
  FxBool tiled, bigAssTexture, wideTexture, splitTextures;
  FxI32 m, n;

  // Don't let this diag be run with command fifo on
  for(m=0; m<argc; m++)
    {
      if(!strncmp(argv[m], "-W", 2))
	{
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "Warning: Stripping this arg \"%s\"!\n", argv[m]);
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  for(n=m; n<argc-1; n++)
	    argv[n] = argv[n+1];
	  
	  argc--;
	  m--;
	}	
      else if(!strcmp(argv[m], "--enableAA"))
	{
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "Warning: Stripping this arg \"%s\"!\n", argv[m]);
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  for(n=m; n<argc-1; n++)
	    argv[n] = argv[n+1];
	  
	  argc--;
	  m--;
	}	
      else if(!strcmp(argv[m], "--chipCount"))
	{
	  if(m == argc-1)
	    {
	      GDBG_ERROR("compton::main", "F'ed --chipCount with no arg\n");
	      exit(-1);
	    }

	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "Warning: Stripping this arg \"%s %s\"!\n", argv[m], 
		    argv[m+1]);
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  GDBG_INFO(0, "\n");
	  for(n=m; n<argc-2; n++)
	    argv[n] = argv[n+2];
	  
	  argc-=2;
	  m-=2;
	}	

    }
  

  sst = SST_BEGIN(argc,argv);
  
  // make rawlfb all linear
  sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));

  SET(sstio->lfbMemoryConfig, ((0x1FFF)<<SST_RAW_LFB_TILE_BEGIN_PAGE_SHIFT) | (3<<23));
  
  rawLFB = (FxU32 *)(((int)sst) - SST_3D_OFFSET + SST_RAW_LFB_OFFSET);
    
  //Download the texture fragment
  downloadTexture(sst);

  //Set up the color buffer
  SET(sst->colBufferAddr, COLOR_BUFFER_BASE);
  SET(sst->colBufferStride, COLOR_BUFFER_STRIDE);

  //Make sure we can write to the color buffer RGB
  SET(sst->fbzMode, SST_RGBWRMASK);
  SET(sst->renderMode, SST_RM_32BPP | SST_RM_RGB_WMASK);

  //Set it up so TMU 0's output is written to the color buffer
  SET(sst->fbzColorPath, SST_RGBSEL_TMUOUT | SST_ENTEXTUREMAP);
  SET_0(sst->textureMode, 0);
  SET_0(sst->combineMode, SST_CM_TC_OTHERSELECT_LOCAL_TRGB | SST_CM_USE_COMBINE_MODE);
  

  if(diago.compressedTextures)
    {
      for(bitsPerTexel=4; bitsPerTexel<=8; bitsPerTexel+=4)
	for(bigAssTexture=0; bigAssTexture<=1; bigAssTexture++)
	  {
	    if(bigAssTexture)
	      maximumLOD = 11;
	    else
	      maximumLOD = 8;
	    
	    for(tiled = 0; tiled<=1; tiled++)
	      for(wideTexture=0; wideTexture<=1; wideTexture++)
		for(lod=0; lod<=maximumLOD; lod++)
		  for(logAspectRatio=0; logAspectRatio<4; logAspectRatio++)
		    lodAddressDrawTriangle(sst, lod, logAspectRatio, bitsPerTexel, tiled,
					   FXTRUE, bigAssTexture, wideTexture, FXFALSE);
	    
	  }
    }
  else //Non-Compressed textures
    {
      for(bitsPerTexel=8; bitsPerTexel<=32; bitsPerTexel+=8)
	{
	  
	  //No 24bpt texture formats
	  if(bitsPerTexel == 24)
	    continue;
	  
	  for(bigAssTexture=0; bigAssTexture<=1; bigAssTexture++)
	    {
	      if(bigAssTexture)
		maximumLOD = 11;
	      else
		maximumLOD = 8;
	      
	      for(tiled = 0; tiled<=1; tiled++)
		{
		  for(splitTextures=1; splitTextures>=0; splitTextures--)
		    {
		      //There's no such thing as split tiled textures
		      if(tiled)
			splitTextures--;

		      for(wideTexture=0; wideTexture<=1; wideTexture++)
			for(lod=0; lod<=maximumLOD; lod++)
			  for(logAspectRatio=0; logAspectRatio<4; logAspectRatio++)
			    lodAddressDrawTriangle(sst, lod, logAspectRatio, bitsPerTexel, tiled,
						   FXFALSE, bigAssTexture, wideTexture, splitTextures);
		    }
		}	      
	    }   
	}   
    }
      
  //Check all the LODs now
  lodAddressCheck(sst);

  DIAG_PASS(-1);
  
  return(0);
}


//This downloads 1 compressed texture block that has one white pixel at 0,0.
//The texture is stored at address 0
//All the other texels are black
void downloadTexture(SstRegs *sst)
{              
  if(diago.compressedTextures)
    {
      SET(rawLFB[0], 0x0000FFFF);
      SET(rawLFB[1], 0xFFFFFFFC);
      SET(rawLFB[2], 0x0000FFFF);
      SET(rawLFB[3], 0xFFFFFFFC);
    }
  else
    {
      SET(rawLFB[0], 0xFFFFFFFF);
    }
}



void lodAddressCheck(SstRegs *sst)
{
  FxI32 i;
  FxU32 result;

  //Idle to be super cautious
  sst_idle_really(sst);
  
  for(i=0; i<lodCounter; i++)
    {
      //Check that the pixel is correct
      result = GET(((FxU32*)(((FxU32)rawLFB) + COLOR_BUFFER_BASE))[i]);
      
      if(result != 0xFFFFFF)
	GDBG_ERROR("", "%s fails!\n", lodDescription[i]);
    }
}


void lodAddressDrawTriangle(SstRegs *sst, FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel, FxBool tiled,
			    FxBool compressed, FxBool bigAssTexture, FxBool wideTexture, FxBool splitTextures)
{
  FxU32 width, height;
  FxU32 texBaseAddr;

  assert(lodCounter < MAX_LOD_DESCRIPTIONS);

  if(wideTexture)
    {
      if(bigAssTexture)
	{
	  width = 1<<(11-lod);
	  height = width >> logAspectRatio;
	}
      else
	{
	  width = 1<<(8-lod);
	  height = width >> logAspectRatio;	  
	}
    }
  else
    {
      if(bigAssTexture)
	{
	  height = 1<<(11-lod);
	  width = height >> logAspectRatio;
	}
      else
	{
	  height = 1<<(8-lod);
	  width = height >> logAspectRatio;	  
	}
    }
  if(width == 0)
    width = 1;
  if(height == 0)
    height = 1;
  
  sprintf(lodDescription[lodCounter], "(Case %d) lod %d %4dx%-4d %d bit %s%s%smipmap", 
	  (int)lodCounter, (int)lod, (int)width, (int)height, (int)bitsPerTexel,
	  (compressed ? "compressed " : ""), 
	  (tiled ? "tiled " : "linear "),
	  (splitTextures ? "split " : ""));
  GDBG_INFO(0, "%s\n", lodDescription[lodCounter]);
  
  //set the texture format
  if(compressed)
    {
      if(bitsPerTexel == 4)    
	SET(sst->textureMode, shadowRegisters3D[0][1].textureMode |
	    SST_COMPRESSED_TEXTURES | SST_DXT1);
      else if(bitsPerTexel == 8)    
	SET(sst->textureMode, shadowRegisters3D[0][1].textureMode |
	    SST_COMPRESSED_TEXTURES | SST_DXT2);     
      else
	assert(0);
    }
  else
    {
      if(bitsPerTexel == 8)    
	SET(sst->textureMode, shadowRegisters3D[0][1].textureMode | SST_RGB332);
      else if(bitsPerTexel == 16)    
	SET(sst->textureMode, shadowRegisters3D[0][1].textureMode | SST_RGB565);
      else if(bitsPerTexel == 32)    
	SET(sst->textureMode, shadowRegisters3D[0][1].textureMode | SST_ARGB8888);
      else
	assert(0);

    }

  //Set texBaseAddr
  if(tiled)
    {
      tiledStruct mipmap;
      FxU32 tileStride, depth;

      //Figure out where texBaseAddr should be
      mipmap = sstTiledMipMapOffset(lod, logAspectRatio, bitsPerTexel, bigAssTexture, wideTexture, compressed);

      if(compressed)
	{
	  convertToMicroTile(shadowRegisters3D[0][1].textureMode, &mipmap.uoff, &mipmap.voff);
	  depth = 16;
	}
      else
	{
	  depth = bitsPerTexel/8;
	}
                  
      texBaseAddr = tiledAddress(0, 96, depth, -mipmap.uoff, -mipmap.voff);
      
      if(texBaseAddr & 0xF)
	{
	  GDBG_INFO(0, "%s skipped!\n", lodDescription[lodCounter]);
	  return;
	}
      
      tileStride = 96;

      SET(sst->texBaseAddr, SST_TEXTURE_MUNGE_ADDRESS(texBaseAddr) |
	  ((tileStride << SST_TEXTURE_TILESTRIDE_SHIFT) & SST_TEXTURE_TILESTRIDE) |
	  SST_TEXTURE_IS_TILED);
    }
  else
    {
      FxI32 offset;
      
      offset = sstLinearMipMapOffset(lod, logAspectRatio, bitsPerTexel, bigAssTexture, wideTexture, compressed,
				     splitTextures);
      texBaseAddr = ((FxU32)(-offset)) & SST_TEXTURE_FULL_ADDRESS;

      if(texBaseAddr & 0xF)
	{
	  GDBG_INFO(0, "%s skipped!\n", lodDescription[lodCounter]);
	  return;
	}

      SET(sst->texBaseAddr, SST_TEXTURE_MUNGE_ADDRESS(texBaseAddr));
    }

  //set tLOD
  SET(sst->tLOD, (lod << 2) << SST_LODMIN_SHIFT |
      (lod << 2) << SST_LODMAX_SHIFT |
      ((splitTextures) ? SST_LOD_TSPLIT : 0) |
      ((bigAssTexture) ? SST_TBIG : 0) |
      ((wideTexture) ? SST_LOD_S_IS_WIDER : 0) |
      ((logAspectRatio << SST_LOD_ASPECT_SHIFT) & SST_LOD_ASPECT));
      
  //Draw a 1 pixel triangle
  SETF(sst->Fs, 0.0f);
  SETF(sst->Ft, 0.0f);
  SETF(sst->Fw, 1.0f);
  SETF(sst->FvA.x, ((float)lodCounter));
  SETF(sst->FvA.y, 0.0f);
  SETF(sst->FvB.x, ((float)lodCounter) + 1.0f);
  SETF(sst->FvB.y, 0.5f);
  SETF(sst->FvC.x, ((float)lodCounter));
  SETF(sst->FvC.y, 1.0f);
  SETF(sst->FtriangleCMD, 1.0f);

  lodCounter++;
}

