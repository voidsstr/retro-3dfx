#
# read a list of registers, divide them into fields, and display the fields
#
use lib './packages';

use h3defs;
use h3regs;

die "\nusage: perl $0 regname1 value\n" if ( @ARGV == 0 );

print "\n";

while ( @ARGV ) {
    $name = shift @ARGV;
    $data = shift @ARGV;
    $data = hex($data) if ( $data =~ /0x/ || $data =~ /0X/ );
    $name =~ tr/a-z/A-Z/;
    
    if ( ! exists $h3port{$name} && ! exists $h3reg{$name} && ! exists $h3cfg{$name} ) {
	print "unknown register '$name'\n";
	next;
    }
    
    PARSE($name,$data);
    print "\n";
}
