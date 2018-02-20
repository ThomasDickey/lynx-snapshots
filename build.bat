@echo off
@rem $LynxId: build.bat,v 1.1 2018/02/19 14:54:26 tom Exp $
	setlocal
	set SSL_DIR=C:\OpenSSL-%TARGET_ARCH%

if not exist lynx.man     goto :not_lynx
if not exist lynx.cfg     goto :not_lynx
if not exist makefile.msc goto :not_lynx
if not exist make-msc.bat goto :not_lynx

if "x"=="x%PARTITION%" goto :no_build

rem The locally-built libraries are in a directory tree which has subfolders
rem for each version of Visual Studio.  The OpenSSL directories are in
rem a separate location.  Simplify building Lynx by copying the link-libraries
rem and DLLs to the build-tree.

	call :copybin \project\all\%PARTITION%
	call :copylib \project\all\%PARTITION%

	xcopy /f/q lynx.cfg bin
	xcopy /f/q samples\lynx.lss bin

	call :clean
	call :build %1

	goto :eof

:no_build
	echo The build-tools were not setup
	goto :eof

:not_lynx
	echo This is not the Lynx build-directory
	goto :eof

rem Copy configuration and OpenSSL DLLs to bin-directory to simplify
rem ad hoc testing.  The "\com" has some GnuWin32 executables which can be
rem used for decompressing temporary files.  Copy those here as well to
rem simplify packaging.

:copybin
	setlocal
	echo ** Copying files to bin-directory
	if not exist bin mkdir bin
	cd bin
	del /f /q lynx*.*
	del /f /q *.dll
	del /f /q *.exe
	copy %1\dynamic\*.dll .
	copy c:\com\bzip2.exe .
	copy c:\com\gzip.exe .
	call :copy_ssl libeay
	call :copy_ssl ssleay
	call :copy_ssl libcrypto
	call :copy_ssl libssl
	call :copy_ssl msvcr
	endlocal
	goto :eof

:copy_ssl
	setlocal
	set DESTDIR=%CD%
	cd %SSL_DIR%
	for /f "usebackq" %%i in (`dir /b /s bin`) do xcopy /s /c /f /y %%i\%1*.dll %DESTDIR%
	endlocal
	goto :eof

rem We could link with locally-built DLLs, but it would complicate
rem things unnecessarily, and in the case of the slang library, more
rem than double the size of an installer since its DLL is larger than all
rem of the other parts of Lynx combined.

:copylib
	setlocal
	echo ** Copying files to lib-directory
	if not exist lib mkdir lib
	cd lib
	del /f /q *.lib
	del /f /q bzip*.h
	del /f /q curs*.h
	del /f /q slang*.h
	del /f /q z*.h
	copy %1\include\* .
	copy %1\static\* .
	endlocal
	goto :eof

:clean
	setlocal
	call make-msc.bat clean
	endlocal
	goto :eof

:build
	setlocal
	if "x%1"=="x" call :do_default
	if not "x%1"=="x" call :do_%1
	endlocal
	goto :eof

:do_all
	call :do_default
	call :do_sock
	call :do_oldssl
	call :do_newssl
	call :do_cs
	call :do_slang
	call make-msc clean
	goto :eof

@rem just for testing
:do_default
	call :doit bin\lynx-default.exe
	goto :eof

:do_sock
	call :doit bin\lynx-sock.exe "OPT_SOCK=yes"
	goto :eof

rem The "with-openssl" batch-file simplifies some of the task of linking
rem with OpenSSL DLLs by setting SSL_LIBS for the appropriate old/new
rem filenames.

:do_oldssl
	setlocal
	call with-openssl 1.0.2n 0
	call :doit bin\lynx-oldssl.exe "OPT_SSL=yes" "OPT_SOCK=yes"
	endlocal
	goto :eof

:do_newssl
	setlocal
	call with-openssl 1.1.0g 1
	call :doit bin\lynx-newssl.exe "OPT_SSL=yes" "OPT_SOCK=yes"
	endlocal
	goto :eof

@rem for packages
:do_cs
	call :doit bin\lynx-cs.exe "OPT_SOCK=yes" "OPT_CS=yes"
	goto :eof

:do_slang
	call :doit bin\lynx-slang.exe "OPT_SOCK=yes" "SCREEN=slang"
	goto :eof

:do_none
	goto :eof

:doit
	setlocal
	echo ** START BUILD: %*
	set progname=%1
	shift
	call make-msc clean
	echo on
	call make-msc "PROGNAME=%progname%" %1 %2 %3 %4 %5 %6 %7 %8 %9
	if errorlevel 1 goto :failed
	echo ** BUILD-SUCCESS %progname%
	echo.
	%progname% -version
	echo.
	endlocal
	goto :eof
:failed
	echo ?? BUILD-FAILURE %progname%
	endlocal
	goto :eof
