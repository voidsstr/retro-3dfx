@echo off
echo ****************************************************
echo * CLEAN
echo ****************************************************
echo *
echo * This erases all intermediate files, after running
echo * clean, you will likely want to run a 'build'
echo ****************************************************
attrib -r bin\mdc.exe
erase bin\mdc.exe
cd core
@echo on
wmake /f wmake.mak clean
cd ..
