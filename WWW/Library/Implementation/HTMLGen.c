/*		HTML Generator
**		==============
**
**	This version of the HTML object sends HTML markup to the output stream.
**
** Bugs:	Line wrapping is not done at all.
**		All data handled as PCDATA.
**		Should convert old XMP, LISTING and PLAINTEXT to PRE.
**
**	It is not obvious to me right now whether the HEAD should be generated
**	from the incomming data or the anchor.  Currently it is from the former
**	which is cleanest.
*/

#include "HTUtils.h"
#include "tcp.h"

#define BUFFER_SIZE	80	/* Line buffer attempts to make neat breaks */

/* Implements:
*/
#include "HTMLGen.h"

#include <stdio.h>
#include "HTMLDTD.h"
#include "HTStream.h"
#include "SGML.h"
#include "HTFormat.h"

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

#define PUTC(c) (*me->targetClass.put_character)(me->target, c)
/* #define PUTS(s) (*me->targetClass.put_string)(me->target, s) */
#define PUTB(s,l) (*me->targetClass.put_block)(me->target, s, l)

/*		HTML Object
**		-----------
*/

struct _HTStream {
	CONST HTStreamClass *		isa;	
	HTStream * 			target;
	HTStreamClass			targetClass;	/* COPY for speed */
};

struct _HTStructured {
	CONST HTStructuredClass *	isa;
	HTStream * 			target;
	HTStreamClass			targetClass;	/* COPY for speed */
	
	char				buffer[BUFFER_SIZE+1]; /* 1for NL */
	char *				write_pointer;
	char *				line_break;
	int				cleanness;
	BOOL				delete_line_break_char;
	BOOL				preformatted;
};


/*	Flush Buffer
**	------------
*/
PRIVATE void HTMLGen_flush ARGS1(HTStructured *, me)
{
    (*me->targetClass.put_block)(me->target, 
    				me->buffer,
				me->write_pointer - me->buffer);
    me->write_pointer = me->buffer;
    me->line_break = me->buffer;
    me->cleanness = 0;
    me->delete_line_break_char = NO;
}


/*	Character handling
**	------------------
**
**	The tricky bits are the line break handling.  This attempts
**	to synchrononise line breaks on sentence or phrase ends. This
**	is important if one stores SGML files in a line-oriented code
**	repository, so that if a small change is made, line ends don't
**	shift in a ripple-through to apparently change a large part of the
**	file. We give extra "cleanness" to spaces appearing directly
**	after periods (full stops), [semi]colons and commas.
**	   This should make the source files easier to read and modify
**	by hand, too, though this is not a primary design consideration.
*/
PRIVATE void HTMLGen_put_character ARGS2(HTStructured *, me, char, c)
{

    *me->write_pointer++ = c;
    
    if (c=='\n') {
        HTMLGen_flush(me);
	return;
    }
    
    if ((!me->preformatted  && c==' ')) {
        int new_cleanness = 1;
	if (me->write_pointer > (me->buffer + 1)) {
	    char delims[5];
	    char * p;
	    strcpy(delims, ",;:.");		/* @@ english bias */
	    p = strchr(delims, me->write_pointer[-2]);
	    if (p) new_cleanness = p - delims + 2;
	}
	if (new_cleanness >= me->cleanness) {
	    me->line_break = me->write_pointer - 1;  /* Point to space */
	    me->cleanness = new_cleanness;
	    me->delete_line_break_char = YES;
	}
    }
    
    /* Flush buffer out when full */
    if (me->write_pointer == me->buffer + BUFFER_SIZE) {
    	if (me->cleanness) {
	    char line_break_char = me->line_break[0];
	    char * saved = me->line_break;
	    
	    if (me->delete_line_break_char) saved++; 
	    me->line_break[0] = '\n'; 
	    (*me->targetClass.put_block)(me->target,
	    				me->buffer,
					me->line_break - me->buffer + 1);
	    me->line_break[0] = line_break_char;
	    {  /* move next line in */
	    	char * p=saved;
		char *q;
		for(q=me->buffer; p < me->write_pointer; )
			*q++ = *p++;
	    }
	    me->cleanness = 0;
	    me->delete_line_break_char = 0;
	    me->write_pointer = me->write_pointer - (saved-me->buffer);

	} else {
	    (*me->targetClass.put_block)(me->target,
	    	me->buffer,
		BUFFER_SIZE);
	    me->write_pointer = me->buffer;
	}
	me->line_break = me->buffer;
    }
}



/*	String handling
**	---------------
*/
PRIVATE void HTMLGen_put_string ARGS2(HTStructured *, me, CONST char*, s)
{
    CONST char * p;
    for(p=s; *p; p++) HTMLGen_put_character(me, *p);
}

PRIVATE void HTMLGen_write ARGS3(HTStructured *, me, CONST char*, s, int, l)
{
    CONST char * p;
    for(p=s; p<s+l; p++) HTMLGen_put_character(me, *p);
}


/*	Start Element
**	-------------
**
**	Within the opening tag, there may be spaces
**	and the line may be broken at these spaces.
*/
PRIVATE void HTMLGen_start_element ARGS5(
	HTStructured *, 	me,
	int,			element_number,
	CONST BOOL*,	 	present,
	CONST char **,		value,
	char **,		insert)
{
    int i;
    
    BOOL was_preformatted = me->preformatted;
    HTTag * tag = &HTML_dtd.tags[element_number];

    me->preformatted = NO;	/* free text within tags */
    HTMLGen_put_character(me, '<');
    HTMLGen_put_string(me, tag->name);
    if (present) for (i=0; i< tag->number_of_attributes; i++) {
        if (present[i]) {
	    HTMLGen_put_character(me, ' ');
	    HTMLGen_put_string(me, tag->attributes[i].name);
	    if (value[i]) {
	 	HTMLGen_put_string(me, "=\"");
		HTMLGen_put_string(me, value[i]);
		HTMLGen_put_character(me, '"');
	    }
	}
    }
    HTMLGen_put_string(me, ">"); /* got rid of \n LJM */
    
    /* Make very specific HTML assumption that PRE can't be
       nested! */
    me->preformatted = (element_number == HTML_PRE)  ? YES : was_preformatted;

	/* can break after element start */ 
    if (!me->preformatted && tag->contents != SGML_EMPTY) {  
    	me->line_break = me->write_pointer;	/* Don't you hate SGML?  */
	me->cleanness = 1;
	me->delete_line_break_char = NO;
    }
}


/*		End Element
**		-----------
**
*/
/*	When we end an element, the style must be returned to that
**	in effect before that element.  Note that anchors (etc?)
**	don't have an associated style, so that we must scan down the
**	stack for an element with a defined style. (In fact, the styles
**	should be linked to the whole stack not just the top one.)
**	TBL 921119
*/
PRIVATE void HTMLGen_end_element ARGS3(HTStructured *, me,
			int , element_number, char **, insert)
{
    if (!me->preformatted && 
		HTML_dtd.tags[element_number].contents != SGML_EMPTY) {
    				/* can break before element end */ 
    	me->line_break = me->write_pointer;	/* Don't you hate SGML?  */
	me->cleanness = 1;
	me->delete_line_break_char = NO;
    }
    HTMLGen_put_string(me, "</");
    HTMLGen_put_string(me, HTML_dtd.tags[element_number].name);
    HTMLGen_put_character(me, '>');
    if (element_number == HTML_PRE) me->preformatted = NO;
}


/*		Expanding entities
**		------------------
**
*/

PRIVATE void HTMLGen_put_entity ARGS2(HTStructured *, me, int, entity_number)
{
    HTMLGen_put_character(me, '&');
    HTMLGen_put_string(me, HTML_dtd.entity_names[entity_number]);
    HTMLGen_put_character(me, ';');
}



/*	Free an HTML object
**	-------------------
**
*/
PRIVATE void HTMLGen_free ARGS1(HTStructured *, me)
{
    (*me->targetClass.put_character)(me->target, '\n');
    HTMLGen_flush(me);
    (*me->targetClass._free)(me->target);	/* ripple through */
    FREE(me);
}


PRIVATE void PlainToHTML_free ARGS1(HTStructured *, me)
{
    HTMLGen_end_element(me, HTML_PRE, 0);
    HTMLGen_free(me);
}



PRIVATE void HTMLGen_abort ARGS2(HTStructured *, me, HTError, e)
{
    HTMLGen_free(me);
}


PRIVATE void PlainToHTML_abort ARGS2(HTStructured *, me, HTError, e)
{
    PlainToHTML_free(me);
}



/*	Structured Object Class
**	-----------------------
*/
PRIVATE CONST HTStructuredClass HTMLGeneration = /* As opposed to print etc */
{		
	"text/html",
	HTMLGen_free,
	HTMLGen_abort,
	HTMLGen_put_character, 	HTMLGen_put_string, HTMLGen_write,
	HTMLGen_start_element, 	HTMLGen_end_element,
	HTMLGen_put_entity
}; 


/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStructured * HTMLGenerator ARGS1(HTStream *, output)
{
    HTStructured* me = (HTStructured*)malloc(sizeof(*me));
    if (me == NULL)
        outofmem(__FILE__, "HTMLGenerator");
    me->isa = &HTMLGeneration;       

    me->target = output;
    me->targetClass = *me->target->isa; /* Copy pointers to routines for speed*/
    
    me->write_pointer = me->buffer;
    me->line_break = 	me->buffer;
    me->cleanness = 	0;
    me->delete_line_break_char = NO;
    me->preformatted = 	NO;
    return me;
}

/*	Stream Object Class
**	-------------------
**
**	This object just converts a plain text stream into HTML
**	It is officially a structured strem but only the stream bits exist.
**	This is just the easiest way of typecasting all the routines.
*/
PRIVATE CONST HTStructuredClass PlainToHTMLConversion =
{		
	"plaintexttoHTML",
	HTMLGen_free,	
	PlainToHTML_abort,	
	HTMLGen_put_character,
	HTMLGen_put_string,
	HTMLGen_write,
	NULL,		/* Structured stuff */
	NULL,
	NULL
}; 


/*	HTConverter from plain text to HTML Stream
**	------------------------------------------
*/

PUBLIC HTStream* HTPlainToHTML ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	
	HTStream *,		sink)
{
    HTStructured* me = (HTStructured*)malloc(sizeof(*me));
    if (me == NULL)
        outofmem(__FILE__, "PlainToHTML");
    me->isa = (HTStructuredClass*) &PlainToHTMLConversion;       

    me->target = sink;
    me->targetClass = *me->target->isa;
    	/* Copy pointers to routines for speed*/
	
    HTMLGen_put_string(me, "<HTML>\n<BODY>\n<PRE>\n");
    me->preformatted = YES;
    return (HTStream*) me;
}
