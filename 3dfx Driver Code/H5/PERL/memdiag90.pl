use lib './packages';

use h3init;
use h3regs;
use h3defs;
use pciwrap;
use diags;

#
# set mem/grx clock to 90 MHz
#

$ENV{SSTH3_MEMCLOCK}=90;
$ENV{SSTH3_GRXCLOCK}=90;

#
# Initialize the device
#

	h3init;

	h3init::h3initVideoTiming( 640, 480, 60);

	# disable transparent latch, set delay to zerl
	$dramInit1 = IN32("DRAMINIT1");
	$dramInit1 &= ~($h3def{SST_SGRAM_OFLOP_TRANS_LATCH} | $h3def{SST_SGRAM_OFLOP_DEL_ADJ});
	printf "$0: setting dramInit1 = 0x%x\n",$dramInit1;
	OUT32("DRAMINIT1",$dramInit1);

	# configure raw lfb to be linear
        OUT32("LFBMEMORYCONFIG", 0x1FFF);

        # Determine memory size by writing and reading a pattern
	print "\n";
        WR32(0x0, 0x0ead0001, 1);
        WR32(0x400000, 0x0eef0002, 1);
	$rdval = RD32(0x0,1);
	if ( $rdval == 0x0eef0002 ) {
	    $real_size = 4;
	} elsif ( $rdval == 0x0ead0001) {
	    WR32(0x800000, 0x0ace0003, 1);
	    $rdval = RD32(0x0, 1);
	    if ($rdval == 0x0ace0003) {
		$real_size = 8;
	    } elsif ($rdval == 0x0ead0001) {
		$real_size = 16;
	    } else {
		die "memory error: read invalid pattern";
	    }
	} else {
	    die "memory error: read invalid pattern";
	}
	
	print "Pattern-detected memory size is ${real_size} Mbytes\n";
	
	# read/display memory size
	$dramInit0 = IN32("DRAMINIT0");
	
	$nbanks = ( ($dramInit0 & $h3def{SST_SGRAM_NUM_CHIPSETS}) == 0 ) ? 1 : 2;
	if ( ($dramInit0&$h3def{SST_SGRAM_TYPE}) == $h3def{SST_SGRAM_TYPE_8MBIT} ) {
	    $size = 8;
	    $sgramName .= "8";
	} elsif ( ($dramInit0&$h3def{SST_SGRAM_TYPE}) == $h3def{SST_SGRAM_TYPE_16MBIT} ) {
	    $size = 16;
	    $sgramName .= "16";
	} else {
	    die "h3initSgram: invalid sgram type = ", 
	    ($dramInit0&$h3def{SST_SGRAM_TYPE})<<$h3def{SST_SGRAM_TYPE_SHIFT};
	}
	
	$strap_size = $nbanks*$size/2;
	print "Strapped memory configuration is $nbanks banks x $size Mbit $sgramName parts = ",$strap_size," Mbytes\n";
	
	die "memory straps failed" if ($real_size!=$strap_size);
	
	$memsize = ($real_size)<<20;  # convert to bytes
	print "\n";

	# set up video and fill memory
        OUT32("VIDPROCCFG", 0x80081);	
        OUT32("VIDSCREENSIZE",(480<<12) | 640);
        OUT32("VIDDESKTOPSTARTADDR", 0);
        OUT32("VIDDESKTOPOVERLAYSTRIDE", 640*3);

        for ($count = 0; $count < 1000; $count++) {

	     	printf "Read test\n\n";
		$data = (int(rand(0xffff))<<16) | int(rand(0xffff));   		# 32-bit random number
	     	$addr = (int(rand($memsize>>16))<<16) | int(rand(0xffff));	# a random, legal memory address
	     	$addrDummy = ~$addr; $addrDummy &= (($memsize-1)<<20) | SST_MASK(20);  # invert address
	     	WR32($addr,$data,1);
	     	for ($i=0; $i<10000; $i++) {
		 	$dummy = int(RD32($addrDummy,1)); #clear any cached read data
		 	$rd = int(RD32($addr,1));
		 	if ( $rd ^ $data ) {
		 	    printf "read error at 0x%x: got 0x%x, expected 0x%x\n",$addr,$rd,$data;
		   	  exit;
		 	}
	     	}

                print "Solid zeros fill test\n\n";
                if (!diags::solid($MEMBASE1, 0, $memsize)) {
                        exit;
                }

                print "Solid ones fill test\n\n";
                if (!diags::solid($MEMBASE1, 0xFFFFFFFF, $memsize)){
                        exit;
                }

                print "Solid 'A' fill test\n\n";
                if (!diags::solid($MEMBASE1, 0xAAAAAAAA, $memsize)) {
                        exit;
                }

                print "Solid '5' fill test\n\n";
                if (!diags::solid($MEMBASE1, 0x55555555, $memsize)) {
                        exit;
                }

                print "Address test\n\n";
                if (!diags::address($MEMBASE1, $memsize)) {
                        exit;
                }

                print "Random fill test\n\n";
                if (!diags::random($MEMBASE1, $memsize, 1000000)) {
                        exit;
                }
        }
        print "**** Memory test passes ****\n";
