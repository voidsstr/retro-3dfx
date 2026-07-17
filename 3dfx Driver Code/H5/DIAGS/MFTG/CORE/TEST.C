
/**************** Detect Memory Size *******************/

FxU32 GetMemSize( );
  typedef struct tagVBEINFOBLOCK		//	VESA BIOS information
  { 
	BYTE	VESASignature[4];
	WORD	VESAVersion;
	LPBYTE	OEMStringPtr;
	BYTE	Capabilities[4];
	LPWORD	VideoModePtr;
	WORD	TotalMemory;
	WORD	VBE_SoftwareRev;
	LPBYTE	VBE_VendorNamePtr;
	LPBYTE	VBE_ProductNamePtr;
	LPBYTE	VBE_ProductRevPtr;
	BYTE	reserved[222];				// Fill out to 256 bytes
	BYTE	VBE_OEM_DATA[256];
  } VBEINFOBLOCK;

  typedef	VBEINFOBLOCK	FAR *LPVBEINFOBLOCK;

   union	REGS regs;
   struct	SREGS sregs;
   CRS		crs;
   LPBYTE	lpvbeibTmp;
   VBEINFOBLOCK	vbeInfoVoodoo		// VBE Information about the Voodoo card

    //
    // Allocate DOS Memory Block
    //
    crs.Regs16.Client_AX = 0x0100;         
    crs.Regs16.Client_BX = (sizeof (VBEINFOBLOCK) / 16) + 1;

    regs.w.ax = 0x0100;
    regs.w.bx =  (sizeof (VBEINFOBLOCK) / 16) + 1;
    int386x(0x31, &regs, &regs);
    if (regs.x.cflag)  {
        printf("DPMI Physical Mapping Failure!\n\r$");
        return (0);
    } else {
        wSeg = regs.w.ax;
	wSel = regs.w.dx;
    }

    printf(" voodoo address=  %x	\n", wSeg);

    // Return VBE 2.0 (and above) information
    _fmemcpy (MK_FP (wSel, 0), "VBE2", 4);

    //
    // Do a VBE get info
    // AX = 4F00h, ES:DI = Info block ptr, INT 10h
    //
    crs.Regs16.Client_AX = 0x4F00;
    crs.Regs16.Client_ES = wSeg;
    crs.Regs16.Client_DI = 0x0;
    DPMIRealModeInt(0x10, &crs);
    if (crs.Regs16.Client_AX != 0x004F)
	size = 0;
    else {
	// Copy the info out of the Real mode segment
	_fmemcpy (vbeInfoVoodoo, MK_FP (wSel, 0), sizeof (VBEINFOBLOCK));
	// Convert the segment:offset pointers to selector:offset pointers.
	// Since the original offset from the DOS block was a base of 0, all
	// we need to do is to add the address of the structure to the returned
	// WORD offset.

	lpvbeibTmp = (LPBYTE) vbeInfoVoodoo;
	if (HIWORD (vbeInfoVoodoo->OEMStringPtr) == wSeg)
		vbeInfoVoodoo->OEMStringPtr = (LPBYTE) (lpvbeibTmp + (FP_OFF (vbeInfoVoodoo->OEMStringPtr) & 0xFFFF));
	else
		vbeInfoVoodoo->OEMStringPtr = (LPBYTE) szVBE20Error;

	if (HIWORD (vbeInfoVoodoo->VideoModePtr) == wSeg)
		vbeInfoVoodoo->VideoModePtr = (LPWORD) (lpvbeibTmp + (FP_OFF (vbeInfoVoodoo->VideoModePtr) & 0xFFFF));
	else
		vbeInfoVoodoo->VideoModePtr = &wFFFF;

	if (HIWORD (vbeInfoVoodoo->VBE_VendorNamePtr) == wSeg)
		vbeInfoVoodoo->VBE_VendorNamePtr = (LPBYTE) (lpvbeibTmp + (FP_OFF (vbeInfoVoodoo->VBE_VendorNamePtr) & 0xFFFF));
	else
		vbeInfoVoodoo->VBE_VendorNamePtr = (LPBYTE) szVBE20Error;

	if (HIWORD (vbeInfoVoodoo->VBE_ProductNamePtr) == wSeg)
		vbeInfoVoodoo->VBE_ProductNamePtr = (LPBYTE) (lpvbeibTmp + (FP_OFF (vbeInfoVoodoo->VBE_ProductNamePtr) & 0xFFFF));
	else
		vbeInfoVoodoo->VBE_ProductNamePtr = (LPBYTE) szVBE20Error;

	if (HIWORD (vbeInfoVoodoo->VBE_ProductRevPtr) == wSeg)
		vbeInfoVoodoo->VBE_ProductRevPtr = (LPBYTE) (lpvbeibTmp + (FP_OFF (vbeInfoVoodoo->VBE_ProductRevPtr) & 0xFFFF));
	else
		vbeInfoVoodoo->VBE_ProductRevPtr = (LPBYTE) szVBE20Error;

	size = vbeInfoVoodoo.TotalMemory * 64 * 1024;
  }

  // Free allocated DOS block
  regs.w.ax = 0x0101;
  regs.w.dx = wSel;
  int386x(0x31, &regs, &regs);

  return(size);
}
