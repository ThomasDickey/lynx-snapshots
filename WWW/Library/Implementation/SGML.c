/*
 * $LynxId: SGML.c,v 1.188 2024/04/11 20:22:19 tom Exp $
 *
 *			General SGML Parser code		SGML.c
 *			========================
 *
 *	This module implements an HTStream object.  To parse an
 *	SGML file, create this object which is a parser.  The object
 *	is (currently) created by being passed a DTD structure,
 *	and a target HTStructured object at which to throw the parsed stuff.
 *
 *	 6 Feb 93  Binary searches used. Interface modified.
 */

#define HTSTREAM_INTERNAL 1

#include <HTUtils.h>

#include <SGML.h>
#include <HTMLDTD.h>
#include <HTAccess.h>
#include <UCAux.h>

#include <HTChunk.h>
#include <HTUtils.h>

#include <LYCharSets.h>
#include <LYCharVals.h>		/* S/390 -- gil -- 0635 */
#include <LYGlobalDefs.h>
#include <LYStrings.h>
#include <LYLeaks.h>
#include <LYUtils.h>

#ifdef USE_COLOR_STYLE
# include <LYStyle.h>
#endif
#ifdef USE_PRETTYSRC
# include <LYPrettySrc.h>
#endif

/* a global variable doesn't work with info-stages which convert encoding */
#if defined(EXP_CHINESEUTF8_SUPPORT)
#undef IS_CJK_TTY
#define IS_CJK_TTY me->T.do_cjk
#endif

#define AssumeCP1252(me) \
	(((me)->inUCLYhndl == LATIN1 \
	  || (me)->inUCLYhndl == US_ASCII) \
	 && html5_charsets)

#define INVALID (-1)

static int sgml_offset;

#ifdef USE_PRETTYSRC

static char *entity_string;	/* this is used for printing entity name.

				   Unconditionally added since redundant assignments don't hurt much */

static void fake_put_character(HTStream *p GCC_UNUSED,
			       int c GCC_UNUSED)
{
}

#define START TRUE
#define STOP FALSE

#define PUTS_TR(x) psrc_convert_string = TRUE; PUTS(x)

#endif

/* my_casecomp() - optimized by the first character, NOT_ASCII ok */
#define my_casecomp(a,b)  ((TOUPPER(*a) == TOUPPER(*b)) ? \
			AS_casecomp(a,b) : \
			(TOASCII(TOUPPER(*a)) - TOASCII(TOUPPER(*b))))

/* ...used for comments and attributes value like href... */
#define HTChunkPutUtf8Char(ch,x) \
    { \
    if ((TOASCII(x) < 128)  && (ch->size < ch->allocated)) \
	ch->data[ch->size++] = (char)x; \
    else \
	(HTChunkPutUtf8Char)(ch,x); \
    }

#define PUTS(str) ((*me->actions->put_string)(me->target, str))
#define PUTC(ch)  ((*me->actions->put_character)(me->target, (char) ch))
#define PUTUTF8(code) (UCPutUtf8_charstring((HTStream *)me->target, \
		      (putc_func_t*)(me->actions->put_character), code))

#ifdef USE_PRETTYSRC
#define PRETTYSRC_PUTC(c) if (psrc_view) PUTC(c)
#else
#define PRETTYSRC_PUTC(c)	/* nothing */
#endif

/*the following macros are used for pretty source view. */
#define IS_C(attr) (attr.type == HTMLA_CLASS)

#if defined(USE_JAPANESEUTF8_SUPPORT)
# define UTF8_TTY_ISO2022JP (me->T.output_utf8)
#else
# define UTF8_TTY_ISO2022JP 0
#endif

HTCJKlang HTCJK = NOCJK;	/* CJK enum value.              */
BOOL HTPassEightBitRaw = FALSE;	/* Pass 161-172,174-255 raw.    */
BOOL HTPassEightBitNum = FALSE;	/* Pass ^ numeric entities raw. */
BOOL HTPassHighCtrlRaw = FALSE;	/* Pass 127-160,173,&#127; raw. */
BOOL HTPassHighCtrlNum = FALSE;	/* Pass &#128;-&#159; raw.      */

/*	The State (context) of the parser
 *
 *	This is passed with each call to make the parser reentrant
 */

#define MAX_ATTRIBUTES 36	/* Max number of attributes per element */

/*		Element Stack
 *		-------------
 *	This allows us to return down the stack reselecting styles.
 *	As we return, attribute values will be garbage in general.
 */
typedef struct _HTElement HTElement;
struct _HTElement {
    HTElement *next;		/* Previously nested element or 0 */
    HTTag *tag;			/* The tag at this level  */
};

typedef enum {
    S_text = 0
    ,S_attr
    ,S_attr_gap
    ,S_comment
    ,S_cro
    ,S_doctype
    ,S_dollar
    ,S_dollar_dq
    ,S_dollar_paren
    ,S_dollar_paren_dq
    ,S_dollar_paren_sq
    ,S_dollar_sq
    ,S_dquoted
    ,S_end
    ,S_entity
    ,S_equals
    ,S_ero
    ,S_esc
    ,S_esc_dq
    ,S_esc_sq
    ,S_exclamation
    ,S_in_kanji
    ,S_incro
    ,S_junk_tag
    ,S_litteral
    ,S_marked
    ,S_nonascii_text
    ,S_nonascii_text_dq
    ,S_nonascii_text_sq
    ,S_paren
    ,S_paren_dq
    ,S_paren_sq
    ,S_pcdata
    ,S_pi
    ,S_script
    ,S_sgmlatt
    ,S_sgmlele
    ,S_sgmlent
    ,S_squoted
    ,S_tag
    ,S_tag_gap
    ,S_tagname_slash
    ,S_value
} sgml_state;

/*	Internal Context Data Structure
 *	-------------------------------
 */
struct _HTStream {

    const HTStreamClass *isa;	/* inherited from HTStream */

    const SGML_dtd *dtd;
    const HTStructuredClass *actions;	/* target class  */
    HTStructured *target;	/* target object */

    HTTag *current_tag;
    HTTag *slashedtag;
    const HTTag *unknown_tag;
    BOOL extended_html;		/* xhtml */
    BOOL strict_xml;		/* xml */
    BOOL inSELECT;
    BOOL no_lynx_specialcodes;
    int current_attribute_number;
    HTChunk *string;
    int leading_spaces;
    int trailing_spaces;
    HTElement *element_stack;
    sgml_state state;
    unsigned char kanji_buf;
#ifdef CALLERDATA
    void *callerData;
#endif				/* CALLERDATA */
    BOOL present[MAX_ATTRIBUTES];	/* Flags: attribute is present? */
    char *value[MAX_ATTRIBUTES];	/* NULL, or strings alloc'd with StrAllocCopy_extra() */

    BOOL lead_exclamation;
    BOOL first_dash;
    BOOL end_comment;
    BOOL doctype_bracket;
    BOOL first_bracket;
    BOOL second_bracket;
    BOOL isHex;

    HTParentAnchor *node_anchor;
    LYUCcharset *inUCI;		/* pointer to anchor UCInfo */
    int inUCLYhndl;		/* charset we are fed       */
    LYUCcharset *outUCI;	/* anchor UCInfo for target */
    int outUCLYhndl;		/* charset for target       */
    UTFDecodeState U;
    UCTransParams T;
    int current_tag_charset;	/* charset to pass attributes */

    char *recover;
    int recover_index;
    char *include;
    char *active_include;
    int include_index;
    char *url;
    char *csi;
    int csi_index;
#ifdef USE_PRETTYSRC
    BOOL cur_attr_is_href;
    BOOL cur_attr_is_name;
#endif
};

#ifdef NO_LYNX_TRACE
#define state_name(n) "state"
#else
static const char *state_name(sgml_state n)
{
    const char *result;
    /* *INDENT-OFF* */
    switch (n) {
    default:
    case S_attr:                result = "S_attr";              break;
    case S_attr_gap:            result = "S_attr_gap";          break;
    case S_comment:             result = "S_comment";           break;
    case S_cro:                 result = "S_cro";               break;
    case S_doctype:             result = "S_doctype";           break;
    case S_dollar:              result = "S_dollar";            break;
    case S_dollar_dq:           result = "S_dollar_dq";         break;
    case S_dollar_paren:        result = "S_dollar_paren";      break;
    case S_dollar_paren_dq:     result = "S_dollar_paren_dq";   break;
    case S_dollar_paren_sq:     result = "S_dollar_paren_sq";   break;
    case S_dollar_sq:           result = "S_dollar_sq";         break;
    case S_dquoted:             result = "S_dquoted";           break;
    case S_end:                 result = "S_end";               break;
    case S_entity:              result = "S_entity";            break;
    case S_equals:              result = "S_equals";            break;
    case S_ero:                 result = "S_ero";               break;
    case S_esc:                 result = "S_esc";               break;
    case S_esc_dq:              result = "S_esc_dq";            break;
    case S_esc_sq:              result = "S_esc_sq";            break;
    case S_exclamation:         result = "S_exclamation";       break;
    case S_in_kanji:            result = "S_in_kanji";          break;
    case S_incro:               result = "S_incro";             break;
    case S_pi:                  result = "S_pi";                break;
    case S_junk_tag:            result = "S_junk_tag";          break;
    case S_litteral:            result = "S_litteral";          break;
    case S_marked:              result = "S_marked";            break;
    case S_nonascii_text:       result = "S_nonascii_text";     break;
    case S_nonascii_text_dq:    result = "S_nonascii_text_dq";  break;
    case S_nonascii_text_sq:    result = "S_nonascii_text_sq";  break;
    case S_paren:               result = "S_paren";             break;
    case S_paren_dq:            result = "S_paren_dq";          break;
    case S_paren_sq:            result = "S_paren_sq";          break;
    case S_pcdata:              result = "S_pcdata";            break;
    case S_script:              result = "S_script";            break;
    case S_sgmlatt:             result = "S_sgmlatt";           break;
    case S_sgmlele:             result = "S_sgmlele";           break;
    case S_sgmlent:             result = "S_sgmlent";           break;
    case S_squoted:             result = "S_squoted";           break;
    case S_tag:                 result = "S_tag";               break;
    case S_tag_gap:             result = "S_tag_gap";           break;
    case S_tagname_slash:       result = "S_tagname_slash";     break;
    case S_text:                result = "S_text";              break;
    case S_value:               result = "S_value";             break;
    }
    /* *INDENT-ON* */

    return result;
}
#endif

/* storage for Element Stack */
#define DEPTH 10
static HTElement pool[DEPTH];
static int depth = 0;

static HTElement *pool_alloc(void)
{
    depth++;
    if (depth > DEPTH)
	return (HTElement *) malloc(sizeof(HTElement));
    return (pool + depth - 1);
}

static void pool_free(HTElement * e)
{
    if (depth > DEPTH)
	FREE(e);
    depth--;
    return;
}

#ifdef USE_PRETTYSRC

static void HTMLSRC_apply_markup(HTStream *me,
				 HTlexeme lexeme,
				 int start)
{
    HT_tagspec *ts = *((start ? lexeme_start : lexeme_end) + lexeme);

    while (ts) {
#ifdef USE_COLOR_STYLE
	if (ts->start) {
	    current_tag_style = ts->style;
	    force_current_tag_style = TRUE;
	    forced_classname = ts->class_name;
	    force_classname = TRUE;
	}
#endif
	CTRACE((tfp, ts->start ? "SRCSTART %d\n" : "SRCSTOP %d\n", (int) lexeme));
	if (ts->start)
	    (*me->actions->start_element) (me->target,
					   (int) ts->element,
					   ts->present,
					   (STRING2PTR) ts->value,
					   me->current_tag_charset,
					   &me->include);
	else
	    (*me->actions->end_element) (me->target,
					 (int) ts->element,
					 &me->include);
	ts = ts->next;
    }
}

#define PSRCSTART(x)	HTMLSRC_apply_markup(me,HTL_##x,START)
#define PSRCSTOP(x)   HTMLSRC_apply_markup(me,HTL_##x,STOP)

#define attr_is_href me->cur_attr_is_href
#define attr_is_name me->cur_attr_is_name
#endif

static void set_chartrans_handling(HTStream *me,
				   HTParentAnchor *anchor,
				   int chndl)
{
    if (chndl < 0) {
	/*
	 * Nothing was set for the parser in earlier stages, so the HTML
	 * parser's UCLYhndl should still be its default.  - FM
	 */
	chndl = HTAnchor_getUCLYhndl(anchor, UCT_STAGE_STRUCTURED);
	if (chndl < 0)
	    /*
	     * That wasn't set either, so seek the HText default.  - FM
	     */
	    chndl = HTAnchor_getUCLYhndl(anchor, UCT_STAGE_HTEXT);
	if (chndl < 0)
	    /*
	     * That wasn't set either, so assume the current display character
	     * set.  - FM
	     */
	    chndl = current_char_set;
	/*
	 * Try to set the HText and HTML stages' chartrans info with the
	 * default lock level (will not be changed if it was set previously
	 * with a higher lock level).  - FM
	 */
	HTAnchor_setUCInfoStage(anchor, chndl,
				UCT_STAGE_HTEXT,
				UCT_SETBY_DEFAULT);
	HTAnchor_setUCInfoStage(anchor, chndl,
				UCT_STAGE_STRUCTURED,
				UCT_SETBY_DEFAULT);
	/*
	 * Get the chartrans info for output to the HTML parser.  - FM
	 */
	me->outUCI = HTAnchor_getUCInfoStage(anchor,
					     UCT_STAGE_STRUCTURED);
	me->outUCLYhndl = HTAnchor_getUCLYhndl(me->node_anchor,
					       UCT_STAGE_STRUCTURED);
    }
    /*
     * Set the in->out transformation parameters.  - FM
     */
    UCSetTransParams(&me->T,
		     me->inUCLYhndl, me->inUCI,
		     me->outUCLYhndl, me->outUCI);
    /*
     * This is intended for passing the SGML parser's input charset as an
     * argument in each call to the HTML parser's start tag function, but it
     * would be better to call a Lynx_HTML_parser function to set an element in
     * its HTStructured object, itself, if this were needed.  - FM
     */
#ifndef USE_JAPANESEUTF8_SUPPORT
    if (IS_CJK_TTY) {
	me->current_tag_charset = -1;
    } else
#endif
    if (me->T.transp) {
	me->current_tag_charset = me->inUCLYhndl;
    } else if (me->T.decode_utf8) {
	me->current_tag_charset = me->inUCLYhndl;
    } else if (me->T.do_8bitraw ||
	       me->T.use_raw_char_in) {
	me->current_tag_charset = me->inUCLYhndl;
    } else if (me->T.output_utf8 ||
	       me->T.trans_from_uni) {
	me->current_tag_charset = UCGetLYhndl_byMIME("utf-8");
    } else {
	me->current_tag_charset = LATIN1;
    }
}

static void change_chartrans_handling(HTStream *me)
{
    int new_LYhndl = HTAnchor_getUCLYhndl(me->node_anchor,
					  UCT_STAGE_PARSER);

    if (new_LYhndl != me->inUCLYhndl &&
	new_LYhndl >= 0) {
	/*
	 * Something changed.  but ignore if a META wants an unknown charset.
	 */
	LYUCcharset *new_UCI = HTAnchor_getUCInfoStage(me->node_anchor,
						       UCT_STAGE_PARSER);

	if (new_UCI) {
	    LYUCcharset *next_UCI = HTAnchor_getUCInfoStage(me->node_anchor,
							    UCT_STAGE_STRUCTURED);
	    int next_LYhndl = HTAnchor_getUCLYhndl(me->node_anchor, UCT_STAGE_STRUCTURED);

	    me->inUCI = new_UCI;
	    me->inUCLYhndl = new_LYhndl;
	    me->outUCI = next_UCI;
	    me->outUCLYhndl = next_LYhndl;
	    set_chartrans_handling(me,
				   me->node_anchor, next_LYhndl);
	}
    }
}

#ifdef USE_COLOR_STYLE
#include <AttrList.h>
static int current_is_class = 0;
#endif

/*	Handle Attribute
 *	----------------
 */
/* PUBLIC const char * SGML_default = "";   ?? */

static void handle_attribute_name(HTStream *me, const char *s)
{
    HTTag *tag = me->current_tag;
    const attr *attributes = tag->attributes;
    int high, low, i, diff;

#ifdef USE_PRETTYSRC
    if (psrc_view) {
	attr_is_href = FALSE;
	attr_is_name = FALSE;
    }
#endif
    /*
     * Ignore unknown tag.  - KW
     */
    if (tag == me->unknown_tag) {
#ifdef USE_PRETTYSRC
	if (psrc_view)
	    me->current_attribute_number = 1;	/* anything !=INVALID */
#endif
	return;
    }

    /*
     * Binary search for attribute name.
     */
    for (low = 0, high = tag->number_of_attributes;
	 high > low;
	 diff < 0 ? (low = i + 1) : (high = i)) {
	i = (low + (high - low) / 2);
	diff = my_casecomp(attributes[i].name, s);
	if (diff == 0) {	/* success: found it */
	    me->current_attribute_number = i;
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		attr_is_name = (BOOL) (attributes[i].type == HTMLA_ANAME);
		attr_is_href = (BOOL) (attributes[i].type == HTMLA_HREF);
	    } else
#endif
	    {
		me->present[i] = YES;
		Clear_extra(me->value[i]);
#ifdef USE_COLOR_STYLE
#   ifdef USE_PRETTYSRC
		current_is_class = IS_C(attributes[i]);
#   else
		current_is_class = (!strcasecomp("class", s));
#   endif
		CTRACE((tfp, "SGML: found attribute %s, %d\n", s, current_is_class));
#endif
	    }
	    return;
	}
	/* if */
    }				/* for */

    CTRACE((tfp, "SGML: Unknown attribute %s for tag %s\n",
	    s, NonNull(me->current_tag->name)));
    me->current_attribute_number = INVALID;	/* Invalid */
}

/*	Handle attribute value
 *	----------------------
 */
static void handle_attribute_value(HTStream *me, const char *s)
{
    if (me->current_attribute_number != INVALID) {
	StrAllocCopy_extra(me->value[me->current_attribute_number], s);
#ifdef USE_COLOR_STYLE
	if (current_is_class) {
	    StrNCpy(class_string, s, TEMPSTRINGSIZE);
	    CTRACE((tfp, "SGML: class is '%s'\n", s));
	} else {
	    CTRACE((tfp, "SGML: attribute value is '%s'\n", s));
	}
#endif
    } else {
	CTRACE((tfp, "SGML: Attribute value %s ***ignored\n", s));
    }
    me->current_attribute_number = INVALID;	/* can't have two assignments! */
}

/*
 *  Translate some Unicodes to Lynx special codes and output them.
 *  Special codes - ones those output depend on parsing.
 *
 *  Additional issue, like handling bidirectional text if necessary
 *  may be called from here:  zwnj (8204), zwj (8205), lrm (8206), rlm (8207)
 *  - currently they are ignored in SGML.c and LYCharUtils.c
 *  but also in UCdomap.c because they are non printable...
 *
 */
static BOOL put_special_unicodes(HTStream *me, UCode_t code)
{
    /* (Tgf_nolyspcl) */
    if (me->no_lynx_specialcodes) {
	/*
	 * We were asked by a "DTD" flag to not generate lynx specials.  - kw
	 */
	return NO;
    }

    if (code == CH_NBSP) {	/* S/390 -- gil -- 0657 */
	/*
	 * Use Lynx special character for nbsp.
	 */
#ifdef USE_PRETTYSRC
	if (!psrc_view)
#endif
	    PUTC(HT_NON_BREAK_SPACE);
    } else if (code == CH_SHY) {
	/*
	 * Use Lynx special character for shy.
	 */
#ifdef USE_PRETTYSRC
	if (!psrc_view)
#endif
	    PUTC(LY_SOFT_HYPHEN);
    } else if (code == 8194 || code == 8201) {
	/*
	 * Use Lynx special character for ensp or thinsp.
	 *
	 * Originally, Lynx use space '32' as word delimiter and omits this
	 * space at end of line if word is wrapped to the next line.  There are
	 * several other spaces in the Unicode repertoire and we should teach
	 * Lynx to understand them, not only as regular characters but in the
	 * context of line wrapping.  Unfortunately, if we use HT_EN_SPACE we
	 * override the chartrans tables for those spaces with a single '32'
	 * for all (but do line wrapping more fancy).
	 *
	 * We may treat emsp as one or two ensp (below).
	 */
#ifdef USE_PRETTYSRC
	if (!psrc_view)
#endif
	    PUTC(HT_EN_SPACE);
    } else if (code == 8195) {
	/*
	 * Use Lynx special character for emsp.
	 */
#ifdef USE_PRETTYSRC
	if (!psrc_view) {
#endif
	    /* PUTC(HT_EN_SPACE);  let's stay with a single space :) */
	    PUTC(HT_EN_SPACE);
#ifdef USE_PRETTYSRC
	}
#endif
    } else {
	/*
	 * Return NO if nothing done.
	 */
	return NO;
    }
    /*
     * We have handled it.
     */
    return YES;
}

#ifdef USE_PRETTYSRC
static void put_pretty_entity(HTStream *me, int term)
{
    PSRCSTART(entity);
    PUTC('&');
    PUTS(entity_string);
    if (term)
	PUTC((char) term);
    PSRCSTOP(entity);
}

static void put_pretty_number(HTStream *me)
{
    PSRCSTART(entity);
    PUTS((me->isHex ? "&#x" : "&#"));
    PUTS(entity_string);
    PUTC(';');
    PSRCSTOP(entity);
}
#endif /* USE_PRETTYSRC */

/*	Handle entity
 *	-------------
 *
 * On entry,
 *	s	contains the entity name zero terminated
 * Bugs:
 *	If the entity name is unknown, the terminator is treated as
 *	a printable non-special character in all cases, even if it is '<'
 * Bug-fix:
 *	Modified SGML_character() so we only come here with terminator
 *	as '\0' and check a FoundEntity flag. -- Foteos Macrides
 *
 * Modified more (for use with Lynx character translation code):
 */
static char replace_buf[64];	/* buffer for replacement strings */
static BOOL FoundEntity = FALSE;

static void handle_entity(HTStream *me, int term)
{
    UCode_t code;
    long uck = -1;
    const char *s = me->string->data;

    /*
     * Handle all entities normally.  - FM
     */
    FoundEntity = FALSE;
    if ((code = HTMLGetEntityUCValue(s)) != 0) {
	/*
	 * We got a Unicode value for the entity name.  Check for special
	 * Unicodes.  - FM
	 */
	if (put_special_unicodes(me, code)) {
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		put_pretty_entity(me, term);
	    }
#endif
	    FoundEntity = TRUE;
	    return;
	}
	/*
	 * Seek a translation from the chartrans tables.
	 */
	if ((uck = UCTransUniChar(code, me->outUCLYhndl)) >= 32 &&
/* =============== work in ASCII below here ===============  S/390 -- gil -- 0672 */
	    uck < 256 &&
	    (uck < 127 ||
	     uck >= LYlowest_eightbit[me->outUCLYhndl])) {
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		put_pretty_entity(me, term);
	    } else
#endif
		PUTC(FROMASCII((char) uck));
	    FoundEntity = TRUE;
	    return;
	} else if ((uck == -4 ||
		    (me->T.repl_translated_C0 &&
		     uck > 0 && uck < 32)) &&
	    /*
	     * Not found; look for replacement string.
	     */
		   (uck = UCTransUniCharStr(replace_buf, 60, code,
					    me->outUCLYhndl, 0) >= 0)) {
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		put_pretty_entity(me, term);
	    } else
#endif
		PUTS(replace_buf);
	    FoundEntity = TRUE;
	    return;
	}
	/*
	 * If we're displaying UTF-8, try that now.  - FM
	 */
#ifndef USE_PRETTYSRC
	if (me->T.output_utf8 && PUTUTF8(code)) {
	    FoundEntity = TRUE;
	    return;
	}
#else
	if (me->T.output_utf8 && (psrc_view
				  ? (UCPutUtf8_charstring((HTStream *) me->target,
							  (putc_func_t *) (fake_put_character),
							  code))
				  : PUTUTF8(code))) {

	    if (psrc_view) {
		put_pretty_entity(me, term);
	    }

	    FoundEntity = TRUE;
	    return;
	}
#endif
	/*
	 * If it's safe ASCII, use it.  - FM
	 */
	if (code >= 32 && code < 127) {
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		put_pretty_entity(me, term);
	    } else
#endif

		PUTC(FROMASCII((char) code));
	    FoundEntity = TRUE;
	    return;
	}
/* =============== work in ASCII above here ===============  S/390 -- gil -- 0682 */
	/*
	 * Ignore zwnj (8204) and zwj (8205), if we get to here.  Note that
	 * zwnj may have been handled as <WBR> by the calling function.  - FM
	 */
	if (!strcmp(s, "zwnj") ||
	    !strcmp(s, "zwj")) {
	    CTRACE((tfp, "handle_entity: Ignoring '%s'.\n", s));
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		put_pretty_entity(me, term);
	    }
#endif
	    FoundEntity = TRUE;
	    return;
	}
	/*
	 * Ignore lrm (8206), and rln (8207), if we get to here.  - FM
	 */
	if (!strcmp(s, "lrm") ||
	    !strcmp(s, "rlm")) {
	    CTRACE((tfp, "handle_entity: Ignoring '%s'.\n", s));
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		put_pretty_entity(me, term);
	    }
#endif
	    FoundEntity = TRUE;
	    return;
	}
    }

    /*
     * If entity string not found, display as text.
     */
#ifdef USE_PRETTYSRC
    if (psrc_view)
	PSRCSTART(badseq);
#endif
    /* S/390 -- gil -- 0695 */
    CTRACE((tfp, "SGML: Unknown entity '%s' %" PRI_UCode_t " %ld\n", s, code, uck));
    PUTC('&');
    PUTS(s);
    if (term != '\0')
	PUTC(term);
#ifdef USE_PRETTYSRC
    if (psrc_view)
	PSRCSTOP(badseq);
#endif
}

/*	Handle comment
 *	--------------
 */
static void handle_comment(HTStream *me)
{
    const char *s = me->string->data;

    CTRACE((tfp, "SGML Comment:\n<%s>\n", s));

    if (me->csi == NULL &&
	StrNCmp(s, "!--#", 4) == 0 &&
	LYCheckForCSI(me->node_anchor, &me->url) == TRUE) {
	LYDoCSI(me->url, s, &me->csi);
    } else {
	LYCommentHacks(me->node_anchor, me->string->data);
    }

    return;
}

/*	Handle identifier
 *	-----------------
 */
static void handle_identifier(HTStream *me)
{
    const char *s = me->string->data;

    CTRACE((tfp, "SGML Identifier:\n<%s>\n", s));

    return;
}

/*	Handle doctype
 *	--------------
 */
static void handle_doctype(HTStream *me)
{
    const char *s = me->string->data;

    CTRACE((tfp, "SGML Doctype:\n<%s>\n", s));
    if (strstr(s, "DTD XHTML ") != 0) {
	CTRACE((tfp, "...processing extended HTML\n"));
	me->extended_html = TRUE;
    }

    return;
}

/*	Handle marked
 *	-------------
 */
static void handle_marked(HTStream *me)
{
    const char *s = me->string->data;

    CTRACE((tfp, "SGML Marked Section:\n<%s>\n", s));

    if (!StrNCmp(me->string->data, "![INCLUDE[", 10)) {
	me->string->data[me->string->size - 3] = '\0';
	StrAllocCat(me->include, me->string->data + 10);
	/* @@@ This needs to take charset into account! @@@
	   the wrong assumptions will be made about the data's
	   charset once it is in include - kw */

    } else if (!StrNCmp(me->string->data, "![CDATA[", 8)) {
	(*me->actions->put_block) (me->target,
				   me->string->data + 8,
				   me->string->size - 11);

    }
    return;
}

/*	Handle processing instruction
 *	-----------------------------
 */
static void handle_processing_instruction(HTStream *me)
{
    const char *s = me->string->data;

    CTRACE((tfp, "SGML Processing instruction:\n<%s>\n", s));

    if (!StrNCmp(s, "?xml ", 5)) {
	int flag = me->T.decode_utf8;

	me->strict_xml = TRUE;
	/*
	 * Switch to UTF-8 if the encoding is explicitly "utf-8".
	 */
	if (!flag) {
	    char *t = strstr(s, "encoding=");

	    if (t != 0) {
		char delim = 0;

		t += 9;
		if (*t == '"' || *t == '\'')
		    delim = *t++;
		flag = (!strncasecomp(t, "utf-8", 5) &&
			(delim == 0 || t[5] == delim));
	    }
	    if (flag) {
		CTRACE((tfp, "...Use UTF-8 for XML\n"));
		me->T.decode_utf8 = TRUE;
	    }
	}
    }

    return;
}

/*	Handle sgmlent
 *	--------------
 */
static void handle_sgmlent(HTStream *me)
{
    const char *s = me->string->data;

    CTRACE((tfp, "SGML Entity Declaration:\n<%s>\n", s));

    return;
}

/*	Handle sgmlent
 *	--------------
 */
static void handle_sgmlele(HTStream *me)
{
    const char *s = me->string->data;

    CTRACE((tfp, "SGML Element Declaration:\n<%s>\n", s));

    return;
}

/*	Handle sgmlatt
 *	--------------
 */
static void handle_sgmlatt(HTStream *me)
{
    const char *s = me->string->data;

    CTRACE((tfp, "SGML Attribute Declaration:\n<%s>\n", s));

    return;
}

/*
 * Convenience macros - tags (elements) are identified sometimes by an int or
 * enum value ('TAGNUM'), sometimes by a pointer to HTTag ('TAGP').  - kw
 */
#define TAGNUM_OF_TAGP(t) (HTMLElement) (t - me->dtd->tags)
#define TAGP_OF_TAGNUM(e) (me->dtd->tags + e)

/*
 * The following implement special knowledge about OBJECT.  As long as
 * HTML_OBJECT is the only tag for which an alternative variant exist, they can
 * be simple macros.  - kw
 */
/* does 'TAGNUM' e have an alternative (variant) parsing mode? */
#define HAS_ALT_TAGNUM(e) (e == HTML_OBJECT)

/* return 'TAGNUM' of the alternative mode for 'TAGNUM' e, if any. */
#define ALT_TAGNUM(e) ((e == HTML_OBJECT) ? HTML_ALT_OBJECT : e)

/* return 'TAGNUM' of the normal mode for 'TAGNUM' e which may be alt. */
#define NORMAL_TAGNUM(e) (((int)(e) >= HTML_ELEMENTS) ? HTML_OBJECT : (HTMLElement)e)

/* More convenience stuff. - kw */
#define ALT_TAGP_OF_TAGNUM(e) TAGP_OF_TAGNUM(ALT_TAGNUM(e))
#define NORMAL_TAGP_OF_TAGNUM(e) TAGP_OF_TAGNUM(NORMAL_TAGNUM(e))

#define ALT_TAGP(t) ALT_TAGP_OF_TAGNUM(TAGNUM_OF_TAGP(t))
#define NORMAL_TAGP(t) NORMAL_TAGP_OF_TAGNUM(TAGNUM_OF_TAGP(t))

#define IsTagAlias(a,b) (((a) == (b)) || ((a) - (a)->alias == (b) - (b)->alias))

static BOOL element_valid_within(HTTag * new_tag, HTTag * stacked_tag, int direct)
{
    BOOL result = YES;
    TagClass usecontains, usecontained;

    if (stacked_tag && new_tag) {
	usecontains = (direct ? stacked_tag->contains : stacked_tag->icontains);
	usecontained = (direct ? new_tag->contained : new_tag->icontained);
	if (IsTagAlias(new_tag, stacked_tag)) {
	    result = (BOOL) ((Tgc_same & usecontains) &&
			     (Tgc_same & usecontained));
	} else {
	    result = (BOOL) ((new_tag->tagclass & usecontains) &&
			     (stacked_tag->tagclass & usecontained));
	}
    }
    return result;
}

static BOOL element_really_within(HTTag * new_tag, HTTag * stacked_tag, int direct)
{
    BOOL result = YES;
    TagClass usecontains, usecontained;

    if (stacked_tag && new_tag) {
	usecontains = (direct ? stacked_tag->contains : stacked_tag->icontains);
	usecontained = (direct ? new_tag->contained : new_tag->icontained);
	if (IsTagAlias(new_tag, stacked_tag)) {
	    result = (BOOL) ((Tgc_same & usecontains) &&
			     (Tgc_same & usecontained));
	} else {
	    result = (BOOL) ((new_tag->tagclass & usecontains) ==
			     new_tag->tagclass &&
			     (stacked_tag->tagclass & usecontained) == stacked_tag->tagclass);
	}
    }
    return result;
}

typedef enum {
    close_NO = 0,
    close_error = 1,
    close_valid = 2
} canclose_t;

static canclose_t can_close(HTTag * new_tag, HTTag * stacked_tag)
{
    canclose_t result;

    if (!stacked_tag) {
	result = close_NO;
    } else if (stacked_tag->flags & Tgf_endO) {
	result = close_valid;
    } else if (IsTagAlias(new_tag, stacked_tag)) {
	result = ((Tgc_same & new_tag->canclose)
		  ? close_error
		  : close_NO);
    } else {
	result = ((stacked_tag->tagclass & new_tag->canclose)
		  ? close_error
		  : close_NO);
    }
    return result;
}

static void do_close_stacked(HTStream *me)
{
    HTElement *stacked = me->element_stack;
    HTMLElement e;

    if (!stacked)
	return;			/* stack was empty */
    if (me->inSELECT && !strcasecomp(stacked->tag->name, "SELECT")) {
	me->inSELECT = FALSE;
    }
    e = NORMAL_TAGNUM(TAGNUM_OF_TAGP(stacked->tag));
#ifdef USE_PRETTYSRC
    if (!psrc_view)		/* Don't actually pass call on if viewing psrc - kw */
#endif
	(*me->actions->end_element) (me->target,
				     (int) e,
				     &me->include);
    me->element_stack = stacked->next;
    pool_free(stacked);
    me->no_lynx_specialcodes =
	(BOOL) (me->element_stack
		? (me->element_stack->tag->flags & Tgf_nolyspcl)
		: NO);
}

static int is_on_stack(HTStream *me, HTTag * old_tag)
{
    HTElement *stacked = me->element_stack;
    int i = 1;

    for (; stacked; stacked = stacked->next, i++) {
	if (IsTagAlias(stacked->tag, old_tag) ||
	    stacked->tag == ALT_TAGP(old_tag))
	    return i;
    }
    return 0;
}

/*	End element
 *	-----------
 */
static void end_element(HTStream *me, HTTag * old_tag)
{
    BOOL extra_action_taken = NO;
    canclose_t canclose_check = close_valid;
    int stackpos = is_on_stack(me, old_tag);
    BOOL direct_container = YES;

    if (!Old_DTD) {
	if (old_tag->aliases) {
	    if (me->element_stack) {
		if (!element_really_within(old_tag,
					   me->element_stack->tag,
					   direct_container) &&
		    element_really_within(old_tag + 1,
					  me->element_stack->tag,
					  direct_container)) {
		    ++old_tag;
		}
	    }
	}
	while (canclose_check != close_NO &&
	       me->element_stack &&
	       (stackpos > 1 || (!extra_action_taken && stackpos == 0))) {
	    if (stackpos == 0 && (old_tag->flags & Tgf_startO) &&
		element_valid_within(old_tag, me->element_stack->tag, YES)) {
		CTRACE((tfp, "SGML: </%s> ignored\n", old_tag->name));
		return;
	    }
	    canclose_check = can_close(old_tag, me->element_stack->tag);
	    if (canclose_check != close_NO) {
		CTRACE((tfp, "SGML: End </%s> \t<- %s end </%s>\n",
			me->element_stack->tag->name,
			((canclose_check == close_valid)
			 ? "supplied,"
			 : "***forced by"),
			old_tag->name));
		do_close_stacked(me);
		extra_action_taken = YES;
		stackpos = is_on_stack(me, old_tag);
	    }
	}

	if (stackpos == 0 && old_tag->contents != SGML_EMPTY) {
	    CTRACE((tfp, "SGML: Still open %s, ***no open %s for </%s>\n",
		    me->element_stack ?
		    me->element_stack->tag->name : "none",
		    old_tag->name,
		    old_tag->name));
	    return;
	}
	if (stackpos > 1) {
	    CTRACE((tfp,
		    "SGML: Nesting <%s>...<%s> \t<- ***invalid end </%s>\n",
		    old_tag->name,
		    me->element_stack ?
		    me->element_stack->tag->name : "none",
		    old_tag->name));
	    return;
	}
    }
    /* Now let the non-extended code deal with the rest. - kw */

    /*
     * If we are in a SELECT block, ignore anything but a SELECT end tag.  - FM
     */
    if (me->inSELECT) {
	if (!strcasecomp(old_tag->name, "SELECT")) {
	    /*
	     * Turn off the inSELECT flag and fall through.  - FM
	     */
	    me->inSELECT = FALSE;
	} else {
	    /*
	     * Ignore the end tag.  - FM
	     */
	    CTRACE((tfp, "SGML: ***Ignoring end tag </%s> in SELECT block.\n",
		    old_tag->name));
	    return;
	}
    }
    /*
     * Handle the end tag.  - FM
     */
    CTRACE((tfp, "SGML: End </%s>\n", old_tag->name));
    if (old_tag->contents == SGML_EMPTY) {
	CTRACE((tfp, "SGML: ***Illegal end tag </%s> found.\n",
		old_tag->name));
	return;
    }
#ifdef WIND_DOWN_STACK
    while (me->element_stack)	/* Loop is error path only */
#else
    if (me->element_stack)	/* Substitute and remove one stack element */
#endif /* WIND_DOWN_STACK */
    {
	int status = HT_OK;
	HTMLElement e;
	HTElement *N = me->element_stack;
	HTTag *t = (N->tag != old_tag) ? NORMAL_TAGP(N->tag) : N->tag;

	if (old_tag != t) {	/* Mismatch: syntax error */
	    if (me->element_stack->next) {	/* This is not the last level */
		CTRACE((tfp,
			"SGML: Found </%s> when expecting </%s>. </%s> ***assumed.\n",
			old_tag->name, t->name, t->name));
	    } else {		/* last level */
		CTRACE((tfp,
			"SGML: Found </%s> when expecting </%s>. </%s> ***Ignored.\n",
			old_tag->name, t->name, old_tag->name));
		return;		/* Ignore */
	    }
	}

	e = NORMAL_TAGNUM(TAGNUM_OF_TAGP(t));
	CTRACE2(TRACE_SGML, (tfp, "tagnum(%p) = %d\n", (void *) t, (int) e));
#ifdef USE_PRETTYSRC
	if (!psrc_view)		/* Don't actually pass call on if viewing psrc - kw */
#endif
	    status = (*me->actions->end_element) (me->target,
						  (int) e,
						  &me->include);
	if (status == HT_PARSER_REOPEN_ELT) {
	    CTRACE((tfp, "SGML: Restart <%s>\n", t->name));
	    (*me->actions->start_element) (me->target,
					   (int) e,
					   NULL,
					   NULL,
					   me->current_tag_charset,
					   &me->include);
	} else if (status == HT_PARSER_OTHER_CONTENT) {
	    CTRACE((tfp, "SGML: Continue with other content model for <%s>\n", t->name));
	    me->element_stack->tag = ALT_TAGP_OF_TAGNUM(e);
	} else {
	    me->element_stack = N->next;	/* Remove from stack */
	    pool_free(N);
	}
	me->no_lynx_specialcodes =
	    (BOOL) (me->element_stack
		    ? (me->element_stack->tag->flags & Tgf_nolyspcl)
		    : NO);
#ifdef WIND_DOWN_STACK
	if (IsTagAlias(old_tag, t))
	    return;		/* Correct sequence */
#else
	return;
#endif /* WIND_DOWN_STACK */

	/* Syntax error path only */

    }
    CTRACE((tfp, "SGML: Extra end tag </%s> found and ignored.\n",
	    old_tag->name));
}

/*	Start a element
*/
static void start_element(HTStream *me)
{
    int status;
    HTTag *new_tag = me->current_tag;
    HTMLElement e = TAGNUM_OF_TAGP(new_tag);
    BOOL ok = FALSE;

    BOOL valid = YES;
    BOOL direct_container = YES;
    BOOL extra_action_taken = NO;
    canclose_t canclose_check = close_valid;

    if (!Old_DTD) {
	if (new_tag->aliases) {
	    if (me->element_stack) {
		if (!element_really_within(new_tag,
					   me->element_stack->tag,
					   direct_container) &&
		    element_really_within(new_tag + 1,
					  me->element_stack->tag,
					  direct_container)) {
		    ++new_tag;
		}
	    }
	}
	while (me->element_stack &&
	       (canclose_check == close_valid ||
		(canclose_check == close_error &&
		 IsTagAlias(new_tag, me->element_stack->tag))) &&
	       !(valid = element_valid_within(new_tag,
					      me->element_stack->tag,
					      direct_container))) {
	    canclose_check = can_close(new_tag, me->element_stack->tag);
	    if (canclose_check != close_NO) {
		CTRACE((tfp, "SGML: End </%s> \t<- %s start <%s>\n",
			me->element_stack->tag->name,
			((canclose_check == close_valid)
			 ? "supplied,"
			 : "***forced by"),
			new_tag->name));
		do_close_stacked(me);
		extra_action_taken = YES;
		if (canclose_check == close_error)
		    direct_container = NO;
	    } else {
		CTRACE((tfp,
			"SGML: Still open %s \t<- ***invalid start <%s>\n",
			me->element_stack->tag->name,
			new_tag->name));
	    }
	}
	if (me->element_stack && !valid &&
	    (me->element_stack->tag->flags & Tgf_strict) &&
	    !(valid = element_valid_within(new_tag,
					   me->element_stack->tag,
					   direct_container))) {
	    CTRACE((tfp, "SGML: Still open %s \t<- ***ignoring start <%s>\n",
		    me->element_stack->tag->name,
		    new_tag->name));
	    return;
	}

	if (me->element_stack &&
	    !extra_action_taken &&
	    (canclose_check == close_NO) &&
	    !valid && (new_tag->flags & Tgf_mafse)) {
	    BOOL has_attributes = NO;
	    int i = 0;

	    for (; i < new_tag->number_of_attributes && !has_attributes; i++)
		has_attributes = me->present[i];
	    if (!has_attributes) {
		CTRACE((tfp,
			"SGML: Still open %s, ***converting invalid <%s> to </%s>\n",
			me->element_stack->tag->name,
			new_tag->name,
			new_tag->name));
		end_element(me, new_tag);
		return;
	    }
	}

	if (me->element_stack &&
	    (canclose_check == close_error) &&
	    !element_valid_within(new_tag,
				  me->element_stack->tag,
				  direct_container)) {
	    CTRACE((tfp, "SGML: Still open %s \t<- ***invalid start <%s>\n",
		    me->element_stack->tag->name,
		    new_tag->name));
	}
    }
    /* Fall through to the non-extended code - kw */

    /*
     * If we are not in a SELECT block, check if this is a SELECT start tag. 
     * Otherwise (i.e., we are in a SELECT block) accept only OPTION as valid,
     * terminate the SELECT block if it is any other form-related element, and
     * otherwise ignore it.  - FM
     */
    if (!me->inSELECT) {
	/*
	 * We are not in a SELECT block, so check if this starts one.  - FM
	 * (frequent case!)
	 */
	/* my_casecomp() - optimized by the first character */
	if (!my_casecomp(new_tag->name, "SELECT")) {
	    /*
	     * Set the inSELECT flag and fall through.  - FM
	     */
	    me->inSELECT = TRUE;
	}
    } else {
	/*
	 * We are in a SELECT block.  - FM
	 */
	if (strcasecomp(new_tag->name, "OPTION")) {
	    /*
	     * Ugh, it is not an OPTION.  - FM
	     */
	    switch (e) {
	    case HTML_INPUT:
	    case HTML_TEXTAREA:
	    case HTML_SELECT:
	    case HTML_BUTTON:
	    case HTML_FIELDSET:
	    case HTML_LABEL:
	    case HTML_LEGEND:
	    case HTML_FORM:
		ok = TRUE;
		break;
	    default:
		break;
	    }
	    if (ok) {
		/*
		 * It is another form-related start tag, so terminate the
		 * current SELECT block and fall through.  - FM
		 */
		CTRACE((tfp,
			"SGML: ***Faking SELECT end tag before <%s> start tag.\n",
			new_tag->name));
		end_element(me, SGMLFindTag(me->dtd, "SELECT"));
	    } else {
		/*
		 * Ignore the start tag.  - FM
		 */
		CTRACE((tfp,
			"SGML: ***Ignoring start tag <%s> in SELECT block.\n",
			new_tag->name));
		return;
	    }
	}
    }
    /*
     * Handle the start tag.  - FM
     */
    CTRACE((tfp, "SGML: Start <%s>\n", new_tag->name));
    status = (*me->actions->start_element) (me->target,
					    (int) TAGNUM_OF_TAGP(new_tag),
					    me->present,
					    (STRING2PTR) me->value,	/* coerce type for think c */
					    me->current_tag_charset,
					    &me->include);
    if (status == HT_PARSER_OTHER_CONTENT)
	new_tag = ALT_TAGP(new_tag);	/* this is only returned for OBJECT */
    if (new_tag->contents != SGML_EMPTY) {	/* i.e., tag not empty */
	HTElement *N = pool_alloc();

	if (N == NULL)
	    outofmem(__FILE__, "start_element");

	N->next = me->element_stack;
	N->tag = new_tag;
	me->element_stack = N;
	me->no_lynx_specialcodes = (BOOLEAN) (new_tag->flags & Tgf_nolyspcl);

    } else if (e == HTML_META) {
	/*
	 * Check for result of META tag.  - KW & FM
	 */
	change_chartrans_handling(me);
    }
}

/*		Find Tag in DTD tag list
 *		------------------------
 *
 * On entry,
 *	dtd	points to dtd structure including valid tag list
 *	string	points to name of tag in question
 *
 * On exit,
 *	returns:
 *		NULL		tag not found
 *		else		address of tag structure in dtd
 */
HTTag *SGMLFindTag(const SGML_dtd * dtd,
		   const char *s)
{
    int high, low, i, diff;
    static HTTag *last[64] =
    {NULL};			/*optimize using the previous results */
    HTTag **res = last + (UCH(*s) % 64);	/*pointer arithmetic */

    if (*res) {
	if ((*res)->name == NULL)
	    return NULL;
	if (!strcasecomp((*res)->name, s))
	    return *res;
    }

    for (low = 0, high = dtd->number_of_tags;
	 high > low;
	 diff < 0 ? (low = i + 1) : (high = i)) {	/* Binary search */
	i = (low + (high - low) / 2);
	/* my_casecomp() - optimized by the first character, NOT_ASCII ok */
	diff = my_casecomp(dtd->tags[i].name, s);	/* Case insensitive */
	if (diff == 0) {	/* success: found it */
	    i -= dtd->tags[i].alias;
	    *res = &dtd->tags[i];
	    return *res;
	}
    }
    if (IsNmStart(*s)) {
	/*
	 * Unrecognized, but may be valid.  - KW
	 */
	return &HTTag_unrecognized;
    }
    return NULL;
}

/*________________________________________________________________________
 *			Public Methods
 */

/*	Could check that we are back to bottom of stack! @@  */
/*	Do check! - FM					     */
/*							     */
static void SGML_free(HTStream *me)
{
    int i;
    HTElement *cur;
    HTTag *t;

    /*
     * Free the buffers.  - FM
     */
    FREE(me->recover);
    FREE(me->url);
    FREE(me->csi);
    FREE(me->include);
    FREE(me->active_include);

    /*
     * Wind down stack if any elements are open.  - FM
     */
    while (me->element_stack) {
	cur = me->element_stack;
	t = cur->tag;
	me->element_stack = cur->next;	/* Remove from stack */
	pool_free(cur);
#ifdef USE_PRETTYSRC
	if (!psrc_view)		/* Don't actually call on target if viewing psrc - kw */
#endif
	    (*me->actions->end_element)
		(me->target,
		 (int) NORMAL_TAGNUM(TAGNUM_OF_TAGP(t)),
		 &me->include);
	FREE(me->include);
    }

    /*
     * Finish off the target.  - FM
     */
    (*me->actions->_free) (me->target);

    /*
     * Free the strings and context structure.  - FM
     */
    HTChunkFree(me->string);
    for (i = 0; i < MAX_ATTRIBUTES; i++)
	FREE_extra(me->value[i]);
    FREE(me);

#ifdef USE_PRETTYSRC
    sgml_in_psrc_was_initialized = FALSE;
#endif
}

static void SGML_abort(HTStream *me, HTError e)
{
    int i;
    HTElement *cur;

    /*
     * Abort the target.  - FM
     */
    (*me->actions->_abort) (me->target, e);

    /*
     * Free the buffers.  - FM
     */
    FREE(me->recover);
    FREE(me->include);
    FREE(me->active_include);
    FREE(me->url);
    FREE(me->csi);

    /*
     * Free stack memory if any elements were left open.  - KW
     */
    while (me->element_stack) {
	cur = me->element_stack;
	me->element_stack = cur->next;	/* Remove from stack */
	pool_free(cur);
    }

    /*
     * Free the strings and context structure.  - FM
     */
    HTChunkFree(me->string);
    for (i = 0; i < MAX_ATTRIBUTES; i++)
	FREE_extra(me->value[i]);
    FREE(me);

#ifdef USE_PRETTYSRC
    sgml_in_psrc_was_initialized = FALSE;
#endif
}

/*	Read and write user callback handle
 *	-----------------------------------
 *
 *   The callbacks from the SGML parser have an SGML context parameter.
 *   These calls allow the caller to associate his own context with a
 *   particular SGML context.
 */

#ifdef CALLERDATA
void *SGML_callerData(HTStream *me)
{
    return me->callerData;
}

void SGML_setCallerData(HTStream *me, void *data)
{
    me->callerData = data;
}
#endif /* CALLERDATA */

#ifdef USE_PRETTYSRC
static void transform_tag(HTStream *me, HTChunk *string)
{
    if (!me->strict_xml) {
	if (tagname_transform != 1) {
	    if (tagname_transform == 0)
		LYLowerCase(string->data);
	    else
		LYUpperCase(string->data);
	}
    }
}
#endif /* USE_PRETTYSRC */

static BOOL ignore_when_empty(HTTag * tag)
{
    BOOL result = FALSE;

    if (!LYPreparsedSource
	&& LYxhtml_parsing
	&& tag->name != 0
	&& !(tag->flags & Tgf_mafse)
	&& tag->contents != SGML_EMPTY
	&& tag->tagclass != Tgc_Plike
	&& (tag->tagclass == Tgc_APPLETlike
	    || tag->tagclass == Tgc_SELECTlike
	    || (tag->contains && tag->icontains))) {
	result = TRUE;
    }
    CTRACE((tfp, "SGML Do%s ignore_when_empty:%s\n",
	    result ? "" : " not",
	    NonNull(tag->name)));
    return result;
}

static void discard_empty(HTStream *me)
{
    static HTTag empty_tag;

    CTRACE((tfp, "SGML discarding empty %s\n",
	    NonNull(me->current_tag->name)));
    CTRACE_FLUSH(tfp);

    memset(&empty_tag, 0, sizeof(empty_tag));
    me->current_tag = &empty_tag;
    me->string->size = 0;

    /* do not call end_element() if start_element() was not called */
}

#ifdef USE_PRETTYSRC
static BOOL end_if_prettysrc(HTStream *me, HTChunk *string, int end_ch)
{
    BOOL result = psrc_view;

    if (psrc_view) {
	if (attr_is_name) {
	    HTStartAnchor(me->target, string->data, NULL);
	    (*me->actions->end_element) (me->target,
					 HTML_A,
					 &me->include);
	} else if (attr_is_href) {
	    PSRCSTART(href);
	    HTStartAnchor(me->target, NULL, string->data);
	}
	PUTS_TR(string->data);
	if (attr_is_href) {
	    (*me->actions->end_element) (me->target,
					 HTML_A,
					 &me->include);
	    PSRCSTOP(href);
	}
	if (end_ch)
	    PUTC(end_ch);
	PSRCSTOP(attrval);
    }
    return result;
}
#endif

static void SGML_character(HTStream *me, int c_in)
{
    const SGML_dtd *dtd = me->dtd;
    HTChunk *string = me->string;
    const char *EntityName;
    HTTag *testtag = NULL;
    BOOLEAN chk;		/* Helps (?) walk through all the else ifs... */
    UCode_t clong, uck = 0;	/* Enough bits for UCS4 ... */
    int testlast;

    unsigned char c;
    unsigned char saved_char_in = '\0';

    ++sgml_offset;

    c = UCH(c_in);
    clong = UCH(c);

    if (me->T.decode_utf8) {
	switch (HTDecodeUTF8(&(me->U), &c_in, &clong)) {
	case dUTF8_ok:
	    if (clong < 256) {
		c_in = FROMASCII(UCH(clong));
	    }
	    break;
	case dUTF8_err:
	    clong = UCS_REPL;
	    strcpy(me->U.utf_buf, "\357\277\275");
	    me->U.utf_buf_p = (me->U.utf_buf + 3);
	    break;
	case dUTF8_more:
	    return;
	}

	c = UCH(c_in);
	if ((me->U.utf_buf_p - me->U.utf_buf) > 1) {
	    goto top1;
	}
    }

    /*
     * If we want the raw input converted to Unicode, try that now.  - FM
     */
#ifdef USE_JAPANESEUTF8_SUPPORT
    /* Convert ISO-2022-JP to Unicode (charset=iso-2022-jp is unrecognized) */
#define IS_JIS7_HILO(c) (0x20<(c)&&(c)<0x7F)
    if (UTF8_TTY_ISO2022JP && (me->state == S_nonascii_text
			       || me->state == S_nonascii_text_sq
			       || me->state == S_nonascii_text_dq)) {
	/* end of ISO-2022-JP? || not in ISO-2022-JP range */
	if (TOASCII(c) == '\033' || !IS_JIS7_HILO(c)) {
	    me->kanji_buf = '\0';
	    goto top1;
	}
	if (me->kanji_buf == '\t') {	/* flag for single byte kana in "ESC(I" */
	    if (conv_jisx0201kana) {
		JISx0201TO0208_SJIS(c | 0200,
				    (unsigned char *) me->U.utf_buf,
				    (unsigned char *) me->U.utf_buf + 1);
		clong = UCTransJPToUni(me->U.utf_buf, 2,
				       UCGetLYhndl_byMIME("shift_jis"));
	    } else {
		clong = UCTransToUni(c | 0200, UCGetLYhndl_byMIME("shift_jis"));
	    }
	} else if (me->kanji_buf) {
	    me->U.utf_buf[0] = (char) (me->kanji_buf | 0200);	/* to EUC-JP */
	    me->U.utf_buf[1] = (char) (c | 0200);
	    clong = UCTransJPToUni(me->U.utf_buf, 2,
				   UCGetLYhndl_byMIME("euc-jp"));
	    me->kanji_buf = '\0';
	} else {
	    me->kanji_buf = c;
	    clong = ucNeedMore;
	}
	goto top1;
    }
#endif /* USE_JAPANESEUTF8_SUPPORT */
#ifdef USE_JAPANESEUTF8_SUPPORT
    if (me->T.trans_to_uni &&
	((strcmp(LYCharSet_UC[me->inUCLYhndl].MIMEname, "euc-jp") == 0) ||
	 (strcmp(LYCharSet_UC[me->inUCLYhndl].MIMEname, "shift_jis") == 0))) {
	if (strcmp(LYCharSet_UC[me->inUCLYhndl].MIMEname, "shift_jis") == 0) {
	    if (me->U.utf_count == 0) {
		if (IS_SJIS_HI1(c) ||
		    IS_SJIS_HI2(c)) {
		    me->U.utf_buf[0] = (char) c;
		    me->U.utf_count = 1;
		    clong = ucCannotConvert;
		} else if (IS_SJIS_X0201KANA(c)) {
		    if (conv_jisx0201kana) {
			JISx0201TO0208_SJIS(c,
					    (unsigned char *) me->U.utf_buf,
					    (unsigned char *) me->U.utf_buf + 1);
			clong = UCTransJPToUni(me->U.utf_buf, 2, me->inUCLYhndl);
		    } else {
			clong = UCTransToUni(c, me->inUCLYhndl);
		    }
		}
	    } else {
		if (IS_SJIS_LO(c)) {
		    me->U.utf_buf[1] = (char) c;
		    clong = UCTransJPToUni(me->U.utf_buf, 2, me->inUCLYhndl);
		}
		me->U.utf_count = 0;
	    }
	} else {
	    if (me->U.utf_count == 0) {
		if (IS_EUC_HI(c) || c == 0x8E) {
		    me->U.utf_buf[0] = (char) c;
		    me->U.utf_count = 1;
		    clong = ucCannotConvert;
		}
	    } else {
		if (IS_EUC_LOX(c)) {
		    me->U.utf_buf[1] = (char) c;
		    clong = UCTransJPToUni(me->U.utf_buf, 2, me->inUCLYhndl);
		}
		me->U.utf_count = 0;
	    }
	}
	goto top1;
    } else
#endif /* USE_JAPANESEUTF8_SUPPORT */
#ifdef EXP_CHINESEUTF8_SUPPORT
	if (me->T.trans_to_uni &&
	    ((strcmp(LYCharSet_UC[me->inUCLYhndl].MIMEname, "euc-cn") == 0))) {
	if (me->U.utf_count == 0) {
	    if (IS_GBK_HI(c)) {
		me->U.utf_buf[0] = (char) c;
		me->U.utf_count = 1;
		clong = ucCannotConvert;
		CTRACE((tfp, "Get EUC-CN: 0x%02X\n", UCH(c)));
	    }
	} else {
	    if (IS_GBK_LO(c)) {
		me->U.utf_buf[1] = (char) c;
		clong = UCTransJPToUni(me->U.utf_buf, 2, me->inUCLYhndl);
		if (clong > 0) {
		    CTRACE((tfp, "... second: [%02X%02X] U+%04lX\n",
			    UCH(me->U.utf_buf[0]),
			    UCH(me->U.utf_buf[1]),
			    clong));
		} else {
		    CTRACE((tfp, "... second: [%02X%02X] %ld\n",
			    UCH(me->U.utf_buf[0]),
			    UCH(me->U.utf_buf[1]),
			    clong));
		}
	    }
	    me->U.utf_count = 0;
	}
	goto top1;
    } else
#endif /* EXP_CHINESEUTF8_SUPPORT */
#ifdef EXP_CHINESEUTF8_SUPPORT
	if (me->T.trans_to_uni &&
	    ((strcmp(LYCharSet_UC[me->inUCLYhndl].MIMEname, "euc-kr") == 0))) {
	if (me->U.utf_count == 0) {
	    if (IS_EUC_HI(c)) {
		me->U.utf_buf[0] = (char) c;
		me->U.utf_count = 1;
		clong = ucCannotConvert;
		CTRACE((tfp, "Get EUC-KR: 0x%02X\n", UCH(c)));
	    }
	} else {
	    if (IS_EUC_LOS(c) ||
		IS_EUC_LOX(c)) {
		me->U.utf_buf[1] = (char) c;
		clong = UCTransJPToUni(me->U.utf_buf, 2, me->inUCLYhndl);
		if (clong > 0) {
		    CTRACE((tfp, "... second: [%02X%02X] U+%04lX\n",
			    UCH(me->U.utf_buf[0]),
			    UCH(me->U.utf_buf[1]),
			    clong));
		} else {
		    CTRACE((tfp, "... second: [%02X%02X] %ld\n",
			    UCH(me->U.utf_buf[0]),
			    UCH(me->U.utf_buf[1]),
			    clong));
		}
	    }
	    me->U.utf_count = 0;
	}
	goto top1;
    } else
#endif /* EXP_CHINESEUTF8_SUPPORT */
#ifdef EXP_CHINESEUTF8_SUPPORT
	if (me->T.trans_to_uni &&
	    ((strcmp(LYCharSet_UC[me->inUCLYhndl].MIMEname, "big5") == 0))) {
	if (me->U.utf_count == 0) {
	    if (IS_BIG5_HI(c)) {
		me->U.utf_buf[0] = (char) c;
		me->U.utf_count = 1;
		clong = ucCannotConvert;
		CTRACE((tfp, "Get BIG5: 0x%02X\n", UCH(c)));
	    }
	} else {
	    if (IS_BIG5_LOS(c) ||
		IS_BIG5_LOX(c)) {
		me->U.utf_buf[1] = (char) c;
		clong = UCTransJPToUni(me->U.utf_buf, 2, me->inUCLYhndl);
		if (clong > 0) {
		    CTRACE((tfp, "... second: [%02X%02X] U+%04lX\n",
			    UCH(me->U.utf_buf[0]),
			    UCH(me->U.utf_buf[1]),
			    clong));
		} else {
		    CTRACE((tfp, "... second: [%02X%02X] %ld\n",
			    UCH(me->U.utf_buf[0]),
			    UCH(me->U.utf_buf[1]),
			    clong));
		}
	    }
	    me->U.utf_count = 0;
	}
	goto top1;
    } else
#endif /* EXP_CHINESEUTF8_SUPPORT */
	if (me->T.trans_to_uni &&
	/* S/390 -- gil -- 0744 */
	    ((TOASCII(clong) >= LYlowest_eightbit[me->inUCLYhndl]) ||
	     (clong < ' ' && clong != 0 &&
	      me->T.trans_C0_to_uni))) {
	/*
	 * Convert the octet to Unicode.  - FM
	 */
	clong = UCTransToUni((char) c, me->inUCLYhndl);
	if (clong > 0) {
	    saved_char_in = c;
	    if (clong < 256) {
		c = FROMASCII(UCH(clong));
	    }
	}
	goto top1;
    } else if (clong < ' ' && clong != 0 &&	/* S/390 -- gil -- 0768 */
	       me->T.trans_C0_to_uni) {
	/*
	 * This else if may be too ugly to keep.  - KW
	 */
	if (me->T.trans_from_uni &&
	    (((clong = UCTransToUni((char) c, me->inUCLYhndl)) >= ' ') ||
	     (me->T.transp &&
	      (clong = UCTransToUni((char) c, me->inUCLYhndl)) > 0))) {
	    saved_char_in = c;
	    if (clong < 256) {
		c = FROMASCII(UCH(clong));
	    }
	    goto top1;
	} else {
	    uck = -1;
	    if (me->T.transp) {
		uck = UCTransCharStr(replace_buf, 60, (char) c,
				     me->inUCLYhndl,
				     me->inUCLYhndl, NO);
	    }
	    if (!me->T.transp || uck < 0) {
		uck = UCTransCharStr(replace_buf, 60, (char) c,
				     me->inUCLYhndl,
				     me->outUCLYhndl, YES);
	    }
	    if (uck == 0) {
		return;
	    } else if (uck < 0) {
		goto top0a;
	    }
	    c = UCH(replace_buf[0]);
	    if (c && replace_buf[1]) {
		if (me->state == S_text) {
		    PUTS(replace_buf);
		    return;
		}
		StrAllocCat(me->recover, replace_buf + 1);
	    }
	    goto top0a;
	}			/*  Next line end of ugly stuff for C0. - KW */
    } else {			/* end of me->T.trans_to_uni  S/390 -- gil -- 0791 */
	goto top0a;
    }

    /*
     *  We jump up to here from below if we have
     *  stuff in the recover, insert, or csi buffers
     *  to process.      We zero saved_char_in, in effect
     *  as a flag that the octet is not that of the
     *  actual call to this function.  This may be OK
     *  for now, for the stuff this function adds to
     *  its recover buffer, but it might not be for
     *  stuff other functions added to the insert or
     *  csi buffer, so bear that in mind. - FM
     *  Stuff from the recover buffer is now handled
     *  as UTF-8 if we can expect that's what it is,
     *  and in that case we don't come back up here. - kw
     */
  top:
    saved_char_in = '\0';
    /*
     *  We jump to here from above when we don't have
     *  UTF-8 input, haven't converted to Unicode, and
     *  want clong set to the input octet (unsigned)
     *  without zeroing its saved_char_in copy (which
     *  is signed). - FM
     */
  top0a:
    *(me->U.utf_buf) = '\0';
    clong = UCH(c);
    /*
     *  We jump to here from above if we have converted
     *  the input, or a multibyte sequence across calls,
     *  to a Unicode value and loaded it into clong (to
     *  which unsign_c has been defined), and from below
     *  when we are recycling a character (e.g., because
     *  it terminated an entity but is not the standard
     *  semi-colon).  The character will already have
     *  been put through the Unicode conversions. - FM
     */
  top1:
    /*
     * Ignore low ISO 646 7-bit control characters if HTCJK is not set.  - FM
     */
    /*
     * Works for both ASCII and EBCDIC. -- gil
     * S/390 -- gil -- 0811
     */
    if (TOASCII(clong) < 32 &&
	c != '\t' && c != '\n' && c != '\r' &&
	!IS_CJK_TTY &&
	!(UTF8_TTY_ISO2022JP && (TOASCII(c) == '\033')))
	goto after_switch;

    /*
     * Ignore 127 if we don't have HTPassHighCtrlRaw or HTCJK set.  - FM
     */
#define PASSHICTRL (me->T.transp || \
		    clong >= LYlowest_eightbit[me->inUCLYhndl])
    if (TOASCII(c) == 127 &&	/* S/390 -- gil -- 0830 */
	!(PASSHICTRL || IS_CJK_TTY))
	goto after_switch;

    /*
     * Ignore 8-bit control characters 128 - 159 if neither HTPassHighCtrlRaw
     * nor HTCJK is set.  - FM
     */
    if (TOASCII(clong) > 127 && TOASCII(clong) < 160 &&		/* S/390 -- gil -- 0847 */
	!(PASSHICTRL || IS_CJK_TTY)) {
	/*
	 * If we happen to be reading from an "ISO-8859-1" or "US-ASCII"
	 * document, allow the cp-1252 codes, to accommodate the HTML5 draft
	 * recommendation for replacement encoding:
	 *
	 * http://www.whatwg.org/specs/web-apps/current-work/multipage/infrastructure.html#character-encodings-0
	 */
	if (AssumeCP1252(me)) {
	    clong = LYcp1252ToUnicode((UCode_t) c);
	    goto top1;
	}
	goto after_switch;
    }

    /* Almost all CJK characters are double byte but only Japanese
     * JIS X0201 Kana is single byte. To prevent to fail SGML parsing
     * we have to take care of them here. -- TH
     */
    if ((HTCJK == JAPANESE) && (me->state == S_in_kanji) &&
	!IS_JAPANESE_2BYTE(me->kanji_buf, UCH(c))
#ifdef USE_JAPANESEUTF8_SUPPORT
	&& !me->T.decode_utf8
#endif
	) {
#ifdef CONV_JISX0201KANA_JISX0208KANA
	if (IS_SJIS_X0201KANA(me->kanji_buf)) {
	    unsigned char sjis_hi, sjis_lo;

	    JISx0201TO0208_SJIS(me->kanji_buf, &sjis_hi, &sjis_lo);
	    PUTC(sjis_hi);
	    PUTC(sjis_lo);
	} else
#endif
	    PUTC(me->kanji_buf);
	me->state = S_text;
    }

    /*
     * Handle character based on me->state.
     */
    CTRACE2(TRACE_SGML, (tfp, "SGML before %s|%.*s|%c|\n",
			 state_name(me->state),
			 string->size,
			 NonNull(string->data),
			 UCH(c)));
    switch (me->state) {

    case S_in_kanji:
	/*
	 * Note that if we don't have a CJK input, then this is not the second
	 * byte of a CJK di-byte, and we're trashing the input.  That's why
	 * 8-bit characters followed by, for example, '<' can cause the tag to
	 * be treated as text, not markup.  We could try to deal with it by
	 * holding each first byte and then checking byte pairs, but that
	 * doesn't seem worth the overhead (see below).  - FM
	 */
	me->state = S_text;
	PUTC(me->kanji_buf);
	PUTC(c);
	break;

    case S_tagname_slash:
	/*
	 * We had something link "<name/" so far, set state to S_text but keep
	 * me->slashedtag as a flag; except if we get '>' directly
	 * after the "<name/", and really have a tag for that name in
	 * me->slashedtag, in which case keep state as is and let code
	 * below deal with it.  - kw
	 */
	if (!(c == '>' && me->slashedtag && TOASCII(clong) < 127)) {
	    me->state = S_text;
	}
	/* FALLTHRU */
    case S_text:
#ifdef EXP_CHINESEUTF8_SUPPORT
	if (IS_CJK_TTY &&
	    (!strcmp(LYCharSet_UC[me->inUCLYhndl].MIMEname, "euc-cn") ||
	     !strcmp(LYCharSet_UC[me->inUCLYhndl].MIMEname, "big5") ||
	     !strcmp(LYCharSet_UC[me->inUCLYhndl].MIMEname, "euc-kr"))) {
	    /*
	     * Leave the case statement if we have not collected both of the
	     * bytes for the EUC-CN character.  If we have, then continue on
	     * to convert it to Unicode.
	     */
	    if (clong == ucCannotConvert) {
		break;
	    }
	} else
#endif
	    if (IS_CJK_TTY && ((TOASCII(c) & 0200) != 0)
#ifdef USE_JAPANESEUTF8_SUPPORT
		&& !me->T.decode_utf8
#endif
	    ) {			/* S/390 -- gil -- 0864 */
	    /*
	     * Setting up for Kanji multibyte handling (based on Takuya ASADA's
	     * (asada@three-a.co.jp) CJK Lynx).  Note that if the input is not
	     * in fact CJK, the next byte also will be mishandled, as explained
	     * above.  Toggle raw mode off in such cases, or select the "7 bit
	     * approximations" display character set, which is largely
	     * equivalent to having raw mode off with CJK.  - FM
	     */
	    me->state = S_in_kanji;
	    me->kanji_buf = c;
	    break;
	} else if ((IS_CJK_TTY || UTF8_TTY_ISO2022JP) && TOASCII(c) == '\033') {
	    /* S/390 -- gil -- 0881 */
	    /*
	     * Setting up for CJK escape sequence handling (based on Takuya
	     * ASADA's (asada@three-a.co.jp) CJK Lynx).  - FM
	     */
	    me->state = S_esc;
	    if (!UTF8_TTY_ISO2022JP)
		PUTC(c);
	    break;
	}

	if (c == '&' || c == '<') {
#ifdef USE_PRETTYSRC
	    if (psrc_view) {	/*there is nothing useful in the element_stack */
		testtag = me->current_tag;
	    } else
#endif
	    {
		testtag = me->element_stack ?
		    me->element_stack->tag : NULL;
	    }
	}

	if (c == '&' && TOASCII(clong) < 127 &&		/* S/390 -- gil -- 0898 */
	    (!testtag ||
	     (testtag->contents == SGML_MIXED ||
	      testtag->contents == SGML_ELEMENT ||
	      testtag->contents == SGML_PCDATA ||
#ifdef USE_PRETTYSRC
	      testtag->contents == SGML_EMPTY ||
#endif
	      testtag->contents == SGML_RCDATA))) {
	    /*
	     * Setting up for possible entity, without the leading '&'.  - FM
	     */
	    string->size = 0;
	    me->state = S_ero;
	} else if (c == '<' && TOASCII(clong) < 127) {	/* S/390 -- gil -- 0915 */
	    /*
	     * Setting up for possible tag.  - FM
	     */
	    string->size = 0;
	    if (testtag && testtag->contents == SGML_PCDATA) {
		me->state = S_pcdata;
	    } else if (testtag && (testtag->contents == SGML_LITTERAL
				   || testtag->contents == SGML_CDATA)) {
		me->state = S_litteral;
	    } else if (testtag && (testtag->contents == SGML_SCRIPT)) {
		me->state = S_script;
	    } else {
		me->state = S_tag;
	    }
	    me->slashedtag = NULL;
	} else if (me->slashedtag &&
		   me->slashedtag->name &&
		   (c == '/' ||
		    (c == '>' && me->state == S_tagname_slash)) &&
		   TOASCII(clong) < 127) {
	    /*
	     * We got either the second slash of a pending "<NAME/blah blah/"
	     * shortref construct, or the '>' of a mere "<NAME/>".  In both
	     * cases generate a "</NAME>" end tag in the recover buffer for
	     * reparsing unless NAME is really an empty element.  - kw
	     */
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(abracket);
		PUTC(c);
		PSRCSTOP(abracket);
	    } else
#endif
		if (me->slashedtag != me->unknown_tag &&
		    !ReallyEmptyTag(me->slashedtag)) {
		if (me->recover == NULL) {
		    StrAllocCopy(me->recover, "</");
		    me->recover_index = 0;
		} else {
		    StrAllocCat(me->recover, "</");
		}
		StrAllocCat(me->recover, me->slashedtag->name);
		StrAllocCat(me->recover, ">");
	    }
	    me->slashedtag = NULL;

	} else if (me->element_stack &&
		   (me->element_stack->tag->flags & Tgf_frecyc)) {
	    /*
	     * The element stack says we are within the contents of an element
	     * that the next stage (HTML.c) may want to feed us back again (via
	     * the *include string).  So try to output text in UTF-8 if
	     * possible, using the same logic as for attribute values (which
	     * should be in line with what me->current_tag_charset
	     * indicates).  - kw
	     */
	    if (me->T.decode_utf8 &&
		*me->U.utf_buf) {
		PUTS(me->U.utf_buf);
		me->U.utf_buf_p = me->U.utf_buf;
		*(me->U.utf_buf_p) = '\0';
	    } else if (!IS_CJK_TTY &&
		       (me->T.output_utf8 ||
			me->T.trans_from_uni)) {
		if (LYIsASCII(clong)) {
		    PUTC(c);
		} else if (clong == UCS_REPL && saved_char_in &&
			   HTPassEightBitRaw &&
			   saved_char_in >=
			   LYlowest_eightbit[me->outUCLYhndl]) {
		    PUTUTF8((UCode_t) (0xf000 | saved_char_in));
		} else {
		    PUTUTF8(clong);
		}
	    } else if (saved_char_in && me->T.use_raw_char_in) {
		PUTC(saved_char_in);
	    } else {
		PUTC(c);
	    }

#define PASS8859SPECL me->T.pass_160_173_raw
	    /*
	     * Convert 160 (nbsp) to Lynx special character if neither
	     * HTPassHighCtrlRaw nor HTCJK is set.  - FM
	     */
	} else if (clong == CH_NBSP &&	/* S/390 -- gil -- 0932 */
		   !me->no_lynx_specialcodes &&
		   !(PASS8859SPECL || IS_CJK_TTY)) {
	    PUTC(HT_NON_BREAK_SPACE);
	    /*
	     * Convert 173 (shy) to Lynx special character if neither
	     * HTPassHighCtrlRaw nor HTCJK is set.  - FM
	     */
	} else if (clong == CH_SHY &&	/* S/390 -- gil -- 0949 */
		   !me->no_lynx_specialcodes &&
		   !(PASS8859SPECL || IS_CJK_TTY)) {
	    PUTC(LY_SOFT_HYPHEN);
	    /*
	     * Handle the case in which we think we have a character which
	     * doesn't need further processing (e.g., a koi8-r input for a
	     * koi8-r output).  - FM
	     */
	} else if (me->T.use_raw_char_in && saved_char_in) {
	    /*
	     * Only if the original character is still in saved_char_in,
	     * otherwise we may be iterating from a goto top.  - KW
	     */
	    PUTC(saved_char_in);
	} else if ((chk = (BOOL) (me->T.trans_from_uni &&
				  TOASCII(clong) >= 160)) &&	/* S/390 -- gil -- 0968 */
		   (uck = UCTransUniChar(clong,
					 me->outUCLYhndl)) >= ' ' &&
		   uck < 256) {
	    CTRACE((tfp, "UCTransUniChar returned 0x%.2" PRI_UCode_t
		    ":'%c'.\n",
		    uck, FROMASCII((char)uck)));
	    /*
	     * We got one octet from the conversions, so use it.  - FM
	     */
	    PUTC(FROMASCII((char) uck));
	} else if ((chk &&
		    (uck == -4 ||
		     (me->T.repl_translated_C0 &&
		      uck > 0 && uck < 32))) &&
	    /*
	     * Not found; look for replacement string.  - KW
	     */
		   (uck = UCTransUniCharStr(replace_buf, 60, clong,
					    me->outUCLYhndl,
					    0) >= 0)) {
	    /*
	     * Got a replacement string.  No further tests for validity -
	     * assume that whoever defined replacement strings knew what she
	     * was doing.  - KW
	     */
	    PUTS(replace_buf);
	    /*
	     * If we're displaying UTF-8, try that now.  - FM
	     */
	} else if (me->T.output_utf8 && PUTUTF8(clong)) {
	    ;			/* do nothing more */
	    /*
	     * If it's any other (> 160) 8-bit character, and we have not set
	     * HTPassEightBitRaw nor HTCJK, nor have the "ISO Latin 1"
	     * character set selected, back translate for our character set.  -
	     * FM
	     */
#define IncludesLatin1Enc \
		(me->outUCLYhndl == LATIN1 || \
		 (me->outUCI && \
		  (me->outUCI->enc & (UCT_CP_SUPERSETOF_LAT1))))

#define PASSHI8BIT (HTPassEightBitRaw || \
		    (me->T.do_8bitraw && !me->T.trans_from_uni))

	} else if (clong > 160 && clong < 256 &&
		   !(PASSHI8BIT || IS_CJK_TTY) &&
		   !IncludesLatin1Enc) {
#ifdef USE_PRETTYSRC
	    int psrc_view_backup = 0;
#endif

	    string->size = 0;
	    EntityName = HTMLGetEntityName((UCode_t) (clong - 160));
	    HTChunkPuts(string, EntityName);
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    /* we need to disable it temporarily */
	    if (psrc_view) {
		psrc_view_backup = 1;
		psrc_view = 0;
	    }
#endif
	    handle_entity(me, '\0');
#ifdef USE_PRETTYSRC
	    /* we need to disable it temporarily */
	    if (psrc_view_backup)
		psrc_view = TRUE;
#endif

	    string->size = 0;
	    if (!FoundEntity)
		PUTC(';');
	    /*
	     * If we get to here and have an ASCII char, pass the character.  -
	     * KW
	     */
	} else if (TOASCII(clong) < 127 && clong > 0) {		/* S/390 -- gil -- 0987 */
	    PUTC(c);
	    /*
	     * If we get to here, and should have translated, translation has
	     * failed so far.  - KW
	     *
	     * We should have sent UTF-8 output to the parser already, but what
	     * the heck, try again.  - FM
	     */
	} else if (me->T.output_utf8 && *me->U.utf_buf) {
	    PUTS(me->U.utf_buf);
	    me->U.utf_buf_p = me->U.utf_buf;
	    *(me->U.utf_buf_p) = '\0';
	    /*
	     * If we don't actually want the character, make it safe and output
	     * that now.  - FM
	     */
	} else if (TOASCII(UCH(c)) <	/* S/390 -- gil -- 0997 */
		   LYlowest_eightbit[me->outUCLYhndl] ||
		   (me->T.trans_from_uni && !HTPassEightBitRaw)) {
	    /*
	     * If we get to here, pass the character.  - FM
	     */
	} else {
	    PUTC(c);
	}
	break;

	/*
	 * Found '<' in SGML_PCDATA content; treat this mode nearly like
	 * S_litteral, but recognize '<!' and '<?' to filter out comments and
	 * processing instructions.  - kw
	 */
    case S_pcdata:
	if (!string->size && TOASCII(clong) < 127) {	/* first after '<' */
	    if (c == '!') {	/* <! */
		/*
		 * Terminate and set up for possible comment, identifier,
		 * declaration, or marked section as under S_tag.  - kw
		 */
		me->state = S_exclamation;
		me->lead_exclamation = TRUE;
		me->doctype_bracket = FALSE;
		me->first_bracket = FALSE;
		HTChunkPutc(string, c);
		break;
	    } else if (c == '?') {	/* <? - ignore as a PI until '>' - kw */
		CTRACE((tfp,
			"SGML: Found PI in PCDATA, junking it until '>'\n"));
#ifdef USE_PRETTYSRC
		if (psrc_view) {
		    PSRCSTART(abracket);
		    PUTS("<?");
		    PSRCSTOP(abracket);
		}
#endif
		me->state = S_pi;
		break;
	    }
	}
	goto case_S_litteral;

	/*
	 * Found '<' in SGML_SCRIPT content; treat this mode nearly like
	 * S_litteral, but recognize '<!' to allow the content to be treated as
	 * a comment by lynx.
	 */
    case S_script:
	if (!string->size && TOASCII(clong) < 127) {	/* first after '<' */
	    if (c == '!') {	/* <! */
		/*
		 * Terminate and set up for possible comment, identifier,
		 * declaration, or marked section as under S_tag.  - kw
		 */
		me->state = S_exclamation;
		me->lead_exclamation = TRUE;
		me->doctype_bracket = FALSE;
		me->first_bracket = FALSE;
		HTChunkPutc(string, c);
		break;
	    }
	}
	goto case_S_litteral;

	/*
	 * In literal mode, waits only for specific end tag (for compatibility
	 * with old servers, and for Lynx).  - FM
	 */
      case_S_litteral:
    case S_litteral:
	/*PSRC:this case not understood completely by HV, not done */
	HTChunkPutc(string, c);
#ifdef USE_PRETTYSRC
	if (psrc_view) {
	    /* there is nothing useful in the element_stack */
	    testtag = me->current_tag;
	} else
#endif
	    testtag = (me->element_stack
		       ? me->element_stack->tag
		       : NULL);

	if (testtag == NULL || testtag->name == NULL) {
	    string->size--;
	    me->state = S_text;
	    goto top1;
	}

	/*
	 * Normally when we get the closing ">",
	 *      testtag contains something like "TITLE"
	 *      string contains something like "/title>"
	 * so we decrement by 2 to compare the final character of each.
	 */
	testlast = string->size - 2 - me->trailing_spaces - me->leading_spaces;

#ifdef USE_COLOR_STYLE
#define TagSize(p) ((p)->name_len)
#else
#define TagSize(p) (strlen((p)->name))
#endif

	if (TOUPPER(c) != ((testlast < 0)
			   ? '/'
			   : ((testlast < (int) TagSize(testtag))
			      ? testtag->name[testlast]
			      : 0))) {
	    int i;

	    /*
	     * If complete match, end literal.
	     */
	    if ((c == '>') &&
		testlast >= 0 && !testtag->name[testlast]) {
#ifdef USE_PRETTYSRC
		if (psrc_view) {
		    char *trailing = NULL;

		    if (me->trailing_spaces) {
			StrAllocCopy(trailing,
				     string->data
				     + string->size
				     - 1
				     - me->trailing_spaces);
			trailing[me->trailing_spaces] = '\0';
		    }

		    PSRCSTART(abracket);
		    PUTS("</");
		    PSRCSTOP(abracket);
		    PSRCSTART(tag);

		    strcpy(string->data, me->current_tag->name);
		    transform_tag(me, string);
		    PUTS(string->data);

		    if (trailing) {
			PUTS(trailing);
			FREE(trailing);
		    }

		    PSRCSTOP(tag);
		    PSRCSTART(abracket);
		    PUTC('>');
		    PSRCSTOP(abracket);

		    me->current_tag = NULL;
		} else
#endif
		    end_element(me, me->element_stack->tag);

		string->size = 0;
		me->current_attribute_number = INVALID;
		me->state = S_text;
		me->leading_spaces = 0;
		me->trailing_spaces = 0;
		break;
	    }

	    /*
	     * Allow whitespace between the "<" or ">" and the keyword, for
	     * error-recovery.
	     */
	    if (isspace(UCH(c))) {
		if (testlast == -1) {
		    me->leading_spaces += 1;
		    CTRACE2(TRACE_SGML, (tfp, "leading spaces: %d\n", me->leading_spaces));
		    break;
		} else if (testlast > 0) {
		    me->trailing_spaces += 1;
		    CTRACE2(TRACE_SGML, (tfp, "trailing spaces: %d\n", me->trailing_spaces));
		    break;
		}
	    }

	    /*
	     * Mismatch - recover.
	     */
	    me->leading_spaces = 0;
	    me->trailing_spaces = 0;
	    if (((testtag->contents != SGML_LITTERAL &&
		  (testtag->flags & Tgf_strict)) ||
		 (me->state == S_pcdata &&
		  (testtag->flags & (Tgf_strict | Tgf_endO)))) &&
		(testlast > -1 &&
		 (c == '>' || testlast > 0 || IsNmStart(c)))) {
		me->state = S_end;
		string->size--;
		for (i = 0; i < string->size; i++)	/* remove '/' */
		    string->data[i] = string->data[i + 1];
		if ((string->size == 1) ? IsNmStart(c) : IsNmChar(c))
		    break;
		string->size--;
		goto top1;
	    }
	    if (me->state == S_pcdata &&
		(testtag->flags & (Tgf_strict | Tgf_endO)) &&
		(testlast < 0 && IsNmStart(c))) {
		me->state = S_tag;
		break;
	    }
	    /*
	     * If Mismatch:  recover string literally.
	     */
	    PUTC('<');
	    for (i = 0; i < string->size - 1; i++)	/* recover, except last c */
		PUTC(string->data[i]);
	    string->size = 0;
	    me->state = S_text;
	    goto top1;		/* to recover last c */
	}
	break;

	/*
	 * Character reference (numeric entity) or named entity.
	 */
    case S_ero:
	if (c == '#') {
	    /*
	     * Setting up for possible numeric entity.
	     */
	    me->state = S_cro;	/* &# is Char Ref Open */
	    break;
	}
	me->state = S_entity;	/* Fall through! */

	/*
	 * Handle possible named entity.
	 */
    case S_entity:
	if (TOASCII(clong) < 127 && (string->size ?	/* S/390 -- gil -- 1029 */
				     isalnum(UCH(c)) : isalpha(UCH(c)))) {
	    /* Should probably use IsNmStart/IsNmChar above (is that right?),
	       but the world is not ready for that - there's &nbsp: (note
	       colon!) and stuff around. */
	    /*
	     * Accept valid ASCII character.  - FM
	     */
	    HTChunkPutc(string, c);
	} else if (string->size == 0) {
	    /*
	     * It was an ampersand that's just text, so output the ampersand
	     * and recycle this character.  - FM
	     */
#ifdef USE_PRETTYSRC
	    if (psrc_view)
		PSRCSTART(badseq);
#endif
	    PUTC('&');
#ifdef USE_PRETTYSRC
	    if (psrc_view)
		PSRCSTOP(badseq);
#endif
	    me->state = S_text;
	    goto top1;
	} else {
	    /*
	     * Terminate entity name and try to handle it.  - FM
	     */
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    entity_string = string->data;
#endif
	    if (!strcmp(string->data, "zwnj") &&
		(!me->element_stack ||
		 (me->element_stack->tag &&
		  me->element_stack->tag->contents == SGML_MIXED))) {
		/*
		 * Handle zwnj (8204) as <WBR>.  - FM
		 */
		char temp[8];

		CTRACE((tfp,
			"SGML_character: Handling 'zwnj' entity as 'WBR' element.\n"));

		if (c != ';') {
		    sprintf(temp, "<WBR>%c", c);
		} else {
		    sprintf(temp, "<WBR>");
		}
		if (me->recover == NULL) {
		    StrAllocCopy(me->recover, temp);
		    me->recover_index = 0;
		} else {
		    StrAllocCat(me->recover, temp);
		}
		string->size = 0;
		me->state = S_text;
		break;
	    } else {
		handle_entity(me, '\0');
	    }
	    string->size = 0;
	    me->state = S_text;
	    /*
	     * Don't eat the terminator if we didn't find the entity name and
	     * therefore sent the raw string via handle_entity(), or if the
	     * terminator is not the "standard" semi-colon for HTML.  - FM
	     */
#ifdef USE_PRETTYSRC
	    if (psrc_view && FoundEntity && c == ';') {
		PSRCSTART(entity);
		PUTC(c);
		PSRCSTOP(entity);
	    }
#endif
	    if (!FoundEntity || c != ';')
		goto top1;
	}
	break;

	/*
	 * Check for a numeric entity.
	 */
    case S_cro:
	if (TOASCII(clong) < 127 && TOLOWER(UCH(c)) == 'x') {	/* S/390 -- gil -- 1060 */
	    me->isHex = TRUE;
	    me->state = S_incro;
	} else if (TOASCII(clong) < 127 && isdigit(UCH(c))) {
	    /*
	     * Accept only valid ASCII digits.  - FM
	     */
	    HTChunkPutc(string, c);	/* accumulate a character NUMBER */
	    me->isHex = FALSE;
	    me->state = S_incro;
	} else if (string->size == 0) {
	    /*
	     * No 'x' or digit following the "&#" so recover them and recycle
	     * the character.  - FM
	     */
#ifdef USE_PRETTYSRC
	    if (psrc_view)
		PSRCSTART(badseq);
#endif
	    PUTC('&');
	    PUTC('#');
#ifdef USE_PRETTYSRC
	    if (psrc_view)
		PSRCSTOP(badseq);
#endif
	    me->state = S_text;
	    goto top1;
	}
	break;

	/*
	 * Handle a numeric entity.
	 */
    case S_incro:
	/* S/390 -- gil -- 1075 */
	if ((TOASCII(clong) < 127) &&
	    (me->isHex
	     ? isxdigit(UCH(c))
	     : isdigit(UCH(c)))) {
	    /*
	     * Accept only valid hex or ASCII digits.  - FM
	     */
	    HTChunkPutc(string, c);	/* accumulate a character NUMBER */
	} else if (string->size == 0) {
	    /*
	     * No hex digit following the "&#x" so recover them and recycle the
	     * character.  - FM
	     */
#ifdef USE_PRETTYSRC
	    if (psrc_view)
		PSRCSTART(badseq);
#endif
	    PUTS("&#x");
#ifdef USE_PRETTYSRC
	    if (psrc_view)
		PSRCSTOP(badseq);
#endif
	    me->isHex = FALSE;
	    me->state = S_text;
	    goto top1;
	} else {
	    /*
	     * Terminate the numeric entity and try to handle it.  - FM
	     */
	    UCode_t code;
	    int i;

	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    entity_string = string->data;
#endif
	    if (UCScanCode(&code, string->data, me->isHex)) {

/* =============== work in ASCII below here ===============  S/390 -- gil -- 1092 */
		if (AssumeCP1252(me)) {
		    code = LYcp1252ToUnicode(code);
		}
		/*
		 * Check for special values.  - FM
		 */
		if ((code == 8204) &&
		    (!me->element_stack ||
		     (me->element_stack->tag &&
		      me->element_stack->tag->contents == SGML_MIXED))) {
		    /*
		     * Handle zwnj (8204) as <WBR>.  - FM
		     */
		    char temp[8];

		    CTRACE((tfp,
			    "SGML_character: Handling '8204' (zwnj) reference as 'WBR' element.\n"));

		    /*
		     * Include the terminator if it is not the standard
		     * semi-colon.  - FM
		     */
		    if (c != ';') {
			sprintf(temp, "<WBR>%c", c);
		    } else {
			sprintf(temp, "<WBR>");
		    }
		    /*
		     * Add the replacement string to the recover buffer for
		     * processing.  - FM
		     */
		    if (me->recover == NULL) {
			StrAllocCopy(me->recover, temp);
			me->recover_index = 0;
		    } else {
			StrAllocCat(me->recover, temp);
		    }
		    string->size = 0;
		    me->isHex = FALSE;
		    me->state = S_text;
		    break;
		} else if (put_special_unicodes(me, code)) {
		    /*
		     * We handled the value as a special character, so recycle
		     * the terminator or break.  - FM
		     */
#ifdef USE_PRETTYSRC
		    if (psrc_view) {
			PSRCSTART(entity);
			PUTS((me->isHex ? "&#x" : "&#"));
			PUTS(entity_string);
			if (c == ';')
			    PUTC(';');
			PSRCSTOP(entity);
		    }
#endif
		    string->size = 0;
		    me->isHex = FALSE;
		    me->state = S_text;
		    if (c != ';')
			goto top1;
		    break;
		}
		/*
		 * Seek a translation from the chartrans tables.
		 */
		if ((uck = UCTransUniChar(code,
					  me->outUCLYhndl)) >= 32 &&
		    uck < 256 &&
		    (uck < 127 ||
		     uck >= LYlowest_eightbit[me->outUCLYhndl])) {
#ifdef USE_PRETTYSRC
		    if (!psrc_view) {
#endif
			PUTC(FROMASCII((char) uck));
#ifdef USE_PRETTYSRC
		    } else {
			put_pretty_number(me);
		    }
#endif
		} else if ((uck == -4 ||
			    (me->T.repl_translated_C0 &&
			     uck > 0 && uck < 32)) &&
		    /*
		     * Not found; look for replacement string.
		     */
			   (uck = UCTransUniCharStr(replace_buf, 60, code,
						    me->outUCLYhndl,
						    0) >= 0)) {
#ifdef USE_PRETTYSRC
		    if (psrc_view) {
			put_pretty_number(me);
		    } else
#endif
			PUTS(replace_buf);
		    /*
		     * If we're displaying UTF-8, try that now.  - FM
		     */
		} else if (me->T.output_utf8 && PUTUTF8(code)) {
		    ;		/* do nothing more */
		    /*
		     * Ignore 8205 (zwj), 8206 (lrm), and 8207 (rln), if we get
		     * to here.  - FM
		     */
		} else if (code == 8205 ||
			   code == 8206 ||
			   code == 8207) {
		    if (TRACE) {
			string->size--;
			LYStrNCpy(replace_buf,
				  string->data,
				  (string->size < 64 ? string->size : 63));
			fprintf(tfp,
				"SGML_character: Ignoring '%s%s'.\n",
				(me->isHex ? "&#x" : "&#"),
				replace_buf);
		    }
#ifdef USE_PRETTYSRC
		    if (psrc_view) {
			PSRCSTART(badseq);
			PUTS((me->isHex ? "&#x" : "&#"));
			PUTS(entity_string);
			if (c == ';')
			    PUTC(';');
			PSRCSTOP(badseq);
		    }
#endif
		    string->size = 0;
		    me->isHex = FALSE;
		    me->state = S_text;
		    if (c != ';')
			goto top1;
		    break;
		    /*
		     * Show the numeric entity if we get to here and the value:
		     * (1) Is greater than 255 (but use ASCII characters for
		     * spaces or dashes).
		     * (2) Is less than 32, and not valid or we don't have
		     * HTCJK set.
		     * (3) Is 127 and we don't have HTPassHighCtrlRaw or HTCJK
		     * set.
		     * (4) Is 128 - 159 and we don't have HTPassHighCtrlNum
		     * set.
		     * - FM
		     */
		} else if ((code > 255) ||
			   (code < ' ' &&	/* S/390 -- gil -- 1140 */
			    code != '\t' && code != '\n' && code != '\r' &&
			    !IS_CJK_TTY) ||
			   (TOASCII(code) == 127 &&
			    !(HTPassHighCtrlRaw || IS_CJK_TTY)) ||
			   (TOASCII(code) > 127 && code < 160 &&
			    !HTPassHighCtrlNum)) {
		    /*
		     * Unhandled or illegal value.  Recover the "&#" or "&#x"
		     * and digit(s), and recycle the terminator.  - FM
		     */
#ifdef USE_PRETTYSRC
		    if (psrc_view) {
			PSRCSTART(badseq);
		    }
#endif
		    if (me->isHex) {
			PUTS("&#x");
			me->isHex = FALSE;
		    } else {
			PUTS("&#");
		    }
		    string->size--;
		    for (i = 0; i < string->size; i++)	/* recover */
			PUTC(string->data[i]);
#ifdef USE_PRETTYSRC
		    if (psrc_view) {
			PSRCSTOP(badseq);
		    }
#endif
		    string->size = 0;
		    me->isHex = FALSE;
		    me->state = S_text;
		    goto top1;
		} else if (TOASCII(code) < 161 ||	/* S/390 -- gil -- 1162 */
			   HTPassEightBitNum ||
			   IncludesLatin1Enc) {
		    /*
		     * No conversion needed.  - FM
		     */
#ifdef USE_PRETTYSRC
		    if (psrc_view) {
			put_pretty_number(me);
		    } else
#endif
			PUTC(FROMASCII((char) code));
		} else {
		    /*
		     * Handle as named entity.  - FM
		     */
		    code -= 160;
		    EntityName = HTMLGetEntityName(code);
		    if (EntityName && EntityName[0] != '\0') {
			string->size = 0;
			HTChunkPuts(string, EntityName);
			HTChunkTerminate(string);
			handle_entity(me, '\0');
			/*
			 * Add a semi-colon if something went wrong and
			 * handle_entity() sent the string.  - FM
			 */
			if (!FoundEntity) {
			    PUTC(';');
			}
		    } else {
			/*
			 * Our conversion failed, so recover the "&#" and
			 * digit(s), and recycle the terminator.  - FM
			 */
#ifdef USE_PRETTYSRC
			if (psrc_view)
			    PSRCSTART(badseq);
#endif
			if (me->isHex) {
			    PUTS("&#x");
			    me->isHex = FALSE;
			} else {
			    PUTS("&#");
			}
			string->size--;
			for (i = 0; i < string->size; i++)	/* recover */
			    PUTC(string->data[i]);
#ifdef USE_PRETTYSRC
			if (psrc_view)
			    PSRCSTOP(badseq);
#endif
			string->size = 0;
			me->isHex = FALSE;
			me->state = S_text;
			goto top1;
		    }
		}
		/*
		 * If we get to here, we succeeded.  Hoorah!!!  - FM
		 */
		string->size = 0;
		me->isHex = FALSE;
		me->state = S_text;
		/*
		 * Don't eat the terminator if it's not the "standard"
		 * semi-colon for HTML.  - FM
		 */
		if (c != ';') {
		    goto top1;
		}
	    } else {
		/*
		 * Not an entity, and don't know why not, so add the terminator
		 * to the string, output the "&#" or "&#x", and process the
		 * string via the recover element.  - FM
		 */
		string->size--;
		HTChunkPutc(string, c);
		HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
		if (psrc_view)
		    PSRCSTART(badseq);
#endif
		if (me->isHex) {
		    PUTS("&#x");
		    me->isHex = FALSE;
		} else {
		    PUTS("&#");
		}
#ifdef USE_PRETTYSRC
		if (psrc_view)
		    PSRCSTOP(badseq);
#endif
		if (me->recover == NULL) {
		    StrAllocCopy(me->recover, string->data);
		    me->recover_index = 0;
		} else {
		    StrAllocCat(me->recover, string->data);
		}
		string->size = 0;
		me->isHex = FALSE;
		me->state = S_text;
		break;
	    }
	}
	break;

	/*
	 * Tag
	 */
    case S_tag:		/* new tag */
	if (TOASCII(clong) < 127 && (string->size ?	/* S/390 -- gil -- 1179 */
				     IsNmChar(c) : IsNmStart(c))) {
	    /*
	     * Add valid ASCII character.  - FM
	     */
	    HTChunkPutc(string, c);
	} else if (c == '!' && !string->size) {		/* <! */
	    /*
	     * Terminate and set up for possible comment, identifier,
	     * declaration, or marked section.  - FM
	     */
	    me->state = S_exclamation;
	    me->lead_exclamation = TRUE;
	    me->doctype_bracket = FALSE;
	    me->first_bracket = FALSE;
	    HTChunkPutc(string, c);
	    break;
	} else if (!string->size &&
		   (TOASCII(clong) <= 160 &&	/* S/390 -- gil -- 1196 */
		    (c != '/' && c != '?' && c != '_' && c != ':'))) {
	    /*
	     * '<' must be followed by an ASCII letter to be a valid start tag. 
	     * Here it isn't, nor do we have a '/' for an end tag, nor one of
	     * some other characters with a special meaning for SGML or which
	     * are likely to be legal Name Start characters in XML or some
	     * other extension.  So recover the '<' and following character as
	     * data.  - FM & KW
	     */
	    me->state = S_text;
#ifdef USE_PRETTYSRC
	    if (psrc_view)
		PSRCSTART(badseq);
#endif
	    PUTC('<');
#ifdef USE_PRETTYSRC
	    if (psrc_view)
		PSRCSTOP(badseq);
#endif
	    goto top1;
	} else {		/* End of tag name */
	    /*
	     * Try to handle tag.  - FM
	     */
	    HTTag *t;

	    if (c == '/') {
		if (string->size == 0) {
		    me->state = S_end;
		    break;
		}
		CTRACE((tfp, "SGML: `<%.*s/' found!\n", string->size, string->data));
	    }
	    HTChunkTerminate(string);

	    t = SGMLFindTag(dtd, string->data);
	    if (t == me->unknown_tag &&
		((c == ':' &&
		  string->size == 4 && 0 == strcasecomp(string->data, "URL")) ||
		 (string->size > 4 && 0 == strncasecomp(string->data, "URL:", 4)))) {
		/*
		 * Treat <URL:  as text rather than a junk tag, so we display
		 * it and the URL (Lynxism 8-).  - FM
		 */
#ifdef USE_PRETTYSRC
		if (psrc_view)
		    PSRCSTART(badseq);
#endif
		PUTC('<');
		PUTS(string->data);	/* recover */
		PUTC(c);
#ifdef USE_PRETTYSRC
		if (psrc_view)
		    PSRCSTOP(badseq);
#endif
		CTRACE((tfp, "SGML: Treating <%s%c as text\n",
			string->data, c));
		string->size = 0;
		me->state = S_text;
		break;
	    }
	    if (c == '/' && t) {
		/*
		 * Element name was ended by '/'.  Remember the tag that ended
		 * thusly, we'll interpret this as either an indication of an
		 * empty element (if '>' follows directly) or do some
		 * SGMLshortref-ish treatment.  - kw
		 */
		me->slashedtag = t;
	    }
	    if (!t) {
		if (c == '?' && string->size <= 1) {
		    CTRACE((tfp, "SGML: Found PI, looking for '>'\n"));
#ifdef USE_PRETTYSRC
		    if (psrc_view) {
			PSRCSTART(abracket);
			PUTS("<?");
			PSRCSTOP(abracket);
		    }
#endif
		    string->size = 0;
		    me->state = S_pi;
		    HTChunkPutc(string, c);
		    break;
		}
		CTRACE((tfp, "SGML: *** Invalid element %s\n",
			string->data));

#ifdef USE_PRETTYSRC
		if (psrc_view) {
		    PSRCSTART(abracket);
		    PUTC('<');
		    PSRCSTOP(abracket);
		    PSRCSTART(badtag);
		    transform_tag(me, string);
		    PUTS(string->data);
		    if (c == '>') {
			PSRCSTOP(badtag);
			PSRCSTART(abracket);
			PUTC('>');
			PSRCSTOP(abracket);
		    } else {
			PUTC(c);
		    }
		}
#endif
		me->state = (c == '>') ? S_text : S_junk_tag;
		break;
	    } else if (t == me->unknown_tag) {
		CTRACE((tfp, "SGML: *** Unknown element \"%s\"\n",
			string->data));
		/*
		 * Fall through and treat like valid tag for attribute parsing. 
		 * - KW
		 */

	    }
	    me->current_tag = t;

#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(abracket);
		PUTC('<');
		PSRCSTOP(abracket);
		if (t != me->unknown_tag)
		    PSRCSTART(tag);
		else
		    PSRCSTART(badtag);
		transform_tag(me, string);
		PUTS(string->data);
		if (t != me->unknown_tag)
		    PSRCSTOP(tag);
		else
		    PSRCSTOP(badtag);
	    }
	    if (!psrc_view)	/*don't waste time */
#endif
	    {
		/*
		 * Clear out attributes.
		 */
		memset((void *) me->present, 0, sizeof(BOOL) *
		         (unsigned) (me->current_tag->number_of_attributes));
	    }

	    string->size = 0;
	    me->current_attribute_number = INVALID;
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		if (c == '>' || c == '<' || (c == '/' && me->slashedtag)) {
		    if (c != '<') {
			PSRCSTART(abracket);
			PUTC(c);
			PSRCSTOP(abracket);
			me->state = (c == '>') ? S_text : S_tagname_slash;
		    } else {
			me->state = S_tag;
		    }
		} else {
		    if (!WHITE(c))
			PUTC(c);
		    me->state = S_tag_gap;
		}
	    } else
#endif
	    if (c == '>' || c == '<' || (c == '/' && me->slashedtag)) {
		if (me->current_tag->name)
		    start_element(me);
		me->state = (c == '>') ? S_text :
		    (c == '<') ? S_tag : S_tagname_slash;
	    } else {
		me->state = S_tag_gap;
	    }
	}
	break;

    case S_exclamation:
	if (me->lead_exclamation && c == '-') {
	    /*
	     * Set up for possible comment.  - FM
	     */
	    me->lead_exclamation = FALSE;
	    me->first_dash = TRUE;
	    HTChunkPutc(string, c);
	    break;
	}
	if (me->lead_exclamation && c == '[') {
	    /*
	     * Set up for possible marked section.  - FM
	     */
	    me->lead_exclamation = FALSE;
	    me->first_bracket = TRUE;
	    me->second_bracket = FALSE;
	    HTChunkPutc(string, c);
	    me->state = S_marked;
	    break;
	}
	if (me->first_dash && c == '-') {
	    /*
	     * Set up to handle comment.  - FM
	     */
	    me->lead_exclamation = FALSE;
	    me->first_dash = FALSE;
	    me->end_comment = FALSE;
	    HTChunkPutc(string, c);
	    me->state = S_comment;
	    break;
	}
	me->lead_exclamation = FALSE;
	me->first_dash = FALSE;
	if (c == '>') {
	    /*
	     * Try to handle identifier.  - FM
	     */
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(sgmlspecial);
		PUTC('<');
		PUTS(string->data);
		PUTC('>');
		PSRCSTOP(sgmlspecial);
	    } else
#endif
		handle_identifier(me);
	    string->size = 0;
	    me->state = S_text;
	    break;
	}
	if (WHITE(c)) {
	    if (string->size == 8 &&
		!strncasecomp(string->data, "!DOCTYPE", 8)) {
		/*
		 * Set up for DOCTYPE declaration.  - FM
		 */
		HTChunkPutc(string, c);
		me->doctype_bracket = FALSE;
		me->state = S_doctype;
		break;
	    }
	    if (string->size == 7 &&
		!strncasecomp(string->data, "!ENTITY", 7)) {
		/*
		 * Set up for ENTITY declaration.  - FM
		 */
		HTChunkPutc(string, c);
		me->first_dash = FALSE;
		me->end_comment = TRUE;
		me->state = S_sgmlent;
		break;
	    }
	    if (string->size == 8 &&
		!strncasecomp(string->data, "!ELEMENT", 8)) {
		/*
		 * Set up for ELEMENT declaration.  - FM
		 */
		HTChunkPutc(string, c);
		me->first_dash = FALSE;
		me->end_comment = TRUE;
		me->state = S_sgmlele;
		break;
	    }
	    if (string->size == 8 &&
		!strncasecomp(string->data, "!ATTLIST", 8)) {
		/*
		 * Set up for ATTLIST declaration.  - FM
		 */
		HTChunkPutc(string, c);
		me->first_dash = FALSE;
		me->end_comment = TRUE;
		me->state = S_sgmlatt;
		break;
	    }
	}
	HTChunkPutc(string, c);
	break;

    case S_comment:		/* Expecting comment. - FM */
	if (historical_comments) {
	    /*
	     * Any '>' terminates.  - FM
	     */
	    if (c == '>') {
		HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
		if (psrc_view) {
		    PSRCSTART(comm);
		    PUTC('<');
		    PUTS_TR(string->data);
		    PUTC('>');
		    PSRCSTOP(comm);
		} else
#endif
		    handle_comment(me);
		string->size = 0;
		me->end_comment = FALSE;
		me->first_dash = FALSE;
		me->state = S_text;
		break;
	    }
	    goto S_comment_put_c;
	}
	if (!me->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    me->first_dash = TRUE;
	    break;
	}
	if (me->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    me->first_dash = FALSE;
	    if (!me->end_comment)
		me->end_comment = TRUE;
	    else if (!minimal_comments)
		/*
		 * Validly treat '--' pairs as successive comments (for
		 * minimal, any "--WHITE>" terminates).  - FM
		 */
		me->end_comment = FALSE;
	    break;
	}
	if (me->end_comment && c == '>') {
	    /*
	     * Terminate and handle the comment.  - FM
	     */
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(comm);
		PUTC('<');
		PUTS_TR(string->data);
		PUTC('>');
		PSRCSTOP(comm);
	    } else
#endif
		handle_comment(me);
	    string->size = 0;
	    me->end_comment = FALSE;
	    me->first_dash = FALSE;
	    me->state = S_text;
	    break;
	}
	me->first_dash = FALSE;
	if (me->end_comment && !isspace(UCH(c)))
	    me->end_comment = FALSE;

      S_comment_put_c:
	if (me->T.decode_utf8 &&
	    *me->U.utf_buf) {
	    HTChunkPuts(string, me->U.utf_buf);
	    me->U.utf_buf_p = me->U.utf_buf;
	    *(me->U.utf_buf_p) = '\0';
	} else if (!IS_CJK_TTY &&
		   (me->T.output_utf8 ||
		    me->T.trans_from_uni)) {
	    if (clong == UCS_REPL && saved_char_in &&
		HTPassEightBitRaw &&
		saved_char_in >=
		LYlowest_eightbit[me->outUCLYhndl]) {
		(HTChunkPutUtf8Char) (string,
				      (UCode_t) (0xf000 | saved_char_in));
	    } else {
		HTChunkPutUtf8Char(string, clong);
	    }
	} else if (saved_char_in && me->T.use_raw_char_in) {
	    HTChunkPutc(string, saved_char_in);
	} else {
	    HTChunkPutc(string, c);
	}
	break;

    case S_doctype:		/* Expecting DOCTYPE. - FM */
	if (me->doctype_bracket) {
	    HTChunkPutc(string, c);
	    if (c == ']')
		me->doctype_bracket = FALSE;
	    break;
	}
	if (c == '[' && WHITE(string->data[string->size - 1])) {
	    HTChunkPutc(string, c);
	    me->doctype_bracket = TRUE;
	    break;
	}
	if (c == '>') {
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(sgmlspecial);
		PUTC('<');
		PUTS(string->data);
		PUTC('>');
		PSRCSTOP(sgmlspecial);
	    } else
#endif
		handle_doctype(me);
	    string->size = 0;
	    me->state = S_text;
	    break;
	}
	HTChunkPutc(string, c);
	break;

    case S_marked:		/* Expecting marked section. - FM */
	if (me->first_bracket && c == '[') {
	    HTChunkPutc(string, c);
	    me->first_bracket = FALSE;
	    me->second_bracket = TRUE;
	    break;
	}
	if (me->second_bracket && c == ']' &&
	    string->data[string->size - 1] == ']') {
	    HTChunkPutc(string, c);
	    me->second_bracket = FALSE;
	    break;
	}
	if (!me->second_bracket && c == '>') {
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(sgmlspecial);
		PUTC('<');
		PUTS(string->data);
		PUTC('>');
		PSRCSTOP(sgmlspecial);
	    } else
#endif
		handle_marked(me);
	    string->size = 0;
	    me->state = S_text;
	    break;
	}

	if (me->T.decode_utf8) {
	    HTChunkPutUtf8Char(string, clong);
	} else {
	    HTChunkPutc(string, c);
	}
	break;
    case S_sgmlent:		/* Expecting ENTITY. - FM */
	if (!me->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    me->first_dash = TRUE;
	    break;
	}
	if (me->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    me->first_dash = FALSE;
	    if (!me->end_comment)
		me->end_comment = TRUE;
	    else
		me->end_comment = FALSE;
	    break;
	}
	if (me->end_comment && c == '>') {
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(sgmlspecial);
		PUTC('<');
		PUTS(string->data);
		PUTC('>');
		PSRCSTOP(sgmlspecial);
	    } else
#endif
		handle_sgmlent(me);
	    string->size = 0;
	    me->end_comment = FALSE;
	    me->first_dash = FALSE;
	    me->state = S_text;
	    break;
	}
	me->first_dash = FALSE;
	HTChunkPutc(string, c);
	break;

    case S_sgmlele:		/* Expecting ELEMENT. - FM */
	if (!me->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    me->first_dash = TRUE;
	    break;
	}
	if (me->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    me->first_dash = FALSE;
	    if (!me->end_comment)
		me->end_comment = TRUE;
	    else
		me->end_comment = FALSE;
	    break;
	}
	if (me->end_comment && c == '>') {
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(sgmlspecial);
		PUTC('<');
		PUTS(string->data);
		PUTC('>');
		PSRCSTOP(sgmlspecial);
	    } else
#endif
		handle_sgmlele(me);
	    string->size = 0;
	    me->end_comment = FALSE;
	    me->first_dash = FALSE;
	    me->state = S_text;
	    break;
	}
	me->first_dash = FALSE;
	HTChunkPutc(string, c);
	break;

    case S_sgmlatt:		/* Expecting ATTLIST. - FM */
	if (!me->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    me->first_dash = TRUE;
	    break;
	}
	if (me->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    me->first_dash = FALSE;
	    if (!me->end_comment)
		me->end_comment = TRUE;
	    else
		me->end_comment = FALSE;
	    break;
	}
	if (me->end_comment && c == '>') {
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(sgmlspecial);
		PUTC('<');
		PUTS(string->data);
		PUTC('>');
		PSRCSTOP(sgmlspecial);
	    } else
#endif
		handle_sgmlatt(me);
	    string->size = 0;
	    me->end_comment = FALSE;
	    me->first_dash = FALSE;
	    me->state = S_text;
	    break;
	}
	me->first_dash = FALSE;
	HTChunkPutc(string, c);
	break;

    case S_tag_gap:		/* Expecting attribute or '>' */
	if (WHITE(c)) {
	    /* PUTC(c); - no, done as special case */
	    break;		/* Gap between attributes */
	}
	if (c == '>') {		/* End of tag */
#ifdef USE_PRETTYSRC
	    if (!psrc_view)
#endif
		if (me->current_tag->name)
		    start_element(me);
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(abracket);
		PUTC('>');
		PSRCSTOP(abracket);
	    }
#endif
	    me->state = S_text;
	    break;
	}
	HTChunkPutc(string, c);
	me->state = S_attr;	/* Get attribute */
	break;

	/* accumulating value */
    case S_attr:
	if (WHITE(c) || (c == '>') || (c == '=')) {	/* End of word */
	    if ((c == '>')
		&& (string->size >= 1)
		&& (string->data[string->size - 1] == '/')) {
		if ((LYxhtml_parsing || me->extended_html)
		    && ignore_when_empty(me->current_tag)) {
		    discard_empty(me);
		} else {
		    HTChunkTerminate(string);
		}
	    } else {
		HTChunkTerminate(string);
		handle_attribute_name(me, string->data);
	    }
#ifdef USE_PRETTYSRC
	    if (!psrc_view) {
#endif
		string->size = 0;
		if (c == '>') {	/* End of tag */
		    if (me->current_tag->name)
			start_element(me);
		    me->state = S_text;
		    break;
		}
#ifdef USE_PRETTYSRC
	    } else {
		PUTC(' ');
		if (me->current_attribute_number == INVALID)
		    PSRCSTART(badattr);
		else
		    PSRCSTART(attrib);
		if (attrname_transform != 1) {
		    if (attrname_transform == 0)
			LYLowerCase(string->data);
		    else
			LYUpperCase(string->data);
		}
		PUTS(string->data);
		if (c == '=' || WHITE(c))
		    PUTC(c);
		if (c == '=' || c == '>') {
		    if (me->current_attribute_number == INVALID) {
			PSRCSTOP(badattr);
		    } else {
			PSRCSTOP(attrib);
		    }
		}
		if (c == '>') {
		    PSRCSTART(abracket);
		    PUTC('>');
		    PSRCSTOP(abracket);
		    me->state = S_text;
		    break;
		}
		string->size = 0;
	    }
#endif
	    me->state = (c == '=' ? S_equals : S_attr_gap);
	} else {
	    HTChunkPutc(string, c);
	}
	break;

    case S_attr_gap:		/* Expecting attribute or '=' or '>' */
	if (WHITE(c)) {
	    PRETTYSRC_PUTC(c);
	    break;		/* Gap after attribute */
	}
	if (c == '>') {		/* End of tag */
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		if (me->current_attribute_number == INVALID) {
		    PSRCSTOP(badattr);
		} else {
		    PSRCSTOP(attrib);
		}
		PSRCSTART(abracket);
		PUTC('>');
		PSRCSTOP(abracket);
	    } else
#endif
	    if (me->current_tag->name)
		start_element(me);
	    me->state = S_text;
	    break;
	} else if (c == '=') {
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PUTC('=');
		if (me->current_attribute_number == INVALID) {
		    PSRCSTOP(badattr);
		} else {
		    PSRCSTOP(attrib);
		}
	    }
#endif
	    me->state = S_equals;
	    break;
	}
	HTChunkPutc(string, c);
	me->state = S_attr;	/* Get next attribute */
	break;

    case S_equals:		/* After attr = */
	if (WHITE(c)) {
	    PRETTYSRC_PUTC(c);
	    break;		/* Before attribute value */
	}
	if (c == '>') {		/* End of tag */
	    CTRACE((tfp, "SGML: found = but no value\n"));
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(abracket);
		PUTC('>');
		PSRCSTOP(abracket);
	    } else
#endif
	    if (me->current_tag->name)
		start_element(me);
	    me->state = S_text;
	    break;

	} else if (c == '\'') {
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(attrval);
		PUTC(c);
	    }
#endif
	    me->state = S_squoted;
	    break;

	} else if (c == '"') {
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PSRCSTART(attrval);
		PUTC(c);
	    }
#endif
	    me->state = S_dquoted;
	    break;
	}
#ifdef USE_PRETTYSRC
	if (psrc_view)
	    PSRCSTART(attrval);
#endif
	me->state = S_value;
	/* FALLTHRU */

    case S_value:
	if (WHITE(c) || (c == '>')) {	/* End of word */
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (!end_if_prettysrc(me, string, 0))
#endif
	    {
#ifdef CJK_EX			/* Quick hack. - JH7AYN */
		if (IS_CJK_TTY) {
		    if (string->data[0] == '$') {
			if (string->data[1] == 'B' || string->data[1] == '@') {
			    char *jis_buf = 0;

			    HTSprintf0(&jis_buf, "\033%s", string->data);
			    TO_EUC((const unsigned char *) jis_buf,
				   (unsigned char *) string->data);
			    FREE(jis_buf);
			}
		    }
		}
#endif
		handle_attribute_value(me, string->data);
	    }
	    string->size = 0;
	    if (c == '>') {	/* End of tag */
#ifdef USE_PRETTYSRC
		if (psrc_view) {
		    PSRCSTART(abracket);
		    PUTC('>');
		    PSRCSTOP(abracket);
		} else
#endif
		if (me->current_tag->name)
		    start_element(me);
		me->state = S_text;
		break;
	    } else
		me->state = S_tag_gap;
	} else if (me->T.decode_utf8 &&
		   *me->U.utf_buf) {
	    HTChunkPuts(string, me->U.utf_buf);
	    me->U.utf_buf_p = me->U.utf_buf;
	    *(me->U.utf_buf_p) = '\0';
	} else if (!IS_CJK_TTY &&
		   (me->T.output_utf8 ||
		    me->T.trans_from_uni)) {
	    if (clong == UCS_REPL && saved_char_in &&
		HTPassEightBitRaw &&
		saved_char_in >=
		LYlowest_eightbit[me->outUCLYhndl]) {
		(HTChunkPutUtf8Char) (string,
				      (UCode_t) (0xf000 | saved_char_in));
	    } else {
		HTChunkPutUtf8Char(string, clong);
	    }
	} else if (saved_char_in && me->T.use_raw_char_in) {
	    HTChunkPutc(string, saved_char_in);
	} else {
	    HTChunkPutc(string, c);
	}
	break;

    case S_squoted:		/* Quoted attribute value */
	if (c == '\'') {	/* End of attribute value */
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (!end_if_prettysrc(me, string, '\''))
#endif
		handle_attribute_value(me, string->data);
	    string->size = 0;
	    me->state = S_tag_gap;
	} else if (TOASCII(c) == '\033') {	/* S/390 -- gil -- 1213 */
	    /*
	     * Setting up for possible single quotes in CJK escape sequences. 
	     * - Takuya ASADA (asada@three-a.co.jp)
	     */
	    me->state = S_esc_sq;
	    if (!UTF8_TTY_ISO2022JP)
		HTChunkPutc(string, c);
	} else if (me->T.decode_utf8 &&
		   *me->U.utf_buf) {
	    HTChunkPuts(string, me->U.utf_buf);
	    me->U.utf_buf_p = me->U.utf_buf;
	    *(me->U.utf_buf_p) = '\0';
	} else if (!IS_CJK_TTY &&
		   (me->T.output_utf8 ||
		    me->T.trans_from_uni)) {
	    if (clong == UCS_REPL && saved_char_in &&
		HTPassEightBitRaw &&
		saved_char_in >=
		LYlowest_eightbit[me->outUCLYhndl]) {
		(HTChunkPutUtf8Char) (string,
				      (UCode_t) (0xf000 | saved_char_in));
	    } else {
		HTChunkPutUtf8Char(string, clong);
	    }
	} else if (saved_char_in && me->T.use_raw_char_in) {
	    HTChunkPutc(string, saved_char_in);
	} else {
	    HTChunkPutc(string, c);
	}
	break;

    case S_dquoted:		/* Quoted attribute value */
	if (c == '"' ||		/* Valid end of attribute value */
	    (soft_dquotes &&	/*  If emulating old Netscape bug, treat '>' */
	     c == '>')) {	/*  as a co-terminator of dquoted and tag    */
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (!end_if_prettysrc(me, string, (char) c))
#endif
		handle_attribute_value(me, string->data);
	    string->size = 0;
	    me->state = S_tag_gap;
	    if (c == '>')	/* We emulated the Netscape bug, so we go  */
		goto top1;	/* back and treat it as the tag terminator */
	} else if (TOASCII(c) == '\033') {	/* S/390 -- gil -- 1230 */
	    /*
	     * Setting up for possible double quotes in CJK escape sequences. 
	     * - Takuya ASADA (asada@three-a.co.jp)
	     */
	    me->state = S_esc_dq;
	    if (!UTF8_TTY_ISO2022JP)
		HTChunkPutc(string, c);
	} else if (me->T.decode_utf8 &&
		   *me->U.utf_buf) {
	    HTChunkPuts(string, me->U.utf_buf);
	    me->U.utf_buf_p = me->U.utf_buf;
	    *(me->U.utf_buf_p) = '\0';
	} else if (!IS_CJK_TTY &&
		   (me->T.output_utf8 ||
		    me->T.trans_from_uni)) {
	    if (clong == UCS_REPL && saved_char_in &&
		HTPassEightBitRaw &&
		saved_char_in >=
		LYlowest_eightbit[me->outUCLYhndl]) {
		(HTChunkPutUtf8Char) (string,
				      (UCode_t) (0xf000 | saved_char_in));
	    } else {
		HTChunkPutUtf8Char(string, clong);
	    }
	} else if (saved_char_in && me->T.use_raw_char_in) {
	    HTChunkPutc(string, saved_char_in);
	} else {
	    HTChunkPutc(string, c);
	}
	break;

    case S_end:		/* </ */
	if (TOASCII(clong) < 127 && (string->size ?	/* S/390 -- gil -- 1247 */
				     IsNmChar(c) : IsNmStart(c))) {
	    HTChunkPutc(string, c);
	} else {		/* End of end tag name */
	    HTTag *t = 0;

#ifdef USE_PRETTYSRC
	    BOOL psrc_tagname_processed = FALSE;
#endif

	    HTChunkTerminate(string);
	    if (!*string->data) {	/* Empty end tag */
		if (me->element_stack)
		    t = me->element_stack->tag;
	    } else {
		t = SGMLFindTag(dtd, string->data);
	    }
	    if (!t || t == me->unknown_tag) {
		CTRACE((tfp, "Unknown end tag </%s>\n", string->data));
#ifdef USE_PRETTYSRC
		if (psrc_view) {
		    PSRCSTART(abracket);
		    PUTS("</");
		    PSRCSTOP(abracket);
		    PSRCSTART(badtag);
		    transform_tag(me, string);
		    PUTS(string->data);
		    if (c != '>') {
			PUTC(c);
		    } else {
			PSRCSTOP(badtag);
			PSRCSTART(abracket);
			PUTC('>');
			PSRCSTOP(abracket);
		    }
		    psrc_tagname_processed = TRUE;
		}
	    } else if (psrc_view) {
#endif
	    } else {
		BOOL tag_OK = (BOOL) (c == '>' || WHITE(c));
		HTMLElement e = TAGNUM_OF_TAGP(t);
		int branch = 2;	/* it can be 0,1,2 */

		me->current_tag = t;
		if (HAS_ALT_TAGNUM(TAGNUM_OF_TAGP(t)) &&
		    me->element_stack &&
		    ALT_TAGP(t) == me->element_stack->tag)
		    me->element_stack->tag = NORMAL_TAGP(me->element_stack->tag);

		if (tag_OK && Old_DTD) {
		    switch (e) {
		    case HTML_DD:
		    case HTML_DT:
		    case HTML_LI:
		    case HTML_LH:
		    case HTML_TD:
		    case HTML_TH:
		    case HTML_TR:
		    case HTML_THEAD:
		    case HTML_TFOOT:
		    case HTML_TBODY:
		    case HTML_COLGROUP:
			branch = 0;
			break;

		    case HTML_A:
		    case HTML_B:
		    case HTML_BLINK:
		    case HTML_CITE:
		    case HTML_EM:
		    case HTML_FONT:
		    case HTML_FORM:
		    case HTML_I:
		    case HTML_P:
		    case HTML_STRONG:
		    case HTML_TT:
		    case HTML_U:
			branch = 1;
			break;
		    default:
			break;
		    }
		}

		/*
		 * Just handle ALL end tags normally :-) - kw
		 */
		if (!Old_DTD) {
		    end_element(me, me->current_tag);
		} else if (tag_OK && (branch == 0)) {
		    /*
		     * Don't treat these end tags as invalid, nor act on them. 
		     * - FM
		     */
		    CTRACE((tfp, "SGML: `</%s%c' found!  Ignoring it.\n",
			    string->data, c));
		    string->size = 0;
		    me->current_attribute_number = INVALID;
		    if (c != '>') {
			me->state = S_junk_tag;
		    } else {
			me->current_tag = NULL;
			me->state = S_text;
		    }
		    break;
		} else if (tag_OK && (branch == 1)) {
		    /*
		     * Handle end tags for container elements declared as
		     * SGML_EMPTY to prevent "expected tag substitution" but
		     * still processed via HTML_end_element() in HTML.c with
		     * checks there to avoid throwing the HTML.c stack out of
		     * whack (Ugh, what a hack!  8-).  - FM
		     */
		    if (me->inSELECT) {
			/*
			 * We are in a SELECT block.  - FM
			 */
			if (strcasecomp(string->data, "FORM")) {
			    /*
			     * It is not at FORM end tag, so ignore it.  - FM
			     */
			    CTRACE((tfp,
				    "SGML: ***Ignoring end tag </%s> in SELECT block.\n",
				    string->data));
			} else {
			    /*
			     * End the SELECT block and then handle the FORM
			     * end tag.  - FM
			     */
			    CTRACE((tfp,
				    "SGML: ***Faking SELECT end tag before </%s> end tag.\n",
				    string->data));
			    end_element(me,
					SGMLFindTag(me->dtd, "SELECT"));
			    CTRACE((tfp, "SGML: End </%s>\n", string->data));

#ifdef USE_PRETTYSRC
			    if (!psrc_view)	/* Don't actually call if viewing psrc - kw */
#endif
				(*me->actions->end_element)
				    (me->target,
				     (int) TAGNUM_OF_TAGP(me->current_tag),
				     &me->include);
			}
		    } else if (!strcasecomp(string->data, "P")) {
			/*
			 * Treat a P end tag like a P start tag (Ugh, what a
			 * hack!  8-).  - FM
			 */
			CTRACE((tfp,
				"SGML: `</%s%c' found!  Treating as '<%s%c'.\n",
				string->data, c, string->data, c));
			{
			    int i;

			    for (i = 0;
				 i < me->current_tag->number_of_attributes;
				 i++) {
				me->present[i] = NO;
			    }
			}
			if (me->current_tag->name)
			    start_element(me);
		    } else {
			CTRACE((tfp, "SGML: End </%s>\n", string->data));

#ifdef USE_PRETTYSRC
			if (!psrc_view)		/* Don't actually call if viewing psrc - kw */
#endif
			    (*me->actions->end_element)
				(me->target,
				 (int) TAGNUM_OF_TAGP(me->current_tag),
				 &me->include);
		    }
		    string->size = 0;
		    me->current_attribute_number = INVALID;
		    if (c != '>') {
			me->state = S_junk_tag;
		    } else {
			me->current_tag = NULL;
			me->state = S_text;
		    }
		    break;
		} else {
		    /*
		     * Handle all other end tags normally.  - FM
		     */
		    end_element(me, me->current_tag);
		}
	    }

#ifdef USE_PRETTYSRC
	    if (psrc_view && !psrc_tagname_processed) {
		PSRCSTART(abracket);
		PUTS("</");
		PSRCSTOP(abracket);
		PSRCSTART(tag);
		if (tagname_transform != 1) {
		    if (tagname_transform == 0)
			LYLowerCase(string->data);
		    else
			LYUpperCase(string->data);
		}
		PUTS(string->data);
		PSRCSTOP(tag);
		if (c != '>') {
		    PSRCSTART(badtag);
		    PUTC(c);
		} else {
		    PSRCSTART(abracket);
		    PUTC('>');
		    PSRCSTOP(abracket);
		}
	    }
#endif

	    string->size = 0;
	    me->current_attribute_number = INVALID;
	    if (c != '>') {
		if (!WHITE(c))
		    CTRACE((tfp, "SGML: `</%s%c' found!\n", string->data, c));
		me->state = S_junk_tag;
	    } else {
		me->current_tag = NULL;
		me->state = S_text;
	    }
	}
	break;

    case S_esc:		/* Expecting '$'or '(' following CJK ESC. */
	if (c == '$') {
	    me->state = S_dollar;
	} else if (c == '(') {
	    me->state = S_paren;
	} else {
	    me->state = S_text;
	    if (UTF8_TTY_ISO2022JP)
		goto top1;
	}
	if (!UTF8_TTY_ISO2022JP)
	    PUTC(c);
	break;

    case S_dollar:		/* Expecting '@', 'B', 'A' or '(' after CJK "ESC$". */
	if (c == '@' || c == 'B' || c == 'A') {
	    me->state = S_nonascii_text;
	} else if (c == '(') {
	    me->state = S_dollar_paren;
	}
	if (!UTF8_TTY_ISO2022JP)
	    PUTC(c);
	break;

    case S_dollar_paren:	/* Expecting 'C' after CJK "ESC$(". */
	if (c == 'C') {
	    me->state = S_nonascii_text;
	} else {
	    me->state = S_text;
	    if (UTF8_TTY_ISO2022JP) {
		PUTS("$(");
		goto top1;
	    }
	}
	if (!UTF8_TTY_ISO2022JP)
	    PUTC(c);
	break;

    case S_paren:		/* Expecting 'B', 'J', 'T' or 'I' after CJK "ESC(". */
	if (c == 'B' || c == 'J' || c == 'T') {
	    me->state = S_text;
	} else if (c == 'I') {
	    me->state = S_nonascii_text;
	    if (UTF8_TTY_ISO2022JP)
		me->kanji_buf = '\t';	/* flag for single byte katakana */
	} else {
	    me->state = S_text;
	    if (UTF8_TTY_ISO2022JP) {
		PUTC('(');
		goto top1;
	    }
	}
	if (!UTF8_TTY_ISO2022JP)
	    PUTC(c);
	break;

    case S_nonascii_text:	/* Expecting CJK ESC after non-ASCII text. */
	if (TOASCII(c) == '\033') {	/* S/390 -- gil -- 1264 */
	    me->state = S_esc;
	} else if (c < 32) {
	    me->state = S_text;
	}
	if (UTF8_TTY_ISO2022JP) {
	    if (TOASCII(c) != '\033')
		PUTUTF8(clong);
	} else
	    PUTC(c);
	break;

    case S_esc_sq:		/* Expecting '$'or '(' following CJK ESC. */
	if (c == '$') {
	    me->state = S_dollar_sq;
	} else if (c == '(') {
	    me->state = S_paren_sq;
	} else {
	    me->state = S_squoted;
	    if (UTF8_TTY_ISO2022JP)
		goto top1;
	}
	if (!UTF8_TTY_ISO2022JP)
	    HTChunkPutc(string, c);
	break;

    case S_dollar_sq:		/* Expecting '@', 'B', 'A' or '(' after CJK "ESC$". */
	if (c == '@' || c == 'B' || c == 'A') {
	    me->state = S_nonascii_text_sq;
	} else if (c == '(') {
	    me->state = S_dollar_paren_sq;
	}
	if (!UTF8_TTY_ISO2022JP)
	    HTChunkPutc(string, c);
	break;

    case S_dollar_paren_sq:	/* Expecting 'C' after CJK "ESC$(". */
	if (c == 'C') {
	    me->state = S_nonascii_text_sq;
	} else {
	    me->state = S_squoted;
	    if (UTF8_TTY_ISO2022JP) {
		HTChunkPuts(string, "$(");
		goto top1;
	    }
	}
	if (!UTF8_TTY_ISO2022JP)
	    HTChunkPutc(string, c);
	break;

    case S_paren_sq:		/* Expecting 'B', 'J', 'T' or 'I' after CJK "ESC(". */
	if (c == 'B' || c == 'J' || c == 'T') {
	    me->state = S_squoted;
	} else if (c == 'I') {
	    me->state = S_nonascii_text_sq;
	    if (UTF8_TTY_ISO2022JP)
		me->kanji_buf = '\t';	/* flag for single byte katakana */
	} else {
	    me->state = S_squoted;
	    if (UTF8_TTY_ISO2022JP) {
		HTChunkPutc(string, '(');
		goto top1;
	    }
	}
	if (!UTF8_TTY_ISO2022JP)
	    HTChunkPutc(string, c);
	break;

    case S_nonascii_text_sq:	/* Expecting CJK ESC after non-ASCII text. */
	if (TOASCII(c) == '\033') {	/* S/390 -- gil -- 1281 */
	    me->state = S_esc_sq;
	}
	if (UTF8_TTY_ISO2022JP) {
	    if (TOASCII(c) != '\033')
		HTChunkPutUtf8Char(string, clong);
	} else
	    HTChunkPutc(string, c);
	break;

    case S_esc_dq:		/* Expecting '$'or '(' following CJK ESC. */
	if (c == '$') {
	    me->state = S_dollar_dq;
	} else if (c == '(') {
	    me->state = S_paren_dq;
	} else {
	    me->state = S_dquoted;
	    if (UTF8_TTY_ISO2022JP)
		goto top1;
	}
	if (!UTF8_TTY_ISO2022JP)
	    HTChunkPutc(string, c);
	break;

    case S_dollar_dq:		/* Expecting '@', 'B', 'A' or '(' after CJK "ESC$". */
	if (c == '@' || c == 'B' || c == 'A') {
	    me->state = S_nonascii_text_dq;
	} else if (c == '(') {
	    me->state = S_dollar_paren_dq;
	}
	if (!UTF8_TTY_ISO2022JP)
	    HTChunkPutc(string, c);
	break;

    case S_dollar_paren_dq:	/* Expecting 'C' after CJK "ESC$(". */
	if (c == 'C') {
	    me->state = S_nonascii_text_dq;
	} else {
	    me->state = S_dquoted;
	    if (UTF8_TTY_ISO2022JP) {
		HTChunkPuts(string, "$(");
		goto top1;
	    }
	}
	if (!UTF8_TTY_ISO2022JP)
	    HTChunkPutc(string, c);
	break;

    case S_paren_dq:		/* Expecting 'B', 'J', 'T' or 'I' after CJK "ESC(". */
	if (c == 'B' || c == 'J' || c == 'T') {
	    me->state = S_dquoted;
	} else if (c == 'I') {
	    me->state = S_nonascii_text_dq;
	    if (UTF8_TTY_ISO2022JP)
		me->kanji_buf = '\t';	/* flag for single byte katakana */
	} else {
	    me->state = S_dquoted;
	    if (UTF8_TTY_ISO2022JP) {
		HTChunkPutc(string, '(');
		goto top1;
	    }
	}
	if (!UTF8_TTY_ISO2022JP)
	    HTChunkPutc(string, c);
	break;

    case S_nonascii_text_dq:	/* Expecting CJK ESC after non-ASCII text. */
	if (TOASCII(c) == '\033') {	/* S/390 -- gil -- 1298 */
	    me->state = S_esc_dq;
	}
	if (UTF8_TTY_ISO2022JP) {
	    if (TOASCII(c) != '\033')
		HTChunkPutUtf8Char(string, clong);
	} else
	    HTChunkPutc(string, c);
	break;

    case S_junk_tag:
    case S_pi:
	if (c == '>') {
	    HTChunkTerminate(string);
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		if (me->state == S_junk_tag) {
		    PSRCSTOP(badtag);
		}
		PSRCSTART(abracket);
		PUTC('>');
		PSRCSTOP(abracket);
	    }
#endif
	    if (me->state == S_pi)
		handle_processing_instruction(me);
	    string->size = 0;
	    me->current_tag = NULL;
	    me->state = S_text;
	} else {
	    HTChunkPutc(string, c);
#ifdef USE_PRETTYSRC
	    if (psrc_view) {
		PUTC(c);
	    }
#endif
	}

    }				/* switch on me->state */
    CTRACE2(TRACE_SGML, (tfp, "SGML after  %s|%.*s|%c|\n",
			 state_name(me->state),
			 string->size,
			 NonNull(string->data),
			 UCH(c)));

  after_switch:
    /*
     * Check whether an external function has added anything to the include
     * buffer.  If so, move the new stuff to the beginning of active_include. 
     * - kw
     */
    if (me->include != NULL) {
	if (me->include[0] == '\0') {
	    FREE(me->include);
	} else {
	    if (me->active_include &&
		me->active_include[me->include_index] != '\0')
		StrAllocCat(me->include,
			    me->active_include + me->include_index);
	    FREE(me->active_include);
	    me->active_include = me->include;
	    me->include_index = 0;
	    me->include = NULL;
	}
    }

    /*
     * Check whether we've added anything to the recover buffer.  - FM
     */
    if (me->recover != NULL) {
	if (me->recover[me->recover_index] == '\0') {
	    FREE(me->recover);
	    me->recover_index = 0;
	} else {
	    c = UCH(me->recover[me->recover_index]);
	    me->recover_index++;
	    goto top;
	}
    }

    /*
     * Check whether an external function had added anything to the include
     * buffer; it should now be in active_include.  - FM / kw
     */
    if (me->active_include != NULL) {
	if (me->active_include[me->include_index] == '\0') {
	    FREE(me->active_include);
	    me->include_index = 0;
	} else {
	    if (me->current_tag_charset == UTF8_handle ||
		me->T.trans_from_uni) {
		/*
		 * If it looks like we would have fed UTF-8 to the next
		 * processing stage, assume that whatever we were fed back is
		 * in UTF-8 form, too.  This won't be always true for all uses
		 * of the include buffer, but it's a start.  - kw
		 */
		const char *puni = me->active_include + me->include_index;

		c = UCH(*puni);
		clong = UCGetUniFromUtf8String(&puni);
		if (clong < 256 && clong >= 0) {
		    c = UCH((clong & 0xff));
		}
		saved_char_in = '\0';
		me->include_index = (int) (puni
					   - me->active_include
					   + 1);
		goto top1;
	    } else {
		/*
		 * Otherwise assume no UTF-8 - do charset-naive processing and
		 * hope for the best.  - kw
		 */
		c = UCH(me->active_include[me->include_index]);
		me->include_index++;
		goto top;
	    }
	}
    }

    /*
     * Check whether an external function has added anything to the csi buffer. 
     * - FM
     */
    if (me->csi != NULL) {
	if (me->csi[me->csi_index] == '\0') {
	    FREE(me->csi);
	    me->csi_index = 0;
	} else {
	    c = UCH(me->csi[me->csi_index]);
	    me->csi_index++;
	    goto top;
	}
    }
}				/* SGML_character */

static void InferUtfFromBom(HTStream *me, int chndl)
{
    HTAnchor_setUCInfoStage(me->node_anchor, chndl,
			    UCT_STAGE_PARSER,
			    UCT_SETBY_PARSER);
    change_chartrans_handling(me);
}

/*
 * Avoid rewrite of SGML_character() to handle hypothetical case of UTF-16
 * webpages, by pretending that the data is UTF-8.
 */
static void SGML_widechar(HTStream *me, int ch)
{
    if (!UCPutUtf8_charstring(me, SGML_character, (UCode_t) ch)) {
	SGML_character(me, ch);
    }
}

static void SGML_write(HTStream *me, const char *str, int l)
{
    const char *p;
    const char *e = str + l;

    if (sgml_offset == 0) {
	if (l > 3
	    && !MemCmp(str, "\357\273\277", 3)) {
	    CTRACE((tfp, "SGML_write found UTF-8 BOM\n"));
	    InferUtfFromBom(me, UTF8_handle);
	    str += 3;
	} else if (l > 2) {
	    if (!MemCmp(str, "\377\376", 2)) {
		CTRACE((tfp, "SGML_write found UCS-2 LE BOM\n"));
		InferUtfFromBom(me, UTF8_handle);
		str += 2;
		me->T.ucs_mode = -1;
	    } else if (!MemCmp(str, "\376\377", 2)) {
		CTRACE((tfp, "SGML_write found UCS-2 BE BOM\n"));
		InferUtfFromBom(me, UTF8_handle);
		str += 2;
		me->T.ucs_mode = 1;
	    }
	}
    }
    switch (me->T.ucs_mode) {
    case -1:
	for (p = str; p < e; p += 2)
	    SGML_widechar(me, (UCH(p[1]) << 8) | UCH(p[0]));
	break;
    case 1:
	for (p = str; p < e; p += 2)
	    SGML_widechar(me, (UCH(p[0]) << 8) | UCH(p[1]));
	break;
    default:
	for (p = str; p < e; p++)
	    SGML_character(me, *p);
	break;
    }
}

static void SGML_string(HTStream *me, const char *str)
{
    SGML_write(me, str, (int) strlen(str));
}

/*_______________________________________________________________________
*/

/*	Structured Object Class
 *	-----------------------
 */
const HTStreamClass SGMLParser =
{
    "SGMLParser",
    SGML_free,
    SGML_abort,
    SGML_character,
    SGML_string,
    SGML_write,
};

/*	Create SGML Engine
 *	------------------
 *
 * On entry,
 *	dtd		represents the DTD, along with
 *	actions		is the sink for the data as a set of routines.
 *
 */

HTStream *SGML_new(const SGML_dtd * dtd,
		   HTParentAnchor *anchor,
		   HTStructured * target,
		   int extended_html)
{
    HTStream *me = typecalloc(struct _HTStream);

    if (!me)
	outofmem(__FILE__, "SGML_begin");

    me->isa = &SGMLParser;
    me->string = HTChunkCreate(128);	/* Grow by this much */
    me->dtd = dtd;
    me->target = target;
    me->actions = (const HTStructuredClass *) (((HTStream *) target)->isa);
    /* Ugh: no OO */
    me->unknown_tag = &HTTag_unrecognized;
    me->current_tag = me->slashedtag = NULL;
    me->state = S_text;
#ifdef CALLERDATA
    me->callerData = (void *) callerData;
#endif /* CALLERDATA */

    me->node_anchor = anchor;	/* Could be NULL? */
    me->U.utf_buf_p = me->U.utf_buf;
    UCTransParams_clear(&me->T);
    me->inUCLYhndl = HTAnchor_getUCLYhndl(anchor,
					  UCT_STAGE_PARSER);
    if (me->inUCLYhndl < 0) {
	HTAnchor_copyUCInfoStage(anchor,
				 UCT_STAGE_PARSER,
				 UCT_STAGE_MIME,
				 -1);
	me->inUCLYhndl = HTAnchor_getUCLYhndl(anchor,
					      UCT_STAGE_PARSER);
    }
#ifdef CAN_SWITCH_DISPLAY_CHARSET	/* Allow a switch to a more suitable display charset */
    else if (anchor->UCStages
	     && anchor->UCStages->s[UCT_STAGE_PARSER].LYhndl >= 0
	     && anchor->UCStages->s[UCT_STAGE_PARSER].LYhndl != current_char_set) {
	int o = anchor->UCStages->s[UCT_STAGE_PARSER].LYhndl;

	anchor->UCStages->s[UCT_STAGE_PARSER].LYhndl = -1;	/* Force reset */
	HTAnchor_resetUCInfoStage(anchor, o, UCT_STAGE_PARSER,
	/* Preserve change this: */
				  anchor->UCStages->s[UCT_STAGE_PARSER].lock);
    }
#endif

    me->inUCI = HTAnchor_getUCInfoStage(anchor,
					UCT_STAGE_PARSER);
    set_chartrans_handling(me, anchor, -1);

    me->recover = NULL;
    me->recover_index = 0;
    me->include = NULL;
    me->active_include = NULL;
    me->include_index = 0;
    me->url = NULL;
    me->csi = NULL;
    me->csi_index = 0;

#ifdef USE_PRETTYSRC
    if (psrc_view) {
	psrc_view = FALSE;
	mark_htext_as_source = TRUE;
	SGML_string(me,
		    "<HTML><HEAD><TITLE>source</TITLE></HEAD><BODY><PRE>");
	psrc_view = TRUE;
	psrc_convert_string = FALSE;
	sgml_in_psrc_was_initialized = TRUE;
    }
#endif
    if (extended_html) {
	me->extended_html = TRUE;
    }

    sgml_offset = 0;
    return me;
}

/*
 * Return the offset within the document where we're parsing.  This is used
 * to help identify anchors which shift around while reparsing.
 */
int SGML_offset(void)
{
    int result = sgml_offset;

#ifdef USE_PRETTYSRC
    result += psrc_view;
#endif
    return result;
}

/*		Asian character conversion functions
 *		====================================
 *
 *	Added 24-Mar-96 by FM, based on:
 *
 ////////////////////////////////////////////////////////////////////////
Copyright (c) 1993 Electrotechnical Laboratory (ETL)

Permission to use, copy, modify, and distribute this material
for any purpose and without fee is hereby granted, provided
that the above copyright notice and this permission notice
appear in all copies, and that the name of ETL not be
used in advertising or publicity pertaining to this
material without the specific, prior written permission
of an authorized representative of ETL.
ETL MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
/////////////////////////////////////////////////////////////////////////
Content-Type:	program/C; charset=US-ASCII
Program:	SJIS.c
Author:		Yutaka Sato <ysato@etl.go.jp>
Description:
History:
	930923	extracted from codeconv.c of cosmos
///////////////////////////////////////////////////////////////////////
*/

static int TREAT_SJIS = 1;

void JISx0201TO0208_EUC(unsigned IHI,
			unsigned ILO,
			unsigned char *OHI,
			unsigned char *OLO)
{
    static const char *table[] =
    {
	"\241\243",		/* A1,A3 */
	"\241\326",		/* A1,D6 */
	"\241\327",		/* A1,D7 */
	"\241\242",		/* A1,A2 */
	"\241\246",		/* A1,A6 */
	"\245\362",		/* A5,F2 */
	"\245\241",		/* A5,A1 */
	"\245\243",		/* A5,A3 */
	"\245\245",		/* A5,A5 */
	"\245\247",		/* A5,A7 */
	"\245\251",		/* A5,A9 */
	"\245\343",		/* A5,E3 */
	"\245\345",		/* A5,E5 */
	"\245\347",		/* A5,E7 */
	"\245\303",		/* A5,C3 */
	"\241\274",		/* A1,BC */
	"\245\242",		/* A5,A2 */
	"\245\244",		/* A5,A4 */
	"\245\246",		/* A5,A6 */
	"\245\250",		/* A5,A8 */
	"\245\252",		/* A5,AA */
	"\245\253",		/* A5,AB */
	"\245\255",		/* A5,AD */
	"\245\257",		/* A5,AF */
	"\245\261",		/* A5,B1 */
	"\245\263",		/* A5,B3 */
	"\245\265",		/* A5,B5 */
	"\245\267",		/* A5,B7 */
	"\245\271",		/* A5,B9 */
	"\245\273",		/* A5,BB */
	"\245\275",		/* A5,BD */
	"\245\277",		/* A5,BF */
	"\245\301",		/* A5,C1 */
	"\245\304",		/* A5,C4 */
	"\245\306",		/* A5,C6 */
	"\245\310",		/* A5,C8 */
	"\245\312",		/* A5,CA */
	"\245\313",		/* A5,CB */
	"\245\314",		/* A5,CC */
	"\245\315",		/* A5,CD */
	"\245\316",		/* A5,CE */
	"\245\317",		/* A5,CF */
	"\245\322",		/* A5,D2 */
	"\245\325",		/* A5,D5 */
	"\245\330",		/* A5,D8 */
	"\245\333",		/* A5,DB */
	"\245\336",		/* A5,DE */
	"\245\337",		/* A5,DF */
	"\245\340",		/* A5,E0 */
	"\245\341",		/* A5,E1 */
	"\245\342",		/* A5,E2 */
	"\245\344",		/* A5,E4 */
	"\245\346",		/* A5,E6 */
	"\245\350",		/* A5,E8 */
	"\245\351",		/* A5,E9 */
	"\245\352",		/* A5,EA */
	"\245\353",		/* A5,EB */
	"\245\354",		/* A5,EC */
	"\245\355",		/* A5,ED */
	"\245\357",		/* A5,EF */
	"\245\363",		/* A5,F3 */
	"\241\253",		/* A1,AB */
	"\241\254"		/* A1,AC */
    };

    if ((IHI == 0x8E) && (ILO >= 0xA1) && (ILO <= 0xDF)) {
	*OHI = UCH(table[ILO - 0xA1][0]);
	*OLO = UCH(table[ILO - 0xA1][1]);
    } else {
	*OHI = UCH(IHI);
	*OLO = UCH(ILO);
    }
}

static int IS_SJIS_STR(const unsigned char *str)
{
    const unsigned char *s;
    unsigned char ch;
    int is_sjis = 0;

    s = str;
    while ((ch = *s++) != '\0') {
	if (ch & 0x80)
	    if (IS_SJIS(ch, *s, is_sjis))
		return 1;
    }
    return 0;
}

unsigned char *SJIS_TO_JIS1(unsigned HI,
			    unsigned LO,
			    unsigned char *JCODE)
{
    HI = UCH(HI - (unsigned) UCH((HI <= 0x9F) ? 0x71 : 0xB1));
    HI = UCH((HI << 1) + 1);
    if (0x7F < LO)
	LO--;
    if (0x9E <= LO) {
	LO = UCH(LO - UCH(0x7D));
	HI++;
    } else {
	LO = UCH(LO - UCH(0x1F));
    }
    JCODE[0] = UCH(HI);
    JCODE[1] = UCH(LO);
    return JCODE;
}

unsigned char *JIS_TO_SJIS1(unsigned HI,
			    unsigned LO,
			    unsigned char *SJCODE)
{
    if (HI & 1)
	LO = UCH(LO + UCH(0x1F));
    else
	LO = UCH(LO + UCH(0x7D));
    if (0x7F <= LO)
	LO++;

    HI = UCH(((HI - 0x21) >> 1) + 0x81);
    if (0x9F < HI)
	HI = UCH(HI + UCH(0x40));
    SJCODE[0] = UCH(HI);
    SJCODE[1] = UCH(LO);
    return SJCODE;
}

unsigned char *EUC_TO_SJIS1(unsigned HI,
			    unsigned LO,
			    unsigned char *SJCODE)
{
    unsigned char HI_data[2];
    unsigned char LO_data[2];

    HI_data[0] = UCH(HI);
    LO_data[0] = UCH(LO);
    if (HI == 0x8E) {
	JISx0201TO0208_EUC(HI, LO, HI_data, LO_data);
    }
    JIS_TO_SJIS1(UCH(HI_data[0] & 0x7F), UCH(LO_data[0] & 0x7F), SJCODE);
    return SJCODE;
}

void JISx0201TO0208_SJIS(unsigned I,
			 unsigned char *OHI,
			 unsigned char *OLO)
{
    unsigned char SJCODE[2];

    JISx0201TO0208_EUC(0x8E, I, OHI, OLO);
    JIS_TO_SJIS1(UCH(*OHI & 0x7F), UCH(*OLO & 0x7F), SJCODE);
    *OHI = SJCODE[0];
    *OLO = SJCODE[1];
}

unsigned char *SJIS_TO_EUC1(unsigned HI,
			    unsigned LO,
			    unsigned char *data)
{
    SJIS_TO_JIS1(HI, LO, data);
    data[0] |= 0x80;
    data[1] |= 0x80;
    return data;
}

unsigned char *SJIS_TO_EUC(unsigned char *src,
			   unsigned char *dst)
{
    unsigned char hi, lo, *sp, *dp;
    int in_sjis = 0;

    in_sjis = IS_SJIS_STR(src);
    for (sp = src, dp = dst; (hi = sp[0]) != '\0';) {
	lo = sp[1];
	if (TREAT_SJIS && IS_SJIS(hi, lo, in_sjis)) {
	    SJIS_TO_JIS1(hi, lo, dp);
	    dp[0] |= 0x80;
	    dp[1] |= 0x80;
	    dp += 2;
	    sp += 2;
	} else
	    *dp++ = *sp++;
    }
    *dp = 0;
    return dst;
}

unsigned char *EUC_TO_SJIS(unsigned char *src,
			   unsigned char *dst)
{
    unsigned char *sp, *dp;

    for (sp = src, dp = dst; *sp;) {
	if (*sp & 0x80) {
	    if (sp[1] && (sp[1] & 0x80)) {
		JIS_TO_SJIS1(UCH(sp[0] & 0x7F), UCH(sp[1] & 0x7F), dp);
		dp += 2;
		sp += 2;
	    } else {
		sp++;
	    }
	} else {
	    *dp++ = *sp++;
	}
    }
    *dp = 0;
    return dst;
}

#define Strcpy(a,b)	(strcpy((char*)a,(const char*)b),&a[strlen((const char*)a)])

unsigned char *EUC_TO_JIS(unsigned char *src,
			  unsigned char *dst,
			  const char *toK,
			  const char *toA)
{
    unsigned char kana_mode = 0;
    unsigned char cch;
    unsigned char *sp = src;
    unsigned char *dp = dst;
    int is_JIS = 0;

    while ((cch = *sp++) != '\0') {
	if (cch & 0x80) {
	    if (!IS_EUC(cch, *sp)) {
		if (cch == 0xA0 && is_JIS)	/* ignore NBSP */
		    continue;
		is_JIS++;
		*dp++ = cch;
		continue;
	    }
	    if (!kana_mode) {
		kana_mode = UCH(~kana_mode);
		dp = Strcpy(dp, toK);
	    }
	    if (*sp & 0x80) {
		*dp++ = UCH(cch & ~0x80);
		*dp++ = UCH(*sp++ & ~0x80);
	    }
	} else {
	    if (kana_mode) {
		kana_mode = UCH(~kana_mode);
		dp = Strcpy(dp, toA);
	    }
	    *dp++ = cch;
	}
    }
    if (kana_mode)
	dp = Strcpy(dp, toA);

    if (dp)
	*dp = 0;
    return dst;
}

#define	IS_JIS7(c1,c2)	(0x20<(c1)&&(c1)<0x7F && 0x20<(c2)&&(c2)<0x7F)
#define SO		('N'-0x40)
#define SI		('O'-0x40)

static int repair_JIS = 0;

static const unsigned char *repairJIStoEUC(const unsigned char *src,
					   unsigned char **dstp)
{
    const unsigned char *s;
    unsigned char *d, ch1, ch2;

    d = *dstp;
    s = src;
    while ((ch1 = s[0]) && (ch2 = s[1])) {
	s += 2;
	if (ch1 == '(')
	    if (ch2 == 'B' || ch2 == 'J') {
		*dstp = d;
		return s;
	    }
	if (!IS_JIS7(ch1, ch2))
	    return 0;

	*d++ = UCH(0x80 | ch1);
	*d++ = UCH(0x80 | ch2);
    }
    return 0;
}

unsigned char *TO_EUC(const unsigned char *jis,
		      unsigned char *euc)
{
    const unsigned char *s;
    unsigned char c, jis_stat;
    unsigned char *d;
    int to1B, to2B;
    int in_sjis = 0;
    int n8bits;
    int is_JIS;

    n8bits = 0;
    s = jis;
    d = euc;
    jis_stat = 0;
    to2B = TO_2BCODE;
    to1B = TO_1BCODE;
    in_sjis = IS_SJIS_STR(jis);
    is_JIS = 0;

    while ((c = *s++) != '\0') {
	if (c == 0x80)
	    continue;		/* ignore it */
	if (c == 0xA0 && is_JIS)
	    continue;		/* ignore Non-breaking space */

	if (c == to2B && jis_stat == 0 && repair_JIS) {
	    if (*s == 'B' || *s == '@') {
		const unsigned char *ts;

		if ((ts = repairJIStoEUC(s + 1, &d)) != NULL) {
		    s = ts;
		    continue;
		}
	    }
	}
	if (c == CH_ESC) {
	    if (*s == to2B) {
		if ((s[1] == 'B') || (s[1] == '@')) {
		    jis_stat = 0x80;
		    s += 2;
		    is_JIS++;
		    continue;
		}
		jis_stat = 0;
	    } else if (*s == to1B) {
		jis_stat = 0;
		if ((s[1] == 'B') || (s[1] == 'J') || (s[1] == 'H')) {
		    s += 2;
		    continue;
		}
	    } else if (*s == ',') {	/* MULE */
		jis_stat = 0;
	    }
	}
	if (c & 0x80)
	    n8bits++;

	if (IS_SJIS(c, *s, in_sjis)) {
	    SJIS_TO_EUC1(c, *s, d);
	    d += 2;
	    s++;
	    is_JIS++;
	} else if (jis_stat) {
	    if (c <= 0x20 || 0x7F <= c) {
		*d++ = c;
		if (c == '\n')
		    jis_stat = 0;
	    } else {
		if (IS_JIS7(c, *s)) {
		    *d++ = jis_stat | c;
		    *d++ = jis_stat | *s++;
		} else
		    *d++ = c;
	    }
	} else {
	    if (n8bits == 0 && (c == SI || c == SO)) {
	    } else {
		*d++ = c;
	    }
	}
    }
    *d = 0;
    return euc;
}

#define non94(ch) ((ch) <= 0x20 || (ch) == 0x7F)

static int is_EUC_JP(unsigned char *euc)
{
    unsigned char *cp;
    int ch1, ch2;

    for (cp = euc; (ch1 = *cp) != '\0'; cp++) {
	if (ch1 & 0x80) {
	    ch2 = cp[1] & 0xFF;
	    if ((ch2 & 0x80) == 0) {
		/* sv1log("NOT_EUC1[%x][%x]\n",ch1,ch2); */
		return 0;
	    }
	    if (non94(ch1 & 0x7F) || non94(ch2 & 0x7F)) {
		/* sv1log("NOT_EUC2[%x][%x]\n",ch1,ch2); */
		return 0;
	    }
	    cp++;
	}
    }
    return 1;
}

void TO_SJIS(const unsigned char *arg,
	     unsigned char *sjis)
{
    unsigned char *euc;

    euc = typeMallocn(unsigned char, strlen((const char *) arg) + 1);

#ifdef CJK_EX
    if (!euc)
	outofmem(__FILE__, "TO_SJIS");
#endif
    TO_EUC(arg, euc);
    if (is_EUC_JP(euc))
	EUC_TO_SJIS(euc, sjis);
    else
	strcpy((char *) sjis, (const char *) arg);
    free(euc);
}

void TO_JIS(const unsigned char *arg,
	    unsigned char *jis)
{
    unsigned char *euc;

    if (arg[0] == 0) {
	jis[0] = 0;
	return;
    }
    euc = typeMallocn(unsigned char, strlen((const char *)arg) + 1);
#ifdef CJK_EX
    if (!euc)
	outofmem(__FILE__, "TO_JIS");
#endif
    TO_EUC(arg, euc);
    EUC_TO_JIS(euc, jis, TO_KANJI, TO_ASCII);

    free(euc);
}
