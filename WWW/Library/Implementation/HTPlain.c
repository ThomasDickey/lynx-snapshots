/*		Plain text object		HTWrite.c
**		=================
**
**	This version of the stream object just writes to a socket.
**	The socket is assumed open and left open.
**
**	Bugs:
**		strings written must be less than buffer size.
*/
#include "HTUtils.h"

#include "HTPlain.h"

#define BUFFER_SIZE 4096;	/* Tradeoff */

#include "HText.h"
#include "HTStyle.h"
#include "HTMLDTD.h"
#include "HTCJK.h"

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

extern HTStyleSheet * styleSheet;

extern int current_char_set;
extern char * LYchar_set_names[];
extern CONST char **LYCharSets[];
extern CONST char * HTMLGetEntityName PARAMS((int i));
extern BOOL HTPassEightBitRaw;
extern BOOL HTPassHighCtrlRaw;
extern HTCJKlang HTCJK;

PUBLIC int HTPlain_lastraw = -1;

/*		HTML Object
**		-----------
*/
struct _HTStream {
	CONST HTStreamClass *	isa;

	HText * 		text;
};

/*	Write the buffer out to the socket
**	----------------------------------
*/


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/
PRIVATE void HTPlain_put_character ARGS2(HTStream *, me, char, c)
{
#ifdef REMOVE_CR_ONLY
    /* throw away \r's */
    if (c != '\r') {
       HText_appendCharacter(me->text, c);
    }
#else
    /*
    **  See HTPlain_write() for explanations of the following code
    **  (we've been called via HTPlain_put_string() to do for each
    **  character of a terminated string what HTPlain_write() does
    **  via a while loop for each character in a stream of given
    **  length). - FM
    */
    if ((HTPlain_lastraw == '\r') && c == '\n') {
	HTPlain_lastraw = -1;
	return;
    }
    HTPlain_lastraw = c;
    if (c == '\r') {
	HText_appendCharacter(me->text, '\n');
    } else if (HTCJK != NOCJK) {
	HText_appendCharacter(me->text, c);
    } else if ((unsigned char)c >= 127 && (unsigned char)c < 161 &&
    	       HTPassHighCtrlRaw) {
	HText_appendCharacter(me->text, c);
    } else if ((unsigned char)c == 160) {
	HText_appendCharacter(me->text, ' ');
    } else if ((unsigned char)c == 173) {
        return;
    } else if (((unsigned char)c >= 32 && (unsigned char)c < 127) ||
	       c == '\n' || c == '\t') {
	HText_appendCharacter(me->text, c);
    } else if ((unsigned char)c > 160) {
	if (!HTPassEightBitRaw &&
	    strncmp(LYchar_set_names[current_char_set], "ISO Latin 1", 11)) {
	    int len, high, low, i, diff;
	    CONST char * name;
	    int value = (int)((unsigned char)c - 160);
	    name = HTMLGetEntityName(value);
	    len =  strlen(name);
	    for(low=0, high = HTML_dtd.number_of_entities;
		high > low;
		diff < 0 ? (low = i+1) : (high = i)) {
		/* Binary search */
		i = (low + (high-low)/2);
		diff = strncmp(HTML_dtd.entity_names[i], name, len);
		if (diff==0) {
		    HText_appendText(me->text,
		    		     LYCharSets[current_char_set][i]);
		    break;
		}
	    }
	    if (diff) {
		HText_appendCharacter(me->text, c);
	    }
	} else {
	    HText_appendCharacter(me->text, c);
	}
    }
#endif /* REMOVE_CR_ONLY */
}


/*	String handling
**	---------------
**
*/
PRIVATE void HTPlain_put_string ARGS2(HTStream *, me, CONST char*, s)
{
#ifdef REMOVE_CR_ONLY
    HText_appendText(me->text, s);
#else
    CONST char * p;

    if (s == NULL)
	return;
    for (p = s; *p; p++) {
        HTPlain_put_character(me, *p);
    }
#endif /* REMOVE_CR_ONLY */
}


/*
**	Entry function for displayed text/plain and WWW_SOURCE strings. - FM
**	---------------------------------------------------------------
*/
PRIVATE void HTPlain_write ARGS3(HTStream *, me, CONST char*, s, int, l)
{
    CONST char * p;
    CONST char * e = s+l;

    for (p = s; p < e; p++) {
#ifdef REMOVE_CR_ONLY
	/* 
	**  Append the whole string, but remove any \r's. - FM
	*/
	if (*p != '\r') {
	    HText_appendCharacter(me->text, *p);
	}
#else
	/*
	**  Try to handle lone LFs, CRLFs and lone CRs
	**  as newline, and to deal with control, ASCII,
	**  and 8-bit characters based on best guesses
	**  of what's appropriate. - FM
	*/
	if ((HTPlain_lastraw == '\r') && *p == '\n') {
	    HTPlain_lastraw = -1;
	    continue;
	}
	HTPlain_lastraw = *p;
	if (*p == '\r') {
	    HText_appendCharacter(me->text, '\n');
	/*
	**  If CJK mode is on, we'll assume the document matches
	**  the user's selected character set, and if not, the
	**  user should toggle off raw/CJK mode to reload. - FM
	*/
	} else if (HTCJK != NOCJK) {
	    HText_appendCharacter(me->text, *p);
	/*
	**  If HTPassHighCtrlRaw is set (e.g., for KOI8-R) assume the
	**  document matches and pass 127-160 8-bit characters.  If it
	**  doesn't match, the user should toggle raw/CJK mode off. - FM
	*/
	} else if ((unsigned char)*p >= 127 && (unsigned char)*p < 161 &&
		    HTPassHighCtrlRaw) {
	    HText_appendCharacter(me->text, *p);
	/*
	**  If neither HTPassHighCtrlRaw nor CJK is set, play it safe
	**  and treat 160 (nbsp) as an ASCII space (32). - FM
	*/
	} else if ((unsigned char)*p == 160) {
	    HText_appendCharacter(me->text, ' ');
	/*
	**  If neither HTPassHighCtrlRaw nor CJK is set, play it safe
	**  and ignore 173 (shy). - FM
	*/
	} else if ((unsigned char)*p == 173) {
	    continue;
	/*
	**  If we get to here, pass the displayable ASCII characters. - FM
	*/
	} else if (((unsigned char)*p >= 32 && (unsigned char)*p < 127) ||
		   *p == '\n' || *p == '\t') {
	    HText_appendCharacter(me->text, *p);
	/*
	**  If we get to here and HTPassEightBitRaw or the
	**  selected character set is not "ISO Latin 1",
	**  use the translation tables for 161-255 8-bit
	**  characters (173 was handled above). - FM
	*/
	} else if ((unsigned char)*p > 160) {
	    if (!HTPassEightBitRaw &&
		strncmp(LYchar_set_names[current_char_set],
		   	"ISO Latin 1", 11)) {
		/*
		**  Attempt to translate. - FM
		*/
		int len, high, low, i, diff;
		CONST char * name;
		int value = (int)((unsigned char)*p - 160);
		name = HTMLGetEntityName(value);
		len =  strlen(name);
		for(low=0, high = HTML_dtd.number_of_entities;
		    high > low;
		    diff < 0 ? (low = i+1) : (high = i)) {
		    /* Binary search */
		    i = (low + (high-low)/2);
		    diff = strncmp(HTML_dtd.entity_names[i], name, len);
		    if (diff==0) {
			HText_appendText(me->text,
					 LYCharSets[current_char_set][i]);
			break;
		    }
		}
		if (diff) {
		    /*
		    **  Something went wrong in the translation, so
		    **  pass the raw character and hope it's OK. - FM
		    */
		    HText_appendCharacter(me->text, *p);
		}
	    } else {
	        /*
		**  Didn't attempt a translation. - FM
		*/
	        HText_appendCharacter(me->text, *p);
	    }
	}
#endif /* REMOVE_CR_ONLY */
    }
}


/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is
**	not, as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE void HTPlain_free ARGS1(HTStream *, me)
{
    FREE(me);
}


/*	End writing
*/
PRIVATE void HTPlain_abort ARGS2(HTStream *, me, HTError, e)
{
    HTPlain_free(me);
}


/*		Structured Object Class
**		-----------------------
*/
PUBLIC CONST HTStreamClass HTPlain =
{		
	"SocketWriter",
	HTPlain_free,
	HTPlain_abort,
	HTPlain_put_character, 	HTPlain_put_string, HTPlain_write,
}; 


/*		New object
**		----------
*/
PUBLIC HTStream* HTPlainPresent ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink)
{

    HTStream* me = (HTStream*)malloc(sizeof(*me));
    if (me == NULL)
        outofmem(__FILE__, "HTPlain_new");
    me->isa = &HTPlain;

    HTPlain_lastraw = -1;

    me->text = HText_new(anchor);
    HText_setStyle(me->text, HTStyleNamed(styleSheet, "Example"));
    HText_beginAppend(me->text);

    return (HTStream*) me;
}

