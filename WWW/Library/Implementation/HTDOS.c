/*	       DOS specific routines

 */

#include <HTUtils.h>
#include <HTDOS.h>

/*
 * Make a copy of the source argument in the result, allowing some extra
 * space so we can append directly onto the result without reallocating.
 */
PRIVATE char * copy_plus ARGS2(char **, result, CONST char *, source)
{
    int length = strlen(source);
    HTSprintf0(result, "%-*s", length+10, source);
    (*result)[length] = 0;
    return (*result);
}

/* PUBLIC							HTDOS_wwwName()
**		CONVERTS DOS Name into WWW Name
** ON ENTRY:
**	dosname 	DOS file specification (NO NODE)
**
** ON EXIT:
**	returns 	WWW file specification
**
*/
char * HTDOS_wwwName ARGS1(CONST char *, dosname)
{
    static char *wwwname = NULL;
    char *cp_url = copy_plus(&wwwname, dosname);
    int wwwname_len;

#ifdef SH_EX
    char ch;

    while ((ch = *dosname) != '\0') {
	switch (ch) {
	case '\\':
	/* convert dos backslash to unix-style */
	    *cp_url++ = '/';
	    break;
	case ' ':
	    *cp_url++ = '%';
	    *cp_url++ = '2';
	    *cp_url++ = '0';
	    break;
      default:
	    *cp_url++ = ch;
	    break;
      }
      dosname++;
    }
    *cp_url = '\0';
#else
    for ( ; *cp_url != '\0' ; cp_url++)
	if(*cp_url == '\\')
	    *cp_url = '/';   /* convert dos backslash to unix-style */
#endif

    wwwname_len = strlen(wwwname);
    if (wwwname_len > 1)
	cp_url--;	/* point last char */

    if (wwwname_len > 3 && *cp_url == '/')
	*cp_url = '\0';

#ifdef NOTUSED
    if(*cp_url == ':') {
	cp_url++;
	*cp_url = '/';	/* terminate drive letter to survive */
    }
#endif

    return(wwwname);
}


/* PUBLIC							HTDOS_name()
**		CONVERTS WWW name into a DOS name
** ON ENTRY:
**	wwwname 	WWW file name
**
** ON EXIT:
**	returns 	DOS file specification
*/
char * HTDOS_name ARGS1(char *, wwwname)
{
#ifdef _WINDOWS	/* 1998/04/02 (Thu) 08:47:20 */
    extern char windows_drive[];
    char temp_buff[LY_MAXPATH];
#endif
    static char *cp_url = NULL;
    char *result, *ret;
    int joe;

    CTRACE((tfp, "HTDOS_name changed `%s'\n", wwwname));
    copy_plus(&cp_url, wwwname);

    for (joe = 0; cp_url[joe] != '\0'; joe++)	{
	if (cp_url[joe] == '/')	{
	    cp_url[joe] = '\\';	/* convert slashes to dos-style */
	}
    }

    /* pesky leading slash, rudiment from file://localhost/  */
    /* the rest of path may be with or without drive letter  */
    if((cp_url[1] == '\\') || (cp_url[0]  != '\\')) {
	result = cp_url;
    } else {
	result = cp_url+1;
    }

#ifdef _WINDOWS	/* 1998/04/02 (Thu) 08:59:48 */
    if (strchr(result, '\\') && strchr(result, ':')==NULL) {
	sprintf(temp_buff, "%s\\%s", windows_drive, result);
	ret = NULL;
	StrAllocCopy(ret, temp_buff);
	free(cp_url);
    } else {
	char *p;
	p = strchr(result, ':');
	if (p && (strcmp(p, ":\\") == 0)) {
	    p[2] = '.';
	    p[3] = '\0';
	}
	ret = result;
    }
#else
    ret = result;
#endif

    CTRACE((tfp, "HTDOS_name changed `%s' to `%s'\n", wwwname, ret));
    return (ret);
}

