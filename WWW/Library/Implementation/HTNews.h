/*                                  Network News Transfer protocol module for the WWW library
                                          HTNEWS
                                             
 */
/* History:
**      26 Sep 90       Written TBL in Objective-C
**      29 Nov 91       Downgraded to C, for portable implementation.
*/

#ifndef HTNEWS_H
#define HTNEWS_H

#include "HTAccess.h"
#include "HTAnchor.h"

#ifdef GLOBALREF_IS_MACRO
extern GLOBALREF(HTProtocol, HTNews);
extern GLOBALREF(HTProtocol, HTNNTP);
extern GLOBALREF(HTProtocol, HTSNews);
#else
GLOBALREF HTProtocol HTNews;
GLOBALREF HTProtocol HTNNTP;
GLOBALREF HTProtocol HTSNews;
#endif /* GLOBALREF_IS_MACRO */

extern void HTSetNewsHost PARAMS((CONST char *value));
extern CONST char * HTGetNewsHost NOPARAMS;
extern char * HTNewsHost;

#endif /* HTNEWS_H */


/*

   tbl */
