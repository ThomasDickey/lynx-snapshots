/*			General SGML Parser code		SGML.c
**			========================
**
**	This module implements an HTStream object. To parse an
**	SGML file, create this object which is a parser. The object
**	is (currently) created by being passed a DTD structure,
**	and a target HTStructured oject at which to throw the parsed stuff.
**	
**	 6 Feb 93  Binary seraches used. Intreface modified.
*/
#include "HTUtils.h"
#include "tcp.h"		/* For FROMASCII */

#include "SGML.h"
#include "HTMLDTD.h"
#include "HTCJK.h"

#include <ctype.h>
/*#include <stdio.h> included in HTUtils.h -- FM */
#include "HTChunk.h"

#include "LYLeaks.h"

#define INVALID (-1)

#define FREE(x) if (x) {free(x); x = NULL;}

PUBLIC HTCJKlang HTCJK = NOCJK;		/* CJK enum value.		*/
PUBLIC BOOL HTPassEightBitRaw = FALSE;	/* Pass 161-172,174-255 raw.	*/ 
PUBLIC BOOL HTPassEightBitNum = FALSE;	/* Pass ^ numeric entities raw.	*/
PUBLIC BOOL HTPassHighCtrlRaw = FALSE;	/* Pass 127-160,173,&#127; raw.	*/
PUBLIC BOOL HTPassHighCtrlNum = FALSE;	/* Pass &#128;-&#159; raw.	*/

extern BOOLEAN LYCheckForCSI PARAMS((HTStructured *target, char **url));
extern void LYDoCSI PARAMS((char *url, CONST char *comment, char **csi));

/*	The State (context) of the parser
**
**	This is passed with each call to make the parser reentrant
**
*/

#define MAX_ATTRIBUTES 36	/* Max number of attributes per element */

	
/*		Element Stack
**		-------------
**	This allows us to return down the stack reselcting styles.
**	As we return, attribute values will be garbage in general.
*/
typedef struct _HTElement HTElement;
struct _HTElement {
	HTElement *	next;	/* Previously nested element or 0 */
	HTTag*		tag;	/* The tag at this level  */
};


/*	Internal Context Data Structure
**	-------------------------------
*/
struct _HTStream {

    CONST HTStreamClass *	isa;		/* inherited from HTStream */
    
    CONST SGML_dtd 		*dtd;
    HTStructuredClass		*actions;	/* target class  */
    HTStructured		*target;	/* target object */

    HTTag 			*current_tag;
    int 			current_attribute_number;
    HTChunk			*string;
    HTElement			*element_stack;
    enum sgml_state { S_text, S_litteral,
    		S_tag, S_tag_gap, S_attr, S_attr_gap, S_equals, S_value,
		S_ero, S_cro,
		S_exclamation, S_comment, S_doctype, S_marked,
		S_sgmlent, S_sgmlele, S_sgmlatt,
		S_squoted, S_dquoted, S_end, S_entity,
		S_esc,    S_dollar,    S_paren,    S_nonascii_text,
		S_dollar_paren,
		S_esc_sq, S_dollar_sq, S_paren_sq, S_nonascii_text_sq,
		S_dollar_paren_sq,
		S_esc_dq, S_dollar_dq, S_paren_dq, S_nonascii_text_dq,
		S_dollar_paren_dq,
		S_in_kanji, S_junk_tag} state;
#ifdef CALLERDATA
    void *			callerData;
#endif /* CALLERDATA */
    BOOL present[MAX_ATTRIBUTES];	/* Flags: attribute is present? */
    char * value[MAX_ATTRIBUTES];	/* malloc'd strings or NULL if none */

    BOOL			lead_exclamation;
    BOOL			first_dash;
    BOOL			end_comment;
    BOOL			doctype_bracket;
    BOOL			first_bracket;
    BOOL			second_bracket;

    char *			recover;
    int				recover_index;
    char *			include;
    int				include_index;
    char *			url;
    char *			csi;
    int				csi_index;
} ;


#define PUTC(ch) ((*context->actions->put_character)(context->target, ch))

extern BOOL historical_comments;
extern BOOL minimal_comments;
extern BOOL soft_dquotes;

/*	Handle Attribute
**	----------------
*/
/* PUBLIC CONST char * SGML_default = "";   ?? */

PRIVATE void handle_attribute_name ARGS2(
	HTStream *,	context,
	CONST char *,	s)
{

    HTTag * tag = context->current_tag;
    attr * attributes = tag->attributes;

    int high, low, i, diff;		/* Binary search for attribute name */
    for (low = 0, high = tag->number_of_attributes;
    	 high > low;
	 diff < 0 ? (low = i+1) : (high = i)) {
	i = (low + (high-low)/2);
	diff = strcasecomp(attributes[i].name, s);
	if (diff == 0) {		/* success: found it */
    	    context->current_attribute_number = i;
	    context->present[i] = YES;
	    FREE(context->value[i]);
	    return;
	} /* if */
	
    } /* for */
    
    if (TRACE)
	fprintf(stderr, "SGML: Unknown attribute %s for tag %s\n",
	    s, context->current_tag->name);
    context->current_attribute_number = INVALID;	/* Invalid */
}


/*	Handle attribute value
**	----------------------
*/
PRIVATE void handle_attribute_value ARGS2(
	HTStream *,	context,
	CONST char *,	s)
{
    if (context->current_attribute_number != INVALID) {
	StrAllocCopy(context->value[context->current_attribute_number], s);
    } else {
        if (TRACE)
	    fprintf(stderr, "SGML: Attribute value %s ignored\n", s);
    }
    context->current_attribute_number = INVALID; /* can't have two assignments! */
}


/*	Handle entity
**	-------------
**
** On entry,
**	s	contains the entity name zero terminated
** Bugs:
**	If the entity name is unknown, the terminator is treated as
**	a printable non-special character in all cases, even if it is '<'
** Bug-fix:
**	Modified SGML_character() so we only come here with terminator
**	as '\0' and check a FoundEntity flag. -- Foteos Macrides
*/

PRIVATE BOOL FoundEntity = FALSE;

PRIVATE void handle_entity ARGS2(
	HTStream *,	context,
	char,		term)
{
    CONST char ** entities = context->dtd->entity_names;
    CONST char *s = context->string->data;
    int high, low, i, diff;

    /*
    **  Use Lynx special characters directly for nbsp, ensp, emsp,
    **  thinsp, and shy so we go through the HTML_put_character()
    **  filters instead of using HTML_put_string(). - FM
    */
    if (!strcmp(s, "nbsp")) {
        PUTC(1);
	FoundEntity = TRUE;
	return;
    }
    if (!strcmp(s, "ensp") || !strcmp(s, "emsp") || !strcmp(s, "thinsp")) {
        PUTC(2);
	FoundEntity = TRUE;
	return;
    }
    if (!strcmp(s, "shy")) {
        PUTC(7);
	FoundEntity = TRUE;
	return;
    }

    /*
    **  Handle all other entities normally. - FM
    */
    FoundEntity = FALSE;
    for (low = 0, high = context->dtd->number_of_entities;
    	 high > low;
	 diff < 0 ? (low = i+1) : (high = i)) {  /* Binary serach */
	i = (low + (high-low)/2);
	diff = strcmp(entities[i], s);	/* Case sensitive! */
	if (diff == 0) {		/* success: found it */
	    (*context->actions->put_entity)(context->target, i);
	    FoundEntity = TRUE;
	    return;
	}
    }
    /*
    **  If entity string not found, display as text.
    */
    if (TRACE)
	fprintf(stderr, "SGML: Unknown entity %s\n", s); 
    PUTC('&');
    {
	CONST char *p;
	for (p = s; *p; p++) {
	    PUTC(*p);
	}
    }
    if (term != '\0')
        PUTC(term);
}


/*	Handle comment
**	--------------
*/
PRIVATE void handle_comment ARGS1(
	HTStream *,		context)
{
    CONST char *s = context->string->data;

    if (TRACE)
	fprintf(stderr, "SGML Comment:\n<%s>\n", s);

    if (context->csi == NULL &&
        strncmp(s, "!--#", 4) == 0 &&
        LYCheckForCSI(context->target, (char **)&context->url) == TRUE) {
	LYDoCSI(context->url, s, (char **)&context->csi);
    }

    return;
}


/*	Handle identifier
**	-----------------
*/
PRIVATE void handle_identifier ARGS1(
	HTStream *,		context)
{
    CONST char *s = context->string->data;

    if (TRACE)
	fprintf(stderr, "SGML Identifier\n<%s>\n", s);

    return;
}


/*	Handle doctype
**	--------------
*/
PRIVATE void handle_doctype ARGS1(
	HTStream *,		context)
{
    CONST char *s = context->string->data;

    if (TRACE)
	fprintf(stderr, "SGML Doctype\n<%s>\n", s);

    return;
}


/*	Handle marked
**	-------------
*/
PRIVATE void handle_marked ARGS1(
	HTStream *,		context)
{
    CONST char *s = context->string->data;

    if (TRACE)
	fprintf(stderr, "SGML Marked Section:\n<%s>\n", s);

    return;
}


/*	Handle sgmlent
**	--------------
*/
PRIVATE void handle_sgmlent ARGS1(
	HTStream *,		context)
{
    CONST char *s = context->string->data;

    if (TRACE)
	fprintf(stderr, "SGML Entity Declaration:\n<%s>\n", s);

    return;
}


/*	Handle sgmlent
**	--------------
*/
PRIVATE void handle_sgmlele ARGS1(
	HTStream *,		context)
{
    CONST char *s = context->string->data;

    if (TRACE)
	fprintf(stderr, "SGML Element Declaration:\n<%s>\n", s);

    return;
}


/*	Handle sgmlatt
**	--------------
*/
PRIVATE void handle_sgmlatt ARGS1(
	HTStream *,		context)
{
    CONST char *s = context->string->data;

    if (TRACE)
	fprintf(stderr, "SGML Attribute Declaration:\n<%s>\n", s);

    return;
}


/*	End element
**	-----------
*/
PRIVATE void end_element ARGS2(
	HTStream *,	context,
	HTTag *,	old_tag)
{
    if (TRACE)
        fprintf(stderr, "SGML: End </%s>\n", old_tag->name);
    if (old_tag->contents == SGML_EMPTY) {
        if (TRACE)
	    fprintf(stderr, "SGML: Illegal end tag </%s> found.\n",
	    		    old_tag->name);
	return;
    }
#ifdef WIND_DOWN_STACK
    while (context->element_stack) { /* Loop is error path only */
#else
    if (context->element_stack) { /* Substitute and remove one stack element */ 
#endif /* WIND_DOWN_STACK */
	HTElement * N = context->element_stack;
	HTTag * t = N->tag;
	
	if (old_tag != t) {		/* Mismatch: syntax error */
	    if (context->element_stack->next) {	/* This is not the last level */
		if (TRACE) fprintf(stderr,
	    	"SGML: Found </%s> when expecting </%s>. </%s> assumed.\n",
		    old_tag->name, t->name, t->name);
	    } else {			/* last level */
		if (TRACE) fprintf(stderr,
	            "SGML: Found </%s> when expecting </%s>. </%s> Ignored.\n",
		    old_tag->name, t->name, old_tag->name);
	        return;			/* Ignore */
	    }
	}
	
	context->element_stack = N->next;		/* Remove from stack */
	FREE(N);
	(*context->actions->end_element)(context->target,
		 t - context->dtd->tags, (char **)&context->include);
#ifdef WIND_DOWN_STACK
	if (old_tag == t)
	    return;  /* Correct sequence */
#else
	return;
#endif /* WIND_DOWN_STACK */
	
	/* Syntax error path only */
	
    }
    if (TRACE)
        fprintf(stderr, "SGML: Extra end tag </%s> found and ignored.\n",
			old_tag->name);
}


/*	Start a element
*/
PRIVATE void start_element ARGS1(
	HTStream *,	context)
{
    HTTag * new_tag = context->current_tag;
    
    if (TRACE)
        fprintf(stderr, "SGML: Start <%s>\n", new_tag->name);
    (*context->actions->start_element)(
    	context->target,
	new_tag - context->dtd->tags,
	context->present,
	(CONST char**) context->value,  /* coerce type for think c */
	(char **)&context->include);
    if (new_tag->contents != SGML_EMPTY) {		/* i.e. tag not empty */
	HTElement * N = (HTElement *)malloc(sizeof(HTElement));
        if (N == NULL)
	    outofmem(__FILE__, "start_element");
	N->next = context->element_stack;
	N->tag = new_tag;
	context->element_stack = N;
    }
}


/*		Find Tag in DTD tag list
**		------------------------
**
** On entry,
**	dtd	points to dtd structire including valid tag list
**	string	points to name of tag in question
**
** On exit,
**	returns:
**		NULL		tag not found
**		else		address of tag structure in dtd
*/
PUBLIC HTTag * SGMLFindTag ARGS2(
	CONST SGML_dtd*,	dtd,
	CONST char *,		string)
{
    int high, low, i, diff;
    for (low = 0, high=dtd->number_of_tags;
    	 high > low;
	 diff < 0 ? (low = i+1) : (high = i)) {  /* Binary serach */
	i = (low + (high-low)/2);
	diff = strcasecomp(dtd->tags[i].name, string);	/* Case insensitive */
	if (diff == 0) {		/* success: found it */
	    return &dtd->tags[i];
	}
    }
    return NULL;
}

/*________________________________________________________________________
**			Public Methods
*/


/*	Could check that we are back to bottom of stack! @@  */
/* 	Do check! - FM					     */
/*							     */
PUBLIC void SGML_free  ARGS1(
	HTStream *,	context)
{
    int i;
    HTElement * cur;
    HTElement * next;
    HTTag * t;

    /*
    **  Free the buffers. - FM
    */
    FREE(context->recover);
    FREE(context->url);
    FREE(context->csi);
    FREE(context->include);

    /*
    **  Wind down stack if any elements are open. - FM
    */
    while (context->element_stack) {
        cur = context->element_stack;
	t = cur->tag;
	context->element_stack = cur->next;	/* Remove from stack */
	FREE(cur);
	(*context->actions->end_element)(context->target,
		 t - context->dtd->tags, (char **)&context->include);
	FREE(context->include);
    }

    /*
    **  Finish off the target. - FM
    */
    (*context->actions->_free)(context->target);

    /*
    **  Free the strings and context structure. - FM
    */
    HTChunkFree(context->string);
    for (i = 0; i < MAX_ATTRIBUTES; i++) 
	FREE(context->value[i]);
    FREE(context);
}

PUBLIC void SGML_abort ARGS2(
	HTStream *,	context,
	HTError, 	e)
{
    int i;

    /*
    **  Abort the target. - FM
    */
    (*context->actions->_abort)(context->target, e);

    /*
    **  Free the buffers. - FM
    */
    FREE(context->recover);
    FREE(context->include);
    FREE(context->url);
    FREE(context->csi);

    /*
    **  Free the strings and context structure. - FM
    */
    HTChunkFree(context->string);
    for (i = 0; i < MAX_ATTRIBUTES; i++) 
	FREE(context->value[i]);
    FREE(context);
}


/*	Read and write user callback handle
**	-----------------------------------
**
**   The callbacks from the SGML parser have an SGML context parameter.
**   These calls allow the caller to associate his own context with a
**   particular SGML context.
*/

#ifdef CALLERDATA		  
PUBLIC void* SGML_callerData ARGS1(
	HTStream *,	context)
{
    return context->callerData;
}

PUBLIC void SGML_setCallerData ARGS2(
	HTStream *,	context,
	void*,		data)
{
    context->callerData = data;
}
#endif /* CALLERDATA */

PUBLIC void SGML_character ARGS2(
	HTStream *,	context,
	char,		c)
{
    CONST SGML_dtd *dtd	=	context->dtd;
    HTChunk	*string = 	context->string;
    CONST char * EntityName;
    extern int current_char_set;
    extern char *LYchar_set_names[];
    extern CONST char * HTMLGetEntityName PARAMS((int i));

top:
    /*
    **  Ignore low ISO 646 7-bit control characters
    **  if HTCJK is not set. - FM
    */
    if ((unsigned char)c < 32 &&
	c != 9 && c != 10 && c != 13 &&
	HTCJK == NOCJK)
        return;

    /*
    **  Ignore 127 if we don't have HTPassHighCtrlRaw
    **  or HTCJK set. - FM
    */
    if (c == 127 &&
        !(HTPassHighCtrlRaw || HTCJK != NOCJK))
        return;

    /*
    **  Ignore 8-bit control characters 128 - 159 if
    **  neither HTPassHighCtrlRaw nor HTCJK is set. - FM
    */
    if ((unsigned char)c > 127 && (unsigned char)c < 160 &&
	!(HTPassHighCtrlRaw || HTCJK != NOCJK))
        return;

    /*
    **  Handle character based on context->state.
    */
    switch(context->state) {

    case S_in_kanji:
	context->state = S_text;
	PUTC(c);
	break;

    case S_text:
	if (HTCJK != NOCJK && (c & 0200) != 0) {
	    /*
	    **  Setting up for Kanji multibyte handling (based on
	    **  Takuya ASADA's (asada@three-a.co.jp) CJK Lynx). - FM
	    */
	    context->state = S_in_kanji;
	    PUTC(c);
	    break;
	} else if (HTCJK != NOCJK && c == '\033') {
	    /*
	    **  Setting up for CJK escape sequence handling (based on
	    **  Takuya ASADA's (asada@three-a.co.jp) CJK Lynx). - FM
	    */
	    context->state = S_esc;
	    PUTC(c);
	    break;
	}
	if (c == '&' && (!context->element_stack ||
			 (context->element_stack->tag  &&
	    		  (context->element_stack->tag->contents ==
			  		SGML_MIXED ||
			   context->element_stack->tag->contents ==
			      		SGML_RCDATA)))) {
	    /*
	    **  Setting up for possible entity, without the leading '&'. - FM
	    */
	    string->size = 0;
	    context->state = S_ero;
	} else if (c == '<') {
	    /*
	    **  Setting up for possible tag. - FM
	    */
	    string->size = 0;
	    context->state = (context->element_stack &&
	    		context->element_stack->tag  &&
			context->element_stack->tag->contents == SGML_LITTERAL)
	      				 ?
	    		      S_litteral : S_tag;
	/*
	**  Convert 160 (nbsp) to Lynx special character if
	**  neither HTPassHighCtrlRaw nor HTCJK is set. - FM
	*/
	} else if ((unsigned char)c == 160 &&
		   !(HTPassHighCtrlRaw || HTCJK != NOCJK)) {
            PUTC(1);
	/*
	**  Convert 173 (shy) to Lynx special character if
	**  neither HTPassHighCtrlRaw nor HTCJK is set. - FM
	*/
	} else if ((unsigned char)c == 173 &&
		   !(HTPassHighCtrlRaw || HTCJK != NOCJK)) {
            PUTC(7);
	/*
	**  If it's any other (> 160) 8-bit chararcter, and
	**  we have not set HTPassEightBitRaw nor HTCJK, nor
	**  have the "ISO Latin 1" character set selected,
	**  back translate for our character set. - FM
	*/
	} else if ((unsigned char)c > 160 &&
		   !(HTPassEightBitRaw || HTCJK != NOCJK) &&
		   strncmp(LYchar_set_names[current_char_set],
		   	   "ISO Latin 1", 11)) {
	    int i;
	    int value;

	    string->size = 0;
	    value = (int)((unsigned char)c - 160);
	    EntityName = HTMLGetEntityName(value);
	    for (i = 0; EntityName[i]; i++)
	        HTChunkPutc(string, EntityName[i]);
	    HTChunkTerminate(string);
	    handle_entity(context, '\0');
	    string->size = 0;
	    if (!FoundEntity)
	        PUTC(';');
	/*
	**  If we get to here, pass the character. - FM
	*/
	} else {
	    PUTC(c);
	}
	break;

    /*	
    **  In litteral mode, waits only for specific end tag (for
    **  compatibility with old servers, and for Lynx). - FM
    */
    case S_litteral :
	HTChunkPutc(string, c);
	if (TOUPPER(c) != ((string->size == 1) ?
					   '/' :
			context->element_stack->tag->name[string->size-2])) {
	    int i;
	    
	    /*
	    **  If complete match, end litteral.
	    */
	    if ((c == '>') &&
	        (!context->element_stack->tag->name[string->size-2])) {
		end_element(context, context->element_stack->tag);
		string->size = 0;
		context->current_attribute_number = INVALID;
		context->state = S_text;
		break;
	    }
	    /*
	    **  If Mismatch: recover string.
	    */
	    PUTC('<');
	    for (i = 0; i < string->size; i++)	/* recover */
	       PUTC(string->data[i]);
	    string->size = 0;
	    context->state = S_text;	
	}
        break;

    /*
    **  Character reference (numeric entity) or named entity.
    */
    case S_ero:
   	if (c == '#') {
	    /*
	    **  Setting up for possible numeric entity.
	    */
	    context->state = S_cro;  /* &# is Char Ref Open */ 
	    break;
	}
	context->state = S_entity;   /* Fall through! */
	
    /*
    **  Handle possible named entity.
    */
    case S_entity:
	if ((unsigned char)c < 127 && isalnum((unsigned char)c)) {
	    /*
	    **  Accept valid ASCII character. - FM
	    */
	    HTChunkPutc(string, c);
	} else if (string->size == 0) {
	    /*
	    **  It was an ampersand that's just text, so output
	    **  the ampersand and recycle this character. - FM
	    */
	    PUTC('&');
	    context->state = S_text;
	    goto top;
	} else {
	    /*
	    **  Terminate entity name and try to handle it. - FM
	    */
	    HTChunkTerminate(string);
	    handle_entity(context, '\0');
	    string->size = 0;
	    context->state = S_text;
	    /*
	    **  Don't eat the terminator if we didn't find the
	    **  entity name and therefore sent the raw string
	    **  via handle_entity(), or if the terminator is
	    **  not the "standard" semi-colon for HTML. - FM
	    */
	    if (!FoundEntity || c != ';')
	        goto top;
	}
	break;

    /*
    **  Handle possible numeric entity.
    */
    case S_cro:
	if ((unsigned char)c < 127 && isdigit((unsigned char)c)) {
	    /*
	    **  Accept only valid ASCII digits. - FM
	    */
	    HTChunkPutc(string, c);	/* accumulate a character NUMBER */
	} else if (string->size == 0) {
	    /*
	    **  No digits following the "&#" so recover
	    **  them and recycle the character. - FM
	    */
	    PUTC('&');
	    PUTC('#');
	    context->state = S_text;
	    goto top;
	} else if ((unsigned char)c > 127 || isalnum((unsigned char)c)) {
	    /*
	    **  We have digit(s), but not a valid terminator,
	    **  so recover the "&#" and digit(s) and recycle
	    **  the character. - FM
	    */
	    int i;
	    PUTC('&');
	    PUTC('#');
	    for (i = 0; i < string->size; i++)	/* recover */
	       PUTC(string->data[i]);
	    string->size = 0;
	    context->state = S_text;
	    goto top;
	} else {
	    /*
	    **  Terminate the numeric entity and try to handle it. - FM
	    */
	    int value, i;
	    HTChunkTerminate(string);
	    if (sscanf(string->data, "%d", &value) == 1) {
	        if (value == 8482) {
		    /*
		    **  trade  Treat as reg. - FM
		    */
		    value = 174;
		}
	        /*
		** Show the numeric entity if the value:
		**  (1) Is greater than 255 (until we support Unicode).
		**  (2) Is less than 32, and not valid or we don't
		**	have HTCJK set.
		**  (3) Is 127 and we don't have HTPassHighCtrlRaw or
		**	HTCJK set.
		**  (4) Is 128 - 159 and we don't have HTPassHighCtrlNum
		**	set.
		** - FM
		*/
	        if ((value > 255) ||
		    (value < 32 &&
		     value != 9 && value != 10 && value != 13 &&
		     HTCJK == NOCJK) ||
		    (value == 127 &&
		     !(HTPassHighCtrlRaw || HTCJK != NOCJK)) ||
		    (value > 127 && value < 160 &&
		     !HTPassHighCtrlNum)) {
		    if (value == 8194 || value == 8195 || value == 8201) {
		        /*
			**  ensp, emsp or thinsp. - FM
			*/
			PUTC(2);
			break;
		    }
		    if (value == 8211 || value == 8212) {
		        /*
			**  ndash or mdash. - FM
			*/
			PUTC('-');
			break;
		    }
		    /*
		    **  Unhandled or llegal value.  Recover the "&#"
		    **  and digit(s), and recycle the terminator. - FM
		    */
		    PUTC('&');
		    PUTC('#');
		    string->size--;
		    for (i = 0; i < string->size; i++)	/* recover */
		        PUTC(string->data[i]);
		    string->size = 0;
		    context->state = S_text;
		    goto top;
		} else if (value == 160) {
		    /*
		    **  Use Lynx special character for 160 (nbsp). - FM
		    */
		    PUTC(1);
		} else if (value == 173) {
		    /*
		    **  Use Lynx special character for 173 (shy) - FM
		    */
		    PUTC(7);
		} else if (value < 161 || HTPassEightBitNum ||
			   !strncmp(LYchar_set_names[current_char_set],
			   	    "ISO Latin 1", 11)) {
		    /*
		    **  No conversion needed. - FM
		    */
	            PUTC(FROMASCII((char)value));
		} else {
		    /*
		    **  Convert and handle as named entity. - FM
		    */
		    value -= 160;
		    EntityName = HTMLGetEntityName(value);
		    if (EntityName && EntityName[0] != '\0') {
			string->size = 0;
			for (i = 0; EntityName[i]; i++)
			    HTChunkPutc(string, EntityName[i]);
			HTChunkTerminate(string);
			handle_entity(context, '\0');
			/*
			**  Add a semi-colon if something went wrong
			**  and handle_entity() sent the string. - FM
			*/
			if (!FoundEntity) {
			    PUTC(';');
			}
		    } else {
		        /*
			**  Our conversion failed, so recover the "&#"
			**  and digit(s), and recycle the terminator. - FM
			*/
			PUTC('&');
			PUTC('#');
			string->size--;
			for (i = 0; i < string->size; i++)	/* recover */
			    PUTC(string->data[i]);
			string->size = 0;
			context->state = S_text;
			goto top;
		    }
		}
		/*
		**  If we get to here, we succeeded.  Hoorah!!! - FM
		*/
		string->size = 0;
		context->state = S_text;
		/*
		**  Don't eat the terminator if it's not
		**  the "standard" semi-colon for HTML. - FM
		*/
		if (c != ';')
		    goto top;
	    } else {
	        /*
		**  Not an entity, and don't know why not, so add the
		**  terminator to the string, output the "&#", and
		**  process the string via the recover element. - FM
		*/
		string->size--;
		HTChunkPutc(string, c);
		HTChunkTerminate(string);
	        PUTC('&');
	        PUTC('#');
		if (context->recover == NULL) {
		    StrAllocCopy(context->recover, string->data);
		    context->recover_index = 0;
		} else {
		    StrAllocCat(context->recover, string->data);
		}
		string->size = 0;
		context->state = S_text;
		break;
	    }
	}
	break;

    /*
    **  Tag
    */	    
    case S_tag:					/* new tag */
	if ((unsigned char)c < 127 && isalnum((unsigned char)c)) {
	    /*
	    **  Add valid ASCII character. - FM
	    */
	    HTChunkPutc(string, c);
        } else if (c == '!' && !string->size) {	/* <! */
	    /*
	    **  Terminate and set up for possible comment,
	    **  identifier, declaration, or marked section. - FM
	    */
	    context->state = S_exclamation;
	    context->lead_exclamation = TRUE;
	    context->doctype_bracket = FALSE;
	    context->first_bracket = FALSE;
	    HTChunkPutc(string, c);
	    break;
        } else if (WHITE(c) && !string->size) {	/* <WHITE */
	    /*
	    **  Recover the '<' and WHITE character. - FM
	    */
	    context->state = S_text;
	    PUTC('<');
	    goto top;
	} else {				/* End of tag name */
	    /*
	    **  Try to handle tag. - FM
	    */
	    HTTag * t;
	    if (c == '/') {
		if (TRACE)
		    if (string->size!=0)
		        fprintf(stderr,"SGML: `<%s/' found!\n", string->data);
		context->state = S_end;
		break;
	    }
	    HTChunkTerminate(string) ;

	    t = SGMLFindTag(dtd, string->data);
	    if (!t) {
	        if (c == ':' && 0 == strcasecomp(string->data, "URL")) {
		    /*
		    **  Treat <URL: as text rather than a junk tag,
		    **  so we display it and the URL (Lynxism 8-). - FM
		    */
		    int i;
		    PUTC('<');
		    for (i = 0; i < 3; i++)	/* recover */
		        PUTC(string->data[i]);
		    PUTC(c);
		    if (TRACE)
		        fprintf(stderr, "SGML: Treating <%s%c as text\n",
		    			string->data, c);
		    string->size = 0;
		    context->state = S_text;	
		} else {
		    if (TRACE)
		        fprintf(stderr, "SGML: *** Unknown element %s\n",
		    			string->data);
		    context->state = (c == '>') ? S_text : S_junk_tag;
		}
		break;
	    }
	    context->current_tag = t;
	    
	    /* 
	    **  Clear out attributes.
	    */
	    {
	        int i;
	        for (i=0; i< context->current_tag->number_of_attributes; i++)
	    	    context->present[i] = NO;
	    }
	    string->size = 0;
	    context->current_attribute_number = INVALID;
	    
	    if (c == '>') {
		if (context->current_tag->name)
		    start_element(context);
		context->state = S_text;
	    } else {
	        context->state = S_tag_gap;
	    }
	}
	break;

    case S_exclamation:
        if (context->lead_exclamation && c == '-') {
	    /*
	    **  Set up for possible comment. - FM
	    */
	    context->lead_exclamation = FALSE;
	    context->first_dash = TRUE;
	    HTChunkPutc(string, c);
	    break;
	}
	if (context->lead_exclamation && c == '[') {
	    /*
	    **  Set up for possible marked section. - FM
	    */
	    context->lead_exclamation = FALSE;
	    context->first_bracket = TRUE;
	    context->second_bracket = FALSE;
	    HTChunkPutc(string, c);
	    context->state = S_marked;
	    break;
	}
	if (context->first_dash && c == '-') {
	    /*
	    **  Set up to handle comment. - FM
	    */
	    context->lead_exclamation = FALSE;
	    context->first_dash = FALSE;
	    context->end_comment = FALSE;
	    HTChunkPutc(string, c);
	    context->state = S_comment;
	    break;
	}
	context->lead_exclamation = FALSE;
	context->first_dash = FALSE;
	if (c == '>') {
	    /*
	    **  Try to handle identifier. - FM
	    */
	    HTChunkTerminate(string);
	    handle_identifier(context);
	    string->size = 0;
	    context->state = S_text;
	    break;
	}
	if (WHITE(c)) {
	    if (string->size == 8 &&
	        !strncasecomp(string->data, "!DOCTYPE", 8)) {
		/*
		**  Set up for DOCTYPE declaration. - FM
		*/
		HTChunkPutc(string, c);
		context->doctype_bracket = FALSE;
		context->state = S_doctype;
	        break;
	    }
	    if (string->size == 7 &&
	        !strncasecomp(string->data, "!ENTITY", 7)) {
		/*
		**  Set up for ENTITY declaration. - FM
		*/
		HTChunkPutc(string, c);
		context->first_dash = FALSE;
		context->end_comment = TRUE;
		context->state = S_sgmlent;
		break;
	    }
	    if (string->size == 8 &&
	        !strncasecomp(string->data, "!ELEMENT", 8)) {
		/*
		**  Set up for ELEMENT declaration. - FM
		*/
		HTChunkPutc(string, c);
		context->first_dash = FALSE;
		context->end_comment = TRUE;
		context->state = S_sgmlele;
		break;
	    }
	    if (string->size == 8 &&
	        !strncasecomp(string->data, "!ATTLIST", 8)) {
		/*
		**  Set up for ATTLIST declaration. - FM
		*/
		HTChunkPutc(string, c);
		context->first_dash = FALSE;
		context->end_comment = TRUE;
		context->state = S_sgmlatt;
		break;
	    }
	}
	HTChunkPutc(string, c);
	break;

    case S_comment:		/* Expecting comment. - FM */
        if (historical_comments) {
	    /*
	    **  Any '>' terminates. - FM
	    */
	    if (c == '>') {
	        HTChunkTerminate(string);
		handle_comment(context);
		string->size = 0;
		context->end_comment = FALSE;
		context->first_dash = FALSE;
		context->state = S_text;
		break;
	    }
	    HTChunkPutc(string, c);
	    break;
	}
        if (!context->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    context->first_dash = TRUE;
	    break;
	}
	if (context->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    context->first_dash = FALSE;
	    if (!context->end_comment)
	        context->end_comment = TRUE;
	    else if (!minimal_comments)
	        /*
		**  Validly treat '--' pairs as successive comments
		**  (for minimal, any "--WHITE>" terminates). - FM
		*/
	        context->end_comment = FALSE;
	    break;
	}
	if (context->end_comment && c == '>') {
	    /*
	    **  Terminate and handle the comment. - FM
	    */
	    HTChunkTerminate(string);
	    handle_comment(context);
	    string->size = 0;
	    context->end_comment = FALSE;
	    context->first_dash = FALSE;
	    context->state = S_text;
	    break;
	}
	context->first_dash = FALSE;
	if (context->end_comment && !isspace(c))
	    context->end_comment = FALSE;
	HTChunkPutc(string, c);
	break;

    case S_doctype:		/* Expecting DOCTYPE. - FM */
        if (context->doctype_bracket) {
	    HTChunkPutc(string, c);
	    if (c == ']')
	        context->doctype_bracket = FALSE;
	    break;
	}
	if (c == '[' && WHITE(string->data[string->size - 1])) {
	    HTChunkPutc(string, c);
	    context->doctype_bracket = TRUE;
	    break;
	}
	if (c == '>') {
	    HTChunkTerminate(string);
	    handle_doctype(context);
	    string->size = 0;
	    context->state = S_text;
	    break;
	}
	HTChunkPutc(string, c);
	break;

    case S_marked:		/* Expecting marked section. - FM */
        if (context->first_bracket && c == '[') {
	    HTChunkPutc(string, c);
	    context->first_bracket = FALSE;
	    context->second_bracket = TRUE;
	    break;
	}
	if (context->second_bracket && c == ']' &&
	    string->data[string->size - 1] == ']') {
	    HTChunkPutc(string, c);
	    context->second_bracket = FALSE;
	    break;
	}
	if (!context->second_bracket && c == '>') {
	    HTChunkTerminate(string);
	    handle_marked(context);
	    string->size = 0;
	    context->state = S_text;
	    break;
	}
	HTChunkPutc(string, c);
	break;

    case S_sgmlent:		/* Expecting ENTITY. - FM */
        if (!context->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    context->first_dash = TRUE;
	    break;
	}
	if (context->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    context->first_dash = FALSE;
	    if (!context->end_comment)
	        context->end_comment = TRUE;
	    else
	        context->end_comment = FALSE;
	    break;
	}
	if (context->end_comment && c == '>') {
	    HTChunkTerminate(string);
	    handle_sgmlent(context);
	    string->size = 0;
	    context->end_comment = FALSE;
	    context->first_dash = FALSE;
	    context->state = S_text;
	    break;
	}
	context->first_dash = FALSE;
	HTChunkPutc(string, c);
	break;

    case S_sgmlele:		/* Expecting ELEMENT. - FM */
        if (!context->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    context->first_dash = TRUE;
	    break;
	}
	if (context->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    context->first_dash = FALSE;
	    if (!context->end_comment)
	        context->end_comment = TRUE;
	    else
	        context->end_comment = FALSE;
	    break;
	}
	if (context->end_comment && c == '>') {
	    HTChunkTerminate(string);
	    handle_sgmlele(context);
	    string->size = 0;
	    context->end_comment = FALSE;
	    context->first_dash = FALSE;
	    context->state = S_text;
	    break;
	}
	context->first_dash = FALSE;
	HTChunkPutc(string, c);
	break;

    case S_sgmlatt:		/* Expecting ATTLIST. - FM */
        if (!context->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    context->first_dash = TRUE;
	    break;
	}
	if (context->first_dash && c == '-') {
	    HTChunkPutc(string, c);
	    context->first_dash = FALSE;
	    if (!context->end_comment)
	        context->end_comment = TRUE;
	    else
	        context->end_comment = FALSE;
	    break;
	}
	if (context->end_comment && c == '>') {
	    HTChunkTerminate(string);
	    handle_sgmlatt(context);
	    string->size = 0;
	    context->end_comment = FALSE;
	    context->first_dash = FALSE;
	    context->state = S_text;
	    break;
	}
	context->first_dash = FALSE;
	HTChunkPutc(string, c);
	break;

    case S_tag_gap:		/* Expecting attribute or '>' */
	if (WHITE(c))
	    break;		/* Gap between attributes */
	if (c == '>') {		/* End of tag */
	    if (context->current_tag->name)
	    	start_element(context);
	    context->state = S_text;
	    break;
	}
	HTChunkPutc(string, c);
	context->state = S_attr; /* Get attribute */
	break;
	
   				/* accumulating value */
    case S_attr:
	if (WHITE(c) || (c == '>') || (c == '=')) {	/* End of word */
	    HTChunkTerminate(string);
	    handle_attribute_name(context, string->data);
	    string->size = 0;
	    if (c == '>') {				/* End of tag */
		if (context->current_tag->name)
		    start_element(context);
		context->state = S_text;
		break;
	    }
	    context->state = (c == '=' ?  S_equals: S_attr_gap);
	} else {
	    HTChunkPutc(string, c);
	}
	break;
		
    case S_attr_gap:		/* Expecting attribute or '=' or '>' */
	if (WHITE(c))
	    break;		/* Gap after attribute */
	if (c == '>') {		/* End of tag */
	    if (context->current_tag->name)
	        start_element(context);
	    context->state = S_text;
	    break;
	} else if (c == '=') {
	    context->state = S_equals;
	    break;
	}
	HTChunkPutc(string, c);
	context->state = S_attr;		/* Get next attribute */
	break;
	
    case S_equals:		/* After attr = */ 
	if (WHITE(c))
	    break;		/* Before attribute value */
	if (c == '>') {		/* End of tag */
	    if (TRACE)
	        fprintf(stderr, "SGML: found = but no value\n");
	    if (context->current_tag->name)
	        start_element(context);
	    context->state = S_text;
	    break;
	    
	} else if (c == '\'') {
	    context->state = S_squoted;
	    break;

	} else if (c == '"') {
	    context->state = S_dquoted;
	    break;
	}
	HTChunkPutc(string, c);
	context->state = S_value;
	break;
	
    case S_value:
	if (WHITE(c) || (c == '>')) {		/* End of word */
	    HTChunkTerminate(string) ;
	    handle_attribute_value(context, string->data);
	    string->size = 0;
	    if (c == '>') {		/* End of tag */
		if (context->current_tag->name)
		    start_element(context);
		context->state = S_text;
		break;
	    }
	    else context->state = S_tag_gap;
	} else {
	    HTChunkPutc(string, c);
	}
	break;
		
    case S_squoted:		/* Quoted attribute value */
	if (c == '\'') {	/* End of attribute value */
	    HTChunkTerminate(string) ;
	    handle_attribute_value(context, string->data);
	    string->size = 0;
	    context->state = S_tag_gap;
	} else if (c == '\033') {
	    /*
	    **  Setting up for possible single quotes in CJK escape
	    **  sequences. - Takuya ASADA (asada@three-a.co.jp)
	    */
	    context->state = S_esc_sq;
	    HTChunkPutc(string, c);
	} else {
	    HTChunkPutc(string, c);
	}
	break;
	
    case S_dquoted:		/* Quoted attribute value */
	if (c == '"' ||		/* Valid end of attribute value */
	    (soft_dquotes &&	/*  If emulating old Netscape bug, treat '>' */
	     c == '>')) {	/*  as a co-terminator of dquoted and tag    */
	    HTChunkTerminate(string) ;
	    handle_attribute_value(context, string->data);
	    string->size = 0;
	    context->state = S_tag_gap;
	    if (c == '>')	/* We emulated the Netscape bug, so we go  */
	        goto top;	/* back and treat it as the tag terminator */
	} else if (c == '\033') {
	    /*
	    **  Setting up for possible double quotes in CJK escape
	    **  sequences. - Takuya ASADA (asada@three-a.co.jp)
	    */
	    context->state = S_esc_dq;
	    HTChunkPutc(string, c);
	} else {
	    HTChunkPutc(string, c);
	}
	break;
	
    case S_end:					/* </ */
	if ((unsigned char)c < 127 && isalnum((unsigned char)c))
	    HTChunkPutc(string, c);
	else {				/* End of end tag name */
	    HTTag * t=0;
	    HTChunkTerminate(string) ;
	    if (!*string->data)	{	/* Empty end tag */
		if (context->element_stack)
	            t = context->element_stack->tag;
	    } else {
		t = SGMLFindTag(dtd, string->data);
	    }
	    if (!t) {
		if (TRACE)
		    fprintf(stderr, "Unknown end tag </%s>\n", string->data); 
	    } else {
		BOOL tag_OK = (c == '>' || WHITE(c));
	        context->current_tag = t;
		if (tag_OK &&
		    (!strcasecomp(string->data, "DD") ||
		     !strcasecomp(string->data, "DT") ||
		     !strcasecomp(string->data, "LI") ||
		     !strcasecomp(string->data, "LH") ||
		     !strcasecomp(string->data, "TD") ||
		     !strcasecomp(string->data, "TH") ||
		     !strcasecomp(string->data, "TR") ||
		     !strcasecomp(string->data, "THEAD") ||
		     !strcasecomp(string->data, "TFOOT") ||
		     !strcasecomp(string->data, "TBODY") ||
		     !strcasecomp(string->data, "COLGROUP"))) {
		    /*
		    **  Don't treat these end tags as invalid,
		    **  nor act on them. - FM
		    */
		    if (TRACE)
		        fprintf(stderr,
				"SGML: `</%s%c' found!  Ignoring it.\n",
				string->data, c);
		    string->size = 0;
		    context->current_attribute_number = INVALID;
		    if (c != '>') {
			context->state = S_junk_tag;
		    } else {
			context->state = S_text;
		    }
		    break;
		} else if (tag_OK &&
			   !strcasecomp(string->data, "P")) {
		    /*
		    **  Treat a P end tag like a P start tag (Ugh,
		    **  what a hack! 8-). - FM
		    */
		    if (TRACE)
		        fprintf(stderr,
				"SGML: `</%s%c' found!  Treating as '<%s%c'.\n",
				string->data, c, string->data, c);
		    {
		        int i;
			for (i = 0;
			     i < context->current_tag->number_of_attributes;
			     i++) {
			    context->present[i] = NO;
			}
		    }
		    string->size = 0;
		    context->current_attribute_number = INVALID;
		    if (context->current_tag->name)
			start_element(context);
		    if (c != '>') {
			context->state = S_junk_tag;
		    } else {
			context->state = S_text;
		    }
		    break;
		} else if (tag_OK &&
			   !strcasecomp(string->data, "FONT")) {
		    /*
		    **  Treat a FONT end tag as a FONT start tag with
		    **  a dummy END attribute.  It's too likely to be
		    **  interdigited and mess up the parsing, so we've
		    **  declared FONT as SGML_EMPTY and will handle the
		    **  end tag in HTML_start_element. - FM
		    */
		    if (TRACE)
		        fprintf(stderr,
				"SGML: `</%s%c' found!  Treating as '<%s%c'.\n",
				string->data, c, string->data, c);
		    {
		        int i;
			for (i = 0;
			     i < context->current_tag->number_of_attributes;
			     i++) {
			    context->present[i] = (i == HTML_FONT_END);
			}
		    }
		    string->size = 0;
		    context->current_attribute_number = INVALID;
		    if (context->current_tag->name)
			start_element(context);
		    if (c != '>') {
			context->state = S_junk_tag;
		    } else {
			context->state = S_text;
		    }
		    break;
		} else {
		    /*
		    **  Handle all other end tags normally. - FM
		    */
		    end_element( context, context->current_tag);
		}
	    }

	    string->size = 0;
	    context->current_attribute_number = INVALID;
	    if (c != '>') {
		if (TRACE && !WHITE(c))
		    fprintf(stderr,"SGML: `</%s%c' found!\n", string->data, c);
		context->state = S_junk_tag;
	    } else {
	        context->state = S_text;
	    }
	}
	break;


    case S_esc:		/* Expecting '$'or '(' following CJK ESC. */
	if (c == '$') {
	    context->state = S_dollar;
	} else if (c == '(') {
	    context->state = S_paren;
	} else {
	    context->state = S_text;
	}
	PUTC(c);
	break;

    case S_dollar:	/* Expecting '@', 'B', 'A' or '(' after CJK "ESC$". */
	if (c == '@' || c == 'B' || c == 'A') {
	    context->state = S_nonascii_text;
	} else if (c == '(') {
	    context->state = S_dollar_paren;
	}
	PUTC(c);
	break;

    case S_dollar_paren: /* Expecting 'C' after CJK "ESC$(". */
	if (c == 'C') {
	    context->state = S_nonascii_text;
	} else {
	    context->state = S_text;
	}
	PUTC(c);
	break;

    case S_paren:	/* Expecting 'B', 'J', 'T' or 'I' after CJK "ESC(". */
	if (c == 'B' || c == 'J' || c == 'T') {
	    context->state = S_text;
	} else if (c == 'I') {
	    context->state = S_nonascii_text;
	} else {
	    context->state = S_text;
	}
	PUTC(c);
	break;

    case S_nonascii_text: /* Expecting CJK ESC after non-ASCII text. */
	if (c == '\033') {
	    context->state = S_esc;
	}
	PUTC(c);
	break;

    case S_esc_sq:	/* Expecting '$'or '(' following CJK ESC. */
	if (c == '$') {
	    context->state = S_dollar_sq;
	} else if (c == '(') {
	    context->state = S_paren_sq;
	} else {
	    context->state = S_squoted;
	}
	HTChunkPutc(string, c);
	break;

    case S_dollar_sq:	/* Expecting '@', 'B', 'A' or '(' after CJK "ESC$". */
	if (c == '@' || c == 'B' || c == 'A') {
	    context->state = S_nonascii_text_sq;
	} else if (c == '(') {
	    context->state = S_dollar_paren_sq;
	}
	HTChunkPutc(string, c);
	break;

    case S_dollar_paren_sq: /* Expecting 'C' after CJK "ESC$(". */
	if (c == 'C') {
	    context->state = S_nonascii_text_sq;
	} else {
	    context->state = S_squoted;
	}
	HTChunkPutc(string, c);
	break;

    case S_paren_sq:	/* Expecting 'B', 'J', 'T' or 'I' after CJK "ESC(". */
	if (c == 'B' || c == 'J' || c == 'T') {
	    context->state = S_squoted;
	} else if (c == 'I') {
	    context->state = S_nonascii_text_sq;
	} else {
	    context->state = S_squoted;
	}
	HTChunkPutc(string, c);
	break;

    case S_nonascii_text_sq: /* Expecting CJK ESC after non-ASCII text. */
	if (c == '\033') {
	    context->state = S_esc_sq;
	}
	HTChunkPutc(string, c);
	break;

    case S_esc_dq:		/* Expecting '$'or '(' following CJK ESC. */
	if (c == '$') {
	    context->state = S_dollar_dq;
	} else if (c == '(') {
	    context->state = S_paren_dq;
	} else {
	    context->state = S_dquoted;
	}
	HTChunkPutc(string, c);
	break;

    case S_dollar_dq:	/* Expecting '@', 'B', 'A' or '(' after CJK "ESC$". */
	if (c == '@' || c == 'B' || c == 'A') {
	    context->state = S_nonascii_text_dq;
	} else if (c == '(') {
	    context->state = S_dollar_paren_dq;
	}
	HTChunkPutc(string, c);
	break;

    case S_dollar_paren_dq: /* Expecting 'C' after CJK "ESC$(". */
	if (c == 'C') {
	    context->state = S_nonascii_text_dq;
	} else {
	    context->state = S_dquoted;
	}
	HTChunkPutc(string, c);
	break;

    case S_paren_dq:	/* Expecting 'B', 'J', 'T' or 'I' after CJK "ESC(". */
	if (c == 'B' || c == 'J' || c == 'T') {
	    context->state = S_dquoted;
	} else if (c == 'I') {
	    context->state = S_nonascii_text_dq;
	} else {
	    context->state = S_dquoted;
	}
	HTChunkPutc(string, c);
	break;

    case S_nonascii_text_dq: /* Expecting CJK ESC after non-ASCII text. */
	if (c == '\033') {
	    context->state = S_esc_dq;
	}
	HTChunkPutc(string, c);
	break;

    case S_junk_tag:
	if (c == '>') {
	    context->state = S_text;
	}
    } /* switch on context->state */

    /*
    **  Check whether we've added anything to the recover buffer. - FM
    */
    if (context->recover != NULL) {
        if (context->recover[context->recover_index] == '\0') {
	    FREE(context->recover);
	    context->recover_index = 0;
	} else {
	    c = context->recover[context->recover_index];
	    context->recover_index++;
	    goto top;
	}
    }

    /*
    **  Check whether an external function has added
    **  anything to the include buffer. - FM
    */
    if (context->include != NULL) {
        if (context->include[context->include_index] == '\0') {
	    FREE(context->include);
	    context->include_index = 0;
	} else {
	    c = context->include[context->include_index];
	    context->include_index++;
	    goto top;
	}
    }

    /*
    **  Check whether an external function has added
    **  anything to the csi buffer. - FM
    */
    if (context->csi != NULL) {
        if (context->csi[context->csi_index] == '\0') {
	    FREE(context->csi);
	    context->csi_index = 0;
	} else {
	    c = context->csi[context->csi_index];
	    context->csi_index++;
	    goto top;
	}
    }
}  /* SGML_character */


PUBLIC void SGML_string ARGS2(
	HTStream *,	context,
	CONST char*,	str)
{
    CONST char *p;
    for (p = str; *p; p++)
        SGML_character(context, *p);
}


PUBLIC void SGML_write ARGS3(
	HTStream *,	context,
	CONST char*,	str,
	int,		l)
{
    CONST char *p;
    CONST char *e = str+l;
    for (p = str; p < e; p++)
        SGML_character(context, *p);
}

/*_______________________________________________________________________
*/

/*	Structured Object Class
**	-----------------------
*/
PUBLIC CONST HTStreamClass SGMLParser = 
{		
	"SGMLParser",
	SGML_free,
	SGML_abort,
	SGML_character, 
	SGML_string,
	SGML_write,
}; 

/*	Create SGML Engine
**	------------------
**
** On entry,
**	dtd		represents the DTD, along with
**	actions		is the sink for the data as a set of routines.
**
*/

PUBLIC HTStream* SGML_new  ARGS2(
	CONST SGML_dtd *,	dtd,
	HTStructured *,		target)
{
    int i;
    HTStream* context = (HTStream *) malloc(sizeof(*context));
    if (!context)
        outofmem(__FILE__, "SGML_begin");

    context->isa = &SGMLParser;
    context->string = HTChunkCreate(128);	/* Grow by this much */
    context->dtd = dtd;
    context->target = target;
    context->actions = (HTStructuredClass*)(((HTStream*)target)->isa);
    					/* Ugh: no OO */
    context->state = S_text;
    context->element_stack = 0;			/* empty */
#ifdef CALLERDATA		  
    context->callerData = (void*) callerData;
#endif /* CALLERDATA */
    for (i = 0; i < MAX_ATTRIBUTES; i++)
        context->value[i] = 0;

    context->lead_exclamation = FALSE;
    context->first_dash = FALSE;
    context->end_comment = FALSE;
    context->doctype_bracket = FALSE;
    context->first_bracket = FALSE;
    context->second_bracket = FALSE;
    context->recover = NULL;

    context->recover_index = 0;
    context->include = NULL;
    context->include_index = 0;
    context->url = NULL;
    context->csi = NULL;
    context->csi_index = 0;

    return context;
}

/*		Asian character conversion functions
**		====================================
**
**	Added 24-Mar-96 by FM, based on:
**
////////////////////////////////////////////////////////////////////////
Copyright (c) 1993 Electrotechnical Laboratry (ETL)

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

PUBLIC int TREAT_SJIS = 1;

PUBLIC void JISx0201TO0208_EUC ARGS4(
	register unsigned char,		IHI,
	register unsigned char,		ILO,
	register unsigned char *,	OHI,
	register unsigned char *,	OLO)
{
    static char *table[] = { 
	"\xA1\xA3", "\xA1\xD6", "\xA1\xD7", "\xA1\xA2", "\xA1\xA6", "\xA5\xF2",
	"\xA5\xA1", "\xA5\xA3", "\xA5\xA5", "\xA5\xA7", "\xA5\xA9",
	"\xA5\xE3", "\xA5\xE5", "\xA5\xE7", "\xA5\xC3", "\xA1\xBC",
	"\xA5\xA2", "\xA5\xA4", "\xA5\xA6", "\xA5\xA8", "\xA5\xAA",
	"\xA5\xAB", "\xA5\xAD", "\xA5\xAF", "\xA5\xB1", "\xA5\xB3",
	"\xA5\xB5", "\xA5\xB7", "\xA5\xB9", "\xA5\xBB", "\xA5\xBD",
	"\xA5\xBF", "\xA5\xC1", "\xA5\xC4", "\xA5\xC6", "\xA5\xC8",
	"\xA5\xCA", "\xA5\xCB", "\xA5\xCC", "\xA5\xCD", "\xA5\xCE",
	"\xA5\xCF", "\xA5\xD2", "\xA5\xD5", "\xA5\xD8", "\xA5\xDB",
	"\xA5\xDE", "\xA5\xDF", "\xA5\xE0", "\xA5\xE1", "\xA5\xE2",
	"\xA5\xE4", "\xA5\xE6", "\xA5\xE8", "\xA5\xE9", "\xA5\xEA",
	"\xA5\xEB", "\xA5\xEC", "\xA5\xED", "\xA5\xEF", "\xA5\xF3",
	"\xA1\xAB", "\xA1\xAC"
    };

    if ((IHI == 0x8E) && (ILO >= 0xA1) && (ILO <= 0xDF)) {
	*OHI = table[ILO - 0xA1][0];
	*OLO = table[ILO - 0xA1][1];
    } else {
	*OHI = IHI;
	*OLO = ILO;
    }
}

PUBLIC unsigned char * SJIS_TO_JIS1 ARGS3(
	register unsigned char,		HI,
	register unsigned char,		LO,
	register unsigned char *,	JCODE)
{
    HI -= (HI <= 0x9F) ? 0x71 : 0xB1;
    HI = (HI << 1) + 1;
    if (0x7F < LO)
	LO--;
    if (0x9E <= LO) {
	LO -= 0x7D;
	HI++;
    } else {
        LO -= 0x1F;
    }
    JCODE[0] = HI;
    JCODE[1] = LO;
    return JCODE;
}

PUBLIC unsigned char * JIS_TO_SJIS1 ARGS3(
	register unsigned char,		HI,
	register unsigned char,		LO,
	register unsigned char *,	SJCODE)
{
    if (HI & 1)
	LO += 0x1F;
    else
	LO += 0x7D;
    if (0x7F <= LO)
	LO++;

    HI = ((HI - 0x21) >> 1) + 0x81;
    if (0x9F < HI)
	HI += 0x40;
    SJCODE[0] = HI;
    SJCODE[1] = LO;
    return SJCODE;
}

PUBLIC unsigned char * EUC_TO_SJIS1 ARGS3(
	unsigned char,			HI,
	unsigned char,			LO,
	register unsigned char *,	SJCODE)
{
    if (HI == 0x8E) JISx0201TO0208_EUC(HI, LO, &HI, &LO);
    JIS_TO_SJIS1(HI&0x7F, LO&0x7F, SJCODE);
    return SJCODE;
}

PUBLIC void JISx0201TO0208_SJIS ARGS3(
	register unsigned char,		I,
	register unsigned char *,	OHI,
	register unsigned char *,	OLO)
{
    unsigned char SJCODE[2];

    JISx0201TO0208_EUC('\x8E', I, OHI, OLO);
    JIS_TO_SJIS1(*OHI&0x7F, *OLO&0x7F, SJCODE);
    *OHI = SJCODE[0];
    *OLO = SJCODE[1];
}

PUBLIC unsigned char * SJIS_TO_EUC1 ARGS3(
	unsigned char,		HI,
	unsigned char,		LO,
	unsigned char *,	EUC)
{
    SJIS_TO_JIS1(HI, LO, EUC);
    EUC[0] |= 0x80;
    EUC[1] |= 0x80;
    return EUC;
}

PUBLIC unsigned char * SJIS_TO_EUC ARGS2(
	unsigned char *,	src,
	unsigned char *,	dst)
{
    register unsigned char hi, lo, *sp, *dp;
    register int in_sjis = 0;

    for (sp = src, dp = dst; (0 != (hi = sp[0]));) {
	lo = sp[1];
	if (TREAT_SJIS && IS_SJIS(hi, lo, in_sjis)) {
	    SJIS_TO_JIS1(hi,lo,dp);
	    dp[0] |= 0x80;
	    dp[1] |= 0x80;
	    dp += 2;
	    sp += 2;
	} else {
	    *dp++ = *sp++;
	}
    }
    *dp = 0;
    return dst;
}

PUBLIC unsigned char * EUC_TO_SJIS ARGS2(
	unsigned char *,	src,
	unsigned char *,	dst)
{
    register unsigned char *sp, *dp;

    for (sp = src, dp = dst; *sp;) {
	if (*sp & 0x80) {
	    if (sp[1] && (sp[1] & 0x80)) {
		JIS_TO_SJIS1(sp[0]&0x7F, sp[1]&0x7F, dp);
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

PUBLIC unsigned char * EUC_TO_JIS ARGS4(
	unsigned char *,	src,
	unsigned char *,	dst,
	CONST char *,		toK,
	CONST char *,		toA)
{
    register unsigned char kana_mode = 0;
    register unsigned char cch;
    register unsigned char *sp = src;
    register unsigned char *dp = dst;
    register int i;

    while (0 != (cch = *sp++)) {
	if (cch & 0x80) {
	    if (!kana_mode) {
		kana_mode = ~kana_mode;
		for (i = 0; toK[i]; i++) {
		    *dp++ = (unsigned char)toK[i];
		}
	    }
	    if (*sp & 0x80) {
		*dp++ = cch & ~0x80;
		*dp++ = *sp++ & ~0x80;
	    }
	} else {
	    if (kana_mode) {
		kana_mode = ~kana_mode;
		for (i = 0; toA[i]; i++) {
		    *dp++ = (unsigned char)toA[i];
		    *dp = '\0';
		}
	    }
	    *dp++ = cch;
	}
    }
    if (kana_mode) {
	for (i = 0; toA[i]; i++) {
	    *dp++ = (unsigned char)toA[i];
	}
    }

    if (dp)
        *dp = 0;
    return dst;
}

PUBLIC unsigned char * TO_EUC ARGS2(
	unsigned char *,	jis,
	unsigned char *,	euc)
{
    register unsigned char *s, *d, c, jis_stat;
    register to1B, to2B;
    register int in_sjis = 0;

    s = jis;
    d = euc;
    jis_stat = 0;
    to2B = TO_2BCODE;
    to1B = TO_1BCODE;

    while (0 != (c = *s++)) {
	if (c == ESC) {
	    if (*s == to2B) {
		if ((s[1] == 'B') || (s[1] == '@') || (s[1] == 'A')) {
		    jis_stat = 0x80;
		    s += 2;
		    continue;
		} else if ((s[1] == '(') && s[2] && (s[2] == 'C')) {
		    jis_stat = 0x80;
		    s += 3;
		    continue;
		}
	    } else {
		if (*s == to1B) {
		    if ((s[1]=='B') || (s[1]=='J') ||
			(s[1]=='H') || (s[1]=='T')) {
			jis_stat = 0;
			s += 2;
			continue;
		    }
		}
	    }
	}
	if (IS_SJIS(c,*s,in_sjis)) {
	    SJIS_TO_EUC1(c, *s, d);
	    d += 2;
	    s++;
	} else {
	    if (jis_stat && (0x20 < c)) {
		*d++ = jis_stat | c;
	    } else {
	        *d++ = c;
	    }
	}
    }
    *d = 0;
    return euc;
}

PUBLIC void TO_SJIS ARGS2(
	unsigned char *,	any,
	unsigned char *,	sjis)
{
    unsigned char *euc;

    if (!any || !sjis)
       return;

    euc = (unsigned char*)malloc(strlen((CONST char *)any)+1);
    if (euc == NULL)
	outofmem(__FILE__, "TO_SJIS");

    TO_EUC(any, euc);
    EUC_TO_SJIS(euc, sjis);
    FREE(euc);
}

PUBLIC void TO_JIS ARGS2(
	unsigned char *,	any,
	unsigned char *,	jis)
{
    unsigned char *euc;

    if (!any || !jis)
       return;

    euc = (unsigned char*)malloc(strlen((CONST char *)any)+1);
    if (euc == NULL)
	outofmem(__FILE__, "TO_JIS");

    TO_EUC(any, euc);
    EUC_TO_JIS(euc, jis, TO_KANJI, TO_ASCII);
    FREE(euc);
}
