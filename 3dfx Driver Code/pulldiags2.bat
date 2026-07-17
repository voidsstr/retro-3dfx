REM *************************************************
REM this should be run from ./tdfx/devel
REM it should be invoked with args like swlibs h3 cvg
REM *************************************************
@echo off
setlocal

REM setlocal doesn't seem to be working under Win95, so explicitly clear vars
set SWLIBS=
set H3=
set H4=
set CVG=
set SST2= 
set APE=
set H5=

REM get user inputs
set dirs=%1 %2 %3 %4 %5 %6 %7 %8 %9

REM first go thru and create all the directories if they don't exist
for %%d IN (%dirs%) DO if not exist %%d md %%d
@echo on
REM now go thru and check out files in these directories
for %%d IN (%dirs%) DO ss get -I-N -GCD -GL%%d $/devel/%%d

@echo off
REM now build up the list of subdirectories we need
for %%d IN (%dirs%) DO if %%d == swlibs set SWLIBS=swlibs\bin swlibs\binsrc swlibs\lib swlibs\libsrc swlibs\include swlibs\incsrc swlibs\fxmisc swlibs\fxmemmap swlibs\fxagp swlibs\fxntdrvr swlibs\newpci swlibs\fxremap
for %%d IN (%dirs%) DO if %%d == h3 set H3=h3\csim h3\diags h3\hal h3\include h3\incsrc h3\util h3\bin h3\binsrc h3\lib h3\libsrc h3\perl h3\cinit
for %%d IN (%dirs%) DO if %%d == h4 set H4=h4\csim h4\diags h4\hal h4\include h4\incsrc h4\util h4\bin h4\binsrc h4\lib h4\libsrc h4\cinit
for %%d IN (%dirs%) DO if %%d == ape set ape=ape\csim ape\diags ape\hal ape\include ape\incsrc ape\util ape\bin ape\binsrc ape\lib ape\libsrc ape\cinit
for %%d IN (%dirs%) DO if %%d == h5 set H5=h5\csim h5\diags h5\hal h5\include h5\incsrc h5\util h5\bin h5\binsrc h5\lib h5\libsrc h5\cinit
for %%d IN (%dirs%) DO if %%d == sst2 set SST2=sst2\csim sst2\diags sst2\hal sst2\include sst2\incsrc sst2\util
for %%d IN (%dirs%) DO if %%d == cvg set CVG=cvg\csim cvg\diags cvg\hal cvg\include cvg\incsrc cvg\util

REM first go thru and create all the sub directories if they don't exist
for %%d IN (%SWLIBS% %H3% %H4% %CVG% %SST2% %APE% %H5%) DO if not exist %%d md %%d

@echo on
REM no go thru and get all the files 
for %%d IN (%SWLIBS% %H3% %H4% %CVG% %SST2% %APE% %H5%) DO ss get -r -I-N -GCD -GL%%d $/devel/%%d
