#ifndef LYMAINLOOP_H
#define LYMAINLOOP_H

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

#ifdef DISP_PARTIAL
extern BOOL LYMainLoop_pageDisplay(int line_num);
#endif
extern BOOLEAN LYOpenTraceLog(void);
extern char *LYDownLoadAddress(void);
extern int LYGetNewline(void);
extern int mainloop(void);
extern void HTAddGotoURL(char *url);
extern void LYCloseTracelog(void);
extern void LYSetNewline(int value);
extern void handle_LYK_TRACE_TOGGLE(void);
extern void handle_LYK_WHEREIS(int cmd, BOOLEAN *refresh_screen);
extern void repaint_main_statusline(int for_what);

#ifdef SUPPORT_CHDIR
extern void handle_LYK_CHDIR(void);
#endif

#endif /* LYMAINLOOP_H */
