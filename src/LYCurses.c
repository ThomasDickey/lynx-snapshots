#include "HTUtils.h"
#include "tcp.h"
#include "LYCurses.h"
#include "LYUtils.h"
#include "LYGlobalDefs.h"
#include "LYSignal.h"
#include "LYClean.h"
#include "LYStrings.h"
#ifdef USE_SLANG
#include "LYCharSets.h"
#endif /* USE_SLANG */

#include "LYexit.h"
#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

#ifdef VMS
#define DISPLAY "DECW$DISPLAY"
#else
#define DISPLAY "DISPLAY"
#endif /* VMS */

#if defined(VMS) && defined(__GNUC__)
#include <gnu_hacks.h>
#undef LINES
#undef COLS
#define LINES lines
#define COLS cols
extern int _NOSHARE(LINES);
extern int _NOSHARE(COLS);
#endif /* VMS && __GNUC__ */

/*
 *  These are routines to start and stop curses and to cleanup
 *  the screen at the end.
 */

PRIVATE int dumbterm PARAMS((char *terminal));
BOOLEAN LYCursesON = FALSE;

#ifdef USE_SLANG
PUBLIC unsigned int Lynx_Color_Flags = 0;
PRIVATE int Current_Attr;
PUBLIC BOOLEAN FullRefresh = FALSE;
PUBLIC int curscr = 0;

PUBLIC void LY_SLrefresh NOARGS
{
    if (FullRefresh) {
        SLsmg_touch_lines(0, LYlines);
	FullRefresh = FALSE;
    }
    SLsmg_refresh();

    return;
}

#ifdef VMS
PUBLIC void VTHome NOARGS
{
    printf("\033[;H");

    return;
}
#endif /* VMS */

PUBLIC void sl_add_attr ARGS1(
	int,		a)
{
    Current_Attr |= a;
    SLsmg_set_color(Current_Attr);
}

PUBLIC void sl_sub_attr ARGS1(
	int,		a)
{
    Current_Attr &= ~a;
    SLsmg_set_color(Current_Attr);
}

PUBLIC void lynx_setup_colors NOARGS
{
    SLtt_set_color(0, NULL, "black", "white");
    SLtt_set_color(1, NULL, "blue", "white");	 /* bold */
    SLtt_set_color(2, NULL, "yellow", "blue");	 /* reverse */
    SLtt_set_color(4, NULL, "magenta", "white"); /* underline */
    /*
     *  The other objects are '|'ed together to get rest.
     */
    SLtt_set_color(3, NULL, "green", "white");	 /* bold-reverse */
    SLtt_set_color(5, NULL, "blue", "white");    /* bold-underline */
    SLtt_set_color(6, NULL, "red", "white");	 /* reverse-underline */
    SLtt_set_color(7, NULL, "magenta", "cyan");	 /* reverse-underline-bold */

    /*
     *  Now set monchrome attributes.
     */
    SLtt_set_mono(1, NULL, SLTT_BOLD_MASK);
    SLtt_set_mono(2, NULL, SLTT_REV_MASK);
    SLtt_set_mono(3, NULL, SLTT_REV_MASK | SLTT_BOLD_MASK);
    SLtt_set_mono(4, NULL, SLTT_ULINE_MASK);
    SLtt_set_mono(5, NULL, SLTT_ULINE_MASK | SLTT_BOLD_MASK);
    SLtt_set_mono(6, NULL, SLTT_ULINE_MASK | SLTT_REV_MASK);
    SLtt_set_mono(7, NULL, SLTT_ULINE_MASK | SLTT_BOLD_MASK | SLTT_REV_MASK);
}

PRIVATE void sl_suspend ARGS1(
	int,		sig)
{
#ifndef VMS
#ifdef SIGSTOP
    int r, c;

    if (sig == SIGTSTP)
        SLsmg_suspend_smg();
    SLang_reset_tty();
    kill(getpid(),SIGSTOP);
#if SLANG_VERSION > 9929
    SLang_init_tty(-1, 0, 1);
#else
    SLang_init_tty(3, 0, 1);
#endif /* SLANG_VERSION > 9929 */
    signal(SIGTSTP, sl_suspend);
    SLtty_set_suspend_state(1);
    if (sig == SIGTSTP)
        SLsmg_resume_smg();
    /*
     *  Get new window size in case it changed.
     */
    r = SLtt_Screen_Rows;
    c = SLtt_Screen_Cols;
    size_change(0);
    if ((r != SLtt_Screen_Rows) || (c != SLtt_Screen_Cols)) {
	recent_sizechange = TRUE;
    }
#endif /* SIGSTOP */
#endif /* !VMS */
   return;
}
#endif /* USE_SLANG */


PUBLIC void start_curses NOARGS
{
#ifdef USE_SLANG
    static int slinit;

    if (LYCursesON)
        return;

    if (slinit == 0) {
	SLtt_get_terminfo();
	if (Lynx_Color_Flags & SL_LYNX_USE_COLOR)
	    SLtt_Use_Ansi_Colors = 1;
	size_change(0);

	SLtt_add_color_attribute(4, SLTT_ULINE_MASK);
	SLtt_add_color_attribute(5, SLTT_ULINE_MASK);
	/*
	 *  If set, the blink escape sequence will turn on high
	 *  intensity background (rxvt and maybe Linux console).
	 */
	if (Lynx_Color_Flags & SL_LYNX_USE_BLINK)
	    SLtt_Blink_Mode = 1;	       
	else
	    SLtt_Blink_Mode = 0;
    }
    slinit = 1;
    Current_Attr = 0;
#ifndef VMS
#if SLANG_VERSION > 9929
    SLang_init_tty(-1, 0, 1);
#else
    SLang_init_tty(3, 0, 1);
#endif /* SLANG_VERSION > 9929 */
#endif /* !VMS */
    SLsmg_init_smg();
    SLsmg_Display_Eight_Bit = LYlowest_eightbit[current_char_set];
    SLsmg_Newline_Moves = -1;
    SLsmg_Backspace_Moves = 1;
#ifndef VMS
   SLtty_set_suspend_state(1);
#ifdef SIGTSTP
    signal(SIGTSTP, sl_suspend);
#endif /* SIGTSTP */
    signal(SIGINT, cleanup_sig);
#endif /* !VMS */

#else /* Using curses: */

#ifdef VMS
    /*
     *  If we are VMS then do initsrc() everytime start_curses()
     *  is called!
     */
    initscr();  /* start curses */
#else /* Unix: */
    static BOOLEAN first_time = TRUE;

    if (first_time) {
	/*
	 *  If we're not VMS then only do initsrc() one time,
	 *  and one time only!
	 */
	if (initscr() == NULL) {  /* start curses */
	    fprintf(stderr, 
	        "Terminal initialisation failed - unknown terminal type?\n");
            (void) signal(SIGHUP, SIG_DFL);
            (void) signal(SIGTERM, SIG_DFL);
            (void) signal(SIGINT, SIG_DFL);
#ifdef SIGTSTP
	    if (no_suspend)
	        (void) signal(SIGTSTP,SIG_DFL);
#endif /* SIGTSTP */
	    exit (-1);
	}
        first_time = FALSE;
    }
#endif /* VMS */

    /* nonl();   *//* seems to slow things down */

#ifdef VMS
    crmode();
    raw();
#else
#ifdef NO_CBREAK
    crmode();
#else
    cbreak();
#endif /* NO_CBREAK */
    signal(SIGINT, cleanup_sig);
#endif /* VMS */

    noecho();

#if !defined(VMS) && !defined(NO_KEYPAD)
    keypad(stdscr,TRUE);
#endif /* !VMS && !NO_KEYPAD */

    fflush(stdin);
    fflush(stdout);
#endif /* USE_SLANG */

    LYCursesON = TRUE;
}


PUBLIC void stop_curses NOARGS
{
    echo();

    /*
     *	Fixed for better dumb terminal support.
     *	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
     */
    if(LYCursesON == TRUE)	{
    	endwin();	/* stop curses */
    }

    fflush(stdout);

    LYCursesON = FALSE;

#ifndef VMS
    signal(SIGINT, SIG_DFL);
#endif /* !VMS */
}


#ifdef VMS
/*
 *  Check terminal type, start curses & setup terminal.
 */
PUBLIC BOOLEAN setup ARGS1(
	char *,		terminal)
{
    int c;
    int status;
    char *dummy, *cp, term[81];
#ifdef USE_SLANG
    extern void longname();
#endif /* USE_SLANG */

    /*
     *  If the display was not set by a command line option then
     *  see if it is available from the environment.
     */
    if ((display = getenv(DISPLAY)) != NULL && *display == '\0')
        display = NULL;

    /*
     *  Get terminal type, and convert to lower case.
     */
    term[0] = '\0';
    longname(dummy, term);
    if (term[0] == '\0' && (form_get_data || form_post_data)) {
        /*
	 *  Some yoyo used these under conditions which require
	 *  -dump, so force that mode here. - FM
	 */
        extern int mainloop();

	dump_output_immediately = TRUE;
        LYcols = 80;
	keypad_mode = LINKS_ARE_NUMBERED;
	status = mainloop();
        (void) signal (SIGHUP, SIG_DFL);
        (void) signal (SIGTERM, SIG_DFL);
#ifdef SIGTSTP
	if (no_suspend)
	  (void) signal(SIGTSTP,SIG_DFL);
#endif /* SIGTSTP */
	exit(status);
    }
    for (cp=term; *cp!='\0'; cp++)
	if (isupper(*cp))
	    *cp = TOLOWER(*cp);

    printf("Terminal = %s\n", term);
    sleep(InfoSecs);
    if ((strlen(term) < 5) ||
        strncmp(term, "vt", 2) || !isdigit(term[2])) {
	printf(
	    "You must use a vt100, 200, etc. terminal with this program.\n");
	printf("Proceed (n/y)? ");
	c = getchar();
	if (c != 'y' && c != 'Y') {
	    printf("\n");
	    return(FALSE);
	}
	strcpy(term,"vt100");
    }

    ttopen();
    start_curses();

    LYlines = LINES;
    LYcols = COLS;

    return(TRUE);
}

#else	/* Not VMS: */

/*
 *  Check terminal type, start curses & setup terminal.
 */
PUBLIC BOOLEAN setup ARGS1(
	char *,		terminal)
{
    static char term_putenv[120];
    char buffer[120];

   /*
    *  If the display was not set by a command line option then
    *  see if it is available from the environment .
    */
    if ((display = getenv(DISPLAY)) != NULL && *display == '\0')
        display = NULL;

    if (terminal != NULL) {
	sprintf(term_putenv,"TERM=%s",terminal);
	(void) putenv(term_putenv);
    }

    /*
     *  Query the terminal type.
     */
    if (dumbterm(getenv("TERM"))) {
	char *s;

	printf("\n\n  Your Terminal type is unknown!\n\n");
	printf("  Enter a terminal type: [vt100] ");
	*buffer = '\0';
	fgets(buffer, sizeof(buffer), stdin);
	if ((s = strchr(buffer, '\n')) != NULL)
	    *s = '\0';

	if (strlen(buffer) == 0)
	    strcpy(buffer,"vt100"); 
	
	sprintf(term_putenv,"TERM=%s", buffer);
	putenv(term_putenv);
	printf("\nTERMINAL TYPE IS SET TO %s\n",getenv("TERM"));
	sleep(MESSAGESECS);
    }

    start_curses();

#ifndef NO_TTYTYPE
    /*
     *  Get terminal type (strip 'dec-' from vms style types).
     */
    if (strncmp((CONST char*)ttytype, "dec-vt", 6) == 0) {
	(void) setterm((char *)ttytype + 4);
    }
#endif /* !NO_TTYTYPE */

    LYlines = LINES;
    LYcols = COLS;

    return(1);
}

PRIVATE int dumbterm ARGS1(
	char *,		terminal)
{
    int dumb = FALSE;

    /*
     *	Began checking for terminal == NULL in case that TERM environemnt
     *	variable is not set.  Thanks to Dick Wesseling (ftu@fi.ruu.nl).
     */
    if (terminal == NULL ||
	!strcasecomp(terminal, "network") ||
	!strcasecomp(terminal, "unknown") ||
	!strcasecomp(terminal, "dialup")  ||
	!strcasecomp(terminal, "dumb")    ||
	!strcasecomp(terminal, "switch")  ||
	!strcasecomp(terminal, "ethernet")  )
	dumb = TRUE;
    return(dumb);
}
#endif /* VMS */

#ifdef VMS
/*
 *	Cut-down termio --
 *		Do character-oriented stream input for Jeff.
 *		Code ripped off from Micro-Emacs 3.7 by Daniel Lawrence.
 *
 *		Ever-so-slightly modified by Kathryn Huxtable.  29-Jan-1991.
 *		Cut down for Lou.  8 Sep 1992.
 *		Cut down farther for Lou.  19 Apr 1993.
 *			We don't set PASSALL or PASTHRU since we don't
 *			want to block CTRL/C, CTRL/Y, CTRL/S or CTRL/Q.
 *			Simply setting NOECHO and doing timed reads
 *			is sufficient.
 *              Further mods by Fote.  29-June-1993
 *			ttopen() and ttclose() are now terminal initialization
 *			 and restoration procedures, called once at startup
 *			 and at exit, respectively, of the LYNX image.
 *			ttclose() should be called before an exit from LYNX
 *			 no matter how the exit is invoked.
 *			setup(terminal) does the ttopen().
 *			cleanup() calls cleanup_files() and ttclose().
 *			ttgetc() now handles NOECHO and NOFLITR (instead of
 *			 setting the terminal itself to NOECHO in ttopen()).
 *			VMSsignal() added for handling both Ctrl-C *and* Ctrl-Y
 *			 interrupts, and disabling system response to Ctrl-T.
 *              Further mods by Fote.  15-Dec-1993
 *			Added edit handler in ttopen() which will invoke
 *			 VMSexit() and behave intelligently on ACCVIO's.
 *              Further mods by Fote.  29-Dec-1993
 *			Simplified ttgetc().
 *		Further mods by Fote.  16-Jan-1994
 *			Added code in ttopen() which will invoke VMSVersion()
 *			 to get the version of VMS as VersionVMS for use by
 *			 by new or modified interrupt or spawning routines.
 *		Further mods by Fote.  27-Jan-1994
 *			Added back a typeahead() which supports 'z' or 'Z' as
 *			an "Zap transfer" command via HTCheckForInterrupt()
 *			in LYUtils.c.
 */

#include <descrip.h>
#include <iodef.h>
#include <ssdef.h>
#include <stdlib.h>
#include <msgdef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <libclidef.h>
#include <lib$routines.h>
#include <starlet.h>
#include <clidef.h>
#include <syidef.h>
#ifdef signal
#undef signal
#endif /* signal */
#include <signal.h>
#ifdef system
#undef system
#endif /* system */
#include <processes.h>

#ifndef CLI$M_TRUSTED
#define CLI$M_TRUSTED 64 /* May not be in the compiler's clidef.h	*/
#endif /* !CLI$M_TRUSTED */
#ifndef LIB$_INVARG
#define LIB$_INVARG 1409588
#endif /* !LIB$_INVARG */

#define EFN	0			/* Event flag			*/

static	unsigned char buffer[20];	/* Input buffer			*/
static	int	in_pos, in_len;		/* For escape sequences		*/
static	int	oldmode[3];		/* Old TTY mode bits		*/
static	int	newmode[3];		/* New TTY mode bits		*/
static	short	iochan;			/* TTY I/O channel		*/
static  $DESCRIPTOR(term_nam_dsc,"TT"); /* Descriptor for iochan	*/
static	unsigned long mask = LIB$M_CLI_CTRLY|LIB$M_CLI_CTRLT; /* ^Y and ^T */
static	unsigned long old_msk;		/* Saved control mask		*/
static	short	trap_flag = FALSE;	/* TRUE if AST is set		*/
BOOLEAN DidCleanup = FALSE;		/* Exit handler flag		*/
static char VersionVMS[20];		/* Version of VMS		*/

PUBLIC int VMSVersion ARGS2(
	char *,		VerString,
	int,		VerLen)
{
     unsigned long status, itm_cod = SYI$_VERSION;
     int i, verlen = 0;
     struct dsc$descriptor version;
     char *m;

     version.dsc$a_pointer = VerString;
     version.dsc$w_length = VerLen - 1;
     version.dsc$b_dtype = DSC$K_DTYPE_B;
     version.dsc$b_class = DSC$K_CLASS_S;

     status = lib$getsyi(&itm_cod, 0, &version, &verlen, 0, 0);
     if (!(status&1) || verlen == 0)
	  return 0;

     /*
      *  Cut out trailing spaces
      */
     for (m = VerString+verlen, i = verlen-1; i > 0 && VerString[i] == ' '; --i)
	  *(--m) = '\0';

     return strlen(VerString)+1;	/* Transmit ending 0 too */
}

PUBLIC void VMSexit NOARGS
{
    /*
     *  If we get here and DidCleanup is not set, it was via an
     *  ACCVIO, or outofmemory forced exit, so make *sure* we
     *  attempt a cleanup and reset the terminal.
     */
    if (!DidCleanup) {
        if (LYOutOfMemory == FALSE) {
            fprintf(stderr,
"\nA Fatal error has occured in %s Ver. %s\n", LYNX_NAME, LYNX_VERSION);
	    fprintf(stderr,
"\nPlease notify your system administrator to confirm a bug, and if\n");
	    fprintf(stderr,
"confirmed, to notify the lynx-dev list.  Bug reports should have concise\n");
	    fprintf(stderr,
"descriptions of the command and/or URL which causes the problem, the\n");
	    fprintf(stderr,
"operating system name with version number, the TCPIP implementation, the\n");
	    fprintf(stderr,
"TRACEBACK if it can be captured, and any other relevant information.\n");

            fprintf(stderr,"\nPress RETURN to clean up: ");
	    (void) getchar();
	} else if (LYCursesON) {
	    _statusline(MEMORY_EXHAUSTED_ABORT);
	    sleep(AlertSecs);
	}
        cleanup();
    }
    if (LYOutOfMemory == TRUE) {
	printf("\r\n%s\r\n\r\n", MEMORY_EXHAUSTED_ABORT);
	fflush(stdout);
    }
}

/*
 *	TTOPEN --
 *		This function is called once to set up the terminal
 *		device streams.	 It translates TT until it finds
 *		the terminal, then assigns a channel to it, sets it
 *		to EDIT, and sets up the Ctrl-C and Ctrl-Y interrupt
 *		handling.
 */
PUBLIC int ttopen NOARGS
{
	extern	void cleanup_sig();
	int	iosb[2];
	int	status;
	static unsigned long condition;
	static struct _exit_block {
	    unsigned long forward;
	    unsigned long address;
	    unsigned long zero;
	    unsigned long condition;
	} exit_handler_block;

	status = sys$assign( &term_nam_dsc, &iochan, 0, 0 );
	if( status != SS$_NORMAL )
		exit( status );

	status = sys$qiow( EFN, iochan, IO$_SENSEMODE, &iosb, 0, 0,
			  &oldmode, sizeof(oldmode), 0, 0, 0, 0 );
	if( status != SS$_NORMAL )
		exit( status );

	status = iosb[0] & 0xFFFF;
	if( status != SS$_NORMAL )
		exit( status );

	newmode[0] = oldmode[0];
	newmode[1] = oldmode[1];
	newmode[2] = oldmode[2] | TT2$M_EDIT;

	status = sys$qiow( EFN, iochan, IO$_SETMODE, &iosb, 0, 0,
			  &newmode, sizeof(newmode), 0, 0, 0, 0 );
	if( status != SS$_NORMAL )
		exit( status );

	status = iosb[0] & 0xFFFF;
	if( status != SS$_NORMAL )
		exit( status );

	/*
	 *  Declare the exit handler block.
	 */
	exit_handler_block.forward   = 0;
	exit_handler_block.address   = (unsigned long) &VMSexit;
	exit_handler_block.zero      = 0;
	exit_handler_block.condition = (unsigned long) &condition;
	status = sys$dclexh(&exit_handler_block);
	if (status != SS$_NORMAL)
		exit( status );

	/*
	 *  Set the AST.
	 */
	lib$disable_ctrl(&mask, &old_msk);
	trap_flag = TRUE;
	status = sys$qiow ( EFN, iochan,
			    IO$_SETMODE|IO$M_CTRLCAST|IO$M_CTRLYAST,
			    &iosb, 0, 0,
			    &cleanup_sig, SIGINT, 0, 0, 0, 0 );
	if ( status != SS$_NORMAL ) {
		lib$enable_ctrl(&old_msk);
		exit ( status );
	}

	/*
	 *  Get the version of VMS.
	 */
	if (VMSVersion(VersionVMS, 20) < 3)
		/*
		 *  Load zeros on error.
		 */
		strcpy(VersionVMS, "V0.0-0");

	return(0);
}	/*  ttopen  */

/*
 *	TTCLOSE --
 *		This function gets called just before we go back home
 *		to the command interpreter.  It puts the terminal back
 *		in a reasonable state.
 */
PUBLIC int ttclose NOARGS
{
	int	status;
	int	iosb[1];

	status = sys$qiow( EFN, iochan, IO$_SETMODE, &iosb, 0, 0,
			  &oldmode, sizeof(oldmode), 0, 0, 0, 0 );

	if( status != SS$_NORMAL || (iosb[0] & 0xFFFF) != SS$_NORMAL )
		exit( status );

	if (trap_flag) {
	    status = sys$dassgn (iochan);
	    status = lib$enable_ctrl(&old_msk);
	    trap_flag = FALSE;
	}
	return(0);
}	/*  ttclose  */

/*
 *	TTGETC --
 *		Read a character from the terminal, with NOECHO and NOFILTR.
 */
PUBLIC int ttgetc NOARGS
{
    int status;
    unsigned short iosb[4];

    if (in_pos < in_len)
        return(buffer[in_pos++]);

    status = sys$qiow(EFN, iochan,
		      IO$_READVBLK|IO$M_NOECHO|IO$M_NOFILTR,
		      &iosb, 0, 0,
		      &buffer, 1, 0, 0, 0, 0);
    if ((status&1) == 1)
        status = iosb[0];
    if (status == SS$_PARTESCAPE) {
	/*
	 *  Escape sequence in progress.  Fake a successful read.
	 */
	status = 1;
    }
    if ((status&1) != 1 && status != SS$_DATAOVERUN)
        exit(status);
    in_pos = 1;
    in_len = iosb[1] + iosb[3];
    return(buffer[0]);
}

/*
 *	TYPEAHEAD -- Fote Macrides 27-Jan-1994
 *		Check whether a keystroke has been entered, and return
 *		 it, or -1 if none was entered.
 */
PUBLIC int typeahead NOARGS
{
    int status;
    unsigned short iosb[4];

    if (dump_output_immediately)
        return -1;

    if (in_pos < in_len)
        return(buffer[in_pos++]);

again:
    status = sys$qiow (EFN, iochan,
		       IO$_READVBLK|IO$M_TIMED|IO$M_NOECHO|IO$M_NOFILTR,
		       &iosb, 0, 0,
		       &buffer, 1, 0, 0, 0, 0);
    if ((status&1) == 1)
        status = iosb[0];
    if (status == SS$_PARTESCAPE) {
	/*
	 *  Escape sequence in progress, finish reading it.
	 */
	goto again;
    }

    in_pos = 1;
    in_len = iosb[1] + iosb[3];
    if (status == SS$_TIMEOUT || status == SS$_DATAOVERUN)
        return(-1);
    return (buffer[0]);
}

/*
 *	VMSSIGNAL -- Fote Macrides 29-Jun-1993
 *		Sets up AST for both Ctrl-C and Ctrl-Y, with system response
 *		 to Ctrl-T disabled.  If called with a sig other than SIGINT,
 *		 it will use the C library's system(sig, func).
 *              The equivalent of VMSsignal(SIGINT, cleanup_sig) is done on
 *               intialization by ttopen(), so don't do it again.
 *		VMSsignal(SIGINT, SIG_DFL) is treated as a call to ttclose().
 *              Call VMSsignal(SIGINT, SIG_IGN) before system() calls to
 *               enable Ctrl-C and Ctrl-Y in the subprocess, and then call
 *               VMSsignal(SIG_INT, cleanup_sig) on return from the subprocess.
 *		For func's which set flags and do not invoke an exit from
 *		 LYNX, the func should reassert itself.
 *              The VMS signal() calls do not fully emulate the Unix calls,
 *		 and VMSsignal() is just a "helper", also not a full emulation.
 */

PUBLIC void *VMSsignal (sig,func)
int sig;
void (*func)();
{
	int status;
	short iosb[4];
	static int SIG_IGN_flag;

	/*
	 *  Pass all signals other than SIGINT to signal().
	 *  Also pass SIGINT to signal() if we're dumping.
	 */
	if (sig != SIGINT || dump_output_immediately) {
	    signal(sig, func);
	    return;
	}

	/*
	 *  If func is SIG_DFL, treat it as ttclose().
	 */
	if (func == SIG_DFL) {
	    ttclose();
	    return;
	}

	/*
	 *  Clear any previous AST.
	 */
	if (trap_flag) {
	    status = sys$dassgn (iochan);
	    status = lib$enable_ctrl(&old_msk);
	    trap_flag = FALSE;
	}

	/*
	 *  If func is SIG_IGN, leave the TT channel closed and the
	 *  system response to interrupts enabled for system() calls.
	 */
	if (func == SIG_IGN)
	    return;

	/*
	 *  If we get to here, we have a LYNX func, so set the AST.
	 */
	lib$disable_ctrl(&mask, &old_msk);
	trap_flag = TRUE;
	status = sys$assign (&term_nam_dsc, &iochan, 0, 0 );
	status = sys$qiow ( EFN, iochan,
			    IO$_SETMODE|IO$M_CTRLCAST|IO$M_CTRLYAST,
			    &iosb, 0, 0,
			    func, SIGINT, 0, 0, 0, 0 );

}	/* VMSsignal */

/*
 *  DCLspawn_exception, spawn_DCLprocess, DCLsystem -- F.Macrides 16-Jan-1994
 *	Exception-handler routines for regulating interrupts and enabling
 *	Control-T during spawns.  Includes TRUSTED flag for versions of VMS
 *	which require it in captive accounts.  This code should be used
 *	instead of the VAXC or DECC system(), by including LYSystem.h in
 *	modules which have system() calls.  It helps ensure that we return
 *	to Lynx instead of breaking out to DCL if a user issues interrupts
 *	or generates an ACCVIO during spawns.
 */
#ifdef __DECC
PRIVATE unsigned int DCLspawn_exception ARGS2(
	void *,		sigarr,
	void *,		mecharr)
{
#else
PRIVATE int DCLspawn_exception ARGS2(
	void *,		sigarr,
	void *,		mecharr)
{
#endif /* __DECC */
     int status;
     
     status = lib$sig_to_ret(sigarr, mecharr);
     return(SS$_UNWIND);
}

PRIVATE int spawn_DCLprocess ARGS1(
	char *,		command)
{
     int status;
     unsigned long Status = 0;
     /*
      *  Keep DECC from complaining.
      */
     struct dsc$descriptor_s  command_desc;
     command_desc.dsc$w_length  = strlen(command);
     command_desc.dsc$b_class   = DSC$K_CLASS_S;
     command_desc.dsc$b_dtype   = DSC$K_DTYPE_T;
     command_desc.dsc$a_pointer = command;

     VAXC$ESTABLISH(DCLspawn_exception);

#ifdef __ALPHA /** OpenVMS/AXP lacked the TRUSTED flag before v6.1 **/
     if (VersionVMS[1] > '6' ||
         (VersionVMS[1] == '6' && VersionVMS[2] == '.' &&
	  VersionVMS[3] >= '1')) {
#else
     if (VersionVMS[1] >= '6') {
#endif /* __ALPHA */
	 /*
	  *  Include TRUSTED flag.
	  */
	 unsigned long trusted = CLI$M_TRUSTED;
         status = lib$spawn(&command_desc,0,0,&trusted,
	 		    0,0,&Status);
	 /*
	  *  If it was invalid, try again without the flag.
	  */
         if (status == LIB$_INVARG)
            status = lib$spawn(&command_desc,0,0,0,
	    		       0,0,&Status );
     } else
	 status = lib$spawn(&command_desc,0,0,0,
	 		    0,0,&Status);
     /*
      *  Return -1 on error.
      */
     if ((status&1) != 1 || (Status&1) != 1)
         return(-1);
     /*
      *  Return 0 on success.
      */
     return(0);
}

PUBLIC int DCLsystem ARGS1(
	char *,		command)
{
     int status;
     extern void controlc();
     
     VMSsignal(SIGINT, SIG_IGN);
     status = spawn_DCLprocess(command);
     VMSsignal(SIGINT, cleanup_sig);
     /*
      *  Returns 0 on success, -1 any error.
      */
     return(status);
}

#ifndef USE_SLANG
/*
**  This function boxes windows with graphic characters for curses.
**  Pass it the window, it's height, and it's width. - FM
*/
PUBLIC void VMSbox ARGS3(
	WINDOW *,	win,
	int,		height,
	int,		width)
{
    int i;

    wmove(win, 0, 0);
    waddstr(win, "\033)0\016l");
    for (i = 1; i < width; i++)
       waddch(win, 'q');
    waddch(win, 'k');
    for (i = 1; i < height-1; i++) {
        wmove(win, i, 0);
	waddch(win, 'x');
        wmove(win, i, width-1);
	waddch(win, 'x');
    }
    wmove(win, i, 0);
    waddch(win, 'm');
    for (i = 1; i < width; i++)
       waddch(win, 'q');
    waddstr(win, "j\017");
}
#endif /* !USE_SLANG */
#endif /* VMS */
