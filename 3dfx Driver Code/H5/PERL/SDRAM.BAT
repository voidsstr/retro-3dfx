set ssth3_grxclock=95
set ssth3_memclock=100
perl 3dvideo.pl
perl pll.pl 95 100
perl wr.pl draminit1 0x40202031
perl wr.pl miscinit1 0x1f018021
perl wr.pl dramdata 0x37
perl wr.pl dramcommand 0x10d
set HAL_NOINIT=1
