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
#include <HTAlert.h>
#include <LYGlobalDefs.h>
#include <LYExtern.h>
#include <LYCurses.h>

#include <LYLeaks.h>

#ifdef USE_EXTERNALS
void run_external ARGS1(char *, c)
{
	char command[1024];
	lynx_html_item_type *externals2=0;

	if (externals == NULL) return;

	for(externals2=externals; externals2 != NULL;
		 externals2=externals2->next)
	{

	 if (externals2->command != 0
	  && !strncasecomp(externals2->name,c,strlen(externals2->name)))
	 {
	     char *cp;

		if(no_externals && !externals2->always_enabled)
		{
		  HTUserMsg(EXTERNALS_DISABLED);
		  return;
		}

		/*  Too dangerous to leave any URL that may come along unquoted.
		 *  They often contain '&', ';', and '?' chars, and who knows
		 *  what else may occur.
		 *  Prevent spoofing of the shell.
		 *  Dunno how this needs to be modified for VMS or DOS. - kw
		 */
#if defined(VMS) || defined(DOSPATH) || defined(__EMX__)
		sprintf(command, externals2->command, c);
#else /* Unix: */
		cp = quote_pathname(c);
		sprintf(command, externals2->command, cp);
		FREE(cp);
#endif /* VMS */

		if (*command != '\0')
		{

		 HTUserMsg(command);

		 stop_curses();
		 LYSystem(command);
		 start_curses();
		}

		return;
	 }
	}

	return;
}
#endif /* USE_EXTERNALS */
