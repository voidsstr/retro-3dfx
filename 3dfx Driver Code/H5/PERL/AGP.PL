#
# Get AGP Information and print it for now...
#
use lib './packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;

#$PCIVERBOSE = 1;

use Pci;
        #
        # Find the Host to Host bridge
        #
        if (!Pci::find(0x8086, 0x7180, 0)) {
                exit;
        }


#
# First config HOST to do what it should!
#

        $agpcmd  = Pci::readcfg(0xa8);
        Pci::writecfg(0xa8, $agpcmd |
                                (1<<9) |  # sideband enable
                                (1<<8) |  # agp enable
                                (1<<7) |  # GART Enable
                                (1)       # 1X data Rate
                                );



#
# Now report the status of the HOST AGP controller
#
        $agpstat = Pci::readcfg(0xa4);
        $agpcmd  = Pci::readcfg(0xa8);
        $agpctrl = Pci::readcfg(0xb0);
        $apsize  = Pci::readcfg(0xb4);
        $attbase = Pci::readcfg(0xb8);

        $apsz = (0x40 - $apsize) *4;

        $rqdepth   = ($agpstat >> 24) & 0xff;
        $sbsupport = (($agpstat >> 9) & 0x1) ? "Yes": "No";
        $drate     = ($agpstat & 0x3);
        if($drate == 0x0) {$datarate = "None"};
        if($drate == 0x1) {$datarate = "1x"};
        if($drate == 0x2) {$datarate = "2x"};
        if($drate == 0x3) {$datarate = "1x and 2x"};

        $agpena = (($agpcmd >> 8) & 0x1) ? "Enabled": "Disabled";
        $sbena = (($agpcmd >> 9) & 0x1) ? "Enabled": "Disabled";
        $drate     = ($agpcmd & 0x3);
        if($drate == 0x0) {$xrate = "None"};
        if($drate == 0x1) {$xrate = "1x"};
        if($drate == 0x2) {$xrate = "2x"};
        if($drate == 0x3) {$xrate = "Illegal"};

        $wrsync = (($agpctrl >> 13) & 0x1) ? "Enabled": "Disabled";
        $gtlb = (($agpctrl >> 7) & 0x1) ? "Enabled": "Disabled";


#
# Print out the devices
# Info
#
        printf " =====================================\n";
        printf " =       Host AGP Configuration      =\n";
        printf " =====================================\n";

        printf "AGP Request Queue Depth : 0x%2.2x\n", $rqdepth;
        printf "AGP Sideband Support    : %s\n", $sbsupport;
        printf "AGP Data Rate(s)        : %s\n", $datarate;
        printf "AGP Enable              : %s\n", $agpena;
        printf "AGP Sideband Enable     : %s\n", $sbena;
        printf "AGP Transfer Rate       : %s\n", $xrate;

        printf "AGP Write - Read Sync   : %s\n", $wrsync;
        printf "AGP GART Enable         : %s\n", $gtlb;
        printf "AGP GART Base           : 0x%8.8x\n", $attbase;

        printf "AGP APSIZE              : %d MB\n", $apsz;

#
# Now configure Banshee!
#

        h3init;

$agpptr = 0;
#
# Now report the status of the Banshee AGP controller
    # check whether AGP is enabled
    $data = CFGIN32("PCISTATUS");
    $agpEnabled = 0;
    if (data & PCISTATUS_CAPLIST) {
	$capptr = CFGIN32("CAPPTR");
	while ($capptr != 0) {
	    $data = CFGIN32($capptr);
            $agpptr = $capptr
		if (($data & $h3def{CAPID_MASK}) >> $h3def{CAPID_SHIFT}  == $h3def{AGP_ID});
	    $capptr = ($data & $h3def{NEXTPTR_MASK}) >> $h3def{NEXTPTR_SHIFT};
	}
    }
#
        $agpstat = Pci::readcfg($agpptr +4);
        $agpcmd  = Pci::readcfg($agpptr+ 8);
        $agpctrl = Pci::readcfg($agpptr + 0xc);

        $rqdepth   = ($agpstat >> 24) & 0xff;
        $sbsupport = (($agpstat >> 9) & 0x1) ? "Yes": "No";
        $drate     = ($agpstat & 0x3);
        if($drate == 0x0) {$datarate = "None"};
        if($drate == 0x1) {$datarate = "1x"};
        if($drate == 0x2) {$datarate = "2x"};
        if($drate == 0x3) {$datarate = "1x and 2x"};

        $agpena = (($agpcmd >> 8) & 0x1) ? "Enabled": "Disabled";
        $sbena = (($agpcmd >> 9) & 0x1) ? "Enabled": "Disabled";
        $drate     = ($agpcmd & 0x3);
        if($drate == 0x0) {$xrate = "None"};
        if($drate == 0x1) {$xrate = "1x"};
        if($drate == 0x2) {$xrate = "2x"};
        if($drate == 0x3) {$xrate = "Illegal"};

        $wrsync = (($agpctrl >> 13) & 0x1) ? "Enabled": "Disabled";
        $gtlb = (($agpctrl >> 7) & 0x1) ? "Enabled": "Disabled";


#
# Print out the devices
# Info
#

        printf " =====================================\n";
        printf " =      Banshee AGP Configuration    =\n";
        printf " =====================================\n";

        if($agpptr == 0)
        {
           printf "AGP Not present! (is this an AGP Board?)\n";
        }
        else
        {
           printf "AGP Capptr : 0x%2.2x\n", $agpptr;
           printf "AGP Request Queue Depth : 0x%2.2x\n", $rqdepth;
           printf "AGP Sideband Support    : %s\n", $sbsupport;
           printf "AGP Data Rate(s)        : %s\n", $datarate;
           printf "AGP Enable              : %s\n", $agpena;
           printf "AGP Sideband Enable     : %s\n", $sbena;
           printf "AGP Transfer Rate       : %s\n", $xrate;
        }

        $Width =  0x1;
        $Hieght = 0x1;

        $Dest = 0x0; # Destination is always memory for these tests

        $HostX = 0x0;
        $HostY = 0x0;
        $HostStride = 0x1f;

        $DestX = 0x0;
        $DestY = 0x0;
        $DestStride =  640;

        $Size = $Hieght * $Width;

        printf("Moving from X: 0x%x , Y: 0x%x, W: 0x%x, H: 0x%x to X: 0x%x, Y: 0x%x  Size: 0x%x\n",
                        $HostX, $HostY, $Width, $Hieght, $DestX, $DestY, $Size );
 
        DataMove ($HostX, $HostY, $HostStride, $Dest, $DestX, $DestY,
                  $DestStride, $Width, $Size);


        sleep 1;

        $data = RD32(0x0, 1);
        printf "data = $data \n";


        Pci::close();




# ****************************************************
# Move Data from AGP memory to Frame Buffer
# ****************************************************

sub DataMove
{
        $HostX       = $_[0];
        $HostY       = $_[1];
        $HostStride  = $_[2];
        $Dest        = $_[3];
        $DestX       = $_[4];
        $DestY       = $_[5];
        $DestStride  = $_[6];
        $Width       = $_[7];
        $Size        = $_[8];

        $HostAddress = 0x555555 + $HostX + ($HostY * $HostStride);
        $DestAddress = $DestX + ($DestY * $DestStride);
	
        WR32("AGPREQSIZE", $Size);                                         
        WR32("HOSTADDRLOW", $HostAddress & 0xffffffff);                                     
        WR32("HOSTADDRHIGH", ($HostStride << 14) | $Width);                                   
        WR32("GRAPHICSADDR", $DestAddress & 0xffffffff);                                     
        WR32("GRAPHICSSTRIDE", $DestStride );                                  
        WR32("MOVECMD", $Dest << 3);                                    
}












