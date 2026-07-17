#!/usr/local/bin/perl
## 
## You should set HAL_VIDEO=0 and HAL_NOINIT=1 before running this.
## 

use lib './packages';
use h3defs;

$ENV{"HAL_VIDEO"} = 0;

##
## default values
##

$header = '             /------- input adjustments ---------\  /-output adj -\
clk_freq     nodelay  inv_sample  inv_clk  del_amt  nodelay del_amt  pass/fail';

$initial_draminit1 = 0x1002031;
printf "initial draminit1 = %x\n", $initial_draminit1;

##
## first, heat up part a lot by running perf -p 20, @ 120MHz
##
print "Heat it up!\n";
$ENV{SSTH3_MEMCLOCK} = 120;
$init = 1;
$passes = 3;
$fail = &run_test($init);

###
### stepping for i/o delays
###
print "Now, speed testing.\n";
$passes = 1;
$i_step = 4;
$o_step = 4;

for ($clk = 105; $clk < 130; $clk+=5) {
    $ENV{SSTH3_MEMCLOCK} = $clk;

    @results=();
    
    print "$header\n";  
    printf OUTFILE "==========================================================\n";
    
    for ($o_del_min=0; $o_del_min<=1; $o_del_min++) {

	$o_del_max = ($o_del_min==0) ? 1 : 32;

	for ($o_del=0; $o_del<$o_del_max; $o_del+= $o_step) {
	    
	    ##
	    ## 1-0, 0-0, 0-1, 0-2 ...
	    ##
	    for ($i_del_min=0; $i_del_min<=1; $i_del_min++) {

		$i_del_max = ($i_del_min==0) ? 1 : 16;
		
		for ($i_del_clk_inv=0; $i_del_clk_inv<=1; $i_del_clk_inv++) {

		    for ($i_del_inv_samp=0; $i_del_inv_samp<=1; $i_del_inv_samp++) {
			
			for ($i_del=0; $i_del<$i_del_max; $i_del+=$i_step) {
		    
			    $init = 0;
			    $init |= $h3def{SST_SGRAM_CLK_NODELAY} if (!$i_del_min);
			    $init |= $h3def{SST_SGRAM_USE_INV_SAMPLE} if ($i_del_inv_samp);
			    $init |= $h3def{SST_SGRAM_DEL_CLK_INVERT} if ($i_del_clk_inv);
			    $init |= ($i_del << $h3def{SST_SGRAM_CLK_ADJ_SHIFT}) & $h3def{SST_SGRAM_CLK_ADJ};
			    $init |= ($o_del << $h3def{SST_SGRAM_OFLOP_DEL_ADJ_SHIFT}) & $h3def{SST_SGRAM_OFLOP_DEL_ADJ};
			    $init |= $h3def{SST_SGRAM_OFLOP_TRANS_LATCH} if (!$o_del_min);

#		     $i_del_adj = ($i_del & 0x7) << 1;
#		     $i_del_otherbits = (($i_del>>2) & 0x6) | $i_del_min;
#                    $init = (($i_del_adj << $h3def{SST_SGRAM_CLK_ADJ_SHIFT}) | $i_del_otherbits;
#                    $init = $init | ($o_del << 7) | ($o_del_min << 11);

			    $fail = &run_test($init);
			    push (@results, $init . "_" . $fail . "_" . $clk); 
			    $outstr = &make_result($init, $fail, $clk);
			    print "$outstr\n";
			}
			
		    }
		    
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
    $nodelay    = ($init & $h3def{SST_SGRAM_CLK_NODELAY}) ? 1 : 0;
    $inv_sample = ($init & $h3def{SST_SGRAM_USE_INV_SAMPLE}) ? 1 : 0;
    $inv_clk    = ($init & $h3def{SST_SGRAM_DEL_CLK_INVERT}) ? 1 : 0;
    $del_amt    = ($init & $h3def{SST_SGRAM_CLK_ADJ}) >> $h3def{SST_SGRAM_CLK_ADJ_SHIFT};
    $o_delay    = ($init & $h3def{SST_SGRAM_OFLOP_DEL_ADJ}) >> $h3def{SST_SGRAM_OFLOP_DEL_ADJ_SHIFT};
    $o_trans    = ($init & $h3def{SST_SGRAM_OFLOP_TRANS_LATCH}) ? 1 : 0;

    $result = sprintf "[%3d MHz]  %7d  %10d  %7d  %7d  %7d %7d  %9s",
    $clk, $nodelay, $inv_sample, $inv_clk, $del_amt, $o_trans, $o_delay, $passfail;

    $result;
}

sub run_test {
    my($init) = @_;
    my($error,$mask,$dramInit1);

    $mask = $h3def{SST_SGRAM_CLK_NODELAY} |
	$h3def{SST_SGRAM_USE_INV_SAMPLE} |
	$h3def{SST_SGRAM_DEL_CLK_INVERT} |
	$h3def{SST_SGRAM_CLK_ADJ} |
	$h3def{SST_SGRAM_OFLOP_DEL_ADJ} |
	$h3def{SST_SGRAM_OFLOP_TRANS_LATCH};
    

    $dramInit1 = ($initial_draminit1 & ~$mask) | ($init & $mask);
#    &execCommand("perl wr.pl DRAMINIT1 $dramInit1");
    $ENV{SSTH3_DRAMINIT1} = $dramInit1;
    $error = &execCommand("perf -O5 -p" . $passes);

    return $error;
}


sub execCommand {
    my ($cmd) = @_;
    my ($fail) = 0;

    open (CMD, "$cmd |") || die "can't execute command: $cmd";
    while (<CMD>) {
	$fail = 1 if /ERROR/;
    }
    close (CMD);

    return $fail;
}

  
