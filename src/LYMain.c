/*
 * $LynxId: LYMain.c,v 1.282 2018/07/08 15:22:44 tom Exp $
 */
#include <HTUtils.h>
#include <HTTP.h>
#include <HTParse.h>
#include <HTAccess.h>
#include <HTList.h>
#include <HTFile.h>
#include <UCMap.h>
#include <UCDefs.h>
#include <HTInit.h>
#include <HTAlert.h>
#include <LYCurses.h>
#include <LYStyle.h>
#include <HTML.h>
#include <LYUtils.h>
#include <LYGlobalDefs.h>
#include <LYMail.h>
#include <LYOptions.h>
#include <LYSignal.h>
#include <LYGetFile.h>
#include <LYStrings.h>
#include <LYClean.h>
#include <LYCharSets.h>
#include <LYCharUtils.h>
#include <LYReadCFG.h>
#include <LYrcFile.h>
#include <LYKeymap.h>
#include <HTForms.h>
#include <LYList.h>
#include <LYJump.h>

#ifdef USE_SESSIONS
#include <LYSession.h>
#endif

#include <LYMainLoop.h>
#include <LYBookmark.h>
#include <LYCookie.h>
#include <LYPrettySrc.h>
#include <LYShowInfo.h>
#include <LYHistory.h>

#ifdef VMS
#include <HTFTP.h>
#endif /* !DECNET */

#ifdef __DJGPP__
#include <dos.h>
#include <dpmi.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/exceptn.h>
#endif /* __DJGPP__ */

#ifdef __EMX__
#include <io.h>
#endif

#if defined(LOCALE) && (!defined(HAVE_LIBINTL_H) || !defined(LC_ALL))
#undef gettext			/* Solaris locale.h prototypes gettext() */
#include <locale.h>
#ifndef HAVE_GETTEXT
#define gettext(s) s
#endif
#endif /* LOCALE */

#include <LYexit.h>
#include <LYLeaks.h>

/* ahhhhhhhhhh!! Global variables :-< */
#ifdef SOCKS
BOOLEAN socks_flag = TRUE;
#endif /* SOCKS */

#ifdef IGNORE_CTRL_C
BOOLEAN sigint = FALSE;
#endif /* IGNORE_CTRL_C */

#ifdef __DJGPP__
static char init_ctrl_break[1];
#endif /* __DJGPP__ */

#if USE_VMS_MAILER
char *mail_adrs = NULL;		/* the mask for a VMS mail transport */
#endif

#ifdef VMS
	       /* create FIXED 512 binaries */
BOOLEAN UseFixedRecords = USE_FIXED_RECORDS;
#endif /* VMS */

#ifndef VMS
static char *lynx_version_putenv_command = NULL;
char *list_format = NULL;	/* LONG_LIST formatting mask */
#endif /* !VMS */

char *ftp_format = NULL;	/* LONG_LIST formatting mask */

#ifdef SYSLOG_REQUESTED_URLS
char *syslog_txt = NULL;	/* syslog arb text for session */
BOOLEAN syslog_requested_urls = FALSE;
#endif

int cfg_bad_html = BAD_HTML_WARN;

#ifdef DIRED_SUPPORT
BOOLEAN lynx_edit_mode = FALSE;
BOOLEAN no_dired_support = FALSE;
HTList *tagged = NULL;
int LYAutoUncacheDirLists = 2;	/* default dired uncaching behavior */
int dir_list_order = ORDER_BY_NAME;
int dir_list_style = MIXED_STYLE;

#ifdef OK_OVERRIDE
BOOLEAN prev_lynx_edit_mode = FALSE;
#endif /* OK_OVERRIDE */

#ifdef OK_PERMIT
#ifdef NO_CHANGE_EXECUTE_PERMS
BOOLEAN no_change_exec_perms = TRUE;

#else
BOOLEAN no_change_exec_perms = FALSE;
#endif /* NO_CHANGE_EXECUTE_PERMS */
#endif /* OK_PERMIT */

#endif /* DIRED_SUPPORT */

	   /* Number of docs cached in memory */
int HTCacheSize = DEFAULT_CACHE_SIZE;

#if defined(VMS) && defined(VAXC) && !defined(__DECC)
	   /* Don't dump doc cache unless this size is exceeded */
int HTVirtualMemorySize = DEFAULT_VIRTUAL_MEMORY_SIZE;
#endif /* VMS && VAXC && !_DECC */

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
#ifndef NEVER_ALLOW_REMOTE_EXEC
BOOLEAN local_exec = LOCAL_EXECUTION_LINKS_ALWAYS_ON;

#else
BOOLEAN local_exec = FALSE;
#endif /* NEVER_ALLOW_REMOTE_EXEC */
BOOLEAN local_exec_on_local_files =
LOCAL_EXECUTION_LINKS_ON_BUT_NOT_REMOTE;
#endif /* EXEC_LINKS || EXEC_SCRIPTS */

#if defined(LYNXCGI_LINKS) && !defined(VMS)	/* WebSter Mods -jkt */
char *LYCgiDocumentRoot = NULL;	/* DOCUMENT_ROOT in the lynxcgi env */
#endif /* LYNXCGI_LINKS */

#ifdef TRACK_INTERNAL_LINKS
BOOLEAN track_internal_links = TRUE;

#else
BOOLEAN track_internal_links = FALSE;
#endif

BOOLEAN enable_scrollback = FALSE;

char empty_string[] =
{'\0'};

int display_lines;		/* number of lines in display */
int www_search_result = -1;

			       /* linked list of printers */
lynx_list_item_type *printers = NULL;

			    /* linked list of download options */
lynx_list_item_type *downloaders = NULL;

			    /* linked list of upload options */
#ifdef USE_EXTERNALS
lynx_list_item_type *externals = NULL;

			    /* linked list of external options */
#endif

lynx_list_item_type *uploaders = NULL;
int LYShowColor = SHOW_COLOR_UNKNOWN;	/* to show or not */
int LYrcShowColor = SHOW_COLOR_UNKNOWN;		/* ... last used */

#if !defined(NO_OPTION_FORMS) && !defined(NO_OPTION_MENU)
BOOLEAN LYUseFormsOptions = TRUE;	/* use forms-based options menu */
#endif

BOOLEAN LYGuessScheme = FALSE;
BOOLEAN LYJumpFileURL = FALSE;	/* always FALSE the first time */
BOOLEAN LYPermitURL = FALSE;
BOOLEAN LYRestricted = FALSE;	/* whether we have -anonymous option */
BOOLEAN LYShowCursor = SHOW_CURSOR;	/* to show or not to show */
BOOLEAN LYUnderlineLinks = UNDERLINE_LINKS;	/* Show the links underlined vs bold */
BOOLEAN LYUseDefShoCur = TRUE;	/* Command line -show_cursor toggle */
BOOLEAN LYUserSpecifiedURL = TRUE;	/* always TRUE  the first time */
BOOLEAN LYValidate = FALSE;
BOOLEAN LYforce_no_cache = FALSE;
BOOLEAN LYinternal_flag = FALSE;	/* override no-cache b/c internal link */
BOOLEAN LYoverride_no_cache = FALSE;	/*override no-cache b/c history etc */
BOOLEAN LYresubmit_posts = ALWAYS_RESUBMIT_POSTS;
BOOLEAN LYtrimBlankLines = TRUE;
BOOLEAN LYtrimInputFields = FALSE;
BOOLEAN LYxhtml_parsing = FALSE;
BOOLEAN bold_H1 = FALSE;
BOOLEAN bold_headers = FALSE;
BOOLEAN bold_name_anchors = FALSE;
BOOLEAN LYcase_sensitive = CASE_SENSITIVE_ALWAYS_ON;
BOOLEAN check_mail = CHECKMAIL;
BOOLEAN child_lynx = FALSE;
BOOLEAN dump_links_inline = FALSE;
BOOLEAN dump_links_only = FALSE;
BOOLEAN dump_output_immediately = FALSE;
BOOLEAN dump_to_stderr = FALSE;
BOOLEAN emacs_keys = EMACS_KEYS_ALWAYS_ON;
BOOLEAN error_logging = MAIL_SYSTEM_ERROR_LOGGING;
BOOLEAN goto_buffer = GOTOBUFFER;	/* TRUE if offering default goto URL */
BOOLEAN historical_comments = FALSE;
BOOLEAN html5_charsets = FALSE;
BOOLEAN is_www_index = FALSE;
BOOLEAN jump_buffer = JUMPBUFFER;	/* TRUE if offering default shortcut */
BOOLEAN lynx_mode = NORMAL_LYNX_MODE;
BOOLEAN minimal_comments = FALSE;
BOOLEAN number_fields_on_left = TRUE;
BOOLEAN number_links_on_left = TRUE;
BOOLEAN recent_sizechange = FALSE;	/* the window size changed recently? */
BOOLEAN soft_dquotes = FALSE;
BOOLEAN unique_urls = FALSE;
BOOLEAN use_underscore = SUBSTITUTE_UNDERSCORES;
BOOLEAN verbose_img = VERBOSE_IMAGES;	/* show filenames or not */
BOOLEAN vi_keys = VI_KEYS_ALWAYS_ON;
int keypad_mode = DEFAULT_KEYPAD_MODE;
int user_mode = NOVICE_MODE;

BOOLEAN telnet_ok = TRUE;

#ifndef DISABLE_NEWS
BOOLEAN news_ok = TRUE;
#endif
BOOLEAN rlogin_ok = TRUE;
BOOLEAN long_url_ok = FALSE;
BOOLEAN ftp_ok = TRUE;
BOOLEAN system_editor = FALSE;

BOOLEAN had_restrictions_default = FALSE;
BOOLEAN had_restrictions_all = FALSE;

BOOLEAN exec_frozen = FALSE;
BOOLEAN no_bookmark = FALSE;
BOOLEAN no_bookmark_exec = FALSE;
BOOLEAN no_chdir = FALSE;
BOOLEAN no_disk_save = FALSE;
BOOLEAN no_dotfiles = NO_DOT_FILES;
BOOLEAN no_download = FALSE;
BOOLEAN no_editor = FALSE;
BOOLEAN no_exec = FALSE;
BOOLEAN no_file_url = FALSE;
BOOLEAN no_goto = FALSE;
BOOLEAN no_goto_configinfo = FALSE;
BOOLEAN no_goto_cso = FALSE;
BOOLEAN no_goto_file = FALSE;
BOOLEAN no_goto_finger = FALSE;
BOOLEAN no_goto_ftp = FALSE;
BOOLEAN no_goto_gopher = FALSE;
BOOLEAN no_goto_http = FALSE;
BOOLEAN no_goto_https = FALSE;
BOOLEAN no_goto_lynxcgi = FALSE;
BOOLEAN no_goto_lynxexec = FALSE;
BOOLEAN no_goto_lynxprog = FALSE;
BOOLEAN no_goto_mailto = FALSE;
BOOLEAN no_goto_rlogin = FALSE;
BOOLEAN no_goto_telnet = FALSE;
BOOLEAN no_goto_tn3270 = FALSE;
BOOLEAN no_goto_wais = FALSE;
BOOLEAN no_inside_ftp = FALSE;
BOOLEAN no_inside_rlogin = FALSE;
BOOLEAN no_inside_telnet = FALSE;
BOOLEAN no_jump = FALSE;
BOOLEAN no_lynxcfg_info = FALSE;
BOOLEAN no_lynxcgi = FALSE;
BOOLEAN no_mail = FALSE;
BOOLEAN no_multibook = FALSE;
BOOLEAN no_option_save = FALSE;
BOOLEAN no_outside_ftp = FALSE;
BOOLEAN no_outside_rlogin = FALSE;
BOOLEAN no_outside_telnet = FALSE;
BOOLEAN no_print = FALSE;
BOOLEAN no_shell = FALSE;
BOOLEAN no_suspend = FALSE;
BOOLEAN no_telnet_port = FALSE;
BOOLEAN no_useragent = FALSE;

#ifndef DISABLE_FTP
BOOLEAN ftp_passive = FTP_PASSIVE;	/* TRUE if doing ftp in passive mode */
BOOLEAN ftp_local_passive;
HTList *broken_ftp_epsv = NULL;
HTList *broken_ftp_retr = NULL;
char *ftp_lasthost = NULL;
#endif

#ifndef DISABLE_NEWS
BOOLEAN no_goto_news = FALSE;
BOOLEAN no_goto_nntp = FALSE;
BOOLEAN no_goto_snews = FALSE;
BOOLEAN no_inside_news = FALSE;
BOOLEAN no_newspost = FALSE;
BOOLEAN no_outside_news = FALSE;
#endif

#ifdef USE_EXTERNALS
BOOLEAN no_externals = FALSE;
#endif

#ifndef NO_CONFIG_INFO
BOOLEAN no_lynxcfg_xinfo = FALSE;

#ifdef HAVE_CONFIG_H
BOOLEAN no_compileopts_info = FALSE;
#endif
#endif

BOOLEAN no_statusline = FALSE;
BOOLEAN no_filereferer = TRUE;
char LYRefererWithQuery = 'D';	/* 'D' for drop */
BOOLEAN local_host_only = FALSE;
BOOLEAN override_no_download = FALSE;
BOOLEAN show_dotfiles = FALSE;	/* From rcfile if no_dotfiles is false */
BOOLEAN LYforce_HTML_mode = FALSE;
BOOLEAN LYfind_leaks = TRUE;

#ifdef __DJGPP__
BOOLEAN watt_debug = FALSE;	/* WATT-32 debugging */
BOOLEAN dj_is_bash = FALSE;	/* Check for bash shell under DJGPP */
#endif /* __DJGPP__ */

#ifdef WIN_EX
BOOLEAN focus_window = FALSE;	/* 1998/10/05 (Mon) 17:18:42 */
char windows_drive[4];		/* 1998/01/13 (Tue) 21:13:24 */
#endif

#ifdef _WINDOWS
#define	TIMEOUT	180		/* 1998/03/30 (Mon) 14:50:44 */
int lynx_timeout = TIMEOUT;
CRITICAL_SECTION critSec_READ;	/* 1998/09/03 (Thu) 22:01:56 */
#endif /* _WINDOWS */

#if defined(WIN_EX)
BOOLEAN system_is_NT = FALSE;
#endif

BOOLEAN show_cfg = FALSE;

BOOLEAN no_table_center = FALSE;	/* 1998/10/09 (Fri) 15:12:49 */

#if USE_BLAT_MAILER
BOOLEAN mail_is_blat = TRUE;
BOOLEAN mail_is_altblat = USE_ALT_BLAT_MAILER;

#if USE_ALT_BLAT_MAILER
#define THIS_BLAT_MAIL ALTBLAT_MAIL
#define THAT_BLAT_MAIL BLAT_MAIL
#else
#define THIS_BLAT_MAIL BLAT_MAIL
#define THAT_BLAT_MAIL ALTBLAT_MAIL
#endif
#endif

#ifdef USE_BLINK
#  ifdef __EMX__
BOOLEAN term_blink_is_boldbg = TRUE;

#  else
BOOLEAN term_blink_is_boldbg = FALSE;

#  endif
#endif

BOOLEAN HEAD_request = FALSE;
BOOLEAN LYAcceptAllCookies = ACCEPT_ALL_COOKIES;	/* take all cookies? */
BOOLEAN LYCancelledFetch = FALSE;	/* TRUE if cancelled binary fetch */
BOOLEAN LYCollapseBRs = COLLAPSE_BR_TAGS;	/* Collapse serial BRs? */
BOOLEAN LYDefaultRawMode;
BOOLEAN LYListNewsDates = LIST_NEWS_DATES;
BOOLEAN LYListNewsNumbers = LIST_NEWS_NUMBERS;
BOOLEAN LYMBMBlocked = BLOCK_MULTI_BOOKMARKS;
BOOLEAN LYNewsPosting = NEWS_POSTING;	/* News posting supported? */
BOOLEAN LYNoFromHeader = TRUE;	/* Never send From header?         */
BOOLEAN LYNoRefererForThis = FALSE;	/* No Referer header for this URL? */
BOOLEAN LYNoRefererHeader = FALSE;	/* Never send Referer header?     */
BOOLEAN LYRawMode;
BOOLEAN LYSelectPopups = USE_SELECT_POPUPS;
BOOLEAN LYSendUserAgent = SEND_USERAGENT;	/* send Lynx User-Agent header? */
BOOLEAN LYSetCookies = SET_COOKIES;	/* Process Set-Cookie headers? */
BOOLEAN LYUseDefSelPop = TRUE;	/* Command line -popup toggle */
BOOLEAN LYUseDefaultRawMode = TRUE;
BOOLEAN LYUseMouse = FALSE;
BOOLEAN LYisConfiguredForX = FALSE;
BOOLEAN UCForce8bitTOUPPER = FALSE;	/* override locale for case-conversion? */
BOOLEAN UCSaveBookmarksInUnicode = FALSE;
BOOLEAN bookmark_start = FALSE;
BOOLEAN check_realm = FALSE;	/* Restrict to the starting realm? */
BOOLEAN clickable_images = MAKE_LINKS_FOR_ALL_IMAGES;
BOOLEAN crawl = FALSE;		/* Do crawl? */
BOOLEAN keep_mime_headers = FALSE;	/* Include mime headers with source dump */
BOOLEAN more_text = FALSE;	/* is there more text to display? */
BOOLEAN more_links = FALSE;	/* Links beyond a displayed page with no links? */
BOOLEAN no_list = FALSE;
BOOLEAN no_margins = FALSE;
BOOLEAN no_pause = FALSE;
BOOLEAN no_title = FALSE;
BOOLEAN no_url_redirection = FALSE;	/* Don't follow URL redirections */
BOOLEAN pseudo_inline_alts = MAKE_PSEUDO_ALTS_FOR_INLINES;
BOOLEAN scan_for_buried_news_references = TRUE;
BOOLEAN startfile_ok = FALSE;
static BOOLEAN startfile_stdin = FALSE;
BOOLEAN traversal = FALSE;	/* Do traversals? */

char *BookmarkPage = NULL;	/* the name of the current bookmark page */
char *LYCookieAcceptDomains = NULL;	/* domains to accept all cookies */
char *LYCookieLooseCheckDomains = NULL;		/* check loosely   */
char *LYCookieQueryCheckDomains = NULL;		/* check w/a query */
char *LYCookieRejectDomains = NULL;	/* domains to reject all cookies */
char *LYCookieSAcceptDomains = NULL;	/* domains to accept all cookies */
char *LYCookieSLooseCheckDomains = NULL;	/* check loosely   */
char *LYCookieSQueryCheckDomains = NULL;	/* check w/a query */
char *LYCookieSRejectDomains = NULL;	/* domains to reject all cookies */
char *LYCookieSStrictCheckDomains = NULL;	/* check strictly  */
char *LYCookieStrictCheckDomains = NULL;	/* check strictly  */
char *LYHostName = NULL;	/* treat as a local host name */
char *LYLocalDomain = NULL;	/* treat as a local domain tail */
char *LYUserAgent = NULL;	/* Lynx User-Agent header          */
char *LYUserAgentDefault = NULL;	/* Lynx default User-Agent header  */
char *LynxHome = NULL;		/* the default Home HREF. */
char *LynxSigFile = NULL;	/* Signature file, in or off home */
char *UCAssume_MIMEcharset = NULL;
char *URLDomainPrefixes = NULL;
char *URLDomainSuffixes = NULL;
char *anonftp_password = NULL;	/* anonymous ftp password (default: email) */
char *authentication_info[2] =
{NULL, NULL};			/* Id:Password for protected documents */
char *bookmark_page = NULL;	/* the name of the default bookmark page */
char *editor = NULL;		/* the name of the current editor */
char *form_get_data = NULL;	/* User data for get form */
char *form_post_data = NULL;	/* User data for post form */
char *global_extension_map = NULL;	/* global mime.types */
char *global_type_map = NULL;	/* global mailcap */
char *helpfile = NULL;		/* the main help file */
char *helpfilepath = NULL;	/* the path to the help file set */
char *homepage = NULL;		/* home page or main screen */
char *http_error_file = NULL;	/* Place HTTP status code in this file */
char *indexfile = NULL;		/* an index file if there is one */
char *jumpfile = NULL;		/* the name of the default jumps file */
char *jumpprompt = NULL;	/* the default jumps prompt */
char *language = NULL;		/* preferred language */
char *lynx_cfg_file = NULL;	/* location of active lynx.cfg */
char *lynx_cmd_logfile;		/* file to write keystroke commands, if any */
char *lynx_cmd_script;		/* file to read keystroke commands, if any */
char *lynx_save_space = NULL;	/* The prefix for save to disk paths */
char *lynx_temp_space = NULL;	/* The prefix for temporary file paths */
char *lynxjumpfile = NULL;	/* the current jump file URL */
char *lynxlinksfile = NULL;	/* the current visited links file URL */
char *lynxlistfile = NULL;	/* the current list file URL */
char *original_dir = NULL;	/* the original directory */
char *personal_extension_map = NULL;	/* .mime.types */
char *personal_mail_address = NULL;	/* the user's mail address */
char *personal_mail_name = NULL;	/* the user's personal name mail */
char *personal_type_map = NULL;	/* .mailcap */
char *pref_charset = NULL;	/* preferred character set */
char *proxyauth_info[2] =
{NULL, NULL};			/* Id:Password for protected proxy servers */

#ifdef USE_SESSIONS
BOOLEAN LYAutoSession = FALSE;	/* enable/disable auto saving/restoring of */

				/* session */
char *LYSessionFile = NULL;	/* the session file from lynx.cfg */
char *session_file = NULL;	/* the current session file */
char *sessionin_file = NULL;	/* only resume session from this file */
char *sessionout_file = NULL;	/* only save session to this file */
short session_limit = 250;	/* maximal number of entries saved per */

				/* session file, rest will be ignored */
#endif /* USE_SESSIONS */
char *startfile = NULL;		/* the first file */
char *startrealm = NULL;	/* the startfile realm */
char *system_mail = NULL;	/* The path for sending mail */
char *system_mail_flags = NULL;	/* Flags for sending mail */
char *x_display = NULL;		/* display environment variable */

HistInfo *history;
int nhist = 0;			/* number of used history entries */
int size_history;		/* number of allocated history entries */

LinkInfo links[MAXLINKS];

BOOLEAN nomore = FALSE;		/* display -more- string in statusline messages */
int AlertSecs;			/* time-delay for HTAlert() messages   */
int DelaySecs;			/* time-delay for HTProgress messages */
int InfoSecs;			/* time-delay for Information messages */
int LYMultiBookmarks = MULTI_BOOKMARK_SUPPORT;
int LYStatusLine = -1;		/* Line for statusline() if > -1 */
int LYcols = DFT_COLS;
int LYlines = DFT_ROWS;
int MessageSecs;		/* time-delay for important Messages   */
int ReplaySecs;			/* time-delay for command-scripts */
int crawl_count = 0;		/* Starting number for lnk#.dat files in crawls */
int dump_output_width = 0;
int dump_server_status = 0;
int lynx_temp_subspace = 0;	/* > 0 if we made temp-directory */
int max_cookies_domain = 50;
int max_cookies_global = 500;
int max_cookies_buffer = 4096;
int max_uri_size = 8192;
int nlinks = 0;			/* number of links in memory */
int outgoing_mail_charset = -1;	/* translate mail to this charset */

#ifndef DISABLE_BIBP
BOOLEAN BibP_bibhost_available = FALSE;		/* until check succeeds  */
BOOLEAN BibP_bibhost_checked = FALSE;	/*  until LYCheckBibHost   */
BOOLEAN no_goto_bibp = FALSE;
char *BibP_bibhost = NULL;	/* local server for bibp: links  */
char *BibP_globalserver = NULL;	/* global server for bibp: links */
#endif

#ifdef USE_PERSISTENT_COOKIES
BOOLEAN persistent_cookies = FALSE;	/* disabled by default! */
char *LYCookieFile = NULL;	/* cookie read file */
char *LYCookieSaveFile = NULL;	/* cookie save file */
#endif /* USE_PERSISTENT_COOKIES */

#ifdef EXP_NESTED_TABLES
BOOLEAN nested_tables =
#if defined(USE_COLOR_STYLE)
TRUE
#else
FALSE				/* see 2001-08-15  */
#endif
 ;
#endif

BOOLEAN LYShowTransferRate = TRUE;
int LYTransferRate = rateKB;
int LYAcceptEncoding = encodingALL;
int LYAcceptMedia = mediaOpt1;
int LYContentType = contentTEXT;
const char *ContentTypes[] =
{
    STR_BINARY,
    STR_PLAINTEXT,
    STR_HTML
};
char *LYTransferName = NULL;

char *XLoadImageCommand = NULL;	/* Default image viewer for X */
BOOLEAN LYNoISMAPifUSEMAP = FALSE;	/* Omit ISMAP link if MAP present? */
int LYHiddenLinks = HIDDENLINKS_SEPARATE;	/* Show hidden links? */

char *SSL_cert_file = NULL;	/* Default CA CERT file */
char *SSL_client_cert_file = NULL;
char *SSL_client_key_file = NULL;

int HTprotocolLevel = HTTP_1_0;

int Old_DTD = NO;
static BOOLEAN DTD_recovery = NO;

#ifndef NO_LYNX_TRACE
FILE *LYTraceLogFP = NULL;	/* Pointer for TRACE log  */
#endif
char *LYTraceLogPath = NULL;	/* Path for TRACE log      */
BOOLEAN LYUseTraceLog = USE_TRACE_LOG;	/* Use a TRACE log?        */

#ifdef LY_FIND_LEAKS
char LYLeaksPath[LY_MAXPATH];
#endif

BOOLEAN LYSeekFragMAPinCur = TRUE;
BOOLEAN LYSeekFragAREAinCur = TRUE;
BOOLEAN LYStripDotDotURLs = TRUE;	/* Try to fix ../ in some URLs? */
BOOLEAN LYForceSSLCookiesSecure = FALSE;
BOOLEAN LYNoCc = FALSE;
BOOLEAN LYPreparsedSource = FALSE;	/* Show source as preparsed? */
BOOLEAN LYPrependBaseToSource = TRUE;
BOOLEAN LYPrependCharsetToSource = TRUE;
BOOLEAN LYQuitDefaultYes = QUIT_DEFAULT_YES;
BOOLEAN dont_wrap_pre = FALSE;

int cookie_noprompt;

#ifdef USE_SSL
int ssl_noprompt = FORCE_PROMPT_DFT;
#endif
BOOLEAN conv_jisx0201kana = TRUE;
BOOLEAN wait_viewer_termination = FALSE;

int connect_timeout = 18000; /*=180000*0.1 - used in HTDoConnect.*/
int reading_timeout = 18000; /*=180000*0.1 - used in HTDoConnect.*/

#ifdef USE_JUSTIFY_ELTS
BOOLEAN ok_justify = FALSE;
int justify_max_void_percent = 35;
#endif

#ifdef USE_LOCALE_CHARSET
BOOLEAN LYLocaleCharset = FALSE;
#endif
BOOLEAN assumed_charset = FALSE;

#ifndef NO_DUMP_WITH_BACKSPACES
BOOLEAN with_backspaces = FALSE;
#endif

#if defined(PDCURSES) && defined(PDC_BUILD) && PDC_BUILD >= 2401
int scrsize_x = 0;
int scrsize_y = 0;
#endif

BOOLEAN force_empty_hrefless_a = FALSE;

#ifdef TEXTFIELDS_MAY_NEED_ACTIVATION
BOOL textfields_need_activation = FALSE;
BOOLEAN textfields_activation_option = FALSE;
#endif

BOOLEAN textfield_prompt_at_left_edge = FALSE;

#ifdef MARK_HIDDEN_LINKS
char *hidden_link_marker = NULL;
#endif

#ifdef DISP_PARTIAL
BOOLEAN display_partial_flag = TRUE;	/* Display document during download */
BOOLEAN debug_display_partial = FALSE;	/* Show with MessageSecs delay */
int partial_threshold = -1;	/* # of lines to be d/l'ed until we repaint */
#endif

BOOLEAN LYNonRestartingSIGWINCH = FALSE;
BOOLEAN LYReuseTempfiles = FALSE;
BOOLEAN LYUseBuiltinSuffixes = TRUE;

int LYNoZapKey = 0;		/* 0: off (do z checking), 1: full, 2: initially */

#ifndef DISABLE_NEWS
#include <HTNews.h>
#endif

BOOLEAN FileInitAlreadyDone = FALSE;

#ifdef USE_PROGRAM_DIR
char *program_dir = NULL;
#endif

static BOOLEAN stack_dump = FALSE;
static char *terminal = NULL;
static const char *pgm;
static BOOLEAN no_numbers = FALSE;
static BOOLEAN number_links = FALSE;
static BOOLEAN number_fields = FALSE;
static BOOLEAN LYPrependBase = FALSE;
static HTList *LYStdinArgs = NULL;
HTList *positionable_editor = NULL;

#ifndef EXTENDED_OPTION_LOGIC
/* if set then '--' will be recognized as the end of options */
#define EXTENDED_OPTION_LOGIC 1
#endif

#ifndef EXTENDED_STARTFILE_RECALL
/* if set then additional non-option args (before the last one) will be
   made available for 'g'oto recall - kw */
#define EXTENDED_STARTFILE_RECALL 1
#endif

#if EXTENDED_STARTFILE_RECALL
static char *nonoption = 0;
#endif

#ifndef OPTNAME_ALLOW_DASHES
/* if set, then will allow dashes and underscores to be used interchangeable
   in commandline option's names - VH */
#define OPTNAME_ALLOW_DASHES 1
#endif

static BOOL parse_arg(char **arg, unsigned mask, int *countp);
static void print_help_and_exit(int exit_status) GCC_NORETURN;
static void print_help_strings(const char *name,
			       const char *help,
			       const char *value,
			       int option);

#ifndef VMS
BOOLEAN LYNoCore = NO_FORCED_CORE_DUMP;
BOOLEAN restore_sigpipe_for_children = FALSE;
static void FatalProblem(int sig);
#endif /* !VMS */

#if defined(USE_COLOR_STYLE)
int LYuse_color_style = TRUE;
char *lynx_lss_file = NULL;	/* from config-file, etc. */
static char *lynx_lss_file2 = NULL;	/* from command-line options */
const char *default_color_styles = "\
lynx.lss;\
blue-background.lss;\
bright-blue.lss;\
midnight.lss;\
mild-colors.lss;\
opaque.lss\
";
#endif

#ifdef USE_DEFAULT_COLORS
BOOLEAN LYuse_default_colors = TRUE;
#endif

#ifdef __DJGPP__
static void LY_set_ctrl_break(int setting)
{
    (void) signal(SIGINT, (setting ? SIG_DFL : SIG_IGN));
    setcbrk(setting);
}

static int LY_get_ctrl_break(void)
{
    __dpmi_regs regs;

    regs.h.ah = 0x33;
    regs.h.al = 0x00;
    __dpmi_int(0x21, &regs);
    return ((int) regs.h.dl);
}

static void reset_break(void)
{
    LY_set_ctrl_break(init_ctrl_break[0]);
}
#endif /* __DJGPP__ */

#if defined(WIN_EX)
static int is_windows_nt(void)
{
    DWORD version;

    version = GetVersion();
    if ((version & 0x80000000) == 0)
	return 1;
    else
	return 0;
}
#endif

#ifdef LY_FIND_LEAKS
static void free_lynx_globals(void)
{
    int i;

#if defined(USE_COLOR_STYLE)
    clear_lss_list();
#endif
    FREE(ftp_format);
#ifndef VMS
    FREE(list_format);
#ifdef LYNXCGI_LINKS		/* WebSter Mods -jkt */
    FREE(LYCgiDocumentRoot);
#endif /* LYNXCGI_LINKS */
    free_lynx_cfg();
#endif /* !VMS */

#ifdef SYSLOG_REQUESTED_URLS
    FREE(syslog_txt);
#endif

#ifdef VMS
    Define_VMSLogical("LYNX_VERSION", "");
#else
    (void) putenv("LYNX_VERSION=" LYNX_VERSION);
#endif /* VMS */
#ifndef VMS
    FREE(lynx_version_putenv_command);
#endif

#if USE_VMS_MAILER
    FREE(mail_adrs);
#endif

    FREE(LynxHome);
    FREE(history);
    FREE(homepage);
    FREE(original_dir);
    FREE(startfile);
    FREE(helpfile);
    FREE(helpfilepath);
    FREE(jumpprompt);
#ifdef JUMPFILE
    FREE(jumpfile);
#endif /* JUMPFILE */
    FREE(indexfile);
    FREE(x_display);
    FREE(global_type_map);
    FREE(personal_type_map);
    FREE(global_extension_map);
    FREE(personal_extension_map);
    FREE(language);
    FREE(pref_charset);
    FREE(LynxSigFile);
    FREE(system_mail);
    FREE(system_mail_flags);
#ifndef DISABLE_BIBP
    FREE(BibP_bibhost);
    FREE(BibP_globalserver);
#endif
#ifdef USE_PERSISTENT_COOKIES
    FREE(LYCookieFile);
    FREE(LYCookieSaveFile);
#endif
    FREE(LYCookieAcceptDomains);
    FREE(LYCookieRejectDomains);
    FREE(LYCookieLooseCheckDomains);
    FREE(LYCookieStrictCheckDomains);
    FREE(LYCookieQueryCheckDomains);
    FREE(LYUserAgent);
    FREE(LYUserAgentDefault);
    FREE(LYHostName);
    FREE(LYLocalDomain);
    FREE(lynx_save_space);
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
#ifndef DISABLE_FTP
    FREE(ftp_lasthost);
    LYFreeStringList(broken_ftp_epsv);
    LYFreeStringList(broken_ftp_retr);
#endif
    FREE(startrealm);
    FREE(personal_mail_address);
    FREE(personal_mail_name);
    FREE(anonftp_password);
    FREE(URLDomainPrefixes);
    FREE(URLDomainSuffixes);
    FREE(XLoadImageCommand);
    FREE(lynx_temp_space);
    FREE(LYTransferName);
    FREE(LYTraceLogPath);
    FREE(lynx_cfg_file);
    FREE(SSL_cert_file);
    FREE(SSL_client_cert_file);
    FREE(SSL_client_key_file);
#if defined(USE_COLOR_STYLE)
    FREE(lynx_lss_file2);
    FREE(lynx_lss_file);
#endif
    FREE(UCAssume_MIMEcharset);
    LYUIPages_free();
    LYFreeHilites(0, nlinks);
    nlinks = 0;
    LYFreeStringList(LYcommandList());
    HTInitProgramPaths(FALSE);
#if EXTENDED_STARTFILE_RECALL
    FREE(nonoption);
#endif
    LYFreeStringList(positionable_editor);

    return;
}
#endif /* LY_FIND_LEAKS */

/*
 * This function frees the LYStdinArgs list.  - FM
 */
static void LYStdinArgs_free(void)
{
    LYFreeStringList(LYStdinArgs);
    LYStdinArgs = NULL;
}

void reset_signals(void)
{
#ifndef NOSIGHUP
    (void) signal(SIGHUP, SIG_DFL);
#endif /* NOSIGHUP */
    (void) signal(SIGTERM, SIG_DFL);
#ifndef VMS
    (void) signal(SIGINT, SIG_DFL);
#endif /* !VMS */
#ifdef SIGTSTP
    if (no_suspend)
	(void) signal(SIGTSTP, SIG_DFL);
#endif /* SIGTSTP */
}

void exit_immediately(int code)
{
    reset_signals();
    exit(code);
}

#ifdef  EBCDIC
static void FixCharacters(void)
{
    int c;
    int work1[256], work2[256];

    for (c = 0; c < 256; c++) {
	work1[c] = keymap[c + 1];
	work2[c] = key_override[c + 1];
    }
    for (c = 0; c < 256; c++) {
	keymap[IBM1047[c] + 1] = work1[c];
	key_override[IBM1047[c] + 1] = work2[c];
    }
}
#endif /* EBCDIC */

static BOOL GetStdin(char **buf,
		     int marker)
{
    if (LYSafeGets(buf, stdin) != 0
	&& (!marker || StrNCmp(*buf, "---", 3) != 0)) {
	LYTrimTrailing(*buf);
	CTRACE((tfp, "...data: %s\n", *buf));
	return TRUE;
    }
    CTRACE((tfp, "...mark: %s\n", *buf ? *buf : ""));
    return FALSE;
}

#ifdef WIN32
static BOOL cleanup_win32(DWORD fdwCtrlType)
{
    switch (fdwCtrlType) {
    case CTRL_CLOSE_EVENT:
	cleanup_sig(-1);
	return TRUE;
    default:
	return FALSE;
    }
}
#endif

/*
 * Append the SSL version to lynx version or user-agent string.
 */
#ifdef USE_SSL
static void append_ssl_version(char **target,
			       const char *separator)
{
    char SSLLibraryVersion[256];
    char *SSLcp;

    HTSprintf(target, " SSL-MM%s1.4.1", separator);

#undef LYNX_SSL_VERSION

#if defined(SSLEAY_VERSION)
#define LYNX_SSL_VERSION SSLeay_version(SSLEAY_VERSION)
#elif defined(OPENSSL_VERSION_TEXT)
#define LYNX_SSL_VERSION OPENSSL_VERSION_TEXT
#elif defined(GNUTLS_VERSION)
#define LYNX_SSL_VERSION "GNUTLS " GNUTLS_VERSION " "
#endif

#ifdef LYNX_SSL_VERSION
    if (*separator == ' ')
	StrAllocCat(*target, ",");
    LYStrNCpy(SSLLibraryVersion, LYNX_SSL_VERSION, sizeof(SSLLibraryVersion) - 1);
    if ((SSLcp = StrChr(SSLLibraryVersion, ' ')) != NULL) {
	*SSLcp++ = *separator;
	if ((SSLcp = StrChr(SSLcp, ' ')) != NULL) {
	    *SSLcp = '\0';
	    StrAllocCat(*target, " ");
	    StrAllocCat(*target, SSLLibraryVersion);
	}
    }
#endif /* LYNX_SSL_VERSION */
}
#endif /* USE_SSL */

/* Set the text message domain.  */
void LYSetTextDomain(void)
{
#if defined(HAVE_LIBINTL_H) || defined(HAVE_LIBGETTEXT_H)
    const char *cp;

    if ((cp = LYGetEnv("LYNX_LOCALEDIR")) == 0) {
#ifdef USE_PROGRAM_DIR
	char *localedir = NULL;

	HTSprintf0(&localedir, "%s\\locale", program_dir);
	cp = localedir;
#else
	cp = LOCALEDIR;
#endif
    }
    bindtextdomain(NLS_TEXTDOMAIN, cp);
    textdomain(NLS_TEXTDOMAIN);
#endif
}

static void SetLocale(void)
{
#ifdef LOCALE
    /*
     * LOCALE support for international characters.
     */
    setlocale(LC_ALL, "");
#endif /* LOCALE */
    LYSetTextDomain();
}

/*
 * Wow!  Someone wants to start up Lynx.
 */
int main(int argc,
	 char **argv)
{
    int i;			/* indexing variable */
    int status = 0;		/* exit status */
    char *temp = NULL;
    const char *ccp;
    char *cp;
    FILE *fp;
    struct stat dir_info;
    char filename[LY_MAXPATH];
    BOOL LYGetStdinArgs = FALSE;

#ifdef _WINDOWS
    WSADATA WSAData;
#endif /* _WINDOWS */

    /*
     * Just in case someone has the idea to install lynx set-uid, let's try
     * to discourage it.
     */
#if defined(GETUID) && defined(SETUID)
    setuid(getuid());
#endif

#ifdef LY_FIND_LEAKS
    /*
     * Register the final function to be executed when being exited.  Will
     * display memory leaks if the -find-leaks option is used.  This should
     * be the first call to atexit() for leak-checking, which ensures that 
     * all of the other functions will be invoked before LYLeaks().
     */
    atexit(LYLeaks);
    /*
     * Register the function which will free our allocated globals.
     */
    atexit(free_lynx_globals);

    LYAddPathToHome(LYLeaksPath, (size_t) LY_MAXPATH, LEAKAGE_SINK);
#endif /* LY_FIND_LEAKS */

#ifdef    NOT_ASCII
    FixCharacters();
#endif /* NOT_ASCII */

#ifndef DISABLE_FTP
    /* malloc a sizeof(char) so 1st strcmp() won't dump in HTLoadFile() */
    ftp_lasthost = typecalloc(char);
#endif

    LYinitEditmap();
    LYinitKeymap();
#ifdef USE_CHARSET_CHOICE
    memset((char *) charset_subsets, 0, sizeof(charset_subset_t) * MAXCHARSETS);
#endif

#ifdef _WINDOWS
    {
	int err;
	WORD wVerReq;

	wVerReq = MAKEWORD(1, 1);

	err = WSAStartup(wVerReq, &WSAData);
	if (err != 0) {
	    puts(gettext("No Winsock found, sorry."));
	    sleep(5);
	    return 1;
	}
    }

    /* 1998/09/03 (Thu) 22:02:32 */
    InitializeCriticalSection(&critSec_READ);

#endif /* _WINDOWS */

#if defined(WIN_EX)
    /* 1997/10/19 (Sun) 21:40:54 */
    system_is_NT = (BOOL) is_windows_nt();

    /* 1998/01/13 (Tue) 21:13:47 */
    GetWindowsDirectory(filename, sizeof filename);
    windows_drive[0] = filename[0];
    windows_drive[1] = filename[1];
    windows_drive[2] = '\0';
#endif

#ifdef __DJGPP__
    if (LY_get_ctrl_break() == 0) {
	LY_set_ctrl_break(TRUE);
	init_ctrl_break[0] = 0;
    } else {
	init_ctrl_break[0] = 1;
    }
    __djgpp_set_sigquit_key(0x082D);	/* Bind ALT-X to SIGQUIT */
    signal(SIGQUIT, cleanup_sig);
    atexit(reset_break);

    if (((ccp = LYGetEnv("SHELL")) != NULL)
	&& (strstr(LYPathLeaf(ccp), "sh") != NULL))
	dj_is_bash = TRUE;
#endif /* __DJGPP__ */

    /*
     * To prevent corrupting binary data on DOS, MS-WINDOWS or OS/2
     * we open files and stdout in BINARY mode by default.
     * Where necessary we should open and (close!) TEXT mode.
     * (use LYNewTxtFile/LYAppendToTxtFile to open text files for writing)
     */
    SetDefaultMode(O_BINARY);
    SetOutputMode(O_BINARY);

#ifdef DOSPATH
    if (LYGetEnv("TERM") == NULL)
	putenv("TERM=vt100");
#endif

    LYShowColor = (SHOW_COLOR ? SHOW_COLOR_ON : SHOW_COLOR_OFF);
    /*
     * Set up the argument list.
     */
    pgm = argv[0];
    cp = NULL;
#ifdef USE_PROGRAM_DIR
    StrAllocCopy(program_dir, pgm);
    if ((cp = strrchr(program_dir, '\\')) != NULL) {
	*cp = '\0';
    } else {
	FREE(program_dir);
	StrAllocCopy(program_dir, ".");
    }
#endif
    if ((cp = LYLastPathSep(pgm)) != NULL) {
	pgm = cp + 1;
    }

    /*
     * Set up trace, the anonymous account defaults, validate restrictions,
     * and/or the nosocks flag, if requested, and an alternate configuration
     * file, if specified, NOW.  Also, if we only want the help menu, output
     * that and exit.  - FM
     */
#ifndef NO_LYNX_TRACE
    if (LYGetEnv("LYNX_TRACE") != 0) {
	WWW_TraceFlag = TRUE;
    }
#endif

    /*
     * Set up the TRACE log path, and logging if appropriate.  - FM
     */
    if ((ccp = LYGetEnv("LYNX_TRACE_FILE")) == 0)
	ccp = FNAME_LYNX_TRACE;
    LYTraceLogPath = typeMallocn(char, LY_MAXPATH);

    LYAddPathToHome(LYTraceLogPath, (size_t) LY_MAXPATH, ccp);

    /*
     * Act on -version, -trace and -trace-mask NOW.
     */
    for (i = 1; i < argc; i++) {
	parse_arg(&argv[i], 1, &i);
    }
    LYOpenTraceLog();

    SetLocale();

    /*
     * Initialize our startup and global variables.
     */
#ifdef ULTRIX
    /*
     * Need this for Ultrix.
     */
    terminal = LYGetEnv("TERM");
    if ((terminal == NULL) || !strncasecomp(terminal, "xterm", 5))
	terminal = "vt100";
#endif /* ULTRIX */
    /*
     * Zero the links and history struct arrays.
     */
    memset((void *) links, 0, sizeof(LinkInfo) * MAXLINKS);
    LYAllocHistory(8);
    /*
     * Zero the MultiBookmark arrays.
     */
    memset((void *) MBM_A_subbookmark, 0, sizeof(char *) * (MBM_V_MAXFILES + 1));
    memset((void *) MBM_A_subdescript, 0, sizeof(char *) * (MBM_V_MAXFILES + 1));

#ifndef VMS
    StrAllocCopy(list_format, LIST_FORMAT);
    StrAllocCopy(ftp_format, FTP_FORMAT);
#endif /* !VMS */

    AlertSecs = SECS2Secs(ALERTSECS);
    DelaySecs = SECS2Secs(DEBUGSECS);
    InfoSecs = SECS2Secs(INFOSECS);
    MessageSecs = SECS2Secs(MESSAGESECS);
    ReplaySecs = SECS2Secs(REPLAYSECS);

    StrAllocCopy(LYTransferName, "KiB");
    StrAllocCopy(helpfile, HELPFILE);
    StrAllocCopy(startfile, STARTFILE);
    LYEscapeStartfile(&startfile);
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
#ifdef USE_SSL
    append_ssl_version(&LYUserAgent, "/");
#endif /* USE_SSL */
    StrAllocCopy(LYUserAgentDefault, LYUserAgent);

#ifdef VMS
    Define_VMSLogical("LYNX_VERSION", LYNX_VERSION);
#else
    StrAllocCopy(lynx_version_putenv_command, "LYNX_VERSION=");
    StrAllocCat(lynx_version_putenv_command, LYNX_VERSION);
    (void) putenv(lynx_version_putenv_command);
    /* Note: you must not free the data passed to 'putenv()' until you give it
     * a new value for that variable.
     */
#endif /* VMS */

    if ((ccp = LYGetEnv("LYNX_TEMP_SPACE")) != NULL)
	StrAllocCopy(lynx_temp_space, ccp);
#if defined (UNIX) || defined (__DJGPP__)
    else if ((ccp = LYGetEnv("TMPDIR")) != NULL)
	StrAllocCopy(lynx_temp_space, ccp);
#endif
#if defined (DOSPATH) || defined (__EMX__)
    else if ((ccp = LYGetEnv("TEMP")) != NULL)
	StrAllocCopy(lynx_temp_space, ccp);
    else if ((ccp = LYGetEnv("TMP")) != NULL)
	StrAllocCopy(lynx_temp_space, ccp);
#endif
    else {
#if defined(USE_PROGRAM_DIR)
	StrAllocCopy(lynx_temp_space, program_dir);
#elif defined(TEMP_SPACE)
	StrAllocCopy(lynx_temp_space, TEMP_SPACE);
#else
	puts(gettext("You MUST define a valid TMP or TEMP area!"));
	exit_immediately(EXIT_FAILURE);
#endif
    }

#ifdef WIN_EX			/* for Windows 2000 ... 1999/08/23 (Mon) 08:24:35 */
    if (access(lynx_temp_space, 0) != 0)
#endif
	LYTildeExpand(&lynx_temp_space, TRUE);

    if ((cp = strstr(lynx_temp_space, "$USER")) != NULL) {
	char *cp1;

	if ((cp1 = LYGetEnv("USER")) != NULL) {
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
    if (StrChr(lynx_temp_space, '/') != NULL) {
	if (strlen(lynx_temp_space) == 1) {
	    StrAllocCopy(lynx_temp_space, "sys$scratch:");
	} else {
	    LYAddPathSep(&lynx_temp_space);
	    StrAllocCopy(temp, HTVMS_name("", lynx_temp_space));
	    StrAllocCopy(lynx_temp_space, temp);
	    FREE(temp);
	}
    }
    if (StrChr(lynx_temp_space, ':') == NULL &&
	StrChr(lynx_temp_space, ']') == NULL) {
	StrAllocCat(lynx_temp_space, ":");
    }
#else
    LYAddPathSep(&lynx_temp_space);
    StrAllocCopy(lynx_temp_space, HTSYS_name(lynx_temp_space));
#endif /* VMS */

    if ((HTStat(lynx_temp_space, &dir_info) < 0
#if defined(MULTI_USER_UNIX)
	 && mkdir(lynx_temp_space, 0700) < 0
#endif
	)
	|| !S_ISDIR(dir_info.st_mode)) {
	fprintf(stderr, "%s: %s\n",
		lynx_temp_space,
		gettext("No such directory"));
	exit_immediately(EXIT_FAILURE);
    }
#if USE_VMS_MAILER
#ifndef MAIL_ADRS
#define MAIL_ADRS "\"IN%%\"\"%s\"\"\""
#endif
    StrAllocCopy(mail_adrs, MAIL_ADRS);
#endif

#ifdef LYNX_HOST_NAME
    StrAllocCopy(LYHostName, LYNX_HOST_NAME);
#else
    StrAllocCopy(LYHostName, HTHostName());
#endif /* LYNX_HOST_NAME */

    StrAllocCopy(LYLocalDomain, LOCAL_DOMAIN);
    StrAllocCopy(URLDomainPrefixes, URL_DOMAIN_PREFIXES);
    StrAllocCopy(URLDomainSuffixes, URL_DOMAIN_SUFFIXES);
    StrAllocCopy(XLoadImageCommand, XLOADIMAGE_COMMAND);
    StrAllocCopy(SSL_cert_file, SSL_CERT_FILE);

#ifndef DISABLE_BIBP
    StrAllocCopy(BibP_globalserver, BIBP_GLOBAL_SERVER);
    StrAllocCopy(BibP_bibhost, "http://bibhost/");	/* protocol specified. */
#endif

    /*
     * Disable news posting if the compilation-based LYNewsPosting value is
     * FALSE.  This may be changed further down via lynx.cfg or the
     * -restriction command line switch.  - FM
     */
#ifndef DISABLE_NEWS
    no_newspost = (BOOL) (LYNewsPosting == FALSE);
#endif

    for (i = 1; i < argc; i++) {
	parse_arg(&argv[i], 2, &i);
    }

    /*
     * If we have a lone "-" switch for getting arguments from stdin, get them
     * NOW, and act on the relevant ones, saving the others into an HTList for
     * handling after the other initializations.  The primary purpose of this
     * feature is to allow for the potentially very long command line that can
     * be associated with post or get data.  The original implementation
     * required that the lone "-" be the only command line argument, but that
     * precluded its use when the lynx command is aliased with other arguments.
     * When interactive, the stdin input is terminated by by Control-D on Unix
     * or Control-Z on VMS, and each argument is terminated by a RETURN.  When
     * the argument is -get_data or -post_data, the data are terminated by a
     * "---" string, alone on the line (also terminated by RETURN).  - FM
     */
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-") == 0) {
	    LYGetStdinArgs = TRUE;
	    break;
	}
    }
    if (LYGetStdinArgs == TRUE) {
	char *buf = NULL;

	CTRACE((tfp, "processing stdin arguments\n"));
	while (GetStdin(&buf, TRUE)) {
	    char *noargv[2];

	    noargv[0] = buf;
	    noargv[1] = NULL;
	    LYTrimTrailing(buf);

	    if (parse_arg(&noargv[0], 2, (int *) 0) == FALSE
		&& buf[0] != '\0') {
		char *argument = NULL;

		if (LYStdinArgs == NULL) {
		    LYStdinArgs = HTList_new();
#ifdef LY_FIND_LEAKS
		    atexit(LYStdinArgs_free);
#endif
		}
		StrAllocCopy(argument, buf);
		HTList_appendObject(LYStdinArgs, argument);
		CTRACE((tfp, "...StdinArg:%s\n", argument));
	    } else {
		CTRACE((tfp, "...complete:%s\n", buf));
	    }
	}
	CTRACE((tfp, "...done with stdin arguments\n"));
	FREE(buf);
    }
#ifdef SOCKS
    if (socks_flag)
	SOCKSinit(argv[0]);
#endif /* SOCKS */

    /*
     * If we had -validate set all of the restrictions and disallow a TRACE log
     * NOW.  - FM
     */
    if (LYValidate == TRUE) {
	parse_restrictions("all");
	LYUseTraceLog = FALSE;
    }

    /*
     * If we didn't get and act on a -validate or -anonymous switch, but can
     * verify that this is the anonymous account, set the default restrictions
     * for that account and disallow a TRACE log NOW.  - FM
     */
    if (!LYValidate && !LYRestricted &&
	strlen(ANONYMOUS_USER) > 0 &&
#if defined (VMS) || defined (NOUSERS)
	!strcasecomp((LYGetEnv("USER") == NULL ? " " : LYGetEnv("USER")),
		     ANONYMOUS_USER)
#else
#ifdef HAVE_CUSERID
	STREQ((char *) cuserid((char *) NULL), ANONYMOUS_USER)
#else
	STREQ(((char *) getlogin() == NULL ? " " : getlogin()), ANONYMOUS_USER)
#endif /* HAVE_CUSERID */
#endif /* VMS */
	) {
	parse_restrictions("default");
	LYRestricted = TRUE;
	LYUseTraceLog = FALSE;
    }
#ifdef USE_CMD_LOGGING
    /*
     * Open command-script, if specified
     */
    if (non_empty(lynx_cmd_script)) {
	LYTildeExpand(&lynx_cmd_script, TRUE);
	LYOpenCmdScript();
    }
    /*
     * Open command-logging, if specified
     */
    if (non_empty(lynx_cmd_logfile)) {
	LYTildeExpand(&lynx_cmd_logfile, TRUE);
	LYOpenCmdLogfile(argc, argv);
    }
#endif

    /*
     * Set up the default jump file stuff.  - FM
     */
    StrAllocCopy(jumpprompt, JUMP_PROMPT);
#ifdef JUMPFILE
    StrAllocCopy(jumpfile, JUMPFILE);
    {
	temp = NULL;
	HTSprintf0(&temp, "JUMPFILE:%s", jumpfile);
	if (!LYJumpInit(temp)) {
	    CTRACE((tfp, "Failed to register %s\n", temp));
	}
	FREE(temp);
    }
#endif /* JUMPFILE */

    /*
     * If no alternate configuration file was specified on the command line,
     * see if it's in the environment.
     */
    if (isEmpty(lynx_cfg_file)) {
	if (((cp = LYGetEnv("LYNX_CFG")) != NULL) ||
	    (cp = LYGetEnv("lynx_cfg")) != NULL)
	    StrAllocCopy(lynx_cfg_file, cp);
    }
#ifdef USE_PROGRAM_DIR
    if (isEmpty(lynx_cfg_file)) {
	HTSprintf0(&lynx_cfg_file, "%s\\lynx.cfg", program_dir);
	if (!LYCanReadFile(lynx_cfg_file)) {
	    FREE(lynx_cfg_file);
	    lynx_cfg_file = NULL;
	}
    }
#endif

    /*
     * If we still don't have a configuration file, use the userdefs.h
     * definition.
     */
    if (isEmpty(lynx_cfg_file))
	StrAllocCopy(lynx_cfg_file, LYNX_CFG_FILE);

#ifndef _WINDOWS		/* avoid the whole ~ thing for now */
    LYTildeExpand(&lynx_cfg_file, FALSE);
#endif

    /*
     * If the configuration file is not available, inform the user and exit.
     */
    if (!LYCanReadFile(lynx_cfg_file)) {
	fprintf(stderr,
		gettext("\nConfiguration file \"%s\" is not available.\n\n"),
		lynx_cfg_file);
	exit_immediately(EXIT_FAILURE);
    }

    /*
     * Make sure we have the character sets declared.  This will initialize the
     * CHARTRANS handling.  - KW
     */
    if (!LYCharSetsDeclared()) {
	fprintf(stderr, gettext("\nLynx character sets not declared.\n\n"));
	exit_immediately(EXIT_FAILURE);
    }
    /*
     * (**) in Lynx, UCLYhndl_HTFile_for_unspec and UCLYhndl_for_unrec may be
     * valid or not, but current_char_set and UCLYhndl_for_unspec SHOULD ALWAYS
     * be a valid charset.  Initialized here and may be changed later from
     * lynx.cfg/command_line/options_menu.  - LP (**)
     */
    /*
     * Set up the compilation default character set.  - FM
     */
#ifdef CAN_AUTODETECT_DISPLAY_CHARSET
    if (auto_display_charset >= 0)
	current_char_set = auto_display_charset;
    else
#endif
	current_char_set = safeUCGetLYhndl_byMIME(CHARACTER_SET);
    /*
     * Set up HTTP default for unlabeled charset (iso-8859-1).
     */
    UCLYhndl_for_unspec = LATIN1;
    StrAllocCopy(UCAssume_MIMEcharset,
		 LYCharSet_UC[UCLYhndl_for_unspec].MIMEname);

#ifdef USE_COLOR_TABLE
    /*
     * Set up default foreground and background colors.
     */
    lynx_setup_colors();
#endif /* USE_COLOR_TABLE */

    /*
     * Set the original directory, used for default download
     */
    if (!strcmp(Current_Dir(filename), ".")) {
	if ((cp = LYGetEnv("PWD")) != 0)
	    StrAllocCopy(original_dir, cp);
    } else {
	StrAllocCopy(original_dir, filename);
    }

    /*
     * Set the compilation default signature file.  - FM
     */
    LYStrNCpy(filename, LYNX_SIG_FILE, sizeof(filename) - 1);
    if (LYPathOffHomeOK(filename, sizeof(filename))) {
	StrAllocCopy(LynxSigFile, filename);
	LYAddPathToHome(filename, sizeof(filename), LynxSigFile);
	StrAllocCopy(LynxSigFile, filename);
	CTRACE((tfp, "LYNX_SIG_FILE set to '%s'\n", LynxSigFile));
    } else {
	CTRACE((tfp, "LYNX_SIG_FILE '%s' is bad. Ignoring.\n", LYNX_SIG_FILE));
    }

#ifdef USE_PRETTYSRC
    /*this is required for checking the tagspecs when parsing cfg file by
       LYReadCFG.c:parse_html_src_spec -HV */
    HTSwitchDTD(TRUE);
#endif
    /*
     * Process the configuration file.
     */
    read_cfg(lynx_cfg_file, "main program", 1, (FILE *) 0);

    {
	static char *client_keyfile = NULL;
	static char *client_certfile = NULL;

	if ((client_keyfile = LYGetEnv("SSL_CLIENT_KEY_FILE")) != NULL) {
	    CTRACE((tfp,
		    "HTGetSSLHandle: client keyfile is set to %s by SSL_CLIENT_KEY_FILE\n",
		    client_keyfile));
	    StrAllocCopy(SSL_client_key_file, client_keyfile);
	}

	if ((client_certfile = LYGetEnv("SSL_CLIENT_CERT_FILE")) != NULL) {
	    CTRACE((tfp,
		    "HTGetSSLHandle: client certfile is set to %s by SSL_CLIENT_CERT_FILE\n",
		    client_certfile));
	    StrAllocCopy(SSL_client_cert_file, client_certfile);
	}
    }

#if defined(USE_COLOR_STYLE)
    if (!dump_output_immediately) {
	init_color_styles(&lynx_lss_file2, default_color_styles);
    }
#endif /* USE_COLOR_STYLE */

    /*
     * Process the RC file.
     */
    read_rc(NULL);

#ifdef USE_LOCALE_CHARSET
    LYFindLocaleCharset();
#endif

    /*
     * Get WWW_HOME environment variable if it exists.
     */
    if ((cp = LYGetEnv("WWW_HOME")) != NULL) {
	StrAllocCopy(startfile, cp);
	LYEscapeStartfile(&startfile);
    }

    /*
     * Set the LynxHome URL.  If it's a file URL and the
     * host is defaulted, force in "//localhost", and if
     * it's not an absolute URL, make it one. - FM
     */
    StrAllocCopy(LynxHome, startfile);
    LYEnsureAbsoluteURL(&LynxHome, "LynxHome", FALSE);

    /*
     * Process any command line arguments not already handled.  - FM
     * May set startfile as a side-effect.
     */
    for (i = 1; i < argc; i++) {
	parse_arg(&argv[i], 4, &i);
    }

    /*
     * Process any stdin-derived arguments for a lone "-" which we've loaded
     * into LYStdinArgs.  - FM
     */
    if (LYStdinArgs != NULL) {
	char *my_args[2];
	HTList *cur = LYStdinArgs;

	my_args[1] = NULL;
	while (NULL != (my_args[0] = (char *) HTList_nextObject(cur))) {
	    parse_arg(my_args, 4, (int *) 0);
	}
	LYStdinArgs_free();
    }
#ifdef HAVE_TTYNAME
    /*
     * If the input is not a tty, we are either running in cron, or are
     * getting input via a pipe:
     *
     * a) in cron, none of stdin/stdout/stderr are tty's.
     * b) from a pipe, we should have either "-" or "-stdin" options.
     */
    if (!LYGetStdinArgs
	&& !startfile_stdin
	&& !isatty(fileno(stdin))
	&& (isatty(fileno(stdout) || isatty(fileno(stderr))))) {
	int ignored = 0;

	while (fgetc(stdin) != EOF) {
	    ++ignored;
	}
	if (ignored) {
	    fprintf(stderr,
		    gettext("Ignored %d characters from standard input.\n"), ignored);
	    fprintf(stderr,
		    gettext("Use \"-stdin\" or \"-\" to tell how to handle piped input.\n"));
	}
    }
#endif /* HAVE_TTYNAME */

#ifdef CAN_SWITCH_DISPLAY_CHARSET
    if (current_char_set == auto_display_charset)	/* Better: explicit option */
	switch_display_charsets = 1;
#endif

#if defined (TTY_DEVICE) || defined(HAVE_TTYNAME)
    /*
     * If we are told to read the startfile from standard input, do it now,
     * after we have read all of the option data from standard input.
     * Later we'll use LYReopenInput().
     */
    if (startfile_stdin) {
	char result[LY_MAXPATH];
	char *buf = NULL;

	CTRACE((tfp, "processing stdin startfile\n"));
	if ((fp = LYOpenTemp(result, HTML_SUFFIX, "w")) != 0) {
	    StrAllocCopy(startfile, result);
	    while (GetStdin(&buf, FALSE)) {
		fputs(buf, fp);
		fputc('\n', fp);
	    }
	    FREE(buf);
	    LYCloseTempFP(fp);
	}
	CTRACE((tfp, "...done stdin startfile\n"));
    }
#endif

    /*
     * Initialize other things based on the configuration read.
     */

#ifdef USE_PRETTYSRC
    if ((!Old_DTD) != TRUE)	/* skip if they are already initialized -HV */
#endif
	HTSwitchDTD(!Old_DTD);

    /*
     * Set up the proper character set with the desired
     * startup raw 8-bit or CJK mode handling.  - FM
     */
    HTMLUseCharacterSet(current_char_set);

#ifdef USE_PERSISTENT_COOKIES
    /*
     * Sod it, this looks like a reasonable place to load the
     * cookies file, probably.  - RP
     *
     * And to set LYCookieSaveFile. - BJP
     */
    if (persistent_cookies) {
	if (LYCookieFile == NULL) {
	    LYCookieFile = typeMallocn(char, LY_MAXPATH);

	    LYAddPathToHome(LYCookieFile, (size_t) LY_MAXPATH, FNAME_LYNX_COOKIES);
	} else {
	    LYTildeExpand(&LYCookieFile, FALSE);
	}
	LYLoadCookies(LYCookieFile);
    }

    /* tilde-expand LYCookieSaveFile */
    if (non_empty(LYCookieSaveFile)) {
	LYTildeExpand(&LYCookieSaveFile, FALSE);
    }
#ifdef USE_PROGRAM_DIR
    if (is_url(helpfile) == 0) {
	char *tmp = NULL;

	HTSprintf0(&tmp, "%s\\%s", program_dir, helpfile);
	FREE(helpfile);
	LYLocalFileToURL(&helpfile, tmp);
	FREE(tmp);
    }
#endif

    /*
     * In dump_output_immediately mode, LYCookieSaveFile defaults to
     * /dev/null, otherwise it defaults to LYCookieFile.
     */

    if (LYCookieSaveFile == NULL) {
	if (dump_output_immediately) {
	    StrAllocCopy(LYCookieSaveFile, "/dev/null");
	} else {
	    StrAllocCopy(LYCookieSaveFile, LYCookieFile);
	}
    }
#endif

    /*
     * Check for a help file URL in the environment. Overiding
     * compiled-in default and configuration file setting, if found.
     */
    if ((cp = LYGetEnv("LYNX_HELPFILE")) != NULL)
	StrAllocCopy(helpfile, cp);

    /*
     * Set up our help and about file base paths. - FM
     */
    StrAllocCopy(helpfilepath, helpfile);
    if ((cp = LYPathLeaf(helpfilepath)) != helpfilepath)
	*cp = '\0';
    LYAddHtmlSep(&helpfilepath);

    /*
     * Check for a save space path in the environment.  If one was set in the
     * configuration file, that one will be overridden.  - FM
     */
    if ((cp = LYGetEnv("LYNX_SAVE_SPACE")) != NULL)
	StrAllocCopy(lynx_save_space, cp);

    /*
     * We have a save space path, make sure it's valid.  - FM
     */
    if (isEmpty(lynx_save_space)) {
	FREE(lynx_save_space);
    }
    if (non_empty(lynx_save_space)) {
	LYTildeExpand(&lynx_save_space, TRUE);
#ifdef VMS
	LYLowerCase(lynx_save_space);
	if (StrChr(lynx_save_space, '/') != NULL) {
	    if (strlen(lynx_save_space) == 1) {
		StrAllocCopy(lynx_save_space, "sys$login:");
	    } else {
		LYAddPathSep(&lynx_save_space);
		StrAllocCopy(temp, HTVMS_name("", lynx_save_space));
		StrAllocCopy(lynx_save_space, temp);
		FREE(temp);
	    }
	}
	if (StrChr(lynx_save_space, ':') == NULL &&
	    StrChr(lynx_save_space, ']') == NULL) {
	    StrAllocCat(lynx_save_space, ":");
	}
#else
	LYAddPathSep(&lynx_save_space);
#endif /* VMS */
    }

    /*
     * Set up the file extension and mime type maps from src/HTInit.c and the
     * global and personal mime.types and mailcap files.  These will override
     * any SUFFIX or VIEWER maps in userdefs.h or the configuration file, if
     * they overlap.
     */
    HTFormatInit();
    if (!FileInitAlreadyDone)
	HTFileInit();

    if (!LYCheckUserAgent()) {
	HTAlwaysAlert(gettext("Warning:"), UA_NO_LYNX_WARNING);
    }
    if (show_cfg) {
	cleanup();
	exit_immediately(EXIT_SUCCESS);
    }
#ifdef USE_SLANG
    if (LYShowColor >= SHOW_COLOR_ON &&
	!(Lynx_Color_Flags & SL_LYNX_USE_COLOR)) {
	Lynx_Color_Flags |= SL_LYNX_USE_COLOR;
    } else if ((Lynx_Color_Flags & SL_LYNX_USE_COLOR) ||
	       LYGetEnv("COLORTERM") != NULL) {
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

    if (no_numbers) {
	number_links = FALSE;
	number_fields = FALSE;
	keypad_mode = NUMBERS_AS_ARROWS;
	set_numbers_as_arrows();
    }

    if (crawl) {
	/* No numbered links by default, as documented
	   in CRAWL.announce. - kw */
	if (!number_links) {
	    keypad_mode = NUMBERS_AS_ARROWS;
	}
    }

    if (!links_are_numbered()) {
	if (number_fields)
	    keypad_mode = LINKS_AND_FIELDS_ARE_NUMBERED;
	if (number_links)
	    keypad_mode = LINKS_ARE_NUMBERED;
	set_numbers_as_arrows();
    }

    /*
     * Check the -popup command line toggle.  - FM
     */
    if (LYUseDefSelPop == FALSE) {
	LYSelectPopups = (BOOLEAN) !LYSelectPopups;
    }

    /*
     * Check the -show_cursor command line toggle.  - FM
     */
    if (LYUseDefShoCur == FALSE) {
	LYShowCursor = (BOOLEAN) !LYShowCursor;
    }

    /*
     * Check the -base command line switch with -source.  - FM
     */
    if (LYPrependBase && HTOutputFormat == HTAtom_for("www/download")) {
	LYPrependBaseToSource = TRUE;
    }

    /*
     * Disable multiple bookmark support if not interactive, so it doesn't
     * crash on curses functions, or if the support was blocked via userdefs.h
     * and/or lynx.cfg, or via command line restrictions.  - FM
     */
    if (no_multibook)
	LYMBMBlocked = TRUE;
    if (dump_output_immediately || LYMBMBlocked || no_multibook) {
	LYMultiBookmarks = MBM_OFF;
	LYMBMBlocked = TRUE;
	no_multibook = TRUE;
    }
#ifdef USE_SOURCE_CACHE
    /*
     * Disable source caching if not interactive.
     */
    if (dump_output_immediately)
	LYCacheSource = SOURCE_CACHE_NONE;
#endif
#ifdef DISP_PARTIAL
    /*
     * Disable partial mode if not interactive.
     */
    if (dump_output_immediately)
	display_partial_flag = FALSE;
#endif

#ifdef VMS
    set_vms_keys();
#endif /* VMS */

#if defined (__DJGPP__)
    if (watt_debug)
	dbug_init();
    sock_init();

    __system_flags =
	__system_emulate_chdir |	/* handle `cd' internally */
	__system_handle_null_commands |		/* ignore cmds with no effect */
	__system_allow_long_cmds |	/* handle commands > 126 chars   */
	__system_use_shell |	/* use $SHELL if set */
	__system_allow_multiple_cmds |	/* allow `cmd1; cmd2; ...' */
	__system_redirect;	/* redirect internally */

    /* This speeds up stat() tremendously */
    _djstat_flags |= _STAT_INODE | _STAT_EXEC_MAGIC | _STAT_DIRSIZE;
#endif /* __DJGPP__ */

    /* trap interrupts */
#ifdef WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE) cleanup_win32, TRUE);
#endif

#ifndef NOSIGHUP
    if (!dump_output_immediately)
	(void) signal(SIGHUP, cleanup_sig);
#endif /* NOSIGHUP */

    (void) signal(SIGTERM, cleanup_sig);
#ifdef SIGWINCH
    LYExtSignal(SIGWINCH, size_change);
#endif /* SIGWINCH */
#ifndef VMS
    if (!TRACE && !dump_output_immediately && !stack_dump) {
	(void) signal(SIGINT, cleanup_sig);
#ifndef __linux__
#ifdef SIGBUS
	(void) signal(SIGBUS, FatalProblem);
#endif /* SIGBUS */
#endif /* !__linux__ */
	(void) signal(SIGSEGV, FatalProblem);
	(void) signal(SIGILL, FatalProblem);
	/*
	 * Since we're doing lots of TCP, just ignore SIGPIPE altogether.
	 *
	 * HTTCP.c should deal with a broken pipe for servers.  Rick Mallet's
	 * check after c = GetChar() in LYStrings.c should deal with a
	 * disconnected terminal.  So the runaway CPU time problem on Unix
	 * should not occur any more.
	 */
#ifdef SIGPIPE
	if (signal(SIGPIPE, SIG_IGN) != SIG_IGN)
	    restore_sigpipe_for_children = TRUE;
#endif /* SIGPIPE */
    }
#endif /* !VMS */

#ifdef SIGTSTP
    /*
     * Block Control-Z suspending if requested.  - FM
     */
    if (no_suspend)
	(void) signal(SIGTSTP, SIG_IGN);
#endif /* SIGTSTP */

    /*
     * Check for a valid HEAD request.  - FM
     */
    if (HEAD_request && LYCanDoHEAD(startfile) != TRUE) {
	fprintf(stderr,
		"The '-head' switch is for http HEAD requests and cannot be used for\n'%s'.\n",
		startfile);
	exit_immediately(EXIT_FAILURE);
    }

    /*
     * Check for a valid MIME headers request.  - FM
     */
    if (keep_mime_headers && LYCanDoHEAD(startfile) != TRUE) {
	fprintf(stderr,
		"The '-mime_header' switch is for http URLs and cannot be used for\n'%s'.\n",
		startfile);
	exit_immediately(EXIT_FAILURE);
    }

    /*
     * Check for a valid traversal request.  - FM
     */
    if (traversal && StrNCmp(startfile, "http", 4)) {
	fprintf(stderr,
		"The '-traversal' switch is for http URLs and cannot be used for\n'%s'.\n",
		startfile);
	exit_immediately(EXIT_FAILURE);
    }

    /*
     * Finish setting up for an INTERACTIVE session.  Done here so that URL
     * guessing in LYEnsureAbsoluteURL() can be interruptible (terminal is in
     * raw mode, select() works).  -BL
     */
#ifdef USE_PRETTYSRC
    if (!dump_output_immediately) {
	HTMLSRC_init_caches(FALSE);	/* do it before terminal is initialized */
    }
#ifdef LY_FIND_LEAKS
    atexit(html_src_clean_data);
#endif
#endif

    if (!dump_output_immediately) {
	setup(terminal);
    }
    /*
     * If startfile is a file URL and the host is defaulted, force in
     * "//localhost", and if it's not an absolute URL, make it one.  - FM
     */
    LYEnsureAbsoluteURL(&startfile, "STARTFILE", FALSE);

    /*
     * If homepage was specified and is a file URL with the host defaulted,
     * force in "//localhost", and if it's not an absolute URL, make it one.  -
     * FM
     */
    if (non_empty(homepage)) {
	LYEnsureAbsoluteURL(&homepage, "HOMEPAGE", FALSE);
    }

    /*
     * If we don't have a homepage specified, set it to startfile.  Otherwise,
     * reset LynxHome.  - FM
     */
    if (isEmpty(homepage)) {
	StrAllocCopy(homepage, startfile);
    } else {
	StrAllocCopy(LynxHome, homepage);
    }

    /*
     * Set up the inside/outside domain restriction flags.  - FM
     */
    if (inlocaldomain()) {
#if !defined(HAVE_UTMP) || defined(VMS)		/* not selective */
	telnet_ok = (BOOL) (!no_inside_telnet && !no_outside_telnet && telnet_ok);
#ifndef DISABLE_NEWS
	news_ok = (BOOL) (!no_inside_news && !no_outside_news && news_ok);
#endif
	ftp_ok = (BOOL) (!no_inside_ftp && !no_outside_ftp && ftp_ok);
	rlogin_ok = (BOOL) (!no_inside_rlogin && !no_outside_rlogin && rlogin_ok);
#else
	CTRACE((tfp, "LYMain: User in Local domain\n"));
	telnet_ok = (BOOL) (!no_inside_telnet && telnet_ok);
#ifndef DISABLE_NEWS
	news_ok = (BOOL) (!no_inside_news && news_ok);
#endif
	ftp_ok = (BOOL) (!no_inside_ftp && ftp_ok);
	rlogin_ok = (BOOL) (!no_inside_rlogin && rlogin_ok);
#endif /* !HAVE_UTMP || VMS */
    } else {
	CTRACE((tfp, "LYMain: User in REMOTE domain\n"));
	telnet_ok = (BOOL) (!no_outside_telnet && telnet_ok);
#ifndef DISABLE_NEWS
	news_ok = (BOOL) (!no_outside_news && news_ok);
#endif
	ftp_ok = (BOOL) (!no_outside_ftp && ftp_ok);
	rlogin_ok = (BOOL) (!no_outside_rlogin && rlogin_ok);
    }
#ifdef DISABLE_FTP
    ftp_ok = FALSE;
#else
    /* predefine some known broken ftp servers */
    LYSetConfigValue(RC_BROKEN_FTP_RETR, "ProFTPD 1.2.5");
    LYSetConfigValue(RC_BROKEN_FTP_RETR, "spftp/");
    LYSetConfigValue(RC_BROKEN_FTP_EPSV, "(Version wu-2.6.2-12)");
#endif

    /*
     * Make sure our bookmark default strings are all allocated and
     * synchronized.  - FM
     */
    if (isEmpty(bookmark_page)) {
	temp = NULL;
	HTSprintf0(&temp, "lynx_bookmarks%s", HTML_SUFFIX);
	set_default_bookmark_page(temp);
	FREE(temp);
    }
    if (isEmpty(BookmarkPage)) {
	set_default_bookmark_page(bookmark_page);
    }
#if defined(SYSLOG_REQUESTED_URLS)
    LYOpenlog(syslog_txt);
#endif

    if (non_empty(x_display)) {
	LYisConfiguredForX = TRUE;
    }

    /*
     * Here's where we do all the work.
     */
    if (dump_output_immediately) {
	/*
	 * Finish setting up and start a NON-INTERACTIVE session.  - FM
	 */
	if (crawl && !number_links && !number_fields) {
	    keypad_mode = NUMBERS_AS_ARROWS;
	} else if (no_numbers) {
	    keypad_mode = NUMBERS_AS_ARROWS;
	} else if (!no_list) {
	    if (!links_are_numbered()) {
		if (number_fields)
		    keypad_mode = LINKS_AND_FIELDS_ARE_NUMBERED;
		else
		    keypad_mode = LINKS_ARE_NUMBERED;
	    }
	}
	if (dump_output_width > 0) {
	    LYcols = dump_output_width;
	}
	/*
	 * Normal argument processing puts non-options (URLs) into the Goto
	 * history.  Use this to dump all of the pages listed on the command
	 * line, or (if none are listed) via the startfile mechanism.
	 * history.
	 */
#ifdef EXTENDED_STARTFILE_RECALL
	HTAddGotoURL(startfile);
	for (i = HTList_count(Goto_URLs) - 1; i >= 0; --i) {
	    StrAllocCopy(startfile, (char *) HTList_objectAt(Goto_URLs, i));
	    CTRACE((tfp, "dumping %d:%d %s\n",
		    i + 1, HTList_count(Goto_URLs), startfile));
	    status = mainloop();
	    if (!no_list &&
		!dump_links_inline &&
		!crawl)		/* For -crawl it has already been done! */
		printlist(stdout, FALSE);
	    if (i != 0)
		printf("\n");
	}
#else
	status = mainloop();
	if (!no_list &&
	    !dump_links_inline &&
	    !crawl &&		/* For -crawl it has already been done! */
	    links_are_numbered())
	    printlist(stdout, FALSE);
#endif
#ifdef USE_PERSISTENT_COOKIES
	/*
	 * We want to save cookies picked up when in immediate dump mode.
	 * Instead of calling cleanup() here, let's only call this one.  - BJP
	 */
	if (persistent_cookies)
	    LYStoreCookies(LYCookieSaveFile);
#endif /* USE_PERSISTENT_COOKIES */
	exit_immediately(status);
    } else {
	/*
	 * Start an INTERACTIVE session.  - FM
	 */
#ifdef USE_COLOR_STYLE
	cache_tag_styles();
#endif

#ifndef NO_DUMP_WITH_BACKSPACES
	if (with_backspaces) {
	    /* we should warn about this somehow (nop for now) -VH */
	    with_backspaces = FALSE;
	}
#endif

#ifndef ALL_CHARSETS_IN_O_MENU_SCREEN
	init_charset_subsets();
#endif

	ena_csi((BOOLEAN) (LYlowest_eightbit[current_char_set] > 155));
#ifdef USE_SESSIONS
	RestoreSession();
#endif /* USE_SESSIONS */
	status = mainloop();
	LYCloseCloset(RECALL_URL);
	LYCloseCloset(RECALL_MAIL);
#if defined(PDCURSES) && defined(PDC_BUILD) && PDC_BUILD >= 2401
	if (!isendwin()) {
	    if ((saved_scrsize_x != 0) && (saved_scrsize_y != 0)) {
		resize_term(saved_scrsize_y, saved_scrsize_x);
	    }
	}
#endif
	cleanup();
	exit_immediately(status);
    }

    return (status);		/* though redundant, for compiler-warnings */
}

/*
 * Called by HTAccessInit to register any protocols supported by lynx.
 * Protocols added by lynx:
 *    LYNXKEYMAP, lynxcgi, LYNXIMGMAP, LYNXCOOKIE, LYNXCACHE, LYNXMESSAGES
 */
#ifdef GLOBALREF_IS_MACRO
extern GLOBALREF (HTProtocol, LYLynxEditmap);
extern GLOBALREF (HTProtocol, LYLynxKeymap);
extern GLOBALREF (HTProtocol, LYLynxCGI);
extern GLOBALREF (HTProtocol, LYLynxIMGmap);
extern GLOBALREF (HTProtocol, LYLynxCookies);

#ifdef USE_CACHEJAR
extern GLOBALREF (HTProtocol, LYLynxCache);
#endif
extern GLOBALREF (HTProtocol, LYLynxStatusMessages);

#else
GLOBALREF HTProtocol LYLynxEditmap;
GLOBALREF HTProtocol LYLynxKeymap;
GLOBALREF HTProtocol LYLynxCGI;
GLOBALREF HTProtocol LYLynxIMGmap;
GLOBALREF HTProtocol LYLynxCookies;

#ifdef USE_CACHEJAR
GLOBALREF HTProtocol LYLynxCache;
#endif
GLOBALREF HTProtocol LYLynxStatusMessages;
#endif /* GLOBALREF_IS_MACRO */

void LYRegisterLynxProtocols(void)
{
    HTRegisterProtocol(&LYLynxEditmap);
    HTRegisterProtocol(&LYLynxKeymap);
    HTRegisterProtocol(&LYLynxCGI);
    HTRegisterProtocol(&LYLynxIMGmap);
    HTRegisterProtocol(&LYLynxCookies);
#ifdef USE_CACHEJAR
    HTRegisterProtocol(&LYLynxCache);
#endif
    HTRegisterProtocol(&LYLynxStatusMessages);
}

#ifndef NO_CONFIG_INFO
/*
 * Some stuff to reload lynx.cfg without restarting new lynx session, also load
 * options menu items and command-line options to make things consistent.
 *
 * Called by user of interactive session by LYNXCFG://reload/ link.
 *
 * Warning:  experimental, more main() reorganization required.
 *	*Known* exceptions: persistent cookies, cookie files.
 *
 *	Some aspects of COLOR (with slang?).
 *	Viewer stuff, mailcap files
 *	SUFFIX, mime.types files
 *	RULESFILE/RULE
 *
 *	All work "somewhat", but not exactly as the first time.
 */
void reload_read_cfg(void)
{
    char *tempfile;
    FILE *rcfp;

    /*
     * no_option_save is always set for -anonymous and -validate.  It is better
     * to check for one or several specific restriction flags than for
     * 'LYRestricted', which doesn't get set for individual restrictions or for
     * -validate!  However, no_option_save may not be the appropriate one to
     * check - in that case, a new no_something should be added that gets
     * automatically set for -anonymous and -validate (and whether it applies
     * for -anonymous can be made installer- configurable in the usual way at
     * the bottom of userdefs.h).  - kw
     *
     */
    if (no_option_save) {
	/* current logic requires(?) that saving user preferences is
	   possible.  Additional applicable restrictions are already
	   checked by caller. - kw */
	return;
    }

    /*
     * Current user preferences are saved in a temporary file, to be read in
     * again after lynx.cfg has been read.  This avoids accidental changing of
     * the preferences file.  The regular preferences file doesn't even need to
     * exist, and won't be created as a side effect of this function.  Honoring
     * the no_option_save restriction may thus be unnecessarily restrictive,
     * but the check is currently still left in place.  - kw
     */
    tempfile = typecallocn(char, LY_MAXPATH);
    if (!tempfile) {
	HTAlwaysAlert(NULL, NOT_ENOUGH_MEMORY);
	return;
    }
    rcfp = LYOpenTemp(tempfile, ".rc", "w");
    if (rcfp == NULL) {
	FREE(tempfile);
	HTAlwaysAlert(NULL, CANNOT_OPEN_TEMP);
	return;
    }
    if (!save_rc(rcfp)) {
	HTAlwaysAlert(NULL, OPTIONS_NOT_SAVED);
	(void) LYRemoveTemp(tempfile);
	FREE(tempfile);
	return;			/* can not write the very own file :( */
    }
    if (LYCookieFile != 0 && LYCookieSaveFile != 0) {
	/* set few safe flags: */
#ifdef USE_PERSISTENT_COOKIES
	BOOLEAN persistent_cookies_flag = persistent_cookies;
	char *LYCookieFile_flag = NULL;
	char *LYCookieSaveFile_flag = NULL;

	if (persistent_cookies) {
	    StrAllocCopy(LYCookieFile_flag, LYCookieFile);
	    StrAllocCopy(LYCookieSaveFile_flag, LYCookieSaveFile);
	}
#endif

#ifdef USE_CHARSET_CHOICE
	custom_assumed_doc_charset = FALSE;
	custom_display_charset = FALSE;
	memset((char *) charset_subsets, 0, sizeof(charset_subset_t) * MAXCHARSETS);
#endif

#ifdef USE_PRETTYSRC
	html_src_on_lynxcfg_reload();
#endif
	/* free downloaders, printers, environments, dired menu */
	free_lynx_cfg();
#ifdef USE_SOURCE_CACHE
	source_cache_file_error = FALSE;	/* reset flag */
#endif

	/*
	 * Process the configuration file.
	 */
	read_cfg(lynx_cfg_file, "main program", 1, (FILE *) 0);

	/*
	 * Process the temporary RC file.
	 */
	rcfp = fopen(tempfile, "r");
	read_rc(rcfp);
	(void) LYRemoveTemp(tempfile);
	FREE(tempfile);		/* done with it - kw */

#ifdef USE_CHARSET_CHOICE
	init_charset_subsets();
#endif

	/*
	 * Initialize other things based on the configuration read.
	 */
	LYSetDisplayLines();
	/* Not implemented yet here,
	 * a major problem: file paths
	 * like lynx_save_space, LYCookieFile etc.
	 */
#ifdef USE_PERSISTENT_COOKIES
	/* restore old settings */
	if (persistent_cookies != persistent_cookies_flag) {
	    persistent_cookies = persistent_cookies_flag;
	    HTAlert(gettext("persistent cookies state will be changed in next session only."));
	}
	if (persistent_cookies) {
	    if (strcmp(LYCookieFile, LYCookieFile_flag)) {
		StrAllocCopy(LYCookieFile, LYCookieFile_flag);
		CTRACE((tfp,
			"cookie file can be changed in next session only, restored.\n"));
	    }
	    if (strcmp(LYCookieSaveFile, LYCookieSaveFile_flag)) {
		StrAllocCopy(LYCookieSaveFile, LYCookieSaveFile_flag);
		CTRACE((tfp,
			"cookie save file can be changed in next session only, restored.\n"));
	    }
	    FREE(LYCookieFile_flag);
	    FREE(LYCookieSaveFile_flag);
	}
#endif

    }
}
#endif /* !NO_CONFIG_INFO */

static void force_dump_mode(void)
{
    dump_output_immediately = TRUE;
    no_pause = TRUE;
    LYcols = DFT_COLS;
}

/* There are different ways of setting arguments on the command line, and
 * there are different types of arguments.  These include:
 *
 *   -set_some_variable		 ==> some_variable  = TRUE
 *   -toggle_some_variable	 ==> some_variable = !some_variable
 *   -some_variable=value	 ==> some_variable = value
 *
 * Others are complicated and require a function call.
 */

#define PARSE_SET(n,t,v,h) {n,    t, UNION_SET(v), h}
#define PARSE_INT(n,t,v,h) {n,    t, UNION_INT(v), h}
#define PARSE_STR(n,t,v,h) {n,    t, UNION_STR(v), h}
#define PARSE_FUN(n,t,v,h) {n,    t, UNION_FUN(v), h}
#define PARSE_NIL          {NULL, 0, UNION_DEF(0), NULL}

typedef struct parse_args_type {
    const char *name;
    int type;

#define TOGGLE_ARG		0x0010
#define SET_ARG			0x0020
#define UNSET_ARG		0x0030
#define FUNCTION_ARG		0x0040
#define LYSTRING_ARG		0x0050
#define INT_ARG			0x0060
#define STRING_ARG		0x0070
#define TIME_ARG		0x0080
#define ARG_TYPE_MASK		0x0FF0
#define NEED_NEXT_ARG		0x1000

#define NEED_INT_ARG		(NEED_NEXT_ARG | INT_ARG)
#define NEED_TIME_ARG		(NEED_NEXT_ARG | TIME_ARG)
#define NEED_LYSTRING_ARG	(NEED_NEXT_ARG | LYSTRING_ARG)
#define NEED_STRING_ARG		(NEED_NEXT_ARG | STRING_ARG)
#define NEED_FUNCTION_ARG	(NEED_NEXT_ARG | FUNCTION_ARG)

    /* If the NEED_NEXT_ARG flags is set, and the option was not specified
     * with an '=' character, then use the next argument in the argv list.
     */

      ParseData;
    const char *help_string;
} Config_Type;

/* -auth, -pauth */
static int parse_authentication(char *next_arg,
				char **result)
{
    /*
     * Authentication information for protected documents.
     */
    char *auth_info = 0;

    if (next_arg != 0) {
	StrAllocCopy(auth_info, next_arg);
	memset(next_arg, ' ', strlen(next_arg));	/* Let's not show too much */
    }

    if (auth_info != 0) {
	char *cp;

	if ((cp = StrChr(auth_info, ':')) != 0) {	/* Pw */
	    *cp++ = '\0';	/* Terminate ID */
	    HTUnEscape(cp);
	    StrAllocCopy(result[1], cp);
	}
	if (*auth_info) {	/* Id */
	    HTUnEscape(auth_info);
	    StrAllocCopy(result[0], auth_info);
	}
	FREE(auth_info);
    }
    return 0;
}

/* -anonymous */
static int anonymous_fun(char *next_arg GCC_UNUSED)
{
    if (!LYValidate && !LYRestricted)
	parse_restrictions("default");
    LYRestricted = TRUE;
    return 0;
}

/* -assume_charset */
static int assume_charset_fun(char *next_arg)
{
    assumed_charset = TRUE;
    UCLYhndl_for_unspec = safeUCGetLYhndl_byMIME(next_arg);
    StrAllocCopy(UCAssume_MIMEcharset,
		 LYCharSet_UC[UCLYhndl_for_unspec].MIMEname);
    CTRACE((tfp, "assume_charset_fun %s ->%d ->%s\n",
	    NonNull(next_arg),
	    UCLYhndl_for_unspec,
	    UCAssume_MIMEcharset));
    return 0;
}

/* -assume_local_charset */
static int assume_local_charset_fun(char *next_arg)
{
    UCLYhndl_HTFile_for_unspec = safeUCGetLYhndl_byMIME(next_arg);
    return 0;
}

/* -assume_unrec_charset */
static int assume_unrec_charset_fun(char *next_arg)
{
    UCLYhndl_for_unrec = safeUCGetLYhndl_byMIME(next_arg);
    return 0;
}

/* -auth */
static int auth_fun(char *next_arg)
{
    parse_authentication(next_arg, authentication_info);
    return 0;
}

/* -base */
static int base_fun(char *next_arg GCC_UNUSED)
{
    /*
     * Treat -source equivalently to an interactive download with
     * LYPrefixBaseToSource configured to TRUE, so that a BASE tag is prepended
     * for text/html content types.  We normally treat the module-wide global
     * LYPrefixBaseToSource flag as FALSE with -source, but force it TRUE,
     * later, if LYPrependBase is set TRUE here.  - FM
     */
    LYPrependBase = TRUE;
    if (HTOutputFormat == HTAtom_for("www/dump"))
	HTOutputFormat = HTAtom_for("www/download");

    return 0;
}

/* -cache */
static int cache_fun(char *next_arg)
{
    if (next_arg != 0)
	HTCacheSize = atoi(next_arg);
    /*
     * Limit size.
     */
    if (HTCacheSize < 2)
	HTCacheSize = 2;

    return 0;
}

/* -child */
static int child_fun(char *next_arg GCC_UNUSED)
{
    child_lynx = TRUE;
    no_disk_save = TRUE;
    no_mail = TRUE;
    return 0;
}

/* -child_relaxed */
static int child_relaxed_fun(char *next_arg GCC_UNUSED)
{
    child_lynx = TRUE;
    return 0;
}

#ifdef USE_SLANG
/* -color */
static int color_fun(char *next_arg GCC_UNUSED)
{
    Lynx_Color_Flags |= SL_LYNX_USE_COLOR;

    if (LYShowColor != SHOW_COLOR_ALWAYS)
	LYShowColor = SHOW_COLOR_ON;

    return 0;
}
#endif

/* -convert_to */
static int convert_to_fun(char *next_arg)
{
    if (next_arg != 0) {
	char *outformat = NULL;
	char *cp1, *cp2, *cp4;
	int chndl;

	StrAllocCopy(outformat, next_arg);
	/* not lowercased, to allow for experimentation - kw */
	/*LYLowerCase(outformat); */
	if ((cp1 = StrChr(outformat, ';')) != NULL) {
	    if ((cp2 = LYstrstr(cp1, "charset")) != NULL) {
		cp2 += 7;
		while (*cp2 == ' ' || *cp2 == '=' || *cp2 == '"')
		    cp2++;
		for (cp4 = cp2; (*cp4 != '\0' && *cp4 != '"' &&
				 *cp4 != ';' &&
				 !WHITE(*cp4)); cp4++) ;	/* do nothing */
		*cp4 = '\0';
		/* This is intentionally not the "safe" version,
		   to allow for experimentation. */
		chndl = UCGetLYhndl_byMIME(cp2);
		if (chndl < 0)
		    chndl = UCLYhndl_for_unrec;
		if (chndl < 0) {
		    fprintf(stderr,
			    gettext("Lynx: ignoring unrecognized charset=%s\n"), cp2);
		} else {
		    current_char_set = chndl;
		}
		*cp1 = '\0';	/* truncate outformat */
	    }
	}
	HTOutputFormat = HTAtom_for(outformat);
	FREE(outformat);
    } else {
	HTOutputFormat = NULL;
    }
    return 0;
}

/* -crawl */
static int crawl_fun(char *next_arg GCC_UNUSED)
{
    crawl = TRUE;
    LYcols = DFT_COLS;
    return 0;
}

/* -display */
static int display_fun(char *next_arg)
{
    if (next_arg != 0) {
	LYsetXDisplay(next_arg);
    }

    return 0;
}

/* -display_charset */
static int display_charset_fun(char *next_arg)
{
    int i = UCGetLYhndl_byMIME(next_arg);

#ifdef CAN_AUTODETECT_DISPLAY_CHARSET
    if (i < 0 && !strcasecomp(next_arg, "auto"))
	i = auto_display_charset;
#endif
    if (i < 0) {		/* do nothing here: so fallback to lynx.cfg */
	fprintf(stderr,
		gettext("Lynx: ignoring unrecognized charset=%s\n"), next_arg);
    } else
	current_char_set = i;
    return 0;
}

/* -dump */
static int dump_output_fun(char *next_arg GCC_UNUSED)
{
    force_dump_mode();
    return 0;
}

/* -editor */
static int editor_fun(char *next_arg)
{
    if (next_arg != 0)
	StrAllocCopy(editor, next_arg);
    system_editor = TRUE;
    return 0;
}

/* -error_file */
static int error_file_fun(char *next_arg)
{
    /*
     * Output return (success/failure) code of an HTTP transaction.
     */
    if (next_arg != 0)
	http_error_file = next_arg;
    return 0;
}

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
/* -exec */
static int exec_fun(char *next_arg GCC_UNUSED)
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
static int get_data_fun(char *next_arg GCC_UNUSED)
{
    /*
     * User data for GET form.
     */
    char **get_data;
    char *buf = NULL;

    /*
     * On Unix, conflicts with curses when interactive so let's force a dump.
     * -CL
     *
     * On VMS, mods have been made in LYCurses.c to deal with potential
     * conflicts, so don't force the dump here.  - FM
     */
#ifndef VMS
    force_dump_mode();
#endif /* VMS */

    StrAllocCopy(form_get_data, "?");	/* Prime the pump */
    get_data = &form_get_data;

    /*
     * Build GET data for later.  Stop reading when we see a line with "---" as
     * its first three characters.
     */
    while (GetStdin(&buf, TRUE)) {
	StrAllocCat(*get_data, buf);
    }

    CTRACE((tfp, "get_data:%s\n", *get_data));
    CTRACE((tfp, "get_data:%s\n", form_get_data));
    return 0;
}

/* -help */
static int help_fun(char *next_arg GCC_UNUSED)
{
    print_help_and_exit(0);
    return 0;
}

/* -hiddenlinks */
int hiddenlinks_fun(char *next_arg)
{
    /* *INDENT-OFF* */
    static Config_Enum table[] = {
	{ "merge",	HIDDENLINKS_MERGE },
	{ "listonly",	HIDDENLINKS_SEPARATE },
	{ "ignore",	HIDDENLINKS_IGNORE },
	{ NULL,		-1 },
    };
    /* *INDENT-ON* */

    if (next_arg != 0) {
	if (!LYgetEnum(table, next_arg, &LYHiddenLinks))
	    print_help_and_exit(-1);
    } else {
	LYHiddenLinks = HIDDENLINKS_MERGE;
    }

    return 0;
}

/* -homepage */
static int homepage_fun(char *next_arg)
{
    if (next_arg != 0) {
	StrAllocCopy(homepage, next_arg);
	LYEscapeStartfile(&homepage);
    }
    return 0;
}

/* -mime_header */
static int mime_header_fun(char *next_arg GCC_UNUSED)
{
    /*
     * Include mime headers and force source dump.
     */
    keep_mime_headers = TRUE;
    force_dump_mode();
    HTOutputFormat = (LYPrependBase ?
		      HTAtom_for("www/download") : HTAtom_for("www/dump"));
    LYcols = MAX_COLS;
    return 0;
}

#ifndef DISABLE_NEWS
/* -newschunksize */
static int newschunksize_fun(char *next_arg)
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
static int newsmaxchunk_fun(char *next_arg)
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
#endif /* not DISABLE_NEWS */

/* -nobold */
static int nobold_fun(char *next_arg GCC_UNUSED)
{
    LYnoVideo(1);
    return 0;
}

/* -nobrowse */
static int nobrowse_fun(char *next_arg GCC_UNUSED)
{
    HTDirAccess = HT_DIR_FORBID;
    return 0;
}

/* -nocolor */
static int nocolor_fun(char *next_arg GCC_UNUSED)
{
    LYShowColor = SHOW_COLOR_NEVER;
#ifdef USE_SLANG
    Lynx_Color_Flags &= ~(unsigned) SL_LYNX_USE_COLOR;
    Lynx_Color_Flags |= SL_LYNX_OVERRIDE_COLOR;
#endif
    return 0;
}

/* -nopause */
static int nopause_fun(char *next_arg GCC_UNUSED)
{
    no_pause = TRUE;
    return 0;
}

/* -nomore */
static int nomore_fun(char *next_arg GCC_UNUSED)
{
    nomore = TRUE;
    return 0;
}

/* -noreverse */
static int noreverse_fun(char *next_arg GCC_UNUSED)
{
    LYnoVideo(2);
    return 0;
}

/* -nounderline */
static int nounderline_fun(char *next_arg GCC_UNUSED)
{
    LYnoVideo(4);
    return 0;
}

/* -nozap */
static int nozap_fun(char *next_arg)
{
    LYNoZapKey = 1;		/* everything but "initially" treated as "full" - kw */
    if (next_arg != 0) {
	if (strcasecomp(next_arg, "initially") == 0)
	    LYNoZapKey = 2;

    }
    return 0;
}

/* -pauth */
static int pauth_fun(char *next_arg)
{
    parse_authentication(next_arg, proxyauth_info);
    return 0;
}

/* -post_data */
static int post_data_fun(char *next_arg GCC_UNUSED)
{
    /*
     * User data for POST form.
     */
    char **post_data;
    char *buf = NULL;

    /*
     * On Unix, conflicts with curses when interactive so let's force a dump.
     * - CL
     *
     * On VMS, mods have been made in LYCurses.c to deal with potential
     * conflicts, so don't force a dump here.  - FM
     */
#ifndef VMS
    force_dump_mode();
#endif /* VMS */

    post_data = &form_post_data;

    /*
     * Build post data for later.  Stop reading when we see a line with "---"
     * as its first three characters.
     */
    while (GetStdin(&buf, TRUE)) {
	StrAllocCat(*post_data, buf);
    }
    return 0;
}

static const char *show_restriction(const char *name)
{
    const char *value = 0;

    switch (find_restriction(name, -1)) {
    case TRUE:
	value = "on";
	break;
    case FALSE:
	value = "off";
	break;
    default:
	value = "?";
	break;
    }
    return value;
}

/* -restrictions */
static int restrictions_fun(char *next_arg)
{
    /* *INDENT-OFF* */
    static const struct {
	const char *name;
	const char *help;
    } table[] = {
	{ "all", "restricts all options." },
	{ "bookmark", "disallow changing the location of the bookmark file" },
	{ "bookmark_exec", "disallow execution links via the bookmark file" },
#if defined(DIRED_SUPPORT) && defined(OK_PERMIT)
	{ "change_exec_perms", "\
disallow changing the eXecute permission on files\n\
(but still allow it for directories) when local file\n\
management is enabled." },
#endif /* DIRED_SUPPORT && OK_PERMIT */
#ifdef SUPPORT_CHDIR
	{ "chdir", "\
disallow changing the working directory of lynx, e.g.,\n\
to affect the behavior of download command" },
#endif
#if defined(HAVE_CONFIG_H) && !defined(NO_CONFIG_INFO)
	{ "compileopts_info", "\
disable info on options used to compile the binary" },
#endif
	{ "default", "\
same as commandline option -anonymous.  Sets the\n\
default service restrictions for anonymous users.  Set to\n\
all restricted, except for: inside_telnet, outside_telnet,\n\
inside_ftp, outside_ftp, inside_rlogin, outside_rlogin,\n\
inside_news, outside_news, telnet_port, jump, mail, print,\n\
exec, and goto.  The settings for these, as well as\n\
additional goto restrictions for specific URL schemes\n\
that are also applied, are derived from definitions\n\
within userdefs.h." },
#ifdef DIRED_SUPPORT
	{ "dired_support", "disallow local file management" },
#endif /* DIRED_SUPPORT */
	{ "disk_save", "disallow saving to disk in the download and print menus" },
	{ "dotfiles", "disallow access to, or creation of, hidden (dot) files" },
	{ "download", "disallow some downloaders in the download menu" },
	{ "editor", "disallow editing" },
	{ "exec", "disable execution scripts" },
	{ "exec_frozen", "disallow the user from changing the execution link option" },
#ifdef USE_EXTERNALS
	{ "externals", "disable passing URLs to some external programs" },
#endif
	{ "file_url", "\
disallow using G)oto, served links or bookmarks for\n\
file: URL's" },
	{ "goto", "disable the 'g' (goto) command" },
#if !defined(HAVE_UTMP) || defined(VMS) /* not selective */
	{ "inside_ftp", "\
disallow ftps coming from inside your\n\
domain (utmp required for selectivity)" },
	{ "inside_news", "\
disallow USENET news reading and posting coming\n\
from inside your domain (utmp required for selectivity)" },
	{ "inside_rlogin", "\
disallow rlogins coming from inside your\n\
domain (utmp required for selectivity)" },
	{ "inside_telnet", "\
disallow telnets coming from inside your\n\
domain (utmp required for selectivity)" },
#else
	{ "inside_ftp", "\
disallow ftps coming from inside your domain" },
	{ "inside_news", "\
disallow USENET news reading and posting coming\n\
from inside your domain" },
	{ "inside_rlogin", "\
disallow rlogins coming from inside your domain" },
	{ "inside_telnet", "\
disallow telnets coming from inside your domain" },
#endif /* HAVE_UTMP || VMS */
	{ "jump", "disable the 'j' (jump) command" },
	{ "lynxcfg_info", "\
disable viewing of lynx.cfg configuration file info" },
#ifndef NO_CONFIG_INFO
	{ "lynxcfg_xinfo", "\
disable extended lynx.cfg viewing and reloading" },
#endif
	{ "lynxcgi", "\
disallow execution of Lynx CGI URLs" },
	{ "mail", "disallow mail" },
	{ "multibook", "disallow multiple bookmark files" },
	{ "news_post", "disallow USENET News posting." },
	{ "option_save", "disallow saving options in .lynxrc" },
#if !defined(HAVE_UTMP) || defined(VMS) /* not selective */
	{ "outside_ftp", "\
disallow ftps coming from outside your\n\
domain (utmp required for selectivity)" },
	{ "outside_news", "\
disallow USENET news reading and posting coming\n\
from outside your domain (utmp required for selectivity)" },
	{ "outside_rlogin", "\
disallow rlogins coming from outside your\n\
domain (utmp required for selectivity)" },
	{ "outside_telnet", "\
disallow telnets coming from outside your\n\
domain (utmp required for selectivity)" },
#else
	{ "outside_ftp", "\
disallow ftp coming from outside your domain" },
	{ "outside_news", "\
disallow USENET news reading and posting coming\n\
from outside your domain" },
	{ "outside_rlogin", "\
disallow rlogins coming from outside your domain" },
	{ "outside_telnet", "\
disallow telnets coming from outside your domain" },
#endif /* !HAVE_UTMP || VMS */
	{ "print", "disallow most print options" },
	{ "shell", "\
disallow shell escapes, and lynxexec, lynxprog or lynxcgi\n\
G)oto's" },
	{ "suspend", "disallow Control-Z suspends with escape to shell" },
	{ "telnet_port", "disallow specifying a port in telnet G)oto's" },
	{ "useragent", "disallow modifications of the User-Agent header" },
    };
    /* *INDENT-ON* */

    static const char *Usage[] =
    {
	""
	,"USAGE: lynx -restrictions=[option][,option][,option]"
	,"List of Options:"
	,"  ?                 when used alone, list restrictions in effect."

    };
    unsigned j, k, column = 0;
    const char *name;
    const char *value;
    BOOLEAN found, first;

    if (isEmpty(next_arg)) {
	SetOutputMode(O_TEXT);
	for (j = 0; j < TABLESIZE(Usage); j++) {
	    printf("%s\n", Usage[j]);
	}
	for (j = 0; j < TABLESIZE(table); j++) {
	    if (!strcmp(table[j].name, "all")
		|| !strcmp(table[j].name, "default")) {
		value = NULL;
	    } else {
		value = show_restriction(table[j].name);
	    }
	    print_help_strings(table[j].name, table[j].help, value, FALSE);
	}
	first = TRUE;
	for (j = 0; j < TABLESIZE(table); j++) {
	    found = FALSE;
	    if ((name = index_to_restriction(j)) == 0) {
		break;
	    }
	    for (k = 0; k < TABLESIZE(table); k++) {
		if (!strcmp(name, table[k].name)) {
		    found = TRUE;
		}
	    }
	    if (!found) {
		if (first) {
		    printf("Other restrictions (see the user's guide):\n");
		}
		value = show_restriction(table[j].name);
		printf("%s%s (%s)", column ? ", " : "  ", name, value);
		column += (unsigned) (5 + strlen(name) + strlen(value));
		if (column > 50) {
		    column = 0;
		    printf("\n");
		}
		first = FALSE;
	    }
	}
	if (column)
	    printf("\n");
	SetOutputMode(O_BINARY);
	exit_immediately(EXIT_SUCCESS);
    } else if (*next_arg == '?') {
	SetOutputMode(O_TEXT);
	print_restrictions_to_fd(stdout);
	SetOutputMode(O_BINARY);
	exit_immediately(EXIT_SUCCESS);
    } else {
	parse_restrictions(next_arg);
    }
    return 0;
}

/* -selective */
static int selective_fun(char *next_arg GCC_UNUSED)
{
    HTDirAccess = HT_DIR_SELECTIVE;
    return 0;
}

/* -source */
static int source_fun(char *next_arg GCC_UNUSED)
{
    force_dump_mode();
    HTOutputFormat = (LYPrependBase ?
		      HTAtom_for("www/download") : HTAtom_for("www/dump"));
    LYcols = MAX_COLS;
    return 0;
}

/* -traversal */
static int traversal_fun(char *next_arg GCC_UNUSED)
{
    traversal = TRUE;
#ifdef USE_SLANG
    LYcols = DFT_COLS;
#else
    LYcols = MAX_COLS;
#endif /* USE_SLANG */

    return 0;
}

/* -version */
static int version_fun(char *next_arg GCC_UNUSED)
{
    char *result = NULL;

    SetLocale();
    SetOutputMode(O_TEXT);

    HTSprintf0(&result, gettext("%s Version %s (%s)"),
	       LYNX_NAME, LYNX_VERSION,
	       LYVersionDate());

    StrAllocCat(result, "\n");
#ifdef USE_SSL
    HTSprintf(&result, "libwww-FM %s,", HTLibraryVersion);
    append_ssl_version(&result, " ");
#else
    HTSprintf(&result, "libwww-FM %s", HTLibraryVersion);
#endif /* USE_SSL */

#if defined(NCURSES) && defined(HAVE_CURSES_VERSION)
    HTSprintf(&result, ", %s", curses_version());
#if defined(WIDEC_CURSES)
    HTSprintf(&result, "(wide)");
#endif
#elif defined(PDCURSES) && defined(PDC_BUILD)
    HTSprintf(&result, ", pdcurses %.3f", PDC_BUILD * 0.001);
#elif defined(USE_SLANG) && defined(SLANG_VERSION_STRING)
    HTSprintf(&result, ", s-lang %s", SLANG_VERSION_STRING);
#endif

    printf("%s\n", result);
    free(result);

/*
 * Define NO_BUILDSTAMP if you really want an executable with no timestamp in
 * the -version message.
 */
#ifdef NO_BUILDSTAMP
#define BUILDSTAMP ""
#else
#define BUILDSTAMP " (" __DATE__ " " __TIME__ ")"
#endif

/*
 * SYSTEM_NAME is set by the configure script.  Show build date/time for other
 * systems, according to predefined compiler symbols.
 */
#ifdef SYSTEM_NAME
    printf(gettext("Built on %s%s.\n"), SYSTEM_NAME, BUILDSTAMP);
#elif defined(__CYGWIN__)
    printf("Compiled by CYGWIN%s.\n", BUILDSTAMP);
#elif defined(__BORLANDC__)
    printf("Compiled by Borland C++%s.\n", BUILDSTAMP);
#elif defined(_MSC_VER)
    printf("Compiled by Microsoft Visual C++%s.\n", BUILDSTAMP);
#elif defined(__DJGPP__)
    printf("Compiled by DJGPP%s.\n", BUILDSTAMP);
#elif !defined(NO_BUILDSTAMP)
    printf("Compiled at %s %s.\n", __DATE__, __TIME__);
#endif

    puts("");
    puts(gettext("Copyrights held by the Lynx Developers Group,"));
    puts(gettext("the University of Kansas, CERN, and other contributors."));
    puts(gettext("Distributed under the GNU General Public License (Version 2)."));
    puts(gettext("See https://lynx.invisible-island.net/ and the online help for more information."));
    puts("");
#ifdef USE_SSL
#if defined(OPENSSL_VERSION_TEXT) && !defined(LIBGNUTLS_VERSION)
    puts("See http://www.openssl.org/ for information about OpenSSL.");
#endif /* OPENSSL_VERSION_TEXT */
    puts("");
#endif /* USE_SSL */

    SetOutputMode(O_BINARY);

    exit_immediately(EXIT_SUCCESS);
    /* NOT REACHED */
    return 0;
}

/* -width */
static int width_fun(char *next_arg)
{
    if (next_arg != 0) {
	int w = atoi(next_arg);

	if (w > 0)
	    dump_output_width = ((w < MAX_COLS) ? w : MAX_COLS);
    }

    return 0;
}

#if defined(PDCURSES) && defined(PDC_BUILD) && PDC_BUILD >= 2401
/* -scrsize */
static int scrsize_fun(char *next_arg)
{
    if (next_arg != 0) {
	char *cp;

	if ((cp = StrChr(next_arg, ',')) != 0) {
	    *cp++ = '\0';	/* Terminate ID */
	    scrsize_x = atoi(next_arg);
	    scrsize_y = atoi(cp);
	    if ((scrsize_x <= 1) || (scrsize_y <= 1)) {
		scrsize_x = scrsize_y = 0;
	    }
	    if ((scrsize_x > 0) && (scrsize_x < 40)) {
		scrsize_x = 40;
	    }
	    if ((scrsize_y > 0) && (scrsize_y < 6)) {
		scrsize_y = 6;
	    }
	    CTRACE((tfp, "scrsize: x=%d, y=%d\n", scrsize_x, scrsize_y));
	}
    }
    return 0;
}
#endif

/* NOTE: This table is sorted by name to make the help message useful */
/* *INDENT-OFF* */
static Config_Type Arg_Table [] =
{
   PARSE_SET(
      "accept_all_cookies", 4|SET_ARG,		LYAcceptAllCookies,
      "\naccept cookies without prompting if Set-Cookie handling\nis on"
   ),
#if USE_BLAT_MAILER
   PARSE_SET(
      "altblat",	4|TOGGLE_ARG,		mail_is_altblat,
      "select mail tool (`"THIS_BLAT_MAIL"' ==> `"THAT_BLAT_MAIL"')"
   ),
#endif
   PARSE_FUN(
      "anonymous",	2|FUNCTION_ARG,		anonymous_fun,
      "apply restrictions for anonymous account,\nsee also -restrictions"
   ),
   PARSE_FUN(
      "assume_charset", 4|NEED_FUNCTION_ARG,	assume_charset_fun,
      "=MIMEname\ncharset for documents that don't specify it"
   ),
   PARSE_FUN(
      "assume_local_charset", 4|NEED_FUNCTION_ARG, assume_local_charset_fun,
      "=MIMEname\ncharset assumed for local files"
   ),
   PARSE_FUN(
      "assume_unrec_charset", 4|NEED_FUNCTION_ARG, assume_unrec_charset_fun,
      "=MIMEname\nuse this instead of unrecognized charsets"
   ),
   PARSE_FUN(
      "auth",		4|NEED_FUNCTION_ARG,	auth_fun,
      "=id:pw\nauthentication information for protected documents"
   ),
   PARSE_FUN(
      "base",		4|FUNCTION_ARG,		base_fun,
      "prepend a request URL comment and BASE tag to " STR_HTML "\n\
outputs for -source dumps"
   ),
#ifndef DISABLE_BIBP
   PARSE_STR(
      "bibhost",	4|NEED_LYSTRING_ARG,	BibP_bibhost,
      "=URL\nlocal bibp server (default http://bibhost/)"
   ),
#endif
#ifdef USE_BLINK
   PARSE_SET(
      "blink",		4|SET_ARG,		term_blink_is_boldbg,
      "enable bright background via the BLINK terminal attribute"
   ),
#endif
   PARSE_SET(
      "book",		4|SET_ARG,		bookmark_start,
      "use the bookmark page as the startfile"
   ),
   PARSE_SET(
      "buried_news",	4|TOGGLE_ARG,		scan_for_buried_news_references,
      "toggles scanning of news articles for buried references"
   ),
   PARSE_FUN(
      "cache",		4|NEED_FUNCTION_ARG,	cache_fun,
      "=NUMBER\nNUMBER of documents cached in memory"
   ),
   PARSE_SET(
      "case",		4|SET_ARG,		LYcase_sensitive,
      "enable case sensitive user searching"
   ),
   PARSE_SET(
      "center",		4|TOGGLE_ARG,		no_table_center,
      "toggle center alignment in HTML TABLE"
   ),
   PARSE_STR(
      "cfg",		2|NEED_LYSTRING_ARG,	lynx_cfg_file,
      "=FILENAME\nspecifies a lynx.cfg file other than the default"
   ),
   PARSE_FUN(
      "child",		4|FUNCTION_ARG,		child_fun,
      "exit on left-arrow in startfile, and disable save to disk"
   ),
   PARSE_FUN(
      "child_relaxed",	4|FUNCTION_ARG,		child_relaxed_fun,
      "exit on left-arrow in startfile (allows save to disk)"
   ),
#ifdef USE_CMD_LOGGING
   PARSE_STR(
      "cmd_log",	2|NEED_LYSTRING_ARG,	lynx_cmd_logfile,
      "=FILENAME\nlog keystroke commands to the given file"
   ),
   PARSE_STR(
      "cmd_script",	2|NEED_LYSTRING_ARG,	lynx_cmd_script,
      "=FILENAME\nread keystroke commands from the given file\n(see -cmd_log)"
   ),
#endif
   PARSE_SET(
      "collapse_br_tags", 4|TOGGLE_ARG,		LYCollapseBRs,
      "toggles collapsing of BR tags"
   ),
#ifdef USE_SLANG
   PARSE_FUN(
      "color",		4|FUNCTION_ARG,		color_fun,
      "force color mode on with standard bg colors"
   ),
#endif
   PARSE_INT(
      "connect_timeout", 4|NEED_INT_ARG,	connect_timeout,
      "=N\nset the N-second connection timeout"
   ),
   PARSE_FUN(
      "convert_to",	4|FUNCTION_ARG,		convert_to_fun,
      "=FORMAT\nconvert input, FORMAT is in MIME type notation\n(experimental)"
   ),
#ifdef USE_PERSISTENT_COOKIES
   PARSE_STR(
      "cookie_file",	4|LYSTRING_ARG,		LYCookieFile,
      "=FILENAME\nspecifies a file to use to read cookies"
   ),
   PARSE_STR(
      "cookie_save_file", 4|LYSTRING_ARG,	LYCookieSaveFile,
      "=FILENAME\nspecifies a file to use to store cookies"
   ),
#endif /* USE_PERSISTENT_COOKIES */
   PARSE_SET(
      "cookies",	4|TOGGLE_ARG,		LYSetCookies,
      "toggles handling of Set-Cookie headers"
   ),
#ifndef VMS
   PARSE_SET(
      "core",		4|TOGGLE_ARG,		LYNoCore,
      "toggles forced core dumps on fatal errors"
   ),
#endif
   PARSE_FUN(
      "crawl",		4|FUNCTION_ARG,		crawl_fun,
      "with -traversal, output each page to a file\n\
with -dump, format output as with -traversal, but to stdout"
   ),
#ifdef USE_CURSES_PADS
   PARSE_SET(
      "curses_pads",	4|TOGGLE_ARG,		LYuseCursesPads,
      "uses curses pad feature to support left/right shifting"
   ),
#endif
#ifdef DISP_PARTIAL
   PARSE_SET(
      "debug_partial",	4|TOGGLE_ARG,		debug_display_partial,
      "incremental display stages with MessageSecs delay"
   ),
#endif
#ifdef USE_DEFAULT_COLORS
   PARSE_SET(
      "default_colors",	4|TOGGLE_ARG,		LYuse_default_colors,
      "use terminal default foreground/background colors"
   ),
#endif
   PARSE_INT(
      "delay",		4|NEED_TIME_ARG,	DelaySecs,
      "=NNN\nset NNN-second delay at statusline message"
   ),
   PARSE_FUN(
      "display",	4|NEED_FUNCTION_ARG,	display_fun,
      "=DISPLAY\nset the display variable for X exec'ed programs"
   ),
   PARSE_FUN(
      "display_charset", 4|NEED_FUNCTION_ARG,	display_charset_fun,
      "=MIMEname\ncharset for the terminal output"
   ),
   PARSE_SET(
      "dont_wrap_pre",	4|SET_ARG,		dont_wrap_pre,
      "inhibit wrapping of text in <pre> when -dump'ing and\n\
-crawl'ing, mark wrapped lines in interactive session"
   ),
   PARSE_FUN(
      "dump",		1|FUNCTION_ARG,		dump_output_fun,
      "dump the first file to stdout and exit"
   ),
   PARSE_FUN(
      "editor",		4|NEED_FUNCTION_ARG,	editor_fun,
      "=EDITOR\nenable edit mode with specified editor"
   ),
   PARSE_SET(
      "emacskeys",	4|SET_ARG,		emacs_keys,
      "enable emacs-like key movement"
   ),
   PARSE_SET(
      "enable_scrollback", 4|TOGGLE_ARG,	enable_scrollback,
      "\ntoggles compatibility with comm programs' scrollback\n\
keys (may be incompatible with some curses packages)"
   ),
   PARSE_FUN(
      "error_file",	4|NEED_FUNCTION_ARG,	error_file_fun,
      "=FILE\nwrite the HTTP status code here"
   ),
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
#ifndef NEVER_ALLOW_REMOTE_EXEC
   PARSE_FUN(
      "exec",		4|FUNCTION_ARG,		exec_fun,
      "enable local program execution"
   ),
#endif
#endif /* EXEC_LINKS || EXEC_SCRIPTS */
#ifdef VMS
   PARSE_SET(
      "fileversions",	4|SET_ARG,		HTVMSFileVersions,
      "include all versions of files in local VMS directory\nlistings"
   ),
#endif
#ifdef LY_FIND_LEAKS
   PARSE_SET(
      "find_leaks",	4|TOGGLE_ARG,		LYfind_leaks,
      "toggles memory-leak checking"
   ),
#endif
   PARSE_SET(
      "force_empty_hrefless_a",	4|SET_ARG,	force_empty_hrefless_a,
      "\nforce HREF-less 'A' elements to be empty (close them as\n\
soon as they are seen)"
   ),
   PARSE_SET(
      "force_html",	4|SET_ARG,		LYforce_HTML_mode,
      "forces the first document to be interpreted as HTML"
   ),
   PARSE_SET(
      "force_secure",	4|TOGGLE_ARG,		LYForceSSLCookiesSecure,
      "toggles forcing of the secure flag for SSL cookies"
   ),
#if !defined(NO_OPTION_FORMS) && !defined(NO_OPTION_MENU)
   PARSE_SET(
      "forms_options",	4|TOGGLE_ARG,		LYUseFormsOptions,
      "toggles forms-based vs old-style options menu"
   ),
#endif
   PARSE_SET(
      "from",		4|TOGGLE_ARG,		LYNoFromHeader,
      "toggle transmission of From headers"
   ),
#ifndef DISABLE_FTP
   PARSE_SET(
      "ftp",		4|UNSET_ARG,		ftp_ok,
      "disable ftp access"
   ),
#endif
   PARSE_FUN(
      "get_data",	2|FUNCTION_ARG,		get_data_fun,
      "user data for get forms, read from stdin,\nterminated by '---' on a line"
   ),
   PARSE_SET(
      "head",		4|SET_ARG,		HEAD_request,
      "send a HEAD request"
   ),
   PARSE_FUN(
      "help",		4|FUNCTION_ARG,		help_fun,
      "print this usage message"
   ),
   PARSE_FUN(
      "hiddenlinks",	4|NEED_FUNCTION_ARG,	hiddenlinks_fun,
      "=[option]\nhidden links: options are merge, listonly, or ignore"
   ),
   PARSE_SET(
      "historical",	4|TOGGLE_ARG,		historical_comments,
      "toggles use of '>' or '-->' as terminator for comments"
   ),
   PARSE_FUN(
      "homepage",	4|NEED_FUNCTION_ARG,	homepage_fun,
      "=URL\nset homepage separate from start page"
   ),
   PARSE_SET(
      "html5_charsets",	4|TOGGLE_ARG,		html5_charsets,
      "toggles use of HTML5 charset replacements"
   ),
   PARSE_SET(
      "image_links",	4|TOGGLE_ARG,		clickable_images,
      "toggles inclusion of links for all images"
   ),
   PARSE_STR(
      "index",		4|NEED_LYSTRING_ARG,	indexfile,
      "=URL\nset the default index file to URL"
   ),
   PARSE_SET(
      "ismap",		4|TOGGLE_ARG,		LYNoISMAPifUSEMAP,
      "toggles inclusion of ISMAP links when client-side\nMAPs are present"
   ),
#ifdef USE_JUSTIFY_ELTS
   PARSE_SET(
      "justify",	4|SET_ARG,		ok_justify,
      "do justification of text"
   ),
#endif
   PARSE_INT(
      "link",		4|NEED_INT_ARG,		crawl_count,
      "=NUMBER\nstarting count for lnk#.dat files produced by -crawl"
   ),
   PARSE_SET(
      "list_inline",	4|TOGGLE_ARG,		dump_links_inline,
      "with -dump, forces it to show links inline with text"
   ),
   PARSE_SET(
      "listonly",	4|TOGGLE_ARG,		dump_links_only,
      "with -dump, forces it to show only the list of links"
   ),
   PARSE_SET(
      "localhost",	4|SET_ARG,		local_host_only,
      "disable URLs that point to remote hosts"
   ),
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
   PARSE_SET(
      "locexec",	4|SET_ARG,		local_exec_on_local_files,
      "enable local program execution from local files only"
   ),
#endif /* EXEC_LINKS || EXEC_SCRIPTS */
#if defined(USE_COLOR_STYLE)
   PARSE_STR(
      "lss",		2|NEED_LYSTRING_ARG,	lynx_lss_file2,
      "=FILENAME\nspecifies a lynx.lss file other than the default"
   ),
#endif
   PARSE_FUN(
      "mime_header",	4|FUNCTION_ARG,		mime_header_fun,
      "include mime headers and force source dump"
   ),
   PARSE_SET(
      "minimal",	4|TOGGLE_ARG,		minimal_comments,
      "toggles minimal versus valid comment parsing"
   ),
#ifdef EXP_NESTED_TABLES
   PARSE_SET(
      "nested_tables",	4|TOGGLE_ARG,		nested_tables,
      "toggles nested-tables logic"
   ),
#endif
#ifndef DISABLE_NEWS
   PARSE_FUN(
      "newschunksize",	4|NEED_FUNCTION_ARG,	newschunksize_fun,
      "=NUMBER\nnumber of articles in chunked news listings"
   ),
   PARSE_FUN(
      "newsmaxchunk",	4|NEED_FUNCTION_ARG,	newsmaxchunk_fun,
      "=NUMBER\nmaximum news articles in listings before chunking"
   ),
#endif
#if USE_BLAT_MAILER
   PARSE_SET(
      "noblat",		4|TOGGLE_ARG,		mail_is_blat,
      "select mail tool (`"THIS_BLAT_MAIL"' ==> `"SYSTEM_MAIL"')"
   ),
#endif
   PARSE_FUN(
      "nobold",		4|FUNCTION_ARG,		nobold_fun,
      "disable bold video-attribute"
   ),
   PARSE_FUN(
      "nobrowse",	4|FUNCTION_ARG,		nobrowse_fun,
      "disable directory browsing"
   ),
   PARSE_SET(
      "nocc",		4|SET_ARG,		LYNoCc,
      "disable Cc: prompts for self copies of mailings"
   ),
   PARSE_FUN(
      "nocolor",	4|FUNCTION_ARG,		nocolor_fun,
      "turn off color support"
   ),
#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)
   PARSE_SET(
      "noexec",		4|UNSET_ARG,		local_exec,
      "disable local program execution (DEFAULT)"
   ),
#endif /* EXEC_LINKS || EXEC_SCRIPTS */
   PARSE_SET(
      "nofilereferer",	4|SET_ARG,		no_filereferer,
      "disable transmission of Referer headers for file URLs"
   ),
   PARSE_SET(
      "nolist",		4|SET_ARG,		no_list,
      "disable the link list feature in dumps"
   ),
   PARSE_SET(
      "nolog",		4|UNSET_ARG,		error_logging,
      "disable mailing of error messages to document owners"
   ),
   PARSE_SET(
      "nomargins",	4|SET_ARG,		no_margins,
      "disable the right/left margins in the default\nstyle-sheet"
   ),
   PARSE_FUN(
      "nomore",		4|FUNCTION_ARG,		nomore_fun,
      "disable -more- string in statusline messages"
   ),
#if defined(HAVE_SIGACTION) && defined(SIGWINCH)
   PARSE_SET(
      "nonrestarting_sigwinch", 4|SET_ARG,	LYNonRestartingSIGWINCH,
      "\nmake window size change handler non-restarting"
   ),
#endif /* HAVE_SIGACTION */
   PARSE_SET(
      "nonumbers",	4|SET_ARG,		no_numbers,
      "disable the link/form numbering feature in dumps"
   ),
   PARSE_FUN(
      "nopause",	4|FUNCTION_ARG,		nopause_fun,
      "disable forced pauses for statusline messages"
   ),
   PARSE_SET(
      "noprint",	4|SET_ARG,		no_print,
      "disable some print functions, like -restrictions=print"
   ),
   PARSE_SET(
      "noredir",	4|SET_ARG,		no_url_redirection,
      "don't follow Location: redirection"
   ),
   PARSE_SET(
      "noreferer",	4|SET_ARG,		LYNoRefererHeader,
      "disable transmission of Referer headers"
   ),
   PARSE_FUN(
      "noreverse",	4|FUNCTION_ARG,		noreverse_fun,
      "disable reverse video-attribute"
   ),
#ifdef SOCKS
   PARSE_SET(
      "nosocks",	2|UNSET_ARG,		socks_flag,
      "don't use SOCKS proxy for this session"
   ),
#endif
   PARSE_SET(
      "nostatus",	4|SET_ARG,		no_statusline,
      "disable the miscellaneous information messages"
   ),
   PARSE_SET(
      "notitle",	4|SET_ARG,		no_title,
      "disable the title at the top of each page"
   ),
   PARSE_FUN(
      "nounderline",	4|FUNCTION_ARG,		nounderline_fun,
      "disable underline video-attribute"
   ),
   PARSE_FUN(
      "nozap",		4|FUNCTION_ARG,		nozap_fun,
      "=DURATION (\"initially\" or \"full\") disable checks for 'z' key"
   ),
   PARSE_SET(
      "number_fields",	4|SET_ARG,		number_fields,
      "force numbering of links as well as form input fields"
   ),
   PARSE_SET(
      "number_links",	4|SET_ARG,		number_links,
      "force numbering of links"
   ),
#ifdef DISP_PARTIAL
   PARSE_SET(
      "partial",	4|TOGGLE_ARG,		display_partial_flag,
      "toggles display partial pages while downloading"
   ),
   PARSE_INT(
      "partial_thres",	4|NEED_INT_ARG,		partial_threshold,
      "[=NUMBER]\nnumber of lines to render before repainting display\n\
with partial-display logic"
   ),
#endif
#ifndef DISABLE_FTP
   PARSE_SET(
      "passive_ftp",	4|TOGGLE_ARG,		ftp_passive,
      "toggles passive ftp connection"
   ),
#endif
   PARSE_FUN(
      "pauth",		4|NEED_FUNCTION_ARG,	pauth_fun,
      "=id:pw\nauthentication information for protected proxy server"
   ),
   PARSE_SET(
      "popup",		4|UNSET_ARG,		LYUseDefSelPop,
      "toggles handling of single-choice SELECT options via\n\
popup windows or as lists of radio buttons"
   ),
   PARSE_FUN(
      "post_data",	2|FUNCTION_ARG,		post_data_fun,
      "user data for post forms, read from stdin,\n\
terminated by '---' on a line"
   ),
   PARSE_SET(
      "preparsed",	4|SET_ARG,		LYPreparsedSource,
      "show parsed " STR_HTML " with -source and in source view\n\
to visualize how lynx behaves with invalid HTML"
   ),
#ifdef USE_PRETTYSRC
   PARSE_SET(
      "prettysrc",	4|SET_ARG,		LYpsrc,
      "do syntax highlighting and hyperlink handling in source\nview"
   ),
#endif
   PARSE_SET(
      "print",		4|UNSET_ARG,		no_print,
      "enable print functions (DEFAULT), opposite of -noprint"
   ),
   PARSE_SET(
      "pseudo_inlines", 4|TOGGLE_ARG,		pseudo_inline_alts,
      "toggles pseudo-ALTs for inlines with no ALT string"
   ),
   PARSE_SET(
      "raw",		4|UNSET_ARG,		LYUseDefaultRawMode,
      "toggles default setting of 8-bit character translations\n\
or CJK mode for the startup character set"
   ),
   PARSE_SET(
      "realm",		4|SET_ARG,		check_realm,
      "restricts access to URLs in the starting realm"
   ),
   PARSE_INT(
      "read_timeout",	4|NEED_INT_ARG,		reading_timeout,
      "=N\nset the N-second read-timeout"
   ),
   PARSE_SET(
      "reload",		4|SET_ARG,		reloading,
      "flushes the cache on a proxy server\n(only the first document affected)"
   ),
   PARSE_FUN(
      "restrictions",	4|FUNCTION_ARG,		restrictions_fun,
      "=[options]\nuse -restrictions to see list"
   ),
   PARSE_SET(
      "resubmit_posts", 4|TOGGLE_ARG,		LYresubmit_posts,
      "toggles forced resubmissions (no-cache) of forms with\n\
method POST when the documents they returned are sought\n\
with the PREV_DOC command or from the History List"
   ),
   PARSE_SET(
      "rlogin",		4|UNSET_ARG,		rlogin_ok,
      "disable rlogins"
   ),
#if defined(PDCURSES) && defined(PDC_BUILD) && PDC_BUILD >= 2401
   PARSE_FUN(
      "scrsize",	4|NEED_FUNCTION_ARG,	scrsize_fun,
      "=width,height\nsize of window"
   ),
#endif
#ifdef USE_SCROLLBAR
   PARSE_SET(
      "scrollbar",	4|TOGGLE_ARG,		LYShowScrollbar,
      "toggles showing scrollbar"
   ),
   PARSE_SET(
      "scrollbar_arrow", 4|TOGGLE_ARG,		LYsb_arrow,
      "toggles showing arrows at ends of the scrollbar"
   ),
#endif
   PARSE_FUN(
      "selective",	4|FUNCTION_ARG,		selective_fun,
      "require .www_browsable files to browse directories"
   ),
#ifdef USE_SESSIONS
   PARSE_STR(
      "session",	2|NEED_LYSTRING_ARG,	session_file,
      "=FILENAME\nresumes from specified file on startup and\n\
saves session to that file on exit"
   ),
   PARSE_STR(
      "sessionin",	2|NEED_LYSTRING_ARG,	sessionin_file,
      "=FILENAME\nresumes session from specified file"
   ),
   PARSE_STR(
      "sessionout",	2|NEED_LYSTRING_ARG,	sessionout_file,
      "=FILENAME\nsaves session to specified file"
   ),
#endif /* USE_SESSIONS */
   PARSE_SET(
      "short_url",	4|SET_ARG,		long_url_ok,
      "enables examination of beginning and end of long URL in\nstatus line"
   ),
   PARSE_SET(
      "show_cfg",	1|SET_ARG,		show_cfg,
      "Show `LYNX.CFG' setting"
   ),
   PARSE_SET(
      "show_cursor",	4|TOGGLE_ARG,		LYUseDefShoCur,
      "toggles hiding of the cursor in the lower right corner"
   ),
#ifdef USE_READPROGRESS
   PARSE_SET(
      "show_rate",	4|TOGGLE_ARG,		LYShowTransferRate,
      "toggles display of transfer rate"
   ),
#endif
   PARSE_SET(
      "soft_dquotes",	4|TOGGLE_ARG,		soft_dquotes,
      "toggles emulation of the old Netscape and Mosaic\n\
bug which treated '>' as a co-terminator for\ndouble-quotes and tags"
   ),
   PARSE_FUN(
      "source",		4|FUNCTION_ARG,		source_fun,
      "dump the source of the first file to stdout and exit"
   ),
   PARSE_SET(
      "stack_dump",	4|SET_ARG,		stack_dump,
      "disable SIGINT cleanup handler"
   ),
   PARSE_SET(
      "startfile_ok",	4|SET_ARG,		startfile_ok,
      "allow non-http startfile and homepage with -validate"
   ),
   PARSE_SET(
      "stderr",		4|SET_ARG,		dump_to_stderr,
      "write warning messages to standard error when -dump\nor -source is used"
   ),
   PARSE_SET(
      "stdin",		4|SET_ARG,		startfile_stdin,
      "read startfile from standard input"
   ),
#ifdef SYSLOG_REQUESTED_URLS
   PARSE_STR(
      "syslog",		4|NEED_LYSTRING_ARG,	syslog_txt,
      "=text\ninformation for syslog call"
   ),
   PARSE_SET(
      "syslog_urls",	4|SET_ARG,		syslog_requested_urls,
      "log requested URLs with syslog"
   ),
#endif
   PARSE_SET(
      "tagsoup",	4|SET_ARG,		DTD_recovery,
      "use TagSoup rather than SortaSGML parser"
   ),
   PARSE_SET(
      "telnet",		4|UNSET_ARG,		telnet_ok,
      "disable telnets"
   ),
   PARSE_STR(
      "term",		4|NEED_STRING_ARG,	terminal,
      "=TERM\nset terminal type to TERM"
   ),
#ifdef _WINDOWS
   PARSE_INT(
      "timeout",	4|INT_ARG,		lynx_timeout,
      "=NUMBER\nset TCP/IP timeout"
   ),
#endif
   PARSE_SET(
      "tlog",		2|TOGGLE_ARG,		LYUseTraceLog,
      "toggles use of a Lynx Trace Log for the current\nsession"
   ),
#ifdef TEXTFIELDS_MAY_NEED_ACTIVATION
   PARSE_SET(
      "tna",		4|SET_ARG,		textfields_activation_option,
      "turn on \"Textfields Need Activation\" mode"
   ),
#endif
#ifndef NO_LYNX_TRACE
   PARSE_SET(
      "trace",		1|SET_ARG,		WWW_TraceFlag,
      "turns on Lynx trace mode"
   ),
   PARSE_INT(
      "trace_mask",	1|INT_ARG,		WWW_TraceMask,
      "customize Lynx trace mode"
   ),
#endif
   PARSE_FUN(
      "traversal",	4|FUNCTION_ARG,		traversal_fun,
      "traverse all http links derived from startfile"
   ),
   PARSE_SET(
      "trim_blank_lines", 2|TOGGLE_ARG,		LYtrimBlankLines,
      "\ntoggle trimming of leading/trailing/collapsed-br blank lines"
   ),
   PARSE_SET(
      "trim_input_fields", 2|SET_ARG,		LYtrimInputFields,
      "\ntrim input text/textarea fields in forms"
   ),
   PARSE_SET(
      "underline_links",4|TOGGLE_ARG,		LYUnderlineLinks,
      "toggles use of underline/bold attribute for links"
   ),
   PARSE_SET(
      "underscore",	4|TOGGLE_ARG,		use_underscore,
      "toggles use of _underline_ format in dumps"
   ),
   PARSE_SET(
      "unique_urls",	4|TOGGLE_ARG,		unique_urls,
      "toggles use of unique-urls setting for -dump and -listonly options"
   ),
#if defined(USE_MOUSE)
   PARSE_SET(
      "use_mouse",	4|SET_ARG,		LYUseMouse,
      "turn on mouse support"
   ),
#endif
   PARSE_STR(
      "useragent",	4|NEED_LYSTRING_ARG,	LYUserAgent,
      "=Name\nset alternate Lynx User-Agent header"
   ),
   PARSE_SET(
      "validate",	2|SET_ARG,		LYValidate,
      "accept only http URLs (meant for validation)\n\
implies more restrictions than -anonymous, but\n\
goto is allowed for http and https"
   ),
   PARSE_SET(
      "verbose",	4|TOGGLE_ARG,		verbose_img,
      "toggles [LINK], [IMAGE] and [INLINE] comments\n\
with filenames of these images"
   ),
   PARSE_FUN(
      "version",	1|FUNCTION_ARG,		version_fun,
      "print Lynx version information"
   ),
   PARSE_SET(
      "vikeys",		4|SET_ARG,		vi_keys,
      "enable vi-like key movement"
   ),
#ifdef __DJGPP__
   PARSE_SET(
      "wdebug",		4|TOGGLE_ARG,		watt_debug,
      "enables Waterloo tcp/ip packet debug. Prints to watt\ndebugfile"
  ),
#endif /* __DJGPP__ */
   PARSE_FUN(
      "width",		4|NEED_FUNCTION_ARG,	width_fun,
      "=NUMBER\nscreen width for formatting of dumps (default is 80)"
   ),
#ifndef NO_DUMP_WITH_BACKSPACES
   PARSE_SET(
      "with_backspaces", 4|SET_ARG,		with_backspaces,
      "emit backspaces in output if -dumping or -crawling\n(like 'man' does)"
   ),
#endif
   PARSE_SET(
      "xhtml_parsing",	4|SET_ARG,		LYxhtml_parsing,
      "enable XHTML 1.0 parsing"
   ),
   PARSE_NIL
};
/* *INDENT-ON* */

static void print_help_strings(const char *name,
			       const char *help,
			       const char *value,
			       int option)
{
    int pad;
    int c;
    int first;
    int field_width = 20;

    pad = field_width - (2 + option + (int) strlen(name));

    fprintf(stdout, "  %s%s", option ? "-" : "", name);

    if (*help != '=') {
	pad--;
	while (pad > 0) {
	    fputc(' ', stdout);
	    pad--;
	}
	fputc(' ', stdout);	/* at least one space */
	first = 0;
    } else {
	first = pad;
    }

    if (StrChr(help, '\n') == 0) {
	fprintf(stdout, "%s", help);
    } else {
	while ((c = *help) != 0) {
	    if (c == '\n') {
		if ((pad = --first) < 0) {
		    pad = field_width;
		} else {
		    c = ' ';
		}
		fputc(c, stdout);
		while (pad--)
		    fputc(' ', stdout);
	    } else {
		fputc(c, stdout);
	    }
	    help++;
	    first--;
	}
    }
    if (value)
	printf(" (%s)", value);
    fputc('\n', stdout);
}

static void print_help_and_exit(int exit_status)
{
    Config_Type *p;

    if (pgm == NULL)
	pgm = "lynx";

    SetOutputMode(O_TEXT);

    fprintf(stdout, gettext("USAGE: %s [options] [file]\n"), pgm);
    fprintf(stdout, gettext("Options are:\n"));
#ifdef VMS
    print_help_strings("",
		       "receive the arguments from stdin (enclose\n\
in double-quotes (\"-\") on VMS)", NULL, TRUE);
#else
    print_help_strings("", "receive options and arguments from stdin", NULL, TRUE);
#endif /* VMS */

    for (p = Arg_Table; p->name != 0; p++) {
	char temp[LINESIZE], *value = temp;
	ParseUnionPtr q = ParseUnionOf(p);

	switch (p->type & ARG_TYPE_MASK) {
	case TOGGLE_ARG:
	case SET_ARG:
	    strcpy(temp, *(q->set_value) ? "on" : "off");
	    break;
	case UNSET_ARG:
	    strcpy(temp, *(q->set_value) ? "off" : "on");
	    break;
	case INT_ARG:
	    sprintf(temp, "%d", *(q->int_value));
	    break;
	case TIME_ARG:
	    sprintf(temp, SECS_FMT, (double) Secs2SECS(*(q->int_value)));
	    break;
	case STRING_ARG:
	    if ((value = *(q->str_value)) != 0
		&& !*value)
		value = 0;
	    break;
	default:
	    value = 0;
	    break;
	}
	print_help_strings(p->name, p->help_string, value, TRUE);
    }

    SetOutputMode(O_BINARY);

    exit_immediately(exit_status);
}

/*
 * This function performs a string comparison on two strings a and b.  a is
 * assumed to be an ordinary null terminated string, but b may be terminated
 * by an '=', '+' or '-' character.  If terminated by '=', *c will be pointed
 * to the character following the '='.  If terminated by '+' or '-', *c will
 * be pointed to that character.  (+/- added for toggle processing - BL.)
 * If a and b match, it returns 1.  Otherwise 0 is returned.
 */
static int arg_eqs_parse(const char *a,
			 char *b,
			 char **c)
{
    int result = -1;

    *c = NULL;
    while (result < 0) {
	if ((*a != *b)
	    || (*a == 0)
	    || (*b == 0)) {
	    if (*a == 0) {
		switch (*b) {
		case '\t':	/* embedded blank when reading stdin */
		case ' ':
		    *c = LYSkipBlanks(b);
		    result = 1;
		    break;
		case '=':
		case ':':
		    *c = b + 1;
		    result = 1;
		    break;
		case '-':
#if OPTNAME_ALLOW_DASHES
		    if (isalpha(UCH(b[1]))) {
			result = 0;
			break;
		    }
#endif
		    /* FALLTHRU */
		case '+':
		    *c = b;
		    result = 1;
		    break;
		case 0:
		    result = 1;
		    break;
		default:
		    result = 0;
		    break;
		}
	    } else {
#if OPTNAME_ALLOW_DASHES
		if (!(*a == '_' && *b == '-'))
#endif
		    result = 0;
	    }
	}
	a++;
	b++;
    }
    return result;
}

#define is_true(s)  (*s == '1' || *s == '+' || !strcasecomp(s, "on")  || !strcasecomp(s, "true"))
#define is_false(s) (*s == '0' || *s == '-' || !strcasecomp(s, "off") || !strcasecomp(s, "false"))

/*
 * Parse an option.
 *	argv[] points to the beginning of the unprocessed options.
 *	mask is used to select certain options which must be processed
 *		before others.
 *	countp (if nonnull) points to an index into argv[], which is updated
 *		to reflect option values which are also parsed.
 */
static BOOL parse_arg(char **argv,
		      unsigned mask,
		      int *countp)
{
    Config_Type *p;
    char *arg_name;

#if EXTENDED_STARTFILE_RECALL
    static BOOLEAN no_options_further = FALSE;	/* set to TRUE after '--' argument */
    static int nof_index = 0;	/* set the index of -- argument */
#endif

    arg_name = argv[0];
    CTRACE((tfp, "parse_arg(arg_name=%s, mask=%u, count=%d)\n",
	    arg_name, mask, countp ? *countp : -1));

#if EXTENDED_STARTFILE_RECALL
    if (mask == (unsigned) ((countp != 0) ? 0 : 1)) {
	no_options_further = FALSE;
	/* want to reset nonoption when beginning scan for --stdin */
	if (nonoption != 0) {
	    FREE(nonoption);
	}
    }
#endif

    /*
     * Check for a command line startfile.  - FM
     */
    if (*arg_name != '-'
#if EXTENDED_OPTION_LOGIC
	|| ((no_options_further == TRUE)
	    && (countp != 0)
	    && (nof_index < (*countp)))
#endif
	) {
#if EXTENDED_STARTFILE_RECALL
	/*
	 * On the last pass (mask==4), check for cases where we may want to
	 * provide G)oto history for multiple startfiles.
	 */
	if (mask == 4) {
	    if (nonoption != 0) {
		LYEnsureAbsoluteURL(&nonoption, "NONOPTION", FALSE);
		HTAddGotoURL(nonoption);
		FREE(nonoption);
	    }
	    StrAllocCopy(nonoption, arg_name);
	}
#endif
	StrAllocCopy(startfile, arg_name);
	LYEscapeStartfile(&startfile);
#ifdef _WINDOWS			/* 1998/01/14 (Wed) 20:11:17 */
	HTUnEscape(startfile);
	{
	    char *q = startfile;

	    while (*q++) {
		if (*q == '|')
		    *q = ':';
	    }
	}
#endif
	CTRACE((tfp, "parse_arg startfile:%s\n", startfile));
	return (BOOL) (countp != 0);
    }
#if EXTENDED_OPTION_LOGIC
    if (strcmp(arg_name, "--") == 0) {
	no_options_further = TRUE;
	nof_index = countp ? *countp : -1;
	return TRUE;
    }
#endif

    /* lose the first '-' character */
    arg_name++;

    /*
     * Skip any lone "-" arguments, because we've loaded the stdin input into
     * an HTList structure for special handling.  - FM
     */
    if (*arg_name == 0)
	return TRUE;

    /* allow GNU-style options with -- prefix */
    if (*arg_name == '-')
	++arg_name;

    CTRACE((tfp, "parse_arg lookup(%s)\n", arg_name));

    p = Arg_Table;
    while (p->name != 0) {
	ParseUnionPtr q = ParseUnionOf(p);
	ParseFunc fun;
	char *next_arg = NULL;
	char *temp_ptr = NULL;

	if ((p->name[0] != *arg_name)
	    || (0 == arg_eqs_parse(p->name, arg_name, &next_arg))) {
	    p++;
	    continue;
	}

	if (p->type & NEED_NEXT_ARG) {
	    if (next_arg == 0) {
		next_arg = argv[1];
		if ((countp != 0) && (next_arg != 0))
		    (*countp)++;
	    }
	    CTRACE((tfp, "...arg:%s\n", NONNULL(next_arg)));
	}

	/* ignore option if it's not our turn */
	if (((unsigned) (p->type) & mask) == 0) {
	    CTRACE((tfp, "...skip (mask %u/%d)\n", mask, p->type & 7));
	    return FALSE;
	}

	switch (p->type & ARG_TYPE_MASK) {
	case TOGGLE_ARG:	/* FALLTHRU */
	case SET_ARG:		/* FALLTHRU */
	case UNSET_ARG:
	    if (q->set_value != 0) {
		if (next_arg == 0) {
		    switch (p->type & ARG_TYPE_MASK) {
		    case TOGGLE_ARG:
			*(q->set_value) = (BOOL) !(*(q->set_value));
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
		if (-1 == (*fun) (next_arg)) {
		}
	    }
	    break;

	case LYSTRING_ARG:
	    if ((q->str_value != 0) && (next_arg != 0))
		StrAllocCopy(*(q->str_value), next_arg);
	    break;

	case INT_ARG:
	    if ((q->int_value != 0) && (next_arg != 0))
		*(q->int_value) = (int) strtol(next_arg, &temp_ptr, 0);
	    break;

	case TIME_ARG:
	    if ((q->int_value != 0) && (next_arg != 0)) {
		float ival;

		if (1 == LYscanFloat(next_arg, &ival)) {
		    *(q->int_value) = (int) SECS2Secs(ival);
		}
	    }
	    break;

	case STRING_ARG:
	    if ((q->str_value != 0) && (next_arg != 0))
		*(q->str_value) = next_arg;
	    break;
	}

	Old_DTD = DTD_recovery;	/* BOOL != int */
	return TRUE;
    }

    if (pgm == 0)
	pgm = "LYNX";

    fprintf(stderr, gettext("%s: Invalid Option: %s\n"), pgm, argv[0]);
    print_help_and_exit(-1);
    return FALSE;
}

#ifndef VMS
static void FatalProblem(int sig)
{
    /*
     * Ignore further interrupts.  - mhc:  11/2/91
     */
#ifndef NOSIGHUP
    (void) signal(SIGHUP, SIG_IGN);
#endif /* NOSIGHUP */
    (void) signal(SIGTERM, SIG_IGN);
    (void) signal(SIGINT, SIG_IGN);
#ifndef __linux__
#ifdef SIGBUS
    (void) signal(SIGBUS, SIG_IGN);
#endif /* ! SIGBUS */
#endif /* !__linux__ */
    (void) signal(SIGSEGV, SIG_IGN);
    (void) signal(SIGILL, SIG_IGN);

    /*
     * Flush all messages.  - FM
     */
    fflush(stderr);
    fflush(stdout);

    /*
     * Deal with curses, if on, and clean up.  - FM
     */
    if (LYOutOfMemory && LYCursesON) {
	LYSleepAlert();
    }
    cleanup_sig(0);
#ifndef __linux__
#ifdef SIGBUS
    signal(SIGBUS, SIG_DFL);
#endif /* SIGBUS */
#endif /* !__linux__ */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);

    /*
     * Issue appropriate messages and abort or exit.  - FM
     */
    if (LYOutOfMemory == FALSE) {
	fprintf(stderr, "\r\n\
A Fatal error has occurred in %s Ver. %s\r\n", LYNX_NAME, LYNX_VERSION);

	fprintf(stderr, "\r\n\
Please notify your system administrator to confirm a bug, and\r\n\
if confirmed, to notify the lynx-dev list.  Bug reports should\r\n\
have concise descriptions of the command and/or URL which causes\r\n\
the problem, the operating system name with version number, the\r\n\
TCPIP implementation, and any other relevant information.\r\n");

	if (!(sig == 0 && LYNoCore)) {
	    fprintf(stderr, "\r\n\
Do NOT mail the core file if one was generated.\r\n");
	}
	if (sig != 0) {
	    fprintf(stderr, "\r\n\
Lynx now exiting with signal:  %d\r\n\r\n", sig);
#ifdef WIN_EX			/* 1998/08/09 (Sun) 09:58:25 */
	    {
		char *msg;

		switch (sig) {
		case SIGABRT:
		    msg = "SIGABRT";
		    break;
		case SIGFPE:
		    msg = "SIGFPE";
		    break;
		case SIGILL:
		    msg = "SIGILL";
		    break;
		case SIGSEGV:
		    msg = "SIGSEGV";
		    break;
		default:
		    msg = "Not-def";
		    break;
		}
		fprintf(stderr, "signal code = %s\n", msg);
	    }
#endif
	}

	/*
	 * Exit and possibly dump core.
	 */
	if (LYNoCore) {
	    exit_immediately(EXIT_FAILURE);
	}
	abort();

    } else {
	LYOutOfMemory = FALSE;
	printf("\r\n%s\r\n\r\n", MEMORY_EXHAUSTED_ABORT);
	fflush(stdout);

	/*
	 * Exit without dumping core.
	 */
	exit_immediately(EXIT_FAILURE);
    }
}
#endif /* !VMS */
