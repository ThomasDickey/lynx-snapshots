/*                                                                 String handling for libwww
                                         STRINGS
                                             
   Case-independent string comparison and allocations with copies etc
   
 */
#ifndef HTSTRING_H
#define HTSTRING_H

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif /* HTUTILS_H */

extern int WWW_TraceFlag;       /* Global flag for all W3 trace */

extern CONST char * HTLibraryVersion;   /* String for help screen etc */

/*

Case-insensitive string comparison

   The usual routines (comp instead of cmp) had some problem.
   
 */
extern int strcasecomp  PARAMS((CONST char *a, CONST char *b));
extern int strncasecomp PARAMS((CONST char *a, CONST char *b, int n));

/*

Malloced string manipulation

 */
#define StrAllocCopy(dest, src) HTSACopy (&(dest), src)
#define StrAllocCat(dest, src)  HTSACat  (&(dest), src)
extern char * HTSACopy PARAMS ((char **dest, CONST char *src));
extern char * HTSACat  PARAMS ((char **dest, CONST char *src));

/*

Next word or quoted string

 */
extern char * HTNextField PARAMS ((char** pstr));


#endif
/*

   end
   
    */
