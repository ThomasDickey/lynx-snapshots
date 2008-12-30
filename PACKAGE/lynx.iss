; $LynxId: lynx.iss,v 1.1 2008/12/29 13:27:28 tom Exp $
;
; This is the BASE script for different flavors of the installer for Lynx.
; It can be overridden to select different source-executables (and their associated
; screen library, e.g., pdcurses or slang).
;
; The script assumes environment variables have been set, e.g., to point to
; data which is used by the installer:
;
; LYNX_BINDIR - directory containing lynx.exe (or different names)
; LYNX_DLLSDIR - directory containing curses or slang dlls.
; LYNX_DOCSDIR - files and subdirectories installed from Unix with "make install-doc"
; LYNX_HELPDIR - files and subdirectories installed from Unix with "make install-help"

#include "version.iss"

#ifndef MyAppExeName
#define MyAppExeName "lynx.exe"
#endif

#ifndef SourceExeName
#define SourceExeName "lynx.exe"
#endif

#ifndef NoScreenDll
#ifndef ScreenDllName
#define ScreenDllName "pdcurses.dll"
#endif
#endif

#ifndef SetupBaseName
#define SetupBaseName "lynx"
#endif

#ifndef BinsSrcDir
#define BinsSrcDir GetEnv("LYNX_BINDIR")
#if BinsSrcDir == ""
#define BinsSrcDir "..\bin"
#endif
#endif

#ifndef DllsSrcDir
#define DllsSrcDir GetEnv("LYNX_DLLSDIR")
#if DllsSrcDir == ""
#define DllsSrcDir "..\dlls"
#endif
#endif

#ifndef DocsSrcDir
#define DocsSrcDir GetEnv("LYNX_DOCSDIR")
#if DocsSrcDir == ""
#define DocsSrcDir "..\docs"
#endif
#endif

#ifndef HelpSrcDir
#define HelpSrcDir GetEnv("LYNX_HELPDIR")
#if HelpSrcDir == ""
#define HelpSrcDir "..\lynx_help"
#endif
#endif

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppVerName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=..\COPYHEADER
InfoBeforeFile=..\README
OutputDir=..\PACKAGE\lynx-setup
#emit 'OutputBaseFilename=' + SetupBaseName + LYNX_VERSION + '-setup'
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Dirs]
Name: "{app}\doc"
Name: "{app}\doc\samples"
Name: "{app}\doc\test"
Name: "{app}\help"
Name: "{app}\help\keystrokes"
Name: "{app}\icon"
Name: "{app}\tmp"

[Files]
#emit 'Source: "' + BinsSrcDir + '\' + SourceExeName + '"; DestDir: "{app}"; DestName: ' + MyAppExeName + '; Flags: ignoreversion'
#ifndef NoScreenDll
#emit 'Source: "' + DllsSrcDir + '\' + ScreenDllName + '"; DestDir: "{app}"; DestName: ' + ScreenDllName + '; Flags: ignoreversion'
#endif
#emit 'Source: "' + DocsSrcDir + '\*.*"; DestDir: "{app}\doc"; Flags: '
#emit 'Source: "' + DocsSrcDir + '\samples\*.*"; DestDir: "{app}\doc\samples"; Flags: '
#emit 'Source: "' + DocsSrcDir + '\test\*.*"; DestDir: "{app}\doc\test"; Flags: '
#emit 'Source: "' + HelpSrcDir + '\*.*"; DestDir: "{app}\help"; Flags: '
#emit 'Source: "' + HelpSrcDir + '\keystrokes\*.*"; DestDir: "{app}\help\keystrokes"; Flags: '

; some of these data files are from an earlier installer by Claudio Santambrogio
Source: "..\samples\lynx.ico"; DestDir: "{app}\icon"
Source: "..\samples\lynx.bat"; DestDir: "{app}"
Source: "..\samples\jumps.htm"; DestDir: "{app}"
Source: "..\samples\home.htm"; DestDir: "{app}"
Source: "..\samples\lynx_bookmarks.htm"; DestDir: "{app}"
Source: "..\samples\opaque.lss"; DestDir: "{app}"
Source: "..\samples\lynx.bat"; DestDir: "{app}"
Source: "..\samples\lynx-demo.cfg"; DestDir: "{app}"
Source: "..\lynx.man"; DestDir: "{app}"
Source: "..\lynx.cfg"; DestDir: "{app}"
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: ..\samples\lynx.ico
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; IconFilename: ..\samples\lynx.ico

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall skipifsilent

