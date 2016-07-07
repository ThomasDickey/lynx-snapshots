#include <HTUtils.h>
#include <tcp.h>
#include <HTTP.h>
#include <HTParse.h>
#include <HTAccess.h>
#include <HTList.h>
#include <HTFile.h>
#include <UCMap.h>
#include <UCDefs.h>
#ifdef VMS
#include <HTVMSUtils.h>
#endif /* VMS */
#include <HTInit.h>
#include <LYCurses.h>
#include <LYStyle.h>
#include <HTML.h>
#include <LYUtils.h>
#include <LYGlobalDefs.h>
#include <LYSignal.h>
#include <LYGetFile.h>
#include <LYStrings.h>
#include <LYClean.h>
#include <LYCharSets.h>
#include <LYCharUtils.h>
#include <UCMap.h>
#include <LYReadCFG.h>
#include <LYrcFile.h>
#include <LYKeymap.h>
#include <LYList.h>
#include <LYJump.h>
#include <LYMainLoop.h>
#include <LYBookmark.h>
#ifdef DOSPATH
#include <HTDOS.h>
#endif

#ifdef __DJGPP__
#include <dos.h>
#endif /* __DJGPP__ */

#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
#include <syslog.h>
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

#ifdef LOCALE
#include <locale.h>
#endif /* LOCALE */

#include <LYexit.h>
#include <LYLeaks.h>

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

#ifdef __DJGPP__
char init_ctrl_break[1];
#endif /* __DJGPP__ */

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
PUBLIC char *syslog_txt = NULL; 	/* syslog arb text for session */
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

#if defined(LYNXCGI_LINKS) && !defined(VMS)  /* WebSter Mods -jkt */
PUBLIC char *LYCgiDocumentRoot = NULL; /* DOCUMENT_ROOT in the lynxcgi env */
#endif /* LYNXCGI_LINKS */

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
#ifdef USE_EXTERNALS
PUBLIC lynx_html_item_type *externals = NULL;
			    /* linked list of external options */
#endif
PUBLIC lynx_html_item_type *uploaders = NULL;
PUBLIC int port_syntax = 1;
PUBLIC int LYShowColor = SHOW_COLOR_UNKNOWN; /* to show or not to show */
PUBLIC int LYChosenShowColor = SHOW_COLOR_UNKNOWN; /* whether to show and save */
PUBLIC int LYrcShowColor = SHOW_COLOR_UNKNOWN;	/* ... as last read or written */
PUBLIC BOOLEAN LYShowCursor = SHOW_CURSOR; /* to show or not to show */
PUBLIC BOOLEAN verbose_img = VERBOSE_IMAGES;  /* show filenames or not */
PUBLIC BOOLEAN LYUseDefShoCur = TRUE;	/* Command line -show_cursor toggle */
PUBLIC BOOLEAN LYforce_no_cache = FALSE;
PUBLIC BOOLEAN LYoverride_no_cache = FALSE;/*override no-cache b/c history etc*/
PUBLIC BOOLEAN LYinternal_flag = FALSE; /* override no-cache b/c internal link*/
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
#ifdef USE_EXTERNALS
PUBLIC BOOLEAN no_externals = FALSE;
#endif
#ifdef RAWDOSKEYHACK
PUBLIC BOOLEAN raw_dos_key_hack = TRUE;
#endif /* RAWDOSKEYHACK */
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
PUBLIC char *jumpprompt = NULL; /* the default jumps prompt */
PUBLIC char *bookmark_page = NULL; /* the name of the default bookmark page */
PUBLIC char *BookmarkPage = NULL;  /* the name of the current bookmark page */
PUBLIC char *LynxHome = NULL;	/* the default Home HREF. */
PUBLIC char *startfile = NULL;	/* the first file */
PUBLIC char *helpfile = NULL;	/* the main help file */
PUBLIC char *helpfilepath = NULL;   /* the path to the help file set */
PUBLIC char *lynxjumpfile = NULL;   /* the current jump file URL */
PUBLIC char *lynxlistfile = NULL;   /* the current list file URL */
PUBLIC char *lynxlinksfile = NULL;  /* the current visited links file URL */
PUBLIC char *startrealm = NULL;     /* the startfile realm */
PUBLIC char *indexfile = NULL;	    /* an index file if there is one */
PUBLIC char *personal_mail_address = NULL; /* the users mail address */
PUBLIC char *display = NULL;	    /* display environment variable */
PUBLIC char *personal_type_map = NULL;	   /* .mailcap */
PUBLIC char *global_type_map = NULL;	   /* global mailcap */
PUBLIC char *global_extension_map = NULL;  /* global mime.types */
PUBLIC char *personal_extension_map = NULL;/* .mime.types */
PUBLIC char *language = NULL;	    /* preferred language */
PUBLIC char *pref_charset = NULL;   /* preferred character set */
PUBLIC BOOLEAN LYNewsPosting = NEWS_POSTING; /* News posting supported? */
PUBLIC char *LynxSigFile = NULL;    /* Signature file, in or off home */
PUBLIC char *system_mail = NULL;	  /* The path for sending mail */
PUBLIC char *system_mail_flags = NULL;	  /* Flags for sending mail */
PUBLIC char *lynx_temp_space = NULL; /* The prefix for temporary file paths */
PUBLIC char *lynx_save_space = NULL; /* The prefix for save to disk paths */
PUBLIC char *LYHostName = NULL; 	/* treat as a local host name */
PUBLIC char *LYLocalDomain = NULL;	/* treat as a local domain tail */
PUBLIC BOOLEAN clickable_images = MAKE_LINKS_FOR_ALL_IMAGES;
PUBLIC BOOLEAN pseudo_inline_alts = MAKE_PSEUDO_ALTS_FOR_INLINES;
PUBLIC BOOLEAN crawl = FALSE;	     /* Do crawl? */
PUBLIC BOOLEAN traversal = FALSE;    /* Do traversals? */
PUBLIC BOOLEAN check_realm = FALSE;  /* Restrict to the starting realm? */
	       /* Links beyond a displayed page with no links? */
PUBLIC BOOLEAN more_links = FALSE;
PUBLIC int     ccount = 0; /* Starting number for lnk#.dat files in crawls */
PUBLIC BOOLEAN LYCancelledFetch = FALSE; /* TRUE if cancelled binary fetch */
	       /* Include mime headers with source dump */
PUBLIC BOOLEAN keep_mime_headers = FALSE;
PUBLIC BOOLEAN no_url_redirection = FALSE; /* Don't follow URL redirections */
PUBLIC BOOLEAN display_partial = FALSE; /* Display document during download */
PUBLIC char *form_post_data = NULL;  /* User data for post form */
PUBLIC char *form_get_data = NULL;   /* User data for get form */
PUBLIC char *http_error_file = NULL; /* Place HTTP status code in this file */
	     /* Id:Password for protected documents */
PUBLIC char *authentication_info[2] = {NULL, NULL};
	     /* Id:Password for protected proxy servers */
PUBLIC char *proxyauth_info[2] = {NULL, NULL};
PUBLIC BOOLEAN HEAD_request = FALSE;
PUBLIC BOOLEAN scan_for_buried_news_references = TRUE;
PUBLIC BOOLEAN LYRawMode;
PUBLIC BOOLEAN LYDefaultRawMode;
PUBLIC BOOLEAN LYUseDefaultRawMode = TRUE;
PUBLIC char *UCAssume_MIMEcharset = NULL;
PUBLIC char *UCAssume_localMIMEcharset = NULL;
PUBLIC char *UCAssume_unrecMIMEcharset = NULL;
PUBLIC int LYlines = 24;
PUBLIC int LYcols = 80;
PUBLIC int dump_output_width = 0;
PUBLIC linkstruct links[MAXLINKS];
PUBLIC histstruct history[MAXHIST];
PUBLIC int nlinks = 0;		/* number of links in memory */
PUBLIC int nhist = 0;		/* number of history entries */
PUBLIC int more = FALSE;	/* is there more text to display? */
PUBLIC int InfoSecs;	/* Seconds to sleep() for Information messages */
PUBLIC int MessageSecs; /* Seconds to sleep() for important Messages   */
PUBLIC int AlertSecs;	/* Seconds to sleep() for HTAlert() messages   */
PUBLIC BOOLEAN bookmark_start = FALSE;
PUBLIC char *LYUserAgent = NULL;	/* Lynx User-Agent header	   */
PUBLIC char *LYUserAgentDefault = NULL; /* Lynx default User-Agent header  */
PUBLIC BOOLEAN LYUseMouse = FALSE;
PUBLIC BOOLEAN LYNoRefererHeader=FALSE; /* Never send Referer header?	   */
PUBLIC BOOLEAN LYNoRefererForThis=FALSE;/* No Referer header for this URL? */
PUBLIC BOOLEAN LYNoFromHeader = TRUE;	/* Never send From header?	   */
PUBLIC BOOLEAN LYListNewsNumbers = LIST_NEWS_NUMBERS;
PUBLIC BOOLEAN LYListNewsDates = LIST_NEWS_DATES;
PUBLIC BOOLEAN LYisConfiguredForX = FALSE;
PUBLIC char *URLDomainPrefixes = NULL;
PUBLIC char *URLDomainSuffixes = NULL;
PUBLIC BOOLEAN startfile_ok = FALSE;
PUBLIC BOOLEAN LYSelectPopups = USE_SELECT_POPUPS;
PUBLIC BOOLEAN LYUseDefSelPop = TRUE;	/* Command line -popup toggle */
PUBLIC BOOLEAN LYMultiBookmarks = MULTI_BOOKMARK_SUPPORT;
PUBLIC BOOLEAN LYMBMBlocked = BLOCK_MULTI_BOOKMARKS;
PUBLIC BOOLEAN LYMBMAdvanced = TRUE;
PUBLIC int LYStatusLine = -1;		 /* Line for statusline() if > -1 */
PUBLIC BOOLEAN LYCollapseBRs = COLLAPSE_BR_TAGS;  /* Collapse serial BRs? */
PUBLIC BOOLEAN LYSetCookies = SET_COOKIES; /* Process Set-Cookie headers? */
PUBLIC char *XLoadImageCommand = NULL;	/* Default image viewer for X */
PUBLIC BOOLEAN LYNoISMAPifUSEMAP = FALSE; /* Omit ISMAP link if MAP present? */
PUBLIC int LYHiddenLinks = HIDDENLINKS_SEPARATE; /* Show hidden links? */

PUBLIC BOOL New_DTD = YES;
PUBLIC FILE *LYTraceLogFP = NULL;		/* Pointer for TRACE log  */
PUBLIC char *LYTraceLogPath = NULL;		/* Path for TRACE log	   */
PUBLIC BOOLEAN LYUseTraceLog = USE_TRACE_LOG;	/* Use a TRACE log?	   */
PUBLIC BOOLEAN LYSeekFragMAPinCur = TRUE;
PUBLIC BOOLEAN LYSeekFragAREAinCur = TRUE;

PUBLIC BOOLEAN LYStripDotDotURLs = TRUE;	/* Try to fix ../ in some URLs? */
PUBLIC BOOLEAN LYForceSSLCookiesSecure = FALSE;
PUBLIC BOOLEAN LYNoCc = FALSE;
PUBLIC BOOLEAN LYPreparsedSource = FALSE;	/* Show source as preparsed?	 */
PUBLIC BOOLEAN LYPrependBaseToSource = TRUE;
PUBLIC BOOLEAN LYPrependCharsetToSource = FALSE;
PUBLIC BOOLEAN LYQuitDefaultYes = QUIT_DEFAULT_YES;

/* These are declared in cutil.h for current freeWAIS libraries. - FM */
#ifdef DECLARE_WAIS_LOGFILES
PUBLIC char *log_file_name = NULL; /* for WAIS log file name	in libWWW */
PUBLIC FILE *logfile = NULL;	   /* for WAIS log file output	in libWWW */
#endif /* DECLARE_WAIS_LOGFILES */

extern int HTNewsChunkSize; /* Number of news articles per chunk (HTNews.c) */
extern int HTNewsMaxChunk;  /* Max news articles before chunking (HTNews.c) */

PRIVATE BOOLEAN anon_restrictions_set = FALSE;
PRIVATE BOOLEAN stack_dump = FALSE;
PRIVATE char *terminal = NULL;
PRIVATE char *pgm;
PRIVATE BOOLEAN number_links = FALSE;
PRIVATE BOOLEAN LYPrependBase = FALSE;
PRIVATE HTList *LYStdinArgs = NULL;

PRIVATE void parse_arg PARAMS((char **arg, int *i));
PRIVATE void print_help_and_exit PARAMS((int exit_status));

#ifndef VMS
PUBLIC BOOLEAN LYNoCore = NO_FORCED_CORE_DUMP;
PRIVATE void FatalProblem PARAMS((int sig));
#endif /* !VMS */

#if defined(USE_HASH)
    char *lynx_lss_file=NULL;
#endif

#ifdef __DJGPP__
PRIVATE int LY_set_ctrl_break(int setting)
{
    (void)signal(SIGINT, (setting ? SIG_DFL : SIG_IGN));
    setcbrk(setting);
}

PRIVATE int LY_get_ctrl_break(void)
{
    typedef union REGS Regs;
    extern Regs regs;
    regs.h.ah = 0x33;
    regs.h.al = 0x00;
    int86(0x21, &regs, &regs);
    return ((int) regs.h.dl);
}

PRIVATE void reset_break(void)
{
    LY_set_ctrl_break(init_ctrl_break[0]);
}
#endif /* __DJGPP__ */

PRIVATE void free_lynx_globals NOARGS
{
    int i;

#ifndef VMS
    FREE(list_format);
#ifdef SYSLOG_REQUESTED_URLS
    FREE(syslog_txt);
#endif /* SYSLOG_REQUESTED_URLS */
#ifdef LYNXCGI_LINKS  /* WebSter Mods -jkt */
    FREE(LYCgiDocumentRoot);
#endif /* LYNXCGI_LINKS */
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
    FREE(display);
    FREE(global_type_map);
    FREE(personal_type_map);
    FREE(global_extension_map);
    FREE(personal_extension_map);
    FREE(language);
    FREE(pref_charset);
    FREE(LynxSigFile);
    FREE(system_mail);
    FREE(system_mail_flags);
    FREE(LYUserAgent);
    FREE(LYUserAgentDefault);
    FREE(LYHostName);
    FREE(LYLocalDomain);
    FREE(lynx_save_space);
    FREE(homepage);
    FREE(helpfilepath);
    FREE(bookmark_page);
    FREE(BookmarkPage);
    for (i = 0; i <= MBM_V_MAXFILES; i++) {
	FREE(MBM_A_subbookmark[i]);
	FREE(MBM_A_subdescript[i]);
    }
    FREE(editor);
    FREE(authentication_info[0]);
    FREE(authentication_info[1]);
    FREE(proxyauth_info[0]);
    FREE(proxyauth_info[1]);
    FREE(lynxjumpfile);
    FREE(startrealm);
    FREE(personal_mail_address);
    FREE(URLDomainPrefixes);
    FREE(URLDomainSuffixes);
    FREE(XLoadImageCommand);
    FREE(LYTraceLogPath);
#if defined(USE_HASH)
    FREE(lynx_lss_file);
#endif
    FREE(UCAssume_MIMEcharset);
    FREE(UCAssume_unrecMIMEcharset);
    FREE(UCAssume_localMIMEcharset);
    for (i = 0; i < nlinks; i++) {
	FREE(links[i].lname);
    }
    nlinks = 0;

    return;
}

/*
 *  This function frees the LYStdinArgs list. - FM
 */
PRIVATE void LYStdinArgs_free NOARGS
{
    char *argument;
    HTList *cur = LYStdinArgs;

    if (cur == NULL)
	return;

    while (NULL != (argument = (char *)HTList_nextObject(cur))) {
	FREE(argument);
    }
    HTList_delete(LYStdinArgs);
    LYStdinArgs = NULL;
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
    BOOL LYGetStdinArgs = FALSE;

#ifdef _WINDOWS
    WSADATA WSAData;
    {
	int err;
	WORD wVerReq;

	wVerReq = MAKEWORD(1,1);

	err = WSAStartup(wVerReq, &WSAData);
	if (err != 0)
	{
	    printf("No Winsock found, sorry.");
	    sleep(5);
	    return;
	}
    }
#endif /* _WINDOWS */

#ifdef __DJGPP__
    if (LY_get_ctrl_break() == 0) {
	LY_set_ctrl_break(TRUE);
	init_ctrl_break[0] = 0;
    } else {
	init_ctrl_break[0] = 1;
    }
    atexit(reset_break);
    sock_init();
#endif

#if defined(_WINDOWS) || defined(DJGPP)
    /*
     * To prevent corrupting binary data with _WINDOWS and DJGPP
     * we open files and stdout in BINARY mode by default.
     * Where necessary we should open and (close!) TEXT mode.
     */
    _fmode = O_BINARY;
    setmode( fileno( stdout ), O_BINARY );
#endif

#ifdef DOSPATH
    if (getenv("TERM")==NULL) putenv("TERM=vt100");
#endif

    LYShowColor = (SHOW_COLOR ? SHOW_COLOR_ON : SHOW_COLOR_OFF);
    /*
     *	Set up the argument list.
     */
    pgm = argv[0];
    if ((cp = strrchr(pgm, '/')) != NULL) {
	pgm = cp + 1;
    }

    /*
     *	Act on -help NOW, so we only output the help and exit. - FM
     */
    for (i = 1; i < argc; i++) {
	if (strncmp(argv[i], "-help", 5) == 0) {
	    parse_arg(&argv[i], &i);
	}
    }

#ifdef LY_FIND_LEAKS
    /*
     *	Register the final function to be executed when being exited.
     *	Will display memory leaks if LY_FIND_LEAKS is defined.
     */
    atexit(LYLeaks);
#endif /* LY_FIND_LEAKS */
    /*
     *	Register the function which will free our allocated globals.
     */
    atexit(free_lynx_globals);

#ifdef LOCALE
    /*
     *	LOCALE support for international characters.
     */
    setlocale(LC_ALL, "");
#endif /* LOCALE */

    /*
     *	Initialize our startup and global variables.
     */
#ifdef ULTRIX
    /*
     *	Need this for Ultrix.
     */
    terminal = getenv("TERM");
    if ((terminal == NULL) || !strncasecomp(terminal, "xterm", 5))
	terminal = "vt100";
#endif /* ULTRIX */
    /*
     *	Zero the links and history struct arrays.
     */
    memset((void *)links, 0, sizeof(linkstruct)*MAXLINKS);
    memset((void *)history, 0, sizeof(histstruct)*MAXHIST);
    /*
     *	Zero the MultiBookmark arrays.
     */
    memset((void *)MBM_A_subbookmark, 0, sizeof(char)*(MBM_V_MAXFILES+1));
    memset((void *)MBM_A_subdescript, 0, sizeof(char)*(MBM_V_MAXFILES+1));
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
    openlog("lynx", LOG_PID, LOG_LOCAL5);
#endif /* SYSLOG_REQUESTED_URLS */
    StrAllocCopy(list_format, LIST_FORMAT);
#endif /* !VMS */
    InfoSecs	= (int)INFOSECS;
    MessageSecs = (int)MESSAGESECS;
    AlertSecs	= (int)ALERTSECS;
    StrAllocCopy(helpfile, HELPFILE);
    StrAllocCopy(startfile, STARTFILE);
    LYTrimHead(startfile);
    if (!strncasecomp(startfile, "lynxexec:", 9) ||
	!strncasecomp(startfile, "lynxprog:", 9)) {
	/*
	 *  The original implementations of these schemes expected
	 *  white space without hex escaping, and did not check
	 *  for hex escaping, so we'll continue to support that,
	 *  until that code is redone in conformance with SGML
	 *  principles.  - FM
	 */
	HTUnEscapeSome(startfile, " \r\n\t");
	convert_to_spaces(startfile, TRUE);
    }
    StrAllocCopy(indexfile, DEFAULT_INDEX_FILE);
    StrAllocCopy(global_type_map, GLOBAL_MAILCAP);
    StrAllocCopy(personal_type_map, PERSONAL_MAILCAP);
    StrAllocCopy(global_extension_map, GLOBAL_EXTENSION_MAP);
    StrAllocCopy(personal_extension_map, PERSONAL_EXTENSION_MAP);
    StrAllocCopy(language, PREFERRED_LANGUAGE);
    StrAllocCopy(pref_charset, PREFERRED_CHARSET);
    StrAllocCopy(system_mail, SYSTEM_MAIL);
    StrAllocCopy(system_mail_flags, SYSTEM_MAIL_FLAGS);
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
    if ((cp = getenv("LYNX_TEMP_SPACE")) != NULL)
	StrAllocCopy(lynx_temp_space, cp);
    else
#if defined (DOSPATH) || defined (__EMX__)
    if ((cp = getenv("TEMP")) != NULL)
	StrAllocCopy(lynx_temp_space, cp);
    else if ((cp = getenv("TMP")) != NULL)
	StrAllocCopy(lynx_temp_space, cp);
    else
    {
	printf("You MUST define a valid TMP or TEMP area!\n");
	exit(-1);
    }
#else
	StrAllocCopy(lynx_temp_space, TEMP_SPACE);
#endif
    if ((cp = strchr(lynx_temp_space, '~'))) {
	*(cp++) = '\0';
	StrAllocCopy(temp, lynx_temp_space);
	if (((len = strlen(temp)) > 0) && temp[len-1] == '/')
	    temp[len-1] = '\0';
#ifdef DOSPATH
	StrAllocCat(temp, HTDOS_wwwName((char *)Home_Dir()));
#else
#ifdef VMS
	StrAllocCat(temp, HTVMS_wwwName((char *)Home_Dir()));
#else
	StrAllocCat(temp, Home_Dir());
#endif /* VMS */
#endif /* DOSPATH */
	StrAllocCat(temp, cp);
	StrAllocCopy(lynx_temp_space, temp);
	FREE(temp);
    }
    if ((cp = strstr(lynx_temp_space, "$USER")) != NULL) {
	char *cp1;

	if ((cp1 = (char *)getenv("USER")) != NULL) {
	    *cp = '\0';
	    StrAllocCopy(temp, lynx_temp_space);
	    *cp = '$';
	    StrAllocCat(temp, cp1);
	    cp += 5;
	    StrAllocCat(temp, cp);
	    StrAllocCopy(lynx_temp_space, temp);
	    FREE(temp);
	}
    }
#ifdef VMS
    LYLowerCase(lynx_temp_space);
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
#if !defined(__DJGPP__) && !defined(_WINDOWS)
    if (((len = strlen(lynx_temp_space)) > 1) &&
	lynx_temp_space[len-1] != '/') {
	StrAllocCat(lynx_temp_space, "/");
    }
#else
    if (((len = strlen(lynx_temp_space)) > 1) &&
	lynx_temp_space[len-1] != '\\') {
	StrAllocCat(lynx_temp_space, "\\");
    }
#endif /* !__DJGPP__ && !_WINDOWS */
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
     *	Set up the compilation default character set. - FM
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
     *	Disable news posting if the compilation-based
     *	LYNewsPosting value is FALSE.  This may be changed
     *	further down via lynx.cfg or the -restriction
     *	command line switch. - FM
     */
    no_newspost = (LYNewsPosting == FALSE);

    /*
     *	Set up trace, the anonymous account defaults,
     *	validate restrictions, and/or the nosocks flag,
     *	if requested, and an alternate configuration
     *	file, if specified, NOW.  Also, if we only want
     *	the help menu, output that and exit. - FM
     */
    for (i = 1; i < argc; i++) {
	if (strncmp(argv[i], "-trace", 6) == 0) {
	    WWW_TraceFlag = TRUE;
	} else if (strncmp(argv[i], "-tlog", 5) == 0) {
	    if (LYUseTraceLog) {
		LYUseTraceLog = FALSE;
	    } else {
		LYUseTraceLog = TRUE;
	    }
	} else if (strncmp(argv[i], "-anonymous", 10) == 0) {
	    if (!LYValidate)
		parse_restrictions("default");
	    anon_restrictions_set = TRUE;
	} else if (strcmp(argv[i], "-validate") == 0) {
	    /*
	     *	Follow only http URLs.
	     */
	    LYValidate = TRUE;
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

#if defined(USE_HASH)
	} else if (strncmp(argv[i], "-lss", 4) == 0) {
	    if ((cp=strchr(argv[i],'=')) != NULL)
		StrAllocCopy(lynx_lss_file, cp+1);
	    else {
		StrAllocCopy(lynx_lss_file, argv[i+1]);
		i++;
	    }
	    CTRACE(tfp, "LYMain found -lss flag, lss file is %s\n",
		    lynx_lss_file ? lynx_lss_file : "<NONE>");
#endif
	}
    }

    /*
     *	If we have a lone "-" switch for getting arguments from stdin,
     *	get them NOW, and act on the relevant ones, saving the others
     *	into an HTList for handling after the other initializations.
     *	The primary purpose of this feature is to allow for the
     *	potentially very long command line that can be associated with
     *	post or get data.  The original implementation required that
     *	the lone "-" be the only command line argument, but that
     *	precluded its use when the lynx command is aliased with other
     *	arguments.  When interactive, the stdin input is terminated by
     *	by Control-D on Unix or Control-Z on VMS, and each argument
     *	is terminated by a RETURN.  When the argument is -get_data or
     *	-post_data, the data are terminate by a "___" string, alone
     *	on the line (also terminated by RETURN). - FM
     */
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-") == 0) {
	    LYGetStdinArgs = TRUE;
	}
    }
    if (LYGetStdinArgs == TRUE) {
	char buf[1025];

	while (fgets(buf, sizeof(buf) - 1, stdin)) {
	    int j;

	    for (j = strlen(buf) - 1; j > 0 &&
		(buf[j] == CR || buf[j] == LF); j--) {
		buf[j] = '\0';
	    }

	    if (strncmp(buf, "-trace", 6) == 0) {
		WWW_TraceFlag = TRUE;
	    } else if (strncmp(buf, "-tlog", 5) == 0) {
		if (LYUseTraceLog) {
		    LYUseTraceLog = FALSE;
		} else {
		    LYUseTraceLog = TRUE;
		}
	    } else if (strncmp(buf, "-anonymous", 10) == 0) {
		if (!LYValidate && !anon_restrictions_set)
		    parse_restrictions("default");
		anon_restrictions_set = TRUE;
	    } else if (strcmp(buf, "-validate") == 0) {
		/*
		 *  Follow only http URLs.
		 */
		LYValidate = TRUE;
#ifdef SOCKS
	    } else if (strncmp(buf, "-nosocks", 8) == 0) {
		socks_flag = FALSE;
#endif /* SOCKS */
	    } else if (strncmp(buf, "-cfg", 4) == 0) {
		if ((cp = strchr(buf,'=')) != NULL) {
		    StrAllocCopy(lynx_cfg_file, cp+1);
		} else {
		    cp = LYSkipNonBlanks(buf);
		    cp = LYSkipBlanks(cp);
		    if (*cp)
			StrAllocCopy(lynx_cfg_file, cp);
		}
#if defined(USE_HASH)
	    } else if (strncmp(buf, "-lss", 4) == 0) {
		if ((cp = strchr(buf,'=')) != NULL) {
		    StrAllocCopy(lynx_lss_file, cp+1);
		} else {
		    cp = LYSkipNonBlanks(buf);
		    cp = LYSkipBlanks(cp);
		    if (*cp)
			StrAllocCopy(lynx_cfg_file, cp);
		}
		CTRACE(tfp, "LYMain found -lss flag, lss file is %s\n",
			lynx_lss_file ? lynx_lss_file : "<NONE>");
#endif
	    } else if (strcmp(buf, "-get_data") == 0) {
		/*
		 *  User data for GET form.
		 */
		char **get_data;

		/*
		 *  On Unix, conflicts with curses when interactive
		 *  so let's force a dump.  - CL
		 *
		 *  On VMS, mods have been made in LYCurses.c to deal
		 *  with potential conflicts, so don't force the dump
		 *  here. - FM
		 */
#ifndef VMS
		dump_output_immediately = TRUE;
		LYcols = 80;
#endif /* VMS */

		StrAllocCopy(form_get_data, "?");   /* Prime the pump */
		get_data = &form_get_data;

		/*
		 *  Build GET data for later.  Stop reading when we see
		 *  a line with "---" as its first three characters.
		 */
		while (fgets(buf, sizeof(buf), stdin) &&
		       strncmp(buf, "---", 3) != 0) {
		    int j2;

		    /*
		     *	Strip line terminators.
		     */
		    for (j2 = strlen(buf) - 1; j2 >= 0 &&
			 (buf[j2] == CR || buf[j2] == LF); j2--) {
			buf[j2] = '\0';
		    }
		    StrAllocCat(*get_data, buf);
		}
	    } else if (strcmp(buf, "-post_data") == 0) {
		/*
		 *  User data for POST form.
		 */
		char **post_data;

		/*
		 *  On Unix, conflicts with curses when interactive
		 *  so let's force a dump.  - CL
		 *
		 *  On VMS, mods have been made in LYCurses.c to deal
		 *  with potential conflicts, so don't force a dump
		 *  here. - FM
		 */
#ifndef VMS
		dump_output_immediately = TRUE;
		LYcols = 80;
#endif /* VMS */

		post_data = &form_post_data;

		/*
		 *  Build post data for later.	Stop reading when we see
		 *  a line with "---" as its first three characters.
		 */
		while (fgets(buf, sizeof(buf), stdin) &&
		       strncmp(buf, "---", 3) != 0) {
		    int j2;

		     /*
		      *  Strip line terminators.
		      */
		    for (j2 = strlen(buf) - 1; j2 >= 0 &&
			 (buf[j2] == CR || buf[j2] == LF); j2--) {
			buf[j2] = '\0';
		    }
		    StrAllocCat(*post_data, buf);
		}
	    } else if (buf[0] != '\0') {
		char *argument = NULL;

		if (LYStdinArgs == NULL) {
		    LYStdinArgs = HTList_new();
		    atexit(LYStdinArgs_free);
		}
		StrAllocCopy(argument, buf);
		HTList_appendObject(LYStdinArgs, argument);
	    }
	}
    }

#ifdef SOCKS
    if (socks_flag)
	SOCKSinit(argv[0]);
#endif /* SOCKS */

    /*
     *	If we had -validate set all of the restrictions
     *	and disallow a TRACE log NOW. - FM
     */
    if (LYValidate == TRUE) {
	parse_restrictions("all");
	LYUseTraceLog = FALSE;
    }

    /*
     *	If we didn't get and act on a -validate or -anonymous
     *	switch, but can verify that this is the anonymous account,
     *	set the default restrictions for that account and disallow
     *	a TRACE log NOW. - FM
     */
    if (!LYValidate && !anon_restrictions_set &&
	strlen((char *)ANONYMOUS_USER) > 0 &&
#if defined (VMS) || defined (NOUSERS)
	!strcasecomp(((char *)getenv("USER")==NULL ? " " : getenv("USER")),
		     ANONYMOUS_USER))
#else
#if HAVE_CUSERID
	STREQ((char *)cuserid((char *) NULL), ANONYMOUS_USER))
#else
	STREQ(((char *)getlogin()==NULL ? " " : getlogin()), ANONYMOUS_USER))
#endif /* HAVE_CUSERID */
#endif /* VMS */
    {
	parse_restrictions("default");
	anon_restrictions_set = TRUE;
	LYUseTraceLog = FALSE;
    }

    /*
     *	Set up the TRACE log path, and logging if appropriate. - FM
     */
#ifdef VMS
    StrAllocCopy(LYTraceLogPath, "sys$login:Lynx.trace");
#else
    StrAllocCopy(LYTraceLogPath, (Home_Dir() ? Home_Dir() : ""));
    StrAllocCat(LYTraceLogPath, "/Lynx.trace");
#endif /* VMS */
    if (TRACE && LYUseTraceLog) {
	/*
	 *  If we can't open it for writing, give up.
	 *  Otherwise, on VMS close it, delete it and any
	 *  versions from previous sessions so they don't
	 *  accumulate, and open it again. - FM
	 */
	if ((LYTraceLogFP = LYNewTxtFile(LYTraceLogPath)) == NULL) {
	    WWW_TraceFlag = FALSE;
	    fprintf(stderr, "%s\n", TRACELOG_OPEN_FAILED);
	    exit(-1);
	}
#ifdef VMS
	LYCloseTracelog();
	while (remove(LYTraceLogPath) == 0)
	    ;
	if ((LYTraceLogFP = LYNewTxtFile(LYTraceLogPath)) == NULL) {
	    WWW_TraceFlag = FALSE;
	    printf("%s\n", TRACELOG_OPEN_FAILED);
	    exit(-1);
	}
#endif /* VMS */
	fflush(stdout);
	fflush(stderr);
	fprintf(tfp, "\t\t%s\n\n", LYNX_TRACELOG_TITLE);
    }

    /*
     *	If TRACE is on, indicate whether the
     *	anonymous restrictions are set. - FM
     */
    if (anon_restrictions_set) {
	CTRACE(tfp, "LYMain: Anonymous restrictions set.\n");
    }

    /*
     *	Set up the default jump file stuff. - FM
     */
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
		CTRACE(tfp, "Failed to register %s\n", temp);
	    }
	    FREE(temp);
	}
    }
#endif /* JUMPFILE */

    /*
     *	If no alternate configuration file was specified on
     *	the command line, see if it's in the environment.
     */
    if (!lynx_cfg_file) {
	if (((cp=getenv("LYNX_CFG")) != NULL) ||
	    (cp=getenv("lynx_cfg")) != NULL)
	    StrAllocCopy(lynx_cfg_file, cp);
    }

    /*
     *	If we still don't have a configuration file,
     *	use the userdefs.h definition.
     */
    if (!lynx_cfg_file)
	StrAllocCopy(lynx_cfg_file, LYNX_CFG_FILE);

    /*
     *	Convert a '~' in the configuration file path to $HOME.
     */
#ifndef _WINDOWS /* avoid the whole ~ thing for now */
   /* I think this should only be performed if lynx_cfg_file starts with ~/ */
   if ((lynx_cfg_file[0] == '~') && (lynx_cfg_file[1] == '/'))
     {
#ifdef VMS
	StrAllocCopy(temp, HTVMS_wwwName((char *)Home_Dir()));
#else
	StrAllocCopy(temp, Home_Dir());
#endif /* VMS */
	StrAllocCat(temp, lynx_cfg_file + 1);
	StrAllocCopy(lynx_cfg_file, temp);
	FREE(temp);
     }
#endif

    /*
     *	If the configuration file is not available,
     *	inform the user and exit.
     */
    if ((fp = fopen(lynx_cfg_file, "r")) == NULL) {
	fprintf(stderr, "\nConfiguration file %s is not available.\n\n",
			lynx_cfg_file);
	exit(-1);
    }
    fclose(fp);

#if defined(USE_SLANG_KEYMAPS) 
    if (-1 == lynx_initialize_keymaps ()) 
	exit (-1); 
#endif 
    /*
     * Make sure we have the character sets declared.
     *	This will initialize the CHARTRANS handling. - KW
     */
    if (!LYCharSetsDeclared()) {
	fprintf(stderr, "\nLynx character sets not declared.\n\n");
	exit(-1);
    }

#if defined(USE_HASH)
    /*
     *	If no alternate lynx-style file was specified on
     *	the command line, see if it's in the environment.
     */
    if (!lynx_lss_file) {
	if (((cp=getenv("LYNX_LSS")) != NULL) ||
	    (cp=getenv("lynx_lss")) != NULL)
	    StrAllocCopy(lynx_lss_file, cp);
    }

    /*
     *	If we still don't have a lynx-style file,
     *	use the userdefs.h definition.
     */
    if (!lynx_lss_file)
	StrAllocCopy(lynx_lss_file, LYNX_LSS_FILE);

    /*
     *	Convert a '~' in the lynx-style file path to $HOME.
     */
    if ((cp = strchr(lynx_lss_file, '~'))) {
	*(cp++) = '\0';
	StrAllocCopy(temp, lynx_lss_file);
	if ((len=strlen(temp)) > 0 && temp[len-1] == '/')
	    temp[len-1] = '\0';
#ifdef VMS
	StrAllocCat(temp, HTVMS_wwwName((char *)Home_Dir()));
#else
	StrAllocCat(temp, Home_Dir());
#endif /* VMS */
	StrAllocCat(temp, cp);
	StrAllocCopy(lynx_lss_file, temp);
	FREE(temp);
    }
    /*
     *	If the lynx-style file is not available,
     *	inform the user and exit.
     */
    if ((fp = fopen(lynx_lss_file, "r")) == NULL) {
	fprintf(stderr, "\nLynxile file %s is not available.\n\n",
			lynx_lss_file);
    }
    else
    {
	fclose(fp);
	style_readFromFile(lynx_lss_file);
    }
#endif

    /*
     *	Make sure we have the edit map declared. - FM
     */
    if (!LYEditmapDeclared()) {
	fprintf(stderr, "\nLynx edit map not declared.\n\n");
	exit(-1);
    }

#if USE_COLOR_TABLE
    /*
     *	Set up default foreground and background colors.
     */
    lynx_setup_colors();
#endif /* USE_SLANG */

    /*
     *	Set the compilation default signature file. - FM
     */
    strcpy(filename, LYNX_SIG_FILE);
    if (LYPathOffHomeOK(filename, sizeof(filename))) {
	StrAllocCopy(LynxSigFile, filename);
	LYAddPathToHome(filename, sizeof(filename), LynxSigFile);
	StrAllocCopy(LynxSigFile, filename);
	CTRACE(tfp, "LYNX_SIG_FILE set to '%s'\n", LynxSigFile);
    } else {
	CTRACE(tfp, "LYNX_SIG_FILE '%s' is bad. Ignoring.\n", LYNX_SIG_FILE);
    }

    /*
     *	Process the configuration file.
     */
    read_cfg(lynx_cfg_file, "main program", 1);
    FREE(lynx_cfg_file);

    HTSwitchDTD(New_DTD);

    /*
     *	Check for a save space path in the environment.
     *	If one was set in the configuration file, that
     *	one will be overridden. - FM
     */
    if ((cp=getenv("LYNX_SAVE_SPACE")) != NULL)
	StrAllocCopy(lynx_save_space, cp);

    /*
     *	We have a save space path, make sure it's valid. - FM
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
#ifdef DOSPATH
	    StrAllocCat(temp, HTDOS_wwwName((char *)Home_Dir()));
#else
#ifdef VMS
	    StrAllocCat(temp, HTVMS_wwwName((char *)Home_Dir()));
#else
	    StrAllocCat(temp, Home_Dir());
#endif /* VMS */
#endif /* DOSPATH */
	    StrAllocCat(temp, cp);
	    StrAllocCopy(lynx_save_space, temp);
	    FREE(temp);
	}
#ifdef VMS
	LYLowerCase(lynx_save_space);
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
     *	Set up the file extension and mime type maps from
     *	src/HTInit.c and the global and personal mime.types
     *	and mailcap files.  These will override any SUFFIX
     *	or VIEWER maps in userdefs.h or the configuration
     *	file, if they overlap.
     */
    HTFormatInit();
    HTFileInit();

    /*
     *	Get WWW_HOME environment variable if it exists.
     */
    if ((cp = getenv("WWW_HOME")) != NULL) {
	StrAllocCopy(startfile, cp);
	LYTrimHead(startfile);
	if (!strncasecomp(startfile, "lynxexec:", 9) ||
	    !strncasecomp(startfile, "lynxprog:", 9)) {
	    /*
	     *	The original implementations of these schemes expected
	     *	white space without hex escaping, and did not check
	     *	for hex escaping, so we'll continue to support that,
	     *	until that code is redone in conformance with SGML
	     *	principles.  - FM
	     */
	    HTUnEscapeSome(startfile, " \r\n\t");
	    convert_to_spaces(startfile, TRUE);
	}
    }

    /*
     *	Set the LynxHome URL.  If it's a file URL and the
     *	host is defaulted, force in "//localhost", and if
     *	it's not an absolute URL, make it one. - FM
     */
    StrAllocCopy(LynxHome, startfile);
    LYFillLocalFileURL((char **)&LynxHome, "file://localhost");
    LYEnsureAbsoluteURL((char **)&LynxHome, "LynxHome");

    /*
     *	Process any command line arguments not already handled. - FM
     */
    for (i = 1; i < argc; i++) {
	parse_arg(&argv[i], &i);
    }

    /*
     *	Process any stdin-derived arguments for a lone "-"  which we've
     *	loaded into LYStdinArgs. - FM
     */
    if (LYStdinArgs != NULL) {
	char *my_args[2];
	HTList *cur = LYStdinArgs;

	my_args[1] = NULL;
	while (NULL != (my_args[0] = (char *)HTList_nextObject(cur))) {
	    parse_arg(my_args, (int *)0);
	}
	LYStdinArgs_free();
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
     *	Process the RC file.
     */
    read_rc();

#ifdef USE_SLANG
    if (LYShowColor >= SHOW_COLOR_ON &&
	!(Lynx_Color_Flags & SL_LYNX_USE_COLOR)) {
	Lynx_Color_Flags |= SL_LYNX_USE_COLOR;
    } else if ((Lynx_Color_Flags & SL_LYNX_USE_COLOR) ||
	       getenv("COLORTERM") != NULL) {
	if (LYShowColor != SHOW_COLOR_NEVER &&
	    LYShowColor != SHOW_COLOR_ALWAYS) {
	    LYShowColor = SHOW_COLOR_ON;
	}
    }
#endif /* USE_SLANG */

    if (LYPreparsedSource) {
	HTPreparsedFormatInit();
    }

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
#ifdef NEVER_ALLOW_REMOTE_EXEC
    if (local_exec) {
	local_exec = FALSE;
	local_exec_on_local_files = TRUE;
    }
#endif /* NEVER_ALLOW_REMOTE_EXEC */
#endif /* EXEC_LINKS || EXEC_SCRIPTS */

    if (emacs_keys)
	set_emacs_keys();

    if (vi_keys)
	set_vi_keys();

    if (number_links && keypad_mode == NUMBERS_AS_ARROWS)
	keypad_mode = LINKS_ARE_NUMBERED;
    if (keypad_mode == NUMBERS_AS_ARROWS)
	set_numbers_as_arrows();

    /*
     *	Check the -popup command line toggle. - FM
     */
    if (LYUseDefSelPop == FALSE) {
	if (LYSelectPopups == TRUE)
	    LYSelectPopups = FALSE;
	else
	    LYSelectPopups = TRUE;
    }

    /*
     *	Check the -show_cursor command line toggle. - FM
     */
    if (LYUseDefShoCur == FALSE) {
	if (LYShowCursor == TRUE)
	    LYShowCursor = FALSE;
	else
	    LYShowCursor = TRUE;
    }

    /*
     *	Check the -base command line switch with -source. - FM
     */
    if (LYPrependBase && HTOutputFormat == HTAtom_for("www/download")) {
	LYPrependBaseToSource = TRUE;
    }

    /*
     *	Disable multiple bookmark support if not interactive,
     *	so it doesn't crash on curses functions, or if the
     *	support was blocked via userdefs.h and/or lynx.cfg,
     *	or via command line restrictions. - FM
     */
    if (no_multibook)
	LYMBMBlocked = TRUE;
    if (dump_output_immediately || LYMBMBlocked || no_multibook) {
	LYMultiBookmarks = FALSE;
	LYMBMBlocked = TRUE;
	no_multibook = TRUE;
    }

#ifdef VMS
    set_vms_keys();
#endif /* VMS */

    /* trap interrupts */
    if (!dump_output_immediately)
#ifndef NOSIGHUP
	(void) signal(SIGHUP, cleanup_sig);
#endif /* NOSIGHUP */
    (void) signal(SIGTERM, cleanup_sig);
#ifdef SIGWINCH
    (void) signal(SIGWINCH, size_change);
#endif /* SIGWINCH */
#ifndef VMS
    if (!TRACE && !dump_output_immediately && !stack_dump) {
	(void) signal(SIGINT, cleanup_sig);
#ifndef __linux__
#ifndef DOSPATH
	(void) signal(SIGBUS, FatalProblem);
#endif /* DOSPATH */
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
#ifndef DOSPATH
	(void) signal(SIGPIPE, SIG_IGN);
#endif /* DOSPATH */
    }
#endif /* !VMS */

    /*
     *	Set up the proper character set with the desired
     *	startup raw 8-bit or CJK mode handling.  - FM
     */
    HTMLUseCharacterSet(current_char_set);

    /*
     *	If startfile is a file URL and the host is defaulted,
     *	force in "//localhost", and if it's not an absolute URL,
     *	make it one. - FM
     */
    LYFillLocalFileURL((char **)&startfile, "file://localhost");
    LYEnsureAbsoluteURL((char **)&startfile, "STARTFILE");

    /*
     *	If homepage was specified and is a file URL with the
     *	host defaulted, force in "//localhost", and if it's
     *	not an absolute URL, make it one. - FM
     */
    if (homepage) {
	LYFillLocalFileURL((char **)&homepage, "file://localhost");
	LYEnsureAbsoluteURL((char **)&homepage, "HOMEPAGE");
    }

    /*
     *	If we don't have a homepage specified,
     *	set it to startfile.  Otherwise, reset
     *	LynxHome. - FM
     */
    if (!(homepage && *homepage)) {
	StrAllocCopy(homepage, startfile);
    } else {
	StrAllocCopy(LynxHome, homepage);
    }

    /*
     *	Set up the inside/outside domain restriction flags. - FM
     */
    if (inlocaldomain()) {
#if !defined(HAVE_UTMP) || defined(VMS) /* not selective */
	telnet_ok = !no_inside_telnet && !no_outside_telnet && telnet_ok;
	news_ok = !no_inside_news && !no_outside_news && news_ok;
	ftp_ok = !no_inside_ftp && !no_outside_ftp && ftp_ok;
	rlogin_ok = !no_inside_rlogin && !no_outside_rlogin && rlogin_ok;
#else
	CTRACE(tfp,"LYMain.c: User in Local domain\n");
	telnet_ok = !no_inside_telnet && telnet_ok;
	news_ok = !no_inside_news && news_ok;
	ftp_ok = !no_inside_ftp && ftp_ok;
	rlogin_ok = !no_inside_rlogin && rlogin_ok;
#endif /* !HAVE_UTMP || VMS */
    } else {
	CTRACE(tfp,"LYMain.c: User in REMOTE domain\n");
	telnet_ok = !no_outside_telnet && telnet_ok;
	news_ok = !no_outside_news && news_ok;
	ftp_ok = !no_outside_ftp && ftp_ok;
	rlogin_ok = !no_outside_rlogin && rlogin_ok;
    }

#ifdef SIGTSTP
    /*
     *	Block Control-Z suspending if requested. - FM
     */
    if (no_suspend)
	(void) signal(SIGTSTP,SIG_IGN);
#endif /* SIGTSTP */

    /*
     *	Check for a valid HEAD request. - FM
     */
    if (HEAD_request && LYCanDoHEAD(startfile) != TRUE) {
	fprintf(stderr,
 "The '-head' switch is for http HEAD requests and cannot be used for\n'%s'.\n",
		startfile);
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

    /*
     *	Check for a valid MIME headers request. - FM
     */
    if (keep_mime_headers && LYCanDoHEAD(startfile) != TRUE) {
	fprintf(stderr,
 "The '-mime_header' switch is for http URLs and cannot be used for\n'%s'.\n",
		startfile);
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

    /*
     *	Check for a valid traversal request. - FM
     */
    if (traversal && strncmp(startfile, "http", 4)) {
	fprintf(stderr,
 "The '-traversal' switch is for http URLs and cannot be used for\n'%s'.\n",
		startfile);
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

    /*
     *	Set up our help and about file base paths. - FM
     */
    StrAllocCopy(helpfilepath, helpfile);
    if ((cp=strrchr(helpfilepath, '/')) != NULL)
	*cp = '\0';
    /*
     *	Remove code to merge the historical about_lynx
     *	directory into lynx_help. - HN
     */
    StrAllocCat(helpfilepath, "/");


    /*
     *	Make sure our bookmark default strings
     *	are all allocated and synchronized. - FM
     */
    if (!bookmark_page || *bookmark_page == '\0') {
	StrAllocCopy(bookmark_page, "lynx_bookmarks");
	StrAllocCat(bookmark_page, HTML_SUFFIX);
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
     *	Here's where we do all the work.
     */
    if (dump_output_immediately) {
	/*
	 *  Finish setting up and start a
	 *  NON-INTERACTIVE session. - FM
	 */
	if (crawl && !number_links) {
	    keypad_mode = NUMBERS_AS_ARROWS;
	} else if (!nolist) {
	    if (keypad_mode == NUMBERS_AS_ARROWS) {
		keypad_mode = LINKS_ARE_NUMBERED;
	    }
	}
	if (display != NULL && *display != '\0') {
	    LYisConfiguredForX = TRUE;
	}
	if (dump_output_width > 0) {
	    LYcols = dump_output_width;
	}
	status = mainloop();
	if (!nolist &&
	    (keypad_mode == LINKS_ARE_NUMBERED ||
	     keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED))
	    printlist(stdout,FALSE);
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
    } else {
	/*
	 *  Finish setting up and start an
	 *  INTERACTIVE session. - FM
	 */
	if (setup(terminal)) {
	    if (display != NULL && *display != '\0') {
		LYisConfiguredForX = TRUE;
	    }
	    ena_csi((LYlowest_eightbit[current_char_set] > 155));
	    status = mainloop();
	    cleanup();
	}
    }

    exit(status);
    return(status);	/* though redundant, for compiler-warnings */
}

/*
 *  Called by HTAccessInit to register any protocols supported by lynx.
 *  Protocols added by lynx:
 *    LYNXKEYMAP, lynxcgi, LYNXIMGMAP, LYNXCOOKIE
 */
#ifdef GLOBALREF_IS_MACRO
extern GLOBALREF (HTProtocol, LYLynxKeymap);
extern GLOBALREF (HTProtocol, LYLynxCGI);
extern GLOBALREF (HTProtocol, LYLynxIMGmap);
extern GLOBALREF (HTProtocol, LYLynxCookies);
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

/* There are different ways of setting arguments on the command line, and
 * there are different types of arguments.  These include:
 *
 *   -set_some_variable 	 ==> some_variable  = TRUE
 *   -toggle_some_variable	 ==> some_variable = !some_variable
 *   -some_variable=value	 ==> some_variable = value
 *
 * Others are complicated and require a function call.
 */

struct parse_args_type;
typedef int (*ParseFunc) PARAMS((struct parse_args_type *, char **, char *));

typedef union {
	BOOLEAN * set_value;
	int *     int_value;
	char **   str_value;
	ParseFunc fun_value;
} ParseUnion;

#undef  PARSE_DEBUG

/*
 * Storing the four types of data in separate fields costs about 1K of data.
 * However, this provides usable type-checking.  The initial version of the
 * parse_args_type used 'long' for all types, and dumped core when processing
 * "lynx -help".  (The compiler was unable to detect some minor errors).
 */
#ifdef  PARSE_DEBUG
#define ParseData BOOLEAN *set_value; int *int_value; char **str_value; ParseFunc fun_value
#define PARSE_SET(n,t,v,h) {n,t,    v,  0,  0,  0,    h}
#define PARSE_INT(n,t,v,h) {n,t,    0, &v,  0,  0,    h}
#define PARSE_STR(n,t,v,h) {n,t,    0,  0,  v,  0,    h}
#define PARSE_FUN(n,t,v,h) {n,t,    0,  0,  0,  v,    h}
#else
#define ParseData long value
#define PARSE_SET(n,t,v,h) {n,t,   (long) (v),        h}
#define PARSE_INT(n,t,v,h) {n,t,   (long)&(v),        h}
#define PARSE_STR(n,t,v,h) {n,t,   (long) (v),        h}
#define PARSE_FUN(n,t,v,h) {n,t,   (long) (v),        h}
#endif

typedef struct parse_args_type
{
   char *name;
   int type;
#define IGNORE_ARG		0x000
#define TOGGLE_ARG		0x001
#define SET_ARG 		0x002
#define UNSET_ARG		0x003
#define FUNCTION_ARG		0x004
#define LYSTRING_ARG		0x005
#define INT_ARG 		0x006
#define STRING_ARG		0x007
#define ARG_TYPE_MASK		0x0FF
#define NEED_NEXT_ARG		0x100

#define NEED_INT_ARG		(NEED_NEXT_ARG | INT_ARG)
#define NEED_LYSTRING_ARG	(NEED_NEXT_ARG | LYSTRING_ARG)
#define NEED_STRING_ARG		(NEED_NEXT_ARG | STRING_ARG)
#define NEED_FUNCTION_ARG	(NEED_NEXT_ARG | FUNCTION_ARG)

   /* If the NEED_NEXT_ARG flags is set, and the option was not specified
    * with an '=' character, then use the next argument in the argv list.
    */

   ParseData;
   char *help_string;
}
Parse_Args_Type;

/* -auth, -pauth */
static int parse_authentication ARGS2(
	char *,			next_arg,
	char **,		result)
{
    /*
     *  Authentication information for protected documents.
     */
    char *auth_info = 0;

    if (next_arg != 0) {
	StrAllocCopy(auth_info, next_arg);
	memset(next_arg, ' ', strlen(next_arg));  /* Let's not show too much */
    }

    if (auth_info != 0) {
	char *cp;

	if ((cp = strchr(auth_info, ':')) != 0) {		/* Pw */
	    *cp++ = '\0';	/* Terminate ID */
	    HTUnEscape(cp);
	    StrAllocCopy(result[1], cp);
	}
	if (*auth_info) {					/* Id */
	    HTUnEscape(auth_info);
	    StrAllocCopy(result[0], auth_info);
	}
	FREE(auth_info);
    }
    return 0;
}

/* -anonymous */
static int anonymous_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
   /*
    *  Should already have been set, so we don't
    *  override or replace any additional
    *  restrictions from the command line. - FM
    */
   if (!anon_restrictions_set)
      parse_restrictions("default");
   anon_restrictions_set = TRUE;
   return 0;
}

/* -assume_charset */
static int assume_charset_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg == 0) {
	UCLYhndl_for_unspec = 0;
    } else {
        LYLowerCase(next_arg);
        StrAllocCopy(UCAssume_MIMEcharset, next_arg);
        if (UCAssume_MIMEcharset && *UCAssume_MIMEcharset)
            UCLYhndl_for_unspec = UCGetLYhndl_byMIME(UCAssume_MIMEcharset);
    }
    return 0;
}

/* -assume_local_charset */
static int assume_local_charset_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg == 0) {
	UCLYhndl_HTFile_for_unspec = 0;
    } else {
        LYLowerCase(next_arg);
        StrAllocCopy(UCAssume_localMIMEcharset, next_arg);
        if (UCAssume_localMIMEcharset && *UCAssume_localMIMEcharset)
            UCLYhndl_HTFile_for_unspec =
	        UCGetLYhndl_byMIME(UCAssume_localMIMEcharset);
    }
    return 0;
}

/* -assume_unrec_charset */
static int assume_unrec_charset_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg == 0) {
	UCLYhndl_for_unrec = 0;
    } else {
	LYLowerCase(next_arg);
	StrAllocCopy(UCAssume_unrecMIMEcharset, next_arg);
	if (UCAssume_unrecMIMEcharset && *UCAssume_unrecMIMEcharset)
	    UCLYhndl_for_unrec = UCGetLYhndl_byMIME(UCAssume_unrecMIMEcharset);
    }
    return 0;
}

/* -auth */
static int auth_fun ARGS3(
	Parse_Args_Type*,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    parse_authentication(next_arg, authentication_info);
    return 0;
}

/* -base */
static int base_fun ARGS3(
	Parse_Args_Type*,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    /*
     *  Treat -source equivalently to an interactive download with
     *  LYPrefixBaseToSource configured to TRUE, so that a BASE tag is
     *  prepended for text/html content types.  We normally treat the
     *  module-wide global LYPrefixBaseToSource flag as FALSE with
     *  -source, but force it TRUE, later, if LYPrependBase is set
     *  TRUE here. - FM
     */
    LYPrependBase = TRUE;
    if (HTOutputFormat == HTAtom_for("www/dump"))
        HTOutputFormat = HTAtom_for("www/download");

    return 0;
}

#ifdef USE_SLANG
/* -blink */
static int blink_fun ARGS3(
	Parse_Args_Type*,	p,
	char **,		argv,
	char *,			next_arg)
{
    Lynx_Color_Flags |= SL_LYNX_USE_BLINK;
    return 0;
}
#endif

/* -cache */
static int cache_fun ARGS3(
	Parse_Args_Type*,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg != 0)
        HTCacheSize = atoi(next_arg);
    /*
     *  Limit size.
     */
    if (HTCacheSize < 2) HTCacheSize = 2;

    return 0;
}

/* -child */
static int child_fun ARGS3(
	Parse_Args_Type*,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    child_lynx = TRUE;
    no_disk_save = TRUE;
    return 0;
}

#ifdef USE_SLANG
/* -color */
static int color_fun ARGS3(
	Parse_Args_Type*,	p,
	char **,		argv,
	char *,			next_arg)
{
    Lynx_Color_Flags |= SL_LYNX_USE_COLOR;

    if (LYShowColor != SHOW_COLOR_ALWAYS)
        LYShowColor = SHOW_COLOR_ON;

    return 0;
}
#endif

/* -crawl */
static int crawl_fun ARGS3(
	Parse_Args_Type*,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    crawl = TRUE;
    LYcols = 80;
    return 0;
}

/* -display */
static int display_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg != 0) {
#ifdef VMS
	LYUpperCase(next_arg);
	Define_VMSLogical(DISPLAY, next_arg);
#else
	static char display_putenv_command[142];

	sprintf(display_putenv_command, "DISPLAY=%s", next_arg);
	putenv(display_putenv_command);
#endif /* VMS */

	if ((0 != (next_arg = getenv(DISPLAY)))
	    && (*next_arg != '\0'))
	  StrAllocCopy(display, next_arg);
    }

    return 0;
}

/* -dump */
static int dump_output_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    dump_output_immediately = TRUE;
    LYcols = 80;
    return 0;
}

/* -editor */
static int editor_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg != 0)
	StrAllocCopy(editor, next_arg);
    system_editor = TRUE;
    return 0;
}

/* -error_file */
static int error_file_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    /*
     *  Output return (success/failure) code
     *  of an HTTP transaction.
     */
    if (next_arg != 0)
        http_error_file = next_arg;
    return 0;
}

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
/* -exec */
static int exec_fun ARGS3(
	Parse_Args_Type*,	p,
	char **,		argv,
	char *,			next_arg)
{
#ifndef NEVER_ALLOW_REMOTE_EXEC
    local_exec = TRUE;
#else
    local_exec_on_local_files = TRUE;
#endif /* NEVER_ALLOW_REMOTE_EXEC */
    return 0;
}
#endif

/* -get_data */
static int get_data_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
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
	    (buf[j] == CR || buf[j] == LF); j--)
	    buf[j] = '\0';

        StrAllocCat(*get_data, buf);
    }

    return 0;
}

/* -help */
static int help_fun ARGS3(
	Parse_Args_Type*,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    print_help_and_exit (0);
    return 0;
}

/* -hiddenlinks */
static int hiddenlinks_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg != 0) {
	if (strncasecomp(next_arg, "merge", 1) == 0)
	    LYHiddenLinks = HIDDENLINKS_MERGE;
	else if (strncasecomp(next_arg, "listonly", 1) == 0)
	    LYHiddenLinks = HIDDENLINKS_SEPARATE;
	else if (strncasecomp(next_arg, "ignore", 1) == 0)
	    LYHiddenLinks = HIDDENLINKS_IGNORE;
	else
	    print_help_and_exit (-1);
    } else {
        LYHiddenLinks = HIDDENLINKS_MERGE;
    }

    return 0;
}

/* -homepage */
static int homepage_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg != 0) {
	StrAllocCopy(homepage, next_arg);
	LYTrimHead(homepage);
	if (!strncasecomp(homepage, "lynxexec:", 9) ||
	    !strncasecomp(homepage, "lynxprog:", 9)) {
	    /*
	     *  The original implementations of these schemes expected
	     *  white space without hex escaping, and did not check
	     *  for hex escaping, so we'll continue to support that,
	     *  until that code is redone in conformance with SGML
	     *  principles.  - FM
	     */
	    HTUnEscapeSome(homepage, " \r\n\t");
	    convert_to_spaces(homepage, TRUE);
	}
    }
    return 0;
}

/* -mime_header */
static int mime_header_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    /*
     *  Include mime headers and force source dump.
     */
    keep_mime_headers = TRUE;
    dump_output_immediately = TRUE;
    HTOutputFormat = (LYPrependBase ?
		      HTAtom_for("www/download") : HTAtom_for("www/dump"));
    LYcols = 999;
    return 0;
}

/* -newschunksize */
static int newschunksize_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg != 0) {
	HTNewsChunkSize = atoi(next_arg);
	/*
	 * If the new HTNewsChunkSize exceeds the maximum,
	 * increase HTNewsMaxChunk to this size. - FM
	 */
	if (HTNewsChunkSize > HTNewsMaxChunk)
	    HTNewsMaxChunk = HTNewsChunkSize;
    }
    return 0;
}

/* -newsmaxchunk */
static int newsmaxchunk_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg) {
	HTNewsMaxChunk = atoi(next_arg);
	/*
	 * If HTNewsChunkSize exceeds the new maximum,
	 * reduce HTNewsChunkSize to this maximum. - FM
	 */
	if (HTNewsChunkSize > HTNewsMaxChunk)
	    HTNewsChunkSize = HTNewsMaxChunk;
    }
    return 0;
}

/* -nobrowse */
static int nobrowse_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
   HTDirAccess = HT_DIR_FORBID;
   return 0;
}

/* -nocolor */
static int nocolor_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    LYShowColor = SHOW_COLOR_NEVER;
#ifdef USE_SLANG
    Lynx_Color_Flags &= ~SL_LYNX_USE_COLOR;
    Lynx_Color_Flags |= SL_LYNX_OVERRIDE_COLOR;
#endif
    return 0;
}

/* -nopause */
static int nopause_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    InfoSecs = 0;
    MessageSecs = 0;
    AlertSecs = 0;
    return 0;
}

/* -pauth */
static int pauth_fun ARGS3(
	Parse_Args_Type*,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    parse_authentication(next_arg, proxyauth_info);
    return 0;
}

/* -post_data */
static int post_data_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    /*
     *  User data for POST form.
     */
    char **post_data;
    char buf[1024];

    /*
     * On Unix, conflicts with curses when interactive so let's force a dump.
     * - CL
     *
     * On VMS, mods have been made in LYCurses.c to deal with potential
     * conflicts, so don't force a dump here.  - FM
     */
#ifndef VMS
    dump_output_immediately = TRUE;
    LYcols = 80;
#endif /* VMS */

    post_data = &form_post_data;

    /*
     * Build post data for later.  Stop reading when we see a line with "---"
     * as its first three characters.
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
    return 0;
}

/* -restrictions */
static int restrictions_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    static CONST char *Usage[] = {
 ""
,"   USAGE: lynx -restrictions=[option][,option][,option]"
,"   List of Options:"
,"   all             restricts all options."
,"   bookmark        disallow changing the location of the bookmark file."
,"   bookmark_exec   disallow execution links via the bookmark file"
#if defined(DIRED_SUPPORT) && defined(OK_PERMIT)
,"   change_exec_perms  disallow changing the eXecute permission on files"
,"                   (but still allow it for directories) when local file"
,"                   management is enabled."
#endif /* DIRED_SUPPORT && OK_PERMIT */
,"   default         same as commandline option -anonymous.  Disables"
,"                   default services for anonymous users.  Currently set to,"
,"                   all restricted except for: inside_telnet, outside_telnet,"
,"                   inside_news, inside_ftp, outside_ftp, inside_rlogin,"
,"                   outside_rlogin, goto, jump and mail.  Defaults"
,"                   are settable within userdefs.h"
#ifdef DIRED_SUPPORT
,"   dired_support   disallow local file management"
#endif /* DIRED_SUPPORT */
,"   disk_save       disallow saving to disk in the download and print menus"
,"   dotfiles        disallow access to, or creation of, hidden (dot) files"
,"   download        disallow downloaders in the download menu"
,"   editor          disallow editing"
,"   exec            disable execution scripts"
,"   exec_frozen     disallow the user from changing the execution link"
#ifdef USE_EXTERNALS
,"   externals       disable passing URLs to external programs"
#endif
,"   file_url        disallow using G)oto, served links or bookmarks for"
,"                   file: URL's"
,"   goto            disable the 'g' (goto) command"
#if !defined(HAVE_UTMP) || defined(VMS) /* not selective */
,"   inside_ftp      disallow ftps for people coming from inside your"
,"                   domain (utmp required for selectivity)"
,"   inside_news     disallow USENET news posting for people coming from"
,"                   inside your domain (utmp required for selectivity)"
,"   inside_rlogin   disallow rlogins for people coming from inside your"
,"                   domain (utmp required for selectivity)"
,"   inside_telnet   disallow telnets for people coming from inside your"
,"                   domain (utmp required for selectivity)"
#else
,"   inside_ftp      disallow ftps for people coming from inside your domain"
,"   inside_news     disallow USENET news posting for people coming from inside"
,"                   your domain"
,"   inside_rlogin   disallow rlogins for people coming from inside your domain"
,"   inside_telnet   disallow telnets for people coming from inside your domain"
#endif /* HAVE_UTMP || VMS */
,"   jump            disable the 'j' (jump) command"
,"   mail            disallow mail"
,"   multibook       disallow multiple bookmark files"
,"   news_post       disallow USENET News posting."
,"   option_save     disallow saving options in .lynxrc"
#if !defined(HAVE_UTMP) || defined(VMS) /* not selective */
,"   outside_ftp     disallow ftps for people coming from outside your"
,"                   domain (utmp required for selectivity)"
,"   outside_news    disallow USENET news posting for people coming from"
,"                   outside your domain (utmp required for selectivity)"
,"   outside_rlogin  disallow rlogins for people coming from outside your"
,"                   domain (utmp required for selectivity)"
,"   outside_telnet  disallow telnets for people coming from outside your"
,"                   domain (utmp required for selectivity)"
#else
,"   outside_ftp     disallow ftps for people coming from outside your domain"
,"   outside_news    disallow USENET news posting for people coming from outside"
,"                   your domain"
,"   outside_rlogin  disallow rlogins for people coming from outside your domain"
,"   outside_telnet  disallow telnets for people coming from outside your domain"
#endif /* !HAVE_UTMP || VMS */
,"   print           disallow most print options"
,"   shell           disallow shell escapes, and lynxexec, lynxprog or lynxcgi"
,"                   G)oto's"
,"   suspend         disallow Control-Z suspends with escape to shell"
,"   telnet_port     disallow specifying a port in telnet G)oto's"
,"   useragent       disallow modifications of the User-Agent header"
    };
    size_t n;

    if (next_arg != 0) {
	parse_restrictions(next_arg);
    } else {
        for (n = 0; n < sizeof(Usage)/sizeof(Usage[0]); n++)
            printf("%s\n", Usage[n]);
        exit(0);
    }
    return 0;
}

/* -selective */
static int selective_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
   HTDirAccess = HT_DIR_SELECTIVE;
   return 0;
}

/* -source */
static int source_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    dump_output_immediately = TRUE;
    HTOutputFormat = (LYPrependBase ?
		      HTAtom_for("www/download") : HTAtom_for("www/dump"));
    LYcols = 999;
    return 0;
}

/* -tagsoup */
static int tagsoup_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    HTSwitchDTD(New_DTD = NO);
    return 0;
}

/* -traversal */
static int traversal_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    traversal = TRUE;
#ifdef USE_SLANG
    LYcols = 80;
#else
    LYcols = 999;
#endif /* USE_SLANG */

    return 0;
}

/* -version */
static int version_fun ARGS3(
	Parse_Args_Type *,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg GCC_UNUSED)
{
    printf("\n%s Version %s (1998)\n", LYNX_NAME, LYNX_VERSION);
    printf(
	  "Copyrights held by the University of Kansas, CERN, and other contributors.\n"
	  );
    printf("Distributed under the GNU General Public License.\n");
    printf(
	  "See http://lynx.browser.org/ and the online help for more information.\n\n"
	  );
    exit(0);
    /* NOT REACHED */
    return 0;
}

/* -width */
static int width_fun ARGS3(
	Parse_Args_Type*,	p GCC_UNUSED,
	char **,		argv GCC_UNUSED,
	char *,			next_arg)
{
    if (next_arg != 0) {
	int w = atoi(next_arg);
	if (w > 0)
	    dump_output_width = ((w < 999) ? w : 999);
    }

    return 0;
}

static Parse_Args_Type Arg_Table [] =
{
   PARSE_FUN(
      "anonymous",	FUNCTION_ARG,	anonymous_fun,
      "used to specify the anonymous account"
   ),
   PARSE_FUN(
      "assume_charset", NEED_FUNCTION_ARG, assume_charset_fun,
      "=MIMEname\ncharset for documents that don't specify it"
   ),
   PARSE_FUN(
      "assume_local_charset",	NEED_FUNCTION_ARG,assume_local_charset_fun,
      "=MIMEname\ncharset assumed for local files"
   ),
   PARSE_FUN(
      "assume_unrec_charset",	NEED_FUNCTION_ARG,assume_unrec_charset_fun,
      "=MIMEname\nuse this instead of unrecognized charsets"
   ),
   PARSE_FUN(
      "auth",		NEED_FUNCTION_ARG,	auth_fun,
      "=id:pw\nauthentication information for protected documents"
   ),
   PARSE_FUN(
      "base",		FUNCTION_ARG,		base_fun,
      "prepend a request URL comment and BASE tag to text/html\noutputs for -source dumps"
   ),
#ifdef USE_SLANG
   PARSE_FUN(
      "blink",		FUNCTION_ARG,		blink_fun,
      "force high intensity bg colors in color mode"
   ),
#endif
   PARSE_SET(
      "book",		SET_ARG,		&bookmark_start,
      "use the bookmark page as the startfile"
   ),
   PARSE_SET(
      "buried_news",	TOGGLE_ARG,		&scan_for_buried_news_references,
      "toggles scanning of news articles for buried references"
   ),
   PARSE_FUN(
      "cache",		NEED_FUNCTION_ARG,	cache_fun,
      "=NUMBER\nNUMBER of documents cached in memory"
   ),
   PARSE_SET(
      "case",		SET_ARG,		&case_sensitive,
      "enable case sensitive user searching"
   ),
   PARSE_STR(
      "cfg",		IGNORE_ARG|NEED_NEXT_ARG,	0,
      "=FILENAME\nspecifies a lynx.cfg file other than the default"
   ),
   PARSE_FUN(
      "child",		FUNCTION_ARG,		child_fun,
      "exit on left-arrow in startfile, and disable save to disk"
   ),
#ifdef USE_SLANG
   PARSE_FUN(
      "color",		FUNCTION_ARG,		color_fun,
      "force color mode on with standard bg colors"
   ),
#endif
   PARSE_SET(
      "cookies",	TOGGLE_ARG,		&LYSetCookies,
      "toggles handling of Set-Cookie headers"
   ),
#ifndef VMS
   PARSE_SET(
      "core",		TOGGLE_ARG,		&LYNoCore,
      "toggles forced core dumps on fatal errors"
   ),
#endif
   PARSE_FUN(
      "crawl",		FUNCTION_ARG,		crawl_fun,
      "with -traversal, output each page to a file\n\
with -dump, format output as with -traversal, but to stdout"
   ),
   PARSE_FUN(
      "display",	NEED_FUNCTION_ARG,	display_fun,
      "=DISPLAY\nset the display variable for X exec'ed programs"
   ),
   PARSE_FUN(
      "dump",		FUNCTION_ARG,		dump_output_fun,
      "dump the first file to stdout and exit"
   ),
   PARSE_FUN(
      "editor", 	NEED_FUNCTION_ARG,	editor_fun,
      "=EDITOR\nenable edit mode with specified editor"
   ),
   PARSE_SET(
      "emacskeys",	SET_ARG,		&emacs_keys,
      "enable emacs-like key movement"
   ),
   PARSE_SET(
      "enable_scrollback", TOGGLE_ARG,		&enable_scrollback,
      "\ntoggles compatibility with comm programs' scrollback\n\
keys (may be incompatible with some curses packages)"
   ),
   PARSE_FUN(
      "error_file",	NEED_FUNCTION_ARG,	error_file_fun,
      "=FILE\nwrite the HTTP status code here"
   ),
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
#ifndef NEVER_ALLOW_REMOTE_EXEC
   PARSE_FUN(
      "exec",		FUNCTION_ARG,		exec_fun,
      "enable local program execution"
   ),
#endif
   PARSE_SET(
      "locexec",	SET_ARG,		&local_exec_on_local_files,
      "enable local program execution from local files only"
   ),
   PARSE_SET(
      "noexec", 	UNSET_ARG,		&local_exec,
      "disable local program execution (DEFAULT)"
   ),
#endif /* EXEC_LINKS || EXEC_SCRIPTS */
#ifdef VMS
   PARSE_SET(
      "fileversions",	SET_ARG,		&HTVMSFileVersions,
      "include all versions of files in local VMS directory\nlistings"
   ),
#endif
   PARSE_SET(
      "force_html",	SET_ARG,		&LYforce_HTML_mode,
      "forces the first document to be interpreted as HTML"
   ),
   PARSE_SET(
      "force_secure",	TOGGLE_ARG,		&LYForceSSLCookiesSecure,
      "toggles forcing of the secure flag for SSL cookies"
   ),
   PARSE_SET(
      "from",		TOGGLE_ARG,		&LYNoFromHeader,
      "toggle transmissions of From headers"
   ),
   PARSE_SET(
      "ftp",		UNSET_ARG,		&ftp_ok,
      "disable ftp access"
   ),
   PARSE_FUN(
      "get_data",	FUNCTION_ARG,		get_data_fun,
      "user data for get forms, read from stdin,\nterminated by '---' on a line"
   ),
   PARSE_SET(
      "head",		SET_ARG,		&HEAD_request,
      "send a HEAD request"
   ),
   PARSE_FUN(
      "help",		FUNCTION_ARG,		help_fun,
      "print this usage message"
   ),
   PARSE_FUN(
      "hiddenlinks",	NEED_FUNCTION_ARG,	hiddenlinks_fun,
      "=[option]\nhidden links: options are merge, listonly, or ignore"
   ),
   PARSE_SET(
      "historical",	TOGGLE_ARG,		&historical_comments,
      "toggles use of '>' or '-->' as a terminator for comments"
   ),
   PARSE_FUN(
      "homepage",	NEED_FUNCTION_ARG,	homepage_fun,
      "=URL\nset homepage separate from start page"
   ),
   PARSE_SET(
      "image_links",	TOGGLE_ARG,		&clickable_images,
      "toggles inclusion of links for all images"
   ),
   PARSE_STR(
      "index",		NEED_LYSTRING_ARG,	&indexfile,
      "=URL\nset the default index file to URL"
   ),
   PARSE_SET(
      "ismap",		TOGGLE_ARG,		&LYNoISMAPifUSEMAP,
      "toggles inclusion of ISMAP links when client-side\nMAPs are present"
   ),
   PARSE_INT(
      "link",		NEED_INT_ARG,		ccount,
      "=NUMBER\nstarting count for lnk#.dat files produced by -crawl"
   ),
   PARSE_SET(
      "localhost",	SET_ARG,		&local_host_only,
      "disable URLs that point to remote hosts"
   ),
#if defined(USE_HASH)
   PARSE_STR(
      "lss",		IGNORE_ARG|NEED_NEXT_ARG,	0,
      "=FILENAME\nspecifies a lynx.css file other than the default"
   ),
#endif
   PARSE_FUN(
      "mime_header",	FUNCTION_ARG,		mime_header_fun,
      "include mime headers and force source dump"
   ),
   PARSE_SET(
      "minimal",	TOGGLE_ARG,		&minimal_comments,
      "toggles minimal versus valid comment parsing"
   ),
   PARSE_FUN(
      "newschunksize",	NEED_FUNCTION_ARG,	newschunksize_fun,
      "=NUMBER\nnumber of articles in chunked news listings"
   ),
   PARSE_FUN(
      "newsmaxchunk",	NEED_FUNCTION_ARG,	newsmaxchunk_fun,
      "=NUMBER\nmaximum news articles in listings before chunking"
   ),
   PARSE_FUN(
      "nobrowse",	FUNCTION_ARG,		nobrowse_fun,
      "disable directory browsing"
   ),
   PARSE_SET(
      "nocc",		SET_ARG,		&LYNoCc,
      "disable Cc: prompts for self copies of mailings"
   ),
   PARSE_FUN(
      "nocolor",	FUNCTION_ARG,		nocolor_fun,
      "turn off color support"
   ),
   PARSE_SET(
      "nofilereferer",	SET_ARG,		&no_filereferer,
      "disable transmissions of Referer headers for file URLs"
   ),
   PARSE_SET(
      "nolist", 	SET_ARG,		&nolist,
      "disable the link list feature in dumps"
   ),
   PARSE_SET(
      "nolog",		UNSET_ARG,		&error_logging,
      "disable mailing of error messages to document owners"
   ),
   PARSE_FUN(
      "nopause",	FUNCTION_ARG,		nopause_fun,
      "disable forced pauses for statusline messages"
   ),
   PARSE_SET(
      "noprint",	SET_ARG,		&no_print,
      "disable print functions"
   ),
   PARSE_SET(
      "noredir",	SET_ARG,		&no_url_redirection,
      "don't follow Location: redirection"
   ),
   PARSE_SET(
      "noreferer",	SET_ARG,		&LYNoRefererHeader,
      "disable transmissions of Referer headers"
   ),
#ifdef SOCKS
   PARSE_SET(
      "nosocks",	UNSET_ARG,		&socks_flag,
      "don't use SOCKS proxy for this session"
   ),
#endif
   PARSE_SET(
      "nostatus",	SET_ARG,		&no_statusline,
      "disable the miscellaneous information messages"
   ),
   PARSE_SET(
      "number_links",	SET_ARG,		&number_links,
      "force numbering of links"
   ),
   PARSE_SET(
      "partial",	TOGGLE_ARG,		&display_partial,
      "display partial pages while downloading"
   ),
   PARSE_FUN(
      "pauth",		NEED_FUNCTION_ARG,	pauth_fun,
      "=id:pw\nauthentication information for protected proxy server"
   ),
   PARSE_SET(
      "popup",		UNSET_ARG,		&LYUseDefSelPop,
      "toggles handling of single-choice SELECT options via\npopup windows or as lists of radio buttons"
   ),
   PARSE_FUN(
      "post_data",	FUNCTION_ARG,		post_data_fun,
      "user data for post forms, read from stdin,\nterminated by '---' on a line"
   ),
   PARSE_SET(
      "preparsed",	SET_ARG,		&LYPreparsedSource,
      "show parsed text/html with -source and in source view"
   ),
   PARSE_SET(
      "print",		UNSET_ARG,		&no_print,
      "enable print functions (DEFAULT)"
   ),
   PARSE_SET(
      "pseudo_inlines", TOGGLE_ARG,		&pseudo_inline_alts,
      "toggles pseudo-ALTs for inlines with no ALT string"
   ),
   PARSE_SET(
      "raw",		UNSET_ARG,		&LYUseDefaultRawMode,
      "toggles default setting of 8-bit character translations\nor CJK mode for the startup character set"
   ),
   PARSE_SET(
      "realm",		SET_ARG,		&check_realm,
      "restricts access to URLs in the starting realm"
   ),
   PARSE_SET(
      "reload", 	SET_ARG,		&reloading,
      "flushes the cache on a proxy server\n(only the first document affected)"
   ),
   PARSE_FUN(
      "restrictions",	FUNCTION_ARG,		restrictions_fun,
      "=[options]\nuse -restrictions to see list"
   ),
   PARSE_SET(
      "resubmit_posts", TOGGLE_ARG,		&LYresubmit_posts,
      "toggles forced resubmissions (no-cache) of forms with\n\
method POST when the documents they returned are sought\n\
with the PREV_DOC command or from the History List"
   ),
   PARSE_SET(
      "rlogin", 	UNSET_ARG,		&rlogin_ok,
      "disable rlogins"
   ),
   PARSE_FUN(
      "selective",	FUNCTION_ARG,		selective_fun,
      "require .www_browsable files to browse directories"
   ),
   PARSE_SET(
      "show_cursor",	TOGGLE_ARG,		&LYUseDefShoCur,
      "toggles hiding of the cursor in the lower right corner"
   ),
   PARSE_SET(
      "soft_dquotes",	TOGGLE_ARG,		&soft_dquotes,
      "toggles emulation of the old Netscape and Mosaic bug which\n\
treated '>' as a co-terminator for double-quotes and tags"
   ),
   PARSE_FUN(
      "source", 	FUNCTION_ARG,		source_fun,
      "dump the source of the first file to stdout and exit"
   ),
   PARSE_SET(
      "stack_dump",	SET_ARG,		&stack_dump,
      "disable SIGINT cleanup handler"
   ),
   PARSE_SET(
      "startfile_ok",	SET_ARG,		&startfile_ok,
      "allow non-http startfile and homepage with -validate"
   ),
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
   PARSE_STR(
      "syslog", 	NEED_LYSTRING_ARG, 	&syslog_txt,
      "=text\ninformation for syslog call"
   ),
#endif
#endif
   PARSE_FUN(
      "tagsoup",	FUNCTION_ARG,		tagsoup_fun,
      "use TagSoup rather than SortaSGML parser"
   ),
   PARSE_SET(
      "telnet", 	UNSET_ARG,		&telnet_ok,
      "disable telnets"
   ),
   PARSE_STR(
      "term",		NEED_STRING_ARG,	&terminal,
      "=TERM\nset terminal type to TERM"
   ),
   PARSE_SET(
      "tlog",		IGNORE_ARG,		0,
      "toggles use of a Lynx Trace Log for the current session"
   ),
   PARSE_SET(
      "trace",		IGNORE_ARG,		0,
      "turns on Lynx trace mode"
   ),
   PARSE_FUN(
      "traversal",	FUNCTION_ARG,		traversal_fun,
      "traverse all http links derived from startfile"
   ),
   PARSE_SET(
      "underscore",	TOGGLE_ARG,		&use_underscore,
      "toggles use of _underline_ format in dumps"
   ),
#if defined(NCURSES_MOUSE_VERSION) || defined(USE_SLANG_MOUSE)
   PARSE_SET(
      "use_mouse",	SET_ARG,		&LYUseMouse,
      "turn on xterm mouse support"
   ),
#endif
   PARSE_STR(
      "useragent",	NEED_LYSTRING_ARG,	&LYUserAgent,
      "=Name\nset alternate Lynx User-Agent header"
   ),
   PARSE_SET(
      "validate",	IGNORE_ARG,		0,
      "accept only http URLs (for validation)"
   ),
   PARSE_FUN(
      "version",	FUNCTION_ARG,		version_fun,
      "print Lynx version information"
   ),
   PARSE_SET(
      "vikeys", 	SET_ARG,		&vi_keys,
      "enable vi-like key movement"
   ),
   PARSE_FUN(
      "width",		NEED_FUNCTION_ARG,	width_fun,
      "=NUMBER\nscreen width for formatting of dumps (default is 80)"
   ),
   {NULL}
};

static void print_help_strings ARGS2(
	char *,	name,
	char *,	help)
{
    int pad;
    int c;
    int first;
    int field_width = 21;

    pad = field_width - (5 + (int) strlen (name));

    fprintf (stdout, "    -%s", name);

    if (*help != '=') {
	pad--;
	while (pad > 0) {
	    fputc (' ', stdout);
	    pad--;
	}
	fputc (' ', stdout);	  /* at least one space */
	first = 0;
    } else {
	first = pad;
    }

    if (strchr (help, '\n') == 0) {
	fprintf (stdout, "%s\n", help);
    } else {
	while ((c = *help) != 0) {
	    if (c == '\n') {
		if ((pad = --first) < 0) {
		    pad = field_width;
		} else {
		    c = ' ';
		}
	        fputc (c, stdout);
		while (pad--)
		    fputc (' ', stdout);
	    } else {
	        fputc (c, stdout);
	    }
	    help++;
	    first--;
	}
	fputc ('\n', stdout);
    }
}

static void print_help_and_exit ARGS1(int, exit_status)
{
    Parse_Args_Type *p;

    if (pgm == NULL) pgm = "lynx";

    fprintf (stdout, "USAGE: %s [options] [file]\n", pgm);
    fprintf (stdout, "Options are:\n");
    print_help_strings("",
"receive the arguments from stdin (enclose\n\
in double-quotes (\"-\") on VMS)");

    for (p = Arg_Table; p->name != 0; p++)
	print_help_strings(p->name, p->help_string);

    exit (exit_status);
}

/*
 * This function performs a string comparison on two strings a and b.  a is
 * assumed to be an ordinary null terminated string, but b may be terminated
 * by an '=', '+' or '-' character.  If terminated by '=', *c will be pointed
 * to the character following the '='.  If terminated by '+' or '-', *c will
 * be pointed to that character.  (+/- added for toggle processing - BL.)
 * If a and b match, it returns 1.  Otherwise 0 is returned.
 */
static int arg_eqs_parse ARGS3(
	char *,		a,
	char *,		b,
	char **,	c)
{
    *c = NULL;
    while (1) {
	if ((*a != *b)
	 || (*a == 0)
	 || (*b == 0)) {
	    if (*a == 0) {
		switch (*b) {
		case '=':
		    *c = b + 1;
		    return 1;
		case '-':	/* FALLTHRU */
		case '+':
		    *c = b;
		    return 1;
		case 0:
		    return 1;
		default:
		    return 0;
		}
	    } else {
		return 0;
	    }
	}
	a++;
	b++;
     }
}

#define is_true(s)  (*s == '1' || *s == '+' || !strcmp(s, "on"))
#define is_false(s) (*s == '0' || *s == '-' || !strcmp(s, "off"))

PRIVATE void parse_arg ARGS2(
	char **,	argv,
	int *,		i)
{
    Parse_Args_Type *p;
    char *arg_name;

    arg_name = argv[0];

    /*
     *	Check for a command line startfile. - FM
     */
    if (*arg_name != '-') {
	StrAllocCopy(startfile, arg_name);
	LYTrimHead(startfile);
	if (!strncasecomp(startfile, "lynxexec:", 9) ||
	    !strncasecomp(startfile, "lynxprog:", 9)) {
	    /*
	     *	The original implementations of these schemes expected
	     *	white space without hex escaping, and did not check
	     *	for hex escaping, so we'll continue to support that,
	     *	until that code is redone in conformance with SGML
	     *	principles.  - FM
	     */
	    HTUnEscapeSome(startfile, " \r\n\t");
	    convert_to_spaces(startfile, TRUE);
	}
	return;
    }

    /* lose the first '-' character */
    arg_name++;

    /*
     *	Skip any lone "-" arguments, because we've loaded
     *	the stdin input into an HTList structure for
     *	special handling. - FM
     */
    if (*arg_name == 0)
        return;

    p = Arg_Table;
    while (p->name != 0) {
#ifdef PARSE_DEBUG
	Parse_Args_Type *q = p;
#else
	ParseUnion *q = (ParseUnion *)(&(p->value));
#endif
	ParseFunc fun;
	char *next_arg = NULL;

	if ((p->name[0] != *arg_name)
	    || (0 == arg_eqs_parse (p->name, arg_name, &next_arg))) {
	    p++;
	    continue;
	}

	if ((p->type & NEED_NEXT_ARG) && (next_arg == 0)) {
	    next_arg = argv[1];
	    if ((i != 0) && (next_arg != 0))
	        (*i)++;
	}

	switch (p->type & ARG_TYPE_MASK) {
	case TOGGLE_ARG:	/* FALLTHRU */
	case SET_ARG:		/* FALLTHRU */
	case UNSET_ARG:
	     if (q->set_value != 0) {
		 if (next_arg == 0) {
		    switch (p->type & ARG_TYPE_MASK) {
		    case TOGGLE_ARG:
			 *(q->set_value) = !(*(q->set_value));
			 break;
		    case SET_ARG:
			 *(q->set_value) = TRUE;
			 break;
		    case UNSET_ARG:
			 *(q->set_value) = FALSE;
			 break;
		    }
		 } else if (is_true(next_arg)) {
		     *(q->set_value) = TRUE;
		 } else if (is_false(next_arg)) {
		     *(q->set_value) = FALSE;
		 }
		 /* deliberately ignore anything else - BL */
	     }
	     break;

	case FUNCTION_ARG:
	     fun = q->fun_value;
	     if (0 != fun) {
		 if (-1 == (*fun) (p, argv, next_arg)) {
		 }
	     }
	     break;

	case LYSTRING_ARG:
	     if ((q->str_value != 0) && (next_arg != 0))
	         StrAllocCopy(*(q->str_value), next_arg);
	     break;

	case INT_ARG:
	     if ((q->int_value != 0) && (next_arg != 0))
	         *(q->int_value) = atoi (next_arg);
	     break;

	case STRING_ARG:
	     if ((q->str_value != 0) && (next_arg != 0))
	        *(q->str_value) = next_arg;
	     break;

	case IGNORE_ARG:
	     break;
	}

	return;
    }

    if (pgm == 0) pgm = "LYNX";

    fprintf (stderr, "%s: Invalid Option: %s\n", pgm, argv[0]);
    print_help_and_exit (-1);
}

#ifndef VMS
PRIVATE void FatalProblem ARGS1(
	int,		sig)
{
    /*
     *	Ignore further interrupts. - mhc: 11/2/91
     */
#ifndef NOSIGHUP
    (void) signal(SIGHUP, SIG_IGN);
#endif /* NOSIGHUP */
    (void) signal (SIGTERM, SIG_IGN);
    (void) signal (SIGINT, SIG_IGN);
#ifndef __linux__
#ifndef DOSPATH
    (void) signal(SIGBUS, SIG_IGN);
#endif /* ! DOSPATH */
#endif /* !__linux__ */
    (void) signal(SIGSEGV, SIG_IGN);
    (void) signal(SIGILL, SIG_IGN);

    /*
     *	Flush all messages. - FM
     */
    fflush(stderr);
    fflush(stdout);

    /*
     *	Deal with curses, if on, and clean up. - FM
     */
    if (LYOutOfMemory && LYCursesON) {
	sleep(AlertSecs);
    }
    cleanup_sig(0);
#ifndef __linux__
#ifndef DOSPATH
    signal(SIGBUS, SIG_DFL);
#endif /* DOSPATH */
#endif /* !__linux__ */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);

    /*
     *	Issue appropriate messages and abort or exit. - FM
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
	 *  Exit and possibly dump core.
	 */
	if (LYNoCore) {
	    exit(-1);
	}
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
