/* $LynxId: LYKeymap.c,v 1.125 2024/01/10 08:53:58 tom Exp $ */
#include <HTUtils.h>
#include <LYUtils.h>
#include <LYGlobalDefs.h>
#include <LYKeymap.h>
#include <LYCharSets.h>		/* for LYlowest_eightbit - kw */
#include <HTAccess.h>
#include <HTFormat.h>
#include <HTAlert.h>
#include <LYStrings.h>		/* for USE_KEYMAP stuff - kw */

#include <LYLeaks.h>

#ifdef EXP_KEYBOARD_LAYOUT
#include <jcuken_kb.h>
#include <yawerty_kb.h>
#include <rot13_kb.h>
#endif

#define PUTS(buf)    (*target->isa->put_block)(target, buf, (int) strlen(buf))

#ifdef EXP_KEYBOARD_LAYOUT
int current_layout = 0;		/* Index into LYKbLayouts[]   */

LYKbLayout_t *LYKbLayouts[] =
{
    kb_layout_rot13,
    kb_layout_jcuken,
    kb_layout_yawerty
};

const char *LYKbLayoutNames[] =
{
    "ROT13'd keyboard layout",
    "JCUKEN Cyrillic, for AT 101-key kbd",
    "YAWERTY Cyrillic, for DEC LK201 kbd",
    (char *) 0
};
#endif /* EXP_KEYBOARD_LAYOUT */

/* * * Tables mapping LynxKeyCodes to LynxActionCodes  * * */

/*
 * Lynxkeycodes include all single-byte keys as well as codes for function keys
 * and some special purposes.  See LYStrings.h.  Extended lynxkeycode values
 * can also contain flags for modifiers and other purposes, but here only the
 * base values are mapped to lynxactioncodes.  They are called `keystrokes' in
 * lynx.cfg.
 *
 * Lynxactioncodes (confusingly, constants are named LYK_foo and typed as
 * specify key `functions', see LYKeymap.h.
 */

/* the character gets 1 added to it before lookup,
 * so that EOF maps to 0
 */
LYKeymap_t keymap[KEYMAP_SIZE];

static const LYEditInit initKeymapData[] =
{
    {KTL('@'), LYK_DO_NOTHING},
    {KTL('A'), LYK_HOME},
    {KTL('B'), LYK_PREV_PAGE},
    {KTL('D'), LYK_ABORT},
    {KTL('E'), LYK_END},
    {KTL('F'), LYK_NEXT_PAGE},
    {KTL('H'), LYK_HISTORY},
    {KTL('I'), LYK_FASTFORW_LINK},
    {KTL('J'), LYK_ACTIVATE},
    {KTL('K'), LYK_COOKIE_JAR},
    {KTL('L'), LYK_REFRESH},
    {KTL('M'), LYK_ACTIVATE},
    {KTL('N'), LYK_DOWN_TWO},
    {KTL('P'), LYK_UP_TWO},
    {KTL('Q'), LYK_CHANGE_CENTER},
    {KTL('R'), LYK_RELOAD},
#ifdef CAN_CUT_AND_PASTE
    {KTL('S'), LYK_TO_CLIPBOARD},
#endif
    {KTL('T'), LYK_TRACE_TOGGLE},
    {KTL('U'), LYK_NEXT_DOC},
    {KTL('V'), LYK_SWITCH_DTD},
    {KTL('W'), LYK_REFRESH},
    {KTL('X'), LYK_CACHE_JAR},
    {KTL('Z'), LYK_MAXSCREEN_TOGGLE},
    {KHR(' '), LYK_NEXT_PAGE},
    {KHR('!'), LYK_SHELL},
    {KHR('"'), LYK_SOFT_DQUOTES},
    {KHR('#'), LYK_TOOLBAR},
    {KHR('$'), LYK_LAST_LINK},
    {KHR('\''), LYK_HISTORICAL},
    {KHR('('), LYK_UP_HALF},
    {KHR(')'), LYK_DOWN_HALF},
    {KHR('*'), LYK_IMAGE_TOGGLE},
    {KHR('+'), LYK_NEXT_PAGE},
    {KHR(','), LYK_EXTERN_PAGE},
    {KHR('-'), LYK_PREV_PAGE},
    {KHR('.'), LYK_EXTERN_LINK},
    {KHR('/'), LYK_WHEREIS},
    {KHR('0'), LYK_F_LINK_NUM},
    {KHR('1'), LYK_1},
    {KHR('2'), LYK_2},
    {KHR('3'), LYK_3},
    {KHR('4'), LYK_4},
    {KHR('5'), LYK_5},
    {KHR('6'), LYK_6},
    {KHR('7'), LYK_7},
    {KHR('8'), LYK_8},
    {KHR('9'), LYK_9},
    {KHR(':'), LYK_COMMAND},
    {KHR(';'), LYK_TRACE_LOG},
    {KHR('<'), LYK_UP_LINK},
    {KHR('='), LYK_INFO},
    {KHR('>'), LYK_DOWN_LINK},
    {KHR('?'), LYK_HELP},
    {KHR('@'), LYK_RAW_TOGGLE},
    {KHR('A'), LYK_ADDRLIST},
    {KHR('B'), LYK_PREV_PAGE},
#ifdef SUPPORT_CHDIR
    {KHR('C'), LYK_CHDIR},
#else
    {KHR('C'), LYK_COMMENT},
#endif
    {KHR('D'), LYK_DOWNLOAD},
    {KHR('E'), LYK_ELGOTO},
    {KHR('F'), LYK_DIRED_MENU},
    {KHR('G'), LYK_ECGOTO},
    {KHR('H'), LYK_HELP},
    {KHR('I'), LYK_INDEX},
#ifdef KANJI_CODE_OVERRIDE
    {KHR('J'), LYK_CHANGE_KCODE},
#else
    {KHR('J'), LYK_JUMP},
#endif
    {KHR('K'), LYK_KEYMAP},
    {KHR('L'), LYK_LIST},
    {KHR('M'), LYK_MAIN_MENU},
    {KHR('N'), LYK_PREV},
    {KHR('O'), LYK_OPTIONS},
    {KHR('P'), LYK_PRINT},
    {KHR('Q'), LYK_ABORT},
    {KHR('R'), LYK_DEL_BOOKMARK},
    {KHR('S'), LYK_INDEX_SEARCH},
    {KHR('T'), LYK_TAG_LINK},
    {KHR('U'), LYK_PREV_DOC},
    {KHR('V'), LYK_VLINKS},
    {KHR('X'), LYK_NOCACHE},
    {KHR('Z'), LYK_INTERRUPT},
    {KHR('['), LYK_INLINE_TOGGLE},
    {KHR('\\'), LYK_SOURCE},
    {KHR(']'), LYK_HEAD},
    {KHR('^'), LYK_FIRST_LINK},
    {KHR('_'), LYK_CLEAR_AUTH},
    {KHR('`'), LYK_MINIMAL},
    {KHR('a'), LYK_ADD_BOOKMARK},
    {KHR('b'), LYK_PREV_PAGE},
    {KHR('c'), LYK_COMMENT},
    {KHR('d'), LYK_DOWNLOAD},
    {KHR('e'), LYK_EDIT},
    {KHR('f'), LYK_DIRED_MENU},
    {KHR('g'), LYK_GOTO},
    {KHR('h'), LYK_HELP},
    {KHR('i'), LYK_INDEX},
    {KHR('j'), LYK_JUMP},
    {KHR('k'), LYK_KEYMAP},
    {KHR('l'), LYK_LIST},
    {KHR('m'), LYK_MAIN_MENU},
    {KHR('n'), LYK_NEXT},
    {KHR('o'), LYK_OPTIONS},
    {KHR('p'), LYK_PRINT},
    {KHR('q'), LYK_QUIT},
    {KHR('r'), LYK_DEL_BOOKMARK},
    {KHR('s'), LYK_INDEX_SEARCH},
    {KHR('t'), LYK_TAG_LINK},
    {KHR('u'), LYK_PREV_DOC},
    {KHR('v'), LYK_VIEW_BOOKMARK},
    {KHR('x'), LYK_NOCACHE},
    {KHR('z'), LYK_INTERRUPT},
    {KHR('{'), LYK_SHIFT_LEFT},
    {KHR('|'), LYK_LINEWRAP_TOGGLE},
    {KHR('}'), LYK_SHIFT_RIGHT},
    {KHR('~'), LYK_NESTED_TABLES},
    {KHR(DEL_KEY), LYK_HISTORY},
    {KHR(UPARROW_KEY), LYK_PREV_LINK},
    {KHR(DNARROW_KEY), LYK_NEXT_LINK},
    {KHR(RTARROW_KEY), LYK_ACTIVATE},
    {KHR(LTARROW_KEY), LYK_PREV_DOC},
    {KHR(PGDOWN_KEY), LYK_NEXT_PAGE},
    {KHR(PGUP_KEY), LYK_PREV_PAGE},
    {KHR(HOME_KEY), LYK_HOME},
    {KHR(END_KEY), LYK_END},
    {KHR(F1_KEY), LYK_DWIMHELP},
#if !(defined(_WINDOWS) || defined(__DJGPP__))
    {KHR(DO_KEY), LYK_ACTIVATE},
    {KHR(FIND_KEY), LYK_HOME},
    {KHR(SELECT_KEY), LYK_END},
#endif
    {KHR(INSERT_KEY), LYK_UP_TWO},
    {KHR(REMOVE_KEY), LYK_DOWN_TWO},
    {KHR(DO_NOTHING), LYK_DO_NOTHING},
    {KHR(BACKTAB_KEY), LYK_FASTBACKW_LINK},
    {KHR(F11_KEY), LYK_DO_NOTHING},
#ifdef DJGPP_KEYHANDLER
    {302, LYK_ABORT},
#endif				/* DJGPP_KEYHANDLER */
#if (defined(_WINDOWS) || defined(__DJGPP__) || defined(__CYGWIN__)) && !defined(USE_SLANG)	/* PDCurses */
    {441, LYK_ABORT},		/* ALT_X */
    {459, LYK_WHEREIS},		/* KP_SLASH */
    {464, LYK_IMAGE_TOGGLE},	/* KP_* */
    {465, LYK_PREV_PAGE},	/* KP_- */
    {466, LYK_NEXT_PAGE},	/* KP_+ */
#endif
    {657, LYK_CHANGE_LINK},
    {-1, LYE_UNKNOWN}
};

static LYEditConfig myKeymapData =
{
    "Key Map", initKeymapData, keymap
};

#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
/*
 * This table is used to override the standard keyboard assignments
 * when lynx_edit_mode is in effect and keyboard overrides have been
 * allowed at compile time.
 */

LYKeymap_t key_override[KEYMAP_SIZE];

#define EDIT_INIT(c,l) {KHR(c), (l)}
static const LYEditInit initOverrideData[] =
{
    EDIT_INIT(CTL('V'), LYK_NEXT_DOC),
    EDIT_INIT('.', LYK_TAG_LINK),
#ifndef SUPPORT_CHDIR
    EDIT_INIT('C', LYK_CREATE),
#else
    EDIT_INIT('C', LYK_CHDIR),
#endif
    EDIT_INIT('F', LYK_DIRED_MENU),
    EDIT_INIT('M', LYK_MODIFY),
    EDIT_INIT('R', LYK_REMOVE),
    EDIT_INIT('T', LYK_TAG_LINK),
    EDIT_INIT('U', LYK_UPLOAD),
    EDIT_INIT('c', LYK_CREATE),
    EDIT_INIT('f', LYK_DIRED_MENU),
    EDIT_INIT('m', LYK_MODIFY),
    EDIT_INIT('r', LYK_REMOVE),
    EDIT_INIT('t', LYK_TAG_LINK),
    EDIT_INIT('u', LYK_UPLOAD),
    EDIT_INIT(DO_NOTHING, LYK_DO_NOTHING),
    {-1, LYE_UNKNOWN}
};

static LYEditConfig myOverrideData =
{
    "Key Override", initOverrideData, key_override
};
#endif /* DIRED_SUPPORT && OK_OVERRIDE */

#define DATA(code, name, doc) { code, name, doc }
/* The order of this array must match the LYKeymapCode enum in LYKeymap.h */
static Kcmd revmap[] =
{
    DATA(
	    LYK_UNKNOWN, "UNMAPPED",
	    NULL),
    DATA(
	    LYK_COMMAND, "COMMAND",
	    "prompt for, execute a command"),
    DATA(
	    LYK_1, "1",
	    NULL),
    DATA(
	    LYK_2, "2",
	    NULL),
    DATA(
	    LYK_3, "3",
	    NULL),
    DATA(
	    LYK_4, "4",
	    NULL),
    DATA(
	    LYK_5, "5",
	    NULL),
    DATA(
	    LYK_6, "6",
	    NULL),
    DATA(
	    LYK_7, "7",
	    NULL),
    DATA(
	    LYK_8, "8",
	    NULL),
    DATA(
	    LYK_9, "9",
	    NULL),
    DATA(
	    LYK_SOURCE, "SOURCE",
	    "toggle source/presentation for current document"),
    DATA(
	    LYK_RELOAD, "RELOAD",
	    "reload the current document"),
    DATA(
	    LYK_QUIT, "QUIT",
	    "quit the browser"),
    DATA(
	    LYK_ABORT, "ABORT",
	    "quit the browser unconditionally"),
    DATA(
	    LYK_NEXT_PAGE, "NEXT_PAGE",
	    "view the next page of the document"),
    DATA(
	    LYK_PREV_PAGE, "PREV_PAGE",
	    "view the previous page of the document"),
    DATA(
	    LYK_UP_TWO, "UP_TWO",
	    "go back two lines in the document"),
    DATA(
	    LYK_DOWN_TWO, "DOWN_TWO",
	    "go forward two lines in the document"),
    DATA(
	    LYK_UP_HALF, "UP_HALF",
	    "go back half a page in the document"),
    DATA(
	    LYK_DOWN_HALF, "DOWN_HALF",
	    "go forward half a page in the document"),
    DATA(
	    LYK_REFRESH, "REFRESH",
	    "refresh the screen to clear garbled text"),
    DATA(
	    LYK_HOME, "HOME",
	    "go to the beginning of the current document"),
    DATA(
	    LYK_END, "END",
	    "go to the end of the current document"),
    DATA(
	    LYK_FIRST_LINK, "FIRST_LINK",
	    "make the first link on the line current"),
    DATA(
	    LYK_LAST_LINK, "LAST_LINK",
	    "make the last link on the line current"),
    DATA(
	    LYK_PREV_LINK, "PREV_LINK",
	    "make the previous link current"),
    DATA(
	    LYK_NEXT_LINK, "NEXT_LINK",
	    "make the next link current"),
    DATA(
	    LYK_LPOS_PREV_LINK, "LPOS_PREV_LINK",
	    "make previous link current, same column for input"),
    DATA(
	    LYK_LPOS_NEXT_LINK, "LPOS_NEXT_LINK",
	    "make next link current, same column for input"),
    DATA(
	    LYK_FASTBACKW_LINK, "FASTBACKW_LINK",
	    "previous link or text area, only stops on links"),
    DATA(
	    LYK_FASTFORW_LINK, "FASTFORW_LINK",
	    "next link or text area, only stops on links"),
    DATA(
	    LYK_UP_LINK, "UP_LINK",
	    "move up the page to a previous link"),
    DATA(
	    LYK_DOWN_LINK, "DOWN_LINK",
	    "move down the page to another link"),
    DATA(
	    LYK_RIGHT_LINK, "RIGHT_LINK",
	    "move right to another link"),
    DATA(
	    LYK_LEFT_LINK, "LEFT_LINK",
	    "move left to a previous link"),
    DATA(
	    LYK_HISTORY, "HISTORY",
	    "display stack of currently-suspended documents"),
    DATA(
	    LYK_PREV_DOC, "PREV_DOC",
	    "go back to the previous document"),
    DATA(
	    LYK_NEXT_DOC, "NEXT_DOC",
	    "undo going back to the previous document"),
    DATA(
	    LYK_ACTIVATE, "ACTIVATE",
	    "go to the document given by the current link"),
    DATA(
	    LYK_MOUSE_SUBMIT, "MOUSE_SUBMIT",
	    "DO NOT MAP:  follow current link, submit"),
    DATA(
	    LYK_SUBMIT, "SUBMIT",
	    "prompt and submit form"),
    DATA(
	    LYK_RESET, "RESET",
	    "reset fields on current form"),
    DATA(
	    LYK_GOTO, "GOTO",
	    "go to a document given as a URL"),
    DATA(
	    LYK_ECGOTO, "ECGOTO",
	    "edit the current document's URL and go to it"),
    DATA(
	    LYK_HELP, "HELP",
	    "display help on using the browser"),
    DATA(
	    LYK_DWIMHELP, "DWIMHELP",
	    "display help page that may depend on context"),
    DATA(
	    LYK_INDEX, "INDEX",
	    "display an index of potentially useful documents"),
    DATA(
	    LYK_NOCACHE, "NOCACHE",
	    "force submission of form or link with no-cache"),
    DATA(
	    LYK_INTERRUPT, "INTERRUPT",
	    "interrupt network connection or transmission"),
    DATA(
	    LYK_MAIN_MENU, "MAIN_MENU",
	    "return to the first screen (home page)"),
    DATA(
	    LYK_OPTIONS, "OPTIONS",
	    "display and change option settings"),
    DATA(
	    LYK_INDEX_SEARCH, "INDEX_SEARCH",
	    "allow searching of an index"),
    DATA(
	    LYK_WHEREIS, "WHEREIS",
	    "search within the current document"),
    DATA(
	    LYK_PREV, "PREV",
	    "search for the previous occurrence"),
    DATA(
	    LYK_NEXT, "NEXT",
	    "search for the next occurrence"),
    DATA(
	    LYK_COMMENT, "COMMENT",
	    "send a comment to the author of the current document"),
    DATA(
	    LYK_EDIT, "EDIT",
	    "edit the current document or a form's textarea"),
    DATA(
	    LYK_INFO, "INFO",
	    "display information on the current document and link"),
    DATA(
	    LYK_PRINT, "PRINT",
	    "display choices for printing the current document"),
    DATA(
	    LYK_ADD_BOOKMARK, "ADD_BOOKMARK",
	    "add to your personal bookmark list"),
    DATA(
	    LYK_DEL_BOOKMARK, "DEL_BOOKMARK",
	    "delete from your personal bookmark list"),
    DATA(
	    LYK_VIEW_BOOKMARK, "VIEW_BOOKMARK",
	    "view your personal bookmark list"),
    DATA(
	    LYK_VLINKS, "VLINKS",
	    "list links visited during the current Lynx session"),
    DATA(
	    LYK_SHELL, "SHELL",
	    "escape from the browser to the system"),
    DATA(
	    LYK_DOWNLOAD, "DOWNLOAD",
	    "download the current link to your computer"),
    DATA(
	    LYK_TRACE_TOGGLE, "TRACE_TOGGLE",
	    "toggle tracing of browser operations"),
    DATA(
	    LYK_TRACE_LOG, "TRACE_LOG",
	    "view trace log if started in the current session"),
    DATA(
	    LYK_IMAGE_TOGGLE, "IMAGE_TOGGLE",
	    "toggle handling of all images as links"),
    DATA(
	    LYK_INLINE_TOGGLE, "INLINE_TOGGLE",
	    "toggle pseudo-ALTs for inlines with no ALT string"),
    DATA(
	    LYK_HEAD, "HEAD",
	    "send a HEAD request for the current document or link"),
    DATA(
	    LYK_DO_NOTHING, "DO_NOTHING",
	    NULL),
    DATA(
	    LYK_TOGGLE_HELP, "TOGGLE_HELP",
	    "show other commands in the novice help menu"),
    DATA(
	    LYK_JUMP, "JUMP",
	    "go directly to a target document or action"),
    DATA(
	    LYK_EDITMAP, "EDITMAP",
	    "display the current edit-key map"),
    DATA(
	    LYK_KEYMAP, "KEYMAP",
	    "display the current key map"),
    DATA(
	    LYK_LIST, "LIST",
	    "list the references (links) in the current document"),
    DATA(
	    LYK_TOOLBAR, "TOOLBAR",
	    "go to Toolbar or Banner in the current document"),
    DATA(
	    LYK_HISTORICAL, "HISTORICAL",
	    "toggle historical vs.  valid/minimal comment parsing"),
    DATA(
	    LYK_MINIMAL, "MINIMAL",
	    "toggle minimal vs.  valid comment parsing"),
    DATA(
	    LYK_SOFT_DQUOTES, "SOFT_DQUOTES",
	    "toggle valid vs.  soft double-quote parsing"),
    DATA(
	    LYK_RAW_TOGGLE, "RAW_TOGGLE",
	    "toggle raw 8-bit translations or CJK mode ON or OFF"),
    DATA(
	    LYK_COOKIE_JAR, "COOKIE_JAR",
	    "examine the Cookie Jar"),
    DATA(
	    LYK_F_LINK_NUM, "F_LINK_NUM",
	    "invoke the 'Follow link (or page) number:' prompt"),
    DATA(
	    LYK_CLEAR_AUTH, "CLEAR_AUTH",
	    "clear all authorization info for this session"),
    DATA(
	    LYK_SWITCH_DTD, "SWITCH_DTD",
	    "switch between two ways of parsing HTML"),
    DATA(
	    LYK_ELGOTO, "ELGOTO",
	    "edit the current link's URL or ACTION and go to it"),
    DATA(
	    LYK_CHANGE_LINK, "CHANGE_LINK",
	    "force reset of the current link on the page"),
    DATA(
	    LYK_DWIMEDIT, "DWIMEDIT",
	    "use external editor for context-dependent purpose"),
    DATA(
	    LYK_EDITTEXTAREA, "EDITTEXTAREA",
	    "use an external editor to edit a form's textarea"),
    DATA(
	    LYK_GROWTEXTAREA, "GROWTEXTAREA",
	    "add 5 new blank lines to the bottom of a textarea"),
    DATA(
	    LYK_INSERTFILE, "INSERTFILE",
	    "insert file into a textarea (just above cursorline)"),
#ifdef USE_ADDRLIST_PAGE
    DATA(
	    LYK_ADDRLIST, "ADDRLIST",
	    "like LIST command, but always shows the links' URLs"),
#endif
#ifdef USE_EXTERNALS
    DATA(
	    LYK_EXTERN_LINK, "EXTERN_LINK",
	    "run external program with current link"),
    DATA(
	    LYK_EXTERN_PAGE, "EXTERN_PAGE",
	    "run external program with current page"),
#endif
#ifdef VMS
    DATA(
	    LYK_DIRED_MENU, "DIRED_MENU",
	    "invoke File/Directory Manager, if available"),
#else
#ifdef DIRED_SUPPORT
    DATA(
	    LYK_DIRED_MENU, "DIRED_MENU",
	    "display a full menu of file operations"),
    DATA(
	    LYK_CREATE, "CREATE",
	    "create a new file or directory"),
    DATA(
	    LYK_REMOVE, "REMOVE",
	    "remove a file or directory"),
    DATA(
	    LYK_MODIFY, "MODIFY",
	    "modify the name or location of a file or directory"),
    DATA(
	    LYK_TAG_LINK, "TAG_LINK",
	    "tag a file or directory for later action"),
    DATA(
	    LYK_UPLOAD, "UPLOAD",
	    "upload from your computer to the current directory"),
    DATA(
	    LYK_INSTALL, "INSTALL",
	    "install file or tagged files into a system area"),
#endif				/* DIRED_SUPPORT */
    DATA(
	    LYK_CHANGE_CENTER, "CHANGE_CENTER",
	    "toggle center alignment in HTML TABLE"),
#ifdef KANJI_CODE_OVERRIDE
    DATA(
	    LYK_CHANGE_KCODE, "CHANGE_KCODE",
	    "Change Kanji code"),
#endif
#endif				/* VMS */
#ifdef SUPPORT_CHDIR
    DATA(
	    LYK_CHDIR, "CHDIR",
	    "change current directory"),
    DATA(
	    LYK_PWD, "PWD",
	    "print current directory"),
#endif
#ifdef USE_CURSES_PADS
    DATA(
	    LYK_SHIFT_LEFT, "SHIFT_LEFT",
	    "shift the screen left"),
    DATA(
	    LYK_SHIFT_RIGHT, "SHIFT_RIGHT",
	    "shift the screen right"),
    DATA(
	    LYK_LINEWRAP_TOGGLE, "LINEWRAP_TOGGLE",
	    "toggle linewrap on/off"),
#endif
#ifdef CAN_CUT_AND_PASTE
    DATA(
	    LYK_PASTE_URL, "PASTE_URL",
	    "Goto the URL in the clipboard"),
    DATA(
	    LYK_TO_CLIPBOARD, "TO_CLIPBOARD",
	    "link's URL to Clip Board"),
#endif
#ifdef EXP_NESTED_TABLES
    DATA(
	    LYK_NESTED_TABLES, "NESTED_TABLES",
	    "toggle nested-table parsing on/off"),
#endif
#ifdef USE_CACHEJAR
    DATA(
	    LYK_CACHE_JAR, "CACHE_JAR",
	    "examine list of cached documents"),
#endif
#ifdef USE_MAXSCREEN_TOGGLE
    DATA(
	    LYK_MAXSCREEN_TOGGLE, "MAXSCREEN_TOGGLE",
	    "toggle max screen and normal"),
#endif
    DATA(
	    LYK_UNKNOWN, NULL,
	    "")
};


#undef DATA
/* *INDENT-OFF* */
static const struct {
    int key;
    const char *name;
} named_keys[] = {
    { '\t',		"<tab>" },
    { '\r',		"<return>" },
    { CH_ESC,		"ESC" },
    { ' ',		"<space>" },
    { '<',		"<" },
    { '>',		">" },
    /* LYExtraKeys */
    { CH_DEL,		"<delete>" },
    { UPARROW_KEY,	"Up Arrow" },
    { DNARROW_KEY,	"Down Arrow" },
    { RTARROW_KEY,	"Right Arrow" },
    { LTARROW_KEY,	"Left Arrow" },
    { PGDOWN_KEY,	"Page Down" },
    { PGUP_KEY,		"Page Up" },
    { HOME_KEY,		"Home" },
    { END_KEY,		"End" },
    { F1_KEY,		"F1" },
    { F2_KEY,		"F2" },
    { F3_KEY,		"F3" },
    { F4_KEY,		"F4" },
    { F5_KEY,		"F5" },
    { F6_KEY,		"F6" },
    { F7_KEY,		"F7" },
    { F8_KEY,		"F8" },
    { F9_KEY,		"F9" },
    { F10_KEY,		"F10" },
    { F11_KEY,		"F11" },
    { F12_KEY,		"F12" },
    { DO_KEY,		"Do key" },
    { FIND_KEY,		"Find key" },
    { SELECT_KEY,	"Select key" },
    { INSERT_KEY,	"Insert key" },
    { REMOVE_KEY,	"Remove key" },
    { DO_NOTHING,	"(DO_NOTHING)" },
    { BACKTAB_KEY,	"Back Tab" },
    { MOUSE_KEY,	"mouse pseudo key" },
};
/* *INDENT-ON* */

/*
 * Build a list of Lynx's commands, for use in the tab-completion in LYgetstr.
 */
HTList *LYcommandList(void)
{
    static HTList *myList = NULL;

    if (myList == NULL) {
	unsigned j;

	myList = HTList_new();
	for (j = 0; revmap[j].name != 0; j++) {
	    if (revmap[j].doc != 0) {
		char *data = NULL;

		StrAllocCopy(data, revmap[j].name);
		HTList_addObject(myList, data);
	    }
	}
    }
    return myList;
}

/*
 * Find the given keycode.
 */
Kcmd *LYKeycodeToKcmd(LYKeymapCode code)
{
    unsigned j;
    Kcmd *result = 0;

    if (code > LYK_UNKNOWN) {
	for (j = 0; revmap[j].name != 0; j++) {
	    if (revmap[j].code == code) {
		result = revmap + j;
		break;
	    }
	}
    }
    return result;
}

/*
 * Find the given command-name, accepting an abbreviation if it is unique.
 */
Kcmd *LYStringToKcmd(const char *name)
{
    size_t need = strlen(name);
    size_t j;
    BOOL exact = FALSE;
    Kcmd *result = 0;
    Kcmd *maybe = 0;

    if (non_empty(name)) {
	for (j = 0; revmap[j].name != 0; j++) {
	    if (!strcasecomp(revmap[j].name, name)) {
		result = revmap + j;
		break;
	    } else if (!exact
		       && !strncasecomp(revmap[j].name, name, (int) need)) {
		if (maybe == 0) {
		    maybe = revmap + j;
		} else {
		    if (revmap[j].name[need] != 0
			&& maybe->name[need] != 0) {
			maybe = 0;
			exact = TRUE;
		    }
		}
	    }
	}
    }
    return (result != 0) ? result : maybe;
}

char *LYKeycodeToString(int c,
			int upper8)
{
    static char buf[30];
    unsigned n;
    BOOLEAN named = FALSE;

    for (n = 0; n < TABLESIZE(named_keys); n++) {
	if (named_keys[n].key == c) {
	    named = TRUE;
	    LYStrNCpy(buf, named_keys[n].name, sizeof(buf) - 1);
	    break;
	}
    }

    if (!named) {
	if (c <= 0377
	    && TOASCII(c) > TOASCII(' ')
	    && TOASCII(c) < 0177)
	    sprintf(buf, "%c", c);
	else if (upper8
		 && TOASCII(c) > TOASCII(' ')
		 && c <= 0377
		 && c <= LYlowest_eightbit[current_char_set])
	    sprintf(buf, "%c", c);
	else if (TOASCII(c) < TOASCII(' '))
	    sprintf(buf, "^%c", FROMASCII(TOASCII(c) | 0100));
	else if (c >= 0400)
	    sprintf(buf, "key-0x%x", (unsigned) c);
	else
	    sprintf(buf, "0x%x", (unsigned) c);
    }
    return buf;
}

int LYStringToKeycode(char *src)
{
    unsigned n;
    int key = -1;
    int len = (int) strlen(src);

    if (len == 1) {
	key = *src;
    } else if (len == 2 && *src == '^') {
	key = src[1] & 0x1f;
    } else if (len > 2 && !strncasecomp(src, "0x", 2)) {
	char *dst = 0;

	key = (int) strtol(src, &dst, 0);
	if (non_empty(dst))
	    key = -1;
    } else if (len > 6 && !strncasecomp(src, "key-", 4)) {
	char *dst = 0;

	key = (int) strtol(src + 4, &dst, 0);
	if (isEmpty(dst))
	    key = -1;
    }
    if (key < 0) {
	for (n = 0; n < TABLESIZE(named_keys); n++) {
	    if (!strcasecomp(named_keys[n].name, src)) {
		key = named_keys[n].key;
		break;
	    }
	}
    }
    return key;
}

#define PRETTY_LEN 11

static char *pretty_html(int c)
{
    char *src = LYKeycodeToString(c, TRUE);


    if (src != 0) {
	/* *INDENT-OFF* */
	static const struct {
	    int	code;
	    const char *name;
	} table[] = {
	    { '<',	"&lt;" },
	    { '>',	"&gt;" },
	    { '"',	"&quot;" },
	    { '&',	"&amp;" }
	};
	/* *INDENT-ON* */

	static char buf[30];
	char *dst = buf;
	int adj = 0;
	unsigned n;
	BOOLEAN found;

	while ((c = *src++) != 0) {
	    found = FALSE;
	    for (n = 0; n < TABLESIZE(table); n++) {
		if (c == table[n].code) {
		    found = TRUE;
		    LYStrNCpy(dst,
			      table[n].name,
			      sizeof(buf) - (size_t) ((dst - buf) - 1));
		    adj += (int) strlen(dst) - 1;
		    dst += (int) strlen(dst);
		    break;
		}
	    }
	    if (!found) {
		*dst++ = (char) c;
	    }
	}
	adj -= (int) (dst - buf) - PRETTY_LEN;
	while (adj-- > 0)
	    *dst++ = ' ';
	*dst = 0;
	return buf;
    }

    return 0;
}

static char *format_binding(LYKeymap_t *table, int i)
{
    LYKeymap_t the_key = table[i];
    char *buf = 0;
    char *formatted;
    Kcmd *rmap = LYKeycodeToKcmd((LYKeymapCode) the_key);

    if (rmap != 0
	&& rmap->name != 0
	&& rmap->doc != 0
	&& (formatted = pretty_html(i - 1)) != 0) {
	HTSprintf0(&buf, "%-*s %-13s %s\n",
		   PRETTY_LEN, formatted,
		   rmap->name,
		   rmap->doc);
	return buf;
    }
    return 0;
}

/* if both is true, produce an additional line for the corresponding
   uppercase key if its binding is different. - kw */
static void print_binding(HTStream *target, int i, int both)
{
    char *buf;
    LYKeymap_t lac1 = LYK_UNKNOWN;	/* 0 */

#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
    if (prev_lynx_edit_mode && !no_dired_support &&
	(lac1 = key_override[i]) != LYK_UNKNOWN) {
	if ((buf = format_binding(key_override, i)) != 0) {
	    PUTS(buf);
	    FREE(buf);
	}
    } else
#endif /* DIRED_SUPPORT && OK_OVERRIDE */
    if ((buf = format_binding(keymap, i)) != 0) {
	lac1 = keymap[i];
	PUTS(buf);
	FREE(buf);
    }

    if (!both)
	return;
    i -= ' ';			/* corresponding uppercase key */

#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
    if (prev_lynx_edit_mode && !no_dired_support && key_override[i]) {
	if (key_override[i] != lac1 &&
	    (buf = format_binding(key_override, i)) != 0) {
	    PUTS(buf);
	    FREE(buf);
	}
    } else
#endif /* DIRED_SUPPORT && OK_OVERRIDE */
    if (keymap[i] != lac1 && (buf = format_binding(keymap, i)) != 0) {
	PUTS(buf);
	FREE(buf);
    }
}

/*
 * Return lynxactioncode whose name is the string func.  returns -1 if not
 * found.  - kw
 */
int lacname_to_lac(const char *func)
{
    Kcmd *mp = LYStringToKcmd(func);

    return (mp != 0) ? (int) mp->code : -1;
}

/*
 * Return lynxkeycode represented by string src.  returns -1 if not valid.
 *
 * This is simpler than what map_string_to_keysym() does for USE_KEYMAP, but
 * compatible with revmap() used for processing KEYMAP options in the
 * configuration file.  - kw
 */
int lkcstring_to_lkc(const char *src)
{
    int c = -1;

    if (strlen(src) == 1) {
	c = *src;
    } else if (strlen(src) == 2 && *src == '^') {
	c = src[1] & 037;
    } else if (strlen(src) >= 2 && isdigit(UCH(*src))) {
	char *next = 0;

	c = (int) strtol(src, &next, 0);
	if (next != 0 && *next != '\0')
	    c = (-1);
#ifdef USE_KEYMAPS
    } else {
	map_string_to_keysym(src, &c, TRUE);
#ifndef USE_SLANG
	if (c >= 0) {
	    /* make curses-keys mapped from Keysym_Strings[] available here */
	    if ((c & LKC_MASK) > 255)
		c &= ~LKC_ISLKC;
	}
#endif
#endif
    }

    if (c == CH_ESC) {
	escape_bound = 1;
    } else if (c < -1) {
	c = (-1);
    }

    return c;
}

static int LYLoadKeymap(const char *arg GCC_UNUSED,
			HTParentAnchor *anAnchor,
			HTFormat format_out,
			HTStream *sink)
{
    HTFormat format_in = WWW_HTML;
    HTStream *target;
    char *buf = 0;
    int i;

    /*
     * Set up the stream.  - FM
     */
    target = HTStreamStack(format_in, format_out, sink, anAnchor);
    if (target == NULL) {
	HTSprintf0(&buf, CANNOT_CONVERT_I_TO_O,
		   HTAtom_name(format_in), HTAtom_name(format_out));
	HTAlert(buf);
	FREE(buf);
	return (HT_NOT_LOADED);
    }
    anAnchor->no_cache = TRUE;

    HTSprintf0(&buf, "<html>\n<head>\n<title>%s</title>\n</head>\n<body>\n",
	       CURRENT_KEYMAP_TITLE);
    PUTS(buf);
    HTSprintf0(&buf, "<pre>\n");
    PUTS(buf);

    for (i = KHR('a'); i <= KHR('z'); i++) {
	print_binding(target, i, TRUE);
    }
    for (i = 1; i < KEYMAP_SIZE; i++) {
	/*
	 * Don't show CHANGE_LINK if mouse not enabled.
	 */
	if ((i >= 0200 || i <= ' ' || !isalpha(i - 1)) &&
	    (LYUseMouse || (keymap[i] != LYK_CHANGE_LINK))) {
	    print_binding(target, i, FALSE);
	}
    }

    HTSprintf0(&buf, "</pre>\n</body>\n</html>\n");
    PUTS(buf);

    (*target->isa->_free) (target);
    FREE(buf);
    return (HT_LOADED);
}

#ifdef GLOBALDEF_IS_MACRO
#define _LYKEYMAP_C_GLOBALDEF_1_INIT { "LYNXKEYMAP", LYLoadKeymap, 0}
GLOBALDEF(HTProtocol, LYLynxKeymap, _LYKEYMAP_C_GLOBALDEF_1_INIT);
#else
GLOBALDEF HTProtocol LYLynxKeymap =
{"LYNXKEYMAP", LYLoadKeymap, 0};
#endif /* GLOBALDEF_IS_MACRO */

/*
 * Install func as the mapping for key.
 * If for_dired is TRUE, install it in the key_override[] table
 * for Dired mode, otherwise in the general keymap[] table.
 * If DIRED_SUPPORT or OK_OVERRIDE is not defined, don't do anything
 * when for_dired is requested.
 * returns lynxkeycode value != 0 if the mapping was made, 0 if not.
 */
int remap(char *key,
	  const char *func,
	  int for_dired)
{
    Kcmd *mp;
    int c;
    int result = 0;

#if !defined(DIRED_SUPPORT) || !defined(OK_OVERRIDE)
    if (for_dired) {
	return result;
    }
#endif
    if (func != NULL) {
	c = lkcstring_to_lkc(key);
	if ((c >= 0) && (c < KEYMAP_SIZE)) {
	    /* Remapping of key actions is supported only for basic
	     * lynxkeycodes, without modifiers etc.!  If we get somehow
	     * called for an invalid lynxkeycode, fail or silently ignore
	     * modifiers -KW
	     */
	    if (!(c & (LKC_ISLECLAC | LKC_ISLAC))) {
		c &= LKC_MASK;
		if ((mp = LYStringToKcmd(func)) != 0) {
#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
		    if (for_dired)
			key_override[KHR(c)] = mp->code;
		    else
#endif
			keymap[KHR(c)] = (LYKeymap_t) mp->code;
		    /* don't return 0, successful */
		    result = (c ? c : (int) LAC_TO_LKC0(mp->code));
		}
	    }
	}
    }
    return result;
}

typedef struct {
    int code;
    LYKeymap_t map;
    LYKeymap_t save;
} ANY_KEYS;

/*
 * Save the given keys in the table, setting them to the map'd value.
 */
static void set_any_keys(ANY_KEYS * table, size_t size)
{
    size_t j;

    for (j = 0; j < size; ++j) {
	int c = KHR(table[j].code);

	table[j].save = keymap[c];
	keymap[c] = table[j].map;
    }
}

/*
 * Restore the given keys from the table.
 */
static void reset_any_keys(ANY_KEYS * table, size_t size)
{
    size_t j;

    for (j = 0; j < size; ++j) {
	int c = KHR(table[j].code);

	keymap[c] = table[j].save;
    }
}

static ANY_KEYS vms_keys_table[] =
{
    {26, LYK_ABORT, 0},		/* control-Z */
    {'$', LYK_SHELL, 0},
};

void set_vms_keys(void)
{
    set_any_keys(vms_keys_table, TABLESIZE(vms_keys_table));
}

static ANY_KEYS vi_keys_table[] =
{
    {'h', LYK_PREV_DOC, 0},
    {'j', LYK_NEXT_LINK, 0},
    {'k', LYK_PREV_LINK, 0},
    {'l', LYK_ACTIVATE, 0},
};

static BOOLEAN did_vi_keys;

void set_vi_keys(void)
{
    set_any_keys(vi_keys_table, TABLESIZE(vi_keys_table));
    did_vi_keys = TRUE;
}

void reset_vi_keys(void)
{
    if (did_vi_keys) {
	reset_any_keys(vi_keys_table, TABLESIZE(vi_keys_table));
	did_vi_keys = FALSE;
    }
}

static ANY_KEYS emacs_keys_table[] =
{
    {2, LYK_PREV_DOC, 0},	/* ^B */
    {14, LYK_NEXT_LINK, 0},	/* ^N */
    {16, LYK_PREV_LINK, 0},	/* ^P */
    {6, LYK_ACTIVATE, 0},	/* ^F */
};

static BOOLEAN did_emacs_keys;

void set_emacs_keys(void)
{
    set_any_keys(emacs_keys_table, TABLESIZE(emacs_keys_table));
    did_emacs_keys = TRUE;
}

void reset_emacs_keys(void)
{
    if (did_emacs_keys) {
	reset_any_keys(emacs_keys_table, TABLESIZE(emacs_keys_table));
	did_emacs_keys = FALSE;
    }
}

/*
 * Map numbers to functions as labeled on the IBM Enhanced keypad, and save
 * their original mapping for reset_numbers_as_arrows().  - FM
 */
static ANY_KEYS number_keys_table[] =
{
    {'1', LYK_END, 0},
    {'2', LYK_NEXT_LINK, 0},
    {'3', LYK_NEXT_PAGE, 0},
    {'4', LYK_PREV_DOC, 0},
    {'5', LYK_DO_NOTHING, 0},
    {'6', LYK_ACTIVATE, 0},
    {'7', LYK_HOME, 0},
    {'8', LYK_PREV_LINK, 0},
    {'9', LYK_PREV_PAGE, 0},
};

static BOOLEAN did_number_keys;

void set_numbers_as_arrows(void)
{
    set_any_keys(number_keys_table, TABLESIZE(number_keys_table));
    did_number_keys = TRUE;
}

void reset_numbers_as_arrows(void)
{
    if (did_number_keys) {
	reset_any_keys(number_keys_table, TABLESIZE(number_keys_table));
	did_number_keys = FALSE;
    }
}

char *key_for_func(int func)
{
    static char *buf;
    int i;
    char *formatted;

    if ((i = LYReverseKeymap(func)) >= 0) {
	formatted = LYKeycodeToString(i, TRUE);
	StrAllocCopy(buf, formatted != 0 ? formatted : "?");
    } else if (buf == 0) {
	StrAllocCopy(buf, "");
    }
    return buf;
}

/*
 * Given one or two keys as lynxkeycodes, returns an allocated string
 * representing the key(s) suitable for statusline messages, or NULL if no
 * valid lynxkeycode is passed in (i.e., lkc_first < 0 or some other failure). 
 * The caller must free the string.  - kw
 */
char *fmt_keys(int lkc_first,
	       int lkc_second)
{
    char *buf = NULL;
    BOOLEAN quotes = FALSE;
    char *fmt_first;
    char *fmt_second;

    if (lkc_first < 0)
	return NULL;
    fmt_first = LYKeycodeToString(lkc_first, TRUE);
    if (fmt_first && strlen(fmt_first) == 1 && *fmt_first != '\'') {
	quotes = TRUE;
    }
    if (quotes) {
	if (lkc_second < 0) {
	    HTSprintf0(&buf, "'%s'", fmt_first);
	    return buf;
	} else {
	    HTSprintf0(&buf, "'%s", fmt_first);
	}
    } else {
	StrAllocCopy(buf, fmt_first);
    }
    if (lkc_second >= 0) {
	fmt_second = LYKeycodeToString(lkc_second, TRUE);
	if (!fmt_second) {
	    FREE(buf);
	    return NULL;
	}
	HTSprintf(&buf, "%s%s%s",
		  (((strlen(fmt_second) > 2 && *fmt_second != '<') ||
		    (strlen(buf) > 2 && buf[strlen(buf) - 1] != '>'))
		   ? " "
		   : ""),
		  fmt_second, quotes ? "'" : "");
    }
    return buf;
}

/*
 * This function returns the (int)ch mapped to the LYK_foo value passed to it
 * as an argument.  It is like LYReverseKeymap, only the order of search is
 * different; e.g., small ASCII letters will be returned in preference to
 * capital ones.  Cf.  LYKeyForEditAction, LYEditKeyForAction in LYEditmap.c
 * which use the same order to find a best key.  In addition, this function
 * takes the dired override map into account while LYReverseKeymap doesn't. 
 * The caller must free the returned string.  - kw
 */
#define FIRST_I 97
#define NEXT_I(i,imax) ((i==122) ? 32 : (i==96) ? 123 : (i==126) ? 0 :\
			(i==31) ? 256 : (i==imax) ? 127 :\
			(i==255) ? (-1) :i+1)
static int best_reverse_keymap(int lac)
{
    int i, c;

    for (i = FIRST_I; i >= 0; i = NEXT_I(i, KEYMAP_SIZE - 1)) {
#ifdef NOT_ASCII
	if (i < 256) {
	    c = FROMASCII(i);
	} else
#endif
	    c = i;
#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
	if (lynx_edit_mode && !no_dired_support && lac &&
	    LKC_TO_LAC(key_override, c) == lac)
	    return c;
#endif /* DIRED_SUPPORT && OK_OVERRIDE */
	if (LKC_TO_LAC(keymap, c) == lac) {
	    return c;
	}
    }

    return (-1);
}

/*
 * This function returns a string representing a key mapped to a LYK_foo
 * function, or NULL if not found.  The string may represent a pair of keys. 
 * if context_code is FOR_INPUT, an appropriate binding for use while in the
 * (forms) line editor is sought.  - kw
 */
char *key_for_func_ext(int lac,
		       int context_code)
{
    int lkc, modkey = -1;

    if (context_code == FOR_INPUT) {
	lkc = LYEditKeyForAction(lac, &modkey);
	if (lkc >= 0) {
	    if (lkc & (LKC_MOD1 | LKC_MOD2 | LKC_MOD3)) {
		return fmt_keys(modkey, lkc & ~(LKC_MOD1 | LKC_MOD2 | LKC_MOD3));
	    } else {
		return fmt_keys(lkc, -1);
	    }
	}
    }
    lkc = best_reverse_keymap(lac);
    if (lkc < 0)
	return NULL;
    if (context_code == FOR_INPUT) {
	modkey = LYKeyForEditAction(LYE_LKCMD);
	if (modkey < 0)
	    return NULL;
	return fmt_keys(modkey, lkc);
    } else {
	return fmt_keys(lkc, -1);
    }
}

/*
 * This function returns TRUE if the ch is non-alphanumeric and maps to KeyName
 * (LYK_foo in the keymap[] array).  - FM
 */
BOOLEAN LYisNonAlnumKeyname(int ch,
			    int KeyName)
{
    BOOLEAN result = FALSE;

    if (ch >= 0 && KHR(ch) < KEYMAP_SIZE) {
	if ((ch <= 0
	     || StrChr("0123456789"
		       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		       "abcdefghijklmnopqrstuvwxyz", ch) == NULL)
	    && (keymap[KHR(ch)] == KeyName)) {
	    result = TRUE;
	}
    }
    return result;
}

/*
 * This function returns the (int)ch mapped to the LYK_foo value passed to it
 * as an argument.  - FM
 */
int LYReverseKeymap(int KeyName)
{
    int i;
    int result = -1;

    for (i = 1; i < KEYMAP_SIZE; i++) {
	if (keymap[i] == KeyName) {
	    result = (i - 1);
	    break;
	}
    }

    return result;
}

#ifdef EXP_KEYBOARD_LAYOUT
BOOLEAN LYSetKbLayout(char *layout_id)
{
    int i;
    BOOLEAN result = FALSE;

    for (i = 0; i < (int) TABLESIZE(LYKbLayoutNames) - 1; i++) {
	if (!strcasecomp(LYKbLayoutNames[i], layout_id)) {
	    current_layout = i;
	    result = TRUE;
	    break;
	}
    }

    return result;
}
#endif

#if 0
/*
 * This function was useful in converting the hand-crafted key-bindings to
 * their reusable form in 2.8.8 -TD
 */
static void checkKeyMap(LYEditConfig * table)
{
    unsigned j, k;
    char comment[80];
    int first = TRUE;

    for (j = 0; table->init[j].code >= 0; ++j) {
	int code = table->init[j].code;

	if (table->init[j].edit != table->used[code]) {
	    if (first) {
		printf("TABLE %s\n", table->name);
		first = FALSE;
	    }
	    printf("%u: init %d vs used %d\n",
		   j,
		   table->init[j].edit,
		   table->used[code]);
	}
    }
    for (j = 0; j < KEYMAP_SIZE; ++j) {
	int code = (int) j;
	BOOL found = FALSE;

	for (k = 0; table->init[k].code >= 0; ++k) {
	    if (code == table->init[k].code) {
		found = TRUE;
		break;
	    }
	}
	if (!found) {
	    if (table->used[j] != 0) {
		unsigned used = (j - 1);
		int edit = table->used[j];
		const char *prefix = "LYK_";
		const char *name = 0;
		Kcmd *cmd = LYKeycodeToKcmd(edit + 0);

		if (cmd != 0) {
		    name = cmd->name;
		}

		if (used < 32) {
		    char temp[80];
		    const char *what = 0;

		    switch (used) {
		    case 0:
			what = "nul";
			break;
		    case 17:
			what = "XON";
			break;
		    case 19:
			what = "XOFF";
			break;
		    default:
			sprintf(temp, "^%c", used + 'A');
			what = temp;
			break;
		    }
		    sprintf(comment, "\t/* %s */", what);
		} else if (used < 127) {
		    sprintf(comment, "\t/* %c */", used);
		} else if (used == 127) {
		    strcpy(comment, "\t/* DEL */");
		} else {
		    const char *what = LYextraKeysToName(used);

		    if (non_empty(what)) {
			sprintf(comment, "\t/* %s%s */", what,
				((StrChr(what, '_') != 0)
				 ? ""
				 : "_KEY"));
		    } else {
			strcpy(comment, "");
		    }
		}
		if (name == 0) {
		    name = "XXX";
		}
		if (first) {
		    printf("TABLE %s\n", table->name);
		    first = FALSE;
		}
		printf("\t{ %d, %s%s },%s\n", code, prefix, name, comment);
	    }
	}
    }
}

#else
#define checkKeyMap(table)	/* nothing */
#endif

static void initKeyMap(LYEditConfig * table)
{
    unsigned k;
    LYEditCode *used = table->used;
    const LYEditInit *init = table->init;

    memset(used, 0, sizeof(LYEditCode) * KEYMAP_SIZE);

    for (k = 0; init[k].code >= 0; ++k) {
	int code = init[k].code;

	used[code] = init[k].edit;
    }
    checkKeyMap(table);
}

/*
 * Reset the key bindings to their default values.
 */
void LYinitKeymap(void)
{
    CTRACE((tfp, "LYinitKeymap\n"));
    initKeyMap(&myKeymapData);
#if defined(DIRED_SUPPORT) && defined(OK_OVERRIDE)
    initKeyMap(&myOverrideData);
#endif
}
