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
**	from the incomming data or the anchor.	Currently it is from the former
**	which is cleanest.
*/

#include <HTUtils.h>
#include <tcp.h>

#define BUFFER_SIZE    200	/* Line buffer attempts to make neat breaks */
#define MAX_CLEANNESS	20

/* Implements:
*/
#include <HTMLGen.h>

#include <stdio.h>
#include <HTMLDTD.h>
#include <HTStream.h>
#include <SGML.h>
#include <HTFormat.h>

#include <LYLeaks.h>

#define FREE(x) if (x) {free(x); x = NULL;}

#define PUTC(c) (*me->targetClass.put_character)(me->target, c)
/* #define PUTS(s) (*me->targetClass.put_string)(me->target, s) */
#define PUTB(s,l) (*me->targetClass.put_block)(me->target, s, l)

/*		HTML Object
**		-----------
*/
struct _HTStream {
	CONST HTStreamClass *		isa;
	HTStream *			target;
	HTStreamClass			targetClass;	/* COPY for speed */
};

struct _HTStructured {
	CONST HTStructuredClass *	isa;
	HTStream *			target;
	HTStreamClass			targetClass;	/* COPY for speed */

	char				buffer[BUFFER_SIZE+1]; /* 1for NL */
	int				buffer_maxchars;
	char *				write_pointer;
	char *				line_break [MAX_CLEANNESS+1];
	int				cleanness;
	BOOL				overflowed;
	BOOL				delete_line_break_char[MAX_CLEANNESS+1];
	BOOL				preformatted;
	BOOL				escape_specials;
	BOOL				in_attrval;
};

/*	Flush Buffer
**	------------
*/

PRIVATE void flush_breaks ARGS1(
	HTStructured *, 	me)
{
    int i;
    for (i=0; i<= MAX_CLEANNESS; i++) {
	me->line_break[i] = NULL;
    }
}

PRIVATE void HTMLGen_flush ARGS1(
	HTStructured *, 	me)
{
    (*me->targetClass.put_block)(me->target,
				 me->buffer,
				 me->write_pointer - me->buffer);
    me->write_pointer = me->buffer;
    flush_breaks(me);
    me->cleanness = 0;
    me->delete_line_break_char[0] = NO;
}

/*	Weighted optional line break
**
**	We keep track of all the breaks for when we chop the line
*/

PRIVATE void allow_break ARGS3(
	HTStructured *, me,
	int,		new_cleanness,
	BOOL,		dlbc)
{
    if (dlbc && me->write_pointer == me->buffer) dlbc = NO;
    me->line_break[new_cleanness] =
			 dlbc ? me->write_pointer - 1 /* Point to space */
			      : me->write_pointer ;   /* point to gap */
    me->delete_line_break_char[new_cleanness] = dlbc;
    if (new_cleanness >= me->cleanness &&
	(me->overflowed || me->line_break[new_cleanness] > me->buffer))
	me->cleanness = new_cleanness;
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
**	by hand, too, though this is not a primary design consideration. TBL
*/
PRIVATE void HTMLGen_put_character ARGS2(
	HTStructured *, 	me,
	char,			c)
{
    if (me->escape_specials && (unsigned char)c < 32) {
	if (c == HT_NON_BREAK_SPACE || c == HT_EM_SPACE ||
	    c == LY_SOFT_HYPHEN) { /* recursion... */
	    HTMLGen_put_character(me, '&');
	    HTMLGen_put_character(me, '#');
	    HTMLGen_put_character(me, 'x');
	    switch(c) {
	    case HT_NON_BREAK_SPACE: /* &#xA0; */
		HTMLGen_put_character(me, 'A');
		HTMLGen_put_character(me, '0');
		break;
	    case HT_EM_SPACE: /* &#x2003; */
		HTMLGen_put_character(me, '2');
		HTMLGen_put_character(me, '0');
		HTMLGen_put_character(me, '0');
		HTMLGen_put_character(me, '3');
		break;
	    case LY_SOFT_HYPHEN: /* &#xAD; */
		HTMLGen_put_character(me, 'A');
		HTMLGen_put_character(me, 'D');
		break;
	    }
	    c = ';';
	}
    }

    *me->write_pointer++ = c;

    if (c == '\n') {
	HTMLGen_flush(me);
	return;
    }

    /* Figure our whether we can break at this point
    */
    if ((!me->preformatted && (c == ' ' || c == '\t'))) {
	int new_cleanness = 3;
	if (me->write_pointer > (me->buffer + 1)) {
	    char delims[5];
	    char * p;
	    strcpy(delims, ",;:.");		/* @@ english bias */
	    p = strchr(delims, me->write_pointer[-2]);
	    if (p) new_cleanness = p - delims + 6;
	    if (!me->in_attrval) new_cleanness += 10;
	}
	allow_break(me, new_cleanness, YES);
    }

    /*
     *	Flush buffer out when full, or whenever the line is over
     *	the nominal maximum and we can break at all
     */
    if (me->write_pointer >= me->buffer + me->buffer_maxchars ||
	(me->overflowed && me->cleanness)) {
	if (me->cleanness) {
	    char line_break_char = me->line_break[me->cleanness][0];
	    char * saved = me->line_break[me->cleanness];

	    if (me->delete_line_break_char[me->cleanness]) saved++;
	    me->line_break[me->cleanness][0] = '\n';
	    (*me->targetClass.put_block)(me->target,
					 me->buffer,
			       me->line_break[me->cleanness] - me->buffer + 1);
	    me->line_break[me->cleanness][0] = line_break_char;
	    {  /* move next line in */
		char * p = saved;
		char *q;
		for (q = me->buffer; p < me->write_pointer; )
		    *q++ = *p++;
	    }
	    me->cleanness = 0;
	    /* Now we have to check whether ther are any perfectly good breaks
	    ** which weren't good enough for the last line but may be
	    **	good enough for the next
	    */
	    {
		int i;
		for(i=0; i <= MAX_CLEANNESS; i++) {
		    if (me->line_break[i] != NULL &&
			me->line_break[i] > saved) {
			me->line_break[i] = me->line_break[i] -
						(saved-me->buffer);
			me->cleanness = i;
		    } else {
			me->line_break[i] = NULL;
		    }
		}
	    }

	    me->delete_line_break_char[0] = 0;
	    me->write_pointer = me->write_pointer - (saved-me->buffer);
	    me->overflowed = NO;

	} else {
	    (*me->targetClass.put_block)(me->target,
					 me->buffer,
					 me->buffer_maxchars);
	    me->write_pointer = me->buffer;
	    flush_breaks(me);
	    me->overflowed = YES;
	}
    }
}

/*	String handling
**	---------------
*/
PRIVATE void HTMLGen_put_string ARGS2(
	HTStructured *, 	me,
	CONST char *,		s)
{
    CONST char * p;

    for (p = s; *p; p++)
	HTMLGen_put_character(me, *p);
}

PRIVATE void HTMLGen_write ARGS3(
	HTStructured *, 	me,
	CONST char *,		s,
	int,			l)
{
    CONST char * p;

    for (p = s; p < (s + l); p++)
	HTMLGen_put_character(me, *p);
}

/*	Start Element
**	-------------
**
**	Within the opening tag, there may be spaces
**	and the line may be broken at these spaces.
*/
PRIVATE void HTMLGen_start_element ARGS6(
	HTStructured *, 	me,
	int,			element_number,
	CONST BOOL*,		present,
	CONST char **,		value,
	int,			charset GCC_UNUSED,
	char **,		insert GCC_UNUSED)
{
    int i;
    BOOL was_preformatted = me->preformatted;
    HTTag * tag = &HTML_dtd.tags[element_number];

    me->preformatted = YES;	/* free text within tags */
    HTMLGen_put_character(me, '<');
    HTMLGen_put_string(me, tag->name);
    if (present) {
	BOOL had_attr = NO;
	for (i = 0; i < tag->number_of_attributes; i++) {
	    if (present[i]) {
		had_attr = YES;
		HTMLGen_put_character(me, ' ');
		allow_break(me, 11, YES);
		HTMLGen_put_string(me, tag->attributes[i].name);
		if (value[i]) {
		    me->preformatted = was_preformatted;
		    me->in_attrval = YES;
		    if (strchr(value[i], '"') == NULL) {
			HTMLGen_put_string(me, "=\"");
			HTMLGen_put_string(me, value[i]);
			HTMLGen_put_character(me, '"');
		    } else if (strchr(value[i], '\'') == NULL) {
			HTMLGen_put_string(me, "='");
			HTMLGen_put_string(me, value[i]);
			HTMLGen_put_character(me, '\'');
		    } else {  /* attribute value has both kinds of quotes */
			CONST char *p;
			HTMLGen_put_string(me, "=\"");
			for (p = value[i]; *p; p++) {
			    if (*p != '"') {
				HTMLGen_put_character(me, *p);
			    } else {
				HTMLGen_put_string(me, "&#34;");
			    }
			}
			HTMLGen_put_character(me, '"');
		    }
		    me->preformatted = YES;
		    me->in_attrval = NO;
		}
	    }
	}
	if (had_attr)
	    allow_break(me, 12, NO);
    }
    HTMLGen_put_string(me, ">"); /* got rid of \n LJM */

    /*
     *	Make very specific HTML assumption that PRE can't be nested!
     */
    me->preformatted = (element_number == HTML_PRE)  ? YES : was_preformatted;

    /*
     *	Can break after element start.
     */
    if (!me->preformatted && tag->contents != SGML_EMPTY) {
	if (HTML_dtd.tags[element_number].contents == SGML_ELEMENT)
	    allow_break(me, 15, NO);
	else
	    allow_break(me, 2, NO);
    }
}

/*		End Element
**		-----------
**
*/
/*	When we end an element, the style must be returned to that
**	in effect before that element.	Note that anchors (etc?)
**	don't have an associated style, so that we must scan down the
**	stack for an element with a defined style. (In fact, the styles
**	should be linked to the whole stack not just the top one.)
**	TBL 921119
*/
PRIVATE void HTMLGen_end_element ARGS3(
	HTStructured *, 	me,
	int,			element_number,
	char **,		insert GCC_UNUSED)
{
    if (!me->preformatted &&
	HTML_dtd.tags[element_number].contents != SGML_EMPTY) {
	/*
	 *  Can break before element end.
	 */
	if (HTML_dtd.tags[element_number].contents == SGML_ELEMENT)
	    allow_break(me, 14, NO);
	else
	    allow_break(me, 1, NO);
    }
    HTMLGen_put_string(me, "</");
    HTMLGen_put_string(me, HTML_dtd.tags[element_number].name);
    HTMLGen_put_character(me, '>');
    if (element_number == HTML_PRE) {
	me->preformatted = NO;
    }
}

/*		Expanding entities
**		------------------
**
*/
PRIVATE int HTMLGen_put_entity ARGS2(
	HTStructured *, 	me,
	int,			entity_number)
{
    int nent = HTML_dtd.number_of_entities;

    HTMLGen_put_character(me, '&');
    if (entity_number < nent) {
      HTMLGen_put_string(me, HTML_dtd.entity_names[entity_number]);
    }
    HTMLGen_put_character(me, ';');
    return HT_OK;
}

/*	Free an HTML object
**	-------------------
**
*/
PRIVATE void HTMLGen_free ARGS1(
	HTStructured *, 	me)
{
    (*me->targetClass.put_character)(me->target, '\n');
    HTMLGen_flush(me);
    (*me->targetClass._free)(me->target);	/* ripple through */
    FREE(me);
}

PRIVATE void PlainToHTML_free ARGS1(
	HTStructured *, 	me)
{
    HTMLGen_end_element(me, HTML_PRE, 0);
    HTMLGen_free(me);
}

PRIVATE void HTMLGen_abort ARGS2(
	HTStructured *, 	me,
	HTError,		e GCC_UNUSED)
{
    HTMLGen_free(me);
}

PRIVATE void PlainToHTML_abort ARGS2(
	HTStructured *, 	me,
	HTError,		e GCC_UNUSED)
{
    PlainToHTML_free(me);
}

/*	Structured Object Class
**	-----------------------
*/
PRIVATE CONST HTStructuredClass HTMLGeneration = /* As opposed to print etc */
{
	"HTMLGen",
	HTMLGen_free,
	HTMLGen_abort,
	HTMLGen_put_character,	HTMLGen_put_string, HTMLGen_write,
	HTMLGen_start_element,	HTMLGen_end_element,
	HTMLGen_put_entity
};

/*	Subclass-specific Methods
**	-------------------------
*/
extern int LYcols;			/* LYCurses.h, set in LYMain.c	*/
extern BOOL dump_output_immediately;	/* TRUE if no interactive user	*/
extern int dump_output_width;		/* -width instead of 80 	*/
extern BOOLEAN LYPreparsedSource;	/* Show source as preparsed?	*/

PUBLIC HTStructured * HTMLGenerator ARGS1(
	HTStream *,		output)
{
    HTStructured* me = (HTStructured*)malloc(sizeof(*me));
    if (me == NULL)
	outofmem(__FILE__, "HTMLGenerator");
    me->isa = &HTMLGeneration;

    me->target = output;
    me->targetClass = *me->target->isa; /* Copy pointers to routines for speed*/

    me->write_pointer = me->buffer;
    flush_breaks(me);
    me->line_break[0] = me->buffer;
    me->cleanness =	0;
    me->overflowed = NO;
    me->delete_line_break_char[0] = NO;
    me->preformatted =	NO;
    me->in_attrval = NO;

    /*
     *	For what line length should we attempt to wrap ? - kw
     */
    if (!LYPreparsedSource) {
	me->buffer_maxchars = 80; /* work as before - kw */
    } else if (dump_output_width > 1) {
	me->buffer_maxchars = dump_output_width; /* try to honor -width - kw */
    } else if (dump_output_immediately) {
	me->buffer_maxchars = 80; /* try to honor -width - kw */
    } else {
	me->buffer_maxchars = LYcols - 2;
	if (me->buffer_maxchars < 38) /* too narrow, let GridText deal */
	    me->buffer_maxchars = 40;
    }
    if (me->buffer_maxchars > 900) /* likely not true - kw */
	me->buffer_maxchars = 78;
    if (me->buffer_maxchars > BUFFER_SIZE) /* must not be larger! */
	me->buffer_maxchars = BUFFER_SIZE - 2;

    /*
     *	If dump_output_immediately is set, there likely isn't anything
     *	after this stream to interpret the Lynx special chars.	Also
     *	if they get displayed via HTPlain, that will probably make
     *	non-breaking space chars etc. invisible.  So let's translate
     *	them to numerical character references.  For debugging
     *	purposes we'll use the new hex format.
     */
    me->escape_specials = LYPreparsedSource;

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
	HTPresentation *,	pres GCC_UNUSED,
	HTParentAnchor *,	anchor GCC_UNUSED,
	HTStream *,		sink)
{
    HTStructured *me = (HTStructured *)malloc(sizeof(*me));
    if (me == NULL)
	outofmem(__FILE__, "PlainToHTML");
    me->isa = (CONST HTStructuredClass *)&PlainToHTMLConversion;

    /*
     *	Copy pointers to routines for speed.
     */
    me->target = sink;
    me->targetClass = *me->target->isa;
    me->write_pointer = me->buffer;
    flush_breaks(me);
    me->cleanness =	0;
    me->overflowed = NO;
    me->delete_line_break_char[0] = NO;
    /* try to honor -width - kw */
    me->buffer_maxchars = (dump_output_width > 1 ?
			   dump_output_width : 80);

    HTMLGen_put_string(me, "<HTML>\n<BODY>\n<PRE>\n");
    me->preformatted = YES;
    me->escape_specials = NO;
    me->in_attrval = NO;
    return (HTStream*) me;
}
