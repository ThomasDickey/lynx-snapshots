#include <HTUtils.h>
#include <HTParse.h>
#include <HTAlert.h>
#include <LYCurses.h>
#include <LYSignal.h>
#include <LYUtils.h>
#include <LYClean.h>
#include <LYGlobalDefs.h>
#include <LYEdit.h>
#include <LYStrings.h>
#include <LYUtils.h>
#ifdef VMS
#include <unixio.h>
#endif /* VMS */

#include <LYLeaks.h>

PRIVATE BOOLEAN editor_can_position NOARGS
{
#ifdef VMS
    return (strstr(editor, "sedt") || strstr(editor, "SEDT"));
#else
    return (strstr(editor, "emacs") || strstr(editor, "vi") ||
	strstr(editor, "pico") || strstr(editor, "jove") ||
	strstr(editor, "jed"));
#endif
}

/*
 *  In edit mode invoke either emacs, vi, pico, jove, jed sedt or the
 *  default editor to display and edit the current file.
 *  For emacs, vi, pico, jove and jed, Lynx will open the file to the
 *  same line that the screen cursor is on when editing is invoked.
 *  Returns FALSE if file is uneditable.
 */
PUBLIC int edit_current_file ARGS3(
	char *,		newfile,
	int,		cur,
	int,		lineno)
{
    int result = FALSE;
    int params = 1;
    char *format = "%s %s";
    char *command = NULL;
    char *filename = NULL;
    char *colon, *number_sign;
    char position[80];
    FILE *fp;

    /*
     *  If its a remote file then we can't edit it.
     */
    if (!LYisLocalFile(newfile)) {
	HTUserMsg(CANNOT_EDIT_REMOTE_FILES);
	return FALSE;
    }

    /*
     *  If there's a fragment, trim it. - FM
     */
    number_sign = strchr(newfile, '#');
    if (number_sign)
	*number_sign = '\0';

    /*
     *  On Unix, first try to open it as a completely referenced file,
     *  then via the path alone.
     *
     * On VMS, only try the path.
     */
#if !defined (VMS) && !defined (DOSPATH) && !defined (__EMX__)
    colon = strchr(newfile, ':');
    StrAllocCopy(filename, (colon + 1));
    HTUnEscape(filename);
    if ((fp = fopen(filename, "r")) == NULL) {
	FREE(filename);
#endif /* !VMS */
	filename = HTParse(newfile, "", PARSE_PATH+PARSE_PUNCTUATION);
	HTUnEscape(filename);
#if defined (DOSPATH) || defined (__EMX__)
	if (strlen(filename) > 1) {	/* FIXME: why do we need to do this? */
	    int n;
	    for (n = 0; (filename[n] = filename[n+1]) != 0; n++)
		;
	}
#endif
	if ((fp = fopen(HTSYS_name(filename), "r")) == NULL)
	{
	    HTAlert(COULD_NOT_ACCESS_FILE);
	    goto done;
	}
#if !defined (VMS) && !defined (DOSPATH) && !defined (__EMX__)
    }
#endif /* !VMS */
    fclose(fp);

#if defined(VMS) || defined(CANT_EDIT_UNWRITABLE_FILES)
    /*
     *  Don't allow editing if user lacks append access.
     */
    if ((fp = fopen(HTSYS_name(filename), "a")) == NULL)
    {
	HTUserMsg(NOAUTH_TO_EDIT_FILE);
	goto done;
    }
    fclose(fp);
#endif /* VMS || CANT_EDIT_UNWRITABLE_FILES */

    /*
     *  Make sure cur is at least zero. - FM
     */
    if (cur < 0) {
	cur = 0;
    }

    /*
     *  Set up the command for the editor. - FM
     */
    *position = 0;
#ifdef VMS
    lineno--;
#endif
    lineno += (nlinks ? links[cur].ly : 0);
    if (lineno > 0)
	sprintf(position, "%d", lineno);

    if (editor_can_position() && *position) {
#ifdef VMS
	format = "%s %s -%s";
	HTAddParam(&command, format, params++, editor);
	HTAddParam(&command, format, params++, HTVMS_name("", filename));
	HTAddParam(&command, format, params++, position);
	HTEndParam(&command, format, params);
#else
	format = "%s +%s %s";
	HTAddParam(&command, format, params++, editor);
	HTAddParam(&command, format, params++, position);
	HTAddParam(&command, format, params++, HTSYS_name(filename));
	HTEndParam(&command, format, params);
#endif
    } else {
	HTAddParam(&command, format, params++, editor);
	HTAddParam(&command, format, params++, HTSYS_name(filename));
	HTEndParam(&command, format, params);
    }

    CTRACE(tfp, "LYEdit: %s\n", command);
    CTRACE_SLEEP(MessageSecs);

    /*
     *  Invoke the editor. - FM
     */
    stop_curses();
    LYSystem(command);
    start_curses();

    result = TRUE;

done:
    /*
     *  Restore the fragment if there was one. - FM
     */
    if (number_sign)
	*number_sign = '#';

    FREE(command);
    FREE(filename);
    return (result);
}
