REM Rough version of Dos batch makefile for MingW32 and lynx.exe 
Rem Remember to precede this by "command /E:32000" and set the 
Rem MingW32 paths 
 
SET DEFINES=-DNO_UNISTD_H 
SET DEFINES=%DEFINES% -D_WINDOWS 
SET DEFINES=%DEFINES% -DXMOSAIC_HACK 
SET DEFINES=%DEFINES% -DACCESS_AUTH 
SET DEFINES=%DEFINES% -DNO_UTMP 
SET DEFINES=%DEFINES% -DNO_CUSERID 
SET DEFINES=%DEFINES% -DNO_TTYTYPE 
SET DEFINES=%DEFINES% -DNOSIGHUP 
SET DEFINES=%DEFINES% -DDOSPATH 
SET DEFINES=%DEFINES% -DNOUSERS 
SET DEFINES=%DEFINES% -DHAVE_KEYPAD 
SET DEFINES=%DEFINES% -DVC="2.14FM" 
SET DEFINES=%DEFINES% -DUSE_SLANG 
SET DEFINES=%DEFINES% -DWIN32 
 
SET INCLUDES=-I. -I..\..\..\ -I..\..\..\SRC 
 
SET CFLAGS=-O %INCLUDES% %DEFINES% 
SET COMPILE_CMD=gcc -c %CFLAGS% 
 
 
cd WWW\Library\Implementation 
 
SET FILES=crypt.c 
SET FILES=%FILES% crypt_util.c 
SET FILES=%FILES% getline.c 
SET FILES=%FILES% getpass.c 
SET FILES=%FILES% HTAABrow.c 
SET FILES=%FILES% HTAAProt.c 
SET FILES=%FILES% HTAAUtil.c 
SET FILES=%FILES% HTAccess.c 
SET FILES=%FILES% HTAnchor.c 
SET FILES=%FILES% HTAssoc.c 
 
PAUSE 
for %%f in ( %FILES% ) do %COMPILE_CMD% %%f 
PAUSE 
 
SET FILES=HTAtom.c 
SET FILES=%FILES% HTBTree.c 
SET FILES=%FILES% HTChunk.c 
SET FILES=%FILES% HTDOS.c 
SET FILES=%FILES% HTFile.c 
SET FILES=%FILES% HTFinger.c 
SET FILES=%FILES% HTFormat.c 
SET FILES=%FILES% HTFTP.c 
SET FILES=%FILES% HTFWriter.c 
SET FILES=%FILES% HTGopher.c 
SET FILES=%FILES% HTGroup.c 
 
PAUSE 
for %%f in ( %FILES% ) do %COMPILE_CMD% %%f 
PAUSE 
 
SET FILES=HTLex.c 
SET FILES=%FILES% HTList.c 
SET FILES=%FILES% HTMIME.c 
SET FILES=%FILES% HTMLDTD.c 
SET FILES=%FILES% HTMLGen.c 
SET FILES=%FILES% HTNews.c 
SET FILES=%FILES% HTParse.c 
SET FILES=%FILES% HTPlain.c 
SET FILES=%FILES% HTRules.c 
SET FILES=%FILES% HTString.c 
 
PAUSE 
for %%f in ( %FILES% ) do %COMPILE_CMD% %%f 
PAUSE 
 
SET FILES=HTStyle.c 
SET FILES=%FILES% HTTCP.c 
SET FILES=%FILES% HTTelnet.c 
SET FILES=%FILES% HTTP.c 
SET FILES=%FILES% HTUU.c 
SET FILES=%FILES% HTVMS_WaisUI.c 
SET FILES=%FILES% HTVMS_WaisProt.c 
SET FILES=%FILES% HTWAIS.c 
SET FILES=%FILES% HTWSRC.c 
SET FILES=%FILES% SGML.c 
 
PAUSE 
for %%f in ( %FILES% ) do %COMPILE_CMD% %%f 
PAUSE 
 
ar crv *.o libwww.a 
 
PAUSE 
 
cd ..\..\..\src\chrtrans 
 
SET INCLUDES=-I. -I.. -I..\.. -I..\..\WWW\Library\Implementation 
SET CFLAGS=-O %INCLUDES% %DEFINES% 
SET COMPILE_CMD=gcc -c %CFLAGS% 
 
%COMPILE_CMD% makeuctb.c 
 
PAUSE 
gcc -o makeuctb.exe makeuctb.o 
PAUSE 
call make32.bat 
PAUSE 
cd ..\ 
 
SET INCLUDES=-I. -I.. -I.\chrtrans -I..\WWW\Library\Implementation 
SET LIBS=-lslang -L..\WWW\Library\Implementation -lwww 
SET CFLAGS=-O %INCLUDES% %DEFINES% 
SET COMPILE_CMD=gcc -c %CFLAGS% 
 
SET FILES=%FILES% DefaultStyle.c 
SET FILES=%FILES% GridText.c 
SET FILES=%FILES% HTAlert.c 
SET FILES=%FILES% HTFWriter.c 
SET FILES=%FILES% HTInit.c 
SET FILES=%FILES% HTML.c 
SET FILES=%FILES% LYBookmark.c 
SET FILES=%FILES% LYCgi.c 
SET FILES=%FILES% LYCharSets.c 
SET FILES=%FILES% LYCharUtils.c 
SET FILES=%FILES% LYClean.c 
SET FILES=%FILES% LYCookie.c 
SET FILES=%FILES% LYCurses.c 
SET FILES=%FILES% LYDownload.c 
 
PAUSE 
for %%f in ( %FILES% ) do %COMPILE_CMD% %%f 
PAUSE 
 
SET FILES=LYEdit.c 
SET FILES=%FILES% LYEditmap.c 
SET FILES=%FILES% LYexit.c 
SET FILES=%FILES% LYExtern.c 
SET FILES=%FILES% LYForms.c 
SET FILES=%FILES% LYGetFile.c 
SET FILES=%FILES% LYHash.c 
SET FILES=%FILES% LYHistory.c 
SET FILES=%FILES% LYJump.c 
SET FILES=%FILES% LYKeymap.c 
SET FILES=%FILES% LYLeaks.c 
SET FILES=%FILES% LYList.c 
SET FILES=%FILES% LYLocal.c 
SET FILES=%FILES% LYMail.c 
SET FILES=%FILES% LYMain.c 
SET FILES=%FILES% LYMainLoop.c 
 
PAUSE 
for %%f in ( %FILES% ) do %COMPILE_CMD% %%f 
PAUSE 
 
SET FILES=LYMap.c 
SET FILES=%FILES% LYNews.c 
SET FILES=%FILES% LYOptions.c 
SET FILES=%FILES% LYPrettySrc.c 
SET FILES=%FILES% LYPrint.c 
SET FILES=%FILES% LYrcFile.c 
SET FILES=%FILES% LYReadCFG.c 
SET FILES=%FILES% LYSearch.c 
SET FILES=%FILES% LYShowInfo.c 
SET FILES=%FILES% LYStrings.c 
SET FILES=%FILES% LYStyle.c 
SET FILES=%FILES% LYTraversal.c 
SET FILES=%FILES% LYUpload.c 
SET FILES=%FILES% LYUtils.c 
SET FILES=%FILES% mktime.c 
SET FILES=%FILES% strstr.c 
SET FILES=%FILES% UCAuto.c 
SET FILES=%FILES% UCAux.c 
SET FILES=%FILES% UCdomap.c 
 
PAUSE 
for %%f in ( %FILES% ) do %COMPILE_CMD% %%f 
PAUSE 
 
gcc -o lynx.exe %LIBS% 
strip lynx.exe 
ECHO "Welcome to lynx!" 
