#include <HTUtils.h>
#include <HTAlert.h>

#ifdef __MINGW32__
#ifdef UNIX
#undef UNIX
#endif /* UNIX */
#endif /* __MINGW32__ */

#include <LYCurses.h>
#include <LYStyle.h>
#include <LYUtils.h>
#include <LYGlobalDefs.h>
#include <LYSignal.h>
#include <LYClean.h>
#include <LYReadCFG.h>
#include <LYStrings.h>
#include <LYCharSets.h>
#include <UCAux.h>
#include <HTFont.h>

#include <LYexit.h>
#include <LYLeaks.h>

#ifdef VMS
#include <LYMainLoop.h>
#endif

#if defined(VMS) && defined(__GNUC__)
#include <gnu_hacks.h>
#undef LINES
#undef COLS
#define LINES lines
#define COLS cols
extern int _NOSHARE(LINES);
extern int _NOSHARE(COLS);
#endif /* VMS && __GNUC__ */

#ifdef USE_COLOR_STYLE
#include <AttrList.h>
#include <LYHash.h>
#endif

#ifdef NEED_WCHAR_H
#include <wchar.h>
#endif

#if defined(COLOR_CURSES)
int lynx_has_color = FALSE;
#endif

#ifdef HAVE_XCURSES
char *XCursesProgramName = "Lynx";
#endif

#if defined(USE_COLOR_STYLE) && !defined(USE_COLOR_TABLE)
#define COLOR_BKGD ((s_normal != NOSTYLE) ? hashStyles[s_normal].color : A_NORMAL)
#else
#define COLOR_BKGD ((COLOR_PAIRS >= 9) ? (chtype) get_color_pair(9) : A_NORMAL)
#endif

#ifdef USE_CURSES_PADS
WINDOW *LYwin = 0;
int LYshiftWin = 0;
int LYwideLines = FALSE;
int LYtableCols = 0;		/* in 1/12 of screen width */
BOOL LYuseCursesPads = TRUE;	/* use pads for left/right shifting */
#endif

/*
 * These are routines to start and stop curses and to cleanup the screen at the
 * end.
 */

static int dumbterm(char *terminal);
BOOLEAN LYCursesON = FALSE;

#if defined(USE_BLINK) && defined(__EMX__)
static void make_blink_boldbg(void);
#endif

#if defined(USE_COLOR_TABLE) || defined(USE_SLANG)
int Current_Attr;
static int Masked_Attr;
#endif

#ifdef USE_SLANG
unsigned Lynx_Color_Flags = 0;
BOOLEAN FullRefresh = FALSE;
int curscr = 0;

#ifdef SLANG_MBCS_HACK
/*
 * Will be set by size_change.  - KW
 */
int PHYSICAL_SLtt_Screen_Cols = 10;
#endif /* SLANG_MBCS_HACK */

void LY_SLrefresh(void)
{
    if (FullRefresh) {
	SLsmg_suspend_smg();
	SLsmg_resume_smg();
	FullRefresh = FALSE;
    } else {
	SLsmg_refresh();
    }

    return;
}

/* the following renamed from LY_SLclear since it is more like erase()
   described in curses man pages than like clear(); but for USE_SLANG
   clear() is still a macro calling this, and will do the same thing as
   erase(). - kw */
void LY_SLerase(void)
{
    SLsmg_gotorc(0, 0);
    SLsmg_erase_eos();
}

#ifdef VMS
void VTHome(void)
{
    printf("\033[;H");

    return;
}
#endif /* VMS */

void LYaddAttr(int a)
{
    Current_Attr |= a;
    SLsmg_set_color(Current_Attr & ~Masked_Attr);
}

void LYsubAttr(int a)
{
    Current_Attr &= ~a;
    SLsmg_set_color(Current_Attr & ~Masked_Attr);
}

static void lynx_setup_attrs(void)
{
    static int monoattr[] =
    {
	0,
	SLTT_BOLD_MASK,
	SLTT_REV_MASK,
	SLTT_REV_MASK | SLTT_BOLD_MASK,
	SLTT_ULINE_MASK,
	SLTT_ULINE_MASK | SLTT_BOLD_MASK,
	SLTT_ULINE_MASK | SLTT_REV_MASK,
	SLTT_ULINE_MASK | SLTT_BOLD_MASK | SLTT_REV_MASK
    };
    int n;

    for (n = 1; n <= 7; n++)
	SLtt_set_mono(n, NULL, (monoattr[n] & ~Masked_Attr));
}

void lynx_setup_colors(void)
{
    CTRACE((tfp, "lynx_setup_colors\n"));
    SLtt_set_color(0, NULL, DEFAULT_FG, DEFAULT_BG);
    SLtt_set_color(1, NULL, "blue", DEFAULT_BG);	/* bold */
    SLtt_set_color(2, NULL, "yellow", "blue");	/* reverse */
    SLtt_set_color(4, NULL, "magenta", DEFAULT_BG);	/* underline */
    /*
     * The other objects are '|'ed together to get rest.
     */
    SLtt_set_color(3, NULL, "green", DEFAULT_BG);	/* bold-reverse */
    SLtt_set_color(5, NULL, "blue", DEFAULT_BG);	/* bold-underline */
    SLtt_set_color(6, NULL, "red", DEFAULT_BG);		/* reverse-underline */
    SLtt_set_color(7, NULL, "magenta", "cyan");		/* reverse-underline-bold */
    /*
     * Now set monochrome attributes.
     */
    lynx_setup_attrs();
}

static void sl_suspend(int sig)
{
#ifdef SIGSTOP
#ifndef VMS
    int r, c;

    lynx_enable_mouse(0);
    if (sig == SIGTSTP)
	SLsmg_suspend_smg();
    SLang_reset_tty();
    kill(getpid(), SIGSTOP);
#if SLANG_VERSION > 9929
    SLang_init_tty(-1, 0, 1);
#else
    SLang_init_tty(3, 0, 1);
#endif /* SLANG_VERSION > 9929 */
    signal(SIGTSTP, sl_suspend);
#if defined(REAL_UNIX_SYSTEM) && !defined(__CYGWIN__)
    SLtty_set_suspend_state(1);
#endif
    if (sig == SIGTSTP)
	SLsmg_resume_smg();
    /*
     * Get new window size in case it changed.
     */
    r = SLtt_Screen_Rows;
    c = SLtt_Screen_Cols;
    size_change(0);
    if ((r != SLtt_Screen_Rows) || (c != SLtt_Screen_Cols)) {
	recent_sizechange = TRUE;
    }
    lynx_enable_mouse(1);
#endif /* !VMS */
#endif /* SIGSTOP */
    return;
}
#else

#ifdef FANCY_CURSES

#ifndef VMS
/* *INDENT-OFF* */
/* definitions for the mono attributes we can use */
static struct {
    const char *name;
    int code;
} Mono_Attrs[7] =
{
    { "normal",		A_NORMAL },
    { "bold",		A_BOLD },
    { "reverse",	A_REVERSE },
    { "underline",	A_UNDERLINE },
    { "standout",	A_STANDOUT },
    { "blink",		A_BLINK },
    { "dim",		A_DIM },
};
/* *INDENT-ON* */

int string_to_attr(char *name)
{
    unsigned i;

    for (i = 0; i < TABLESIZE(Mono_Attrs); i++) {
	if (!strcasecomp(Mono_Attrs[i].name, name)) {
	    return Mono_Attrs[i].code;
	}
    }
    return 0;
}
#endif /* VMS */

#ifdef USE_COLOR_STYLE
static char *attr_to_string(int code)
{
    static char result[sizeof(Mono_Attrs) + 80];
    unsigned i;
    int pair = PAIR_NUMBER(code);
    int bold = (pair != 0 && (code & A_BOLD) != 0);

    if (bold)
	code &= ~A_BOLD;

    *result = 0;
    for (i = 0; i < TABLESIZE(Mono_Attrs); i++) {
	if (Mono_Attrs[i].code & code) {
	    if (*result)
		strcat(result, "+");
	    strcat(result, Mono_Attrs[i].name);
	}
    }
    if (pair != 0) {
	short f, b;

	if (pair_content((short) pair, &f, &b) != ERR) {
	    const char *fg = lookup_color(bold ? f + COLORS : f);
	    const char *bg = lookup_color(b);

	    if (*result)
		strcat(result, "+");
	    sprintf(result + strlen(result), "%s/%s", fg, bg);
	}
    }
    return result;
}
#endif /* USE_COLOR_STYLE */
#endif /* FANCY_CURSES */
#endif /* USE_SLANG */

/*
 *  This function boxes windows for (n)curses.
 */
void LYbox(WINDOW * win, BOOLEAN formfield GCC_UNUSED)
{
#ifdef USE_SLANG
    SLsmg_draw_box(win->top_y, win->left_x, win->height, win->width + 4);
#else
#ifdef VMS
    /*
     * This should work for VAX-C and DEC-C, since they both have the same
     * win._max_y and win._max_x members -TD
     *
     * (originally VMSbox by FM)
     */
    int i;

    wmove(win, 0, 0);
    waddstr(win, "\033)0\016l");
    for (i = 1; i < win->_max_x; i++)
	waddch(win, 'q');
    waddch(win, 'k');
    for (i = 1; i < win->_max_y - 1; i++) {
	wmove(win, i, 0);
	waddch(win, 'x');
	wmove(win, i, win->_max_x - 1);
	waddch(win, 'x');
    }
    wmove(win, i, 0);
    waddch(win, 'm');
    for (i = 1; i < win->_max_x; i++)
	waddch(win, 'q');
    waddstr(win, "j\017");
#else /* !VMS */
    int boxvert, boxhori;

    UCSetBoxChars(current_char_set, &boxvert, &boxhori, BOXVERT, BOXHORI);
#ifdef CSS
    if (formfield)
	wcurses_css(win, "frame", ABS_ON);
#endif
    /*
     * If we don't have explicitly specified characters for either vertical or
     * horizontal lines, the characters that box() would use for the corners
     * probably also won't work well.  So we specify our own ASCII characters
     * for the corners and call wborder() instead of box().  - kw
     */
    LynxWChangeStyle(win, s_menu_frame, STACK_ON);
#ifdef HAVE_WBORDER
    if (!boxvert || !boxhori)
	box(win, boxvert, boxhori);
    else if (boxvert == '*' || boxhori == '*')
	wborder(win, boxvert, boxvert, boxhori, boxhori, '*', '*', '*', '*');
    else
	wborder(win, boxvert, boxvert, boxhori, boxhori, '/', '\\', '\\', '/');
#else
    box(win, boxvert, boxhori);
#endif
    LynxWChangeStyle(win, s_menu_frame, STACK_OFF);
#ifdef CSS
    if (formfield)
	wcurses_css(win, "frame", ABS_OFF);
#endif
#endif /* VMS */
    wrefresh(win);
#endif /* USE_SLANG */
}

#if defined(USE_COLOR_STYLE)
/* Ok, explanation of the USE_COLOR_STYLE styles.  The basic styles (ie non
 * HTML) are set the same as the SLANG version for ease of programming.  The
 * other styles are simply the HTML enum from HTMLDTD.h + 16.
 */
HTCharStyle displayStyles[DSTYLE_ELEMENTS];

/*
 * set a style's attributes - RP
 */
void setStyle(int style,
	      int color,
	      int cattr,
	      int mono)
{
    displayStyles[style].color = color;
    displayStyles[style].cattr = cattr;
    displayStyles[style].mono = mono;
}

void setHashStyle(int style,
		  int color,
		  int cattr,
		  int mono,
		  char *element)
{
    bucket *ds = &hashStyles[style];

    CTRACE2(TRACE_STYLE,
	    (tfp, "CSS(SET): <%s> hash=%d, ca=%#x, ma=%#x\n",
	     element, style, color, mono));

    ds->color = color;
    ds->cattr = cattr;
    ds->mono = mono;
    ds->code = style;
    FREE(ds->name);
    StrAllocCopy(ds->name, element);
}

/*
 * set the curses attributes to be color or mono - RP
 */
static void LYAttrset(WINDOW * win, int color,
		      int mono)
{
    if (lynx_has_color
	&& LYShowColor >= SHOW_COLOR_ON
	&& color >= 0) {
	CTRACE2(TRACE_STYLE, (tfp, "CSS:LYAttrset color %#x -> (%s)\n",
			      color, attr_to_string(color)));
	wattrset(win, color);
    } else if (mono >= 0) {
	CTRACE2(TRACE_STYLE, (tfp, "CSS:LYAttrset mono %#x -> (%s)\n",
			      mono, attr_to_string(mono)));
	wattrset(win, mono);
    } else {
	CTRACE2(TRACE_STYLE, (tfp, "CSS:LYAttrset (A_NORMAL)\n"));
	wattrset(win, A_NORMAL);
    }
}

void curses_w_style(WINDOW * win, int style,
		    int dir)
{
#if OMIT_SCN_KEEPING
# define SPECIAL_STYLE /*(CSHASHSIZE+1) */ 88888
/* if TRACEs are not compiled in, this macro is redundant - we needn't valid
'ds' to stack off. */
#endif

    int YP, XP;

#if !OMIT_SCN_KEEPING
    bucket *ds = (style == NOSTYLE ? &nostyle_bucket : &hashStyles[style]);

#else
    bucket *ds = (style == NOSTYLE ? &nostyle_bucket :
		  (style == SPECIAL_STYLE ? &special_bucket : &hashStyles[style]));
#endif

    if (!ds->name) {
	CTRACE2(TRACE_STYLE, (tfp, "CSS.CS:Style %d not configured\n", style));
#if !OMIT_SCN_KEEPING
	return;
#endif
    }

    CTRACE2(TRACE_STYLE, (tfp, "CSS.CS:<%s%s> style %d code %#x, color %#x\n",
			  (dir ? "" : "/"),
			  ds->name, style, ds->code, ds->color));

    getyx(win, YP, XP);

    if (style == s_normal && dir) {
	LYAttrset(win, ds->color, ds->mono);
	if (win == LYwin && CACHE_VALIDATE_YX(YP, XP))
	    cached_styles[YP][XP] = s_normal;
	return;
    }

    switch (dir) {
	/* ABS_OFF is the same as STACK_OFF for the moment */
    case STACK_OFF:
	if (last_colorattr_ptr) {
	    int last_attr = last_styles[--last_colorattr_ptr];

	    LYAttrset(win, last_attr, last_attr);
	} else
	    LYAttrset(win, A_NORMAL, -1);
	break;

    case STACK_ON:		/* remember the current attributes */
	if (last_colorattr_ptr >= MAX_LAST_STYLES) {
	    CTRACE2(TRACE_STYLE, (tfp, "........... %s (0x%x) %s\r\n",
				  "attribute cache FULL, dropping last",
				  last_styles[last_colorattr_ptr],
				  "in LynxChangeStyle(curses_w_style)"));
	    last_colorattr_ptr = MAX_LAST_STYLES - 1;
	}
	last_styles[last_colorattr_ptr++] = LYgetattrs(win);
	/* don't cache style changes for active links */
#if OMIT_SCN_KEEPING
	/* since we don't compute the hcode to stack off in HTML.c, we
	 * don't know whether this style is configured.  So, we
	 * shouldn't simply return on stacking on unconfigured
	 * styles, we should push curr attrs on stack.  -HV
	 */
	if (!ds->name)
	    break;
#endif
	/* FALL THROUGH */
    case ABS_ON:		/* change without remembering the previous style */
	/* don't cache style changes for active links and edits */
	if (style != s_alink
	    && style != s_curedit
	    && style != s_aedit
	    && style != s_aedit_sel
	    && style != s_aedit_pad
	    && style != s_aedit_arr) {
	    CTRACE2(TRACE_STYLE, (tfp, "CACHED: <%s> @(%d,%d)\n",
				  ds->name, YP, XP));
	    if (win == LYwin && CACHE_VALIDATE_YX(YP, XP))
		cached_styles[YP][XP] = style;
	}
	LYAttrset(win, ds->color, ds->mono);
	break;
    }
}

/*
 * wrapper function to set on-screen styles - RP
 */
void wcurses_css(WINDOW * win, char *name,
		 int dir)
{
    int try_again = 1;

    while (try_again) {
	int tmpHash = hash_code(name);

	CTRACE2(TRACE_STYLE, (tfp, "CSSTRIM:trying to set [%s] style - ", name));
	if (tmpHash == NOSTYLE) {
	    char *pclass = strrchr(name, '.');

	    CTRACE2(TRACE_STYLE, (tfp, "undefined, trimming at %p\n", pclass));
	    if (pclass)
		*pclass = '\0';
	    else
		try_again = 0;
	} else {
	    CTRACE2(TRACE_STYLE, (tfp, "ok (%d)\n", hash_code(name)));
	    curses_w_style(win, hash_code(name), dir);
	    try_again = 0;
	}
    }
}

void curses_css(char *name,
		int dir)
{
    wcurses_css(LYwin, name, dir);
}

void curses_style(int style,
		  int dir)
{
    curses_w_style(LYwin, style, dir);
}
#endif /* USE_COLOR_STYLE */

static BOOL lynx_called_initscr = FALSE;

#if defined(USE_COLOR_TABLE) && defined(COLOR_CURSES)
#define COLOR_CFG_MAX 8

/*
 * This block of code is designed to produce the same color effects using SVr4
 * curses as the slang library's implementation in this module.  That maps the
 * SGR codes into a 0-7 index into the color table, with special treatment for
 * backgrounds.  There's a bit of convoluted (but necessary) code handling the
 * special case of initialization before 'initscr()' is called.
 * 1997/1/19 - T.E.Dickey <dickey@clark.net>
 */
/* *INDENT-OFF* */
#define COLOR_CFG(c) c, (c) == DEFAULT_COLOR
static struct {
    int fg, dft_fg, bg, dft_bg;
} lynx_color_cfg[] = {
    /*0*/ { COLOR_CFG(DEFAULT_FG),     COLOR_CFG(DEFAULT_BG)},
    /*1*/ { COLOR_CFG(COLOR_BLUE),     COLOR_CFG(DEFAULT_BG)},
    /*2*/ { COLOR_CFG(COLOR_YELLOW+8), COLOR_CFG(COLOR_BLUE)},
    /*3*/ { COLOR_CFG(COLOR_GREEN),    COLOR_CFG(DEFAULT_BG)},
    /*4*/ { COLOR_CFG(COLOR_MAGENTA),  COLOR_CFG(DEFAULT_BG)},
    /*5*/ { COLOR_CFG(COLOR_BLUE),     COLOR_CFG(DEFAULT_BG)},
    /*6*/ { COLOR_CFG(COLOR_RED),      COLOR_CFG(DEFAULT_BG)},
    /*7*/ { COLOR_CFG(COLOR_MAGENTA),  COLOR_CFG(COLOR_CYAN)}
};
/* *INDENT-ON* */

#define COLOR_PAIRS_MAX (COLOR_CFG_MAX * 3 + 1)

/*
 * Hold the codes for color-pairs here until 'initscr()' is called.
 */
static struct {
    int fg;
    int bg;
} lynx_color_pairs[COLOR_PAIRS_MAX];

/*
 * If we find an exact match for the given default colors, force curses to use
 * color pair 0, which corresponds to the terminal's default colors.  Normally
 * curses assumes white-on-black, but we can override the assumption with this
 * function.
 */
static int get_color_pair(int n)
{
#ifdef USE_CURSES_PAIR_0
    if ((n < (int) TABLESIZE(lynx_color_pairs))
	&& lynx_color_pairs[n].fg == default_fg
	&& lynx_color_pairs[n].bg == default_bg)
	return 0;
#endif
    return COLOR_PAIR(n);
}

/*
 * Lynx "knows" about 16 colors.  ANSI colors (and most color terminal
 * emulators) only go to 8, though some curses implementations (ncurses and
 * PDCurses) handle 16.  If lynx's configuration calls for a color past the
 * number of colors that the terminal handles (COLORS), map the extra value
 * to bold.
 */
#define is_boldc(c) ((c) > (COLORS-1))
#define map2bold(c) (is_boldc(c) ? ((c) & (COLORS-1)) : (c))

/*
 * Return the extra color as A_BOLD.
 * If there is no extra color, return A_NORMAL.
 */
static int lynx_color_cfg_attr(int code)
{
    int result = A_NORMAL;

    if (code >= 0 && code < COLOR_CFG_MAX) {
	int fg = lynx_color_cfg[code].fg;

	if (is_boldc(fg) && (fg & COLORS))
	    result = A_BOLD;
    }
    return result;
}

static int encode_color_attr(int color_attr)
{
    int result;
    int code = 0;
    int offs = 1;

    if (color_attr & A_BOLD)
	code |= 1;
    if (color_attr & (A_REVERSE | A_DIM))
	code |= 2;
    if (color_attr & A_UNDERLINE)
	code |= 4;
    result = lynx_color_cfg_attr(code);

    if (code + offs < COLOR_PAIRS) {
	result |= get_color_pair(code + offs);
    }
    return result;
}

static int decode_mono_code(int mono_code)
{
    int result = 0;

    if (mono_code & 1)
	result |= A_BOLD;
    if (mono_code & 2)
	result |= A_REVERSE;
    if (mono_code & 4)
	result |= A_UNDERLINE;

    return result;
}

/*
 * Map the SGR attributes (0-7) into ANSI colors, modified with the actual BOLD
 * attribute to get 16 colors.
 */
int LYgetTableAttr(void)
{
    int result;

    if (lynx_has_color && LYShowColor >= SHOW_COLOR_ON) {
	result = encode_color_attr(Current_Attr);
    } else {
	result = Current_Attr;
    }
    return result & ~Masked_Attr;
}

#ifdef USE_COLOR_STYLE
/*
 * Return a string that corresponds to the attributes that would be returned by
 * LYgetTableAttr().
 */
char *LYgetTableString(int code)
{
    int mask = decode_mono_code(code);
    int second = encode_color_attr(mask);
    int pair = PAIR_NUMBER(second);
    int mono = mask & A_ATTRIBUTES;
    int fg = lynx_color_pairs[pair].fg;
    int bg = lynx_color_pairs[pair].bg;
    unsigned n;
    char *result = 0;

    CTRACE((tfp, "LYgetTableString(%d)\n", code));

    if (fg == 0 && bg == 0) {
	fg = COLOR_WHITE;
    }
    CTRACE((tfp, "%#x -> %#x (mono %#x pair %d) fg=%d, bg=%d\n",
	    mask, second, mono, pair, fg, bg));
    for (n = 0; n < TABLESIZE(Mono_Attrs); ++n) {
	if ((Mono_Attrs[n].code & mono) != 0) {
	    if (result != 0)
		StrAllocCat(result, "+");
	    StrAllocCat(result, Mono_Attrs[n].name);
	}
    }
    if (result == 0)
	StrAllocCopy(result, "normal");
    StrAllocCat(result, ":");
    StrAllocCat(result, lookup_color(fg));
    if (bg >= 0) {
	StrAllocCat(result, ":");
	StrAllocCat(result, lookup_color(bg));
    }
    CTRACE((tfp, "->%s\n", result));
    return result;
}
#endif

/*
 * Initialize a curses color-pair based on our configured color values.
 */
static void lynx_init_color_pair(int n)
{
#ifdef USE_COLOR_STYLE
    (void) n;			/* we only use lynx_color_pairs[] data */
#else
    int m;

    if (lynx_called_initscr) {
	for (m = 0; m <= 16; m += 8) {
	    int pair = n + m + 1;

	    if (pair < COLOR_PAIRS)
		init_pair((short) pair,
			  (short) map2bold(lynx_color_pairs[pair].fg),
			  (short) map2bold(lynx_color_pairs[pair].bg));
	}
	if (n == 0 && LYShowColor >= SHOW_COLOR_ON) {
	    wbkgd(LYwin, COLOR_BKGD | ' ');
	}
    }
#endif
}

static void lynx_map_color(int n)
{
    int j;

    CTRACE((tfp, "lynx_map_color(%d)\n", n));

    if (n + 1 < (int) TABLESIZE(lynx_color_pairs)) {
	for (j = n + 1; j < COLOR_PAIRS_MAX; j += COLOR_CFG_MAX) {
	    lynx_color_pairs[j].fg = lynx_color_cfg[n].fg;
	    lynx_color_pairs[j].bg = lynx_color_cfg[n].bg;
	}

	/* special case (does not apply to 3rd set) */
	lynx_color_pairs[n + 1 + COLOR_CFG_MAX].bg = lynx_color_cfg[0].bg;
    }

    lynx_init_color_pair(n);
}

/*
 * Change a configured color value.  This may be called before initscr(), so
 * we may not be able to call init_pair() to finish the change.
 */
int lynx_chg_color(int color,
		   int fg,
		   int bg)
{
    CTRACE((tfp, "lynx_chg_color(color=%d, fg=%d, bg=%d)\n", color, fg, bg));

    if (fg == ERR_COLOR || bg == ERR_COLOR)
	return -1;
    if (color >= 0 && color < COLOR_CFG_MAX) {
	lynx_color_cfg[color].fg = fg;
	lynx_color_cfg[color].bg = bg;
	lynx_map_color(color);
    } else {
	return -1;
    }
    return 0;
}

void lynx_set_color(int a)
{
    if (lynx_has_color && LYShowColor >= SHOW_COLOR_ON) {
	wattrset(LYwin, lynx_color_cfg_attr(a)
		 | (((a + 1) < COLOR_PAIRS)
		    ? (chtype) get_color_pair(a + 1)
		    : A_NORMAL));
    }
}

void lynx_standout(int flag)
{
    if (flag)
	LYaddAttr(A_REVERSE);
    else
	LYsubAttr(A_REVERSE);
}

static void lynx_init_colors(void)
{
    if (lynx_has_color) {
	size_t n;

	CTRACE((tfp, "lynx_init_colors (default %d/%d)\n",
		default_fg, default_bg));

	lynx_color_cfg[0].fg = default_fg;
	lynx_color_cfg[0].bg = default_bg;

	for (n = 0; n < TABLESIZE(lynx_color_cfg); n++) {
	    lynx_init_color_pair(n);
	}
    } else if (LYShowColor != SHOW_COLOR_NEVER) {
	LYShowColor = SHOW_COLOR_OFF;
    }
}

void lynx_setup_colors(void)
{
    int n;

    CTRACE((tfp, "lynx_setup_colors\n"));
#ifdef USE_DEFAULT_COLORS
    if (!LYuse_default_colors) {
	for (n = 0; n < COLOR_CFG_MAX; n++) {
	    if (lynx_color_cfg[n].dft_fg)
		lynx_color_cfg[n].fg = COLOR_BLACK;
	    if (lynx_color_cfg[n].dft_bg)
		lynx_color_cfg[n].bg = COLOR_WHITE;
	}
    }
#endif
    for (n = 0; n < COLOR_CFG_MAX; n++)
	lynx_map_color(n);
}
#endif /* USE_COLOR_TABLE */

void LYnoVideo(int a)
{
    CTRACE((tfp, "LYnoVideo(%d)\n", a));
#ifdef USE_SLANG
    if (a & 1)
	Masked_Attr |= SLTT_BOLD_MASK;
    if (a & 2)
	Masked_Attr |= SLTT_REV_MASK;
    if (a & 4)
	Masked_Attr |= SLTT_ULINE_MASK;
    lynx_setup_attrs();
#else
#ifdef USE_COLOR_TABLE
    Masked_Attr = decode_mono_code(a);
#endif
#endif
}

#define NEWTERM_NAME "newterm"
#if       !defined(VMS) && !defined(USE_SLANG)
/*
 * If newterm is not defined, assume a curses subset which
 * supports only initscr.  --gil
 */
#if defined(HAVE_NEWTERM) && defined(HAVE_DELSCREEN) && !defined(PDCURSES) && !(defined(NCURSES) && defined(HAVE_RESIZETERM))
static SCREEN *LYscreen = NULL;

#define LYDELSCR() { \
if (recent_sizechange) { \
    CTRACE((tfp, "Screen size: delscreen()\n")); \
    delscreen(LYscreen); \
    LYscreen = NULL; } }
/*
 * Surrogates for newterm and delscreen
 */
#else /* HAVE_NEWTERM   */
static WINDOW *LYscreen = NULL;

#undef  NEWTERM_NAME
#define NEWTERM_NAME "initscr"
#undef  newterm
#define newterm(type, out, in) (initscr())
#define LYDELSCR()		/* nothing */
#endif /* HAVE_NEWTERM   */
#else /* !defined(VMS) && !defined(USE_SLANG) */
/*
 * Provide last recourse definitions of LYscreen and LYDELSCR for
 * stop_curses, which only tests LYscreen for zero/nonzero but
 * never uses it as a pointer or L-value.
 */
#define LYscreen TRUE
#define LYDELSCR()		/* nothing */
#endif /* !defined(VMS) && !defined(USE_SLANG) */

#if defined(PDCURSES) && defined(PDC_BUILD) && PDC_BUILD >= 2401
int saved_scrsize_x = 0;
int saved_scrsize_y = 0;
#endif

void start_curses(void)
{
#ifdef USE_SLANG
    static int slinit;

    if (LYCursesON) {
	CTRACE((tfp, "start_curses: Hmm, already ON.\n"));
	return;
    }

    if (slinit == 0) {
#if defined(HAVE_TTYNAME)
	if (isatty(fileno(stdout)) && LYReopenInput() < 0) {
	    fprintf(stderr, "Cannot open tty input\n");
	    exit_immediately(EXIT_FAILURE);
	}
#endif
#if defined(USE_KEYMAPS)
	if (-1 == lynx_initialize_keymaps())
	    exit_immediately(EXIT_FAILURE);
#else
	SLtt_get_terminfo();
#endif
#if (defined(__DJGPP__) && !defined(DJGPP_KEYHANDLER)) || defined(__CYGWIN__)
	SLkp_init();
#endif /* __DJGPP__ && !DJGPP_KEYHANDLER */

#if defined(REAL_UNIX_SYSTEM) && !defined(__CYGWIN__)
#if SLANG_VERSION >= 9935
	SLang_TT_Read_FD = fileno(stdin);
#endif /* SLANG_VERSION >= 9935 */
#endif /* REAL_UNIX_SYSTEM && !__CYGWIN__ */

#if !defined(USE_KEYMAPS) && defined(ENHANCED_LINEEDIT) && defined(ESCDELAY)
	/* way to get ESC that's not part of a recognized sequence through */
	ESCDELAY = 2000;
#endif
	/*
	 * Check whether a saved show_color:off override is in effect.  - kw
	 */
	if (LYrcShowColor == SHOW_COLOR_NEVER) {
	    SLtt_Use_Ansi_Colors = 0;
	}
	/*
	 * Check whether we're forcing color on.  - FM
	 */
	if ((LYShowColor > 1) && (Lynx_Color_Flags & SL_LYNX_USE_COLOR))
	    SLtt_Use_Ansi_Colors = 1;
	/*
	 * Check whether a -nocolor override is in effect.  - kw
	 */
	if (Lynx_Color_Flags & SL_LYNX_OVERRIDE_COLOR)
	    SLtt_Use_Ansi_Colors = 0;
	/*
	 * Make sure our flags are in register.  - FM
	 */
	if (SLtt_Use_Ansi_Colors == 1) {
	    if (LYShowColor != SHOW_COLOR_ALWAYS) {
		LYShowColor = SHOW_COLOR_ON;
	    }
	} else {
	    if (LYShowColor != SHOW_COLOR_NEVER) {
		LYShowColor = SHOW_COLOR_OFF;
	    }
	}
	size_change(0);

#if (defined(VMS) || defined(REAL_UNIX_SYSTEM)) && !defined(__CYGWIN__)
	if ((Masked_Attr & SLTT_ULINE_MASK) == 0) {
	    SLtt_add_color_attribute(4, SLTT_ULINE_MASK);
	    SLtt_add_color_attribute(5, SLTT_ULINE_MASK);
	}
	/*
	 * If set, the blink escape sequence will turn on high intensity
	 * background (rxvt and maybe Linux console).
	 */
	SLtt_Blink_Mode = term_blink_is_boldbg;
#endif /* (VMS || REAL_UNIX_SYSTEM) && !__CYGWIN__  */
    }
#ifdef __DJGPP__
    _eth_init();
#endif /* __DJGPP__ */

    slinit = 1;
    Current_Attr = 0;
#ifndef VMS
#if SLANG_VERSION > 9929
    SLang_init_tty(-1, 0, 1);
#else
    SLang_init_tty(3, 0, 1);
#endif /* SLANG_VERSION > 9929 */
#endif /* !VMS */
    SLsmg_init_smg();
    SLsmg_Display_Eight_Bit = LYlowest_eightbit[current_char_set];
    if (SLsmg_Display_Eight_Bit > 191)
	SLsmg_Display_Eight_Bit = 191;	/* may print ctrl chars otherwise - kw */
    scrollok(0, 0);
    SLsmg_Backspace_Moves = 1;
#if SLANG_VERSION > 10306
    SLsmg_touch_screen();
#endif
#ifndef VMS
#if defined(REAL_UNIX_SYSTEM) && !defined(__CYGWIN__)
    SLtty_set_suspend_state(1);
#endif /* REAL_UNIX_SYSTEM && !__CYGWIN__ */
#ifdef SIGTSTP
    if (!no_suspend)
	signal(SIGTSTP, sl_suspend);
#endif /* SIGTSTP */
    signal(SIGINT, cleanup_sig);
#endif /* !VMS */

    lynx_enable_mouse(1);

#else /* USE_SLANG; Now using curses: */
    int keypad_on = 0;

#ifdef VMS
    /*
     * If we are VMS then do initscr() everytime start_curses() is called!
     */
    CTRACE((tfp, "Screen size: initscr()\n"));
    initscr();			/* start curses */
#else /* Unix: */

#if defined(HAVE_TTYNAME)
    if (isatty(fileno(stdout)) && LYReopenInput() < 0) {
	fprintf(stderr, "Cannot open tty input\n");
	exit_immediately(EXIT_FAILURE);
    }
#endif

#ifdef __CYGWIN__
    /*
     * Workaround for buggy Cygwin, which breaks subprocesses of a
     * full-screen application (tested with cygwin dll, dated
     * 2002/6/23 -TD)
     */
    if (!lynx_called_initscr) {
	FILE *fp = fopen("/dev/tty", "w");

	if (fp != 0)
	    stdout = fp;
    }
#endif

    if (!LYscreen) {
	/*
	 * If we're not VMS then only do initscr() one time, and one time only!
	 */
#if defined(HAVE_NEWTERM)
#if !(defined(NCURSES) && !defined(HAVE_RESIZETERM))
	BOOLEAN savesize;

	savesize = recent_sizechange;
	size_change(0);
	recent_sizechange = savesize;	/* avoid extra redraw */
#if defined(__MVS__)
	{
	    /*
	     * The requirement to do this may be a bug in OS/390.
	     *
	     * Put screen geometry in environment variables used by
	     * XOpen curses before calling newterm().  I believe this
	     * completes work left unfinished by AJL & FM -- gil
	     */
	    static char lines_putenv[] = "LINES=abcde", cols_putenv[] = "COLUMNS=abcde";

	    sprintf(lines_putenv + 6, "%d", LYlines & 0xfff);
	    sprintf(cols_putenv + 8, "%d", LYcols & 0xfff);
	    putenv(lines_putenv);
	    putenv(cols_putenv);
	    CTRACE((tfp, "start_curses putenv %s, %s\n", lines_putenv, cols_putenv));
	}
#endif /* defined(__MVS__) */
#endif /* !(defined(NCURSES) && defined(HAVE_RESIZETERM)) */
	CTRACE((tfp, "Screen size: %s()\n", NEWTERM_NAME));
	if (!(LYscreen = newterm(NULL, stdout, stdin))) {	/* start curses */
	    fprintf(tfp, "%s\n",
		    gettext("Terminal initialisation failed - unknown terminal type?"));
	    exit_immediately(EXIT_FAILURE);
	}
#else
	CTRACE((tfp, "Screen size: initscr()\n"));
	initscr();
#endif /* HAVE_NEWTERM */
	lynx_called_initscr = TRUE;
	LYlines = LYscreenHeight();
	LYcols = LYscreenWidth();

#if defined(SIGWINCH) && defined(NCURSES_VERSION)
	size_change(0);
	recent_sizechange = FALSE;	/* prevent mainloop drawing 1st doc twice */
#endif /* SIGWINCH */
	CTRACE((tfp, "Screen size is now %d x %d\n", LYlines, LYcols));

#ifdef USE_CURSES_PADS
	if (LYuseCursesPads) {
	    LYwin = newpad(LYlines, MAX_COLS);
	    LYshiftWin = 0;
	    LYwideLines = FALSE;
	} else {
	    LYwin = stdscr;
	}
#endif

#if defined(USE_KEYMAPS) && defined(NCURSES_VERSION)
#  if HAVE_KEYPAD
	/* Need to switch keypad on before initializing keymaps, otherwise
	   when the keypad is switched on, some keybindings may be overriden. */
	keypad(LYwin, TRUE);
	keypad_on = 1;
#  endif			/* HAVE_KEYPAD */

	if (-1 == lynx_initialize_keymaps()) {
	    endwin();
	    exit_immediately(EXIT_FAILURE);
	}
#endif

	/*
	 * This is a workaround for a bug in SVr4 curses, observed on Solaris
	 * 2.4:  if your terminal's alternate-character set contains codes in
	 * the range 128-255, they'll be sign-extended in the acs_map[] table,
	 * which in turn causes their values to be emitted as 255 (0xff).
	 * "Fix" this by forcing the table to 8-bit codes (it has to be
	 * anyway).
	 */
#if defined(ALT_CHAR_SET) && !defined(NCURSES_VERSION)
	{
	    int n;

	    for (n = 0; n < 128; n++)
		if (ALT_CHAR_SET[n] & 0x80) {
		    ALT_CHAR_SET[n] &= 0xff;
		    ALT_CHAR_SET[n] |= A_ALTCHARSET;
		}
	}
#endif

#if defined(USE_COLOR_STYLE) || defined(USE_COLOR_TABLE)
	if (has_colors()) {
	    lynx_has_color = TRUE;
	    start_color();

#ifndef COLORS
	    /* map2boldc() relies on COLORS being a power of 2 */
	    if (COLORS > 16)
		COLORS = 16;
	    if (COLORS < 8)
		COLORS = 2;
	    if (COLORS > 8 && COLORS != 16)
		COLORS = 8;
#endif

#ifdef USE_DEFAULT_COLORS
	    if (LYuse_default_colors) {
#if defined(EXP_ASSUMED_COLOR) && defined(USE_COLOR_TABLE)
		/*
		 * Adjust the color mapping table to match the ASSUMED_COLOR
		 * setting in lynx.cfg
		 */
		if (assume_default_colors(default_fg, default_bg) != OK) {
		    default_fg = COLOR_WHITE;
		    default_bg = COLOR_BLACK;
		}
		CTRACE((tfp, "initializing default colors %d/%d\n",
			default_fg, default_bg));
		if (default_fg >= 0 || default_bg >= 0) {
		    unsigned n;

		    for (n = 0; n < TABLESIZE(lynx_color_cfg); n++) {
			if (default_fg >= 0 && lynx_color_cfg[n].fg < 0)
			    lynx_color_cfg[n].fg = default_fg;
			if (default_bg >= 0 && lynx_color_cfg[n].bg < 0)
			    lynx_color_cfg[n].bg = default_bg;
			CTRACE((tfp, "color_cfg[%u] = %d/%d\n", n,
				lynx_color_cfg[n].fg,
				lynx_color_cfg[n].bg));
		    }
		    lynx_setup_colors();
		}
#else
#if defined(HAVE_USE_DEFAULT_COLORS)
		if (!default_color_reset) {
		    if (lynx_called_initscr) {
			if (LYuse_default_colors && (use_default_colors() == OK)) {
			    default_fg = DEFAULT_COLOR;
			    default_bg = DEFAULT_COLOR;
			} else {
			    default_fg = COLOR_WHITE;
			    default_bg = COLOR_BLACK;
			    default_color_reset = TRUE;
			}
		    }
		}
#endif /* HAVE_USE_DEFAULT_COLORS */
#endif /* EXP_ASSUMED_COLOR */
	    }
#endif /* USE_DEFAULT_COLORS */
	}
#endif /* USE_COLOR_STYLE || USE_COLOR_TABLE */

#ifdef USE_COLOR_STYLE
	/* Curses forgets color settings when we call delscreen() */
	if (!isEmpty(lynx_lss_file) && LYCanReadFile(lynx_lss_file)) {
	    style_readFromFile(lynx_lss_file);
	}
	parse_userstyles();
#endif
#ifdef USE_COLOR_TABLE
	lynx_init_colors();
#endif
    }
#ifdef __DJGPP__
    _eth_init();
#endif /* __DJGPP__ */
#endif /* not VMS */

/* nonl();   *//* seems to slow things down */

#ifdef VMS
    crmode();
    raw();
#else
#ifdef HAVE_CBREAK
    cbreak();
#else
    crmode();
#endif /* HAVE_CBREAK */
    signal(SIGINT, cleanup_sig);
#endif /* VMS */

    noecho();

#ifdef HAVE_KEYPAD
    if (!keypad_on)
	keypad(LYwin, TRUE);
#endif /* HAVE_KEYPAD */

    lynx_enable_mouse(1);

    fflush(stdin);
    fflush(stdout);
    fflush(stderr);
#endif /* USE_SLANG */

#if defined(WIN_EX)
    LYclear();
#endif

#if defined(USE_BLINK) && defined(__EMX__)
    if (term_blink_is_boldbg)	/* Now actually make it so! */
	make_blink_boldbg();
#endif

    LYCursesON = TRUE;
#if defined(PDCURSES) && defined(PDC_BUILD) && PDC_BUILD >= 2401
    if ((scrsize_x != 0) && (scrsize_y != 0)) {
	if (saved_scrsize_x == 0) {
	    saved_scrsize_x = COLS;
	    saved_scrsize_y = LINES;
	}
	CTRACE((tfp, "resize_term: x=%d, y=%d\n", scrsize_x, scrsize_y));
	CTRACE((tfp, "saved terminal size: x=%d, y=%d\n", saved_scrsize_x, saved_scrsize_y));
	resize_term(scrsize_y, scrsize_x);
	LYclear();
    }
#endif
    CTRACE((tfp, "start_curses: done.\n"));
}				/* end of start_curses() */

void lynx_enable_mouse(int state)
{
#ifdef USE_MOUSE
/***********************************************************************/

#if defined(WIN_EX)
/* modify lynx_enable_mouse() for pdcurses configuration so that mouse support
   is disabled unless -use_mouse is specified
*/
    HANDLE hConIn = INVALID_HANDLE_VALUE;

    hConIn = GetStdHandle(STD_INPUT_HANDLE);
    if (LYUseMouse == 0) {
	SetConsoleMode(hConIn, ENABLE_WINDOW_INPUT);
	FlushConsoleInputBuffer(hConIn);
	return;
    }
#endif

    (void) state;

    if (LYUseMouse == 0)
	return;

#if defined(USE_SLANG)
    SLtt_set_mouse_mode(state, 0);
    SLtt_flush_output();
#else

#if defined(WIN_EX) && defined(PDCURSES)
    if (state) {
	SetConsoleMode(hConIn, ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
	FlushConsoleInputBuffer(hConIn);
    }
#else
#if defined(NCURSES)
    if (state) {
	/* Compensate for small value of maxclick in ncurses.  */
	static int was = 0;

	if (!was) {
	    int old = mouseinterval(-1);

	    was++;
	    if (old < 200)	/* Default 166 */
		mouseinterval(300);
	}
	/* Inform ncurses which mouse events we're interested in.
	 * We shouldn't need to include BUTTONn_PRESSED and BUTTONn_RELEASED
	 * events, since ncurses should translate them to click events. - kw
	 * However, if we do not include them, then ncurses effectively
	 * ignores mouseinterval(), thus translates *any* sequence of
	 * press/release to a click, which leads to inconveniences.
	 * We special-case these events in LYStrings.c.
	 */
	mousemask(BUTTON_CTRL | BUTTON_ALT
		  | BUTTON1_PRESSED | BUTTON1_RELEASED
		  | BUTTON1_CLICKED
		  | BUTTON1_DOUBLE_CLICKED | BUTTON1_TRIPLE_CLICKED
		  | BUTTON2_PRESSED | BUTTON2_RELEASED
		  | BUTTON2_CLICKED
		  | BUTTON3_PRESSED | BUTTON3_RELEASED
		  | BUTTON3_CLICKED
		  | BUTTON3_DOUBLE_CLICKED | BUTTON3_TRIPLE_CLICKED,
		  NULL);
    } else
	mousemask(0, NULL);
#endif /* NCURSES */
#endif /* WIN_EX and PDCURSES */

#if defined(PDCURSES)
    if (state)
	mouse_set(
		     BUTTON1_CLICKED | BUTTON1_PRESSED | BUTTON1_RELEASED |
		     BUTTON2_CLICKED | BUTTON2_PRESSED | BUTTON2_RELEASED |
		     BUTTON3_CLICKED | BUTTON3_PRESSED | BUTTON3_RELEASED);
#endif
#endif /* NOT USE_SLANG */

/***********************************************************************/
#endif /* USE_MOUSE */
}

/*
 * SVr4 curses (and ncurses) initialize the terminal I/O to raw mode, and
 * simulate other modes in the library.  This means that when running, it
 * simulates the OCRNL setting.  Normally that is not a problem.  However, when
 * spawning a subprocess (e.g., xli), the subprocess may write to the screen.
 * Fine so far - curses resets the terminal I/O to the normal state on exit.
 * But the subprocess's messages can still be coming to the screen when lynx
 * returns to the screen mode.  This function delays restoring OCRNL until
 * after the first getch() call.
 *
 * The OCRNL setting is controlled by nl()/nonl() of course - but we do not
 * want to give up that optimization since it would be a bit slower.  (Note -
 * slang does not use this optimization; if it did, the same screen glitch
 * would occur).
 *
 * FIXME:  for simplicity, only ncurses is implemented here - the TTY and
 * SET_TTY definitions are ncurses-specific.  The same effect could be done for
 * other curses implementations, since the "cur_term->Nttyb" part is common to
 * SVr4 curses.
 */
void lynx_nl2crlf(int normal GCC_UNUSED)
{
#if defined(NCURSES_VERSION_PATCH) && defined(SET_TTY) && defined(TERMIOS) && defined(ONLCR)
    static TTY saved_tty;
    static int did_save = FALSE;
    static int waiting = FALSE;
    static int can_fix = TRUE;

    if (!did_save) {
	saved_tty = cur_term->Nttyb;
	did_save = TRUE;
#if NCURSES_VERSION_PATCH < 20010529
	/* workaround for optimizer bug with nonl() */
	if ((tigetstr("cud1") != 0 && *tigetstr("cud1") == '\n')
	    || (tigetstr("ind") != 0 && *tigetstr("ind") == '\n'))
	    can_fix = FALSE;
#endif
    }
    if (can_fix) {
	if (normal) {
	    if (!waiting) {
		cur_term->Nttyb.c_oflag |= ONLCR;
		waiting = TRUE;
		nonl();
	    }
	} else {
	    if (waiting) {
		cur_term->Nttyb = saved_tty;
		SET_TTY(fileno(stdout), &saved_tty);
		waiting = FALSE;
		nl();
		LYrefresh();
	    }
	}
    }
#endif
}

void stop_curses(void)
{
    if (LYCursesON) {
	echo();
    }
#if defined(PDCURSES) && defined(PDC_BUILD) && PDC_BUILD >= 2401
    resetty();
#endif

#ifdef __DJGPP__
    _eth_release();
#endif /* __DJGPP__ */

#if defined(DOSPATH) && !(defined(USE_SLANG) || defined(_WIN_CC))
#if defined(PDCURSES)
    endwin();
#else /* !PDCURSES */
#ifdef __DJGPP__
    ScreenClear();
#else /* some flavor of win32?  */
#ifdef __MINGW32__
    clear();
#else
    clrscr();
#endif
#endif /* win32 */
#endif /* PDCURSES */
#else

    if (LYCursesON == TRUE) {
	lynx_nl2crlf(TRUE);
	lynx_enable_mouse(0);
	if (LYscreen || lynx_called_initscr) {
	    endwin();		/* stop curses */
	    LYDELSCR();
	}
    } else {
#ifdef SH_EX
	int i;

	for (i = 0; i <= 3; i++) {
	    printf("\r\n");
	}
#endif
    }

    fflush(stdout);
#endif /* defined(DOSPATH) && !(defined(USE_SLANG) || defined(_WIN_CC)) */
    fflush(stderr);

    LYCursesON = FALSE;
    CTRACE((tfp, "stop_curses: done.\n"));

#if defined(SIGTSTP) && defined(USE_SLANG)
#ifndef VMS
    if (!no_suspend)
	signal(SIGTSTP, SIG_DFL);
#endif /* !VMS */
#endif /* SIGTSTP && USE_SLANG */

#ifndef VMS
    signal(SIGINT, SIG_DFL);
#endif /* !VMS */
}

#ifdef VMS

#ifdef USE_SLANG
extern void longname(char *, char *);
#endif /* USE_SLANG */

/*
 * Check terminal type, start curses & setup terminal.
 */
BOOLEAN setup(char *terminal)
{
    int c;
    int status;
    char *dummy = 0, *cp, term[81];

    /*
     * If the display was not set by a command line option then see if it is
     * available from the environment.
     */
    if ((cp = LYgetXDisplay()) != 0) {
	StrAllocCopy(x_display, cp);
    } else {
	FREE(x_display);
    }

    /*
     * Get terminal type, and convert to lower case.
     */
    term[0] = '\0';
    longname(dummy, term);
    if (term[0] == '\0' && (form_get_data || form_post_data)) {
	/*
	 * Some yoyo used these under conditions which require -dump, so force
	 * that mode here.  - FM
	 */
	dump_output_immediately = TRUE;
	LYcols = DFT_COLS;
	if (keypad_mode == NUMBERS_AS_ARROWS)
	    keypad_mode = LINKS_ARE_NUMBERED;
	status = mainloop();
	exit_immediately(status);
    }
    LYLowerCase(term);

    printf("%s%s\n", gettext("Terminal ="), term);
    if ((strlen(term) < 5) ||
	strncmp(term, "vt", 2) || !isdigit(term[2])) {
	printf("%s\n",
	       gettext("You must use a vt100, 200, etc. terminal with this program."));
	printf(CONFIRM_PROCEED, "n/y");
	c = getchar();
	if (c != 'y' && c != 'Y') {
	    printf("\n");
	    return (FALSE);
	}
	strcpy(term, "vt100");
    }

    ttopen();
    start_curses();

    LYlines = LYscreenHeight();
    LYcols = LYscreenWidth();

    return (TRUE);
}

#else /* Not VMS: */

/*
 * Check terminal type, start curses & setup terminal.
 */
BOOLEAN setup(char *terminal)
{
    char *term_putenv = NULL;
    char *buffer = NULL;
    char *cp;

    /*
     * If the display was not set by a command line option then see if it is
     * available from the environment .
     */
    if ((cp = LYgetXDisplay()) != NULL) {
	StrAllocCopy(x_display, cp);
    } else {
	FREE(x_display);
    }

    if (terminal != NULL) {
	HTSprintf0(&term_putenv, "TERM=%.106s", terminal);
	(void) putenv(term_putenv);
    }

    /*
     * Query the terminal type.
     */
    if (dumbterm(LYGetEnv("TERM"))) {
	printf("\n\n  %s\n\n", gettext("Your Terminal type is unknown!"));
	printf("  %s [vt100] ", gettext("Enter a terminal type:"));

	if (LYSafeGets(&buffer, stdin) != 0) {
	    LYTrimLeading(buffer);
	    LYTrimTrailing(buffer);
	}

	if (isEmpty(buffer))
	    StrAllocCopy(buffer, "vt100");

	HTSprintf0(&term_putenv, "TERM=%.106s", buffer);
	FREE(buffer);

	(void) putenv(term_putenv);
	printf("\n%s %s\n", gettext("TERMINAL TYPE IS SET TO"),
	       LYGetEnv("TERM"));
	LYSleepMsg();
    }

    start_curses();

#ifdef HAVE_TTYTYPE
    /*
     * Account for lossage on the 'sun' terminal type (80x24) Sun text console
     * driver.  It only supports reverse video, but all SGR sequences produce
     * that same reverse video, and the terminfo entry lists different SGRs for
     * 'bold' and 'rev'.  As a result, the current link is indistinguishable
     * from all other links.  The workaround here is to disable the 'rev'
     * capability.
     */
    if ((strncmp((const char *) ttytype, "sun", 3) == 0)) {
	LYnoVideo(2);
    }
#endif /* HAVE_TTYTYPE */

    LYlines = LYscreenHeight();
    LYcols = LYscreenWidth();

    return (1);
}

static int dumbterm(char *terminal)
{
    int dumb = FALSE;

    /*
     * Began checking for terminal == NULL in case that TERM environment
     * variable is not set.  Thanks to Dick Wesseling (ftu@fi.ruu.nl).
     */
    if (terminal == NULL ||
	!strcasecomp(terminal, "network") ||
	!strcasecomp(terminal, "unknown") ||
	!strcasecomp(terminal, "dialup") ||
	!strcasecomp(terminal, "dumb") ||
	!strcasecomp(terminal, "switch") ||
	!strcasecomp(terminal, "ethernet"))
	dumb = TRUE;
    return (dumb);
}

#ifdef FANCY_CURSES
#ifndef USE_COLOR_STYLE
#ifdef USE_COLOR_TABLE
static void LYsetWAttr(WINDOW * win)
{
    wattrset(win, LYgetTableAttr());
}

void LYaddWAttr(WINDOW * win, int a)
{
    Current_Attr |= a;
    LYsetWAttr(win);
}

void LYaddAttr(int a)
{
    LYaddWAttr(LYwin, a);
}

void LYsubWAttr(WINDOW * win, int a)
{
    Current_Attr &= ~a;
    LYsetWAttr(win);
}

void LYsubAttr(int a)
{
    LYsubWAttr(LYwin, a);
}
#endif /* USE_COLOR_TABLE */
#endif /* !USE_COLOR_STYLE */
#endif /* FANCY_CURSES */
#endif /* VMS */

/* Use this rather than the 'wprintw()' function to write a blank-padded
 * string to the given window, since someone's asserted that printw doesn't
 * handle 8-bit characters unlike addstr (though more info would be useful).
 *
 * We're blank-filling so that with SVr4 curses, it'll show the background
 * color to a uniform width in the popup-menu.
 */
#ifndef USE_SLANG
void LYpaddstr(WINDOW * the_window, int width,
	       const char *the_string)
{
    int y, x;
    int actual = strlen(the_string);

    getyx(the_window, y, x);
    if (width + x > LYcolLimit)
	width = LYcolLimit - x;
    if (actual > width)
	actual = width;
    LYwaddnstr(the_window, the_string, actual);
    width -= actual;
    while (width-- > 0)
	waddstr(the_window, " ");
}

/*
 * Work around limitation of curses's order-of-refresh by setting a pointer to
 * the topmost window that should be displayed.
 *
 * FIXME: the associated call on 'keypad()' is not needed for Unix, but
 * something in the OS/2 EMX port requires it.
 */
static WINDOW *my_subwindow;

void LYsubwindow(WINDOW * param)
{
    if (param != 0) {
	my_subwindow = param;
#if defined(NCURSES) || defined(PDCURSES)
	keypad(my_subwindow, TRUE);
#if defined(USE_COLOR_STYLE)
	LynxWChangeStyle(my_subwindow, s_menu_bg, STACK_ON);
	{
	    long b = LYgetattrs(my_subwindow);

	    wbkgd(my_subwindow, b | ' ');
	}
	LynxWChangeStyle(my_subwindow, s_menu_bg, STACK_OFF);
#elif defined(HAVE_GETBKGD)	/* not defined in ncurses 1.8.7 */
	wbkgd(my_subwindow, getbkgd(LYwin));
#endif
#endif
	scrollok(my_subwindow, TRUE);
    } else {
	touchwin(LYwin);
	delwin(my_subwindow);
	my_subwindow = 0;
    }
}

WINDOW *LYtopwindow(void)
{
    return (my_subwindow ? my_subwindow : LYwin);
}
#endif

WINDOW *LYstartPopup(int *top_y,
		     int *left_x,
		     int *height,
		     int *width)
{
    WINDOW *form_window = 0;

#ifdef USE_SLANG
    static WINDOW fake_window;

    if (*left_x < 1 || (*left_x + *width + 4) >= LYcolLimit) {
	*left_x = 1;
	*width = LYcolLimit - 5;
    }

    SLsmg_fill_region(*top_y, *left_x - 1, *height, *width + 4, ' ');
    form_window = &fake_window;
    form_window->top_y = *top_y;
    form_window->left_x = *left_x;
    form_window->height = *height;
    form_window->width = *width;
#else
    if (*left_x > 0 && (*left_x + *width + 4) < LYcolLimit)
	form_window = newwin(*height, *width + 4, *top_y, *left_x - 1);
    if (form_window == 0) {
	if (*width > LYcolLimit - 4) {
	    *width = LYcolLimit - 4;
	    *left_x = 1;
	} else {
	    *left_x = LYcolLimit - 4 - *width;
	}
	form_window = newwin(*height, *width + 4, *top_y, *left_x - 1);
    }
    if (form_window == 0) {
	HTAlert(POPUP_FAILED);
    } else {
	LYsubwindow(form_window);
    }
#endif /* USE_SLANG */
    return form_window;
}

void LYstartTargetEmphasis(void)
{
#ifdef USE_COLOR_STYLE
    if (s_whereis != NOSTYLE) {
	curses_style(s_whereis, STACK_ON);
	return;
    }
#endif
#if defined(FANCY_CURSES) || defined(USE_SLANG)
    lynx_start_bold();
    lynx_start_reverse();
#endif /* FANCY_CURSES || USE_SLANG */
    lynx_start_underline();
}

void LYstopTargetEmphasis(void)
{
#ifdef USE_COLOR_STYLE
    if (s_whereis != NOSTYLE) {
	curses_style(s_whereis, STACK_OFF);
	return;
    }
#endif
    lynx_stop_underline();
#if defined(FANCY_CURSES) || defined(USE_SLANG)
    lynx_stop_reverse();
    lynx_stop_bold();
#endif /* FANCY_CURSES || USE_SLANG */
}

/*
 * Accommodate the different flavors of touchline
 */
void LYtouchline(int row)
{
#if defined(HAVE_WREDRAWLN) && !defined(NCURSES_VERSION)
    wredrawln(LYwin, row, 1);
#else
#if defined(HAVE_TOUCHLINE)
    /* touchline() is not available on VMS before version 7.0, and then only on
     * Alpha, since prior ports of curses were broken.  BSD touchline() has a
     * 4th parameter since it is used internally by touchwin().
     */
#if defined(HAVE_BSD_TOUCHLINE)
    touchline(LYwin, row, 0, COLS);
#else
    touchline(LYwin, row, 1);
#endif
#else
#if !defined(USE_SLANG)
    touchwin(LYwin);
#else
    SLsmg_touch_lines(row, 1);
#endif
#endif
#endif
}

/*
 * Wrapper for waddnstr().
 */
void LYwaddnstr(WINDOW * w GCC_UNUSED,
		const char *src,
		size_t len)
{
#ifdef USE_CURSES_PADS
    /*
     * If we've configured to use pads for left/right scrolling, that can
     * interfere with calls to this function that assume they're wrapping. 
     * Writing to a pad which is wider than the screen will simply not wrap.
     *
     * Link-highlighting uses wrapping.  You can see this by viewing the
     * options screen in a terminal which is narrower than 80 columns.
     *
     * Check for that case, and use curses's wrapping in a derived window to
     * simplify things, e.g., in case the string contains multibyte or
     * multicolumn characters.
     */
    int y0, x0;

    getyx(LYwin, y0, x0);

    if (LYuseCursesPads
	&& (LYwin == w)		/* popups do not wrap */
	&&LYshiftWin == 0
	&& LYwideLines == FALSE
	&& ((int) len > (LYcolLimit - x0))
	&& (x0 < LYcolLimit)) {
	WINDOW *sub = derwin(LYwin, LYlines, LYcolLimit, 0, 0);

	if (sub != 0) {
	    wmove(sub, y0, x0);
	    LYwideLines = TRUE;
	    LYwaddnstr(sub, src, len);
	    getyx(sub, y0, x0);
	    delwin(sub);
	    wmove(LYwin, y0, x0);
	}
	LYwideLines = FALSE;

	return;
    }
#endif
    /*
     * We only want to trace this function for the color-style code.  It would
     * be too much logging if not needed.
     */
#ifdef USE_COLOR_STYLE
    if (TRACE) {
	int y, x;

	LYGetYX(y, x);
	CTRACE2(TRACE_STYLE, (tfp, "[%2d,%2d] LYwaddnstr(%.*s, %u)\n",
			      y, x, (int) len, src, (unsigned) len));
    }
#endif
    /*
     * There's no guarantee that a library won't temporarily write on its input.
     * Be safe and copy it when we have const-data.
     */
    while ((int) len > 0) {
	char temp[MAX_LINE];
	size_t use = (len >= MAX_LINE) ? MAX_LINE - 1 : len;

	memcpy(temp, src, use);
	temp[use] = 0;
	waddstr(w, temp);
	len -= use;
    }
}

/*
 * Determine the number of cells the given string would take up on the screen,
 * limited by the maxCells parameter.  This is used for constructing aligned
 * text in the options and similar forms.
 *
 * FIXME: make this account for wrapping, too.
 * FIXME: make this useful for "lynx -dump", which hasn't initialized curses.
 */
int LYstrExtent(const char *string, int len, int maxCells)
{
    int result = 0;
    int used;

    if (len < 0)
	used = strlen(string);
    else
	used = len;

    result = used;
#ifdef WIDEC_CURSES
    if (used > 0 && lynx_called_initscr) {
	static WINDOW *fake_win;
	static int fake_max;
	int n;

	if (fake_max < maxCells) {
	    fake_max = (maxCells + 1) * 2;
	    if (fake_win != 0) {
		delwin(fake_win);
		fake_win = 0;
	    }
	}
	if (fake_win == 0) {
	    fake_win = newwin(2, fake_max, 0, 0);
	}
	if (fake_win != 0) {
	    int new_x = 0;
	    int new_y = 0;

	    result = 0;
	    wmove(fake_win, 0, 0);
	    for (n = 0; n < used; ++n) {
		if (IsNormalChar(string[n])) {
		    waddch(fake_win, UCH(string[n]));
		    getyx(fake_win, new_y, new_x);
		    if (new_y > 0 || new_x > maxCells)
			break;
		    result = new_x;
		}
	    }
	}
    }
#endif
    if (result > maxCells)
	result = maxCells;
    return result;
}

/*
 * A simple call that relies upon the coincidence that multicell characters
 * use at least as many bytes as cells.
 */
int LYstrExtent2(const char *string, int len)
{
    return LYstrExtent(string, len, len);
}

/*
 * Returns the total number of cells that the string would use.
 */
int LYstrCells(const char *string)
{
    return LYstrExtent2(string, strlen(string));
}

#ifdef VMS
/*
 *	Cut-down termio --
 *		Do character-oriented stream input for Jeff.
 *		Code ripped off from Micro-Emacs 3.7 by Daniel Lawrence.
 *
 *		Ever-so-slightly modified by Kathryn Huxtable.	29-Jan-1991.
 *		Cut down for Lou.  8 Sep 1992.
 *		Cut down farther for Lou.  19 Apr 1993.
 *			We don't set PASSALL or PASTHRU since we don't
 *			want to block CTRL/C, CTRL/Y, CTRL/S or CTRL/Q.
 *			Simply setting NOECHO and doing timed reads
 *			is sufficient.
 *		Further mods by Fote.  29-June-1993
 *			ttopen() and ttclose() are now terminal initialization
 *			 and restoration procedures, called once at startup
 *			 and at exit, respectively, of the LYNX image.
 *			ttclose() should be called before an exit from LYNX
 *			 no matter how the exit is invoked.
 *			setup(terminal) does the ttopen().
 *			cleanup() calls cleanup_files() and ttclose().
 *			ttgetc() now handles NOECHO and NOFLITR (instead of
 *			 setting the terminal itself to NOECHO in ttopen()).
 *			VMSsignal() added for handling both Ctrl-C *and* Ctrl-Y
 *			 interrupts, and disabling system response to Ctrl-T.
 *		Further mods by Fote.  15-Dec-1993
 *			Added edit handler in ttopen() which will invoke
 *			 VMSexit() and behave intelligently on ACCVIO's.
 *		Further mods by Fote.  29-Dec-1993
 *			Simplified ttgetc().
 *		Further mods by Fote.  16-Jan-1994
 *			Added code in ttopen() which will invoke VMSVersion()
 *			 to get the version of VMS as VersionVMS for use by
 *			 by new or modified interrupt or spawning routines.
 *		Further mods by Fote.  27-Jan-1994
 *			Added back a typeahead() which supports 'z' or 'Z' as
 *			an "Zap transfer" command via HTCheckForInterrupt()
 *			in LYUtils.c.
 */

#include <descrip.h>
#include <iodef.h>
#include <ssdef.h>
#include <msgdef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <libclidef.h>
#include <lib$routines.h>
#include <starlet.h>
#include <clidef.h>
#include <syidef.h>
#ifdef signal
#undef signal
#endif /* signal */
#include <signal.h>
#ifdef system
#undef system
#endif /* system */
#include <processes.h>
#include <LYVMSdef.h>

#define EFN	0		/* Event flag                   */

static unsigned char buffer[20];	/* Input buffer                 */
static int in_pos, in_len;	/* For escape sequences         */
static int oldmode[3];		/* Old TTY mode bits            */
static int newmode[3];		/* New TTY mode bits            */
static short iochan;		/* TTY I/O channel              */
static $DESCRIPTOR(term_nam_dsc, "TT");		/* Descriptor for iochan        */
static unsigned long mask = LIB$M_CLI_CTRLY | LIB$M_CLI_CTRLT;	/* ^Y and ^T */
static unsigned long old_msk;	/* Saved control mask           */
static short trap_flag = FALSE;	/* TRUE if AST is set           */
BOOLEAN DidCleanup = FALSE;	/* Exit handler flag            */
static char VersionVMS[20];	/* Version of VMS               */

int VMSVersion(char *VerString,
	       int VerLen)
{
    unsigned long status, itm_cod = SYI$_VERSION;
    int i, verlen = 0;
    struct dsc$descriptor version;
    char *m;

    version.dsc$a_pointer = VerString;
    version.dsc$w_length = VerLen - 1;
    version.dsc$b_dtype = DSC$K_DTYPE_B;
    version.dsc$b_class = DSC$K_CLASS_S;

    status = lib$getsyi(&itm_cod, 0, &version, &verlen, 0, 0);
    if (!(status & 1) || verlen == 0)
	return 0;

    /*
     * Cut out trailing spaces
     */
    for (m = VerString + verlen, i = verlen - 1; i > 0 && VerString[i] == ' '; --i)
	*(--m) = '\0';

    return strlen(VerString) + 1;	/* Transmit ending 0 too */
}

void VMSexit(void)
{
    /*
     * If we get here and DidCleanup is not set, it was via an ACCVIO, or
     * outofmemory forced exit, so make *sure* we attempt a cleanup and reset
     * the terminal.
     */
    if (!DidCleanup) {
	if (LYOutOfMemory == FALSE) {
	    fprintf(stderr,
		    gettext("\nA Fatal error has occurred in %s Ver. %s\n"),
		    LYNX_NAME, LYNX_VERSION);
	    fprintf(stderr,
		    gettext("\nPlease notify your system administrator to confirm a bug, and if\n\
confirmed, to notify the lynx-dev list.  Bug reports should have concise\n\
descriptions of the command and/or URL which causes the problem, the\n\
operating system name with version number, the TCPIP implementation, the\n\
TRACEBACK if it can be captured, and any other relevant information.\n"));

	    if (LYTraceLogFP == NULL) {
		fprintf(stderr, RETURN_TO_CLEANUP);
		(void) getchar();
	    }
	} else if (LYCursesON) {
	    HTAlert(MEMORY_EXHAUSTED_ABORT);
	}
	cleanup();
    }
    if (LYOutOfMemory == TRUE) {
	printf("\r\n%s\r\n\r\n", MEMORY_EXHAUSTED_ABORT);
	fflush(stdout);
	fflush(stderr);
    }
}

/*
 *	TTOPEN --
 *		This function is called once to set up the terminal
 *		device streams.  It translates TT until it finds
 *		the terminal, then assigns a channel to it, sets it
 *		to EDIT, and sets up the Ctrl-C and Ctrl-Y interrupt
 *		handling.
 */
int ttopen(void)
{
    int iosb[2];
    int status;
    static unsigned long condition;
    static struct _exit_block {
	unsigned long forward;
	unsigned long address;
	unsigned long zero;
	unsigned long condition;
    } exit_handler_block;

    status = sys$assign(&term_nam_dsc, &iochan, 0, 0);
    if (status != SS$_NORMAL)
	exit_immediately(status);

    status = sys$qiow(EFN, iochan, IO$_SENSEMODE, &iosb, 0, 0,
		      &oldmode, sizeof(oldmode), 0, 0, 0, 0);
    if (status != SS$_NORMAL)
	exit_immediately(status);

    status = iosb[0] & 0xFFFF;
    if (status != SS$_NORMAL)
	exit_immediately(status);

    newmode[0] = oldmode[0];
    newmode[1] = oldmode[1];
    newmode[2] = oldmode[2] | TT2$M_EDIT;

    status = sys$qiow(EFN, iochan, IO$_SETMODE, &iosb, 0, 0,
		      &newmode, sizeof(newmode), 0, 0, 0, 0);
    if (status != SS$_NORMAL)
	exit_immediately(status);

    status = iosb[0] & 0xFFFF;
    if (status != SS$_NORMAL)
	exit_immediately(status);

    /*
     * Declare the exit handler block.
     */
    exit_handler_block.forward = 0;
    exit_handler_block.address = (unsigned long) &VMSexit;
    exit_handler_block.zero = 0;
    exit_handler_block.condition = (unsigned long) &condition;
    status = sys$dclexh(&exit_handler_block);
    if (status != SS$_NORMAL)
	exit_immediately(status);

    /*
     * Set the AST.
     */
    lib$disable_ctrl(&mask, &old_msk);
    trap_flag = TRUE;
    status = sys$qiow(EFN, iochan,
		      IO$_SETMODE | IO$M_CTRLCAST | IO$M_CTRLYAST,
		      &iosb, 0, 0,
		      &cleanup_sig, SIGINT, 0, 0, 0, 0);
    if (status != SS$_NORMAL) {
	lib$enable_ctrl(&old_msk);
	exit_immediately(status);
    }

    /*
     * Get the version of VMS.
     */
    if (VMSVersion(VersionVMS, 20) < 3)
	/*
	 * Load zeros on error.
	 */
	strcpy(VersionVMS, "V0.0-0");

    return (0);
}				/*  ttopen  */

/*
 *	TTCLOSE --
 *		This function gets called just before we go back home
 *		to the command interpreter.  It puts the terminal back
 *		in a reasonable state.
 */
int ttclose(void)
{
    int status;
    int iosb[1];

    status = sys$qiow(EFN, iochan, IO$_SETMODE, &iosb, 0, 0,
		      &oldmode, sizeof(oldmode), 0, 0, 0, 0);

    if (status != SS$_NORMAL || (iosb[0] & 0xFFFF) != SS$_NORMAL)
	exit_immediately(status);

    if (trap_flag) {
	status = sys$dassgn(iochan);
	status = lib$enable_ctrl(&old_msk);
	trap_flag = FALSE;
    }
    return (0);
}				/*  ttclose  */

/*
 *	TTGETC --
 *		Read a character from the terminal, with NOECHO and NOFILTR.
 */
int ttgetc(void)
{
    int status;
    unsigned short iosb[4];

    if (in_pos < in_len)
	return (buffer[in_pos++]);

    status = sys$qiow(EFN, iochan,
		      IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR,
		      &iosb, 0, 0,
		      &buffer, 1, 0, 0, 0, 0);
    if ((status & 1) == 1)
	status = iosb[0];
    if (status == SS$_PARTESCAPE) {
	/*
	 * Escape sequence in progress.  Fake a successful read.
	 */
	status = 1;
    }
    if ((status & 1) != 1 && status != SS$_DATAOVERUN)
	exit_immediately(status);
    in_pos = 1;
    in_len = iosb[1] + iosb[3];
    return (buffer[0]);
}

/*
 *	TYPEAHEAD -- Fote Macrides 27-Jan-1994
 *		Check whether a keystroke has been entered, and return
 *		 it, or -1 if none was entered.
 */
int typeahead(void)
{
    int status;
    unsigned short iosb[4];

    if (dump_output_immediately)
	return -1;

    if (in_pos < in_len)
	return (buffer[in_pos++]);

  again:
    status = sys$qiow(EFN, iochan,
		      IO$_READVBLK | IO$M_TIMED | IO$M_NOECHO | IO$M_NOFILTR,
		      &iosb, 0, 0,
		      &buffer, 1, 0, 0, 0, 0);
    if ((status & 1) == 1)
	status = iosb[0];
    if (status == SS$_PARTESCAPE) {
	/*
	 * Escape sequence in progress, finish reading it.
	 */
	goto again;
    }

    in_pos = 1;
    in_len = iosb[1] + iosb[3];
    if (status == SS$_TIMEOUT || status == SS$_DATAOVERUN)
	return (-1);
    return (buffer[0]);
}

/*
 *	VMSSIGNAL -- Fote Macrides 29-Jun-1993
 *		Sets up AST for both Ctrl-C and Ctrl-Y, with system response
 *		 to Ctrl-T disabled.  If called with a sig other than SIGINT,
 *		 it will use the C library's system(sig, func).
 *		The equivalent of VMSsignal(SIGINT, cleanup_sig) is done on
 *		 intialization by ttopen(), so don't do it again.
 *		VMSsignal(SIGINT, SIG_DFL) is treated as a call to ttclose().
 *		Call VMSsignal(SIGINT, SIG_IGN) before system() calls to
 *		 enable Ctrl-C and Ctrl-Y in the subprocess, and then call
 *		 VMSsignal(SIG_INT, cleanup_sig) on return from the subprocess.
 *		For func's which set flags and do not invoke an exit from
 *		 LYNX, the func should reassert itself.
 *		The VMS signal() calls do not fully emulate the Unix calls,
 *		 and VMSsignal() is just a "helper", also not a full emulation.
 */

void VMSsignal(int sig,
	       void (*func) ())
{
    int status;
    short iosb[4];
    static int SIG_IGN_flag;

    /*
     * Pass all signals other than SIGINT to signal().
     * Also pass SIGINT to signal() if we're dumping.
     */
    if (sig != SIGINT || dump_output_immediately) {
	signal(sig, func);
	return;
    }

    /*
     * If func is SIG_DFL, treat it as ttclose().
     */
    if (func == SIG_DFL) {
	ttclose();
	return;
    }

    /*
     * Clear any previous AST.
     */
    if (trap_flag) {
	status = sys$dassgn(iochan);
	status = lib$enable_ctrl(&old_msk);
	trap_flag = FALSE;
    }

    /*
     * If func is SIG_IGN, leave the TT channel closed and the system response
     * to interrupts enabled for system() calls.
     */
    if (func == SIG_IGN)
	return;

    /*
     * If we get to here, we have a LYNX func, so set the AST.
     */
    lib$disable_ctrl(&mask, &old_msk);
    trap_flag = TRUE;
    status = sys$assign(&term_nam_dsc, &iochan, 0, 0);
    status = sys$qiow(EFN, iochan,
		      IO$_SETMODE | IO$M_CTRLCAST | IO$M_CTRLYAST,
		      &iosb, 0, 0,
		      func, SIGINT, 0, 0, 0, 0);

}				/* VMSsignal */

/*
 * DCLspawn_exception, spawn_DCLprocess, DCLsystem -- F.Macrides 16-Jan-1994
 *	Exception-handler routines for regulating interrupts and enabling
 *	Control-T during spawns.  Includes TRUSTED flag for versions of VMS
 *	which require it in captive accounts.  This code should be used
 *	instead of the VAXC or DECC system(), by including LYUtils.h in
 *	modules which have system() calls.  It helps ensure that we return
 *	to Lynx instead of breaking out to DCL if a user issues interrupts
 *	or generates an ACCVIO during spawns.
 */
#ifdef __DECC
static unsigned int DCLspawn_exception(void *sigarr,
				       void *mecharr)
#else
static int DCLspawn_exception(void *sigarr,
			      void *mecharr)
#endif				/* __DECC */
{
    int status;

    status = lib$sig_to_ret(sigarr, mecharr);
    return (SS$_UNWIND);
}

static int spawn_DCLprocess(char *command)
{
    int status;
    unsigned long Status = 0;

    /*
     * Keep DECC from complaining.
     */
    struct dsc$descriptor_s command_desc;

    command_desc.dsc$w_length = strlen(command);
    command_desc.dsc$b_class = DSC$K_CLASS_S;
    command_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    command_desc.dsc$a_pointer = command;

    VAXC$ESTABLISH(DCLspawn_exception);

#ifdef __ALPHA /** OpenVMS/AXP lacked the TRUSTED flag before v6.1 **/
    if (VersionVMS[1] > '6' ||
	(VersionVMS[1] == '6' && VersionVMS[2] == '.' &&
	 VersionVMS[3] >= '1'))
#else
    if (VersionVMS[1] >= '6')
#endif /* __ALPHA */
    {
	/*
	 * Include TRUSTED flag.
	 */
	unsigned long trusted = CLI$M_TRUSTED;

	status = lib$spawn(&command_desc, 0, 0, &trusted,
			   0, 0, &Status);
	/*
	 * If it was invalid, try again without the flag.
	 */
	if (status == LIB$_INVARG)
	    status = lib$spawn(&command_desc, 0, 0, 0,
			       0, 0, &Status);
    } else
	status = lib$spawn(&command_desc, 0, 0, 0,
			   0, 0, &Status);
    /*
     * Return -1 on error.
     */
    if ((status & 1) != 1 || (Status & 1) != 1)
	return (-1);
    /*
     * Return 0 on success.
     */
    return (0);
}

int DCLsystem(char *command)
{
    int status;

    VMSsignal(SIGINT, SIG_IGN);
    status = spawn_DCLprocess(command);
    VMSsignal(SIGINT, cleanup_sig);
    /*
     * Returns 0 on success, -1 any error.
     */
    return (status);
}
#endif /* VMS */

/*
 * Return the physical screen dimensions that we're allowed to use.
 */
int LYscreenHeight(void)
{
    int result = LINES;

    if (result <= 0)
	result = DFT_ROWS;
    return result;
}

int LYscreenWidth(void)
{
    int result = COLS;

#if defined(PDCURSES_EXP) && defined(WIN_EX) && defined(CJK_EX)		/* 1999/08/26 (Thu) 17:53:38 */
    {
	extern int current_codepage;	/* PDCurses lib. */

	if (current_codepage == 932)
	    result--;
    }
#endif
    if (result <= 0)
	result = DFT_COLS;
    return result;
}

/*
 * Set the window's background color (make the pad's color agree), e.g., when
 * we have just parsed it from the config file, or after clearing the screen.
 */
void LYnormalColor(void)
{
#if defined(USE_COLOR_STYLE) && defined(USE_CURSES_PADS)
    if (LYwin != stdscr) {
	int color = displayStyles[DSTYLE_NORMAL].color;

	if (color >= 0) {
	    wbkgd(LYwin, color | ' ');
	    LYrefresh();
	}
    }
#endif
}

/*
 * The functions ifdef'd with USE_CURSES_PADS are implemented that way so we
 * don't break the slang configuration.
 */
void LYclear(void)
{
#ifdef USE_CURSES_PADS
    wclear(LYwin);
#else
    clear();
#endif
    LYnormalColor();
}

void LYclrtoeol(void)
{
#ifdef USE_CURSES_PADS
    wclrtoeol(LYwin);
#else
    clrtoeol();
#endif
}

void LYerase(void)
{
#ifdef USE_CURSES_PADS
    werase(LYwin);
#else
    erase();
#endif
    LYnormalColor();
}

void LYmove(int y, int x)
{
#ifdef USE_CURSES_PADS
    wmove(LYwin, y, x);
#else
    move(y, x);
#endif
}

void LYrefresh(void)
{
#ifdef USE_CURSES_PADS
    if (LYwin != stdscr) {
	/*
	 * Workaround for special case where lynx is prompting for a mailto,
	 * and has a subject line that is wider than the screen.  The
	 * wnoutrefresh() call resets newscr's position to match stdscr's,
	 * which happens to be the window's origin because we were not updating
	 * that, and other stray wmove's in lynx fail because the coordinate
	 * is on/after the right margin.  Force things to look ok here.
	 */
	int y, x;

	getyx(LYwin, y, x);
	if (x > LYcolLimit)
	    x = LYcolLimit;
	wmove(stdscr, y, x);

	wnoutrefresh(stdscr);
	pnoutrefresh(LYwin, 0, LYshiftWin, 0, 0, LYlines, LYscreenWidth() - 1);

	/*
	 * Keep a popup window visible.  This can happen if the user presses
	 * '/' to do a search within a popup.
	 */
	if (my_subwindow != 0) {
	    touchwin(my_subwindow);
	    wnoutrefresh(my_subwindow);
	}
	doupdate();
    } else {
	refresh();
    }
#else
    refresh();
#endif
}

void lynx_force_repaint(void)
{
    clearok(curscr, TRUE);
}

void lynx_start_title_color(void)
{
#ifdef SH_EX
    lynx_start_reverse();
#endif
}

void lynx_stop_title_color(void)
{
#ifdef SH_EX
    lynx_stop_reverse();
#endif
}

void lynx_start_link_color(int flag,
			   int pending)
{
    if (flag) {
	/* makes some terminals work wrong because
	 * they can't handle two attributes at the
	 * same time
	 */
	/* lynx_start_bold();  */
	lynx_start_reverse();
#if defined(USE_SLANG)
#ifndef __DJGPP__
	if (SLtt_Use_Ansi_Colors)
#endif /* !__DJGPP__ */
	    lynx_start_underline();
#endif /* USE_SLANG */
#if defined(FANCY_CURSES) && defined(COLOR_CURSES)
	if (lynx_has_color && LYShowColor >= SHOW_COLOR_ON)
	    lynx_start_underline();
#endif /* USE_SLANG */
    } else {
	lynx_start_bold();
	/*
	 * Make sure when flag is OFF that "unhighlighted" links will be
	 * underlined if appropriate.  - LE & FM
	 */
	if (pending)
	    lynx_start_underline();
    }
}

void lynx_stop_link_color(int flag,
			  int pending GCC_UNUSED)
{
#ifdef USE_COLOR_STYLE
    LynxChangeStyle(flag == ON ? s_alink : s_a, ABS_OFF);
#else
    if (flag) {
	lynx_stop_reverse();
#if defined(USE_SLANG)
#ifndef __DJGPP__
	if (SLtt_Use_Ansi_Colors)
#endif /* !__DJGPP__ */
	    lynx_stop_underline();
#endif /* USE_SLANG */
#if defined(FANCY_CURSES) && defined(COLOR_CURSES)
	if (lynx_has_color && LYShowColor >= SHOW_COLOR_ON)
	    lynx_stop_underline();
#endif /* FANCY_CURSES && COLOR_CURSES */
    } else {
	lynx_stop_bold();
	/*
	 * If underlining was turned on above, turn it off.  - LE & FM
	 */
	if (pending)
	    lynx_stop_underline();
    }
#endif
}

/* FIXME: consider inlining these */

void lynx_stop_target_color(void)
{
    lynx_stop_underline();
    lynx_stop_reverse();
    lynx_stop_bold();
}

void lynx_start_target_color(void)
{
    lynx_start_bold();
    lynx_start_reverse();
    lynx_start_underline();
}

void lynx_start_status_color(void)
{
#if defined(USE_COLOR_TABLE) && defined(COLOR_CURSES)
    if (lynx_has_color && LYShowColor >= SHOW_COLOR_ON)
	lynx_set_color(2);
    else
#endif
	lynx_start_reverse();
}

void lynx_stop_status_color(void)
{
#if defined(USE_COLOR_TABLE) && defined(COLOR_CURSES)
    if (lynx_has_color && LYShowColor >= SHOW_COLOR_ON)
	lynx_set_color(0);
    else
#endif
	lynx_stop_reverse();
}

void lynx_start_h1_color(void)
{
    if (bold_H1 || bold_headers)
	lynx_start_bold();
}

void lynx_stop_h1_color(void)
{
    if (bold_H1 || bold_headers)
	lynx_stop_bold();
}

void lynx_start_prompt_color(void)
{
    lynx_start_reverse();
}

void lynx_stop_prompt_color(void)
{
    lynx_stop_reverse();
}

void lynx_start_radio_color(void)
{
    lynx_start_bold();
}

void lynx_stop_radio_color(void)
{
    lynx_stop_bold();
}

void lynx_stop_all_colors(void)
{
    lynx_stop_underline();
    lynx_stop_reverse();
    lynx_stop_bold();
}

/*
 * Wrappers for LYUnderlineLinks flag.
 */
void lynx_start_bold(void)
{
    start_bold();
}

void lynx_start_reverse(void)
{
    start_reverse();
}

void lynx_start_underline(void)
{
    start_underline();
}

void lynx_stop_bold(void)
{
    stop_bold();
}

void lynx_stop_reverse(void)
{
    stop_reverse();
}

void lynx_stop_underline(void)
{
    stop_underline();
}

/*
 * If LYShowCursor is ON, move the cursor to the left of the current option, so
 * that blind users, who are most likely to have LYShowCursor ON, will have
 * it's string spoken or passed to the braille interface as each option is made
 * current.  Otherwise, move it to the bottom, right column of the screen, to
 * "hide" the cursor as for the main document, and let sighted users rely on
 * the current option's highlighting or color without the distraction of a
 * blinking cursor in the window.  - FM
 */
void LYstowCursor(WINDOW * win, int row, int col)
{
    if (LYShowCursor) {
	wmove(win, row, col);
    } else {
	LYHideCursor();
    }
#ifdef USE_SLANG
    SLsmg_refresh();
#else
    wrefresh(win);
#endif /* USE_SLANG  */
}

#if defined(USE_BLINK) && defined(__EMX__)	/* Can't put it earler due to BOOLEAN conflict */
#  define BOOLEAN os2BOOLEAN
#  define INCL_VIO
#  include "os2.h"
static void make_blink_boldbg(void)
{
    VIOINTENSITY buf;		/* VIO windows have it anyway, */

    /* but FS session need a switch */
    buf.cb = sizeof(buf);
    buf.type = 2;		/* VIOINTENSITY request */
    buf.fs = 1;			/* Intensity == boldbg */
    VioSetState(&buf, 0);
}
#endif

#if defined(HAVE_WATTR_GET)
/*
 * getattrs() is not in X/Open curses, but it is more convenient than this.
 */
long LYgetattrs(WINDOW * win)
{
#if ( defined(HAVE_GETATTRS) && ( !defined(NCURSES_VERSION_MAJOR) || NCURSES_VERSION_MAJOR < 5 ) )
    long result = 0;

    result = getattrs(win);
#else
    attr_t result = 0;
    short pair = 0;

    /*
     * FIXME: this ignores the color-pair, which for most implementations is
     * not stored in the attribute value.
     */
    wattr_get(win, &result, &pair, NULL);
#endif
    return result;
}
#endif /* HAVE_WATTR_GET */

#if defined(NCURSES_VERSION_PATCH) && NCURSES_VERSION_PATCH > 20021012
#ifndef HAVE_USE_LEGACY_CODING
/*
 * Between ncurses 5.3 and 5.4 as part of fixes for wide-character mode, the
 * locale support no longer allows characters in the range 128-159 to be
 * treated as printable characters.  Here is a workaround to fool
 * waddch_nosync() into treating "all" 8-bit characters as printable.
 */
NCURSES_CONST char *unctrl(chtype ch)
{
    static char result[3];
    unsigned data = (unsigned char) ch;

    if (data < 32) {
	result[0] = '^';
	result[1] = ch | '@';
	result[2] = 0;
    } else if (data == 127) {
	result[0] = '^';
	result[1] = '?';
	result[2] = 0;
    } else {
	result[0] = data;
	result[1] = 0;
    }
    return result;
}
#endif /* HAVE_USE_LEGACY_CODING */
#endif
