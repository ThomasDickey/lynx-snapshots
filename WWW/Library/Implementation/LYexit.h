#ifndef __LYEXIT_H
/*
 *	Avoid include redundancy
 */
#define __LYEXIT_H

/*
 *	Copyright (c) 1994, University of Kansas, All Rights Reserved
 *
 *	Include File:	LYexit.h
 *	Purpose:	Provide an atexit function for libraries without such.
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Include this header in every file that you have an exit or
 *			atexit statement.
 *	Revision History:
 *		06-15-94	created Lynx 2-3-1 Garrett Arch Blythe
 */

/*
 *	Required includes
 */
#ifdef _WINDOWS
#include <process.h>		/* declares exit() */
#endif

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	Constant defines
 */
#ifdef exit
#undef exit
#endif
#define exit(code) LYexit(code)
#define atexit LYatexit
#define ATEXITSIZE 50

/*
 *	Data structures
 */

/*
 * Global variable declarations
 */

/*
 * Macros
 */

/*
 * Function declarations
 */
    extern GCC_NORETURN void outofmem(const char *fname, const char *func);
    extern void reset_signals(void);
    extern GCC_NORETURN void exit_immediately(int status);
    extern GCC_NORETURN void LYexit(int status);
    extern int LYatexit(void (*function) (void));

#ifdef __cplusplus
}
#endif
#endif				/* __LYEXIT_H */
