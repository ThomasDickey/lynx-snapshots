/*			Lynx Document Reference List Support	      LYList.c
**			====================================
**
**	Author: FM	Foteos Macrides (macrides@sci.wfbr.edu)
**
*/

#include <HTUtils.h>
#include <tcp.h>
#include <LYUtils.h>
#include <GridText.h>
#include <LYList.h>
#include <LYSignal.h>
#include <LYGlobalDefs.h>
#include <LYCharUtils.h>

#ifdef DIRED_SUPPORT
#include <LYUpload.h>
#include <LYLocal.h>
#endif /* DIRED_SUPPORT */

#include <LYexit.h>
#include <LYLeaks.h>

#define FREE(x) if (x) {free(x); x = NULL;}

/*	showlist - F.Macrides (macrides@sci.wfeb.edu)
**	--------
**	Create a temporary text/html file with a list of links to
**	HyperText References in the current document.
**
**  On entry
**	titles		Set:	if we want titles where available
**			Clear:	we only get addresses.
*/

static char list_filename[256] = "\0";

/*
 *  Returns the name of the file used for the List Page, if one has
 *  been created, as a full URL; otherwise, returns an empty string.
 * - kw
 */
PUBLIC char * LYlist_temp_url NOARGS
{
    return list_filename;
}

PUBLIC int showlist ARGS2(
	document *,	newdoc,
	BOOLEAN,	titles)
{
    int cnt;
    int refs, hidden_links;
    static char tempfile[256];
    FILE *fp0;
    char *Address = NULL, *Title = NULL, *cp = NULL;
    BOOLEAN intern_w_post = FALSE;
    char *desc = "unknown field or link";

    refs = HText_sourceAnchors(HTMainText);
    hidden_links = HText_HiddenLinkCount(HTMainText);
    if (refs <= 0 && hidden_links > 0 &&
	LYHiddenLinks != HIDDENLINKS_SEPARATE) {
	_statusline(NO_VISIBLE_REFS_FROM_DOC);
	sleep(MessageSecs);
	return(-1);
    }
    if (refs <= 0 && hidden_links <= 0) {
	_statusline(NO_REFS_FROM_DOC);
	sleep(MessageSecs);
	return(-1);
    }

    LYRemoveTemp(tempfile);
    if ((fp0 = LYOpenTemp(tempfile, HTML_SUFFIX, "w")) == NULL) {
	_statusline(CANNOT_OPEN_TEMP);
	sleep(MessageSecs);
	return(-1);
    }

    LYLocalFileToURL(list_filename, tempfile);

    StrAllocCopy(newdoc->address, list_filename);
    LYforce_HTML_mode = TRUE;	/* force this file to be HTML */
    LYforce_no_cache = TRUE;	/* force this file to be new */


    fprintf(fp0, "<head>\n");
    LYAddMETAcharsetToFD(fp0, -1);
    if (strchr(HTLoadedDocumentURL(), '"') == NULL) {
	/*
	 *  Insert a BASE tag so there is some way to relate the List Page
	 *  file to its underlying document after we are done.	It won't
	 *  be actually used for resolving relative URLs. - kw
	 */
	StrAllocCopy(Address, HTLoadedDocumentURL());
	LYEntify(&Address, FALSE);
	fprintf(fp0, "<base href=\"%s\">\n", Address);
	FREE(Address);
    }
    fprintf(fp0, "<title>%s</title>\n</head>\n<body>\n",
		 LIST_PAGE_TITLE);
    fprintf(fp0, "<h1>You have reached the List Page</h1>\n");
    fprintf(fp0, "<h2>%s Version %s</h2>\n", LYNX_NAME, LYNX_VERSION);
    StrAllocCopy(Address, HTLoadedDocumentURL());
    LYEntify(&Address, FALSE);
    fprintf(fp0,
	    "  References in %s<p>\n",
	    ((Address != NULL && *Address != '\0') ?
					   Address : "this document:"));
    FREE(Address);
    if (refs > 0) {
	fprintf(fp0, "<%s compact>\n", ((keypad_mode == NUMBERS_AS_ARROWS) ?
				       "ol" : "ul"));
	if (hidden_links > 0)
	    fprintf(fp0, "<lh><em>Visible links:</em>\n");
    }
    if (hidden_links > 0) {
	if (LYHiddenLinks == HIDDENLINKS_IGNORE)
	    hidden_links = 0;
    }
    for (cnt = 1; cnt <= refs; cnt++) {
	HTChildAnchor *child = HText_childNumber(cnt);
	HTAnchor *dest_intl = NULL;
	HTAnchor *dest;
	HTParentAnchor *parent;
	char *address;
	CONST char *title;

	if (child == 0) {
	    /*
	     *	child should not be 0 unless form field numbering is on
	     *	and cnt is the number of a form input field.
	     *	HText_FormDescNumber() will set desc to a description
	     *	of what type of input field this is.  We'll list it to
	     *	ensure that the link numbers on the list page match the
	     *	numbering in the original document, but won't create a
	     *	forward link to the form. - FM && LE
	     *
	     *	Changed to create a fake hidden link, to get the numbering
	     *	right in connection with always treating this file as
	     *	HIDDENLINKS_MERGE in GridText.c - kw
	     */
	    if (keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED) {
		HText_FormDescNumber(cnt, (char **)&desc);
		fprintf(fp0,
		"<li><a id=%d href=\"#%d\">form field</a> = <em>%s</em>\n",
			cnt, cnt, desc);
	    }
	    continue;
	}
#ifndef DONT_TRACK_INTERNAL_LINKS
	dest_intl = HTAnchor_followTypedLink((HTAnchor *)child,
						       LINK_INTERNAL);
#endif
	dest = dest_intl ?
	    dest_intl : HTAnchor_followMainLink((HTAnchor *)child);
	parent = HTAnchor_parent(dest);
	if (!intern_w_post && dest_intl &&
	    HTMainAnchor && HTMainAnchor->post_data &&
	    parent->post_data &&
	    !strcmp(HTMainAnchor->post_data, parent->post_data)) {
	    /*
	     *	Set flag to note that we had at least one internal link,
	     *	if the document from which we are generating the list
	     *	has assosiated POST data; after an extra check that the
	     *	link destination really has hthe same POST data so that
	     *	we can believe it is an internal link.
	     */
	    intern_w_post = TRUE;
	}
	address =  HTAnchor_address(dest);
	title = titles ? HTAnchor_title(parent) : NULL;
	StrAllocCopy(Address, address);
	FREE(address);
	LYEntify(&Address, TRUE);
	if (title && *title) {
	    StrAllocCopy(Title, title);
	    LYEntify(&Title, TRUE);
	    if (*Title) {
		cp = strchr(Address, '#');
	    } else {
		FREE(Title);
	    }
	}

	fprintf(fp0, "<li><a href=\"%s\"%s>%s%s%s%s%s</a>\n", Address,
			dest_intl ? " TYPE=\"internal link\"" : "",
			dest_intl ? "(internal) " : "",
			((HTAnchor*)parent != dest) && Title ? "in " : "",
			(char *)(Title ? Title : Address),
			(Title && cp) ? " - " : "",
			(Title && cp) ? (cp+1) : "");

	FREE(Address);
	FREE(Title);
    }

    if (hidden_links > 0) {
	if (refs > 0)
	    fprintf(fp0, "\n</%s>\n\n<p>\n",
			 ((keypad_mode == NUMBERS_AS_ARROWS) ?
							"ol" : "ul"));
	fprintf(fp0, "<%s compact>\n", ((keypad_mode == NUMBERS_AS_ARROWS) ?
					"ol continue" : "ul"));
	fprintf(fp0, "<lh><em>Hidden links:</em>\n");
    }

    for (cnt = 0; cnt < hidden_links; cnt++) {
	StrAllocCopy(Address, HText_HiddenLinkAt(HTMainText, cnt));
	LYEntify(&Address, FALSE);
	if (!(Address && *Address)) {
	    FREE(Address);
	    continue;
	}
	fprintf(fp0, "<li><a href=\"%s\">%s</a>\n", Address, Address);

	FREE(Address);
    }

    fprintf(fp0,"\n</%s>\n</body>\n", ((keypad_mode == NUMBERS_AS_ARROWS) ?
				       "ol" : "ul"));

    /*
     *	Make necessary changes to newdoc before returning to caller.
     *	If the intern_w_post flag is set, we keep the POST data in
     *	newdoc that have been passed in.  They should be the same as
     *	in the loaded locument for which we generated the list.
     *	In that case the file we have written will be associated with
     *	the same POST data when it is loaded after we are done here,
     *	so that following one of the links we have marked as "internal
     *	link" can lead back to the underlying document with the right
     *	address+post_data combination. - kw
     */
    if (intern_w_post) {
	newdoc->internal_link = TRUE;
    } else {
	FREE(newdoc->post_data);
	FREE(newdoc->post_content_type);
	newdoc->internal_link = FALSE;
    }
    newdoc->isHEAD = FALSE;
    newdoc->safe = FALSE;
    LYCloseTempFP(fp0);
    return(0);
}

/*	printlist - F.Macrides (macrides@sci.wfeb.edu)
**	---------
**	Print a text/plain list of HyperText References
**	in the current document.
**
**  On entry
**	titles		Set:	if we want titles where available
**			Clear:	we only get addresses.
*/
PUBLIC void printlist ARGS2(
	FILE *, 	fp,
	BOOLEAN,	titles)
{
#ifdef VMS
    extern BOOLEAN HadVMSInterrupt;
#endif /* VMS */
    int cnt;
    int refs, hidden_links;
    char *address = NULL;
    char *desc = "unknown field or link";

    refs = HText_sourceAnchors(HTMainText);
    if (refs <= 0 && LYHiddenLinks != HIDDENLINKS_SEPARATE)
	return;
    hidden_links = HText_HiddenLinkCount(HTMainText);
    if (refs <= 0 && hidden_links <= 0) {
	return;
    } else {
	fprintf(fp, "\n%s\n\n", "References");
	if (hidden_links > 0) {
	    fprintf(fp, "   Visible links:\n");
	    if (LYHiddenLinks == HIDDENLINKS_IGNORE)
		hidden_links = 0;
	}
	for (cnt = 1; cnt <= refs; cnt++) {
	    HTChildAnchor *child = HText_childNumber(cnt);
	    HTAnchor *dest;
	    HTParentAnchor *parent;
	    CONST char *title;

	    if (child == 0) {
		/*
		 *  child should not be 0 unless form field numbering is on
		 *  and cnt is the number of a form intput field.
		 *  HText_FormDescNumber() will set desc to a description
		 *  of what type of input field this is.  We'll create a
		 *  within-document link to ensure that the link numbers on
		 *  the list page match the numbering in the original document,
		 *  but won't create a forward link to the form. - FM && LE
		 */
		if (keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED) {
		    HText_FormDescNumber(cnt, (char **)&desc);
		    fprintf(fp, "%4d. form field = %s\n", cnt, desc);
		}
		continue;
	    }
	    dest = HTAnchor_followMainLink((HTAnchor *)child);
	    /*
	     *	Ignore if child anchor points to itself, i.e. we had
	     *	something like <A NAME=xyz HREF="#xyz"> and it is not
	     *	treated as a hidden link.  Useful if someone 'P'rints
	     *	the List Page (which isn't a very useful action to do,
	     *	but anyway...) - kw
	     */
	    if (dest == (HTAnchor *)child)
		continue;
	    parent = HTAnchor_parent(dest);
	    title = titles ? HTAnchor_title(parent) : NULL;
	    address =  HTAnchor_address(dest);
	    fprintf(fp, "%4d. %s%s\n", cnt,
		    ((HTAnchor*)parent != dest) && title ? "in " : "",
		    (title ? title : address));
	    FREE(address);
#ifdef VMS
	    if (HadVMSInterrupt)
		break;
#endif /* VMS */
	}

	if (hidden_links > 0)
	    fprintf(fp, "%s   Hidden links:\n", ((refs > 0) ? "\n" : ""));
	for (cnt = 0; cnt < hidden_links; cnt++) {
	    StrAllocCopy(address, HText_HiddenLinkAt(HTMainText, cnt));
	    if (!(address && *address)) {
		FREE(address);
		continue;
	    }
	    fprintf(fp, "%4d. %s\n", ((cnt + 1) + refs), address);
	    FREE(address);
#ifdef VMS
	    if (HadVMSInterrupt)
		break;
#endif /* VMS */
	}
    }
    return;
}
