#include <HTUtils.h>
#include <HTParse.h>
#include <LYGlobalDefs.h>
#include <HTAlert.h>
#include <LYCurses.h>
#include <LYSignal.h>
#include <LYUtils.h>
#include <LYClean.h>
#include <LYStrings.h>
#include <GridText.h>
#include <LYMail.h>
#include <LYCharSets.h>  /* to get current charset for mail header */

#include <LYLeaks.h>

BOOLEAN term_letter;	/* Global variable for async i/o. */
PRIVATE void terminate_letter  PARAMS((int sig));
PRIVATE void remove_tildes PARAMS((char *string));

#ifdef _WINDOWS
#define system(p) xsystem(p)	/* 1998/06/05 (Fri) 21:53:30 */
#endif

#if USE_BLAT_MAILER

/*
syntax:
Blat <filename> -t <recipient> [optional switches (see below)]

<filename>    : file with the message body
-t <recipient>: recipient list (comma separated)
-s <subj>     : subject line
-f <sender>   : overrides the default sender address (must be known to server)
-i <addr>     : a 'From:' address, not necessarily known to the SMTP server.
-c <recipient>: carbon copy recipient list (comma separated)
-b <recipient>: blind carbon copy recipient list (comma separated)
-h            : displays this help.
-mime         : MIME Quoted-Printable Content-Transfer-Encoding.
-q            : supresses *all* output.
-server <addr>: overrides the default SMTP server to be used.

*/


PRIVATE char *blat_cmd(
	char *mail_cmd,
	char *filename,
	char *address,
	char *subject,
	char *ccaddr,
	char *mail_addr)
{
    static char *b_cmd;

#ifdef USE_ALT_BLAT_MAILER

    HTSprintf0(&b_cmd, "%s %s -t %s -s \"%s\" %s %s %s %s %s",
		mail_cmd,
		filename,
		address,
		subject,
		system_mail_flags,
		ccaddr? "-c" : "",
		ccaddr? ccaddr : "",
		mail_addr? (mail_addr[0]? "-f" : "") : "",
		mail_addr? (mail_addr[0]? mail_addr : "") : "");

#else /* !USE_ALT_BLAT_MAILER */

    static char bl_cmd_file[512];
    FILE *fp;
#ifdef __CYGWIN__
    char dosname[LY_MAXPATH];
#endif

    bl_cmd_file[0] = '\0';
    if ((fp = LYOpenTemp(bl_cmd_file, ".blt", "w")) == NULL) {
	HTAlert(FORM_MAILTO_FAILED);
	return NULL;
    }

#ifdef __CYGWIN__
    cygwin_conv_to_full_win32_path(filename, dosname);
    fprintf(fp, "%s\n", dosname);
#else
    fprintf(fp, "%s\n", filename);
#endif
    fprintf(fp, "-t\n%s\n", address);
    if (subject)
	fprintf(fp, "-s\n%s\n", subject);
    if (mail_addr && strlen(mail_addr) > 0) {
	fprintf(fp, "-f\n%s\n", mail_addr);
    }
    if (ccaddr && strlen(ccaddr) > 0) {
	fprintf(fp, "-c\n%s\n", ccaddr);
    }
    fclose(fp);

#ifdef __CYGWIN__
    cygwin_conv_to_full_win32_path(bl_cmd_file, dosname);
    HTSprintf0(&b_cmd, "%s \"@%s\"", mail_cmd, dosname);
#else
    HTSprintf0(&b_cmd, "%s @%s", mail_cmd, bl_cmd_file);
#endif

#endif /* USE_ALT_BLAT_MAILER */

    return b_cmd;
}

#endif /* USE_BLAT_MAILER */

/* HTUnEscape with control-code nuking */
PRIVATE void SafeHTUnEscape ARGS1(
	char *,	string)
{
     int i;
     int flg = FALSE;

     HTUnEscape(string);
     for (i=0; string[i] != '\0'; i++)
     {
	/* FIXME: this is no longer explicitly 7-bit ASCII,
	   but are there portability problems? */
	if ((!LYIsASCII(string[i])) || !isprint(string[i]))
	{
	   string[i] = '?';
	   flg = TRUE;
	}
     }
     if (flg)
	HTAlert(MAILTO_SQUASH_CTL);
}

/*
**  mailform() sends form content to the mailto address(es). - FM
*/
PUBLIC void mailform ARGS4(
	CONST char *, 	mailto_address,
	CONST char *, 	mailto_subject,
	CONST char *, 	mailto_content,
	CONST char *, 	mailto_type)
{
    static char *cmd;
    FILE *fd;
    char *address = NULL;
    char *ccaddr = NULL;
    char *keywords = NULL;
    char *cp = NULL;
    char self[80];
    char subject[80];
    char *searchpart = NULL;
    char buf[512];
    char *cp0 = NULL, *cp1 = NULL;
    int ch, len, i;
#ifdef VMS
    char *address_ptr1, *address_ptr2;
    BOOLEAN isPMDF = !strncasecomp(system_mail, "PMDF SEND", 9);
    BOOLEAN first = TRUE;
    FILE *hfd;
    char hdrfile[LY_MAXPATH];
#endif
#if !CAN_PIPE_TO_MAILER
    char *command = NULL;
    char my_tmpfile[LY_MAXPATH];
#endif

    if (!mailto_address || !mailto_content) {
	HTAlert(BAD_FORM_MAILTO);
	return;
    }
    subject[0] = '\0';
    self[0] = '\0';

    if ((cp = (char *)strchr(mailto_address,'\n')) != NULL)
	*cp = '\0';
    StrAllocCopy(address, mailto_address);

    /*
     *	Check for a ?searchpart. - FM
     */
    if ((cp = strchr(address, '?')) != NULL) {
	StrAllocCopy(searchpart, cp);
	*cp = '\0';
	cp = (searchpart + 1);
	if (*cp != '\0') {
	    /*
	     *	Seek and handle a subject=foo. - FM
	     */
	    while (*cp != '\0') {
		if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "subject=", 8))
		    break;
		cp++;
	    }
	    if (*cp) {
		cp += 8;
		if ((cp1 = strchr(cp, '&')) != NULL) {
		    *cp1 = '\0';
		}
		if (*cp) {
		    SafeHTUnEscape(subject);
		    LYstrncpy(subject, cp, 70);
		}
		if (cp1) {
		    *cp1 = '&';
		    cp1 = NULL;
		}
	    }

	    /*
	     *	Seek and handle to=address(es) fields.
	     *	Appends to address. - FM
	     */
	    cp = (searchpart + 1);
	    while (*cp != '\0') {
		if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "to=", 3)) {
		    cp += 3;
		    if ((cp1 = strchr(cp, '&')) != NULL) {
			*cp1 = '\0';
		    }
		    while (*cp == ',' || isspace((unsigned char)*cp))
			cp++;
		    if (*cp) {
			if (*address) {
			    StrAllocCat(address, ",");
			}
			StrAllocCat(address, cp);
		    }
		    if (cp1) {
			*cp1 = '&';
			cp = cp1;
			cp1 = NULL;
		    } else {
			break;
		    }
		}
		cp++;
	    }

	    /*
	     *	Seek and handle cc=address(es) fields.	Excludes
	     *	Bcc=address(es) as unsafe.  We may append our own
	     *	cc (below) as a list for the actual mailing. - FM
	     */
	    cp = (searchpart + 1);
	    while (*cp != '\0') {
		if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "cc=", 3)) {
		    cp += 3;
		    if ((cp1 = strchr(cp, '&')) != NULL) {
			*cp1 = '\0';
		    }
		    while (*cp == ',' || isspace((unsigned char)*cp))
			cp++;
		    if (*cp) {
			if (ccaddr == NULL) {
			    StrAllocCopy(ccaddr, cp);
			} else {
			    StrAllocCat(ccaddr, ",");
			    StrAllocCat(ccaddr, cp);
			}
		    }
		    if (cp1) {
			*cp1 = '&';
			cp = cp1;
			cp1 = NULL;
		    } else {
			break;
		    }
		}
		cp++;
	    }

	    /*
	     *	Seek and handle keywords=term(s) fields. - FM
	     */
	    cp = (searchpart + 1);
	    while (*cp != '\0') {
		if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "keywords=", 9)) {
		    cp += 9;
		    if ((cp1 = strchr(cp, '&')) != NULL) {
			*cp1 = '\0';
		    }
		    while (*cp == ',' || isspace((unsigned char)*cp))
			cp++;
		    if (*cp) {
			if (keywords == NULL) {
			    StrAllocCopy(keywords, cp);
			} else {
			    StrAllocCat(keywords, cp);
			    StrAllocCat(keywords, ", ");
			}
		    }
		    if (cp1) {
			*cp1 = '&';
			cp = cp1;
			cp1 = NULL;
		    } else {
			break;
		    }
		}
		cp++;
	    }
	    if (keywords != NULL) {
		if (*keywords != '\0') {
		    SafeHTUnEscape(keywords);
		} else {
		    FREE(keywords);
		}
	    }

	    FREE(searchpart);
	}
    }

    /*
     * Convert any Explorer semi-colon Internet address
     * separators to commas. - FM
     */
    cp = address;
    while ((cp1 = strchr(cp, '@')) != NULL) {
	cp1++;
	if ((cp0 = strchr(cp1, ';')) != NULL) {
	    *cp0 = ',';
	    cp1 = cp0 + 1;
	}
	cp = cp1;
    }
    if (address[(strlen(address) - 1)] == ',')
	address[(strlen(address) - 1)] = '\0';
    if (*address == '\0') {
	FREE(address);
	FREE(ccaddr);
	FREE(keywords);
	HTAlert(BAD_FORM_MAILTO);
	return;
    }
    if (ccaddr != NULL) {
	cp = ccaddr;
	while ((cp1 = strchr(cp, '@')) != NULL) {
	    cp1++;
	    if ((cp0 = strchr(cp1, ';')) != NULL) {
		*cp0 = ',';
		cp1 = cp0 + 1;
	    }
	    cp = cp1;
	}
	if (ccaddr[(strlen(ccaddr) - 1)] == ',') {
	    ccaddr[(strlen(ccaddr) - 1)] = '\0';
	}
	if (*ccaddr == '\0') {
	    FREE(ccaddr);
	}
    }

    /*
     *	Unescape the address and ccaddr fields. - FM
     */
    SafeHTUnEscape(address);
    if (ccaddr != NULL) {
	SafeHTUnEscape(ccaddr);
    }

    /*
     *	Allow user to edit the default Subject - FM
     */
    if (subject[0] == '\0') {
	if (mailto_subject && *mailto_subject) {
	    LYstrncpy(subject, mailto_subject, 70);
	} else {
	    strcpy(subject, "mailto:");
	    LYstrncpy((char*)&subject[7], address, 63);
	}
    }
    _statusline(SUBJECT_PROMPT);
    if ((ch = LYgetstr(subject, VISIBLE, 71, NORECALL)) < 0) {
	/*
	 * User cancelled via ^G. - FM
	 */
	HTInfoMsg(FORM_MAILTO_CANCELLED);
	FREE(address);
	FREE(ccaddr);
	FREE(keywords);
	return;
    }

    /*
     *	Allow user to specify a self copy via a CC:
     *	entry, if permitted. - FM
     */
    if (!LYNoCc) {
	sprintf(self, "%.79s", (personal_mail_address ?
				personal_mail_address : ""));
	self[79] = '\0';
	_statusline("Cc: ");
	if ((ch = LYgetstr(self, VISIBLE, sizeof(self), NORECALL)) < 0) {
	    /*
	     * User cancelled via ^G. - FM
	     */
	    HTInfoMsg(FORM_MAILTO_CANCELLED);
	    FREE(address);
	    FREE(ccaddr);
	    FREE(keywords);
	    return;
	}
	remove_tildes(self);
	if (ccaddr == NULL) {
	    StrAllocCopy(ccaddr, self);
	} else {
	    StrAllocCat(ccaddr, ",");
	    StrAllocCat(ccaddr, self);
	}
    }

#if CAN_PIPE_TO_MAILER
    HTSprintf0(&cmd, "%s %s", system_mail, system_mail_flags);
    if ((fd = popen(cmd, "w")) == NULL) {
	HTAlert(FORM_MAILTO_FAILED);
	FREE(address);
	FREE(ccaddr);
	FREE(keywords);
	return;
    }

    if (mailto_type && *mailto_type) {
	fprintf(fd, "Mime-Version: 1.0\n");
	fprintf(fd, "Content-Type: %s\n", mailto_type);
    }
    fprintf(fd, "To: %s\n", address);
    if (personal_mail_address && *personal_mail_address)
	fprintf(fd, "From: %s\n", personal_mail_address);
    if (ccaddr != NULL && *ccaddr != '\0')
	fprintf(fd, "Cc: %s\n", ccaddr);
    fprintf(fd, "Subject: %s\n\n", subject);
    if (keywords != NULL && *keywords != '\0')
	fprintf(fd, "Keywords: %s\n", keywords);
    _statusline(SENDING_FORM_CONTENT);
#else	/* e.g., VMS, DOSPATH */
    if ((fd = LYOpenTemp(my_tmpfile, ".txt", "w")) == NULL) {
	HTAlert(FORM_MAILTO_FAILED);
	FREE(address);
	FREE(ccaddr);
	FREE(keywords);
	return;
    }
#ifdef VMS
    if (isPMDF) {
	if ((hfd = LYOpenTemp(hdrfile, ".txt", "w")) == NULL) {
	    HTAlert(FORM_MAILTO_FAILED);
	    LYCloseTempFP(fd);
	    FREE(address);
	    FREE(ccaddr);
	    FREE(keywords);
	    return;
	}
    }
    if (isPMDF) {
	if (mailto_type && *mailto_type) {
	    fprintf(hfd, "Mime-Version: 1.0\n");
	    fprintf(hfd, "Content-Type: %s\n", mailto_type);
	    if (personal_mail_address && *personal_mail_address)
		fprintf(hfd, "From: %s\n", personal_mail_address);
	    }
    } else if (mailto_type &&
	       !strncasecomp(mailto_type, "multipart/form-data", 19)) {
	/*
	 *  Ugh!  There's no good way to include headers while
	 *  we're still using "generic" VMS MAIL, so we'll put
	 *  this in the body of the message. - FM
	 */
	fprintf(fd, "X-Content-Type: %s\n\n", mailto_type);
    }
#else	/* !VMS (DOS) */
#if USE_BLAT_MAILER
    if (mail_is_blat) {
	if (strlen(subject) > 70)
	    subject[70] = '\0';
    } else
#endif
    {
	if (mailto_type && *mailto_type) {
	    fprintf(fd, "Mime-Version: 1.0\n");
	    fprintf(fd, "Content-Type: %s\n", mailto_type);
	}
	fprintf(fd,"To: %s\n", address);
	if (personal_mail_address && *personal_mail_address)
	    fprintf(fd,"From: %s\n", personal_mail_address);
	remove_tildes(self);
	fprintf(fd,"Subject: %.70s\n\n", subject);
    }
#endif /* VMS */
#endif /* CAN_PIPE_TO_MAILER */

    /*
     *	Break up the content into lines with a maximum length of 78.
     *	If the ENCTYPE was text/plain, we have physical newlines and
     *	should take them into account.	Otherwise, the actual newline
     *	characters in the content are hex escaped. - FM
     */
    while((cp = strchr(mailto_content, '\n')) != NULL) {
	*cp = '\0';
	i = 0;
	len = strlen(mailto_content);
	while (len > 78) {
	    strncpy(buf, &mailto_content[i], 78);
	    buf[78] = '\0';
	    fprintf(fd, "%s\n", buf);
	    i += 78;
	    len = strlen(&mailto_content[i]);
	}
	fprintf(fd, "%s\n", &mailto_content[i]);
	mailto_content = (cp+1);
    }
    i = 0;
    len = strlen(mailto_content);
    while (len > 78) {
	strncpy(buf, &mailto_content[i], 78);
	buf[78] = '\0';
	fprintf(fd, "%s\n", buf);
	i += 78;
	len = strlen(&mailto_content[i]);
    }
    if (len)
	fprintf(fd, "%s\n", &mailto_content[i]);

#if CAN_PIPE_TO_MAILER
    pclose(fd);
    LYSleepMsg();
#endif /* UNIX */
#if defined(VMS) || defined(DOSPATH) || defined(SH_EX)
    LYCloseTempFP(fd);
#ifdef VMS
    /*
     *	Set the mail command. - FM
     */
    if (isPMDF) {
	/*
	 *  For PMDF, put any keywords and the subject
	 *  in the header file and close it. - FM
	 */
	if (keywords != NULL && *keywords != '\0') {
	    fprintf(hfd, "Keywords: %s\n", keywords);
	}
	fprintf(hfd, "Subject: %s\n\n", subject);
	LYCloseTempFP(hfd);
	/*
	 *  Now set up the command. - FM
	 */
	HTSprintf0(&cmd,
		"%s %s %s,%s ",
		system_mail,
		system_mail_flags,
		hdrfile,
		my_tmpfile);
    } else {
	/*
	 *  For "generic" VMS MAIL, include the subject in the
	 *  command, and ignore any keywords to minimize risk
	 *  of them making the line too long or having problem
	 *  characters. - FM
	 */
	HTSprintf0(&cmd,
		"%s %s%s/subject=\"%s\" %s ",
		system_mail,
		system_mail_flags,
		(strncasecomp(system_mail, "MAIL", 4) ? "" : "/noself"),
		subject,
		my_tmpfile);
    }
    StrAllocCopy(command, cmd);

    /*
     *	Now add all the people in the address field. - FM
     */
    address_ptr1 = address;
    do {
	if ((cp = strchr(address_ptr1, ',')) != NULL) {
	    address_ptr2 = (cp+1);
	    *cp = '\0';
	} else {
	    address_ptr2 = NULL;
	}

	/*
	 *  4 letters is arbitrarily the smallest possible mail
	 *  address, at least for lynx.  That way extra spaces
	 *  won't confuse the mailer and give a blank address.
	 */
	if (strlen(address_ptr1) > 3) {
	    if (!first) {
		StrAllocCat(command, ",");
	    }
	    HTSprintf0(&cmd, mail_adrs, address_ptr1);
	    StrAllocCat(command, cmd);
	    first = FALSE;
	}
	address_ptr1 = address_ptr2;
    } while (address_ptr1 != NULL);

    /*
     *	Now add all the people in the CC field. - FM
     */
    if (ccaddr != NULL && *ccaddr != '\0') {
	address_ptr1 = ccaddr;
	do {
	    if ((cp = strchr(address_ptr1, ',')) != NULL) {
		address_ptr2 = (cp+1);
		*cp = '\0';
	    } else {
		address_ptr2 = NULL;
	    }

	    /*
	     *	4 letters is arbitrarily the smallest possible mail
	     *	address, at least for lynx.  That way extra spaces
	     *	won't confuse the mailer and give a blank address.
	     */
	    if (strlen(address_ptr1) > 3) {
		StrAllocCat(command, ",");
		HTSprintf(&command, mail_adrs, address_ptr1);
		if (isPMDF) {
		    StrAllocCat(command, "/CC");
		}
	    }
	    address_ptr1 = address_ptr2;
	} while (address_ptr1 != NULL);
    }

    stop_curses();
    printf("%s\n\n$ %s\n\n%s", SENDING_FORM_CONTENT, command, PLEASE_WAIT);
    LYSystem(command);	/* Mail (VMS) */
    FREE(command);
    LYSleepAlert();
    start_curses();
    LYRemoveTemp(my_tmpfile);
    LYRemoveTemp(hdrfile);
#else /* DOSPATH */
#if USE_BLAT_MAILER
    if (mail_is_blat) {
	StrAllocCopy(command,
		blat_cmd(
		    system_mail,
		    my_tmpfile,
		    address,
		    subject,
		    ccaddr,
		    personal_mail_address
		)
	);
    } else
#endif
    {
	/* for sendmail.exe */
	StrAllocCopy(command, system_mail);
	StrAllocCat(command, " -t \"");
	StrAllocCat(command, address);
	StrAllocCat(command, "\" -F ");
	StrAllocCat(command, my_tmpfile);
    }

    stop_curses();
    printf("%s\n\n$ %s\n\n%s", SENDING_FORM_CONTENT, command, PLEASE_WAIT);
    LYSystem(command);	/* Mail sending form content (DOS/Windows) */
    FREE(command);
    LYSleepMsg();
    start_curses();
    LYRemoveTemp(my_tmpfile);
#endif
#endif /* VMS */

    FREE(address);
    FREE(ccaddr);
    FREE(keywords);
    return;
}

/*
**  mailmsg() sends a message to the owner of the file, if one is defined,
**  telling of errors (i.e., link not available).
*/
PUBLIC void mailmsg ARGS4(
	int,		cur,
	char *, 	owner_address,
	char *, 	filename,
	char *, 	linkname)
{
    FILE *fd, *fp;
    char *address = NULL;
    char *searchpart = NULL;
    char *cmd = NULL, *cp, *cp0, *cp1;
#ifdef ALERTMAIL
    BOOLEAN skip_parsing = FALSE;
#endif
#if !CAN_PIPE_TO_MAILER
    char *ccaddr;
    char subject[128];
    char my_tmpfile[LY_MAXPATH];
#endif
#ifdef VMS
    char *address_ptr1, *address_ptr2;
    BOOLEAN first = TRUE;
    BOOLEAN isPMDF = FALSE;
    char hdrfile[LY_MAXPATH];
    FILE *hfd;
    char *command = NULL;

    CTRACE((tfp, "mailmsg(%d, \"%s\", \"%s\", \"%s\")\n", cur,
	owner_address?owner_address:"<nil>",
	filename?filename:"<nil>",
	linkname?linkname:"<nil>"));

    if (!strncasecomp(system_mail, "PMDF SEND", 9)) {
	isPMDF = TRUE;
    }
#endif /* VMS */

#ifdef ALERTMAIL
    if (owner_address == NULL) {
	owner_address = ALERTMAIL;
	skip_parsing = TRUE;
    }
#endif

    if (owner_address == NULL || *owner_address == '\0') {
	return;
    }
    if ((cp = (char *)strchr(owner_address,'\n')) != NULL) {
#ifdef ALERTMAIL
	if (skip_parsing)
	    return;		/* invalidly defined - ignore - kw */
#else
	*cp = '\0';
#endif
    }
    if (!strncasecomp(owner_address, "lynx-dev@", 9)) {
	/*
	 *  Silently refuse sending bad link messages to lynx-dev.
	 */
	return;
    }
    StrAllocCopy(address, owner_address);

#ifdef ALERTMAIL
    /*
     *  If we are using a fixed address given by ALERTMAIL, it is
     *  supposed to already be in usable form, without URL-isms like
     *  ?-searchpart and URL-escaping.  So skip some code. - kw
     */
    if (!skip_parsing)
#endif
    {
	/*
	 *	Check for a ?searchpart. - FM
	 */
	if ((cp = strchr(address, '?')) != NULL) {
	    StrAllocCopy(searchpart, cp);
	    *cp = '\0';
	    cp = (searchpart + 1);
	    if (*cp != '\0') {
		/*
		 *	Seek and handle to=address(es) fields.
		 *	Appends to address.  We ignore any other
		 *	headers in the ?searchpart. - FM
		 */
		cp = (searchpart + 1);
		while (*cp != '\0') {
		    if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
			!strncasecomp(cp, "to=", 3)) {
			cp += 3;
			if ((cp1 = strchr(cp, '&')) != NULL) {
			    *cp1 = '\0';
			}
			while (*cp == ',' || isspace((unsigned char)*cp))
			    cp++;
			if (*cp) {
			    if (*address) {
				StrAllocCat(address, ",");
			    }
			    StrAllocCat(address, cp);
			}
			if (cp1) {
			    *cp1 = '&';
			    cp = cp1;
			    cp1 = NULL;
			} else {
			    break;
			}
		    }
		    cp++;
		}
	    }
	}

	/*
	 *	Convert any Explorer semi-colon Internet address
	 *	separators to commas. - FM
	 */
	cp = address;
	while ((cp1 = strchr(cp, '@')) != NULL) {
	    cp1++;
	    if ((cp0 = strchr(cp1, ';')) != NULL) {
		*cp0 = ',';
		cp1 = cp0 + 1;
	    }
	    cp = cp1;
	}

	/*
	 *	Unescape the address field. - FM
	 */
	SafeHTUnEscape(address);
    }

    if (address[(strlen(address) - 1)] == ',')
	address[(strlen(address) - 1)] = '\0';
    if (*address == '\0') {
	FREE(address);
	CTRACE((tfp, "mailmsg: No address in '%s'.\n", owner_address));
	return;
    }

#if CAN_PIPE_TO_MAILER
    HTSprintf0(&cmd, "%s %s", system_mail, system_mail_flags);
    if ((fd = popen(cmd, "w")) == NULL) {
	FREE(address);
	CTRACE((tfp, "mailmsg: '%s' failed.\n", cmd));
	return;
    }

    fprintf(fd, "To: %s\n", address);
    fprintf(fd, "Subject: Lynx Error in %s\n", filename);
    if (personal_mail_address != NULL && *personal_mail_address != '\0') {
	fprintf(fd, "Cc: %s\n", personal_mail_address);
    }
    fprintf(fd, "X-URL: %s\n", filename);
    fprintf(fd, "X-Mailer: Lynx, Version %s\n\n", LYNX_VERSION);
#else
    if ((fd = LYOpenTemp(my_tmpfile, ".txt", "w")) == NULL) {
	CTRACE((tfp, "mailmsg: Could not fopen '%s'.\n", my_tmpfile));
	FREE(address);
	return;
    }
#ifdef VMS
    if (isPMDF) {
	if ((hfd = LYOpenTemp(hdrfile, ".txt", "w")) == NULL) {
	    CTRACE((tfp, "mailmsg: Could not fopen '%s'.\n", hdrfile));
	    FREE(address);
	    return;
	}

	if (personal_mail_address != NULL && *personal_mail_address != '\0') {
	    fprintf(fd, "Cc: %s\n", personal_mail_address);
	}
	fprintf(fd, "X-URL: %s\n", filename);
	fprintf(fd, "X-Mailer: Lynx, Version %s\n\n", LYNX_VERSION);
    }
#else /* !VMS, e.g., DOSPATH */
    sprintf(subject, "Lynx Error in %.56s", filename);
    ccaddr = personal_mail_address;
#endif /* VMS */
#endif /* CAN_PIPE_TO_MAILER */

    fprintf(fd, gettext("The link   %s :?: %s \n"),
		links[cur].lname, links[cur].target);
    fprintf(fd, gettext("called \"%s\"\n"), links[cur].hightext);
    fprintf(fd, gettext("in the file \"%s\" called \"%s\"\n"), filename, linkname);
    fprintf(fd, "%s\n\n", gettext("was requested but was not available."));
    fprintf(fd, "%s\n\n", gettext("Thought you might want to know."));

    fprintf(fd, "%s\n", gettext("This message was automatically generated by"));
    fprintf(fd, gettext("Lynx ver. %s"), LYNX_VERSION);
    if ((LynxSigFile != NULL) &&
	(fp = fopen(LynxSigFile, TXT_R)) != NULL) {
	fputs("-- \n", fd);
	while (LYSafeGets(&cmd, fp) != NULL)
	    fputs(cmd, fd);
	fclose(fp);
    }
#if defined(UNIX) && !defined(__CYGWIN__)
    pclose(fd);
#endif /* UNIX */
#if defined(VMS) || defined(DOSPATH) || defined(WIN_EX)
    LYCloseTempFP(fd);
#ifdef VMS
    if (isPMDF) {
	/*
	 *  For PMDF, put the subject in the
	 *  header file and close it. - FM
	 */
	fprintf(hfd, "Subject: Lynx Error in %.56s\n\n", filename);
	LYCloseTempFP(hfd);
	/*
	 *  Now set up the command. - FM
	 */
	HTSprintf0(&command,
		"%s %s %s,%s ",
		system_mail,
		system_mail_flags,
		hdrfile,
		my_tmpfile);
    } else {
	/*
	 *  For "generic" VMS MAIL, include the
	 *  subject in the command. - FM
	 */
	HTSprintf0(&command,
		"%s %s/self/subject=\"Lynx Error in %.56s\" %s ",
		system_mail,
		system_mail_flags,
		filename,
		my_tmpfile);
    }
    address_ptr1 = address;
    do {
	if ((cp = strchr(address_ptr1, ',')) != NULL) {
	    address_ptr2 = (cp+1);
	    *cp = '\0';
	} else
	    address_ptr2 = NULL;

	/*
	 *  4 letters is arbitrarily the smallest possible mail
	 *  address, at least for lynx.  That way extra spaces
	 *  won't confuse the mailer and give a blank address.
	 */
	if (!first) {
	    StrAllocCat(command, ",");
	}
	HTSprintf0(&cmd, mail_adrs, address_ptr1);
	StrAllocCat(command, cmd);
	first = FALSE;
  	address_ptr1 = address_ptr2;
    } while (address_ptr1 != NULL);

    LYSystem(command);	/* VMS */
    FREE(command);
    FREE(cmd);
    LYRemoveTemp(my_tmpfile);
    if (isPMDF) {
	LYRemoveTemp(hdrfile);
    }
#else /* DOSPATH */
#if USE_BLAT_MAILER
    if (mail_is_blat)
	StrAllocCopy(cmd,
		blat_cmd(
		    system_mail,
		    my_tmpfile,
		    address,
		    subject,
		    ccaddr,
		    personal_mail_address
		)
	);
    else
#endif
	HTSprintf0(&cmd, "%s -t \"%s\" -F %s", system_mail, address, my_tmpfile);

    LYSystem(cmd);	/* Mail (DOS/Windows) */
    FREE(cmd);

    LYRemoveTemp(my_tmpfile);
#endif
#endif /* VMS */

    if (traversal) {
	FILE *ofp;

	if ((ofp = LYAppendToTxtFile(TRAVERSE_ERRORS)) == NULL) {
	    if ((ofp = LYNewTxtFile(TRAVERSE_ERRORS)) == NULL) {
		perror(NOOPEN_TRAV_ERR_FILE);
		exit_immediately(-1);
	    }
	}

	fprintf(ofp, "%s\t%s \tin %s\n",
		     links[cur].lname, links[cur].target, filename);
	fclose(ofp);
    }

    FREE(address);
    return;
}

/*
**  reply_by_mail() invokes sendmail on Unix or mail on VMS to send
**  a comment  from the users to the owner
*/
PUBLIC void reply_by_mail ARGS4(
	char *, 	mail_address,
	char *, 	filename,
	CONST char *,	title,
	CONST char *,	refid)
{
    char user_input[1000];
    FILE *fd, *fp;
    char *address = NULL;
    char *ccaddr = NULL;
    char *keywords = NULL;
    char *searchpart = NULL;
    char *body = NULL;
    char *cp = NULL, *cp0 = NULL, *cp1 = NULL;
    char *temp = NULL;
    int i, len;
    int c = 0;	/* user input */
    char my_tmpfile[LY_MAXPATH];
    char *command = NULL;
#if !CAN_PIPE_TO_MAILER
    char tmpfile2[LY_MAXPATH];
#endif
#ifndef NO_ANONYMOUS_EMAIL
    static char *personal_name = NULL;
#endif
    char subject[80];
#ifdef VMS
    char *address_ptr1 = NULL, *address_ptr2 = NULL;
    BOOLEAN first = TRUE;
    BOOLEAN isPMDF = FALSE;
    char hdrfile[LY_MAXPATH];
    FILE *hfd;

    if (!strncasecomp(system_mail, "PMDF SEND", 9)) {
	isPMDF = TRUE;
    }
#else
    char buf[4096];	/* 512 */
    char *header = NULL;
    int n;
#endif /* VMS */

    CTRACE((tfp, "reply_by_mail(\"%s\", \"%s\", \"%s\", \"%s\")\n",
	mail_address?mail_address:"<nil>",
	filename?filename:"<nil>",
	title?title:"<nil>",
	refid?refid:"<nil>"));

    term_letter = FALSE;

    if (mail_address && *mail_address) {
	StrAllocCopy(address, mail_address);
    } else {
	HTAlert(NO_ADDRESS_IN_MAILTO_URL);
	return;
    }

    if ((fd = LYOpenTemp(my_tmpfile, ".txt", "w")) == NULL) {
	HTAlert(MAILTO_URL_TEMPOPEN_FAILED);
	return;
    }
#ifdef VMS
    if (isPMDF) {
	if ((hfd = LYOpenTemp(hdrfile, ".txt", "w")) == NULL) {
	    HTAlert(MAILTO_URL_TEMPOPEN_FAILED);
	    return;
	}
    }
#endif /* VMS */
    subject[0] = '\0';

    /*
     *	Check for a ?searchpart. - FM
     */
    if ((cp = strchr(address, '?')) != NULL) {
	StrAllocCopy(searchpart, cp);
	*cp = '\0';
	cp = (searchpart + 1);
	if (*cp != '\0') {
	    /*
	     *	Seek and handle a subject=foo. - FM
	     */
	    while (*cp != '\0') {
		if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "subject=", 8))
		    break;
		cp++;
	    }
	    if (*cp) {
		cp += 8;
		if ((cp1 = strchr(cp, '&')) != NULL) {
		    *cp1 = '\0';
		}
		if (*cp) {
		    strncpy(subject, cp, 70);
		    subject[70] = '\0';
		    SafeHTUnEscape(subject);
		}
		if (cp1) {
		    *cp1 = '&';
		    cp1 = NULL;
		}
	    }

	    /*
	     *	Seek and handle to=address(es) fields.
	     *	Appends to address. - FM
	     */
	    cp = (searchpart + 1);
	    while (*cp != '\0') {
		if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "to=", 3)) {
		    cp += 3;
		    if ((cp1 = strchr(cp, '&')) != NULL) {
			*cp1 = '\0';
		    }
		    while (*cp == ',' || isspace((unsigned char)*cp))
			cp++;
		    if (*cp) {
			if (*address) {
			    StrAllocCat(address, ",");
			}
			StrAllocCat(address, cp);
		    }
		    if (cp1) {
			*cp1 = '&';
			cp = cp1;
			cp1 = NULL;
		    } else {
			break;
		    }
		}
		cp++;
	    }

	    /*
	     *	Seek and handle cc=address(es) fields.	Excludes
	     *	Bcc=address(es) as unsafe.  We may append our own
	     *	cc (below) as a list for the actual mailing. - FM
	     */
	    cp = (searchpart + 1);
	    while (*cp != '\0') {
		if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "cc=", 3)) {
		    cp += 3;
		    if ((cp1 = strchr(cp, '&')) != NULL) {
			*cp1 = '\0';
		    }
		    while (*cp == ',' || isspace((unsigned char)*cp))
			cp++;
		    if (*cp) {
			if (ccaddr == NULL) {
			    StrAllocCopy(ccaddr, cp);
			} else {
			    StrAllocCat(ccaddr, ",");
			    StrAllocCat(ccaddr, cp);
			}
		    }
		    if (cp1) {
			*cp1 = '&';
			cp = cp1;
			cp1 = NULL;
		    } else {
			break;
		    }
		}
		cp++;
	    }

	    /*
	     *	Seek and handle keywords=term(s) fields. - FM
	     */
	    cp = (searchpart + 1);
	    while (*cp != '\0') {
		if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "keywords=", 9)) {
		    cp += 9;
		    if ((cp1 = strchr(cp, '&')) != NULL) {
			*cp1 = '\0';
		    }
		    while (*cp == ',' || isspace((unsigned char)*cp))
			cp++;
		    if (*cp) {
			if (keywords == NULL) {
			    StrAllocCopy(keywords, cp);
			} else {
			    StrAllocCat(keywords, cp);
			    StrAllocCat(keywords, ", ");
			}
			StrAllocCat(keywords, cp);
		    }
		    if (cp1) {
			*cp1 = '&';
			cp = cp1;
			cp1 = NULL;
		    } else {
			break;
		    }
		}
		cp++;
	    }
	    if (keywords != NULL) {
		if (*keywords != '\0') {
		    SafeHTUnEscape(keywords);
		} else {
		    FREE(keywords);
		}
	    }

	    /*
	     *	Seek and handle body=foo fields. - FM
	     */
	    cp = (searchpart + 1);
	    while (*cp != '\0') {
		if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "body=", 5)) {
		    cp += 5;
		    if ((cp1 = strchr(cp, '&')) != NULL) {
			*cp1 = '\0';
		    }
		    if (*cp) {
			/*
			 *  Break up the value into lines with
			 *  a maximum length of 78. - FM
			 */
			StrAllocCopy(temp, cp);
			HTUnEscape(temp);
			cp0 = temp;
			while((cp = strchr(cp0, '\n')) != NULL) {
			    *cp = '\0';
			    if (cp > cp0) {
				if (*(cp - 1) == '\r') {
				    *(cp - 1) = '\0';
				}
			    }
			    i = 0;
			    len = strlen(cp0);
			    while (len > 78) {
				HTSprintf(&body, "%.78s\n", &cp0[i]);
				i += 78;
				len = strlen(&cp0[i]);
			    }
			    HTSprintf(&body, "%s\n", &cp0[i]);
			    cp0 = (cp + 1);
			}
			i = 0;
			len = strlen(cp0);
			while (len > 78) {
			    HTSprintf(&body, "%.78s\n", &cp0[i]);
			    i += 78;
			    len = strlen(&cp0[i]);
			}
			if (len) {
			    HTSprintf(&body, "%s\n", &cp0[i]);
			}
			FREE(temp);
		    }
		    if (cp1) {
			*cp1 = '&';
			cp = cp1;
			cp1 = NULL;
		    } else {
			break;
		    }
		}
		cp++;
	    }

	    FREE(searchpart);
	}
    }

    /*
     *	Convert any Explorer semi-colon Internet address
     *	separators to commas. - FM
     */
    cp = address;
    while ((cp1 = strchr(cp, '@')) != NULL) {
	cp1++;
	if ((cp0 = strchr(cp1, ';')) != NULL) {
	    *cp0 = ',';
	    cp1 = cp0 + 1;
	}
	cp = cp1;
    }
    if (address[(strlen(address) - 1)] == ',')
	address[(strlen(address) - 1)] = '\0';
    if (*address == '\0') {
	FREE(address);
	FREE(ccaddr);
	FREE(keywords);
	FREE(body);
	LYCloseTempFP(fd);		/* Close the tmpfile.  */
	LYRemoveTemp(my_tmpfile);	/* Delete the tmpfile. */
	HTAlert(NO_ADDRESS_IN_MAILTO_URL);
	return;
    }
    if (ccaddr != NULL) {
	cp = ccaddr;
	while ((cp1 = strchr(cp, '@')) != NULL) {
	    cp1++;
	    if ((cp0 = strchr(cp1, ';')) != NULL) {
		*cp0 = ',';
		cp1 = cp0 + 1;
	    }
	    cp = cp1;
	}
	if (ccaddr[(strlen(ccaddr) - 1)] == ',') {
	    ccaddr[(strlen(ccaddr) - 1)] = '\0';
	}
	if (*ccaddr == '\0') {
	    FREE(ccaddr);
	}
    }

    /*
     *	Unescape the address and ccaddr fields. - FM
     */
    SafeHTUnEscape(address);
    if (ccaddr != NULL) {
	SafeHTUnEscape(ccaddr);
    }

    /*
     *	Set the default subject. - FM
     */
    if (subject[0] == '\0' && title && *title) {
	strncpy(subject, title, 70);
	subject[70] = '\0';
    }

    /*
     *	Use ^G to cancel mailing of comment
     *	and don't let SIGINTs exit lynx.
     */
    signal(SIGINT, terminate_letter);


#ifdef VMS
    if (isPMDF || !body) {
	/*
	 *  Put the X-URL and X-Mailer lines in the hdrfile
	 *  for PMDF or my_tmpfile for VMS MAIL. - FM
	 */
	fprintf((isPMDF ? hfd : fd),
		"X-URL: %s%s\n",
		(filename && *filename) ? filename : "mailto:",
		(filename && *filename) ? "" : address);
	fprintf((isPMDF ? hfd : fd),
		"X-Mailer: Lynx, Version %s\n",LYNX_VERSION);
#ifdef NO_ANONYMOUS_EMAIL
	if (!isPMDF) {
	    fprintf(fd, "\n");
	}
#endif /* NO_ANONYMOUS_EMAIL */
    }
#else /* Unix/DOS/Windows */
    /*
     *	Put the To: line in the header.
     */
#ifndef DOSPATH
    StrAllocCopy(header, "To: ");
    StrAllocCat(header, address);
    StrAllocCat(header, "\n");
#endif

    /*
     *	Put the Mime-Version, Content-Type and
     *	Content-Transfer-Encoding in the header.
     *	This assumes that the same character set is used
     *	for composing the mail which is currently selected
     *	as display character set...
     *	Don't send a charset if we have a CJK character set
     *	selected, since it may not be appropriate for mail...
     *	Also don't use an unofficial "x-" charset.
     *	Also if the charset would be "us-ascii" (7-bit replacements
     *	selected, don't send any MIME headers. - kw
     */
    if (strncasecomp(LYCharSet_UC[current_char_set].MIMEname,
		     "us-ascii", 8) != 0) {
	StrAllocCat(header, "Mime-Version: 1.0\n");
	if (!LYHaveCJKCharacterSet &&
	    strncasecomp(LYCharSet_UC[current_char_set].MIMEname, "x-", 2)
	    != 0) {
	    HTSprintf(&header, "Content-Type: text/plain; charset=%s\n",
		    LYCharSet_UC[current_char_set].MIMEname);
	}
	StrAllocCat(header, "Content-Transfer-Encoding: 8bit\n");
    }
    /*
     *	Put the X-URL and X-Mailer lines in the header.
     */
    StrAllocCat(header, "X-URL: ");
    if (filename && *filename) {
	StrAllocCat(header, filename);
    }
    else {
	StrAllocCat(header, "mailto:");
	StrAllocCat(header, address);
    }
    StrAllocCat(header, "\n");
    HTSprintf(&header, "X-Mailer: Lynx, Version %s\n", LYNX_VERSION);

    if (refid && *refid) {
	StrAllocCat(header, "In-Reply-To: <");
	StrAllocCat(header, refid);
	StrAllocCat(header, ">\n");
    }
#endif /* VMS */

    /*
     *	Clear the screen and inform the user.
     */
    clear();
    move(2,0);
    scrollok(stdscr, TRUE);	/* Enable scrolling. */
    if (body)
	addstr(SENDING_MESSAGE_WITH_BODY_TO);
    else
	addstr(SENDING_COMMENT_TO);
    cp = address;
    while ((cp1 = strchr(cp, ',')) != NULL) {
	*cp1 = '\0';
	while (*cp == ' ')
	    cp++;
	if (*cp) {
	    addstr(cp);
	    addstr(",\n  ");
	}
	*cp1 = ',';
	cp = (cp1 + 1);
    }
    if (*cp) {
	addstr(cp);
    }
#ifdef VMS
    if ((isPMDF == TRUE) &&
	(cp = ccaddr) != NULL)
#else
    if ((cp = ccaddr) != NULL)
#endif /* VMS */
    {
	if (strchr(cp, ',') != NULL) {
	    addstr(WITH_COPIES_TO);
	} else {
	    addstr(WITH_COPY_TO);
	}
	while ((cp1 = strchr(cp, ',')) != NULL) {
	    *cp1 = '\0';
	    while (*cp == ' ')
		cp++;
	    if (*cp) {
		addstr(cp);
		addstr(",\n  ");
	    }
	    *cp1 = ',';
	    cp = (cp1 + 1);
	}
	if (*cp) {
	    addstr(cp);
	}
    }
    addstr(CTRL_G_TO_CANCEL_SEND);

#ifdef VMS
    if (isPMDF || !body) {
#endif /* VMS */
#ifndef NO_ANONYMOUS_EMAIL
    /*
     *	Get the user's personal name.
     */
    addstr(ENTER_NAME_OR_BLANK);
    if (personal_name == NULL)
	*user_input = '\0';
    else {
	addstr(CTRL_U_TO_ERASE);
	LYstrncpy(user_input, personal_name, sizeof(user_input)-1);
    }
#ifdef VMS
    if (isPMDF) {
	addstr(gettext("Personal_name: "));
    } else {
	addstr(gettext("X_Personal_name: "));
    }
#else
    addstr(gettext("Personal Name: "));
#endif /* VMS */
    if (LYgetstr(user_input, VISIBLE, sizeof(user_input), NORECALL) < 0 ||
	term_letter) {
	addstr("\n");
	HTInfoMsg(COMMENT_REQUEST_CANCELLED);
	LYCloseTempFP(fd);	/* Close the tmpfile. */
	scrollok(stdscr,FALSE); /* Stop scrolling.    */
	goto cleanup;
    }
    addstr("\n");
    remove_tildes(user_input);
    StrAllocCopy(personal_name, user_input);
    term_letter = FALSE;
    if (*user_input) {
#ifdef VMS
	fprintf((isPMDF ? hfd : fd),
		"X-Personal_name: %s\n",user_input);
#else
	StrAllocCat(header, "X-Personal_name: ");
	StrAllocCat(header, user_input);
	StrAllocCat(header, "\n");
#endif /* VMS */
    }

    /*
     *	Get the user's return address.
     */
    addstr(ENTER_MAIL_ADDRESS_OR_OTHER);
    addstr(MEANS_TO_CONTACT_FOR_RESPONSE);
    if (personal_mail_address)
	addstr(CTRL_U_TO_ERASE);
#ifdef VMS
    if (isPMDF) {
	addstr("From: ");
    } else {
	addstr("X-From: ");
    }
#else
    addstr("From: ");
#endif /* VMS */
    /* Add the personal mail address if there is one. */
    sprintf(user_input, "%.*s", (int)(sizeof(user_input) - 1),
	    (personal_mail_address
	    ? personal_mail_address
	    : ""));
    if (LYgetstr(user_input, VISIBLE, sizeof(user_input), NORECALL) < 0 ||
	term_letter) {
	addstr("\n");
	HTInfoMsg(COMMENT_REQUEST_CANCELLED);
	LYCloseTempFP(fd);	/* Close the tmpfile. */
	scrollok(stdscr,FALSE); /* Stop scrolling.    */
	goto cleanup;
    }
    addstr("\n");
    remove_tildes(user_input);
#ifdef VMS
    if (*user_input) {
	if (isPMDF) {
	    fprintf(hfd, "From: %s\n", user_input);
	} else {
	    fprintf(fd, "X-From: %s\n\n", user_input);
	}
    } else if (!isPMDF) {
	fprintf(fd, "\n");
    }
#else
    StrAllocCat(header, "From: ");
    StrAllocCat(header, user_input);
    StrAllocCat(header, "\n");
#endif /* VMS */
#endif /* !NO_ANONYMOUS_EMAIL */
#ifdef VMS
    }
#endif /* VMS */

    /*
     *	Get the subject line.
     */
    addstr(ENTER_SUBJECT_LINE);
    addstr(CTRL_U_TO_ERASE);
    addstr(SUBJECT_PROMPT);
    /* Add the default subject. */
    sprintf(user_input, "%.70s%.63s",
			(subject[0] != '\0') ?
				     subject :
		    ((filename && *filename) ?
				    filename : "mailto:"),
			(subject[0] != '\0') ?
					  "" :
		    ((filename && *filename) ?
					  "" : address));
    if (LYgetstr(user_input, VISIBLE, 71, NORECALL) < 0 ||
	term_letter) {
	addstr("\n");
	HTInfoMsg(COMMENT_REQUEST_CANCELLED);
	LYCloseTempFP(fd);	/* Close the tmpfile. */
	scrollok(stdscr,FALSE); /* Stop scrolling.    */
	goto cleanup;
    }
    addstr("\n");
    remove_tildes(user_input);
    sprintf(subject, "%.70s", user_input);
#ifndef VMS
    StrAllocCat(header, "Subject: ");
    StrAllocCat(header, user_input);
    StrAllocCat(header, "\n");
#endif /* VMS */

    /*
     *	Offer a CC line, if permitted. - FM
     */
    user_input[0] = '\0';
    if (!LYNoCc) {
	addstr(ENTER_ADDRESS_FOR_CC);
	if (personal_mail_address)
	    addstr(CTRL_U_TO_ERASE);
	addstr(BLANK_FOR_NO_COPY);
	addstr("Cc: ");
	/*
	 *  Add the mail address if there is one.
	 */
	sprintf(user_input, "%.*s", (int) (sizeof(user_input) - 1),
		(personal_mail_address
		    ? personal_mail_address
		    : ""));
	if (LYgetstr(user_input, VISIBLE, sizeof(user_input), NORECALL) < 0 ||
	    term_letter) {
	    addstr("\n");
	    HTInfoMsg(COMMENT_REQUEST_CANCELLED);
	    LYCloseTempFP(fd); 		/* Close the tmpfile. */
	    scrollok(stdscr, FALSE);	/* Stop scrolling.    */
	    goto cleanup;
	}
	addstr("\n");
    }
    remove_tildes(user_input);

    if (*user_input) {
	cp = user_input;
	while (*cp == ',' || isspace((unsigned char)*cp))
	    cp++;
	if (*cp) {
	    if (ccaddr == NULL) {
		StrAllocCopy(ccaddr, cp);
	    } else {
		StrAllocCat(ccaddr, ",");
		StrAllocCat(ccaddr, cp);
	    }
	}
    }

#if defined(DOSPATH) || defined(SH_EX)
    if (*address) {
	StrAllocCat(header, "To: ");
	StrAllocCat(header, address);
	StrAllocCat(header, "\n");
    }
#endif

#ifndef VMS
    /*
    **	Add the Cc: header. - FM
    */
    if (ccaddr != NULL && *ccaddr != '\0') {
	StrAllocCat(header, "Cc: ");
	StrAllocCat(header, ccaddr);
	StrAllocCat(header, "\n");
    }

    /*
    **	Add the Keywords: header. - FM
    */
    if (keywords != NULL && *keywords != '\0') {
	StrAllocCat(header, "Keywords: ");
	StrAllocCat(header, keywords);
	StrAllocCat(header, "\n");
    }

    /*
     *	Terminate the header.
     */
    StrAllocCat(header, "\n");
    CTRACE((tfp,"**header==\n%s",header));
#endif /* !VMS */

    if (!no_editor && editor && *editor != '\0') {
	/*
	 *  Use an external editor for the message.
	 */
	char *editor_arg = "";

	if (body) {
	    cp1 = body;
	    while((cp = strchr(cp1, '\n')) != NULL) {
		*cp++ = '\0';
		fprintf(fd, "%s\n", cp1);
		cp1 = cp;
	    }
	} else if (strcmp(HTLoadedDocumentURL(), "")) {
	    /*
	     *	Ask if the user wants to include the original message.
	     */
	    BOOLEAN is_preparsed = (BOOL) (LYPreparsedSource &&
				    HTisDocumentSource());
	    if (HTConfirm(is_preparsed
	    	? INC_PREPARSED_MSG_PROMPT
		: INC_ORIG_MSG_PROMPT) == YES) {
		/*
		 *  The 1 will add the reply "> " in front of every line.
		 */
		if (is_preparsed)
		    print_wwwfile_to_fd(fd, 0);
		else
		    print_wwwfile_to_fd(fd, 1);
	    }
	}
	LYCloseTempFP(fd);	/* Close the tmpfile. */
	scrollok(stdscr,FALSE); /* Stop scrolling.    */

	if (term_letter || c == 7 || c == 3)
	    goto cleanup;

	/*
	 *  Spawn the users editor on the mail file
	 */
	if (strstr(editor, "pico")) {
	    editor_arg = " -t"; /* No prompt for filename to use */
	}
	command = 0;
	HTSprintf0(&command, "%s%s %s", editor, editor_arg, my_tmpfile);
	_statusline(SPAWNING_EDITOR_FOR_MAIL);
	stop_curses();
	if (LYSystem(command)) {	/* Spawn Editor */
	    start_curses();
	    HTAlert(ERROR_SPAWNING_EDITOR);
	} else {
	    start_curses();
	}
	FREE(command);

    } else if (body) {
	/*
	 *  Let user review the body. - FM
	 */
	clear();
	move(0,0);
	addstr(REVIEW_MESSAGE_BODY);
	refresh();
	cp1 = body;
	i = (LYlines - 5);
	while((cp = strchr(cp1, '\n')) != NULL) {
	    if (i <= 0) {
		addstr(RETURN_TO_CONTINUE);
		refresh();
		c = LYgetch();
		addstr("\n");
		if (term_letter || c == 7 || c == 3) {
		    addstr(CANCELLED);
		    LYSleepInfo();
		    LYCloseTempFP(fd); 		/* Close the tmpfile. */
		    scrollok(stdscr, FALSE);	/* Stop scrolling.    */
		    goto cleanup;
		}
		i = (LYlines - 2);
	    }
	    *cp++ = '\0';
	    fprintf(fd, "%s\n", cp1);
	    addstr(cp1);
	    addstr("\n");
	    cp1 = cp;
	    i--;
	}
	while (i >= 0) {
	    addstr("\n");
	    i--;
	}
	refresh();
	LYCloseTempFP(fd);	/* Close the tmpfile.	  */
	scrollok(stdscr,FALSE); /* Stop scrolling.	  */

    } else {
	/*
	 *  Use the internal line editor for the message.
	 */
	addstr(ENTER_MESSAGE_BELOW);
	addstr(ENTER_PERIOD_WHEN_DONE_A);
	addstr(ENTER_PERIOD_WHEN_DONE_B);
	addstr("\n\n");
	refresh();
	*user_input = '\0';
	if (LYgetstr(user_input, VISIBLE, sizeof(user_input), NORECALL) < 0 ||
	    term_letter || STREQ(user_input, ".")) {
	    HTInfoMsg(COMMENT_REQUEST_CANCELLED);
	    LYCloseTempFP(fd); 		/* Close the tmpfile. */
	    scrollok(stdscr,FALSE);	/* Stop scrolling.    */
	    goto cleanup;
	}

	while (!STREQ(user_input, ".") && !term_letter) {
	    addstr("\n");
	    remove_tildes(user_input);
	    fprintf(fd, "%s\n", user_input);
	    *user_input = '\0';
	    if (LYgetstr(user_input, VISIBLE,
			 sizeof(user_input), NORECALL) < 0) {
		HTInfoMsg(COMMENT_REQUEST_CANCELLED);
		LYCloseTempFP(fd);	/* Close the tmpfile. */
		scrollok(stdscr,FALSE); /* Stop scrolling.    */
		goto cleanup;
	    }
	}

	fprintf(fd, "\n");	/* Terminate the message. */
	LYCloseTempFP(fd);	/* Close the tmpfile.	  */
	scrollok(stdscr,FALSE); /* Stop scrolling.	  */
    }

#ifndef VMS
    /*
     *	Ignore CTRL-C on this last question.
     */
    signal(SIGINT, SIG_IGN);
#endif /* !VMS */
    LYStatusLine = (LYlines - 1);
    c = HTConfirm (body ? SEND_MESSAGE_PROMPT : SEND_COMMENT_PROMPT);
    LYStatusLine = -1;
    if (c != YES) {
	clear();  /* clear the screen */
	goto cleanup;
    }
    if ((body == NULL && LynxSigFile != NULL) &&
	(fp = fopen(LynxSigFile, TXT_R)) != NULL) {
	LYStatusLine = (LYlines - 1);
	if (term_letter) {
	    _user_message(APPEND_SIG_FILE, LynxSigFile);
	    c = 0;
	} else {
	    char *msg = NULL;
	    HTSprintf0(&msg, APPEND_SIG_FILE, LynxSigFile);
	    c = HTConfirm(msg);
	    FREE(msg);
	}
	LYStatusLine = -1;
	if (c == YES) {
	    if ((fd = fopen(my_tmpfile, TXT_A)) != NULL) {
		char *buffer = NULL;
		fputs("-- \n", fd);
		while (LYSafeGets(&buffer, fp) != NULL) {
		    fputs(buffer, fd);
		}
		fclose(fd);
	    }
	}
	fclose(fp);
    }
    clear();  /* Clear the screen. */

    /*
     *	Send the message.
     */
#ifdef VMS
    /*
     *	Set the mail command. - FM
     */
    if (isPMDF) {
	/*
	 *  For PMDF, put any keywords and the subject
	 *  in the header file and close it. - FM
	 */
	if (keywords != NULL && *keywords != '\0') {
	    fprintf(hfd, "Keywords: %s\n", keywords);
	}
	fprintf(hfd, "Subject: %s\n\n", subject);
	LYCloseTempFP(hfd);
	/*
	 *  Now set up the command. - FM
	 */
	HTSprintf0(&command, "%s %s %s,%s ",
		system_mail,
		system_mail_flags,
		hdrfile,
		my_tmpfile);
    } else {
	/*
	 *  For "generic" VMS MAIL, include the subject in the
	 *  command, and ignore any keywords to minimize risk
	 *  of them making the line too long or having problem
	 *  characters. - FM
	 */
	HTSprintf0(&command, "%s %s%s/subject=\"%s\" %s ",
		system_mail,
		system_mail_flags,
		(strncasecomp(system_mail, "MAIL", 4) ? "" : "/noself"),
		subject,
		my_tmpfile);
    }

    /*
     *	Now add all the people in the address field. - FM
     */
    address_ptr1 = address;
    do {
	if ((cp = strchr(address_ptr1, ',')) != NULL) {
	    address_ptr2 = (cp+1);
	    *cp = '\0';
	} else {
	    address_ptr2 = NULL;
	}

	/*
	 *  4 letters is arbitrarily the smallest possible mail
	 *  address, at least for lynx.  That way extra spaces
	 *  won't confuse the mailer and give a blank address.
	 */
	if (strlen(address_ptr1) > 3) {
	    if (!first) {
		StrAllocCat(command, ",");
	    }
	    HTSprintf(&command, mail_adrs, address_ptr1);
	    first = FALSE;
	}
	address_ptr1 = address_ptr2;
    } while (address_ptr1 != NULL);

    /*
     *	Now add all the people in the CC field. - FM
     */
    if (ccaddr != NULL && *ccaddr != '\0') {
	address_ptr1 = ccaddr;
	do {
	    if ((cp = strchr(address_ptr1, ',')) != NULL) {
		address_ptr2 = (cp+1);
		*cp = '\0';
	    } else {
		address_ptr2 = NULL;
	    }

	    /*
	     *	4 letters is arbitrarily the smallest possible mail
	     *	address, at least for lynx.  That way extra spaces
	     *	won't confuse the mailer and give a blank address.
	     */
	    if (strlen(address_ptr1) > 3) {
		StrAllocCat(command, ",");
		HTSprintf(&command, mail_adrs, address_ptr1);
		if (isPMDF) {
		    StrAllocCat(command, "/CC");
		}
	    }
	    address_ptr1 = address_ptr2;
	} while (address_ptr1 != NULL);
    }

    stop_curses();
    printf("%s\n\n$ %s\n\n%s", SENDING_COMMENT, command, PLEASE_WAIT);
    LYSystem(command);	/* SENDING COMMENT (VMS) */
    FREE(command);
    LYSleepAlert();
    start_curses();
#else /* Unix/DOS/Windows */
    /*
     *	Send the tmpfile into sendmail.
     */
    _statusline(SENDING_YOUR_MSG);
#if CAN_PIPE_TO_MAILER
    signal(SIGINT, SIG_IGN);
    HTSprintf0(&command, "%s %s", system_mail, system_mail_flags);
    CTRACE((tfp, "%s\n", command));
    fp = popen(command, "w");
    if (fp == NULL) {
	HTInfoMsg(COMMENT_REQUEST_CANCELLED);
    }
    FREE(command);
#else
    if ((fp = LYOpenTemp(tmpfile2, ".txt", "w")) == NULL) {
	HTAlert(MAILTO_URL_TEMPOPEN_FAILED);
    }
#endif /* CAN_PIPE_TO_MAILER */
    if (fp != 0) {
	fd = fopen(my_tmpfile, TXT_R);
	if (fd == NULL) {
	    HTInfoMsg(COMMENT_REQUEST_CANCELLED);
#if CAN_PIPE_TO_MAILER
	    pclose(fp);
#else
	    LYCloseTempFP(fp);
#endif /* CAN_PIPE_TO_MAILER */
	} else {
#if USE_BLAT_MAILER
	    if (!mail_is_blat)
		fputs(header, fp);
#else
	    fputs(header, fp);
#endif
	    while ((n = fread(buf, 1, sizeof(buf), fd)) != 0) {
		fwrite(buf, 1, n, fp);
	    }
#if CAN_PIPE_TO_MAILER
	    pclose(fp);
#else
	    LYCloseTempFP(fp);	/* Close the tmpfile. */
#if USE_BLAT_MAILER
	    if (mail_is_blat) {
		StrAllocCopy(command,
			blat_cmd(
			    system_mail,
			    tmpfile2,
			    address,
			    subject,
			    ccaddr,
			    personal_mail_address
			)
		);
	    } else
#endif	/* SH_EX */
	    {
		StrAllocCopy(command, system_mail);
		StrAllocCat(command, " -t \"");
		StrAllocCat(command, address);
		StrAllocCat(command, "\" -F ");
		StrAllocCat(command, tmpfile2);
	    }
	    stop_curses();
	    printf("%s\n\n$ %s\n\n%s", SENDING_COMMENT, command, PLEASE_WAIT);
	    LYSystem(command);	/* SENDING COMMENT (DOS/Windows/Unix) */
	    FREE(command);
	    LYSleepMsg();
	    start_curses();
	    LYRemoveTemp(tmpfile2);	/* Delete the tmpfile. */
#endif /* CAN_PIPE_TO_MAILER */
	    fclose(fd); /* Close the tmpfile. */
	}
    }
#endif /* VMS */

    /*
     *	Come here to cleanup and exit.
     */
cleanup:
    signal(SIGINT, cleanup_sig);
    term_letter = FALSE;

#ifdef VMS
    FREE(command);
    while (LYRemoveTemp(my_tmpfile) == 0)
	;		 /* Delete the tmpfile(s). */
    if (isPMDF) {
	LYRemoveTemp(hdrfile); /* Delete the hdrfile. */
    }
#else
    FREE(header);
    LYRemoveTemp(my_tmpfile);  /* Delete the tmpfile. */
#endif /* VMS */

    FREE(address);
    FREE(ccaddr);
    FREE(keywords);
    FREE(body);
    return;
}

PRIVATE void terminate_letter ARGS1(int,sig GCC_UNUSED)
{
    term_letter = TRUE;
    /* Reassert the AST */
    signal(SIGINT, terminate_letter);
#if defined(VMS) || defined(DOSPATH) || defined(WIN_EX)
    /*
     *	Refresh the screen to get rid of the "interrupt" message.
     */
    if (!dump_output_immediately) {
	lynx_force_repaint();
	refresh();
    }
#endif /* VMS */
}

PRIVATE void remove_tildes ARGS1(char *,string)
{
   /*
    *  Change the first character to
    *  a space if it is a '~'.
    */
    if (*string == '~')
	*string = ' ';
}
