/* $LynxId: LYGetFile.c,v 1.96 2018/04/01 15:27:18 tom Exp $ */
#include <HTUtils.h>
#include <HTTP.h>
#include <HTAnchor.h>		/* Anchor class */
#include <HTAccess.h>
#include <HTParse.h>
#include <LYCurses.h>
#include <GridText.h>
#include <LYGlobalDefs.h>
#include <LYUtils.h>
#include <LYCharSets.h>
#include <LYCharUtils.h>
#include <HTAlert.h>
#include <LYSignal.h>
#include <LYGetFile.h>
#include <LYPrint.h>
#include <LYOptions.h>
#include <LYStrings.h>
#include <LYClean.h>
#include <LYDownload.h>
#include <LYNews.h>
#include <LYMail.h>
#include <LYKeymap.h>
#include <LYBookmark.h>
#include <LYMap.h>
#include <LYList.h>
#ifdef DIRED_SUPPORT
#include <LYLocal.h>
#endif /* DIRED_SUPPORT */
#include <LYReadCFG.h>
#include <LYHistory.h>
#include <LYPrettySrc.h>

#include <LYexit.h>
#include <LYLeaks.h>

static int fix_httplike_urls(DocInfo *doc, UrlTypes type);

#ifdef VMS
#define STRNADDRCOMP strncasecomp
#else
#define STRNADDRCOMP strncmp
#endif /* !VMS */

int HTNoDataOK = 0;

/*
 * getfile is the main mechanism to load a new document (or a previously loaded
 * one whose rendering is cached in a HText structure) from mainloop, nearly
 * everything goes through it.
 * It should return one of the values
 *     NORMAL     - requested document loaded successfully, usually [always?]
 *                  its rendering is available as HTMainText.  It can be an
 *                  HTTP error message page or similar, we make no
 *                  distinction here.
 *     NOT_FOUND  - requested document cannot be accessed, and the reason
 *                  is a real error (as may be caused by an invalid link),
 *                  not just that lynx disallows access because of some
 *                  permission restrictions, and we have no error page
 *                  to show for it either.
 *     NULLFILE   - requested document not loaded into HTMainText, either
 *                  some interactive protocol was requested (like telnet),
 *                  or lynx does not allow access.
 * The distinction between NOT_FOUND and NULLFILE is not very crucial, but
 * getting it right prevents mainloop from exiting with the wrong message if it
 * happens for the first file, and from logging (or not logging) errors
 * inappropriately with -traversal, and from sending bogus error mail with
 * MAIL_SYSTEM_ERROR_LOGGING:TRUE.  - kw
 */
int getfile(DocInfo *doc, int *target)
{
    UrlTypes url_type = NOT_A_URL_TYPE;
    char *pound;
    char *cp = NULL;
    char *temp = NULL;
    DocAddress WWWDoc;		/* a WWW absolute doc address struct */

    /*
     * Reset LYCancelDownload to prevent unwanted delayed effect.  - KW
     */
    if (LYCancelDownload) {
	CTRACE((tfp, "getfile:    resetting LYCancelDownload to FALSE\n"));
	LYCancelDownload = FALSE;
    }

    /*
     * Reset fake 'Z' to prevent unwanted delayed effect.  - kw
     */
    LYFakeZap(NO);

    /*
     * Reset redirection counter to prevent bogus TOO_MANY_REDIRECTIONS in rare
     * situations if the previous cycle got to the limit, but did not fail for
     * that reason because the URL of the final location was handled specially,
     * not via HTLoadAbsolute.  - kw
     */
    redirection_attempts = 0;

  Try_Redirected_URL:
    /*
     * Load the WWWDoc struct in case we need to use it.
     */
    WWWDoc.address = doc->address;
    WWWDoc.post_data = doc->post_data;
    WWWDoc.post_content_type = doc->post_content_type;
    WWWDoc.bookmark = doc->bookmark;
    WWWDoc.isHEAD = doc->isHEAD;
    WWWDoc.safe = doc->safe;

    /*
     * Reset HTPermitRedir, it has done its job if it was set.  - kw
     */
    HTPermitRedir = FALSE;

    /*
     * Reset WWW_Download_File just in case.
     */
    FREE(WWW_Download_File);

    /*
     * Reset redirect_post_content just in case.
     */
    redirect_post_content = FALSE;

    /*
     * This flag is a hack to allow us to pass on the fact that 'no data' may
     * not really be an error although HTLoadAbsolute returned NO.  There
     * should be a better way...  HT_NO_DATA should always mean 'not data but
     * not an error', and be passed on to us as that, but current usage if
     * HT_NO_DATA vs HT_NOT_LOADED has to be reviewed everywhere.  Anyway, some
     * protocol module can set it to say 'I really mean it', we have to reset
     * it here.  - kw
     */
    HTNoDataOK = 0;

    CTRACE((tfp, "getfile: getting %s\n\n", doc->address));

    /*
     * Protect against denial of service attacks via the port 19 CHARGEN
     * service, and block connections to the port 25 ESMTP service.  Also
     * reject any likely spoof attempts via wrap arounds at 65536.  - FM
     */
    if ((temp = HTParse(doc->address, "", PARSE_HOST)) != NULL &&
	strlen(temp) > 3) {
	char *cp1;

	if ((cp1 = StrChr(temp, '@')) == NULL)
	    cp1 = temp;
	if ((cp = strrchr(cp1, ':')) != NULL) {
	    long int value;

	    cp++;
	    if (sscanf(cp, "%ld", &value) == 1) {
		if (value == 19 || value == 65555) {
		    HTAlert(PORT_NINETEEN_INVALID);
		    FREE(temp);
		    return (NULLFILE);
		} else if (value == 25 || value == 65561) {
		    HTAlert(PORT_TWENTYFIVE_INVALID);
		    FREE(temp);
		    return (NULLFILE);
		} else if (value > 65535 || value < 0) {
		    char *msg = 0;

		    HTSprintf0(&msg, PORT_INVALID, (unsigned long) value);
		    HTAlert(msg);
		    FREE(msg);
		    FREE(temp);
		    return (NULLFILE);
		}
	    } else if (isdigit(UCH(*cp))) {
		HTAlert(URL_PORT_BAD);
		FREE(temp);
		return (NULLFILE);
	    }
	}
    }
    cp = NULL;
    FREE(temp);

    /*
     * Check to see if this is a universal document ID that lib WWW wants to
     * handle.
     *
     * Some special URL's we handle ourselves.  :)
     */
    if ((url_type = is_url(doc->address)) != 0) {
	if (LYValidate && !LYPermitURL) {
	    if (!(url_type == HTTP_URL_TYPE ||
		  url_type == HTTPS_URL_TYPE ||
		  url_type == LYNXHIST_URL_TYPE ||
		  url_type == LYNXEDITMAP_URL_TYPE ||
		  url_type == LYNXKEYMAP_URL_TYPE ||
		  url_type == LYNXIMGMAP_URL_TYPE ||
		  url_type == LYNXCOOKIE_URL_TYPE ||
#ifdef USE_CACHEJAR
		  url_type == LYNXCACHE_URL_TYPE ||
#endif
		  url_type == LYNXMESSAGES_URL_TYPE ||
		  (url_type == LYNXOPTIONS_URL_TYPE &&
		   WWWDoc.post_data) ||
		  (non_empty(helpfilepath) &&
		   0 == STRNADDRCOMP(WWWDoc.address, helpfilepath,
				     strlen(helpfilepath))) ||
		  (non_empty(lynxlistfile) &&
		   0 == STRNADDRCOMP(WWWDoc.address, lynxlistfile,
				     strlen(lynxlistfile))) ||
		  (non_empty(lynxlinksfile) &&
		   0 == STRNADDRCOMP(WWWDoc.address, lynxlinksfile,
				     strlen(lynxlinksfile))) ||
		  (non_empty(lynxjumpfile) &&
		   0 == STRNADDRCOMP(WWWDoc.address, lynxjumpfile,
				     strlen(lynxjumpfile))))) {
		HTUserMsg(NOT_HTTP_URL_OR_ACTION);
		return (NULLFILE);
	    }
	}
	if (traversal) {
	    /*
	     * Only traverse http URLs.
	     */
	    if (url_type != HTTP_URL_TYPE &&
		url_type != LYNXIMGMAP_URL_TYPE) {
		return (NULLFILE);
	    }
	} else if (check_realm && !LYPermitURL && !LYJumpFileURL) {
	    if (!(0 == StrNCmp(startrealm, WWWDoc.address,
			       strlen(startrealm)) ||
		  url_type == LYNXHIST_URL_TYPE ||
		  url_type == LYNXEDITMAP_URL_TYPE ||
		  url_type == LYNXKEYMAP_URL_TYPE ||
		  url_type == LYNXIMGMAP_URL_TYPE ||
		  url_type == LYNXCOOKIE_URL_TYPE ||
#ifdef USE_CACHEJAR
		  url_type == LYNXCACHE_URL_TYPE ||
#endif
		  url_type == LYNXPRINT_URL_TYPE ||
		  url_type == LYNXOPTIONS_URL_TYPE ||
		  url_type == LYNXCFG_URL_TYPE ||
		  url_type == LYNXCOMPILE_OPTS_URL_TYPE ||
		  url_type == LYNXMESSAGES_URL_TYPE ||
		  url_type == LYNXDOWNLOAD_URL_TYPE ||
		  url_type == MAILTO_URL_TYPE ||
		  url_type == NEWSPOST_URL_TYPE ||
		  url_type == NEWSREPLY_URL_TYPE ||
		  url_type == SNEWSPOST_URL_TYPE ||
		  url_type == SNEWSREPLY_URL_TYPE ||
		  (!LYUserSpecifiedURL &&
		   (url_type == LYNXEXEC_URL_TYPE ||
		    url_type == LYNXPROG_URL_TYPE ||
		    url_type == LYNXCGI_URL_TYPE)) ||
		  (WWWDoc.bookmark != NULL &&
		   *WWWDoc.bookmark != '\0') ||
		  0 == STRNADDRCOMP(WWWDoc.address, helpfilepath,
				    strlen(helpfilepath)) ||
		  (lynxlistfile != NULL &&
		   0 == STRNADDRCOMP(WWWDoc.address, lynxlistfile,
				     strlen(lynxlistfile))) ||
		  (lynxjumpfile != NULL &&
		   0 == STRNADDRCOMP(WWWDoc.address, lynxjumpfile,
				     strlen(lynxjumpfile))))) {
		HTUserMsg(NOT_IN_STARTING_REALM);
		return (NULLFILE);
	    }
	}
	if (WWWDoc.post_data &&
	    url_type != HTTP_URL_TYPE &&
	    url_type != HTTPS_URL_TYPE &&
	    url_type != LYNXCGI_URL_TYPE &&
	    url_type != LYNXIMGMAP_URL_TYPE &&
	    url_type != GOPHER_URL_TYPE &&
	    url_type != CSO_URL_TYPE &&
	    url_type != PROXY_URL_TYPE &&
	    url_type != LYNXOPTIONS_URL_TYPE &&
	    !(url_type == FILE_URL_TYPE &&
	      (LYIsUIPage(WWWDoc.address, UIP_LIST_PAGE) ||
	       LYIsUIPage(WWWDoc.address, UIP_ADDRLIST_PAGE)))) {
	    CTRACE((tfp, "getfile: dropping post_data!\n"));
	    HTAlert(IGNORED_POST);
	    LYFreePostData(doc);
	    WWWDoc.post_data = NULL;
	    WWWDoc.post_content_type = NULL;
	}
#ifdef SYSLOG_REQUESTED_URLS
	LYSyslog(doc->address);
#endif
	if (url_type == UNKNOWN_URL_TYPE ||
	    url_type == AFS_URL_TYPE ||
	    url_type == PROSPERO_URL_TYPE) {
	    HTAlert(UNSUPPORTED_URL_SCHEME);
	    return (NULLFILE);

	} else if (url_type == DATA_URL_TYPE) {
	    HTAlert(UNSUPPORTED_DATA_URL);
	    return (NULLFILE);

	} else if (url_type == LYNXPRINT_URL_TYPE) {
	    return (printfile(doc));

#ifndef NO_OPTION_FORMS
	} else if (url_type == LYNXOPTIONS_URL_TYPE) {
	    /* proceed forms-based options menu */
	    return (postoptions(doc));
#endif

	} else if (url_type == LYNXCFG_URL_TYPE &&
		   !no_lynxcfg_info) {
	    /* @@@ maybe we should generate a specific error message
	       if attempted but restricted. - kw */
	    /* show/change/reload lynx.cfg settings */
	    return (lynx_cfg_infopage(doc));

#if defined(HAVE_CONFIG_H) && !defined(NO_CONFIG_INFO)
	} else if (url_type == LYNXCOMPILE_OPTS_URL_TYPE &&
		   !no_compileopts_info) {
	    /* @@@ maybe we should generate a specific error message
	       if attempted but restricted or not supported. - kw */
	    /* show compile-time settings */
	    return (lynx_compile_opts(doc));
#endif

#ifndef DISABLE_NEWS
	} else if (url_type == NEWSPOST_URL_TYPE ||
		   url_type == NEWSREPLY_URL_TYPE ||
		   url_type == SNEWSPOST_URL_TYPE ||
		   url_type == SNEWSREPLY_URL_TYPE) {

	    if (no_newspost) {
		HTUserMsg(NEWSPOSTING_DISABLED);
		return (NULLFILE);
	    } else if (!news_ok && (url_type == NEWSPOST_URL_TYPE ||
				    url_type == NEWSREPLY_URL_TYPE)) {
		HTUserMsg(NEWS_DISABLED);
		return (NULLFILE);
	    } else {
		HTLoadAbsolute(&WWWDoc);
		return (NULLFILE);
	    }
#endif

	} else if (url_type == LYNXDOWNLOAD_URL_TYPE) {
	    LYDownload(doc->address);
#ifdef VMS
	    if (LYDidRename) {
		/*
		 * The temporary file was saved to disk via a rename(), so we
		 * can't access the temporary file again via the download menu. 
		 * Clear the flag, and return NULLFILE to pop.  - FM
		 */
		LYDidRename = FALSE;
		return (NULLFILE);
	    }
#endif /* VMS */
	    return (NORMAL);
	} else if (url_type == LYNXDIRED_URL_TYPE) {
#ifdef DIRED_SUPPORT
	    if (!no_dired_support) {
		local_dired(doc);
		WWWDoc.address = doc->address;
		WWWDoc.post_data = doc->post_data;
		WWWDoc.post_content_type = doc->post_content_type;
		WWWDoc.bookmark = doc->bookmark;
		WWWDoc.isHEAD = doc->isHEAD;
		WWWDoc.safe = doc->safe;

		if (!HTLoadAbsolute(&WWWDoc)) {
		    return (NOT_FOUND);
		}
		return (NORMAL);
	    }
#endif /* DIRED_SUPPORT */
	    HTUserMsg(DIRED_DISABLED);
	    return (NULLFILE);
	}

	if (LYNoRefererHeader == FALSE &&
	    LYNoRefererForThis == FALSE) {
	    const char *ref_url = HTLoadedDocumentURL();

	    if (isLYNXIMGMAP(ref_url))
		ref_url += LEN_LYNXIMGMAP;
	    if (no_filereferer == TRUE && isFILE_URL(ref_url)) {
		LYNoRefererForThis = TRUE;
	    }
	    if (LYNoRefererForThis == FALSE &&
		(cp = StrChr(ref_url, '?')) != NULL &&
		StrChr(cp, '=') != NULL) {
		/*
		 * Don't send a Referer header if the URL is the reply from a
		 * form with method GET, in case the content has personal data
		 * (e.g., a password or credit card number) which would become
		 * visible in logs.  - FM
		 *
		 * Changed 1999-11-01 to be controlled by REFERER_WITH_QUERY
		 * option.  - kw
		 */
		if (LYRefererWithQuery == 'S') {	/* SEND */
		    StrAllocCopy(LYRequestReferer, ref_url);
		} else if (LYRefererWithQuery == 'P') {		/* PARTIAL */
		    FREE(LYRequestReferer);	/* just to be sure */
		    LYRequestReferer = HTParse(ref_url, "",
					       PARSE_ACCESS
					       | PARSE_HOST
					       | PARSE_STRICTPATH
					       | PARSE_PUNCTUATION);
		} else {	/* Everything else - don't send Referer */
		    LYNoRefererForThis = TRUE;
		}
		cp = NULL;
	    } else if (LYNoRefererForThis == FALSE) {
		StrAllocCopy(LYRequestReferer, ref_url);
	    }
	} else {
	    StrAllocCopy(LYRequestReferer, HTLoadedDocumentURL());
	}
	if (url_type == LYNXHIST_URL_TYPE) {
	    /*
	     * 'doc' will change to the new file if we had a successful
	     * LYpop_num(), and the return value will be FALSE if we had a
	     * cancel.  - FM
	     */
	    if ((historytarget(doc) == FALSE) ||
		!doc || !doc->address) {
		return (NOT_FOUND);
	    }

	    /*
	     * We changed it so reload.
	     */
	    WWWDoc.address = doc->address;
	    WWWDoc.post_data = doc->post_data;
	    WWWDoc.post_content_type = doc->post_content_type;
	    WWWDoc.bookmark = doc->bookmark;
	    WWWDoc.isHEAD = doc->isHEAD;
	    WWWDoc.safe = doc->safe;
	    if (track_internal_links && doc->internal_link && !reloading) {
		LYinternal_flag = TRUE;
	    }
#ifdef DIRED_SUPPORT
	    lynx_edit_mode = FALSE;
#endif /* DIRED_SUPPORT */
	    if (!HTLoadAbsolute(&WWWDoc)) {
		return (NOT_FOUND);
	    }
	    return (NORMAL);

	} else if (url_type == LYNXEXEC_URL_TYPE ||
		   url_type == LYNXPROG_URL_TYPE) {
#ifdef EXEC_LINKS
	    if (no_exec &&
		!exec_ok(HTLoadedDocumentURL(),
			 doc->address + 9, ALWAYS_EXEC_PATH)) {
		HTUserMsg(EXECUTION_DISABLED);
	    } else if (no_bookmark_exec &&
		       HTLoadedDocumentBookmark()) {
		HTUserMsg(BOOKMARK_EXEC_DISABLED);
	    } else if (local_exec || (local_exec_on_local_files &&
				      exec_ok(HTLoadedDocumentURL(),
					      doc->address + 9, EXEC_PATH))) {

		char *p = NULL;

		/*
		 * Bug puts slash on end if none is in the string.
		 */
		char *last_slash = strrchr(doc->address, '/');

		if (last_slash - doc->address
		    == (int) strlen(doc->address) - 1)
		    doc->address[strlen(doc->address) - 1] = '\0';

		/*
		 * Convert '~' to $HOME.
		 */
		StrAllocCopy(p, doc->address);
		LYTildeExpand(&p, TRUE);

		/*
		 * Show URL before executing it.
		 */
		HTInfoMsg(doc->address);
		stop_curses();
		/*
		 * Run the command.
		 */
		if (strstr(p, "//") == p + 9)
		    LYSystem(p + 11);
		else
		    LYSystem(p + 9);
		FREE(p);

		if (url_type != LYNXPROG_URL_TYPE) {
		    /*
		     * Make sure user gets to see screen output.
		     */
#ifndef VMS
		    signal(SIGINT, SIG_IGN);
#endif /* !VMS */
		    printf("\n%s", RETURN_TO_LYNX);
		    fflush(stdout);
		    (void) LYgetch();
#ifdef VMS
		    HadVMSInterrupt = FALSE;
#endif /* VMS */
		}
		if (!dump_output_immediately) {
		    start_curses();
		    LYAddVisitedLink(doc);
		}

	    } else {
		char *buf = 0;

		HTSprintf0(&buf,
			   EXECUTION_DISABLED_FOR_FILE,
			   key_for_func(LYK_OPTIONS));
		HTAlert(buf);
		FREE(buf);
	    }
#else /* no exec_links */
	    HTUserMsg(EXECUTION_NOT_COMPILED);
#endif /* EXEC_LINKS */
	    return (NULLFILE);

	} else if (url_type == MAILTO_URL_TYPE) {
	    if (no_mail) {
		HTUserMsg(MAIL_DISABLED);
	    } else if (!dump_output_immediately) {
		HTParentAnchor *tmpanchor = HTAnchor_findAddress(&WWWDoc);
		const char *title;
		char *tmptitle = NULL;

		title = "";
		if (HTAnchor_title(tmpanchor)) {
		    title = HTAnchor_title(tmpanchor);
		} else if (HTMainAnchor && !LYUserSpecifiedURL) {
		    title = HTAnchor_subject(HTMainAnchor);
		    if (non_empty(title)) {
			if (strncasecomp(title, "Re:", 3)) {
			    StrAllocCopy(tmptitle, "Re: ");
			    StrAllocCat(tmptitle, title);
			    title = tmptitle;
			}
		    } else {
			title = "";
		    }
		}
		cp = StrChr(doc->address, ':') + 1;
		reply_by_mail(cp,
			      ((HTMainAnchor && !LYUserSpecifiedURL)
			       ? (char *) HTMainAnchor->address
			       : (char *) doc->address),
			      title,
			      (HTMainAnchor && !LYUserSpecifiedURL)
			      ? HTMainAnchor->message_id
			      : NULL);
		FREE(tmptitle);
	    }
	    return (NULLFILE);

	    /*
	     * From here on we could have a remote host, so check if that's
	     * allowed.
	     */
	} else if (local_host_only &&
		   url_type != LYNXEDITMAP_URL_TYPE &&
		   url_type != LYNXKEYMAP_URL_TYPE &&
		   url_type != LYNXIMGMAP_URL_TYPE &&
		   url_type != LYNXCOOKIE_URL_TYPE &&
		   url_type != LYNXMESSAGES_URL_TYPE &&
#ifdef USE_CACHEJAR
		   url_type != LYNXCACHE_URL_TYPE &&
#endif
		   url_type != LYNXCGI_URL_TYPE &&
		   !(url_type == NEWS_URL_TYPE &&
		     StrNCmp(doc->address, "news://", 7)) &&
		   !(LYisLocalHost(doc->address) ||
		     LYisLocalAlias(doc->address))) {
	    HTUserMsg(ACCESS_ONLY_LOCALHOST);
	    return (NULLFILE);

	    /*
	     * Disable www telnet access if not telnet_ok.
	     */
	} else if (url_type == TELNET_URL_TYPE ||
		   url_type == TN3270_URL_TYPE ||
		   url_type == TELNET_GOPHER_URL_TYPE) {
	    char *proxy;

	    if (!telnet_ok) {
		HTUserMsg(TELNET_DISABLED);
		return (NULLFILE);
	    } else if (no_telnet_port && StrChr(doc->address + 7, ':')) {
		HTUserMsg(TELNET_PORT_SPECS_DISABLED);
		return (NULLFILE);
		/*
		 * Detect weird case where interactive protocol would be
		 * proxied, and to a non-interactive protocol at that.
		 */
	    } else if ((proxy = LYGetEnv(((url_type == TN3270_URL_TYPE)
					  ? "tn3270_proxy"
					  :
					  ((url_type == TELNET_GOPHER_URL_TYPE)
					   ? "gopher_proxy"
					   : "telnet_proxy")))) != NULL &&
		       !override_proxy(doc->address) &&
		       (!isTELNET_URL(proxy) &&
			!isTN3270_URL(proxy) &&
			!isRLOGIN_URL(proxy))) {
		/* Do nothing, fall through to generic code - kw */
	    } else {
		stop_curses();
		HTLoadAbsolute(&WWWDoc);
		if (!dump_output_immediately) {
		    start_curses();
		    fflush(stdout);
		    LYAddVisitedLink(doc);
		}
		return (NULLFILE);
	    }

	    /*
	     * Disable www news access if not news_ok.
	     */
#ifndef DISABLE_NEWS
	} else if (!news_ok && (url_type == NEWS_URL_TYPE ||
				url_type == NNTP_URL_TYPE)) {
	    HTUserMsg(NEWS_DISABLED);
	    return (NULLFILE);
#endif

	} else if (url_type == RLOGIN_URL_TYPE) {
	    char *proxy;

	    if (!rlogin_ok) {
		HTUserMsg(RLOGIN_DISABLED);
		return (NULLFILE);
		/*
		 * Detect weird case where interactive protocol would be
		 * proxied, and to a non-interactive protocol at that.
		 */
	    } else if ((proxy = LYGetEnv("rlogin_proxy")) != NULL &&
		       !override_proxy(doc->address) &&
		       (!isTELNET_URL(proxy) &&
			!isTN3270_URL(proxy) &&
			!isRLOGIN_URL(proxy))) {
		/* Do nothing, fall through to generic code - kw */
	    } else {
		stop_curses();
		HTLoadAbsolute(&WWWDoc);
		fflush(stdout);
		if (!dump_output_immediately) {
		    start_curses();
		    LYAddVisitedLink(doc);
		}
		return (NULLFILE);
	    }

	    /*
	     * If it's a gopher index type and there isn't a search term
	     * already attached then do this.  Otherwise just load it!
	     */
	} else if (url_type == INDEX_GOPHER_URL_TYPE &&
		   StrChr(doc->address, '?') == NULL) {
	    int status;

	    /*
	     * Make sure we don't have a gopher+ escaped tab instead of a
	     * gopher0 question mark delimiting the search term.  - FM
	     */
	    if ((cp = strstr(doc->address, "%09")) != NULL) {
		*cp = '\0';
		StrAllocCopy(temp, doc->address);
		cp += 3;
		if (*cp && StrNCmp(cp, "%09", 3)) {
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
	     * Load it because the do_www_search routine uses the base url of
	     * the currently loaded document :(
	     */
	    if (!HTLoadAbsolute(&WWWDoc)) {
		return (NOT_FOUND);
	    }
	    status = do_www_search(doc);
	    if (status == NULLFILE) {
		LYpop(doc);
		WWWDoc.address = doc->address;
		WWWDoc.post_data = doc->post_data;
		WWWDoc.post_content_type = doc->post_content_type;
		WWWDoc.bookmark = doc->bookmark;
		WWWDoc.isHEAD = doc->isHEAD;
		WWWDoc.safe = doc->safe;
		status = HTLoadAbsolute(&WWWDoc);
#ifdef DIRED_SUPPORT
	    } else {
		lynx_edit_mode = FALSE;
#endif /* DIRED_SUPPORT */
	    }
	    return (status);
	}

	if (!ftp_ok
	    && (url_type == FTP_URL_TYPE
		|| url_type == NCFTP_URL_TYPE)) {
	    HTUserMsg(FTP_DISABLED);
	    return (NULLFILE);
	} else if (url_type == HTML_GOPHER_URL_TYPE) {
	    char *tmp = NULL;

	    /*
	     * If tuple's Path=GET%20/...  convert to an http URL.
	     */
	    if ((cp = StrChr(doc->address + 9, '/')) != NULL &&
		0 == StrNCmp(++cp, "hGET%20/", 8)) {
		StrAllocCopy(tmp, "http://");
		CTRACE((tfp, "getfile: URL '%s'\n",
			doc->address));
		*cp = '\0';
		StrAllocCat(tmp, doc->address + 9);
		/*
		 * If the port is defaulted, it should stay 70.
		 */
		if (StrChr(tmp + 6, ':') == NULL) {
		    StrAllocCat(tmp, "70/");
		    tmp[strlen(tmp) - 4] = ':';
		}
		if (strlen(cp + 7) > 1)
		    StrAllocCat(tmp, cp + 8);
		StrAllocCopy(doc->address, tmp);
		CTRACE((tfp, "  changed to '%s'\n",
			doc->address));
		FREE(tmp);
		url_type = HTTP_URL_TYPE;
	    }
	}

	if (url_type == HTTP_URL_TYPE ||
	    url_type == HTTPS_URL_TYPE ||
	    url_type == FTP_URL_TYPE ||
	    url_type == NCFTP_URL_TYPE ||
	    url_type == CSO_URL_TYPE) {
	    fix_httplike_urls(doc, url_type);
	}

	WWWDoc.address = doc->address;	/* possible reload */
#ifdef DIRED_SUPPORT
	lynx_edit_mode = FALSE;
#endif /* DIRED_SUPPORT */

#ifndef DISABLE_BIBP
	if (url_type == BIBP_URL_TYPE) {
	    char *bibpTmp = NULL;

	    if (!BibP_bibhost_checked)
		LYCheckBibHost();
	    if (BibP_bibhost_available) {
		StrAllocCopy(bibpTmp, BibP_bibhost);
	    } else if (HTMainAnchor && HTAnchor_citehost(HTMainAnchor)) {
		StrAllocCopy(bibpTmp, HTAnchor_citehost(HTMainAnchor));
	    } else {
		StrAllocCopy(bibpTmp, BibP_globalserver);
	    }
	    if (HTMainAnchor && HTAnchor_citehost(HTMainAnchor)) {
		StrAllocCat(bibpTmp, "bibp1.0/resolve?citehost=");
		StrAllocCat(bibpTmp, HTAnchor_citehost(HTMainAnchor));
		StrAllocCat(bibpTmp, "&usin=");
	    } else {
		StrAllocCat(bibpTmp, "bibp1.0/resolve?usin=");
	    }
	    StrAllocCat(bibpTmp, doc->address + 5);	/* USIN after bibp: */
	    StrAllocCopy(doc->address, bibpTmp);
	    WWWDoc.address = doc->address;
	    FREE(bibpTmp);
	}
#endif /* !DISABLE_BIBP */

	if (url_type == FILE_URL_TYPE) {
	    /*
	     * If a file URL has a '~' as the lead character of its first
	     * symbolic element, convert the '~' to Home_Dir(), then append
	     * the rest of of path, if present, skipping "user" if "~user"
	     * was entered, simplifying, and eliminating any residual
	     * relative elements.  - FM
	     */
	    LYTildeExpand(&(doc->address), TRUE);
	    WWWDoc.address = doc->address;
	}
	CTRACE_SLEEP(MessageSecs);
	user_message(WWW_WAIT_MESSAGE, doc->address);

	if (TRACE) {
#ifdef USE_SLANG
	    if (LYCursesON) {
		LYaddstr("*\n");
		LYrefresh();
	    }
#endif /* USE_SLANG */
	    CTRACE((tfp, "\n"));
	}

	if (!HTLoadAbsolute(&WWWDoc)) {
	    /*
	     * Check for redirection.
	     */
	    if (use_this_url_instead != NULL) {
		if (!is_url(use_this_url_instead)) {
		    /*
		     * The server did not return a complete URL in its
		     * Location:  header, probably due to a FORM or other
		     * CGI script written by someone who doesn't know that
		     * the http protocol requires that it be a complete
		     * URL, or using a server which does not treat such a
		     * redirect string from the script as an instruction to
		     * resolve it versus the initial request, check
		     * authentication with that URL, and then act on it
		     * without returning redirection to us.  We'll violate
		     * the http protocol and resolve it ourselves using the
		     * URL of the original request as the BASE, rather than
		     * doing the RIGHT thing and returning an invalid
		     * address message.  - FM
		     */
		    HTUserMsg(LOCATION_NOT_ABSOLUTE);
		    temp = HTParse(use_this_url_instead,
				   WWWDoc.address,
				   PARSE_ALL);
		    if (non_empty(temp)) {
			StrAllocCopy(use_this_url_instead, temp);
		    }
		    FREE(temp);
		}
		url_type = is_url(use_this_url_instead);
		if (!HTPermitRedir &&
		    (url_type == LYNXDOWNLOAD_URL_TYPE ||
		     url_type == LYNXEXEC_URL_TYPE ||
		     url_type == LYNXPROG_URL_TYPE ||
#ifdef DIRED_SUPPORT
		     url_type == LYNXDIRED_URL_TYPE ||
#endif /* DIRED_SUPPORT */
		     url_type == LYNXPRINT_URL_TYPE ||
		     url_type == LYNXOPTIONS_URL_TYPE ||
		     url_type == LYNXCFG_URL_TYPE ||
		     url_type == LYNXCOMPILE_OPTS_URL_TYPE ||
		     url_type == LYNXHIST_URL_TYPE ||
		     url_type == LYNXCOOKIE_URL_TYPE ||
#ifdef USE_CACHEJAR
		     url_type == LYNXCACHE_URL_TYPE ||
#endif
		     url_type == LYNXMESSAGES_URL_TYPE ||
		     (LYValidate &&
		      url_type != HTTP_URL_TYPE &&
		      url_type != HTTPS_URL_TYPE) ||
		     ((no_file_url || no_goto_file) &&
		      url_type == FILE_URL_TYPE) ||
		     (no_goto_lynxcgi &&
		      url_type == LYNXCGI_URL_TYPE) ||
#ifndef DISABLE_BIBP
		     (no_goto_bibp &&
		      url_type == BIBP_URL_TYPE) ||
#endif
		     (no_goto_cso &&
		      url_type == CSO_URL_TYPE) ||
		     (no_goto_finger &&
		      url_type == FINGER_URL_TYPE) ||
		     (no_goto_ftp &&
		      (url_type == FTP_URL_TYPE ||
		       url_type == NCFTP_URL_TYPE)) ||
		     (no_goto_gopher &&
		      url_type == GOPHER_URL_TYPE) ||
		     (no_goto_http &&
		      url_type == HTTP_URL_TYPE) ||
		     (no_goto_https &&
		      url_type == HTTPS_URL_TYPE) ||
		     (no_goto_mailto &&
		      url_type == MAILTO_URL_TYPE) ||
#ifndef DISABLE_NEWS
		     (no_goto_news &&
		      url_type == NEWS_URL_TYPE) ||
		     (no_goto_nntp &&
		      url_type == NNTP_URL_TYPE) ||
#endif
		     (no_goto_rlogin &&
		      url_type == RLOGIN_URL_TYPE) ||
#ifndef DISABLE_NEWS
		     (no_goto_snews &&
		      url_type == SNEWS_URL_TYPE) ||
#endif
		     (no_goto_telnet &&
		      url_type == TELNET_URL_TYPE) ||
		     (no_goto_tn3270 &&
		      url_type == TN3270_URL_TYPE) ||
		     (no_goto_wais &&
		      url_type == WAIS_URL_TYPE))) {
		    /*
		     * Some schemes are not acceptable from server
		     * redirections.  - KW & FM
		     */
		    HTAlert(ILLEGAL_REDIRECTION_URL);
		    if (LYCursesON) {
			HTUserMsg2(WWW_ILLEGAL_URL_MESSAGE,
				   use_this_url_instead);
		    } else {
			fprintf(stderr,
				WWW_ILLEGAL_URL_MESSAGE,
				use_this_url_instead);
		    }
		    FREE(use_this_url_instead);
		    return (NULLFILE);
		}
		if ((pound = findPoundSelector(doc->address)) != NULL
		    && findPoundSelector(use_this_url_instead) == NULL) {
		    /*
		     * Our requested URL had a fragment associated with it,
		     * and the redirection URL doesn't, so we'll append the
		     * fragment associated with the original request.  If
		     * it's bogus for the redirection URL, we'll be
		     * positioned at the top of that document, so there's
		     * no harm done.  - FM
		     */
		    CTRACE((tfp,
			    "getfile: Adding fragment '%s' to redirection URL.\n",
			    pound));
		    StrAllocCat(use_this_url_instead, pound);
		    doc->link = -1;
		}
		CTRACE_SLEEP(MessageSecs);
		HTUserMsg2(WWW_USING_MESSAGE, use_this_url_instead);
		CTRACE((tfp, "\n"));
		StrAllocCopy(doc->address,
			     use_this_url_instead);
		FREE(use_this_url_instead);
		if (redirect_post_content == FALSE) {
		    /*
		     * Freeing the content also yields a GET request.  - FM
		     */
		    LYFreePostData(doc);
		}
		/*
		 * Go to top to check for URL's which get special handling
		 * and/or security checks in Lynx.  - FM
		 */
		goto Try_Redirected_URL;
	    }
	    if (HTNoDataOK) {
		return (NULLFILE);
	    } else {
		return (NOT_FOUND);
	    }
	} else {

	    lynx_mode = NORMAL_LYNX_MODE;

	    /*
	     * Some URL's don't actually return a document; compare
	     * doc->address with the document that is actually loaded and
	     * return NULLFILE if not loaded.  If www_search_result is not -1
	     * then this is a reference to a named anchor within the same
	     * document; do NOT return NULLFILE in that case.
	     */

	    /*
	     * Check for a #fragment selector.
	     */
	    pound = findPoundSelector(doc->address);

	    /*
	     * Check to see if there is a temp file waiting for us to
	     * download.
	     */
	    if (WWW_Download_File) {
		HTParentAnchor *tmpanchor = HTAnchor_findAddress(&WWWDoc);
		char *fname = NULL;

		/*
		 * Check for a suggested filename from the
		 * Content-Disposition header.  - FM
		 */
		if (HTAnchor_SugFname(tmpanchor) != NULL) {
		    StrAllocCopy(fname, HTAnchor_SugFname(tmpanchor));
		} else {
		    StrAllocCopy(fname, doc->address);
		}
		/*
		 * Check whether this is a compressed file, which we don't
		 * uncompress for downloads, and adjust any suffix
		 * appropriately.  - FM
		 */
		HTCheckFnameForCompression(&fname, tmpanchor, FALSE);

		if (LYdownload_options(&fname,
				       WWW_Download_File) < 0) {
		    FREE(fname);
		    return (NOT_FOUND);
		}
		LYAddVisitedLink(doc);
		StrAllocCopy(doc->address, fname);
		FREE(fname);
		doc->internal_link = FALSE;
		WWWDoc.address = doc->address;
		LYFreePostData(doc);
		WWWDoc.post_data = NULL;
		WWWDoc.post_content_type = NULL;
		WWWDoc.bookmark = doc->bookmark = FALSE;
		WWWDoc.isHEAD = doc->isHEAD = FALSE;
		WWWDoc.safe = doc->safe = FALSE;
		HTOutputFormat = WWW_PRESENT;
		if (!HTLoadAbsolute(&WWWDoc)) {
		    return (NOT_FOUND);
		} else {
		    return (NORMAL);
		}

	    } else if (pound == NULL &&
		/*
		 * HTAnchor hash-table searches are now case-sensitive
		 * (hopefully, without anchor deletion problems), so this
		 * is too.  - FM
		 */
		       (strcmp(doc->address,
			       HTLoadedDocumentURL()) ||
		/*
		 * Also check the post_data elements.  - FM
		 */
			!BINEQ(doc->post_data,
			       HTLoadedDocumentPost_data()) ||
		/*
		 * Also check the isHEAD element.  - FM
		 */
			doc->isHEAD != HTLoadedDocumentIsHEAD())) {
		/*
		 * Nothing needed to be shown.
		 */
		LYAddVisitedLink(doc);
		return (NULLFILE);

	    } else {
		if (pound != NULL) {
		    if (!HTMainText) {	/* this should not happen... */
			return (NULLFILE);	/* but it can. - kw */
		    }
		    /*
		     * May set www_search_result.
		     */
		    if (HTFindPoundSelector(pound + 1)) {
			*target = www_search_result;
			doc->link = -1;
		    }
		}
		return (NORMAL);
	    }
	}
    } else {
	CTRACE_SLEEP(MessageSecs);
	HTUserMsg2(WWW_BAD_ADDR_MESSAGE, doc->address);
	CTRACE((tfp, "\n"));
	return (NULLFILE);
    }
}

/*
 * Set source mode for the next retrieval via getfile or HTreparse_document.
 * mode == -1:  force normal presentation
 * mode == 1:  force source presentation
 * mode == 0:  reset to normal if it was set to source
 * - kw
 */
void srcmode_for_next_retrieval(int mode)
{
    if (mode < 0) {
	HTOutputFormat = WWW_PRESENT;
#ifdef USE_PRETTYSRC
	psrc_view = FALSE;
#endif

    } else if (mode == 0) {
	if (HTOutputFormat == WWW_SOURCE)
	    HTOutputFormat = WWW_PRESENT;
#ifdef USE_PRETTYSRC
	else if (LYpsrc)
	    psrc_view = FALSE;
#endif

    } else {
#ifdef USE_PRETTYSRC
	if (LYpsrc)
	    psrc_view = TRUE;
	else
	    HTOutputFormat = WWW_SOURCE;
#else
	HTOutputFormat = WWW_SOURCE;
#endif
    }
}

/*
 * The user wants to select a link or page by number.
 *
 * If follow_link_number returns DO_LINK_STUFF do_link will be run immediately
 * following its execution.
 *
 * If follow_link_number returns DO_GOTOLINK_STUFF it has updated the passed in
 * doc for positioning on a link.
 *
 * If follow_link_number returns DO_GOTOPAGE_STUFF it has set doc->line to the
 * top line of the desired page for displaying that page.
 *
 * If follow_link_number returns PRINT_ERROR an error message will be given to
 * the user.
 *
 * If follow_link_number returns DO_FORMS_STUFF some forms stuff will be done. 
 * (Not yet implemented.)
 *
 * If follow_link_number returns DO_NOTHING nothing special will run after it.
 */
int follow_link_number(int c,
		       int cur,
		       DocInfo *doc,
		       int *num)
{
    bstring *temp = NULL;
    char *p;
    int rel = 0;
    int new_top, new_link;
    BOOL want_go;
    int curline = *num;		/* passed in from mainloop() */
    int code;

    CTRACE((tfp, "follow_link_number(%d,%d,...)\n", c, cur));
    BStrCopy0(temp, "?");
    temp->str[0] = (char) c;
    *num = -1;
    _statusline(FOLLOW_LINK_NUMBER);

    /*
     * Get the number, possibly with a letter suffix, from the user.
     */
    if (LYgetBString(&temp, FALSE, 120, NORECALL) < 0 ||
	isBEmpty(temp)) {
	HTInfoMsg(CANCELLED);
	return (DO_NOTHING);
    }

    p = temp->str;
    *num = atoi(p);
    while (isdigit(UCH(*p)))
	++p;
    c = *p;			/* reuse c; 0 or g or p or + or - */
    switch (c) {
    case '+':
    case '-':
	/* 123+ or 123- */
	rel = c;
	c = *++p;
	break;
    default:
	rel = *++p;
	break;
    case 0:
	break;
    }
    /* don't currently check for errors typing suffix */

    CTRACE((tfp, "  temp=%s, *num=%d, rel='%c'\n", temp->str, *num, rel));
    /*
     * Check if we had a 'p' or 'P' following the number as a flag for
     * displaying the page with that number.  - FM
     */
    if ((c == 'p' || c == 'P') && display_lines == 0) {
	CTRACE((tfp, " curline=%d, LYlines=%d, display too small!\n",
		curline, LYlines));
	code = PRINT_ERROR;
    } else if (c == 'p' || c == 'P') {
	int nlines = HText_getNumOfLines();
	int npages = (((nlines + 1) > display_lines)
		      ? (((nlines + 1) + (display_lines - 1)) / (display_lines))
		      : 1);
	int curpage = (((curline + 1) > display_lines)
		       ? (((curline + 1) + (display_lines - 1)) / (display_lines))
		       : 1);

	CTRACE((tfp, " nlines=%d, npages=%d, curline=%d, curpage=%d\n",
		nlines, npages, curline, curpage));
	if (*num < 1)
	    *num = rel ? 0 : 1;
	if (rel == '+')
	    *num = curpage + *num;
	else if (rel == '-')
	    *num = curpage - *num;
	doc->line = ((npages <= 1)
		     ? 1
		     : ((*num <= npages)
			? (((*num - 1) * display_lines) + 1)
			: (((npages - 1) * display_lines) + 1)));
	code = DO_GOTOPAGE_STUFF;
    } else {

	/*
	 * Check if we want to make the link corresponding to the number the
	 * current link, rather than ACTIVATE-ing it.
	 */
	want_go = (BOOL) (c == 'g' || c == 'G');

	/* If rel, add or subtract num from current link, or
	 * nearest previous/subsequent link if current link is not on screen.
	 */
	if (rel)
	    *num = HTGetRelLinkNum(*num, rel, cur);
	/*
	 * If we have a valid number, act on it.
	 */
	if (*num > 0) {
	    int info;
	    char *text = NULL;

	    /*
	     * Get the lname, and hightext, directly from www structures and
	     * add it to the cur link so that we can pass it transparently on
	     * to getfile(), and load new_top and new_link if we instead want
	     * to make the link number current.  These things are done so that
	     * a link can be selected anywhere in the current document, whether
	     * it is displayed on the screen or not!
	     */
	    info = HTGetLinkInfo(*num,
				 want_go,
				 &new_top,
				 &new_link,
				 &text,
				 &links[cur].lname);
	    if (text != NULL)
		LYSetHilite(cur, text);

	    if (info == WWW_INTERN_LINK_TYPE) {
		links[cur].type = WWW_INTERN_LINK_TYPE;
		code = DO_LINK_STUFF;
	    } else if (info == LINK_LINE_FOUND) {
		doc->line = new_top + 1;
		doc->link = new_link;
		code = DO_GOTOLINK_STUFF;
	    } else if (info) {
		links[cur].type = WWW_LINK_TYPE;
		code = DO_LINK_STUFF;
	    } else {
		code = PRINT_ERROR;
	    }
	} else {
	    code = PRINT_ERROR;
	}
    }
    BStrFree(temp);
    return code;
}

#if defined(EXEC_LINKS) || defined(LYNXCGI_LINKS)

struct trust {
    char *src;
    char *path;
    int type;
    struct trust *next;
};

static struct trust *trusted_exec = 0;
static struct trust *always_trusted_exec;
static struct trust *trusted_cgi = 0;

static struct trust *new_trust(const char *src, const char *path, int type)
{
    struct trust *tp;

    tp = typecalloc(struct trust);

    if (tp == NULL)
	outofmem(__FILE__, "new_trust");

    tp->type = type;
    StrAllocCopy(tp->src, src);
    StrAllocCopy(tp->path, path);

    return tp;
}

static struct trust *get_trust(struct trust **table, const char *src, int type)
{
    if (*table == 0) {
	*table = new_trust(src, "", type);
    }
    return *table;
}

#ifdef LY_FIND_LEAKS
static void free_data(struct trust **data)
{
    struct trust *cur = (*data);
    struct trust *next;

    while (cur) {
	FREE(cur->src);
	FREE(cur->path);
	next = cur->next;
	FREE(cur);
	cur = next;
    }
    *data = NULL;
}

static void LYTrusted_free(void)
{
    free_data(&trusted_exec);
    free_data(&always_trusted_exec);
    free_data(&trusted_cgi);

    return;
}
#endif /* LY_FIND_LEAKS */

void add_trusted(char *str,
		 int type)
{
    struct trust *tp;
    char *path;
    char *src = str;
    const char *after_tab;
    int Type = type;
    static BOOLEAN first = TRUE;

    if (!src)
	return;
    if (first) {
#ifdef LY_FIND_LEAKS
	atexit(LYTrusted_free);
#endif
	first = FALSE;
    }

    path = StrChr(src, '\t');
    if (path) {
	*path++ = '\0';
	after_tab = path;
    } else {
	after_tab = "";
    }

    tp = new_trust(src, after_tab, Type);

    if (Type == EXEC_PATH) {
	tp->next = trusted_exec;
	trusted_exec = tp;
    } else if (Type == ALWAYS_EXEC_PATH) {
	tp->next = always_trusted_exec;
	always_trusted_exec = tp;
    } else if (Type == CGI_PATH) {
	tp->next = trusted_cgi;
	trusted_cgi = tp;
    }
}

/*
 * Check to see if the supplied paths is allowed to be executed.
 */
BOOLEAN exec_ok(const char *source,
		const char *linktext,
		int type)
{
    struct trust *tp;
    const char *cp;
    const char *allowed_extra_chars;
    int Type = type;

    /*
     * Always OK if it is a jump file shortcut.
     */
    if (LYJumpFileURL)
	return TRUE;

    /*
     * Choose the trust structure based on the type.
     */
    if (Type == EXEC_PATH) {
	tp = get_trust(&trusted_exec, "file://localhost/", EXEC_PATH);
    } else if (Type == ALWAYS_EXEC_PATH) {
	tp = get_trust(&always_trusted_exec, "none", ALWAYS_EXEC_PATH);
    } else if (Type == CGI_PATH) {
	tp = get_trust(&trusted_cgi, "none", CGI_PATH);
    } else {
	HTAlert(MALFORMED_EXEC_REQUEST);
	return FALSE;
    }

#ifdef VMS
    /*
     * Security:  reject on relative path.
     */
    if ((cp = StrChr(linktext, '[')) != NULL) {
	char *cp1;

	if (((cp1 = StrChr(cp, '-')) != NULL) &&
	    StrChr(cp1, ']') != NULL) {
	    while (cp1[1] == '-')
		cp1++;
	    if (cp1[1] == ']' ||
		cp1[1] == '.') {
		HTAlert(RELPATH_IN_EXEC_LINK);
		return FALSE;
	    }
	}
    }
#else
    /*
     * Security:  reject on relative path.
     */
    if (strstr(linktext, "../") != NULL) {
	HTAlert(RELPATH_IN_EXEC_LINK);
	return FALSE;
    }

    /*
     * Security:  reject on strange character.
     */
    if (Type == CGI_PATH)
	allowed_extra_chars = " _-:./@~$&+=\t";
    else
	allowed_extra_chars = " _-:./@~$+=\t";
    for (cp = linktext; *cp != '\0'; cp++) {
	if (!isalnum(UCH(*cp)) && !StrChr(allowed_extra_chars, *cp)) {
	    char *buf = 0;

	    HTSprintf0(&buf,
		       BADCHAR_IN_EXEC_LINK,
		       *cp);
	    HTAlert(buf);
	    FREE(buf);
	    return FALSE;
	}
    }
#endif /* VMS */

  check_tp_for_entry:
    while (tp) {
	if (tp->type == Type) {
	    char const *command = linktext;

	    if (strstr(command, "//") == linktext) {
		command += 2;
	    }
	    CTRACE((tfp, "comparing source\n\t'%s'\n\t'%s'\n", source, tp->src));
	    CTRACE((tfp, "comparing command\n\t'%s'\n\t'%s'\n", command, tp->path));
	    if (STRNADDRCOMP(source, tp->src, strlen(tp->src)) == 0 &&
		STRNADDRCOMP(command, tp->path, strlen(tp->path)) == 0)
		return TRUE;
	}
	tp = tp->next;
    }
    if (Type == EXEC_PATH &&
	always_trusted_exec->next != 0) {
	Type = ALWAYS_EXEC_PATH;
	tp = always_trusted_exec;
	goto check_tp_for_entry;
    }
    if (!(no_exec && type == ALWAYS_EXEC_PATH))
	HTAlert(BADLOCPATH_IN_EXEC_LINK);
    return FALSE;
}
#endif /* EXEC_LINKS || LYNXCGI_LINKS */

static int fix_httplike_urls(DocInfo *doc, UrlTypes type)
{
    char *slash;

    /*
     * If there's a fragment present, our simplistic methods won't work.  - kw
     */
    if (findPoundSelector(doc->address) != NULL)
	return 0;

#ifndef DISABLE_FTP
    /*
     * If it's an ftp URL with a trailing slash, trim it off.
     */
    if (type == FTP_URL_TYPE &&
	LYIsHtmlSep(doc->address[strlen(doc->address) - 1])) {
	char *path = HTParse(doc->address, "", PARSE_PATH | PARSE_PUNCTUATION);

	/*
	 * If the path is a lone slash, we're done.  - FM
	 */
	if (path) {
	    if (LYIsHtmlSep(path[0]) && path[1] == '\0') {
		FREE(path);
		return 0;
	    }
	    FREE(path);
	}

	/*
	 * If we're proxying ftp, don't trim anything.  - KW
	 */
	if ((LYGetEnv("ftp_proxy") != NULL) &&
	    !override_proxy(doc->address))
	    return 0;

	/*
	 * If we get to here, trim the trailing slash.  - FM
	 */
	CTRACE((tfp, "fix_httplike_urls: URL '%s'\n", doc->address));
	LYTrimHtmlSep(doc->address);
	CTRACE((tfp, "            changed to '%s'\n", doc->address));
	CTRACE_SLEEP(MessageSecs);
    } else if (type == NCFTP_URL_TYPE) {
	char *path = NULL;
	char *first = doc->address;
	char *second = StrChr(first, ':');

	CTRACE((tfp, "fix_httplike_urls: URL '%s'\n", doc->address));
	if (second == 0)
	    second = first + strlen(first);
	else
	    *second++ = '\0';
	HTSprintf0(&path, "%s//%s%s", STR_FTP_URL, first, second);
	FREE(doc->address);
	doc->address = path;

	CTRACE((tfp, "            changed to '%s'\n", doc->address));
	CTRACE_SLEEP(MessageSecs);
    }
#endif /* DISABLE_FTP */

    /*
     * If there isn't a slash besides the two at the beginning, append one.
     */
    if ((slash = strrchr(doc->address, '/')) != NULL) {
	if (!LYIsHtmlSep(*(slash - 1)) || *(slash - 2) != ':') {
	    return (0);
	}
	if (type == HTTP_URL_TYPE ||
	    type == HTTPS_URL_TYPE) {
	    if ((slash - 2) != StrChr(doc->address, ':')) {
		/*
		 * Turns out we were not looking at the right slash after all,
		 * there must have been more than one "://" which is valid at
		 * least for http URLs (later occurrences can be part of a
		 * query string, for example), so leave this alone, too.  - kw
		 */
		return (0);
	    }
	    if (StrChr(doc->address, '?')) {
		/*
		 * If there is a question mark that appears to be part of the
		 * hostname, don't append anything either.  Leave it to HTParse
		 * to interpret the question mark as ending the hostname.  - kw
		 */
		return (0);
	    }
	}
    }
    CTRACE((tfp, "fix_httplike_urls: URL '%s'\n", doc->address));
    LYAddHtmlSep(&(doc->address));
    CTRACE((tfp, "            changed to '%s'\n", doc->address));
    CTRACE_SLEEP(MessageSecs);

    return (1);
}
