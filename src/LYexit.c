/*
 *	Copyright (c) 1994, University of Kansas, All Rights Reserved
 */
#include "HTUtils.h"
#include "tcp.h"
#include "LYexit.h"
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
#include <syslog.h>
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

PRIVATE void LYCompleteExit NOPARAMS;

PUBLIC void LYexit ARGS1(int, status)	{
/*
 *	Purpose:	Terminates program.
 *	Arguments:	status	Exit code.
 *	Return Value:	void
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Function calls stdlib.h exit
 *	Revision History:
 *		06-15-94	created Lynx 2-3-1 Garrett Arch Blythe
 */

	/*
	 *	Do functions registered with LYatexit
	 */
	LYCompleteExit();

#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
	syslog(LOG_INFO, "Session over");
	closelog();
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

#ifdef exit
/*
 *	Make sure we use stdlib exit and not LYexit.
 */
#undef exit
#endif /* exit */

	exit(status);
}

/*
 *	Stack of functions to call upon exit.
 */
PRIVATE void (*callstack[ATEXITSIZE])();
PRIVATE int topOfStack = 0;

#ifdef __STDC__
PUBLIC int LYatexit(void (*function)())	{
#else /* Not ANSI, ugh! */
PUBLIC int LYatexit(function)
void (*function)();	{
#endif /* __STDC__ */
/*
 *	Purpose:	Registers termination function.
 *	Arguments:	function	The function to register.
 *	Return Value:	int	0	registered
 *				!0	no more space to register
 *	Remarks/Portability/Dependencies/Restrictions:
 *	Revision History:
 *		06-15-94	created Lynx 2-3-1 Garrett Arch Blythe
 */
	/*
	 *	Check for available space
	 */
	if(topOfStack == ATEXITSIZE)	{
		return(-1);
	}

	/*
	 *	Register the function.
	 */
	callstack[topOfStack] = function;
	topOfStack++;
	return(0);
}

PRIVATE void LYCompleteExit NOPARAMS	{
/*
 *	Purpose:	Call the functions registered with LYatexit
 *	Arguments:	void
 *	Return Value:	void
 *	Remarks/Portability/Dependencies/Restrictions:
 *	Revision History:
 *		06-15-94	created Lynx 2-3-1 Garrett Arch Blythe
 */

	/*
	 *	Just loop through registered functions.
	 *	This is reentrant if more exits occur in the registered
	 *		functions.
	 */
	while(--topOfStack >= 0)	{
		callstack[topOfStack]();
	}
}
