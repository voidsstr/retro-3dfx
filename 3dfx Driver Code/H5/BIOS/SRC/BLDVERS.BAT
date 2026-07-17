del agp-sg.xxx
del agp-sd.xxx
del pci-sg.xxx
del pci-sd.xxx

del agptvn-sd.xxx
del agptvp-sd.xxx
del pcitvn-sd.xxx
del pcitvp-sd.xxx
del agptvn-sg.xxx
del agptvp-sg.xxx
del pcitvn-sg.xxx
del pcitvp-sg.xxx

REM Build TVOUT BIOSes

nmake clean
set vgabios_opt=-DBUILD_SDRAM_BIOS -DBUILD_TVOUT_BIOS
nmake /s /nologo
ren full.rom agptvn-sd.xxx

nmake clean
set vgabios_opt=-DBUILD_SDRAM_BIOS -DBUILD_TVOUT_BIOS -DBUILD_PAL
nmake /s /nologo
ren full.rom agptvp-sd.xxx

nmake clean
set vgabios_opt=-DBUILD_TVOUT_BIOS
nmake /s /nologo
ren full.rom agptvn-sg.xxx

nmake clean
set vgabios_opt=-DBUILD_TVOUT_BIOS -DBUILD_PAL
nmake /s /nologo
ren full.rom agptvp-sg.xxx

REM Build non-TVOUT BIOSes

nmake clean
set vgabios_opt=-DBUILD_SDRAM_BIOS
nmake /s /nologo
ren full.rom agp-sd.xxx

nmake clean
set vgabios_opt=
nmake /s /nologo
ren full.rom agp-sg.xxx

ren agptvn-sd.xxx agptvn-sd.rom
ren agptvp-sd.xxx agptvp-sd.rom
ren agptvn-sg.xxx agptvn-sg.rom
ren agptvp-sg.xxx agptvp-sg.rom

ren agp-sg.xxx agp-sg.rom
ren agp-sd.xxx agp-sd.rom

del odd.rom

copy agptvn-sd.rom pcitvn-sd.rom
copy agptvp-sd.rom pcitvp-sd.rom
copy agptvn-sg.rom pcitvn-sg.rom
copy agptvp-sg.rom pcitvp-sg.rom

copy agp-sg.rom pci-sg.rom
copy agp-sd.rom pci-sd.rom

resub agptvn-sd.rom 121a 1
resub agptvp-sd.rom 121a 1
resub pcitvn-sd.rom 121a 2
resub pcitvp-sd.rom 121a 2
resub agptvn-sg.rom 121a 3
resub agptvp-sg.rom 121a 3
resub pcitvn-sg.rom 121a 4
resub pcitvp-sg.rom 121a 4

resub agp-sd.rom 121a 1
resub pci-sd.rom 121a 2
resub agp-sg.rom 121a 3
resub pci-sg.rom 121a 4
