#include "HTUtils.h"
#include "tcp.h"
#include "HTAnchor.h"       /* Anchor class */
#include "HTAccess.h"
#include "HTParse.h"
#include "LYCurses.h"
#include "GridText.h"
#include "LYGlobalDefs.h"
#include "LYUtils.h"
#include "LYCharSets.h"
#include "LYCharUtils.h"
#include "HTAlert.h"
#include "LYSignal.h"
#include "LYGetFile.h"
#include "LYPrint.h"
#include "LYHistory.h"
#include "LYStrings.h"
#include "LYClean.h"
#include "LYDownload.h"
#include "LYNews.h"
#include "LYMail.h"
#include "LYSystem.h"
#include "LYKeymap.h"
#include "LYBookmark.h"
#include "LYMap.h"
#ifdef VMS
#include "HTVMSUtils.h"
#endif /* VMS */
#ifdef DIRED_SUPPORT
#include "LYLocal.h"
#endif /* DIRED_SUPPORT */

#include "LYexit.h"
#include "LYLeaks.h"

#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
#include <syslog.h>
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

#define FREE(x) if (x) {free(x); x = NULL;}

PRIVATE int fix_http_urls PARAMS((document *doc));
extern char * WWW_Download_File;
extern BOOL redirect_post_content;
#ifdef VMS
extern BOOLEAN LYDidRename;
#endif /* VMS */


#ifdef DIRED_SUPPORT
PRIVATE char * LYSanctify ARGS1(char *, href) 
{
    int i;
    char *p, *cp, *tp;
    char address_buffer[1024];

    i = (strlen(href) - 1);
    while (i && href[i] == '/') href[i--] = '\0';

    if ((cp = (char *)strchr(href,'~')) != NULL) {
       if (!strncmp(href, "file://localhost/", 17))
	 tp = (href + 17);
       else 
	 tp = (href + 5);
       if ((cp - tp) && *(cp-1) != '/')
	 return href;
       LYstrncpy(address_buffer, href, (cp - href));
       if (address_buffer[(strlen(address_buffer) - 1)] == '/')
	 address_buffer[(strlen(address_buffer) - 1)] = '\0';
       p = (char *)Home_Dir();
       strcat(address_buffer, p);
       if (strlen(++cp))
	 strcat(address_buffer, cp);
       if (strcmp(href, address_buffer))
	 StrAllocCopy(href, address_buffer);
    }
    return href;
}
#endif /* DIRED_SUPPORT */


PUBLIC BOOLEAN getfile ARGS1(document *,doc)
{
        int url_type;
	char *cp;
	DocAddress WWWDoc;  /* a WWW absolute doc address struct */

Try_Redirected_URL:
	/* load the WWWDoc struct in case we need to use it */
	WWWDoc.address = doc->address;
        WWWDoc.post_data = doc->post_data;
        WWWDoc.post_content_type = doc->post_content_type;
        WWWDoc.bookmark = doc->bookmark;
	WWWDoc.isHEAD = doc->isHEAD;

	/* reset WWW_Download_File just in case */
	FREE(WWW_Download_File);

	/* reset redirect_post_content just in case */
	redirect_post_content = FALSE;

	if (TRACE) {
	    fprintf(stderr,"LYGetFile: getting %s\n\n",doc->address);
	}

	/* check to see if this is a universal document ID
	 * that lib WWW wants to handle
 	 *
	 * some special URL's we handle ourselves :)
	 */
	if ((url_type = is_url(doc->address)) != 0) {
		if (LYValidate && !LYPermitURL) {
		    if (!(url_type == HTTP_URL_TYPE ||
		    	  url_type == HTTPS_URL_TYPE ||
			  url_type == LYNXHIST_URL_TYPE ||
		    	  url_type == LYNXKEYMAP_URL_TYPE ||
			  url_type == LYNXIMGMAP_URL_TYPE ||
			  0==strncasecomp(WWWDoc.address, helpfilepath,
					  strlen(helpfilepath)) ||
			  0==strncasecomp(WWWDoc.address, aboutfilepath,
			  		  strlen(aboutfilepath)) ||
			  (lynxlistfile != NULL &&
			   0==strncasecomp(WWWDoc.address, lynxlistfile,
			  		  strlen(lynxlistfile))) ||
			  (lynxjumpfile != NULL &&
			   0==strncasecomp(WWWDoc.address, lynxjumpfile,
			  		  strlen(lynxjumpfile))))) {
		        _statusline(NOT_HTTP_URL_OR_ACTION);
			sleep(MessageSecs);
			return(NULLFILE);
		    }
		}
		if (traversal) {
		    /* only traverse http URLs */
		    if (url_type != HTTP_URL_TYPE &&
		        url_type != LYNXIMGMAP_URL_TYPE)
		        return(NULLFILE);
		} else if (check_realm && !LYPermitURL && !LYJumpFileURL) {
		    if (!(0==strncmp(startrealm, WWWDoc.address,
				     strlen(startrealm)) ||
			  url_type == LYNXHIST_URL_TYPE ||
		    	  url_type == LYNXKEYMAP_URL_TYPE ||
			  url_type == LYNXIMGMAP_URL_TYPE ||
			  url_type == LYNXPRINT_URL_TYPE ||
			  url_type == LYNXDOWNLOAD_URL_TYPE ||
			  url_type == MAILTO_URL_TYPE ||
			  url_type == NEWSPOST_URL_TYPE ||
			  url_type == NEWSREPLY_URL_TYPE ||
			  (!LYUserSpecifiedURL &&
			   (url_type == LYNXEXEC_URL_TYPE ||
			    url_type == LYNXPROG_URL_TYPE ||
			    url_type == LYNXCGI_URL_TYPE)) ||
			  0==strncasecomp(WWWDoc.address, helpfilepath,
					  strlen(helpfilepath)) ||
			  0==strncasecomp(WWWDoc.address, aboutfilepath,
			  		  strlen(aboutfilepath)) ||
			  (lynxlistfile != NULL &&
			   0==strncasecomp(WWWDoc.address, lynxlistfile,
			  		  strlen(lynxlistfile))) ||
			  (lynxbookfile != NULL &&
			   0==strncasecomp(WWWDoc.address, lynxbookfile,
			  		  strlen(lynxbookfile))) ||
			  (lynxjumpfile != NULL &&
			   0==strncasecomp(WWWDoc.address, lynxjumpfile,
			  		  strlen(lynxjumpfile))))) {
		        _statusline(NOT_IN_STARTING_REALM);
			sleep(MessageSecs);
			return(NULLFILE);
		    }
		}
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
		syslog(LOG_INFO|LOG_LOCAL5, "%s", doc->address);
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */
		if (url_type == UNKNOWN_URL_TYPE ||
		    url_type == AFS_URL_TYPE ||
		    url_type == PROSPERO_URL_TYPE) {
		    HTAlert(UNSUPPORTED_URL_SCHEME);
		    return(NULLFILE);

		} else if (url_type == DATA_URL_TYPE) {
		    HTAlert(UNSUPPORTED_DATA_URL);
		    return(NULLFILE);

		} else if (url_type == LYNXPRINT_URL_TYPE) {
		    return(printfile(doc));

		} else if (url_type == NEWSPOST_URL_TYPE) {

		    if (no_newspost) {
			_statusline(NEWSPOSTING_DISABLED);
			sleep(MessageSecs);
			return(NULLFILE);
		    } else {
		        BOOLEAN followup = FALSE;
		        return(LYNewsPost(doc, followup));
		    }

		} else if (url_type == NEWSREPLY_URL_TYPE) {

		    if (no_newspost) {
			_statusline(NEWSPOSTING_DISABLED);
			sleep(MessageSecs);
			return(NULLFILE);
		    } else {
		        BOOLEAN followup = TRUE;
		        return(LYNewsPost(doc, followup));
		    }

		} else if (url_type == LYNXDOWNLOAD_URL_TYPE) {
		    LYDownload(doc->address);
#ifdef VMS
		    if (LYDidRename) {
		        /*
			 * The temporary file was saved to disk via a
			 * rename(), so we can't access the temporary
			 * file again via the download menu.  Clear the
			 * flag, and return NULLFILE to pop. - FM
			 */
		        LYDidRename = FALSE;
		        return(NULLFILE);
		    } else {
		        return(NORMAL);
		    }
#else
		    return(NORMAL);
#endif /* VMS */
		} else if (url_type == LYNXDIRED_URL_TYPE) {
#ifdef DIRED_SUPPORT
		    if (no_dired_support) {
		       _statusline(DIRED_DISABLED);
		       sleep(MessageSecs);
		       return(NULLFILE);
		    } else {
		       local_dired(doc);
		       WWWDoc.address = doc->address;
        	       WWWDoc.post_data = doc->post_data;
        	       WWWDoc.post_content_type = doc->post_content_type;
		       WWWDoc.bookmark = doc->bookmark;
		       WWWDoc.isHEAD = doc->isHEAD;

		       if (!HTLoadAbsolute(&WWWDoc))
		           return(NOT_FOUND);
		       return(NORMAL);
		    }
#else
		    _statusline(DIRED_DISABLED);
		    sleep(MessageSecs);
		    return(NULLFILE);
#endif /* DIRED_SUPPORT */

		} else if (url_type == LYNXHIST_URL_TYPE) {
		    /*
		     *  'doc' will change to the new file
		     *  if we had a successful pop. - FM
		     */
		    historytarget(doc);
		    if (!doc || !doc->address) {
		        HTMLSetCharacterHandling(current_char_set);
		        return(NOT_FOUND);
		    }

		    /*
		     *  We changed it so reload.
		     */
		    WWWDoc.address = doc->address;
        	    WWWDoc.post_data = doc->post_data;
        	    WWWDoc.post_content_type = doc->post_content_type;
		    WWWDoc.bookmark = doc->bookmark;
		    WWWDoc.isHEAD = doc->isHEAD;

		    if (!HTLoadAbsolute(&WWWDoc)) {
		        HTMLSetCharacterHandling(current_char_set);
		        return(NOT_FOUND);
		    }
		    HTMLSetCharacterHandling(current_char_set);
		    return(NORMAL);

		} else if (url_type == LYNXEXEC_URL_TYPE ||
			   url_type == LYNXPROG_URL_TYPE) {
#ifdef EXEC_LINKS
		    if (no_exec &&
		        !exec_ok(HTLoadedDocumentURL(),
				 doc->address+9, ALWAYS_EXEC_PATH)) {
            	        statusline(EXECUTION_DISABLED);
            		sleep(MessageSecs);
		    } else if (no_bookmark_exec &&
		    	       HTLoadedDocumentBookmark()) {
			statusline(BOOKMARK_EXEC_DISABLED);
			sleep(MessageSecs);
		    } else if (local_exec || (local_exec_on_local_files &&
			       exec_ok(HTLoadedDocumentURL(),
				       doc->address+9, EXEC_PATH))) {

			char *p, addressbuf[1024];

			/* Bug puts slash on end if none is in the string */
			char *last_slash = strrchr(doc->address,'/');
			if (last_slash-doc->address==strlen(doc->address)-1)
			    doc->address[strlen(doc->address)-1] = '\0';

			p = doc->address;
			/** Convert '~' to $HOME **/
			if ((cp = strchr(doc->address, '~'))) {
			    strncpy(addressbuf, doc->address, cp-doc->address);
			    addressbuf[cp - doc->address] = '\0';
#ifdef VMS
			    p = HTVMS_wwwName((char *)Home_Dir());
#else
			    p = (char *)Home_Dir();
#endif /* VMS */
			    strcat(addressbuf, p);
			    strcat(addressbuf, cp+1);
			    p = addressbuf;
			}
			/* Show URL before executing it */
			statusline(doc->address);
			sleep(InfoSecs);
			stop_curses();
			/* run the command */
			if (strstr(p,"//") == p+9)
			    system(p+11);
			else
			    system(p+9);
			if (url_type != LYNXPROG_URL_TYPE) {
			    /* Make sure user gets to see screen output */
#ifndef VMS
			    signal(SIGINT, SIG_IGN);
#endif /* !VMS */
			    printf("\n%s", RETURN_TO_LYNX);
			    fflush(stdout);
			    LYgetch();
#ifdef VMS
			    {
			      extern BOOLEAN HadVMSInterrupt;
			      HadVMSInterrupt = FALSE;
			    }
#endif /* VMS */
			}
			start_curses();
			
           	     } else {
			char buf[512];

                	sprintf(buf,
				EXECUTION_DISABLED_FOR_FILE,
				key_for_func(LYK_OPTIONS));
			_statusline(buf);
                	sleep(AlertSecs);
		     }
#else /* no exec_links */
		     _statusline(EXECUTION_NOT_COMPILED);
		     sleep(MessageSecs);
#endif /* EXEC_LINKS */
                     return(NULLFILE);

		} else if (url_type == MAILTO_URL_TYPE) {
		    if (no_mail) {
		    	_statusline(MAIL_DISABLED);
		    	sleep(MessageSecs);
		    } else {
			HTParentAnchor *tmpanchor;
			CONST char *title;

			title = "";
			if ((tmpanchor = HTAnchor_parent(
						HTAnchor_findAddress(&WWWDoc)
				    			)) != NULL) {
			    if (HTAnchor_title(tmpanchor)) {
			        title = HTAnchor_title(tmpanchor);
			    }
			}
		        cp = (char *)strchr(doc->address,':')+1;
		        reply_by_mail(cp,
				      ((HTMainAnchor && !LYUserSpecifiedURL) ?
				       (char *)HTMainAnchor->address :
				       (char *)doc->address),
				      (char *)title);
		    }
		    return(NULLFILE);
		
		  /* from here on we could have a remote host,
		   * so check if that's allowed.
		   */
		} else if (local_host_only &&
			   url_type != NEWS_URL_TYPE &&
			   url_type != LYNXKEYMAP_URL_TYPE &&
			   url_type != LYNXIMGMAP_URL_TYPE &&
			   url_type != LYNXCGI_URL_TYPE &&
			   !(LYisLocalHost(doc->address) ||
			     LYisLocalAlias(doc->address))) {
		    statusline(ACCESS_ONLY_LOCALHOST);
		    sleep(MessageSecs);
		    return(NULLFILE);

		  /* disable www telnet access if not telnet_ok */
		} else if (url_type == TELNET_URL_TYPE || 
			   url_type == TN3270_URL_TYPE ||
			   url_type == TELNET_GOPHER_URL_TYPE) {

		    if (!telnet_ok) {
		    	_statusline(TELNET_DISABLED);
		    	sleep(MessageSecs);
		    } else if (no_telnet_port && strchr(doc->address+7, ':')) {
			statusline(TELNET_PORT_SPECS_DISABLED);
			sleep(MessageSecs);
		    } else {
			stop_curses();
                        HTLoadAbsolute(&WWWDoc);
			start_curses();
                        fflush(stdout);

		    }

		    return(NULLFILE);

  		/* disable www news access if not news_ok */
                } else if (url_type == NEWS_URL_TYPE && !news_ok) {
                    _statusline(NEWS_DISABLED);
                    sleep(MessageSecs);
                    return(NULLFILE);

		} else if (url_type == RLOGIN_URL_TYPE) {

		    if (!rlogin_ok) {
			statusline(RLOGIN_DISABLED);
			sleep(MessageSecs);
		    } else {
			stop_curses();
			HTLoadAbsolute(&WWWDoc);
			fflush(stdout);
			start_curses();
		    }
		    return(NULLFILE);

		/*
		 *  If its a gopher index type and there isn't a search
		 *  term already attached then do this.  Otherwise
   		 *   just load it!
		 */
		} else if (url_type == INDEX_GOPHER_URL_TYPE &&
					strchr(doc->address,'?') == NULL) {
		    int status;
		    /*
		     *  Make sure we don't have a gopher+ escaped tab
		     *  instead of a gopher0 question mark delimiting
		     *  the search term. - FM
		     */
		    if ((cp = strstr(doc->address, "%09")) != NULL) {
		        char *temp = NULL;
		        *cp = '\0';
			StrAllocCopy(temp, doc->address);
			cp += 3;
			if (*cp && strncmp(cp, "%09", 3)) {
			    StrAllocCat(temp, "?");
			    StrAllocCat(temp, cp);
			    if ((cp = strstr(temp, "%09")) != NULL) {
			        *cp = '\0';
			    }
			}
			StrAllocCopy(doc->address, temp);
			FREE(temp);
			goto Try_Redirected_URL;
		    }
		    /*
		     *  Load it because the do_www_search routine
		     *  uses the base url of the currently loaded
		     *  document :(
		     */
		    if (!HTLoadAbsolute(&WWWDoc))
			return(NOT_FOUND);
		    status = do_www_search(doc);
		    if (status == NULLFILE) {
			LYpop(doc);
			WWWDoc.address = doc->address;
			WWWDoc.post_data = doc->post_data;
			WWWDoc.post_content_type = doc->post_content_type;
			WWWDoc.bookmark = doc->bookmark;
			WWWDoc.isHEAD = doc->isHEAD;
		        status = HTLoadAbsolute(&WWWDoc);
		    }
		    return(status); 

		} else {

		    if (url_type == FTP_URL_TYPE && !ftp_ok) {
			statusline(FTP_DISABLED);
			sleep(MessageSecs);
			return(NULLFILE);
		    }

		    if (url_type == HTML_GOPHER_URL_TYPE) {
		        char *tmp=NULL;
		       /*
		        * If tuple's Path=GET%20/... convert to an http URL
		        */ 
		        if ((cp=strchr(doc->address+9, '/')) != NULL &&
		           0==strncmp(++cp, "hGET%20/", 8)) {
			    StrAllocCopy(tmp, "http://");
			    if (TRACE)
			        fprintf(stderr,
					"LYGetFile: URL %s\nchanged to ",
					doc->address);
			    *cp = '\0';
			    StrAllocCat(tmp, doc->address+9);
			   /*
			    * If the port is defaulted, it should stay 70
			    */
			    if (strchr(tmp+6, ':') == NULL) {
			        StrAllocCat(tmp, "70/");
				tmp[strlen(tmp)-4] = ':';
			    }
			    if (strlen(cp+7) > 1)
			        StrAllocCat(tmp, cp+8);
			    StrAllocCopy(doc->address, tmp);
			    if (TRACE)
			        fprintf(stderr, "%s\n",doc->address);
			    FREE(tmp);
			    url_type = HTTP_URL_TYPE;
		        }
		    }
		    if (url_type == HTTP_URL_TYPE ||
			url_type == HTTPS_URL_TYPE ||
		        url_type == FTP_URL_TYPE ||
			url_type == CSO_URL_TYPE)
			fix_http_urls(doc);
		    WWWDoc.address = doc->address;  /* possible reload */

#ifdef DIRED_SUPPORT
		    lynx_edit_mode = FALSE;
		    if (url_type == FILE_URL_TYPE) {
		        doc->address = LYSanctify(doc->address);
		        WWWDoc.address = doc->address;
		    }
#else
		    if (url_type == FILE_URL_TYPE &&
		        (cp=strstr(doc->address, "/~")) != NULL) {
			char *temp=NULL;
			*cp = '\0';
			cp += 2;
			StrAllocCopy(temp, doc->address);
#ifdef VMS
			StrAllocCat(temp, HTVMS_wwwName((char *)Home_Dir()));
#else
			StrAllocCat(temp, Home_Dir());
#endif /* VMS */
			if (*cp)
			    StrAllocCat(temp, cp);
			StrAllocCopy(doc->address, temp);
			FREE(temp);
			WWWDoc.address = doc->address;
		    }
#endif /* DIRED_SUPPORT */
		    if (TRACE)
		        sleep(MessageSecs);
		    user_message(WWW_WAIT_MESSAGE,doc->address);
#ifdef NOTDEFINED
		    sleep(InfoSecs);
#endif /* NOTDEFINED */
		    if (TRACE) {
#ifdef USE_SLANG
			if (LYCursesON) {
			    addstr("*\n");
			    refresh();
			}
#endif /* USE_SLANG */
		        fprintf(stderr,"\n");
		    }
		    if (!HTLoadAbsolute(&WWWDoc)) {
			/*
			 *  Check for redirection.
			 */
			if (use_this_url_instead != NULL) {
			    if (!is_url(use_this_url_instead)) {
			        /*
				 *  The server did not return a complete
				 *  URL in its Location: header, probably
				 *  due to a FORM or other CGI script written
				 *  by someone who doesn't know that the http
				 *  protocol requires that it be a complete
				 *  URL, or using a server which does not treat
				 *  such a redirect string from the script as
				 *  an instruction to resolve it versus the
				 *  initial request, check authentication with
				 *  that URL, and then act on it without
				 *  returning redirection to us.  We'll
				 *  violate the http protocol and resolve it
				 *  ourselves using the URL of the original
				 *  request as the BASE, rather than doing
				 *  the RIGHT thing and returning an invalid
				 *  address message. - FM
				 */
			        char *temp = HTParse(use_this_url_instead,
						     WWWDoc.address,
						     PARSE_ALL);
				if (temp && *temp) {
				    StrAllocCopy(use_this_url_instead, temp);
				}
				FREE(temp);
			    }
			    HTMLSetCharacterHandling(current_char_set);
			    if(TRACE)
			        sleep(MessageSecs);
			    _user_message("Using %s", use_this_url_instead);
			    sleep(InfoSecs);
			    if (TRACE)
			        fprintf(stderr,"\n");
			    StrAllocCopy(doc->address,
					use_this_url_instead);
			    FREE(use_this_url_instead);
			    if (redirect_post_content == FALSE) {
			        /*
				 *  Freeing the content also yields
				 *  a GET request. - FM
				 */
			        FREE(doc->post_data);
			        FREE(doc->post_content_type);
			    }
			    /*
			     *  Go to top to check for URL's which get
			     *  special handling and/or security checks
			     *  in Lynx. - FM
			     */
			    goto Try_Redirected_URL;
			}
			HTMLSetCharacterHandling(current_char_set);
		        return(NOT_FOUND);
		    }

		    lynx_mode = NORMAL_LYNX_MODE;

		    /* some URL's don't actually return a document
		     * compare doc->address with the document that is 
		     * actually loaded and return NULL if not
		     * loaded.  If www_search_result is not -1
		     * then this is a reference to a named anchor
		     * within the same document.  Do NOT return
		     * NULL
		     */
                    {
                        char *pound;
                        /*
			 *  Check for #selector.
			 */
                        pound = (char *)strchr(doc->address, '#');

			/*
			 *  Check to see if there is a temp
			 *  file waiting for us to download.
			 */
			if (WWW_Download_File) {
			    HTParentAnchor *tmpanchor;
			    char *fname = NULL;

			    HTMLSetCharacterHandling(current_char_set);
			    /*
			     *  Check for a suggested filename from
			     *  the Content-Dispostion header. - FM
			     */
			    if (((tmpanchor = HTAnchor_parent(
						HTAnchor_findAddress(&WWWDoc)
							     )) != NULL) &&
				HTAnchor_SugFname(tmpanchor) != NULL) {
				StrAllocCopy(fname,
					     HTAnchor_SugFname(tmpanchor));
			    } else {
			        StrAllocCopy(fname, doc->address);
			    }
			    if (LYdownload_options(&fname,
						   WWW_Download_File) < 0) {
				FREE(fname);
				return(NOT_FOUND);
			    }
			    StrAllocCopy(doc->address, fname);
			    FREE(fname);
		    	    WWWDoc.address = doc->address;
		    	    FREE(WWWDoc.post_data);
		    	    FREE(WWWDoc.post_content_type);
			    WWWDoc.bookmark = doc->bookmark;
			    WWWDoc.isHEAD = FALSE;
			    HTOutputFormat = WWW_PRESENT;
			    if (!HTLoadAbsolute(&WWWDoc)) 
                        	return(NOT_FOUND);
			    else 
				return(NORMAL);

			} else if (pound == NULL &&
				   /*
				    * HTAnchor hash-table searches are now
				    * case-sensitive (hopefully, without
				    * anchor deletion problems), so this
				    * is too. - FM
				    */
				   (strcmp(doc->address,
				  	   HTLoadedDocumentURL()) ||
				   /*
				    * Also check the post_data elements. - FM
				    */
				   strcmp((doc->post_data ?
				   	   doc->post_data : ""),
				    	  HTLoadedDocumentPost_data()) ||
				   /*
				    * Also check the isHEAD element. - FM
				    */
				   doc->isHEAD != HTLoadedDocumentIsHEAD())) {
			    HTMLSetCharacterHandling(current_char_set);
			    /*
			     *  Nothing needed to be shown.
			     */
			    return(NULLFILE);

                        } else {
                        /*
			 *  May set www_search_result.
			 */
                            if (pound != NULL)
                                HTFindPoundSelector(pound+1);
			    HTMLSetCharacterHandling(current_char_set);
                            return(NORMAL);
                        }
                    }
		}
	  } else {
	      if (TRACE)
	          sleep(MessageSecs);
	      _user_message("Badly formed address %s",doc->address);
	      if (TRACE)
	          fprintf(stderr,"\n");
	      sleep(MessageSecs);
              return(NULLFILE);
	  }
}

/* the user wants to select a link by number
 * if follow_link_number returns DO_LINK_STUFF do_link will be
 * run immeditely following its execution.
 * if follow_link_number returns PRINT_ERROR an error message will
 * be given to the user, if follow_link_number returns DO_FORMS_STUFF
 * some forms stuff will be done, and if follow_link_number returns
 * DO_NOTHING nothing special will run after it.
 */

PUBLIC int follow_link_number ARGS2(int,c, int,cur)
{
    char temp[120];
    int link_number;

    temp[0] = c;
    temp[1] = '\0';
    _statusline(FOLLOW_LINK_NUMBER);
    /* get the number from the user */
    if (LYgetstr(temp, VISIBLE, sizeof(temp), NORECALL) < 0 || *temp == 0) {
        _statusline(CANCELLED);
        sleep(InfoSecs);
        return(DO_NOTHING);
    }

    link_number = atoi(temp);

    if (link_number > 0)  {
              /* get the lname, and hightext,
               * direct from
               * www structures and add it to the cur link
               * so that we can pass it transparently on to
               * get_file()
               * this is done so that you may select a link
               * anywhere in the document, whether it is displayed
               * on the screen or not!
               */
	       if (HTGetLinkInfo(link_number, &links[cur].hightext, 
					 &links[cur].lname)) {
                   links[cur].type = WWW_LINK_TYPE;

		   return(DO_LINK_STUFF);
       		} else {
		   return(PRINT_ERROR);
		}
    } else {
            return(PRINT_ERROR);
    }

}

#if defined(EXEC_LINKS) || defined(LYNXCGI_LINKS)

struct trust {
	char *src;
	char *path;
	int type;
	struct trust *next;
};

static struct trust trusted_exec_default = {
  "file://localhost/",	"",	EXEC_PATH,		NULL
};
static struct trust always_trusted_exec_default = {
  "none",		"",	ALWAYS_EXEC_PATH,	NULL
};
static struct trust trusted_cgi_default = {
  "",			"",	CGI_PATH,		NULL
};

static struct trust *trusted_exec = &trusted_exec_default;
static struct trust *always_trusted_exec = &always_trusted_exec_default;
static struct trust *trusted_cgi = &trusted_cgi_default;

PRIVATE void LYTrusted_free NOARGS
{
    struct trust *cur;
    struct trust *next;
    
    if (trusted_exec != &trusted_exec_default) {
        cur = trusted_exec;
	while (cur) {
	    FREE(cur->src);
	    FREE(cur->path);
	    next = cur->next;
	    FREE(cur);
	    cur = next;
	}
    }

    if (always_trusted_exec != &always_trusted_exec_default) {
        cur = always_trusted_exec;
	while (cur) {
	    FREE(cur->src);
	    FREE(cur->path);
	    next = cur->next;
	    FREE(cur);
	    cur = next;
	}
    }

    if (trusted_cgi != &trusted_cgi_default) {
        cur = trusted_cgi;
	while (cur) {
	    FREE(cur->src);
	    FREE(cur->path);
	    next = cur->next;
	    FREE(cur);
	    cur = next;
	}
    }

    return;
}

PUBLIC void add_trusted ARGS2 (char *,str, int,type)
{
    struct trust *tp;
    char *path;
    char *src = str;
    int Type = type;
    static BOOLEAN first = TRUE;

    if (!src)
        return;
    if (first) {
        atexit(LYTrusted_free);
	first = FALSE;
    }

    path = strchr(src, '\t');
    if (path)
	*path++ = '\0';
    else
	path = "";

    tp = (struct trust *)malloc(sizeof(*tp));
    if (tp == NULL)
	outofmem(__FILE__, "add_trusted");
    tp->src = NULL;
    tp->path = NULL;
    tp->type = Type;
    StrAllocCopy(tp->src, src);
    StrAllocCopy(tp->path, path);
    if (Type == EXEC_PATH) {
	if (trusted_exec == &trusted_exec_default)
	    tp->next = NULL;
	else
	    tp->next = trusted_exec;
	trusted_exec = tp;
    } else if (Type == ALWAYS_EXEC_PATH) {
	if (always_trusted_exec == &always_trusted_exec_default)
	    tp->next = NULL;
	else
	    tp->next = always_trusted_exec;
	always_trusted_exec = tp;
    } else if (Type == CGI_PATH) {
	if (trusted_cgi == &trusted_cgi_default)
	    tp->next = NULL;
	else
	    tp->next = trusted_cgi;
	trusted_cgi = tp;
    }
}

/*
 * Check to see if the supplied paths is allowed to be executed.
 */
PUBLIC BOOLEAN exec_ok ARGS3(CONST char *,source, CONST char *,link, int, type)
{
    struct trust *tp;
    char CONST *cp;
    int Type = type;

    if (LYJumpFileURL)
	return TRUE;

    if (Type == EXEC_PATH) {
	tp = trusted_exec;
    } else if (Type == ALWAYS_EXEC_PATH) {
	tp = always_trusted_exec;
    } else if (Type == CGI_PATH) {
	tp = trusted_cgi;
    } else {
        HTAlert(MALFORMED_EXEC_REQUEST);
	return FALSE;
    }
#ifndef VMS
    /* security: reject on strange character */
    for (cp = link; *cp != '\0'; cp++) {
	if (!isalnum(*cp) && *cp != '_' && *cp != '-' &&
	   *cp != ' ' && *cp != ':' && *cp != '.' &&
	   *cp != '/' && *cp != '@' && *cp != '~' &&
	   *cp != '$' && *cp != '\t') {
	    char buf[128];
	    
	    sprintf(buf,
		    BADCHAR_IN_EXEC_LINK,
		    *cp);
	    HTAlert(buf);
	    return FALSE;
	}
    }
#endif /* !VMS */

check_tp_for_entry:
    while (tp) {
	if (tp->type == Type) {
	    char CONST *command = link;

	    if (strstr(command,"//") == link) {
		command += 2;
	    }
#ifdef VMS
	    if (strncasecomp(source, tp->src, strlen(tp->src)) == 0 &&
		strncasecomp(command, tp->path, strlen(tp->path)) == 0)
#else
	    if (strncmp(source, tp->src, strlen(tp->src)) == 0 &&
		strncmp(command, tp->path, strlen(tp->path)) == 0)
#endif /* VMS */
		return TRUE;
	}
	tp = tp->next;
    }
    if (Type == EXEC_PATH &&
        always_trusted_exec != &always_trusted_exec_default) {
        Type = ALWAYS_EXEC_PATH;
	tp = always_trusted_exec;
	goto check_tp_for_entry;
    }
    if (!(no_exec && type == ALWAYS_EXEC_PATH))
        HTAlert(BADLOCPATH_IN_EXEC_LINK);
    return FALSE;
}
#endif /* EXEC_LINKS || LYNXCGI_LINKS */

PRIVATE int fix_http_urls ARGS1(document *,doc)
{
    char *slash;

    /* if it's an ftp URL with a trailing slash, trim it off */
    if (!strncmp(doc->address, "ftp", 3) &&
        doc->address[strlen(doc->address)-1] == '/') {
	char *path = HTParse(doc->address, "", PARSE_PATH|PARSE_PUNCTUATION);
	if (path) {
	    if (path[0] == '/' && path[1] == '\0') {
	        FREE(path);
		return 0;
	    }
	    FREE(path);
	}
	if (TRACE)
	    fprintf(stderr,"LYGetFile: URL %s\n", doc->address);
	doc->address[strlen(doc->address)-1] = '\0';
	if (TRACE) {
	    fprintf(stderr,"    changed to %s\n", doc->address);
	    sleep(MessageSecs);
	}
    }

    /* if there isn't a slash besides the two at the beginning, append one */
    if ((slash = strrchr(doc->address, '/')) != NULL) {
	if (*(slash-1) != '/' || *(slash-2) != ':') {
	    return(0);
	}
    }
    if (TRACE)
        fprintf(stderr,"LYGetFile: URL %s\n", doc->address);
    StrAllocCat(doc->address, "/");
    if (TRACE) {
        fprintf(stderr,"    changed to %s\n",doc->address);
	sleep(MessageSecs);
    }

    return(1);
}
