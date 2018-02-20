@echo off
setlocal
if "x%TARGET_ARCH"=="x" set TARGET_ARCH=x86
rem Build with Visual C++
cd src\chrtrans
nmake -f makefile.msc MACHINE=%TARGET_ARCH% %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\..
nmake -f makefile.msc MACHINE=%TARGET_ARCH% %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
