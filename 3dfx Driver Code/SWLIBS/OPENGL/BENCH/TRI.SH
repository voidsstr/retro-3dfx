#!/bin/mksdos/tcsh.exe

set args = "-s 4 -p 1 -c cos"
glcos

trispeed $args -P poly  -t 2
trispeed $args -P poly  -t 3
trispeed $args -P poly  -t 4
trispeed $args -P poly  -t 5
trispeed $args -P tfan  -t 1
trispeed $args -P tfan  -t 2
trispeed $args -P tfan  -t 3
trispeed $args -P tfan  -t 4
trispeed $args -P tfan  -t 5
trispeed $args -P tfan  -t 6
trispeed $args -P quad  -t 2

set args = "-s 4 -p 1 -c qk"
glqk

trispeed $args -P poly  -t 2
trispeed $args -P poly  -t 3
trispeed $args -P poly  -t 4
trispeed $args -P poly  -t 5
trispeed $args -P tfan  -t 1
trispeed $args -P tfan  -t 2
trispeed $args -P tfan  -t 3
trispeed $args -P tfan  -t 4
trispeed $args -P tfan  -t 5
trispeed $args -P tfan  -t 6
trispeed $args -P quad  -t 2

