
#ifndef LYBOOKMARK_H
#define LYBOOKMARK_H

#ifndef LYSTRUCTS_H
#include "LYStructs.h"
#endif /* LYSTRUCTS_H */

extern char * get_bookmark_filename PARAMS((char **name));
extern void save_bookmark_link PARAMS((char *address, char *title));
extern void remove_bookmark_link PARAMS((int cur, char *cur_bookmark_page));
extern int select_multi_bookmarks NOPARAMS;
extern int select_menu_multi_bookmarks NOPARAMS;
extern BOOLEAN LYHaveSubBookmarks NOPARAMS;
extern void LYMBM_statusline PARAMS((char *text));

#define BOOKMARK_TITLE "Bookmark file"
#define MOSAIC_BOOKMARK_TITLE "Converted Mosaic Hotlist"
#define MBM_V_MAXFILES  25	/* Max number of sub-bookmark files */
/*
 *  Arrays that holds the names of sub-bookmark files
 *  and their descriptions.
 */
char  *MBM_A_subbookmark[MBM_V_MAXFILES+1];
char  *MBM_A_subdescript[MBM_V_MAXFILES+1];

#endif /* LYBOOKMARK_H */

