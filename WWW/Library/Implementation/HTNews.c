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
#endif
#ifndef SERVER_FILE
#define SERVER_FILE "/usr/local/lib/rn/server"
#endif

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

/*
**  Module-wide variables.
*/
PUBLIC  char * HTNewsHost=NULL;
PRIVATE char * NewsHost=NULL;
PRIVATE char * NewsHREF=NULL;
PRIVATE int s;					/* Socket for NewsHost */
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

/*   Get name of Host
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
#endif
	if (status < 0){
	    if (TRACE) fprintf(stderr,
	        "HTNews: Unable to send command. Disconnecting.\n");
	    NEWS_NETCLOSE(s);
	    s = -1;
	    return status;
	} /* if bad status */
    } /* if command to be sent */
    
    for(;;) {  
	if (((*p++ = NEXT_CHAR) == LF) ||
	    (p == &response_text[LINE_LENGTH])) {
	    *p++ = '\0';			/* Terminate the string */
	    if (TRACE) fprintf(stderr, "NNTP Response: %s\n", response_text);
	    sscanf(response_text, "%d", &result);
	    return result;	    
	} /* if end of line */
	
	if ((ch = *(p-1)) == (char)EOF) {
	    if (TRACE) fprintf(stderr,
	    	"HTNews: EOF on read, closing socket %d\n", s);
	    NEWS_NETCLOSE(s);	/* End of file, close socket */
	    return s = -1;	/* End of file on response */
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

    /* Default to the first word */
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
   
    for (i=0; i < HTML_OL_ATTRIBUTES; i++)
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
    return;
}

/*	Read in an Article					read_article
**	------------------
**
**
**	Note the termination condition of a single dot on a line by itself.
**	RFC 977 specifies that the line "folding" of RFC850 is not used, so we
**	do not handle it here.
**
** On entry,
**	s	Global socket number is OK
**	HT	Global hypertext object is ready for appending text
*/       
PRIVATE void read_article NOARGS
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
    **  The header fields are either ignored, or formatted and put into the
    **  Text.
    */
    if (!diagnostic) {
	while (!done) {
	    char ch = *p++ = NEXT_CHAR;
	    if (ch == (char)EOF) {
		abort_socket();	/* End of file, close socket */
	    	return;		/* End of file on response */
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
		    StrAllocCopy(organization, HTStrip(strchr(full_line,':')+1));
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
	/* put in the owner as a link rel. */
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

	if (newsgroups && !strncmp(NewsHREF, "news:", 5)) {
	    /* make posting possible */
	    StrAllocCopy(href,"newsreply:");
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

    /*
    **  Read in the BODY of the Article.
    */
    START(HTML_PRE);
    PUTC('\n');

    p = line;
    while (!done) {
	char ch = *p++ = NEXT_CHAR;
	if (ch == (char)EOF) {
	    abort_socket();	/* End of file, close socket */
	    return;		/* End of file on response */
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

/*	All lines are passed as unmodified source. - FM
*/
	            PUTS(line);

	        } else {

/*	Normal lines are scanned for buried references to other articles.
**	Unfortunately, it could pick up mail addresses as well!  It also
**	can corrupt uuencoded messages!  So we don't do this when fetching
**	articles as WWW_SOURCE or when downloading (diagnostic is TRUE) or
**	if the client has set scan_for_buried_news_references to FALSE.
**	Otherwise, we convert all "<...@...>" strings preceded by "rticle "
**	to "news:...@..." links, and any strings that look like URLs to
**	links. - FM
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
    
    END(HTML_PRE);
    PUTC('\n');
}

/*	Read in a List of Newsgroups
**	----------------------------
**
**	Note the termination condition of a single dot on a line by itself.
**	RFC 977 specifies that the line "folding" of RFC850 is not used, so we
**	do not handle it here.
*/        
PRIVATE void read_list ARGS1(char *, arg)
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
     * Support head or tail matches for groups to list. - FM
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
    **  The header fields are either ignored, or formatted and put into the
    **  Text.
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
	    abort_socket();	/* End of file, close socket */
	    FREE(pattern);
	    return;		/* End of file on response */
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
    return;
}

/*	Read in a Newsgroup
**	-------------------
**	Unfortunately, we have to ask for each article one by one if we
**	want more than one field.
**
*/
PRIVATE void read_group ARGS3(
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
    if (last ==0 ) {
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
    	sprintf(buffer, "%s/%d-%d", groupName, before, first_required-1);
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
		    abort_socket();	/* End of file, close socket */
		    return;		/* End of file on response */
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
	
	/*	Normal lines are scanned for references to articles.
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
/* With this code we try to keep the server running flat out by queuing just
** one extra command ahead of time. We assume (1) that the server won't abort
** if it gets input during output, and (2) that TCP buffering is enough for the
** two commands. Both these assumptions seem very reasonable. However, we HAVE
** had a hangup with a loaded server.
*/
	    if (art == first_required) {
		if (art == last_required) {		/* Only one */
			sprintf(buffer,
				"HEAD %d%c%c", art, CR, LF);
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
	    
#else	/* NOT OVERLAP: */
	    sprintf(buffer, "HEAD %d%c%c", art, CR, LF);
	    status = response(buffer);
#endif	/* OVERLAP */

	    if (status == 221) {	/* Head follows - parse it:*/
    
		p = line;				/* Write pointer */
		done = NO;
		while( !done ) {
		    char ch = *p++ = NEXT_CHAR;
		    if (ch == (char)EOF) {
			abort_socket();	/* End of file, close socket */
			return;		/* End of file on response */
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
		
    
/*	 indicate progress!   @@@@@@
*/
    
	    } /* If good response */
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
    if (!strncmp(NewsHREF, "news:", 5)) {
	char *href = NULL;
	START(HTML_HR);
	PUTC('\n');
	StrAllocCopy(href,"newspost:");
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
}

/*		Load by name					HTLoadNews
**		============
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
    BOOL group_wanted;			/* Flag: group was asked for, not article */
    BOOL list_wanted;			/* Flag: group was asked for, not article */
    int first, last;			/* First and last articles asked for */
    char *cp;
    char *ListArg = NULL;

    diagnostic = (format_out == WWW_SOURCE ||	/* set global flag */
    		  format_out == HTAtom_for("www/download") ||
		  format_out == HTAtom_for("www/dump"));
    
    if (TRACE) fprintf(stderr, "HTNews: Looking for %s\n", arg);
    
    if (!initialized)
	initialized = initialize();
    if (!initialized)
	return -1;	/* FAIL */
    
    FREE(NewsHREF);

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
	group_wanted = (strchr(arg, '@') == NULL) && (strchr(arg, '*') == NULL);
	list_wanted  = (strchr(arg, '@') == NULL) && (strchr(arg, '*') != NULL);

	/* p1 = HTParse(arg, "", PARSE_PATH | PARSE_PUNCTUATION); */
	/*
	 *  Don't use HTParse because news: access doesn't follow traditional
	 *  rules. For instance, if the article reference contains a '#',
	 *  the rest of it is lost -- JFG 10/7/92, from a bug report
	 */
 	if (!strncasecomp (arg, "nntp://", 7)) {
	  if (!(cp = strchr(arg+7, '/')) || *(cp+1) == '\0') {
	      p1 = "*";
	      group_wanted = FALSE;
	      list_wanted = TRUE;
	  } else {
	      p1 = (cp+1);
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
	  sprintf(command, "nntp://%.250s/", NewsHost);
	  StrAllocCopy(NewsHREF, command);
	}
	else if (!strncasecomp(arg, "snews:", 6)) {
	  HTAlert("This client does not contain support for SNEWS URLs.");
	  return HT_NO_DATA;
	}
 	else if (!strncasecomp (arg, "news://", 7)) {
	  if (!(cp = strchr(arg+7, '/')) || *(cp+1) == '\0') {
	      p1 = "*";
	      group_wanted = FALSE;
	      list_wanted = TRUE;
	  } else {
	      p1 = (cp+1);
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
	  sprintf(command, "news://%.250s/", NewsHost);
	  StrAllocCopy(NewsHREF, command);
	} else {
	  p1 = arg + 5;  /* Skip "news:" prefix */
	  if (s >= 0 && NewsHost && strcasecomp(NewsHost, HTNewsHost)) {
	      NEWS_NETCLOSE(s);
	      s = -1;
	  }
	  StrAllocCopy(NewsHost, HTNewsHost);
	  StrAllocCopy(NewsHREF, "news:");
	}

	/*
	 *  Set up any proxy for snews URLs that returns NNTP
	 *  responses for Lynx to convert to HTML, instead of
	 *  doing the conversion itself. - TZ & FM
	 */
	proxycmd[0] = '\0';
 	if (!strncasecomp (p1, "snews://", 8)) {
	  if ((cp = strchr((p1+8), '/')) != NULL)
	    *cp = '\0';
	  sprintf(command, "%.258s/", p1);
	  StrAllocCopy(NewsHREF, command);
	  sprintf(proxycmd, "GET %.250s%c%c%c%c", p1, CR, LF, CR, LF);
	  if ((cp == NULL) || *(cp+1) == '\0') {
	      p1 = "*";
	      group_wanted = FALSE;
	      list_wanted = TRUE;
	  } else {
	      p1 = (cp+1);
	  }
	  if (cp != NULL)
	      *cp = '/';
	  if (TRACE)
	      fprintf(stderr,
	      	      "HTNews: Proxy command is '%.*s'\n",
		      (strlen(proxycmd) - 4), proxycmd);
	}

	/*
	 *  Set up command for a listing or article request. - FM
	 */
	if (list_wanted) {
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
		     *  We got a number greater than 0, which will be
		     *  loaded as first, and either no range or the
		     *  range computes to zero, so make last negative,
		     *  as a flag to select the group and then fetch
		     *  an article by number (first) instead of by
		     *  messageID. - FM
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
	     *  Teminate command with CRLF, as in RFC 977.
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
     *  Make a hypertext object with an anchor list.
     */
    node_anchor = anAnchor;
    target = HTML_new(anAnchor, format_out, stream);
    targetClass = *target->isa;	/* Copy routine entry points */

    /*
     *  Now, let's get a stream setup up from the NewsHost.
     */       
    for (retries = 0; retries < 2; retries++) {
        if (s < 0) {
	    /* CONNECTING to news host */
            char url[1024];
	    if (!strncmp(arg, "news:", 5))
                sprintf (url, "lose://%s/", NewsHost);
	    else
                strcpy (url, arg);
            if (TRACE)
                fprintf (stderr, "News: doing HTDoConnect on '%s'\n", url);

            _HTProgress("Connecting to NewsHost ...");

	    status = HTDoConnect (url, "NNTP", NEWS_PORT, &s);
            if (status == HT_INTERRUPTED) {
                /*
		 *  Interrupt cleanly.
		 */
		if (TRACE)
                    fprintf(stderr,
                         "News: Interrupted on connect; recovering cleanly.\n");
                _HTProgress("Connection interrupted.");

		(*targetClass._abort)(target, NULL);
  
		FREE(NewsHost);
		FREE(NewsHREF);
		FREE(ListArg);
                return HT_INTERRUPTED;
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
		if ((response(NULL) / 100) != 2) {
			char message[BIG];
			NEWS_NETCLOSE(s);
			s = -1;
			if (retries < 1)
			    continue;
			sprintf(message, 
		  "Can't read news info. News host %.20s responded: %.200s",
		  		NewsHost, response_text);
			FREE(NewsHost);
			FREE(NewsHREF);
			FREE(ListArg);
		        return HTLoadError(stream, 500, message);
		}
	    }
	} /* If needed opening */
	
        /*
	 *  Ensure reader mode, but don't bother checking the
	 *  status for anything but HT_INERRUPTED, because if
	 *  if the reader mode command is not needed, the server
	 *  probably return a 500, which is irrelevant at this
	 *  point. - FM
	 */
	{
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
	if ((status/100) != 2) {
	    if (retries)
	        HTAlert(response_text);
	    else
	        _HTProgress(response_text);
	    NEWS_NETCLOSE(s);
	    s = -1;
	    /*
	     *  Message might be a leftover "Timeout-disconnected",
	     *  so try again if retries is not exhausted.
	     */
	    continue;
	}
  
	/*
	 *  Load a group, article, etc
	 */
	if (list_wanted) {
	    _HTProgress("Reading list of available newsgroups.");
	    read_list(ListArg);
	} else if (group_wanted) {
	    if (last < 0) {
	        /*
		 *  We got one article number rather than a range
		 *  following the slash which followed the group
		 *  name, or the range was zero, so now that we
		 *  have selected that group, load ARTICLE and the
		 *  the number (first) as the command and go back
		 *  to send it and check the response. - FM
		 */
		sprintf(command, "ARTICLE %d%c%c", first, CR, LF);
		group_wanted = FALSE;
		retries = 2;
		goto Send_NNTP_command;
	    }
	    _HTProgress("Reading list of articles in newsgroup.");
	    read_group(groupName, first, last);
        } else {
	    _HTProgress("Reading news article.");
	    read_article();
	}

	(*targetClass._free)(target);
	FREE(NewsHREF);
	FREE(ListArg);
	return HT_LOADED;
	
    } /* Retry loop */
    
    
    /* HTAlert("Sorry, could not load requested news."); */
        
/*    NXRunAlertPanel(NULL, "Sorry, could not load `%s'.",
	    NULL,NULL,NULL, arg);No -- message earlier wil have covered it */

    (*targetClass._abort)(target, NULL);
    FREE(NewsHREF);
    FREE(ListArg);
    return HT_NO_DATA;
}

#ifdef GLOBALDEF_IS_MACRO
#define _HTNEWS_C_1_INIT { "news", HTLoadNews, NULL }
GLOBALDEF (HTProtocol,HTNews,_HTNEWS_C_1_INIT);
#define _HTNEWS_C_2_INIT { "nntp", HTLoadNews, NULL }
GLOBALDEF (HTProtocol,HTNNTP,_HTNEWS_C_2_INIT);
#define _HTNEWS_C_3_INIT { "snews", HTLoadNews, NULL }
GLOBALDEF (HTProtocol,HTSNews,_HTNEWS_C_1_INIT);
#else
GLOBALDEF PUBLIC HTProtocol HTNews = { "news", HTLoadNews, NULL };
GLOBALDEF PUBLIC HTProtocol HTNNTP = { "nntp", HTLoadNews, NULL };
GLOBALDEF PUBLIC HTProtocol HTSNews = { "snews", HTLoadNews, NULL };
#endif /* GLOBALDEF_IS_MACRO */
