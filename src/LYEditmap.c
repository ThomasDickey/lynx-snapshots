/* LYEditMap.c
   Keybindings for line and form editting.
*/

#include <HTUtils.h>
#include <LYGlobalDefs.h>
#include <LYStrings.h>
#include <LYKeymap.h>	/* KEYMAP_SIZE, LKC_*, LYK_* - kw */

/* * * * *  LynxEditactionCodes  * * * * */
#ifdef EXP_ALT_BINDINGS

/*  Last valid index for the (lynxkeycode+modifier -> lynxeditactioncode)
 *  tables.  Currently all three tables are the same. - kw
 */
#define LAST_MOD1_LKC	0x111
#define LAST_MOD2_LKC	0x111
#define LAST_MOD3_LKC	0x111

/*  Get (lynxkeycode+modifier -> lynxeditactioncode) mapping, intermediate.
 */
#define LKC_TO_LEC_M1(c) ((c)>LAST_MOD1_LKC? LYE_UNMOD: Mod1Binding[c])
#define LKC_TO_LEC_M2(c) ((c)>LAST_MOD2_LKC? LYE_UNMOD: Mod2Binding[c])
#define LKC_TO_LEC_M3(c) ((c)>LAST_MOD3_LKC? LYE_UNMOD: Mod3Binding[c])

#endif  /* EXP_ALT_BINDINGS */

PUBLIC int current_lineedit = 0;  /* Index into LYLineEditors[]   */

/*
 * See LYStrings.h for the LYE definitions.
 */
PRIVATE char DefaultEditBinding[KEYMAP_SIZE-1]={

LYE_NOP,        LYE_BOL,        LYE_DELPW,      LYE_ABORT,
/* nul          ^A              ^B              ^C      */

LYE_DELN,       LYE_EOL,        LYE_DELNW,      LYE_ABORT,
/* ^D           ^E              ^F              ^G      */

LYE_DELP,       LYE_TAB,      LYE_ENTER,      LYE_LOWER,
/* bs           tab             nl              ^K      */

LYE_NOP,        LYE_ENTER,      LYE_FORWW,      LYE_ABORT,
/* ^L           cr              ^N              ^O      */

LYE_BACKW,      LYE_NOP,        LYE_DELN,       LYE_NOP,
/* ^P           XON             ^R              XOFF    */

#ifdef WIN_EX
LYE_UPPER,      LYE_ERASE,      LYE_LKCMD,      LYE_PASTE,
#else
LYE_UPPER,      LYE_ERASE,      LYE_LKCMD,      LYE_NOP,
#endif
/* ^T           ^U              ^V              ^W      */

LYE_ERASE,      LYE_NOP,        LYE_NOP,        LYE_NOP,
/* ^X           ^Y              ^Z              ESC     */

LYE_NOP,        LYE_NOP,        LYE_SWMAP,      LYE_DELEL,
/* ^\           ^]              ^^              ^_      */

/* sp .. RUBOUT                                         */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_DELP,

/* 80..9F ISO-8859-1 8-bit escape characters. */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
#ifdef CJK_EX	/* 1997/11/03 (Mon) 20:30:54 */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
#else
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_AIX,
/*                                               97 AIX    */
#endif
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,

/* A0..FF (permissible ISO-8859-1) 8-bit characters. */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,

/* 100..10F function key definitions in LYStrings.h */
LYE_FORM_PASS,  LYE_FORM_PASS,  LYE_FORW,       LYE_BACK,
/* UPARROW      DNARROW         RTARROW         LTARROW     */

LYE_FORM_PASS,  LYE_FORM_PASS,  LYE_BOL,        LYE_EOL,
/* PGDOWN       PGUP            HOME            END         */

#if (defined(_WINDOWS) || defined(__DJGPP__))

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* F1 */

#else

LYE_NOP,        LYE_TAB,        LYE_BOL,        LYE_EOL,
/* F1           Do key          Find key        Select key  */

#endif /* _WINDOWS || __DJGPP__ */

LYE_NOP,        LYE_DELP,       LYE_NOP,        LYE_FORM_PASS,
/* Insert key   Remove key      DO_NOTHING      Back tab */

/* 110..18F */
#if (defined(_WINDOWS) || defined(__DJGPP__)) && defined(USE_SLANG) && !defined(DJGPP_KEYHANDLER)

LYE_DELP,       LYE_ENTER,      LYE_NOP,        LYE_NOP,
/* Backspace    Enter */

#else

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,

#endif /* USE_SLANG &&(_WINDOWS || __DJGPP) && !DJGPP_KEYHANDLER */

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/*             MOUSE_KEY  */
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* 190..20F */

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* 210..28F */

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* 290..293 */
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
};

/*
 * Add your favorite key bindings HERE
 */

/* KED-01 */ /* Default except: ^B=cursor-backward,  ^F=cursor-forward,   */
             /*                 ^K=delete-to-EOL,    ^X=delete-to-BOL,    */
             /*                 ^R=delete-prev-word, ^T=delete-next-word, */
             /*                 ^^=upper-case-line,  ^_=lower-case-line   */
/* Why the difference for tab? - kw */

#ifdef EXP_ALT_BINDINGS
PRIVATE char BetterEditBinding[KEYMAP_SIZE-1]={

LYE_NOP,        LYE_BOL,        LYE_BACK,       LYE_ABORT,
/* nul          ^A              ^B              ^C      */

LYE_DELN,       LYE_EOL,        LYE_FORW,       LYE_ABORT,
/* ^D           ^E              ^F              ^G      */

LYE_DELP,       LYE_ENTER,      LYE_ENTER,      LYE_DELNW, /* LYE_DELEL,*/
/* bs           tab             nl              ^K      */

LYE_NOP,        LYE_ENTER,      LYE_FORWW,      LYE_ABORT,
/* ^L           cr              ^N              ^O      */

LYE_BACKW,      LYE_NOP,        LYE_DELPW,      LYE_NOP,
/* ^P           XON             ^R              XOFF    */

#ifdef WIN_EX
LYE_DELNW,      LYE_ERASE,      LYE_LKCMD,      LYE_PASTE,
#else
LYE_DELNW,      LYE_ERASE,      LYE_LKCMD,      LYE_NOP,
#endif
/* ^T           ^U              ^V              ^W      */

LYE_DELBL,      LYE_NOP,        LYE_NOP,        LYE_NOP,
/* ^X           ^Y              ^Z              ESC     */

LYE_NOP,        LYE_NOP,        LYE_UPPER,      LYE_LOWER,
/* ^\           ^]              ^^              ^_      */

/* sp .. RUBOUT                                         */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_DELP,

/* 80..9F ISO-8859-1 8-bit escape characters. */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
#ifdef CJK_EX	/* 1997/11/03 (Mon) 20:30:54 */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
#else
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_AIX,
/*                                               97 AIX    */
#endif
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,

/* A0..FF (permissible ISO-8859-1) 8-bit characters. */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,

/* 100..10F function key definitions in LYStrings.h */
LYE_FORM_PASS,  LYE_FORM_PASS,  LYE_FORW,       LYE_BACK,
/* UPARROW      DNARROW         RTARROW         LTARROW     */

LYE_FORM_PASS,  LYE_FORM_PASS,  LYE_BOL,        LYE_EOL,
/* PGDOWN       PGUP            HOME            END         */

#if (defined(_WINDOWS) || defined(__DJGPP__))

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* F1 */

#else

LYE_NOP,        LYE_TAB,        LYE_BOL,        LYE_EOL,
/* F1           Do key          Find key        Select key  */

#endif /* _WINDOWS || __DJGPP__ */

LYE_NOP,        LYE_DELP,       LYE_NOP,        LYE_FORM_PASS,
/* Insert key   Remove key      DO_NOTHING      Back tab */

/* 110..18F */
#if (defined(_WINDOWS) || defined(__DJGPP__)) && defined(USE_SLANG) && !defined(DJGPP_KEYHANDLER)

LYE_DELP,       LYE_ENTER,      LYE_NOP,        LYE_NOP,
/* Backspace    Enter */

#else

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,

#endif /* USE_SLANG &&(_WINDOWS || __DJGPP) && !DJGPP_KEYHANDLER */

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/*             MOUSE_KEY  */
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* 190..20F */

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* 210..28F */

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* 290..293 */
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
};

/* kw     */ /* Default except: ^B=cursor-backward[+],^F=cursor-forward[+], */
             /*                 ^K=delete-to-EOL[+][++],^X=Modifier Prefix, */
             /*                 ^[ (ESC)=Modifier Prefix,                   */
             /*                 ^R=BACKW,             ^S=FORWW,             */
             /*                 ^T=transpose-chars,                         */
             /*                 ^U=delete-to-BOL,     ^W=delete-prev-word,  */
             /*                 ^@ (NUL)=SETMARK,     ^Y=YANK,              */
             /*                 ^_=ABORT (undo),                            */
             /*                 ^P=FORM_PASS,         ^N=FORM_PASS,         */
             /*                 ^O=FORM_PASS,         ^L=FORM_PASS,         */
             /*                 ^\=FORM_PASS,         ^]=FORM_PASS,         */
             /*                 ^Z=FORM_PASS,         F1=FORM_PASS,         */
             /*                 ^E=EOL[++],           Remove=DELN           */
             /* [+]: same as BetterEditBinding                              */
             /* [++]: additionally set double-key modifier                  */

/* Default where BetterEditBinding deviates:          ^^=SWMAP,            */
             /*                tab=LYE_TAB                                 */

/* Some functions for which the modifier binding is preferred:             */
             /*         M-bs,M-del=delete-prev-word, M-d=delete-next-word, */
             /*                M-b=BACKW,            M-f=FORWW,            */

PRIVATE char BashlikeEditBinding[KEYMAP_SIZE-1]={

LYE_SETMARK,    LYE_BOL,        LYE_BACK,       LYE_ABORT,
/* nul          ^A              ^B              ^C      */

LYE_DELN,       LYE_EOL|LYE_DF, LYE_FORW,       LYE_ABORT,
/* ^D           ^E              ^F              ^G      */

LYE_DELP,       LYE_TAB,        LYE_ENTER,      LYE_DELEL|LYE_DF,
/* bs           tab             nl              ^K      */

LYE_FORM_PASS,  LYE_ENTER,      LYE_FORM_PASS,  LYE_FORM_PASS,
/* ^L           cr              ^N              ^O      */

LYE_FORM_PASS,  LYE_NOP,        LYE_BACKW,      LYE_FORWW,
/* ^P           XON             ^R              ^S/XOFF */

LYE_TPOS,       LYE_DELBL,      LYE_LKCMD,      LYE_DELPW,
/* ^T           ^U              ^V              ^W      */

LYE_SETM1,      LYE_YANK,       LYE_FORM_PASS,  LYE_SETM2,
/* ^X           ^Y              ^Z              ESC     */

LYE_FORM_PASS,  LYE_FORM_PASS,  LYE_SWMAP,      LYE_ABORT,
/* ^\           ^]              ^^              ^_      */

/* sp .. RUBOUT                                         */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_DELP,

/* 80..9F ISO-8859-1 8-bit escape characters. */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_AIX,
/*                                               97 AIX    */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,

/* A0..FF (permissible ISO-8859-1) 8-bit characters. */
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,
LYE_CHAR,       LYE_CHAR,       LYE_CHAR,       LYE_CHAR,

/* 100..10F function key definitions in LYStrings.h */
LYE_FORM_PASS,  LYE_FORM_PASS,  LYE_FORW,       LYE_BACK,
/* UPARROW      DNARROW         RTARROW         LTARROW     */

LYE_FORM_PASS,  LYE_FORM_PASS,  LYE_BOL,        LYE_EOL,
/* PGDOWN       PGUP            HOME            END         */

#if (defined(_WINDOWS) || defined(__DJGPP__))

LYE_FORM_PASS,  LYE_NOP,        LYE_NOP,        LYE_NOP,
/* F1 */

#else

LYE_FORM_PASS,  LYE_TAB,        LYE_BOL,        LYE_EOL,
/* F1           Do key          Find key        Select key  */

#endif /* _WINDOWS || __DJGPP__ */

LYE_NOP,        LYE_DELN,       LYE_NOP,        LYE_FORM_PASS,
/* Insert key   Remove key      DO_NOTHING      Back tab */

/* 110..18F */
#if (defined(_WINDOWS) || defined(__DJGPP__)) && defined(USE_SLANG) && !defined(DJGPP_KEYHANDLER)

LYE_DELP,       LYE_ENTER,      LYE_NOP,        LYE_NOP,
/* Backspace    Enter */

#else

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,

#endif /* USE_SLANG &&(_WINDOWS || __DJGPP) && !DJGPP_KEYHANDLER */

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/*             MOUSE_KEY  */
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* 190..20F */

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* 210..28F */

LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
/* 290..293 */
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
};

/*  Oh no, not another one of those tables...
 *
 *  If modifier bit is set in a lynxkeycode, it is first looked up
 *  here.  Note the type different from the previous tables (short
 *  vs. char), since we want to hold larger values.  OTOH we can
 *  keep the size shorter, everything beyond the end is effectively
 *  LYE_UNMOD (ignore modifier) by virtue of the LKC_TO_LEC_M1
 *  macro.
 *
 *  Currently this table isn't specific to the current_lineedit value,
 *  it is shared by all alternative "Bindings" to save space.
 *  However, if the modifier flag is set only by a LYE_SETMn
 *  lynxeditaction, this table can have effect only for those Bindings
 *  that map a lynxkeycode to LYE_SETMn.  ( This doesn't apply if
 *  the modifier is already being set in LYgetch(). ) - kw
 */
PRIVATE short Mod1Binding[LAST_MOD1_LKC+1]={

LYE_NOP,        LYE_BOL,        LYE_BACKW,      LYE_UNMOD,
/* nul          ^A              ^B              ^C      */

LYE_FORM_LAC|LYK_NEXT_LINK,
                LYE_FORM_LAC|LYK_EDIT_TEXTAREA,
                                LYE_FORWW,      LYE_ABORT,
/* ^D           ^E              ^F              ^G      */

LYE_DELPW,      LYE_UNMOD,      LYE_ENTER,     LYE_FORM_LAC|LYK_LPOS_NEXT_LINK,
/* bs           tab             nl              ^K      */

LYE_FORM_PASS,  LYE_ENTER,      LYE_FORWW,      LYE_UNMOD,
/* ^L           cr              ^N              ^O      */

LYE_BACKW,      LYE_NOP,        LYE_BACKW,      LYE_NOP,
/* ^P           XON             ^R              ^S/XOFF */

LYE_NOP,        LYE_FORM_PASS,  LYE_NOP,        LYE_KILLREG,
/* ^T           ^U              ^V              ^W      */

LYE_XPMARK,     LYE_UNMOD,      LYE_FORM_PASS,  LYE_NOP,
/* ^X           ^Y              ^Z              ESC     */

LYE_FORM_PASS,  LYE_FORM_PASS,  LYE_UNMOD,      LYE_NOP,
/* ^\           ^]              ^^              ^_      */

/* sp .. ?                                              */
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_FORM_PASS,

LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,

/* @, A .. Z, [, \, ], ^, _                             */
LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,
LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,
LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,
LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,
LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,
LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,
LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,
LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,     LYE_C1CHAR,

/* `, a .. z, {, |, }, ~, RUBOUT                        */
LYE_UNMOD,      LYE_BOL,        LYE_BACKW,      LYE_UNMOD,
LYE_DELNW,      LYE_FORM_LAC|LYK_EDIT_TEXTAREA,
                                LYE_FORWW,      LYE_FORM_LAC|LYK_GROW_TEXTAREA,
LYE_CHAR,       LYE_FORM_LAC|LYK_INSERT_FILE,
                                LYE_CHAR,       LYE_ERASE,
LYE_LOWER,      LYE_CHAR,       LYE_FORM_PASS,  LYE_UNMOD,
LYE_CHAR,       LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_UPPER,      LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_DELPW,

/* 80..9F ISO-8859-1 8-bit escape characters. */
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,
LYE_NOP,        LYE_NOP,        LYE_NOP,        LYE_NOP,

/* A0..FF (permissible ISO-8859-1) 8-bit characters. */
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,

/* 100..10F function key definitions in LYStrings.h */
LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
/* UPARROW      DNARROW         RTARROW         LTARROW     */

LYE_UNMOD,      LYE_UNMOD,      LYE_FORM_PASS,  LYE_FORM_PASS,
/* PGDOWN       PGUP            HOME            END         */

#if (defined(_WINDOWS) || defined(__DJGPP__))

LYE_FORM_LAC|LYK_DWIMHELP,
                LYE_UNMOD,      LYE_UNMOD,      LYE_UNMOD,
/* F1 */

#else

LYE_FORM_LAC|LYK_DWIMHELP,
                LYE_UNMOD,  LYE_FORM_LAC|LYK_WHEREIS, LYE_FORM_LAC|LYK_NEXT,
/* F1           Do key          Find key        Select key  */

#endif /* _WINDOWS || __DJGPP__ */

LYE_UNMOD,      LYE_NOP,        LYE_UNMOD,      LYE_UNMOD,
/* Insert key   Remove key      DO_NOTHING      Back tab */

/* 110..18F */
#if (defined(_WINDOWS) || defined(__DJGPP__)) && defined(USE_SLANG) && !defined(DJGPP_KEYHANDLER)

LYE_DELPW,      LYE_UNMOD,
/* Backspace    Enter */

#else

LYE_UNMOD,      LYE_UNMOD,

#endif /* USE_SLANG &&(_WINDOWS || __DJGPP) && !DJGPP_KEYHANDLER */
};

/*  Two more tables here, but currently they are all the same.
    In other words, we are cheating to save space, until there
    is a need for different tables. - kw */
PRIVATE short *Mod2Binding = Mod1Binding;
PRIVATE short *Mod3Binding = Mod1Binding;

#endif /* EXP_ALT_BINDINGS */


/*
 * Add the array name to LYLineEditors
 */

PUBLIC char * LYLineEditors[]={
        DefaultEditBinding,     /* You can't please everyone, so you ... DW */
#ifdef EXP_ALT_BINDINGS
	BetterEditBinding,      /* No, you certainly can't ... /ked 10/27/98*/
	BashlikeEditBinding,      /* and one more... - kw 1999-02-15 */
#endif
};

/*
 * Add the name that the user will see below.
 * The order of LYLineEditors and LYLineditNames MUST be the same.
 */
PUBLIC char * LYLineeditNames[]={
	"Default Binding",
#ifdef EXP_ALT_BINDINGS
	"Alternate Bindings",
	"Bash-like Bindings",
#endif
	(char *) 0
};

/*
 * Add the URL (relative to helpfilepath) used for context-dependent
 * help on form field editing.
 *
 * The order must correspond to that of LYLineditNames.
 */
PUBLIC CONST char * LYLineeditHelpURLs[]={
	EDIT_HELP,
#ifdef EXP_ALT_BINDINGS
	ALT_EDIT_HELP,
	BASHLIKE_EDIT_HELP,
#endif
	(char *) 0
};

PUBLIC int EditBinding ARGS1(
    int,	xlkc)
{
    int editaction, xleac = LYE_UNMOD;
    int c = xlkc & LKC_MASK;

    if (xlkc == -1)
	return LYE_NOP;	/* maybe LYE_ABORT? or LYE_FORM_LAC|LYK_UNKNOWN? */
#ifdef NOT_ASCII
    if (c < 256) {
	c = TOASCII(c);
    }
#endif
#ifdef EXP_ALT_BINDINGS
    /*
     *  Get intermediate code from one of the lynxkeycode+modifier
     *  tables if applicable, otherwise get the lynxeditactioncode
     *  directly.
     *  If we have more than one modifier bits, the first currently
     *  wins. - kw
     */
    if (xlkc & LKC_MOD1) {
	xleac = LKC_TO_LEC_M1(c);
    } else if (xlkc & LKC_MOD2) {
	xleac = LKC_TO_LEC_M2(c);
    } else if (xlkc & LKC_MOD3) {
	xleac = LKC_TO_LEC_M3(c);
    } else {
	xleac = (unsigned char)LYLineEditors[current_lineedit][c];
    }
#endif
    /*
     *  If we have an intermediate code that says "same as without
     *  modifier", look that up now; otherwise we are already done. - kw
     */
    if (xleac == LYE_UNMOD) {
	editaction = LYLineEditors[current_lineedit][c];
    } else {
	editaction = xleac;
    }
    return editaction;
}

/*
 *  Install lec as the lynxeditaction for lynxkeycode xlkc.
 *  func must be present in the revmap table.
 *  returns TRUE if the mapping was made, FALSE if not.
 *  Note that this remapping cannot be undone (as might be desirable
 *  as a result of re-parsing lynx.cfg), we don't remember the
 *  original editaction from the Bindings tables anywhere. - kw
 */
PUBLIC int LYRemapEditBinding ARGS2(
    int,	xlkc,
    int,	lec)
{
    int j;
    int c = xlkc & LKC_MASK;
    if (xlkc < 0 || (xlkc&LKC_ISLAC) || c >= KEYMAP_SIZE + 1)
	return FALSE;
#ifdef EXP_ALT_BINDINGS
    if (xlkc & LKC_MOD1) {
	if (c > LAST_MOD1_LKC)
	    return FALSE;
	else
	    Mod1Binding[c] = (short) lec;
    } else if (xlkc & LKC_MOD2) {
	if (c > LAST_MOD2_LKC)
	    return FALSE;
	else
	    Mod2Binding[c] = (short) lec;
    } else if (xlkc & LKC_MOD3) {
	if (c > LAST_MOD3_LKC)
	    return FALSE;
	else
	    Mod3Binding[c] = (short) lec;
    } else
#endif /* EXP_ALT_BINDINGS */
    {
#ifndef UCHAR_MAX
#define UCHAR_MAX 255
#endif
	if ((unsigned int)lec > UCHAR_MAX)
	    return FALSE;	/* cannot do, doesn't fit in a char - kw */
	for (j = 0; LYLineeditNames[j]; j++) {
	    LYLineEditors[j][c] = (char) lec;
	}
    }
    return TRUE;
}

/*
 * Dummy initializer to ensure this module is linked
 * if the external model is common block, and the
 * module is ever placed in a library. - FM
 */
PUBLIC int LYEditmapDeclared NOPARAMS
{
    int status = 1;

    return status;
}

