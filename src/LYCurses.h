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
#include "slang.h"
#include "slcurses.h"

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

#ifdef NCURSES
# ifndef NCURSESHEADER
#  include <ncurses/curses.h>
# else
#  ifdef __NetBSD__
#    include <ncurses/ncurses.h>
#  else
#    include <ncurses.h>
#  endif /* __NetBSD__ */
# endif /* NCURSESINCDIR */
#else
# ifdef ULTRIX
#  include <cursesX.h>  /* ultrix */
# else
#  if defined(SUN) || defined(sun) || defined(SGI) || defined(SCO) || defined(ISC) || defined(PTX2)
#   include "curses.h"
#  else
#   if defined(VMS) && defined(__GNUC__)
#    include "LYGCurses.h"
#   else
#    if defined(sony_news)
#     include "/usr/sony/include/jcurses.h"  /* sony_news */
#    else
#     include <curses.h>  /* everything else */
#    endif /* sony_news */
#   endif /* VMS && __GNUC__ */
#  endif /* SUN || sun || SGI || SCO || ISC || PTX2 */
# endif /* ULTRIX */
#endif /* NCURSES */
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


#ifdef USE_SLANG
extern void sl_add_attr PARAMS((int a));
extern void sl_sub_attr PARAMS((int a));
extern void lynx_setup_colors NOPARAMS;

#define start_bold()      sl_add_attr(1)
#define start_reverse()   sl_add_attr(2)
#define start_underline() sl_add_attr(4)
#define stop_bold()       sl_sub_attr(1)
#define stop_reverse()    sl_sub_attr(2)
#define stop_underline()  sl_sub_attr(4)

extern unsigned int Lynx_Color_Flags;
#define SL_LYNX_USE_COLOR	1
#define SL_LYNX_USE_BLINK	2

#ifdef FANCY_CURSES
#undef FANCY_CURSES
#endif /* FANCY_CURSES */
#ifndef NO_KEYPAD
#define NO_KEYPAD
#endif /* !NO_KEYPAD */
#ifndef NO_TTYTYPE
#define NO_TTYTYPE
#endif /* !NO_TTYTYPE */

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
#ifdef endwin
#undef endwin
#endif /*endwin */
#define endwin() clear(),refresh(),SLsmg_reset_smg(),VTHome()
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

#ifdef UNDERLINE_LINKS
#define start_bold()      attrset(A_UNDERLINE)
#define stop_bold()       attroff(A_UNDERLINE)
#define start_underline() attrset(A_BOLD)
#define stop_underline()  attroff(A_BOLD)
#else /* not UNDERLINE_LINKS: */
#define start_bold()      attrset(A_BOLD)
#define stop_bold()       attroff(A_BOLD)
#define start_underline() attrset(A_UNDERLINE)
#define stop_underline()  attroff(A_UNDERLINE)
#endif /* UNDERLINE_LINKS */
#if defined(SNAKE) && defined(HP_TERMINAL)
#define start_reverse()   wattrset(stdscr,A_DIM)
#define wstart_reverse(a) wattrset(a,A_DIM)
#define stop_reverse()    wattroff(stdscr,A_DIM)
#define wstop_reverse(a)  wattroff(a,A_DIM)
#else
#define start_reverse()   attrset(A_REVERSE)
#define wstart_reverse(a) wattrset(a,A_REVERSE)
#define stop_reverse()    attroff(A_REVERSE)
#define wstop_reverse(a)  wattroff(a,A_REVERSE)
#endif /* SNAKE && HP_TERMINAL */
#endif /* VMS */

#else /* Not FANCY_CURSES: */

#define start_bold()      standout()  
#define start_underline() 1  /* nothing */
#define start_reverse()   standout()
#define wstart_reverse(a)   wstandout(a)
#define stop_bold()       standend()  
#define stop_underline()  1  /* nothing */
#define stop_reverse()    standend()
#define wstop_reverse(a)    wstandend(a)

#endif /* FANCY_CURSES */
#endif /* USE_SLANG */

#endif /* LYCURSES_H */
