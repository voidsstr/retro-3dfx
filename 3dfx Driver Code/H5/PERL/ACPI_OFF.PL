#
#

#use lib './packages';

#use h3init;
#use pciwrap;
#use h3regs;

#$PCIVERBOSE = 1;

#
# Initialize the device (set up globals)
#
#        h3init;

use Pci;

        if (!Pci::find(0x121a, 0x3, 0)) {
                exit;
        }

        $iobase = Pci::readcfg(0x18) & 0xFFF0;

open(FD, ">reset_config.pl");

        for($i =0; $i <0xff; $i ++)
        {
           $cfg = Pci::readcfg($i);
           printf FD "Pci::writecfg(0x%2.2x, 0x%8.8x);\n", $i, $cfg;

        }
        printf FD "1;";


        $acpi_cmd = Pci::readcfg(0x64);
        Pci::writecfg(0x64, $acpi_cmd || 0x3);


        Pci::close();

