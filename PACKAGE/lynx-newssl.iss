; $LynxId: lynx-newssl.iss,v 1.1 2017/04/27 00:19:42 tom Exp $
;
; This is an installer for Lynx built with "new" OpenSSL (1.1.x).
;
; The script assumes environment variables have been set, e.g., to point to
; data which is used by the installer (see "lynx.lss" for details).

#define SetupBaseName "lynx-newssl"
#define SourceExeName "lynx-newssl.exe"

#include "lynx.iss"
