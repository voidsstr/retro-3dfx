use lib './packages';

use h3init;
use pciwrap;
use h3regs;
$ppmfile = "tex_dbg_32x32.ppm";

#
# Initialize the device
#
$mci0 = IN32("MISCINIT0");
$mci0 = $mci0 | 0x41;
OUT32("MISCINIT0", $mci0);
$mci0 = $mci0 & ~0x41;
OUT32("MISCINIT0", $mci0);


h3init;
h3init::h3initVideoTiming( 640, 480, 60);

$mci0 = IN32("MISCINIT0");
printf "miscinit0 = %x\n", $mci0;

foreach $arg (@ARGV) {
    if ($arg =~ /-t(\d+)/) {
	$texsize = $1;
    } elsif ($arg =~ /-camoff/) {
	$camoff = 1;
    } elsif ($arg =~ /-ppm/) {
	$ppm = 1;
    } else {
	push (@tmp, $arg);
    }
}
@ARGV = @tmp;

OUT32("LFBMEMORYCONFIG", 0x1FFF);
OUT32("VIDPROCCFG", 0xe00101);

#        OUT32("VIDSCREENSIZE",(480<<12) | 640);
#        OUT32("VIDDESKTOPSTARTADDR", 0);
OUT32("VIDDESKTOPOVERLAYSTRIDE", 0x05300530);

# Setup 3D pipeline
WR32("CLIPLEFTRIGHT", 640);
WR32("CLIPBOTTOMTOP", 480);
WR32("FBZMODE", 0x0200);
#        WR32("FBZCOLORPATH", 2);
WR32("FBZCOLORPATH", 0x08000001);
WR32("C1", 0x00FF00);
WR32("C0", 0x00FF00);
WR32("COLBUFFERADDR", 0x470);
WR32("COLBUFFERSTRIDE", 0x530);
WR32("LEFTOVERLAYBUF", 0x470);
WR32("SWAPBUFFERCMD", 0);
WR32("TEXTUREMODE", 0x8241a00);
if ($texsize == 8) {
    WR32("TLOD", 0x514);             # 8x8
} elsif ($texsize == 16) {
    WR32("TLOD", 0x410);             # 16x16
} else {
    WR32("TLOD", 0x0);               # 256x256
}
#        WR32("TLOD", 0x30c);            # 32x32
WR32("TEXBASEADDR", 0x3a6cc0);  # 32x32
#        &download("$ppmfile", 0x470 + 400, 0x530);
#        &download("$ppmfile", 0x3d16c0, 0x40);
#        &download("$ppmfile", 0x3d0cc0, 0x40);
#        &generate_ramp(0x3d16c0, 0x10, 0x08, 0x08);
#        &generate_ramp(0x470 + 800, 0x530, 0x08, 0x08);

if ($texsize == 8) {
    ## 8x8
    print "CAMON: using texsize = 8\n";
    &onepixel(0x3d16c0, 0x10, 0x08, 0x08, @ARGV);
    &onepixel(0x470 + 800, 0x530, 0x08, 0x08, @ARGV);
} elsif ($texsize == 16) {
    # 16x16
    &onepixel(0x3d14c0, 0x20, 0x10, 0x10, @ARGV);
    &onepixel(0x470 + 800, 0x530, 0x10, 0x10, @ARGV);
} else {
    # 256x256
    #          dir-yellow r  g    y          b 
    @pixels = (256*256-1, 0, 256, 256*256-4, 256*3-1);
    &onepixel(0x3a6cc0, 0x200, 0x100, 0x100, @pixels);
    &onepixel(0x470 + 800, 0x530, 0x100, 0x100, @pixels);
}

if ($camoff) {
    WR32("TMUGBEINIT", 0x4555);
} else {
    WR32("TMUGBEINIT", 0x555);
}
# Setup 1 triangle  (0,0 - 0.0, 100.0  - 100.0, 100.0)
WR32("SSETUPMODE", 0x25);
$fl_255 = 0x437f0000;
$fl_258 = 0x43810000;
$fl_259 = 0x43818000;
$fl_2 =   0x40000000;
$fl_3 =   0x40400000;

WR32("SVX", 0);
WR32("SVY", 0);
WR32("SOOWFBI", 0x3f800000); #1.0
WR32("SSOW0", $fl_255);
WR32("STOW0", $fl_255);
WR32("SBEGINTRICMD", 0);

WR32("SVX", 0);
WR32("SVY", $fl_255);
WR32("SOOWFBI", 0x3f800000); #1.0
WR32("SSOW0", $fl_255);
WR32("STOW0", $fl_259);
WR32("SDRAWTRICMD", 0);

WR32("SVX", $fl_255);
WR32("SVY", $fl_255);
WR32("SOOWFBI", 0x3f800000); #1.0
WR32("SSOW0", $fl_259);
WR32("STOW0", $fl_259);
WR32("SDRAWTRICMD", 0);

# WR32("SVX", 0x437f0000); #255
# WR32("SVY", 0);
# WR32("SOOWFBI", 0x3f800000); #1.0
# WR32("SSOW0", 0x437f0000);
# WR32("STOW0", 0);
# WR32("SBEGINTRICMD", 0);
# 
# WR32("SVX", 0x437f0000); #255.0
# WR32("SVY", 0x437f0000); #255.0
# WR32("SOOWFBI", 0x3f800000); #1.0
# WR32("SSOW0", 0x437f0000);   #255.0
# WR32("STOW0", 0x437f0000);   #255.0
# WR32("SDRAWTRICMD", 0);
# 
# WR32("SVX", 0x0); #0.0
# WR32("SVY", 0x437f0000); #255.0
# WR32("SOOWFBI", 0x3f800000); #1.0
# WR32("SSOW0", 0x0000);   #0.0
# WR32("STOW0", 0x437f0000);   #255.0
# WR32("SDRAWTRICMD", 0);

sleep 1;
&ppmdump("dump.ppm", 0x470, 0x530, 255, 255) if $ppm; 

sub download {
    my $filename = shift @_;
    my $address = shift @_;
    my $stride = shift @_;
    
    open (FILE, "< $filename");
    chomp($line = <FILE>);
    ($first_tok) = split(' ', $line);
    if ($first_tok ne "P6") {
	print ("Error: File $filename is not a ppm file!\n");
	exit;
    }

    $line = <FILE>;
    ($first_tok, $comment) = split(' ', $line);
    if ($first_tok eq "#") {
	print ("$comment\n");
	$line = <FILE>;
    }

    ($width, $height) = split(' ', $line);
    print ("picture size $width x $height\n");

    chomp($line = <FILE>);
    ($first_tok) = split(' ', $line);
    if ($first_tok ne "255") {
	print ("Error: File $filename is not a ppm file!\n");
	exit;
    }

    $pixels = $width * $height;
    binmode FILE;

    $laddress = $address;
    for ($k=0; $k < $height; $k++) {
	$iaddress = $laddress;
	for ($j = 0; $j < $width; $j++) {
	    $i = $k * $width + $j;
	    $len = read FILE, $pix_buf, 3;
	    if ($len != 3) { print "Error: EOF at pixel $i\n"; }
	    $red = ord(substr($pix_buf, 0, 1)) & 0xFF;
	    $green = ord(substr($pix_buf, 1, 1)) & 0xFF;
	    $blue = ord(substr($pix_buf, 2, 1)) & 0xFF;
	    $pix = (($red >> 3) << 11)  |
		(($green >> 2) << 5) |
		    (($blue >> 3));
	    WR16($iaddress, $pix, 1);
	    $iaddress = $iaddress + 2;
	}
	$laddress = $laddress + $stride;
    }
    close (FILE);
}

sub generate_ramp {
    my ($address, $stride, $width, $height) = @_;

    $laddress = $address;
    for ($y = 0; $y < $height; $y++) {
	$iaddress = $laddress;
	for ($x = 0; $x < $width; $x++) {
	    $red = 0xFF - int(($y / $height) * 0xFF);
	    $green = int(($x / $width) * 0xFF);
	    $blue = 0;
	    $pixel = (($red >> 3) << 11) |
		(($green >> 2) << 5) |
		    (($blue >> 3));
	    WR16($iaddress, $pixel, 1);
	    $iaddress = $iaddress + 2;
	}
	$laddress = $laddress + $stride;
    }
}

sub ppmdump {
    my ($filename, $address, $stride, $width, $height) = @_;
    
    open (FILE, "> $filename");
    print FILE "P6\n";
    print FILE "# Created by Brian\n";
    print FILE "$width $height\n";
    print FILE "255\n";

    binmode FILE;
    $pix_buf = "   ";

    $laddress = $address;
    for ($k=0; $k < $height; $k++) {
	$iaddress = $laddress;
	for ($j = 0; $j < $width; $j++) {
	    $i = $k * $width + $j;
	    $pix = RD16($iaddress, 1);
	    $red = ($pix >> 11) & 0x1f;
	    $red = ($red << 3) | ($red >> 2);
	    $green = ($pix >> 5) & 0x3f;
	    $green = ($green << 2) | ($green >> 4);
	    $blue = ($pix & 0x1f);
	    $blue = ($blue << 3) | ($blue >> 2);

	    substr($pix_buf, 0, 1) = chr($red);
	    substr($pix_buf, 1, 1) = chr($green);
	    substr($pix_buf, 2, 1) = chr($blue);
	    
	    print FILE $pix_buf;
	    $iaddress = $iaddress + 2;
	}
	$laddress = $laddress + $stride;
    }
    close (FILE);
}

sub onepixel {
    my ($address, $stride, $width, $height, @locs) = @_;
    local (@loctbl);
    foreach $loc (@locs) {
	$loctbl[$loc] = 1;
    }

    @coltbl = ("0xff_0x00_0x00",
	       "0x00_0xff_0x00",
	       "0x00_0x00_0xff",
	       "0xff_0xff_0x00",
	       "0x8f_0x8f_0x00", 
	       "0x8f_0x00_0x8f");

    $laddress = $address;
    for ($i = 0; $i < $height; $i++) {
	$iaddress = $laddress;
	for ($j = 0; $j < $width; $j++) {
	    $y = $i * $height + $j;
	    if ($loctbl[$y]) {
		($r,$g,$b) = split(/_/, shift(@coltbl));
		print "col = $r, $g, $b\n";
		$r=hex($r); 
		$g=hex($g); 
		$b=hex($b); 
		WR16($iaddress, &pix16($r, $g, $b), 1);
	    } else {
		WR16($iaddress, &pix16(0x0, 0xFF, 0xFF), 1);
	    }
	    $iaddress = $iaddress + 2;
	}
	$laddress = $laddress + $stride;
    }
}

sub pix16 {
    my ($red, $green, $blue) = @_;
    my $ret;

    $ret = ($red >> 3) << 11;
    $ret = (($green >> 2) << 5)  | $ret;
    $ret = ($blue >> 3) | $ret;

    $ret;

}
