#!/usr/local/bin/perl

use lib '.\packages';

use h3init;
use pciwrap;
use h3regs;
use h3defs;
h3init::h3initMapBoard();

        # Get screen size
        $screen_x = 640;
        $screen_y = 480;
        $refresh = 60;
       h3init::h3initVideoTiming ($screen_x, $screen_y, $refresh);
