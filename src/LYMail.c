#include "HTUtils.h"
#include "tcp.h"
#include "HTParse.h"
#include "LYGlobalDefs.h"
#include "HTAlert.h"
#include "LYCurses.h"
#include "LYSignal.h"
#include "LYUtils.h"
#include "LYClean.h"
#include "LYStrings.h"
#include "GridText.h"
#include "LYSystem.h"
#include "LYMail.h"
#ifdef EXP_CHARTRANS
#include "LYCharSets.h"  /* to get current charset for mail header */
extern BOOLEAN LYHaveCJKCharacterSet;
#endif

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

BOOLEAN term_letter;	/* Global variable for async i/o. */
PRIVATE void terminate_letter  PARAMS((int sig));
PRIVATE void remove_tildes PARAMS((char *string));

/*
**  mailform() sends form content to the mailto address(es). - FM
*/
PUBLIC void mailform ARGS4(
	char *,		mailto_address,
	char *,		mailto_subject,
	char *,		mailto_content,
	char *,		mailto_type)
{
    FILE *fd;
    char *address = NULL;
    char *searchpart = NULL;
    char *cp = NULL, *cp0 = NULL, *cp1 = NULL;
    char subject[80];
    char self[80];
    char cmd[512];
    int len, i, ch;
#if defined(VMS) || defined(DOSPATH)
    char my_tempfile[256];
    char *address_ptr1, *address_ptr2;
    BOOLEAN first = TRUE;
#endif /* VMS */

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
     *  Check for a ?searchpart. - FM
     */
    if ((cp = strchr(address, '?')) != NULL) {
	StrAllocCopy(searchpart, cp);
        *cp = '\0';
	cp = (searchpart + 1);
	if (*cp != '\0') {
	    /*
	     *  Seek and handle a subject=foo. - FM
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
		    HTUnEscape(subject);
		}
		if (cp1) {
		    *cp1 = '&';
		    cp1 = NULL;
		}
	    }

	    /*
	     *  Seek and handle cc=foo fields.  Excludes Bcc=foo
	     *  and appends to address, so we can use our own cc
	     *  field for the actual mailing. - FM
	     */
	    cp = (searchpart + 1);
	    while (*cp != '\0') {
	        if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "cc=", 3)) {
		    cp += 3;
		    if ((cp1 = strchr(cp, '&')) != NULL) {
		        *cp1 = '\0';
		    }
		    while (*cp == ',')
		        cp++;
		    if (*cp) {
			StrAllocCat(address, ",");
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

	    FREE(searchpart);
	}
    }

    /*
     *  Unescape the address field. - FM
     */
    HTUnEscape(address);

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

    /*
     *  Allow user to edit the default Subject - FM
     */
    if (subject[0] == '\0') {
        if (mailto_subject && *mailto_subject) {
	    strncpy(subject, mailto_subject, 70);
	} else {
	    strcpy(subject, "mailto:");
	    strncpy((char*)&subject[7], address, 63);
	}
	subject[70] = '\0';
    }
    _statusline(SUBJECT_PROMPT);
    if ((ch = LYgetstr(subject, VISIBLE, 71, NORECALL)) < 0) {
	/*
	 * User cancelled via ^G. - FM
	 */
	_statusline(FORM_MAILTO_CANCELLED);
	sleep(InfoSecs);
	FREE(address);
	return;
    }

    /*
     *  Allow user to specify a self copy via a CC:
     *  entry, if permitted. - FM
     */
    if (!LYNoCc) {
	sprintf(self,"%.79s", (personal_mail_address ? 
			       personal_mail_address : ""));
	self[79] = '\0';
	_statusline("Cc: ");
	if ((ch = LYgetstr(self, VISIBLE, sizeof(self), NORECALL)) < 0) {
	    /*
	     * User cancelled via ^G. - FM
	     */
	    _statusline(FORM_MAILTO_CANCELLED);
	    sleep(InfoSecs);
	    FREE(address);
	    return;
	}
    }

#if defined(VMS) || defined(DOSPATH)
    sprintf(my_tempfile, "%s%s", lynx_temp_space, "temp_mail.txt");
    if ((fd = fopen(my_tempfile,"w")) == NULL) {
	HTAlert(FORM_MAILTO_FAILED);
	FREE(address);
	return;
    }
    chmod(tmpfile, 0600);
    if (*self) {
        cp = self;
	while (*cp == ' ' || *cp == ',')
	    cp++;
	if (*cp) {
	    StrAllocCat(address, ",");
	    StrAllocCat(address, cp);
	}
    }
#ifdef VMS
    if (mailto_type &&
        !strncasecomp(mailto_type, "multipart/form-data", 19)) {
	/*
	 *  Ugh!  There's no good way to include headers while
	 *  we're still using "generic" VMS MAIL, so we'll put
	 *  this in the body of the message. - FM
	 */
	fprintf(fd, "X-Content-Type: %s\n\n", mailto_type);
    }
#else
	 if (mailto_type && *mailto_type) {
	fprintf(fd, "Mime-Version: 1.0\n");
	fprintf(fd, "Content-Type: %s\n", mailto_type);
	 }
	 fprintf(fd,"To: %s\n", address);
	 if (personal_mail_address && *personal_mail_address)
		fprintf(fd,"From: %s\n", personal_mail_address);
	 remove_tildes(self);
	 fprintf(fd,"Subject: %.70s\n\n", subject);
#endif

#else
    sprintf(cmd, "%s %s", system_mail, system_mail_flags);

    if ((fd = popen(cmd, "w")) == NULL) {
	HTAlert(FORM_MAILTO_FAILED);
	FREE(address);
	return;
    }

    if (mailto_type && *mailto_type) {
	fprintf(fd, "Mime-Version: 1.0\n");
	fprintf(fd, "Content-Type: %s\n", mailto_type);
    }
    fprintf(fd,"To: %s\n", address);
    if (personal_mail_address && *personal_mail_address)
	fprintf(fd,"From: %s\n", personal_mail_address);
    remove_tildes(self);
    if (*self)
	fprintf(fd,"Cc: %s\n", self);
    fprintf(fd,"Subject: %.70s\n\n", subject);
    _statusline(SENDING_FORM_CONTENT);
#endif /* VMS */

    /*
     *  Break up the content into lines with a maximimum length of 78.
     *  If the ENCTYPE was text/plain, we have physical newlines and
     *  should take them into account.  Otherwise, the actual newline
     *  characters in the content are hex escaped. - FM
     */
    while((cp = strchr(mailto_content, '\n')) != NULL) {
	*cp = '\0';
        i = 0;
	len = strlen(mailto_content);
	while (len > 78) {
	    strncpy(cmd, (char *)&mailto_content[i], 78);
	    cmd[78] = '\0';
	    fprintf(fd, "%s\n", cmd);
	    i += 78;
	    len = strlen((char *)&mailto_content[i]);
	}
	fprintf(fd, "%s\n", (char *)&mailto_content[i]);
	mailto_content = (cp+1);
    }
    i = 0;
    len = strlen(mailto_content);
    while (len > 78) {
	strncpy(cmd, (char *)&mailto_content[i], 78);
	cmd[78] = '\0';
	fprintf(fd, "%s\n", cmd);
	i += 78;
	len = strlen((char *)&mailto_content[i]);
    }
    if (len)
	fprintf(fd, "%s\n", (char *)&mailto_content[i]);

#ifdef UNIX
    pclose(fd);
    sleep(MessageSecs);
#endif /* UNIX */
#if defined(VMS) || defined(DOSPATH)
    fclose(fd);
#ifdef VMS
    sprintf(cmd, "%s %s/subject=\"%.70s\" %s ",
    		 system_mail,
		 (strncasecomp(system_mail, "MAIL", 4) ? "" : "/noself"),
		 subject, my_tempfile);

    address_ptr1 = address;
    do {
	if ((cp = strchr(address_ptr1, ',')) != NULL) {
	    address_ptr2 = (cp+1);
	    *cp = '\0';
	} else
	    address_ptr2 = NULL;

	if (strlen(address) > 3) {
	    if (!first)
		strcat(cmd, ",");  /* add a comma */
	    sprintf( &cmd[strlen(cmd)], mail_adrs, address_ptr1);
	    first = FALSE;
	}

	address_ptr1 = address_ptr2;
    } while (address_ptr1 != NULL);
#else
	 sprintf(cmd, "%s -t \"%s\" -F %s", system_mail, address, my_tempfile);
#endif
    stop_curses();
    printf("Sending form content:\n\n$ %s\n\nPlease wait...", cmd);
    system(cmd);
    sleep(MessageSecs);
    start_curses();
    remove(my_tempfile);
#endif /* VMS */

    FREE(address);
    return;
}

/*
**  mailmsg() sends a message to the owner of the file, if one is defined,
**  telling of errors (i.e., link not available).
*/
PUBLIC void mailmsg ARGS4(int,cur, char *,owner_address, 
		char *,filename, char *,linkname)
{
    FILE *fd, *fp;
    char *address = NULL;
    char cmd[512], *cp, *cp0, *cp1;
#if defined(VMS) || defined(DOSPATH)
    char my_tempfile[256];
    char *address_ptr1, *address_ptr2;
    BOOLEAN first = TRUE;
#endif /* VMS */

    if ((cp = (char *)strchr(owner_address,'\n')) != NULL)
	*cp = '\0';
    StrAllocCopy(address, owner_address);

    /*
     *  Check for a ?searchpart and trim it. - FM
     */
    if ((cp = strchr(address, '?')) != NULL)
	*cp = '\0';

    /*
     *  Unescape the address field. - FM
     */
    HTUnEscape(address);
	
    /*
     *  Convert any Explorer semi-colon Internet address
     *  separators to commas. - FM
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

#ifdef UNIX
    sprintf(cmd, "%s %s", system_mail, system_mail_flags);

    if ((fd = popen(cmd, "w")) == NULL) {
	FREE(address);
	return;
    }

    fprintf(fd,"To: %s\n", address);
    fprintf(fd,"Subject: Lynx Error in %s\n", filename);
    fprintf(fd,"X-URL: %s\n", filename);
    fprintf(fd,"X-Mailer: Lynx, Version %s\n\n", LYNX_VERSION);
#endif /* UNIX */
#if defined(VMS) || defined(DOSPATH)
    sprintf(my_tempfile, "%s%s", lynx_temp_space, "temp_mail.txt");
    if ((fd = fopen(my_tempfile,"w")) == NULL) {
	FREE(address);
	return;
    }
    chmod(tmpfile, 0600);
#endif /* VMS */

    fprintf(fd, "The link   %s :?: %s \n",
    		links[cur].lname, links[cur].target);
    fprintf(fd, "called \"%s\"\n", links[cur].hightext);
    fprintf(fd, "in the file \"%s\" called \"%s\"", filename, linkname);

    fputs("\nwas requested but was not available.",fd);
    fputs("\n\nThought you might want to know.",fd);

    fputs("\n\nThis message was automatically generated by\n", fd);
    fprintf(fd, "Lynx ver. %s", LYNX_VERSION);
    if ((LynxSigFile != NULL) &&
        (fp = fopen(LynxSigFile, "r")) != NULL) {
	fputs("-- \n", fd);
	while (fgets(cmd, sizeof(cmd), fp) != NULL)
	    fputs(cmd, fd);
	fclose(fp);
    }
#ifdef UNIX
    pclose(fd);
#endif /* UNIX */
#if defined(VMS) || defined(DOSPATH)
    fclose(fd);
#ifdef VMS
    sprintf(cmd, "%s /subject=\"Lynx Error in %s\" %s ",
    		 system_mail, filename, my_tempfile);

    address_ptr1 = address;
    do {
	if ((cp = strchr(address_ptr1, ',')) != NULL) {
	    address_ptr2 = (cp+1);
	    *cp = '\0';
	} else
	    address_ptr2 = NULL;

	if (strlen(address) > 3) {
	    if (!first)
		strcat(cmd, ",");  /* add a comma */
	    sprintf(&cmd[strlen(cmd)], mail_adrs, address_ptr1);
	    first = FALSE;
	}

	address_ptr1 = address_ptr2;
    } while (address_ptr1 != NULL);

#else
	 sprintf(cmd, "%s -t \"%s\" -F %s", system_mail, address, my_tempfile);
#endif
    system(cmd);
    remove(my_tempfile);
#endif /* VMS */

    if (traversal) {
	FILE *ofp;

	if ((ofp = fopen(TRAVERSE_ERRORS,"a+")) == NULL) {
	    if ((ofp = fopen(TRAVERSE_ERRORS,"w")) == NULL) {
		perror(NOOPEN_TRAV_ERR_FILE);
#ifndef NOSIGHUP
		(void) signal(SIGHUP, SIG_DFL);
#endif /* NOSIGHUP */
		(void) signal(SIGTERM, SIG_DFL);
#ifndef VMS
		(void) signal(SIGINT, SIG_DFL);
#endif /* !VMS */
#ifdef SIGTSTP
		if (no_suspend)
		    (void) signal(SIGTSTP,SIG_DFL);
#endif /* SIGTSTP */
		exit(-1);
            }
	    chmod(TRAVERSE_ERRORS, 0600);
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
PUBLIC void reply_by_mail ARGS3(
	char *,		mail_address,
	char *,		filename,
	char *,		title)
{
    char user_input[1000];
    FILE *fd, *fp;
    char *address = NULL;
    char *searchpart = NULL;
    char *body = NULL;
    char *cp = NULL, *cp0 = NULL, *cp1 = NULL;
    char *temp = NULL;
    int i, len;
    int c = 0;  /* user input */
    char my_tempfile[256], cmd[512];
#ifdef DOSPATH
	 char tmpfile2[256];
#endif
    static char *personal_name = NULL;
    char subject[80];
#ifdef VMS
    char *address_ptr1 = NULL, *address_ptr2 = NULL;
    BOOLEAN first = TRUE;
#else
    char buf[512];
    char *header = NULL;
    int n;
#endif /* VMS */

    term_letter = FALSE;

    if (mail_address && *mail_address) {
	StrAllocCopy(address, mail_address);
    } else {
	HTAlert(NO_ADDRESS_IN_MAILTO_URL);
	return;
    }

    tempname(my_tempfile,NEW_FILE);
    if (((cp = strrchr(my_tempfile, '.')) != NULL) &&
#ifdef VMS
	NULL == strchr(cp, ']') &&
#endif /* VMS */
	NULL == strchr(cp, '/')) {
	*cp = '\0';
	strcat(my_tempfile, ".txt");
    }
    if ((fd = fopen(my_tempfile,"w")) == NULL) {
	HTAlert(MAILTO_URL_TEMPOPEN_FAILED);
	return;
    }
    chmod(my_tempfile, 0600);
    subject[0] = '\0';

    /*
     *  Check for a ?searchpart. - FM
     */
    if ((cp = strchr(address, '?')) != NULL) {
	StrAllocCopy(searchpart, cp);
	*cp = '\0';
	cp = (searchpart + 1);
	if (*cp != '\0') {
	    /*
	     *  Seek and handle a subject=foo. - FM
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
		    HTUnEscape(subject);
		}
		if (cp1) {
		    *cp1 = '&';
		    cp1 = NULL;
		}
	    }

	    /*
	     *  Seek and handle cc=foo fields.  Excludes Bcc=foo
	     *  and appends to address, so we can use our own cc
	     *  field for the actual mailing. - FM
	     */
	    cp = (searchpart + 1);
	    while (*cp != '\0') {
	        if ((*(cp - 1) == '?' || *(cp - 1) == '&') &&
		    !strncasecomp(cp, "cc=", 3)) {
		    cp += 3;
		    if ((cp1 = strchr(cp, '&')) != NULL) {
		        *cp1 = '\0';
		    }
		    while (*cp == ',')
		        cp++;
		    if (*cp) {
		        StrAllocCat(address, ",");
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
	     *  Seek and handle body=foo fields. - FM
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
			 *  a maximimum length of 78. - FM
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
				strncpy(cmd, (char *)&cp0[i], 78);
				cmd[78] = '\0';
				strcat(cmd, "\n");
				StrAllocCat(body, cmd);
				i += 78;
				len = strlen((char *)&cp0[i]);
			    }
			    sprintf(cmd, "%s\n", (char *)&cp0[i]);
			    StrAllocCat(body, cmd);
			    cp0 = (cp + 1);
			}
			i = 0;
			len = strlen(cp0);
			while (len > 78) {
			    strncpy(cmd, (char *)&cp0[i], 78);
			    cmd[78] = '\0';
			    strcat(cmd, "\n");
			    StrAllocCat(body, cmd);
			    i += 78;
			    len = strlen((char *)&cp0[i]);
			}
			if (len) {
			    sprintf(cmd, "%s\n", (char *)&cp0[i]);
			    StrAllocCat(body, cmd);
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
    if (subject[0] == '\0' && title && *title) {
	strncpy(subject, title, 70);
	subject[70] = '\0';
    }

    /*
     *  Unescape the address field. - FM
     */
    HTUnEscape(address);

    /*
     *  Convert any Explorer semi-colon Internet address
     *  separators to commas. - FM
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
	FREE(body);
	fclose(fd);		/* Close the tmpfile.  */
	remove(my_tempfile);	/* Delete the tmpfile. */
	HTAlert(NO_ADDRESS_IN_MAILTO_URL);
	return;
    }

    /*
     *  Use ^G to cancel mailing of comment
     *  and don't let SIGINTs exit lynx.
     */
    signal(SIGINT, terminate_letter);

    
#ifdef VMS
    if (!body) {
	/*
	 *  Put the X-URL and X-Mailer lines in the tmpfile.
	 */
	fprintf(fd, "X-URL: %s%s\n",
		    (filename && *filename) ? filename : "mailto:",
		    (filename && *filename) ? "" : address);
	fprintf(fd,"X-Mailer: Lynx, Version %s\n",LYNX_VERSION);
#ifdef NO_ANONYMOUS_MAIL
	fprintf(fd,"\n");
#endif /* NO_ANONYMOUS_MAIL */
    }
#else /* Unix: */
    /*
     *  Put the To: line in the header.
     */
#ifndef DOSPATH
    sprintf(buf,"To: %s\n", address);
    StrAllocCopy(header, buf);
#endif

#ifdef EXP_CHARTRANS
    /*
     *  Put the Mime-Version, Content-Type and
     *  Content-Transfer-Encoding in the header.
     *  This assumes that the same character set is used
     *  for composing the mail which is currently selected
     *  as display character set...
     *  Don't send a charset if we have a CJK character set
     *  selected, since it may not be appropriate for mail...
     *  Also don't use an inofficial "x-" charset.
     *  Also if the charset would be "us-ascii" (7-bit replacements
     *  selected, don't send any MIME headers. - kw
     */
    if (strncasecomp(LYCharSet_UC[current_char_set].MIMEname,
		     "us-ascii", 8) != 0) {
	StrAllocCat(header, "Mime-Version: 1.0\n");
	if (!LYHaveCJKCharacterSet &&
	    strncasecomp(LYCharSet_UC[current_char_set].MIMEname, "x-", 2)
	    != 0) {
	    sprintf(buf,"Content-Type: text/plain; charset=%s\n",
		    LYCharSet_UC[current_char_set].MIMEname);
	    StrAllocCat(header, buf);
	}
	StrAllocCat(header, "Content-Transfer-Encoding: 8bit\n");
    }
#endif
    /*
     *  Put the X-URL and X-Mailer lines in the header.
     */
    sprintf(buf, "X-URL: %s%s\n",
    		 (filename && *filename) ? filename : "mailto:",
		 (filename && *filename) ? "" : address);
    StrAllocCat(header, buf);
    sprintf(buf,"X-Mailer: Lynx, Version %s\n",LYNX_VERSION);
    StrAllocCat(header, buf);
#endif /* VMS */

    /*
     *  Clear the screen and inform the user.
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
    if (*cp)
        addstr(cp);
    addstr(CTRL_G_TO_CANCEL_SEND);

#ifdef VMS
    if (!body) {
#endif /* VMS */
#ifndef NO_ANONYMOUS_EMAIL
    /*
     *  Get the user's personal name.
     */
    addstr(ENTER_NAME_OR_BLANK);
    if (personal_name == NULL)
	*user_input = '\0';
    else {
	addstr(CTRL_U_TO_ERASE);
	strcpy(user_input, personal_name);
    }
#ifdef VMS
    addstr("X_Personal_name: ");
#else
    addstr("Personal Name: ");
#endif /* VMS */
    if (LYgetstr(user_input, VISIBLE, sizeof(user_input), NORECALL) < 0 ||
	term_letter) {
	addstr("\n");
	_statusline(COMMENT_REQUEST_CANCELLED);
	sleep(InfoSecs);
	fclose(fd);		/* Close the tmpfile. */
	scrollok(stdscr,FALSE);	/* Stop scrolling.    */
	goto cleanup;
    }
    addstr("\n");
    remove_tildes(user_input);
    StrAllocCopy(personal_name, user_input);
    term_letter = FALSE;
    if (*user_input) {
#ifdef VMS
	fprintf(fd,"X-Personal_name: %s\n",user_input);
#else
	sprintf(buf,"X-Personal_name: %s\n",user_input);
	StrAllocCat(header, buf);
#endif /* VMS */
    }

    /*
     *  Get the user's return address.
     */
    addstr(ENTER_MAIL_ADDRESS_OR_OTHER);
    addstr(MEANS_TO_CONTACT_FOR_RESPONSE);
    if (personal_mail_address)
	addstr(CTRL_U_TO_ERASE);
#ifdef VMS
    addstr("X-From: ");
#else
    addstr("From: ");
#endif /* VMS */
    /* Add the personal mail address if there is one. */
    sprintf(user_input,"%s", (personal_mail_address ? 
			      personal_mail_address : ""));
    if (LYgetstr(user_input, VISIBLE, sizeof(user_input), NORECALL) < 0 ||
	term_letter) {
	addstr("\n");
	_statusline(COMMENT_REQUEST_CANCELLED);
	sleep(InfoSecs);
	fclose(fd);		/* Close the tmpfile. */
	scrollok(stdscr,FALSE);	/* Stop scrolling.    */
	goto cleanup;
    }
    addstr("\n");
    remove_tildes(user_input);
#ifdef VMS
    if (*user_input)
	fprintf(fd,"X-From: %s\n\n",user_input);
    else
	fprintf(fd, "\n");
#else
    sprintf(buf,"From: %s\n",user_input);
    StrAllocCat(header, buf);
#endif /* VMS */
#endif /* !NO_ANONYMOUS_EMAIL */
#ifdef VMS
    }
#endif /* VMS */

    /*
     *  Get the subject line.
     */
    addstr(ENTER_SUBJECT_LINE);
    addstr(CTRL_U_TO_ERASE);
    addstr("Subject: ");
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
	_statusline(COMMENT_REQUEST_CANCELLED);
	sleep(InfoSecs);
	fclose(fd);		/* Close the tmpfile. */
	scrollok(stdscr,FALSE);	/* Stop scrolling.    */
	goto cleanup;
    }
    addstr("\n");
    remove_tildes(user_input);
#ifdef VMS
    sprintf(subject, "%.70s", user_input);
#else
    sprintf(buf,"Subject: %s\n",user_input);
    StrAllocCat(header, buf);
#endif /* VMS */

    /*
     *  Offer a CC line, if permitted. - FM
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
	sprintf(user_input,"%s", (personal_mail_address ? 
				  personal_mail_address : ""));
	if (LYgetstr(user_input, VISIBLE, sizeof(user_input), NORECALL) < 0 ||
	    term_letter) {
	    addstr("\n");
	    _statusline(COMMENT_REQUEST_CANCELLED);
	    sleep(InfoSecs);
	    fclose(fd);			/* Close the tmpfile. */
	    scrollok(stdscr,FALSE);	/* Stop scrolling.    */
	    goto cleanup;
	}
	addstr("\n");
    }
    remove_tildes(user_input);
#if defined (VMS) || defined (DOSPATH)
    if (*user_input) {
        cp = user_input;
	while (*cp == ' ' || *cp == ',')
	    cp++;
	if (*cp) {
	    StrAllocCat(address, ",");
	    StrAllocCat(address, cp);
	}
    }
#ifdef DOSPATH
	 if (*address) {
		sprintf(buf,"To: %s\n",address);
		StrAllocCat(header, buf);
	 }
#endif

#else
    if (*user_input) {
	sprintf(buf,"Cc: %s\n",user_input);
	StrAllocCat(header, buf);
    }

    /*
     *  Terminate the header.
     */
    sprintf(buf,"\n");
    StrAllocCat(header, buf);
#endif /* VMS */

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
	     *  Ask if the user wants to include the original message.
	     */
            _statusline(INC_ORIG_MSG_PROMPT);
            c = 0;
            while (TOUPPER(c) != 'Y' && TOUPPER(c) != 'N' &&
	           !term_letter && c != 7   && c != 3)
                c = LYgetch();
            if (TOUPPER(c) == 'Y') {
                /*
		 *  The 1 will add the reply "> " in front of every line.
		 */
                print_wwwfile_to_fd(fd, 1);
	    }
        }
	fclose(fd);		/* Close the tmpfile. */
	scrollok(stdscr,FALSE);	/* Stop scrolling.    */

	if (term_letter || c == 7 || c == 3)
	    goto cleanup;

	/*
	 *  Spawn the users editor on the mail file
	 */
	if (strstr(editor, "pico")) {
	    editor_arg = " -t"; /* No prompt for filename to use */
	}
	sprintf(user_input,"%s%s %s",editor,editor_arg,my_tempfile);
	_statusline(SPAWNING_EDITOR_FOR_MAIL);
	stop_curses();
	if (system(user_input)) {
	    start_curses();
	    _statusline(ERROR_SPAWNING_EDITOR);
	    sleep(AlertSecs);
	} else {
	    start_curses();
	}

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
		    sleep(InfoSecs);
		    fclose(fd);			/* Close the tmpfile. */
		    scrollok(stdscr, FALSE);	/* Stop scrolling.    */
		    goto cleanup;
		}
		i = (LYlines - 2);
	    }
	    *cp++ = '\0';
	    sprintf(cmd, "%s\n", cp1);
	    fprintf(fd, cmd);
	    addstr(cmd);
	    cp1 = cp;
	    i--;
	}
	while (i >= 0) {
	    addstr("\n");
	    i--;
	}
	refresh();
	fclose(fd);		/* Close the tmpfile.	  */
	scrollok(stdscr,FALSE);	/* Stop scrolling.	  */

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
	    term_letter || STREQ(user_input,".")) {
	    _statusline(COMMENT_REQUEST_CANCELLED);
	    sleep(InfoSecs);
	    fclose(fd);			/* Close the tmpfile. */
	    scrollok(stdscr,FALSE);	/* Stop scrolling.    */
	    goto cleanup;
	}

	while (!STREQ(user_input,".") && !term_letter) { 
	    addstr("\n");
	    remove_tildes(user_input);
	    fprintf(fd,"%s\n",user_input);
	    *user_input = '\0';
	    if (LYgetstr(user_input, VISIBLE,
	    		 sizeof(user_input), NORECALL) < 0) {
		_statusline(COMMENT_REQUEST_CANCELLED);
		sleep(InfoSecs);
		fclose(fd);		/* Close the tmpfile. */
		scrollok(stdscr,FALSE);	/* Stop scrolling.    */
		goto cleanup;
	    }
	}

	fprintf(fd,"\n");	/* Terminate the message. */
	fclose(fd);		/* Close the tmpfile.	  */
	scrollok(stdscr,FALSE);	/* Stop scrolling.	  */
    }

#ifndef VMS
    /*
     *  Ignore CTRL-C on this last question.
     */
    signal(SIGINT, SIG_IGN);
#endif /* !VMS */
    LYStatusLine = (LYlines - 1);
    if (body)
        _statusline(SEND_MESSAGE_PROMPT);
    else
        _statusline(SEND_COMMENT_PROMPT);
    LYStatusLine = -1;
    c = 0;
    while (TOUPPER(c) != 'Y' && TOUPPER(c) != 'N' &&
	   !term_letter && c != 7   && c != 3)
	c = LYgetch();
    if (TOUPPER(c) != 'Y') {
        clear();  /* clear the screen */
	goto cleanup;
    }
    if ((body == NULL && LynxSigFile != NULL) &&
        (fp = fopen(LynxSigFile, "r")) != NULL) {
	LYStatusLine = (LYlines - 1);
	_user_message(APPEND_SIG_FILE, LynxSigFile);
	c = 0;
        LYStatusLine = -1;
	while (TOUPPER(c) != 'Y' && TOUPPER(c) != 'N' &&
	       !term_letter && c != 7   && c != 3)
	    c = LYgetch();
	if (TOUPPER(c) == 'Y') {
	    if ((fd = fopen(my_tempfile, "a")) != NULL) {
	        fputs("-- \n", fd);
	        while (fgets(user_input, sizeof(user_input), fp) != NULL) {
		    fputs(user_input, fd);
		}
		fclose(fd);
	    }
	}
	fclose(fp);
    }
    clear();  /* Clear the screen. */

    /*
     *  Send the message.
     */
#ifdef VMS
    /*
     *  Set the mail command.
     */
    sprintf(cmd, "%s %s/subject=\"%s\" %s ",
    		 system_mail,
		 (strncasecomp(system_mail, "MAIL", 4) ? "" : "/noself"),
		 subject, my_tempfile);

    /*
     *  Now add all the people in the address field.
     */
    address_ptr1 = address;
    do {
	if ((cp = strchr(address_ptr1, ',')) != NULL) {
	    address_ptr2 = (cp+1);
	    *cp = '\0';
	} else
	    address_ptr2 = NULL;

	/*
	 *  4 letters is arbitrarily the smallest posible mail
	 *  address, at least for lynx.  That way extra spaces
	 *  won't confuse the mailer and give a blank address.
	 */
	if (strlen(address_ptr1) > 3) {	
	    if (!first)
		strcat(cmd, ",");  /* add a comma */
	    sprintf( &cmd[strlen(cmd)], mail_adrs, address_ptr1);
	    first = FALSE;
	}

	address_ptr1 = address_ptr2;
    } while (address_ptr1 != NULL);

    stop_curses();
    printf("Sending your comment:\n\n$ %s\n\nPlease wait...", cmd);
    system(cmd);
    sleep(MessageSecs);
    start_curses();
    goto cleandown;
#else /* Unix: */
    /*
     *  Send the tmpfile into sendmail.  
     */
    _statusline(SENDING_YOUR_MSG);
    sprintf(cmd, "%s %s", system_mail, system_mail_flags);
#ifdef DOSPATH
	 tempname(tmpfile2,NEW_FILE);
	 if (((cp = strrchr(tmpfile2, '.')) != NULL) &&
		NULL == strchr(cp, '/')) {
			*cp = '\0';
			strcat(tmpfile2, ".txt");
	 }
	 if ((fp = fopen(tmpfile2,"w")) == NULL) {
		HTAlert(MAILTO_URL_TEMPOPEN_FAILED);
		return;
	 }
#else
    signal(SIGINT, SIG_IGN);
    fp = popen(cmd, "w");
    if (fp == NULL) {
	_statusline(COMMENT_REQUEST_CANCELLED);
	sleep(InfoSecs);
	goto cleanup;
    }
#endif /* DOSPATH */
    fd = fopen(my_tempfile, "r");
    if (fd == NULL) {
	_statusline(COMMENT_REQUEST_CANCELLED);
	sleep(InfoSecs);
	pclose(fp);
	goto cleanup;
    }
    fputs(header, fp);
    while ((n = fread(buf, 1, sizeof(buf), fd)) != 0)
	fwrite(buf, 1, n, fp);
#ifdef DOSPATH
	 sprintf(cmd, "%s -t \"%s\" -F %s", system_mail, address, tmpfile2);
	 fclose(fp);	/* Close the tmpfile. */
	 stop_curses();
	 printf("Sending your comment:\n\n$ %s\n\nPlease wait...", cmd);
	 system(cmd);
	 sleep(MessageSecs);
	 start_curses();
	 remove(tmpfile2);	/* Delete the tmpfile. */
#else
    pclose(fp);
#endif
    fclose(fd);	/* Close the tmpfile. */

    if (TRACE)
	printf("%s\n",cmd);
#endif /* VMS */

    /*
     *  Come here to cleanup and exit.
     */
cleanup:
    signal(SIGINT, cleanup_sig);
#if !defined(VMS) && !defined(DOSPATH)
    FREE(header);
#endif /* !VMS */

#if defined(VMS) || defined(DOSPATH)
cleandown:
#endif /* VMS */
    term_letter = FALSE;
    remove(my_tempfile);	/* Delete the tmpfile. */
    FREE(address);
    FREE(body);
    return;
}

PRIVATE void terminate_letter ARGS1(int,sig)
{
    term_letter = TRUE;
    /* Reassert the AST */
    signal(SIGINT, terminate_letter);
#if defined(VMS) || defined(DOSPATH)
    /* Refresh the screen to get rid of the "interrupt" message */
    if (!dump_output_immediately) {
	lynx_force_repaint();
	refresh();
    }
#endif /* VMS */
}

PRIVATE void remove_tildes ARGS1(char *,string)
{
       /* change the first character to a space if
	* it is a '~'
	*/
    if(*string == '~')
	*string = ' ';
}
