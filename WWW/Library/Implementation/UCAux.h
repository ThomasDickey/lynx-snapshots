#ifndef UCAUX_H
#define UCAUX_H

extern BOOL UCCanUniTranslateFrom PARAMS((int from));
extern BOOL UCCanTranslateUniTo PARAMS((int to));
extern BOOL UCCanTranslateFromTo PARAMS((int from, int to));

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
