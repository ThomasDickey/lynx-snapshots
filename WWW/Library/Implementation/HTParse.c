/*		Parse HyperText Document Address		HTParse.c
**		================================
*/

#include "HTUtils.h"
#include "tcp.h"
#include "HTParse.h"

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

#define HEX_ESCAPE '%'

struct struct_parts {
	char * access;
	char * host;
	char * absolute;
	char * relative;
/*	char * search;		no - treated as part of path */
	char * anchor;
};


/*	Strip white space off a string
**	------------------------------
**
** On exit,
**	Return value points to first non-white character, or to 0 if none.
**	All trailing white space is OVERWRITTEN with zero.
*/

PUBLIC char * HTStrip ARGS1(
	char *,		s)
{
#define SPACE(c) ((c==' ')||(c=='\t')||(c=='\n')) 
    char * p = s;
    for (p = s; *p; p++)
        ;		        /* Find end of string */
    for (p--; p >= s; p--) {
    	if (SPACE(*p))
	    *p = '\0';		/* Zap trailing blanks */
	else
	    break;
    }
    while (SPACE(*s))
        s++;			/* Strip leading blanks */
    return s;
}


/*	Scan a filename for its consituents
**	-----------------------------------
**
** On entry,
**	name	points to a document name which may be incomplete.
** On exit,
**      absolute or relative may be nonzero (but not both).
**	host, anchor and access may be nonzero if they were specified.
**	Any which are nonzero point to zero terminated strings.
*/
PRIVATE void scan ARGS2(
	char *,			name,
	struct struct_parts *,	parts)
{
    char * after_access;
    char * p;
    int length = strlen(name);
    
    parts->access = NULL;
    parts->host = NULL;
    parts->absolute = NULL;
    parts->relative = NULL;
    parts->anchor = NULL;
    
    after_access = name;
    for (p = name; *p; p++) {
	if (*p==':') {
	    *p = '\0';
	    parts->access = name;	/* Access name has been specified */
	    after_access = p+1;
	    break;
	}
	if (*p=='/')
	    break;
	if (*p=='#')
	    break;
    }
    
    for (p = (name+length-1); p >= name; p--) {
	if (*p =='#') {
	    parts->anchor = p + 1;
	    *p = '\0';			/* terminate the rest */
	}
    }
    p = after_access;
    if (*p == '/') {
	if (p[1] == '/') {
	    parts->host = p+2;		 /* host has been specified 	*/
	    *p = '\0';			 /* Terminate access 		*/
	    p = strchr(parts->host,'/'); /* look for end of host name if any */
	    if (p) {
	        *p = '\0';			/* Terminate host */
	        parts->absolute = p+1;		/* Root has been found */
	    }
	} else {
	    parts->absolute = p+1;		/* Root found but no host */
	}	    
    } else {
        parts->relative = (*after_access) ? after_access : 0;	/* zero for "" */
    }

    if (parts->access && parts->anchor) {
        if ((!parts->host && strcasecomp(parts->access, "lynxcgi")) ||
	    !strcasecomp(parts->access, "nntp") ||
	    !strcasecomp(parts->access, "snews") ||
	    !strcasecomp(parts->access, "news") ||
	    !strcasecomp(parts->access, "data")) {
	    /* 
	     *  Access specified but no host and not a lynxcgi URL, so the
	     *  anchor may not really be one, e.g., news:j462#36487@foo.bar,
	     *  or it's an nntp or snews URL, or news URL with a host.
	     *  Restore the '#' in the address.
	     */
	    *(parts->anchor - 1) = '#';
	    parts->anchor = NULL;
	}
    }

#ifdef NOT_DEFINED	/* search is just treated as part of path */
    {
        char *p = relative ? relative : absolute;
	if (p) {
	    char * q = strchr(p, '?');	/* Any search string? */
	    if (q) {
	    	*q = '\0';			/* If so, chop that off. */
		parts->search = q+1;
	    }
	}
    }
#endif /* NOT_DEFINED */
} /*scan */    


/*	Parse a Name relative to another name
**	-------------------------------------
**
**	This returns those parts of a name which are given (and requested)
**	substituting bits from the related name where necessary.
**
** On entry,
**	aName		A filename given
**      relatedName     A name relative to which aName is to be parsed
**      wanted          A mask for the bits which are wanted.
**
** On exit,
**	returns		A pointer to a malloc'd string which MUST BE FREED
*/
PUBLIC char * HTParse ARGS3(
	CONST char *,	aName,
	CONST char *,	relatedName,
	int,		wanted)
{
    char * result = NULL;
    char * return_value = NULL;
    int len;
    char * name = NULL;
    char * rel = NULL;
    char * p;
    char * access;
    struct struct_parts given, related;
    
    /* Make working copies of input strings to cut up:
    */
    len = strlen(aName)+strlen(relatedName)+10;
    result = (char *)malloc(len);	/* Lots of space: more than enough */
    if (result == NULL)
        outofmem(__FILE__, "HTParse");

    if (TRACE)
	fprintf(stderr,
		"HTParse: aName:%s   relatedName:%s\n",aName,relatedName);
    
    StrAllocCopy(name, aName);
    StrAllocCopy(rel, relatedName);

    scan(name, &given);
    scan(rel,  &related); 
    result[0] = '\0';		/* Clear string  */
    if (given.access && given.host && !given.relative && !given.absolute) {
        if (!strcmp(given.access, "http") ||
	    !strcmp(given.access, "https") ||
	    !strcmp(given.access, "ftp"))
	    given.absolute = "";	/* Assume root */
    }
    access = given.access ? given.access : related.access;
    if (wanted & PARSE_ACCESS)
        if (access) {
	    strcat(result, access);
	    if (wanted & PARSE_PUNCTUATION)
	        strcat(result, ":");
	}

    /* If different, inherit nothing. */
    if (given.access && related.access && strcmp(given.access,related.access)) {
	related.host = NULL;
	related.absolute = NULL;
	related.relative = NULL;
	related.anchor = NULL;
    }
	
    if (wanted & PARSE_HOST)
        if (given.host || related.host) {
	    char * tail = result + strlen(result);
	    if (wanted & PARSE_PUNCTUATION)
	        strcat(result, "//");
	    strcat(result, given.host ? given.host : related.host);
#define CLEAN_URLS
#ifdef CLEAN_URLS
	    /* Ignore default port numbers, and trailing dots on FQDNs
	       which will only cause identical addresses to look different */
	    {
	    	char * p, * h;
		p = strchr(tail, ':');
		if (p && access) {		/* Port specified */
		    if ( (    strcmp(access, "http") == 0
		    	   && strcmp(p, ":80") == 0 )
			||
		          (   strcmp(access, "gopher") == 0
		    	   && strcmp(p, ":70") == 0 )
			||
		          (   strcmp(access, "ftp") == 0
		    	   && strcmp(p, ":21") == 0 )
			||
		          (   strcmp(access, "wais") == 0
		    	   && strcmp(p, ":210") == 0 )
		    	||
		          (   (strcmp(access, "nntp") == 0 ||
			       strcmp(access, "news") == 0)
		    	   && strcmp(p, ":119") == 0 )
		    	||
		          (   strcmp(access, "snews") == 0
		    	   && strcmp(p, ":563") == 0 )
		    	||
		          (   strcmp(access, "finger") == 0
		    	   && strcmp(p, ":79") == 0 )
		    	||
		          (   strcmp(access, "cso") == 0
		    	   && strcmp(p, ":105") == 0 )
		    	)
		    *p = '\0';	/* It is the default: ignore it */
		}
		if (!p) { 
		    int len = strlen(tail);
	
		    if (len > 0) {
		        h = tail + len - 1;	/* last char of hostname */
		        if (*h == '.') 
		            *h = '\0';		/* chop final . */
		    }
		} else { 
		    h = p;
		    h--;		/* End of hostname */
		    if (*h == '.') 
			strcpy(h, p);  /* slide p over h */
		}
	    }
#endif /* CLEAN_URLS */
	}
	
    if (given.host && related.host)  /* If different hosts, inherit no path. */
        if (strcmp(given.host, related.host) != 0) {
	    related.absolute = NULL;
	    related.relative = NULL;
	    related.anchor = NULL;
	}
	
    if (wanted & PARSE_PATH) {
        if (access && !given.absolute && given.relative) {
	    if (!strcasecomp(access, "nntp") ||
	        !strcasecomp(access, "snews") ||
		(!strcasecomp(access, "news") &&
		 !strncasecomp(result, "news://", 7))) {
		/*
		 * Treat all given nntp or snews paths,
		 * or given paths for news URLs with a host,
		 * as absolute.
		 */
		given.absolute = given.relative;
		given.relative = NULL;
	    }
	}
        if (given.absolute) {				/* All is given */
	    if (wanted & PARSE_PUNCTUATION)
	        strcat(result, "/");
	    strcat(result, given.absolute);
	    if (TRACE)
	        fprintf(stderr,"1\n");
	} else if (related.absolute) {	/* Adopt path not name */
	    strcat(result, "/");
	    strcat(result, related.absolute);
	    if (given.relative) {
		p = strchr(result, '?');	/* Search part? */
		if (!p)
		    p = result+strlen(result)-1;
		for (; *p!='/'; p--)
		    ;				/* last / */
		p[1] = '\0';			/* Remove filename */
		strcat(result, given.relative);	/* Add given one */
		HTSimplify (result);
	    }
	    if (TRACE)
	        fprintf(stderr,"2\n");
	} else if (given.relative) {
	    strcat(result, given.relative);		/* what we've got */
	    if (TRACE)
	        fprintf(stderr,"3\n");
	} else if (related.relative) {
	    strcat(result, related.relative);
	    if (TRACE)
	        fprintf(stderr,"4\n");
	} else {  /* No inheritance */
	    strcat(result, "/");
	    if (TRACE)
	        fprintf(stderr,"5\n");
	}
    }
		
    if (TRACE)
	fprintf(stderr,"HTParse: result:%s\n",result);

    if (wanted & PARSE_ANCHOR)
        if (given.anchor || related.anchor) {
	    if (wanted & PARSE_PUNCTUATION)
	        strcat(result, "#");
	    strcat(result, given.anchor ? given.anchor : related.anchor);
	}
    FREE(rel);
    FREE(name);
    
    StrAllocCopy(return_value, result);
    FREE(result);

    return return_value;		/* exactly the right length */
}


/*	        Simplify a filename
**		-------------------
**
** A unix-style file is allowed to contain the seqeunce xxx/../ which may be
** replaced by "" , and the seqeunce "/./" which may be replaced by "/".
** Simplification helps us recognize duplicate filenames.
**
**	Thus, 	/etc/junk/../fred 	becomes	/etc/fred
**		/etc/junk/./fred	becomes	/etc/junk/fred
**
**      but we should NOT change
**		http://fred.xxx.edu/../..
**
**	or	../../albert.html
*/
PUBLIC void HTSimplify ARGS1(
	char *,		filename)
{
    char * p;
    char * q;

    if (filename == NULL)
	return;

    if (filename[0] && filename[1])	/* Bug fix 12 Mar 93 TBL */
     for (p = filename+2; *p; p++) {
        if (*p == '/') {
	    if ((p[1] == '.') && (p[2]=='.') && (p[3] == '/' || !p[3])) {
		for (q = (p-1); (q >= filename) && (*q != '/'); q--)
		    ;			/* prev slash */
		if (q[0] == '/' && 0 != strncmp(q, "/../", 4) &&
		    !(q-1 > filename && q[-1] == '/')) {
	            strcpy(q, p+3);	/* Remove  /xxx/.. */
		    if (!(*filename))
		        strcpy(filename, "/");
		    p = q-1;		/* Start again with prev slash 	*/
		} else {		/*   xxx/.. leave it! */
#ifdef BUG_CODE
		    strcpy(filename, p[3] ? p+4 : p+3); /* rm  xxx/../	*/
		    p = filename;	/* Start again */
#endif /* BUG_CODE */
		}
	    } else if ((p[1] == '.') && (p[2] == '/' || !p[2])) {
	        strcpy(p, p+2);		/* Remove a slash and a dot */
	    }
	}
    }
}


/*		Make Relative Name
**		------------------
**
** This function creates and returns a string which gives an expression of
** one address as related to another. Where there is no relation, an absolute
** address is retured.
**
**  On entry,
**	Both names must be absolute, fully qualified names of nodes
**	(no anchor bits)
**
**  On exit,
**	The return result points to a newly allocated name which, if
**	parsed by HTParse relative to relatedName, will yield aName.
**	The caller is responsible for freeing the resulting name later.
**
*/
PUBLIC char * HTRelative ARGS2(
	CONST char *,	aName,
	CONST char *,	relatedName)
{
    char * result = NULL;
    CONST char *p = aName;
    CONST char *q = relatedName;
    CONST char * after_access = NULL;
    CONST char * path = NULL;
    CONST char * last_slash = NULL;
    int slashes = 0;
    
    for (; *p; p++, q++) {	/* Find extent of match */
    	if (*p != *q)
	    break;
	if (*p == ':')
	    after_access = p+1;
	if (*p == '/') {
	    last_slash = p;
	    slashes++;
	    if (slashes == 3)
	        path=p;
	}
    }
    
    /* q, p point to the first non-matching character or zero */
    
    if (!after_access) {			/* Different access */
        StrAllocCopy(result, aName);
    } else if (slashes < 3){			/* Different nodes */
    	StrAllocCopy(result, after_access);
    } else if (slashes == 3){			/* Same node, different path */
        StrAllocCopy(result, path);
    } else {					/* Some path in common */
        int levels = 0;
        for (; *q && (*q!='#'); q++)
	    if (*q=='/')
	        levels++;
	result = (char *)malloc(3*levels + strlen(last_slash) + 1);
        if (result == NULL)
	    outofmem(__FILE__, "HTRelative");
	result[0] = '\0';
	for (; levels; levels--)
	    strcat(result, "../");
	strcat(result, last_slash+1);
    }
    if (TRACE)
        fprintf(stderr, "HT: `%s' expressed relative to\n    `%s' is\n   `%s'.",
    		aName, relatedName, result);
    return result;
}


/*		Escape undesirable characters using %		HTEscape()
**		-------------------------------------
**
**	This function takes a pointer to a string in which
**	some characters may be unacceptable unescaped.
**	It returns a string which has these characters
**	represented by a '%' character followed by two hex digits.
**
**	Unlike HTUnEscape(), this routine returns a malloced string.
*/

PRIVATE CONST unsigned char isAcceptable[96] =

/*	Bit 0		xalpha		-- see HTFile.h
**	Bit 1		xpalpha		-- as xalpha but with plus.
**	Bit 3 ...	path		-- as xpalphas but with /
*/
    /*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
    {    0,0,0,0,0,0,0,0,0,0,7,6,0,7,7,4,	/* 2x   !"#$%&'()*+,-./	 */
         7,7,7,7,7,7,7,7,7,7,0,0,0,0,0,0,	/* 3x  0123456789:;<=>?	 */
	 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,	/* 4x  @ABCDEFGHIJKLMNO  */
	 7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,7,	/* 5X  PQRSTUVWXYZ[\]^_	 */
	 0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,	/* 6x  `abcdefghijklmno	 */
	 7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,0 };	/* 7X  pqrstuvwxyz{\}~	DEL */

PRIVATE char *hex = "0123456789ABCDEF";
#define ACCEPTABLE(a)	( a>=32 && a<128 && ((isAcceptable[a-32]) & mask))

PUBLIC char * HTEscape ARGS2(
	CONST char *,	str,
	unsigned char,	mask)
{
    CONST char * p;
    char * q;
    char * result;
    int unacceptable = 0;
    for (p = str; *p; p++)
        if (!ACCEPTABLE((unsigned char)TOASCII(*p)))
	    unacceptable++;
    result = (char *) malloc(p-str + unacceptable+ unacceptable + 1);
    if (result == NULL)
        outofmem(__FILE__, "HTEscape");
    for (q = result, p = str; *p; p++) {
    	unsigned char a = TOASCII(*p);
	if (!ACCEPTABLE(a)) {
	    *q++ = HEX_ESCAPE;	/* Means hex commming */
	    *q++ = hex[a >> 4];
	    *q++ = hex[a & 15];
	}
	else *q++ = *p;
    }
    *q++ = '\0';			/* Terminate */
    return result;
}


/*		Escape undesirable characters using %		HTEscapeSP()
**		-------------------------------------
**
**	This function takes a pointer to a string in which
**	some characters may be unacceptable unescaped.
**	It returns a string which has these characters
**	represented by a '%' character followed by two hex digits,
**	except that spaces are converted to '+'.
**
**	Unlike HTUnEscape(), this routine returns a malloced string.
*/
PUBLIC char * HTEscapeSP ARGS2(
	CONST char *,	str,
	unsigned char,	mask)
{
    CONST char * p;
    char * q;
    char * result;
    int unacceptable = 0;
    for (p = str; *p; p++)
        if (!(*p == ' ' || ACCEPTABLE((unsigned char)TOASCII(*p))))
	    unacceptable++;
    result = (char *) malloc(p-str + unacceptable+ unacceptable + 1);
    if (result == NULL)
        outofmem(__FILE__, "HTEscape");
    for (q = result, p = str; *p; p++) {
    	unsigned char a = TOASCII(*p);
	if (a == 32) {
	    *q++ = '+';
	} else if (!ACCEPTABLE(a)) {
	    *q++ = HEX_ESCAPE;	/* Means hex commming */
	    *q++ = hex[a >> 4];
	    *q++ = hex[a & 15];
	} else {
	    *q++ = *p;
	}
    }
    *q++ = '\0';			/* Terminate */
    return result;
}


/*		Decode %xx escaped characters			HTUnEscape()
**		-----------------------------
**
**	This function takes a pointer to a string in which some
**	characters may have been encoded in %xy form, where xy is
**	the acsii hex code for character 16x+y.
**	The string is converted in place, as it will never grow.
*/

PRIVATE char from_hex ARGS1(
	char,		c)
{
    return  c >= '0' && c <= '9' ?  c - '0' 
    	    : c >= 'A' && c <= 'F'? c - 'A' + 10
    	    : c - 'a' + 10;	/* accept small letters just in case */
}

PUBLIC char * HTUnEscape ARGS1(
	char *,		str)
{
    char * p = str;
    char * q = str;
    while (*p) {
        if (*p == HEX_ESCAPE) {
	    p++;
	    if (*p)
	        *q = from_hex(*p++) * 16;
	    if (*p)
	        *q = FROMASCII(*q + from_hex(*p++));
	    q++;
	} else {
	    *q++ = *p++; 
	}
    }
    
    *q++ = '\0';
    return str;
    
} /* HTUnEscape */


