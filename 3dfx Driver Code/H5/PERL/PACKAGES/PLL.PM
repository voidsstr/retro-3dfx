#----------------------------------------------------------------------------
#                   Avenger PLL utility routines
#
# based on sst1InitComputeClkParams() in $/devel/sst1/init/initvg/dac.c rev 5
#
#----------------------------------------------------------------------------

#
# Copyright (c) 1998, 3Dfx Interactive, Inc.
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
# the contents of this file may not be disclosed to third parties, copied or
# duplicated in any form, in whole or in part, without the prior written
# permission of 3Dfx Interactive, Inc.
# 
# RESTRICTED RIGHTS LEGEND:
# Use, duplication or disclosure by the Government is subject to restrictions
# as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
# and Computer Software clause at DFARS 252.227-7013, and/or in similar or
# successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
# rights reserved under the Copyright Laws of the United States.
# 
# 
# $Revision: $
# $Date: $
# 
#

package pll;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(calcPllParams calcOEMPllParams);
@EXPORT_OK = qw();

############################################################################
# HACK ALERT
# This is a temporary version of the calcPllParams sub which uses the lab
# "whiteboard" formula of m=k=1. pault 11-16-98
# An new function called calcOEMPllParams is used for H4_OEM. This is our
# original formula that uses m=11 below 141 MHz.
############################################################################

#
# Calculate PLL parameters for given clock frequency
# If $m_wedge_value = -1, let m be any value (no wedge). If
# $m_wedge_value > -1, calcPllParams will force m to be this value. This
# feature was added for Avenger OEM.
#
# ($n,$m,$k) = calcPllParams($freq,$m_wedge_value)
#

sub calcPllParams
{
    @_ == 2 or die "$0: Wrong number of arguments: @_\n";
    my($freq,$m_wedge_value) = @_;

    my($m,$n,$k);

    $m = $k = 1;
    
    $n = ((6 * $freq) / 14.31818) - 2;
    $n = int($n + 0.5); # Round up
    return($n,$m,$k);
}

#
# calcOEMPllParams
#
# For Avenger revisions A3 and beyond, k is wedged to 1, and the lower 4
# bits of M are wedged to 11. Here is the rtl code:
#  gk[1:0]  = dis_tmu2 ? 2'b01 : pll_ctrl1[1:0];
#  gm[5:0]  = dis_tmu2 ? {pll_ctrl1[7:6], 4'hb} : pll_ctrl1[7:2];
#  gn[7:0]  = pll_ctrl1[15:8];
# In this code, I will just use the m_wedge_value as given (with the upper
# two bits of M as 2'b00). I could set these bits to some non-zero value.
# Setting these to some non-zero value gives us a smaller step size in
# setting the frequency, but it doesn't matter. With these bits at 0, the
# step size is 0.55 MHz.

sub calcOEMPllParams
{
    @_ == 2 or die "$0: Wrong number of arguments: @_\n";
    my($freq,$m_wedge_value) = @_;
    my($m,$n,$k,$vcoFreqDivide,$freqMultRatio,$clkErrorMin);
    my($mPlusTwo,$nPlusTwo,$clkError);

    $f0 = 14.31818;

    $vcoFreqDivide = 2;
    $k = 1;
    
    # Divide by 14.31818
    $freqMultRatio = ($freq/$f0) * $vcoFreqDivide;
    
    # Calculate proper N and M parameters which yield the lowest error 
    $clkErrorMin = 9999.0; 
    if ($m_wedge_value < 0) {
	$m = -1;
	for( $mPlusTwo = 2; $mPlusTwo <= 65; $mPlusTwo++ ) {
	    $nPlusTwo = int( $mPlusTwo * $freqMultRatio + 0.5);
	    $clkError = ($nPlusTwo / $mPlusTwo) - $freqMultRatio;
	    if ($clkError < 0.0) {
		$clkError = -1*$clkError;
	    }
	    if(($clkError < $clkErrorMin) && (($nPlusTwo - 2) < 255)) {
		$clkErrorMin = $clkError;
		$m = $mPlusTwo - 2;
		$n = $nPlusTwo - 2;
	    }
	}
	die "unable to compute m,n for the frequency $freq" if ($m == -1);
    }
    else {
	$mPlusTwo = $m_wedge_value + 2;
	$n = -1;
	for ($nPlusTwo = 2; $nPlusTwo <= 257; $nPlusTwo++) {
	    $clkError = ($nPlusTwo / $mPlusTwo) - $freqMultRatio;
	    if ($clkError < 0.0) {
		$clkError = -1*$clkError;
	    }
	    if(($clkError < $clkErrorMin) && (($nPlusTwo - 2) < 255)) {
		$clkErrorMin = $clkError;
		$m = $mPlusTwo - 2;
		$n = $nPlusTwo - 2;
	    }
	}
	die "unable to compute n for the frequency $freq with m wedged to $m_wedge_value and k = $k" if ($n == -1);
    }
    return($n,$m,$k);
}

1;
