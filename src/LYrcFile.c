#include "HTUtils.h"
#include "tcp.h"
#include "HTFTP.h"
#include "LYUtils.h"
#include "LYrcFile.h"
#include "LYStrings.h"
#include "LYGlobalDefs.h"
#include "LYCharSets.h"

#include "LYLeaks.h"

PUBLIC void read_rc()
{
    char line_buffer[256];
    char rcfile[256];
    FILE *fp;
    char *cp, *cp2;
    int number_sign;

    /* make a name */
#ifdef UNIX
    sprintf(rcfile,"%s/.lynxrc", Home_Dir());  /* UNIX */
#else
    sprintf(rcfile,"sys$login:.lynxrc");  /* VMS */
#endif /* UNIX */

    if ((fp = fopen(rcfile,"r")) == NULL) {
	return;
    }

    while (fgets(line_buffer, 256, fp) != NULL) {

	/* remove the end /n */
	if (line_buffer[0] && line_buffer[strlen(line_buffer)-1] == '\n')
	    line_buffer[strlen(line_buffer)-1] = '\0';

	/* remove trailing white space */
	while (line_buffer[0] && isspace(line_buffer[strlen(line_buffer)-1]))
	    line_buffer[strlen(line_buffer)-1] = '\0';
	
	/*  skip any comment or blank lines */
	if (line_buffer[0] == '\0' || line_buffer[0] == '#')
	    continue;

        /* find the line position of the number sign if there is one */
        if ((cp = (char *)strchr(line_buffer,'#')) == NULL)
	    number_sign = 999;
	else
	    number_sign = cp - line_buffer;


	/* Character set */
	if ((cp=LYstrstr(line_buffer,"character_set"))!=NULL &&
		cp-line_buffer < number_sign) {

		int i=0;

	       	if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		    cp = cp2+1;

	       	while (isspace(*cp)) cp++;  /* get rid of spaces */

                for (; LYchar_set_names[i]; i++)
                    if (!strncmp(cp,LYchar_set_names[i],strlen(cp))) {
                        current_char_set=i;
			HTMLSetRawModeDefault(i);
                        break;
                    }

	/* Linedit mode */
	} else if ((cp=LYstrstr(line_buffer,"lineedit_mode"))!=NULL &&
		cp-line_buffer < number_sign) {

		int i=0;

	       	if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		    cp = cp2+1;

	       	while (isspace(*cp)) cp++;  /* get rid of spaces */

                for (; LYLineeditNames[i]; i++)
                    if (!strncmp(cp,LYLineeditNames[i],strlen(cp))) {
                        current_lineedit=i;
                        break;
                    }

	/* user mode */
	} else if ((cp=LYstrstr(line_buffer,"user_mode"))!=NULL &&
		cp-line_buffer < number_sign) {

		if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		    cp = cp2+1;

		while (isspace(*cp)) cp++;  /* get rid of spaces */
	
		if (LYstrstr(cp,"ADVANCED") != NULL)
	            user_mode = ADVANCED_MODE;
		else if (LYstrstr(cp,"INTERMEDIATE") != NULL)
	            user_mode = INTERMEDIATE_MODE;
		else
	            user_mode = NOVICE_MODE;

        /* file editor */
	} else if (!system_editor &&
		(cp=LYstrstr(line_buffer,"file_editor"))!=NULL &&
		cp-line_buffer < number_sign) {

	       if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		    cp = cp2+1;

	       while (isspace(*cp)) cp++;  /* get rid of spaces */
	
 	       StrAllocCopy(editor, cp);

	/* bookmark file */
	} else if ((cp=LYstrstr(line_buffer,"bookmark_file"))!=NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */

	   StrAllocCopy(bookmark_page, cp);

	/* personal_mail_address */
	} else if ((cp=LYstrstr(line_buffer,"personal_mail_address"))!=NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */

	   StrAllocCopy(personal_mail_address, cp);

	} else if ((cp = LYstrstr(line_buffer,"file_sorting_method")) != NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */

	   if (!strncasecomp(cp, "BY_FILENAME", 11))
		HTfileSortMethod = FILE_BY_NAME;
	   else if (!strncasecomp(cp, "BY_TYPE", 7))
		HTfileSortMethod = FILE_BY_TYPE;
	   else if (!strncasecomp(cp, "BY_SIZE", 7))
		HTfileSortMethod = FILE_BY_SIZE;
	   else if (!strncasecomp(cp, "BY_DATE", 7))
		HTfileSortMethod = FILE_BY_DATE;

	} else if ((cp = LYstrstr(line_buffer,"case_sensitive_searching"))
								   != NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */

	   if (!strncasecomp(cp, "on", 2))
	      case_sensitive=TRUE;
	   else
	      case_sensitive=FALSE;

#ifdef ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS

	} else if ((cp = LYstrstr(line_buffer,"run_all_execution_links")) 
								 != NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */

	   if (!strncasecomp(cp, "on", 2))
	      local_exec=TRUE;
	   else
	      local_exec=FALSE;

	} else if ((cp = LYstrstr(line_buffer,
			"run_execution_links_on_local_files")) != NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */

	   if (!strncasecomp(cp, "on", 2))
	      local_exec_on_local_files=TRUE;
	   else
	      local_exec_on_local_files=FALSE;

#endif /* ALLOW_USERS_TO_CHANGE_EXEC_WITHIN_OPTIONS */

	} else if ((cp=LYstrstr(line_buffer,"vi_keys"))!=NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char * )strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */
	
	   if (!strncasecomp(cp, "on", 2))
	      vi_keys=TRUE;
	   else
	      vi_keys=FALSE;

        } else if ((cp=LYstrstr(line_buffer,"emacs_keys"))!=NULL &&
                cp-line_buffer < number_sign) {

           if ((cp2 = (char *)strchr(cp,'=')) != NULL)
                cp = cp2+1;

           while (isspace(*cp)) cp++;  /* get rid of spaces */

           if (!strncasecomp(cp, "on", 2))
              emacs_keys=TRUE;
           else
              emacs_keys=FALSE;


	} else if ((cp=LYstrstr(line_buffer,"keypad_mode"))!=NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */
	
	   if (LYstrstr(cp,"LINKS_ARE_NUMBERED"))
	      keypad_mode = LINKS_ARE_NUMBERED;
	   else
	      keypad_mode = NUMBERS_AS_ARROWS;

	/* preferred language */
	} else if ((cp=LYstrstr(line_buffer,"preferred_language"))!=NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */

	   StrAllocCopy(language, cp);

	/* preferred charset */
	} else if ((cp=LYstrstr(line_buffer,"preferred_charset"))!=NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */

	   StrAllocCopy(pref_charset, cp);

	/* show dot files */
	} else if ((cp=LYstrstr(line_buffer,"show_dotfiles")) != NULL &&
		cp-line_buffer < number_sign) {

	   if ((cp2 = (char * )strchr(cp,'=')) != NULL)
		cp = cp2+1;

	   while (isspace(*cp)) cp++;  /* get rid of spaces */
	
	   if (!strncasecomp(cp, "on", 2))
	      show_dotfiles=TRUE;
	   else
	      show_dotfiles=FALSE;

#ifdef DIRED_SUPPORT
	/* list directory style */
	} else if ((cp=LYstrstr(line_buffer,"dir_list_style"))!=NULL &&
		cp-line_buffer < number_sign) {

		if ((cp2 = (char *)strchr(cp,'=')) != NULL)
		    cp = cp2+1;

		while (isspace(*cp)) cp++;  /* get rid of spaces */
	
		if (LYstrstr(cp,"FILES_FIRST") != NULL)
	            dir_list_style = FILES_FIRST;
		else if (LYstrstr(cp,"DIRECTORIES_FIRST") != NULL)
	            dir_list_style = 0;
		else
	            dir_list_style = MIXED_STYLE;

#endif /* DIRED_SUPPORT */

	} /* end of if */

    } /* end of while */
 
    fclose(fp);
} /* big end */
	
PUBLIC int save_rc ()
{
    char rcfile[256];
    FILE *fp;
    int i;

    /* make a name */
#ifdef UNIX
    sprintf(rcfile,"%s/.lynxrc", Home_Dir());  /* UNIX */
#else
#ifdef VMS
    sprintf(rcfile,"sys$login:.lynxrc");  /* VMS */
#else
    sprintf(rcfile,".lynxrc");   /* anything else */
#endif /* VMS */
#endif /* UNIX */
    
    if ((fp = fopen(rcfile,"w")) == NULL) {
	return FALSE;
    }

    /* header */
    fprintf(fp,"# Lynx user defaults file\n\n");

    /* user mode */
    fprintf(fp,"\
# user_mode specifies the users level of knowledge with Lynx.\n\
# The default is NOVICE which displays two extra lines of help at the\n\
# bottom of the screen to aid the user in learning the basic Lynx\n\
# commands.  Set user_mode to INTERMEDIATE to turn off the extra info.\n\
# Use ADVANCED to see the URL of the currently selected link at the\n\
# bottom of the screen\n");
    fprintf(fp,"user_mode=%s\n\n",(user_mode==NOVICE_MODE ? "NOVICE" : 
				  (user_mode==ADVANCED_MODE ? "ADVANCED" :
							"INTERMEDIATE")));

    /* editor */
    fprintf(fp,"\
# file editor specifies the editor to be invoked when editing Lynx files.\n\
# if no editor is specified then file editing is disabled unless it\n\
# is activated from the command line\n");
    fprintf(fp,"file_editor=%s\n\n", (editor ? editor : ""));

    /* home file */
    fprintf(fp,"\
# bookmark_file specifies the name and location of a custom file which the\n\
# user can paste links to for easy access at a later date\n");
    fprintf(fp,"bookmark_file=%s\n\n", (bookmark_page ? bookmark_page : ""));

    /* personal_mail_address */
    fprintf(fp,"\
# personal_mail_address specifies your personal mail address.  The\n\
# address will be sent during HTTP file transfers for authorization and\n\
# logging purposes, and for mailed comments.\n\
# If you do not want this information given out, leave this field blank\n");
    fprintf(fp,"personal_mail_address=%s\n\n", (personal_mail_address ?
						personal_mail_address : ""));

    /* case sensitive */
    fprintf(fp,"\
# if case sensitive searching is on then when the user invokes a search\n\
# using the 's' or '/' keys, the search performed will be case sensitive\n\
# instead of case INsensitive.\n# the default is usually off\n");
  fprintf(fp,"case_sensitive_searching=%s\n\n",(case_sensitive ? "on" : "off"));

    /* file sort method */
    fprintf(fp,"\
# The file sort method specifies which value to sort on when viewing file\n\
# lists such as FTP directories.  The options are:\n\
#    BY_FILENAME -- sorts on the name of the file\n\
#    BY_TYPE     -- sorts on the type of the file\n\
#    BY_SIZE     -- sorts on the size of the file\n\
#    BY_DATE     -- sorts on the date of the file\n");
  fprintf(fp,"file_sorting_method=%s\n\n",
	  (HTfileSortMethod==FILE_BY_NAME ? "BY_FILENAME" : 
	 	(HTfileSortMethod==FILE_BY_SIZE ? "BY_SIZE" :
			(HTfileSortMethod==FILE_BY_TYPE ? "BY_TYPE" :
				"BY_DATE"))));

    /* Character Set */
    fprintf(fp,"\
# The character set definition controls the representation of\n\
# 8 bit characters for your terminal.  If 8 bit characters do\n\
# not show up correctly on your screen you may try changing\n\
# to a different 8 bit set or using the 7 bit character approximations.\n\
# Current valid characters sets are:\n");
    for (i=0;LYchar_set_names[i];i++)
	fprintf(fp,"#    %s\n",LYchar_set_names[i]);
    fprintf(fp,"character_set=%s\n\n",LYchar_set_names[current_char_set]);


#if defined(EXEC_LINKS) || defined(EXEC_SCRIPTS) 
    /* local_exec */
    fprintf(fp,"# if run all execution links is on then all local exection links will\n\
# be executed when they are selected.\n\
#\n\
# WARNING - this is potentially VERY dangerous.  Since you may view\n\
#           information that is written by unknown and untrusted sources\n\
#           there exists the possibility that trojan horse links could be\n\
#           written.  Trojan horse links could be written to erase files\n\
#           or compromise security.  This should only be set to on if you\n\
#           are viewing trusted source information\n");

    fprintf(fp,"run_all_execution_links=%s\n\n",(local_exec ? "on" : "off"));

    /* local_exec_on_local_files */
    fprintf(fp,"# if run all execution links is on then all local exection links that\n\
# are found in LOCAL files will be executed when they are selected.\n\
# This is different from \"run all execution links\" in that only files\n\
# that reside on the local system will have execution link permissions\n\
#\n\
# WARNING - this is potentially dangerous.  Since you may view\n\
#           information that is written by unknown and untrusted sources\n\
#           there exists the possibility that trojon horse links could be\n\
#           written.  Trojon horse links could be written to erase files\n\
#           or compromise security.  This should only be set to on if you\n\
#           are viewing trusted source information\n");

   fprintf(fp,"run_execution_links_on_local_files=%s\n\n",
			(local_exec_on_local_files ? "on" : "off"));

#endif /* defined(EXEC_LINKS) || defined(EXEC_SCRIPTS) */

    /* vi keys */
    fprintf(fp,"\
# if VI keys are turned on then the normal VI movement keys:\n\
# j - down    k - up\n\
# h - left    l - right\n\
# will be enabled.\n\
# These keys are only lower case.\n\
# Capital 'H' will still activate help\n");
    fprintf(fp,"vi_keys=%s\n\n",(vi_keys ? "on" : "off"));

    /* emacs keys */
    fprintf(fp,"\
# if EMACS keys are turned on then the normal EMACS movement keys:\n\
# ^N - down    ^p - up\n\
# ^B - left    ^F - right\n\
# will be enabled.\n");
    fprintf(fp,"emacs_keys=%s\n\n",(emacs_keys ? "on" : "off"));

    /* keypad mode */
    fprintf(fp,"\
# if keypad_mode is set to NUMBERS_AS_ARROWS then the numbers\n\
# on your keypad when the numlock is on will act as arrow keys.\n\
# i.e. 4 - Left Arrow, 6 - Right Arrow, 2 - Down Arrow, 8 - Up Arrow, etc.\n\
# if keypad_mode is set to LINKS_ARE_NUMBERED then numbers will appear\n\
# next to each link and numbers are used to select links.\n\
# note: some fixed format documents may look disfigured when\n\
# LINKS_ARE_NUMBERED is enabled.\n");
    fprintf(fp,"keypad_mode=%s\n\n",(keypad_mode==NUMBERS_AS_ARROWS ? 
				"NUMBERS_AS_ARROWS" : "LINKS_ARE_NUMBERED"));
 
    /* lineedit */
    fprintf(fp,"\
# linedit_mode specifies the key binding used for inputting strings in\n\
# prompts and forms. if line_editmode is set to DEFAULT_BINDING then the\n\
# following control characters are used for moving and deleting:\n\
#\n\
#      left word char       char word right     Enter = Next line\n\
#            W E    remove   R T                ^G    = Cancel input\n\
#             D <-   move  -> F                 ^U    = Erase line\n\
#\n\
# Current lineedit modes are:\n");

    { char **bindings = LYLineeditNames;
      while (*bindings) {
          fprintf(fp,"#    %s\n",*bindings);
          bindings++;
      }
    }
    fprintf(fp,"lineedit_mode=%s\n\n",LYLineeditNames[current_lineedit]);

    /* preferred language */
    fprintf(fp,"\
# preferred_language specifies the language in MIME notation (e.g., en,\n\
# fr) which Lynx will indicate you prefer in requests to http servers.\n\
# If a file in that language is available, the server will send it.\n\
# Otherwise, the server will send the file in it's default language.\n");
    fprintf(fp,"preferred_language=%s\n\n", (language ? language : ""));

    /* preferred charset */
    fprintf(fp,"\
# preferred_charset specifies the character set in MIME notation (e.g.,\n\
# ISO-8859-2, ISO-8859-5) which Lynx will indicate you prefer in requests\n\
# to http servers using an Accept-Charset header.  The value should NOT\n\
# include ISO-8859-1 or US-ASCII, since those values are always assumed by\n\
# default.  If a file in that character set is available, the server will\n\
# send it.  Otherwise, the server will send the file in ISO-8859-1 or\n\
# US-ASCII.\n");
    fprintf(fp,"preferred_charset=%s\n\n", (pref_charset ? pref_charset : ""));

    /* show dot files */
    fprintf(fp, "\
# show_dotfiles specifies that the directory listing should include\n\
# \"hidden\" (dot) files/directories.  If set \"on\", this will be\n\
# honored only if enabled via userdefs.h and/or lynx.cfg, and not\n\
# restricted via a command line switch.  If display of hidden files\n\
# is disabled, creation of such files via Lynx also is disabled.\n");
    fprintf(fp, "show_dotfiles=%s\n\n",(show_dotfiles ? "on" : "off"));

#ifdef DIRED_SUPPORT
    /* list directory style */
    fprintf(fp,"\
# dir_list_styles specifies the directory list style under DIRED_SUPPORT\n\
# (if implemented).  The default is MIXED_STYLE, which sorts both files and\n\
# directories together.  FILES_FIRST lists files first and DIRECTORIES_FIRST\n\
# lists directories first.\n");
    fprintf(fp,"dir_list_style=%s\n\n",
    			(dir_list_style==FILES_FIRST ? "FILES_FIRST" : 
			 (dir_list_style==MIXED_STYLE ? "MIXED_STYLE" :
							"DIRECTORIES_FIRST")));
#endif /* DIRED_SUPPORT */

    fclose(fp);

    /* get rid of any copies of the .lynxrc file that VMS creates */
#ifdef VMS
	while (remove("sys$login:.lynxrc;-1") == 0) ;
	/* reset version number */
	rename("sys$login:.lynxrc", "sys$login:.lynxrc;1");
#endif /* VMS */

   return TRUE;

}
