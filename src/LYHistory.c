#include "HTUtils.h"
#include "tcp.h"
#include "HTAlert.h"
#include "HText.h"
#include "LYUtils.h"
#include "LYHistory.h"
#include "LYPrint.h"
#include "LYDownload.h"
#include "LYGlobalDefs.h"
#include "LYSignal.h"
#include "LYStrings.h"
#include "LYCharUtils.h"

#ifdef DIRED_SUPPORT
#include "LYUpload.h"
#include "LYLocal.h"
#endif /* DIRED_SUPPORT */

#include "LYexit.h"
#include "LYLeaks.h"
 
#define FREE(x) if (x) {free(x); x = NULL;}

/*
 *  Push the current filename, link and line number onto the history list.
 */
PUBLIC void LYpush ARGS1(document *,doc)
{

    if( *doc->address == '\0')  /* dont push null file names */
	return;

    /* don't push the history, download, or printer lists */
    if(!strcmp(doc->title, HISTORY_PAGE_TITLE) ||
		!strcmp(doc->title, PRINT_OPTIONS_TITLE) ||
		!strcmp(doc->title, DOWNLOAD_OPTIONS_TITLE) ) {
	if (!LYforce_no_cache)
	    LYoverride_no_cache = TRUE;
	return;
    }

#ifdef DIRED_SUPPORT
    if(!strcmp(doc->title, DIRED_MENU_TITLE) ||
		!strcmp(doc->title, UPLOAD_OPTIONS_TITLE) ||
                !strcmp(doc->title, PERMIT_OPTIONS_TITLE)) {
	if (!LYforce_no_cache)
	    LYoverride_no_cache = TRUE;
	return;
    }
#endif /* DIRED_SUPPORT */

    if(nhist>1 && STREQ(history[nhist-1].address, doc->address) &&
       !strcmp(history[nhist-1].post_data ? history[nhist-1].post_data : "",
               doc->post_data ? doc->post_data : "") &&
	       history[nhist-1].isHEAD == doc->isHEAD) 
        return;  /* file is identical to one before it don't push it */

    if(nhist>2 && STREQ(history[nhist-2].address, doc->address) &&
       !strcmp(history[nhist-2].post_data ? history[nhist-2].post_data : "",
               doc->post_data ? doc->post_data : "") &&
	       history[nhist-2].isHEAD == doc->isHEAD) {
	  nhist--; /* pop one off the stack */
          return;  /* file is identical to one two before it don't push it */
    }

    if (nhist<MAXHIST)  {
	history[nhist].link = doc->link;
	history[nhist].page = doc->line;
	history[nhist].title = NULL;
	StrAllocCopy(history[nhist].title, doc->title);
	history[nhist].address = NULL;
	StrAllocCopy(history[nhist].address, doc->address);
	history[nhist].post_data = NULL;
	StrAllocCopy(history[nhist].post_data, doc->post_data);
	history[nhist].post_content_type = NULL;
	StrAllocCopy(history[nhist].post_content_type, doc->post_content_type);
	history[nhist].bookmark = NULL;
	StrAllocCopy(history[nhist].bookmark, doc->bookmark);
	history[nhist].isHEAD = doc->isHEAD;
	nhist++;

        if(TRACE)
    	    fprintf(stderr,"\nLYpush: address:%s\n        title:%s\n",
						doc->address,doc->title);
    }
}

/*
 *  Pop the previous filename, link and line number from the history list.
 */
PUBLIC void LYpop ARGS1(document *,doc)
{
 
    if (nhist>0) {
	nhist--;
	doc->link = history[nhist].link;
	doc->line = history[nhist].page;
	FREE(doc->title);
	doc->title = history[nhist].title;    /* will be freed later */
	FREE(doc->address);
	doc->address = history[nhist].address;  /* will be freed later */
	FREE(doc->post_data);
	doc->post_data = history[nhist].post_data;
	FREE(doc->post_content_type);
	doc->post_content_type = history[nhist].post_content_type;
	FREE(doc->bookmark);
	doc->bookmark = history[nhist].bookmark;
	doc->isHEAD = history[nhist].isHEAD;

        if(TRACE)
	    fprintf(stderr,"LYpop: address:%s\n     title:%s\n",
						doc->address,doc->title);

    }
}

/*
 *  Pop the specified hist entry, link and line number from the history
 *  list but don't actually remove the entry, just return it.
 *  (This procedure is badly named :)
 */
PUBLIC void LYpop_num ARGS2(int,number, document *,doc)
{
    if (number >= 0 && nhist >= number) {
	doc->link = history[number].link;
	doc->line = history[number].page;
	StrAllocCopy(doc->title, history[number].title);
	StrAllocCopy(doc->address, history[number].address);
	StrAllocCopy(doc->post_data, history[number].post_data);
	StrAllocCopy(doc->post_content_type, history[number].post_content_type);
	StrAllocCopy(doc->bookmark, history[number].bookmark);
	doc->isHEAD = history[number].isHEAD;
    }
}

/*
 *  This procedure outputs the history buffer into a temporary file.
 *  
 */
PUBLIC int showhistory ARGS1(char **,newfile)
{
    static char tempfile[256];
    static BOOLEAN first = TRUE;
    static char hist_filename[256];
    char *Title = NULL;
    int x = 0;
    FILE *fp0;

    if (first) {
	tempname(tempfile, NEW_FILE);
	first = FALSE;
#ifdef VMS
    } else {
	remove(tempfile);  /* Remove duplicates on VMS. */
#endif /* VMS */
    }

    if ((fp0 = fopen(tempfile,"w")) == NULL) {
	HTAlert(CANNOT_OPEN_TEMP);
	return(-1);
    }

    /*
     *  Make the file a URL now.
     */
#ifdef VMS
    sprintf(hist_filename,"file://localhost/%s", tempfile);
#else
    sprintf(hist_filename,"file://localhost%s", tempfile);
#endif /* VMS */
    StrAllocCopy(*newfile, hist_filename);
    LYforce_HTML_mode = TRUE;	/* force this file to be HTML */
    LYforce_no_cache = TRUE;	/* force this file to be new */

    fprintf(fp0, "<head>\n<title>%s</title>\n</head>\n<body>\n",
		 HISTORY_PAGE_TITLE);

    fprintf(fp0, "<h1>You have reached the History Page</h1>\n");
    fprintf(fp0, "<h2>%s Version %s</h2>\n<pre>", LYNX_NAME, LYNX_VERSION);
    fprintf(fp0, "<em>You selected:</em>\n");
    for (x = nhist-1; x >= 0; x--) {
	/*
	 *  The number of the document in the hist stack,
	 *  its name in a link, and its address. - FM
	 */
	if (history[x].title != NULL) {
	    StrAllocCopy(Title, history[x].title);
	    LYEntify(&Title, TRUE);
	} else {
	    StrAllocCopy(Title, "(no title)");
	}
	fprintf(fp0,
		"%s<em>%d</em>. <tab id=t%d><a href=\"LYNXHIST:%d\">%s</a>\n",
		(x > 99 ? "" : x < 9 ? "  " : " "),  
		x, x, x, Title);
	if (history[x].address != NULL) {
	    StrAllocCopy(Title, history[x].address);
	    LYEntify(&Title, TRUE);
	} else {
	    StrAllocCopy(Title, "(no address)");
	}
	fprintf(fp0, "<tab to=t%d>%s\n", x, Title);
    }

    fprintf(fp0,"</pre>\n</body>\n");

    fclose(fp0);
    FREE(Title);
    return(0);
}

/* 
 *  This is a kludge to make the history page seem like any other type of
 *  file since more info is needed than can be provided by the normal link
 *  structure.  I saved out the history number to a special URL.  The info
 *  looks like:  LYNXHIST:#
 */
PUBLIC void historytarget ARGS1(document *,newdoc)
{
    int number, c;
    DocAddress WWWDoc;

    if ((!newdoc || !newdoc->address) ||
        strlen(newdoc->address) < 10 || !isdigit(*(newdoc->address+9)))
	return;

    if ((number = atoi(newdoc->address+9)) > nhist || number < 0)
        return;

    LYpop_num(number, newdoc);

    /*
     *  If we have POST content, aren't forcing no_cache,
     *  and still have the text cached, ask the user whether
     *  to resubmit the form. - FM
     */
    if (newdoc->post_data != NULL && LYforce_no_cache == FALSE) {
        if (LYresubmit_posts == TRUE) {
	    LYoverride_no_cache = FALSE;
	} else {
	    WWWDoc.address = newdoc->address;
            WWWDoc.post_data = newdoc->post_data;
            WWWDoc.post_content_type = newdoc->post_content_type;
            WWWDoc.bookmark = newdoc->bookmark;
	    WWWDoc.isHEAD = newdoc->isHEAD;
	    if ((HText *)HTAnchor_document(
				HTAnchor_parent(HTAnchor_findAddress(&WWWDoc))
					  )) {
		LYoverride_no_cache = TRUE;
		_statusline(
		   "Document from Form with POST content.  Resubmit? (y/n) ");
		c = LYgetch();
		if (TOUPPER(c) == 'Y') {
		    LYoverride_no_cache = FALSE;
		}
	    }
	}
    }

    if (number != 0)
	StrAllocCat(newdoc->title," (From History)");
}
