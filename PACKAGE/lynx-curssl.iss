; $LynxId: lynx-curssl.iss,v 1.1 2022/12/29 00:38:46 tom Exp $
;
; This is an installer for Lynx built with "current" OpenSSL (3.0.x).
;
; The script assumes environment variables have been set, e.g., to point to
; data which is used by the installer (see "lynx.lss" for details).

#define NoScreenDll
#define SslGlob1      "'libssl-3*.dll'"
#define SslGlob2      "'libcrypto-3*.dll'"
#define SetupBaseName "lynx-curssl"
#define SourceExeName "lynx-curssl.exe"

#include "lynx.iss"
