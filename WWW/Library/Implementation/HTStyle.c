/*
 * $LynxId: HTStyle.c,v 1.17 2023/10/24 08:14:31 tom Exp $
 *
 *	Style Implementation for Hypertext			HTStyle.c
 *	==================================
 *
 *	Styles allow the translation between a logical property
 *	of a piece of text and its physical representation.
 *
 *	A StyleSheet is a collection of styles, defining the
 *	translation necessary to
 *	represent a document.  It is a linked list of styles.
 */

#include <HTUtils.h>
#include <HTStyle.h>

#include <LYLeaks.h>

/*	Create a new style
*/
HTStyle *HTStyleNew(void)
{
    HTStyle *self = typecalloc(HTStyle);

    if (self == NULL)
	outofmem(__FILE__, "HTStyleNew");
    return self;
}

/*	Create a new style with a name
*/
HTStyle *HTStyleNewNamed(const char *name)
{
    HTStyle *self = HTStyleNew();

    StrAllocCopy(self->w_name, name);
    self->id = -1;		/* <0 */
    return self;
}

/*	Free a style
*/
HTStyle *HTStyleFree(HTStyle *self)
{
    FREE(self->w_name);
    FREE(self->w_SGMLTag);
    FREE(self);
    return NULL;
}

/*			StyleSheet Functions
 *			====================
 */

/*	Searching for styles:
*/
HTStyle *HTStyleNamed(HTStyleSheet *self, const char *name)
{
    HTStyle *scan;

    for (scan = self->styles; scan; scan = scan->next)
	if (0 == strcmp(GetHTStyleName(scan), name))
	    return scan;
    CTRACE((tfp, "StyleSheet: No style named `%s'\n", name));
    return NULL;
}

/*	Add a style to a sheet
 *	----------------------
 */
HTStyleSheet *HTStyleSheetAddStyle(HTStyleSheet *self, HTStyle *style)
{
    style->next = 0;		/* The style will go on the end */
    if (!self->styles) {
	self->styles = style;
    } else {
	HTStyle *scan;

	for (scan = self->styles; scan->next; scan = scan->next) ;	/* Find end */
	scan->next = style;
    }
    return self;
}

/*	Remove the given object from a style sheet if it exists
*/
HTStyleSheet *HTStyleSheetRemoveStyle(HTStyleSheet *self, HTStyle *style)
{
    if (self->styles == style) {
	self->styles = style->next;
	return self;
    } else {
	HTStyle *scan;

	for (scan = self->styles; scan; scan = scan->next) {
	    if (scan->next == style) {
		scan->next = style->next;
		return self;
	    }
	}
    }
    return NULL;
}

/*	Create new style sheet
*/

HTStyleSheet *HTStyleSheetNew(void)
{
    HTStyleSheet *self = typecalloc(HTStyleSheet);

    if (self == NULL)
	outofmem(__FILE__, "HTStyleSheetNew");
    return self;
}

/*	Free off a style sheet pointer
*/
HTStyleSheet *HTStyleSheetFree(HTStyleSheet *self)
{
    HTStyle *style;

    while ((style = self->styles) != 0) {
	self->styles = style->next;
	HTStyleFree(style);
    }
    FREE(self);
    return NULL;
}
