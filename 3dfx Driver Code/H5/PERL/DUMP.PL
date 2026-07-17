#
# This is an example file on calling the 
# subroutine set_video_timing.
#


use set_crtc;
use pci;
use h3regs;

#
# Initialize the device (set up globals)
#
        set_crtc::b_init;



#
# FIx PLL clocks
#

        Pci::outpd( $iobase + 0x40, 0xc02);
        Pci::outpd( $iobase + 0x44, 0xc02);
        Pci::outpd( $iobase + 0x48, 0xc02);


        for($i = 0; $i < 0xb0; $i = $i + 4)
        {

                $status = Pci::inpd( $iobase + $i);
                printf "register %x =  %lx\n", $i , $status;
        }

        for($i = 0xe0; $i < 0xff; $i = $i + 4)
        {

                $status = Pci::inpd( $iobase + $i);
                printf "register %x =  %lx\n", $i , $status;
        }

#
# 2D registers
#
        Pci::writed($membase0 + 0x10003c, 0xdeadbeef);
        Pci::writed($membase0 + 0x200140, 0xbabecafe);

        $status = Pci::readd( $membase0 + 0x10003c);
        printf "register 2D =  %lx\n", $status;


        $status = Pci::readd( $membase0 + 0x200140);
        printf "register 3D =  %lx\n", $status;

        Pci::writed($membase0 + 0x200104, 0xffffffff);
        $status = Pci::readd( $membase0 + 0x200104);
        printf "register 3D (default) =  %lx\n", $status;


#
# NOw init DRAM
#

    Pci::outpd($iobase + $h3regs{"DRAMINIT0"}, 0x169d25);

    # load sgram mode register
    Pci::outpd($iobase + $h3regs{"DRAMDATA"}, 0x37); 
    Pci::outpd($iobase + $h3regs{"DRAMCOMMAND"},0x10d);

    sleep 1;

    # load sgram mask register
    Pci::outpd($iobase + $h3regs{"DRAMDATA"}, 0xffffffff); 
    Pci::outpd($iobase + $h3regs{"DRAMCOMMAND"},0x10e);

    # load sgram color register
    Pci::outpd($iobase + $h3regs{"DRAMDATA"}, 0x0); 
    Pci::outpd($iobase + $h3regs{"DRAMCOMMAND"},0x10f);

    for($i =0 ;$i < 0x3f; $i = $i + 4)
    {
#        Pci::writed($membase1 + $i, $i);
        $data = Pci::readd( $membase1 + $i);
        printf " i = 0x%lx , data = 0x%lx\n", $i, $data;

#        if($i != $data)
#        {
#                printf " oh shit!\n", $status;
#        }
     }

#
# Close up card
#
	Pci::close();
