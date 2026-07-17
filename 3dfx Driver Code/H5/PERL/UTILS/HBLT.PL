#!/usr/local/bin/perl

use lib '.\packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;


        if (hex($ARGV[1])) { $base = hex($ARGV[1]); }
        else { print "usage: perl hblt.pl <name>.ppm <address> [<stride>]\n";
               exit;  }

        # Reinit
        h3init;
        h3init::h3initVideoTiming (1024, 768, 75);

	# Don't CLUT bypass
        $vpc = IN32("VIDPROCCFG");
        $vpc &= 0xfffff3ff;
        OUT32("VIDPROCCFG", $vpc);

        # Refill CLUT
        for($i= 0; $i < 0x1ff; $i = $i + 1) {
                OUT32("DACADDR", $i);
                IN32("DACADDR");
                $j = $i & 0xff;
                OUT32("DACDATA", ~(($j<<16) | ($j<<8) | $j));
                IN32("DACDATA");
        }

        # Set stride
        OUT32("VIDDESKTOPOVERLAYSTRIDE", 0x00000a00);
        OUT32("VIDDESKTOPSTARTADDR", $base);

        # Set desktop pixel depth/ tiled/linear
        $vidProcCfg = $h3def{SST_VIDEO_PROCESSOR_EN};
        $vidProcCfg |= $h3def{SST_DESKTOP_EN};
        $vidProcCfg |= $h3def{SST_DESKTOP_PIXEL_RGB24};
        OUT32("VIDPROCCFG",$vidProcCfg);

        printf("Base Addr = %x\n", $base);

# Open the file
	$filename = $ARGV[0];
	print("file: $filename\n");
	open(FILE, "< $filename");

# Make sure 1st line says "P6"
	chomp($line = <FILE>);
        ($first_tok) = split(' ', $line);
        if ($first_tok ne "P6") {
		print("ERROR: File $filename is not a ppm file!\n");
                print "usage: perl hblt.pl <name>.ppm <address> [<stride>]\n";
                exit;
	};

# Check to see if the next line is a comment, if it is print it and get the next
	$line = <FILE>;
        ($first_tok, $comment) = split(' ', $line);
        if ($first_tok eq "#") {
                print("$comment\n");
		$line = <FILE>;
	};

# Read the height & width & display
	($width, $height) = split(' ', $line);
	print("picture size $width x $height\n");

# Make sure the next line says "255"
	chomp($line = <FILE>);
        ($first_tok) = split(' ', $line);
        if ($first_tok ne "255") {
		print("ERROR: File $filename is not a ppm file!\n");
                print "usage: perl hblt.pl <name>.ppm <address> [<stride>]\n";
                exit;
	};

# If stride is not given, calculate from width
        if (hex($ARGV[2])) { $stride = hex($ARGV[2]); }
        else { $stride = $width * 3; }
        OUT32("VIDDESKTOPOVERLAYSTRIDE", $stride);

$PCIVERBOSE = 1;

        # Clip window 1 (0,0) - (2k, 2k)
        WR32("CLIP1MIN", 0x00000000);
        WR32("CLIP1MAX", 0x08000800);

        # set dst base, 24bpp, stride
        WR32("DSTBASEADDR", $base);
        WR32("DSTFORMAT", 0x00040a00);
        WR32("DSTXY", 0x00000000);

	# Src format is always 32bpp, dword packed to make my life easy =o)
        WR32("SRCFORMAT", 0x00c50000);

# Start Banshee setup for an hblt

	# hblt mode, rop = cc (default)
        WR32("COMMAND", 0xcc800003);

        WR32("DSTFORMAT", 0x00040000 | $stride);

# Set dst size = ppm size
        WR32("DSTSIZE", ($height*0x10000) + $width);

$PCIVERBOSE = 0;

# Calculate the number of pixels in the file
	$pixels = $width * $height;

        binmode FILE;
# Iterate over pixels
	for ($i=0; $i<$pixels; $i++) {

	# Read each color channel
		$len = read FILE, $pix_buf, 3;
#		if ($len != 3) { print("ERROR: EOF at pixel $i\n"); }
		$red   = substr($pix_buf,0,1);
		$green = substr($pix_buf,1,1);
		$blue  = substr($pix_buf,2,1);

	# write the pixel as a 32bit dword, alpha = 0
               $pix = (ord($red)<<16) | (ord($green)<<8) | (ord($blue));
                WR32("LAUNCH", $pix);
	};
	
	close(FILE);


$PCIVERBOSE = 0;
