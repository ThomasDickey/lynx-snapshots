#include <HTUtils.h>
#include <HTCJK.h>
#include <LYCurses.h>
#include <LYUtils.h>
#include <LYStrings.h>
#include <LYGlobalDefs.h>
#include <GridText.h>
#include <LYKeymap.h>
#include <LYSignal.h>
#include <LYClean.h>
#include <LYMail.h>
#include <LYNews.h>
#include <LYOptions.h>
#include <LYCharSets.h>
#include <HTString.h>

#ifdef DJGPP_KEYHANDLER
#include <pc.h>
#include <keys.h>
#endif /* DJGPP_KEYHANDLER */

#include <LYLeaks.h>

extern BOOL HTPassHighCtrlRaw;
extern HTCJKlang HTCJK;

/* If you want to add mouse support for some new platform, it's fairly
** simple to do.  Once you've determined the X and Y coordinates of
** the mouse event, loop through the elements in the links[] array and
** see if the coordinates fall within a highlighted link area.	If so,
** the code must set mouse_link to the index of the chosen link,
** and return a key value that corresponds to LYK_ACTIVATE.  The
** LYK_ACTIVATE code in LYMainLoop.c will then check mouse_link
** and activate that link.  If the mouse event didn't fall within a
** link, the code should just set mouse_link to -1 and return -1. --AMK
**/

/* The number of the link selected w/ the mouse (-1 if none) */
static int mouse_link = -1;

/* Return the value of mouse_link, erasing it */
PUBLIC int get_mouse_link NOARGS
{
  int t;
  t=mouse_link;
  mouse_link = -1;
  return t;
}

/* Given X and Y coordinates of a mouse event, set mouse_link to the
** index of the corresponding hyperlink, or set mouse_link to -1 if no
** link matches the event.  Returns -1 if no link matched the click,
** or a keycode that must be returned from LYgetch() to activate the
** link.
**/

PRIVATE int set_clicked_link ARGS2(int,x,int,y)
{
    int left = 6;
    int right = LYcols-6;
    /* yes, I am assuming that my screen will be a certain width. */
    int i;
    int c = -1;

    if (y == (LYlines-1)) {
	if (x < left) c = LTARROW;
	else if (x > right) c = '\b';
	else c = PGDOWN;
    } else if (y == 0) {
	if (x < left) c = LTARROW;
	else if (x > right) c = '\b';
	else c = PGUP;
    } else {
	/* Loop over the links and see if we can get a match */
	for (i = 0; i < nlinks; i++) {
	    /* Check the first line of the link */
	    if ( links[i].hightext != NULL &&
		links[i].ly == y &&
		(x - links[i].lx) < (int)strlen(links[i].hightext ) ) {
		mouse_link = i;
		break;
	    }
	    /* Check the second line */
	    if (links[i].hightext2 != NULL &&
		1+links[i].ly == y &&
		(x - links[i].hightext2_offset) < (int)strlen(links[i].hightext2) ) {
		mouse_link = i;
		break;
	    }
	}
	/*
	 * If a link was hit, we must look for a key which will activate
	 * LYK_ACTIVATE We expect to find LYK_ACTIVATE (it's usually mapped to
	 * the Enter key).
	 */
	if (mouse_link >= 0)
	    c = lookup_keymap(LYK_ACTIVATE);
    }
    return c;
}


/*
 *  LYstrncpy() terminates strings with a null byte.
 *  Writes a null byte into the n+1 byte of dst.
 */
PUBLIC char *LYstrncpy ARGS3(
	char *, 	dst,
	CONST char *,	src,
	int,		n)
{
    char *val;
    int len = strlen(src);

    if (n < 0)
	n = 0;

    val = strncpy(dst, src, n);
    if (len < n)
	*(dst+len) = '\0';
    else
	*(dst+n) = '\0';
    return val;
}

#define IS_NEW_GLYPH(ch) (utf_flag && ((unsigned char)(ch)&0xc0) != 0x80)
#define IS_UTF_EXTRA(ch) (utf_flag && ((unsigned char)(ch)&0xc0) == 0x80)

/*
 *  LYmbcsstrncpy() terminates strings with a null byte.
 *  It takes account of multibyte characters.
 *  The src string is copied until either end of string or max number of
 *  either bytes or glyphs (mbcs sequences) (CJK or UTF8).  The utf_flag
 *  argument should be TRUE for UTF8. - KW & FM
 */
PUBLIC char * LYmbcsstrncpy ARGS5(
	char *, 	dst,
	CONST char *,	src,
	int,		n_bytes,
	int,		n_glyphs,
	BOOL,		utf_flag)
{
    char *val = dst;
    int i_bytes = 0, i_glyphs = 0;

    if (n_bytes < 0)
	n_bytes = 0;
    if (n_glyphs < 0)
	n_glyphs = 0;

    for (; *src != '\0' && i_bytes < n_bytes; i_bytes++) {
	if (IS_NEW_GLYPH(*src)) {
	    if (i_glyphs++ >= n_glyphs) {
		*dst = '\0';
		return val;
	    }
	}
	*(dst++) = *(src++);
    }
    *dst = '\0';

    return val;
}

/*
 *  LYmbcs_skip_glyphs() skips a given number of display positions
 *  in a string and returns the resulting pointer.  It takes account
 *  of UTF-8 encoded characters. - KW
 */
PUBLIC char * LYmbcs_skip_glyphs ARGS3(
	char *, 	data,
	int,		n_glyphs,
	BOOL,		utf_flag)
{
    int i_glyphs = 0;

    if (n_glyphs < 0)
	n_glyphs = 0;

    if (!data)
	return NULL;
    if (!utf_flag)
	return (data + n_glyphs);

    while (*data) {
	if (IS_NEW_GLYPH(*data)) {
	    if (i_glyphs++ >= n_glyphs) {
		return data;
	    }
	}
	data++;
    }
    return data;
}

/*
 *  LYmbcsstrlen() returns the printable length of a string
 *  that might contain IsSpecial or multibyte (CJK or UTF8)
 *  characters. - FM
 */
PUBLIC int LYmbcsstrlen ARGS2(
	char *, 	str,
	BOOL,		utf_flag)
{
    int i, j, len = 0;

    if (!str && *str)
	return(len);

    for (i = 0; str[i] != '\0'; i++) {
	if (IsSpecialAttrChar(str[i])) {
	    continue;
	} else {
	    len++;
	}
	if (IS_NEW_GLYPH(str[i])) {
	    j = 0;
	    while (str[(i + 1)] != '\0' &&
		   !IsSpecialAttrChar(str[(i + 1)]) &&
		   j < 5 &&
		   IS_UTF_EXTRA(str[(i + 1)])) {
		i++;
		j++;
	    }
	} else if (!utf_flag && HTCJK != NOCJK && !isascii(str[i]) &&
		    str[(i + 1)] != '\0' &&
		    !IsSpecialAttrChar(str[(i + 1)])) {
	    i++;
	}
    }

    return(len);
}

#undef GetChar

#ifdef USE_SLANG
#ifdef VMS
#define GetChar() ttgetc()
#else
#ifdef __DJGPP__
#ifdef DJGPP_KEYHANDLER
#define GetChar getxkey
#else
#define GetChar SLkp_getkey
#endif /* DJGPP_KEYHANDLER */
#else
#define GetChar (int)SLang_getkey
#endif /* __DJGPP__ */
#endif /* VMS */
#endif /* USE_SLANG */

#if !defined(GetChar) && defined(NCURSES)
#define GetChar() wgetch(my_subwindow ? my_subwindow : stdscr)
#endif

#if !defined(GetChar) && defined(SNAKE)
#define GetChar() wgetch(stdscr)
#endif

#if !defined(GetChar) && defined(VMS)
#define GetChar() ttgetc()
#endif

#if !defined(GetChar)
#if HAVE_KEYPAD
#define GetChar getch
#else
#ifndef USE_GETCHAR
#define USE_GETCHAR
#endif /* !USE_GETCHAR */
#define GetChar() getchar()  /* used to be "getc(stdin)" and "getch()" */
#endif /* HAVE_KEYPAD */
#endif /* !defined(GetChar) */

#if defined(NCURSES)
/*
 * Workaround a bug in ncurses order-of-refresh by setting a pointer to
 * the topmost window that should be displayed.
 *
 * FIXME: the associated call on 'keypad()' is not needed for Unix, but
 * something in the OS/2 EMX port requires it.
 */
PRIVATE WINDOW *my_subwindow;

PUBLIC void LYsubwindow ARGS1(WINDOW *, param)
{
    if ((my_subwindow = param) != 0)
	keypad(param, TRUE);
}
#endif

#ifdef USE_SLANG_MOUSE
PRIVATE int sl_parse_mouse_event ARGS3(int *, x, int *, y, int *, button)
{
    /* "ESC [ M" has already been processed.  There more characters are
     * expected:  BUTTON X Y
     */
    *button = SLang_getkey ();
    switch (*button)
    {
    case 040:			/* left button */
    case 041:			/* middle button */
    case 042:			/* right button */
	*button -= 040;
	break;

    default:			/* Hmmm.... */
	SLang_flush_input ();
	return -1;
    }

    *x = SLang_getkey () - 33;
    *y = SLang_getkey () - 33;
    return 0;
}

PRIVATE int sl_read_mouse_event NOARGS
{
   int mouse_x, mouse_y, button;

   mouse_link = -1;
   if (-1 != sl_parse_mouse_event (&mouse_x, &mouse_y, &button))
     {
	if (button == 0)  /* left */
	  return set_clicked_link (mouse_x, mouse_y);

	if (button == 2)   /* right */
	  {
	     /* Right button: go back to prev document.
	      * The problem is that we need to determine
	      * what to return to achieve this.
	      */
	     return LYReverseKeymap (LYK_PREV_DOC);
	  }
     }
   return -1;
}
#endif

PRIVATE BOOLEAN csi_is_csi = TRUE;
PUBLIC void ena_csi ARGS1(
    BOOLEAN,	flag)
{
    csi_is_csi = flag;
}

#if defined(USE_KEYMAPS)

#ifdef USE_SLANG
#define define_key(string, code) \
	SLkm_define_keysym (string, code, Keymap_List)
#define expand_substring(dst, first, last) \
	SLexpand_escaped_string(dst, first, last)
static SLKeyMap_List_Type *Keymap_List;
/* This value should be larger than anything in LYStrings.h */
#define MOUSE_KEYSYM 0x1000
#endif


#define SQUOTE '\''
#define DQUOTE '"'
#define ESCAPE '\\'
#define LPAREN '('
#define RPAREN ')'

/*
 * For ncurses, we use the predefined keysyms, since that lets us also reuse
 * the CSI logic and other special cases for VMS, NCSA telnet, etc.
 */
#ifdef USE_SLANG
#define DEFINE_KEY(string,lynx,curses) {string,lynx}
#else
#define DEFINE_KEY(string,lynx,curses) {string,curses}
#endif

typedef struct
{
   char *string;
   int value;
}
Keysym_String_List;

static Keysym_String_List Keysym_Strings [] =
{
    DEFINE_KEY( "UPARROW",	UPARROW,	KEY_UP ),
    DEFINE_KEY( "DNARROW",	DNARROW,	KEY_DOWN ),
    DEFINE_KEY( "RTARROW",	RTARROW,	KEY_RIGHT ),
    DEFINE_KEY( "LTARROW",	LTARROW,	KEY_LEFT ),
    DEFINE_KEY( "PGDOWN",	PGDOWN,		KEY_NPAGE ),
    DEFINE_KEY( "PGUP",		PGUP,		KEY_PPAGE ),
    DEFINE_KEY( "HOME",		HOME,		KEY_HOME ),
    DEFINE_KEY( "END",		END_KEY,	KEY_END ),
    DEFINE_KEY( "F1",		F1,		KEY_F(1) ),
    DEFINE_KEY( "DO_KEY",	DO_KEY,		KEY_F(16) ),
    DEFINE_KEY( "FIND_KEY",	FIND_KEY,	KEY_FIND ),
    DEFINE_KEY( "SELECT_KEY",	SELECT_KEY,	KEY_SELECT ),
    DEFINE_KEY( "INSERT_KEY",	INSERT_KEY,	KEY_IC ),
    DEFINE_KEY( "REMOVE_KEY",	REMOVE_KEY,	KEY_DC ),
    DEFINE_KEY( "DO_NOTHING",	DO_NOTHING,	0 ),
    DEFINE_KEY( NULL, 		-1,		ERR )
};

#ifdef NCURSES_VERSION
/*
 * Ncurses stores the termcap/terminfo names in arrays sorted to match the
 * array of strings in the TERMTYPE struct.
 */
PRIVATE int lookup_tiname (char *name, NCURSES_CONST char *const *names)
{
    int code;

    for (code = 0; names[code] != 0; code++)
	if (!strcmp(names[code], name))
	    return code;
    return -1;
}

PRIVATE char *expand_tiname (char *first, size_t len, char **result)
{
    char name[BUFSIZ];
    int code;

    strncpy(name, first, len);
    name[len] = '\0';
    if ((code = lookup_tiname(name, strnames)) >= 0
     || (code = lookup_tiname(name, strfnames)) >= 0) {
	strcpy(*result, cur_term->type.Strings[code]);
	(*result) += strlen(*result);
    }
    return first + len;
}

PRIVATE char *expand_tichar (char *first, char **result)
{
    int ch;
    int limit = 0;
    int radix = 0;
    int value = 0;
    char *name = 0;

    switch (ch = *first++) {
    case 'E': case 'e':	value = 27;			break;
    case 'a':		name  = "bel";			break;
    case 'b':		value = '\b';			break;
    case 'f':		value = '\f';			break;
    case 'n':		value = '\n';			break;
    case 'r':		value = '\r';			break;
    case 't':		value = '\t';			break;
    case 'v':		value = '\v';			break;
    case 'd':		radix = 10;	limit = 3;	break;
    case 'x':		radix = 16;	limit = 2;	break;
    default:
	if (isdigit(ch)) {
	    radix = 8;
	    limit = 3;
	    first--;
	} else {
	    value = *first;
	}
	break;
    }

    if (radix != 0) {
	char *last;
	int save = first[limit];
	first[limit] = '\0';
	value = strtol(first, &last, radix);
	first[limit] = save;
	first = last;
    }

    if (name != 0) {
	(void) expand_tiname(name, strlen(name), result);
    } else {
	**result = value;
	(*result) += 1;
    }

    return first;
}

PRIVATE void expand_substring (char* dst, char* first, char* last)
{
    int ch;
    while (first < last) {
	switch (ch = *first++) {
	case ESCAPE:
	    first = expand_tichar(first, &dst);
	    break;
	case '^':
	    ch = *first++;
	    if (ch == LPAREN) {
		char *s = strchr(first, RPAREN);
		if (s == 0)
		    s = first + strlen(first);
		first = expand_tiname(first, s-first, &dst);
	    } else if (ch == '?') {		/* ASCII delete? */
		*dst++ = 127;
	    } else if ((ch & 0x3f) < 0x20) {	/* ASCII control char? */
		*dst++ = (ch & 0x1f);
	    } else {
		*dst++ = '^';
		first--;	/* not legal... */
	    }
	    break;
	case 0:					/* convert nulls for terminfo */
	    ch = 0200;
	    /* FALLTHRU */
	default:
	    *dst++ = ch;
	    break;
	}
    }
    *dst = '\0';
}
#endif

PRIVATE void unescaped_char ARGS2(char*, parse, int*,keysym)
{
    size_t len = strlen(parse);
    char buf[BUFSIZ];

    if (len >= 3) {
	expand_substring(buf, parse + 1, parse + len - 1);
	if (strlen(buf) == 1)
	    *keysym = *buf;
    }
}

PRIVATE BOOLEAN unescape_string ARGS2(char*, src, char *, dst)
{
    BOOLEAN ok = FALSE;

    if (*src == SQUOTE) {
	int keysym;
	unescaped_char(src, &keysym);
	if (keysym >= 0) {
	    dst[0] = keysym;
	    dst[1] = '\0';
	    ok = TRUE;
	}
    } else if (*src == DQUOTE) {
	expand_substring(dst, src + 1, src + strlen(src) - 1);
	ok = TRUE;
    }
    return ok;
}

PRIVATE int map_string_to_keysym ARGS2(char*, str, int*,keysym)
{
    *keysym = -1;

    if (*str == SQUOTE) {
	unescaped_char(str, keysym);
    } else if (isdigit(*str)) {
	char *tmp;
	long value = strtol(str, &tmp, 0);
	if (!isalnum(*tmp))
	    *keysym = value;
    } else {
	Keysym_String_List *k;

	k = Keysym_Strings;
	while (k->string != NULL) {
	    if (0 == strcmp (k->string, str)) {
		*keysym = k->value;
		break;
	    }
	    k++;
	}
    }

    return (*keysym);
}

/*
 * Starting at a nonblank character, skip over a token, counting quoted and
 * escaped characters. 
 */
PRIVATE char *skip_keysym ARGS1(char *, parse)
{
    int quoted = 0;
    int escaped = 0;

    while (*parse) {
	if (escaped) {
	    escaped = 0;
	} else if (quoted) {
	    if (*parse == ESCAPE) {
		escaped = 1;
	    } else if (*parse == quoted) {
		quoted = 0;
	    }
	} else if (*parse == ESCAPE) {
	    escaped = 1;
	} else if (*parse == DQUOTE || *parse == SQUOTE) {
	    quoted = *parse;
	} else if (isspace(*parse)) {
	    break;
	}
	parse++;
    }
    return (quoted || escaped) ? 0 : parse;
}

/*
 * The first token is the string to define, the second is the name (of the
 * keysym) to define it to.
 */
PRIVATE int setkey_cmd (char *parse)
{
    char *s, *t;
    int keysym;
    char buf[BUFSIZ];

    if ((s = skip_keysym(parse)) != 0) {
	if (isspace(*s)) {
	    *s++ = '\0';
	    s = LYSkipBlanks(s);
	    if ((t = skip_keysym(s)) == 0)
		return -1;
	    if (t != s)
		*t = '\0';
	    if (map_string_to_keysym (s, &keysym) >= 0
	     && unescape_string(parse, buf)) {
		return define_key(buf, keysym);
	    }
	}
    }
    return -1;
}

PRIVATE int unsetkey_cmd (char *parse)
{
    char *s = skip_keysym(parse);
    if (s != parse) {
	*s = '\0';
#ifdef NCURSES_VERSION
	/*
	 * This won't work with Slang.  Remove the definition for the given
	 * keysym.
	 */
	{
	    int keysym;
	    if (map_string_to_keysym (parse, &keysym) >= 0)
		define_key((char *)0, keysym);
	}
#endif
#ifdef USE_SLANG
	/* Slang implements this, for undefining the string which is associated
	 * with a keysym (the reverse of what we normally want, but may
	 * occasionally find useful).
	 */
	SLang_undefine_key (parse, Keymap_List);
	if (SLang_Error) return -1;
#endif
    }
    return 0;
}

#ifdef FNAMES_8_3
#define FNAME_LYNX_KEYMAPS "_lynxkey.map"
#else
#define FNAME_LYNX_KEYMAPS ".lynx-keymaps"
#endif /* FNAMES_8_3 */

PRIVATE int read_keymap_file NOARGS
{
    static struct {
	CONST char *name;
	int (*func) PARAMS((char *s));
    } table[] = {
	{"setkey",   setkey_cmd },
	{"unsetkey", unsetkey_cmd },
    };

    char line[1024];
    FILE *fp;
    char file[LY_MAXPATH];
    int ret;
    int linenum;
    size_t n;

    LYAddPathToHome(file, sizeof(file), FNAME_LYNX_KEYMAPS);

    if ((fp = fopen (file, "r")) == 0)
	return 0;

    linenum = 0;
    ret = 0;
    while ((fgets (line, sizeof (line), fp) != 0) && (ret == 0))
    {
	char *s = LYSkipBlanks(line);

	linenum++;

	if ((*s == 0) || (*s == '#'))
	    continue;

	for (n = 0; n < TABLESIZE(table); n++) {
	    size_t len = strlen(table[n].name);
	    if (strlen(s) > len
	     && !strncmp(s, table[n].name, len)) {
		if ((*(table[n].func))(LYSkipBlanks(s+len)) < 0) {
		    ret = -1;
		    break;
		}
	    }
	}
    }

    fclose (fp);

    if (ret == -1)
	fprintf (stderr, "Error processing line %d of %s\n", linenum, file);

    return ret;
}

PRIVATE void setup_vtXXX_keymap NOARGS
{
    static Keysym_String_List table[] = {
	DEFINE_KEY( "\033[A",	UPARROW,	KEY_UP ),
	DEFINE_KEY( "\033OA",	UPARROW,	KEY_UP ),
	DEFINE_KEY( "\033[B",	DNARROW,	KEY_DOWN ),
	DEFINE_KEY( "\033OB",	DNARROW,	KEY_DOWN ),
	DEFINE_KEY( "\033[C",	RTARROW,	KEY_RIGHT ),
	DEFINE_KEY( "\033OC",	RTARROW,	KEY_RIGHT ),
	DEFINE_KEY( "\033[D",	LTARROW,	KEY_LEFT ),
	DEFINE_KEY( "\033OD",	LTARROW,	KEY_LEFT ),
	DEFINE_KEY( "\033[1~",	FIND_KEY,	KEY_FIND ),
	DEFINE_KEY( "\033[2~",	INSERT_KEY,	KEY_IC ),
	DEFINE_KEY( "\033[3~",	REMOVE_KEY,	KEY_DC ),
	DEFINE_KEY( "\033[4~",	SELECT_KEY,	KEY_SELECT ),
	DEFINE_KEY( "\033[5~",	PGUP,		KEY_PPAGE ),
	DEFINE_KEY( "\033[6~",	PGDOWN,		KEY_NPAGE ),
	DEFINE_KEY( "\033[8~",	END_KEY,	KEY_END ),
	DEFINE_KEY( "\033[7~",	HOME,		KEY_HOME),
	DEFINE_KEY( "\033[28~",	F1,		KEY_F(1) ),
	DEFINE_KEY( "\033OP",	F1,		KEY_F(1) ),
	DEFINE_KEY( "\033[OP",	F1,		KEY_F(1) ),
	DEFINE_KEY( "\033[29~",	DO_KEY,		KEY_F(16) ),
    };
    size_t n;
    for (n = 0; n < TABLESIZE(table); n++)
    	define_key(table[n].string, table[n].value);
}

PUBLIC int lynx_initialize_keymaps NOARGS
{
#ifdef USE_SLANG
    int i;
    char keybuf[2];

    if (NULL == (Keymap_List = SLang_create_keymap ("Lynx", NULL)))
	return -1;

    keybuf[1] = 0;
    for (i = 1; i < 256; i++)
    {
	keybuf[0] = (char) i;
	define_key (keybuf, i);
    }

    setup_vtXXX_keymap();
    define_key ("\033[M", MOUSE_KEYSYM);

    if (SLang_Error
    || (-1 == read_keymap_file ()))
    SLang_exit_error ("Unable to initialize keymaps");
    return 0;
#else
    setup_vtXXX_keymap();
    return read_keymap_file();
#endif
}

#endif				       /* USE_KEYMAPS */

#if defined(USE_KEYMAPS) && defined(USE_SLANG)

/* We cannot guarantee the type for 'GetChar', and should not use a cast. */
PRIVATE int myGetChar NOARGS
{
   return GetChar();
}

PUBLIC int LYgetch NOARGS
{
   SLang_Key_Type *key;
   int keysym;

   key = SLang_do_key (Keymap_List, myGetChar);
   if ((key == NULL) || (key->type != SLKEY_F_KEYSYM))
     return DO_NOTHING;

   keysym = key->f.keysym;

#if defined (USE_SLANG_MOUSE)
   if (keysym == MOUSE_KEYSYM)
     return sl_read_mouse_event ();
#endif

   if ((keysym > DO_NOTHING) || (keysym < 0))
     return 0;

   return keysym;
}

#else /* !USE_KEYMAPS */

/*
 *  LYgetch() translates some escape sequences and may fake noecho.
 */
#define found_CSI(first,second) ((second) == '[' || (first) == 155)

PUBLIC int LYgetch NOARGS
{
    int a, b, c, d = -1;

#if defined(IGNORE_CTRL_C) || defined(USE_GETCHAR) || !defined(NCURSES)
re_read:
#endif /* IGNORE_CTRL_C || USE_GETCHAR */
#ifndef USE_SLANG
    clearerr(stdin); /* needed here for ultrix and SOCKETSHR, but why? - FM */
#endif /* !USE_SLANG */
#if !defined(USE_SLANG) || defined(VMS) || defined(DJGPP_KEYHANDLER)
    c = GetChar();
#else
    if (LYCursesON) {
	c = GetChar();
    } else {
	c = getchar();
	if (c == EOF && errno == EINTR) /* Ctrl-Z causes EINTR in getchar() */
	    clearerr(stdin);
	if (feof(stdin) || ferror(stdin) || c == EOF) {
#ifdef IGNORE_CTRL_C
	    if (sigint)
		sigint = FALSE;
#endif /* IGNORE_CTRL_C */
	    return(7); /* use ^G to cancel whatever called us. */
	}
    }
#endif /* !USE_SLANG || VMS */

#ifdef USE_GETCHAR
    if (c == EOF && errno == EINTR)	/* Ctrl-Z causes EINTR in getchar() */
	goto re_read;
#endif /* USE_GETCHAR */

#ifdef USE_SLANG
    if (c == 0xFFFF && LYCursesON) {
#ifdef IGNORE_CTRL_C
	if (sigint) {
	    sigint = FALSE;
	    goto re_read;
	}
#endif /* IGNORE_CTRL_C */
	return(7); /* use ^G to cancel whatever called us. */
    }
#else
    if (feof(stdin) || ferror(stdin) || c == EOF) {
	if (recent_sizechange)
	    return(7); /* use ^G to cancel whatever called us. */
#ifdef IGNORE_CTRL_C
	if (sigint) {
	    sigint = FALSE;
	    /* clearerr(stdin);  don't need here if stays above - FM */
	    goto re_read;
	}
#endif /* IGNORE_CTRL_C */
#if !defined(USE_GETCHAR) && !defined(VMS) && !defined(NCURSES)
	if (c == ERR && errno == EINTR) /* may have been handled signal - kw */
	    goto re_read;
#endif /* USE_GETCHAR */

	cleanup();
#ifndef NOSIGHUP
	(void) signal(SIGHUP, SIG_DFL);
#endif /* NOSIGHUP */
	(void) signal(SIGTERM, SIG_DFL);
#ifndef VMS
	(void) signal(SIGINT, SIG_DFL);
#endif /* !VMS */
#ifdef SIGTSTP
	if (no_suspend)
	  (void) signal(SIGTSTP,SIG_DFL);
#endif /* SIGTSTP */
	exit(0);
    }
#endif /* USE_SLANG */

    if (c == 27 || (csi_is_csi && c == 155)) {	    /* handle escape sequence */
	b = GetChar();

	if (b == '[' || b == 'O') {
	    a = GetChar();
	} else {
	    a = b;
	}

	switch (a) {
	case 'A': c = UPARROW; break;
	case 'x': c = UPARROW; break;  /* keypad up on pc ncsa telnet */
	case 'B': c = DNARROW; break;
	case 'r': c = DNARROW; break; /* keypad down on pc ncsa telnet */
	case 'C': c = RTARROW; break;
	case 'v': c = RTARROW; break; /* keypad right on pc ncsa telnet */
	case 'D': c = LTARROW; break;
	case 't': c = LTARROW; break;  /* keypad left on pc ncsa telnet */
	case 'y': c = PGUP;    break;  /* keypad on pc ncsa telnet */
	case 's': c = PGDOWN;  break;  /* keypad on pc ncsa telnet */
	case 'w': c = HOME;    break;  /* keypad on pc ncsa telnet */
	case 'q': c = END_KEY; break;  /* keypad on pc ncsa telnet */
	case 'M':
#ifdef USE_SLANG_MOUSE
	   if (found_CSI(c,b))
	     {
		c = sl_read_mouse_event ();
	     }
	   else
#endif
	     c = '\n'; /* keypad enter on pc ncsa telnet */
	   break;

	case 'm':
#ifdef VMS
	    if (b != 'O')
#endif /* VMS */
		c = '-';  /* keypad on pc ncsa telnet */
	    break;
	case 'k':
	    if (b == 'O')
		c = '+';  /* keypad + on my xterminal :) */
	    break;
	case 'l':
#ifdef VMS
	    if (b != 'O')
#endif /* VMS */
		c = '+';  /* keypad on pc ncsa telnet */
	    break;
	case 'P':
#ifdef VMS
	    if (b != 'O')
#endif /* VMS */
		c = F1;
	    break;
	case 'u':
#ifdef VMS
	    if (b != 'O')
#endif /* VMS */
		c = F1;  /* macintosh help button */
	    break;
	case 'p':
#ifdef VMS
	    if (b == 'O')
#endif /* VMS */
		c = '0';  /* keypad 0 */
	    break;
	case '1':			    /** VTxxx  Find  **/
	    if (found_CSI(c,b) && (d=GetChar()) == '~')
		c = FIND_KEY;
	    break;
	case '2':
	    if (found_CSI(c,b)) {
		if ((d=GetChar())=='~')     /** VTxxx Insert **/
		    c = INSERT_KEY;
		else if ((d == '8' ||
			  d == '9') &&
			 GetChar() == '~')
		 {
		    if (d == '8')	     /** VTxxx	Help **/
			c = F1;
		    else if (d == '9')	     /** VTxxx	 Do  **/
			c = DO_KEY;
		    d = -1;
		 }
	    }
	    break;
	case '3':			     /** VTxxx Delete **/
	    if (found_CSI(c,b) && (d=GetChar()) == '~')
		c = REMOVE_KEY;
	    break;
	case '4':			     /** VTxxx Select **/
	    if (found_CSI(c,b) && (d=GetChar()) == '~')
		c = SELECT_KEY;
	    break;
	case '5':			     /** VTxxx PrevScreen **/
	    if (found_CSI(c,b) && (d=GetChar()) == '~')
		c = PGUP;
	    break;
	case '6':			     /** VTxxx NextScreen **/
	    if (found_CSI(c,b) && (d=GetChar()) == '~')
		c = PGDOWN;
	    break;
	case '[':			     /** Linux F1-F5: ^[[[A etc. **/
	    if (found_CSI(c,b)) {
		if ((d=GetChar()) == 'A')
		    c = F1;
		break;
	    }
	default:
	    CTRACE(tfp,"Unknown key sequence: %d:%d:%d\n",c,b,a);
	    CTRACE_SLEEP(MessageSecs);
	    break;
	}
	if (isdigit(a) && found_CSI(c,b) && d != -1 && d != '~')
	    d = GetChar();
    }
#if HAVE_KEYPAD
    else {
	/*
	 *  Convert keypad() mode keys into Lynx defined keys.
	 */
	switch(c) {
	case KEY_DOWN:		   /* The four arrow keys ... */
	   c = DNARROW;
	   break;
	case KEY_UP:
	   c = UPARROW;
	   break;
	case KEY_LEFT:
	   c = LTARROW;
	   break;
	case KEY_RIGHT: 	   /* ... */
	   c = RTARROW;
	   break;
	case KEY_HOME:		   /* Home key (upward+left arrow) */
	   c = HOME;
	   break;
	case KEY_CLEAR: 	   /* Clear screen */
	   c = 18; /* CTRL-R */
	   break;
	case KEY_NPAGE: 	   /* Next page */
	   c = PGDOWN;
	   break;
	case KEY_PPAGE: 	   /* Previous page */
	   c = PGUP;
	   break;
	case KEY_LL:		   /* home down or bottom (lower left) */
	   c = END_KEY;
	   break;
					/* The keypad is arranged like this:*/
					/*    a1    up	  a3   */
					/*   left   b2	right  */
					/*    c1   down   c3   */
	case KEY_A1:		   /* upper left of keypad */
	   c = HOME;
	   break;
	case KEY_A3:		   /* upper right of keypad */
	   c = PGUP;
	   break;
	case KEY_B2:		   /* center of keypad */
	   c = DO_NOTHING;
	   break;
	case KEY_C1:		   /* lower left of keypad */
	   c = END_KEY;
	   break;
	case KEY_C3:		   /* lower right of keypad */
	   c = PGDOWN;
	   break;
#ifdef KEY_END
	case KEY_END:		   /* end key		001 */
	   c = END_KEY;
	   break;
#endif /* KEY_END */
#ifdef KEY_HELP
	case KEY_HELP:		   /* help key		001 */
	   c = F1;
	   break;
#endif /* KEY_HELP */
#ifdef KEY_BACKSPACE
	case KEY_BACKSPACE:
	   c = 127;		   /* backspace key (delete, not Ctrl-H) */
	   break;
#endif /* KEY_BACKSPACE */
#if defined(KEY_F) && !defined(__DJGPP__) && !defined(_WINDOWS)
	case KEY_F(1):
	   c = F1;		   /* VTxxx Help */
	   break;
	case KEY_F(16):
	   c = DO_KEY;		   /* VTxxx Do */
	   break;
#endif /* KEY_F */
#ifdef KEY_REDO
	case KEY_REDO:		   /* VTxxx Do */
	   c = DO_KEY;
	   break;
#endif /* KEY_REDO */
#ifdef KEY_FIND
	case KEY_FIND:
	   c = FIND_KEY;	   /* VTxxx Find */
	   break;
#endif /* KEY_FIND */
#ifdef KEY_SELECT
	case KEY_SELECT:
	   c = SELECT_KEY;	   /* VTxxx Select */
	   break;
#endif /* KEY_SELECT */
#ifdef KEY_IC
	case KEY_IC:
	   c = INSERT_KEY;	   /* VTxxx Insert */
	   break;
#endif /* KEY_IC */
#ifdef KEY_DC
	case KEY_DC:
	   c = REMOVE_KEY;	   /* VTxxx Remove */
	   break;
#endif /* KEY_DC */
#ifdef NCURSES_MOUSE_VERSION
	case KEY_MOUSE:
	    {
#ifndef DOSPATH
		MEVENT event;
		int err;

		c = -1;
		mouse_link = -1;
		err = getmouse(&event);
		if (event.bstate & BUTTON1_CLICKED) {
		    c = set_clicked_link(event.x, event.y);
		} else if (event.bstate & BUTTON3_CLICKED) {
		    c = LYReverseKeymap (LYK_PREV_DOC);
		}
#else /* pdcurses version */
		c = -1;
		mouse_link = -1;
		request_mouse_pos();
		if (BUTTON_STATUS(1) & BUTTON_CLICKED) {
		    c = set_clicked_link(MOUSE_X_POS, MOUSE_Y_POS);
		} else if (BUTTON_STATUS(3) & BUTTON_CLICKED) {
		    c = LYReverseKeymap (LYK_PREV_DOC);
		}
#endif /* DOSPATH */
	    }
	    break;
#endif /* NCURSES_MOUSE_VERSION */
	}
    }
#endif /* HAVE_KEYPAD */
#ifdef DJGPP_KEYHANDLER
    else {
	switch(c) {
	case K_Down:		   /* The four arrow keys ... */
	case K_EDown:
	   c = DNARROW;
	   break;
	case K_Up:
	case K_EUp:
	   c = UPARROW;
	   break;
	case K_Left:
	case K_ELeft:
	   c = LTARROW;
	   break;
	case K_Right: 		   /* ... */
	case K_ERight:
	   c = RTARROW;
	   break;
	case K_Home:		   /* Home key (upward+left arrow) */
	case K_EHome:
	   c = HOME;
	   break;
	case K_PageDown: 	   /* Next page */
	case K_EPageDown:
	   c = PGDOWN;
	   break;
	case K_PageUp:	 	   /* Previous page */
	case K_EPageUp:
	   c = PGUP;
	   break;
	case K_End:		   /* home down or bottom (lower left) */
	case K_EEnd:
	   c = END_KEY;
	   break;
	}
    }
#endif /* DGJPP_KEYHANDLER */
#if defined(USE_SLANG) && defined(__DJGPP__) && !defined(DJGPP_KEYHANDLER)  && !defined(USE_KEYMAPS)
    else {
	switch(c) {
	case SL_KEY_DOWN:	   /* The four arrow keys ... */
	   c = DNARROW;
	   break;
	case SL_KEY_UP:
	   c = UPARROW;
	   break;
	case SL_KEY_LEFT:
	   c = LTARROW;
	   break;
	case SL_KEY_RIGHT: 	   /* ... */
	   c = RTARROW;
	   break;
	case SL_KEY_HOME:	   /* Home key (upward+left arrow) */
	case SL_KEY_A1:		   /* upper left of keypad */
	   c = HOME;
	   break;
	case SL_KEY_NPAGE: 	   /* Next page */
	case SL_KEY_C3:		   /* lower right of keypad */
	   c = PGDOWN;
	   break;
	case SL_KEY_PPAGE: 	   /* Previous page */
	case SL_KEY_A3:		   /* upper right of keypad */
	   c = PGUP;
	   break;
	case SL_KEY_END:	   /* home down or bottom (lower left) */
	case SL_KEY_C1:		   /* lower left of keypad */
	   c = END_KEY;
	   break;
	}
    }
#endif /* USE_SLANG && __DJGPP__ && !DJGPP_KEYHANDLER && !USE_KEYMAPS */

#if (defined(__DJGPP__) || defined(_WINDOWS))
    if (c > 659)
#else
    if (c > DO_NOTHING)
#endif /* __DJGPP__ || _WINDOWS */
    {
	/*
	 *  Don't return raw values for KEYPAD symbols which we may have
	 *  missed in the switch above if they are obviously invalid when
	 *  used as an index into (e.g.) keypad[]. - KW
	 */
	return (0);
    } else {
	return(c);
    }
}

#endif				       /* NOT USE_KEYMAPS */

/*
 * Convert a null-terminated string to lowercase
 */
PUBLIC void LYLowerCase ARGS1(
	char *, 	buffer)
{
    size_t i;
    for (i = 0; buffer[i]; i++)
	buffer[i] = TOLOWER(buffer[i]);
}

/*
 * Convert a null-terminated string to uppercase
 */
PUBLIC void LYUpperCase ARGS1(
	char *, 	buffer)
{
    size_t i;
    for (i = 0; buffer[i]; i++)
	buffer[i] = TOUPPER(buffer[i]);
}

/*
 * Remove ALL whitespace from a string (including embedded blanks).
 */
PUBLIC void LYRemoveBlanks ARGS1(
	char *, 	buffer)
{
    if (buffer != 0) {
	size_t i, j;
	for (i = j = 0; buffer[i]; i++)
	    if (!isspace((unsigned char)(buffer[i])))
		buffer[j++] = buffer[i];
	buffer[j] = 0;
    }
}

/*
 * Skip whitespace
 */
PUBLIC char * LYSkipBlanks ARGS1(
	char *, 	buffer)
{
    while (isspace((unsigned char)(*buffer)))
	buffer++;
    return buffer;
}

/*
 * Skip non-whitespace
 */
PUBLIC char * LYSkipNonBlanks ARGS1(
	char *, 	buffer)
{
    while (*buffer != 0 && !isspace((unsigned char)(*buffer)))
	buffer++;
    return buffer;
}

/*
 * Skip CONST whitespace
 */
PUBLIC CONST char * LYSkipCBlanks ARGS1(
	CONST char *,	buffer)
{
    while (isspace((unsigned char)(*buffer)))
	buffer++;
    return buffer;
}

/*
 * Skip CONST non-whitespace
 */
PUBLIC CONST char * LYSkipCNonBlanks ARGS1(
	CONST char *,	buffer)
{
    while (*buffer != 0 && !isspace((unsigned char)(*buffer)))
	buffer++;
    return buffer;
}

/*
 * Trim leading blanks from a string
 */
PUBLIC void LYTrimLeading ARGS1(
	char *, 	buffer)
{
    char *skipped = LYSkipBlanks(buffer);
    while ((*buffer++ = *skipped++) != 0)
	;
}

/*
 * Trim trailing blanks from a string
 */
PUBLIC void LYTrimTrailing ARGS1(
	char *, 	buffer)
{
    size_t i = strlen(buffer);
    while (i != 0 && isspace((unsigned char)buffer[i-1]))
	buffer[--i] = 0;
}

/*
**  Display the current value of the string and allow the user
**  to edit it.
*/

#define EDREC	 EditFieldData

/*
 *  Shorthand to get rid of all most of the "edit->suchandsos".
 */
#define Buf	 edit->buffer
#define Pos	 edit->pos
#define StrLen	 edit->strlen
#define MaxLen	 edit->maxlen
#define DspWdth  edit->dspwdth
#define DspStart edit->xpan
#define Margin	 edit->margin

PUBLIC void LYSetupEdit ARGS4(
	EDREC *,	edit,
	char *, 	old,
	int,		maxstr,
	int,		maxdsp)
{
    /*
     *	Initialize edit record
     */
    LYGetYX(edit->sy, edit->sx);
    edit->pad	= ' ';
    edit->dirty = TRUE;
    edit->panon = FALSE;

    StrLen  = strlen(old);
    MaxLen  = maxstr;
    DspWdth = maxdsp;
    Margin  = 0;
    Pos = strlen(old);
    DspStart = 0;

    if (maxstr > maxdsp) {  /* Need panning? */
	if (DspWdth > 4)    /* Else "{}" take up precious screen space */
	    edit->panon = TRUE;

	/*
	 *  Figure out margins.  If too big, we do a lot of unnecessary
	 *  scrolling.	If too small, user doesn't have sufficient
	 *  look-ahead.  Let's say 25% for each margin, upper bound is
	 *  10 columns.
	 */
	Margin = DspWdth/4;
	if (Margin > 10)
	    Margin = 10;
    }

    /*
     *	We expect the called function to pass us a default (old) value
     *	with a length that is less than or equal to maxstr, and to
     *	handle any messaging associated with actions to achieve that
     *	requirement.  However, in case the calling function screwed
     *	up, we'll check it here, and ensure that no buffer overrun can
     *	occur by loading only as much of the head as fits. - FM
     */
    if (strlen(old) >= maxstr) {
	strncpy(edit->buffer, old, maxstr);
	edit->buffer[maxstr] = '\0';
	StrLen = maxstr;
    } else {
	strcpy(edit->buffer, old);
    }
}

PUBLIC int LYEdit1 ARGS4(
	EDREC *,	edit,
	int,		ch,
	int,		action,
	BOOL,		maxMessage)
{   /* returns 0    character processed
     *	       ch   otherwise
     */
    int i;
    int length;

    if (MaxLen <= 0)
	return(0); /* Be defensive */

    length = strlen(&Buf[0]);
    StrLen = length;

    switch (action) {
    case LYE_AIX:
	/*
	 *  Hex 97.
	 *  Fall through as a character for CJK, or if this is a valid
	 *  character in the current display character set.
	 *  Otherwise, we treat this as LYE_ENTER.
	 */
	 if (HTCJK == NOCJK && LYlowest_eightbit[current_char_set] > 0x97)
	     return(ch);
    case LYE_CHAR:
	/*
	 *  ch is printable or ISO-8859-1 escape character.
	 */
	if (Pos <= (MaxLen) && StrLen < (MaxLen)) {
	    for(i = length; i >= Pos; i--)    /* Make room */
		Buf[i+1] = Buf[i];
	    Buf[length+1]='\0';
	    Buf[Pos] = (unsigned char) ch;
	    Pos++;
	} else if (maxMessage) {
	    _statusline(MAXLEN_REACHED_DEL_OR_MOV);
	}
	break;

    case LYE_BACKW:
	/*
	 *  Backword.
	 *  Definition of word is very naive: 1 or more a/n characters.
	 */
	while (Pos && !isalnum(Buf[Pos-1]))
	    Pos--;
	while (Pos &&  isalnum(Buf[Pos-1]))
	    Pos--;
	break;

    case LYE_FORWW:
	/*
	 *  Word forward.
	 */
	while (isalnum(Buf[Pos]))
	    Pos++;   /* '\0' is not a/n */
	while (!isalnum(Buf[Pos]) && Buf[Pos])
	    Pos++ ;
	break;

    case LYE_ERASE:
	/*
	 *  Erase the line to start fresh.
	 */
	 Buf[0] = '\0';
	 /* fall through */

    case LYE_BOL:
	/*
	 *  Go to first column.
	 */
	Pos = 0;
	break;

    case LYE_EOL:
	/*
	 *  Go to last column.
	 */
	Pos = length;
	break;

    case LYE_DELNW:
	/*
	 *  Delete next word.
	 */
	{
	    int pos0 = Pos;
	    LYEdit1 (edit, 0, LYE_FORWW, FALSE);
	    while (Pos > pos0)
		LYEdit1(edit, 0, LYE_DELP, FALSE);
	}
	break;

    case LYE_DELPW:
	/*
	 *  Delete previous word.
	 */
	{
	    int pos0 = Pos;
	    LYEdit1 (edit, 0, LYE_BACKW, FALSE);
	    pos0 -= Pos;
	    while (pos0--)
		LYEdit1(edit, 0, LYE_DELN, FALSE);
	}
	break;

    case LYE_DELN:
	/*
	 *  Delete next character
	 */
	if (Pos >= length)
	    break;
	Pos++;
	/* fall through */

    case LYE_DELP:
	/*
	 *  Delete preceding character.
	 */
	if (length == 0 || Pos == 0)
	    break;
	Pos--;
	for (i = Pos; i < length; i++)
	    Buf[i] = Buf[i+1];
	i--;
	Buf[i] = 0;
	break;

    case LYE_DELC:
	/*
	 *  Delete current character.
	 */
	if (length == 0 || Pos == length)
	    break;
	for (i = Pos; i < length; i++)
	    Buf[i] = Buf[i+1];
	i--;
	Buf[i] = 0;
	break;

    case LYE_FORW:
	/*
	 *  Move cursor to the right.
	 */
	if (Pos < length)
	    Pos++;
	break;

    case LYE_BACK:
	/*
	 *  Left-arrow move cursor to the left.
	 */
	if (Pos > 0)
	    Pos--;
	break;

    case LYE_UPPER:
	LYUpperCase(Buf);
	break;

    case LYE_LOWER:
	LYLowerCase(Buf);
	break;

    default:
	return(ch);
    }
    edit->dirty = TRUE;
    StrLen = strlen(&Buf[0]);
    return(0);
}


PUBLIC void LYRefreshEdit ARGS1(
	EDREC *,	edit)
{
    int i;
    int length;
    int nrdisplayed;
    int padsize;
    char *str;
    char buffer[3];

    buffer[0] = buffer[1] = buffer[2] = '\0';
    if (!edit->dirty || (DspWdth == 0))
	return;
    edit->dirty = FALSE;

    length=strlen(&Buf[0]);
    edit->strlen = length;
/*
 *  Now we have:
 *		  .--DspWdth---.
 *	+---------+=============+-----------+
 *	|	  |M	       M|	    |	(M=margin)
 *	+---------+=============+-----------+
 *	0	  DspStart		     length
 *
 *  Insertion point can be anywhere between 0 and stringlength.
 *  Figure out new display starting point.
 *
 *   The first "if" below makes Lynx scroll several columns at a time when
 *   extending the string. Looks awful, but that way we can keep up with
 *   data entry at low baudrates.
 */
    if ((DspStart + DspWdth) <= length)
	if (Pos >= (DspStart + DspWdth) - Margin)
	    DspStart=(Pos - DspWdth) + Margin;

    if (Pos < DspStart + Margin) {
	DspStart = Pos - Margin;
	if (DspStart < 0)
	    DspStart = 0;
    }

    str = &Buf[DspStart];

    nrdisplayed = length-DspStart;
    if (nrdisplayed > DspWdth)
	nrdisplayed = DspWdth;

    move(edit->sy, edit->sx);
    if (edit->hidden) {
	for (i = 0; i < nrdisplayed; i++)
	    addch('*');
    } else {
	for (i = 0; i < nrdisplayed; i++)
	    if ((buffer[0] = str[i]) == 1 || buffer[0] == 2 ||
		((unsigned char)buffer[0] == 160 &&
		 !(HTPassHighCtrlRaw || HTCJK != NOCJK ||
		   (LYCharSet_UC[current_char_set].enc != UCT_ENC_8859 &&
		    !(LYCharSet_UC[current_char_set].like8859
		      & UCT_R_8859SPECL))))) {
		addch(' ');
	    } else {
		/* For CJK strings, by Masanobu Kimura */
		if (HTCJK != NOCJK && !isascii(buffer[0])) {
		    if (i < (nrdisplayed - 1))
			buffer[1] = str[++i];
		    addstr(buffer);
		    buffer[1] = '\0';
		} else {
		    addstr(buffer);
		}
	    }
    }

    /*
     *	Erase rest of input area.
     */
    padsize = DspWdth-nrdisplayed;
    while (padsize--)
	addch((unsigned char)edit->pad);

    /*
     *	Scrolling indicators.
     */
    if (edit->panon) {
	if ((DspStart + nrdisplayed) < length) {
	    move(edit->sy, edit->sx+nrdisplayed-1);
	    addch('}');
	}
	if (DspStart) {
	    move(edit->sy, edit->sx);
	    addch('{');
	}
    }

    move(edit->sy, edit->sx + Pos - DspStart);
    refresh();
}


PUBLIC int LYgetstr ARGS4(
	char *, 	inputline,
	int,		hidden,
	size_t, 	bufsize,
	int,		recall)
{
    int x, y, MaxStringSize;
    int ch;
    EditFieldData MyEdit;

    LYGetYX(y, x);		/* Use screen from cursor position to eol */
    MaxStringSize = (bufsize < sizeof(MyEdit.buffer)) ?
		    (bufsize - 1) : (sizeof(MyEdit.buffer) - 1);
    LYSetupEdit(&MyEdit, inputline, MaxStringSize, (LYcols-1)-x);
    MyEdit.hidden = hidden ;

    for (;;) {
again:
	LYRefreshEdit(&MyEdit);
	ch = LYgetch();
#ifdef VMS
	if (term_letter || term_options || term_message || HadVMSInterrupt) {
	    HadVMSInterrupt = FALSE;
	    ch = 7;
	}
#else
	if (term_letter || term_options || term_message)
	    ch = 7;
#endif /* VMS */
	if (recall && (ch == UPARROW || ch == DNARROW)) {
	    strcpy(inputline, MyEdit.buffer);
	    return(ch);
	}
	if (keymap[ch + 1] == LYK_REFRESH)
	    goto again;
	switch (EditBinding(ch)) {
	case LYE_TAB:
	    ch = '\t';
	    /* fall through */
	case LYE_AIX:
	    /*
	     *	Hex 97.
	     *	Treat as a character for CJK, or if this is a valid
	     *	character in the current display character set.
	     *	Otherwise, we treat this as LYE_ENTER.
	     */
	    if (ch != '\t' &&
		(HTCJK != NOCJK ||
		 LYlowest_eightbit[current_char_set] <= 0x97)) {
		LYLineEdit(&MyEdit,ch, FALSE);
		break;
	    }
	case LYE_ENTER:
	    /*
	     *	Terminate the string and return.
	     */
	    strcpy(inputline, MyEdit.buffer);
	    return(ch);

	case LYE_ABORT:
	    /*
	     *	Control-C or Control-G aborts.
	     */
	    inputline[0] = '\0';
	    return(-1);

	case LYE_LKCMD:
	    /*
	     *	Used only in form_getstr() for invoking
	     *	the LYK_F_LINK_NUM prompt when in form
	     *	text fields. - FM
	     */
	    break;

	default:
	    LYLineEdit(&MyEdit, ch, FALSE);
	}
    }
}

/*
 *  A replacement for 'strsep()'
 */
PUBLIC char *LYstrsep ARGS2(
	char **,	stringp,
	CONST char *,	delim)
{
    char *tmp, *out;

    if (!stringp || !*stringp)		/* nothing to do? */
	return 0;			/* then don't fall on our faces */

    if (!**stringp) {			/* empty string: */
	*stringp = 0;			/* let caller see he's done; */
	return 0;			/* no tokens in an empty string */
    }

    out = *stringp;			/* save the start of the string */
    tmp = strpbrk(*stringp, delim);
    if (tmp) {
	*tmp = '\0';			/* terminate the substring with \0 */
	*stringp = ++tmp;		/* point at the next substring */
    }
    else *stringp = 0;			/* this was the last substring: */
					/* let caller see he's done */
    return out;
}

/*
 *  LYstrstr will find the first occurrence of the string
 *  pointed to by tarptr in the string pointed to by chptr.
 *  It returns NULL if string not found.
 *  It is a case insensitive search.
 */
PUBLIC char * LYstrstr ARGS2(
	char *, 	chptr,
	CONST char *, 	tarptr)
{
    int len = strlen(tarptr);

    for(; *chptr != '\0'; chptr++) {
	if (0 == UPPER8(*chptr, *tarptr)) {
	    if (0 == strncasecomp8(chptr+1, tarptr+1, len-1))
		return(chptr);
	}
    } /* end for */

    return(NULL); /* string not found or initial chptr was empty */
}

/*
 *  LYno_attr_char_case_strstr will find the first occurrence of the
 *  string pointed to by tarptr in the string pointed to by chptr.
 *  It ignores the characters: LY_UNDERLINE_START_CHAR and
 *			       LY_UNDERLINE_END_CHAR
 *			       LY_BOLD_START_CHAR
 *			       LY_BOLD_END_CHAR
 *			       LY_SOFT_HYPHEN
 *			       if present in chptr.
 *  It is a case insensitive search.
 */
PUBLIC char * LYno_attr_char_case_strstr ARGS2(
	char *, 	chptr,
	char *, 	tarptr)
{
    register char *tmpchptr, *tmptarptr;

    if (!chptr)
	return(NULL);

    while (IsSpecialAttrChar(*chptr) && *chptr != '\0')
	chptr++;

    for (; *chptr != '\0'; chptr++) {
	 if (0 == UPPER8(*chptr, *tarptr)) {
	    /*
	     *	See if they line up.
	     */
	    tmpchptr = chptr+1;
	    tmptarptr = tarptr+1;

	    if (*tmptarptr == '\0')  /* one char target */
		 return(chptr);

	    while (1) {
		if (!IsSpecialAttrChar(*tmpchptr)) {
		    if (0 != UPPER8(*tmpchptr, *tmptarptr))
			break;
		    tmpchptr++;
		    tmptarptr++;
		} else {
		    tmpchptr++;
		}
		if (*tmptarptr == '\0')
		    return(chptr);
		if (*tmpchptr == '\0')
		    break;
	    }
	}
    } /* end for */

    return(NULL);
}

/*
 *  LYno_attr_char_strstr will find the first occurrence of the
 *  string pointed to by tarptr in the string pointed to by chptr.
 *  It ignores the characters: LY_UNDERLINE_START_CHAR and
 *			       LY_UNDERLINE_END_CHAR
 *			       LY_BOLD_START_CHAR
 *			       LY_BOLD_END_CHAR
 *				LY_SOFT_HYPHEN
 *			       if present in chptr.
 *  It is a case sensitive search.
 */
PUBLIC char * LYno_attr_char_strstr ARGS2(
	char *, 	chptr,
	char *, 	tarptr)
{
    register char *tmpchptr, *tmptarptr;

    if (!chptr)
	return(NULL);

    while (IsSpecialAttrChar(*chptr) && *chptr != '\0')
	chptr++;

    for (; *chptr != '\0'; chptr++) {
	if ((*chptr) == (*tarptr)) {
	    /*
	     *	See if they line up.
	     */
	    tmpchptr = chptr + 1;
	    tmptarptr = tarptr + 1;

	    if (*tmptarptr == '\0')  /* one char target */
		 return(chptr);

	    while (1) {
		 if (!IsSpecialAttrChar(*tmpchptr)) {
		    if ((*tmpchptr) != (*tmptarptr))
			break;
		    tmpchptr++;
		    tmptarptr++;
		 } else {
		    tmpchptr++;
		 }
		 if (*tmptarptr == '\0')
		     return(chptr);
		 if (*tmpchptr == '\0')
		     break;
	    }
	}
    } /* end for */

    return(NULL);
}

/*
 * LYno_attr_mbcs_case_strstr will find the first occurrence of the string
 * pointed to by tarptr in the string pointed to by chptr.
 * It takes account of MultiByte Character Sequences (UTF8).
 * The physical length of the displayed string up to the end of the target
 * string is returned in *nendp if the search is successful.
 * It ignores the characters: LY_UNDERLINE_START_CHAR and
 *			      LY_UNDERLINE_END_CHAR
 *			      LY_BOLD_START_CHAR
 *				LY_BOLD_END_CHAR
 *				LY_SOFT_HYPHEN
 *			      if present in chptr.
 * It assumes UTF8 if utf_flag is set.
 *  It is a case insensitive search. - KW & FM
 */
PUBLIC char * LYno_attr_mbcs_case_strstr ARGS5(
	char *, 	chptr,
	char *, 	tarptr,
	BOOL,		utf_flag,
	int *,		nstartp,
	int *,		nendp)
{
    register char *tmpchptr, *tmptarptr;
    int len = 0;
    int offset;

    if (!(chptr && tarptr))
	return(NULL);

    /*
     *	Skip initial IsSpecial chars. - FM
     */
    while (IsSpecialAttrChar(*chptr) && *chptr != '\0')
	chptr++;

    /*
     *	Seek a first target match. - FM
     */
    for (; *chptr != '\0'; chptr++) {
	if ((!utf_flag && HTCJK != NOCJK && !isascii(*chptr) &&
	     *chptr == *tarptr &&
	     *(chptr + 1) != '\0' &&
	     !IsSpecialAttrChar(*(chptr + 1))) ||
	    (0 == UPPER8(*chptr, *tarptr))) {
	    int tarlen = 0;
	    offset = len;
	    len++;

	    /*
	     *	See if they line up.
	     */
	    tmpchptr = (chptr + 1);
	    tmptarptr = (tarptr + 1);

	    if (*tmptarptr == '\0') {
		/*
		 *  One char target.
		 */
		*nstartp = offset;
		*nendp = len;
		 return(chptr);
	    }
	    if (!utf_flag && HTCJK != NOCJK && !isascii(*chptr) &&
		 *chptr == *tarptr &&
		 *tmpchptr != '\0' &&
		 !IsSpecialAttrChar(*tmpchptr)) {
		/*
		 *  Check the CJK multibyte. - FM
		 */
		if (*tmpchptr == *tmptarptr) {
		    /*
		     *	It's a match.  Advance to next char. - FM
		     */
		    tmpchptr++;
		    tmptarptr++;
		    if (*tmptarptr == '\0') {
			/*
			 *  One character match. - FM
			 */
			*nstartp = offset;
			*nendp = len + tarlen;
			return(chptr);
		    }
		    tarlen++;
		} else {
		    /*
		     *	It's not a match, so go back to
		     *	seeking a first target match. - FM
		     */
		    chptr++;
		    continue;
		}
	    }
	    /*
	     *	See if the rest of the target matches. - FM
	     */
	    while (1) {
		 if (!IsSpecialAttrChar(*tmpchptr)) {
		    if (!utf_flag && HTCJK != NOCJK && !isascii(*tmpchptr)) {
			if (*tmpchptr == *tmptarptr &&
			    *(tmpchptr + 1) == *(tmptarptr + 1) &&
			    !IsSpecialAttrChar(*(tmpchptr + 1))) {
			    tmpchptr++;
			    tmptarptr++;
			} else {
			break;
			}
		    } else if (0 != UPPER8(*tmpchptr, *tmptarptr)) {
			break;
		    }

		    if (!IS_UTF_EXTRA(*tmptarptr)) {
			tarlen++;
		    }
		    tmpchptr++;
		    tmptarptr++;

		 } else {
		    tmpchptr++;
		 }

		 if (*tmptarptr == '\0') {
		    *nstartp = offset;
		     *nendp = len + tarlen;
		     return(chptr);
		 }
		if (*tmpchptr == '\0') {
		     break;
	    }
	    }
	} else if (!(IS_UTF_EXTRA(*chptr) ||
		      IsSpecialAttrChar(*chptr))) {
	    if (!utf_flag && HTCJK != NOCJK && !isascii(*chptr) &&
		*(chptr + 1) != '\0' &&
		!IsSpecialAttrChar(*(chptr + 1))) {
		chptr++;
	    }
	    len++;
	}
    } /* end for */

    return(NULL);
}

/*
 * LYno_attr_mbcs_strstr will find the first occurrence of the string
 * pointed to by tarptr in the string pointed to by chptr.
 *  It takes account of CJK and MultiByte Character Sequences (UTF8).
 *  The physical lengths of the displayed string up to the start and
 *  end of the target string are returned in *nstartp and *nendp if
 *  the search is successful.
 * It ignores the characters: LY_UNDERLINE_START_CHAR and
 *			      LY_UNDERLINE_END_CHAR
 *			      LY_BOLD_START_CHAR
 *			      LY_BOLD_END_CHAR
 *				LY_SOFT_HYPHEN
 *			      if present in chptr.
 * It assumes UTF8 if utf_flag is set.
 *  It is a case sensitive search. - KW & FM
 */
PUBLIC char * LYno_attr_mbcs_strstr ARGS5(
	char *, 	chptr,
	char *, 	tarptr,
	BOOL,		utf_flag,
	int *,		nstartp,
	int *,		nendp)
{
    register char *tmpchptr, *tmptarptr;
    int len = 0;
    int offset;

    if (!(chptr && tarptr))
	return(NULL);

    /*
     *	Skip initial IsSpecial chars. - FM
     */
    while (IsSpecialAttrChar(*chptr) && *chptr != '\0')
	chptr++;

    /*
     *	Seek a first target match. - FM
     */
    for (; *chptr != '\0'; chptr++) {
	if ((*chptr) == (*tarptr)) {
	    int tarlen = 0;
	    offset = len;
	    len++;

	    /*
	     *	See if they line up.
	     */
	    tmpchptr = (chptr + 1);
	    tmptarptr = (tarptr + 1);

	    if (*tmptarptr == '\0') {
		/*
		 *  One char target.
		 */
		*nstartp = offset;
		*nendp = len + 1;
		 return(chptr);
	    }
	    if (!utf_flag && HTCJK != NOCJK && !isascii(*chptr) &&
		 *tmpchptr != '\0' &&
		 !IsSpecialAttrChar(*tmpchptr)) {
		/*
		 *  Check the CJK multibyte. - FM
		 */
		if (*tmpchptr == *tmptarptr) {
		    /*
		     *	It's a match.  Advance to next char. - FM
		     */
		    tmpchptr++;
		    tmptarptr++;
		    if (*tmptarptr == '\0') {
			/*
			 *  One character match. - FM
			 */
			*nstartp = offset;
			*nendp = len + tarlen;
			return(chptr);
		    }
		    tarlen++;
		} else {
		    /*
		     *	It's not a match, so go back to
		     *	seeking a first target match. - FM
		     */
		    chptr++;
		    continue;
		}
	    }
	    /*
	     *	See if the rest of the target matches. - FM
	     */
	    while (1) {
		 if (!IsSpecialAttrChar(*tmpchptr)) {
		    if (!utf_flag && HTCJK != NOCJK && !isascii(*tmpchptr)) {
			if (*tmpchptr == *tmptarptr &&
			    *(tmpchptr + 1) == *(tmptarptr + 1) &&
			    !IsSpecialAttrChar(*(tmpchptr + 1))) {
			    tmpchptr++;
			    tmptarptr++;
			} else {
			    break;
			}
		    } else if ((*tmpchptr) != (*tmptarptr)) {
			break;
		    }

		    if (!IS_UTF_EXTRA(*tmptarptr)) {
			tarlen++;
		    }
		    tmpchptr++;
		    tmptarptr++;
		 } else {
		    tmpchptr++;
		 }

		 if (*tmptarptr == '\0') {
		    *nstartp = offset;
		     *nendp = len + tarlen;
		     return(chptr);
		 }
		if (*tmpchptr == '\0') {
		     break;
	    }
	    }
	} else if (!(IS_UTF_EXTRA(*chptr) ||
		      IsSpecialAttrChar(*chptr))) {
	    if (!utf_flag && HTCJK != NOCJK && !isascii(*chptr) &&
		*(chptr + 1) != '\0' &&
		!IsSpecialAttrChar(*(chptr + 1))) {
		chptr++;
	    }
	    len++;
	}
    } /* end for */

    return(NULL);
}

/*
 *  Allocate a new copy of a string, and returns it.
 */
PUBLIC char * SNACopy ARGS3(
	char **,	dest,
	CONST char *,	src,
	int,		n)
{
    FREE(*dest);
    if (src) {
	*dest = (char *)calloc(1, n + 1);
	if (*dest == NULL) {
	    CTRACE(tfp, "Tried to calloc %d bytes\n", n);
	    outofmem(__FILE__, "SNACopy");
	}
	strncpy (*dest, src, n);
	*(*dest + n) = '\0'; /* terminate */
    }
    return *dest;
}

/*
 *  String Allocate and Concatenate.
 */
PUBLIC char * SNACat ARGS3(
	char **,	dest,
	CONST char *,	src,
	int,		n)
{
    if (src && *src) {
	if (*dest) {
	    int length = strlen(*dest);
	    *dest = (char *)realloc(*dest, length + n + 1);
	    if (*dest == NULL)
		outofmem(__FILE__, "SNACat");
	    strncpy(*dest + length, src, n);
	    *(*dest + length + n) = '\0'; /* terminate */
	} else {
	    *dest = (char *)calloc(1, strlen(src) + 1);
	    if (*dest == NULL)
		outofmem(__FILE__, "SNACat");
	    strncpy(*dest, src, n);
	    *dest[n] = '\0'; /* terminate */
	}
    }
    return *dest;
}


/*
**   UPPER8 ?
**   it was "TOUPPER(a) - TOUPPER(b)" in its previous life...
**
**   It was realized that case-insensitive user search
**   got information about upper/lower mapping from TOUPPER
**   (precisely from "(TOUPPER(a) - TOUPPER(b))==0").
**   This function depends on locale in its 8bit mapping
**   and usually fails with DOS/WINDOWS display charsets
**   as well as on non-UNIX systems.
**
**   We extend this function for 8bit letters
**   using Lynx internal chartrans feature:
**   we assume that upper/lower case letters
**   have their "7bit approximation" images (in def7_uni.tbl)
**   matched case-insensitive (7bit).
**
**   By this technique we automatically cover *any* charset
**   known for Lynx chartrans and need no any extra information for it.
**
**   The cost of this assumption is that several differently accented letters
**   may be interpreted as equal, but this side effect is negligible
**   if the user search string is more than one character long.  - LP
**
**   We enable new technique only if  DisplayCharsetMatchLocale = FALSE
**   (see description in LYCharSets.c)
*/
PUBLIC int UPPER8 ARGS2(int,ch1, int,ch2)
{

    /* case-insensitive match for us-ascii */
    if ((unsigned char)ch1 < 128 && (unsigned char)ch2 < 128)
	return(TOUPPER(ch1) - TOUPPER(ch2));

    /* case-insensitive match for upper half */
    if ((unsigned char)ch1 > 127 && (unsigned char)ch2 >127)
    {
	if (DisplayCharsetMatchLocale)
	   return(TOUPPER(ch1) - TOUPPER(ch2)); /* old-style */
	else
	{
	/* compare "7bit approximation" for letters >127   */
	/* BTW, if we remove the check for >127 above	   */
	/* we get even more "relaxed" insensitive match... */

	int charset_in, charset_out, uck1, uck2;
	char replace_buf1 [10], replace_buf2 [10];

	charset_in  = current_char_set;  /* display character set */
	charset_out = UCGetLYhndl_byMIME("us-ascii");

	uck1 = UCTransCharStr(replace_buf1, sizeof(replace_buf1), ch1,
			      charset_in, charset_out, YES);
	uck2 = UCTransCharStr(replace_buf2, sizeof(replace_buf2), ch2,
			      charset_in, charset_out, YES);

	if ((uck1 > 0) && (uck2 > 0))  /* both replacement strings found */
	    return (strcasecomp(replace_buf1, replace_buf2));

	/* check to be sure we have not lost any strange characters */
	/* which are not found in def7_uni.tbl but _equal_ in fact. */
	/* this also applied for "x-transparent" display mode.	    */
	if ((unsigned char)ch1==(unsigned char)ch2)
	    return(0);	 /* match */
	}
    }

    return(-10);  /* mismatch, if we come to here */
}
