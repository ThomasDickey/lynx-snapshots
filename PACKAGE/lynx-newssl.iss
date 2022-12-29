; $LynxId: lynx-newssl.iss,v 1.6 2022/12/29 00:38:07 tom Exp $
;
; This is an installer for Lynx built with "new" OpenSSL (1.1.x).
;
; The script assumes environment variables have been set, e.g., to point to
; data which is used by the installer (see "lynx.lss" for details).

#define NoScreenDll
#define SslGlob1      "'libssl-1*.dll'"
#define SslGlob2      "'libcrypto-1*.dll'"
#define SetupBaseName "lynx-newssl"
#define SourceExeName "lynx-newssl.exe"

#include "lynx.iss"
