@echo off
@rem $LynxId: lynx.bat,v 1.6 2018/03/18 17:20:13 tom Exp $
@rem Claudio Santambrogio
@rem improved by loto1992@inbox.ru
setlocal

set TERM=vt100
set HOME=%~dp0

rem  normally set, but just in case..
if "x%TEMP%"=="x" set TEMP=%HOME%tmp
mkdir "%TEMP%"

set LYNX_CFG=%HOME%lynx-demo.cfg
set LYNX_LSS=%HOME%lynx.lss

"%HOME%lynx.exe" %1 %2 %3 %4 %5
endlocal
