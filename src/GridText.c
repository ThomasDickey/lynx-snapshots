/*		Character grid hypertext object
**		===============================
*/

#include "HTUtils.h"
#include "tcp.h"

#include "LYCurses.h" /* lynx defined curses */

#include <assert.h>
#include <ctype.h>
#include "HTString.h"
#include "GridText.h"
#include "HTFont.h"
#include "HTAccess.h"
#include "HTParse.h"
#include "HTTP.h"
#include "HTAlert.h"
#include "HTCJK.h"

/* lynx specific defines */
#include "LYUtils.h"
#include "LYStrings.h"
#include "LYStructs.h"
#include "LYGlobalDefs.h"
#include "LYGetFile.h"
#include "LYSignal.h"
#include "LYMail.h"
#include "LYList.h"
#include "LYCharSets.h"

#include "LYexit.h"
#include "LYLeaks.h"
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
#include <syslog.h>
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

struct _HTStream {                      /* only know it as object */
    CONST HTStreamClass *       isa;
    /* ... */
};

#define TITLE_LINES  1

#define FREE(x) if (x) {free(x); x = NULL;}

extern BOOL HTPassHighCtrlRaw;
extern HTkcode kanji_code;
extern HTCJKlang HTCJK;

/*	From default style sheet:
*/
extern HTStyleSheet * styleSheet;	/* Default or overridden */

extern int display_lines; /* number of lines in display */

/*	Exports
*/ 
PUBLIC HText * HTMainText = NULL;		/* Equivalent of main window */
PUBLIC HTParentAnchor * HTMainAnchor = NULL;	/* Anchor for HTMainText */

PUBLIC char * HTAppName = "Lynx";		/* Application name */
PUBLIC char * HTAppVersion = LYNX_VERSION;	/* Application version */

PUBLIC int HTFormNumber = 0;
PUBLIC int HTFormFields = 0;
PUBLIC char * HTCurSelectGroup = NULL;		/* Form select group name */
PUBLIC int HTCurSelectGroupType = F_RADIO_TYPE;	/* Group type */
PUBLIC char * HTCurSelectGroupSize = NULL;	/* Length of select */
PRIVATE char * HTCurSelectedOptionValue = NULL;	/* Select choice */

PUBLIC char * checked_box = "[X]";
PUBLIC char * unchecked_box = "[ ]";
PUBLIC char * checked_radio = "(*)";
PUBLIC char * unchecked_radio = "( )";

PUBLIC BOOLEAN underline_on = OFF;
PUBLIC BOOLEAN bold_on      = OFF;

typedef struct _line {
	struct _line	*next;
	struct _line	*prev;
	int unsigned	offset;		/* Implicit initial spaces */
	int unsigned	size;		/* Number of characters */
	BOOL	split_after;		/* Can we split after? */
	BOOL	bullet;			/* Do we bullet? */
	char	data[1];		/* Space for terminator at least! */
} HTLine;

#define LINE_SIZE(l) (sizeof(HTLine)+(l))	/* Allow for terminator */

typedef struct _TextAnchor {
	struct _TextAnchor *	next;
	int			number;		/* For user interface */
	int			start;		/* Characters */
        int			line_pos;	/* Position in text */
	int			extent;		/* Characters */
	int			line_num;	/* Place in document */
	char *		        hightext;	/* The link text */
	char *		        hightext2;	/* A second line*/
        int 		    	hightext2offset;/* offset from left */
	int			link_type;	/* Normal or form? */
	FormInfo *		input_field;	/* Info for form links */
	BOOL			show_anchor;	/* Show the anchor? */
	HTChildAnchor *		anchor;
} TextAnchor;

typedef struct _HTTabID {
	char *			name;		/* ID value of TAB */
	int			column;		/* Zero-based column value */
} HTTabID;


/*	Notes on struct _Htext:
**	next_line is valid if state is false.
**	top_of_screen line means the line at the top of the screen
**			or just under the title if there is one.
*/
struct _HText {
	HTParentAnchor *	node_anchor;
	HTLine * 		last_line;
	int			Lines;		/* Number of them */
	int			chars;		/* Number of them */
	TextAnchor *		first_anchor;	/* Singly linked list */
	TextAnchor *		last_anchor;
	int			last_anchor_number;	/* user number */
	BOOL			source;		/* Is the text source? */
	BOOL			toolbar;	/* Toolbar set? */
	HTList *		tabs;		/* TAB IDs */
	BOOL			no_cache;	/* Always refresh? */
	char			LastChar;	/* For aborbing white space */
	BOOL			IgnoreExcess;	/* Ignore chars at wrap point */

/* For Internal use: */	
	HTStyle *		style;			/* Current style */
	int			display_on_the_fly;	/* Lines left */
	int			top_of_screen;		/* Line number */
	HTLine *		top_of_screen_line;	/* Top */
	HTLine *		next_line;		/* Bottom + 1 */
	int			permissible_split;	/* in last line */
	BOOL			in_line_1;		/* of paragraph */
	BOOL			stale;			/* Must refresh */

        HTkcode			kcode;			/* Kanji code? */
	enum grid_state	      { S_text, S_esc, S_dollar, S_paren,
				S_nonascii_text, S_dollar_paren,
				S_jisx0201_text }
				state;			/* Escape sequence? */
	char			kanji_buf;		/* Lead multibyte */
	int			in_sjis;		/* SJIS flag */

        HTStream*               target;                 /* Output stream */
        HTStreamClass           targetClass;            /* Output routines */
};

/*
 *  Boring static variable used for moving cursor across
 */
#define UNDERSCORES(n) \
 ((n) >= MAX_LINE ? underscore_string : &underscore_string[(MAX_LINE-1)] - (n))

/*
 *	Memory leak fixed.
 *	05-29-94 Lynx 2-3-1 Garrett Arch Blythe
 *	Changed to arrays.
 */
PRIVATE char underscore_string[MAX_LINE + 1];
PUBLIC char star_string[MAX_LINE + 1];

PRIVATE int ctrl_chars_on_this_line = 0; /* num of ctrl chars in current line */

PRIVATE HTStyle default_style =
	{ 0,  "(Unstyled)", "",
	(HTFont)0, 1.0, HT_BLACK,		0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	0, 
	NO, NO, 0, 0,			0 };	



PRIVATE HTList * loaded_texts = NULL;	 /* A list of all those in memory */
PUBLIC  HTList * search_queries = NULL;  /* isindex and whereis queries   */
PRIVATE void free_all_texts NOARGS;
PRIVATE int HText_TrueLineSize PARAMS((HTLine *line));

/*			Creation Method
**			---------------
*/
PUBLIC HText *	HText_new ARGS1(
	HTParentAnchor *,	anchor)
{
#if defined(VMS) && defined(VAXC) && !defined(__DECC)
#include <lib$routines.h>
    int status, VMType=3, VMTotal;
#endif /* VMS && VAXC && !__DECC */
    HTLine * line = NULL;
    HText * self = (HText *) calloc(1, sizeof(*self));
    if (!self)
        return self;
    
#if defined(VMS) && defined (VAXC) && !defined(__DECC)
    status = lib$stat_vm(&VMType, &VMTotal);
    if (TRACE)
        fprintf(stderr, "GritText: VMTotal = %d\n", VMTotal);
#endif /* VMS && VAXC && !__DECC */

    if (!loaded_texts)	{
	loaded_texts = HTList_new();
	atexit(free_all_texts);
    }

    /*
     *  Links between anchors & documents are a 1-1 relationship. If
     *  an anchor is already linked to a document we didn't call
     *  HTuncache_current_document(), e.g., for the showinfo, options,
     *  dowload, print, etc., temporary file URLs, so we'll check now
     *  and free it before reloading. - Dick Wesseling (ftu@fi.ruu.nl)
     */
    if (anchor->document) {
       HTList_removeObject(loaded_texts, anchor->document);
       if (TRACE)
           fprintf(stderr, "GridText: Auto-uncaching\n") ;
       ((HText *)anchor->document)->node_anchor = NULL;
       HText_free((HText *)anchor->document);
       anchor->document = NULL;
    }

    HTList_addObject(loaded_texts, self);
#if defined(VMS) && defined(VAXC) && !defined(__DECC)
    while (HTList_count(loaded_texts) > HTCacheSize &&
    	   VMTotal > HTVirtualMemorySize) {
#else
    if (HTList_count(loaded_texts) > HTCacheSize) {
#endif /* VMS && VAXC && !__DECC */
        if (TRACE)
	    fprintf(stderr, "GridText: Freeing off cached doc.\n"); 
        HText_free((HText *)HTList_removeFirstObject(loaded_texts));
#if defined(VMS) && defined (VAXC) && !defined(__DECC)
        status = lib$stat_vm(&VMType, &VMTotal);
        if (TRACE)
            fprintf(stderr, "GridText: VMTotal reduced to %d\n", VMTotal);
#endif /* VMS && VAXC && !__DECC */
    }
    
    line = self->last_line = (HTLine *)calloc(1, LINE_SIZE(MAX_LINE));
    if (line == NULL)
        outofmem(__FILE__, "HText_New");
    line->next = line->prev = line;
    line->offset = line->size = 0;
    self->Lines = self->chars = 0;
    self->first_anchor = self->last_anchor = NULL;
    self->style = &default_style;
    self->top_of_screen = 0;
    self->node_anchor = anchor;
    self->last_anchor_number = 0;	/* Numbering of them for references */
    self->stale = YES;
    self->toolbar = NO;
    self->tabs = NULL;
    self->no_cache = ((anchor->no_cache || anchor->post_data) ?
    							  YES : NO);
    self->LastChar = '\0';
    self->IgnoreExcess = FALSE;

    if (HTOutputFormat == WWW_SOURCE)
        self->source = YES;
    else
        self->source = NO;
    HTAnchor_setDocument(anchor, (HyperDoc *)self);
    HTFormNumber = 0;  /* no forms started yet */
    HTMainText = self;
    HTMainAnchor = anchor;
    self->display_on_the_fly = 0;
    self->kcode = NOKANJI;
    self->state = S_text;
    self->kanji_buf = '\0';
    self->in_sjis = 0;

    /*
     *  Check the kcode setting if the anchor has a charset element. - FM
     */
    if (anchor->charset)
        HText_setKcode(self, anchor->charset);

    /*
     *	Memory leak fixed.
     *  05-29-94 Lynx 2-3-1 Garrett Arch Blythe
     *	Check to see if our underline and star_string need initialization
     *		if the underline is not filled with dots.
     */ 
    if (underscore_string[0] != '.') {
        char *p;
	/*
	 *  Create and array of dots for the UNDERSCORES macro. - FM
	 */
	memset(underscore_string, '.', (MAX_LINE-1));
        underscore_string[(MAX_LINE-1)] = '\0';
        underscore_string[MAX_LINE] = '\0';
	/*
	 *  Create and array of underscores for the STARS macro. - FM
	 */
	memset(star_string, '_', (MAX_LINE-1));
        star_string[(MAX_LINE-1)] = '\0';
        star_string[MAX_LINE] = '\0';
    }

    underline_on = FALSE; /* reset */
    bold_on = FALSE;
    
    return self;
}

/*                      Creation Method 2
**                      ---------------
**
**      Stream is assumed open and left open.
*/
PUBLIC HText *  HText_new2 ARGS2(
	HTParentAnchor *,	anchor,
	HTStream *,		stream)

{
    HText * this = HText_new(anchor);

    if (stream) {
        this->target = stream;
        this->targetClass = *stream->isa;       /* copy action procedures */
    }
    return this;
}

/*	Free Entire Text
**	----------------
*/
PUBLIC void HText_free ARGS1(
	HText *,	self)
{
    if (!self)
        return;

    HTAnchor_setDocument(self->node_anchor, (HyperDoc *)0);
    
    while (YES) {	/* Free off line array */
        HTLine * l = self->last_line;
	if (l) {
	    l->next->prev = l->prev;
	    l->prev->next = l->next;	/* Unlink l */
	    self->last_line = l->prev;
	    if (l != self->last_line) {
	        FREE(l);
	    } else {
	        free(l);
	    }
	}
	if (l == self->last_line) {	/* empty */
	    l = NULL;
	    break;
	}
    };
    
    while (self->first_anchor) {		/* Free off anchor array */
        TextAnchor * l = self->first_anchor;
	self->first_anchor = l->next;

	if (l->link_type == INPUT_ANCHOR && l->input_field) {
	    /*
	     *  Free form fields.
	     */
	    if (l->input_field->type == F_OPTION_LIST_TYPE) {
		/*
		 *  Free off option lists.
		 */
		OptionType *optptr = l->input_field->select_list;
		OptionType *tmp;
		while (optptr) {
		    tmp = optptr;
		    optptr = tmp->next;
		    FREE(tmp->name);
		    FREE(tmp->cp_submit_value);
		    FREE(tmp);
		}
		l->input_field->select_list = NULL;
		/* 
		 *  Don't free the value field on option
		 *  lists since it points to a option value
		 *  same for orig value.
		 */
		l->input_field->value = NULL;
		l->input_field->orig_value = NULL;
		l->input_field->cp_submit_value = NULL;
		l->input_field->orig_submit_value = NULL;
	    } else {
                FREE(l->input_field->value);
                FREE(l->input_field->orig_value);
                FREE(l->input_field->cp_submit_value);
                FREE(l->input_field->orig_submit_value);
	    }
	    FREE(l->input_field->name);
            FREE(l->input_field->submit_action);
            FREE(l->input_field->submit_enctype);
            FREE(l->input_field->submit_title);
		
	    FREE(l->input_field);
	}

	FREE(l->hightext);
	FREE(l->hightext2);

	FREE(l);
    }

    /*
     *  Free the tabs list. - FM
     */
    if (self->tabs) {
        HTTabID * Tab = NULL;
	HTList * cur = self->tabs;

	while (NULL != (Tab = (HTTabID *)HTList_nextObject(cur))) {
	    FREE(Tab->name);
	}
	HTList_delete(self->tabs);
    }

    /*
     *  Invoke HTAnchor_delete() to free the node_anchor
     *  if it is not a destination of other links. - FM
     */
    if (self->node_anchor) {
        HTAnchor_delete(self->node_anchor);
    }

    FREE(self);
}

/*		Display Methods
**		---------------
*/


/*	Output a line
**	-------------
*/
PRIVATE int display_line ARGS1(
	HTLine *,	line)
{
    register int i,j;
    char buffer[3];
    char *data;

    buffer[0] = buffer[1] = buffer[2] = '\0';
    clrtoeol();
    /* make sure that we don't go over the COLS limit on the display! */

    /* add offset */
    j = (int)line->offset;
    if (j > (int)LYcols - 1)
        j = (int)LYcols - 1;
#ifdef USE_SLANG
    SLsmg_forward (j);
    i = j;
#else
    for (i = 0; i < j; i++)
        addch (' ');
#endif /* USE_SLANG */

    /* add data */
    data = line->data;
    i++;
    while ((i < LYcols) && ((buffer[0] = *data) != '\0')) {
	data++;
	i++;

   	switch (buffer[0]) {

	    case LY_UNDERLINE_START_CHAR:
	        if (!(dump_output_immediately && use_underscore)) {
		    start_underline();
		    i--;
		} else {
		    addch('_');
		}
		break;

	    case LY_UNDERLINE_END_CHAR:
	        if (!(dump_output_immediately && use_underscore)) {
		    stop_underline();
		    i--;
		} else {
		    addch('_');
		}
		break;

	    case LY_BOLD_START_CHAR:
	        start_bold();
		i--;
		break;

	    case LY_BOLD_END_CHAR:
	        stop_bold ();
		i--;
		break;

	    case LY_SOFT_HYPHEN:
	        if (*data != '\0') {
		    /*
		     *  Ignore the soft hyphen if it is not
		     *  the last character in the line. - FM
		     */
		    i--;
		    break;
		} else {
		    /*
		     *  Make it a hard hyphen and fall through. - FM
		     */
		    buffer[0] = '-';
		}

	    default:
		/* For CJK strings, by Masanobu Kimura */
		if (HTCJK != NOCJK && !isascii(buffer[0])) { 
		    buffer[1] = *data;
		    data++;
		    i++;
		    addstr(buffer);
		    buffer[1] = '\0';
		} else {
		    addstr(buffer);
		}
	} /* end of switch */
    } /* end of while */

    /* add the return */
    addch('\n');

    stop_underline();
    stop_bold();
    return(0);
}

/*	Output the title line
**	---------------------
*/
PRIVATE void display_title ARGS1(
	HText *,	text)
{
    char *title = NULL;
    char percent[20], format[20];
    char *cp = NULL;
    unsigned char *tmp = NULL;
    int i = 0, j = 0;

    /*
     *  Make sure we have a text structure. - FM
     */
    if (!text)
        return;

    /*
     *  Load the title field. - FM
     */
    StrAllocCopy(title,
    		 (HTAnchor_title(text->node_anchor) ?
		  HTAnchor_title(text->node_anchor) : ""));

    /*
     *  There shouldn't be any \n in the title field,
     *  but if there is, lets kill it now!
     */
    if ((cp = strchr(title,'\n')) != NULL)
	*cp = '\0';

    /*
     *  Generate the page indicator (percent) string.
     */
    if ((text->Lines + 1) > (display_lines)) {
	/*
	 *  In a small attempt to correct the number of pages counted....
	 *    GAB 07-14-94
	 *
	 *  In a bigger attempt (hope it holds up 8-)....
	 *    FM 02-08-95
	 */
	int total_pages =
	 	(((text->Lines + 1) + (display_lines - 1))/(display_lines));
	int start_of_last_page =
		((text->Lines + 1) < display_lines) ? 0 :
		((text->Lines + 1) - display_lines);	

	sprintf(percent, " (p%d of %d)",
		((text->top_of_screen >= start_of_last_page) ?
						 total_pages :
	            ((text->top_of_screen + display_lines)/(display_lines))),
		total_pages);
    } else {
	strcpy(percent, "");	/* Null string */
    }

    /*
     *  Generate format string.
     */
    sprintf(format, "%s%%%d.%ds%%s\n",
    		    ((text->top_of_screen > 0 &&
		      HText_hasToolbar(text)) ?
		      			  "#" : " "),
		    (LYcols-2)-strlen(percent),
		    (LYcols-2)-strlen(percent));

    /*
     *  Generate and display the complete title string.
     */
    cp = (char *)calloc(1, (LYcols * 2));
    if (cp == NULL)
        outofmem(__FILE__, "display_title");
    if (HTCJK != NOCJK) {
        if (*title &&
	    (tmp = (unsigned char *)calloc(1, (strlen(title) + 1)))) {
	    if (kanji_code == EUC) {
	        TO_EUC((unsigned char *)title, tmp);
	    } else if (kanji_code == SJIS) {
	        TO_SJIS((unsigned char *)title, tmp);
	    } else {
	        for (i = 0, j = 0; title[i]; i++) {
		    if (title[i] != '\033') {
		        tmp[j++] = title[i];
		    }
		}
	    }
	    sprintf(cp, format, tmp, percent);
	    FREE(tmp);
        } else {
	    sprintf(cp, format, title, percent);
        }
    } else {
        sprintf(cp, format, title, percent);
    }
    move(0,0);
    addstr(cp);
    FREE(cp);
    FREE(title);
    return;
}


/*	Output a page
**	-------------
*/
PRIVATE void display_page ARGS3(
	HText *,	text,
	int,		line_number,
	char *,		target)
{
    HTLine * line = NULL;
    int i;
    char *cp, tmp[3];
    int last_screen;
    TextAnchor *Anchor_ptr = NULL;
    FormInfo *FormInfo_ptr;
    BOOL display_flag = FALSE;
    HTAnchor *link_dest;
    static int last_nlinks = 0;

    lynx_mode = NORMAL_LYNX_MODE;
 
    if (text == NULL) {
	/*
	 *  Check whether to force a screen clear to enable scrollback,
	 *  or as a hack to fix a reverse clear screen problem for some
	 *  curses packages. - shf@access.digex.net & seldon@eskimo.com
	 */
	if (enable_scrollback) {
	    addch('*');
	    refresh();
	    clear();
	}
	addstr("\n\nError accessing document\nNo data available\n");
	refresh();
	nlinks = 0;  /* set number of links to 0 */
	return;
    }

    tmp[0] = tmp[1] = tmp[2] = '\0';
    last_screen = text->Lines - (display_lines-2);
    line = text->last_line->prev;

    /*
     *  Constrain the line number to be within the document.
     */
    if (text->Lines < (display_lines))
        line_number = 0;
    else if (line_number > text->Lines)
        line_number = last_screen;
    else if (line_number < 0)
        line_number = 0;
    
    for (i = 0, line = text->last_line->next;		/* Find line */
    	 i < line_number && (line != text->last_line);
         i++, line = line->next) 			/* Loop */
	assert(line->next != NULL);

    /*
     *  Check whether to force a screen clear to enable scrollback,
     *  or as a hack to fix a reverse clear screen problem for some
     *  curses packages. - shf@access.digex.net & seldon@eskimo.com
     */
    if (enable_scrollback) {
	addch('*');
	refresh();
	clear();
    }

    text->top_of_screen = line_number;
    display_title(text);  /* will move cursor to top of screen */
    display_flag=TRUE;
    
    /*
     *  Print it.
     */
    if (line) {
      for (i = 0; i < (display_lines); i++)  {

        assert(line != NULL);
        display_line(line);

        /*
	 *  If the target is on this line, underline it.
	 */
        if (strlen(target) > 0 &&
	    (case_sensitive ?  
	    (cp = LYno_attr_char_strstr(line->data, target)) != NULL : 
	    (cp = LYno_attr_char_case_strstr(line->data, target)) != NULL) &&
            ((int)(cp - (char *)line->data) +
	     (int)line->offset + strlen(target)) < LYcols) {

	    int itmp = 0;
	    int written = 0;
	    int x_pos=(int)line->offset + (int)(cp - line->data);
	    int len = strlen(target);

	    start_underline();
		/* underline string */
	    for (; written < len && (tmp[0] = line->data[itmp]) != '\0';
		   itmp++)  {
		if (IsSpecialAttrChar(tmp[0])) {
		   /* ignore special characters */
		   x_pos--;

		} else if (cp == &line->data[itmp]) {
		    /* first character of target */
            	    move(i+1, x_pos);
		    if (HTCJK != NOCJK && !isascii(tmp[0])) {
		        /* For CJK strings, by Masanobu Kimura */
		        tmp[1] = line->data[++itmp];
			addstr(tmp);
			tmp[1] = '\0';
			written += 2;
		    } else {
		        addstr(tmp);
			written++;
		    }

		} else if (&line->data[itmp] > cp) { 
		    /* print all the other target chars */
		    if (HTCJK != NOCJK && !isascii(tmp[0])) {
		        /* For CJK strings, by Masanobu Kimura */
		        tmp[1] = line->data[++itmp];
			addstr(tmp);
			tmp[1] = '\0';
			written += 2;
		    } else {
		        addstr(tmp);
			written++;
		    }
		}
	    }

	    stop_underline();
	    move(i+2, 0);
	}

	/*
	 *  Stop if at the last line.
	 */
	if (line == text->last_line)  {
	    /* clr remaining lines of display */
	    for (i++; i < (display_lines); i++) {
		move(i+1,0);
		clrtoeol();
	    }
	    break;
	}

	display_flag=TRUE;
	line = line->next;
      }
    }

    text->next_line = line;	/* Line after screen */
    text->stale = NO;		/* Display is up-to-date */

    /*
     *  Add the anchors to Lynx structures.
     */
    nlinks = 0;
    for (Anchor_ptr=text->first_anchor;  Anchor_ptr != NULL &&
		Anchor_ptr->line_num <= line_number+(display_lines);
					    Anchor_ptr = Anchor_ptr->next) {

	if (Anchor_ptr->line_num >= line_number &&
		Anchor_ptr->line_num < line_number+(display_lines)) {

		/* load normal hypertext anchors */
	    if (Anchor_ptr->show_anchor && Anchor_ptr->hightext && 
			strlen(Anchor_ptr->hightext)>0 && 
			Anchor_ptr->link_type == HYPERTEXT_ANCHOR) {

                links[nlinks].hightext  = Anchor_ptr->hightext;
                links[nlinks].hightext2 = Anchor_ptr->hightext2;
                links[nlinks].hightext2_offset = Anchor_ptr->hightext2offset;

                links[nlinks].anchor_number = Anchor_ptr->number;

		link_dest = HTAnchor_followMainLink(
					     (HTAnchor *)Anchor_ptr->anchor);
		{
		    /*
		     *	Memory leak fixed 05-27-94
		     *	Garrett Arch Blythe
		     */
	            auto char *cp_AnchorAddress = NULL;
		    if (traversal)
		        cp_AnchorAddress = stub_HTAnchor_address(link_dest);
		    else
			cp_AnchorAddress = HTAnchor_address(link_dest);

		    FREE(links[nlinks].lname);

		    if (cp_AnchorAddress != NULL)
			links[nlinks].lname = cp_AnchorAddress;
		    else
			StrAllocCopy(links[nlinks].lname, empty_string);
		}

      	        links[nlinks].lx = Anchor_ptr->line_pos;
      	        links[nlinks].ly = (Anchor_ptr->line_num+1)-line_number;
	        links[nlinks].type = WWW_LINK_TYPE;
		links[nlinks].target = empty_string;

	        nlinks++;
		display_flag = TRUE;

	    } else if (Anchor_ptr->link_type == INPUT_ANCHOR
			&& Anchor_ptr->input_field->type != F_HIDDEN_TYPE) {

		lynx_mode = FORMS_LYNX_MODE;

		FormInfo_ptr = Anchor_ptr->input_field;

                links[nlinks].anchor_number = Anchor_ptr->number;

	   	links[nlinks].form = FormInfo_ptr;
		links[nlinks].lx = Anchor_ptr->line_pos;
		links[nlinks].ly = (Anchor_ptr->line_num+1)-line_number;
		links[nlinks].type = WWW_FORM_LINK_TYPE;
		links[nlinks].target = empty_string;
		StrAllocCopy(links[nlinks].lname, empty_string);

		if (FormInfo_ptr->type == F_RADIO_TYPE) {
		    if (FormInfo_ptr->num_value)
			links[nlinks].hightext = checked_radio;
		    else
			links[nlinks].hightext = unchecked_radio;

		} else if (FormInfo_ptr->type == F_CHECKBOX_TYPE) {
		    if (FormInfo_ptr->num_value)
			links[nlinks].hightext = checked_box;
		    else
			links[nlinks].hightext = unchecked_box;

		} else if (FormInfo_ptr->type == F_PASSWORD_TYPE) {
		    links[nlinks].hightext = STARS(strlen(FormInfo_ptr->value));

		} else {  /* TEXT type */
		    links[nlinks].hightext = FormInfo_ptr->value;
		}

  		/* never a second line on form types */
		links[nlinks].hightext2 = NULL;

		nlinks++;
	         /* bold the link after incrementing nlinks */
		highlight(OFF,nlinks-1);
	
		display_flag = TRUE;

	    } else { /* not showing anchor */
		if (TRACE &&
		    Anchor_ptr->hightext && *Anchor_ptr->hightext) 
		    fprintf(stderr,
		    	    "\nGridText: Not showing link, hightext=%s\n",
			    Anchor_ptr->hightext);
	    }
	} 

	if (Anchor_ptr == text->last_anchor)
	    /*
	     *  No more links in document. - FM
	     */
	    break;

	if (nlinks == MAXLINKS) {
	    /*
	     *  Links array is full.  If interactive, tell user
	     *  to use half-page or two-line scrolling. - FM
	     */
	    if (LYCursesON) {
		_statusline(MAXLINKS_REACHED);
		sleep(AlertSecs);
	    } 
	    if (TRACE)
	        fprintf(stderr, "\ndisplay_page: MAXLINKS reached.\n");
	    break;
	}
    }

    /*
     *  Free any un-reallocated links[] entries
     *  from the previous page draw. - FM
     */
    for (i = nlinks; i < last_nlinks; i++)
        FREE(links[i].lname);
    last_nlinks = nlinks;

    /*
     *  If Anchor_ptr is not NULL and is not pointing to the last
     *  anchor, then there are anchors farther down in the document,
     *  and we need to flag this for traversals.
     */
    more_links = FALSE;
    if (traversal && Anchor_ptr) {
        if (Anchor_ptr->next)
            more_links = TRUE;
    }

    if (!display_flag) /* nothing on the page */
	addstr("\n     Document is empty");

    if (HTCJK != NOCJK || TRACE) {
        /* for non-multibyte curses ;_; */
        clearok(curscr, TRUE);
    }
    refresh();

}


/*			Object Building methods
**			-----------------------
**
**	These are used by a parser to build the text in an object
*/
PUBLIC void HText_beginAppend ARGS1(
	HText *,	text)
{
    text->permissible_split = 0;
    text->in_line_1 = YES;

}


/*	Add a new line of text
**	----------------------
**
** On entry,
**
**	split	is zero for newline function, else number of characters
**		before split.
**	text->display_on_the_fly
**		may be set to indicate direct output of the finished line.
** On exit,
**		A new line has been made, justified according to the
**		current style. Text after the split (if split nonzero)
**		is taken over onto the next line.
**
**		If display_on_the_fly is set, then it is decremented and
**		the finished line is displayed.
*/
#define new_line(text) split_line(text, 0)

PRIVATE void split_line ARGS2(
	HText *,	text,
	int,		split)
{
    HTStyle * style = text->style;
    HTLine * temp;
    int spare;
    int indent = text->in_line_1 ?
    	  text->style->indent1st : text->style->leftIndent;

    /*
     *  Make new line.
     */
    HTLine * previous = text->last_line;
    int ctrl_chars_on_previous_line = 0;
    char * cp;
    HTLine * line = (HTLine *)calloc(1, LINE_SIZE(MAX_LINE));

    ctrl_chars_on_this_line = 0; /*reset since we are going to a new line*/
    text->LastChar = ' ';

    if (TRACE)
	fprintf(stderr,"GridText: split_line called\n");
    
    if (line == NULL)
        outofmem(__FILE__, "split_line");
    text->Lines++;
    
    previous->next->prev = line;
    line->prev = previous;
    line->next = previous->next;
    previous->next = line;
    text->last_line = line;
    line->size = 0;
    line->offset = 0;
    text->permissible_split = 0;  /* 12/13/93 */
    line->data[0] = '\0';

    /*
     *  If we are not splitting and need an underline char, add it now. - FM
     */
    if ((split < 1) &&
        !(dump_output_immediately && use_underscore) && underline_on) {
	line->data[line->size++] = LY_UNDERLINE_START_CHAR;
	line->data[line->size] = '\0';
	ctrl_chars_on_this_line++;
    }
    /*
     *  If we are not splitting and need a bold char, add it now. - FM
     */
    if ((split < 1) && bold_on) {
	line->data[line->size++] = LY_BOLD_START_CHAR;
	line->data[line->size] = '\0';
	ctrl_chars_on_this_line++;
    }

    /*
     *  Split at required point
     */    
    if (split > 0) {	/* Delete space at "split" splitting line */
        char *p, *prevdata = previous->data, *linedata = line->data;
        unsigned int plen;
	int i;

        /*
	 *  Split the line. - FM
	 */
	prevdata[previous->size] = '\0';
	previous->size = split;

	/*
	 *  Trim any spaces or soft hyphens from the beginning
	 *  of our new line. - FM
	 */
	p = prevdata + split;
        while (*p == ' ' || *p == LY_SOFT_HYPHEN)
	    p++;
        plen = strlen(p);

	/*
	 *  Add underline char if needed. - FM
	 */
        if (!(dump_output_immediately && use_underscore)) {
	    /*
	     * Make sure our global flag is correct. - FM
	     */
	    underline_on = NO;
	    for (i = (split-1); i >= 0; i--) {
	        if (prevdata[i] == LY_UNDERLINE_END_CHAR) {
		    break;
		}
		if (prevdata[i] == LY_UNDERLINE_START_CHAR) {
		    underline_on = YES;
		    break;
		}
	    }
	    /*
	     *  Act on the global flag if set above. - FM
	     */
	    if (underline_on && *p != LY_UNDERLINE_END_CHAR) {
	        linedata[line->size++] = LY_UNDERLINE_START_CHAR;
		linedata[line->size] = '\0';
		ctrl_chars_on_this_line++;
	    }
	    for (i = (plen - 1); i >= 0; i--) {
		if (p[i] == LY_UNDERLINE_START_CHAR) {
		    underline_on = YES;
		    break;
		}
		if (p[i] == LY_UNDERLINE_END_CHAR) {
		    underline_on = NO;
		    break;
		}
	    }
	    for (i = (plen - 1); i >= 0; i--) {
	        if (p[i] == LY_UNDERLINE_START_CHAR ||
		    p[i] == LY_UNDERLINE_END_CHAR) {
		    ctrl_chars_on_this_line++;
		}
	    }
	}

	/*
	 *  Add bold char if needed, first making
	 *  sure that our global flag is correct. - FM
	 */
	bold_on = NO;
	for (i = (split - 1); i >= 0; i--) {
	    if (prevdata[i] == LY_BOLD_END_CHAR) {
		break;
	    }
	    if (prevdata[i] == LY_BOLD_START_CHAR) {
	        bold_on = YES;
		break;
	    }
	}
	/*
	 *  Act on the global flag if set above. - FM
	 */
	if (bold_on && *p != LY_BOLD_END_CHAR) {
	    linedata[line->size++] = LY_BOLD_START_CHAR;
	    linedata[line->size] = '\0';
	    ctrl_chars_on_this_line++;
	}
	for (i = (plen - 1); i >= 0; i--) {
	    if (p[i] == LY_BOLD_START_CHAR) {
	        bold_on = YES;
		break;
	    }
	    if (p[i] == LY_BOLD_END_CHAR) {
		bold_on = NO;
		break;
	    }
	}
	for (i = (plen - 1); i >= 0; i--) {
	    if (p[i] == LY_BOLD_START_CHAR ||
	        p[i] == LY_BOLD_END_CHAR ||
		p[i] == LY_SOFT_HYPHEN) {
	        ctrl_chars_on_this_line++;
	    }
	    if (p[i] == LY_SOFT_HYPHEN && text->permissible_split < i) {
	        text->permissible_split = i + 1;
	    }
	}

	/*
	 *  Add the data to the new line. - FM
	 */
	strcat(linedata, p);
	line->size += plen;
    }

    /*
     *  Economize on space.
     */
    while ((previous->size > 0) &&
    	(previous->data[previous->size-1] == ' ')) {
	/*
	 *  Strip trailers.
	 */
	previous->data[previous->size-1] = '\0';
        previous->size--;
    }
    temp = (HTLine *)calloc(1, LINE_SIZE(previous->size));
    if (temp == NULL)
        outofmem(__FILE__, "split_line");
    memcpy(temp, previous, LINE_SIZE(previous->size));
    FREE(previous);
    previous = temp;

    previous->prev->next = previous;	/* Link in new line */
    previous->next->prev = previous;	/* Could be same node of course */

    /*
     *  Terminate finished line for printing.
     */
    previous->data[previous->size] = '\0';
     
    
    /*
     *  Align left, right or center.
     */
    for (cp = previous->data; *cp; cp++) {
        if (*cp == LY_UNDERLINE_START_CHAR ||
	    *cp == LY_UNDERLINE_END_CHAR ||
	    *cp == LY_BOLD_START_CHAR ||
	    *cp == LY_BOLD_END_CHAR ||
	    *cp == LY_SOFT_HYPHEN)
	    ctrl_chars_on_previous_line++;
    }
    /* @@ first line indent */
    spare =  (LYcols-1) -
    		(int)style->rightIndent - indent +
    		ctrl_chars_on_previous_line - previous->size -
		((previous->size > 0) &&
		 (int)(previous->data[previous->size-1] ==
		 			    LY_SOFT_HYPHEN ?
							 1 : 0));

    switch (style->alignment) {
	case HT_CENTER :
	    previous->offset = previous->offset + indent + spare/2;
	    break;
	case HT_RIGHT :
	    previous->offset = previous->offset + indent + spare;
	    break;
	case HT_LEFT :
	case HT_JUSTIFY :		/* Not implemented */
	default:
	    previous->offset = previous->offset + indent;
	    break;
    } /* switch */

    text->chars = text->chars + previous->size + 1;	/* 1 for the line */
    text->in_line_1 = NO;		/* unless caller sets it otherwise */
    
} /* split_line */


/*	Allow vertical blank space
**	--------------------------
*/
PRIVATE void blank_lines ARGS2(
	HText *,	text,
	int,		newlines)
{
    if (!HText_LastLineSize(text)) {	/* No text on current line */
	HTLine * line = text->last_line->prev;
	while ((line != text->last_line) &&
	       (HText_TrueLineSize(line) == 0)) {
	    if (newlines == 0)
	        break;
	    newlines--;		/* Don't bother: already blank */
	    line = line->prev;
	}
    } else {
	newlines++;			/* Need also to finish this line */
    }

    for (; newlines; newlines--) {
	new_line(text);
    }
    text->in_line_1 = YES;
}


/*	New paragraph in current style
**	------------------------------
** See also: setStyle.
*/
PUBLIC void HText_appendParagraph ARGS1(
	HText *,	text)
{
    int after = text->style->spaceAfter;
    int before = text->style->spaceBefore;
    blank_lines(text, ((after > before) ? after : before));
}


/*	Set Style
**	---------
**
**	Does not filter unnecessary style changes.
*/
PUBLIC void HText_setStyle ARGS2(
	HText *,	text,
	HTStyle *,	style)
{
    int after, before;

    if (!style)
        return;				/* Safety */
    after = text->style->spaceAfter;
    before = style->spaceBefore;
    if (TRACE)
        fprintf(stderr, "GridText: Change to style %s\n", style->name);

    blank_lines (text, ((after > before) ? after : before));

    text->style = style;
}

/*	Append a character to the text object
**	-------------------------------------
*/
PUBLIC void HText_appendCharacter ARGS2(
	HText *,	text,
	char,		ch)
{
    HTLine * line;
    HTStyle * style;
    int indent;

    /*
     *  Make sure we don't crash on NULLs.
     */
    if (!text)
	return;

#ifdef NOTDEFINED
    /* Make sure nbsp is handled properly. */
    if ((unsigned char)ch == 160)
        ch = HT_NON_BREAK_SPACE;
#endif /* NOTDEFINED */

    /*
     *  Make sure we don't hang on escape sequences.
     */
    if (ch == '\033' && HTCJK == NOCJK)			/* decimal 27 */
	return;
    if ((unsigned char)ch == 155 && HTCJK == NOCJK) {	/* octal 233 */
        if (!HTPassHighCtrlRaw &&
	    strncmp(LYchar_set_names[current_char_set],
		    "IBM PC character set", 20) &&
	    strncmp(LYchar_set_names[current_char_set],
		    "IBM PC codepage 850", 19) &&
	    strncmp(LYchar_set_names[current_char_set],
		    "Macintosh (8 bit)", 17) &&
	    strncmp(LYchar_set_names[current_char_set],
		    "NeXT character set", 18)) {
	    return;
	}
    }

    line = text->last_line;
    style = text->style;

    indent = text->in_line_1 ? (int)style->indent1st : (int)style->leftIndent;
    
    if (HTCJK != NOCJK) {
	switch(text->state) {
	    case S_text:
		if (ch == '\033') {
		    text->state = S_esc;
		    text->kanji_buf = '\0';
		    return;
		}
		break;

		case S_esc:
		if (ch == '$') {
		    text->state = S_dollar;
		    return;
		} else if (ch == '(') {
		    text->state = S_paren;
		    return;
		} else {
		    text->state = S_text;
		}

		case S_dollar:
		if (ch == '@' || ch == 'B' || ch=='A') {
		    text->state = S_nonascii_text;
		    return;
		} else if (ch == '(') {
		    text->state = S_dollar_paren;
		    return;
		} else {
		    text->state = S_text;
		}
		break;

		case S_dollar_paren:
		if (ch == 'C') {
		    text->state = S_nonascii_text;
		    return;
		} else {
		    text->state = S_text;
		}
		break;

		case S_paren:
		if (ch == 'B' || ch == 'J' || ch == 'T')  {
		    text->state = S_text;
		    return;
		} else if (ch == 'I')  {
		    text->state = S_jisx0201_text;
		    return;
		} else {
		    text->state = S_text;
		}
		break;

		case S_nonascii_text:
		if (ch == '\033') {
		    text->state = S_esc;
		    text->kanji_buf = '\0';
		    return;
		} else {
		    ch |= 0200;
		}
		break;

		/*
		 *  JIS X0201 Kana in JIS support. - by ASATAKU
		 */
		case S_jisx0201_text:
		if (ch == '\033') {
		    text->state = S_esc;
		    text->kanji_buf = '\0';
		    return;
		} else {
		    text->kanji_buf = '\x8E';
		    ch |= 0200;
		}
		break;
	}

        if (!text->kanji_buf) {
	    if ((ch & 0200) != 0) {
		/*
		 *  JIS X0201 Kana in SJIS support. - by ASATAKU
		 */
	        if ((text->kcode == SJIS) &&
		    ((unsigned char)ch >= 0xA1) &&
		    ((unsigned char)ch <= 0xDF)) {
		    unsigned char c = (unsigned char)ch;
		    unsigned char kb = (unsigned char)text->kanji_buf;
		    JISx0201TO0208_SJIS(c,
		    			(unsigned char *)&kb,
					(unsigned char *)&c);
		    ch = (char)c;
		    text->kanji_buf = (char)kb;
	        } else {
		    text->kanji_buf = ch;
		    text->permissible_split = line->size;   /* Can split here */
		    return;
	        }
	    }
	} else {
	    goto check_IgnoreExcess;
	}
    } else if (ch == '\033') {
	return;
    }

    if (ch != LY_SOFT_HYPHEN && IsSpecialAttrChar(ch)) {
        if (ch == LY_UNDERLINE_START_CHAR) { 
            line->data[line->size++] = LY_UNDERLINE_START_CHAR;
	    line->data[line->size] = '\0';
	    underline_on = ON;
	    if (!(dump_output_immediately && use_underscore))
		ctrl_chars_on_this_line++;
	    return;
        } else if (ch == LY_UNDERLINE_END_CHAR) {
            line->data[line->size++] = LY_UNDERLINE_END_CHAR;
	    line->data[line->size] = '\0';
	    underline_on = OFF;
	    if (!(dump_output_immediately && use_underscore))
	    	ctrl_chars_on_this_line++;
	    return;
        } else if (ch == LY_BOLD_START_CHAR) {
            line->data[line->size++] = LY_BOLD_START_CHAR;
	    line->data[line->size] = '\0';
            bold_on = ON;
	    ctrl_chars_on_this_line++;
            return;
        } else if (ch == LY_BOLD_END_CHAR) {
            line->data[line->size++] = LY_BOLD_END_CHAR;
	    line->data[line->size] = '\0';
            bold_on = OFF;
	    ctrl_chars_on_this_line++;
            return;
	}
    }


    /*
     *  New Line.
     */
    if (ch == '\n') {
	    new_line(text);
	    text->in_line_1 = YES;	/* First line of new paragraph */
	    return;
    }

    /*
     *  Convert EM_SPACE to a space here so that it doesn't get collapsed.
     */
    if (ch == HT_EM_SPACE)
	ch = ' ';

    /*
     *  I'm going to cheat here in a BIG way.  Since I know that all
     *  \r's will be trapped by HTML_put_character I'm going to use
     *  \r to mean go down a line but don't start a new paragraph.  
     *  i.e. use the second line indenting.
     */
    if (ch == '\r') {
	new_line(text);
	text->in_line_1 = NO;
	return;
    }


    /*
     *  Tabs.
     */
    if (ch == '\t') {
        HTTabStop * Tab;
	int target;	/* Where to tab to */
	int here;

	if (line->size > 0 && line->data[line->size-1] == LY_SOFT_HYPHEN) {
	    /*
	     *  A tab shouldn't follow a soft hyphen, so
	     *  if one does, we'll dump the soft hyphen. - FM
	     */
	    line->data[--line->size] = '\0';
	    ctrl_chars_on_this_line--;
	}
	here = (((int)line->size + (int)line->offset) + indent)
		- ctrl_chars_on_this_line; /* Consider special chars GAB */
        if (style->tabs) {	/* Use tab table */
	    for (Tab = style->tabs;
	    	Tab->position <= here;
		Tab++)
		if (!Tab->position) {
		    new_line(text);
		    return;
		}
	    target = Tab->position;
	} else if (text->in_line_1) {	/* Use 2nd indent */
	    if (here >= (int)style->leftIndent) {
	        new_line(text); /* wrap */
		return;
	    } else {
	        target = (int)style->leftIndent;
	    }
	} else {		/* Default tabs align with left indent mod 8 */
#ifdef DEFAULT_TABS_8
	    target = (((int)line->offset + (int)line->size + 8) & (-8))
	    		+ (int)style->leftIndent;
#else
	    new_line(text);
	    return;
#endif
	}

	if (target > (LYcols-1) - (int)style->rightIndent &&
	    HTOutputFormat != WWW_SOURCE) {
	    new_line(text);
	    return;
	} else {
            text->permissible_split = (int)line->size;	/* Can split here */
	    if (line->size == 0) {
	        line->offset = line->offset + target - here;
	    } else {
	        for (; here<target; here++) {
		    /* Put character into line */
                    line->data[line->size++] = ' ';
		    line->data[line->size] = '\0';
	        }
	    }
	    return;
	}
	/*NOTREACHED*/
    } /* if tab */ 

    
    if (ch == ' ') {
        text->permissible_split = (int)line->size;	/* Can split here */
	/* 
	 *  There are some pages witten in
	 *  different kanji codes. - TA
	 */
	if (HTCJK == JAPANESE)
	    text->kcode = NOKANJI;
    }

    /*
     *  Check if we should ignore characters at the wrap point.
     */    
check_IgnoreExcess:
    if (text->IgnoreExcess &&
        ((indent + (int)line->offset + (int)line->size) + 
	(int)style->rightIndent - ctrl_chars_on_this_line) >= (LYcols-1))
        return;

    /*
     *  Check for end of line.
     */
    if (((indent + (int)line->offset + (int)line->size) + 
	(int)style->rightIndent - ctrl_chars_on_this_line +
	(int)(line->data[line->size] == LY_SOFT_HYPHEN ?
						     1 : 0)) >= (LYcols-1)) {

        if (style->wordWrap && HTOutputFormat != WWW_SOURCE) {
	    split_line(text, text->permissible_split);
	    if (ch == ' ') return;	/* Ignore space causing split */

	}  else if (HTOutputFormat == WWW_SOURCE) {
		 /*
		  *  For source output we dont want to wrap this stuff
		  *  unless absolutely neccessary. - LJM 
		  *  !
		  *  If we don't wrap here we might get a segmentation fault.
		  *  but let's see what happens
		  */
		if ((int)line->size >= (int)(MAX_LINE-1))
		   new_line(text);  /* try not to linewrap */
	} else {
		/*
		 *  For normal stuff like pre let's go ahead and
		 *  wrap so the user can see all of the text.
		 */
		new_line(text);  
	}
    }

    /*
     *  Insert normal characters.
     */
    if (ch == HT_NON_BREAK_SPACE) {
        ch = ' ';
    }

    {
        HTLine * line = text->last_line;	/* May have changed */
        HTFont font = style->font;
	unsigned char hi, lo, tmp[2];

	if (HTCJK != NOCJK && text->kanji_buf) {
	    hi = (unsigned char)text->kanji_buf, lo = (unsigned char)ch;
	    if (HTCJK == JAPANESE && text->kcode == NOKANJI) {
		if (IS_SJIS(hi, lo, text->in_sjis) && IS_EUC(hi, lo)) {
		    text->kcode = NOKANJI;
		} else if (IS_SJIS(hi, lo, text->in_sjis)) {
		    text->kcode = SJIS;
		} else if (IS_EUC(hi, lo)) {
		    text->kcode = EUC;
		}
	    }
	    if (HTCJK == JAPANESE &&
		(kanji_code == EUC) && (text->kcode == SJIS)) {
		SJIS_TO_EUC1(hi, lo, tmp);
		line->data[line->size++] = tmp[0];
		line->data[line->size++] = tmp[1];
	    } else if (HTCJK == JAPANESE &&
		       (kanji_code == EUC) && (text->kcode == EUC)) {
		JISx0201TO0208_EUC(hi, lo, &hi, &lo);
		line->data[line->size++] = hi;
		line->data[line->size++] = lo;
	    } else if (HTCJK == JAPANESE &&
		       (kanji_code == SJIS) && (text->kcode == EUC)) {
		EUC_TO_SJIS1(hi, lo, tmp);
		line->data[line->size++] = tmp[0];
		line->data[line->size++] = tmp[1];
	    } else {
		line->data[line->size++] = hi;
		line->data[line->size++] = lo;
	    }
	    text->kanji_buf = 0;
	} else if (HTCJK != NOCJK) {
	    line->data[line->size++] = (kanji_code != NOKANJI) ?
	    						    ch :
					  (font & HT_CAPITALS) ?
					    	   TOUPPER(ch) : ch;
	} else {
            line->data[line->size++] =	/* Put character into line */
	    	font & HT_CAPITALS ? TOUPPER(ch) : ch;
	}
	line->data[line->size] = '\0';
        if (font & HT_DOUBLE)		/* Do again if doubled */
            HText_appendCharacter(text, HT_NON_BREAK_SPACE);
	    /* NOT a permissible split */ 
    }

    if (ch == LY_SOFT_HYPHEN) {
        ctrl_chars_on_this_line++;
        text->permissible_split = (int)line->size;	/* Can split here */
    }
}

/*	Set LastChar element in the text object.
**	----------------------------------------
*/
PUBLIC void HText_setLastChar ARGS2(
	HText *,	text,
	char,		ch)
{
    if (!text)
        return;

    text->LastChar = ch;
}

/*	Get LastChar element in the text object.
**	----------------------------------------
*/
PUBLIC char HText_getLastChar ARGS1(
	HText *,	text)
{
    if (!text)
        return('\0');

    return((char)text->LastChar);
}

/*	Set IgnoreExcess element in the text object.
**	--------------------------------------------
*/
PUBLIC void HText_setIgnoreExcess ARGS2(
	HText *,	text,
	BOOL,		ignore)
{
    if (!text)
        return;

    text->IgnoreExcess = ignore;
}

/*		Anchor handling
**		---------------
*/

/*	Start an anchor field
*/
PUBLIC void HText_beginAnchor ARGS2(
	HText *,		text,
	HTChildAnchor *,	anc)
{
    char marker[16];

    TextAnchor * a = (TextAnchor *) calloc(1, sizeof(*a));
    
    if (a == NULL)
        outofmem(__FILE__, "HText_beginAnchor");
    a->hightext  = 0;
    a->hightext2 = 0;
    a->start = text->chars + text->last_line->size;

    a->line_pos = text->last_line->size;
    if (text->last_anchor) {
        text->last_anchor->next = a;
    } else {
        text->first_anchor = a;
    }
    a->next = 0;
    a->anchor = anc;
    a->extent = 0;
    a->link_type = HYPERTEXT_ANCHOR;
    text->last_anchor = a;
    
    if (HTAnchor_followMainLink((HTAnchor*)anc)) {
        a->number = ++(text->last_anchor_number);
    } else {
        a->number = 0;
    }

    /* if we are doing link_numbering add the link number */
    if (keypad_mode == LINKS_ARE_NUMBERED && a->number > 0) {
	sprintf(marker,"[%d]", a->number);
        HText_appendText(text, marker);
	a->start += strlen(marker);
    }
}


PUBLIC void HText_endAnchor ARGS1(
	HText *,	text)
{
    TextAnchor * a = text->last_anchor;
    if (a->number) {
        /*
	 *  If it goes somewhere...
	 */
        a->extent += text->chars + text->last_line->size - a->start;
	if (a->extent <= 2) {
	    /*
	     *  Might be a blank anchor from an ALT="". - FM
	     */
    	    int j;
	    a->show_anchor = NO;
	    for (j = (text->last_line->size - a->extent);
	         text->last_line->data[j] != '\0'; j++) {
		if (!IsSpecialAttrChar(text->last_line->data[j])) {
		    a->show_anchor = YES;
		    break;
		}
	    }
	} else {
	    a->show_anchor = YES;
	}
    } else {
        a->show_anchor = NO;
	a->extent = 0;
    }
 
}


PUBLIC void HText_appendText ARGS2(
	HText *,	text,
	CONST char *,	str)
{
    CONST char *p;

    if (str == NULL)
	return;

    for (p = str; *p; p++) {
        HText_appendCharacter(text, *p);
    }
}


PRIVATE void remove_special_attr_chars ARGS1(
	char *,		buf)
{
    register char *cp;

    for (cp=buf; *cp != '\0' ; cp++) {
         /* don't print underline chars */
        if (!IsSpecialAttrChar(*cp)) {
           *buf = *cp, 
           buf++;
	}
    }
    *buf = '\0';
}


/*
**  This function trims blank lines from the end of the document, and
**  then gets the hightext from the text by finding the char position,
**  and brings the anchors in line with the text by adding the text
**  offset to each of the anchors
*/
PUBLIC void HText_endAppend ARGS1(
	HText *,	text)
{
    int cur_line, cur_char;
    TextAnchor *anchor_ptr;
    HTLine *line_ptr;
    unsigned char ch;

    if (!text)
	return;

    new_line(text);
    
    /*
     *  Get the first line
     */
    line_ptr = text->last_line->next;
    cur_char = line_ptr->size;;
    cur_line = 0;

    /*
     *  Remove the blank lines at the end of document.
     */
    while (text->last_line->data[0] == '\0' && text->Lines > 2) {
        HTLine *next_to_the_last_line;

        if (TRACE)
            fprintf(stderr, "GridText: Removing bottom blank line: %s\n",
                            text->last_line->data);
    
        next_to_the_last_line = text->last_line->prev;

        /* line_ptr points to the first line */
        next_to_the_last_line->next = line_ptr;
        line_ptr->prev = next_to_the_last_line;
	FREE(text->last_line);
        text->last_line = next_to_the_last_line;
        text->Lines--;

        if (TRACE)
            fprintf(stderr, "GridText: New bottom line: %s\n",
                            text->last_line->data);

    }

    if (TRACE)
	fprintf(stderr,"Gridtext: Entering HText_endAppend\n");

    for (anchor_ptr = text->first_anchor;
         anchor_ptr; anchor_ptr=anchor_ptr->next) {

re_parse:
	/*
	 *  Find the right line.
	 */
	for (; anchor_ptr->start >= cur_char;
	       line_ptr = line_ptr->next,
	       cur_char += line_ptr->size+1,
	       cur_line++) 
	    ; /* null body */

	if (anchor_ptr->start == cur_char)
	    anchor_ptr->line_pos = line_ptr->size;
	else
	    anchor_ptr->line_pos = anchor_ptr->start-(cur_char-line_ptr->size);

	if (anchor_ptr->line_pos < 0)
	    anchor_ptr->line_pos = 0;

	if (TRACE)
	    fprintf(stderr, "Gridtext: Anchor found on line:%d col:%d\n",
			    cur_line, anchor_ptr->line_pos);

	/* 
	 *  Strip off any spaces or SpecialAttrChars at the beginning,
	 *  if they exist, but only on HYPERTEXT_ANCHORS.
	 */
	if (anchor_ptr->link_type == HYPERTEXT_ANCHOR) {
	    ch = (unsigned char)line_ptr->data[anchor_ptr->line_pos];
            while (isspace(ch) ||
	           IsSpecialAttrChar(ch)) {
		anchor_ptr->line_pos++;
		anchor_ptr->extent--;
		ch = (unsigned char)line_ptr->data[anchor_ptr->line_pos];
	    }
	}

	if (anchor_ptr->extent < 0)
	    anchor_ptr->extent = 0;

	if (TRACE)
	    fprintf(stderr, "anchor text: '%s'   pos: %d\n",
	    		    line_ptr->data, anchor_ptr->line_pos);

	/* 
	 *  If the link begins with a end of line and we have more
	 *  lines, then start the highlighting on the next line.
	 */
	if (anchor_ptr->line_pos >= strlen(line_ptr->data) &&
	    cur_line < text->Lines) {
	    anchor_ptr->start++;

	    if (TRACE)
		fprintf(stderr, "found anchor at end of line\n");
	    goto re_parse;
	}

	if (TRACE)
	    fprintf(stderr, "anchor text: '%s'   pos: %d\n",
	    		    line_ptr->data, anchor_ptr->line_pos);

	/*
	 *  Copy the link name into the data structure.
	 */
	if (line_ptr->data &&
	    anchor_ptr->extent > 0 && anchor_ptr->line_pos >= 0) {

	    StrnAllocCopy(anchor_ptr->hightext,
	    		  &line_ptr->data[anchor_ptr->line_pos],
			  anchor_ptr->extent);
	} else {
	    StrAllocCopy(anchor_ptr->hightext, ""); 
	}

	/*
	 *  If true the anchor extends over two lines,
	 *  copy that into the data structure.
	 */
	if (anchor_ptr->extent > strlen(anchor_ptr->hightext)) {
            HTLine *line_ptr2 = line_ptr->next;
	    /* double check! */
	    if (line_ptr) {
		StrnAllocCopy(anchor_ptr->hightext2, line_ptr2->data, 
			 (anchor_ptr->extent - strlen(anchor_ptr->hightext))-1);
	        anchor_ptr->hightext2offset = line_ptr2->offset;
	 	remove_special_attr_chars(anchor_ptr->hightext2);
	    }
	}   

	remove_special_attr_chars(anchor_ptr->hightext);

        /*
	 *  Subtract any formatting characters from the x position
         *  of the link.
         */
        if (anchor_ptr->line_pos > 0) {
            register int offset = 0, i = 0;
            for (; i < anchor_ptr->line_pos; i++)
                if (IsSpecialAttrChar(line_ptr->data[i]))
                    offset++;
            anchor_ptr->line_pos -= offset;
        }

        anchor_ptr->line_pos += line_ptr->offset;  /* add the offset */
        anchor_ptr->line_num  = cur_line;


	if (TRACE)
	    fprintf(stderr,
	    	    "GridText: adding link on line %d in HText_endAppend\n",
		    cur_line);

 	if (anchor_ptr == text->last_anchor)
	    break;
    }
}


/* 	Dump diagnostics to stderr
*/
PUBLIC void HText_dump ARGS1(
	HText *,	text)
{
    fprintf(stderr, "HText: Dump called\n");
}
	

/*	Return the anchor associated with this node
*/
PUBLIC HTParentAnchor * HText_nodeAnchor ARGS1(
	HText *,	text)
{
    return text->node_anchor;
}

/*				GridText specials
**				=================
*/
/*	Return the anchor with index N
**
**	The index corresponds to the number we print in the anchor.
*/
PUBLIC HTChildAnchor * HText_childNumber ARGS1(
	int,		number)
{
    TextAnchor * a;

    if (!HTMainText)
        return (HTChildAnchor *)0;	/* Fail */

    for (a = HTMainText->first_anchor; a; a = a->next) {
        if (a->number == number)
	    return(a->anchor);
    }
    return (HTChildAnchor *)0;	/* Fail */
}

/*
 *  HTGetLinkInfo returns some link info based on the number.
 */
PUBLIC int HTGetLinkInfo ARGS3(
	int,		number,
	char **,	hightext,
	char **,	lname)
{
    TextAnchor * a;
    HTAnchor *link_dest;

    if (!HTMainText)
        return(NO);

    for (a = HTMainText->first_anchor; a; a = a->next) {
        if (a->number == number) {
	    *hightext= a->hightext;
            link_dest = HTAnchor_followMainLink(
                                               (HTAnchor *)a->anchor);
	    {
		/*
		 *	Memory Leak fixed.
		 *	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
	 	 */
		auto char *cp_freeme = NULL;
		if (traversal)
		    cp_freeme = stub_HTAnchor_address(link_dest);
		else
		    cp_freeme = HTAnchor_address(link_dest);
            	StrAllocCopy(*lname, cp_freeme);
		FREE(cp_freeme);
	    }
	    return(YES);
	}
    }
    return(NO);
}

/*
 *  HText_getNumOfLines returns the number of lines in the
 *  current document.
 */
PUBLIC int HText_getNumOfLines NOARGS
{
     return(HTMainText ? HTMainText->Lines : 0);
}

/*
 *  HText_getTitle returns the title of the
 *  current document.
 */
PUBLIC char * HText_getTitle NOARGS
{
   return(HTMainText ?
   	  (char *) HTAnchor_title(HTMainText->node_anchor) : NULL);
}

/*
 *  HText_getSugFname returns the suggested filename of the current
 *  document (normally derived from a Content-Disposition header with
 *  file; filename=name.suffix). - FM
 */
PUBLIC char * HText_getSugFname NOARGS
{
   return(HTMainText ?
   	  (char *) HTAnchor_SugFname(HTMainText->node_anchor) : NULL);
}

/*
 *  HText_getLastModified returns the Last-Modified header
 *  if available, for the current document. - FM
 */
PUBLIC char * HText_getLastModified NOARGS
{
   return(HTMainText ?
   	  (char *) HTAnchor_last_modified(HTMainText->node_anchor) : NULL);
}

/*
 *  HText_getDate returns the Date header
 *  if available, for the current document. - FM
 */
PUBLIC char * HText_getDate NOARGS
{
   return(HTMainText ?
   	  (char *) HTAnchor_date(HTMainText->node_anchor) : NULL);
}

/*
 *  HText_getServer returns the Server header
 *  if available, for the current document. - FM
 */
PUBLIC char * HText_getServer NOARGS
{
   return(HTMainText ?
   	  (char *)HTAnchor_server(HTMainText->node_anchor) : NULL);
}

/*
 *  HText_pageDisplay displays a screen of text
 *  starting from the line 'line_num'-1
 *  this is the primary call for lynx
 */
PUBLIC void HText_pageDisplay ARGS2(
	int,		line_num,
	char *,		target)
{
    display_page(HTMainText, line_num-1, target);

    is_www_index = HTAnchor_isIndex(HTMainAnchor);
} 

/*
 *  HText_LinksInLines returns the number of links in the
 *  'Lines' number of lines beginning with 'line_num'-1. - FM
 */
PUBLIC int HText_LinksInLines ARGS3(
	HText *,	text,
	int,		line_num,
	int,		Lines)
{
    int total = 0;
    int start = (line_num - 1);
    int end = (start + Lines);
    TextAnchor *Anchor_ptr = NULL;

    if (!text)
        return total;

    for (Anchor_ptr = text->first_anchor;
		Anchor_ptr != NULL && Anchor_ptr->line_num <= end;
			Anchor_ptr = Anchor_ptr->next) {
	if (Anchor_ptr->line_num >= start &&
	    Anchor_ptr->line_num < end &&
	    Anchor_ptr->show_anchor)
	    ++total;
	if (Anchor_ptr == text->last_anchor)
	    break;
    }

    return total;
}

PUBLIC void HText_setStale ARGS1(
	HText *,	text)
{
    text->stale = YES;
}

PUBLIC void HText_refresh ARGS1(
	HText *,	text)
{
    if (text->stale)
        display_page(text, text->top_of_screen, "");
}

PUBLIC int HText_sourceAnchors ARGS1(
	HText *,	text)
{
    return (text ? text->last_anchor_number : -1);
}

PUBLIC BOOL HText_canScrollUp ARGS1(
	HText *,	text)
{
    return (text->top_of_screen != 0);
}

PUBLIC BOOL HText_canScrollDown NOARGS
{
    HText * text = HTMainText;

    return ((text->top_of_screen + display_lines) < text->Lines+1);
}

/*		Scroll actions
*/
PUBLIC void HText_scrollTop ARGS1(
	HText *,	text)
{
    display_page(text, 0, "");
}

PUBLIC void HText_scrollDown ARGS1(
	HText *,	text)
{
    display_page(text, text->top_of_screen + display_lines, "");
}

PUBLIC void HText_scrollUp ARGS1(
	HText *,	text)
{
    display_page(text, text->top_of_screen - display_lines, "");
}

PUBLIC void HText_scrollBottom ARGS1(
	HText *,	text)
{
    display_page(text, text->Lines - display_lines, "");
}


/*		Browsing functions
**		==================
*/

/* Bring to front and highlight it
*/
PRIVATE int line_for_char ARGS2(
	HText *,	text,
	int,		char_num)
{
    int line_number = 0;
    int characters = 0;
    HTLine * line = text->last_line->next;
    for (;;) {
	if (line == text->last_line) return 0;	/* Invalid */
        characters = characters + line->size + 1;
	if (characters > char_num) return line_number;
	line_number ++;
	line = line->next;
    }
}

PUBLIC BOOL HText_select ARGS1(
	HText *,	text)
{
    if (text != HTMainText) {
        HTMainText = text;
	HTMainAnchor = text->node_anchor;
	/*
	 *  Make this text the most current in the loaded texts list. - FM
	 */
	if (loaded_texts && HTList_removeObject(loaded_texts, text))
	    HTList_addObject(loaded_texts, text);
	  /* let lynx do it */
	/* display_page(text, text->top_of_screen, ""); */
    }
    return YES;
}

/*
 *  This function returns TRUE if doc's post_data, address
 *  and isHEAD elements are identical to those of a loaded
 *  (memory cached) text. - FM
 */
PUBLIC BOOL HText_POSTReplyLoaded ARGS1(
	document *,	doc)
{
    HText *text = NULL;
    HTList *cur = loaded_texts;
    char *post_data, *address;
    BOOL is_head;

    /*
     *  Make sure we have the structures. - FM
     */
    if (!cur || !doc)
	return(FALSE);

    /*
     *  Make sure doc is for a POST reply. - FM
     */
    if ((post_data = doc->post_data) == NULL ||
        (address = doc->address) == NULL)
	return(FALSE);
    is_head = doc->isHEAD;

    /*
     *  Loop through the loaded texts looking for a
     *  POST reply match. - FM
     */
    while (NULL != (text = (HText *)HTList_nextObject(cur))) {
	if (text->node_anchor &&
	    text->node_anchor->post_data &&
	    !strcmp(post_data, text->node_anchor->post_data) &&
	    text->node_anchor->address &&
	    !strcmp(address, text->node_anchor->address) &&
	    is_head == text->node_anchor->isHEAD) {
	    return(TRUE);
	}
    }

    return(FALSE);
}

PUBLIC BOOL HTFindPoundSelector ARGS1(
	char *,		selector)
{
    TextAnchor * a;

    for (a=HTMainText->first_anchor; a; a=a->next) {

        if (a->anchor && a->anchor->tag)
            if (!strcmp(a->anchor->tag, selector)) {

                 www_search_result = a->line_num+1;
                 if (TRACE) 
		    fprintf(stderr, 
		"HText: Selecting anchor [%d] at character %d, line %d\n",
                                     a->number, a->start, www_search_result);
		if (!strcmp(selector, LYToolbarName))
		    --www_search_result;

                 return(YES);
            }
    }

    return(NO);

}

PUBLIC BOOL HText_selectAnchor ARGS2(
	HText *,		text,
	HTChildAnchor *,	anchor)
{
    TextAnchor * a;

/* This is done later, hence HText_select is unused in GridText.c
   Should it be the contrary ? @@@
    if (text != HTMainText) {
        HText_select(text);
    }
*/

    for (a=text->first_anchor; a; a=a->next) {
        if (a->anchor == anchor) break;
    }
    if (!a) {
        if (TRACE) fprintf(stderr, "HText: No such anchor in this text!\n");
        return NO;
    }

    if (text != HTMainText) {		/* Comment out by ??? */
        HTMainText = text;		/* Put back in by tbl 921208 */
	HTMainAnchor = text->node_anchor;
    }

    {
	 int l = line_for_char(text, a->start);
	if (TRACE) fprintf(stderr,
	    "HText: Selecting anchor [%d] at character %d, line %d\n",
	    a->number, a->start, l);

	if ( !text->stale &&
	     (l >= text->top_of_screen) &&
	     ( l < text->top_of_screen + display_lines+1))
	         return YES;

	www_search_result = l - (display_lines/3); /* put in global variable */
    }
    
    return YES;
}
 

/*		Editing functions		- NOT IMPLEMENTED
**		=================
**
**	These are called from the application. There are many more functions
**	not included here from the orginal text object.
*/

/*	Style handling:
*/
/*	Apply this style to the selection
*/
PUBLIC void HText_applyStyle ARGS2(
	HText *,	me,
	HTStyle *,	style)
{
    
}


/*	Update all text with changed style.
*/
PUBLIC void HText_updateStyle ARGS2(
	HText *,	me,
	HTStyle *,	style)
{
    
}


/*	Return style of  selection
*/
PUBLIC HTStyle * HText_selectionStyle ARGS2(
	HText *,		me,
	HTStyleSheet *,		sheet)
{
    return 0;
}


/*	Paste in styled text
*/
PUBLIC void HText_replaceSel ARGS3(
	HText *,	me,
	CONST char *,	aString,
	HTStyle *,	aStyle)
{
}


/*	Apply this style to the selection and all similarly formatted text
**	(style recovery only)
*/
PUBLIC void HTextApplyToSimilar ARGS2(
	HText *,	me,
	HTStyle *,	style)
{
    
}

 
/*	Select the first unstyled run.
**	(style recovery only)
*/
PUBLIC void HTextSelectUnstyled ARGS2(
	HText *,		me,
	HTStyleSheet *,		sheet)
{
    
}


/*	Anchor handling:
*/
PUBLIC void HText_unlinkSelection ARGS1(
	HText *,	me)
{
    
}

PUBLIC HTAnchor * HText_referenceSelected ARGS1(
	HText *,	me)
{
     return 0;   
}


PUBLIC int HText_getTopOfScreen NOARGS
{
      HText * text = HTMainText;
      return text->top_of_screen;
}

PUBLIC int HText_getLines ARGS1(
	HText *,	text)
{
      return text->Lines;
}

PUBLIC HTAnchor * HText_linkSelTo ARGS2(
	HText *,	me,
	HTAnchor *,	anchor)
{
    return 0;
}

/* 
 *  Utility for freeing the list of previous isindex and whereis queries. - FM
 */
PUBLIC void HTSearchQueries_free NOARGS
{
    char *query;
    HTList *cur = search_queries;

    if (!cur)
        return;

    while (NULL != (query = (char *)HTList_nextObject(cur))) {
	FREE(query);
    }
    HTList_delete(search_queries);
    search_queries = NULL;
    return;
}

/* 
 *  Utility for listing isindex and whereis queries, making
 *  any repeated queries the most current in the list. - FM
 */
PUBLIC void HTAddSearchQuery ARGS1(
	char *,		query)
{
    char *new;
    char *old;
    HTList *cur;

    if (!(query && *query))
        return;

    if ((new = (char *)calloc(1, (strlen(query) + 1))) == NULL)
    	outofmem(__FILE__, "HTAddSearchQuery");
    strcpy(new, query);

    if (!search_queries) {
        search_queries = HTList_new();
	atexit(HTSearchQueries_free);
	HTList_addObject(search_queries, new);
	return;
    }

    cur = search_queries;
    while (NULL != (old = (char *)HTList_nextObject(cur))) {
	if (!strcmp(old, new)) {
	    HTList_removeObject(search_queries, old);
	    FREE(old);
	    break;
	}
    }
    HTList_addObject(search_queries, new);

    return;
}

PUBLIC int do_www_search ARGS1(
	document *,	doc)
{
    char searchstring[256], temp[256], *cp, *tmpaddress = NULL;
    int ch, recall, i;
    int QueryTotal;
    int QueryNum;
    BOOLEAN PreviousSearch = FALSE;

    /*
     *  Load the default query buffer
     */
    if ((cp=strchr(doc->address, '?')) != NULL) {
        /*
	 *  This is an index from a previous search.
	 *  Use its query as the default.
	 */
	PreviousSearch = TRUE;
	strcpy(searchstring, ++cp);
	for (cp=searchstring; *cp; cp++)
	    if (*cp == '+')
	        *cp = ' ';
	HTUnEscape(searchstring);
	strcpy(temp, searchstring);
	/*
	 *  Make sure it's treated as the most recent query. - FM
	 */
	HTAddSearchQuery(searchstring);
    } else {
        /*
	 *  New search; no default.
	 */
        searchstring[0] = '\0';
	temp[0] = '\0';
    }

    /*
     *  Prompt for a query string.
     */
    if (searchstring[0] == '\0') {
        if (HTMainAnchor->isIndexPrompt)
            _statusline(HTMainAnchor->isIndexPrompt);
	else
            _statusline(ENTER_DATABASE_QUERY);
    } else
        _statusline(EDIT_CURRENT_QUERY);
    QueryTotal = (search_queries ? HTList_count(search_queries) : 0);
    recall = (((PreviousSearch && QueryTotal >= 2) ||
    	       (!PreviousSearch && QueryTotal >= 1)) ? RECALL : NORECALL);
    QueryNum = QueryTotal;
get_query:
    if ((ch=LYgetstr(searchstring, VISIBLE,
    		     sizeof(searchstring), recall)) < 0 ||
        *searchstring == '\0' || ch == UPARROW || ch == DNARROW) {
	if (recall && ch == UPARROW) {
	    if (PreviousSearch) {
	        /*
		 *  Use the second to last query in the list. - FM
		 */
	        QueryNum = 1;
		PreviousSearch = FALSE;
	    } else {
	        /*
		 *  Go back to the previous query in the list. - FM
		 */
	        QueryNum++;
	    }
	    if (QueryNum >= QueryTotal)
	        /*
		 *  Roll around to the last query in the list. - FM
		 */
	        QueryNum = 0;
	    if ((cp=(char *)HTList_objectAt(search_queries,
	    				    QueryNum)) != NULL) {
		strcpy(searchstring, cp);
		if (*temp && !strcmp(temp, searchstring)) {
		    _statusline(EDIT_CURRENT_QUERY);
		} else if ((*temp && QueryTotal == 2) ||
			   (!(*temp) && QueryTotal == 1)) {
		    _statusline(EDIT_THE_PREV_QUERY);
		} else {
		    _statusline(EDIT_A_PREV_QUERY);
		}
		goto get_query;
	    }
	} else if (recall && ch == DNARROW) {
	    if (PreviousSearch) {
	        /*
		 *  Use the first query in the list. - FM
		 */
	        QueryNum = QueryTotal - 1;
		PreviousSearch = FALSE;
	    } else {
	        /*
		 *  Advance to the next query in the list. - FM
		 */
	        QueryNum--;
	    }
	    if (QueryNum < 0)
	        /*
		 *  Roll around to the first query in the list. - FM
		 */
		QueryNum = QueryTotal - 1;
	    if ((cp=(char *)HTList_objectAt(search_queries,
	    				    QueryNum)) != NULL) {
		strcpy(searchstring, cp);
		if (*temp && !strcmp(temp, searchstring)) {
		    _statusline(EDIT_CURRENT_QUERY);
		} else if ((*temp && QueryTotal == 2) ||
			   (!(*temp) && QueryTotal == 1)) {
		    _statusline(EDIT_THE_PREV_QUERY);
		} else {
		    _statusline(EDIT_A_PREV_QUERY);
		}
		goto get_query;
	    }
	}

        /*
	 *  Search cancelled.
	 */
        _statusline(CANCELLED);
        sleep(InfoSecs);
	return(NULLFILE);
    }

    /*
     *  Strip leaders and trailers. - FM
     */
    cp = searchstring;
    while (*cp && isspace((unsigned char)*cp))
        cp++;
    if (!(*cp)) {
        _statusline(CANCELLED);
        sleep(InfoSecs);
        return(NULLFILE);
    }
    if (cp > searchstring) {
        for (i = 0; *cp; i++)
	    searchstring[i] = *cp++;
	searchstring[i] = '\0';
    }
    cp = searchstring + strlen(searchstring) - 1;
    while ((cp > searchstring) && isspace((unsigned char)*cp))
        *cp-- = '\0';

    /*
     *  Don't resubmit the same query unintentionally.
     */
    if (!LYforce_no_cache && 0 == strcmp(temp, searchstring)) {
	_statusline(USE_C_R_TO_RESUB_CUR_QUERY);
	sleep(MessageSecs);
	return(NULLFILE);
    }

    /*
     *  Add searchstring to the query list,
     *  or make it the most current. - FM
     */
    HTAddSearchQuery(searchstring);

    /*
     *  Show the URL with the new query.
     */
    if ((cp=strchr(doc->address, '?')) != NULL)
        *cp = '\0';
    StrAllocCopy(tmpaddress, doc->address);
    StrAllocCat(tmpaddress, "?");
    StrAllocCat(tmpaddress, searchstring);
    user_message(WWW_WAIT_MESSAGE, tmpaddress);
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
    syslog(LOG_INFO|LOG_LOCAL5, "%s", tmpaddress);
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */
    FREE(tmpaddress);
    if (cp)
        *cp = '?';

    /*
     *  OK, now we do the search.
     */
    if (HTSearch(searchstring, HTMainAnchor)) {
	/*
	 *	Memory leak fixed.
	 *	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
	 */
	auto char *cp_freeme = NULL;
	if (traversal)
	    cp_freeme = stub_HTAnchor_address((HTAnchor *)HTMainAnchor);
	else
	    cp_freeme = HTAnchor_address((HTAnchor *)HTMainAnchor);
        StrAllocCopy(doc->address, cp_freeme);
	FREE(cp_freeme);

	if (TRACE)
	    fprintf(stderr,"\ndo_www_search: newfile: %s\n",doc->address);

        /*
	 *  Yah, the search succeeded.
	 */
	return(NORMAL);
    }

    /*
     *  Either the search failed (Yuk), or we got redirection.
     *  If it's redirection, use_this_url_instead is set, and
     *  mainloop() will deal with it such that security features
     *  and restrictions are checked before acting on the URL, or
     *  rejecting it. - FM
     */
    return(NOT_FOUND);
}

/*
 *  Print the contents of the file in HTMainText to
 *  the file descripter fp.
 *  If is_reply is TRUE add ">" to the beginning of each
 *  line to specify the file is a reply to message.
 */
PUBLIC void print_wwwfile_to_fd ARGS2(
	FILE *,		fp,
	int,		is_reply)
{
      register int i;
      HTLine * line;
#ifdef VMS
      extern BOOLEAN HadVMSInterrupt;
#endif /* VMS */

      if (!HTMainText)
          return;

      line = HTMainText->last_line->next;
      for (;; line = line->next) {

	  if (is_reply)
             fputc('>',fp);

            /* add offset */
          for (i = 0; i < (int)line->offset; i++)
             fputc(' ',fp);

            /* add data */
          for (i = 0; line->data[i] != '\0'; i++)
             if (!IsSpecialAttrChar(line->data[i]))
                 fputc(line->data[i],fp);
	     else if (dump_output_immediately && use_underscore) {
		switch (line->data[i]) {
		    case LY_UNDERLINE_START_CHAR:
		    case LY_UNDERLINE_END_CHAR:
			fputc('_', fp);
			break;
		    case LY_BOLD_START_CHAR:
		    case LY_BOLD_END_CHAR:
			break;
		}
	     } 

         /* add the return */
         fputc('\n',fp);

	 if (line == HTMainText->last_line)
	    break;

#ifdef VMS
	if (HadVMSInterrupt)
	    break;
#endif /* VMS */
    }

}

/*
 *  Print the contents of the file in HTMainText to
 *  the file descripter fp.
 *  First output line is "thelink", ie, the URL for this file.
 */
PUBLIC void print_crawl_to_fd ARGS3(
	FILE *,		fp,
	char *,		thelink,
	char *,		thetitle)
{
    register int i;
    HTLine * line;
#ifdef VMS
    extern BOOLEAN HadVMSInterrupt;
#endif /* VMS */

    if (!HTMainText)
        return;

    line = HTMainText->last_line->next;
    fprintf(fp,"THE_URL:%s\n",thelink);
    if (thetitle != NULL)fprintf(fp,"THE_TITLE:%s\n",thetitle);;
      
    for (;; line = line->next) {
        /* add offset */
        for (i = 0; i < (int)line->offset; i++)
            fputc(' ',fp);

        /* add data */
        for (i = 0; line->data[i] != '\0'; i++)
            if (!IsSpecialAttrChar(line->data[i]))
                fputc(line->data[i],fp);

        /* add the return */
        fputc('\n',fp);

	if (line == HTMainText->last_line)
	    break;
    }

    /* add the References list if appropriate */
    if (keypad_mode == LINKS_ARE_NUMBERED && !nolist)
        printlist(fp,FALSE);

#ifdef VMS
    HadVMSInterrupt = FALSE;
#endif /* VMS */
}

PUBLIC void www_user_search ARGS2(
	int,		start_line,
	char *,		target)
{
    register HTLine * line;
    register int count;
    extern BOOLEAN case_sensitive;

    if (!HTMainText) {
        return;
    }

    /*
     *  Advance to the start line.
     */
    line = HTMainText->last_line->next;
    for (count = 1; count <= start_line; line = line->next, count++) {
        if (line == HTMainText->last_line) {
	    line = HTMainText->last_line->next; /* set to first line */
	    count = 1;
	    break;
	}
    }

    for (;;) {
	if (case_sensitive && LYno_attr_char_strstr(line->data, target)) {
	    www_search_result=count;
	    return;
	} else if (!case_sensitive &&
		   LYno_attr_char_case_strstr(line->data, target)) {
	    www_search_result=count;
	    return;
	} else if (line == HTMainText->last_line) {  /* next line */
	    break;
	} else {			/* end */
	    line = line->next;
	    count++;
	}
    }

	/* search from the beginning */
    line = HTMainText->last_line->next; /* set to first line */
    count = 1;

    for (;;) {
	    if (case_sensitive && LYno_attr_char_strstr(line->data, target)) {
	        www_search_result=count;
		return;
	    } else if (!case_sensitive &&
	    	       LYno_attr_char_case_strstr(line->data, target)) {
	        www_search_result=count;
		return;
	    } else if (count > start_line) {  /* next line */
    		_user_message(STRING_NOT_FOUND, target);
    		sleep(MessageSecs);
	        return;			/* end */
	    } else {
	        line = line->next;
		count++;
	    }
    }
}

PUBLIC void user_message ARGS2(
	char *,		message,
	char *,		argument) 
{
    char *temp = NULL;
    char temp_arg[256];

    if (message == NULL) {
        mustshow = FALSE;
	return;
    }

   /* make sure we don't overun any buffers */
    LYstrncpy(temp_arg, ((argument == NULL) ? "" : argument), 255);
    temp_arg[255] = '\0';
    temp = (char *)malloc(strlen(message) + strlen(temp_arg) + 1);
    if (temp == NULL)
        outofmem(__FILE__, "user_message");
    sprintf(temp, message, temp_arg);

    statusline(temp);
   
    FREE(temp);
    return;
}

/*
 *  HText_getOwner returns the owner of the
 *  current document.
 */
PUBLIC char * HText_getOwner NOARGS
{
    return(HTMainText ?
    	   (char *)HTAnchor_owner(HTMainText->node_anchor) : NULL);
}

/*
*   HText_setMainTextOwner sets the owner for the
 *  current document.
 */
PUBLIC void HText_setMainTextOwner ARGS1(
	CONST char *,	owner)
{
    if (!HTMainText)
        return;

    HTAnchor_setOwner(HTMainText->node_anchor, owner);
}

/*
 *  HText_getRevTitle returns the RevTitle element of the
 *  current document, used as the subject for mailto comments
 *  to the owner.
 */
PUBLIC char * HText_getRevTitle NOARGS
{
    return(HTMainText ?
    	   (char *)HTAnchor_RevTitle(HTMainText->node_anchor) : NULL);
}

/*
 *  HText_getContentBase returns the Content-Base header
 *  of the current document.
 */
PUBLIC char * HText_getContentBase NOARGS
{
    return(HTMainText ?
    	   (char *)HTAnchor_content_base(HTMainText->node_anchor) : NULL);
}

/*
 *  HText_getContentLocation returns the Content-Location header
 *  of the current document.
 */
PUBLIC char * HText_getContentLocation NOARGS
{
    return(HTMainText ?
	   (char *)HTAnchor_content_location(HTMainText->node_anchor) : NULL);
}

PUBLIC void HTuncache_current_document NOARGS
{
    /*
     *  Should remove current document from memory.
     */
    if (HTMainText) {
        HTList_removeObject(loaded_texts, HTMainText);
	HText_free(HTMainText);
	HTMainText = NULL;
    }
}

PUBLIC int HTisDocumentSource NOARGS
{
    return(HTMainText->source);
}

PUBLIC char * HTLoadedDocumentURL NOARGS
{
    if (!HTMainText)
	return ("");

    if (HTMainText->node_anchor && HTMainText->node_anchor->address) 
       	return(HTMainText->node_anchor->address);
    else
	return ("");
}

PUBLIC char * HTLoadedDocumentPost_data NOARGS
{
    if (!HTMainText)
	return ("");

    if (HTMainText->node_anchor && HTMainText->node_anchor->post_data) 
       	return(HTMainText->node_anchor->post_data);
    else
	return ("");
}

PUBLIC char * HTLoadedDocumentTitle NOARGS
{
    if (!HTMainText)
	return ("");

    if (HTMainText->node_anchor && HTMainText->node_anchor->title) 
       	return(HTMainText->node_anchor->title);
    else
	return ("");
}

PUBLIC BOOLEAN HTLoadedDocumentIsHEAD NOARGS
{
    if (!HTMainText)
	return (FALSE);

    if (HTMainText->node_anchor && HTMainText->node_anchor->isHEAD) 
       	return(HTMainText->node_anchor->isHEAD);
    else
	return (FALSE);
}

PUBLIC BOOLEAN HTLoadedDocumentIsSafe NOARGS
{
    if (!HTMainText)
	return (FALSE);

    if (HTMainText->node_anchor && HTMainText->node_anchor->safe) 
       	return(HTMainText->node_anchor->safe);
    else
	return (FALSE);
}

PUBLIC char * HTLoadedDocumentCharset NOARGS
{
    if (!HTMainText)
	return (NULL);

    if (HTMainText->node_anchor && HTMainText->node_anchor->charset) 
       	return(HTMainText->node_anchor->charset);
    else
	return (NULL);
}

PUBLIC void HText_setNodeAnchorBookmark ARGS1(
	CONST char *,	bookmark)
{
    if (!HTMainText)
	return;

    if (HTMainText->node_anchor)
	HTAnchor_setBookmark(HTMainText->node_anchor, bookmark);
}

PUBLIC char * HTLoadedDocumentBookmark NOARGS
{
    if (!HTMainText)
	return (NULL);

    if (HTMainText->node_anchor && HTMainText->node_anchor->bookmark) 
       	return(HTMainText->node_anchor->bookmark);
    else
	return (NULL);
}

PUBLIC int HText_LastLineSize ARGS1(
	HText *,	text)
{
    if (!text || !text->last_line || !text->last_line->size)
        return 0;
    return HText_TrueLineSize(text->last_line);
}

PUBLIC int HText_PreviousLineSize ARGS1(
	HText *,	text)
{
    HTLine * line;

    if (!text || !text->last_line)
        return 0;
    if (!(line = text->last_line->prev))
        return 0;
    return HText_TrueLineSize(line);
}

PRIVATE int HText_TrueLineSize ARGS1(
	HTLine *,	line)
{
    int i, true_size = 0;

    if (!line || !line->size)
        return 0;

    for (i = 0; i < line->size; i++) {
    	if (!IsSpecialAttrChar(line->data[i])) {
	    true_size++;
	}
    }
    return true_size;
}

PUBLIC void HText_NegateLineOne ARGS1(
	HText *,	text)
{
    if (text) {
        text->in_line_1 = NO;
    }
    return;
}

/*
 *  This function is for removing the first of two
 *  successive blank lines.  It should be called after
 *  checking the situation with HText_LastLineSize()
 *  and HText_PreviousLineSize().  Any characters in
 *  the removed line (i.e., control characters, or it
 *  wouldn't have tested blank) should have been
 *  reiterated by split_line() in the retained blank
 *  line. - FM
 */
PUBLIC void HText_RemovePreviousLine ARGS1(
	HText *,	text)
{
    HTLine *line, *previous;
    char *data;

    if (!(text && text->Lines > 1))
        return;

    line = text->last_line->prev;
    data = line->data;
    previous = line->prev;
    previous->next = text->last_line;
    text->last_line->prev = previous;
    text->chars -= ((data && *data == '\0') ?
    					  1 : strlen(line->data) + 1);
    text->Lines--;
    FREE(line);
}

/*
 *  NOTE: This function presently is correct only if the
 *	  alignment is HT_LEFT.  The offset is still zero,
 *	  because that's not determined for HT_CENTER or
 *	  HT_RIGHT until subsequent characters are received
 *	  and split_line() is called. - FM
 */
PUBLIC int HText_getCurrentColumn ARGS1(
	HText *,	text)
{
    int column = 0;

    if (text) {
        column = (text->in_line_1 ?
		  (int)text->style->indent1st : (int)text->style->leftIndent)
		  + HText_LastLineSize(text) + (int)text->last_line->offset;
    }
    return column;
}

PUBLIC int HText_getMaximumColumn ARGS1(
	HText *,	text)
{
    int column = (LYcols-2);
    if (text) {
        column = ((int)text->style->rightIndent ? (LYcols-2) :
		  ((LYcols-1) - (int)text->style->rightIndent));
    }
    return column;
}

/*
 *  NOTE: This function uses HText_getCurrentColumn() which
 *	  presently is correct only if the alignment is
 *	  HT_LEFT. - FM
 */
PUBLIC void HText_setTabID ARGS2(
	HText *,	text,
	CONST char *,	name)
{
    HTTabID * Tab = NULL;
    HTList * cur = text->tabs;
    HTList * last = NULL;

    if (!text || !name || !*name)
    	return;

    if (!cur) {
        cur = text->tabs = HTList_new();
    } else {
        while (NULL != (Tab = (HTTabID *)HTList_nextObject(cur))) {
	    if (Tab->name && !strcmp(Tab->name, name))
	        return; /* Already set.  Keep the first value. */
	    last = cur;
	}
	cur = last;
    }
    if (!Tab) { /* New name.  Create a new node */
	Tab = (HTTabID *)calloc(1, sizeof(HTTabID));
	if (Tab == NULL)
	    outofmem(__FILE__, "HText_setTabID");
	HTList_addObject(cur, Tab);
	StrAllocCopy(Tab->name, name);
    }
    Tab->column = HText_getCurrentColumn(text);
    return;
}

PUBLIC int HText_getTabIDColumn ARGS2(
	HText *,	text,
	CONST char *,	name)
{
    int column = 0;
    HTTabID * Tab;
    HTList * cur = text->tabs;

    if (text && name && *name && cur) {
        while (NULL != (Tab = (HTTabID *)HTList_nextObject(cur))) {
	    if (Tab->name && !strcmp(Tab->name, name))
	        break;
	}
	if (Tab)
	    column = Tab->column;
    }
    return column;
}


/*
 *  Form methods
 *    These routines are used to build forms consisting
 *    of input fields 
 */
PRIVATE int HTFormMethod;
PRIVATE char * HTFormAction = NULL;
PRIVATE char * HTFormEnctype = NULL;
PRIVATE char * HTFormTitle = NULL;
PRIVATE BOOLEAN HTFormDisabled = FALSE;

PUBLIC void HText_beginForm ARGS4(
	char *,		action,
	char *,		method,
	char *,		enctype,
	char *,		title)
{
    HTFormMethod = URL_GET_METHOD;
    HTFormNumber++;
    HTFormFields = 0;
    HTFormDisabled = FALSE;

    /*
     *  Check the ACTION. - FM
     */
    if (action != NULL) {
	if (!strncmp(action, "mailto:", 7)) {
	    HTFormMethod = URL_MAIL_METHOD;
	}
        StrAllocCopy(HTFormAction, action);
    }
    else
	StrAllocCopy(HTFormAction, HTLoadedDocumentURL());
    
    /*
     *  Check the METHOD. - FM
     */
    if (method != NULL && HTFormMethod != URL_MAIL_METHOD)
	if (!strcasecomp(method,"post") || !strcasecomp(method,"pget"))
	    HTFormMethod = URL_POST_METHOD;

    /*
     *  Check the ENCTYPE. - FM
     */
    if ((enctype != NULL) && *enctype) {
        StrAllocCopy(HTFormEnctype, enctype);
	if (HTFormMethod != URL_MAIL_METHOD &&
	    !strncasecomp(enctype, "multipart/form-data", 19)) 
	    HTFormMethod = URL_POST_METHOD;
    } else {
        FREE(HTFormEnctype);
    }

    /*
     *  Check the TITLE. - FM
     */
    if ((title != NULL) && *title)
        StrAllocCopy(HTFormTitle, title);
    else
        FREE(HTFormTitle);

    if (TRACE)
	fprintf(stderr,
		"BeginForm: action:%s Method:%d%s%s%s%s\n",
		HTFormAction, HTFormMethod,
		(HTFormTitle ? " Title:" : ""),
		(HTFormTitle ? HTFormTitle : ""),
		(HTFormEnctype ? " Enctype:" : ""),
		(HTFormEnctype ? HTFormEnctype : ""));
}

PUBLIC void HText_endForm ARGS1(
	HText *,	text)
{
    if (HTFormFields == 1 && text && text->first_anchor) {
        /*
	 *  Support submission of a single text input field in
	 *  the form via <return> instead of a submit botton. - FM
	 */
        TextAnchor * a = text->first_anchor;
	/*
	 *  Go through list of anchors and get our input field. - FM
	 */
	while (a) {
	    if (a->link_type == INPUT_ANCHOR &&
	        a->input_field->number == HTFormNumber &&
		a->input_field->type == F_TEXT_TYPE) {
		/*
		 *  Got it.  Make it submitting. - FM
		 */
		a->input_field->submit_action = NULL;
		StrAllocCopy(a->input_field->submit_action, HTFormAction);
		if (HTFormEnctype != NULL)
		    StrAllocCopy(a->input_field->submit_enctype,
		    		 HTFormEnctype);
		if (HTFormTitle != NULL)
		    StrAllocCopy(a->input_field->submit_title, HTFormTitle);
		a->input_field->submit_method = HTFormMethod;
		a->input_field->type = F_TEXT_SUBMIT_TYPE;
		if (HTFormDisabled)
		    a->input_field->disabled = TRUE;
		break;
	    }
	    if (a == text->last_anchor)
	        break;
	    a = a->next;
	}
    }
    FREE(HTCurSelectGroup);
    FREE(HTCurSelectGroupSize);
    FREE(HTCurSelectedOptionValue);
    FREE(HTFormAction);
    FREE(HTFormEnctype);
    FREE(HTFormTitle);
    HTFormFields = 0;
    HTFormDisabled = FALSE;
}

PUBLIC void HText_beginSelect ARGS3(
	char *,		name,
	BOOLEAN,	multiple,
	char *,		size)
{
   /*
    *  Save the group name.
    */
   StrAllocCopy(HTCurSelectGroup, name);

   /*
    *  If multiple then all options are actually checkboxes.
    */
   if (multiple)
      HTCurSelectGroupType = F_CHECKBOX_TYPE;
   /*
    *  If not multiple then all options are radio buttons.
    */
   else
      HTCurSelectGroupType = F_RADIO_TYPE;

    /*
     *  Length of an option list.
     */
    StrAllocCopy(HTCurSelectGroupSize, size);

   if (TRACE)
       fprintf(stderr,"HText_beginSelect: name=%s type=%d size=%s\n",
	       ((HTCurSelectGroup == NULL) ? 
	       			  "<NULL>" : HTCurSelectGroup),
		HTCurSelectGroupType,
	       ((HTCurSelectGroupSize == NULL) ? 
	       			      "<NULL>" : HTCurSelectGroupSize));
} 

/*
**  We couln't set the value field for the previous option
**  tag so we have to do it now.  Assume that the last anchor
**  was the previous options tag.
*/
PUBLIC char * HText_setLastOptionValue ARGS5(
	HText *,	text,
	char *,		value,
	char*,		submit_value,
	int,		order,
	BOOLEAN,	checked)
{
    char *cp;
    unsigned char *tmp = NULL;
    int number = 0, i, j;

    if (!(text && text->last_anchor &&
    	  text->last_anchor->link_type == INPUT_ANCHOR))
	return NULL;

    if (TRACE)
	fprintf(stderr,
		"Entering HText_setLastOptionValue: value:%s, checked:%s\n",
		value, (checked ? "on" : "off"));

    /*
     *  Strip end spaces, newline is also whitespace.
     */
    if (*value) {
        cp = &value[strlen(value)-1];
        while ((cp >= value) && (isspace((unsigned char)*cp) ||
       				 IsSpecialAttrChar((unsigned char)*cp)))
            cp--;
        *(cp+1) = '\0';
    }

    /*
     *  Find first non space
     */
    cp = value;
    while (isspace((unsigned char)*cp) ||
   	   IsSpecialAttrChar((unsigned char)*cp))
        cp++;

    if (HTCurSelectGroupType == F_CHECKBOX_TYPE) {
        StrAllocCopy(text->last_anchor->input_field->value, cp);
        /*
	 *  Put the text on the screen as well.
	 */
        HText_appendText(text, cp);

    } else if (LYSelectPopups == FALSE) {
        StrAllocCopy(text->last_anchor->input_field->value,
		     (submit_value ? submit_value : cp));
        /*
	 *  Put the text on the screen as well.
	 */
        HText_appendText(text, cp);

    } else {
	/*
	 *  Create a linked list of option values.
	 */
	OptionType * op_ptr = text->last_anchor->input_field->select_list;
	OptionType * new_ptr = NULL;
	BOOLEAN first_option = FALSE;

	/*
	 *  Deal with newlines or tabs.
	 */
	convert_to_spaces(value, FALSE);

	if (!op_ptr) {
	    /*
	     *  No option items yet.
	     */
	    new_ptr = text->last_anchor->input_field->select_list = 
				(OptionType *) calloc(1, sizeof(OptionType));
	    if (new_ptr == NULL)
	        outofmem(__FILE__, "HText_setLastOptionValue");

	    first_option = TRUE;
	} else {
	    while (op_ptr->next) {
		number++;
		op_ptr=op_ptr->next;
	    }
	    number++;  /* add one more */

	    op_ptr->next = new_ptr =
	    			(OptionType *) calloc(1, sizeof(OptionType));
	    if (new_ptr == NULL)
	        outofmem(__FILE__, "HText_setLastOptionValue");
	}

	new_ptr->name = NULL;
	new_ptr->cp_submit_value = NULL;
	new_ptr->next = NULL;
	for (i = 0, j = 0; cp[i]; i++) {
	    if (cp[i] == HT_NON_BREAK_SPACE ||
	        cp[i] == HT_EM_SPACE) {
		cp[j++] = ' ';
	    } else if (cp[i] != LY_SOFT_HYPHEN &&
	    	       !IsSpecialAttrChar((unsigned char)cp[i])) {
		cp[j++] = cp[i];
	    }
	}
	cp[j] = '\0';
	if (HTCJK != NOCJK) {
	    if (cp &&
	        (tmp = (unsigned char *)calloc(1, strlen(cp)+1))) {
		if (kanji_code == EUC) {
		    TO_EUC((unsigned char *)cp, tmp);
		} else if (kanji_code == SJIS) {
		    TO_SJIS((unsigned char *)cp, tmp);
		} else {
		    for (i = 0, j = 0; cp[i]; i++) {
		        if (cp[i] != '\033') {
			    tmp[j++] = cp[i];
			}
		    }
		}
		StrAllocCopy(new_ptr->name, (CONST char *)tmp);
		FREE(tmp);
	    }
	} else {
	    StrAllocCopy(new_ptr->name, cp);
	}
	StrAllocCopy(new_ptr->cp_submit_value,
	    	     (submit_value ? submit_value : new_ptr->name));
	
	if (first_option) {
	    StrAllocCopy(HTCurSelectedOptionValue, new_ptr->name);
	    text->last_anchor->input_field->num_value = 0;
	    text->last_anchor->input_field->value = 
		text->last_anchor->input_field->select_list->name;
	    text->last_anchor->input_field->orig_value = 
		text->last_anchor->input_field->select_list->name;
	    text->last_anchor->input_field->cp_submit_value = 
		text->last_anchor->input_field->select_list->cp_submit_value;
	    text->last_anchor->input_field->orig_submit_value = 
		text->last_anchor->input_field->select_list->cp_submit_value;
	} else {
	    int newlen = strlen(new_ptr->name);
	    int curlen = strlen(HTCurSelectedOptionValue);
		/*
		 *  Make the selected Option Value as long as
		 *  the longest option.
		 */
	    if (newlen > curlen)
		StrAllocCat(HTCurSelectedOptionValue,
			    UNDERSCORES(newlen-curlen));
	}

	if (checked) {
	    int curlen = strlen(new_ptr->name);
	    int newlen = strlen(HTCurSelectedOptionValue);
	    /*
	     *  Set the default option as this one.
	     */
	    text->last_anchor->input_field->num_value = number;
	    text->last_anchor->input_field->value = new_ptr->name;
	    text->last_anchor->input_field->orig_value = new_ptr->name;
	    text->last_anchor->input_field->cp_submit_value =
	    			   new_ptr->cp_submit_value;
	    text->last_anchor->input_field->orig_submit_value =
	    			   new_ptr->cp_submit_value;
	    StrAllocCopy(HTCurSelectedOptionValue, new_ptr->name);
	    if (newlen > curlen)
		StrAllocCat(HTCurSelectedOptionValue,
			    UNDERSCORES(newlen-curlen));
	}
	     
	/*
	 *  Return the selected Option value to be sent to the screen.
	 */
	if (order == LAST_ORDER) {
	    /*
	     *  Change the value.
	     */
	    text->last_anchor->input_field->size =
	    			strlen(HTCurSelectedOptionValue); 
	    return(HTCurSelectedOptionValue);
	} else 
	   return(NULL);
    }

    if (TRACE)
	fprintf(stderr,"HText_setLastOptionValue: value=%s\n", value);

    return(NULL);
}

/*
 *  Assign a form input anchor
 *  returns the number of charactors to leave blank
 *  so that the input field can fit
 */
PUBLIC int HText_beginInput ARGS2(
	HText *,		text,
	InputFieldData *,	I)
{
	
    TextAnchor * a = (TextAnchor *) calloc(1, sizeof(*a));
    FormInfo * f = (FormInfo *) calloc(1, sizeof(*f)); 
    char *cp_option = NULL;
    char *IValue = NULL;
    unsigned char *tmp = NULL;
    int i, j;

    if (TRACE)
	fprintf(stderr,"Entering HText_beginInput\n");

    if (a == NULL || f == NULL)
        outofmem(__FILE__, "HText_beginInput");

    a->start = text->chars + text->last_line->size;
    a->line_pos = text->last_line->size;


    /*
     *  If this is a radio button, or an OPTION we're converting
     *  to a radio button, and it's the first with this name, make
     *  sure it's checked by default.  Otherwise, if it's checked,
     *  uncheck the default or any preceding radio button with this
     *  name that was checked. - FM
     */
    if (I->type != NULL && !strcmp(I->type,"OPTION") &&
 	HTCurSelectGroupType == F_RADIO_TYPE && LYSelectPopups == FALSE) {
	I->type = "RADIO";
	I->name = HTCurSelectGroup;
    }
    if (I->name && I->type && !strcasecomp(I->type, "radio")) {
        if (!text->last_anchor) {
	    I->checked = TRUE;
	} else {
	    TextAnchor * b = text->first_anchor;
	    int i = 0;
	    while (b) {
	        if (b->link_type == INPUT_ANCHOR &&
		    b->input_field->type == F_RADIO_TYPE &&
                    b->input_field->number == HTFormNumber) {
		    if (!strcmp(b->input_field->name, I->name)) {
			if (I->checked && b->input_field->num_value) {
			    b->input_field->num_value = 0;
			    StrAllocCopy(b->input_field->orig_value, "0");
			    break;
			}
			i++;
		    }
		}
		if (b == text->last_anchor) {
		    if (i == 0)
		       I->checked = TRUE;
		    break;
		}
		b = b->next;
	    }
	}
    }

    if (text->last_anchor) {
        text->last_anchor->next = a;
    } else {
        text->first_anchor = a;
    }
    a->next = 0;
    a->anchor = NULL;
    a->link_type = INPUT_ANCHOR;
    a->show_anchor = YES;

    a->hightext = NULL;
    a->extent = 2;

    a->input_field = f;

    f->select_list = 0;
    f->number = HTFormNumber;
    f->disabled = (HTFormDisabled ? TRUE : I->disabled);
    f->no_cache = NO;

    HTFormFields++;


    /*
     *  Set the no_cache flag if the METHOD is POST. - FM
     */
    if (HTFormMethod == URL_POST_METHOD)
        f->no_cache = TRUE;

    /*
     *  Set up VALUE.
     */
    if (I->value)
        StrAllocCopy(IValue, I->value);
    if (IValue && HTCJK != NOCJK) {
	if ((tmp = (unsigned char *)calloc(1, (strlen(IValue) + 1)))) {
	    if (kanji_code == EUC) {
		TO_EUC((unsigned char *)IValue, tmp);
	    } else if (kanji_code == SJIS) {
		TO_SJIS((unsigned char *)IValue, tmp);
	    } else {
		for (i = 0, j = 0; IValue[i]; i++) {
		    if (IValue[i] != '\033') {
		        tmp[j++] = IValue[i];
		    }
		}
	    }
	    StrAllocCopy(IValue, (CONST char *)tmp);
	    FREE(tmp);
	}
    }

    /*
     *  Special case of OPTION.
     *  Is handled above if radio type and LYSelectPopups is FALSE.
     */
    /* set the values and let the parsing below do the work */
    if (I->type != NULL && !strcmp(I->type,"OPTION")) {
	cp_option = I->type;
 	if (HTCurSelectGroupType == F_RADIO_TYPE)
	    I->type = "OPTION_LIST";
	else
	    I->type = "CHECKBOX";
	I->name = HTCurSelectGroup;

	/*
	 *  The option's size parameter actually gives the length and not
	 *    the width of the list.  Perform the conversion here
	 *    and get rid of the allocated HTCurSelect....
	 *  0 is ok as it means any length (arbitrary decision).
	 */
	if (HTCurSelectGroupSize != NULL) {
	    f->size_l = atoi(HTCurSelectGroupSize);
	    FREE(HTCurSelectGroupSize);
	}
    }

    /*
     *  Set SIZE.
     */
    if (I->size != NULL) {
	f->size = atoi(I->size);
	/*
	 *  Leave at zero for option lists.
	 */
	if (f->size == 0 && cp_option == NULL) {
	   f->size = 20;  /* default */
	}
    } else {
	f->size = 20;  /* default */
    }

    /*
     *  Set MAXLENGTH.
     */
    if (I->maxlength != NULL) {
	f->maxlength = atoi(I->maxlength);
    } else {
	f->maxlength = 0;  /* 0 means infinite */
    }

    /*
     *  Set CHECKED
     *  (num_value is only relevent to check and radio types).
     */
    if (I->checked == TRUE)
 	f->num_value = 1; 
    else
 	f->num_value = 0;

    /*
     *  Set TYPE.
     */
    if (I->type != NULL) {
	if (!strcasecomp(I->type,"password")) {
	    f->type = F_PASSWORD_TYPE;
	} else if (!strcasecomp(I->type,"checkbox")) {
	    f->type = F_CHECKBOX_TYPE;
	} else if (!strcasecomp(I->type,"radio")) {
	    f->type = F_RADIO_TYPE;
	} else if (!strcasecomp(I->type,"submit")) {
	    f->type = F_SUBMIT_TYPE;
	} else if (!strcasecomp(I->type,"image")) {
	    f->type = F_IMAGE_SUBMIT_TYPE;
	} else if (!strcasecomp(I->type,"reset")) {
	    f->type = F_RESET_TYPE;
	} else if (!strcasecomp(I->type,"OPTION_LIST")) {
	    f->type = F_OPTION_LIST_TYPE;
	} else if (!strcasecomp(I->type,"hidden")) {
	    f->type = F_HIDDEN_TYPE;
	    HTFormFields--;
	    f->size = 0;
	} else if (!strcasecomp(I->type,"textarea")) {
	    f->type = F_TEXTAREA_TYPE;
	} else if (!strcasecomp(I->type,"range")) {
	    f->type = F_RANGE_TYPE;
	} else if (!strcasecomp(I->type,"file")) {
	    f->type = F_FILE_TYPE;
	} else {
	    /*
	     *  Note that TYPE="scribble" defaults to TYPE="text". - FM
	     */
	    f->type = F_TEXT_TYPE; /* default */
	}
    } else {
	f->type = F_TEXT_TYPE;
    }

    /*
     *  Set NAME.
     */
    if (I->name != NULL) {
        StrAllocCopy(f->name,I->name);
    } else {
	if (f->type == F_RESET_TYPE ||
	    f->type == F_SUBMIT_TYPE ||
	    f->type == F_IMAGE_SUBMIT_TYPE) {
	    /*
	     *  Set name to empty string.
	     */
	    StrAllocCopy(f->name, "");
	} else {
	    /*
	     *  Error!  NAME must be present.
	     */
	    if (TRACE)
		fprintf(stderr,
		  "GridText: No name present in input field; not displaying\n");
	    FREE(a);
	    FREE(f);
	    FREE(IValue);
	    return(0);
	}
    }

    /* 
     *  Set VALUE, if it exists.  Otherwise, if it's not
     *  an option list make it a zero-length string. - FM
     */
    if (IValue != NULL) {
	/*
	 *  OPTION VALUE is not actually the value to be seen but is to
	 *    be sent....
	 */
	if (f->type == F_OPTION_LIST_TYPE ||
	    f->type == F_CHECKBOX_TYPE) {
	    /*
	     *  Fill both with the value.  The f->value may be
	     *  overwritten in HText_setLastOptionValue....
	     */
	    StrAllocCopy(f->value, IValue);
	    StrAllocCopy(f->cp_submit_value, IValue);
	} else {
	    StrAllocCopy(f->value, IValue);
	}
    } else if (f->type != F_OPTION_LIST_TYPE) {
	StrAllocCopy(f->value, "");
    }

    /*
     *  Run checks and fill in neccessary values.
     */
    if (f->type == F_RESET_TYPE) {
	if (f->value && *f->value != '\0') {
	    f->size = strlen(f->value);
	} else {
	    StrAllocCopy(f->value, "Reset");
	    f->size = 5;
	}
    } else if (f->type == F_IMAGE_SUBMIT_TYPE ||
    	       f->type == F_SUBMIT_TYPE) {
        if (f->value && *f->value != '\0') {
	    f->size = strlen(f->value);
	} else if (f->type == F_IMAGE_SUBMIT_TYPE) {
	    StrAllocCopy(f->value, "[IMAGE]-Submit");
	    f->size = 14;
	} else {
	    StrAllocCopy(f->value, "[IMAGE]-Submit");
	    f->size = 14;
	    StrAllocCopy(f->value, "Submit");
	    f->size = 6;
	}
	f->submit_action = NULL;
	StrAllocCopy(f->submit_action, HTFormAction);
	if (HTFormEnctype != NULL)
	    StrAllocCopy(f->submit_enctype, HTFormEnctype);
	if (HTFormTitle != NULL)
	    StrAllocCopy(f->submit_title, HTFormTitle);
	f->submit_method = HTFormMethod;

    } else if (f->type == F_RADIO_TYPE || f->type == F_CHECKBOX_TYPE) {
	f->size=3;
	if (IValue == NULL)
	   StrAllocCopy(f->value, (f->type == F_CHECKBOX_TYPE ? "on" : ""));

    }
    FREE(IValue); 

    /*
     *  Set original values.
     */
    if (f->type == F_RADIO_TYPE || f->type == F_CHECKBOX_TYPE ) {
	if (f->num_value)
            StrAllocCopy(f->orig_value, "1");
	else
            StrAllocCopy(f->orig_value, "0");
    } else if (f->type == F_OPTION_LIST_TYPE) {
	f->orig_value = NULL;
    } else {
        StrAllocCopy(f->orig_value, f->value);
    }

    /*
     *  Restrict SIZE to maximum allowable size.
     */
    if (f->size > LYcols-10)
	f->size = LYcols-10;  /* maximum */

    /*
     *  Add this anchor to the anchor list
     */
    text->last_anchor = a;

    if (TRACE)
	fprintf(stderr,"Input link: name=%s\nvalue=%s\nsize=%d\n",
		 	f->name,
			((f->value != NULL) ? f->value : ""),
			f->size);
	
    /*
     *  Return the SIZE of the input field.
     */
    return(f->size);
}


PUBLIC void HText_SubmitForm ARGS4(
	FormInfo *,	submit_item,
	document *,	doc,
	char *,		link_name,
	char *,		link_value)
{
    TextAnchor *anchor_ptr;
    int form_number = submit_item->number;
    FormInfo *form_ptr;
    int len, i;
    char *query = NULL;
    char *escaped1 = NULL, *escaped2 = NULL;
    int first_one = 1;
    char *last_textarea_name = NULL;
    char *previous_blanks = NULL;
    BOOLEAN PlainText = FALSE;
    BOOLEAN SemiColon = FALSE;
    char *Boundary = NULL;
    char *MultipartContentType = NULL;

    if (!HTMainText)
        return;

    if (submit_item->submit_action) {
        /*
	 *  If we're mailing, make sure it's a mailto ACTION. - FM
	 */
        if ((submit_item->submit_method == URL_MAIL_METHOD) &&
	    strncmp(submit_item->submit_action, "mailto:", 7)) {
	    HTAlert(BAD_FORM_MAILTO);
	    return;
	}

        /*
	 *  Set length plus breathing room.
	 */
        len = strlen(submit_item->submit_action) + 2048;
    } else {
	return;
    }

    /*
     *  Check the ENCTYPE and set up the appropriate variables. - FM
     */
    if (submit_item->submit_enctype &&
	!strncasecomp(submit_item->submit_enctype, "text/plain", 10)) {
	/*
	 *  Do not hex escape, and use physical newlines
	 *  to separate name=value pairs. - FM
	 */
	PlainText = TRUE;
    } else if (submit_item->submit_enctype &&
	       !strncasecomp(submit_item->submit_enctype,
			     "application/sgml-form-urlencoded", 32)) {
	/*
	 *  Use semicolons instead of ampersands as the
	 *  separators for name=value pairs. - FM
	 */
	SemiColon = TRUE;
    } else if (submit_item->submit_enctype &&
	       !strncasecomp(submit_item->submit_enctype,
			     "multipart/form-data", 19)) {
	/*
	 *  Use the multipart MIME format.  We should generate
	 *  a boundary string which we are sure doesn't occur
	 *  in the content, but for now we'll just assume that
	 *  this string doesn't. - FM
	 */
	Boundary = "xnyLAaB03X";
    }

    /*
     *  Go through list of anchors and get size first.
     */
    anchor_ptr = HTMainText->first_anchor;
    while (anchor_ptr) {
        if (anchor_ptr->link_type == INPUT_ANCHOR) {
   	    if (anchor_ptr->input_field->number == form_number) {

	        form_ptr = anchor_ptr->input_field;
	
	        len += (strlen(form_ptr->name) + (Boundary ? 100 : 10));
		/*
		 *  Calculate by the option submit value if present.
		 */
		if (form_ptr->cp_submit_value != NULL) {
		    len += (strlen(form_ptr->cp_submit_value) + 10);
		} else {
	            len += (strlen(form_ptr->value) + 10);
		}
	        len += 32; /* plus and ampersand + safty net */

	    } else if (anchor_ptr->input_field->number > form_number) {
	        break;
	    }
	}

	if (anchor_ptr == HTMainText->last_anchor)
	    break;

	anchor_ptr = anchor_ptr->next;
    }

    /*
     *  Get query ready.
     */
    query = (char *)calloc(1, len);
    if (query == NULL)
        outofmem(__FILE__, "HText_SubmitForm");

    if (submit_item->submit_method == URL_GET_METHOD && Boundary == NULL) {
       	strcpy (query, submit_item->submit_action);
       	/*
	 *  Method is GET.  Clip out any anchor in the current URL.
	 */
       	strtok (query, "#");
       	/*
	 *  Clip out any old query in the current URL.
	 */
       	strtok (query, "?");
	/*
	 *  Add the lead question mark for the new URL.
	 */  
	strcat(query,"?");
    } else {
        query[0] = '\0';
	/*
	 *  We are submitting POST content to a server,
	 *  so load the post_content_type element. - FM
	 */
	if (SemiColon == TRUE) {
	    StrAllocCopy(doc->post_content_type,
			 "application/sgml-form-urlencoded");
	} else if (PlainText == TRUE) {
	    StrAllocCopy(doc->post_content_type,
			 "text/plain");
	} else if (Boundary != NULL) {
	    StrAllocCopy(doc->post_content_type,
			 "multipart/form-data; boundary=");
	    StrAllocCat(doc->post_content_type, Boundary);
	} else {
	    StrAllocCopy(doc->post_content_type,
			 "application/x-www-form-urlencoded");
	}

	/*
	 *  Append the exended charset info if known, and it is not
	 *  ISO-8859-1 or US-ASCII.  We'll assume the user has the
	 *  matching character set selected, or a download offer would
	 *  have been forced and we would not be processing the form
	 *  here.  We don't yet want to do this unless the server
	 *  indicated the charset in the original transmission, because
	 *  otherwise it might be an old server and CGI script which
	 *  will not parse out the extended charset info, and reject
	 *  the POST Content-Type as invalid.  If the ENCTYPE is
	 *  multipart/form-data and the charset is known, set up a
	 *  Content-Type string for the text fields and append the
	 *  charset even if it is ISO-8859-1 or US-ASCII, but don't
	 *  append it to the post_content_type header.  Note that we do
	 *  not yet have a way to vary the charset among multipart form
	 *  fields, so this code assumes it is the same for all of the
	 *  text fields. - FM
	 */
	if (HTMainText->node_anchor->charset != NULL &&
	    *HTMainText->node_anchor->charset != '\0') {
	    if (Boundary == NULL &&
	        strcasecomp(HTMainText->node_anchor->charset, "iso-8859-1") &&
		strcasecomp(HTMainText->node_anchor->charset, "us-ascii")) {
		StrAllocCat(doc->post_content_type, "; charset=");
		StrAllocCat(doc->post_content_type,
			    HTMainText->node_anchor->charset);
	    } else if (Boundary != NULL) {
	        MultipartContentType = (char *)calloc(1,
			     (40 + strlen(HTMainText->node_anchor->charset)));
		if (query == NULL)
		    outofmem(__FILE__, "HText_SubmitForm");
		sprintf(MultipartContentType,
			"\r\nContent-Type: text/plain; charset=%s",
			HTMainText->node_anchor->charset);
	    }
	}
    }

    /*
     *  Reset anchor->ptr.
     */
    anchor_ptr = HTMainText->first_anchor;
    /*
     *  Go through list of anchors and assemble URL query.
     */
    while (anchor_ptr) {
        if (anchor_ptr->link_type == INPUT_ANCHOR) {
	    if (anchor_ptr->input_field->number == form_number) {

                form_ptr = anchor_ptr->input_field;

                switch(form_ptr->type) {

	        case F_RESET_TYPE:
		    break;

	        case F_SUBMIT_TYPE:
	        case F_TEXT_SUBMIT_TYPE:
	        case F_IMAGE_SUBMIT_TYPE:
		    /*
		     *  If it has a non-zero length name (e.g., because
		     *  it's IMAGE_SUBMIT_TYPE to be handled homologously
		     *  to an image map, or a SUBMIT_TYPE in a set of
		     *  multiple submit buttons, or a single type="text"
		     *  that's been converted to a TEXT_SUBMIT_TYPE),
		     *  include the name=value pair, or fake name.x=0 and
		     *  name.y=0 pairs for IMAGE_SUBMIT_TYPE. - FM
		     */
		    if ((form_ptr->name && *form_ptr->name != '\0' &&
		        !strcmp(form_ptr->name, link_name)) &&
		       (form_ptr->type == F_TEXT_SUBMIT_TYPE ||
		        (form_ptr->value && *form_ptr->value != '\0' &&
		         !strcmp(form_ptr->value, link_value)))) {
		        if (first_one) {
			    if (Boundary) {
			        sprintf(&query[strlen(query)],
					"--%s\r\n", Boundary);
			    }
                            first_one=FALSE;
                        } else {
			    if (PlainText) {
			        strcat(query, "\n");
			    } else if (SemiColon) {
			        strcat(query, ";");
			    } else if (Boundary) {
			        sprintf(&query[strlen(query)],
					"\r\n--%s\r\n", Boundary);
			    } else {
                                strcat(query, "&");
			    }
			}

			if (PlainText) {
			    StrAllocCopy(escaped1, (form_ptr->name ?
			    			    form_ptr->name : ""));
			} else if (Boundary) {
			    StrAllocCopy(escaped1,
			    	    "Content-Disposition: form-data; name=");
			    StrAllocCat(escaped1, (form_ptr->name ?
			    			    form_ptr->name : ""));
			    if (MultipartContentType)
			        StrAllocCat(escaped1, MultipartContentType);
			    StrAllocCat(escaped1, "\r\n\r\n");
			} else {
		            escaped1 = HTEscape(form_ptr->name,URL_XALPHAS);
			}

		        /*
		         *  Be sure to actually look at
			 *  the option submit value.
		         */
		        if (form_ptr->cp_submit_value != NULL) {
			    for (i = 0; form_ptr->cp_submit_value[i]; i++) {
			        if (form_ptr->cp_submit_value[i] ==
					HT_NON_BREAK_SPACE ||
				    form_ptr->cp_submit_value[i] ==
				    	HT_EM_SPACE) {
				    if (PlainText) {
				        form_ptr->cp_submit_value[i] = ' ';
				    } else {
				        form_ptr->cp_submit_value[i] = 160;
				    }
				} else if (form_ptr->cp_submit_value[i] ==
					LY_SOFT_HYPHEN) {
				    form_ptr->cp_submit_value[i] = 173;
				}
			    }
			    if (PlainText || Boundary) {
			        StrAllocCopy(escaped2,
					     (form_ptr->cp_submit_value ?
					      form_ptr->cp_submit_value : ""));
			    } else {
		    	        escaped2 = HTEscapeSP(form_ptr->cp_submit_value,
						      URL_XALPHAS);
			    }
		        } else {
			    for (i = 0; form_ptr->value[i]; i++) {
			        if (form_ptr->value[i] ==
					HT_NON_BREAK_SPACE ||
				    form_ptr->value[i] ==
				    	HT_EM_SPACE) {
				    if (PlainText) {
				        form_ptr->value[i] = ' ';
				    } else {
				        form_ptr->value[i] = 160;
				    }
				} else if (form_ptr->value[i] ==
					LY_SOFT_HYPHEN) {
				    form_ptr->value[i] = 173;
				}
			    }
			    if (PlainText || Boundary) {
			        StrAllocCopy(escaped2, (form_ptr->value ?
							form_ptr->value : ""));
			    } else {
			        escaped2 = HTEscapeSP(form_ptr->value,
						      URL_XALPHAS);
			    }
		        }

			if (form_ptr->type == F_IMAGE_SUBMIT_TYPE) {
			    /*
			     *  It's a clickable image submit button.
			     *  Fake a 0,0 coordinate pair, which
			     *  typically returns the image's default. - FM
			     */
			    if (Boundary) {
			        escaped1[(strlen(escaped1) - 4)] = '\0';
			        sprintf(&query[strlen(query)],
				    "%s.x\r\n\r\n0\r\n--%s\r\n%s.y\r\n\r\n0",
					escaped1,
					Boundary,
					escaped1);
			    } else {
			        sprintf(&query[strlen(query)],
					"%s.x=0%s%s.y=0%s",
					escaped1,
					(PlainText ?
					      "\n" : (SemiColon ?
							    ";" : "&")),
					escaped1,
					((PlainText && *escaped1) ?
				    			     "\n" : ""));
			    }
			} else {
			    /*
			     *  It's a standard submit button.
			     *  Use the name=value pair. = FM
			     */
			    sprintf(&query[strlen(query)],
				    "%s%s%s%s%s",
				    escaped1,
				    (Boundary ?
				    	   "" : "="),
				    (PlainText ?
				          "\n" : ""),
				    escaped2,
				    ((PlainText && *escaped2) ?
				    			 "\n" : ""));
			}
		        FREE(escaped1);
		        FREE(escaped2);
		    }
		    break;

	        case F_RADIO_TYPE:
                case F_CHECKBOX_TYPE:
		    /*
		     *  Only add if selected.
		     */
		    if (form_ptr->num_value) {
	                if (first_one) {
			    if (Boundary) {
			        sprintf(&query[strlen(query)],
					"--%s\r\n", Boundary);
			    }
		            first_one=FALSE;
	                } else {
			    if (PlainText) {
			        strcat(query, "\n");
			    } else if (SemiColon) {
			        strcat(query, ";");
			    } else if (Boundary) {
			        sprintf(&query[strlen(query)],
					"\r\n--%s\r\n", Boundary);
			    } else {
		                strcat(query, "&");
			    }
			}

			if (PlainText) {
			    StrAllocCopy(escaped1, (form_ptr->name ?
			    			    form_ptr->name : ""));
			} else if (Boundary) {
			    StrAllocCopy(escaped1,
			    	     "Content-Disposition: form-data; name=");
			    StrAllocCat(escaped1,
				        (form_ptr->name ?
			    		 form_ptr->name : ""));
			    if (MultipartContentType)
			        StrAllocCat(escaped1, MultipartContentType);
			    StrAllocCat(escaped1, "\r\n\r\n");
			} else {
		            escaped1 = HTEscape(form_ptr->name, URL_XALPHAS);
			}
			/*
			 *  Be sure to use the submit option value.
			 */
			if (form_ptr->cp_submit_value != NULL) {
			    for (i = 0; form_ptr->cp_submit_value[i]; i++) {
			        if (form_ptr->cp_submit_value[i] ==
					HT_NON_BREAK_SPACE ||
				    form_ptr->cp_submit_value[i] ==
				    	HT_EM_SPACE) {
				    if (PlainText) {
				        form_ptr->cp_submit_value[i] = ' ';
				    } else {
				        form_ptr->cp_submit_value[i] = 160;
				    }
				 } else if (form_ptr->cp_submit_value[i] ==
				    	LY_SOFT_HYPHEN) {
				    form_ptr->cp_submit_value[i] = 173;
				 }
			    }
			    if (PlainText || Boundary) {
			        StrAllocCopy(escaped2,
					     (form_ptr->cp_submit_value ?
					      form_ptr->cp_submit_value : ""));
			    } else {
			        escaped2 = HTEscapeSP(form_ptr->cp_submit_value,
						      URL_XALPHAS);
			    }
			} else {
			    for (i = 0; form_ptr->value[i]; i++) {
				if (form_ptr->value[i] ==
					HT_NON_BREAK_SPACE ||
				    form_ptr->value[i] ==
				    	HT_EM_SPACE) {
				    if (PlainText) {
				        form_ptr->value[i] = ' ';
				    } else {
				        form_ptr->value[i] = 160;
				    }
				} else if (form_ptr->value[i] ==
				    	LY_SOFT_HYPHEN) {
				    form_ptr->value[i] = 173;

				}
			    }
			    if (PlainText || Boundary) {
			        StrAllocCopy(escaped2, (form_ptr->value ?
							form_ptr->value : ""));
			    } else {
		                escaped2 = HTEscapeSP(form_ptr->value,
						      URL_XALPHAS);
			    }
			}

                        sprintf(&query[strlen(query)],
				"%s%s%s%s%s",
				escaped1,
				(Boundary ?
				       "" : "="),
				(PlainText ?
				      "\n" : ""),
				escaped2,
				((PlainText && *escaped2) ?
						     "\n" : ""));
		        FREE(escaped1);
		        FREE(escaped2);
		    }
		    break;
		
		case F_TEXTAREA_TYPE:
		    for (i = 0; form_ptr->value[i]; i++) {
			if (form_ptr->value[i] == HT_NON_BREAK_SPACE ||
			    form_ptr->value[i] == HT_EM_SPACE) {
			    if (PlainText) {
			        form_ptr->value[i] = ' ';
			    } else {
			        form_ptr->value[i] = 160;
			    }
			} else if (form_ptr->value[i] == LY_SOFT_HYPHEN) {
			    form_ptr->value[i] = 173;
			}
		    }
		    if (PlainText || Boundary) {
		        StrAllocCopy(escaped2, (form_ptr->value ? 
						form_ptr->value : ""));
		    } else {
                        escaped2 = HTEscapeSP(form_ptr->value, URL_XALPHAS);
		    }

		    if (!last_textarea_name || 
			strcmp(last_textarea_name, form_ptr->name)) {
			/*
			 *  Names are different so this is the first
			 *  textarea or a different one from any before
			 *  it.
			 */
			FREE(previous_blanks);
		        if (first_one) {
			    if (Boundary) {
			        sprintf(&query[strlen(query)],
					"--%s\r\n", Boundary);
			    }
                            first_one=FALSE;
                        } else {
			    if (PlainText) {
			        strcat(query, "\n");
			    } else if (SemiColon) {
			        strcat(query, ";");
			    } else if (Boundary) {
			        sprintf(&query[strlen(query)],
					"\r\n--%s\r\n", Boundary);
			    } else {
                                strcat(query, "&");
			    }
			}
			if (PlainText) {
			    StrAllocCopy(escaped1, (form_ptr->name ?
			    			    form_ptr->name : ""));
			} else if (Boundary) {
			    StrAllocCopy(escaped1,
			    	    "Content-Disposition: form-data; name=");
			    StrAllocCat(escaped1, (form_ptr->name ?
			    			    form_ptr->name : ""));
			    if (MultipartContentType)
			        StrAllocCat(escaped1, MultipartContentType);
			    StrAllocCat(escaped1, "\r\n\r\n");
			} else {
                            escaped1 = HTEscape(form_ptr->name, URL_XALPHAS);
			}
                        sprintf(&query[strlen(query)],
				"%s%s%s%s%s",
				escaped1,
				(Boundary ?
				       "" : "="),
				(PlainText ?
				      "\n" : ""),
				escaped2,
				((PlainText && *escaped2) ?
						     "\n" : ""));
                        FREE(escaped1);
			last_textarea_name = form_ptr->name;
		    } else {
			/*
			 *  This is a continuation of a previous textarea
			 *  add %0a (\n) and the escaped string.
			 */
			if (escaped2[0] != '\0') {
			    if (previous_blanks) {
				strcat(query, previous_blanks);
				FREE(previous_blanks);
			    }
			    if (PlainText) {
			        sprintf(&query[strlen(query)], "%s\n",
							       escaped2);
			    } else if (Boundary) {
			        sprintf(&query[strlen(query)], "%s\r\n",
							       escaped2);
			    } else {
			        sprintf(&query[strlen(query)], "%%0a%s",
							       escaped2);
			    }
			} else {
			    if (PlainText) {
			        StrAllocCat(previous_blanks, "\n");
			    } else if (Boundary) {
			        StrAllocCat(previous_blanks, "\r\n");
			    } else {
			        StrAllocCat(previous_blanks, "%0a");
			    }
			}
		    }
                    FREE(escaped2);
                    break;

                case F_PASSWORD_TYPE:
	        case F_TEXT_TYPE:
		case F_OPTION_LIST_TYPE:
		case F_HIDDEN_TYPE:
	            if (first_one) {
			if (Boundary) {
			    sprintf(&query[strlen(query)],
				    "--%s\r\n", Boundary);
			}
		        first_one=FALSE;
	            } else {
		        if (PlainText) {
			    strcat(query, "\n");
			} else if (SemiColon) {
			    strcat(query, ";");
			} else if (Boundary) {
			    sprintf(&query[strlen(query)],
			    	    "\r\n--%s\r\n", Boundary);
			} else {
		            strcat(query, "&");
			}
		    }
    
		    if (PlainText) {
		       StrAllocCopy(escaped1, (form_ptr->name ?
		       			       form_ptr->name : ""));
		    } else if (Boundary) {
			StrAllocCopy(escaped1,
			    	    "Content-Disposition: form-data; name=");
			StrAllocCat(escaped1, (form_ptr->name ?
			    		       form_ptr->name : ""));
			if (MultipartContentType)
			    StrAllocCat(escaped1, MultipartContentType);
			StrAllocCat(escaped1, "\r\n\r\n");
		    } else {
		        escaped1 = HTEscape(form_ptr->name, URL_XALPHAS);
		    }

		    /*
		     *	Be sure to actually look at the option submit value.
		     */
		    if (form_ptr->cp_submit_value != NULL) {
			for (i = 0; form_ptr->cp_submit_value[i]; i++) {
			    if (form_ptr->cp_submit_value[i] ==
			    		HT_NON_BREAK_SPACE ||
			        form_ptr->cp_submit_value[i] ==
					HT_EM_SPACE) {
				if (PlainText) {
				    form_ptr->cp_submit_value[i] = ' ';
				} else {
				    form_ptr->cp_submit_value[i] = 160;
				}
			    } else if (form_ptr->cp_submit_value[i] ==
					LY_SOFT_HYPHEN) {
				form_ptr->cp_submit_value[i] = 173;
			    }
			}
			if (PlainText || Boundary) {
			    StrAllocCopy(escaped2,
			    		 (form_ptr->cp_submit_value ?
					  form_ptr->cp_submit_value : ""));
			} else {
		    	    escaped2 = HTEscapeSP(form_ptr->cp_submit_value,
					          URL_XALPHAS);
			}
		    } else {
			for (i = 0; form_ptr->value[i]; i++) {
			    if (form_ptr->value[i] ==
			    		HT_NON_BREAK_SPACE ||
			        form_ptr->value[i] ==
					HT_EM_SPACE) {
				if (PlainText) {
				    form_ptr->value[i] = ' ';
				} else {
				    form_ptr->value[i] = 160;
				}
			    } else if (form_ptr->value[i] ==
					LY_SOFT_HYPHEN) {
				form_ptr->value[i] = 173;
			    }
			}
			if (PlainText || Boundary) {
			    StrAllocCopy(escaped2, (form_ptr->value ?
			    			    form_ptr->value : ""));
			} else {
			    escaped2 = HTEscapeSP(form_ptr->value,
			    			  URL_XALPHAS);
			}
		    }

                    sprintf(&query[strlen(query)],
		    	    "%s%s%s%s%s",
			    escaped1,
			    (Boundary ?
			    	   "" : "="),
			    (PlainText ?
				  "\n" : ""),
			    escaped2,
			    ((PlainText && *escaped2) ?
		    				 "\n" : ""));
		    FREE(escaped1);
		    FREE(escaped2);
		    break;
	        }
	    } else if (anchor_ptr->input_field->number > form_number) {
	        break;
	    }
        }

        if (anchor_ptr == HTMainText->last_anchor)
            break;

	anchor_ptr = anchor_ptr->next;
    }
    if (Boundary) {
        sprintf(&query[strlen(query)], "\r\n--%s--\r\n", Boundary);
    }
    FREE(previous_blanks);

    if (submit_item->submit_method == URL_MAIL_METHOD) {
	_user_message("Submitting %s", submit_item->submit_action);
	if (TRACE) {
	    fprintf(stderr, "\nGridText - mailto_address: %s\n",
	    		    (submit_item->submit_action+7));
	    fprintf(stderr, "GridText - mailto_subject: %s\n",
	    		    ((submit_item->submit_title &&
			      *submit_item->submit_title) ?
			      (submit_item->submit_title) :
			      	        (HText_getTitle() ?
				         HText_getTitle() : "")));
	    fprintf(stderr,"GridText - mailto_content: %s\n",query);
	}
	sleep(MessageSecs);
	mailform((submit_item->submit_action+7),
		 ((submit_item->submit_title &&
		   *submit_item->submit_title) ?
		   (submit_item->submit_title) :
		   	     (HText_getTitle() ?
			      HText_getTitle() : "")),
		 query,
		 doc->post_content_type);
	FREE(query);
        FREE(doc->post_content_type);
	return;
    } else {
        _statusline(SUBMITTING_FORM);
    }
   
    if (submit_item->submit_method == URL_POST_METHOD || Boundary) {
        StrAllocCopy(doc->post_data, query);
        if (TRACE)
	    fprintf(stderr,"GridText - post_data: %s\n",doc->post_data);
        StrAllocCopy(doc->address, submit_item->submit_action);
        FREE(query);
        return;
    } else { /* GET_METHOD */ 
        StrAllocCopy(doc->address, query);
        FREE(doc->post_data);
        FREE(doc->post_content_type);
        FREE(query);
        return;
    }
}

PUBLIC void HText_DisableCurrentForm NOARGS
{
    TextAnchor * anchor_ptr;

    HTFormDisabled = TRUE;
    if (!HTMainText)
        return;

    /*
     *  Go through list of anchors and set the disabled flag.
     */
    anchor_ptr = HTMainText->first_anchor;
    while (anchor_ptr) {
        if (anchor_ptr->link_type == INPUT_ANCHOR &&
            anchor_ptr->input_field->number == HTFormNumber) {

            anchor_ptr->input_field->disabled = TRUE;
        }

        if (anchor_ptr == HTMainText->last_anchor)
            break;


        anchor_ptr = anchor_ptr->next;
    }

    return;
}

PUBLIC void HText_ResetForm ARGS1(
	FormInfo *,	form)
{
    TextAnchor * anchor_ptr;

    _statusline(RESETTING_FORM);
    if (!HTMainText)
        return;

    /*
     *  Go through list of anchors and reset values.
     */
    anchor_ptr = HTMainText->first_anchor;
    while (anchor_ptr) {
        if (anchor_ptr->link_type == INPUT_ANCHOR) {
            if (anchor_ptr->input_field->number == form->number) {

                 if (anchor_ptr->input_field->type == F_RADIO_TYPE ||
                     anchor_ptr->input_field->type == F_CHECKBOX_TYPE) {

		    if (anchor_ptr->input_field->orig_value[0] == '0')
		        anchor_ptr->input_field->num_value = 0;
		    else
		        anchor_ptr->input_field->num_value = 1;
		
		 } else if (anchor_ptr->input_field->type ==
		 	    F_OPTION_LIST_TYPE) {
		    anchor_ptr->input_field->value =
				anchor_ptr->input_field->orig_value;
		    
		    anchor_ptr->input_field->cp_submit_value =
		    		anchor_ptr->input_field->orig_submit_value;

	         } else {
		    StrAllocCopy(anchor_ptr->input_field->value,
					anchor_ptr->input_field->orig_value);
		 }
	     } else if (anchor_ptr->input_field->number > form->number) {
                 break;
	     }

        }

        if (anchor_ptr == HTMainText->last_anchor)
            break;


        anchor_ptr = anchor_ptr->next;
    }
}

PUBLIC void HText_activateRadioButton ARGS1(
	FormInfo *,	form)
{
    TextAnchor * anchor_ptr;
    int form_number = form->number;

    if (!HTMainText)
        return;
    anchor_ptr = HTMainText->first_anchor;
    while (anchor_ptr) {
        if (anchor_ptr->link_type == INPUT_ANCHOR &&
                anchor_ptr->input_field->type == F_RADIO_TYPE) {
                    
	    if (anchor_ptr->input_field->number == form_number) {

		    /* if it has the same name and its on */
	         if (!strcmp(anchor_ptr->input_field->name, form->name) &&
		     anchor_ptr->input_field->num_value) {
		    anchor_ptr->input_field->num_value = 0;
		    break;
	         }
	    } else if (anchor_ptr->input_field->number > form_number) {
	            break;
	    }

        }

        if (anchor_ptr == HTMainText->last_anchor)
            break;

        anchor_ptr = anchor_ptr->next;
   }

   form->num_value = 1;
}

/*
 *	Purpose:	Free all currently loaded HText objects in memory.
 *	Arguments:	void
 *	Return Value:	void
 *	Remarks/Portability/Dependencies/Restrictions:
 *		Usage of this function should really be limited to program
 *			termination.
 *	Revision History:
 *		05-27-94	created Lynx 2-3-1 Garrett Arch Blythe
 */
PRIVATE void free_all_texts NOARGS
{
    HText *cur = NULL;

    if (!loaded_texts)
	return;

    /*
     *  Simply loop through the loaded texts list killing them off.
     */
    while (loaded_texts && !HTList_isEmpty(loaded_texts)) {
        if ((cur = (HText *)HTList_removeLastObject(loaded_texts)) != NULL) {
	    HText_free(cur);
	}
    }

    /*
     *  Get rid of the text list.
     */
    if (loaded_texts) {
	HTList_delete(loaded_texts);
    }

    /*
     *  Insurance for bad HTML.
     */
    FREE(HTCurSelectGroup);
    FREE(HTCurSelectGroupSize);
    FREE(HTCurSelectedOptionValue);
    FREE(HTFormAction);
    FREE(HTFormEnctype);
    FREE(HTFormTitle);

    return;
}

/*
**  stub_HTAnchor_address is like HTAnchor_address, but it returns the
**  parent address for child links.  This is only useful for traversal's
**  where one does not want to index a text file N times, once for each
**  of N internal links.  Since the parent link has already been taken,
**  it won't go again, hence the (incorrect) links won't cause problems.
*/
PUBLIC char * stub_HTAnchor_address ARGS1(
	HTAnchor *,	me)
{
    char *addr = NULL;
    if (me)
	StrAllocCopy (addr, me->parent->address);
    return addr;
}

PUBLIC void HText_setToolbar ARGS1(
	HText *,	text)
{
    if (text)
        text->toolbar = TRUE;
    return;
}

PUBLIC BOOL HText_hasToolbar ARGS1(
	HText *,	text)
{
    return ((text && text->toolbar) ? TRUE : FALSE);
}

PUBLIC void HText_setNoCache ARGS1(
	HText *,	text)
{
    if (text)
        text->no_cache = TRUE;
    return;
}

PUBLIC BOOL HText_hasNoCacheSet ARGS1(
	HText *,	text)
{
    return ((text && text->no_cache) ? TRUE : FALSE);
}

/*
**  Check charset and set the kcode element. - FM
*/
PUBLIC void HText_setKcode ARGS2(
	HText *,	text,
	CONST char *,	charset)
{
    if (!text)
        return;

    /*
    **  Check whether we have a sepecified charset. - FM
    */
    if (!charset || *charset == '\0') {
	return;
    }

    /*
    **  We've included the charset, and not forced a download offer,
    **  only if the currently selected character set can handle it,
    **  so check the charset value and set the text->kcode element
    **  appropriately. - FM
    */
    if (!strcmp(charset, "shift_jis")) {
	text->kcode = SJIS;
    } else if (!strcmp(charset, "euc-jp") ||
	       !strcmp(charset, "iso-2022-jp") ||
	       !strcmp(charset, "iso-2022-jp-2") ||
	       !strcmp(charset, "euc-kr") ||
	       !strcmp(charset, "iso-2022-kr") ||
	       !strcmp(charset, "big5") ||
	       !strcmp(charset, "euc-cn") ||
	       !strcmp(charset, "gb2312") ||
	       !strcmp(charset, "iso-2022-cn")) {
	text->kcode = EUC;
    } else {
        /*
	**  If we get to here, it's not CJK, so disable that. - FM
	*/
	text->kcode = NOKANJI;
	HTCJK = NOCJK;
    }

    return;
}

