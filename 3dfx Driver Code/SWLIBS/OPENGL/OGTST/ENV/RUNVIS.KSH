#! c:/mksnt/sh.exe

for i in $* ; do
ogtst -V $i -A avoid.txt | tee log.$i
done
