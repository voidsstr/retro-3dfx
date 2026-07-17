/*
    VGACore.c - This file contains VGA core tests ported from Elpin Systems VGA
    		 Test Suite.

    Date      Name    Change
    ----      ----    ---------------------------------------------------------
    6/16	bf     Initial release.

*/

#include <stdio.h>
#include "3dfx.h"
#include "h3regs.h"
#include "h3defs.h"
#include "pcibrd.h"
#include "banshee.h"
#include "vgatst\ediag_ex.h"
#include "vgacore.h"


/*
   bansheeTestVGACore:

   This routine tests the VGA core.

*/
FxBool bansheeTestVGACore(LPCARDINFO card) {
int dummy,vm;
int vgacorepass = 0;	
int vgacorefailures = 0;

   printf("Testing the VGA Core.\n");
   
   vm = GetMode();
   vgacorepass = 0;
   vgacorefailures = 0;

   dummy = UndocLightpenTest(card);
   if(dummy == 0){
	vgacorepass++;
   }else{
	vgacorefailures++;
   }

// printf("lightpentest pass\n");
   dummy = UndocLatchTest();
   if (dummy == ERROR_NONE){
      vgacorepass++;
   }else{
      vgacorefailures++;
   }

// printf("undoclatch test pass\n");
    
   dummy = UndocATCToggleTest();
   if (dummy == ERROR_NONE){
      vgacorepass++;
   }else{
      vgacorefailures++;
   }

// printf("undocATCToggle test pass\n");
	
	dummy =	UndocATCIndexTest();
   	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}

//  printf("undocATCIndex test pass\n");

 	dummy = IOTest();
     if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
// printf("IOTest   test pass\n");	 
	
 	dummy = CRTCAddressTest();
  	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}

//  printf(" CRTCAddressTest  test pass\n");
	
  	dummy = IORWTest();
  	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
// 		printf(" IORWTest  test pass\n");

	dummy =	ShiftRegisterModeTest();
  	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" ShiftRegisterModeTest  test pass\n");	 
  
 	dummy = RandomAccessTest();
 	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}

//		printf("RandomAccessTest test pass\n");

    dummy = ColorPlaneEnableTest();
  	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" ColorPlaneEnableTest  test pass\n");
    dummy =	PixelWidthTest();
  	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" PixelWidthTest  test pass\n");

     dummy = CGAHerculesTest();
  	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" CGAHerculesTest  test pass\n");
     dummy = ByteModeTest();
  	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("  ByteModeTest test pass\n");
  
  	 dummy = HiResMono2Test();
 		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" HiResMono2Test  test pass\n");
	 dummy = HiResMono1Test();
		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" HiResMono1Test  test pass\n");
 	dummy =	PaletteAddressSourceTest();
  	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("PaletteAddressSourceTest  test pass\n");
   
   	dummy = HiResColor1Test();
	 	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}

// 			printf("HiResColor1Test test pass\n");
        
    dummy =	LargeCharTest();
	   	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" LargeCharTest  test pass\n");
    dummy =	LineCharTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" LineCharTest  test pass\n");
    
        
    dummy = GraphicsModeBlinkTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" GraphicsModeBlinkTest  test pass\n");
     dummy =	V67Test();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("V67Test   test pass\n");
   	 dummy =	V54Test();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("V54Test   test pass\n");


   
   	 dummy = SyncPolarityTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" SyncPolarityTest  test pass\n");
     dummy =	CursorLocationTest();
	  	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" CursorLocationTest  test pass\n");
   	 dummy =	CursorTypeTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
   
//		printf("CursorTypeTest   test pass\n");
   	 dummy =	LimitedSetupTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("LimitedSetupTest   test pass\n");
	 dummy =	Text64KTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" Text64KTest  test pass\n");

	 dummy =	OverscanTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("OverscanTest   test pass\n");
     dummy =	VideoStatusTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" VideoStatusTest  test pass\n");
   	 dummy = CRTCWriteProtectTest();
   		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("CRTCWriteProtectTest   test pass\n");


   	 dummy =	Ext512CharSetTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" Ext512CharSetTest  test pass\n");
   	 dummy =	Std256CharSetTest();
 		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" Std256CharSetTest  test pass\n");
	 dummy = ModeXTest();     
	   	if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}

// 			printf("ModeXTest test pass\n");
   
	 dummy =	CursorDisableTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("CursorDisableTest   test pass\n");
	 dummy =	CountBy4Test();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" CountBy4Test  test pass\n");
   	 dummy =	Panning256Test();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" Panning256Test  test pass\n");
   
   	 dummy =	CursorSkewTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" CursorSkewTest  test pass\n");

	 dummy =	SkewTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("SkewTest   test pass\n");
   
     dummy =	Vload2Vload4Test();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("Vload2Vload4Test   test pass\n");

	 dummy =	DoubleScanTest();
   		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("DoubleScanTest   test pass\n");
	 dummy = SwitchReadbackTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf(" SwitchReadbackTest  test pass\n");
	 dummy =	SyncPulseTimingTest();
  		if (dummy == ERROR_NONE){
		vgacorepass++;
	  	}else{
		vgacorefailures++;
	 	}
	
//		printf("SyncPulseTimingTest   test pass\n");
  
        dummy =	BlinkVsIntensityTest();
        if (dummy == ERROR_NONE){
	   vgacorepass++;
	}
        else{
           vgacorefailures++;
        }
	
//	printf(" BlinkVsIntensityTest  test pass\n");

     dummy = TextModeSkewTest();
     if (dummy == ERROR_NONE){
        vgacorepass++;
     }
     else {
        vgacorefailures++;
     }
	
//   printf("TextModeSkewTest test pass\n");
       
     SetMode(vm);
     if (vgacorefailures == 0) {
        printf("VGA Core test passed.\n");
        return (FXTRUE);
     }
     else {
        printf("VGA Core test failed.\n");
        return (FXFALSE);
     }
}
