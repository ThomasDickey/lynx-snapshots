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

/* Remove the following to disable the experimental HTML DTD parsing.
   Currently only used in this source file. - kw */

#define EXTENDED_HTMLDTD


#include "HTUtils.h"
#include "tcp.h"		/* For FROMASCII */

#include "SGML.h"
#include "HTMLDTD.h"
#include "HTCJK.h"
#ifdef EXP_CHARTRANS
#include "UCMap.h"
#include "UCDefs.h"
#include "UCAux.h"
#endif


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

extern BOOLEAN LYCheckForCSI PARAMS((HTParentAnchor *anchor, char **url));
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
		S_ero, S_cro, S_incro,
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
    BOOL			isHex;

#ifdef EXP_CHARTRANS
    HTParentAnchor *		node_anchor;
    LYUCcharset	*		UCI;		/* anchor UCInfo	    */
    int				in_char_set;	/* charset we are fed	    */
    LYUCcharset	*		htmlUCI;	/* anchor UCInfo for target */
    int				html_char_set;	/* feed it to target stream */
    char			utf_count;
    long			utf_char;
    char 			utf_buf[7];
    char *			utf_buf_p;
    UCTransParams		T;
#endif /* EXP_CHARTRANS */

    char *			recover;
    int				recover_index;
    char *			include;
    int				include_index;
    char *			url;
    char *			csi;
    int				csi_index;
} ;

#ifdef EXP_CHARTRANS

PRIVATE void set_chartrans_handling ARGS3(
	HTStream *,	context,
	HTParentAnchor *, anchor,
	int,	chndl)
{
    extern int current_char_set;

    if (chndl < 0) {
	chndl = HTAnchor_getUCLYhndl(anchor, UCT_STAGE_STRUCTURED);
	if (chndl < 0)
	    chndl = HTAnchor_getUCLYhndl(anchor, UCT_STAGE_HTEXT);
	if (chndl < 0)
	    chndl = current_char_set;
	HTAnchor_setUCInfoStage(anchor, chndl, UCT_STAGE_HTEXT,
			    UCT_SETBY_DEFAULT);
	HTAnchor_setUCInfoStage(anchor, chndl, UCT_STAGE_STRUCTURED,
			    UCT_SETBY_DEFAULT);
	context->htmlUCI = HTAnchor_getUCInfoStage(anchor,
						   UCT_STAGE_STRUCTURED);
	context->html_char_set = HTAnchor_getUCLYhndl(context->node_anchor,
						      UCT_STAGE_STRUCTURED);
    }
    UCSetTransParams(&context->T,
		     context->in_char_set, context->UCI,
		     context->html_char_set, context->htmlUCI);
}

PRIVATE void change_chartrans_handling ARGS1(
	HTStream *,	context)
{
    int new_LYhndl = HTAnchor_getUCLYhndl(context->node_anchor,
					  UCT_STAGE_PARSER);
    if (new_LYhndl != context->in_char_set &&
	new_LYhndl >= 0) {
	/*
	 *  Something changed. but ignore if a META wants an unknown charset.
	 */
	LYUCcharset * new_UCI = HTAnchor_getUCInfoStage(context->node_anchor,
							UCT_STAGE_PARSER);
	if (new_UCI) {
            LYUCcharset * next_UCI = HTAnchor_getUCInfoStage(
				    context->node_anchor, UCT_STAGE_STRUCTURED
							    );
	    int next_LYhndl = HTAnchor_getUCLYhndl(
				    context->node_anchor, UCT_STAGE_STRUCTURED
						  );
	    context->UCI = new_UCI;
	    context->in_char_set = new_LYhndl;
	    context->htmlUCI = next_UCI;
	    context->html_char_set = next_LYhndl;
	    set_chartrans_handling(context,
				   context->node_anchor, next_LYhndl);
	}
    }
}
#endif /* EXP_CHARTRANS */


#define PUTC(ch) ((*context->actions->put_character)(context->target, ch))
#ifdef EXP_CHARTRANS
#define PUTUTF8(code) (UCPutUtf8_charstring((HTStream *)context->target, \
		      (putc_func_t*)(context->actions->put_character), code))
#endif

extern BOOL historical_comments;
extern BOOL minimal_comments;
extern BOOL soft_dquotes;

#ifdef USE_COLOR_STYLE
#include "AttrList.h"
extern char class_string[TEMPSTRINGSIZE];
int current_is_class=0;
#endif

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
#ifdef USE_COLOR_STYLE
	current_is_class=(!strcasecomp("class", s));
	if (TRACE)
		fprintf(stderr, "SGML: found attribute %s, %d\n", s, current_is_class);
#endif
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
#ifdef USE_COLOR_STYLE
        if (current_is_class)
        {
                strncpy (class_string, s, TEMPSTRINGSIZE);
                if (TRACE)
                fprintf(stderr, "SGML: class is '%s'\n", s);
        }
        else
        {
                if (TRACE)
                fprintf(stderr, "SGML: attribute value is '%s'\n", s);
        }
#endif
    } else {
        if (TRACE)
	    fprintf(stderr, "SGML: Attribute value %s ignored\n", s);
    }
    context->current_attribute_number = INVALID; /* can't have two assignments! */
}

#ifdef EXP_CHARTRANS
/* translate some Unicodes to Lynx special codes and output them. */
PRIVATE BOOL put_special_unicodes ARGS2(
	HTStream *,	context,
	long, code)
{
    if (code == 160) {
	PUTC(HT_NON_BREAK_SPACE);
    } else  if (code==173) {
	PUTC(LY_SOFT_HYPHEN);
    } else if (code == 8194 || code == 8195 || code == 8201) {
		        /*
			**  ensp, emsp or thinsp.
			*/
	PUTC(HT_EM_SPACE);
    } else if (code == 8211 || code == 8212) {
		        /*
			**  ndash or mdash.
			*/
			PUTC('-');
    } else {
	/*
	**  Return NO if nothing done.
	*/
	return NO;
    }
    /*
    **  We have handled it.
    */
    return YES;
}
#endif

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
**
** Modified more (for use with CHARTRANS):
*/

#ifdef EXP_CHARTRANS
PRIVATE char replace_buf [61];        /* buffer for replacement strings */
#endif

PRIVATE BOOL FoundEntity = FALSE;

PRIVATE void handle_entity ARGS2(
	HTStream *,	context,
	char,		term)
{
    CONST char ** entities = context->dtd->entity_names;
#ifdef EXP_CHARTRANS
    CONST UC_entity_info * extra_entities = context->dtd->extra_entity_info;
    extern int current_char_set;
    int rc;
#endif
    CONST char *s = context->string->data;
    int high, low, i, diff;

    /*
    **  Use Lynx special characters directly for nbsp, ensp, emsp,
    **  thinsp, and shy so we go through the HTML_put_character()
    **  filters instead of using HTML_put_string(). - FM
    */
    if (!strcmp(s, "nbsp")) {
        PUTC(HT_NON_BREAK_SPACE);
	FoundEntity = TRUE;
	return;
    }
    if (!strcmp(s, "ensp") || !strcmp(s, "emsp") || !strcmp(s, "thinsp")) {
        PUTC(HT_EM_SPACE);
	FoundEntity = TRUE;
	return;
    }
    if (!strcmp(s, "shy")) {
        PUTC(LY_SOFT_HYPHEN);
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
#ifdef EXP_CHARTRANS
    /* repeat for extra entities if not found... hack... -kw */
    if (TRACE)
	fprintf(stderr,
		"SGML: Unknown entity %s so far, checking extra...\n", s); 
    for (low = 0, high = context->dtd->number_of_extra_entities;
    	 high > low;
	 diff < 0 ? (low = i+1) : (high = i)) {  /* Binary serach */
	i = (low + (high-low)/2);
	diff = strcmp(extra_entities[i].name, s);	/* Case sensitive! */
	if (diff==0) {			/* success: found it */
	  if (put_special_unicodes(context, extra_entities[i].code)) {  
	    FoundEntity = TRUE;
	    return;
	  } else if (context->T.output_utf8 &&
		     PUTUTF8(extra_entities[i].code)) {
	    FoundEntity = TRUE;
	    return;
	  }
	    if ((rc = UCTransUniChar(extra_entities[i].code,
				     current_char_set)) > 0) {
		/*
		 *  Could do further checks here... - KW
		 */
	    PUTC(rc);
	    FoundEntity = TRUE;
	    return;
	    } else if ((rc == -4) &&
		       /* Not found; look for replacement string */
		     (rc = UCTransUniCharStr(replace_buf,60,
					     extra_entities[i].code,
					     current_char_set, 0)   >= 0 ) ) { 
	    CONST char *p;
	    for (p=replace_buf; *p; p++)
	      PUTC(*p);
	    FoundEntity = TRUE;
	    return;
	  } 
	  rc = (*context->actions->put_entity)(context->target,
					  i+context->dtd->number_of_entities);
	  if (rc != HT_CANNOT_TRANSLATE) {
	      FoundEntity = TRUE;
	      return;
	  }
	}
    }
#endif
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
        LYCheckForCSI(context->node_anchor, (char **)&context->url) == TRUE) {
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


PRIVATE BOOL element_valid_within ARGS3(
    HTTag *, 	new_tag,
    HTTag *,	stacked_tag,
    BOOL,	direct)
{
    TagClass usecontains, usecontained;
    if (!stacked_tag || !new_tag)
	return YES;
    usecontains = (direct ? stacked_tag->contains : stacked_tag->icontains);
    usecontained = (direct ? new_tag->contained : new_tag->icontained);
    if (new_tag == stacked_tag)
	return ((Tgc_same & usecontains) &&
		(Tgc_same & usecontained));
    else
	return ((new_tag->tagclass & usecontains) &&
		(stacked_tag->tagclass & usecontained));
}

#ifdef EXTENDED_HTMLDTD

extern BOOL New_DTD;

typedef enum {
    close_NO	= 0,
    close_error = 1,
    close_valid	= 2,
} canclose_t;

PRIVATE canclose_t can_close ARGS2(
    HTTag *, 	new_tag,
    HTTag *,	stacked_tag)
{
    if (!stacked_tag)
	return close_NO;
    if (stacked_tag->flags & Tgf_endO)
	return close_valid;
    else if (new_tag == stacked_tag)
	return ((Tgc_same & new_tag->canclose) ? close_error : close_NO);
    else
	return ((stacked_tag->tagclass & new_tag->canclose) ?
		close_error : close_NO);
}
PRIVATE void do_close_stacked ARGS1(
    HTStream *,	context)
{
    HTElement * stacked = context->element_stack;
    if (!stacked)
	return;			/* stack was empty */
    (*context->actions->end_element)(
        context->target,
        stacked->tag - context->dtd->tags,
        (char **)&context->include);
    context->element_stack = stacked->next;
    FREE(stacked);
}
PRIVATE int is_on_stack ARGS2(
	HTStream *,	context,
	HTTag *,	old_tag)
{
    HTElement * stacked = context->element_stack;
    int i = 1;
    for (; stacked; stacked = stacked->next, i++) {
	if (stacked->tag == old_tag)
	    return i;
    }
    return 0;
}
#endif /* EXTENDED_HTMLDTD */

/*	End element
**	-----------
*/
PRIVATE void end_element ARGS2(
	HTStream *,	context,
	HTTag *,	old_tag)
{
#ifdef EXTENDED_HTMLDTD

    BOOL extra_action_taken = NO;
    canclose_t canclose_check = close_valid;
    int stackpos = is_on_stack(context, old_tag);

    if (New_DTD) {
	while (canclose_check != close_NO &&
	       context->element_stack &&
	       (stackpos > 1 || (!extra_action_taken && stackpos == 0))) {
	    canclose_check = can_close(old_tag, context->element_stack->tag);
	    if (canclose_check != close_NO) {
		if (TRACE)
		    fprintf(stderr, "SGML: End </%s> \t<- %s end </%s>\n",
			    context->element_stack->tag->name,
			    canclose_check == close_valid ? "supplied," : "forced by",
			    old_tag->name);
		do_close_stacked(context);
		extra_action_taken = YES;
		stackpos = is_on_stack(context, old_tag);
	    } else {
		if (TRACE)
		    fprintf(stderr, "SGML: Still open %s \t<- invalid end </%s>\n",
			    context->element_stack->tag->name,
			    old_tag->name);
		return;
	    }
	}

	if (stackpos == 0 && old_tag->contents != SGML_EMPTY) {
	    if (TRACE)
		fprintf(stderr, "SGML: Still open %s, no open %s for </%s>\n",
			context->element_stack ?
			context->element_stack->tag->name : "none",
			old_tag->name,
			old_tag->name);
	    return;
	}
	if (stackpos > 1) {
	    if (TRACE)
		fprintf(stderr, "SGML: Nesting <%s>...<%s> \t<- invalid end </%s>\n",
			old_tag->name,
			context->element_stack->tag->name,
			old_tag->name);
	    return;
	}
    }
    /* Now let the old code deal with the rest... - kw */

#endif /* EXTENDED_HTMLDTD */

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

#ifdef EXTENDED_HTMLDTD

    BOOL valid = YES;
    BOOL direct_container = YES;
    BOOL extra_action_taken = NO;
    canclose_t canclose_check = close_valid;

    if (New_DTD) {
	while (context->element_stack &&
	       (canclose_check == close_valid ||
		(canclose_check == close_error &&
		 new_tag == context->element_stack->tag)) &&
	       !(valid = element_valid_within(new_tag, context->element_stack->tag,
					      direct_container))) {
	    canclose_check = can_close(new_tag, context->element_stack->tag);
	    if (canclose_check != close_NO) {
		if (TRACE)
		    fprintf(stderr, "SGML: End </%s> \t<- %s start <%s>\n",
			    context->element_stack->tag->name,
			    canclose_check == close_valid ? "supplied," : "forced by",
			    new_tag->name);
		do_close_stacked(context);
		extra_action_taken = YES;
		if (canclose_check  == close_error)
		    direct_container = NO;
	    } else {
		if (TRACE)
		    fprintf(stderr, "SGML: Still open %s \t<- invalid start <%s>\n",
			    context->element_stack->tag->name,
			    new_tag->name);
	    }
	}

	if (context->element_stack && !extra_action_taken &&
	    canclose_check == close_NO && !valid && (new_tag->flags & Tgf_mafse)) {
	    BOOL has_attributes = NO;
	    int i = 0;
	    for (; i< new_tag->number_of_attributes && !has_attributes; i++)
		has_attributes = context->present[i];
	    if (!has_attributes) {
		if (TRACE)
		    fprintf(stderr, "SGML: Still open %s, converting invalid <%s> to </%s>\n",
			    context->element_stack->tag->name,
			    new_tag->name,
			    new_tag->name);
		end_element(context, new_tag);
		return;
	    }
	}
    
	if (context->element_stack &&
	    canclose_check == close_error && !(valid =
					       element_valid_within(
						   new_tag,
						   context->element_stack->tag,
						   direct_container))) {
	    if (TRACE)
		fprintf(stderr, "SGML: Still open %s \t<- invalid start <%s>\n",
			context->element_stack->tag->name,
			new_tag->name);
	}
    }
    /* fall through to the old code - kw */

#endif /* EXTENDED_HTMLDTD */

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
#ifdef EXP_CHARTRANS
    else {			/* check for result of META tag. */
	change_chartrans_handling(context);
    }
#endif /* EXP_CHARTRANS */
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
    HTElement * cur;

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
    **  Free stack memory if any elements were left open. - KW
    */
    while (context->element_stack) {
        cur = context->element_stack;
	context->element_stack = cur->next;	/* Remove from stack */
	FREE(cur);
    }

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
	char,		c_in)
{
    CONST SGML_dtd *dtd	=	context->dtd;
    HTChunk	*string = 	context->string;
    CONST char * EntityName;
    extern int current_char_set;
    extern CONST char *LYchar_set_names[];
    extern CONST char * HTMLGetEntityName PARAMS((int i));

#ifdef EXP_CHARTRANS
    extern int LYlowest_eightbit[];
    char * p;
    BOOLEAN chk;	/* Helps (?) walk through all the else ifs... */
    long clong, uck;	/* Enough bits for UCS4 ... */
    char c;
    char saved_char_in = '\0';
    /*
    **  Now some fun with the preprocessor...
    **  use copies for c an unsign_c == clong, so that we
    **  can revert back to the unchanged c_in.
    */
#define unsign_c clong

#else
#define c c_in
#define unsign_c (unsigned char)c
#endif    

#ifdef EXP_CHARTRANS
    c = c_in;
    clong = (unsigned char)c;	/* a.k.a. unsign_c */

    if (context->T.decode_utf8) {
	/*
	**  Combine UTF-8 into Unicode.
	**  Incomplete characters silently ignored.
	**  From Linux kernel's console.c.
	*/
	if((unsigned char)c > 0x7f) {
	    if (context->utf_count > 0 && (c & 0xc0) == 0x80) {
		context->utf_char = (context->utf_char << 6) | (c & 0x3f);
		context->utf_count--;
		*(context->utf_buf_p++) = c;
		if (context->utf_count == 0) {
		    *(context->utf_buf_p) = '\0';
		    clong = context->utf_char;
		    if (clong < 256) {
			c = (char)clong;
		    }
		    goto top1;
		} else {
		    /*
		    **  Wait for more.
		    */
		    return;
		}
	    } else {
		context->utf_buf_p = context->utf_buf;
		*(context->utf_buf_p++) = c;
		if ((c & 0xe0) == 0xc0) {
		    context->utf_count = 1;
		    context->utf_char = (c & 0x1f);
		} else if ((c & 0xf0) == 0xe0) {
		    context->utf_count = 2;
		    context->utf_char = (c & 0x0f);
		} else if ((c & 0xf8) == 0xf0) {
		    context->utf_count = 3;
		    context->utf_char = (c & 0x07);
		} else if ((c & 0xfc) == 0xf8) {
		    context->utf_count = 4;
		    context->utf_char = (c & 0x03);
		} else if ((c & 0xfe) == 0xfc) {
		    context->utf_count = 5;
		    context->utf_char = (c & 0x01);
		} else {
		    /*
		    **  Garbage.
		    */
		    context->utf_count = 0;
		    context->utf_buf_p = context->utf_buf;
		    *(context->utf_buf_p) = '\0';
		}
		/*
		**  Wait for more.
		*/
		return;
	    }
	} else {
	    /*
	    **  Got an ASCII char.
	    */
	    context->utf_count = 0;
	    context->utf_buf_p = context->utf_buf;
	    *(context->utf_buf_p) = '\0';
		    /*  goto top;  */
	}
    }

    if (context->T.strip_raw_char_in)
	saved_char_in = c;

    if (context->T.trans_to_uni &&
	(unsign_c >= 127 ||
	    (unsign_c < 32 && unsign_c != 0 &&
	     context->T.trans_C0_to_uni))) {
	clong = UCTransToUni(c, context->in_char_set);
	if (clong > 0) {
	    saved_char_in = c;
	    if (clong < 256) {
		c = (char)clong;
	    }
	}
	goto top1;
    } else if (unsign_c < 32 && unsign_c != 0 &&
	       context->T.trans_C0_to_uni) {
	/*
	**  This else if may be too ugly to keep. - KW
	*/
	if (context->T.trans_from_uni &&
	    (((clong = UCTransToUni(c, context->in_char_set)) >= 32) ||
	     (context->T.transp &&
	      (clong = UCTransToUni(c, context->in_char_set)) > 0))) {
	    saved_char_in = c;
	    if (clong < 256) {
		c = (char)clong;
	    }
	    goto top1;
	} else {
	    uck = -1;
	    if (context->T.transp) {
		uck = UCTransCharStr(replace_buf, 60, c,
				     context->in_char_set,
				     context->in_char_set, NO);
	    }
	    if (!context->T.transp || uck < 0) {
		uck = UCTransCharStr(replace_buf, 60, c,
				     context->in_char_set,
				     context->html_char_set, YES);
	    }
	    if (uck == 0) {
		return;
	    } else if (uck < 0) {
		goto top0a;
	    }
	    c = replace_buf[0];
	    if (c && replace_buf[1]) {
		if (context->state == S_text) {
		    for (p=replace_buf; *p; p++)
			PUTC(*p);
		    return;
		}
		StrAllocCat(context->recover, replace_buf + 1);
	    }
	    goto top0a;
	} /*  Next line end of ugly stuff for C0. - KW */
    } else {
	goto top0a;
    }

    /* At this point we have either unsign_c a.k.a. clong in Unicode
       (and c in latin1 if clong is in the latin1 range),
       or unsign_c and c will have to be passed raw. */

#endif /* EXP_CHARTRANS */


top:
#ifdef EXP_CHARTRANS
    saved_char_in = '\0';
top0a:
    *(context->utf_buf) = '\0';
    clong = (unsigned char)c;
#endif
top1:
    /*
    **  Ignore low ISO 646 7-bit control characters
    **  if HTCJK is not set. - FM
    */
    if (unsign_c < 32 &&
	c != 9 && c != 10 && c != 13 &&
	HTCJK == NOCJK)
        return;

    /*
    **  Ignore 127 if we don't have HTPassHighCtrlRaw
    **  or HTCJK set. - FM
    */
#ifndef EXP_CHARTRANS
#define PASSHICTRL HTPassHighCtrlRaw
#else
#define PASSHICTRL (context->T.transp || unsign_c >= LYlowest_eightbit[context->in_char_set])
#endif /* EXP_CHARTRANS */
    if (c == 127 &&
        !(PASSHICTRL || HTCJK != NOCJK))
        return;

    /*
    **  Ignore 8-bit control characters 128 - 159 if
    **  neither HTPassHighCtrlRaw nor HTCJK is set. - FM
    */
    if (unsign_c > 127 && unsign_c < 160 &&
	!(PASSHICTRL || HTCJK != NOCJK))
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
	if (c == '&' && unsign_c < 127  &&
	    (!context->element_stack ||
			 (context->element_stack->tag  &&
	      (context->element_stack->tag->contents == SGML_MIXED ||
	       context->element_stack->tag->contents == SGML_PCDATA ||
	       context->element_stack->tag->contents == SGML_RCDATA)))) {
	    /*
	    **  Setting up for possible entity, without the leading '&'. - FM
	    */
	    string->size = 0;
	    context->state = S_ero;
	} else if (c == '<' && unsign_c < 127) {
	    /*
	    **  Setting up for possible tag. - FM
	    */
	    string->size = 0;
	    context->state = (context->element_stack &&
	    		context->element_stack->tag  &&
			context->element_stack->tag->contents == SGML_LITTERAL)
	      				 ?
	    		      S_litteral : S_tag;
#ifndef EXP_CHARTRANS
#define PASS8859SPECL HTPassHighCtrlRaw
#else
#define PASS8859SPECL context->T.pass_160_173_raw
#endif /* EXP_CHARTRANS */
	/*
	**  Convert 160 (nbsp) to Lynx special character if
	**  neither HTPassHighCtrlRaw nor HTCJK is set. - FM
	*/
	} else if (unsign_c == 160 &&
		   !(PASS8859SPECL || HTCJK != NOCJK)) {
            PUTC(HT_NON_BREAK_SPACE);
	/*
	**  Convert 173 (shy) to Lynx special character if
	**  neither HTPassHighCtrlRaw nor HTCJK is set. - FM
	*/
	} else if (unsign_c == 173 &&
		   !(PASS8859SPECL || HTCJK != NOCJK)) {
            PUTC(LY_SOFT_HYPHEN);

#ifdef EXP_CHARTRANS
	} else if (context->T.use_raw_char_in && saved_char_in) {
	    /*
	    **  Only if the original character is still in saved_char_in,
	    **  otherwise we may be iterating from a goto top
	    */
	    PUTC(saved_char_in);
	    saved_char_in = '\0';
/******************************************************************
 *   I. LATIN-1 OR UCS2  TO  DISPLAY CHARSET
 ******************************************************************/  
	} else if ((chk = (context->T.trans_from_uni && unsign_c >= 160)) &&
		   (uck = UCTransUniChar(unsign_c,
					 context->html_char_set)) >= 32 &&
		   uck < 256) {
	    if (TRACE) {
		fprintf(stderr,
			"UCTransUniChar returned 0x%lx:'%c'.\n",
			uck, (char)uck);
	    }
	    c = (char)(uck & 0xff);
	    PUTC(c);
	} else if (chk && ((uck == -4 ||
			    (context->T.repl_translated_C0 &&
			     uck > 0 && uck < 32))) &&
		   /*
		   **  Not found; look for replacement string. - KW
		   */
		   (uck = UCTransUniCharStr(replace_buf,60, clong,
					    context->html_char_set,
					    0) >= 0)) { 
	    /*
	    **  No further tests for valididy - assume that whoever
	    **  defined replacement strings knew what she was doing.
	    */
	      for (p=replace_buf; *p; p++)
		PUTC(*p);
#endif /* EXP_CHARTRANS */

	/*
	**  If it's any other (> 160) 8-bit chararcter, and
	**  we have not set HTPassEightBitRaw nor HTCJK, nor
	**  have the "ISO Latin 1" character set selected,
	**  back translate for our character set. - FM
	*/
#ifndef EXP_CHARTRANS
#define PASSHI8BIT HTPassEightBitRaw
#else
#define PASSHI8BIT (HTPassEightBitRaw || (context->T.do_8bitraw && !context->T.trans_from_uni))
#endif /* EXP_CHARTRANS */
	} else if (unsign_c > 160 && unsign_c < 256 &&
		   !(PASSHI8BIT || HTCJK != NOCJK) &&
		   strncmp(LYchar_set_names[current_char_set],
		   	   "ISO Latin 1", 11)) {
	    int i;
	    int value;

	    string->size = 0;
	    value = (int)(unsign_c - 160);
	    EntityName = HTMLGetEntityName(value);
	    for (i = 0; EntityName[i]; i++)
	        HTChunkPutc(string, EntityName[i]);
	    HTChunkTerminate(string);
	    handle_entity(context, '\0');
	    string->size = 0;
	    if (!FoundEntity)
	        PUTC(';');
#ifdef EXP_CHARTRANS
	/*
	**  If we get to here and have an ASCII char, pass the character.
	*/
	} else if (unsign_c < 127 && unsign_c > 0) {
	    PUTC(c);
	/*
	**  If we get to here, and should have translated,
	**  translation has failed so far.  
	*/
	} else if (context->T.output_utf8 && *context->utf_buf) {
	    for (p=context->utf_buf; *p; p++)
		PUTC(*p);
	    context->utf_buf_p = context->utf_buf;
	    *(context->utf_buf_p) = '\0';

	} else if (context->T.strip_raw_char_in && saved_char_in &&
		   ((unsigned char)saved_char_in >= 0xc0) &&
		   ((unsigned char)saved_char_in < 255)) {
	    /*
	    **  KOI8 special: strip high bit, gives (somewhat) readable
	    **  ASCII or KOI7 - it was constructed that way!
	    */
	    PUTC((char)(saved_char_in & 0x7f));
	    saved_char_in = '\0';
	} else if ((unsigned char)c <
			LYlowest_eightbit[context->html_char_set] ||
		   (context->T.trans_from_uni && !HTPassEightBitRaw)) {
	    sprintf(replace_buf,"U%.2lx",unsign_c);
	    for (p=replace_buf; *p; p++)
		PUTC(*p);
#endif /* EXP_CHARTRANS */
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
	if (unsign_c < 127 && isalnum((unsigned char)c)) {
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
	    goto top1;
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
	        goto top1;
	}
	break;

    /*
    **  Check for a numeric entity.
    */
    case S_cro:
	if (unsign_c < 127 && (unsigned char)c == 'x') {
	    context->isHex = TRUE;
	    context->state = S_incro;
	} else if (unsign_c < 127 && isdigit((unsigned char)c)) {
	    /*
	    **  Accept only valid ASCII digits. - FM
	    */
	    HTChunkPutc(string, c);	/* accumulate a character NUMBER */
	    context->isHex = FALSE;
	    context->state = S_incro;
	} else if (string->size == 0) {
	    /*
	    **  No 'x' or digit following the "&#" so recover
	    **  them and recycle the character. - FM
	    */
	    PUTC('&');
	    PUTC('#');
	    context->state = S_text;
	    goto top1;
	}
	break;

    /*
    **  Handle a numeric entity.
    */
    case S_incro:
	if ((unsign_c < 127) &&
	    (context->isHex ? isxdigit((unsigned char)c) :
			      isdigit((unsigned char)c))) {
	    /*
	    **  Accept only valid hex or ASCII digits. - FM
	    */
	    HTChunkPutc(string, c);	/* accumulate a character NUMBER */
	} else if (string->size == 0) {
	    /*
	    **  No hex digit following the "&#x" so recover
	    **  them and recycle the character. - FM
	    */
	    PUTC('&');
	    PUTC('#');
	    PUTC('x');
	    context->isHex = FALSE;
	    context->state = S_text;
	    goto top1;
	} else {
	    /*
	    **  Terminate the numeric entity and try to handle it. - FM
	    */
	    int value, i;
	    HTChunkTerminate(string);
	    if ((context->isHex ? sscanf(string->data, "%x", &value) :
				  sscanf(string->data, "%d", &value)) == 1) {
#ifdef EXP_CHARTRANS
		if (value == 160 || value == 173) {
		    /*
		    **  We *always* should interpret these as Latin1 here!
		    **  Output the Lynx special character for nbsp and
		    **  then recycle the terminator or break. - FM
		    */
		    if (value == 160) {
			PUTC(HT_NON_BREAK_SPACE);
		    } else {
			PUTC(LY_SOFT_HYPHEN);
		    }
		    string->size = 0;
		    context->isHex = FALSE;
		    context->state = S_text;
		    if (c != ';')
			goto top1;
		    break;
		}
		/*
		 *  Seek a translation from the chartrans tables.
		 */
	      if ((uck = UCTransUniChar(value,current_char_set)) >= 32 &&
		    uck < 256 &&
		    (uck < 127 ||
		     uck >= LYlowest_eightbit[context->html_char_set])) {
		    if (uck == 160 && current_char_set == 0) {
			/*
			**  Would only happen if some other unicode
			**  is mapped to Latin-1 160.
			*/
			PUTC(HT_NON_BREAK_SPACE);
		    } else if (uck == 173 && current_char_set == 0) {
			/*
			**  Would only happen if some other unicode
			**  is mapped to Latin-1 173.
			*/
			PUTC(LY_SOFT_HYPHEN);
		    } else {
			PUTC(FROMASCII((char)uck));
		    }
		} else if ((uck == -4 ||
			    (context->T.repl_translated_C0 &&
			     uck > 0 && uck < 32)) &&
			   /*
			   **  Not found; look for replacement string.
			   */
		(uck = UCTransUniCharStr(replace_buf,60,value,
				      current_char_set, 0)   >= 0 ) ) { 
		    for (p = replace_buf; *p; p++) {
			PUTC(*p);
		    }
	      } else if (context->T.output_utf8 &&
			 PUTUTF8(value)) {
		  /* do nothing more */ ;
	      } else 
#endif /* EXP_CHARTRANS */
	        if (value == 8482) {
		    /*
		    **  trade  Handle as named entity. - FM
		    */
		    string->size = 0;
		    HTChunkPutc(string, 't');
		    HTChunkPutc(string, 'r');
		    HTChunkPutc(string, 'a');
		    HTChunkPutc(string, 'd');
		    HTChunkPutc(string, 'e');
		    context->isHex = FALSE;
		    context->state = S_entity;
		    goto top1;
	        /*
		**  Show the numeric entity if we get to here
		**  and the value:
		**   (1) Is greater than 255 (but use ASCII characters
		**	 for spaces or dashes).
		**  (2) Is less than 32, and not valid or we don't
		**	have HTCJK set.
		**  (3) Is 127 and we don't have HTPassHighCtrlRaw or
		**	HTCJK set.
		**  (4) Is 128 - 159 and we don't have HTPassHighCtrlNum
		**	set.
		** - FM
		*/
		} else if ((value > 255) ||
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
			PUTC(HT_EM_SPACE);
		    } else if (value == 8211 || value == 8212) {
		        /*
			**  ndash or mdash. - FM
			*/
			PUTC('-');
		    } else {
			/*
			**  Unhandled or illegal value.  Recover the
			**  "&#" or "&#x" and digit(s), and recycle
			**  the terminator. - FM
			*/
			PUTC('&');
			PUTC('#');
			if (context->isHex) {
			    PUTC('x');
			    context->isHex = FALSE;
			}
			string->size--;
			for (i = 0; i < string->size; i++)	/* recover */
			    PUTC(string->data[i]);
			string->size = 0;
			context->isHex = FALSE;
			context->state = S_text;
			goto top1;
		    }
		} else if (value == 160) {
		    /*
		    **  Use Lynx special character for 160 (nbsp). - FM
		    */
		    PUTC(HT_NON_BREAK_SPACE);
		} else if (value == 173) {
		    /*
		    **  Use Lynx special character for 173 (shy) - FM
		    */
		    PUTC(LY_SOFT_HYPHEN);
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
			if (context->isHex) {
			    PUTC('x');
			    context->isHex = FALSE;
			}
			string->size--;
			for (i = 0; i < string->size; i++)	/* recover */
			    PUTC(string->data[i]);
			string->size = 0;
			context->isHex = FALSE;
			context->state = S_text;
			goto top1;
		    }
		}
		/*
		**  If we get to here, we succeeded.  Hoorah!!! - FM
		*/
		string->size = 0;
		context->isHex = FALSE;
		context->state = S_text;
		/*
		**  Don't eat the terminator if it's not
		**  the "standard" semi-colon for HTML. - FM
		*/
		if (c != ';')
		    goto top1;
	    } else {
	        /*
		**  Not an entity, and don't know why not, so add
		**  the terminator to the string, output the "&#"
		**  or "&#x", and process the string via the recover
		**  element. - FM
		*/
		string->size--;
		HTChunkPutc(string, c);
		HTChunkTerminate(string);
	        PUTC('&');
	        PUTC('#');
		if (context->isHex) {
		    PUTC('x');
		    context->isHex = FALSE;
		}
		if (context->recover == NULL) {
		    StrAllocCopy(context->recover, string->data);
		    context->recover_index = 0;
		} else {
		    StrAllocCat(context->recover, string->data);
		}
		string->size = 0;
		context->isHex = FALSE;
		context->state = S_text;
		break;
	    }
	}
	break;

    /*
    **  Tag
    */	    
    case S_tag:					/* new tag */
	if (unsign_c < 127 && isalnum((unsigned char)c)) {
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
        } else if (!string->size && (WHITE(c) || c == '=')) {/* <WHITE or <= */
	    /*
	    **  Recover the '<' and WHITE or '=' character. - FM & KW
	    */
	    context->state = S_text;
	    PUTC('<');
	    goto top1;
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
	        goto top1;	/* back and treat it as the tag terminator */
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
	if (unsign_c < 127 && isalnum((unsigned char)c))
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
#ifdef EXTENDED_HTMLDTD
		/*
		**  Just handle ALL end tags normally :-) - kw
		*/
		if (New_DTD) {
		    end_element( context, context->current_tag);
		} else
#endif /* EXTENDED_HTMLDTD */
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
			   (!strcasecomp(string->data, "A") ||
			    !strcasecomp(string->data, "B") ||
			    !strcasecomp(string->data, "BLINK") ||
			    !strcasecomp(string->data, "CITE") ||
			    !strcasecomp(string->data, "EM") ||
			    !strcasecomp(string->data, "FONT") ||
			    !strcasecomp(string->data, "FORM") ||
			    !strcasecomp(string->data, "I") ||
			    !strcasecomp(string->data, "STRONG") ||
			    !strcasecomp(string->data, "TT") ||
			    !strcasecomp(string->data, "U"))) {
		    /*
		    **  Handle end tags for container elements declared
		    **  as SGML_EMPTY to prevent "expected tag substitution"
		    **  but still processed via HTML_end_element() in HTML.c
		    **  with checks there to avoid throwing the HTML.c stack
		    **  out of whack (Ugh, what a hack! 8-). - FM
		    */
		    if (TRACE)
		        fprintf(stderr, "SGML: End </%s>\n", string->data);
		    (*context->actions->end_element)
			(context->target,
			 (context->current_tag - context->dtd->tags),
			 (char **)&context->include);
		    string->size = 0;
		    context->current_attribute_number = INVALID;
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

PUBLIC HTStream* SGML_new  ARGS3(
	CONST SGML_dtd *,	dtd,
	HTParentAnchor *,	anchor,
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
    context->isHex = FALSE;

#ifdef EXP_CHARTRANS
    context->node_anchor = anchor; /*only for chartrans info. could be NULL? */

    context->utf_count = 0;
    context->utf_char = 0;
    context->utf_buf[0] = context->utf_buf[6] = '\0';
    context->utf_buf_p = context->utf_buf;

    UCTransParams_clear(&context->T);
    context->in_char_set = HTAnchor_getUCLYhndl(anchor, UCT_STAGE_PARSER);
    if (context->in_char_set < 0) {
	HTAnchor_copyUCInfoStage(anchor,
				 UCT_STAGE_PARSER, UCT_STAGE_MIME, -1);
	context->in_char_set = HTAnchor_getUCLYhndl(anchor, UCT_STAGE_PARSER);
    }
    context->UCI=HTAnchor_getUCInfoStage(anchor, UCT_STAGE_PARSER);
    set_chartrans_handling(context, anchor, -1);
#endif /* EXP_CHARTRANS */

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
