#include "HTUtils.h"
#include "tcp.h"
#include "HTAlert.h"
#include "LYUtils.h"
#include "LYStrings.h"
#include "LYBookmark.h"
#include "LYGlobalDefs.h"
#include "LYSignal.h"
#include "LYSystem.h"
#include "LYKeymap.h"
#include "LYCharUtils.h"

#ifdef VMS
#include "HTVMSUtils.h"
#include <nam.h>
#endif /* VMS */

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

PRIVATE BOOLEAN is_mosaic_hotlist=FALSE;
PRIVATE char * convert_mosaic_bookmark_file PARAMS((char *filename_buffer));

/*
 *  Tries to open the bookmark file for reading.
 *  if successful the file is closed and the filename
 *  is returned and the URL is given in name.
 */
PUBLIC char * get_bookmark_filename ARGS1(char **,URL)
{
    char URL_buffer[256];
    static char filename_buffer[256];
    char string_buffer[256];
    FILE *fp;

    if (!bookmark_page) {
	sprintf(string_buffer,
		BOOKMARK_FILE_NOT_DEFINED,
		key_for_func(LYK_OPTIONS));
	_statusline(string_buffer);
	sleep(AlertSecs);
	return(NULL);
    }

    /*
     *  See if it is in the home path.
     */
#ifdef VMS
    sprintf(filename_buffer,"%s%s", Home_Dir(), bookmark_page);
#else
    sprintf(filename_buffer,"%s/%s", Home_Dir(), bookmark_page);
#endif /* VMS */
    if ((fp = fopen(filename_buffer,"r")) != NULL) {
	goto success;
    }

    /*
     *  See if we can open it raw.
     */
    if ((fp = fopen(bookmark_page,"r")) != NULL) {
	strcpy(filename_buffer, bookmark_page);
	goto success;
    } 

    /*
     *  Failure.
     */
    return(NULL);

success:
    /*
     *  We now have the file open.  Check if it is a mosaic
     *  hotlist.
     */
    if (fgets(string_buffer, 255, fp) &&
	!strncmp(string_buffer, "ncsa-xmosaic-hotlist-format-1", 29)) {
	char *newname;
	/*
	 *  It is a mosaic hotlist file.
	 */
	is_mosaic_hotlist=TRUE;
	fclose(fp);
	newname = convert_mosaic_bookmark_file(filename_buffer);
#ifdef VMS
    sprintf(URL_buffer,"file://localhost%s", HTVMS_wwwName((char *)newname));
#else
    sprintf(URL_buffer,"file://localhost%s", newname);
#endif /* VMS */

    } else {
	fclose(fp);
	is_mosaic_hotlist=FALSE;
#ifdef VMS
    sprintf(URL_buffer,"file://localhost%s",
    			HTVMS_wwwName((char *)filename_buffer));
#else
    sprintf(URL_buffer,"file://localhost%s", filename_buffer);
#endif /* VMS */
    }

    StrAllocCopy(*URL, URL_buffer);
    return(filename_buffer);  /* bookmark file exists */

} /* big end */

PRIVATE char * convert_mosaic_bookmark_file ARGS1(char *,filename_buffer)
{
    static char newfile[256];
    static BOOLEAN first = TRUE;
    FILE *fp, *nfp;
    char buf[BUFSIZ];
    int line = -2;
    char *endline;

    if (first) {
        tempname(newfile, NEW_FILE);
	first = FALSE;
#ifdef VMS
    } else {
        remove(newfile);   /* Remove duplicates on VMS. */
#endif /* VMS */
    }


    if((nfp = fopen(newfile, "w")) == NULL) {
	_statusline(NO_TEMP_FOR_HOTLIST);
	sleep(AlertSecs);
	return ("");
    }

    if((fp = fopen(filename_buffer, "r")) == NULL)
	return ("");  /* should always open */

    fprintf(nfp,"<head>\n<title>%s</title>\n</head>\n",MOSAIC_BOOKMARK_TITLE);
    fprintf(nfp,"\
     This file is an HTML representation of the X Mosaic hotlist file.\n\
     Outdated or invalid links may be removed by using the\n\
     remove bookmark command, it is usually the 'R' key but may have\n\
     been remapped by you or your system administrator.\n\n<p>\n<ol>\n");

    while (fgets(buf, sizeof(buf), fp) != NULL) {
	if(line >= 0) {
	    endline = &buf[strlen(buf)-1];
	    if(*endline == '\n')
		*endline = '\0';
	    if((line % 2) == 0) { /* even lines */
		if(*buf != '\0') {
		    strtok(buf," "); /* kill everything after the space */
	            fprintf(nfp,"<LI><a href=\"%s\">",buf); /* the URL */
		}
	    } else { /* odd lines */
	        fprintf(nfp,"%s</a>\n",buf);  /* the title */
	    }
	} 
	/* else - ignore the line (this gets rid of first two lines) */
	line++;
    }
    fclose(nfp);
    fclose(fp);
    return(newfile);
}

PUBLIC void save_bookmark_link ARGS2(char *,address, char *,title)
{
    FILE *fp;
    BOOLEAN first_time = FALSE;
    char *filename;
    char *bookmark_URL = NULL;
    char filename_buffer[256];
    char *Address = NULL;
    char *Title = NULL;

    if (!(address && *address)) {
        HTAlert(MALFORMED_ADDRESS);
	return;
    }

    filename = get_bookmark_filename(&bookmark_URL);
    FREE(bookmark_URL); /* don't need it */
    if (!bookmark_page)
	return;

    /*
     *  Allow user to change the title. - FM
     */
    filename_buffer[255] = '\0';
    LYstrncpy(filename_buffer, title, 255);
    convert_to_spaces(filename_buffer);
    _statusline(TITLE_PROMPT); 
    LYgetstr(filename_buffer, VISIBLE, sizeof(filename_buffer), NORECALL);
    if (*filename_buffer == '\0') {
	_statusline(CANCELLED);
	sleep(MessageSecs);
	return;
    }

    /*
     *  Create the Title with any left-angle-brackets converted to &lt;
     *  entities and any ampersands converted to &amp; entities.  - FM
     */
    StrAllocCopy(Title, filename_buffer);
    LYEntify(&Title, TRUE);

    /*
     *  Open the bookmark file. - FM
     */
    if (filename == NULL) {
	first_time = TRUE;
	/*
	 *  Try in the home directory first.
	 */
#ifdef VMS
    	sprintf(filename_buffer, "sys$login:%s", bookmark_page);
#else
    	sprintf(filename_buffer, "%s/%s", Home_Dir(), bookmark_page);
#endif /* VMS */
    	if ((fp = fopen(filename_buffer,"w")) == NULL) {
	   /*
	    *  Try it raw.
	    */
    	    if ((fp = fopen(bookmark_page,"r")) == NULL) {
	        _statusline(BOOKMARK_OPEN_FAILED);
	        sleep(AlertSecs);
	        return;
	    }
	}
    } else {
	if ((fp = fopen(filename,"a+")) == NULL) {
	    _statusline(BOOKMARK_OPEN_FAILED);
	    sleep(AlertSecs);
	    return;
	}
    }

    /*
     *  Convert all ampersands in the address to &amp; entities. - FM
     */
    StrAllocCopy(Address, address);
    LYEntify(&Address, FALSE);

    /*
     *  If we created a new bookmark file, write the headers. - FM
     */
    if (first_time) {
	fprintf(fp,"<head>\n<title>%s</title>\n</head>\n",BOOKMARK_TITLE);
	fprintf(fp,"\
     You can delete links using the new remove bookmark command.\n\
     it is usually the 'R' key but may have been remapped by you or\n\
     your system administrator.<br>\n\
     This file may also be edited with a standard text editor.\n\
     Outdated or invalid links may be removed by simply deleting\n\
     the line the link appears on in this file.\n\
     Please refer to the Lynx documentation or help files\n\
     for the HTML link syntax.\n\n<p>\n<ol>\n");
    }

    /*
     *  Add the bookmark link, in Mosaic hotlist or Lynx format. - FM
     */
    if (is_mosaic_hotlist) {
	time_t NowTime = time(NULL);
	char *TimeString = (char *)ctime (&NowTime);
	/*
	 *  TimeString has a \n at the end.
	 */
	fprintf(fp,"%s %s%s\n", Address, TimeString, Title);
    } else {
	fprintf(fp,"<LI><a href=\"%s\">%s</a>\n", Address, Title);
    }

    fclose(fp);
    FREE(Title);
    FREE(Address);

    _statusline(OPERATION_DONE);
    sleep(MessageSecs);
}
	
PUBLIC void remove_bookmark_link ARGS1(int,cur)
{
    FILE *fp, *nfp;
    char buf[BUFSIZ];
    int n;
#ifdef VMS
    char newfile[NAM$C_MAXRSS+12];
#else
    char newfile[128];
    struct stat stat_buf;
    mode_t mode;
#endif /* VMS */
    char *filename;
    char *URL = 0;

    if (TRACE)
	fprintf(stderr, "remove_bookmark_link: deleting link number: %d\n",
			cur);

    filename = get_bookmark_filename(&URL);
    FREE(URL); /* don't need it */
    if (!bookmark_page)
	return;

    if ((!filename) || (fp=fopen(filename, "r")) == NULL) {
	_statusline(BOOKMARK_OPEN_FAILED_FOR_DEL);
	sleep(AlertSecs);
	return;
    }

#ifdef VMS
    sprintf(newfile, "%s-%d", filename, getpid());
#else
    tempname(newfile, NEW_FILE);
#endif /* VMS */
    if ((nfp = fopen(newfile, "w")) == NULL) {
	fclose(fp);
#ifdef VMS
	_statusline(BOOKSCRA_OPEN_FAILED_FOR_DEL);
#else
	_statusline(BOOKTEMP_OPEN_FAILED_FOR_DEL);
#endif /* VMS */
	sleep(AlertSecs);
	return;
    }

#ifndef VMS
    /*
     *  Explicitly preserve bookmark file mode on Unix. - DSL
     */
    if (stat(filename,&stat_buf) == 0) {
	mode = ((stat_buf.st_mode & 0777) | 0600);
	(void) fclose(nfp);
	nfp = NULL;
	(void) chmod(newfile, mode);
	if ((nfp = fopen(newfile, "a")) == NULL) {
	    (void) fclose(fp);
	    _statusline(BOOKTEMP_REOPEN_FAIL_FOR_DEL);
	    sleep(AlertSecs);
	    return;
	}
    }
#endif /* !VMS */

    if (is_mosaic_hotlist) {
	int del_line = cur*2;  /* two lines per entry */
	n = -3;  /* skip past cookie and name lines */
        while (fgets(buf, sizeof(buf), fp) != NULL) {
	    n++;
	    if (n == del_line || n == del_line+1) 
		continue;  /* remove two lines */
            if (fputs(buf, nfp) == EOF)
                goto failure;
	}

    } else {
	char *cp;
	BOOLEAN retain;
	int seen;

        n = -1;
        while (fgets(buf, sizeof(buf), fp) != NULL) {
	    retain = TRUE;
	    seen = 0;
	    cp = buf;
            while (n < cur && (cp = LYstrstr(cp, "<a href="))) {
		seen++;
                if (++n == cur) {
		    if (seen != 1 || !LYstrstr(buf, "</a>") ||
			LYstrstr(cp+1, "<a href=")) {
			_statusline(BOOKMARK_LINK_NOT_ONE_LINE);
			sleep(AlertSecs);
			goto failure;
		    }
		    if (TRACE)
	    		fprintf(stderr,
				"remove_bookmark_link: skipping link %d\n", n);
                    retain = FALSE;
		}
		cp += 8;
            }
            if (retain && fputs(buf, nfp) == EOF)
                goto failure;
        }
    }

    if (TRACE)
	fprintf(stderr, "remove_bookmark_link: files: %s %s\n",
			newfile, filename);

    fclose(fp);
    fp = NULL;
    fclose(nfp);
    nfp = NULL;
 	
    if (rename(newfile, filename) != -1) {
#ifdef VMS
	char VMSfilename[256];
	/*
	 *  Purge lower version of file.
	 */
	sprintf(VMSfilename, "%s;-1", filename);
        while (remove(VMSfilename) == 0)
	    ;
	/*
	 *  Reset version number.
	 */
	sprintf(VMSfilename, "%s;1", filename);
	rename(filename, VMSfilename);
#endif /* VMS */
        return;
    } else {
#ifndef VMS
	/*
	 *  Rename won't work across file systems.
	 *  Check if this is the case and do something appropriate.
	 *  Used to be ODD_RENAME
	 */
	if (errno == EXDEV) {
	    char buffer[2048];
	    sprintf(buffer, "%s %s %s", MV_PATH, newfile, filename);
	    system(buffer);
	    return;
	}
#endif /* !VMS */

#ifdef VMS
	_statusline(ERROR_RENAMING_SCRA);
#else
	_statusline(ERROR_RENAMING_TEMP);
#endif /* VMS */
	if (TRACE)
	    perror("renaming the file");
	sleep(AlertSecs);
    }
	   
failure:
    _statusline(BOOKMARK_DEL_FAILED);
    sleep(AlertSecs);
    if (nfp != NULL)
	fclose(nfp);
    if (fp != NULL)
        fclose(fp);
    remove(newfile);
}
