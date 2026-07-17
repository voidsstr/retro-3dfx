#!/usr/local/bin/perl
## 
## You should set HAL_VIDEO=0 and HAL_NOINIT=1 before running this.
## 

while ($arg = pop(@ARGV)) {
    if ($arg =~ /-sdram/) {
	$type_sdram = 1;
    } elsif ($arg =~ /-o/) {
	$cycle_outdel = 1;
    } else {
	&usage;
    }
}

$ENV{"HAL_VIDEO"} = 0;
$ENV{"HAL_NOINIT"} = 1;
$header = '             /------- input adjustments ---------\  /-output adj -\
clk_freq     nodelay  inv_sample  inv_clk  del_amt  nodelay del_amt  pass/fail';

#$initial_draminit1 = 0x1002031;
#$initial_draminit1 = 0x2238031;
#if ($type_sdram) {
#    $initial_draminit1 = 0x40202031;
#}
open (CMD, "perl rd.pl draminit1 |") || "can't exec perl rd.pl draminit1";
while (<CMD>) {
    if (/DRAMINIT1 = (\S+)/) {
	$initial_draminit1 = hex($1);
	print ;
    }
}
close(CMD);
$type_sdram = ($initial_draminit1 & 0x40000000) ? 1 : 0;

printf "initial draminit1 = %x\n", $initial_draminit1;

##
## first, heat up part a lot by running perf -p 20, @ 120MHz
##
print "Heat it up!\n";
$clk = 100;
$cmd = "perl m:\\users\\tao\\mpll.pl $clk";
&exec_cmd($cmd);
$init = 1;
$passes = 3;
$fail = &run_test($init);
$passes = 3;
print "Now, speed testing.\n";

###
### stepping for i/o delays
###
$i_step = 1;
$o_step = 1;

for ($clk = 115; $clk < 125; $clk+=5) {
    $cmd = "perl m:\\users\\tao\\mpll.pl $clk";
    &exec_cmd($cmd);
 
    @results=();

    print "$header\n";  
    printf OUTFILE "==========================================================\n";
    
#	 for ($o_mindel=1; $o_mindel>=0; $o_mindel--) {
	{
		###
		### fix output mindel = 0;
		###
		$o_mindel = 0;

		if ($o_mindel) {
			$odel_max = 1;
		} else {
			$odel_max = 17;
			$odel_max = 4;
		}
#		  for ($o_del_adj=$odel_max; $o_del_adj>=0; $o_del_adj-= $o_step) {
		{
			###
			### fix output delay
			###
			$o_del_adj = 1;

			##
			## 1-0, 0-0, 0-1, 0-2 ...
			##
			for ($i_mindel=1; $i_mindel>=0; $i_mindel--) {
#			for ($i_mindel=1; $i_mindel>=1; $i_mindel--) {
				if ($i_mindel) {
					$idel_max = 1;
				} else {
					$idel_max = 32;
#					$idel_max = 16;
				}
				for ($i_del=0; $i_del<$idel_max; $i_del+=$i_step) {
					$i_del_adj = ($i_del & 0x7) << 1;
					$i_del_otherbits = (($i_del>>2) & 0x6) | $i_mindel;

                    $init = ($i_del_adj << 3) | $i_del_otherbits;
                    $init = $init | ($o_del_adj << 7) | ($o_mindel << 11);


                    $fail = &run_test($init);
                    push (@results, $init . "_" . $fail . "_" . $clk); 
                    $outstr = &make_result($init, $fail, $clk);
                    print "$outstr\n";
                }

            }
            
        }
    }

    $outfile = "clk_${clk}.log";
    open (OUTFILE, "> $outfile");
    print OUTFILE "\n";
    print OUTFILE "\n";
    print OUTFILE "Results: mclk = $clk\n";
    printf OUTFILE "$header\n";
    printf OUTFILE "==========================================================\n";
    foreach $result (@results) {
        ($init, $fail, $clk) = split(/_/, $result);
        
        $outstr = &make_result($init, $fail, $clk);
        printf OUTFILE "$outstr\n";
    }
    close(OUTFILE);
}

exit;

sub make_result {
    local($init, $fail, $clk) = @_;
    local($result);

    $passfail   = $fail ? "FAIL" : "ok";
    $nodelay    = ($init & 1) ? 1 : 0;
    $inv_sample = ($init & 2) ? 1 : 0;
    $inv_clk    = ($init & 4) ? 1 : 0;
    $del_amt    = ($init >> 3) & 0xf;
    $o_delay    = ($init >> 7) & 0xf;
    $o_trans    = ($init >> 11) & 1;

    $result = sprintf "[%3d MHz]  %7d  %10d  %7d  %7d  %7d %7d  %9s",
    $clk, $nodelay, $inv_sample, $inv_clk, $del_amt, $o_trans, $o_delay, $passfail;

    $result;
}

sub exec_cmd {
    local($cmd) = @_;
    open (CMD, "$cmd |") || die "can't execute a command: $cmd";
    while (<CMD>) {print;}
    close (CMD);
}

sub run_test {
    local ($init) = @_;
    local ($error);

    $newdram = ($initial_draminit1 & ~0x1ffe000) | $init<<13;
    $cmd= "perl wr.pl DRAMINIT1 $newdram";
    open (CMD, "$cmd |") || die "can't execute a command: $cmd";
    while (<CMD>) {;}
    close (CMD);

    if ($type_sdram) {
	$cmd = "sblt -s5 -p" . $passes;
    } else {
	$cmd = "perf -O6 -t 15 -y2 -w 800 -p" . $passes;
    }
    open(CMD, "$cmd |") || die "can't execute a command: $cmd";

    $error = 0;
    while (<CMD>) {
		$error = 1 if (/ERROR/);
    }

    close(CMD);
    $error;
}

sub usage {
    print "run.pl [-sdram]\n";
    print "   -sdram - run with sdram type chips\n";
    exit;
}
