
#ifndef LYCHARUTILS_H
#define LYCHARUTILS_H

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif /* HTUTILS_H */

extern char * LYUnEscapeEntities PARAMS((char *str,
					 BOOLEAN plain_space,
					 BOOLEAN hidden));
extern void LYUnEscapeToLatinOne PARAMS((char **str,
					 BOOLEAN isURL));
extern void LYExpandString PARAMS((char **str));
extern void LYEntify PARAMS((char **str,
			     BOOLEAN isTITLE));
extern void LYTrimHead PARAMS((char *str));
extern void LYTrimTail PARAMS((char *str));
extern char *LYFindEndOfComment PARAMS((char *str));
extern char *LYUppercaseA_OL_String PARAMS((int seqnum));
extern char *LYLowercaseA_OL_String PARAMS((int seqnum));
extern char *LYUppercaseI_OL_String PARAMS((int seqnum));
extern char *LYLowercaseI_OL_String PARAMS((int seqnum));

#endif /* LYCHARUTILS_H */
