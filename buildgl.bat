@echo off
cd src
nmake /nologo /f Makefile.win all
cd ../files
nmake /nologo /f Makefile.win
cd samples
nmake /nologo /f MakeFile.win
cd ../..
combat.exe maps/map1 BlackCats.exe Fire.exe
echo ==========
echo =====================
echo This is a sample run.
echo combat.exe maps/map1 BlackCats.exe Fire.exe
echo To run your binaries type:
echo combat.exe maps/mapfile executable1 executable2