#include "HTUtils.h"
#include "tcp.h"
#include "HTAlert.h"
#include "LYCurses.h"
#include "LYSignal.h"
#include "LYUtils.h"
#include "LYClean.h"
#include "LYStrings.h"
#include "GridText.h"
#include "LYSystem.h"
#include "LYGlobalDefs.h"
#include "HTParse.h"

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
    char cmd[512], *cp, *cp0, *cp1;
    int len, i, ch, recall;
    char subject[80];
#ifdef VMS
    char tmpfile[256];
    char *address_ptr1, *address_ptr2;
    BOOLEAN first = TRUE;
#endif /* VMS */

    if (!mailto_address || !mailto_content) {
	HTAlert(BAD_FORM_MAILTO);
	return;
    }

    if ((cp = (char *)strchr(mailto_address,'\n')) != NULL)
	*cp = '\0';
    StrAllocCopy(address, mailto_address);

    /*
     *  Check for a ?searchpart with subject=foo. - FM
     */
    subject[0] = '\0';
    if ((cp = strchr(address, '?')) != NULL) {
	*cp++ = '\0';
	while (*cp != '\0' && strncasecomp(cp, "subject=", 8))
	    cp++;
	cp0 = (cp - 1);
	if ((*cp != '\0') &&
	    (*cp0 == '\0' || *cp0 == '&' || *cp0 == ';')) {
	    if ((cp1 = strchr(cp, '&')) != NULL) {
	        *cp1 = '\0';
	    } else if ((cp1 = strchr(cp, ';')) != NULL) {
	        *cp1 = '\0';
	    }
	    strncpy(subject, cp, 70);
	    subject[70] = '\0';
	    HTUnEscape(subject);
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
     *  Allow user to edit the default Subject. - FM
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
    recall = 0;
    _statusline(SUBJECT_PROMPT);
    if ((ch = LYgetstr(subject, VISIBLE, 71, recall)) < 0) {
	/*
	 * User cancelled via ^G. - FM
	 */
	_statusline(FORM_MAILTO_CANCELLED);
	sleep(InfoSecs);
	FREE(address);
	return;
    }

#ifdef UNIX
#ifdef MMDF
    sprintf(cmd, "%s -mlruxto,cc\\*",system_mail);
#else
    sprintf(cmd, "%s -t -oi", system_mail);
#endif /* MMDF */

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
    fprintf(fd,"Subject: %.70s\n\n", subject);
    _statusline(SENDING_FORM_CONTENT);
#endif /* UNIX */
#ifdef VMS
    sprintf(tmpfile,"%s%s",lynx_temp_space, "temp_mail.txt");
    if ((fd = fopen(tmpfile,"w")) == NULL) {
	HTAlert(FORM_MAILTO_FAILED);
	FREE(address);
	return;
    }
    if (mailto_type &&
        !strncasecomp(mailto_type, "multipart/form-data", 19)) {
	/*
	 *  Ugh!  There's no good way to include headers while
	 *  we're still using "generic" VMS MAIL, so we'll put
	 *  this in the body of the message. - FM
	 */
	fprintf(fd, "X-Content-Type: %s\n\n", mailto_type);
    }
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
#ifdef VMS
    fclose(fd);
    sprintf(cmd, "%s /subject=\"%.70s\" %s ",system_mail, subject,tmpfile);

    address_ptr1 = address;
    do {
	if ((cp = strchr(address_ptr1, ',')) != NULL) {
	    address_ptr2 = (cp+1);
	    *cp = '\0';
	} else
	    address_ptr2 = NULL;

	if (strlen(address) > 3) {
	    if (!first)
		strcat(cmd, ", ");  /* add a comma and a space */
	    sprintf( &cmd[strlen(cmd)], mail_adrs, address_ptr1);
	    first = FALSE;
	}

	address_ptr1 = address_ptr2;
    } while (address_ptr1 != NULL);

    stop_curses();
    printf("Sending form content:\n\n$ %s\n\nPlease wait...", cmd);
    system(cmd);
    sleep(MessageSecs);
    start_curses();
    remove(tmpfile);
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
    FILE *fd;
    char *address = NULL;
    char cmd[512], *cp, *cp0, *cp1;
    int i;
#ifdef VMS
    char tmpfile[256];
    char *address_ptr1, *address_ptr2;
    BOOLEAN first = TRUE;
#endif /* VMS */

    if ((cp = (char *)strchr(owner_address,'\n')) != NULL)
	*cp = '\0';
    StrAllocCopy(address, owner_address);

    /*
     *  Check for a ?searchpart and trim it. - FM
     */
    if ((cp = strchr(address, '?')) != NULL && strchr(cp+1, '=') != NULL)
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
#ifdef MMDF
    sprintf(cmd, "%s -mlruxto,cc\\*",system_mail);
#else
    sprintf(cmd, "%s -t -oi", system_mail);
#endif /* MMDF */

    if ((fd = popen(cmd, "w")) == NULL) {
	FREE(address);
	return;
    }

    fprintf(fd,"To: %s\n", address);
    fprintf(fd,"Subject: Lynx Error in %s\n", filename);
    fprintf(fd,"X-URL: %s\n", filename);
    fprintf(fd,"X-Mailer: Lynx, Version %s\n\n",LYNX_VERSION);
#endif /* UNIX */
#ifdef VMS
    sprintf(tmpfile,"%s%s",lynx_temp_space, "temp_mail.txt");
    if ((fd = fopen(tmpfile,"w")) == NULL) {
	FREE(address);
	return;
    }

#endif /* VMS */

    fprintf(fd, "The link   %s :?: %s \n",
    		links[cur].lname, links[cur].target);
    fprintf(fd, "called \"%s\"\n",links[cur].hightext);
    fprintf(fd, "in the file \"%s\" called \"%s\"", filename, linkname);

    fputs("\nwas requested but was not available.",fd);
    fputs("\n\nThought you might want to know.",fd);

    fputs("\n\nThis message was automatically generated by\n",fd);
    fprintf(fd,"Lynx ver. %s",LYNX_VERSION);
#ifdef UNIX
    pclose(fd);
#endif /* UNIX */
#ifdef VMS
    fclose(fd);
    sprintf(cmd, "%s /subject=\"Lynx Error in %s\" %s ",
    		 system_mail, filename, tmpfile);

    address_ptr1 = address;
    do {
	if ((cp = strchr(address_ptr1, ',')) != NULL) {
	    address_ptr2 = (cp+1);
	    *cp = '\0';
	} else
	    address_ptr2 = NULL;

	if (strlen(address) > 3) {
	    if (!first)
		strcat(cmd, ", ");  /* add a comma and a space */
	    sprintf(&cmd[strlen(cmd)], mail_adrs, address_ptr1);
	    first = FALSE;
	}

	address_ptr1 = address_ptr2;
    } while (address_ptr1 != NULL);

    system(cmd);
    remove(tmpfile);
#endif /* VMS */

    if (traversal) {
	FILE *ofp;

	if ((ofp = fopen(TRAVERSE_ERRORS,"a+")) == NULL) {
	    if ((ofp = fopen(TRAVERSE_ERRORS,"w")) == NULL) {
		perror(NOOPEN_TRAV_ERR_FILE);
		(void) signal(SIGHUP, SIG_DFL);
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
	}

	fprintf(ofp, "%s	%s 	in %s\n",
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
    FILE *fd;
    char *address = NULL, *cp, *cp0, *cp1;
    int i;
    int c = 0;  /* user input */
    char tmpfile[256], cmd[512];
    static char *personal_name = NULL;
    char subject[80];
#ifdef VMS
    char *address_ptr1 = NULL, *address_ptr2 = NULL;
    BOOLEAN first = TRUE;
#else
    char buf[512];
    char *header = NULL;
    FILE *fp;
    int n;
#endif /* VMS */

    term_letter = FALSE;

    if (mail_address && *mail_address) {
	StrAllocCopy(address, mail_address);
    } else {
	HTAlert(NO_ADDRESS_IN_MAILTO_URL);
	return;
    }

    tempname(tmpfile,NEW_FILE);
    if (((cp = strrchr(tmpfile, '.')) != NULL) &&
#ifdef VMS
	NULL == strchr(cp, ']') &&
#endif /* VMS */
	NULL == strchr(cp, '/')) {
	*cp = '\0';
	strcat(tmpfile, ".txt");
    }
    if ((fd = fopen(tmpfile,"w")) == NULL) {
	HTAlert(MAILTO_URL_TEMPOPEN_FAILED);
	return;
    }

    /*
     *  Check for a ?searchpart with subject=foo. - FM
     */
    subject[0] = '\0';
    if ((cp = strchr(address, '?')) != NULL) {
	*cp++ = '\0';
	while (*cp != '\0' && strncasecomp(cp, "subject=", 8))
	    cp++;
	cp0 = (cp - 1);
	if ((*cp != '\0') &&
	    (*cp0 == '\0' || *cp0 == '&' || *cp0 == ';')) {
	    if ((cp1 = strchr(cp, '&')) != NULL) {
	        *cp1 = '\0';
	    } else if ((cp1 = strchr(cp, ';')) != NULL) {
	        *cp1 = '\0';
	    }
	    strncpy(subject, cp, 70);
	    subject[70] = '\0';
	    HTUnEscape(subject);
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
    if (*address == '\0') {
        FREE(address);
	fclose(fd);		/* Close the tmpfile.  */
	remove(tmpfile);	/* Delete the tmpfile. */
	HTAlert(NO_ADDRESS_IN_MAILTO_URL);
	return;
    }

    /*
     *  Use ^G to cancel mailing of comment
     *  and don't let SIGINTs exit lynx.
     */
    signal(SIGINT, terminate_letter);

    
#ifdef VMS
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
#else /* Unix: */
    /*
     *  Put the To: line in the header.
     */
    sprintf(buf,"To: %s\n", address);
    StrAllocCopy(header, buf);
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
     *  Cear the screen and inform the user.
     */
    clear();
    move(2,0);
    scrollok(stdscr,TRUE);	/* Enable scrolling. */
    addstr("You are now sending a comment to:");
    addstr("\n	");
    addstr(address);
    addstr("\n\nUse Ctrl-G to cancel if you do not want to send a message\n");

#ifndef NO_ANONYMOUS_EMAIL
    /*
     *  Get the user's personal name.
     */
    addstr("\n Please enter your name, ");
    addstr("or leave it blank if you wish to remain anonymous\n");
    if (personal_name == NULL)
	*user_input = '\0';
    else {
	addstr(" Use Control-U to erase the default.\n");
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
    addstr("\n Please enter a mail address or some other\n");
    addstr(" means to contact you, if you desire a response.\n");
    if (personal_mail_address)
	addstr(" Use Control-U to erase the default.\n");
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

    /*
     *  Get the subject line.
     */
    addstr("\n Please enter a subject line.\n");
    addstr(" Use Control-U to erase the default.\n");
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

#ifndef VMS
    /*
     *  Offer a CC line.
     */
    addstr("\n Enter a mail address for a CC of your message.\n");
    if (personal_mail_address)
	addstr(" Use Control-U to erase the default.\n");
    addstr(" (Leave blank if you don't want a copy.)\n");
    addstr("Cc: ");
    /* Add the mail address if there is one. */
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
    if (*user_input) {
	sprintf(buf,"Cc: %s\n",user_input);
	StrAllocCat(header, buf);
    }

    /*
     *  Terminate the header.
     */
    sprintf(buf,"\n");
    StrAllocCat(header, buf);
#endif /* !VMS */

    if (!no_editor && editor && *editor != '\0') {
        /*
	 *  Use an external editor for the message.
	 */
	char *editor_arg = "";

	if (strcmp(HTLoadedDocumentURL(),"")) {
            /*
	     *  Ask if the user wants to include the original message.
	     */
            _statusline(INC_ORIG_MSG_PROMPT);
            c = 0;
            while (TOUPPER(c) != 'Y' && TOUPPER(c) != 'N' &&
	           !term_letter && c != 7   && c != 3)
                c = LYgetch();
            if (TOUPPER(c) == 'Y') {
                /* the 1 will add the reply ">" in front of every line */
                print_wwwfile_to_fd(fd,1);
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
	sprintf(user_input,"%s%s %s",editor,editor_arg,tmpfile);
	_statusline(SPAWNING_EDITOR_FOR_MAIL);
	stop_curses();
	if (system(user_input)) {
	    start_curses();
	    _statusline(ERROR_SPAWNING_EDITOR);
	    sleep(AlertSecs);
	} else {
	    start_curses();
	}

    } else {
        /*
	 *  Use the internal line editor for the message.
	 */
	addstr("\n Please enter your message below.");
	addstr("\n When you are done, press enter and put a single period (.)");
	addstr("\n on a line and press enter again.");
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
    _statusline(SEND_COMMENT_PROMPT);
    c = 0;
    while (TOUPPER(c) != 'Y' && TOUPPER(c) != 'N' &&
	   !term_letter && c != 7   && c != 3)
	c = LYgetch();

    clear();  /* Clear the screen. */

    if (TOUPPER(c) != 'Y') {
	goto cleanup;
    }

    /*
     *  Send the message.
     */
#ifdef VMS
    /*
     *  Set the mail command.
     */
    sprintf(cmd, "%s /subject=\"%s\" %s ", system_mail, subject, tmpfile);

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
		strcat(cmd, ", ");  /* add a comma and a space */
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
#ifdef MMDF
    sprintf(cmd, "%s -mlruxto,cc\\*",system_mail);
#else
    sprintf(cmd,"%s -t -oi", system_mail);
#endif /* MMDF */
    signal(SIGINT, SIG_IGN);
    fp = popen(cmd, "w");
    if (fp == NULL) {
	_statusline(COMMENT_REQUEST_CANCELLED);
	sleep(InfoSecs);
	goto cleanup;
    }
    fd = fopen(tmpfile, "r");
    if (fd == NULL) {
	_statusline(COMMENT_REQUEST_CANCELLED);
	sleep(InfoSecs);
	pclose(fp);
	goto cleanup;
    }
    fputs(header, fp);
    while ((n = fread(buf, 1, sizeof(buf), fd)) != 0)
	fwrite(buf, 1, n, fp);
    pclose(fp);
    fclose(fd);	/* Close the tmpfile. */

    if (TRACE)
	printf("%s\n",cmd);
#endif /* VMS */

    /*
     *  Come here to cleanup and exit.
     */
cleanup:
    signal(SIGINT, cleanup_sig);
#ifndef VMS
    FREE(header);
#endif /* !VMS */

#ifdef VMS
cleandown:
#endif /* VMS */
    term_letter = FALSE;
    remove(tmpfile);	/* Delete the tmpfile. */
    FREE(address);
    return;
}

PRIVATE void terminate_letter ARGS1(int,sig)
{
    term_letter = TRUE;
    /* Reassert the AST */
    signal(SIGINT, terminate_letter);
#ifdef VMS
    /* Refresh the screen to get rid of the "interrupt" message */
    if (!dump_output_immediately) {
	clearok(curscr, TRUE);
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
