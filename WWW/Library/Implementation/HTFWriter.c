/*		FILE WRITER				HTFWrite.h
**		===========
**
**	This version of the stream object just writes to a C file.
**	The file is assumed open and left open.
**
**	Bugs:
**		strings written must be less than buffer size.
*/

#include <HTUtils.h>

#include <HTFWriter.h>

#include <HTFormat.h>
#include <HTAlert.h>
#include <HTFile.h>

#include <LYLeaks.h>

#define FREE(x) if (x) {free(x); x = NULL;}

/*		Stream Object
**		------------
*/

struct _HTStream {
	CONST HTStreamClass *	isa;
	
	FILE *			fp;
	char * 			end_command;
	char * 			remove_command;
	BOOL			announce;
};


/*_________________________________________________________________________
**
**		B L A C K    H O L E    C L A S S
**
**	There is only one black hole instance shared by anyone
**	who wanst a black hole.  These black holes don't radiate,
**	they just absorb data.
*/
PRIVATE void HTBlackHole_put_character ARGS2(HTStream *, me, char, c)
{}
PRIVATE void HTBlackHole_put_string ARGS2(HTStream *, me, CONST char*, s)
{}
PRIVATE void HTBlackHole_write ARGS3(HTStream *, me, CONST char*, s, int, l)
{}
PRIVATE void HTBlackHole_free ARGS1(HTStream *, me)
{}
PRIVATE void HTBlackHole_abort ARGS2(HTStream *, me, HTError, e)
{}


/*	Black Hole stream
**	-----------------
*/
PRIVATE CONST HTStreamClass HTBlackHoleClass =
{		
	"BlackHole",
	HTBlackHole_free,
	HTBlackHole_abort,
	HTBlackHole_put_character, 	HTBlackHole_put_string,
	HTBlackHole_write
}; 

PRIVATE HTStream HTBlackHoleInstance =
{
	&HTBlackHoleClass,
	NULL,
	NULL,
	NULL,
	NO
};

/*	Black hole craetion
*/
PUBLIC HTStream * HTBlackHole NOARGS
{
    return &HTBlackHoleInstance;
}


/*_________________________________________________________________________
**
**		F I L E     A C T I O N 	R O U T I N E S
**  Bug:
**	All errors are ignored.
*/

/*	Character handling
**	------------------
*/

PRIVATE void HTFWriter_put_character ARGS2(HTStream *, me, char, c)
{
    putc(c, me->fp);
}



/*	String handling
**	---------------
**
**	Strings must be smaller than this buffer size.
*/
PRIVATE void HTFWriter_put_string ARGS2(HTStream *, me, CONST char*, s)
{
    fputs(s, me->fp);
}


/*	Buffer write.  Buffers can (and should!) be big.
**	------------
*/
PRIVATE void HTFWriter_write ARGS3(HTStream *, me, CONST char*, s, int, l)
{
    fwrite(s, 1, l, me->fp); 
}




/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created
**	object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE void HTFWriter_free ARGS1(HTStream *, me)
{
    fclose(me->fp);
    if (me->end_command) {		/* Temp file */
        _HTProgress(me->end_command);	/* Tell user what's happening */
	system(me->end_command);
	FREE(me->end_command);
	if (me->remove_command) {
	    system(me->remove_command);
	    FREE(me->remove_command);
	}
    }

    FREE(me);
}

/*	End writing
*/

PRIVATE void HTFWriter_abort ARGS2(HTStream *, me, HTError, e)
{
    fclose(me->fp);
    if (me->end_command) {		/* Temp file */
	CTRACE(tfp, "HTFWriter: Aborting: file not executed.\n");
	FREE(me->end_command);
	if (me->remove_command) {
	    system(me->remove_command);
	    FREE(me->remove_command);
	}
    }

    FREE(me);
}



/*	Structured Object Class
**	-----------------------
*/
PRIVATE CONST HTStreamClass HTFWriter = /* As opposed to print etc */
{		
	"FileWriter",
	HTFWriter_free,
	HTFWriter_abort,
	HTFWriter_put_character, 	HTFWriter_put_string,
	HTFWriter_write
}; 


/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStream* HTFWriter_new ARGS1(FILE *, fp)
{
    HTStream* me;
    
    if (!fp) return NULL;

    me = (HTStream*)malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTML_new");
    me->isa = &HTFWriter;       

    me->fp = fp;
    me->end_command = NULL;
    me->remove_command = NULL;
    me->announce = NO;

    return me;
}

/*	Make system command from template
**	---------------------------------
**
**	See mailcap spec for description of template.
*/
/* @@ to be written.  sprintfs will do for now.  */



/*	Take action using a system command
**	----------------------------------
**
**	originally from Ghostview handling by Marc Andreseen.
**	Creates temporary file, writes to it, executes system command
**	on end-document.  The suffix of the temp file can be given
**	in case the application is fussy, or so that a generic opener can
**	be used.
*/
PUBLIC HTStream* HTSaveAndExecute ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	/* Not used */
	HTStream *,		sink)	/* Not used */

#ifdef unix
#define REMOVE_COMMAND "/bin/rm -f %s\n"
#endif
#ifdef VMS
#define REMOVE_COMMAND "delete/noconfirm/nolog %s.."
#endif

#ifdef REMOVE_COMMAND
{
    char *fnam;
    CONST char * suffix;
    
    HTStream* me;
    
    if (HTClientHost) {
        HTAlert("Can't save data to file -- please run WWW locally");
	return HTBlackHole();
    }
    
    me = (HTStream*)malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "Save and execute");
    me->isa = &HTFWriter;  
    
    /* Save the file under a suitably suffixed name */
    
    suffix = HTFileSuffix(pres->rep);

    fnam = (char *)malloc (L_tmpnam + 16 + strlen(suffix));
    tmpnam (fnam);
    if (suffix) strcat(fnam, suffix);
    
    me->fp = fopen (fnam, "w");
    if (!me->fp) {
	HTAlert("Can't open temporary file!");
        FREE(fnam);
	FREE(me);
	return NULL;
    }

/*	Make command to process file
*/
    me->end_command = (char *)malloc (
    			(strlen (pres->command) + 10+ 3*strlen(fnam))
    			 * sizeof (char));
    if (me == NULL) outofmem(__FILE__, "SaveAndExecute");
    
    sprintf (me->end_command, pres->command, fnam, fnam, fnam);

    me->remove_command = NULL;	/* If needed, put into end_command */
#ifdef NOPE
/*	Make command to delete file
*/ 
    me->remove_command = (char *)malloc (
    			(strlen (REMOVE_COMMAND) + 10+ strlen(fnam))
    			 * sizeof (char));
    if (me == NULL) outofmem(__FILE__, "SaveAndExecute");
    
    sprintf (me->remove_command, REMOVE_COMMAND, fnam);
#endif

    me->announce = NO;
    FREE(fnam);
    return me;
}

#else	/* can do remove */
{ return NULL; }
#endif


/*	Save Locally
**	------------
**
**  Bugs:
**	GUI Apps should open local Save panel here really.
**
*/
PUBLIC HTStream* HTSaveLocally ARGS3(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	/* Not used */
	HTStream *,		sink)	/* Not used */

{
    char *fnam;
    char *answer;
    CONST char * suffix;
    
    HTStream* me;
    
    if (HTClientHost) {
        HTAlert("Can't save data to file -- please run WWW locally");
	return HTBlackHole();
    }
    
    me = (HTStream*)malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "SaveLocally");
    me->isa = &HTFWriter;  
    me->end_command = NULL;
    me->remove_command = NULL;	/* If needed, put into end_command */
    me->announce = YES;
    
    /* Save the file under a suitably suffixed name */
    
    suffix = HTFileSuffix(pres->rep);

    fnam = (char *)malloc (L_tmpnam + 16 + strlen(suffix));
    tmpnam (fnam);
    if (suffix) strcat(fnam, suffix);
    
    /*	Save Panel */
    answer = HTPrompt("Give name of file to save in", fnam);
    
    FREE(fnam);
    
    me->fp = fopen (answer, "w");
    if (!me->fp) {
	HTAlert("Can't open local file to write into.");
        FREE(answer);
	FREE(me);
	return NULL;
    }

    FREE(answer);
    return me;
}



/*	Format Converter using system command
**	-------------------------------------
*/
