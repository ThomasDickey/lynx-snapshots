/*	       DOS specific routines

 */

#include <HTUtils.h>
#include <HTDOS.h>

/*
 * Make a copy of the source argument in the result, allowing some extra
 * space so we can append directly onto the result without reallocating.
 */
PRIVATE char * copy_plus ARGS2(char **, result, char *, source)
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
char * HTDOS_wwwName ARGS1(char *, dosname)
{
    static char *wwwname;
    char *cp_url = copy_plus(&wwwname, dosname);

    for ( ; *cp_url != '\0' ; cp_url++)
	if(*cp_url == '\\')
	    *cp_url = '/';   /* convert dos backslash to unix-style */

    if(strlen(wwwname) > 3 && *cp_url == '/')
	*cp_url = '\0';

    if(*cp_url == ':') {
	cp_url++;
	*cp_url = '/';
    }

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
    static char *cp_url;
    char *result;
    int joe;

    copy_plus(&cp_url, wwwname);

    for (joe = 0; cp_url[joe] != '\0'; joe++)	{
	if (cp_url[joe] == '/')	{
	    cp_url[joe] = '\\';
	}
    }

    /* Needed to surf the root of a local drive. */

    if(strlen(cp_url) < 4) cp_url[2] = ':';
    if(strlen(cp_url) == 3) strcpy(cp_url+3, "\\");
    if(strlen(cp_url) == 4) strcpy(cp_url+4, ".");

    if((strlen(cp_url) > 2) && (cp_url[1] == '|'))
	cp_url[1] = ':';

    if((cp_url[1] == '\\') || (cp_url[0]  != '\\')) {
	result = cp_url;
    } else {
	result = cp_url+1;
    }

    CTRACE(tfp, "HTDOS_name changed `%s' to `%s'\n", wwwname, result);
    return (result);
}
