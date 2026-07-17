#
# Flash the ROM in!
#

use lib '../packages';

use h3init;
use pciwrap;
use h3regs;
use Pci;

#$PCIVERBOSE = 1;

#
# get the filename
#

        $program = (shift @ARGV);
        open FD, "<$program" or die "Can't Open $program !\n";
        binmode FD;


#
# Initialize the device (set up globals)
#
        h3init::h3initMapBoard();
#
# Map in the ROM
#
# This is accomplished by stealing register space from 
# the card that has already been mapped in.  When the ROM 
# is done programming, the registers are restored.
#
# We will use Membase0 + 0x800000 in this case.  There 
# is no associated logic with it, so the device should
# respond and the ROM circuit will control data flow and 
# handshaking.
#

   printf "--------------------------------\n";
   printf "-  Flash! Programming Utility  -\n";
   printf "--------------------------------\n";


   $membase0_cfg = CFGIN32(0x10) & 0xfffffff0;
   $pcicmd_cfg   = CFGIN32(0x04) ;

   $rombase_cfg = $membase0_cfg + 0x800000;
   CFGOUT32(0x30, $rombase_cfg | 0x1);
   CFGOUT32(0x04, $pcicmd_cfg | 0x2);

   $rombase = 0x800000;

   RD8($rombase);


#
# Enable ROM writes, 64 K ROM
#
        $miscinit1 = IN32("MISCINIT1");
        OUT32("MISCINIT1", ($miscinit1 & 0xfdffffff) | 0x0000010 );

#
# Actual Programming
#

        SoftId();
        ChipErase();

#
# Programming Sequence
#

   printf "Programming...\n";

   $address = 0x0000;
   $len = read FD, $buf, 256;

   while($len != 0)
   {

      $i =0;

      #
      # Print Nifty Progress Message...
      #

      Progress(32767, $address);

      while($i < $len)
      {
          $data = ord(substr($buf,$i,1));

          PUT_ROM_BYTE($address, $data);

          #
          # Copy Sub Vendor / subsyste ID to to
          #
          if($address >= 0x7ff8 && $address <= 0x7fff)
          {
             PUT_ROM_BYTE($address | 0x8000, $data);
          }

          $i++;
          $address++;

       }
       $len = read FD, $buf, 16;    
    }

#
# Disable ROM writes, put CFG registers back...
#
   OUT32("MISCINIT1", $miscinit1 & ~0x10);

   CFGOUT32(0x04, $pcicmd_cfg );
   CFGOUT32(0x30, $rombase_cfg );

        Pci::close();


#
# ====================================================================
#

#
# Write ROM Address, data
#

sub WR_ROM
{
        IN32("STATUS");
        IN32("STATUS");
        WR8($rombase + $_[0], $_[1]);
        IN32("STATUS");

}

# ========================
# Burn a byte into the ROM
# ========================

sub PUT_ROM_BYTE
{
   $l_address = $_[0];
   $l_data = $_[1];

   WR_ROM(0x5555, 0xaa);
   WR_ROM(0x2aaa, 0x55);
   WR_ROM(0x5555, 0xa0);
   WR_ROM($l_address, $l_data);

   while(1)
   {
      $temp = RD8($rombase + $l_address);
      last if(($temp & 0x80) == ($l_data & 0x80));
      if(($temp & 0x20) != 1)
      {
          $temp = RD8($rombase + $l_address);
          last if(($temp & 0x80) == ($l_data & 0x80)) ;
          die "Programming Error! $l_address, $l_data != $temp \n";
      }
   }
}

#
# EXIT SOFTWARE ID
#

sub SoftIdExit
{
        WR_ROM(0x5555, 0xaa);
        WR_ROM(0x2aaa, 0x55);
        WR_ROM(0x5555, 0xf0);
}


#
# SOFTWARE PRODUCT ID ENTRY
# Get the part type to see if we can program it!
#

sub SoftId
{

        $flashRoms{"1 32"} = "AMD AM29F010";

        SoftIdExit;

        WR_ROM(0x5555, 0xaa);
        WR_ROM(0x2aaa, 0x55);
        WR_ROM(0x5555, 0x90);
	
        $ManID = RD8($rombase);
        $Device = RD8($rombase + 0x1);

        SoftIdExit;

        if(! exists $flashRoms{"$ManID $Device"})
        {
         printf "Manufacuture: $ManID Device: $Device \n";
         die "Unknown ROM Type!\n";
        }

        printf $flashRoms{"$ManID $Device"};
        printf "\n";


}


#
# =================================================================
#

#
# Chip Erase
#

sub ChipErase
{
        WR_ROM(0x5555, 0xaa);
        WR_ROM(0x2aaa, 0x55);
        WR_ROM(0x5555, 0x80);
        WR_ROM(0x5555, 0xaa);
        WR_ROM(0x2aaa, 0x55);
        WR_ROM(0x5555, 0x10);

        while(1)
        {
           last if(RD8($rombase) &0x80);
        }

        printf "Chip Erase Complete.\n";
}

#
# IBM / ASCII Progress Message
#
sub Progress
{
   $blocks = 20;

   $last = $_[0];
   $done = $_[1];

   $max_x = ($blocks/$last) * $done;

   if($max_x == $last_max)
   {     return;
   }

   $last_max = $max_x;

   printf "Percent Done %3d ", ($done/$last)*100;

   #
   # Print done part
   #
   for($x = 1; $x < $blocks+1; $x++)
   {

      if($x <= $max_x)
      {   printf "\262"; }
      else
      {   printf "\260"; }

   }

   #
   # Backspace
   #
   for($x = 0; $x < $blocks; $x++)
   {
      printf "\b";
   }

   printf "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";

}


