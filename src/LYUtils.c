#include "HTUtils.h"
#include "tcp.h"
#include "HTParse.h"
#include "HTAccess.h"
#include "HTCJK.h"
#include "HTAlert.h"
#include "LYCurses.h"
#include "LYUtils.h"
#include "LYStrings.h"
#include "LYGlobalDefs.h"
#include "LYSignal.h"
#include "GridText.h"

#ifdef VMS
#include <descrip.h>
#include <libclidef.h>
#include <lib$routines.h>
#include "HTVMSUtils.h"
#endif /* VMS */

#ifdef UNIX
#include <pwd.h>
#ifdef UTMPX_FOR_UTMP
#include <utmpx.h>
#define utmp utmpx
#ifdef UTMP_FILE
#undef UTMP_FILE
#endif /* UTMP_FILE */
#define UTMP_FILE UTMPX_FILE
#else
#include <utmp.h>
#endif /* UTMPX_FOR_UTMP */
#endif /* UNIX */

#include "LYLeaks.h"

#ifdef SVR4_BSDSELECT
extern int BSDselect PARAMS((int nfds, fd_set * readfds, fd_set * writefds,
	 		     fd_set * exceptfds, struct timeval * timeout));
#ifdef select
#undef select
#endif /* select */
#define select BSDselect
#ifdef SOCKS
#ifdef Rselect
#undef Rselect
#endif /* Rselect */
#define Rselect BSDselect
#endif /* SOCKS */
#endif /* SVR4_BSDSELECT */

#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif /* !FD_SETSIZE */

#ifndef UTMP_FILE
#if defined(__FreeBSD__) || defined(__bsdi__)
#define UTMP_FILE _PATH_UTMP
#else
#define UTMP_FILE "/etc/utmp"
#endif /* __FreeBSD__ || __bsdi__ */
#endif /* !UTMP_FILE */

#define FREE(x) if (x) {free(x); x = NULL;}

extern HTkcode kanji_code;
extern BOOLEAN LYHaveCJKCharacterSet;
extern HTCJKlang HTCJK;

PRIVATE HTList * localhost_aliases = NULL;	/* Hosts to treat as local */
PUBLIC  HTList * sug_filenames = NULL;		/* Suggested filenames   */

/*
 * highlight (or unhighlight) a given link
 */
PUBLIC void highlight ARGS2(int,flag, int,cur)
{
    char buffer[200];
    int i;
    char tmp[3];

    tmp[0] = tmp[1] = tmp[2] = '\0';

    /* Bug in history code can cause -1 to be sent, which will yield
    ** an ACCVIO when LYstrncpy() is called with a nonsense pointer.
    ** This works around the bug, for now. -- FM
    */
    if (cur < 0)
        cur = 0;

    if (nlinks > 0) {
	move(links[cur].ly, links[cur].lx);
	if (flag == ON) { 
	    /* makes some terminals work wrong because
	     * they can't handle two attributes at the 
	     * same time
	     */
	    /* start_bold();  */
	    start_reverse();
#ifdef USE_SLANG
	    start_underline ();
#endif /* USE_SLANG */
	} else {
	    start_bold();
	}

	if (links[cur].type == WWW_FORM_LINK_TYPE) {
	    int len;
	    int avail_space = (LYcols-links[cur].lx)-1;

	    LYstrncpy(buffer,
	    	      links[cur].hightext, 
		      (avail_space > links[cur].form->size ? 
				      links[cur].form->size : avail_space));
	    addstr(buffer);  

	    len = strlen(buffer);
	    for (; len < links[cur].form->size && len < avail_space; len++)
	        addch('_');

	} else {

	    /* copy into the buffer only what will fit within the
	     * width of the screen
	     */
	    LYstrncpy(buffer,links[cur].hightext, LYcols-links[cur].lx-1);
	    addstr(buffer);  
	}

	/* display a second line as well */
	if (links[cur].hightext2 && links[cur].ly < display_lines) {
	    if (flag == ON) {
	        stop_reverse();
#ifdef USE_SLANG
		stop_underline ();
#endif /* USE_SLANG */
	    } else {
	        stop_bold();
	    }

	    addch('\n');
	    for (i=0; i < links[cur].hightext2_offset; i++)
	        addch(' ');

	    if (flag == ON) {
	        start_reverse();
#ifdef USE_SLANG
		start_underline ();
#endif /* USE_SLANG */
	    } else {
	        start_bold();
	    }

	    for (i=0; (tmp[0] = links[cur].hightext2[i]) != '\0' &&
	    	      i+links[cur].hightext2_offset < LYcols; i++) {
		if (!IsSpecialAttrChar(links[cur].hightext2[i])) {
		    /* For CJK strings, by Masanobu Kimura */
		    if (HTCJK != NOCJK && !isascii(tmp[0])) {
		        tmp[1] = links[cur].hightext2[++i];
			addstr(tmp);
			tmp[1] = '\0';
		    } else {
		        addstr(tmp);
		    }
		 }
	    }
	}

	if (flag == ON) {
	    stop_reverse();
#ifdef USE_SLANG
	    stop_underline ();
#endif /* USE_SLANG */
	} else {
	    stop_bold();
	}

#if defined(FANCY_CURSES) || defined(USE_SLANG)
	if (!LYShowCursor)
	    move(LYlines-1, LYcols-1);  /* get cursor out of the way */
	else
#endif /* FANCY CURSES || USE_SLANG */
	    /* never hide the cursor if there's no FANCY CURSES or SLANG */
	    move(links[cur].ly, links[cur].lx - 1);

	if (flag)
	    refresh();
    }
    return;
}

/*
 * free_and_clear will free a pointer if it is non-zero and
 * then set it to zero
 */
PUBLIC void free_and_clear ARGS1(char **,pointer)
{
    if (*pointer) {
	free(*pointer);
        *pointer = 0;
    }
    return;
}

/*
 * Collapse (REMOVE) all spaces in the string. 
 */
PUBLIC void collapse_spaces ARGS1(char *,string)
{
    int i=0;
    int j=0;

    if (!string)
        return;

    for (; string[i] != '\0'; i++) 
	if (!isspace(string[i])) 
	    string[j++] = string[i];

    string[j] = '\0';  /* terminate */
    return;
}

/*
 * Convert single or serial newlines to single spaces throughout a string
 * (ignore newlines if the preceding character is a space) and convert
 * tabs to single spaces (but don't ignore any explicit tabs or spaces).
 */
PUBLIC void convert_to_spaces ARGS1(char *,string)
{
    char *s = string;
    char *ns = string;
    BOOL last_is_space = FALSE;

    if (!string)
        return;

    while (*s) {
	switch (*s) {
	    case ' ':
	    case '\t':
		*(ns++) = ' ';
		last_is_space = TRUE;
		break;

	    case '\r':
	    case '\n':
	        if (!last_is_space) {
		    *(ns++) = ' ';
		    last_is_space = TRUE;
		}
		break;

	    default:
		*(ns++) = *s;
		last_is_space = FALSE;
		break;
	}
	s++;
    }
    *ns = '\0';
    return;
}

/*
 * display (or hide) the status line
 */
BOOLEAN mustshow = FALSE;

PUBLIC void statusline ARGS1(char *,text)
{
    char buffer[256];
    unsigned char *temp = NULL;
    extern BOOLEAN no_statusline;
    int max_length, len, i, j;
    unsigned char k;

    if (!text || text==NULL)
	return;

    /*
     *  Don't print statusline messages if dumping to stdout.
     */
    if (dump_output_immediately)
	return;

    /*
     *  Don't print statusline message if turned off.
     */
    if (mustshow != TRUE) {
	if (no_statusline == TRUE) {
	    return;
	}
    }
    mustshow = FALSE;

    /*
     *  Deal with any CJK escape sequences and Kanji if we have a CJK
     *  character set selected, otherwise, strip any escapes.  Also,
     *  make sure text is not longer than the statusline window. - FM
     */
    max_length = ((LYcols - 2) < 256) ? (LYcols - 2) : 255;
    if ((buffer[0] != '\0') &&
        (LYHaveCJKCharacterSet)) {
        /*
	 *  Translate or filter any escape sequences.
	 */
	if ((temp = (unsigned char *)calloc(1, strlen(text) + 1)) == NULL)
	    outofmem(__FILE__, "statusline");
	if (kanji_code == EUC) {
	    TO_EUC((unsigned char *)text, temp);
	} else if (kanji_code == SJIS) {
	    TO_SJIS((unsigned char *)text, temp);
	} else {
	    for (i = 0, j = 0; text[i]; i++) {
		if (text[i] != '\033') {
		    temp[j++] = text[i];
		}
	    }
	    temp[j] = '\0';
	}

        /*
	 *  Deal with any newlines or tabs in the string.
	 */
	convert_to_spaces((char *)temp);

	/*
	 *  Handle the Kanji, making sure the text is not
	 *  longer than the statusline window. - FM
	 */
	for (i = 0, j = 0, len = 0, k = '\0';
	     temp[i] != '\0' && len < max_length; i++) {
	    if (k != '\0') {
	        buffer[j++] = k;
		buffer[j++] = temp[i];
		k = '\0';
		len += 2;
	    } else if ((temp[i] & 0200) != 0) {
	        k = temp[i];
	    } else {
	        buffer[j++] = temp[i];
		len++;
	    }
	}
	buffer[j] = '\0';
	FREE(temp);
    } else {
        /*
	 *  Strip any escapes, and shorten text if necessary. - FM
	 */
	for (i = 0, len = 0; text[i] != '\0' && len < max_length; i++) {
	    if (text[i] != '\033') {
	        buffer[len++] = text[i];
	    }
	}
	buffer[len] = '\0';
        /*
	 *  Deal with any newlines or tabs in the string.
	 */
	convert_to_spaces(buffer);
    }

    /*
     *  Move to the statusline window and output the text highlighted.
     */
    if (user_mode == NOVICE_MODE)
        move(LYlines-3,0);
    else
        move(LYlines-1,0);
    clrtoeol();
    if (text != NULL) {
	start_reverse();
	addstr(buffer);
	stop_reverse();
    }

    refresh();
    return;
}

static char *novice_lines[] = {
#ifndef	NOVICE_LINE_TWO_A
#define	NOVICE_LINE_TWO_A	NOVICE_LINE_TWO
#define	NOVICE_LINE_TWO_B	""
#define	NOVICE_LINE_TWO_C	""
#endif /* !NOVICE_LINE_TWO_A */
  NOVICE_LINE_TWO_A,
  NOVICE_LINE_TWO_B,
  NOVICE_LINE_TWO_C,
  ""
};
static int lineno = 0;

PUBLIC void toggle_novice_line NOARGS
{
	lineno++;
	if (*novice_lines[lineno] == '\0')
		lineno = 0;
	return;
}

PUBLIC void noviceline ARGS1(int,more)
{

    if (dump_output_immediately)
	return;

    move(LYlines-2,0);
    stop_reverse();
    clrtoeol();
    addstr(NOVICE_LINE_ONE);
    clrtoeol();

#if defined(DIRED_SUPPORT ) && defined(OK_OVERRIDE)
    if (lynx_edit_mode && !no_dired_support) 
       addstr(DIRED_NOVICELINE);
    else
#endif /* DIRED_SUPPORT && OK_OVERRIDE */

    if (LYUseNoviceLineTwo)
        addstr(NOVICE_LINE_TWO);
    else
        addstr(novice_lines[lineno]);

#ifdef NOTDEFINED
    if (is_www_index && more) {
        addstr("This is a searchable index.  Use ");
	addstr(key_for_func(LYK_INDEX_SEARCH));
	addstr(" to search:");
	stop_reverse();
	addstr("                ");
	start_reverse();
        addstr("space for more");

    } else if (is_www_index) {
        addstr("This is a searchable index.  Use ");
	addstr(key_for_func(LYK_INDEX_SEARCH));
	addstr(" to search:");
    } else {
        addstr("Type a command or ? for help:");                   

        if (more) {
	    stop_reverse();
	    addstr("                       ");
	    start_reverse();
            addstr("Press space for next page");
	}
    }

#endif /* NOTDEFINED */

    refresh();
    return;
}

PUBLIC int HTCheckForInterrupt NOARGS
{
#ifndef VMS /* UNIX stuff: */
    int c;
#ifndef USE_SLANG
    struct timeval socket_timeout;
    int ret = 0;
    fd_set readfds;
#endif /* !USE_SLANG */

    /** Curses or slang setup was not invoked **/
    if (dump_output_immediately)
	return((int)FALSE);

#ifdef USE_SLANG
    /** No keystroke was entered
        Note that this isn't taking possible SOCKSification
	and the socks_flag into account, and may fail on the
	slang library's select() when SOCKSified. - FM **/
    if (0 == SLang_input_pending(0))
        return(FALSE);

#else /* Unix curses: */

    socket_timeout.tv_sec = 0;
    socket_timeout.tv_usec = 100;
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
#ifdef SOCKS
    if (socks_flag)
        ret = Rselect(FD_SETSIZE, (void *)&readfds, NULL, NULL,
	  	      &socket_timeout);
    else
#endif /* SOCKS */
        ret = select(FD_SETSIZE, (void *)&readfds, NULL, NULL,
	  	     &socket_timeout);

    /** No keystroke was entered **/
    if (!FD_ISSET(0,&readfds))
	 return((int)FALSE); 
#endif /* USE_SLANG */

    /** Keyboard 'Z' or 'z', or Control-G or Control-C **/
    c = LYgetch();
    if (TOUPPER(c) == 'Z' || c == 7 || c == 3)
	return((int)TRUE);

    /** Other keystrokes **/
    return((int)FALSE);

#else /* VMS: */

    int c;
    extern BOOLEAN HadVMSInterrupt;
    extern int typeahead();

    /** Curses or slang setup was not invoked **/
    if (dump_output_immediately)
	  return((int)FALSE);

    /** Control-C or Control-Y and a 'N'o reply to exit query **/
    if (HadVMSInterrupt) {
        HadVMSInterrupt = FALSE;
        return((int)TRUE);
    }

    /** Keyboard 'Z' or 'z', or Control-G or Control-C **/
    c = typeahead();
    if (TOUPPER(c) == 'Z' || c == 7 || c == 3)
        return((int)TRUE);

    /** Other or no keystrokes **/
    return((int)FALSE);
#endif /* !VMS */
}

/*
 * A file URL for a remote host is an obsolete ftp URL.
 * Return YES only if we're certain it's a local file. - FM
 */
PUBLIC BOOLEAN LYisLocalFile ARGS1(char *,filename)
{
    char *host=NULL;
    char *access=NULL;
    char *cp;

    if (!filename)
        return NO;
    if (!(host = HTParse(filename, "", PARSE_HOST)))
        return NO;
    if (!*host) {
        FREE(host);
	return NO;
    }

    if ((cp=strchr(host, ':')) != NULL)
        *cp = '\0';

    if ((access = HTParse(filename, "", PARSE_ACCESS))) {
        if (0==strcmp("file", access) &&
	    (0==strcmp(host, "localhost") ||
#ifdef VMS
             0==strcasecomp(host, HTHostName())))
#else
             0==strcmp(host, HTHostName())))
#endif /* VMS */
        {
	    FREE(host);
	    FREE(access);
	    return YES;
	}
    }

    FREE(host);
    FREE(access);
    return NO;
}

/*
 * Utility for checking URLs with a host field.
 * Return YES only if we're certain it's the local host. - FM
 */
PUBLIC BOOLEAN LYisLocalHost ARGS1(char *,filename)
{
    char *host=NULL;
    char *cp;

    if (!filename)
        return NO;
    if (!(host = HTParse(filename, "", PARSE_HOST)))
        return NO;
    if (!*host) {
        FREE(host);
	return NO;
    }

    if ((cp=strchr(host, ':')) != NULL)
        *cp = '\0';

#ifdef VMS
    if ((0==strcasecomp(host, "localhost") ||
	 0==strcasecomp(host, LYHostName) ||
         0==strcasecomp(host, HTHostName()))) {
#else
    if ((0==strcmp(host, "localhost") ||
	 0==strcmp(host, LYHostName) ||
         0==strcmp(host, HTHostName()))) {
#endif /* VMS */
	    FREE(host);
	    return YES;
    }

    FREE(host);
    return NO;
}

/* 
 * Utility for freeing the list of local host aliases. - FM
 */
PUBLIC void LYLocalhostAliases_free NOARGS
{
    char *alias;
    HTList *cur = localhost_aliases;

    if (!cur)
        return;

    while (NULL != (alias = (char *)HTList_nextObject(cur))) {
	FREE(alias);
    }
    HTList_delete(localhost_aliases);
    localhost_aliases = NULL;
    return;
}

/* 
 * Utility for listing hosts to be treated as local aliases. - FM
 */
PUBLIC void LYAddLocalhostAlias ARGS1(char *, alias)
{
    char *LocalAlias;

    if (!(alias && *alias))
        return;

    if (!localhost_aliases) {
        localhost_aliases = HTList_new();
	atexit(LYLocalhostAliases_free);
    }

    if ((LocalAlias = (char *)calloc(1, (strlen(alias) + 1))) == NULL)
    	outofmem(__FILE__, "HTAddLocalhosAlias");
    strcpy(LocalAlias, alias);
    HTList_addObject(localhost_aliases, LocalAlias);

    return;
}

/*
 * Utility for checking URLs with a host field.
 * Return YES only if we've listed the host as a local alias. - FM
 */
PUBLIC BOOLEAN LYisLocalAlias ARGS1(char *,filename)
{
    char *host=NULL;
    char *alias;
    char *cp;
    HTList *cur = localhost_aliases;

    if (!cur || !filename)
        return NO;
    if (!(host = HTParse(filename, "", PARSE_HOST)))
        return NO;
    if (!(*host)) {
        FREE(host);
	return NO;
    }

    if ((cp=strchr(host, ':')) != NULL)
        *cp = '\0';

    while (NULL != (alias = (char *)HTList_nextObject(cur))) {
#ifdef VMS
        if (0==strcasecomp(host, alias)) {
#else
        if (0==strcmp(host, alias)) {
#endif /* VMS */
	    FREE(host);
	    return YES;
	}
    }

    FREE(host);
    return NO;
}

/*
**  This function checks for a URL with an unknown scheme,
**  but for which proxying has been set up, and if so,
**  returns PROXY_URL_TYPE. - FM
**
**  If a colon is present but the string segment which
**  precedes it is not being proxied, and we can rule
**  out that what follows the colon is not a port field,
**  it returns UNKNOWN_URL_TYPE.  Otherwise, it returns
**  0 (not a URL). - FM
*/
PUBLIC int LYCheckForProxyURL ARGS1(char *, filename) {
    char *cp=filename;
    char *cp1;
    char *cp2 = NULL;

    /*
     *  Don't crash on an empty argument.
     */
    if (cp == NULL || *cp == '\0')
        return(0);

    /* kill beginning spaces */
    while (isspace(*cp))
        cp++;

    /*
     * Check for a colon, and if present,
     * see if we have proxying set up.
     */
    if ((cp1=strchr((cp+1), ':')) != NULL) {
	*cp1 = '\0';
	StrAllocCopy(cp2, cp);
	*cp1 = ':';
	StrAllocCat(cp2, "_proxy");
	if (getenv(cp2) != NULL) {
	    FREE(cp2);
	    return(PROXY_URL_TYPE);
	}
	FREE(cp2);
	cp1++;
	if (isdigit(*cp1)) {
	    while (*cp1 && isdigit(*cp1))
	        cp1++;
	    if (*cp1 && *cp1 != '/')
	        return(UNKNOWN_URL_TYPE);
	}
    }

    return(0);
}

/*
**  Must recognize a URL and return the type.
**  If recognized, based on a case-insensitive
**  analyis of the scheme field, ensures that
**  the scheme field has the expected case.
**
**  Returns 0 (not a URL) for a NULL argument,
**  one which lacks a colon.
**
**  Chains to LYCheckForProxyURL() if a colon
**  is present but the type is not recognized.
*/
PUBLIC int is_url ARGS1(char *,filename)
{
    char *cp = filename;
    char *cp1;
    int i;

    /*
     *  Don't crash on an empty argument.
     */
    if (cp == NULL || *cp == '\0')
        return(0);

    /*
     *  Can't be a URL if it lacks a colon.
     */
    if (NULL == strchr(cp, ':'))
        return(0);

    /*
     *  Kill beginning spaces.
     */
    while (isspace(*cp))
        cp++;

    if (!strncasecomp(cp, "news:", 5)) {
        if (strncmp(cp, "news", 4)) {
	    for (i = 0; i < 4; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(NEWS_URL_TYPE);

    } else if (!strncasecomp(cp, "snews:", 6)) {
        if (strncmp(cp, "snews", 5)) {
	    for (i = 0; i < 5; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(SNEWS_URL_TYPE);

    } else if (!strncasecomp(cp, "mailto:", 7)) {
        if (strncmp(cp, "mailto", 6)) {
	    for (i = 0; i < 6; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(MAILTO_URL_TYPE);

    } else if (!strncasecomp(cp, "data:", 5)) {
        if (strncmp(cp, "data", 4)) {
	    for (i = 0; i < 4; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(DATA_URL_TYPE);

    } else if (!strncasecomp(cp, "lynxexec:", 9)) {
	/*
	 *  Special External Lynx type to handle execution
	 *  of commands or scripts which require a pause to
	 *  read the screen upon completion.
	 */
        if (strncmp(cp, "lynxexec", 8)) {
	    for (i = 0; i < 8; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(LYNXEXEC_URL_TYPE);

    } else if (!strncasecomp(cp, "lynxprog:", 9)) {
	/*
	 *  Special External Lynx type to handle execution
	 *  of commans, sriptis or programs with do not
	 *  require a pause to read screen upon completion.
	 */
        if (strncmp(cp, "lynxprog", 8)) {
	    for (i = 0; i < 8; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(LYNXPROG_URL_TYPE);

    } else if (!strncasecomp(cp, "lynxcgi:", 8)) {
	/*
	 *  Special External Lynx type to handle cgi scripts.
	 */
        if (strncmp(cp, "lynxcgi", 7)) {
	    for (i = 0; i < 7; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(LYNXCGI_URL_TYPE);

    } else if (!strncasecomp(cp, "newspost:", 9)) {
	/*
	 *  Special Internal Lynx type to handle news posts.
	 */
        if (strncmp(cp, "newspost", 8)) {
	    for (i = 0; i < 8; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(NEWSPOST_URL_TYPE);

    } else if (!strncasecomp(cp, "newsreply:", 10)) {
	/*
	 *  Special Internal Lynx type to handle news replies (followups).
	 */
        if (strncmp(cp, "newsreply", 9)) {
	    for (i = 0; i < 9; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(NEWSREPLY_URL_TYPE);

    } else if (!strncasecomp(cp, "LYNXPRINT:", 10)) {
	/*
	 *  Special Internal Lynx type.
	 */
        if (strncmp(cp, "LYNXPRINT", 9)) {
	    for (i = 0; i < 9; i++)
	        cp[i] = TOUPPER(cp[i]);
	}
	return(LYNXPRINT_URL_TYPE);

    } else if (!strncasecomp(cp, "LYNXDOWNLOAD:", 13)) {
	/*
	 *  Special Internal Lynx type.
	 */
        if (strncmp(cp, "LYDOWNLOAD", 12)) {
	    for (i = 0; i < 12; i++)
	        cp[i] = TOUPPER(cp[i]);
	}
	return(LYNXDOWNLOAD_URL_TYPE);

    } else if (!strncasecomp(cp, "LYNXDIRED:", 10)) {
	/*
	 *  Special Internal Lynx type.
	 */
        if (strncmp(cp, "LYNXDIRED", 9)) {
	    for (i = 0; i < 9; i++)
	        cp[i] = TOUPPER(cp[i]);
	}
	return(LYNXDIRED_URL_TYPE);

    } else if (!strncasecomp(cp, "LYNXHIST:", 9)) {
	/*
	 *  Special Internal Lynx type.
	 */
        if (strncmp(cp, "LYNXHIST", 8)) {
	    for (i = 0; i < 8; i++)
	        cp[i] = TOUPPER(cp[i]);
	}
	return(LYNXHIST_URL_TYPE);

    } else if (!strncasecomp(cp, "LYNXKEYMAP:", 11)) {
	/*
	 *  Special Internal Lynx type.
	 */
        if (strncmp(cp, "LYNXKEYMAP", 10)) {
	    for (i = 0; i < 10; i++)
	        cp[i] = TOUPPER(cp[i]);
	}
	return(LYNXKEYMAP_URL_TYPE);

    } else if (!strncasecomp(cp, "LYNXIMGMAP:", 11)) {
	/*
	 *  Special Internal Lynx type.
	 */
        if (strncmp(cp, "LYNXIMGMAP", 10)) {
	    for (i = 0; i < 10; i++)
	        cp[i] = TOUPPER(cp[i]);
	}
	return(LYNXIMGMAP_URL_TYPE);

    } else if (strstr((cp+3), ":/") == NULL) {  
	/*
	 *  If it doesn't contain ":/", and it's not one of the
	 *  the above, it can't be a URL with a scheme we know,
	 *  so check if it's an unknown scheme for which proxying
	 *  has been set up. - FM
	 */
	return(LYCheckForProxyURL(filename));

    } else if (!strncasecomp(cp, "https:", 6)) {
        if (strncmp(cp, "https", 5)) {
	    for (i = 0; i < 5; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(HTTPS_URL_TYPE);

    } else if (!strncasecomp(cp, "http:", 5)) {
        if (strncmp(cp, "http", 4)) {
	    for (i = 0; i < 4; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(HTTP_URL_TYPE);

    } else if (!strncasecomp(cp, "file:", 5)) {
        if (strncmp(cp, "file", 4)) {
	    for (i = 0; i < 4; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
        /*
	 *  We won't expend the overhead here of
	 *  determining whether it's really an
	 *  ftp URL unless we are restricting
	 *  ftp access, in which case getfile()
	 *  needs to know in order to issue an
	 *  appropriate statusline message and
	 *  and return NULLFILE.
	 */
        if ((ftp_ok) || LYisLocalFile(cp))
	    return(FILE_URL_TYPE);
	else
	    return(FTP_URL_TYPE);

    } else if (!strncasecomp(cp, "gopher:", 7)) {
        if (strncmp(cp, "gopher", 6)) {
	    for (i = 0; i < 6; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	if ((cp1 = strchr(cp+11,'/')) != NULL) {

	    if (TOUPPER(*(cp1+1)) == 'H' || *(cp1+1) == 'w')
		/* if this is a gopher html type */
	        return(HTML_GOPHER_URL_TYPE);
	    else if (*(cp1+1) == 'T' || *(cp1+1) == '8')
	        return(TELNET_GOPHER_URL_TYPE);
	    else if (*(cp1+1) == '7')
	        return(INDEX_GOPHER_URL_TYPE);
	    else
	        return(GOPHER_URL_TYPE);
	} else {
	    return(GOPHER_URL_TYPE);
	}

    } else if (!strncasecomp(cp, "ftp:", 4)) {
        if (strncmp(cp, "ftp", 3)) {
	    for (i = 0; i < 3; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(FTP_URL_TYPE);

    } else if (!strncasecomp(cp, "wais:", 5)) {
        if (strncmp(cp, "wais", 4)) {
	    for (i = 0; i < 4; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(WAIS_URL_TYPE);

    } else if (!strncasecomp(cp, "telnet:", 7)) {
        if (strncmp(cp, "telnet", 6)) {
	    for (i = 0; i < 6; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(TELNET_URL_TYPE);

    } else if (!strncasecomp(cp, "tn3270:", 7)) {
        if (strncmp(cp, "tn", 2)) {
	    for (i = 0; i < 2; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(TN3270_URL_TYPE);

    } else if (!strncasecomp(cp, "rlogin:", 7)) {
        if (strncmp(cp, "rlogin", 6)) {
	    for (i = 0; i < 6; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(RLOGIN_URL_TYPE);

    } else if (!strncasecomp(cp, "nntp:", 5)) {
        if (strncmp(cp, "nntp", 4)) {
	    for (i = 0; i < 4; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(NNTP_URL_TYPE);

    } else if (!strncasecomp(cp, "cso:", 4)) {
        if (strncmp(cp, "cso", 3)) {
	    for (i = 0; i < 3; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(CSO_URL_TYPE);

    } else if (!strncasecomp(cp, "finger:", 7)) {
        if (strncmp(cp, "finger", 6)) {
	    for (i = 0; i < 6; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(FINGER_URL_TYPE);

    } else if (!strncasecomp(cp, "afs:", 4)) {
        if (strncmp(cp, "afs", 3)) {
	    for (i = 0; i < 3; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(AFS_URL_TYPE);

    } else if (!strncasecomp(cp, "prospero:", 9)) {
        if (strncmp(cp, "prospero", 8)) {
	    for (i = 0; i < 8; i++)
	        cp[i] = TOLOWER(cp[i]);
	}
	return(PROSPERO_URL_TYPE);

    } else {
        /*
	 *  Check if it's an unknown scheme for which
	 *  proxying has been set up. - FM
	 */
	return(LYCheckForProxyURL(filename));
    }
}

/*
 *  Remove backslashes from any string.
 */
PUBLIC void remove_backslashes ARGS1(char *,buf)
{
    char *cp;

    for (cp = buf; *cp != '\0' ; cp++) {

	if (*cp != '\\') { /* don't print slashes */
	    *buf = *cp;
	    buf++;
	} else if (*cp == '\\' &&      /* print one slash if there */
		   *(cp+1) == '\\') {  /* are two in a row         */
	    *buf = *cp;
	    buf++;
	}
    }
    *buf = '\0';
    return;
}

/*
 *  Quote the path to make it safe for shell command processing.
 *
 *  We use a simple technique which involves quoting the entire
 *  string using single quotes, escaping the real single quotes
 *  with double quotes. This may be gross but it seems to work.
 */
PUBLIC char * quote_pathname ARGS1(char *, pathname)
{
    int i, n = 0;
    char * result;

    for (i=0; i < strlen(pathname); ++i)
        if (pathname[i] == '\'') ++n;

    result = (char *)malloc(strlen(pathname) + 5*n + 3);
    if (result == NULL)
        outofmem(__FILE__, "quote_pathname");

    result[0] = '\'';
    for (i = 0, n = 1; i < strlen(pathname); i++)
        if (pathname[i] == '\'') {
            result[n++] = '\'';
            result[n++] = '"';
            result[n++] = '\'';
            result[n++] = '"';
            result[n++] = '\'';
        } else {
            result[n++] = pathname[i];
	}
    result[n++] = '\'';
    result[n] = '\0';
    return result;
}

/*
 * checks to see if the current process is attached via a terminal in the
 * local domain
 *
 */
#ifdef VMS
#ifndef NO_UTMP
#define NO_UTMP
#endif /* NO_UTMP */
#endif /* VMS */

PUBLIC BOOLEAN inlocaldomain NOARGS
{
#ifdef NO_UTMP
    return(TRUE);
#else
    int n;
    FILE *fp;
    struct utmp me;
    char *cp, *mytty=NULL;
    char *ttyname();

    if ((cp=ttyname(0)))
	mytty = strrchr(cp, '/');

    if (mytty && (fp=fopen(UTMP_FILE, "r")) != NULL) {
	    mytty++;
	    do {
		n = fread((char *) &me, sizeof(struct utmp), 1, fp);
	    } while (n>0 && !STREQ(me.ut_line,mytty));
	    (void) fclose(fp);

	    if (n > 0 &&
	        strlen(me.ut_host) > strlen(LYLocalDomain) &&
	        STREQ(LYLocalDomain,
		  me.ut_host+strlen(me.ut_host)-strlen(LYLocalDomain)) )
		return(TRUE);
#ifdef LINUX
/* Linux fix to check for local user. J.Cullen 11Jul94          */
		if ((n > 0) && (strlen(me.ut_host) == 0))
			return(TRUE);
#endif /* LINUX */

    } else {
	if (TRACE)
	   fprintf(stderr,"Could not get ttyname or open UTMP file");
    }

    return(FALSE);
#endif /* NO_UTMP */
}

/**************
** This bit of code catches window size change signals
**/

#if defined(VMS) || defined(SNAKE)
#define NO_SIZECHANGE
#endif /* VMS || SNAKE */

#if !defined(VMS) && !defined(ISC)
#include <sys/ioctl.h>
#endif /* !VMS && !ISC */

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#else
#ifdef HAVE_TERMIO_H
#include <termio.h>
#endif /* HAVE_TERMIO_H */
#endif /* HAVE_TERMIOS_H */

PUBLIC void size_change ARGS1(int,sig)
{
#ifndef NO_SIZECHANGE
#ifdef TIOCGSIZE
    struct ttysize win;
#else
#ifdef TIOCGWINSZ
    struct winsize win;
#endif /* TIOCGWINSZ */
#endif /* TIOCGSIZE */

#ifdef TIOCGSIZE
    if (ioctl(0, TIOCGSIZE, &win) == 0) {
        if (win.ts_lines != 0) {
#ifdef USE_SLANG
	    LYlines = win.ts_lines;
#else
	    LYlines = win.ts_lines - 1;
#endif /* USE_SLANG */
	}
	if (win.ts_cols != 0) {
	    LYcols = win.ts_cols;
	}
    }
#else
#ifdef TIOCGWINSZ
    if (ioctl(0, TIOCGWINSZ, &win) == 0) {
        if (win.ws_row != 0) {
#ifdef USE_SLANG
	    LYlines = win.ws_row;
#else
	    LYlines = win.ws_row - 1;
#endif /* USE_SLANG */
	}
	if (win.ws_col != 0) {
	    LYcols = win.ws_col;
	}
    }
#endif /* TIOCGWINSZ */
#endif /* TIOCGSIZE */
#endif /* !NO_SIZECHANGE */

    if (LYlines <= 0)
        LYlines = 24;
    if (LYcols <= 0)
        LYcols = 80;

#ifdef USE_SLANG
    SLtt_Screen_Rows = LYlines; 
    SLtt_Screen_Cols = LYcols;
    if (sig == 0)
        return;	/* called from start_curses */
#endif /* USE_SLANG */

    recent_sizechange = TRUE; 
#ifdef SIGWINCH
    (void)signal (SIGWINCH, size_change);
#endif /* SIGWINCH */

    return;
}

/* 
 * Utility for freeing the list of previous suggested filenames. - FM
 */
PUBLIC void HTSugFilenames_free NOARGS
{
    char *fname;
    HTList *cur = sug_filenames;

    if (!cur)
        return;

    while (NULL != (fname = (char *)HTList_nextObject(cur))) {
	FREE(fname);
    }
    HTList_delete(sug_filenames);
    sug_filenames = NULL;
    return;
}

/* 
 *  Utility for listing suggested filenames, making any
 *  repeated filenanmes the most current in the list. - FM
 */
PUBLIC void HTAddSugFilename ARGS1(char *, fname)
{
    char *new;
    char *old;
    HTList *cur;

    if (!(fname && *fname))
        return;

    if ((new = (char *)calloc(1, (strlen(fname) + 1))) == NULL)
    	outofmem(__FILE__, "HTAddSugFilename");
    strcpy(new, fname);

    if (!sug_filenames) {
        sug_filenames = HTList_new();
	atexit(HTSugFilenames_free);
	HTList_addObject(sug_filenames, new);
	return;
    }

    cur = sug_filenames;
    while (NULL != (old = (char *)HTList_nextObject(cur))) {
	if (!strcmp(old, new)) {
	    HTList_removeObject(sug_filenames, old);
	    FREE(old);
	    break;
	}
    }
    HTList_addObject(sug_filenames, new);

    return;
}

/*
 *  CHANGE_SUG_FILENAME -- Foteos Macrides 29-Dec-1993
 *	Upgraded for use with Lynx2.2 - FM 17-Jan-1994
 */
PUBLIC void change_sug_filename ARGS1(char *,fname)
{
     char *temp, *cp, *cp1, *end;
     int len;
#ifdef VMS
     char *dot;
     int j,k;
#endif /* VMS */

     /*** establish the current end of fname ***/
     end = fname + strlen(fname);

     /*** unescape fname ***/
     HTUnEscape(fname);

     /*** rename any temporary files ***/
     temp = (char *)calloc(1, (strlen(lynx_temp_space) + 60));
     if (*lynx_temp_space == '/')
         sprintf(temp, "file://localhost%sL%d", lynx_temp_space, getpid());
     else
         sprintf(temp, "file://localhost/%sL%d", lynx_temp_space, getpid());
     len = strlen(temp);
     if (0==strncmp(fname, temp, len)) {
         cp = strrchr(fname, '.');
	 if (strlen(cp) > (len - 4))
	     cp = NULL;
	 strcpy(temp, (cp ? cp : ""));
	 strcpy(fname, "temp");
	 strcat(fname, temp);
     }
     FREE(temp);

     /*** remove everything up the the last_slash if there is one ***/
     if ((cp = strrchr(fname,'/')) != NULL && strlen(cp) > 1) {
	 cp1=fname;
	 cp++; /* go past the slash */
	 for (; *cp != '\0'; cp++, cp1++)
	    *cp1 = *cp;

	 *cp1 = '\0'; /* terminate */
     }

     /*** Trim off date-size suffix, if present ***/
     if ((*(end - 1) == ']') && ((cp = strrchr(fname, '[')) != NULL) &&
         (cp > fname) && *(--cp) == ' ')
	  while (*cp == ' ')
	       *(cp--) = '\0';

     /*** Trim off VMS device and/or directory specs, if present ***/
     if ((cp=strchr(fname,'[')) != NULL &&
         (cp1=strrchr(cp,']')) != NULL && strlen(cp1) > 1) {
	  cp1++;
	  for (cp=fname; *cp1 != '\0'; cp1++)
	       *(cp++) = *cp1;
	  *cp = '\0';
     }

#ifdef VMS
     /*** Replace illegal or problem characters ***/
     dot = fname + strlen(fname);
     for (cp = fname; cp < dot; cp++) {

	  /** Replace with underscores **/
	  if (*cp == ' ' || *cp == '/' || *cp == ':' ||
	      *cp == '[' || *cp == ']' || *cp == '&')
	       *cp = '_';

	  /** Replace with dashes **/
	  else if (*cp == '!' || *cp == '?' || *cp == '\'' || 
	           *cp == ',' || *cp == ':' || *cp == '\"' ||
	           *cp == '+' || *cp == '@' || *cp == '\\' ||
	           *cp == '(' || *cp == ')' || *cp == '=' ||
	           *cp == '<' || *cp == '>' || *cp == '#' ||
	           *cp == '%' || *cp == '*' || *cp == '`' ||
	           *cp == '~' || *cp == '^' || *cp == '|' ||
		   *cp <  ' ' || ((unsigned char)*cp) > 126)
	       *cp = '-';
     }

     /** Collapse any serial underscores **/
     cp = fname + 1;
     j = 0;
     while (cp < dot) {
	  if (fname[j] == '_' && *cp == '_')
	       cp++;
	  else
	       fname[++j] = *cp++;
     }
     fname[++j] = '\0';

     /** Collapse any serial dashes **/
     dot = fname + (strlen(fname));
     cp = fname + 1;
     j = 0;
     while (cp < dot) {
          if (fname[j] == '-' && *cp == '-')
	       cp++;
	  else
	       fname[++j] = *cp++;
     }
     fname[++j] = '\0';

     /** Trim any trailing or leading **/
     /** underscrores or dashes       **/
     cp = fname + (strlen(fname)) - 1;
     while (*cp == '_' || *cp == '-')
          *cp-- = '\0';
     if (fname[0] == '_' || fname[0] == '-') {
          dot = fname + (strlen(fname));
          cp = fname;
          while ((*cp == '_' || *cp == '-') && cp < dot)
	       cp++;
	  j = 0;
          while (cp < dot)
	       fname[j++] = *cp++;
	  fname[j] = '\0';
     }

     /** Replace all but the last period with _'s, or second **/
     /** to last if last is followed by a terminal Z or z,   **/
     /** or GZ or gz,					     **/
     /** e.g., convert foo.tar.Z to                          **/
     /**               foo.tar_Z                             **/
     /**   or, convert foo.tar.gz to                         **/
     /**               foo.tar-gz                            **/
     j = strlen(fname) - 1;
     if ((dot = strrchr(fname, '.')) != NULL) {
	  if (TOUPPER(fname[j]) == 'Z') {
	      if ((fname[j-1] == '.') &&
	          (((cp = strchr(fname, '.')) != NULL) && cp < dot)) {
		  *dot = '_';
		  dot = strrchr(fname, '.');
	      } else if (((TOUPPER(fname[j-1]) == 'G') &&
	      		  fname[j-2] == '.') &&
			 (((cp = strchr(fname, '.')) != NULL) && cp < dot)) {
		  *dot = '-';
		  dot = strrchr(fname, '.');
	      }
	  }
	  cp = fname;
	  while ((cp = strchr(cp, '.')) != NULL && cp < dot)
	       *cp = '_';

          /** But if the root is > 39 characters, move **/
          /** the period appropriately to the left     **/
	  while (dot - fname > 39) {
	       *dot = '\0';
	       if ((cp = strrchr(fname, '_')) != NULL) {
		    *cp  = '.';
		    *dot = '_';
	       } 
	       else if ((cp = strrchr(fname, '-')) != NULL) {
		    *cp  = '.';
		    *dot = '_';
	       }
	       else {
		    *dot = '_';
		    j = strlen(fname);
		    fname[j+1] = '\0';
		    while (j > 39)
			 fname[j--] = fname[j];
		    fname[j] = '.';
	       }
               dot = strrchr(fname, '.');
	  }

          /** Make sure the extension is < 40 characters **/
          if ((fname + strlen(fname) - dot) > 39)
	       *(dot+40) = '\0';

	  /** Trim trailing dashes or underscores **/
	  j = strlen(fname) - 1;
	  while (fname[j] == '_' || fname[j] == '-')
	       fname[j--] = '\0';
     }
     else {
	  /** No period, so put one on the end, or after   **/
	  /** the 39th character, trimming trailing dashes **/
	  /** or underscrores                              **/
	  if (strlen(fname) > 39)
	       fname[39] = '\0';
	  j = strlen(fname) - 1;
	  while ((fname[j] == '_') || (fname[j] == '-'))
	       j--;
	  fname[++j] = '.';
	  fname[++j] = '\0';
     }

#else /* Not VMS (UNIX): */

     /*** Replace problem characters ***/
     for (cp = fname; *cp != '\0'; cp++) {
	  switch (*cp) {
	  case '\'':
	  case '\"':
	  case '/':
	  case ' ':
	       *cp = '-';
	  }
     }
#endif /* VMS (UNIX) */

     /** Make sure the rest of the original string in nulled. **/
     cp = fname + strlen(fname);
     while (cp < end)
          *cp++ = '\0';

    return;
}

/*
 *	To create standard temporary file names
 */
PUBLIC void tempname ARGS2(char *,namebuffer, int,action)
{
	static int counter = 0;


	if (action == REMOVE_FILES) { /* REMOVE ALL FILES */ 
	    for (; counter > 0; counter--) {
	        sprintf(namebuffer, "%sL%d%uTMP.txt", lynx_temp_space,
						      getpid(), counter-1);
		remove(namebuffer);
	        sprintf(namebuffer, "%sL%d%uTMP.html", lynx_temp_space,
						       getpid(), counter-1);
		remove(namebuffer);
	    }
	} else /* add a file */ {
	/*
	 * 	Create name
	 */
	    sprintf(namebuffer, "%sL%d%uTMP.html", lynx_temp_space,
	    					   getpid(), counter++);
	}
	return;
}

/*
 *  Convert 4, 6, 2, 8 to left, right, down, up, etc.
 */
PUBLIC int number2arrows ARGS1(int,number)
{
      switch(number) {
            case '1':
                number=END;
                  break;
            case '2':
                number=DNARROW;
                  break;
            case '3':
                number=PGDOWN;
                  break;
            case '4':
                number=LTARROW;
                  break;
	    case '5':
		number=DO_NOTHING;
		break;
            case '6':
                number=RTARROW;
                  break;
            case '7':
                number=HOME;
                  break;
 	    case '8':
                number=UPARROW;
                  break;
            case '9':
                number=PGUP;
                  break;
      }

      return(number);
}

/*
 * parse_restrictions takes a string of comma-separated restrictions
 * and sets the corresponding flags to restrict the facilities available
 */
PRIVATE char *restrict_name[] = {
       "inside_telnet" ,
       "outside_telnet",
       "telnet_port"   ,
       "inside_ftp"    ,
       "outside_ftp"   ,
       "inside_rlogin" ,
       "outside_rlogin",
       "suspend"       ,
       "editor"        ,
       "shell"         ,
       "bookmark"      ,
       "multibook"     ,
       "bookmark_exec" ,
       "option_save"   ,
       "print"         ,
       "download"      ,
       "disk_save"     ,
       "exec"          ,
       "lynxcgi"       ,
       "exec_frozen"   ,
       "goto"          ,
       "jump"          ,
       "file_url"      ,
       "news_post"     ,
       "inside_news"   ,
       "outside_news"  ,
       "mail"          ,
       "dotfiles"      ,
       "useragent"     ,
#ifdef DIRED_SUPPORT
       "dired_support" ,
#ifdef OK_PERMIT
       "change_exec_perms",
#endif /* OK_PERMIT */
#endif /* DIRED_SUPPORT */
       (char *) 0     };

	/* restrict_name and restrict_flag structure order
	 * must be maintained exactly!
	 */

PRIVATE BOOLEAN *restrict_flag[] = {
       &no_inside_telnet,
       &no_outside_telnet,
       &no_telnet_port,
       &no_inside_ftp,
       &no_outside_ftp,
       &no_inside_rlogin,
       &no_outside_rlogin,
       &no_suspend  ,
       &no_editor   ,
       &no_shell    ,
       &no_bookmark ,
       &no_multibook ,
       &no_bookmark_exec,
       &no_option_save,
       &no_print    ,
       &no_download ,
       &no_disk_save,
       &no_exec     ,
       &no_lynxcgi  ,
       &exec_frozen ,
       &no_goto     ,
       &no_jump     ,
       &no_file_url ,
       &no_newspost ,
       &no_inside_news,
       &no_outside_news,
       &no_mail     ,
       &no_dotfiles ,
       &no_useragent ,
#ifdef DIRED_SUPPORT
       &no_dired_support,
#ifdef OK_PERMIT
       &no_change_exec_perms,
#endif /* OK_PERMIT */
#endif /* DIRED_SUPPORT */
       (BOOLEAN *) 0  };

PUBLIC void parse_restrictions ARGS1(char *,s)
{
      char *p;
      char *word;
      int i;

      if (STREQ("all", s)) {
	   /* set all restrictions */
          for (i=0; restrict_flag[i]; i++) 
              *restrict_flag[i] = TRUE;
          return;
      }

      if (STREQ("default", s)) {
	   /* set all restrictions */
          for (i=0; restrict_flag[i]; i++) 
              *restrict_flag[i] = TRUE;

	     /* reset these to defaults */
             no_inside_telnet = !(CAN_ANONYMOUS_INSIDE_DOMAIN_TELNET);
            no_outside_telnet = !(CAN_ANONYMOUS_OUTSIDE_DOMAIN_TELNET);
	       no_inside_news = !(CAN_ANONYMOUS_INSIDE_DOMAIN_READ_NEWS);
	      no_outside_news = !(CAN_ANONYMOUS_OUTSIDE_DOMAIN_READ_NEWS);
                no_inside_ftp = !(CAN_ANONYMOUS_INSIDE_DOMAIN_FTP);
               no_outside_ftp = !(CAN_ANONYMOUS_OUTSIDE_DOMAIN_FTP);
             no_inside_rlogin = !(CAN_ANONYMOUS_INSIDE_DOMAIN_RLOGIN);
            no_outside_rlogin = !(CAN_ANONYMOUS_OUTSIDE_DOMAIN_RLOGIN);
		      no_goto = !(CAN_ANONYMOUS_GOTO);
		  no_goto_cso = !(CAN_ANONYMOUS_GOTO_CSO);
		 no_goto_file = !(CAN_ANONYMOUS_GOTO_FILE);
	       no_goto_finger = !(CAN_ANONYMOUS_GOTO_FINGER);
		  no_goto_ftp = !(CAN_ANONYMOUS_GOTO_FTP);
	       no_goto_gopher = !(CAN_ANONYMOUS_GOTO_GOPHER);
		 no_goto_http = !(CAN_ANONYMOUS_GOTO_HTTP);
		no_goto_https = !(CAN_ANONYMOUS_GOTO_HTTPS);
	      no_goto_lynxcgi = !(CAN_ANONYMOUS_GOTO_LYNXCGI);
	     no_goto_lynxexec = !(CAN_ANONYMOUS_GOTO_LYNXEXEC);
	     no_goto_lynxprog = !(CAN_ANONYMOUS_GOTO_LYNXPROG);
	       no_goto_mailto = !(CAN_ANONYMOUS_GOTO_MAILTO);
		 no_goto_news = !(CAN_ANONYMOUS_GOTO_NEWS);
		 no_goto_nntp = !(CAN_ANONYMOUS_GOTO_NNTP);
	       no_goto_rlogin = !(CAN_ANONYMOUS_GOTO_RLOGIN);
		no_goto_snews = !(CAN_ANONYMOUS_GOTO_SNEWS);
	       no_goto_telnet = !(CAN_ANONYMOUS_GOTO_TELNET);
	       no_goto_tn3270 = !(CAN_ANONYMOUS_GOTO_TN3270);
		 no_goto_wais = !(CAN_ANONYMOUS_GOTO_WAIS);
	       no_telnet_port = !(CAN_ANONYMOUS_GOTO_TELNET_PORT);
		      no_jump = !(CAN_ANONYMOUS_JUMP);
                      no_mail = !(CAN_ANONYMOUS_MAIL);
                     no_print = !(CAN_ANONYMOUS_PRINT);
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
		      no_exec = LOCAL_EXECUTION_LINKS_ALWAYS_OFF_FOR_ANONYMOUS;
#endif /* EXEC_LINKS || EXEC_SCRIPTS */
          return;
      }

      p = s;
      while (*p) {
          while (isspace(*p))
              p++;
          if (*p == '\0')
              break;
          word = p;
          while (*p != ',' && *p != '\0')
              p++;
          if (*p)
              *p++ = '\0';

	  for (i=0; restrict_name[i]; i++) 
             if (STREQ(word, restrict_name[i])) {
                 *restrict_flag[i] = TRUE;
		 break;
	     }
      }
      return;
}

#ifdef VMS
#include <jpidef.h>
#include <maildef.h>
#include <starlet.h>

typedef struct _VMSMailItemList
{
  short buffer_length;
  short item_code;
  void *buffer_address;
  long *return_length_address;
} VMSMailItemList;

PUBLIC int LYCheckMail NOARGS
{
    static BOOL firsttime = TRUE, failure = FALSE;
    static char user[13], dir[252];
    static long userlen = 0, dirlen;
    static time_t lastcheck = 0;
    time_t now;
    static short new, lastcount;
    long ucontext = 0, status;
    short flags = MAIL$M_NEWMSG;
    VMSMailItemList
      null_list[] = {{0,0,0,0}},
      jpi_list[]  = {{sizeof(user) - 1,JPI$_USERNAME,(void *)user,&userlen},
		     {0,0,0,0}},
      uilist[]    = {{0,MAIL$_USER_USERNAME,0,0},
    		     {0,0,0,0}},
      uolist[]    = {{sizeof(new),MAIL$_USER_NEW_MESSAGES,&new,0},
                     {sizeof(dir),MAIL$_USER_FULL_DIRECTORY,dir,&dirlen},
                     {0,0,0,0}};
    extern long mail$user_begin();
    extern long mail$user_get_info(); 
    extern long mail$user_end();

    if (failure)
        return 0;

    if (firsttime) {
        firsttime = FALSE;
        /* Get the username. */
        status = sys$getjpiw(0,0,0,jpi_list,0,0,0);
        if (!(status & 1)) {
            failure = TRUE;
            return 0;
        }
        user[userlen] = '\0';
        while (user[0] &&
	       isspace(user[--userlen])) /* suck up trailing spaces */
            user[userlen] = '\0';
    }

    /* Minimum report interval is 60 sec. */
    time(&now);
    if (now - lastcheck < 60)
	return 0;
    lastcheck = now;

    /* Get the current newmail count. */
    status = mail$user_begin(&ucontext,null_list,null_list);
    if (!(status & 1)) {
        failure = TRUE;
        return 0;
    }
    uilist[0].buffer_length = strlen(user);
    uilist[0].buffer_address = user;
    status = mail$user_get_info(&ucontext,uilist,uolist);
    if (!(status & 1)) {
        failure = TRUE;
        return 0;
    }

    /* Should we report anything to the user? */
    if (new > 0) {
	if (lastcount == 0)
	    /* Have newmail at startup of Lynx. */
	    _statusline(HAVE_UNREAD_MAIL_MSG);
	else if (new > lastcount)
	    /* Have additional mail since last report. */
	    _statusline(HAVE_NEW_MAIL_MSG);
        lastcount = new;
	return 1;
    }
    lastcount = new;

    /* Clear the context */
    mail$user_end((long *)&ucontext,null_list,null_list);
    return 0;
}
#else
PUBLIC int LYCheckMail NOARGS
{
    static BOOL firsttime = TRUE;
    static char *mf;
    static time_t lastcheck;
    static long lastsize;
    time_t now;
    struct stat st;

    if (firsttime) {
	mf = getenv("MAIL");
	firsttime = FALSE;
    }

    if (mf == NULL)
	return 0;

    time(&now);
    if (now - lastcheck < 60)
	return 0;
    lastcheck = now;

    if (stat(mf,&st) < 0) {
	mf = NULL;
	return 0;
    }

    if (st.st_size > 0) {
	if (st.st_mtime > st.st_atime ||
	    (lastsize && st.st_size > lastsize))
	    _statusline(HAVE_NEW_MAIL_MSG);
	else if (lastsize == 0)
	    _statusline(HAVE_MAIL_MSG);
        lastsize = st.st_size;
	return 1;
    }
    lastsize = st.st_size;
    return 0;
}
#endif /* VMS */

/*
 *  Rewrite and reallocate a previously allocated string
 *  as a file URL if the string resolves to a file or
 *  directory on the local system, otherwise as an
 *  http URL. - FM
 */
PUBLIC void LYConvertToURL ARGS1(char **, AllocatedString)
{
    char *old_string = *AllocatedString;

    if (!old_string || *old_string == '\0')
        return;

    *AllocatedString = NULL;  /* so StrAllocCopy doesn't free it */
    StrAllocCopy(*AllocatedString,"file://localhost");

    if (*old_string != '/') {
#ifdef VMS
	/*
	 *  Not a SHELL pathspec.  Get the full VMS spec and convert it.
	 */
	char *cp, *cur_dir=NULL;
	static char url_file[256], file_name[256], dir_name[256];
	unsigned long context = 0;
	$DESCRIPTOR(url_file_dsc, url_file);
	$DESCRIPTOR(file_name_dsc, file_name);
	if (*old_string == '~') {
	    /*
	     *  On VMS, we'll accept '~' on the command line as
	     *  $HOME, and assume the rest of the path, if any,
	     *  has SHELL syntax.
	     */
	    StrAllocCat(*AllocatedString, HTVMS_wwwName(getenv("HOME")));
	    if (old_string[1])
		StrAllocCat(*AllocatedString, (old_string+1));
	    goto have_VMS_URL;
	} else {
	    strcpy(url_file, old_string);
	}
	url_file_dsc.dsc$w_length = (short) strlen(url_file);
	if (1&lib$find_file(&url_file_dsc, &file_name_dsc, &context,
    			    0, 0, 0, 0)) {
	    /*
	     *  We found the file.  Convert to a URL pathspec.
	     */
	    if ((cp=strchr(file_name, ';')) != NULL) {
		*cp = '\0';
	    }
	    for (cp = file_name; *cp; cp++) {
		*cp = TOLOWER(*cp);
	    }
	    StrAllocCat(*AllocatedString, HTVMS_wwwName(file_name));
	    if ((cp=strchr(old_string, ';')) != NULL) {
		StrAllocCat(*AllocatedString, cp);
	    }
	} else if ((NULL != getcwd(dir_name, 255, 0)) &&
		   0 == chdir(old_string)) {
	    /*
	     * Probably a directory.  Try converting that.
	     */
	    StrAllocCopy(cur_dir, dir_name);
	    if (NULL != getcwd(dir_name, 255, 0)) {
		/*
		 * Yup, we got it!
		 */
		StrAllocCat(*AllocatedString, dir_name);
	    } else {
		/*
		 *  Nope.  Assume it's an http URL with
		 *  the "http://" defaulted, if we can't
		 *  rule out a bad VMS path.
		 */
		if (strchr(old_string, '[') ||
		    ((cp=strchr(old_string, ':')) != NULL &&
		     !isdigit(cp[1])) ||
		    !LYExpandHostForURL((char **)&old_string,
		    			URLDomainPrefixes,
					URLDomainSuffixes)) {
		    /*
		     *  Probably a bad VMS path (but can't be
		     *  sure).  Use original pathspec for the
		     *  error message that will result.
		     */
		    strcpy(url_file, "/");
		    strcat(url_file, old_string);
		    if (TRACE) {
			fprintf(stderr,
			    "Can't find '%s'  Will assume it's a bad path.\n",
			        old_string);
		    }
		    StrAllocCat(*AllocatedString, url_file);
		} else {
		    /*
		     *  Assume a URL is wanted, so guess the
		     *  scheme with "http://" as the default. - FM
		     */
		    if (!LYAddSchemeForURL((char **)&old_string, "http://")) {
		        StrAllocCopy(*AllocatedString, "http://");
			StrAllocCat(*AllocatedString, old_string);
		    } else {
		        StrAllocCopy(*AllocatedString, old_string);
		    }
		}
	    }
	} else {
	    /*
	     *  Nothing found.  Assume it's an http URL
	     *  with the "http://" defaulted, if we can't
	     *  rule out a bad VMS path.
	     */
	    if (strchr(old_string, '[') ||
		((cp=strchr(old_string, ':')) != NULL &&
		 !isdigit(cp[1])) ||
		!LYExpandHostForURL((char **)&old_string,
		    		    URLDomainPrefixes,
				    URLDomainSuffixes)) {
		/*
		 *  Probably a bad VMS path (but can't be
		 *  sure).  Use original pathspec for the
		 *  error message that will result.
		 */
		strcpy(url_file, "/");
		strcat(url_file, old_string);
		if (TRACE) {
		    fprintf(stderr,
			    "Can't find '%s'  Will assume it's a bad path.\n",
			        old_string);
		}
		StrAllocCat(*AllocatedString, url_file);
	    } else {
		/*
		 *  Assume a URL is wanted, so guess the
		 *  scheme with "http://" as the default. - FM
		 */
		if (!LYAddSchemeForURL((char **)&old_string, "http://")) {
		    StrAllocCopy(*AllocatedString, "http://");
		    StrAllocCat(*AllocatedString, old_string);
		} else {
		    StrAllocCopy(*AllocatedString, old_string);
		}
	    }
	}
	lib$find_file_end(&context);
	FREE(cur_dir);
have_VMS_URL:
	if (TRACE) {
	    fprintf(stderr, "Trying: '%s'\n", *AllocatedString);
	}
#else /* Unix: */
	if (*old_string == '~') {
	    /*
	     * On Unix, covert '~' to $HOME.
	     */
	    StrAllocCat(*AllocatedString, getenv("HOME"));
	    if (old_string[1]) {
		StrAllocCat(*AllocatedString, (old_string+1));
	    }
	    if (TRACE) {
		fprintf(stderr, "Converted '%s' to '%s'\n",
		    		old_string, *AllocatedString);
	    }
	} else {
	    /*
	     *  Create a full path to the current default directory.
	     */
	    char curdir[DIRNAMESIZE];
	    char *temp=NULL;
	    struct stat st;
	    FILE *fptemp=NULL;
#ifdef NO_GETCWD
	    getwd (curdir);
#else
	    getcwd (curdir, DIRNAMESIZE);
#endif /* NO_GETCWD */
	    StrAllocCopy(temp, curdir);
	    StrAllocCat(temp, "/");
	    StrAllocCat(temp, old_string);
	    if (TRACE) {
		fprintf(stderr, "Converted '%s' to '%s'\n", old_string, temp);
	    }
	    if ((stat(temp, &st) < 0) &&
		!(fptemp=fopen(temp, "r"))) {
		/*
		 *  It's not an accessible subdirectory or file on the
		 *  local system, so assume it's a URL request and guess
		 *  the scheme with "http://" as the default.
		 */
		if (TRACE) {
		    fprintf(stderr, "Can't stat() or fopen() '%s'\n", temp);
		}
		if (LYExpandHostForURL((char **)&old_string,
		    		       URLDomainPrefixes,
				       URLDomainSuffixes)) {
		    if (!LYAddSchemeForURL((char **)&old_string, "http://")) {
		        StrAllocCopy(*AllocatedString, "http://");
			StrAllocCat(*AllocatedString, old_string);
		    } else {
		        StrAllocCopy(*AllocatedString, old_string);
		    }
		} else {
		    StrAllocCat(*AllocatedString, temp);
		}
		if (TRACE) {
		    fprintf(stderr, "Trying: '%s'\n", *AllocatedString);
		}
	    } else {
		/*
		 *  It is a subdirectory or file on the local system.
		 */
		StrAllocCat(*AllocatedString, temp);
		if (TRACE) {
		    fprintf(stderr, "Converted '%s' to '%s'\n",
				    old_string, *AllocatedString);
		}
	    }
	    FREE(temp);
	    if (fptemp) {
		fclose(fptemp);
	    }
	}
#endif /* VMS */
    } else { 
	/*
	 *  Path begins with a slash.  Use it as is.
	 */
	StrAllocCat(*AllocatedString, old_string);
	if (TRACE) {
	    fprintf(stderr, "Converted '%s' to '%s'\n",
			    old_string, *AllocatedString);
	}
    }
    if (old_string) {
	FREE(old_string);
    }
    if (TRACE) {
	/* Pause so we can read the messages before invoking curses */
	sleep(AlertSecs);
    }
}

/*
 *  This function rewrites and reallocates a previously allocated
 *  string so that the first element is a confirmed Internet host,
 *  and returns TRUE, otherwise it does not modify the string and
 *  returns FALSE.  It first tries the element as is, then, if the
 *  element does not end with a dot, it adds prefixes from the
 *  (comma separated) prefix list arguement, and, if the element
 *  does not begin with a dot, suffixes from the (comma separated)
 *  suffix list arguments (e.g., www.host.com, then www.host,edu,
 *  then www.host.net, then www.host.org).  The remaining path, if
 *  one is present, will be appended to the expanded host.  It also
 *  takes into account whether a colon is in the element or suffix,
 *  and includes that and what follows as a port field for the
 *  expanded host field (e.g, wfbr:8002/dir/lynx should yield
 *  www.wfbr.edu:8002/dir/lynx).  The calling function should
 *  prepend the scheme field (e.g., http://), or pass the string
 *  to LYAddSchemeForURL(), if this function returns TRUE. - FM
 */
PUBLIC BOOLEAN LYExpandHostForURL ARGS3(
	char **,	AllocatedString,
	char *,		prefix_list,
	char *,		suffix_list)
{
    char DomainPrefix[80], *StartP, *EndP;
    char DomainSuffix[80], *StartS, *EndS;
    char *Str = NULL, *StrColon = NULL, *MsgStr = NULL;
    char *Host = NULL, *HostColon = NULL;
    char *Path = NULL;
    struct hostent  *phost;
    BOOLEAN GotHost = FALSE;

    /*
     *  If we were passed a NULL or zero-length string, or if it
     *  begins with a slash, don't continue pointlessly. - FM
     */
    if (!(*AllocatedString) ||
        *AllocatedString[0] == '\0' || *AllocatedString[0] == '/') {
        return GotHost;
    }

    /*
     *  Make a clean copy of the string, and trim off the
     *  path if one is present, but save the information
     *  so we can restore the path after filling in the
     *  Host[:port] field. - FM
     */
    StrAllocCopy(Str, *AllocatedString);
    if ((Path = strchr(Str, '/')) != NULL) {
        *Path = '\0';
    }

    /*
     *  If the potential host string has a colon, assume it
     *  begins a port field, and trim it off, but save the
     *  information so we can restore the port field after
     *  filling in the host field. - FM
     */
    if ((StrColon = strrchr(Str, ':')) != NULL && isdigit(StrColon[1])) {
        if (StrColon == Str) {
	    FREE(Str);
	    return GotHost;
	}
	*StrColon = '\0';
    }

    /*
     *  Do a DNS test on the potential host field
     *  as presently trimmed. - FM
     */
    if (LYCursesON) {
        StrAllocCopy(MsgStr, "Looking up ");
	StrAllocCat(MsgStr, Str);
	StrAllocCat(MsgStr, " first.");
	HTProgress(MsgStr);
    }
    if ((phost = gethostbyname(Str)) != NULL) {
        GotHost = TRUE;
        FREE(Str);
        FREE(MsgStr);
	return GotHost;
    }

    /*
    **  Set the first prefix, making it a zero-length string
    **  if the list is NULL or if the potential host field
    **  ends with a dot. - FM
    */
    StartP = ((prefix_list && Str[strlen(Str)-1] != '.') ?
    					     prefix_list : "");
    while ((*StartP) && (WHITE(*StartP) || *StartP == ',')) {
        StartP++;	/* Skip whitespace and separators */
    }
    EndP = StartP;
    while (*EndP && !WHITE(*EndP) && *EndP != ',') {
	EndP++;		/* Find separator */
    }
    LYstrncpy(DomainPrefix, StartP, (EndP - StartP));

    /*
    **  Test each prefix with each suffix. - FM
    */
    do {
        /*
	**  Set the first suffix, making it a zero-length string
	**  if the list is NULL or if the potential host field
	**  begins with a dot. - FM
	*/
	StartS = ((suffix_list && *Str != '.') ?
				   suffix_list : "");
	while ((*StartS) && (WHITE(*StartS) || *StartS == ',')) {
	    StartS++;	/* Skip whitespace and separators */
	}
	EndS = StartS;
	while (*EndS && !WHITE(*EndS) && *EndS != ',') {
	    EndS++;	/* Find separator */
	}
	LYstrncpy(DomainSuffix, StartS, (EndS - StartS));

	/*
	**  Create domain names and do DNS tests. - FM
	*/
	do {
	    StrAllocCopy(Host, DomainPrefix);
	    StrAllocCat(Host, ((*Str == '.') ? (Str + 1) : Str));
	    if (Host[strlen(Host)-1] == '.') {
	        Host[strlen(Host)-1] = '\0';
	    }
	    StrAllocCat(Host, DomainSuffix);
	    if ((HostColon = strrchr(Host, ':')) != NULL &&
	        isdigit(HostColon[1])) {
		*HostColon = '\0';
	    }
	    if (LYCursesON) {
 	        StrAllocCopy(MsgStr, "Looking up ");
 		StrAllocCat(MsgStr, Host);
 		StrAllocCat(MsgStr, ", guessing...");
 		HTProgress(MsgStr);
	    }
	    GotHost = ((phost = gethostbyname(Host)) != NULL);
	    if (HostColon != NULL) {
	        *HostColon = ':';
	    }
	    if (GotHost == FALSE) {
		/*
		 *  Give the user chance to interrupt lookup cycles. - KW
		 */
		if (LYCursesON && HTCheckForInterrupt()) {
		    if (TRACE) {
			fprintf(stderr,
	 "*** LYExpandHostForURL interrupted while %s failed to resolve\n",
				Host);
			    }
		    FREE(Str);
		    FREE(MsgStr);
		    FREE(Host);
		    return FALSE; /* We didn't find a valid name. */
		}

	        /*
		**  Advance to the next suffix, or end of suffix list. - FM
		*/
		StartS = ((*EndS == '\0') ? EndS : (EndS + 1));
		while ((*StartS) && (WHITE(*StartS) || *StartS == ',')) {
		    StartS++;	/* Skip whitespace and separators */
		}
		EndS = StartS;
		while (*EndS && !WHITE(*EndS) && *EndS != ',') {
		    EndS++;	/* Find separator */
		}
		LYstrncpy(DomainSuffix, StartS, (EndS - StartS));
	    }
	}  while ((GotHost == FALSE) && (*DomainSuffix != '\0'));

	if (GotHost == FALSE) {
	   /*
	   **  Advance to the next prefix, or end of prefix list. - FM
	   */
	   StartP = ((*EndP == '\0') ? EndP : (EndP + 1));
	   while ((*StartP) && (WHITE(*StartP) || *StartP == ',')) {
	       StartP++;	/* Skip whitespace and separators */
	   }
	   EndP = StartP;
	   while (*EndP && !WHITE(*EndP) && *EndP != ',') {
	       EndP++;		/* Find separator */
	   }
	   LYstrncpy(DomainPrefix, StartP, (EndP - StartP));
     	}
    } while ((GotHost == FALSE) && (*DomainPrefix != '\0'));

    /*
     *  If a test passed, restore the port field if we had one
     *  and there is no colon in the expanded host, and the path
     *  if we had one, and reallocate the original string with
     *  the expanded Host[:port] field included. - FM
     */
    if (GotHost) {
        if (StrColon && strchr(Host, ':') == NULL) {
	    *StrColon = ':';
	    StrAllocCat(Host, StrColon);
	}
        if (Path) {
	    *Path = '/';
	    StrAllocCat(Host, Path);
	}
	StrAllocCopy(*AllocatedString, Host);
    }

    /*
     *  Clean up and return the last test result. - FM
     */
    FREE(Str);
    FREE(MsgStr);
    FREE(Host);
    return GotHost;
}

/*
 *  This function rewrites and reallocates a previously allocated
 *  string that begins with an Internet host name so that the string
 *  begins with its guess of the scheme based on the first field of
 *  the host name, or the default scheme if no guess was made, and
 *  returns TRUE, otherwise it does not modify the string and returns
 *  FALSE.  It also returns FALSE without modifying the string if the
 *  default_scheme argument was NULL or zero-length and no guess was
 *  made. - FM
  */
PUBLIC BOOLEAN LYAddSchemeForURL ARGS2(
	char **,	AllocatedString,
	char *,		default_scheme)
{
    char *Str = NULL;
    BOOLEAN GotScheme = FALSE;

    /*
     *  If we were passed a NULL or zero-length string,
     *  don't continue pointlessly. - FM
     */
    if (!(*AllocatedString) || *AllocatedString[0] == '\0') {
        return GotScheme;
    }

    /*
     * Try to guess the appropriate scheme. - FM
     */
    if (0 == strncasecomp(*AllocatedString, "www", 3)) {
        /*
	 *  This could be either http or https, so check
	 *  the default and otherwise use "http". - FM
	 */
        if (default_scheme != NULL && 
	    NULL != strstr(default_scheme, "http")) {
            StrAllocCopy(Str, default_scheme);
	} else {
            StrAllocCopy(Str, "http://");
	}
	GotScheme = TRUE;

    } else if (0 == strncasecomp(*AllocatedString, "ftp", 3)) {
        StrAllocCopy(Str, "ftp://");
	GotScheme = TRUE;

    } else if (0 == strncasecomp(*AllocatedString, "gopher", 6)) {
        StrAllocCopy(Str, "gopher://");
	GotScheme = TRUE;

    } else if (0 == strncasecomp(*AllocatedString, "wais", 4)) {
        StrAllocCopy(Str, "wais://");
	GotScheme = TRUE;

    } else if (0 == strncasecomp(*AllocatedString, "cso", 3) ||
    	       0 == strncasecomp(*AllocatedString, "ns.", 3) ||
	       0 == strncasecomp(*AllocatedString, "ph.", 3)) {
        StrAllocCopy(Str, "cso://");
	GotScheme = TRUE;

    } else if (0 == strncasecomp(*AllocatedString, "finger", 6)) {
        StrAllocCopy(Str, "finger://");
	GotScheme = TRUE;

    } else if (0 == strncasecomp(*AllocatedString, "news", 4)) {
        /*
	 *  This could be either news, snews, or nntp, so
	 *  check the default, and otherwise use news. - FM
	 */
        if ((default_scheme != NULL) && 
	    (NULL != strstr(default_scheme, "news") ||
	     NULL != strstr(default_scheme, "nntp"))) {
            StrAllocCopy(Str, default_scheme);
	} else {
            StrAllocCopy(Str, "news://");
	}
	GotScheme = TRUE;

    } else if (0 == strncasecomp(*AllocatedString, "nntp", 4)) {
        StrAllocCopy(Str, "nntp://");
	GotScheme = TRUE;

    }

    /*
     *  If we've make a guess, use it.  Otherwise, if we
     *  were passed a default scheme prefix, use that. - FM
     */
    if (GotScheme == TRUE) {
        StrAllocCat(Str, *AllocatedString);
        StrAllocCopy(*AllocatedString, Str);
	FREE(Str);
	return GotScheme;

    } else if (default_scheme != NULL && *default_scheme != '\0') {
        StrAllocCopy(Str, default_scheme);
	GotScheme = TRUE;
        StrAllocCat(Str, *AllocatedString);
        StrAllocCopy(*AllocatedString, Str);
	FREE(Str);
	return GotScheme;
    }

    return GotScheme;
}

/*
 *  Example Client-Side Include interface.
 *
 *  This is called from SGML.c and simply returns markup for reporting
 *  the URL of the document being loaded if a comment begins with
 *  "<!--#lynxCSI".  The markup will be included as if it were in the
 *  document.  Move this function to a separate module for doing this
 *  kind of thing seriously, someday. - FM
 */
PUBLIC void LYDoCSI ARGS3(
	char *,		url,
	CONST char *,	comment,
	char **,	csi)
{
    char *cp = (char *)comment;

    if (cp == NULL)
        return;

    if (strncmp(cp, "!--#", 4))
        return;

    cp += 4;
    if (!strncasecomp(cp, "lynxCSI", 7)) {
        StrAllocCat(*csi, "\n<p align=\"center\">URL: ");
        StrAllocCat(*csi, url);
	StrAllocCat(*csi, "</p>\n\n");
    }

    return;
}

#ifdef VMS
/*
 *  Define_VMSLogical -- Fote Macrides 04-Apr-1995
 *	Define VMS logicals in the process table.
 */
PUBLIC void Define_VMSLogical ARGS2(char *, LogicalName, char *, LogicalValue)
{
    $DESCRIPTOR(lname, "");
    $DESCRIPTOR(lvalue, "");
    $DESCRIPTOR(ltable, "LNM$PROCESS");

    if (!LogicalName || *LogicalName == '\0')
        return;

    lname.dsc$w_length = strlen(LogicalName);
    lname.dsc$a_pointer = LogicalName;

    if (!LogicalValue || *LogicalValue == '\0') {
        lib$delete_logical(&lname, &ltable);
	return;
    }

    lvalue.dsc$w_length = strlen(LogicalValue);
    lvalue.dsc$a_pointer = LogicalValue;
    lib$set_logical(&lname, &lvalue, &ltable, 0, 0);
    return;
}
#endif /* VMS */

PUBLIC CONST char * Home_Dir NOARGS
{
    static CONST char *homedir;

    if (!homedir) {
	if ((homedir = getenv("HOME")) == NULL) {
#ifdef VMS
	    if ((homedir = getenv("SYS$LOGIN")) == NULL) {
	        if ((homedir = getenv("SYS$SCRATCH")) == NULL)
		    homedir = "sys$scratch:";
	    }
#else
	    /* One could use getlogin() and getpwnam() here instead */
	    struct passwd *pw = getpwuid(geteuid());
	    if (pw && pw->pw_dir) {
		homedir = (CONST char *)malloc(1+strlen(pw->pw_dir));
		if (homedir == NULL)
		    outofmem(__FILE__, "Home_Dir");
		strcpy((char *)homedir, pw->pw_dir);
	    }
	    else {
		/* Use /tmp; it should be writable. */
		homedir = "/tmp";
	    }
#endif /* VMS */
	}
    }
    return homedir;
}

#ifdef NO_PUTENV
/* no putenv on the next so we use this code instead!
 */

/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can  redistribute it and/or
modify it under the terms of the GNU Library General  Public License as
published by the Free Software Foundation; either  version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it  will be useful,
but WITHOUT ANY WARRANTY; without even the implied  warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library  General Public
License along with the GNU C Library; see the file  COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675  Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sys/types.h>
#include <errno.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#else
extern int errno;
#endif /* STDC_HEADERS */

#if defined(STDC_HEADERS) || defined(USG)
#include <string.h>
#define index strchr
#define bcopy(s, d, n) memcpy((d), (s), (n))
#else /* Not (STDC_HEADERS or USG): */
#include <strings.h>
#endif /* STDC_HEADERS or USG */

#ifndef NULL
#define NULL 0
#endif /* !NULL */

#if !__STDC__
#define const
#endif /* !__STDC__ */

extern char **environ;

/* Put STRING, which is of the form "NAME=VALUE", in  the environment.  */
int
putenv (string)
     const char *string;
{
  char *name_end = index (string, '=');
  register size_t size;
  register char **ep;

  if (name_end == NULL)
    {
      /* Remove the variable from the environment.  */
      size = strlen (string);
      for (ep = environ; *ep != NULL; ++ep)
	if (!strncmp (*ep, string, size) && (*ep)[size]  == '=')
	  {
	    while (ep[1] != NULL)
	      {
		ep[0] = ep[1];
		++ep;
	      }
	    *ep = NULL;
	    return 0;
	  }
    }

  size = 0;
  for (ep = environ; *ep != NULL; ++ep)
    if (!strncmp (*ep, string, name_end - string) && (*ep)[name_end - string] == '=')
      break;
    else
      ++size;

  if (*ep == NULL)
    {
      static char **last_environ = NULL;
      char **new_environ = (char **) malloc ((size + 2)  * sizeof (char *));
      if (new_environ == NULL)
	return -1;
      (void) bcopy ((char *) environ, (char *)  new_environ, size * sizeof (char *));
      new_environ[size] = (char *) string;
      new_environ[size + 1] = NULL;
      if (last_environ != NULL)
	free ((char *) last_environ);
      last_environ = new_environ;
      environ = new_environ;
    }
  else
    *ep = (char *) string;

  return 0;
}
#endif /* NO_PUTENV */

#ifdef VMS
/*
 *  This function appends fname to the home path and returns
 *  the full path and filename in VMS syntax.  The fname
 *  string can be just a filename, or include a subirectory
 *  off the home directory, in which chase fname should
 *  with "./" (e.g., ./BM/lynx_bookmarks.html). - FM
 */
PUBLIC void LYVMS_HomePathAndFilename ARGS3(
	char *,		fbuffer,
	int,		fbuffer_size,
	char *,		fname)
{
    char *home = NULL;
    char *temp = NULL;
    int len;

    /*
     *  Make sure we have a buffer and string. - FM
     */
    if (!fbuffer)
        return;
    if (!(fname && *fname) || fbuffer_size < 1) {
        fbuffer[0] = '\0';
	return;
    }

    /*
     *  Set up home string and length. - FM
     */
    StrAllocCopy(home, Home_Dir());
    if (!(home && *home))
        StrAllocCopy(home, "Error:");
    len = fbuffer_size - strlen(home) - 1;
    if (len < 0) {
        len = 0;
	home[fbuffer_size] = '\0';
    }

    /*
     *  Check whether we have a subdirectory path or just a filename. - FM
     */
    if (!strncmp(fname, "./", 2)) {
        /*
	 *  We have a subdirectory path. - FM
	 */
	if (home[strlen(home)-1] == ']') {
	    /*
	     *  We got the home directory, so convert it to
	     *  SHELL syntax and append subdirectory path,
	     *  then convert that to VMS syntax. - FM
	     */
	    temp = (char *)calloc(1, (strlen(home) + strlen(fname) + 10));
	    sprintf(temp, "%s%s", HTVMS_wwwName(home), (fname + 1));
	    sprintf(fbuffer, "%.*s",
	    	    (fbuffer_size - 1), HTVMS_name("", temp));
	    FREE(temp);
	} else {
	    /*
	     *  This will fail, but we need something in the buffer. - FM
	     */
	    sprintf(fbuffer,"%s%.*s", home, len, fname);
	}
    } else {
        /*
	 *  We have a file in the home directory. - FM
	 */
	sprintf(fbuffer,"%s%.*s", home, len, fname);
    }
    FREE(home);
}
#endif /* VMS */
