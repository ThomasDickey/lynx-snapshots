#include "HTUtils.h"
#include "tcp.h"

#include "HTCJK.h"
#include "UCDefs.h"
#include "HTStream.h"
#include "UCAux.h"

extern HTCJKlang HTCJK;
extern LYUCcharset LYCharSet_UC[];

PUBLIC BOOL UCCanUniTranslateFrom ARGS1(
	int,		from)
{
    if (from < 0)
	return NO;
    if (LYCharSet_UC[from].enc == UCT_ENC_7BIT ||
	LYCharSet_UC[from].enc == UCT_ENC_UTF8)
	return YES;
    if (LYCharSet_UC[from].codepoints & (UCT_CP_SUBSETOF_LAT1))
	return YES;
    return (LYCharSet_UC[from].UChndl >= 0);
}
PUBLIC BOOL UCCanTranslateUniTo ARGS1(
	int,		to)
{
    if (to < 0)
	return NO;
    return YES;			/* well at least some characters... */
}
PUBLIC BOOL UCCanTranslateFromTo ARGS2(
	int,		from,
	int,		to)
{
    if (from == to)
	return YES;
    if (from < 0 || to < 0)
	return NO;
    if (from == 0)
	return UCCanTranslateUniTo(to);
    if (to == 0)
	return UCCanUniTranslateFrom(from);
    if (LYCharSet_UC[to].enc == UCT_ENC_UTF8) {
	return (LYCharSet_UC[from].UChndl >= 0);
    }
    {
	char * fromname = LYCharSet_UC[from].MIMEname;
	char * toname = LYCharSet_UC[to].MIMEname;
	if (!strcmp(fromname, "x-transparent") ||
	    !strcmp(toname, "x-transparent")) {
	    return YES;
	}
	if (LYCharSet_UC[from].enc == UCT_ENC_CJK) {
	    if (HTCJK == NOCJK)	/* use that global flag, for now */
		return NO;
	    if (HTCJK == JAPANESE &&
		(!strcmp(fromname, "euc-jp") ||
		 !strncmp(fromname, "iso-2022-jp",11) ||
		 !strcmp(fromname, "shift_jis")))
		return YES;
	    return NO;	/* if not handled by (from == to) above */
	}
	if (!strcmp(fromname, "koi8-r")) {
	    /*
	     *  Will try to uses stripping of high bit...
	     */
	    return YES;
	}
	    
	if (!strcmp(fromname, "koi8-r") || /* from cyrillic */
	    !strcmp(fromname, "iso-8859-5") ||
	    !strcmp(fromname, "koi-8")) {
	    if (strcmp(toname, "iso-8859-5") &&
		strcmp(toname, "koi8-r") &&
		strcmp(toname, "iso-8859-2"))
		return NO;
	}
    }
    return (LYCharSet_UC[from].UChndl >= 0);
}

/*
 *  The idea here is that any stage of the stream pipe which is interested
 *  in some charset dependent processing will call this function.
 *  Given input and ouptput charsets, this function will set various flags
 *  in a UCTransParams structure that _suggest_ to the caller what to do.
 *
 *  Should be called once when a stage starts processing text (and the
 *  input and output charsets are known), or whenever one of input or
 *  output charsets has changed (e.g. by SGML.c stage after HTML.c stage
 *  has processed a META tag).
 *  The global flags (LYRawMode, HTPassEightBitRaw etc.) are currently
 *  not taken into account here, it's still up to the caller to do something
 *  about them.
 */
PUBLIC void UCSetTransParams ARGS5(
    UCTransParams *, 	pT,
    int,		cs_in,
    CONST LYUCcharset*,	p_in,
    int,		cs_out,
    CONST LYUCcharset*,	p_out)
{
    pT->transp = (!strcmp(p_in->MIMEname, "x-transparent") ||
		  !strcmp(p_out->MIMEname, "x-transparent"));
    if (pT->transp) {
	pT->do_cjk = FALSE;
	pT->decode_utf8 = FALSE;
	pT->output_utf8 = FALSE;	/* we may, but won't know about it */
	pT->do_8bitraw = TRUE;
	pT->use_raw_char_in = TRUE;
	pT->strip_raw_char_in = FALSE;
	pT->pass_160_173_raw = TRUE;
    } else {
	BOOL intm_ucs = FALSE;
	BOOL use_ucs = FALSE;
	pT->do_cjk = ((p_in->enc == UCT_ENC_CJK) && (HTCJK != NOCJK));
	pT->decode_utf8 = (p_in->enc == UCT_ENC_UTF8);
	pT->output_utf8 = (p_out->enc == UCT_ENC_UTF8);
	if (pT->do_cjk) {
	    intm_ucs = FALSE;
	    pT->trans_to_uni = FALSE;
	    use_ucs = FALSE;
	    pT->do_8bitraw = FALSE;
	    pT->pass_160_173_raw = TRUE;
	    pT->use_raw_char_in = FALSE; /* not used for CJK */
	    pT->trans_from_uni = FALSE; /* not used for CJK */
	} else {
	    intm_ucs = (cs_in == 0 || pT->decode_utf8 ||
			(p_in->codepoints &
			 (UCT_CP_SUBSETOF_LAT1|UCT_CP_SUBSETOF_UCS2)));
	    pT->trans_to_uni = (!intm_ucs &&
				UCCanUniTranslateFrom(cs_in));
	    pT->strip_raw_char_in = ((!intm_ucs ||
				      (p_out->enc == UCT_ENC_7BIT) ||
				       (p_out->repertoire &
				        UCT_REP_SUBSETOF_LAT1)) &&
				     cs_in != cs_out &&
				     !strcmp(p_in->MIMEname, "koi8-r"));
	    use_ucs = (intm_ucs || pT->trans_to_uni);
	    pT->do_8bitraw = (!use_ucs);
	    pT->pass_160_173_raw = (!use_ucs &&
				    !(p_in->like8859 & UCT_R_8859SPECL));
	    pT->use_raw_char_in = (!pT->output_utf8 && cs_in == cs_out);
	    pT->trans_from_uni = (use_ucs && !pT->do_8bitraw &&
				  !pT->use_raw_char_in &&
				  UCCanTranslateUniTo(cs_out));
	}
    }
}

PUBLIC void UCTransParams_clear ARGS1(
    UCTransParams *,    pT)
{
    pT->transp = FALSE;
    pT->do_cjk = FALSE;
    pT->decode_utf8 = FALSE;
    pT->output_utf8 = FALSE;
    pT->do_8bitraw = FALSE;
    pT->use_raw_char_in = FALSE;
    pT->strip_raw_char_in = FALSE;
    pT->pass_160_173_raw = FALSE;
    pT->trans_to_uni = FALSE;
    pT->trans_from_uni = FALSE;
}

/*
 *  Given an output target HTStream* (can also be a HTStructured* via
 *  typecast), the target stream's put_character method, and a unicode
 *  character,  CPutUtf8_charstring() will either output the UTF8
 *  encoding of the unicode and return YES, or do nothing and return
 *  NO (if conversion would be unnecessary or the unicode character is
 *  considered invalid).
 *
 *  [Could be used more generally, but is currently only used for &#nnnnn 
 *  stuff - generation of UTF8 from 8-bit encoded charsets not yet done
 *  by SGML.c etc.]
 */
#define PUTC(ch) ((*myPutc)(target, (char)(ch)))
#define PUTC2(ch) ((*myPutc)(target,(char)(0x80|(0x3f &(ch)))))

PUBLIC BOOL UCPutUtf8_charstring ARGS3(
	HTStream *,	target,
	putc_func_t *,	myPutc,
	long,		code)
{
    if (code < 128)
	return NO;		/* indicate to caller we didn't handle it */
    else if   (code < 0x800L) {
	PUTC(0xc0 | (code>>6));
	PUTC2(code);
    } else if (code < 0x10000L) {
	PUTC(0xe0 | (code>>12));
	PUTC2(code>>6);
	PUTC2(code);
    } else if (code < 0x200000L) {
	PUTC(0xf0 | (code>>18));
	PUTC2(code>>12);
	PUTC2(code>>6);
	PUTC2(code);
    } else if (code < 0x4000000L) {
	PUTC(0xf8 | (code>>24));
	PUTC2(code>>18);
	PUTC2(code>>12);
	PUTC2(code>>6);
	PUTC2(code);
    } else if (code <= 0x7fffffffL) {
	PUTC(0xfc | (code>>30));
	PUTC2(code>>24);
	PUTC2(code>>18);
	PUTC2(code>>12);
	PUTC2(code>>6);
	PUTC2(code);
    } else
	return NO;
    return YES;
}
