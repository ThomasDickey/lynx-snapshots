; $LynxId: lynx.iss,v 1.2 2009/03/17 00:25:56 tom Exp $
; vile:ts=2 sw=2 notabinsert
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

#define MySendTo '{sendto}\' + myAppName + '.lnk'

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

[Components]
Name: main; Description: The Lynx executable; types: full custom compact
Name: explorer; Description: Windows Explorer integration; types: full custom

[Tasks]
Name: for_all_users; Description: Install for all users on this machine; GroupDescription: Configuration Settings; Components: main; Check: isGuru; Flags: unchecked
Name: register_vars; Description: Use registry for environment variables; GroupDescription: Configuration Settings; Components: main; Flags: unchecked
Name: use_sendto; Description: Add Send To Entry; GroupDescription: Windows Explorer; Components: explorer; Flags: unchecked
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Components: main; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Components: main; Flags: unchecked


[Dirs]
Name: "{app}\doc"
Name: "{app}\doc\samples"
Name: "{app}\doc\test"
Name: "{app}\help"
Name: "{app}\help\keystrokes"
Name: "{app}\icon"
Name: "{app}\tmp"

[Files]
#emit 'Source: "' + BinsSrcDir + '\' + SourceExeName + '"; DestDir: "{app}"; DestName: ' + MyAppExeName + '; AfterInstall: myPostExecutable; Flags: ignoreversion'
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

[UninstallDelete]
Type: files; Name: {app}\.lynx_cookies
Type: dirifempty; Name: {app}
#emit 'Type: files; Name: ' + mySendTo

[Code]
#emit 'const MY_APP_NAME = ''{app}\' + myAppName + '.exe'';'

function isGuru(): Boolean;
begin
    Result := isAdminLoggedOn();
end;

function environRootKey(): Integer;
begin
    Result := HKEY_CURRENT_USER;
end;

function appKey(): string;
begin
    Result := 'Software\Lynx';
end;

function envSubKey(): string;
begin
    Result := 'Environment';
end;

function appSubKey(): string;
begin
    Result := appKey() + '\' + envSubKey();
end;

function envSysKey(): string;
begin
    Result := 'System\CurrentControlSet\Control\Session Manager\Environment';
end;

// Set the environment variable ValueName.
procedure addVarToEnv(const RootKey: Integer; const SubKeyName, ValueName, toAdd: String);
var
    Updated : string;
begin
    Updated := ExpandConstant(toAdd);
    RegWriteStringValue(RootKey, SubKeyName, ValueName, Updated);
    Log('Added ' + toAdd + ' to ' + ValueName);
    // MsgBox('addVarToEnv: ' #13#13 + ValueName + '="' + Updated + '"', mbInformation, MB_OK)
end;

// Remove the given environment variable ValueName.
function removeVarFromEnv(const RootKey: Integer; const SubKeyName, ValueName: String): Boolean;
var
    Current : string;
begin
    Result := False;
    if RegQueryStringValue(RootKey, SubKeyName, ValueName, Current) then
    begin
        RegDeleteValue(RootKey, SubKeyName, ValueName);
        Result := True;
        Log('Removed ' + ValueName);
        // MsgBox('removeVarFromEnv: ' #13#13 + ValueName + '="' + Current + '"', mbInformation, MB_OK)
    end;
end;

function selectedVarsRootKey(): Integer;
begin
    if isTaskSelected('for_all_users') then
        Result := HKEY_LOCAL_MACHINE
    else
        Result := HKEY_CURRENT_USER;
end;

function selectedVarsSubKey(): String;
begin
    if isTaskSelected('for_all_users') then
    begin
        if isTaskSelected('register_vars') then
            Result := appSubKey()
        else
            Result := envSysKey();
    end else
    begin
        if isTaskSelected('register_vars') then
            Result := appSubKey()
        else
            Result := envSubKey();
    end;
end;

procedure addAnyVariable(const ValueName, newValue: String);
begin
    addVarToEnv(selectedVarsRootKey(), selectedVarsSubKey(), ValueName, NewValue);
end;

// FIXME: should only remove if it matches the installer's value
procedure removeAnyVariable(const ValueName: String);
begin
    removeVarFromEnv(HKEY_CURRENT_USER, envSubKey(), ValueName);
    removeVarFromEnv(HKEY_CURRENT_USER, appSubKey(), ValueName);
    removeVarFromEnv(HKEY_LOCAL_MACHINE, appSubKey(), ValueName);
    removeVarFromEnv(HKEY_LOCAL_MACHINE, envSysKey(), ValueName);
end;

// http://www.delphidabbler.com/articles?article=12
procedure AddSendTo();
begin
  CreateShellLink(
#emit 'ExpandConstant(''' + MySendTo + '''),'
#emit '''SendTo link for ' + myAppName + ''','
    ExpandConstant(MY_APP_NAME),      // program
    '',                               // option(s) will be followed by pathname
    '',                               // no target directory
    '',                               // no icon filename
    -1,                               // no icon index
    SW_SHOWNORMAL);
end;

// This is called after installing the executable.
procedure myPostExecutable();
var
  Keypath : String;
  AppDir  : String;
begin
  Keypath := appKey();
  AppDir := ExpandConstant('{app}');
  Log('Setting registry key "' + Keypath + '" to "' + AppDir + '"');
  if not RegWriteStringValue(selectedVarsRootKey(), Keypath, '', AppDir) then
    Log('Failed to set key');

  addAnyVariable('LYNX_CFG', AppDir + '/lynx.cfg');
  addAnyVariable('LYNX_LSS', AppDir + '/opaque.lss');
  addAnyVariable('LYNX_HELPFILE', AppDir + '/help/Lynx_users_guide.html.gz');

  if isTaskSelected('use_sendto') then
    begin
    AddSendTo();
    end;
end;

function CleanupMyKey(const theRootKey: Integer): Boolean;
var
  Path : String;
  Value : String;
begin
  Result := False;
  if RegQueryStringValue(theRootKey, appKey(), '', Value) then
    begin
      if Value <> '' then
        begin
        Result := True;
        Log('Deleting value of "' + appKey() + '" = "' + Value + '"');
        if not RegDeleteValue(theRootKey, appKey(), '') then
          Log('Failed to delete value');

        Path := appKey() + '\Environment';
        Log('Checking for subkey "' + Path + '"');
        if RegValueExists(theRootKey, Path, '') then
          begin
          if RegDeleteKeyIncludingSubkeys(theRootKey, Path) then
            Log('Deleted key "' + Path + '"')
          else
            Log('Failed to delete key "' + Path + '"');
          end;

        if RegDeleteKeyIfEmpty(theRootKey, appKey()) then
          Log('Deleted key "' + appKey() + '"')
        else
          Log('Failed to delete key "' + appKey() + '"');
        end
    end
end;
      
// On uninstall, we do not know which registry setting was selected during install, so we remove all.
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  case CurUninstallStep of
    usUninstall:
      begin
        // MsgBox('CurUninstallStepChanged:' #13#13 'Uninstall is about to start.', mbInformation, MB_OK)
        // ...insert code to perform pre-uninstall tasks here...
      end;
    usPostUninstall:
      begin
        removeAnyVariable('LYNX_CFG');
        removeAnyVariable('LYNX_LSS');
        removeAnyVariable('LYNX_HELPFILE');

      {
        If we don't find the settings in the current user, try the local machine.
        The setup program cannot pass the all-users flag to the uninstaller, so we
        have to try both.
      }
      Log('Checking current-user registry key');
      if not CleanupMyKey(HKEY_CURRENT_USER) then
        begin
        Log('Checking local-machine registry key');
        CleanupMyKey(HKEY_LOCAL_MACHINE);
        end;
    
        // MsgBox('CurUninstallStepChanged:' #13#13 'Uninstall just finished.', mbInformation, MB_OK);
      end;
  end;
end;
