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
#include "LYGlobalDefs.h"
#include "LYCurses.h"
#include "LYStrings.h"
#include "LYUtils.h"
#include "LYSignal.h"
#include "GridText.h"

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


#define	SERVER_ASKED_FOR_REDIRECTION \
 "Server asked for redirection of POST content to"
#define	PROCEED_GET_CANCEL "P)roceed, use G)ET or C)ancel "
#define	ADVANCED_POST_REDIRECT \
 "Redirection of POST content. P)roceed, see U)RL, use G)ET or C)ancel"
#define	LOCATION_HEADER "Location: "

/*      Confirm redirection of POST		HTConfirmPostRedirect()
**
** On entry,
**      redirecting_url             is the Location.
**
** On exit,
**      Returns 0 on cancel,
**	  1 for redirect of POST with content,
**	303 for redirect as GET without content
*/
PUBLIC int HTConfirmPostRedirect ARGS1(
	CONST char *,	redirecting_url)
{
    char *show_POST_url = NULL;
    char url[256];
    int on_screen = 0;	/* 0 - show menu
   			 * 1 - show url
			 * 2 - menu is already on screen */

    if (dump_output_immediately)
	/*
	 *  Treat as 303 (GET without content) if not interactive.
	 */
        return 303;

    if (user_mode == NOVICE_MODE) {
        on_screen = 2;
        move(LYlines-2, 0);
        addstr(SERVER_ASKED_FOR_REDIRECTION);
	clrtoeol();
        move(LYlines-1, 0);
	sprintf(url, "URL: %.*s",
		    (LYcols < 250 ? LYcols-6 : 250), redirecting_url);
        addstr(url);
	clrtoeol();
        _statusline(PROCEED_GET_CANCEL);
    } else {
	StrAllocCopy(show_POST_url, LOCATION_HEADER);
	StrAllocCat(show_POST_url, redirecting_url);
    }
    while (1) {
	int c;  

	switch (on_screen) {
	    case 0:
	        _statusline(ADVANCED_POST_REDIRECT);
		break;
	    case 1:
	        _statusline(show_POST_url);
	}
	c = LYgetch();
	switch (TOUPPER(c)) {
	    case 'P':
		/*
		 *  Proceed with 301 or 302 redirect of POST
		 *  (we check only for 0 and 303 in HTTP.c).
		 */
	        FREE(show_POST_url);
		return 1;	

 	    case 7:
 	    case 'C':
	        /*
		 * Cancel request.
		 */
	        FREE(show_POST_url);
		return 0;

	    case 'G':
	        /*
		 *  Treat as 303 (GET without content).
		 */
	        FREE(show_POST_url);
		return 303;

	    case 'U':
	        /*
		 *  Show URL for intermediate or advanced mode.
		 */
	        if (user_mode != NOVICE_MODE)
		    if (on_screen == 1)
			on_screen = 0;
		    else
			on_screen = 1;
		break;

	    default:
	        /*
		 *  Get another character.
		 */
		if (on_screen == 1)
		    on_screen = 0;
		else
		    on_screen = 2;
	}
    }
}

