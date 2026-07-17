#!/usr/local/bin/perl

#
# This routine is used to take the values from a comma seperated file
# (The spreadsheet of VESA modes called vidmodes.csv) and generates 
# a perl array for use by other programs.
#
#


eval "exec /usr/local/bin/perl -S $0 $*"
    if $running_under_some_shell;
			# this emulates #! processing on NIH machines.
			# (remove #! line above if indigestible)

eval '$'.$1.'$2;' while $ARGV[0] =~ /^([A-Za-z_0-9]+=)(.*)/ && shift;
			# process any FOO=bar switches


$FS = ',';
$line = 0;
#
# define Array of PLL frequencies so I don't have to calculate them
# format is {"frequency"} = "m n k"
#

		# $freq_mnk{"15.750"}  = "42 3 3";
		# $freq_mnk{"18.000"}  = "119 10 3";		
		# $freq_mnk{"32.500"}  = "116 11 2";		
		# $freq_mnk{"37.500"}  = "113 9 2"; 		
		# $freq_mnk{"39.375"}  = "31 1 2";			
		# $freq_mnk{"47.250"}  = "64 3 2"; 	

                $freq_mnk{"12.970"}  = "59 219 2";
                $freq_mnk{"12.588"}  = "58 209 2";
                $freq_mnk{"15.750"}  = "13 130 3";
	   	$freq_mnk{"18.000"}  = "15 169 3";		
		$freq_mnk{"32.500"}  = "23 225 2";		
		$freq_mnk{"37.500"}  = "19 218 2"; 		
		$freq_mnk{"39.375"}  = "10 130 2";			
		$freq_mnk{"47.250"}  = "13 196 2"; 		
		$freq_mnk{"20.000"}  = "61 86 0";		# 20.0000
		$freq_mnk{"25.000"}  = "61 180 0";		# 25.0000
		$freq_mnk{"25.047"}  = "42 75 0";		# 25.047
		$freq_mnk{"25.175"}  = "31 114 1";		# 25.1653 		
		$freq_mnk{"25.940"}  = "59 219 1";		# 25.94 		
		$freq_mnk{"28.125"}  = "26 53 0";		# 25.94 		
		$freq_mnk{"30.050"}  = "49 105 0";		# 30.050
		$freq_mnk{"30.060"}  = "8 19 0";		# 30.060
		$freq_mnk{"31.500"}  = "13 130 2"; 		
		$freq_mnk{"35.000"}  = "7 86 2";			
		$freq_mnk{"35.500"}  = "46 117 0";		# 35.500	
		$freq_mnk{"36.000"}  = "15 169 2"; 		# 36.0060
		$freq_mnk{"36.055"}  = "54 139 0";		# 36.06
		$freq_mnk{"37.800"}  = "23 64 0";		# 37.7999
		$freq_mnk{"38.572"}  = "47 130 0";		# 38.572
		$freq_mnk{"40.000"}  = "15 188 2";		# 40.0067
		$freq_mnk{"42.350"}  = "22 69 0";		# 42.352
		$freq_mnk{"43.266"}  = "44 137 0";		# 42.266
		$freq_mnk{"49.500"}  = "21 157 1";		# 49.4911

            $freq_mnk{"50.000"}  = "1 40 2";                # 50.1136
                $freq_mnk{"50.820"}  = "18 69 0";                # 50.8235
		$freq_mnk{"56.250"}  = "12 108 1";		# 65.0045
		$freq_mnk{"60.091"}  = "59 254 0";		# 60.091
		$freq_mnk{"65.000"}  = "23 225 1";
		$freq_mnk{"66.176"}  = "35 169 0";
		$freq_mnk{"75.000"}  = "19 218 1";
                $freq_mnk{"75.6"} = "23 130 0";                 # 75.599998
		$freq_mnk{"77.143"}  = "29 165 0";
		$freq_mnk{"78.750"}  = "10 130 1";
		$freq_mnk{"79.400"}  = "9 59 0";
                $freq_mnk{"81.000"}  = "33 196 0";
		$freq_mnk{"94.500"}  = "13 196 1";
		$freq_mnk{"108.000"} = "20 164 0";		# 108.0372
		$freq_mnk{"111.176"} = "15 130 0";		# 121.4577
		$freq_mnk{"121.500"} = "27 244 0";		# 121.4577
		$freq_mnk{"126.000"} = "3 42 0";		# 121.4577
		$freq_mnk{"133.410"} = "20 203 0";
		$freq_mnk{"135.000"} = "12 130 0";
		$freq_mnk{"144.000"} = "16 179 0";		# 148.5511
		$freq_mnk{"148.500"} = "14 164 0";		# 148.5511
		$freq_mnk{"157.500"} = "10 130 0";
		$freq_mnk{"162.000"} = "17 213 0";		# 162.0215
		$freq_mnk{"175.500"} = "17 231 0";		# 175.5861
		$freq_mnk{"185.294"} = "15 218 0";
		$freq_mnk{"189.000"} = "13 196 0";
		$freq_mnk{"197.450"} = "12 191 0";
		$freq_mnk{"202.500"} = "12 196 0";
		$freq_mnk{"209.780"} = "15 247 0";
		$freq_mnk{"216.000"} = "9 164 0";		# 216.0744
		$freq_mnk{"220.000"} = "3 78 0";		# 220.1420
                $freq_mnk{"234.000"} = "12 227 0";              # 234.2045
                $freq_mnk{"236.940"} = "9 180 0";              # 234.2045
                $freq_mnk{"249.900"} = "9 190 0";              # 249.9173





# get stuff from fields
#printf "                \t\tD4                                                  C2  C4  40.41  4C\n";
#printf "                \t\t00  01  02  03  04  05  06  07  10  11  12  15  16  --  01  -- --  --\n";


    open( PF, ">mode_chart.pl");
    open( CF, ">modetabl.h");

while (<>) {
    ($Fld1,$Fld2,$Fld3,$Fld4,$Fld5,$Fld6,$Fld7,$Fld8,$Fld9,$Fld10,$Fld11,$Fld12,$Fld13,
    $Fld14,$Fld15,$Fld16,$Fld17,$Fld18,$Fld19,$Fld20)

      = split(/[,\n]/, $_, 9999);

    if($Fld1>0)
    {
    $clock = $Fld4;
    $halfmode = $Fld20;   # swan -- add half mode field
    $htotal = $Fld8;      # Horizontal Total
    $hde = $Fld9;         # Horizontal Display Enable End
    $hbstart = $Fld10;    # Hblank Start
    $hbwidth = $Fld11;    # Hblank Width
    $hrstart = $Fld12;    # Hret Start
    $hrwidth = $Fld13;    # Hret Width

    $vtotal = $Fld14;     # Vertical Total
    $vde = $Fld15;        # Vertical Display Enable End
    $vbstart = $Fld16;    # Vertical Start
    $vbwidth = $Fld17;    # Vertical Width
    $vrstart = $Fld18;    # Vertical Start
    $vrwidth = $Fld19;    # Vertical Width

#
# calculate the register values here 
#

# if($clock > 135.0)  then 2x mode
# elsif($halfmode eq 'Y')  then half mode

    $dac_mode = 0;

    if ($clock > 135.0) {
		$dac_mode = 0x1;
		$htotal = $htotal / 2;
		$hde = $hde / 2;
		$hbstart = $hbstart / 2;
		$hbwidth = $hbwidth / 2;
		$hrstart = $hrstart / 2;
		$hrwidth = $hrwidth / 2;
    }
    elsif($halfmode eq 'Y') { 
	# swan -- add half mode
	$clock = $clock / 2;

	$htotal = $htotal / 2;
	$hde = $hde / 2;
	$hbstart = $hbstart / 2;
	$hbwidth = $hbwidth / 2;
	$hrstart = $hrstart / 2;
	$hrwidth = $hrwidth / 2;
    }

    #
    # Calc true timing....
    #
    $hbstart = $hbstart - 1;
    $hrstart = $hrstart;
    $hbend   = $hbstart + $hbwidth;
    $hrend   = $hrstart + $hrwidth;
    $hde     = $hde -1;
    $htotal  = $htotal - 5;

    $vbend   = $vbstart + $vbwidth;
    $vrend   = $vrstart + $vrwidth;
    $vtotal  = $vtotal - 2;
    $vbstart = $vbstart - 1;
    $vbend   = $vbend - 1;
    $vde     = $vde -1;

    $crtc0  = $htotal & 0xff;
    $crtc1  = $hde & 0xff;
    $crtc2  = $hbstart & 0xff;
    $crtc3  = 0x80 | ($hbend & 0x1f);
    $crtc4  = $hrstart & 0xff;  
    $crtc5  = ($hrend & 0x1f) | ((($hbend) & 0x20) << 2);
    $crtc6  = $vtotal & 0xff;

    $crtc7	 = ((($vtotal >> 8) & 0x1) << 0);
    $crtc7	|= ((($vtotal >> 9) & 0x1) << 5);
    $crtc7	|= ((($vde >> 8) & 0x1) << 1);
    $crtc7	|= ((($vde >> 9) & 0x1) << 6);
    $crtc7	|= ((($vrstart >> 8) & 0x1) << 2);
    $crtc7	|= ((($vrstart >> 9) & 0x1) << 7);
    $crtc7      |= ((($vbstart >> 8) & 0x1) << 3);
    $crtc7      |= (0x1 << 4);

    $crtc9  = 0x40 | ((($vbstart >>9) & 0x1) << 5);
    $crtc10 = $vrstart & 0xff;
    $crtc11 = ($vrend   & 0xf) | 0x20; #Disable Interrupts
    $crtc12 = $vde     & 0xff;
    $crtc15 = $vbstart & 0xff; 
    $crtc16 = $vbend   & 0xff;

    $crtc1a  = (($htotal >> 8) & 0x1);
    $crtc1a |= ((($hde >> 8) & 0x1) << 2);
    $crtc1a |= ((($hbstart >> 8 ) & 0x1) << 4);
    $crtc1a |= ((($hbend >> 6) & 0x1) << 5);
    $crtc1a |= ((($hrstart >> 8) & 0x1) << 6);
    $crtc1a |= ((($hrend >> 5) & 0x1)  << 7);

    $crtc1b  = ($vtotal >> 10) & 0x1;
    $crtc1b |= (($vde >> 10) & 0x1) << 2;
    $crtc1b |= (($vbstart >> 10) & 0x1) << 4;
    $crtc1b |= (($vrstart >> 10) & 0x1) << 6;

    $misc_reg = 0x03;

    if($Fld6 eq '-') {
		$misc_reg |= 0x40;
	}

    if($Fld7 eq '-') {
		$misc_reg |= 0x80;
	}


#
# Calc which Clock to Use
#

        $misc_reg = $misc_reg | 0x0c;


	$clk_mode = 0x20;	# screen off
	if($Fld5 == 8)
	{
		$clk_mode |= 0x1;
	}

	$pfq = sprintf("%1.3f",$clock); # swan -- changes $fld4 to $clock
	
    if (! exists $freq_mnk{$pfq}) # swan
    {
        printf ("Need new entries for the clock freq %1.3f!\n", $pfq);
	exit;
    }
	($m, $n, $k) = split (' ', $freq_mnk{$pfq});
#    printf "m, n, k are $m, $n, $k";
	$mnk0 = ($k & 0x3) | (($m & 0x3f) << 2);
	$mnk1 = $n & 0xff;

#
# Print out all registers
#


    printf PF "\$mode_num{\"%2.2d %2.2d %2.2d\"} = ", 
    	$Fld1, $Fld2, $Fld3;

    printf PF "\"0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x ",
    	$crtc0, $crtc1, $crtc2, $crtc3, $crtc4, $crtc5;

    printf PF "0x%2.2x 0x%2.2x 0x%2.2x ", $crtc6, $crtc7, $crtc9; 

    printf PF "0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x ",
      $crtc10, $crtc11, $crtc12, $crtc15, $crtc16, $crtc1a, $crtc1b;

    printf PF "0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\";\n",
		$misc_reg, $clk_mode, $mnk0, $mnk1, $dac_mode;


#
# Do it again for C
#


    printf CF "{%2.2d, %2.2d, %2.2d, ", 
    	$Fld1, $Fld2, $Fld3;

    printf CF "0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, ",
    	$crtc0, $crtc1, $crtc2, $crtc3, $crtc4, $crtc5;

    printf CF "0x%2.2x, 0x%2.2x, 0x%2.2x, ", $crtc6, $crtc7, $crtc9; 

    printf CF "0x%2.2x,\n\t0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, ",
      $crtc10, $crtc11, $crtc12, $crtc15, $crtc16, $crtc1a, $crtc1b;

    printf CF "0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x},\n",
		$misc_reg, $clk_mode, $mnk0, $mnk1, $dac_mode;


   }

}

close(PF);
close(CF);

