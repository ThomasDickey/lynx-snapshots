#ifndef LYJUMP_H
#define LYJUMP_H

#include <HTList.h>

typedef struct _JumpDatum {
    char *key;
    char *url;
} JumpDatum;

struct JumpTable { 
    int key;
    int nel;
    char *msg;
    char *file;
    char *shortcut;
    HTList *history;
    JumpDatum *table;
    struct JumpTable *next;
    char *mp;
};

extern struct JumpTable *JThead;
extern void LYJumpTable_free (void);
extern void LYAddJumpShortcut (HTList *the_history, char *shortcut);
extern BOOL LYJumpInit (char *config);
extern char *LYJump (int key);

#endif /* LYJUMP_H */
