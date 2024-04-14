; $LynxId: lynx.iss,v 1.42 2024/04/14 23:28:50 tom Exp $
; vile:ts=4 sw=4 notabinsert fk=8bit
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

#if Ver < 0x5060100
#define MySendTo '{sendto}\' + myAppName + '.lnk'
#else
#define MySendTo '{usersendto}\' + myAppName + '.lnk'
#endif
#define MyQuickLaunch '{userappdata}\Microsoft\Internet Explorer\Quick Launch\' + myAppName + '.lnk'

#ifndef SourceExeName
#define SourceExeName "lynx-default.exe"
#endif

#ifndef NoScreenDll
#ifndef ScreenDllName
#define ScreenDllName "pdcurses.dll"
#endif
#endif

#ifndef BzipDllName
#define BzipDllName "libbz2.dll"
#endif

#ifndef ZlibDllName
#define ZlibDllName "zlib1.dll"
#endif

#ifndef BzipExeName
#define BzipExeName "bzip2.exe"
#endif

#ifndef GzipExeName
#define GzipExeName "gzip.exe"
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
#define DllsSrcDir "..\bin"
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
#emit 'AppVersion=' + LYNX_VERSION
#emit 'VersionInfoDescription=Setup for "' + MyAppName + '"'
#emit 'VersionInfoVersion=' + LYNX_VERSION
AppVerName={#MyAppVerName}
AppPublisher={#MyAppPublisher}
AppCopyright=© 1997-2023,2024, Thomas E. Dickey
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={commonpf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=..\COPYHEADER
InfoBeforeFile=..\README
OutputDir=..\PACKAGE\lynx-setup
#emit 'OutputBaseFilename=' + SetupBaseName + LYNX_VERSION + '-setup'
Compression=lzma
SolidCompression=yes
PrivilegesRequired=none
SetupLogging=no

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

[Files]
#emit 'Source: "' + BinsSrcDir + '\' + SourceExeName + '"; DestDir: "{app}"; DestName: ' + MyAppExeName + '; AfterInstall: myPostExecutable; Flags: ignoreversion'
#ifndef NoScreenDll
#emit 'Source: "' + DllsSrcDir + '\' + ScreenDllName + '"; DestDir: "{app}"; DestName: ' + ScreenDllName + '; Flags: ignoreversion'
#endif
#emit 'Source: "' + DllsSrcDir + '\' + BzipExeName + '"; DestDir: "{app}"; DestName: ' + BzipExeName + '; Flags: ignoreversion'
#emit 'Source: "' + DllsSrcDir + '\' + GzipExeName + '"; DestDir: "{app}"; DestName: ' + GzipExeName + '; Flags: ignoreversion'
#emit 'Source: "' + DocsSrcDir + '\*.*"; DestDir: "{app}\doc"; Flags: '
#emit 'Source: "' + DocsSrcDir + '\samples\*.*"; DestDir: "{app}\doc\samples"; Flags: '
#emit 'Source: "' + DocsSrcDir + '\test\*.*"; DestDir: "{app}\doc\test"; Flags: '
#emit 'Source: "' + HelpSrcDir + '\*.*"; DestDir: "{app}\help"; Flags: '
#emit 'Source: "' + HelpSrcDir + '\keystrokes\*.*"; DestDir: "{app}\help\keystrokes"; Flags: '

; some of these data files are from an earlier installer by Claudio Santambrogio
Source: "..\lynx.cfg"; DestDir: "{app}" ; AfterInstall: myCustomCfg; Flags: ignoreversion
Source: "..\lynx.man"; DestDir: "{app}"
Source: "..\samples\*.lss"; DestDir: "{app}"
Source: "..\samples\home.htm"; DestDir: "{app}"
Source: "..\samples\jumps.htm"; DestDir: "{app}"
Source: "..\samples\lynx-demo.cfg"; DestDir: "{app}"
Source: "..\samples\lynx.bat"; DestDir: "{app}"
Source: "..\samples\lynx.ico"; DestDir: "{app}\icon"
Source: "..\samples\lynx_bookmarks.htm"; DestDir: "{app}"
Source: "..\samples\oldlynx.bat"; DestDir: "{app}"
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: {app}\.lynx_cookies
Type: dirifempty; Name: {app}
#emit 'Type: files; Name: ' + mySendTo
#emit 'Type: files; Name: ' + myQuickLaunch

[Code]
#emit 'const MY_APP_NAME = ''{app}\' + myAppProg + '.exe'';'


function isGuru(): Boolean;
begin
    Result := isAdmin();
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
    if WizardisTaskSelected('for_all_users') then
        Result := HKEY_LOCAL_MACHINE
    else
        Result := HKEY_CURRENT_USER;
end;

function selectedVarsSubKey(): String;
begin
    if WizardisTaskSelected('for_all_users') then
    begin
        if WizardisTaskSelected('register_vars') then
            Result := appSubKey()
        else
            Result := envSysKey();
    end else
    begin
        if WizardisTaskSelected('register_vars') then
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

procedure AddQuickLaunch();
begin
  CreateShellLink(
#emit 'ExpandConstant(''' + MyQuickLaunch + '''),'
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

  if WizardisTaskSelected('use_sendto') then
    begin
    AddSendTo();
    Log('** added Send-To link');
    end;

  if WizardisTaskSelected('quicklaunchicon') then
    begin
    AddQuickLaunch();
    Log('** added Quick-launch link');
    end;
end;

// This is called after installing the lynx.cfg file.
procedure myCustomCfg();
var
  AppDir  : String;
  CfgFile : String;
begin
  AppDir := ExpandConstant('{app}');
  CfgFile := AppDir + '\lynx.cfg';

  addAnyVariable('LYNX_CFG', CfgFile);
  Log('** set LYNX_CFG=' + CfgFile);

  SaveStringToFile(CfgFile, 'HELPFILE:' + AppDir + '/help/Lynx_help_main.html.gz' + #10, True);
  SaveStringToFile(CfgFile, 'COLOR_STYLE:' + AppDir + '/lynx.lss' + #10, True);

  SaveStringToFile(CfgFile, 'CHMOD_PATH:' + #10, True);
  SaveStringToFile(CfgFile, 'COPY_PATH:' + #10, True);
  SaveStringToFile(CfgFile, 'MKDIR_PATH:' + #10, True);
  SaveStringToFile(CfgFile, 'MV_PATH:' + #10, True);
  SaveStringToFile(CfgFile, 'RMDIR_PATH:' + #10, True);
  SaveStringToFile(CfgFile, 'RM_PATH:' + #10, True);
  SaveStringToFile(CfgFile, 'TOUCH_PATH:' + #10, True);

  Log('** customized ' + CfgFile);

  if WizardisTaskSelected('use_sendto') then
    begin
    AddSendTo();
    Log('** added Send-To link');
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

#ifdef SslGlob1
#emit "const SslGlob1 = " + SslGlob1 + ";"
#emit "const SslGlob2 = " + SslGlob2 + ";"
var
    SslDirPage     : TInputFileWizardPage;
    SslLibraryPath : String;

function DirContains(const PathList: string; const PathItem: string): Boolean;
var
    myList : string;
    myItem : string;
    myPart : string;
    offset : integer;
begin
    Result := False;
    myList := Uppercase(PathList);
    myItem := Uppercase(RemoveBackslashUnlessRoot(PathItem));
    offset := Pos(myItem, myList);
    if offset <> 0 then
        begin
        myPart := Copy( myList, offset + Length(myItem), Length(myList) );
        if ( Length(myPart) = 0 ) or ( Pos(';', myPart) = 1 ) or ( Pos('\;', myPart) = 1 ) then
            begin
            Result := True;
            end;
        end;
end;

function PathCat(const head, tail: string): string;
begin
    Result := RemoveBackslashUnlessRoot(head) + '\' + tail;
end;

procedure CopyFromTo(const source, target, name: string);
var
    FailExists : Boolean;
    Status:      Boolean;
begin
    Log('Copy from: ' + PathCat(source, name));
    Log('Copy   to: ' + PathCat(target, name));
    FailExists := False;
    Status := FileCopy(PathCat(source, name), PathCat(target, name), FailExists);
    if not Status then
        begin
        MsgBox('Failed to copy ' + name, mbError, MB_OK);
        Abort();
        end;
end;

procedure ReallyDelete(const fullPath: string);
begin
    if FileExists( fullpath ) then
        begin
        if DeleteFile( fullPath ) then
            Log( '...successful' )
        else
            begin
            MsgBox('Failed to delete ' + fullPath, mbError, MB_OK);
            end;
        end;
end;

procedure DeleteAppFile(const name: string);
var
    AppDir   : string;
    fullPath : string;
    findRec  : TFindRec;
begin
    AppDir := ExpandConstant('{app}');
    fullPath := PathCat( AppDir, name );
    Log( 'Deleting ' + fullPath );
    if Pos('*', fullPath) <> 0 then
        begin
        if FindFirst(fullPath, findRec) then
            begin
            ReallyDelete( PathCat( AppDir, findRec.name ) );
            while FindNext( findRec ) do
                ReallyDelete( PathCat( AppDir, findRec.name ) );
            end
        end
    else
        begin
        ReallyDelete( fullPath );
        end;
end;

function checkSslDir(): Boolean;
begin
    Result := True;
    SslLibraryPath := Trim( SslDirPage.Values[0] );
    if ( Length( SslLibraryPath ) = 0 ) or ( not FileExists( SslLibraryPath ) ) then
        begin
        MsgBox('No SSL library found', mbError, MB_OK)
        Result := False;
        end;
end;

procedure copySslDlls();
var
    SslDirectory   : String;
    SslFilename    : String;
    TargetDir      : String;
begin
    Log('Copying SSL DLLs');
    SslDirectory := ExtractFilePath(SslLibraryPath);

    // If the directory is not already in the PATH, copy the DLLs to
    // the application directory.
    if DirContains( GetEnv('PATH'), SslDirectory) then
        Log( 'PATH contains SSL directory' )
    else
        begin
        TargetDir := ExpandConstant('{app}');
        CreateDir(TargetDir);
        if DirExists(TargetDir) then
            begin
            SslFilename := Lowercase( ExtractFileName( SslLibraryPath ) );
            CopyFromTo( SslDirectory, TargetDir, SslFilename );
            Log( 'comparing: ' + SslFilename + ' to ' + SslGlob1 );
            if CompareText( SslFilename, SslGlob1 ) = 0 then
                // old-ssl is literal, new is a glob...
                CopyFromTo( SslDirectory, TargetDir, SslGlob2 )
            else
                // new-ssl matches "libssl-x-x-z", s/libssl/libcrypto/
                SslFilename := 'libcrypto' + Copy(SslFilename, 7, Length(SslFilename));
                CopyFromTo( SslDirectory, TargetDir, SslFilename );
#ifdef RuntimeBundle
            // older releases of OpenSSL bundled the Visual Studio runtime
            SslFilename := SslDirectory + '\' + 'msvcr120.dll' ;
            if FileExists(SslFilename) then
                begin
                CopyFromTo( SslDirectory, TargetDir, 'msvcr120.dll' );
                end
#endif
            end
        else
            begin
            MsgBox( 'Cannot create application directory', mbError, MB_OK )
            Abort();
            end
        end;
    Log('done - Copying SSL DLLs');
end;

procedure RegisterPreviousData(PreviousDataKey: Integer);
begin
    SetPreviousData( PreviousDataKey, 'SSL PATH', SslLibraryPath );
end;

function NextButtonClick(CurPageId: integer): Boolean;
begin
    Result := True;
    if CurPageId = SslDirPage.Id then
        begin
        Result := checkSslDir();
        end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
    if CurStep = ssInstall then
    begin
        CopySslDlls();
    end;
end;

procedure InitializeWizard();
var
    myGlob: string;
begin
    // Create a page to locate the SSL library
    SslDirPage := CreateInputFilePage(wpSelectDir,
        'Select SSL Library Location',
        'Where is the SSL library located?',
        'Select it from the directory, then click Next.');

    myGlob := 'SSL dll|' + SslGlob1;
    Log( 'search for SSL libraries ' + myGlob );
    SslDirPage.Add( 'Locate SSL library:', myGlob, '*.dll' );
    SslDirPage.Values[0] := GetPreviousData( 'SSL PATH', '' );
end;
#endif

// On uninstall, we do not know which registry setting was selected during install, so we remove all.
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  case CurUninstallStep of
    usUninstall:
      begin
      Log('CurUninstallStepChanged:' + 'Uninstall is about to start.')
        // ...insert code to perform pre-uninstall tasks here...
#ifdef SslGlob1
      DeleteAppFile( SslGlob1 );
      DeleteAppFile( SslGlob2 );
      DeleteAppFile( 'msvcr120.dll' );
#endif
      end;
    usPostUninstall:
      begin
        removeAnyVariable('LYNX_CFG');
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

        Log('CurUninstallStepChanged:' + 'Uninstall just finished.');
      end;
  end;
end;
