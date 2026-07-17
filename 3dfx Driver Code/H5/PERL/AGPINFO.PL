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
# Now report the status of the HOST AGP controller
#
        $agpstat = Pci::readcfg(0xa4);
        $agpcmd  = Pci::readcfg(0xa8);
        $agpctrl = Pci::readcfg(0xb0);
        $apsize  = Pci::readcfg(0xb4);
        $attbase = Pci::readcfg(0xb8);
        $amtt    = (Pci::readcfg(0xbc) >> 3) & 0x1f;
        $lmtt    = (Pci::readcfg(0xbc) >> 11) & 0x1f;

        $paccfg  = Pci::readcfg(0x50) & 0xffff;

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



h3init::h3initMapBoard();

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

        printf "AGP AMTT                : %x\n", $amtt;
        printf "AGP LPTT                : %x\n", $lptt;

        printf "PAC CFG                 : %x\n", $paccfg;


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

