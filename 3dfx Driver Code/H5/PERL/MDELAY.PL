use lib './packages';

use h3init;
use h3regs;
use h3defs;
use pciwrap;
use diags;

#
# Initialize the device
#
$dramInit1 = 0;
	    $dramInit1 |= 2<<$h3def{SST_SGRAM_OFLOP_DEL_ADJ_SHIFT};
            $dramInit1 |= $h3def{SST_SGRAM_CLK_NODELAY};
	    $dramInit1 |= $h3def{SST_DRAM_REFRESH_EN};
	    $dramInit1 |= (0x18 << $h3def{SST_DRAM_REFRESH_VALUE_SHIFT}) & $h3def{SST_DRAM_REFRESH_VALUE};  
PARSE("DRAMINIT1",$dramInit1);
$ENV{SSTH3_DRAMINIT1}=$dramInit1;

	$ENV{SSTH3_GRXCLOCK} = 95.0 if ( ! exists $ENV{SSTH3_GRXCLOCK});
	$ENV{SSTH3_MEMCLOCK} = 110.0 if ( ! exists $ENV{SSTH3_MEMCLOCK});

	h3init::h3initMapBoard();
	 h3init::h3initPlls();
   	h3init::h3initSgram();

	$dramInit0 = IN32("DRAMINIT0");
	$dramInit1 = IN32("DRAMINIT1");
	
	printf "grx clk = %d Mhz\n",$ENV{SSTH3_GRXCLOCK};
	printf "mem clk = %d Mhz\n",$ENV{SSTH3_MEMCLOCK};
    	print "\n";

	printf "dramInit0 = 0x%x\n", $dramInit0;
    	PARSE("DRAMINIT0",$dramInit0);
    	print "\n";

	printf "dramInit1 = 0x%x\n", $dramInit1;
    	PARSE("DRAMINIT1",$dramInit1);
    	print "\n";
