@ECHO off
@rem $LynxId: oldlynx.bat,v 1.8 2018/03/21 16:08:49 tom Exp $
@rem demonstrate lynx without color-style
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
	lynx.exe -help 2>&1 | find "-lss" >NUL
	if not errorlevel 1 goto :do_cfg2
	echo This copy of Lynx was not compiled with color-style.
	goto :eof

:do_cfg2
	set LYNX_CFG_PATH=%HOME%
	set LYNX_CFG=%LYNX_TEMP_SPACE%\oldlynx.cfg
	set LYNX_LSS=

	echo DEFAULT_COLORS:off    >>"%LYNX_CFG%"
	echo include:lynx-demo.cfg >>"%LYNX_CFG%"
	echo COLOR_STYLE:          >>"%LYNX_CFG%"
	echo NESTED_TABLES:off     >>"%LYNX_CFG%"

:do_exe
	lynx.exe -lss="" %1 %2 %3 %4 %5 %6 %7 %8 %9
	erase %LYNX_CFG%
endlocal
