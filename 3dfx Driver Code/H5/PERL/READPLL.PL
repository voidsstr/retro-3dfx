use lib './packages';

use h3init;
use h3regs;
use h3defs;
use pciwrap;
use diags;

#
# Initialize the device
#

 h3init::h3initMapBoard();

$pllCtrl0 = IN32("PLLCTRL0");
($n,$m,$k) = extractPllParams($pllCtrl0);
$vidclk = 14.31818 * ($n + 2) / ($m + 2) / 2**$k;
printf "vidclk = %6.2f MHz (pllCtrl0=0x%x; m=%d,n=%d,k=%d)\n",$vidclk,$pllCtrl0,$m,$n,$k;

$pllCtrl1 = IN32("PLLCTRL1");
($n,$m,$k) = extractPllParams($pllCtrl1);
$grxclk = 14.31818 * ($n + 2) / ($m + 2) / 2**$k;
printf "grxclk = %6.2f MHz (pllCtrl1=0x%x; m=%d,n=%d,k=%d)\n",$grxclk,$pllCtrl1,$m,$n,$k;

$pllCtrl2 = IN32("PLLCTRL2");
($n,$m,$k) = extractPllParams($pllCtrl2);
$agpclk = 14.31818 * ($n + 2) / ($m + 2) / 2**$k;
printf "agpclk = %6.2f MHz (pllCtrl2=0x%x; m=%d,n=%d,k=%d)\n",$agpclk,$pllCtrl2,$m,$n,$k;

sub extractPllParams {

    my($pllCtrl) = @_;
    my($n,$m,$k);

    $k = ($pllCtrl & $h3def{SST_PLL_K}) >> $h3def{SST_PLL_K_SHIFT};
    $m = ($pllCtrl & $h3def{SST_PLL_M}) >> $h3def{SST_PLL_M_SHIFT};
    $n = ($pllCtrl & $h3def{SST_PLL_N}) >> $h3def{SST_PLL_N_SHIFT};

    return ($n,$m,$k);
}

