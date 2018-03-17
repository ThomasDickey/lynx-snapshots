; $LynxId: lynx-curses.iss,v 1.6 2018/03/14 08:34:56 tom Exp $
;
; This is an installer for the "color-style" flavor of Lynx.
;
; The script assumes environment variables have been set, e.g., to point to
; data which is used by the installer (see "lynx.lss" for details).

#define NoScreenDll
#define SetupBaseName "lynx-cs"
#define SourceExeName "lynx-cs.exe"

#include "lynx.iss"
