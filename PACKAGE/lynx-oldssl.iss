; $LynxId: lynx-oldssl.iss,v 1.5 2022/12/28 22:37:15 tom Exp $
;
; This is an installer for Lynx built with "old" OpenSSL (before 1.1.x).
;
; The script assumes environment variables have been set, e.g., to point to
; data which is used by the installer (see "lynx.lss" for details).

#define NoScreenDll
#define SslGlob1      "'ssleay32.dll'"
#define SslGlob2      "'libeay32.dll'"
#define SetupBaseName "lynx-oldssl"
#define SourceExeName "lynx-oldssl.exe"
#define RuntimeBundle "'msvcr120.dll'"

#include "lynx.iss"
