#ifndef LYCURSES_H
#define LYCURSES_H

#ifdef TRUE
#undef TRUE  /* to prevent parse error :( */
#endif /* TRUE */
#ifdef FALSE
#undef FALSE  /* to prevent parse error :( */
#endif /* FALSE */

#ifdef USE_SLANG
#if defined(UNIX) && !defined(unix)
#define unix
#endif /* UNIX && !unix */
#include <slang.h>

#else /* Using curses: */

#ifdef VMS
#define FANCY_CURSES
#endif

/*
 *	CR may be defined before the curses.h include occurs.
 *	There is a conflict between the termcap char *CR and the define.
 *	Assuming that the definition of CR will always be carriage return.
 *	06-09-94 Lynx 2-3-1 Garrett Arch Blythe
 */
#ifdef CR
#undef CR  /* to prevent parse error :( */
#define REDEFINE_CR
#endif /* CR */

#ifdef DOSPATH
#   include "curses.h"
#else
#ifdef NCURSES
# ifdef HAVE_CONFIG_H
# ifdef NCURSESHEADER
#  include <ncurses.h>
# else
#  include <curses.h>
# endif
extern void LYsubwindow PARAMS((WINDOW * param));

# else /* FIXME: remove this after configure script is complete */
# ifndef NCURSESHEADER
#  include <ncurses/curses.h>
# else
#  ifdef __NetBSD__
#    include <ncurses/ncurses.h>
#  else
#    include <ncurses.h>
#  endif /* __NetBSD__ */
# endif /* HAVE_CONFIG_H */
# endif /* NCURSESHEADER */

#else
# ifdef ULTRIX
#  include <cursesX.h>  /* ultrix */
# else
#  if defined(VMS) && defined(__GNUC__)
#   include "LYGCurses.h"
#  else
#   if defined(sony_news)
#    include "/usr/sony/include/jcurses.h"  /* sony_news */
#   else
#    include <curses.h>  /* everything else */
#   endif /* sony_news */
#  endif /* VMS && __GNUC__ */
# endif /* ULTRIX */
#endif /* NCURSES */
#endif /* DOSPATH */
#endif /* USE_SLANG */


/* Both slang and curses: */
#ifndef TRUE
#define TRUE  1
#endif /* !TRUE */
#ifndef FALSE
#define FALSE 0
#endif /* !FALSE */

#ifdef REDEFINE_CR
#define CR FROMASCII('\015')
#endif /* REDEFINE_CR */

extern int LYlines;  /* replaces LINES */
extern int LYcols;   /* replaces COLS */

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif /* HTUTILS_H */

extern void start_curses NOPARAMS;
extern void stop_curses NOPARAMS;
extern BOOLEAN setup PARAMS((char *terminal));

#ifdef VMS
extern void VMSexit();
extern int ttopen();
extern int ttclose();
extern int ttgetc();
extern void *VMSsignal PARAMS((int sig, void (*func)()));
#ifndef USE_SLANG
extern void VMSbox PARAMS((WINDOW *win, int height, int width));
#endif /* !USE_SLANG */
#endif /* VMS */

#if defined(USE_SLANG) || defined(COLOR_CURSES)
extern void lynx_add_attr PARAMS((int a));
extern void lynx_sub_attr PARAMS((int a));
extern void lynx_setup_colors NOPARAMS;
extern unsigned int Lynx_Color_Flags;
#endif

#ifdef USE_SLANG
#define SL_LYNX_USE_COLOR	1
#define SL_LYNX_USE_BLINK	2
#define start_bold()      lynx_add_attr(1)
#define start_reverse()   lynx_add_attr(2)
#define start_underline() lynx_add_attr(4)
#define stop_bold()       lynx_sub_attr(1)
#define stop_reverse()    lynx_sub_attr(2)
#define stop_underline()  lynx_sub_attr(4)

#ifdef FANCY_CURSES
#undef FANCY_CURSES
#endif /* FANCY_CURSES */

/* Map some curses functions to slang functions. */
#define stdscr NULL
#ifdef SLANG_MBCS_HACK
extern int PHYSICAL_SLtt_Screen_Cols;
#define COLS PHYSICAL_SLtt_Screen_Cols
#else
#define COLS SLtt_Screen_Cols
#endif /* SLANG_MBCS_HACK */
#define LINES SLtt_Screen_Rows
#define move SLsmg_gotorc
#define addstr SLsmg_write_string
#define clear SLsmg_cls
#define standout SLsmg_reverse_video
#define standend  SLsmg_normal_video
#define clrtoeol SLsmg_erase_eol
#define scrollok(a,b) SLsmg_Newline_Moves = ((b) ? 1 : -1)
#define addch SLsmg_write_char
#define echo()
#define printw SLsmg_printf
 
extern int curscr;
extern BOOLEAN FullRefresh;
#ifdef clearok
#undef clearok
#endif /* clearok */
#define clearok(a,b) { FullRefresh = (BOOLEAN)b; }
extern void LY_SLrefresh NOPARAMS;
#ifdef refresh
#undef refresh
#endif /* refresh */
#define refresh LY_SLrefresh

#ifdef VMS
extern void VTHome NOPARAMS;
#define endwin() clear(),refresh(),SLsmg_reset_smg(),VTHome()
#else
#define endwin SLsmg_reset_smg(),SLang_reset_tty
#endif /* VMS */

#else /* Define curses functions: */

#ifdef FANCY_CURSES

#ifdef VMS
#ifdef UNDERLINE_LINKS
#define start_bold()      setattr(_UNDERLINE)
#define stop_bold()       clrattr(_UNDERLINE)
#define start_underline() setattr(_BOLD)
#define stop_underline()  clrattr(_BOLD)
#else /* not UNDERLINE_LINKS */
#define start_bold()      setattr(_BOLD)
#define stop_bold()       clrattr(_BOLD)
#define start_underline() setattr(_UNDERLINE)
#define stop_underline()  clrattr(_UNDERLINE)
#endif /* UNDERLINE_LINKS */
#define start_reverse()   setattr(_REVERSE)
#define wstart_reverse(a)   wsetattr(a,_REVERSE)
#define wstop_underline(a)  wclrattr(a,_UNDERLINE)
#define stop_reverse()    clrattr(_REVERSE)
#define wstop_reverse(a)    wclrattr(a,_REVERSE)

#else /* NOT VMS: */

#ifdef COLOR_CURSES
extern void lynx_add_wattr PARAMS((WINDOW *, int));
extern void lynx_sub_wattr PARAMS((WINDOW *, int));
extern void lynx_set_color PARAMS((int));
extern void lynx_standout  PARAMS((int));
extern int  lynx_chg_color PARAMS((int, int, int));
#undef  standout
#define standout() lynx_standout(TRUE)
#undef  standend
#define standend() lynx_standout(FALSE)
#else
#define lynx_add_attr	attrset
#define lynx_add_wattr	wattrset
#define lynx_sub_attr	attroff
#define lynx_sub_wattr	wattroff
#endif

#ifdef UNDERLINE_LINKS
#define start_bold()      lynx_add_attr(A_UNDERLINE)
#define stop_bold()       lynx_sub_attr(A_UNDERLINE)
#define start_underline() lynx_add_attr(A_BOLD)
#define stop_underline()  lynx_sub_attr(A_BOLD)
#else /* not UNDERLINE_LINKS: */
#define start_bold()      lynx_add_attr(A_BOLD)
#define stop_bold()       lynx_sub_attr(A_BOLD)
#define start_underline() lynx_add_attr(A_UNDERLINE)
#define stop_underline()  lynx_sub_attr(A_UNDERLINE)
#endif /* UNDERLINE_LINKS */
#if defined(SNAKE) && defined(HP_TERMINAL)
#define start_reverse()   lynx_add_wattr(stdscr,A_DIM)
#define wstart_reverse(a) lynx_add_wattr(a,A_DIM)
#define stop_reverse()    lynx_sub_wattr(stdscr,A_DIM)
#define wstop_reverse(a)  lynx_sub_wattr(a,A_DIM)
#else
#define start_reverse()   lynx_add_attr(A_REVERSE)
#define wstart_reverse(a) lynx_add_wattr(a,A_REVERSE)
#define stop_reverse()    lynx_sub_attr(A_REVERSE)
#define wstop_reverse(a)  lynx_sub_wattr(a,A_REVERSE)
#endif /* SNAKE && HP_TERMINAL */
#endif /* VMS */

#else /* Not FANCY_CURSES: */

#define start_bold()      standout()  
#define start_underline() /* nothing */
#define start_reverse()   standout()
#define wstart_reverse(a)   wstandout(a)
#define stop_bold()       standend()  
#define stop_underline()  /* nothing */
#define stop_reverse()    standend()
#define wstop_reverse(a)    wstandend(a)

#endif /* FANCY_CURSES */
#endif /* USE_SLANG */

#endif /* LYCURSES_H */
