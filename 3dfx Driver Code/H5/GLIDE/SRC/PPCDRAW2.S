	
	MACRO
	MakeFunction &fnName
		EXPORT &fnName[DS]
 		EXPORT .&fnName[PR]
		
		TC &fnName[TC], &fnName[DS]
			
		CSECT &fnName[DS]
			DC.L .&fnName[PR]
 			DC.L TOC[tc0]
		
		CSECT .&fnName[PR]
		FUNCTION .&fnName[PR]	
		
	ENDM

; ======================================================================
; external global variables
; ======================================================================
	
	import ._trisetup_nogradients
	 
  
  IF 1 
  
; About 3 clock cycles of stalls in the code below, waiting for loads. (But
; about 12 clocks that could be used for something else)

	MakeFunction	grDrawTriangle

  b        ._trisetup_nogradients
 
	ENDIF