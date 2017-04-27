; $LynxId: lynx-oldssl.iss,v 1.1 2017/04/27 00:20:10 tom Exp $
;
; This is an installer for Lynx built with "old" OpenSSL (before 1.1.x).
;
; The script assumes environment variables have been set, e.g., to point to
; data which is used by the installer (see "lynx.lss" for details).

#define SetupBaseName "lynx-oldssl"
#define SourceExeName "lynx-oldssl.exe"

#include "lynx.iss"
