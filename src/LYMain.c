#include "HTUtils.h"
#include "tcp.h"
#include "HTParse.h"
#include "HTAccess.h"
#include "HTList.h"
#include "HTFile.h"
#ifdef VMS
#include "HTVMSUtils.h"
#endif /* VMS */
#include "HTInit.h"
#include "LYCurses.h"
#include "HTML.h"
#include "LYUtils.h"
#include "LYGlobalDefs.h"
#include "LYSignal.h"
#include "LYGetFile.h"
#include "LYStrings.h"
#include "LYClean.h"
#include "LYCharSets.h"
#include "LYCharUtils.h"
#include "LYReadCFG.h"
#include "LYrcFile.h"
#include "LYKeymap.h"
#include "LYList.h"
#include "LYJump.h"
#include "LYBookmark.h"

#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
#include <syslog.h>
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

#ifdef LOCALE
#include<locale.h>
#endif /* LOCALE */

#include "LYexit.h"
#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

#ifdef VMS
#define DISPLAY "DECW$DISPLAY"
#else
#define DISPLAY "DISPLAY"
#endif /* VMS */

/* ahhhhhhhhhh!! Global variables :-< */
#ifdef SOCKS
PUBLIC BOOLEAN socks_flag=TRUE;
#endif /* SOCKS */

#ifdef IGNORE_CTRL_C
PUBLIC BOOLEAN sigint = FALSE;
#endif /* IGNORE_CTRL_C */

#ifdef VMS
PUBLIC char *mail_adrs = NULL;	/* the mask for a VMS mail transport */
	       /* create FIXED 512 binaries */
PUBLIC BOOLEAN UseFixedRecords = USE_FIXED_RECORDS;
#endif /* VMS */

#ifndef VMS
PUBLIC char *lynx_version_putenv_command = NULL;
PUBLIC char *NNTPSERVER_putenv_cmd = NULL;   /* lynx.cfg defined NNTPSERVER */
PUBLIC char *http_proxy_putenv_cmd = NULL;   /* lynx.cfg defined http_proxy */
PUBLIC char *https_proxy_putenv_cmd = NULL;  /* lynx.cfg defined https_proxy */
PUBLIC char *ftp_proxy_putenv_cmd = NULL;    /* lynx.cfg defined ftp_proxy */
PUBLIC char *gopher_proxy_putenv_cmd = NULL; /* lynx.cfg defined gopher_proxy */
PUBLIC char *cso_proxy_putenv_cmd = NULL;    /* lynx.cfg defined cso_proxy */
PUBLIC char *news_proxy_putenv_cmd = NULL;   /* lynx.cfg defined news_proxy */
PUBLIC char *newspost_proxy_putenv_cmd = NULL;
PUBLIC char *newsreply_proxy_putenv_cmd = NULL;
PUBLIC char *snews_proxy_putenv_cmd = NULL;  /* lynx.cfg defined snews_proxy */
PUBLIC char *snewspost_proxy_putenv_cmd = NULL;
PUBLIC char *snewsreply_proxy_putenv_cmd = NULL;
PUBLIC char *nntp_proxy_putenv_cmd = NULL;   /* lynx.cfg defined nntp_proxy */
PUBLIC char *wais_proxy_putenv_cmd = NULL;   /* lynx.cfg defined wais_proxy */
PUBLIC char *finger_proxy_putenv_cmd = NULL; /* lynx.cfg defined finger_proxy */
PUBLIC char *no_proxy_putenv_cmd = NULL;     /* lynx.cfg defined no_proxy */
PUBLIC char *list_format=NULL;		/* LONG_LIST formatting mask */
#ifdef SYSLOG_REQUESTED_URLS
PUBLIC char *syslog_txt = NULL;		/* syslog arb text for session */
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

#ifdef VMS
PUBLIC char *LYCSwingPath = NULL;
#endif /* VMS */

#ifdef DIRED_SUPPORT
PUBLIC BOOLEAN lynx_edit_mode = FALSE;
PUBLIC BOOLEAN no_dired_support = FALSE;
PUBLIC BOOLEAN dir_list_style = MIXED_STYLE;
PUBLIC HTList *tagged = NULL;
#ifdef OK_OVERRIDE
PUBLIC BOOLEAN prev_lynx_edit_mode = FALSE;
#endif /* OK_OVERRIDE */
#ifdef OK_PERMIT
#ifdef NO_CHANGE_EXECUTE_PERMS
PUBLIC BOOLEAN no_change_exec_perms = TRUE;
#else
PUBLIC BOOLEAN no_change_exec_perms = FALSE;
#endif /* NO_CHANGE_EXECUTE_PERMS */
#endif /* OK_PERMIT */
#endif /* DIRED_SUPPORT */

	   /* Number of docs cached in memory */
PUBLIC int HTCacheSize = DEFAULT_CACHE_SIZE;
#if defined(VMS) && defined(VAXC) && !defined(__DECC)
	   /* Don't dump doc cache unless this size is exceeded */
PUBLIC int HTVirtualMemorySize = DEFAULT_VIRTUAL_MEMORY_SIZE;
#endif /* VMS && VAXC && !_DECC */

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
#ifndef NEVER_ALLOW_REMOTE_EXEC
PUBLIC BOOLEAN local_exec = LOCAL_EXECUTION_LINKS_ALWAYS_ON;
#else
PUBLIC BOOLEAN local_exec = FALSE;
#endif /* NEVER_ALLOW_REMOTE_EXEC */
PUBLIC BOOLEAN local_exec_on_local_files =
	       LOCAL_EXECUTION_LINKS_ON_BUT_NOT_REMOTE;
#endif /* EXEC_LINKS || EXEC_SCRIPTS */

#ifdef REVERSE_CLEAR_SCREEN_PROBLEM
PUBLIC BOOLEAN enable_scrollback=TRUE;
#else
PUBLIC BOOLEAN enable_scrollback=FALSE;
#endif /* REVERSE_CLEAR_SCREEN_PROBLEM */

PUBLIC char *empty_string = "\0";
PUBLIC int display_lines;  /* number of lines in display */
PUBLIC int www_search_result= -1;
			       /* linked list of printers */
PUBLIC lynx_printer_item_type *printers = NULL;
			    /* linked list of download options */
PUBLIC lynx_html_item_type *downloaders = NULL;
			    /* linked list of upload options */
PUBLIC lynx_html_item_type *uploaders = NULL;
PUBLIC int port_syntax = 1;
PUBLIC BOOLEAN LYShowCursor = SHOW_CURSOR; /* to show or not to show */
PUBLIC BOOLEAN LYforce_no_cache = FALSE;
PUBLIC BOOLEAN LYoverride_no_cache = FALSE;
PUBLIC BOOLEAN LYresubmit_posts = ALWAYS_RESUBMIT_POSTS;
PUBLIC BOOLEAN LYUserSpecifiedURL = TRUE;/* always TRUE  the first time */
PUBLIC BOOLEAN LYJumpFileURL = FALSE;	 /* always FALSE the first time */
PUBLIC BOOLEAN jump_buffer = JUMPBUFFER; /* TRUE if offering default shortcut */
PUBLIC BOOLEAN goto_buffer = GOTOBUFFER; /* TRUE if offering default goto URL */
PUBLIC BOOLEAN recent_sizechange = FALSE;/* the window size changed recently? */
PUBLIC BOOLEAN user_mode = NOVICE_MODE;
PUBLIC BOOLEAN dump_output_immediately = FALSE;
PUBLIC BOOLEAN is_www_index = FALSE;
PUBLIC BOOLEAN lynx_mode = NORMAL_LYNX_MODE;
PUBLIC BOOLEAN bold_headers = FALSE;
PUBLIC BOOLEAN bold_H1 = FALSE;
PUBLIC BOOLEAN bold_name_anchors = FALSE;
PUBLIC BOOLEAN use_underscore = SUBSTITUTE_UNDERSCORES;
PUBLIC BOOLEAN nolist = FALSE;
PUBLIC BOOLEAN historical_comments = FALSE;
PUBLIC BOOLEAN minimal_comments = FALSE;
PUBLIC BOOLEAN soft_dquotes = FALSE;
PUBLIC BOOLEAN LYValidate = FALSE;
PUBLIC BOOLEAN LYPermitURL = FALSE;
PUBLIC BOOLEAN child_lynx = FALSE;
PUBLIC BOOLEAN error_logging = MAIL_SYSTEM_ERROR_LOGGING;
PUBLIC BOOLEAN check_mail = CHECKMAIL;
PUBLIC BOOLEAN vi_keys = VI_KEYS_ALWAYS_ON;
PUBLIC BOOLEAN emacs_keys = EMACS_KEYS_ALWAYS_ON;
PUBLIC BOOLEAN keypad_mode = DEFAULT_KEYPAD_MODE;
PUBLIC BOOLEAN case_sensitive = CASE_SENSITIVE_ALWAYS_ON;
PUBLIC BOOLEAN telnet_ok = TRUE;
PUBLIC BOOLEAN news_ok = TRUE;
PUBLIC BOOLEAN rlogin_ok = TRUE;
PUBLIC BOOLEAN ftp_ok = TRUE;
PUBLIC BOOLEAN system_editor = FALSE;
PUBLIC BOOLEAN no_inside_telnet = FALSE;
PUBLIC BOOLEAN no_outside_telnet = FALSE;
PUBLIC BOOLEAN no_telnet_port = FALSE;
PUBLIC BOOLEAN no_inside_news = FALSE;
PUBLIC BOOLEAN no_outside_news = FALSE;
PUBLIC BOOLEAN no_inside_ftp = FALSE;
PUBLIC BOOLEAN no_outside_ftp = FALSE;
PUBLIC BOOLEAN no_inside_rlogin = FALSE;
PUBLIC BOOLEAN no_outside_rlogin = FALSE;
PUBLIC BOOLEAN no_suspend = FALSE;
PUBLIC BOOLEAN no_editor = FALSE;
PUBLIC BOOLEAN no_shell = FALSE;
PUBLIC BOOLEAN no_bookmark = FALSE;
PUBLIC BOOLEAN no_multibook = FALSE;
PUBLIC BOOLEAN no_bookmark_exec = FALSE;
PUBLIC BOOLEAN no_option_save = FALSE;
PUBLIC BOOLEAN no_print = FALSE;
PUBLIC BOOLEAN no_download = FALSE;
PUBLIC BOOLEAN no_disk_save = FALSE;
PUBLIC BOOLEAN no_exec = FALSE;
PUBLIC BOOLEAN no_lynxcgi = FALSE;
PUBLIC BOOLEAN exec_frozen = FALSE;
PUBLIC BOOLEAN no_goto = FALSE;
PUBLIC BOOLEAN no_goto_cso = FALSE;
PUBLIC BOOLEAN no_goto_file = FALSE;
PUBLIC BOOLEAN no_goto_finger = FALSE;
PUBLIC BOOLEAN no_goto_ftp = FALSE;
PUBLIC BOOLEAN no_goto_gopher = FALSE;
PUBLIC BOOLEAN no_goto_http = FALSE;
PUBLIC BOOLEAN no_goto_https = FALSE;
PUBLIC BOOLEAN no_goto_lynxcgi = FALSE;
PUBLIC BOOLEAN no_goto_lynxexec = FALSE;
PUBLIC BOOLEAN no_goto_lynxprog = FALSE;
PUBLIC BOOLEAN no_goto_mailto = FALSE;
PUBLIC BOOLEAN no_goto_news = FALSE;
PUBLIC BOOLEAN no_goto_nntp = FALSE;
PUBLIC BOOLEAN no_goto_rlogin = FALSE;
PUBLIC BOOLEAN no_goto_snews = FALSE;
PUBLIC BOOLEAN no_goto_telnet = FALSE;
PUBLIC BOOLEAN no_goto_tn3270 = FALSE;
PUBLIC BOOLEAN no_goto_wais = FALSE;
PUBLIC BOOLEAN no_jump = FALSE;
PUBLIC BOOLEAN no_file_url = FALSE;
PUBLIC BOOLEAN no_newspost = FALSE;
PUBLIC BOOLEAN no_mail = FALSE;
PUBLIC BOOLEAN no_dotfiles = NO_DOT_FILES;
PUBLIC BOOLEAN no_useragent = FALSE;
PUBLIC BOOLEAN no_statusline = FALSE;
PUBLIC BOOLEAN no_filereferer = FALSE;
PUBLIC BOOLEAN local_host_only = FALSE;
PUBLIC BOOLEAN override_no_download = FALSE;
PUBLIC BOOLEAN show_dotfiles = FALSE; /* From rcfile if no_dotfiles is false */
PUBLIC BOOLEAN LYforce_HTML_mode = FALSE;
PUBLIC char *homepage = NULL;	/* home page or main screen */
PUBLIC char *editor = NULL;	/* the name of the current editor */
PUBLIC char *jumpfile = NULL;	/* the name of the default jumps file */
PUBLIC char *jumpprompt = NULL;	/* the default jumps prompt */
PUBLIC char *bookmark_page = NULL; /* the name of the default bookmark page */
PUBLIC char *BookmarkPage = NULL;  /* the name of the current bookmark page */
PUBLIC char *LynxHome = NULL;	/* the default Home HREF. */
PUBLIC char *startfile = NULL;	/* the first file */
PUBLIC char *helpfile = NULL; 	/* the main help file */
PUBLIC char *helpfilepath = NULL;   /* the path to the help file set */
PUBLIC char *aboutfilepath = NULL;  /* the path to the about lynx file */
PUBLIC char *lynxjumpfile = NULL;   /* the current jump file URL */
PUBLIC char *lynxlistfile = NULL;   /* the current list file URL */
PUBLIC char *lynxlinksfile = NULL;  /* the current visited links file URL */
PUBLIC char *startrealm = NULL;     /* the startfile realm */
PUBLIC char *indexfile = NULL;	    /* an index file if there is one */
PUBLIC char *personal_mail_address = NULL; /* the users mail address */
PUBLIC char *display=NULL;	    /* display environment variable */
PUBLIC char *personal_type_map = NULL;	   /* .mailcap */
PUBLIC char *global_type_map = NULL;	   /* global mailcap */
PUBLIC char *global_extension_map = NULL;  /* global mime.types */
PUBLIC char *personal_extension_map = NULL;/* .mime.types */
PUBLIC char *language = NULL;	    /* preferred language */
PUBLIC char *pref_charset = NULL;   /* preferred character set */
PUBLIC BOOLEAN LYNewsPosting = NEWS_POSTING; /* News posting supported? */
PUBLIC char *LynxSigFile = NULL;    /* Signature file, in or off home */
PUBLIC char *system_mail = NULL;    /* The path for sending mail */
PUBLIC char *lynx_temp_space = NULL; /* The prefix for temporary file paths */
PUBLIC char *lynx_save_space = NULL; /* The prefix for save to disk paths */
PUBLIC char *LYHostName = NULL;		/* treat as a local host name */
PUBLIC char *LYLocalDomain = NULL;	/* treat as a local domain tail */
PUBLIC BOOLEAN clickable_images = MAKE_LINKS_FOR_ALL_IMAGES;
PUBLIC BOOLEAN pseudo_inline_alts = MAKE_PSEUDO_ALTS_FOR_INLINES;
PUBLIC BOOLEAN crawl=FALSE;	     /* Do crawl? */
PUBLIC BOOLEAN traversal=FALSE;     /* Do traversals? */
PUBLIC BOOLEAN check_realm=FALSE;   /* Restrict to the starting realm? */
	       /* Links beyond a displayed page with no links? */ 
PUBLIC BOOLEAN more_links=FALSE;
PUBLIC int     ccount=0;  /* Starting number for lnk#.dat files in crawls */
PUBLIC BOOLEAN LYCancelledFetch=FALSE; /* TRUE if cancelled binary fetch */
	       /* Include mime headers with source dump */
PUBLIC BOOLEAN keep_mime_headers = FALSE;
PUBLIC BOOLEAN no_url_redirection = FALSE; /* Don't follow URL redirections */
PUBLIC char *form_post_data = NULL;  /* User data for post form */
PUBLIC char *form_get_data = NULL;   /* User data for get form */
PUBLIC char *http_error_file = NULL; /* Place HTTP status code in this file */
	     /* Id:Password for protected forms */
PUBLIC char *authentication_info[2] = {NULL, NULL};
PUBLIC BOOLEAN HEAD_request = FALSE;
PUBLIC BOOLEAN scan_for_buried_news_references = TRUE;
PUBLIC BOOLEAN LYRawMode;
PUBLIC BOOLEAN LYDefaultRawMode;
PUBLIC BOOLEAN LYUseDefaultRawMode = TRUE;
PUBLIC int LYlines = 24;
PUBLIC int LYcols = 80;
PUBLIC linkstruct links[MAXLINKS];
PUBLIC histstruct history[MAXHIST];
PUBLIC int nlinks = 0;		/* number of links in memory */
PUBLIC int nhist = 0;		/* number of history entries */
PUBLIC int more = FALSE;	/* is there more text to display? */
PUBLIC int InfoSecs;	/* Seconds to sleep() for Information messages */
PUBLIC int MessageSecs;	/* Seconds to sleep() for important Messages   */
PUBLIC int AlertSecs;	/* Seconds to sleep() for HTAlert() messages   */
PUBLIC BOOLEAN bookmark_start = FALSE;
PUBLIC char *LYUserAgent = NULL;	/* Lynx User-Agent header          */
PUBLIC char *LYUserAgentDefault = NULL;	/* Lynx default User-Agent header  */
PUBLIC BOOLEAN LYNoRefererHeader=FALSE;	/* Never send Referer header?      */
PUBLIC BOOLEAN LYNoRefererForThis=FALSE;/* No Referer header for this URL? */
PUBLIC BOOLEAN LYNoFromHeader=FALSE;	/* Never send From header?         */
PUBLIC BOOLEAN LYListNewsNumbers = LIST_NEWS_NUMBERS;
PUBLIC BOOLEAN LYListNewsDates = LIST_NEWS_DATES;
PUBLIC BOOLEAN LYisConfiguredForX = FALSE;
PUBLIC char *URLDomainPrefixes = NULL;
PUBLIC char *URLDomainSuffixes = NULL;
PUBLIC BOOLEAN startfile_ok = FALSE;
PUBLIC BOOLEAN LYSelectPopups = USE_SELECT_POPUPS;
PUBLIC BOOLEAN LYUseDefSelPop = TRUE;	/* Command line -popup toggle */
PUBLIC int LYMultiBookmarks = MULTI_BOOKMARK_SUPPORT;
PUBLIC BOOLEAN LYMBMBlocked = BLOCK_MULTI_BOOKMARKS;
PUBLIC BOOLEAN LYMBMAdvanced = TRUE;
PUBLIC int LYStatusLine = -1;		 /* Line for statusline() if > -1 */
PUBLIC BOOLEAN LYCollapseBRs = COLLAPSE_BR_TAGS;  /* Collapse serial BRs? */
PUBLIC BOOLEAN LYSetCookies = SET_COOKIES; /* Process Set-Cookie headers? */
PUBLIC char *XLoadImageCommand = NULL;	/* Default image viewer for X */

/* These are declared in cutil.h for current freeWAIS libraries. - FM */
#ifdef DECLARE_WAIS_LOGFILES
PUBLIC char *log_file_name = NULL; /* for WAIS log file name    in libWWW */
PUBLIC FILE *logfile = NULL;	   /* for WAIS log file output  in libWWW */
#endif /* DECLARE_WAIS_LOGFILES */

extern BOOL reloading;	    /* For Flushing Cache on Proxy Server (HTTP.c)  */
extern int HTNewsChunkSize; /* Number of news articles per chunk (HTNews.c) */
extern int HTNewsMaxChunk;  /* Max news articles before chunking (HTNews.c) */

extern int mainloop NOPARAMS;

PRIVATE BOOLEAN anon_restrictions_set = FALSE;
PRIVATE BOOLEAN stack_dump = FALSE;
PRIVATE char *terminal = NULL;
PRIVATE char *pgm;
PRIVATE BOOLEAN number_links = FALSE;
PRIVATE BOOLEAN LYPrependBase = FALSE;

PRIVATE void parse_arg PARAMS((char **arg, int *i, int argc));
#ifndef VMS
PRIVATE void FatalProblem PARAMS((int sig));
#endif /* !VMS */

PRIVATE void free_lynx_globals NOARGS
{
    int i;

#ifndef VMS
    FREE(list_format);
#ifdef SYSLOG_REQUESTED_URLS
    FREE(syslog_txt);
#endif /* SYSLOG_REQUESTED_URLS */
    FREE(lynx_version_putenv_command);
    FREE(NNTPSERVER_putenv_cmd);
    FREE(http_proxy_putenv_cmd);
    FREE(https_proxy_putenv_cmd);
    FREE(ftp_proxy_putenv_cmd);
    FREE(gopher_proxy_putenv_cmd);
    FREE(cso_proxy_putenv_cmd);
    FREE(news_proxy_putenv_cmd);
    FREE(newspost_proxy_putenv_cmd);
    FREE(newsreply_proxy_putenv_cmd);
    FREE(snews_proxy_putenv_cmd);
    FREE(snewspost_proxy_putenv_cmd);
    FREE(snewsreply_proxy_putenv_cmd);
    FREE(nntp_proxy_putenv_cmd);
    FREE(wais_proxy_putenv_cmd);
    FREE(finger_proxy_putenv_cmd);
    FREE(no_proxy_putenv_cmd);
#endif /* !VMS */

#ifdef VMS
    Define_VMSLogical("LYNX_VERSION", "");
    FREE(mail_adrs);
    FREE(LYCSwingPath);
#endif /* VMS */

    FREE(LynxHome);
    FREE(startfile);
    FREE(helpfile);
    FREE(jumpprompt);
#ifdef JUMPFILE
    FREE(jumpfile);
#endif /* JUMPFILE */
    FREE(indexfile);
    FREE(global_type_map);
    FREE(personal_type_map);
    FREE(global_extension_map);
    FREE(personal_extension_map);
    FREE(language);
    FREE(pref_charset);
    FREE(LynxSigFile);
    FREE(system_mail);
    FREE(LYUserAgent);
    FREE(LYUserAgentDefault);
    FREE(LYHostName);
    FREE(LYLocalDomain);
    FREE(lynx_save_space);
    FREE(homepage);
    FREE(helpfilepath);
    FREE(aboutfilepath);
    FREE(bookmark_page);
    FREE(BookmarkPage);
    for (i = 0; i <= MBM_V_MAXFILES; i++) {
        FREE(MBM_A_subbookmark[i]);
        FREE(MBM_A_subdescript[i]);
    }
    FREE(editor);
    FREE(authentication_info[0]);
    FREE(authentication_info[1]);
    FREE(lynxjumpfile);
    FREE(startrealm);
    FREE(personal_mail_address);
    FREE(URLDomainPrefixes);
    FREE(URLDomainSuffixes);
    FREE(XLoadImageCommand);
    for (i = 0; i < nlinks; i++) {
        FREE(links[i].lname);
    }
    nlinks = 0;

    return;
}

/*
 * Wow!  Someone wants to start up Lynx.
 */
PUBLIC int main ARGS2(
	int,		argc,
	char **,	argv)
{
    int  i;		/* indexing variable */
    int status = 0;	/* exit status */
    int len;
    char *lynx_cfg_file = NULL;
    char *temp = NULL;
    char *cp;
    FILE *fp;
    char filename[256];

    /*
     *  Set up the argument list.
     */
    pgm = argv[0];
    if ((cp = strrchr(pgm, '/')) != NULL) {
	pgm = cp + 1;
    }

#ifdef LY_FIND_LEAKS
    /*
     *	Register the final function to be executed when being exited.
     *	Will display memory leaks if LY_FIND_LEAKS is defined.
     */
    atexit(LYLeaks);
#endif /* LY_FIND_LEAKS */
    /*
     *  Register the function which will free our allocated globals.
     */
    atexit(free_lynx_globals);

#ifdef LOCALE
    /*
     *  LOCALE support for international characters.
     */
    setlocale(LC_ALL, "");
#endif /* LOCALE */

    /*
     *  Initialize our startup and global variables.
     */
#ifdef ULTRIX
    /*
     *  Need this for ultrix.
     */
    terminal = getenv("TERM");
    if ((terminal == NULL) || !strncasecomp(terminal, "xterm", 5))
        terminal = "vt100";
#endif /* ULTRIX */
    /*
     *  Zero the links and history struct arrays.
     */
    memset((void *)links, 0, sizeof(linkstruct)*MAXLINKS);
    memset((void *)history, 0, sizeof(histstruct)*MAXHIST);
    /*
     *  Zero the MultiBookmark arrays.
     */
    memset((void *)MBM_A_subbookmark, 0, sizeof(char)*(MBM_V_MAXFILES+1));
    memset((void *)MBM_A_subdescript, 0, sizeof(char)*(MBM_V_MAXFILES+1));
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
    openlog("lynx", LOG_PID, LOG_LOCAL5);
#endif /* SYSLOG_REQUESTED_URLS */
    StrAllocCopy(list_format, LIST_FORMAT);
#endif /* !VMS */
    InfoSecs    = (int)INFOSECS;
    MessageSecs = (int)MESSAGESECS;
    AlertSecs   = (int)ALERTSECS;
    StrAllocCopy(helpfile, HELPFILE);
    StrAllocCopy(startfile, STARTFILE);
    LYTrimHead(startfile);
    if (!strncasecomp(startfile, "lynxexec:", 9) ||
        !strncasecomp(startfile, "lynxprog:", 9)) {
	/*
	 *  The original implementions of these schemes expected
	 *  white space without hex escaping, and did not check
	 *  for hex escaping, so we'll continue to support that,
	 *  until that code is redone in conformance with SGML
	 *  principles.  - FM
	 */
	HTUnEscapeSome(startfile, " \r\n\t");
	convert_to_spaces(startfile, TRUE);
    }
    StrAllocCopy(jumpprompt, JUMP_PROMPT);
#ifdef JUMPFILE
    StrAllocCopy(jumpfile, JUMPFILE);
    {
        temp = (char *)malloc(strlen(jumpfile) + 10);
	if (!temp) {
	    outofmem(__FILE__, "main");
	} else {
	    sprintf(temp, "JUMPFILE:%s", jumpfile);
	    if (!LYJumpInit(temp)) {
		if (TRACE)
		    fprintf(stderr, "Failed to register %s\n", temp);
	    }
	    FREE(temp);
	}
    }
#endif /* JUMPFILE */
    StrAllocCopy(indexfile, DEFAULT_INDEX_FILE);
    StrAllocCopy(global_type_map, GLOBAL_MAILCAP);
    StrAllocCopy(personal_type_map, PERSONAL_MAILCAP);
    StrAllocCopy(global_extension_map, GLOBAL_EXTENSION_MAP);
    StrAllocCopy(personal_extension_map, PERSONAL_EXTENSION_MAP);
    StrAllocCopy(language, PREFERRED_LANGUAGE);
    StrAllocCopy(pref_charset, PREFERRED_CHARSET);
    StrAllocCopy(system_mail, SYSTEM_MAIL);
    if ((cp = getenv("LYNX_TEMP_SPACE")) != NULL)
        StrAllocCopy(lynx_temp_space, cp);
    else
        StrAllocCopy(lynx_temp_space, TEMP_SPACE);
    if ((cp = strchr(lynx_temp_space, '~'))) {
	*(cp++) = '\0';
	StrAllocCopy(temp, lynx_temp_space);
	if (((len = strlen(temp)) > 0) && temp[len-1] == '/')
	    temp[len-1] = '\0';
#ifdef VMS
	StrAllocCat(temp, HTVMS_wwwName((char *)Home_Dir()));
#else
	StrAllocCat(temp, Home_Dir());
#endif /* VMS */
	StrAllocCat(temp, cp);
	StrAllocCopy(lynx_temp_space, temp);
	FREE(temp);
    }
    StrAllocCopy(LYUserAgent, LYNX_NAME);
    StrAllocCat(LYUserAgent, "/");
    StrAllocCat(LYUserAgent, LYNX_VERSION);
    if (HTLibraryVersion) {
        StrAllocCat(LYUserAgent, " libwww-FM/");
	StrAllocCat(LYUserAgent, HTLibraryVersion);
    }
    StrAllocCopy(LYUserAgentDefault, LYUserAgent);
#ifdef VMS
    Define_VMSLogical("LYNX_VERSION", LYNX_VERSION);
#else
    StrAllocCopy(lynx_version_putenv_command, "LYNX_VERSION=");
    StrAllocCat(lynx_version_putenv_command, LYNX_VERSION);
    putenv(lynx_version_putenv_command);
#endif /* VMS */
#ifdef VMS
    for (i = 0; lynx_temp_space[i]; i++)
        lynx_temp_space[i] = TOLOWER(lynx_temp_space[i]);
    if (strchr(lynx_temp_space, '/') != NULL) {
	if ((len = strlen(lynx_temp_space)) == 1) {
	    StrAllocCopy(lynx_temp_space, "sys$scratch:");
	} else {
	    if (lynx_temp_space[len-1] != '/')
	        StrAllocCat(lynx_temp_space, "/");
            StrAllocCopy(temp, HTVMS_name("", lynx_temp_space));
	    StrAllocCopy(lynx_temp_space, temp);
	    FREE(temp);
	}
    }
    if (strchr(lynx_temp_space, ':') == NULL &&
	strchr(lynx_temp_space, ']') == NULL) {
	StrAllocCat(lynx_temp_space, ":");
    }
#else
    {
        len;
	if (((len = strlen(lynx_temp_space)) > 1) &&
	    lynx_temp_space[len-1] != '/') {
	    StrAllocCat(lynx_temp_space, "/");
	}
    }
#endif /* VMS */
#ifdef VMS
    StrAllocCopy(mail_adrs, MAIL_ADRS);
#ifdef CSWING_PATH
    StrAllocCopy(LYCSwingPath, CSWING_PATH);
#endif /* CSWING_PATH */
#endif /* VMS */
#ifdef LYNX_HOST_NAME
    StrAllocCopy(LYHostName, LYNX_HOST_NAME);
#else
    StrAllocCopy(LYHostName, HTHostName());
#endif /* LYNX_HOST_NAME */
    StrAllocCopy(LYLocalDomain, LOCAL_DOMAIN);
    StrAllocCopy(URLDomainPrefixes, URL_DOMAIN_PREFIXES);
    StrAllocCopy(URLDomainSuffixes, URL_DOMAIN_SUFFIXES);
    StrAllocCopy(XLoadImageCommand, XLOADIMAGE_COMMAND);
    /*
     *  Set up the compilation default character set. - FM
     */
    for (i = 0; LYchar_set_names[i]; i++) {
	if (!strncmp(CHARACTER_SET, LYchar_set_names[i],
		     strlen(CHARACTER_SET))) {
	    current_char_set=i;
	    break;
	}
    }
    if (!LYchar_set_names[i])
        current_char_set = i = 0;
    HTMLSetRawModeDefault(i);

    /*
     *  Disable news posting if the compilation-based
     *  LYNewsPosting value is FALSE.  This may be changed
     *  further down via lynx.cfg or the -restriction
     *  command line switch. - FM
     */
    no_newspost = (LYNewsPosting == FALSE);

    /*
     *  Set up trace, the anonymous account defaults, and/or
     *  the nosocks flag, if requested, and an alternate
     *  configuration file, if specified, NOW.  Also, if
     *  we only want the help menu, output that and exit. - FM
     */
    for (i=1; i<argc; i++) {
	if (strncmp(argv[i], "-trace", 6) == 0) {
	    WWW_TraceFlag = TRUE;
	} else if (strncmp(argv[i], "-anonymous", 10) == 0) {
	    parse_restrictions("default");
	    anon_restrictions_set = TRUE;
#ifdef SOCKS
	} else if (strncmp(argv[i], "-nosocks", 8) == 0) {
	    socks_flag = FALSE;
#endif /* SOCKS */
	} else if (strncmp(argv[i], "-cfg", 4) == 0) {
	    if ((cp=strchr(argv[i],'=')) != NULL)
                StrAllocCopy(lynx_cfg_file, cp+1);
            else {
                StrAllocCopy(lynx_cfg_file, argv[i+1]);
                i++;
            }
	} else if (strncmp(argv[i], "-help", 5) == 0) {
	    parse_arg(&argv[i], &i, argc);
	}
    }
#ifdef SOCKS
    if (socks_flag)
        SOCKSinit(argv[0]);
#endif /* SOCKS */

    /*
     *  If we didn't get and act on an -anonymous switch,
     *  but can verify that this is the anonymous account,
     *  set the default restrictions for that account NOW. - FM
     */
    if (!anon_restrictions_set && strlen((char *)ANONYMOUS_USER) > 0 &&
#ifdef VMS
	!strcasecomp(((char *)getenv("USER")==NULL ? " " : getenv("USER")),
		     ANONYMOUS_USER)) {
#else
#ifdef NO_CUSERID
        STREQ(((char *)getlogin()==NULL ? " " : getlogin()), ANONYMOUS_USER)) {
#else
        STREQ((char *)cuserid((char *) NULL), ANONYMOUS_USER)) {
#endif /* NO_CUSERID */
#endif /* VMS */
	parse_restrictions("default");
	anon_restrictions_set = TRUE;
    }
    if (TRACE && anon_restrictions_set) {
	fprintf(stderr, "LYMain: Anonymous restrictions set.\n");
    }

    /*
     *  If no alternate configuration file was specified on
     *  the command line, see if it's in the environment.
     */
    if (!lynx_cfg_file) {
        if (((cp=getenv("LYNX_CFG")) != NULL) ||
	    (cp=getenv("lynx_cfg")) != NULL)
	    StrAllocCopy(lynx_cfg_file, cp);
    }

    /*
     *  If we still don't have a configuration file,
     *  use the userdefs.h definition.
     */
    if (!lynx_cfg_file)
    	StrAllocCopy(lynx_cfg_file, LYNX_CFG_FILE);

    /*
     *  Convert a '~' in the configuration file path to $HOME.
     */
    if ((cp = strchr(lynx_cfg_file, '~'))) {
	*(cp++) = '\0';
	StrAllocCopy(temp, lynx_cfg_file);
	if (((len = strlen(temp)) > 0) && temp[len-1] == '/')
	    temp[len-1] = '\0';
#ifdef VMS
	StrAllocCat(temp, HTVMS_wwwName((char *)Home_Dir()));
#else
	StrAllocCat(temp, Home_Dir());
#endif /* VMS */
	StrAllocCat(temp, cp);
	StrAllocCopy(lynx_cfg_file, temp);
	FREE(temp);
    }

    /*
     *  If the configuration file is not available,
     *  inform the user and exit.
     */
    if ((fp = fopen(lynx_cfg_file, "r")) == NULL) {
        fprintf(stderr, "\nConfiguration file %s is not available.\n\n",
			lynx_cfg_file);
	exit(-1);
    }
    fclose(fp);

    /*
     *  Make sure we have the edit map declared. - FM
     */
    if (!LYEditmapDeclared()) {
        fprintf(stderr, "\nLynx edit map not declared.\n\n");
	exit(-1);
    }

#ifdef USE_SLANG
    /*
     *  Set up default foreground and background colors.
     */
    lynx_setup_colors();
#endif /* USE_SLANG */

    /*
     *  Set the compilation default signature file. - FM
     */
    strcpy(filename, LYNX_SIG_FILE);
    if (LYPathOffHomeOK(filename, sizeof(filename))) {
    	StrAllocCopy(LynxSigFile, filename);
	LYAddPathToHome(filename, sizeof(filename), LynxSigFile);
	StrAllocCopy(LynxSigFile, filename);
	if (TRACE)
	    fprintf(stderr, "LYNX_SIG_FILE set to '%s'\n", LynxSigFile);
    } else {
	if (TRACE)
	    fprintf(stderr, "LYNX_SIG_FILE '%s' is bad. Ignoring.\n",
	    		    LYNX_SIG_FILE);
    }

    /*
     *  Process the configuration file.
     */
    read_cfg(lynx_cfg_file);
    FREE(lynx_cfg_file);

    /*
     *  Check for a save space path in the environment.
     *  If one was set in the configuration file, that
     *  one will be overridden. - FM
     */
    if ((cp=getenv("LYNX_SAVE_SPACE")) != NULL)
        StrAllocCopy(lynx_save_space, cp);

    /*
     *  We have a save space path, make sure it's valid. - FM
     */
    if (lynx_save_space && *lynx_save_space == '\0') {
        FREE(lynx_save_space);
    }
    if (lynx_save_space) {
        if ((cp = strchr(lynx_save_space, '~')) != NULL) {
	    *(cp++) = '\0';
	    StrAllocCopy(temp, lynx_save_space);
	    if (((len = strlen(temp)) > 0) && temp[len-1] == '/')
	        temp[len-1] = '\0';
#ifdef VMS
	    StrAllocCat(temp, HTVMS_wwwName((char *)Home_Dir()));
#else
	    StrAllocCat(temp, Home_Dir());
#endif /* VMS */
	    StrAllocCat(temp, cp);
	    StrAllocCopy(lynx_save_space, temp);
	    FREE(temp);
        }
#ifdef VMS
        for (i = 0; lynx_save_space[i]; i++)
            lynx_save_space[i] = TOLOWER(lynx_save_space[i]);
        if (strchr(lynx_save_space, '/') != NULL) {
	    if ((len = strlen(lynx_save_space)) == 1) {
	        StrAllocCopy(lynx_save_space, "sys$login:");
	    } else {
	        if (lynx_save_space[len-1] != '/')
	            StrAllocCat(lynx_save_space, "/");
                StrAllocCopy(temp, HTVMS_name("", lynx_save_space));
	        StrAllocCopy(lynx_save_space, temp);
	        FREE(temp);
	    }
        }
	if (strchr(lynx_save_space, ':') == NULL &&
	    strchr(lynx_save_space, ']') == NULL) {
	    StrAllocCat(lynx_save_space, ":");
	}
#else
    {
	if (((len = strlen(lynx_save_space)) > 1) &&
	    lynx_save_space[len-1] != '/') {
	    StrAllocCat(lynx_save_space, "/");
	}
    }
#endif /* VMS */
    }

    /*
     *  Set up the file extension and mime type maps from
     *  src/HTInit.c and the global and personal mime.types
     *  and mailcap files.  These will override any SUFFIX
     *  or VIEWER maps in userdefs.h or the configuration
     *  file, if they overlap.
     */
    HTFormatInit();
    HTFileInit();

    /*
     *  Get WWW_HOME environment variable if it exists.
     */
    if ((cp = getenv("WWW_HOME")) != NULL) {
	StrAllocCopy(startfile, cp);
	LYTrimHead(startfile);
	if (!strncasecomp(startfile, "lynxexec:", 9) ||
	    !strncasecomp(startfile, "lynxprog:", 9)) {
	    /*
	     *  The original implementions of these schemes expected
	     *  white space without hex escaping, and did not check
	     *  for hex escaping, so we'll continue to support that,
	     *  until that code is redone in conformance with SGML
	     *  principles.  - FM
	     */
	    HTUnEscapeSome(startfile, " \r\n\t");
	    convert_to_spaces(startfile, TRUE);
	}
    }

    /*
     *  Set the LynxHome URL.  If it's a file URL and the
     *  host is defaulted, force in "//localhost", and if
     *  it's not an absolute URL, make it one. - FM
     */
    StrAllocCopy(LynxHome, startfile);
    LYFillLocalFileURL((char **)&LynxHome, "file://localhost");
    LYEnsureAbsoluteURL((char **)&LynxHome, "LynxHome");

    /*
     *  Process arguments - with none, look for the database in STARTDIR,
     *  starting with STARTFILE.
     *
     *  If a pathname is given, use it as the starting point.  Split it
     *  into directory and file components, 'cd' to the directory, and
     *  view the file.
     *
     *  If the only argument is '-' then we expect to see the arguments on
     *  stdin, this is to allow for the potentially very long command line
     *  that can be associated with post or get data.
     */
    if (argc == 2 && strcmp(argv[1], "-") == 0) {
	char buf[1025];
	char *argv[2];
 
	argv[0] = buf;
	argv[1] = NULL;
 
	while (fgets(buf, sizeof(buf) - 1, stdin)) {
	    int j;
     
	    for (j = strlen(buf) - 1; j > 0 && 
		(buf[j] == CR || buf[j] == LF); j--) {
		buf[j] = '\0';
	    }
	    parse_arg(argv, NULL, -1);
	}
    } else {
	for (i = 1; i < argc; i++) {
	    parse_arg(&argv[i], &i, argc);
	}
    }

#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
    if (syslog_txt) {
	syslog(LOG_INFO, "Session start:%s", syslog_txt);
    } else {
	syslog(LOG_INFO, "Session start");
    }
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

    /*
     *  Process the RC file.
     */
    read_rc();

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
#ifdef NEVER_ALLOW_REMOTE_EXEC
    if (local_exec) {
        local_exec = FALSE;
	local_exec_on_local_files=TRUE;
    }
#endif /* NEVER_ALLOW_REMOTE_EXEC */
#endif /* EXEC_LINKS || EXEC_SCRIPTS */

    if (emacs_keys)
        set_emacs_keys();
 
    if (vi_keys)
        set_vi_keys();
 
    if (number_links)
	keypad_mode = LINKS_ARE_NUMBERED;
    if (keypad_mode == NUMBERS_AS_ARROWS)
        set_numbers_as_arrows();

    /*
     *  Check the -popup command line toggle. - FM
     */
    if (LYUseDefSelPop == FALSE) {
        if (LYSelectPopups == TRUE)
	    LYSelectPopups = FALSE;
	else
	    LYSelectPopups = TRUE;
    }

    /*
     *  Disable multiple bookmark support if not interactive,
     *  so it doesn't crash on curses functions, or if the
     *  support was blocked via userdefs.h and/or lynx.cfg,
     *  or via command line restrictions. - FM
     */
    if (no_multibook)
        LYMBMBlocked = TRUE;
    if (dump_output_immediately || LYMBMBlocked || no_multibook) {
        LYMultiBookmarks = FALSE;
	LYMBMBlocked = TRUE;
	no_multibook == TRUE;
    }

#ifdef VMS
    set_vms_keys();
#endif /* VMS */

    /* trap interrupts */    
    if (!dump_output_immediately)
        (void) signal(SIGHUP, cleanup_sig);
    (void) signal(SIGTERM, cleanup_sig);
#ifdef SIGWINCH
    (void) signal(SIGWINCH, size_change);
#endif /* SIGWINCH */
#ifndef VMS
    if (!TRACE && !dump_output_immediately && !stack_dump) {
        (void) signal(SIGINT, cleanup_sig);
#ifndef __linux__
        (void) signal(SIGBUS, FatalProblem);
#endif /* !__linux__ */
        (void) signal(SIGSEGV, FatalProblem);
        (void) signal(SIGILL, FatalProblem);
        /*
	 *  Since we're doing lots of TCP, just ignore SIGPIPE altogether.
	 *
	 *  HTTCP.c should deal with a broken pipe for servers.
	 *  Rick Mallet's check after c = GetChar() in LYStrings.c should
	 *   deal with a disconnected terminal.
	 *  So the runaway CPU time problem on Unix should not occur any
	 *   more.
	 */
        (void) signal(SIGPIPE, SIG_IGN);
    }
#endif /* !VMS */

    /*
     *  Set up the proper character set with the desired
     *  startup raw 8-bit or CJK mode handling.  - FM
     */
    HTMLUseCharacterSet(current_char_set);

    /*
     *  If startfile is a file URL and the host is defaulted,
     *  force in "//localhost", and if it's not an absolute URL,
     *  make it one. - FM
     */
    LYFillLocalFileURL((char **)&startfile, "file://localhost");
    LYEnsureAbsoluteURL((char **)&startfile, "STARTFILE");

    /*
     *  If homepage was specified and is a file URL with the
     *  host defaulted, force in "//localhost", and if it's
     *  not an absolute URL, make it one. - FM
     */
    if (homepage) {
        LYFillLocalFileURL((char **)&homepage, "file://localhost");
	LYEnsureAbsoluteURL((char **)&homepage, "HOMEPAGE");
    }

    /*
     *  If we don't have a homepage specified,
     *  set it to startfile.  Otherwise, reset
     *  LynxHome. - FM
     */
    if (!(homepage && *homepage)) {
	StrAllocCopy(homepage, startfile);
    } else {
        StrAllocCopy(LynxHome, homepage);
    }

    /*
     *  Set up the inside/outside domain restriction flags. - FM
     */
    if (inlocaldomain()) {
#if defined(NO_UTMP) || defined(VMS) /* not selective */
        telnet_ok = !no_inside_telnet && !no_outside_telnet && telnet_ok;
	news_ok = !no_inside_news && !no_outside_news && news_ok;
	ftp_ok = !no_inside_ftp && !no_outside_ftp && ftp_ok;
	rlogin_ok = !no_inside_rlogin && !no_outside_rlogin && rlogin_ok;
#else
	if (TRACE)
	   fprintf(stderr,"LYMain.c: User in Local domain\n");
        telnet_ok = !no_inside_telnet && telnet_ok;
	news_ok = !no_inside_news && news_ok;
	ftp_ok = !no_inside_ftp && ftp_ok;
	rlogin_ok = !no_inside_rlogin && rlogin_ok;
#endif /* NO_UTMP || VMS */
    } else {
	if (TRACE)
	   fprintf(stderr,"LYMain.c: User in REMOTE domain\n");
        telnet_ok = !no_outside_telnet && telnet_ok;
	news_ok = !no_outside_news && news_ok;
	ftp_ok = !no_outside_ftp && ftp_ok;
	rlogin_ok = !no_outside_rlogin && rlogin_ok;
    }

#ifdef SIGTSTP
    /*
     *  Block Control-Z suspending if requested. - FM
     */
    if (no_suspend)
	(void) signal(SIGTSTP,SIG_IGN);
#endif /* SIGTSTP */

    /*
     *  Check for a valid HEAD request. - FM
     */
    if (HEAD_request && strncmp(startfile, "http", 4)) {
        fprintf(stderr,
 "The '-head' switch is for http HEAD requests and cannot be used for\n'%s'.\n",
		startfile);
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

    /*
     *  Check for a valid MIME headers request. - FM
     */
    if (keep_mime_headers && strncmp(startfile, "http", 4)) {
        fprintf(stderr,
 "The '-mime_header' switch is for http URLs and cannot be used for\n'%s'.\n",
		startfile);
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

    /*
     *  Check for a valid traversal request. - FM
     */
    if (traversal && strncmp(startfile, "http", 4)) {
        fprintf(stderr,
 "The '-traversal' switch is for http URLs and cannot be used for\n'%s'.\n",
		startfile);
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

    /*
     *  Set up our help and about file base paths. - FM
     */
    StrAllocCopy(helpfilepath, helpfile);
    if ((cp=strrchr(helpfilepath, '/')) != NULL)
        *cp = '\0';
    StrAllocCopy(aboutfilepath, helpfilepath);
    if ((cp=strrchr(aboutfilepath, '/')) != NULL) {
        *cp = '\0';
	StrAllocCat(aboutfilepath, "/about_lynx/");
    }
    StrAllocCat(helpfilepath, "/");


    /*
     *  Make sure our bookmark default strings
     *  are all allocated and synchronized. - FM
     */
    if (!bookmark_page || *bookmark_page == '\0') {
        StrAllocCopy(bookmark_page, "lynx_bookmarks.html");
        StrAllocCopy(BookmarkPage, bookmark_page);
        StrAllocCopy(MBM_A_subbookmark[0], bookmark_page);
        StrAllocCopy(MBM_A_subdescript[0], "Default");
    }
    if (!BookmarkPage || *BookmarkPage == '\0') {
        StrAllocCopy(BookmarkPage, bookmark_page);
        StrAllocCopy(MBM_A_subbookmark[0], bookmark_page);
        StrAllocCopy(MBM_A_subdescript[0], MULTIBOOKMARKS_DEFAULT);
    }

    /*
     *  Here's where we do all the work.
     */
    if (dump_output_immediately) {
        /*
	 *  Finish setting up and start a
	 *  NON-INTERACTIVE session. - FM
	 */
        if (crawl && !number_links) {
	    keypad_mode = NUMBERS_AS_ARROWS;
	} else if (!nolist) {
	    keypad_mode = LINKS_ARE_NUMBERED;
	}
	if (display != NULL && *display != '\0') {
	    LYisConfiguredForX = TRUE;
	}
	status = mainloop();
        if (!nolist && keypad_mode == LINKS_ARE_NUMBERED)
	    printlist(stdout,FALSE);
        (void) signal(SIGHUP, SIG_DFL);
        (void) signal(SIGTERM, SIG_DFL);
#ifndef VMS
        (void) signal(SIGINT, SIG_DFL);
#endif /* !VMS */
#ifdef SIGTSTP
	if (no_suspend)
	  (void) signal(SIGTSTP,SIG_DFL);
#endif /* SIGTSTP */
    } else {
        /*
	 *  Finish setting up and start an
	 *  INTERACTIVE session. - FM
	 */
 	if (setup(terminal)) {
	    if (display != NULL && *display != '\0') {
	        LYisConfiguredForX = TRUE;
	    }
	    status = mainloop();
	    cleanup();
	}
    }
 
    exit(status);
}

/*
 *  Called by HTAccessInit to register any protocols supported by lynx.
 *  Protocols added by lynx:
 *    LYNXKEYMAP, lynxcgi, LYNXIMGMAP, LYNXCOOKIE
 */
#ifdef GLOBALREF_IS_MACRO
extern GLOBALREF (HTProtocol,LYLynxKeymap);
extern GLOBALREF (HTProtocol,LYLynxCGI);
extern GLOBALREF (HTProtocol,LYLynxIMGmap);
extern GLOBALREF (HTProtocol,LYLynxCookies);
#else
GLOBALREF  HTProtocol LYLynxKeymap;
GLOBALREF  HTProtocol LYLynxCGI;
GLOBALREF  HTProtocol LYLynxIMGmap;
GLOBALREF  HTProtocol LYLynxCookies;
#endif /* GLOBALREF_IS_MACRO */

PUBLIC void LYRegisterLynxProtocols NOARGS
{
    HTRegisterProtocol(&LYLynxKeymap);
    HTRegisterProtocol(&LYLynxCGI);
    HTRegisterProtocol(&LYLynxIMGmap);
    HTRegisterProtocol(&LYLynxCookies);
}

/*
 *  Parse one argument, optionally picking up the next entry in argv (if
 *  appropriate).
 */

PRIVATE char * scan3D ARGS2(
	char **,	argv,
	int *,		i)
{
    char *result;

    if ((result=strchr(argv[0],'=')) != NULL)
        return result+1;
    if (argv[1] && i)
        (*i)++; /* Let master know we've stolen an argument */
    return argv[1];
}

PRIVATE void parse_arg ARGS3(
	char **,	argv,
	int *,		i,
	int,		argc)
{
    char *cp;
#ifndef VMS
    static char display_putenv_command[142];
#endif /* !VMS */
#define nextarg ((cp=scan3D(&argv[0], i))!=NULL)

    /*
     *  Check for a command line startfile. - FM
     */
    if (argv[0][0] != '-') {
	StrAllocCopy(startfile, argv[0]);
	LYTrimHead(startfile);
	if (!strncasecomp(startfile, "lynxexec:", 9) ||
	    !strncasecomp(startfile, "lynxprog:", 9)) {
	    /*
	     *  The original implementions of these schemes expected
	     *  white space without hex escaping, and did not check
	     *  for hex escaping, so we'll continue to support that,
	     *  until that code is redone in conformance with SGML
	     *  principles.  - FM
	     */
	    HTUnEscapeSome(startfile, " \r\n\t");
	    convert_to_spaces(startfile, TRUE);
	}
	return;
    }

    switch (TOLOWER(argv[0][1])) {

    case 'a':
    if (strncmp(argv[0], "-anonymous", 10) == 0) {
        /*
	 *  Should already have been set, so we don't
	 *  override or replace any additional
	 *  restrictions from the command line. - FM
	 */
	if (!anon_restrictions_set)
	    parse_restrictions("default");
	    anon_restrictions_set = TRUE;

    } else if (strncmp(argv[0], "-auth", 5) == 0) {
        /*
	 *  Authentication information for protected forms.
	 */
	char *auth_info = NULL;
	
	if (nextarg) {
	    StrAllocCopy(auth_info, cp);
	    memset(cp, ' ', strlen(cp));/* Let's not show too much */
	}
        if (auth_info != NULL)  {
	    if ((cp = strchr(auth_info, ':')) != NULL) { /* Pw */
		*cp++ = '\0';	/* Terminate ID */
		if (*cp) {
		    HTUnEscape(cp);
		    StrAllocCopy(authentication_info[1], cp);
		}
	    }
	    if (*auth_info) { /* Id */
	        HTUnEscape(auth_info);
		StrAllocCopy(authentication_info[0], auth_info);
	    }
	    FREE(auth_info);
	}

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'b':
    if (strcmp(argv[0], "-base") == 0) {
        /*
	 *  Treat -source equivalently to an interactive
	 *  download, so that a BASE tag is prepended for
	 *  text/html content types. - FM
	 */
	LYPrependBase = TRUE;
	if (HTOutputFormat == HTAtom_for("www/dump"))
	    HTOutputFormat = HTAtom_for("www/download");

    } else if (strcmp(argv[0], "-book") == 0) {
        /*
	 *  Use bookmarks as startfile.
	 */
	bookmark_start = TRUE;

    } else if (strcmp(argv[0], "-buried_news") == 0) {
        /*
	 *  Toggle scans for buried news references.
	 */
        if (scan_for_buried_news_references)
	    scan_for_buried_news_references = FALSE;
	else
	    scan_for_buried_news_references = TRUE;

#ifdef USE_SLANG
    } else if (strncmp(argv[0], "-blink", 6) == 0) {
        Lynx_Color_Flags |= SL_LYNX_USE_BLINK;
#endif /* USE_SLANG */

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'c':
    if (strncmp(argv[0], "-cache", 6) == 0) {
	if (nextarg)
	    HTCacheSize = atoi(cp);
	/*
	 *  Limit size.
	 */
	if (HTCacheSize < 2) HTCacheSize = 2;

    } else if (strncmp(argv[0], "-case", 5) == 0) {
	case_sensitive = TRUE;

    } else if (strncmp(argv[0], "-cfg", 4) == 0) {
	/*
	 *  Already read the alternate configuration file 
	 *  so just check whether we need to increment i
	 */
	if (nextarg)
	    ; /* do nothing */

    } else if (strncmp(argv[0], "-child", 6) == 0) {
	child_lynx = TRUE;
	no_disk_save = TRUE;

#ifdef USE_SLANG
    } else if (strncmp(argv[0], "-color", 6) == 0) {
        Lynx_Color_Flags |= SL_LYNX_USE_COLOR;
#endif /* USE_SLANG */

    } else if (strncmp(argv[0], "-crawl", 6) == 0) {
	crawl = TRUE;
	LYcols = 80;

    } else if (strncmp(argv[0], "-cookies", 8) == 0) {
        if (LYSetCookies)
	    LYSetCookies = FALSE;
	else
	    LYSetCookies = TRUE;

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'd':
    if (strncmp(argv[0], "-display", 8) == 0) {
	if (nextarg) {
#ifdef VMS
	    int j;
	    for (j = 0; cp[j]; j++)
	        cp[j] = TOUPPER(cp[j]);
	    Define_VMSLogical(DISPLAY, cp ? cp : "");
#else
	    sprintf(display_putenv_command, "DISPLAY=%s", cp ? cp : "");
	    putenv(display_putenv_command);
#endif /* VMS */
	    if ((cp = getenv(DISPLAY)) != NULL && *cp != '\0') {
		display = cp;
	    }
	}

    } else if (strncmp(argv[0], "-dump", 5) == 0) {
	dump_output_immediately = TRUE;
	LYcols=80;

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'e':
    if (strncmp(argv[0], "-editor", 7) == 0) {
	if (nextarg)
	    StrAllocCopy(editor,cp);
	system_editor = TRUE;

    } else if (strncmp(argv[0], "-emacskeys", 10) == 0) {
	emacs_keys = TRUE;

    } else if (strncmp(argv[0], "-enable_scrollback", 18) == 0) {
        if (enable_scrollback)
	    enable_scrollback = FALSE;
	else
	    enable_scrollback = TRUE;

    } else if (strncmp(argv[0], "-error_file", 11) == 0) {
        /*
	 *  Output return (success/failure) code
	 *  of an HTTP transaction.
	 */
	if (nextarg)
	    http_error_file = cp;

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
    } else if (strncmp(argv[0], "-exec", 5) == 0) {
#ifndef NEVER_ALLOW_REMOTE_EXEC
	local_exec = TRUE;
#else
	local_exec_on_local_files = TRUE;
#endif /* NEVER_ALLOW_REMOTE_EXEC */
#endif /* EXEC_LINKS || EXEC_SCRIPTS */

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'f':
    if (strncmp(argv[0], "-force_html", 11) == 0) {
	LYforce_HTML_mode = TRUE;

    } else if (strncmp(argv[0], "-fileversions", 13) == 0) {
#ifdef VMS
	HTVMSFileVersions = TRUE;
#else
	break;;
#endif /* VMS */
	
    } else if (strncmp(argv[0], "-ftp", 4) == 0) {
	ftp_ok = FALSE;

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'g':
    if (strcmp(argv[0], "-get_data") == 0) {
        /*
	 *  User data for GET form.
	 */
	char **get_data;
	char buf[1024];

        /*
	 *  On Unix, conflicts with curses when interactive
         *  so let's force a dump.  - CL
	 *
	 *  On VMS, mods have been made in LYCurses.c to deal with
	 *  potential conflicts, so don't force the dump here. - FM
         */
#ifndef VMS
	dump_output_immediately = TRUE;
        LYcols = 80;
#endif /* VMS */

	StrAllocCopy(form_get_data, "?");   /* Prime the pump */
	get_data = &form_get_data;

	/*
	 *  Build GET data for later.  Stop reading when we see a line
	 *  with "---" as its first three characters.
	 */
	while (fgets(buf, sizeof(buf), stdin) &&
	       strncmp(buf, "---", 3) != 0) {
	    int j;

	    for (j = strlen(buf) - 1; j >= 0 && /* Strip line terminators */
		(buf[j] == CR || buf[j] == LF); j--) {
		buf[j] = '\0';
	    }
	    StrAllocCat(*get_data, buf);
	}

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'h':
    if (strcmp(argv[0], "-help") == 0) {
        goto Output_Help_List;

    } else if (strcmp(argv[0], "-head") == 0) {
        /*
	 *  Return mime headers.
	 */
	HEAD_request = TRUE;

    } else if (strncmp(argv[0], "-historical", 11) == 0) {
        if (historical_comments)
	    historical_comments = FALSE;
	else
	    historical_comments = TRUE;

    } else if (strncmp(argv[0], "-homepage", 9) == 0) {
	if (nextarg) {
	    StrAllocCopy(homepage, cp);
	    LYTrimHead(homepage);
	    if (!strncasecomp(homepage, "lynxexec:", 9) ||
	        !strncasecomp(homepage, "lynxprog:", 9)) {
		/*
		 *  The original implementions of these schemes expected
		 *  white space without hex escaping, and did not check
		 *  for hex escaping, so we'll continue to support that,
		 *  until that code is redone in conformance with SGML
		 *  principles.  - FM
		 */
		HTUnEscapeSome(homepage, " \r\n\t");
		convert_to_spaces(homepage, TRUE);
	    }
	}
    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'i':
    if (strncmp(argv[0], "-image_links", 12) == 0) {
	if (clickable_images)
	    clickable_images = FALSE;
	else
	    clickable_images = TRUE;

    } else if (strncmp(argv[0], "-index", 6) == 0) {
	if (nextarg)
	    StrAllocCopy(indexfile, cp);

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'l':
    if (strncmp(argv[0], "-link", 5) == 0) {
        if (nextarg)
	    ccount = atoi(cp);

    } else if (strncmp(argv[0], "-localhost", 10) == 0) {
	local_host_only = TRUE;

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
    } else if (strncmp(argv[0], "-locexec", 8) == 0) {
	local_exec_on_local_files = TRUE;
#endif /* EXEC_LINKS || EXEC_SCRIPTS */

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'm':
    if (strcmp(argv[0], "-mime_header") == 0) {
        /*
	 *  Include mime headers and force source dump.
	 */
	keep_mime_headers = TRUE;
	dump_output_immediately = TRUE;
	HTOutputFormat = (LYPrependBase ?
	     HTAtom_for("www/download") : HTAtom_for("www/dump"));
	LYcols=999;

    } else if (strncmp(argv[0], "-minimal", 11) == 0) {
        if (minimal_comments)
	    minimal_comments = FALSE;
	else
	    minimal_comments = TRUE;

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'n':
    if (strncmp(argv[0], "-newschunksize", 14) == 0) {
        if (nextarg) {
	    HTNewsChunkSize = atoi(cp);
	    /*
	     * If the new HTNewsChunkSize exceeds the maximum,
	     * increase HTNewsMaxChunk to this size. - FM
	     */
	    if (HTNewsChunkSize > HTNewsMaxChunk) {
	        HTNewsMaxChunk = HTNewsChunkSize; 
	    }
	}

    } else if (strncmp(argv[0], "-newsmaxchunk", 13) == 0) {
        if (nextarg) {
	    HTNewsMaxChunk = atoi(cp);
	    /*
	     * If HTNewsChunkSize exceeds the new maximum,
	     * reduce HTNewsChunkSize to this maximum. - FM
	     */
	    if (HTNewsChunkSize > HTNewsMaxChunk) {
	        HTNewsChunkSize = HTNewsMaxChunk;
	    }
	}

    } else if (strncmp(argv[0], "-nobrowse", 9) == 0) {
	HTDirAccess = HT_DIR_FORBID;

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
    } else if (strncmp(argv[0], "-noexec", 7) == 0) {
	local_exec = FALSE;
#endif /* EXEC_LINKS || EXEC_SCRIPTS */

    } else if (strncmp(argv[0], "-nofilereferer", 14) == 0) {
	no_filereferer = TRUE;

    } else if (strncmp(argv[0], "-nofrom", 7) == 0) {
	LYNoFromHeader = TRUE;

    } else if (strncmp(argv[0], "-nolist", 7) == 0) {
	nolist = TRUE;

    } else if (strncmp(argv[0], "-nolog", 6) == 0) {
	error_logging = FALSE;

    } else if (strcmp(argv[0], "-nopause") == 0) { /* No statusline pauses */
        InfoSecs = 0;
	MessageSecs = 0;
	AlertSecs = 0;

    } else if (strncmp(argv[0], "-noprint", 8) == 0) {
	no_print = TRUE;

    } else if (strcmp(argv[0], "-noredir") == 0) {
        /*
	 *  Don't follow URL redirections.
	 */
	no_url_redirection = TRUE;

    } else if (strncmp(argv[0], "-noreferer", 10) == 0) {
	LYNoRefererHeader = TRUE;

#ifdef SOCKS	
    } else if (strncmp(argv[0], "-nosocks", 8) == 0) {
	socks_flag = FALSE;
#endif /* SOCKS */

    } else if (strncmp(argv[0], "-nostatus", 9) == 0)	{
	no_statusline = TRUE;

    } else if (strncmp(argv[0], "-number_links", 9) == 0) {
        number_links = TRUE;

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'p':
    if (strncmp(argv[0], "-popup", 6) == 0) {
	LYUseDefSelPop = FALSE;

    } else if (strcmp(argv[0], "-post_data") == 0) {
        /*
	 *  User data for POST form.
	 */
	char **post_data;
	char buf[1024];

        /*
	 *  On Unix, conflicts with curses when interactive
         *  so let's force a dump.  - CL
	 *
	 *  On VMS, mods have been made in LYCurses.c to deal with
	 *  potential conflicts, so don't force a dump here. - FM
         */
#ifndef VMS
	dump_output_immediately = TRUE;
        LYcols = 80;
#endif /* VMS */

	post_data = &form_post_data;

	/*
	 *  Build post data for later.  Stop reading when we see a line
	 *  with "---" as its first three characters.
	 */
	while (fgets(buf, sizeof(buf), stdin) &&
	       strncmp(buf, "---", 3) != 0) {
	    int j;

	    for (j = strlen(buf) - 1; j >= 0 && /* Strip line terminators */
		(buf[j] == CR || buf[j] == LF); j--) {
		buf[j] = '\0';
	    }
	    StrAllocCat(*post_data, buf);
	}

    } else if (strncmp(argv[0], "-print", 6) == 0) {
	no_print=FALSE;

    } else if (strncmp(argv[0], "-pseudo_inlines", 15) == 0) {
	if (pseudo_inline_alts)
	    pseudo_inline_alts = FALSE;
	else
	    pseudo_inline_alts = TRUE;

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'r':
    if (strncmp(argv[0], "-raw", 4) == 0) {
        LYUseDefaultRawMode = FALSE;

    } else if (strncmp(argv[0], "-realm", 6) == 0) {
	check_realm = TRUE;

    } else if (strncmp(argv[0], "-reload", 7) == 0) {
        reloading = TRUE;

    } else if (strncmp(argv[0], "-restrictions", 13) == 0) {
	if ((cp=strchr(argv[0],'=')) != NULL)
	    parse_restrictions(cp+1);
	else
	    {
		/* print help */
		printf("\n\
   USAGE: lynx -restrictions=[option][,option][,option]\n\
   List of Options:\n\
   all             restricts all options.\n");
	        printf("\
   bookmark        disallow changing the location of the bookmark file.\n\
   bookmark_exec   disallow execution links via the bookmark file\n");
#if defined(DIRED_SUPPORT) && defined(OK_PERMIT)
	        printf("\
   change_exec_perms  disallow changing the eXecute permission on files\n\
                   (but still allow it for directories) when local file\n\
		   management is enabled.\n");
#endif /* DIRED_SUPPORT && OK_PERMIT */
	        printf("\
   default         same as commandline option -anonymous.  Disables\n\
                   default services for anonymous users.  Currently set to,\n\
                   all restricted except for: inside_telnet, outside_telnet,\n\
                   inside_news, inside_ftp, outside_ftp, inside_rlogin,\n\
		   outside_rlogin, goto, jump and mail.  Defaults\n\
                   are settable within userdefs.h\n");
#ifdef DIRED_SUPPORT
	        printf("\
   dired_support   disallow local file management\n");
#endif /* DIRED_SUPPORT */
	        printf("\
   disk_save       disallow saving binary files to disk in the download menu\n\
   dotfiles        disallow access to, or creation of, hidden (dot) files\n\
   download        disallow downloaders in the download menu\n\
   editor          disallow editing\n\
   exec            disable execution scripts\n\
   exec_frozen     disallow the user from changing the execution link\n\
   file_url        disallow using G)oto, served links or bookmarks for\n\
                   file: URL's\n\
   goto            disable the 'g' (goto) command\n");
#if defined(NO_UTMP) || defined(VMS) /* not selective */
	        printf("\
   inside_ftp      disallow ftps for people coming from inside your\n\
                   domain (utmp required for selectivity)\n\
   inside_news     disallow USENET news posting for people coming from\n\
                   inside your domain (utmp required for selectivity)\n\
   inside_rlogin   disallow rlogins for people coming from inside your\n\
                   domain (utmp required for selectivity)\n\
   inside_telnet   disallow telnets for people coming from inside your\n\
                   domain (utmp required for selectivity)\n");
#else
	        printf("\
   inside_ftp      disallow ftps for people coming from inside your domain\n\
   inside_news     disallow USENET news posting for people coming from inside\n\
                   your domain\n\
   inside_rlogin   disallow rlogins for people coming from inside your domain\n\
   inside_telnet   disallow telnets for people coming from inside your domain\n");
#endif /* NO_UTMP || VMS */
	        printf("\
   jump            disable the 'j' (jump) command\n\
   mail            disallow mail\n\
   multibook       disallow multiple bookmark files\n\
   news_post       disallow USENET News posting.\n\
   option_save     disallow saving options in .lynxrc\n");
#if defined(NO_UTMP) || defined(VMS) /* not selective */
	        printf("\
   outside_ftp     disallow ftps for people coming from outside your\n\
                   domain (utmp required for selectivity)\n\
   outside_news    disallow USENET news posting for people coming from\n\
                   outside your domain (utmp required for selectivity)\n\
   outside_rlogin  disallow rlogins for people coming from outside your\n\
                   domain (utmp required for selectivity)\n\
   outside_telnet  disallow telnets for people coming from outside your\n\
                   domain (utmp required for selectivity)\n");
#else
		printf("\
   outside_ftp     disallow ftps for people coming from outside your domain\n\
   outside_news    disallow USENET news posting for people coming from outside\n\
                   your domain\n\
   outside_rlogin  disallow rlogins for people coming from outside your domain\n\
   outside_telnet  disallow telnets for people coming from outside your domain\n");
#endif /* NO_UTMP || VMS */
		printf("\
   print           disallow most print options\n\
   shell           disallow shell escapes, and lynxexec, lynxprog or lynxcgi\n\
                   G)oto's\n\
   suspend         disallow Control-Z suspends with escape to shell\n\
   telnet_port     disallow specifying a port in telnet G)oto's\n\
   useragent       disallow modifications of the User-Agent header\n");
		exit(0);
	    }

    } else if (strncmp(argv[0], "-resubmit_posts", 14) == 0) {
	if (LYresubmit_posts)
	    LYresubmit_posts = FALSE;
	else
	    LYresubmit_posts = TRUE;

    } else if (strncmp(argv[0], "-rlogin", 7) == 0) {
	rlogin_ok = FALSE;

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 's':
    if (strncmp(argv[0], "-selective", 10) == 0) {
	HTDirAccess = HT_DIR_SELECTIVE;

    } else if (strncmp(argv[0], "-show_cursor", 12) == 0) {
	if (LYShowCursor)
	    LYShowCursor = FALSE;
	else
	    LYShowCursor = TRUE;

    } else if (strncmp(argv[0], "-soft_dquotes", 13) == 0) {
        if (soft_dquotes)
	    soft_dquotes = FALSE;
	else
	    soft_dquotes = TRUE;

    } else if (strncmp(argv[0], "-source", 7) == 0) {
	dump_output_immediately = TRUE;
	HTOutputFormat = (LYPrependBase ?
	     HTAtom_for("www/download") : HTAtom_for("www/dump"));
	LYcols=999;

    } else if (strncmp(argv[0], "-stack_dump", 11) == 0) {
	stack_dump = TRUE;

    } else if (strncmp(argv[0], "-startfile_ok", 13) == 0) {
        startfile_ok = TRUE;

#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
    } else if (strncmp(argv[0], "-syslog", 7) == 0) {
	if (nextarg) 
	    StrAllocCopy(syslog_txt, cp);
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 't':
    if (strncmp(argv[0], "-telnet", 7) == 0) {
	telnet_ok = FALSE;

    } else if (strncmp(argv[0], "-term", 5) == 0) {
	if (nextarg)
	    terminal = cp;

    } else if (strncmp(argv[0], "-trace", 6) == 0) {
	WWW_TraceFlag = TRUE;

    } else if (strncmp(argv[0], "-traversal", 10) == 0) {
	traversal = TRUE;
#ifdef USE_SLANG
	LYcols=80;
#else
	LYcols=999;
#endif /* USE_SLANG */

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'u':
    if (strncmp(argv[0], "-underscore", 15) == 0) {
	if (use_underscore)
	    use_underscore = FALSE;
	else
	    use_underscore = TRUE;

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    case 'v':
    if (strcmp(argv[0], "-validate") == 0) {
        /*
	 *  Follow only http URLs.
	 */
	LYValidate = TRUE;
	parse_restrictions("all");

    } else if (strncmp(argv[0], "-version", 8) == 0) {
	printf("\n%s Version %s\n(c)1997 GNU General Public License\n\
<URL:http://lynx.browser.org/>\n\n",
		LYNX_NAME, LYNX_VERSION);
	exit(0);

    } else if (strncmp(argv[0], "-vikeys", 7) == 0) {
	vi_keys = TRUE;

    } else {
        goto Output_Error_and_Help_List;
    }
    break;

    default:
Output_Error_and_Help_List:
#ifdef VMS
    printf(" LYNX: Invalid Option: %s\n", argv[0]);
#else
    printf("%s: Invalid Option: %s\n", pgm, argv[0]);
#endif /* VMS */
Output_Help_List:
#ifdef VMS
    printf("USAGE: lynx [options] [file]\n");
#else
    printf("USAGE: %s [options] [file]\n",pgm);
#endif /* VMS */
    printf("Options are:\n");
    printf("    -                receive the arguments from stdin (enclose\n");
    printf("                     in double-quotes (\"-\") on VMS)\n");
    printf("    -anonymous       used to specify the anonymous account\n");
    printf("    -auth=id:pw      authentication information for protected forms\n");
    printf("    -base            prepend a request URL comment and BASE tag to text/html\n");
    printf("                     outputs for -source or -mime_header dumps\n");
    printf("    -book            use the bookmark page as the startfile\n");
    printf("    -buried_news     toggles scanning of news articles for buried references\n");
    printf("    -cache=NUMBER    NUMBER of documents cached in memory (default is %d)\n",DEFAULT_CACHE_SIZE);
    printf("    -case            enable case sensitive user searching\n");
    printf("    -cfg=FILENAME    specifies a lynx.cfg file other than the default\n");
    printf("    -child           exit on left-arrow in startfile, and disable save to disk\n");
#ifdef USE_SLANG
    printf("    -color           force color mode on with standard bg colors\n");
    printf("    -blink           force color mode on with high intensity bg colors\n");
#endif /* USE_SLANG */
    printf("    -cookies         toggles handling of Set-Cookie headers\n");
    printf("    -crawl           with -traversal, output each page to a file\n");
    printf("                     with -dump, format output as with -traversal, but to stdout\n");
    printf("    -display=DISPLAY set the display variable for X execed programs\n");
    printf("    -dump            dump the first file to stdout and exit\n");
    printf("    -editor=EDITOR   enable edit mode with specified editor\n");
    printf("    -emacskeys       enable emacs-like key movement\n");
    printf("    -enable_scrollback  toggles compatibility with comm programs' scrollback\n");
    printf("                        keys (may be incompatible with some curses packages)\n");
     printf("    -error_file=FILE write the HTTP status code here\n");
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
#ifndef NEVER_ALLOW_REMOTE_EXEC
    printf("    -exec            enable local program execution\n");
#endif /* !NEVER_ALLOW_REMOTE_EXEC */
    printf("    -locexec         enable local program execution from local files only\n");
    printf("    -noexec          disable local program execution (DEFAULT)\n");
#endif /* EXEC_LINKS || EXEC_SCRIPTS */
    printf("    -fileversions    include all versions of files in local VMS directory\n");
    printf("                     listings\n");
    printf("    -force_html      forces the first document to be interpreted as HTML\n");
    printf("    -ftp             disable ftp access\n");
    printf("    -get_data        user data for get forms, read from stdin,\n");
    printf("                     terminated by '---' on a line\n");
    printf("    -head            send a HEAD request\n");
    printf("    -help            print this usage message\n");
    printf("    -historical      toggles use of '>' or '-->' as a terminator for comments\n");
    printf("    -homepage=URL    set homepage separate from start page\n");
    printf("    -image_links     toggles inclusion of links for all images\n");
    printf("    -index=URL       set the default index file to URL\n");
    printf("    -link=NUMBER     starting count for lnk#.dat files produced by -crawl\n");
    printf("    -localhost       disable URLs that point to remote hosts\n");
    printf("    -mime_header     include mime headers and force source dump\n");
    printf("    -minimal         toggles minimal versus valid comment parsing\n");
    printf("    -newschunksize=NUMBER  number of articles in chunked news listings\n");
    printf("    -newsmaxchunk=NUMBER   maximum news articles in listings before chunking\n");
    printf("    -nobrowse        disable directory browsing\n");
    printf("    -nofilereferer   disable transmissions of Referer headers for file URLs\n");
    printf("    -nofrom          disable transmissions of From headers\n");
    printf("    -nolist          disable the link list feature in dumps\n");
    printf("    -nolog           disable mailing of error messages to document owners\n");
    printf("    -nopause         disable forced pauses for statusline messages\n");
    printf("    -noprint         disable print functions\n");
    printf("    -noredir         don't follow Location: redirection\n");
    printf("    -noreferer       disable transmissions of Referer headers\n");
#ifdef SOCKS
    printf("    -nosocks         don't use SOCKS proxy for this session\n");
#endif /* SOCKS */
    printf("    -nostatus        disable the miscellaneous information messages\n");
    printf("    -number_links    force numbering of links\n");
    printf("    -popup           toggles handling of single-choice SELECT options via\n");
    printf("                     popup windows or as lists of radio buttons\n");
    printf("    -post_data       user data for post forms, read from stdin,\n");
    printf("                     terminated by '---' on a line\n");
    printf("    -print           enable print functions (DEFAULT)\n");
    printf("    -pseudo_inlines  toggles pseudo-ALTs for inlines with no ALT string\n");
    printf("    -raw             toggles default setting of 8-bit character translations\n");
    printf("                     or CJK mode for the startup character set\n");
    printf("    -realm           restricts access to URLs in the starting realm\n");
    printf("    -reload          flushes the cache on a proxy server\n");
    printf("                     (only the first document affected)\n");
    printf("    -restrictions=[options]  use -restrictions to see list\n");
    printf("    -resubmit_posts  toggles forced resubmissions (no-cache) of forms with\n");
    printf("                     method POST when the documents they returned are sought\n");
    printf("                     with the PREV_DOC command or from the History List\n");
    printf("    -rlogin          disable rlogins\n");
    printf("    -selective       require .www_browsable files to browse directories\n");
    printf("    -show_cursor     toggles hiding of the curser in the lower right corner\n");
    printf("    -soft_dquotes    toggles emulation of the old Netscape and Mosaic bug which\n");
    printf("                     treated '>' as a co-terminator for double-quotes and tags\n");
    printf("    -source          dump the source of the first file to stdout and exit\n");
    printf("    -startfile_ok    allow non-http startfile and homepage with -validate\n");
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
    printf("    -syslog=text     information for syslog call\n");
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */
    printf("    -telnet          disable telnets\n");
    printf("    -term=TERM       set terminal type to TERM\n");
    printf("    -trace           turns on WWW trace mode\n");
    printf("    -traversal       traverse all http links derived from startfile\n");
    printf("    -underscore      toggles use of _underline_ format in dumps\n");
    printf("    -validate        accept only http URLs (for validation)\n");
    printf("    -version         print Lynx version information\n");
    printf("    -vikeys          enable vi-like key movement\n");
    if (strncmp(argv[0], "-help", 5) != 0)
	exit(-1);
    exit(0);
    break;

    } /* end of switch. */
}

#ifndef VMS
PRIVATE void FatalProblem ARGS1(
	int,		sig)
{
    /*
     *  Ignore further interrupts. - mhc: 11/2/91
     */
    (void) signal (SIGHUP, SIG_IGN);
    (void) signal (SIGTERM, SIG_IGN);
    (void) signal (SIGINT, SIG_IGN);
#ifndef __linux__
    (void) signal(SIGBUS, SIG_IGN);
#endif /* !__linux__ */
    (void) signal(SIGSEGV, SIG_IGN);
    (void) signal(SIGILL, SIG_IGN);

    /*
     *  Flush all messages. - FM
     */
    fflush(stderr);
    fflush(stdout);

    /*
     *  Deal with curses, if on, and clean up. - FM
     */
    if (LYOutOfMemory && LYCursesON) {
	sleep(AlertSecs);
    }
    cleanup_sig(0);
#ifndef __linux__
    signal(SIGBUS, SIG_DFL);
#endif /* !__linux__ */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);

    /*
     *  Issue appropriate messages and abort or exit. - FM
     */
    if (LYOutOfMemory == FALSE) {
        fprintf (stderr, "\r\n\
A Fatal error has occurred in %s Ver. %s\r\n", LYNX_NAME, LYNX_VERSION);

        fprintf(stderr, "\r\n\
Please notify your system administrator to confirm a bug, and\r\n\
if confirmed, to notify the lynx-dev list.  Bug reports should\r\n\
have concise descriptions of the command and/or URL which causes\r\n\
the problem, the operating system name with version number, the\r\n\
TCPIP implementation, and any other relevant information.\r\n");

        fprintf(stderr, "\r\n\
Do NOT mail the core file if one was generated.\r\n");

        fprintf(stderr, "\r\n\
Lynx now exiting with signal:  %d\r\n\r\n", sig);

	/*
	 *  Exit and dump core.
	 */
	abort();

    } else {
	LYOutOfMemory = FALSE;
	printf("\r\n%s\r\n\r\n", MEMORY_EXHAUSTED_ABORT);
	fflush(stdout);

	/*
	 *  Exit without dumping core.
	 */
	exit(0);
    }
}
#endif /* !VMS */
