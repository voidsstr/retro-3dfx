@echo off
echo *********************************************************
echo * This will build the 3dfx Banshee manufacturing diagnostic
echo *********************************************************
echo *
echo * To compile you must:
echo *      - have watcom 10.4 - 10.6 installed and in your PATH
echo *      - be running under windows 95 or windows NT
echo *********************************************************
@echo on
cd core
wmake /f wmake.mak
@echo off

if ERRORLEVEL 2 goto failure
goto success

:failure
@echo on
cd ..
@echo off
echo *
echo *** BUILD FAILED ***
echo *
goto end

:success
@echo on
cd ..
@echo off
echo *
echo *********************************************************
echo *
echo *********************************************************
echo *  If the build was successful, all target files should be
echo *  located in the 'bin' directory.
echo *
echo *  Here is the executable
echo *
@echo on
dir bin\mdc.exe
@echo off
echo *********************************************************
echo *
echo *********************************************************

:end
