#ifndef LYSTRINGS_H
#define LYSTRINGS_H

#include <LYCurses.h>

/*  UPPER8(ch1,ch2) is an extension of (TOUPPER(ch1) - TOUPPER(ch2))  */
extern int UPPER8  PARAMS((
	int		ch1,
	int		ch2));

extern int get_mouse_link NOPARAMS;
extern int peek_mouse_link NOPARAMS;
extern int peek_mouse_levent NOPARAMS;
extern int fancy_mouse PARAMS((WINDOW *win, int row, int *position));

extern char * LYstrncpy PARAMS((
	char *		dst,
	CONST char *	src,
	int		n));
extern void ena_csi PARAMS((BOOLEAN flag));
extern int LYgetch NOPARAMS;
extern int LYgetch_for PARAMS((
	int		code));
extern int LYgetstr PARAMS((
	char *		inputline,
	int		hidden,
	size_t		bufsize,
	int		recall));
extern char *LYstrsep PARAMS((
	char **		stringp,
	CONST char *	delim));
extern char * LYstrstr PARAMS((
	char *		chptr,
	CONST char *	tarptr));
extern char * LYmbcsstrncpy PARAMS((
	char *		dst,
	CONST char *	src,
	int		n_bytes,
	int		n_glyphs,
	BOOL		utf_flag));
extern char * LYmbcs_skip_glyphs PARAMS((
	char *		data,
	int		n_glyphs,
	BOOL		utf_flag));
extern int LYmbcsstrlen PARAMS((
	char *		str,
	BOOL		utf_flag,
	BOOL		count_gcells));
extern char * LYno_attr_mbcs_strstr PARAMS((
	char *		chptr,
	CONST char *	tarptr,
	BOOL		utf_flag,
	BOOL		count_gcells,
	int *		nstartp,
	int *		nendp));
extern char * LYno_attr_mbcs_case_strstr PARAMS((
	char *		chptr,
	CONST char *	tarptr,
	BOOL		utf_flag,
	BOOL		count_gcells,
	int *		nstartp,
	int *		nendp));
extern char * LYno_attr_char_strstr PARAMS((
	char *		chptr,
	char *		tarptr));
extern char * LYno_attr_char_case_strstr PARAMS((
	char *		chptr,
	char *		tarptr));

extern char * SNACopy PARAMS((
	char **		dest,
	CONST char *	src,
	int		n));
extern char * SNACat PARAMS((
	char **		dest,
	CONST char *	src,
	int		n));
#define StrnAllocCopy(dest, src, n)  SNACopy (&(dest), src, n)
#define StrnAllocCat(dest, src, n)   SNACat  (&(dest), src, n)

extern char *LYSafeGets PARAMS((char ** src, FILE * fp));

#ifdef EXP_FILE_UPLOAD
extern void base64_encode PARAMS((char * dest, char * src, int len));
#endif

/* values for LYgetch */
/* The following are lynxkeycodes, not to be confused with
   lynxactioncodes (LYK_*) to which they are often mapped.
   The lynxkeycodes include all single-byte keys as a subset. - kw
*/
#define UPARROW		256	/* 0x100 */
#define DNARROW		257	/* 0x101 */
#define RTARROW		258	/* 0x102 */
#define LTARROW		259	/* 0x103 */
#define PGDOWN		260	/* 0x104 */
#define PGUP		261	/* 0x105 */
#define HOME		262	/* 0x106 */
#define END_KEY		263	/* 0x107 */
#define F1		264	/* 0x108 */
#define DO_KEY		265	/* 0x109 */
#define FIND_KEY	266	/* 0x10A */
#define SELECT_KEY	267	/* 0x10B */
#define INSERT_KEY	268	/* 0x10C */
#define REMOVE_KEY	269	/* 0x10D */
#define DO_NOTHING	270	/* 0x10E */
#define BACKTAB_KEY	271	/* 0x10F */
#define MOUSE_KEY	0x11d	/* 0x11D */
/*  ***** NOTES: *****
    If you add definitions for new lynxkeycodes to the above list that
    need to be mapped to LYK_* lynxactioncodes -
    - AT LEAST the tables keymap[] and key_override[] in LYKeymap.c
      have to be changed/reviewed, AS WELL AS the lineedit binding
      tables in LYEditmap.c !
    - KEYMAP_SIZE, defined in LYKeymap.h, may need to be changed !
    - See also table funckey[] in LYKeymap.c for 'pretty' strings
      for the keys with codes >= 256 (to appear on the 'K'eymap page).
      New keycodes should probably be assigned consecutively, so their
      key names can be easily added to funckey[] (but see next point).
      They should also be documented in lynx.cfg.
    - The DOS port uses its own native codes for some keys, unless
      they are remapped by the code in LYgetch().  See *.key files
      in docs/ directory.  Adding new keys here may conflict with
      those codes (affecting DOS users), unless/until remapping is
      added or changed in LYgetch().
      (N)curses keypad codes (KEY_* from curses.h) can also directly
      appear as lynxkeycodes and conflict with our assignments, although
      that shouldn't happen - the useful ones should be recognized in
      LYgetch().
    - The actual recognition of raw input keys or escape sequences, and
      mapping to our lynxkeycodes, take place in LYgetch() and/or its
      subsidiary functions and/or the curses/slang/etc. libraries.
    The basic lynxkeycodes can appear combined with various flags in
    higher-order bits as extended lynxkeycodes; see macros in LYKeymap.h.
    The range of possible basic values is therefore limited, they have
    to be less than LKC_ISLKC (even if KEYMAP_SIZE is increased).
*/


#  define FOR_PANEL	0	/* normal screen, also LYgetch default */
#  define FOR_CHOICE	1	/* mouse menu */
#  define FOR_INPUT	2	/* form input and textarea field */
#  define FOR_PROMPT	3	/* string prompt editing */
#  define FOR_SINGLEKEY	4	/* single key prompt, confirmation */

#define VISIBLE  0
#define HIDDEN   1
#define NORECALL 0
#define RECALL   1

#ifdef EXP_ALT_BINDINGS
/*  Enable code implementing additional, mostly emacs-like, line-editing
    functions. - kw */
#define ENHANCED_LINEEDIT
#endif

/* EditFieldData preserves state between calls to LYEdit1
 */
typedef struct _EditFieldData {

        int  sx;        /* Origin of editfield                       */
        int  sy;
        int  dspwdth;   /* Screen real estate for editting           */

        int  strlen;    /* Current size of string.                   */
        int  maxlen;    /* Max size of string, excluding zero at end */
        char pad;       /* Right padding  typically ' ' or '_'       */
        BOOL hidden;    /* Masked password entry flag                */

        BOOL dirty;     /* accumulate refresh requests               */
        BOOL panon;     /* Need horizontal scroll indicator          */
        int  xpan;      /* Horizontal scroll offset                  */
        int  pos;       /* Insertion point in string                 */
        int  margin;    /* Number of columns look-ahead/look-back    */
        int  current_modifiers; /* Modifiers for next input lynxkeycode */
#ifdef ENHANCED_LINEEDIT
	int  mark;	/* position of emacs-like mark */
#endif

        char buffer[1024]; /* String buffer                          */

} EditFieldData;

/* line-edit action encoding */

#define LYE_NOP 0		  /* Do Nothing            */
#define LYE_CHAR  (LYE_NOP   +1)  /* Insert printable char */
#define LYE_ENTER (LYE_CHAR  +1)  /* Input complete, return char/lynxkeycode */
#define LYE_TAB   (LYE_ENTER +1)  /* Input complete, return TAB  */
#define LYE_ABORT (LYE_TAB   +1)  /* Input cancelled       */

#define LYE_FORM_PASS (LYE_ABORT +1)  /* In form fields: input complete,
					 return char / lynxkeycode;
					 Elsewhere: Do Nothing */

#define LYE_DELN  (LYE_FORM_PASS +1)  /* Delete next/curr char */
#define LYE_DELC  (LYE_DELN)      /* Obsolete (DELC case was equiv to DELN) */
#define LYE_DELP  (LYE_DELN  +1)  /* Delete prev      char */
#define LYE_DELNW (LYE_DELP  +1)  /* Delete next word      */
#define LYE_DELPW (LYE_DELNW +1)  /* Delete prev word      */

#define LYE_ERASE (LYE_DELPW +1)  /* Erase the line        */

#define LYE_BOL   (LYE_ERASE +1)  /* Go to begin of line   */
#define LYE_EOL   (LYE_BOL   +1)  /* Go to end   of line   */
#define LYE_FORW  (LYE_EOL   +1)  /* Cursor forwards       */
#define LYE_BACK  (LYE_FORW  +1)  /* Cursor backwards      */
#define LYE_FORWW (LYE_BACK  +1)  /* Word forward          */
#define LYE_BACKW (LYE_FORWW +1)  /* Word back             */

#define LYE_LOWER (LYE_BACKW +1)  /* Lower case the line   */
#define LYE_UPPER (LYE_LOWER +1)  /* Upper case the line   */

#define LYE_LKCMD (LYE_UPPER +1)  /* Invoke command prompt */

#define LYE_AIX   (LYE_LKCMD +1)  /* Hex 97                */

#define LYE_DELBL (LYE_AIX   +1)  /* Delete back to BOL    */
#define LYE_DELEL (LYE_DELBL +1)  /* Delete thru EOL       */

#define LYE_SWMAP (LYE_DELEL +1)  /* Switch input keymap   */

#define LYE_TPOS  (LYE_SWMAP +1)  /* Transpose characters  */

#define LYE_SETM1 (LYE_TPOS  +1)  /* Set modifier 1 flag   */
#define LYE_SETM2 (LYE_SETM1 +1)  /* Set modifier 2 flag   */
#define LYE_UNMOD (LYE_SETM2 +1)  /* Fall back to no-modifier command */

#define LYE_C1CHAR  (LYE_UNMOD   +1)  /* Insert C1 char if printable */

#define LYE_SETMARK (LYE_C1CHAR  +1)  /* emacs-like set-mark-command */
#define LYE_XPMARK  (LYE_SETMARK +1)  /* emacs-like exchange-point-and-mark */
#define LYE_KILLREG (LYE_XPMARK  +1)  /* emacs-like kill-region */
#define LYE_YANK    (LYE_KILLREG +1)  /* emacs-like yank */
#if defined(WIN_EX)
#define LYE_PASTE (LYE_YANK +1)	  /* ClipBoard to Lynx	   */
#endif
/* All preceding values must be within 0x00..0x7f - kw */

/*  The following are meant to be bitwise or-ed:  */
#define LYE_DF       0x80         /* Flag to set modifier 3 AND do other
				     action */
#define LYE_FORM_LAC 0x1000       /* Flag to pass lynxactioncode given by
				     lower bits.  Doesn't fit in a char! */


#if defined(USE_KEYMAPS)
extern int lynx_initialize_keymaps NOPARAMS;
extern int map_string_to_keysym PARAMS((CONST char * src, int *lec));
#endif

extern void LYLowerCase PARAMS((
	char *		buffer));
extern void LYUpperCase PARAMS((
	char *		buffer));
extern void LYRemoveBlanks PARAMS((
	char *		buffer));
extern char * LYSkipBlanks PARAMS((
	char *		buffer));
extern char * LYSkipNonBlanks PARAMS((
	char *		buffer));
extern CONST char * LYSkipCBlanks PARAMS((
	CONST char *	buffer));
extern CONST char * LYSkipCNonBlanks PARAMS((
	CONST char *	buffer));
extern void LYTrimLeading PARAMS((
	char *		buffer));
extern void LYTrimTrailing PARAMS((
	char *		buffer));
extern BOOLEAN LYTrimStartfile PARAMS((
	char *		buffer));
extern void LYSetupEdit PARAMS((
	EditFieldData *	edit,
	char *		old,
	int		maxstr,
	int		maxdsp));
extern void LYRefreshEdit PARAMS((
	EditFieldData *	edit));
extern int EditBinding PARAMS((int ch));		   /* in LYEditmap.c */
extern BOOL LYRemapEditBinding PARAMS((
	int		xlkc,
	int		lec,
	int 		select_edi));			   /* in LYEditmap.c */
extern int LYKeyForEditAction PARAMS((int lec));	   /* in LYEditmap.c */
extern int LYEditKeyForAction PARAMS((int lac, int *pmodkey));/* LYEditmap.c */
extern int LYEdit1 PARAMS((
	EditFieldData *	edit,
	int		ch,
	int		action,
	BOOL		maxMessage));
extern void LYOpenCloset NOPARAMS;
extern void LYCloseCloset NOPARAMS;

extern int current_lineedit;
extern char * LYLineeditNames[];
extern char * LYLineEditors[];
extern CONST char * LYLineeditHelpURLs[];

extern CONST char * LYLineeditHelpURL NOPARAMS;

#if 0				/* NOT USED, use function instead - kw */
/* Push a character through the lineedit machinery */
#ifdef    NOT_ASCII  /* S/390 -- gil -- 2080 */
#define EditBinding(c) (LYLineEditors[current_lineedit][(c)<256 ? TOASCII(c) : c])
#else  /* NOT_ASCII */
#define EditBinding(c) (LYLineEditors[current_lineedit][c])
#endif /* NOT_ASCII */
#endif /* 0 */

#define LYLineEdit(e,c,m) LYEdit1(e,c,EditBinding(c)&~LYE_DF,m)

/* Dummy initializer for LYEditmap.c */
extern int LYEditmapDeclared NOPARAMS;

#endif /* LYSTRINGS_H */
