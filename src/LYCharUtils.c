/*
**  Functions associated with LYCharSets.c and the Lynx version of HTML.c - FM
**  ==========================================================================
*/
#include "HTUtils.h"
#include "tcp.h"

#define Lynx_HTML_Handler
#include "HTChunk.h"
#include "HText.h"
#include "HTStyle.h"
#include "HTML.h"

#include "HTCJK.h"
#include "HTAtom.h"
#include "HTMLGen.h"
#include "HTParse.h"

#include "LYGlobalDefs.h"
#include "LYCharUtils.h"
#include "LYCharSets.h"

#ifdef EXP_CHARTRANS
#include "UCMap.h"
#include "UCDefs.h"
#include "UCAux.h"
#endif

#include "HTAlert.h"
#include "HTFont.h"
#include "HTForms.h"
#include "HTNestedList.h"
#include "GridText.h"
#include "LYSignal.h"
#include "LYUtils.h"
#include "LYMap.h"
#include "LYBookmark.h"

#ifdef VMS
#include "LYCurses.h"
#include "HTVMSUtils.h"
#endif /* VMS */
#ifdef DOSPATH
#include "HTDOS.h"
#endif

#include "LYexit.h"
#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

extern BOOL HTPassEightBitRaw;
extern BOOL HTPassEightBitNum;
extern BOOL HTPassHighCtrlRaw;
extern BOOL HTPassHighCtrlNum;
extern HTkcode kanji_code;
extern HTCJKlang HTCJK;

extern void LYSetCookie PARAMS((
	CONST char *	header,
	CONST char *	address));

/*
 *  Used for nested lists. - FM
 */
PUBLIC int OL_CONTINUE = -29999;     /* flag for whether CONTINUE is set */
PUBLIC int OL_VOID = -29998;	     /* flag for whether a count is set */


/*
**  This function converts HTML named entities within a string
**  to their translations in the active LYCharSets.c array.
**  It also converts numeric entities to their HTML entity names
**  and then similarly translates those.  The string is converted
**  in place, on the assumption that the conversion strings will
**  not be longer than the entity strings, such that the overall
**  string will never grow.  This assumption is true for the
**  current LYCharSets arrays.  Make sure it stays true!  If
**  plain_space is TRUE, nbsp (160) will be treated as an ASCII
**  space (32).  If hidden is TRUE, entities will be translated
**  but escape sequences will be passed unaltered. - FM
*/
PUBLIC char * LYUnEscapeEntities ARGS3(
	char *,	str,
	BOOLEAN, plain_space,
	BOOLEAN, hidden)
{
    char * p = str;
    char * q = str;
    char * cp;
    char cpe;
    int len, value;
    int high, low, diff = 0, i;
    enum _state
        { S_text, S_esc, S_dollar, S_paren,
	  S_nonascii_text, S_dollar_paren } state = S_text;

    /*
    **  Make sure we have a non-empty string. - FM
    */
    if (!str || *str == '\0')
        return str;

    /*
    **  Loop through string, making conversions as needed. - FM
    */
    while (*p) {
        if (HTCJK != NOCJK && !hidden) {
	    /*
	    **  Handle CJK escape sequences, based on patch
	    **  from Takuya ASADA (asada@three-a.co.jp). - FM
	    */
	    switch(state) {
	        case S_text:
		    if (*p == '\033') {
		        state = S_esc;
			*q++ = *p++;
			continue;
		    }
		    break;

		case S_esc:
		    if (*p == '$') {
		        state = S_dollar;
			*q++ = *p++;
			continue;
		    } else if (*p == '(') {
		        state = S_paren;
			*q++ = *p++;
			continue;
		    } else {
		        state = S_text;
		    }

		case S_dollar:
		    if (*p == '@' || *p == 'B' || *p == 'A') {
		        state = S_nonascii_text;
			*q++ = *p++;
			continue;
		    } else if (*p == '(') {
		        state = S_dollar_paren;
			*q++ = *p++;
			continue;
		    } else {
		        state = S_text;
		    }
		    break;

		case S_dollar_paren:
		    if (*p == 'C') {
		        state = S_nonascii_text;
			*q++ = *p++;
			continue;
		    } else {
		        state = S_text;
		    }
		    break;

		case S_paren:
		    if (*p == 'B' || *p == 'J' || *p == 'T')  {
		        state = S_text;
			*q++ = *p++;
			continue;
		    } else if (*p == 'I') {
		        state = S_nonascii_text;
			*q++ = *p++;
			continue;
		    } else {
		        state = S_text;
		    }
		    break;

		case S_nonascii_text:
		    if (*p == '\033')
		        state = S_esc;
		    *q++ = *p++;
		    continue;
		    break;

		default:
	            p++;
		    continue;
	    }
        } else if (*p == '\033' &&
		   !hidden) {
	    /*
	    **  CJK handling not on, and not a hidden INPUT,
	    **  so block escape. - FM
	    */
	    p++;
	    continue;
	}

	/*
	**  Check for a numeric or named entity. - FM
	*/
        if (*p == '&') {
	    p++;
	    len = strlen(p);
	    /*
	    **  Check for a numeric entity. - FM
	    */
	    if (*p == '#' && len > 2 &&
	        (unsigned char)*(p+1) < 127 &&
		isdigit((unsigned char)*(p+1))) {
		cp = ++p;
		while (*p && (unsigned char)*p < 127 &&
		       isdigit((unsigned char)*p))
		    p++;
		/*
		**  Make sure we had a valid terminator. - FM
		*/
		if ((unsigned char)*p >= 127 ||
		    isalnum((unsigned char)*p)) {
		    *q++ = '&';
		    *q++ = '#';
		    p = cp;
		    continue;
		}
		/*
		**  Save the terminator and isolate the digit(s). - FM
		*/
		cpe = *p;
		if (*p)
		    *p++ = '\0';
	        /*
		** Show the numeric entity if the value:
		**  (1) Is greater than 255 and unhandled Unicode.
		**  (2) Is less than 32, and not valid or we don't
		**	have HTCJK set.
		**  (3) Is 127 and we don't have HTPassHighCtrlRaw
		**	or HTCJK set.
		**  (4) Is 128 - 159 and we don't have HTPassHighCtrlNum set.
		*/
		if ((sscanf(cp, "%d", &value) != 1) ||
		    (value > 255 &&
		     value != 8194 && value != 8195 && value != 8201 &&
		     value != 8211 && value != 8212 && value != 8482) ||
		    (value < 32 &&
		     value != 9 && value != 10 && value != 13 &&
		     HTCJK == NOCJK) ||
		    (value == 127 &&
		     !(HTPassHighCtrlRaw || HTCJK != NOCJK)) ||
		    (value > 127 && value < 160 &&
		     !HTPassHighCtrlNum)) {
		    /*
		    **  Illegal or not yet handled value.
		    **  Recover the "&#" and continue
		    **  from there. - FM
		    */
		    *q++ = '&';
		    *q++ = '#';
		    if (cpe != '\0')
		       *(p-1) = cpe;
		    p = cp;
		    continue;
		/*
		**  For 160 (nbsp), use that value if it's
		**  a hidden INPUT, otherwise use an ASCII
		**  space (32) if plain_space is TRUE,
		**  otherwise use the Lynx special character. - FM
		*/
		} else if (value == 160) {
		    if (hidden) {
		        *q++ = 160;
		    } else if (plain_space) {
		        *q++ = ' ';
		    } else {
		        *q++ = HT_NON_BREAK_SPACE;
		    }
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		/*
		**  For 173 (shy), use that value if it's
		**  a hidden INPUT, otherwise ignore it if
		**  plain space is TRUE, otherwise use the
		**  Lynx special character. - FM
		*/
		} else if (value == 173) {
		    if (hidden) {
		        *q++ = 173;
		    } else if (plain_space) {
		        ;
		    } else {
		        *q++ = LY_SOFT_HYPHEN;
		    }
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		/*
		**  For 8194 (ensp), 8195 (emsp), or 8201 (thinsp),
		**  use the character reference if it's a hidden INPUT,
		**  otherwise use an ASCII space (32) if plain_space is
		**  TRUE, otherwise use the Lynx special character. - FM
		*/
		} else if (value == 8194 || value == 8195 || value == 8201) {
		    if (hidden) {
		        *q++ = '&';
		        *q++ = '#';
			if (cpe != '\0')
			    *(p-1) = cpe;
			p = cp;
			continue;
		    } else if (plain_space) {
		        *q++ = ' ';
		    } else {
		        *q++ = HT_EM_SPACE;
		    }
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		/*
		**  For 8211 (ndash) or 8212 (mdash), use the character
		**  reference if it's a hidden INPUT, otherwise use an
		**  ASCII dash. - FM
		*/
		} else if (value == 8211 || value == 8212) {
		    if (hidden) {
		        *q++ = '&';
		        *q++ = '#';
			if (cpe != '\0')
			    *(p-1) = cpe;
			p = cp;
			continue;
		    } else {
		        *q++ = '-';
		    }
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		/*
		**  For 8482 (trade) use the character reference if it's
		**  a hidden INPUT, otherwise use whatever the tables have
		**  for &trade;. - FM, kw
		*/
	        } else if (value == 8482 && hidden) {
		        *q++ = '&';
		        *q++ = '#';
			if (cpe != '\0')
			    *(p-1) = cpe;
			p = cp;
			continue;
		/*
		**  If it's ASCII, or is 8-bit but HTPassEightBitNum
		**  is set or the character set is "ISO Latin 1",
		**  use it's value. - FM
		*/
		} else if (value < 161 || 
			   (value < 256 && (HTPassEightBitNum ||
			   !strncmp(LYchar_set_names[current_char_set],
			   	    "ISO Latin 1", 11)))) {
		    /*
		    **  No conversion needed.
		    */
	            *q++ = (unsigned char)value;
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		/*
		**  If we get to here, convert and handle
		**  the character as a named entity. - FM
		*/
		} else {
		    CONST char * name;
		    if (value == 8482) { /* trade mark sign falls through to here -kw */
		      name = "trade";
		    } else {
		      value -= 160;
		      name = HTMLGetEntityName(value);
		    }
		    for(low = 0, high = HTML_dtd.number_of_entities;
		        high > low;
			diff < 0 ? (low = i+1) : (high = i)) {
			/* Binary search */
			i = (low + (high-low)/2);
			diff = strcmp(HTML_dtd.entity_names[i], name);
			if (diff == 0) {
			    /*
			    **  Found the entity.  Assume that the length
			    **  of the value does not exceed the length of
			    **  the raw entity, so that the overall string
			    **  does not need to grow.  Make sure this stays
			    **  true in the LYCharSets arrays. - FM
			    */
			    int j;
			    for (j = 0; p_entity_values[i][j]; j++)
			        *q++ = (unsigned char)(p_entity_values[i][j]);
			    break;
			}
		    }
		    /*
		    **  Recycle the terminator if it isn't the
		    **  standard ';' for HTML. - FM
		    */
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		}
	    /*
	    **  Check for a named entity. - FM
	    */
	    } else if ((unsigned char)*p < 127 &&
	    	       isalnum((unsigned char)*p)) {
		cp = p;
		while (*cp && (unsigned char)*cp < 127 &&
		       isalnum((unsigned char)*cp))
		    cp++;
		cpe = *cp;
		*cp = '\0';
		for (low = 0, high = HTML_dtd.number_of_entities;
		     high > low ;
		     diff < 0 ? (low = i+1) : (high = i)) {
		    /* Binary search */
		    i = (low + (high-low)/2);
		    diff = strcmp(HTML_dtd.entity_names[i], p);
		    if (diff == 0) {
		        /*
			**  Found the entity.  Assume that the length
			**  of the value does not exceed the length of
			**  the raw entity, so that the overall string
			**  does not need to grow.  Make sure this stays
			**  true in the LYCharSets arrays. - FM
			*/
			int j;
			/*
			**  If it's hidden, use 160 for nbsp. - FM
			*/
			if (hidden &&
			    !strcmp("nbsp", HTML_dtd.entity_names[i])) {
			    *q++ = 160;
			/*
			**  If it's hidden, use 173 for shy. - FM
			*/
			} else if (hidden &&
				   !strcmp("shy", HTML_dtd.entity_names[i])) {
			    *q++ = 173;
			/*
			**  Check whether we want a plain space for nbsp,
			**  ensp or emsp. - FM
			*/
			} else if (plain_space &&
				   (!strcmp("nbsp",
				   	    HTML_dtd.entity_names[i]) ||
				    !strcmp("emsp",
					    HTML_dtd.entity_names[i]) ||
				    !strcmp("ensp", 
					    HTML_dtd.entity_names[i]))) {
			    *q++ = ' ';
			/*
			**  If plain_space is set, ignore shy. - FM
			*/
			} else if (plain_space &&
				   !strcmp("shy",
					    HTML_dtd.entity_names[i])) {
			    ;
			/*
			**  If we haven't used something else, use the
			**  the translated value or string. - FM
			*/
			} else {
			    for (j = 0; p_entity_values[i][j]; j++) {
			        *q++ = (unsigned char)(p_entity_values[i][j]);
			    }
			}
			/*
			**  Recycle the terminator if it isn't the
			**  standard ';' for HTML. - FM
			*/
			*cp = cpe;
			if (*cp != ';')
			    p = cp;
			else
			    p = (cp+1);
			break;
		    }
		}
		*cp = cpe;
		if (diff != 0) {
		    /*
		    **  Entity not found.  Add the '&' and
		    **  continue processing from there. - FM
		    */
		    *q++ = '&';
		}
		continue;
	    /*
	    **  If we get to here, it's a raw ampersand. - FM
	    */
	    } else {
		*q++ = '&';
		continue;
	    }
	/*
	**  Not an entity.  Check whether we want nbsp, ensp,
	**  emsp (if translated elsewhere) or 160 converted to
	**  a plain space. - FM
	*/
	} else {
	    if ((plain_space) &&
	        (*p == HT_NON_BREAK_SPACE || *p == HT_EM_SPACE ||
		 (((unsigned char)*p) == 160 &&
		  !(hidden ||
		    HTPassHighCtrlRaw || HTPassHighCtrlNum ||
		    HTCJK != NOCJK)))) {
	        *q++ = ' ';
		p++;
	    } else {
	        *q++ = *p++;
	    }
	}
    }
    
    *q = '\0';
    return str;
}

/*
**  This function converts any named or numeric character
**  references in allocated strings to their ISO-8858-1
**  values or to our substitutes if they are not part of
**  that charset.  If the isURL flag is TRUE, it also
**  hex escapes ESC and any characters greater than 159,
**  and trims any leading or trailing blanks.  Otherwise,
**  it strips out ESC, as would be done when the
**  "ISO Latin 1" Character Set is selected. - FM
*/
PUBLIC void LYUnEscapeToLatinOne ARGS2(
	char **,	str,
	BOOLEAN,	isURL)
{
    char *p = *str;
    char *q = NULL;
    char *url = NULL;
    char *esc = NULL;
    char buf[2];
    int e;
    char *cp;
    char cpe;
    int len, value;
    int high, low, diff = 0, i;
    enum _state
        { S_text, S_esc, S_dollar, S_paren,
	  S_nonascii_text, S_dollar_paren } state = S_text;

    /*
    **  Make sure we have a non-empty string. - FM
    */
    if (!p || *p == '\0')
        return;
    buf[0] = buf[1] = '\0';

    /*
    **  If the isURL flag is TRUE, set up for hex escaping. - FM
    */
    if (isURL == TRUE) {
        if ((url = (char *)calloc(1, ((strlen(p) * 3) + 1))) == NULL) {
	    outofmem(__FILE__, "LYUnEscapeToLatinOne");
	}
	q = url;
    } else {
        q = p;
    }

    /*
    **  Loop through string, making conversions as needed. - FM
    */
    while (*p) {
	/*
	**  Handle any CJK escape sequences. - FM
	*/
	switch(state) {
	    case S_text:
		if (*p == '\033') {
		    state = S_esc;
		    if (isURL == TRUE) {
		        buf[0] = *p;
		        esc = HTEscape(buf, URL_XALPHAS);
			for (e = 0; esc[e]; e++)
		            *q++ = esc[e];
			FREE(esc);
		    }
		    p++;
		    continue;
		}
		break;

	    case S_esc:
		if (*p == '$') {
		    state = S_dollar;
		    *q++ = *p++;
		    continue;
		} else if (*p == '(') {
		    state = S_paren;
		   *q++ = *p++;
		   continue;
		} else {
		    state = S_text;
		}

	    case S_dollar:
		if (*p == '@' || *p == 'B' || *p == 'A') {
		    state = S_nonascii_text;
		    *q++ = *p++;
		    continue;
		} else if (*p == '(') {
		    state = S_dollar_paren;
		    *q++ = *p++;
		    continue;
		} else {
		    state = S_text;
		}
		break;

	   case S_dollar_paren:
		if (*p == 'C') {
		    state = S_nonascii_text;
		    *q++ = *p++;
		    continue;
		} else {
		    state = S_text;
		}
		break;

	    case S_paren:
		if (*p == 'B' || *p == 'J' || *p == 'T')  {
		    state = S_text;
		    *q++ = *p++;
		    continue;
		} else if (*p == 'I') {
		    state = S_nonascii_text;
		    *q++ = *p++;
		    continue;
		} else {
		    state = S_text;
		}
		break;

	    case S_nonascii_text:
		if (*p == '\033') {
		    state = S_esc;
		    if (isURL == TRUE) {
		        buf[0] = *p;
		        esc = HTEscape(buf, URL_XALPHAS);
			for (e = 0; esc[e]; e++)
		            *q++ = esc[e];
			FREE(esc);
		    }
		    p++;
		} else {
		    *q++ = *p++;
		}
		continue;
		break;

	    default:
	        p++;
		continue;
	}

	/*
	**  Check for a numeric or named entity. - FM
	*/
        if (*p == '&') {
	    p++;
	    len = strlen(p);
	    /*
	    **  Check for a numeric entity. - FM
	    */
	    if (*p == '#' && len > 2 &&
	        (unsigned char)*(p+1) < 127 &&
		isalnum((unsigned char)*(p+1))) {
		cp = ++p;
		while (*p && (unsigned char)*p < 127 &&
		       isalnum((unsigned char)*p))
		    p++;
		cpe = *p;
		if (*p)
		    *p++ = '\0';
	        /*
		** Show the numeric entity if the value:
		**  (1) Is greater than 255 (until we support Unicode).
		**  (2) Is less than 32 and not valid.
		**  (3) Is 127.
		**  (4) Is 128 - 159.
		*/
		if ((sscanf(cp, "%d", &value) != 1) ||
		    (value > 255) ||
		    (value < 32 &&
		     value != 9 && value != 10 && value != 13) ||
		    (value == 127) ||
		    (value > 127 && value < 160)) {
		    /*
		    **  Illegal or not yet handled value.
		    **  Recover the "&#" and continue
		    **  from there. - FM
		    */
		    *q++ = '&';
		    *q++ = '#';
		    if (cpe != '\0')
		       *(p-1) = cpe;
		    p = cp;
		    continue;
		}
		/*
		**  Convert the value as an unsigned char,
		**  hex escaped if isURL is set and it's
		**  8-bit, and then recycle the terminator
		**  if it is not a semicolon. - FM
		*/
		if (value > 159 && isURL == TRUE) {
		    buf[0] = value;
		    esc = HTEscape(buf, URL_XALPHAS);
		    for (e = 0; esc[e]; e++)
		        *q++ = esc[e];
		    FREE(esc);
		} else {
	            *q++ = (unsigned char)value;
		}
		if (cpe != ';' && cpe != '\0') {
		    p--;
		    *p = cpe;
		}
		continue;
	    /*
	    **  Check for a named entity. - FM
	    */
	    } else if ((unsigned char)*p < 127 &&
	    	       isalnum((unsigned char)*p)) {
		cp = p;
		while (*cp && (unsigned char)*cp < 127 &&
		       isalnum((unsigned char)*cp))
		    cp++;
		cpe = *cp;
		*cp = '\0';
		for (low = 0, high = HTML_dtd.number_of_entities;
		     high > low ;
		     diff < 0 ? (low = i+1) : (high = i)) {
		    /* Binary search */
		    i = (low + (high-low)/2);
		    diff = strcmp(HTML_dtd.entity_names[i], p);
		    if (diff == 0) {
		        /*
			**  Found the entity. Convert it to
			**  an ISO-8859-1 character, or our
			**  substitute for any non-ISO-8859-1
			**  character, hex escaped if isURL
			**  is set and it's 8-bit. - FM
			*/
			buf[0] = HTMLGetLatinOneValue(i);
                        if (buf[0] == '\0') {
                            /*
                            **  The entity does not have an 8859-1 representation
                            **  of exactly one char length.  Try to deal with it
                            **  anyway - either HTEscape the whole mess, or pass
                            **  through raw.  So make sure the ISO_Latin1 table,
                            **  which is the first table in LYCharSets, has resonable
			    **  substitution strings! (if it really must have any
			    **  longer than one char..) -kw
                            */
			    if (!LYCharSets[0][i][0]) /* totally empty, skip - kw */
			        /* do nothing */ ;
			    else if (isURL) {
			      /* *All* will be HTEscape'd - kw */
			      esc = HTEscape(LYCharSets[0][i], URL_XALPHAS);
			      for (e = 0; esc[e]; e++)
				*q++ = esc[e];
			      FREE(esc);
			    } else {
			      /* *Nothing* will be HTEscape'd - kw */
			      for (e = 0; LYCharSets[0][i][e]; e++)
                                *q++ = (unsigned char)(LYCharSets[0][i][e]);
			    }
                        } else if ((unsigned char)buf[0] > 159 && isURL == TRUE) {
			    esc = HTEscape(buf, URL_XALPHAS);
			    for (e = 0; esc[e]; e++)
			    *q++ = esc[e];
			    FREE(esc);
			} else {
			    *q++ = buf[0];
			}
			/*
			**  Recycle the terminator if it isn't
			**  the standard ';' for HTML. - FM
			*/
			*cp = cpe;
			if (*cp != ';')
			    p = cp;
			else
			    p = (cp+1);
			break;
		    }
		}
		*cp = cpe;
		if (diff != 0) {
		    /*
		    **  Entity not found.  Add the '&' and
		    **  continue processing from there. - FM
		    */
		    *q++ = '&';
		}
		continue;
	    /*
	    **  If we get to here, it's a raw ampersand. - FM
	    */
	    } else {
		*q++ = '&';
		continue;
	    }
	/*
	**  Not an entity.  Use the character. - FM
	*/
	} else {
	  *q++ = *p++;
	}
    }
    
    /*
    **  Clean up and return. - FM
    */
    *q = '\0';
    if (isURL == TRUE) {
        LYTrimHead(url);
	LYTrimTail(url);
        StrAllocCopy(*str, url);
	FREE(url);
    }
}

/*
**  This function reallocates an allocated string with
**  8-bit printable Latin characters (>= 160) converted
**  to their HTML entity names and then translated for
**  the current character set. - FM
*/
PUBLIC void LYExpandString ARGS1(
	char **, str)
{
    char *p = *str;
    char *q = *str;
    CONST char *name;
    int i, j, value, high, low, diff = 0;

    /*
    **  Don't do anything if we have no string
    **  or are in CJK mode. - FM
    */
    if (!p || *p == '\0' ||
        HTCJK != NOCJK)
        return;

    /*
    **  Start a clean copy of the string, without
    **  invalidating our pointer to the original. - FM
    */
    *str = NULL;
    StrAllocCopy(*str, "");

    /*
    **  Check each character in the original string,
    **  and add the characters or substitutions to
    **  our clean copy. - FM
    */
    for (i = 0; p[i]; i++) {
	/*
	**  Substitute Lynx special character for
	**  160 (nbsp) if HTPassHighCtrlRaw is not
	**  set. - FM
	*/
        if (((unsigned char)p[i]) == 160 &&
	    !HTPassHighCtrlRaw) {
	    p[i] = HT_NON_BREAK_SPACE;
	/*
	**  Substitute Lynx special character for
	**  173 (shy) if HTPassHighCtrlRaw is not
	**  set. - FM
	*/
        } else if (((unsigned char)p[i]) == 173 &&
	    !HTPassHighCtrlRaw) {
	    p[i] = LY_SOFT_HYPHEN;
	/*
	**  Substitute other 8-bit characters based on
	**  the LYCharsets.c tables if HTPassEightBitRaw
	**  is not set. - FM
	*/
	} else if (((unsigned char)p[i]) > 160 &&
		   !HTPassEightBitRaw) {
	    value = (int)(((unsigned char)p[i]) - 160);
	    p[i] = '\0';
	    StrAllocCat(*str, q);
	    q = &p[i+1];
	    name = HTMLGetEntityName(value);
	    for (low = 0, high = HTML_dtd.number_of_entities;
		 high > low;
		 diff < 0 ? (low = j+1) : (high = j)) {
		/* Binary search */
		j = (low + (high-low)/2);
		diff = strcmp(HTML_dtd.entity_names[j], name);
		if (diff == 0) {
		    StrAllocCat(*str, p_entity_values[j]);
		    break;
		}
	    }
	}
    }
    StrAllocCat(*str, q);
    free_and_clear(&p);
}

/*
**  This function converts any ampersands in allocated
**  strings to "&amp;".  If isTITLE is TRUE, it also
**  converts any angle-brackets to "&lt;" or "&gt;". - FM
*/
PUBLIC void LYEntify ARGS2(
	char **,	str,
	BOOLEAN,	isTITLE)
{
    char *p = *str;
    char *q = NULL, *cp = NULL;
    int amps = 0, lts = 0, gts = 0;
    
    if (p == NULL || *p == '\0')
        return;

    /*
     *  Count the ampersands. - FM
     */
    while ((*p != '\0') && (q = strchr(p, '&')) != NULL) {
        amps++;
	p = (q + 1);
    }

    /*
     *  Count the left-angle-brackets, if needed. - FM
     */
    if (isTITLE == TRUE) {
        p = *str;
	while ((*p != '\0') && (q = strchr(p, '<')) != NULL) {
	    lts++;
	    p = (q + 1);
	}
    }

    /*
     *  Count the right-angle-brackets, if needed. - FM
     */
    if (isTITLE == TRUE) {
        p = *str;
	while ((*p != '\0') && (q = strchr(p, '>')) != NULL) {
	    gts++;
	    p = (q + 1);
	}
    }

    /*
     *  Check whether we need to convert anything. - FM
     */
    if (amps == 0 && lts == 0 && gts == 0)
        return;

    /*
     *  Allocate space and convert. - FM
     */
    q = (char *)calloc(1,
    		     (strlen(*str) + (4 * amps) + (3 * lts) + (3 * gts) + 1));
    if ((cp = q) == NULL)
        outofmem(__FILE__, "LYEntify");
    for (p = *str; *p; p++) {
    	if (*p == '&') {
	    *q++ = '&';
	    *q++ = 'a';
	    *q++ = 'm';
	    *q++ = 'p';
	    *q++ = ';';
	} else if (isTITLE && *p == '<') {
	    *q++ = '&';
	    *q++ = 'l';
	    *q++ = 't';
	    *q++ = ';';
	} else if (isTITLE && *p == '>') {
	    *q++ = '&';
	    *q++ = 'g';
	    *q++ = 't';
	    *q++ = ';';
	} else {
	    *q++ = *p;
	}
    }
    StrAllocCopy(*str, cp);
}

/*
**  This function trims characters <= that of a space (32),
**  including HT_NON_BREAK_SPACE (1) and HT_EM_SPACE (2),
**  but not ESC, from the heads of strings. - FM
*/
PUBLIC void LYTrimHead ARGS1(
	char *, str)
{
    int i = 0, j;

    if (!str || *str == '\0')
        return;

    while (str[i] != '\0' && WHITE(str[i]) && (unsigned char)str[i] != 27) 
        i++;
    if (i > 0) {
        for (j = 0; str[i] != '\0'; i++) {
	    str[j++] = str[i];
	}
	str[j] = '\0';
    }
}

/*
**  This function trims characters <= that of a space (32),
**  including HT_NON_BREAK_SPACE (1), HT_EM_SPACE (2), and
**  ESC from the tails of strings. - FM
*/
PUBLIC void LYTrimTail ARGS1(
	char *, str)
{
    int i;

    if (!str || *str == '\0')
        return;

    i = (strlen(str) - 1);
    while (i >= 0) {
	if (WHITE(str[i]))
	    str[i] = '\0';
	else
	    break;
	i--;
    }
}

/*
** This function should receive a pointer to the start
** of a comment.  It returns a pointer to the end ('>')
** character of comment, or it's best guess if the comment
** is invalid. - FM
*/
PUBLIC char *LYFindEndOfComment ARGS1(
	char *, str)
{
    char *cp, *cp1;
    enum comment_state { start1, start2, end1, end2 } state;

    if (str == NULL)
        /*
	 *  We got NULL, so return NULL. - FM
	 */
        return NULL;

    if (strncmp(str, "<!--", 4))
        /*
	 *  We don't have the start of a comment, so
	 *  return the beginning of the string. - FM
	 */
        return str;

    cp = (str + 4);
    if (*cp =='>')
        /*
	 * It's an invalid comment, so
	 * return this end character. - FM
	 */
	return cp;

    if ((cp1 = strchr(cp, '>')) == NULL)
        /*
	 *  We don't have an end character, so
	 *  return the beginning of the string. - FM
	 */
	return str;

    if (*cp == '-')
        /*
	 *  Ugh, it's a "decorative" series of dashes,
	 *  so return the next end character. - FM
	 */
	return cp1;

    /*
     *  OK, we're ready to start parsing. - FM
     */
    state = start2;
    while (*cp != '\0') {
        switch (state) {
	    case start1:
	        if (*cp == '-')
		    state = start2;
		else
		    /*
		     *  Invalid comment, so return the first
		     *  '>' from the start of the string. - FM
		     */
		    return cp1;
		break;

	    case start2:
	        if (*cp == '-')
		    state = end1;
		break;

	    case end1:
	        if (*cp == '-')
		    state = end2;
		else
		    /*
		     *  Invalid comment, so return the first
		     *  '>' from the start of the string. - FM
		     */
		    return cp1;
		break;

	    case end2:
	        if (*cp == '>')
		    /*
		     *  Valid comment, so return the end character. - FM
		     */
		    return cp;
		if (*cp == '-') {
		    state = start1;
		} else if (!(WHITE(*cp) && (unsigned char)*cp != 27)) {
		    /*
		     *  Invalid comment, so return the first
		     *  '>' from the start of the string. - FM
		     */
		    return cp1;
		 }
		break;

	    default:
		break;
	}
	cp++;
    }

    /*
     *  Invalid comment, so return the first
     *  '>' from the start of the string. - FM
     */
    return cp1;
}

/*
**  If an HREF, itself or if resolved against a base,
**  represents a file URL, and the host is defaulted,
**  force in "//localhost".  We need this until
**  all the other Lynx code which performs security
**  checks based on the "localhost" string is changed
**  to assume "//localhost" when a host field is not
**  present in file URLs - FM
*/
PUBLIC void LYFillLocalFileURL ARGS2(
	char **,	href,
	char *,		base)
{
    char * temp = NULL;

    if (*href == NULL || *(*href) == '\0')
        return;

    if (!strcmp(*href, "//") || !strncmp(*href, "///", 3)) {
	if (base != NULL && !strncmp(base, "file:", 5)) {
	    StrAllocCopy(temp, "file:");
	    StrAllocCat(temp, *href);
	    StrAllocCopy(*href, temp);
	}
    }
    if (!strncmp(*href, "file:", 5)) {
	if (*(*href+5) == '\0') {
	    StrAllocCat(*href, "//localhost");
	} else if (!strcmp(*href, "file://")) {
	    StrAllocCat(*href, "localhost");
	} else if (!strncmp(*href, "file:///", 8)) {
	    StrAllocCopy(temp, (*href+7));
	    StrAllocCopy(*href, "file://localhost");
	    StrAllocCat(*href, temp);
	} else if (!strncmp(*href, "file:/", 6) && *(*href+6) != '/') {
	    StrAllocCopy(temp, (*href+5));
	    StrAllocCopy(*href, "file://localhost");
	    StrAllocCat(*href, temp);
	}
    }

    /*
     * No path in a file://localhost URL means a
     * directory listing for the current default. - FM
     */
    if (!strcmp(*href, "file://localhost")) {
#ifdef VMS
	StrAllocCat(*href, HTVMS_wwwName(getenv("PATH")));
#else
	char curdir[DIRNAMESIZE];
#if HAVE_GETCWD
	getcwd (curdir, DIRNAMESIZE);
#else
	getwd (curdir);
#endif /* NO_GETCWD */
#ifdef DOSPATH
	StrAllocCat(*href, HTDOS_wwwName(curdir));
#else
	StrAllocCat(*href, curdir);
#endif /* DOSPATH */
#endif /* VMS */
    }

#ifdef VMS
    /*
     * On VMS, a file://localhost/ URL means
     * a listing for the login directory. - FM
     */
    if (!strcmp(*href, "file://localhost/"))
	StrAllocCat(*href, (HTVMS_wwwName((char *)Home_Dir())+1));
#endif /* VMS */

    FREE(temp);
    return;
}

/*
** This function returns OL TYPE="A" strings in
** the range of " A." (1) to "ZZZ." (18278). - FM
*/
PUBLIC char *LYUppercaseA_OL_String ARGS1(
	int, seqnum)
{
    static char OLstring[8];

    if (seqnum <= 1 ) {
        strcpy(OLstring, " A.");
        return OLstring;
    }
    if (seqnum < 27) {
        sprintf(OLstring, " %c.", (seqnum + 64));
        return OLstring;
    }
    if (seqnum < 703) {
        sprintf(OLstring, "%c%c.", ((seqnum-1)/26 + 64),
		(seqnum - ((seqnum-1)/26)*26 + 64));
        return OLstring;
    }
    if (seqnum < 18279) {
        sprintf(OLstring, "%c%c%c.", ((seqnum-27)/676 + 64),
		(((seqnum - ((seqnum-27)/676)*676)-1)/26 + 64),
		(seqnum - ((seqnum-1)/26)*26 + 64));
        return OLstring;
    }
    strcpy(OLstring, "ZZZ.");
    return OLstring;
}

/*
** This function returns OL TYPE="a" strings in
** the range of " a." (1) to "zzz." (18278). - FM
*/
PUBLIC char *LYLowercaseA_OL_String ARGS1(
	int, seqnum)
{
    static char OLstring[8];

    if (seqnum <= 1 ) {
        strcpy(OLstring, " a.");
        return OLstring;
    }
    if (seqnum < 27) {
        sprintf(OLstring, " %c.", (seqnum + 96));
        return OLstring;
    }
    if (seqnum < 703) {
        sprintf(OLstring, "%c%c.", ((seqnum-1)/26 + 96),
		(seqnum - ((seqnum-1)/26)*26 + 96));
        return OLstring;
    }
    if (seqnum < 18279) {
        sprintf(OLstring, "%c%c%c.", ((seqnum-27)/676 + 96),
		(((seqnum - ((seqnum-27)/676)*676)-1)/26 + 96),
		(seqnum - ((seqnum-1)/26)*26 + 96));
        return OLstring;
    }
    strcpy(OLstring, "zzz.");
    return OLstring;
}

/*
** This function returns OL TYPE="I" strings in the
** range of " I." (1) to "MMM." (3000).- FM
*/
PUBLIC char *LYUppercaseI_OL_String ARGS1(
	int, seqnum)
{
    static char OLstring[8];
    int Arabic = seqnum;

    if (Arabic >= 3000) {
        strcpy(OLstring, "MMM.");
        return OLstring;
    }

    switch(Arabic) {
    case 1:
        strcpy(OLstring, " I.");
        return OLstring;
    case 5:
        strcpy(OLstring, " V.");
        return OLstring;
    case 10:
        strcpy(OLstring, " X.");
        return OLstring;
    case 50:
        strcpy(OLstring, " L.");
        return OLstring;
    case 100:
        strcpy(OLstring, " C.");
        return OLstring;
    case 500:
        strcpy(OLstring, " D.");
        return OLstring;
    case 1000:
        strcpy(OLstring, " M.");
        return OLstring;
    default:
        OLstring[0] = '\0';
	break;
    }

    while (Arabic >= 1000) {
        strcat(OLstring, "M");
        Arabic -= 1000;
    }

    if (Arabic >= 900) {
        strcat(OLstring, "CM");
	Arabic -= 900;
    }

    if (Arabic >= 500) {
	strcat(OLstring, "D");
        Arabic -= 500;
	while (Arabic >= 500) {
	    strcat(OLstring, "C");
	    Arabic -= 10;
	}
    }

    if (Arabic >= 400) {
	strcat(OLstring, "CD");
        Arabic -= 400;
    }

    while (Arabic >= 100) {
        strcat(OLstring, "C");
        Arabic -= 100;
    }

    if (Arabic >= 90) {
        strcat(OLstring, "XC");
	Arabic -= 90;
    }

    if (Arabic >= 50) {
	strcat(OLstring, "L");
        Arabic -= 50;
	while (Arabic >= 50) {
	    strcat(OLstring, "X");
	    Arabic -= 10;
	}
    }

    if (Arabic >= 40) {
	strcat(OLstring, "XL");
        Arabic -= 40;
    }

    while (Arabic > 10) {
        strcat(OLstring, "X");
	Arabic -= 10;
    }    

    switch (Arabic) {
    case 1:
        strcat(OLstring, "I.");
	break;
    case 2:
        strcat(OLstring, "II.");
	break;
    case 3:
        strcat(OLstring, "III.");
	break;
    case 4:
        strcat(OLstring, "IV.");
	break;
    case 5:
        strcat(OLstring, "V.");
	break;
    case 6:
        strcat(OLstring, "VI.");
	break;
    case 7:
        strcat(OLstring, "VII.");
	break;
    case 8:
        strcat(OLstring, "VIII.");
	break;
    case 9:
        strcat(OLstring, "IX.");
	break;
    case 10:
        strcat(OLstring, "X.");
	break;
    default:
        strcat(OLstring, ".");
	break;
    }

    return OLstring;
}

/*
** This function returns OL TYPE="i" strings in
** range of " i." (1) to "mmm." (3000).- FM
*/
PUBLIC char *LYLowercaseI_OL_String ARGS1(
	int, seqnum)
{
    static char OLstring[8];
    int Arabic = seqnum;

    if (Arabic >= 3000) {
        strcpy(OLstring, "mmm.");
        return OLstring;
    }

    switch(Arabic) {
    case 1:
        strcpy(OLstring, " i.");
        return OLstring;
    case 5:
        strcpy(OLstring, " v.");
        return OLstring;
    case 10:
        strcpy(OLstring, " x.");
        return OLstring;
    case 50:
        strcpy(OLstring, " l.");
        return OLstring;
    case 100:
        strcpy(OLstring, " c.");
        return OLstring;
    case 500:
        strcpy(OLstring, " d.");
        return OLstring;
    case 1000:
        strcpy(OLstring, " m.");
        return OLstring;
    default:
        OLstring[0] = '\0';
	break;
    }

    while (Arabic >= 1000) {
        strcat(OLstring, "m");
        Arabic -= 1000;
    }

    if (Arabic >= 900) {
        strcat(OLstring, "cm");
	Arabic -= 900;
    }

    if (Arabic >= 500) {
	strcat(OLstring, "d");
        Arabic -= 500;
	while (Arabic >= 500) {
	    strcat(OLstring, "c");
	    Arabic -= 10;
	}
    }

    if (Arabic >= 400) {
	strcat(OLstring, "cd");
        Arabic -= 400;
    }

    while (Arabic >= 100) {
        strcat(OLstring, "c");
        Arabic -= 100;
    }

    if (Arabic >= 90) {
        strcat(OLstring, "xc");
	Arabic -= 90;
    }

    if (Arabic >= 50) {
	strcat(OLstring, "l");
        Arabic -= 50;
	while (Arabic >= 50) {
	    strcat(OLstring, "x");
	    Arabic -= 10;
	}
    }

    if (Arabic >= 40) {
	strcat(OLstring, "xl");
        Arabic -= 40;
    }

    while (Arabic > 10) {
        strcat(OLstring, "x");
	Arabic -= 10;
    }    

    switch (Arabic) {
    case 1:
        strcat(OLstring, "i.");
	break;
    case 2:
        strcat(OLstring, "ii.");
	break;
    case 3:
        strcat(OLstring, "iii.");
	break;
    case 4:
        strcat(OLstring, "iv.");
	break;
    case 5:
        strcat(OLstring, "v.");
	break;
    case 6:
        strcat(OLstring, "vi.");
	break;
    case 7:
        strcat(OLstring, "vii.");
	break;
    case 8:
        strcat(OLstring, "viii.");
	break;
    case 9:
        strcat(OLstring, "ix.");
	break;
    case 10:
        strcat(OLstring, "x.");
	break;
    default:
        strcat(OLstring, ".");
	break;
    }

    return OLstring;
}

/*
**  This function initializes the Ordered List counter. - FM
*/
PUBLIC void LYZero_OL_Counter ARGS1(
	HTStructured *, 	me)
{
    int i;

    if (!me)
        return;

    for (i = 0; i < 7; i++) {
        me->OL_Counter[i] = OL_VOID;
	me->OL_Type[i] = '1';
    }
	
    me->Last_OL_Count = 0;
    me->Last_OL_Type = '1';
    
    return;
}

#ifdef EXP_CHARTRANS
/*
**  This function is used by the HTML Structured object. - kw
*/
PUBLIC void html_get_chartrans_info ARGS1(HTStructured *, me)
{
    me->UCLYhndl = HTAnchor_getUCLYhndl(me->node_anchor,UCT_STAGE_STRUCTURED);
    if (me->UCLYhndl < 0) {
	int chndl = HTAnchor_getUCLYhndl(me->node_anchor, UCT_STAGE_HTEXT);
	if (chndl < 0) {
	    chndl = current_char_set;
	    HTAnchor_setUCInfoStage(me->node_anchor, chndl, UCT_STAGE_HTEXT,
			    UCT_SETBY_STRUCTURED);
	}
	HTAnchor_setUCInfoStage(me->node_anchor, chndl,
				UCT_STAGE_STRUCTURED, UCT_SETBY_STRUCTURED);
	me->UCLYhndl = HTAnchor_getUCLYhndl(me->node_anchor,
					    UCT_STAGE_STRUCTURED);
    }
    me->UCI = HTAnchor_getUCInfoStage(me->node_anchor,UCT_STAGE_STRUCTURED);
}
#endif /* EXP_CHARTRANS */

/*
**  This function processes META tags in HTML streams. - FM
*/
PUBLIC void LYHandleMETA ARGS4(
	HTStructured *, 	me,
	CONST BOOL*,	 	present,
	CONST char **,		value,
	char **,		include)
{
    char *http_equiv = NULL, *name = NULL, *content = NULL;
    char *href = NULL, *id_string = NULL, *temp = NULL;
    char *cp, *cp0, *cp1 = 0;
    int url_type = 0, i;

    if (!me || !present)
        return;

    /*
     *  Load the attributes for possible use by Lynx. - FM
     */
    if (present[HTML_META_HTTP_EQUIV] &&
	value[HTML_META_HTTP_EQUIV] && *value[HTML_META_HTTP_EQUIV]) {
	StrAllocCopy(http_equiv, value[HTML_META_HTTP_EQUIV]);
	convert_to_spaces(http_equiv, TRUE);
	LYUnEscapeToLatinOne(&http_equiv, FALSE);
	LYTrimHead(http_equiv);
	LYTrimTail(http_equiv);
	if (*http_equiv == '\0') {
	    FREE(http_equiv);
	}
    }
    if (present[HTML_META_NAME] &&
	value[HTML_META_NAME] && *value[HTML_META_NAME]) {
	StrAllocCopy(name, value[HTML_META_NAME]);
	convert_to_spaces(name, TRUE);
	LYUnEscapeToLatinOne(&name, FALSE);
	LYTrimHead(name);
	LYTrimTail(name);
	if (*name == '\0') {
	    FREE(name);
	}
    }
    if (present[HTML_META_CONTENT] &&
	value[HTML_META_CONTENT] && *value[HTML_META_CONTENT]) {
	/*
	 *  Technically, we should be creating a comma-separated
	 *  list, but META tags come one at a time, and we'll
	 *  handle (or ignore) them as each is received.  Also,
	 *  at this point, we only trim leading and trailing
	 *  blanks from the CONTENT value, without translating
	 *  any named entities or numeric character references,
	 *  because how we should do that depends on what type
	 *  of information it contains, and whether or not any
	 *  of it might be sent to the screen. - FM
	 */
	StrAllocCopy(content, value[HTML_META_CONTENT]);
	convert_to_spaces(content, FALSE);
	LYTrimHead(content);
	LYTrimTail(content);
	if (*content == '\0') {
	    FREE(content);
	}
    }
    if (TRACE) {
	fprintf(stderr,
	        "LYHandleMETA: HTTP-EQUIV=\"%s\" NAME=\"%s\" CONTENT=\"%s\"\n",
		(http_equiv ? http_equiv : "NULL"),
		(name ? name : "NULL"),
		(content ? content : "NULL"));
    }

    /*
     *  Make sure we have META name/value pairs to handle. - FM
     */
    if (!(http_equiv || name) || !content)
        goto free_META_copies;
		
    /*
     * Check for a no-cache Pragma
     * or Cache-Control directive. - FM
     */
    if (!strcasecomp((http_equiv ? http_equiv : name), "Pragma") ||
        !strcasecomp((http_equiv ? http_equiv : name), "Cache-Control")) {
	LYUnEscapeToLatinOne(&content, FALSE);
	LYTrimHead(content);
	LYTrimTail(content);
	if (!strcasecomp(content, "no-cache")) {
	    me->node_anchor->no_cache = TRUE;
	    HText_setNoCache(me->text);
	}

	/*
	 *  If we didn't get a Cache-Control MIME header,
	 *  and the META has one, convert to lowercase,
	 *  store it in the anchor element, and if we
	 *  haven't yet set no_cache, check whether we
	 *  should. - FM
	 */
	if ((!me->node_anchor->cache_control) &&
	    !strcasecomp((http_equiv ? http_equiv : name), "Cache-Control")) {
	    for (i = 0; content[i]; i++)
		 content[i] = TOLOWER(content[i]);
	    StrAllocCopy(me->node_anchor->cache_control, content);
	    if (me->node_anchor->no_cache == FALSE) {
	        cp0 = content;
		while ((cp = strstr(cp0, "no-cache")) != NULL) {
		    cp += 8;
		    while (*cp != '\0' && WHITE(*cp))
			cp++;
		    if (*cp == '\0' || *cp == ';') {
			me->node_anchor->no_cache = TRUE;
			HText_setNoCache(me->text);
			break;
		    }
		    cp0 = cp;
		}
		if (me->node_anchor->no_cache == TRUE)
		    goto free_META_copies;
		cp0 = content;
		while ((cp = strstr(cp0, "max-age")) != NULL) {
		    cp += 7;
		    while (*cp != '\0' && WHITE(*cp))
			cp++;
		    if (*cp == '=') {
			cp++;
			while (*cp != '\0' && WHITE(*cp))
			    cp++;
			if (isdigit((unsigned char)*cp)) {
			    cp0 = cp;
			    while (isdigit((unsigned char)*cp))
				cp++;
			    if (*cp0 == '0' && cp == (cp0 + 1)) {
			        me->node_anchor->no_cache = TRUE;
				HText_setNoCache(me->text);
				break;
			    }
			}
		    }
		    cp0 = cp;
		}
	    }
	}

    /*
     * Check for an Expires directive. - FM
     */
    } else if (!strcasecomp((http_equiv ? http_equiv : name), "Expires")) {
	/*
	 *  If we didn't get a Expires MIME header,
	 *  store it in the anchor element, and if we
	 *  haven't yet set no_cache, check whether we
	 *  should. - FM
	 */
	LYUnEscapeToLatinOne(&content, FALSE);
	LYTrimHead(content);
	LYTrimTail(content);
	StrAllocCopy(me->node_anchor->expires, content);
	if (me->node_anchor->no_cache == FALSE) {
	    if ((content[0] == '0' && content[1] == '\0') ||
		LYmktime(content) <= 0) {
		me->node_anchor->no_cache = TRUE;
		HText_setNoCache(me->text);
	    }
	}

    /*
     *  Check for a text/html Content-Type with a
     *  charset directive, if we didn't already set
     *  the charset via a server's header. - AAC & FM
     */
    } else if (!(me->node_anchor->charset && *me->node_anchor->charset) && 
	       !strcasecomp((http_equiv ? http_equiv : name), "Content-Type")) {
	LYUnEscapeToLatinOne(&content, FALSE);
	LYTrimHead(content);
	LYTrimTail(content);
	/*
	 *  Force the Content-type value to all lower case. - FM
	 */
	for (cp = content; *cp; cp++)
	    *cp = TOLOWER(*cp);

	if ((cp = strstr(content, "text/html;")) != NULL &&
	    (cp1 = strstr(content, "charset")) != NULL &&
	    cp1 > cp) {
			BOOL chartrans_ok = NO;
			char *cp3 = NULL, *cp4;
			int chndl;

	    cp1 += 7;
			while (*cp1 == ' ' || *cp1 == '=' || *cp1 == '"')
	        cp1++;
#ifdef EXP_CHARTRANS
			    StrAllocCopy(cp3, cp1); /* copy to mutilate more */
			    for (cp4=cp3; (*cp4 != '\0' && *cp4 != '"' &&
					   *cp4 != ';'  && *cp4 != ':' &&
					   !WHITE(*cp4));	cp4++)
				/* nothing */ ;
			    *cp4 = '\0';
			    cp4 = cp3;
			    chndl = UCGetLYhndl_byMIME(cp3);
			    if (chndl < 0) {
				if (0==strcmp(cp4, "cn-big5")) {
				    cp4 += 3;
				    chndl = UCGetLYhndl_byMIME(cp4);
				}
				else if (0==strncmp(cp4, "cn-gb", 5)) {
				    StrAllocCopy(cp3, "gb2312");
				    cp4 = cp3;
				    chndl = UCGetLYhndl_byMIME(cp4);
				}
			    }
			    if (UCCanTranslateFromTo(chndl, current_char_set))
			    {
				chartrans_ok = YES;
				StrAllocCopy(me->node_anchor->charset, cp4);
				HTAnchor_setUCInfoStage(me->node_anchor, chndl,
				   UCT_STAGE_PARSER, UCT_SETBY_STRUCTURED);
			    }
			    else if (chndl < 0)	{/* got something but we don't
						 recognize it */
				chndl = UCLYhndl_for_unrec;
				if (UCCanTranslateFromTo(chndl,
							 current_char_set))
				{
				    chartrans_ok = YES;
				    HTAnchor_setUCInfoStage(me->node_anchor,
							    chndl,
				       UCT_STAGE_PARSER, UCT_SETBY_STRUCTURED);
				}
			    }
			    FREE(cp3);
			    if (chartrans_ok) {
				LYUCcharset * p_in =
				    HTAnchor_getUCInfoStage(me->node_anchor,
							     UCT_STAGE_PARSER);
				LYUCcharset * p_out =
				    HTAnchor_setUCInfoStage(me->node_anchor,
							    current_char_set,
					 UCT_STAGE_HTEXT, UCT_SETBY_DEFAULT);
				if (!p_out) /* try again */
				    p_out =
				      HTAnchor_getUCInfoStage(me->node_anchor,
							     UCT_STAGE_HTEXT);
				if (0==strcmp(p_in->MIMEname,"x-transparent"))
				{
				    HTPassEightBitRaw = TRUE;
				    HTAnchor_setUCInfoStage(me->node_anchor,
				       HTAnchor_getUCLYhndl(me->node_anchor,
							    UCT_STAGE_HTEXT),
				       UCT_STAGE_PARSER, UCT_SETBY_DEFAULT);
				}
				if (0==strcmp(p_out->MIMEname,"x-transparent"))
				{
				    HTPassEightBitRaw = TRUE;
				    HTAnchor_setUCInfoStage(me->node_anchor,
				       HTAnchor_getUCLYhndl(me->node_anchor,
							    UCT_STAGE_PARSER),
				       UCT_STAGE_HTEXT, UCT_SETBY_DEFAULT);
				}
				if (!(p_in->enc & UCT_ENC_CJK) &&
				    (p_in->codepoints & UCT_CP_SUBSETOF_LAT1)){
				    HTCJK = NOCJK;
				} else if (chndl == current_char_set) {
				HTPassEightBitRaw = TRUE;
				}
				html_get_chartrans_info(me);
			} else  /* Fall through to old behavior */
#endif /* EXP_CHARTRANS */
	    if (!strncmp(cp1, "us-ascii", 8) ||
		!strncmp(cp1, "iso-8859-1", 10)) {
		StrAllocCopy(me->node_anchor->charset, "iso-8859-1");
		HTCJK = NOCJK;

	    } else if (!strncmp(cp1, "iso-8859-2", 10) &&
		       !strncmp(LYchar_set_names[current_char_set],
				"ISO Latin 2", 11)) {
		StrAllocCopy(me->node_anchor->charset, "iso-8859-2");
		HTPassEightBitRaw = TRUE;

	    } else if (!strncmp(cp1, "iso-8859-", 9) &&
		       !strncmp(LYchar_set_names[current_char_set],
				"Other ISO Latin", 15)) {
		/*
		 *  Hope it's a match, for now. - FM
		 */
		StrAllocCopy(me->node_anchor->charset, "iso-8859- ");
		me->node_anchor->charset[9] = cp1[9];
		HTPassEightBitRaw = TRUE;
		HTAlert(me->node_anchor->charset);

	    } else if (!strncmp(cp1, "koi8-r", 6) &&
		       !strncmp(LYchar_set_names[current_char_set],
				"KOI8-R character set", 20)) {
		StrAllocCopy(me->node_anchor->charset, "koi8-r");
		HTPassEightBitRaw = TRUE;

	    } else if (!strncmp(cp1, "euc-jp", 6) && HTCJK == JAPANESE) {
		StrAllocCopy(me->node_anchor->charset, "euc-jp");

	    } else if (!strncmp(cp1, "shift_jis", 9) && HTCJK == JAPANESE) {
		StrAllocCopy(me->node_anchor->charset, "shift_jis");

	    } else if (!strncmp(cp1, "iso-2022-jp", 11) &&
	    			HTCJK == JAPANESE) {
		StrAllocCopy(me->node_anchor->charset, "iso-2022-jp");

	    } else if (!strncmp(cp1, "iso-2022-jp-2", 13) &&
	    			HTCJK == JAPANESE) {
		StrAllocCopy(me->node_anchor->charset, "iso-2022-jp-2");

	    } else if (!strncmp(cp1, "euc-kr", 6) && HTCJK == KOREAN) {
		StrAllocCopy(me->node_anchor->charset, "euc-kr");

	    } else if (!strncmp(cp1, "iso-2022-kr", 11) && HTCJK == KOREAN) {
		StrAllocCopy(me->node_anchor->charset, "iso-2022-kr");

	    } else if ((!strncmp(cp1, "big5", 4) ||
			!strncmp(cp1, "cn-big5", 7)) &&
		       HTCJK == TAIPEI) {
		StrAllocCopy(me->node_anchor->charset, "big5");

	    } else if (!strncmp(cp1, "euc-cn", 6) && HTCJK == CHINESE) {
		StrAllocCopy(me->node_anchor->charset, "euc-cn");

	    } else if ((!strncmp(cp1, "gb2312", 6) ||
			!strncmp(cp1, "cn-gb", 5)) &&
		       HTCJK == CHINESE) {
		StrAllocCopy(me->node_anchor->charset, "gb2312");

	    } else if (!strncmp(cp1, "iso-2022-cn", 11) && HTCJK == CHINESE) {
		StrAllocCopy(me->node_anchor->charset, "iso-2022-cn");
	    }

	    if (TRACE && me->node_anchor->charset) {
		fprintf(stderr,
			"HTML: New charset: %s\n",
			me->node_anchor->charset);
	    }
	}
	/*
	 *  Set the kcode element based on the charset. - FM
	 */
	HText_setKcode(me->text, me->node_anchor->charset);

    /*
     *  Check for a Refresh directive. - FM
     */
    } else if (!strcasecomp((http_equiv ? http_equiv : name), "Refresh")) {
	char *Seconds = NULL;

	/*
	 *  Look for the Seconds field. - FM
	 */
	cp = content;
	while (*cp && isspace((unsigned char)*cp))
	    cp++;
	if (*cp && isdigit(*cp)) {
	    cp1 = cp;
	    while (*cp1 && isdigit(*cp1))
		cp1++;
	    *cp1 = '\0';
	    StrAllocCopy(Seconds, cp);
	    cp1++;
	}
	if (Seconds) {
	    /*
	     *  We have the seconds field.
	     *  Now look for a URL field - FM
	     */
	    while (*cp1) {
		if (!strncasecomp(cp1, "URL", 3)) {
		    cp = (cp1 + 3);
		    while (*cp && (*cp == '=' || isspace((unsigned char)*cp)))
			cp++;
		    cp1 = cp;
		    while (*cp1 && !isspace((unsigned char)*cp1))
			cp1++;
		    *cp1 = '\0';
		    if (*cp)
			StrAllocCopy(href, cp);
		    break;
		}
		cp1++;
	    }
	    if (href) {
		/*
		 *  We found a URL field, so check it out. - FM
		 */
		if (!(url_type = LYLegitimizeHREF(me, (char**)&href, TRUE))) {
		    /*
		     *  The specs require a complete URL,
		     *  but this is a Netscapism, so don't
		     *  expect the author to know that. - FM
		     */
		    HTAlert(REFRESH_URL_NOT_ABSOLUTE);
		    /*
		     *  Use the document's address
		     *  as the base. - FM
		     */
		    if (*href != '\0') {
			temp = HTParse(href,
				       me->node_anchor->address, PARSE_ALL);
			StrAllocCopy(href, temp);
			FREE(temp);
		    } else {
			StrAllocCopy(href, me->node_anchor->address);
			HText_setNoCache(me->text);
		    }
		}
		/*
		 *  Check whether to fill in localhost. - FM
		 */
		LYFillLocalFileURL((char **)&href,
				   (me->inBASE ?
				 me->base_href : me->node_anchor->address));
		/*
		 *  Set the no_cache flag if the Refresh URL
		 *  is the same as the document's address. - FM
		 */
		if (!strcmp(href, me->node_anchor->address)) {
		    HText_setNoCache(me->text);
		} 
	    } else {
		/*
		 *  We didn't find a URL field, so use
		 *  the document's own address and set
		 *  the no_cache flag. - FM
		 */
		StrAllocCopy(href, me->node_anchor->address);
		HText_setNoCache(me->text);
	    }
	    /*
	     *  Check for an anchor in http or https URLs. - FM
	     */
	    if ((strncmp(href, "http", 4) == 0) &&
		(cp = strrchr(href, '#')) != NULL) {
		StrAllocCopy(id_string, cp);
		*cp = '\0';
	    }
	    me->CurrentA = HTAnchor_findChildAndLink(
				me->node_anchor,	/* Parent */
				id_string,		/* Tag */
				href,			/* Addresss */
				(void *)0);		/* Type */
	    if (id_string)
		*cp = '#';
	    FREE(id_string);
	    LYEnsureSingleSpace(me);
	    if (me->inUnderline == FALSE)
		HText_appendCharacter(me->text, LY_UNDERLINE_START_CHAR);
	    HTML_put_string(me, "REFRESH(");
	    HTML_put_string(me, Seconds);
	    HTML_put_string(me, " sec):");
	    FREE(Seconds);
	    if (me->inUnderline == FALSE)
		HText_appendCharacter(me->text, LY_UNDERLINE_END_CHAR);
	    HTML_put_character(me, ' ');
	    me->in_word = NO;
	    HText_beginAnchor(me->text, me->CurrentA);
	    if (me->inBoldH == FALSE)
		HText_appendCharacter(me->text, LY_BOLD_START_CHAR);
	    HTML_put_string(me, href);
	    FREE(href);
	    if (me->inBoldH == FALSE)
		HText_appendCharacter(me->text, LY_BOLD_END_CHAR);
	    HText_endAnchor(me->text);
	    LYEnsureSingleSpace(me);
	}

    /*
     *  Check for a suggested filename via a Content-Disposition with
     *  file; filename=name.suffix in it, if we don't already have it
     *  via a server header. - FM
     */
    } else if (!(me->node_anchor->SugFname && *me->node_anchor->SugFname) &&
    	       !strcasecomp((http_equiv ?
    			     http_equiv : name), "Content-Disposition")) {
	cp = content;
	while (*cp != '\0' && strncasecomp(cp, "file;", 5))
	    cp++;
	if (*cp != '\0') {
	    cp += 5;
	    while (*cp != '\0' && WHITE(*cp))
	        cp++;
	    if (*cp != '\0') {
	        while (*cp != '\0' && strncasecomp(cp, "filename=", 9))
		    cp++;
		if (*cp != '\0') {
		    StrAllocCopy(me->node_anchor->SugFname, (cp + 9));
		    cp = me->node_anchor->SugFname;
		    while (*cp != '\0' && !WHITE(*cp))
			cp++;
		    *cp = '\0';
		    if (*me->node_anchor->SugFname == '\0')
		        FREE(me->node_anchor->SugFname);
		}
	    }
	}
    /*
     *  Check for a Set-Cookie directive. - AK
     */
    } else if (!strcasecomp((http_equiv ? http_equiv : name), "Set-Cookie")) {
	/*
	 *  We're using the Request-URI as the second argument,
	 *  regardless of whether a Content-Base header or BASE
	 *  tag are present.
	 */
	LYSetCookie(content, me->node_anchor->address);
    }

    /*
     *  Free the copies. - FM
     */
free_META_copies:
    FREE(http_equiv);
    FREE(name);
    FREE(content);
}

/*
**  This function strips white characters and
**  generally fixes up attribute values that
**  were received from the SGML parser and
**  are to be treated as partial or absolute
**  URLs. - FM
*/
PUBLIC int LYLegitimizeHREF ARGS3(
	HTStructured *, 	me,
	char **,		href,
	BOOL,			force_slash)
{
    int url_type = 0;

    if (!me || !href || *href == NULL || *(*href) == '\0')
        return(url_type);

    LYTrimHead(*href);
    if (!strncasecomp(*href, "lynxexec:", 9) ||
        !strncasecomp(*href, "lynxprog:", 9)) {
	/*
	 *  The original implementions of these schemes expected
	 *  white space without hex escaping, and did not check
	 *  for hex escaping, so we'll continue to support that,
	 *  until that code is redone in conformance with SGML
	 *  principles.  - FM
	 */
	HTUnEscapeSome(*href, " \r\n\t");
	convert_to_spaces(*href, TRUE);
    } else {
        collapse_spaces(*href);
    }
    if (*(*href) == '\0')
        return(url_type);
    LYUnEscapeToLatinOne(&(*href), TRUE);
    url_type = is_url(*href);
    if (!url_type && force_slash &&
	(!strcmp(*href, ".") || !strcmp(*href, "..")) &&
	 strncmp((me->inBASE ?
	       me->base_href : me->node_anchor->address),
		 "file:", 5)) {
        /*
	 *  The Fielding RFC/ID for resolving partial HREFs says
	 *  that a slash should be on the end or the preceding
	 *  symbolic element for "." and "..", but all tested
	 *  browsers only do that for an explicit "./" or "../",
	 *  so we'll respect the RFC/ID only if force_slash was
	 *  TRUE and it's not a file URL. - FM
	 */
	StrAllocCat(*href, "/");
    }
    return(url_type); 
}

/*
**  This function checks for a Content-Base header,
**  and if not present, a Content-Location header
**  which is an absolute URL, and sets the BASE
**  accordingly.  If set, it will be replaced by
**  any BASE tag in the HTML stream, itself. - FM
*/
PUBLIC void LYCheckForContentBase ARGS1(
	HTStructured *,		me)
{
    char *cp = NULL;
    BOOL present[HTML_BASE_ATTRIBUTES];
    CONST char *value[HTML_BASE_ATTRIBUTES];
    int i;

    if (!(me && me->node_anchor))
        return;

    if (me->node_anchor->content_base != NULL) {
        /*
	 *  We have a Content-Base value.  Use it
	 *  if it's non-zero length. - FM
	 */
        if (*me->node_anchor->content_base == '\0')
	    return;
	StrAllocCopy(cp, me->node_anchor->content_base);
	collapse_spaces(cp);
    } else if (me->node_anchor->content_location != NULL) {
        /*
	 *  We didn't have a Content-Base value, but do
	 *  have a Content-Location value.  Use it if
	 *  it's an absolute URL. - FM
	 */
        if (*me->node_anchor->content_location == '\0')
	    return;
	StrAllocCopy(cp, me->node_anchor->content_location);
	collapse_spaces(cp);
	if (!is_url(cp)) {
	    FREE(cp);
	    return;
	}
    } else {
        /*
	 *  We had neither a Content-Base nor
	 *  Content-Location value. - FM
	 */
        return;
    }

    /*
     *  If we collapsed to a zero-length value,
     *  ignore it. - FM
     */
    if (*cp == '\0') {
        FREE(cp);
	return;
    }

    /*
     *  Pass the value to HTML_start_element as
     *  the HREF of a BASE tag. - FM
     */
    for (i = 0; i < HTML_BASE_ATTRIBUTES; i++)
	 present[i] = NO;
    present[HTML_BASE_HREF] = YES;
    value[HTML_BASE_HREF] = (CONST char *)cp;
    (*me->isa->start_element)(me, HTML_BASE, present, value, 0);
    FREE(cp);
}

/*
**  This function creates NAMEd Anchors if a non-zero-length NAME
**  or ID attribute was present in the tag. - FM
*/
PUBLIC void LYCheckForID ARGS4(
	HTStructured *,		me,
	CONST BOOL *,		present,
	CONST char **,		value,
	int,			attribute)
{
    HTChildAnchor *ID_A = NULL;
    char *temp = NULL;

    if (!(me && me->text))
        return;

    if (present && present[attribute]
	&& value[attribute] && *value[attribute]) {
	/*
	 *  Translate any named or numeric character references. - FM
	 */
	StrAllocCopy(temp, value[attribute]);
	LYUnEscapeToLatinOne(&temp, TRUE);

	/*
	 *  Create the link if we still have a non-zero-length string. - FM
	 */
	if ((temp[0] != '\0') &&
	    (ID_A = HTAnchor_findChildAndLink(
				me->node_anchor,	/* Parent */
				temp,			/* Tag */
				NULL,			/* Addresss */
				(void *)0))) {		/* Type */
	    HText_beginAnchor(me->text, ID_A);
	    HText_endAnchor(me->text);
	}
	FREE(temp);
    }
}

/*
**  This function creates a NAMEd Anchor for the ID string
**  passed to it directly as an argument.  It assumes the
**  does not need checking for character references. - FM
*/
PUBLIC void LYHandleID ARGS2(
	HTStructured *,		me,
	char *,			id)
{
    HTChildAnchor *ID_A = NULL;

    if (!(me && me->text) ||
        !(id && *id))
        return;

    /*
     *  Create the link if we still have a non-zero-length string. - FM
     */
    if ((ID_A = HTAnchor_findChildAndLink(
				me->node_anchor,	/* Parent */
				id,			/* Tag */
				NULL,			/* Addresss */
				(void *)0)) != 0) {	/* Type */
	HText_beginAnchor(me->text, ID_A);
	HText_endAnchor(me->text);
    }
}

/*
**  This function checks whether we want to overrride
**  the current default alignment for parargraphs and
**  instead use that specified in the element's style
**  sheet. - FM
*/
PUBLIC BOOLEAN LYoverride_default_alignment ARGS1(
	HTStructured *, me)
{
    if (!me)
        return NO;

    switch(me->sp[0].tag_number) {
	case HTML_BLOCKQUOTE:
	case HTML_BQ:
	case HTML_NOTE:
	case HTML_FN:
        case HTML_ADDRESS:
	    me->sp->style->alignment = HT_LEFT;
	    return YES;
	    break;

	default:
	    break;
    }
    return NO;
}

/*
**  This function inserts newlines if needed to create double spacing,
**  and sets the left margin for subsequent text to the second line
**  indentation of the current style. - FM
*/
PUBLIC void LYEnsureDoubleSpace ARGS1(
	HTStructured *, me)
{
    if (!me || !me->text)
        return;

    if (HText_LastLineSize(me->text)) {
	HText_appendCharacter(me->text, '\r');
	HText_appendCharacter(me->text, '\r');
    } else if (HText_PreviousLineSize(me->text)) {
	HText_appendCharacter(me->text, '\r');
    } else if (me->List_Nesting_Level >= 0) {
	HText_NegateLineOne(me->text);
    }
    me->in_word = NO;
    return;
}

/*
**  This function inserts a newline if needed to create single spacing,
**  and sets the left margin for subsequent text to the second line
**  indentation of the current style. - FM
*/
PUBLIC void LYEnsureSingleSpace ARGS1(
	HTStructured *, me)
{
    if (!me || !me->text)
        return;

    if (HText_LastLineSize(me->text)) {
	HText_appendCharacter(me->text, '\r');
    } else if (me->List_Nesting_Level >= 0) {
	HText_NegateLineOne(me->text);
    }
    me->in_word = NO;
    return;
}

/*
**  This function resets paragraph alignments for block
**  elements which do not have a defined style sheet. - FM
*/
PUBLIC void LYResetParagraphAlignment ARGS1(
	HTStructured *, me)
{
    if (!me)
        return;

    if (me->List_Nesting_Level >= 0 ||
	((me->Division_Level < 0) &&
	 (!strcmp(me->sp->style->name, "Normal") ||
	  !strcmp(me->sp->style->name, "Preformatted")))) {
	me->sp->style->alignment = HT_LEFT;
    } else {
	me->sp->style->alignment = me->current_default_alignment;
    }
    return;
}

PUBLIC BOOLEAN LYCheckForCSI ARGS2(
	HTStructured *, 	me,
	char **,		url)
{
    if (!(me && me->node_anchor && me->node_anchor->address))
        return FALSE;

    if (strncasecomp(me->node_anchor->address, "file:", 5))
        return FALSE;

    if (!LYisLocalHost(me->node_anchor->address))
        return FALSE;
     
    StrAllocCopy(*url, me->node_anchor->address);
    return TRUE;
}
