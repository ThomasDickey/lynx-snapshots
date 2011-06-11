@echo off
rem Borland C
rem    -m                Displays the date and time stamp of each file
rem    -c                Caches auto-dependency information
make.exe -m -c -f makefile.bcb %1 %2
REM make.exe -m -c -f makefile.deb
REM make.exe -f makefile.win
