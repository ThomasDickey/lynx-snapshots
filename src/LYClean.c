#include "HTUtils.h"
#include "tcp.h"
#include "LYCurses.h"
#include "LYUtils.h"
#include "LYSignal.h"
#include "LYClean.h"
#include "LYGlobalDefs.h"
#include "LYStrings.h"
#include "LYTraversal.h"

#include "LYexit.h"
#include "LYLeaks.h"

#ifdef VMS
BOOLEAN HadVMSInterrupt = FALSE;
#endif /* VMS */

/*
 * Interrupt handler.  Stop curses and exit gracefully.
 */
PUBLIC void cleanup_sig ARGS1(int,sig)
{

#ifdef IGNORE_CTRL_C
	if(sig == SIGINT)	{
            /* Need to rearm the signal */
	    signal(SIGINT, cleanup_sig);
	    sigint = TRUE;
	    return;
	}
#endif /* IGNORE_CTRL_C */

#ifdef VMS
    if (!dump_output_immediately) {
        int c;

	/* Reassert the AST  */
	(void) signal(SIGINT, cleanup_sig);
	HadVMSInterrupt = TRUE;
	if (!LYCursesON)
	    return;

        /* Refresh screen to get rid of "cancel" message, then query */
	clearok(curscr, TRUE);
	refresh();

	/* Ask if exit is intended */
	_statusline(REALLY_EXIT);
	c = LYgetch();
#ifdef QUIT_DEFAULT_YES
	if(TOUPPER(c) == 'N')
#else
	if(TOUPPER(c) != 'Y')
#endif /* QUIT_DEFAULT_YES */
	    return;
    }
#endif /* VMS */

    /* ignore further interrupts */     /*  mhc: 11/2/91 */
    (void) signal(SIGHUP, SIG_IGN);

#ifdef VMS  /* use ttclose() from cleanup() for VMS if not dumping */
    if (dump_output_immediately)
#else /* Unix: */
    (void) signal(SIGINT, SIG_IGN);
#endif /* VMS */

    (void) signal(SIGTERM, SIG_IGN);

    if (sig != SIGHUP) {
	if (!dump_output_immediately)
	    cleanup(); /* <==also calls cleanup_files() */
	printf("\nExiting via interrupt: %d\n", sig);
	fflush(stdout);
    } else {
	cleanup_files();
    }

    if (traversal)
        dump_traversal_history();

    (void) signal(SIGHUP, SIG_DFL);
    (void) signal(SIGTERM, SIG_DFL);
#ifndef VMS
    (void) signal(SIGINT, SIG_DFL);
#endif /* !VMS */
#ifdef SIGTSTP
    if (no_suspend)
	(void) signal(SIGTSTP,SIG_DFL);
#endif /* SIGTSTP */
    exit(0);
}

/*
 * called by Interrupt handler or at quit time.  
 * Erases the temporary files that lynx created
 * temporary files are removed by tempname 
 * which created them
 */
PUBLIC void cleanup_files NOARGS
{
    char filename[120];

	tempname(filename, REMOVE_FILES);
	
}

PUBLIC void cleanup NOARGS
{
#ifdef VMS
    extern BOOLEAN DidCleanup;
#endif /* VMS */

    /* cleanup signals - just in case */
    /* ignore further interrupts */     /*  mhc: 11/2/91 */
    (void) signal (SIGHUP, SIG_IGN);
    (void) signal (SIGTERM, SIG_IGN);

#ifndef VMS  /* use ttclose() from cleanup() for VMS */
    (void) signal (SIGINT, SIG_IGN);
#endif /* !VMS */

    if (LYCursesON) {
        move(LYlines-1, 0);
        clrtoeol();

        stop_bold();
        stop_underline();
        stop_reverse();
        refresh();

        stop_curses();
    }
    cleanup_files();
#ifdef VMS
    ttclose();
    DidCleanup = TRUE;
#endif /* VMS */

   fflush(stdout);
}

