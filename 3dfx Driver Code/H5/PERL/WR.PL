#
# read a list of registers, divide them into fields, and display the fields
#
use lib './packages';

use h3defs;
use h3regs;
use h3init;
use pciwrap;

die "\nusage: perl $0 regname value\n" if ( @ARGV % 2 != 0 );

h3init::h3initMapBoard;
print "\n";

while ( @ARGV ) {
    $name = shift @ARGV;
    $data = shift @ARGV;
    $data = hex($data) if ( $data =~ /0x/ || $data =~ /0X/ );
    $name =~ tr/a-z/A-Z/;

    if ( exists $h3port{$name} ) {
	OUT32($name,$data);
    } elsif ( exists $h3reg{$name} ) {
	WR32($name,$data);
    } elsif ( exists $h3cfg{$name} ) {
	CFGOUT32($name,$data);
    } else {
	print "unknown register '$name'\n";
	next;
    }
    PARSE($name,$data);
    print "\n";
}

