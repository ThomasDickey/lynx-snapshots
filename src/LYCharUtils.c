/*
** Functions associated with LYCharSets.c and the Lynx version of HTML.c - FM
** ==========================================================================
**
** These functions should be prototyped in the Lynx version of HTML.c.
*/
#include "HTUtils.h"
#include "tcp.h"

#include "HTML.h"
#include "HTFont.h"
#include "HTCJK.h"
#include "HTParse.h"

#include "LYGlobalDefs.h"
#include "LYCharSets.h"
#include "LYCharUtils.h"
#include "LYUtils.h"
#include "GridText.h"

#include "LYexit.h"
#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

extern BOOL HTPassEightBitRaw;
extern BOOL HTPassEightBitNum;
extern BOOL HTPassHighCtrlRaw;
extern BOOL HTPassHighCtrlNum;
extern HTkcode kanji_code;
extern HTCJKlang HTCJK;


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
		**  a hidden INPUT, otherwise use 174 (reg). - FM
		*/
	        } else if (value == 8482) {
		    if (hidden) {
		        *q++ = '&';
		        *q++ = '#';
			if (cpe != '\0')
			    *(p-1) = cpe;
			p = cp;
			continue;
		    } else {
			if (cpe != '\0') {
			    p--;
			    *p = cpe;
			}
			cp = (p - 5);
			*cp++ = '&';
			*cp++ = '#';
			*cp++ = '1';
			*cp++ = '7';
			*cp++ = '4';
			p -= 5;
			continue;
		    }
		/*
		**  If it's ASCII, or is 8-bit but HTPassEightBitNum
		**  is set or the character set is "ISO Latin 1",
		**  use it's value. - FM
		*/
		} else if (value < 161 || HTPassEightBitNum ||
			   !strncmp(LYchar_set_names[current_char_set],
			   	    "ISO Latin 1", 11)) {
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
		    value -= 160;
		    name = HTMLGetEntityName(value);
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
		        if ((unsigned char)buf[0] > 159 && isURL == TRUE) {
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
    return;
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
	    HTPassHighCtrlRaw) {
	    p[i] = HT_NON_BREAK_SPACE;
	/*
	**  Substitute Lynx special character for
	**  173 (shy) if HTPassHighCtrlRaw is not
	**  set. - FM
	*/
        } else if (((unsigned char)p[i]) == 173 &&
	    HTPassHighCtrlRaw) {
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
    return;
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

    return;
}

/*
** This function trims characters <= that of a space (32),
** including HT_NON_BREAK_SPACE (1) and HT_EM_SPACE (2),
** but not ESC, from the tails of strings. - FM
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
    
    return;
}

/*
** This function trims characters <= that of a space (32),
** including HT_NON_BREAK_SPACE (1), HT_EM_SPACE (2), and
** ESC from the tails of strings. - FM
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

    return;
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

