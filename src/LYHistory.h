
#ifndef LYHISTORY_H
#define LYHISTORY_H

#ifndef LYSTRUCTS_H
#include "LYStructs.h"
#endif /* LYSTRUCTS_H */

extern void LYpush PARAMS((document *doc));
extern void LYpop PARAMS((document *doc));
extern void LYpop_num PARAMS((int number, document *doc));
extern int showhistory PARAMS((char **newfile));
extern void historytarget PARAMS((document *newdoc));

#define HISTORY_PAGE_TITLE  "Lynx History Page"

#endif /* LYHISTORY_H */
