/* $LynxId: LYEdit.c,v 1.42 2013/11/28 11:18:19 tom Exp $ */
#include <HTUtils.h>
#include <HTParse.h>
#include <HTAlert.h>
#include <LYCurses.h>
#include <LYUtils.h>
#include <LYGlobalDefs.h>
#include <LYStrings.h>
#include <LYEdit.h>
#ifdef VMS
#include <unixio.h>
#endif /* VMS */

#include <LYLeaks.h>
#include <www_wait.h>

BOOLEAN editor_can_position(void)
{
    char *value;
    HTList *p = positionable_editor;
    static const char *table[] =
    {
#ifdef VMS
	"sedt",
#else
	"emacs",		/* + xemacs */
	"jed",
	"jmacs",
	"joe",			/* + rjoe */
	"jove",
	"jstar",
	"nano",
	"pico",			/* + jpico */
	"vi"			/* + vim, xvi, vile, elvis, view... + likely false matches */
#endif
    };
    unsigned n;

    for (n = 0; n < TABLESIZE(table); n++) {
	if (LYstrstr(editor, table[n]) != 0) {
	    return TRUE;
	}
    }
    /*
     * This really isn't right.  LYstrstr() might be too lax,
     * but this should at least match basename to basename...
     */
    if (positionable_editor != NULL) {
	while ((value = (char *) HTList_nextObject(p)) != NULL) {
	    if (strcmp(editor, value) == 0) {
		return TRUE;
	    }
	}
    }
    return FALSE;
}

/*
 * In edit mode invoke the given (or default) editor to display and edit the
 * current file.  For editors listed in 'editor_can_position()', Lynx will open
 * the file to the same line that the screen cursor is on (or close...) when
 * editing is invoked.
 *
 * Returns FALSE if file is uneditable.
 */
int edit_current_file(char *newfile,
		      int cur,
		      int lineno)
{
    int result = FALSE;
    char *filename = NULL;

#if !(defined(VMS) || defined(USE_DOS_DRIVES))
    char *colon;
#endif
    char *number_sign;
    char position[80];

#if defined(VMS) || defined(CANT_EDIT_UNWRITABLE_FILES)
    FILE *fp;
#endif

    CTRACE((tfp, "edit_current_file(newfile=%s, cur=%d, lineno=%d)\n",
	    newfile, cur, lineno));

    /*
     * If it's a remote file then we can't edit it.
     */
    if (!LYisLocalFile(newfile)) {
	HTUserMsg(CANNOT_EDIT_REMOTE_FILES);
	return FALSE;
    }

    /*
     * If there's a fragment, trim it.  - FM
     */
    number_sign = trimPoundSelector(newfile);

    /*
     * On Unix, first try to open it as a completely referenced file, then via
     * the path alone.
     *
     * On VMS, only try the path.
     */
#if defined (VMS) || defined (USE_DOS_DRIVES)
    filename = HTParse(newfile, "", PARSE_PATH + PARSE_PUNCTUATION);
    HTUnEscape(filename);
    StrAllocCopy(filename, HTSYS_name(filename));
    if (!LYCanReadFile(filename)) {
#ifdef SH_EX
	HTUserMsg2(COULD_NOT_EDIT_FILE, filename);
#else
	HTAlert(COULD_NOT_ACCESS_FILE);
#endif
	CTRACE((tfp, "filename: '%s'\n", filename));
	goto done;
    }
#else /* something like UNIX */
    if (StrNCmp(newfile, "file://localhost/", 16) == 0)
	colon = newfile + 16;
    else
	colon = StrChr(newfile, ':');
    StrAllocCopy(filename, (colon + 1));
    HTUnEscape(filename);
    if (!LYCanReadFile(filename)) {
	FREE(filename);
	filename = HTParse(newfile, "", PARSE_PATH + PARSE_PUNCTUATION);
	HTUnEscape(filename);
	if (!LYCanReadFile(HTSYS_name(filename))) {
	    HTAlert(COULD_NOT_ACCESS_FILE);
	    goto done;
	}
    }
#endif

#if defined(VMS) || defined(CANT_EDIT_UNWRITABLE_FILES)
    /*
     * Don't allow editing if user lacks append access.
     */
    if ((fp = fopen(filename, TXT_A)) == NULL) {
	HTUserMsg(NOAUTH_TO_EDIT_FILE);
	goto done;
    }
    fclose(fp);
#endif /* VMS || CANT_EDIT_UNWRITABLE_FILES */

    /*
     * Make sure cur is at least zero.  - FM
     */
    if (cur < 0) {
	cur = 0;
    }

    /*
     * Set up the command for the editor.  - FM
     */
    if (lineno >= 0) {
	*position = 0;
#ifdef VMS
	lineno--;
#endif
	lineno += (nlinks ? links[cur].ly : 0);
	if (lineno > 0)
	    sprintf(position, "%d", lineno);
    } else {
	*position = '\0';
    }

    edit_temporary_file(filename, position, NULL);
    result = TRUE;

  done:
    /*
     * Restore the fragment if there was one.  - FM
     */
    restorePoundSelector(number_sign);

    FREE(filename);
    CTRACE((tfp, "edit_current_file returns %d\n", result));
    return (result);
}

void edit_temporary_file(char *filename,
			 const char *position,
			 const char *message)
{
#ifdef UNIX
    struct stat stat_info;
#endif
    const char *format = "%s %s";
    char *command = NULL;
    const char *editor_arg = "";
    int params = 1;
    int rv;

    if (LYstrstr(editor, "pico")) {
	editor_arg = " -t";	/* No prompt for filename to use */
    }
    if (editor_can_position() && *position) {
#ifdef VMS
	format = "%s %s -%s%s";
	HTAddXpand(&command, format, params++, editor);
	HTAddParam(&command, format, params++, filename);
	HTAddParam(&command, format, params++, position);
	HTAddParam(&command, format, params++, editor_arg);
	HTEndParam(&command, format, params);
#else
	format = "%s +%s%s %s";
	HTAddXpand(&command, format, params++, editor);
	HTAddParam(&command, format, params++, position);
	HTAddParam(&command, format, params++, editor_arg);
	HTAddParam(&command, format, params++, filename);
	HTEndParam(&command, format, params);
#endif
    }
#ifdef DOSPATH
    else if (StrNCmp(editor, "VZ", 2) == 0) {
	/* for Vz editor */
	format = "%s %s -%s";
	HTAddXpand(&command, format, params++, editor);
	HTAddParam(&command, format, params++, HTDOS_short_name(filename));
	HTAddParam(&command, format, params++, position);
	HTEndParam(&command, format, params);
    } else if (StrNCmp(editor, "edit", 4) == 0) {
	/* for standard editor */
	HTAddXpand(&command, format, params++, editor);
	HTAddParam(&command, format, params++, HTDOS_short_name(filename));
	HTEndParam(&command, format, params);
    }
#endif
    else {
#ifdef _WINDOWS
	if (StrChr(editor, ' '))
	    HTAddXpand(&command, format, params++, HTDOS_short_name(editor));
	else
	    HTAddXpand(&command, format, params++, editor);
#else
	HTAddXpand(&command, format, params++, editor);
#endif
	HTAddParam(&command, format, params++, filename);
	HTEndParam(&command, format, params);
    }
    if (message != NULL) {
	_statusline(message);
    }

    CTRACE((tfp, "LYEdit: %s\n", command));
    CTRACE_SLEEP(MessageSecs);

    stop_curses();

#ifdef UNIX
    set_errno(0);
#endif
    if ((rv = LYSystem(command)) != 0) {	/* Spawn Editor */
	start_curses();
	/*
	 * If something went wrong, we should probably return soon; currently
	 * we don't, but at least put out a message.  - kw
	 */
	{
#if defined(UNIX) && defined(WIFEXITED)
	    int save_err = errno;

	    CTRACE((tfp, "ExtEditForm: system() returned %d (0x%x), %s\n",
		    rv, rv,
		    (save_err
		     ? LYStrerror(save_err)
		     : "reason unknown")));
	    LYFixCursesOn("show error warning:");
	    if (rv == -1) {
		HTUserMsg2(gettext("Error starting editor, %s"),
			   LYStrerror(save_err));
	    } else if (WIFSIGNALED(rv)) {
		HTAlwaysAlert(NULL, gettext("Editor killed by signal"));
	    } else if (WIFEXITED(rv) && WEXITSTATUS(rv) != 127) {
		char exitcode[80];

		sprintf(exitcode, "%d", WEXITSTATUS(rv));
		HTUserMsg2(gettext("Editor returned with error status %s"),
			   exitcode);
	    } else
#endif
		HTAlwaysAlert(NULL, ERROR_SPAWNING_EDITOR);
	}
    } else {
	start_curses();
    }
#ifdef UNIX
    /*
     * Delete backup file, if that's your style.
     */
    HTSprintf0(&command, "%s~", filename);
    if (stat(command, &stat_info) == 0)
	remove(command);
#endif
    FREE(command);
}
