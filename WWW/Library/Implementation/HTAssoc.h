/*                       ASSOCIATION LIST FOR STORING NAME-VALUE PAIRS

   Lookups from association list are not case-sensitive.

 */

#ifndef HTASSOC_H
#define HTASSOC_H

#include <HTList.h>

typedef HTList HTAssocList;

typedef struct {
    char * name;
    char * value;
} HTAssoc;


extern HTAssocList *HTAssocList_new (void);
extern void HTAssocList_delete (HTAssocList * alist);

extern void HTAssocList_add (HTAssocList *       alist,
                                    const char *        name,
                                    const char *        value);

extern char *HTAssocList_lookup (HTAssocList *   alist,
                                        const char *    name);

#endif /* not HTASSOC_H */
