#include "HTUtils.h"
#include "tcp.h"
#include "HTParse.h"
#include "HTAlert.h"
#include "HTTP.h"
#include "LYCurses.h"
#include "LYStrings.h"
#include "LYUtils.h"
#include "LYStructs.h"
#include "LYGlobalDefs.h"
#include "LYShowInfo.h"
#include "LYSignal.h"
#include "LYCharUtils.h"

#include "LYLeaks.h"

#ifdef DIRED_SUPPORT
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "LYLocal.h"
#endif /* DIRED_SUPPORT */

#define FREE(x) if (x) {free(x); x = NULL;}

/* 
 *  Showinfo prints a page of info about the current file and the link
 *  that the cursor is on.
 */
	     
PUBLIC int showinfo ARGS4(
	document *,	doc,
	int,		size_of_file,
	document *,	newdoc,
	char *,		owner_address)
{
    static char tempfile[256];
    static BOOLEAN first = TRUE;
    char *info_url = NULL;
    int url_type;
    FILE *fp0;
    char *Address = NULL, *Title = NULL;
    char *cp;

#ifdef DIRED_SUPPORT
    char temp[300];
    struct stat dir_info;
    struct passwd *pw;
    struct group *grp;
#endif /* DIRED_SUPPORT */
    if (first) {
        tempname(tempfile,NEW_FILE);
	first = FALSE;
#ifdef VMS
    } else {
        remove(tempfile);   /* Remove duplicates on VMS. */
#endif /* VMS */
    }


    /*
     *  Make the temporary file a URL now.
     */
#ifdef VMS
    StrAllocCopy(info_url,"file://localhost/");
#else
    StrAllocCopy(info_url,"file://localhost");
#endif /* VMS */
    StrAllocCat(info_url,tempfile);

    if ((fp0 = fopen(tempfile,"w")) == NULL) {
        HTAlert(CANNOT_OPEN_TEMP);
	FREE(info_url);
        return(0);
    }

    /*
     *  Point the address pointer at this Url
     */
    StrAllocCopy(newdoc->address, info_url);
    FREE(info_url);

    if (nlinks > 0 && links[doc->link].lname != NULL &&
	(url_type = is_url(links[doc->link].lname)) != 0 &&
	(url_type == LYNXEXEC_URL_TYPE ||
	 url_type == LYNXPROG_URL_TYPE)) {
	char *last_slash = strrchr(links[doc->link].lname,'/');
	if (last_slash-links[doc->link].lname ==
	    	strlen(links[doc->link].lname)-1) {
	    links[doc->link].lname[strlen(links[doc->link].lname)-1] = '\0';
	}
    }

    fprintf(fp0, "<head>\n<title>%s</title>\n</head>\n<body>\n",
		 SHOWINFO_TITLE);
    fprintf(fp0,"<h1>You have reached the Information Page</h1>\n");
    fprintf(fp0,"<h2>%s Version %s</h2>\n", LYNX_NAME, LYNX_VERSION);

#ifdef DIRED_SUPPORT
    if (lynx_edit_mode) {
	fprintf(fp0,
	   	"<h2>Directory that you are currently viewing</h2>\n<pre>");

	cp = doc->address;
	if (!strncmp(cp, "file://localhost", 16)) 
	    cp += 16;
	else if (!strncmp(cp, "file:", 5))
	    cp += 5;
	strcpy(temp, cp);
	HTUnEscape(temp);

	fprintf(fp0,"   Name:  %s\n", temp);
	fprintf(fp0,"    URL:  %s\n", doc->address);

	cp = links[doc->link].lname;
	if (!strncmp(cp, "file://localhost", 16)) 
	    cp += 16;
	else if (!strncmp(cp, "file:", 5))
	    cp += 5;
	strcpy(temp, cp);
	HTUnEscape(temp);
	if (lstat(temp, &dir_info) == -1) {
	    _statusline(CURRENT_LINK_STATUS_FAILED);
	    sleep(AlertSecs);
	} else {
	    char modes[80];
	    if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) {
		fprintf(fp0,
		 	"\nDirectory that you have currently selected\n\n");
	    } else if (((dir_info.st_mode) & S_IFMT) == S_IFREG) {
		fprintf(fp0, "\nFile that you have currently selected\n\n");
	    } else if (((dir_info.st_mode) & S_IFMT) == S_IFLNK) {
		fprintf(fp0,
		      "\nSymbolic link that you have currently selected\n\n");
	    } else {
		fprintf(fp0, "\nItem that you have currently selected\n\n");
	    }
	    fprintf(fp0,"       Full name:  %s\n", temp);
	    if (((dir_info.st_mode) & S_IFMT) == S_IFLNK) {
		char buf[1025];
		int buf_size;

		if ((buf_size = readlink(temp, buf, sizeof(buf)-1)) != -1) {
		    buf[buf_size] = '\0';
		} else {
		    strcpy(buf, "Unable to follow link");
		}
		fprintf(fp0, "  Points to file:  %s\n", buf);
	    }
	    pw = getpwuid(dir_info.st_uid);
	    if (pw)
	        fprintf(fp0, "   Name of owner:  %s\n", pw->pw_name);
	    grp = getgrgid(dir_info.st_gid);
	    if (grp && grp->gr_name)
	        fprintf(fp0, "      Group name:  %s\n", grp->gr_name);
	    if (((dir_info.st_mode) & S_IFMT) == S_IFREG) {
		sprintf(temp, "       File size:  %ld (bytes)\n",
		 	      (long)dir_info.st_size);
		fprintf(fp0, "%s", temp);
	    }
	    /*
	     *  Include date and time information.
	     */
	    cp = ctime(&dir_info.st_ctime);
	    fprintf(fp0, "   Creation date:  %s", cp);

	    cp = ctime(&dir_info.st_mtime);	      
	    fprintf(fp0, "   Last modified:  %s", cp);

	    cp = ctime(&dir_info.st_atime);
	    fprintf(fp0, "   Last accessed:  %s\n", cp);

	    fprintf(fp0, "   Access Permissions\n");
	    fprintf(fp0, "      Owner:  ");
	    modes[0] = '\0';
	    modes[1] = '\0';   /* In case there are no permissions */
	    modes[2] = '\0';
	    if ((dir_info.st_mode & S_IRUSR))
		strcat(modes, ", read");
	    if ((dir_info.st_mode & S_IWUSR))
		strcat(modes, ", write");
	    if ((dir_info.st_mode & S_IXUSR)) {
		if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) 
		    strcat(modes, ", search");
	        else {
		    strcat(modes, ", execute");
		    if ((dir_info.st_mode & S_ISUID))
		        strcat(modes, ", setuid");
	        }
	    }
	    fprintf(fp0, "%s\n", (char *)&modes[2]); /* Skip leading ', ' */

	    fprintf(fp0, "      Group:  ");
	    modes[0] = '\0';
	    modes[1] = '\0';   /* In case there are no permissions */
	    modes[2] = '\0';
	    if ((dir_info.st_mode & S_IRGRP)) 
		strcat(modes, ", read");
	    if ((dir_info.st_mode & S_IWGRP))
		strcat(modes, ", write");
	    if ((dir_info.st_mode & S_IXGRP)) {
		if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) 
		    strcat(modes, ", search");
	        else {
		    strcat(modes, ", execute");
		    if ((dir_info.st_mode & S_ISGID))
		        strcat(modes, ", setgid");
	        }
	    }
	    fprintf(fp0, "%s\n", (char *)&modes[2]);  /* Skip leading ', ' */

	    fprintf(fp0, "      World:  ");
	    modes[0] = '\0';
	    modes[1] = '\0';   /* In case there are no permissions */
	    modes[2] = '\0';
	    if ((dir_info.st_mode & S_IROTH))
		strcat(modes, ", read");
	    if ((dir_info.st_mode & S_IWOTH))
		strcat(modes, ", write");
	    if ((dir_info.st_mode & S_IXOTH)) {
		if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) 
		    strcat(modes, ", search");
	        else {
		    strcat(modes, ", execute");
		    if ((dir_info.st_mode & S_ISVTX))
		        strcat(modes, ", sticky");
	        }
	    }
	    fprintf(fp0, "%s\n", (char *)&modes[2]);  /* Skip leading ', ' */
	}
	fprintf(fp0,"</pre>\n");
    } else {
#endif /* DIRED_SUPPORT */

    fprintf(fp0, "<h2>File that you are currently viewing</h2>\n<dl compact>");

    StrAllocCopy(Title, doc->title);
    LYEntify(&Title, TRUE);
    fprintf(fp0,"<dt>Linkname: %s\n", Title);

    StrAllocCopy(Address, doc->address);
    LYEntify(&Address, FALSE);
    fprintf(fp0, "<dt>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;URL: %s\n", Address);

    if (doc->post_data) {
        StrAllocCopy(Address, doc->post_data);
	LYEntify(&Address, FALSE);
	fprintf(fp0, "<dt>Post Data: %s\n", Address);
	fprintf(fp0, "<dt>Post Content Type: %s\n", doc->post_content_type);
    }

    if (owner_address) {
        StrAllocCopy(Address, owner_address);
	LYEntify(&Address, FALSE);
    } else {
        StrAllocCopy(Address, "None");
    }
    fprintf(fp0, "<dt>Owner(s): %s\n", Address);

    fprintf(fp0, "<dt>&nbsp;&nbsp;&nbsp;&nbsp;size: %d lines\n", size_of_file);

    fprintf(fp0, "<dt>&nbsp;&nbsp;&nbsp;&nbsp;mode: %s\n",
		 (lynx_mode == FORMS_LYNX_MODE ? "forms mode" : "normal"));

    fprintf(fp0, "</dl>\n");  /* end of list */

    if (nlinks > 0) {
	fprintf(fp0,
	        "<h2>Link that you currently have selected</h2>\n<dl compact>");
	StrAllocCopy(Title, links[doc->link].hightext);
	LYEntify(&Title, TRUE);
	fprintf(fp0, "<dt>Linkname: %s\n", Title);
	if (lynx_mode == FORMS_LYNX_MODE &&
	    links[doc->link].type == WWW_FORM_LINK_TYPE) {
	    if (links[doc->link].form->submit_method) {
	        int method = links[doc->link].form->submit_method;
		fprintf(fp0, "<dt>&nbsp;&nbsp;Method: %s\n",
			     (method == URL_POST_METHOD) ? "POST" :
			     (method == URL_MAIL_METHOD) ? "(email)" :
							   "GET");
	    }
	    if (links[doc->link].form->submit_action) {
	        StrAllocCopy(Address, links[doc->link].form->submit_action);
		LYEntify(&Address, FALSE);
	        fprintf(fp0, "<dt>&nbsp;&nbsp;Action: %s\n", Address);
	    }
	    if (!(links[doc->link].form->submit_method &&
		links[doc->link].form->submit_action)) {
	        fprintf(fp0,"<dt>&nbsp;(Form field)\n");
	    }
	} else {
	    if (links[doc->link].lname) {
	        StrAllocCopy(Title, links[doc->link].lname);
		LYEntify(&Title, TRUE);
	    } else {
	        StrAllocCopy(Title, "");
	    }
	    fprintf(fp0, "<dt>Filename: %s\n", Title);
	}
	fprintf(fp0, "</dl>\n");  /* end of list */

    } else
	fprintf(fp0, "<h2>No Links on the current page</h2>");

#ifdef DIRED_SUPPORT
    }
#endif /* DIRED_SUPPORT */
    fprintf(fp0, "</body>\n");

    refresh();

    fclose(fp0);
    FREE(Address);
    FREE(Title);

    return(1);
}
