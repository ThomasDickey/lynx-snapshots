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
    char command[512];
    char *filename = NULL;
    char *colon, *number_sign;
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
	if (strlen(filename)>1) filename++;
#endif
	if ((fp = fopen(HTSYS_name(filename), "r")) == NULL)
	{
	    HTAlert(COULD_NOT_ACCESS_FILE);
	    FREE(filename);
	    goto failure;
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
	goto failure;
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
#ifdef VMS
    if ((strstr(editor, "sedt") || strstr(editor, "SEDT")) &&
	((lineno - 1) + (nlinks ? links[cur].ly : 0)) > 0) {
	sprintf(command, "%s %s -%d",
			 editor,
			 HTVMS_name("", filename),
			 ((lineno - 1) + (nlinks ? links[cur].ly : 0)));
    } else {
	sprintf(command, "%s %s", editor, HTVMS_name("", filename));
    }
#else
    if (strstr(editor, "emacs") || strstr(editor, "vi") ||
	strstr(editor, "pico") || strstr(editor, "jove") ||
	strstr(editor, "jed"))
	sprintf(command, "%s +%d \"%s\"",
			 editor,
			 (lineno + (nlinks ? links[cur].ly : 0)),
			 HTSYS_name(filename));
    else
#ifdef __DJGPP__
	sprintf(command, "%s %s", editor, HTDOS_name(filename));
#else
	sprintf(command, "%s \"%s\"", editor, HTSYS_name(filename));
#endif /* __DJGPP__ */
#endif /* VMS */
    CTRACE(tfp, "LYEdit: %s\n", command);
    CTRACE_SLEEP(MessageSecs);
#ifndef __EMX__
    FREE(filename);
#endif

    /*
     *  Invoke the editor. - FM
     */
    stop_curses();
    LYSystem(command);
    start_curses();

    /*
     *  Restore the fragment if there was one. - FM
     */
    if (number_sign)
	*number_sign = '#';
    return TRUE;

failure:
    /*
     *  Restore the fragment if there was one. - FM
     */
    if (number_sign)
	*number_sign = '#';
    return FALSE;
}
