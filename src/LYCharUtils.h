
#ifndef LYCHARUTILS_H
#define LYCHARUTILS_H

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif /* HTUTILS_H */

extern char * LYUnEscapeEntities PARAMS((
	char *		str,
	BOOLEAN		plain_space,
	BOOLEAN		hidden));
extern void LYUnEscapeToLatinOne PARAMS((
	char **		str,
	BOOLEAN		isURL));
extern void LYExpandString PARAMS((
	char **		str));
extern void LYEntify PARAMS((
	char **		str,
	BOOLEAN		isTITLE));
extern void LYTrimHead PARAMS((
	char *		str));
extern void LYTrimTail PARAMS((
	char *		str));
extern char *LYFindEndOfComment PARAMS((
	char *		str));
extern void LYFillLocalFileURL PARAMS((
	char **		href,
	char *		base));

#ifdef Lynx_HTML_Handler
extern int OL_CONTINUE;		/* flag for whether CONTINUE is set */
extern int OL_VOID;		/* flag for whether a count is set */
extern void LYZero_OL_Counter PARAMS((
	HTStructured *		me));
extern char *LYUppercaseA_OL_String PARAMS((
	int			seqnum));
extern char *LYLowercaseA_OL_String PARAMS((
	int			seqnum));
extern char *LYUppercaseI_OL_String PARAMS((
	int			seqnum));
extern char *LYLowercaseI_OL_String PARAMS((
	int			seqnum));
extern void LYHandleMETA PARAMS((
	HTStructured *		me,
	CONST BOOL*	 	present,
	CONST char **		value,
	char **			include));
extern int LYLegitimizeHREF PARAMS((
	HTStructured *	 	me,
	char **			href,
	BOOL			force_slash));
extern void LYCheckForContentBase PARAMS((
	HTStructured *		me));
extern void LYCheckForID PARAMS((
	HTStructured *		me,
	CONST BOOL *		present,
	CONST char **		value,
	int			attribute));
extern void LYHandleID PARAMS((
	HTStructured *		me,
	char *			id));
extern BOOLEAN LYoverride_default_alignment PARAMS((
	HTStructured *		me));
extern void LYEnsureDoubleSpace PARAMS((
	HTStructured *		me));
extern void LYEnsureSingleSpace PARAMS((
	HTStructured *		me));
extern void LYResetParagraphAlignment PARAMS((
	HTStructured *		me));
extern BOOLEAN LYCheckForCSI PARAMS((
	HTStructured *		me,
	char **			url));
#endif /* Lynx_HTML_Handler */

#endif /* LYCHARUTILS_H */
