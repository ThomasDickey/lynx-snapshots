/*
 * $LynxId: HTInit.h,v 1.11 2025/01/06 15:13:11 tom Exp $
 *
 * /Net/dxcern/userd/timbl/hypertext/WWW/Library/Implementation/HTInit.html
 *                                INITIALISATION MODULE
 *
 * This module registers all the plug & play software modules which will be
 * used in the program.  This is for a browser.
 *
 * To override this, just copy it and link in your version before you link with
 * the library.
 *
 * Implemented by HTInit.c by default.
 */

#ifndef HTINIT_H
#define HTINIT_H 1

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
    /* *INDENT-OFF* */
    extern void HTFormatInit(void);
    extern void HTPreparsedFormatInit(void);
    extern void HTFileInit(void);
    extern int LYTestMailcapCommand(const char *testcommand, const char *content_type, const char *params);
    extern BOOL LYMailcapUsesPctS(const char *controlstring);
    extern char *LYMakeMailcapCommand(const char *command, const char *content_type, const char *params, const char *filename);
    /* *INDENT-ON* */

#ifdef __cplusplus
}
#endif
#endif				/* HTINIT_H */
