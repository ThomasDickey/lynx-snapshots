; $LynxId: lynx-oldssl.iss,v 1.6 2024/01/15 21:35:19 tom Exp $
;
; This is an installer for Lynx built with "old" OpenSSL (before 1.1.x).
;
; The script assumes environment variables have been set, e.g., to point to
; data which is used by the installer (see "lynx.lss" for details).

#define NoScreenDll
#define SslGlob1      "'libssl-1*.dll'"
#define SslGlob2      "'libcrypto-1*.dll'"
#define SetupBaseName "lynx-oldssl"
#define SourceExeName "lynx-oldssl.exe"

#include "lynx.iss"
