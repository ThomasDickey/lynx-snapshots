/* character level styles for Lynx
 * (c) 1996 Rob Partington -- donated to the Lyncei (if they want it :-)
 * @Id: LYStyle.c 1.40 Thu, 21 Dec 2000 18:44:11 -0800 dickey @
 */
#include <HTUtils.h>
#include <HTML.h>
#include <LYGlobalDefs.h>

#include <LYStructs.h>
#include <LYReadCFG.h>
#include <LYCurses.h>
#include <LYCharUtils.h>
#include <LYUtils.h>		/* defines TABLESIZE */
#include <AttrList.h>
#include <SGML.h>
#include <HTMLDTD.h>

/* Hash table definitions */
#include <LYHash.h>
#include <LYStyle.h>

#include <LYexit.h>
#include <LYLeaks.h>
#include <LYStrings.h>

#ifdef USE_COLOR_STYLE

PRIVATE void style_initialiseHashTable NOPARAMS;

/* stack of attributes during page rendering */
PUBLIC int last_styles[128];
PUBLIC int last_colorattr_ptr = 0;

PUBLIC bucket hashStyles[CSHASHSIZE];
PUBLIC bucket special_bucket =
{
    "<special>", /* in order something to be in trace. */
    0, 0, 0, 0, NULL
};
PUBLIC bucket nostyle_bucket =
{
    "<NOSTYLE>", /* in order something to be in trace. */
    0, 0, 0, 0, NULL
};

PUBLIC int cached_tag_styles[HTML_ELEMENTS];
PUBLIC int current_tag_style;
PUBLIC BOOL force_current_tag_style = FALSE;
PUBLIC char* forced_classname;
PUBLIC BOOL force_classname;

/* definitions for the mono attributes we can use */
static int ncursesMono[7] = {
    A_NORMAL, A_BOLD, A_REVERSE, A_UNDERLINE, A_STANDOUT, A_BLINK, A_DIM
};

/*
 * If these strings don't match the meanings of the above attributes,
 * you'll confuse the hell out of people, so make them the same. - RP
 */
static char *Mono_Strings[7] =
{
    "normal", "bold", "reverse", "underline", "standout", "blink", "dim"
};

/* Remember the hash codes for common elements */
PUBLIC int	s_alink  = NOSTYLE, s_a     = NOSTYLE, s_status = NOSTYLE,
		s_normal = NOSTYLE, s_alert = NOSTYLE, s_title  = NOSTYLE,
#ifdef USE_SCROLLBAR
		s_sb_bar = NOSTYLE, s_sb_bg = NOSTYLE,
		s_sb_aa = NOSTYLE, s_sb_naa = NOSTYLE,
#endif
		s_whereis = NOSTYLE, s_aedit = NOSTYLE,
		s_aedit_pad = NOSTYLE, s_aedit_arr = NOSTYLE, 
		s_prompt_edit = NOSTYLE, s_prompt_edit_pad = NOSTYLE,
		s_prompt_edit_arr = NOSTYLE;

/* start somewhere safe */
#define MAX_COLOR 16
PRIVATE int colorPairs = 0;
PRIVATE unsigned char our_pairs[2][MAX_COLOR][MAX_COLOR];

/* icky parsing of the style options */
PRIVATE void parse_attributes ARGS5(char*,mono,char*,fg,char*,bg,int,style,char*,element)
{
    int i;
    int mA = 0;
    short fA = default_fg;
    short bA = default_bg;
    int cA = A_NORMAL;
    int newstyle = hash_code(element);

    CTRACE((tfp, "CSS(PA):style d=%d / h=%d, e=%s\n", style, newstyle,element));

    for (i = 0; i < (int)TABLESIZE(Mono_Strings); i++)
    {
	if (!strcasecomp(Mono_Strings[i], mono))
	{
	    mA = ncursesMono[i];
	}
    }
    if (!mA) {
	/*
	 *  Not found directly yet, see whether we have a combination
	 *  of several mono attributes separated by '+' - kw
	 */
	char *cp0 = mono;
	char csep = '+';
	char *cp = strchr(mono, csep);
	while (cp) {
	    *cp = '\0';
	    for (i = 0; i < (int)TABLESIZE(Mono_Strings); i++)
	    {
		if (!strcasecomp(Mono_Strings[i], cp0))
		{
		    mA |= ncursesMono[i];
		}
	    }
	    if (!csep)
		break;
	    *cp = csep;
	    cp0 = cp + 1;
	    cp = strchr(cp0, csep);
	    if (!cp) {
		cp = cp0 + strlen(cp0);
		csep = '\0';
	    }
	}
    }
    CTRACE((tfp, "CSS(CP):%d\n", colorPairs));

    fA = check_color(fg, default_fg);
    bA = check_color(bg, default_bg);

    if (style == -1) {			/* default */
	CTRACE((tfp, "CSS(DEF):default_fg=%d, default_bg=%d\n", fA, bA));
	default_fg = fA;
	default_bg = bA;
	default_color_reset = TRUE;
	return;
    }
    if (fA == NO_COLOR) {
	bA = NO_COLOR;
    } else if (COLORS) {
	if (fA >= COLORS || bA >= COLORS)
	    cA = A_BOLD;
	if (fA >= COLORS)
	    fA %= COLORS;
	if (bA >= COLORS)
	    bA %= COLORS;
    } else {
	cA = A_BOLD;
	fA = NO_COLOR;
	bA = NO_COLOR;
    }

    /*
     * If we have colour, and space to create a new colour attribute,
     * and we have a valid colour description, then add this style
     */
    if (lynx_has_color && colorPairs < COLOR_PAIRS-1 && fA != NO_COLOR)
    {
	int curPair;

	if (fA < MAX_COLOR
	 && bA < MAX_COLOR
	 && our_pairs[cA == A_BOLD][fA][bA])
	    curPair = our_pairs[cA == A_BOLD][fA][bA] - 1;
	else {
	    curPair = ++colorPairs;
	    init_pair(curPair, fA, bA);
	    if (fA < MAX_COLOR
	     && bA < MAX_COLOR
	     && curPair < 255)
		our_pairs[cA == A_BOLD][fA][bA] = curPair + 1;
	}
	CTRACE((tfp, "CSS(CURPAIR):%d\n", colorPairs));
	if (style < DSTYLE_ELEMENTS)
	    setStyle(style, COLOR_PAIR(curPair)|cA, cA, mA);
	setHashStyle(newstyle, COLOR_PAIR(curPair)|cA, cA, mA, element);
    }
    else
    {
	if (lynx_has_color && fA != NO_COLOR) {
	    CTRACE((tfp, "CSS(NC): maximum of %d colorpairs exhausted\n", COLOR_PAIRS - 1));
	}
	/* only mono is set */
	if (style < DSTYLE_ELEMENTS)
	    setStyle(style, -1, -1, mA);
	setHashStyle(newstyle, -1, -1, mA, element);
    }
}

/* parse a style option of the format
 * STYLE:<OBJECT>:FG:BG
 */
PRIVATE void parse_style ARGS1(char*,buffer)
{
    char *tmp = strchr(buffer, ':');
    char *element, *mono, *fg, *bg;

    if(!tmp)
    {
	fprintf (stderr, gettext("\
Syntax Error parsing style in lss file:\n\
[%s]\n\
The line must be of the form:\n\
OBJECT:MONO:COLOR (ie em:bold:brightblue:white)\n\
where OBJECT is one of EM,STRONG,B,I,U,BLINK etc.\n\n"), buffer);
	if (!dump_output_immediately) {
	    exit_immediately(EXIT_FAILURE);
	}
	exit(1);
    }
    strtolower(buffer);
    *tmp = '\0';
    element = buffer;

    mono = tmp + 1;
    tmp = strchr(mono, ':');

    if (!tmp)
    {
	fg = "nocolor";
	bg = "nocolor";
    }
    else
    {
	*tmp = '\0';
	fg = tmp+1;
	tmp = strchr(fg, ':');
	if (!tmp)
	    bg = "default";
	else
	{
	    *tmp = '\0';
	    bg = tmp + 1;
	}
    }

    CTRACE((tfp, "CSSPARSE:%s => %d %s\n",
		element, hash_code(element),
		(hashStyles[hash_code(element)].name ? "used" : "")));

    strtolower(element);

    /*
    * We use some pseudo-elements, so catch these first
    */
    if (!strncasecomp(element, "default", 7)) /* default fg/bg */
    {
	parse_attributes(mono,fg,bg,-1,"default");
    }
    else if (!strncasecomp(element, "alink", 5)) /* active link */
    {
	parse_attributes(mono,fg,bg,DSTYLE_ALINK,"alink");
    }
    else if (!strcasecomp(element, "a")) /* normal link */
    {
	parse_attributes(mono,fg,bg, DSTYLE_LINK,"a");
	parse_attributes(mono,fg,bg, HTML_A,"a");
    }
    else if (!strncasecomp(element, "status", 4)) /* status bar */
    {
	parse_attributes(mono,fg,bg, DSTYLE_STATUS,"status");
    }
    else if (!strncasecomp(element, "label", 6)) /* [INLINE]'s */
    {
	parse_attributes(mono,fg,bg,DSTYLE_OPTION,"label");
    }
    else if (!strncasecomp(element, "value", 5)) /* [INLINE]'s */
    {
	parse_attributes(mono,fg,bg,DSTYLE_VALUE,"value");
    }
    else if (!strncasecomp(element, "high", 4)) /* [INLINE]'s */
    {
	parse_attributes(mono,fg,bg,DSTYLE_HIGH,"high");
    }
    else if (!strcasecomp(element, "normal")) /* added - kw */
    {
	parse_attributes(mono,fg,bg,DSTYLE_NORMAL,"html");
	s_normal  = hash_code("html"); /* rather bizarre... - kw */
    }
    /* this may vanish */
    else if (!strncasecomp(element, "candy", 5)) /* [INLINE]'s */
    {
	parse_attributes(mono,fg,bg,DSTYLE_CANDY,"candy");
    }
    /* added for whereis search target - kw */
    else if (!strncasecomp(element, "whereis", 7))
    {
	parse_attributes(mono,fg,bg,DSTYLE_WHEREIS,"whereis");
	s_whereis  = hash_code("whereis");
    }
    else if (!strncasecomp(element, "edit.active.pad", 15))
    {
	parse_attributes(mono,fg,bg,DSTYLE_ELEMENTS,"edit.active.pad");
	s_aedit_pad = hash_code("edit.active.pad");
    }
    else if (!strncasecomp(element, "edit.active.arrow", 17))
    {
	parse_attributes(mono,fg,bg,DSTYLE_ELEMENTS,"edit.active.arrow");
	s_aedit_arr  = hash_code("edit.active.arrow");
    }
    else if (!strncasecomp(element, "edit.active", 11))
    {
	parse_attributes(mono,fg,bg,DSTYLE_ELEMENTS,"edit.active");
	s_aedit  = hash_code("edit.active");
    }
    else if (!strncasecomp(element, "edit.prompt.pad", 15))
    {
	parse_attributes(mono,fg,bg,DSTYLE_ELEMENTS,"edit.prompt.pad");
	s_prompt_edit_pad = hash_code("edit.prompt.pad");
    }
    else if (!strncasecomp(element, "edit.prompt.arrow", 17))
    {
	parse_attributes(mono,fg,bg,DSTYLE_ELEMENTS,"edit.prompt.arrow");
	s_prompt_edit_arr  = hash_code("edit.prompt.arrow");
    }
    else if (!strncasecomp(element, "edit.prompt", 11))
    {
	parse_attributes(mono,fg,bg,DSTYLE_ELEMENTS,"edit.prompt");
	s_prompt_edit  = hash_code("edit.prompt");
    }
    /* Ok, it must be a HTML element, so look through the list until we
    * find it
    */
    else
    {
#if !defined(USE_HASH)
	int i;
	for (i = 0; i <HTML_ELEMENTS; i++)
	{
	    if (!strcasecomp (HTML_dtd.tags[i].name, element))
	    {
		CTRACE((tfp, "PARSECSS:applying style <%s,%s,%s> for HTML_%s\n",mono,fg,bg,HTML_dtd.tags[i].name));
			parse_attributes(mono,fg,bg,i+STARTAT,element);
		break;
	    }
	}
#else
	int element_number = -1;
	HTTag * t = SGMLFindTag(&HTML_dtd, element);
	if (t && t->name) {
	    element_number = t - HTML_dtd.tags;
	}
	if (element_number >= HTML_A &&
	    element_number < HTML_ELEMENTS)
	    parse_attributes(mono,fg,bg, element_number+STARTAT,element);
	else
	    parse_attributes(mono,fg,bg, DSTYLE_ELEMENTS,element);
#endif
    }
}

#ifdef LY_FIND_LEAKS
PRIVATE void free_colorstylestuff NOARGS
{
    style_initialiseHashTable();
    style_deleteStyleList();
}
#endif

/*
 * initialise the default style sheet
 * This should be able to be read from a file in CSS format :-)
 */
PRIVATE void initialise_default_stylesheet NOARGS
{
    static CONST char *table[] = {
	"a:bold:green",
	"alert:bold:yellow:red",
	"alink:reverse:yellow:black",
	"label:normal:magenta",
	"status:reverse:yellow:blue",
	"title:normal:magenta",
	"whereis:reverse+underline:magenta:cyan"
    };
    unsigned n;
    char temp[80];
    for (n = 0; n < TABLESIZE(table); n++) {
	parse_style(strcpy(temp, table[n]));
    }
}

/* Set all the buckets in the hash table to be empty */
PRIVATE void style_initialiseHashTable NOARGS
{
    int i;
    static int firsttime = 1;

    for (i = 0; i <CSHASHSIZE; i++)
    {
	if (firsttime)
	    hashStyles[i].name = NULL;
	else
	    FREE(hashStyles[i].name);
	hashStyles[i].color = 0;
	hashStyles[i].cattr = 0;
	hashStyles[i].mono  = 0;
    }
    if (firsttime) {
	firsttime = 0;
#ifdef LY_FIND_LEAKS
	atexit(free_colorstylestuff);
#endif
    }
    s_alink  = hash_code("alink");
    s_a      = hash_code("a");
    s_status = hash_code("status");
    s_alert  = hash_code("alert");
    s_title  = hash_code("title");
#ifdef USE_SCROLLBAR
    s_sb_bar = hash_code("scroll.bar");
    s_sb_bg  = hash_code("scroll.back");
    s_sb_aa  = hash_code("scroll.arrow");
    s_sb_naa = hash_code("scroll.noarrow");
#endif
}

/* because curses isn't started when we parse the config file, we
 * need to remember the STYLE: lines we encounter and parse them
 * after curses has started
 */
HTList *lss_styles = NULL;

PUBLIC void parse_userstyles NOARGS
{
    char *name;
    HTList *cur = lss_styles;

    colorPairs = 0;
    style_initialiseHashTable();

    /* set our styles to be the same as vanilla-curses-lynx */
    if (HTList_isEmpty(cur)) {
	initialise_default_stylesheet();
    } else {
	while ((name = HTList_nextObject(cur)) != NULL) {
	    CTRACE((tfp, "LSS:%s\n", name ? name : "!?! empty !?!"));
	    if (name != NULL)
		parse_style(name);
	}
    }
    if (s_prompt_edit == NOSTYLE)
	s_prompt_edit = s_normal;
    if (s_prompt_edit_arr == NOSTYLE)
	s_prompt_edit_arr = s_prompt_edit;
    if (s_prompt_edit_pad == NOSTYLE)
	s_prompt_edit_pad = s_prompt_edit;
    if (s_aedit == NOSTYLE)
	s_aedit = s_alink;
    if (s_aedit_arr == NOSTYLE)
	s_aedit_arr = s_aedit;
    if (s_aedit_pad == NOSTYLE)
	s_aedit_pad = s_aedit;
}


/* Add a STYLE: option line to our list.  Process "default:" early
   for it to have the same semantic as other lines: works at any place
   of the style file, the first line overrides the later ones. */
PRIVATE void HStyle_addStyle ARGS1(char*,buffer)
{
    char *name = NULL;
    StrAllocCopy(name, buffer);
    if (lss_styles == NULL)
	lss_styles = HTList_new();
    strtolower(name);
    if (!strncasecomp(name, "default:", 8)) /* default fg/bg */
    {
	CTRACE( (tfp, "READCSS.default%s:%s\n",
		 (default_color_reset ? ".ignore" : ""),
		 name ? name : "!?! empty !?!"));
	if (!default_color_reset)
	    parse_style(name);
	return;				/* do not need to process it again */
    }
    CTRACE((tfp, "READCSS:%s\n", name ? name : "!?! empty !?!"));
    HTList_addObject (lss_styles, name);
}

PUBLIC void style_deleteStyleList NOARGS
{
    char *name;
    while ((name = HTList_removeLastObject(lss_styles)) != NULL)
	FREE(name);
    HTList_delete (lss_styles);
    lss_styles = NULL;
}

PRIVATE int style_readFromFileREC ARGS2(char*, file, int, toplevel)
{
    FILE *fh;
    char *buffer = NULL;
    int len;

    CTRACE((tfp, "CSS:Reading styles from file: %s\n", file ? file : "?!? empty ?!?"));
    if (file == NULL || *file == '\0')
	return -1;
    fh = fopen(file, TXT_R);
    if (!fh)
    {
	/* this should probably be an alert or something */
	CTRACE((tfp, "CSS:Can't open style file '%s', using defaults\n", file));
	return -1;
    }

    if (toplevel) {
	style_initialiseHashTable();
	style_deleteStyleList();
    }

    while (LYSafeGets(&buffer, fh) != NULL) {
	LYTrimTrailing(buffer);
	LYTrimTail(buffer);
	LYTrimHead(buffer);
	if (!strncasecomp(buffer,"include:",8))
	    style_readFromFileREC(buffer+8, 0);
	else if (buffer[0] != '#' && (len = strlen(buffer)) > 0)
	    HStyle_addStyle(buffer);
    }

    LYCloseInput (fh);
    if (toplevel && LYCursesON)
	parse_userstyles();
    return 0;
}

PUBLIC int style_readFromFile ARGS1(char*, file)
{
    return style_readFromFileREC(file, 1);
}

/* Used in HTStructured methods: - kw */

PUBLIC void TrimColorClass ARGS3(
    CONST char *,	tagname,
    char *,		styleclassname,
    int *,		phcode)
{
    char *end, *start=NULL, *lookfrom;
    char tmp[64];

    sprintf(tmp, ";%.*s", (int) sizeof(tmp) - 3, tagname);
    strtolower(tmp);

    if ((lookfrom = styleclassname) != 0) {
	do {
	    end = start;
	    start = strstr(lookfrom, tmp);
	    if (start)
		lookfrom = start + 1;
	}
	while (start);
	/* trim the last matching element off the end
	** - should match classes here as well (rp)
	*/
	if (end)
	    *end='\0';
    }
    *phcode = hash_code(lookfrom && *lookfrom ? lookfrom : &tmp[1]);
}

/* This function is designed as faster analog to TrimColorClass.
   It assumes that tag_name is present in stylename! -HV
*/
PUBLIC void FastTrimColorClass ARGS5 (
	    CONST char*,	 tag_name,
	    int,		 name_len,
	    char*,		 stylename,
	    char**,		 pstylename_end,/*will be modified*/
	    int*,		 phcode)	/*will be modified*/
{
    char* tag_start = *pstylename_end;
    BOOLEAN found = FALSE;

    CTRACE((tfp, "STYLE.fast-trim: [%s] from [%s]: ", tag_name, stylename));
    while (tag_start >= stylename)
    {
	for (; (tag_start >= stylename) && (*tag_start != ';') ; --tag_start)
	    ;
	if ( !strncasecomp(tag_start+1, tag_name, name_len) ) {
	    found = TRUE;
	    break;
	}
	--tag_start;
    }
    if (found) {
	*tag_start = '\0';
	*pstylename_end = tag_start;
    }
    CTRACE((tfp, found ? "success.\n" : "failed.\n"));
    *phcode = hash_code(tag_start+1);
}

 /* This is called each time lss styles are read. It will fill
    each elt of 'cached_tag_styles' -HV
 */
PUBLIC void cache_tag_styles NOARGS
{
    char buf[200];
    int i;

    for (i = 0; i < HTML_ELEMENTS; ++i)
    {
	strcpy(buf, HTML_dtd.tags[i].name);
	LYLowerCase(buf);
	cached_tag_styles[i] =hash_code(buf);
    }
}

#endif /* USE_COLOR_STYLE */
