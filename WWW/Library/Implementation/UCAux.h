#ifndef UCAUX_H
#define UCAUX_H

/*
 *  A type for a "Translation Quality" (actually, Transcoding Quality).
 *  This is a fuzzy concept since we are just looking at the charset
 *  not what characters are actually there, so it's just a guess for
 *  "common" cases.  TQ_NO must be 0 since callers of functions that
 *  return this type may treat result as a boolean flag.
 *  The functions returning this type could be improved to use more
 *  knowledge from the translation tables.
 */
typedef enum {
    TQ_NO	= 0,		/* must be 0 */
    TQ_POOR	= 1,
    TQ_GOOD	= 2,
    TQ_EXCELLENT = 3
} UCTQ_t;

extern UCTQ_t UCCanUniTranslateFrom PARAMS((int from));
extern UCTQ_t UCCanTranslateUniTo PARAMS((int to));
extern UCTQ_t UCCanTranslateFromTo PARAMS((int from, int to));
extern BOOL UCNeedNotTranslate PARAMS((int from, int to));

struct _UCTransParams
{
    BOOL transp;
    BOOL do_cjk;
    BOOL decode_utf8;
    BOOL output_utf8;
    BOOL use_raw_char_in;
    BOOL strip_raw_char_in;
    BOOL pass_160_173_raw;
    BOOL do_8bitraw;
    BOOL trans_to_uni;
    BOOL trans_C0_to_uni;
    BOOL repl_translated_C0;
    BOOL trans_from_uni;
};
typedef struct _UCTransParams UCTransParams;

#ifndef UCDEFS_H
#include "UCDefs.h"
#endif /* UCDEFS_H */

extern void UCSetTransParams PARAMS((
	UCTransParams * 	pT,
	int			cs_in,
	CONST LYUCcharset *	p_in,
	int			cs_out,
	CONST LYUCcharset *	p_out));

extern void UCTransParams_clear PARAMS((
	UCTransParams *		pT));

extern void UCSetBoxChars PARAMS((
    int		cset,
    int *	pvert_out,
    int *	phori_out,
    int		vert_in,
    int		hori_in));

#ifndef HTSTREAM_H
#include "HTStream.h"
#endif /* HTSTREAM_H */

typedef void putc_func_t PARAMS((
	HTStream *	me,
	char		ch));

extern BOOL UCPutUtf8_charstring PARAMS((
	HTStream *	target,
	putc_func_t *	actions,
	long	code));
    
#endif /* UCAUX_H */
