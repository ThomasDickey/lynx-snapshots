/*
 * Lynx - Hypertext navigation system
 *
 *   (c) Copyright 1992, 1993, 1994 University of Kansas
 *	 1995, 1996: GNU General Public License
 */

/*******************************************************************
 * There are four sections to this document:
 *  Section 1.  Things you MUST change or verify
 *	Section 1a)  VMS specific things
 *	Section 1b)  UNIX specific things
 *	Section 1c)  ALL Platforms
 *
 *  Section 2.  Things you should probably check!
 *
 *  Section 3.  Things you should only change after you have a good
 *              understanding of the program!
 *
 *  Section 4.  Things you MUST check only if you plan to use Lynx in
 *              an anonymous account (allow public access to Lynx)!
 *
 */

#ifndef USERDEFS_H
#define USERDEFS_H

#ifdef HAVE_CONFIG_H
#include <lynx_cfg.h>
#endif

/*******************************************************************
 * Things you must change
 *  Section 1.
 */

/*******************************************************************
 * Things you must change  VMS specific
 *  Section 1a).
 */
#ifdef VMS
/**************************
 * TEMP_SPACE is where Lynx temporary cache files will be placed.
 * Temporary files are removed automatically as long as nothing
 * goes terribly wrong :)  If you include "$USER" in the definition
 * (e.g., "device:[dir.$USER]"), Lynx will replace the "$USER" with
 * the username of the account which invoked the Lynx image.  Such
 * directories should already exist, and have protections/ACLs set
 * so that only the appropriate user(s) will have read/write access.
 * On VMS, "sys$scratch:" defaults to "sys$login:" if it has not been
 * defined externally, or you can use "sys$login:" explicitly here.
 * If the path has SHELL syntax and includes a tilde (e.g, "~/lynxtmp"),
 * Lynx will replace the tilde with the full path for the user's home
 * and convert the result to VMS syntax.
 * The definition here can be overridden at run time by defining a
 * "LYNX_TEMP_SPACE" VMS logical.
 */
#define TEMP_SPACE "sys$scratch:"

/**************************
 * LYNX_CFG_FILE is the location and name of the default lynx
 * global configuration file.  It is sought and processed at
 * startup of Lynx, followed by a seek and processing of a
 * personal RC file (.lynxrc in the user's HOME directory,
 * created if the user saves values in the 'o'ptions menu).
 * You also can define the location and name of the global
 * configuration file via a VMS logical, "LYNX_CFG", which
 * will override the "LYNX_CFG_FILE" definition here.  SYS$LOGIN:
 * can be used as the device in either or both definitions if
 * you want lynx.cfg treated as a personal configuration file.
 * You also can use Unix syntax with a '~' for a subdirectory
 * of the login directory, (e.g., ~/lynx/lynx.cfg).
 * The -cfg command line switch will override these definitions.
 * You can pass the compilation default via build.com or descrip.mms.
 *
 * Note that some implementations of telnet allow passing of
 * environment variables, which might be used by unscrupulous
 * people to modify the environment in anonymous accounts.  When
 * making Lynx and Web access publically available via anonymous
 * accounts intended to run Lynx captively, be sure the wrapper
 * uses the -cfg switch and specifies the startfile, rather than
 * relying on the LYNX_CFG, LYNX_CFG_FILE, or WWW_HOME variables.
 *
 * Note that any SUFFIX or VIEWER mappings in the configuration
 * file will be overidden by any suffix or viewer mappings
 * that are established as defaults in src/HTInit.c.  You can
 * override the src/HTInit.c defaults via the mime.types and
 * mailcap files (see the examples in the samples directory).
 */
#ifndef LYNX_CFG_FILE
#define LYNX_CFG_FILE "Lynx_Dir:lynx.cfg"
#endif /* LYNX_CFG_FILE */

/**************************
 * The EXTENSION_MAP file allows you to map file suffix's to
 * mime types.
 * These global and personal files override anything in
 * lynx.cfg or src/HTInit.c
 */
#define GLOBAL_EXTENSION_MAP "Lynx_Dir:mime.types"
#define PERSONAL_EXTENSION_MAP "mime.types"

/**************************
 * The MAILCAP file allows you to map file MIME types to
 * external viewers.
 * These global and personal files override anything in
 * lynx.cfg or src/HTInit.c
 */
#define GLOBAL_MAILCAP "Lynx_Dir:mailcap"
#define PERSONAL_MAILCAP ".mailcap"

/**************************
 * XLOADIMAGE_COMMAND will be used as a default in src/HTInit.c
 * for viewing image content types when the DECW$DISPLAY logical
 * is set.  Make it the foreign command for your system's X image
 * viewer (commonly, "xv").  Make it "exit" or something like that
 * if you don't have one.  It can be anything that will handle GIF,
 * TIFF and other popular image formats.  Freeware ports of xv for
 * VMS are available in the ftp://ftp.wku.edu/vms/unsupported and
 * http://www.openvms.digital.com/cd/XV310A/ subdirectories.  You
 * must also have a "%s" for the filename.  The default defined
 * here can be overridden in lynx.cfg, or via the global or personal
 * mailcap files.
 */
#define XLOADIMAGE_COMMAND "xv %s"

/**************************
 * SYSTEM_MAIL must be defined here to your mail sending command,
 * and SYSTEM_MAIL_FLAGS to approrpriate qualifiers.  They can be
 * changed in lynx.cfg.
 *
 * The mail command will be spawned as a subprocess of lynx
 * and used to send the email, with headers specified in a
 * temporary file for PMDF.  If you define SYSTEM_MAIL to the
 * "generic" MAIL utility for VMS, headers cannot be specified
 * via a header file (and thus may not be included), and the
 * subject line will be specified by use of the /subject="SUBJECT"
 * qualifier.
 *
 * If your mailer uses another syntax, some hacking of the
 * mailform(), mailmsg() and reply_by_mail() functions in
 * LYMail.c, and printfile() function in LYPrint.c, may be
 * required.
 */
#define SYSTEM_MAIL "PMDF SEND"
#define SYSTEM_MAIL_FLAGS "/headers"
/* #define SYSTEM_MAIL "MAIL"   */
/* #define SYSTEM_MAIL_FLAGS "" */

/*************************
 * Below is the argument for an sprintf command that will add
 * "IN%""ADDRESS""" to the Internet mail address given by the user.
 * It is structured for PMDF's IN%"INTERNET_ADDRESS" scheme.  The %s
 * is replaced with the address given by the user.  If you are using
 * a different Internet mail transport, change the IN appropriately
 * (e.g., to SMTP, MX, or WINS), here or in lynx.cfg.
 */
#define MAIL_ADRS "\"IN%%\"\"%s\"\"\""

/*********************************
 * On VMS, CSwing (an XTree emulation for VTxxx terminals) is intended for
 * use as the Directory/File Manager (sources, objects, or executables are
 * available from ftp://narnia.memst.edu/).  CSWING_PATH should be defined
 * here or in lynx.cfg to your foreign command for CSwing, with any
 * regulatory switches you want included.  If not defined, or defined as
 * a zero-length string ("") or "none" (case-insensitive), the support
 * will be disabled.  It will also be disabled if the -nobrowse or
 * -selective switches are used, or if the file_url restriction is set.
 *
 * When enabled, the DIRED_MENU command (normally 'f' or 'F') will invoke
 * CSwing, normally with the current default directory as an argument to
 * position the user on that node of the directory tree.  However, if the
 * current document is a local directory listing, or a local file and not
 * one of the temporary menu or list files, the associated directory will
 * be passed as an argument, to position the user on that node of the tree.
 */
/* #define CSWING_PATH "swing" */

/*********************************
 * If USE_FIXED_RECORDS is set to TRUE here and/or in lynx.cfg, Lynx will
 * convert 'd'ownloaded binary files to FIXED 512 record format before saving
 * them to disk or acting on a DOWNLOADER option.  If set to FALSE, the
 * headers of such files will indicate that they are Stream_LF with Implied
 * Carriage Control, which is incorrect, and can cause downloading software
 * to get confused and unhappy.  If you do set it FALSE, you can use the
 * FIXED512.COM command file, which is included in this distribution, to do
 * the conversion externally.
 */
#define USE_FIXED_RECORDS	TRUE	/* convert binaries to FIXED 512 */

/********************************
 * If NO_ANONYMOUS_EMAIL is defined, Lynx will not offer to insert X-From
 * and X_Personal_Name lines in the body of email messages.  On VMS, the
 * actual From and Personal Name (if defined for the account) headers always
 * are those of the account running the Lynx image.  If the account is not
 * the one to which the recipient should reply, you can indicate the alternate
 * address and personal name via the X-From and X_Personal_Name entries, but
 * the recipient must explicitly send the reply to the X_From address, rather
 * than using the VMS REPLY command (which will use the actual From address).
 *
 * This symbol constant might be defined on Unix for security reasons that
 * don't apply on VMS.  There is no security reason for defining this on VMS,
 * but if you have no anonymous accounts (i.e., the From always will point to
 * the actual user's email address, you can define it to avoid the bother of
 * X-From and X_Personal_Name offers.
 */
/*#define NO_ANONYMOUS_EMAIL TRUE */

/**************************
 * LYNX_LSS_FILE is the location and name of the default lynx
 * character style sheet file.  It is sought and processed at
 * startup of Lynx only if experimental character style code has
 * been compiled in, otherwise it will be ignored.  Note that use
 * of the character style option is _experimental_ AND _unsupported_.
 * There is no documentation other than a sample lynx.lss file in
 * the samples subdirectory.  This code probably won't even work on
 * VMS.  You can define the location and name of this file via an
 * environment variable, "lynx_lss", which will override the definition
 * here.  You can use '~' to refer to the user's home directory.  The
 * -lss command line switch will override these definitions.
 */
#ifndef LYNX_LSS_FILE
#define LYNX_LSS_FILE "Lynx_Dir:lynx.lss"
#endif /* LYNX_LSS_FILE */

/*******************************************************************
 * Things you must change  UNIX specific
 *  Section 1b).
 */
#else     /* UNIX */

/**************************
 * NOTE: This variable is set by the configure scrip; editing changes will
 * be ignored.
 *
 * LYNX_CFG_FILE is the location and name of the default lynx
 * global configuration file.  It is sought and processed at
 * startup of Lynx, followed by a seek and processing of a
 * personal RC file (.lynxrc in the user's HOME directory,
 * created if the user saves values in the 'o'ptions menu).
 * You also can define the location and name of the global
 * configuration file via an environment variable, "LYNX_CFG",
 * which will override the "LYNX_CFG_FILE" definition here.
 * You can use '~' in either or both definitions if you want
 * lynx.cfg treated as a personal configuration file.  The
 * -cfg command line switch will override these definitions.
 * You can pass the compilation default via the Makefile.
 *
 * If you are building Lynx using the configure script, you should specify
 * the default location of the configuration file via that script, since it
 * also generates the makefile and install-cfg rules.
 *
 * Note that many implementations of telnetd allow passing of
 * environment variables, which might be used by unscrupulous
 * people to modify the environment in anonymous accounts.  When
 * making Lynx and Web access publically available via anonymous
 * accounts intended to run Lynx captively, be sure the wrapper
 * uses the -cfg switch and specifies the startfile, rather than
 * relying on the LYNX_CFG, LYNX_CFG_FILE, or WWW_HOME variables.
 *
 * Note that any SUFFIX or VIEWER mappings in the configuration
 * file will be overidden by any suffix or viewer mappings
 * that are established as defaults in src/HTInit.c.  You can
 * override the src/HTInit.c defaults via the mime.types and
 * mailcap files (see the examples in the samples directory).
 */
#ifndef HAVE_CONFIG_H
#ifndef LYNX_CFG_FILE
#ifdef DOSPATH
#define LYNX_CFG_FILE "./lynx.cfg"
#else
#define LYNX_CFG_FILE "/usr/local/lib/lynx.cfg"
#endif /* DOSPATH */
#endif /* LYNX_CFG_FILE */
#endif /* HAVE_CONFIG_H */

/**************************
 * The EXTENSION_MAP file allows you to map file suffix's to
 * mime types.
 * These global and personal files override anything in
 * lynx.cfg or src/HTInit.c
 */
#define GLOBAL_EXTENSION_MAP "/usr/local/lib/mosaic/mime.types"
#define PERSONAL_EXTENSION_MAP ".mime.types"

/**************************
 * The MAILCAP file allows you to map file MIME types to
 * external viewers.
 * These global and personal files override anything in
 * lynx.cfg or src/HTInit.c
 */
#define GLOBAL_MAILCAP "/usr/local/lib/mosaic/mailcap"
#define PERSONAL_MAILCAP ".mailcap"

/**************************
 * the full path and name of the telnet command
 */
#define TELNET_COMMAND "telnet"

/**************************
 * the full path and name of the tn3270 command
 */
#define TN3270_COMMAND "tn3270"

/**************************
 * the full path and name of the rlogin command
 */
#define RLOGIN_COMMAND "rlogin"

/**************************
 * XLOADIMAGE_COMMAND will be used as a default in src/HTInit.c for
 * viewing image content types when the DISPLAY environment variable
 * is set.  Make it the full path and name of the xli (also known as
 * xloadimage or xview) command, or other image viewer.  Put 'echo' or
 * something like it here if you don't have a suitable viewer.  It can
 * be anything that will handle GIF, TIFF and other popular image formats
 * (xli does).  The freeware distribution of xli is available in the
 * ftp://ftp.x.org/contrib/ subdirectory.  The shareware, xv, also is
 * suitable.  You must also have a "%s" for the filename; "&" for
 * background is optional.  The default defined here can be overridden
 * in lynx.cfg, or via the global or personal mailcap files.  Note that
 * open is used as the default for NeXT, instead of the XLOADIMAGE_COMMAND
 * definition.
 */
#define XLOADIMAGE_COMMAND "xli %s &"

/**************************
 * For UNIX systems, SYSTEM_MAIL and SYSTEM_MAIL_FLAGS are set by the
 * configure-script.
 */

/**************************
 * A place to put temporary files, it's almost always in "/tmp/"
 * for UNIX systems.  If you include "$USER" in the definition
 * (e.g., "/tmp/$USER"), Lynx will replace the "$USER" with the
 * username of the account which invoked the Lynx image.  Such
 * directories should already exist, and have protections/ACLs set
 * so that only the appropriate user(s) will have read/write access.
 * If the path includes a tilde (e.g, "~" or "~/lynxtmp"), Lynx will
 * replace the tilde with the full path for the user's home.
 * The definition here can be overridden at run time by setting a
 * "LYNX_TEMP_SPACE" environment symbol.
 */
#define TEMP_SPACE "/tmp/"

/********************************
 * Don't let the user enter his/her email address when sending a message.
 * Anonymous mail makes it far too easy for a user to spoof someone else's
 * email address.
 * This requires that your mailer agent put in the From: field for you.
 *
 * The default should be to uncomment this line but there probably are too
 * many mail agents out there that won't do the right thing if there is no
 * From: line.
 */
/* #define NO_ANONYMOUS_EMAIL TRUE */

/********************************
 * LIST_FORMAT defines the display for local files when LONG_LIST
 * is defined in the Makefile.  The default set here can be changed
 * in lynx.cfg.
 *
 * The percent items in the list are interpreted as follows:
 *
 *	%p	Unix-style permission bits
 *	%l	link count
 *	%o	owner of file
 *	%g	group of file
 *	%d	date of last modification
 *	%a	anchor pointing to file or directory
 *	%A	as above but don't show symbolic links
 *	%k	size of file in Kilobytes
 *	%K	as above but omit size for directories
 *	%s	size of file in bytes
 *
 * Anything between the percent and the letter is passed on to sprintf.
 * A double percent yields a literal percent on output.  Other characters
 * are passed through literally.
 *
 * If you want only the filename:  "    %a"
 *
 * If you want a brief output:     "    %4K %-12.12d %a"
 *
 * For the Unix "ls -l" format:    "    %p %4l %-8.8o %-8.8g %7s %-12.12d %a"
 */
#define LIST_FORMAT "    %p %4l %-8.8o %-8.8g %7s %-12.12d %a"

/*
 *  If NO_FORCED_CORE_DUMP is set to TRUE, Lynx will not force
 *  core dumps via abort() calls on fatal errors or assert()
 *  calls to check potentially fatal errors.  The default defined
 *  here can be changed in lynx.cfg, and the compilation or
 *  configuration default can be toggled via the -core command
 *  line switch.
 */
#define NO_FORCED_CORE_DUMP	FALSE

/**************************
 * LYNX_LSS_FILE is the location and name of the default lynx
 * character style sheet file.  It is sought and processed at
 * startup of Lynx only if experimental character style code
 * has been compiled in, otherwise it will be ignored.  Note
 * that use of the character style option is _experimental_ AND
 * _unsupported_.  There is no documentation other than a sample
 * lynx.lss file in the samples subdirectory.  You also can
 * define the location and name of this file via environment
 * variables "LYNX_LSS" or "lynx_lss" which will override the
 * "LYNX_LSS_FILE" definition here.  You can use '~' in either or
 * both definitions to refer to the user's home directory.  The
 * -lss command line switch will override these definitions.
 */
#ifndef LYNX_LSS_FILE
#define LYNX_LSS_FILE "/usr/local/lib/lynx.lss"
#endif /* LYNX_LSS_FILE */

#endif /* VMS OR UNIX */

/*************************************************************
 *  Section 1c)   Every platform must change or verify these
 *
 */

/*****************************
 * STARTFILE is the default file if none is specified in lynx.cfg,
 *  on the command line, or via a WWW_HOME environment variable.
 *
 * note: STARTFILE must be a URL.  See the Lynx online help for more
 *       information on URLs
 */
#define STARTFILE "http://lynx.browser.org/"

/*****************************
 * HELPFILE must be defined as a URL and must have a
 * complete path if local:
 * file://localhost/PATH_TO/lynx_help/lynx_help_main.html
 *   Replace PATH_TO with the path to the lynx_help subdirectory
 *   for this distribution (use SHELL syntax including the device
 *   on VMS systems).
 * The default HELPFILE is:
 * http://www.crl.com/~subir/lynx/lynx_help/lynx_help_main.html
 *   This should be changed here or in lynx.cfg to the local path.
 */
#define HELPFILE "http://www.crl.com/~subir/lynx/lynx_help/lynx_help_main.html"
/* #define HELPFILE "file://localhost/PATH_TO/lynx_help/lynx_help_main.html" */

/*****************************
 * DEFAULT_INDEX_FILE is the default file retrieved when the
 * user presses the 'I' key when viewing any document.
 * An index to your CWIS can be placed here or a document containing
 * pointers to lots of interesting places on the web.
 */
#define DEFAULT_INDEX_FILE "http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/MetaIndex.html"

/*****************************
 * If USE_TRACE_LOG is set FALSE, then when TRACE mode is invoked the
 * syserr messages will not be directed to a log file named Lynx.trace
 * in the account's HOME directory.  The default defined here can be
 * toggled via the -tlog command line switch.  Also, it is set FALSE
 * automatically when Lynx is executed in an anonymous or validation
 * account (if indicated via the -anonymous or -validate command line
 * switches, or via the check for the ANONYMOUS_USER, defined below).
 * When FALSE, the TRACE_LOG command (normally ';') cannot be used to
 * examine the Lynx Trace Log during the current session.  If left
 * TRUE, but you wish to use command line piping of stderr to a file
 * you specify, include the -tlog toggle on the command line.  Note
 * that once TRACE mode is turned on during a session and stderr is
 * directed to the log, all stderr messages will continue going to
 * the log, even if TRACE mode is turned off via the TOGGLE_TRACE
 * (Control-T) command.
 */
#define USE_TRACE_LOG	TRUE

/*******************************
 * If GOTOBUFFER is set to TRUE here or in lynx.cfg the last entered
 * goto URL, if any, will be offered as a default for reuse or editing
 * when the 'g'oto command is entered.  All previously used goto URLs
 * can be accessed for reuse or editing via a circular buffer invoked
 * with the Up-Arrow or Down-Arrow keys after entering the 'g'oto
 * command, whether or not a default is offered.
 */
#define GOTOBUFFER	  FALSE

/*****************************
 * JUMPFILE is the default local file checked for shortcut URLs when
 * the user presses the 'J' (JUMP) key.  The user will be prompted for
 * a shortcut entry (analogously to 'g'oto), and can enter one
 * or use '?' for a list of the shortcuts with associated links to
 * their actual URLs.  See the sample jumps files in the samples
 * subdirectory.  Make sure your jumps file includes a '?' shortcut
 * for a file://localhost URL to itself:
 *
 * <dt>?<dd><a href="file://localhost/path/jumps.html">This Shortcut List</a>
 *
 * If not defined here or in lynx.cfg, the JUMP command will invoke
 * the NO_JUMPFILE statusline message (see LYMessages_en.h).  The prompt
 * associated with the default jumps file is defined as JUMP_PROMPT in
 * LYMessages_en.h and can be modified in lynx.cfg.  Additional, alternate
 * jumps files can be defined and mapped to keystrokes, and alternate
 * prompts can be set for them, in lynx.cfg, but at least one default
 * jumps file and associated prompt should be established before adding
 * others.
 *
 * On VMS, use Unix SHELL syntax (including a lead slash) to define it.
 *
 * Do not include "file://localhost" in the definition.
 */
/* #define JUMPFILE "/Lynx_Dir/jumps.html" */

/*******************************
 * If JUMPBUFFER is set to TRUE here or in lynx.cfg the last entered
 * jump shortcut, if any, will be offered as a default for reuse or
 * editing when the JUMP command is entered.  All previously used
 * shortcuts can be accessed for reuse or editing via a circular buffer
 * invoked with the Up-Arrow or Down-Arrow keys after entering the JUMP
 * command, whether or not a default is offered.  If you have multiple
 * jumps files and corresponding key mappings, each will have its own
 * circular buffer.
 */
#define JUMPBUFFER	  FALSE

/********************************
 * If PERMIT_GOTO_FROM_JUMP is defined, then a : or / in a jump target
 * will be treated as a full or partial URL (to be resolved versus the
 * startfile), and will be handled analogously to a 'g'oto command.
 * Such "random URLs" will be entered in the circular buffer for goto
 * URLs, not the buffer for jump targets (shortcuts).  If the target
 * is the single character ':', it will be treated equivalently to an
 * Up-Arrow or Down-Arrow following a 'g'oto command, for accessing the
 * circular buffer of goto URLs.
 */
/* #define PERMIT_GOTO_FROM_JUMP */

/*****************************
 * If LYNX_HOST_NAME is defined here and/or in lynx.cfg, it will be
 * treated as an alias for the local host name in checks for URLs on
 * the local host (e.g., when the -localhost switch is set), and this
 * host name, "localhost", and HTHostName (the fully qualified domain
 * name of the system on which Lynx is running) will all be passed as
 * local.  A different definition in lynx.cfg will override this one.
 */
/* #define LYNX_HOST_NAME "www.cc.ukans.edu" */

/*********************
 * LOCAL_DOMAIN is used for a tail match with the ut_host element of
 * the utmp or utmpx structure on systems with utmp capabilites, to
 * determine if a user is local to your campus or organization when
 * handling -restrictions=inside_foo or outside_foo settings for ftp,
 * news, telnet/tn3270 and rlogin URLs.  An "inside" user is assumed
 * if your system does not have utmp capabilities.  CHANGE THIS here
 * or in lynx.cfg.
 */
#define LOCAL_DOMAIN "ukans.edu"

/********************************
* The DEFAULT_CACHE_SIZE specifies the number of WWW documents to be
* cached in memory at one time.
*
* This so-called cache size (actually, number) may be modified in lynx.cfg
* and or with the command line argument -cache=NUMBER  The minimum allowed
* value is 2, for the current document and at least one to fetch, and there
* is no absolute maximum number of cached documents.  On Unix, and VMS not
* compiled with VAXC, whenever the number is exceeded the least recently
* displayed document will be removed from memory.
*
* On VMS compiled with VAXC, the DEFAULT_VIRTUAL_MEMORY_SIZE specifies the
* amount (bytes) of virtual memory that can be allocated and not yet be freed
* before previous documents are removed from memory.  If the values for both
* the DEFAULT_CACHE_SIZE and DEFAULT_VIRTUAL_MEMORY_SIZE are exceeded, then
* least recently displayed documents will be freed until one or the other
* value is no longer exceeded.  The value can be modified in lynx.cfg.
*
* The Unix and VMS but not VAXC implementations use the C library malloc's
* and calloc's for memory allocation, and procedures for taking the actual
* amount of cache into account still need to be developed.  They use only
* the DEFAULT_CACHE_SIZE value, and that specifies the absolute maximum
* number of documents to cache (rather than the maximum number only if
* DEFAULT_VIRTUAL_MEMORY_SIZE has been exceeded, as with VAXC/VAX).
*/
#define DEFAULT_CACHE_SIZE 10

#if defined(VMS) && defined(VAXC) && !defined(__DECC)
#define DEFAULT_VIRTUAL_MEMORY_SIZE 512000
#endif /* VMS && VAXC && !__DECC */

/********************************
 * If ALWAYS_RESUBMIT_POSTS is set TRUE, Lynx always will resubmit forms
 * with method POST, dumping any cache from a previous submission of the
 * form, including when the document returned by that form is sought with
 * the PREV_DOC command or via the history list.  Lynx always resubmits
 * forms with method POST when a submit button or a submitting text input
 * is activated, but normally retrieves the previously returned document
 * if it had links which you activated, and then go back with the PREV_DOC
 * command or via the history list.
 *
 * The default defined here can be changed in lynx.cfg, and can be toggled
 * via the -resubmit_posts command line switch.
 */
#define ALWAYS_RESUBMIT_POSTS FALSE

/********************************
 * CHARACTER_SET defines the default character set, i.e., that assumed
 * to be installed on the user's termimal.  It determines which characters
 * or strings will be used to represent 8-bit character entities within
 * HTML.  New character sets may be defined as explained in the README
 * files of the src/chrtrans directory in the Lynx source code distribution.
 * For Asian (CJK) character sets, it also determines how Kanji code will
 * be handled.  The default defined here can be changed in lynx.cfg, and
 * via the 'o'ptions menu.  The 'o'ptions menu setting will be stored in
 * the user's RC file whenever those settings are saved, and thereafter
 * will be used as the default.  Also see lynx.cfg for information about
 * the -raw switch and LYE_RAW_TOGGLE command.
 *
 * The default character sets include:
 *
 *   Display Character Set name		MIME name
 *   ==========================		=========
 *   7 bit approximations		us-ascii
 *   Chinese				euc-cn
 *   DEC Multinational			dec-mcs
 *   DosArabic (cp864)			cp864
 *   DosBaltRim (cp775)			cp775
 *   DosCyrillic (cp866)		cp866
 *   DosGreek (cp737)			cp737
 *   DosGreek2 (cp869)			cp869
 *   DosHebrew (cp862)			cp862
 *   DosLatin1 (cp850)			cp850
 *   DosLatin2 (cp852)			cp852
 *   DosLatinUS (cp437)			cp437
 *   ISO 8859-10			iso-8859-10
 *   ISO 8859-5 Cyrillic		iso-8859-5
 *   ISO 8859-6 Arabic			iso-8859-6
 *   ISO 8859-7 Greek			iso-8859-7
 *   ISO 8859-8 Hebrew			iso-8859-8
 *   ISO 8859-9 (Latin 5)		iso-8859-9
 *   ISO Latin 1			iso-8859-1
 *   ISO Latin 2			iso-8859-2
 *   ISO Latin 3			iso-8859-3
 *   ISO Latin 4			iso-8859-4
 *   Japanese (EUC)			euc-jp
 *   Japanese (SJIS)			shift_jis
 *   KOI8-R Cyrillic			koi8-r
 *   Korean				euc-kr
 *   Macintosh (8 bit)			macintosh
 *   NeXT character set			next
 *   RFC 1345 Mnemonic			mnemonic
 *   RFC 1345 w/o Intro			mnemonic+ascii+0
 *   Taipei (Big5)			big5
 *   Transparent			x-transparent
 *   UNICODE UTF-8			utf-8
 *   Vietnamese (VISCII)		viscii
 *   WinArabic (cp1256)			windows-1256
 *   WinBaltRim (cp1257)		windows-1257
 *   WinCyrillic (cp1251)		windows-1251
 *   WinGreek (cp1253)			windows-1253
 *   WinHebrew (cp1255)			windows-1255
 *   WinLatin1 (cp1252)			windows-1252
 *   WinLatin2 (cp1250)			windows-1250
 */
#define CHARACTER_SET "ISO Latin 1"

/*****************************
 * PREFERRED_LANGUAGE is the language in MIME notation (e.g., "en",
 * "fr") which will be indicated by Lynx in its Accept-Language headers
 * as the preferred language.  If available, the document will be
 * transmitted in that language.  This definition can be overriden via
 * lynx.cfg.  Users also can change it via the 'o'ptions menu and save
 * that preference in their RC file. This may be a comma-separated list
 * of languages in decreasing preference.
 */
#define PREFERRED_LANGUAGE "en"

/*****************************
 * PREFERRED_CHARSET specifies the character set in MIME notation (e.g.,
 * "ISO-8859-2", "ISO-8859-5") which Lynx will indicate you prefer in
 * requests to http servers using an Accept-Charsets header.
 * This definition can be overriden via lynx.cfg.  Users also can change it
 * via the 'o'ptions menu and save that preference in their RC file.
 * The value should NOT include "ISO-8859-1" or "US-ASCII", since those
 * values are always assumed by default.
 * If a file in that character set is available, the server will send it.
 * If no Accept-Charset header is present, the default is that any
 * character set is acceptable. If an Accept-Charset header is present,
 * and if the server cannot send a response which is acceptable
 * according to the Accept-Charset header, then the server SHOULD send
 * an error response with the 406 (not acceptable) status code, though
 * the sending of an unacceptable response is also allowed. (RFC2068)
 */
#define PREFERRED_CHARSET ""

/*****************************
* If MULTI_BOOKMARK_SUPPORT is set TRUE, and BLOCK_MULTI_BOOKMARKS (see
* below) is FALSE, and sub-bookmarks exist, all bookmark operations will
* first prompt the user to select an active sub-bookmark file or the
* default bookmark file.  FALSE is the default so that one (the default)
* bookmark file will be available initially.  The default set here can
* be overridden in lynx.cfg.  The user can turn on multiple bookmark
* support via the 'o'ptions menu, and can save that choice as the startup
* default via the .lynxrc file.  When on, the setting can be STANDARD or
* ADVANCED.  If support is set to the latter, and the user mode also is
* ADVANCED, the VIEW_BOOKMARK command will invoke a statusline prompt at
* which the user can enter the letter token (A - Z) of the desired bookmark,
* or '=' to get a menu of available bookmark files.  The menu always is
* presented in NOVICE or INTERMEDIATE mode, or if the support is set to
* STANDARD.  No prompting or menu display occurs if only one (the startup
* default) bookmark file has been defined (define additional ones via the
* 'o'ptions menu).  The startup default, however set, can be overridden on
* the command line via the -restrictions=multibook or the -anonymous or
* -validate switches.
*/
#ifndef MULTI_BOOKMARK_SUPPORT
#define MULTI_BOOKMARK_SUPPORT FALSE
#endif /* MULTI_BOOKMARK_SUPPORT */

/*****************************
* If BLOCK_MULTI_BOOKMARKS is set TRUE, multiple bookmark support will
* be forced off, and cannot be toggled on via the 'o'ptions menu.  This
* compilation setting can be overridden via lynx.cfg.
*/
#ifndef BLOCK_MULTI_BOOKMARKS
#define BLOCK_MULTI_BOOKMARKS FALSE
#endif /* BLOCK_MULTI_BOOKMARKS */

/********************************
 * URL_DOMAIN_PREFIXES and URL_DOMAIN_SUFFIXES are strings which will be
 * prepended (together with a scheme://) and appended to the first element
 * of command line or 'g'oto arguments which are not complete URLs and
 * cannot be opened as a local file (file://localhost/string).  Both
 * can be comma-separated lists.  Each prefix must end with a dot, each
 * suffix must begin with a dot, and either may contain other dots (e.g.,
 * .com.jp).  The default lists are defined here, and can be changed
 * in lynx.cfg.  Each prefix will be used with each suffix, in order,
 * until a valid Internet host is created, based on a successful DNS
 * lookup (e.g., foo will be tested as www.foo.com and then www.foo.edu
 * etc.).  The first element can include a :port and/or /path which will
 * be restored with the expanded host (e.g., wfbr:8002/dir/lynx will
 * become http://www.wfbr.edu:8002/dir/lynx).  The prefixes will not be
 * used if the first element ends in a dot (or has a dot before the
 * :port or /path), and similarly the suffixes will not be used if the
 * the first element begins with a dot (e.g., .nyu.edu will become
 * http://www.nyu.edu without testing www.nyu.com).  Lynx will try to
 * guess the scheme based on the first field of the expanded host name,
 * and use "http://" as the default (e.g., gopher.wfbr.edu or gopher.wfbr.
 * will be made gopher://gopher.wfbr.edu).
 */
#define URL_DOMAIN_PREFIXES "www."
#define URL_DOMAIN_SUFFIXES ".com,.edu,.net,.org"

/********************************
 * If LIST_NEWS_NUMBERS is set TRUE, Lynx will use an ordered list
 * and include the numbers of articles in news listings, instead of
 * using an unordered list.
 *
 * The default defined here can be changed in lynx.cfg.
 */
#define LIST_NEWS_NUMBERS FALSE

/********************************
 * If LIST_NEWS_DATES is set TRUE, Lynx will include the dates of
 * articles in news listings.  The dates always are included in the
 * articles, themselves.
 *
 * The default defined here can be changed in lynx.cfg.
 */
#define LIST_NEWS_DATES FALSE

/*************************
 * Set NEWS_POSTING to FALSE if you do not want to support posting to
 * news groups via Lynx.  If left TRUE, Lynx will use its news gateway to
 * post new messages or followups to news groups, using the URL schemes
 * described in the "Supported URL" section of the online 'h'elp.  The
 * posts will be attempted via the nntp server specified in the URL, or
 * if none was specified, via the NNTPSERVER configuration or environment
 * variable.  Links with these URLs for posting or sending followups are
 * created by the news gateway when reading group listings or articles
 * from nntp servers if the server indicates that it permits posting.
 * The setting here can be changed in lynx.cfg.
 */
#define NEWS_POSTING TRUE

/*************************
 * Define LYNX_SIG_FILE to the name of a file containing a signature which
 * can be appended to email messages and news postings or followups.  The
 * user will be prompted whether to append it.  It is sought in the home
 * directory.  If it is in a subdirectory, begin it with a dot-slash
 * (e.g., ./lynx/.lynxsig).  The definition here can be changed in lynx.cfg.
 */
#define LYNX_SIG_FILE ".lynxsig"

/********************************
 * If USE_SELECT_POPUPS is set FALSE, Lynx will present a vertical list
 * of radio buttons for the OPTIONs in SELECT blocks which lack the
 * MULTIPLE attribute, instead of using a popup menu.  Note that if
 * the MULTIPLE attribute is present in the SELECT start tag, Lynx
 * always will create a vertical list of checkboxes for the OPTIONs.
 *
 * The default defined here can be changed in lynx.cfg.  It can be
 * set and saved via the 'o'ptions menu to override the compilation
 * and configuration defaults, and the default always can be toggled
 * via the -popup command line switch.
 */
#define USE_SELECT_POPUPS TRUE

/********************************
 * If COLLAPSE_BR_TAGS is set FALSE, Lynx will not collapse serial
 * BR tags.  Note that the valid way to insert extra blank lines in
 * HTML is via a PRE block with only newlines in the block.
 *
 * The default defined here can be changed in lynx.cfg.
 */
#define COLLAPSE_BR_TAGS TRUE

/********************************
 * If SET_COOKIES is set FALSE, Lynx will ignore Set-Cookie headers
 * in http server replies.
 *
 * The default defined here can be changed in lynx.cfg, and can be toggled
 * via the -cookies command line switch.
 */
#define SET_COOKIES TRUE


/****************************************************************
 *   Section 2.   Things that you probably want to change or review
 *
 */

/*****************************
 * The following three definitions set the number of seconds for
 * pauses following statusline messages that would otherwise be
 * replaced immediately, and are more important than the unpaused
 * progress messages.  Those set by INFOSECS are also basically
 * progress messages (e.g., that a prompted input has been cancelled)
 * and should have the shortest pause.  Those set by MESSAGESECS are
 * informational (e.g., that a function is disabled) and should have
 * a pause of intermediate duration.  Those set by ALERTSECS typically
 * report a serious problem and should be paused long enough to read
 * whenever they appear (typically unexpectedly).  The default values
 * defined here can be modified via lynx.cfg, should longer pauses be
 * desired for braille-based access to Lynx.
 */
#define INFOSECS 1
#define MESSAGESECS 2
#define ALERTSECS 3

/******************************
 * SHOW_COLOR controls whether the program displays in color by default.
 */
#ifdef COLOR_CURSES
#define SHOW_COLOR TRUE
#else
#define SHOW_COLOR FALSE
#endif

/******************************
 * SHOW_CURSOR controls whether or not the cursor is hidden or appears
 * over the current link, or current option in select popup windows.
 * Showing the cursor is handy if you are a sighted user with a poor
 * terminal that can't do bold and reverse video at the same time or
 * at all.  It also can be useful to blind users, as an alternative
 * or supplement to setting LINKS_AND_FORM_FIELDS_ARE_NUMBERED or
 * LINKS_ARE_NUMBERED.
 *
 * The default defined here can be changed in lynx.cfg.  It can be
 * set and saved via the 'o'ptions menu to override the compilation
 * and configuration defaults, and the default always can be toggled
 * via the -show_cursor command line switch.
 */
#define SHOW_CURSOR FALSE

/******************************
* VERBOSE_IMAGES controls whether or not Lynx replaces the [LINK], [INLINE] and
* [IMAGE] comments (for images without ALT) with filenames of these images. 
* This is extremely useful because now we can determine immediately what images
* are just decorations (button.gif, line.gif) and what images are important. 
* 
* The default defined here can be changed in lynx.cfg. 
*/ 
#define VERBOSE_IMAGES FALSE 
 
/****************************** 
 * BOXVERT and BOXHORI control the layout of popup menus.  Set to 0 if your
 * curses supports line-drawing characters, set to '*' or any other character
 * to not use line-drawing (e.g., '|' for vertical and '-' for horizontal).
 */
#ifndef HAVE_CONFIG_H
#ifdef DOSPATH
#define BOXVERT 0
#define BOXHORI 0
#else
#define BOXVERT '|'
/* #define BOXVERT 0 */
#define BOXHORI '-'
/* #define BOXHORI 0 */
#endif /* DOSPATH */
#endif	/* !HAVE_CONFIG_H */

/******************************
 * LY_UMLAUT controls the 7-bit expansion of characters with dieresis or
 * umlaut.  If defined, a digraph is displayed, e.g., auml --> ae
 * Otherwise, a single character is displayed,  e.g., auml --> a
 * Note that this is currently not supported with the chartrans code,
 * or rather it doesn't have an effect if translations for a display
 * character set are taken from one of the *.tbl files in src/chrtrans.
 * One would have to modify the corresponding *.tbl file for this.
 */
#define LY_UMLAUT

/*******************************
 * Execution links/scripts configuration.
 *
 * Execution links and scripts allow you to run
 * local programs by activating links within Lynx.
 *
 * An execution link is of the form:
 *
 *     lynxexec:<COMMAND>
 * or:
 *     lynxexec://<COMMAND>
 * or:
 *     lynxprog:<COMMAND>
 * or:
 *     lynxprog://<COMMAND>
 *
 * where <COMMAND> is a command that Lynx will run when the link is
 * activated.  The double-slash should be included if the command begins
 * with an '@', as for executing VMS command files.  Otherwise, the double-
 * slash can be omitted.
 * Use lynxexec for commands or scripts that generate a screen output which
 * should be held via a prompt to press <return> before returning to Lynx
 * for display of the current document.
 * Use lynxprog for programs such as mail which do require a pause before
 * Lynx restores the display of the current document.
 *
 * Execution scripts take the form of a standard
 * URL.  Extension mapping or MIME typing is used
 * to decide if the file is a script and should be
 * executed.  The current extensions are:
 * .csh, .ksh, and .sh on UNIX systems and .com on
 * VMS systems.  Any time a file of this type is
 * accessed Lynx will look at the user's options
 * settings to decide if the script can be executed.
 * Current options include: Only exec files that
 * reside on the local machine and are referenced
 * with a "file://localhost" URL, All execution
 * off, and all execution on.
 *
 * The following definitions will add execution
 * capabilities to Lynx.  You may define none, one
 * or both.
 *
 * I strongly recommend that you define neither one
 * of these since execution links/scripts can represent
 * very serious security risk to your system and its
 * users.  If you do define these I suggest that
 * you only allow users to execute files/scripts
 * that reside on your local machine.
 *
 * YOU HAVE BEEN WARNED!
 *
 * Note: if you are enabling execution scripts you should
 * also see src/HTInit.c to verify/change the execution
 * script extensions and/or commands.
 */
/* #define EXEC_LINKS  */
/* #define EXEC_SCRIPTS  */

/**********
 * UNIX:
 * =====
 * CGI script support. Defining LYNXCGI_LINKS allows you to use the
 *
 *   lynxcgi:path
 *
 * URL which allows lynx to access a cgi script directly without the need for
 * a http daemon. Redirection or mime support is not supported but just about
 * everything else is. If the path is not an executable file then the URL is
 * rewritten as file://localhost and passed to the file loader. This means that
 * if your http:html files are currently set up to use relative addressing, you
 * should be able to fire up your main page with lynxcgi:path and everything
 * should work as if you were talking to the http daemon.
 *
 * Note that TRUSTED_LYNXCGI directives must be defined in your lynx.cfg file
 * if you wish to place restrictions on source documents and/or paths for
 * lynxcgi links.
 *
 * The cgi scripts are called with a fork()/execve() sequence so you don't
 * have to worry about people trying to abuse the code. :-)
 *
 *     George Lindholm (George.Lindholm@ubc.ca)
 *
 * VMS:
 * ====
 * The lynxcgi scheme, if enabled, yields an informational message regardless
 * of the path, and use of the freeware OSU DECthreads server as a local
 * script server is recommended instead of lynxcgi URLs.  Uncomment the
 * following line to define LYNXCGI_LINKS, and when running Lynx, enter
 * lynxcgi:advice  as a G)oto URL for more information and links to the
 * OSU server distribution.
 */
/* #define LYNXCGI_LINKS */

#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS)

/**********
 * if ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS
 * is defined then the user will be able to change
 * the execution status within the options screen.
 */
/* #define ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS */

/**********
 * if NEVER_ALLOW_REMOTE_EXEC is defined then local execution of
 * scripts or lynxexec and lynxprog URLs will only be implemented
 * from HTML files that were accessed via a "file://localhost/" URL,
 * and the options menu for "L)ocal executions links" will only
 * allow toggling between "ALWAYS OFF" and "FOR LOCAL FILES ONLY".
 */
/* #define NEVER_ALLOW_REMOTE_EXEC */

/*****************************
 * These are for executable shell scripts and links.
 * Set to FALSE unless you really know what you're
 * doing.
 *
 * This only applies if you are compiling with EXEC_LINKS or
 * EXEC_SCRIPTS defined.
 *
 * The first two settings:
 * LOCAL_EXECUTION_LINKS_ALWAYS_ON
 * LOCAL_EXECUTION_LINKS_ON_BUT_NOT_REMOTE
 * specify the DEFAULT setting of the users execution link
 * options, but the user may still change those options.
 * If you do not wish the user to be able to change the
 * execution link settings you may wish to use the commandline option:
 *    -restrictions=exec_frozen
 *
 * LOCAL_EXECUTION_LINKS_ALWAYS_ON will be FALSE
 * if NEVER_ALLOW_REMOTE_EXEC has been defined.
 *
 * if LOCAL_EXECUTION_LINKS_ALWAYS_OFF_FOR_ANONYMOUS is
 * true all execution links will be disabled when the
 * -anonymous command line option is used.  Anonymous
 * users are not allowed to change the execution options
 * from within the Lynx options menu so you might be able
 * to use this option to enable execution links and set
 * LOCAL_EXECUTION_LINKS_ON_BUT_NOT_REMOTE to TRUE to
 * give anonymous execution link capability without compromising
 * your system (see comments about TRUSTED_EXEC rules in
 * lynx.cfg for more information).
 *
 */
#define LOCAL_EXECUTION_LINKS_ALWAYS_ON          FALSE
#define LOCAL_EXECUTION_LINKS_ON_BUT_NOT_REMOTE  FALSE
#define LOCAL_EXECUTION_LINKS_ALWAYS_OFF_FOR_ANONYMOUS FALSE

#endif /*  defined(EXEC_LINKS) || defined(EXEC_SCRIPTS) */

/*********************************
 *  MAIL_SYSTEM_ERROR_LOGGING will send a message to the owner of
 *  the information if there is one, every time
 *  that a document cannot be accessed!
 *
 *  NOTE: This can generate A LOT of mail, be warned.
 */
#define MAIL_SYSTEM_ERROR_LOGGING   FALSE  /*mail a message for every error?*/

/*********************************
 * If CHECKMAIL is set to TRUE, the user will be informed (via a statusline
 * message) about the existence of any unread mail at startup of Lynx, and
 * will get statusline messages if subsequent new mail arrives.  If a jumps
 * file with a lynxprog URL for invoking mail is available, or your html
 * pages include an mail launch file URL, the user thereby can access mail
 * and read the messages.  The checks and statusline reports will not be
 * performed if Lynx has been invoked with the -restrictions=mail switch.
 *
 *  VMS USERS !!!
 * New mail is normally broadcast as it arrives, via "unsolicitied screen
 * broadcasts", which can be "wiped" from the Lynx display via the Ctrl-W
 * command.  You may prefer to disable the broadcasts and use CHECKMAIL
 * instead (e.g., in a public account which will be used by people who
 * are ignorant about VMS).
 */
#define CHECKMAIL	FALSE	/* report unread and new mail messages */

/*********************************
 * VI_KEYS can be turned on by the user in the options
 * screen or the .lynxrc file.  This is just the default.
 */
#define VI_KEYS_ALWAYS_ON	FALSE /* familiar h,j,k, & l */

/*********************************
 * EMACS_KEYS can be turned on by the user in the options
 * screen or the .lynxrc file.  This is just the default.
 */
#define EMACS_KEYS_ALWAYS_ON	FALSE /* familiar ^N, ^P, ^F, ^B */

/*********************************
 * DEFAULT_KEYPAD_MODE specifies whether by default the user
 * has numbers that work like arrows or else numbered links
 * DEFAULT KEYPAD MODE may be set to
 *	NUMBERS_AS_ARROWS   or
 *	LINKS_ARE_NUMBERED  or
 *	LINKS_AND_FORM_FIELDS_ARE_NUMBERED
 */
#define DEFAULT_KEYPAD_MODE	NUMBERS_AS_ARROWS

/********************************
 * The default search.
 * This is a default that can be overridden by the user!
 */
#define CASE_SENSITIVE_ALWAYS_ON    FALSE /* case sensitive user search */

/********************************
 * If NO_DOT_FILES is set TRUE here or in lynx.cfg, the user will not be
 * allowed to specify files beginning with a dot in reply to output filename
 * prompts, and files beginning with a dot (e.g., file://localhost/foo/.lynxrc)
 * will not be included in the directory browser's listings.  The setting here
 * will be overridden by the setting in lynx.cfg.  If FALSE, you can force it
 * to be treated as TRUE via -restrictions=dotfiles (or -anonymous, which sets
 * this and most other restrictions).
 *
 * If it's FALSE at startup of Lynx, the user can regulate it via the
 * 'o'ptions menu, and may save the preference in the RC file.
 */
#define NO_DOT_FILES    TRUE  /* disallow access to dot files */

/********************************
 * If MAKE_LINKS_FOR_ALL_IMAGES is TRUE, all images will be given links
 * which can be ACTIVATEd.  For inlines, the ALT or pseudo-ALT ("[INLINE]")
 * strings will be links for the resolved SRC rather than just text.  For
 * ISMAP or other graphic links, the ALT or pseudo-ALT ("[ISMAP]" or "[LINK]")
 * strings will have '-' and a link labeled "[IMAGE]" for the resolved SRC
 * appended.
 *
 * The default defined here can be changed in lynx.cfg, and the user can
 * use LYK_IMAGE_TOGGLE to toggle the feature on or off at run time.
 *
 * The default also can be toggled via an "-image_links" command line switch.
 */
#define MAKE_LINKS_FOR_ALL_IMAGES	FALSE /* inlines cast to links */

/********************************
 * If MAKE_PSEUDO_ALTS_FOR_INLINES is FALSE, inline images which do not
 * specify an ALT string will not have "[INLINE]" inserted as a pseudo-ALT,
 * i.e., they'll be treated as having ALT="".  If MAKE_LINKS_FOR_ALL_IMAGES
 * is defined or toggled to TRUE, however, the pseudo-ALTs will be created
 * for inlines, so that they can be used as links to the SRCs.
 *
 * The default defined here can be changed in lynx.cfg, and the user can
 * use LYK_INLINE_TOGGLE to toggle the feature on or off at run time.
 *
 * The default also can be toggled via a "-pseudo_inlines" command line
 * switch.
 */
#define MAKE_PSEUDO_ALTS_FOR_INLINES	TRUE /* Use "[INLINE]" pseudo-ALTs */

/********************************
 * If SUBSTITUTE_UNDERSCORES is TRUE, the _underline_ format will be used
 * for emphasis tags in dumps.
 *
 * The default defined here can be changed in lynx.cfg, and the user can
 * toggle the default via a "-underscore" command line switch.
 */
#define SUBSTITUTE_UNDERSCORES	FALSE /* Use _underline_ format in dumps */

/********************************
 * If QUIT_DEFAULT_YES is defined as TRUE then when the QUIT command
 * is entered, any response other than n or N will confirm.  Define it
 * as FALSE if you prefer the more conservative action of requiring an
 * explicit Y or y to confirm.  The default defined here can be changed
 * in lynx.cfg.
 */
#define QUIT_DEFAULT_YES	TRUE

/********************************
 * These definitions specify files created or used in conjunction
 * with traversals.  See CRAWL.ANNOUNCE for more infomation.
 */
#define TRAVERSE_FILE "traverse.dat"
#define TRAVERSE_FOUND_FILE "traverse2.dat"
#define TRAVERSE_REJECT_FILE "reject.dat"
#define TRAVERSE_ERRORS "traverse.errors"

/****************************************************************
 * The LYMessages_en.h header defines default, English strings
 * used in statusline prompts, messages, and warnings during
 * program execution.  See the comments in LYMessages_en.h for
 * information on translating or customizing them for your site.
 */
#ifndef HTTELNET_H
#include "LYMessages_en.h"
#endif /* !HTTELNET_H */


/****************************************************************
 *   Section 3.   Things that you should not change until you
 *  		  have a good knowledge of the program
 */

#define LYNX_NAME "Lynx"
/* The strange-looking comments on the next line tell PRCS to replace
 * the version definition with the Project Version on checkout. Just
 * ignore it. - kw */
/* $Format: "#define LYNX_VERSION \"$ProjectVersion$\""$ */
#define LYNX_VERSION "2.8.1dev.13"

#ifndef MAXINT
#define MAXINT 2147483647	/* max integer */
#endif /* !MAXINT */
#define MAXBASE 100		/* max length of base directory */
#define MAXHIGHLIGHT 160	/* max length of highlighted text */
#define MAXTARGET 130		/* max length of target string */
#define LINESIZE 1024		/* max length of line to read from file */
#define MAXFNAME 1280		/* max filename length DDD/FILENAME.EXT */
#define MAXCOMMAND MAXFNAME	/* max length of command should be the same */
#define MAXHIST  1024		/* max links we remember in history */
#define MAXLINKS 1024		/* max links on one screen */

#ifndef SEARCH_GOAL_LINE
#define SEARCH_GOAL_LINE 4	/* try to position search target there */
#endif

#define MAXCHARSETS 60		/* max character sets supported */
#define MAXCHARSETSP 61		/* always one more than MAXCHARSETS */

/* Win32 may support more, but old win16 helper apps may not. */
#if defined(__DJGPP__) || defined(_WINDOWS)
#define FNAMES_8_3
#endif

#ifdef FNAMES_8_3
#define HTML_SUFFIX ".htm"
#else
#define HTML_SUFFIX ".html"
#endif

#ifdef VMS
/*
**  Use the VMS port of gzip for uncompressing both .Z and .gz files.
*/
#define UNCOMPRESS_PATH  "gzip -d"
#define GZIP_PATH "gzip"

#else

#ifdef DOSPATH
/*  Something has to be defined for this or we don't compile. */
#define SYSTEM_MAIL "sendmail"
#define SYSTEM_MAIL_FLAGS "-t -oi"
/*
**  Following executables may be sought from your PATH at run-time.
**  To get those programs look for GNU-port stuff elsewhere.
**  Currently, if compiled with -DUSE_ZLIB (default), you need only "cp"
**
**    WINDOWS
**  ===========
*/
#define COMPRESS_PATH   "compress"
#define UNCOMPRESS_PATH "uncompress"
#define UUDECODE_PATH   "uudecode"
#define ZCAT_PATH       "zcat"
#define GZIP_PATH       "gzip"
#define INSTALL_PATH    "install"
#define TAR_PATH        "tar"
#define TOUCH_PATH      "touch"

/*
**    WINDOWS/DOS
**  ===========
*/
#define ZIP_PATH        "zip"
#define UNZIP_PATH      "unzip"
#define MKDIR_PATH      "mkdir"
#define MV_PATH         "mv"
#define RM_PATH         "rm"
#define COPY_PATH       "cp"
#define CHMOD_PATH      "chmod"

#else	/* Unix */
	/* this is done via the configure script */
#endif /* DOSPATH */
#endif /* VMS */


/****************************************************************
 *  Section 4.  Things you MUST check only if you plan to use Lynx
 *              in an anonymous account (allow public access to Lynx).
 *              This section may be skipped by those people building
 *              Lynx for private use only.
 *
 */

/*****************************
 * Enter the name of your anonymous account if you have one
 * as ANONYMOUS_USER.  UNIX systems will use a cuserid
 * or get_login call to determine if the current user is
 * the ANONYMOUS_USER.  VMS systems will use getenv("USER").
 *
 * You may use the "-anonymous" option for multiple accounts,
 * or for precautionary reasons in the anonymous account, as well.
 *
 * Specify privileges for the anonymous account below.
 *
 * It is very important to have this correctly defined or include
 * the "-anonymous" command line option for invocation of Lynx
 * in an anonymous account!  If you do not you will be putting
 * yourself at GREAT security risk!
 */
#define ANONYMOUS_USER ""

/*******************************
 * In the following four pairs of defines,
 * INSIDE_DOMAIN means users connecting from inside your local domain,
 * OUTSIDE_DOMAIN means users connecting from outside your local domain.
 *
 * set to FALSE if you don't want users of your anonymous
 * account to be able to telnet back out
 */
#define CAN_ANONYMOUS_INSIDE_DOMAIN_TELNET	TRUE
#define CAN_ANONYMOUS_OUTSIDE_DOMAIN_TELNET	FALSE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to use ftp
 */
#define CAN_ANONYMOUS_INSIDE_DOMAIN_FTP		TRUE
#define CAN_ANONYMOUS_OUTSIDE_DOMAIN_FTP	FALSE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to use rlogin
 */
#define CAN_ANONYMOUS_INSIDE_DOMAIN_RLOGIN	TRUE
#define CAN_ANONYMOUS_OUTSIDE_DOMAIN_RLOGIN	FALSE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to read news
 */
#define CAN_ANONYMOUS_INSIDE_DOMAIN_READ_NEWS	TRUE
#define CAN_ANONYMOUS_OUTSIDE_DOMAIN_READ_NEWS	FALSE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to goto random URLs. (The 'g' command)
 */
#define CAN_ANONYMOUS_GOTO		TRUE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to goto particular URLs.
 */
#define CAN_ANONYMOUS_GOTO_CSO		FALSE
#define CAN_ANONYMOUS_GOTO_FILE		FALSE
#define CAN_ANONYMOUS_GOTO_FINGER	TRUE
#define CAN_ANONYMOUS_GOTO_FTP		FALSE
#define CAN_ANONYMOUS_GOTO_GOPHER	FALSE
#define CAN_ANONYMOUS_GOTO_HTTP		TRUE
#define CAN_ANONYMOUS_GOTO_HTTPS	FALSE
#define CAN_ANONYMOUS_GOTO_LYNXCGI	FALSE
#define CAN_ANONYMOUS_GOTO_LYNXEXEC	FALSE
#define CAN_ANONYMOUS_GOTO_LYNXPROG	FALSE
#define CAN_ANONYMOUS_GOTO_MAILTO	TRUE
#define CAN_ANONYMOUS_GOTO_NEWS		FALSE
#define CAN_ANONYMOUS_GOTO_NNTP		FALSE
#define CAN_ANONYMOUS_GOTO_RLOGIN	FALSE
#define CAN_ANONYMOUS_GOTO_SNEWS	FALSE
#define CAN_ANONYMOUS_GOTO_TELNET	FALSE
#define CAN_ANONYMOUS_GOTO_TN3270	FALSE
#define CAN_ANONYMOUS_GOTO_WAIS		TRUE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to specify a port in 'g'oto commands
 * for telnet URLs.
 */
#define CAN_ANONYMOUS_GOTO_TELNET_PORT	FALSE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to jump to URLs (The 'J' command)
 * via the shortcut entries in your JUMPFILE.
 */
#define CAN_ANONYMOUS_JUMP	FALSE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to mail
 */
#define CAN_ANONYMOUS_MAIL	TRUE

/*******************************
 * set to FALSE if you don't want users of your anonymous
 * account to be able to print
 */
#define CAN_ANONYMOUS_PRINT	FALSE

/*****************************
 * Be sure you have read about and set defines above in Sections
 * 1, 2 and 3 that could  affect Lynx in an anonymous account,
 * especially LOCAL_EXECUTION_LINKS_ALWAYS_OFF_FOR_ANONYMOUS.
 *
 * This ends the section specific to anonymous accounts.
 */

#endif /* USERDEFS_H */
