@echo off
@rem $LynxId: clean.bat,v 1.2 2018/03/18 23:17:43 tom Exp $
@rem Remove all build-products in subdirectories, leaving only sources
@rem (and unrecognized types)
setlocal

FOR /D %%d IN (*) DO call :dosoln %%d

attrib -h *.suo /s

del/f/s/q *.old
del/f/s/q *.suo
del/f/s/q *.sdf
del/f/s/q *.user
del/f/s/q *.xml

attrib +r *.h /s
attrib +r *.bat /s
attrib +r *.sln /s
attrib +r *.vcxproj* /s
endlocal
goto :eof

:dosoln
	setlocal
	echo Cleanup %*
	cd %1

	set SOLN=
	FOR %%d IN ( *.sln ) DO set SOLN=%%d
	if not "x%SOLN%"=="x" goto :dosoln2

	echo ?? Not a solution directory
	goto :dosolnx

:dosoln2
	FOR /D %%d IN (*) DO rmdir /s /q %%d
:dosolnx
	endlocal
	goto :eof
