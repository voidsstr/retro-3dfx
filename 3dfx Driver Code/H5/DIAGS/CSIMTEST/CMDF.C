#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3.h>
#ifdef SST2
#include "../csim/sst2asm.h"
#else
#include "../csim/h3asm.h"
#endif

main (int argc, char **argv)
{
    int count;
    long ptype;
    SstRegs *sst;

    if (argc < 3) {
	printf ("Usage: cmdf <count> <packet type>\n");
	exit (1);
    }

    fxHalInit(0);
    sst = fxHalMapBoard(0);
    if (!fxHalInitRegisters(sst))
	printf("fxHalInitRegisters failed!\n");
    fflush(stdout);
    if (!fxHalInitGamma(sst, 1.4F))
	printf("fxHalInitGamma failed!\n");
    fflush(stdout);
    if (!fxHalInitVideo(sst, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, NULL))
    //if (!fxHalInitVideo(sst, GR_RESOLUTION_1280x1024, GR_REFRESH_60Hz, NULL))
	printf("fxHalInitVideo failed!\n");
    fflush(stdout);

    count = atoi(argv[1]);
    ptype = atoi(argv[2]);

#define CMDFIFO_START 0x1F8000
#define CP_BEGIN (SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + CMDFIFO_START)

    fxHalInitCmdFifo(sst,0,CMDFIFO_START,4096,0,0,0);
{
    FxU32 *cp = (FxU32 *)CP_BEGIN;
    FxU32 *END = cp + 1024;

#define WRAP(cp) if (cp >= END) cp = (FxU32 *)CP_BEGIN

    while (count-- > 0) {
	static int i,bogus=0, vc;

	switch(ptype) {
	    case 0:
		SET(cp[0], SSTCP_PKT0_NOP);
		cp++;
		WRAP(cp);
		if (cp >= END-1000) {
		    SET(cp[0], ((CMDFIFO_START/4+0)<<SSTCP_PKT0_ADDR_SHIFT) | SSTCP_PKT0_JMP_LOCAL);
		    cp = (FxU32 *)CP_BEGIN;
		}
		break;
	    case 1:
		bogus &= 7;
		SET(cp[0], ((bogus+1)<<SSTCP_PKT1_NWORDS_SHIFT) | SSTCP_INC |
//				SSTCP_PKT1_2D | (2 << SSTCP_REGBASE_SHIFT) | SSTCP_PKT1);	// 2D
				(0x008 << SSTCP_REGBASE_SHIFT) | SSTCP_PKT1);	// 3D
		cp++;
		WRAP(cp);
		if (cp >= END) cp = (FxU32 *)CP_BEGIN;
		for (i=0; i<bogus+1; i++) {
		    SET(cp[0],i);
		    cp++;
		    WRAP(cp);
		}
		break;
	    case 2:		// 2D GUI PACKET
		SET(cp[0], (0xF0F << SSTCP_PKT2_MASK_SHIFT) | SSTCP_PKT2);
		cp++;
		WRAP(cp);
		for (i=0; i<8; i++) {
		    SET(cp[0],i);
		    cp++;
		    WRAP(cp);
		}
		break;
	    case 3:		// 3D TRIANGLE PACKET
		vc = 4;
		i = SST_SETUP_RGB;
		i <<= SSTCP_PKT3_PMASK_SHIFT;
		i |= SSTCP_PKT3_BDDBDD;
		i |= vc << SSTCP_PKT3_NUMVERTEX_SHIFT;
//		i |= SSTCP_PKT3_PACKEDCOLOR;
		SET(cp[0], i | SSTCP_PKT3);
		cp++;
		WRAP(cp);
		while (vc > 0) {		// for each vertex
		    for (i=0; i<2+3; i++) {
			SET(cp[0], i+1);
			cp++;
			WRAP(cp);
		    }
		    vc--;
		}
		break;
	    case 4:
		SET(cp[0], (0xA5 << SSTCP_PKT4_MASK_SHIFT) |
			SSTCP_PKT4_2D | (8 << SSTCP_REGBASE_SHIFT) | SSTCP_PKT4);	// 2D
//			(2 << SSTCP_REGBASE_SHIFT) | SSTCP_PKT4);	// 3D
		cp++;
		WRAP(cp);
		for (i=0; i<4; i++) {
		    SET(cp[0],i+1);
		    cp++;
		    WRAP(cp);
		}
		break;
	    case 5:
//		SET(cp[0], SSTCP_PKT5_3DLFB |
		SET(cp[0], SSTCP_PKT5_YUV |
			((bogus+1)<<SSTCP_PKT5_NWORDS_SHIFT) | SSTCP_PKT5);
		cp++;
		WRAP(cp);
		SET(cp[0],0x100);	// send LFB offset
		cp++;
		WRAP(cp);
		for (i=0; i<bogus+1; i++) {
		    SET(cp[0],0xafb000 + i);
		    cp++;
		    WRAP(cp);
		}
		break;
	    case 6:
	    case 7:
		break;
	}
	bogus++;
    }

    // need to send NOP command via packet as well
    SET(cp[0],(1<<SSTCP_PKT1_NWORDS_SHIFT) |
	(NOPCMD<<(SSTCP_REGBASE_SHIFT-2)) | SSTCP_PKT1);
    cp++;
    WRAP(cp);
    SET(cp[0],0);
    cp++;
    WRAP(cp);
}
    fxHalIdleNoNop(sst);
    fxHalShutdown(sst);
    return 0;
}
