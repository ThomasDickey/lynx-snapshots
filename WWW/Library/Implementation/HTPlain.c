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
#ifdef EXP_CHARTRANS
#include "UCMap.h"
#include "UCDefs.h"
#include "UCAux.h"
#endif /* EXP_CHARTRANS */

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

extern HTStyleSheet * styleSheet;

extern int current_char_set;
extern char * LYchar_set_names[];
extern CONST char **LYCharSets[];
#ifdef EXP_CHARTRANS
extern int LYlowest_eightbit[];
#endif /* EXP_CHARTRANS */
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
#ifdef EXP_CHARTRANS
    LYUCcharset	* UCI;	/* pointer to node_anchor's UCInfo */
    int	in_char_set;		/* tells us what charset we are fed */
    int	htext_char_set;		/* what charset feed to HText */
    char                utf_count;
    long                utf_char;
    char	utf_buf[7];
    char *	utf_buf_p;
    UCTransParams T;
#endif /* EXP_CHARTRANS */
};

#ifdef EXP_CHARTRANS

PRIVATE char replace_buf [61];        /* buffer for replacement strings */

PRIVATE void HTPlain_getChartransInfo ARGS2(
	HTStream *,		me,
	HTParentAnchor *,	anchor)
{
    if (me->in_char_set < 0) {
	HTAnchor_copyUCInfoStage(anchor, UCT_STAGE_PARSER, UCT_STAGE_MIME,
				 	 UCT_SETBY_PARSER);
	me->in_char_set = HTAnchor_getUCLYhndl(anchor, UCT_STAGE_PARSER);
    }
    if (me->htext_char_set < 0) {
	int chndl = HTAnchor_getUCLYhndl(anchor, UCT_STAGE_HTEXT);
	if (chndl < 0) {
	    chndl = current_char_set;
	    HTAnchor_setUCInfoStage(anchor, chndl,
				    UCT_STAGE_HTEXT, UCT_SETBY_DEFAULT);
	}
	HTAnchor_setUCInfoStage(anchor, chndl,
				UCT_STAGE_HTEXT, UCT_SETBY_DEFAULT);
	me->htext_char_set = HTAnchor_getUCLYhndl(anchor, UCT_STAGE_HTEXT);
    }
    me->UCI = HTAnchor_getUCInfoStage(anchor, UCT_STAGE_PARSER);
}
#endif /* EXP_CHARTRANS */

/*	Write the buffer out to the socket
**	----------------------------------
*/

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

#ifdef EXP_CHARTRANS
        /* for forward reference to HTPlain_write - kw */
#ifdef _WINDOWS
PRIVATE void HTPlain_write (HTStream * me, CONST char* s, int l);
#else
PRIVATE void HTPlain_write PARAMS((
	HTStream *		me,
	CONST char *		s,
	int			l));
#endif /* _WINDOWS */
#endif /* EXP_CHARTRANS */

/*	Character handling
**	------------------
*/
PRIVATE void HTPlain_put_character ARGS2(
	HTStream *,		me,
	char,			c)
{
#ifdef REMOVE_CR_ONLY
    /*
    **  Throw away \r's.
    */
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
#ifdef EXP_CHARTRANS
      /* for now don't repeat everything here that has been done below - kw*/
    } else if ((unsigned char)c >= 127) {
	HTPlain_write(me, &c, 1);
#endif
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
	    int len, high, low, i, diff = 1;
	    CONST char * name;
	    int value = (int)((unsigned char)c - 160);

	    name = HTMLGetEntityName(value);
	    len =  strlen(name);
	    for (low = 0, high = HTML_dtd.number_of_entities;
		high > low;
		diff < 0 ? (low = i+1) : (high = i)) {
		/* Binary search */
		i = (low + (high-low)/2);
		diff = strncmp(HTML_dtd.entity_names[i], name, len);
		if (diff == 0) {
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
#ifdef EXP_CHARTRANS
    BOOL chk;
    long unsign_c, uck;
    char c_p;
#endif /* EXP_CHARTRANS */

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
	    continue;
	}
#ifdef EXP_CHARTRANS
	unsign_c = (unsigned char)(*p);
	c_p = *p;

	if (me->T.decode_utf8) {
	    /*
	    **  Combine UTF-8 into Unicode.
	    **  Incomplete characters silently ignored.
	    **  from Linux kernel's console.c
	    */
	    if ((unsigned char)(*p) > 0x7f) {
		if (me->utf_count > 0 && (*p & 0xc0) == 0x80) {
		    me->utf_char = (me->utf_char << 6) | (*p & 0x3f);
		    me->utf_count--;
		    *(me->utf_buf_p++) = *p;
		    if (me->utf_count == 0) {
			*(me->utf_buf_p) = '\0';
			unsign_c = me->utf_char;
			if (unsign_c<256) c_p = (char)unsign_c;
		    }
		    else continue;  /* iterate for more */
		} else {
		    me->utf_buf_p = me->utf_buf;
		    *(me->utf_buf_p++) = *p;
		    if ((*p & 0xe0) == 0xc0) {
			me->utf_count = 1;
			me->utf_char = (*p & 0x1f);
		    } else if ((*p & 0xf0) == 0xe0) {
			me->utf_count = 2;
			me->utf_char = (*p & 0x0f);
		    } else if ((*p & 0xf8) == 0xf0) {
			me->utf_count = 3;
			me->utf_char = (*p & 0x07);
		    } else if ((*p & 0xfc) == 0xf8) {
			me->utf_count = 4;
			me->utf_char = (*p & 0x03);
		    } else if ((*p & 0xfe) == 0xfc) {
			me->utf_count = 5;
			me->utf_char = (*p & 0x01);
		    } else { /* garbage */
			me->utf_count = 0;
			me->utf_buf_p = me->utf_buf;
			*(me->utf_buf_p) = '\0';
		    }
		    continue; /* iterate for more */
		}
	    } else {	/* got an ASCII char */
		me->utf_count = 0;
		me->utf_buf_p = me->utf_buf;
		*(me->utf_buf_p) = '\0';
	    }
	}
	
	if (me->T.trans_to_uni && (unsign_c >= 127 ||
				   (unsign_c < 32 && unsign_c != 0 &&
				    me->T.trans_C0_to_uni))) {
	    unsign_c = UCTransToUni(c_p, me->in_char_set);
	    if (unsign_c > 0) {
		if (unsign_c < 256) {
		    c_p = (char)unsign_c;
		}
	    }
	}
	/*
	**  At this point we have either unsign_c in Unicode
	**  (and c in latin1 if unsign_c is in the latin1 range),
	**  or unsign_c and c will have to be passed raw.
	*/

#else
#define unsign_c (unsigned char)*p	
#define c_p *p
#endif /* EXP_CHARTRANS */
	/*
	**  If CJK mode is on, we'll assume the document matches
	**  the user's selected character set, and if not, the
	**  user should toggle off raw/CJK mode to reload. - FM
	*/
	if (HTCJK != NOCJK) {
	    HText_appendCharacter(me->text, c_p);

#ifndef EXP_CHARTRANS
#define PASSHICTRL HTPassHighCtrlRaw
#define PASS8859SPECL HTPassHighCtrlRaw
#define PASSHI8BIT HTPassEightBitRaw
#else
#define PASSHICTRL (me->T.transp || \
		    unsign_c >= LYlowest_eightbit[me->in_char_set])
#define PASS8859SPECL me->T.pass_160_173_raw
#define PASSHI8BIT (HTPassEightBitRaw || \
		    (me->T.do_8bitraw && !me->T.trans_from_uni))
#endif /* EXP_CHARTRANS */

	/*
	**  If HTPassHighCtrlRaw is set (e.g., for KOI8-R) assume the
	**  document matches and pass 127-160 8-bit characters.  If it
	**  doesn't match, the user should toggle raw/CJK mode off. - FM
	*/
	} else if (unsign_c >= 127 && unsign_c < 161 &&
		   PASSHICTRL && PASS8859SPECL) {
	    HText_appendCharacter(me->text, *p);
	} else if (unsign_c == 173 && PASS8859SPECL) {
	    HText_appendCharacter(me->text, *p);
	/*
	**  If neither HTPassHighCtrlRaw nor CJK is set, play it safe
	**  and treat 160 (nbsp) as an ASCII space (32). - FM
	*/
	} else if (unsign_c == 160) {
	    HText_appendCharacter(me->text, ' ');
	/*
	**  If neither HTPassHighCtrlRaw nor CJK is set, play it safe
	**  and ignore 173 (shy). - FM
	*/
	} else if (unsign_c == 173) {
	    continue;
#ifdef EXP_CHARTRANS
	} else if (me->T.strip_raw_char_in &&
		   (unsigned char)*p >= 0xc0 &&
		   (unsigned char)*p < 255) {
	    /*
	    **  KOI special: strip high bit, gives
	    **  (somewhat) readable ASCII.
	    */
	    HText_appendCharacter(me->text, (char)(*p & 0x7f));
#endif /* EXP_CHARTRANS */
	/*
	**  If we get to here, pass the displayable ASCII characters. - FM
	*/
	} else if ((unsign_c >= 32 && unsign_c < 127) ||
#ifdef EXP_CHARTRANS
		   (PASSHI8BIT && c_p>=LYlowest_eightbit[me->htext_char_set])||
#endif
		   *p == '\n' || *p == '\t') {
	    HText_appendCharacter(me->text, c_p);

#ifdef EXP_CHARTRANS
	} else if (me->T.use_raw_char_in) {
	    HText_appendCharacter(me->text, *p);
/******************************************************************
 *   I. LATIN-1 OR UCS2  TO  DISPLAY CHARSET
 ******************************************************************/  
	} else if ((chk = (me->T.trans_from_uni && unsign_c >= 160)) &&
		   (uck = UCTransUniChar(unsign_c,
					 me->htext_char_set)) >= 32 &&
		   uck < 256) {
	    if (TRACE) {
		fprintf(stderr,
			"UCTransUniChar returned 0x%lx:'%c'.\n",
			uck, (char)uck);
	    }
	    HText_appendCharacter(me->text, (char)(uck & 0xff));
	} else if (chk && (uck == -4 ||
			    (me->T.repl_translated_C0 &&
			     uck > 0 && uck <32)) &&
		   /*
		   **  Not found; look for replacement string.
		   */
		   (uck = UCTransUniCharStr(replace_buf,60, unsign_c,
					    me->htext_char_set, 0) >= 0)) { 
	    /*
	    **  No further tests for valididy - assume that whoever
	    **  defined replacement strings knew what she was doing.
	    */
	    HText_appendText(me->text, replace_buf);
	/*
	**  If we get to here, and should have translated,
	**  translation has failed so far.  
	*/
	} else if (chk && unsign_c > 127 && me->T.output_utf8 &&
		   *me->utf_buf) {
	    HText_appendText(me->text, me->utf_buf);
	    me->utf_buf_p = me->utf_buf;
	    *(me->utf_buf_p) = '\0';
	} else if (me->T.trans_from_uni && unsign_c > 255) {
	    sprintf(replace_buf, "U%.2lx", unsign_c);
	    HText_appendText(me->text, replace_buf);
#endif /* EXP_CHARTRANS */

	/*
	**  If we get to here and HTPassEightBitRaw or the
	**  selected character set is not "ISO Latin 1",
	**  use the translation tables for 161-255 8-bit
	**  characters (173 was handled above). - FM
	*/
	} else if (unsign_c > 160) {
	    if (!HTPassEightBitRaw && unsign_c <= 255 &&
		strncmp(LYchar_set_names[current_char_set],
		   	"ISO Latin 1", 11)) {
		/*
		**  Attempt to translate. - FM
		*/
		int len, high, low, i, diff=1;
		CONST char * name;
		int value = (int)(unsign_c - 160);
		name = HTMLGetEntityName(value);
		len =  strlen(name);
		for(low = 0, high = HTML_dtd.number_of_entities;
		    high > low;
		    diff < 0 ? (low = i+1) : (high = i)) {
		    /* Binary search */
		    i = (low + (high-low)/2);
		    diff = strncmp(HTML_dtd.entity_names[i], name, len);
		    if (diff == 0) {
			HText_appendText(me->text,
					 LYCharSets[current_char_set][i]);
			break;
		    }
		}
		if (diff) {
		    /*
		    **  Something went wrong in the translation, so
		    **  either output as UTF8 or a hex representation or
		    **  pass the raw character and hope it's OK.
		    */
#ifdef EXP_CHARTRANS
		    if (!PASSHI8BIT)
			c_p = (char)unsign_c;
		    if (me->T.output_utf8 &&
			*me->utf_buf) {
			HText_appendText(me->text, me->utf_buf);
			me->utf_buf_p = me->utf_buf;
			*(me->utf_buf_p) = '\0';

		    } else if (me->T.trans_from_uni) {
			sprintf(replace_buf,"U%.2lx",unsign_c);
			HText_appendText(me->text, replace_buf);
		    } else
#endif /* EXP_CHARTRANS */
		    HText_appendCharacter(me->text, c_p);
		}
	    } else {
	        /*
		**  Didn't attempt a translation. - FM
		*/
#ifdef EXP_CHARTRANS
		    /*  either output as UTF8 or a hex representation or
		    **  pass the raw character and hope it's OK.
		    */
		if (unsign_c <= 255 && !PASSHI8BIT)
		    c_p = (char)unsign_c;
		if (unsign_c > 127 &&
		    me->T.output_utf8 &&
		    *me->utf_buf) {
		    HText_appendText(me->text, me->utf_buf);
		    me->utf_buf_p = me->utf_buf;
		    *(me->utf_buf_p) = '\0';

		} else if (me->T.trans_from_uni && unsign_c >= 127) {
		    sprintf(replace_buf,"U%.2lx",unsign_c);
		    HText_appendText(me->text, replace_buf);
		} else
#endif /* EXP_CHARTRANS */
	        HText_appendCharacter(me->text, c_p);
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
PRIVATE void HTPlain_free ARGS1(
	HTStream *,	me)
{
    FREE(me);
}

/*	End writing
*/
PRIVATE void HTPlain_abort ARGS2(
	HTStream *,	me,
	HTError,	e)
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

#ifdef EXP_CHARTRANS
    me->utf_count = 0;
    me->utf_char = 0;
    me->utf_buf[0] = me->utf_buf[6] = '\0';
    me->utf_buf_p = me->utf_buf;
    me->htext_char_set =
		      HTAnchor_getUCLYhndl(anchor,UCT_STAGE_HTEXT);
    me->in_char_set = HTAnchor_getUCLYhndl(anchor,UCT_STAGE_PARSER);
    HTPlain_getChartransInfo(me, anchor);
    UCSetTransParams(&me->T,
		     me->in_char_set, me->UCI,
		     me->htext_char_set,
		     HTAnchor_getUCInfoStage(anchor,UCT_STAGE_HTEXT));
#endif /* EXP_CHARTRANS */
    me->text = HText_new(anchor);
    HText_setStyle(me->text, HTStyleNamed(styleSheet, "Example"));
    HText_beginAppend(me->text);

    return (HTStream*) me;
}

