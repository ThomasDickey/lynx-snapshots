
#ifndef LYSEARCH_H
#define LYSEARCH_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCT_H */

extern BOOL textsearch PARAMS((document *cur_doc,
			       char *prev_target, BOOL next));

#define IN_FILE 1
#define IN_LINKS 2

#ifndef NOT_FOUND
#define NOT_FOUND 0
#endif /* NOT_FOUND */


#endif /* LYSEARCH_H */
