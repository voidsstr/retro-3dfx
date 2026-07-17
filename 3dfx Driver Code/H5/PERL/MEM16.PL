use lib './packages';

use h3init;
use h3regs;
use h3defs;
use pciwrap;
use diags;

h3init::h3initMapBoard();
$dramInit0 = IN32("DRAMINIT0");
$dramInit0 &= ~($h3def{SST_SGRAM_NUM_CHIPSETS} | $h3def{SST_SGRAM_TYPE});
$dramInit0 |= $h3def{SST_SGRAM_TYPE_16MBIT};  # 16Mbit parts
$dramInit0 |= $h3def{SST_SGRAM_NUM_CHIPSETS}; # 2 banks

OUT32("DRAMINIT0",$dramInit0);
