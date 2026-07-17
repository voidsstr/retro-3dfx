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
** $Revision: 4$
** $Date: 10/11/00 8:08:55 PM$
*/

#include <h3.h>

//----------------------------------------------------------------------
// macros for creating assembler offset files
//----------------------------------------------------------------------

#define NEWLINE	printf("\n")
#define COMMENT	printf("/*----------------------------------------------------------------------*/\n")

#define HEADER(str)	NEWLINE; COMMENT; \
			printf("/* Assembler offsets for %s struct\t\t\t\t\t*/\n",str);\
			COMMENT; NEWLINE

#define OFFSET(p,o,pname) if (hex) \
	printf("#define\t%s\t0x%08x\n",pname,((int)&p.o)-(int)&p); \
    else printf("#define\t%s\t%10d\n",pname,((int)&p.o)-(int)&p)

#define SIZEOF(p,pname) if (hex) \
	printf("#define\tSIZEOF_%s\t0x%08x\n",pname,sizeof(p)); \
    else printf("#define\tSIZEOF_%s\t%10d\n",pname,sizeof(p))


main (argc)
{
    int hex=0;		/* default is print in decimal	*/
    SstRegs sst;
    SstGRegs sstg;
    SstCRegs sstc;
    SstIORegs sstio;

    if (argc > 1) hex = 1;

    HEADER ("SSTIO");
    OFFSET (sstio,status,"STATUS\t");
    OFFSET (sstio,pciInit0,"PCIINIT0");
    OFFSET (sstio,sipMonitor,"SIPMONITOR");
    OFFSET (sstio,lfbMemoryConfig,"LFBMEMORYCONFIG");
    OFFSET (sstio,miscInit0,"MISCINIT0");
    OFFSET (sstio,miscInit1,"MISCINIT1");
    OFFSET (sstio,dramInit0,"DRAMINIT0");
    OFFSET (sstio,dramInit1,"DRAMINIT1");
    NEWLINE;

    OFFSET (sstio,agpInit,"AGPINIT");
    OFFSET (sstio,tmuGbeInit,"TMUGBEINIT");
    OFFSET (sstio,vgaInit0,"VGAINIT0");
    OFFSET (sstio,vgaInit1,"VGAINIT1");
    OFFSET (sstio,dramCommand,"DRAMCOMMAND");
    OFFSET (sstio,dramData,"DRAMDATA");
    OFFSET (sstio,strapInfo,"STRAPINFO");
    OFFSET (sstio,vidTvOutBlankVCount,"VIDTVOUTBLANKVCOUNT");
    NEWLINE;

    OFFSET (sstio,pllCtrl0,"PLLCTRL0");
    OFFSET (sstio,pllCtrl1,"PLLCTRL1");
    OFFSET (sstio,pllCtrl2,"PLLCTRL2");
    OFFSET (sstio,dacMode,"DACMODE\t");
    OFFSET (sstio,dacAddr,"DACADDR\t");
    OFFSET (sstio,dacData,"DACDATA\t");
    OFFSET (sstio,vidMaxRGBDelta,"VIDMAXRGBDELTA");
    OFFSET (sstio,vidProcCfg,"VIDPROCCFG");
    NEWLINE;

    OFFSET (sstio,hwCurPatAddr,"HWCURPATADDR");
    OFFSET (sstio,hwCurLoc,"HWCURLOC");
    OFFSET (sstio,hwCurC0,"HWCURC0\t");
    OFFSET (sstio,hwCurC1,"HWCURC1\t");
    OFFSET (sstio,vidInFormat,"VIDINFORMAT");
    OFFSET (sstio,vidTvOutBlankHCount,"VIDTVOUTBLANKHCOUNT");
    OFFSET (sstio,vidSerialParallelPort,"VIDSERIALPARALLELPORT");
    OFFSET (sstio,vidInXDecimDeltas,"VIDINXDECIMDELTAS");
    NEWLINE;

    OFFSET (sstio,vidInDecimInitErrs,"VIDINDECIMINITERRS");
    OFFSET (sstio,vidInYDecimDeltas,"VIDINYDECIMDELTAS");
    OFFSET (sstio,vidPixelBufThold,"VIDPIXELBUFTHOLD");
    OFFSET (sstio,vidChromaMin,"VIDCHROMAMIN\t");
    OFFSET (sstio,vidChromaMax,"VIDCHROMAMAX\t");
    OFFSET (sstio,vidCurrentLine,"VIDCURRENTLINE\t");
    OFFSET (sstio,vidScreenSize,"VIDSCREENSIZE\t");
    OFFSET (sstio,vidOverlayStartCoords,"VIDOVERLAYSTARTCOORDS");
    NEWLINE;

    OFFSET (sstio,vidOverlayEndScreenCoord,"VIDOVERLAYENDSCREENCOORD");
    OFFSET (sstio,vidOverlayDudx,"VIDOVERLAYDUDX\t\t");
    OFFSET (sstio,vidOverlayDudxOffsetSrcWidth,"VIDOVERLAYDUDXOFFSETSRCWIDTH");
    OFFSET (sstio,vidOverlayDvdy,"VIDOVERLAYDVDY\t\t");
    OFFSET (sstio,vgaRegister[0],"VGAREGISTER_0");
    OFFSET (sstio,vgaRegister[1],"VGAREGISTER_1");
    OFFSET (sstio,vgaRegister[2],"VGAREGISTER_2");
    OFFSET (sstio,vgaRegister[3],"VGAREGISTER_3");
    NEWLINE;

    OFFSET (sstio,vgaRegister[4],"VGAREGISTER_4");
    OFFSET (sstio,vgaRegister[5],"VGAREGISTER_5");
    OFFSET (sstio,vgaRegister[6],"VGAREGISTER_6");
    OFFSET (sstio,vgaRegister[7],"VGAREGISTER_7");
    OFFSET (sstio,vgaRegister[8],"VGAREGISTER_8");
    OFFSET (sstio,vgaRegister[9],"VGAREGISTER_9");
    OFFSET (sstio,vgaRegister[10],"VGAREGISTER_10");
    OFFSET (sstio,vgaRegister[11],"VGAREGISTER_11");
    NEWLINE;

    OFFSET (sstio,vidOverlayDvdyOffset,"VIDOVERLAYDVDYOFFSET");
    OFFSET (sstio,vidDesktopStartAddr,"VIDDESKTOPSTARTADDR");
    OFFSET (sstio,vidDesktopOverlayStride,"VIDDESKTOPOVERLAYSTRIDE");
    OFFSET (sstio,vidInAddr0,"VIDINADDR0\t");
    OFFSET (sstio,vidInAddr1,"VIDINADDR1\t");
    OFFSET (sstio,vidInAddr2,"VIDINADDR2\t");
    OFFSET (sstio,vidInStride,"VIDINSTRIDE\t");
    OFFSET (sstio,vidCurrOverlayStartAddr,"VIDCURROVERLAYSTARTADDR");
    NEWLINE;

    SIZEOF (sstio,"SSTIO");

    HEADER ("SSTC");
    OFFSET (sstc,agpReqSize,"AGPREQSIZE");
    OFFSET (sstc,hostAddrLow,"HOSTADDRLOW");
    OFFSET (sstc,hostAddrHigh,"HOSTADDRHIGH");
    OFFSET (sstc,graphicsAddr,"GRAPHICSADDR");
    OFFSET (sstc,graphicsStride,"GRAPHICSSTRIDE");
    OFFSET (sstc,moveCMD,"MOVECMD\t");
    OFFSET (sstc,reservedL[0],"RESERVEDL_0");
    OFFSET (sstc,reservedL[1],"RESERVEDL_1");
    NEWLINE;

    OFFSET (sstc,cmdFifo0.baseAddrL,"CMD0_BASEADDRL");
    OFFSET (sstc,cmdFifo0.baseSize,"CMD0_BASESIZE");
    OFFSET (sstc,cmdFifo0.bump,"CMD0_BUMP");
    OFFSET (sstc,cmdFifo0.readPtrL,"CMD0_READDPTRL");
    OFFSET (sstc,cmdFifo0.readPtrH,"CMD0_READDPTRH");
    OFFSET (sstc,cmdFifo0.aMin,"CMD0_AMIN");
    OFFSET (sstc,cmdFifo0.aMax,"CMD0_AMAX");
    OFFSET (sstc,cmdFifo0.depth,"CMD0_DEPTH");
    OFFSET (sstc,cmdFifo0.holeCount,"CMD0_HOLECOUNT");
    NEWLINE;

    OFFSET (sstc,cmdFifo1.baseAddrL,"CMD1_BASEADDRL");
    OFFSET (sstc,cmdFifo1.baseSize,"CMD1_BASESIZE");
    OFFSET (sstc,cmdFifo1.bump,"CMD1_BUMP");
    OFFSET (sstc,cmdFifo1.readPtrL,"CMD1_READPTRL");
    OFFSET (sstc,cmdFifo1.readPtrH,"CMD1_READPTRH");
    OFFSET (sstc,cmdFifo1.aMin,"CMD1_AMIN");
    OFFSET (sstc,cmdFifo1.aMax,"CMD1_AMAX");
    OFFSET (sstc,cmdFifo1.depth,"CMD1_DEPTH");
    OFFSET (sstc,cmdFifo1.holeCount,"CMD1_HOLECOUNT");
    NEWLINE;

    OFFSET (sstc,cmdFifoThresh,"cmdFifoThresh");
    OFFSET (sstc,reservedO[0],"RESERVEDO_0");
    OFFSET (sstc,reservedO[1],"RESERVEDO_1");
    OFFSET (sstc,reservedO[2],"RESERVEDO_2");
    OFFSET (sstc,reservedO[3],"RESERVEDO_3");
    OFFSET (sstc,reservedO[4],"RESERVEDO_4");
    OFFSET (sstc,reservedO[5],"RESERVEDO_5");
    OFFSET (sstc,reservedO[6],"RESERVEDO_6");
    NEWLINE;

    OFFSET (sstc,reservedP[0],"RESERVEDP_0");
    OFFSET (sstc,reservedP[1],"RESERVEDP_1");
    OFFSET (sstc,reservedP[2],"RESERVEDP_2");
    OFFSET (sstc,reservedP[3],"RESERVEDP_3");
    OFFSET (sstc,reservedP[4],"RESERVEDP_4");
    OFFSET (sstc,reservedP[5],"RESERVEDP_5");
    OFFSET (sstc,reservedP[6],"RESERVEDP_6");
    OFFSET (sstc,reservedP[7],"RESERVEDP_7");
    NEWLINE;

    OFFSET (sstc,reservedQ[0],"RESERVEDQ_0");
    OFFSET (sstc,reservedQ[1],"RESERVEDQ_1");
    OFFSET (sstc,reservedQ[2],"RESERVEDQ_2");
    OFFSET (sstc,reservedQ[3],"RESERVEDQ_3");
    OFFSET (sstc,reservedQ[4],"RESERVEDQ_4");
    OFFSET (sstc,reservedQ[5],"RESERVEDQ_5");
    OFFSET (sstc,reservedQ[6],"RESERVEDQ_6");
    OFFSET (sstc,reservedQ[7],"RESERVEDQ_7");
    NEWLINE;

    OFFSET (sstc,reservedR[0],"RESERVEDR_0");
    OFFSET (sstc,reservedR[1],"RESERVEDR_1");
    OFFSET (sstc,reservedR[2],"RESERVEDR_2");
    OFFSET (sstc,reservedR[3],"RESERVEDR_3");
    OFFSET (sstc,reservedR[4],"RESERVEDR_4");
    OFFSET (sstc,reservedR[5],"RESERVEDR_5");
    OFFSET (sstc,reservedR[6],"RESERVEDR_6");
    OFFSET (sstc,reservedR[7],"RESERVEDR_7");
    NEWLINE;

    OFFSET (sstc,yuvBaseAddr,"YUVBASEADDR");
    OFFSET (sstc,yuvStride,"YUVSTRIDE");
    OFFSET (sstc,reservedS[0],"RESERVEDS_0");
    OFFSET (sstc,reservedS[1],"RESERVEDS_1");
    OFFSET (sstc,reservedS[2],"RESERVEDS_2");
    OFFSET (sstc,reservedS[3],"RESERVEDS_3");
    OFFSET (sstc,reservedS[4],"RESERVEDS_4");
    OFFSET (sstc,reservedS[5],"RESERVEDS_5");
    NEWLINE;

    OFFSET (sstc,crc1,"CRC1");
    OFFSET (sstc,reservedT[0],"RESERVEDT_0");
    OFFSET (sstc,reservedT[1],"RESERVEDT_1");
    OFFSET (sstc,reservedT[2],"RESERVEDT_2");
    OFFSET (sstc,crc2,"CRC2");
    NEWLINE;

    SIZEOF (sstc,"SSTC");

    HEADER ("SSTG");
    OFFSET (sstg,status,"STATUS\t");
    OFFSET (sstg,clip0min,"CLIP0MIN");
    OFFSET (sstg,clip0max,"CLIP0MAX");

    OFFSET (sstg,rop,"ROP\t");
    OFFSET (sstg,commandEx,"COMMANDEX");
    OFFSET (sstg,lineStipple,"LINESTIPPLE");
    OFFSET (sstg,lineStyle,"LINESTYLE");
    OFFSET (sstg,clip1min,"CLIP1MIN");
    OFFSET (sstg,clip1max,"CLIP1MAX");
    OFFSET (sstg,srcSize,"SRCSIZE\t");
    OFFSET (sstg,srcXY,"SRCXY\t");
    OFFSET (sstg,colorBack,"COLORBACK");
    OFFSET (sstg,colorFore,"COLORFORE");
    OFFSET (sstg,dstSize,"DSTSIZE\t");
    OFFSET (sstg,dstXY,"DSTXY\t");
    OFFSET (sstg,command,"COMMAND\t");
    OFFSET (sstg,launch,"LAUNCH\t");
    OFFSET (sstg,colorPattern,"COLORPATTERN");
#if COLORTRANSLUT
    OFFSET (sstg,colorTransLut,"COLORTRANSLUT");
#endif
    SIZEOF (sstg,"SSTG");

    HEADER ("SST");
    OFFSET (sst,status,"STATUS\t");
    OFFSET (sst,intrCtrl,"INTRCTRL");
    OFFSET (sst,vA.x,"VA_X\t");
    OFFSET (sst,vA.y,"VA_Y\t");
    OFFSET (sst,vB.x,"VB_X\t");
    OFFSET (sst,vB.y,"VB_Y\t");
    OFFSET (sst,vC.x,"VC_X\t");
    OFFSET (sst,vC.y,"VC_Y\t");
    NEWLINE;

    OFFSET (sst,r,"R\t");
    OFFSET (sst,g,"G\t");
    OFFSET (sst,b,"B\t");
    OFFSET (sst,z,"Z\t");
    OFFSET (sst,a,"A\t");
    OFFSET (sst,s,"S\t");
    OFFSET (sst,t,"T\t");
    OFFSET (sst,w,"W\t");
    NEWLINE;

    OFFSET (sst,drdx,"DRDX\t");
    OFFSET (sst,dgdx,"DGDX\t");
    OFFSET (sst,dbdx,"DBDX\t");
    OFFSET (sst,dzdx,"DZDX\t");
    OFFSET (sst,dadx,"DADX\t");
    OFFSET (sst,dsdx,"DSDX\t");
    OFFSET (sst,dtdx,"DTDX\t");
    OFFSET (sst,dwdx,"DWDX\t");
    NEWLINE;

    OFFSET (sst,drdy,"DRDY\t");
    OFFSET (sst,dgdy,"DGDY\t");
    OFFSET (sst,dbdy,"DBDY\t");
    OFFSET (sst,dzdy,"DZDY\t");
    OFFSET (sst,dady,"DADY\t");
    OFFSET (sst,dsdy,"DSDY\t");
    OFFSET (sst,dtdy,"DTDY\t");
    OFFSET (sst,dwdy,"DWDY\t");
    NEWLINE;

    OFFSET (sst,triangleCMD,"TRIANGLECMD");
    OFFSET (sst,reservedA,"RESERVEDA");
    OFFSET (sst,FvA.x,"FVA_X\t");
    OFFSET (sst,FvA.y,"FVA_Y\t");
    OFFSET (sst,FvB.x,"FVB_X\t");
    OFFSET (sst,FvB.y,"FVB_Y\t");
    OFFSET (sst,FvC.x,"FVC_X\t");
    OFFSET (sst,FvC.y,"FVC_Y\t");
    NEWLINE;

    OFFSET (sst,Fr,"FR\t");
    OFFSET (sst,Fg,"FG\t");
    OFFSET (sst,Fb,"FB\t");
    OFFSET (sst,Fz,"FZ\t");
    OFFSET (sst,Fa,"FA\t");
    OFFSET (sst,Fs,"FS\t");
    OFFSET (sst,Ft,"FT\t");
    OFFSET (sst,Fw,"FW\t");
    NEWLINE;

    OFFSET (sst,Fdrdx,"FDRDX\t");
    OFFSET (sst,Fdgdx,"FDGDX\t");
    OFFSET (sst,Fdbdx,"FDBDX\t");
    OFFSET (sst,Fdzdx,"FDZDX\t");
    OFFSET (sst,Fdadx,"FDADX\t");
    OFFSET (sst,Fdsdx,"FDSDX\t");
    OFFSET (sst,Fdtdx,"FDTDX\t");
    OFFSET (sst,Fdwdx,"FDWDX\t");
    NEWLINE;

    OFFSET (sst,Fdrdy,"FDRDY\t");
    OFFSET (sst,Fdgdy,"FDGDY\t");
    OFFSET (sst,Fdbdy,"FDBDY\t");
    OFFSET (sst,Fdzdy,"FDZDY\t");
    OFFSET (sst,Fdady,"FDADY\t");
    OFFSET (sst,Fdsdy,"FDSDY\t");
    OFFSET (sst,Fdtdy,"FDTDY\t");
    OFFSET (sst,Fdwdy,"FDWDY\t");
    NEWLINE;

    OFFSET (sst,FtriangleCMD,"FTRIANGLECMD");
    OFFSET (sst,fbzColorPath,"FBZCOLORPATH");
    OFFSET (sst,fogMode,"FOGMODE\t");
    OFFSET (sst,alphaMode,"ALPHAMODE");
    OFFSET (sst,fbzMode,"FBZMODE\t");
    OFFSET (sst,lfbMode,"LFBMODE\t");
    OFFSET (sst,clipLeftRight,"CLIPLEFTRIGHT");
    OFFSET (sst,clipBottomTop,"CLIPBOTTOMTOP");
    NEWLINE;

    OFFSET (sst,nopCMD,"NOPCMD\t");
    OFFSET (sst,fastfillCMD,"FASTFILLCMD");
    OFFSET (sst,swapbufferCMD,"SWAPBUFFERCMD");
    OFFSET (sst,fogColor,"FOGCOLOR");
    OFFSET (sst,zaColor,"ZACOLOR\t");
    OFFSET (sst,chromaKey,"CHROMAKEY");
    OFFSET (sst,chromaRange,"CHROMARANGE");
    OFFSET (sst,userIntrCmd,"USERINTRCMD");
    NEWLINE;

    OFFSET (sst,stipple,"STIPPLE\t");
    OFFSET (sst,c0,"C0\t");
    OFFSET (sst,c1,"C1\t");
    OFFSET (sst,stats.fbiPixelsIn,"FBIPIXELSIN");
    OFFSET (sst,stats.fbiChromaFail,"FBICHROMAFAIL");
    OFFSET (sst,stats.fbiZfuncFail,"FBIZFUNCFAIL");
    OFFSET (sst,stats.fbiAfuncFail,"FBIAFUNCFAIL");
    OFFSET (sst,stats.fbiPixelsOut,"FBIPIXELSOUT");
    NEWLINE;

    OFFSET (sst,fogTable[0],"FOGTABLE");
    NEWLINE;

    OFFSET (sst,renderMode,"RENDERMODE");
    OFFSET (sst,stencilMode,"STENCILMODE");
    OFFSET (sst,stencilOp,"STENCILOP");
    OFFSET (sst,colBufferAddr,"COLBUFFERADDR");
    OFFSET (sst,colBufferStride,"COLBUFFERSTRIDE");
    OFFSET (sst,auxBufferAddr,"AUXBUFFERADDR");
    OFFSET (sst,auxBufferStride,"AUXBUFFERSTRIDE");
    OFFSET (sst,fbiStencilFail,"FBISTENCILFAIL");
    NEWLINE;

    OFFSET (sst,clipLeftRight1,"CLIPLEFTRIGHT1");
    OFFSET (sst,clipBottomTop1,"CLIPBOTTOMTOP1");
    OFFSET (sst,combineMode,"COMBINEMODE");
    NEWLINE;

    OFFSET (sst,swapBufferPend,"SWAPBUFFERPEND");
    OFFSET (sst,leftOverlayBuf,"LEFTOVERLAYBUF");
    OFFSET (sst,rightOverlayBuf,"RIGHTOVERLAYBUF");
    OFFSET (sst,fbiSwapHistory,"FBISWAPHISTORY");
    OFFSET (sst,fbiTrianglesOut,"FBITRIANGLESOUT");
    NEWLINE;

    OFFSET (sst,sSetupMode,"SSETUPMODE");
    OFFSET (sst,sVx,"SVX\t");
    OFFSET (sst,sVy,"SVY\t");
    OFFSET (sst,sARGB,"SARGB\t");
    OFFSET (sst,sRed,"SRED\t");
    OFFSET (sst,sGreen,"SGREEN\t");
    OFFSET (sst,sBlue,"SBLUE\t");
    OFFSET (sst,sAlpha,"SALPHA\t");
    NEWLINE;

    OFFSET (sst,sVz,"SVZ\t");
    OFFSET (sst,sOowfbi,"SOOWFBI\t");
    OFFSET (sst,sOow0,"SOOW0\t");
    OFFSET (sst,sSow0,"SSOW0\t");
    OFFSET (sst,sTow0,"STOW0\t");
    OFFSET (sst,sOow1,"SOOW1\t");
    OFFSET (sst,sSow1,"SSOW1\t");
    OFFSET (sst,sTow1,"STOW1\t");
    OFFSET (sst,sDrawTriCMD,"SDRAWTRICMD");
    OFFSET (sst,sBeginTriCMD,"SBEGINTRICMD");
    NEWLINE;

    OFFSET (sst,textureMode,"TEXTUREMODE");
    OFFSET (sst,tLOD,"TLOD\t");
    OFFSET (sst,tDetail,"TDETAIL\t");
    OFFSET (sst,texBaseAddr,"TEXBASEADDR");
    OFFSET (sst,texBaseAddr1,"TEXBASEADDR1");
    OFFSET (sst,texBaseAddr2,"TEXBASEADDR2");
    OFFSET (sst,texBaseAddr38,"TEXBASEADDR38");
    OFFSET (sst,trexInit0,"TREXINIT0");
    NEWLINE;

    OFFSET (sst,trexInit1,"TREXINIT1");
    OFFSET (sst,nccTable0[0],"NCCTABLE0");
    OFFSET (sst,nccTable1[0],"NCCTABLE1");
    NEWLINE;

    SIZEOF (sst,"SST");
    return 0;
}
