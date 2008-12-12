; $LynxId: lynx-curses.iss,v 1.4 2008/09/24 22:43:18 tom Exp $
;
; This script assumes environment variables have been set, e.g., to point to
; data which is used by the installer:
;
; LYNX_DOCSDIR - files and subdirectories installed from Unix with "make install-doc"
; LYNX_HELPDIR - files and subdirectories installed from Unix with "make install-help"
;
; TODO: install curses or slang dll with executable

#define MyAppName "Lynx"
#define MyAppVerName "Lynx 2.8.7dev.10"
#define MyAppPublisher "Thomas E Dickey"
#define MyAppURL "http://lynx.isc.org"
#define MyAppExeName "lynx.exe"

#define BinsSrcDir GetEnv("LYNX_BINDIR")
#if BinsSrcDir == ""
#define BinsSrcDir "..\bin"
#endif

#define DocsSrcDir GetEnv("LYNX_DOCSDIR")
#if DocsSrcDir == ""
#define DocsSrcDir "..\docs"
#endif

#define HelpSrcDir GetEnv("LYNX_HELPDIR")
#if HelpSrcDir == ""
#define HelpSrcDir "..\lynx_help"
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
OutputBaseFilename=lynx287dev-setup
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
#emit 'Source: "' + BinsSrcDir + '\lynx-cs.exe"; DestDir: "{app}"; DestName: lynx.exe; Flags: ignoreversion'
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

