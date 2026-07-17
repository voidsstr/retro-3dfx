echo off
cd lib
nmake %1
cd ..\func
nmake %1
cd ..\env
nmake %1
echo on
