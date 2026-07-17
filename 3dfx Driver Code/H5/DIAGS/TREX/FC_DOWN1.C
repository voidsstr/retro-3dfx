/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 8:19:03 PM$
*/

#include <assert.h>
#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"

#define TEXBASE_ALIGN 15

int main (int argc, char **argv)
{
    SstRegs *sst, *tmu;


    FxU32 test_textureMode, test_tLOD;
    FxU32 tLOD, textureMode;

    FxI32 s_max_exp, t_max_exp;
    FxU32 s_max, t_max;
    FxU32 st_mask;

    FxI32 write_cnt;

    FxI32 aspect_cnt;
    FxI32 bpt;  //Number of bytes per texel for texture format
    FxI32 lod_shift, s_shift, t_shift;
    FxI32 write_lod;
    FxU32 s, t, ds;
    FxI32 st_offset;
    FxI32 lod_wr_cnt;
    FxU32 textureFormat;

    CsimPrivate *cpriv;

    sst = SST_BEGIN(argc,argv);
    cpriv = CSIM_PRIVATE(diago.sstCSIM);

    if (!diago.ytiled) {
	GDBG_ERROR("fc_down1", "must run in tiled memory mode\n");
	DIAG_FAIL();
    }

    while (DIAG_STARTPASS()) {          // for each pass
      int trex, tmuOffset;
      // optionally choose random trex
      trex = diago.trex;
      if (trex < 0)
	trex = iRandom(-trex);

      tmu = SST_TREX(sst,trex);
      tmuOffset = trex * (SST_TEX1_OFFSET - SST_TEX0_OFFSET);
      gdbg_info(2,"testing TMU chip #%d\n",trex);

      test_textureMode = 0;
	
      test_tLOD = (0x00 << SST_LODMIN_SHIFT) // 4.2	  
	| (0x00 << SST_LODMAX_SHIFT) // 4.2
	| (0x00 << SST_LODBIAS_SHIFT); // 4.2 
	
      write_cnt = 0;

      // various T, S, DATA, texBaseAddr
      // for each of the 7 aspect ratios (lod_aspect, lod_s_is_wider)
      //   (do s and t is wider for 1:1, so 8 combinations
      for (aspect_cnt = 0; aspect_cnt < 8; aspect_cnt++) {
	gdbg_info(2,"aspect_cnt = %d\n",aspect_cnt);
	// for 8 and 16-bit
	for (bpt = 1;  bpt <= 4; bpt++)
	  {
	    if(bpt == 3) //No such thing
	      continue;
	      
	    gdbg_info(2,"bpt = %d\n",bpt);

	    switch(bpt)
	      {
	      case 1:
		s_shift = SST_TEXTURE_S8_SHIFT;
		t_shift = SST_TEXTURE_T8_SHIFT;
		lod_shift = SST_TEXTURE_LOD8_SHIFT;
		textureFormat = SST_RGB332;
		ds=4;
		break;
	      case 2:
		s_shift = SST_TEXTURE_S16_SHIFT;
		t_shift = SST_TEXTURE_T16_SHIFT;
		lod_shift = SST_TEXTURE_LOD16_SHIFT;
		textureFormat = SST_ARGB4444;
		ds=2;
		break;
	      case 4:
		s_shift = SST_TEXTURE_S32_SHIFT;
		t_shift = SST_TEXTURE_T32_SHIFT;
		lod_shift = SST_TEXTURE_LOD32_SHIFT;
		textureFormat = SST_ARGB8888;
		ds=1;
		break;
		
	      default:
		assert(0);
	      }

	    textureMode = test_textureMode | textureFormat;
	    tLOD = test_tLOD 
	      | ((aspect_cnt & 4) ? SST_LOD_S_IS_WIDER : 0)
	      | ((aspect_cnt & 3) << SST_LOD_ASPECT_SHIFT);

	    SET(tmu->textureMode, textureMode);
	    SET(tmu->tLOD, tLOD);
	   
	    // for each LOD == [0,8]
	    for (write_lod = 0; write_lod <= 8; write_lod++) {
	      gdbg_info(2,"write_lod = %d\n",write_lod);


	      // new random base addr
	      {
		int s_is_wider = (aspect_cnt & 4) ? 1 : 0;
		int ar = (aspect_cnt & 3);
		int addr, tStride, w, h;
		FxU32 textureSize, textureBaseAddress;
		tiledStruct mipmap;
		  
		mipmap = sstTiledMipMapOffset2(0, tLOD, textureMode);

		w = mipmap.utot;
		h = mipmap.vtot;
		tStride = w/(SST_TILE_WIDTH/bpt);

		textureSize = (w/(SST_TILE_WIDTH/bpt) + 1) * (h/(SST_TILE_HEIGHT) + 1) * SST_TILE_SIZE;
		  
		addr = allocate(textureSize, "Junk Texture", randomPlacement);
		
		textureBaseAddress = tiledAddress(addr, tStride, bpt, -mipmap.uoff, -mipmap.voff);
		  
		//Unallocate immediately because it doesn't matter if we write over the texture later
		unallocate(addr);
		  
		SET(tmu->texBaseAddr, 
		    SST_TEXTURE_MUNGE_ADDRESS(textureBaseAddress) | SST_TEXTURE_IS_TILED | (tStride<<SST_TEXTURE_TILESTRIDE_SHIFT) );
	      }

	      t_max_exp = (8 - write_lod) - ((aspect_cnt & 4) ? (aspect_cnt & 3) : 0);
	      t_max = (t_max_exp >= 0) ? (1 << t_max_exp) : 1;
	      s_max_exp = (8 - write_lod) - ((aspect_cnt & 4) ? 0 : (aspect_cnt & 3));
	      s_max = (s_max_exp >= 0) ? (1 << s_max_exp) : 1;
	      st_mask = ((t_max-1)<<t_shift) | ((s_max-1)<<s_shift);
	      st_mask &= ~3;
		


	      if (write_lod >= 5) {
		// 8x8 or smaller, so do exhaustive load
		for (t = 0; t < t_max; t++) {
		  for (s = 0; s < s_max; s = s + ds) {
		    FxU32 offset;

		    offset = (write_lod<<lod_shift) | (t<<t_shift) | (s<<s_shift); 
			
		    //We need to make sure that we don't write to an offset of 
		    //greater than 2MB with the old texture ports. This can only
		    //occur when using tiled 32bpt textures
			
		    if(offset < SST_TEX_SIZE)
		      {			  
			SET(*(FxU32 *)((SST_TEX_ADDRESS(sst)) + tmuOffset + offset), iRandom(0xffffffff));
			write_cnt++;
		      }
		  }
		}
	      } else {
		SET(*(FxU32 *)(SST_TEX_ADDRESS(sst) + tmuOffset + (0x0001fe00 & st_mask)),
		    iRandom(0xffffffff));
		SET(*(FxU32 *)(SST_TEX_ADDRESS(sst) + tmuOffset + (0x000001ff & st_mask)),
		    iRandom(0xffffffff));
		SET(*(FxU32 *)(SST_TEX_ADDRESS(sst) + tmuOffset + (0x00015555 & st_mask)),
		    iRandom(0xffffffff));
		SET(*(FxU32 *)(SST_TEX_ADDRESS(sst) + tmuOffset + (0x0000aaaa & st_mask)),
		    iRandom(0xffffffff));
		write_cnt += 4;

		for (lod_wr_cnt = 0; lod_wr_cnt < (1 << (8 - write_lod)); lod_wr_cnt++) {
		  st_offset = iRandom(0x0001ffff);  // separate statement so random 
		  // order is deterministic
		  st_offset &= st_mask;
		  // how about the write_lod????
		  SET(*(FxU32 *)(SST_TEX_ADDRESS(sst) + tmuOffset + ((write_lod<<lod_shift) | st_offset)), 
		      iRandom(0xffffffff));
		  write_cnt++;
		}
	      }
	    }
	  }
      }

      gdbg_info(1,"fc_down1.c:   write_cnt=%d\n", write_cnt);
    }

  DIAG_PASS(0);

  return(0);
}
