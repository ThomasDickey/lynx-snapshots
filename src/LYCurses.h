#ifndef LYCURSES_H
#define LYCURSES_H

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

/*
 * Because we have to configure PDCURSES last, we may get bogus definitions
 * from the system curses library - cancel these now.
 */
#ifdef HAVE_XCURSES

#undef ASSUME_DEFAULT_COLORS
#undef COLOR_CURSES
#undef FANCY_CURSES
#undef HAVE_CBREAK
#undef HAVE_RESIZETERM
#undef HAVE_USE_DEFAULT_COLORS
#undef NCURSES
#undef USE_DEFAULT_COLORS

#define HAVE_CBREAK 1
#define COLOR_CURSES 1
#define FANCY_CURSES 1

#endif

/*
 * The simple color scheme maps the 8 combinations of bold/underline/reverse
 * to the standard 8 ANSI colors (with some variations based on context).
 */
#undef USE_COLOR_TABLE

#ifdef USE_COLOR_STYLE
#define USE_COLOR_TABLE 1	/* default color logic is used */
#else
#if defined(USE_SLANG) || defined(COLOR_CURSES)
#define USE_COLOR_TABLE 1
#endif
#endif

#ifdef TRUE
#undef TRUE			/* to prevent parse error :( */
#endif /* TRUE */
#ifdef FALSE
#undef FALSE			/* to prevent parse error :( */
#endif /* FALSE */

#ifdef USE_SLANG
#include <slang.h>

#undef WINDOW
typedef struct {
    int top_y;
    int left_x;
    int height;
    int width;
} WINDOW;

/* slang doesn't really do windows... */
#define waddch(w,c)  LYaddch(c)
#define waddstr(w,s) addstr(s)
#define wmove(win, row, col) SLsmg_gotorc(((win)?(win)->top_y:0) + (row), ((win)?(win)->left_x:0) + (col))

#ifndef SLSMG_UARROW_CHAR
#define SLSMG_UARROW_CHAR '^'
#endif

#ifndef SLSMG_DARROW_CHAR
#define SLSMG_DARROW_CHAR 'v'
#endif

#ifndef SLSMG_LARROW_CHAR
#define SLSMG_LARROW_CHAR '<'
#endif

#ifndef SLSMG_RARROW_CHAR
#define SLSMG_RARROW_CHAR '>'
#endif

#ifndef SLSMG_CKBRD_CHAR
#define SLSMG_CKBRD_CHAR '#'
#endif

#ifndef SLSMG_BLOCK_CHAR
#define SLSMG_BLOCK_CHAR '#'
#endif

#ifndef ACS_UARROW
#define ACS_UARROW  SLSMG_UARROW_CHAR
#endif

#ifndef ACS_DARROW
#define ACS_DARROW  SLSMG_DARROW_CHAR
#endif

#ifndef ACS_LARROW
#define ACS_LARROW  SLSMG_LARROW_CHAR
#endif

#ifndef ACS_RARROW
#define ACS_RARROW  SLSMG_RARROW_CHAR
#endif

#ifndef ACS_CKBOARD
#define ACS_CKBOARD SLSMG_CKBRD_CHAR
#endif

#ifndef ACS_BLOCK
#define ACS_BLOCK   SLSMG_BLOCK_CHAR
#endif

#else /* Using curses: */

#ifdef VMS
#define FANCY_CURSES
#endif /* VMS */

/*
 *	CR may be defined before the curses.h include occurs.
 *	There is a conflict between the termcap char *CR and the define.
 *	Assuming that the definition of CR will always be carriage return.
 *	06-09-94 Lynx 2-3-1 Garrett Arch Blythe
 */
#ifdef CR
#undef CR			/* to prevent parse error :( */
#define REDEFINE_CR
#endif /* CR */

#ifdef HZ
#undef HZ			/* to prevent parse error :( */
#endif /* HZ */

/* SunOS 4.x has a redefinition between ioctl.h and termios.h */
#if defined(sun) && !defined(__SVR4)
#undef NL0
#undef NL1
#undef CR0
#undef CR1
#undef CR2
#undef CR3
#undef TAB0
#undef TAB1
#undef TAB2
#undef XTABS
#undef BS0
#undef BS1
#undef FF0
#undef FF1
#undef ECHO
#undef NOFLSH
#undef TOSTOP
#undef FLUSHO
#undef PENDIN
#endif

#if defined(_MSC_VER)
#undef MOUSE_MOVED		/* conflict between PDCURSES and _WIN32 */
#endif /* _MSC_VER */

/*
 * Do this to build with glibc 2.1.3 (apparently it was not used to build a
 * system before release).
 */
#include <signal.h>

#undef CS			/* some BSD versions of curses use this */
#define CS curses_CS		/* ...but we don't */

#ifdef ERR
#undef ERR			/* all versions of curses define this */
#endif

#ifdef MOUSE_MOVED
#undef MOUSE_MOVED		/* wincon.h or MINGW32's copy of it */
#endif

#ifdef HAVE_CONFIG_H
# ifdef HAVE_NCURSESW_NCURSES_H
#  undef GCC_PRINTFLIKE		/* <libutf8.h> may define 'printf' */
#  include <ncursesw/ncurses.h>
#  undef printf			/* but we don't want that... */
# else
#  ifdef HAVE_NCURSES_NCURSES_H
#   include <ncurses/ncurses.h>
#  else
#   ifdef HAVE_NCURSES_H
#    include <ncurses.h>
#   else
#    ifdef HAVE_CURSESX_H
#     include <cursesX.h>	/* Ultrix */
#    else
#     ifdef HAVE_JCURSES_H
#      include <jcurses.h>	/* sony_news */
#     else
#      ifdef HAVE_XCURSES
#       include <xcurses.h>	/* PDCurses' UNIX port */
#      else
#       include <curses.h>	/* default */
#      endif
#     endif
#    endif
#   endif
#  endif
# endif

# if defined(wgetbkgd) && !defined(getbkgd)
#  define getbkgd(w) wgetbkgd(w)	/* workaround pre-1.9.9g bug */
# endif

# ifdef FANCY_CURSES
#  if defined(NCURSES) && defined(HAVE_NCURSESW_TERM_H)
#    include <ncursesw/term.h>
#  else
#    if defined(NCURSES) && defined(HAVE_NCURSES_TERM_H)
#      include <ncurses/term.h>
#    else
#     if defined(HAVE_NCURSESW_NCURSES_H) || defined(HAVE_NCURSES_NCURSES_H) || defined(HAVE_XCURSES)
#       undef HAVE_TERM_H	/* only use one in comparable path! */
#     endif
#     if defined(HAVE_TERM_H)
#      include <term.h>
#     endif
#   endif
#  endif
# endif

# if defined(NCURSES_VERSION) && defined(HAVE_DEFINE_KEY)
#  define USE_KEYMAPS		1
# endif

#else
# if defined(VMS) && defined(__GNUC__)
#  include <LYGCurses.h>
#  else
#   include <curses.h>		/* everything else */
# endif				/* VMS && __GNUC__ */
#endif /* HAVE_CONFIG_H */

/*
 * PDCurses' mouse code does nothing in the DJGPP configuration.
 */
#if defined(PDCURSES) && !defined(__DJGPP__) && !defined(HAVE_XCURSES)
#define USE_MOUSE 1
#endif

/*
 * Pick up the native ncurses name:
 */
#if defined(NCURSES_MOUSE_VERSION)
#define USE_MOUSE 1
#endif

/*
 * For systems where select() does not work for TTY's, we can poll using
 * curses.
 */
#if defined(_WINDOWS) || defined(__MINGW32__)
#if defined(PDCURSES) && defined(PDC_BUILD) && PDC_BUILD >= 2401
#define USE_CURSES_NODELAY 1
#endif

#if defined(NCURSES_VERSION)
#define USE_CURSES_NODELAY 1
#endif
#endif	/* _WINDOWS || __MINGW32__ */

#if defined(NCURSES_VERSION) && defined(__BEOS__)
#define USE_CURSES_NODELAY 1
#endif

/*
 * If we have pads, use them to implement left/right scrolling.
 */
#if defined(HAVE_NEWPAD) && defined(HAVE_PNOUTREFRESH) && !defined(PDCURSES)
#define USE_CURSES_PADS 1
#endif

/*
 * ncurses 1.9.9e won't work for pads, but 4.2 does (1.9.9g doesn't have a
 * convenient ifdef, though it would work).
 */
#if defined(NCURSES_VERSION) && !defined(NCURSES_VERSION_MAJOR)
#undef USE_CURSES_PADS
#endif

/*
 * Most implementations of curses treat pair 0 specially, as the default
 * foreground and background color.  Also, the COLORS variable corresponds to
 * the total number of colors.
 *
 * PDCurses does not follow these rules.  Its COLORS variable claims it has
 * 8 colors, but it actually implements 16.  That makes it hard to optimize
 * color settings against color pair 0 in a portable fashion.
 */
#if defined(COLOR_CURSES)
#if defined(PDCURSES) || defined(HAVE_XCURSES)
#define COLORS 16		/* should be a variable... */
#else
#define USE_CURSES_PAIR_0
#endif
#endif

#endif /* USE_SLANG */

#ifdef __cplusplus
extern "C" {
#endif
#ifdef USE_SLANG
#define LYstopPopup()		/* nothing */
#define LYtopwindow() LYwin
#else
    extern void LYsubwindow(WINDOW * param);
    extern WINDOW *LYtopwindow(void);

#define LYstopPopup() LYsubwindow(0)
#endif				/* NCURSES */

    extern void LYbox(WINDOW * win, BOOLEAN formfield);
    extern WINDOW *LYstartPopup(int *top_y, int *left_x, int *height, int *width);

/*
 * Useful macros not in PDCurses or very old ncurses headers.
 */
#if !defined(HAVE_GETBEGX) && !defined(getbegx)
#define getbegx(win) ((win)->_begx)
#endif
#if !defined(HAVE_GETBEGY) && !defined(getbegy)
#define getbegy(win) ((win)->_begy)
#endif
#if !defined(HAVE_GETBKGD) && !defined(getbkgd)
#define getbkgd(win) ((win)->_bkgd)
#endif

#if defined(HAVE_WATTR_GET)
    extern long LYgetattrs(WINDOW * win);

#else
#if defined(HAVE_GETATTRS) || defined(getattrs)
#define LYgetattrs(win) getattrs(win)
#else
#define LYgetattrs(win) ((win)->_attrs)
#endif
#endif				/* HAVE_WATTR_GET */

#if defined(PDCURSES)
#define HAVE_GETBKGD 1		/* can use fallback definition */
#define HAVE_NAPMS 1		/* can use millisecond-delays */
#  if defined(PDC_BUILD) && PDC_BUILD >= 2401
    extern int saved_scrsize_x;
    extern int saved_scrsize_y;
#  endif
#endif

#ifdef HAVE_NAPMS
#define SECS2Secs(n) (1000 * (n))
#define Secs2SECS(n) ((n) / 1000.0)
#define SECS_FMT "%.3f"
#else
#define SECS2Secs(n) (n)
#define Secs2SECS(n) (n)
#define SECS_FMT "%.0f"
#endif

/* Both slang and curses: */
#ifndef TRUE
#define TRUE  1
#endif				/* !TRUE */
#ifndef FALSE
#define FALSE 0
#endif				/* !FALSE */

#ifdef REDEFINE_CR
#define CR FROMASCII('\015')
#endif				/* REDEFINE_CR */

#ifdef ALT_CHAR_SET
#define BOXVERT 0		/* use alt char set for popup window vertical borders */
#define BOXHORI 0		/* use alt char set for popup window vertical borders */
#endif

#ifndef BOXVERT
#define BOXVERT '*'		/* character for popup window vertical borders */
#endif
#ifndef BOXHORI
#define BOXHORI '*'		/* character for popup window horizontal borders */
#endif

#ifndef KEY_DOWN
#undef HAVE_KEYPAD		/* avoid confusion with bogus 'keypad()' */
#endif

    extern int LYlines;		/* replaces LINES */
    extern int LYcols;		/* replaces COLS */

/*
 * The scrollbar, if used, occupies the rightmost column.
 */
#ifdef USE_SCROLLBAR
#define LYbarWidth (LYShowScrollbar ? 1 : 0)
#else
#define LYbarWidth 0
#endif

/*
 * Usable limits for display:
 */
#if defined(FANCY_CURSES) || defined(USE_SLANG)
#if defined(PDCURSES)
#define LYcolLimit (LYcols - LYbarWidth - 1)	/* PDCurses wrapping is buggy */
#else
#define LYcolLimit (LYcols - LYbarWidth)
#endif
#else
#define LYcolLimit (LYcols - 1)
#endif

#ifdef USE_CURSES_PADS
    extern WINDOW *LYwin;
    extern int LYshiftWin;
    extern int LYwideLines;
    extern int LYtableCols;
    extern BOOL LYuseCursesPads;

#else
#define LYwin stdscr
#define LYshiftWin	0
#define LYwideLines	0
#define LYtableCols	0
#endif

    extern BOOLEAN setup(char *terminal);
    extern int LYscreenHeight(void);
    extern int LYscreenWidth(void);
    extern int LYstrExtent(const char *string, int len, int maxCells);
    extern int LYstrExtent2(const char *string, int len);
    extern int LYstrCells(const char *string);
    extern void LYclear(void);
    extern void LYclrtoeol(void);
    extern void LYerase(void);
    extern void LYmove(int y, int x);
    extern void LYnoVideo(int mask);
    extern void LYnormalColor(void);
    extern void LYpaddstr(WINDOW * w, int width, const char *s);
    extern void LYrefresh(void);
    extern void LYstartTargetEmphasis(void);
    extern void LYstopTargetEmphasis(void);
    extern void LYtouchline(int row);
    extern void LYwaddnstr(WINDOW * w, const char *s, size_t len);
    extern void start_curses(void);
    extern void stop_curses(void);

#define LYaddstr(s)      LYwaddnstr(LYwin, s, strlen(s))

#ifdef VMS
    extern int DCLsystem(char *command);
    extern void VMSexit();
    extern int ttopen();
    extern int ttclose();
    extern int ttgetc();
    extern void VMSsignal(int sig, void (*func) ());
#endif				/* VMS */

#if defined(USE_COLOR_STYLE)
    extern void curses_css(char *name, int dir);
    extern void curses_style(int style, int dir);
    extern void setHashStyle(int style, int color, int cattr, int mono, char *element);
    extern void setStyle(int style, int color, int cattr, int mono);
    extern void wcurses_css(WINDOW * win, char *name, int dir);
    extern void curses_w_style(WINDOW * win, int style, int dir);

#  define LynxChangeStyle(style,dir) curses_style(style,dir)
#  define LynxWChangeStyle(win,style,dir) curses_w_style(win,style,dir)
#else
#  define LynxWChangeStyle(win,style,dir)	(void)1
#endif				/* USE_COLOR_STYLE */

#ifdef USE_COLOR_TABLE
    extern void LYaddAttr(int a);
    extern void LYsubAttr(int a);
    extern void lynx_setup_colors(void);
    extern unsigned Lynx_Color_Flags;
#endif

#if defined(USE_COLOR_TABLE) || defined(USE_SLANG)
    extern int Current_Attr;
#endif

#ifdef USE_SLANG
#define SHOW_WHEREIS_TARGETS 1

#if !defined(VMS) && !defined(DJGPP)
#define USE_MOUSE              1
#endif

#if !defined(__DJGPP__) && !defined(__CYGWIN__)
#define USE_KEYMAPS		1
#endif

#define SL_LYNX_USE_COLOR	1
#define SL_LYNX_OVERRIDE_COLOR	2

#define start_bold()      	LYaddAttr(LYUnderlineLinks ? 4 : 1)
#define start_reverse()   	LYaddAttr(2)
#define start_underline() 	LYaddAttr(LYUnderlineLinks ? 1 : 4)
#define stop_bold()       	LYsubAttr(LYUnderlineLinks ? 4 : 1)
#define stop_reverse()    	LYsubAttr(2)
#define stop_underline()  	LYsubAttr(LYUnderlineLinks ? 1 : 4)

#ifdef FANCY_CURSES
#undef FANCY_CURSES
#endif				/* FANCY_CURSES */

/*
 *  Map some curses functions to slang functions.
 */
#define stdscr ((WINDOW *)0)
#ifdef SLANG_MBCS_HACK
    extern int PHYSICAL_SLtt_Screen_Cols;

#define COLS PHYSICAL_SLtt_Screen_Cols
#else
#define COLS SLtt_Screen_Cols
#endif				/* SLANG_MBCS_HACK */
#define LINES SLtt_Screen_Rows
#define move SLsmg_gotorc
#define addstr SLsmg_write_string
    extern void LY_SLerase(void);

#define erase LY_SLerase
#define clear LY_SLerase
#define standout SLsmg_reverse_video
#define standend  SLsmg_normal_video
#define clrtoeol SLsmg_erase_eol

#ifdef SLSMG_NEWLINE_SCROLLS
#define scrollok(a,b) SLsmg_Newline_Behavior \
   = ((b) ? SLSMG_NEWLINE_SCROLLS : SLSMG_NEWLINE_MOVES)
#else
#define scrollok(a,b) SLsmg_Newline_Moves = ((b) ? 1 : -1)
#endif

#define LYaddch(ch)   SLsmg_write_char(ch)

#if SLANG_VERSION >= 20000
#define addch_raw(ch) do {                                \
                        SLsmg_Char_Type buf;              \
                        buf.nchars = 1;                   \
                        buf.wchars[0] = ch;               \
                        buf.color = Current_Attr;         \
                        SLsmg_write_raw (&buf, 1);        \
                      } while (0)
#else
#define addch_raw(ch) do {                                \
                        SLsmg_Char_Type buf;              \
                        buf = (ch) | (Current_Attr << 4); \
                        SLsmg_write_raw (&buf, 1);        \
                      } while (0)
#endif				/* SLANG_VERSION >= 20000 */

#define echo()
#define printw        SLsmg_printf

    extern int curscr;
    extern BOOLEAN FullRefresh;

#ifdef clearok
#undef clearok
#endif				/* clearok */
#define clearok(a,b) { FullRefresh = (BOOLEAN)b; }
    extern void LY_SLrefresh(void);

#ifdef refresh
#undef refresh
#endif				/* refresh */
#define refresh LY_SLrefresh

#ifdef VMS
    extern void VTHome(void);

#define endwin() LYclear(),refresh(),SLsmg_reset_smg(),VTHome()
#else
#define endwin SLsmg_reset_smg(),SLang_reset_tty
#endif				/* VMS */

#else				/* Define curses functions: */

#ifdef FANCY_CURSES
#define SHOW_WHEREIS_TARGETS 1

#ifdef VMS
/*
 *  For VMS curses, [w]setattr() and [w]clrattr()
 *  add and subtract, respectively, the attributes
 *  _UNDERLINE, _BOLD, _REVERSE, and _BLINK. - FM
 */
#define start_bold()		setattr(LYUnderlineLinks ? _UNDERLINE : _BOLD)
#define stop_bold()		clrattr(LYUnderlineLinks ? _UNDERLINE : _BOLD)
#define start_underline()	setattr(LYUnderlineLinks ? _BOLD : _UNDERLINE)
#define stop_underline()	clrattr(LYUnderlineLinks ? _BOLD : _UNDERLINE)
#define start_reverse()		setattr(_REVERSE)
#define wstart_reverse(w)	wsetattr(w, _REVERSE)
#define stop_reverse()		clrattr(_REVERSE)
#define wstop_reverse(w)	wclrattr(w, _REVERSE)

#else				/* Not VMS: */

    extern int string_to_attr(char *name);

/*
 *  For Unix FANCY_FANCY curses we interpose
 *  our own functions to add or subtract the
 *  A_foo attributes. - FM
 */
#if defined(USE_COLOR_TABLE) && !defined(USE_COLOR_STYLE)
    extern void LYaddWAttr(WINDOW * win, int a);
    extern void LYsubWAttr(WINDOW * win, int a);
    extern void LYaddWAttr(WINDOW * win, int a);
    extern void LYsubWAttr(WINDOW * win, int a);

#undef  standout
#define standout() 		lynx_standout(TRUE)
#undef  standend
#define standend() 		lynx_standout(FALSE)
#else
#define LYaddAttr(attr)		LYaddWAttr(LYwin,attr)
#define LYaddWAttr(win,attr)	wattron(win,attr)
#define LYsubAttr(attr)		LYsubWAttr(LYwin,attr)
#define LYsubWAttr(win,attr)	wattroff(win,attr)
#endif

#if defined(USE_COLOR_TABLE)
    extern void lynx_set_color(int a);
    extern void lynx_standout(int a);
    extern char *LYgetTableString(int code);
    extern int LYgetTableAttr(void);
    extern int lynx_chg_color(int, int, int);
#endif

#define start_bold()		LYaddAttr(LYUnderlineLinks ? A_UNDERLINE : A_BOLD)
#define stop_bold()		LYsubAttr(LYUnderlineLinks ? A_UNDERLINE : A_BOLD)
#define start_underline()	LYaddAttr(LYUnderlineLinks ? A_BOLD : A_UNDERLINE)
#define stop_underline()	LYsubAttr(LYUnderlineLinks ? A_BOLD : A_UNDERLINE)

#if defined(SNAKE) && defined(HP_TERMINAL)
#define start_reverse()		LYaddWAttr(LYwin, A_DIM)
#define wstart_reverse(w)	LYaddWAttr(w, A_DIM)
#define stop_reverse()		LYsubWAttr(LYwin, A_DIM)
#define wstop_reverse(w)	LYsubWAttr(w, A_DIM)
#else
#define start_reverse()		LYaddAttr(A_REVERSE)
#define wstart_reverse(w)	LYaddWAttr(w, A_REVERSE)
#define stop_reverse()		LYsubAttr(A_REVERSE)
#define wstop_reverse(w)	LYsubWAttr(w, A_REVERSE)
#endif				/* SNAKE && HP_TERMINAL */

#endif				/* VMS */

#else				/* Not FANCY_CURSES: */
/* *INDENT-OFF* */
#ifdef COLOR_CURSES
#undef COLOR_CURSES
Error FANCY_CURSES
There is a problem with the configuration.  We expect to have FANCY_CURSES
defined when COLOR_CURSES is defined, since we build on the attributes used in
FANCY_CURSES.  Check your config.log to see why the FANCY_CURSES test failed.
#endif
/* *INDENT-ON* */

/*
 *  We only have [w]standout() and [w]standin(),
 *  so we'll use them synonymously for bold and
 *  reverse, and ignore underline. - FM
 */
#define start_bold()		standout()
#define start_underline()	/* nothing */
#define start_reverse()		standout()
#define wstart_reverse(a)	wstandout(a)
#define stop_bold()		standend()
#define stop_underline()	/* nothing */
#define stop_reverse()		standend()
#define wstop_reverse(a)	wstandend(a)

#endif				/* FANCY_CURSES */

#ifdef __hpux			/* FIXME: configure check */
#undef ACS_UARROW
#undef ACS_DARROW
#undef ACS_LARROW
#undef ACS_RARROW
#undef ACS_BLOCK
#undef ACS_CKBOARD
#endif

#ifndef ACS_UARROW
#define ACS_UARROW  '^'
#endif

#ifndef ACS_DARROW
#define ACS_DARROW  'V'
#endif

#ifndef ACS_LARROW
#define ACS_LARROW '{'
#endif

#ifndef ACS_RARROW
#define ACS_RARROW '}'
#endif

#ifndef ACS_BLOCK
#define ACS_BLOCK  '}'
#endif

#ifndef ACS_CKBOARD
#define ACS_CKBOARD '}'
#endif

#define LYaddch(ch)		waddch(LYwin, ch)

#define addch_raw(ch)           LYaddch(ch)

#endif				/* USE_SLANG */

#ifdef USE_SLANG
#define LYGetYX(y, x)   y = SLsmg_get_row(), x = SLsmg_get_column()
#else
#ifdef getyx
#define LYGetYX(y, x)   getyx(LYwin, y, x)
#else
#define LYGetYX(y, x)   y = LYwin->_cury, x = LYwin->_curx
#endif				/* getyx */
#endif				/* USE_SLANG */

/*
 * If the screen library allows us to specify "default" color, allow user to
 * control it.
 */
#ifdef USE_DEFAULT_COLORS
#if defined(USE_SLANG) || defined(HAVE_ASSUME_DEFAULT_COLORS)
#define EXP_ASSUMED_COLOR 1
#endif
#endif

    extern void lynx_enable_mouse(int);
    extern void lynx_force_repaint(void);
    extern void lynx_nl2crlf(int normal);
    extern void lynx_start_title_color(void);
    extern void lynx_stop_title_color(void);
    extern void lynx_start_link_color(int flag, int pending);
    extern void lynx_stop_link_color(int flag, int pending);
    extern void lynx_stop_target_color(void);
    extern void lynx_start_target_color(void);
    extern void lynx_start_status_color(void);
    extern void lynx_stop_status_color(void);
    extern void lynx_start_h1_color(void);
    extern void lynx_stop_h1_color(void);
    extern void lynx_start_prompt_color(void);
    extern void lynx_stop_prompt_color(void);
    extern void lynx_start_radio_color(void);
    extern void lynx_stop_radio_color(void);
    extern void lynx_stop_all_colors(void);

    extern void lynx_start_bold(void);
    extern void lynx_start_reverse(void);
    extern void lynx_start_underline(void);
    extern void lynx_stop_bold(void);
    extern void lynx_stop_reverse(void);
    extern void lynx_stop_underline(void);

/*
 * To prevent corrupting binary data on DOS, MS-WINDOWS or OS/2 we open files
 * and stdout in BINARY mode by default.  Where necessary we should open and
 * (close!) TEXT mode.
 *
 * Note:  EMX has no corresponding variable like _fmode on DOS, but it does
 * have setmode.
 */
#if defined(_WINDOWS) || defined(DJGPP) || defined(__EMX__) || defined(WIN_EX)
#define SetOutputMode(mode) fflush(stdout), setmode(fileno(stdout), mode)
#else
#define SetOutputMode(mode)	/* nothing */
#endif

#if defined(_WINDOWS) || defined(DJGPP)
#define SetDefaultMode(mode) _fmode = mode
#else
#define SetDefaultMode(mode)	/* nothing */
#endif

/*
 * Very old versions of curses cannot put the cursor on the lower right corner.
 * Adjust our "hidden" cursor position accordingly.
 */
#if defined(FANCY_CURSES) || defined(USE_SLANG)
#define LYHideCursor() LYmove((LYlines - 1), (LYcolLimit - 1))
#else
#define LYHideCursor() LYmove((LYlines - 1), (LYcolLimit - 2))
#endif

    extern void LYstowCursor(WINDOW * win, int row, int col);

#ifdef __cplusplus
}
#endif
#endif				/* LYCURSES_H */
