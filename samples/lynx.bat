@echo off
@rem $LynxId: lynx.bat,v 1.9 2018/03/21 16:10:36 tom Exp $
@rem demonstrate lynx with color-style
setlocal

	set TERM=vt100
rem Set HOME to make URLs in config work, though this prevents -trace
	set HOME=%~dp0
	set PATH=%HOME%;%PATH%

rem We need a temporary directory
	if not "x%LYNX_TEMP_SPACE%"=="x" goto :do_cfg
	if "x%TEMP%"=="x"                set LYNX_TEMP_SPACE=%TEMP%
	if not "x%TEMP%"=="x"            goto :do_cfg
	if "x%TMP%"=="x"                 set LYNX_TEMP_SPACE=%TMP%
	if not "x%TMP%"=="x"             goto :do_cfg

	set LYNX_TEMP_SPACE=%HOMEDRIVE%%HOMEPATH%tmp
	mkdir "%LYNX_TEMP_SPACE%"
	if not errorlevel 0              goto :do_cfg

	echo Cannot make temp-directory
	goto :eof

:do_cfg
	set LYNX_CFG_PATH=%HOME%
	set LYNX_CFG=%HOME%lynx-demo.cfg
	set LYNX_LSS=%HOME%lynx.lss

	lynx.exe %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
