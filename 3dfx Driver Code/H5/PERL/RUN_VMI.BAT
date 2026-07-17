perl 3dvideo100.pl
perl pdown.pl
perl out.pl 70 403c # vmi enabled; single buffer; active low hsync; active high vsync, vactive; UYVY; 
#perl out.pl 70 404a # vmi enabled; single buffer; active low hsync; active high vsync, vactive; YUYV
perl out.pl 5c 00c83301 # auto-flipping enabled; UYVY422; desktop disabled
#perl out.pl 5c 0a81381 # auto-flipping enabled; YUYV422 + desktop enable
#perl out.pl 5c 0281381 # auto-flipping enabled; RGB16 + desktop enable
perl out.pl ec c00 # buffer 0 base address
perl out.pl f0 c00000 # buffer 1 base address
#perl out.pl f8 b40 # stride = 720 X 4
#perl out.pl e8 b400b40  # stride = 720 X 4
perl out.pl f8 5a0 # stride = 720 X 2
perl out.pl e8 5a005a0  # stride = 720 X 2
perl out.pl 78 1c6025c2 # vmi resetn = 1
perl out.pl 9c 000000 # overlay start coord
perl out.pl a0 1e0280 # overlay end coord
#perl out.pl a0 400500 # overlay end coord = 1024, 1280
perl out.pl a8 2d000000
perl out.pl e4 c00 # desktop base address
#Turn off weave and turn on bob
perl out.pl 70 402c # vmi enabled; single buffer; active low hsync; active high vsync, vactive; UYVY; 
perl out.pl 5c 80cbb301 # bilinear filter; auto-flipping enabled; UYVY422; desktop disabled
perl out.pl ac 80000 # dvdy = 0.5
perl out.pl e0 20000 # dvdy offset - 0.25
#perl out.pl e0 00000 # dvdy offset - 0.5
# scale down:
#perl out.pl 70 30402c # decimation enabled; vmi enabled; single buffer; active low hsync; active high vsync, vactive; UYVY; 
#perl out.pl 9c 0780ba # overlay start coord
#perl out.pl a0 1671f9 # overlay end coord
#perl out.pl 7c 1900140 # x delta
#perl out.pl 84 f000f0 # y delta
