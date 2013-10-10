; $LynxId: lynx.nsi,v 1.3 2013/10/09 19:06:24 tom Exp $
; Script originally generated with the Venis Install Wizard, but customized.
; The Inno Setup script is preferred; but this can be built via cross-compiling.

; Define the application name
!define APPNAME "Lynx"
!define EXENAME "lynx.exe"

!define VERSION_EPOCH "2"
!define VERSION_MAJOR "8"
!define VERSION_MINOR "8"
!define VERSION_LEVEL "17"
!define VERSION_PATCH "dev.17"

!define SUBKEY "Lynx"

!define INSTALL "Lynx - web browser"
!define VERSION "${VERSION_EPOCH}.${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_PATCH}"

; Main Install settings
Name "${INSTALL}"
InstallDir "$PROGRAMFILES\${INSTALL}"
InstallDirRegKey HKLM "Software\${SUBKEY}" "$INSTDIR\bin"
OutFile "NSIS-Output\${APPNAME}-${VERSION}-setup.exe"

CRCCheck on
SetCompressor /SOLID lzma

VIAddVersionKey ProductName "${SUBKEY}"
VIAddVersionKey CompanyName "Thomas E. Dickey"
VIAddVersionKey LegalCopyright "© 1997-2012,2013, Thomas E. Dickey"
VIAddVersionKey FileDescription "Lynx Installer (MinGW)"
VIAddVersionKey FileVersion "${VERSION}"
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey Comments "This installer was built with NSIS and cross-compiling to MinGW."
VIAddVersionKey InternalName "setup-${APPNAME}-${VERSION}.exe"
VIProductVersion "${VERSION_EPOCH}.${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_LEVEL}"

; Modern interface settings
!include "MUI.nsh"

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_RUN "$INSTDIR\${EXENAME}"

!insertmacro MUI_PAGE_WELCOME
; FIXME "..\README"
!insertmacro MUI_PAGE_LICENSE "..\COPYHEADER"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_RESERVEFILE_LANGDLL

!define LYNX_CFG "lynx.cfg"

InstType "Full"		; SectionIn 1
InstType "Typical"	; SectionIn 2
InstType "Minimal"	; SectionIn 3

Section "${APPNAME}" Section1

	SectionIn 1 2 3

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR"
	File /oname=${EXENAME} ".\bin\*${EXENAME}"
	File ".\bin\bzip2.exe"
	File ".\bin\gzip.exe"
	File ".\bin\*.dll"

	; TODO: bzip2.exe, gzip.exe, *.dll

	CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\${EXENAME}"
	CreateShortCut "$SENDTO\${APPNAME}.lnk" "$INSTDIR\${EXENAME}"
	CreateDirectory "$SMPROGRAMS\${INSTALL}"
	CreateShortCut "$SMPROGRAMS\${INSTALL}\${INSTALL}.lnk" "$INSTDIR\${EXENAME}"
	CreateShortCut "$SMPROGRAMS\${INSTALL}\Uninstall.lnk" "$INSTDIR\uninstall.exe"

	File ".\share\lynx_doc\samples\*.lss"

	; preinstall lynx.cfg as a temporary file
	File /oname=config.tmp ".\etc\${LYNX_CFG}"

	; at install-time, append our customization
	FileOpen $0 "config.tmp" r
	FileOpen $1 ".\${LYNX_CFG}" w
	loop:
		FileRead $0 $2
		IfErrors done
		FileWrite $1 "$2"
		goto loop
	done:
		FileWrite $1 "HELPFILE:$INSTDIR\help\Lynx_help_main.html.gz$\n"
		FileWrite $1 "COLOR_STYLE:$INSTDIR\opaque.lss$\n"
		FileWrite $1 "CHMOD_PATH:$\n"
		FileWrite $1 "COPY_PATH:$\n"
		FileWrite $1 "MKDIR_PATH:$\n"
		FileWrite $1 "MV_PATH:$\n"
		FileWrite $1 "RMDIR_PATH:$\n"
		FileWrite $1 "RM_PATH:$\n"
		FileWrite $1 "TOUCH_PATH:$\n"
	FileClose $0
	FileClose $1

	Delete "config.tmp"

	File "..\samples\lynx.bat"
	File "..\samples\lynx-demo.cfg"

	File "..\samples\jumps.htm"
	File "..\samples\home.htm"
	File "..\samples\lynx_bookmarks.htm"

	SetOutPath "$INSTDIR\icon"
	File "..\samples\lynx.ico"

	SetOutPath "$INSTDIR\tmp"

SectionEnd

Section "documentation" Section2

	SectionIn 1 2

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR"
	File "..\lynx.man"

	SetOutPath "$INSTDIR\doc"
	File ".\share\lynx_doc\CHANGES*.*"
	File ".\share\lynx_doc\COPY*.*"
	File ".\share\lynx_doc\PROBLEM*.*"
	File ".\share\lynx_doc\README*.*"

	SetOutPath "$INSTDIR\help"
	File ".\share\lynx_help\*.*"

	SetOutPath "$INSTDIR\help\keystrokes"
	File ".\share\lynx_help\keystrokes\*.*"

SectionEnd

Section "samples" Section3

	SectionIn 1

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\doc\samples"
	File ".\share\lynx_doc\samples\*.*"

	SetOutPath "$INSTDIR\doc\test"
	File ".\share\lynx_doc\test\*.*"

SectionEnd

Section -FinishSection

	WriteRegStr HKLM "Software\${SUBKEY}" "" "$INSTDIR"
	WriteRegStr HKLM "Software\${SUBKEY}" "Environment" ""
	WriteRegStr HKLM "Software\${SUBKEY}\Environment" "LYNX_CFG" "$INSTDIR\${LYNX_CFG}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALL}" "DisplayName" "${INSTALL} ${VERSION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALL}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Section1} "${SUBKEY}"
	!insertmacro MUI_DESCRIPTION_TEXT ${Section2} "Documentation"
	!insertmacro MUI_DESCRIPTION_TEXT ${Section3} "Samples and Test-files."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall

	;Remove from registry...
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALL}"
	DeleteRegKey HKLM "SOFTWARE\${SUBKEY}"

	; Delete self
	Delete "$INSTDIR\uninstall.exe"

	; Delete Shortcuts
	Delete "$DESKTOP\${APPNAME}.lnk"
	Delete "$SENDTO\${APPNAME}.lnk"
	Delete "$SMPROGRAMS\${INSTALL}\${APPNAME}.lnk"
	Delete "$SMPROGRAMS\${INSTALL}\Uninstall.lnk"

	; Clean up application
	Delete "$INSTDIR\${EXENAME}"
	Delete "$INSTDIR\doc\samples\*.*"
	Delete "$INSTDIR\doc\test\*.*"
	Delete "$INSTDIR\doc\*.*"
	Delete "$INSTDIR\help\keystrokes\*.*"
	Delete "$INSTDIR\help\*.*"
	Delete "$INSTDIR\icon\*.*"
	Delete "$INSTDIR\tmp\*.*"
	Delete "$INSTDIR\bzip2.exe"
	Delete "$INSTDIR\gzip.exe"
	Delete "$INSTDIR\*.bat"
	Delete "$INSTDIR\*.cfg"
	Delete "$INSTDIR\*.dll"
	Delete "$INSTDIR\*.htm"
	Delete "$INSTDIR\*.man"
	Delete "$INSTDIR\*.lss"
	Delete "$INSTDIR\*.tmp"

	; Remove remaining directories
	RMDir "$SMPROGRAMS\${INSTALL}"
	RMDir "$INSTDIR\doc\samples"
	RMDir "$INSTDIR\doc\test"
	RMDir "$INSTDIR\doc"
	RMDir "$INSTDIR\help\keystrokes"
	RMDir "$INSTDIR\help"
	RMDir "$INSTDIR\icon"
	RMDir "$INSTDIR\tmp"
	RMDir "$INSTDIR\"

SectionEnd

; eof