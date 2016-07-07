/*
 *	Copyright (c) 1994, University of Kansas, All Rights Reserved
 */
#include <HTUtils.h>
#include <LYexit.h>
#include <HTAlert.h>
#ifndef VMS
#include <LYGlobalDefs.h>
#include <LYUtils.h>
#include <LYSignal.h>
#include <LYMainLoop.h>
#endif /* !VMS */
#include <LYStrings.h>
#include <LYClean.h>

/*
 *  Flag for outofmem macro. - FM
 */
PUBLIC BOOL LYOutOfMemory = FALSE;


/*
 *  Stack of functions to call upon exit.
 */
PRIVATE void (*callstack[ATEXITSIZE]) NOPARAMS;
PRIVATE int topOfStack = 0;

/*
 *  Purpose:		Registers termination function.
 *  Arguments:		function	The function to register.
 *  Return Value:	int	0	registered
 *				!0	no more space to register
 *  Remarks/Portability/Dependencies/Restrictions:
 *  Revision History:
 *	06-15-94	created Lynx 2-3-1 Garrett Arch Blythe
 */

#ifdef __STDC__
PUBLIC int LYatexit(void (*function) NOPARAMS)
#else /* Not ANSI, ugh! */
PUBLIC int LYatexit(function)
void (*function) NOPARAMS;
#endif /* __STDC__ */
{
    /*
     *  Check for available space.
     */
    if (topOfStack == ATEXITSIZE) {
	CTRACE((tfp, "(LY)atexit: Too many functions, ignoring one!\n"));
	return(-1);
    }

    /*
     *  Register the function.
     */
    callstack[topOfStack] = function;
    topOfStack++;
    return(0);
}

/*
 *  Purpose:		Call the functions registered with LYatexit
 *  Arguments:		void
 *  Return Value:	void
 *  Remarks/Portability/Dependencies/Restrictions:
 *  Revision History:
 *	06-15-94	created Lynx 2-3-1 Garrett Arch Blythe
 */
PRIVATE void LYCompleteExit NOPARAMS
{
    /*
     *  Just loop through registered functions.
     *  This is reentrant if more exits occur in the registered functions.
     */
    while (--topOfStack >= 0) {
	callstack[topOfStack]();
    }
}

/*
 *  Purpose:		Terminates program.
 *  Arguments:		status	Exit code.
 *  Return Value:	void
 *  Remarks/Portability/Dependencies/Restrictions:
 *	Function calls stdlib.h exit
 *  Revision History:
 *	06-15-94	created Lynx 2-3-1 Garrett Arch Blythe
 */
PUBLIC void LYexit ARGS1(
	int,		status)
{
#ifndef VMS	/*  On VMS, the VMSexit() handler does these. - FM */
#ifdef _WINDOWS
    extern CRITICAL_SECTION critSec_DNS;	/* 1998/09/03 (Thu) 22:01:56 */
    extern CRITICAL_SECTION critSec_READ;	/* 1998/09/03 (Thu) 22:01:56 */

    DeleteCriticalSection(&critSec_DNS);
    DeleteCriticalSection(&critSec_READ);

    WSACleanup();
#endif
    if (LYOutOfMemory == TRUE) {
	/*
	 *  Ignore further interrupts. - FM
	 */
#ifndef NOSIGHUP
	(void) signal(SIGHUP, SIG_IGN);
#endif /* NOSIGHUP */
	(void) signal (SIGTERM, SIG_IGN);
	(void) signal (SIGINT, SIG_IGN);
#ifndef __linux__
#ifndef DOSPATH
	(void) signal(SIGBUS, SIG_IGN);
#endif /* DOSPATH */
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
	if (LYCursesON) {
	    LYSleepAlert();
	}
	cleanup_sig(0);
#ifndef __linux__
#ifndef DOSPATH
	signal(SIGBUS, SIG_DFL);
#endif /* DOSPATH */
#endif /* !__linux__ */
	signal(SIGSEGV, SIG_DFL);
	signal(SIGILL, SIG_DFL);
    }
#endif /* !VMS */

    /*
     *	Do functions registered with LYatexit. - GAB
     */
    LYCompleteExit();

#if !defined(VMS) && defined(SYSLOG_REQUESTED_URLS)
    LYCloselog();
#endif /* !VMS && SYSLOG_REQUESTED_URLS */

    LYCloseCmdLogfile();

#ifdef exit
/*  Make sure we use stdlib exit and not LYexit. - GAB
*/
#undef exit
#endif /* exit */

    cleanup_files();	/* if someone starts with LYNXfoo: page */
#ifndef VMS	/*  On VMS, the VMSexit() handler does these. - FM */
    fflush(stderr);
    if (LYOutOfMemory == TRUE) {
	LYOutOfMemory = FALSE;
	printf("\r\n%s\r\n\r\n", MEMORY_EXHAUSTED_ABORT);
	fflush(stdout);
    }
    LYCloseTracelog();
#endif /* !VMS */
    exit(status);
}

PUBLIC void outofmem ARGS2(
	CONST char *,	fname,
	CONST char *,	func)
{
    fprintf(stderr, "\n\n\n%s %s: %s\n", fname, func, MEMORY_EXHAUSTED_ABORTING);
    LYOutOfMemory = TRUE;
    LYexit(-1);
}
