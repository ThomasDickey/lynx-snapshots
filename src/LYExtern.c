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
	    *p = (ASC2HEXD(*(s + 1)) << 4) + ASC2HEXD(*(s + 2));
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
    char buff[STRING_MAX];
    static char s_str[STRING_MAX];
    char *p;
    int len;

    p = str;
    len = strlen(p);

    if (len > STRING_MAX) {
	strncpy(buff, p, STRING_MAX - 1);
	buff[STRING_MAX - 1] = '\0';
	len = STRING_MAX - 1;
    } else {
	strcpy(buff, p);
    }
    if (len > (LYcols - 10)) {
	buff[cut_pos] = '.';
	buff[cut_pos + 1] = '.';
	strcpy(buff + cut_pos + 2, (buff + len) - (LYcols - 10) + cut_pos + 1);
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
    size_t n = 0;
    char * result;

    if (strchr(pathname, ' ') != NULL) {
	n = strlen(pathname);
	result = (char *)malloc(n + 3);
	if (result == NULL)
	    outofmem(__FILE__, "quote_pathname");
	result[0] = '"';
	strcpy(result + 1, pathname);
	result[n+1] = '"';
	result[n+2] = '\0';
    } else {
	result = strdup(pathname);
	if (result == NULL)
	    outofmem(__FILE__, "quote_pathname");
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


void run_external ARGS1(char *, c)
{
#ifdef WIN_EX
    HANDLE handle;
    int stat;
    char pram_string[PATH_MAX];
    int redraw_flag;
    extern int xsystem(char *cmd);
#endif
    char command[1024];
    lynx_html_item_type *externals2 = 0;

    if (externals == NULL)
	return;

#ifdef WIN_EX			/* 1998/01/26 (Mon) 09:16:13 */
    if (c == NULL) {
	HTInfoMsg("Not exist external command");
	return;
    }
#endif

    for (externals2 = externals; externals2 != NULL;
	 externals2 = externals2->next) {

#ifdef WIN_EX
	handle = GetForegroundWindow();
	CTRACE(tfp, "EXTERNAL: '%s' <==> '%s'\n", externals2->name, c);
#endif
	if (externals2->command != 0
	  && !strncasecomp(externals2->name, c, strlen(externals2->name)))
	{
	    char *cp;

	    if (no_externals && !externals2->always_enabled) {
		HTUserMsg(EXTERNALS_DISABLED);
		return;
	    }
	    /*  Too dangerous to leave any URL that may come along unquoted.
	     *  They often contain '&', ';', and '?' chars, and who knows
	     *  what else may occur.
	     *  Prevent spoofing of the shell.
	     *  Dunno how this needs to be modified for VMS or DOS. - kw
	     */
#if (defined(VMS) || defined(DOSPATH) || defined(__EMX__)) && !defined(WIN_EX)
	    sprintf(command, externals2->command, c);
#else	/* Unix or DOS/Win: */
#if defined(WIN_EX)
	    if (*c != '\"' && strchr(c, ' ') != NULL) {
		cp = quote_pathname(c);
		sprintf(command, externals2->command, cp);
		FREE(cp);
	    } else {
		strcpy(pram_string, c);
		decode_string(pram_string);
		c = pram_string;

		/* mailto: */
		if (strnicmp("mailto:", c, 7) == 0) {
		    sprintf(command, externals2->command, c + 7);
		}
		/* telnet:// */
		else if (strnicmp("telnet://", c, 9) == 0) {
		    char host[STRING_MAX];
		    int last_pos;

		    strcpy(host, c + 9);
		    last_pos = strlen(host) - 1;
		    if (last_pos > 1 && host[last_pos] == '/')
			host[last_pos] = '\0';

		    sprintf(command, externals2->command, host);
		}
		/* file:// */
		else if (strnicmp("file://localhost/", c, 17) == 0) {
		    extern char windows_drive[];
		    char e_buff[PATH_MAX], *p;

		    p = c + 17;
		    if (strchr(p, ':') == NULL) {
			sprintf(e_buff, "%s/%s", windows_drive, p);
		    } else {
			strcpy(e_buff, p);
		    }
		    p = strrchr(e_buff, '.');
		    if (p) {
			p = strchr(p, '#');
			if (p) {
			    *p = '\0';
			}
		    }
		    if (*e_buff != '\"' && strchr(e_buff, ' ') != NULL) {
			p = quote_pathname(e_buff);
			strcpy(e_buff, p);
			FREE(p);
		    }

		    /* Less ==> short filename,
		     * less ==> long filename
		     */
		    if (isupper(externals2->command[0])) {
			sprintf(command,
				externals2->command, HTDOS_short_name(e_buff));
		    } else {
			sprintf(command, externals2->command, e_buff);
		    }
		} else {
		    sprintf(command, externals2->command, c);
		}
	    }
#else	/* Unix */
	    cp = HTQuoteParameter(c);
	    sprintf(command, externals2->command, cp);
	    FREE(cp);
#endif
#endif	/* VMS */

	    if (*command != '\0') {
#ifdef WIN_EX			/* 1997/10/17 (Fri) 14:07:50 */
		int len;
		char buff[PATH_MAX];

		CTRACE(tfp, "Lynx EXTERNAL: '%s'\n", command);
#ifdef WIN_GUI			/* 1997/11/06 (Thu) 14:17:15 */
		stat = MessageBox(handle, command,
				  "Lynx (EXTERNAL COMMAND EXEC)",
		       MB_ICONQUESTION | MB_SETFOREGROUND | MB_OKCANCEL);
		if (stat == IDCANCEL) {
		    return;
		}
#else
		stat = HTConfirm(string_short(command, 40));
		if (stat == NO) {
		    return;
		}
#endif

		len = strlen(command);
		if (len > 255) {
		    sprintf(buff, "Lynx: command line too long (%d > 255)", len);
#ifdef WIN_GUI			/* 1997/11/06 (Thu) 14:17:02 */
		    MessageBox(handle, buff,
			       "Lynx (EXTERNAL COMMAND EXEC)",
			  MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_OK);
		    SetConsoleTitle("Lynx for Win32");
#else
		    stat = HTConfirm(string_short(buff, 40));
#endif
		    return;
		} else {
		    SetConsoleTitle(command);
		}
#endif

#ifdef WIN_EX
		if (strnicmp(command, "start ", 6) == 0)
		    redraw_flag = FALSE;
		else
		    redraw_flag = TRUE;

		if (redraw_flag) {
		    stop_curses();
		    fflush(stdout);
		}
#else
		HTUserMsg(command);
		stop_curses();
#endif

		/* command running. */
#ifdef WIN_EX			/* 1997/10/17 (Fri) 14:07:50 */
#ifdef __CYGWIN__
		stat = system(command);
#else
		stat = xsystem(command);
#endif
		if (stat != 0) {
		    sprintf(buff,
			    "EXEC code = %04x (%2d, %2d)\r\n"
			    "'%s'",
			    stat, (stat / 256), (stat & 0xff),
			    command);
#ifdef SH_EX	/* WIN_GUI for ERROR only */
		    MessageBox(handle, buff,
			       "Lynx (EXTERNAL COMMAND EXEC)",
			       MB_ICONSTOP | MB_SETFOREGROUND | MB_OK);
#else
		    stat = HTConfirm(string_short(buff, 40));
#endif		/* 1 */
		}
#else	/* Not WIN_EX */
		LYSystem(command);
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
	    return;
	} /* end if */
    } /* end-for */

    return;
}
#endif	/* USE_EXTERNALS */
