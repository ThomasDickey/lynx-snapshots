#ifndef LYHISTORY_H
#define LYHISTORY_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCTS_H */

extern BOOLEAN LYwouldPush PARAMS((char *title));
extern BOOLEAN historytarget PARAMS((document *newdoc));
extern int LYShowVisitedLinks PARAMS((char **newfile));
extern int showhistory PARAMS((char **newfile));
extern void LYAddVisitedLink PARAMS((document *doc));
extern void LYpop PARAMS((document *doc));
extern void LYpop_num PARAMS((int number, document *doc));
extern void LYpush PARAMS((document *doc, BOOLEAN force_push));

#endif /* LYHISTORY_H */
