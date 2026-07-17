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
# successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
# rights reserved under the Copyright Laws of the United States.
#
#
# $Revision: $
# $Date: $
#
#

#----------------------------------------------------------------------
#              Avenger SGRAM/SDRAM Timing
#----------------------------------------------------------------------

package sgram;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(calcSgramParams %sgramTiming);
@EXPORT_OK = qw();

use h3defs;

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
# successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
# rights reserved under the Copyright Laws of the United States.
#
# --------------------------------------------------------------------------
#
#

#
# NOTES:
# 1. how to determine iRL (min clocks between read and precharge)
#      if ( iCAS == 2 ) { 
#         iRL = 1;  
#      } else { // iCAS == 3
#         iRL = (precharge is pipelined) ? 1 : 2;
#      {
# 2. Generic SGRAM settings are no more aggressive than the least-aggressive
#    part in each category
#

%sgramTiming =
(
#                   iRRD iRCD  iRP iRAS  iRC iCAS iMTC iRd2Dqm iBWC iWL  iBWL  iRL lBST    Spec Spec     Speed
# aliases                                    iCL  iRSC              iWR  iBPL  ???         Rev  Date
#                                                 iLRC              iRWL iBWR
#                                                                   iRDL
#                                                                   iDPL
#                   ---- ----  --- ----  --- ---- ---- ------- ---- ---- ----  --- ----    ---- -------- -----
 GENERIC_SGRAM_8  => "2    3    3    6   10    3    2     1      2    2    2    1    0", #  generic CL=3
 ETRON_SGRAM_8    => "2    3    3    6    9    3    1     1      2    1    2    1    0", #  n/a    10/97   -10 
 GLINK_SGRAM_8    => "2    3    3    6    9    3    1     1      2    2    2    1    0", #  0.0     1/98   -10
 MOSYS_SGRAM_8    => "2    2    2    3    6    2    2     0      2    1    2    1    0", #  1.1  1/26/98   -10 
 SAMSUNG_SGRAM_8  => "2    2    3    5    8    3    1     1      2    1    2    1    0", #
 SIEMENS_SGRAM_8  => "2    3    3    6    9    3    1     1      2    1    2    1    0", #  n/a     5/97   -10
 TOSHIBA_SGRAM_8  => "2    2    3    6   10    3    1     1      2    1    2    1    0", #        6/1/97   -10
#
# New 16 Mb settings for Avenger to match what was put in memtable.h. The above
# 8 Mb settings were not updated.
#
 GENERIC_SDRAM_16 => "2    3    3    8   11    3    2     1      2    1    2    1    0",
 GENERIC_SGRAM_16 => "2    3    3    8   11    3    2     1      2    1    2    1    0",
 CONSERV_SGRAM_16 => "2    3    3    9   12    3    2     1      2    2    2    2    0",
 ETRON_SGRAM_16   => "2    3    3    6    9    3    1     1      2    1    2    1    0",
 FUJITSU_SDRAM_16 => "2    3    3    6    9    3    2     1      2    1    2    1    0",
 FUJITSU_SGRAM_16 => "2    3    3    6    9    3    2     1      2    1    2    1    0",
 SAMSUNG_SDRAM_16 => "2    3    3    8   11    3    2     1      1    1    1    1    0",
 SAMSUNG_SGRAM_16 => "2    3    3    8   11    3    1     1      1    1    1    1    0",
 SIEMENS_SDRAM_16 => "2    3    3    6    9    3    2     1      1    1    1    1    0",
 PING_SDRAM_16    => "2    2    2    5    7    3    2     1      2    2    2    2    0"
 );

#
# determine SGRAM setting given vendor name and size
#
# usage: ($dramInit0,$sgramMode,$sgramMask,$sgramColor) = calcSgramParams($vendorName,$size)
#
sub calcSgramParams {
    my($vendorName,$sdram,$size) = @_;
    my ($partName);
    my ($iRRD,$iRCD,$iRP,$iRAS,$iRC,$iCAS,$iMTC,$iRd2Dqm,$iBWC,$iWL,$iBWL,$iRL,$lBST);
    my ($dramInit0,$sgramMode,$sgramMask,$sgramColor);

    $vendorName =~ tr/a-z/A-Z/;      # upcase
    
    $size = 16 if ( $sdram );
    die "calcSgramParams: invalid size = $size" if ( $sdram == 0 && $size != 8 && $size != 16 );
    
    $partName = $vendorName . ( $sdram ? "_SDRAM_" : "_SGRAM_" ) . "$size";

    # set ram timing parameters

    die "calcSgramParams: unknown ram name = $partName\n" if ( ! exists $sgramTiming{$partName} );

    ($iRRD,$iRCD,$iRP,$iRAS,$iRC,$iCAS,$iMTC,$iRd2Dqm,$iBWC,$iWL,$iBWL,$iRL,$lBST) =
	split ('\s\s*',$sgramTiming{$partName});
    
    $dramInit0 = 0;
    $dramInit0 &= $h3def{SST_SGRAM_NUM_CHIPSETS} | $h3def{SST_SGRAM_TYPE};  # don't overwrite strapping bits
    $dramInit0 |= (($iRRD-1)<<$h3def{SST_SGRAM_RRD_SHIFT}) & $h3def{SST_SGRAM_RRD};
    $dramInit0 |= (($iRCD-1)<<$h3def{SST_SGRAM_RCD_SHIFT}) & $h3def{SST_SGRAM_RCD};
    $dramInit0 |= (($iRP-1)<<$h3def{SST_SGRAM_RP_SHIFT}) & $h3def{SST_SGRAM_RP};
    $dramInit0 |= (($iRAS-1)<<$h3def{SST_SGRAM_RAS_SHIFT}) & $h3def{SST_SGRAM_RAS};
    $dramInit0 |= (($iRC-1)<<$h3def{SST_SGRAM_RC_SHIFT}) & $h3def{SST_SGRAM_RC};
    $dramInit0 |= (($iCAS-1)<<$h3def{SST_SGRAM_CAS_SHIFT}) & $h3def{SST_SGRAM_CAS};
    $dramInit0 |= ($iMTC==1) ? 0: $h3def{SST_SGRAM_MRS};
    $dramInit0 |= ($iRd2Dqm) ? $h3def{SST_SGRAM_RD2DQM} : 0;
    $dramInit0 |= ($iBWC==1) ? 0 : $h3def{SST_SGRAM_BWC};
    $dramInit0 |= ($iWL==1) ? 0 : $h3def{SST_SGRAM_WL};
    $dramInit0 |= (($iBWL-1)<<$h3def{SST_SGRAM_BWL_SHIFT}) & $h3def{SST_SGRAM_BWL};
    $dramInit0 |= ($iRL==1) ? 0 : $h3def{SST_SGRAM_RL};
    $dramInit0 |= ($lBST) ? $h3def{SST_SGRAM_USE_BST} : 0;

   #$dramInit0 |= $h3def{SST_SGRAM_NO_DEAD};
   #$dramInit0 |= $h3def{SST_SGRAM_EN_WPB};

    # load sgram mode register
    $sgramMode = ( exists $ENV{SGRAM_MODE} ) ? $ENV{SGRAM_MODE} : (($iCAS<<4) | 0x7);

    # load sgram mask register
    $sgramMask = exists $ENV{SGRAM_MASK} ? $ENV{SGRAM_MASK} : 0xFFFFFFFF;

    # load sgram color register
    $sgramColor = exists $ENV{SGRAM_COLOR} ? $ENV{SGRAM_COLOR} : 0;

    return ($dramInit0,$sgramMode,$sgramMask,$sgramColor);
}

1;

