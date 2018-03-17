@echo off
@rem $LynxId: lynx.bat,v 1.5 2018/03/14 08:42:53 tom Exp $
@rem Claudio Santambrogio
@rem improved by loto1992@inbox.ru
setlocal

set TERM=vt100
set HOME=%~dp0
set TEMP=%HOME%tmp

set LYNX_CFG=%HOME%lynx-demo.cfg
set LYNX_LSS=%HOME%lynx.lss

"%HOME%lynx.exe" %1 %2 %3 %4 %5
endlocal
