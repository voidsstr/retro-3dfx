FxRemap Lib

Derived from FxRemap...


FxRemap

Purpose:
This program is meant to be used in conjunction with the 3DFX Voodoo
graphics and the S3 968/868 chipset.  It's purpose is to correct an
addressing bug in the S3 968 chipset.  It will also work around other
possible addressing conflicts with your voodoo board.


How to Use:
If you have the S3 chipset simply add a line at the beginning of your
autoexec.bat to call fxRemap.exe.  This should be all that you have to
do.


Advanced Options:
FxRemap.exe will take three possible parameters.

/dS3 : This remaps the board even if the S3 chipset is not present.
example: fxremap.exe /dS3.  This option is unneccessary with this version
of FxRemap.exe.

/x : Excludes a memory range to map the board into.
example: fxremp.exe /x 0xE0000000-0xF0000000

/f : Used to force the voodoo card to a certain memory location.
This feature is no longer supported.

