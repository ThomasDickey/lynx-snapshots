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

#include <HTUtils.h>
#include <HTAlert.h>
#include <LYGlobalDefs.h>
#include <LYCurses.h>
#include <LYStrings.h>
#include <LYUtils.h>
#include <LYClean.h>
#include <GridText.h>
#include <LYCookie.h>
#include <LYHistory.h> /* store statusline messages */

#include <LYLeaks.h>

#if defined(WIN_EX) && defined(UNUSED_CODE)
#include <HTParse.h>
#endif

/*	Issue a message about a problem.		HTAlert()
**	--------------------------------
*/
PUBLIC void HTAlert ARGS1(
	CONST char *,	Msg)
{
    CTRACE((tfp, "\nAlert!: %s\n\n", Msg));
    CTRACE_FLUSH(tfp);
    _user_message(ALERT_FORMAT, Msg);
    LYstore_message2(ALERT_FORMAT, Msg);

    sleep(AlertSecs);
}

PUBLIC void HTAlwaysAlert ARGS2(
	CONST char *,	extra_prefix,
	CONST char *,	Msg)
{
    if (!dump_output_immediately && LYCursesON) {
	HTAlert(Msg);
    } else {
	if (extra_prefix) {
	    fprintf(((TRACE) ? stdout : stderr),
		    "%s %s!\n",
		    extra_prefix, Msg);
	    fflush(stdout);
	    LYstore_message2(ALERT_FORMAT, Msg);
	    sleep(AlertSecs);
	} else {
	    fprintf(((TRACE) ? stdout : stderr),
		    ALERT_FORMAT,
		    (Msg == 0) ? "" : Msg);
	    fflush(stdout);
	    LYstore_message2(ALERT_FORMAT, Msg);
	    sleep(AlertSecs);
	    fprintf(((TRACE) ? stdout : stderr), "\n");
	}
	CTRACE((tfp, "\nAlert!: %s\n\n", Msg));
	CTRACE_FLUSH(tfp);
    }
}

/*	Issue an informational message.			HTInfoMsg()
**	--------------------------------
*/
PUBLIC void HTInfoMsg ARGS1(
	CONST char *,	Msg)
{
    _statusline(Msg);
    if (Msg && *Msg) {
	CTRACE((tfp, "Info message: %s\n", Msg));
	LYstore_message(Msg);
	sleep(InfoSecs);
    }
}

/*	Issue an important message.			HTUserMsg()
**	--------------------------------
*/
PUBLIC void HTUserMsg ARGS1(
	CONST char *,	Msg)
{
    _statusline(Msg);
    if (Msg && *Msg) {
	CTRACE((tfp, "User message: %s\n", Msg));
	LYstore_message(Msg);
	sleep(MessageSecs);
    }
}

PUBLIC void HTUserMsg2 ARGS2(
	CONST char *,	Msg2,
	CONST char *,	Arg)
{
    _user_message(Msg2, Arg);
    if (Msg2 && *Msg2) {
	CTRACE((tfp, "User message: "));
	CTRACE((tfp, Msg2, Arg));
	CTRACE((tfp, "\n"));
	LYstore_message2(Msg2, Arg);
	sleep(MessageSecs);
    }
}

#if defined(WIN_EX) && defined(UNUSED_CODE)	/* 1997/10/28 (Tue) 17:19:43 */

#define MAX_LEN	512

void ws_title(CONST char *str)
{
    char buff[MAX_LEN];
    char *p;
    int len;

#define TITLE_CUT 32

    p = (char *)str;
    len = strlen(p);
    if (len > (MAX_LEN - 1)) {
	strncpy(buff, p, (MAX_LEN - 1));
	len = MAX_LEN - 1;
	buff[MAX_LEN - 1] = '\0';
    } else {
	strcpy(buff, p);
    }

    if (len > LYcols) {
	buff[TITLE_CUT] = '.';
	buff[TITLE_CUT+1] = '.';
	strcpy(buff + TITLE_CUT + 2, (buff + len) - LYcols + TITLE_CUT + 1);
    }
    if (strchr(buff, '%')) {
	HTUnEscape(buff);
    }

    p = buff;
    while (*p++) {
	if (*p == '\r') {
	    *p = '\0';
	    break;
	} else if (*p ==  '\n') {
	    *p = '\0';
	    break;
	}
    }

    /* Quick hack. buff is SJIS only ??? */
    SetConsoleTitle(buff);
}
#endif

/*	Issue a progress message.			HTProgress()
**	-------------------------
*/
PUBLIC void HTProgress ARGS1(
	CONST char *,	Msg)
{
    statusline(Msg);
    LYstore_message(Msg);
    CTRACE((tfp, "%s\n", Msg));
#if defined(SH_EX) && defined(WIN_EX)	/* 1997/10/11 (Sat) 12:51:02 */
    {
	if (debug_delay != 0)
	    Sleep(debug_delay);	/* XXX msec */
    }
#endif
}

#ifdef EXP_READPROGRESS
PRIVATE char *sprint_bytes ARGS3(
	char *,		s,
	long,		n,
	char *, 	was_units)
{
    static long kb_units = 1024;
    static char *bunits;
    static char *kbunits;
    char *u;

    if (!bunits) {
	bunits = gettext("bytes");
	kbunits = gettext("KB");
    }

    u = kbunits;
    if (LYshow_kb_rate && (n >= 10 * kb_units))
	sprintf(s, "%ld", n/kb_units);
    else if (LYshow_kb_rate && (n >= kb_units))
	sprintf(s, "%.2g", ((double)n)/kb_units);
    else {
	sprintf(s, "%ld", n);
	u = bunits;
    }

    if (!was_units || was_units != u)
	sprintf(s + strlen(s), " %s", u);
    return u;
}
#endif /* EXP_READPROGRESS */

/*	Issue a read-progress message.			HTReadProgress()
**	------------------------------
*/
PUBLIC void HTReadProgress ARGS2(
	long,		bytes,
	long,		total)
{
#ifdef WIN_EX	/* 1998/07/08 (Wed) 16:09:47 */

#include <sys/timeb.h>
#define	kb_units 1024L
    static double now, first, last;
    static long bytes_last;

    double transfer_rate;
    char line[80];
    struct timeb tb;
    char *units = "bytes";

    ftime(&tb);
    now = tb.time + (double)tb.millitm / 1000;

    if (bytes == 0) {
	first = last = now;
	bytes_last = bytes;
    } else if ((bytes > 0) && (now > first)) {
	transfer_rate = (double)bytes / (now - first);   /* bytes/sec */

	if (now != last) {
	    last = now;
	    bytes_last = bytes;
	}
	if (total >= kb_units || bytes >= kb_units) {
	    if (total > 0)
		total /= 1024;
	    bytes /= 1024;
	    units = "KB";
	}

	if (total >  0)
	    sprintf (line, "Read %3d%%, %ld of %ld %s.",
		(int) (bytes * 100 / total), bytes, total, units);
	else
	    sprintf (line, "Read %ld %s of data.", bytes, units);

	if (transfer_rate > 0.0) {
	    int n;
	    n = strlen(line);
	    if (LYshow_kb_rate) {
		sprintf (line + n, " %6.2lf KB/sec.", transfer_rate / 1024.0);
	    } else {
		int t_rate;

		t_rate = (int)transfer_rate;
		if (t_rate < 1000)
		    sprintf (line + n, " %6d bytes/sec.", t_rate);
		else
		    sprintf (line + n, " %6d,%03d bytes/sec.",
					t_rate / 1000, t_rate % 1000);
	    }
	}
	if (total <  0) {
	    if (total < -1)
		strcat(line, " (Press 'z' to abort)");
	}
	statusline(line);
    }
#else /* !WIN_EX */
#ifdef EXP_READPROGRESS
    static long bytes_last, total_last;
    static long transfer_rate = 0;
    char line[300], bytesp[80], totalp[80], transferp[80];
    int renew = 0;
    char *was_units;
#if HAVE_GETTIMEOFDAY
    struct timeval tv;
    int dummy = gettimeofday(&tv, (struct timezone *)0);
    double now = tv.tv_sec + tv.tv_usec/1000000. ;
    static double first, last, last_active;
#else
    time_t now = time((time_t *)0);  /* once per second */
    static time_t first, last, last_active;
#endif

    if (bytes == 0) {
	first = last = last_active = now;
	bytes_last = bytes;
	line[0] = 0;
    } else if (bytes < 0) {	/* stalled */
	bytes = bytes_last;
	total = total_last;
    }
    if ((bytes > 0) &&
	       (now != first))
		/* 1 sec delay for transfer_rate calculation without g-t-o-d */ {
	if (transfer_rate <= 0)    /* the very first time */
	    transfer_rate = (bytes) / (now - first);   /* bytes/sec */
	total_last = total;

	/*
	 * Optimal refresh time:  every 0.2 sec, use interpolation.  Transfer
	 * rate is not constant when we have partial content in a proxy, so
	 * interpolation lies - will check every second at least for sure.
	 */
#if HAVE_GETTIMEOFDAY
	if (now >= last + 0.2)
	    renew = 1;
#else
	if (((bytes - bytes_last) > (transfer_rate / 5)) || (now != last)) {
	    renew = 1;
	    bytes_last += (transfer_rate / 5);	/* until we got next second */
	}
#endif
	if (renew) {
	    if (now != last) {
		last = now;
		if (bytes_last != bytes)
		    last_active = now;
		bytes_last = bytes;
		transfer_rate = bytes / (now - first); /* more accurate here */
	    }

	    if (total > 0)
		was_units = sprint_bytes(totalp, total, 0);
	    else
		was_units = 0;
	    sprint_bytes(bytesp, bytes, was_units);
	    sprint_bytes(transferp, transfer_rate, 0);

	    if (total > 0)
		sprintf (line, gettext("Read %s of %s of data"), bytesp, totalp);
	    else
		sprintf (line, gettext("Read %s of data"), bytesp);
	    if (transfer_rate > 0)
		sprintf (line + strlen(line), gettext(", %s/sec"), transferp);
	    if (now - last_active >= 5)
		sprintf (line + strlen(line), gettext(" (stalled for %ld sec)"), (long)(now - last_active));
	    if (total > 0 && transfer_rate)
		sprintf (line + strlen(line), gettext(", ETA %ld sec"), (long)((total - bytes)/transfer_rate));
	    sprintf (line + strlen(line), ".");
	    if (total < -1)
		strcat(line, gettext(" (Press 'z' to abort)"));

	    /* do not store the message for history page. */
	    statusline(line);
	    CTRACE((tfp, "%s\n", line));
	}
    }
#else /* !EXP_READPROGRESS */
    static long kb_units = 1024;
    static time_t first, last;
    static long bytes_last;
    static long transfer_rate = 0;
    long divisor;
    char line[80];
    time_t now = time((time_t *)0);  /* once per second */
    static char *units = "bytes";

    if (bytes == 0) {
	first = last = now;
	bytes_last = bytes;
    } else if ((bytes > 0) &&
	       (now != first))
		/* 1 sec delay for transfer_rate calculation :-( */ {
	if (transfer_rate <= 0)    /* the very first time */
	    transfer_rate = (bytes) / (now - first);   /* bytes/sec */

	/*
	 * Optimal refresh time:  every 0.2 sec, use interpolation.  Transfer
	 * rate is not constant when we have partial content in a proxy, so
	 * interpolation lies - will check every second at least for sure.
	 */
	if (((bytes - bytes_last) > (transfer_rate / 5)) || (now != last)) {

	    bytes_last += (transfer_rate / 5);	/* until we got next second */

	    if (now != last) {
		last = now;
		bytes_last = bytes;
		transfer_rate = (bytes_last) / (last - first); /* more accurate here */
	    }

	    units = gettext("bytes");
	    divisor = 1;
	    if (LYshow_kb_rate
	      && (total >= kb_units || bytes >= kb_units)) {
		units = gettext("KB");
		divisor = kb_units;
		bytes /= divisor;
		if (total > 0) total /= divisor;
	    }

	    if (total >  0)
		sprintf (line, gettext("Read %ld of %ld %s of data"), bytes, total, units);
	    else
		sprintf (line, gettext("Read %ld %s of data"), bytes, units);
	    if ((transfer_rate > 0)
		  && (!LYshow_kb_rate || (bytes * divisor >= kb_units)))
		sprintf (line + strlen(line), gettext(", %ld %s/sec."), transfer_rate / divisor, units);
	    else
		sprintf (line + strlen(line), ".");
	    if (total <  0) {
		if (total < -1)
		    strcat(line, gettext(" (Press 'z' to abort)"));
	    }

	    /* do not store the message for history page. */
	    statusline(line);
	    CTRACE((tfp, "%s\n", line));
	}
    }
#endif /* EXP_READPROGRESS */
#endif /* WIN_EX */
}

PRIVATE BOOL conf_cancelled = NO; /* used by HTConfirm only - kw */

PUBLIC BOOL HTLastConfirmCancelled NOARGS
{
    if (conf_cancelled) {
	conf_cancelled = NO;	/* reset */
	return(YES);
    } else {
	return(NO);
    }
}

#define DFT_CONFIRM ~(YES|NO)

/*	Seek confirmation with default answer.		HTConfirmDefault()
**	--------------------------------------
*/
PUBLIC int HTConfirmDefault ARGS2(CONST char *, Msg, int, Dft)
{
    char *msg_yes = gettext("yes");
    char *msg_no  = gettext("no");
    int result = -1;

    conf_cancelled = NO;
    if (dump_output_immediately) { /* Non-interactive, can't respond */
	if (Dft == DFT_CONFIRM) {
	    CTRACE((tfp, "Confirm: %s (%c/%c) ", Msg, *msg_yes, *msg_no));
	} else {
	    CTRACE((tfp, "Confirm: %s (%c) ", Msg, (Dft == YES) ? *msg_yes : *msg_no));
	}
	CTRACE((tfp, "- NO, not interactive.\n"));
	result = NO;
    } else {
	char *msg = NULL;

	if (Dft == DFT_CONFIRM)
	    HTSprintf0(&msg, "%s (%c/%c) ", Msg, *msg_yes, *msg_no);
	else
	    HTSprintf0(&msg, "%s (%c) ", Msg, (Dft == YES) ? *msg_yes : *msg_no);
	if (LYTraceLogFP) {
	    CTRACE((tfp, "Confirm: %s", msg));
	}
	_statusline(msg);
	FREE(msg);

	while (result < 0) {
	    int c = LYgetch_for(FOR_SINGLEKEY);
#ifdef VMS
	    if (HadVMSInterrupt) {
		HadVMSInterrupt = FALSE;
		c = *msg_no;
	    }
#endif /* VMS */
	    if (c == 7 || c == 3) { /* remember we had ^G or ^C */
		conf_cancelled = YES;
		result = NO;
	    } else if (TOUPPER(c) == TOUPPER(*msg_yes)) {
		result = YES;
	    } else if (TOUPPER(c) == TOUPPER(*msg_no)) {
		result = NO;
	    } else if (Dft != DFT_CONFIRM) {
		result = Dft;
		break;
	    }
	}
	CTRACE((tfp, "- %s%s.\n",
	       (result != NO) ? "YES" : "NO",
	       conf_cancelled ? ", cancelled" : ""));
    }
    return (result);
}

/*	Seek confirmation.				HTConfirm()
**	------------------
*/
PUBLIC BOOL HTConfirm ARGS1(CONST char *, Msg)
{
    return (BOOL) HTConfirmDefault(Msg, DFT_CONFIRM);
}

/*	Prompt for answer and get text back.		HTPrompt()
**	------------------------------------
*/
PUBLIC char * HTPrompt ARGS2(
	CONST char *,	Msg,
	CONST char *,	deflt)
{
    char * rep = NULL;
    char Tmp[200];

    Tmp[0] = '\0';
    Tmp[sizeof(Tmp)-1] = '\0';

    _statusline(Msg);
    if (deflt)
	strncpy(Tmp, deflt, sizeof(Tmp)-1);

    if (!dump_output_immediately)
	LYgetstr(Tmp, VISIBLE, sizeof(Tmp), NORECALL);

    StrAllocCopy(rep, Tmp);

    return rep;
}

/*
**	Prompt for password without echoing the reply.	HTPromptPassword()
**	----------------------------------------------
*/
PUBLIC char * HTPromptPassword ARGS1(
	CONST char *,	Msg)
{
    char *result = NULL;
    char pw[120];

    pw[0] = '\0';

    if (!dump_output_immediately) {
	_statusline(Msg ? Msg : PASSWORD_PROMPT);
	LYgetstr(pw, HIDDEN, sizeof(pw), NORECALL); /* hidden */
	StrAllocCopy(result, pw);
    } else {
	printf("\n%s\n", PASSWORD_REQUIRED);
	StrAllocCopy(result, "");
    }
    return result;
}

/*	Prompt both username and password.	 HTPromptUsernameAndPassword()
**	----------------------------------
**
**  On entry,
**	Msg		is the prompting message.
**	*username and
**	*password	are char pointers which contain default
**			or zero-length strings; they are changed
**			to point to result strings.
**	IsProxy 	should be TRUE if this is for
**			proxy authentication.
**
**			If *username is not NULL, it is taken
**			to point to a default value.
**			Initial value of *password is
**			completely discarded.
**
**  On exit,
**	*username and *password point to newly allocated
**	strings -- original strings pointed to by them
**	are NOT freed.
**
*/
PUBLIC void HTPromptUsernameAndPassword ARGS4(
	CONST char *,	Msg,
	char **,	username,
	char **,	password,
	BOOL,		IsProxy)
{
    if ((IsProxy == FALSE &&
	 authentication_info[0] && authentication_info[1]) ||
	(IsProxy == TRUE &&
	 proxyauth_info[0] && proxyauth_info[1])) {
	/*
	**  The -auth or -pauth parameter gave us both the username
	**  and password to use for the first realm or proxy server,
	**  respectively, so just use them without any prompting. - FM
	*/
	StrAllocCopy(*username, (IsProxy ?
		       proxyauth_info[0] : authentication_info[0]));
	if (IsProxy) {
	    FREE(proxyauth_info[0]);
	} else {
	    FREE(authentication_info[0]);
	}
	StrAllocCopy(*password, (IsProxy ?
		       proxyauth_info[1] : authentication_info[1]));
	if (IsProxy) {
	    FREE(proxyauth_info[1]);
	} else {
	    FREE(authentication_info[1]);
	}
    } else if (dump_output_immediately) {
	/*
	 *  We are not interactive and don't have both the
	 *  username and password from the command line,
	 *  but might have one or the other. - FM
	 */
	if ((IsProxy == FALSE && authentication_info[0]) ||
	    (IsProxy == TRUE && proxyauth_info[0])) {
	    /*
	    **	Use the command line username. - FM
	    */
	    StrAllocCopy(*username, (IsProxy ?
			   proxyauth_info[0] : authentication_info[0]));
	    if (IsProxy) {
		FREE(proxyauth_info[0]);
	    } else {
		FREE(authentication_info[0]);
	    }
	} else {
	    /*
	    **	Default to "WWWuser". - FM
	    */
	    StrAllocCopy(*username, "WWWuser");
	}
	if ((IsProxy == FALSE && authentication_info[1]) ||
	    (IsProxy == TRUE && proxyauth_info[1])) {
	    /*
	    **	Use the command line password. - FM
	    */
	    StrAllocCopy(*password, (IsProxy ?
			   proxyauth_info[1] : authentication_info[1]));
	    if (IsProxy) {
		FREE(proxyauth_info[1]);
	    } else {
		FREE(authentication_info[1]);
	    }
	} else {
	    /*
	    **	Default to a zero-length string. - FM
	    */
	    StrAllocCopy(*password, "");
	}
	printf("\n%s\n", USERNAME_PASSWORD_REQUIRED);

    } else {
	/*
	 *  We are interactive and don't have both the
	 *  username and password from the command line,
	 *  but might have one or the other. - FM
	 */
	if ((IsProxy == FALSE && authentication_info[0]) ||
	    (IsProxy == TRUE && proxyauth_info[0])) {
	    /*
	    **	Offer the command line username in the
	    **	prompt for the first realm. - FM
	    */
	    StrAllocCopy(*username, (IsProxy ?
			   proxyauth_info[0] : authentication_info[0]));
	    if (IsProxy) {
		FREE(proxyauth_info[0]);
	    } else {
		FREE(authentication_info[0]);
	    }
	}
	/*
	 *  Prompt for confirmation or entry of the username. - FM
	 */
	if (Msg != NULL) {
	    *username = HTPrompt(Msg, *username);
	} else {
	    *username = HTPrompt(USERNAME_PROMPT, *username);
	}
	if ((IsProxy == FALSE && authentication_info[1]) ||
	    (IsProxy == TRUE && proxyauth_info[1])) {
	    /*
	    **	Use the command line password for the first realm. - FM
	    */
	    StrAllocCopy(*password, (IsProxy ?
			   proxyauth_info[1] : authentication_info[1]));
	    if (IsProxy) {
		FREE(proxyauth_info[1]);
	    } else {
		FREE(authentication_info[1]);
	    }
	} else if (*username != NULL && *username[0] != '\0') {
	    /*
	    **	We have a non-zero length username,
	    **	so prompt for the password. - FM
	    */
	    *password = HTPromptPassword(PASSWORD_PROMPT);
	} else {
	    /*
	    **	Return a zero-length password. - FM
	    */
	    StrAllocCopy(*password, "");
	}
    }
}

/*	Confirm a cookie operation.			HTConfirmCookie()
**	---------------------------
**
**  On entry,
**	server			is the server sending the Set-Cookie.
**	domain			is the domain of the cookie.
**	path			is the path of the cookie.
**	name			is the name of the cookie.
**	value			is the value of the cookie.
**
**  On exit,
**	Returns FALSE on cancel,
**		TRUE if the cookie should be set.
*/
PUBLIC BOOL HTConfirmCookie ARGS4(
	void *, 	dp,
	CONST char *,	server,
	CONST char *,	name,
	CONST char *,	value)
{
    domain_entry *de;
    int ch;

    if ((de = (domain_entry *)dp) == NULL)
	return FALSE;

    /*	If the user has specified a list of domains to allow or deny
    **	from the config file, then they'll already have de->bv set to
    **	ACCEPT_ALWAYS or REJECT_ALWAYS so we can relax and let the
    **	default cookie handling code cope with this fine.
    */

    /*
    **	If the user has specified a constant action, don't prompt at all.
    */
    if (de->bv == ACCEPT_ALWAYS)
	return TRUE;
    if (de->bv == REJECT_ALWAYS)
	return FALSE;

    if (dump_output_immediately) {
	/*
	**  Non-interactive, can't respond.  Use the LYSetCookies value
	*   based on its compilation or configuration setting, or on the
	**  command line toggle. - FM
	*/
	return LYSetCookies;
    }

    /*
    **	Estimate how much of the cookie we can show.
    */
    if(!LYAcceptAllCookies) {
	int namelen, valuelen, space_free, percentage;
	char *message = 0;

	space_free = ((LYcols - 1)
		      - (strlen(ADVANCED_COOKIE_CONFIRMATION)
			 - 10)		/* %s and %.*s and %.*s chars */
		      - strlen(server));
	if (space_free < 0)
	    space_free = 0;
	namelen = strlen(name);
	valuelen = strlen(value);
	if ((namelen + valuelen) > space_free) {
	    /*
	    **  Argh... there isn't enough space on our single line for
	    **  the whole cookie.  Reduce them both by a percentage.
	    **  This should be smarter.
	    */
	    percentage = (100 * space_free) / (namelen + valuelen);
	    namelen = (percentage * namelen) / 100;
	    valuelen = (percentage * valuelen) / 100;
	}
	HTSprintf(&message, ADVANCED_COOKIE_CONFIRMATION,
		 server, namelen, name, valuelen, value);
	_statusline(message);
	FREE(message);
    }
    while (1) {
	if(!LYAcceptAllCookies) {
	    ch = LYgetch_for(FOR_SINGLEKEY);
	} else {
	    ch = 'A';
	}
#ifdef VMS
	if (HadVMSInterrupt) {
	    HadVMSInterrupt = FALSE;
	    ch = 'N';
	}
#endif /* VMS */
	switch(TOUPPER(ch)) {
	    case 'A':
		/*
		**  Set to accept all cookies for this domain.
		*/
		de->bv = ACCEPT_ALWAYS;
		HTUserMsg2(ALWAYS_ALLOWING_COOKIES, de->domain);
		return TRUE;

	    case 'N':
	    case 7:	/* Ctrl-G */
	    case 3:	/* Ctrl-C */
		/*
		**  Reject the cookie.
		*/
		HTUserMsg(REJECTING_COOKIE);
		return FALSE;

	    case 'V':
		/*
		**  Set to reject all cookies from this domain.
		*/
		de->bv = REJECT_ALWAYS;
		HTUserMsg2(NEVER_ALLOWING_COOKIES, de->domain);
		return FALSE;

	    case 'Y':
		/*
		**  Accept the cookie.
		*/
		HTInfoMsg(ALLOWING_COOKIE);
		return TRUE;

	    default:
		continue;
	}
    }
}

/*	Confirm redirection of POST.		HTConfirmPostRedirect()
**	----------------------------
**
**  On entry,
**	Redirecting_url 	    is the Location.
**	server_status		    is the server status code.
**
**  On exit,
**	Returns 0 on cancel,
**	  1 for redirect of POST with content,
**	303 for redirect as GET without content
*/
PUBLIC int HTConfirmPostRedirect ARGS2(
	CONST char *,	Redirecting_url,
	int,		server_status)
{
    int result = -1;
    char *show_POST_url = NULL;
    char *StatusInfo = 0;
    char *url = 0;
    int on_screen = 0;	/* 0 - show menu
			 * 1 - show url
			 * 2 - menu is already on screen */

    if (server_status == 303 ||
	server_status == 302) {
	/*
	 *  HTTP.c should not have called us for either of
	 *  these because we're treating 302 as historical,
	 *  so just return 303. - FM
	 */
	return 303;
    }

    if (dump_output_immediately) {
	if (server_status == 301) {
	    /*
	    **	Treat 301 as historical, i.e., like 303 (GET
	    **	without content), when not interactive. - FM
	    */
	    return 303;
	} else {
	    /*
	    **	Treat anything else (e.g., 305, 306 or 307) as too
	    **	dangerous to redirect without confirmation, and thus
	    **	cancel when not interactive. - FM
	    */
	    return 0;
	}
    }

    if (user_mode == NOVICE_MODE) {
	on_screen = 2;
	move(LYlines-2, 0);
	HTSprintf0(&StatusInfo, SERVER_ASKED_FOR_REDIRECTION, server_status);
	addstr(StatusInfo);
	clrtoeol();
	move(LYlines-1, 0);
	HTSprintf0(&url, "URL: %.*s",
		    (LYcols < 250 ? LYcols-6 : 250), Redirecting_url);
	addstr(url);
	clrtoeol();
	if (server_status == 301) {
	    _statusline(PROCEED_GET_CANCEL);
	} else {
	    _statusline(PROCEED_OR_CANCEL);
	}
    } else {
	HTSprintf0(&StatusInfo, "%d %.*s",
			    server_status,
			    251,
			    ((server_status == 301) ?
			 ADVANCED_POST_GET_REDIRECT :
			 ADVANCED_POST_REDIRECT));
	StrAllocCopy(show_POST_url, LOCATION_HEADER);
	StrAllocCat(show_POST_url, Redirecting_url);
    }
    while (result < 0) {
	int c;

	switch (on_screen) {
	    case 0:
		_statusline(StatusInfo);
		break;
	    case 1:
		_statusline(show_POST_url);
	}
	c = LYgetch_for(FOR_SINGLEKEY);
	switch (TOUPPER(c)) {
	    case 'P':
		/*
		**  Proceed with 301 or 307 redirect of POST
		**  with same method and POST content. - FM
		*/
		FREE(show_POST_url);
		result = 1;
		break;

	    case 7:
	    case 'C':
		/*
		**  Cancel request.
		*/
		FREE(show_POST_url);
		result = 0;
		break;

	    case 'U':
		/*
		**  Show URL for intermediate or advanced mode.
		*/
		if (user_mode != NOVICE_MODE) {
		    if (on_screen == 1) {
			on_screen = 0;
		    } else {
			on_screen = 1;
		    }
		}
		break;

	    case 'G':
		if (server_status == 301) {
		    /*
		    **	Treat as 303 (GET without content).
		    */
		    FREE(show_POST_url);
		    result = 303;
		    break;
		}
		/* fall through to default */

	    default:
		/*
		**  Get another character.
		*/
		if (on_screen == 1) {
		    on_screen = 0;
		} else {
		    on_screen = 2;
		}
	}
    }
    FREE(StatusInfo);
    FREE(url);
    return (result);
}

/*
 *  LYstrerror emulates the ANSI strerror() function.
 */
#ifdef LYStrerror
    /* defined as macro in .h file. */
#else
PUBLIC char *LYStrerror ARGS1(int, code)
{
    static char temp[80];
    sprintf(temp, "System errno is %d.\r\n", code);
    return temp;
}
#endif /* HAVE_STRERROR */
