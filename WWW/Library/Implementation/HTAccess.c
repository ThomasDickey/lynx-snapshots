/*		Access Manager					HTAccess.c
**		==============
**
**  Authors
**	TBL	Tim Berners-Lee timbl@info.cern.ch
**	JFG	Jean-Francois Groff jfg@dxcern.cern.ch
**	DD	Denis DeLaRoca (310) 825-4580  <CSP1DWD@mvs.oac.ucla.edu>
**	FM	Foteos Macrides macrides@sci.wfeb.edu
**      PDM     Danny Mayer mayer@ljo.dec.com
**
**  History
**       8 Jun 92 Telnet hopping prohibited as telnet is not secure TBL
**	26 Jun 92 When over DECnet, suppressed FTP, Gopher and News. JFG
**	 6 Oct 92 Moved HTClientHost and logfile into here. TBL
**	17 Dec 92 Tn3270 added, bug fix. DD
**	 4 Feb 93 Access registration, Search escapes bad chars TBL
**		  PARAMETERS TO HTSEARCH AND HTLOADRELATIVE CHANGED
**	28 May 93 WAIS gateway explicit if no WAIS library linked in.
**	31 May 94 Added DIRECT_WAIS support for VMS. FM
**      27 Jan 95 Fixed proxy support to use NNTPSERVER for checking
**                whether or not to use the proxy server. PDM
**      27 Jan 95 Ensured that proxy service will be overridden for files
**		  on the local host (because HTLoadFile() doesn't try ftp
**		  for those) and will substitute ftp for remote files. FM
**      28 Jan 95 Tweeked PDM's proxy override mods to handle port info
**		  for news and wais URL's. FM
**
**  Bugs
**	This module assumes that that the graphic object is hypertext, as it
**	needs to select it when it has been loaded.  A superclass needs to be
**	defined which accepts select and select_anchor.
*/

#ifdef VMS
#define DIRECT_WAIS
#endif /* VMS */

#ifndef DEFAULT_WAIS_GATEWAY
#define DEFAULT_WAIS_GATEWAY "http://www.w3.org:8001"
#endif

#include "HTUtils.h"
#include "HTAlert.h"
/*
**  Implements:
*/
#include "HTAccess.h"

/*
**  Uses:
*/
#include "HTParse.h"
#include "HTML.h"		/* SCW */

#ifndef NO_RULES
#include "HTRules.h"
#endif

#include "HTList.h"
#include "HText.h"	/* See bugs above */
#include "HTAlert.h"
#include "HTCJK.h"

#include "LYexit.h"
#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

extern HTCJKlang HTCJK;

/*
**  These flags may be set to modify the operation of this module
*/
PUBLIC char * HTClientHost = NULL; /* Name of remote login host if any */
PUBLIC FILE * HTlogfile = NULL;	   /* File to which to output one-liners */
PUBLIC BOOL HTSecure = NO;	   /* Disable access for telnet users? */

PUBLIC BOOL using_proxy = NO; /* are we using a proxy gateway? */

/*
**  To generate other things, play with these:
*/
PUBLIC HTFormat HTOutputFormat = NULL;
PUBLIC HTStream* HTOutputStream = NULL;	/* For non-interactive, set this */ 

PRIVATE HTList * protocols = NULL; /* List of registered protocol descriptors */

PUBLIC char *use_this_url_instead = NULL;


PRIVATE void free_protocols NOARGS
{
    HTList_delete(protocols);
    protocols = NULL;
}

/*	Register a Protocol.				HTRegisterProtocol()
**	--------------------
*/
PUBLIC BOOL HTRegisterProtocol ARGS1(
	HTProtocol *,	protocol)
{
    if (!protocols) {
        protocols = HTList_new();
	atexit(free_protocols);
    }
    HTList_addObject(protocols, protocol);
    return YES;
}


/*	Register all known protocols.			HTAccessInit()
**	-----------------------------
**
**	Add to or subtract from this list if you add or remove protocol
**	modules. This routine is called the first time the protocol list
**	is needed, unless any protocols are already registered, in which
**	case it is not called.  Therefore the application can override
**	this list.
**
**	Compiling with NO_INIT prevents all known protocols from being
**	forced in at link time.
*/
#ifndef NO_INIT
PRIVATE void HTAccessInit NOARGS			/* Call me once */
{
#ifdef GLOBALREF_IS_MACRO
extern GLOBALREF (HTProtocol, HTTP);
extern GLOBALREF (HTProtocol, HTTPS);
extern GLOBALREF (HTProtocol, HTFile);
extern GLOBALREF (HTProtocol, HTTelnet);
extern GLOBALREF (HTProtocol, HTTn3270);
extern GLOBALREF (HTProtocol, HTRlogin);
#ifndef DECNET
extern GLOBALREF (HTProtocol, HTFTP);
extern GLOBALREF (HTProtocol, HTNews);
extern GLOBALREF (HTProtocol, HTNNTP);
extern GLOBALREF (HTProtocol, HTNewsPost);
extern GLOBALREF (HTProtocol, HTNewsReply);
extern GLOBALREF (HTProtocol, HTSNews);
extern GLOBALREF (HTProtocol, HTSNewsPost);
extern GLOBALREF (HTProtocol, HTSNewsReply);
extern GLOBALREF (HTProtocol, HTGopher);
extern GLOBALREF (HTProtocol, HTCSO);
extern GLOBALREF (HTProtocol, HTFinger);
#ifdef DIRECT_WAIS
extern GLOBALREF (HTProtocol, HTWAIS);
#endif /* DIRECT_WAIS */
#endif /* !DECNET */
#else
GLOBALREF HTProtocol HTTP, HTTPS, HTFile, HTTelnet, HTTn3270, HTRlogin;
#ifndef DECNET
GLOBALREF HTProtocol HTFTP, HTNews, HTNNTP, HTNewsPost, HTNewsReply;
GLOBALREF HTProtocol HTSNews, HTSNewsPost, HTSNewsReply;
GLOBALREF HTProtocol HTGopher, HTCSO, HTFinger;
#ifdef DIRECT_WAIS
GLOBALREF  HTProtocol HTWAIS;
#endif /* DIRECT_WAIS */
#endif /* !DECNET */
#endif /* GLOBALREF_IS_MACRO */
    HTRegisterProtocol(&HTTP);
    HTRegisterProtocol(&HTTPS);
    HTRegisterProtocol(&HTFile);
    HTRegisterProtocol(&HTTelnet);
    HTRegisterProtocol(&HTTn3270);
    HTRegisterProtocol(&HTRlogin);
#ifndef DECNET
    HTRegisterProtocol(&HTFTP);
    HTRegisterProtocol(&HTNews);
    HTRegisterProtocol(&HTNNTP);
    HTRegisterProtocol(&HTNewsPost);
    HTRegisterProtocol(&HTNewsReply);
    HTRegisterProtocol(&HTSNews);
    HTRegisterProtocol(&HTSNewsPost);
    HTRegisterProtocol(&HTSNewsReply);
    HTRegisterProtocol(&HTGopher);
    HTRegisterProtocol(&HTCSO);
    HTRegisterProtocol(&HTFinger);
#ifdef DIRECT_WAIS
    HTRegisterProtocol(&HTWAIS);
#endif /* DIRECT_WAIS */
#endif /* !DECNET */
    LYRegisterLynxProtocols();
}
#endif /* !NO_INIT */

/*	Check for proxy override.			override_proxy()
**	-------------------------
**
**	Check the no_proxy environment variable to get the list
**	of hosts for which proxy server is not consulted.
**
**	no_proxy is a comma- or space-separated list of machine
**	or domain names, with optional :port part.  If no :port
**	part is present, it applies to all ports on that domain.
**
**  Example:
**          no_proxy="cern.ch,some.domain:8001"
**
**  Use "*" to override all proxy service:
**	     no_proxy="*"
*/
PUBLIC BOOL override_proxy ARGS1(
	CONST char *,	addr)
{
    CONST char * no_proxy = getenv("no_proxy");
    char * p = NULL;
    char * at = NULL;
    char * host = NULL;
    char * Host = NULL;
    char * access = NULL;
    int port = 0;
    int h_len = 0;

    /*
     * Check for global override.
     */
    if (no_proxy) {
        if (!strcmp(no_proxy, "*"))
            return YES;
    }

    /*
     *  Never proxy file:// URLs if they are on the local host.
     *  HTLoadFile() will not attempt ftp for those if direct
     *  access fails.  We'll check that first, in case no_proxy
     *  hasn't been defined. - FM
     */
    if (!addr)
        return NO;
    if (!(host = HTParse(addr, "", PARSE_HOST)))
        return NO;
    if (!*host) {
        FREE(host);
	return NO;
    }
    Host = (((at = strchr(host, '@')) != NULL) ? (at+1) : host);

    if ((access = HTParse(addr, "", PARSE_ACCESS))) {
        if (!strcmp("file", access) &&
	    (!strcmp(Host, "localhost") ||
#ifdef VMS
             !strcasecomp(Host, HTHostName())))
#else
             !strcmp(Host, HTHostName())))
#endif /* VMS */
        {
	    FREE(host);
	    FREE(access);
	    return YES;
	}
	FREE(access);
    }

    if (!no_proxy) {
        FREE(host);
        return NO;
    }

    if (NULL != (p = strrchr(Host, ':'))) {	/* Port specified */
        *p++ = 0;                   		/* Chop off port */
        port = atoi(p);
    } else {					/* Use default port */
        access = HTParse(addr, "", PARSE_ACCESS);
        if (access != NULL) {
            if      (!strcmp(access, "http"))		port = 80;
            else if (!strcmp(access, "https"))		port = 443;
            else if (!strcmp(access, "ftp"))		port = 21;
            else if (!strcmp(access, "gopher"))		port = 70;
            else if (!strcmp(access, "cso"))		port = 105;
	    else if (!strcmp(access, "news"))		port = 119;
	    else if (!strcmp(access, "nntp"))		port = 119;
	    else if (!strcmp(access, "newspost"))	port = 119;
	    else if (!strcmp(access, "newsreply"))	port = 119;
	    else if (!strcmp(access, "snews"))		port = 563;
	    else if (!strcmp(access, "snewspost"))	port = 563;
	    else if (!strcmp(access, "snewsreply"))	port = 563;
	    else if (!strcmp(access, "wais"))		port = 210;
	    else if (!strcmp(access, "finger"))		port = 79;
            FREE(access);
        }
    }
    if (!port)
        port = 80;                  /* Default */
    h_len = strlen(Host);

    while (*no_proxy) {
        CONST char * end;
        CONST char * colon = NULL;
        int templ_port = 0;
        int t_len;

        while (*no_proxy && (WHITE(*no_proxy) || *no_proxy == ','))
            no_proxy++;             /* Skip whitespace and separators */

        end = no_proxy;
        while (*end && !WHITE(*end) && *end != ',') {	/* Find separator */
            if (*end == ':') colon = end;		/* Port number given */
            end++;
        }

        if (colon) {
            templ_port = atoi(colon+1);
            t_len = colon - no_proxy;
        }
        else {
            t_len = end - no_proxy;
        }

        if ((!templ_port || templ_port == port)  &&
            (t_len > 0  &&  t_len <= h_len  &&
             !strncmp(Host + h_len - t_len, no_proxy, t_len))) {
            FREE(host);
            return YES;
        }
        if (*end)
	    no_proxy = (end + 1);
        else
	    break;
    }

    FREE(host);
    return NO;
}

/*	Find physical name and access protocol		get_physical()
**	--------------------------------------
**
**  On entry,
**	addr		must point to the fully qualified hypertext reference.
**	anchor		a pareent anchor with whose address is addr
**
**  On exit,
**	returns		HT_NO_ACCESS		Error has occured.
**			HT_OK			Success
*/
PRIVATE int get_physical ARGS2(
	CONST char *,		addr,
	HTParentAnchor *,	anchor)
{
    char * access = NULL;	/* Name of access method */
    char * physical = NULL;
    char * Server_addr = NULL;

#ifndef NO_RULES
    physical = HTTranslate(addr);
    if (!physical) {
	return HT_FORBIDDEN;
    }
    if (anchor->isISMAPScript == TRUE) {
        StrAllocCat(physical, "?0,0");
	if (TRACE)
	    fprintf(stderr, "HTAccess: Appending '?0,0' coordinate pair.\n");
    }
    HTAnchor_setPhysical(anchor, physical);
    FREE(physical);			/* free our copy */
#else
    if (anchor->isISMAPScript == TRUE) {
        StrAllocCopy(physical, addr);
	StrAllocCat(physical, "?0,0");
	if (TRACE)
	    fprintf(stderr, "HTAccess: Appending '?0,0' coordinate pair.\n");
	HTAnchor_setPhysical(anchor, physical);
	FREE(physical);			/* free our copy */
    } else {
        HTAnchor_setPhysical(anchor, addr);
    }
#endif /* NO_RULES */

    access =  HTParse(HTAnchor_physical(anchor),
    		"file:", PARSE_ACCESS);

    /*
    **  Check whether gateway access has been set up for this.
    **
    **  This function can be replaced by the rule system above.
    */
#define USE_GATEWAYS
#ifdef USE_GATEWAYS
    /*
    **	Make sure the using_proxy variable is FALSE.
    */
    using_proxy = NO;

    if (!strcasecomp(access, "news")) {
	/*
	**  News is different, so we need to check the name of the server,
	**  as well as the default port for selective exclusions.
	*/
	char *host = NULL;
	if ((host = HTParse(addr, "", PARSE_HOST))) {
	    if (strchr(host, ':') == NULL) {
	        StrAllocCopy(Server_addr, "news://");
	        StrAllocCat(Server_addr, host);
		StrAllocCat(Server_addr, ":119/");
	    }
	    FREE(host);
        } else if (getenv("NNTPSERVER") != NULL) {
            StrAllocCopy(Server_addr, "news://");
            StrAllocCat(Server_addr, (char *)getenv("NNTPSERVER"));
            StrAllocCat(Server_addr, ":119/");
         }
    } else if (!strcasecomp(access, "wais")) {
    	/*
	**  Wais also needs checking of the default port
	**  for selective exclusions.
	*/
	char *host = NULL;
	if ((host = HTParse(addr, "", PARSE_HOST))) {
	    if (!(strchr(host, ':'))) {
	        StrAllocCopy(Server_addr, "wais://");
	        StrAllocCat(Server_addr, host);
		StrAllocCat(Server_addr, ":210/");
	    }
	    FREE(host);
	}
	else
            StrAllocCopy(Server_addr, addr);
    } else {
        StrAllocCopy(Server_addr, addr);
    }

    if (!override_proxy(Server_addr)) {
	char * gateway_parameter, *gateway, *proxy;

	/*
	**  Search for gateways.
	*/
	gateway_parameter = (char *)malloc(strlen(access) + 20);
	if (gateway_parameter == NULL)
	    outofmem(__FILE__, "HTLoad");
	strcpy(gateway_parameter, "WWW_");
	strcat(gateway_parameter, access);
	strcat(gateway_parameter, "_GATEWAY");
	gateway = (char *)getenv(gateway_parameter); /* coerce for decstation */

	/*
	**  Search for proxy servers.
	*/
	if (!strcmp(access, "file"))
	    /*
	    ** If we got to here, a file URL is for ftp on a remote host. - FM
	    */
	    strcpy(gateway_parameter, "ftp");
	else
	    strcpy(gateway_parameter, access);
        strcat(gateway_parameter, "_proxy");
	proxy = (char *)getenv(gateway_parameter);
	FREE(gateway_parameter);

	if (TRACE && gateway)
	    fprintf(stderr, "Gateway found: %s\n", gateway);
	if (TRACE && proxy)
	    fprintf(stderr, "proxy server found: %s\n", proxy);
	
#ifndef DIRECT_WAIS
	if (gateway == NULL && !strcmp(access, "wais")) {
	    gateway = DEFAULT_WAIS_GATEWAY;
	}
#endif /* direct wais */

	/*
	**  Proxy servers have precedence over gateway servers.
	*/
	if (proxy) {
	    char * gatewayed = NULL;
            StrAllocCopy(gatewayed,proxy);
	    /*
	    ** Ensure that the proxy server uses ftp for file URLs. - FM
	    */
	    if (!strncmp(addr, "file", 4)) {
                StrAllocCat(gatewayed, "ftp");
                StrAllocCat(gatewayed, (addr + 4));
	    } else
                StrAllocCat(gatewayed, addr);
            using_proxy = YES;
	    if (anchor->isISMAPScript == TRUE)
	        StrAllocCat(gatewayed, "?0,0");
            HTAnchor_setPhysical(anchor, gatewayed);
	    FREE(gatewayed);
	    FREE(access);

    	    access =  HTParse(HTAnchor_physical(anchor),
    		"http:", PARSE_ACCESS);

	} else if (gateway) {
	    char * path = HTParse(addr, "",
	    	PARSE_HOST + PARSE_PATH + PARSE_PUNCTUATION);
		/* Chop leading / off to make host into part of path */
	    char * gatewayed = HTParse(path+1, gateway, PARSE_ALL);
	    FREE(path);
            HTAnchor_setPhysical(anchor, gatewayed);
	    FREE(gatewayed);
	    FREE(access);
	    
    	    access =  HTParse(HTAnchor_physical(anchor),
    		"http:", PARSE_ACCESS);
	} 
    }
    FREE(Server_addr);
#endif /* use gateways */

    /*
    **  Search registered protocols to find suitable one.
    */
    {
	int i, n;
#ifndef NO_INIT
        if (!protocols) HTAccessInit();
#endif
	n = HTList_count(protocols);
	for (i = 0; i < n; i++) {
	    HTProtocol *p = (HTProtocol *)HTList_objectAt(protocols, i);
	    if (!strcmp(p->name, access)) {
		HTAnchor_setProtocol(anchor, p);
		FREE(access);
		return (HT_OK);
	    }
	}
    }

    FREE(access);
    return HT_NO_ACCESS;
}

/*	Load a document					HTLoad()
**	---------------
**
**	This is an internal routine, which has an address AND a matching
**	anchor.  (The public routines are called with one OR the other.)
**
**  On entry,
**	addr		must point to the fully qualified hypertext reference.
**	anchor		a pareent anchor with whose address is addr
**
**  On exit,
**	returns		<0		Error has occured.
**			HT_LOADED	Success
**			HT_NO_DATA	Success, but no document loaded.
**					(telnet sesssion started etc)
*/
PRIVATE int HTLoad ARGS4(
	CONST char *,		addr,
	HTParentAnchor *,	anchor,
	HTFormat,		format_out,
	HTStream *,		sink)
{
    HTProtocol *p;
    int status = get_physical(addr, anchor);
    if (status == HT_FORBIDDEN) {
        return HTLoadError(sink, 500, "Access forbidden by rule");
    }
    if (status < 0)
        return status;	/* Can't resolve or forbidden */
    
    p = (HTProtocol *)HTAnchor_protocol(anchor);
    anchor->underway = TRUE;            /* Hack to deal with caching */
    status= (*(p->load))(HTAnchor_physical(anchor),
    			anchor, format_out, sink);
    anchor->underway = FALSE;
    return status;
}

/*	Get a save stream for a document		HTSaveStream()
**	--------------------------------
*/
PUBLIC HTStream *HTSaveStream ARGS1(
	HTParentAnchor *,	anchor)
{
    HTProtocol *p = (HTProtocol *)HTAnchor_protocol(anchor);
    if (!p)
        return NULL;
    
    return (*p->saveStream)(anchor);
}

/*	Load a document - with logging etc		HTLoadDocument()
**	----------------------------------
**
**	- Checks or documents already loaded
**	- Logs the access
**	- Allows stdin filter option
**	- Trace ouput and error messages
**
**  On Entry,
**	  anchor	    is the node_anchor for the document
**        full_address      The address of the document to be accessed.
**        filter            if YES, treat stdin as HTML
**
**  On Exit,
**        returns    YES     Success in opening document
**                   NO      Failure 
*/
PRIVATE BOOL HTLoadDocument ARGS4(
	CONST char *,		full_address,
	HTParentAnchor *,	anchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
    int	        status;
    HText *	text;
    char * address_to_load =  (char *)full_address;
    extern char LYforce_no_cache;		       /* from   GridText.c */
    extern char LYoverride_no_cache;		       /* from LYMainLoop.c */
    extern BOOL HText_hasNoCacheSet PARAMS((HText *text)); /* in GridText.c */
    extern BOOL reloading;
    extern char *redirecting_url;
    extern BOOL permanent_redirection;
    char *cp;
    BOOL ForcingNoCache = LYforce_no_cache;
    static int redirection_attempts = 0;

#ifdef DIRED_SUPPORT
    extern BOOLEAN lynx_edit_mode;
#endif

    if (TRACE)
        fprintf (stderr, "HTAccess: loading document %s\n", address_to_load);

    /*
    **  Free use_this_url_instead and reset permanent_redirection
    **  if not done elsewhere. - FM
    */
    FREE(use_this_url_instead);
    permanent_redirection = FALSE;

    /*
    **  Make sure some yoyo doesn't send us 'round in circles
    **  with redirecting URLs that point back to themselves.
    **  We'll set the HTTP/1.1 limit of 5 redirections per
    **  requested URL from a user.  - FM
    */
    if (redirection_attempts > 5) {
        redirection_attempts = 0;
	HTAlert("Redirection limit of 5 URL's reached.");
        return NO;
    }

    /*
    **  If we don't have POST content, check whether this is a previous
    **  redirecting URL, and keep re-checking until we get to the final
    **  destination or redirection limit.  If we do have POST content,
    **  we didn't allow permanent redirection, and an interactive user
    **  will be deciding whether to keep redirecting. - FM
    */
    if (!anchor->post_data) {
        while ((cp = HTAnchor_physical(anchor)) != NULL &&
	       !strncmp(cp, "Location=", 9)) {
	    DocAddress NewDoc;

	    if (TRACE) {
		fprintf (stderr, "HTAccess: '%s' is a redirection URL.\n",
				  anchor->address);
		fprintf (stderr, "HTAccess: Redirecting to '%s'\n", cp+9);
	    }

	    /*
	    **  Don't exceed the redirection_attempts limit. - FM
	    */
	    if (++redirection_attempts > 5) {
		HTAlert("Redirection limit of 5 URL's reached.");
		redirection_attempts = 0;
		FREE(use_this_url_instead);
 		return NO;
	    }

	    /*
	    ** Set up the redirection. - FM
	    **/
	    StrAllocCopy(use_this_url_instead, cp+9);
	    NewDoc.address = use_this_url_instead;
	    NewDoc.post_data = NULL;
	    NewDoc.post_content_type = NULL;
	    NewDoc.bookmark = anchor->bookmark;
	    NewDoc.isHEAD = anchor->isHEAD;
	    NewDoc.safe = anchor->safe;
	    anchor = (HTParentAnchor *)HTAnchor_findAddress(&NewDoc);
	}
    }
    /*
    **  If we had previous redirection, go back and check out
    **  that the URL under the current restrictions. - FM
    */
    if (use_this_url_instead) {
	FREE(redirecting_url);
        return(NO);
    }

    /*
    **  See if we can use an already loaded document.
    */
    if (!LYforce_no_cache && (text = (HText *)HTAnchor_document(anchor))) {	
	/*
	**  Already loaded.  Check it it's OK to use it. - FM
	*/
	if (LYoverride_no_cache || !HText_hasNoCacheSet(text)) {
            if (TRACE)
	        fprintf(stderr, "HTAccess: Document already in memory.\n");
            HText_select(text);

#ifdef DIRED_SUPPORT
	    if (HTAnchor_format(anchor) == WWW_DIRED)
	        lynx_edit_mode = TRUE;
#endif
	    redirection_attempts = 0;
	    return YES;
	} else {
	    reloading = TRUE;
	    ForcingNoCache = YES;
            if (TRACE)
	        fprintf(stderr, "HTAccess: Auto-reloading document.\n");
	}
    }

    /*
    **  Get the document from the net.  If we are auto-reloading,
    **  the previous rendition will be freed in conjunction with
    **  loading of the new rendition. - FM
    */
    LYforce_no_cache = NO;  /* reset after each time through */
    status = HTLoad(address_to_load, anchor, format_out, sink);
    if (TRACE) {
	fprintf(stderr, "HTAccess:  status=%d\n", status);
    }
    
    /*
    **  Log the access if necessary.
    */
    if (HTlogfile) {
	time_t theTime;
	time(&theTime);
	fprintf(HTlogfile, "%24.24s %s %s %s\n",
		ctime(&theTime),
		HTClientHost ? HTClientHost : "local",
		status < 0 ? "FAIL" : "GET",
		full_address);
	fflush(HTlogfile);	/* Actually update it on disk */
	if (TRACE)
	    fprintf(stderr, "Log: %24.24s %s %s %s\n",
		    ctime(&theTime),
		    HTClientHost ? HTClientHost : "local",
		    status < 0 ? "FAIL" : "GET",
		    full_address);
    }
    
    /*
    **  Check out what we received from the net.
    */
    if (status == HT_REDIRECTING) {
        /*  Exported from HTMIME.c, of all places. *//** NO!! - FM **/
	/*
	**  Doing this via HTMIME.c meant that the redirection cover
	**  page was already loaded before we learned that we want a
	**  different URL.  Also, changing anchor->address, as Lynx
	**  was doing, meant we could never again access its hash
	**  table entry, creating an insolvable memory leak.  Instead,
	**  if we had a 301 status and set permanent_redirection,
	**  we'll load the new URL in anchor->physical, preceded by a
	**  token, which we can check to make replacements on subsequent
	**  access attempts.  We'll check recursively, and retrieve the
	**  final URL if we had multiple redirections to it.  If we just
	**  went to HTLoad now, as Lou originally had this, we couldn't do
	**  Lynx's security checks and alternate handling of some URL types.
	**  So, instead, we'll go all the way back to the top of getfile
	**  in LYGetFile.c when the status is HT_REDIRECTING.  This may
	**  seem bizarre, but it works like a charm! - FM
	*/
        if (TRACE) {
            fprintf(stderr, "HTAccess: '%s' is a redirection URL.\n",
			    address_to_load);
            fprintf(stderr, "HTAccess: Redirecting to '%s'\n",
			     redirecting_url);
        }
	/*
	**  Prevent circular references. 
	*/
	if (strcmp(address_to_load, redirecting_url)) { /* if different */
	    /*
	    **  Load token and redirecting url into anchor->physical
	    **  if we had 301 Permanent redirection.  HTTP.c does not
	    **  allow this if we have POST content. - FM
	    */
	    if (permanent_redirection) {
	        StrAllocCopy(anchor->physical, "Location=");
	        StrAllocCat(anchor->physical, redirecting_url);
	    }

            /*
	    **  Set up flags before return to getfile. - FM
	    */
            StrAllocCopy(use_this_url_instead, redirecting_url);
	    if (ForcingNoCache)
	        LYforce_no_cache = YES;
	    ++redirection_attempts;
	    FREE(redirecting_url);
	    permanent_redirection = FALSE;
	    return(NO);
	}
	++redirection_attempts;
	FREE(redirecting_url);
	permanent_redirection = FALSE;
	return(YES);
    }

    /*
    **  We did not receive a redirecting URL. - FM
    */
    redirection_attempts = 0;
    FREE(redirecting_url);
    permanent_redirection = FALSE;

    if (status == HT_LOADED) {
	if (TRACE) {
	    fprintf(stderr, "HTAccess: `%s' has been accessed.\n",
	    full_address);
	}
	return YES;
    }

    if (status == HT_NO_DATA) {
	if (TRACE) {
	    fprintf(stderr, 
	    "HTAccess: `%s' has been accessed, No data left.\n",
	    full_address);
	}
	return NO;
    }
    
    if (status == HT_NOT_LOADED) {
	if (TRACE) {
	    fprintf(stderr, 
	    "HTAccess: `%s' has been accessed, No data loaded.\n",
	    full_address);
	}
	return NO;
    }
    
    if (status == HT_INTERRUPTED) {
	if (TRACE) {
	    fprintf(stderr, 
	    "HTAccess: `%s' has been accessed, transfer interrupted.\n",
	    full_address);
	}
/*	_HTProgress("Data transfer interrupted."); */
	return NO;
    }

    if (status <= 0) {		/* Failure in accessing a document */
	char *temp = NULL;
	StrAllocCopy(temp, "Can't Access `");
	StrAllocCat(temp, full_address);
	StrAllocCat(temp, "'");
	_HTProgress(temp);
	FREE(temp);
	if (TRACE) fprintf(stderr, 
		"HTAccess: Can't access `%s'\n", full_address);
	HTLoadError(sink, 500, "Unable to access document.");
	return NO;
    }
 
    /*
    **  If you get this, then please find which routine is returning
    **  a positive unrecognised error code!
    */
    fprintf(stderr,
 "**** HTAccess: socket or file number returned by obsolete load routine!\n");
    fprintf(stderr,
 "**** HTAccess: Internal software error. Please mail lynx_dev@sig.net!\n");
    fprintf(stderr, "**** HTAccess: Status returned was: %d\n",status);
    exit(-1);

} /* HTLoadDocument */

/*	Load a document from absolute name.		HTLoadAbsolute()
**	-----------------------------------
**
**  On Entry,
**        addr     The absolute address of the document to be accessed.
**        filter   if YES, treat document as HTML
**
**  On Exit,
**        returns    YES     Success in opening document
**                   NO      Failure 
*/
PUBLIC BOOL HTLoadAbsolute ARGS1(
	CONST DocAddress *,	docaddr)
{
    return HTLoadDocument(docaddr->address,
			  HTAnchor_parent(HTAnchor_findAddress(docaddr)),
			  (HTOutputFormat ? HTOutputFormat : WWW_PRESENT),
			  HTOutputStream);
}

#ifdef NOT_USED_CODE
/*	Load a document from absolute name to stream.	HTLoadToStream()
**	---------------------------------------------
**
**  On Entry,
**        addr     The absolute address of the document to be accessed.
**        sink     if non-NULL, send data down this stream
**
**  On Exit,
**        returns    YES     Success in opening document
**                   NO      Failure 
*/
PUBLIC BOOL HTLoadToStream ARGS3(
	CONST char *,	addr,
	BOOL, 		filter,
	HTStream *, 	sink)
{
    return HTLoadDocument(addr,
			  HTAnchor_parent(HTAnchor_findAddress(addr)),
			  (HTOutputFormat ? HTOutputFormat : WWW_PRESENT),
			  sink);
}
#endif /* NOT_USED_CODE */

/*	Load a document from relative name.		HTLoadRelative()
**	-----------------------------------
**
**  On Entry,
**        relative_name     The relative address of the document
**	  		    to be accessed.
**
**  On Exit,
**        returns    YES     Success in opening document
**                   NO      Failure 
*/
PUBLIC BOOL HTLoadRelative ARGS2(
	CONST char *,		relative_name,
	HTParentAnchor *,	here)
{
    DocAddress full_address;
    BOOL result;
    char * mycopy = NULL;
    char * stripped = NULL;
    char * current_address = HTAnchor_address((HTAnchor*)here);

    full_address.address = NULL;
    full_address.post_data = NULL;
    full_address.post_content_type = NULL;
    full_address.bookmark = NULL;
    full_address.isHEAD = FALSE;
    full_address.safe = FALSE;

    StrAllocCopy(mycopy, relative_name);

    stripped = HTStrip(mycopy);
    full_address.address =
    		HTParse(stripped,
			current_address,
			PARSE_ACCESS|PARSE_HOST|PARSE_PATH|PARSE_PUNCTUATION);
    result = HTLoadAbsolute(&full_address);
    /*
    **  If we got redirection, result will be NO, but use_this_url_instead
    **  will be set.  The calling routine should check both and do whatever
    **  is appropriate. - FM
    */
    FREE(full_address.address);
    FREE(current_address);
    FREE(mycopy);  /* Memory leak fixed 10/7/92 -- JFG */
    return result;
}

/*	Load if necessary, and select an anchor.	HTLoadAnchor()
**	----------------------------------------
**
**  On Entry,
**        destination      	    The child or parenet anchor to be loaded.
**
**  On Exit,
**        returns    YES     Success
**                   NO      Failure 
*/
PUBLIC BOOL HTLoadAnchor ARGS1(
	HTAnchor *,	destination)
{
    HTParentAnchor * parent;
    BOOL loaded = NO;
    if (!destination)
        return NO;	/* No link */
    
    parent = HTAnchor_parent(destination);
    
    if (HTAnchor_document(parent) == NULL) {	/* If not alread loaded */
    						/* TBL 921202 */
        BOOL result;
        char * address = HTAnchor_address((HTAnchor*) parent);

	result = HTLoadDocument(address,
				parent,
				HTOutputFormat ?
				HTOutputFormat : WWW_PRESENT,
				HTOutputStream);
	FREE(address);
	if (!result) return NO;
	loaded = YES;
    }
    
    {
	HText *text = (HText*)HTAnchor_document(parent);

	if (destination != (HTAnchor *)parent) {  /* If child anchor */
	    HText_selectAnchor(text, 		  /* Double display? @@ */
			       (HTChildAnchor*)destination);
	} else {
	    if (!loaded)
	        HText_select(text);
	}
    }
    return YES;
	
} /* HTLoadAnchor */

/*	Search.						HTSearch()
**	-------
**
**	Performs a keyword search on word given by the user.  Adds the
**	keyword to the end of the current address and attempts to open
**	the new address.
**
**  On Entry,
**       *keywords  	space-separated keyword list or similar search list
**	here		is anchor search is to be done on.
*/
PRIVATE char hex ARGS1(
    int,		i)
{
    char * hexchars = "0123456789ABCDEF";
    return hexchars[i];
}

PUBLIC BOOL HTSearch ARGS2(
	CONST char *, 		keywords,
	HTParentAnchor *, 	here)
{
#define acceptable \
"1234567890abcdefghijlkmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-_"

    char *q, *u;
    CONST char * p, *s, *e;		/* Pointers into keywords */
    char * address = NULL;
    BOOL result;
    char * escaped = malloc((strlen(keywords)*3) + 1);
    static CONST BOOL isAcceptable[96] =

    /*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
    {    0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,	/* 2x   !"#$%&'()*+,-./	 */
         1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,	/* 3x  0123456789:;<=>?	 */
	 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/* 4x  @ABCDEFGHIJKLMNO  */
	 1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,	/* 5X  PQRSTUVWXYZ[\]^_	 */
	 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/* 6x  `abcdefghijklmno	 */
	 1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };	/* 7X  pqrstuvwxyz{\}~	DEL */

    if (escaped == NULL)
        outofmem(__FILE__, "HTSearch");
    
    StrAllocCopy(address, here->isIndexAction);

    /*
    **  Convert spaces to + and hex escape unacceptable characters.
    */
    for (s = keywords; *s && WHITE(*s); s++)		 /* Scan */
        ;	/* Skip white space */
    for (e = s + strlen(s); e > s && WHITE(*(e-1)); e--) /* Scan */
        ;	/* Skip trailers */
    for (q = escaped, p = s; p < e; p++) {	/* Scan stripped field */
        unsigned char c = (unsigned char)TOASCII(*p);
        if (WHITE(*p)) {
	    *q++ = '+';
	} else if (HTCJK != NOCJK) {
	    *q++ = *p;
	} else if (c>=32 && c<=(unsigned char)127 && isAcceptable[c-32]) {
	    *q++ = *p;				/* 930706 TBL for MVS bug */
	} else {
	    *q++ = '%';
	    *q++ = hex((int)(c >> 4));
	    *q++ = hex((int)(c & 15));
	}
    }				/* Loop over string */
    *q = '\0';			/* Terminate escaped string */
    u = strchr(address, '?');	/* Find old search string */
    if (u != NULL)
        *u = '\0';		/* Chop old search off */

    StrAllocCat(address, "?");
    StrAllocCat(address, escaped);
    FREE(escaped);
    result = HTLoadRelative(address, here);
    FREE(address);
    
    /*
    **  If we got redirection, result will be NO, but use_this_url_instead
    **  will be set.  The calling routine should check both and do whatever
    **  is appropriate.  Only an http server (not a gopher or wais server)
    **  could return redirection.  Lynx will go all the way back to its
    **  mainloop() and subject a redirecting URL to all of its security and
    **  restrictions checks. - FM
    */
    return result;
}

/*	Search Given Indexname.			HTSearchAbsolute()
**	-----------------------
**
**	Performs a keyword search on word given by the user.  Adds the
**	keyword to the end of the current address and attempts to open
**	the new address.
**
**  On Entry,
**       *keywords  	space-separated keyword list or similar search list
**	*addres		is name of object search is to be done on.
*/
PUBLIC BOOL HTSearchAbsolute ARGS2(
	CONST char *, 	keywords,
	CONST char *, 	indexname)
{
    DocAddress abs_doc;
    HTParentAnchor * anchor;
    abs_doc.address = (char *)indexname;
    abs_doc.post_data = NULL;
    abs_doc.post_content_type = NULL;
    abs_doc.bookmark = NULL;
    abs_doc.isHEAD = FALSE;
    abs_doc.safe = FALSE;

    anchor = (HTParentAnchor*)HTAnchor_findAddress(&abs_doc);
    return HTSearch(keywords, anchor);
}

#ifdef NOT_USED_CODE
/*	Generate the anchor for the home page.		HTHomeAnchor()
**	--------------------------------------
**
**	As it involves file access, this should only be done once
**	when the program first runs.
**	This is a default algorithm -- browser don't HAVE to use this.
**	But consistency betwen browsers is STRONGLY recommended!
**
**  Priority order is:
**		1	WWW_HOME environment variable (logical name, etc)
**		2	~/WWW/default.html
**		3	/usr/local/bin/default.html
**		4	http://www.w3.org/default.html
*/
PUBLIC HTParentAnchor * HTHomeAnchor NOARGS
{
    char * my_home_document = NULL;
    char * home = (char *)getenv(LOGICAL_DEFAULT);
    char * ref;
    HTParentAnchor * anchor;
    
    if (home) {
        StrAllocCopy(my_home_document, home);
#define MAX_FILE_NAME 1024			/* @@@ */
    } else if (HTClientHost) {			/* Telnet server */
	/* 
	**  Someone telnets in, they get a special home.
	*/
    	FILE * fp = fopen(REMOTE_POINTER, "r");
	char * status;
	if (fp) {
	    my_home_document = (char*)malloc(MAX_FILE_NAME);
	    if (my_home_document == NULL)
	        outofmem(__FILE__, "HTHomeAnchor");
	    status = fgets(my_home_document, MAX_FILE_NAME, fp);
	    if (!status) {
	        FREE(my_home_document);
	    }
	    fclose(fp);
	}
	if (my_home_document == NULL)
	    StrAllocCopy(my_home_document, REMOTE_ADDRESS);
    }

#ifdef unix
    if (my_home_document == NULL) {
	FILE * fp = NULL;
	CONST char * home =  (CONST char*)getenv("HOME");
	if (home != null) { 
	    my_home_document = (char *)malloc(
		strlen(home) + 1 + strlen(PERSONAL_DEFAULT) + 1);
	    if (my_home_document == NULL)
	        outofmem(__FILE__, "HTAnchorHome");
	    sprintf(my_home_document, "%s/%s", home, PERSONAL_DEFAULT);
	    fp = fopen(my_home_document, "r");
	}
	
	if (!fp) {
	    StrAllocCopy(my_home_document, LOCAL_DEFAULT_FILE);
	    fp = fopen(my_home_document, "r");
	}
	if (fp) {
	    fclose(fp);
	} else {
	    if (TRACE)
	        fprintf(stderr,
			"HTBrowse: No local home document ~/%s or %s\n",
			PERSONAL_DEFAULT, LOCAL_DEFAULT_FILE);
	    FREE(my_home_document);
	}
    }
#endif /* unix */
    ref = HTParse((my_home_document ?
    		   my_home_document : (HTClientHost ?
		   		     REMOTE_ADDRESS : LAST_RESORT)),
		  "file:",
		  PARSE_ACCESS|PARSE_HOST|PARSE_PATH|PARSE_PUNCTUATION);
    if (my_home_document) {
	if (TRACE)
	    fprintf(stderr,
		    "HTAccess: Using custom home page %s i.e. address %s\n",
		    my_home_document, ref);
	FREE(my_home_document);
    }
    anchor = (HTParentAnchor*)HTAnchor_findAddress(ref);
    FREE(ref);
    return anchor;
}
#endif /* NOT_USED_CODE */
