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

        $acpi_cmd = Pci::readcfg(0x64) & 0xfffffffc;
        Pci::writecfg(0x64, $acpi_cmd );

require "reset_config.pl";

        Pci::close();

