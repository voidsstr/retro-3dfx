#
# read a list of registers, divide them into fields, and display the fields
#
use lib './packages';

use h3defs;
use h3regs;
use h3init;
use pciwrap;

die "\nusage: perl $0 <file>\nAfter running diag w/sd0,+120 > <file>" if ( @ARGV == 0 );

h3init::h3initMapBoard;
print "\n";

$file = $ARGV[0];

# Make sure that the fifo didn't wrap
if (!system("fgrep -i wrap $file")) {
	print("ERROR: FIFO wrapped\n");
	exit(1);
};

# Find the base address and final ReadPtr & store them in a file
system("fgrep \"cmd0.BaseAddr\" $file | tail -1 > fifo_replay.data");
system("fgrep \"cmd0.ReadPtrL\" $file | tail -1 >> fifo_replay.data");
system("fgrep DSTBASEADDR $file | tail -1 >> fifo_replay.data");

open (DATAFILE, "< fifo_replay.data");

# Get the cmd fifo base
$line = <DATAFILE>;
($junk1, $junk2, $junk3) = split(' ', $line);
($baseAddr) = split('\(', $junk3);

# Get amount of data in the cmd fifo
$line = <DATAFILE>;
($junk1, $junk2, $junk3) = split(' ', $line);
($size) = split('\(', $junk3);
$size = ($size - ($baseAddr*0x1000))>>2;

# Get dst surface base
$line = <DATAFILE>;
($junk1, $junk2, $junk3) = split(' ', $line);
($surf) = split('\(', $junk3);

# Blue the screen
WR32("CLIP0MIN", 0x0);
WR32("CLIP0MAX", 0xffffffff);
WR32("COLORFORE", 0xff);
WR32("DSTBASEADDR", $surf);
WR32("DSTFORMAT", 0x00050a00);
WR32("DSTXY", 0x0);
WR32("DSTSIZE", 0x01e00280);
WR32("COMMAND", 0xcc000105);


printf("Size = %d, Base = %x, Surface = %x \n", $size, $baseAddr, $surf);

WR32("CMD0_BASEADDRL",  $baseAddr);
WR32("CMD0_BASESIZE", 0x05ff);
WR32("CMD0_READPTRL", $baseAddr<<12);
WR32("CMD0_READPTRH", $baseAddr>>20);

while ($size>0x010000) {
    WR32("CMD0_BUMP", 0x0ffff);
    $size -= 0x0ffff;
}

WR32("CMD0_BUMP", $size);


