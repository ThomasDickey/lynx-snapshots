#include "HTUtils.h"
#include "tcp.h"
#include "HTAlert.h"
#include "LYCurses.h"
#include "HTAccess.h"
#include "HTParse.h"
#include "LYSignal.h"
#include "LYStructs.h"
#include "LYUtils.h"
#include "LYClean.h"
#include "LYStrings.h"
#include "LYGetFile.h"
#include "LYHistory.h"
#include "LYSystem.h"
#include "GridText.h"
#include "LYSignal.h"

#include "LYGlobalDefs.h"

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

/* global variable for async i/o */
BOOLEAN term_message;
PRIVATE void terminate_message  PARAMS((int sig));

PUBLIC int LYNewsPost ARGS2(document *,newdoc, BOOLEAN,followup)
{
	char *newsgroups=NULL;
	char user_input[1024];
	FILE *fd;
	char *tmptr;
	int c=0;  /* user input */
	char tmpfile[100];
#ifdef VMS
	FILE *fdcom;
	char tmpcom[128];
	char tmpaddress[128];
	char tmpsubject[128];
#endif /* VMS */
	char cmd[130];
        DocAddress WWWDoc;

	StrAllocCopy(newsgroups, strchr(newdoc->address,':')+1);
 	term_message=FALSE;

	/* pop previous document off of stack and load into main memory */
	LYpop(newdoc);
	WWWDoc.address = newdoc->address;
	WWWDoc.post_data = newdoc->post_data;
	WWWDoc.post_content_type = newdoc->post_content_type;
	WWWDoc.bookmark = newdoc->bookmark;
	WWWDoc.isHEAD = newdoc->isHEAD;
	WWWDoc.safe = newdoc->safe;
        if(!HTLoadAbsolute(&WWWDoc)) {
	    FREE(newsgroups);
            return(NOT_FOUND);
	}

	clear();
	move(2,0);

	tempname(tmpfile,NEW_FILE);
	if((fd = fopen(tmpfile,"w")) == NULL) {
	    HTAlert(CANNOT_OPEN_TEMP);
	    FREE(newsgroups);
	    return(NORMAL);
	}
#ifdef VMS
	sprintf(tmpcom, "%s_post", tmpfile);
	if((fdcom = fopen(tmpcom,"w")) == NULL) {
	    HTAlert(CANNOT_OPEN_COMFILE);
	    FREE(newsgroups);
	    fclose(fd);
	    remove(tmpfile);
	    return(NORMAL);
	} else {
	    fprintf(fdcom, "$ v = f$verify(1)\n$!\n");
	    fprintf(fdcom, "$! POSTing via ANU-NEWS\n$!\n");
	    fprintf(fdcom, "$ on error then goto end\n");
	    fprintf(fdcom, "$ %s/noscreen POST/noedit/noself-\n", inews_path);
	    fprintf(fdcom, " /newsgroups=\"%s\"-\n", newsgroups);
	}
#endif /* VMS */
	addstr("You will be posting to:");
	addstr("\n	");
	addstr(newsgroups);
	addch('\n');

	/* Use ^C to cancel mailing of comment */
	/* and don't let sigints exit lynx     */
        signal(SIGINT, terminate_message);

	term_message=FALSE;

	addstr("\n\n Please enter your mail address\n");
	strcpy(user_input,"From: ");
	/* add the mail address if there is one */
	if(personal_mail_address)
	    strcat(user_input,personal_mail_address);

	if (LYgetstr(user_input, VISIBLE,
		     sizeof(user_input), NORECALL) < 0 ||
	    term_message) {
            _statusline(NEWS_POST_CANCELLED);
	    sleep(InfoSecs);
	    fclose(fd);		/* close the temp file */
#ifdef VMS
	    fclose(fdcom);	/* close the command file */
#endif /* VMS */
	    goto cleanup;
	}
#ifdef VMS
	if(LYstrstr(user_input, "From:"))
	    strcpy(tmpaddress, HTStrip(strchr(user_input,':')+1));
	else
	    strcpy(tmpaddress, HTStrip(user_input));
#else
	fprintf(fd,"%s\n",user_input);
#endif /* VMS */

        addstr("\n\n Please enter a subject line\n");
        strcpy(user_input,"Subject: ");

	if (followup) {
	    /* add the default subject */
	    tmptr = newdoc->title;
	    while(isspace(*tmptr)) tmptr++;
	    if(strncasecomp(tmptr, "Re:",3))
                strcat(user_input, "Re: ");
            strcat(user_input, newdoc->title);
	}

        if (LYgetstr(user_input, VISIBLE,
		     sizeof(user_input), NORECALL) < 0 ||
	    term_message) {
            _statusline(NEWS_POST_CANCELLED);
            sleep(InfoSecs);
            fclose(fd);		/* close the temp file */
#ifdef VMS
	    fclose(fdcom);	/* close the command file */
#endif /* VMS */
            goto cleanup;
        }

#ifdef VMS
	if(LYstrstr(user_input, "Subject:"))
	    strcpy(tmpsubject, HTStrip(strchr(user_input,':')+1));
	else
	    strcpy(tmpsubject, HTStrip(user_input));
	fprintf(fdcom, " /subject=\"%s\"-\n", tmpsubject);
	fprintf(fdcom, " /headers %s\n", tmpfile);
	fprintf(fdcom, "\n%s\n", tmpaddress);
	fprintf(fdcom, "%s\n", newsgroups);
	fprintf(fdcom, "\n\n\ny\nexit\n");
	fprintf(fdcom, "$ v = 'f$verify(0)'\n$ exit\n$end:\n");
	fprintf(fdcom, "$ wait 00:00:10\n$ v = 'f$verify(0)'\n$ exit\n");
	fclose(fdcom);
#else
	fprintf(fd,"%s\n",user_input);

	/*
	 *  Add Organization: header.
	 */
	{
	    FILE *fp;
	    char *org;

	    if ((org = getenv("ORGANIZATION")) != NULL && *org != '\0') {
	        fprintf(fd, "Organization: %s\n", org);
	    } else if (fp = fopen("/etc/organization", "r")) {
	        if (fgets(user_input, sizeof(user_input), fp) != NULL)
		    fprintf(fd, "Organization: %s", user_input);
		fclose(fp);
	    }
	}

	/* add Newsgroups: summary: and Keywords: */
	fprintf(fd,"Newsgroups: %s\nSummary: \nKeywords: \n\n",newsgroups);
#endif /* VMS */

	if(!no_editor && editor && *editor != '\0') {
	    if (followup) {
	        /* ask if the user wants to include the original message */
	        _statusline(INC_ORIG_MSG_PROMPT);
	        c = 0;
	        while(TOUPPER(c) != 'Y' && TOUPPER(c) != 'N' &&
	    	      !term_message && c != 7 && c != 3)
	            c = LYgetch();
	        if(TOUPPER(c) == 'Y')
	            /* the 1 will add the reply ">" in front of every line */
	            print_wwwfile_to_fd(fd,1);
	    }

	    fclose(fd);

	    if (term_message || c == 7 || c == 3)
	        goto cleanup;

	    /* spawn the users editor on the news file */
	    sprintf(user_input,"%s %s",editor,tmpfile);
	    _statusline(SPAWNING_EDITOR_FOR_NEWS);
	    stop_curses();
	    if(system(user_input)) {
		_statusline(ERROR_SPAWNING_EDITOR);
	  	sleep(AlertSecs);
	    }
	    start_curses();

	} else {
	
	    addstr("\n\n Please enter your message below.");
	    addstr("\n When you are done, press enter and put a single period (.)");
	    addstr("\n on a line and press enter again.");
	    addstr("\n\n");
	    scrollok(stdscr,TRUE);
	    refresh();
    	    *user_input = '\0';
	    if (LYgetstr(user_input, VISIBLE,
	    		 sizeof(user_input), NORECALL) < 0 ||
	        term_message) {
	        _statusline(NEWS_POST_CANCELLED);
	        sleep(InfoSecs);
	        fclose(fd);	/* close the temp file */
	        goto cleanup;
	    }


	    while(!STREQ(user_input,".") && !term_message) { 
	       addch('\n');
	       fprintf(fd,"%s\n",user_input);
	       *user_input = '\0';
	       if (LYgetstr(user_input, VISIBLE,
	       		    sizeof(user_input), NORECALL) < 0) {
	          _statusline(NEWS_POST_CANCELLED);
	          sleep(InfoSecs);
	          fclose(fd);		/* close the temp file */
	          goto cleanup;
	       }
	    }

	    fprintf(fd,"\n");

	    fclose(fd);		/* close the temp file */
	    scrollok(stdscr,FALSE);  /* stop scrolling */
	}

	_statusline(POST_MSG_PROMPT);
	c = 0;
	while(TOUPPER(c) != 'Y' && TOUPPER(c) != 'N' &&
	      !term_message && c != 7   && c != 3)
	    c = LYgetch();

	clear();  /* clear the screen */

	if(TOUPPER(c) != 'Y') {
	   goto cleanup;
	}

#ifdef VMS
	sprintf(cmd,"@%s",tmpcom);
#else
	sprintf(cmd,"%s %s",inews_path,tmpfile);
#endif /* VMS */

        stop_curses();
	printf("Posting your message:\n\n%s\n\nPlease wait...", cmd);
	system(cmd);
	sleep(MessageSecs);
	start_curses();

	/* come here to cleanup and exit */
cleanup:
#ifndef VMS
	signal(SIGINT, cleanup_sig);
#else
	remove(tmpcom);
#endif /* !VMS */
	term_message = FALSE;

	scrollok(stdscr,FALSE);  /* stop scrolling */
#ifdef VMS
	while (remove(tmpfile) == 0)
	    ; /* loop through all versions */
#else
	remove(tmpfile);
#endif /* VMS */

	FREE(newsgroups);
	return(NORMAL);
}

PRIVATE void terminate_message ARGS1(int,sig)
{
	term_message=TRUE;
	/* Reassert the AST */
	signal(SIGINT, terminate_message);
#ifdef VMS
        /* Refresh the screen to get rid of the "interrupt" message */
	clearok(curscr, TRUE);
	refresh();
#endif /* VMS */
}

