; $LynxId: lynx-default.iss,v 1.1 2018/03/13 09:10:08 tom Exp $
;
; This is an installer for the "default" flavor of Lynx.
;
; The script assumes environment variables have been set, e.g., to point to
; data which is used by the installer (see "lynx.lss" for details).

#define NoScreenDll
#define SourceExeName "lynx-default.exe"

#include "lynx.iss"
