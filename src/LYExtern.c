/*
 External application support.
 This feature allows lynx to pass a given URL to an external program.
 It was written for three reasons.
 1) To overcome the deficiency	of Lynx_386 not supporting ftp and news.
    External programs can be used instead by passing the URL.

 2) To allow for background transfers in multitasking systems.
    I use wget for http and ftp transfers via the external command.

 3) To allow for new URLs to be used through lynx.
    URLs can be made up such as mymail: to spawn desired applications
    via the external command.

 See lynx.cfg for other info.
*/

#include <LYUtils.h>

#ifdef USE_EXTERNALS

#include <HTAlert.h>
#include <LYGlobalDefs.h>
#include <LYExtern.h>
#include <LYLeaks.h>
#include <LYCurses.h>
#include <LYStrings.h>


#ifdef WIN_EX
/* 1997/10/15 (Wed) 17:39:50 */

#ifndef PATH_MAX
#define PATH_MAX	1024
#endif

#define STRING_MAX	512

/* ASCII char -> HEX digit */
#define ASC2HEXD(x) (((x) >= '0' && (x) <= '9') ?               \
		     ((x) - '0') : (toupper(x) - 'A' + 10))


/* Decodes the forms %xy in a URL to the character the hexadecimal
   code of which is xy. xy are hexadecimal digits from
   [0123456789ABCDEF] (case-insensitive). If x or y are not hex-digits
   or '%' is near '\0', the whole sequence is inserted literally. */


static char *decode_string(char *s)
{
    char *save_s;
    char *p = s;

    save_s = s;
    for (; *s; s++, p++) {
	if (*s != '%')
	    *p = *s;
	else {
	    /* Do nothing if at the end of the string. Or if the chars
	       are not hex-digits. */
	    if (!*(s + 1) || !*(s + 2)
		|| !(isxdigit(*(s + 1)) && isxdigit(*(s + 2)))) {
		*p = *s;
		continue;
	    }
	    *p = (char)((ASC2HEXD(*(s + 1)) << 4) + ASC2HEXD(*(s + 2)));
	    s += 2;
	}
    }
    *p = '\0';
    return save_s;
}
#endif	/* WIN_EX */

#ifndef STRING_MAX
#define	STRING_MAX 512
#endif

/* 1997/11/10 (Mon) 14:26:10 */
PUBLIC char *string_short ARGS2(
	char *,		str,
	int,		cut_pos)
{
    char buff[STRING_MAX], *s, *d;
    static char s_str[STRING_MAX];
    int len;

    LYstrncpy(buff, str, sizeof(buff)-1);
    len = strlen(buff);
    if (len > (LYcols - 10)) {
	buff[cut_pos] = '.';
	buff[cut_pos + 1] = '.';
	for (s = (buff + len) - (LYcols - 10) + cut_pos + 1,
	     d = (buff + cut_pos) + 2;
	     s >= buff &&
	     d >= buff &&
	     d < buff + LYcols &&
	     (*d++ = *s++) != 0; )
	    ;
	buff[LYcols] = 0;
    }
    strcpy(s_str, buff);
    return (s_str);
}

#ifdef WIN_EX
/*
 *  Quote the path to make it safe for shell command processing.
 *
 *  We use a simple technique which involves quoting the entire
 *  string using single quotes, escaping the real single quotes
 *  with double quotes. This may be gross but it seems to work.
 */
PUBLIC char * quote_pathname ARGS1(
	char *, 	pathname)
{
    char * result = NULL;

    if (strchr(pathname, ' ') != NULL) {
	HTSprintf0(&result, "\"%s\"", pathname);
    } else {
	StrAllocCopy(result, pathname);
    }
    return result;
}
#endif /* WIN_EX */

#if 0	/* old version */
void run_external_ ARGS1(char *, cmd)
{
    char *the_command = 0;
    lynx_html_item_type *ext = 0;

    for (ext = externals; ext != NULL; ext = ext->next) {

	if (ext->command != 0
	&& !strncasecomp(ext->name, cmd, strlen(ext->name))) {

	    if (no_externals && !ext->always_enabled) {
		HTUserMsg(EXTERNALS_DISABLED);
	    } else {

		HTAddParam(&the_command, ext->command, 1, cmd);
		HTEndParam(&the_command, ext->command, 1);

		HTUserMsg(the_command);

		stop_curses();
		LYSystem(the_command);
		FREE(the_command);
		start_curses();
	    }

	    break;
	}
    }

    return;
}
#endif

PRIVATE void format ARGS3(
    char **,	result,
    char *,	fmt,
    char *,	parm)
{
    *result = NULL;
    HTAddParam(result, fmt, 1, parm);
    HTEndParam(result, fmt, 1);
}

void run_external ARGS1(char *, c)
{
#ifdef WIN_EX
    HANDLE handle;
    int status;
    int confirmed;
    char pram_string[PATH_MAX];
    int redraw_flag;
    extern int xsystem(char *cmd);
#endif
    char *cmdbuf = NULL;
    lynx_html_item_type *externals2 = 0;

    if (externals == NULL)
	return;

#ifdef WIN_EX			/* 1998/01/26 (Mon) 09:16:13 */
    if (c == NULL) {
	HTInfoMsg("Not external command exists");
	return;
    }
#endif

    for (externals2 = externals; externals2 != NULL;
	 externals2 = externals2->next) {

#ifdef WIN_EX
	handle = GetForegroundWindow();
	CTRACE((tfp, "EXTERNAL: '%s' <==> '%s'\n", externals2->name, c));
#endif
	if (externals2->command != 0
	  && !strncasecomp(externals2->name, c, strlen(externals2->name)))
	{
	    if (no_externals && !externals2->always_enabled) {
		HTUserMsg(EXTERNALS_DISABLED);
		break;
	    }
	    /*  Too dangerous to leave any URL that may come along unquoted.
	     *  They often contain '&', ';', and '?' chars, and who knows
	     *  what else may occur.
	     *  Prevent spoofing of the shell.
	     *  Dunno how this needs to be modified for VMS or DOS. - kw
	     */
#if (defined(VMS) || defined(DOSPATH) || defined(__EMX__)) && !defined(WIN_EX)
	    format(&cmdbuf, externals2->command, c);
#else	/* Unix or DOS/Win: */
#if defined(WIN_EX)
	    if (*c != '\"' && strchr(c, ' ') != NULL) {
		char *cp = quote_pathname(c);
		format(&cmdbuf, externals2->command, cp);
		FREE(cp);
	    } else {
		LYstrncpy(pram_string, c, sizeof(pram_string)-1);
		decode_string(pram_string);
		c = pram_string;

		if (strnicmp("mailto:", c, 7) == 0) {
		    format(&cmdbuf, externals2->command, c + 7);
		} else if (strnicmp("telnet://", c, 9) == 0) {
		    char host[sizeof(pram_string)];
		    int last_pos;

		    strcpy(host, c + 9);
		    last_pos = strlen(host) - 1;
		    if (last_pos > 1 && host[last_pos] == '/')
			host[last_pos] = '\0';

		    format(&cmdbuf, externals2->command, host);
		} else if (strnicmp("file://localhost/", c, 17) == 0) {
		    char e_buff[PATH_MAX], *p;

		    p = c + 17;
		    *e_buff = 0;
		    if (strchr(p, ':') == NULL) {
			sprintf(e_buff, "%.3s/", windows_drive);
		    }
		    strncat(e_buff, p, sizeof(e_buff) - strlen(e_buff) - 1);
		    p = strrchr(e_buff, '.');
		    if (p) {
			p = strchr(p, '#');
			if (p) {
			    *p = '\0';
			}
		    }
		    if (*e_buff != '\"' && strchr(e_buff, ' ') != NULL) {
			p = quote_pathname(e_buff);
			LYstrncpy(e_buff, p, sizeof(e_buff)-1);
			FREE(p);
		    }

		    /* Less ==> short filename,
		     * less ==> long filename
		     */
		    if (isupper(externals2->command[0])) {
			format(&cmdbuf,
				externals2->command, HTDOS_short_name(e_buff));
		    } else {
			format(&cmdbuf, externals2->command, e_buff);
		    }
		} else {
		    format(&cmdbuf, externals2->command, c);
		}
	    }
#else	/* Unix */
	    {
		format(&cmdbuf, externals2->command, c);
	    }
#endif
#endif	/* VMS */

	    if (cmdbuf != 0 && *cmdbuf != '\0') {
#ifdef WIN_EX			/* 1997/10/17 (Fri) 14:07:50 */
		int len;
		char buff[PATH_MAX];

		CTRACE((tfp, "Lynx EXTERNAL: '%s'\n", cmdbuf));
#ifdef WIN_GUI			/* 1997/11/06 (Thu) 14:17:15 */
		confirmed = MessageBox(handle, cmdbuf,
				  "Lynx (EXTERNAL COMMAND EXEC)",
		       MB_ICONQUESTION | MB_SETFOREGROUND | MB_OKCANCEL)
			    == IDCANCEL;
#else
		confirmed = HTConfirm(string_short(cmdbuf, 40)) == NO;
#endif
		if (confirmed)
		    break;

		len = strlen(cmdbuf);
		if (len > 255) {
		    sprintf(buff, "Lynx: command line too long (%d > 255)", len);
#ifdef WIN_GUI			/* 1997/11/06 (Thu) 14:17:02 */
		    MessageBox(handle, buff,
			       "Lynx (EXTERNAL COMMAND EXEC)",
			  MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_OK);
		    SetConsoleTitle("Lynx for Win32");
#else
		    HTConfirm(string_short(buff, 40));
#endif
		    break;
		} else {
		    SetConsoleTitle(cmdbuf);
		}

		if (strnicmp(cmdbuf, "start ", 6) == 0)
		    redraw_flag = FALSE;
		else
		    redraw_flag = TRUE;

		if (redraw_flag) {
		    stop_curses();
		    fflush(stdout);
		}
#else
		HTUserMsg(cmdbuf);
		stop_curses();
#endif

		/* command running. */
#ifdef WIN_EX			/* 1997/10/17 (Fri) 14:07:50 */
#ifdef __CYGWIN__
		status = system(cmdbuf);
#else
		status = xsystem(cmdbuf);
#endif
		if (status != 0) {
		    sprintf(buff,
			    "EXEC code = %04x (%2d, %2d)\r\n"
			    "'%s'",
			    status, (status / 256), (status & 0xff),
			    cmdbuf);
#ifdef SH_EX	/* WIN_GUI for ERROR only */
		    MessageBox(handle, buff,
			       "Lynx (EXTERNAL COMMAND EXEC)",
			       MB_ICONSTOP | MB_SETFOREGROUND | MB_OK);
#else
		    HTConfirm(string_short(buff, 40));
#endif		/* 1 */
		}
#else	/* Not WIN_EX */
		LYSystem(cmdbuf);
#endif	/* WIN_EX */

#if defined(WIN_EX)
		SetConsoleTitle("Lynx for Win32");
#endif

#ifdef WIN_EX
		if (redraw_flag) {
		    fflush(stdout);
		    start_curses();
		}
#else
		fflush(stdout);
		start_curses();
#endif
	    }
	    break;
	} /* end if */
    } /* end-for */

    FREE(cmdbuf);
    return;
}
#endif	/* USE_EXTERNALS */
