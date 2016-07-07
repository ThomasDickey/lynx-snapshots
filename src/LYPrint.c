#include <HTUtils.h>
#include <HTAccess.h>
#include <HTList.h>
#include <HTAlert.h>
#include <HTFile.h>
#include <LYCurses.h>
#include <GridText.h>
#include <LYUtils.h>
#include <LYPrint.h>
#include <LYGlobalDefs.h>
#include <LYSignal.h>
#include <LYStrings.h>
#include <LYClean.h>
#include <LYGetFile.h>
#include <LYHistory.h>
#include <LYList.h>
#include <LYCharSets.h>  /* To get current charset for mail header. */

#include <LYLeaks.h>

#define CancelPrint(msg) HTInfoMsg(msg); goto done
#define CannotPrint(msg) HTAlert(msg); goto done

/*
 *  printfile prints out the current file minus the links and targets
 *  to a variety of places
 */

/* it parses an incoming link that looks like
 *
 *  LYNXPRINT://LOCAL_FILE/lines=##
 *  LYNXPRINT://MAIL_FILE/lines=##
 *  LYNXPRINT://TO_SCREEN/lines=##
 *  LYNXPRINT://LPANSI/lines=##
 *  LYNXPRINT://PRINTER/lines=##/number=#
 */

#define TO_FILE   1
#define TO_SCREEN 2
/*
 * "lpansi.c"
 * Original author: Gary Day (gday@comp.uark.edu), 11/30/93
 * Current version: 2.1 by Noel Hunter (noel@wfu.edu), 10/20/94
 *
 * Basic structure based on print -- format files for printing from
 * _Practical_C_Programming by Steve Oualline, O'Reilly & Associates
 *
 * adapted from the README for lpansi.c v2.1, dated 10/20/1994:
 *		    Print to ANSI printer on local terminal
 *     The VT100 standard defines printer on and off escape sequences,
 *     esc[5i is printer on, and esc[4i is printer off.
 *
 * incorporate the idea of "lpansi" directly into LYPrint.c - HN
 */
#define LPANSI	  3
#define MAIL	  4
#define PRINTER   5

#ifdef VMS
PRIVATE int remove_quotes PARAMS((char *string));
#endif /* VMS */

PRIVATE  char* subject_translate8bit PARAMS((char *source));

#define LYNX_PRINT_TITLE   0
#define LYNX_PRINT_URL     1
#define LYNX_PRINT_DATE    2
#define LYNX_PRINT_LASTMOD 3

#define MAX_PUTENV 4

PRIVATE void set_environ ARGS3(
	int,		name,
	CONST char *,	value,
	CONST char *,	no_value)
{
    static CONST char *names[MAX_PUTENV] = {
	"LYNX_PRINT_TITLE",
	"LYNX_PRINT_URL",
	"LYNX_PRINT_DATE",
	"LYNX_PRINT_LASTMOD",
    };
    static char *pointers[MAX_PUTENV];
    char *envbuffer = 0;
#ifdef VMS
#define SET_ENVIRON(name, value, no_value) set_environ(name, value, no_value)
    char temp[80];
    StrAllocCopy(envbuffer, value);
    if (!(envbuffer && *envbuffer))
	StrAllocCopy(envbuffer, no_value);
    Define_VMSLogical(strcpy(temp, names[name]), envbuffer);
    FREE(envbuffer);
#else
#define SET_ENVIRON(name, value, no_value) set_environ(name, value, "")
    /*
     * Once we've given a string to 'putenv()', we must not free it until we
     * give it a string to replace it.
     */
    StrAllocCopy(envbuffer, names[name]);
    StrAllocCat(envbuffer, "=");
    StrAllocCat(envbuffer, value ? value : no_value);
    putenv(envbuffer);
    FREE(pointers[name]);
    pointers[name] = envbuffer;
#endif
}

PRIVATE char *suggested_filename ARGS1(
	document *,	newdoc)
{
    char *cp, *sug_filename = 0;

    /*
     *	Load the suggested filename string. - FM
     */
    if (HText_getSugFname() != 0)
	StrAllocCopy(sug_filename, HText_getSugFname()); /* must be freed */
    else
	StrAllocCopy(sug_filename, newdoc->address); /* must be freed */
    /*
     *	Strip any gzip or compress suffix, if present. - FM
     */
    cp = NULL;
    if (strlen(sug_filename) > 3) {
	cp = (char *)&sug_filename[(strlen(sug_filename) - 3)];
	if ((*cp == '.' || *cp == '-' || *cp == '_') &&
	    !strcasecomp((cp + 1), "gz")) {
	    *cp = '\0';
	} else {
	    cp = NULL;
	}
    }
    if ((cp == NULL) && strlen(sug_filename) > 2) {
	cp = (char *)&sug_filename[(strlen(sug_filename) - 2)];
	if ((*cp == '.' || *cp == '-' || *cp == '_') &&
	    !strcasecomp((cp + 1), "Z")) {
	    *cp = '\0';
	}
    }
    return sug_filename;
}

PRIVATE void SetupFilename ARGS2(
	char *,		filename,
	char *,		sug_filename)
{
    HTFormat format;
    HTAtom *encoding;
    char *cp;

    LYstrncpy(filename, sug_filename, LY_MAXPATH-1);  /* add suggestion info */
    /* make the sug_filename conform to system specs */
    change_sug_filename(filename);
    if (!(HTisDocumentSource())
     && (cp = strrchr(filename, '.')) != NULL
     && (cp - filename) < LY_MAXPATH-5) {
	format = HTFileFormat(filename, &encoding, NULL);
	if (!strcasecomp(format->name, "text/html") ||
	    !IsUnityEnc(encoding)) {
	    strcpy(cp, ".txt");
	}
    }
}

#define FN_INIT 0
#define FN_READ 1
#define FN_DONE 2
#define FN_QUIT 3

#define PRINT_FLAG   0
#define GENERIC_FLAG 1

PRIVATE int RecallFilename ARGS5(
	char *,		filename,
	BOOLEAN *,	first,
	int *,		now,
	int *,		total,
	int,		flag)
{
    int ch;
    char *cp;
    int recall;

    /*
     * Set up the sug_filenames recall buffer.
     */
    if (*now < 0) {
	*total = (sug_filenames ? HTList_count(sug_filenames) : 0);
	*now = *total;
    }
    recall = ((*total >= 1) ? RECALL : NORECALL);

    if ((ch = LYgetstr(filename, VISIBLE, LY_MAXPATH, recall)) < 0 ||
	*filename == '\0' || ch == UPARROW || ch == DNARROW) {
	if (recall && ch == UPARROW) {
	    if (*first) {
		*first = FALSE;
		/*
		 * Use the last Fname in the list.  - FM
		 */
		*now = 0;
	    } else {
		/*
		 * Go back to the previous Fname in the list.  - FM
		 */
		*now += 1;
	    }
	    if (*now >= *total) {
		/*
		 * Reset the *first flag, and use sug_file or a blank.  -
		 * FM
		 */
		*first = TRUE;
		*now = *total;
		_statusline(FILENAME_PROMPT);
		return FN_INIT;
	    } else if ((cp = (char *)HTList_objectAt(
					    sug_filenames,
					    *now)) != NULL) {
		LYstrncpy(filename, cp, LY_MAXPATH-1);
		if (*total == 1) {
		    _statusline(EDIT_THE_PREV_FILENAME);
		} else {
		    _statusline(EDIT_A_PREV_FILENAME);
		}
		return FN_READ;
	    }
	} else if (recall && ch == DNARROW) {
	    if (*first) {
		*first = FALSE;
		/*
		 * Use the first Fname in the list. - FM
		 */
		*now = *total - 1;
	    } else {
		/*
		 * Advance to the next Fname in the list. - FM
		 */
		*now -= 1;
	    }
	    if (*now < 0) {
		/*
		 * Set the *first flag, and use sug_file or a blank.  - FM
		 */
		*first = TRUE;
		*now = *total;
		_statusline(FILENAME_PROMPT);
		return FN_INIT;
	    } else if ((cp = (char *)HTList_objectAt(
					    sug_filenames,
					    *now)) != NULL) {
		LYstrncpy(filename, cp, LY_MAXPATH-1);
		if (*total == 1) {
		    _statusline(EDIT_THE_PREV_FILENAME);
		} else {
		    _statusline(EDIT_A_PREV_FILENAME);
		}
		return FN_READ;
	    }
	}

	/*
	 * Operation cancelled.
	 */
	if (flag == PRINT_FLAG)
	    HTInfoMsg(SAVE_REQUEST_CANCELLED);
	else if (flag == GENERIC_FLAG)
	    return FN_QUIT;

	return FN_QUIT;
    }
    return FN_DONE;
}

PRIVATE BOOLEAN confirm_by_pages ARGS3(
	char *,		prompt,
	int,		lines_in_file,
	int,		lines_per_page)
{
    int pages = lines_in_file/(lines_per_page+1);
    int c;

    /* count fractional pages ! */
    if ((lines_in_file % (LYlines+1)) > 0)
	pages++;

    if (pages > 4) {
	char *msg = 0;

	HTSprintf0(&msg, prompt, pages);
	c = HTConfirmDefault(msg, YES);
	FREE(msg);

	if (c == YES) {
	    addstr("   Ok...");
	} else {
	    HTInfoMsg(PRINT_REQUEST_CANCELLED);
	    return FALSE;
	}
    }
    return TRUE;
}

PRIVATE void send_file_to_file ARGS3(
	document *,	newdoc,
	char *,		content_base,
	char *,		sug_filename)
{
    BOOLEAN FirstRecall = TRUE;
    BOOLEAN use_cte;
    CONST char *disp_charset;
    FILE *outfile_fp;
    char buffer[LY_MAXPATH];
    char filename[LY_MAXPATH];
    int FnameNum = -1;
    int FnameTotal;
    int c = 0;

    _statusline(FILENAME_PROMPT);
retry:
    SetupFilename(filename, sug_filename);
    if (lynx_save_space
     && (strlen(lynx_save_space) + strlen(filename)) < sizeof(filename)) {
	strcpy(buffer, lynx_save_space);
	strcat(buffer, filename);
	strcpy(filename, buffer);
    }
check_recall:
    switch (RecallFilename(filename,    &FirstRecall, &FnameNum,
			   &FnameTotal, PRINT_FLAG))  {
	case FN_INIT:
	    goto retry;
	case FN_READ:
	    goto check_recall;
	case FN_QUIT:
	    goto done;
	default:
	    break;
    }

    if (!LYValidateFilename(buffer, filename)) {
	CancelPrint(SAVE_REQUEST_CANCELLED);
    }

    /*
     * See if it already exists.
     */
    switch (LYValidateOutput(buffer)) {
    case 'Y':
	break;
    case 'N':
	_statusline(NEW_FILENAME_PROMPT);
	FirstRecall = TRUE;
	FnameNum = FnameTotal;
	goto retry;
    default:
	goto done;
    }

    /*
     * See if we can write to it.
     */
    CTRACE(tfp, "LYPrint: filename is %s, action is `%c'\n", buffer, c);

#if HAVE_POPEN
    if (*buffer == '|') {
	if (no_shell) {
	    HTUserMsg(SPAWNING_DISABLED);
	    FirstRecall = TRUE;
	    FnameNum = FnameTotal;
	    goto retry;
	} else if ((outfile_fp = popen(buffer+1, "w")) == NULL) {
	    CTRACE(tfp, "LYPrint: errno is %d\n", errno);
	    HTAlert(CANNOT_WRITE_TO_FILE);
	    _statusline(NEW_FILENAME_PROMPT);
	    FirstRecall = TRUE;
	    FnameNum = FnameTotal;
	    goto retry;
	}
    } else
#endif
    if ((outfile_fp = (TOUPPER(c) == 'A'
	    ? LYAppendToTxtFile(buffer)
	    : LYNewTxtFile(buffer))) == NULL) {
	CTRACE(tfp, "LYPrint: errno is %d\n", errno);
	HTAlert(CANNOT_WRITE_TO_FILE);
	_statusline(NEW_FILENAME_PROMPT);
	FirstRecall = TRUE;
	FnameNum = FnameTotal;
	goto retry;
    }

    if (LYPrependBaseToSource && HTisDocumentSource()) {
	/*
	 * Added the document's base as a BASE tag to the top of the file.  May
	 * create technically invalid HTML, but will help get any partial or
	 * relative URLs resolved properly if no BASE tag is present to replace
	 * it.  - FM
	 *
	 * Add timestamp (last reload).
	 */

	fprintf(outfile_fp,
		"<!-- X-URL: %s -->\n", newdoc->address);
	if (HText_getDate() != NULL)
	     fprintf(outfile_fp,
		"<!-- Date: %s -->\n", HText_getDate());
	fprintf(outfile_fp,
		"<BASE HREF=\"%s\">\n", content_base);
    }

    if (LYPrependCharsetToSource && HTisDocumentSource()) {
	/*
	 * Added the document's charset as a META CHARSET tag to the top of the
	 * file.  May create technically invalid HTML, but will help to resolve
	 * properly the document converted via chartrans:  printed document
	 * correspond to a display charset and we *should* override both
	 * assume_local_charset and original document's META CHARSET (if any).
	 *
	 * Currently, if several META CHARSETs are found Lynx uses the first
	 * only, and it is opposite to BASE where the original BASE in the
	 * <HEAD> overrides ones from the top.
	 *
	 * As in print-to-email we write charset only if the document has 8-bit
	 * characters, and we have no CJK or an unofficial "x-" charset.
	 */
	use_cte = HTLoadedDocumentEightbit();
	disp_charset = LYCharSet_UC[current_char_set].MIMEname;
	if (!use_cte || LYHaveCJKCharacterSet ||
	    strncasecomp(disp_charset, "x-", 2) == 0) {
	} else {
	    fprintf(outfile_fp,
		    "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=%s\">\n\n",
		    disp_charset);
	}
    }

    print_wwwfile_to_fd(outfile_fp,0);
    if (keypad_mode)
	printlist(outfile_fp,FALSE);

#if HAVE_POPEN
    if (LYIsPipeCommand(buffer))
	pclose(outfile_fp);
    else
#endif
    fclose(outfile_fp);
#ifdef VMS
    if (0 == strncasecomp(buffer, "sys$disk:", 9)) {
	if (0 == strncmp((buffer+9), "[]", 2)) {
	    HTAddSugFilename(buffer+11);
	} else {
	    HTAddSugFilename(buffer+9);
	}
    } else {
	HTAddSugFilename(buffer);
    }
#else
    HTAddSugFilename(buffer);
#endif /* VMS */

done:
    return;
}

PRIVATE void send_file_to_mail ARGS3(
	document *,	newdoc,
	char *,		content_base,
	char *,		content_location)
{
    static BOOLEAN first_mail_preparsed = TRUE;

#ifdef VMS
    BOOLEAN isPMDF = !strncasecomp(system_mail, "PMDF SEND", 9);
    FILE *hfd;
    char hdrfile[LY_MAXPATH];
    char my_temp[LY_MAXPATH];
#endif
#ifdef DOSPATH
    char my_temp[LY_MAXPATH];
#endif

    BOOL use_cte;
    BOOL use_mime;
    BOOL use_type;
    CONST char *disp_charset;
    FILE *outfile_fp;
    char *buffer = NULL;
    char *subject = NULL;
    char user_response[LINESIZE];

    if (LYPreparsedSource && first_mail_preparsed &&
	HTisDocumentSource()) {
	if (HTConfirmDefault(CONFIRM_MAIL_SOURCE_PREPARSED, NO) == YES) {
	    addstr("   Ok...");
	    first_mail_preparsed = FALSE;
	} else	{
	    CancelPrint(MAIL_REQUEST_CANCELLED);
	}
    }

    _statusline(MAIL_ADDRESS_PROMPT);
    LYstrncpy(user_response, personal_mail_address, sizeof(user_response)-1);
    if (LYgetstr(user_response, VISIBLE, sizeof(user_response), NORECALL) < 0 ||
	*user_response == '\0') {
	CancelPrint(MAIL_REQUEST_CANCELLED);
    }

    /*
     * Determine which mail headers should be sent.  Use Content-Type and
     * MIME-Version headers only if needed.  We need them if we are mailing
     * HTML source, or if we have 8-bit characters and will be sending
     * Content-Transfer-Encoding to indicate this.  We will append a charset
     * parameter to the Content-Type if we do not have an "x-" charset, and we
     * will include the Content-Transfer-Encoding only if we are appending the
     * charset parameter, because indicating an 8-bit transfer without also
     * indicating the charset can cause problems with many mailers.  - FM & KW
     */
    disp_charset = LYCharSet_UC[current_char_set].MIMEname;
    use_cte = HTLoadedDocumentEightbit();
    if (!(use_cte && strncasecomp(disp_charset, "x-", 2))) {
	disp_charset = NULL;
	use_cte = FALSE;
    }
    use_type =  (disp_charset || HTisDocumentSource());

    /*
     * Use newdoc->title as a subject instead of sug_filename:  MORE readable
     * and 8-bit letters shouldn't be a problem - LP
     */
    /* change_sug_filename(sug_filename); */
   subject = subject_translate8bit(newdoc->title);

   if (newdoc->isHEAD) {
	   /*
	    * Special case for mailing HEAD responce:  this is rather technical
	    * information, show URL.
	    */
	   FREE(subject);
	   StrAllocCopy(subject, "HEAD  ");
	   StrAllocCat(subject, newdoc->address);
    }

#ifdef VMS
    if (strchr(user_response,'@') && !strchr(user_response,':') &&
       !strchr(user_response,'%') && !strchr(user_response,'"')) {
	char *temp = 0;
	HTSprintf0(&temp, mail_adrs, user_response);
	LYstrncpy(user_response, temp, sizeof(user_response)-1);
	FREE(temp);
    }

    outfile_fp = LYOpenTemp(my_temp,
			    (HTisDocumentSource())
				    ? HTML_SUFFIX
				    : ".txt",
			    "w");
    if (outfile_fp == NULL) {
	CannotPrint(UNABLE_TO_OPEN_TEMPFILE);
    }

    if (isPMDF) {
	if ((hfd = LYOpenTemp(hdrfile, ".txt", "w")) == NULL) {
	    CannotPrint(UNABLE_TO_OPEN_TEMPFILE);
	}
	if (use_type) {
	    fprintf(hfd, "Mime-Version: 1.0\n");
	    if (use_cte) {
		fprintf(hfd, "Content-Transfer-Encoding: 8bit\n");
	    }
	}
	if (HTisDocumentSource()) {
	    /*
	     * Add Content-Type, Content-Location, and Content-Base headers for
	     * HTML source.  - FM
	     */
	    fprintf(hfd, "Content-Type: text/html");
	    if (disp_charset != NULL) {
		fprintf(hfd, "; charset=%s\n", disp_charset);
	    } else {
		fprintf(hfd, "\n");
	    }
	    fprintf(hfd, "Content-Base: %s\n", content_base);
	    fprintf(hfd, "Content-Location: %s\n", content_location);
	} else {
	    /*
	     * Add Content-Type:  text/plain if we have 8-bit characters and a
	     * valid charset for non-source documents.  - FM
	     */
	    if (disp_charset != NULL) {
		fprintf(hfd,
			"Content-Type: text/plain; charset=%s\n",
			disp_charset);
	    }
	}
	/*
	 *	X-URL header. - FM
	 */
	fprintf(hfd, "X-URL: %s\n", newdoc->address);
    }

    /*
     *  Write the contents to a temp file.
     */
    if (LYPrependBaseToSource && HTisDocumentSource()) {
	/*
	 * Added the document's base as a BASE tag to the top of the message
	 * body.  May create technically invalid HTML, but will help get any
	 * partial or relative URLs resolved properly if no BASE tag is present
	 * to replace it.  - FM
	 */
	fprintf(outfile_fp,
		"<!-- X-URL: %s -->\n<BASE HREF=\"%s\">\n\n",
		newdoc->address, content_base);
    } else if (!isPMDF) {
	fprintf(outfile_fp, "X-URL: %s\n\n", newdoc->address);
    }
    print_wwwfile_to_fd(outfile_fp, 0);
    if (keypad_mode)
	printlist(outfile_fp, FALSE);
    LYCloseTempFP(outfile_fp);

    if (isPMDF) {
	/*
	 * For PMDF, put the subject in the header file and close it.  - FM
	 */
	fprintf(hfd, "Subject: %.70s\n\n", subject);
	LYCloseTempFP(hfd);
	/*
	 * Now set up the command.  - FM
	 */
	HTSprintf0(&buffer,
		"%s %s %s,%s %s",
		system_mail,
		system_mail_flags,
		hdrfile,
		my_temp,
		user_response);
    } else {
	/*
	 * For "generic" VMS MAIL, include the subject in the command.  - FM
	 */
	remove_quotes(subject);
	HTSprintf0(&buffer,
		"%s %s/subject=\"%.70s\" %s %s",
		system_mail,
		system_mail_flags,
		subject,
		my_temp,
		user_response);
    }

    stop_curses();
    printf(MAILING_FILE);
    LYSystem(buffer);
    sleep(AlertSecs);
    start_curses();
    if (isPMDF)
	LYRemoveTemp(hdrfile);
    LYRemoveTemp(my_temp);
#else /* Unix or DOS */

#ifdef DOSPATH
    outfile_fp = LYOpenTemp(my_temp, ".txt", "w");
#else
    HTSprintf0(&buffer, "%s %s", system_mail, system_mail_flags);
    outfile_fp = popen(buffer, "w");
#endif
    if (outfile_fp == NULL) {
	CannotPrint(MAIL_REQUEST_FAILED);
    }

    /*
     * Determine which mail headers should be sent.  Use Content-Type and
     * MIME-Version headers only if needed.  We need them if we are mailing
     * HTML source, or if we have 8-bit characters and will be sending
     * Content-Transfer-Encoding to indicate this.
     *
     * Send Content-Transfer-Encoding only if the document has 8-bit
     * characters.  Send a charset parameter only if the document has 8-bit
     * characters and we seem to have a valid charset.  - kw
     */
    use_cte = HTLoadedDocumentEightbit();
    disp_charset = LYCharSet_UC[current_char_set].MIMEname;
    /*
     * Don't send a charset if we have a CJK character set selected, since it
     * may not be appropriate for mail...  Also don't use an unofficial "x-"
     * charset.  - kw
     */
    if (!use_cte || LYHaveCJKCharacterSet ||
	strncasecomp(disp_charset, "x-", 2) == 0) {
	disp_charset = NULL;
    }
#ifdef NOTDEFINED
    /*  Enable this if indicating an 8-bit transfer without
     *  also indicating the charset causes problems. - kw */
    if (use_cte && !disp_charset)
	use_cte = FALSE;
#endif /* NOTDEFINED */
    use_type = (disp_charset || HTisDocumentSource());
    use_mime = (use_cte || use_type);

    if (use_mime) {
	fprintf(outfile_fp, "Mime-Version: 1.0\n");
	if (use_cte) {
	    fprintf(outfile_fp, "Content-Transfer-Encoding: 8bit\n");
	}
    }

    if (HTisDocumentSource()) {
	/*
	 * Add Content-Type, Content-Location, and Content-Base headers for
	 * HTML source.  - FM
	 */
	fprintf(outfile_fp, "Content-Type: text/html");
	if (disp_charset != NULL) {
	    fprintf(outfile_fp, "; charset=%s\n", disp_charset);
	} else {
	    fprintf(outfile_fp, "\n");
	}
    } else {
	/*
	 * Add Content-Type:  text/plain if we have 8-bit characters and a
	 * valid charset for non-source documents.  - KW
	 */
	if (disp_charset != NULL) {
	    fprintf(outfile_fp,
		    "Content-Type: text/plain; charset=%s\n",
		    disp_charset);
	}
    }
    /*
     * If we are using MIME headers, add content-base and content-location if
     * we have them.  This will always be the case if the document is source.
     * - kw
     */
    if (use_mime) {
	if (content_base)
	    fprintf(outfile_fp, "Content-Base: %s\n", content_base);
	if (content_location)
	    fprintf(outfile_fp, "Content-Location: %s\n", content_location);
    }

    /*
     *  Add the To, Subject, and X-URL headers. - FM
     */
    fprintf(outfile_fp, "To: %s\nSubject: %s\n", user_response, subject);
    fprintf(outfile_fp, "X-URL: %s\n\n", newdoc->address);

    if (LYPrependBaseToSource && HTisDocumentSource()) {
	/*
	 * Added the document's base as a BASE tag to the top of the message
	 * body.  May create technically invalid HTML, but will help get any
	 * partial or relative URLs resolved properly if no BASE tag is present
	 * to replace it.  - FM
	 */
	fprintf(outfile_fp,
		"<!-- X-URL: %s -->\n<BASE HREF=\"%s\">\n\n",
		newdoc->address, content_base);
    }
    print_wwwfile_to_fd(outfile_fp, 0);
    if (keypad_mode)
	printlist(outfile_fp, FALSE);

#ifdef DOSPATH
    HTSprintf0(&buffer, "%s -t \"%s\" -F %s", system_mail, user_response, my_temp);
    LYCloseTempFP(outfile_fp);	/* Close the tmpfile. */
    stop_curses();
    printf("%s\n\n$ %s\n\n%s", gettext("Sending"), buffer, PLEASE_WAIT);
    LYSystem(buffer);
    sleep(MessageSecs);
    start_curses();
    LYRemoveTemp(my_temp); /* Delete the tmpfile. */
#else
    pclose(outfile_fp);
#endif
#endif /* VMS */

done:
    FREE(buffer);
    FREE(subject);
    return;
}

PRIVATE void send_file_to_printer ARGS4(
	document *,	newdoc,
	char *,		content_base,
	char *,		sug_filename,
	int,		printer_number)
{
    BOOLEAN FirstRecall = TRUE;
    FILE *outfile_fp;
    char *the_command = 0;
    char my_file[LY_MAXPATH];
    char my_temp[LY_MAXPATH];
    int FnameTotal, FnameNum = -1;
    lynx_printer_item_type *cur_printer;

    outfile_fp = LYOpenTemp(my_temp,
			    (HTisDocumentSource())
				    ? HTML_SUFFIX
				    : ".txt",
			    "w");
    if (outfile_fp == NULL) {
	CannotPrint(FILE_ALLOC_FAILED);
    }

    if (LYPrependBaseToSource && HTisDocumentSource()) {
	/*
	 * Added the document's base as a BASE tag to the top of the file.  May
	 * create technically invalid HTML, but will help get any partial or
	 * relative URLs resolved properly if no BASE tag is present to replace
	 * it.  - FM
	 */
	fprintf(outfile_fp,
		"<!-- X-URL: %s -->\n<BASE HREF=\"%s\">\n\n",
		newdoc->address, content_base);
    }
    print_wwwfile_to_fd(outfile_fp, 0);
    if (keypad_mode)
	printlist(outfile_fp, FALSE);

    LYCloseTempFP(outfile_fp);

    /* find the right printer number */
    {
	int count=0;
	for (cur_printer = printers;
	     count < printer_number;
	     count++, cur_printer = cur_printer->next)
	    ; /* null body */
    }

    /*
     * Commands have the form "command %s [%s] [etc]" where %s is the filename
     * and the second optional %s is the suggested filename.
     */
    if (cur_printer->command == NULL) {
	CannotPrint(PRINTER_MISCONF_ERROR);
    }

    /*
     * Check for two '%s' and ask for the second filename argument if there
     * is.
     */
    if (HTCountCommandArgs (cur_printer->command) >= 2) {
	_statusline(FILENAME_PROMPT);
again:
	SetupFilename(my_file, sug_filename);
check_again:
	switch (RecallFilename(my_file,     &FirstRecall, &FnameNum,
			       &FnameTotal, PRINT_FLAG))  {
	    case FN_INIT:
		goto again;
	    case FN_READ:
		goto check_again;
	    case FN_QUIT:
		goto done;
	    default:
		break;
	}

	if (no_dotfiles || !show_dotfiles) {
	    if (*LYPathLeaf(my_file) == '.') {
		HTAlert(FILENAME_CANNOT_BE_DOT);
		_statusline(NEW_FILENAME_PROMPT);
		FirstRecall = TRUE;
		FnameNum = FnameTotal;
		goto again;
	    }
	}
	/*
	 * Cancel if the user entered "/dev/null" on Unix, or an "nl:" path
	 * (case-insensitive) on VMS.  - FM
	 */
#ifdef VMS
	if (!strncasecomp(my_file, "nl:", 3) ||
	    !strncasecomp(my_file, "/nl/", 4))
#else
	if (!strcmp(my_file, "/dev/null"))
#endif /* VMS */
	{
	    CancelPrint(PRINT_REQUEST_CANCELLED);
	}
	HTAddSugFilename(my_file);
    }

    HTAddParam (&the_command, cur_printer->command, 1, my_temp);
    HTAddParam (&the_command, cur_printer->command, 2, my_file);
    HTEndParam (&the_command, cur_printer->command, 2);

    /*
     * Move the cursor to the top of the screen so that output from system'd
     * commands don't scroll up the screen.
     */
    move(1,1);

    stop_curses();
    CTRACE(tfp, "command: %s\n", the_command);
    printf(PRINTING_FILE);
    /*
     * Set various bits of document information as environment variables, for
     * use by external print scripts/etc.  On UNIX, We assume there are values,
     * and leave NULL value checking up to the external PRINTER:  cmd/script -
     * KED
     */
    SET_ENVIRON(LYNX_PRINT_TITLE,   HText_getTitle(),        "No Title");
    SET_ENVIRON(LYNX_PRINT_URL,     newdoc->address,         "No URL");
    SET_ENVIRON(LYNX_PRINT_DATE,    HText_getDate(),         "No Date");
    SET_ENVIRON(LYNX_PRINT_LASTMOD, HText_getLastModified(), "No LastMod");

    LYSystem(the_command);
    FREE(the_command);
    LYRemoveTemp(my_temp);

    /*
     * Remove the various LYNX_PRINT_xxxx logicals.  - KED
     * [could use unsetenv(), but it's not portable]
     */
    SET_ENVIRON(LYNX_PRINT_TITLE,   "", "");
    SET_ENVIRON(LYNX_PRINT_URL,     "", "");
    SET_ENVIRON(LYNX_PRINT_DATE,    "", "");
    SET_ENVIRON(LYNX_PRINT_LASTMOD, "", "");

    fflush(stdout);
#ifndef VMS
    signal(SIGINT, cleanup_sig);
#endif /* !VMS */
    sleep(MessageSecs);
    start_curses();

done:
    return;
}

PRIVATE void send_file_to_screen ARGS3(
	document *,	newdoc,
	char *,		content_base,
	BOOLEAN,	Lpansi)
{
    FILE *outfile_fp;
    char prompt[80];

    if (Lpansi) {
	_statusline(CHECK_PRINTER);
    } else {
	_statusline(PRESS_RETURN_TO_BEGIN);
    }

    *prompt = '\0';
    if (LYgetstr(prompt, VISIBLE, sizeof(prompt), NORECALL) < 0) {
	CancelPrint(PRINT_REQUEST_CANCELLED);
    }

    outfile_fp = stdout;

    stop_curses();
#ifndef VMS
    signal(SIGINT, SIG_IGN);
#endif /* !VMS */

    if (LYPrependBaseToSource && HTisDocumentSource()) {
	/*
	 * Added the document's base as a BASE tag to the top of the file.  May
	 * create technically invalid HTML, but will help get any partial or
	 * relative URLs resolved properly if no BASE tag is present to replace
	 * it.  - FM
	 */
	fprintf(outfile_fp,
		"<!-- X-URL: %s -->\n<BASE HREF=\"%s\">\n\n",
		newdoc->address, content_base);
    }
    if (Lpansi)
	printf("\033[5i");
    print_wwwfile_to_fd(outfile_fp, 0);
    if (keypad_mode)
	printlist(outfile_fp, FALSE);

#ifdef VMS
    if (HadVMSInterrupt) {
	HadVMSInterrupt = FALSE;
	start_curses();
	CancelPrint(PRINT_REQUEST_CANCELLED);
    }
#endif /* VMS */
    if (Lpansi) {
	printf("\n\014");	/* Form feed */
	printf("\033[4i");
	Lpansi = FALSE;
    } else {
	fprintf(stdout,"\n\n%s", PRESS_RETURN_TO_FINISH);
	LYgetch();  /* grab some user input to pause */
#ifdef VMS
	HadVMSInterrupt = FALSE;
#endif /* VMS */
    }
    fflush(stdout);  /* refresh to screen */
    start_curses();

done:
    return;
}

PUBLIC int printfile ARGS1(
	document *,	newdoc)
{
    BOOLEAN Lpansi = FALSE;
    DocAddress WWWDoc;
    char *content_base = NULL;
    char *content_location = NULL;
    char *cp = NULL;
    char *link_info = NULL;
    char *sug_filename = NULL;
    int lines_in_file = 0;
    int pagelen = 0;
    int printer_number = 0;
    int type = 0;

    /*
     *	Extract useful info from URL.
     */
    StrAllocCopy(link_info, newdoc->address+12);

    /*
     *	Reload the file we want to print into memory.
     */
    LYpop(newdoc);
    WWWDoc.address = newdoc->address;
    WWWDoc.post_data = newdoc->post_data;
    WWWDoc.post_content_type = newdoc->post_content_type;
    WWWDoc.bookmark = newdoc->bookmark;
    WWWDoc.isHEAD = newdoc->isHEAD;
    WWWDoc.safe = newdoc->safe;
    if (!HTLoadAbsolute(&WWWDoc))
	return(NOT_FOUND);

    /*
     *	If we have an explicit content-base, we may use it even
     *	if not in source mode. - kw
     */
    if (HText_getContentBase()) {
	StrAllocCopy(content_base, HText_getContentBase());
	LYRemoveBlanks(content_base);
	if (!(content_base && *content_base)) {
	    FREE(content_base);
	}
    }
    /*
     *	If document is source, load the content_base
     *	and content_location strings. - FM
     */
    if (HTisDocumentSource()) {
	if (HText_getContentLocation()) {
	    StrAllocCopy(content_location, HText_getContentLocation());
	    LYRemoveBlanks(content_location);
	    if (!(content_location && *content_location)) {
		FREE(content_location);
	    }
	}
	if (!content_base) {
	    if ((content_location) && is_url(content_location)) {
		StrAllocCopy(content_base, content_location);
	    } else {
		StrAllocCopy(content_base, newdoc->address);
	    }
	}
	if (!content_location) {
	    StrAllocCopy(content_location, newdoc->address);
	}
    }

    sug_filename = suggested_filename(newdoc);

    /*
     *	Get the number of lines in the file.
     */
    if ((cp = (char *)strstr(link_info, "lines=")) != NULL) {
	/*
	 *  Terminate prev string here.
	 */
	*cp = '\0';
	/*
	 *  Number of characters in "lines=".
	 */
	cp += 6;

	lines_in_file = atoi(cp);
    }

    /*
     *	Determine the type.
     */
    if (strstr(link_info, "LOCAL_FILE")) {
	type = TO_FILE;
    } else if (strstr(link_info, "TO_SCREEN")) {
	type = TO_SCREEN;
    } else if (strstr(link_info, "LPANSI")) {
	Lpansi = TRUE;
	type = TO_SCREEN;
    } else if (strstr(link_info, "MAIL_FILE")) {
	type = MAIL;
    } else if (strstr(link_info, "PRINTER")) {
	type = PRINTER;

	if ((cp = (char *)strstr(link_info, "number=")) != NULL) {
	    /* number of characters in "number=" */
	    cp += 7;
	    printer_number = atoi(cp);
	}
	if ((cp = (char *)strstr(link_info, "pagelen=")) != NULL) {
	    /* number of characters in "pagelen=" */
	    cp += 8;
	    pagelen = atoi(cp);
	} else {
	    /* default to 66 lines */
	    pagelen = 66;
	}
    }

    /*
     *	Act on the request. - FM
     */
    switch (type) {

	case TO_FILE:
	    send_file_to_file(newdoc, content_base, sug_filename);
	    break;

	case MAIL:
	    send_file_to_mail(newdoc, content_base, content_location);
	    break;

	case TO_SCREEN:
	    if (confirm_by_pages(CONFIRM_LONG_SCREEN_PRINT, lines_in_file, LYlines))
		send_file_to_screen(newdoc, content_base, Lpansi);
	    break;

	case PRINTER:
	    if (confirm_by_pages(CONFIRM_LONG_PAGE_PRINT, lines_in_file, pagelen))
		send_file_to_printer(newdoc, content_base, sug_filename, printer_number);
	    break;

    } /* end switch */

    FREE(link_info);
    FREE(sug_filename);
    FREE(content_base);
    FREE(content_location);
    return(NORMAL);
}

#ifdef VMS
PRIVATE int remove_quotes ARGS1(
	char *,		string)
{
   int i;

   for(i = 0; string[i] != '\0'; i++)
	if(string[i] == '"')
	   string[i] = ' ';
	else if(string[i] == '&')
	   string[i] = ' ';
	else if(string[i] == '|')
	   string[i] = ' ';

   return(0);
}
#endif /* VMS */

/*
 *  Mail subject may have 8-bit characters and they are in display charset.
 *  There is no stable practice for 8-bit subject encodings:
 *  MIME define "quoted-printable" which holds charset info
 *  but most mailers still don't support it, on the other hand
 *  many mailers send open 8-bit subjects without charset info
 *  and use local assumption for certain countries.  Besides that,
 *  obsolete SMTP software is not 8bit clean but still in use,
 *  it strips the characters in 128-160 range from subjects
 *  which may be a fault outside iso-8859-XX.
 *
 *  We translate subject to "outgoing_mail_charset" (defined in lynx.cfg)
 *  it may correspond to US-ASCII as the safest value or any other
 *  lynx character handler, -1 for no translation (so display charset).
 *
 *  Always returns a new allocated string which has to be freed.
 */
PRIVATE char* subject_translate8bit ARGS1(char *, source)
{
    CONST char *p = source;
    char temp[2];
    char *target = NULL;

    int charset_in, charset_out;
    char replace_buf [10];

    int i = outgoing_mail_charset;  /* from lynx.cfg, -1 by default */

    if (i < 0
     || i == current_char_set
     || LYCharSet_UC[current_char_set].enc == UCT_ENC_CJK
     || LYCharSet_UC[i].enc == UCT_ENC_CJK) {
	StrAllocCopy(target, source);
	return(target); /* OK */
    } else {
	charset_out = i;
	charset_in  = current_char_set;
    }

    for ( ; *p; p++) {
	LYstrncpy(temp, p, sizeof(temp)-1);
	if ((unsigned char)*temp <= 127) {
	    StrAllocCat(target, temp);
	} else {
	    if (UCTransCharStr(replace_buf, sizeof(replace_buf), *temp,
				charset_in, charset_out, YES) > 0)
		StrAllocCat(target, replace_buf);
	}
    }

    return(target);
}

/*
 * print_options writes out the current printer choices to a file
 * so that the user can select printers in the same way that
 * they select all other links
 * printer links look like
 *  LYNXPRINT://LOCAL_FILE/lines=#	     print to a local file
 *  LYNXPRINT://TO_SCREEN/lines=#	     print to the screen
 *  LYNXPRINT://LPANSI/lines=#		     print to the local terminal
 *  LYNXPRINT://MAIL_FILE/lines=#	     mail the file
 *  LYNXPRINT://PRINTER/lines=#/number=#   print to printer number #
 */
PUBLIC int print_options ARGS3(
	char **,	newfile,
	char **,	printed_url,
	int,		lines_in_file)
{
    static char my_temp[LY_MAXPATH];
    char *buffer = 0;
    int count;
    int pages;
    FILE *fp0;
    lynx_printer_item_type *cur_printer;

    LYRemoveTemp(my_temp);
    if ((fp0 = LYOpenTemp(my_temp, HTML_SUFFIX, "w")) == NULL) {
	HTAlert(UNABLE_TO_OPEN_PRINTOP_FILE);
	return(-1);
    }

    LYLocalFileToURL(newfile, my_temp);

    BeginInternalPage(fp0, PRINT_OPTIONS_TITLE, PRINT_OPTIONS_HELP);

    fprintf(fp0, "<pre>\n");

    /*  pages = lines_in_file/66 + 1; */
    pages = (lines_in_file+65)/66;
    HTSprintf0(&buffer, "   <em>%s</em> %s\n   <em>%s</em> %d\n   <em>%s</em> %d %s %s\n",
	    gettext("Document:"), *printed_url,
	    gettext("Number of lines:"), lines_in_file,
	    gettext("Number of pages:"), pages,
	    (pages > 1 ? gettext("pages") : gettext("page")),
	    gettext("(approximately)"));
    fputs(buffer, fp0);
    FREE(buffer);

    if (no_print || no_disk_save || child_lynx || no_mail)
	fprintf(fp0, "   <em>%s</em>\n", gettext("Some print functions have been disabled!"));

    fprintf(fp0, "\n%s\n",
	    (user_mode == NOVICE_MODE)
	    ? gettext("Standard print options:")
	    : gettext("Print options:"));

    if (child_lynx == FALSE && no_disk_save == FALSE && no_print == FALSE) {
	fprintf(fp0,
		"   <a href=\"LYNXPRINT://LOCAL_FILE/lines=%d\">%s</a>\n",
		lines_in_file,
		gettext("Save to a local file"));
    } else {
	fprintf(fp0,"   <em>%s</em>\n", gettext("Save to disk disabled"));
    }
    if (child_lynx == FALSE && no_mail == FALSE && local_host_only == FALSE)
	fprintf(fp0,
		"   <a href=\"LYNXPRINT://MAIL_FILE/lines=%d\">%s</a>\n",
		lines_in_file,
		gettext("Mail the file"));

#ifndef DOSPATH
    fprintf(fp0,
	    "   <a href=\"LYNXPRINT://TO_SCREEN/lines=%d\">%s</a>\n",
	    lines_in_file,
	    gettext("Print to the screen"));
    fprintf(fp0,
	    "   <a href=\"LYNXPRINT://LPANSI/lines=%d\">%s</a>\n",
	    lines_in_file,
	    gettext("Print out on a printer attached to your vt100 terminal"));
#endif

    if (user_mode == NOVICE_MODE)
	fprintf(fp0, "\n%s\n", gettext("Local additions:"));

    for (count = 0, cur_printer = printers; cur_printer != NULL;
	cur_printer = cur_printer->next, count++)
    if (no_print == FALSE || cur_printer->always_enabled) {
	fprintf(fp0,
		"   <a href=\"LYNXPRINT://PRINTER/number=%d/pagelen=%d/lines=%d\">",
		count, cur_printer->pagelen, lines_in_file);
	fprintf(fp0, (cur_printer->name ?
		      cur_printer->name : "No Name Given"));
	fprintf(fp0, "</a>\n");
    }
    fprintf(fp0, "</pre>\n");
    EndInternalPage(fp0);
    LYCloseTempFP(fp0);

    LYforce_no_cache = TRUE;
    return(0);
}


/*
 *  General purpose filename getter.
 *
 *  Returns a pointer to an absolute filename string, if the input
 *  filename exists, and is readable.  Returns NULL if the input
 *  was cancelled (^G, or CR on empty input).
 *
 *  The pointer to the filename string needs to be free()'d by the
 *  caller (when non-NULL).
 *
 *  --KED  02/21/99
 */
PUBLIC char * GetFileName NOARGS
{
    struct stat stat_info;

    FILE *fp;

    char  fbuf[LY_MAXPATH];
    char  tbuf[LY_MAXPATH];
    char *fn;

    BOOLEAN FirstRecall = TRUE;
    int     FnameNum    = -1;
    int     FnameTotal;


    _statusline(FILENAME_PROMPT);

retry:
    /*
     *  No initial filename.
     */
    SetupFilename (fbuf, "");

check_recall:
    /*
     *  Go get a filename (it would be nice to do TAB == filename-completion
     *  as the name is entered, but we'll save doing that for another time.
     */
    switch (RecallFilename (fbuf,        &FirstRecall,  &FnameNum,
			    &FnameTotal, GENERIC_FLAG)) {
	case FN_INIT:
	    goto retry;
	case FN_READ:
	    goto check_recall;
	case FN_QUIT:
	    goto quit;
	default:
	    break;
    }

    /*
     *  Add raw input form to list ... we may want to reuse/edit it on a
     *  subsequent call, etc.
     */
#ifdef VMS
    if (0 == strncasecomp (fbuf, "sys$disk:", 9)) {
	if (0 == strncmp ((fbuf+9), "[]", 2)) {
	    HTAddSugFilename (fbuf+11);
	} else {
	    HTAddSugFilename (fbuf+9);
	}
    } else {
	HTAddSugFilename (fbuf);
    }
#else
    HTAddSugFilename (fbuf);
#endif /* VMS */

    /*
     *  Expand tilde's, make filename absolute, etc.
     */
    if (!LYValidateFilename (tbuf, fbuf))
	goto quit;

    /*
     *  Check for file existence; readability.
     */
    if ((stat (tbuf, &stat_info) < 0) ||
       (!(S_ISREG(stat_info.st_mode)
#ifdef S_IFLNK
	 || S_ISLNK(stat_info.st_mode)
#endif /* S_IFLNK */
	))) {
	HTInfoMsg (FILE_DOES_NOT_EXIST);
	_statusline(FILE_DOES_NOT_EXIST_RE);
	FirstRecall = TRUE;
	FnameNum    = FnameTotal;
	goto retry;
    }

    if ((fp = fopen (tbuf, "r")) == NULL) {
	HTInfoMsg (FILE_NOT_READABLE);
	_statusline(FILE_NOT_READABLE_RE);
	FirstRecall = TRUE;
	FnameNum    = FnameTotal;
	goto retry;
    } else {
	fclose (fp);
    }

    /*
     *  We have a valid filename, and readable file.  Return it to the
     *  caller.
     *
     *  The returned pointer should be free()'d by the caller.
     *
     *  [For some silly reason, if we use StrAllocCopy() here, we get an
     *   "invalid pointer" reported in the Lynx.leaks file (if compiled
     *   with  --enable-find-leaks  turned on.  Dumb.]
     */
    if ((fn = (char *) calloc (1, (strlen (tbuf) + 1))) == NULL)
	outofmem(__FILE__, "GetFileName");
    return (strcpy (fn, tbuf));


quit:
    /*
     *  The user cancelled the input (^G, or CR on empty input field).
     */
    return (NULL);
}
