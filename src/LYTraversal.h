/*	traversal.c function declarations
*/
#ifndef TRAVERSAL_H
#define TRAVERSAL_H

#ifndef HTUTILS_H
#include <HTUtils.h>            /* BOOL, ARGS */
#endif

extern BOOLEAN lookup (char * target);
extern void add_to_table (char * target);
extern void add_to_traverse_list (char * fname, char * prev_link_name);
extern void dump_traversal_history (void);
extern void add_to_reject_list (char * target);
extern BOOLEAN lookup_reject (char * target);

#endif /* TRAVERSAL_H */
