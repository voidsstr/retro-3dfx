#
# read a list of registers, divide them into fields, and display the fields
#
use lib './packages';

use h3defs;
use h3regs;
use h3init;
use pciwrap;

die "\nusage: perl $0 [regname1 regname2 ...]\n" if ( @ARGV == 0 );

h3init::h3initMapBoard;
print "\n";

@CMD0 = ( 
	 CMD0_BASEADDRL,  
	 CMD0_BASESIZE, 
	 CMD0_BUMP,
	 CMD0_READPTRL,
	 CMD0_READPTRH,
	 CMD0_AMIN,
	 CMD0_AMAX,
	 CMD0_DEPTH,
	 CMD0_HOLECOUNT
	 );

@CMD1 = ( 
	 CMD1_BASEADDRL,  
	 CMD1_BASESIZE, 
	 CMD1_BUMP,
	 CMD1_READPTRL,
	 CMD1_READPTRH,
	 CMD1_AMIN,
	 CMD1_AMAX,
	 CMD1_DEPTH,
	 CMD1_HOLECOUNT
	 );

while ( @ARGV ) {
    $_ = shift @ARGV;
    $_ =~ tr/a-z/A-Z/;

    if ( /^CMD0$/ ) { foreach $n ( @CMD0 ) { doit($n); } }
    elsif ( /^CMD1$/ ) { foreach $n ( @CMD1 ) { doit($n); } }
    else { doit($_); }

}

sub doit {
    my ($name) = @_;

    if ( exists $h3port{$name} ) {
	$data = IN32($name);
    } elsif ( exists $h3reg{$name} ) {
	$data = RD32($name);
    } elsif ( exists $h3cfg{$name} ) {
	$data = CFGIN32($name);
    } else {
	print "unknown register '$name'\n";
	next;
    }
    PARSE($name,$data);
    print "\n";
}
