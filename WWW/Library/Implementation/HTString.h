/*                                                   String handling for libwww
                                         STRINGS
                                             
   Case-independent string comparison and allocations with copies etc
   
 */
#ifndef HTSTRING_H
#define HTSTRING_H

#include <HTUtils.h>

extern int WWW_TraceFlag;       /* Global flag for all W3 trace */

extern CONST char * HTLibraryVersion;   /* String for help screen etc */

/*

Case-insensitive string comparison

   The usual routines (comp instead of cmp) had some problem.
   
 */
extern int strcasecomp  PARAMS((CONST char *a, CONST char *b));
extern int strncasecomp PARAMS((CONST char *a, CONST char *b, int n));

extern int strcasecomp8  PARAMS((CONST char *a, CONST char *b));
extern int strncasecomp8 PARAMS((CONST char *a, CONST char *b, int n));
       /*
       **  strcasecomp8 and strncasecomp8 are variants of strcasecomp
       **  and strncasecomp, but use 8bit upper/lower case information
       **  from the current display charset
       */


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

/* A more general parser - kw */
extern char * HTNextTok PARAMS((char ** pstr,
		      CONST char * delims, CONST char * bracks, char * found));

#if ANSI_VARARGS
extern char * HTSprintf (char ** pstr, CONST char * fmt, ...)
			GCC_PRINTFLIKE(2,3);
extern char * HTSprintf0 (char ** pstr, CONST char * fmt, ...)
			 GCC_PRINTFLIKE(2,3);
#else
extern char * HTSprintf () GCC_PRINTFLIKE(2,3);
extern char * HTSprintf0 () GCC_PRINTFLIKE(2,3);
#endif

#if defined(VMS) || defined(DOSPATH) || defined(__EMX__)
#define USE_QUOTED_PARAMETER 0
#else
#define USE_QUOTED_PARAMETER 1
#endif

#if USE_QUOTED_PARAMETER
extern char *HTQuoteParameter PARAMS((CONST char *parameter));
#else
#define HTQuoteParameter(parameter) parameter	/* simplify ifdef'ing */
#endif

extern int HTCountCommandArgs PARAMS((CONST char * command));
extern void HTAddParam PARAMS((char ** result, CONST char * command, int number, CONST char * parameter));
extern void HTEndParam PARAMS((char ** result, CONST char * command, int number));

#endif /* HTSTRING_H */
