/*			NEWS ACCESS				HTNews.c
**			===========
**
** History:
**	26 Sep 90	Written TBL
**	29 Nov 91	Downgraded to C, for portable implementation.
*/

#include "HTUtils.h"		/* Coding convention macros */
#include "tcp.h"

/* Implements:
*/
#include "HTNews.h"

#include "HTCJK.h"
#include "HTMIME.h"
#include "HTTCP.h"

#define FREE(x) if (x) {free(x); x = NULL;}

/* this define should be in HTFont.h :( */
#define HT_NON_BREAK_SPACE ((char)1)   /* For now */

#define CR   FROMASCII('\015')	/* Must be converted to ^M for transmission */
#define LF   FROMASCII('\012')	/* Must be converted to ^J for transmission */

#define NEWS_PORT 119		/* See rfc977 */
#define SNEWS_PORT 563		/* See Lou Montulli */
#define APPEND			/* Use append methods */
PUBLIC int HTNewsChunkSize = 30;/* Number of articles for quick display */
PUBLIC int HTNewsMaxChunk = 40;	/* Largest number of articles in one window */

#ifndef DEFAULT_NEWS_HOST
#define DEFAULT_NEWS_HOST "news"
#endif /* DEFAULE_NEWS_HOST */
#ifndef SERVER_FILE
#define SERVER_FILE "/usr/local/lib/rn/server"
#endif /* SERVER_FILE */

#define NEWS_NETWRITE  NETWRITE
#define NEWS_NETCLOSE  NETCLOSE
#define NEXT_CHAR HTGetCharacter()
			
#include <ctype.h>

#include "HTML.h"
#include "HTParse.h"
#include "HTFormat.h"
#include "HTAlert.h"

#include "LYLeaks.h"

#define BIG 1024 /* @@@ */

struct _HTStructured {
	CONST HTStructuredClass *	isa;
	/* ... */
};

#define LINE_LENGTH 512			/* Maximum length of line of ARTICLE etc */
#define GROUP_NAME_LENGTH	256	/* Maximum length of group name */
extern BOOLEAN scan_for_buried_news_references;
extern BOOLEAN LYListNewsNumbers;
extern BOOLEAN LYListNewsDates;
extern HTCJKlang HTCJK;
extern int interrupted_in_htgetcharacter;

/*
**  Module-wide variables.
*/
PUBLIC  char * HTNewsHost = NULL;		/* Default host */
PRIVATE char * NewsHost = NULL;			/* Current host */
PRIVATE char * NewsHREF = NULL;			/* Current HREF prefix */
PRIVATE int s;					/* Socket for NewsHost */
PRIVATE HTCanPost = FALSE;			/* Current POST permission */
PRIVATE char response_text[LINE_LENGTH+1];	/* Last response */
/* PRIVATE HText *	HT;	*/		/* the new hypertext */
PRIVATE HTStructured * target;			/* The output sink */
PRIVATE HTStructuredClass targetClass;		/* Copy of fn addresses */
PRIVATE HTParentAnchor *node_anchor;		/* Its anchor */
PRIVATE int	diagnostic;			/* level: 0=none 2=source */

#define PUTC(c) (*targetClass.put_character)(target, c)
#define PUTS(s) (*targetClass.put_string)(target, s)
#define START(e) (*targetClass.start_element)(target, e, 0, 0, 0)
#define END(e) (*targetClass.end_element)(target, e, 0)

PUBLIC CONST char * HTGetNewsHost NOARGS
{
	return HTNewsHost;
}

PUBLIC void HTSetNewsHost ARGS1(CONST char *, value)
{
	StrAllocCopy(HTNewsHost, value);
}

/*	Initialisation for this module
**	------------------------------
**
**	Except on the NeXT, we pick up the NewsHost name from
**
**	1.	Environment variable NNTPSERVER
**	2.	File SERVER_FILE
**	3.	Compilation time macro DEFAULT_NEWS_HOST
**	4.	Default to "news"
**
**	On the NeXT, we pick up the NewsHost name from, in order:
**
**	1.	WorldWideWeb default "NewsHost"
**	2.	Global default "NewsHost"
**	3.	News default "NewsHost"
**	4.	Compilation time macro DEFAULT_NEWS_HOST
**	5.	Default to "news"
*/
PRIVATE BOOL initialized = NO;
PRIVATE BOOL initialize NOARGS
{
    /*
    **  Get name of Host.
    */
#ifdef NeXTStep
    if ((HTNewsHost = NXGetDefaultValue("WorldWideWeb","NewsHost"))==0)
        if ((HTNewsHost = NXGetDefaultValue("News","NewsHost")) == 0)
	    HTNewsHost = DEFAULT_NEWS_HOST;
#else
    if (getenv("NNTPSERVER")) {
        StrAllocCopy(HTNewsHost, (char *)getenv("NNTPSERVER"));
	if (TRACE) fprintf(stderr, "HTNews: NNTPSERVER defined as `%s'\n",
		HTNewsHost);
    } else {
        char server_name[256];
        FILE* fp = fopen(SERVER_FILE, "r");
        if (fp) {
	    if (fscanf(fp, "%s", server_name)==1) {
	        StrAllocCopy(HTNewsHost, server_name);
		if (TRACE) fprintf(stderr,
		"HTNews: File %s defines news host as `%s'\n",
		        SERVER_FILE, HTNewsHost);
	    }
	    fclose(fp);
	}
    }
    if (!HTNewsHost)
        HTNewsHost = DEFAULT_NEWS_HOST;
#endif /* NeXTStep */

    s = -1;		/* Disconnected */
    
    return YES;
}

/*	Send NNTP Command line to remote host & Check Response
**	------------------------------------------------------
**
** On entry,
**	command	points to the command to be sent, including CRLF, or is null
**		pointer if no command to be sent.
** On exit,
**	Negative status indicates transmission error, socket closed.
**	Positive status is an NNTP status.
*/
PRIVATE int response ARGS1(CONST char *,command)
{
    int result;    
    char * p = response_text;
    char ch;

    if (command) {
        int status;
	int length = strlen(command);
	if (TRACE) fprintf(stderr, "NNTP command to be sent: %s", command);
#ifdef NOT_ASCII
	{
	    CONST char  * p;
	    char 	* q;
	    char ascii[LINE_LENGTH+1];
	    for(p = command, q=ascii; *p; p++, q++) {
		*q = TOASCII(*p);
	    }
            status = NEWS_NETWRITE(s, ascii, length);
	}
#else
        status = NEWS_NETWRITE(s, (char *)command, length);
#endif /* NOT_ASCII */
	if (status < 0){
	    if (TRACE) fprintf(stderr,
	        "HTNews: Unable to send command. Disconnecting.\n");
	    NEWS_NETCLOSE(s);
	    s = -1;
	    return status;
	} /* if bad status */
    } /* if command to be sent */

    for (;;) {
	if (((*p++ = NEXT_CHAR) == LF) ||
	    (p == &response_text[LINE_LENGTH])) {
	    *--p = '\0';			/* Terminate the string */
	    if (TRACE)
	        fprintf(stderr, "NNTP Response: %s\n", response_text);
	    sscanf(response_text, "%d", &result);
	    return result;
	} /* if end of line */

	if ((ch = *(p-1)) == (char)EOF) {
	    *(p-1) = '\0';
	    if (TRACE) {
		if (interrupted_in_htgetcharacter) {
		    fprintf(stderr,
			    "HTNews: Interrupted on read, closing socket %d\n",
		   	    s);
		} else {
		    fprintf(stderr,
			    "HTNews: EOF on read, closing socket %d\n",
		   	    s);
		}
	    }
	    NEWS_NETCLOSE(s);	/* End of file, close socket */
	    s = -1;
	    if (interrupted_in_htgetcharacter) {
	        interrupted_in_htgetcharacter = 0;
		return(HT_INTERRUPTED);
	    }
	    return((int)EOF);	/* End of file on response */
	}
    } /* Loop over characters */
}

/*	Case insensitive string comparisons
**	-----------------------------------
**
** On entry,
**	template must be already un upper case.
**	unknown may be in upper or lower or mixed case to match.
*/
PRIVATE BOOL match ARGS2 (CONST char *,unknown, CONST char *,template)
{
    CONST char * u = unknown;
    CONST char * t = template;
    for (; *u && *t && (TOUPPER(*u) == *t); u++, t++)
        ; /* Find mismatch or end */
    return (BOOL)(*t == 0);		/* OK if end of template */
}

/*	Find Author's name in mail address
**	----------------------------------
**
** On exit,
**	Returns allocated string which cannot be freed by the
**	calling function, and is reallocated on subsequent calls
**	to this function.
**
** For example, returns "Tim Berners-Lee" if given any of
**	" Tim Berners-Lee <tim@online.cern.ch> "
**  or	" tim@online.cern.ch ( Tim Berners-Lee ) "
*/
PRIVATE char * author_name ARGS1 (char *,email)
{
    static char *name = NULL;
    char *s, *e;
    
    StrAllocCopy(name, email);
    if (TRACE)
        fprintf(stderr,"Trying to find name in: %s\n",name);

    if ((s = strchr(name, '(')) && (e = strchr(name, ')'))) {
        if (e > s) {
	    *e = '\0';			/* Chop off everything after the ')'  */
	    return HTStrip(s+1);	/* Remove leading and trailing spaces */
	}
    }
	
    if ((s = strchr(name, '<')) && (e = strchr(name, '>'))) {
        if (e > s) {
	    strcpy(s, e+1);		/* Remove <...> */
	    return HTStrip(name);	/* Remove leading and trailing spaces */
	}
    }
	
    return HTStrip(name);		/* Default to the whole thing */
}

/*      Find Author's mail address
**      --------------------------
**
** On exit,
**	Returns allocated string which cannot be freed by the
**	calling function, and is reallocated on subsequent calls
**	to this function.
**
** For example, returns "montulli@spaced.out.galaxy.net" if given any of
**      " Lou Montulli <montulli@spaced.out.galaxy.net> "
**  or  " montulli@spacedout.galaxy.net ( Lou "The Stud" Montulli ) "
*/
PRIVATE char * author_address ARGS1(char *,email)
{
    static char *address = NULL;
    char *s, *at, *e;

    StrAllocCopy(address, email);
    if (TRACE)
        fprintf(stderr,"Trying to find address in: %s\n",address);

    if ((s = strchr(address, '<'))) {
        if ((e = strchr(s, '>')) && (at = strchr(s, '@'))) {
	    if (at < e) {
                *e = '\0';               /* Remove > */
                return HTStrip(s+1);  /* Remove leading and trailing spaces */
	    }
	}
    }

    if ((s = strchr(address, '(')) &&
        (e = strchr(address, ')')) && (at = strchr(address, '@'))) {
        if (e > s && at < e) {
            *s = '\0';                  /* Chop off everything after the ')'  */
            return HTStrip(address);    /* Remove leading and trailing spaces */
        }
    }

    if ((at = strchr(address, '@')) && at > address) {
        s = (at - 1);
	e = (at + 1);
        while (s > address && !isspace((unsigned char)*s))
	    s--;
	while (*e && !isspace((unsigned char)*e))
	    e++;
	*e = 0;
	return HTStrip(s);
    }

    /*
    **  Default to the first word.
    */
    s = address;
    while (isspace((unsigned char)*s))
        s++; /* find first non-space */
    e = s;
    while (!isspace((unsigned char)*e) && *e != '\0')
        e++; /* find next space or end */
    *e = '\0'; /* terminate space */

    return(s);
}

/*	Start anchor element
**	--------------------
*/
PRIVATE void start_anchor ARGS1(CONST char *,  href)
{
    BOOL		present[HTML_A_ATTRIBUTES];
    CONST char*		value[HTML_A_ATTRIBUTES];
    
    {
    	int i;
    	for(i=0; i < HTML_A_ATTRIBUTES; i++)
	    present[i] = (i == HTML_A_HREF);
    }
    ((CONST char **)value)[HTML_A_HREF] = href;
    (*targetClass.start_element)(target, HTML_A , present,
    				 (CONST char **)value, 0);
}

/*      Start link element
**      ------------------
*/
PRIVATE void start_link ARGS2(CONST char *,  href, CONST char *, rev)
{
    BOOL                present[HTML_LINK_ATTRIBUTES];
    CONST char*         value[HTML_LINK_ATTRIBUTES];
   
    {
        int i;
        for(i=0; i < HTML_LINK_ATTRIBUTES; i++)
            present[i] = (i == HTML_LINK_HREF || i == HTML_LINK_REV);
    }
    ((CONST char **)value)[HTML_LINK_HREF] = href;
    ((CONST char **)value)[HTML_LINK_REV]  = rev;
    (*targetClass.start_element)(target, HTML_LINK, present,
				 (CONST char **)value, 0);
}

/*      Start list element
**      ------------------
*/
PRIVATE void start_list ARGS1(int, seqnum)
{
    BOOL                present[HTML_OL_ATTRIBUTES];
    CONST char*         value[HTML_OL_ATTRIBUTES];
    char SeqNum[20];
    int i;
   
    for (i = 0; i < HTML_OL_ATTRIBUTES; i++)
        present[i] = (i == HTML_OL_SEQNUM || i == HTML_OL_START);
    sprintf(SeqNum, "%d", seqnum);
    ((CONST char **)value)[HTML_OL_SEQNUM] = SeqNum;
    ((CONST char **)value)[HTML_OL_START]  = SeqNum;
    (*targetClass.start_element)(target, HTML_OL , present,
				 (CONST char **)value, 0);
}

/*	Paste in an Anchor
**	------------------
**
**
** On entry,
**	HT 	has a selection of zero length at the end.
**	text 	points to the text to be put into the file, 0 terminated.
**	addr	points to the hypertext refernce address,
**		terminated by white space, comma, NULL or '>' 
*/
PRIVATE void write_anchor ARGS2(CONST char *,text, CONST char *,addr)
{
    char href[LINE_LENGTH+1];
		
    {
    	CONST char * p;
	strcpy(href, NewsHREF);
	for (p = addr; *p && (*p != '>') && !WHITE(*p) && (*p!=','); p++)
	    ;
        strncat(href, addr, p-addr);	/* Make complete hypertext reference */
    }
    
    start_anchor(href);
    PUTS(text);
    END(HTML_A);
}

/*	Write list of anchors
**	---------------------
**
**	We take a pointer to a list of objects, and write out each,
**	generating an anchor for each.
**
** On entry,
**	HT 	has a selection of zero length at the end.
**	text 	points to a comma or space separated list of addresses.
** On exit,
**	*text	is NOT any more chopped up into substrings.
*/
PRIVATE void write_anchors ARGS1 (char *,text)
{
    char * start = text;
    char * end;
    char c;
    for (;;) {
        for (; *start && (WHITE(*start)); start++)
	    ;  /* Find start */
	if (!*start)
	    return;			/* (Done) */
        for (end = start;
	     *end && (*end != ' ') && (*end != ','); end++)
	    ;/* Find end */
	if (*end)
	    end++;	/* Include comma or space but not NULL */
	c = *end;
	*end = '\0';
	if (*start == '<')
	    write_anchor(start, start+1);
	else
	    write_anchor(start, start);
	START(HTML_BR);
	*end = c;
	start = end;			/* Point to next one */
    }
}

/*	Abort the connection					abort_socket
**	--------------------
*/
PRIVATE void abort_socket NOARGS
{
    if (TRACE)
        fprintf(stderr,
	        "HTNews: EOF on read, closing socket %d\n", s);
    NEWS_NETCLOSE(s);	/* End of file, close socket */
    PUTS("Network Error: connection lost");
    PUTC('\n');
    s = -1;		/* End of file on response */
}

/*
**  Determine if a line is a valid header line.			valid_header
**  -------------------------------------------
*/
PRIVATE BOOLEAN valid_header ARGS1(
	char *,		line)
{
    char *colon, *space;

    /*
    **  Blank or tab in first position implies
    **  this is a continuation header.
    */
    if (line[0] == ' ' || line[0] == '\t')
	return(TRUE);

    /*
    **  Just check for initial letter, colon, and space to make
    **  sure we discard only invalid headers.
    */
    colon = strchr(line, ':');
    space = strchr(line, ' ');
    if (isalpha(line[0]) && colon && space == colon + 1)
        return(TRUE);

    /*
    **  Anything else is a bad header -- it should be ignored.
    */
    return(FALSE);
}

/*	post in an Article					post_article
**	------------------
**  			(added by FM, modeled on Lynx's previous mini inews)
**
**	Note the termination condition of a single dot on a line by itself.
**
**  On entry,
**	s		Global socket number is OK
**	postfile	file with header and article to post.
*/ 
PRIVATE void post_article ARGS1(
	char *,		postfile)
{
    char line[512];
    char buf[512];
    char crlf[3];
    char *cp;
    int status;
    FILE *fd;
    int in_header = 1, seen_header = 0, seen_fromline = 0;
    int blen = 0, llen = 0;


    /*
    **  Open the temporary file with the
    **  nntp headers and message body. - FM
    */
    if ((fd = fopen((postfile ? postfile : ""), "r")) == NULL) {
	HTAlert("Cannot open temporary file for news POST.");
	return;
    }

    /*
    **  Read the temporary file and post
    **  in maximum 512 byte chunks. - FM
    */
    buf[0] = '\0';
    sprintf(crlf, "%c%c", CR, LF);
    while (fgets(line, sizeof(line), fd) != NULL) {
	if ((cp = strchr(line, '\n')) != NULL)
	    *cp = '\0';
	if (line[0] == '.') {
	    /*
	    **  A single '.' means end of transmission
	    **  for nntp.  Lead dots on lines normally
	    **  are trimmed and the EOF is not registered
	    **  if the dot was not followed by CRLF.
	    **  We prepend an extra dot for any line
	    **  beginning with one, to retain the one
	    **  intended, as well as avoid a false EOF
	    **  signal.  We know we have room for it in
	    **  the buffer, because we normally send when
	    **  it would exceed 510. - FM
	    */
	    strcat(buf, ".");
	    blen++;
	}
	llen = strlen(line);
	if (in_header && !strncasecomp(line, "From:", 5)) {
	    seen_header = 1;
	    seen_fromline = 1;
	}
	if (in_header && line[0] == '\0') {
	    if (seen_header) {
		in_header = 0;
		if (!seen_fromline) {
		    if (blen < 475) {
		        strcat(buf, "From: anonymous@nowhere.you.know");
			strcat(buf, crlf);
			blen += 34;
		    } else {
			NEWS_NETWRITE(s, buf, blen);
			sprintf(buf,
				"From: anonymous@nowhere.you.know%s", crlf);
			blen = 34;
		    }
		}
	     } else {
		continue;
	    }
	} else if (in_header) {
	    if (valid_header(line)) {
		seen_header = 1;
	    } else {
		continue;
	    }
	}
	strcat(line, crlf);
	llen += 2;
	if ((blen + llen) < 511) {
	    strcat(buf, line);
	    blen += llen;
	} else {
	    NEWS_NETWRITE(s, buf, blen);
	    strcpy(buf, line);
	    blen = llen;
	}
    }

    /*
    **  Send the nntp EOF and get the server's response. - FM
    */
    if (blen < 508) {
        strcat(buf, ".");
	strcat(buf, crlf);
	blen += 3;
	NEWS_NETWRITE(s, buf, blen);
    } else {
        NEWS_NETWRITE(s, buf, blen);
	sprintf(buf, ".%s", crlf);
	blen = 3;
	NEWS_NETWRITE(s, buf, blen);
    }
    status = response(NULL);
    if (status == 240) {
        /*
	**  Successful post. - FM
	*/
        HTProgress(response_text);
    } else {
        /*
	**  Shucks, something went wrong. - FM
	*/
        HTAlert(response_text);
    }
}

/*	Read in an Article					read_article
**	------------------
**
**	Note the termination condition of a single dot on a line by itself.
**	RFC 977 specifies that the line "folding" of RFC850 is not used, so we
**	do not handle it here.
**
** On entry,
**	s	Global socket number is OK
**	HT	Global hypertext object is ready for appending text
*/ 
PRIVATE int read_article NOARGS
{
    char line[LINE_LENGTH+1];
    char *full_line = NULL;
    char *subject=NULL;				/* Subject string	    */
    char *from=NULL;				/* From string		    */
    char *replyto=NULL;				/* Reply-to string	    */
    char *date=NULL;				/* Date string		    */
    char *organization=NULL;			/* Organization string	    */
    char *references=NULL;			/* Hrefs for other articles */
    char *newsgroups=NULL;			/* Newsgroups list	    */
    char *followupto=NULL;			/* Followup list	    */
    char *href=NULL;
    char *p = line;
    BOOL done = NO;

    /*
    **  Read in the HEADer of the article.
    **
    **  The header fields are either ignored,
    **  or formatted and put into the text.
    */
    if (!diagnostic) {
	while (!done) {
	    char ch = *p++ = NEXT_CHAR;
	    if (ch == (char)EOF) {
	        if (interrupted_in_htgetcharacter) {
		    interrupted_in_htgetcharacter = 0;
		    if (TRACE)
		        fprintf(stderr,
			   "HTNews: Interrupted on read, closing socket %d\n",
				s);
		    NEWS_NETCLOSE(s);
		    s = -1;
		    return(HT_INTERRUPTED);
		}
		abort_socket();		/* End of file, close socket */
	    	return(HT_LOADED);	/* End of file on response */
	    }
	    if ((ch == LF) || (p == &line[LINE_LENGTH])) {
		*--p = '\0';			/* Terminate the string */
		if (TRACE)
		    fprintf(stderr, "H %s\n", line);

		if (line[0] == '\t' || line[0] == ' ') {
		    int i = 0;
		    while (line[i]) {
		        if (line[i] == '\t')
			    line[i] = ' ';
			i++;
		    }
		    if (full_line == NULL) {
		        StrAllocCopy(full_line, line);
		    } else {
		        StrAllocCat(full_line, line);
		    }
		} else {
		    StrAllocCopy(full_line, line);
		}

		if (full_line[0] == '.') {	
		    if (full_line[1] < ' ') {		/* End of article? */
			done = YES;
			break;
		    }
		} else if (full_line[0] < ' ') {
		    break;		/* End of Header? */

		} else if (match(full_line, "SUBJECT:")) {
		    StrAllocCopy(subject, HTStrip(strchr(full_line,':')+1));
		    if (HTCJK == JAPANESE) {
		        HTmmdecode(subject, subject);
			HTrjis(subject, subject);
		    }

		} else if (match(full_line, "DATE:")) {
		    StrAllocCopy(date, HTStrip(strchr(full_line,':')+1));

		} else if (match(full_line, "ORGANIZATION:")) {
		    StrAllocCopy(organization,
		    		 HTStrip(strchr(full_line,':')+1));
		    if (HTCJK == JAPANESE) {
		        HTmmdecode(organization, organization);
			HTrjis(organization, organization);
		    }

		} else if (match(full_line, "FROM:")) {
		    StrAllocCopy(from, HTStrip(strchr(full_line,':')+1));
		    if (HTCJK == JAPANESE) {
		        HTmmdecode(from, from);
			HTrjis(from, from);
		    }

		} else if (match(full_line, "REPLY-TO:")) {
		    StrAllocCopy(replyto, HTStrip(strchr(full_line,':')+1));
		    if (HTCJK == JAPANESE) {
		        HTmmdecode(replyto, replyto);
			HTrjis(replyto, replyto);
		    }

		} else if (match(full_line, "NEWSGROUPS:")) {
		    StrAllocCopy(newsgroups, HTStrip(strchr(full_line,':')+1));

		} else if (match(full_line, "REFERENCES:")) {
		    StrAllocCopy(references, HTStrip(strchr(full_line,':')+1));

		} else if (match(full_line, "FOLLOWUP-TO:")) {
		    StrAllocCopy(followupto, HTStrip(strchr(full_line,':')+1));

		} /* end if match */
		p = line;			/* Restart at beginning */
	    } /* if end of line */
	} /* Loop over characters */
	FREE(full_line);

	START(HTML_HEAD);
	PUTC('\n');
	START(HTML_TITLE);
	if (subject && *subject != '\0')
	    PUTS(subject);
	else
	    PUTS("No Subject");
	END(HTML_TITLE);
	PUTC('\n');
	/*
	**  Put in the owner as a link rel.
	*/
	if (from || replyto) {
	    char *temp=NULL;
	    StrAllocCopy(temp, replyto ? replyto : from);
	    StrAllocCopy(href,"mailto:");
	    StrAllocCat(href, author_address(temp));
	    start_link(href, "made");
	    PUTC('\n');
	    FREE(temp);
	}
	END(HTML_HEAD);
	PUTC('\n');

	START(HTML_H1);
	if (subject && *subject != '\0')
	    PUTS(subject);
	else
	    PUTS("No Subject");
	END(HTML_H1);
	PUTC('\n');

	if (subject)
	    FREE(subject);

	START(HTML_DLC);
	PUTC('\n');

	if (from || replyto) {
	    START(HTML_DT);
	    START(HTML_B);
	    PUTS("From:");
	    END(HTML_B);
	    PUTC(' ');
	    if (from)
		PUTS(from);
	    else
		PUTS(from);
	    PUTC('\n');

	    if (!replyto)
		StrAllocCopy(replyto, from);
	    START(HTML_DT);
	    START(HTML_B);
	    PUTS("Reply to:");
	    END(HTML_B);
            PUTC(' ');
	    start_anchor(href);
	    if (*replyto != '<')
    	        PUTS(author_name(replyto));
	    else
    	        PUTS(author_address(replyto));
     	    END(HTML_A);
	    START(HTML_BR);
	    PUTC('\n');

	    FREE(from);
  	    FREE(replyto);
	}

	if (date) {
	    START(HTML_DT);
	    START(HTML_B);
	    PUTS("Date:");
	    END(HTML_B);
            PUTC(' ');
	    PUTS(date);
	    PUTC('\n');
	    FREE(date);
	}

	if (organization) {
	    START(HTML_DT);
	    START(HTML_B);
	    PUTS("Organization:");
	    END(HTML_B);
            PUTC(' ');
	    PUTS(organization);
	    PUTC('\n');
	    FREE(organization);
	}

	if (newsgroups && HTCanPost) {
	    /*
	    **  We have permission to POST to this host,
	    **  so add a link for posting followups for
	    **  this article. - FM
	    */
	    if (!strncasecomp(NewsHREF, "snews:", 6))
	        StrAllocCopy(href,"snewsreply://");
	    else
	        StrAllocCopy(href,"newsreply://");
	    StrAllocCat(href, NewsHost);
	    StrAllocCat(href, "/");
	    StrAllocCat(href, (followupto ? followupto : newsgroups));

	    START(HTML_DT);
	    START(HTML_B);
	    PUTS("Newsgroups:");
	    END(HTML_B);
	    PUTC('\n');
	    START(HTML_DD);
	    write_anchors(newsgroups);
	    PUTC('\n');

	    START(HTML_DT);
	    START(HTML_B);
            PUTS("Followup to:");
	    END(HTML_B);
            PUTC(' ');
            start_anchor(href);
            PUTS("newsgroup(s)");
            END(HTML_A);
	    PUTC('\n');
	}
	FREE(newsgroups);
	FREE(followupto);
	    
	if (references) {
	    START(HTML_DT);
	    START(HTML_B);
	    PUTS("References:");
	    END(HTML_B);
	    PUTC('\n');
	    START(HTML_DD);
	    write_anchors(references);
	    PUTC('\n');
	    FREE(references);
	}

	END(HTML_DLC);
	PUTC('\n');
	FREE(href);
    }

    if (diagnostic) {
        /*
	**  Read in the HEAD and BODY of the Article
	**  as XMP formatted text. - FM
	*/
	START(HTML_XMP);
    } else {
        /*
	**  Read in the BODY of the Article
	**  as PRE formatted text. - FM
	*/
	START(HTML_PRE);
    }
    PUTC('\n');

    p = line;
    while (!done) {
	char ch = *p++ = NEXT_CHAR;
	if (ch == (char)EOF) {
	    if (interrupted_in_htgetcharacter) {
		interrupted_in_htgetcharacter = 0;
		if (TRACE)
		    fprintf(stderr,
			  "HTNews: Interrupted on read, closing socket %d\n",
			    s);
		NEWS_NETCLOSE(s);
		s = -1;
		return(HT_INTERRUPTED);
	    }
	    abort_socket();	/* End of file, close socket */
	    return(HT_LOADED);	/* End of file on response */
	}
	if ((ch == LF) || (p == &line[LINE_LENGTH])) {
	    *p++ = '\0';			/* Terminate the string */
	    if (TRACE)
	        fprintf(stderr, "B %s", line);
	    if (line[0] == '.') {
		if (line[1] < ' ') {		/* End of article? */
		    done = YES;
		    break;
		} else {			/* Line starts with dot */
		    PUTS(&line[1]);	/* Ignore first dot */
		}
	    } else {
	        if (diagnostic || !scan_for_buried_news_references) {
		    /*
		    **  All lines are passed as unmodified source. - FM
		    */
	            PUTS(line);
	        } else {
		    /*
		    **  Normal lines are scanned for buried references
		    **  to other articles.  Unfortunately, it could pick
		    **  up mail addresses as well!  It also can corrupt
		    **  uuencoded messages!  So we don't do this when
		    **  fetching articles as WWW_SOURCE or when downloading
		    **  (diagnostic is TRUE) or if the client has set
		    **  scan_for_buried_news_references to FALSE.
		    **  Otherwise, we convert all "<...@...>" strings
		    **  preceded by "rticle " to "news:...@..." links,
		    **  and any strings that look like URLs to links. - FM
		    */
		    char *l = line;
		    char *p;

		    while (p=strstr(l, "rticle <")) {
		        char *q  = strchr(p,'>');
		        char *at = strchr(p, '@');
		        if (q && at && at<q) {
		            char c = q[1];
			    q[1] = 0;		/* chop up */
			    p += 7;
			    *p = 0;
			    while (*l) {
			        if (strncmp (l, "news:", 5) &&
				    strncmp (l, "snews://", 8) &&
				    strncmp (l, "nntp://", 7) &&
				    strncmp (l, "ftp://", 6) &&
				    strncmp (l, "file:/", 6) &&
				    strncmp (l, "finger://", 9) &&
				    strncmp (l, "http://", 7) &&
				    strncmp (l, "https://", 8) &&
				    strncmp (l, "wais://", 7) &&
				    strncmp (l, "mailto:", 7) &&
				    strncmp (l, "cso://", 6) &&
				    strncmp (l, "gopher://", 9)) 
				    PUTC (*l++);
				else {
				    StrAllocCopy(href, l);
				    start_anchor(strtok(href, " \r\n\t,>)\""));
				    while (*l && !strchr(" \r\n\t,>)\"", *l))
				        PUTC(*l++);
				    END(HTML_A);
				    FREE(href);
				}
			    }
			    *p = '<'; 		/* again */
			    *q = 0;
			    start_anchor(p+1);
			    *q = '>'; 		/* again */
			    PUTS(p);
			    END(HTML_A);
			    q[1] = c;		/* again */
			    l=q+1;
		        } else {
			    break;		/* line has unmatched <> */
			}
		    }
		    while (*l) {		/* Last bit of the line */
			if (strncmp (l, "news:", 5) &&
			    strncmp (l, "snews://", 8) &&
			    strncmp (l, "nntp://", 7) &&
			    strncmp (l, "ftp://", 6) &&
			    strncmp (l, "file:/", 6) &&
			    strncmp (l, "finger://", 9) &&
			    strncmp (l, "http://", 7) &&
			    strncmp (l, "https://", 8) &&
			    strncmp (l, "wais://", 7) &&
			    strncmp (l, "mailto:", 7) &&
			    strncmp (l, "cso://", 6) &&
			    strncmp (l, "gopher://", 9)) 
			    PUTC (*l++);
			else {
			    StrAllocCopy(href, l);
			    start_anchor(strtok(href, " \r\n\t,>)\""));
			    while (*l && !strchr(" \r\n\t,>)\"", *l))
			        PUTC(*l++);
			    END(HTML_A);
			    FREE(href);
			}
		    }
	        } /* if diagnostic or not scan_for_buried_news_references */
	    } /* if not dot */
	    p = line;				/* Restart at beginning */
	} /* if end of line */
    } /* Loop over characters */
    
    if (diagnostic)
	END(HTML_XMP);
    else
	END(HTML_PRE);
    PUTC('\n');
    return(HT_LOADED);
}

/*	Read in a List of Newsgroups
**	----------------------------
**
**  Note the termination condition of a single dot on a line by itself.
**  RFC 977 specifies that the line "folding" of RFC850 is not used,
**  so we do not handle it here.
*/        
PRIVATE int read_list ARGS1(char *, arg)
{
    char line[LINE_LENGTH+1];
    char *p;
    BOOL done = NO;
    BOOL head = NO;
    BOOL tail = NO;
    int listing = 0;
    char *pattern = NULL;
    int len = 0;
    
    /*
    **  Support head or tail matches for groups to list. - FM
    */
    if (arg && strlen(arg) > 1) {
        if (*arg == '*') {
            tail = YES;
	    StrAllocCopy(pattern, (arg+1));
        } else if (arg[strlen(arg)-1] == '*') {
            head = YES;
	    StrAllocCopy(pattern, arg);
	    pattern[strlen(pattern)-1] = '\0';
        }
        if (tail || head) {
           len = strlen(pattern);
	}

    }

    /*
    **  Read in the HEADer of the article.
    **
    **  The header fields are either ignored,
    **  or formatted and put into the text.
    */
    START(HTML_HEAD);
    PUTC('\n');
    START(HTML_TITLE);
    PUTS("Newsgroups");
    END(HTML_TITLE);
    PUTC('\n');
    END(HTML_HEAD);
    PUTC('\n');
    START(HTML_H1);
    PUTS( "Newsgroups");
    END(HTML_H1);
    PUTC('\n');
    p = line;
    START(HTML_DLC);
    PUTC('\n');
    while (!done) {
	char ch = *p++ = NEXT_CHAR;
	if (ch == (char)EOF) {
	    if (interrupted_in_htgetcharacter) {
		interrupted_in_htgetcharacter = 0;
		if (TRACE)
		    fprintf(stderr,
			  "HTNews: Interrupted on read, closing socket %d\n",
			    s);
		NEWS_NETCLOSE(s);
		s = -1;
		return(HT_INTERRUPTED);
	    }
	    abort_socket();	/* End of file, close socket */
	    FREE(pattern);
	    return(HT_LOADED);	/* End of file on response */
	}
	if ((ch == LF) || (p == &line[LINE_LENGTH])) {
	    *p++ = '\0';			/* Terminate the string */
	    if (TRACE)
	        fprintf(stderr, "B %s", line);
	    if (line[0] == '.') {
		if (line[1] < ' ') {		/* End of article? */
		    done = YES;
		    break;
		} else {			/* Line starts with dot */
		    START(HTML_DT);
		    PUTS(&line[1]);
		}
	    } else {
		/*
		**  Normal lines are scanned for references to newsgroups.
		*/
		int i = 0;

		/* find whitespace if it exits */
		for (; line[i] != '\0' && !WHITE(line[i]); i++)
		    ;  /* null body */
	
		if (line[i] != '\0') {
		    line[i] = '\0';
		    if ((head && strncasecomp(line, pattern, len)) ||
		        (tail && (i < len ||
				  strcasecomp((line + (i - len)), pattern)))) {
		        p = line;	/* Restart at beginning */
			continue;
		    }
		    START(HTML_DT);
		    write_anchor(line, line);
		    listing++;
		    PUTC('\n');
    	            START(HTML_DD);
		    PUTS(&line[i+1]); /* put description */
		} else {
		    if ((head && strncasecomp(line, pattern, len)) ||
		        (tail && (i < len ||
				  strcasecomp((line + (i - len)), pattern)))) {
		        p = line;	/* Restart at beginning */
			continue;
		    }
		    START(HTML_DT);
		    write_anchor(line, line);
		    listing++;
		}
	    } /* if not dot */
	    p = line;			/* Restart at beginning */
	} /* if end of line */
    } /* Loop over characters */
    if (!listing) {
        START(HTML_DT);
	sprintf(line, "No matches for: %s", arg);
	PUTS(line);
    }
    END(HTML_DLC);
    PUTC('\n');
    FREE(pattern);
    return(HT_LOADED);
}

/*	Read in a Newsgroup
**	-------------------
**
**  Unfortunately, we have to ask for each article one by one if we
**  want more than one field.
**
*/
PRIVATE int read_group ARGS3(
  CONST char *,groupName,
  int,first_required,
  int,last_required)
{
    char line[LINE_LENGTH+1];
    char author[LINE_LENGTH+1];
    char subject[LINE_LENGTH+1];
    char *date = NULL;
    int i;
    char *p;
    BOOL done;

    char buffer[LINE_LENGTH];
    char *reference = NULL;		/* Href for article */
    int art;				/* Article number WITHIN GROUP */
    int status, count, first, last;	/* Response fields */
					/* count is only an upper limit */

    author[0] = '\0';
    START(HTML_HEAD);
    PUTC('\n');
    START(HTML_TITLE);
    PUTS("Newsgroup ");
    PUTS(groupName);
    END(HTML_TITLE);
    PUTC('\n');
    END(HTML_HEAD);
    PUTC('\n');

    sscanf(response_text, " %d %d %d %d", &status, &count, &first, &last);
    if (TRACE)
        fprintf(stderr,
    		"Newsgroup status=%d, count=%d, (%d-%d) required:(%d-%d)\n",
		status, count, first, last, first_required, last_required);
    if (last == 0) {
        PUTS("\nNo articles in this group.\n");
	goto add_post;
    }
    
#define FAST_THRESHOLD 100	/* Above this, read IDs fast */
#define CHOP_THRESHOLD 50	/* Above this, chop off the rest */

    if (first_required < first)
        first_required = first;		/* clip */
    if ((last_required == 0) || (last_required > last))
        last_required = last;
    
    if (last_required < first_required) {
        PUTS("\nNo articles in this range.\n");
	goto add_post;
    }

    if (last_required-first_required+1 > HTNewsMaxChunk) { /* Trim this block */
        first_required = last_required-HTNewsChunkSize+1;
    }
    if (TRACE)
        fprintf(stderr, "    Chunk will be (%d-%d)\n",
    		        first_required, last_required);

    /*
    **  Set window title.
    */
    sprintf(buffer, "%s,  Articles %d-%d",
    		    groupName, first_required, last_required);
    START(HTML_H1);
    PUTS(buffer);
    END(HTML_H1);
    PUTC('\n');

    /*
    **  Link to earlier articles.
    */
    if (first_required > first) {
    	int before;			/* Start of one before */
	if (first_required-HTNewsMaxChunk <= first)
	    before = first;
	else
	    before = first_required-HTNewsChunkSize;
    	sprintf(buffer, "%s%s/%d-%d", NewsHREF, groupName,
				      before, first_required-1);
	if (TRACE)
	    fprintf(stderr, "    Block before is %s\n", buffer);
	PUTC('(');
	start_anchor(buffer);
	PUTS("Earlier articles");
	END(HTML_A);
	PUTS("...)\n");
	START(HTML_P);
	PUTC('\n');
    }
    
    done = NO;

/*#define USE_XHDR*/
#ifdef USE_XHDR
    if (count > FAST_THRESHOLD)  {
        sprintf(buffer,
 "\nThere are about %d articles currently available in %s, IDs as follows:\n\n",
		count, groupName); 
        PUTS(buffer);
        sprintf(buffer, "XHDR Message-ID %d-%d%c%c", first, last, CR, LF);
	status = response(buffer);
	if (status == 221) {
	    p = line;
	    while (!done) {
		char ch = *p++ = NEXT_CHAR;
		if (ch == (char)EOF) {
		    if (interrupted_in_htgetcharacter) {
		        interrupted_in_htgetcharacter = 0;
			if (TRACE)
			    fprintf(stderr,
			   "HTNews: Interrupted on read, closing socket %d\n",
				    s);
			NEWS_NETCLOSE(s);
			s = -1;
			return(HT_INTERRUPTED);
		    }
		    abort_socket();	/* End of file, close socket */
		    return(HT_LOADED);	/* End of file on response */
		}
		if ((ch == '\n') || (p == &line[LINE_LENGTH])) {
		    *p++ = '\0';		/* Terminate the string */
		    if (TRACE)
		        fprintf(stderr, "X %s", line);
		    if (line[0] == '.') {
			if (line[1] < ' ') {	/* End of article? */
			    done = YES;
			    break;
			} else {		/* Line starts with dot */
			    	/* Ignore strange line */
			}
		    } else {
			/*
			**  Normal lines are scanned for
			**  references to articles.
			*/
			char * space = strchr(line, ' ');
			if (space++)
			    write_anchor(space, space);
		    } /* if not dot */
		    p = line;			/* Restart at beginning */
		} /* if end of line */
	    } /* Loop over characters */

	    /* leaving loop with "done" set */
	} /* Good status */
    }
#endif /* USE_XHDR */

    /*
    **  Read newsgroup using individual fields.
    */
    if (!done) {
        START(HTML_B);
        if (first == first_required && last == last_required)
	    PUTS("All available articles in ");
        else
	    PUTS("Articles in ");
	PUTS(groupName);
	END(HTML_B);
	PUTC('\n');
	if (LYListNewsNumbers)
	    start_list(first_required);
	else
	    START(HTML_UL);
	for (art = first_required; art <= last_required; art++) {
/*#define OVERLAP*/
#ifdef OVERLAP
	    /*
	    **  With this code we try to keep the server running flat out
	    **  by queuing just one extra command ahead of time.
	    **  We assume (1) that the server won't abort if it gets input
	    **  during output, and (2) that TCP buffering is enough for the
	    **  two commands.  Both these assumptions seem very reasonable.
	    **  However, we HAVE had a hangup with a loaded server.
	    */
	    if (art == first_required) {
		if (art == last_required) {		/* Only one */
		    sprintf(buffer, "HEAD %d%c%c",
		    		    art, CR, LF);
		    status = response(buffer);
		} else {				/* First of many */
		    sprintf(buffer, "HEAD %d%c%cHEAD %d%c%c",
				    art, CR, LF, art+1, CR, LF);
			status = response(buffer);
		}
	    } else if (art == last_required) {		/* Last of many */
		status = response(NULL);
	    } else {					/* Middle of many */
		sprintf(buffer, "HEAD %d%c%c", art+1, CR, LF);
		status = response(buffer);
	    }
#else	/* Not OVERLAP: */
	    sprintf(buffer, "HEAD %d%c%c", art, CR, LF);
	    status = response(buffer);
#endif	/* OVERLAP */
	    /*
	    **  Check for a good response (221) for the HEAD request,
	    **  and if so, parse it.  Otherwise, indicate the error
	    **  so that the number of listings corresponds to what's
	    **  claimed for the range, and if we are listing numbers
	    **  via an ordered list, they stay in synchrony with the
	    **  article numbers. - FM
	    */
	    if (status == 221) {	/* Head follows - parse it:*/
		p = line;				/* Write pointer */
		done = NO;
		while( !done ) {
		    char ch = *p++ = NEXT_CHAR;
		    if (ch == (char)EOF) {
			if (interrupted_in_htgetcharacter) {
			    interrupted_in_htgetcharacter = 0;
			    if (TRACE)
				fprintf(stderr,
			   "HTNews: Interrupted on read, closing socket %d\n",
					s);
			    NEWS_NETCLOSE(s);
			    s = -1;
			    return(HT_INTERRUPTED);
			}
			abort_socket();		/* End of file, close socket */
			return(HT_LOADED);	/* End of file on response */
		    }
		    if ((ch == LF) ||
		        (p == &line[LINE_LENGTH])) {
		    
			*--p = '\0';		/* Terminate  & chop LF*/
			p = line;		/* Restart at beginning */
			if (TRACE)
			    fprintf(stderr, "G %s\n", line);
			switch(line[0]) {
    
			case '.':
			    done = (line[1] < ' ');	/* End of article? */
			    break;
    
			case 'S':
			case 's':
			    if (match(line, "SUBJECT:"))
				strcpy(subject, line+9);/* Save subject */
			    break;
    
			case 'M':
			case 'm':
			    if (match(line, "MESSAGE-ID:")) {
				char * addr = HTStrip(line+11) +1; /* Chop < */
				addr[strlen(addr)-1] = '\0';	   /* Chop > */
				StrAllocCopy(reference, addr);
			    }
			    break;
    
			case 'f':
			case 'F':
			    if (match(line, "FROM:")) {
				char * p;
				strcpy(author,
					author_name(strchr(line,':')+1));
				p = author + strlen(author) - 1;
				if (*p==LF)
				    *p = '\0';	/* Chop off newline */
			    }
			    break;
				    
			case 'd':
			case 'D':
			    if (LYListNewsDates && match(line, "DATE:")) {
			        StrAllocCopy(date,
					     HTStrip(strchr(line,':')+1));
			    }
			    break;
				    
			} /* end switch on first character */
		    } /* if end of line */
		} /* Loop over characters */

		PUTC('\n');
		START(HTML_LI);
		sprintf(buffer, "\"%s\"", subject);
		if (reference) {
		    write_anchor(buffer, reference);
		    FREE(reference);
		} else {
		    PUTS(buffer);
		}
		if (author[0] != '\0') {
		     PUTS(" - ");
		     if (LYListNewsDates)
		         START(HTML_I);
		     PUTS(author);
		     if (LYListNewsDates)
		         END(HTML_I);
		     author[0] = '\0';
		}
		if (date) {
		    if (!diagnostic) {
		        for (i = 0; date[i]; i++) {
			    if (date[i] == ' ') {
			        date[i] = HT_NON_BREAK_SPACE;
			    }
			}
		    }
		    sprintf(buffer, " [%s]", date);
		    PUTS(buffer);
		    FREE(date);
		}
		/*
		**  Indicate progress!   @@@@@@
		*/
	    } else if (status == HT_INTERRUPTED) {
		interrupted_in_htgetcharacter = 0;
		if (TRACE)
		    fprintf(stderr,
			  "HTNews: Interrupted on read, closing socket %d\n",
			    s);
		NEWS_NETCLOSE(s);
		s = -1;
		return(HT_INTERRUPTED);
	    } else {
	        /*
		**  Use the response text on error. - FM
		*/
		PUTC('\n');
		START(HTML_LI);
		START(HTML_I);
		if (LYListNewsNumbers)
		    strcpy(buffer, "Status:");
		else
		    sprintf(buffer, "Status (ARTICLE %d):", art);
		PUTS(buffer);
		END(HTML_I);
		PUTC(' ');
		PUTS(response_text);
	    } /* Handle response to HEAD request */
	} /* Loop over article */	    
    } /* If read headers */
    PUTC('\n');
    if (LYListNewsNumbers)
        END(HTML_OL);
    else
        END(HTML_UL);
    PUTC('\n');
    
    /*
    **  Link to later articles.
    */
    if (last_required < last) {
    	int after;			/* End of article after */
	after = last_required+HTNewsChunkSize;
    	if (after == last)
	    sprintf(buffer, "%s%s", NewsHREF, groupName); /* original group */
    	else
	    sprintf(buffer, "%s%s/%d-%d", NewsHREF, groupName,
	    				  last_required+1, after);
	if (TRACE)
	    fprintf(stderr, "    Block after is %s\n", buffer);
	PUTC('(');
	start_anchor(buffer);
	PUTS("Later articles");
	END(HTML_A);
	PUTS("...)\n");
    }

add_post:
    if (HTCanPost) {
	/*
	**  We have permission to POST to this host,
	**  so add a link for posting messages to
	**  this newsgroup. - FM
	*/
	char *href = NULL;

	START(HTML_HR);
	PUTC('\n');
	if (!strncasecomp(NewsHREF, "snews:", 6))
	    StrAllocCopy(href,"snewspost://");
	else
	    StrAllocCopy(href,"newspost://");
	StrAllocCat(href, NewsHost);
	StrAllocCat(href, "/");
	StrAllocCat(href,groupName);
	start_anchor(href);
	PUTS("Post to ");
	PUTS(groupName);
	END(HTML_A);
	FREE(href);
    } else {
	START(HTML_HR);
    }
    PUTC('\n');
    return(HT_LOADED);
}

/*	Load by name.						HTLoadNews
**	=============
*/
PUBLIC int HTLoadNews ARGS4(
	CONST char *,		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		stream)
{
    char command[260];			/* The whole command */
    char proxycmd[260];			/* The proxy command */
    char groupName[GROUP_NAME_LENGTH];	/* Just the group name */
    int status;				/* tcp return */
    int retries;			/* A count of how hard we have tried */ 
    BOOL group_wanted;		/* Flag: group was asked for, not article */
    BOOL list_wanted;		/* Flag: list was asked for, not article */
    BOOL post_wanted;		/* Flag: new post to group was asked for */
    BOOL reply_wanted;		/* Flag: followup post was asked for */
    BOOL spost_wanted;		/* Flag: new SSL post to group was asked for */
    BOOL sreply_wanted;		/* Flag: followup SSL post was asked for */
    int first, last;		/* First and last articles asked for */
    char *cp;
    char *ListArg = NULL;
    char *postfile = NULL;

    diagnostic = (format_out == WWW_SOURCE ||	/* set global flag */
    		  format_out == HTAtom_for("www/download") ||
		  format_out == HTAtom_for("www/dump"));
    
    if (TRACE) fprintf(stderr, "HTNews: Looking for %s\n", arg);
    
    if (!initialized)
	initialized = initialize();
    if (!initialized)
	return -1;	/* FAIL */
    
    FREE(NewsHREF);
    command[0] = '\0';
    command[259] = '\0';
    proxycmd[0] = '\0';
    proxycmd[259] = '\0';

    {
        CONST char * p1 = arg;

	/*
	**  We will ask for the document, omitting the host name & anchor.
	**
	**  Syntax of address is
	**  	xxx@yyy			Article
	**  	<xxx@yyy>		Same article
	**  	xxxxx			News group (no "@")
	**  	group/n1-n2		Articles n1 to n2 in group
	*/
	post_wanted = (strstr(arg, "newspost:") != NULL);
	reply_wanted = ((!post_wanted) &&
			strstr(arg, "newsreply:") != NULL);
	spost_wanted = (!(post_wanted || reply_wanted) &&
			strstr(arg, "snewspost:") != NULL);
	sreply_wanted = (!(post_wanted || reply_wanted ||
			   spost_wanted) &&
			 strstr(arg, "newsreply:") != NULL);
	group_wanted = (!(post_wanted || reply_wanted ||
			  spost_wanted || sreply_wanted) &&
			strchr(arg, '@') == NULL) && (strchr(arg, '*') == NULL);
	list_wanted  = (!(post_wanted || reply_wanted) &&
			strchr(arg, '@') == NULL) && (strchr(arg, '*') != NULL);

	if (!strncasecomp(arg, "snewspost:", 10) ||
	    !strncasecomp(arg, "snewsreply:", 11)) {
	    HTAlert(
	"This client does not contain support for posting to news with SSL.");
	    return HT_NOT_LOADED;
	}
	if (post_wanted || reply_wanted || spost_wanted || sreply_wanted) {
	    /*
	    **  Make sure we have a non-zero path for the newsgroup(s). - FM
	    */
	    if ((p1 = strrchr(arg, '/')) != NULL) {
	        p1++;
	    } else if ((p1 = strrchr(arg, ':')) != NULL) {
	        p1++;
	    }
	    if (!(p1 && *p1)) {
	    	HTAlert("Invalid URL!");
		return(HT_NO_DATA);
	    }
	    if (!(cp = HTParse(arg, "", PARSE_HOST)) || *cp == '\0') {
		if (s >= 0 && NewsHost && strcasecomp(NewsHost, HTNewsHost)) {
		    NEWS_NETCLOSE(s);
		    s = -1;
		}
		StrAllocCopy(NewsHost, HTNewsHost);
	    } else {
		if (s >= 0 && NewsHost && strcasecomp(NewsHost, cp)) {
		    NEWS_NETCLOSE(s);
		    s = -1;
		}
		StrAllocCopy(NewsHost, cp);
	    }
	    FREE(cp);
	    sprintf(command, "%s://%.245s/",
	    		     (post_wanted ?
			       "newspost" :
			    (reply_wanted ?
			       "newreply" :
			    (spost_wanted ?
			      "snewspost" : "snewsreply"))), NewsHost);
	    StrAllocCopy(NewsHREF, command);

	    /*
	    **  If the SSL daemon is being used as a proxy,
	    **  reset p1 to the start of the proxied URL
	    **  rather than to the start of the newsgroup(s). - FM
	    */
	    if (spost_wanted && strncasecomp(arg, "snewspost:", 10))
	        p1 = strstr(arg, "snewspost:");
	    if (sreply_wanted && strncasecomp(arg, "snewsreply:", 11))
	        p1 = strstr(arg, "snewsreply:");

	/* p1 = HTParse(arg, "", PARSE_PATH | PARSE_PUNCTUATION); */
	/*
	**  Don't use HTParse because news: access doesn't follow traditional
	**  rules. For instance, if the article reference contains a '#',
	**  the rest of it is lost -- JFG 10/7/92, from a bug report
	*/
 	} else if (!strncasecomp (arg, "nntp:", 5)) {
	    if (((*(arg + 5) == '\0') ||
	         (!strcmp((arg + 5), "/") ||
		  !strcmp((arg + 5), "//") ||
		  !strcmp((arg + 5), "///"))) ||
		((!strncmp((arg + 5), "//", 2)) &&
	         (!(cp = strchr((arg + 7), '/')) || *(cp + 1) == '\0'))) {
		p1 = "*";
		group_wanted = FALSE;
		list_wanted = TRUE;
	    } else if (*(arg + 5) != '/') {
	        p1 = (arg + 5);
	    } else if (*(arg + 5) == '/' && *(arg + 6) != '/') {
	        p1 = (arg + 6);
	    } else {
		p1 = (cp + 1);
	    }
	    if (!(cp = HTParse(arg, "", PARSE_HOST)) || *cp == '\0') {
		if (s >= 0 && NewsHost && strcasecomp(NewsHost, HTNewsHost)) {
		    NEWS_NETCLOSE(s);
		    s = -1;
		}
		StrAllocCopy(NewsHost, HTNewsHost);
	    } else {
		if (s >= 0 && NewsHost && strcasecomp(NewsHost, cp)) {
		    NEWS_NETCLOSE(s);
		    s = -1;
		}
		StrAllocCopy(NewsHost, cp);
	    }
	    FREE(cp);
	    sprintf(command, "nntp://%.251s/", NewsHost);
	    StrAllocCopy(NewsHREF, command);
	}
	else if (!strncasecomp(arg, "snews:", 6)) {
	    HTAlert("This client does not contain support for SNEWS URLs.");
	    return HT_NOT_LOADED;
	}
 	else if (!strncasecomp (arg, "news:/", 6)) {
	    if (((*(arg + 6) == '\0') ||
	    	 !strcmp((arg + 6), "/") ||
		 !strcmp((arg + 6), "//")) ||
	        ((*(arg + 6) == '/') &&
		 (!(cp = strchr((arg + 7), '/')) || *(cp + 1) == '\0'))) {
		p1 = "*";
		group_wanted = FALSE;
		list_wanted = TRUE;
	    } else if (*(arg + 6) != '/') {
	        p1 = (arg + 6);
	    } else {
		p1 = (cp + 1);
	    }
	    if (!(cp = HTParse(arg, "", PARSE_HOST)) || *cp == '\0') {
		if (s >= 0 && NewsHost && strcasecomp(NewsHost, HTNewsHost)) {
		    NEWS_NETCLOSE(s);
		    s = -1;
		}
		StrAllocCopy(NewsHost, HTNewsHost);
	    } else {
		if (s >= 0 && NewsHost && strcasecomp(NewsHost, cp)) {
		    NEWS_NETCLOSE(s);
		    s = -1;
		}
		StrAllocCopy(NewsHost, cp);
	    }
	    FREE(cp);
	    sprintf(command, "news://%.251s/", NewsHost);
	    StrAllocCopy(NewsHREF, command);
	} else {
	    p1 = (arg + 5);  /* Skip "news:" prefix */
	    if (*p1 == '\0') {
		p1 = "*";
		group_wanted = FALSE;
		list_wanted = TRUE;
	    }
	    if (s >= 0 && NewsHost && strcasecomp(NewsHost, HTNewsHost)) {
		NEWS_NETCLOSE(s);
		s = -1;
	    }
	    StrAllocCopy(NewsHost, HTNewsHost);
	    StrAllocCopy(NewsHREF, "news:");
	}

	/*
	**  Set up any proxy for snews URLs that returns NNTP
	**  responses for Lynx to convert to HTML, instead of
	**  doing the conversion itself, and for handling posts
	**  or followups.  - TZ & FM
	*/
 	if (!strncasecomp(p1, "snews:", 6) ||
	    !strncasecomp(p1, "snewpost:", 10) ||
	    !strncasecomp(p1, "snewsreply:", 11)) {
	    if ((cp = HTParse(p1, "", PARSE_HOST)) != NULL && *cp != '\0')
		sprintf(command, "snews://%.250s", cp);
	    else
		sprintf(command, "snews://%.250s", NewsHost);
	    command[258] = '\0';
	    FREE(cp);
	    sprintf(proxycmd, "GET %.251s%c%c%c%c", command, CR, LF, CR, LF);
	    if (TRACE)
		fprintf(stderr,
	      		"HTNews: Proxy command is '%.*s'\n",
			(strlen(proxycmd) - 4), proxycmd);
	    strcat(command, "/");
	    StrAllocCopy(NewsHREF, command);
	    if (spost_wanted || sreply_wanted) {
	        /*
		**  Reset p1 so that it points to the newsgroup(s).
		*/
		if ((p1 = strrchr(arg, '/')) != NULL) {
		    p1++;
		} else {
		    p1 = (strrchr(arg, ':') + 1);
		}
	    } else {
	        /*
		**  Reset p1 so that it points to the newgroup
		**  (or a wildcard), or the article.
		*/
		if (!(cp = strrchr((p1 + 6), '/')) || *(cp + 1) == '\0') {
		    p1 = "*";
		    group_wanted = FALSE;
		    list_wanted = TRUE;
		} else {
		    p1 = (cp + 1);
		}
	    }
	}

	/*
	**  Set up command for a post, listing, or article request. - FM
	*/
	if (post_wanted || reply_wanted || spost_wanted || sreply_wanted) {
	    strcpy(command, "POST");
	} else if (list_wanted) {
	    strcpy(command, "LIST NEWSGROUPS");
	} else if (group_wanted) {
	    char * slash = strchr(p1, '/');
	    strcpy(command, "GROUP ");
	    first = 0;
	    last = 0;
	    if (slash) {
		*slash = '\0';
		strcpy(groupName, p1);
		*slash = '/';
		(void)sscanf(slash+1, "%d-%d", &first, &last);
		if ((first > 0) && (isdigit(*(slash+1))) &&
		    (strchr(slash+1, '-') == NULL || first == last)) {
		    /*
		    **  We got a number greater than 0, which will be
		    **  loaded as first, and either no range or the
		    **  range computes to zero, so make last negative,
		    **  as a flag to select the group and then fetch
		    **  an article by number (first) instead of by
		    **  messageID. - FM
		    */
		    last = -1;
		}
	    } else {
		strcpy(groupName, p1);
	    }
	    strcat(command, groupName);
	} else {
	    strcpy(command, "ARTICLE ");
	    if (strchr(p1, '<') == 0)
	        strcat(command,"<");
	    strcat(command, p1);
	    if (strchr(p1, '>') == 0)
	        strcat(command,">");
	}

        {
	    char * p = command + strlen(command);
	    /*
	    **  Teminate command with CRLF, as in RFC 977.
	    */
	    *p++ = CR;		/* Macros to be correct on Mac */
	    *p++ = LF;
	    *p++ = 0;
	}
	StrAllocCopy(ListArg, p1);
    } /* scope of p1 */
    
    if (!*arg) {
        FREE(NewsHREF);
	FREE(ListArg);
        return NO;			/* Ignore if no name */
    }

    /*
    **  Make a hypertext object with an anchor list.
    */
    if (!(post_wanted || reply_wanted || spost_wanted || sreply_wanted)) {
        node_anchor = anAnchor;
	target = HTML_new(anAnchor, format_out, stream);
	targetClass = *target->isa;	/* Copy routine entry points */
    }

    /*
    **  Now, let's get a stream setup up from the NewsHost.
    */       
    for (retries = 0; retries < 2; retries++) {
        if (s < 0) {
	    /* CONNECTING to news host */
            char url[260];
	    if (!strcmp(NewsHREF, "news:"))
                sprintf (url, "lose://%.251s/", NewsHost);
	    else
                sprintf (url, "%.259s", NewsHREF);
            if (TRACE)
                fprintf (stderr, "News: doing HTDoConnect on '%s'\n", url);

            _HTProgress("Connecting to NewsHost ...");

	    status = HTDoConnect (url, "NNTP", NEWS_PORT, &s);
            if (status == HT_INTERRUPTED) {
                /*
		**  Interrupt cleanly.
		*/
		if (TRACE)
		    fprintf(stderr,
		     "HTNews: Interrupted on connect; recovering cleanly.\n");
		_HTProgress("Connection interrupted.");
		if (!(post_wanted || reply_wanted ||
		      spost_wanted || sreply_wanted))
		    (*targetClass._abort)(target, NULL);
		FREE(NewsHost);
		FREE(NewsHREF);
		FREE(ListArg);
		if (postfile) {
#ifdef VMS
		    while (remove(postfile) == 0)
			; /* loop through all versions */
#else
		    remove(postfile);
#endif /* VMS */
		    FREE(postfile);
		}
                return HT_NOT_LOADED;
            }
	    if (status < 0) {
		char message[256];
	        NEWS_NETCLOSE(s);
		s = -1;
		if (TRACE)
		    fprintf(stderr,
		    	    "HTNews: Unable to connect to news host.\n");
		if (retries < 1)
		    continue;
		sprintf(message, "Could not access %s.", NewsHost);
		FREE(NewsHost);
		FREE(NewsHREF);
		FREE(ListArg);
		if (postfile) {
#ifdef VMS
		    while (remove(postfile) == 0)
			; /* loop through all versions */
#else
		    remove(postfile);
#endif /* VMS */
		    FREE(postfile);
		}
		return HTLoadError(stream, 500, message);
	    } else {
		if (TRACE)
		    fprintf(stderr, "HTNews: Connected to news host %s.\n",
				    NewsHost);
		HTInitInput(s);		/* set up buffering */
		if (proxycmd[0]) {
		    status = NEWS_NETWRITE(s, proxycmd, strlen(proxycmd));
		    if (TRACE)
		        fprintf(stderr,
			     "HTNews: Proxy command returned status '%d'.\n",
				status);
		}
		if (((status = response(NULL)) / 100) != 2) {
			char message[BIG];
			NEWS_NETCLOSE(s);
			s = -1;
			if (status == HT_INTERRUPTED) {
			    _HTProgress("Connection interrupted.");
			    if (!(post_wanted || reply_wanted ||
				  spost_wanted || sreply_wanted))
			        (*targetClass._abort)(target, NULL);
			    FREE(NewsHost);
			    FREE(NewsHREF);
			    FREE(ListArg);
			    if (postfile) {
#ifdef VMS
			        while (remove(postfile) == 0)
				    ; /* loop through all versions */
#else
			        remove(postfile);
#endif /* VMS */
			        FREE(postfile);
			    }
			    return(HT_NOT_LOADED);
			}
			if (retries < 1)
			    continue;
			sprintf(message, 
		  "Can't read news info. News host %.20s responded: %.200s",
		  		NewsHost, response_text);
		        return HTLoadError(stream, 500, message);
		}
		if (status == 200) {
		    HTCanPost = TRUE;
		} else {
		    HTCanPost = FALSE;
		    if (post_wanted || reply_wanted ||
		        spost_wanted || sreply_wanted) {
			HTAlert("Cannot POST to this host.");
			FREE(NewsHREF);
			FREE(ListArg);
			if (postfile) {
#ifdef VMS
			    while (remove(postfile) == 0)
				; /* loop through all versions */
#else
			    remove(postfile);
#endif /* VMS */
			    FREE(postfile);
			}
			return(HT_NOT_LOADED);
		    }
		}
	    }
	} /* If needed opening */
	
	if (post_wanted || reply_wanted ||
	     spost_wanted || sreply_wanted) {
	    if (!HTCanPost) {
		HTAlert("Cannot POST to this host.");
		FREE(NewsHREF);
		FREE(ListArg);
		if (postfile) {
#ifdef VMS
		    while (remove(postfile) == 0)
			; /* loop through all versions */
#else
		    remove(postfile);
#endif /* VMS */
		    FREE(postfile);
		}
		return(HT_NOT_LOADED);
	    }
	    if (postfile == NULL) {
	        extern char *LYNewsPost PARAMS((char *newsgroups,
						BOOLEAN followup));
		postfile = LYNewsPost(ListArg, (reply_wanted || sreply_wanted));
	    }
	    if (postfile == NULL) {
		HTProgress("Cancelled!");
		FREE(NewsHREF);
		FREE(ListArg);
		return(HT_NOT_LOADED);
	    }
        } else {
	    /*
	    **  Ensure reader mode, but don't bother checking the
	    **  status for anything but HT_INERRUPTED, because if
	    **  if the reader mode command is not needed, the server
	    **  probably return a 500, which is irrelevant at this
	    **  point. - FM
	    */
	    char buffer[20];

	    sprintf(buffer, "mode reader%c%c", CR, LF);
	    if ((status = response(buffer)) == HT_INTERRUPTED) {
		_HTProgress("Connection interrupted.");
		break;
	    }
	}

Send_NNTP_command:
	if ((status = response(command)) == HT_INTERRUPTED) {
	    _HTProgress("Connection interrupted.");
	    break;
	}
	if (status < 0) {
	    if (retries < 1) {
	        continue;
	    } else {
	        break;
	    }
	}
	if ((status/100) != 2 &&
	    status != 340) {
	    if (retries)
	        HTAlert(response_text);
	    else
	        _HTProgress(response_text);
	    NEWS_NETCLOSE(s);
	    s = -1;
	    /*
	    **  Message might be a leftover "Timeout-disconnected",
	    **  so try again if retries is not exhausted.
	    */
	    continue;
	}
  
	/*
	**  Post or load a group, article, etc
	*/
	if (post_wanted || reply_wanted || spost_wanted || sreply_wanted) {
	    if (status != 340) {
		HTAlert("Cannot POST to this host.");
	    } else {
	        post_article(postfile);
	    }
	    status = HT_NOT_LOADED;
	} else if (list_wanted) {
	    _HTProgress("Reading list of available newsgroups.");
	    status = read_list(ListArg);
	} else if (group_wanted) {
	    if (last < 0) {
	        /*
		**  We got one article number rather than a range
		**  following the slash which followed the group
		**  name, or the range was zero, so now that we
		**  have selected that group, load ARTICLE and the
		**  the number (first) as the command and go back
		**  to send it and check the response. - FM
		*/
		sprintf(command, "ARTICLE %d%c%c", first, CR, LF);
		group_wanted = FALSE;
		retries = 2;
		goto Send_NNTP_command;
	    }
	    _HTProgress("Reading list of articles in newsgroup.");
	    status = read_group(groupName, first, last);
        } else {
	    _HTProgress("Reading news article.");
	    status = read_article();
	}
	if (status == HT_INTERRUPTED) {
	    _HTProgress("Connection interrupted.");
	    status = HT_LOADED;
	}
	if (!(post_wanted || reply_wanted ||
	      spost_wanted || sreply_wanted))
	    (*targetClass._free)(target);
	FREE(NewsHREF);
	FREE(ListArg);
	if (postfile) {
#ifdef VMS
	    while (remove(postfile) == 0)
		; /* loop through all versions */
#else
	    remove(postfile);
#endif /* VMS */
	    FREE(postfile);
	}
	return status;
    } /* Retry loop */
    
    /* HTAlert("Sorry, could not load requested news."); */
        
/*    NXRunAlertPanel(NULL, "Sorry, could not load `%s'.",
	    NULL,NULL,NULL, arg);No -- message earlier wil have covered it */

    if (!(post_wanted || reply_wanted ||
	  spost_wanted || sreply_wanted))
        (*targetClass._abort)(target, NULL);
    FREE(NewsHREF);
    FREE(ListArg);
    if (postfile) {
#ifdef VMS
	while (remove(postfile) == 0)
	    ; /* loop through all versions */
#else
	remove(postfile);
#endif /* VMS */
	FREE(postfile);
    }
    return HT_NOT_LOADED;
}

#ifdef GLOBALDEF_IS_MACRO
#define _HTNEWS_C_1_INIT { "news", HTLoadNews, NULL }
GLOBALDEF (HTProtocol,HTNews,_HTNEWS_C_1_INIT);
#define _HTNEWS_C_2_INIT { "nntp", HTLoadNews, NULL }
GLOBALDEF (HTProtocol,HTNNTP,_HTNEWS_C_2_INIT);
#define _HTNEWS_C_3_INIT { "newspost", HTLoadNews, NULL }
GLOBALDEF (HTProtocol,HTNewsPost,_HTNEWS_C_3_INIT);
#define _HTNEWS_C_4_INIT { "newsreply", HTLoadNews, NULL }
GLOBALDEF (HTProtocol,HTNewsReply,_HTNEWS_C_4_INIT);
#define _HTNEWS_C_5_INIT { "snews", HTLoadNews, NULL }
GLOBALDEF (HTProtocol,HTSNews,_HTNEWS_C_5_INIT);
#define _HTNEWS_C_6_INIT { "snewspost", HTLoadNews, NULL }
GLOBALDEF (HTProtocol,HTSNewsPost,_HTNEWS_C_6_INIT);
#define _HTNEWS_C_7_INIT { "snewsreply", HTLoadNews, NULL }
GLOBALDEF (HTProtocol,HTSNewsReply,_HTNEWS_C_7_INIT);
#else
GLOBALDEF PUBLIC HTProtocol HTNews = { "news", HTLoadNews, NULL };
GLOBALDEF PUBLIC HTProtocol HTNNTP = { "nntp", HTLoadNews, NULL };
GLOBALDEF PUBLIC HTProtocol HTNewsPost = { "newspost", HTLoadNews, NULL };
GLOBALDEF PUBLIC HTProtocol HTNewsReply = { "newsreply", HTLoadNews, NULL };
GLOBALDEF PUBLIC HTProtocol HTSNews = { "snews", HTLoadNews, NULL };
GLOBALDEF PUBLIC HTProtocol HTSNewsPost = { "snewspost", HTLoadNews, NULL };
GLOBALDEF PUBLIC HTProtocol HTSNewsReply = { "snewsreply", HTLoadNews, NULL };
#endif /* GLOBALDEF_IS_MACRO */
