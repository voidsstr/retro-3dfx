@echo off
set RESFILE=%BUILD_ROOT_ARCADE%\src\atd\atdres.h
set RTFFILE=%BUILD_ROOT_ARCADE%\src\atd\atd.rtf
echo // MAKEHELP.BAT generated Help Map file.  Used by DEMO.HPJ. >"hlp\demo.hm"
echo. >>"hlp\demo.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\demo.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 %RESFILE% >>"hlp\demo.hm"
echo. >>"hlp\demo.hm"
echo // Prompts (IDP_*) >>"hlp\demo.hm"
makehm IDP_,HIDP_,0x30000 %RESFILE% >>"hlp\demo.hm"
echo. >>"hlp\demo.hm"
echo // Resources (IDR_*) >>"hlp\demo.hm"
makehm IDR_,HIDR_,0x20000 %RESFILE% >>"hlp\demo.hm"
echo. >>"hlp\demo.hm"
echo // Dialogs (IDD_*) >>"hlp\demo.hm"
makehm IDD_,HIDD_,0x20000 %RESFILE% >>"hlp\demo.hm"
echo. >>"hlp\demo.hm"
echo // Frame Controls (IDW_*) >>"hlp\demo.hm"
makehm IDW_,HIDW_,0x50000 %RESFILE% >>"hlp\demo.hm"
REM -- Make help for Project DEMO

echo Building Win32 Help files
copy %RTFFILE% hlp
start /wait hcrtf -x "hlp\demo.hpj"
echo.
copy "hlp\demo.hlp" .
echo.
