/*	Displaying messages and getting input for Lynx Browser
**	==========================================================
**
**	REPLACE THIS MODULE with a GUI version in a GUI environment!
**
** History:
**	   Jun 92 Created May 1992 By C.T. Barker
**	   Feb 93 Simplified, portablised TBL
**
*/


#include "HTUtils.h"
#include "tcp.h"
#include "HTAlert.h"
#include "LYStrings.h"
#include "LYUtils.h"
#include "LYSignal.h"
#include "GridText.h"
#include "LYGlobalDefs.h"

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}


PUBLIC void HTAlert ARGS1(CONST char *, Msg)
{
    if(TRACE) {
        fprintf(stderr, "\nAlert!: %s", (char *)Msg);
	fflush(stderr);
        _user_message("Alert!: %s", (char *)Msg);
        fprintf(stderr, "\n\n");
	fflush(stderr);
    } else
        _user_message("Alert!: %s", (char *)Msg);

    sleep(AlertSecs);
}


PUBLIC void HTProgress ARGS1(CONST char *, Msg)
{
    if(TRACE)
        fprintf(stderr, "%s\n", (char *)Msg);
    else
        statusline((char *)Msg);
}


PUBLIC BOOL HTConfirm ARGS1(CONST char *, Msg)
{
    if (dump_output_immediately) { /* Non-interactive, can't respond */
	return(NO);
    } else {
	int c;
#ifdef VMS
	extern BOOLEAN HadVMSInterrupt;
#endif /* VMS */
	
	_user_message("WWW: %s (y/n) ", (char *) Msg);
	
	while(1) {
	    c = LYgetch();
#ifdef VMS
	    if(HadVMSInterrupt) {
		HadVMSInterrupt = FALSE;
		c = 'N';
	    }
#endif /* VMS */
	    if(TOUPPER(c)=='Y')
		return(YES);
	    if(TOUPPER(c)=='N' || c == 7 || c == 3) /* ^G or ^C cancels */
		return(NO);
	}
    }
}

/*	Prompt for answer and get text back
*/
PUBLIC char * HTPrompt ARGS2(CONST char *, Msg, CONST char *, deflt)
{
    char * rep = NULL;
    char Tmp[200];

    Tmp[0] = '\0';
    Tmp[199] = '\0';

    _statusline((char *)Msg);
    if (deflt) 
        strncpy(Tmp, deflt, 199);

    if (!dump_output_immediately)
        LYgetstr(Tmp, VISIBLE, sizeof(Tmp), NORECALL);

    StrAllocCopy(rep, Tmp);

    return rep;
}

/*      Prompt for password without echoing the reply
*/
PUBLIC char * HTPromptPassword ARGS1(CONST char *, Msg)
{
    char *result = NULL;
    char pw[120];

    pw[0]='\0';

    if (!dump_output_immediately) {
        _statusline(Msg ? (char *)Msg : PASSWORD_PROMPT);
        LYgetstr(pw, HIDDEN, sizeof(pw), NORECALL); /* hidden */
        StrAllocCopy(result, pw);
    } else {
        printf("\n%s\n", PASSWORD_REQUIRED);
	StrAllocCopy(result, "");
    }
    return result;
}


/*      Prompt both username and password       HTPromptUsernameAndPassword()
**      ---------------------------------
** On entry,
**      Msg             is the prompting message.
**      *username and
**      *password       are char pointers; they are changed
**                      to point to result strings.
**
**                      If *username is not NULL, it is taken
**                      to point to  a default value.
**                      Initial value of *password is
**                      completely discarded.
**
** On exit,
**      *username and *password point to newly allocated
**      strings -- original strings pointed to by them
**      are NOT freed.
**
*/
PUBLIC void HTPromptUsernameAndPassword ARGS3(CONST char *,     Msg,
                                              char **,          username,
                                              char **,          password)
{
    if (authentication_info[0] && authentication_info[1]) {
	/* 
	 *  -auth parameter gave us both the username and password
	 *  to use for the first realm, so just use them without
	 *  any prompting. - FM
	 */
 	StrAllocCopy(*username, authentication_info[0]);
	FREE(authentication_info[0]);
	StrAllocCopy(*password, authentication_info[1]);
	FREE(authentication_info[1]);
    } else if (dump_output_immediately) {
        if (authentication_info[0]) {
	    /*
	     *  Use the command line username. - FM
	     */
	    StrAllocCopy(*username, authentication_info[0]);
	    FREE(authentication_info[0]);
	} else {
	    /*
	     *  Default to "WWWuser". - FM
	     */
            StrAllocCopy(*username, "WWWuser");
	}
	if (authentication_info[1]) {
	    /*
	     *  Use the command line password. - FM
	     */
	    StrAllocCopy(*password, authentication_info[1]);
	    FREE(authentication_info[1]);
	} else {
	    /*
	     *  Default to a zero-length string. - FM
	     */
	    StrAllocCopy(*password, "");
	}
	printf("\n%s\n", USERNAME_PASSWORD_REQUIRED);
    } else {
        if (authentication_info[0]) {
	    /*
	     *  Offer command line username in the prompt
	     *  for the first realm. - FM
	     */
	    StrAllocCopy(*username, authentication_info[0]);
	    FREE(authentication_info[0]);
	}
	if (Msg) {
	    *username = HTPrompt(Msg, *username);
	} else {
	    *username = HTPrompt(USERNAME_PROMPT, *username);
	}
	if (authentication_info[1]) {
	    /*
	     *  Use the command line password for the first realm. - FM
	     */
	    StrAllocCopy(*password, authentication_info[1]);
	    FREE(authentication_info[1]);
	} else if (*username != NULL && *username[0] != '\0') {
	    /*
	     *  If we have a non-zero length username,
	     *  prompt for the password. - FM
	     */
	    *password = HTPromptPassword(PASSWORD_PROMPT);
	} else {
	    /*
	     *  Return a zero-length password. - FM
	     */
	     StrAllocCopy(*password, "");
	}
	
    }
}

