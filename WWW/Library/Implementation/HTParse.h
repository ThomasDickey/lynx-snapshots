/*                                                   HTParse:  URL parsing in the WWW Library
                                         HTPARSE
                                             
   This module of the WWW library contains code to parse URLs and various related things.
   Implemented by HTParse.c .
   
 */
#ifndef HTPARSE_H
#define HTPARSE_H

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif /* HTUTILS_H */

/*

   The following are flag bits which may be ORed together to form a number to give the
   'wanted' argument to HTParse.
   
 */
#define PARSE_ACCESS            16
#define PARSE_HOST               8
#define PARSE_PATH               4
#define PARSE_ANCHOR             2
#define PARSE_PUNCTUATION        1
#define PARSE_ALL               31


/*

HTParse:  Parse a URL relative to another URL

   This returns those parts of a name which are given (and requested) substituting bits
   from the related name where necessary.
   
  ON ENTRY
  
  aName                   A filename given
                         
  relatedName             A name relative to which aName is to be parsed
                         
  wanted                  A mask for the bits which are wanted.
                         
  ON EXIT,
  
  returns                 A pointer to a malloc'd string which MUST BE FREED
                         
 */

extern char * HTParse PARAMS((
	CONST char *	aName,
	CONST char *	relatedName,
	int		wanted));


/*

HTStrip: Strip white space off a string

  ON EXIT
  
   Return value points to first non-white character, or to 0 if none.
   
   All trailing white space is OVERWRITTEN with zero.
   
 */
extern char * HTStrip PARAMS((
	char *		s));

/*

HTSimplify: Simplify a UTL

   A URL is allowed to contain the seqeunce xxx/../ which may be replaced by "" , and the
   seqeunce "/./" which may be replaced by "/". Simplification helps us recognize
   duplicate filenames. It doesn't deal with soft links, though. The new (shorter)
   filename overwrites the old.
   
 */
/*
**      Thus,   /etc/junk/../fred       becomes /etc/fred
**              /etc/junk/./fred        becomes /etc/junk/fred
*/
extern void HTSimplify PARAMS((
	char *		filename));


/*

HTRelative:  Make Relative (Partial) URL

   This function creates and returns a string which gives an expression of one address as
   related to another. Where there is no relation, an absolute address is retured.
   
  ON ENTRY,
  
   Both names must be absolute, fully qualified names of nodes (no anchor bits)
   
  ON EXIT,
  
   The return result points to a newly allocated name which, if parsed by HTParse relative
   to relatedName, will yield aName. The caller is responsible for freeing the resulting
   name later.
   
 */
extern char * HTRelative PARAMS((
	CONST char *	aName,
	CONST char *	relatedName));


/*

HTEscape:  Encode unacceptable characters in string

   This funtion takes a string containing any sequence of ASCII characters, and returns a
   malloced string containing the same infromation but with all "unacceptable" characters
   represented in the form %xy where X and Y are two hex digits.
   
 */
extern char * HTEscape PARAMS((
	CONST char *	str,
	unsigned char	mask));

/* Convert space to plus instead of %2b - FM */
extern char * HTEscapeSP PARAMS((
	CONST char *	str,
	unsigned char	mask));

/*

   The following are valid mask values. The terms are the BNF names in the URL document.
   
 */
#define URL_XALPHAS     (unsigned char) 1
#define URL_XPALPHAS    (unsigned char) 2
#define URL_PATH        (unsigned char) 4


/*

HTUnEscape: Decode %xx escaped characters

   This function takes a pointer to a string in which character smay have been encoded in
   %xy form, where xy is the acsii hex code for character 16x+y. The string is converted
   in place, as it will never grow.
   
 */
extern char * HTUnEscape PARAMS((
	char *		str));


#endif  /* HTPARSE_H */


/*

   end of HTParse
   
    */
