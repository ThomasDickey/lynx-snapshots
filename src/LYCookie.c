/*	  		       Lynx Cookie Support	           LYCookies.c
**			       ===================
**
**	Author:	AMK	A.M. Kuchling (amk@magnet.com)	12/25/96
**
**	Incorporated with mods by FM			01/16/97
**
**  Based on:
**	http://www.ics.uci.edu/pub/ietf/http/draft-ietf-http-state-mgmt-05.txt
**
**  TO DO: (roughly in order of decreasing priority)
      * host_matches() is only lightly tested in the Internet at large.
      * Hex escaping isn't considered at all.  Any semi-colons, commas,
        or spaces actually in cookie names or values (i.e., not serving
	as punctuation for the overall Set-Cookie value) should be hex
	escaped if not quoted, but presumeably the server is expecting
	them to be hex escaped in our Cookie request header as well, so
	in theory we need not unescape them.  We'll see how this works
	out in practice.
      * The prompt should show more information about the cookie being
        set in Novice mode.
      * The truncation heuristic in HTConfirmCookie should probably be 
        smarter, smart enough to leave a really short name/value string 
	alone.
      * We protect against denial-of-service attacks (see section 6.3.1
        of the draft) by limiting a domain to 50 cookies, limiting the
	total number of cookies to 500, and limiting a processed cookie
	to a maximum of 4096 bytes, but we count on the normal garbage
	collections to bring us back down under the limits, rather than
	actively removing cookies and/or domains based on age or frequency
	of use.
      * The comments in this file chould contain extracts from the -05
        HTTP State Management draft (URL above).
      * If the a cookie has the secure flag set, we presently treat only
        SSL connections as secure.  This may need to be expanded for other
        secure communication protocols that become standarized.
      * Cookies could be optionally stored in a file from session to session.
*/

#include "HTUtils.h"
#include "tcp.h"
#include "HTAccess.h"
#include "HTParse.h"
#include "HTAlert.h"
#include "LYCurses.h"
#include "LYSignal.h"
#include "LYUtils.h"
#include "LYCharUtils.h"
#include "LYClean.h"
#include "LYGlobalDefs.h"
#include "LYEdit.h"
#include "LYStrings.h"
#include "LYSystem.h"
#include "GridText.h"
#include "LYCookie.h"

#define FREE(x) if (x) {free(x); x = NULL;}

/*
**  The first level of the cookie list is a list indexed by the domain
**  string; cookies with the same domain will be placed in the same
**  list.  Thus, finding the cookies that apply to a given URL is a
**  two-level scan; first we check each domain to see if it applies,
**  and if so, then we check the paths of all the cookies on that
**  list.   We keep a running total of cookies as we add or delete
**  them
*/
PRIVATE HTList *domain_list = NULL;
PRIVATE HTList *cookie_list = NULL;
PRIVATE int total_cookies = 0;

struct _cookie {
    char *lynx_id; /* Lynx cookie identifier */
    char *name;    /* Name of this cookie */
    char *value;   /* Value of this cookie */
    int version;   /* Cookie protocol version (=1) */
    char *comment; /* Comment to show to user */
    char *domain;  /* Domain for which this cookie is valid */
    int port;      /* Server port from which this cookie was given (usu. 80) */
    char *path;    /* Path prefix for which this cookie is valid */
    int pathlen;   /* Length of the path */
    int flags;     /* Various flags */
    time_t expires;   /* The time when this cookie expires */
    BOOL quoted;   /* Was a value quoted in the Set-Cookie header? */
};
typedef struct _cookie cookie;

#define COOKIE_FLAG_SECURE 1	   /* If set, cookie requires secure links */
#define COOKIE_FLAG_EXPIRES_SET 2  /* If set, an expiry date was set */

struct _HTStream 
{
  HTStreamClass * isa;
};

PRIVATE void MemAllocCopy ARGS3(
	char **,	dest,
	CONST char *,	start,
	CONST char *,	end)
{
    char *temp;
    
    if (!(start && end) || (end <= start)) {
        HTSACopy(dest, "");
	return;
    }

    temp = (char *)calloc(1, ((end - start) + 1));
    if (temp == NULL)
        outofmem(__FILE__, "MemAllocCopy");
    LYstrncpy(temp, (char *)start, (end - start));
    HTSACopy(dest, temp);
    FREE(temp);
}

PRIVATE cookie * newCookie NOARGS
{
    cookie *p = (cookie *)calloc(1, sizeof(cookie));
    char lynx_id[64];

    if (p == NULL)
        outofmem(__FILE__, "newCookie");
    sprintf(lynx_id, "%p", p);
    StrAllocCopy(p->lynx_id, lynx_id);
    p->port = 80; 
    return p;
}

PRIVATE void freeCookie ARGS1(
	cookie *,	co)
{
    if (co) {
        FREE(co->lynx_id);
	FREE(co->name); 
	FREE(co->value);
	FREE(co->comment);
	FREE(co->domain);
	FREE(co->path);
	FREE(co);
    }
}

PRIVATE void LYCookieJar_free NOARGS
{
    HTList *dl = domain_list;
    domain_entry *de = NULL;
    HTList *cl = NULL, *next = NULL;
    cookie *co = NULL;

    while (dl) {
	if ((de = dl->object) != NULL) {
	    cl = de->cookie_list;
	    while (cl) {
		next = cl->next;
		co = cl->object;
		if (co) {
		    HTList_removeObject(de->cookie_list, co);
		    freeCookie(co);
		}
		cl = next;
	    }
	    FREE(de->domain);
	    HTList_delete(de->cookie_list);
	    de->cookie_list = NULL;
	}
	dl = dl->next;
    }
    if (dump_output_immediately) {
	cl = cookie_list;
	while (cl) {
	    next = cl->next;
	    co = cl->object;
	    if (co) {
		HTList_removeObject(cookie_list, co);
		freeCookie(co);
	    }
	    cl = next;
	}
	HTList_delete(cookie_list);
    }
    cookie_list = NULL;
    HTList_delete(domain_list);
    domain_list = NULL;
}

/*
**  Compare two hostnames as specified in Section 2 of:
**    http://www.ics.uci.edu/pub/ietf/http/draft-ietf-http-state-mgmt-05.txt
*/
PRIVATE BOOLEAN host_matches ARGS2(
	CONST char *,	A,
	CONST char *,	B)
{
    /*
     *  The following line will handle both numeric IP addresses and
     *  FQDNs.  Do numeric addresses require special handling?
     */
    if (*B != '.' && !strcmp(A, B))
	return YES;

    /*
     *  The following will pass a "dotted tail" match to "a.b.c.e"
     *  as described in Section 2 of the -05 draft.
     */
    if (*B == '.') {
	int diff = (strlen(A) - strlen(B));
	if (diff > 0) {
	    if (!strcmp((A + diff), B))
		return YES;
	}
    }
    return NO;
}

/*
**  Store a cookie somewhere in the domain list.
*/
PRIVATE void store_cookie ARGS3(
	cookie *,	co,
	CONST char *,	hostname,
	CONST char *,	path)
{
    HTList *hl, *next;
    cookie *c2;
    time_t now = time(NULL);
    int pos;
    CONST char *ptr;
    domain_entry *de = NULL;

    if (co == NULL)
        return;

    /*
     *  Apply sanity checks.
     *
     *  Section 4.3.2, condition 1: The value for the Path attribute is
     *  not a prefix of the request-URI.
     */
    if (strncmp(co->path, path, co->pathlen) != 0) {
	if (TRACE)
	    fprintf(stderr,
	    "store_cookie: Rejecting because '%s' is not a prefix of '%s'.\n",
		    co->path, path);
        return;
    }
    /*
     *  The next 4 conditions do NOT apply if the domain is still 
     *  the default of request-host.
     */
    if (strcmp(co->domain, hostname) != 0) {
        /*
	 *  The hostname does not contain a dot.
	 */
	if (strchr(hostname, '.') == NULL) {
	    if (TRACE)
		fprintf(stderr,
			"store_cookie: Rejecting because '%s' has no dot.\n",
			hostname);
	    return;
	}

	/*
	 *  Section 4.3.2, condition 2: The value for the Domain attribute
	 *  contains no embedded dots or does not start with a dot. 
	 *  (A dot is embedded if it's neither the first nor last character.) 
	 */
	if (co->domain[0] != '.' || co->domain[1] == '\0') {
	    if (TRACE)
		fprintf(stderr,
			"store_cookie: Rejecting domain '%s'.\n",
			co->domain);
	    return;
	}
	ptr = strchr((co->domain + 1), '.');
	if (ptr == NULL || ptr[1] == '\0') {
	    if (TRACE)
		fprintf(stderr,
			"store_cookie: Rejecting domain '%s'.\n",
			co->domain);
	    return;
	}
      
        /*
	 *  Section 4.3.2, condition 3: The value for the request-host does
	 *  not domain-match the Domain attribute.
	 */
	if (!host_matches(hostname, co->domain)) {
	    if (TRACE)
		fprintf(stderr,
			"store_cookie: Rejecting domain '%s' for host '%s'.\n",
			co->domain, hostname);
	    return;
	}

        /*
	 *  Section 4.3.2, condition 4: The request-host is a FQDN (not IP
	 *  address) and has the form HD, where D is the value of the Domain
	 *  attribute, and H is a string that contains one or more dots.
	 */
	ptr = ((hostname + strlen(hostname)) - strlen(co->domain));
	if (strchr(hostname, '.') < ptr) {
	    if (TRACE)
		fprintf(stderr,
			"store_cookie: Rejecting domain '%s' for host '%s'.\n",
			co->domain, hostname);
	    return;
	}
    }

    /*
     *  Ensure that the domain list exists.
     */
    if (domain_list == NULL) {
        atexit(LYCookieJar_free);
        domain_list = HTList_new();
	total_cookies = 0;
    }

    /*
     *  Look through domain_list to see if the cookie's domain
     *  is already listed.
     */
    if (dump_output_immediately) { /* Non-interactive, can't respond */
	if (cookie_list == NULL)
	    cookie_list = HTList_new();
    } else {
	cookie_list = NULL;
	for (hl = domain_list; hl != NULL; hl = hl->next) {
	    de = (domain_entry *)hl->object;
	    if ((de != NULL && de->domain != NULL) &&
	    	!strcmp(co->domain, de->domain)) {
	        cookie_list = de->cookie_list;
		break;
	    }
	}
	if (hl == NULL) {
	    /*
	     *  Domain not found; add a new entry for this domain.
	     */
	    de = (domain_entry *)calloc(1, sizeof(domain_entry));
	    if (de == NULL)
		outofmem(__FILE__, "store_cookie");
	    de->bv = QUERY_USER;
	    cookie_list = de->cookie_list = HTList_new();
	    StrAllocCopy(de->domain, co->domain);
	    HTList_addObject(domain_list, de);
	}
    }
  
    /*
     *  Loop over the cookie list, deleting expired and matching cookies.
     */
    hl = cookie_list;
    pos = 0;
    while (hl) {
	c2 = (cookie *)hl->object;
	next = hl->next;
	/*
	 *  Check if this cookie has expired.
	 */
	if ((c2 != NULL) &&
	    (c2->flags & COOKIE_FLAG_EXPIRES_SET) &&
	    c2->expires < now) {
	    HTList_removeObject(cookie_list, c2);
	    freeCookie(c2);
	    c2 = NULL;
	    total_cookies--;

	/*
	 *  Check if this cookie matches the one we're inserting.
	 */
	} else if ((c2) && (co->port == c2->port) && 
		   !strcmp(co->domain, c2->domain) &&
		   !strcmp(co->path, c2->path) &&
		   !strcmp(co->name, c2->name)) {
	    HTList_removeObject(cookie_list, c2);
	    freeCookie(c2);
	    c2 = NULL;
	    total_cookies--;

	} else if ((c2) && (c2->pathlen) > (co->pathlen)) {
	    pos++;
	}
	hl = next;
    }

    /*
     *  Don't bother to add the cookie if it's already expired.
     */
    if ((co->flags & COOKIE_FLAG_EXPIRES_SET) && co->expires < now) {
	freeCookie(co);
	co = NULL;

    /*
     *  Don't add the cookie if we're over the domain's limit. - FM
     */
    } else if (HTList_count(cookie_list) > 50) {
        if (TRACE)
	    fprintf(stderr,
	"store_cookie: Domain's cookie limit exceeded!  Rejecting cookie.\n");
        freeCookie(co);
	co = NULL;

    /*
     *  Don't add the cookie if we're over the total cookie limit. - FM
     */
    } else if (total_cookies > 500) {
        if (TRACE)
	    fprintf(stderr,
	"store_cookie: Total cookie limit exceeded!  Rejecting cookie.\n");
        freeCookie(co);
	co = NULL;

    /*
     *  Get confirmation if we need it, and add cookie
     *  if confirmed or 'allow' is set to always. - FM
     */
    } else if (HTConfirmCookie(de, hostname, 
			       co->domain, co->path, co->name, co->value)) {
	HTList_insertObjectAt(cookie_list, co, pos);
	total_cookies++;
    } else {
        freeCookie(co);
	co = NULL;
    }
}

PRIVATE char * scan_cookie_sublist ARGS6(
	CONST char *,	hostname,
	CONST char *,	path,
	int,		port,
	HTList *,	sublist,
	char *,		header,
	BOOL,		secure)
{
    HTList *hl = sublist, *next = NULL;
    cookie *co;
    time_t now = time(NULL);
    BOOL Quoted = FALSE;

    while (hl) {
	co = (cookie *)hl->object;
	next = hl->next;

	if (TRACE && co) {
	    fprintf(stderr, "Checking cookie %lx %s=%s\n",
		    	    (long) hl,
			    (co->name ? co->name : "(no name)"),
			    (co->value ? co->value : "(no value)"));
	    fprintf(stderr, "%s %s %i %s %s %i%s\n",
	    		    hostname,
			    (co->domain ? co->domain : "(no domain)"),
			    host_matches(hostname, co->domain),
			    path, co->path, ((co->pathlen > 0) ?
			  strncmp(path, co->path, co->pathlen) : 0),
			    ((co->flags & COOKIE_FLAG_SECURE) ?
			    			   " secure" : ""));
	}
 	/*
	 *  Check if this cookie has expired, and if so, delete it.
	 */
	if (((co) && (co->flags & COOKIE_FLAG_EXPIRES_SET)) &&
	    co->expires < now) {
	    HTList_removeObject(sublist, co);
	    freeCookie(co);
	    co = NULL;
	    total_cookies--;
	}

	/*
	 *  Check if we have a unexpired match, and handle if we do.
	 */
	if (((co != NULL) &&
	     co->port == port && host_matches(hostname, co->domain)) && 
	    (co->pathlen == 0 || !strncmp(path, co->path, co->pathlen))) {
	    /*
	     *  Skip if the secure flag is set and we don't have
	     *  a secure connection.  HTTP.c presently treats only
	     *  SSL connections as secure. - FM
	     */
	    if ((co->flags & COOKIE_FLAG_SECURE) && secure == FALSE) {
	        hl = next;
		continue;
	    }

	    /*
	     *  Start or append to the request header.
	     */
	    if (header == NULL) {
	        if (co->version > 0) {
		    /*
		     *  For Version 1 (or greater) cookies,
		     *  the version number goes before the
		     *  first cookie.
		     */
		    char version[16];
		    sprintf(version, "$Version=%i; ", co->version);
		    StrAllocCopy(header, version);
		}
	    } else {
		/*
		 *  There's already cookie data there,
		 *  so add a separator.
		 */
		StrAllocCat(header, "; ");
	    }
	    /*
	     *  Include the cookie name=value pair.
	     */
	    StrAllocCat(header, co->name);
	    StrAllocCat(header, "=");
	    if (co->quoted && strchr(co->value, ' ')) {
	    	StrAllocCat(header, "\"");
		Quoted = TRUE;
	    }
	    StrAllocCat(header, co->value);
	    if (Quoted) {
	    	StrAllocCat(header, "\"");
		Quoted = FALSE;
	    }
	    /*
	     *  For Version 1 (or greater) cookies, add
	     *  attributes for the cookie. - FM
	     */
	    if (co->version > 0) {
		if (co->path) {
		    /*
		     *  Append the path attribute. - FM
		     */
		    StrAllocCat(header, "; $Path=");
		    if (co->quoted && strchr(co->path, ' ')) {
		        StrAllocCat(header, "\"");
			Quoted = TRUE;
		    }
		    StrAllocCat(header, co->path);
		    if (Quoted) {
		        StrAllocCat(header, "\"");
			Quoted = FALSE;
		    }
		}
		if (co->domain) {
		    /*
		     *  Append the domain attribute. - FM
		     */
		    StrAllocCat(header, "; $Domain=");
		    if (co->quoted && strchr(co->domain, ' ')) {
		        StrAllocCat(header, "\"");
			Quoted = TRUE;
		    }
		    StrAllocCat(header, co->domain);
		    if (Quoted) {
		        StrAllocCat(header, "\"");
			Quoted = FALSE;
		    }
		}
	    }
	}
	hl = next;
    }
 
    return(header);
}

PUBLIC void LYSetCookie ARGS2(
	CONST char *,	header,
	CONST char *,	address)
{
    CONST char *p, *attr_start, *attr_end, *value_start, *value_end;
    char *hostname = NULL, *path = NULL, *ptr;
    cookie *cur_cookie = NULL;
    int port = 80;
    int length = 0;
    BOOL Quoted = FALSE;

    /*
     *  Get the hostname, port and path of the address, and report
     *  the Set-Cookie request if trace mode is on, but set the cookie
     *  only if LYSetCookies is TRUE. - FM
     */
    if (((hostname = HTParse(address, "", PARSE_HOST)) != NULL) &&
	(ptr = strchr(hostname, ':')) != NULL)  {
	/*
	 *  Replace default port number.
	 */
	*ptr = '\0';
	ptr++;
        port = atoi(ptr);
    } else if (!strncasecomp(address, "https:", 6)) {
        port = 443;
    } 
    if (((path = HTParse(address, "",
    			 PARSE_PATH|PARSE_PUNCTUATION)) != NULL) &&
	(ptr = strrchr(path, '/')) != NULL) {
	if (ptr == path)
	    *(ptr+1) = '\0';	/* Leave a single '/' alone */
	else
	    *ptr = '\0'; 
      }
    if (TRACE) {
	fprintf(stderr,
    "LYSetCookie called with host '%s', path '%s',\n    and header '%s'%s\n",
		(hostname ? hostname : ""),
		(path ? path : ""),
		(header ? header : ""),
	     (LYSetCookies ? "" : "\n    Ignoring this Set-Cookie request."));
    }
    if (LYSetCookies == FALSE) {
	FREE(hostname);
	FREE(path);
	return;
    }

    /*
     *  Process the Set-Cookie header value.
     */
    p = (char *)header;
    while (length <= 4096 && *p) {
	attr_start = attr_end = value_start = value_end = NULL;
#define SKIP_SPACES while (*p != '\0' && isspace((unsigned char)*p)) p++;
        SKIP_SPACES;
	/*
	 *  Get the attribute name.
	 */
	attr_start = p;
	while (*p != '\0' && !isspace((unsigned char)*p) &&
	       *p != '=' && *p != ';' && *p != ',')
	    p++;
	attr_end = p;
	SKIP_SPACES;
      
        /*
	 *  Check for an '=' delimiter, or an 'expires' name followed
	 *  by white, since Netscape's bogus parser doesn't require
	 *  an '=' delimiter, and 'expires' attributes are being
	 *  encountered without them. - FM
	 */
	if (*p == '=' ||
	     !strncasecomp(attr_start, "Expires", 7)) {
	    /*
	     *  Get the value string.
	     */
	    p++;
	    SKIP_SPACES;
	    /*
	     *  Hack alert!  We must handle Netscape-style cookies with
	     *		"Expires=Mon, 01-Jan-96 13:45:35 GMT" or
	     *		"Expires=Mon,  1 Jan 1996 13:45:35 GMT".
	     *  No quotes, but there are spaces.  Argh... 
	     *  Anyway, we know it will have at least 3 space separators
	     *  within it, and two dashes or two more spaces, so this code
	     *  looks for a space after the 5th space separator or dash to
	     *  mark the end of the value. - FM
	     */
	    if ((attr_end - attr_start) == 7 && 
		!strncasecomp(attr_start, "Expires", 7)) {
		int spaces = 6;
		value_start = p;
		while (*p != '\0' && *p != ';' && spaces) {
		    p++;
		    if (isspace((unsigned char)*p)) {
		        while (isspace((unsigned char)*(p + 1)))
			    p++;
		        spaces--;
		    } else if (*p == '-') {
		        spaces--;
		    }
		}
	        value_end = p;
	    } else if (*p == '"') {
	        /*
		 *  It's a quoted string.
		 */
		p++;
		value_start = p;
		while (*p != '\0' && *p != '"')
		    p++;
		value_end = p;
		if (*p == '"')
		    p++;
		Quoted = TRUE;
	    } else {
		/*
		 *   Otherwise, it's an unquoted string.
		 */
		value_start = p;
		while (*p != '\0' && *p != ';' && *p != ',')
		    p++;
		value_end = p;
		/*
		 *  Trim trailing spaces.
		 */
		if ((value_end > value_start) &&
		    isspace((unsigned char)*(value_end - 1))) {
		    value_end--;
		    while ((value_end > (value_start + 1)) &&
		    	   isspace((unsigned char)*value_end) &&
			   isspace((unsigned char)*(value_end - 1))) {
		        value_end--;
		    }
		}
	    }
	}

	/*
	 *  Check for a separator character, and skip it.
	 */
	if (*p == ';' || *p == ',')
	    p++;

	/*
	 *  Now, we can handle this attribute/value pair.
	 */
	if (attr_end > attr_start) {
	    int len = (attr_end - attr_start);
	    BOOLEAN known_attr = NO;
	    char *value = NULL;

	    if (value_end > value_start) {
		int value_len = (value_end - value_start);

		length += value_len;
		if (length > 4096)
		    break;
		value = (char *)calloc(1, value_len + 1);
		if (value == NULL)
		    outofmem(__FILE__, "LYSetCookie");
		LYstrncpy(value, (char *)value_start, value_len);
	    }
	    if (len == 6 && !strncasecomp(attr_start, "secure", 6)) {
		known_attr = YES;
		if (cur_cookie != NULL)
		    cur_cookie->flags |= COOKIE_FLAG_SECURE;
	    } else if (len == 7 && !strncasecomp(attr_start, "comment", 7)) {
		known_attr = YES;
		if (cur_cookie != NULL && value)
		    StrAllocCopy(cur_cookie->comment, value);
	    } else if (len == 6 && !strncasecomp(attr_start, "domain", 6)) {
		known_attr = YES;
		if (cur_cookie != NULL && value)
		    StrAllocCopy(cur_cookie->domain, value);
	    } else if (len == 4 && !strncasecomp(attr_start, "path", 4)) {
		known_attr = YES;
		if (cur_cookie != NULL && value) {
		    StrAllocCopy(cur_cookie->path, value);
		    cur_cookie->pathlen = strlen(cur_cookie->path);
		}
	    } else if (len == 7 && !strncasecomp(attr_start, "version", 7)) {
		known_attr = YES;
		if (cur_cookie != NULL && value) {
		    int temp = strtol(value, NULL, 10);
		    if (errno != -ERANGE)
			cur_cookie->version = temp;
		}
	    } else if (len == 7 && !strncasecomp(attr_start, "max-age", 7)) {
		known_attr = YES;
		if (cur_cookie != NULL && value) {
		    int temp = strtol(value, NULL, 10);
		    cur_cookie->flags |= COOKIE_FLAG_EXPIRES_SET;
		    if (errno == -ERANGE) {
			cur_cookie->expires = (time_t)0;
		    } else {
			cur_cookie->expires = (time(NULL) + temp);
			if (TRACE)
			    fprintf(stderr,
			    	    "LYSetCooke: expires %ld, %s",
				    (long) cur_cookie->expires,
				    ctime(&cur_cookie->expires));
		    }
		}
	    } else if (len == 7 && !strncasecomp(attr_start, "expires", 7)) {
	    	/*
		 *  Convert an 'expires' attribute value for Version 0
		 *  cookies, or for Version 1 if we haven't received a
		 *  'max-age', to the equivalent of a Version 1 (or greater)
		 *  'max-age' value added to 'time(NULL)'.  Note that
		 *  'expires' should not be used in Version 1 cookies,
		 *  but, as explained below, it might be used for "backward
		 *  compatibility", and, in turn, ill-informed people
		 *  surely would start using it instead of, rather than
		 *  in addition to, 'max-age'. - FM
		 */
		if (((cur_cookie) && cur_cookie->version < 1) ||
		    ((cur_cookie) &&
		     !(cur_cookie->flags & COOKIE_FLAG_EXPIRES_SET))) {
		    known_attr = YES;
		    if (value) {
			cur_cookie->flags |= COOKIE_FLAG_EXPIRES_SET;
		        cur_cookie->expires = LYmktime(value);
			if (cur_cookie->expires > 0) {
			    if (TRACE)
				fprintf(stderr,
					"LYSetCooke: expires %ld, %s",
					(long) cur_cookie->expires,
					ctime(&cur_cookie->expires));
			}
		    }
		}
		/*
		 *  If it's a version 1 (or greater) cookie, then you
		 *  really can set a cookie named 'expires', so we 
		 *  perhaps shouldn't set known_attr.  However, an
		 *  -06 draft, which hopefully will be shot down as
		 *  bogus, suggests adding (invalidly) 'expires'
		 *  headers for "backward compatibility".  So, we'll
		 *  set known-attr, to ignore it, and hope it works
		 *  out. - FM
		 */
		known_attr = YES;
	    }

	    /*
	     *  If none of the above comparisions succeeded, and we have
	     *  a value, then we have an unknown pair of the form 'foo=bar',
	     *  which means it's time to create a new cookie.  If we don't
	     *  have a non-zero-length value, assume it's an error or a
	     *  new, unknown attribute which doesn't take a value, and
	     *  ignore it. - FM
	     */
	    if (!known_attr && value_end > value_start) {
		store_cookie(cur_cookie, hostname, path);
		cur_cookie = newCookie();
		length += len;
		if (length > 4096) {
		    FREE(value);
		    break;
		}
		StrAllocCopy(cur_cookie->domain, hostname);
		StrAllocCopy(cur_cookie->path, path);
		cur_cookie->pathlen = strlen(cur_cookie->path);
		cur_cookie->port = port;
		MemAllocCopy(&(cur_cookie->name), attr_start, attr_end);
		MemAllocCopy(&(cur_cookie->value), value_start, value_end);
		cur_cookie->quoted = Quoted;
		Quoted = FALSE;
	    }
	    FREE(value);
	}
	if (TRACE) {
	    fprintf(stderr, "LYSetCookie: attr=value pair: ");
	    if (attr_start == attr_end)  {
		fprintf(stderr, "[No attr]");
	    } else {
		CONST char *i;
	      
		for (i = attr_start; i < attr_end; i++)
		    putc(*i, stderr);
	    }
	    fprintf(stderr, "=");
	    if (value_start >= value_end) {
		fprintf(stderr, "[No value]");
	    } else {
		CONST char *i;

		for (i = value_start; i < value_end; i++)
		    putc(*i, stderr);
	    }
	    fprintf(stderr, "\n");
	}
    }

    /*
     *  Store the final cookie if within length limit. - FM
     */
    if (length <= 4096 && cur_cookie != NULL) {
        /*
	 *  Force the secure flag on if it's not set but this
	 *  is an https URL.  This ensures that cookies from
	 *  https servers will not be shared with ones for
	 *  http (non-SSL) servers and thus be transmitted
	 *  unencrypted, and is redundant with the current,
	 *  blanket port restriction.  However, this seemed
	 *  a side-effect rather than conscious intent within
	 *  the port restriction.  A port attribute is likely
	 *  to be added, and we can independently regulate
	 *  sharing based on port versus scheme, with user
	 *  configuration and run time options, "when the
	 *  time is right". - FM
	 */
        if (!strncasecomp(address, "https:", 6) &&
	    !(cur_cookie->flags & COOKIE_FLAG_SECURE)) {
	    cur_cookie->flags |= COOKIE_FLAG_SECURE;
	    if (TRACE)
	        fprintf(stderr, "LYSetCookie: Forced the 'secure' flag on.\n");
	}
        store_cookie(cur_cookie, hostname, path);
    } else {
	if (TRACE)
	    fprintf(stderr, "LYSetCookie: Rejecting cookie due to length!\n");
        freeCookie(cur_cookie);
    }
    FREE(hostname);
    FREE(path);
}

PUBLIC char * LYCookie ARGS4(
	CONST char *,	hostname,
	CONST char *,	path,
	int,		port,
	BOOL,		secure)
{
    char *header = NULL;
    HTList *hl = domain_list, *next = NULL;
    domain_entry *de;

    if (TRACE) {
	fprintf(stderr,
		"LYCookie: Searching for '%s:%i', '%s'.\n",
		(hostname ? hostname : "(null)"),
		port,
		(path ? path : "(null)"));
    }

    /*
     *  Search the cookie_list elements in the domain_list
     *  for any cookies associated with the //hostname:port/path
     */
    while (hl) {
	de = (domain_entry *)hl->object;
	next = hl->next;

	if (de != NULL) {
	    if (!HTList_isEmpty(de->cookie_list)) {
	        /*
		 *  Scan the domain's cookie_list for
		 *  any cookies we should include in
		 *  our request header.
		 */
	        header = scan_cookie_sublist(hostname, path, port,
					     de->cookie_list, header, secure);
	    } else if (de->bv == QUERY_USER) {
		/*
		 *  No cookies in this domain, and no default
		 *  accept/reject choice was set by the user,
		 *  so delete the domain. - FM
		 */
		FREE(de->domain);
		HTList_delete(de->cookie_list);
		de->cookie_list = NULL;
		HTList_removeObject(domain_list, de);
		de = NULL;
	    }
	}
        hl = next;
    }
    if (header)
        return(header);

    /*
     *  If we set a header, perhaps all the cookies have
     *  expired and we deleted the last of them above, so
     *  check if we should delete and NULL the domain_list. - FM
     */
    if (domain_list) {
	if (HTList_isEmpty(domain_list)) {
	    HTList_delete(domain_list);
	    domain_list = NULL;
	}
    }
    return(NULL);
}

/* 	LYHandleCookies - F.Macrides (macrides@sci.wfeb.edu)
**	---------------
**
**  Lists all cookies by domain, and allows deletions of
**  individual cookies or entire domains, and changes of
**  'allow' settings.  The list is invoked via the COOKIE_JAR
**  command (Ctrl-K), and deletions or changes of 'allow'
**  settings are done by activating links in that list.
**  The procedure uses a LYNXCOOKIE: internal URL scheme.
**
**  Semantics:
**	LYNXCOOKIE:/			Create and load the Cookie Jar Page.
**	LYNXCOOKIE://domain		Manipulate the domain.
**	LYNXCOOKIE://domain/lynx_id	Delete cookie with lynx_id in domain.
**
**	New functions can be added as extensions to the path, and/or by
**	assigning meanings to ;parameters, a ?searchpart, and/or #fragments.
*/
PRIVATE int LYHandleCookies ARGS4 (
	CONST char *, 		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
    HTFormat format_in = WWW_HTML;
    HTStream *target = NULL;
    char buf[1024];
    char *domain = NULL;
    char *lynx_id = NULL;
    HTList *dl, *cl, *next;
    domain_entry *de;
    cookie *co;
    char *name = NULL, *value = NULL, *path = NULL, *comment = NULL;
    int ch;
#ifdef VMS
    extern BOOLEAN HadVMSInterrupt;
#endif /* VMS */

    /*
     *  Check whether we have something to do. - FM
     */
    if (domain_list == NULL) {
	HTProgress(COOKIE_JAR_IS_EMPTY);
	sleep(MessageSecs);
	return(HT_NO_DATA);
    }

    /*
     *  If there's a domain string in the "host" field of the
     *  LYNXCOOKIE: URL, this is a request to delete something
     *  or change and 'allow' setting. - FM
     */
    if ((domain = HTParse(arg, "", PARSE_HOST)) != NULL) {
        if (*domain == '\0') {
	    FREE(domain);
	} else {
	    /*
	     *  If there is a path string (not just a slash) in the
	     *  LYNXCOOKIE: URL, that's a cookie's lynx_id and this
	     *  is a request to delete it from the Cookie Jar. - FM
	     */ 
	    if ((lynx_id = HTParse(arg, "", PARSE_PATH)) != NULL) {
	        if (*lynx_id == '\0') {
		    FREE(lynx_id);
		}
	    }
	}
    }
    if (domain) {
        /*
	 *  Seek the domain in the domain_list structure. - FM
	 */
	for (dl = domain_list; dl != NULL; dl = dl->next) {
	    de = dl->object;
	    if (!(de && de->domain))
	        /*
		 *  First object in the list always is empty. - FM
		 */
		continue;
	    if (!strcmp(domain, de->domain)) {
	        /*
		 *  We found the domain.  Check
		 *  whether a lynx_id is present. - FM
		 */
	        if (lynx_id) {
		    /*
		     *  Seek and delete the cookie with this lynx_id
		     *  in the domain's cookie list. - FM
		     */
		    for (cl = de->cookie_list; cl != NULL; cl = cl->next) {
		        if ((co = (cookie *)cl->object) == NULL)
			    /*
			     *  First object is always empty. - FM
			     */
			    continue;
			if (!strcmp(lynx_id, co->lynx_id)) {
			    /*
			     *  We found the cookie.
			     *  Delete it if confirmed. - FM
			     */
			    if (HTConfirm(DELETE_COOKIE_CONFIRMATION) == FALSE)
			        return(HT_NO_DATA);
			    HTList_removeObject(de->cookie_list, co);
			    freeCookie(co);
			    co = NULL;
			    total_cookies--;
			    if ((de->bv == QUERY_USER &&
			         HTList_isEmpty(de->cookie_list)) &&
				HTConfirm(DELETE_EMPTY_DOMAIN_CONFIRMATION)) {
			        /*
				 *  No more cookies in this domain, no
				 *  default accept/reject choice was set
				 *  by the user, and got confirmation on
				 *  deleting the domain, so do it. - FM
				 */
				FREE(de->domain);
				HTList_delete(de->cookie_list);
				de->cookie_list = NULL;
				HTList_removeObject(domain_list, de);
				de = NULL;
				HTProgress(DOMAIN_EATEN);
			    } else {
			        HTProgress(COOKIE_EATEN);
			    }
			    sleep(MessageSecs);
			    break;
			}
		    }
		} else {
		    /*
		     *  Prompt whether to delete all of the cookies in
		     *  this domain, or the domain if no cookies in it,
		     *  or to change its 'allow' setting, or to cancel,
		     *  and then act on the user's response. - FM
		     */
		    if (HTList_isEmpty(de->cookie_list)) {
			_statusline(DELETE_DOMAIN_SET_ALLOW_OR_CANCEL);
		    } else {
			_statusline(DELETE_COOKIES_SET_ALLOW_OR_CANCEL);
		    }
		    while (1) {
			ch = LYgetch();
#ifdef VMS
			if (HadVMSInterrupt) {
			    HadVMSInterrupt = FALSE;
			    ch = 'C';
			}
#endif /* VMS */
			switch(TOUPPER(ch)) {
			    case 'A':
			        /*
				 *  Set to accept all cookies 
				 *  from this domain. - FM
				 */
				de->bv = QUERY_USER;
				_user_message(ALWAYS_ALLOWING_COOKIES,
					      de->domain);
				sleep(MessageSecs);
				return(HT_NO_DATA);

			    case 'C':
			    case 7:	/* Ctrl-G */
			    case 3:	/* Ctrl-C */
			        /*
				 *  Cancelled. - FM
				 */
				_statusline(CANCELLED);
				sleep(MessageSecs);
				return(HT_NO_DATA);

			    case 'D':
			        if (HTList_isEmpty(de->cookie_list)) {
				    /*
				     *  We had an empty domain, so we
				     *  were asked to delete it. - FM
				     */
				    FREE(de->domain);
				    HTList_delete(de->cookie_list);
				    de->cookie_list = NULL;
				    HTList_removeObject(domain_list, de);
				    de = NULL;
				    HTProgress(DOMAIN_EATEN);
				    sleep(MessageSecs);
				    break;
				}
Delete_all_cookies_in_domain:
				/*
				 *  Delete all cookies in this domain. - FM
				 */
				cl = de->cookie_list;
				while (cl) {
				    next = cl->next;
				    co = cl->object;
				    if (co) {
				        HTList_removeObject(de->cookie_list,
							    co);
					freeCookie(co);
					co = NULL;
					total_cookies--;
				    }
				    cl = next;
				}
				HTProgress(DOMAIN_COOKIES_EATEN);
				sleep(MessageSecs);
				/*
				 *  If a default accept/reject
				 *  choice is set, we're done. - FM
				 */
				if (de->bv != QUERY_USER)
				    return(HT_NO_DATA);
				/*
				 *  Check whether to delete
				 *  the empty domain. - FM
				 */
				if(HTConfirm(
				    	DELETE_EMPTY_DOMAIN_CONFIRMATION)) {
				    FREE(de->domain);
				    HTList_delete(de->cookie_list);
				    de->cookie_list = NULL;
				    HTList_removeObject(domain_list, de);
				    de = NULL;
				    HTProgress(DOMAIN_EATEN);
				    sleep(MessageSecs);
				}
				break;

			    case 'P':
			        /*
				 *  Set to prompt for cookie acceptence
				 *  from this domain. - FM
				 */
				de->bv = QUERY_USER;
				_user_message(PROMTING_TO_ALLOW_COOKIES,
					      de->domain);
				sleep(MessageSecs);
				return(HT_NO_DATA);

		    	    case 'V':
			        /*
				 *  Set to reject all cookies
				 *  from this domain. - FM
				 */
				de->bv = REJECT_ALWAYS;
				_user_message(NEVER_ALLOWING_COOKIES,
					      de->domain);
				sleep(MessageSecs);
				if ((!HTList_isEmpty(de->cookie_list)) &&
				    HTConfirm(DELETE_ALL_COOKIES_IN_DOMAIN))
				    goto Delete_all_cookies_in_domain;
				return(HT_NO_DATA);

			    default:
			        continue;
			}
			break;
		    }
		}
		break;
	    }
	}
	if (HTList_isEmpty(domain_list)) {
	    /*
	     *  There are no more domains left,
	     *  so delete the domain_list. - FM
	     */
	    HTList_delete(domain_list);
	    domain_list = NULL;
	    HTProgress(ALL_COOKIES_EATEN);
	    sleep(MessageSecs);
	}
	return(HT_NO_DATA);
    }

    /*
     *  If we get to here, it was a LYNXCOOKIE:/ URL
     *  for creating and displaying the Cookie Jar Page,
     *  or we didn't find the domain or cookie in a
     *  deletion request.  Set up an HTML stream and
     *  return an updated Cookie Jar Page. - FM
     */
    target = HTStreamStack(format_in, 
			   format_out,
			   sink, anAnchor);
    if (!target || target == NULL) {
	sprintf(buf, CANNOT_CONVERT_I_TO_O,
		HTAtom_name(format_in), HTAtom_name(format_out));
	HTAlert(buf);
	return(HT_NOT_LOADED);
    }

    /*
     *  Load HTML strings into buf and pass buf
     *  to the target for parsing and rendering. - FM
     */
    sprintf(buf, "<HEAD>\n<TITLE>%s</title>\n</HEAD>\n<BODY>\n",
		 COOKIE_JAR_TITLE);
    (*target->isa->put_block)(target, buf, strlen(buf));

    sprintf(buf, "<H1>%s</H1>\n", REACHED_COOKIE_JAR_PAGE);
    (*target->isa->put_block)(target, buf, strlen(buf));
    sprintf(buf, "<H2>%s Version %s</H2>\n", LYNX_NAME, LYNX_VERSION);
    (*target->isa->put_block)(target, buf, strlen(buf));

    sprintf(buf, "<NOTE>%s\n", ACTIVATE_TO_GOBBLE);
    (*target->isa->put_block)(target, buf, strlen(buf));
    sprintf(buf, "%s</NOTE>\n", OR_CHANGE_ALLOW);
    (*target->isa->put_block)(target, buf, strlen(buf));

    sprintf(buf, "<DL COMPACT>\n");
    (*target->isa->put_block)(target, buf, strlen(buf));
    for (dl = domain_list; dl != NULL; dl = dl->next) {
	de = dl->object;
	if (de == NULL)
	    /*
	     *  First object always is NULL. - FM
	     */
	    continue;

	/*
	 *  Show the domain link and 'allow' setting. - FM
	 */
	sprintf(buf, "<DT><A HREF=\"LYNXCOOKIE://%s/\">Domain=%s</A>\n",
		      de->domain, de->domain);
	(*target->isa->put_block)(target, buf, strlen(buf));
	switch (de->bv) {
	    case (ACCEPT_ALWAYS):
		sprintf(buf, COOKIES_ALWAYS_ALLOWED);
		break;
	    case (REJECT_ALWAYS):
		sprintf(buf, COOKIES_NEVER_ALLOWED);
		break;
	    case (QUERY_USER):
		sprintf(buf, COOKIES_ALLOWED_VIA_PROMPT);
	    break;
	}
	(*target->isa->put_block)(target, buf, strlen(buf));

	/*
	 *  Show the domain's cookies. - FM
	 */
	for (cl = de->cookie_list; cl != NULL; cl = cl->next) {
	    if ((co = (cookie *)cl->object) == NULL)
		/*
		 *  First object is always NULL. - FM
		 */
	        continue;

	    /*
	     *  Show the name=value pair. - FM
	     */
	    if (co->name) {
	        StrAllocCopy(name, co->name);
		LYEntify(&name, TRUE);
	    } else {
	        StrAllocCopy(name, NO_NAME);
	    }
	    if (co->value) {
	        StrAllocCopy(value, co->value);
		LYEntify(&value, TRUE);
	    } else {
	        StrAllocCopy(value, NO_VALUE);
	    }
	    sprintf(buf, "<DD><A HREF=\"LYNXCOOKIE://%s/%s\">%s=%s</A>\n",
			 de->domain, co->lynx_id, name, value);
	    FREE(name);
	    FREE(value);
	    (*target->isa->put_block)(target, buf, strlen(buf));

	    /*
	     *  Show the path, port, and secure setting. - FM
	     */
 	    if (co->path) {
	        StrAllocCopy(path, co->path);
		LYEntify(&path, TRUE);
	    } else {
	        StrAllocCopy(path, "/");
	    }
	    sprintf(buf, "<DD>Path=%s\n<DD>Port: %i Secure: %s\n",
			 path, co->port,
			 ((co->flags & COOKIE_FLAG_SECURE) ? "YES" : "NO"));
	    FREE(path);
	    (*target->isa->put_block)(target, buf, strlen(buf));

	    /*
	     *  Show the Maximum Gobble Date. - FM
	     */
	    sprintf(buf, "<DD><EM>Maximum Gobble Date:</EM> %s%s",
	    		 ((co->expires > 0) ?
			ctime(&co->expires) : END_OF_SESSION),
			 ((co->expires > 0) ?
				 	 "" : "\n"));
	    (*target->isa->put_block)(target, buf, strlen(buf));

	    /*
	     *  Show the comment, if we have one. - FM
	     */
 	    if (co->comment) {
	        StrAllocCopy(comment, co->comment);
		LYEntify(&comment, TRUE);
		sprintf(buf, "<DD><EM>Comment:</EM> %s\n", comment);
		FREE(comment);
		(*target->isa->put_block)(target, buf, strlen(buf));
	    }
	}
	sprintf(buf, "</DT>\n");
	(*target->isa->put_block)(target, buf, strlen(buf));
    }
    sprintf(buf, "</DL>\n</BODY>\n");
    (*target->isa->put_block)(target, buf, strlen(buf));

    /*
     *  Free the target to complete loading of the
     *  Cookie Jar Page, and report a successful load. - FM
     */
    (*target->isa->_free)(target);
    return(HT_LOADED);
}

#ifdef GLOBALDEF_IS_MACRO
#define _LYCOOKIE_C_GLOBALDEF_1_INIT { "LYNXCOOKIE",LYHandleCookies,0}
GLOBALDEF (HTProtocol,LYLynxCookies,_LYCOOKIE_C_GLOBALDEF_1_INIT);
#else
GLOBALDEF PUBLIC HTProtocol LYLynxCookies = {"LYNXCOOKIE",LYHandleCookies,0};
#endif /* GLOBALDEF_IS_MACRO */
