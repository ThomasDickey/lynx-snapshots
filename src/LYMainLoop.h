#ifndef LYMAINLOOP_H
#define LYMAINLOOP_H

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

#define TEXTAREA_EXPAND_SIZE  5
#define AUTOGROW
#define AUTOEXTEDIT

extern BOOLEAN LYOpenTraceLog NOPARAMS;
extern int mainloop NOPARAMS;
extern void HTAddGotoURL PARAMS((char *url));
extern void LYCloseTracelog NOPARAMS;
extern void repaint_main_statusline NOPARAMS;

#endif /* LYMAINLOOP_H */
