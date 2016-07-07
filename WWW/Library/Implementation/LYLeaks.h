#ifndef __LYLEAKS_H
/*
 *	Avoid include redundancy
 *	Include only if finding memory leaks.
 */
#define __LYLEAKS_H

/*
**  Copyright (c) 1994, University of Kansas, All Rights Reserved
**
**  Include File:	LYLeaks.h
**  Purpose:		Header to convert requests for allocation to Lynx
**			custom functions to track memory leaks.
**  Remarks/Portability/Dependencies/Restrictions:
**	For the stdlib.h allocation functions to be overriden by the
**		Lynx memory tracking functions all modules allocating,
**		freeing, or resizing memory must have LY_FIND_LEAKS
**		defined before including this file.
**	This header file should be included in every source file which
**		does any memory manipulation through use of the
**		stdlib.h memory functions.
**	For proper reporting of memory leaks, the function LYLeaks
**		should be registered for execution by atexit as the
**		very first executable statement in main.
**	This code is slow and should not be used except in debugging
**		circumstances (don't define LY_FIND_LEAKS).
**	If you are using LY_FIND_LEAKS and don't want the LYLeak*
**		memory functions to be used in a certain file,
**		define NO_MEMORY_TRACKING before including this file.
**	The only safe way to call the LYLeak* functions is to use
**		the below macros because they depend on the static
**		string created by __FILE__ to not be dynamic in
**		nature (don't free it and assume will exist at all
**		times during execution).
**  Revision History:
**	05-26-94	created for Lynx 2-3-1, Garrett Arch Blythe
**	10-30-97	modified to handle StrAllocCopy() and
**			StrAllocCat(). - KW & FM
*/

/*
**  Required includes
*/
#include <stdlib.h>
#include <HTUtils.h>

/*
**	Constant defines
*/
#define MAX_CONTENT_LENGTH 50
#ifdef VMS
#define LEAKAGE_SINK "sys$login:Lynx.leaks"
#else
#define LEAKAGE_SINK "Lynx.leaks"
#endif /* VMS */

/*
**	Data structures
*/
typedef struct SourceLocation_tag	{
	/*
	 *	The file name and line number of where an event took place.
	 */
	CONST char *cp_FileName;
	short ssi_LineNumber;
}
SourceLocation;

typedef struct AllocationList_tag	{
	/*
	 *	A singly linked list.
	 */
	struct AllocationList_tag *ALp_Next;

	/*
	 *	The memory pointer allocated.
	 *	If set to NULL, then an invalid request was made.
	 *	The invalid pointer also.
	 */
	void *vp_Alloced;
	void *vp_BadRequest;

	/*
	 *	The size in bytes of the allocated memory.
	 */
	size_t st_Bytes;

	/*
	 *	The source location of specific event (calloc, malloc, free).
	 *	realloc kept separate since will track last realloc on pointer.
	 */
	SourceLocation SL_memory;
	SourceLocation SL_realloc;
} AllocationList;

/*
**  Global variable declarations
*/

/*
**  Macros
*/
#if defined(LY_FIND_LEAKS) && !defined(NO_MEMORY_TRACKING)
/*
**	Only use these macros if we are to track memory allocations.
**	The reason for using a macro instead of a define is that we want
**		to track where the initial allocation took place or where
**		the last reallocation took place.
**	Track where the allocation took place by the __FILE__ and __LINE__
**		defines which are automatic to the compiler.
*/
#ifdef malloc
#undef malloc
#endif /* malloc */
#define malloc(st_bytes) LYLeakMalloc(st_bytes, __FILE__, __LINE__)

#ifdef calloc
#undef calloc
#endif /* calloc */
#define calloc(st_number, st_bytes) LYLeakCalloc(st_number, st_bytes, \
	__FILE__, __LINE__)

#ifdef realloc
#undef realloc
#endif /* realloc */
#define realloc(vp_alloced, st_newbytes) LYLeakRealloc(vp_alloced, \
	st_newbytes, __FILE__, __LINE__)

#ifdef free
#undef free
#endif /* free */
#define free(vp_alloced) LYLeakFree(vp_alloced, __FILE__, __LINE__)

/*
**  Added the following two defines to track Lynx's frequent use
**  of those macros. - KW 1997-10-12
*/
#ifdef StrAllocCopy
#undef StrAllocCopy
#endif /* StrAllocCopy */
#define StrAllocCopy(dest, src) LYLeakSACopy(&(dest), src, __FILE__, __LINE__)
#ifdef StrAllocCat
#undef StrAllocCat
#endif /* StrAllocCat */
#define StrAllocCat(dest, src)  LYLeakSACat(&(dest), src, __FILE__, __LINE__)

#endif /* LY_FIND_LEAKS && !NO_MEMORY_TRACKING */

/*
**	Function declarations
**	See the appropriate source file for usage.
*/
extern void LYLeaks NOPARAMS;
extern void *LYLeakMalloc PARAMS((
	size_t		st_bytes,
	CONST char *	cp_File,
	CONST short	ssi_Line));
extern void *LYLeakCalloc PARAMS((
	size_t		st_number,
	size_t		st_bytes,
	CONST char *	cp_File,
	CONST short	ssi_Line));
extern void *LYLeakRealloc PARAMS((
	void *		vp_alloced,
	size_t		st_newbytes,
	CONST char *	cp_File,
	CONST short	ssi_Line));
extern void LYLeakFree PARAMS((
	void *		vp_alloced,
	CONST char *	cp_File,
	CONST short	ssi_Line));
extern char * LYLeakSACopy PARAMS((
	char **		dest,
	CONST char *	src,
	CONST char *	cp_File,
	CONST short	ssi_Line));
extern char * LYLeakSACat PARAMS((
	char **		dest,
	CONST char *	src,
	CONST char *	cp_File,
	CONST short	ssi_Line));

#endif /* __LYLEAKS_H */
