/*
**  Routines to upload files to the local filesystem.
**  Created by: Rick Mallett, Carleton University
**  Report problems to rmallett@ccs.carleton.ca
**  Modified 15-Dec-95 George Lindholm (lindholm@ucs.ubc.ca):
**	Reread the upload menu page every time, in case the "upload" directory
**	  has changed (make the current directory that for the upload process).
**	Prompt for the upload file name if there is no "%s" in the command
**	  string. Most protocols allow the user to specify the file name
**	  from the client side.  Xmodem appears to be the only that can't
**	  figure out the filename from the transfer data so it needs the
**	  information from lynx (or an upload script which prompts for it).
**	  On the other hand, zmodem aborts when you give it a filename on
**	  the command line (great way of bypassing the nodotfile code :=( ).
*/

#include <HTUtils.h>
#include <HTFile.h>
#include <HTParse.h>
#include <HTAlert.h>
#include <LYCurses.h>
#include <LYUtils.h>
#include <LYGlobalDefs.h>
#include <LYSignal.h>
#include <LYStrings.h>
#include <LYClean.h>
#include <LYGetFile.h>
#include <LYUpload.h>
#include <LYLocal.h>

#include <LYexit.h>
#include <LYLeaks.h>

PUBLIC char LYUploadFileURL[LY_MAXPATH] = "\0";

/*
 *  LYUpload uploads a file to a given location using a
 *  specified upload method.  It parses an incoming link
 *  that looks like:
 *	LYNXDIRED://UPLOAD=<#>/TO=<STRING>
 */
PUBLIC int LYUpload ARGS1(
	char *, 	line)
{
    char *method, *directory, *dir;
    int method_number;
    int count;
    char tmpbuf[LY_MAXPATH];
    char buffer[LY_MAXPATH];
    lynx_html_item_type *upload_command = 0;
    int c;
    char *cp;
    FILE *fp;
    char cmd[20 + (LY_MAXPATH*2)];
#ifdef VMS
    extern BOOLEAN HadVMSInterrupt;
#endif /* VMS */

    /*
     *	Use configured upload commands.
     */
    if((directory = (char *)strstr(line, "TO=")) == NULL)
	goto failed;
    *(directory - 1) = '\0';
    /* go past "Directory=" */
    directory += 3;

    if((method = (char *)strstr(line, "UPLOAD=")) == NULL)
	goto failed;
    /*
     *	Go past "Method=".
     */
    method += 7;
    method_number = atoi(method);

    for (count = 0, upload_command = uploaders; count < method_number;
	count++, upload_command = upload_command->next)
      ; /* null body */

    /*
     *	Parsed out the Method and the Location?
     */
    if (upload_command->command == NULL) {
	HTAlert("ERROR! - upload command is misconfigured");
	goto failed;
    }

    /*
     *	Care about the local name?
     */
    if (strstr(upload_command->command, "%s")) {
	/*
	 *  Commands have the form "command %s [etc]"
	 *  where %s is the filename.
	 */
	_statusline(FILENAME_PROMPT);
retry:
	*tmpbuf = '\0';
	if (LYgetstr(tmpbuf, VISIBLE, sizeof(tmpbuf), NORECALL) < 0)
	    goto cancelled;

	if (*tmpbuf == '\0')
	    goto cancelled;

	if (strstr(tmpbuf, "../") != NULL) {
	    HTAlert("Illegal redirection \"../\" found! Request ignored.");
	    goto cancelled;
	} else if (strchr(tmpbuf, '/') != NULL) {
	    HTAlert("Illegal character \"/\" found! Request ignored.");
	    goto cancelled;
	} else if (tmpbuf[0] == '~') {
	    HTAlert("Illegal redirection using \"~\" found! Request ignored.");
	    goto cancelled;
	}
	sprintf(buffer, "%s/%s", directory, tmpbuf);

	if (no_dotfiles || !show_dotfiles) {
	    if (*LYPathLeaf(buffer) == '.') {
		HTAlert(FILENAME_CANNOT_BE_DOT);
		_statusline(NEW_FILENAME_PROMPT);
		goto retry;
	    }
	}

	/*
	 *  See if it already exists.
	 */
	if ((fp = fopen(buffer, "r")) != NULL) {
	    fclose(fp);

#ifdef VMS
	    _statusline(FILE_EXISTS_HPROMPT);
#else
	    _statusline(FILE_EXISTS_OPROMPT);
#endif /* VMS */
	    c = 0;
	    while (TOUPPER(c) != 'Y' && TOUPPER(c) != 'N' && c != 7 && c != 3)
		c = LYgetch();
#ifdef VMS
	    if (HadVMSInterrupt) {
		HadVMSInterrupt = FALSE;
		goto cancelled;
	    }
#endif /* VMS */

	    if (c == 7 || c == 3) { /* Control-G or Control-C */
		goto cancelled;
	    }

	    if (TOUPPER(c) == 'N') {
		_statusline(NEW_FILENAME_PROMPT);
		goto retry;
	    }
	}

	/*
	 *  See if we can write to it.
	 */
	CTRACE(tfp, "LYUpload: filename is %s", buffer);

	if ((fp = fopen(buffer, "w")) != NULL) {
	    fclose(fp);
	    remove(buffer);
	} else {
	    HTAlert(CANNOT_WRITE_TO_FILE);
	    _statusline(NEW_FILENAME_PROMPT);
	    goto retry;
	}

#if defined (VMS) || defined (__EMX__) || defined(__DJGPP__)
	sprintf(tmpbuf, upload_command->command, buffer, "", "", "", "", "");
#else
	cp = quote_pathname(buffer); /* to prevent spoofing of the shell */
	sprintf(tmpbuf, upload_command->command, cp, "", "", "", "", "");
	FREE(cp);
#endif /* VMS */
    } else {			/* No substitution, no changes */
	strcpy(tmpbuf, upload_command->command);
    }

    dir = quote_pathname(directory);
    sprintf(cmd, "cd %s ; %s", dir, tmpbuf);
    FREE(dir);
    stop_curses();
    CTRACE(tfp, "command: %s\n", cmd);
    LYSystem(cmd);
    start_curses();
#ifdef UNIX
    chmod(buffer, HIDE_CHMOD);
#endif /* UNIX */
    /* don't remove(file); */

    return 1;

failed:
    HTAlert("Unable to upload file.");
    return 0;

cancelled:
    HTInfoMsg("Cancelling.");
    return 0;
}

/*
 *  LYUpload_options writes out the current upload choices to a
 *  file so that the user can select printers in the same way that
 *  they select all other links.  Upload links look like:
 *	LYNXDIRED://UPLOAD=<#>/TO=<STRING>
 */
PUBLIC int LYUpload_options ARGS2(
	char **,	newfile,
	char *, 	directory)
{
    static char tempfile[LY_MAXPATH];
    FILE *fp0;
    lynx_html_item_type *cur_upload;
    int count;
    static char curloc[LY_MAXPATH];
    char *cp;

    LYRemoveTemp(tempfile);
    if ((fp0 = LYOpenTemp(tempfile, HTML_SUFFIX, "w")) == NULL) {
	HTAlert(CANNOT_OPEN_TEMP);
	return(-1);
    }

#ifdef VMS
    strcpy(curloc, "/sys$login");
#else
    cp = HTfullURL_toFile(directory);
    strcpy(curloc,cp);
    LYTrimPathSep(curloc);
    free(cp);
#endif /* VMS */

    LYLocalFileToURL(newfile, tempfile);
    strcpy(LYUploadFileURL, *newfile);

    BeginInternalPage(fp0, UPLOAD_OPTIONS_TITLE, UPLOAD_OPTIONS_HELP);

    fprintf(fp0, "<pre>\n");
    fprintf(fp0, "   <em>Upload To:</em> %s\n", curloc);
    fputs("\nUpload options:\n", fp0);

    if (uploaders != NULL) {
	for (count = 0, cur_upload = uploaders;
	     cur_upload != NULL;
	     cur_upload = cur_upload->next, count++) {
	    fprintf(fp0, "   <a href=\"LYNXDIRED://UPLOAD=%d/TO=%s\">",
			 count, curloc);
	    fprintf(fp0, (cur_upload->name ?
			  cur_upload->name : "No Name Given"));
	    fprintf(fp0, "</a>\n");
	}
    } else {
	fprintf(fp0, "   &lt;NONE&gt;\n");
    }

    fprintf(fp0, "</pre>\n");
    EndInternalPage(fp0);
    LYCloseTempFP(fp0);

    LYforce_no_cache = TRUE;

    return(0);
}
