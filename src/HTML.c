/*		Structured stream to Rich hypertext converter
**		============================================
**
**	This generates of a hypertext object.  It converts from the
**	structured stream interface fro HTMl events into the style-
**	oriented iunterface of the HText.h interface.  This module is
**	only used in clients and shouldnot be linked into servers.
**
**	Override this module if making a new GUI browser.
**
**   Being Overidden
**
*/
#include "HTUtils.h"
#include "tcp.h"

#include "HTML.h"

/* #define CAREFUL		 Check nesting here not really necessary */

/*#include <ctype.h> included by HTUtils.h -- FM */
/*#include <stdio.h> included by HTUtils.h -- FM */

#include "HTCJK.h"
#include "HTAtom.h"
#include "HTChunk.h"
#include "HText.h"
#include "HTStyle.h"

#include "HTAlert.h"
#include "HTMLGen.h"
#include "HTParse.h"

#include "HTNestedList.h"
#include "HTForms.h"

#include "GridText.h"

#include "HTFont.h"

#ifdef VMS
#include "LYCurses.h"
#include "HTVMSUtils.h"
#endif /* VMS */


#include "LYGlobalDefs.h"
#include "LYSignal.h"
#include "LYUtils.h"
#include "LYCharSets.h"
#include "LYCharUtils.h"
#include "LYMap.h"

#include "LYexit.h"
#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

extern BOOL HTPassEightBitRaw;
extern HTCJKlang HTCJK;

extern BOOLEAN HT_Is_Gopher_URL;

PRIVATE char *LastOptionValue = NULL;

PUBLIC BOOLEAN ignore_excess = FALSE;

/* from Curses.h */
extern int LYcols;

extern HTStyleSheet * styleSheet;	/* Application-wide */

/*	Module-wide style cache
*/
PRIVATE int 		got_styles = 0;
PRIVATE HTStyle *styles[HTML_ELEMENTS+31]; /* adding 24 nested list styles  */
					   /* and 3 header alignment styles */
					   /* and 3 div alignment styles    */
PRIVATE HTStyle *default_style;
PUBLIC char HTML_Last_Char = '\0';  /* the last character put on the screen */
PRIVATE char *textarea_name = NULL;
PRIVATE char *textarea_cols = NULL;
PRIVATE int textarea_rows = 4;
PRIVATE int textarea_disabled = NO;
PRIVATE char *textarea_id = NULL;
PRIVATE BOOLEAN LastOptionChecked = FALSE;
PRIVATE BOOLEAN B_hide_mail_header = FALSE;
PRIVATE char *base_href = NULL;
PRIVATE int select_disabled = NO;
PRIVATE int current_default_alignment = HT_LEFT;
PRIVATE BOOLEAN LYUsePlainSpace = FALSE;
PRIVATE BOOLEAN LYHiddenValue = FALSE;

/*		HTML Object
**		-----------
*/
#define MAX_NESTING 800		/* Should be checked by parser */

/*      Track if we are in an anchor, paragraph, address, base, etc.
 */
PRIVATE BOOLEAN B_inA = FALSE;
PRIVATE BOOLEAN B_inAPPLET = FALSE;
PRIVATE BOOLEAN B_inAPPLETwithP = FALSE;
PRIVATE BOOLEAN B_inBadHTML = FALSE;
PRIVATE BOOLEAN B_inBASE = FALSE;
PRIVATE BOOLEAN B_inBoldA = FALSE;
PRIVATE BOOLEAN B_inBoldH = FALSE;
PRIVATE BOOLEAN B_inCAPTION = FALSE;
PRIVATE BOOLEAN B_inCREDIT = FALSE;
PRIVATE BOOLEAN B_inFIG = FALSE;
PRIVATE BOOLEAN B_inFIGwithP = FALSE;
PRIVATE BOOLEAN B_inFORM = FALSE;
PRIVATE BOOLEAN B_inLABEL = FALSE;
PRIVATE BOOLEAN B_inP = FALSE;
PRIVATE BOOLEAN B_inPRE = FALSE;
PRIVATE BOOLEAN B_inSELECT = FALSE;
PRIVATE BOOLEAN B_inTABLE = FALSE;
PRIVATE BOOLEAN B_inTEXTAREA = FALSE;
PRIVATE BOOLEAN B_inUnderline = FALSE;

PRIVATE BOOLEAN B_needBoldH = FALSE;

PUBLIC char *LYToolbarName = "LynxPseudoToolbar";

PUBLIC char *LYMapName = NULL;

/* used for nested lists */
PRIVATE int List_Nesting_Level= -1;  /* counter for list nesting level */
PRIVATE int OL_Counter[7];	     /* counter for ordered lists */
PRIVATE char OL_Type[7];	     /* types for ordered lists */
PRIVATE int Last_OL_Count = 0;	     /* last count in ordered lists */
PRIVATE char Last_OL_Type = '1';     /* last type in ordered lists */
PRIVATE int OL_CONTINUE = -29999;    /* flag for whether CONTINUE is set */
PRIVATE int OL_VOID = -29998;	     /* flag for whether a count is set */

PRIVATE int Division_Level = -1;
PRIVATE short DivisionAlignments[MAX_NESTING];
PRIVATE int Underline_Level = 0;
PRIVATE int Quote_Level = 0;

/* used to turn off a style if the HTML author forgot to
PRIVATE int i_prior_style = -1;
 */

/*
 *	Private function....
 */
PRIVATE void HTML_end_element PARAMS((HTStructured *me,
				      int element_number,
				      char **include));
PRIVATE void HTML_put_entity PARAMS((HTStructured *me, int entity_number));
PRIVATE BOOLEAN HTML_override_default_alignment PARAMS((HTStructured *me));
PRIVATE void HTML_zero_OL_Counter NOPARAMS;
PRIVATE void HTML_EnsureDoubleSpace PARAMS((HTStructured *me));
PRIVATE void HTML_EnsureSingleSpace PARAMS((HTStructured *me));
PRIVATE void HTML_ResetParagraphAlignment PARAMS((HTStructured *me));
PRIVATE void HTMLFillLocalFileURL PARAMS((char **href, char *base));

typedef struct _stack_element {
        HTStyle *	style;
	int		tag_number;
} stack_element;

struct _HTStructured {
    CONST HTStructuredClass * 	isa;
    HTParentAnchor * 		node_anchor;
    HText * 			text;

    HTStream*			target;			/* Output stream */
    HTStreamClass		targetClass;		/* Output routines */

    HTChunk 			title;		/* Grow by 128 */
    HTChunk			object;		/* Grow by 128 */
    BOOL			object_started;
    BOOL			object_declare;
    BOOL			object_shapes;
    BOOL			object_ismap;
    char *			object_usemap;
    char *			object_id;
    char *			object_title;
    char *			object_data;
    char *			object_type;
    char *			object_classid;
    char *			object_codebase;
    char *			object_codetype;
    char *			object_name;
    HTChunk			option;		/* Grow by 128 */
    HTChunk			textarea;	/* Grow by 128 */
    HTChunk			math;		/* Grow by 128 */
    HTChunk			style_block;	/* Grow by 128 */
    HTChunk			script;		/* Grow by 128 */

    char *			comment_start;	/* for literate programming */
    char *			comment_end;

    HTTag *			current_tag;
    BOOL			style_change;
    HTStyle *			new_style;
    HTStyle *			old_style;
    BOOL			in_word;  /* Have just had a non-white char */
    stack_element 	stack[MAX_NESTING];
    stack_element 	*sp;		/* Style stack pointer */
};

struct _HTStream {
    CONST HTStreamClass *	isa;
    /* .... */
};

/*		Forward declarations of routines
*/
PRIVATE void get_styles NOPARAMS;


PRIVATE void actually_set_style PARAMS((HTStructured * me));
PRIVATE void change_paragraph_style PARAMS((HTStructured * me,
					    HTStyle * style));
PRIVATE void HTML_CheckForID PARAMS((HTStructured * me,
				     CONST BOOL * present,
				     CONST char ** value,
				     int attribute));
PRIVATE void HTML_HandleID PARAMS((HTStructured * me, char * id));


/*	Style buffering avoids dummy paragraph begin/ends.
*/
#define UPDATE_STYLE if (me->style_change) { actually_set_style(me); }

PUBLIC BOOLEAN LYCheckForCSI ARGS2(
	HTStructured *, 	me,
	char **,		url)
{
    if (me == NULL || !me->node_anchor->address)
        return FALSE;

    if (strncasecomp(me->node_anchor->address, "file:", 5))
        return FALSE;

    if (!LYisLocalHost(me->node_anchor->address))
        return FALSE;
     
    StrAllocCopy(*url, me->node_anchor->address);
    return TRUE;
}


/*		Flattening the style structure
**		------------------------------
**
On the NeXT, and on any read-only browser, it is simpler for the text to have
a sequence of styles, rather than a nested tree of styles. In this
case we have to flatten the structure as it arrives from SGML tags into
a sequence of styles.
*/

/*
**  If style really needs to be set, call this.
*/
PRIVATE void actually_set_style ARGS1(HTStructured *, me)
{
    if (!me->text) {			/* First time through */
	    me->text = HText_new2(me->node_anchor, me->target);
	    HText_beginAppend(me->text);
	    HText_setStyle(me->text, me->new_style);
	    me->in_word = NO;
    } else {
	    HText_setStyle(me->text, me->new_style);
    }

    me->old_style = me->new_style;
    me->style_change = NO;

}

/*
**  If you THINK you need to change style, call this.
*/
PRIVATE void change_paragraph_style ARGS2(HTStructured *, me, HTStyle *,style)
{
    if (me->new_style != style) {
    	me->style_change = YES;
	me->new_style = style;
    }
    me->in_word = NO;
}

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/
PRIVATE void HTML_put_character ARGS2(HTStructured *, me, char, c)
{
   /*
    * Convert EOL styles:
    *  macintosh:  cr    --> lf
    *  ascii:      cr-lf --> lf
    *  unix:       lf    --> lf
    */
    static int lastraw = -1;

    /*
     *  Ignore all non-MAP content when just
     *  scanning a document for MAPs. - FM
     *  
    if (LYMapsOnly)
        return;

    /*
     *  Do EOL conversion if needed. - FM
     */
    if ((lastraw == '\r') && c == '\n') {
	lastraw = -1;
	return;
    }
    lastraw = c;
    if (c == '\r')
	c = '\n';

    /*
     *  Handle SGML_LITTERAL tags that have HTChunk elements. - FM
     */
    switch (me->sp[0].tag_number) {
 
    case HTML_COMMENT:
    	return;	/* Do Nothing */
	
    case HTML_TITLE:
        if (c == LY_SOFT_HYPHEN)
	    return;
	if (c != '\n' && c != '\t' && c != '\r')
    	    HTChunkPutc(&me->title, c);
	else
    	    HTChunkPutc(&me->title, ' ');
	return;

    case HTML_STYLE:
	HTChunkPutc(&me->style_block, c);
        return;

    case HTML_SCRIPT:
	HTChunkPutc(&me->script, c);
        return;

    case HTML_OBJECT:	
    	HTChunkPutc(&me->object, c);
	return;

    case HTML_TEXTAREA:	
    	HTChunkPutc(&me->textarea, c);
	return;

    case HTML_SELECT:	
    	HTChunkPutc(&me->option, c);
	return;

    case HTML_MATH:
    	HTChunkPutc(&me->math, c);
	return;

    default:
        break;
    } /* end first switch */

    /*
     *  Handle all other tag content. - FM
     */
    switch (me->sp[0].tag_number) {
 
    case HTML_PRE:				/* Formatted text */
	/*
	 *  We guarrantee that the style is up-to-date in begin_litteral
	 *  But we still want to strip \r's
	 */
	if (c != '\r' &&
	    !(c == '\n' && B_inLABEL && !B_inP) &&
	    !(c == '\n' && !B_inPRE)) {
	    B_inP = TRUE;
	    B_inLABEL = FALSE; 
	    HText_appendCharacter(me->text, c);
	}
	B_inPRE = TRUE;
	break;

    case HTML_LISTING:				/* Litteral text */
    case HTML_XMP:
    case HTML_PLAINTEXT:
	/*
	 *  We guarrantee that the style is up-to-date in begin_litteral
	 *  But we still want to strip \r's
	 */
	if (c != '\r')	{
	    B_inP = TRUE;
	    B_inLABEL = FALSE; 
	    HText_appendCharacter(me->text, c);
	}
	break;
	
    default:
        /*
	 *  Free format text.
	 */
	if (!strcmp(me->sp->style->name,"Preformatted")) {
	    if (c != '\r' &&
	        !(c == '\n' && B_inLABEL && !B_inP) &&
		!(c == '\n' && !B_inPRE)) {
		B_inP = TRUE; 
		B_inLABEL = FALSE; 
	        HText_appendCharacter(me->text, c);
	    }
	    B_inPRE = TRUE;

	} else if (!strcmp(me->sp->style->name,"Listing") ||
		   !strcmp(me->sp->style->name,"Example") ||
		   !strcmp(me->sp->style->name,"Style")) {
	    if (c != '\r') {
		B_inP = TRUE; 
		B_inLABEL = FALSE; 
	        HText_appendCharacter(me->text, c);
	    }
	
	} else {
	    if (me->style_change) {
	        if ((c == '\n') || (c == ' '))
		    return;	/* Ignore it */
	        UPDATE_STYLE;
	    }
	    if (c == '\n') {
	        if (me->in_word) {
	            if (HTML_Last_Char != ' ') {
			B_inP = TRUE;
			B_inLABEL = FALSE;
		        HText_appendCharacter(me->text, ' ');
		    }
		    me->in_word = NO;
	        }

	    } else if (c == ' ' || c == '\t') {
	        if (HTML_Last_Char != ' ') {
		    B_inP = TRUE;
		    B_inLABEL = FALSE; 
	            HText_appendCharacter(me->text, ' ');
		}

	    } else if (c == '\r') {
	       /* ignore */

	    } else {
		B_inP = TRUE;
		B_inLABEL = FALSE;
	        HText_appendCharacter(me->text, c);
	        me->in_word = YES;
	    }
	}
    } /* end second switch */

    if (c == '\n' || c == '\t') {
     	HTML_Last_Char = ' '; /* set it to a generic seperater */

	/*
	 *  \r's are ignored.  In order to keep collapsing spaces
	 *  correctly we must default back to the previous
	 *  seperater if there was one
	 */
    } else if (c == '\r' && HTML_Last_Char == ' ') {
     	HTML_Last_Char = ' '; /* set it to a generic seperater */
    } else {
     	HTML_Last_Char = c;
    }
}

/*	String handling
**	---------------
**
**	This is written separately from put_character becuase the loop can
**	in some cases be promoted to a higher function call level for speed.
*/
PRIVATE void HTML_put_string ARGS2(HTStructured *, me, CONST char*, s)
{
   if (LYMapsOnly || s == NULL)
      return;

    switch (me->sp[0].tag_number) {

    case HTML_COMMENT:
    	break;					/* Do Nothing */
	
    case HTML_TITLE:
    	HTChunkPuts(&me->title, s);
	break;

    case HTML_STYLE:
	HTChunkPuts(&me->style_block, s);
        break;

    case HTML_SCRIPT:
    	HTChunkPuts(&me->script, s);
        break;

    case HTML_PRE:				/* Formatted text */
    case HTML_LISTING:				/* Litteral text */
    case HTML_XMP:
    case HTML_PLAINTEXT:
	/*
	 *  We guarrantee that the style is up-to-date in begin_litteral
	 */
    	HText_appendText(me->text, s);
	break;
	
    case HTML_OBJECT:	
    	HTChunkPuts(&me->object, s);
	break;

    case HTML_TEXTAREA:	
    	HTChunkPuts(&me->textarea, s);
	break;

    case HTML_SELECT:	
    	HTChunkPuts(&me->option, s);
	break;
	
    case HTML_MATH:
    	HTChunkPuts(&me->math, s);
	break;

    default:					/* Free format text */
        {
	    CONST char *p = s;
	    if (me->style_change) {
		for (; *p &&
		       ((*p == '\n') || (*p == ' ') || (*p == '\t')); p++)
		    ;	/* Ignore leaders */
		if (!*p)
		    return;
		UPDATE_STYLE;
	    }
	    for (; *p; p++) {
		if (me->style_change) {
		    if ((*p == '\n') || (*p == ' ') || (*p == '\t')) 
			continue;  /* Ignore it */
		    UPDATE_STYLE;
		}
		if (*p == '\n') {
		    if (me->in_word) {
		        if (HTML_Last_Char != ' ')
			    HText_appendCharacter(me->text, ' ');
			me->in_word = NO;
		    }

		} else if (*p == ' ' || *p == '\t') {
		   if (HTML_Last_Char != ' ')
			HText_appendCharacter(me->text, ' ');
			
		} else if (*p == '\r') {
			/* ignore */
		} else {
		    HText_appendCharacter(me->text, *p);
		    me->in_word = YES;
		}

		/* set the Last Character */
    		if (*p == '\n' || *p == '\t') {
        	    HTML_Last_Char = ' '; /* set it to a generic seperater */
    		} else if (*p == '\r' && HTML_Last_Char == ' ') {
		    /* 
		     *  \r's are ignored.  In order to keep collapsing
		     *  spaces correctly, we must default back to the
		     *  previous seperator, if there was one.
		     */
       		    HTML_Last_Char = ' '; /* set it to a generic seperater */
    		} else {
       		    HTML_Last_Char = *p;
    		}

	    } /* for */
	}
    } /* end switch */
}

/*	Buffer write
**	------------
*/
PRIVATE void HTML_write ARGS3(HTStructured *, me, CONST char*, s, int, l)
{
    CONST char* p;
    CONST char* e = s+l;

    if (LYMapsOnly)
        return;

    for (p = s; s < e; p++)
        HTML_put_character(me, *p);
}

/*	Start Element
**	-------------
*/
PRIVATE void HTML_start_element ARGS5(
	HTStructured *, 	me,
	int,			element_number,
	CONST BOOL*,	 	present,
	CONST char **,		value,
	char **,		include)
{
    char *alt_string = NULL;
    char *id_string = NULL;
    char *href = NULL;
    char *map_href = NULL;
    char *title = NULL;
    char *temp = NULL;
    static BOOLEAN first_option = TRUE;	     /* is this the first option tag? */
    static HTChildAnchor *B_CurrentA = NULL; /* current HTML_A anchor */
    HTParentAnchor *dest = NULL;	     /* the anchor's destination */
    BOOL dest_ismap = FALSE;	     	     /* is dest an image map script? */
    HTChildAnchor *B_ID_A = NULL;	     /* HTML_foo_ID anchor */
    int url_type;

    if (LYMapsOnly) {
        if (!(element_number == HTML_MAP || element_number == HTML_AREA)) {
	    return;
	}
    }

    switch (element_number) {

    case HTML_HTML:
        UPDATE_STYLE;
	List_Nesting_Level = -1;
	HTML_zero_OL_Counter();
	Division_Level = -1;
	Underline_Level = 0;
	Quote_Level = 0;
	if (B_inUnderline) {
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    B_inUnderline = FALSE;
	}
	FREE(base_href);
        B_inBASE = FALSE;
	break;

    case HTML_HEAD:
        UPDATE_STYLE;
	List_Nesting_Level = -1;
	HTML_zero_OL_Counter();
	Division_Level = -1;
	Underline_Level = 0;
	Quote_Level = 0;
	if (B_inUnderline) {
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    B_inUnderline = FALSE;
	}
	FREE(base_href);
        B_inBASE = FALSE;
	break;

    case HTML_BASE:
        if (present && present[HTML_BASE_HREF] &&
	    value[HTML_BASE_HREF] && *value[HTML_BASE_HREF]) {
	    char *base = NULL;
	    char *related = NULL;

	    /* 
	     *  Get parent's address for defaulted fields.
	     */
	    StrAllocCopy(related, me->node_anchor->address);

	    /* 
	     *  Create the access field.
	     */
	    StrAllocCopy(base, (char *)value[HTML_BASE_HREF]);
	    convert_to_spaces(base);
	    LYUnEscapeToLatinOne(&base, TRUE);
	    if ((temp = HTParse(base, related,
	    			PARSE_ACCESS+PARSE_PUNCTUATION)) &&
		*temp != '\0')
	        StrAllocCopy(base_href, temp);
	    else
	        StrAllocCopy(base_href, (temp = HTParse(related, "",
					 PARSE_ACCESS+PARSE_PUNCTUATION)));

	    /*
	     *  Create the host[:port] field.
	     */
	    if ((temp = HTParse(base, "",
	    			PARSE_HOST+PARSE_PUNCTUATION)) &&
	    	!strncmp(temp, "//", 2)) {
	        StrAllocCat(base_href, temp);
		if (!strcmp(base_href, "file://"))
		    StrAllocCat(base_href, "localhost");
	    } else {
	        if (!strcmp(base_href, "file:"))
		    StrAllocCat(base_href, "//localhost");
		else
	            StrAllocCat(base_href, (temp = HTParse(related, "",
					    PARSE_HOST+PARSE_PUNCTUATION)));
	    }

	    /*
	     *  Create the path field.
	     */
	    if ((temp = HTParse(base, "",
	    			PARSE_PATH+PARSE_PUNCTUATION)) &&
		*temp != '\0')
	        StrAllocCat(base_href, temp);
	    else
	        StrAllocCat(base_href, "/");

            B_inBASE = TRUE;

	    FREE(base);
	    FREE(related);
	    FREE(temp);
	}
	break;

    case HTML_META:
        if (!me->text)
	    UPDATE_STYLE;

	if (present) {
	    char *http_equiv = NULL, *name = NULL, *content = NULL;
	    /*
	     *  Load the attributes for possible use by Lynx. - FM
	     */
	    if (present[HTML_META_HTTP_EQUIV] &&
		value[HTML_META_HTTP_EQUIV] && *value[HTML_META_HTTP_EQUIV]) {
		StrAllocCopy(http_equiv, value[HTML_META_HTTP_EQUIV]);
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
		LYUnEscapeToLatinOne(&http_equiv, FALSE);
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
		LYTrimHead(content);
		LYTrimTail(content);
		if (*content == '\0') {
		    FREE(content);
		}
	    }

	    if (TRACE) {
	        fprintf(stderr,
		"HTML: META HTTP-EQUIV=\"%s\" NAME=\"%s\" CONTENT=\"%s\"\n",
			(http_equiv ? http_equiv : "NULL"),
			(name ? name : "NULL"),
			(content ? content : "NULL"));
	    }

	    /*
	     *  Add META-handling code or function calls here. - FM
	     */
	    if ((http_equiv || name) && content) {
		char *cp, *cp1;

	        /*
		 * Check for a no-cache Pragma
		 * or Cache-Control directive. - FM
		 */
	    	if (!strcasecomp((name ? name : http_equiv),
				 "Pragma") ||
		    !strcasecomp((name ? name : http_equiv),
		     		 "Cache-Control")) {
		    LYUnEscapeToLatinOne(&content, FALSE);
		    LYTrimHead(content);
		    LYTrimTail(content);
		    if (!strcasecomp(content, "no-cache")) {
		        me->node_anchor->no_cache = TRUE;
		        HText_setNoCache(me->text);
		    }

	            /*
		     *  If we didn't get a Cache-Control MIME
		     *  header, and the META has one, store it
		     *  in the anchor element. - FM
		     */
		    if ((!me->node_anchor->cache_control) &&
			!strcasecomp((name ? name : http_equiv),
				     "Cache-Control")) {
			StrAllocCopy(me->node_anchor->cache_control,
				     (name ? name : http_equiv));
		    }

	        /*
		 *  Check for a text/html Content-Type with a
		 *  charset directive, if we didn't already set
		 *  the charset via a server's header. - AAC & FM
		 */
		} else if (!(me->node_anchor->charset &&
			     *me->node_anchor->charset) && 
			   !strcasecomp((name ? name : http_equiv),
					"Content-Type")) {
		    LYUnEscapeToLatinOne(&content, FALSE);
		    LYTrimHead(content);
		    LYTrimTail(content);
		    /*
		     *  Force the Content-type value to all lower case. - FM
		     */
		    for (cp = content; *cp; cp++)
		        *cp = TOLOWER(*cp);

		    if ((cp=strstr(content, "text/html;")) != NULL &&
			(cp1=strstr(content, "charset")) != NULL &&
			cp1 > cp) {
			cp1 += 7;
			while (*cp1 == ' ' || *cp1 == '=')
			    cp1++;
			if (!strncmp(cp1, "us-ascii", 8) ||
			    !strncmp(cp1, "iso-8859-1", 10)) {
			    StrAllocCopy(me->node_anchor->charset,
					 "iso-8859-1");
			    HTCJK = NOCJK;
			} else if (!strncmp(cp1, "iso-8859-2", 10) &&
				   !strncmp(LYchar_set_names[current_char_set],
				   	    "ISO Latin 2", 11)) {
			    StrAllocCopy(me->node_anchor->charset,
					 "iso-8859-2");
			    HTPassEightBitRaw = TRUE;
			} else if (!strncmp(cp1, "iso-8859-", 9) &&
				   !strncmp(LYchar_set_names[current_char_set],
				   	    "Other ISO Latin", 15)) {
			    /*
			    **  Hope it's a match, for now. - FM
			    */
			    StrAllocCopy(me->node_anchor->charset,
					 "iso-8859- ");
			    me->node_anchor->charset[9] = cp1[9];
			    HTPassEightBitRaw = TRUE;
			    HTAlert(me->node_anchor->charset);
			} else if (!strncmp(cp1, "koi8-r", 6) &&
				   !strncmp(LYchar_set_names[current_char_set],
					    "KOI8-R character set", 20)) {
			    StrAllocCopy(me->node_anchor->charset,
					 "koi8-r");
			    HTPassEightBitRaw = TRUE;
			} else if (!strncmp(cp1, "euc-jp", 6) &&
			           HTCJK == JAPANESE) {
			    StrAllocCopy(me->node_anchor->charset,
					 "euc-jp");
			} else if (!strncmp(cp1, "shift_jis", 9) &&
			           HTCJK == JAPANESE) {
			    StrAllocCopy(me->node_anchor->charset,
					 "shift_jis");
			} else if (!strncmp(cp1, "iso-2022-jp", 11) &&
			           HTCJK == JAPANESE) {
			    StrAllocCopy(me->node_anchor->charset,
					 "iso-2022-jp");
			} else if (!strncmp(cp1, "iso-2022-jp-2", 13) &&
			           HTCJK == JAPANESE) {
			    StrAllocCopy(me->node_anchor->charset,
					 "iso-2022-jp-2");
			} else if (!strncmp(cp1, "euc-kr", 6) &&
			           HTCJK == KOREAN) {
			    StrAllocCopy(me->node_anchor->charset,
					 "euc-kr");
			} else if (!strncmp(cp1, "iso-2022-kr", 11) &&
			           HTCJK == KOREAN) {
			    StrAllocCopy(me->node_anchor->charset,
					 "iso-2022-kr");
			} else if ((!strncmp(cp1, "big5", 4) ||
				    !strncmp(cp1, "cn-big5", 7)) &&
			           HTCJK == TAIPEI) {
			    StrAllocCopy(me->node_anchor->charset,
					 "big5");
			} else if (!strncmp(cp1, "euc-cn", 6) &&
			           HTCJK == CHINESE) {
			    StrAllocCopy(me->node_anchor->charset,
					 "euc-cn");
			} else if ((!strncmp(cp1, "gb2312", 6) ||
				    !strncmp(cp1, "cn-gb", 5)) &&
			           HTCJK == CHINESE) {
			    StrAllocCopy(me->node_anchor->charset,
					 "gb2312");
			} else if (!strncmp(cp1, "iso-2022-cn", 11) &&
			           HTCJK == CHINESE) {
			    StrAllocCopy(me->node_anchor->charset,
					 "iso-2022-cn");
			}
			if (TRACE && me->node_anchor->charset) {
			    fprintf(stderr, "HTML: New charset: %s\n",
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
		} else if (!strcasecomp((name ? name : http_equiv),
					"Refresh")) {
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
				while (*cp && (*cp == '=' ||
					       isspace((unsigned char)*cp)))
				    cp++;
				cp1 = cp;
				while (*cp1 && !isspace((unsigned char)*cp1))
				    cp1++;
				*cp1 = '\0';
				if (*cp) {
				    StrAllocCopy(href, cp);
				    convert_to_spaces(href);
				    /*
				     *  Translate any named or numeric
				     *  character references with the
				     *  isURL flag set. - FM
				     */
				    LYUnEscapeToLatinOne(&href, TRUE);
				}
				break;
			    }
			    cp1++;
			}
			if (href) {
			    /*
			     *  We found a URL field, so check it out. - FM
			     */
			    if (!is_url(href)) {
			        /*
				 *  The specs require a complete URL,
				 *  but this is a Netscapism, so don't
				 *  expect the author to know that. - FM
				 */
				if (*href != '\0' && *href != '/')
				    HTSimplify(href);
				/*
				 *  Use the document's address
				 *  as the base. - FM
				 */
				if (*href != '\0') {
				    temp = HTParse(href,
					           me->node_anchor->address,
					           PARSE_ALL);
				    StrAllocCopy(href, temp);
				    FREE(temp);
				} else {
				    StrAllocCopy(href,
				    		 me->node_anchor->address);
				    HText_setNoCache(me->text);
				}
			    }
			    /*
			     *  Check whether to fill in localhost. - FM
			     */
			    HTMLFillLocalFileURL((char **)&href,
						 (B_inBASE ? base_href :
						  me->node_anchor->address));
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
			B_CurrentA = HTAnchor_findChildAndLink(
				me->node_anchor,		/* Parent */
				(id_string ? id_string : 0),	/* Tag */
				href,				/* Addresss */
				0);				/* Type */
			if (id_string)
			    *cp = '#';
			FREE(id_string);
			HTML_EnsureSingleSpace(me);
			if (B_inUnderline == FALSE)
			    HText_appendCharacter(me->text,
			    			  LY_UNDERLINE_START_CHAR);
			HTML_put_string(me, "REFRESH(");
			HTML_put_string(me, Seconds);
			HTML_put_string(me, " sec):");
			FREE(Seconds);
			if (B_inUnderline == FALSE)
			    HText_appendCharacter(me->text,
			    			  LY_UNDERLINE_END_CHAR);
			HTML_put_character(me, ' ');
			me->in_word = NO;
			HText_beginAnchor(me->text, B_CurrentA);
			if (B_inBoldH == FALSE)
			    HText_appendCharacter(me->text, LY_BOLD_START_CHAR);
			HTML_put_string(me, href);
			FREE(href);
			if (B_inBoldH == FALSE)
			    HText_appendCharacter(me->text, LY_BOLD_END_CHAR);
			HText_endAnchor(me->text);
			HTML_EnsureSingleSpace(me);
		    }
		}
	    }

	    /*
	     *  Free the copies. - FM
	     */
	    FREE(http_equiv);
	    FREE(name);
	    FREE(content);
	}
	break;

    case HTML_TITLE:
        HTChunkClear(&me->title);
	List_Nesting_Level = -1;
	HTML_zero_OL_Counter();
	Division_Level = -1;
	Underline_Level = 0;
	Quote_Level = 0;
	if (B_inUnderline) {
	    if (!me->text)
	        UPDATE_STYLE;
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    B_inUnderline = FALSE;
	}
	break;

    case HTML_LINK:
	if (present && present[HTML_LINK_HREF]) {
	    /*
	     *  Prepare to do housekeeping on the reference. - FM
	     */
	    if (!(value[HTML_LINK_HREF] && *value[HTML_LINK_HREF])) {
		if (B_inBASE && base_href && *base_href) {
		    StrAllocCopy(href, base_href);
		} else {
		    StrAllocCopy(href, me->node_anchor->address);
		}
	    } else {
		StrAllocCopy(href, value[HTML_LINK_HREF]);
		convert_to_spaces(href);
		LYUnEscapeToLatinOne(&href, TRUE);
	    }
	    url_type = is_url(href);

	    /*
	     *  Don't simplify absolute HREFs. - FM
	     */
	    if (!url_type && *href != '/' && *href != '\0')
		HTSimplify(href);

	    /*
	     *  Check whether a base tag is in effect. - FM
	     */
	    if ((B_inBASE) &&
		(temp = HTParse(href, base_href, PARSE_ALL)) &&
		*temp != '\0')
		/*
		 *  Use reference related to the base.
		 */
		StrAllocCopy(href, temp);
	    FREE(temp);

	    /*
	     *  Check whether to fill in localhost. - FM
	     */
	    HTMLFillLocalFileURL((char **)&href,
				 (B_inBASE ? base_href :
					     me->node_anchor->address));

	    /*
	     *  Handle REV="made" or REV="owner". - LM & FM
	     */
	    if (present &&
	        present[HTML_LINK_REV] && value[HTML_LINK_REV]) {
	        if (!strcasecomp("made", value[HTML_LINK_REV]) ||
		    !strcasecomp("owner", value[HTML_LINK_REV]))
		    HTAnchor_setOwner(me->node_anchor, href);
		/*
		 *  Load the RevTitle element if a TITLE attribute
		 *  and value are present. - FM
		 */ 
		if (present && present[HTML_LINK_TITLE] &&
		    value[HTML_LINK_TITLE] &&
		    *value[HTML_LINK_TITLE] != '\0') {
		    StrAllocCopy(title, value[HTML_LINK_TITLE]);
		    if (current_char_set)
		        LYExpandString(&title);
		    /*
		     *  Convert any HTML entities or decimal escaping. - FM
		     */
		    LYUnEscapeEntities(title, TRUE, FALSE);
		    LYTrimHead(title);
		    LYTrimTail(title);
		    if (*title != '\0')
		        HTAnchor_setRevTitle(me->node_anchor, title);
		    FREE(title);
		}

		if (TRACE)
		    fprintf(stderr,"HTML.c: DOC OWNER found\n");
	    }

	    /*
	     *  Handle REL links. - FM
	     */
	    if (present &&
	        present[HTML_LINK_REL] && value[HTML_LINK_REL]) {
		
		/*
		 *  Ignore style sheets, for now. - FM
		 */
		if (!strcasecomp(value[HTML_LINK_REL], "StyleSheet")) {
		    if (TRACE) {
		        fprintf(stderr,
				"HTML.c: StyleSheet link found.\n");
		        fprintf(stderr,
				"        StyleSheets not yet implemented.\n");
		    }
		    FREE(href);
		    break;
		}

		/*
		 *  Ignore anything not registered as of 28-Mar-95
		 *  IETF specs.  We'll make this more efficient when
		 *  the situation stabilizes, and for now, we'll treat
		 *  "Banner" as another toolbar element. - FM
		 */
		if (strcasecomp(value[HTML_LINK_REL], "Home") &&
		    strcasecomp(value[HTML_LINK_REL], "ToC") &&
		    strcasecomp(value[HTML_LINK_REL], "Index") &&
		    strcasecomp(value[HTML_LINK_REL], "Glossary") &&
		    strcasecomp(value[HTML_LINK_REL], "Copyright") &&
		    strcasecomp(value[HTML_LINK_REL], "Up") &&
		    strcasecomp(value[HTML_LINK_REL], "Next") &&
		    strcasecomp(value[HTML_LINK_REL], "Previous") &&
		    strcasecomp(value[HTML_LINK_REL], "Help") &&
		    strcasecomp(value[HTML_LINK_REL], "Bookmark") &&
		    strcasecomp(value[HTML_LINK_REL], "Banner")) {
		    if (TRACE) {
		        fprintf(stderr,
				"HTML.c: LINK with REL=\"%s\" ignored.\n",
				 value[HTML_LINK_REL]);
		    }
		    FREE(href);
		    break;
		}

		/*
		 *  Create a title (link name) from the TITLE value,
		 *  if present, or default to the REL value. - FM
		 */ 
		if (present && present[HTML_LINK_TITLE] &&
		    value[HTML_LINK_TITLE] && *value[HTML_LINK_TITLE] != '\0')
		    StrAllocCopy(title, value[HTML_LINK_TITLE]);
		else
		    StrAllocCopy(title, value[HTML_LINK_REL]);
		if (current_char_set)
		    LYExpandString(&title);
		/*
		 *  Convert any HTML entities or decimal escaping. - FM
		 */
		LYUnEscapeEntities(title, TRUE, FALSE);
		LYTrimHead(title);
		LYTrimTail(title);

		/*
		 *  Create anchors for the links that simulate
		 *  a toolbar. - FM
		 */
	        B_CurrentA = HTAnchor_findChildAndLink(
				me->node_anchor,	/* Parent */
		    		0,			/* Tag */
		    		href ? href : 0,	/* Addresss */
		    		0);			/* Type */
		{
		    if (dest = HTAnchor_parent(
			    HTAnchor_followMainLink((HTAnchor*)B_CurrentA)
			    		      )) {
		        if (!HTAnchor_title(dest))
			    HTAnchor_setTitle(dest, title);
		    }
		    UPDATE_STYLE;
		    if (!HText_hasToolbar(me->text) &&
		        (B_ID_A = HTAnchor_findChildAndLink(
					me->node_anchor,	/* Parent */
					LYToolbarName,		/* Tag */
					0,			/* Addresss */
					0))) {			/* Type */
			HText_beginAnchor(me->text, B_ID_A);
			HText_endAnchor(me->text);
			HText_setToolbar(me->text);
		    }
		    HText_beginAnchor(me->text, B_CurrentA);
		    if (B_inBoldH == FALSE)
		        HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		    HTML_put_string(me, title);
		    if (B_inBoldH == FALSE)
		        HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		    HText_endAnchor(me->text);
		    dest = NULL;
		}
		FREE(title);
	    }
	    FREE(href);
	}
	break;

    case HTML_ISINDEX:
	if (present &&
	    ((present[HTML_ISINDEX_HREF] &&
	      value[HTML_ISINDEX_HREF] && *value[HTML_ISINDEX_HREF]) ||
	    (present[HTML_ISINDEX_ACTION] &&
	     value[HTML_ISINDEX_ACTION] && *value[HTML_ISINDEX_ACTION]))) {
	    char * action = NULL;
	    char * isindex_href = NULL;

	    /*
	     *  Lynx was supporting ACTION, which never made it into
	     *  the HTTP 2.0 specs.  HTTP 3.0 uses HREF, so we'll
	     *  use that too, but allow use of ACTION as an alternate
	     *  until people have fully switched over. - FM
	     */
	    if (present[HTML_ISINDEX_HREF])
	        StrAllocCopy(isindex_href, value[HTML_ISINDEX_HREF]);
	    else
	        StrAllocCopy(isindex_href, value[HTML_ISINDEX_ACTION]);
	    convert_to_spaces(isindex_href);
	    LYUnEscapeToLatinOne(&isindex_href, TRUE);
	    url_type = is_url(isindex_href);
	    if (!url_type && *isindex_href != '/' && *isindex_href != '\0')
	        HTSimplify(isindex_href);

	    /*
	     *  Check whether a base tag is in effect.
	     */
	    if (B_inBASE)
		action = HTParse(isindex_href, base_href, PARSE_ALL);
	    if (!(action && *action)) {
	        char *related = NULL;

		StrAllocCopy(related, me->node_anchor->address);
	        action = HTParse(isindex_href, related, PARSE_ALL);
		FREE(related);
	    }
	    if (action && *action) {
   	        HTAnchor_setIndex(me->node_anchor, action);
	    } else if (B_inBASE) {
	        HTAnchor_setIndex(me->node_anchor, base_href);
	    } else {
	        HTAnchor_setIndex(me->node_anchor, me->node_anchor->address);
	    }
	    FREE(isindex_href);
	    FREE(action);

	} else {
	    if (B_inBASE)
	        /*
	         *  Use base.
	         */
   	        HTAnchor_setIndex(me->node_anchor, base_href);
	    else
	        /*
	         *  Use index's address.
	         */
   	        HTAnchor_setIndex(me->node_anchor, me->node_anchor->address);
	}
	/*
	 *  Support HTML 3.0 PROMPT attribute. - FM
	 */
	if (present &&
	    present[HTML_ISINDEX_PROMPT] &&
	    value[HTML_ISINDEX_PROMPT] && *value[HTML_ISINDEX_PROMPT]) {
	    StrAllocCopy(temp, value[HTML_ISINDEX_PROMPT]);
	    if (current_char_set)
		LYExpandString(&temp);
	    /*
	     *  Convert any HTML entities or decimal escaping. - FM
	     */
	    LYUnEscapeEntities(temp, TRUE, FALSE);
	    LYTrimHead(temp);
	    LYTrimTail(temp);
	    if (*temp != '\0') {
	        StrAllocCat(temp, " ");
		HTAnchor_setPrompt(me->node_anchor, temp);
	    } else {
	        HTAnchor_setPrompt(me->node_anchor,
				   "Enter a database query: ");
	    }
	    FREE(temp);
	} else {
	    HTAnchor_setPrompt(me->node_anchor, "Enter a database query: ");
	}
	break;

    case HTML_NEXTID:
	if (!me->text)
	    UPDATE_STYLE;
    	/* if (present && present[NEXTID_N] && value[NEXTID_N])
		HText_setNextId(me->text, atoi(value[NEXTID_N])); */
    	break;

    case HTML_STYLE:
    	/*
	 *  We're getting it as Litteral text, which, for now,
	 *  we'll just ignore. - FM
	 */
	if (!me->text)
	    UPDATE_STYLE;
	HTChunkClear(&me->style_block);
	break;

    case HTML_SCRIPT:
    	/*
	 *  We're getting it as Litteral text, which, for now,
	 *  we'll just ignore. - FM
	 */
	if (!me->text)
	    UPDATE_STYLE;
	HTChunkClear(&me->script);
	break;

    case HTML_BODY:
        UPDATE_STYLE;
	List_Nesting_Level = -1;
	HTML_zero_OL_Counter();
	Division_Level = -1;
	Underline_Level = 0;
	Quote_Level = 0;
	if (B_inUnderline) {
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    B_inUnderline = FALSE;
	}
	HTML_CheckForID(me, present, value, (int)HTML_BODY_ID);
	break;

    case HTML_FRAMESET:
	if (!me->text)
	    UPDATE_STYLE;
	break;

    case HTML_FRAME:
	if (!me->text)
	    UPDATE_STYLE;
	if (present && present[HTML_FRAME_NAME] &&
	    value[HTML_FRAME_NAME] && *value[HTML_FRAME_NAME]) {
	    StrAllocCopy(id_string, value[HTML_FRAME_NAME]);
	    if (current_char_set)
		LYExpandString(&id_string);
	    /*
	     *  Convert any HTML entities or decimal escaping. - FM
	     */
	    LYUnEscapeEntities(id_string, TRUE, FALSE);
	    LYTrimHead(id_string);
	    LYTrimTail(id_string);
	}
	if (present && present[HTML_FRAME_SRC] &&
	    value[HTML_FRAME_SRC] && *value[HTML_FRAME_SRC] != '\0') {
	    StrAllocCopy(href, value[HTML_FRAME_SRC]);
	    convert_to_spaces(href);
	    LYUnEscapeToLatinOne(&href, TRUE);
	    url_type = is_url(href);
	    if (!url_type && *href != '/' && *href != '\0')
	        HTSimplify(href);

	    /*
	     *  Check whether a base tag is in effect. - FM
	     */
	    if ((B_inBASE) &&
		(temp = HTParse(href, base_href, PARSE_ALL)) &&
		*temp != '\0')
		/*
		 *  Use reference related to the base.
		 */
		StrAllocCopy(href, temp);
	    FREE(temp);

	    /*
	     *  Check whether to fill in localhost. - FM
	     */
	    HTMLFillLocalFileURL((char **)&href,
				 (B_inBASE ? base_href :
					     me->node_anchor->address));

	    B_CurrentA = HTAnchor_findChildAndLink(
				me->node_anchor,	/* Parent */
				0,			/* Tag */
				href,			/* Addresss */
				0);			/* Type */
	    HTML_EnsureSingleSpace(me);
	    if (B_inUnderline == FALSE)
	        HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	    HTML_put_string(me, "FRAME:");
	    if (B_inUnderline == FALSE)
	        HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    HTML_put_character(me, ' ');
	    me->in_word = NO;
	    HText_beginAnchor(me->text, B_CurrentA);
	    if (B_inBoldH == FALSE)
		HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
	    HTML_put_string(me, (id_string ? id_string : href));
	    FREE(href);
	    FREE(id_string);
	    if (B_inBoldH == FALSE)
		HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
	    HText_endAnchor(me->text);
	    HTML_EnsureSingleSpace(me);
	}
	break;

    case HTML_NOFRAMES:
	if (!me->text)
	    UPDATE_STYLE;
	HTML_EnsureDoubleSpace(me);
	HTML_ResetParagraphAlignment(me);
	break;

    case HTML_BANNER:
    case HTML_MARQUEE:
    	change_paragraph_style(me, styles[HTML_BANNER]);
	UPDATE_STYLE;
	if (!HText_hasToolbar(me->text) &&
	    (B_ID_A = HTAnchor_findChildAndLink(
					me->node_anchor,	/* Parent */
					LYToolbarName,		/* Tag */
					0,			/* Addresss */
					0))) {			/* Type */
	    HText_beginAnchor(me->text, B_ID_A);
	    HText_endAnchor(me->text);
	    HText_setToolbar(me->text);
	}
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
	break;

    case HTML_CENTER:
    case HTML_DIV:
	if (Division_Level < MAX_NESTING) {
	    Division_Level++;
	} else if (TRACE) {
            fprintf(stderr, 
		"HTML: ****** Maximum nesting of %d divisions exceeded!\n",
            	MAX_NESTING);
	}
	if (element_number == HTML_CENTER) {
	    DivisionAlignments[Division_Level] = HT_CENTER;
    	    change_paragraph_style(me, styles[HTML_DCENTER]);
	    UPDATE_STYLE;
	    current_default_alignment = styles[HTML_DCENTER]->alignment;
	} else if (present && present[HTML_DIV_ALIGN] &&
		   value[HTML_DIV_ALIGN] && *value[HTML_DIV_ALIGN]) {
	    if (!strcasecomp(value[HTML_DIV_ALIGN], "center")) {
	        DivisionAlignments[Division_Level] = HT_CENTER;
		change_paragraph_style(me, styles[HTML_DCENTER]);
		UPDATE_STYLE;
		current_default_alignment = styles[HTML_DCENTER]->alignment;
	    } else if (!strcasecomp(value[HTML_DIV_ALIGN], "right")) {
	        DivisionAlignments[Division_Level] = HT_RIGHT;
		change_paragraph_style(me, styles[HTML_DRIGHT]);
		UPDATE_STYLE;
		current_default_alignment = styles[HTML_DRIGHT]->alignment;
	    } else {
	        DivisionAlignments[Division_Level] = HT_LEFT;
		change_paragraph_style(me, styles[HTML_DLEFT]);
		UPDATE_STYLE;
		current_default_alignment = styles[HTML_DLEFT]->alignment;
	    }
	} else {
	    DivisionAlignments[Division_Level] = HT_LEFT;
	    change_paragraph_style(me, styles[HTML_DLEFT]);
	    UPDATE_STYLE;
	    current_default_alignment = styles[HTML_DLEFT]->alignment;
	}
	HTML_CheckForID(me, present, value, (int)HTML_DIV_ID);
	break;

    case HTML_H1:
    case HTML_H2:
    case HTML_H3:
    case HTML_H4:
    case HTML_H5:
    case HTML_H6:
	/*
	 *  Close the previous style if not done by HTML doc.
	 *  Added to get rid of core dumps in BAD HTML on the net.
	 *		GAB 07-07-94
	 *  But then again, these are actually allowed to nest.  I guess
	 *  I have to depend on the HTML writers correct style.
	 *		GAB 07-12-94
	if (i_prior_style != -1) {
	    HTML_end_element(me, i_prior_style);
	}
	i_prior_style = element_number;
	 */

	if (present && present[HTML_H_ALIGN] &&
	    value[HTML_H_ALIGN] && *value[HTML_H_ALIGN]) {
	    if (!strcasecomp(value[HTML_H_ALIGN], "center"))
	        change_paragraph_style(me, styles[HTML_HCENTER]);
	    else if (!strcasecomp(value[HTML_H_ALIGN], "right"))
	        change_paragraph_style(me, styles[HTML_HRIGHT]);
	    else if (!strcasecomp(value[HTML_H_ALIGN], "left") ||
	    	     !strcasecomp(value[HTML_H_ALIGN], "justify"))
	        change_paragraph_style(me, styles[HTML_HLEFT]);
	    else
	        change_paragraph_style(me, styles[element_number]);
	} else if (Division_Level >= 0) {
	    if (DivisionAlignments[Division_Level] == HT_CENTER) {
		change_paragraph_style(me, styles[HTML_HCENTER]);
	    } else if (DivisionAlignments[Division_Level] == HT_LEFT) {
		change_paragraph_style(me, styles[HTML_HLEFT]);
	    } else if (DivisionAlignments[Division_Level] == HT_RIGHT) {
		change_paragraph_style(me, styles[HTML_HRIGHT]);
	    }
	} else {
    	    change_paragraph_style(me, styles[element_number]);
	}
	UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_H_ID);
	    
	if ((bold_headers == TRUE ||
	     (element_number == HTML_H1 && bold_H1 == TRUE)) &&
	    (styles[element_number]->font&HT_BOLD)) {
	    if (B_inBoldA == FALSE && B_inBoldH == FALSE) {
	        HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
	    }
	    B_inBoldH = TRUE;
	}
	break;

    case HTML_P:
	/*
	 *  FIG content should be a true block, which like P inherits
	 *  the current style.  APPLET is like character elements or
	 *  an ALT attribute, unless it content contains a block element.
	 *  If we encounter a P in either's content, we set flags to treat
	 *  the content as a block.  - FM
	 */
	if (B_inFIG)
	    B_inFIGwithP = TRUE;

	if (B_inAPPLET)
	    B_inAPPLETwithP = TRUE;

	UPDATE_STYLE;
	if (List_Nesting_Level >= 0) {
	    /*
	     *  We're in a list.  Treat P as an instruction to
	     *  create one blank line, if not already present,
	     *  then fall through to handle attributes, with
	     *  the "second line" margins. - FM
	     */
	    if (B_inP) {
	        if (B_inFIG || B_inAPPLET ||
		    B_inCAPTION || B_inCREDIT ||
		    me->sp->style->spaceAfter > 0 ||
		    me->sp->style->spaceBefore > 0) {
	            HTML_EnsureDoubleSpace(me);
		} else {
	            HTML_EnsureSingleSpace(me);
		}
	    }
	} else if (me->sp[0].tag_number == HTML_ADDRESS) {
	    /*
	     *  We're in an ADDRESS. Treat P as an instruction 
	     *  to start a newline, if needed, then fall through
	     *  to handle attributes. - FM
	     */
	    if (HText_LastLineSize(me->text)) {
	        HText_appendCharacter(me->text, '\r');
	    }
	} else if (!(B_inLABEL && !B_inP)) {
	    HText_appendParagraph(me->text);
	    B_inLABEL = FALSE;
	}
	me->in_word = NO;

	if (HTML_override_default_alignment(me)) {
	    me->sp->style->alignment = styles[me->sp[0].tag_number]->alignment;
	} else if (List_Nesting_Level >= 0 ||
		   ((Division_Level < 0) &&
		    (!strcmp(me->sp->style->name, "Normal") ||
		     !strcmp(me->sp->style->name, "Preformatted")))) {
	        me->sp->style->alignment = HT_LEFT;
	} else {
	    me->sp->style->alignment = current_default_alignment;
	}
	if (present && present[HTML_P_ALIGN] && value[HTML_P_ALIGN]) {
	    if (!strcasecomp(value[HTML_P_ALIGN], "center") &&
	        !(List_Nesting_Level >= 0 && !B_inP))
	        me->sp->style->alignment = HT_CENTER;
	    else if (!strcasecomp(value[HTML_P_ALIGN], "right") &&
	        !(List_Nesting_Level >= 0 && !B_inP))
	        me->sp->style->alignment = HT_RIGHT;
	    else if (!strcasecomp(value[HTML_H_ALIGN], "left") ||
	    	     !strcasecomp(value[HTML_H_ALIGN], "justify"))
	        me->sp->style->alignment = HT_LEFT;
	}

	HTML_CheckForID(me, present, value, (int)HTML_P_ID);

	/*
	 *  Mark that we are starting a new paragraph
	 *  and don't have any of it's text yet. - FM
	 *
	 */
	B_inP = FALSE;

	break;

    case HTML_BR:
        UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
	HTML_Last_Char = ' ';  /* absorb white space */
	HText_appendCharacter(me->text, '\r');
	me->in_word = NO;
	B_inP = FALSE;
	break;

    case HTML_HR:
	{
	    register int i, width, old_alignment;

	    /*
	     *  Start a new line only if we had printable
	     *  characters following the previous newline,
	     *  and save the current style's alignment. - FM
	     */
	    UPDATE_STYLE;
	    if (HText_LastLineSize(me->text)) {
	        HText_appendCharacter(me->text, '\r');
	        me->in_word = NO;
		B_inP = FALSE;
	    }
	    old_alignment = me->sp->style->alignment;

	    /*
	     *  Add and ID link if needed. - FM
	     */
	    HTML_CheckForID(me, present, value, (int)HTML_HR_ID);

           /*
	    *  Center lines within the current margins, if
	    *  a right or left ALIGNment is not specified.
	    *  If WIDTH="#%" is given and not garbage,
	    *  use that to calculate the width, otherwise
	    *  use the default width. - FM
	    */
	    if (present && present[HTML_HR_ALIGN] && value[HTML_HR_ALIGN]) {
	        if (!strcasecomp(value[HTML_HR_ALIGN], "right")) {
		    me->sp->style->alignment = HT_RIGHT;
		} else if (!strcasecomp(value[HTML_HR_ALIGN], "left")) {
		    me->sp->style->alignment = HT_LEFT;
		} else {
		    me->sp->style->alignment = HT_CENTER;
		}
	    } else {
	        me->sp->style->alignment = HT_CENTER;
	    }
	    width = LYcols - 1 -
		    me->new_style->leftIndent - me->new_style->rightIndent;
	    if (present && present[HTML_HR_WIDTH] && value[HTML_HR_WIDTH] &&
	        isdigit(*value[HTML_HR_WIDTH]) &&
	        value[HTML_HR_WIDTH][strlen(value[HTML_HR_WIDTH])-1] == '%') {
	        char *percent = NULL;
		int Percent, Width;
		StrAllocCopy(percent, value[HTML_HR_WIDTH]);
		percent[strlen(percent)-1] = '\0';
		Percent = atoi(percent);
		if (Percent > 100 || Percent < 1)
		    width -= 5;
		else {
		    Width = (width * Percent) / 100;
		    if (Width < 1)
		        width = 1;
		    else
	                width = Width;
		}
		FREE(percent);
	    } else {
	        width -= 5;
	    }
	    for (i = 0; i < width; i++)
	        HTML_put_character(me, '_');

	    HText_appendCharacter(me->text, '\r');
	    HText_appendCharacter(me->text, '\r');
	    me->in_word = NO;
	    B_inP = FALSE;
	    me->sp->style->alignment = old_alignment;
	}
	break;

    case HTML_TAB:
        if (!present) { /* Bad tag.  Must have at least one attribute. - FM */
	    if (TRACE)
	        fprintf(stderr,
			"HTML: TAB tag has no attributes. Ignored.\n");
	    break;
	}
	UPDATE_STYLE;

	if (present[HTML_TAB_ALIGN] && value[HTML_TAB_ALIGN] &&
	    (strcasecomp(value[HTML_TAB_ALIGN], "left") ||
	     !(present[HTML_TAB_TO] || present[HTML_TAB_INDENT]))) {
	    /*
	     *  Just ensure a collapsible space, until we have
	     *  the ALIGN and DP attributes implemented. - FM
	     */
	    HTML_put_character(me, ' ');
	    if (TRACE)
	        fprintf(stderr,
		     "HTML: ALIGN not 'left'. Using space instead of TAB.\n");

	} else if (!HTML_override_default_alignment(me) &&
		   current_default_alignment != HT_LEFT) {
	    /*
	     *  Just ensure a collapsible space, until we
	     *  can replace HText_getCurrentColumn() in
	     *  GridText.c with code which doesn't require
	     *  that the alignment be HT_LEFT. - FM
	     */
	    HTML_put_character(me, ' ');
	    if (TRACE)
	        fprintf(stderr,
			"HTML: Not HT_LEFT. Using space instead of TAB.\n");

	} else if ((present[HTML_TAB_TO] &&
		    value[HTML_TAB_TO] && *value[HTML_TAB_TO]) ||
		   (present[HTML_TAB_INDENT] &&
		    value[HTML_TAB_INDENT] &&
		    isdigit(*value[HTML_TAB_INDENT]))) {
	    int i, column, target = -1;
	    int enval = 2;

	    column = HText_getCurrentColumn(me->text);
	    if (present[HTML_TAB_TO]) {
	        /*
		 *  TO has priority over INDENT if both are present. - FM
		 */
		StrAllocCopy(temp, value[HTML_TAB_TO]);
		LYUnEscapeToLatinOne(&temp, TRUE);
		if (*temp) {
		    target = HText_getTabIDColumn(me->text, temp);
		}
	    } else if (!(temp && *temp) && present[HTML_TAB_INDENT] &&
		       value[HTML_TAB_INDENT] &&
		       isdigit(*value[HTML_TAB_INDENT])) {
		/*
		 *  The INDENT value is in "en" (enval per column) units.
		 *  Divide it by enval, rounding odd values up. - FM
		 */
	        target =
		   (int)((((float)atoi(value[HTML_TAB_INDENT]))/enval)+(0.5));
	    }
	    /*
	     *  If we are being directed to a column too far to the left
	     *  or right, just add a collapsible space, otherwise, add the
	     *  appropriate number of spaces. - FM
	     */
	    if (target < column ||
		target > HText_getMaximumColumn(me->text)) {
	        HTML_put_character(me, ' ');
		if (TRACE)
		    fprintf(stderr,
		 "HTML: Column out of bounds. Using space instead of TAB.\n");
	    } else {
	        for (i = column; i < target; i++)
		    HText_appendCharacter(me->text, ' ');  
	    }
	}
	me->in_word = NO;

	/*
	 *  If we have an ID attribute, save it together
	 *  with the value of the column we've reached. - FM
	 */
	if (present[HTML_TAB_ID] &&
	    value[HTML_TAB_ID] && *value[HTML_TAB_ID]) {
	    StrAllocCopy(temp, value[HTML_TAB_ID]);
	    LYUnEscapeToLatinOne(&temp, TRUE);
	    if (*temp)
	        HText_setTabID(me->text, temp);
	    FREE(temp);
	}
	break;

    case HTML_BASEFONT:
	if (!me->text)
	    UPDATE_STYLE;
	break;

    case HTML_FONT:
	if (!me->text)
	    UPDATE_STYLE;
	break;

    case HTML_B:			/* Physical character highlighting */
    case HTML_BLINK:
    case HTML_I:
    case HTML_U:
    
    case HTML_CITE:			/* Logical character highlighting */
    case HTML_EM:
    case HTML_STRONG:
	UPDATE_STYLE;
	Underline_Level++;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
	/*
	 *  Ignore this if inside of a bold anchor or header.
	 *  Can't display both underline and bold at same time.
	 */
	if (B_inBoldA == TRUE || B_inBoldH == TRUE)	{
	    if (TRACE)
	        fprintf(stderr,"Underline Level is %d\n", Underline_Level);
	    break;
	}
	if (B_inUnderline == FALSE) {
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	    B_inUnderline = TRUE;
	    if (TRACE)
	        fprintf(stderr,"Beginning underline\n");
	} else {
	    if (TRACE)
	        fprintf(stderr,"Underline Level is %d\n", Underline_Level);
	}
    	break;
	
    case HTML_ABBREV:	/* Miscellaneous character containers */
    case HTML_ACRONYM:
    case HTML_AU:
    case HTML_AUTHOR:
    case HTML_BIG:
    case HTML_CODE:
    case HTML_DFN:
    case HTML_KBD:
    case HTML_SAMP:
    case HTML_SMALL:
    case HTML_SUB:
    case HTML_SUP:
    case HTML_TT:
    case HTML_VAR:
        if (!me->text)
	    UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
	break; /* ignore */

    case HTML_DEL:
    case HTML_S:
    case HTML_STRIKE:
        if (!me->text)
	    UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	HTML_put_string(me, "[DEL:");
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	HTML_put_character(me, ' ');
	me->in_word = NO;
	break;

    case HTML_INS:
        if (!me->text)
	    UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	HTML_put_string(me, "[INS:");
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	HTML_put_character(me, ' ');
	me->in_word = NO;
	break;

    case HTML_Q:
	if (!me->text)
	    UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
        /*
	 *  Should check LANG and/or DIR attributes, and the
	 *  me->node_anchor->charset and/or yet to be added
	 *  structure elements, to determine whether we should
	 *  use chevrons, but for now we'll always use double-
	 *  or single-quotes. - FM
	 */
	if (Quote_Level == ((Quote_Level/2)*2))
	    HText_appendCharacter(me->text, '"');
	else
	    HText_appendCharacter(me->text, '`');
	Quote_Level++;
	break;

    case HTML_PRE:				/* Formatted text */
        if (!HText_PreviousLineSize(me->text))
	    B_inPRE = FALSE;
	else
            B_inPRE = TRUE;
    case HTML_LISTING:				/* Litteral text */
    case HTML_XMP:
    case HTML_PLAINTEXT:
	change_paragraph_style(me, styles[element_number]);
	UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
    	if (me->comment_end)
    	    HText_appendText(me->text, me->comment_end);
	break;

    case HTML_BLOCKQUOTE:
    case HTML_BQ:
    	change_paragraph_style(me, styles[element_number]);
	UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_BQ_ID);
	break;

    case HTML_NOTE:
    	change_paragraph_style(me, styles[element_number]);
	UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_NOTE_ID);
	{
	    char *note = NULL;
	    int i;

	    /*
	     *  Indicate the type of NOTE.
	     */
	    if (present && present[HTML_NOTE_CLASS] &&
	        value[HTML_NOTE_CLASS] &&
	        (!strcasecomp(value[HTML_NOTE_CLASS], "CAUTION") ||
		 !strcasecomp(value[HTML_NOTE_CLASS], "WARNING"))) {
	        StrAllocCopy(note, value[HTML_NOTE_CLASS]);
		for (i = 0; note[i] != '\0'; i++)
		    note[i] = TOUPPER(note[i]);
		StrAllocCat(note, ":");
	    } else if (present && present[HTML_NOTE_ROLE] &&
	    	       value[HTML_NOTE_ROLE] &&
	    	       (!strcasecomp(value[HTML_NOTE_ROLE], "CAUTION") ||
		        !strcasecomp(value[HTML_NOTE_ROLE], "WARNING"))) {
	        StrAllocCopy(note, value[HTML_NOTE_ROLE]);
		for (i = 0; note[i] != '\0'; i++)
		    note[i] = TOUPPER(note[i]);
		StrAllocCat(note, ":");
	    } else {
	        StrAllocCopy(note, "NOTE:");
	    }
	    if (B_inUnderline == FALSE)
	        HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	    HTML_put_string(me, note);
	    if (B_inUnderline == FALSE)
	        HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    HTML_put_character(me, ' ');
	    FREE(note);
	}
	B_inLABEL = TRUE;
	me->in_word = NO;
	B_inP = FALSE;
	break;

    case HTML_ADDRESS:
    	change_paragraph_style(me, styles[element_number]);
	UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_ADDRESS_ID);
	break;

    case HTML_DL:
	List_Nesting_Level++;  /* increment the List nesting level */
	if (List_Nesting_Level <= 0) {
            change_paragraph_style(me, present && present[HTML_DL_COMPACT]
    			              ? styles[HTML_DLC] : styles[HTML_DL]);

	} else if (List_Nesting_Level >= 6) {
            change_paragraph_style(me, present && present[HTML_DL_COMPACT]
    			              ? styles[HTML_DLC6] : styles[HTML_DL6]);

	} else {
            change_paragraph_style(me, present && present[HTML_DL_COMPACT]
    		 ? styles[(HTML_DLC1 - 1) + List_Nesting_Level] 
		 : styles[(HTML_DL1 - 1) + List_Nesting_Level]);
	}
	UPDATE_STYLE;	  /* update to the new style */
	HTML_CheckForID(me, present, value, (int)HTML_DL_ID);
	break;
	
    case HTML_DLC:
        List_Nesting_Level++;  /* increment the List nesting level */
        if (List_Nesting_Level <= 0) {
            change_paragraph_style(me, styles[HTML_DLC]);

        } else if (List_Nesting_Level >= 6) {
            change_paragraph_style(me, styles[HTML_DLC6]);

        } else {
            change_paragraph_style(me, 
                               styles[(HTML_DLC1 - 1) + List_Nesting_Level]);
        }
	UPDATE_STYLE;	  /* update to the new style */
	HTML_CheckForID(me, present, value, (int)HTML_DL_ID);
        break;

    case HTML_DT:
	if (!me->text)
	    UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
        if (!me->style_change) {
	    HText_appendParagraph(me->text);
	    me->in_word = NO;
	    me->sp->style->alignment = HT_LEFT;
	}
	B_inP = FALSE;
	break;
	
    case HTML_DD:
	if (!me->text)
	    UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
	HTML_Last_Char = ' ';  /* absorb white space */
        if (!me->style_change) 
	    HText_appendCharacter(me->text, '\r');
	else 
	  {
            UPDATE_STYLE;
	    HText_appendCharacter(me->text, '\t');
	  }
	me->sp->style->alignment = HT_LEFT;
	me->in_word = NO;
	B_inP = FALSE;
	break;

    case HTML_OL:
	/*
	 * Set the default TYPE.
	 */
	 OL_Type[(List_Nesting_Level < 5 ?
	    		List_Nesting_Level+1 : 6)] = '1';

	/*
	 *  Check whether we have a starting sequence number,
	 *  or want to continue the numbering from a previous
	 *  OL in this nest. - FM
	 */ 
        if (present && (present[HTML_OL_SEQNUM] || present[HTML_OL_START])) {
	    int seqnum;

	    /*
	     *  Give preference to the valid HTML 3.0 SEQNUM attribute name
	     *  over the Netscape START attribute name (too bad the Netscape
	     *  developers didn't read the HTML 3.0 specs before re-inventing
	     *  the "wheel" as "we'll"). - FM
	     */
	    if (present[HTML_OL_SEQNUM] &&
	        value[HTML_OL_SEQNUM] && *value[HTML_OL_SEQNUM]) {
		seqnum = atoi(value[HTML_OL_SEQNUM]);
	    } else if (present[HTML_OL_START] &&
	    	       value[HTML_OL_START] && *value[HTML_OL_START]) {
		seqnum = atoi(value[HTML_OL_START]);
	    } else {
	        seqnum = 1;
	    }

	    /*
	     *  Don't allow negative numbers less than
	     *  or equal to our flags, or numbers less
	     *  than 1 if an Alphabetic or Roman TYPE. - FM
	     */
	    if (present[HTML_OL_TYPE] && value[HTML_OL_TYPE]) {
	        if (*value[HTML_OL_TYPE] == 'A') {
		    OL_Type[(List_Nesting_Level < 5 ?
			     List_Nesting_Level+1 : 6)] = 'A';
		    if (seqnum < 1)
		        seqnum = 1;
		} else if (*value[HTML_OL_TYPE] == 'a') {
		    OL_Type[(List_Nesting_Level < 5 ?
			     List_Nesting_Level+1 : 6)] = 'a';
		    if (seqnum < 1)
		        seqnum = 1;
		} else if (*value[HTML_OL_TYPE] == 'I') {
		    OL_Type[(List_Nesting_Level < 5 ?
			     List_Nesting_Level+1 : 6)] = 'I';
		    if (seqnum < 1)
		        seqnum = 1;
		} else if (*value[HTML_OL_TYPE] == 'i') {
		    OL_Type[(List_Nesting_Level < 5 ?
			     List_Nesting_Level+1 : 6)] = 'i';
		    if (seqnum < 1)
		        seqnum = 1;
		} else {
		  if (seqnum <= OL_VOID)
		      seqnum = OL_VOID + 1;
		}
	    } else if (seqnum <= OL_VOID) {
	        seqnum = OL_VOID + 1;
	    }

	    OL_Counter[(List_Nesting_Level < 5 ?
	    		List_Nesting_Level+1 : 6)] = seqnum;

	} else if (present && present[HTML_OL_CONTINUE]) {
	    OL_Counter[List_Nesting_Level < 5 ?
	    	       List_Nesting_Level+1 : 6] = OL_CONTINUE;

	} else {
	    OL_Counter[(List_Nesting_Level < 5 ?
	    	List_Nesting_Level+1 : 6)] = 1;
	    if (present && present[HTML_OL_TYPE] && value[HTML_OL_TYPE]) {
	        if (*value[HTML_OL_TYPE] == 'A') {
		    OL_Type[(List_Nesting_Level < 5 ?
			     List_Nesting_Level+1 : 6)] = 'A';
		} else if (*value[HTML_OL_TYPE] == 'a') {
		    OL_Type[(List_Nesting_Level < 5 ?
			     List_Nesting_Level+1 : 6)] = 'a';
		} else if (*value[HTML_OL_TYPE] == 'I') {
		    OL_Type[(List_Nesting_Level < 5 ?
			     List_Nesting_Level+1 : 6)] = 'I';
		} else if (*value[HTML_OL_TYPE] == 'i') {
		    OL_Type[(List_Nesting_Level < 5 ?
			     List_Nesting_Level+1 : 6)] = 'i';
		}
	    }
	}
	List_Nesting_Level++;

	if (List_Nesting_Level <= 0) {
       	    change_paragraph_style(me, styles[element_number]);

	} else if (List_Nesting_Level >= 6) {
       	    change_paragraph_style(me, styles[HTML_OL6]);

	} else {
            change_paragraph_style(me, 
		          styles[HTML_OL1 + List_Nesting_Level - 1]);
	}
	UPDATE_STYLE;  /* update to the new style */
	HTML_CheckForID(me, present, value, (int)HTML_OL_ID);
	break;

    case HTML_UL:
	List_Nesting_Level++;

	if (List_Nesting_Level <= 0) {
	    if (!(present && present[HTML_UL_PLAIN]) &&
	        !(present && present[HTML_UL_TYPE] &&
		  value[HTML_UL_TYPE] &&
		  0==strcasecomp(value[HTML_UL_TYPE], "PLAIN"))) {
       	        change_paragraph_style(me, styles[element_number]);
	    } else {
       	        change_paragraph_style(me, styles[HTML_DIR]);
		element_number = HTML_DIR;
	    }

	} else if (List_Nesting_Level >= 6) {
	    if (!(present && present[HTML_UL_PLAIN]) &&
	        !(present && present[HTML_UL_TYPE] &&
		  value[HTML_UL_TYPE] &&
		  0==strcasecomp(value[HTML_UL_TYPE], "PLAIN"))) {
       	        change_paragraph_style(me, styles[HTML_OL6]);
	    } else {
       	        change_paragraph_style(me, styles[HTML_MENU6]);
		element_number = HTML_DIR;
	    }

	} else {
	    if (!(present && present[HTML_UL_PLAIN]) &&
	        !(present && present[HTML_UL_TYPE] &&
		  value[HTML_UL_TYPE] &&
		  0==strcasecomp(value[HTML_UL_TYPE], "PLAIN"))) {
                change_paragraph_style(me, 
		          styles[HTML_OL1 + List_Nesting_Level - 1]);
	    } else {
                change_paragraph_style(me, 
		          styles[HTML_MENU1 + List_Nesting_Level - 1]);
		element_number = HTML_DIR;
	    }
	}
	UPDATE_STYLE;  /* update to the new style */
	HTML_CheckForID(me, present, value, (int)HTML_UL_ID);
	break;

    case HTML_MENU:
    case HTML_DIR:
	List_Nesting_Level++;

	if (List_Nesting_Level <= 0) {
       	    change_paragraph_style(me, styles[element_number]);

	} else if (List_Nesting_Level >= 6) {
       	    change_paragraph_style(me, styles[HTML_MENU6]);

	} else {
            change_paragraph_style(me, 
		          styles[HTML_MENU1 + List_Nesting_Level - 1]);
	}
	UPDATE_STYLE;  /* update to the new style */
	HTML_CheckForID(me, present, value, (int)HTML_UL_ID);
	break;
	
    case HTML_LH:
        UPDATE_STYLE;  /* update to the new style */
	HText_appendParagraph(me->text);
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
	HText_appendCharacter(me->text, ' ');
	me->in_word = NO;
	break;

    case HTML_LI:
        UPDATE_STYLE;  /* update to the new style */
	HText_appendParagraph(me->text);
	me->sp->style->alignment = HT_LEFT;
	HTML_CheckForID(me, present, value, (int)HTML_LI_ID);
	if (me->sp[0].tag_number == HTML_OL) {
	    char number_string[20];
	    register int i, counter, seqnum;
	    char seqtype;

	    counter = List_Nesting_Level < 6 ? List_Nesting_Level : 6;
	    if (present && present[HTML_LI_TYPE] && value[HTML_LI_TYPE]) {
	        if (*value[HTML_LI_TYPE] == '1') {
		    OL_Type[counter] = '1';
	        } else if (*value[HTML_LI_TYPE] == 'A') {
		    OL_Type[counter] = 'A';
	        } else if (*value[HTML_LI_TYPE] == 'a') {
		    OL_Type[counter] = 'a';
	        } else if (*value[HTML_LI_TYPE] == 'I') {
		    OL_Type[counter] = 'I';
	        } else if (*value[HTML_LI_TYPE] == 'i') {
		    OL_Type[counter] = 'i';
		}
	    }
	    if (present && present[HTML_LI_VALUE] &&
	        ((value[HTML_LI_VALUE] != NULL) &&
		 (*value[HTML_LI_VALUE] != '\0')) &&
		((isdigit(*value[HTML_LI_VALUE])) ||
		 (*value[HTML_LI_VALUE] == '-' &&
		  isdigit(*(value[HTML_LI_VALUE] + 1))))) {
		seqnum = atoi(value[HTML_LI_VALUE]);
		if (seqnum <= OL_VOID)
		    seqnum = OL_VOID + 1;
		seqtype = OL_Type[counter];
		if (seqtype != '1' && seqnum < 1)
		    seqnum = 1;
		OL_Counter[counter] = seqnum + 1;
	    } else if (OL_Counter[counter] >= OL_VOID) {
	        seqnum = OL_Counter[counter]++;
		seqtype = OL_Type[counter];
		if (seqtype != '1' && seqnum < 1) {
		    seqnum = 1;
		    OL_Counter[counter] = seqnum + 1;
		}
	    } else {
	        seqnum = Last_OL_Count + 1;
		seqtype = Last_OL_Type;
		for (i = (counter - 1); i >= 0; i--) {
		    if (OL_Counter[i] > OL_VOID) {
		        seqnum = OL_Counter[i]++;
			seqtype = OL_Type[i];
			i = 0;
		    }
		}
	    }
	    if (seqtype == 'A') {
	        sprintf(number_string, LYUppercaseA_OL_String(seqnum));
	    } else if (seqtype == 'a') {
	        sprintf(number_string, LYLowercaseA_OL_String(seqnum));
	    } else if (seqtype == 'I') {
	        sprintf(number_string, LYUppercaseI_OL_String(seqnum));
	    } else if (seqtype == 'i') {
	        sprintf(number_string, LYLowercaseI_OL_String(seqnum));
	    } else {
	        sprintf(number_string, "%2d.", seqnum);
	    }
	    Last_OL_Count = seqnum;
	    Last_OL_Type = seqtype;
	    /* hack, because there is no append string! */
	    for (i = 0; number_string[i] != '\0'; i++)
		if (number_string[i] == ' ')
	            HText_appendCharacter(me->text, number_string[i]);
		else
	    	    HTML_put_character(me, number_string[i]);

	    /* use HTML_put_character so that any other spaces
	     * comming through will be collapsed
	     */
	    HTML_put_character(me, ' ');  /* the spacing charactor */	

	} else if (me->sp[0].tag_number == HTML_UL) {
	    /*
	     *  Hack, because there is no append string!
	     */
	    HText_appendCharacter(me->text, ' ');
	    HText_appendCharacter(me->text, ' ');
	    /* 
	     *  Use HTML_put_character so that any other spaces
	     *  comming through will be collapsed
	     */
	    switch(List_Nesting_Level % 7) {
		case 0:
	    	    HTML_put_character(me, '*');
		    break;
		case 1:
	    	    HTML_put_character(me, '+');
		    break;
		case 2:
	    	    HTML_put_character(me, 'o');
		    break;
		case 3:
	    	    HTML_put_character(me, '#');
		    break;
		case 4:
	    	    HTML_put_character(me, '@');
		    break;
		case 5:
	    	    HTML_put_character(me, '-');
		    break;
		case 6:
	    	    HTML_put_character(me, '=');
		    break;
		    
	    }
	    HTML_put_character(me, ' ');	
	} else {
	    /*
	     *  Hack, because there is no append string!
	     */
	    HText_appendCharacter(me->text, ' ');
	    HText_appendCharacter(me->text, ' ');
	}
	me->in_word = NO;
	B_inP = FALSE;
	break;

    case HTML_SPAN:
	if (!me->text)
	    UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
        /*
	 *  Should check LANG and/or DIR attributes, and the
	 *  me->node_anchor->charset and/or yet to be added
	 *  structure elements, and do something here. - FM
	 */
	break;

    case HTML_BDO:
	if (!me->text)
	    UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
        /*
	 *  Should check DIR (and LANG) attributes, and the
	 *  me->node_anchor->charset and/or yet to be added
	 *  structure elements, and do something here. - FM
	 */
	break;

    case HTML_SPOT:
	if (!me->text)
	    UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
	break;

    case HTML_FN:
    	change_paragraph_style(me, styles[element_number]);
	UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_FN_ID);
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	HTML_put_string(me, "FOOTNOTE:");
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	HTML_put_character(me, ' ');
	B_inLABEL = TRUE;
	me->in_word = NO;
	B_inP = FALSE;
	break;

    case HTML_A:
	{
	    /*
	     *  Set to know we are in an anchor.
	     */
	    B_inA = TRUE;

	    /*
	     *  Load id_string if we have an ID or NAME. - FM
	     */
	    if (present && present[HTML_A_ID] &&
	        value[HTML_A_ID] && *value[HTML_A_ID]) {
		StrAllocCopy(id_string, value[HTML_A_ID]);
	    } else if (present && present[HTML_A_NAME] &&
		       value[HTML_A_NAME] && *value[HTML_A_NAME]) {
		StrAllocCopy(id_string, value[HTML_A_NAME]);
	    }
	    if (id_string) {
	        LYUnEscapeToLatinOne(&id_string, TRUE);
		if (*id_string == '\0') {
		    FREE(id_string);
		}
	    }

	    /*
	     *  Handle the reference. - FM
	     */
	    if (present && present[HTML_A_HREF]) {
	        /*
		 *  Prepare to do housekeeping on the reference. - FM
		 */
		if (!(value[HTML_A_HREF] && *value[HTML_A_HREF])) {
		    if (B_inBASE && base_href && *base_href) {
		        StrAllocCopy(href, base_href);
		    } else {
		        StrAllocCopy(href, me->node_anchor->address);
		    }
		} else {
		    StrAllocCopy(href, value[HTML_A_HREF]);
		    convert_to_spaces(href);
		    LYUnEscapeToLatinOne(&href, TRUE);
		}
	        url_type = is_url(href);

		/*
		 *  Deal with our ftp gateway kludge. - FM
		 */
		if (!url_type && !strncmp(href, "/foo/..", 7) &&
		    (!strncmp(me->node_anchor->address, "ftp:", 4) ||
		     !strncmp(me->node_anchor->address, "file:", 5))) {
		    int i;
		    for (i = 0; href[i]; i++)
		        href[i] = href[i+7];
		}

		/*
		 *  Don't simplify gopher gateway URLs or absolute HREFs. - FM
		 */
		if (HT_Is_Gopher_URL) {
		    HT_Is_Gopher_URL = FALSE;
		} else if (!url_type && *href != '/' && *href != '\0')
		    HTSimplify(href);

	        /*
		 *  Set to know we are making the content bold.
		 */
		B_inBoldA = TRUE;

		/*
		 *  Check whether a base tag is in effect. - FM
		 */
		if ((B_inBASE) &&
		    (temp = HTParse(href, base_href, PARSE_ALL)) &&
		    *temp != '\0')
	            /*
		     *  Use reference related to the base.
		     */
		    StrAllocCopy(href, temp);
		FREE(temp);

		/*
		 *  Check whether to fill in localhost. - FM
		 */
		HTMLFillLocalFileURL((char **)&href,
				     (B_inBASE ? base_href :
						 me->node_anchor->address));
	    } else {
	        if (bold_name_anchors == TRUE) {
	            B_inBoldA = TRUE;
		}
	    }

	    B_CurrentA = HTAnchor_findChildAndLink(
			me->node_anchor,		/* Parent */
			(id_string ? id_string : 0),	/* Tag */
			(href ? href : 0),		/* Address */
			(present &&
			 present[HTML_A_TYPE] &&
			   value[HTML_A_TYPE]) ? 
   (HTLinkType*)HTAtom_for(value[HTML_A_TYPE]) : 0);	/* Type */

	    /*
	     *	Get rid of href since no longer needed.
	     *	Memory leak fixed
	     *	06-16-94 Lynx 2-3-1 Garrett Arch Blythe
	     */
	    FREE(href);
	    FREE(id_string);
	    
	    if (B_CurrentA && present) {
	        if (present[HTML_A_TITLE] &&
		    value[HTML_A_TITLE] && *value[HTML_A_TITLE] != '\0') {
		    StrAllocCopy(title, value[HTML_A_TITLE]);
		    if (current_char_set)
		        LYExpandString(&title);
		    /*
		     *  Convert any HTML entities or decimal escaping. - FM
		     */
		    LYUnEscapeEntities(title, TRUE, FALSE);
		    LYTrimHead(title);
		    LYTrimTail(title);
		    if (*title == '\0') {
		        FREE(title);
		    }
		}
	        if (present[HTML_A_ISMAP])
		    dest_ismap = TRUE;
		if (title != NULL || dest_ismap == TRUE)
	            dest = HTAnchor_parent(
			HTAnchor_followMainLink((HTAnchor*)B_CurrentA)
		    			  );
		if (dest && title != NULL && HTAnchor_title(dest) == NULL)
		    HTAnchor_setTitle(dest, title);
		if (dest && dest_ismap)
		    dest->isISMAPScript = TRUE;
		dest = NULL;
		dest_ismap = FALSE;
		FREE(title);
	    }
	    UPDATE_STYLE;
	    HText_beginAnchor(me->text, B_CurrentA);
	    if (B_inBoldA == TRUE && B_inBoldH == FALSE)
	        HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
	}
    	break;

    case HTML_IMG:			/* Images */
	if (!me->text)
	    UPDATE_STYLE;
	/*
	 *  If it's a clickable image for the current anchor,
	 *  set our flags for faking a 0,0 coordinate pair,
	 *  which typically returns the image's default. - FM
	 */
	if (B_inA && B_CurrentA) {
	    if (dest = HTAnchor_parent(
				HTAnchor_followMainLink((HTAnchor*)B_CurrentA)
				      )) {
		if (dest->isISMAPScript == TRUE) {
		    dest_ismap = TRUE;
		    if (TRACE)
		        fprintf(stderr,
				"HTML: '%s' as an ISMAP script\n",
				dest->address);
		} else if (present && present[HTML_IMG_ISMAP]) {
		    dest_ismap = TRUE;
		    dest->isISMAPScript = TRUE;
		    if (TRACE)
		        fprintf(stderr,
				"HTML: Designating '%s' as an ISMAP script\n",
				dest->address);
		}
	    }
	}

	/*
	 *  If there's a USEMAP, resolve it. - FM
	 */
	if (present && present[HTML_IMG_USEMAP] &&
	    value[HTML_IMG_USEMAP] && *value[HTML_IMG_USEMAP]) {
	    StrAllocCopy(map_href, value[HTML_IMG_USEMAP]);
	    convert_to_spaces(map_href);
	    LYUnEscapeToLatinOne(&map_href, TRUE);
	    if (*map_href == '\0') {
	        FREE(map_href);
	    }
	}
	if (map_href && strchr(map_href, '#')) {
	    url_type = is_url(map_href);
	    if (!url_type && *map_href != '#')
	        HTSimplify(map_href);

	    /*
	     *  Check whether a base tag is in effect. - FM
	     */
	    if ((B_inBASE) &&
	        (temp = HTParse(map_href, base_href, PARSE_ALL)) &&
	        *temp != '\0')
		/*
		 *  Use reference related to the base.
		 */
		StrAllocCopy(map_href, temp);
	    FREE(temp);

	    /*
	     *  Check whether to fill in localhost. - FM
	     */
	    HTMLFillLocalFileURL((char **)&map_href,
				 (B_inBASE ? base_href :
					     me->node_anchor->address));

	    /*
	     *  If it's not yet a URL, resolve versus
	     *  the current document's address. - FM
	     */
	    if (!(url_type = is_url(map_href))) {
	    	temp = HTParse(map_href, me->node_anchor->address, PARSE_ALL);
		StrAllocCopy(map_href, temp);
		FREE(temp);
	    }

	    /*
	     *  Prepend our client-side MAP access field. - FM
	     */
	    StrAllocCopy(temp, "LYNXIMGMAP:");
	    StrAllocCat(temp, map_href);
	    StrAllocCopy(map_href, temp);
	    FREE(temp);
	}

	/*
	 *  Check for a TITLE attribute. - FM
	 */
	if (present && present[HTML_IMG_TITLE] &&
	    value[HTML_IMG_TITLE] && *value[HTML_IMG_TITLE]) {
	    StrAllocCopy(title, value[HTML_IMG_TITLE]);
	    if (current_char_set)
		LYExpandString(&title);
	    /*
	     *  Convert any HTML entities or decimal escaping. - FM
	     */
	    LYUnEscapeEntities(title, TRUE, FALSE);
	    LYTrimHead(title);
	    LYTrimTail(title);
	    if (*title == '\0') {
		FREE(title);
	    }
	}

	/*
	 *  If there's an ALT string, use it, unless the ALT string
	 *  is zero-length or just spaces and we are making all SRCs
	 *  links or have a USEMAP link. - FM
	 */
	if (((present) &&
	     (present[HTML_IMG_ALT] && value[HTML_IMG_ALT])) &&
	    (!clickable_images ||
	     ((clickable_images || map_href) &&
	      *value[HTML_IMG_ALT] != '\0'))) {
	    StrAllocCopy(alt_string, value[HTML_IMG_ALT]);
	    if (current_char_set)
	        LYExpandString(&alt_string);
	    /*
	     *  Convert any HTML entities or decimal escaping. - FM
	     */
	    LYUnEscapeEntities(alt_string,
	    		       LYUsePlainSpace, LYHiddenValue);
	    /*
	     *  If it's all spaces and we are making SRC or
	     *  USEMAP links, treat it as zero-length. - FM
	     */
	    if (clickable_images || map_href) {
	        LYTrimHead(alt_string);
	        LYTrimTail(alt_string);
		if (*alt_string == '\0') {
		    if (map_href)
		        StrAllocCopy(alt_string, (title ?
						  title : "[USEMAP]"));
		    else if (dest_ismap || present[HTML_IMG_ISMAP]) {
		        StrAllocCopy(alt_string, (title ?
						  title : "[ISMAP]"));
		    } else if (B_inA == TRUE) {
		        StrAllocCopy(alt_string, (title ?
						  title : "[LINK]"));
		    } else {
		        StrAllocCopy(alt_string,
					     (title ? title :
				(present[HTML_IMG_ISOBJECT] ?
			     			 "(OBJECT)" : "[INLINE]")));
		    }
		}
	    }

	} else if (map_href) {
	    StrAllocCopy(alt_string, (title ?
	    			      title : "[USEMAP]"));

	} else if (dest_ismap || present && present[HTML_IMG_ISMAP]) {
	    StrAllocCopy(alt_string, (title ?
	    			      title : "[ISMAP]"));

	} else if (B_inA == TRUE) {
	    StrAllocCopy(alt_string, (title ?
	    			      title : "[LINK]"));

	} else {
	    if (pseudo_inline_alts || clickable_images)
	        StrAllocCopy(alt_string, (title ? title :
			  ((present &&
			    present[HTML_IMG_ISOBJECT]) ?
			     		     "(OBJECT)" : "[INLINE]")));
	    else
	        StrAllocCopy(alt_string, (title ?
					  title : ""));
	}
	if (*alt_string == '\0' && map_href) {
	    StrAllocCopy(alt_string, "[USEMAP]");
	}

	if (TRACE) {
	    fprintf(stderr,
	    	    "HTML IMG: USEMAP=%d ISMAP=%d ANCHOR=%d PARA=%d\n",
		    map_href ? 1 : 0,
		    (dest_ismap ||
		     (present && present[HTML_IMG_ISMAP])) ? 1 : 0,
		    B_inA, B_inP);
	}

	/*
	 *  Check for an ID attribute. - FM
	 */
	if (present && present[HTML_IMG_ID] &&
	    value[HTML_IMG_ID] && *value[HTML_IMG_ID]) {
	    StrAllocCopy(id_string, value[HTML_IMG_ID]);
	    LYUnEscapeToLatinOne(&id_string, TRUE);
	    if (*id_string == '\0') {
	        FREE(id_string);
	    }
	}

	/*
	 *  Create links to the SRC for all images, if desired. - FM
	 */
	if (clickable_images &&
	    present && present[HTML_IMG_SRC] &&
	    value[HTML_IMG_SRC] && *value[HTML_IMG_SRC] != '\0') {
	    StrAllocCopy(href, value[HTML_IMG_SRC]);
	    convert_to_spaces(href);
	    LYUnEscapeToLatinOne(&href, TRUE);
	    url_type = is_url(href);
	    if (!url_type && *href != '\0')
	        HTSimplify(href);

	    /*
	     *  Check whether a base tag is in effect. - FM
	     */
	    if ((B_inBASE) &&
		(temp = HTParse(href, base_href, PARSE_ALL)) &&
		*temp != '\0')
		/*
		 *  Use reference related to the base.
		 */
		StrAllocCopy(href, temp);
	    FREE(temp);

	    /*
	     *  Check whether to fill in localhost. - FM
	     */
	    HTMLFillLocalFileURL((char **)&href,
				 (B_inBASE ? base_href :
					     me->node_anchor->address));

	    /*
	     *  If it's an ISMAP and/or USEMAP, or graphic for an
	     *  anchor, end that anchor and start one for the SRC. - FM
	     */
	    if (B_inA) {
	        /*
		 *  If we have a USEMAP, end this anchor and
		 *  start a new one for the client-side MAP. - FM
		 */
		if (map_href) {
		    if (dest_ismap || (present && present[HTML_IMG_ISMAP])) {
		        HTML_put_string(me, "[ISMAP]");
		    } else {
		        HTML_put_string(me, "[LINK]");
		    }
		    if (B_inBoldA == TRUE && B_inBoldH == FALSE) {
		        HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		    }
		    B_inBoldA = FALSE;
		    HText_endAnchor(me->text);
		    HText_appendCharacter(me->text, '-');
		    if (id_string) {
		        if (B_ID_A = HTAnchor_findChildAndLink(
				  me->node_anchor,	/* Parent */
				  id_string,		/* Tag */
				  0,			/* Addresss */
				  0)) {			/* Type */
		            HText_beginAnchor(me->text, B_ID_A);
		            HText_endAnchor(me->text);
		        }
		    }
		    B_CurrentA = HTAnchor_findChildAndLink(
		    		me->node_anchor,	/* Parent */
				0,			/* Tag */
				map_href,		/* Addresss */
				0);			/* Type */
		    if (B_CurrentA && title) {
			if (dest = HTAnchor_parent(
				HTAnchor_followMainLink((HTAnchor*)B_CurrentA)
					          )) {
			    if (!HTAnchor_title(dest))
			        HTAnchor_setTitle(dest, title);
			}
		    }
		    HText_beginAnchor(me->text, B_CurrentA);
		    if (B_inBoldA == FALSE && B_inBoldH == FALSE) {
		        HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		    }
		    B_inBoldA = TRUE;
		}
	        HTML_put_string(me, alt_string);
		if (B_inBoldA == TRUE && B_inBoldH == FALSE) {
		    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		}
		B_inBoldA = FALSE;
		HText_endAnchor(me->text);
		HText_appendCharacter(me->text, '-');
		StrAllocCopy(alt_string,
			     ((present &&
			       present[HTML_IMG_ISOBJECT]) ?
		   ((map_href || dest_ismap) ?
			     	   "(IMAGE)" : "(OBJECT)") : "[IMAGE]"));
		if (id_string && !map_href) {
		    if (B_ID_A = HTAnchor_findChildAndLink(
				  me->node_anchor,	/* Parent */
				  id_string,		/* Tag */
				  0,			/* Addresss */
				  0)) {			/* Type */
		        HText_beginAnchor(me->text, B_ID_A);
		        HText_endAnchor(me->text);
		    }
		}
	    } else if (map_href) {
	        HTML_put_character(me, ' ');  /* space char may be ignored */
		me->in_word = NO;
		if (id_string) {
		    if (B_ID_A = HTAnchor_findChildAndLink(
				  me->node_anchor,	/* Parent */
				  id_string,		/* Tag */
				  0,			/* Addresss */
				  0)) {			/* Type */
		        HText_beginAnchor(me->text, B_ID_A);
		        HText_endAnchor(me->text);
		    }
		}
		B_CurrentA = HTAnchor_findChildAndLink(
		    		me->node_anchor,	/* Parent */
				0,			/* Tag */
				map_href,		/* Addresss */
				0);			/* Type */
		if (B_CurrentA && title) {
		    if (dest = HTAnchor_parent(
				HTAnchor_followMainLink((HTAnchor*)B_CurrentA)
					      )) {
		        if (!HTAnchor_title(dest))
			    HTAnchor_setTitle(dest, title);
		    }
		}
		HText_beginAnchor(me->text, B_CurrentA);
		if (B_inBoldA == FALSE && B_inBoldH == FALSE)
		    HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		B_inBoldA = TRUE;
	        HTML_put_string(me, alt_string);
		if (B_inBoldA == TRUE && B_inBoldH == FALSE) {
		    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		}
		B_inBoldA = FALSE;
		HText_endAnchor(me->text);
		HText_appendCharacter(me->text, '-');
		StrAllocCopy(alt_string,
			     ((present &&
			       present[HTML_IMG_ISOBJECT]) ?
			     			  "(IMAGE)" : "[IMAGE]"));
	    } else {
	        HTML_put_character(me, ' ');  /* space char may be ignored */
		me->in_word = NO;
		if (id_string) {
		    if (B_ID_A = HTAnchor_findChildAndLink(
				  me->node_anchor,	/* Parent */
				  id_string,		/* Tag */
				  0,			/* Addresss */
				  0)) {			/* Type */
		        HText_beginAnchor(me->text, B_ID_A);
		        HText_endAnchor(me->text);
		    }
		}
	    }

	    /*
	     *  Create the link to the SRC. - FM
	     */
	    B_CurrentA = HTAnchor_findChildAndLink(
			me->node_anchor,		/* Parent */
			0,				/* Tag */
			href ? href : 0,		/* Addresss */
			0);				/* Type */
	    FREE(href);
	    HText_beginAnchor(me->text, B_CurrentA);
	    if (B_inBoldH == FALSE)
		HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
	    HTML_put_string(me, alt_string);
	    if (!B_inA) {
	        if (B_inBoldH == FALSE)
		    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		HText_endAnchor(me->text);
		HTML_put_character(me, ' ');  /* space char may be ignored */
		me->in_word = NO;
	    } else {
	        B_inBoldA = TRUE;
	    }
	} else if (map_href) {
	    if (B_inA) {
	        /*
		 *  We're in an anchor and have a USEMAP, so end the anchor
		 *  and start a new one for the client-side MAP. - FM
		 */
		if (dest_ismap || (present && present[HTML_IMG_ISMAP])) {
		    HTML_put_string(me, "[ISMAP]");
		} else {
		    HTML_put_string(me, "[LINK]");
		}
		if (B_inBoldA == TRUE && B_inBoldH == FALSE) {
		    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		}
		B_inBoldA = FALSE;
		HText_endAnchor(me->text);
		HText_appendCharacter(me->text, '-');
	    } else {
	        HTML_put_character(me, ' ');
	        me->in_word = NO;
	    }
	    B_CurrentA = HTAnchor_findChildAndLink(
		    		me->node_anchor,	/* Parent */
				0,			/* Tag */
				map_href,		/* Addresss */
				0);			/* Type */
	    if (B_CurrentA && title) {
		if (dest = HTAnchor_parent(
				HTAnchor_followMainLink((HTAnchor*)B_CurrentA)
				          )) {
		    if (!HTAnchor_title(dest))
		        HTAnchor_setTitle(dest, title);
		}
	    }
	    HText_beginAnchor(me->text, B_CurrentA);
	    if (B_inBoldA == FALSE && B_inBoldH == FALSE) {
		HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
	    }
	    B_inBoldA = TRUE;
	    HTML_put_string(me, alt_string);
	    if (!B_inA) {
	        if (B_inBoldA == TRUE && B_inBoldH == FALSE) {
		    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		}
		B_inBoldA = FALSE;
		HText_endAnchor(me->text);
	    }
	} else {
	    /*
	     *  Just put in the ALT or pseudo-ALT string
	     *  for the current anchor or inline, with an
	     *  ID link if indicated. - FM
	     */
	    if (!B_inA) {
	        HTML_put_character(me, ' ');  /* space char may be ignored */
		me->in_word = NO;
	    }
	    if (id_string) {
		if (B_ID_A = HTAnchor_findChildAndLink(
				  me->node_anchor,	/* Parent */
				  id_string,		/* Tag */
				  0,			/* Addresss */
				  0)) {			/* Type */
		    HText_beginAnchor(me->text, B_ID_A);
		    HText_endAnchor(me->text);
		}
	    }
	    HTML_put_string(me, alt_string);
	    if (!B_inA) {
	        HTML_put_character(me, ' ');  /* space char may be ignored */
		me->in_word = NO;
	    }
	}
	FREE(map_href);
	FREE(alt_string);
	FREE(id_string);
	FREE(title);
	dest = NULL;
	dest_ismap = FALSE;
	break;
    
    case HTML_MAP:
	/*
	 *  Load id_string if we have an ID or NAME. - FM
	 */
	if (present && present[HTML_MAP_ID] &&
	    value[HTML_MAP_ID] && *value[HTML_MAP_ID]) {
	    StrAllocCopy(id_string, value[HTML_MAP_ID]);
	} else if (present && present[HTML_MAP_NAME] &&
		   value[HTML_MAP_NAME] && *value[HTML_MAP_NAME]) {
	    StrAllocCopy(id_string, value[HTML_MAP_NAME]);
	}
	if (id_string) {
	    LYUnEscapeToLatinOne(&id_string, TRUE);
	    if (*id_string == '\0') {
	        FREE(id_string);
	    }
	}

	/*
	 *  Load LYMapName. - FM
	 */
	if (id_string) {
	    char * cp;
	    if (B_inBASE && base_href && *base_href) {
		StrAllocCopy(LYMapName, base_href);
	    } else {
		StrAllocCopy(LYMapName, me->node_anchor->address);
	    }
	    if ((cp=strrchr(LYMapName, '#')) != NULL)
	        *cp = '\0';
	    StrAllocCat(LYMapName, "#");
	    StrAllocCat(LYMapName, id_string);
	    FREE(id_string);
	    if (present && present[HTML_MAP_TITLE] &&
	        value[HTML_MAP_TITLE] && *value[HTML_MAP_TITLE] != '\0') {
	        StrAllocCopy(title, value[HTML_MAP_TITLE]);
		if (current_char_set)
		    LYExpandString(&title);
		/*
		 *  Convert any HTML entities or decimal escaping. - FM
		 */
		LYUnEscapeEntities(title, TRUE, FALSE);
		LYTrimHead(title);
		LYTrimTail(title);
		if (*title == '\0') {
		    FREE(title);
		}
	    }
	    LYAddImageMap(LYMapName, title);
	    FREE(title);
	}
        break;

    case HTML_AREA:
        if (LYMapName &&
	    present && present[HTML_AREA_HREF] &&
	    value[HTML_AREA_HREF] && *value[HTML_AREA_HREF]) {
	    /*
	     * Resolve the HREF. - FM
	     */
	    StrAllocCopy(href, value[HTML_AREA_HREF]);
	    convert_to_spaces(href);
	    LYUnEscapeToLatinOne(&href, TRUE);
	    url_type = is_url(href);
	    if (!url_type && *href != '/' && *href != '\0')
	        HTSimplify(href);

	    /*
	     *  Check whether a base tag is in effect. - FM
	     */
	    if ((B_inBASE) &&
		(temp = HTParse(href, base_href, PARSE_ALL)) &&
		*temp != '\0')
		/*
		 *  Use reference related to the base.
		 */
		StrAllocCopy(href, temp);
	    FREE(temp);

	    /*
	     *  Check whether to fill in localhost. - FM
	     */
	    HTMLFillLocalFileURL((char **)&href,
				 (B_inBASE ? base_href :
					     me->node_anchor->address));

	    if (!(url_type = is_url(href))) {
	        temp = HTParse(href, me->node_anchor->address, PARSE_ALL);
		if (!(temp && *temp)) {
		   FREE(href);
		   FREE(temp);
		   break;
		}
		StrAllocCopy(href, temp);
		FREE(temp);
	    }

	    /*
	     *  Check for an ALT. - FM
	     */
	    if (present[HTML_AREA_ALT] &&
	        value[HTML_AREA_ALT] && *value[HTML_AREA_ALT]) {
	        StrAllocCopy(alt_string, value[HTML_AREA_ALT]);
	    } else if (present[HTML_AREA_TITLE] &&
	        value[HTML_AREA_TITLE] && *value[HTML_AREA_TITLE]) {
		/*
		 *  Use the TITLE as an ALT. - FM
		 */
	        StrAllocCopy(alt_string, value[HTML_AREA_TITLE]);
	    }
	    if (alt_string != NULL) {
		if (current_char_set)
	            LYExpandString(&alt_string);
		/*
		 *  Convert any HTML entities or decimal escaping. - FM
		 */
		LYUnEscapeEntities(alt_string,
				   LYUsePlainSpace, LYHiddenValue);
		/*
		 *  Make sure it's not just space(s). - FM
		 */
	        LYTrimHead(alt_string);
		LYTrimTail(alt_string);
		if (*alt_string == '\0') {
		    StrAllocCopy(alt_string, href);
		}
	    } else {
	        /*
		 *  Use the HREF as an ALT. - FM
		 */
	        StrAllocCopy(alt_string, href);
	    }

	    LYAddMapElement(LYMapName, href, alt_string);
	    FREE(href);
	    FREE(alt_string);
	}
        break;

    case HTML_PARAM:
        /*
	 *  We may need to look at this someday to deal with
	 *  MAPs, OBJECTs or APPLETs optimally, but just ignore
	 *  it for now. - FM
	 */
	break;

    case HTML_BODYTEXT:
        HTML_CheckForID(me, present, value, (int)HTML_BODYTEXT_ID);
        /*
	 *  We may need to look at this someday to deal with
	 *  OBJECTs optimally, but just ignore it for now. - FM
	 */
	break;

    case HTML_TEXTFLOW:
        HTML_CheckForID(me, present, value, (int)HTML_BODYTEXT_ID);
        /*
	 *  We may need to look at this someday to deal with
	 *  APPLETs optimally, but just ignore it for now. - FM
	 */
	break;

    case HTML_FIG:
        B_inFIG = TRUE;
	if (!me->text)
	    UPDATE_STYLE;
	if (!present ||
	    (present && !present[HTML_FIG_ISOBJECT])) {
	    HTML_EnsureDoubleSpace(me);
	    HTML_ResetParagraphAlignment(me);
	    B_inFIGwithP = TRUE;
	} else {
	    B_inFIGwithP = FALSE;
	    HTML_put_character(me, ' ');  /* space char may be ignored */
	}
	HTML_CheckForID(me, present, value, (int)HTML_FIG_ID);
	me->in_word = NO;
	B_inP = FALSE;

	if (clickable_images && present && present[HTML_FIG_SRC] &&
	    value[HTML_FIG_SRC] && *value[HTML_FIG_SRC] != '\0') {
	    StrAllocCopy(href, value[HTML_FIG_SRC]);
	    convert_to_spaces(href);
	    LYUnEscapeToLatinOne(&href, TRUE);
	    if (*href) {
	        url_type = is_url(href);
		if (!url_type && *href != '/')
		    HTSimplify(href);

		/*
		 *  Check whether a base tag is in effect. - FM
		 */
		if ((B_inBASE) &&
		    (temp = HTParse(href, base_href, PARSE_ALL)) &&
		    *temp != '\0')
		    /*
		     *  Use reference related to the base.
		     */
		    StrAllocCopy(href, temp);
		FREE(temp);

		/*
		 *  Check whether to fill in localhost. - FM
		 */
		HTMLFillLocalFileURL((char **)&href,
				     (B_inBASE ?
				     base_href : me->node_anchor->address));

		if ((B_CurrentA = HTAnchor_findChildAndLink(
		       			me->node_anchor,	/* Parent */
		       			0,			/* Tag */
		       			href,			/* Addresss */
		       			0))) {			/* Type */
		    HText_beginAnchor(me->text, B_CurrentA);
		    if (B_inBoldH == FALSE)
			HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		    HTML_put_string(me,
		    		    (present[HTML_FIG_ISOBJECT] ?
		      (present[HTML_FIG_IMAGEMAP] ?
					"(IMAGE)" : "(OBJECT)") : "[FIGURE]"));
		    if (B_inBoldH == FALSE)
			HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		    HText_endAnchor(me->text);
		    HText_appendCharacter(me->text, '-');
		    HTML_put_character(me, ' ');  /* space char may be ignored */
		    me->in_word = NO;
		}
	    }
	    FREE(href);
	}
	break;

    case HTML_OBJECT:
	if (!me->text)
	    UPDATE_STYLE;
	if (!me->object_started) {
	    /*
	     *  This is an outer OBJECT start tag,
	     *  i.e., not a nested OBJECT, so save
	     *  it's relevant attributes. - FM
	     */
	    if (present) {
	        if (present[HTML_OBJECT_DECLARE])
		    me->object_declare = TRUE;
		if (present[HTML_OBJECT_SHAPES])
		    me->object_shapes = TRUE;
		if (present[HTML_OBJECT_ISMAP])
		    me->object_ismap = TRUE;
		if (present[HTML_OBJECT_USEMAP] &&
		    value[HTML_OBJECT_USEMAP] && *value[HTML_OBJECT_USEMAP]) {
		    StrAllocCopy(me->object_usemap, value[HTML_OBJECT_USEMAP]);
		    LYUnEscapeToLatinOne(&me->object_usemap, TRUE);
		    if (*me->object_usemap == '\0') {
		        FREE(me->object_usemap);
		    }
		}
		if (present[HTML_OBJECT_ID] &&
		    value[HTML_OBJECT_ID] && *value[HTML_OBJECT_ID]) {
		    StrAllocCopy(me->object_id, value[HTML_OBJECT_ID]);
		    LYUnEscapeToLatinOne(&me->object_id, TRUE);
		    if (*me->object_id == '\0') {
		        FREE(me->object_id);
		    }
		}
		if (present[HTML_OBJECT_TITLE] &&
		    value[HTML_OBJECT_TITLE] && *value[HTML_OBJECT_TITLE]) {
		    StrAllocCopy(me->object_title, value[HTML_OBJECT_TITLE]);
		    if (current_char_set)
		        LYExpandString(&me->object_title);
		    LYUnEscapeEntities(me->object_title, TRUE, FALSE);
		    LYTrimHead(me->object_title);
		    LYTrimTail(me->object_title);
		    if (me->object_title == '\0') {
		        FREE(me->object_title);
		    }
		}
		if (present[HTML_OBJECT_DATA] &&
		    value[HTML_OBJECT_DATA] && *value[HTML_OBJECT_DATA]) {
		    StrAllocCopy(me->object_data, value[HTML_OBJECT_DATA]);
		    LYUnEscapeToLatinOne(&me->object_data, TRUE);
		    if (*me->object_data == '\0') {
		        FREE(me->object_data);
		    }
		}
		if (present[HTML_OBJECT_TYPE] &&
		    value[HTML_OBJECT_TYPE] && *value[HTML_OBJECT_TYPE]) {
		    StrAllocCopy(me->object_type, value[HTML_OBJECT_TYPE]);
		    if (current_char_set)
		        LYExpandString(&me->object_type);
		    LYUnEscapeEntities(me->object_type, TRUE, FALSE);
		    LYTrimHead(me->object_type);
		    LYTrimTail(me->object_type);
		    if (me->object_type == '\0') {
		        FREE(me->object_type);
		    }
		}
		if (present[HTML_OBJECT_CLASSID] &&
		    value[HTML_OBJECT_CLASSID] &&
		    *value[HTML_OBJECT_CLASSID]) {
		    StrAllocCopy(me->object_classid,
		    		 value[HTML_OBJECT_CLASSID]);
		    if (current_char_set)
		        LYExpandString(&me->object_classid);
		    LYUnEscapeEntities(me->object_classid, TRUE, FALSE);
		    LYTrimHead(me->object_classid);
		    LYTrimTail(me->object_classid);
		    if (me->object_classid == '\0') {
		        FREE(me->object_classid);
		    }
		}
		if (present[HTML_OBJECT_CODEBASE] &&
		    value[HTML_OBJECT_CODEBASE] &&
		    *value[HTML_OBJECT_CODEBASE]) {
		    StrAllocCopy(me->object_codebase,
		    		 value[HTML_OBJECT_CODEBASE]);
		    LYUnEscapeToLatinOne(&me->object_codebase, TRUE);
		    if (*me->object_codebase == '\0') {
		        FREE(me->object_codebase);
		    }
		}
		if (present[HTML_OBJECT_CODETYPE] &&
		    value[HTML_OBJECT_CODETYPE] &&
		    *value[HTML_OBJECT_CODETYPE]) {
		    StrAllocCopy(me->object_codetype,
		    		 value[HTML_OBJECT_CODETYPE]);
		    if (current_char_set)
		        LYExpandString(&me->object_codetype);
		    LYUnEscapeEntities(me->object_codetype, TRUE, FALSE);
		    LYTrimHead(me->object_codetype);
		    LYTrimTail(me->object_codetype);
		    if (me->object_codetype == '\0') {
		        FREE(me->object_codetype);
		    }
		}
		if (present[HTML_OBJECT_NAME] &&
		    value[HTML_OBJECT_NAME] && *value[HTML_OBJECT_NAME]) {
		    StrAllocCopy(me->object_name, value[HTML_OBJECT_NAME]);
		    if (current_char_set)
		        LYExpandString(&me->object_name);
		    LYUnEscapeEntities(me->object_name, TRUE, FALSE);
		    LYTrimHead(me->object_name);
		    LYTrimTail(me->object_name);
		    if (me->object_name == '\0') {
		        FREE(me->object_name);
		    }
		}
	    }
	    /*
	     *  Set flag that we are accumulating OBJECT content. - FM
	     */
	    me->object_started = TRUE;
	}
	break;

    case HTML_OVERLAY:
	if (clickable_images && B_inFIG &&
	    present && present[HTML_OVERLAY_SRC] &&
	    value[HTML_OVERLAY_SRC] && *value[HTML_OVERLAY_SRC] != '\0') {
	    StrAllocCopy(href, value[HTML_OVERLAY_SRC]);
	    convert_to_spaces(href);
	    LYUnEscapeToLatinOne(&href, TRUE);
	    if (*href) {
		url_type = is_url(href);
		if (!url_type && *href != '/')
		    HTSimplify(href);

		/*
		 *  Check whether a base tag is in effect. - FM
		 */
		if ((B_inBASE) &&
		    (temp = HTParse(href, base_href, PARSE_ALL)) &&
		    *temp != '\0')
		    /*
		     *  Use reference related to the base.
		     */
		    StrAllocCopy(href, temp);
		FREE(temp);

		/*
		 *  Check whether to fill in localhost. - FM
		 */
		HTMLFillLocalFileURL((char **)&href,
				     (B_inBASE ?
				     base_href : me->node_anchor->address));

		if ((B_CurrentA = HTAnchor_findChildAndLink(
					me->node_anchor,	/* Parent */
					0,			/* Tag */
					href,			/* Addresss */
					0))) {			/* Type */
		    if (!me->text) {
		        UPDATE_STYLE;
		    } else {
		        HTML_put_character(me, ' ');
			HText_appendCharacter(me->text, '+');
		    }
		    HText_beginAnchor(me->text, B_CurrentA);
		    if (B_inBoldH == FALSE)
			HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		    HTML_put_string(me, "[OVERLAY]");
		    if (B_inBoldH == FALSE)
			HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		    HText_endAnchor(me->text);
		    HTML_put_character(me, ' ');
		    me->in_word = NO;
		}
	    }
	    FREE(href);
	}
	break;

    case HTML_APPLET:
        B_inAPPLET = TRUE;
	B_inAPPLETwithP = FALSE;
	if (!me->text)
	    UPDATE_STYLE;
	HTML_put_character(me, ' ');  /* space char may be ignored */
	/*
	 *  Load id_string if we have an ID or NAME. - FM
	 */
	if (present && present[HTML_APPLET_ID] &&
	    value[HTML_APPLET_ID] && *value[HTML_APPLET_ID]) {
	    StrAllocCopy(id_string, value[HTML_APPLET_ID]);
	} else if (present && present[HTML_APPLET_NAME] &&
		   value[HTML_APPLET_NAME] && *value[HTML_APPLET_NAME]) {
	    StrAllocCopy(id_string, value[HTML_APPLET_NAME]);
	}
	if (id_string) {
	    LYUnEscapeToLatinOne(&id_string, TRUE);
	    HTML_HandleID(me, id_string);
	    FREE(id_string);
	}
	me->in_word = NO;

	/*
	 *  If there's an ALT string, use it, unless the ALT string
	 *  is zero-length and we are making all sources links. - FM
	 */
	if (present && present[HTML_APPLET_ALT] && value[HTML_APPLET_ALT] &&
	    (!clickable_images ||
	     (clickable_images && *value[HTML_APPLET_ALT] != '\0'))) {
	    StrAllocCopy(alt_string, value[HTML_APPLET_ALT]);
	    if (current_char_set)
	        LYExpandString(&alt_string);
	    /*
	     *  Convert any HTML entities or decimal escaping. - FM
	     */
	    LYUnEscapeEntities(alt_string,
	    		       LYUsePlainSpace, LYHiddenValue);
	    /*
	     *  If it's all spaces and we are making sources links,
	     *  treat it as zero-length. - FM
	     */
	    if (clickable_images) {
	        LYTrimHead(alt_string);
	        LYTrimTail(alt_string);
		if (*alt_string == '\0') {
		    StrAllocCopy(alt_string, "[APPLET]");
		}
	    }

	} else {
	    if (clickable_images)
	        StrAllocCopy(alt_string, "[APPLET]");
	    else
	        StrAllocCopy(alt_string, "");
	}

	/*
	 *  If we're making all sources links, get the source. - FM
	 */
	if (clickable_images && present && present[HTML_APPLET_CODE] &&
	    value[HTML_APPLET_CODE] && *value[HTML_APPLET_CODE] != '\0') {
	    char * base = NULL;
	    char * code = NULL;

	    /*
	     *  Check for a CODEBASE attribute. - FM
	     */
	    if (present[HTML_APPLET_CODEBASE] &&
	        value[HTML_APPLET_CODEBASE] && *value[HTML_APPLET_CODEBASE]) {
	        StrAllocCopy(base, value[HTML_APPLET_CODEBASE]);
		convert_to_spaces(base);
		LYUnEscapeToLatinOne(&base, TRUE);
		/*
		 *  Force it to be a directory. - FM
		 */
		if (*base == '\0')
		    StrAllocCopy(base, "/");
		if (base[strlen(base)-1] != '/')
		    StrAllocCat(base, "/");
		url_type = is_url(base);

		/*
		 *  Don't simplify absolute CODEBASEs. - FM
		 */
		if (!url_type && *base != '/')
		    HTSimplify(base);

		/*
		 *  Check whether to fill in localhost. - FM
		 */
		HTMLFillLocalFileURL((char **)&base,
				     (B_inBASE ? base_href :
						 me->node_anchor->address));

		if (!(url_type = is_url(base))) {
		    /*
		     *  Check whether a base tag is in effect.
		     */
		    if (B_inBASE) {
		        temp = HTParse(base, base_href, PARSE_ALL);
		    } else {
		        temp = HTParse(base, me->node_anchor->address,
							PARSE_ALL);
		    }
		    StrAllocCopy(base, temp);
		    FREE(temp);
		}
	    } else {
		if (B_inBASE) {
		    StrAllocCopy(base, base_href);
		} else {
		    StrAllocCopy(base, me->node_anchor->address);
		}
	    }

	    StrAllocCopy(code, value[HTML_APPLET_CODE]);
	    convert_to_spaces(code);
	    LYUnEscapeToLatinOne(&code, TRUE);
	    HTSimplify(code);
	    href = HTParse(code, base, PARSE_ALL);
	    FREE(base);
	    FREE(code);

	    if ((href && *href) &&
	        (B_CurrentA = HTAnchor_findChildAndLink(
					me->node_anchor,	/* Parent */
					0,			/* Tag */
					href,			/* Addresss */
					0))) {			/* Type */
		HText_beginAnchor(me->text, B_CurrentA);
		if (B_inBoldH == FALSE)
		    HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		HTML_put_string(me, alt_string);
		if (B_inBoldH == FALSE)
		    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		HText_endAnchor(me->text);
		HTML_put_character(me, ' ');  /* space char may be ignored */
		me->in_word = NO;
	    }
	    FREE(href);
	} else if (*alt_string) {
	    /*
	     *  Just put up the ALT string, if non-zero. - FM
	     */
	    HTML_put_string(me, alt_string);
	    HTML_put_character(me, ' ');  /* space char may be ignored */
	    me->in_word = NO;
	}
	FREE(alt_string);
	FREE(id_string);
	break;

    case HTML_BGSOUND:
	/*
	 *  If we're making all sources links, get the source. - FM
	 */
	if (clickable_images && present && present[HTML_BGSOUND_SRC] &&
	    value[HTML_BGSOUND_SRC] && *value[HTML_BGSOUND_SRC] != '\0') {
	    StrAllocCopy(href, value[HTML_BGSOUND_SRC]);
	    convert_to_spaces(href);
	    LYUnEscapeToLatinOne(&href, TRUE);
	    if (*href == '\0') {
	        FREE(href);
		break;
	    }
	    url_type = is_url(href);
	    if (!url_type && *href != '/')
	        HTSimplify(href);

	    /*
	     *  Check whether a base tag is in effect. - FM
	     */
	    if ((B_inBASE) &&
		(temp = HTParse(href, base_href, PARSE_ALL)) &&
		*temp != '\0')
		/*
		 *  Use reference related to the base.
		 */
		StrAllocCopy(href, temp);
	    FREE(temp);

	    /*
	     *  Check whether to fill in localhost. - FM
	     */
	    HTMLFillLocalFileURL((char **)&href,
				 (B_inBASE ? base_href :
					     me->node_anchor->address));

	    if (!me->text)
	        UPDATE_STYLE;
	    if ((B_CurrentA = HTAnchor_findChildAndLink(
					me->node_anchor,	/* Parent */
					0,			/* Tag */
					href,			/* Addresss */
					0))) {			/* Type */
		HTML_put_character(me, ' ');  /* space char may be ignored */
		me->in_word = NO;
		HText_beginAnchor(me->text, B_CurrentA);
		if (B_inBoldH == FALSE)
		    HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		HTML_put_string(me, "[BGSOUND]");
		if (B_inBoldH == FALSE)
		    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		HText_endAnchor(me->text);
		HTML_put_character(me, ' ');  /* space char may be ignored */
		me->in_word = NO;
	    }
	    FREE(href);
	}
	break;

    case HTML_EMBED:
	if (!me->text)
	    UPDATE_STYLE;
	if (pseudo_inline_alts || clickable_images)
	    HTML_put_character(me, ' ');  /* space char may be ignored */
	/*
	 *  Load id_string if we have an ID or NAME. - FM
	 */
	if (present && present[HTML_EMBED_ID] &&
	    value[HTML_EMBED_ID] && *value[HTML_EMBED_ID]) {
	    StrAllocCopy(id_string, value[HTML_EMBED_ID]);
	} else if (present && present[HTML_EMBED_NAME] &&
		   value[HTML_EMBED_NAME] && *value[HTML_EMBED_NAME]) {
	    StrAllocCopy(id_string, value[HTML_EMBED_NAME]);
	}
	if (id_string) {
	    LYUnEscapeToLatinOne(&id_string, TRUE);
	    HTML_HandleID(me, id_string);
	    FREE(id_string);
	}
	if (pseudo_inline_alts || clickable_images)
	    me->in_word = NO;

	/*
	 *  If there's an ALT string, use it, unless the ALT string
	 *  is zero-length and we are making all sources links. - FM
	 */
	if (present && present[HTML_EMBED_ALT] && value[HTML_EMBED_ALT] &&
	    (!clickable_images ||
	     (clickable_images && *value[HTML_EMBED_ALT] != '\0'))) {
	    StrAllocCopy(alt_string, value[HTML_EMBED_ALT]);
	    if (current_char_set)
	        LYExpandString(&alt_string);
	    /*
	     *  Convert any HTML entities or decimal escaping. - FM
	     */
	    LYUnEscapeEntities(alt_string,
	    		       LYUsePlainSpace, LYHiddenValue);
	    /*
	     *  If it's all spaces and we are making sources links,
	     *  treat it as zero-length. - FM
	     */
	    if (clickable_images) {
	        LYTrimHead(alt_string);
	        LYTrimTail(alt_string);
		if (*alt_string == '\0') {
		    StrAllocCopy(alt_string, "[EMBED]");
		}
	    }
	} else {
	    if (pseudo_inline_alts || clickable_images)
	        StrAllocCopy(alt_string, "[EMBED]");
	    else
	        StrAllocCopy(alt_string, "");
	}

	/*
	 *  If we're making all sources links, get the source. - FM
	 */
	if (clickable_images && present && present[HTML_EMBED_SRC] &&
	    value[HTML_EMBED_SRC] && *value[HTML_EMBED_SRC] != '\0') {
	    StrAllocCopy(href, value[HTML_EMBED_SRC]);
	    convert_to_spaces(href);
	    LYUnEscapeToLatinOne(&href, TRUE);
	    if (*href != '\0') {
		url_type = is_url(href);
		if (!url_type && *href != '/')
		     HTSimplify(href);

		/*
		 *  Check whether a base tag is in effect. - FM
		 */
		if ((B_inBASE) &&
		    (temp = HTParse(href, base_href, PARSE_ALL)) &&
		    *temp != '\0')
		    /*
		     *  Use reference related to the base.
		     */
		    StrAllocCopy(href, temp);
		FREE(temp);

		/*
		 *  Check whether to fill in localhost. - FM
		 */
		HTMLFillLocalFileURL((char **)&href,
				     (B_inBASE ?
				     base_href : me->node_anchor->address));

		if ((B_CurrentA = HTAnchor_findChildAndLink(
					me->node_anchor,	/* Parent */
					0,			/* Tag */
					href,			/* Addresss */
					0))) {			/* Type */
		    HText_beginAnchor(me->text, B_CurrentA);
		    if (B_inBoldH == FALSE)
			HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		    HTML_put_string(me, alt_string);
		    if (B_inBoldH == FALSE)
			HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		    HText_endAnchor(me->text);
		    HTML_put_character(me, ' ');
		    me->in_word = NO;
		}
	    }
	    FREE(href);
	} else if (*alt_string) {
	    /*
	     *  Just put up the ALT string, if non-zero. - FM
	     */
	    HTML_put_string(me, alt_string);
	    HTML_put_character(me, ' ');  /* space char may be ignored */
	    me->in_word = NO;
	}
	FREE(alt_string);
	FREE(id_string);
	break;

    case HTML_CREDIT:
	if (!me->text)
	    UPDATE_STYLE;
	HTML_EnsureDoubleSpace(me);
	HTML_ResetParagraphAlignment(me);
	B_inCREDIT = TRUE;
	HTML_CheckForID(me, present, value, (int)HTML_CREDIT_ID);
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	HTML_put_string(me, "CREDIT:");
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	HTML_put_character(me, ' ');

	if (B_inFIG)
	    /*
	     *  Assume all text in the FIG container is intended
	     *  to be paragraphed. - FM
	     */
	    B_inFIGwithP = TRUE;

	if (B_inAPPLET)
	    /*
	     *  Assume all text in the APPLET container is intended
	     *  to be paragraphed. - FM
	     */
	    B_inAPPLETwithP = TRUE;

	B_inLABEL = TRUE;
	me->in_word = NO;
	B_inP = FALSE;
	break;

    case HTML_CAPTION:
	if (!me->text)
	    UPDATE_STYLE;
	HTML_EnsureDoubleSpace(me);
	HTML_ResetParagraphAlignment(me);
	B_inCAPTION = TRUE;
	HTML_CheckForID(me, present, value, (int)HTML_CAPTION_ID);
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	HTML_put_string(me, "CAPTION:");
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	HTML_put_character(me, ' ');

	if (B_inFIG)
	    /*
	     *  Assume all text in the FIG container is intended
	     *  to be paragraphed. - FM
	     */
	    B_inFIGwithP = TRUE;

	if (B_inAPPLET)
	    /*
	     *  Assume all text in the APPLET container is intended
	     *  to be paragraphed. - FM
	     */
	    B_inAPPLETwithP = TRUE;

	B_inLABEL = TRUE;
	me->in_word = NO;
	B_inP = FALSE;
	break;

    case HTML_FORM:
	{
	    char * action = NULL;
	    char * method = NULL;
	    char * enctype = NULL;
	    char * cp;
	    HTChildAnchor * source;
	    HTAnchor *link_dest;

	    /*
	     *  Set to know we are in a form.
	     */
	    B_inFORM = TRUE;
	    UPDATE_STYLE;

	    if (present && present[HTML_FORM_ACTION] &&
	        value[HTML_FORM_ACTION])  {
	        /*
		 *  Prepare to do housekeeping on the reference. - FM
		 */
		StrAllocCopy(action, value[HTML_FORM_ACTION]);
		convert_to_spaces(action);
		LYUnEscapeToLatinOne(&action, TRUE);
	        url_type = is_url(action);

		/*
		 *  Don't simplify absolute ACTIONs. - FM
		 */
		if (!url_type && *action != '/' && *action != '\0')
		    HTSimplify(action);
		/*
		 *  Check whether a base tag is in effect.
		 */
		if ((B_inBASE) &&
		    (temp = HTParse(action, base_href, PARSE_ALL)) &&
		    *temp != '\0')
	            /*
		     *  Use action related to the base.
		     */
		    StrAllocCopy(action, temp);
		FREE(temp);
	    }
	    if (!(action && *action)) {
	    	if (B_inBASE && base_href && *base_href) {
		     StrAllocCopy(action, base_href);
		} else {
		     StrAllocCopy(action, me->node_anchor->address);
		}
	    }
	    if (action) {
	        source = HTAnchor_findChildAndLink(me->node_anchor, 
						   0,
						   action,
						   0);
		if (link_dest = HTAnchor_followMainLink((HTAnchor *)source)) {
		    /*
		     *  Memory leak fixed.
		     *  05-28-94 Lynx 2-3-1 Garrett Arch Blythe
		     */
		    auto char *cp_freeme = HTAnchor_address(link_dest);
                    if (cp_freeme != NULL) {
                    	StrAllocCopy(action, cp_freeme);
			FREE(cp_freeme);
		    } else {
                    	StrAllocCopy(action, "");
		    }
		}
	    }

	    if (present && present[HTML_FORM_METHOD])
	    	StrAllocCopy(method, value[HTML_FORM_METHOD] ?
				     value[HTML_FORM_METHOD] : "GET");

	    if (present && present[HTML_FORM_ENCTYPE] &&
	        value[HTML_FORM_ENCTYPE] && *value[HTML_FORM_ENCTYPE]) {
	    	StrAllocCopy(enctype, value[HTML_FORM_ENCTYPE]);
		/*
		 *  Force the enctype value to all lower case. - FM
		 */
		for (cp = enctype; *cp; cp++)
		    *cp = TOLOWER(*cp);
	    }

	    if (present) {
		/*
		 *  Check for a TITLE attribute, and if none is present,
		 *  check for a SUBJECT attribute as a synonym. - FM
		 */
	        if (present[HTML_FORM_TITLE] &&
	            value[HTML_FORM_TITLE] &&
		    *value[HTML_FORM_TITLE] != '\0') {
		    StrAllocCopy(title, value[HTML_FORM_TITLE]);
	        } else if (present[HTML_FORM_SUBJECT] &&
			   value[HTML_FORM_SUBJECT] &&
			   *value[HTML_FORM_SUBJECT] != '\0') {
		    StrAllocCopy(title, value[HTML_FORM_SUBJECT]);
		}
		if (title != NULL && *title != '\0') {
		    if (current_char_set)
		        LYExpandString(&title);
		    /*
		     *  Convert any HTML entities or decimal escaping. - FM
		     */
		    LYUnEscapeEntities(title, TRUE, FALSE);
		    LYTrimHead(title);
		    LYTrimTail(title);
		    if (*title == '\0') {
		        FREE(title);
		    }
		}
	    }

	    HText_beginForm(action, method, enctype, title);

	    FREE(action);
	    FREE(method);
	    FREE(enctype);
	    FREE(title);
	}
	if (!me->text)
	    UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_FORM_ID);
	break;

    case HTML_FIELDSET:
        HTML_CheckForID(me, present, value, (int)HTML_FIELDSET_ID);
        break;

    case HTML_LABEL:
        HTML_CheckForID(me, present, value, (int)HTML_LABEL_ID);
        break;

    case HTML_INPUT:
	{
	    InputFieldData I;
	    int chars;

	    /*
	     *  Make sure we're in a form.
	     */
	    if (!B_inFORM) {
	        if (TRACE) {
		    fprintf(stderr, "HTML: INPUT tag not within FORM tag\n");
		} else if (!B_inBadHTML) {
	            _statusline(BAD_HTML_USE_TRACE);
		    B_inBadHTML = TRUE;
		    sleep(MessageSecs);
		}
		/*
		 *  We'll process it, since the chances of a crash are
		 *  small, and we probably do have a form started. - FM
		 *
		break;
		 */
	    }

	    /* Check for unclosed TEXTAREA */
	    if (B_inTEXTAREA) {
	        if (TRACE) {
		    fprintf(stderr, "HTML: Missing TEXTAREA end tag\n");
		} else if (!B_inBadHTML) {
		    _statusline(BAD_HTML_USE_TRACE);
		    B_inBadHTML = TRUE;
		    sleep(MessageSecs);
		}
	    }

	    /* init */
	    I.align=NULL; I.accept=NULL; I.checked=NO; I.class=NULL;
	    I.disabled=NO; I.error=NULL; I.height= NULL; I.id=NULL;
	    I.lang=NULL; I.max=NULL; I.maxlength=NULL; I.md=NULL;
	    I.min=NULL; I.name=NULL; I.size=NULL; I.src=NULL;
	    I.type=NULL; I.value=NULL; I.width=NULL;

	    UPDATE_STYLE;

	    /*
	     *  Before any input field add a space if necessary
	     */
	    HTML_put_character(me, ' ');
	    me->in_word = NO;

	    if (present && present[HTML_INPUT_NAME] && value[HTML_INPUT_NAME])
		I.name = (char *)value[HTML_INPUT_NAME];
	    else
	        I.name = "";
	    if (present && present[HTML_INPUT_TYPE] &&
	        value[HTML_INPUT_TYPE] && *value[HTML_INPUT_TYPE]) {
		I.type = (char *)value[HTML_INPUT_TYPE];

		if (!strcasecomp(I.type, "range")) {
		    if (present[HTML_INPUT_MIN])
		        I.min = (char *)value[HTML_INPUT_MIN];
		    if (present[HTML_INPUT_MAX])
		        I.max = (char *)value[HTML_INPUT_MAX];
		    /*
		     *  Not yet implemented.
		     */
		    HTML_put_string(me,"[RANGE Input] (Not yet implemented.)");
		    if (TRACE)
		        fprintf(stderr, "HTML: Ignoring TYPE=\"range\"\n");
		    break;

		} else if (!strcasecomp(I.type, "file")) {
		    if (present[HTML_INPUT_ACCEPT])
		        I.accept = (char *)value[HTML_INPUT_ACCEPT];
		    /*
		     *  Not yet implemented.
		     */
		    if (B_inUnderline == FALSE) {
		        HText_appendCharacter(me->text,
					      LY_UNDERLINE_START_CHAR);
		    }
		    HTML_put_string(me,"[FILE Input] (Not yet implemented.)");
		    if (B_inUnderline == FALSE) {
		        HText_appendCharacter(me->text,
					      LY_UNDERLINE_END_CHAR);
		    }
		    if (TRACE)
		        fprintf(stderr, "HTML: Ignoring TYPE=\"file\"\n");
		    break;
		}
	    }
	    if (present && present[HTML_INPUT_VALUE] &&
	        value[HTML_INPUT_VALUE] && *value[HTML_INPUT_VALUE]) {
	        /*
		 *  Convert any HTML entities or decimal escaping. - FM
		 */
		int CurrentCharSet = current_char_set;
		BOOL CurrentEightBitRaw = HTPassEightBitRaw;
		BOOLEAN CurrentUseDefaultRawMode = LYUseDefaultRawMode;
		HTCJKlang CurrentHTCJK = HTCJK;
		int len;
		
		if (I.type && !strcasecomp(I.type, "hidden")) {
		    LYHiddenValue = TRUE;
		    current_char_set = 0;	/* Default ISO-Latin1 */
		    LYUseDefaultRawMode = TRUE;
		    HTMLSetCharacterHandling(current_char_set);
		}

		if (!I.type)
		    LYUsePlainSpace = TRUE;
		else if (!strcasecomp(I.type, "text") ||
			 !strcasecomp(I.type, "submit") ||
			 !strcasecomp(I.type, "reset"))
		    LYUsePlainSpace = TRUE;
		if (current_char_set && LYUsePlainSpace)
		    LYExpandString((char**)&value[HTML_INPUT_VALUE]);
	        LYUnEscapeEntities((char *)value[HTML_INPUT_VALUE],
				   LYUsePlainSpace, LYHiddenValue);
		I.value = (char *)value[HTML_INPUT_VALUE];
		if (LYUsePlainSpace == TRUE) {
		    /*
		     *  Convert any newlines or tabs to spaces,
		     *  and trim any lead or trailing spaces. - FM
		     */
		    convert_to_spaces(I.value);
		    while (I.value && I.value[0] == ' ')
		        I.value++;
		    len = strlen(I.value) - 1;
		    while (len > 0 && I.value[len] == ' ')
		        I.value[len--] = '\0';
		}
		LYUsePlainSpace = FALSE;
		

		if (I.type && !strcasecomp(I.type, "hidden")) {
		    LYHiddenValue = FALSE;
		    current_char_set = CurrentCharSet;
		    LYUseDefaultRawMode = CurrentUseDefaultRawMode;
		    HTMLSetCharacterHandling(current_char_set);
		    HTPassEightBitRaw = CurrentEightBitRaw;
		    HTCJK = CurrentHTCJK;
		}
	    }
	    if (present && present[HTML_INPUT_CHECKED])
		I.checked = YES;
	    if (present && present[HTML_INPUT_SIZE] &&
	        value[HTML_INPUT_SIZE] && *value[HTML_INPUT_SIZE])
		I.size = (char *)value[HTML_INPUT_SIZE];
	    if (present && present[HTML_INPUT_MAXLENGTH] &&
	        value[HTML_INPUT_MAXLENGTH] && *value[HTML_INPUT_MAXLENGTH])
		I.maxlength = (char *)value[HTML_INPUT_MAXLENGTH];
	    if (present && present[HTML_INPUT_DISABLED])
		I.disabled = YES;

	    if (present && present[HTML_INPUT_ALIGN] && /* Not yet used. */
	        value[HTML_INPUT_ALIGN] && *value[HTML_INPUT_ALIGN])
		I.align = (char *)value[HTML_INPUT_ALIGN];
	    if (present && present[HTML_INPUT_CLASS] && /* Not yet used. */
	        value[HTML_INPUT_CLASS] && *value[HTML_INPUT_CLASS])
		I.class = (char *)value[HTML_INPUT_CLASS];
	    if (present && present[HTML_INPUT_ERROR] && /* Not yet used. */
	        value[HTML_INPUT_ERROR] && *value[HTML_INPUT_ERROR])
		I.error = (char *)value[HTML_INPUT_ERROR];
	    if (present && present[HTML_INPUT_HEIGHT] && /* Not yet used. */
	        value[HTML_INPUT_HEIGHT] && *value[HTML_INPUT_HEIGHT])
		I.height = (char *)value[HTML_INPUT_HEIGHT];
	    if (present && present[HTML_INPUT_WIDTH] && /* Not yet used. */
	        value[HTML_INPUT_WIDTH] && *value[HTML_INPUT_WIDTH])
		I.width = (char *)value[HTML_INPUT_WIDTH];
	    if (present && present[HTML_INPUT_ID] &&
	        value[HTML_INPUT_ID] && *value[HTML_INPUT_ID]) {
		I.id = (char *)value[HTML_INPUT_ID];
		HTML_CheckForID(me, present, value, (int)HTML_INPUT_ID);
	    }
	    if (present && present[HTML_INPUT_LANG] && /* Not yet used. */
	        value[HTML_INPUT_LANG] && *value[HTML_INPUT_LANG])
		I.lang = (char *)value[HTML_INPUT_LANG];
	    if (present && present[HTML_INPUT_MD] && /* Not yet used. */
	        value[HTML_INPUT_MD] && *value[HTML_INPUT_MD])
		I.md = (char *)value[HTML_INPUT_MD];
	    if (present && present[HTML_INPUT_SRC] && /* Not yet used. */
	        value[HTML_INPUT_MD] && *value[HTML_INPUT_MD])
		I.src = (char *)value[HTML_INPUT_SRC]; /* Should be resolved. */

	    chars = HText_beginInput(me->text, &I);
	    if (me->sp[0].tag_number == HTML_PRE && chars > 20) {
	        /*
		 *  The code inadequately handles INPUT fields in PRE tags.
		 *  We'll put up a minimum of 20 underscores, and if any
		 *  more would exceed the wrap column, we'll ignore them.
		 */
	        int i;
		for (i = 0; i < 20; i++) {
	            HTML_put_character(me, '_');
		    chars--;
		}
		ignore_excess = TRUE;
	    }
	    for (; chars > 0; chars--)
	    	HTML_put_character(me, '_');
	    ignore_excess = FALSE;
	}
	break;

    case HTML_TEXTAREA:
	/*
	 *  Make sure we're in a form.
	 */
	if (!B_inFORM) {
	    if (TRACE) {
		fprintf(stderr,
			"HTML: TEXTAREA start tag not within FORM tag\n");
	    } else if (!B_inBadHTML) {
	        _statusline(BAD_HTML_USE_TRACE);
		B_inBadHTML = TRUE;
	        sleep(MessageSecs);
	    }
	    /*
	     *  Too likely to cause a crash, so we'll ignore it. - FM
	     */
	    break;
	}

	/* 
	 *  Set to know we are in a textarea.
	 */
	B_inTEXTAREA = TRUE;

	/*
	 *  Get ready for the value.
	 */
        HTChunkClear(&me->textarea);
	if (present && present[HTML_TEXTAREA_NAME] &&
	    value[HTML_TEXTAREA_NAME])  
	    StrAllocCopy(textarea_name, value[HTML_TEXTAREA_NAME]);
	else
	    StrAllocCopy(textarea_name, "");

	if (present && present[HTML_TEXTAREA_COLS] &&
	    value[HTML_TEXTAREA_COLS] &&
	    isdigit((unsigned char)*value[HTML_TEXTAREA_COLS]))  
	    StrAllocCopy(textarea_cols, value[HTML_TEXTAREA_COLS]);
	else
	    StrAllocCopy(textarea_cols, "60");

	if (present && present[HTML_TEXTAREA_ROWS] &&
	    value[HTML_TEXTAREA_ROWS] &&
	    isdigit((unsigned char)*value[HTML_TEXTAREA_ROWS]))  
	    textarea_rows = atoi(value[HTML_TEXTAREA_ROWS]);
	else
	    textarea_rows = 4;

	if (present && present[HTML_TEXTAREA_DISABLED])
	    textarea_disabled = YES;
	else
	    textarea_disabled = NO;

	if (present && present[HTML_TEXTAREA_ID]
	    && value[HTML_TEXTAREA_ID] && *value[HTML_TEXTAREA_ID]) {
	    StrAllocCopy(id_string, value[HTML_TEXTAREA_ID]);
	    LYUnEscapeToLatinOne(&id_string, TRUE);
	    if ((id_string != '\0') &&
	        (B_ID_A = HTAnchor_findChildAndLink(
				me->node_anchor,	 /* Parent */
				id_string,		 /* Tag */
				0,			 /* Addresss */
				0))) {			 /* Type */
		if (!me->text)
		    UPDATE_STYLE;
		HText_beginAnchor(me->text, B_ID_A);
		HText_endAnchor(me->text);
		StrAllocCopy(textarea_id, id_string);
	    } else {
	        FREE(textarea_id);
	    }
	    FREE(id_string);
	} else {
	    FREE(textarea_id);
	}
	break;

    case HTML_SELECT:
	{
	    char *name = NULL;
	    BOOLEAN multiple=NO;
	    char *size = NULL;

            /*
	     *  Initialize the disable attribute.
	     */
	    select_disabled = NO;

	    /*
	     *  Make sure we're in a form.
	     */
	    if (!B_inFORM) {
	        if (TRACE) {
		    fprintf(stderr,
			    "HTML: SELECT start tag not within FORM tag\n");
		} else if (!B_inBadHTML) {
		    _statusline(BAD_HTML_USE_TRACE);
		    B_inBadHTML = TRUE;
		    sleep(MessageSecs);
		}

	        /*
		 *  Too likely to cause a crash, so we'll ignore it. - FM
		 */
		break;
	    }

	    /*
	     *  Check for unclosed TEXTAREA.
	     */
	    if (B_inTEXTAREA) {
	        if (TRACE) {
		    fprintf(stderr, "HTML: Missing TEXTAREA end tag\n");
		} else if (!B_inBadHTML) {
		    _statusline(BAD_HTML_USE_TRACE);
		    B_inBadHTML = TRUE;
		    sleep(MessageSecs);
		}
	    }

	    /*
	     *  Set to know we are in a select tag.
	     */
	    B_inSELECT = TRUE;

	    if (!me->text)
	        UPDATE_STYLE;
	    if (present && present[HTML_SELECT_NAME] &&
	        value[HTML_SELECT_NAME] && *value[HTML_SELECT_NAME])  
		StrAllocCopy(name, value[HTML_SELECT_NAME]);
	    else
	        StrAllocCopy(name, "");
	    if (present && present[HTML_SELECT_MULTIPLE])  
		multiple=YES;
	    if (present && present[HTML_SELECT_DISABLED])  
		select_disabled=YES;
	    if (present && present[HTML_SELECT_SIZE] &&
	        value[HTML_SELECT_SIZE] && *value[HTML_SELECT_SIZE]) {
		StrAllocCopy(size, value[HTML_SELECT_SIZE]);
	    }

	    if (B_inBoldH == TRUE && multiple == NO) {
	        HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
		B_inBoldH = FALSE;
		B_needBoldH = TRUE;
	    }
	    if (B_inUnderline == TRUE && multiple == NO) {
	        HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
		B_inUnderline = FALSE;
	    }

	    HTML_CheckForID(me, present, value, (int)HTML_SELECT_ID);

	    HText_beginSelect(name, multiple, size);
	    FREE(name);
	    FREE(size);

	    first_option = TRUE;
	}
	break;

    case HTML_OPTION:
	{
	    /*
	     *  An option is a special case of an input field.
	     */
	    InputFieldData I;
	    int i;

	    /*
	     *  Make sure we're in a select tag.
	     */
	    if (!B_inSELECT) {
	        if (TRACE) {
		    fprintf(stderr,
			    "HTML: OPTION tag not within SELECT tag\n");
		} else if (!B_inBadHTML) {
		    _statusline(BAD_HTML_USE_TRACE);
		    B_inBadHTML = TRUE;
		    sleep(MessageSecs);
		}

	        /*
		 *  Too likely to cause a crash, so we'll ignore it. - FM
		 */
		break;
	    }

	    if (!me->text)
	        UPDATE_STYLE;
	    if (!first_option) {
	        /*
		 *  Finish the data off.
		 */
       	        HTChunkTerminate(&me->option);

		/*
		 *  Finish the previous option @@@@@
		 */
	        HText_setLastOptionValue(me->text,
					 me->option.data, LastOptionValue,
				         MIDDLE_ORDER, LastOptionChecked);
	    }

	    /*
	     *  If its not a multiple option list then don't
	     *  use the checkbox method, and don't put
	     *  anything on the screen yet.
	     */
	    if (first_option || HTCurSelectGroupType == F_CHECKBOX_TYPE) {
		if (HTCurSelectGroupType == F_CHECKBOX_TYPE) {
	            /*
		     *  Start a newline before each option.
		     */
		    HTML_EnsureSingleSpace(me);
		} else {
		    /*
		     *  Add option list designation character.
		     */
                    HText_appendCharacter(me->text, '[');
		    me->in_word = YES;
		}

                /*
		 *  Inititialize.
		 */
		I.align=NULL; I.accept=NULL; I.checked=NO; I.class=NULL;
		I.disabled=NO; I.error=NULL; I.height= NULL; I.id=NULL;
		I.lang=NULL; I.max=NULL; I.maxlength=NULL; I.md=NULL;
		I.min=NULL; I.name=NULL; I.size=NULL; I.src=NULL;
		I.type=NULL; I.value=NULL; I.width=NULL;

	        I.type = "OPTION";
    
	        if (present && present[HTML_OPTION_SELECTED])
		    I.checked=YES;

		if (present && present[HTML_OPTION_VALUE] &&
		    value[HTML_OPTION_VALUE]) {
	            /*
		     *  Convert any HTML entities or decimal escaping. - FM
		     */
		    int CurrentCharSet = current_char_set;
		    BOOL CurrentEightBitRaw = HTPassEightBitRaw;
		    BOOLEAN CurrentUseDefaultRawMode = LYUseDefaultRawMode;
		    HTCJKlang CurrentHTCJK = HTCJK;

		    if (CurrentCharSet) {
		        current_char_set = 0;	/* Default ISO-Latin1 */
			LYUseDefaultRawMode = TRUE;
			HTMLSetCharacterHandling(current_char_set);
		    }
	            LYUnEscapeEntities((char *)value[HTML_OPTION_VALUE],
		    		       LYUsePlainSpace, LYHiddenValue);
		    if (CurrentCharSet) {
		        current_char_set = CurrentCharSet;
			LYUseDefaultRawMode = CurrentUseDefaultRawMode;
			HTMLSetCharacterHandling(current_char_set);
			HTPassEightBitRaw = CurrentEightBitRaw;
			HTCJK = CurrentHTCJK;
		    }

		    I.value = (char *)value[HTML_OPTION_VALUE];
		}

	        if (select_disabled ||
		   (present && present[HTML_OPTION_DISABLED]))
		    I.disabled=YES;

	        if (present && present[HTML_OPTION_ID]
		    && value[HTML_OPTION_ID] && *value[HTML_OPTION_ID]) {
		    if (B_ID_A = HTAnchor_findChildAndLink(
				    me->node_anchor,	   /* Parent */
				    value[HTML_OPTION_ID], /* Tag */
				    0,			   /* Addresss */
				    0)) {		   /* Type */
			HText_beginAnchor(me->text, B_ID_A);
			HText_endAnchor(me->text);
		        I.id = (char *)value[HTML_OPTION_ID];
		    }
		}

	        HText_beginInput(me->text, &I);
    
	        first_option = FALSE;

		if (HTCurSelectGroupType == F_CHECKBOX_TYPE) {
	            /*
		     *  Put 3 underscores and one space before each option.
		     */
                    for (i = 0; i < 3; i++)
	    	        HText_appendCharacter(me->text, '_');
	            HText_appendCharacter(me->text, ' ');
		    me->in_word = NO;
		}
	    }

	    /*
	     *  Get ready for the next value.
	     */
            HTChunkClear(&me->option);
	    if (present && present[HTML_OPTION_SELECTED])
		LastOptionChecked=YES;
	    else
		LastOptionChecked=NO;

	    if (present && present[HTML_OPTION_VALUE] &&
	        value[HTML_OPTION_VALUE])
	        StrAllocCopy(LastOptionValue, value[HTML_OPTION_VALUE]);
	    else
	        StrAllocCopy(LastOptionValue, me->option.data);
	}
	break;

    case HTML_TABLE:
        UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_TABLE_ID);
	B_inTABLE = TRUE;
        break;

    case HTML_TR:
        /*
	 *  Not yet implemented.  Just start a new row,
	 *  if needed, and check for an ID link. - FM
	 */
        UPDATE_STYLE;
	if (HText_LastLineSize(me->text)) {
	    HText_appendCharacter(me->text, '\r');
	}
	HTML_CheckForID(me, present, value, (int)HTML_TR_ID);
	me->in_word = NO;
        break;

    case HTML_THEAD:
    case HTML_TFOOT:
    case HTML_TBODY:
        /*
	 *  Not yet implemented.  Just check for an ID link. - FM
	 */
        UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_TR_ID);
        break;
    
    case HTML_COL:
    case HTML_COLGROUP:
        /*
	 *  Not yet implemented.  Just check for an ID link. - FM
	 */
        UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_COL_ID);
        break;
    
    case HTML_TH:
        UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_TD_ID);
        /*
	 *  Not yet implemented.  Just add a collapsible space and break. - FM
	 */
	HTML_put_character(me, ' ');
	me->in_word = NO;
        break;

    case HTML_TD:
        UPDATE_STYLE;
	HTML_CheckForID(me, present, value, (int)HTML_TD_ID);
        /*
	 *  Not yet implemented.  Just add a collapsible space and break. - FM
	 */
	HTML_put_character(me, ' ');
	me->in_word = NO;
        break;

    case HTML_MATH:
        /*
	 *  We're getting it as Litteral text, which, until we can process
	 *  it, we'll display as is, within brackets to alert the user. - FM
	 */
	if (!me->text)
	    UPDATE_STYLE;
	HTChunkClear(&me->math);
	HTML_CheckForID(me, present, value, (int)HTML_GEN_ID);
	break;

    default:
	break;

    } /* end switch */

    if (HTML_dtd.tags[element_number].contents!= SGML_EMPTY) {
	if (me->sp == me->stack) {
            fprintf(stderr, 
		"HTML: ****** Maximum nesting of %d tags exceeded!\n",
            	MAX_NESTING);
            return;
        }

    	(me->sp)--;
	me->sp[0].style = me->new_style;	/* Stack new style */
	me->sp[0].tag_number = element_number;

	if (TRACE)
	    fprintf(stderr,"HTML:begin_element: adding style to stack - %s\n",
							me->new_style->name);
    }	
}

/*		End Element
**		-----------
**
**	When we end an element, the style must be returned to that
**	in effect before that element.  Note that anchors (etc?)
**	don't have an associated style, so that we must scan down the
**	stack for an element with a defined style. (In fact, the styles
**	should be linked to the whole stack not just the top one.)
**	TBL 921119
**
**	We don't turn on "CAREFUL" check because the parser produces
**	(internal code errors apart) good nesting. The parser checks
**	incoming code errors, not this module.
*/
PRIVATE void HTML_end_element ARGS3(
	HTStructured *,		me,
	int,			element_number,
	char **,		include)
{
#ifdef CAREFUL			/* parser assumed to produce good nesting */
    if (element_number != me->sp[0].tag_number) {
        fprintf(stderr, 
		"HTMLText: end of element %s when expecting end of %s\n",
		HTML_dtd.tags[element_number].name,
		HTML_dtd.tags[me->sp->tag_number].name);
		/* panic */
    }
#endif
    if (LYMapsOnly) {
        if (!(element_number == HTML_MAP || element_number == HTML_AREA)) {
	    return;
	}
    }
    
    if (me->sp < me->stack + MAX_NESTING+1) {
        (me->sp)++;				/* Pop state off stack */
        if (TRACE)
	    fprintf(stderr,
	    	    "HTML:end_element: Popped style off stack - %s\n",
		    me->sp->style->name);
    } else {
	if (TRACE)
	    fprintf(stderr,
  "Stack underflow error!  Tried to pop off more styles than exist in stack\n");
    }
    
    /* Check for unclosed TEXTAREA */
    if (B_inTEXTAREA && element_number != HTML_TEXTAREA)
        if (TRACE) {
	    fprintf(stderr, "HTML: Missing TEXTAREA end tag\n");
	} else if (!B_inBadHTML) {
	    _statusline(BAD_HTML_USE_TRACE);
	    B_inBadHTML = TRUE;
	    sleep(MessageSecs);
	}

    switch(element_number) {

    case HTML_HTML:
	List_Nesting_Level = -1;
	HTML_zero_OL_Counter();
	Division_Level = -1;
	Underline_Level = 0;
	Quote_Level = 0;
	if (B_inUnderline) {
	    if (!me->text)
	        UPDATE_STYLE;
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    B_inUnderline = FALSE;
	}
	FREE(base_href);
        B_inBASE = FALSE;
	if (B_inA || B_inFORM || B_inSELECT || B_inTEXTAREA)
	    if (TRACE)
	        fprintf(stderr,
			"HTML: Something not closed before HTML close-tag\n");
	    else if (!B_inBadHTML) {
	        _statusline(BAD_HTML_USE_TRACE);
	    }
	break;

    case HTML_HEAD:
        List_Nesting_Level = -1;
	HTML_zero_OL_Counter();
	Division_Level = -1;
	Underline_Level = 0;
	Quote_Level = 0;
	if (B_inUnderline) {
	    if (!me->text)
	        UPDATE_STYLE;
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    B_inUnderline = FALSE;
	}
	break;

#ifdef NOT_DEFINED /* BASE will be a container in HTML+ */
    case HTML_BASE:
	FREE(base_href);
        B_inBASE = FALSE;
	break;
#endif /* NOT_DEFINED */

    case HTML_TITLE:
        HTChunkTerminate(&me->title);
    	HTAnchor_setTitle(me->node_anchor, me->title.data);
        HTChunkClear(&me->title);
	break;
	
    case HTML_STYLE:
	if (!me->text)
	    UPDATE_STYLE;
    	/*
	 *  We're getting it as Litteral text, which, for now,
	 *  we'll just ignore. - FM
	 */
	HTChunkTerminate(&me->style_block);
	if (TRACE) {
	    fprintf(stderr, "HTML: STYLE content =\n%s\n",
	    		    me->style_block.data);
	}
	HTChunkClear(&me->style_block);
	break;

    case HTML_SCRIPT:
	if (!me->text)
	    UPDATE_STYLE;
    	/*
	 *  We're getting it as Litteral text, which, for now,
	 *  we'll just ignore. - FM
	 */
	HTChunkTerminate(&me->script);
	if (TRACE) {
	    fprintf(stderr, "HTML: SCRIPT content =\n%s\n",
	    		    me->script.data);
	}
	HTChunkClear(&me->script);
	break;

    case HTML_BODY:
	List_Nesting_Level = -1;
	HTML_zero_OL_Counter();
	Division_Level = -1;
	Underline_Level = 0;
	Quote_Level = 0;
	if (B_inUnderline) {
	    if (!me->text)
	        UPDATE_STYLE;
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    B_inUnderline = FALSE;
	}
	if (B_inA || B_inFORM || B_inSELECT || B_inTEXTAREA)
	    if (TRACE)
	        fprintf(stderr,
			"HTML: Something not closed before BODY close-tag\n");
	    else if (!B_inBadHTML) {
	        _statusline(BAD_HTML_USE_TRACE);
	    }
	break;

    case HTML_FRAMESET:
	if (!me->text)
	    UPDATE_STYLE;
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	break;

    case HTML_NOFRAMES:
	if (!me->text)
	    UPDATE_STYLE;
	HTML_EnsureDoubleSpace(me);
	HTML_ResetParagraphAlignment(me);
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	break;

    case HTML_BANNER:
    case HTML_MARQUEE:
    case HTML_BLOCKQUOTE:
    case HTML_BQ:
    case HTML_ADDRESS:
		/*
		 *  Set flag to know that style has ended.
		 *  Fall through.
		i_prior_style = -1;
		 */
	change_paragraph_style(me, me->sp->style);
	UPDATE_STYLE;
	if (List_Nesting_Level >= 0)
	    HText_NegateLineOne(me->text);
	break;

    case HTML_CENTER:
    case HTML_DIV:
	if (Division_Level >= 0)
	    Division_Level--;
	if (Division_Level >= 0)
	    me->sp->style->alignment = DivisionAlignments[Division_Level];
	change_paragraph_style(me, me->sp->style);
	UPDATE_STYLE;
	current_default_alignment = me->sp->style->alignment;
	if (List_Nesting_Level >= 0)
	    HText_NegateLineOne(me->text);
	break;

    case HTML_H1:                       /* header styles */
    case HTML_H2:
    case HTML_H3:
    case HTML_H4:
    case HTML_H5:
    case HTML_H6:
	if (Division_Level >= 0) {
	    me->sp->style->alignment = DivisionAlignments[Division_Level];
	}
	change_paragraph_style(me, me->sp->style);
	UPDATE_STYLE;
	if (styles[element_number]->font & HT_BOLD) {
	    if (B_inBoldA == FALSE && B_inBoldH == TRUE) {
	        HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
	    }
	    B_inBoldH = FALSE;
	}
	if (List_Nesting_Level >= 0)
	    HText_NegateLineOne(me->text);
	if (Underline_Level > 0 && B_inUnderline == FALSE) {
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	    B_inUnderline = TRUE;
	}
	break;

#ifdef NOTDEFINED
    case HTML_P:
        break;
#endif /* NOTDEFINED */

    case HTML_B:			/* Physical character highlighting */
    case HTML_BLINK:
    case HTML_I:
    case HTML_U:
    
    case HTML_CITE:			/* Logical character highlighting */
    case HTML_EM:
    case HTML_STRONG:
	if (!me->text)
	    UPDATE_STYLE;
	if (Underline_Level > 0)
	    Underline_Level--;
	if (B_inUnderline == TRUE && Underline_Level < 1) {
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    B_inUnderline = FALSE;
	    if (TRACE)
	        fprintf(stderr,"Ending underline\n");
	} else {
	    if (TRACE)
	        fprintf(stderr,"Underline Level is %d\n", Underline_Level);
	}
    	break;
	
    case HTML_ABBREV:	/* Miscellaneous character containers */
    case HTML_ACRONYM:
    case HTML_AU:
    case HTML_AUTHOR:
    case HTML_BIG:
    case HTML_CODE:
    case HTML_DFN:
    case HTML_KBD:
    case HTML_SAMP:
    case HTML_SMALL:
    case HTML_SUB:
    case HTML_SUP:
    case HTML_TT:
    case HTML_VAR:
	break;

    case HTML_DEL:
    case HTML_S:
    case HTML_STRIKE:
        if (!me->text)
	    UPDATE_STYLE;
	HTML_put_character(me, ' ');
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	HTML_put_string(me, ":DEL]");
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	HTML_put_character(me, ' ');
	me->in_word = NO;
	break;

    case HTML_INS:
        if (!me->text)
	    UPDATE_STYLE;
	HTML_put_character(me, ' ');
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	HTML_put_string(me, ":INS]");
	if (B_inUnderline == FALSE)
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	HTML_put_character(me, ' ');
	me->in_word = NO;
	break;

    case HTML_Q:
        if (Quote_Level > 0)
	    Quote_Level--;
        /*
	 *  Should check LANG and/or DIR attributes, and the
	 *  me->node_anchor->charset and/or yet to be added
	 *  structure elements, to determine whether we should
	 *  use chevrons, but for now we'll always use double-
	 *  or single-quotes. - FM
	 */
	if (Quote_Level == ((Quote_Level/2)*2))
	    HText_appendCharacter(me->text, '"');
	else
	    HText_appendCharacter(me->text, '\'');
        break;

    case HTML_PRE:				/* Formatted text */
    case HTML_LISTING:				/* Litteral text */
    case HTML_XMP:
    case HTML_PLAINTEXT:
	if (!me->text)
	    UPDATE_STYLE;
    	if (me->comment_start)
    	    HText_appendText(me->text, me->comment_start);
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	break;

    case HTML_NOTE:
    case HTML_FN:
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	UPDATE_STYLE;
	if (List_Nesting_Level >= 0)
	    HText_NegateLineOne(me->text);
	B_inLABEL = FALSE;
	break;

    case HTML_OL:
        OL_Counter[List_Nesting_Level < 6 ?
		   List_Nesting_Level : 6] = OL_VOID;
    case HTML_DL:
    case HTML_UL:
    case HTML_MENU:
    case HTML_DIR:
	List_Nesting_Level--;
	if (TRACE)
	    fprintf(stderr,"Reducing List Nesting Level to %d\n",
						    List_Nesting_Level);
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	UPDATE_STYLE;
	if (List_Nesting_Level >= 0)
	    HText_NegateLineOne(me->text);
        break;

    case HTML_SPAN:
        /*
	 *  Should undo anything we did based on LANG and/or DIR
	 *  attributes, and the me->node_anchor->charset and/or
	 *  yet to be added structure elements. - FM
	 */
        break;

    case HTML_BDO:
        /*
	 *  Should undo anything we did based on DIR (and/or LANG)
	 *  attributes, and the me->node_anchor->charset and/or
	 *  yet to be added structure elements. - FM
	 */
        break;

    case HTML_A:
	/*	Set to know that we are no longer in an anchor.
	 */
	B_inA = FALSE;

	UPDATE_STYLE;
	if (B_inBoldA == TRUE && B_inBoldH == FALSE)
	    HText_appendCharacter(me->text,LY_BOLD_END_CHAR);
	HText_endAnchor(me->text);
	B_inBoldA = FALSE;
	if (Underline_Level > 0 && B_inUnderline == FALSE) {
	    HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	    B_inUnderline = TRUE;
	}
	break;

    case HTML_MAP:
        FREE(LYMapName);
        break;

    case HTML_BODYTEXT:
        /*
	 *  We may need to look at this someday to deal with
	 *  OBJECTs optimally, but just ignore it for now. - FM
	 */
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	break;

    case HTML_TEXTFLOW:
        /*
	 *  We may need to look at this someday to deal with
	 *  APPLETs optimally, but just ignore it for now. - FM
	 */
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	break;

    case HTML_FIG:
	if (B_inFIGwithP) {
	    HTML_EnsureDoubleSpace(me);
	} else {
	    HTML_put_character(me, ' ');  /* space char may be ignored */
	}
	HTML_ResetParagraphAlignment(me);
	B_inFIGwithP = FALSE;
    	B_inFIG = FALSE;
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	break;

    case HTML_OBJECT:
	if (!me->text)
	    UPDATE_STYLE;
	/*
	 *  Finish the data off.
	 */
	{
	    int s = 0, e = 0, i = 0, n = 0;
	    char *start = NULL, *first_end = NULL;
	    BOOL have_param = FALSE;
	    char *cp = NULL, *data = NULL;
	    HTPresentation *Pres;

	    HTChunkTerminate(&me->object);
	    data = me->object.data;
	    while ((cp = strchr(data, '<')) != NULL) {
	        /*
		 *  Look for nested OBJECTs.  This procedure
		 *  could get tripped up if invalid comments
		 *  are present in the content, or if an OBJECT
		 *  end tag is present in a quoted attribute. - FM
		 */
		if (!strncmp(cp, "<!--", 4)) {
		    data = LYFindEndOfComment(cp);
		    cp = data;
		} else if (s == 0 && !strncasecomp(cp, "<PARAM", 6)) {
		    have_param = TRUE;
	        } else if (!strncasecomp(cp, "<OBJECT", 7)) {
		    if (s == 0)
		        start = cp;
		    s++;
		} else if (!strncasecomp(cp, "</OBJECT", 8)) {
		    if (e == 0)
		        first_end = cp;
		    e++;
		}
		data = ++cp;
	    }
	    if (s > e) {
	        /*
		 *  We have nested OBJECT tags, and not yet all of the
		 *  end tags, so restore an end tag to the content, and
		 *  pass a dummy start tag to the SGML parser so that it
		 *  will resume the accumulation of OBJECT content. - FM
		 */
		if (TRACE)
		    fprintf(stderr, "HTML: Nested OBJECT tags.  Recycling.\n");
		if (*include == NULL) {
		    StrAllocCopy(*include, "<OBJECT>");
		} else {
		    if (0 && strstr(*include, me->object.data) == NULL) {
		        StrAllocCat(*include, "<OBJECT>");
		    }
		}
	        me->object.size--;
		HTChunkPuts(&me->object, "</OBJECT>");
		change_paragraph_style(me, me->sp->style);
		break;
	    }
	    if (s < e) {
	        /*
		 *  We had more end tags than start tags, so
		 *  we have bad HTML or otherwise misparsed. - FM
		 */
		if (TRACE) {
		    fprintf(stderr,
      "HTML: Unmatched OBJECT start and end tags.  Discarding content:\n%s\n",
			    me->object.data);
		} else if (!B_inBadHTML) {
		    _statusline(BAD_HTML_USE_TRACE);
		    B_inBadHTML = TRUE;
		    sleep(MessageSecs);
		}
		goto End_Object;
	    }

	    /*
	     *  OBJECT start and end tags are fully matched,
	     *  assuming we weren't tripped up by comments
	     *  or quoted attributes. - FM
	     */
	    if (TRACE)
	        fprintf(stderr, "HTML:OBJECT content:\n%s\n", me->object.data);

	    /*
	     *  OBJECTs with DECLARE should be saved but
	     *  not instantiated, and if nested, can have
	     *  only other DECLAREd OBJECTs.  Until we have
	     *  code to handle these, we'll just create an
	     *  anchor for the ID, if present, and discard
	     *  the content (sigh 8-). - FM
	     */
	    if (me->object_declare == TRUE) {
	        if (me->object_id && *me->object_id)
		    HTML_HandleID(me, me->object_id);
		if (TRACE)
		    fprintf(stderr, "HTML: DECLAREd OBJECT.  Ignoring!\n");
	        goto End_Object;
	    }

	    /*
	     *  OBJECTs with NAME are for FORM submissions.
	     *  We'll just create an anchor for the ID, if
	     *  present, and discard the content until we
	     *  have code to handle these. (sigh 8-). - FM
	     */
	    if (me->object_name != NULL) {
	        if (me->object_id && *me->object_id)
		    HTML_HandleID(me, me->object_id);
		if (TRACE)
		    fprintf(stderr, "HTML: NAMEd OBJECT.  Ignoring!\n");
	        goto End_Object;
	    }

	    /*
	     *  Deal with any nested OBJECTs by descending
	     *  to the inner-most OBJECT. - FM
	     */
	    if (s > 0) {
		if (start != NULL &&
		    first_end != NULL && first_end > start) {
		    /*
		     *  Minumum requirements for the ad hoc parsing
		     *  to have succeeded are met.  We'll hope that
		     *  it did succeed. - FM
		     */
		    *first_end = '\0';
		    data = NULL;
		    StrAllocCopy(data, start);
		    if (e > 1) {
			for (i = e; i > 1; i--) {
			    StrAllocCat(data, "</OBJECT><OBJECT>");
			}
		    }
		    StrAllocCat(data, "</OBJECT>");
		    StrAllocCat(*include, data);
		    if (TRACE)
			fprintf(stderr, "HTML: Recycling nested OBJECT%s.\n",
					(e > 1) ? "s" : "");
		    FREE(data);
		    goto End_Object;
		} else {
		    if (TRACE) {
			fprintf(stderr,
	"HTML: Unmatched OBJECT start and end tags.  Discarding content.\n");
			goto End_Object;
		    } else if (!B_inBadHTML) {
		        _statusline(BAD_HTML_USE_TRACE);
			B_inBadHTML = TRUE;
			sleep(MessageSecs);
			goto End_Object;
		    }
		}
	    }

	    /*
	     *  If it's content has SHAPES, convert it to FIG. - FM
	     */
	    if (me->object_shapes == TRUE) {
		if (TRACE)
		    fprintf(stderr,
		    "HTML: OBJECT has SHAPES.  Converting to FIG.\n");
	        StrAllocCat(*include, "<FIG ISOBJECT IMAGEMAP");
		if (me->object_ismap == TRUE)
		    StrAllocCat(*include, " IMAGEMAP");
	        if (me->object_id != NULL) {
		    StrAllocCat(*include, " ID=\"");
		    StrAllocCat(*include, me->object_id);
		    StrAllocCat(*include, "\"");
		}
	        if (me->object_data != NULL &&
		    me->object_classid == NULL) {
		    StrAllocCat(*include, " SRC=\"");
		    StrAllocCat(*include, me->object_data);
		    StrAllocCat(*include, "\"");
		}
		StrAllocCat(*include, ">");
		me->object.size--;
		HTChunkPuts(&me->object, "</FIG>");
		HTChunkTerminate(&me->object);
		StrAllocCat(*include, me->object.data);
		goto End_Object;
	    }

	    /*
	     *  If it has a USEMAP attribute and didn't have SHAPES,
	     *  convert it to IMG. - FM
	     */
	    if (me->object_usemap != NULL) {
		if (TRACE)
		    fprintf(stderr,
		    "HTML: OBJECT has USEMAP.  Converting to IMG.\n");
Object_as_IMG:
	        StrAllocCat(*include, "<IMG ISOBJECT");
	        if (me->object_id != NULL) {
		    /*
		     *  Pass the ID. - FM
		     */
		    StrAllocCat(*include, " ID=\"");
		    StrAllocCat(*include, me->object_id);
		    StrAllocCat(*include, "\"");
		}
	        if (me->object_data != NULL &&
		    me->object_classid == NULL) {
		    /*
		     *  We have DATA with no CLASSID, so let's
		     *  hope it' equivalent to an SRC. - FM
		     */
		    StrAllocCat(*include, " SRC=\"");
		    StrAllocCat(*include, me->object_data);
		    StrAllocCat(*include, "\"");
		}
	        if (me->object_title != NULL) {
		    /*
		     *  Use the TITLE for both the MAP
		     *  and the IMGs ALT. - FM
		     */
		    StrAllocCat(*include, " TITLE=\"");
		    StrAllocCat(*include, me->object_title);
		    StrAllocCat(*include, "\" ALT=\"");
		    StrAllocCat(*include, me->object_title);
		    StrAllocCat(*include, "\"");
		}
		/*
		 *  Add the USEMAP, and an ISMAP if present. - FM
		 */
		if (me->object_usemap != NULL) {
		    StrAllocCat(*include, " USEMAP=\"");
		    StrAllocCat(*include, me->object_usemap);
		    if (me->object_ismap == TRUE)
		        StrAllocCat(*include, "\" ISMAP>");
		    else
		        StrAllocCat(*include, "\">");
		} else {
		    StrAllocCat(*include, ">");
		}
		goto End_Object;
	    }

	    /*
	     *  Add an ID link if needed. - FM
	     */
	    if (me->object_id && *me->object_id)
	        HTML_HandleID(me, me->object_id);

	    /*
	     *  Add the OBJECTs content if not empty. - FM
	     */
	    if (me->object.size > 1)
		StrAllocCat(*include, me->object.data);

	    /*
	     *  Create a link to the DATA, if desired, and
	     *  we can rule out that it involves scripting
	     *  code.  This a risky thing to do, but we can
	     *  toggle clickable_images mode off if it really
	     *  screws things up, and so we may as well give
	     *  it a try. - FM
	     */
	    if (clickable_images) {
	        if (me->object_data != NULL &&
	            !have_param &&
		    me->object_classid == NULL &&
		    me->object_codebase == NULL &&
		    me->object_codetype == NULL) {
		    /*
		     *  We have a DATA value and no need for scripting
		     *  code, so close the current Anchor, if one is
		     *  open, and add an Anchor for this source.  If
		     *  we also have a TYPE value, check whether it's
		     *  an image or not, and set the link name
		     *  accordingly. - FM
		     */
		    if (B_inA)
		        StrAllocCat(*include, "</A>");
		    StrAllocCat(*include, " -<A HREF=\"");
		    StrAllocCat(*include, me->object_data);
		    StrAllocCat(*include, "\">");
		    if ((me->object_type != NULL) &&
		        !strncasecomp(me->object_type, "image/", 6)) {
			StrAllocCat(*include, "(IMAGE)");
		    } else {
		        StrAllocCat(*include, "(OBJECT)");
		    }
		    StrAllocCat(*include, "</A> ");
		}
	    }
	}

        /*
	 *  Re-intialize all of the OBJECT elements. - FM
	 */
End_Object:
	HTChunkClear(&me->object);
	me->object_started = FALSE;
	me->object_declare = FALSE;
	me->object_shapes = FALSE;
	me->object_ismap = FALSE;
	FREE(me->object_usemap);
	FREE(me->object_id);
	FREE(me->object_title);
	FREE(me->object_data);
	FREE(me->object_type);
	FREE(me->object_classid);
	FREE(me->object_codebase);
	FREE(me->object_codetype);
	FREE(me->object_name);

	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	break;

    case HTML_APPLET:
	if (B_inAPPLETwithP) {
	    HTML_EnsureDoubleSpace(me);
	} else {
	    HTML_put_character(me, ' ');  /* space char may be ignored */
	}
	HTML_ResetParagraphAlignment(me);
	B_inAPPLETwithP = FALSE;
    	B_inAPPLET = FALSE;
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	break;

    case HTML_CAPTION:
	HTML_EnsureDoubleSpace(me);
	HTML_ResetParagraphAlignment(me);
	B_inCAPTION = FALSE;
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	B_inLABEL = FALSE;
	break;

    case HTML_CREDIT:
	HTML_EnsureDoubleSpace(me);
	HTML_ResetParagraphAlignment(me);
	B_inCREDIT = FALSE;
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	B_inLABEL = FALSE;
	break;

    case HTML_FORM:
	/* Make sure we had a form start tag. */
	if (!B_inFORM) {
	    if (TRACE) {
		fprintf(stderr, "HTML: Unmatched FORM end tag\n");
	    } else if (!B_inBadHTML) {
	        _statusline(BAD_HTML_USE_TRACE);
		B_inBadHTML = TRUE;
		sleep(MessageSecs);
	    }
	    /*
	     *  We probably did start a form, for which bad HTML
	     *  caused a substitution, so we'll try to end.
	     *
	    break;
	     */
	}

	/*
	 *  Set to know that we are no longer in an form.
	 */
	B_inFORM = FALSE;

	UPDATE_STYLE;
	HText_endForm(me->text);
	HText_appendCharacter(me->text, '\r'); 
	break;

    case HTML_FIELDSET:
        break;

    case HTML_LABEL:
        break;

    case HTML_TEXTAREA:
        {
            InputFieldData I;
            int chars;
 	    char *cp = NULL;
	    int i;

	    /*
	     *  Make sure we had a textarea start tag.
	     */
	    if (!B_inTEXTAREA) {
	        if (TRACE) {
		    fprintf(stderr, "HTML: Unmatched TEXTAREA end tag\n");
		} else if (!B_inBadHTML) {
		    _statusline(BAD_HTML_USE_TRACE);
		    B_inBadHTML = TRUE;
		    sleep(MessageSecs);
		}
		break;
	    }

	    /*
	     *  Set to know that we are no longer in a textarea tag.
	     */
	    B_inTEXTAREA = FALSE;

            /*
	     *  Initialize.
	     */
	    I.align=NULL; I.accept=NULL; I.checked=NO; I.class=NULL;
	    I.disabled=NO; I.error=NULL; I.height= NULL; I.id=NULL;
	    I.lang=NULL; I.max=NULL; I.maxlength=NULL; I.md=NULL;
	    I.min=NULL; I.name=NULL; I.size=NULL; I.src=NULL;
	    I.type=NULL; I.value=NULL; I.width=NULL;

            UPDATE_STYLE;
            /*
	     *  Before any input field add a space if necessary.
	     */
            HTML_put_character(me, ' ');
	    me->in_word = NO;
	    /*
	     *  Add a return.
	     */
	    HText_appendCharacter(me->text, '\r');

	    /*
	     *  Finish the data off.
	     */
            HTChunkTerminate(&me->textarea);

	    I.type = "textarea";
	    I.value = cp;  /* may be NULL */
	    I.size = textarea_cols;
	    I.name = textarea_name;
	    I.disabled = textarea_disabled;
	    I.id = textarea_id;

	    LYUsePlainSpace = TRUE;
	    if (current_char_set)
	        LYExpandString(&me->textarea.data);

	    cp = strtok(me->textarea.data, "\n");
	    LYUnEscapeEntities(cp, LYUsePlainSpace, LYHiddenValue);
	    for (i = 0; i < textarea_rows; i++) {
		I.value = cp;

                chars = HText_beginInput(me->text, &I);
	        for (; chars > 0; chars--)
	    	    HTML_put_character(me, '_');
	        HText_appendCharacter(me->text, '\r');
	
		cp = strtok(NULL, "\n");
		LYUnEscapeEntities(cp, LYUsePlainSpace, LYHiddenValue);
	    }

	    /*
	     *  Check for more data lines than the rows attribute.
   	     */
	    while (cp) {
		I.value = cp;

                chars = HText_beginInput(me->text, &I);
                for (chars = atoi(textarea_cols); chars>0; chars--)
                    HTML_put_character(me, '_');
                HText_appendCharacter(me->text, '\r');
        
                cp = strtok(NULL, "\n");
		LYUnEscapeEntities(cp, LYUsePlainSpace, LYHiddenValue);
            }

	    LYUsePlainSpace = FALSE;

	    HTChunkClear(&me->textarea);
	    FREE(textarea_name);
	    FREE(textarea_cols);
	    FREE(textarea_id);
	    break;
	}

    case HTML_SELECT:
	{
	    char *ptr;
	    if (!me->text)
	        UPDATE_STYLE;

	    /*
	     *  Make sure we had a select start tag.
	     */
	    if (!B_inSELECT) {
	        if (TRACE) {
		    fprintf(stderr, "HTML: Unmatched SELECT end tag\n");
		} else if (!B_inBadHTML) {
		    _statusline(BAD_HTML_USE_TRACE);
		    B_inBadHTML = TRUE;
		    sleep(MessageSecs);
		}
		break;
	    }

	    /*
	     *  Set to know that we are no longer in a select tag.
	     */
	    B_inSELECT = FALSE;

	    /*
	     *  Clear the disable attribute.
	     */
	    select_disabled=NO;

	    /*
	     *  Finish the data off.
	     */
       	    HTChunkTerminate(&me->option);
	    /* finish the previous option @@@@@ */
	    ptr = HText_setLastOptionValue(me->text,
	    				   me->option.data, LastOptionValue,
					   LAST_ORDER, LastOptionChecked);
	    FREE(LastOptionValue);

	    LastOptionChecked = FALSE;

	    if (HTCurSelectGroupType == F_CHECKBOX_TYPE) {
	            /*
		     *  Start a newline after the last checkbox option.
		     */
		    HTML_EnsureSingleSpace(me);
	    } else {
	        /*
		 *  Output popup box with the default option to screen,
		 *  but use non-breaking spaces for output.
		 */
	        if (ptr &&
		    me->sp[0].tag_number == HTML_PRE && strlen(ptr) > 20) {
	            /*
		     *  The code inadequately handles OPTION fields in PRE tags.
		     *  We'll put up a minimum of 20 characters, and if any
		     *  more would exceed the wrap column, we'll ignore them.
		     */
	            int i;
		    for (i = 0; i < 20; i++) {
		        if (*ptr == ' ')
	    	            HText_appendCharacter(me->text,HT_NON_BREAK_SPACE); 
		        else
	    	            HText_appendCharacter(me->text,*ptr);
			ptr++;
		    }
		    ignore_excess = TRUE;
	        }
	        for (; ptr && *ptr != '\0'; ptr++) {
		    if (*ptr == ' ')
	    	        HText_appendCharacter(me->text,HT_NON_BREAK_SPACE); 
		    else
	    	        HText_appendCharacter(me->text,*ptr);
		}
	        /*
		 *  Add end option character.
		 */
	        HText_appendCharacter(me->text, ']');
		HTML_Last_Char = ']';
		me->in_word = YES;
		ignore_excess = FALSE; 
	    }
    	    HTChunkClear(&me->option);

	    if (Underline_Level > 0 && B_inUnderline == FALSE) {
	        HText_appendCharacter(me->text,LY_UNDERLINE_START_CHAR);
	        B_inUnderline = TRUE;
	    }
	    if (B_needBoldH == TRUE && B_inBoldH == FALSE) {
	        HText_appendCharacter(me->text,LY_BOLD_START_CHAR);
		B_inBoldH = TRUE;
		B_needBoldH = FALSE;
	    }
	}
	break;

    case HTML_TABLE:
        B_inTABLE = FALSE;
        break;

#ifdef NOTDEFINED /* These are SGML_EMPTY for now. - FM */
    case HTML_TR:
        break;

    case HTML_THEAD:
    case HTML_TFOOT:
    case HTML_TBODY:
        break;

    case HTML_COLGROUP:
        break;

    case HTML_TH:
        break;

    case HTML_TD:
        break;
#endif /* NOTDEFINED */

    case HTML_MATH:
	{
	    int i;
	    if (!me->text)
	        UPDATE_STYLE;
            /*
	     *  We're getting it as Litteral text, which, until we can process
	     *  it, we'll display as is, within brackets to alert the user. - FM
	     */
	    HTChunkPutc(&me->math, ' ');
            HTChunkTerminate(&me->math);
	    if (me->math.size > 2) {
	        HTML_EnsureSingleSpace(me);
	        if (B_inUnderline == FALSE)
		    HText_appendCharacter(me->text, LY_UNDERLINE_START_CHAR);
		HTML_put_string(me, "[MATH:");
		HText_appendCharacter(me->text, LY_UNDERLINE_END_CHAR);
	        HTML_put_character(me, ' ');
		HTML_put_string(me, me->math.data);
		HText_appendCharacter(me->text, LY_UNDERLINE_START_CHAR);
		HTML_put_string(me, ":MATH]");
	        if (B_inUnderline == FALSE)
		    HText_appendCharacter(me->text, LY_UNDERLINE_END_CHAR);
		HTML_EnsureSingleSpace(me);
	    }
	    HTChunkClear(&me->math);
	    break;
	}

    default:
	change_paragraph_style(me, me->sp->style);  /* Often won't really change */
	break;
	
    } /* switch */
}

/*		Expanding entities
**		------------------
*/
/*	(In fact, they all shrink!)
*/
PRIVATE void HTML_put_entity ARGS2(HTStructured *, me, int, entity_number)
{
    HTML_put_string(me, p_entity_values[entity_number]);
}

/*	Free an HTML object
**	-------------------
**
**	If the document is empty, the text object will not yet exist.
**	So we could in fact abandon creating the document and return
**	an error code.  In fact an empty document is an important type
**	of document, so we don't.
**
**	If non-interactive, everything is freed off.   No: crashes -listrefs
**	Otherwise, the interactive object is left.	
*/
PUBLIC void HTML_free ARGS1(HTStructured *, me)
{
    UPDATE_STYLE;		/* Creates empty document here! */
    if (me->comment_end)
		HTML_put_string(me,me->comment_end);
    HText_endAppend(me->text);

    if (me->target) {
        (*me->targetClass._free)(me->target);
    }
    if (me->sp && me->sp->style && me->sp->style->name) {
        if (!strcmp(me->sp->style->name, "DivCenter") ||
	    !strcmp(me->sp->style->name, "HeadingCenter")) {
	    me->sp->style->alignment = HT_CENTER;
	} else if (!strcmp(me->sp->style->name, "DivRight") ||
		   !strcmp(me->sp->style->name, "HeadingRight")) {
	    me->sp->style->alignment = HT_RIGHT;
	} else  {
	    me->sp->style->alignment = HT_LEFT;
	}
	styles[HTML_PRE]->alignment = HT_LEFT;
    }
    FREE(base_href);
    FREE(LYMapName);
    FREE(me);
}

PRIVATE void HTML_abort ARGS2(HTStructured *, me, HTError, e)
{
    List_Nesting_Level = -1;
    HTML_zero_OL_Counter();
    Division_Level = -1;
    Underline_Level = 0;
    Quote_Level = 0;

    if (me->text) {
	if (B_inUnderline) {
	    HText_appendCharacter(me->text,LY_UNDERLINE_END_CHAR);
	    B_inUnderline = FALSE;
	}
	HText_endAppend(me->text);
    }

    if (me->target) {
        (*me->targetClass._abort)(me->target, e);
    }
    FREE(me);
}

/*	Get Styles from style sheet
**	---------------------------
*/
PRIVATE void get_styles NOARGS
{
    got_styles = YES;

    default_style =		HTStyleNamed(styleSheet, "Normal");

    styles[HTML_H1] =		HTStyleNamed(styleSheet, "Heading1");
    styles[HTML_H2] =		HTStyleNamed(styleSheet, "Heading2");
    styles[HTML_H3] =		HTStyleNamed(styleSheet, "Heading3");
    styles[HTML_H4] =		HTStyleNamed(styleSheet, "Heading4");
    styles[HTML_H5] =		HTStyleNamed(styleSheet, "Heading5");
    styles[HTML_H6] =		HTStyleNamed(styleSheet, "Heading6");
    styles[HTML_HCENTER] =	HTStyleNamed(styleSheet, "HeadingCenter");
    styles[HTML_HLEFT] =	HTStyleNamed(styleSheet, "HeadingLeft");
    styles[HTML_HRIGHT] =	HTStyleNamed(styleSheet, "HeadingRight");

    styles[HTML_DCENTER] =	HTStyleNamed(styleSheet, "DivCenter");
    styles[HTML_DLEFT] =	HTStyleNamed(styleSheet, "DivLeft");
    styles[HTML_DRIGHT] =	HTStyleNamed(styleSheet, "DivRight");

    styles[HTML_DL] =		HTStyleNamed(styleSheet, "Glossary");
	/* nested list styles */
    styles[HTML_DL1] =		HTStyleNamed(styleSheet, "Glossary1");
    styles[HTML_DL2] =		HTStyleNamed(styleSheet, "Glossary2");
    styles[HTML_DL3] =		HTStyleNamed(styleSheet, "Glossary3");
    styles[HTML_DL4] =		HTStyleNamed(styleSheet, "Glossary4");
    styles[HTML_DL5] =		HTStyleNamed(styleSheet, "Glossary5");
    styles[HTML_DL6] =		HTStyleNamed(styleSheet, "Glossary6");

    styles[HTML_UL] =
    styles[HTML_OL] =		HTStyleNamed(styleSheet, "List");
	/* nested list styles */
    styles[HTML_OL1] =		HTStyleNamed(styleSheet, "List1");
    styles[HTML_OL2] =		HTStyleNamed(styleSheet, "List2");
    styles[HTML_OL3] =		HTStyleNamed(styleSheet, "List3");
    styles[HTML_OL4] =		HTStyleNamed(styleSheet, "List4");
    styles[HTML_OL5] =		HTStyleNamed(styleSheet, "List5");
    styles[HTML_OL6] =		HTStyleNamed(styleSheet, "List6");

    styles[HTML_MENU] =
    styles[HTML_DIR] =		HTStyleNamed(styleSheet, "Menu");    
	/* nested list styles */
    styles[HTML_MENU1] =	HTStyleNamed(styleSheet, "Menu1");    
    styles[HTML_MENU2] =	HTStyleNamed(styleSheet, "Menu2");    
    styles[HTML_MENU3] =	HTStyleNamed(styleSheet, "Menu3");    
    styles[HTML_MENU4] =	HTStyleNamed(styleSheet, "Menu4");    
    styles[HTML_MENU5] =	HTStyleNamed(styleSheet, "Menu5");    
    styles[HTML_MENU6] =	HTStyleNamed(styleSheet, "Menu6");    

    styles[HTML_DLC] =		HTStyleNamed(styleSheet, "GlossaryCompact");
	/* nested list styles */
    styles[HTML_DLC1] =		HTStyleNamed(styleSheet, "GlossaryCompact1");
    styles[HTML_DLC2] =		HTStyleNamed(styleSheet, "GlossaryCompact2");
    styles[HTML_DLC3] =		HTStyleNamed(styleSheet, "GlossaryCompact3");
    styles[HTML_DLC4] =		HTStyleNamed(styleSheet, "GlossaryCompact4");
    styles[HTML_DLC5] =		HTStyleNamed(styleSheet, "GlossaryCompact5");
    styles[HTML_DLC6] =		HTStyleNamed(styleSheet, "GlossaryCompact6");

    styles[HTML_ADDRESS] =	HTStyleNamed(styleSheet, "Address");
    styles[HTML_BANNER] =	HTStyleNamed(styleSheet, "Banner");
    styles[HTML_BLOCKQUOTE] =	HTStyleNamed(styleSheet, "Blockquote");
    styles[HTML_BQ] =		HTStyleNamed(styleSheet, "Bq");
    styles[HTML_FN] =		HTStyleNamed(styleSheet, "Footnote");
    styles[HTML_NOTE] =		HTStyleNamed(styleSheet, "Note");
    styles[HTML_PLAINTEXT] =
    styles[HTML_XMP] =		HTStyleNamed(styleSheet, "Example");
    styles[HTML_PRE] =		HTStyleNamed(styleSheet, "Preformatted");
    styles[HTML_STYLE] =	HTStyleNamed(styleSheet, "Style");
    styles[HTML_LISTING] =	HTStyleNamed(styleSheet, "Listing");
}
/*				P U B L I C
*/

/*	Structured Object Class
**	-----------------------
*/
PUBLIC CONST HTStructuredClass HTMLPresentation = /* As opposed to print etc */
{		
	"text/html",
	HTML_free,
	HTML_abort,
	HTML_put_character, 	HTML_put_string,  HTML_write,
	HTML_start_element, 	HTML_end_element,
	HTML_put_entity
}; 

/*		New Structured Text object
**		--------------------------
**
**	The strutcured stream can generate either presentation,
**	or plain text, or HTML.
*/
PUBLIC HTStructured* HTML_new ARGS3(
	HTParentAnchor *, 	anchor,
	HTFormat,		format_out,
	HTStream*,		stream)
{

    HTStructured * me;
   
	/*  Reset to know that we aren't in a list, anchor, bold header
	 *  or paragraph.  Hmm... May as well reset all of the flags.
	 */
	List_Nesting_Level = -1;
	HTML_zero_OL_Counter();
	Division_Level = -1;
	Underline_Level = 0;
	Quote_Level = 0;
	FREE(base_href);
	FREE(LYMapName);

	B_inA = FALSE;
	B_inAPPLET = FALSE;
	B_inAPPLETwithP = FALSE;
	B_inBadHTML = FALSE;
        B_inBASE = FALSE;
	B_inBoldA = FALSE;
	B_inBoldH = FALSE;
	B_inCAPTION = FALSE;
	B_inCREDIT = FALSE;
	B_inFIG = FALSE;
	B_inFIGwithP = FALSE;
	B_inFORM = FALSE;
	B_inLABEL = FALSE;
	B_inP = FALSE;
	B_inPRE = FALSE;
	B_inSELECT = FALSE;
	B_inTABLE = FALSE;
	B_inUnderline = FALSE;

	B_needBoldH = FALSE;
	current_default_alignment=HT_LEFT;
 
    if (format_out != WWW_PLAINTEXT && format_out != WWW_PRESENT) {
        HTStream * intermediate = HTStreamStack(WWW_HTML, format_out,
						stream, anchor);
	if (intermediate)
	    return HTMLGenerator(intermediate);
        fprintf(stderr, "\n** Internal error: can't parse HTML to %s\n",
       		HTAtom_name(format_out));
        (void) signal(SIGHUP, SIG_DFL);
        (void) signal(SIGTERM, SIG_DFL);
#ifndef VMS
        (void) signal(SIGINT, SIG_DFL);
#endif /* !VMS */
#ifdef SIGTSTP
	if (no_suspend)
	  (void) signal(SIGTSTP,SIG_DFL);
#endif /* SIGTSTP */
	exit (-1);
    }

    me = (HTStructured*) calloc(sizeof(*me),1);
    if (me == NULL)
        outofmem(__FILE__, "HTML_new");

    if (!got_styles)
        get_styles();
    else
        default_style =	HTStyleNamed(styleSheet, "Normal");

    me->isa = &HTMLPresentation;
    me->node_anchor =  anchor;
    me->title.size = 0;
    me->title.growby = 128;
    me->title.allocated = 0;
    me->title.data = 0;
    me->object.size = 0;
    me->object.growby = 128;
    me->object.allocated = 0;
    me->object.data = 0;
    me->object_started = FALSE;
    me->object_declare = FALSE;
    me->object_shapes = FALSE;
    me->object_ismap = FALSE;
    me->object_id = NULL;
    me->object_title = NULL;
    me->object_data = NULL;
    me->object_type = NULL;
    me->object_classid = NULL;
    me->object_codebase = NULL;
    me->object_codetype = NULL;
    me->object_usemap = NULL;
    me->object_name = NULL;
    me->option.size = 0;
    me->option.growby = 128;
    me->option.allocated = 0;
    me->option.data = 0;
    me->textarea.size = 0;
    me->textarea.growby = 128;
    me->textarea.allocated = 0;
    me->textarea.data = 0;
    me->math.size = 0;
    me->math.growby = 128;
    me->math.allocated = 0;
    me->math.data = 0;
    me->style_block.size = 0;
    me->style_block.growby = 128;
    me->style_block.allocated = 0;
    me->style_block.data = 0;
    me->script.size = 0;
    me->script.growby = 128;
    me->script.allocated = 0;
    me->script.data = 0;
    me->text = 0;
    me->style_change = YES;	/* Force check leading to text creation */
    me->new_style = default_style;
    me->old_style = 0;
    me->sp = me->stack + MAX_NESTING - 1;
    me->sp->tag_number = -1;				/* INVALID */
    me->sp->style = default_style;			/* INVALID */
    me->sp->style->alignment = HT_LEFT;

    me->comment_start = NULL;
    me->comment_end = NULL;
    me->target = stream;
    if (stream)
        me->targetClass = *stream->isa;			/* Copy pointers */
    
    return (HTStructured*) me;
}

/*	HTConverter for HTML to plain text
**	----------------------------------
**
**	This will convert from HTML to presentation or plain text.
*/
PUBLIC HTStream* HTMLToPlain ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink)
{
    return SGML_new(&HTML_dtd, HTML_new(anchor, pres->rep_out, sink));
}

/*	HTConverter for HTML to C code
**	------------------------------
**
**	C code is like plain text but all non-preformatted code
**	is commented out.
**	This will convert from HTML to presentation or plain text.
*/
PUBLIC HTStream* HTMLToC ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink)
{
    HTStructured * html;
    
    (*sink->isa->put_string)(sink, "/* ");	/* Before even title */
    html = HTML_new(anchor, WWW_PLAINTEXT, sink);
    html->comment_start = "/* ";
    html->comment_end = " */\n";	/* Must start in col 1 for cpp */
/*    HTML_put_string(html,html->comment_start); */
    return SGML_new(&HTML_dtd, html);
}

/*	Presenter for HTML
**	------------------
**
**	This will convert from HTML to presentation or plain text.
**
**	Override this if you have a windows version
*/
#ifndef GUI
PUBLIC HTStream* HTMLPresent ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink)
{
    return SGML_new(&HTML_dtd, HTML_new(anchor, WWW_PRESENT, NULL));
}
#endif /* !GUI */

/*	Record error message as a hypertext object
**	------------------------------------------
**
**	The error message should be marked as an error so that
**	it can be reloaded later.
**	This implementation just throws up an error message
**	and leaves the document unloaded.
**	A smarter implementation would load an error document,
**	marking at such so that it is retried on reload.
**
** On entry,
**	sink 	is a stream to the output device if any
**	number	is the HTTP error number
**	message	is the human readable message.
**
** On exit,
**	returns	a negative number to indicate lack of success in the load.
*/
PUBLIC int HTLoadError ARGS3(
	HTStream *, 	sink,
	int,		number,
	CONST char *,	message)
{
    HTAlert(message);		/* @@@@@@@@@@@@@@@@@@@ */
    return -number;
} 

/*
**  This function checks whether we want to overrride
**  the current default alignment for parargraphs and
**  instead use that specified in the element's style
**  sheet. - FM
*/
PRIVATE BOOLEAN HTML_override_default_alignment ARGS1(
	HTStructured *, me)
{
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
**  This function initializes the Ordered List counter. - FM
*/
PRIVATE void HTML_zero_OL_Counter NOARGS
{
    int i;

    for (i = 0; i < 7; i++) {
        OL_Counter[i] = OL_VOID;
	OL_Type[i] = '1';
    }
	
    Last_OL_Count = 0;
    Last_OL_Type = '1';
    
    return;
}

/*
**  This function inserts newlines if needed to create double spacing,
**  and sets the left margin for subsequent text to the second line
**  indentation of the current style. - FM
*/
PRIVATE void HTML_EnsureDoubleSpace ARGS1(
	HTStructured *, me)
{
    if (!me || !me->text)
        return;

    if (HText_LastLineSize(me->text)) {
	HText_appendCharacter(me->text, '\r');
	HText_appendCharacter(me->text, '\r');
    } else if (HText_PreviousLineSize(me->text)) {
	HText_appendCharacter(me->text, '\r');
    } else if (List_Nesting_Level >= 0) {
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
PRIVATE void HTML_EnsureSingleSpace ARGS1(
	HTStructured *, me)
{
    if (!me || !me->text)
        return;

    if (HText_LastLineSize(me->text)) {
	HText_appendCharacter(me->text, '\r');
    } else if (List_Nesting_Level >= 0) {
	HText_NegateLineOne(me->text);
    }
    me->in_word = NO;
    return;
}

/*
**  This function resets paragraph alignments for block
**  elements which do not have a defined style sheet. - FM
*/
PRIVATE void HTML_ResetParagraphAlignment ARGS1(
	HTStructured *, me)
{
    if (!me)
        return;

    if (List_Nesting_Level >= 0 ||
	((Division_Level < 0) &&
	 (!strcmp(me->sp->style->name, "Normal") ||
	  !strcmp(me->sp->style->name, "Preformatted")))) {
	me->sp->style->alignment = HT_LEFT;
    } else {
	me->sp->style->alignment = current_default_alignment;
    }
    return;
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
PRIVATE void HTMLFillLocalFileURL ARGS2(
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
#ifdef NO_GETCWD
	getwd (curdir);
#else
	getcwd (curdir, DIRNAMESIZE);
#endif /* NO_GETCWD */
	StrAllocCat(*href, curdir);
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
**  This function creates NAMEd Anchors of a non-zero-length NAME
**  or ID attribute was present in the tag.
*/
PRIVATE void HTML_CheckForID ARGS4(
	HTStructured *,		me,
	CONST BOOL *,		present,
	CONST char **,		value,
	int,			attribute)
{
    HTChildAnchor *B_ID_A = NULL;
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
	    (B_ID_A = HTAnchor_findChildAndLink(
				me->node_anchor,	/* Parent */
				temp,			/* Tag */
				0,			/* Addresss */
				0))) {			/* Type */
	    HText_beginAnchor(me->text, B_ID_A);
	    HText_endAnchor(me->text);
	}
	FREE(temp);
    }

    return;
}

/*
**  This function creates a NAMEd Anchor for the ID string
**  passed to it directly as an argument.  It assumes the
**  does not need checking for character references. - FM
*/
PRIVATE void HTML_HandleID ARGS2(
	HTStructured *,		me,
	char *,			id)
{
    HTChildAnchor *B_ID_A = NULL;

    if (!(me && me->text) ||
        !(id && *id))
        return;

    /*
     *  Create the link if we still have a non-zero-length string. - FM
     */
    if (B_ID_A = HTAnchor_findChildAndLink(
				me->node_anchor,	/* Parent */
				id,			/* Tag */
				0,			/* Addresss */
				0)) {			/* Type */
	HText_beginAnchor(me->text, B_ID_A);
	HText_endAnchor(me->text);
    }

    return;
}
