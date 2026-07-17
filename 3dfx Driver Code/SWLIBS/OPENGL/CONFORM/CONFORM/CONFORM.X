#!/bin/csh -f

echo ""
echo "**********************************************************************"
echo "     OpenGL & GLX Conformance Report"
echo "**********************************************************************"
echo ""
echo ""
echo "     This OpenGL Conformance Certification Report ("\""Report"\"") is "
echo ""
echo ""
echo "     made the _____ day of _____________, 199___ "
echo ""
echo ""
echo "     by _________________________________________________________," 
echo ""
echo ""
echo "     a _____________________________ corporation having a place of" 
echo ""
echo ""
echo "     business at ________________________________________________"
echo ""
echo ""
echo "     ____________________________________________________________."
echo ""
echo ""
echo ""
echo ""
echo "     This Report will be made available to anyone who requests the"
echo "     Report from the Secretary of the Architecture Review Board."
echo "     This Report certifies Licensee ran the Conformance Tests of"
echo "     OpenGL Release __________ on the OpenGL implementation on the"
echo "     configuration written below (include names of CPU, graphics"
echo "     options, add-ons, O/S release, window systems, or other"
echo "     identifying information):"
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo "     Licensee believes that the results of these Conformance Tests"
echo "     can be generalized and that the results would be identical,"
echo "     if run on the configurations listed below (include names of"
echo "     CPU, graphics options, add-ons, O/S release, window systems,"
echo "     or other identifying information):"
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo "     These conformance tests are run from the provided script,"
echo "     conform.x.  This is the required script for X Window" 
echo "     System implementations." 
echo ""
echo "     By checking off the boxes below, the associated statement is"
echo "     certified to be true. The first and last box must be checked."
echo ""
echo "      _"
echo "     [_] The OpenGL implementation on the configuration listed above"
echo "     passes the basic conformance test suite (mustpass, covgl,"
echo "     covglu, and primtest)."
echo ""
echo "     For any implementation on the X Window System, the following"
echo "     three boxes must be checked. To conform to the GLX spec, the"
echo "     implementation must also support the following two visuals as"
echo "     specified in section 3.2.2 of the GLX spec."
echo ""
echo "      _"
echo "     [_] The OpenGL implementation certified above has an RGBA"
echo "     visual, which supports OpenGL, with at least one color"
echo "     buffer, a stencil buffer of at least 1 bit, a depth buffer of"
echo "     at least 12 bits, and an accumulation buffer. Alpha bitplanes"
echo "     are optional in this visual. However, its color buffer size"
echo "     must be as great as that of the deepest TrueColor,"
echo "     DirectColor, PseudoColor, or StaticColor visual supported on"
echo "     framebuffer level zero."
echo ""
echo "      _"
echo "     [_] The OpenGL implementation certified above has "
echo "     an OpenGL visual of color index type with at least one"
echo "     color buffer, a stencil buffer of at least 1 bit, and a depth"
echo "     buffer of at least 12 bits. This visual must have as many"
echo "     color bitplanes as the deepest PseudoColor or Static Color"
echo "     visual supported on framebuffer level zero, and it is"
echo "     itself available on level zero."
echo ""
echo "      _"
echo "     [_] The implementation on the configuration listed above passes the"
echo "     covglx test for all visuals that GLX supports. In addition the"
echo "     results of the conformx tests for these visuals are reported" 
echo "     below."
echo ""
echo ""
echo "  		Conformance Suite"
echo "      _ "
echo "     [_] The conformance suite has been run on the OpenGL"
echo "     implementation as configured above, and these are the results"
echo "     of running the conformance suite."
echo ""
echo ""
date
echo ""

setenv SEED $$
echo "**********************************************************************"
echo "     covglx."
echo "**********************************************************************"
echo ""
covglx/covglx
@ returnVal = $status
if ($returnVal) then
	echo "ERROR: returned exit value of $returnVal"
 	echo "aborting..."
 	exit $returnVal
endif

echo "**********************************************************************"
echo "     covgl."
echo "**********************************************************************"
echo ""
covgl/covgl 
@ returnVal = $status
if ($returnVal) then
	echo "ERROR: returned exit value of $returnVal"
 	echo "aborting..."
 	exit $returnVal
endif
echo "**********************************************************************"
echo "     covglu."
echo "**********************************************************************"
echo ""
covglu/covglu
@ returnVal = $status
if ($returnVal) then
	echo "ERROR: returned exit value of $returnVal"
 	echo "aborting..."
 	exit $returnVal
endif
echo "**********************************************************************"
echo "     primtest."
echo "**********************************************************************"
echo ""
primtest/primtest 
@ returnVal = $status
if ($returnVal) then
	echo "ERROR: returned exit value of $returnVal"
 	echo "aborting..."
 	exit $returnVal
endif
echo "**********************************************************************"
echo "     conform mustpass."
echo "**********************************************************************"
echo ""
@ SEED = $SEED + 1
conform/conform -r $SEED -1 mustpass.c
@ returnVal = $status
if ($returnVal) then
	echo "ERROR: returned exit value of $returnVal"
 	echo "aborting..."
 	exit $returnVal
endif
echo "**********************************************************************"
echo "     conform path 0 through 3"
echo "**********************************************************************"
echo ""
conform/conform -r $SEED -f conform/TESTLIST
@ returnVal = $status
if ($returnVal) then
	echo "ERROR: returned exit value of $returnVal"
 	echo "aborting..."
 	exit $returnVal
endif
echo "**********************************************************************"
echo ""
@ SEED = $SEED + 1
conform/conform -r $SEED -f conform/TESTLIST -p 1
@ returnVal = $status
if ($returnVal) then
	echo "ERROR: returned exit value of $returnVal"
 	echo "aborting..."
 	exit $returnVal
endif
echo "**********************************************************************"
echo ""
@ SEED = $SEED + 1
conform/conform -r $SEED -f conform/TESTLIST -p 2
@ returnVal = $status
if ($returnVal) then
	echo "ERROR: returned exit value of $returnVal"
 	echo "aborting..."
 	exit $returnVal
endif
echo "**********************************************************************"
echo ""
@ SEED = $SEED + 1
conform/conform -r $SEED -f conform/TESTLIST -p 3
@ returnVal = $status
if ($returnVal) then
	echo "ERROR: returned exit value of $returnVal"
 	echo "aborting..."
 	exit $returnVal
endif
echo "**********************************************************************"
echo "     conformx."
echo "**********************************************************************"
echo ""
@ SEED = $SEED + 1
conformx/conformx -r $SEED -f conformx/TESTLIST
@ returnVal = $status
if ($returnVal) then
	echo "ERROR: returned exit value of $returnVal"
 	echo "aborting..."
 	exit $returnVal
endif
echo "**********************************************************************"
echo "     conformance suite completed."
echo "**********************************************************************"
date
echo ""
echo ""
echo "     Comment Field"
echo ""
echo "     The licensee may add a statement below which will be provided"
echo "     along with the rest of the Report to all who request this"
echo "     Report."
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo "         SILICON GRAPHICS                       LICENSEE"
echo ""
echo ""
echo "By: _____________________________   By: _____________________________"
echo ""
echo ""
echo "_________________________________   _________________________________"
echo "NAME (PRINT OR TYPE)                NAME (PRINT OR TYPE)"
echo ""
echo "_________________________________   _________________________________"
echo "TITLE                               TITLE"
echo ""
echo "_________________________________   _________________________________"
echo "DATE                                DATE"
echo ""
echo ""
