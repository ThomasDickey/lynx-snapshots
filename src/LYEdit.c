#include "HTUtils.h"
#include "tcp.h"
#include "HTParse.h"
#include "HTAlert.h"
#include "LYCurses.h"
#include "LYSignal.h"
#include "LYUtils.h"
#include "LYClean.h"
#include "LYGlobalDefs.h"
#include "LYEdit.h"
#include "LYStrings.h"
#include "LYSystem.h"
#ifdef VMS
#include <unixio.h>
#include "HTVMSUtils.h"
#endif /* VMS */

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

/*
 *  in edit mode invoke either emacs, vi, pico, jove, jed or the default
 *  editor to display and edit the current file
 *  emacs, vi, pico, jove and jed will open the file to the same line that
 *  the screen cursor is on when editing is invoked
 *  returns FALSE if file uneditable
 */

PUBLIC int edit_current_file ARGS3(char *,newfile, int,cur, int,lineno)
{

	char command[512];
        char *filename = NULL;
	char *colon, *number_sign;
	FILE *fp;

	/*
	 * If its a remote file then we can't edit it.
	 */
	if(!LYisLocalFile(newfile)) {
	    _statusline(CANNOT_EDIT_REMOTE_FILES);
	    sleep(MessageSecs);
	    return FALSE;
	}

	number_sign = strchr(newfile,'#');
	if(number_sign)
	    *number_sign = '\0';
	   
	 /*
	  * On Unix, first try to open it as a completely referenced file,
	  * then via the path alone.
	  *
	  * On VMS, only try the path.
	  */
#ifndef VMS
	colon = strchr(newfile,':');
	StrAllocCopy(filename, colon+1);
	HTUnEscape(filename);
	if((fp = fopen(filename,"r")) == NULL) {
	    FREE(filename);
#endif /* !VMS */
	    filename = HTParse(newfile,"",PARSE_PATH+PARSE_PUNCTUATION);
	    HTUnEscape(filename);
#ifdef VMS
	    if ((fp = fopen(HTVMS_name("",filename),"r")) == NULL) {
#else
	    if ((fp = fopen(filename,"r")) == NULL) {
		    
#endif /* VMS */
	        HTAlert(COULD_NOT_ACCESS_FILE);
		FREE(filename);
		goto failure;
	    }
#ifndef VMS
	}
#endif /* !VMS */
	fclose(fp);
		

#if defined(VMS) || defined(CANT_EDIT_UNWRITABLE_FILES)
        /*
	 * Don't allow editing if user lacks append access.
	 */
#ifdef VMS
	if ((fp = fopen(HTVMS_name("",filename),"a")) == NULL) {
#else
	if ((fp = fopen(filename,"a")) == NULL) {
#endif /* VMS */
		_statusline(NOAUTH_TO_EDIT_FILE);
		sleep(MessageSecs);
		goto failure;
	}
	fclose(fp);
#endif /* VMS || CANT_EDIT_UNWRITABLE_FILES */

#ifdef VMS
        sprintf(command,"%s %s",editor, HTVMS_name("",filename));
#else
	if (strstr(editor,"emacs") || strstr(editor,"vi") ||
	    strstr(editor, "pico") || strstr(editor,"jove") ||
	    strstr(editor, "jed"))
	    sprintf(command,"%s +%d \"%s\"",editor, lineno+links[cur].ly, 
                                                                filename);
	else
	    sprintf(command,"%s \"%s\"",editor, filename);
#endif /* VMS */
	if (TRACE) {
	    fprintf(stderr, "LYEdit: %s\n",command);
	    sleep(MessageSecs);
	}
	FREE(filename);

	stop_curses();
	system(command);
	start_curses();

	if(number_sign)
	    *number_sign = '#';
	return TRUE;

failure:
	if(number_sign)
	    *number_sign = '#';
	return FALSE;
}
