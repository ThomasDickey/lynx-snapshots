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
#include "tcp.h"

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

#include "LYCharSets.h"
#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

extern HTStyleSheet * styleSheet;

extern BOOLEAN LYRawMode;
extern BOOL HTPassEightBitRaw;
extern BOOL HTPassHighCtrlRaw;
extern HTCJKlang HTCJK;

PUBLIC int HTPlain_lastraw = -1;

/*		HTML Object
**		-----------
*/
struct _HTStream {
    CONST HTStreamClass *	isa;
    HText *			text;
    LYUCcharset	* UCI;	/* pointer to node_anchor's UCInfo */
    int	in_char_set;		/* tells us what charset we are fed */
    int	htext_char_set;		/* what charset we feed to HText */
    char			utf_count;
    UCode_t			utf_char;
    char 			utf_buf[8];
    char *			utf_buf_p;
    UCTransParams		T;
};

PRIVATE char replace_buf [64];        /* buffer for replacement strings */

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
    } else if (HTCJK != NOCJK) {
	HText_appendCharacter(me->text, c);
    } else if ((unsigned char)c >= 127) {
	/*
	**  For now, don't repeat everything here
	**  that has been done below - KW
	*/
	HTPlain_write(me, &c, 1);
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
	    current_char_set != 0) {
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
    BOOL chk;
    UCode_t code;
    long uck;
    char c_p;

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
	code = (unsigned char)*p;
	c_p = *p;

	if (me->T.decode_utf8) {
	    /*
	    **  Combine UTF-8 into Unicode.
	    **  Incomplete characters silently ignored.
	    **  from Linux kernel's console.c
	    */
	    if ((unsigned char)(*p) > 127) {
		if (me->utf_count > 0 && (*p & 0xc0) == 0x80) {
		    me->utf_char = (me->utf_char << 6) | (*p & 0x3f);
		    me->utf_count--;
		    *(me->utf_buf_p) = *p;
		    (me->utf_buf_p)++;
		    if (me->utf_count == 0) {
		        /*
			**  Got a complete multibyte character.
			*/
			*(me->utf_buf_p) = '\0';
			code = me->utf_char;
			if (code < 256) {
			    c_p = FROMASCII((char)code);
			}
		    } else {
			continue;  /* iterate for more */
		    }
		} else {
		    /*
		    **  Accumulate a multibyte character.
		    */
		    me->utf_buf_p = me->utf_buf;
		    *(me->utf_buf_p) = *p;
		    (me->utf_buf_p)++;
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
		    } else {
			/*
			 *  Garbage.
			 */
			me->utf_count = 0;
			me->utf_buf_p = me->utf_buf;
			*(me->utf_buf_p) = '\0';
		    }
		    continue; /* iterate for more */
		}
	    } else {
	        /*
		**  Got an ASCII character.
		*/
		me->utf_count = 0;
		me->utf_buf_p = me->utf_buf;
		*(me->utf_buf_p) = '\0';
	    }
	}
	
	if (me->T.trans_to_uni &&
	    (code >= 127 ||
	     (code < 32 && code != 0 && me->T.trans_C0_to_uni))) {
	    code = UCTransToUni(c_p, me->in_char_set);
	    if (code > 0) {
		if (code < 256) {
		    c_p = FROMASCII((char)code);
		}
	    }
	}
	/*
	**  At this point we have either code in Unicode
	**  (and c in latin1 if code is in the latin1 range),
	**  or code and c will have to be passed raw.
	*/

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
		    code >= LYlowest_eightbit[me->in_char_set])
#define PASS8859SPECL me->T.pass_160_173_raw
#define PASSHI8BIT (HTPassEightBitRaw || \
		    (me->T.do_8bitraw && !me->T.trans_from_uni))
#endif /* EXP_CHARTRANS */

	/*
	**  If HTPassHighCtrlRaw is set (e.g., for KOI8-R) assume the
	**  document matches and pass 127-160 8-bit characters.  If it
	**  doesn't match, the user should toggle raw/CJK mode off. - FM
	*/
	} else if (code >= 127 && code < 161 &&
		   PASSHICTRL && PASS8859SPECL) {
	    HText_appendCharacter(me->text, *p);
	} else if (code == 173 && PASS8859SPECL) {
	    HText_appendCharacter(me->text, *p);
	/*
	**  If neither HTPassHighCtrlRaw nor CJK is set, play it safe
	**  and treat 160 (nbsp) as an ASCII space (32). - FM
	*/
	} else if (code == 160) {
	    HText_appendCharacter(me->text, ' ');
	/*
	**  If neither HTPassHighCtrlRaw nor CJK is set, play it safe
	**  and ignore 173 (shy). - FM
	*/
	} else if (code == 173) {
	    continue;
	/*
	**  If we get to here, pass the displayable ASCII characters. - FM
	*/
	} else if ((code >= 32 && code < 127) ||
		   (PASSHI8BIT &&
		    c_p >= LYlowest_eightbit[me->htext_char_set]) ||
		   *p == '\n' || *p == '\t') {
	    HText_appendCharacter(me->text, c_p);

	} else if (me->T.use_raw_char_in) {
	    HText_appendCharacter(me->text, *p);
#ifdef NOTUSED_FOTEMODS
	/*
	**  Use an ASCII space (32) for ensp, emsp or thinsp. - FM 
	*/
	} else if (code == 8194 || code == 8195 || code == 8201) {
	    HText_appendCharacter(me->text, ' ');
	/*
	**  Use ASCII hyphen for 8211 (ndash/endash)
	**  or 8212 (mdash/emdash). - FM
	*/
	} else if (code == 8211 || code == 8212) {
	    HText_appendCharacter(me->text, '-');
#endif /* NOTUSED_FOTEMODS */

/******************************************************************
 *   I. LATIN-1 OR UCS2  TO  DISPLAY CHARSET
 ******************************************************************/  
	} else if ((chk = (me->T.trans_from_uni && code >= 160)) &&
		   (uck = UCTransUniChar(code,
					 me->htext_char_set)) >= 32 &&
		   uck < 256) {
	    if (TRACE) {
		fprintf(stderr,
			"UCTransUniChar returned 0x%.2lX:'%c'.\n",
			uck, FROMASCII((char)uck));
	    }
	    HText_appendCharacter(me->text, (char)(uck & 0xff));
	} else if (chk &&
		   (uck == -4 ||
		    (me->T.repl_translated_C0 && uck > 0 && uck < 32)) &&
		   /*
		   **  Not found; look for replacement string.
		   */
		   (uck = UCTransUniCharStr(replace_buf, 60, code,
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
	} else if (chk && code > 127 && me->T.output_utf8) {
	    /*
	    **  We want UTF-8 output, so do it now. - FM
	    */
	    if (*me->utf_buf) {
		HText_appendText(me->text, me->utf_buf);
		me->utf_buf_p = me->utf_buf;
		*(me->utf_buf_p) = '\0';
	    } else if (UCConvertUniToUtf8(code, replace_buf)) {
		HText_appendText(me->text, replace_buf);
	    } else {
		sprintf(replace_buf, "U%.2lX", code);
		HText_appendText(me->text, replace_buf);
	    }
#ifdef NOTUSED_FOTEMODS
	} else if (me->T.strip_raw_char_in &&
		   (unsigned char)*p >= 0xc0 &&
		   (unsigned char)*p < 255) {
	    /*
	    **  KOI special: strip high bit, gives
	    **  (somewhat) readable ASCII.
	    */
	    HText_appendCharacter(me->text, (char)(*p & 0x7f));
	    /*
	    **  If we do not have the "7-bit approximations" as our
	    **  output character set (in which case we did it already)
	    **  seek a translation for that.  Otherwise, or if the
	    **  translation fails, use UHHH notation. - FM
	    */
	} else if (chk &&
		   (chk = (!HTPassEightBitRaw &&
			   (me->htext_char_set !=
			    UCGetLYhndl_byMIME("us-ascii")))) &&
		   (uck = UCTransUniChar(code,
					 UCGetLYhndl_byMIME("us-ascii")))
				      >= 32 && uck < 127) {
		/*
		**  Got an ASCII character (yippey). - FM
		*/
	    c = ((char)(uck & 0xff));
	    HText_appendCharacter(me->text, c);
	} else if ((chk && uck == -4) &&
		       (uck = UCTransUniCharStr(replace_buf,
						60, code,
						UCGetLYhndl_byMIME("us-ascii"),
						0) >= 0)) {
		/*
		**  Got a repacement string (yippey). - FM
		*/
	    HText_appendText(me->text, replace_buf);
	} else if (code == 8204 || code == 8205) {
	    /*
	    **  Ignore 8204 (zwnj) or 8205 (zwj), if we get to here. - FM
	    */
	    if (TRACE) {
		fprintf(stderr,
			"HTPlain_write: Ignoring '%ld'.\n", code);
	    }
	} else if (code == 8206 || code == 8207) {
	    /*
	    **  Ignore 8206 (lrm) or 8207 (rlm), if we get to here. - FM
	    */
	    if (TRACE) {
		fprintf(stderr,
			"HTPlain_write: Ignoring '%ld'.\n", code);
	    }
#endif /* NOTUSED_FOTEMODS */
	} else if (me->T.trans_from_uni && code > 255) {
	    if (PASSHI8BIT && PASSHICTRL && LYRawMode &&
		(unsigned char)*p >= LYlowest_eightbit[me->htext_char_set]) {
		HText_appendCharacter(me->text, *p);
	    } else {
		sprintf(replace_buf, "U%.2lX", code);
		HText_appendText(me->text, replace_buf);
	    }
	/*
	**  If we get to here and HTPassEightBitRaw or the
	**  selected character set is not "ISO Latin 1",
	**  use the translation tables for 161-255 8-bit
	**  characters (173 was handled above). - FM
	*/
	} else if (code > 160) {
	    if (!HTPassEightBitRaw && code <= 255 &&
		me->htext_char_set != 0) {
		/*
		**  Attempt to translate. - FM
		*/
		int len, high, low, i, diff=1;
		CONST char * name;
		int value = (int)(code - 160);

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
					 LYCharSets[me->htext_char_set][i]);
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
			c_p = FROMASCII((char)code);
		    if (me->T.output_utf8 &&
			*me->utf_buf) {
			HText_appendText(me->text, me->utf_buf);
			me->utf_buf_p = me->utf_buf;
			*(me->utf_buf_p) = '\0';

		    } else if (me->T.trans_from_uni) {
			sprintf(replace_buf, "U%.2lX", code);
			HText_appendText(me->text, replace_buf);
		    } else
#endif /* EXP_CHARTRANS */
		    HText_appendCharacter(me->text, c_p);
		}
	    } else {
	        /*
		**  Didn't attempt a translation. - FM
		*/
		/*  Either output as UTF8 or a hex representation or
		**  pass the raw character and hope it's OK.
		*/
		if (code <= 255 && !PASSHI8BIT)
		    c_p = FROMASCII((char)code);
		if (code > 127 && me->T.output_utf8 && *me->utf_buf) {
		    HText_appendText(me->text, me->utf_buf);
		    me->utf_buf_p = me->utf_buf;
		    *(me->utf_buf_p) = '\0';

		} else if (LYRawMode &&
			   me->in_char_set != me->htext_char_set &&
			   (PASSHI8BIT || PASSHICTRL) &&
			   (unsigned char)c_p >=
			             LYlowest_eightbit[me->htext_char_set]) {
		    HText_appendCharacter(me->text, c_p);
		} else if (me->T.trans_from_uni && code >= 127) {
		    sprintf(replace_buf, "U%.2lX", code);
		    HText_appendText(me->text, replace_buf);
		} else
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
	"PlainPresenter",
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
    me->utf_buf[0] = me->utf_buf[6] =me->utf_buf[7] = '\0';
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

