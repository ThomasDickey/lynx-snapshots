/* $LynxId: LYNews.h,v 1.11 2026/10/12 12:52:31 tom Exp $ */
#ifndef LYNEWSPOST_H
#define LYNEWSPOST_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCTS_H */

#ifdef __cplusplus
extern "C" {
#endif
    extern BOOLEAN term_message;

    extern char *LYNewsPost(const char *newsgroups, int followup);

#ifdef __cplusplus
}
#endif
#endif				/* LYNEWSPOST_H */
