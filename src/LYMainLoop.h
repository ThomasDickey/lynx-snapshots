#ifndef LYMAINLOOP_H
#define LYMAINLOOP_H

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

extern BOOLEAN LYOpenTraceLog NOPARAMS;
extern int mainloop NOPARAMS;
extern void HTAddGotoURL PARAMS((char *url));
extern void LYCloseTracelog NOPARAMS;
extern void repaint_main_statusline PARAMS((int for_what));

/* made them available in partial mode */
extern void handle_LYK_TRACE_TOGGLE  NOPARAMS;
extern void handle_LYK_WHEREIS  PARAMS((int cmd, BOOLEAN *refresh_screen));


#endif /* LYMAINLOOP_H */
