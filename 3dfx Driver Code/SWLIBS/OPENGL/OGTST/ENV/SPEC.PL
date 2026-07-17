#!/usr/bin/perl5
#
# Generate group and test data structures from an ogtst spec file.
#
# Reads the description of valid architectures from a .h file where
# every define of the form '#define ARCH_XXX ....' is interpreted to
# define XXX (case is important) as a legal architecture.
#
# The spec file is a list of blank line separated test specifications
# of the form:
# testName()
#     {ARCHNAM  nOfPasses tolerance SkipOptions}+ # multiple arch spec lines
#     visuals   VisualPropertyIndications
#     type      TestType
#     group	GroupName
#
# Where:
#   - test name must be in the form of a valid C identifier name.
#   - TestType
#      one of the types listed in ogtst.h
#   - GroupName
#      group the test belongs too.
#   - VisualRestrictions
#      nothing or a list of blank separated visual indication properties
#      symbolic constants.
#   All other lines are architecture specific specifications:
#   - Architecture name is one of the architectures defined in
#     in the file provided by the -archfile option.
#   - nOfPasses to run (ignored by chksum tests)
#   - tolerance can be queried by the test.
#   - SkipOptions
#      either 0 or a list of blank separated architecture specific skip
#      option symbolic constants.
#
#   The legal symbolic constants that can be used for VisualRestiction, TestType
#   and skipOption fields are listed in ogtst.h.  But here the 'OG_' prefix is
#   dropped (i.e., if ogtst.h says OG_GLBUG than in this file you'd say GLBUG).
#   
# Anything after a '#' (in the spec file) is a comment which is discarded.
#
#
# $Id: spec.pl,v 1.1 1997/02/05 08:03:24 pho Exp $
#

use Getopt::Long;
&GetOptions("archfile=s");
$opt_archfile or die "Missing architectures .h file option";

open(ARCHFILE, $opt_archfile) ||
    die "Can't open architectures .h file `$archfile' - $!";
while (<ARCHFILE>) {
    /^\#define/ or next;
    ($dummy, $arch) = split;
    $arch =~ s/^ARCH_(.+)$/$1/;
    $arch ne "" or next;
    push(@archNames, $arch);
    $arches{$arch}++;
    $archesSpec{$arch} = [ undef # passes
			  undef # tolerance
			  undef #flags
			  ];
}
#
# These need to correspond to what's defined in ogtst.h or things won't
# compile.
#
@testTypes = qw(NORMAL IMAGED);
&listToLegalHash("testTypes");
@skipConditions = qw(TSTBUG GLBUG CONTEXTBUG CRASHBUG DONOTRUN);
&listToLegalHash("skipConditions");
@visualTypes = qw(RGB RGBA INDEX SINGLEBUF DOUBLEBUF
		  STEREO AUXBUF DEPTH STENCIL ACCUM
		  MULTISAMPLE OVERLAY RGB_ORL RGB_ORL_A
		  LUMINANCE LUMINANCE_A AUXBUF_TOO LUMINANCE_OK
		  RGB_OVERLAY_OK);
&listToLegalHash("visualTypes");

open(SPECFILE, $ARGV[0]) ||
    die "Error: can't open spec file `$ARGV[0]' - $!";
mainLoop: while (<SPECFILE>) {
    next mainLoop if /^\s*(\#.*)?$/;
    if (/^\s*(\w+)\(\)/) {
	&getTest($1);
	next mainLoop;
    }
    &death("Bad syntax");
}

print "/*\n * THIS FILE IS GENERATED, DO NOT EDIT!\n */\n\n";
print "#include \"ogtst.h\"\n#include \"../env/env.h\"\n\n"; 
for (@tests) {
    print "extern void $_->{name}_test(int pass);\
extern void $_->{name}_clean(void);\n";
}
while (($groupName, $groupSt) = (each %groups)) {
    $groupName =~ s/__Was__Minus__/__/;
    print "short ${groupName}_tests[] = {\n   ";
    $i = 0;
    for (@{$groupSt->{tests}}) {
	print " $_->{index},";
	((++$i) % 10) == 0 and $i != @{$groupSt->{tests}} and print "\n   ";
    }
    print "\n};\n";
}
print "Test tests[];\n";
print "Group groups[] = {\n";
$groupIndex = 0;
for $groupName (sort keys(%groups)) {
    $groupSt = $groups{$groupName};
    $groupName =~ s/__Was__Minus__/-/;
    ($nameNoDash = $groupName) =~ s/-/__/;
    print "    {\"$groupName\", $groupSt->{nTests}, ${nameNoDash}_tests,},\n";
    $groupSt->{index} = $groupIndex++;
};
print "};\n";
print "Test tests[] = {\n";
for $test (@tests) {
    print "    {\n        \"$test->{name}\",\n",
      "        {\n";
    for $arch (@archNames) {
	print "            {$test->{specs}{$arch}[0],",
	  " $test->{specs}{$arch}[1],",
	  " $test->{specs}{$arch}[2]}, /* $arch */\n";
    }
    print "        },\n",
      "        $test->{restrictions}, /* Restrictions */\n",
      "        $test->{type}, /* type */\n",
      "        &groups\[$groups{$test->{group}}->{index}\], /* Group */\n",
      "        $test->{name}_test, $test->{name}_clean,\n",
      "        0xdeadbeef, 0xdeadbeef, /* Exepcted & Returned chksums */\n",
      "        0, /* run status */\n",
      "    },\n";
}
print "};\n",
    "int nTests = ", scalar(@tests), ";\n",
    "int nGroups = $groupIndex;\n";
exit (0);

sub getTest {
    my ($testName) = @_;
    my ($testRec);
    my ($type, $group, $dummy, $arch, @flags, $passes, $tol);

    $testsSeen{$testName} &&
	&death("Multiple test specification for `$testName'");
    $testsSeen{$testName}++;
    $testRec = \%{$testName};
    $testRec->{specs} = { %archesSpec };
    $testRec->{name} = $testName;
    push(@tests, $testRec);
    $testRec->{index} = $#tests;
    # print "Doing test $testName\n";
    loop: while (<SPECFILE>) {
      SWITCH: {
	  /^\s*$/ && do {	# Empty line indicates end of test description
	      last loop;
	  };
	  /^\s*\#.*/ && do {	# Comment
	      last SWITCH;
	  };
	  /^\s+visuals([^\#]*)(\#.*)?/ && do { # visual restriction
	      @visRestrictions = split(/\s+/, $1);
	      shift(@visRestrictions); # Remove the first null element from split
	      foreach (@visRestrictions) {
		  $legal_visualTypes{$_} ||
			   &death("Illegal visual restriction $_");
		  s/^/OG_/;
	      }
	      $testRec->{restrictions} = @visRestrictions != 0 ?
		  join(' | ', @visRestrictions) : "0";
	      # print "\tRestriction $testRec->{restrictions}\n";
	      last SWITCH;
	  };
	  /^\s+type([^\#]+)(\#.*)?/ && do { # type
	      defined($type) &&
		  &death("Multiple type specifications for $testName");
	      (($dummy, $type) = split(/\s+/, $1)) == 3 ||
		  &death("Bad syntax for 'type' line");
	      ($testRec->{type} = $type) =~ s/^/OG_/;
	      last SWITCH;
	  };
	  /^\s+group([^\#]+)(\#.*)?/ && do { # group
	      defined($group) &&
		  &death("Multiple type specifications for $testName");
	      (($dummy, $group) = split(/\s+/, $1)) == 3 ||
		  &death("Bad syntax for 'type' line");
	      # Convert '-' in group names to __Was__Minus__ so it can always be used
	      # as a variable name via eval
	      $group =~ s/-/__Was__Minus__/g;
	      last SWITCH;
	  };
	  #
	  # Everything else is architecture specific parameters
	  # 
	  s/\#.*//;
	  ($dummy, $arch, $passes, $tol, @skipConds) = split(/\s+/);
	  forLoop: foreach (@skipConds) {
	      "$_" eq "0" && next forLoop;
	      $legal_skipConditions{$_} || &death("Illegal skip condition $_");
	      s/^/OG_/;
	  }
	  ($testRec->{specs}{$arch}->[0]) &&
	      &death("Multiple specification for arch $arch");
	  defined($arches{$arch}) || &death("Unkown architecture `$arch'");
	  $testRec->{specs}{$arch} = [ $passes, $tol, join(' | ', @skipConds) ];
      }
    }
    defined($testRec->{restrictions}) ||
	die("Error: Test `$testName' is missing visual restrictions spec.\n");
    defined($type) ||
	die("Error: Test `$testName' is missing type spec.\n");
    defined($group) ||
	die("Error: Test `$testName' is missing group spec.\n");
    while (($arch, $spec) = each %{$testRec->{specs}}) {
	"$spec->[0]" ne "undef" ||
	    die("Error: Test $testName is missing spec for $arch\n");
    }
    $testRec->{group} = $group;
    $groups{$group}->{nTests}++;
    push(@{$groups{$group}->{tests}}, $testRec);
}

#
# Take a list name as a parameter and create a hash table name legal_<name>
# such that for each element of the original list $legal_<name>{element} == 1
#
sub listToLegalHash {
    my($name) = @_;
    my($list, $hash);

    $list = \@{$name};
    $hash = \%{"legal_" . $name};
    foreach (@$list) {
	$hash->{$_} = 1;
    }
}

sub death {
    die "Error: @_ at line $.\n";
}
