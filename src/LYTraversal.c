#include <HTUtils.h>
#include <LYGlobalDefs.h>
#include <LYUtils.h>
#include <LYSignal.h>
#include <LYClean.h>
#include <LYCurses.h>
#include <LYTraversal.h>

#include <LYexit.h>
#include <LYLeaks.h>

/* routines to handle special traversal feature */

PRIVATE void final_perror ARGS2(CONST char *,msg, BOOLEAN, clean_flag)
{
    int saved_errno = errno;
    if (LYCursesON) {
	if (clean_flag)
	    cleanup();
	else
	    stop_curses();
    }
    errno = saved_errno;
    perror(msg);
}

PRIVATE void exit_with_perror ARGS1(CONST char *,msg)
{
    final_perror(msg, TRUE);
    exit_immediately(-1);
}

PUBLIC BOOLEAN lookup ARGS1(char *,target)
{
    FILE *ifp;
    char buffer[200], line[200];

    if ((ifp = fopen(TRAVERSE_FILE,"r")) == NULL) {
	if ((ifp = LYNewTxtFile(TRAVERSE_FILE)) == NULL) {
	    exit_with_perror(CANNOT_OPEN_TRAV_FILE);
	} else {
	    fclose(ifp);
	    return(FALSE);
	}
    }

    sprintf(line,"%s\n",target);

    while(fgets(buffer, 200, ifp) != NULL) {
	if (STREQ(line,buffer)) {
	    fclose(ifp);
	    return(TRUE);
	}
    } /* end while */

    fclose(ifp);
    return(FALSE);
}

PUBLIC void add_to_table ARGS1(char *,target)
{

    FILE *ifp;

    if ((ifp = LYAppendToTxtFile(TRAVERSE_FILE)) == NULL) {
	exit_with_perror(CANNOT_OPEN_TRAV_FILE);
    }

    fprintf(ifp,"%s\n",target);

    fclose(ifp);
}

PUBLIC void add_to_traverse_list ARGS2(char *,fname, char *,prev_link_name)
{

    FILE *ifp;

    if ((ifp = LYAppendToTxtFile(TRAVERSE_FOUND_FILE)) == NULL) {
	exit_with_perror(CANNOT_OPEN_TRAF_FILE);
    }

    fprintf(ifp,"%s\t%s\n",fname, prev_link_name);

    fclose(ifp);
}

PUBLIC void dump_traversal_history NOARGS
{
    int x;
    FILE *ifp;

    if (nhist <= 0)
	return;

    if ((ifp = LYAppendToTxtFile(TRAVERSE_FILE)) == NULL) {
	final_perror(CANNOT_OPEN_TRAV_FILE, FALSE);
	return;
    }

    fprintf(ifp, "\n\n%s\n\n\t    %s\n\n",
	    TRAV_WAS_INTERRUPTED,
	    gettext("here is a list of the history stack so that you may rebuild"));

    for (x = nhist-1; x >= 0; x--) {
	fprintf(ifp,"%s\t%s\n", history[x].title, history[x].address);
    }

    fclose(ifp);
}

PUBLIC void add_to_reject_list ARGS1(char *,target)
{

    FILE *ifp;

    if ((ifp = LYAppendToTxtFile(TRAVERSE_REJECT_FILE)) == NULL) {
	exit_with_perror(CANNOT_OPEN_REJ_FILE);
    }

    fprintf(ifp,"%s\n",target);

    fclose(ifp);
}

/* there need not be a reject file, so if it doesn't open, just return
   FALSE, meaning "target not in reject file" If the last character in
   a line in a reject file is "*", then also reject if target matches up to
   that point in the string
   Blank lines are ignored
   Lines that contain just a * are allowed, but since they mean "reject
   everything" it shouldn't come up much!
 */

PUBLIC BOOLEAN lookup_reject ARGS1(char *,target)
{
    FILE *ifp;
    char buffer[200], line[200], ch;
    int  frag;

    if ((ifp = fopen(TRAVERSE_REJECT_FILE,"r")) == NULL){
	return(FALSE);
    }

    sprintf(line,"%s\n",target);

    while (fgets(buffer, 200, ifp) != NULL) {
	frag = strlen(buffer) - 1; /* real length, minus trailing null */
	ch   = buffer[frag - 1];   /* last character in buffer */
	if (frag > 0) { 	   /* if not an empty line */
	    if (ch == '*') {
		if (frag == 1 || ((strncmp(line,buffer,frag - 1)) == 0)) {
		   fclose(ifp);
		   return(TRUE);
		}
	    } else { /* last character = "*" test */
		if (STREQ(line,buffer)) {
		    fclose(ifp);
		    return(TRUE);
		}
	    } /* last character = "*" test */
	} /* frag >= 0 */
    } /* end while */

    fclose(ifp);
    return(FALSE);
}
