/*					HTML to rich text converter for libwww
**
**			THE HTML TO RTF OBJECT CONVERTER
**
**  This interprets the HTML semantics.
*/
#ifndef HTML_H
#define HTML_H

#ifdef EXP_CHARTRANS
#include "UCDefs.h"
#include "UCAux.h"
#endif

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif /* HTUTILS_H */
#include "HTAnchor.h"
#include "HTMLDTD.h"

#ifdef SHORT_NAMES
#define HTMLPresentation        HTMLPren
#define HTMLPresent             HTMLPres
#endif /* SHORT_NAMES */

extern CONST HTStructuredClass HTMLPresentation;

#ifdef Lynx_HTML_Handler
/*
**	This section is semi-private to HTML.c and it's helper modules. - FM
**	--------------------------------------------------------------------
*/

typedef struct _stack_element {
        HTStyle *	style;
	int		tag_number;
} stack_element;

/*		HTML Object
**		-----------
*/
#define MAX_NESTING 800		/* Should be checked by parser */

struct _HTStructured {
    CONST HTStructuredClass * 	isa;
    HTParentAnchor * 		node_anchor;
    HText * 			text;

    HTStream*			target;			/* Output stream */
    HTStreamClass		targetClass;		/* Output routines */

    HTChildAnchor *		CurrentA;	/* current HTML_A anchor */
    int				CurrentANum;	/* current HTML_A number */
    char *			base_href;	/* current HTML_BASE href */
    char *			map_address;	/* current HTML_MAP address */

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
    BOOL			first_option;	/* First OPTION in SELECT? */
    char *			LastOptionValue;
    BOOL			LastOptionChecked;
    BOOL			select_disabled;
    HTChunk			textarea;	/* Grow by 128 */
    char *			textarea_name;
    int				textarea_name_cs;
    char *			textarea_accept_cs;
    char *			textarea_cols;
    int 			textarea_rows;
    int				textarea_disabled;
    char *			textarea_id;
    HTChunk			math;		/* Grow by 128 */
    HTChunk			style_block;	/* Grow by 128 */
    HTChunk			script;		/* Grow by 128 */

    /*
     *  Used for nested lists. - FM
     */
    int		List_Nesting_Level;	/* counter for list nesting level */
    int 	OL_Counter[12];		/* counter for ordered lists */
    char 	OL_Type[12];		/* types for ordered lists */
    int 	Last_OL_Count;		/* last count in ordered lists */
    char 	Last_OL_Type;		/* last type in ordered lists */

    int				Division_Level;
    short			DivisionAlignments[MAX_NESTING];
    int				Underline_Level;
    int				Quote_Level;

    BOOL			UsePlainSpace;
    BOOL			HiddenValue;
    int				lastraw;

    char *			comment_start;	/* for literate programming */
    char *			comment_end;

    HTTag *			current_tag;
    BOOL			style_change;
    HTStyle *			new_style;
    HTStyle *			old_style;
    int				current_default_alignment;
    BOOL			in_word;  /* Have just had a non-white char */
    stack_element 	stack[MAX_NESTING];
    stack_element 	*sp;		/* Style stack pointer */
    BOOL		stack_overrun;	/* Was MAX_NESTING exceeded? */
    int			skip_stack; /* flag to skip next style stack operation */

    /*
    **  Track if we are in an anchor, paragraph, address, base, etc.
    */
    BOOL		inA;
    BOOL		inAPPLET;
    BOOL		inAPPLETwithP;
    BOOL		inBadBASE;
    BOOL		inBadHREF;
    BOOL		inBadHTML;
    BOOL		inBASE;
    BOOL		inBoldA;
    BOOL		inBoldH;
    BOOL		inCAPTION;
    BOOL		inCREDIT;
    BOOL		inFIG;
    BOOL		inFIGwithP;
    BOOL		inFONT;
    BOOL		inFORM;
    BOOL		inLABEL;
    BOOL		inP;
    BOOL		inPRE;
    BOOL		inSELECT;
    BOOL		inTABLE;
    BOOL		inTEXTAREA;
    BOOL		inUnderline;

    BOOL		needBoldH;

#ifdef EXP_CHARTRANS
    LYUCcharset	* UCI;	/* pointer to node_anchor's UCInfo */
    int	UCLYhndl;		/* tells us what charset we are fed */
    UCTransParams T;
    int 		tag_charset; /* charset for attribute values etc. */
#endif
};

struct _HTStream {
    CONST HTStreamClass *	isa;
    /* .... */
};

/*
 *	Semi-Private functions. - FM
 */
extern void HTML_put_character PARAMS((HTStructured *me, char c));
extern void HTML_put_string PARAMS((HTStructured *me, CONST char *s));
extern void HTML_write PARAMS((HTStructured *me, CONST char *s, int l));
extern int HTML_put_entity PARAMS((HTStructured *me, int entity_number));
#endif /* Lynx_HTML_Handler */

extern void strtolower PARAMS((char* i));

/*				P U B L I C
*/

/*
**  HTConverter to present HTML
*/
extern HTStream* HTMLToPlain PARAMS((
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink));

extern HTStream* HTMLParsedPresent PARAMS((
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink));

extern HTStream* HTMLToC PARAMS((
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink));

extern HTStream* HTMLPresent PARAMS((
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink));

extern HTStructured* HTML_new PARAMS((
        HTParentAnchor * anchor,
        HTFormat        format_out,
        HTStream *      target));

/*
**  Names for selected internal representations.
*/
typedef enum _HTMLCharacterSet {
        HTML_ISO_LATIN1,
        HTML_NEXT_CHARS,
        HTML_PC_CP950
} HTMLCharacterSet;

/*
**  Record error message as a hypertext object.
**
**  The error message should be marked as an error so that it can be
**  reloaded later.  This implementation just throws up an error message
**  and leaves the document unloaded.
**
**  On entry,
**      sink    is a stream to the output device if any
**      number  is the HTTP error number
**      message is the human readable message.
**  On exit,
**      a retrun code like HT_LOADED if object exists else 60; 0
*/
extern int HTLoadError PARAMS((
	HTStream *	sink,
	int		number,
	CONST char *	message));

#endif /* HTML_H */

