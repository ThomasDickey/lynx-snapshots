#ifndef LYLOCAL_H
#define LYLOCAL_H

#ifdef DIRED_SUPPORT

#include <HTUtils.h>

/* Special return code for LYMainLoop.c */
#define PERMIT_FORM_RESULT (-99)

extern int local_create (DocInfo *doc);
extern int local_modify (DocInfo *doc, char **newpath);
extern int local_remove (DocInfo *doc);
#ifdef OK_INSTALL
extern BOOLEAN local_install (char *destpath, char *srcpath, char **newpath);
#endif

/* MainLoop needs to know about this one for atexit cleanup */
extern void clear_tags (void);

extern int dired_options (DocInfo *doc, char ** newfile);
extern int local_dired (DocInfo *doc);
extern void add_menu_item (char *str);
extern void reset_dired_menu (void);
extern void showtags (HTList *tag);
extern void tagflag (int flag, int cur); 

#endif /* DIRED_SUPPORT */

#endif /* LYLOCAL_H */
