#ifndef LYNEWSPOST_H
#define LYNEWSPOST_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCTS_H */

extern BOOLEAN term_message;

extern char *LYNewsPost(char *newsgroups, BOOLEAN followup);

#endif /* LYNEWSPOST_H */
