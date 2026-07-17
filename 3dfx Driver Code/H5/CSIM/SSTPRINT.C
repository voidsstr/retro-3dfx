#include "vxd.h"
/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:09:19 PM$
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#include <h3.h>
#include "h3sim.h"

static char *cmdNames[] = {"****","NOP","TRIANGLE","FASTFILL","SWAPBUF"};
static char *rm_3d_str[] = {"16", "15", "32", "**"};
static char *rm_arm_str[] = {"ZERO", "ONE ", "MSB ", "****"};
static char *so_str[] = {"KEEP","ZERO", "REPL", "INCS ", "DECS", "NEG ", "INCW", "DECW"};
static char *srcFactor_str[] = {"0","sA","dC","dA",
				"1","1-sA","1-dC","1-dA",
				"sC","1-sC","****","****",
				"****","****","****","sATU"};
static char *dstFactor_str[] = {"0","sA","sC","dA",
				"1","1-sA","1-sC","1-dA",
				"dC","1-dC","****","****",
				"****","****","****","sC<F"};
static char *tfmt_str[] = {"R-332","Y-422","A8   ","I8   ",
			   "AI44 ","P8   ","P6666","*****",
			   "R8332","T8422","R-565","R1555",
			   "R4444","AI88 ","AP88 ","R8888"};
static char *otherColorSel_str[] = {"IT", "TX", "C1", "**","Ait","Atx","Ac1","0 "};
static char *localColorSel_str[] = {"IT", "C0", "TX", "Ait", "Ac0", "Atx", "0 ","0 "};
static char *localAlphaSel_str[] = {"IT", "C0", "Z ", "W ", "Atx", "0 ","0 ","0 "};
static char *mSelectFbiRGB_str[] = {"0","Loc","AOth","ALoc",
				"Atex","Ctex","0","Msel7"};
static char *mSelectFbiA_str[] = {"0","Loc","AOth","ALoc",
				"Atex","Ait","Ac1","Msel7"};
static char *addLocalFbi_str[] = {"0","ALoc","Cloc","TX"};
static char *invertOther_str[] = {"+","-","1-","-.5+"};
static char *invertLocal_str[] = {"-","+","1-","-.5+"};


//----------------------------------------------------------------------
// subroutine to print SST textureMode info
//----------------------------------------------------------------------
static char blanks[] = "";

static char *tComposite_str(char *buf, FxU32 tMode,
				char *names_mSelect[], char *names_addlocal[])
{
    sprintf(buf,"%s(%s%s)*%s%s+%s",
		tMode & SST_TC_INVERT_OUTPUT ? "!" : blanks,
		tMode & SST_TC_ZERO_OTHER ? "0" : "Oth",
		tMode & SST_TC_SUB_CLOCAL ? "-Loc" : blanks,
		tMode & SST_TC_REVERSE_BLEND ? blanks : "~",
		names_mSelect[(tMode & SST_TC_MSELECT)>>SST_TC_MSELECT_SHIFT],
		names_addlocal[(((tMode & SST_TC_ADD_CLOCAL)!=0) << 1) |
				((tMode & SST_TC_ADD_ALOCAL)!=0)]);
    return buf;
}

static char *ccu_str(FxU32 combineMode, FxU32 tMode, 
			char *names_mSelect[], char *names_addlocal[])
{
    static char buf[80];
    tMode <<= SST_TCOMBINE_SHIFT;
    tMode &= SST_TCOMBINE;
    if (combineMode & SST_CM_USE_COMBINE_MODE) {
	sprintf(buf,"%s((%s%s)+(%s%s))*%s%s+%s%s << %d\n\t\t\t\t\t ",
		tMode & SST_TC_INVERT_OUTPUT ? "!" : blanks,
		invertOther_str[(combineMode & SST_CM_TC_INVERT_OTHER)>>SST_CM_TC_INVERT_OTHER_SHIFT],
		tMode & SST_TC_ZERO_OTHER ? "0" : "Oth",
		invertLocal_str[(combineMode & SST_CM_TC_INVERT_LOCAL)>>SST_CM_TC_INVERT_LOCAL_SHIFT],
		tMode & SST_TC_SUB_CLOCAL ? "Loc" : "0",
		tMode & SST_TC_REVERSE_BLEND ? blanks : "~",
		names_mSelect[(tMode & SST_TC_MSELECT)>>SST_TC_MSELECT_SHIFT],
		combineMode & SST_CM_TC_INVERT_ADD_LOCAL ? "1-" : blanks,
		names_addlocal[(((tMode & SST_TC_ADD_CLOCAL)!=0) << 1) |
				((tMode & SST_TC_ADD_ALOCAL)!=0)],
		(combineMode & SST_CM_TC_OUTSHIFT) >> SST_CM_TC_OUTSHIFT_SHIFT);
    }
    else
	return tComposite_str(buf,tMode,names_mSelect,names_addlocal);
    return buf;
}

static char *acu_str(FxU32 combineMode, FxU32 tMode, 
			char *names_mSelect[], char *names_addlocal[])
{
    static char buf[80];
    tMode <<= SST_TCOMBINE_SHIFT;
    tMode &= SST_TCOMBINE;
    if (combineMode & SST_CM_USE_COMBINE_MODE) {
	sprintf(buf,"%s((%s%s)+(%s%s))*%s%s+%s%s << %d",
		tMode & SST_TC_INVERT_OUTPUT ? "!" : blanks,
		invertOther_str[(combineMode & SST_CM_TCA_INVERT_OTHER)>>SST_CM_TCA_INVERT_OTHER_SHIFT],
		tMode & SST_TC_ZERO_OTHER ? "0" : "Oth",
		invertLocal_str[(combineMode & SST_CM_TCA_INVERT_LOCAL)>>SST_CM_TCA_INVERT_LOCAL_SHIFT],
		tMode & SST_TC_SUB_CLOCAL ? "Loc" : "0",
		tMode & SST_TC_REVERSE_BLEND ? blanks : "~",
		names_mSelect[(tMode & SST_TC_MSELECT)>>SST_TC_MSELECT_SHIFT],
		combineMode & SST_CM_TCA_INVERT_ADD_LOCAL ? "1-" : blanks,
		names_addlocal[(((tMode & SST_TC_ADD_CLOCAL)!=0) << 1) |
				((tMode & SST_TC_ADD_ALOCAL)!=0)],
		(combineMode & SST_CM_TCA_OUTSHIFT) >> SST_CM_TCA_OUTSHIFT_SHIFT);
    }
    else
	return tComposite_str(buf,tMode,names_mSelect,names_addlocal);
    return buf;
}

static void printTextureMode(SstRegs *tmu)
{
    unsigned long tMode, tLOD;
    TmuData *td = TMU_PRIVATE(tmu);
    static char *otherColorSel_str[] = {"CTXoth", "ATXoth", "CTX", "Atx",
					"IT","Ait","Ccr","Acr"};
    static char *otherAlphaSel_str[] = {"ATXoth", "Atx", "Ait", "Acr"};
    static char *localColorSel_str[] = {"CTX", "Atx", "CTXoth", "ATXoth",
					"IT", "Ait","Cck","Ack"};
    static char *localAlphaSel_str[] = {"ATX", "ATXoth", "Ait ", "Ack"};
    static char *mSelectTrexRGB_str[] = {"0","Loc","AOth","ALoc",
					 "LOD","LodF","0","Msel7"};
    static char *mSelectTrexA_str[] = { "0","Loc","AOth","ALoc",
					"LOD","LodF","Ait","Acr"};
    static char *mSelect7RGB_str[] = {"CTX","0","CTXOther","00","IT","Ait","Ccr","Acr"};
    static char *addLocalTrex_str[] = {"0","ALoc","Loc","IT"};

    tMode = tmu->textureMode;
    tLOD = tmu->tLOD;
    if (!(SST_TREX_ACTIVE(tMode) ||			// if not active
	(tmu->combineMode & SST_CM_USE_COMBINE_MODE)))	// or if using new combineMode
		return;

    gdbg_printf("\t texM%d: %5s filt:%s,%s %4s %s NCC=%d %6s texAddr=%x.%s\n",
		td->myNumber,
		tMode & SST_TPERSP_ST ? "PERSP" : "AFFIN",
		tMode & SST_TMINFILTER ? "BIL":"PNT",
		tMode & SST_TMAGFILTER ? "BIL":"PNT",
		tMode & SST_TLODDITHER ? "LDIT" : blanks,
		tfmt_str[(tMode & SST_TFORMAT)>>SST_TFORMAT_SHIFT],
		tMode & SST_TNCCSELECT ? 1 : 0,
		tMode & SST_TRILINEAR ? "TRILIN":blanks,
		SST_TEXTURE_UNMUNGE_ADDRESS(tmu->texBaseAddr),
		tmu->texBaseAddr & SST_TEXTURE_IS_TILED ? "T" : "L");
    if (tmu->tLOD & SST_TMULTIBASEADDR)
	gdbg_printf("\t\ttexAddr1 = %x  texAddr2 = %x  texAddr38 = %x\n",
		tmu->texBaseAddr1,tmu->texBaseAddr2,tmu->texBaseAddr38);

    gdbg_printf("\t\tCLAMP:%s,%s,%s M:%s,%s %4s  RGB=%s A=%s\n",
		tMode & SST_TCLAMPS ? "S":"-",
		tMode & SST_TCLAMPT ? "T":"-",
		tMode & SST_TCLAMPW ? "W":"-",
		tLOD & SST_TMIRRORS ? "S" : "-",
		tLOD & SST_TMIRRORT ? "T" : "-",
		blanks,
		ccu_str(tmu->combineMode,tMode>>SST_TCOMBINE_SHIFT,
			mSelectTrexRGB_str,addLocalTrex_str),
		acu_str(tmu->combineMode,tMode>>SST_TACOMBINE_SHIFT,
			mSelectTrexA_str,addLocalTrex_str));
    if (tmu->tDetail & SST_TFILTER_SEPARATE) {
	gdbg_printf("\t\tSFilter rgb:%s,%s  a:%s,%s\n",
			tmu->tDetail & SST_TMINFILTER_RGB ? "BIL":"PNT",
			tmu->tDetail & SST_TMAGFILTER_RGB ? "BIL":"PNT",
			tmu->tDetail & SST_TMINFILTER_A ? "BIL":"PNT",
			tmu->tDetail & SST_TMAGFILTER_A ? "BIL":"PNT");
    }
    if (tmu->combineMode & SST_CM_USE_COMBINE_MODE) {
	FxU32 cm = tmu->combineMode;
	gdbg_printf("\t\tCM: Other=%s,%s  Local=%s,%s    Msel7=%s\n",
		otherColorSel_str[(cm & SST_CM_TC_OTHERSELECT)>>SST_CM_TC_OTHERSELECT_SHIFT],
		otherAlphaSel_str[(cm & SST_CM_TCA_OTHERSELECT)>>SST_CM_TCA_OTHERSELECT_SHIFT],
		localColorSel_str[(cm & SST_CM_TC_LOCALSELECT)>>SST_CM_TC_LOCALSELECT_SHIFT],
		localAlphaSel_str[(cm & SST_CM_TCA_LOCALSELECT)>>SST_CM_TCA_LOCALSELECT_SHIFT],
		mSelect7RGB_str[(cm & SST_CM_TC_MSELECT_7)>>SST_CM_TC_MSELECT_7_SHIFT]
		);
    }

#ifndef NO_FLOAT
/* NO_FLOAT
 * another printf that's unavailable in NO_FLOAT mode
 */

// sign extend LOD values and convert to 4.2 format
#define PLOD(x) 0.25F*(float)(((x)<<(32-SST_LOD_SIZE))>>(32-SST_LOD_SIZE))
#define PLODU(x) 0.25F*(float)(x)
    gdbg_printf("\t   lod: min=%.2f max=%.2f Bias=%.2f %3s %5s %s=1:%d %5s %4s %4s\n",
		PLODU((tLOD & SST_LODMIN)>>SST_LODMIN_SHIFT),
		PLODU((tLOD & SST_LODMAX)>>SST_LODMAX_SHIFT),
		PLOD((tLOD & SST_LODBIAS)>>SST_LODBIAS_SHIFT),
		tLOD & SST_LOD_ODD ? "ODD" : "EVN",
		tLOD & SST_LOD_TSPLIT ? "TSPLT" : blanks,
		tLOD & SST_LOD_S_IS_WIDER ? "T:S" : "S:T",
		1<<((tLOD & SST_LOD_ASPECT) >> SST_LOD_ASPECT_SHIFT),
		tLOD & SST_LOD_ZEROFRAC ? "ZFRAC" : blanks,
		tLOD & SST_TDATA_SWIZZLE ? "SWIZ" : blanks,
		tLOD & SST_TDATA_SWAP ? "SWAP" : blanks);
#else
    gdbg_printf("\t   lod: floating point output unavailable in a VxD\n");
#endif /* #ifndef NO_FLOAT */

    if (((tMode & SST_TC_MSELECT) == SST_TC_MLOD) ||
	((tMode & SST_TCA_MSELECT) == SST_TCA_MLOD))
    gdbg_printf("\tdetail: bias=%d shift=%x clamp=%2x\n",
	SIGN_EXTEND((tmu->tDetail & SST_DETAIL_BIAS)>>SST_DETAIL_BIAS_SHIFT,6),
		(tmu->tDetail & SST_DETAIL_SCALE)>>SST_DETAIL_SCALE_SHIFT,
		(tmu->tDetail & SST_DETAIL_MAX)>>SST_DETAIL_MAX_SHIFT);
}

//----------------------------------------------------------------------
// print SST mode information
//----------------------------------------------------------------------
void sstPrintModes(SstRegs *sst, char *msg, int cmdCode)
{
    int i;
    int rm = sst->renderMode;
    int sm = sst->stencilMode;
    int so = sst->stencilOp;
    int fbz = sst->fbzMode;
    int fbzCP = sst->fbzColorPath;
    int fog = sst->fogMode;
    int alp = sst->alphaMode;
    CsimPrivate *cpriv = CSIM_PRIVATE(sst);

#ifdef CVG
    gdbg_info(125,"%s %s    status=0x%x %6s yTop=%d\n",
		msg,cmdNames[cmdCode],
		sst->status,
		sst->fbiInit3 & SST_ALT_REGMAPPING ? "ALTMAP" : blanks,
		(sst->fbiInit3 & SST_YORIGIN_TOP)>>SST_YORIGIN_TOP_SHIFT);
#else // H3
    gdbg_info(125,"%s %s    status=0x%x %6s yTop=%d\n",
		msg,cmdNames[cmdCode],
		sst->status,
		cpriv->io.miscInit0 & SST_ALT_REGMAPPING ? "ALTMAP" : blanks,
		(sst->renderMode & SST_RM_YORIGIN_SELECT) ?
		  (sst->renderMode & SST_RM_YORIGIN_TOP) >> SST_RM_YORIGIN_TOP_SHIFT :
		  (cpriv->io.miscInit0 & SST_YORIGIN_TOP)>>SST_YORIGIN_TOP_SHIFT);
#endif
    
    for (i = cpriv->info->numberTmus-1; i>=0; i--)
	printTextureMode(cpriv->trex+i);

    if (fbz & SST_ENRECTCLIP) {
	gdbg_printf("\tclipRc0: %d,%d to %d,%d",
		    HIWORD(sst->clipLeftRight),HIWORD(sst->clipBottomTop),
		    LOWORD(sst->clipLeftRight),LOWORD(sst->clipBottomTop));
	if (sst->renderMode & SST_RM_ENGUARDBAND)
	    gdbg_printf(" GBAND");
    }
    if (fbz & SST_ENCHROMAKEY) {
	if (!(fbz & SST_ENRECTCLIP))
	    gdbg_printf("\t\t\t\t");
	if (sst->chromaRange & SST_ENCHROMARANGE) 
	    gdbg_printf("\tchromaKey %s : %02x-%02x %02x-%02x %02x-%02x\n",
			sst->chromaRange & SST_CHROMARANGE_BLOCK_OR ? "||" : "&&",
			(sst->chromaKey>>16)&0xFF,(sst->chromaRange>>16)&0xFF,
			(sst->chromaKey>>8)&0xFF,(sst->chromaRange>>8)&0xFF,
			sst->chromaKey&0xFF,sst->chromaRange&0xFF);
	else
	    gdbg_printf("\tchromaKey: %02x %02x %02x\n",
			(sst->chromaKey>>16)&0xFF,
			(sst->chromaKey>>8)&0xFF,
			sst->chromaKey&0xFF);
    }
    else if (fbz & SST_ENRECTCLIP)
	gdbg_printf("\n");
#ifndef CVG
    if (sst->clipLeftRight1 & SST_ENRECTCLIP1) {
	gdbg_printf("\tclipRc1: %d,%d to %d,%d %s\n",
		    HIWORD(sst->clipLeftRight1)&0xFFF,HIWORD(sst->clipBottomTop1)&0xFFF,
		    LOWORD(sst->clipLeftRight1),LOWORD(sst->clipBottomTop1),
		    sst->clipBottomTop1 & SST_RECTCLIP1_EX ? "exclusive" : "inclusive");
    }
#endif

    gdbg_printf(
"\trenMod: %2s-bpp  alpha=%4s\tRGBAmasks=%d%d%d%d\n",
		    rm_3d_str[(rm & SST_RM_3D_MODE)>>SST_RM_3D_SHIFT],
		    rm_arm_str[(rm & SST_RM_ALPHAMODE)>>SST_RM_ALPHAMODE_SHIFT],
		    (rm & SST_RM_RED_WMASK) != 0,
		    (rm & SST_RM_GREEN_WMASK) != 0,
		    (rm & SST_RM_BLUE_WMASK) != 0,
		    (rm & SST_RM_ALPHA_WMASK) != 0 );
    gdbg_printf(
"\tstnMod: %2s  ref=%02x mask=%02x wmask=%02x func:%s%s%s\n",
		    (sm & SST_STENCIL_ENABLE) ? "EN" : blanks,
		    (sm & SST_STENCIL_REF) >> SST_STENCIL_REF_SHIFT,
		    (sm & SST_STENCIL_MASK) >> SST_STENCIL_MASK_SHIFT,
		    (sm & SST_STENCIL_WMASK) >> SST_STENCIL_WMASK_SHIFT,
		    sm & SST_SFUNC_LT ? "<" : blanks,
		    sm & SST_SFUNC_GT ? ">" : blanks,
		    sm & SST_SFUNC_EQ ? "=" : blanks);
    if (sm & SST_STENCIL_ENABLE) {
	gdbg_printf(
	"\tstenOp: sfail:%s\tzfail:%s\tzpass:%s\n",
		    so_str[(so & SST_STENCIL_SFAIL_OP)>>SST_STENCIL_SFAIL_OP_SHIFT],
		    so_str[(so & SST_STENCIL_ZFAIL_OP)>>SST_STENCIL_ZFAIL_OP_SHIFT],
		    so_str[(so & SST_STENCIL_ZPASS_OP)>>SST_STENCIL_ZPASS_OP_SHIFT]);
    }

    gdbg_printf(
"\tfbzMod: %4s %4s %4s %4s%1s %s(%s%s):%s%s%s %3s%s wm:%3s %2s buf:%4s%2s %1s%1s%1s\n",
		    fbz & SST_ENRECTCLIP ? "CLIP" : blanks,
		    fbz & SST_ENCHROMAKEY ? "CKEY" : blanks,
		    fbz & SST_ENALPHAMASK ? "AMSK" : blanks,
		    fbz & SST_ENSTIPPLE ? "STIP" : blanks,
		    fbz & SST_ENSTIPPLEPATTERN ? "+" : blanks,
		    fbz & SST_ENDEPTHBUFFER ? "ENZ" : "noz",
		    fbz & SST_WBUFFER ?
			(fbz & SST_DEPTH_FLOAT_SEL ? "fz" : "w"): "z",
		    fbz & SST_ENZBIAS ? "+" : blanks,
		    fbz & SST_ZFUNC_LT ? "<" : blanks,
		    fbz & SST_ZFUNC_GT ? ">" : blanks,
		    fbz & SST_ZFUNC_EQ ? "=" : blanks,
		    fbz & SST_ENDITHER ? "DIT" : blanks,
		    fbz & SST_DITHER2x2 ? "2x2" : "4x4",
		    fbz & SST_RGBWRMASK ? "RGB" : blanks,
		    fbz & SST_ZAWRMASK ? "ZA" : blanks,
#ifdef CVG
		    fbz & SST_DRAWBUFFER_BACK ? "BACK" : "FRNT",
#else // H3
                    "COLR",
#endif
		    fbz & SST_ENALPHABUFFER ? "+A" : blanks,
		    fbz & SST_YORIGIN ? "~Y" : blanks,
		    fbz & SST_ENDITHERSUBTRACT ? "~S" : blanks,
		    fbz & SST_ZCOMPARE_TO_ZACOLOR ? "~Z" : blanks);
    gdbg_printf("\tfbzCol: %2s %2s %2s %2s oth=%2s,%2s loc=%2s,%2s\tRGB=%s A=%s\n",
		fbzCP & SST_PARMADJUST ? "Pj" : blanks,
		fbzCP & SST_RGBAZ_CLAMP ? "Cl" : blanks,
		fbzCP & SST_ENTEXTUREMAP ? "TX" : blanks,
		fbzCP & SST_ENANTIALIAS ? "AA" : blanks,
		otherColorSel_str[(fbzCP & SST_RGBSELECT)>>SST_RGBSELECT_SHIFT],
		otherColorSel_str[(fbzCP & SST_ASELECT)>>SST_ASELECT_SHIFT],
		fbzCP & SST_LOCALSELECT_OVERRIDE_WITH_ATEX ? "@A" : 
		localColorSel_str[(fbzCP & SST_LOCALSELECT)>>SST_LOCALSELECT_SHIFT],
		localAlphaSel_str[(fbzCP & SST_ALOCALSELECT)>>SST_ALOCALSELECT_SHIFT],
		ccu_str(sst->combineMode,fbzCP>>SST_CCOMBINE_SHIFT,
			mSelectFbiRGB_str,addLocalFbi_str),
		acu_str(sst->combineMode,fbzCP>>SST_CACOMBINE_SHIFT,
			mSelectFbiA_str,addLocalFbi_str));
    if (sst->combineMode & SST_CM_USE_COMBINE_MODE) {	// if using new combineMode register
	FxU32 cm = sst->combineMode;
	static char *mSelect7RGB_str[] = {"IT","C1","Ait","Ac1"};
	gdbg_printf("\tcombMD: Other=%s,%s  Local=%s,%s    Msel7=%s\n",
		otherColorSel_str[(cm & SST_CM_CC_OTHERSELECT)>>SST_CM_CC_OTHERSELECT_SHIFT],
		otherColorSel_str[(cm & SST_CM_CCA_OTHERSELECT)>>SST_CM_CCA_OTHERSELECT_SHIFT],
		localColorSel_str[(cm & SST_CM_CC_LOCALSELECT)>>SST_CM_CC_LOCALSELECT_SHIFT],
		localAlphaSel_str[(cm & SST_CM_CCA_LOCALSELECT)>>SST_CM_CCA_LOCALSELECT_SHIFT],
		mSelect7RGB_str[(cm & SST_CM_CC_MSELECT_7)>>SST_CM_CC_MSELECT_7_SHIFT]
		);
    }
  
    gdbg_printf("\talphaM: %5s:%1s%1s%1s%02x %s rgb: %4s,%s%s,%-4s a:%4s,%s%s,%-4s\n",
		    alp & SST_ENALPHAFUNC ? "AFUNC" : "AFoff",
		    alp & SST_ALPHAFUNC_LT ? "<" : blanks,
		    alp & SST_ALPHAFUNC_GT ? ">" : blanks,
		    alp & SST_ALPHAFUNC_EQ ? "=" : blanks,
		    ((unsigned)alp&SST_ALPHAREF)>>SST_ALPHAREF_SHIFT,
		    alp & SST_ENALPHABLEND ? "ABLEND" : "no-ablend",
 		    srcFactor_str[(alp&SST_RGBSRCFACT)>>SST_RGBSRCFACT_SHIFT],
		    fog & SST_RGB_BLEND_SUB ? "-" : "+",
		    fog & SST_RGB_BLEND_REVERSE ? "r" : "",
		    dstFactor_str[(alp&SST_RGBDSTFACT)>>SST_RGBDSTFACT_SHIFT],
 		    srcFactor_str[(alp&SST_ASRCFACT)>>SST_ASRCFACT_SHIFT],
		    fog & SST_A_BLEND_SUB ? "-" : "+",
		    fog & SST_A_BLEND_REVERSE ? "r" : "",
		    dstFactor_str[(alp&SST_ADSTFACT)>>SST_ADSTFACT_SHIFT]);

    if (fog & SST_ENFOGGING)
	gdbg_printf("\tfogMod: %3s %4s %5s %5s %3s %5s RGB = %02x %02x %02x\n",
		    fog & SST_FOGADD ? "ADD" : blanks,
		    fog & SST_FOGMULT ? "MULT" : blanks,
		    (fog & (SST_FOG_ALPHA|SST_FOG_Z))==(SST_FOG_ALPHA|SST_FOG_Z) ? "W_LIN" :
			(fog & SST_FOG_ALPHA ? "ALPHA" : 
			(fog & SST_FOG_Z ? "Z" : blanks)),
		    fog & SST_FOG_CONSTANT ? "CONST" : blanks,
		    fog & SST_FOG_DITHER ? "DIT" : blanks,
		    fog & SST_FOG_ZONES ? "ZONES" : blanks,
		    (sst->fogColor>>16)&0xFF,
		    (sst->fogColor>>8)&0xFF,
		    sst->fogColor&0xFF);
}

#define PE(e)	printFix("%4d.%1x",e,SST_E_SIZE,SST_XY_FRACBITS)
#define PC(c)	printFix("%4d.%03x",c,SST_RGBA_SIZE,SST_RGBA_FRACBITS)
#define PZ(c)	((sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP)?\
		printFix64("%8x.%05x",c,SST_Z64_SIZE,SST_Z64_FRACBITS_32BPP):\
		printFix64("%6x.%07x",c,SST_Z64_SIZE,SST_Z64_FRACBITS_16BPP)
#define PST(w)	printFix64("%4d.%08x",w,SST_ST64_SIZE,SST_ST64_FRACBITS)
#define PW(w)	printFix64("%3d.%08x",w,SST_W64_SIZE,SST_W64_FRACBITS)
#define PJIM(x) (int)((x>>32)&0xFFFF),(int)((x>>16)&0xFFFF),(int)x&0xFFFF

// print out a fixed point number with a specified number of fraction bits
// the hard part is to handle negative numbers, especially -0.fraction
// also returns a static char buffer (one of 16 in a circle)
static char *printFix(char *fmt, int val, int size, int fracbits)
{
    char *pbuf;
    static char buf[16][32];
    static int nbuf;

    nbuf++; if (nbuf==16) nbuf=0;	// circulate the buffers
    pbuf = buf[nbuf];			// get current buffer
    if (!(val & (1<<(size-1))))			// if positive, then easy
	sprintf(pbuf, fmt, (val >> fracbits) & SST_MASK(size-fracbits),
				val & SST_MASK(fracbits));
    else {					// else negative, then tricky
	int i = ((-val)>>fracbits) & SST_MASK(size-fracbits);
	if (fmt[2] == 'x')
	    i &= 0xFFFFFFFF >> (32-(fmt[1] - '0')*4);
	else
	    i = -i;
	sprintf(pbuf, fmt, i, (-val) & SST_MASK(fracbits));
	if (i == 0 || fmt[2] == 'x') {		// special case -0.fraction
	    for (i=0; i<sizeof(buf[0]); i++) {	// search for 1st char
		if (pbuf[i] != ' ') {
		    if (i==0)
			GDBG_ERROR("printFix", "i==0\n");
		    else
			pbuf[i-1] = '-';	// add a '-' in front of it
		    break;
		}
	    }
	}
    }
    return pbuf;
}

static char *printFix64(char *fmt, FxI64 val, int size, int fracbits)
{
    char *pbuf;
    static char buf[16][32];
    static int nbuf;

    nbuf++; if (nbuf==16) nbuf=0;	// circulate the buffers
    pbuf = buf[nbuf];			// get current buffer
    // assume that FxI64 sign bit is in upper 32 bits (otherwise why bother?)
    if (!(FX_HI64(val) & (1<<(size-33)))) { // if positive, then easy
	sprintf(pbuf, fmt,		// NOTE: fracbits must be <= 32
		FX_LO64(FX_SHR64(val,fracbits)) & SST_MASK(size-fracbits),
		FX_LO64(val) & SST_MASK(fracbits));
    }
    else {					// else negative, then tricky
	int i;
	FxI64 negval;
	negval = FX_NEG64(val);			// display the negative of it
	i = FX_LO64(FX_SHR64(negval,fracbits));	// integer portion
	i &= SST_MASK(size-fracbits);
	if (fmt[2] == 'x')			// detect %#x format
	    i &= 0xFFFFFFFF >> (32-(fmt[1] - '0')*4);
	else
	    i = -i;				// go back to original integer
	sprintf(pbuf, fmt, i, FX_LO64(negval) & SST_MASK(fracbits));
	if (i == 0 || fmt[2] == 'x') {		// special case -0.fraction
	    for (i=0; i<sizeof(buf[0]); i++) {	// search for 1st char
		if (pbuf[i] != ' ') {
		    if (i==0)
			GDBG_ERROR("printFix64", "i==0\n");
		    else
			pbuf[i-1] = '-';	// add a '-' in front of it
		    break;
		}
	    }
	}
    }
    return pbuf;
}

void sstPrintRegs(SstRegs *sst, char *msg)
{
    CsimPrivate *cpriv = CSIM_PRIVATE(sst);

    gdbg_printf("%s",msg);
    gdbg_printf("\t    x,y = %4d,%4d\tstipple: %08x\n",
		SIGN_EXTEND(cpriv->fbiData.spanFbi.x,SST_XY_INTBITS),
		SIGN_EXTEND(cpriv->fbiData.spanFbi.y,SST_XY_INTBITS),
		sst->stipple);
    gdbg_printf("\tedge0.e = %s\tdedx  =%s\t dedy =%s\n",
		PE(cpriv->fbiData.edge0.e),
		PE(cpriv->fbiData.edge0.dedx),
		PE(cpriv->fbiData.edge0.dedy));
    gdbg_printf("\tedge1.e = %s\tdedx  =%s\t dedy =%s\n",
		PE(cpriv->fbiData.edge1.e),
		PE(cpriv->fbiData.edge1.dedx),
		PE(cpriv->fbiData.edge1.dedy));

    gdbg_printf("\t      z =%s\t.rgba =%s%s%s%s\n",
		PZ(cpriv->fbiData.z64),
		PC(sst->r),PC(sst->g),PC(sst->b),PC(sst->a));
    gdbg_printf("\t   dzdx =%s\t.d*dx =%s%s%s%s\n",
		PZ(cpriv->fbiData.dzdx64),
		PC(sst->drdx),PC(sst->dgdx),PC(sst->dbdx),PC(sst->dadx));
    gdbg_printf("\t   dzdy =%s\t.d*dy =%s%s%s%s\n",
		PZ(cpriv->fbiData.dzdy64),
		PC(sst->drdy),PC(sst->dgdy),PC(sst->dbdy),PC(sst->dady));
    if ((sst->fbzColorPath & SST_ALOCALSELECT) == SST_ALOCAL_W) {
	gdbg_printf("\t      w =%s\tdwdx,y = %s %s\n",
		   PST(cpriv->fbiData.w64),PST(cpriv->fbiData.dwdx64),PW(cpriv->fbiData.dwdy64));
    }
    if (sst->fbzColorPath & SST_ENTEXTUREMAP) {
	unsigned int i;
	for (i = 0; i < cpriv->info->numberTmus; i++) {
	    TmuData *td = TMU_PRIVATE(cpriv->trex+i);
	    if (!SST_TREX_ACTIVE(cpriv->trex[i].textureMode)) continue;
	    gdbg_printf("\tTREX%d:\t\t\t.stw  =%s %s %s\n",
		    td->myNumber,
		    PST(td->s64),PST(td->t64),PW(td->w64));
	    gdbg_printf("\t\t\t\t.d*dx =%s %s %s\n",
		    PST(td->dsdx64),PST(td->dtdx64),PW(td->dwdx64));
	    gdbg_printf("\t\t\t\t.d*dy =%s %s %s\n",
		    PST(td->dsdy64),PST(td->dtdy64),PW(td->dwdy64));
	    if (halInfo.hsim) {
      #ifndef __WATCOMC__
		gdbg_printf("\t\t\tJIM.stw  =%04x_%04x_%04x %04x_%04x_%04x %04x_%04x_%04x\n",
		    PJIM(td->s64),PJIM(td->t64),PJIM(td->w64));
		gdbg_printf("\t\t\tJIM.d*dx =%04x_%04x_%04x %04x_%04x_%04x %04x_%04x_%04x\n",
		    PJIM(td->dsdx64),PJIM(td->dtdx64),PJIM(td->dwdx64));
		gdbg_printf("\t\t\tJIM.d*dy =%04x_%04x_%04x %04x_%04x_%04x %04x_%04x_%04x\n",
		    PJIM(td->dsdy64),PJIM(td->dtdy64),PJIM(td->dwdy64));
      #endif
	    }
	}
    }
    else gdbg_printf("\n");
}

// print out the span iteration registers
void sstPrintSpanRegs(SstRegs *sst, char *msg, int level)
{
    CsimPrivate *cpriv = CSIM_PRIVATE(sst);

    FXUNUSED(msg);
    gdbg_info(level,"span.xy = %4d,%d\n",
			SIGN_EXTEND(cpriv->fbiData.spanFbi.x,SST_XY_INTBITS),
			SIGN_EXTEND(cpriv->fbiData.spanFbi.y,SST_XY_INTBITS));
    gdbg_printf("\tedge0.e = %s\tdedx  =%s\t dedy =%s\n",
		PE(cpriv->fbiData.edge0.e),
		PE(cpriv->fbiData.edge0.dedx),
		PE(cpriv->fbiData.edge0.dedy));
    gdbg_printf("\tedge1.e = %s\tdedx  =%s\t dedy =%s\n",
		PE(cpriv->fbiData.edge1.e),
		PE(cpriv->fbiData.edge1.dedx),
		PE(cpriv->fbiData.edge1.dedy));

    if (sst->fbzColorPath & SST_ENTEXTUREMAP) {
	unsigned int i;
	for (i = 0; i < cpriv->info->numberTmus; i++) {
	    TmuData *td = TMU_PRIVATE(cpriv->trex+i);
	    gdbg_printf("\ttrex%d.stw = %s %s %s\n",
		    td->myNumber,
		    PST(td->spanTrex.s64),PST(td->spanTrex.t64),
		    PW(td->spanTrex.w64));
	    if (halInfo.hsim) {
       #ifndef __WATCOMC__
		gdbg_printf("\ttJIM.stw = %04x_%04x_%04x %04x_%04x_%04x %04x_%04x_%04x\n",
		    PJIM(td->spanTrex.s64),PJIM(td->spanTrex.t64),
		    PJIM(td->spanTrex.w64));
       #endif
           }
       }
    }
    gdbg_printf("\t.rgba =%s%s%s%s\n",
		PC(cpriv->fbiData.spanFbi.r),PC(cpriv->fbiData.spanFbi.g),
		PC(cpriv->fbiData.spanFbi.b),PC(cpriv->fbiData.spanFbi.a));

    if (sst->fbzMode & (SST_ENDEPTHBUFFER|SST_ZAWRMASK))
	gdbg_info(level+1,"\t\t\t.z    =%s\n",
		(sst->fbzMode & SST_DEPTH_FLOAT_SEL)||!(sst->fbzMode & SST_WBUFFER) ?
			PZ(cpriv->fbiData.spanFbi.z64) :
			PW(cpriv->fbiData.spanFbi.w64));
}

//----------------------------------------------------------------------
// print SST statistics
// NOTE: negative verbose levels reset the stats every call
//----------------------------------------------------------------------
void sstPrintStats(SstRegs *sst)
{
    int avl, vl, pixin;
    CsimPrivate *cpriv = CSIM_PRIVATE(sst);

    if (!GDBG_GET_DEBUGLEVEL(106)) return; // quick exit

    vl = avl = cpriv->environment.statsVerboseLevel;
    if (vl == 0) return;

    if (avl < 0) avl = -avl;		// abs(vl)
    pixin = sst->stats.fbiPixelsIn;
    if (pixin == 0) pixin = 1;		// prevent divide by 0
    gdbg_info(106,"SST statistics:\n");

#define P(x) x,x*100/pixin
    gdbg_printf("\tFBI pixels in:  %6d %3d%%\n",P(pixin));
    gdbg_printf("\tFBI chroma fail:%6d %3d%%\n",P(sst->stats.fbiChromaFail));
    gdbg_printf("\tFBI stencil fail: %6d %3d%%\n",P(sst->fbiStencilFail));
    gdbg_printf("\tFBI zfunc fail: %6d %3d%%\n",P(sst->stats.fbiZfuncFail));
    gdbg_printf("\tFBI afunc fail: %6d %3d%%\n",P(sst->stats.fbiAfuncFail));
    gdbg_printf("\tFBI pixels out: %6d %3d%%\n",P(sst->stats.fbiPixelsOut));
    gdbg_printf("\n");

    if (vl < 0) {	// reset the statistics
	memset((void *)&sst->stats,0,sizeof(sst->stats));
    	cpriv->environment.statsVerboseLevel = vl;
    }
}
