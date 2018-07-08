/*
 * $LynxId: LYMainLoop.c,v 1.241 2018/06/10 20:44:10 tom Exp $
 */
#include <HTUtils.h>
#include <HTAccess.h>
#include <HTParse.h>
#include <HTList.h>
#include <HTML.h>
#include <HTFTP.h>
#include <HTFile.h>
#include <HTTP.h>
#include <HTAABrow.h>
#include <HTNews.h>
#include <LYCurses.h>
#include <LYStyle.h>
#include <LYGlobalDefs.h>
#include <HTAlert.h>
#include <LYUtils.h>
#include <GridText.h>
#include <LYStrings.h>
#include <LYOptions.h>
#include <LYSignal.h>
#include <LYGetFile.h>
#include <HTForms.h>
#include <LYSearch.h>
#include <LYClean.h>
#include <LYHistory.h>
#include <LYPrint.h>
#include <LYMail.h>
#include <LYEdit.h>
#include <LYShowInfo.h>
#include <LYBookmark.h>
#include <LYKeymap.h>
#include <LYJump.h>
#include <LYDownload.h>
#include <LYList.h>
#include <LYMap.h>
#include <LYTraversal.h>
#include <LYCharSets.h>
#include <LYCharUtils.h>
#include <LYCookie.h>
#include <LYMainLoop.h>
#include <LYPrettySrc.h>

#ifdef USE_SESSIONS
#include <LYSession.h>
#endif

#ifdef KANJI_CODE_OVERRIDE
#include <HTCJK.h>
#endif

#define LinkIsTextarea(linkNumber) \
		(links[linkNumber].type == WWW_FORM_LINK_TYPE && \
		 links[linkNumber].l_form->type == F_TEXTAREA_TYPE)

#define LinkIsTextLike(linkNumber) \
	     (links[linkNumber].type == WWW_FORM_LINK_TYPE && \
	      F_TEXTLIKE(links[linkNumber].l_form->type))

#ifdef KANJI_CODE_OVERRIDE
char *str_kcode(HTkcode code)
{
    char *p;
    static char buff[8];

    if (current_char_set == TRANSPARENT) {
	p = "THRU";
    } else if (!LYRawMode) {
	p = "RAW";
    } else {
	switch (code) {
	case NOKANJI:
	    p = "AUTO";
	    break;

	case EUC:
	    p = "EUC+";
	    break;

	case SJIS:
	    p = "SJIS";
	    break;

	case JIS:
	    p = " JIS";
	    break;

	default:
	    p = " ???";
	    break;
	}
    }

    if (no_table_center) {
	buff[0] = '!';
	strcpy(buff + 1, p);
    } else {
	strcpy(buff, p);
    }

    return buff;
}
#endif

#ifdef WIN_EX

static char *str_sjis(char *to, char *from)
{
    if (!LYRawMode) {
	strcpy(to, from);
#ifdef KANJI_CODE_OVERRIDE
    } else if (last_kcode == EUC) {
	EUC_TO_SJIS(from, to);
    } else if (last_kcode == SJIS) {
	strcpy(to, from);
#endif
    } else {
	TO_SJIS((unsigned char *) from, (unsigned char *) to);
    }
    return to;
}

static void set_ws_title(char *str)
{
    SetConsoleTitle(str);
}

#endif /* WIN_EX */

#if defined(USE_EXTERNALS) || defined(WIN_EX)
#include <LYExtern.h>
#endif

#ifdef __EMX__
#include <io.h>
#endif

#ifdef DIRED_SUPPORT
#include <LYLocal.h>
#include <LYUpload.h>
#endif /* DIRED_SUPPORT */

#include <LYexit.h>
#include <LYLeaks.h>

/* two constants: */
HTLinkType *HTInternalLink = 0;
HTAtom *WWW_SOURCE = 0;

#define NONINTERNAL_OR_PHYS_DIFFERENT(p,n) \
	((track_internal_links && \
	 (!curdoc.internal_link || are_phys_different(p,n))) || \
	are_different(p,n))

#define NO_INTERNAL_OR_DIFFERENT(c,n) \
	(track_internal_links || are_different(c,n))

static void exit_immediately_with_error_message(int state, int first_file);
static void status_link(char *curlink_name, int show_more, int show_indx);
static void show_main_statusline(const LinkInfo curlink, int for_what);
static void form_noviceline(int);
static int are_different(DocInfo *doc1, DocInfo *doc2);

static int are_phys_different(DocInfo *doc1, DocInfo *doc2);

#define FASTTAB

static int sametext(char *een,
		    char *twee)
{
    if (een && twee)
	return (strcmp(een, twee) == 0);
    return TRUE;
}

HTList *Goto_URLs = NULL;	/* List of Goto URLs */

char *LYRequestTitle = NULL;	/* newdoc.title in calls to getfile() */
char *LYRequestReferer = NULL;	/* Referer, may be set in getfile() */

static bstring *prev_target = NULL;

#ifdef DISP_PARTIAL
BOOLEAN display_partial = FALSE;	/* could be enabled in HText_new() */
int NumOfLines_partial = 0;	/* number of lines displayed in partial mode */
#endif

static int Newline = 0;
static DocInfo newdoc;
static DocInfo curdoc;
static char *traversal_host = NULL;
static char *traversal_link_to_add = NULL;
static char *owner_address = NULL;	/* Holds the responsible owner's address     */
static char *ownerS_address = NULL;	/* Holds owner's address during source fetch */

#ifdef TEXTFIELDS_MAY_NEED_ACTIVATION
static BOOL textinput_activated = FALSE;

#else
#define textinput_activated TRUE	/* a current text input is always active */
#endif
#ifdef INACTIVE_INPUT_STYLE_VH
BOOL textinput_redrawn = FALSE;

    /*must be public since used in LYhighlight(..) */
#endif

#ifdef LY_FIND_LEAKS
/*
 * Function for freeing allocated mainloop() variables.  - FM
 */
static void free_mainloop_variables(void)
{
    LYFreeDocInfo(&newdoc);
    LYFreeDocInfo(&curdoc);

#ifdef USE_COLOR_STYLE
    FREE(curdoc.style);
    FREE(newdoc.style);
#endif
    FREE(traversal_host);
    FREE(traversal_link_to_add);
    FREE(owner_address);
    FREE(ownerS_address);
#ifdef DIRED_SUPPORT
    clear_tags();
    reset_dired_menu();
#endif /* DIRED_SUPPORT */
    FREE(WWW_Download_File);	/* LYGetFile.c/HTFWriter.c */
    FREE(LYRequestReferer);

    return;
}
#endif /* LY_FIND_LEAKS */

#ifndef NO_LYNX_TRACE
static void TracelogOpenFailed(void)
{
    WWW_TraceFlag = FALSE;
    if (LYCursesON) {
	HTUserMsg(TRACELOG_OPEN_FAILED);
    } else {
	fprintf(stderr, "%s\n", TRACELOG_OPEN_FAILED);
	exit_immediately(EXIT_FAILURE);
    }
}

static BOOLEAN LYReopenTracelog(BOOLEAN *trace_flag_ptr)
{
    CTRACE((tfp, "\nTurning off TRACE for fetch of log.\n"));
    LYCloseTracelog();
    if ((LYTraceLogFP = LYAppendToTxtFile(LYTraceLogPath)) == NULL) {
	TracelogOpenFailed();
	return FALSE;
    }
    if (TRACE) {
	WWW_TraceFlag = FALSE;
	*trace_flag_ptr = TRUE;
    }
    return TRUE;
}

static void turn_trace_back_on(BOOLEAN *trace_flag_ptr)
{
    if (*trace_flag_ptr == TRUE) {
	WWW_TraceFlag = TRUE;
	*trace_flag_ptr = FALSE;
	fprintf(tfp, "Turning TRACE back on.\n\n");
    }
}
#else
#define LYReopenTracelog(flag) TRUE
#define turn_trace_back_on(flag)	/*nothing */
#endif /* NO_LYNX_TRACE */

FILE *TraceFP(void)
{
#ifndef NO_LYNX_TRACE
    if (LYTraceLogFP != 0) {
	return LYTraceLogFP;
    }
#endif /* NO_LYNX_TRACE */
    return stderr;
}

BOOLEAN LYOpenTraceLog(void)
{
#ifndef NO_LYNX_TRACE
    if (TRACE && LYUseTraceLog && LYTraceLogFP == NULL) {
	/*
	 * If we can't open it for writing, give up.  Otherwise, on VMS close
	 * it, delete it and any versions from previous sessions so they don't
	 * accumulate, and open it again.  - FM
	 */
	if ((LYTraceLogFP = LYNewTxtFile(LYTraceLogPath)) == NULL) {
	    TracelogOpenFailed();
	    return FALSE;
	}
#ifdef VMS
	LYCloseTracelog();
	HTSYS_remove(LYTraceLogPath);
	if ((LYTraceLogFP = LYNewTxtFile(LYTraceLogPath)) == NULL) {
	    TracelogOpenFailed();
	    return FALSE;
	}
#endif /* VMS */
	fflush(stdout);
	fflush(stderr);
	fprintf(tfp, "\t\t%s (%s)\n\n", LYNX_TRACELOG_TITLE, LYNX_VERSION);
	/*
	 * If TRACE is on, indicate whether the anonymous restrictions are set.
	 * - FM, LP, kw
	 *
	 * This is only a summary for convenience - it doesn't take the case of
	 * individual -restrictions= options into account.  - kw
	 */
	if (LYValidate) {
	    if (LYRestricted && had_restrictions_default) {
		CTRACE((tfp,
			"Validate and some anonymous restrictions are set.\n"));
	    } else if (had_restrictions_default) {
		CTRACE((tfp,
			"Validate restrictions set, restriction \"default\" was given.\n"));
	    } else if (LYRestricted) {
		CTRACE((tfp,
			"Validate restrictions set, additional anonymous restrictions ignored.\n"));
	    } else {
		CTRACE((tfp, "Validate restrictions are set.\n"));
	    }
	    /* But none of the above can actually happen, since there should
	     * never be a Trace Log with -validate.  If it appears in a log
	     * file something went wrong! */
	} else if (LYRestricted) {
	    if (had_restrictions_all) {
		CTRACE((tfp,
			"Anonymous restrictions set, restriction \"all\" was given.\n"));
	    } else {
		CTRACE((tfp, "Anonymous restrictions are set.\n"));
	    }
	} else if (had_restrictions_all && had_restrictions_default) {
	    CTRACE((tfp, "Restrictions \"all\" and \"default\" were given.\n"));
	} else if (had_restrictions_default) {
	    CTRACE((tfp, "Restriction \"default\" was given.\n"));
	} else if (had_restrictions_all) {
	    CTRACE((tfp, "\"all\" restrictions are set.\n"));
	}
    }
#endif /* NO_LYNX_TRACE */
    return TRUE;
}

void LYCloseTracelog(void)
{
#ifndef NO_LYNX_TRACE
    if (LYTraceLogFP != 0) {
	fflush(stdout);
	fflush(stderr);
	fclose(LYTraceLogFP);
	LYTraceLogFP = 0;
    }
#endif /* NO_LYNX_TRACE */
}

void handle_LYK_TRACE_TOGGLE(void)
{
#ifndef NO_LYNX_TRACE
    WWW_TraceFlag = (BOOLEAN) !WWW_TraceFlag;
    if (LYOpenTraceLog())
	HTUserMsg(WWW_TraceFlag ? TRACE_ON : TRACE_OFF);
#else
    HTUserMsg(TRACE_DISABLED);
#endif /* NO_LYNX_TRACE */
}

void LYSetNewline(int value)
{
    Newline = value;
}

#define LYSetNewline(value)	Newline = value

int LYGetNewline(void)
{
    return Newline;
}

#define LYGetNewline()		Newline

void LYChgNewline(int adjust)
{
    LYSetNewline(Newline + adjust);
}

#define LYChgNewline(adjust)	Newline += (adjust)

#ifdef USE_SOURCE_CACHE
static BOOLEAN from_source_cache = FALSE;

/*
 * Like HTreparse_document(), but also set the flag.
 */
static BOOLEAN reparse_document(void)
{
    BOOLEAN result;

    from_source_cache = TRUE;	/* set for LYMainLoop_pageDisplay() */
    if ((result = HTreparse_document()) != FALSE) {
	from_source_cache = TRUE;	/* set for mainloop refresh */
    } else {
	from_source_cache = FALSE;
    }
    return result;
}
#endif /* USE_SOURCE_CACHE */

/*
 * Prefer reparsing if we can, but reload if we must - to force regeneration
 * of the display.
 */
static BOOLEAN reparse_or_reload(int *cmd)
{
#ifdef USE_SOURCE_CACHE
    if (reparse_document()) {
	return FALSE;
    }
#endif
    *cmd = LYK_RELOAD;
    return TRUE;
}

/*
 * Functions for setting the current address
 */
static void set_address(DocInfo *doc, const char *address)
{
    StrAllocCopy(doc->address, address);
}

static void copy_address(DocInfo *dst, DocInfo *src)
{
    StrAllocCopy(dst->address, src->address);
}

static void free_address(DocInfo *doc)
{
    FREE(doc->address);
}

static void move_address(DocInfo *dst, DocInfo *src)
{
    copy_address(dst, src);
    free_address(src);
}

#ifdef DISP_PARTIAL
/*
 * This is for traversal call from within partial mode in LYUtils.c
 * and HTFormat.c  It simply calls HText_pageDisplay() but utilizes
 * LYMainLoop.c static variables to manage proper newline position
 * in case of #fragment
 */
BOOL LYMainLoop_pageDisplay(int line_num)
{
    const char *pound;
    int prev_newline = LYGetNewline();

    /*
     * Override Newline with a new value if user scrolled the document while
     * loading (in LYUtils.c).
     */
    LYSetNewline(line_num);

#ifdef USE_SOURCE_CACHE
    /*
     * reparse_document() acts on 'curdoc' which always on top of the
     * history stack: no need to resolve #fragment position since
     * we already know it (curdoc.line).
     * So bypass here. Sorry for possible confusion...
     */
    if (!from_source_cache)
#endif
	/*
	 * If the requested URL has the #fragment, and we are not popped
	 * from the history stack, and have not scrolled the document yet -
	 * we should calculate correct newline position for the fragment.
	 * (This is a bit suboptimal since HTFindPoundSelector() traverse
	 * anchors list each time, so we have a quadratic complexity
	 * and may load CPU in a worst case).
	 */
	if (display_partial
	    && newdoc.line == 1 && line_num == 1 && prev_newline == 1
	    && (pound = findPoundSelector(newdoc.address))
	    && *pound && *(pound + 1)) {
	    if (HTFindPoundSelector(pound + 1)) {
		/* HTFindPoundSelector will initialize www_search_result */
		LYSetNewline(www_search_result);
	    } else {
		LYSetNewline(prev_newline);	/* restore ??? */
		return NO;	/* no repaint */
	    }
	}

    HText_pageDisplay(LYGetNewline(), prev_target->str);
    return YES;
}
#endif /* DISP_PARTIAL */

static BOOL set_curdoc_link(int nextlink)
{
    BOOL result = FALSE;

    if (curdoc.link != nextlink
	&& nextlink >= 0
	&& nextlink < nlinks) {
	if (curdoc.link >= 0 && curdoc.link < nlinks) {
	    LYhighlight(FALSE, curdoc.link, prev_target->str);
	    result = TRUE;
	}
	curdoc.link = nextlink;
    }
    return result;
}

/*
 * Setup newdoc to jump to the given line.
 *
 * FIXME: prefer to also jump to the link given in a URL fragment, but the
 * interface of getfile() does not provide that ability yet.
 */
static void goto_line(int nextline)
{
    int n;
    int old_link = newdoc.link;

    newdoc.link = 0;
    for (n = 0; n < nlinks; ++n) {
	if (nextline == links[n].anchor_line_num + 1) {
	    CTRACE((tfp, "top_of_screen %d\n", HText_getTopOfScreen() + 1));
	    CTRACE((tfp, "goto_line(%d) -> link %d -> %d\n", nextline,
		    old_link, n));
	    newdoc.link = n;
	    break;
	}
    }
}

#ifdef USE_MOUSE
static void set_curdoc_link_by_mouse(int nextlink)
{
    if (set_curdoc_link(nextlink)) {
	LYhighlight(TRUE, nextlink, prev_target->str);
	LYmsec_delay(20);
    }
}
#else
#define set_curdoc_link_by_mouse(nextlink) set_curdoc_link(nextlink)
#endif

static int do_change_link(void)
{
#ifdef USE_MOUSE
    /* Is there a mouse-clicked link waiting? */
    int mouse_tmp = get_mouse_link();

    /* If yes, use it as the link */
    if (mouse_tmp != -1) {
	if (mouse_tmp < 0 || mouse_tmp >= nlinks) {
	    char *msgtmp = NULL;

	    HTSprintf0(&msgtmp,
		       gettext("Internal error: Invalid mouse link %d!"),
		       mouse_tmp);
	    HTAlert(msgtmp);
	    FREE(msgtmp);
	    return (-1);	/* indicates unexpected error */
	}
	set_curdoc_link_by_mouse(mouse_tmp);
    }
#endif /* USE_MOUSE */
    return (0);			/* indicates OK */
}

#ifdef DIRED_SUPPORT
#define DIRED_UNCACHE_1 if (LYAutoUncacheDirLists < 1) /*nothing*/ ;\
			else HTuncache_current_document()
#define DIRED_UNCACHE_2 if (LYAutoUncacheDirLists < 2) /*nothing*/ ;\
			else HTuncache_current_document()
#endif /* DIRED_SUPPORT */

static void do_check_goto_URL(bstring **user_input,
			      char **old_user_input,
			      BOOLEAN *force_load)
{
    static BOOLEAN always = TRUE;
    /* *INDENT-OFF* */
    static struct {
	const char *name;
	BOOLEAN *flag;
    } table[] = {
	{ STR_FILE_URL,		&no_file_url },
	{ STR_FILE_URL,		&no_goto_file },
	{ STR_LYNXEXEC,		&no_goto_lynxexec },
	{ STR_LYNXPROG,		&no_goto_lynxprog },
	{ STR_LYNXCGI,		&no_goto_lynxcgi },
	{ STR_CSO_URL,		&no_goto_cso },
	{ STR_FINGER_URL,	&no_goto_finger },
	{ STR_FTP_URL,		&no_goto_ftp },
	{ STR_GOPHER_URL,	&no_goto_gopher },
	{ STR_HTTP_URL,		&no_goto_http },
	{ STR_HTTPS_URL,	&no_goto_https },
	{ STR_MAILTO_URL,	&no_goto_mailto },
	{ STR_RLOGIN_URL,	&no_goto_rlogin },
	{ STR_TELNET_URL,	&no_goto_telnet },
	{ STR_TN3270_URL,	&no_goto_tn3270 },
	{ STR_WAIS_URL,		&no_goto_wais },
#ifndef DISABLE_BIBP
	{ STR_BIBP_URL,		&no_goto_bibp },
#endif
#ifndef DISABLE_NEWS
	{ STR_NEWS_URL,		&no_goto_news },
	{ STR_NNTP_URL,		&no_goto_nntp },
	{ STR_SNEWS_URL,	&no_goto_snews },
#endif
#ifdef EXEC_LINKS
	{ STR_LYNXEXEC,		&local_exec_on_local_files },
	{ STR_LYNXPROG,		&local_exec_on_local_files },
#endif /* EXEC_LINKS */
	{ STR_LYNXCFG,		&no_goto_configinfo },
	{ STR_LYNXCFLAGS,	&no_goto_configinfo },
	{ STR_LYNXCOOKIE,	&always },
#ifdef USE_CACHEJAR
	{ STR_LYNXCACHE,	&always },
#endif
	{ STR_LYNXDIRED,	&always },
	{ STR_LYNXDOWNLOAD,	&always },
	{ STR_LYNXOPTIONS,	&always },
	{ STR_LYNXPRINT,	&always },
    };
    /* *INDENT-ON* */

    unsigned n;
    BOOLEAN found = FALSE;

    /* allow going to anchors */
    if ((*user_input)->str[0] == '#') {
	if ((*user_input)->str[1] &&
	    HTFindPoundSelector((*user_input)->str + 1)) {
	    /* HTFindPoundSelector will initialize www_search_result,
	       so we do nothing else. */
	    HTAddGotoURL((*user_input)->str);
	    trimPoundSelector(curdoc.address);
	    StrAllocCat(curdoc.address, (*user_input)->str);
	}
    } else {
	/*
	 * If it's not a URL then make it one.
	 */
	StrAllocCopy(*old_user_input, (*user_input)->str);
	LYEnsureAbsoluteURL(old_user_input, "", TRUE);
	BStrCopy0((*user_input), *old_user_input);
	FREE(*old_user_input);

	for (n = 0; n < TABLESIZE(table); n++) {
	    if (*(table[n].flag)
		&& !StrNCmp((*user_input)->str,
			    table[n].name,
			    strlen(table[n].name))) {
		found = TRUE;
		HTUserMsg2(GOTO_XXXX_DISALLOWED, table[n].name);
		break;
	    }
	}
	if (found) {
	    ;
	} else if (LYValidate &&
		   !isHTTP_URL((*user_input)->str) &&
		   !isHTTPS_URL((*user_input)->str)) {
	    HTUserMsg(GOTO_NON_HTTP_DISALLOWED);

	} else {
	    set_address(&newdoc, (*user_input)->str);
	    newdoc.isHEAD = FALSE;
	    /*
	     * Might be an anchor in the same doc from a POST form.  If so,
	     * dont't free the content.  -- FM
	     */
	    if (are_different(&curdoc, &newdoc)) {
		/*
		 * Make a name for this new URL.
		 */
		StrAllocCopy(newdoc.title,
			     gettext("A URL specified by the user"));
		LYFreePostData(&newdoc);
		FREE(newdoc.bookmark);
		newdoc.safe = FALSE;
		newdoc.internal_link = FALSE;
		*force_load = TRUE;
#ifdef DIRED_SUPPORT
		if (lynx_edit_mode) {
		    DIRED_UNCACHE_2;
		}
#endif /* DIRED_SUPPORT */
	    }
	    LYUserSpecifiedURL = TRUE;
	    HTAddGotoURL(newdoc.address);
	}
    }
}

/* returns FALSE if user cancelled input or URL was invalid, TRUE otherwise */
static BOOL do_check_recall(int ch,
			    bstring **user_input,
			    char **old_user_input,
			    int URLTotal,
			    int *URLNum,
			    RecallType recall,
			    BOOLEAN *FirstURLRecall)
{
    char *cp;
    BOOL ret = FALSE;

    if (*old_user_input == 0)
	StrAllocCopy(*old_user_input, "");

    for (;;) {
#ifdef WIN_EX			/* 1998/10/11 (Sun) 10:41:05 */
	int len = (int) strlen((*user_input)->str);

	if (len >= 3) {
	    if (len < MAX_LINE - 1
		&& LYIsHtmlSep((*user_input)->str[len - 3])
		&& LYIsDosDrive((*user_input)->str + len - 2))
		LYAddPathSep0((*user_input)->str);

	} else if (len == 2 && (*user_input)->str[1] == ':') {
	    if (LYIsDosDrive((*user_input)->str)) {
		LYAddPathSep0((*user_input)->str);
	    } else {
		HTUserMsg2(WWW_ILLEGAL_URL_MESSAGE, (*user_input)->str);
		BStrCopy0((*user_input), *old_user_input);
		FREE(*old_user_input);
		ret = FALSE;
		break;
	    }
	}
#endif
	/*
	 * Get rid of leading spaces (and any other spaces).
	 */
	LYTrimAllStartfile((*user_input)->str);
	if (isBEmpty(*user_input) &&
	    !(recall && (ch == UPARROW_KEY || ch == DNARROW_KEY))) {
	    BStrCopy0((*user_input), *old_user_input);
	    FREE(*old_user_input);
	    HTInfoMsg(CANCELLED);
	    ret = FALSE;
	    break;
	}
	if (recall && ch == UPARROW_KEY) {
	    if (*FirstURLRecall) {
		/*
		 * Use last URL in the list.  - FM
		 */
		*FirstURLRecall = FALSE;
		*URLNum = 0;
	    } else {
		/*
		 * Go back to the previous URL in the list.  - FM
		 */
		*URLNum += 1;
	    }
	    if (*URLNum >= URLTotal)
		/*
		 * Roll around to the last URL in the list.  - FM
		 */
		*URLNum = 0;
	    if ((cp = (char *) HTList_objectAt(Goto_URLs,
					       *URLNum)) != NULL) {
		BStrCopy0((*user_input), cp);
		if (goto_buffer
		    && **old_user_input
		    && !strcmp(*old_user_input, (*user_input)->str)) {
		    _statusline(EDIT_CURRENT_GOTO);
		} else if ((goto_buffer && URLTotal == 2) ||
			   (!goto_buffer && URLTotal == 1)) {
		    _statusline(EDIT_THE_PREV_GOTO);
		} else {
		    _statusline(EDIT_A_PREV_GOTO);
		}
		if ((ch = LYgetBString(user_input, FALSE, 0, recall)) < 0) {
		    /*
		     * User cancelled the Goto via ^G.  Restore
		     * user_input and break.  - FM
		     */
		    BStrCopy0((*user_input), *old_user_input);
		    FREE(*old_user_input);
		    HTInfoMsg(CANCELLED);
		    ret = FALSE;
		    break;
		}
		continue;
	    }
	} else if (recall && ch == DNARROW_KEY) {
	    if (*FirstURLRecall) {
		/*
		 * Use the first URL in the list.  - FM
		 */
		*FirstURLRecall = FALSE;
		*URLNum = URLTotal - 1;
	    } else {
		/*
		 * Advance to the next URL in the list.  - FM
		 */
		*URLNum -= 1;
	    }
	    if (*URLNum < 0)
		/*
		 * Roll around to the first URL in the list.  - FM
		 */
		*URLNum = URLTotal - 1;
	    if ((cp = (char *) HTList_objectAt(Goto_URLs, *URLNum)) != NULL) {
		BStrCopy0((*user_input), cp);
		if (goto_buffer && **old_user_input &&
		    !strcmp(*old_user_input, (*user_input)->str)) {
		    _statusline(EDIT_CURRENT_GOTO);
		} else if ((goto_buffer && URLTotal == 2) ||
			   (!goto_buffer && URLTotal == 1)) {
		    _statusline(EDIT_THE_PREV_GOTO);
		} else {
		    _statusline(EDIT_A_PREV_GOTO);
		}
		if ((ch = LYgetBString(user_input, FALSE, 0, recall)) < 0) {
		    /*
		     * User cancelled the Goto via ^G.  Restore
		     * user_input and break.  - FM
		     */
		    BStrCopy0((*user_input), *old_user_input);
		    FREE(*old_user_input);
		    HTInfoMsg(CANCELLED);
		    ret = FALSE;
		    break;
		}
		continue;
	    }
	} else {
	    ret = TRUE;
	    break;
	}
    }
    return ret;
}

static void do_cleanup_after_delete(void)
{
    HTuncache_current_document();
    move_address(&newdoc, &curdoc);
    newdoc.line = curdoc.line;
    if (curdoc.link == nlinks - 1) {
	/*
	 * We deleted the last link on the page.  - FM
	 */
	newdoc.link = curdoc.link - 1;
    } else {
	newdoc.link = curdoc.link;
    }
}

static int find_link_near_col(int col,
			      int delta)
{
    int i;

    for (i = curdoc.link; delta > 0 ? (i < nlinks) : (i >= 0); i += delta) {
	if ((links[i].ly - links[curdoc.link].ly) * delta > 0) {
	    int cy = links[i].ly, best = -1, dist = 1000000;

	    while ((delta > 0 ? (i < nlinks) : (i >= 0)) && cy == links[i].ly) {
		int cx = links[i].lx;
		const char *text = LYGetHiliteStr(i, 0);

		if (text != NULL)
		    cx += (int) strlen(text) / 2;
		cx -= col;
		if (cx < 0)
		    cx = -cx;
		if (cx < dist) {
		    dist = cx;
		    best = i;
		}
		i += delta;
	    }
	    return (best);
	}
    }
    return (-1);
}

/*
 * This is a special feature to traverse every http link derived from startfile
 * and check for errors or create crawl output files.  Only URL's that begin
 * with "traversal_host" are searched - this keeps the search from crossing to
 * other servers (a feature, not a bug!).
 */
static int DoTraversal(int c,
		       BOOLEAN *crawl_ok)
{
    BOOLEAN rlink_rejected = FALSE;
    BOOLEAN rlink_exists;
    BOOLEAN rlink_allowed;

    rlink_exists = (BOOL) (nlinks > 0 &&
			   links[curdoc.link].type != WWW_FORM_LINK_TYPE &&
			   links[curdoc.link].lname != NULL);

    if (rlink_exists) {
	rlink_rejected = lookup_reject(links[curdoc.link].lname);
	if (!rlink_rejected &&
	    traversal_host &&
	    links[curdoc.link].lname) {
	    if (!isLYNXIMGMAP(links[curdoc.link].lname)) {
		rlink_allowed = (BOOL) !StrNCmp(traversal_host,
						links[curdoc.link].lname,
						strlen(traversal_host));
	    } else {
		rlink_allowed = (BOOL) !StrNCmp(traversal_host,
						links[curdoc.link].lname + LEN_LYNXIMGMAP,
						strlen(traversal_host));
	    }
	} else {
	    rlink_allowed = FALSE;
	}
    } else {
	rlink_allowed = FALSE;
    }
    if (rlink_exists && rlink_allowed) {
	if (lookup_link(links[curdoc.link].lname)) {
	    if (more_links ||
		(curdoc.link > -1 && curdoc.link < nlinks - 1)) {
		c = DNARROW_KEY;
	    } else {
		if (STREQ(curdoc.title, "Entry into main screen") ||
		    (nhist <= 0)) {
		    if (!dump_output_immediately) {
			cleanup();
			exit_immediately(EXIT_FAILURE);
		    }
		    c = -1;
		} else {
		    c = LTARROW_KEY;
		}
	    }
	} else {
	    StrAllocCopy(traversal_link_to_add,
			 links[curdoc.link].lname);
	    if (!isLYNXIMGMAP(traversal_link_to_add))
		*crawl_ok = TRUE;
	    c = RTARROW_KEY;
	}
    } else {			/* no good right link, so only down and left arrow ok */
	if (rlink_exists /* && !rlink_rejected */ )
	    /* uncomment in previous line to avoid duplicates - kw */
	    add_to_reject_list(links[curdoc.link].lname);
	if (more_links ||
	    (curdoc.link > -1 && curdoc.link < nlinks - 1)) {
	    c = DNARROW_KEY;
	} else {
	    /*
	     * curdoc.title doesn't always work, so bail out if the history
	     * list is empty.
	     */
	    if (STREQ(curdoc.title, "Entry into main screen") ||
		(nhist <= 0)) {
		if (!dump_output_immediately) {
		    cleanup();
		    exit_immediately(EXIT_FAILURE);
		}
		c = -1;
	    } else {
		c = LTARROW_KEY;
	    }
	}
    }
    CTRACE((tfp, "DoTraversal(%d:%d) -> %s\n",
	    nlinks > 0 ? curdoc.link : 0,
	    nlinks,
	    LYKeycodeToString(c, FALSE)));
    return c;
}

static BOOLEAN check_history(void)
{
    const char *base;

    if (!curdoc.post_data)
	/*
	 * Normal case - List Page is not associated with post data.  - kw
	 */
	return TRUE;

    if (nhist > 0
	&& !LYresubmit_posts
	&& HDOC(nhist - 1).post_data
	&& BINEQ(curdoc.post_data, HDOC(nhist - 1).post_data)
	&& (base = HText_getContentBase()) != 0) {
	char *text = !isLYNXIMGMAP(HDOC(nhist - 1).address)
	? HDOC(nhist - 1).address
	: HDOC(nhist - 1).address + LEN_LYNXIMGMAP;

	if (!StrNCmp(base, text, strlen(base))) {
	    /*
	     * Normal case - as best as we can check, the document at the top
	     * of the history stack seems to be the document the List Page is
	     * about (or a LYNXIMGMAP derived from it), and LYresubmit_posts is
	     * not set, so don't prompt here.  If we actually have to repeat a
	     * POST because, against expectations, the underlying document
	     * isn't cached any more, HTAccess will prompt for confirmation,
	     * unless we had LYK_NOCACHE -kw
	     */
	    return TRUE;
	}
    }
    return FALSE;
}

static int handle_LYK_ACTIVATE(int *c,
			       int cmd GCC_UNUSED,
			       BOOLEAN *try_internal GCC_UNUSED,
			       BOOLEAN *refresh_screen,
			       BOOLEAN *force_load,
			       int real_cmd)
{
    if (do_change_link() == -1) {
	LYforce_no_cache = FALSE;
	reloading = FALSE;
	return 1;		/* mouse stuff was confused, ignore - kw */
    }
    if (nlinks > 0) {
	if (links[curdoc.link].type == WWW_FORM_LINK_TYPE) {
#ifdef TEXTFIELDS_MAY_NEED_ACTIVATION
	    if (real_cmd == LYK_ACTIVATE && textfields_need_activation &&
		F_TEXTLIKE(links[curdoc.link].l_form->type)) {

		textinput_activated = TRUE;
		show_main_statusline(links[curdoc.link], FOR_INPUT);
		textfields_need_activation = textfields_activation_option;

		return 0;
	    }
#endif
	    /*
	     * Don't try to submit forms with bad actions.  - FM
	     */
	    if (links[curdoc.link].l_form->type == F_SUBMIT_TYPE ||
		links[curdoc.link].l_form->type == F_IMAGE_SUBMIT_TYPE ||
		links[curdoc.link].l_form->type == F_TEXT_SUBMIT_TYPE) {
		/*
		 * Do nothing if it's disabled.  - FM
		 */
		if (links[curdoc.link].l_form->disabled == YES) {
		    HTOutputFormat = WWW_PRESENT;
		    LYforce_no_cache = FALSE;
		    reloading = FALSE;
		    return 0;
		}
		/*
		 * Make sure we have an action.  - FM
		 */
		if (isEmpty(links[curdoc.link].l_form->submit_action)) {
		    HTUserMsg(NO_FORM_ACTION);
		    HTOutputFormat = WWW_PRESENT;
		    LYforce_no_cache = FALSE;
		    reloading = FALSE;
		    return 0;
		}
		/*
		 * Check for no_mail if the form action is a mailto URL.  - FM
		 */
		if (links[curdoc.link].l_form->submit_method
		    == URL_MAIL_METHOD && no_mail) {
		    HTAlert(FORM_MAILTO_DISALLOWED);
		    HTOutputFormat = WWW_PRESENT;
		    LYforce_no_cache = FALSE;
		    reloading = FALSE;
		    return 0;
		}
		/*
		 * Make sure this isn't a spoof in an account with restrictions
		 * on file URLs.  - FM
		 */
		if (no_file_url &&
		    isFILE_URL(links[curdoc.link].l_form->submit_action)) {
		    HTAlert(FILE_ACTIONS_DISALLOWED);
		    HTOutputFormat = WWW_PRESENT;
		    LYforce_no_cache = FALSE;
		    reloading = FALSE;
		    return 0;
		}
		/*
		 * Make sure this isn't a spoof attempt via an internal URL.  -
		 * FM
		 */
		if (isLYNXCOOKIE(links[curdoc.link].l_form->submit_action) ||
		    isLYNXCACHE(links[curdoc.link].l_form->submit_action) ||
#ifdef DIRED_SUPPORT
#ifdef OK_PERMIT
		    (isLYNXDIRED(links[curdoc.link].l_form->submit_action) &&
		     (no_dired_support ||
		      strncasecomp((links[curdoc.link].l_form->submit_action
				    + 10),
				   "//PERMIT_LOCATION", 17) ||
		      !LYIsUIPage(curdoc.address, UIP_PERMIT_OPTIONS))) ||
#else
		    isLYNXDIRED(links[curdoc.link].l_form->submit_action) ||
#endif /* OK_PERMIT */
#endif /* DIRED_SUPPORT */
		    isLYNXDOWNLOAD(links[curdoc.link].l_form->submit_action) ||
		    isLYNXHIST(links[curdoc.link].l_form->submit_action) ||
		    isLYNXEDITMAP(links[curdoc.link].l_form->submit_action) ||
		    isLYNXKEYMAP(links[curdoc.link].l_form->submit_action) ||
		    isLYNXIMGMAP(links[curdoc.link].l_form->submit_action) ||
		    isLYNXPRINT(links[curdoc.link].l_form->submit_action) ||
		    isLYNXEXEC(links[curdoc.link].l_form->submit_action) ||
		    isLYNXPROG(links[curdoc.link].l_form->submit_action)) {

		    HTAlert(SPECIAL_ACTION_DISALLOWED);
		    CTRACE((tfp, "LYMainLoop: Rejected '%s'\n",
			    links[curdoc.link].l_form->submit_action));
		    HTOutputFormat = WWW_PRESENT;
		    LYforce_no_cache = FALSE;
		    reloading = FALSE;
		    return 0;
		}
#ifdef NOTDEFINED		/* We're disabling form inputs instead of using this. - FM */
		/*
		 * Check for enctype and let user know we don't yet support
		 * multipart/form-data - FM
		 */
		if (links[curdoc.link].l_form->submit_enctype) {
		    if (!strcmp(links[curdoc.link].l_form->submit_enctype,
				"multipart/form-data")) {
			HTAlert(gettext("Enctype multipart/form-data not yet supported!  Cannot submit."));
			HTOutputFormat = WWW_PRESENT;
			LYforce_no_cache = FALSE;
			reloading = FALSE;
			return 0;
		    }
		}
#endif /* NOTDEFINED */
		if (check_realm) {
		    LYPermitURL = TRUE;
		}
		if (no_filereferer == TRUE && isFILE_URL(curdoc.address)) {
		    LYNoRefererForThis = TRUE;
		}
		if (links[curdoc.link].l_form->submit_method != URL_MAIL_METHOD) {
		    StrAllocCopy(newdoc.title,
				 LYGetHiliteStr(curdoc.link, 0));
		}
	    }

	    /*
	     * Normally we don't get here for text input fields, but it can
	     * happen as a result of mouse positioning.  In that case the
	     * statusline will not have updated info, so update it now.  - kw
	     */
	    if (F_TEXTLIKE(links[curdoc.link].l_form->type)) {
		show_formlink_statusline(links[curdoc.link].l_form,
					 (real_cmd == LYK_NOCACHE ||
					  real_cmd == LYK_DOWNLOAD ||
					  real_cmd == LYK_HEAD ||
					  (real_cmd == LYK_MOUSE_SUBMIT &&
					   !textinput_activated)) ?
					 FOR_PANEL : FOR_INPUT);
		if (user_mode == NOVICE_MODE &&
		    textinput_activated &&
		    (real_cmd == LYK_ACTIVATE ||
		     real_cmd == LYK_MOUSE_SUBMIT)) {
		    form_noviceline(FormIsReadonly(links[curdoc.link].l_form));
		}
	    }

	    *c = change_form_link(curdoc.link,
				  &newdoc, refresh_screen,
				  FALSE,
				  (real_cmd == LYK_MOUSE_SUBMIT ||
				   real_cmd == LYK_NOCACHE ||
				   real_cmd == LYK_DOWNLOAD ||
				   real_cmd == LYK_HEAD));
	    if (*c != LKC_DONE || *refresh_screen) {
		/*
		 * Cannot have been a submit field for which newdoc was filled
		 * in.  - kw
		 */
		if ((links[curdoc.link].l_form->type == F_SUBMIT_TYPE ||
		     links[curdoc.link].l_form->type == F_IMAGE_SUBMIT_TYPE ||
		     links[curdoc.link].l_form->type == F_TEXT_SUBMIT_TYPE) &&
		    links[curdoc.link].l_form->submit_method
		    != URL_MAIL_METHOD) {
		    /*
		     * Try to undo change of newdoc.title done above.
		     */
		    if (HText_getTitle()) {
			StrAllocCopy(newdoc.title, HText_getTitle());
		    } else if (curdoc.title) {
			StrAllocCopy(newdoc.title, curdoc.title);
		    }
		}
	    } else {
		if (HTOutputFormat == HTAtom_for("www/download") &&
		    newdoc.post_data != NULL &&
		    newdoc.safe == FALSE) {

		    if ((HText_POSTReplyLoaded(&newdoc) == TRUE) &&
			HTConfirm(CONFIRM_POST_RESUBMISSION) == FALSE) {
			HTInfoMsg(CANCELLED);
			HTOutputFormat = WWW_PRESENT;
			LYforce_no_cache = FALSE;
			copy_address(&newdoc, &curdoc);
			StrAllocCopy(newdoc.title, curdoc.title);
			BStrCopy(newdoc.post_data, curdoc.post_data);
			StrAllocCopy(newdoc.post_content_type,
				     curdoc.post_content_type);
			StrAllocCopy(newdoc.bookmark, curdoc.bookmark);
			newdoc.isHEAD = curdoc.isHEAD;
			newdoc.safe = curdoc.safe;
			newdoc.internal_link = curdoc.internal_link;
			return 0;
		    }
		}
		/*
		 * Moved here from earlier to only apply when it should.
		 * Anyway, why should realm checking be overridden for form
		 * submissions, this seems to be an unnecessary loophole??  But
		 * that's the way it was, maybe there is some reason.  However,
		 * at least make sure this doesn't weaken restrictions implied
		 * by -validate!
		 * - kw 1999-05-25
		 */
		if (check_realm && !LYValidate) {
		    LYPermitURL = TRUE;
		}
	    }
	    if (*c == LKC_DONE) {
		*c = DO_NOTHING;
	    } else if (*c == 23) {
		*c = DO_NOTHING;
		*refresh_screen = TRUE;
	    } else {
		/* Avoid getting stuck with repeatedly calling
		 * handle_LYK_ACTIVATE(), instead of calling change_form_link()
		 * directly from mainloop(), for text input fields.  - kw
		 */
		switch (LKC_TO_C(*c)) {
		case '\n':
		case '\r':
		default:
		    if ((real_cmd == LYK_ACTIVATE ||
			 real_cmd == LYK_MOUSE_SUBMIT) &&
			F_TEXTLIKE(links[curdoc.link].l_form->type) &&
			textinput_activated) {
			return 3;
		    }
		    break;
		}
	    }
	    return 2;
	} else {
	    /*
	     * Not a forms link.
	     *
	     * Make sure this isn't a spoof in an account with restrictions on
	     * file URLs.  - FM
	     */
	    if (no_file_url && isFILE_URL(links[curdoc.link].lname)) {
		if (!isFILE_URL(curdoc.address) &&
		    !((isLYNXEDITMAP(curdoc.address) ||
		       isLYNXKEYMAP(curdoc.address) ||
		       isLYNXCOOKIE(curdoc.address) ||
		       isLYNXCACHE(curdoc.address)) &&
		      !StrNCmp(links[curdoc.link].lname,
			       helpfilepath,
			       strlen(helpfilepath)))) {
		    HTAlert(FILE_SERVED_LINKS_DISALLOWED);
		    reloading = FALSE;
		    return 0;
		} else if (curdoc.bookmark != NULL) {
		    HTAlert(FILE_BOOKMARKS_DISALLOWED);
		    reloading = FALSE;
		    return 0;
		}
	    }
	    /*
	     * Make sure this isn't a spoof attempt via an internal URL in a
	     * non-internal document.  - FM
	     */
	    if ((isLYNXCOOKIE(links[curdoc.link].lname) &&
		 (strcmp(NonNull(curdoc.title), COOKIE_JAR_TITLE) ||
		  !isLYNXCOOKIE(curdoc.address))) ||
#ifdef USE_CACHEJAR
		(isLYNXCACHE(links[curdoc.link].lname) &&
		 (strcmp(NonNull(curdoc.title), CACHE_JAR_TITLE) ||
		  !isLYNXCACHE(curdoc.address))) ||
#endif
#ifdef DIRED_SUPPORT
		(isLYNXDIRED(links[curdoc.link].lname) &&
		 !LYIsUIPage(curdoc.address, UIP_DIRED_MENU) &&
		 !LYIsUIPage(curdoc.address, UIP_PERMIT_OPTIONS) &&
#ifdef OK_INSTALL
		 !LYIsUIPage(curdoc.address, UIP_INSTALL) &&
#endif /* OK_INSTALL */
		 !LYIsUIPage(curdoc.address, UIP_UPLOAD_OPTIONS)) ||
#endif /* DIRED_SUPPORT */
		(isLYNXDOWNLOAD(links[curdoc.link].lname) &&
		 !LYIsUIPage(curdoc.address, UIP_DOWNLOAD_OPTIONS)) ||
		(isLYNXHIST(links[curdoc.link].lname) &&
		 !LYIsUIPage(curdoc.address, UIP_HISTORY) &&
		 !LYIsUIPage(curdoc.address, UIP_LIST_PAGE) &&
		 !LYIsUIPage(curdoc.address, UIP_ADDRLIST_PAGE)) ||
		(isLYNXPRINT(links[curdoc.link].lname) &&
		 !LYIsUIPage(curdoc.address, UIP_PRINT_OPTIONS))) {
		HTAlert(SPECIAL_VIA_EXTERNAL_DISALLOWED);
		HTOutputFormat = WWW_PRESENT;
		LYforce_no_cache = FALSE;
		reloading = FALSE;
		return 0;
	    }
#ifdef USE_EXTERNALS
	    if (run_external(links[curdoc.link].lname, TRUE)) {
		*refresh_screen = TRUE;
		return 0;
	    }
#endif /* USE_EXTERNALS */

	    /*
	     * Follow a normal link or anchor.
	     */
	    set_address(&newdoc, links[curdoc.link].lname);
	    StrAllocCopy(newdoc.title, LYGetHiliteStr(curdoc.link, 0));
	    /*
	     * For internal links, retain POST content if present.  If we are
	     * on the List Page, prevent pushing it on the history stack.
	     * Otherwise set try_internal to signal that the top of the loop
	     * should attempt to reposition directly, without calling getfile.
	     * - kw
	     */
	    if (track_internal_links) {
		/*
		 * Might be an internal link anchor in the same doc.  If so, take
		 * the try_internal shortcut if we didn't fall through from
		 * LYK_NOCACHE.  - kw
		 */
		newdoc.internal_link =
		    (links[curdoc.link].type == WWW_INTERN_LINK_TYPE);
		if (newdoc.internal_link) {
		    /*
		     * Special case of List Page document with an internal link
		     * indication, which may really stand for an internal link
		     * within the document the List Page is about.  - kw
		     */
		    if (LYIsListpageTitle(NonNull(curdoc.title)) &&
			(LYIsUIPage(curdoc.address, UIP_LIST_PAGE) ||
			 LYIsUIPage(curdoc.address, UIP_ADDRLIST_PAGE))) {
			if (check_history()) {
			    LYinternal_flag = TRUE;
			} else {
			    HTLastConfirmCancelled();	/* reset flag */
			    if (!confirm_post_resub(newdoc.address,
						    newdoc.title,
						    ((LYresubmit_posts &&
						      HText_POSTReplyLoaded(&newdoc))
						     ? 1
						     : 2),
						    2)) {
				if (HTLastConfirmCancelled() ||
				    (LYresubmit_posts &&
				     cmd != LYK_NOCACHE &&
				     !HText_POSTReplyLoaded(&newdoc))) {
				    /* cancel the whole thing */
				    LYforce_no_cache = FALSE;
				    reloading = FALSE;
				    copy_address(&newdoc, &curdoc);
				    StrAllocCopy(newdoc.title, curdoc.title);
				    newdoc.internal_link = curdoc.internal_link;
				    HTInfoMsg(CANCELLED);
				    return 1;
				} else if (LYresubmit_posts &&
					   cmd != LYK_NOCACHE) {
				    /* If LYresubmit_posts is set, and the
				       answer was No, and the key wasn't
				       NOCACHE, and we have a cached copy,
				       then use it. - kw */
				    LYforce_no_cache = FALSE;
				} else {
				    /* if No, but not ^C or ^G, drop
				     * the post data.  Maybe the link
				     * wasn't meant to be internal after
				     * all, here we can recover from that
				     * assumption. - kw */
				    LYFreePostData(&newdoc);
				    newdoc.internal_link = FALSE;
				    HTAlert(DISCARDING_POST_DATA);
				}
			    }
			}
			/*
			 * Don't push the List Page if we follow an internal link
			 * given by it.  - kw
			 */
			free_address(&curdoc);
		    } else if (cmd != LYK_NOCACHE) {
			*try_internal = TRUE;
		    }
		    if (!(LYresubmit_posts && newdoc.post_data))
			LYinternal_flag = TRUE;
		    /* We still set force_load so that history pushing
		     * etc. will be done.  - kw
		     */
		    *force_load = TRUE;
		    return 1;
		} else {
		    /*
		     * Free POST content if not an internal link.  - kw
		     */
		    LYFreePostData(&newdoc);
		}
	    }
	    /*
	     * Might be an anchor in the same doc from a POST form.  If so,
	     * don't free the content.  -- FM
	     */
	    if (are_different(&curdoc, &newdoc)) {
		LYFreePostData(&newdoc);
		FREE(newdoc.bookmark);
		if (isLYNXMESSAGES(newdoc.address))
		    LYforce_no_cache = TRUE;
	    }
	    if (!no_jump && lynxjumpfile && curdoc.address &&
		!strcmp(lynxjumpfile, curdoc.address)) {
		LYJumpFileURL = TRUE;
		LYUserSpecifiedURL = TRUE;
	    } else if ((curdoc.title &&
			(LYIsUIPage(curdoc.address, UIP_HISTORY) ||
			 !strcmp(curdoc.title, HISTORY_PAGE_TITLE))) ||
		       curdoc.bookmark != NULL ||
		       (lynxjumpfile &&
			curdoc.address &&
			!strcmp(lynxjumpfile, curdoc.address))) {
		LYUserSpecifiedURL = TRUE;
	    } else if (no_filereferer == TRUE &&
		       curdoc.address != NULL &&
		       isFILE_URL(curdoc.address)) {
		LYNoRefererForThis = TRUE;
	    }
	    newdoc.link = 0;
	    *force_load = TRUE;	/* force MainLoop to reload */
#ifdef USE_PRETTYSRC
	    psrc_view = FALSE;	/* we get here if link is not internal */
#endif

#if defined(DIRED_SUPPORT) && !defined(__DJGPP__)
	    if (lynx_edit_mode) {
		DIRED_UNCACHE_2;
		/*
		 * Unescaping any slash chars in the URL, but avoid double
		 * unescaping and too-early unescaping of other chars.  - KW
		 */
		HTUnEscapeSome(newdoc.address, "/");
		/* avoid stripping final slash for root dir - kw */
		if (strcasecomp(newdoc.address, "file://localhost/"))
		    strip_trailing_slash(newdoc.address);
	    }
#endif /* DIRED_SUPPORT  && !__DJGPP__ */
	    if (isLYNXCOOKIE(curdoc.address)
		|| isLYNXCACHE(curdoc.address)) {
		HTuncache_current_document();
	    }
	}
    }
    return 0;
}
/*
 * If the given form link does not point to the requested type, search for
 * the first link belonging to the form which does.  If there are none,
 * return null.
 */
#define SameFormAction(form,submit) \
 	((submit) \
	 ? (F_SUBMITLIKE((form)->type)) \
	 : ((form)->type == F_RESET_TYPE))

static FormInfo *FindFormAction(FormInfo * given, int submit)
{
    FormInfo *result = NULL;
    FormInfo *fi;
    int i;

    if (given == NULL) {
	HTAlert(LINK_NOT_IN_FORM);
    } else if (SameFormAction(given, submit)) {
	result = given;
    } else {
	for (i = 0; i < nlinks; i++) {
	    if ((fi = links[i].l_form) != 0 &&
		fi->number == given->number &&
		(SameFormAction(fi, submit))) {
		result = fi;
		break;
	    }
	}
    }
    return result;
}

static FormInfo *MakeFormAction(FormInfo * given, int submit)
{
    FormInfo *result = 0;

    if (given != 0) {
	result = typecalloc(FormInfo);

	if (result == NULL)
	    outofmem(__FILE__, "MakeFormAction");

	*result = *given;
	if (submit) {
	    if (result->submit_action == 0) {
		PerFormInfo *pfi = HText_PerFormInfo(result->number);

		*result = pfi->data;
	    }
	    result->type = F_SUBMIT_TYPE;
	} else {
	    result->type = F_RESET_TYPE;
	}
	result->number = given->number;
    }
    return result;
}

static void handle_LYK_SUBMIT(int cur, DocInfo *doc, BOOLEAN *refresh_screen)
{
    FormInfo *form = FindFormAction(links[cur].l_form, 1);
    FormInfo *make = NULL;
    char *save_submit_action = NULL;

    if (form == 0) {
	make = MakeFormAction(links[cur].l_form, 1);
	form = make;
    }

    if (form != 0) {
	StrAllocCopy(save_submit_action, form->submit_action);
	form->submit_action = HTPrompt(EDIT_SUBMIT_URL, form->submit_action);

	if (isEmpty(form->submit_action) ||
	    (!isLYNXCGI(form->submit_action) &&
	     StrNCmp(form->submit_action, "http", 4))) {
	    HTUserMsg(FORM_ACTION_NOT_HTTP_URL);
	} else {
	    HTInfoMsg(SUBMITTING_FORM);
	    HText_SubmitForm(form, doc, form->name, form->value);
	    *refresh_screen = TRUE;
	}

	StrAllocCopy(form->submit_action, save_submit_action);
	FREE(make);
    }
}

static void handle_LYK_RESET(int cur, BOOLEAN *refresh_screen)
{
    FormInfo *form = FindFormAction(links[cur].l_form, 0);
    FormInfo *make = NULL;

    if (form == 0) {
	make = MakeFormAction(links[cur].l_form, 0);
	form = make;
    }

    if (form != 0) {
	HTInfoMsg(RESETTING_FORM);
	HText_ResetForm(form);
	*refresh_screen = TRUE;
	FREE(make);
    }
}

#ifdef USE_ADDRLIST_PAGE
static BOOLEAN handle_LYK_ADDRLIST(int *cmd)
{
    /*
     * Don't do if already viewing list addresses page.
     */
    if (LYIsUIPage(curdoc.address, UIP_ADDRLIST_PAGE)) {
	/*
	 * Already viewing list page, so get out.
	 */
	*cmd = LYK_PREV_DOC;
	return TRUE;
    }

    /*
     * Print address list page to file.
     */
    if (showlist(&newdoc, FALSE) < 0)
	return FALSE;
    StrAllocCopy(newdoc.title, ADDRLIST_PAGE_TITLE);
    /*
     * showlist will set newdoc's other fields.  It may leave post_data intact
     * so the list can be used to follow internal links in the current document
     * even if it is a POST response.  - kw
     */

    if (LYValidate || check_realm) {
	LYPermitURL = TRUE;
	StrAllocCopy(lynxlistfile, newdoc.address);
    }
    return FALSE;
}
#endif /* USE_ADDRLIST_PAGE */

static void handle_LYK_ADD_BOOKMARK(BOOLEAN *refresh_screen,
				    int *old_c,
				    int real_c)
{
    int c;

    if (LYValidate) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(BOOKMARKS_DISABLED);
	}
	return;
    }

    if (!LYIsUIPage(curdoc.address, UIP_HISTORY) &&
	!LYIsUIPage(curdoc.address, UIP_SHOWINFO) &&
	!LYIsUIPage(curdoc.address, UIP_PRINT_OPTIONS) &&
#ifdef DIRED_SUPPORT
	!LYIsUIPage(curdoc.address, UIP_DIRED_MENU) &&
	!LYIsUIPage(curdoc.address, UIP_PERMIT_OPTIONS) &&
	!LYIsUIPage(curdoc.address, UIP_UPLOAD_OPTIONS) &&
#endif /* DIRED_SUPPORT */
	!LYIsUIPage(curdoc.address, UIP_DOWNLOAD_OPTIONS) &&
	!isLYNXCOOKIE(curdoc.address) &&
	!isLYNXCACHE(curdoc.address) &&
	!LYIsUIPage(curdoc.address, UIP_OPTIONS_MENU) &&
	((nlinks <= 0) ||
	 (links[curdoc.link].lname != NULL &&
	  !isLYNXHIST(links[curdoc.link].lname) &&
	  !isLYNXPRINT(links[curdoc.link].lname) &&
	  !isLYNXDIRED(links[curdoc.link].lname) &&
	  !isLYNXDOWNLOAD(links[curdoc.link].lname) &&
	  !isLYNXCOOKIE(links[curdoc.link].lname) &&
	  !isLYNXCACHE(links[curdoc.link].lname) &&
	  !isLYNXPRINT(links[curdoc.link].lname)))) {
	if (nlinks > 0) {
	    if (curdoc.post_data == NULL &&
		curdoc.bookmark == NULL &&
		!LYIsUIPage(curdoc.address, UIP_LIST_PAGE) &&
		!LYIsUIPage(curdoc.address, UIP_ADDRLIST_PAGE) &&
		!LYIsUIPage(curdoc.address, UIP_VLINKS)) {
		/*
		 * The document doesn't have POST content, and is not a
		 * bookmark file, nor is the list or visited links page, so we
		 * can save either that or the link.  - FM
		 */
		_statusline(BOOK_D_L_OR_CANCEL);
		if ((c = LYgetch_single()) == 'D') {
		    save_bookmark_link(curdoc.address, curdoc.title);
		    *refresh_screen = TRUE;	/* MultiBookmark support */
		    goto check_add_bookmark_to_self;
		}
	    } else {
		if (LYMultiBookmarks == MBM_OFF &&
		    curdoc.bookmark != NULL &&
		    strstr(curdoc.address,
			   (*bookmark_page == '.'
			    ? (bookmark_page + 1)
			    : bookmark_page)) != NULL) {
		    /*
		     * If multiple bookmarks are disabled, offer the L)ink or
		     * C)ancel, but with wording which indicates that the link
		     * already exists in this bookmark file.  - FM
		     */
		    _statusline(MULTIBOOKMARKS_SELF);
		} else if (curdoc.post_data != NULL &&
			   links[curdoc.link].type == WWW_INTERN_LINK_TYPE) {
		    /*
		     * Internal link, and document has POST content.
		     */
		    HTUserMsg(NOBOOK_POST_FORM);
		    return;
		} else {
		    /*
		     * Only offer the link in a document with POST content, or
		     * if the current document is a bookmark file and multiple
		     * bookmarks are enabled.  - FM
		     */
		    _statusline(BOOK_L_OR_CANCEL);
		}
		c = LYgetch_single();
	    }
	    if (c == 'L') {
		if (curdoc.post_data != NULL &&
		    links[curdoc.link].type == WWW_INTERN_LINK_TYPE) {
		    /*
		     * Internal link, and document has POST content.
		     */
		    HTUserMsg(NOBOOK_POST_FORM);
		    return;
		}
		/*
		 * User does want to save the link.  - FM
		 */
		if (links[curdoc.link].type != WWW_FORM_LINK_TYPE) {
		    save_bookmark_link(links[curdoc.link].lname,
				       LYGetHiliteStr(curdoc.link, 0));
		    *refresh_screen = TRUE;	/* MultiBookmark support */
		} else {
		    HTUserMsg(NOBOOK_FORM_FIELD);
		    return;
		}
	    } else {
		return;
	    }
	} else if (curdoc.post_data != NULL) {
	    /*
	     * No links, and document has POST content.  - FM
	     */
	    HTUserMsg(NOBOOK_POST_FORM);
	    return;
	} else if (curdoc.bookmark != NULL) {
	    /*
	     * It's a bookmark file from which all of the links were deleted.
	     * - FM
	     */
	    HTUserMsg(BOOKMARKS_NOLINKS);
	    return;
	} else {
	    _statusline(BOOK_D_OR_CANCEL);
	    if (LYgetch_single() == 'D') {
		save_bookmark_link(curdoc.address, curdoc.title);
		*refresh_screen = TRUE;		/* MultiBookmark support */
	    } else {
		return;
	    }
	}
      check_add_bookmark_to_self:
	if (curdoc.bookmark && BookmarkPage &&
	    !strcmp(curdoc.bookmark, BookmarkPage)) {
	    HTuncache_current_document();
	    move_address(&newdoc, &curdoc);
	    StrAllocCopy(newdoc.bookmark, curdoc.bookmark);
	    newdoc.line = curdoc.line;
	    newdoc.link = curdoc.link;
	    newdoc.internal_link = FALSE;
	}
    } else {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(NOBOOK_HSML);
	}
    }
}

static void handle_LYK_CLEAR_AUTH(int *old_c,
				  int real_c)
{
    if (*old_c != real_c) {
	*old_c = real_c;
	if (HTConfirm(CLEAR_ALL_AUTH_INFO)) {
	    FREE(authentication_info[0]);
	    FREE(authentication_info[1]);
	    FREE(proxyauth_info[0]);
	    FREE(proxyauth_info[1]);
	    HTClearHTTPAuthInfo();
#ifndef DISABLE_NEWS
	    HTClearNNTPAuthInfo();
#endif
#ifndef DISABLE_FTP
	    HTClearFTPPassword();
#endif
	    HTUserMsg(AUTH_INFO_CLEARED);
	} else {
	    HTUserMsg(CANCELLED);
	}
    }
}

static int handle_LYK_COMMAND(bstring **user_input)
{
    LYKeymapCode ch;
    Kcmd *mp;
    char *src, *tmp;

    BStrCopy0((*user_input), "");
    _statusline(": ");
    if (LYgetBString(user_input, FALSE, 0, RECALL_CMD) >= 0) {
	src = LYSkipBlanks((*user_input)->str);
	tmp = LYSkipNonBlanks(src);
	*tmp = 0;
	ch = ((mp = LYStringToKcmd(src)) != 0) ? mp->code : LYK_UNKNOWN;
	CTRACE((tfp, "LYK_COMMAND(%s.%s) = %d\n", src, tmp, (int) ch));
	if (ch == 0) {
	    return *src ? -1 : 0;
	}
	/* FIXME: reuse the rest of the buffer for parameters */
	return ch;
    }
    return 0;
}

static void handle_LYK_COMMENT(BOOLEAN *refresh_screen,
			       char **owner_address_p,
			       int *old_c,
			       int real_c)
{
    int c;

    if (!*owner_address_p &&
	strncasecomp(curdoc.address, "http", 4)) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(NO_OWNER);
	}
    } else if (no_mail) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(MAIL_DISALLOWED);
	}
    } else {
	if (HTConfirmDefault(CONFIRM_COMMENT, NO)) {
	    if (!*owner_address_p) {
		/*
		 * No owner defined, so make a guess and and offer it to the
		 * user.  - FM
		 */
		char *address = NULL;
		char *temp = HTParse(curdoc.address, "", PARSE_PATH);
		char *cp;

		if (temp != NULL) {
		    HTUnEscape(temp);
		    if (LYIsTilde(*temp) && strlen(temp) > 1) {
			/*
			 * It's a ~user URL so guess user@host.  - FM
			 */
			if ((cp = StrChr((temp + 1), '/')) != NULL)
			    *cp = '\0';
			StrAllocCopy(address, STR_MAILTO_URL);
			StrAllocCat(address, (temp + 1));
			StrAllocCat(address, "@");
		    }
		    FREE(temp);
		}
		if (address == NULL)
		    /*
		     * Wasn't a ~user URL so guess WebMaster@host.  - FM
		     */
		    StrAllocCopy(address, "mailto:WebMaster@");
		temp = HTParse(curdoc.address, "", PARSE_HOST);
		StrAllocCat(address, temp);
		HTSprintf0(&temp, NO_OWNER_USE, address);
		c = HTConfirmDefault(temp, NO);
		FREE(temp);
		if (c == YES) {
		    StrAllocCopy(*owner_address_p, address);
		    FREE(address);
		} else {
		    FREE(address);
		    return;
		}
	    }
	    if (is_url(*owner_address_p) != MAILTO_URL_TYPE) {
		/*
		 * The address is a URL.  Just follow the link.
		 */
		set_address(&newdoc, *owner_address_p);
		newdoc.internal_link = FALSE;
	    } else {
		/*
		 * The owner_address is a mailto:  URL.
		 */
		const char *kp = HText_getRevTitle();
		const char *id = HText_getMessageID();
		char *tmptitle = NULL;

		if (!kp && HTMainAnchor) {
		    kp = HTAnchor_subject(HTMainAnchor);
		    if (non_empty(kp)) {
			if (strncasecomp(kp, "Re: ", 4)) {
			    StrAllocCopy(tmptitle, "Re: ");
			    StrAllocCat(tmptitle, kp);
			    kp = tmptitle;
			}
		    }
		}

		if (StrChr(*owner_address_p, ':') != NULL)
		    /*
		     * Send a reply.  The address is after the colon.
		     */
		    reply_by_mail(StrChr(*owner_address_p, ':') + 1,
				  curdoc.address,
				  NonNull(kp), id);
		else
		    reply_by_mail(*owner_address_p, curdoc.address,
				  NonNull(kp), id);

		FREE(tmptitle);
		*refresh_screen = TRUE;		/* to force a showpage */
	    }
	}
    }
}

#ifdef USE_CACHEJAR
static BOOLEAN handle_LYK_CACHE_JAR(int *cmd)
{
    /*
     * Don't do this if already viewing cache jar.
     */
    if (!isLYNXCACHE(curdoc.address)) {
	set_address(&newdoc, STR_LYNXCACHE "/");
	LYFreePostData(&newdoc);
	FREE(newdoc.bookmark);
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	newdoc.internal_link = FALSE;
	LYforce_no_cache = TRUE;
	if (LYValidate || check_realm) {
	    LYPermitURL = TRUE;
	}
    } else {
	/*
	 * If already in the cache jar, get out.
	 */
	*cmd = LYK_PREV_DOC;
	return TRUE;
    }
    return FALSE;
}
#endif /* USE_CACHEJAR */

static BOOLEAN handle_LYK_COOKIE_JAR(int *cmd)
{
    /*
     * Don't do if already viewing the cookie jar.
     */
    if (!isLYNXCOOKIE(curdoc.address)) {
	set_address(&newdoc, "LYNXCOOKIE:/");
	LYFreePostData(&newdoc);
	FREE(newdoc.bookmark);
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	newdoc.internal_link = FALSE;
	LYforce_no_cache = TRUE;
	if (LYValidate || check_realm) {
	    LYPermitURL = TRUE;
	}
    } else {
	/*
	 * If already in the cookie jar, get out.
	 */
	*cmd = LYK_PREV_DOC;
	return TRUE;
    }
    return FALSE;
}

#if defined(DIRED_SUPPORT)
static void handle_LYK_CREATE(void)
{
    if (lynx_edit_mode && !no_dired_support) {
	if (local_create(&curdoc) > 0) {
	    DIRED_UNCACHE_1;
	    move_address(&newdoc, &curdoc);
	    LYFreePostData(&newdoc);
	    FREE(newdoc.bookmark);
	    newdoc.isHEAD = FALSE;
	    newdoc.safe = FALSE;
	    newdoc.line = curdoc.line;
	    newdoc.link = curdoc.link > -1 ? curdoc.link : 0;
	    LYclear();
	}
    }
}
#endif /* DIRED_SUPPORT */

static void handle_LYK_DEL_BOOKMARK(BOOLEAN *refresh_screen,
				    int *old_c,
				    int real_c)
{
    if (curdoc.bookmark != NULL) {
	if (HTConfirmDefault(CONFIRM_BOOKMARK_DELETE, NO) != YES)
	    return;
	remove_bookmark_link(links[curdoc.link].anchor_number - 1,
			     curdoc.bookmark);
    } else {			/* behave like REFRESH for backward compatibility */
	*refresh_screen = TRUE;
	if (*old_c != real_c) {
	    *old_c = real_c;
	    lynx_force_repaint();
	}
	return;
    }
    do_cleanup_after_delete();
}

#if defined(DIRED_SUPPORT) || defined(VMS)
static void handle_LYK_DIRED_MENU(BOOLEAN *refresh_screen,
				  int *old_c GCC_UNUSED,
				  int real_c GCC_UNUSED)
{
#ifdef VMS
    char *cp, *temp = 0;
    const char *test = HTGetProgramPath(ppCSWING);

    /*
     * Check if the CSwing Directory/File Manager is available.  Will be
     * disabled if CSWING path is NULL, zero-length, or "none" (case
     * insensitive), if no_file_url was set via the file_url restriction, if
     * no_goto_file was set for the anonymous account, or if HTDirAccess was
     * set to HT_DIR_FORBID or HT_DIR_SELECTIVE via the -nobrowse or -selective
     * switches.  - FM
     */
    if (isEmpty(test) ||
	!strcasecomp(test, "none") ||
	no_file_url || no_goto_file ||
	HTDirAccess == HT_DIR_FORBID ||
	HTDirAccess == HT_DIR_SELECTIVE) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(DFM_NOT_AVAILABLE);
	}
	return;
    }

    /*
     * If we are viewing a local directory listing or a local file which is not
     * temporary, invoke CSwing with the URL's directory converted to VMS path
     * specs and passed as the argument, so we start up CSwing positioned on
     * that node of the directory tree.  Otherwise, pass the current default
     * directory as the argument.  - FM
     */
    if (LYisLocalFile(curdoc.address) &&
	strncasecomp(curdoc.address,
		     lynx_temp_space, strlen(lynx_temp_space))) {
	/*
	 * We are viewing a local directory or a local file which is not
	 * temporary.  - FM
	 */
	struct stat stat_info;

	cp = HTParse(curdoc.address, "", PARSE_PATH | PARSE_PUNCTUATION);
	HTUnEscape(cp);
	if (HTStat(cp, &stat_info) == -1) {
	    CTRACE((tfp, "mainloop: Can't stat %s\n", cp));
	    FREE(cp);
	    HTSprintf0(&temp, "%s []", HTGetProgramPath(ppCSWING));
	    *refresh_screen = TRUE;	/* redisplay */
	} else {
	    char *VMSdir = NULL;

	    if (S_ISDIR(stat_info.st_mode)) {
		/*
		 * We're viewing a local directory.  Make that the CSwing
		 * argument.  - FM
		 */
		LYAddPathSep(&cp);
		StrAllocCopy(VMSdir, HTVMS_name("", cp));
		FREE(cp);
	    } else {
		/*
		 * We're viewing a local file.  Make its directory the CSwing
		 * argument.  - FM
		 */
		StrAllocCopy(VMSdir, HTVMS_name("", cp));
		FREE(cp);
		if ((cp = strrchr(VMSdir, ']')) != NULL) {
		    *(cp + 1) = '\0';
		} else if ((cp = strrchr(VMSdir, ':')) != NULL) {
		    *(cp + 1) = '\0';
		}
	    }
	    HTSprintf0(&temp, "%s %s", HTGetProgramPath(ppCSWING), VMSdir);
	    FREE(VMSdir);
	    /*
	     * Uncache the current document in case we change, move, or delete
	     * it during the CSwing session.  - FM
	     */
	    /* could use DIRED_UNCACHE_1 but it's currently only defined
	       for dired - kw */
	    HTuncache_current_document();
	    move_address(&newdoc, &curdoc);
	    StrAllocCopy(newdoc.title, NonNull(curdoc.title));
	    StrAllocCopy(newdoc.bookmark, curdoc.bookmark);
	    newdoc.line = curdoc.line;
	    newdoc.link = curdoc.link;
	}
    } else {
	/*
	 * We're not viewing a local directory or file.  Pass CSwing the
	 * current default directory as an argument and don't uncache the
	 * current document.  - FM
	 */
	HTSprintf0(&temp, "%s []", HTGetProgramPath(ppCSWING));
	*refresh_screen = TRUE;	/* redisplay */
    }
    stop_curses();
    LYSystem(temp);
    start_curses();
    FREE(temp);
#else
    /*
     * Don't do if not allowed or already viewing the menu.
     */
    if (lynx_edit_mode && !no_dired_support &&
	!LYIsUIPage(curdoc.address, UIP_DIRED_MENU) &&
	strcmp(NonNull(curdoc.title), DIRED_MENU_TITLE)) {
	dired_options(&curdoc, &newdoc.address);
	*refresh_screen = TRUE;	/* redisplay */
    }
#endif /* VMS */
}
#endif /* defined(DIRED_SUPPORT) || defined(VMS) */

static int handle_LYK_DOWNLOAD(int *cmd,
			       int *old_c,
			       int real_c)
{

    /*
     * Don't do if both download and disk_save are restricted.
     */
    if (LYValidate ||
	(no_download && !override_no_download && no_disk_save)) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(DOWNLOAD_DISABLED);
	}
	return 0;
    }

    /*
     * Don't do if already viewing download options page.
     */
    if (LYIsUIPage(curdoc.address, UIP_DOWNLOAD_OPTIONS))
	return 0;

    if (do_change_link() == -1)
	return 1;		/* mouse stuff was confused, ignore - kw */
    if (nlinks > 0) {
	if (links[curdoc.link].type == WWW_FORM_LINK_TYPE) {
	    if (links[curdoc.link].l_form->type == F_SUBMIT_TYPE ||
		links[curdoc.link].l_form->type == F_IMAGE_SUBMIT_TYPE ||
		links[curdoc.link].l_form->type == F_TEXT_SUBMIT_TYPE) {
		if (links[curdoc.link].l_form->submit_method ==
		    URL_MAIL_METHOD) {
		    if (*old_c != real_c) {
			*old_c = real_c;
			HTUserMsg(NO_DOWNLOAD_MAILTO_ACTION);
		    }
		    return 0;
		}
		if (isEmpty(links[curdoc.link].l_form->submit_action) ||
		    isLYNXOPTIONS(links[curdoc.link].l_form->submit_action)) {
		    if (*old_c != real_c) {
			*old_c = real_c;
			HTUserMsg(NO_DOWNLOAD_SPECIAL);
		    }
		    return 0;
		}
		HTOutputFormat = HTAtom_for("www/download");
		LYforce_no_cache = TRUE;
		*cmd = LYK_ACTIVATE;
		return 2;
	    }
	    if (*old_c != real_c) {
		*old_c = real_c;
		HTUserMsg(NO_DOWNLOAD_INPUT);
	    }

	} else if (isLYNXCOOKIE(curdoc.address)) {
	    if (*old_c != real_c) {
		*old_c = real_c;
		HTUserMsg(NO_DOWNLOAD_COOKIES);
	    }
	} else if (LYIsUIPage(curdoc.address, UIP_PRINT_OPTIONS)) {
	    if (*old_c != real_c) {
		*old_c = real_c;
		HTUserMsg(NO_DOWNLOAD_PRINT_OP);
	    }
#ifdef DIRED_SUPPORT
	} else if (LYIsUIPage(curdoc.address, UIP_UPLOAD_OPTIONS)) {
	    if (*old_c != real_c) {
		*old_c = real_c;
		HTUserMsg(NO_DOWNLOAD_UPLOAD_OP);
	    }

	} else if (LYIsUIPage(curdoc.address, UIP_PERMIT_OPTIONS)) {
	    if (*old_c != real_c) {
		*old_c = real_c;
		HTUserMsg(NO_DOWNLOAD_PERMIT_OP);
	    }

	} else if (lynx_edit_mode && !no_dired_support &&
		   !strstr(links[curdoc.link].lname, "/SugFile=")) {
	    /*
	     * Don't bother making a /tmp copy of the local file.
	     */
	    static DocInfo temp;

	    copy_address(&temp, &newdoc);
	    set_address(&newdoc, links[curdoc.link].lname);
	    if (LYdownload_options(&newdoc.address,
				   links[curdoc.link].lname) < 0)
		copy_address(&newdoc, &temp);
	    else
		newdoc.internal_link = FALSE;
	    LYFreeDocInfo(&temp);
#endif /* DIRED_SUPPORT */

	} else if (LYIsUIPage(curdoc.address, UIP_HISTORY) &&
		   isLYNXHIST(links[curdoc.link].lname)) {
	    int number = atoi(links[curdoc.link].lname + LEN_LYNXHIST);

	    if (number >= nhist || number < 0) {
		HTUserMsg(NO_DOWNLOAD_SPECIAL);
		return 0;
	    }
	    if ((HDOC(number).post_data != NULL &&
		 HDOC(number).safe != TRUE) &&
		HTConfirm(CONFIRM_POST_RESUBMISSION) == FALSE) {
		HTInfoMsg(CANCELLED);
		return 0;
	    }
	    /*
	     * OK, we download from history page, restore URL from stack.
	     */
	    copy_address(&newdoc, &HDOC(number));
	    StrAllocCopy(newdoc.title, LYGetHiliteStr(curdoc.link, 0));
	    StrAllocCopy(newdoc.bookmark, HDOC(number).bookmark);
	    LYFreePostData(&newdoc);
	    if (HDOC(number).post_data)
		BStrCopy(newdoc.post_data,
			 HDOC(number).post_data);
	    if (HDOC(number).post_content_type)
		StrAllocCopy(newdoc.post_content_type,
			     HDOC(number).post_content_type);
	    newdoc.isHEAD = HDOC(number).isHEAD;
	    newdoc.safe = HDOC(number).safe;
	    newdoc.internal_link = FALSE;
	    newdoc.link = (user_mode == NOVICE_MODE) ? 1 : 0;
	    HTOutputFormat = HTAtom_for("www/download");
	    LYUserSpecifiedURL = TRUE;
	    /*
	     * Force the document to be reloaded.
	     */
	    LYforce_no_cache = TRUE;

	} else if (!StrNCmp(links[curdoc.link].lname, "data:", 5)) {
	    if (*old_c != real_c) {
		*old_c = real_c;
		HTAlert(UNSUPPORTED_DATA_URL);
	    }

	} else if (isLYNXCOOKIE(links[curdoc.link].lname) ||
		   isLYNXCACHE(links[curdoc.link].lname) ||
		   isLYNXDIRED(links[curdoc.link].lname) ||
		   isLYNXDOWNLOAD(links[curdoc.link].lname) ||
		   isLYNXPRINT(links[curdoc.link].lname) ||
		   isLYNXOPTIONS(links[curdoc.link].lname) ||
		   isLYNXHIST(links[curdoc.link].lname) ||
	    /* handled above if valid - kw */
/* @@@ should next two be downloadable? - kw */
		   isLYNXHIST(links[curdoc.link].lname) ||
		   isLYNXCFLAGS(links[curdoc.link].lname) ||
		   isLYNXEXEC(links[curdoc.link].lname) ||
		   isLYNXPROG(links[curdoc.link].lname)) {
	    HTUserMsg(NO_DOWNLOAD_SPECIAL);

	} else if (isMAILTO_URL(links[curdoc.link].lname)) {
	    HTUserMsg(NO_DOWNLOAD_MAILTO_LINK);

	    /*
	     * From here on we could have a remote host, so check if that's
	     * allowed.
	     *
	     * We copy all these checks from getfile() to LYK_DOWNLOAD here
	     * because LYNXDOWNLOAD:// will NOT be pushing the previous
	     * document into the history stack so preserve getfile() from
	     * returning a wrong status (NULLFILE).
	     */
	} else if (local_host_only &&
		   !(LYisLocalHost(links[curdoc.link].lname) ||
		     LYisLocalAlias(links[curdoc.link].lname))) {
	    HTUserMsg(ACCESS_ONLY_LOCALHOST);
	} else {		/* Not a forms, options or history link */
	    /*
	     * Follow a normal link or anchor.  Note that if it's an anchor
	     * within the same document, entire document will be downloaded.
	     */
	    set_address(&newdoc, links[curdoc.link].lname);
	    StrAllocCopy(newdoc.title, LYGetHiliteStr(curdoc.link, 0));
	    /*
	     * Might be an internal link in the same doc from a POST form.  If
	     * so, don't free the content.  - kw
	     */
	    if (track_internal_links) {
		if (links[curdoc.link].type != WWW_INTERN_LINK_TYPE) {
		    LYFreePostData(&newdoc);
		    FREE(newdoc.bookmark);
		    newdoc.isHEAD = FALSE;
		    newdoc.safe = FALSE;
		}
	    } else {
		/*
		 * Might be an anchor in the same doc from a POST form.  If so,
		 * don't free the content.  -- FM
		 */
		if (are_different(&curdoc, &newdoc)) {
		    LYFreePostData(&newdoc);
		    FREE(newdoc.bookmark);
		    newdoc.isHEAD = FALSE;
		    newdoc.safe = FALSE;
		}
	    }
	    newdoc.internal_link = FALSE;
	    newdoc.link = (user_mode == NOVICE_MODE) ? 1 : 0;
	    HTOutputFormat = HTAtom_for("www/download");
	    /*
	     * Force the document to be reloaded.
	     */
	    LYforce_no_cache = TRUE;
	}
    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTUserMsg(NO_DOWNLOAD_CHOICE);
    }
    return 0;
}

static void handle_LYK_DOWN_xxx(int *old_c,
				int real_c,
				int scroll_by)
{
    int i;

    if (more_text) {
	LYChgNewline(scroll_by);
	if (nlinks > 0 && curdoc.link > -1 &&
	    links[curdoc.link].ly > scroll_by) {
	    newdoc.link = curdoc.link;
	    for (i = 0; links[i].ly <= scroll_by; i++)
		--newdoc.link;
	}
    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTInfoMsg(ALREADY_AT_END);
    }
}

static void handle_LYK_DOWN_HALF(int *old_c,
				 int real_c)
{
    handle_LYK_DOWN_xxx(old_c, real_c, display_lines / 2);
}

static void handle_LYK_DOWN_LINK(int *follow_col,
				 int *old_c,
				 int real_c)
{
    if (curdoc.link < (nlinks - 1)) {	/* more links? */
	int newlink;

	if (*follow_col == -1) {
	    const char *text = LYGetHiliteStr(curdoc.link, 0);

	    *follow_col = links[curdoc.link].lx;

	    if (text != NULL)
		*follow_col += (int) strlen(text) / 2;
	}

	newlink = find_link_near_col(*follow_col, 1);
	if (newlink > -1) {
	    set_curdoc_link(newlink);
	} else if (more_text) {	/* next page */
	    LYChgNewline(display_lines);
	} else if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(NO_LINKS_BELOW);
	    return;
	}
    } else if (more_text) {	/* next page */
	LYChgNewline(display_lines);
    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTInfoMsg(ALREADY_AT_END);
    }
}

static void handle_LYK_DOWN_TWO(int *old_c,
				int real_c)
{
    handle_LYK_DOWN_xxx(old_c, real_c, 2);
}

static int handle_LYK_DWIMEDIT(int *cmd,
			       int *old_c,
			       int real_c)
{
#ifdef TEXTAREA_AUTOEXTEDIT
    /*
     * If we're in a forms TEXTAREA, invoke the editor on *its* contents,
     * rather than attempting to edit the html source document.  KED
     */
    if (nlinks > 0 &&
	LinkIsTextarea(curdoc.link)) {
	*cmd = LYK_EDITTEXTAREA;
	return 2;
    }

    /*
     * If we're in a forms TEXT type, tell user the request is bogus (though in
     * reality, without this trap, if the document with the TEXT field is
     * local, the editor *would* be invoked on the source .html file; eg, the
     * o(ptions) form tempfile).
     *
     * [This is done to avoid possible user confusion, due to auto invocation
     * of the editor on the TEXTAREA's contents via the above if() statement.]
     */
    if (nlinks > 0 &&
	links[curdoc.link].type == WWW_FORM_LINK_TYPE &&
	links[curdoc.link].l_form->type == F_TEXT_TYPE) {
	HTUserMsg(CANNOT_EDIT_FIELD);
	return 1;
    }

    if (no_editor) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(ANYEDIT_DISABLED);
	}
	return 1;
    }
#endif /* TEXTAREA_AUTOEXTEDIT */
    return 0;
}

static int handle_LYK_ECGOTO(int *ch,
			     bstring **user_input,
			     char **old_user_input,
			     int *old_c,
			     int real_c)
{
    if (no_goto && !LYValidate) {
	/*
	 * Go to not allowed.  - FM
	 */
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(GOTO_DISALLOWED);
	}
	return 0;
    }
#ifdef DIRED_SUPPORT
    if (LYIsUIPage(curdoc.address, UIP_DIRED_MENU) ||
	LYIsUIPage(curdoc.address, UIP_PERMIT_OPTIONS) ||
	LYIsUIPage(curdoc.address, UIP_UPLOAD_OPTIONS)) {
	/*
	 * Disallow editing of File Management URLs.  - FM
	 */
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(EDIT_FM_MENU_URLS_DISALLOWED);
	}
	return 0;
    }
#endif /* DIRED_SUPPORT */

    /*
     * Save the current user_input string, and load the current
     * document's address.
     */
    StrAllocCopy(*old_user_input, (*user_input)->str);
    BStrCopy0((*user_input), curdoc.address);

    /*
     * Warn the user if the current document has POST data associated with it.
     * - FM
     */
    if (curdoc.post_data)
	HTAlert(CURRENT_DOC_HAS_POST_DATA);

    /*
     * Offer the current document's URL for editing.  - FM
     */
    _statusline(EDIT_CURDOC_URL);
    if (((*ch = LYgetBString(user_input, FALSE, 0, RECALL_URL)) >= 0) &&
	!isBEmpty(*user_input) &&
	strcmp((*user_input)->str, curdoc.address)) {
	LYTrimAllStartfile((*user_input)->str);
	if (!isBEmpty(*user_input)) {
	    return 2;
	}
    }
    /*
     * User cancelled via ^G, a full deletion, or not modifying the URL.  - FM
     */
    HTInfoMsg(CANCELLED);
    BStrCopy0((*user_input), *old_user_input);
    FREE(*old_user_input);
    return 0;
}

static void handle_LYK_EDIT(int *old_c,
			    int real_c)
{
#ifdef DIRED_SUPPORT
    char *cp;
    char *tp = NULL;
    struct stat dir_info;
#endif /* DIRED_SUPPORT */

    if (no_editor) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(EDIT_DISABLED);
	}
    }
#ifdef DIRED_SUPPORT
    /*
     * Allow the user to edit the link rather than curdoc in edit mode.
     */
    else if (lynx_edit_mode &&
	     non_empty(editor) && !no_dired_support) {
	if (nlinks > 0) {
	    cp = links[curdoc.link].lname;
	    if (is_url(cp) == FILE_URL_TYPE) {
		cp = HTfullURL_toFile(cp);
		StrAllocCopy(tp, cp);
		FREE(cp);

		if (stat(tp, &dir_info) == -1) {
		    HTAlert(NO_STATUS);
		} else {
		    if (S_ISREG(dir_info.st_mode)) {
			StrAllocCopy(tp, links[curdoc.link].lname);
			HTUnEscapeSome(tp, "/");
			if (edit_current_file(tp, curdoc.link, -1)) {
			    DIRED_UNCACHE_1;
			    move_address(&newdoc, &curdoc);
#ifdef NO_SEEK_OLD_POSITION
			    /*
			     * Go to top of file.
			     */
			    newdoc.line = 1;
			    newdoc.link = 0;
#else
			    /*
			     * Seek old position, which probably changed.
			     */
			    newdoc.line = curdoc.line;
			    newdoc.link = curdoc.link;
#endif /* NO_SEEK_OLD_POSITION */
			    LYclear();	/* clear the screen */
			}
		    }
		}
		FREE(tp);
	    }
	}
    }
#endif /* DIRED_SUPPORT */
    else if (non_empty(editor)) {
	if (edit_current_file(newdoc.address, curdoc.link, LYGetNewline())) {
	    HTuncache_current_document();
	    LYforce_no_cache = TRUE;	/*force reload of document */
	    free_address(&curdoc);	/* so it doesn't get pushed */
#ifdef NO_SEEK_OLD_POSITION
	    /*
	     * Go to top of file.
	     */
	    newdoc.line = 1;
	    newdoc.link = 0;
#else
	    /*
	     * Seek old position, which probably changed.
	     */
	    newdoc.line = curdoc.line;
	    newdoc.link = curdoc.link;
#endif /* NO_SEEK_OLD_POSITION */
	    LYclear();		/* clear the screen */
	}

    } else {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(NO_EDITOR);
	}
    }
}

static void handle_LYK_DWIMHELP(const char **cshelpfile)
{
    /*
     * Currently a help file different from the main 'helpfile' is shown only
     * if current link is a text input form field.  - kw
     */
    if (curdoc.link >= 0 && curdoc.link < nlinks &&
	!FormIsReadonly(links[curdoc.link].l_form) &&
	LinkIsTextLike(curdoc.link)) {
	*cshelpfile = STR_LYNXEDITMAP;
    }
}

static void handle_LYK_EDITMAP(int *old_c,
			       int real_c)
{
    if (*old_c != real_c) {
	*old_c = real_c;
	set_address(&newdoc, STR_LYNXEDITMAP);
	StrAllocCopy(newdoc.title, CURRENT_EDITMAP_TITLE);
	LYFreePostData(&newdoc);
	FREE(newdoc.bookmark);
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	newdoc.internal_link = FALSE;
#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
	/*
	 * Remember whether we are in dired menu so we can display the right
	 * keymap.
	 */
	if (!no_dired_support) {
	    prev_lynx_edit_mode = lynx_edit_mode;
	}
#endif /* DIRED_SUPPORT && OK_OVERRIDE */
	LYforce_no_cache = TRUE;
    }
}

static void handle_LYK_EDIT_TEXTAREA(BOOLEAN *refresh_screen,
				     int *old_c,
				     int real_c)
{
    if (no_editor) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(ANYEDIT_DISABLED);
	}
    } else if (isEmpty(editor)) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(NO_EDITOR);
	}
    } else if (LinkIsTextarea(curdoc.link)) {
	/*
	 * if the current link is in a form TEXTAREA, it requires handling
	 * for the possible multiple lines.
	 */

	/* stop screen */
	stop_curses();

	(void) HText_EditTextArea(&links[curdoc.link]);

	/*
	 * TODO:
	 * Move cursor "n" lines from the current line to position it on the
	 * 1st trailing blank line in the now edited TEXTAREA.  If the target
	 * line/ anchor requires us to scroll up/down, position the target in
	 * the approximate center of the screen.
	 */

	/* curdoc.link += n; */
	/* works, except for page crossing, */
	/* damnit; why is nothing ever easy */

	/* start screen */
	start_curses();
	*refresh_screen = TRUE;

    } else if (LinkIsTextLike(curdoc.link)) {
	/*
	 * other text fields are single-line
	 */
	stop_curses();
	HText_EditTextField(&links[curdoc.link]);
	start_curses();
	*refresh_screen = TRUE;
    } else {

	HTInfoMsg(NOT_IN_TEXTAREA_NOEDIT);
    }
}

static int handle_LYK_ELGOTO(int *ch,
			     bstring **user_input,
			     char **old_user_input,
			     int *old_c,
			     int real_c)
{
    if (no_goto && !LYValidate) {
	/*
	 * Go to not allowed.  - FM
	 */
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(GOTO_DISALLOWED);
	}
	return 0;
    }
    if (!(nlinks > 0 && curdoc.link > -1) ||
	(links[curdoc.link].type == WWW_FORM_LINK_TYPE &&
	 links[curdoc.link].l_form->type != F_SUBMIT_TYPE &&
	 links[curdoc.link].l_form->type != F_IMAGE_SUBMIT_TYPE &&
	 links[curdoc.link].l_form->type != F_TEXT_SUBMIT_TYPE)) {
	/*
	 * No links on page, or not a normal link or form submit button.  - FM
	 */
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(NOT_ON_SUBMIT_OR_LINK);
	}
	return 0;
    }
    if ((links[curdoc.link].type == WWW_FORM_LINK_TYPE) &&
	(isEmpty(links[curdoc.link].l_form->submit_action))) {
	/*
	 * Form submit button with no ACTION defined.  - FM
	 */
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(NO_FORM_ACTION);
	}
	return 0;
    }
#ifdef DIRED_SUPPORT
    if (isLYNXDIRED(links[curdoc.link].lname) ||
	LYIsUIPage(curdoc.address, UIP_DIRED_MENU) ||
	LYIsUIPage(curdoc.address, UIP_PERMIT_OPTIONS) ||
	LYIsUIPage(curdoc.address, UIP_UPLOAD_OPTIONS)) {
	/*
	 * Disallow editing of File Management URLs.  - FM
	 */
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(EDIT_FM_MENU_URLS_DISALLOWED);
	}
	return 0;
    }
#endif /* DIRED_SUPPORT */

    /*
     * Save the current user_input string, and load the current link's
     * address.  - FM
     */
    StrAllocCopy(*old_user_input, (*user_input)->str);
    BStrCopy0((*user_input),
	      ((links[curdoc.link].type == WWW_FORM_LINK_TYPE)
	       ? links[curdoc.link].l_form->submit_action
	       : links[curdoc.link].lname));
    /*
     * Offer the current link's URL for editing.  - FM
     */
    _statusline(EDIT_CURLINK_URL);
    if (((*ch = LYgetBString(user_input, FALSE, 0, RECALL_URL)) >= 0) &&
	!isBEmpty(*user_input) &&
	strcmp((*user_input)->str,
	       ((links[curdoc.link].type == WWW_FORM_LINK_TYPE)
		? links[curdoc.link].l_form->submit_action
		: links[curdoc.link].lname))) {
	LYTrimAllStartfile((*user_input)->str);
	if (!isBEmpty(*user_input)) {
	    return 2;
	}
    }
    /*
     * User cancelled via ^G, a full deletion, or not modifying the URL.  - FM
     */
    HTInfoMsg(CANCELLED);
    BStrCopy0((*user_input), *old_user_input);
    FREE(*old_user_input);
    return 0;
}

#ifdef USE_EXTERNALS
static void handle_LYK_EXTERN_LINK(BOOLEAN *refresh_screen)
{
    if ((nlinks > 0) && (links[curdoc.link].lname != NULL)) {
	run_external(links[curdoc.link].lname, FALSE);
	*refresh_screen = TRUE;
    }
}

static void handle_LYK_EXTERN_PAGE(BOOLEAN *refresh_screen)
{
    if (curdoc.address != NULL) {
	run_external(curdoc.address, FALSE);
	*refresh_screen = TRUE;
    }
}
#endif

static BOOLEAN handle_LYK_FASTBACKW_LINK(int *cmd,
					 int *old_c,
					 int real_c)
{
    int samepage = 0, nextlink = curdoc.link;
    int res;
    BOOLEAN code = FALSE;

    if (nlinks > 1) {

	/*
	 * If in textarea, move to first link or textarea group before it if
	 * there is one on this screen.  - kw
	 */
	if (LinkIsTextarea(curdoc.link)) {
	    int thisgroup = links[curdoc.link].l_form->number;
	    char *thisname = links[curdoc.link].l_form->name;

	    if (curdoc.link > 0 &&
		!(LinkIsTextarea(0) &&
		  links[0].l_form->number == thisgroup &&
		  sametext(links[0].l_form->name, thisname))) {
		do
		    nextlink--;
		while
		    (LinkIsTextarea(nextlink) &&
		     links[nextlink].l_form->number == thisgroup &&
		     sametext(links[nextlink].l_form->name, thisname));
		samepage = 1;

	    } else if (!more_text && LYGetNewline() == 1 &&
		       (LinkIsTextarea(0) &&
			links[0].l_form->number == thisgroup &&
			sametext(links[0].l_form->name, thisname)) &&
		       !(LinkIsTextarea(nlinks - 1) &&
			 links[nlinks - 1].l_form->number == thisgroup &&
			 sametext(links[nlinks - 1].l_form->name, thisname))) {
		nextlink = nlinks - 1;
		samepage = 1;

	    } else if (!more_text && LYGetNewline() == 1 && curdoc.link > 0) {
		nextlink = 0;
		samepage = 1;
	    }
	} else if (curdoc.link > 0) {
	    nextlink--;
	    samepage = 1;
	} else if (!more_text && LYGetNewline() == 1) {
	    nextlink = nlinks - 1;
	    samepage = 1;
	}
    }

    if (samepage) {
	/*
	 * If the link as determined so far is part of a group of textarea
	 * fields, try to use the first of them that's on the screen instead.
	 * - kw
	 */
	if (nextlink > 0 &&
	    LinkIsTextarea(nextlink)) {
	    int thisgroup = links[nextlink].l_form->number;
	    char *thisname = links[nextlink].l_form->name;

	    if (LinkIsTextarea(0) &&
		links[0].l_form->number == thisgroup &&
		sametext(links[0].l_form->name, thisname)) {
		nextlink = 0;
	    } else
		while
		    (nextlink > 1 &&
		     LinkIsTextarea(nextlink - 1) &&
		     links[nextlink - 1].l_form->number == thisgroup &&
		     sametext(links[nextlink - 1].l_form->name, thisname)) {
		    nextlink--;
		}
	}
	set_curdoc_link(nextlink);

    } else if (LYGetNewline() > 1 &&	/* need a previous page */
	       (res = HTGetLinkOrFieldStart(curdoc.link,
					    &Newline, &newdoc.link,
					    -1, TRUE)) != NO) {
	if (res == LINK_DO_ARROWUP) {
	    /*
	     * It says we should use the normal PREV_LINK mechanism, so we'll
	     * do that.  - kw
	     */
	    if (nlinks > 0)
		curdoc.link = 0;
	    *cmd = LYK_PREV_LINK;
	    code = TRUE;
	} else {
	    LYChgNewline(1);	/* our line counting starts with 1 not 0 */
	}
    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTInfoMsg(NO_LINKS_ABOVE);
    }
    return code;
}

static void handle_LYK_FASTFORW_LINK(int *old_c,
				     int real_c)
{
    int samepage = 0, nextlink = curdoc.link;

    if (nlinks > 1) {

	/*
	 * If in textarea, move to first link or field after it if there is one
	 * on this screen.  - kw
	 */
	if (LinkIsTextarea(curdoc.link)) {
	    int thisgroup = links[curdoc.link].l_form->number;
	    char *thisname = links[curdoc.link].l_form->name;

	    if (curdoc.link < nlinks - 1 &&
		!(LinkIsTextarea(nlinks - 1) &&
		  links[nlinks - 1].l_form->number == thisgroup &&
		  sametext(links[nlinks - 1].l_form->name, thisname))) {
		do
		    nextlink++;
		while
		    (LinkIsTextarea(nextlink) &&
		     links[nextlink].l_form->number == thisgroup &&
		     sametext(links[nextlink].l_form->name, thisname));
		samepage = 1;
	    } else if (!more_text && LYGetNewline() == 1 && curdoc.link > 0) {
		nextlink = 0;
		samepage = 1;
	    }
	} else if (curdoc.link < nlinks - 1) {
	    nextlink++;
	    samepage = 1;
	} else if (!more_text && LYGetNewline() == 1 && curdoc.link > 0) {
	    nextlink = 0;
	    samepage = 1;
	}
    }

    if (samepage) {
	set_curdoc_link(nextlink);
    } else if (!more_text && LYGetNewline() == 1 && curdoc.link == nlinks - 1) {
	/*
	 * At the bottom of list and there is only one page.  Move to the top
	 * link on the page.
	 */
	set_curdoc_link(0);

    } else if (more_text &&	/* need a later page */
	       HTGetLinkOrFieldStart(curdoc.link,
				     &Newline, &newdoc.link,
				     1, TRUE) != NO) {
	LYChgNewline(1);	/* our line counting starts with 1 not 0 */
	/* nothing more to do here */

    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTInfoMsg(NO_LINKS_BELOW);
    }
    return;
}

static void handle_LYK_FIRST_LINK(void)
{
    int i = curdoc.link;

    for (;;) {
	if (--i < 0
	    || links[i].ly != links[curdoc.link].ly) {
	    set_curdoc_link(i + 1);
	    break;
	}
    }
}

static BOOLEAN handle_LYK_GOTO(int *ch,
			       bstring **user_input,
			       char **old_user_input,
			       RecallType * recall,
			       int *URLTotal,
			       int *URLNum,
			       BOOLEAN *FirstURLRecall,
			       int *old_c,
			       int real_c)
{

    if (no_goto && !LYValidate) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(GOTO_DISALLOWED);
	}
	return FALSE;
    }

    StrAllocCopy(*old_user_input, (*user_input)->str);
    if (!goto_buffer)
	BStrCopy0((*user_input), "");

    *URLTotal = (Goto_URLs ? HTList_count(Goto_URLs) : 0);
    if (goto_buffer && !isBEmpty(*user_input)) {
	*recall = ((*URLTotal > 1) ? RECALL_URL : NORECALL);
	*URLNum = 0;
	*FirstURLRecall = FALSE;
    } else {
	*recall = ((*URLTotal >= 1) ? RECALL_URL : NORECALL);
	*URLNum = *URLTotal;
	*FirstURLRecall = TRUE;
    }

    /*
     * Ask the user.
     */
    _statusline(URL_TO_OPEN);
    if ((*ch = LYgetBString(user_input, FALSE, 0, *recall)) < 0) {
	/*
	 * User cancelled the Goto via ^G.  Restore user_input and
	 * break.  - FM
	 */
	BStrCopy0((*user_input), *old_user_input);
	FREE(*old_user_input);
	HTInfoMsg(CANCELLED);
	return FALSE;
    }
    return TRUE;
}

static void handle_LYK_GROW_TEXTAREA(BOOLEAN *refresh_screen)
{
    /*
     * See if the current link is in a form TEXTAREA.
     */
    if (LinkIsTextarea(curdoc.link)) {

	HText_ExpandTextarea(&links[curdoc.link], TEXTAREA_EXPAND_SIZE);

	*refresh_screen = TRUE;

    } else {

	HTInfoMsg(NOT_IN_TEXTAREA);
    }
}

static BOOLEAN handle_LYK_HEAD(int *cmd)
{
    int c;

    if (nlinks > 0 &&
	(links[curdoc.link].type != WWW_FORM_LINK_TYPE ||
	 links[curdoc.link].l_form->type == F_SUBMIT_TYPE ||
	 links[curdoc.link].l_form->type == F_IMAGE_SUBMIT_TYPE ||
	 links[curdoc.link].l_form->type == F_TEXT_SUBMIT_TYPE)) {
	/*
	 * We have links, and the current link is a normal link or a form's
	 * submit button.  - FM
	 */
	_statusline(HEAD_D_L_OR_CANCEL);
	c = LYgetch_single();
	if (c == 'D') {
	    char *scheme = !isLYNXIMGMAP(curdoc.address)
	    ? curdoc.address
	    : curdoc.address + LEN_LYNXIMGMAP;

	    if (LYCanDoHEAD(scheme) != TRUE) {
		HTUserMsg(DOC_NOT_HTTP_URL);
	    } else {
		/*
		 * Check if this is a reply from a POST, and if so, seek
		 * confirmation if the safe element is not set.  - FM
		 */
		if ((curdoc.post_data != NULL &&
		     curdoc.safe != TRUE) &&
		    HTConfirm(CONFIRM_POST_DOC_HEAD) == FALSE) {
		    HTInfoMsg(CANCELLED);
		} else {
		    HEAD_request = TRUE;
		    LYforce_no_cache = TRUE;
		    StrAllocCopy(newdoc.title, curdoc.title);
		    if (HTLoadedDocumentIsHEAD()) {
			HText_setNoCache(HTMainText);
			free_address(&curdoc);
		    } else {
			StrAllocCat(newdoc.title, " - HEAD");
		    }
		}
	    }
	} else if (c == 'L') {
	    if (links[curdoc.link].type != WWW_FORM_LINK_TYPE &&
		StrNCmp(links[curdoc.link].lname, "http", 4) &&
		StrNCmp(links[curdoc.link].lname, "LYNXIMGMAP:http", 15) &&
		LYCanDoHEAD(links[curdoc.link].lname) != TRUE &&
		(links[curdoc.link].type != WWW_INTERN_LINK_TYPE ||
		 !curdoc.address ||
		 StrNCmp(curdoc.address, "http", 4))) {
		HTUserMsg(LINK_NOT_HTTP_URL);
	    } else if (links[curdoc.link].type == WWW_FORM_LINK_TYPE &&
		       FormIsReadonly(links[curdoc.link].l_form)) {
		HTUserMsg(FORM_ACTION_DISABLED);
	    } else if (links[curdoc.link].type == WWW_FORM_LINK_TYPE &&
		       links[curdoc.link].l_form->submit_action != 0 &&
		       !isLYNXCGI(links[curdoc.link].l_form->submit_action) &&
		       StrNCmp(links[curdoc.link].l_form->submit_action,
			       "http", 4)) {
		HTUserMsg(FORM_ACTION_NOT_HTTP_URL);
	    } else if (links[curdoc.link].type == WWW_FORM_LINK_TYPE &&
		       links[curdoc.link].l_form->submit_method ==
		       URL_POST_METHOD &&
		       HTConfirm(CONFIRM_POST_LINK_HEAD) == FALSE) {
		HTInfoMsg(CANCELLED);
	    } else {
		HEAD_request = TRUE;
		LYforce_no_cache = TRUE;
		*cmd = LYK_ACTIVATE;
		return TRUE;
	    }
	}
    } else {
	/*
	 * We can offer only this document for a HEAD request.  Check if this
	 * is a reply from a POST, and if so, seek confirmation if the safe
	 * element is not set.  - FM
	 */
	if ((curdoc.post_data != NULL &&
	     curdoc.safe != TRUE) &&
	    HTConfirm(CONFIRM_POST_DOC_HEAD) == FALSE) {
	    HTInfoMsg(CANCELLED);
	} else {
	    if (nlinks > 0) {
		/*
		 * The current link is a non-submittable form link, so prompt
		 * the user to make it clear that the HEAD request would be for
		 * the current document, not the form link.  - FM
		 */
		_statusline(HEAD_D_OR_CANCEL);
		c = LYgetch_single();
	    } else {
		/*
		 * No links, so we can just assume that the user wants a HEAD
		 * request for the current document.  - FM
		 */
		c = 'D';
	    }
	    if (c == 'D') {
		char *scheme = !isLYNXIMGMAP(curdoc.address)
		? curdoc.address
		: curdoc.address + LEN_LYNXIMGMAP;

		/*
		 * The user didn't cancel, so check if a HEAD request is
		 * appropriate for the current document.  - FM
		 */
		if (LYCanDoHEAD(scheme) != TRUE) {
		    HTUserMsg(DOC_NOT_HTTP_URL);
		} else {
		    HEAD_request = TRUE;
		    LYforce_no_cache = TRUE;
		    StrAllocCopy(newdoc.title, curdoc.title);
		    if (HTLoadedDocumentIsHEAD()) {
			HText_setNoCache(HTMainText);
			free_address(&curdoc);
		    } else {
			StrAllocCat(newdoc.title, " - HEAD");
		    }
		}
	    }
	}
    }
    return FALSE;
}

static void handle_LYK_HELP(const char **cshelpfile)
{
    char *my_value = NULL;

    if (*cshelpfile == NULL)
	*cshelpfile = helpfile;
    StrAllocCopy(my_value, *cshelpfile);
    LYEnsureAbsoluteURL(&my_value, *cshelpfile, FALSE);
    if (!STREQ(curdoc.address, my_value)) {
	/*
	 * Set the filename.
	 */
	set_address(&newdoc, my_value);
	/*
	 * Make a name for this help file.
	 */
	StrAllocCopy(newdoc.title, gettext("Help Screen"));
	LYFreePostData(&newdoc);
	FREE(newdoc.bookmark);
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	newdoc.internal_link = FALSE;
    }
    FREE(my_value);
    *cshelpfile = NULL;		/* reset pointer - kw */
}

static void handle_LYK_HISTORICAL(void)
{
#ifdef USE_SOURCE_CACHE
    if (!HTcan_reparse_document()) {
#endif
	/*
	 * Check if this is a reply from a POST, and if so, seek confirmation
	 * of reload if the safe element is not set.  - FM
	 */
	if ((curdoc.post_data != NULL &&
	     curdoc.safe != TRUE) &&
	    confirm_post_resub(curdoc.address, NULL, 0, 0) == FALSE) {
	    HTInfoMsg(WILL_NOT_RELOAD_DOC);
	} else {
	    HText_setNoCache(HTMainText);
	    move_address(&newdoc, &curdoc);
	    newdoc.line = curdoc.line;
	    newdoc.link = curdoc.link;
	}
#ifdef USE_SOURCE_CACHE
    }				/* end if no bypass */
#endif
    historical_comments = (BOOLEAN) !historical_comments;
    if (minimal_comments) {
	HTAlert(historical_comments ?
		HISTORICAL_ON_MINIMAL_OFF : HISTORICAL_OFF_MINIMAL_ON);
    } else {
	HTAlert(historical_comments ?
		HISTORICAL_ON_VALID_OFF : HISTORICAL_OFF_VALID_ON);
    }
#ifdef USE_SOURCE_CACHE
    (void) reparse_document();
#endif
    return;
}

static BOOLEAN handle_LYK_HISTORY(int ForcePush)
{
    if (curdoc.title && !LYIsUIPage(curdoc.address, UIP_HISTORY)) {
	/*
	 * Don't do this if already viewing history page.
	 *
	 * Push the current file so that the history list contains the current
	 * file for printing purposes.  Pop the file afterwards to prevent
	 * multiple copies.
	 */
	if (TRACE && !LYUseTraceLog && LYCursesON) {
	    LYHideCursor();	/* make sure cursor is down */
#ifdef USE_SLANG
	    LYaddstr("\n");
#endif /* USE_SLANG */
	    LYrefresh();
	}
	LYpush(&curdoc, ForcePush);

	/*
	 * Print history options to file.
	 */
	if (showhistory(&newdoc.address) < 0) {
	    LYpop(&curdoc);
	    return TRUE;
	}
	LYRegisterUIPage(newdoc.address, UIP_HISTORY);
	StrAllocCopy(newdoc.title, HISTORY_PAGE_TITLE);
	LYFreePostData(&newdoc);
	FREE(newdoc.bookmark);
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	newdoc.internal_link = FALSE;
	newdoc.link = 1;	/*@@@ bypass "recent statusline messages" link */
	free_address(&curdoc);	/* so it doesn't get pushed */

	if (LYValidate || check_realm) {
	    LYPermitURL = TRUE;
	}
	return TRUE;
    }				/* end if StrNCmp */
    return FALSE;
}

static BOOLEAN handle_LYK_IMAGE_TOGGLE(int *cmd)
{
    clickable_images = (BOOLEAN) !clickable_images;

    HTUserMsg(clickable_images ?
	      CLICKABLE_IMAGES_ON : CLICKABLE_IMAGES_OFF);
    return reparse_or_reload(cmd);
}

static void handle_LYK_INDEX(int *old_c,
			     int real_c)
{
    /*
     * Make sure we are not in the index already.
     */
    if (!STREQ(curdoc.address, indexfile)) {

	if (indexfile[0] == '\0') {	/* no defined index */
	    if (*old_c != real_c) {
		*old_c = real_c;
		HTUserMsg(NO_INDEX_FILE);
	    }

	} else {
#ifdef KANJI_CODE_OVERRIDE
	    if (HTCJK == JAPANESE) {
		last_kcode = NOKANJI;	/* AUTO */
	    }
#endif
#ifdef USE_PROGRAM_DIR
	    if (is_url(indexfile) == 0) {
		char *tmp = NULL;

		HTSprintf0(&tmp, "%s\\%s", program_dir, indexfile);
		FREE(indexfile);
		LYLocalFileToURL(&indexfile, tmp);
		FREE(tmp);
	    }
#endif
	    set_address(&newdoc, indexfile);
	    StrAllocCopy(newdoc.title, gettext("System Index"));	/* name it */
	    LYFreePostData(&newdoc);
	    FREE(newdoc.bookmark);
	    newdoc.isHEAD = FALSE;
	    newdoc.safe = FALSE;
	    newdoc.internal_link = FALSE;
	}			/* end else */
    }				/* end if */
}

static void handle_LYK_INDEX_SEARCH(BOOLEAN *force_load,
				    int ForcePush,
				    int *old_c,
				    int real_c)
{
    if (is_www_index) {
	/*
	 * Perform a database search.
	 *
	 * do_www_search will try to go out and get the document.  If it
	 * returns TRUE, a new document was returned and is named in the
	 * newdoc.address.
	 */
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	if (do_www_search(&newdoc) == NORMAL) {
	    /*
	     * Yah, the search succeeded.
	     */
	    if (TRACE && !LYUseTraceLog && LYCursesON) {
		/*
		 * Make sure cursor is down.
		 */
		LYHideCursor();
#ifdef USE_SLANG
		LYaddstr("\n");
#endif /* USE_SLANG */
		LYrefresh();
	    }
	    LYpush(&curdoc, ForcePush);
	    /*
	     * Make the curdoc.address the newdoc.address so that getfile
	     * doesn't try to get the newdoc.address.  Since we have already
	     * gotten it.
	     */
	    copy_address(&curdoc, &newdoc);
	    BStrCopy(newdoc.post_data, curdoc.post_data);
	    StrAllocCopy(newdoc.post_content_type, curdoc.post_content_type);
	    newdoc.internal_link = FALSE;
	    curdoc.line = -1;
	    LYSetNewline(0);
	} else if (use_this_url_instead != NULL) {
	    /*
	     * Got back a redirecting URL.  Check it out.
	     */
	    HTUserMsg2(WWW_USING_MESSAGE, use_this_url_instead);

	    /*
	     * Make a name for this URL.
	     */
	    StrAllocCopy(newdoc.title,
			 "A URL specified by redirection");
	    set_address(&newdoc, use_this_url_instead);
	    LYFreePostData(&newdoc);
	    FREE(newdoc.bookmark);
	    newdoc.isHEAD = FALSE;
	    newdoc.safe = FALSE;
	    newdoc.internal_link = FALSE;
	    FREE(use_this_url_instead);
	    *force_load = TRUE;
	} else {
	    /*
	     * Yuk, the search failed.  Restore the old file.
	     */
	    copy_address(&newdoc, &curdoc);
	    BStrCopy(newdoc.post_data, curdoc.post_data);
	    StrAllocCopy(newdoc.post_content_type,
			 curdoc.post_content_type);
	    StrAllocCopy(newdoc.bookmark, curdoc.bookmark);
	    newdoc.isHEAD = curdoc.isHEAD;
	    newdoc.safe = curdoc.safe;
	    newdoc.internal_link = curdoc.internal_link;
	}
    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTUserMsg(NOT_ISINDEX);
    }
}

static BOOLEAN handle_LYK_INFO(int *cmd)
{
    /*
     * Don't do if already viewing info page.
     */
    if (!LYIsUIPage(curdoc.address, UIP_SHOWINFO)) {
	if (do_change_link() != -1
	    && LYShowInfo(&curdoc, &newdoc, owner_address) >= 0) {
	    LYRegisterUIPage(newdoc.address, UIP_SHOWINFO);
	    StrAllocCopy(newdoc.title, SHOWINFO_TITLE);
	    LYFreePostData(&newdoc);
	    FREE(newdoc.bookmark);
	    newdoc.isHEAD = FALSE;
	    newdoc.safe = FALSE;
	    newdoc.internal_link = FALSE;
	    LYforce_no_cache = TRUE;
	    if (LYValidate || check_realm)
		LYPermitURL = TRUE;
	}
    } else {
	/*
	 * If already in info page, get out.
	 */
	*cmd = LYK_PREV_DOC;
	return TRUE;
    }
    return FALSE;
}

static BOOLEAN handle_LYK_INLINE_TOGGLE(int *cmd)
{
    pseudo_inline_alts = (BOOLEAN) !pseudo_inline_alts;

    HTUserMsg(pseudo_inline_alts ?
	      PSEUDO_INLINE_ALTS_ON : PSEUDO_INLINE_ALTS_OFF);
    return reparse_or_reload(cmd);
}

static void handle_LYK_INSERT_FILE(BOOLEAN *refresh_screen,
				   int *old_c,
				   int real_c)
{
    /*
     * See if the current link is in a form TEXTAREA.
     */
    if (LinkIsTextarea(curdoc.link)) {

	/*
	 * Reject attempts to use this for gaining access to local files when
	 * such access is restricted:  if no_file_url was set via the file_url
	 * restriction, if no_goto_file was set for the anonymous account, or
	 * if HTDirAccess was set to HT_DIR_FORBID or HT_DIR_SELECTIVE via the
	 * -nobrowse or -selective switches, it is assumed that inserting files
	 * or checking for existence of files needs to be denied.  - kw
	 */
	if (no_file_url || no_goto_file ||
	    HTDirAccess == HT_DIR_FORBID ||
	    HTDirAccess == HT_DIR_SELECTIVE) {
	    if (*old_c != real_c) {
		*old_c = real_c;
		if (no_goto_file)
		    HTUserMsg2(GOTO_XXXX_DISALLOWED, STR_FILE_URL);
		else
		    HTUserMsg(NOAUTH_TO_ACCESS_FILES);
		HTInfoMsg(FILE_INSERT_CANCELLED);
	    }
	    return;
	}

	(void) HText_InsertFile(&links[curdoc.link]);

	/*
	 * TODO:
	 * Move cursor "n" lines from the current line to position it on the
	 * 1st line following the text that was inserted.  If the target
	 * line/anchor requires us to scroll up/down, position the target in
	 * the approximate center of the screen.
	 *
	 * [Current behavior leaves cursor on the same line relative to the
	 * start of the TEXTAREA that it was on before the insertion.  This is
	 * the same behavior that occurs with (my) editor, so this TODO will
	 * stay unimplemented.]
	 */

	*refresh_screen = TRUE;

    } else {

	HTInfoMsg(NOT_IN_TEXTAREA);
    }
}

#if defined(DIRED_SUPPORT) && defined(OK_INSTALL)
static void handle_LYK_INSTALL(void)
{
    if (lynx_edit_mode && nlinks > 0 && !no_dired_support)
	local_install(NULL, links[curdoc.link].lname, &newdoc.address);
}
#endif

static const char *hexy = "0123456789ABCDEF";

#define HEX(n) hexy[(n) & 0xf]
/*
 * URL-encode a parameter which can then be appended to a URI.
 * RFC-3986 lists reserved characters, which should be encoded.
 */
static char *urlencode(char *str)
{
    char *result = NULL;
    char *ptr;
    int ch;

    if (str != NULL) {
	result = malloc(strlen(str) * 3 + 1);
	ptr = result;

	if (result == NULL)
	    outofmem(__FILE__, "urlencode");

	while ((ch = UCH(*str++)) != 0) {
	    if (ch == ' ') {
		*ptr = '+';
		ptr++;
	    } else if (ch > 127 ||
		       StrChr(":/?#[]@!$&'()*+,;=", ch) != 0) {
		*ptr++ = '%';
		*ptr++ = HEX(ch >> 4);
		*ptr++ = HEX(ch);
	    } else {
		*ptr++ = (char) ch;
	    }
	}
	*ptr = '\0';
    }

    return result;
}

/*
 * Fill in "%s" marker(s) in the url_template by prompting the user for the
 * values.
 */
static BOOLEAN check_JUMP_param(char **url_template)
{
    int param = 1;
    char *subs;
    char *result = *url_template;
    char *encoded = NULL;
    int code = TRUE;
    bstring *input = NULL;

    CTRACE((tfp, "check_JUMP_param: %s\n", result));

    while ((subs = strstr(result, "%s")) != 0) {
	char prompt[MAX_LINE];
	RecallType recall = NORECALL;

	CTRACE((tfp, "Prompt for query param%d: %s\n", param, result));

	sprintf(prompt, gettext("Query parameter %d: "), param++);
	statusline(prompt);
	BStrCopy0(input, "");

	if (encoded)
	    FREE(encoded);

	if (LYgetBString(&input, FALSE, 0, recall) < 0) {
	    /*
	     * cancelled via ^G
	     */
	    HTInfoMsg(CANCELLED);
	    code = FALSE;
	    break;
	} else if (*(encoded = urlencode(input->str)) != '\0') {
	    int subs_at = (int) (subs - result);
	    int fill_in = (int) strlen(encoded) - 2;
	    size_t have = strlen(result);
	    size_t want = strlen(encoded) + have - 1;
	    int n;
	    char *update = realloc(result, want + 1);

	    if (update == 0) {
		HTInfoMsg(NOT_ENOUGH_MEMORY);
		code = FALSE;
		break;
	    }

	    CTRACE((tfp, "  reply: %s\n", input->str));
	    CTRACE((tfp, "  coded: %s\n", encoded));

	    result = update;
	    result[want] = '\0';
	    for (n = (int) want; (n - fill_in) >= subs_at; --n) {
		result[n] = result[n - fill_in];
	    }
	    for (n = subs_at; encoded[n - subs_at] != '\0'; ++n) {
		result[n] = encoded[n - subs_at];
	    }
	    CTRACE((tfp, "  subst: %s\n", result));
	} else {
	    HTInfoMsg(CANCELLED);
	    code = FALSE;
	    break;
	}
    }
    BStrFree(input);
    FREE(encoded);
    *url_template = result;
    return (BOOLEAN) code;
}

static void fill_JUMP_Params(char **addressp)
{
    if (LYJumpFileURL) {
	check_JUMP_param(addressp);
    }
}

static BOOLEAN handle_LYK_JUMP(int c,
			       bstring **user_input,
			       char **old_user_input GCC_UNUSED,
			       RecallType * recall GCC_UNUSED,
			       BOOLEAN *FirstURLRecall GCC_UNUSED,
			       int *URLNum GCC_UNUSED,
			       int *URLTotal GCC_UNUSED,
			       int *ch GCC_UNUSED,
			       int *old_c,
			       int real_c)
{
    char *ret;

    if (no_jump || JThead == NULL) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    if (no_jump)
		HTUserMsg(JUMP_DISALLOWED);
	    else
		HTUserMsg(NO_JUMPFILE);
	}
    } else {
	LYJumpFileURL = TRUE;
	if ((ret = LYJump(c)) != NULL) {
#ifdef PERMIT_GOTO_FROM_JUMP
	    if (!strncasecomp(ret, "Go ", 3)) {
		LYJumpFileURL = FALSE;
		StrAllocCopy(*old_user_input, (*user_input)->str);
		*URLTotal = (Goto_URLs ? HTList_count(Goto_URLs) : 0);
		*recall = ((*URLTotal >= 1) ? RECALL_URL : NORECALL);
		*URLNum = *URLTotal;
		*FirstURLRecall = TRUE;
		if (!strcasecomp(ret, "Go :")) {
		    if (recall) {
			*ch = UPARROW_KEY;
			return TRUE;
		    }
		    FREE(*old_user_input);
		    HTUserMsg(NO_RANDOM_URLS_YET);
		    return FALSE;
		}
		ret = HTParse((ret + 3), startfile, PARSE_ALL);
		BStrCopy0((*user_input), ret);
		FREE(ret);
		return TRUE;
	    }
#endif /* PERMIT_GOTO_FROM_JUMP */
	    ret = HTParse(ret, startfile, PARSE_ALL);
	    if (!LYTrimStartfile(ret)) {
		LYRemoveBlanks((*user_input)->str);
	    }
	    if (check_JUMP_param(&ret)) {
		set_address(&newdoc, ret);
		StrAllocCopy(lynxjumpfile, ret);
		LYFreePostData(&newdoc);
		FREE(newdoc.bookmark);
		newdoc.isHEAD = FALSE;
		newdoc.safe = FALSE;
		newdoc.internal_link = FALSE;
		LYUserSpecifiedURL = TRUE;
	    }
	    FREE(ret);
	} else {
	    LYJumpFileURL = FALSE;
	}
    }
    return FALSE;
}

static void handle_LYK_KEYMAP(BOOLEAN *vi_keys_flag,
			      BOOLEAN *emacs_keys_flag,
			      int *old_c,
			      int real_c)
{
    if (*old_c != real_c) {
	*old_c = real_c;
	set_address(&newdoc, STR_LYNXKEYMAP);
	StrAllocCopy(newdoc.title, CURRENT_KEYMAP_TITLE);
	LYFreePostData(&newdoc);
	FREE(newdoc.bookmark);
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	newdoc.internal_link = FALSE;
	/*
	 * If vi_keys changed, the keymap did too, so force no cache, and reset
	 * the flag.  - FM
	 */
	if (*vi_keys_flag != vi_keys ||
	    *emacs_keys_flag != emacs_keys) {
	    LYforce_no_cache = TRUE;
	    *vi_keys_flag = vi_keys;
	    *emacs_keys_flag = emacs_keys;
	}
#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
	/*
	 * Remember whether we are in dired menu so we can display the right
	 * keymap.
	 */
	if (!no_dired_support) {
	    prev_lynx_edit_mode = lynx_edit_mode;
	}
#endif /* DIRED_SUPPORT && OK_OVERRIDE */
	LYforce_no_cache = TRUE;
    }
}

static void handle_LYK_LAST_LINK(void)
{
    int i = curdoc.link;

    for (;;) {
	if (++i >= nlinks
	    || links[i].ly != links[curdoc.link].ly) {
	    set_curdoc_link(i - 1);
	    break;
	}
    }
}

static void handle_LYK_LEFT_LINK(void)
{
    if (curdoc.link > 0 &&
	links[curdoc.link].ly == links[curdoc.link - 1].ly) {
	set_curdoc_link(curdoc.link - 1);
    }
}

static BOOLEAN handle_LYK_LIST(int *cmd)
{
    /*
     * Don't do if already viewing list page.
     */
    if (!strcmp(NonNull(curdoc.title), LIST_PAGE_TITLE) &&
	LYIsUIPage(curdoc.address, UIP_LIST_PAGE)) {
	/*
	 * Already viewing list page, so get out.
	 */
	*cmd = LYK_PREV_DOC;
	return TRUE;
    }

    /*
     * Print list page to file.
     */
    if (showlist(&newdoc, TRUE) < 0)
	return FALSE;
    StrAllocCopy(newdoc.title, LIST_PAGE_TITLE);
    /*
     * showlist will set newdoc's other fields.  It may leave post_data intact
     * so the list can be used to follow internal links in the current document
     * even if it is a POST response.  - kw
     */

    if (LYValidate || check_realm) {
	LYPermitURL = TRUE;
	StrAllocCopy(lynxlistfile, newdoc.address);
    }
    return FALSE;
}

static void handle_LYK_MAIN_MENU(int *old_c,
				 int real_c)
{
    /*
     * If its already the homepage then don't reload it.
     */
    if (!STREQ(curdoc.address, homepage)) {

	if (HTConfirmDefault(CONFIRM_MAIN_SCREEN, NO) == YES) {
	    set_address(&newdoc, homepage);
	    StrAllocCopy(newdoc.title, gettext("Entry into main screen"));
	    LYFreePostData(&newdoc);
	    FREE(newdoc.bookmark);
	    newdoc.isHEAD = FALSE;
	    newdoc.safe = FALSE;
	    newdoc.internal_link = FALSE;
	    LYhighlight(FALSE, curdoc.link, prev_target->str);
#ifdef DIRED_SUPPORT
	    if (lynx_edit_mode) {
		DIRED_UNCACHE_2;
	    }
#endif /* DIRED_SUPPORT */
	}
    } else {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(IN_MAIN_SCREEN);
	}
    }
}

static void handle_LYK_MINIMAL(void)
{
    if (!historical_comments) {
#ifdef USE_SOURCE_CACHE
	if (!HTcan_reparse_document()) {
#endif
	    /*
	     * Check if this is a reply from a POST, and if so, seek
	     * confirmation of reload if the safe element is not set.  - FM
	     */
	    if ((curdoc.post_data != NULL &&
		 curdoc.safe != TRUE) &&
		confirm_post_resub(curdoc.address, NULL, 0, 0) == FALSE) {
		HTInfoMsg(WILL_NOT_RELOAD_DOC);
	    } else {
		HText_setNoCache(HTMainText);
		move_address(&newdoc, &curdoc);
		newdoc.line = curdoc.line;
		newdoc.link = curdoc.link;
	    }
#ifdef USE_SOURCE_CACHE
	}			/* end if no bypass */
#endif
    }
    minimal_comments = (BOOLEAN) !minimal_comments;
    if (!historical_comments) {
	HTAlert(minimal_comments ?
		MINIMAL_ON_IN_EFFECT : MINIMAL_OFF_VALID_ON);
    } else {
	HTAlert(minimal_comments ?
		MINIMAL_ON_BUT_HISTORICAL : MINIMAL_OFF_HISTORICAL_ON);
    }
#ifdef USE_SOURCE_CACHE
    (void) reparse_document();
#endif
    return;
}

#if defined(DIRED_SUPPORT)
static void handle_LYK_MODIFY(BOOLEAN *refresh_screen)
{
    if (lynx_edit_mode && nlinks > 0 && !no_dired_support) {
	int ret;

	ret = local_modify(&curdoc, &newdoc.address);
	if (ret == PERMIT_FORM_RESULT) {	/* Permit form thrown up */
	    *refresh_screen = TRUE;
	} else if (ret) {
	    DIRED_UNCACHE_1;
	    move_address(&newdoc, &curdoc);
	    LYFreePostData(&newdoc);
	    FREE(newdoc.bookmark);
	    newdoc.isHEAD = FALSE;
	    newdoc.safe = FALSE;
	    newdoc.internal_link = FALSE;
	    newdoc.line = curdoc.line;
	    newdoc.link = curdoc.link;
	    LYclear();
	}
    }
}
#endif /* DIRED_SUPPORT */

#ifdef EXP_NESTED_TABLES
static BOOLEAN handle_LYK_NESTED_TABLES(int *cmd)
{
    nested_tables = (BOOLEAN) !nested_tables;
    HTUserMsg(nested_tables ? NESTED_TABLES_ON : NESTED_TABLES_OFF);
    return reparse_or_reload(cmd);
}
#endif

static BOOLEAN handle_LYK_OPTIONS(int *cmd,
				  BOOLEAN *refresh_screen)
{
#ifndef NO_OPTION_MENU
    if (!LYUseFormsOptions) {
	BOOLEAN LYUseDefaultRawMode_flag = LYUseDefaultRawMode;
	BOOLEAN LYSelectPopups_flag = LYSelectPopups;
	BOOLEAN verbose_img_flag = verbose_img;
	BOOLEAN keypad_mode_flag = (BOOL) keypad_mode;
	BOOLEAN show_dotfiles_flag = show_dotfiles;
	BOOLEAN user_mode_flag = (BOOL) user_mode;
	int CurrentAssumeCharSet_flag = UCLYhndl_for_unspec;
	int CurrentCharSet_flag = current_char_set;
	int HTfileSortMethod_flag = HTfileSortMethod;
	char *CurrentUserAgent = NULL;
	char *CurrentNegoLanguage = NULL;
	char *CurrentNegoCharset = NULL;

	StrAllocCopy(CurrentUserAgent, NonNull(LYUserAgent));
	StrAllocCopy(CurrentNegoLanguage, NonNull(language));
	StrAllocCopy(CurrentNegoCharset, NonNull(pref_charset));

	LYoptions(); /** do the old-style options stuff **/

	if (keypad_mode_flag != keypad_mode ||
	    (user_mode_flag != user_mode &&
	     (user_mode_flag == NOVICE_MODE ||
	      user_mode == NOVICE_MODE)) ||
	    (((HTfileSortMethod_flag != HTfileSortMethod) ||
	      (show_dotfiles_flag != show_dotfiles)) &&
	     (isFILE_URL(curdoc.address) ||
	      isFTP_URL(curdoc.address))) ||
	    CurrentCharSet_flag != current_char_set ||
	    CurrentAssumeCharSet_flag != UCLYhndl_for_unspec ||
	    verbose_img_flag != verbose_img ||
	    LYUseDefaultRawMode_flag != LYUseDefaultRawMode ||
	    LYSelectPopups_flag != LYSelectPopups ||
	    ((strcmp(CurrentUserAgent, NonNull(LYUserAgent)) ||
	      strcmp(CurrentNegoLanguage, NonNull(language)) ||
	      strcmp(CurrentNegoCharset, NonNull(pref_charset))) &&
	     (!StrNCmp(curdoc.address, "http", 4) ||
	      isLYNXCGI(curdoc.address)))) {

	    BOOLEAN canreparse_post = FALSE;

	    /*
	     * Check if this is a reply from a POST, and if so, seek
	     * confirmation of reload if the safe element is not set.  - FM
	     */
	    if ((curdoc.post_data != NULL &&
		 curdoc.safe != TRUE) &&
#ifdef USE_SOURCE_CACHE
		(!(canreparse_post = HTcan_reparse_document())) &&
#endif
		confirm_post_resub(curdoc.address, curdoc.title,
				   2, 1) == FALSE) {
		HTInfoMsg(WILL_NOT_RELOAD_DOC);
	    } else {
		copy_address(&newdoc, &curdoc);
		if (((strcmp(CurrentUserAgent, NonNull(LYUserAgent)) ||
		      strcmp(CurrentNegoLanguage, NonNull(language)) ||
		      strcmp(CurrentNegoCharset, NonNull(pref_charset))) &&
		     (StrNCmp(curdoc.address, "http", 4) == 0 ||
		      isLYNXCGI(curdoc.address)))) {
		    /*
		     * An option has changed which may influence content
		     * negotiation, and the resource is from a http or https or
		     * lynxcgi URL (the only protocols which currently do
		     * anything with this information).  Set reloading = TRUE
		     * so that proxy caches will be flushed, which is necessary
		     * until the time when all proxies understand HTTP 1.1
		     * Vary:  and all Servers properly use it...  Treat like
		     * case LYK_RELOAD (see comments there).  - KW
		     */
		    reloading = TRUE;
		}
		if (HTisDocumentSource()) {
		    srcmode_for_next_retrieval(1);
		}
#ifdef USE_SOURCE_CACHE
		if (reloading == FALSE) {
		    /* one more attempt to be smart enough: */
		    if (reparse_document()) {
			FREE(CurrentUserAgent);
			FREE(CurrentNegoLanguage);
			FREE(CurrentNegoCharset);
			return FALSE;
		    }
		}
#endif
		if (canreparse_post &&
		    confirm_post_resub(curdoc.address, curdoc.title,
				       2, 1) == FALSE) {
		    if (HTisDocumentSource()) {
			srcmode_for_next_retrieval(0);
		    }
		    FREE(CurrentUserAgent);
		    FREE(CurrentNegoLanguage);
		    FREE(CurrentNegoCharset);
		    return FALSE;
		}

		HEAD_request = HTLoadedDocumentIsHEAD();
		HText_setNoCache(HTMainText);
		newdoc.line = curdoc.line;
		newdoc.link = curdoc.link;
		LYforce_no_cache = TRUE;
		free_address(&curdoc);	/* So it doesn't get pushed. */
	    }
	}
	FREE(CurrentUserAgent);
	FREE(CurrentNegoLanguage);
	FREE(CurrentNegoCharset);
	*refresh_screen = TRUE;	/* to repaint screen */
	return FALSE;
    }				/* end if !LYUseFormsOptions */
#endif /* !NO_OPTION_MENU */
#ifndef NO_OPTION_FORMS
    /*
     * Generally stolen from LYK_COOKIE_JAR.  Options menu handling is
     * done in postoptions(), called from getfile() currently.
     *
     * postoptions() is also responsible for reloading the document
     * before the 'options menu' but only when (a few) important
     * options were changed.
     *
     * It is critical that post_data is freed here since the
     * submission of changed options is done via the same protocol as
     * LYNXOPTIONS:
     */
    /*
     * Don't do if already viewing options page.
     */
    if (!LYIsUIPage(curdoc.address, UIP_OPTIONS_MENU)) {

	set_address(&newdoc, LYNXOPTIONS_PAGE("/"));
	LYFreePostData(&newdoc);
	FREE(newdoc.bookmark);
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	newdoc.internal_link = FALSE;
	LYforce_no_cache = TRUE;
	/* change to 'if (check_realm && !LYValidate)' and
	   make change near top of getfile to forbid
	   using forms options menu with -validate:  - kw */
	if (LYValidate || check_realm) {
	    LYPermitURL = TRUE;
	}
    } else {
	/*
	 * If already in the options menu, get out.
	 */
	*cmd = LYK_PREV_DOC;
	return TRUE;
    }
#endif /* !NO_OPTION_FORMS */
    return FALSE;
}

static void handle_NEXT_DOC(void)
{
    if (LYhist_next(&curdoc, &newdoc)) {
	free_address(&curdoc);	/* avoid push */
	return;
    }
    HTUserMsg(gettext("No next document present"));
}

static void handle_LYK_NEXT_LINK(int c,
				 int *old_c,
				 int real_c)
{
    if (curdoc.link < nlinks - 1) {	/* next link */
	LYhighlight(FALSE, curdoc.link, prev_target->str);
#ifdef FASTTAB
	/*
	 * Move to different textarea if TAB in textarea.
	 */
	if (LinkIsTextarea(curdoc.link) &&
	    c == '\t') {
	    int thisgroup = links[curdoc.link].l_form->number;
	    char *thisname = links[curdoc.link].l_form->name;

	    do
		curdoc.link++;
	    while ((curdoc.link < nlinks - 1) &&
		   LinkIsTextarea(curdoc.link) &&
		   links[curdoc.link].l_form->number == thisgroup &&
		   sametext(links[curdoc.link].l_form->name, thisname));
	} else {
	    curdoc.link++;
	}
#else
	curdoc.link++;
#endif /* FASTTAB */
	/*
	 * At the bottom of list and there is only one page.  Move to the top
	 * link on the page.
	 */
    } else if (!more_text && LYGetNewline() == 1 && curdoc.link == nlinks - 1) {
	set_curdoc_link(0);

    } else if (more_text) {	/* next page */
	LYChgNewline(display_lines);
    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTInfoMsg(ALREADY_AT_END);
    }
}

static void handle_LYK_NEXT_PAGE(int *old_c,
				 int real_c)
{
    if (more_text) {
	LYChgNewline(display_lines);
    } else if (curdoc.link < nlinks - 1) {
	set_curdoc_link(nlinks - 1);
    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTInfoMsg(ALREADY_AT_END);
    }
}

static BOOLEAN handle_LYK_NOCACHE(int *old_c,
				  int real_c)
{
    if (nlinks > 0) {
	if (links[curdoc.link].type == WWW_FORM_LINK_TYPE &&
	    links[curdoc.link].l_form->type != F_SUBMIT_TYPE &&
	    links[curdoc.link].l_form->type != F_IMAGE_SUBMIT_TYPE &&
	    links[curdoc.link].l_form->type != F_TEXT_SUBMIT_TYPE) {
	    if (*old_c != real_c) {
		*old_c = real_c;
		HTUserMsg(NOT_ON_SUBMIT_OR_LINK);
	    }
	    return FALSE;
	} else {
	    LYforce_no_cache = TRUE;
	    reloading = TRUE;
	}
    }
    return TRUE;
}

static void handle_LYK_PREV_LINK(int *arrowup,
				 int *old_c,
				 int real_c)
{
    if (curdoc.link > 0) {	/* previous link */
	set_curdoc_link(curdoc.link - 1);

    } else if (!more_text &&
	       curdoc.link == 0 && LYGetNewline() == 1) {	/* at the top of list */
	/*
	 * If there is only one page of data and the user goes off the top,
	 * just move the cursor to last link on the page.
	 */
	set_curdoc_link(nlinks - 1);

    } else if (curdoc.line > 1) {	/* previous page */
	/*
	 * Go back to the previous page.
	 */
	int scrollamount = (LYGetNewline() > display_lines
			    ? display_lines
			    : LYGetNewline() - 1);

	LYChgNewline(-scrollamount);
	if (scrollamount < display_lines &&
	    nlinks > 0 && curdoc.link == 0 &&
	    links[0].ly - 1 + scrollamount <= display_lines) {
	    newdoc.link = HText_LinksInLines(HTMainText,
					     1,
					     scrollamount) - 1;
	} else {
	    *arrowup = TRUE;
	}

    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTInfoMsg(ALREADY_AT_BEGIN);
    }
}

#define nhist_1 (nhist - 1)	/* workaround for indent */

static int handle_PREV_DOC(int *cmd,
			   int *old_c,
			   int real_c)
{
    if (nhist > 0) {		/* if there is anything to go back to */
	/*
	 * Check if the previous document is a reply from a POST, and if so,
	 * seek confirmation of resubmission if the safe element is not set and
	 * the document is not still in the cache or LYresubmit_posts is set.
	 * If not confirmed and it is not the startfile, pop it so we go to the
	 * yet previous document, until we're OK or reach the startfile.  If we
	 * reach the startfile and its not OK or we don't get confirmation,
	 * cancel.  - FM
	 */
	DocAddress WWWDoc;
	HTParentAnchor *tmpanchor;
	BOOLEAN conf = FALSE, first = TRUE;

	HTLastConfirmCancelled();	/* reset flag */
	while (nhist > 0) {
	    conf = FALSE;
	    if (HDOC(nhist_1).post_data == NULL) {
		break;
	    }
	    WWWDoc.address = HDOC(nhist_1).address;
	    WWWDoc.post_data = HDOC(nhist_1).post_data;
	    WWWDoc.post_content_type =
		HDOC(nhist_1).post_content_type;
	    WWWDoc.bookmark = HDOC(nhist_1).bookmark;
	    WWWDoc.isHEAD = HDOC(nhist_1).isHEAD;
	    WWWDoc.safe = HDOC(nhist_1).safe;
	    tmpanchor = HTAnchor_findAddress(&WWWDoc);
	    if (HTAnchor_safe(tmpanchor)) {
		break;
	    }
	    if ((HTAnchor_document(tmpanchor) == NULL &&
		 (isLYNXIMGMAP(WWWDoc.address) ||
		  (conf = confirm_post_resub(WWWDoc.address,
					     HDOC(nhist_1).title,
					     0, 0))
		  == FALSE)) ||
		((LYresubmit_posts && !conf &&
		  (NONINTERNAL_OR_PHYS_DIFFERENT((DocInfo *) &history[(nhist_1)],
						 &curdoc) ||
		   NONINTERNAL_OR_PHYS_DIFFERENT((DocInfo *) &history[(nhist_1)],
						 &newdoc))) &&
		 !confirm_post_resub(WWWDoc.address,
				     HDOC(nhist_1).title,
				     2, 2))) {
		if (HTLastConfirmCancelled()) {
		    if (!first && curdoc.internal_link)
			free_address(&curdoc);
		    *cmd = LYK_DO_NOTHING;
		    return 2;
		}
		if (nhist == 1) {
		    HTInfoMsg(CANCELLED);
		    *old_c = 0;
		    *cmd = LYK_DO_NOTHING;
		    return 2;
		} else {
		    HTUserMsg2(WWW_SKIP_MESSAGE, WWWDoc.address);
		    do {	/* Should be LYhist_prev when _next supports */
			LYpop(&curdoc);		/* skipping of forms */
		    } while (nhist > 1
			     && !are_different((DocInfo *) &history[nhist_1],
					       &curdoc));
		    first = FALSE;	/* have popped at least one */
		    continue;
		}
	    } else {
		/*
		 * Break from loop; if user just confirmed to load again
		 * because document wasn't in cache, set LYforce_no_cache to
		 * avoid unnecessary repeat question down the road.  - kw
		 */
		if (conf)
		    LYforce_no_cache = TRUE;
		break;
	    }
	}

	if (!first)
	    curdoc.internal_link = FALSE;

	/*
	 * Set newdoc.address to empty to pop a file.
	 */
	LYhist_prev_register(&curdoc);	/* Why not call _prev instead of zeroing address?  */
	free_address(&newdoc);
#ifdef DIRED_SUPPORT
	if (lynx_edit_mode) {
	    DIRED_UNCACHE_2;
	}
#endif /* DIRED_SUPPORT */
    } else if (child_lynx == TRUE) {
	return (1);		/* exit on left arrow in main screen */

    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTUserMsg(ALREADY_AT_FIRST);
    }
    return 0;
}

static void handle_LYK_PREV_PAGE(int *old_c,
				 int real_c)
{
    if (LYGetNewline() > 1) {
	LYChgNewline(-display_lines);
    } else if (curdoc.link > 0) {
	set_curdoc_link(0);
    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTInfoMsg(ALREADY_AT_BEGIN);
    }
}

static void handle_LYK_PRINT(BOOLEAN *ForcePush,
			     int *old_c,
			     int real_c)
{
    if (LYValidate) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(PRINT_DISABLED);
	}
	return;
    }

    /*
     * Don't do if already viewing print options page.
     */
    if (!LYIsUIPage(curdoc.address, UIP_PRINT_OPTIONS)
	&& print_options(&newdoc.address,
			 curdoc.address, HText_getNumOfLines()) >= 0) {
	LYRegisterUIPage(newdoc.address, UIP_PRINT_OPTIONS);
	StrAllocCopy(newdoc.title, PRINT_OPTIONS_TITLE);
	LYFreePostData(&newdoc);
	FREE(newdoc.bookmark);
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	*ForcePush = TRUE;	/* see LYpush() and print_options() */
	if (check_realm)
	    LYPermitURL = TRUE;
    }
}

static BOOLEAN handle_LYK_QUIT(void)
{
    int c;

    if (LYQuitDefaultYes == TRUE) {
	c = HTConfirmDefault(REALLY_QUIT, YES);
    } else {
	c = HTConfirmDefault(REALLY_QUIT, NO);
    }
    if (LYQuitDefaultYes == TRUE) {
	if (c != NO) {
	    return (TRUE);
	} else {
	    HTInfoMsg(NO_CANCEL);
	}
    } else if (c == YES) {
	return (TRUE);
    } else {
	HTInfoMsg(NO_CANCEL);
    }
    return FALSE;
}

static BOOLEAN handle_LYK_RAW_TOGGLE(int *cmd)
{
    if (HTLoadedDocumentCharset()) {
	HTUserMsg(gettext("charset for this document specified explicitly, sorry..."));
	return FALSE;
    } else {
	LYUseDefaultRawMode = (BOOL) !LYUseDefaultRawMode;
	HTUserMsg(LYRawMode ? RAWMODE_OFF : RAWMODE_ON);
	HTMLSetCharacterHandling(current_char_set);
	return reparse_or_reload(cmd);
    }
}

static void handle_LYK_RELOAD(int real_cmd)
{
    /*
     * Check if this is a reply from a POST, and if so,
     * seek confirmation if the safe element is not set.  - FM
     */
    if ((curdoc.post_data != NULL &&
	 curdoc.safe != TRUE) &&
	HTConfirm(CONFIRM_POST_RESUBMISSION) == FALSE) {
	HTInfoMsg(CANCELLED);
	return;
    }

    /*
     * Check to see if should reload source, or load html
     */

    if (HTisDocumentSource()) {
	if ((forced_UCLYhdnl = HTMainText_Get_UCLYhndl()) >= 0)
	    force_old_UCLYhndl_on_reload = TRUE;
	srcmode_for_next_retrieval(1);
    }

    HEAD_request = HTLoadedDocumentIsHEAD();
    HText_setNoCache(HTMainText);
    /*
     * Do assume the reloaded document will be the same.  - FM
     *
     * (I don't remember all the reasons why we couldn't assume this.  As the
     * problems show up, we'll try to fix them, or add warnings.  - FM)
     */
    newdoc.line = curdoc.line;
    newdoc.link = curdoc.link;
    free_address(&curdoc);	/* so it doesn't get pushed */
#ifdef VMS
    lynx_force_repaint();
#endif /* VMS */
    /*
     * Reload should force a cache refresh on a proxy.  -- Ari L.
     * <luotonen@dxcern.cern.ch>
     *
     * -- but only if this was really a reload requested by the user, not if we
     * jumped here to handle reloading for INLINE_TOGGLE, IMAGE_TOGGLE,
     * RAW_TOGGLE, etc.  - KW
     */
    if (real_cmd == LYK_RELOAD)
	reloading = REAL_RELOAD;

    return;
}

#ifdef DIRED_SUPPORT
static void handle_LYK_REMOVE(BOOLEAN *refresh_screen)
{
    if (lynx_edit_mode && nlinks > 0 && !no_dired_support) {
	int linkno = curdoc.link;	/* may be changed in local_remove - kw */

	local_remove(&curdoc);
	if (LYAutoUncacheDirLists >= 1)
	    do_cleanup_after_delete();
	else if (curdoc.link != linkno)
	    *refresh_screen = TRUE;
    }
}
#endif /* DIRED_SUPPORT */

static void handle_LYK_RIGHT_LINK(void)
{
    if (curdoc.link < nlinks - 1 &&
	links[curdoc.link].ly == links[curdoc.link + 1].ly) {
	set_curdoc_link(curdoc.link + 1);
    }
}

static void handle_LYK_SHELL(BOOLEAN *refresh_screen,
			     int *old_c,
			     int real_c)
{
    if (!no_shell) {
	stop_curses();
	printf("%s\r\n", SPAWNING_MSG);
#if defined(__CYGWIN__)
	/* handling "exec $SHELL" does not work if $SHELL is null */
	if (LYGetEnv("SHELL") == NULL) {
	    Cygwin_Shell();
	} else
#endif
	{
	    static char *shell = NULL;

	    if (shell == 0)
		StrAllocCopy(shell, LYSysShell());
	    LYSystem(shell);
	}
	start_curses();
	*refresh_screen = TRUE;	/* for an HText_pageDisplay() */
    } else {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(SPAWNING_DISABLED);
	}
    }
}

static void handle_LYK_SOFT_DQUOTES(void)
{
#ifdef USE_SOURCE_CACHE
    if (!HTcan_reparse_document()) {
#endif
	/*
	 * Check if this is a reply from a POST, and if so, seek confirmation
	 * of reload if the safe element is not set.  - FM
	 */
	if ((curdoc.post_data != NULL &&
	     curdoc.safe != TRUE) &&
	    confirm_post_resub(curdoc.address, NULL, 1, 1) == FALSE) {
	    HTInfoMsg(WILL_NOT_RELOAD_DOC);
	} else {
	    HText_setNoCache(HTMainText);
	    move_address(&newdoc, &curdoc);
	    newdoc.line = curdoc.line;
	    newdoc.link = curdoc.link;
	}
#ifdef USE_SOURCE_CACHE
    }				/* end if no bypass */
#endif
    soft_dquotes = (BOOLEAN) !soft_dquotes;
    HTUserMsg(soft_dquotes ?
	      SOFT_DOUBLE_QUOTE_ON : SOFT_DOUBLE_QUOTE_OFF);
#ifdef USE_SOURCE_CACHE
    (void) reparse_document();
#endif
    return;
}

#define GetAnchorNumber(link) \
			((nlinks > 0 && link >= 0) \
    			 ? links[link].anchor_number \
			 : -1)
#define GetAnchorLineNo(link) \
			((nlinks > 0 && link >= 0) \
    			 ? links[link].anchor_line_num \
			 : -1)

/*
 * Adjust the top-of-screen line number for the new document if the redisplayed
 * screen would not show the given link-number.
 */
#ifdef USE_SOURCE_CACHE
static int wrap_reparse_document(void)
{
    int result;
    int anchor_number = GetAnchorNumber(curdoc.link);
    int old_line_num = HText_getAbsLineNumber(HTMainText, anchor_number);
    int old_from_top = old_line_num - LYGetNewline() + 1;

    /* get the offset for the current anchor */
    int old_offset = ((nlinks > 0 && curdoc.link >= 0)
		      ? links[curdoc.link].sgml_offset
		      : -1);

    CTRACE((tfp, "original anchor %d, topline %d, link %d, offset %d\n",
	    anchor_number, old_line_num, curdoc.link, old_offset));

    /* reparse the document (producing a new anchor list) */
    result = reparse_document();

    /* readjust top-line and link-number */
    if (result && old_offset >= 0) {
	int new_anchor = HText_closestAnchor(HTMainText, old_offset);
	int new_lineno = HText_getAbsLineNumber(HTMainText, new_anchor);
	int top_lineno;

	CTRACE((tfp, "old anchor %d -> new anchor %d\n", anchor_number, new_anchor));

	if (new_lineno - old_from_top < 0)
	    old_from_top = new_lineno;

	/* Newline and newdoc.line are 1-based,
	 * but 0-based lines are simpler to work with.
	 */
	top_lineno = HText_getPreferredTopLine(HTMainText, new_lineno -
					       old_from_top) + 1;
	CTRACE((tfp, "preferred top %d\n", top_lineno));

	if (top_lineno != LYGetNewline()) {
	    LYSetNewline(top_lineno);
	    newdoc.link = HText_anchorRelativeTo(HTMainText, top_lineno - 1, new_anchor);
	    curdoc.link = newdoc.link;
	    CTRACE((tfp,
		    "adjusted anchor %d, topline %d, link %d, offset %d\n",
		    new_anchor,
		    top_lineno,
		    curdoc.link,
		    HText_locateAnchor(HTMainText, new_anchor)));
	} else {
	    newdoc.link = curdoc.link;
	}
    }
    return result;
}
#endif /* USE_SOURCE_CACHE */

static void handle_LYK_SOURCE(char **ownerS_address_p)
{
#ifdef USE_SOURCE_CACHE
    BOOLEAN canreparse_post = FALSE;
#endif

    /*
     * Check if this is a reply from a POST, and if so,
     * seek confirmation if the safe element is not set.  - FM
     */
    if ((curdoc.post_data != NULL &&
	 curdoc.safe != TRUE) &&
#ifdef USE_SOURCE_CACHE
	(!(canreparse_post = HTcan_reparse_document())) &&
#endif
	(curdoc.isHEAD ? HTConfirm(CONFIRM_POST_RESUBMISSION) :
	 confirm_post_resub(curdoc.address, curdoc.title, 1, 1)) == FALSE) {
	HTInfoMsg(CANCELLED);
	return;
    }

    if (HTisDocumentSource()) {
	srcmode_for_next_retrieval(-1);
    } else {
	if (HText_getOwner())
	    StrAllocCopy(*ownerS_address_p, HText_getOwner());
	LYUCPushAssumed(HTMainAnchor);
	srcmode_for_next_retrieval(1);
    }

#ifdef USE_SOURCE_CACHE
    if (wrap_reparse_document()) {
	/*
	 * These normally get cleaned up after getfile() returns;
	 * since we're not calling getfile(), we have to clean them
	 * up ourselves.  -dsb
	 */
	HTOutputFormat = WWW_PRESENT;
#ifdef USE_PRETTYSRC
	if (psrc_view)
	    HTMark_asSource();
	psrc_view = FALSE;
#endif
	FREE(*ownerS_address_p);	/* not used with source_cache */
	LYUCPopAssumed();	/* probably a right place here */
	HTMLSetCharacterHandling(current_char_set);	/* restore now */

	return;
    } else if (canreparse_post) {
	srcmode_for_next_retrieval(0);
	LYUCPopAssumed();	/* probably a right place here */
	return;
    }
#endif

    if (curdoc.title)
	StrAllocCopy(newdoc.title, curdoc.title);

    free_address(&curdoc);	/* so it doesn't get pushed */
    LYforce_no_cache = TRUE;
}

static void handle_LYK_SWITCH_DTD(void)
{
#ifdef USE_SOURCE_CACHE
    BOOLEAN canreparse = FALSE;

    if (!(canreparse = HTcan_reparse_document())) {
#endif
	/*
	 * Check if this is a reply from a POST, and if so,
	 * seek confirmation of reload if the safe element
	 * is not set.  - FM, kw
	 */
	if ((curdoc.post_data != NULL &&
	     curdoc.safe != TRUE) &&
	    confirm_post_resub(curdoc.address, NULL, 1, 1) == FALSE) {
	    HTInfoMsg(WILL_NOT_RELOAD_DOC);
	} else {
	    /*
	     * If currently viewing preparsed source, switching to the other
	     * DTD parsing may show source differences, so stay in source view
	     * - kw
	     */

	    /* NOTE: this conditional can be considered incorrect -
	       current behaviour - when viewing source and
	       LYPreparsedSource==TRUE, pressing ^V will toggle parser mode
	       AND switch back from the source view to presentation view.-HV
	     */
	    if (HTisDocumentSource() && LYPreparsedSource) {
		srcmode_for_next_retrieval(1);
	    }
	    HText_setNoCache(HTMainText);
	    move_address(&newdoc, &curdoc);
	    newdoc.line = curdoc.line;
	    newdoc.link = curdoc.link;
	}
#ifdef USE_SOURCE_CACHE
    }				/* end if no bypass */
#endif
    Old_DTD = !Old_DTD;
    HTSwitchDTD(!Old_DTD);
    HTUserMsg(Old_DTD ? USING_DTD_0 : USING_DTD_1);
#ifdef USE_SOURCE_CACHE
    if (canreparse) {
	if (HTisDocumentSource() && LYPreparsedSource) {
	    srcmode_for_next_retrieval(1);
	}
	if (!reparse_document()) {
	    srcmode_for_next_retrieval(0);
	}
    }
#endif
    return;
}

#ifdef DIRED_SUPPORT
static void handle_LYK_TAG_LINK(void)
{
    if (lynx_edit_mode && nlinks > 0 && !no_dired_support) {
	if (!strcmp(LYGetHiliteStr(curdoc.link, 0), ".."))
	    return;		/* Never tag the parent directory */
	if (dir_list_style == MIXED_STYLE) {
	    if (!strcmp(LYGetHiliteStr(curdoc.link, 0), "../"))
		return;
	} else if (!StrNCmp(LYGetHiliteStr(curdoc.link, 0), "Up to ", 6))
	    return;
	{
	    /*
	     * HTList-based management of tag list, see LYLocal.c - KW
	     */
	    HTList *t1 = tagged;
	    char *tagname = NULL;
	    BOOLEAN found = FALSE;

	    while ((tagname = (char *) HTList_nextObject(t1)) != NULL) {
		if (!strcmp(links[curdoc.link].lname, tagname)) {
		    found = TRUE;
		    HTList_removeObject(tagged, tagname);
		    FREE(tagname);
		    tagflag(FALSE, curdoc.link);
		    break;
		}
	    }
	    if (!found) {
		if (tagged == NULL)
		    tagged = HTList_new();
		tagname = NULL;
		StrAllocCopy(tagname, links[curdoc.link].lname);
		HTList_addObject(tagged, tagname);
		tagflag(TRUE, curdoc.link);
	    }
	}
	if (curdoc.link < nlinks - 1) {
	    set_curdoc_link(curdoc.link + 1);
	} else if (!more_text && LYGetNewline() == 1 && curdoc.link == nlinks
		   - 1) {
	    set_curdoc_link(0);
	} else if (more_text) {	/* next page */
	    LYChgNewline(display_lines);
	}
    }
}
#endif /* DIRED_SUPPORT */

static void handle_LYK_TOGGLE_HELP(void)
{
    if (user_mode == NOVICE_MODE) {
	toggle_novice_line();
	noviceline(more_text);
    }
}

static void handle_LYK_TOOLBAR(BOOLEAN *try_internal,
			       BOOLEAN *force_load,
			       int *old_c,
			       int real_c)
{
    char *cp;
    char *toolbar = NULL;

    if (!HText_hasToolbar(HTMainText)) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(NO_TOOLBAR);
	}
    } else if (*old_c != real_c) {
	*old_c = real_c;
	cp = trimPoundSelector(curdoc.address);
	HTSprintf0(&toolbar, "%s#%s", curdoc.address, LYToolbarName);
	restorePoundSelector(cp);
	set_address(&newdoc, toolbar);
	FREE(toolbar);
	*try_internal = TRUE;
	*force_load = TRUE;	/* force MainLoop to reload */
    }
}

static void handle_LYK_TRACE_LOG(BOOLEAN *trace_flag_ptr)
{
#ifndef NO_LYNX_TRACE
    /*
     * Check whether we've started a TRACE log in this session.  - FM
     */
    if (LYTraceLogFP == NULL) {
	HTUserMsg(NO_TRACELOG_STARTED);
	return;
    }

    /*
     * Don't do if already viewing the TRACE log.  - FM
     */
    if (LYIsUIPage(curdoc.address, UIP_TRACELOG))
	return;

    /*
     * If TRACE mode is on, turn it off during this fetch of the TRACE log, so
     * we don't enter stuff about this fetch, and set a flag for turning it
     * back on when we return to this loop.  Note that we'll miss any messages
     * about memory exhaustion if it should occur.  It seems unlikely that
     * anything else bad might happen, but if it does, we'll miss messages
     * about that too.  We also fflush(), close, and open it again, to make
     * sure all stderr messages thus far will be in the log.  - FM
     */
    if (!LYReopenTracelog(trace_flag_ptr))
	return;

    LYLocalFileToURL(&(newdoc.address), LYTraceLogPath);
    LYRegisterUIPage(newdoc.address, UIP_TRACELOG);
    StrAllocCopy(newdoc.title, LYNX_TRACELOG_TITLE);
    LYFreePostData(&newdoc);
    FREE(newdoc.bookmark);
    newdoc.isHEAD = FALSE;
    newdoc.safe = FALSE;
    newdoc.internal_link = FALSE;
    if (LYValidate || check_realm) {
	LYPermitURL = TRUE;
    }
    LYforce_no_cache = TRUE;
#else
    HTUserMsg(TRACE_DISABLED);
#endif /* NO_LYNX_TRACE */
}

#ifdef DIRED_SUPPORT
static void handle_LYK_UPLOAD(void)
{
    /*
     * Don't do if already viewing upload options page.
     */
    if (LYIsUIPage(curdoc.address, UIP_UPLOAD_OPTIONS))
	return;

    if (lynx_edit_mode && !no_dired_support) {
	LYUpload_options(&(newdoc.address), curdoc.address);
	StrAllocCopy(newdoc.title, UPLOAD_OPTIONS_TITLE);
	LYFreePostData(&newdoc);
	FREE(newdoc.bookmark);
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	newdoc.internal_link = FALSE;
	/*
	 * Uncache the current listing so that it will be updated to included
	 * the uploaded file if placed in the current directory.  - FM
	 */
	DIRED_UNCACHE_1;
    }
}
#endif /* DIRED_SUPPORT */

static void handle_LYK_UP_xxx(int *arrowup,
			      int *old_c,
			      int real_c,
			      int scroll_by)
{
    if (LYGetNewline() > 1) {
	if (LYGetNewline() - scroll_by < 1)
	    scroll_by = LYGetNewline() - 1;
	LYChgNewline(-scroll_by);
	if (nlinks > 0 && curdoc.link > -1) {
	    if (links[curdoc.link].ly + scroll_by <= display_lines) {
		newdoc.link = curdoc.link +
		    HText_LinksInLines(HTMainText,
				       LYGetNewline(),
				       scroll_by);
	    } else {
		*arrowup = TRUE;
	    }
	}
    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTInfoMsg(ALREADY_AT_BEGIN);
    }
}

static void handle_LYK_UP_HALF(int *arrowup,
			       int *old_c,
			       int real_c)
{
    handle_LYK_UP_xxx(arrowup, old_c, real_c, display_lines / 2);
}

static void handle_LYK_UP_LINK(int *follow_col,
			       int *arrowup,
			       int *old_c,
			       int real_c)
{
    if (curdoc.link > 0 &&
	(links[0].ly != links[curdoc.link].ly ||
	 !HText_LinksInLines(HTMainText, 1, LYGetNewline() - 1))) {
	/* more links before this on screen, and first of them on
	   a different line or no previous links before this screen? */
	int newlink;

	if (*follow_col == -1) {
	    const char *text = LYGetHiliteStr(curdoc.link, 0);

	    *follow_col = links[curdoc.link].lx;

	    if (text != NULL)
		*follow_col += (int) strlen(text) / 2;
	}

	newlink = find_link_near_col(*follow_col, -1);
	if (newlink > -1) {
	    set_curdoc_link(newlink);
	} else if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(NO_LINKS_ABOVE);
	}

    } else if (curdoc.line > 1 && LYGetNewline() > 1) {		/* previous page */
	int scrollamount = (LYGetNewline() > display_lines
			    ? display_lines
			    : LYGetNewline() - 1);

	LYChgNewline(-scrollamount);
	if (scrollamount < display_lines &&
	    nlinks > 0 && curdoc.link > -1 &&
	    links[0].ly - 1 + scrollamount <= display_lines) {
	    newdoc.link = HText_LinksInLines(HTMainText,
					     1,
					     scrollamount) - 1;
	} else {
	    *arrowup = TRUE;
	}

    } else if (*old_c != real_c) {
	*old_c = real_c;
	HTInfoMsg(ALREADY_AT_BEGIN);
    }
}

static void handle_LYK_UP_TWO(int *arrowup,
			      int *old_c,
			      int real_c)
{
    handle_LYK_UP_xxx(arrowup, old_c, real_c, 2);
}

static void handle_LYK_VIEW_BOOKMARK(BOOLEAN *refresh_screen,
				     int *old_c,
				     int real_c)
{
    const char *cp;

    if (LYValidate) {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    HTUserMsg(BOOKMARKS_DISABLED);
	}
	return;
    }

    /*
     * See if a bookmark exists.  If it does replace newdoc.address with its
     * name.
     */
    if ((cp = get_bookmark_filename(&newdoc.address)) != NULL) {
	if (*cp == '\0' || !strcmp(cp, " ") ||
	    !strcmp(curdoc.address, newdoc.address)) {
	    if (LYMultiBookmarks != MBM_OFF)
		*refresh_screen = TRUE;
	    return;
	}
#ifdef KANJI_CODE_OVERRIDE
	if (HTCJK == JAPANESE) {
	    last_kcode = NOKANJI;	/* AUTO */
	}
#endif
	LYforce_no_cache = TRUE;	/*force the document to be reloaded */
	StrAllocCopy(newdoc.title, BOOKMARK_TITLE);
	StrAllocCopy(newdoc.bookmark, BookmarkPage);
	LYFreePostData(&newdoc);
	newdoc.isHEAD = FALSE;
	newdoc.safe = FALSE;
	newdoc.internal_link = FALSE;
    } else {
	if (*old_c != real_c) {
	    *old_c = real_c;
	    LYMBM_statusline(BOOKMARKS_NOT_OPEN);
	    LYSleepAlert();
	    if (LYMultiBookmarks != MBM_OFF) {
		*refresh_screen = TRUE;
	    }
	}
    }
}

static BOOLEAN handle_LYK_VLINKS(int *cmd,
				 BOOLEAN *newdoc_link_is_absolute)
{
    int c;

    if (LYIsUIPage(curdoc.address, UIP_VLINKS)) {
	/*
	 * Already viewing visited links page, so get out.
	 */
	*cmd = LYK_PREV_DOC;
	return TRUE;
    }

    /*
     * Print visited links page to file.
     */
    c = LYShowVisitedLinks(&newdoc.address);
    if (c < 0) {
	HTUserMsg(VISITED_LINKS_EMPTY);
	return FALSE;
    }
    StrAllocCopy(newdoc.title, VISITED_LINKS_TITLE);
    LYFreePostData(&newdoc);
    FREE(newdoc.bookmark);
    newdoc.isHEAD = FALSE;
    newdoc.safe = FALSE;
    newdoc.internal_link = FALSE;
    if (c > 0) {
	/* Select a correct link. */
	*newdoc_link_is_absolute = TRUE;
	newdoc.link = c - 1;
    }
    if (LYValidate || check_realm) {
	LYPermitURL = TRUE;
	StrAllocCopy(lynxlinksfile, newdoc.address);
    }
    return FALSE;
}

void handle_LYK_WHEREIS(int cmd,
			BOOLEAN *refresh_screen)
{
    BOOLEAN have_target_onscreen = (BOOLEAN) (!isBEmpty(prev_target) &&
					      HText_pageHasPrevTarget());
    BOOL found;
    int oldcur = curdoc.link;	/* temporarily remember */
    char *remember_old_target = NULL;

    if (have_target_onscreen)
	StrAllocCopy(remember_old_target, prev_target->str);
    else
	StrAllocCopy(remember_old_target, "");

    if (cmd == LYK_WHEREIS) {
	/*
	 * Reset prev_target to force prompting for a new search string and to
	 * turn off highlighting if no search string is entered by the user.
	 */
	BStrCopy0(prev_target, "");
    }
    found = textsearch(&curdoc, &prev_target,
		       (cmd == LYK_WHEREIS)
		       ? 0
		       : ((cmd == LYK_NEXT)
			  ? 1
			  : -1));

    /*
     * Force a redraw to ensure highlighting of hits even when found on the
     * same page, or clearing of highlighting if the default search string was
     * erased without replacement.  - FM
     */
    /*
     * Well let's try to avoid it at least in a few cases
     * where it is not needed. - kw
     */
    if (www_search_result >= 0 && www_search_result != curdoc.line) {
	*refresh_screen = TRUE;	/* doesn't really matter */
    } else if (!found) {
	*refresh_screen = have_target_onscreen;
    } else if (!have_target_onscreen && found) {
	*refresh_screen = TRUE;
    } else if (www_search_result == curdoc.line &&
	       curdoc.link == oldcur &&
	       curdoc.link >= 0 && nlinks > 0 &&
	       links[curdoc.link].ly >= (display_lines / 3)) {
	*refresh_screen = TRUE;
    } else if ((LYcase_sensitive && 0 != strcmp(prev_target->str,
						remember_old_target)) ||
	       (!LYcase_sensitive && 0 != strcasecomp8(prev_target->str,
						       remember_old_target))) {
	*refresh_screen = TRUE;
    }
    FREE(remember_old_target);
}

/*
 * Get a number from the user and follow that link number.
 */
static void handle_LYK_digit(int c,
			     BOOLEAN *force_load,
			     int *old_c,
			     int real_c,
			     BOOLEAN *try_internal GCC_UNUSED)
{
    int lindx = ((nlinks > 0) ? curdoc.link : 0);
    int number;
    char *temp = NULL;

    /* pass cur line num for use in follow_link_number()
     * Note: Current line may not equal links[cur].line
     */
    number = curdoc.line;
    switch (follow_link_number(c, lindx, &newdoc, &number)) {
    case DO_LINK_STUFF:
	/*
	 * Follow a normal link.
	 */
	set_address(&newdoc, links[lindx].lname);
	StrAllocCopy(newdoc.title, LYGetHiliteStr(lindx, 0));
	/*
	 * For internal links, retain POST content if present.  If we are on
	 * the List Page, prevent pushing it on the history stack.  Otherwise
	 * set try_internal to signal that the top of the loop should attempt
	 * to reposition directly, without calling getfile.  - kw
	 */
	if (track_internal_links) {
	    if (links[lindx].type == WWW_INTERN_LINK_TYPE) {
		LYinternal_flag = TRUE;
		newdoc.internal_link = TRUE;
		if (LYIsListpageTitle(NonNull(curdoc.title)) &&
		    (LYIsUIPage(curdoc.address, UIP_LIST_PAGE) ||
		     LYIsUIPage(curdoc.address, UIP_ADDRLIST_PAGE))) {
		    if (check_history()) {
			LYinternal_flag = TRUE;
		    } else {
			HTLastConfirmCancelled();	/* reset flag */
			if (!confirm_post_resub(newdoc.address,
						newdoc.title,
						((LYresubmit_posts &&
						  HText_POSTReplyLoaded(&newdoc))
						 ? 1
						 : 2),
						2)) {
			    if (HTLastConfirmCancelled() ||
				(LYresubmit_posts &&
				 !HText_POSTReplyLoaded(&newdoc))) {
				/* cancel the whole thing */
				LYforce_no_cache = FALSE;
				reloading = FALSE;
				copy_address(&newdoc, &curdoc);
				StrAllocCopy(newdoc.title, curdoc.title);
				newdoc.internal_link = curdoc.internal_link;
				HTInfoMsg(CANCELLED);
				if (nlinks > 0)
				    HText_pageDisplay(curdoc.line, prev_target->str);
				break;
			    } else if (LYresubmit_posts) {
				/* If LYresubmit_posts is set, and the
				   answer was No, and we have a cached
				   copy, then use it. - kw */
				LYforce_no_cache = FALSE;
			    } else {
				/* if No, but not ^C or ^G, drop
				 * the post data.  Maybe the link
				 * wasn't meant to be internal after
				 * all, here we can recover from that
				 * assumption. - kw */
				LYFreePostData(&newdoc);
				newdoc.internal_link = FALSE;
				HTAlert(DISCARDING_POST_DATA);
			    }
			}
		    }
		    /*
		     * Don't push the List Page if we follow an internal link given
		     * by it.  - kw
		     */
		    free_address(&curdoc);
		} else
		    *try_internal = TRUE;
		if (!(LYresubmit_posts && newdoc.post_data))
		    LYinternal_flag = TRUE;
		*force_load = TRUE;
		break;
	    } else {
		/*
		 * Free POST content if not an internal link.  - kw
		 */
		LYFreePostData(&newdoc);
	    }
	}
	/*
	 * Might be an anchor in the same doc from a POST form.  If so, don't
	 * free the content.  -- FM
	 */
	if (are_different(&curdoc, &newdoc)) {
	    LYFreePostData(&newdoc);
	    FREE(newdoc.bookmark);
	    newdoc.isHEAD = FALSE;
	    newdoc.safe = FALSE;
	    if (isLYNXMESSAGES(newdoc.address))
		LYforce_no_cache = TRUE;
	}
	newdoc.internal_link = FALSE;
	*force_load = TRUE;	/* force MainLoop to reload */
	break;

    case DO_GOTOLINK_STUFF:
	/*
	 * Position on a normal link, don't follow it.  - KW
	 */
	LYSetNewline(newdoc.line);
	newdoc.line = 1;
	if (LYGetNewline() == curdoc.line) {
	    /*
	     * It's a link in the current page.  - FM
	     */
	    if (nlinks > 0 && curdoc.link > -1) {
		if (curdoc.link == newdoc.link) {
		    /*
		     * It's the current link, and presumably reflects a typo in
		     * the statusline entry, so issue a statusline message for
		     * the typo-prone users (like me 8-).  - FM
		     */
		    HTSprintf0(&temp, LINK_ALREADY_CURRENT, number);
		    HTUserMsg(temp);
		    FREE(temp);
		} else {
		    /*
		     * It's a different link on this page,
		     */
		    set_curdoc_link(newdoc.link);
		    newdoc.link = 0;
		}
	    }
	}
	break;			/* nothing more to do */

    case DO_GOTOPAGE_STUFF:
	/*
	 * Position on a page in this document.  - FM
	 */
	LYSetNewline(newdoc.line);
	newdoc.line = 1;
	if (LYGetNewline() == curdoc.line) {
	    /*
	     * It's the current page, so issue a statusline message for the
	     * typo-prone users (like me 8-).  - FM
	     */
	    if (LYGetNewline() <= 1) {
		HTInfoMsg(ALREADY_AT_BEGIN);
	    } else if (!more_text) {
		HTInfoMsg(ALREADY_AT_END);
	    } else {
		HTSprintf0(&temp, ALREADY_AT_PAGE, number);
		HTUserMsg(temp);
		FREE(temp);
	    }
	}
	break;

    case PRINT_ERROR:
	*old_c = real_c;
	HTUserMsg(BAD_LINK_NUM_ENTERED);
	break;
    }
    return;
}

#ifdef SUPPORT_CHDIR

/* original implementation by VH */
void handle_LYK_CHDIR(void)
{
    static bstring *buf = NULL;
    char *p = NULL;

    if (no_chdir) {
	HTUserMsg(CHDIR_DISABLED);
	return;
    }

    _statusline(gettext("cd to:"));
    if (LYgetBString(&buf, FALSE, 0, NORECALL) < 0 || isBEmpty(buf)) {
	HTInfoMsg(CANCELLED);
	return;
    }

    if (LYIsTilde(buf->str[0]) &&
	(LYIsPathSep(buf->str[1]) || buf->str[1] == '\0')) {
	HTSprintf0(&p, "%s%s", Home_Dir(), buf->str + 1);
    } else {
	StrAllocCopy(p, buf->str);
    }

    CTRACE((tfp, "changing directory to '%s'\n", p));
    if (chdir(p)) {
	switch (errno) {
	case EACCES:
	    HTInfoMsg(COULD_NOT_ACCESS_DIR);
	    break;
	case ENOENT:
	    HTInfoMsg(gettext("No such directory"));
	    break;
	case ENOTDIR:
	    HTInfoMsg(gettext("A component of path is not a directory"));
	    break;
	default:
	    HTInfoMsg(gettext("failed to change directory"));
	    break;
	}
    } else {
#ifdef DIRED_SUPPORT
	/*if in dired, load content of other directory */
	if (!no_dired_support
	    && (lynx_edit_mode || (LYIsUIPage(curdoc.address, UIP_DIRED_MENU)))) {
	    char buf2[LY_MAXPATH];
	    char *addr = NULL;

	    Current_Dir(buf2);
	    LYLocalFileToURL(&addr, buf2);

	    newdoc.address = addr;
	    newdoc.isHEAD = FALSE;
	    StrAllocCopy(newdoc.title, gettext("A URL specified by the user"));
	    LYFreePostData(&newdoc);
	    FREE(newdoc.bookmark);
	    newdoc.safe = FALSE;
	    newdoc.internal_link = FALSE;
	    /**force_load = TRUE;*/
	    if (lynx_edit_mode) {
		DIRED_UNCACHE_2;
	    }
	} else
#endif
	    HTInfoMsg(OPERATION_DONE);
    }
    FREE(p);
}

static void handle_LYK_PWD(void)
{
    char buffer[LY_MAXPATH];
    int save_secs = InfoSecs;
    BOOLEAN save_wait = no_pause;

    if (Secs2SECS(save_secs) < 1)
	InfoSecs = SECS2Secs(1);
    no_pause = FALSE;

    HTInfoMsg(Current_Dir(buffer));

    InfoSecs = save_secs;
    no_pause = save_wait;
}
#endif

#ifdef USE_CURSES_PADS
/*
 * Having jumps larger than this is counter-productive.  Indeed, it is natural
 * to expect that when the relevant text appears, one would "overshoot" and
 * would scroll 3-4 extra full screens.  When going back, the "accumulation"
 * logic would again start moving in full screens, so one would overshoot
 * again, etc.
 *
 * Going back, one can fix it in 28 keypresses. The relevant text will appear
 * on the screen soon enough for the key-repeat to become not that important,
 * and we are still moving in smaller steps than when we overshot.  Since key
 * repeat is not important, even if we overshoot again, it is going to be by 30
 * steps, which is easy to fix by reversing the direction again.
 */
static int repeat_to_delta(int n)
{
    int threshold = LYcols / 3;

    while (threshold > 0) {
	if (n >= threshold) {
	    n = threshold;
	    break;
	}
	threshold = (threshold * 2) / 3;
    }
    return n;
}

static void handle_LYK_SHIFT_LEFT(BOOLEAN *flag, int count)
{
    if (!LYwideLines) {
	HTAlert(SHIFT_VS_LINEWRAP);
	return;
    }
    if (LYshiftWin > 0) {
	LYshiftWin -= repeat_to_delta(count);
	*flag = TRUE;
    }
    if (LYshiftWin < 0)
	LYshiftWin = 0;
}

static void handle_LYK_SHIFT_RIGHT(BOOLEAN *flag, int count)
{
    if (!LYwideLines) {
	HTAlert(SHIFT_VS_LINEWRAP);
	return;
    }
    LYshiftWin += repeat_to_delta(count);
    *flag = TRUE;
}

static BOOLEAN handle_LYK_LINEWRAP_TOGGLE(int *cmd,
					  BOOLEAN *flag)
{
    static const char *choices[] =
    {
	"Try to fit screen width",
	"No line wrap in columns",
	"Wrap columns at screen width",
	"Wrap columns at 3/4 screen width",
	"Wrap columns at 2/3 screen width",
	"Wrap columns at 1/2 screen width",
	"Wrap columns at 1/3 screen width",
	"Wrap columns at 1/4 screen width",
	NULL
    };
    static int wrap[] =
    {
	0,
	0,
	12,			/* In units of 1/12 */
	9,
	8,
	6,
	4,
	3
    };
    int c;
    int code = FALSE;

    CTRACE((tfp, "Entering handle_LYK_LINEWRAP_TOGGLE\n"));
    if (LYwin != stdscr) {
	/* Somehow the mouse is over the number instead of being over the
	   name, so we decrease x. */
	c = LYChoosePopup(!LYwideLines,
			  LYlines / 2 - 2,
			  LYcolLimit / 2 - 6,
			  choices, (int) TABLESIZE(choices) - 1,
			  FALSE, TRUE);
	/*
	 * LYhandlePopupList() wasn't really meant to be used outside of
	 * old-style Options menu processing.  One result of mis-using it here
	 * is that we have to deal with side-effects regarding SIGINT signal
	 * handler and the term_options global variable.  - kw
	 */
	if (!term_options) {
	    CTRACE((tfp,
		    "...setting LYwideLines %d, LYtableCols %d (have %d and %d)\n",
		    c, wrap[c],
		    LYwideLines,
		    LYtableCols));

	    LYwideLines = c;
	    LYtableCols = wrap[c];

	    if (LYwideLines == 0)
		LYshiftWin = 0;
	    *flag = TRUE;
	    HTUserMsg(LYwideLines ? LINEWRAP_OFF : LINEWRAP_ON);
	    code = reparse_or_reload(cmd);
	}
    }
    return (BOOLEAN) code;
}
#endif

#ifdef USE_MAXSCREEN_TOGGLE
static BOOLEAN handle_LYK_MAXSCREEN_TOGGLE(int *cmd)
{
    static int flag = 0;

    CTRACE((tfp, "Entering handle_LYK_MAXSCREEN_TOGGLE\n"));
    if (flag) {
	CTRACE((tfp, "Calling recoverWindowSize()\n"));
	recoverWindowSize();
	flag = 0;
    } else {
	CTRACE((tfp, "Calling maxmizeWindowSize()\n"));
	maxmizeWindowSize();
	flag = 1;
    }
    return reparse_or_reload(cmd);
}
#endif

#ifdef LY_FIND_LEAKS
#define CleanupMainLoop() \
 	BStrFree(prev_target); \
 	BStrFree(user_input_buffer)
#else
#define CleanupMainLoop()	/* nothing */
#endif

/*
 * Here's where we do all the work.
 * mainloop is basically just a big switch dependent on the users input.  I
 * have tried to offload most of the work done here to procedures to make it
 * more modular, but this procedure still does a lot of variable manipulation.
 * This needs some work to make it neater.  - Lou Moutilli
 *					(memoir from the original Lynx - FM)
 */
int mainloop(void)
{
#if defined(WIN_EX)		/* 1997/10/08 (Wed) 14:52:06 */
    char sjis_buff[MAX_LINE];
    char temp_buff[sizeof(sjis_buff) * 4];
#endif
    int c = 0;
    int real_c = 0;
    int old_c = 0;
    int pending_form_c = -1;
    int cmd = LYK_DO_NOTHING, real_cmd = LYK_DO_NOTHING;
    int getresult;
    int arrowup = FALSE, show_help = FALSE;
    bstring *user_input_buffer = NULL;
    const char *cshelpfile = NULL;
    BOOLEAN first_file = TRUE;
    BOOLEAN popped_doc = FALSE;
    BOOLEAN refresh_screen = FALSE;
    BOOLEAN force_load = FALSE;
    BOOLEAN try_internal = FALSE;
    BOOLEAN crawl_ok = FALSE;
    BOOLEAN vi_keys_flag = vi_keys;
    BOOLEAN emacs_keys_flag = emacs_keys;
    BOOLEAN trace_mode_flag = FALSE;
    BOOLEAN forced_HTML_mode = LYforce_HTML_mode;
    char cfile[128];
    FILE *cfp;
    char *cp;
    int ch = 0;
    RecallType recall = NORECALL;
    int URLTotal = 0;
    int URLNum;
    BOOLEAN FirstURLRecall = TRUE;
    char *temp = NULL;
    BOOLEAN ForcePush = FALSE;
    BOOLEAN override_LYresubmit_posts = FALSE;
    BOOLEAN newdoc_link_is_absolute = FALSE;
    BOOLEAN curlink_is_editable;
    BOOLEAN use_last_tfpos;
    unsigned int len;
    int i;
    int follow_col = -1, key_count = 0, last_key = 0;
    int tmpNewline;
    DocInfo tmpDocInfo;

    /* "internal" means "within the same document, with certainty".  It includes a
     * space so it cannot conflict with any (valid) "TYPE" attributes on A
     * elements.  [According to which DTD, anyway??] - kw
     */
    HTInternalLink = HTAtom_for("internal link");	/* init, used as const */

#ifndef WWW_SOURCE
    WWW_SOURCE = HTAtom_for("www/source");	/* init, used as const */
#endif

    /*
     * curdoc.address contains the name of the file that is currently open.
     * newdoc.address contains the name of the file that will soon be
     *                opened if it exits.
     * prev_target    contains the last search string the user searched for.
     * newdoc.title   contains the link name that the user last chose to get
     *                into the current link (file).
     */
    /* initialize some variables */
    newdoc.address = NULL;
    newdoc.title = NULL;
    newdoc.post_data = NULL;
    newdoc.post_content_type = NULL;
    newdoc.bookmark = NULL;
    newdoc.internal_link = FALSE;
    curdoc.address = NULL;
    curdoc.title = NULL;
    curdoc.post_data = NULL;
    curdoc.post_content_type = NULL;
    curdoc.bookmark = NULL;
    curdoc.internal_link = FALSE;
#ifdef USE_COLOR_STYLE
    curdoc.style = NULL;
    newdoc.style = NULL;
#endif
#ifndef USE_SESSIONS
    nhist = 0;
#endif
    BStrCopy0(user_input_buffer, "");
    BStrCopy0(prev_target, "");
#ifdef LY_FIND_LEAKS
    atexit(free_mainloop_variables);
#endif
  initialize:
    set_address(&newdoc, startfile);
    StrAllocCopy(startrealm, startfile);
    StrAllocCopy(newdoc.title, gettext("Entry into main screen"));
    newdoc.isHEAD = FALSE;
    newdoc.safe = FALSE;
    newdoc.line = 1;
    newdoc.link = 0;

#ifdef USE_SLANG
    if (TRACE && LYCursesON) {
	LYaddstr("\n");
	LYrefresh();
    }
#endif /* USE_SLANG */
    CTRACE((tfp, "Entering mainloop, startfile=%s\n", startfile));

    if (form_post_data) {
	BStrCopy0(newdoc.post_data, form_post_data);
	StrAllocCopy(newdoc.post_content_type,
		     "application/x-www-form-urlencoded");
    } else if (form_get_data) {
	StrAllocCat(newdoc.address, form_get_data);
    }

    if (bookmark_start) {
	if (LYValidate) {
	    HTAlert(BOOKMARKS_DISABLED);
	    bookmark_start = FALSE;
	    goto initialize;
	} else if (traversal) {
	    HTAlert(BOOKMARKS_NOT_TRAVERSED);
	    traversal = FALSE;
	    crawl = FALSE;
	    bookmark_start = FALSE;
	    goto initialize;
	} else {
	    const char *cp1;

	    /*
	     * See if a bookmark page exists.  If it does, replace
	     * newdoc.address with its name
	     */
	    if ((cp1 = get_bookmark_filename(&newdoc.address)) != NULL &&
		*cp1 != '\0' && strcmp(cp1, " ")) {
		StrAllocCopy(newdoc.title, BOOKMARK_TITLE);
		StrAllocCopy(newdoc.bookmark, BookmarkPage);
		StrAllocCopy(startrealm, newdoc.address);
		LYFreePostData(&newdoc);
		newdoc.isHEAD = FALSE;
		newdoc.safe = FALSE;
		CTRACE((tfp, "Using bookmarks=%s\n", newdoc.address));
	    } else {
		HTUserMsg(BOOKMARKS_NOT_OPEN);
		bookmark_start = FALSE;
		goto initialize;
	    }
	}
    }

    FREE(form_post_data);
    FREE(form_get_data);

    LYSetDisplayLines();

    while (TRUE) {
#ifdef USE_COLOR_STYLE
	if (curdoc.style != NULL)
	    force_load = TRUE;
#endif
	/*
	 * If newdoc.address is different from curdoc.address then we need to
	 * go out and find and load newdoc.address.
	 */
	if (LYforce_no_cache || force_load ||
	    are_different(&curdoc, &newdoc)) {

	    force_load = FALSE;	/* done */
	    if (TRACE && LYCursesON) {
		LYHideCursor();	/* make sure cursor is down */
#ifdef USE_SLANG
		LYaddstr("\n");
#endif /* USE_SLANG */
		LYrefresh();
	    }
	  try_again:
	    /*
	     * Push the old file onto the history stack if we have a current
	     * doc and a new address.  - FM
	     */
	    if (curdoc.address && newdoc.address) {
		/*
		 * Don't actually push if this is a LYNXDOWNLOAD URL, because
		 * that returns NORMAL even if it fails due to a spoof attempt
		 * or file access problem, and we set the newdoc structure
		 * elements to the curdoc structure elements under case NORMAL.
		 * - FM
		 */
		if (!isLYNXDOWNLOAD(newdoc.address)) {
		    LYpush(&curdoc, ForcePush);
		}
	    } else if (!newdoc.address) {
		/*
		 * If newdoc.address is empty then pop a file and load it.  -
		 * FM
		 */
		LYhist_prev(&newdoc);
		popped_doc = TRUE;

		/*
		 * If curdoc had been reached via an internal
		 * (fragment) link from what we now have just
		 * popped into newdoc, then override non-caching in
		 * all cases. - kw
		 */
		if (track_internal_links &&
		    curdoc.internal_link &&
		    !are_phys_different(&curdoc, &newdoc)) {
		    LYinternal_flag = TRUE;
		    LYoverride_no_cache = TRUE;
		    LYforce_no_cache = FALSE;
		    try_internal = TRUE;
		} else {
		    /*
		     * Force a no_cache override unless it's a bookmark file,
		     * or it has POST content and LYresubmit_posts is set
		     * without safe also set, and we are not going to another
		     * position in the current document or restoring the
		     * previous document due to a NOT_FOUND or NULLFILE return
		     * value from getfile().  - FM
		     */
		    if ((newdoc.bookmark != NULL) ||
			(newdoc.post_data != NULL &&
			 !newdoc.safe &&
			 LYresubmit_posts &&
			 !override_LYresubmit_posts &&
			 NO_INTERNAL_OR_DIFFERENT(&curdoc, &newdoc))) {
			LYoverride_no_cache = FALSE;
		    } else {
			LYoverride_no_cache = TRUE;
		    }
		}
	    }
	    override_LYresubmit_posts = FALSE;

	    if (HEAD_request) {
		/*
		 * Make SURE this is an appropriate request.  - FM
		 */
		if (newdoc.address) {
		    if (LYCanDoHEAD(newdoc.address) == TRUE) {
			newdoc.isHEAD = TRUE;
		    } else if (isLYNXIMGMAP(newdoc.address)) {
			if (LYCanDoHEAD(newdoc.address + LEN_LYNXIMGMAP) == TRUE) {
			    StrAllocCopy(temp, newdoc.address + LEN_LYNXIMGMAP);
			    free_address(&newdoc);
			    newdoc.address = temp;
			    newdoc.isHEAD = TRUE;
			    temp = NULL;
			}
		    }
		}
		try_internal = FALSE;
		HEAD_request = FALSE;
	    }

	    /*
	     * If we're getting the TRACE log and it's not new, check whether
	     * its HText structure has been dumped, and if so, fflush() and
	     * fclose() it to ensure it's fully updated, and then fopen() it
	     * again.  - FM
	     */
	    if (LYUseTraceLog == TRUE &&
		trace_mode_flag == FALSE &&
		LYTraceLogFP != NULL &&
		LYIsUIPage(newdoc.address, UIP_TRACELOG)) {
		DocAddress WWWDoc;
		HTParentAnchor *tmpanchor;

		WWWDoc.address = newdoc.address;
		WWWDoc.post_data = newdoc.post_data;
		WWWDoc.post_content_type = newdoc.post_content_type;
		WWWDoc.bookmark = newdoc.bookmark;
		WWWDoc.isHEAD = newdoc.isHEAD;
		WWWDoc.safe = newdoc.safe;
		tmpanchor = HTAnchor_findAddress(&WWWDoc);
		if ((HText *) HTAnchor_document(tmpanchor) == NULL) {
		    if (!LYReopenTracelog(&trace_mode_flag)) {
			old_c = 0;
			cmd = LYK_PREV_DOC;
			goto new_cmd;
		    }
		}
	    }

	    LYRequestTitle = newdoc.title;
	    if (newdoc.bookmark)
		LYforce_HTML_mode = TRUE;
	    if (LYValidate &&
		startfile_ok &&
		newdoc.address && startfile && homepage &&
		(!strcmp(newdoc.address, startfile) ||
		 !strcmp(newdoc.address, homepage))) {
		LYPermitURL = TRUE;
	    }

	    /* reset these two variables here before getfile()
	     * so they will be available in partial mode
	     * (was previously implemented in case NORMAL).
	     */
	    BStrCopy0(prev_target, "");		/* Reset for new coming document */
	    LYSetNewline(newdoc.line);	/* set for LYGetNewline() */

#ifdef USE_PRETTYSRC
	    psrc_first_tag = TRUE;
#endif
#ifdef TEXTFIELDS_MAY_NEED_ACTIVATION
	    textfields_need_activation = textfields_activation_option;
#endif
	    FREE(LYRequestReferer);
	    /*
	     * Don't send Referer if we have to load a document again that we
	     * got from the history stack.  We don't know any more how we
	     * originally got to that page.  Using a Referer based on the
	     * current HTMainText could only be right by coincidence.  - kw
	     * 1999-11-01
	     */
	    if (popped_doc)
		LYNoRefererForThis = TRUE;

	    if (track_internal_links) {
		if (try_internal) {
		    if (newdoc.address &&
			isLYNXIMGMAP(newdoc.address)) {
			try_internal = FALSE;
		    } else if (curdoc.address &&
			       isLYNXIMGMAP(curdoc.address)) {
			try_internal = FALSE;
		    }
		}
		if (try_internal) {
		    char *hashp = findPoundSelector(newdoc.address);

		    if (hashp) {
			HTFindPoundSelector(hashp + 1);
		    }
		    getresult = (HTMainText != NULL) ? NORMAL : NOT_FOUND;
		    try_internal = FALSE;	/* done */
		    /* fix up newdoc.address which may have been fragment-only */
		    if (getresult == NORMAL && (!hashp || hashp == newdoc.address)) {
			if (!hashp) {
			    set_address(&newdoc, HTLoadedDocumentURL());
			} else {
			    StrAllocCopy(temp, HTLoadedDocumentURL());
			    StrAllocCat(temp, hashp);	/* append fragment */
			    set_address(&newdoc, temp);
			    FREE(temp);
			}
		    }
		} else {
		    if (newdoc.internal_link && newdoc.address &&
			*newdoc.address == '#' && nhist > 0) {
			char *cp0;

			if (isLYNXIMGMAP(HDOC(nhist_1).address))
			    cp0 = HDOC(nhist_1).address + LEN_LYNXIMGMAP;
			else
			    cp0 = HDOC(nhist_1).address;
			StrAllocCopy(temp, cp0);
			(void) trimPoundSelector(temp);
			StrAllocCat(temp, newdoc.address);
			free_address(&newdoc);
			newdoc.address = temp;
			temp = NULL;
		    }
		    tmpDocInfo = newdoc;
		    tmpNewline = -1;
		    fill_JUMP_Params(&newdoc.address);
		    getresult = getfile(&newdoc, &tmpNewline);
		    if (!reloading && !popped_doc && (tmpNewline >= 0)) {
			LYSetNewline(tmpNewline);
		    } else {
			newdoc.link = tmpDocInfo.link;
		    }
		}
	    } else {
		tmpDocInfo = newdoc;
		tmpNewline = -1;
		fill_JUMP_Params(&newdoc.address);
		getresult = getfile(&newdoc, &tmpNewline);
		if (!reloading && !popped_doc && (tmpNewline >= 0)) {
		    LYSetNewline(tmpNewline);
		} else {
		    newdoc.link = tmpDocInfo.link;
		}
	    }

#ifdef INACTIVE_INPUT_STYLE_VH
	    textinput_redrawn = FALSE;	/* for sure */
#endif

	    switch (getresult) {

	    case NOT_FOUND:
		/*
		 * OK!  can't find the file, so it must not be around now.  Do
		 * any error logging, if appropriate.
		 */
		LYoverride_no_cache = FALSE;	/* Was TRUE if popped. - FM */
		LYinternal_flag = FALSE;	/* Reset to default. - kw */
		turn_trace_back_on(&trace_mode_flag);
		if (!first_file && !LYCancelledFetch) {
		    /*
		     * Do error mail sending and/or traversal stuff.  Note that
		     * the links[] elements may not be valid at this point, if
		     * we did call HTuncache_current_document!  This should not
		     * have happened for traversal, but for sending error mail
		     * check that HTMainText exists for this reason.  - kw
		     */
		    if (error_logging && nhist > 0 && !popped_doc &&
			!LYUserSpecifiedURL &&
			HTMainText &&
			nlinks > 0 && curdoc.link < nlinks &&
			!isLYNXHIST(NonNull(newdoc.address)) &&
			!isLYNXCACHE(NonNull(newdoc.address)) &&
			!isLYNXCOOKIE(NonNull(newdoc.address))) {
			char *mail_owner = NULL;

			if (owner_address && isMAILTO_URL(owner_address)) {
			    mail_owner = owner_address + LEN_MAILTO_URL;
			}
			/*
			 * Email a bad link message to the owner of the
			 * document, or to ALERTMAIL if defined, but NOT to
			 * lynx-dev (it is rejected in mailmsg).  - FM, kw
			 */
#ifndef ALERTMAIL
			if (mail_owner)
#endif
			    mailmsg(curdoc.link,
				    mail_owner,
				    HDOC(nhist_1).address,
				    HDOC(nhist_1).title);
		    }
		    if (traversal) {
			FILE *ofp;

			if ((ofp = LYAppendToTxtFile(TRAVERSE_ERRORS)) == NULL) {
			    if ((ofp = LYNewTxtFile(TRAVERSE_ERRORS)) == NULL) {
				perror(NOOPEN_TRAV_ERR_FILE);
				exit_immediately(EXIT_FAILURE);
			    }
			}
			if (nhist > 0) {
			    fprintf(ofp,
				    "%s %s\tin %s\n",
				    popped_doc ?
				    newdoc.address : links[curdoc.link].lname,
				    links[curdoc.link].target,
				    HDOC(nhist_1).address);
			} else {
			    fprintf(ofp,
				    "%s %s\t\n",
				    popped_doc ?
				    newdoc.address : links[curdoc.link].lname,
				    links[curdoc.link].target);
			}
			LYCloseOutput(ofp);
		    }
		}

		/*
		 * Fall through to do the NULL stuff and reload the old file,
		 * unless the first file wasn't found or has gone missing.
		 */
		if (!nhist) {
		    /*
		     * If nhist = 0 then it must be the first file.
		     */
		    CleanupMainLoop();
		    exit_immediately_with_error_message(NOT_FOUND, first_file);
		    return (EXIT_FAILURE);
		}
		/* FALLTHRU */

	    case NULLFILE:
		/*
		 * Not supposed to return any file.
		 */
		LYoverride_no_cache = FALSE;	/* Was TRUE if popped. - FM */
		popped_doc = FALSE;	/* Was TRUE if popped. - FM */
		LYinternal_flag = FALSE;	/* Reset to default. - kw */
		turn_trace_back_on(&trace_mode_flag);
		free_address(&newdoc);	/* to pop last doc */
		FREE(newdoc.bookmark);
		LYJumpFileURL = FALSE;
		reloading = FALSE;
		LYPermitURL = FALSE;
		LYCancelledFetch = FALSE;
		ForcePush = FALSE;
		LYforce_HTML_mode = FALSE;
		force_old_UCLYhndl_on_reload = FALSE;
		if (traversal) {
		    crawl_ok = FALSE;
		    if (traversal_link_to_add) {
			/*
			 * It's a binary file, or the fetch attempt failed.
			 * Add it to TRAVERSE_REJECT_FILE so we don't try again
			 * in this run.
			 */
			if (!lookup_reject(traversal_link_to_add)) {
			    add_to_reject_list(traversal_link_to_add);
			}
			FREE(traversal_link_to_add);
		    }
		}
		/*
		 * Make sure the first file was found and has not gone missing.
		 */
		if (!nhist) {
		    /*
		     * If nhist = 0 then it must be the first file.
		     */
		    if (first_file && homepage &&
			!LYSameFilename(homepage, startfile)) {
			/*
			 * Couldn't return to the first file but there is a
			 * homepage we can use instead.  Useful for when the
			 * first URL causes a program to be invoked.  - GL
			 *
			 * But first make sure homepage is different from
			 * startfile (above), then make it the same (below) so
			 * we don't enter an infinite getfile() loop on on
			 * failures to find the files.  - FM
			 */
			set_address(&newdoc, homepage);
			LYFreePostData(&newdoc);
			FREE(newdoc.bookmark);
			StrAllocCopy(startfile, homepage);
			newdoc.isHEAD = FALSE;
			newdoc.safe = FALSE;
			newdoc.internal_link = FALSE;
			goto try_again;
		    } else {
			CleanupMainLoop();
			exit_immediately_with_error_message(NULLFILE, first_file);
			return (EXIT_FAILURE);
		    }
		}

		/*
		 * If we're going to pop from history because getfile didn't
		 * succeed, reset LYforce_no_cache first.  This would have been
		 * done in HTAccess.c if the request got that far, but the URL
		 * may have been handled or rejected in getfile without taking
		 * care of that.  - kw
		 */
		LYforce_no_cache = FALSE;
		/*
		 * Retrieval of a newdoc just failed, and just going to
		 * try_again would pop the next doc from history and try to get
		 * it without further questions.  This may not be the right
		 * thing to do if we have POST data, so fake a PREV_DOC key if
		 * it seems that some prompting should be done.  This doesn't
		 * affect the traversal logic, since with traversal POST data
		 * can never occur.  - kw
		 */
		if (HDOC(nhist - 1).post_data &&
		    !HDOC(nhist - 1).safe) {
		    if (HText_POSTReplyLoaded((DocInfo *) &history[(nhist_1)])) {
			override_LYresubmit_posts = TRUE;
			goto try_again;
		    }
		    /* Set newdoc fields, just in case the PREV_DOC gets
		     * cancelled.  - kw
		     */
		    if (!curdoc.address) {
			set_address(&newdoc, HTLoadedDocumentURL());
			StrAllocCopy(newdoc.title, HTLoadedDocumentTitle());
			if (HTMainAnchor
			    && HTMainAnchor->post_data) {
			    BStrCopy(newdoc.post_data,
				     HTMainAnchor->post_data);
			    StrAllocCopy(newdoc.post_content_type,
					 HTMainAnchor->post_content_type);
			} else {
			    BStrFree(newdoc.post_data);
			}
			newdoc.isHEAD = HTLoadedDocumentIsHEAD();
			newdoc.safe = HTLoadedDocumentIsSafe();
			newdoc.internal_link = FALSE;
		    } else {
			copy_address(&newdoc, &curdoc);
			StrAllocCopy(newdoc.title, curdoc.title);
			BStrCopy(newdoc.post_data, curdoc.post_data);
			StrAllocCopy(newdoc.post_content_type,
				     curdoc.post_content_type);
			newdoc.isHEAD = curdoc.isHEAD;
			newdoc.safe = curdoc.safe;
			newdoc.internal_link = curdoc.internal_link;
			newdoc.line = curdoc.line;
			newdoc.link = curdoc.link;
		    }
		    cmd = LYK_PREV_DOC;
		    goto new_cmd;
		}
		override_LYresubmit_posts = TRUE;
		goto try_again;

	    case NORMAL:
		/*
		 * Marvelously, we got the document!
		 */
		LYoverride_no_cache = FALSE;	/* Was TRUE if popped. - FM */
		LYinternal_flag = FALSE;	/* Reset to default. - kw */
		turn_trace_back_on(&trace_mode_flag);

		/*
		 * If it's the first file and we're interactive, check whether
		 * it's a bookmark file which was not accessed via the -book
		 * switch.  - FM
		 */
		if (((first_file == TRUE) &&
		     (dump_output_immediately == FALSE) &&
		     isEmpty(newdoc.bookmark)) &&
		    ((LYisLocalFile(newdoc.address) == TRUE) &&
		     !(strcmp(NonNull(HText_getTitle()),
			      BOOKMARK_TITLE))) &&
		    (temp = HTParse(newdoc.address, "",
				    PARSE_PATH + PARSE_PUNCTUATION)) != NULL) {
		    const char *name = wwwName(Home_Dir());

		    len = (unsigned) strlen(name);
#ifdef VMS
		    if (!strncasecomp(temp, name, len) &&
			strlen(temp) > len)
#else
		    if (!StrNCmp(temp, name, len) &&
			strlen(temp) > len)
#endif /* VMS */
		    {
			/*
			 * We're interactive and this might be a bookmark file
			 * entered as a startfile rather than invoked via
			 * -book.  Check if it's in our bookmark file list, and
			 * if so, reload if with the relevant bookmark elements
			 * set.  - FM
			 */
			cp = NULL;
			if (temp[len] == '/') {
			    if (StrChr(&temp[(len + 1)], '/')) {
				HTSprintf0(&cp, ".%s", &temp[len]);
			    } else {
				StrAllocCopy(cp, &temp[(len + 1)]);
			    }
			} else {
			    StrAllocCopy(cp, &temp[len]);
			}
			for (i = 0; i <= MBM_V_MAXFILES; i++) {
			    if (MBM_A_subbookmark[i] &&
				LYSameFilename(cp, MBM_A_subbookmark[i])) {
				StrAllocCopy(BookmarkPage,
					     MBM_A_subbookmark[i]);
				break;
			    }
			}
			FREE(cp);
			if (i <= MBM_V_MAXFILES) {
			    FREE(temp);
			    if (LYValidate) {
				HTAlert(BOOKMARKS_DISABLED);
				CleanupMainLoop();
				return (EXIT_FAILURE);
			    }
			    if ((temp = HTParse(newdoc.address, "",
						PARSE_ACCESS + PARSE_HOST + PARSE_PUNCTUATION))) {
				set_address(&newdoc, temp);
				HTuncache_current_document();
				free_address(&curdoc);
				StrAllocCat(newdoc.address,
					    wwwName(Home_Dir()));
				StrAllocCat(newdoc.address, "/");
				StrAllocCat(newdoc.address,
					    (StrNCmp(BookmarkPage, "./", 2) ?
					     BookmarkPage :
					     (BookmarkPage + 2)));
				StrAllocCopy(newdoc.title, BOOKMARK_TITLE);
				StrAllocCopy(newdoc.bookmark, BookmarkPage);
#ifdef USE_COLOR_STYLE
				if (curdoc.style)
				    StrAllocCopy(newdoc.style, curdoc.style);
#endif
				StrAllocCopy(startrealm, newdoc.address);
				LYFreePostData(&newdoc);
				newdoc.isHEAD = FALSE;
				newdoc.safe = FALSE;
				FREE(temp);
				if (!strcmp(homepage, startfile))
				    StrAllocCopy(homepage, newdoc.address);
				StrAllocCopy(startfile, newdoc.address);
				CTRACE((tfp, "Reloading as bookmarks=%s\n",
					newdoc.address));
				goto try_again;
			    }
			}
		    }
		    cp = NULL;
		}
		FREE(temp);

		if (traversal) {
		    /*
		     * During traversal build up lists of all links traversed.
		     * Traversal mode is a special feature for traversing http
		     * links in the web.
		     */
		    if (traversal_link_to_add) {
			/*
			 * Add the address we sought to TRAVERSE_FILE.
			 */
			if (!lookup_link(traversal_link_to_add))
			    add_to_table(traversal_link_to_add);
			FREE(traversal_link_to_add);
		    }
		    if (curdoc.address && curdoc.title &&
			!isLYNXIMGMAP(curdoc.address))
			/*
			 * Add the address we got to TRAVERSE_FOUND_FILE.
			 */
			add_to_traverse_list(curdoc.address, curdoc.title);
		}

		/*
		 * If this was a LYNXDOWNLOAD, we still have curdoc, not a
		 * newdoc, so reset the address, title and positioning
		 * elements.  - FM
		 */
		if (newdoc.address && curdoc.address &&
		    isLYNXDOWNLOAD(newdoc.address)) {
		    copy_address(&newdoc, &curdoc);
		    StrAllocCopy(newdoc.title, NonNull(curdoc.title));
		    StrAllocCopy(newdoc.bookmark, curdoc.bookmark);
		    newdoc.line = curdoc.line;
		    newdoc.link = curdoc.link;
		    newdoc.internal_link = FALSE;	/* can't be true. - kw */
		}

		/*
		 * Set Newline to the saved line.  It contains the line the
		 * user was on if s/he has been in the file before, or it is 1
		 * if this is a new file.
		 *
		 * We already set Newline before getfile() and probably update
		 * it explicitly if popping from the history stack via LYpop()
		 * or LYpop_num() within getfile() cycle.
		 *
		 * In partial mode, Newline was probably updated in
		 * LYMainLoop_pageDisplay() if user scrolled the document while
		 * loading.  Incremental loading stage already closed in
		 * HT*Copy().
		 */
#ifdef DISP_PARTIAL
		/* Newline = newdoc.line; */
		display_partial = FALSE;	/* for sure, LYNXfoo:/ may be a problem */
#else
		/* Should not be needed either if we remove "DISP_PARTIAL" from
		 * LYHistory.c, but lets leave it as an important comment for
		 * now.
		 */
		/* Newline = newdoc.line; */
#endif

		/*
		 * If we are going to a target line or the first page of a
		 * popped document, override any www_search line result.
		 */
		if (LYGetNewline() > 1 || popped_doc == TRUE)
		    www_search_result = -1;

		/*
		 * Make sure curdoc.line will not be equal to Newline, so we
		 * get a redraw.
		 */
		curdoc.line = -1;
		break;
	    }			/* end switch */

	    if (TRACE) {
		if (!LYTraceLogFP || trace_mode_flag) {
		    LYSleepAlert();	/* allow me to look at the results */
		}
	    }

	    /*
	     * Set the files the same.
	     */
	    copy_address(&curdoc, &newdoc);
	    BStrCopy(curdoc.post_data, newdoc.post_data);
	    StrAllocCopy(curdoc.post_content_type, newdoc.post_content_type);
	    StrAllocCopy(curdoc.bookmark, newdoc.bookmark);
#ifdef USE_COLOR_STYLE
	    StrAllocCopy(curdoc.style, HText_getStyle());
	    if (curdoc.style != NULL)
		style_readFromFile(curdoc.style);
#endif
	    curdoc.isHEAD = newdoc.isHEAD;
	    curdoc.internal_link = newdoc.internal_link;

	    /*
	     * Set the remaining document elements and add to the visited links
	     * list.  - FM
	     */
	    if (ownerS_address != NULL) {
#ifndef USE_PRETTYSRC
		if (HTOutputFormat == WWW_SOURCE && !HText_getOwner())
#else
		if ((LYpsrc ? psrc_view : HTOutputFormat == WWW_SOURCE)
		    && !HText_getOwner())
#endif
		    HText_setMainTextOwner(ownerS_address);
		FREE(ownerS_address);
	    }
	    if (HText_getTitle()) {
		StrAllocCopy(curdoc.title, HText_getTitle());
	    } else if (!dump_output_immediately) {
		StrAllocCopy(curdoc.title, newdoc.title);
	    }
	    StrAllocCopy(owner_address, HText_getOwner());
	    curdoc.safe = HTLoadedDocumentIsSafe();
	    if (!dump_output_immediately) {
		LYAddVisitedLink(&curdoc);
	    }

	    /*
	     * Reset WWW present mode so that if we were getting the source, we
	     * get rendered HTML from now on.
	     */
	    HTOutputFormat = WWW_PRESENT;
#ifdef USE_PRETTYSRC
	    psrc_view = FALSE;
#endif

	    HTMLSetCharacterHandling(current_char_set);		/* restore, for sure? */

	    /*
	     * Reset all of the other relevant flags.  - FM
	     */
	    LYUserSpecifiedURL = FALSE;		/* only set for goto's and jumps's */
	    LYJumpFileURL = FALSE;	/* only set for jump's */
	    LYNoRefererForThis = FALSE;		/* always reset on return here */
	    reloading = FALSE;	/* set for RELOAD and NOCACHE keys */
	    HEAD_request = FALSE;	/* only set for HEAD requests */
	    LYPermitURL = FALSE;	/* only for LYValidate or check_realm */
	    ForcePush = FALSE;	/* only set for some PRINT requests. */
	    LYforce_HTML_mode = FALSE;
	    force_old_UCLYhndl_on_reload = FALSE;
	    popped_doc = FALSE;
	    pending_form_c = -1;

	}
	/* end if (LYforce_no_cache || force_load || are_different(...)) */
	if (dump_output_immediately) {
	    if (crawl) {
		print_crawl_to_fd(stdout, curdoc.address, curdoc.title);
	    } else if (!dump_links_only) {
		print_wwwfile_to_fd(stdout, FALSE, FALSE);
	    }
	    CleanupMainLoop();
	    return ((dump_server_status >= 400) ? EXIT_FAILURE : EXIT_SUCCESS);
	}

	/*
	 * If the recent_sizechange variable is set to TRUE then the window
	 * size changed recently.
	 */
	if (recent_sizechange) {
	    /*
	     * First we need to make sure the display library - curses, slang,
	     * whatever - gets notified about the change, and gets a chance to
	     * update external structures appropriately.  Hopefully the
	     * stop_curses()/start_curses() sequence achieves this, at least if
	     * the display library has a way to get the new screen size from
	     * the OS.
	     *
	     * However, at least for ncurses, the update of the internal
	     * structures will come still too late - the changed screen size is
	     * detected in doupdate(), which would only be called (indirectly
	     * through the HText_pageDisplay below) after the WINDOW structures
	     * are already filled based on the old size.  So we notify the
	     * ncurses library directly here.  - kw
	     */
#if defined(NCURSES) && defined(HAVE_RESIZETERM) && defined(HAVE_WRESIZE)
	    resizeterm(LYlines, LYcols);
	    wresize(LYwin, LYlines, LYcols);
#else
#if 0				/* defined(PDCURSES) && defined(HAVE_XCURSES) */
	    resize_term(LYlines, LYcols);
	    if (LYwin != 0)
		LYwin = resize_window(LYwin, LYlines, LYcols);
	    refresh();
#else
	    stop_curses();
	    start_curses();
	    LYclear();
#endif
#endif
	    refresh_screen = TRUE;	/* to force a redraw */
	    if (HTMainText)	/* to REALLY force it... - kw */
		HText_setStale(HTMainText);
	    recent_sizechange = FALSE;

	    LYSetDisplayLines();
	}

	if (www_search_result != -1) {
	    /*
	     * This was a WWW search, set the line to the result of the search.
	     */
	    LYSetNewline(www_search_result);
	    www_search_result = -1;	/* reset */
	}

	if (first_file == TRUE) {
	    /*
	     * We can never again have the first file.
	     */
	    first_file = FALSE;

	    /*
	     * Set the startrealm, and deal as best we can with preserving
	     * forced HTML mode for a local startfile.  - FM
	     */
	    temp = HTParse(curdoc.address, "",
			   PARSE_ACCESS + PARSE_HOST + PARSE_PUNCTUATION);
	    if (isEmpty(temp)) {
		StrAllocCopy(startrealm, NO_NOTHING);
	    } else {
		StrAllocCopy(startrealm, temp);
		FREE(temp);
		if (!(temp = HTParse(curdoc.address, "",
				     PARSE_PATH + PARSE_PUNCTUATION))) {
		    LYAddHtmlSep(&startrealm);
		} else {
		    if (forced_HTML_mode &&
			!dump_output_immediately &&
			!curdoc.bookmark &&
			isFILE_URL(curdoc.address) &&
			strlen(temp) > 1) {
			/*
			 * We forced HTML for a local startfile which is not a
			 * bookmark file and has a path of at least two
			 * letters.  If it doesn't have a suffix mapped to
			 * text/html, we'll set the entire path (including the
			 * lead slash) as a "suffix" mapped to text/html to
			 * ensure it is always treated as an HTML source file.
			 * We are counting on a tail match to this full path
			 * for some other URL fetched during the session having
			 * too low a probability to worry about, but it could
			 * happen.  - FM
			 */
			HTAtom *encoding;

			if (HTFileFormat(temp, &encoding, NULL) != WWW_HTML) {
			    HTSetSuffix(temp, STR_HTML, "8bit", 1.0);
			}
		    }
		    if ((cp = strrchr(temp, '/')) != NULL) {
			*(cp + 1) = '\0';
			StrAllocCat(startrealm, temp);
		    }
		}
	    }
	    FREE(temp);
	    CTRACE((tfp, "Starting realm is '%s'\n\n", startrealm));
	    if (traversal) {
		/*
		 * Set up the crawl output stuff.
		 */
		if (curdoc.address && !lookup_link(curdoc.address)) {
		    if (!isLYNXIMGMAP(curdoc.address))
			crawl_ok = TRUE;
		    add_to_table(curdoc.address);
		}
		/*
		 * Set up the traversal_host comparison string.
		 */
		if (StrNCmp((curdoc.address ? curdoc.address : "NULL"),
			    "http", 4)) {
		    StrAllocCopy(traversal_host, NO_NOTHING);
		} else if (check_realm) {
		    StrAllocCopy(traversal_host, startrealm);
		} else {
		    temp = HTParse(curdoc.address, "",
				   PARSE_ACCESS + PARSE_HOST + PARSE_PUNCTUATION);
		    if (isEmpty(temp)) {
			StrAllocCopy(traversal_host, NO_NOTHING);
		    } else {
			StrAllocCopy(traversal_host, temp);
			LYAddHtmlSep(&traversal_host);
		    }
		    FREE(temp);
		}
		CTRACE((tfp, "Traversal host is '%s'\n\n", traversal_host));
	    }
	    if (startfile) {
		/*
		 * If homepage was not equated to startfile, make the homepage
		 * URL the first goto entry.  - FM
		 */
		if (homepage && strcmp(startfile, homepage))
		    HTAddGotoURL(homepage);
		/*
		 * If we are not starting up with startfile (e.g., had -book),
		 * or if we are using the startfile and it has no POST content,
		 * make the startfile URL a goto entry.  - FM
		 */
		if (strcmp(startfile, newdoc.address) ||
		    newdoc.post_data == NULL)
		    HTAddGotoURL(startfile);
	    }
	    if (TRACE) {
		refresh_screen = TRUE;
		if (!LYTraceLogFP || trace_mode_flag) {
		    LYSleepAlert();
		}
	    }
	}
#ifdef USE_SOURCE_CACHE
	/*
	 * If the parse settings have changed since this HText was
	 * generated, we need to reparse and redraw it.  -dsb
	 *
	 * Should be configured to avoid shock for experienced lynx users.
	 * Currently enabled for cached sources only.
	 */
	if (HTdocument_settings_changed()) {
	    if (HTcan_reparse_document()) {
		HTInfoMsg(gettext("Reparsing document under current settings..."));
		reparse_document();
	    } else {
		/*
		 * Urk.  I have no idea how to recover from a failure here.
		 * At a guess, I'll try reloading.  -dsb
		 */
		/*  currently disabled ***
		   HTUserMsg(gettext("Reparsing document under current settings..."));
		   cmd = LYK_RELOAD;
		   goto new_cmd;
		 */
	    }
	}

	if (from_source_cache) {
	    from_source_cache = FALSE;	/* reset */
	    curdoc.line = -1;	/* so curdoc.line != Newline, see below */
	}
#endif

	/*
	 * If the curdoc.line is different than Newline then there must have
	 * been a change since last update.  Run HText_pageDisplay() to create
	 * a fresh screen of text output.
	 *
	 * If we got new HTMainText go this way.  All display_partial calls
	 * ends here for final redraw.
	 */
	if (curdoc.line != LYGetNewline()) {
#ifdef INACTIVE_INPUT_STYLE_VH
	    textinput_redrawn = FALSE;
#endif

	    refresh_screen = FALSE;

	    HText_pageDisplay(LYGetNewline(), prev_target->str);

#ifdef DIRED_SUPPORT
	    if (lynx_edit_mode && nlinks > 0 && !HTList_isEmpty(tagged))
		showtags(tagged);
#endif /* DIRED_SUPPORT */

	    /*
	     * Check if there is more info below this page.
	     */
	    more_text = HText_canScrollDown();

	    if (newdoc.link < 0)
		goto_line(LYGetNewline());
	    LYSetNewline(HText_getTopOfScreen() + 1);
	    curdoc.line = LYGetNewline();

	    if (curdoc.title == NULL) {
		/*
		 * If we don't yet have a title, try to get it, or set to that
		 * for newdoc.title.  - FM
		 */
		if (HText_getTitle()) {
		    StrAllocCopy(curdoc.title, HText_getTitle());
		} else {
		    StrAllocCopy(curdoc.title, newdoc.title);
		}
	    }

	    /*
	     * If the request is to highlight a link which is counted from the
	     * start of document, correct the link number:
	     */
	    if (newdoc_link_is_absolute) {
		newdoc_link_is_absolute = FALSE;
		if (curdoc.line > 1)
		    newdoc.link -= HText_LinksInLines(HTMainText, 1,
						      curdoc.line - 1);
	    }

	    if (arrowup) {
		/*
		 * arrowup is set if we just came up from a page below.
		 */
		curdoc.link = nlinks - 1;
		arrowup = FALSE;
	    } else {
		curdoc.link = newdoc.link;
		if (curdoc.link >= nlinks) {
		    curdoc.link = nlinks - 1;
		} else if (curdoc.link < 0 && nlinks > 0) {
		    /*
		     * We may have popped a doc (possibly in local_dired) which
		     * didn't have any links when it was pushed, but does have
		     * links now (e.g., a file was created).  Code below
		     * assumes that curdoc.link is valid and that
		     * (curdoc.link==-1) only occurs if (nlinks==0) is true.  -
		     * KW
		     */
		    curdoc.link = 0;
		}
	    }

	    show_help = FALSE;	/* reset */
	    newdoc.line = 1;
	    newdoc.link = 0;
	    curdoc.line = LYGetNewline();	/* set */
	} else if (newdoc.link < 0) {
	    newdoc.link = 0;	/* ...just in case getfile set this */
	}

	/*
	 * Refresh the screen if necessary.
	 */
	if (refresh_screen) {
#if defined(FANCY_CURSES) || defined (USE_SLANG)
	    if (enable_scrollback) {
		LYclear();
	    } else {
		LYerase();
	    }
#else
	    LYclear();
#endif /* FANCY_CURSES || USE_SLANG */
	    HText_pageDisplay(LYGetNewline(), prev_target->str);

#ifdef DIRED_SUPPORT
	    if (lynx_edit_mode && nlinks > 0 && !HTList_isEmpty(tagged))
		showtags(tagged);
#endif /* DIRED_SUPPORT */

	    /*
	     * Check if there is more info below this page.
	     */
	    more_text = HText_canScrollDown();

	    /*
	     * Adjust curdoc.link as above; nlinks may have changed, if the
	     * refresh_screen flag was set as a result of a size change.  Code
	     * below assumes that curdoc.link is valid and that
	     * (curdoc.link==-1) only occurs if (nlinks==0) is true.  - kw
	     */
	    if (curdoc.link >= nlinks) {
		curdoc.link = nlinks - 1;
	    } else if (curdoc.link < 0 && nlinks > 0) {
		curdoc.link = 0;
	    }

	    if (user_mode == NOVICE_MODE)
		noviceline(more_text);	/* print help message */
	    refresh_screen = FALSE;

	}

	curlink_is_editable = (BOOLEAN)
	    (nlinks > 0 &&
	     LinkIsTextLike(curdoc.link));

	use_last_tfpos = (BOOLEAN)
	    (curlink_is_editable &&
	     (real_cmd == LYK_LPOS_PREV_LINK ||
	      real_cmd == LYK_LPOS_NEXT_LINK));

#ifdef TEXTFIELDS_MAY_NEED_ACTIVATION
	if (!textfields_need_activation)
	    textinput_activated = TRUE;
#endif

#if defined(WIN_EX)		/* 1997/10/08 (Wed) 14:52:06 */
	if (nlinks > 0) {
	    char *p = "LYNX (unknown link type)";

	    /* Show the URL & kanji code . */
	    if (strlen(links[curdoc.link].lname) == 0) {

		if (links[curdoc.link].type == WWW_FORM_LINK_TYPE) {

		    switch (links[curdoc.link].l_form->type) {
		    case F_TEXT_SUBMIT_TYPE:
		    case F_SUBMIT_TYPE:
		    case F_IMAGE_SUBMIT_TYPE:
			p = "[SUBMIT]";
			break;
		    case F_PASSWORD_TYPE:
			p = "Password";
			break;
		    case F_OPTION_LIST_TYPE:
			p = "Option list";
			break;
		    case F_CHECKBOX_TYPE:
			p = "Check box";
			break;
		    case F_RADIO_TYPE:
			p = "[Radio]";
			break;
		    case F_RESET_TYPE:
			p = "[Reset]";
			break;
		    case F_TEXT_TYPE:
			p = "Text input";
			break;
		    case F_TEXTAREA_TYPE:
			p = "Text input lines";
			break;
		    default:
			break;
		    }
		    set_ws_title(p);
		}
	    } else {
		if (user_mode == ADVANCED_MODE) {
		    p = curdoc.title;
		} else {
		    p = links[curdoc.link].lname;
		}

		if (strlen(p) < ((sizeof(sjis_buff) / 2) - 1)) {
		    strcpy(temp_buff, p);
		    if (StrChr(temp_buff, '%')) {
			HTUnEscape(temp_buff);
		    }
		    str_sjis(sjis_buff, temp_buff);
		    set_ws_title(LYElideString(sjis_buff, 10));
		}
	    }
	} else {
	    if (strlen(curdoc.address) < sizeof(temp_buff) - 1) {
		if (user_mode == ADVANCED_MODE) {
		    str_sjis(temp_buff, curdoc.title);
		} else {
		    strcpy(temp_buff, curdoc.address);
		}
		set_ws_title(HTUnEscape(temp_buff));
	    }
	}
#endif /* WIN_EX */

	/*
	 * Report unread or new mail, if appropriate.
	 */
	if (check_mail && !no_mail)
	    LYCheckMail();

	/*
	 * If help is not on the screen, then put a message on the screen to
	 * tell the user other misc info.
	 */
	if (!show_help) {
	    show_main_statusline(links[curdoc.link],
				 ((curlink_is_editable &&
				   textinput_activated)
				  ? FOR_INPUT
				  : FOR_PANEL));
	} else {
	    show_help = FALSE;
	}

	if (nlinks > 0) {
	    /*
	     * Highlight current link, unless it is an active text input field.
	     */
	    if (!curlink_is_editable) {
		LYhighlight(TRUE, curdoc.link, prev_target->str);
#ifndef INACTIVE_INPUT_STYLE_VH
	    } else if (!textinput_activated) {
		LYhighlight(TRUE, curdoc.link, prev_target->str);
#endif
	    }
	}

	if (traversal) {
	    /*
	     * Don't go interactively into forms, or accept keystrokes from the
	     * user
	     */
	    if (crawl && crawl_ok) {
		crawl_ok = FALSE;
#ifdef FNAMES_8_3
		sprintf(cfile, "lnk%05d.dat", crawl_count);
#else
		sprintf(cfile, "lnk%08d.dat", crawl_count);
#endif /* FNAMES_8_3 */
		crawl_count = crawl_count + 1;
		if ((cfp = LYNewTxtFile(cfile)) != NULL) {
		    print_crawl_to_fd(cfp, curdoc.address, curdoc.title);
		    LYCloseOutput(cfp);
		} else {
#ifdef UNIX
		    FILE *fp = (dump_output_immediately
				? stderr
				: stdout);

#else
		    FILE *fp = stdout;
#endif
		    if (!dump_output_immediately)
			cleanup();
		    fprintf(fp,
			    gettext("Fatal error - could not open output file %s\n"),
			    cfile);
		    CleanupMainLoop();
		    if (!dump_output_immediately) {
			exit_immediately(EXIT_FAILURE);
		    }
		    return (EXIT_FAILURE);
		}
	    }
	} else {
	    /*
	     * Normal, non-traversal handling.
	     */
	    if (curlink_is_editable &&
		(textinput_activated || pending_form_c != -1)) {
		if (pending_form_c != -1) {
		    real_c = pending_form_c;
		    pending_form_c = -1;
		} else {
		    /*
		     * Replace novice lines if in NOVICE_MODE.
		     */
		    if (user_mode == NOVICE_MODE) {
			form_noviceline(FormIsReadonly(links[curdoc.link].l_form));
		    }
		    real_c = change_form_link(curdoc.link,
					      &newdoc, &refresh_screen,
					      use_last_tfpos, FALSE);
		}
#ifdef TEXTFIELDS_MAY_NEED_ACTIVATION
		if (textfields_need_activation)
		    textinput_activated = FALSE;
#ifdef INACTIVE_INPUT_STYLE_VH
		textinput_redrawn = FALSE;
#endif
#endif

		c = (real_c == LKC_DONE) ? DO_NOTHING : LKC_TO_C(real_c);
		if (c != DO_NOTHING &&
		    peek_mouse_link() != -1 && peek_mouse_link() != -2)
		    old_c = 0;
		if (peek_mouse_link() >= 0 &&
		    LKC_TO_LAC(keymap, real_c) != LYK_CHANGE_LINK) {
		    do_change_link();
		    if ((c == '\n' || c == '\r') &&
			LinkIsTextLike(curdoc.link) &&
			!textfields_need_activation) {
			c = DO_NOTHING;
		    }
#ifdef TEXTFIELDS_MAY_NEED_ACTIVATION
		} else if (LinkIsTextarea(curdoc.link)
			   && textfields_need_activation
			   && !FormIsReadonly(links[curdoc.link].l_form)
			   && peek_mouse_link() < 0 &&
			   (((LKC_TO_LAC(keymap, real_c) == LYK_NEXT_LINK ||
#ifdef TEXTAREA_AUTOGROW
			      LKC_TO_LAC(keymap, real_c) == LYK_ACTIVATE ||
#endif
			      LKC_TO_LAC(keymap, real_c) == LYK_LPOS_NEXT_LINK ||
			      LKC_TO_LAC(keymap, real_c) == LYK_DOWN_LINK) &&
			     ((curdoc.link < nlinks - 1 &&
			       LinkIsTextarea(curdoc.link + 1)
			       && (links[curdoc.link].l_form->number ==
				   links[curdoc.link + 1].l_form->number)
			       && strcmp(links[curdoc.link].l_form->name,
					 links[curdoc.link + 1].l_form->name)
			       == 0) ||
			      (curdoc.link == nlinks - 1 && more_text &&
			       HText_TAHasMoreLines(curdoc.link, 1)))) ||
			    ((LKC_TO_LAC(keymap, real_c) == LYK_PREV_LINK ||
			      LKC_TO_LAC(keymap, real_c) == LYK_LPOS_PREV_LINK ||
			      LKC_TO_LAC(keymap, real_c) == LYK_UP_LINK) &&
			     ((curdoc.link > 0 &&
			       LinkIsTextarea(curdoc.link - 1)
			       && (links[curdoc.link].l_form->number ==
				   links[curdoc.link - 1].l_form->number) &&
			       strcmp(links[curdoc.link].l_form->name,
				      links[curdoc.link - 1].l_form->name) == 0)
			      || (curdoc.link == 0 && curdoc.line > 1 &&
				  HText_TAHasMoreLines(curdoc.link, -1)))))) {
		    textinput_activated = TRUE;
#ifdef TEXTAREA_AUTOGROW
		    if ((c == '\n' || c == '\r') &&
			LKC_TO_LAC(keymap, real_c) == LYK_ACTIVATE)
			c = LAC_TO_LKC0(LYK_NEXT_LINK);
#endif /* TEXTAREA_AUTOGROW */
#endif /* TEXTFIELDS_MAY_NEED_ACTIVATION */
		} else
		    switch (c) {
		    case '\n':
		    case '\r':
#ifdef TEXTAREA_AUTOGROW
			/*
			 * If on the bottom line of a TEXTAREA, and the user
			 * hit the ENTER key, we add a new line/anchor
			 * automatically, positioning the cursor on it.
			 *
			 * If at the bottom of the screen, we effectively
			 * perform an LYK_DOWN_HALF-like operation, then move
			 * down to the new line we just added.  --KED 02/14/99
			 *
			 * [There is some redundancy and non-standard
			 * indentation in the monster-if() below.  This is
			 * intentional ...  to try and improve the
			 * "readability" (such as it is).  Caveat emptor to
			 * anyone trying to change it.]
			 */
			if (LinkIsTextarea(curdoc.link)
			    && ((curdoc.link == nlinks - 1 &&
				 !(more_text &&
				   HText_TAHasMoreLines(curdoc.link, 1)))
				||
				((curdoc.link < nlinks - 1) &&
				 !LinkIsTextarea(curdoc.link + 1))
				||
				((curdoc.link < nlinks - 1) &&
				 (LinkIsTextarea(curdoc.link + 1)
				  && ((links[curdoc.link].l_form->number !=
				       links[curdoc.link + 1].l_form->number) ||
				      (strcmp(links[curdoc.link].l_form->name,
					      links[curdoc.link + 1].l_form->name)
				       != 0)))))) {

			    HText_ExpandTextarea(&links[curdoc.link], 1);

			    if (links[curdoc.link].ly < display_lines) {
				refresh_screen = TRUE;
			    } else {
				LYChgNewline(display_lines / 2);
				if (nlinks > 0 && curdoc.link > -1 &&
				    links[curdoc.link].ly > display_lines / 2) {
				    newdoc.link = curdoc.link;
				    for (i = 0;
					 links[i].ly <= (display_lines / 2);
					 i++)
					--newdoc.link;
				    newdoc.link++;
				}
			    }
#ifdef TEXTFIELDS_MAY_NEED_ACTIVATION
			    if (textfields_need_activation) {
				textinput_activated = TRUE;
				textfields_need_activation = textfields_activation_option;
#ifdef INACTIVE_INPUT_STYLE_VH
				textinput_redrawn = TRUE;
#endif
			    };
#endif

			}
#endif /* TEXTAREA_AUTOGROW */

			/*
			 * Make return in input field (if it was returned by
			 * change_form_link) act as LYK_NEXT_LINK, independent
			 * of what key (if any) is mapped to LYK_NEXT_LINK.  -
			 * kw
			 */
			c = LAC_TO_LKC0(LYK_NEXT_LINK);
			break;
		    default:

			if (old_c != c && old_c != real_c && c != real_c)
			    real_c = c;
		    }
	    } else {
#if defined(TEXTFIELDS_MAY_NEED_ACTIVATION) && defined(INACTIVE_INPUT_STYLE_VH)
		if (curlink_is_editable && !textinput_redrawn) {
		    /*draw the text entry, but don't activate it */
		    textinput_redrawn = TRUE;
		    change_form_link_ex(curdoc.link,
					&newdoc, &refresh_screen,
					use_last_tfpos, FALSE, TRUE);
		    if (LYShowCursor) {
			LYmove(links[curdoc.link].ly,
			       ((links[curdoc.link].lx > 0) ?
				(links[curdoc.link].lx - 1) : 0));
		    } else {
			LYHideCursor();
		    }
		}
#endif /* TEXTFIELDS_MAY_NEED_ACTIVATION && INACTIVE_INPUT_STYLE_VH */
		/*
		 * Get a keystroke from the user.  Save the last keystroke to
		 * avoid redundant error reporting.
		 */
		real_c = c = LYgetch();		/* get user input */

		if (c != last_key)
		    key_count = 0;
		key_count++;
		last_key = c;
#ifndef VMS
		if (c == 3) {	/* ^C */
		    /*
		     * This shouldn't happen.  We'll try to deal with whatever
		     * bug caused it.  - FM
		     */
		    signal(SIGINT, cleanup_sig);
		    old_c = 0;
		    cmd = LYK_QUIT;
		    goto new_cmd;
		}
#endif /* !VMS */
		if (LKC_HAS_ESC_MOD(c) && EditBinding(c) != LYE_FORM_PASS) {
		    /*
		     * If ESC + <key> was read (and not recognized as a
		     * terminal escape sequence for another key), ignore the
		     * ESC modifier and act on <key> only if the line editor
		     * binding would have passed the same ESC-modified
		     * lynxkeycode back to us if it had been pressed in a text
		     * input field.  Otherwise set interesting part so that it
		     * will map to 0, to prevent that ESC + <key> acts like
		     * <key>, which might be unexpected.  - kw
		     */
		    c = (c & ~LKC_MASK) | LAC_TO_LKC(0);
		}
		if (old_c != real_c) {
		    old_c = 0;
		}
	    }
	}

#ifdef VMS
	if (HadVMSInterrupt) {
	    HadVMSInterrupt = FALSE;
	    c = DO_NOTHING;
	}
#else
	if (recent_sizechange) {
	    if (c <= 0)
		c = DO_NOTHING;
	}
#endif /* VMS */

      new_keyboard_input:
	/*
	 * A goto point for new input without going back through the getch()
	 * loop.
	 */
	if (traversal) {
	    if ((c = DoTraversal(c, &crawl_ok)) < 0) {
		CleanupMainLoop();
		return (EXIT_FAILURE);
	    }
	}
	/* traversal */
#ifdef WIN_EX
	if (c == DO_NOTHING)
	    cmd = LYK_DO_NOTHING;
	else
#endif
	    cmd = LKC_TO_LAC(keymap, c);	/* adds 1 to map EOF to 0 */

#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
	if (lynx_edit_mode && !no_dired_support && LKC_TO_LAC(key_override, c))
	    cmd = LKC_TO_LAC(key_override, c);
#endif /* DIRED_SUPPORT && OK_OVERRIDE */

	real_cmd = cmd;

	/*
	 * A goto point for new input without going back through the getch()
	 * loop.
	 */
      new_cmd:

	force_old_UCLYhndl_on_reload = FALSE;
	CTRACE_FLUSH(tfp);

	if (cmd != LYK_UP_LINK && cmd != LYK_DOWN_LINK)
	    follow_col = -1;

	CTRACE((tfp, "Handling key as %s\n",
		((LYKeycodeToKcmd((LYKeymapCode) cmd) != 0)
		 ? LYKeycodeToKcmd((LYKeymapCode) cmd)->name
		 : "unknown")));
	switch (cmd) {
	case -1:
	    HTUserMsg(COMMAND_UNKNOWN);
	    break;
	case 0:		/* unmapped character */
	default:
	    if (curdoc.link >= 0 && curdoc.link < nlinks &&
		LinkIsTextLike(curdoc.link)) {

#ifdef TEXTFIELDS_MAY_NEED_ACTIVATION
		if (textfields_need_activation) {
		    show_main_statusline(links[curdoc.link], FOR_PANEL);
#ifdef INACTIVE_INPUT_STYLE_VH
		    textinput_redrawn = FALSE;
#endif
		} else
#endif
		    show_main_statusline(links[curdoc.link], FOR_INPUT);
	    } else if (more_text) {
		HTInfoMsg(MOREHELP);
	    } else {
		HTInfoMsg(HELP);
	    }
	    show_help = TRUE;

	    if (TRACE) {
		sprintf(cfile, "%d", c);
		LYaddstr(cfile);	/* show the user input */
		cfile[0] = '\0';
	    }
	    break;

	case LYK_COMMAND:
	    cmd = handle_LYK_COMMAND(&user_input_buffer);
	    goto new_cmd;

	case LYK_INTERRUPT:
	    /*
	     * No network transmission to interrupt - 'til we multithread.
	     */
	    break;

	case LYK_F_LINK_NUM:
	    c = '\0';
	    /* FALLTHRU */
	case LYK_1:		/* FALLTHRU */
	case LYK_2:		/* FALLTHRU */
	case LYK_3:		/* FALLTHRU */
	case LYK_4:		/* FALLTHRU */
	case LYK_5:		/* FALLTHRU */
	case LYK_6:		/* FALLTHRU */
	case LYK_7:		/* FALLTHRU */
	case LYK_8:		/* FALLTHRU */
	case LYK_9:
	    handle_LYK_digit(c, &force_load, &old_c, real_c, &try_internal);
	    break;

	case LYK_SOURCE:	/* toggle view source mode */
	    handle_LYK_SOURCE(&ownerS_address);
	    break;

	case LYK_CHANGE_CENTER:	/* ^Q */

	    if (no_table_center) {
		no_table_center = FALSE;
		HTInfoMsg(gettext("TABLE center enable."));
	    } else {
		no_table_center = TRUE;
		HTInfoMsg(gettext("TABLE center disable."));
	    }
	    /* FALLTHRU */

	case LYK_RELOAD:	/* control-R to reload and refresh */
	    handle_LYK_RELOAD(real_cmd);
	    break;

	case LYK_HISTORICAL:	/* toggle 'historical' comments parsing */
	    handle_LYK_HISTORICAL();
	    break;

	case LYK_MINIMAL:	/* toggle 'minimal' comments parsing */
	    handle_LYK_MINIMAL();
	    break;

	case LYK_SOFT_DQUOTES:
	    handle_LYK_SOFT_DQUOTES();
	    break;

	case LYK_SWITCH_DTD:
	    handle_LYK_SWITCH_DTD();
	    break;

	case LYK_QUIT:		/* quit */
	    if (handle_LYK_QUIT()) {
		CleanupMainLoop();
		return (EXIT_SUCCESS);
	    }
	    break;

	case LYK_ABORT:	/* don't ask the user about quitting */
	    CleanupMainLoop();
	    return (EXIT_SUCCESS);

	case LYK_NEXT_PAGE:	/* next page */
	    handle_LYK_NEXT_PAGE(&old_c, real_c);
	    break;

	case LYK_PREV_PAGE:	/* page up */
	    handle_LYK_PREV_PAGE(&old_c, real_c);
	    break;

	case LYK_UP_TWO:
	    handle_LYK_UP_TWO(&arrowup, &old_c, real_c);
	    break;

	case LYK_DOWN_TWO:
	    handle_LYK_DOWN_TWO(&old_c, real_c);
	    break;

	case LYK_UP_HALF:
	    handle_LYK_UP_HALF(&arrowup, &old_c, real_c);
	    break;

	case LYK_DOWN_HALF:
	    handle_LYK_DOWN_HALF(&old_c, real_c);
	    break;

#ifdef CAN_CUT_AND_PASTE
	case LYK_TO_CLIPBOARD:	/* ^S */
	    {
		char *s;
		int ch2;

		/* The logic resembles one of ADD_BOOKMARK */
		if (nlinks > 0 && links[curdoc.link].lname
		    && links[curdoc.link].type != WWW_FORM_LINK_TYPE) {
		    /* Makes sense to copy a link */
		    _statusline("Copy D)ocument's or L)ink's URL to clipboard or C)ancel?");
		    ch2 = LYgetch_single();
		    if (ch2 == 'D')
			s = curdoc.address;
		    else if (ch2 == 'C')
			break;
		    else
			s = links[curdoc.link].lname;
		} else
		    s = curdoc.address;
		if (isEmpty(s))
		    HTInfoMsg(gettext("Current URL is empty."));
		if (put_clip(s))
		    HTInfoMsg(gettext("Copy to clipboard failed."));
		else if (s == curdoc.address)
		    HTInfoMsg(gettext("Document URL put to clipboard."));
		else
		    HTInfoMsg(gettext("Link URL put to clipboard."));
	    }
	    break;

	case LYK_PASTE_URL:
	    if (no_goto && !LYValidate) {	/*  Go to not allowed. - FM */
		HTUserMsg(GOTO_DISALLOWED);
	    } else {
		unsigned char *s = (unsigned char *) get_clip_grab(), *e, *t;
		char *buf;
		int len2;

		if (!s)
		    break;
		len2 = (int) strlen((const char *) s);
		e = s + len2;
		while (s < e && StrChr(" \t\n\r", *s))
		    s++;
		while (s < e && StrChr(" \t\n\r", e[-1]))
		    e--;
		if (s[0] == '<' && e > s && e[-1] == '>') {
		    s++;
		    e--;
		    if (!strncasecomp((const char *) s, "URL:", 4))
			s += 4;
		}
		if (s >= e) {
		    HTInfoMsg(gettext("No URL in the clipboard."));
		    break;
		}
		len = (unsigned) (e - s + 1);
		if (len < MAX_LINE)
		    len = MAX_LINE;	/* Required for do_check_goto_URL() */
		buf = typeMallocn(char, len);

		LYStrNCpy(buf, (const char *) s, (e - s));
		t = (unsigned char *) buf;

		while (s < e) {
		    if (StrChr(" \t\n\r", *s)) {
			int nl2 = 0;	/* Keep whitespace without NL - file names! */
			unsigned char *s1 = s;

			while (StrChr(" \t\n\r", *s)) {
			    if (!nl2 && *s == '\n')
				nl2 = 1;
			    s++;
			}
			if (!nl2) {
			    while (s1 < s) {
				if (*s1 != '\r' && *s1 != '\n')
				    *t = *s1;
				t++, s1++;
			    }
			}
		    } else
			*t++ = *s++;
		}
		*t = '\0';
		get_clip_release();
		BStrCopy0(user_input_buffer, buf);
		do_check_goto_URL(&user_input_buffer, &temp, &force_load);
		free(buf);
	    }
	    break;
#endif

#ifdef KANJI_CODE_OVERRIDE
	case LYK_CHG_KCODE:
	    if (LYRawMode && (HTCJK == JAPANESE)) {
		switch (last_kcode) {
		case NOKANJI:
		    last_kcode = SJIS;
		    break;
		case SJIS:
		    last_kcode = EUC;
		    break;
		case EUC:
		    last_kcode = NOKANJI;
		    break;
		default:
		    break;
		}
	    }
	    LYmove(0, 0);
	    lynx_start_title_color();
	    LYaddstr(str_kcode(last_kcode));
	    lynx_stop_title_color();

	    break;
#endif

	case LYK_REFRESH:
	    refresh_screen = TRUE;
	    lynx_force_repaint();
	    break;

	case LYK_HOME:
	    if (curdoc.line > 1) {
		LYSetNewline(1);
	    } else {
		cmd = LYK_PREV_PAGE;
		goto new_cmd;
	    }
	    break;

	case LYK_END:
	    i = HText_getNumOfLines() - display_lines + 2;
	    if (i >= 1 && LYGetNewline() != i) {
		LYSetNewline(i);	/* go to end of file */
		arrowup = TRUE;	/* position on last link */
	    } else {
		cmd = LYK_NEXT_PAGE;
		goto new_cmd;
	    }
	    break;

	case LYK_FIRST_LINK:
	    handle_LYK_FIRST_LINK();
	    break;

	case LYK_LAST_LINK:
	    handle_LYK_LAST_LINK();
	    break;

	case LYK_PREV_LINK:
	case LYK_LPOS_PREV_LINK:
	    handle_LYK_PREV_LINK(&arrowup, &old_c, real_c);
	    break;

	case LYK_NEXT_LINK:
	case LYK_LPOS_NEXT_LINK:
	    handle_LYK_NEXT_LINK(c, &old_c, real_c);
	    break;

	case LYK_FASTFORW_LINK:
	    handle_LYK_FASTFORW_LINK(&old_c, real_c);
	    break;

	case LYK_FASTBACKW_LINK:
	    if (handle_LYK_FASTBACKW_LINK(&cmd, &old_c, real_c))
		goto new_cmd;
	    break;

	case LYK_UP_LINK:
	    handle_LYK_UP_LINK(&follow_col, &arrowup, &old_c, real_c);
	    break;

	case LYK_DOWN_LINK:
	    handle_LYK_DOWN_LINK(&follow_col, &old_c, real_c);
	    break;

	case LYK_CHANGE_LINK:
	    do_change_link();
#if defined(TEXTFIELDS_MAY_NEED_ACTIVATION) && defined(INACTIVE_INPUT_STYLE_VH)
	    if (textfields_need_activation)
		textinput_redrawn = FALSE;
#endif /* TEXTFIELDS_MAY_NEED_ACTIVATION && INACTIVE_INPUT_STYLE_VH */
	    break;

	case LYK_RIGHT_LINK:
	    handle_LYK_RIGHT_LINK();
	    break;

	case LYK_LEFT_LINK:
	    handle_LYK_LEFT_LINK();
	    break;

	case LYK_COOKIE_JAR:	/* show the cookie jar */
	    if (handle_LYK_COOKIE_JAR(&cmd))
		goto new_cmd;
	    break;

#ifdef USE_CACHEJAR
	case LYK_CACHE_JAR:	/* show the cache jar */
	    if (handle_LYK_CACHE_JAR(&cmd))
		goto new_cmd;
	    break;
#endif

	case LYK_HISTORY:	/* show the history page */
	    if (handle_LYK_HISTORY(ForcePush))
		break;

	    /* FALLTHRU */
	case LYK_PREV_DOC:	/* back up a level */
	    switch (handle_PREV_DOC(&cmd, &old_c, real_c)) {
	    case 1:
		CleanupMainLoop();
		return (EXIT_SUCCESS);
	    case 2:
		goto new_cmd;
	    }
	    break;

	case LYK_NEXT_DOC:	/* undo back up a level */
	    handle_NEXT_DOC();
	    break;

	case LYK_NOCACHE:	/* Force submission of form or link with no-cache */
	    if (!handle_LYK_NOCACHE(&old_c, real_c))
		break;

	    /* FALLTHRU */
	case LYK_ACTIVATE:	/* follow a link */
	case LYK_MOUSE_SUBMIT:	/* follow a link, submit TEXT_SUBMIT input */
	    switch (handle_LYK_ACTIVATE(&c,
					cmd,
					&try_internal,
					&refresh_screen,
					&force_load,
					real_cmd)) {
	    case 1:
		continue;
	    case 2:
		goto new_keyboard_input;
	    case 3:
		pending_form_c = c;
		break;
	    }
	    break;

	case LYK_SUBMIT:
	    handle_LYK_SUBMIT(curdoc.link, &newdoc, &refresh_screen);
	    break;

	case LYK_RESET:
	    handle_LYK_RESET(curdoc.link, &refresh_screen);
	    break;

	case LYK_ELGOTO:	/* edit URL of current link and go to it  */
	    if (handle_LYK_ELGOTO(&ch, &user_input_buffer, &temp, &old_c, real_c))
		do_check_goto_URL(&user_input_buffer, &temp, &force_load);
	    break;

	case LYK_ECGOTO:	/* edit current URL and go to to it     */
	    if (handle_LYK_ECGOTO(&ch, &user_input_buffer, &temp, &old_c, real_c))
		do_check_goto_URL(&user_input_buffer, &temp, &force_load);
	    break;

	case LYK_GOTO:		/* 'g' to goto a random URL  */
	    if (handle_LYK_GOTO(&ch, &user_input_buffer, &temp, &recall,
				&URLTotal, &URLNum, &FirstURLRecall, &old_c,
				real_c)) {
		if (do_check_recall(ch, &user_input_buffer, &temp, URLTotal,
				    &URLNum, recall, &FirstURLRecall))
		    do_check_goto_URL(&user_input_buffer, &temp, &force_load);
	    }
	    break;

	case LYK_DWIMHELP:	/* show context-dependent help file */
	    handle_LYK_DWIMHELP(&cshelpfile);
	    /* FALLTHRU */

	case LYK_HELP:		/* show help file */
	    handle_LYK_HELP(&cshelpfile);
	    break;

	case LYK_INDEX:	/* index file */
	    handle_LYK_INDEX(&old_c, real_c);
	    break;

	case LYK_MAIN_MENU:	/* return to main screen */
	    handle_LYK_MAIN_MENU(&old_c, real_c);
	    break;

#ifdef EXP_NESTED_TABLES
	case LYK_NESTED_TABLES:
	    if (handle_LYK_NESTED_TABLES(&cmd))
		goto new_cmd;
	    break;
#endif
	case LYK_OPTIONS:	/* options screen */
	    if (handle_LYK_OPTIONS(&cmd, &refresh_screen))
		goto new_cmd;
	    break;

	case LYK_INDEX_SEARCH:	/* search for a user string */
	    handle_LYK_INDEX_SEARCH(&force_load, ForcePush, &old_c, real_c);
	    break;

	case LYK_WHEREIS:	/* search within the document */
	case LYK_NEXT:		/* find the next occurrence in the document */
	case LYK_PREV:		/* find the previous occurrence in the document */
	    handle_LYK_WHEREIS(cmd, &refresh_screen);
	    break;

	case LYK_COMMENT:	/* reply by mail */
	    handle_LYK_COMMENT(&refresh_screen, &owner_address, &old_c, real_c);
	    break;

#ifdef DIRED_SUPPORT
	case LYK_TAG_LINK:	/* tag or untag the current link */
	    handle_LYK_TAG_LINK();
	    break;

	case LYK_MODIFY:	/* rename a file or directory */
	    handle_LYK_MODIFY(&refresh_screen);
	    break;

	case LYK_CREATE:	/* create a new file or directory */
	    handle_LYK_CREATE();
	    break;
#endif /* DIRED_SUPPORT */

	case LYK_DWIMEDIT:	/* context-dependent edit */
	    switch (handle_LYK_DWIMEDIT(&cmd, &old_c, real_c)) {
	    case 1:
		continue;
	    case 2:
		goto new_cmd;
	    }
	    /* FALLTHRU */

	case LYK_EDIT:		/* edit */
	    handle_LYK_EDIT(&old_c, real_c);
	    break;

	case LYK_DEL_BOOKMARK:	/* remove a bookmark file link */
	    handle_LYK_DEL_BOOKMARK(&refresh_screen, &old_c, real_c);
	    break;

#ifdef DIRED_SUPPORT
	case LYK_REMOVE:	/* remove files and directories */
	    handle_LYK_REMOVE(&refresh_screen);
	    break;
#endif /* DIRED_SUPPORT */

#if defined(DIRED_SUPPORT) && defined(OK_INSTALL)
	case LYK_INSTALL:	/* install a file into system area */
	    handle_LYK_INSTALL();
	    break;
#endif /* DIRED_SUPPORT && OK_INSTALL */

	case LYK_INFO:		/* show document info */
	    if (handle_LYK_INFO(&cmd))
		goto new_cmd;
	    break;

	case LYK_EDITTEXTAREA:	/* use external editor on a TEXTAREA - KED */
	    handle_LYK_EDIT_TEXTAREA(&refresh_screen, &old_c, real_c);
	    break;

	case LYK_GROWTEXTAREA:	/* add new lines to bottom of TEXTAREA - KED */
	    handle_LYK_GROW_TEXTAREA(&refresh_screen);
	    break;

	case LYK_INSERTFILE:	/* insert file in TEXTAREA, above cursor - KED */
	    handle_LYK_INSERT_FILE(&refresh_screen, &old_c, real_c);
	    break;

	case LYK_PRINT:	/* print the file */
	    handle_LYK_PRINT(&ForcePush, &old_c, real_c);
	    break;

	case LYK_LIST:		/* list links in the current document */
	    if (handle_LYK_LIST(&cmd))
		goto new_cmd;
	    break;

#ifdef USE_ADDRLIST_PAGE
	case LYK_ADDRLIST:	/* always list URL's (only) */
	    if (handle_LYK_ADDRLIST(&cmd))
		goto new_cmd;
	    break;
#endif /* USE_ADDRLIST_PAGE */

	case LYK_VLINKS:	/* list links visited during the current session */
	    if (handle_LYK_VLINKS(&cmd, &newdoc_link_is_absolute))
		goto new_cmd;
	    break;

	case LYK_TOOLBAR:	/* go to Toolbar or Banner in current document */
	    handle_LYK_TOOLBAR(&try_internal, &force_load, &old_c, real_c);
	    break;

#if defined(DIRED_SUPPORT) || defined(VMS)
	case LYK_DIRED_MENU:	/* provide full file management menu */
	    handle_LYK_DIRED_MENU(&refresh_screen, &old_c, real_c);
	    break;
#endif /* DIRED_SUPPORT || VMS */

#ifdef USE_EXTERNALS
	case LYK_EXTERN_LINK:	/* use external program on url */
	    handle_LYK_EXTERN_LINK(&refresh_screen);
	    break;
	case LYK_EXTERN_PAGE:	/* use external program on current page */
	    handle_LYK_EXTERN_PAGE(&refresh_screen);
	    break;
#endif /* USE_EXTERNALS */

	case LYK_ADD_BOOKMARK:	/* add link to bookmark file */
	    handle_LYK_ADD_BOOKMARK(&refresh_screen, &old_c, real_c);
	    break;

	case LYK_VIEW_BOOKMARK:	/* v to view home page */
	    handle_LYK_VIEW_BOOKMARK(&refresh_screen, &old_c, real_c);
	    break;

	case LYK_SHELL:	/* (!) shell escape */
	    handle_LYK_SHELL(&refresh_screen, &old_c, real_c);
	    break;

	case LYK_DOWNLOAD:
	    switch (handle_LYK_DOWNLOAD(&cmd, &old_c, real_c)) {
	    case 1:
		continue;
	    case 2:
		goto new_cmd;
	    }
	    break;

#ifdef DIRED_SUPPORT
	case LYK_UPLOAD:
	    handle_LYK_UPLOAD();
	    break;
#endif /* DIRED_SUPPORT */

	case LYK_TRACE_TOGGLE:	/*  Toggle TRACE mode. */
	    handle_LYK_TRACE_TOGGLE();
	    break;

	case LYK_TRACE_LOG:	/*  View TRACE log. */
	    handle_LYK_TRACE_LOG(&trace_mode_flag);
	    break;

	case LYK_IMAGE_TOGGLE:
	    if (handle_LYK_IMAGE_TOGGLE(&cmd))
		goto new_cmd;
	    break;

	case LYK_INLINE_TOGGLE:
	    if (handle_LYK_INLINE_TOGGLE(&cmd))
		goto new_cmd;
	    break;

	case LYK_RAW_TOGGLE:
	    if (handle_LYK_RAW_TOGGLE(&cmd))
		goto new_cmd;
	    break;

	case LYK_HEAD:
	    if (handle_LYK_HEAD(&cmd))
		goto new_cmd;
	    break;

	case LYK_TOGGLE_HELP:
	    handle_LYK_TOGGLE_HELP();
	    break;

	case LYK_EDITMAP:
	    handle_LYK_EDITMAP(&old_c, real_c);
	    break;

	case LYK_KEYMAP:
	    handle_LYK_KEYMAP(&vi_keys_flag, &emacs_keys_flag, &old_c, real_c);
	    break;

	case LYK_JUMP:
	    if (handle_LYK_JUMP(c, &user_input_buffer, &temp, &recall,
				&FirstURLRecall, &URLNum, &URLTotal, &ch,
				&old_c, real_c)) {
		if (do_check_recall(ch, &user_input_buffer, &temp, URLTotal,
				    &URLNum, recall, &FirstURLRecall))
		    do_check_goto_URL(&user_input_buffer, &temp, &force_load);
	    }
	    break;

	case LYK_CLEAR_AUTH:
	    handle_LYK_CLEAR_AUTH(&old_c, real_c);
	    break;

	case LYK_DO_NOTHING:	/* pretty self explanatory */
	    break;
#ifdef SUPPORT_CHDIR
	case LYK_CHDIR:
	    handle_LYK_CHDIR();
	    break;
	case LYK_PWD:
	    handle_LYK_PWD();
	    break;
#endif
#ifdef USE_CURSES_PADS
	case LYK_SHIFT_LEFT:
	    handle_LYK_SHIFT_LEFT(&refresh_screen, key_count);
	    break;
	case LYK_SHIFT_RIGHT:
	    handle_LYK_SHIFT_RIGHT(&refresh_screen, key_count);
	    break;
	case LYK_LINEWRAP_TOGGLE:
	    if (handle_LYK_LINEWRAP_TOGGLE(&cmd, &refresh_screen))
		goto new_cmd;
	    break;
#endif

#ifdef USE_MAXSCREEN_TOGGLE
	case LYK_MAXSCREEN_TOGGLE:
	    if (handle_LYK_MAXSCREEN_TOGGLE(&cmd))
		goto new_cmd;
	    break;
#endif
	}			/* end of BIG switch */
    }
}

static int are_different(DocInfo *doc1, DocInfo *doc2)
{
    char *cp1, *cp2;

    /*
     * Do we have two addresses?
     */
    if (!doc1->address || !doc2->address)
	return (TRUE);

    /*
     * Do they differ in the type of request?
     */
    if (doc1->isHEAD != doc2->isHEAD)
	return (TRUE);

    /*
     * See if the addresses are different, making sure we're not tripped up by
     * multiple anchors in the the same document from a POST form.  -- FM
     */
    cp1 = trimPoundSelector(doc1->address);
    cp2 = trimPoundSelector(doc2->address);
    /*
     * Are the base addresses different?
     */
    if (strcmp(doc1->address, doc2->address)) {
	restorePoundSelector(cp1);
	restorePoundSelector(cp2);
	return (TRUE);
    }
    restorePoundSelector(cp1);
    restorePoundSelector(cp2);

    /*
     * Do the docs have different contents?
     */
    if (doc1->post_data) {
	if (doc2->post_data) {
	    if (!BINEQ(doc1->post_data, doc2->post_data))
		return (TRUE);
	} else
	    return (TRUE);
    } else if (doc2->post_data)
	return (TRUE);

    /*
     * We'll assume the two documents in fact are the same.
     */
    return (FALSE);
}

/* This determines whether two docs are _physically_ different,
 * meaning they are "from different files". - kw
 */
static int are_phys_different(DocInfo *doc1, DocInfo *doc2)
{
    char *cp1, *cp2, *ap1 = doc1->address, *ap2 = doc2->address;

    /*
     * Do we have two addresses?
     */
    if (!doc1->address || !doc2->address)
	return (TRUE);

    /*
     * Do they differ in the type of request?
     */
    if (doc1->isHEAD != doc2->isHEAD)
	return (TRUE);

    /*
     * Skip over possible LYNXIMGMAP parts. - kw
     */
    if (isLYNXIMGMAP(doc1->address))
	ap1 += LEN_LYNXIMGMAP;
    if (isLYNXIMGMAP(doc2->address))
	ap2 += LEN_LYNXIMGMAP;
    /*
     * If there isn't any real URL in doc2->address, but maybe just
     * a fragment, doc2 is assumed to be an internal reference in
     * the same physical document, so return FALSE. - kw
     */
    if (*ap2 == '\0' || *ap2 == '#')
	return (FALSE);

    /*
     * See if the addresses are different, making sure we're not tripped up by
     * multiple anchors in the the same document from a POST form.  -- FM
     */
    cp1 = trimPoundSelector(doc1->address);
    cp2 = trimPoundSelector(doc2->address);
    /*
     * Are the base addresses different?
     */
    if (strcmp(ap1, ap2)) {
	restorePoundSelector(cp1);
	restorePoundSelector(cp2);
	return (TRUE);
    }
    restorePoundSelector(cp1);
    restorePoundSelector(cp2);

    /*
     * Do the docs have different contents?
     */
    if (doc1->post_data) {
	if (doc2->post_data) {
	    if (!BINEQ(doc1->post_data, doc2->post_data))
		return (TRUE);
	} else
	    return (TRUE);
    } else if (doc2->post_data)
	return (TRUE);

    /*
     * We'll assume the two documents in fact are the same.
     */
    return (FALSE);
}

/*
 * Utility for freeing the list of goto URLs.  - FM
 */
#ifdef LY_FIND_LEAKS
static void HTGotoURLs_free(void)
{
    LYFreeStringList(Goto_URLs);
    Goto_URLs = NULL;
}
#endif

/*
 * Utility for listing Goto URLs, making any repeated URLs the most current in
 * the list.  - FM
 */
void HTAddGotoURL(char *url)
{
    char *mycopy = NULL;
    char *old;
    HTList *cur;

    if (isEmpty(url))
	return;

    CTRACE((tfp, "HTAddGotoURL %s\n", url));
    StrAllocCopy(mycopy, url);

    if (!Goto_URLs) {
	Goto_URLs = HTList_new();
#ifdef LY_FIND_LEAKS
	atexit(HTGotoURLs_free);
#endif
	HTList_addObject(Goto_URLs, mycopy);
	return;
    }

    cur = Goto_URLs;
    while (NULL != (old = (char *) HTList_nextObject(cur))) {
	if (!strcmp(old, mycopy)) {
	    HTList_removeObject(Goto_URLs, old);
	    FREE(old);
	    break;
	}
    }
    HTList_addObject(Goto_URLs, mycopy);

    return;
}

/*
 * When help is not on the screen, put a message on the screen to tell the user
 * other misc info.
 */
static void show_main_statusline(const LinkInfo curlink,
				 int for_what)
{
    /*
     * Make sure form novice lines are replaced.
     */
    if (user_mode == NOVICE_MODE && for_what != FOR_INPUT) {
	noviceline(more_text);
    }

    if (HTisDocumentSource()) {
	/*
	 * Currently displaying HTML source.
	 */
	_statusline(SOURCE_HELP);

	/*
	 * If we are in forms mode then explicitly tell the user what each kind
	 * of link is.
	 */
#ifdef INDICATE_FORMS_MODE_FOR_ALL_LINKS_ON_PAGE
    } else if (lynx_mode == FORMS_LYNX_MODE && nlinks > 0) {
#else
#ifdef NORMAL_NON_FORM_LINK_STATUSLINES_FOR_ALL_USER_MODES
    } else if (lynx_mode == FORMS_LYNX_MODE && nlinks > 0 &&
	       !(curlink.type & WWW_LINK_TYPE)) {
#else
    } else if (lynx_mode == FORMS_LYNX_MODE && nlinks > 0 &&
	       !(user_mode == ADVANCED_MODE &&
		 (curlink.type & WWW_LINK_TYPE))) {
#endif /* NORMAL_NON_FORM_LINK_STATUSLINES_FOR_ALL_USER_MODES */
#endif /* INDICATE_FORMS_MODE_FOR_ALL_LINKS_ON_PAGE */
	if (curlink.type == WWW_FORM_LINK_TYPE) {
	    show_formlink_statusline(curlink.l_form, for_what);
	} else {
	    statusline(NORMAL_LINK_MESSAGE);
	}

	/*
	 * Let them know if it's an index -- very rare.
	 */
	if (is_www_index) {
	    const char *indx = gettext("-index-");

	    LYmove(LYlines - 1, LYcolLimit - (int) strlen(indx));
	    lynx_start_reverse();
	    LYaddstr(indx);
	    lynx_stop_reverse();
	}

    } else if (user_mode == ADVANCED_MODE && nlinks > 0) {
	/*
	 * Show the URL or, for some internal links, the fragment
	 */
	char *cp = NULL;

	if (curlink.type == WWW_INTERN_LINK_TYPE &&
	    !isLYNXIMGMAP(curlink.lname)) {
	    cp = findPoundSelector(curlink.lname);
	}
	if (!cp)
	    cp = curlink.lname;
	status_link(cp, more_text, is_www_index);
    } else if (is_www_index && more_text) {
	char buf[128];

	sprintf(buf, WWW_INDEX_MORE_MESSAGE, key_for_func(LYK_INDEX_SEARCH));
	_statusline(buf);
    } else if (is_www_index) {
	char buf[128];

	sprintf(buf, WWW_INDEX_MESSAGE, key_for_func(LYK_INDEX_SEARCH));
	_statusline(buf);
    } else if (more_text) {
	if (user_mode == NOVICE_MODE)
	    _statusline(MORE);
	else
	    _statusline(MOREHELP);
    } else {
	_statusline(HELP);
    }

    /* turn off cursor since now it's probably on statusline -HV */
    /* But not if LYShowCursor is on.  -show_cursor may be used as a
     * workaround to avoid putting the cursor in the last position, for
     * curses implementations or terminals that cannot deal with that
     * correctly. - kw */
    if (!LYShowCursor) {
	LYHideCursor();
    }
}

/*
 * Public function for redrawing the statusline appropriate for the selected
 * link.  It should only be called at times when curdoc.link, nlinks, and the
 * links[] array are valid.  - kw
 */
void repaint_main_statusline(int for_what)
{
    if (curdoc.link >= 0 && curdoc.link < nlinks)
	show_main_statusline(links[curdoc.link], for_what);
}

static void form_noviceline(int disabled)
{
    LYmove(LYlines - 2, 0);
    LYclrtoeol();
    if (!disabled) {
	LYaddstr(FORM_NOVICELINE_ONE);
    }
    LYParkCursor();

    if (disabled)
	return;
    if (EditBinding(FROMASCII('\025')) == LYE_ERASE) {
	LYaddstr(FORM_NOVICELINE_TWO);
    } else if (EditBinding(FROMASCII('\025')) == LYE_DELBL) {
	LYaddstr(FORM_NOVICELINE_TWO_DELBL);
    } else {
	char *temp = NULL;
	char *erasekey = fmt_keys(LYKeyForEditAction(LYE_ERASE), -1);

	if (erasekey) {
	    HTSprintf0(&temp, FORM_NOVICELINE_TWO_VAR, erasekey);
	} else {
	    erasekey = fmt_keys(LYKeyForEditAction(LYE_DELBL), -1);
	    if (erasekey)
		HTSprintf0(&temp,
			   FORM_NOVICELINE_TWO_DELBL_VAR, erasekey);
	}
	if (temp) {
	    LYaddstr(temp);
	    FREE(temp);
	}
	FREE(erasekey);
    }
}

static void exit_immediately_with_error_message(int state, int first_file)
{
    char *buf = 0;
    char *buf2 = 0;

    if (first_file) {
	/* print statusline messages as a hint, if any */
	LYstatusline_messages_on_exit(&buf2);
    }

    if (state == NOT_FOUND) {
	HTSprintf0(&buf, "%s\n%s %s\n",
		   NonNull(buf2),
		   gettext("lynx: Can't access startfile"),
	/*
	 * hack: if we fail in HTAccess.c
	 * avoid duplicating URL, oh.
	 */
		   (buf2 && strstr(buf2, gettext("Can't Access"))) ?
		   "" : startfile);
    }

    if (state == NULLFILE) {
	HTSprintf0(&buf, "%s\n%s\n%s\n",
		   NonNull(buf2),
		   gettext("lynx: Start file could not be found or is not text/html or text/plain"),
		   gettext("      Exiting..."));
    }

    FREE(buf2);

    if (!dump_output_immediately)
	cleanup();

    if (buf != 0) {
#ifdef UNIX
	if (dump_output_immediately) {
	    fputs(buf, stderr);
	} else
#endif /* UNIX */
	{
	    SetOutputMode(O_TEXT);
	    fputs(buf, stdout);
	    SetOutputMode(O_BINARY);
	}

	FREE(buf);
    }

    if (!dump_output_immediately) {
	exit_immediately(EXIT_FAILURE);
    }
    /* else: return(EXIT_FAILURE) in mainloop */
}

static void status_link(char *curlink_name,
			int show_more,
			int show_indx)
{
#define MAX_STATUS (LYcolLimit - 1)
#define MIN_STATUS 0
    char format[MAX_LINE];
    int prefix = 0;
    int length;

    *format = 0;
    if (show_more && !nomore) {
	sprintf(format, "%.*s ",
		(int) (sizeof(format) - 2),
		gettext("-more-"));
	prefix = (int) strlen(format);
    }
    if (show_indx) {
	sprintf(format + prefix, "%.*s ",
		((int) sizeof(format) - prefix - 2),
		gettext("-index-"));
    }
    prefix = (int) strlen(format);
    length = (int) strlen(curlink_name);

    if (prefix > MAX_STATUS || prefix >= MAX_LINE - 10) {
	_user_message("%s", format);	/* no room for url */
    } else {
	sprintf(format + prefix, "%%.%ds", MAX_STATUS - prefix);

	if ((length + prefix > MAX_STATUS) && long_url_ok) {
	    char *buf = NULL;
	    int cut_from_pos;
	    int cut_to_pos;
	    int n;

	    StrAllocCopy(buf, curlink_name);
	    /*
	     * Scan to find the final leaf of the URL.  Ignore trailing '/'.
	     */
	    for (cut_to_pos = length - 2;
		 (cut_to_pos > 0) && (buf[cut_to_pos] != '/');
		 cut_to_pos--) ;
	    /*
	     * Jump back to the next leaf to remove.
	     */
	    for (cut_from_pos = cut_to_pos - 4;
		 (cut_from_pos > 0) && ((buf[cut_from_pos] != '/')
					|| ((prefix + cut_from_pos
					     + 4
					     + (length - cut_to_pos)) >= MAX_STATUS));
		 cut_from_pos--) ;
	    /*
	     * Replace some leaves to '...', if possible, and put the final
	     * leaf at the end.  We assume that one can recognize the link from
	     * at least MIN_STATUS characters.
	     */
	    if (cut_from_pos > MIN_STATUS) {
		for (n = 1; n <= 3; n++)
		    buf[cut_from_pos + n] = '.';
		for (n = 0; cut_to_pos + n <= length; n++)
		    buf[cut_from_pos + 4 + n] = buf[cut_to_pos + n];
	    }
	    _user_message(format, buf);
	    CTRACE((tfp, "lastline = %s\n", buf));	/* don't forget to erase me */
	    FREE(buf);
	} else {		/* show (possibly truncated) url */
	    _user_message(format, curlink_name);
	}
    }
}

const char *LYDownLoadAddress(void)
{
    return NonNull(newdoc.address);
}
