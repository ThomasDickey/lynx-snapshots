/*
**  Functions associated with LYCharSets.c and the Lynx version of HTML.c - FM
**  ==========================================================================
*/
#include "HTUtils.h"
#include "tcp.h"

#define Lynx_HTML_Handler
#include "HTChunk.h"
#include "HText.h"
#include "HTStyle.h"
#include "HTMIME.h"
#include "HTML.h"

#include "HTCJK.h"
#include "HTAtom.h"
#include "HTMLGen.h"
#include "HTParse.h"

#ifdef EXP_CHARTRANS
#include "UCMap.h"
#include "UCDefs.h"
#include "UCAux.h"
#endif

#include "LYGlobalDefs.h"
#include "LYCharUtils.h"
#include "LYCharSets.h"

#include "HTAlert.h"
#include "HTFont.h"
#include "HTForms.h"
#include "HTNestedList.h"
#include "GridText.h"
#include "LYSignal.h"
#include "LYUtils.h"
#include "LYMap.h"
#include "LYBookmark.h"
#include "LYCurses.h"
#include "LYCookie.h"

#ifdef VMS
#include "HTVMSUtils.h"
#endif /* VMS */
#ifdef DOSPATH
#include "HTDOS.h"
#endif

#include "LYexit.h"
#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

extern BOOL HTPassEightBitRaw;
extern BOOL HTPassEightBitNum;
extern BOOL HTPassHighCtrlRaw;
extern BOOL HTPassHighCtrlNum;
extern HTkcode kanji_code;
extern HTCJKlang HTCJK;

/*
 *  Used for nested lists. - FM
 */
PUBLIC int OL_CONTINUE = -29999;     /* flag for whether CONTINUE is set */
PUBLIC int OL_VOID = -29998;	     /* flag for whether a count is set */


/*
**  This function converts any ampersands in allocated
**  strings to "&amp;".  If isTITLE is TRUE, it also
**  converts any angle-brackets to "&lt;" or "&gt;". - FM
*/
PUBLIC void LYEntify ARGS2(
	char **,	str,
	BOOLEAN,	isTITLE)
{
    char *p = *str;
    char *q = NULL, *cp = NULL;
    int amps = 0, lts = 0, gts = 0;
    
    if (p == NULL || *p == '\0')
        return;

    /*
     *  Count the ampersands. - FM
     */
    while ((*p != '\0') && (q = strchr(p, '&')) != NULL) {
        amps++;
	p = (q + 1);
    }

    /*
     *  Count the left-angle-brackets, if needed. - FM
     */
    if (isTITLE == TRUE) {
        p = *str;
	while ((*p != '\0') && (q = strchr(p, '<')) != NULL) {
	    lts++;
	    p = (q + 1);
	}
    }

    /*
     *  Count the right-angle-brackets, if needed. - FM
     */
    if (isTITLE == TRUE) {
        p = *str;
	while ((*p != '\0') && (q = strchr(p, '>')) != NULL) {
	    gts++;
	    p = (q + 1);
	}
    }

    /*
     *  Check whether we need to convert anything. - FM
     */
    if (amps == 0 && lts == 0 && gts == 0)
        return;

    /*
     *  Allocate space and convert. - FM
     */
    q = (char *)calloc(1,
    		     (strlen(*str) + (4 * amps) + (3 * lts) + (3 * gts) + 1));
    if ((cp = q) == NULL)
        outofmem(__FILE__, "LYEntify");
    for (p = *str; *p; p++) {
    	if (*p == '&') {
	    *q++ = '&';
	    *q++ = 'a';
	    *q++ = 'm';
	    *q++ = 'p';
	    *q++ = ';';
	} else if (isTITLE && *p == '<') {
	    *q++ = '&';
	    *q++ = 'l';
	    *q++ = 't';
	    *q++ = ';';
	} else if (isTITLE && *p == '>') {
	    *q++ = '&';
	    *q++ = 'g';
	    *q++ = 't';
	    *q++ = ';';
	} else {
	    *q++ = *p;
	}
    }
    StrAllocCopy(*str, cp);
    FREE(cp);
}

/*
**  This function trims characters <= that of a space (32),
**  including HT_NON_BREAK_SPACE (1) and HT_EM_SPACE (2),
**  but not ESC, from the heads of strings. - FM
*/
PUBLIC void LYTrimHead ARGS1(
	char *, str)
{
    int i = 0, j;

    if (!str || *str == '\0')
        return;

    while (str[i] != '\0' && WHITE(str[i]) && (unsigned char)str[i] != 27) 
        i++;
    if (i > 0) {
        for (j = 0; str[i] != '\0'; i++) {
	    str[j++] = str[i];
	}
	str[j] = '\0';
    }
}

/*
**  This function trims characters <= that of a space (32),
**  including HT_NON_BREAK_SPACE (1), HT_EM_SPACE (2), and
**  ESC from the tails of strings. - FM
*/
PUBLIC void LYTrimTail ARGS1(
	char *, str)
{
    int i;

    if (!str || *str == '\0')
        return;

    i = (strlen(str) - 1);
    while (i >= 0) {
	if (WHITE(str[i]))
	    str[i] = '\0';
	else
	    break;
	i--;
    }
}

/*
** This function should receive a pointer to the start
** of a comment.  It returns a pointer to the end ('>')
** character of comment, or it's best guess if the comment
** is invalid. - FM
*/
PUBLIC char *LYFindEndOfComment ARGS1(
	char *, str)
{
    char *cp, *cp1;
    enum comment_state { start1, start2, end1, end2 } state;

    if (str == NULL)
        /*
	 *  We got NULL, so return NULL. - FM
	 */
        return NULL;

    if (strncmp(str, "<!--", 4))
        /*
	 *  We don't have the start of a comment, so
	 *  return the beginning of the string. - FM
	 */
        return str;

    cp = (str + 4);
    if (*cp =='>')
        /*
	 * It's an invalid comment, so
	 * return this end character. - FM
	 */
	return cp;

    if ((cp1 = strchr(cp, '>')) == NULL)
        /*
	 *  We don't have an end character, so
	 *  return the beginning of the string. - FM
	 */
	return str;

    if (*cp == '-')
        /*
	 *  Ugh, it's a "decorative" series of dashes,
	 *  so return the next end character. - FM
	 */
	return cp1;

    /*
     *  OK, we're ready to start parsing. - FM
     */
    state = start2;
    while (*cp != '\0') {
        switch (state) {
	    case start1:
	        if (*cp == '-')
		    state = start2;
		else
		    /*
		     *  Invalid comment, so return the first
		     *  '>' from the start of the string. - FM
		     */
		    return cp1;
		break;

	    case start2:
	        if (*cp == '-')
		    state = end1;
		break;

	    case end1:
	        if (*cp == '-')
		    state = end2;
		else
		    /*
		     *  Invalid comment, so return the first
		     *  '>' from the start of the string. - FM
		     */
		    return cp1;
		break;

	    case end2:
	        if (*cp == '>')
		    /*
		     *  Valid comment, so return the end character. - FM
		     */
		    return cp;
		if (*cp == '-') {
		    state = start1;
		} else if (!(WHITE(*cp) && (unsigned char)*cp != 27)) {
		    /*
		     *  Invalid comment, so return the first
		     *  '>' from the start of the string. - FM
		     */
		    return cp1;
		 }
		break;

	    default:
		break;
	}
	cp++;
    }

    /*
     *  Invalid comment, so return the first
     *  '>' from the start of the string. - FM
     */
    return cp1;
}

/*
**  If an HREF, itself or if resolved against a base,
**  represents a file URL, and the host is defaulted,
**  force in "//localhost".  We need this until
**  all the other Lynx code which performs security
**  checks based on the "localhost" string is changed
**  to assume "//localhost" when a host field is not
**  present in file URLs - FM
*/
PUBLIC void LYFillLocalFileURL ARGS2(
	char **,	href,
	char *,		base)
{
    char * temp = NULL;

    if (*href == NULL || *(*href) == '\0')
        return;

    if (!strcmp(*href, "//") || !strncmp(*href, "///", 3)) {
	if (base != NULL && !strncmp(base, "file:", 5)) {
	    StrAllocCopy(temp, "file:");
	    StrAllocCat(temp, *href);
	    StrAllocCopy(*href, temp);
	}
    }
    if (!strncmp(*href, "file:", 5)) {
	if (*(*href+5) == '\0') {
	    StrAllocCat(*href, "//localhost");
	} else if (!strcmp(*href, "file://")) {
	    StrAllocCat(*href, "localhost");
	} else if (!strncmp(*href, "file:///", 8)) {
	    StrAllocCopy(temp, (*href+7));
	    StrAllocCopy(*href, "file://localhost");
	    StrAllocCat(*href, temp);
	} else if (!strncmp(*href, "file:/", 6) && *(*href+6) != '/') {
	    StrAllocCopy(temp, (*href+5));
	    StrAllocCopy(*href, "file://localhost");
	    StrAllocCat(*href, temp);
	}
    }

    /*
     * No path in a file://localhost URL means a
     * directory listing for the current default. - FM
     */
    if (!strcmp(*href, "file://localhost")) {
#ifdef VMS
	StrAllocCat(*href, HTVMS_wwwName(getenv("PATH")));
#else
	char curdir[DIRNAMESIZE];
#if HAVE_GETCWD
	getcwd (curdir, DIRNAMESIZE);
#else
	getwd (curdir);
#endif /* NO_GETCWD */
#ifdef DOSPATH
	StrAllocCat(*href, HTDOS_wwwName(curdir));
#else
	StrAllocCat(*href, curdir);
#endif /* DOSPATH */
#endif /* VMS */
    }

#ifdef VMS
    /*
     * On VMS, a file://localhost/ URL means
     * a listing for the login directory. - FM
     */
    if (!strcmp(*href, "file://localhost/"))
	StrAllocCat(*href, (HTVMS_wwwName((char *)Home_Dir())+1));
#endif /* VMS */

    FREE(temp);
    return;
}

#ifdef EXP_CHARTRANS
/*
**  This function writes a line with a META tag to an open file,
**  which will specify a charset parameter to use when the file is
**  read back in.  It is meant for temporary HTML files used by the
**  various special pages which may show titles of documents.  When those
**  files are created, the title strings normally have been translated and
**  expanded to the display character set, so we have to make sure they
**  don't get translated again.
**  If the user has changed the display character set during the lifetime
**  of the Lynx session (or, more exactly, during the time the title
**  strings to be written were generated), they may now have different
**  character encodings and there is currently no way to get it all right.
**  To change this, we would have to add a variable for each string which
**  keeps track of its character encoding...
**  But at least we can try to ensure that reading the file after future
**  display character set changes will give reasonable output.
**
**  The META tag is not written if the display character set (passed as
**  disp_chndl) already corresponds to the charset assumption that
**  would be made when the file is read. - KW
*/
PUBLIC void LYAddMETAcharsetToFD ARGS2(
	FILE *,		fd,
	int,		disp_chndl)
{
    if (disp_chndl == -1)
	/*
	 *  -1 means use current_char_set.
	 */
	disp_chndl = current_char_set;

    if (fd == NULL || disp_chndl < 0)
	/*
	 *  Should not happen.
	 */
	return;

    if (UCLYhndl_HTFile_for_unspec == disp_chndl)
	/*
	 *  Not need to do, so we don't.
	 */
	return;

    if (LYCharSet_UC[disp_chndl].enc == UCT_ENC_7BIT)
	/*
	 *  There shouldn't be any 8-bit characters in this case.
	 */
	return;

    /*
     *  In other cases we don't know because UCLYhndl_for_unspec may
     *  change during the lifetime of the file (by toggling raw mode
     *  or changing the display character set), so proceed.
     */
    fprintf(fd, "<META %s content=\"text/html;charset=%s\">\n",
		"http-equiv=\"content-type\"",
		LYCharSet_UC[disp_chndl].MIMEname);
}
#endif /* EXP_CHARTRANS */

/*
** This function returns OL TYPE="A" strings in
** the range of " A." (1) to "ZZZ." (18278). - FM
*/
PUBLIC char *LYUppercaseA_OL_String ARGS1(
	int, seqnum)
{
    static char OLstring[8];

    if (seqnum <= 1 ) {
        strcpy(OLstring, " A.");
        return OLstring;
    }
    if (seqnum < 27) {
        sprintf(OLstring, " %c.", (seqnum + 64));
        return OLstring;
    }
    if (seqnum < 703) {
        sprintf(OLstring, "%c%c.", ((seqnum-1)/26 + 64),
		(seqnum - ((seqnum-1)/26)*26 + 64));
        return OLstring;
    }
    if (seqnum < 18279) {
        sprintf(OLstring, "%c%c%c.", ((seqnum-27)/676 + 64),
		(((seqnum - ((seqnum-27)/676)*676)-1)/26 + 64),
		(seqnum - ((seqnum-1)/26)*26 + 64));
        return OLstring;
    }
    strcpy(OLstring, "ZZZ.");
    return OLstring;
}

/*
** This function returns OL TYPE="a" strings in
** the range of " a." (1) to "zzz." (18278). - FM
*/
PUBLIC char *LYLowercaseA_OL_String ARGS1(
	int, seqnum)
{
    static char OLstring[8];

    if (seqnum <= 1 ) {
        strcpy(OLstring, " a.");
        return OLstring;
    }
    if (seqnum < 27) {
        sprintf(OLstring, " %c.", (seqnum + 96));
        return OLstring;
    }
    if (seqnum < 703) {
        sprintf(OLstring, "%c%c.", ((seqnum-1)/26 + 96),
		(seqnum - ((seqnum-1)/26)*26 + 96));
        return OLstring;
    }
    if (seqnum < 18279) {
        sprintf(OLstring, "%c%c%c.", ((seqnum-27)/676 + 96),
		(((seqnum - ((seqnum-27)/676)*676)-1)/26 + 96),
		(seqnum - ((seqnum-1)/26)*26 + 96));
        return OLstring;
    }
    strcpy(OLstring, "zzz.");
    return OLstring;
}

/*
** This function returns OL TYPE="I" strings in the
** range of " I." (1) to "MMM." (3000).- FM
*/
PUBLIC char *LYUppercaseI_OL_String ARGS1(
	int, seqnum)
{
    static char OLstring[8];
    int Arabic = seqnum;

    if (Arabic >= 3000) {
        strcpy(OLstring, "MMM.");
        return OLstring;
    }

    switch(Arabic) {
    case 1:
        strcpy(OLstring, " I.");
        return OLstring;
    case 5:
        strcpy(OLstring, " V.");
        return OLstring;
    case 10:
        strcpy(OLstring, " X.");
        return OLstring;
    case 50:
        strcpy(OLstring, " L.");
        return OLstring;
    case 100:
        strcpy(OLstring, " C.");
        return OLstring;
    case 500:
        strcpy(OLstring, " D.");
        return OLstring;
    case 1000:
        strcpy(OLstring, " M.");
        return OLstring;
    default:
        OLstring[0] = '\0';
	break;
    }

    while (Arabic >= 1000) {
        strcat(OLstring, "M");
        Arabic -= 1000;
    }

    if (Arabic >= 900) {
        strcat(OLstring, "CM");
	Arabic -= 900;
    }

    if (Arabic >= 500) {
	strcat(OLstring, "D");
        Arabic -= 500;
	while (Arabic >= 500) {
	    strcat(OLstring, "C");
	    Arabic -= 10;
	}
    }

    if (Arabic >= 400) {
	strcat(OLstring, "CD");
        Arabic -= 400;
    }

    while (Arabic >= 100) {
        strcat(OLstring, "C");
        Arabic -= 100;
    }

    if (Arabic >= 90) {
        strcat(OLstring, "XC");
	Arabic -= 90;
    }

    if (Arabic >= 50) {
	strcat(OLstring, "L");
        Arabic -= 50;
	while (Arabic >= 50) {
	    strcat(OLstring, "X");
	    Arabic -= 10;
	}
    }

    if (Arabic >= 40) {
	strcat(OLstring, "XL");
        Arabic -= 40;
    }

    while (Arabic > 10) {
        strcat(OLstring, "X");
	Arabic -= 10;
    }    

    switch (Arabic) {
    case 1:
        strcat(OLstring, "I.");
	break;
    case 2:
        strcat(OLstring, "II.");
	break;
    case 3:
        strcat(OLstring, "III.");
	break;
    case 4:
        strcat(OLstring, "IV.");
	break;
    case 5:
        strcat(OLstring, "V.");
	break;
    case 6:
        strcat(OLstring, "VI.");
	break;
    case 7:
        strcat(OLstring, "VII.");
	break;
    case 8:
        strcat(OLstring, "VIII.");
	break;
    case 9:
        strcat(OLstring, "IX.");
	break;
    case 10:
        strcat(OLstring, "X.");
	break;
    default:
        strcat(OLstring, ".");
	break;
    }

    return OLstring;
}

/*
** This function returns OL TYPE="i" strings in
** range of " i." (1) to "mmm." (3000).- FM
*/
PUBLIC char *LYLowercaseI_OL_String ARGS1(
	int, seqnum)
{
    static char OLstring[8];
    int Arabic = seqnum;

    if (Arabic >= 3000) {
        strcpy(OLstring, "mmm.");
        return OLstring;
    }

    switch(Arabic) {
    case 1:
        strcpy(OLstring, " i.");
        return OLstring;
    case 5:
        strcpy(OLstring, " v.");
        return OLstring;
    case 10:
        strcpy(OLstring, " x.");
        return OLstring;
    case 50:
        strcpy(OLstring, " l.");
        return OLstring;
    case 100:
        strcpy(OLstring, " c.");
        return OLstring;
    case 500:
        strcpy(OLstring, " d.");
        return OLstring;
    case 1000:
        strcpy(OLstring, " m.");
        return OLstring;
    default:
        OLstring[0] = '\0';
	break;
    }

    while (Arabic >= 1000) {
        strcat(OLstring, "m");
        Arabic -= 1000;
    }

    if (Arabic >= 900) {
        strcat(OLstring, "cm");
	Arabic -= 900;
    }

    if (Arabic >= 500) {
	strcat(OLstring, "d");
        Arabic -= 500;
	while (Arabic >= 500) {
	    strcat(OLstring, "c");
	    Arabic -= 10;
	}
    }

    if (Arabic >= 400) {
	strcat(OLstring, "cd");
        Arabic -= 400;
    }

    while (Arabic >= 100) {
        strcat(OLstring, "c");
        Arabic -= 100;
    }

    if (Arabic >= 90) {
        strcat(OLstring, "xc");
	Arabic -= 90;
    }

    if (Arabic >= 50) {
	strcat(OLstring, "l");
        Arabic -= 50;
	while (Arabic >= 50) {
	    strcat(OLstring, "x");
	    Arabic -= 10;
	}
    }

    if (Arabic >= 40) {
	strcat(OLstring, "xl");
        Arabic -= 40;
    }

    while (Arabic > 10) {
        strcat(OLstring, "x");
	Arabic -= 10;
    }    

    switch (Arabic) {
    case 1:
        strcat(OLstring, "i.");
	break;
    case 2:
        strcat(OLstring, "ii.");
	break;
    case 3:
        strcat(OLstring, "iii.");
	break;
    case 4:
        strcat(OLstring, "iv.");
	break;
    case 5:
        strcat(OLstring, "v.");
	break;
    case 6:
        strcat(OLstring, "vi.");
	break;
    case 7:
        strcat(OLstring, "vii.");
	break;
    case 8:
        strcat(OLstring, "viii.");
	break;
    case 9:
        strcat(OLstring, "ix.");
	break;
    case 10:
        strcat(OLstring, "x.");
	break;
    default:
        strcat(OLstring, ".");
	break;
    }

    return OLstring;
}

/*
**  This function initializes the Ordered List counter. - FM
*/
PUBLIC void LYZero_OL_Counter ARGS1(
	HTStructured *, 	me)
{
    int i;

    if (!me)
        return;

    for (i = 0; i < 12; i++) {
        me->OL_Counter[i] = OL_VOID;
	me->OL_Type[i] = '1';
    }
	
    me->Last_OL_Count = 0;
    me->Last_OL_Type = '1';
    
    return;
}

#ifdef EXP_CHARTRANS
/*
**  This function is used by the HTML Structured object. - kw
*/
PUBLIC void LYGetChartransInfo ARGS1(
	HTStructured *,		me)
{
    me->UCLYhndl = HTAnchor_getUCLYhndl(me->node_anchor,
					UCT_STAGE_STRUCTURED);
    if (me->UCLYhndl < 0) {
	int chndl = HTAnchor_getUCLYhndl(me->node_anchor, UCT_STAGE_HTEXT);

	if (chndl < 0) {
	    chndl = current_char_set;
	    HTAnchor_setUCInfoStage(me->node_anchor, chndl, UCT_STAGE_HTEXT,
				    UCT_SETBY_STRUCTURED);
	}
	HTAnchor_setUCInfoStage(me->node_anchor, chndl,
				UCT_STAGE_STRUCTURED, UCT_SETBY_STRUCTURED);
	me->UCLYhndl = HTAnchor_getUCLYhndl(me->node_anchor,
					    UCT_STAGE_STRUCTURED);
    }
    me->UCI = HTAnchor_getUCInfoStage(me->node_anchor, UCT_STAGE_STRUCTURED);
}

#endif /* EXP_CHARTRANS */

/*
**  This function reallocates an allocated string with
**  8-bit printable Latin characters (>= 160) converted
**  to their HTML entity names and then translated for
**  the current character set. - FM
*/
PUBLIC void LYExpandString ARGS1(
	char **, str)
{
    char *p = *str;
    char *q = *str;
    CONST char *name;
    int i, j, value, high, low, diff = 0;

    /*
    **  Don't do anything if we have no string
    **  or are in CJK mode. - FM
    */
    if (!p || *p == '\0' ||
        HTCJK != NOCJK)
        return;

    /*
    **  Start a clean copy of the string, without
    **  invalidating our pointer to the original. - FM
    */
    *str = NULL;
    StrAllocCopy(*str, "");

    /*
    **  Check each character in the original string,
    **  and add the characters or substitutions to
    **  our clean copy. - FM
    */
    for (i = 0; p[i]; i++) {
	/*
	**  Substitute Lynx special character for
	**  160 (nbsp) if HTPassHighCtrlRaw is not
	**  set. - FM
	*/
        if (((unsigned char)p[i]) == 160 &&
	    !HTPassHighCtrlRaw) {
	    p[i] = HT_NON_BREAK_SPACE;
	/*
	**  Substitute Lynx special character for
	**  173 (shy) if HTPassHighCtrlRaw is not
	**  set. - FM
	*/
        } else if (((unsigned char)p[i]) == 173 &&
	    !HTPassHighCtrlRaw) {
	    p[i] = LY_SOFT_HYPHEN;
	/*
	**  Substitute other 8-bit characters based on
	**  the LYCharsets.c tables if HTPassEightBitRaw
	**  is not set. - FM
	*/
	} else if (((unsigned char)p[i]) > 160 &&
		   !HTPassEightBitRaw) {
	    value = (int)(((unsigned char)p[i]) - 160);
	    p[i] = '\0';
	    StrAllocCat(*str, q);
	    q = &p[i+1];
	    name = HTMLGetEntityName(value);
	    for (low = 0, high = HTML_dtd.number_of_entities;
		 high > low;
		 diff < 0 ? (low = j+1) : (high = j)) {
		/* Binary search */
		j = (low + (high-low)/2);
		diff = strcmp(HTML_dtd.entity_names[j], name);
		if (diff == 0) {
		    StrAllocCat(*str, p_entity_values[j]);
		    break;
		}
	    }
	}
    }
    StrAllocCat(*str, q);
    free_and_clear(&p);
}

/*
**  This function converts HTML named entities within a string
**  to their translations in the active LYCharSets.c array.
**  It also converts numeric entities to their HTML entity names
**  and then similarly translates those.  The string is converted
**  in place, on the assumption that the conversion strings will
**  not be longer than the entity strings, such that the overall
**  string will never grow.  This assumption is true for the
**  current LYCharSets arrays.  Make sure it stays true!  If
**  plain_space is TRUE, nbsp (160) will be treated as an ASCII
**  space (32).  If hidden is TRUE, entities will be translated
**  but escape sequences will be passed unaltered. - FM
*/
PUBLIC char * LYUnEscapeEntities ARGS3(
	char *,	str,
	BOOLEAN, plain_space,
	BOOLEAN, hidden)
{
    char * p = str;
    char * q = str;
    char * cp;
    char cpe;
    int len, value;
    int high, low, diff = 0, i;
    enum _state
        { S_text, S_esc, S_dollar, S_paren,
	  S_nonascii_text, S_dollar_paren } state = S_text;

    /*
    **  Make sure we have a non-empty string. - FM
    */
    if (!str || *str == '\0')
        return str;

    /*
    **  Loop through string, making conversions as needed. - FM
    */
    while (*p) {
        if (HTCJK != NOCJK && !hidden) {
	    /*
	    **  Handle CJK escape sequences, based on patch
	    **  from Takuya ASADA (asada@three-a.co.jp). - FM
	    */
	    switch(state) {
	        case S_text:
		    if (*p == '\033') {
		        state = S_esc;
			*q++ = *p++;
			continue;
		    }
		    break;

		case S_esc:
		    if (*p == '$') {
		        state = S_dollar;
			*q++ = *p++;
			continue;
		    } else if (*p == '(') {
		        state = S_paren;
			*q++ = *p++;
			continue;
		    } else {
		        state = S_text;
		    }

		case S_dollar:
		    if (*p == '@' || *p == 'B' || *p == 'A') {
		        state = S_nonascii_text;
			*q++ = *p++;
			continue;
		    } else if (*p == '(') {
		        state = S_dollar_paren;
			*q++ = *p++;
			continue;
		    } else {
		        state = S_text;
		    }
		    break;

		case S_dollar_paren:
		    if (*p == 'C') {
		        state = S_nonascii_text;
			*q++ = *p++;
			continue;
		    } else {
		        state = S_text;
		    }
		    break;

		case S_paren:
		    if (*p == 'B' || *p == 'J' || *p == 'T')  {
		        state = S_text;
			*q++ = *p++;
			continue;
		    } else if (*p == 'I') {
		        state = S_nonascii_text;
			*q++ = *p++;
			continue;
		    } else {
		        state = S_text;
		    }
		    break;

		case S_nonascii_text:
		    if (*p == '\033')
		        state = S_esc;
		    *q++ = *p++;
		    continue;
		    break;

		default:
	            p++;
		    continue;
	    }
        } else if (*p == '\033' &&
		   !hidden) {
	    /*
	    **  CJK handling not on, and not a hidden INPUT,
	    **  so block escape. - FM
	    */
	    p++;
	    continue;
	}

	/*
	**  Check for a numeric or named entity. - FM
	*/
        if (*p == '&') {
	    BOOL isHex = FALSE;
	    BOOL isDecimal = FALSE;
	    p++;
	    len = strlen(p);
	    /*
	    **  Check for a numeric entity. - FM
	    */
	    if (*p == '#' && len > 2 &&
	        (unsigned char)*(p+1) == 'x' &&
		(unsigned char)*(p+2) < 127 && 
		isxdigit((unsigned char)*(p+2))) {
		isHex = TRUE;
	    } else if (*p == '#' && len > 2 &&
		       (unsigned char)*(p+1) < 127 &&
		       isdigit((unsigned char)*(p+1))) {
		isDecimal = TRUE;
	    }
	    if (isHex || isDecimal) {
		if (isHex) {
		    p += 2;
		    cp = p;
		} else {
		    cp = ++p;
		}
		while (*p && (unsigned char)*p < 127 &&
		       (isHex ? isxdigit((unsigned char)*p) :
				isdigit((unsigned char)*p))) {
		    p++;
		}
		/*
		**  Save the terminator and isolate the digit(s). - FM
		*/
		cpe = *p;
		if (*p)
		    *p++ = '\0';
	        /*
		** Show the numeric entity if the value:
		**  (1) Is greater than 255 and unhandled Unicode.
		**  (2) Is less than 32, and not valid or we don't
		**	have HTCJK set.
		**  (3) Is 127 and we don't have HTPassHighCtrlRaw
		**	or HTCJK set.
		**  (4) Is 128 - 159 and we don't have HTPassHighCtrlNum set.
		*/
		if (((isHex ? sscanf(cp, "%x", &value) :
			      sscanf(cp, "%d", &value)) != 1) ||
		    (value > 255 &&
		     value != 8194 && value != 8195 && value != 8201 &&
		     value != 8211 && value != 8212 && value != 8482) ||
		    (value < 32 &&
		     value != 9 && value != 10 && value != 13 &&
		     HTCJK == NOCJK) ||
		    (value == 127 &&
		     !(HTPassHighCtrlRaw || HTCJK != NOCJK)) ||
		    (value > 127 && value < 160 &&
		     !HTPassHighCtrlNum)) {
		    /*
		    **  Illegal or not yet handled value.
		    **  Recover the "&#" and continue
		    **  from there. - FM
		    */
		    *q++ = '&';
		    *q++ = '#';
		    if (isHex)
		        *q++ = 'x';
		    if (cpe != '\0')
		       *(p-1) = cpe;
		    p = cp;
		    continue;
		/*
		**  For 160 (nbsp), use that value if it's
		**  a hidden INPUT, otherwise use an ASCII
		**  space (32) if plain_space is TRUE,
		**  otherwise use the Lynx special character. - FM
		*/
		} else if (value == 160) {
		    if (hidden) {
		        *q++ = 160;
		    } else if (plain_space) {
		        *q++ = ' ';
		    } else {
		        *q++ = HT_NON_BREAK_SPACE;
		    }
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		/*
		**  For 173 (shy), use that value if it's
		**  a hidden INPUT, otherwise ignore it if
		**  plain space is TRUE, otherwise use the
		**  Lynx special character. - FM
		*/
		} else if (value == 173) {
		    if (hidden) {
		        *q++ = 173;
		    } else if (plain_space) {
		        ;
		    } else {
		        *q++ = LY_SOFT_HYPHEN;
		    }
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		/*
		**  For 8194 (ensp), 8195 (emsp), or 8201 (thinsp),
		**  use the character reference if it's a hidden INPUT,
		**  otherwise use an ASCII space (32) if plain_space is
		**  TRUE, otherwise use the Lynx special character. - FM
		*/
		} else if (value == 8194 || value == 8195 || value == 8201) {
		    if (hidden) {
		        *q++ = '&';
		        *q++ = '#';
			if (isHex)
			    *q++ = 'x';
			if (cpe != '\0')
			    *(p-1) = cpe;
			p = cp;
			continue;
		    } else if (plain_space) {
		        *q++ = ' ';
		    } else {
		        *q++ = HT_EM_SPACE;
		    }
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		/*
		**  For 8211 (ndash) or 8212 (mdash), use the character
		**  reference if it's a hidden INPUT, otherwise use an
		**  ASCII dash. - FM
		*/
		} else if (value == 8211 || value == 8212) {
		    if (hidden) {
		        *q++ = '&';
		        *q++ = '#';
			if (isHex)
			    *q++ = 'x';
			if (cpe != '\0')
			    *(p-1) = cpe;
			p = cp;
			continue;
		    } else {
		        *q++ = '-';
		    }
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		/*
		**  For 8482 (trade) use the character reference if it's
		**  a hidden INPUT, otherwise use whatever the tables have
		**  for &trade;. - FM & KW
		*/
	        } else if (value == 8482 && hidden) {
		        *q++ = '&';
		        *q++ = '#';
			if (isHex)
			    *q++ = 'x';
			if (cpe != '\0')
			    *(p-1) = cpe;
			p = cp;
			continue;
		/*
		**  If it's ASCII, or is 8-bit but HTPassEightBitNum
		**  is set or the character set is "ISO Latin 1",
		**  use it's value. - FM
		*/
		} else if (value < 161 || 
			   (value < 256 &&
			    (HTPassEightBitNum ||
			     !strncmp(LYchar_set_names[current_char_set],
				      "ISO Latin 1", 11)))) {
		    /*
		    **  No conversion needed.
		    */
	            *q++ = (unsigned char)value;
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		/*
		**  If we get to here, convert and handle
		**  the character as a named entity. - FM
		*/
		} else {
		    CONST char * name;
		    if (value == 8482) {
			/*
			**  Trade mark sign falls through to here. - KW
			*/
			name = "trade";
		    } else {
			value -= 160;
			name = HTMLGetEntityName(value);
		    }
		    for (low = 0, high = HTML_dtd.number_of_entities;
			 high > low;
			 diff < 0 ? (low = i+1) : (high = i)) {
			/*
			**  Binary search.
			*/
			i = (low + (high-low)/2);
			diff = strcmp(HTML_dtd.entity_names[i], name);
			if (diff == 0) {
			    /*
			    **  Found the entity.  Assume that the length
			    **  of the value does not exceed the length of
			    **  the raw entity, so that the overall string
			    **  does not need to grow.  Make sure this stays
			    **  true in the LYCharSets arrays. - FM
			    */
			    int j;
			    for (j = 0; p_entity_values[i][j]; j++)
			        *q++ = (unsigned char)(p_entity_values[i][j]);
			    break;
			}
		    }
		    /*
		    **  Recycle the terminator if it isn't the
		    **  standard ';' for HTML. - FM
		    */
		    if (cpe != ';' && cpe != '\0') {
		        p--;
			*p = cpe;
		    }
		    continue;
		}
	    /*
	    **  Check for a named entity. - FM
	    */
	    } else if ((unsigned char)*p < 127 &&
	    	       isalnum((unsigned char)*p)) {
		cp = p;
		while (*cp && (unsigned char)*cp < 127 &&
		       isalnum((unsigned char)*cp))
		    cp++;
		cpe = *cp;
		*cp = '\0';
		for (low = 0, high = HTML_dtd.number_of_entities;
		     high > low ;
		     diff < 0 ? (low = i+1) : (high = i)) {
		    /*
		    **  Binary search.
		    */
		    i = (low + (high-low)/2);
		    diff = strcmp(HTML_dtd.entity_names[i], p);
		    if (diff == 0) {
		        /*
			**  Found the entity.  Assume that the length
			**  of the value does not exceed the length of
			**  the raw entity, so that the overall string
			**  does not need to grow.  Make sure this stays
			**  true in the LYCharSets arrays. - FM
			*/
			int j;
			/*
			**  If it's hidden, use 160 for nbsp. - FM
			*/
			if (hidden &&
			    !strcmp("nbsp", HTML_dtd.entity_names[i])) {
			    *q++ = 160;
			/*
			**  If it's hidden, use 173 for shy. - FM
			*/
			} else if (hidden &&
				   !strcmp("shy", HTML_dtd.entity_names[i])) {
			    *q++ = 173;
			/*
			**  Check whether we want a plain space for nbsp,
			**  ensp or emsp. - FM
			*/
			} else if (plain_space &&
				   (!strcmp("nbsp",
				   	    HTML_dtd.entity_names[i]) ||
				    !strcmp("emsp",
					    HTML_dtd.entity_names[i]) ||
				    !strcmp("ensp", 
					    HTML_dtd.entity_names[i]))) {
			    *q++ = ' ';
			/*
			**  If plain_space is set, ignore shy. - FM
			*/
			} else if (plain_space &&
				   !strcmp("shy",
					    HTML_dtd.entity_names[i])) {
			    ;
			/*
			**  If we haven't used something else, use the
			**  the translated value or string. - FM
			*/
			} else {
			    for (j = 0; p_entity_values[i][j]; j++) {
			        *q++ = (unsigned char)(p_entity_values[i][j]);
			    }
			}
			/*
			**  Recycle the terminator if it isn't the
			**  standard ';' for HTML. - FM
			*/
			*cp = cpe;
			if (*cp != ';')
			    p = cp;
			else
			    p = (cp+1);
			break;
		    }
		}
		*cp = cpe;
		if (diff != 0) {
		    /*
		    **  Entity not found.  Add the '&' and
		    **  continue processing from there. - FM
		    */
		    *q++ = '&';
		}
		continue;
	    /*
	    **  If we get to here, it's a raw ampersand. - FM
	    */
	    } else {
		*q++ = '&';
		continue;
	    }
	/*
	**  Not an entity.  Check whether we want nbsp, ensp,
	**  emsp (if translated elsewhere) or 160 converted to
	**  a plain space. - FM
	*/
	} else {
	    if ((plain_space) &&
	        (*p == HT_NON_BREAK_SPACE || *p == HT_EM_SPACE ||
		 (((unsigned char)*p) == 160 &&
		  !(hidden ||
		    HTPassHighCtrlRaw || HTPassHighCtrlNum ||
		    HTCJK != NOCJK)))) {
	        *q++ = ' ';
		p++;
	    } else if (!hidden && *p == 10 && q != str && *(q-1) == 13) {
		/*
		 *  If this is not a hidden string, and the current char
		 *  is the LF ('\n') of a CRLF pair, drop the CR ('\r'). - kw
		 */
	        *(q-1) = *p++;
	    } else {
	        *q++ = *p++;
	    }
	}
    }
    
    *q = '\0';
    return str;
}

/*
**  This function converts any named or numeric character
**  references in allocated strings to their ISO-8858-1
**  values or to our substitutes if they are not part of
**  that charset.  If the isURL flag is TRUE, it also
**  hex escapes ESC and any characters greater than 159,
**  and trims any leading or trailing blanks.  Otherwise,
**  it strips out ESC, as would be done when the
**  "ISO Latin 1" Character Set is selected. - FM
*/
PUBLIC void LYUnEscapeToLatinOne ARGS2(
	char **,	str,
	BOOLEAN,	isURL)
{
    char *p = *str;
    char *q = NULL;
    char *url = NULL;
    char *esc = NULL;
    char buf[2];
    int e;
    char *cp;
    char cpe;
    int len, value;
    int high, low, diff = 0, i;
    enum _state
        { S_text, S_esc, S_dollar, S_paren,
	  S_nonascii_text, S_dollar_paren } state = S_text;

    /*
    **  Make sure we have a non-empty string. - FM
    */
    if (!p || *p == '\0')
        return;
    buf[0] = buf[1] = '\0';

    /*
    **  If the isURL flag is TRUE, set up for hex escaping. - FM
    */
    if (isURL == TRUE) {
        if ((url = (char *)calloc(1, ((strlen(p) * 3) + 1))) == NULL) {
	    outofmem(__FILE__, "LYUnEscapeToLatinOne");
	}
	q = url;
    } else {
        q = p;
    }

    /*
    **  Loop through string, making conversions as needed. - FM
    */
    while (*p) {
	/*
	**  Handle any CJK escape sequences. - FM
	*/
	switch(state) {
	    case S_text:
		if (*p == '\033') {
		    state = S_esc;
		    if (isURL == TRUE) {
		        buf[0] = *p;
		        esc = HTEscape(buf, URL_XALPHAS);
			for (e = 0; esc[e]; e++)
		            *q++ = esc[e];
			FREE(esc);
		    }
		    p++;
		    continue;
		}
		break;

	    case S_esc:
		if (*p == '$') {
		    state = S_dollar;
		    *q++ = *p++;
		    continue;
		} else if (*p == '(') {
		    state = S_paren;
		   *q++ = *p++;
		   continue;
		} else {
		    state = S_text;
		}

	    case S_dollar:
		if (*p == '@' || *p == 'B' || *p == 'A') {
		    state = S_nonascii_text;
		    *q++ = *p++;
		    continue;
		} else if (*p == '(') {
		    state = S_dollar_paren;
		    *q++ = *p++;
		    continue;
		} else {
		    state = S_text;
		}
		break;

	   case S_dollar_paren:
		if (*p == 'C') {
		    state = S_nonascii_text;
		    *q++ = *p++;
		    continue;
		} else {
		    state = S_text;
		}
		break;

	    case S_paren:
		if (*p == 'B' || *p == 'J' || *p == 'T')  {
		    state = S_text;
		    *q++ = *p++;
		    continue;
		} else if (*p == 'I') {
		    state = S_nonascii_text;
		    *q++ = *p++;
		    continue;
		} else {
		    state = S_text;
		}
		break;

	    case S_nonascii_text:
		if (*p == '\033') {
		    state = S_esc;
		    if (isURL == TRUE) {
		        buf[0] = *p;
		        esc = HTEscape(buf, URL_XALPHAS);
			for (e = 0; esc[e]; e++)
		            *q++ = esc[e];
			FREE(esc);
		    }
		    p++;
		} else {
		    *q++ = *p++;
		}
		continue;
		break;

	    default:
	        p++;
		continue;
	}

	/*
	**  Check for a numeric or named entity. - FM
	*/
        if (*p == '&') {
	    p++;
	    len = strlen(p);
	    /*
	    **  Check for a numeric entity. - FM
	    */
	    if (*p == '#' && len > 2 &&
	        (unsigned char)*(p+1) < 127 &&
		isdigit((unsigned char)*(p+1))) {
		cp = ++p;
		while (*p && (unsigned char)*p < 127 &&
		       isdigit((unsigned char)*p))
		    p++;
		cpe = *p;
		if (*p)
		    *p++ = '\0';
	        /*
		** Show the numeric entity if the value:
		**  (1) Is greater than 255 (until we support Unicode).
		**  (2) Is less than 32 and not valid.
		**  (3) Is 127.
		**  (4) Is 128 - 159.
		*/
		if ((sscanf(cp, "%d", &value) != 1) ||
		    (value > 255) ||
		    (value < 32 &&
		     value != 9 && value != 10 && value != 13) ||
		    (value == 127) ||
		    (value > 127 && value < 160)) {
		    /*
		    **  Illegal or not yet handled value.
		    **  Recover the "&#" and continue
		    **  from there. - FM
		    */
		    *q++ = '&';
		    *q++ = '#';
		    if (cpe != '\0')
		       *(p-1) = cpe;
		    p = cp;
		    continue;
		}
		/*
		**  Convert the value as an unsigned char,
		**  hex escaped if isURL is set and it's
		**  8-bit, and then recycle the terminator
		**  if it is not a semicolon. - FM
		*/
		if (value > 159 && isURL == TRUE) {
		    buf[0] = value;
		    esc = HTEscape(buf, URL_XALPHAS);
		    for (e = 0; esc[e]; e++)
		        *q++ = esc[e];
		    FREE(esc);
		} else {
	            *q++ = (unsigned char)value;
		}
		if (cpe != ';' && cpe != '\0') {
		    p--;
		    *p = cpe;
		}
		continue;
	    /*
	    **  Check for a named entity. - FM
	    */
	    } else if ((unsigned char)*p < 127 &&
	    	       isalnum((unsigned char)*p)) {
		cp = p;
		while (*cp && (unsigned char)*cp < 127 &&
		       isalnum((unsigned char)*cp))
		    cp++;
		cpe = *cp;
		*cp = '\0';
		for (low = 0, high = HTML_dtd.number_of_entities;
		     high > low ;
		     diff < 0 ? (low = i+1) : (high = i)) {
		    /*
		    **  Binary search.
		    */
		    i = (low + (high-low)/2);
		    diff = strcmp(HTML_dtd.entity_names[i], p);
		    if (diff == 0) {
		        /*
			**  Found the entity. Convert it to
			**  an ISO-8859-1 character, or our
			**  substitute for any non-ISO-8859-1
			**  character, hex escaped if isURL
			**  is set and it's 8-bit. - FM
			*/
			buf[0] = HTMLGetLatinOneValue(i);
                        if (buf[0] == '\0') {
                            /*
                            **  The entity does not have an 8859-1
			    **  representation of exactly one char length.
			    **  Try to deal with it anyway - either HTEscape
			    **  the whole mess, or pass through raw.  So
			    **  make sure the ISO_Latin1 table, which is the
			    **  first table in LYCharSets, has reasonable
			    **  substitution strings! (if it really must
			    **  have any longer than one char) - KW
                            */
			    if (!LYCharSets[0][i][0]) {
				/*
				**  Totally empty, skip. - KW
				*/
			        ; /* do nothing */
			    } else if (isURL) {
				/*
				**  All will be HTEscape'd. - KW
				*/
				esc = HTEscape(LYCharSets[0][i], URL_XALPHAS);
				for (e = 0; esc[e]; e++)
				    *q++ = esc[e];
				FREE(esc);
			    } else {
				/*
				**  Nothing will be HTEscape'd. - KW 
				*/
				for (e = 0; LYCharSets[0][i][e]; e++) {
				    *q++ =
					(unsigned char)(LYCharSets[0][i][e]);
				}
			    }
		        } else if ((unsigned char)buf[0] > 159 &&
				   isURL == TRUE) {
			    esc = HTEscape(buf, URL_XALPHAS);
			    for (e = 0; esc[e]; e++)
				*q++ = esc[e];
			    FREE(esc);
			} else {
			    *q++ = buf[0];
			}
			/*
			**  Recycle the terminator if it isn't
			**  the standard ';' for HTML. - FM
			*/
			*cp = cpe;
			if (*cp != ';')
			    p = cp;
			else
			    p = (cp+1);
			break;
		    }
		}
		*cp = cpe;
		if (diff != 0) {
		    /*
		    **  Entity not found.  Add the '&' and
		    **  continue processing from there. - FM
		    */
		    *q++ = '&';
		}
		continue;
	    /*
	    **  If we get to here, it's a raw ampersand. - FM
	    */
	    } else {
		*q++ = '&';
		continue;
	    }
	/*
	**  Not an entity.  Use the character. - FM
	*/
	} else {
	  *q++ = *p++;
	}
    }
    
    /*
    **  Clean up and return. - FM
    */
    *q = '\0';
    if (isURL == TRUE) {
        LYTrimHead(url);
	LYTrimTail(url);
        StrAllocCopy(*str, url);
	FREE(url);
    }
}

/*
**  This function processes META tags in HTML streams. - FM
*/
PUBLIC void LYHandleMETA ARGS4(
	HTStructured *, 	me,
	CONST BOOL*,	 	present,
	CONST char **,		value,
	char **,		include)
{
    char *http_equiv = NULL, *name = NULL, *content = NULL;
    char *href = NULL, *id_string = NULL, *temp = NULL;
    char *cp, *cp0, *cp1 = NULL;
    int url_type = 0, i;

    if (!me || !present)
        return;

    /*
     *  Load the attributes for possible use by Lynx. - FM
     */
    if (present[HTML_META_HTTP_EQUIV] &&
	value[HTML_META_HTTP_EQUIV] && *value[HTML_META_HTTP_EQUIV]) {
	StrAllocCopy(http_equiv, value[HTML_META_HTTP_EQUIV]);
	convert_to_spaces(http_equiv, TRUE);
	LYUnEscapeToLatinOne(&http_equiv, FALSE);
	LYTrimHead(http_equiv);
	LYTrimTail(http_equiv);
	if (*http_equiv == '\0') {
	    FREE(http_equiv);
	}
    }
    if (present[HTML_META_NAME] &&
	value[HTML_META_NAME] && *value[HTML_META_NAME]) {
	StrAllocCopy(name, value[HTML_META_NAME]);
	convert_to_spaces(name, TRUE);
	LYUnEscapeToLatinOne(&name, FALSE);
	LYTrimHead(name);
	LYTrimTail(name);
	if (*name == '\0') {
	    FREE(name);
	}
    }
    if (present[HTML_META_CONTENT] &&
	value[HTML_META_CONTENT] && *value[HTML_META_CONTENT]) {
	/*
	 *  Technically, we should be creating a comma-separated
	 *  list, but META tags come one at a time, and we'll
	 *  handle (or ignore) them as each is received.  Also,
	 *  at this point, we only trim leading and trailing
	 *  blanks from the CONTENT value, without translating
	 *  any named entities or numeric character references,
	 *  because how we should do that depends on what type
	 *  of information it contains, and whether or not any
	 *  of it might be sent to the screen. - FM
	 */
	StrAllocCopy(content, value[HTML_META_CONTENT]);
	convert_to_spaces(content, FALSE);
	LYTrimHead(content);
	LYTrimTail(content);
	if (*content == '\0') {
	    FREE(content);
	}
    }
    if (TRACE) {
	fprintf(stderr,
	        "LYHandleMETA: HTTP-EQUIV=\"%s\" NAME=\"%s\" CONTENT=\"%s\"\n",
		(http_equiv ? http_equiv : "NULL"),
		(name ? name : "NULL"),
		(content ? content : "NULL"));
    }

    /*
     *  Make sure we have META name/value pairs to handle. - FM
     */
    if (!(http_equiv || name) || !content)
        goto free_META_copies;
		
    /*
     * Check for a no-cache Pragma
     * or Cache-Control directive. - FM
     */
    if (!strcasecomp((http_equiv ? http_equiv : ""), "Pragma") ||
        !strcasecomp((http_equiv ? http_equiv : ""), "Cache-Control")) {
	LYUnEscapeToLatinOne(&content, FALSE);
	LYTrimHead(content);
	LYTrimTail(content);
	if (!strcasecomp(content, "no-cache")) {
	    me->node_anchor->no_cache = TRUE;
	    HText_setNoCache(me->text);
	}

	/*
	 *  If we didn't get a Cache-Control MIME header,
	 *  and the META has one, convert to lowercase,
	 *  store it in the anchor element, and if we
	 *  haven't yet set no_cache, check whether we
	 *  should. - FM
	 */
	if ((!me->node_anchor->cache_control) &&
	    !strcasecomp((http_equiv ? http_equiv : ""), "Cache-Control")) {
	    for (i = 0; content[i]; i++)
		 content[i] = TOLOWER(content[i]);
	    StrAllocCopy(me->node_anchor->cache_control, content);
	    if (me->node_anchor->no_cache == FALSE) {
	        cp0 = content;
		while ((cp = strstr(cp0, "no-cache")) != NULL) {
		    cp += 8;
		    while (*cp != '\0' && WHITE(*cp))
			cp++;
		    if (*cp == '\0' || *cp == ';') {
			me->node_anchor->no_cache = TRUE;
			HText_setNoCache(me->text);
			break;
		    }
		    cp0 = cp;
		}
		if (me->node_anchor->no_cache == TRUE)
		    goto free_META_copies;
		cp0 = content;
		while ((cp = strstr(cp0, "max-age")) != NULL) {
		    cp += 7;
		    while (*cp != '\0' && WHITE(*cp))
			cp++;
		    if (*cp == '=') {
			cp++;
			while (*cp != '\0' && WHITE(*cp))
			    cp++;
			if (isdigit((unsigned char)*cp)) {
			    cp0 = cp;
			    while (isdigit((unsigned char)*cp))
				cp++;
			    if (*cp0 == '0' && cp == (cp0 + 1)) {
			        me->node_anchor->no_cache = TRUE;
				HText_setNoCache(me->text);
				break;
			    }
			}
		    }
		    cp0 = cp;
		}
	    }
	}

    /*
     * Check for an Expires directive. - FM
     */
    } else if (!strcasecomp((http_equiv ? http_equiv : ""), "Expires")) {
	/*
	 *  If we didn't get an Expires MIME header,
	 *  store it in the anchor element, and if we
	 *  haven't yet set no_cache, check whether we
	 *  should.  Note that we don't accept a Date
	 *  header via META tags, because it's likely
	 *  to be untrustworthy, but do check for a
	 *  Date header from a server when making the
	 *  comparsion. - FM
	 */
	LYUnEscapeToLatinOne(&content, FALSE);
	LYTrimHead(content);
	LYTrimTail(content);
	StrAllocCopy(me->node_anchor->expires, content);
	if (me->node_anchor->no_cache == FALSE) {
	    if (!strcmp(content, "0")) {
		/*
		 *  The value is zero, which we treat as
		 *  an absolute no-cache directive. - FM
		 */
		me->node_anchor->no_cache = TRUE;
		HText_setNoCache(me->text);
	    } else if (me->node_anchor->date != NULL) {
	        /*
		 *  We have a Date header, so check if
		 *  the value is less than or equal to
		 *  that. - FM
		 */
		if (LYmktime(content, TRUE) <=
		    LYmktime(me->node_anchor->date, TRUE)) {
		    me->node_anchor->no_cache = TRUE;
		    HText_setNoCache(me->text);
		}
	    } else if (LYmktime(content, FALSE) <= 0) {
	        /*
		 *  We don't have a Date header, and
		 *  the value is in past for us. - FM
		 */
		me->node_anchor->no_cache = TRUE;
		HText_setNoCache(me->text);
	    }
	}

    /*
     *  Check for a text/html Content-Type with a
     *  charset directive, if we didn't already set
     *  the charset via a server's header. - AAC & FM
     */
    } else if (!(me->node_anchor->charset && *me->node_anchor->charset) && 
	       !strcasecomp((http_equiv ? http_equiv : ""), "Content-Type")) {
	LYUnEscapeToLatinOne(&content, FALSE);
	LYTrimHead(content);
	LYTrimTail(content);
	/*
	 *  Force the Content-type value to all lower case. - FM
	 */
	for (cp = content; *cp; cp++)
	    *cp = TOLOWER(*cp);

	if ((cp = strstr(content, "text/html;")) != NULL &&
	    (cp1 = strstr(content, "charset")) != NULL &&
	    cp1 > cp) {
	    BOOL chartrans_ok = NO;
	    char *cp3 = NULL, *cp4;
	    int chndl;

	    cp1 += 7;
	    while (*cp1 == ' ' || *cp1 == '=' || *cp1 == '"')
	        cp1++;
#ifdef EXP_CHARTRANS
	    StrAllocCopy(cp3, cp1); /* copy to mutilate more */
	    for (cp4 = cp3; (*cp4 != '\0' && *cp4 != '"' &&
			     *cp4 != ';'  && *cp4 != ':' &&
			     !WHITE(*cp4)); cp4++) {
		; /* do nothing */
	    }
	    *cp4 = '\0';
	    cp4 = cp3;
	    chndl = UCGetLYhndl_byMIME(cp3);
	    if (chndl < 0) {
		if (!strcmp(cp4, "cn-big5")) {
		    cp4 += 3;
		    chndl = UCGetLYhndl_byMIME(cp4);
		} else if (!strncmp(cp4, "cn-gb", 5)) {
		    StrAllocCopy(cp3, "gb2312");
		    cp4 = cp3;
		    chndl = UCGetLYhndl_byMIME(cp4);
		}
	    }
	    if (UCCanTranslateFromTo(chndl, current_char_set)) {
		chartrans_ok = YES;
		StrAllocCopy(me->node_anchor->charset, cp4);
		HTAnchor_setUCInfoStage(me->node_anchor, chndl,
					UCT_STAGE_PARSER,
					UCT_SETBY_STRUCTURED);
	    } else if (chndl < 0) {
		/*
		 *  Got something but we don't recognize it.
		 */
		chndl = UCLYhndl_for_unrec;
		if (UCCanTranslateFromTo(chndl, current_char_set)) {
		    chartrans_ok = YES;
		    HTAnchor_setUCInfoStage(me->node_anchor, chndl,
					    UCT_STAGE_PARSER,
					    UCT_SETBY_STRUCTURED);
		}
	    }
	    FREE(cp3);
	    if (chartrans_ok) {
		LYUCcharset * p_in =
				HTAnchor_getUCInfoStage(me->node_anchor,
							UCT_STAGE_PARSER);
		LYUCcharset * p_out =
				HTAnchor_setUCInfoStage(me->node_anchor,
							current_char_set,
							UCT_STAGE_HTEXT,
							UCT_SETBY_DEFAULT);
		if (!p_out) {
		    /*
		     *  Try again.
		     */
		    p_out = HTAnchor_getUCInfoStage(me->node_anchor,
						    UCT_STAGE_HTEXT);
		}
		if (!strcmp(p_in->MIMEname, "x-transparent")) {
		    HTPassEightBitRaw = TRUE;
		    HTAnchor_setUCInfoStage(me->node_anchor,
				HTAnchor_getUCLYhndl(me->node_anchor,
						     UCT_STAGE_HTEXT),
						     UCT_STAGE_PARSER,
						     UCT_SETBY_DEFAULT);
		}
		if (!strcmp(p_out->MIMEname, "x-transparent")) {
		    HTPassEightBitRaw = TRUE;
		    HTAnchor_setUCInfoStage(me->node_anchor,
				HTAnchor_getUCLYhndl(me->node_anchor,
						     UCT_STAGE_PARSER),
						     UCT_STAGE_HTEXT,
						     UCT_SETBY_DEFAULT);
		}
		if (p_in->enc != UCT_ENC_CJK &&
		    (p_in->codepoints & UCT_CP_SUBSETOF_LAT1)) {
		    HTCJK = NOCJK;
		} else if (chndl == current_char_set) {
		    HTPassEightBitRaw = TRUE;
		}
		LYGetChartransInfo(me);
	    /*
	     *  Fall through to old behavior.
	     */
			} else
#endif /* EXP_CHARTRANS */
	    if (!strncmp(cp1, "us-ascii", 8) ||
		       !strncmp(cp1, "iso-8859-1", 10)) {
		StrAllocCopy(me->node_anchor->charset, "iso-8859-1");
		HTCJK = NOCJK;

	    } else if (!strncmp(cp1, "iso-8859-2", 10) &&
		       !strncmp(LYchar_set_names[current_char_set],
				"ISO Latin 2", 11)) {
		StrAllocCopy(me->node_anchor->charset, "iso-8859-2");
		HTPassEightBitRaw = TRUE;

	    } else if (!strncmp(cp1, "iso-8859-", 9) &&
		       !strncmp(LYchar_set_names[current_char_set],
				"Other ISO Latin", 15)) {
		/*
		 *  Hope it's a match, for now. - FM
		 */
		StrAllocCopy(me->node_anchor->charset, "iso-8859- ");
		me->node_anchor->charset[9] = cp1[9];
		HTPassEightBitRaw = TRUE;
		HTAlert(me->node_anchor->charset);

	    } else if (!strncmp(cp1, "koi8-r", 6) &&
		       !strncmp(LYchar_set_names[current_char_set],
				"KOI8-R character set", 20)) {
		StrAllocCopy(me->node_anchor->charset, "koi8-r");
		HTPassEightBitRaw = TRUE;

	    } else if (!strncmp(cp1, "euc-jp", 6) && HTCJK == JAPANESE) {
		StrAllocCopy(me->node_anchor->charset, "euc-jp");

	    } else if (!strncmp(cp1, "shift_jis", 9) && HTCJK == JAPANESE) {
		StrAllocCopy(me->node_anchor->charset, "shift_jis");

	    } else if (!strncmp(cp1, "iso-2022-jp", 11) &&
	    			HTCJK == JAPANESE) {
		StrAllocCopy(me->node_anchor->charset, "iso-2022-jp");

	    } else if (!strncmp(cp1, "iso-2022-jp-2", 13) &&
	    			HTCJK == JAPANESE) {
		StrAllocCopy(me->node_anchor->charset, "iso-2022-jp-2");

	    } else if (!strncmp(cp1, "euc-kr", 6) && HTCJK == KOREAN) {
		StrAllocCopy(me->node_anchor->charset, "euc-kr");

	    } else if (!strncmp(cp1, "iso-2022-kr", 11) && HTCJK == KOREAN) {
		StrAllocCopy(me->node_anchor->charset, "iso-2022-kr");

	    } else if ((!strncmp(cp1, "big5", 4) ||
			!strncmp(cp1, "cn-big5", 7)) &&
		       HTCJK == TAIPEI) {
		StrAllocCopy(me->node_anchor->charset, "big5");

	    } else if (!strncmp(cp1, "euc-cn", 6) && HTCJK == CHINESE) {
		StrAllocCopy(me->node_anchor->charset, "euc-cn");

	    } else if ((!strncmp(cp1, "gb2312", 6) ||
			!strncmp(cp1, "cn-gb", 5)) &&
		       HTCJK == CHINESE) {
		StrAllocCopy(me->node_anchor->charset, "gb2312");

	    } else if (!strncmp(cp1, "iso-2022-cn", 11) && HTCJK == CHINESE) {
		StrAllocCopy(me->node_anchor->charset, "iso-2022-cn");
	    }

	    if (TRACE && me->node_anchor->charset) {
		fprintf(stderr,
			"HTML: New charset: %s\n",
			me->node_anchor->charset);
	    }
	}
	/*
	 *  Set the kcode element based on the charset. - FM
	 */
	HText_setKcode(me->text, me->node_anchor->charset);

    /*
     *  Check for a Refresh directive. - FM
     */
    } else if (!strcasecomp((http_equiv ? http_equiv : ""), "Refresh")) {
	char *Seconds = NULL;

	/*
	 *  Look for the Seconds field. - FM
	 */
	cp = content;
	while (*cp && isspace((unsigned char)*cp))
	    cp++;
	if (*cp && isdigit(*cp)) {
	    cp1 = cp;
	    while (*cp1 && isdigit(*cp1))
		cp1++;
	    if (*cp1)
		*cp1++ = '\0';
	    StrAllocCopy(Seconds, cp);
	}
	if (Seconds) {
	    /*
	     *  We have the seconds field.
	     *  Now look for a URL field - FM
	     */
	    while (*cp1) {
		if (!strncasecomp(cp1, "URL", 3)) {
		    cp = (cp1 + 3);
		    while (*cp && (*cp == '=' || isspace((unsigned char)*cp)))
			cp++;
		    cp1 = cp;
		    while (*cp1 && !isspace((unsigned char)*cp1))
			cp1++;
		    *cp1 = '\0';
		    if (*cp)
			StrAllocCopy(href, cp);
		    break;
		}
		cp1++;
	    }
	    if (href) {
		/*
		 *  We found a URL field, so check it out. - FM
		 */
		if (!(url_type = LYLegitimizeHREF(me, (char**)&href,
						  TRUE, FALSE))) {
		    /*
		     *  The specs require a complete URL,
		     *  but this is a Netscapism, so don't
		     *  expect the author to know that. - FM
		     */
		    HTAlert(REFRESH_URL_NOT_ABSOLUTE);
		    /*
		     *  Use the document's address
		     *  as the base. - FM
		     */
		    if (*href != '\0') {
			temp = HTParse(href,
				       me->node_anchor->address, PARSE_ALL);
			StrAllocCopy(href, temp);
			FREE(temp);
		    } else {
			StrAllocCopy(href, me->node_anchor->address);
			HText_setNoCache(me->text);
		    }
		}
		/*
		 *  Check whether to fill in localhost. - FM
		 */
		LYFillLocalFileURL((char **)&href,
				   (me->inBASE ?
				 me->base_href : me->node_anchor->address));
		/*
		 *  Set the no_cache flag if the Refresh URL
		 *  is the same as the document's address. - FM
		 */
		if (!strcmp(href, me->node_anchor->address)) {
		    HText_setNoCache(me->text);
		} 
	    } else {
		/*
		 *  We didn't find a URL field, so use
		 *  the document's own address and set
		 *  the no_cache flag. - FM
		 */
		StrAllocCopy(href, me->node_anchor->address);
		HText_setNoCache(me->text);
	    }
	    /*
	     *  Check for an anchor in http or https URLs. - FM
	     */
	    if ((strncmp(href, "http", 4) == 0) &&
		(cp = strrchr(href, '#')) != NULL) {
		StrAllocCopy(id_string, cp);
		*cp = '\0';
	    }
	    if (me->inA) {
	        /*
		 *  Ugh!  The META tag, which is a HEAD element,
		 *  is in an Anchor, which is BODY element.  All
		 *  we can do is close the Anchor and cross our
		 *  fingers. - FM
		 */
		if (me->inBoldA == TRUE && me->inBoldH == FALSE)
		    HText_appendCharacter(me->text, LY_BOLD_END_CHAR);
		me->inBoldA = FALSE;
	        HText_endAnchor(me->text, me->CurrentANum);
		me->inA = FALSE;
		me->CurrentANum = 0;
	    }
	    me->CurrentA = HTAnchor_findChildAndLink(
				me->node_anchor,	/* Parent */
				id_string,		/* Tag */
				href,			/* Addresss */
				(void *)0);		/* Type */
	    if (id_string)
		*cp = '#';
	    FREE(id_string);
	    LYEnsureSingleSpace(me);
	    if (me->inUnderline == FALSE)
		HText_appendCharacter(me->text, LY_UNDERLINE_START_CHAR);
	    HTML_put_string(me, "REFRESH(");
	    HTML_put_string(me, Seconds);
	    HTML_put_string(me, " sec):");
	    FREE(Seconds);
	    if (me->inUnderline == FALSE)
		HText_appendCharacter(me->text, LY_UNDERLINE_END_CHAR);
	    HTML_put_character(me, ' ');
	    me->in_word = NO;
	    HText_beginAnchor(me->text, me->inUnderline, me->CurrentA);
	    if (me->inBoldH == FALSE)
		HText_appendCharacter(me->text, LY_BOLD_START_CHAR);
	    HTML_put_string(me, href);
	    FREE(href);
	    if (me->inBoldH == FALSE)
		HText_appendCharacter(me->text, LY_BOLD_END_CHAR);
	    HText_endAnchor(me->text, 0);
	    LYEnsureSingleSpace(me);
	}

    /*
     *  Check for a suggested filename via a Content-Disposition with
     *  a filename=name.suffix in it, if we don't already have it
     *  via a server header. - FM
     */
    } else if (!(me->node_anchor->SugFname && *me->node_anchor->SugFname) &&
    	       !strcasecomp((http_equiv ?
	       		     http_equiv : ""), "Content-Disposition")) {
	cp = content;
	while (*cp != '\0' && strncasecomp(cp, "filename", 8))
	    cp++;
	if (*cp != '\0') {
	    cp += 8;
	    while ((*cp != '\0') && (WHITE(*cp) || *cp == '='))
	        cp++;
	    while (*cp != '\0' && WHITE(*cp))
	        cp++;
	    if (*cp != '\0') {
		StrAllocCopy(me->node_anchor->SugFname, cp);
		if (*me->node_anchor->SugFname == '\"') {
		    if ((cp = strchr((me->node_anchor->SugFname + 1),
		    		     '\"')) != NULL) {
			*(cp + 1) = '\0';
			HTMIME_TrimDoubleQuotes(me->node_anchor->SugFname);
		    } else {
			FREE(me->node_anchor->SugFname);
		    }
		    if (me->node_anchor->SugFname != NULL &&
		        *me->node_anchor->SugFname == '\0') {
			FREE(me->node_anchor->SugFname);
		    }
		}
		if ((cp = me->node_anchor->SugFname) != NULL) {
		    while (*cp != '\0' && !WHITE(*cp))
			cp++;
		    *cp = '\0';
		    if (*me->node_anchor->SugFname == '\0')
		        FREE(me->node_anchor->SugFname);
		}
	    }
	}
    /*
     *  Check for a Set-Cookie directive. - AK
     */
    } else if (!strcasecomp((http_equiv ? http_equiv : ""), "Set-Cookie")) {
	/*
	 *  This will need to be updated when Set-Cookie/Set-Cookie2
	 *  handling is finalized.  For now, we'll still assume
	 *  "historical" cookies in META directives. - FM
	 */
	url_type = is_url(me->inBASE ?
		       me->base_href : me->node_anchor->address);
	if (url_type == HTTP_URL_TYPE || url_type == HTTPS_URL_TYPE) {
	    LYSetCookie(content,
	    		NULL,
			(me->inBASE ?
		      me->base_href : me->node_anchor->address));
	}
    }

    /*
     *  Free the copies. - FM
     */
free_META_copies:
    FREE(http_equiv);
    FREE(name);
    FREE(content);
}

#ifdef NOTUSED_FOTEMODS
/*  next two functions done in HTML.c instead in this code set -
    see there. - kw */
/*
**  This function handles P elements in HTML streams.
**  If start is TRUE it handles a start tag, and if
**  FALSE, an end tag.  We presently handle start
**  and end tags identically, but this can lead to
**  a different number of blank lines between the
**  current paragraph and subsequent text when a P
**  end tag is present or not in the markup. - FM
*/
PUBLIC void LYHandleP ARGS5(
	HTStructured *,		me,
	CONST BOOL*,	 	present,
	CONST char **,		value,
	char **,		include,
	BOOL,			start)
{
    if (TRUE) {
	/*
	 *  FIG content should be a true block, which like P inherits
	 *  the current style.  APPLET is like character elements or
	 *  an ALT attribute, unless it content contains a block element.
	 *  If we encounter a P in either's content, we set flags to treat
	 *  the content as a block.  - FM
	 */
	if (me->inFIG)
	    me->inFIGwithP = TRUE;

	if (me->inAPPLET)
	    me->inAPPLETwithP = TRUE;

	UPDATE_STYLE;
	if (me->List_Nesting_Level >= 0) {
	    /*
	     *  We're in a list.  Treat P as an instruction to
	     *  create one blank line, if not already present,
	     *  then fall through to handle attributes, with
	     *  the "second line" margins. - FM
	     */
	    if (me->inP) {
	        if (me->inFIG || me->inAPPLET ||
		    me->inCAPTION || me->inCREDIT ||
		    me->sp->style->spaceAfter > 0 ||
		    me->sp->style->spaceBefore > 0) {
	            LYEnsureDoubleSpace(me);
		} else {
	            LYEnsureSingleSpace(me);
		}
	    }
	} else if (me->sp[0].tag_number == HTML_ADDRESS) {
	    /*
	     *  We're in an ADDRESS. Treat P as an instruction 
	     *  to start a newline, if needed, then fall through
	     *  to handle attributes. - FM
	     */
	    if (HText_LastLineSize(me->text, FALSE)) {
		HText_setLastChar(me->text, ' ');  /* absorb white space */
	        HText_appendCharacter(me->text, '\r');
	    }
	} else if (!(me->inLABEL && !me->inP)) {
	    HText_appendParagraph(me->text);
	    me->inLABEL = FALSE;
	}
	me->in_word = NO;

	if (LYoverride_default_alignment(me)) {
	    me->sp->style->alignment = styles[me->sp[0].tag_number]->alignment;
	} else if (me->List_Nesting_Level >= 0 ||
		   ((me->Division_Level < 0) &&
		    (!strcmp(me->sp->style->name, "Normal") ||
		     !strcmp(me->sp->style->name, "Preformatted")))) {
	        me->sp->style->alignment = HT_LEFT;
	} else {
	    me->sp->style->alignment = me->current_default_alignment;
	}
	if (present && present[HTML_P_ALIGN] && value[HTML_P_ALIGN]) {
	    if (!strcasecomp(value[HTML_P_ALIGN], "center") &&
	        !(me->List_Nesting_Level >= 0 && !me->inP))
	        me->sp->style->alignment = HT_CENTER;
	    else if (!strcasecomp(value[HTML_P_ALIGN], "right") &&
	        !(me->List_Nesting_Level >= 0 && !me->inP))
	        me->sp->style->alignment = HT_RIGHT;
	    else if (!strcasecomp(value[HTML_P_ALIGN], "left") ||
	    	     !strcasecomp(value[HTML_P_ALIGN], "justify"))
	        me->sp->style->alignment = HT_LEFT;
	}

	LYCheckForID(me, present, value, (int)HTML_P_ID);

	/*
	 *  Mark that we are starting a new paragraph
	 *  and don't have any of it's text yet. - FM
	 *
	 */
	me->inP = FALSE;
    }

    return;
}

/*
**  This function handles SELECT elements in HTML streams.
**  If start is TRUE it handles a start tag, and if FALSE,
**  an end tag. - FM
*/
PUBLIC void LYHandleSELECT ARGS5(
	HTStructured *,		me,
	CONST BOOL*,	 	present,
	CONST char **,		value,
	char **,		include,
	BOOL,			start)
{
    int i;

    if (start == TRUE) {
	char *name = NULL;
	BOOLEAN multiple = NO;
	char *size = NULL;

        /*
	 *  Initialize the disable attribute.
	 */
	me->select_disabled = FALSE;

	/*
	 *  Make sure we're in a form.
	 */
	if (!me->inFORM) {
	    if (TRACE) {
		fprintf(stderr,
			"HTML: SELECT start tag not within FORM tag\n");
	    } else if (!me->inBadHTML) {
		_statusline(BAD_HTML_USE_TRACE);
		me->inBadHTML = TRUE;
		sleep(MessageSecs);
	    }

	    /*
	     *  Too likely to cause a crash, so we'll ignore it. - FM
	     */
	    return;
	}

	/*
	 *  Check for unclosed TEXTAREA.
	 */
	if (me->inTEXTAREA) {
	    if (TRACE) {
		fprintf(stderr, "HTML: Missing TEXTAREA end tag\n");
	    } else if (!me->inBadHTML) {
		_statusline(BAD_HTML_USE_TRACE);
		me->inBadHTML = TRUE;
		sleep(MessageSecs);
	    }
	}

	/*
	 *  Set to know we are in a select tag.
	 */
	me->inSELECT = TRUE;

	if (!me->text)
	    UPDATE_STYLE;
	if (present && present[HTML_SELECT_NAME] &&
	    value[HTML_SELECT_NAME] && *value[HTML_SELECT_NAME])  
	    StrAllocCopy(name, value[HTML_SELECT_NAME]);
	else
	    StrAllocCopy(name, "");
	if (present && present[HTML_SELECT_MULTIPLE])  
	    multiple=YES;
	if (present && present[HTML_SELECT_DISABLED])  
	    me->select_disabled = TRUE;
	if (present && present[HTML_SELECT_SIZE] &&
	    value[HTML_SELECT_SIZE] && *value[HTML_SELECT_SIZE]) {
#ifdef NOTDEFINED
	    StrAllocCopy(size, value[HTML_SELECT_SIZE]);
#else
	    /*
	     *  Let the size be determined by the number of OPTIONs. - FM
	     */
	    if (TRACE)
		fprintf(stderr,
			"HTML: Ignoring SIZE=\"%s\" for SELECT.\n",
			(char *)value[HTML_SELECT_SIZE]);
#endif /* NOTDEFINED */
	}

	if (me->inBoldH == TRUE &&
	    (multiple == NO || LYSelectPopups == FALSE)) {
	    HText_appendCharacter(me->text, LY_BOLD_END_CHAR);
	    me->inBoldH = FALSE;
	    me->needBoldH = TRUE;
	}
	if (me->inUnderline == TRUE &&
	    (multiple == NO || LYSelectPopups == FALSE)) {
	    HText_appendCharacter(me->text, LY_UNDERLINE_END_CHAR);
	    me->inUnderline = FALSE;
	}

	if ((multiple == NO && LYSelectPopups == TRUE) &&
	    me->sp[0].tag_number == HTML_PRE &&
	        HText_LastLineSize(me->text, FALSE) > (LYcols - 8)) {
		/*
		 *  Force a newline when we're using a popup in
		 *  a PRE block and are within 7 columns from the
		 *  right margin.  This will allow for the '['
		 *  popup designater and help avoid a wrap in the
		 *  underscore placeholder for the retracted popup
		 *  entry in the HText structure. - FM
		 */
		HTML_put_character(me, '\n');
		me->in_word = NO;
	    }

	LYCheckForID(me, present, value, (int)HTML_SELECT_ID);

	HText_beginSelect(name, multiple, size);
	FREE(name);
	FREE(size);

	me->first_option = TRUE;
    } else {
        /*
	 *  Handle end tag.
	 */
	char *ptr;
	if (!me->text)
	    UPDATE_STYLE;

	/*
	 *  Make sure we had a select start tag.
	 */
	if (!me->inSELECT) {
	    if (TRACE) {
		fprintf(stderr, "HTML: Unmatched SELECT end tag\n");
	    } else if (!me->inBadHTML) {
		_statusline(BAD_HTML_USE_TRACE);
		me->inBadHTML = TRUE;
		sleep(MessageSecs);
	    }
	    return;
	}

	/*
	 *  Set to know that we are no longer in a select tag.
	 */
	me->inSELECT = FALSE;

	/*
	 *  Clear the disable attribute.
	 */
	me->select_disabled = FALSE;

	/*
	 *  Finish the data off.
	 */
       	HTChunkTerminate(&me->option);
	/*
	 *  Finish the previous option.
	 */
	ptr = HText_setLastOptionValue(me->text,
				       me->option.data,
				       me->LastOptionValue,
				       LAST_ORDER,
				       me->LastOptionChecked);
	FREE(me->LastOptionValue);

	me->LastOptionChecked = FALSE;

	if (HTCurSelectGroupType == F_CHECKBOX_TYPE ||
	    LYSelectPopups == FALSE) {
	    /*
	     *  Start a newline after the last checkbox/button option.
	     */
	    LYEnsureSingleSpace(me);
	} else {
	    /*
	     *  Output popup box with the default option to screen,
	     *  but use non-breaking spaces for output.
	     */
	    if (ptr &&
		me->sp[0].tag_number == HTML_PRE && strlen(ptr) > 6) {
	        /*
		 *  The code inadequately handles OPTION fields in PRE tags.
		 *  We'll put up a minimum of 6 characters, and if any
		 *  more would exceed the wrap column, we'll ignore them.
		 */
		for (i = 0; i < 6; i++) {
		    if (*ptr == ' ')
	                HText_appendCharacter(me->text, HT_NON_BREAK_SPACE); 
		    else
	    	        HText_appendCharacter(me->text, *ptr);
		    ptr++;
		}
		HText_setIgnoreExcess(me->text, TRUE);
	    }
	    for (; ptr && *ptr != '\0'; ptr++) {
		if (*ptr == ' ')
	            HText_appendCharacter(me->text, HT_NON_BREAK_SPACE); 
		else
	    	    HText_appendCharacter(me->text, *ptr);
	    }
	    /*
	     *  Add end option character.
	     */
	    HText_appendCharacter(me->text, ']');
	    HText_setLastChar(me->text, ']');
	    me->in_word = YES;
	    HText_setIgnoreExcess(me->text, FALSE); 
	}
    	HTChunkClear(&me->option);

	if (me->Underline_Level > 0 && me->inUnderline == FALSE) {
	    HText_appendCharacter(me->text, LY_UNDERLINE_START_CHAR);
	    me->inUnderline = TRUE;
	}
	if (me->needBoldH == TRUE && me->inBoldH == FALSE) {
	    HText_appendCharacter(me->text, LY_BOLD_START_CHAR);
	    me->inBoldH = TRUE;
	    me->needBoldH = FALSE;
	}
    }
}
#endif /* NOTUSED_FOTEMODS */

/*
**  This function strips white characters and
**  generally fixes up attribute values that
**  were received from the SGML parser and
**  are to be treated as partial or absolute
**  URLs. - FM
*/
PUBLIC int LYLegitimizeHREF ARGS4(
	HTStructured *, 	me,
	char **,		href,
	BOOL,			force_slash,
	BOOL,			strip_dots)
{
    int url_type = 0;
    char *pound = NULL;
    char *fragment = NULL;

    if (!me || !href || *href == NULL || *(*href) == '\0')
        return(url_type);

    LYTrimHead(*href);
    if (!strncasecomp(*href, "lynxexec:", 9) ||
        !strncasecomp(*href, "lynxprog:", 9)) {
	/*
	 *  The original implementions of these schemes expected
	 *  white space without hex escaping, and did not check
	 *  for hex escaping, so we'll continue to support that,
	 *  until that code is redone in conformance with SGML
	 *  principles.  - FM
	 */
	HTUnEscapeSome(*href, " \r\n\t");
	convert_to_spaces(*href, TRUE);
    } else {
        /*
	 *  Collapse spaces in the actual URL, but just
	 *  protect against tabs or newlines in the
	 *  fragment, if present.  This seeks to cope
	 *  with atrocities inflicted on the Web by
	 *  authoring tools such as Frontpage. - FM
	 */
	if ((pound = strchr(*href, '#')) != NULL) {
	    StrAllocCopy(fragment, pound);
	    *pound = '\0';
	    convert_to_spaces(fragment, FALSE);
	}
        collapse_spaces(*href);
	if (fragment != NULL) {
	    StrAllocCat(*href, fragment);
	    FREE(fragment);
	}
    }
    if (*(*href) == '\0')
        return(url_type);
    LYUnEscapeToLatinOne(&(*href), TRUE);
    url_type = is_url(*href);
    if (!url_type && force_slash &&
	(!strcmp(*href, ".") || !strcmp(*href, "..")) &&
	 strncmp((me->inBASE ?
	       me->base_href : me->node_anchor->address),
		 "file:", 5)) {
        /*
	 *  The Fielding RFC/ID for resolving partial HREFs says
	 *  that a slash should be on the end of the preceding
	 *  symbolic element for "." and "..", but all tested
	 *  browsers only do that for an explicit "./" or "../",
	 *  so we'll respect the RFC/ID only if force_slash was
	 *  TRUE and it's not a file URL. - FM
	 */
	StrAllocCat(*href, "/");
    }
    if ((!url_type && LYStripDotDotURLs && strip_dots && *(*href) == '.') &&
	 !strncasecomp((me->inBASE ?
		     me->base_href : me->node_anchor->address),
		       "http", 4)) {
	/*
	 *  We will be resolving a partial reference versus an http
	 *  or https URL, and it has lead dots, which may be retained
	 *  when resolving via HTParse(), but the request would fail
	 *  if the first element of the resultant path is two dots,
	 *  because no http or https server accepts such paths, and
	 *  the current URL draft, likely to become an RFC, says that
	 *  it's optional for the UA to strip them as a form of error
	 *  recovery.  So we will, recursively, for http/https URLs,
	 *  like the "major market browsers" which made this problem
	 *  so common on the Web, but we'll also issue a message about
	 *  it, such that the bad partial reference might get corrected
	 *  by the document provider. - FM
	 */
	char *temp = NULL, *path = NULL, *str = "", *cp;

	if (((temp = HTParse(*href,
			     (me->inBASE ?
			   me->base_href : me->node_anchor->address),
			     PARSE_ALL)) != NULL && temp[0] != '\0') &&
	    (path = HTParse(temp, "",
			    PARSE_PATH+PARSE_PUNCTUATION)) != NULL &&
	    !strncmp(path, "/..", 3)) {
	    cp = (path + 3);
	    if (*cp == '/' || *cp == '\0') {
		if ((me->inBASE ?
	       me->base_href[4] : me->node_anchor->address[4]) == 's') {
		    str = "s";
		}
		if (TRACE) {
		    fprintf(stderr,
			 "LYLegitimizeHREF: Bad value '%s' for http%s URL.\n",
			   *href, str);
		    fprintf(stderr,
			 "                  Stripping lead dots.\n");
		} else if (!me->inBadHREF) {
		    _statusline(BAD_PARTIAL_REFERENCE);
		    me->inBadHREF = TRUE;
		    sleep(AlertSecs);
		}
	    }
	    if (*cp == '\0') {
		StrAllocCopy(*href, "/");
	    } else if (*cp == '/') {
		while (!strncmp(cp, "/..", 3)) {
		    if (*(cp + 3) == '/') {
			cp += 3;
			continue;
		    } else if (*(cp + 3) == '\0') {
		        *(cp + 1) = '\0';
		        *(cp + 2) = '\0';
		    }
		    break;
		}
		StrAllocCopy(*href, cp);
	    }
	}
	FREE(temp);
	FREE(path);
    }
    return(url_type); 
}

/*
**  This function checks for a Content-Base header,
**  and if not present, a Content-Location header
**  which is an absolute URL, and sets the BASE
**  accordingly.  If set, it will be replaced by
**  any BASE tag in the HTML stream, itself. - FM
*/
PUBLIC void LYCheckForContentBase ARGS1(
	HTStructured *,		me)
{
    char *cp = NULL;
    BOOL present[HTML_BASE_ATTRIBUTES];
    char *value[HTML_BASE_ATTRIBUTES];
    int i;

    if (!(me && me->node_anchor))
        return;

    if (me->node_anchor->content_base != NULL) {
        /*
	 *  We have a Content-Base value.  Use it
	 *  if it's non-zero length. - FM
	 */
        if (*me->node_anchor->content_base == '\0')
	    return;
	StrAllocCopy(cp, me->node_anchor->content_base);
	collapse_spaces(cp);
    } else if (me->node_anchor->content_location != NULL) {
        /*
	 *  We didn't have a Content-Base value, but do
	 *  have a Content-Location value.  Use it if
	 *  it's an absolute URL. - FM
	 */
        if (*me->node_anchor->content_location == '\0')
	    return;
	StrAllocCopy(cp, me->node_anchor->content_location);
	collapse_spaces(cp);
	if (!is_url(cp)) {
	    FREE(cp);
	    return;
	}
    } else {
        /*
	 *  We had neither a Content-Base nor
	 *  Content-Location value. - FM
	 */
        return;
    }

    /*
     *  If we collapsed to a zero-length value,
     *  ignore it. - FM
     */
    if (*cp == '\0') {
        FREE(cp);
	return;
    }

    /*
     *  Pass the value to HTML_start_element as
     *  the HREF of a BASE tag. - FM
     */
    for (i = 0; i < HTML_BASE_ATTRIBUTES; i++)
	 present[i] = NO;
    present[HTML_BASE_HREF] = YES;
    value[HTML_BASE_HREF] = cp;
    (*me->isa->start_element)(me, HTML_BASE, present, value, 0);
    FREE(cp);
}

/*
**  This function creates NAMEd Anchors if a non-zero-length NAME
**  or ID attribute was present in the tag. - FM
*/
PUBLIC void LYCheckForID ARGS4(
	HTStructured *,		me,
	CONST BOOL *,		present,
	CONST char **,		value,
	int,			attribute)
{
    HTChildAnchor *ID_A = NULL;
    char *temp = NULL;

    if (!(me && me->text))
        return;

    if (present && present[attribute]
	&& value[attribute] && *value[attribute]) {
	/*
	 *  Translate any named or numeric character references. - FM
	 */
	StrAllocCopy(temp, value[attribute]);
	LYUnEscapeToLatinOne(&temp, TRUE);

	/*
	 *  Create the link if we still have a non-zero-length string. - FM
	 */
	if ((temp[0] != '\0') &&
	    (ID_A = HTAnchor_findChildAndLink(
				me->node_anchor,	/* Parent */
				temp,			/* Tag */
				NULL,			/* Addresss */
				(void *)0))) {		/* Type */
	    HText_beginAnchor(me->text, me->inUnderline, ID_A);
	    HText_endAnchor(me->text, 0);
	}
	FREE(temp);
    }
}

/*
**  This function creates a NAMEd Anchor for the ID string
**  passed to it directly as an argument.  It assumes the
**  does not need checking for character references. - FM
*/
PUBLIC void LYHandleID ARGS2(
	HTStructured *,		me,
	char *,			id)
{
    HTChildAnchor *ID_A = NULL;

    if (!(me && me->text) ||
        !(id && *id))
        return;

    /*
     *  Create the link if we still have a non-zero-length string. - FM
     */
    if ((ID_A = HTAnchor_findChildAndLink(
				me->node_anchor,	/* Parent */
				id,			/* Tag */
				NULL,			/* Addresss */
				(void *)0)) != NULL) {	/* Type */
	HText_beginAnchor(me->text, me->inUnderline, ID_A);
	HText_endAnchor(me->text, 0);
    }
}

/*
**  This function checks whether we want to overrride
**  the current default alignment for parargraphs and
**  instead use that specified in the element's style
**  sheet. - FM
*/
PUBLIC BOOLEAN LYoverride_default_alignment ARGS1(
	HTStructured *, me)
{
    if (!me)
        return NO;

    switch(me->sp[0].tag_number) {
	case HTML_BLOCKQUOTE:
	case HTML_BQ:
	case HTML_NOTE:
	case HTML_FN:
        case HTML_ADDRESS:
	    me->sp->style->alignment = HT_LEFT;
	    return YES;
	    break;

	default:
	    break;
    }
    return NO;
}

/*
**  This function inserts newlines if needed to create double spacing,
**  and sets the left margin for subsequent text to the second line
**  indentation of the current style. - FM
*/
PUBLIC void LYEnsureDoubleSpace ARGS1(
	HTStructured *, me)
{
    if (!me || !me->text)
        return;

    if (HText_LastLineSize(me->text, FALSE)) {
	HText_setLastChar(me->text, ' ');  /* absorb white space */
	HText_appendCharacter(me->text, '\r');
	HText_appendCharacter(me->text, '\r');
    } else if (HText_PreviousLineSize(me->text, FALSE)) {
	HText_setLastChar(me->text, ' ');  /* absorb white space */
	HText_appendCharacter(me->text, '\r');
    } else if (me->List_Nesting_Level >= 0) {
	HText_NegateLineOne(me->text);
    }
    me->in_word = NO;
    return;
}

/*
**  This function inserts a newline if needed to create single spacing,
**  and sets the left margin for subsequent text to the second line
**  indentation of the current style. - FM
*/
PUBLIC void LYEnsureSingleSpace ARGS1(
	HTStructured *, me)
{
    if (!me || !me->text)
        return;

    if (HText_LastLineSize(me->text, FALSE)) {
	HText_setLastChar(me->text, ' ');  /* absorb white space */
	HText_appendCharacter(me->text, '\r');
    } else if (me->List_Nesting_Level >= 0) {
	HText_NegateLineOne(me->text);
    }
    me->in_word = NO;
    return;
}

/*
**  This function resets paragraph alignments for block
**  elements which do not have a defined style sheet. - FM
*/
PUBLIC void LYResetParagraphAlignment ARGS1(
	HTStructured *, me)
{
    if (!me)
        return;

    if (me->List_Nesting_Level >= 0 ||
	((me->Division_Level < 0) &&
	 (!strcmp(me->sp->style->name, "Normal") ||
	  !strcmp(me->sp->style->name, "Preformatted")))) {
	me->sp->style->alignment = HT_LEFT;
    } else {
	me->sp->style->alignment = me->current_default_alignment;
    }
    return;
}

PUBLIC BOOLEAN LYCheckForCSI ARGS2(
	HTParentAnchor *, 	anchor,
	char **,		url)
{
    if (!(anchor && anchor->address))
        return FALSE;

    if (strncasecomp(anchor->address, "file:", 5))
        return FALSE;

    if (!LYisLocalHost(anchor->address))
        return FALSE;
     
    StrAllocCopy(*url, anchor->address);
    return TRUE;
}
