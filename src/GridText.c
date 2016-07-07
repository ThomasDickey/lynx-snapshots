/*		Character grid hypertext object
**		===============================
*/

#include <HTUtils.h>
#include <tcp.h>
#include <HTString.h>
#include <HTFont.h>
#include <HTAccess.h>
#include <HTAnchor.h>
#include <HTParse.h>
#include <HTTP.h>
#include <HTAlert.h>
#include <HTCJK.h>
#include <UCDefs.h>
#include <UCAux.h>

#include <assert.h>
#include <ctype.h>
#ifndef VMS
#ifdef SYSLOG_REQUESTED_URLS
#include <syslog.h>
#endif /* SYSLOG_REQUESTED_URLS */
#endif /* !VMS */

#include <GridText.h>
#include <LYCurses.h>
#include <LYUtils.h>
#include <LYStrings.h>
#include <LYStructs.h>
#include <LYGlobalDefs.h>
#include <LYGetFile.h>
#include <LYSignal.h>
#include <LYMail.h>
#include <LYList.h>
#include <LYCharSets.h>
#include <LYCharUtils.h>	/* LYUCTranslateBack... */
#include <UCMap.h>
#ifdef EXP_CHARTRANS_AUTOSWITCH
#include <UCAuto.h>
#endif /* EXP_CHARTRANS_AUTOSWITCH */

#include <LYexit.h>
#include <LYLeaks.h>

#ifdef USE_COLOR_STYLE
#include <AttrList.h>
#include <LYHash.h>

unsigned int cached_styles[CACHEH][CACHEW];

#endif

#ifdef USE_COLOR_STYLE_UNUSED
void LynxClearScreenCache NOARGS
{
    int i,j;

    CTRACE(tfp, "flushing cached screen styles\n");
    for (i=0;i<CACHEH;i++)
	for (j=0;j<CACHEW;j++)
	    cached_styles[i][j]=s_a;
}
#endif /* USE_COLOR_STYLE */

struct _HTStream {                      /* only know it as object */
    CONST HTStreamClass *       isa;
    /* ... */
};

#define TITLE_LINES  1
#define IS_UTF_EXTRA(ch) (text->T.output_utf8 && \
			  ((unsigned char)(ch)&0xc0) == 0x80)

#define FREE(x) if (x) {free(x); x = NULL;}

extern BOOL HTPassHighCtrlRaw;
extern HTkcode kanji_code;
extern HTCJKlang HTCJK;

/*	Exports
*/
PUBLIC HText * HTMainText = NULL;		/* Equivalent of main window */
PUBLIC HTParentAnchor * HTMainAnchor = NULL;	/* Anchor for HTMainText */

PUBLIC char * HTAppName = "Lynx";		/* Application name */
PUBLIC char * HTAppVersion = LYNX_VERSION;	/* Application version */

PUBLIC int HTFormNumber = 0;
PUBLIC int HTFormFields = 0;
PUBLIC char * HTCurSelectGroup = NULL;		/* Form select group name */
PRIVATE int HTCurSelectGroupCharset = -1;	/* ... and name's charset */
PUBLIC int HTCurSelectGroupType = F_RADIO_TYPE;	/* Group type */
PUBLIC char * HTCurSelectGroupSize = NULL;	/* Length of select */
PRIVATE char * HTCurSelectedOptionValue = NULL;	/* Select choice */

PUBLIC char * checked_box = "[X]";
PUBLIC char * unchecked_box = "[ ]";
PUBLIC char * checked_radio = "(*)";
PUBLIC char * unchecked_radio = "( )";

PUBLIC BOOLEAN underline_on = OFF;
PUBLIC BOOLEAN bold_on      = OFF;

#if defined(USE_COLOR_STYLE)
#define MAX_STYLES_ON_LINE 64

typedef struct _stylechange {
	int	horizpos;	/* horizontal position of this change */
	int	style;		/* which style to change to */
	int	direction;	/* on or off */
	int	previous;	/* previous style */
} HTStyleChange;
#endif

typedef struct _line {
	struct _line	*next;
	struct _line	*prev;
	int unsigned	offset;		/* Implicit initial spaces */
	int unsigned	size;		/* Number of characters */
	BOOL	split_after;		/* Can we split after? */
	BOOL	bullet;			/* Do we bullet? */
#if defined(USE_COLOR_STYLE)
	HTStyleChange	styles[MAX_STYLES_ON_LINE];
	int	numstyles;
#endif
	char	data[1];		/* Space for terminator at least! */
} HTLine;

#define LINE_SIZE(l) (sizeof(HTLine)+(l))	/* Allow for terminator */

typedef struct _TextAnchor {
	struct _TextAnchor *	next;
	int			number;		/* For user interface */
	int			start;		/* Characters */
	int			line_pos;	/* Position in text */
	int			extent; 	/* Characters */
	int			line_num;	/* Place in document */
	char *			hightext;	/* The link text */
	char *			hightext2;	/* A second line*/
	int			hightext2offset;/* offset from left */
	int			link_type;	/* Normal, internal, or form? */
	FormInfo *		input_field;	/* Info for form links */
	BOOL			show_anchor;	/* Show the anchor? */
	BOOL			inUnderline;	/* context is underlined */
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
	HTList *		forms;		/* also linked internally */
	int			last_anchor_number;	/* user number */
	BOOL			source;		/* Is the text source? */
	BOOL			toolbar;	/* Toolbar set? */
	HTList *		tabs;		/* TAB IDs */
	HTList *		hidden_links;	/* Content-less links ... */
	int			hiddenlinkflag; /*  ... and how to treat them */
	BOOL			no_cache;	/* Always refresh? */
	char			LastChar;	/* For absorbing white space */
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
	BOOL			page_has_target; /* has target on screen */

	HTkcode 		kcode;			/* Kanji code? */
	enum grid_state       { S_text, S_esc, S_dollar, S_paren,
				S_nonascii_text, S_dollar_paren,
				S_jisx0201_text }
				state;			/* Escape sequence? */
	char			kanji_buf;		/* Lead multibyte */
	int			in_sjis;		/* SJIS flag */
	int			halted; 		/* emergency halt */

	BOOL			have_8bit_chars;   /* Any non-ASCII chars? */
	LYUCcharset *		UCI;		   /* node_anchor UCInfo */
	int			UCLYhndl;	   /* charset we are fed */
	UCTransParams		T;

	HTStream *		target; 		/* Output stream */
	HTStreamClass		targetClass;		/* Output routines */
};

PRIVATE void HText_AddHiddenLink PARAMS((HText *text, TextAnchor *textanchor));

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
	(HTFont)0, 1, HT_BLACK,		0, 0,
	0, 0, 0, HT_LEFT,		1, 0,	0,
	NO, NO, 0, 0,			0 };



PRIVATE HTList * loaded_texts = NULL;	 /* A list of all those in memory */
PUBLIC  HTList * search_queries = NULL;  /* isindex and whereis queries   */
PRIVATE void free_all_texts NOARGS;
PRIVATE int HText_TrueLineSize PARAMS((
	HTLine *	line,
	HText *		text,
	BOOL		IgnoreSpaces));

#ifndef VMS			/* VMS has a better way - right? - kw */
#define CHECK_FREE_MEM
#endif

#ifdef CHECK_FREE_MEM

/*
 *  text->halted = 1: have set fake 'Z' and output a message
 *                 2: next time when HText_appendCharacter is called
 *		      it will append *** MEMORY EXHAUSTED ***, then set
 *		      to 3.
 *		   3: normal text output will be suppressed (but not anchors,
 *		      form fields etc.)
 */
PRIVATE void HText_halt NOARGS
{
    if (HTFormNumber > 0)
	HText_DisableCurrentForm();
    if (!HTMainText)
	return;
    if (HTMainText->halted < 2)
	HTMainText->halted = 2;
}

#define MIN_NEEDED_MEM 5000

/*
 *  Check whether factor*min(bytes,MIN_NEEDED_MEM) is available,
 *  or bytes if factor is 0.
 *  MIN_NEEDED_MEM and factor together represent a security margin,
 *  to take account of all the memory allocations where we don't check
 *  and of buffers which may be emptied before HTCheckForInterupt()
 *  is (maybe) called and other things happening, with some chance of
 *  success.
 *  This just tries to malloc() the to-be-checked-for amount of memory,
 *  which might make the situation worse depending how allocation works.
 *  There should be a better way... - kw
 */
PRIVATE BOOL mem_is_avail ARGS2(
    size_t,	factor,
    size_t,	bytes)
{
    void *p;
    if (bytes < MIN_NEEDED_MEM && factor > 0)
	bytes = MIN_NEEDED_MEM;
    if (factor == 0)
	factor = 1;
    p = malloc(factor * bytes);
    if (p) {
	FREE(p);
	return YES;
    } else {
	return NO;
    }
}

/*
 *  Replacement for calloc which checks for "enough" free memory
 *  (with some security margins) and tries various recovery actions
 *  if deemed necessary. - kw
 */
PRIVATE void * LY_check_calloc ARGS2(
    size_t,	nmemb,
    size_t,	size)
{
    int i, n;
    if (mem_is_avail(4, nmemb * size)) {
	return (calloc(nmemb, size));
    }
    n = HTList_count(loaded_texts);
    for (i = n - 1; i > 0; i--) {
	HText * t = HTList_objectAt(loaded_texts, i);
	if (t == HTMainText)
	    t = NULL;		/* shouldn't happen */
	{
	CTRACE(tfp, "\r *** Emergency freeing document %d/%d for '%s'%s!\n",
		    i + 1, n,
		    ((t && t->node_anchor &&
		      t->node_anchor->address) ?
		     t->node_anchor->address : "unknown anchor"),
		    ((t && t->node_anchor &&
		      t->node_anchor->post_data) ?
		     " with POST data" : ""));
	}
	HTList_removeObjectAt(loaded_texts, i);
	HText_free(t);
	if (mem_is_avail(4, nmemb * size)) {
	    return (calloc(nmemb, size));
	}
    }
    LYFakeZap(YES);
    if (!HTMainText || HTMainText->halted <= 1) {
	if (!mem_is_avail(2, nmemb * size)) {
	    HText_halt();
	    if (mem_is_avail(0, 700)) {
		HTAlert("Memory exhausted, display interrupted!");
	    }
	} else {
	    if ((!HTMainText || HTMainText->halted == 0) &&
		mem_is_avail(0, 700)) {
		HTAlert("Memory exhausted, will interrupt transfer!");
		if (HTMainText)
		    HTMainText->halted = 1;
	    }
	}
    }
    return (calloc(nmemb, size));
}

#define LY_CALLOC LY_check_calloc

#else  /* CHECK_FREE_MEM */

  /* using the regular calloc */
#define LY_CALLOC calloc

#endif /* CHECK_FREE_MEM */

PRIVATE void HText_getChartransInfo ARGS1(
	HText *,	me)
{
    me->UCLYhndl = HTAnchor_getUCLYhndl(me->node_anchor, UCT_STAGE_HTEXT);
    if (me->UCLYhndl < 0) {
	int chndl = current_char_set;
	HTAnchor_setUCInfoStage(me->node_anchor, chndl,
				UCT_STAGE_HTEXT, UCT_SETBY_STRUCTURED);
	me->UCLYhndl = HTAnchor_getUCLYhndl(me->node_anchor,
					    UCT_STAGE_HTEXT);
    }
    me->UCI = HTAnchor_getUCInfoStage(me->node_anchor, UCT_STAGE_HTEXT);
}

PRIVATE void PerFormInfo_free ARGS1(
    PerFormInfo *,	form)
{
    if (form) {
	FREE(form->accept_cs);
	FREE(form->thisacceptcs);
	FREE(form);
    }
}

PRIVATE void FormList_delete ARGS1(
    HTList *,		forms)
{
    HTList *cur = forms;
    PerFormInfo *form;
    while ((form = (PerFormInfo *)HTList_nextObject(cur)) != NULL)
	PerFormInfo_free(form);
    HTList_delete(forms);
}

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
    CTRACE(tfp, "GridText: VMTotal = %d\n", VMTotal);
#endif /* VMS && VAXC && !__DECC */

    if (!loaded_texts)	{
	loaded_texts = HTList_new();
	atexit(free_all_texts);
    }

    /*
     *  Links between anchors & documents are a 1-1 relationship. If
     *  an anchor is already linked to a document we didn't call
     *  HTuncache_current_document(), e.g., for the showinfo, options,
     *  download, print, etc., temporary file URLs, so we'll check now
     *  and free it before reloading. - Dick Wesseling (ftu@fi.ruu.nl)
     */
    if (anchor->document) {
	HTList_removeObject(loaded_texts, anchor->document);
	CTRACE(tfp, "GridText: Auto-uncaching\n") ;
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
	CTRACE(tfp, "GridText: Freeing off cached doc.\n");
	HText_free((HText *)HTList_removeFirstObject(loaded_texts));
#if defined(VMS) && defined (VAXC) && !defined(__DECC)
	status = lib$stat_vm(&VMType, &VMTotal);
	CTRACE(tfp, "GridText: VMTotal reduced to %d\n", VMTotal);
#endif /* VMS && VAXC && !__DECC */
    }

    line = self->last_line = (HTLine *)calloc(1, LINE_SIZE(MAX_LINE));
    if (line == NULL)
	outofmem(__FILE__, "HText_New");
    line->next = line->prev = line;
    line->offset = line->size = 0;
#ifdef USE_COLOR_STYLE
    line->numstyles = 0;
#endif
    self->Lines = self->chars = 0;
    self->first_anchor = self->last_anchor = NULL;
    self->style = &default_style;
    self->top_of_screen = 0;
    self->node_anchor = anchor;
    self->last_anchor_number = 0;	/* Numbering of them for references */
    self->stale = YES;
    self->toolbar = NO;
    self->tabs = NULL;
    /*
     *  If we are going to render the List Page, always merge in hidden
     *  links to get the numbering consistent if form fields are numbered
     *  and show up as hidden links in the list of links. - kw
     */
    if (anchor->address && !strcmp(anchor->address, LYlist_temp_url()))
	self->hiddenlinkflag = HIDDENLINKS_MERGE;
    else
	self->hiddenlinkflag = LYHiddenLinks;
    self->hidden_links = NULL;
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
    self->have_8bit_chars = NO;
    HText_getChartransInfo(self);
    UCSetTransParams(&self->T,
		     self->UCLYhndl, self->UCI,
		     current_char_set,
		     &LYCharSet_UC[current_char_set]);

    /*
     *  Check the kcode setting if the anchor has a charset element. - FM
     */
    if (anchor->charset)
	HText_setKcode(self, anchor->charset,
		       HTAnchor_getUCInfoStage(anchor, UCT_STAGE_HTEXT));

    /*
     *	Memory leak fixed.
     *  05-29-94 Lynx 2-3-1 Garrett Arch Blythe
     *	Check to see if our underline and star_string need initialization
     *		if the underline is not filled with dots.
     */
    if (underscore_string[0] != '.') {
	/*
	 *  Create an array of dots for the UNDERSCORES macro. - FM
	 */
	memset(underscore_string, '.', (MAX_LINE-1));
	underscore_string[(MAX_LINE-1)] = '\0';
	underscore_string[MAX_LINE] = '\0';
	/*
	 *  Create an array of underscores for the STARS macro. - FM
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
	this->targetClass = *stream->isa;	/* copy action procedures */
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
	    if (l->input_field->type == F_OPTION_LIST_TYPE &&
		l->input_field->select_list != NULL) {
		/*
		 *  Free off option lists if present.
		 *  It should always be present for F_OPTION_LIST_TYPE
		 *  unless we had invalid markup which prevented
		 *  HText_setLastOptionValue from finishing its job
		 *  and left the input field in an insane state. - kw
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

	    FREE(l->input_field->accept_cs);

	    FREE(l->input_field);
	}

	FREE(l->hightext);
	FREE(l->hightext2);

	FREE(l);
    }
    FormList_delete(self->forms);

    /*
     *  Free the tabs list. - FM
     */
    if (self->tabs) {
	HTTabID * Tab = NULL;
	HTList * cur = self->tabs;

	while (NULL != (Tab = (HTTabID *)HTList_nextObject(cur))) {
	    FREE(Tab->name);
	    FREE(Tab);
	}
	HTList_delete(self->tabs);
	self->tabs = NULL;
    }

    /*
     *  Free the hidden links list. - FM
     */
    if (self->hidden_links) {
	char * href = NULL;
	HTList * cur = self->hidden_links;

	while (NULL != (href = (char *)HTList_nextObject(cur)))
	    FREE(href);
	HTList_delete(self->hidden_links);
	self->hidden_links = NULL;
    }

    /*
     *  Invoke HTAnchor_delete() to free the node_anchor
     *  if it is not a destination of other links. - FM
     */
    if (self->node_anchor) {
	HTAnchor_resetUCInfoStage(self->node_anchor, -1, UCT_STAGE_STRUCTURED,
				  UCT_SETBY_NONE);
	HTAnchor_resetUCInfoStage(self->node_anchor, -1, UCT_STAGE_HTEXT,
				  UCT_SETBY_NONE);
	if (HTAnchor_delete(self->node_anchor))
	    /*
	     *  Make sure HTMainAnchor won't point
	     *  to an invalid structure. - KW
	     */
	    HTMainAnchor = NULL;
    }

    FREE(self);
}

/*		Display Methods
**		---------------
*/


/*	Output a line
**	-------------
*/
PRIVATE int display_line ARGS2(
	HTLine *,	line,
	HText *,	text)
{
    register int i, j;
    char buffer[7];
    char *data;
    size_t utf_extra = 0;
#ifdef USE_COLOR_STYLE
    int current_style = 0;
#endif
    char LastDisplayChar = ' ';

    /*
     *  Set up the multibyte character buffer,
     *  and clear the line to which we will be
     *  writing.
     */
    buffer[0] = buffer[1] = buffer[2] = '\0';
    clrtoeol();

    /*
     *  Add offset, making sure that we do not
     *  go over the COLS limit on the display.
     */
    j = (int)line->offset;
    if (j > (int)LYcols - 1)
	j = (int)LYcols - 1;
#ifdef USE_SLANG
    SLsmg_forward (j);
    i = j;
#else
#ifdef USE_COLOR_STYLE
    if (line->size == 0)
	i = j;
    else
#endif
    for (i = 0; i < j; i++)
	addch (' ');
#endif /* USE_SLANG */

    /*
     *  Add the data, making sure that we do not
     *  go over the COLS limit on the display.
     */
    data = line->data;
    i++;
    while ((i < LYcols) && ((buffer[0] = *data) != '\0')) {
	data++;

#if defined(USE_COLOR_STYLE) || defined(SLSC)
#define CStyle line->styles[current_style]

	while (current_style < line->numstyles &&
	       i >= (int) (CStyle.horizpos + line->offset + 1))
	{
		LynxChangeStyle (CStyle.style,CStyle.direction,CStyle.previous);
		current_style++;
	}
#endif
	switch (buffer[0]) {

#ifndef USE_COLOR_STYLE
	    case LY_UNDERLINE_START_CHAR:
	        if (dump_output_immediately && use_underscore) {
		    addch('_');
		    i++;
		} else {
		    start_underline();
		}
		break;

	    case LY_UNDERLINE_END_CHAR:
	        if (dump_output_immediately && use_underscore) {
		    addch('_');
		    i++;
		} else {
		    stop_underline();
		}
		break;

	    case LY_BOLD_START_CHAR:
		start_bold();
		break;

	    case LY_BOLD_END_CHAR:
		stop_bold ();
		break;
#endif

	    case LY_SOFT_HYPHEN:
	        if (*data != '\0' ||
		    isspace((unsigned char)LastDisplayChar) ||
		    LastDisplayChar == '-') {
		    /*
		     *  Ignore the soft hyphen if it is not the last
		     *  character in the line.  Also ignore it if it
		     *  first character following the margin, or if it
		     *  is preceded by a white character (we loaded 'M'
		     *  into LastDisplayChar if it was a multibyte
		     *  character) or hyphen, though it should have
		     *  been excluded by HText_appendCharacter() or by
		     *  split_line() in those cases. - FM
		     */
		    break;
		} else {
		    /*
		     *  Make it a hard hyphen and fall through. - FM
		     */
		    buffer[0] = '-';
		    i++;
		}

	    default:
		i++;
		if (text->T.output_utf8 && !isascii(buffer[0])) {
		    if ((*buffer & 0xe0) == 0xc0) {
			utf_extra = 1;
		    } else if ((*buffer & 0xf0) == 0xe0) {
			utf_extra = 2;
		    } else if ((*buffer & 0xf8) == 0xf0) {
			utf_extra = 3;
		    } else if ((*buffer & 0xfc) == 0xf8) {
			utf_extra = 4;
		    } else if ((*buffer & 0xfe) == 0xfc) {
			utf_extra = 5;
		    } else {
			 /*
			  *  Garbage.
			  */
			utf_extra = 0;
		    }
		    if (strlen(data) < utf_extra) {
			/*
			 *  Shouldn't happen.
			 */
			utf_extra = 0;
		    }
		    LastDisplayChar = 'M';
		}
		if (utf_extra) {
		    strncpy(&buffer[1], data, utf_extra);
		    buffer[utf_extra+1] = '\0';
		    addstr(buffer);
		    buffer[1] = '\0';
		    data += utf_extra;
		    utf_extra = 0;
		} else if (HTCJK != NOCJK && !isascii(buffer[0])) {
		    /*
		     *  For CJK strings, by Masanobu Kimura.
		     */
		    buffer[1] = *data;
		    data++;
		    addstr(buffer);
		    buffer[1] = '\0';
		    /*
		     *  For now, load 'M' into LastDisplayChar,
		     *  but we should check whether it's white
		     *  and if so, use ' '.  I don't know if
		     *  there actually are white CJK characters,
		     *  and we're loading ' ' for multibyte
		     *  spacing characters in this code set,
		     *  but this will become an issue when
		     *  the development code set's multibyte
		     *  character handling is used. - FM
		     */
		    LastDisplayChar = 'M';
		} else {
		    addstr(buffer);
		    LastDisplayChar = buffer[0];
		}
	} /* end of switch */
    } /* end of while */

    /*
     *  Add the return.
     */
    addch('\n');

#ifndef USE_COLOR_STYLE
    stop_underline();
    stop_bold();
#else
    while (current_style < line->numstyles)
    {
	LynxChangeStyle (CStyle.style, CStyle.direction, CStyle.previous);
	current_style++;
    }
#undef CStyle
#endif
    return(0);
}

/*	Output the title line
**	---------------------
*/
PRIVATE void display_title ARGS1(
	HText *,	text)
{
    char *title = NULL;
    char percent[20];
    char *cp = NULL;
    unsigned char *tmp = NULL;
    int i = 0, j = 0;

    /*
     *  Make sure we have a text structure. - FM
     */
    if (!text)
	return;

    lynx_start_title_color ();
#ifdef USE_COLOR_STYLE
/* turn the TITLE style on */
    LynxChangeStyle(s_title, ABS_ON, 0);
#endif /* USE_COLOR_STYLE */

    /*
     *  Load the title field. - FM
     */
    StrAllocCopy(title,
		 (HTAnchor_title(text->node_anchor) ?
		  HTAnchor_title(text->node_anchor) : ""));

    /*
     *  There shouldn't be any \n in the title field,
     *  but if there is, lets kill it now.  Also trim
     *  any trailing spaces. - FM
     */
    if ((cp = strchr(title,'\n')) != NULL)
	*cp = '\0';
    i = (*title ? (strlen(title) - 1) : 0);
    while ((i >= 0) && title[i] == ' ')
	title[i--] = '\0';

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
	percent[0] = '\0';	/* Null string */
    }

    /*
     *  Generate and display the title string, with page indicator
     *  if appropriate, preceded by the toolbar token if appropriate,
     *  and truncated if necessary. - FM & KW
     */
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
		tmp[j] = '\0';
	    }
	    StrAllocCopy(title, (CONST char *)tmp);
	    FREE(tmp);
	}
    }
    move(0, 0);
    clrtoeol();
    if (text->top_of_screen > 0 && HText_hasToolbar(text)) {
	addch('#');
    }
    i = (LYcols - 1) - strlen(percent) - strlen(title);
    if (i > 0) {
	move(0, i);
    } else {
	/*
	 *  Note that this truncation is not taking into
	 *  account the possibility that multibyte
	 *  characters might be present. - FM
	 */
	title[((LYcols - 2) - strlen(percent))] = '\0';
	move(0, 1);
    }
    addstr(title);
    if (percent[0] != '\0')
	addstr(percent);
    addch('\n');
    FREE(title);

#ifdef USE_COLOR_STYLE
/* turn the TITLE style off */
    LynxChangeStyle(s_title, ABS_OFF, 0);
#endif /* USE_COLOR_STYLE */
    lynx_stop_title_color ();

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
    char *cp, tmp[7];
    int last_screen;
    TextAnchor *Anchor_ptr = NULL;
    FormInfo *FormInfo_ptr;
    BOOL display_flag = FALSE;
    HTAnchor *link_dest;
    HTAnchor *link_dest_intl = NULL;
    static int last_nlinks = 0;
    static int charset_last_displayed = -1;

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
	addstr("\n\nError accessing document!\nNo data available!\n");
	refresh();
	nlinks = 0;  /* set number of links to 0 */
	return;
    }

    tmp[0] = tmp[1] = tmp[2] = '\0';
    text->page_has_target = NO;
    last_screen = text->Lines - (display_lines - 2);
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
	 i++, line = line->next) {			/* Loop */
#ifndef VMS
	if (!LYNoCore) {
	    assert(line->next != NULL);
	} else if (line->next == NULL) {
	    if (enable_scrollback) {
		addch('*');
		refresh();
		clear();
	    }
	    addstr("\n\nError drawing page!\nBad HText structure!\n");
	    refresh();
	    nlinks = 0;  /* set number of links to 0 */
	    return;
	}
#else
	assert(line->next != NULL);
#endif /* !VMS */
    }

    if (LYlowest_eightbit[current_char_set] <= 255 &&
	(current_char_set != charset_last_displayed) &&
	/*
	 *  current_char_set has changed since last invocation,
	 *  and it's not just 7-bit.
	 *  Also we don't want to do this for -dump and -source etc.
	 */
	LYCursesON) {
	charset_last_displayed = current_char_set;
#ifdef EXP_CHARTRANS_AUTOSWITCH
#ifdef LINUX
	/*
	 *  Currently implemented only for LINUX
	 */
	stop_curses();
	UCChangeTerminalCodepage(current_char_set,
				 &LYCharSet_UC[current_char_set]);
	start_curses();
#endif /* LINUX */
#endif /* EXP_CHARTRANS_AUTOSWITCH */
    }

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
     *  Output the page.
     */
    if (line) {
	char *data;
	int offset, HitOffset, LenNeeded;
	for (i = 0; i < (display_lines); i++)  {
	    /*
	     *  Verify and display each line.
	     */
#ifndef VMS
	    if (!LYNoCore) {
		assert(line != NULL);
	    } else if (line == NULL) {
		if (enable_scrollback) {
		    addch('*');
		    refresh();
		    clear();
		}
		addstr("\n\nError drawing page!\nBad HText structure!\n");
		refresh();
		nlinks = 0;  /* set number of links to 0 */
		return;
	    }
#else
	    assert(line != NULL);
#endif /* !VMS */
	    display_line(line, text);

#if defined(FANCY_CURSES) || defined(USE_SLANG)
	    /*
	     *  If the target is on this line, recursively
	     *  seek and emphasize it. - FM
	     */
	    data = (char *)line->data;
	    offset = (int)line->offset;
	    while ((target && *target) &&
		   (case_sensitive ?
		    (cp = LYno_attr_mbcs_strstr(data,
						target,
						text->T.output_utf8,
						&HitOffset,
						&LenNeeded)) != NULL :
		    (cp = LYno_attr_mbcs_case_strstr(data,
						     target,
						text->T.output_utf8,
						&HitOffset,
						&LenNeeded)) != NULL) &&
		   ((int)line->offset + LenNeeded) < LYcols) {
		int itmp = 0;
		int written = 0;
		int x_pos = offset + (int)(cp - data);
		int len = strlen(target);
		size_t utf_extra = 0;
		int y;

		text->page_has_target = YES;

		/*
		 *  Start the emphasis.
		 */
		LYstartTargetEmphasis();

		/*
		 *  Output the target characters.
		 */
		for (;
		     written < len && (tmp[0] = data[itmp]) != '\0';
		     itmp++)  {
		    if (IsSpecialAttrChar(tmp[0])) {
			/*
			 *  Ignore special characters.
			 */
			x_pos--;

		    } else if (cp == &data[itmp]) {
			/*
			 *  First printable character of target.
			 */
			move((i + 1), x_pos);
			if (text->T.output_utf8 && !isascii(tmp[0])) {
			    if ((*tmp & 0xe0) == 0xc0) {
				utf_extra = 1;
			    } else if ((*tmp & 0xf0) == 0xe0) {
				utf_extra = 2;
			    } else if ((*tmp & 0xf8) == 0xf0) {
				utf_extra = 3;
			    } else if ((*tmp & 0xfc) == 0xf8) {
				utf_extra = 4;
			    } else if ((*tmp & 0xfe) == 0xfc) {
				utf_extra = 5;
			    } else {
				/*
				 *  Garbage.
				 */
				utf_extra = 0;
			    }
			    if (strlen(&line->data[itmp+1]) < utf_extra) {
				/*
				 *  Shouldn't happen.
				 */
				utf_extra = 0;
			    }
			}
			if (utf_extra) {
			    strncpy(&tmp[1], &line->data[itmp+1], utf_extra);
			    tmp[utf_extra+1] = '\0';
			    itmp += utf_extra;
			    addstr(tmp);
			    tmp[1] = '\0';
			    written += (utf_extra + 1);
			    utf_extra = 0;
			} else if (HTCJK != NOCJK && !isascii(tmp[0])) {
			    /*
			     *  For CJK strings, by Masanobu Kimura.
			     */
			    tmp[1] = data[++itmp];
			    addstr(tmp);
			    tmp[1] = '\0';
			    written += 2;
			} else {
			    addstr(tmp);
			    written++;
			}

		    } else if (&data[itmp] > cp) {
			/*
			 *  Output all the other printable target chars.
			 */
			if (text->T.output_utf8 && !isascii(tmp[0])) {
			    if ((*tmp & 0xe0) == 0xc0) {
				utf_extra = 1;
			    } else if ((*tmp & 0xf0) == 0xe0) {
				utf_extra = 2;
			    } else if ((*tmp & 0xf8) == 0xf0) {
				utf_extra = 3;
			    } else if ((*tmp & 0xfc) == 0xf8) {
				utf_extra = 4;
			    } else if ((*tmp & 0xfe) == 0xfc) {
				utf_extra = 5;
			    } else {
				/*
				 *  Garbage.
				 */
				utf_extra = 0;
			    }
			    if (strlen(&line->data[itmp+1]) < utf_extra) {
				/*
				 *  Shouldn't happen.
				 */
				utf_extra = 0;
			    }
			}
			if (utf_extra) {
			    strncpy(&tmp[1], &line->data[itmp+1], utf_extra);
			    tmp[utf_extra+1] = '\0';
			    itmp += utf_extra;
			    addstr(tmp);
			    tmp[1] = '\0';
			    written += (utf_extra + 1);
			    utf_extra = 0;
			} else if (HTCJK != NOCJK && !isascii(tmp[0])) {
			    /*
			     *  For CJK strings, by Masanobu Kimura.
			     */
			    tmp[1] = data[++itmp];
			    addstr(tmp);
			    tmp[1] = '\0';
			    written += 2;
			} else {
			    addstr(tmp);
			    written++;
			}
		    }
		}

		/*
		 *  Stop the emphasis, and reset the offset and
		 *  data pointer for our current position in the
		 *  line. - FM
		 */
		LYstopTargetEmphasis();
		LYGetYX(y, offset);
		data = (char *)&data[itmp];

		/*
		 *  Adjust the cursor position, should we be at
		 *  the end of the line, or not have another hit
		 *  in it. - FM
		 */
		move((i + 2), 0);
	    }
#endif /* FANCY CURSES || USE_SLANG */

	    /*
	     *  Stop if this is the last line.  Otherwise, make sure
	     *  display_flag is set and process the next line. - FM
	     */
	    if (line == text->last_line) {
		/*
		 *  Clear remaining lines of display.
		 */
		for (i++; i < (display_lines); i++) {
		    move((i + 1), 0);
		    clrtoeol();
		}
		break;
	    }
	    display_flag = TRUE;
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
	    /*
	     *  Load normal hypertext anchors.
	     */
	    if (Anchor_ptr->show_anchor && Anchor_ptr->hightext &&
			strlen(Anchor_ptr->hightext) > 0 &&
			(Anchor_ptr->link_type & HYPERTEXT_ANCHOR)) {

		links[nlinks].hightext	= Anchor_ptr->hightext;
		links[nlinks].hightext2 = Anchor_ptr->hightext2;
		links[nlinks].hightext2_offset = Anchor_ptr->hightext2offset;
		links[nlinks].inUnderline = Anchor_ptr->inUnderline;

		links[nlinks].anchor_number = Anchor_ptr->number;
		links[nlinks].anchor_line_num = Anchor_ptr->line_num;

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
		    else {
#ifndef DONT_TRACK_INTERNAL_LINKS
			if (Anchor_ptr->link_type == INTERNAL_LINK_ANCHOR) {
			    link_dest_intl = HTAnchor_followTypedLink(
				(HTAnchor *)Anchor_ptr->anchor, LINK_INTERNAL);
			    if (link_dest_intl && link_dest_intl != link_dest) {

				CTRACE(tfp,
				    "display_page: unexpected typed link to %s!\n",
					    link_dest_intl->parent->address);
				link_dest_intl = NULL;
			    }
			} else
			    link_dest_intl = NULL;
			if (link_dest_intl) {
			    char *cp2 = HTAnchor_address(link_dest_intl);
			    cp_AnchorAddress = cp2;
			} else
#endif
			    cp_AnchorAddress = HTAnchor_address(link_dest);
		    }
		    FREE(links[nlinks].lname);

		    if (cp_AnchorAddress != NULL)
			links[nlinks].lname = cp_AnchorAddress;
		    else
			StrAllocCopy(links[nlinks].lname, empty_string);
		}

		links[nlinks].lx = Anchor_ptr->line_pos;
		links[nlinks].ly = ((Anchor_ptr->line_num + 1) - line_number);
		if (link_dest_intl)
		    links[nlinks].type = WWW_INTERN_LINK_TYPE;
		else
		    links[nlinks].type = WWW_LINK_TYPE;
		links[nlinks].target = empty_string;
		links[nlinks].form = NULL;

	        nlinks++;
		display_flag = TRUE;

	    } else if (Anchor_ptr->link_type == INPUT_ANCHOR
			&& Anchor_ptr->input_field->type != F_HIDDEN_TYPE) {
		/*
		 *  Handle form fields.
		 */
		lynx_mode = FORMS_LYNX_MODE;

		FormInfo_ptr = Anchor_ptr->input_field;

		links[nlinks].anchor_number = Anchor_ptr->number;
		links[nlinks].anchor_line_num = Anchor_ptr->line_num;

		links[nlinks].form = FormInfo_ptr;
		links[nlinks].lx = Anchor_ptr->line_pos;
		links[nlinks].ly = ((Anchor_ptr->line_num + 1) - line_number);
		links[nlinks].type = WWW_FORM_LINK_TYPE;
		links[nlinks].inUnderline = Anchor_ptr->inUnderline;
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

		/*
		 *  Never a second line on form types.
		 */
		links[nlinks].hightext2 = NULL;
		links[nlinks].hightext2_offset = 0;

		nlinks++;
	        /*
		 *  Bold the link after incrementing nlinks.
		 */
		highlight(OFF, (nlinks - 1), target);

		display_flag = TRUE;

	    } else {
		/*
		 *  Not showing anchor.
		 */
		if (Anchor_ptr->hightext && *Anchor_ptr->hightext)
		    CTRACE(tfp,
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
	    CTRACE(tfp, "\ndisplay_page: MAXLINKS reached.\n");
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

    if (!display_flag) {
	/*
	 *  Nothing on the page.
	 */
	addstr("\n     Document is empty");
    }

    if (HTCJK != NOCJK || text->T.output_utf8 || TRACE) {
	/*
	 *  For non-multibyte curses.
	 */
	lynx_force_repaint();
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
#if defined(USE_COLOR_STYLE)
    int inew;
#endif
    int indent = text->in_line_1 ?
	  text->style->indent1st : text->style->leftIndent;
    TextAnchor * a;
    int CurLine = text->Lines;
    int HeadTrim = 0;
    int SpecialAttrChars = 0;
    int TailTrim = 0;

    /*
     *  Make new line.
     */
    HTLine * previous = text->last_line;
    int ctrl_chars_on_previous_line = 0;
    char * cp;
    HTLine * line = (HTLine *)LY_CALLOC(1, LINE_SIZE(MAX_LINE));
    if (line == NULL)
	outofmem(__FILE__, "split_line_1");

    ctrl_chars_on_this_line = 0; /*reset since we are going to a new line*/
    text->LastChar = ' ';

    CTRACE(tfp,"GridText: split_line called\n");

    text->Lines++;

    previous->next->prev = line;
    line->prev = previous;
    line->next = previous->next;
#if defined(USE_COLOR_STYLE)
#define LastStyle (previous->numstyles-1)
    line->numstyles = 0;
    inew = MAX_STYLES_ON_LINE - 1;
    spare = previous->numstyles;
    while (previous->numstyles && inew >= 0) {
	if (previous->numstyles >= 2 &&
	    previous->styles[LastStyle].style
	    == previous->styles[previous->numstyles-2].style &&
	    previous->styles[LastStyle].horizpos
	    == previous->styles[previous->numstyles-2].horizpos &&
	    ((previous->styles[LastStyle].direction == STACK_OFF &&
	      previous->styles[previous->numstyles-2].direction == STACK_ON) ||
	     (previous->styles[LastStyle].direction == ABS_OFF &&
	      previous->styles[previous->numstyles-2].direction == ABS_ON) ||
	     (previous->styles[LastStyle].direction == ABS_ON &&
	      previous->styles[previous->numstyles-2].direction == ABS_OFF)
		)) {
	    /*
	     *  Discard pairs of ON/OFF for the same color style, but only
	     *  if they appear at the same position. - kw
	     */
	    previous->numstyles -= 2;
	    if (spare > previous->numstyles)
		spare = previous->numstyles;
	} else if (spare > 0 && previous->styles[spare - 1].direction &&
		   previous->numstyles < MAX_STYLES_ON_LINE) {
	    /*
	     *  The index variable spare walks backwards through the
	     *  list of color style changes on the previous line, trying
	     *  to find an ON change which isn't followed by a
	     *  corresponding OFF.  When it finds one, the missing OFF
	     *  change is appended to the end, and an ON change is added
	     *  at the beginning of the current line.  The OFF change
	     *  appended to the previous line may get removed again in
	     *  the next iteration. - kw
	     */
	    line->styles[inew].horizpos = 0;
	    line->styles[inew].direction = ON;
	    line->styles[inew].style = previous->styles[spare - 1].style;
	    inew --;
	    line->numstyles ++;
	    previous->styles[previous->numstyles].style = line->styles[inew + 1].style;

	    previous->styles[previous->numstyles].direction = ABS_OFF;
	    previous->styles[previous->numstyles].horizpos = previous->size;
	    previous->numstyles++;
	    spare --;
	} else if (spare >= 2 &&
		   previous->styles[spare - 1].style == previous->styles[spare - 2].style &&
		   ((previous->styles[spare - 1].direction == STACK_OFF &&
		     previous->styles[spare - 2].direction == STACK_ON) ||
		    (previous->styles[spare - 1].direction == ABS_OFF &&
		     previous->styles[spare - 2].direction == ABS_ON) ||
		    (previous->styles[spare - 1].direction == STACK_ON &&
		     previous->styles[spare - 2].direction == STACK_OFF) ||
		    (previous->styles[spare - 1].direction == ABS_ON &&
		     previous->styles[spare - 2].direction == ABS_OFF)
		       )) {
	       /*
		*  Skip pairs of adjacent ON/OFF or OFF/ON changes.
		*/
	    spare -= 2;
	} else if (spare && !previous->styles[spare - 1].direction) {
	    /*
	     *  Found an OFF change not part of a matched pair.
	     *  Assume it is safer to leave whatever comes before
	     *  it on the previous line alone.  Setting spare to 0
	     *  ensures that it won't be used in a following
	     *  iteration. - kw
	     */
	    spare = 0;
	} else {
	    /*
	     *  Nothing applied, so we are done with the loop. - kw
	     */
	    break;
	}
    }
    if (previous->numstyles > 0 && previous->styles[LastStyle].direction) {

	CTRACE(tfp, "%s\n%s%s\n",
		    "........... Too many character styles on line:",
		    "........... ", previous->data);
    }
    if (line->numstyles > 0 && line->numstyles < MAX_STYLES_ON_LINE) {
	int n;
	inew ++;
	for (n = line->numstyles; n >= 0; n--)
		line->styles[n + inew] = line->styles[n];
    } else
	if (line->numstyles == 0)
	/* FIXME: RJP - shouldn't use 0xffffffff for largest integer */
	line->styles[0].horizpos = 0xffffffff;
    if (previous->numstyles == 0)
	previous->styles[0].horizpos = 0xffffffff;
#endif
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
	while (*p == ' ' || *p == LY_SOFT_HYPHEN) {
	    p++;
	    HeadTrim++;
	}
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
		SpecialAttrChars++;
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
	    SpecialAttrChars++;;
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
		IS_UTF_EXTRA(p[i]) ||
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
	TailTrim++;
    }
    temp = (HTLine *)LY_CALLOC(1, LINE_SIZE(previous->size));
    if (temp == NULL)
	outofmem(__FILE__, "split_line_2");
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
	    IS_UTF_EXTRA(*cp) ||
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

    /*
     *  If we split the line, adjust the anchor
     *  structure values for the new line. - FM
     */
    if (split > 0) {
	for (a = text->first_anchor; a; a = a->next) {
	    if (a->line_num == CurLine) {
		if (a->line_pos >= split) {
		    a->start += (1 + SpecialAttrChars - HeadTrim - TailTrim);
		    a->line_pos -= (split - SpecialAttrChars + HeadTrim);
		    a->line_num = text->Lines;
		} else if ((a->link_type & HYPERTEXT_ANCHOR) &&
			   (a->line_pos + a->extent) >= split) {
		    a->extent -= (TailTrim + HeadTrim);
		    if (a->extent < 0) {
		        a->extent = 0;
		    }
		}
	    }
	}
    }
} /* split_line */


/*	Allow vertical blank space
**	--------------------------
*/
PRIVATE void blank_lines ARGS2(
	HText *,	text,
	int,		newlines)
{
    BOOL IgnoreSpaces = FALSE;

    if (!HText_LastLineSize(text, IgnoreSpaces)) { /* No text on current line */
	HTLine * line = text->last_line->prev;
	while ((line != text->last_line) &&
	       (HText_TrueLineSize(line, text, IgnoreSpaces) == 0)) {
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
	return; 			/* Safety */
    after = text->style->spaceAfter;
    before = style->spaceBefore;

    CTRACE(tfp, "GridText: Change to style %s\n", style->name);

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

    if (text->halted > 1) {
	/*
	 *  We should stop outputting more text, because low memory was
	 *  detected.  - kw
	 */
	if (text->halted == 2) {
	    /*
	     *  But if we haven't done so yet, first append a warning.
	     *  We should still have a few bytes left for that :).
	     *  We temporarily reset test->halted to 0 for this, since
	     *  this function will get called recursively. - kw
	     */
	    text->halted = 0;
	    text->kanji_buf = '\0';
	    HText_appendText(text, " *** MEMORY EXHAUSTED ***");
	}
	text->halted = 3;
	return;
    }
    /*
     *  Make sure we don't hang on escape sequences.
     */
    if (ch == '\033' && HTCJK == NOCJK)			/* decimal 27 */
	return;
#ifndef USE_SLANG
    /*
     *  Block 8-bit chars not allowed by the current display character
     *  set if they are below what LYlowest_eightbit indicates.
     *  Slang used its own replacements, so for USE_SLANG blocking here
     *  is not necessary to protect terminals from those characters.
     *  They should have been filtered out or translated by an earlier
     *  processing stage anyway. - kw
     */
    if ((unsigned char)ch >= 128 && HTCJK == NOCJK &&
	!text->T.transp && !text->T.output_utf8 &&
	(unsigned char)ch < LYlowest_eightbit[current_char_set])
	return;
#endif /* USE_SLANG */
    if ((unsigned char)ch == 155 && HTCJK == NOCJK) {	/* octal 233 */
	if (!HTPassHighCtrlRaw &&
	    !text->T.transp && !text->T.output_utf8 &&
	    (155 < LYlowest_eightbit[current_char_set]) &&
	    strncmp(LYchar_set_names[current_char_set],
		    "DosLatin1 (cp850)", 17) &&
	    strncmp(LYchar_set_names[current_char_set],
		    "DosLatinUS (cp437)", 18) &&
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
		    /*
		    **  Setting up for CJK escape sequence handling (based on
		    **  Takuya ASADA's (asada@three-a.co.jp) CJK Lynx). - FM
		    */
		    text->state = S_esc;
		    text->kanji_buf = '\0';
		    return;
		}
		break;

		case S_esc:
		/*
		 *  Expecting '$'or '(' following CJK ESC.
		 */
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
		/*
		 *  Expecting '@', 'B', 'A' or '(' after CJK "ESC$".
		 */
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
		/*
		 * Expecting 'C' after CJK "ESC$(".
		 */
		if (ch == 'C') {
		    text->state = S_nonascii_text;
		    return;
		} else {
		    text->state = S_text;
		}
		break;

		case S_paren:
		/*
		 *  Expecting 'B', 'J', 'T' or 'I' after CJK "ESC(".
		 */
		if (ch == 'B' || ch == 'J' || ch == 'T')  {
		    /*
		     *  Can split here. - FM
		     */
		    text->permissible_split = (int)text->last_line->size;
		    text->state = S_text;
		    return;
		} else if (ch == 'I')  {
		    text->state = S_jisx0201_text;
		    /*
		     *  Can split here. - FM
		     */
		    text->permissible_split = (int)text->last_line->size;
		    return;
		} else {
		    text->state = S_text;
		}
		break;

		case S_nonascii_text:
		/*
		 *  Expecting CJK ESC after non-ASCII text.
		 */
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
		    text->kanji_buf = '\216';
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
		    /*
		     *  Can split here. - FM
		     */
		    text->permissible_split = (int)text->last_line->size;
		    return;
	        }
	    }
	} else {
	    goto check_IgnoreExcess;
	}
    } else if (ch == '\033') {
	return;
    }

    if (IsSpecialAttrChar(ch)) {
#ifndef USE_COLOR_STYLE
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
	} else if (ch == LY_SOFT_HYPHEN) {
	    int i;

	    /*
	     *  Ignore the soft hyphen if it is the first character
	     *  on the line, or if it is preceded by a space or
	     *  hyphen. - FM
	     */
	    if (line->size < 1 || text->permissible_split >= (int)line->size)
		return;

	    for (i = (text->permissible_split + 1); line->data[i]; i++) {
		if (!IsSpecialAttrChar((unsigned char)line->data[i]) &&
		    !isspace((unsigned char)line->data[i]) &&
		    (unsigned char)line->data[i] != '-' &&
		    (unsigned char)line->data[i] != HT_NON_BREAK_SPACE &&
		    (unsigned char)line->data[i] != HT_EM_SPACE) {
		    break;
		}
	    }
	    if (line->data[i] == '\0') {
		return;
	    }
	}
#else
	return;
#endif
    }

    if (IS_UTF_EXTRA(ch)) {
	line->data[line->size++] = ch;
	line->data[line->size] = '\0';
	ctrl_chars_on_this_line++;
	return;
    }

    /*
     *  New Line.
     */
    if (ch == '\n') {
	    new_line(text);
	    text->in_line_1 = YES;	/* First line of new paragraph */
	    /*
	     *  There are some pages written in
	     *  different kanji codes. - TA & kw
	     */
	    if (HTCJK == JAPANESE)
		text->kcode = NOKANJI;
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
	/*
	 *  There are some pages written in
	 *  different kanji codes. - TA & kw
	 */
	if (HTCJK == JAPANESE)
	    text->kcode = NOKANJI;
	return;
    }


    /*
     *  Tabs.
     */
    if (ch == '\t') {
	CONST HTTabStop * Tab;
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
	    /*
	     *  Can split here. - FM
	     */
	    text->permissible_split = (int)line->size;
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
	/*
	 *  Can split here. - FM
	 */
	text->permissible_split = (int)text->last_line->size;
	/*
	 *  There are some pages written in
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
	 ((line->size > 0) &&
	  (int)(line->data[line->size-1] ==
				LY_SOFT_HYPHEN ?
					     1 : 0))) >= (LYcols - 1)) {

	if (style->wordWrap && HTOutputFormat != WWW_SOURCE) {
	    split_line(text, text->permissible_split);
	    if (ch == ' ') return;	/* Ignore space causing split */

	}  else if (HTOutputFormat == WWW_SOURCE) {
		 /*
		  *  For source output we don't want to wrap this stuff
		  *  unless absolutely necessary. - LJM
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
    } else if ((int)line->size >= (int)(MAX_LINE-1)) {
	/*
	 *  Never overrun memory if LYcols is set to a large value - KW
	 */
	new_line(text);
    }

    /*
     *  Insert normal characters.
     */
    if (ch == HT_NON_BREAK_SPACE) {
	ch = ' ';
    }

    if (ch & 0x80)
	text->have_8bit_chars = YES;

    {
	HTFont font = style->font;
	unsigned char hi, lo, tmp[2];

	line = text->last_line; /* May have changed */
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

	if (ch == LY_SOFT_HYPHEN) {
	    ctrl_chars_on_this_line++;
	    /*
	     *  Can split here. - FM
	     */
	    text->permissible_split = (int)text->last_line->size;
	}
    }
}

#ifdef USE_COLOR_STYLE
/*  Insert a style change into the current line
**  -------------------------------------------
*/
PUBLIC void _internal_HTC ARGS3(HText *,text, int,style, int,dir)
{
 HTLine* line;
 static int last_style = -1;
 static int last_dir = -1;

 /* can't change style if we have no text to change style with */
 if (!text) return;

 line = text->last_line;

 if ((style != last_style || dir != last_dir) && line->numstyles < MAX_STYLES_ON_LINE)
 {
      line->styles[line->numstyles].horizpos = line->size;
      line->styles[line->numstyles].style = style;
      line->styles[line->numstyles].direction = dir;
      line->numstyles++;
 }
 last_style = style;
 last_dir = dir;
}
#endif



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
PUBLIC int HText_beginAnchor ARGS3(
	HText *,		text,
	BOOL,			underline,
	HTChildAnchor *,	anc)
{
    char marker[16];

    TextAnchor * a = (TextAnchor *) calloc(1, sizeof(*a));

    if (a == NULL)
	outofmem(__FILE__, "HText_beginAnchor");
    a->hightext  = NULL;
    a->hightext2 = NULL;
    a->start = text->chars + text->last_line->size;
    a->inUnderline = underline;

    a->line_num = text->Lines;
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

#ifndef DONT_TRACK_INTERNAL_LINKS
    if (HTAnchor_followTypedLink((HTAnchor*)anc, LINK_INTERNAL)) {
	a->number = ++(text->last_anchor_number);
	a->link_type = INTERNAL_LINK_ANCHOR;
    } else
#endif
	if (HTAnchor_followMainLink((HTAnchor*)anc)) {
	a->number = ++(text->last_anchor_number);
    } else {
	a->number = 0;
    }

    /*
     *  If we are doing link_numbering add the link number.
     */
    if ((a->number > 0) &&
	(keypad_mode == LINKS_ARE_NUMBERED ||
	 keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED)) {
	sprintf(marker,"[%d]", a->number);
	HText_appendText(text, marker);
	a->start = text->chars + text->last_line->size;
	a->line_num = text->Lines;
	a->line_pos = text->last_line->size;
    }

    return(a->number);
}


PUBLIC void HText_endAnchor ARGS2(
	HText *,	text,
	int,		number)
{
    TextAnchor *a;

    /*
     *  The number argument is set to 0 in HTML.c and
     *  LYCharUtils.c when we want to end the anchor
     *  for the immediately preceding HText_beginAnchor()
     *  call.  If it's greater than 0, we want to handle
     *  a particular anchor.  This allows us to set links
     *  for positions indicated by NAME or ID attributes,
     *  without needing to close any anchor with an HREF
     *  within which that link might be embedded. - FM
     */
    if (number <= 0 || number == text->last_anchor->number) {
	a = text->last_anchor;
    } else {
	for (a = text->first_anchor; a; a = a->next) {
	    if (a->number == number) {
	        break;
	    }
	}
	if (a == NULL) {
	    /*
	     *  There's no anchor with that number,
	     *  so we'll default to the last anchor,
	     *  and cross our fingers. - FM
	     */
	    a = text->last_anchor;
	}
    }

    CTRACE(tfp, "HText_endAnchor: number:%d link_type:%d\n",
			a->number, a->link_type);
    if (a->link_type == INPUT_ANCHOR) {
	/*
	 *  Shouldn't happen, but put test here anyway to be safe. - LE
	 */

	CTRACE(tfp,
	   "HText_endAnchor: internal error: last anchor was input field!\n");
	return;
    }
    if (a->number) {
	/*
	 *  If it goes somewhere...
	 */
	int i, j, k, l;
	BOOL remove_numbers_on_empty =
	    ((keypad_mode == LINKS_ARE_NUMBERED ||
	      keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED) &&
	     (text->hiddenlinkflag != HIDDENLINKS_MERGE ||
	      (LYNoISMAPifUSEMAP &&
	       HTAnchor_isISMAPScript(
		   HTAnchor_followMainLink((HTAnchor *)a->anchor)))));
	HTLine *last = text->last_line;
	HTLine *prev = text->last_line->prev;
	int CurBlankExtent = 0;
	int BlankExtent = 0;

	/*
	 *  Check if the anchor content has only
	 *  white and special characters, starting
	 *  with the content on the last line. - FM
	 */
	a->extent += (text->chars + last->size) - a->start -
		     (text->Lines - a->line_num);
	if (a->extent > last->size) {
	    /*
	     *  The anchor extends over more than one line,
	     *  so set up to check the entire last line. - FM
	     */
	    i = last->size;
	} else {
	    /*
	     *  The anchor is restricted to the last line,
	     *  so check from the start of the anchor. - FM
	     */
	    i = a->extent;
	}
	j = (last->size - i);
	while (j < last->size) {
	    if (!IsSpecialAttrChar(last->data[j]) &&
	        !isspace((unsigned char)last->data[j]) &&
		last->data[j] != HT_NON_BREAK_SPACE &&
		last->data[j] != HT_EM_SPACE)
		break;
	    i--;
	    j++;
	}
	if (i == 0) {
	    if (a->extent > last->size) {
		/*
		 *  The anchor starts on a preceding line, and
		 *  the last line has only white and special
		 *  characters, so declare the entire extent
		 *  of the last line as blank. - FM
		 */
		CurBlankExtent = BlankExtent = last->size;
	    } else {
		/*
		 *  The anchor starts on the last line, and
		 *  has only white or special characters, so
		 *  declare the anchor's extent as blank. - FM
		 */
		CurBlankExtent = BlankExtent = a->extent;
	    }
	}
	/*
	 *  While the anchor starts on a line preceding
	 *  the one we just checked, and the one we just
	 *  checked has only white and special characters,
	 *  check whether the anchor's content on the
	 *  immediately preceding line also has only
	 *  white and special characters. - FM
	 */
	while (i == 0 && a->extent > CurBlankExtent) {
	    j = prev->size - a->extent + CurBlankExtent;
	    if (j < 0) {
	        /*
		 *  The anchor starts on a preceding line,
		 *  so check all of this line. - FM
		 */
	        j = 0;
		i = prev->size;
	    } else {
	        /*
		 *  The anchor starts on this line. - FM
		 */
	        i = a->extent - CurBlankExtent;
	    }
	    while (j < prev->size) {
	        if (!IsSpecialAttrChar(prev->data[j]) &&
		    !isspace((unsigned char)prev->data[j]) &&
		    prev->data[j] != HT_NON_BREAK_SPACE &&
		    prev->data[j] != HT_EM_SPACE)
		    break;
		i--;
		j++;
	    }
	    if (i == 0) {
		if (a->extent > (CurBlankExtent + prev->size)) {
		    /*
		     *  This line has only white and special
		     *  characters, so treat its entire extent
		     *  as blank, and decrement the pointer for
		     *  the line to be analyzed. - FM
		     */
		    CurBlankExtent += prev->size;
		    BlankExtent = CurBlankExtent;
		    prev = prev->prev;
		} else {
		    /*
		     *  The anchor starts on this line, and it
		     *  has only white or special characters, so
		     *  declare the anchor's extent as blank. - FM
		     */
		    BlankExtent = a->extent;
		    break;
		}
	    }
	}
	if (i == 0) {
	    /*
	     *  It's an invisible anchor probably from an ALT=""
	     *  or an ignored ISMAP attribute due to a companion
	     *  USEMAP. - FM
	     */
	    a->show_anchor = NO;

	    /*
	     *  If links are numbered, then try to get rid of the
	     *  numbered bracket and adjust the anchor count. - FM
	     *
	     * Well, let's do this only if -hiddenlinks=merged is not in
	     * effect, or if we can be reasonably sure that
	     * this is the result of an intentional non-generation of
	     * anchor text via NO_ISMAP_IF_USEMAP. In other cases it can
	     * actually be a feature that numbered links alert the viewer
	     * to the presence of a link which is otherwise not selectable -
	     * possibly caused by HTML errors. - kw
	     */
	    if (remove_numbers_on_empty) {
		HTLine *start;
		int NumSize = 0;
		TextAnchor *anc;

		/*
		 *  Set start->data[j] to the close-square-bracket,
		 *  or to the beginning of the line on which the
		 *  anchor start. - FM
		 */
		if (prev == last->prev) {
		    /*
		     *  The anchor starts on the last line. - FM
		     */
		    start = last;
		    j = (last->size - a->extent - 1);
		} else {
		    /*
		     *  The anchor starts on a previous line. - FM
		     */
		    start = prev;
		    prev = prev->prev;
		    j = (start->size - a->extent + CurBlankExtent - 1);
		}
		if (j < 0)
		    j = 0;
		i = j;

		/*
		 *  If start->data[j] is a close-square-bracket, verify
		 *  that it's the end of the numbered bracket, and if so,
		 *  strip the numbered bracket.  If start->data[j] is not
		 *  a close-square-bracket, check whether we had a wrap
		 *  and the close-square-bracket is at the end of the
		 *  previous line.  If so, strip the numbered bracket
		 *  from that line. - FM
		 */
		if (start->data[j] == ']') {
		    j--;
		    NumSize++;
		    while (j >= 0 && isdigit((unsigned char)start->data[j])) {
		        j--;
			NumSize++;
		    }
		    while (j < 0) {
		        j++;
			NumSize--;
		    }
		    if (start->data[j] == '[') {
		        /*
			 *  The numbered bracket is entirely
			 *  on this line. - FM
			 */
			NumSize++;
			k = j + NumSize;
			while (k < start->size)
			    start->data[j++] = start->data[k++];
			if (start != last)
			    text->chars -= NumSize;
			for (anc = a; anc; anc = anc->next) {
			    anc->start -= NumSize;
			    anc->line_pos -= NumSize;
			}
		        start->size = j;
			start->data[j++] = '\0';
			while (j < k)
			     start->data[j++] = '\0';
		    } else if (prev && prev->size > 1) {
			k = (i + 1);
			j = (prev->size - 1);
			while ((j >= 0) &&
			       (prev->data[j] == LY_BOLD_START_CHAR ||
			        prev->data[j] == LY_BOLD_END_CHAR ||
				prev->data[j] == LY_UNDERLINE_START_CHAR ||
			        prev->data[j] == LY_UNDERLINE_END_CHAR ||
				prev->data[j] == LY_SOFT_HYPHEN))
			    j--;
		        i = (j + 1);
			while (j >= 0 &&
			       isdigit((unsigned char)prev->data[j])) {
			    j--;
			    NumSize++;
			}
			while (j < 0) {
			    j++;
			    NumSize--;
			}
			if (prev->data[j] == '[') {
			    /*
			     *  The numbered bracket started on the
			     *  previous line, and part of it was
			     *  wrapped to this line. - FM
			     */
			    NumSize++;
			    l = (i - j);
			    while (i < prev->size)
				start->data[j++] = start->data[i++];
			    text->chars -= l;
			    for (anc = a; anc; anc = anc->next) {
				anc->start -= l;
			    }
			    prev->size = j;
			    prev->data[j] = '\0';
			    while (j < i)
				start->data[j++] = '\0';
			    j = 0;
			    i = k;
			    while (k < start->size)
			        start->data[j++] = start->data[k++];
			    if (start != last)
			        text->chars -= i;
			    for (anc = a; anc; anc = anc->next) {
				anc->start -= i;
				anc->line_pos -= i;
			    }
			    start->size = j;
			    start->data[j++] = '\0';
			    while (j < k)
			        start->data[j++] = '\0';
			} else {
			    /*
			     *  Shucks!  We didn't find the
			     *  numbered bracket. - FM
			     */
			    a->show_anchor = YES;
			}
		    } else {
			/*
			 *  Shucks!  We didn't find the
			 *  numbered bracket. - FM
			 */
			a->show_anchor = YES;
		    }
		} else if (prev && prev->size > 2) {
		    j = (prev->size - 1);
		    while ((j >= 0) &&
			   (prev->data[j] == LY_BOLD_START_CHAR ||
			    prev->data[j] == LY_BOLD_END_CHAR ||
			    prev->data[j] == LY_UNDERLINE_START_CHAR ||
			    prev->data[j] == LY_UNDERLINE_END_CHAR ||
			    prev->data[j] == LY_SOFT_HYPHEN))
		        j--;
		    if (j < 0)
		        j = 0;
		    i = (j + 1);
		    if ((j > 2) &&
		        (prev->data[j] == ']' &&
			 isdigit((unsigned char)prev->data[j - 1]))) {
		        j--;
			NumSize++;
		        k = (j + 1);
			while (j >= 0 &&
			       isdigit((unsigned char)prev->data[j])) {
			    j--;
			    NumSize++;
			}
			while (j < 0) {
			    j++;
			    NumSize--;
			}
			if (prev->data[j] == '[') {
			    /*
			     *  The numbered bracket is all on the
			     *  previous line, and the anchor content
			     *  was wrapped to the last line. - FM
			     */
			    NumSize++;
			    k = j + NumSize;
			    while (k < prev->size)
				prev->data[j++] = prev->data[k++];
			    text->chars -= NumSize;
			    for (anc = a; anc; anc = anc->next) {
				anc->start -= NumSize;
			    }
			    prev->size = j;
			    prev->data[j++] = '\0';
			    while (j < k)
				start->data[j++] = '\0';
			} else {
			    /*
			     *  Shucks!  We didn't find the
			     *  numbered bracket. - FM
			     */
			    a->show_anchor = YES;
			}
		    } else {
			/*
			 *  Shucks!  We didn't find the
			 *  numbered bracket. - FM
			 */
		        a->show_anchor = YES;
		    }
		} else {
		    /*
		     *  Shucks!  We didn't find the
		     *  numbered bracket. - FM
		     */
		    a->show_anchor = YES;
		}
	    }
	} else {
	    /*
	     *  The anchor's content is not restricted to only
	     *  white and special characters, so we'll show it
	     *  as a link. - FM
	     */
	    a->show_anchor = YES;
	}
	if (a->show_anchor == NO) {
	    /*
	     *  The anchor's content is restricted to white
	     *  and special characters, so set it's number
	     *  and extent to zero, decrement the visible
	     *  anchor number counter, and add this anchor
	     *  to the hidden links list. - FM
	     */
	    a->extent = 0;
	    if (text->hiddenlinkflag != HIDDENLINKS_MERGE) {
		a->number = 0;
	        text->last_anchor_number--;
		HText_AddHiddenLink(text, a);
	    }
	} else {
	    /*
	     *  The anchor's content is not restricted to white
	     *  and special characters, so we'll display the
	     *  content, but shorten it's extent by any trailing
	     *  blank lines we've detected. - FM
	     */
	    a->extent -= ((BlankExtent < a->extent) ?
					BlankExtent : 0);
	}
    } else {
	/*
	 *  It's a named anchor without an HREF, so it
	 *  should be registered but not shown as a
	 *  link. - FM
	 */
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

    if (text->halted == 3)
	return;

    for (p = str; *p; p++) {
	HText_appendCharacter(text, *p);
    }
}


PRIVATE void remove_special_attr_chars ARGS1(
	char *,		buf)
{
    register char *cp;

    for (cp = buf; *cp != '\0' ; cp++) {
	/*
	 *  Don't print underline chars.
	 */
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
**  offset to each of the anchors.
*/
PUBLIC void HText_endAppend ARGS1(
	HText *,	text)
{
    int cur_line, cur_char, cur_shift;
    TextAnchor *anchor_ptr;
    HTLine *line_ptr;
    unsigned char ch;

    if (!text)
	return;

    CTRACE(tfp,"Gridtext: Entering HText_endAppend\n");

    /*
     *  Create a  blank line at the bottom.
     */
    new_line(text);

    if (text->halted) {
	/*
	 *  If output was stopped because memory was low, and we made
	 *  it to the end of the document, reset those flags and hope
	 *  things are better now. - kw
	 */
	LYFakeZap(NO);
	text->halted = 0;
    }

    /*
     *  Get the first line.
     */
    line_ptr = text->last_line->next;
    cur_char = line_ptr->size;
    cur_line = 0;
    cur_shift = 0;

    /*
     *  Remove the blank lines at the end of document.
     */
    while (text->last_line->data[0] == '\0' && text->Lines > 2) {
	HTLine *next_to_the_last_line = text->last_line->prev;


	CTRACE(tfp, "GridText: Removing bottom blank line: %s\n",
			    text->last_line->data);
	/*
	 *  line_ptr points to the first line.
	 */
	next_to_the_last_line->next = line_ptr;
	line_ptr->prev = next_to_the_last_line;
	FREE(text->last_line);
	text->last_line = next_to_the_last_line;
	text->Lines--;
#ifdef NOTUSED_BAD_FOR_SCREEN
	CTRACE(tfp, "GridText: New bottom line: %s\n",
			    text->last_line->data);
#endif
    }

    /*
     *  Fix up the anchor structure values and
     *  create the hightext strings. - FM
     */
    for (anchor_ptr = text->first_anchor;
	 anchor_ptr; anchor_ptr=anchor_ptr->next) {
re_parse:
	/*
	 *  Find the right line.
	 */
	for (; anchor_ptr->start >= cur_char;
	       line_ptr = line_ptr->next,
	       cur_char += line_ptr->size+1,
	       cur_line++) {
	    ; /* null body */
	}
	if (anchor_ptr->start == cur_char) {
	    anchor_ptr->line_pos = line_ptr->size;
	} else {
	    anchor_ptr->line_pos = anchor_ptr->start -
				   (cur_char - line_ptr->size);
	}
	if (anchor_ptr->line_pos < 0)
	    anchor_ptr->line_pos = 0;

	CTRACE(tfp, "Gridtext: Anchor found on line:%d col:%d\n",
			    cur_line, anchor_ptr->line_pos);

	/*
	 *  Strip off any spaces or SpecialAttrChars at the beginning,
	 *  if they exist, but only on HYPERTEXT_ANCHORS.
	 */
	if (anchor_ptr->link_type & HYPERTEXT_ANCHOR) {
	    ch = (unsigned char)line_ptr->data[anchor_ptr->line_pos];
	    while (isspace(ch) ||
	           IsSpecialAttrChar(ch)) {
		anchor_ptr->line_pos++;
		anchor_ptr->extent--;
		cur_shift++;
		ch = (unsigned char)line_ptr->data[anchor_ptr->line_pos];
	    }
	}
	if (anchor_ptr->extent < 0) {
	    anchor_ptr->extent = 0;
	}
#ifdef NOTUSED_BAD_FOR_SCREEN
	CTRACE(tfp, "anchor text: '%s'   pos: %d\n",
			    line_ptr->data, anchor_ptr->line_pos);
#endif
	/*
	 *  If the link begins with an end of line and we have more
	 *  lines, then start the highlighting on the next line. - FM
	 */
	if (anchor_ptr->line_pos >= strlen(line_ptr->data) &&
	    cur_line < text->Lines) {
	    anchor_ptr->start += (cur_shift + 1);
	    cur_shift = 0;
	    CTRACE(tfp, "found anchor at end of line\n");
	    goto re_parse;
	}
	cur_shift = 0;
#ifdef NOTUSED_BAD_FOR_SCREEN
	CTRACE(tfp, "anchor text: '%s'   pos: %d\n",
			    line_ptr->data, anchor_ptr->line_pos);
#endif
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
	    /*
	     *  Double check that we have a line pointer,
	     *  and if so, copy into hightext2.
	     */
	    if (line_ptr2) {
		StrnAllocCopy(anchor_ptr->hightext2,
			      line_ptr2->data,
			      (anchor_ptr->extent -
			       strlen(anchor_ptr->hightext)));
	        anchor_ptr->hightext2offset = line_ptr2->offset;
		remove_special_attr_chars(anchor_ptr->hightext2);
		if (anchor_ptr->link_type & HYPERTEXT_ANCHOR) {
		    LYTrimTrailing(anchor_ptr->hightext2);
		    if (anchor_ptr->hightext2[0] == '\0') {
			FREE(anchor_ptr->hightext2);
			anchor_ptr->hightext2offset = 0;
		    }
		}
	    }
	}
	remove_special_attr_chars(anchor_ptr->hightext);
	if (anchor_ptr->link_type & HYPERTEXT_ANCHOR) {
	    LYTrimTrailing(anchor_ptr->hightext);
	}

	/*
	 *  Subtract any formatting characters from the x position
	 *  of the link.
	 */
	if (anchor_ptr->line_pos > 0) {
	    register int offset = 0, i = 0;
	    for (; i < anchor_ptr->line_pos; i++)
		if (IS_UTF_EXTRA(line_ptr->data[i]) ||
		    IsSpecialAttrChar(line_ptr->data[i]))
		    offset++;
	    anchor_ptr->line_pos -= offset;
	}

	/*
	 *  Add the offset, and set the line number.
	 */
	anchor_ptr->line_pos += line_ptr->offset;
	anchor_ptr->line_num  = cur_line;

	CTRACE(tfp, "GridText: adding link on line %d in HText_endAppend\n",
		    cur_line);

	/*
	 *  If this is the last anchor, we're done!
	 */
	if (anchor_ptr == text->last_anchor)
	    break;
    }
}


/*	Dump diagnostics to tfp
*/
PUBLIC void HText_dump ARGS1(
	HText *,	text GCC_UNUSED)
{
    fprintf(tfp, "HText: Dump called\n");
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
/*
 *  HTChildAnchor() returns the anchor with index N.
 *  The index corresponds to the [number] we print for the anchor.
 */
PUBLIC HTChildAnchor * HText_childNumber ARGS1(
	int,		number)
{
    TextAnchor * a;

    if (!(HTMainText && HTMainText->first_anchor) || number <= 0)
	return (HTChildAnchor *)0;	/* Fail */

    for (a = HTMainText->first_anchor; a; a = a->next) {
	if (a->number == number)
	    return(a->anchor);
    }
    return (HTChildAnchor *)0;	/* Fail */
}

/*
 *  HText_FormDescNumber() returns a description of the form field
 *  with index N.  The index corresponds to the [number] we print
 *  for the field. - FM & LE
 */
PUBLIC void HText_FormDescNumber ARGS2(
	int,		number,
	char **,	desc)
{
    TextAnchor * a;

    if (!desc)
	return;

    if (!(HTMainText && HTMainText->first_anchor) || number <= 0) {
	 *desc = "unknown field or link";
	 return;
    }

    for (a = HTMainText->first_anchor; a; a = a->next) {
	if (a->number == number) {
	    if (!(a->input_field && a->input_field->type)) {
	        *desc = "unknown field or link";
		return;
	    }
	    break;
	}
    }

    switch (a->input_field->type) {
	case F_TEXT_TYPE:
	    *desc = "text entry field";
	    return;
	case F_PASSWORD_TYPE:
	    *desc = "password entry field";
	    return;
	case F_CHECKBOX_TYPE:
	    *desc = "checkbox";
	    return;
	case F_RADIO_TYPE:
	    *desc = "radio button";
	    return;
	case F_SUBMIT_TYPE:
	    *desc = "submit button";
	    return;
	case F_RESET_TYPE:
	    *desc = "reset button";
	    return;
	case F_OPTION_LIST_TYPE:
	    *desc = "popup menu";
	    return;
	case F_HIDDEN_TYPE:
	    *desc = "hidden form field";
	    return;
	case F_TEXTAREA_TYPE:
	    *desc = "text entry area";
	    return;
	case F_RANGE_TYPE:
	    *desc = "range entry field";
	    return;
	case F_FILE_TYPE:
	    *desc = "file entry field";
	    return;
	case F_TEXT_SUBMIT_TYPE:
	    *desc = "text-submit field";
	    return;
	case F_IMAGE_SUBMIT_TYPE:
	    *desc = "image-submit button";
	    return;
	case F_KEYGEN_TYPE:
	    *desc = "keygen field";
	    return;
	default:
	    *desc = "unknown form field";
	    return;
    }
}

/*
 *  HTGetLinkInfo returns some link info based on the number.
 *
 *  If want_go is not NULL, caller requests to know a line number for
 *  the link indicated by number.  It will be returned in *go_line, and
 *  *linknum will be set to an index into the links[] array, to use after
 *  the line in *line has been made the new top screen line.
 *  *hightext and *lname are unchanged. - KW
 *
 *  If want_go is 0 and the number doesn't represent an input field, info
 *  on the link indicated by number is deposited in *hightext and *lname.
 */
PUBLIC int HTGetLinkInfo ARGS6(
	int,		number,
	int,		want_go,
	int *,		go_line,
	int *,		linknum,
	char **,	hightext,
	char **,	lname)
{
    TextAnchor *a;
    HTAnchor *link_dest;
#ifndef DONT_TRACK_INTERNAL_LINKS
    HTAnchor *link_dest_intl = NULL;
#endif
    int anchors_this_line = 0, anchors_this_screen = 0;
    int prev_anchor_line = -1, prev_prev_anchor_line = -1;

    if (!HTMainText)
	return(NO);

    for (a = HTMainText->first_anchor; a; a = a->next) {
	/*
	 *  Count anchors, first on current line if there is more
	 *  than one.  We have to count all links, including form
	 *  field anchors and others with a->number == 0, because
	 *  they are or will be included in the links[] array.
	 *  The exceptions are hidden form fields and anchors with
	 *  show_anchor not set, because they won't appear in links[]
	 *  and don't count towards nlinks. - KW
	 */
	if ((a->show_anchor) &&
	    (a->link_type != INPUT_ANCHOR ||
	     a->input_field->type != F_HIDDEN_TYPE)) {
	    if (a->line_num == prev_anchor_line) {
		anchors_this_line++;
	    } else {
		/*
		 *  This anchor is on a different line than the previous one.
		 *  Remember which was the line number of the previous anchor,
		 *  for use in screen positioning later. - KW
		 */
		anchors_this_line = 1;
		prev_prev_anchor_line = prev_anchor_line;
		prev_anchor_line = a->line_num;
	    }
	    if (a->line_num >= HTMainText->top_of_screen) {
		/*
		 *  Count all anchors starting with the top line of the
		 *  currently displayed screen.  Just keep on counting
		 *  beyond this screen's bottom line - we'll know whether
		 *  a found anchor is below the current screen by a check
		 *  against nlinks later. - KW
		 */
		anchors_this_screen++;
	    }
	}

	if (a->number == number) {
	    /*
	     *  We found it.  Now process it, depending
	     *  on what kind of info is requested. - KW
	     */
	    if (want_go || a->link_type == INPUT_ANCHOR) {
		if (a->show_anchor == NO) {
		    /*
		     *  The number requested has been assigned to an anchor
		     *  without any selectable text, so we cannot position
		     *  on it.  The code for suppressing such anchors in
		     *  HText_endAnchor() may not have applied, or it may
		     *  have failed.  Return a failure indication so that
		     *  the user will notice that something is wrong,
		     *  instead of positioning on some other anchor which
		     *  might result in inadvertent activation. - KW
		     */
		    return(NO);
		}
	        if (anchors_this_screen > 0 &&
		    anchors_this_screen <= nlinks &&
		    a->line_num >= HTMainText->top_of_screen &&
		    a->line_num < HTMainText->top_of_screen+(display_lines)) {
		    /*
		     *  If the requested anchor is within the current screen,
		     *  just set *go_line so that the screen window won't move
		     *  (keep it as it is), and set *linknum to the index of
		     *  this link in the current links[] array. - KW
		     */
		    *go_line = HTMainText->top_of_screen;
		    if (linknum)
		        *linknum = anchors_this_screen - 1;
		} else {
		    /*
		     *  if the requested anchor is not within the currently
		     *  displayed screen, set *go_line such that the top line
		     *  will be either
		     *  (1) the line immediately below the previous
		     *      anchor, or
		     *  (2) about one third of a screenful above the line
		     *      with the target, or
		     *  (3) the first line of the document -
		     *  whichever comes last.  In all cases the line with our
		     *  target will end up being the first line with any links
		     *  on the new screen, so that we can use the
		     *  anchors_this_line counter to point to the anchor in
		     *  the new links[] array.  - kw
		     */
		    int max_offset = SEARCH_GOAL_LINE - 1;
		    if (max_offset < 0)
			max_offset = 0;
		    else if (max_offset >= display_lines)
			max_offset = display_lines - 1;
		    *go_line = prev_anchor_line - max_offset;
		    if (*go_line <= prev_prev_anchor_line)
		        *go_line = prev_prev_anchor_line + 1;
		    if (*go_line < 0)
		        *go_line = 0;
		    if (linknum)
		        *linknum = anchors_this_line - 1;
	        }
	        return(LINK_LINE_FOUND);
	    } else {
		*hightext= a->hightext;
		link_dest = HTAnchor_followMainLink((HTAnchor *)a->anchor);
		{
		    char *cp_freeme = NULL;
		    if (traversal) {
			cp_freeme = stub_HTAnchor_address(link_dest);
		    } else {
#ifndef DONT_TRACK_INTERNAL_LINKS
			if (a->link_type == INTERNAL_LINK_ANCHOR) {
			    link_dest_intl = HTAnchor_followTypedLink(
				(HTAnchor *)a->anchor, LINK_INTERNAL);
			    if (link_dest_intl && link_dest_intl != link_dest) {

				CTRACE(tfp, "HTGetLinkInfo: unexpected typed link to %s!\n",
					    link_dest_intl->parent->address);
				link_dest_intl = NULL;
			    }
			}
			if (link_dest_intl) {
			    char *cp2 = HTAnchor_address(link_dest_intl);
			    FREE(*lname);
			    *lname = cp2;
			    return(WWW_INTERN_LINK_TYPE);
			} else
#endif
			    cp_freeme = HTAnchor_address(link_dest);
		    }
		    StrAllocCopy(*lname, cp_freeme);
		    FREE(cp_freeme);
		}
		return(WWW_LINK_TYPE);
	    }
	}
    }
    return(NO);
}

/*
 *  This function finds the line indicated by line_num in the
 *  HText structure indicated by text, and searches that line
 *  for the first hit with the string indicated by target.  If
 *  there is no hit, FALSE is returned.  If there is a hit, then
 *  a copy of the line starting at that first hit is loaded into
 *  *data with all IsSpecial characters stripped, it's offset and
 *  the printable target length (without IsSpecial, or extra CJK
 *  or utf8 characters) are loaded into *offset and *tLen, and
 *  TRUE is returned. - FM
 */
PUBLIC BOOL HText_getFirstTargetInLine ARGS7(
	HText *,	text,
	int,		line_num,
	BOOL,		utf_flag,
	int *,		offset,
	int *,		tLen,
	char **,	data,
	char *,		target)
{
    HTLine *line;
    char *LineData;
    int LineOffset, HitOffset, LenNeeded, i;
    char *cp;

    /*
     *  Make sure we have an HText structure, that line_num is
     *  in its range, and that we have a target string. - FM
     */
    if (!(text && line_num >= 0 && line_num <= text->Lines &&
	  target && *target))
	return(FALSE);

    /*
     *  Find the line and set up its data and offset - FM
     */
    for (i = 0, line = text->last_line->next;
	 i < line_num && (line != text->last_line);
	 i++, line = line->next) {
	if (line->next == NULL) {
	    return(FALSE);
	}
    }
    if (!line && line->data[0])
	return(FALSE);
    LineData = (char *)line->data;
    LineOffset = (int)line->offset;

    /*
     *  If the target is on the line, load the offset of
     *  its first character and the subsequent line data,
     *  strip any special characters from the loaded line
     *  data, and return TRUE. - FM
     */
    if ((case_sensitive ?
	 (cp = LYno_attr_mbcs_strstr(LineData,
				     target,
				     utf_flag,
				     &HitOffset,
				     &LenNeeded)) != NULL :
	 (cp = LYno_attr_mbcs_case_strstr(LineData,
				     target,
				     utf_flag,
				     &HitOffset,
				     &LenNeeded)) != NULL) &&
	(LineOffset + LenNeeded) < LYcols) {
	/*
	 *  We had a hit so load the results,
	 *  remove IsSpecial characters from
	 *  the allocated data string, and
	 *  return TRUE. - FM
	 */
	*offset = (LineOffset + HitOffset);
	*tLen = (LenNeeded - HitOffset);
	 StrAllocCopy(*data, cp);
	 remove_special_attr_chars(*data);
	 return(TRUE);
    }

    /*
     *  The line does not contain the target. - FM
     */
    return(FALSE);
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

#ifdef USE_HASH
PUBLIC char *HText_getStyle NOARGS
{
   return(HTMainText ?
	  (char *) HTAnchor_style(HTMainText->node_anchor) : NULL);
}
#endif

/*
 *  HText_getSugFname returns the suggested filename of the current
 *  document (normally derived from a Content-Disposition header with
 *  attachment; filename=name.suffix). - FM
 */
PUBLIC CONST char * HText_getSugFname NOARGS
{
    return(HTMainText ?
	  HTAnchor_SugFname(HTMainText->node_anchor) : 0);
}

/*
 *  HTCheckFnameForCompression receives the address of an allocated
 *  string containing a filename, and an anchor pointer, and expands
 *  or truncates the string's suffix if appropriate, based on whether
 *  the anchor indicates that the file is compressed.  We assume
 *  that the file was not uncompressed (as when downloading), and
 *  believe the headers about whether it's compressed or not. - FM
 *
 *  Added third arg - if strip_ok is FALSE, we don't trust the anchor
 *  info enough to remove a compression suffix if the anchor object
 *  does not indicate compression. - kw
 */
PUBLIC void HTCheckFnameForCompression ARGS3(
	char **,		fname,
	HTParentAnchor *,	anchor,
	BOOL,			strip_ok)
{
    char *fn = *fname;
    char *dot = NULL, *cp = NULL;
    CONST char *ct = NULL;
    CONST char *ce = NULL;
    BOOLEAN method = 0;

    /*
     *  Make sure we have a string and anchor. - FM
     */
    if (!(fn && *fn && anchor))
	return;

    /*
     *  Make sure we have a file, not directory, name. -FM
     */
    if ((cp = strrchr(fn, '/')) != NULL) {
	fn = (cp +1);
	if (*fn == '\0') {
	    return;
	}
    }

    /*
     *  Check the anchor's content_type and content_encoding
     *  elements for a gzip or Unix compressed file. - FM
     */
    ct = HTAnchor_content_type(anchor);
    ce = HTAnchor_content_encoding(anchor);
    if (ce == NULL) {
	/*
	 *  No Content-Encoding, so check
	 *  the Content-Type. - FM
	 */
	if (!strncasecomp((ct ? ct : ""), "application/gzip", 16) ||
	    !strncasecomp((ct ? ct : ""), "application/x-gzip", 18)) {
	    method = 1;
	} else if (!strncasecomp((ct ? ct : ""),
				 "application/compress", 20) ||
		   !strncasecomp((ct ? ct : ""),
				 "application/x-compress", 22)) {
	    method = 2;
	}
    } else if (!strcasecomp(ce, "gzip") ||
	       !strcasecomp(ce, "x-gzip")) {
	    /*
	     *  It's gzipped. - FM
	     */
	    method = 1;
    } else if (!strcasecomp(ce, "compress") ||
	       !strcasecomp(ce, "x-compress")) {
	    /*
	     *  It's Unix compressed. - FM
	     */
	    method = 2;
    }

    /*
     *  If no Content-Encoding has been detected via the anchor
     *  pointer, but strip_ok is not set, there is nothing left
     *  to do. - kw
     */
    if (method == 0 && !strip_ok)
	return;

    /*
     *  Seek the last dot, and check whether
     *  we have a gzip or compress suffix. - FM
     */
    if ((dot = strrchr(fn, '.')) != NULL) {
	if (!strcasecomp(dot, ".tgz") ||
	    !strcasecomp(dot, ".gz") ||
	    !strcasecomp(dot, ".Z")) {
	    if (!method) {
	        /*
		 *  It has a suffix which signifies a gzipped
		 *  or compressed file for us, but the anchor
		 *  claims otherwise, so tweak the suffix. - FM
		 */
	        cp = (dot + 1);
		*dot = '\0';
		if (!strcasecomp(cp, "tgz")) {
		    StrAllocCat(*fname, ".tar");
		}
	    }
	    return;
	}
	if (strlen(dot) > 4) {
	    cp = ((dot + strlen(dot)) - 3);
	    if (!strcasecomp(cp, "-gz") ||
		!strcasecomp(cp, "_gz")) {
		if (!method) {
	            /*
		     *  It has a tail which signifies a gzipped
		     *  file for us, but the anchor claims otherwise,
		     *  so tweak the suffix. - FM
		     */
		    *dot = '\0';
		} else {
		    /*
		     *  The anchor claims it's gzipped, and we
		     *  believe it, so force this tail to the
		     *  conventional suffix. - FM
		     */
#ifdef VMS
		    *cp = '-';
#else
		    *cp = '.';
#endif /* VMS */
		    cp++;
		    *cp = TOLOWER(*cp);
		    cp++;
		    *cp = TOLOWER(*cp);
		}
		return;
	    }
	}
	if (strlen(dot) > 3) {
	    cp = ((dot + strlen(dot)) - 2);
	    if (!strcasecomp(cp, "-Z") ||
		!strcasecomp(cp, "_Z")) {
		if (!method) {
	            /*
		     *  It has a tail which signifies a compressed
		     *  file for us, but the anchor claims otherwise,
		     *  so tweak the suffix. - FM
		     */
		    *dot = '\0';
		} else {
		    /*
		     *  The anchor claims it's compressed, and
		     *  we believe it, so force this tail to the
		     *  conventional suffix. - FM
		     */
#ifdef VMS
		    *cp = '-';
#else
		    *cp = '.';
#endif /* VMS */
		    cp++;
		    *cp = TOUPPER(*cp);
		}
		return;
	    }
	}
    }
    if (!method) {
	/*
	 *  Don't know what compression method
	 *  was used, if any, so we won't do
	 *  anything. - FM
	 */
	return;
    }

    /*
     *  Add the appropriate suffix. - FM
     */
    if (!dot) {
	StrAllocCat(*fname, ((method == 1) ? ".gz" : ".Z"));
	return;
    }
    dot++;
    if (*dot == '\0') {
	StrAllocCat(*fname, ((method == 1) ? "gz" : "Z"));
	return;
    }
#ifdef VMS
    StrAllocCat(*fname, ((method == 1) ? "-gz" : "-Z"));
#else
    StrAllocCat(*fname, ((method == 1) ? ".gz" : ".Z"));
#endif /* !VMS */
    return;
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
 *  Return YES if we have a whereis search target on the displayed
 *  page. - kw
 */
PUBLIC BOOL HText_pageHasPrevTarget NOARGS
{
    if (!HTMainText)
	return NO;
    else
	return HTMainText->page_has_target;
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
	    Anchor_ptr->show_anchor &&
	    (Anchor_ptr->link_type != INPUT_ANCHOR ||
	     Anchor_ptr->input_field->type != F_HIDDEN_TYPE))
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
	 *  Reset flag for whereis search string - cannot be true here
	 *  since text is not our HTMainText. - kw
	 */
	if (text)
	    text->page_has_target = NO;
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

		CTRACE(tfp,
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
	CTRACE(tfp, "HText: No such anchor in this text!\n");
	return NO;
    }

    if (text != HTMainText) {		/* Comment out by ??? */
	HTMainText = text;		/* Put back in by tbl 921208 */
	HTMainAnchor = text->node_anchor;
    }

    {
	 int l = line_for_char(text, a->start);
	 CTRACE(tfp,
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
**	not included here from the original text object.
*/

/*	Style handling:
*/
/*	Apply this style to the selection
*/
PUBLIC void HText_applyStyle ARGS2(
	HText *,	me GCC_UNUSED,
	HTStyle *,	style GCC_UNUSED)
{

}


/*	Update all text with changed style.
*/
PUBLIC void HText_updateStyle ARGS2(
	HText *,	me GCC_UNUSED,
	HTStyle *,	style GCC_UNUSED)
{

}


/*	Return style of  selection
*/
PUBLIC HTStyle * HText_selectionStyle ARGS2(
	HText *,		me GCC_UNUSED,
	HTStyleSheet *,		sheet GCC_UNUSED)
{
    return 0;
}


/*	Paste in styled text
*/
PUBLIC void HText_replaceSel ARGS3(
	HText *,	me GCC_UNUSED,
	CONST char *,	aString GCC_UNUSED,
	HTStyle *,	aStyle GCC_UNUSED)
{
}


/*	Apply this style to the selection and all similarly formatted text
**	(style recovery only)
*/
PUBLIC void HTextApplyToSimilar ARGS2(
	HText *,	me GCC_UNUSED,
	HTStyle *,	style GCC_UNUSED)
{

}


/*	Select the first unstyled run.
**	(style recovery only)
*/
PUBLIC void HTextSelectUnstyled ARGS2(
	HText *,		me GCC_UNUSED,
	HTStyleSheet *,		sheet GCC_UNUSED)
{

}


/*	Anchor handling:
*/
PUBLIC void HText_unlinkSelection ARGS1(
	HText *,	me GCC_UNUSED)
{

}

PUBLIC HTAnchor * HText_referenceSelected ARGS1(
	HText *,	me GCC_UNUSED)
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
	HText *,	me GCC_UNUSED,
	HTAnchor *,	anchor GCC_UNUSED)
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
    int ch, recall;
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
    LYTrimLeading(searchstring);
    if (!(*searchstring)) {
	_statusline(CANCELLED);
	sleep(InfoSecs);
	return(NULLFILE);
    }
    LYTrimTrailing(searchstring);

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

	CTRACE(tfp,"\ndo_www_search: newfile: %s\n",doc->address);

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
 *  the file descriptor fp.
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
	/*
	 *  Add news-style quotation if requested. - FM
	 */
	if (is_reply) {
	    fputc('>',fp);
	}

        /*
	 *  Add offset.
	 */
	for (i = 0; i < (int)line->offset; i++) {
	     fputc(' ', fp);
	}

	/*
	 *  Add data.
	 */
	for (i = 0; line->data[i] != '\0'; i++) {
	    if (!IsSpecialAttrChar(line->data[i])) {
		fputc(line->data[i],fp);
	    } else if (line->data[i] == LY_SOFT_HYPHEN &&
		line->data[i + 1] == '\0') { /* last char on line */
		if (dump_output_immediately &&
		    LYRawMode &&
		    LYlowest_eightbit[current_char_set] <= 173 &&
		    (current_char_set == 0 ||
		     LYCharSet_UC[current_char_set].enc == UCT_ENC_8859 ||
		     LYCharSet_UC[current_char_set].like8859 &
				  UCT_R_8859SPECL)) {
		    fputc(0xad, fp); /* the iso8859 byte for SHY */
		} else {
		    fputc('-', fp);
		}
	    } else if (dump_output_immediately && use_underscore) {
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
	}

	/*
	 *  Add the return.
	 */
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
 *  the file descriptor fp.
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
    fprintf(fp, "THE_URL:%s\n", thelink);
    if (thetitle != NULL) {
	fprintf(fp, "THE_TITLE:%s\n", thetitle);
    }

    for (;; line = line->next) {
	/*
	 *  Add offset.
	 */
	for (i = 0; i < (int)line->offset; i++) {
	    fputc(' ', fp);
	}

	/*
	 *  Add data.
	 */
	for (i = 0; line->data[i] != '\0'; i++) {
	    if (!IsSpecialAttrChar(line->data[i])) {
		fputc(line->data[i], fp);
	     } else if (line->data[i] == LY_SOFT_HYPHEN &&
		 line->data[i + 1] == '\0') { /* last char on line */
		 if (dump_output_immediately &&
		     LYRawMode &&
		     LYlowest_eightbit[current_char_set] <= 173 &&
		     (current_char_set == 0 ||
		      LYCharSet_UC[current_char_set].enc == UCT_ENC_8859 ||
		      LYCharSet_UC[current_char_set].like8859 &
				   UCT_R_8859SPECL)) {
		     fputc(0xad, fp); /* the iso8859 byte for SHY */
		 } else {
		     fputc('-', fp);
		 }
	     }
	}

	/*
	 *  Add the return.
	 */
	fputc('\n',fp);

	if (line == HTMainText->last_line) {
	    break;
	}
    }

    /*
     *  Add the References list if appropriate
     */
    if ((nolist == FALSE) &&
	(keypad_mode == LINKS_ARE_NUMBERED ||
	 keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED)) {
	printlist(fp,FALSE);
    }

#ifdef VMS
    HadVMSInterrupt = FALSE;
#endif /* VMS */
}

PRIVATE void adjust_search_result ARGS3(
    document *,	doc,
    int,	tentative_result,
    int,	start_line)
{
    if (tentative_result > 0) {
	int anch_line = -1;
	TextAnchor * a;
	int nl_closest = -1;
	int goal = SEARCH_GOAL_LINE;
	int max_offset;
	BOOL on_screen = (tentative_result > HTMainText->top_of_screen &&
	    tentative_result <= HTMainText->top_of_screen+display_lines);
	if (goal < 1)
	    goal = 1;
	else if (goal > display_lines)
	    goal = display_lines;
	max_offset = goal - 1;

	if (on_screen && nlinks > 0) {
	    int i;
	    for (i = 0; i < nlinks; i++) {
		if (doc->line + links[i].ly - 1 <= tentative_result)
		    nl_closest = i;
		if (doc->line + links[i].ly - 1 >= tentative_result)
		    break;
	    }
	    if (nl_closest >= 0 &&
		doc->line + links[nl_closest].ly - 1 == tentative_result) {
		www_search_result = doc->line;
		doc->link = nl_closest;
		return;
	    }
	}

	/* find last anchor before or on target line */
	for (a = HTMainText->first_anchor;
	     a && a->line_num <= tentative_result-1; a = a->next) {
	    anch_line = a->line_num + 1;
	}
	/* position such that the anchor found is on first screen line,
	   if it is not too far above the target line; but also try to
	   make sure we move forward. */
	if (anch_line >= 0 &&
	    anch_line >= tentative_result - max_offset &&
	    (anch_line > start_line ||
		tentative_result <= HTMainText->top_of_screen)) {
	    www_search_result = anch_line;
	} else
	if (tentative_result - start_line > 0 &&
	    tentative_result - (start_line + 1) <= max_offset) {
	    www_search_result = start_line + 1;
	} else
	if (tentative_result > HTMainText->top_of_screen &&
	    tentative_result <= start_line && /* have wrapped */
	    tentative_result <= HTMainText->top_of_screen+goal) {
	    www_search_result = HTMainText->top_of_screen + 1;
	} else
	if (tentative_result <= goal)
	    www_search_result = 1;
	else
	    www_search_result = tentative_result - max_offset;
	if (www_search_result == doc->line) {
	    if (nl_closest >= 0) {
		doc->link = nl_closest;
		return;
	    }
	}
    }
}

PUBLIC void www_user_search ARGS3(
	int,		start_line,
	document *,	doc,
	char *,		target)
{
    register HTLine * line;
    register int count;
    int tentative_result = -1;
    TextAnchor *a;
    OptionType *option;
    char *stars = NULL, *cp;

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
    a = HTMainText->first_anchor;
    while (a && a->line_num < count - 1) {
	a = a->next;
    }

    for (;;) {
	while ((a != NULL) && a->line_num == (count - 1)) {
	    if (a->show_anchor &&
		(a->link_type != INPUT_ANCHOR ||
		 a->input_field->type != F_HIDDEN_TYPE)) {
		if (((a->hightext != NULL && case_sensitive == TRUE) &&
		     LYno_attr_char_strstr(a->hightext, target)) ||
		    ((a->hightext != NULL && case_sensitive == FALSE) &&
		     LYno_attr_char_case_strstr(a->hightext, target))) {
		    adjust_search_result(doc, count, start_line);
		    return;
		}
		if (((a->hightext2 != NULL && case_sensitive == TRUE) &&
		     LYno_attr_char_strstr(a->hightext2, target)) ||
		    ((a->hightext2 != NULL && case_sensitive == FALSE) &&
		     LYno_attr_char_case_strstr(a->hightext2, target))) {
		    adjust_search_result(doc, count, start_line);
		    return;
		}

		/*
		 *  Search the relevant form fields, taking the
		 *  case_sensitive setting into account. - FM
		 */
		if ((a->input_field != NULL && a->input_field->value != NULL) &&
		    a->input_field->type != F_HIDDEN_TYPE) {
		    if (a->input_field->type == F_PASSWORD_TYPE) {
		        /*
			 *  Check the actual, hidden password, and then
			 *  the displayed string. - FM
			 */
			if (((case_sensitive == TRUE) &&
			     LYno_attr_char_strstr(a->input_field->value,
						   target)) ||
			    ((case_sensitive == FALSE) &&
			     LYno_attr_char_case_strstr(a->input_field->value,
							target))) {
			    adjust_search_result(doc, count, start_line);
			    return;
			}
			StrAllocCopy(stars, a->input_field->value);
			for (cp = stars; *cp != '\0'; cp++)
			    *cp = '*';
			if (((case_sensitive == TRUE) &&
			     LYno_attr_char_strstr(stars, target)) ||
			    ((case_sensitive == FALSE) &&
			     LYno_attr_char_case_strstr(stars, target))) {
			    FREE(stars);
			    adjust_search_result(doc, count, start_line);
			    return;
			}
			FREE(stars);
		   } else if (a->input_field->type == F_OPTION_LIST_TYPE) {
			/*
			 *  Search the option strings that are displayed
			 *  when the popup is invoked. - FM
			 */
			option = a->input_field->select_list;
			while (option != NULL) {
			    if (((option->name != NULL &&
				  case_sensitive == TRUE) &&
				 LYno_attr_char_strstr(option->name,
						       target)) ||
				((option->name != NULL &&
				  case_sensitive == FALSE) &&
				 LYno_attr_char_case_strstr(option->name,
							    target))) {
				adjust_search_result(doc, count, start_line);
				return;
			    }
			    option = option->next;
			}
		    } else if (a->input_field->type == F_RADIO_TYPE) {
			/*
			 *  Search for checked or unchecked parens. - FM
			 */
		        if (a->input_field->num_value) {
			    cp = checked_radio;
			} else {
			    cp = unchecked_radio;
			}
			if (((case_sensitive == TRUE) &&
			     LYno_attr_char_strstr(cp, target)) ||
			    ((case_sensitive == FALSE) &&
			     LYno_attr_char_case_strstr(cp, target))) {
			    adjust_search_result(doc, count, start_line);
			    return;
			}
		    } else if (a->input_field->type == F_CHECKBOX_TYPE) {
			/*
			 *  Search for checked or unchecked
			 *  square brackets. - FM
			 */
		        if (a->input_field->num_value) {
			    cp = checked_box;
			} else {
			    cp = unchecked_box;
			}
			if (((case_sensitive == TRUE) &&
			     LYno_attr_char_strstr(cp, target)) ||
			    ((case_sensitive == FALSE) &&
			     LYno_attr_char_case_strstr(cp, target))) {
			    adjust_search_result(doc, count, start_line);
			    return;
			}
		    } else {
		        /*
			 *  Check the values intended for display.
			 *  May have been found already via the
			 *  hightext search, but make sure here
			 *  that the entire value is searched. - FM
			 */
			if (((case_sensitive == TRUE) &&
			     LYno_attr_char_strstr(a->input_field->value,
						   target)) ||
			    ((case_sensitive == FALSE) &&
			     LYno_attr_char_case_strstr(a->input_field->value,
							target))) {
			    adjust_search_result(doc, count, start_line);
			    return;
			}
		    }
		}
	    }
	    a = a->next;
	}
	if (a != NULL && a->line_num <= (count - 1)) {
	    a = a->next;
	}

	if (case_sensitive && LYno_attr_char_strstr(line->data, target)) {
	    tentative_result = count;
	    break;
	} else if (!case_sensitive &&
		   LYno_attr_char_case_strstr(line->data, target)) {
	    tentative_result = count;
	    break;
	} else if (line == HTMainText->last_line) {  /* next line */
	    break;
	} else {			/* end */
	    line = line->next;
	    count++;
	}
    }
    if (tentative_result > 0) {
	adjust_search_result(doc, tentative_result, start_line);
	return;
    }

    /*
     *  Search from the beginning.
     */
    line = HTMainText->last_line->next; /* set to first line */
    count = 1;
    a = HTMainText->first_anchor;
    while (a && a->line_num < count - 1) {
	a = a->next;
    }

    for (;;) {
	while ((a != NULL) && a->line_num == (count - 1)) {
	    if (a->show_anchor &&
		(a->link_type != INPUT_ANCHOR ||
		 a->input_field->type != F_HIDDEN_TYPE)) {
		if (((a->hightext != NULL && case_sensitive == TRUE) &&
		     LYno_attr_char_strstr(a->hightext, target)) ||
		    ((a->hightext != NULL && case_sensitive == FALSE) &&
		     LYno_attr_char_case_strstr(a->hightext, target))) {
		    adjust_search_result(doc, count, start_line);
		    return;
		}
		if (((a->hightext2 != NULL && case_sensitive == TRUE) &&
		     LYno_attr_char_strstr(a->hightext2, target)) ||
		    ((a->hightext2 != NULL && case_sensitive == FALSE) &&
		     LYno_attr_char_case_strstr(a->hightext2, target))) {
		    adjust_search_result(doc, count, start_line);
		    return;
		}

		/*
		 *  Search the relevant form fields, taking the
		 *  case_sensitive setting into account. - FM
		 */
		if ((a->input_field != NULL && a->input_field->value != NULL) &&
		    a->input_field->type != F_HIDDEN_TYPE) {
		    if (a->input_field->type == F_PASSWORD_TYPE) {
		        /*
			 *  Check the actual, hidden password, and then
			 *  the displayed string. - FM
			 */
			if (((case_sensitive == TRUE) &&
			     LYno_attr_char_strstr(a->input_field->value,
						   target)) ||
			    ((case_sensitive == FALSE) &&
			     LYno_attr_char_case_strstr(a->input_field->value,
							target))) {
			    adjust_search_result(doc, count, start_line);
			    return;
			}
			StrAllocCopy(stars, a->input_field->value);
			for (cp = stars; *cp != '\0'; cp++)
			    *cp = '*';
			if (((case_sensitive == TRUE) &&
			     LYno_attr_char_strstr(stars, target)) ||
			    ((case_sensitive == FALSE) &&
			     LYno_attr_char_case_strstr(stars, target))) {
			    FREE(stars);
			    adjust_search_result(doc, count, start_line);
			    return;
			}
			FREE(stars);
		   } else if (a->input_field->type == F_OPTION_LIST_TYPE) {
			/*
			 *  Search the option strings that are displayed
			 *  when the popup is invoked. - FM
			 */
			option = a->input_field->select_list;
			while (option != NULL) {
			    if (((option->name != NULL &&
				  case_sensitive == TRUE) &&
				 LYno_attr_char_strstr(option->name,
						       target)) ||
				((option->name != NULL &&
				  case_sensitive == FALSE) &&
				 LYno_attr_char_case_strstr(option->name,
							    target))) {
				adjust_search_result(doc, count, start_line);
				return;
			    }
			    option = option->next;
			}
		    } else if (a->input_field->type == F_RADIO_TYPE) {
			/*
			 *  Search for checked or unchecked parens. - FM
			 */
		        if (a->input_field->num_value) {
			    cp = checked_radio;
			} else {
			    cp = unchecked_radio;
			}
			if (((case_sensitive == TRUE) &&
			     LYno_attr_char_strstr(cp, target)) ||
			    ((case_sensitive == FALSE) &&
			     LYno_attr_char_case_strstr(cp, target))) {
			    adjust_search_result(doc, count, start_line);
			    return;
			}
		    } else if (a->input_field->type == F_CHECKBOX_TYPE) {
			/*
			 *  Search for checked or unchecked
			 *  square brackets. - FM
			 */
		        if (a->input_field->num_value) {
			    cp = checked_box;
			} else {
			    cp = unchecked_box;
			}
			if (((case_sensitive == TRUE) &&
			     LYno_attr_char_strstr(cp, target)) ||
			    ((case_sensitive == FALSE) &&
			     LYno_attr_char_case_strstr(cp, target))) {
			    adjust_search_result(doc, count, start_line);
			    return;
			}
		    } else {
		        /*
			 *  Check the values intended for display.
			 *  May have been found already via the
			 *  hightext search, but make sure here
			 *  that the entire value is searched. - FM
			 */
			if (((case_sensitive == TRUE) &&
			     LYno_attr_char_strstr(a->input_field->value,
						   target)) ||
			    ((case_sensitive == FALSE) &&
			     LYno_attr_char_case_strstr(a->input_field->value,
							target))) {
			    adjust_search_result(doc, count, start_line);
			    return;
			}
		    }
		}
	    }
	    a = a->next;
	}
	if (a != NULL && a->line_num <= (count - 1)) {
	    a = a->next;
	}

	    if (case_sensitive && LYno_attr_char_strstr(line->data, target)) {
	        tentative_result=count;
		break;
	    } else if (!case_sensitive &&
		       LYno_attr_char_case_strstr(line->data, target)) {
	        tentative_result = count;
		break;
	    } else if (count > start_line) {  /* next line */
		_user_message(STRING_NOT_FOUND, target);
		sleep(MessageSecs);
	        return;			/* end */
	    } else {
	        line = line->next;
		count++;
	}
    }
    if (tentative_result > 0) {
	adjust_search_result(doc, tentative_result, start_line);
    }
}

PUBLIC void user_message ARGS2(
	CONST char *,	message,
	CONST char *,	argument)
{
    char *temp = NULL;
    char temp_arg[256];

    if (message == NULL) {
	mustshow = FALSE;
	return;
    }

    /*
     *  Make sure we don't overrun any buffers.
     */
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
PUBLIC CONST char * HText_getContentBase NOARGS
{
    return(HTMainText ?
	   HTAnchor_content_base(HTMainText->node_anchor) : 0);
}

/*
 *  HText_getContentLocation returns the Content-Location header
 *  of the current document.
 */
PUBLIC CONST char * HText_getContentLocation NOARGS
{
    return(HTMainText ?
	   HTAnchor_content_location(HTMainText->node_anchor) : 0);
}

PUBLIC void HTuncache_current_document NOARGS
{
    /*
     *  Should remove current document from memory.
     */
    if (HTMainText) {
	HTParentAnchor * htmain_anchor = HTMainText->node_anchor;

	if (htmain_anchor) {
	    if (!(HTOutputFormat && HTOutputFormat == WWW_SOURCE)) {
		FREE(htmain_anchor->UCStages);
	    }
	}
	CTRACE(tfp, "\rHTuncache.. freeing document for '%s'%s\n",
			    ((htmain_anchor &&
			      htmain_anchor->address) ?
			       htmain_anchor->address : "unknown anchor"),
			    ((htmain_anchor &&
			      htmain_anchor->post_data) ?
				      " with POST data" : ""));
	HTList_removeObject(loaded_texts, HTMainText);
	HText_free(HTMainText);
	HTMainText = NULL;
    } else {
	CTRACE(tfp, "HTuncache.. HTMainText already is NULL!\n");
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

PUBLIC BOOL HTLoadedDocumentEightbit NOARGS
{
    if (!HTMainText)
	return (NO);
    else
	return (HTMainText->have_8bit_chars);
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

PUBLIC int HText_LastLineSize ARGS2(
	HText *,	text,
	BOOL,		IgnoreSpaces)
{
    if (!text || !text->last_line || !text->last_line->size)
	return 0;
    return HText_TrueLineSize(text->last_line, text, IgnoreSpaces);
}

PUBLIC int HText_PreviousLineSize ARGS2(
	HText *,	text,
	BOOL,		IgnoreSpaces)
{
    HTLine * line;

    if (!text || !text->last_line)
	return 0;
    if (!(line = text->last_line->prev))
	return 0;
    return HText_TrueLineSize(line, text, IgnoreSpaces);
}

PRIVATE int HText_TrueLineSize ARGS3(
	HTLine *,	line,
	HText *,	text,
	BOOL,		IgnoreSpaces)
{
    size_t i;
    int true_size = 0;

    if (!(line && line->size))
	return 0;

    if (IgnoreSpaces) {
	for (i = 0; i < line->size; i++) {
	    if (!IsSpecialAttrChar((unsigned char)line->data[i]) &&
		(!(text && text->T.output_utf8) ||
		 (unsigned char)line->data[i] < 128 ||
		 ((unsigned char)(line->data[i] & 0xc0) == 0xc0)) &&
	        !isspace((unsigned char)line->data[i]) &&
		(unsigned char)line->data[i] != HT_NON_BREAK_SPACE &&
		(unsigned char)line->data[i] != HT_EM_SPACE) {
		true_size++;
	    }
	}
    } else {
	for (i = 0; i < line->size; i++) {
	    if (!IsSpecialAttrChar(line->data[i]) &&
		(!(text && text->T.output_utf8) ||
		 (unsigned char)line->data[i] < 128 ||
		 ((unsigned char)(line->data[i] & 0xc0) == 0xc0))) {
		true_size++;
	    }
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
    BOOL IgnoreSpaces = FALSE;

    if (text) {
	column = (text->in_line_1 ?
		  (int)text->style->indent1st : (int)text->style->leftIndent)
		  + HText_LastLineSize(text, IgnoreSpaces)
		  + (int)text->last_line->offset;
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
	if (last)
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
 *  This function is for saving the address of a link
 *  which had an attribute in the markup that resolved
 *  to a URL (i.e., not just a NAME or ID attribute),
 *  but was found in HText_endAnchor() to have no visible
 *  content for use as a link name.  It loads the address
 *  into text->hidden_links, whose count can be determined
 *  via HText_HiddenLinks(), below.  The addresses can be
 *  retrieved via HText_HiddenLinkAt(), below, based on
 *  count. - FM
 */
PRIVATE void HText_AddHiddenLink ARGS2(
	HText *,	text,
	TextAnchor *,	textanchor)
{
    HTAnchor *dest;

    /*
     *  Make sure we have an HText structure and anchor. - FM
     */
    if (!(text && textanchor && textanchor->anchor))
	return;

    /*
     *  Create the hidden links list
     *  if it hasn't been already. - FM
     */
    if (text->hidden_links == NULL)
	text->hidden_links = HTList_new();

    /*
     *  Store the address, in reverse list order
     *  so that first in will be first out on
     *  retrievals. - FM
     */
    if ((dest = HTAnchor_followMainLink((HTAnchor *)textanchor->anchor)) &&
	(text->hiddenlinkflag != HIDDENLINKS_IGNORE ||
	 HTList_isEmpty(text->hidden_links)))
	HTList_appendObject(text->hidden_links, HTAnchor_address(dest));

    return;
}

/*
 *  This function returns the number of addresses
 *  that are loaded in text->hidden_links. - FM
 */
PUBLIC int HText_HiddenLinkCount ARGS1(
	HText *,	text)
{
    int count = 0;

    if (text && text->hidden_links)
	count = HTList_count((HTList *)text->hidden_links);

    return(count);
}

/*
 *  This function returns the address, corresponding to
 *  a hidden link, at the position (zero-based) in the
 *  text->hidden_links list of the number argument. - FM
 */
PUBLIC char * HText_HiddenLinkAt ARGS2(
	HText *,	text,
	int,		number)
{
    char *href = NULL;

    if (text && text->hidden_links && number >= 0)
	href = (char *)HTList_objectAt((HTList *)text->hidden_links, number);

    return(href);
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
PRIVATE char * HTFormAcceptCharset = NULL;
PRIVATE BOOLEAN HTFormDisabled = FALSE;
PRIVATE PerFormInfo * HTCurrentForm;

PUBLIC void HText_beginForm ARGS5(
	char *,		action,
	char *,		method,
	char *,		enctype,
	char *,		title,
	CONST char *,	accept_cs)
{
    PerFormInfo * newform;
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

    /*
     *  Check for an ACCEPT_CHARSET.  If present, store it and
     *  convert to lowercase and collapse spaces. - kw
     */
    if (accept_cs != NULL) {
	StrAllocCopy(HTFormAcceptCharset, accept_cs);
	LYRemoveBlanks(HTFormAcceptCharset);
	LYLowerCase(HTFormAcceptCharset);
    }

    /*
     *  Create a new "PerFormInfo" structure to hold info on the current
     *  form.  The HTForm* variables could all migrate there, currently
     *  this isn't done (yet?) but it might be less confusing.
     *  Currently the only data saved in this structure that will actually
     *  be used is the accept_cs string.
     *  This will be appended to the forms list kept by the HText object
     *  if and when we reach a HText_endForm. - kw
     */
    newform = (PerFormInfo *)calloc(1, sizeof(PerFormInfo));
    if (newform == NULL)
	outofmem(__FILE__,"HText_beginForm");
    newform->number = HTFormNumber;

    PerFormInfo_free(HTCurrentForm); /* shouldn't happen here - kw */
    HTCurrentForm = newform;

    CTRACE(tfp, "BeginForm: action:%s Method:%d%s%s%s%s%s%s\n",
		HTFormAction, HTFormMethod,
		(HTFormTitle ? " Title:" : ""),
		(HTFormTitle ? HTFormTitle : ""),
		(HTFormEnctype ? " Enctype:" : ""),
		(HTFormEnctype ? HTFormEnctype : ""),
		(HTFormAcceptCharset ? " Accept-charset:" : ""),
		(HTFormAcceptCharset ? HTFormAcceptCharset : ""));
}

PUBLIC void HText_endForm ARGS1(
	HText *,	text)
{
    if (HTFormFields == 1 && text && text->first_anchor) {
	/*
	 *  Support submission of a single text input field in
	 *  the form via <return> instead of a submit button. - FM
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
    /*
     *  Append info on the current form to the HText object's list of
     *  forms.
     *  HText_beginInput call will have set some of the data in the
     *  PerFormInfo structure (if there were any form fields at all),
     *  we also fill in the ACCEPT-CHARSET data now (this could have
     *  been done earlier). - kw
     */
    if (HTCurrentForm) {
	if (HTFormDisabled)
	    HTCurrentForm->disabled = TRUE;
	HTCurrentForm->accept_cs = HTFormAcceptCharset;
	HTFormAcceptCharset = NULL;
	if (!text->forms)
	    text->forms = HTList_new();
	HTList_appendObject(text->forms, HTCurrentForm);
	HTCurrentForm = NULL;
    } else {
	CTRACE(tfp, "endForm:    HTCurrentForm is missing!\n");
    }

    FREE(HTCurSelectGroup);
    FREE(HTCurSelectGroupSize);
    FREE(HTCurSelectedOptionValue);
    FREE(HTFormAction);
    FREE(HTFormEnctype);
    FREE(HTFormTitle);
    FREE(HTFormAcceptCharset);
    HTFormFields = 0;
    HTFormDisabled = FALSE;
}

PUBLIC void HText_beginSelect ARGS4(
	char *,		name,
	int,		name_cs,
	BOOLEAN,	multiple,
	char *,		size)
{
   /*
    *  Save the group name.
    */
   StrAllocCopy(HTCurSelectGroup, name);
   HTCurSelectGroupCharset = name_cs;

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

    CTRACE(tfp,"HText_beginSelect: name=%s type=%d size=%s\n",
	       ((HTCurSelectGroup == NULL) ?
				  "<NULL>" : HTCurSelectGroup),
		HTCurSelectGroupType,
	       ((HTCurSelectGroupSize == NULL) ?
				      "<NULL>" : HTCurSelectGroupSize));
    CTRACE(tfp,"HText_beginSelect: name_cs=%d \"%s\"\n",
		HTCurSelectGroupCharset,
		(HTCurSelectGroupCharset >= 0 ?
		 LYCharSet_UC[HTCurSelectGroupCharset].MIMEname : "<UNKNOWN>"));
}

/*
**  This function returns the number of the option whose
**  value currently is being accumulated for a select
**  block. - LE && FM
*/
PUBLIC int HText_getOptionNum ARGS1(
	HText *,	text)
{
    TextAnchor *a;
    OptionType *op;
    int n = 1; /* start count at 1 */

    if (!(text && text->last_anchor))
	return(0);

    a = text->last_anchor;
    if (!(a->link_type == INPUT_ANCHOR && a->input_field &&
	  a->input_field->type == F_OPTION_LIST_TYPE))
	return(0);

    for (op = a->input_field->select_list; op; op = op->next)
	n++;
    CTRACE(tfp, "HText_getOptionNum: Got number '%d'.\n", n);
    return(n);
}

/*
**  This function checks for a numbered option pattern
**  as the prefix for an option value.  If present, and
**  we are in the correct keypad mode, it returns a
**  pointer to the actual value, following that prefix.
**  Otherwise, it returns the original pointer.
*/
PRIVATE char * HText_skipOptionNumPrefix ARGS1(
	char *,		opname)
{
    /*
     *  Check if we are in the correct keypad mode.
     */
    if (keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED) {
	/*
	 *  Skip the option number embedded in the option name so the
	 *  extra chars won't mess up cgi scripts processing the value.
	 *  The format is (nnn)__ where nnn is a number and there is a
	 *  minimum of 5 chars (no underscores if (nnn) exceeds 5 chars).
	 *  See HTML.c.  If the chars don't exactly match this format,
	 *  just use all of opname.  - LE
	 */
	char *cp = opname;

	if ((cp && *cp && *cp++ == '(') &&
	    *cp && isdigit(*cp++)) {
	    while (*cp && isdigit(*cp))
	        ++cp;
	    if (*cp && *cp++ == ')') {
		int i = (cp - opname);

		while (i < 5) {
		    if (*cp != '_')
		        break;
		    i++;
		    cp++;
		}
		if (i < 5 ) {
		    cp = opname;
		}
	    } else {
	        cp = opname;
	    }
	} else {
	    cp = opname;
	}
	return(cp);
    }

    return(opname);
}

/*
**  We couldn't set the value field for the previous option
**  tag so we have to do it now.  Assume that the last anchor
**  was the previous options tag.
*/
PUBLIC char * HText_setLastOptionValue ARGS7(
	HText *,	text,
	char *,		value,
	char*,		submit_value,
	int,		order,
	BOOLEAN,	checked,
	int,		val_cs,
	int,		submit_val_cs)
{
    char *cp, *cp1;
    char *ret_Value = NULL;
    unsigned char *tmp = NULL;
    int number = 0, i, j;

    if (!(text && text->last_anchor &&
	  text->last_anchor->link_type == INPUT_ANCHOR)) {
	CTRACE(tfp, "HText_setLastOptionValue: invalid call!  value:%s!\n",
		    (value ? value : "<NULL>"));
	return NULL;
    }

    CTRACE(tfp, "Entering HText_setLastOptionValue: value:%s, checked:%s\n",
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
    if (HTCurSelectGroupType == F_RADIO_TYPE &&
	LYSelectPopups &&
	keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED) {
	/*
	 *  Collapse any space between the popup option
	 *  prefix and actual value. - FM
	 */
	if ((cp1 = HText_skipOptionNumPrefix(cp)) > cp) {
	    i = 0, j = (cp1 - cp);
	    while (isspace((unsigned char)cp1[i]) ||
		   IsSpecialAttrChar((unsigned char)cp1[i])) {
		i++;
	    }
	    if (i > 0) {
		while (cp1[i] != '\0')
		    cp[j++] = cp1[i++];
		cp[j] = '\0';
	    }
	}
    }

    if (HTCurSelectGroupType == F_CHECKBOX_TYPE) {
	StrAllocCopy(text->last_anchor->input_field->value, cp);
	text->last_anchor->input_field->value_cs = val_cs;
	/*
	 *  Put the text on the screen as well.
	 */
	HText_appendText(text, cp);

    } else if (LYSelectPopups == FALSE) {
	StrAllocCopy(text->last_anchor->input_field->value,
		     (submit_value ? submit_value : cp));
	text->last_anchor->input_field->value_cs = (submit_value ?
						    submit_val_cs : val_cs);
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
	    if (text->last_anchor->input_field->type != F_OPTION_LIST_TYPE) {
		CTRACE(tfp, "HText_setLastOptionValue: last input_field not F_OPTION_LIST_TYPE (%d)\n",
			    F_OPTION_LIST_TYPE);
		CTRACE(tfp, "                          but %d, ignoring!\n",
			    text->last_anchor->input_field->type);
		return NULL;
	    }

	    new_ptr = text->last_anchor->input_field->select_list =
				(OptionType *)calloc(1, sizeof(OptionType));
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
				(OptionType *)calloc(1, sizeof(OptionType));
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
		    val_cs = current_char_set;
		} else if (kanji_code == SJIS) {
		    TO_SJIS((unsigned char *)cp, tmp);
		    val_cs = current_char_set;
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
		     (submit_value ? submit_value :
		      HText_skipOptionNumPrefix(new_ptr->name)));
	new_ptr->value_cs = (submit_value ? submit_val_cs : val_cs);

	if (first_option) {
	    StrAllocCopy(HTCurSelectedOptionValue, new_ptr->name);
	    text->last_anchor->input_field->num_value = 0;
	    /*
	     *  If this is the first option in a popup select list,
	     *  HText_beginInput may have allocated the value and
	     *  cp_submit_value fields, so free them now to avoid
	     *  a memory leak. - kw
	     */
	    FREE(text->last_anchor->input_field->value);
	    FREE(text->last_anchor->input_field->cp_submit_value);

	    text->last_anchor->input_field->value =
		text->last_anchor->input_field->select_list->name;
	    text->last_anchor->input_field->orig_value =
		text->last_anchor->input_field->select_list->name;
	    text->last_anchor->input_field->cp_submit_value =
		text->last_anchor->input_field->select_list->cp_submit_value;
	    text->last_anchor->input_field->orig_submit_value =
		text->last_anchor->input_field->select_list->cp_submit_value;
	    text->last_anchor->input_field->value_cs =
		new_ptr->value_cs;
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
	    text->last_anchor->input_field->value_cs =
		new_ptr->value_cs;
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
	    ret_Value = HTCurSelectedOptionValue;
	}
    }

    if (TRACE) {
	fprintf(tfp,"HText_setLastOptionValue:%s value=%s",
		(order == LAST_ORDER) ? " LAST_ORDER" : "",
		value);
	fprintf(tfp,"            val_cs=%d \"%s\"",
			val_cs,
			(val_cs >= 0 ?
			 LYCharSet_UC[val_cs].MIMEname : "<UNKNOWN>"));
	if (submit_value) {
	    fprintf(tfp, " (submit_val_cs %d \"%s\") submit_value%s=%s\n",
		    submit_val_cs,
		    (submit_val_cs >= 0 ?
		     LYCharSet_UC[submit_val_cs].MIMEname : "<UNKNOWN>"),
		    (HTCurSelectGroupType == F_CHECKBOX_TYPE) ?
		                                  "(ignored)" : "",
		    submit_value);
	}
	else {
	    fprintf(tfp,"\n");
	}
    }
    return(ret_Value);
}

/*
 *  Assign a form input anchor.
 *  Returns the number of characters to leave
 *  blank so that the input field can fit.
 */
PUBLIC int HText_beginInput ARGS3(
	HText *,		text,
	BOOL,			underline,
	InputFieldData *,	I)
{

    TextAnchor * a = (TextAnchor *) calloc(1, sizeof(*a));
    FormInfo * f = (FormInfo *) calloc(1, sizeof(*f));
    CONST char *cp_option = NULL;
    char *IValue = NULL;
    unsigned char *tmp = NULL;
    int i, j;

    CTRACE(tfp,"Entering HText_beginInput\n");

    if (a == NULL || f == NULL)
	outofmem(__FILE__, "HText_beginInput");

    a->start = text->chars + text->last_line->size;
    a->inUnderline = underline;
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
	I->name_cs = HTCurSelectGroupCharset;
    }
    if (I->name && I->type && !strcasecomp(I->type, "radio")) {
	if (!text->last_anchor) {
	    I->checked = TRUE;
	} else {
	    TextAnchor * b = text->first_anchor;
	    int i2 = 0;
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
			i2++;
		    }
		}
		if (b == text->last_anchor) {
		    if (i2 == 0)
		       I->checked = TRUE;
		    break;
		}
		b = b->next;
	    }
	}
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
		I->value_cs = current_char_set;
	    } else if (kanji_code == SJIS) {
		TO_SJIS((unsigned char *)IValue, tmp);
		I->value_cs = current_char_set;
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
	I->name_cs = HTCurSelectGroupCharset;

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
     *  (num_value is only relevant to check and radio types).
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
	} else if (!strcasecomp(I->type,"keygen")) {
	    f->type = F_KEYGEN_TYPE;
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
	f->name_cs = I->name_cs;
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
	    CTRACE(tfp,
		  "GridText: No name present in input field; not displaying\n");
	    FREE(a);
	    FREE(f);
	    FREE(IValue);
	    return(0);
	}
    }

    /*
     *  Add this anchor to the anchor list
     */
    if (text->last_anchor) {
	text->last_anchor->next = a;
    } else {
	text->first_anchor = a;
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
	f->value_cs = I->value_cs;
    } else if (f->type != F_OPTION_LIST_TYPE) {
	StrAllocCopy(f->value, "");
	/*
	 *  May be an empty INPUT field.  The text entered will then
	 *  probably be in the current display character set. - kw
	 */
	f->value_cs = current_char_set;
    }

    /*
     *  Run checks and fill in necessary values.
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
     *  Store accept-charset if present, converting to lowercase
     *  and collapsing spaces. - kw
     */
    if (I->accept_cs) {
	StrAllocCopy(f->accept_cs, I->accept_cs);
	LYRemoveBlanks(f->accept_cs);
	LYLowerCase(f->accept_cs);
    }

    /*
     *  Add numbers to form fields if needed. - LE & FM
     */
    switch (f->type) {
	/*
	 *  Do not supply number for hidden fields, nor
	 *  for types that are not yet implemented.
	 */
	case F_HIDDEN_TYPE:
	case F_FILE_TYPE:
	case F_RANGE_TYPE:
	case F_KEYGEN_TYPE:
	    a->number = 0;
	    break;

	default:
	    if (keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED)
		a->number = ++(text->last_anchor_number);
	    else
		a->number = 0;
	    break;
    }
    if (keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED && a->number > 0) {
	char marker[16];

	if (f->type != F_OPTION_LIST_TYPE)
	    /*
	     *  '[' was already put out for a popup menu
	     *  designator.  See HTML.c.
	     */
	    HText_appendCharacter(text, '[');
	sprintf(marker,"%d]", a->number);
	HText_appendText(text, marker);
	if (f->type == F_OPTION_LIST_TYPE)
	    /*
	     *  Add option list designation char.
	     */
	    HText_appendCharacter(text, '[');
	a->start = text->chars + text->last_line->size;
	a->line_pos = text->last_line->size;
    }

    /*
     *  Restrict SIZE to maximum allowable size.
     */
    switch (f->type) {
	int MaximumSize;

	case F_SUBMIT_TYPE:
	case F_IMAGE_SUBMIT_TYPE:
	case F_RESET_TYPE:
	case F_TEXT_TYPE:
	case F_TEXTAREA_TYPE:
	    /*
	     *  For submit and reset buttons, and for text entry
	     *  fields and areas, we limit the size element to that
	     *  of one line for the current style because that's
	     *  the most we could highlight on overwrites, and/or
	     *  handle in the line editor.  The actual values for
	     *  text entry lines can be long, and will be scrolled
	     *  horizontally within the editing window. - FM
	     */
	    MaximumSize = (LYcols - 1) -
			  (int)text->style->leftIndent -
			  (int)text->style->rightIndent;

	    /*
	     *  If we are numbering form links, take that into
	     *  account as well. - FM
	     */
	    if (keypad_mode == LINKS_AND_FORM_FIELDS_ARE_NUMBERED)
	        MaximumSize -= ((a->number/10) + 3);
	    if (f->size > MaximumSize)
	        f->size = MaximumSize;

	    /*
	     *  Save value for submit/reset buttons so they
	     *  will be visible when printing the page. - LE
	     */
	    I->value = f->value;
	    break;


	default:
	    /*
	     *  For all other fields we limit the size element to
	     *  10 less than the screen width, because either they
	     *  are types with small placeholders, and/or are a
	     *  type which is handled via a popup window. - FM
	     */
	    if (f->size > LYcols-10)
		f->size = LYcols-10;  /* maximum */
	    break;
    }

    /*
     *  Add this anchor to the anchor list
     */
    text->last_anchor = a;

    if (HTCurrentForm) {	/* should always apply! - kw */
	if (!HTCurrentForm->first_field) {
	    HTCurrentForm->first_field = f;
	}
	HTCurrentForm->last_field = f;
	HTCurrentForm->nfields++; /* will count hidden fields - kw */
	/*
	 *  Propagate form field's accept-charset attribute to enclosing
	 *  form if the form itself didn't have an accept-charset - kw
	 */
	if (f->accept_cs && !HTFormAcceptCharset) {
	    StrAllocCopy(HTFormAcceptCharset, f->accept_cs);
	}
	if (!text->forms) {
	    text->forms = HTList_new();
	}
    } else {
	CTRACE(tfp, "beginInput: HTCurrentForm is missing!\n");
    }

    CTRACE(tfp, "Input link: name=%s\nvalue=%s\nsize=%d\n",
			f->name,
			((f->value != NULL) ? f->value : ""),
			f->size);
    CTRACE(tfp, "Input link: name_cs=%d \"%s\" (from %d \"%s\")\n",
			f->name_cs,
			(f->name_cs >= 0 ?
			 LYCharSet_UC[f->name_cs].MIMEname : "<UNKNOWN>"),
			I->name_cs,
			(I->name_cs >= 0 ?
			 LYCharSet_UC[I->name_cs].MIMEname : "<UNKNOWN>"));
    CTRACE(tfp, "            value_cs=%d \"%s\" (from %d \"%s\")\n",
			f->value_cs,
			(f->value_cs >= 0 ?
			 LYCharSet_UC[f->value_cs].MIMEname : "<UNKNOWN>"),
			I->value_cs,
			(I->value_cs >= 0 ?
			 LYCharSet_UC[I->value_cs].MIMEname : "<UNKNOWN>"));

    /*
     *  Return the SIZE of the input field.
     */
    return(f->size);
}

/*
 *  Get a translation (properly: transcoding) quality, factoring in
 *  our ability to translate (an UCTQ_t) and a possible q parameter
 *  on the given charset string, for cs_from -> givenmime.
 *  The parsed input string will be mutilated on exit(!).
 *  Note that results are not normalised to 1.0, but results from
 *  different calls of this function can be compared. - kw
 *
 *  Obsolete, it was planned to use here a quality parametr UCTQ_t,
 *  which is boolean now.
 */
PRIVATE double get_trans_q ARGS2(
    int,		cs_from,
    char *,		givenmime)
{
    double df = 1.0;
    BOOL tq;
    char *p;
    if (!givenmime || !(*givenmime))
	return 0.0;
    if ((p = strchr(givenmime,';')) != NULL) {
	*p++ = '\0';
    }
    if (!strcmp(givenmime, "*"))
	tq = UCCanTranslateFromTo(cs_from,
				  UCGetLYhndl_byMIME("utf-8"));
    else
	tq = UCCanTranslateFromTo(cs_from,
				  UCGetLYhndl_byMIME(givenmime));
    if (!tq)
	return 0.0;
    if (p && *p) {
	char *pair, *field = p, *pval, *ptok;
	/* Get all the parameters to the Charset */
	while ((pair = HTNextTok(&field, ";", "\"", NULL)) != NULL) {
	    if ((ptok = HTNextTok(&pair, "= ", NULL, NULL)) != NULL &&
		(pval = HTNextField(&pair)) != NULL) {
		if (0==strcasecomp(ptok,"q")) {
		    df = strtod(pval, NULL);
		    break;
		}
	    }
	}
	return (df * tq);
    } else {
	return tq;
    }
}

/*
 *  Find the best charset for submission, if we have an ACCEPT_CHARSET
 *  list.  It factors in how well we can translate (just as guess, and
 *  not a very good one..) and possible  ";q=" factors.  Yes this is
 *  more general than it needs to be here.
 *
 *  Input is cs_in and acceptstring.
 *
 *  Will return charset handle as int.
 *  best_csname will point to a newly allocated MIME string for the
 *  charset corresponding to the return value if return value >= 0.
 *  - kw
 */
PRIVATE int find_best_target_cs ARGS3(
    char **,		best_csname,
    int,		cs_from,
    CONST char *,	acceptstring)
{
    char *paccept = NULL;
    double bestq = -1.0;
    char *bestmime = NULL;
    char *field, *nextfield;
    StrAllocCopy(paccept, acceptstring);
    nextfield = paccept;
    while ((field = HTNextTok(&nextfield, ",", "\"", NULL)) != NULL) {
	double q;
	if (*field != '\0') {
	    /* Get the Charset*/
	    q = get_trans_q(cs_from, field);
	    if (q > bestq) {
		bestq = q;
		bestmime = field;
	    }
	}
    }
    if (bestmime) {
	if (!strcmp(bestmime, "*")) /* non-standard for HTML attribute.. */
	    StrAllocCopy(*best_csname, "utf-8");
	else
	    StrAllocCopy(*best_csname, bestmime);
	FREE(paccept);
	if (bestq > 0)
	    return (UCGetLYhndl_byMIME(*best_csname));
	else
	    return (-1);
    }
    FREE(paccept);
    return (-1);
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
    PerFormInfo *thisform;
    int len;
    char *query = NULL;
    char *escaped1 = NULL, *escaped2 = NULL;
    int first_one = 1;
    char *last_textarea_name = NULL;
    int textarea_lineno = 0;
    char *previous_blanks = NULL;
    BOOLEAN PlainText = FALSE;
    BOOLEAN SemiColon = FALSE;
    char *Boundary = NULL;
    char *MultipartContentType = NULL;
    int target_cs = -1;
    CONST char *out_csname;
    CONST char *target_csname = NULL;
    char *name_used = "";
    BOOL form_has_8bit = NO, form_has_special = NO;
    BOOL field_has_8bit = NO, field_has_special = NO;
    BOOL name_has_8bit = NO, name_has_special = NO;
    BOOL have_accept_cs = NO;
    BOOL success;
    BOOL had_chartrans_warning = NO;
    char *val_used = "";
    char *copied_val_used = NULL;
    char *copied_name_used = NULL;

    if (!HTMainText)
	return;

    thisform = HTList_objectAt(HTMainText->forms, form_number - 1);
    /*  Sanity check */
    if (!thisform) {
	CTRACE(tfp, "SubmitForm: form %d not in HTMainText's list!\n",
		    form_number);
    } else if (thisform->number != form_number) {
	CTRACE(tfp, "SubmitForm: failed sanity check, %d!=%d !\n",
		    thisform->number, form_number);
	thisform = NULL;
    }

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
     *  Determine in what character encoding (aka. charset) we should
     *  submit.  We call this target_cs and the MIME name for it
     *  target_csname.
     *  TODO:   - actually use ACCEPT-CHARSET stuff from FORM
     *  TODO:   - deal with list in ACCEPT-CHARSET, find a "best"
     *		  charset to submit
     */

    /* Look at ACCEPT-CHARSET on the submitting field if we have one. */
    if (thisform && submit_item->accept_cs &&
	strcasecomp(submit_item->accept_cs, "UNKNOWN")) {
	have_accept_cs = YES;
	target_cs = find_best_target_cs(&thisform->thisacceptcs,
					current_char_set,
					submit_item->accept_cs);
    }
    /* Look at ACCEPT-CHARSET on form as a whole if submitting field
     * didn't have one. */
    if (thisform && !have_accept_cs && thisform->accept_cs &&
	strcasecomp(thisform->accept_cs, "UNKNOWN")) {
	have_accept_cs = YES;
	target_cs = find_best_target_cs(&thisform->thisacceptcs,
					current_char_set,
					thisform->accept_cs);
    }
    if (have_accept_cs && (target_cs >= 0) && thisform->thisacceptcs) {
	target_csname = thisform->thisacceptcs;
    }

    if (target_cs < 0 &&
	HTMainText->node_anchor->charset &&
	*HTMainText->node_anchor->charset) {
	target_cs = UCGetLYhndl_byMIME(HTMainText->node_anchor->charset);
	if (target_cs >= 0) {
	    target_csname = HTMainText->node_anchor->charset;
	} else {
	    target_cs = UCLYhndl_for_unspec;
	    if (target_cs >= 0)
		target_csname = LYCharSet_UC[target_cs].MIMEname;
	}
    }
    if (target_cs < 0) {
	target_cs = UCLYhndl_for_unspec;
    }

    /*
     *  Go through list of anchors and get size first.
     */
    /*
     *  also get a "max." charset parameter - kw
     */
    anchor_ptr = HTMainText->first_anchor;
    while (anchor_ptr) {
	if (anchor_ptr->link_type == INPUT_ANCHOR) {
	    if (anchor_ptr->input_field->number == form_number) {

		char *p;
		char * val;
	        form_ptr = anchor_ptr->input_field;
		val = form_ptr->cp_submit_value != NULL ?
			            form_ptr->cp_submit_value : form_ptr->value;
		field_has_8bit = NO;
		field_has_special = NO;

	        len += (strlen(form_ptr->name) + (Boundary ? 100 : 10));
		/*
		 *  Calculate by the option submit value if present.
		 */
		if (form_ptr->cp_submit_value != NULL) {
		    len += (strlen(form_ptr->cp_submit_value) + 10);
		} else {
	            len += (strlen(form_ptr->value) + 10);
		}
	        len += 32; /* plus and ampersand + safety net */

		for (p = val;
		     p && *p && !(field_has_8bit && field_has_special);
		     p++)
		    if ((*p == HT_NON_BREAK_SPACE) ||
			(*p == HT_EM_SPACE) ||
			(*p == LY_SOFT_HYPHEN)) {
			field_has_special = YES;
		    } else if ((*p & 0x80) != 0) {
			field_has_8bit = YES;
		    }
		for (p = form_ptr->name;
		     p && *p && !(name_has_8bit && name_has_special);
		     p++)
		    if ((*p == HT_NON_BREAK_SPACE) ||
			(*p == HT_EM_SPACE) ||
			(*p == LY_SOFT_HYPHEN)) {
			name_has_special = YES;
		    } else if ((*p & 0x80) != 0) {
			name_has_8bit = YES;
		    }

		if (field_has_8bit || name_has_8bit)
		    form_has_8bit = YES;
		if (field_has_special || name_has_special)
		    form_has_special = YES;

		if (!field_has_8bit && !field_has_special) {
		    /* already ok */
		} else if (target_cs < 0) {
		    /* already confused */
		} else if (!field_has_8bit &&
		    (LYCharSet_UC[target_cs].enc == UCT_ENC_8859 ||
		     (LYCharSet_UC[target_cs].like8859 & UCT_R_8859SPECL))) {
		    /* those specials will be trivial */
		} else if (UCNeedNotTranslate(form_ptr->value_cs, target_cs)) {
		    /* already ok */
		} else if (UCCanTranslateFromTo(form_ptr->value_cs, target_cs)) {
		    /* also ok */
		} else if (UCCanTranslateFromTo(target_cs, form_ptr->value_cs)) {
		    target_cs = form_ptr->value_cs;	/* try this */
		    target_csname = NULL; /* will be set after loop */
		} else {
		    target_cs = -1; /* don't know what to do */
		}

		/*  Same for name */
		if (!name_has_8bit && !name_has_special) {
		    /* already ok */
		} else if (target_cs < 0) {
		    /* already confused */
		} else if (!name_has_8bit &&
		    (LYCharSet_UC[target_cs].enc == UCT_ENC_8859 ||
		     (LYCharSet_UC[target_cs].like8859 & UCT_R_8859SPECL))) {
		    /* those specials will be trivial */
		} else if (UCNeedNotTranslate(form_ptr->name_cs, target_cs)) {
		    /* already ok */
		} else if (UCCanTranslateFromTo(form_ptr->name_cs, target_cs)) {
		    /* also ok */
		} else if (UCCanTranslateFromTo(target_cs, form_ptr->name_cs)) {
		    target_cs = form_ptr->value_cs;	/* try this */
		    target_csname = NULL; /* will be set after loop */
		} else {
		    target_cs = -1; /* don't know what to do */
		}

	    } else if (anchor_ptr->input_field->number > form_number) {
	        break;
	    }
	}

	if (anchor_ptr == HTMainText->last_anchor)
	    break;

	anchor_ptr = anchor_ptr->next;
    }

    if (target_csname == NULL && target_cs >= 0) {
	if (form_has_8bit) {
	    target_csname = LYCharSet_UC[target_cs].MIMEname;
	} else if (form_has_special) {
	    target_csname = LYCharSet_UC[target_cs].MIMEname;
	} else {
	    target_csname = "us-ascii";
	}
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
	 *  If the ENCTYPE is not multipart/form-data, append the
	 *  charset we'll be sending to the post_content_type, IF
	 *  (1) there was an explicit accept-charset attribute, OR
	 *  (2) we have 8-bit or special chars, AND the document had
	 *      an explicit (recognized and accepted) charset parameter,
	 *	AND it or target_csname is different from iso-8859-1,
	 *      OR
	 *  (3) we have 8-bit or special chars, AND the document had
	 *      no explicit (recognized and accepted) charset parameter,
	 *	AND target_cs is different from the currently effective
	 *	assumed charset (which should have been set by the user
	 *	so that it reflects what the server is sending, if the
	 *	document is rendered correctly).
	 *  For multipart/form-data the equivalent will be done later,
	 *  separately for each form field. - kw
	 */
	if (have_accept_cs ||
	    (form_has_8bit || form_has_special)) {
	    if (target_cs >= 0 && target_csname) {
		if (Boundary == NULL) {
		    if ((HTMainText->node_anchor->charset &&
			 (strcmp(HTMainText->node_anchor->charset,
				 "iso-8859-1") ||
			  strcmp(target_csname, "iso-8859-1"))) ||
			(!HTMainText->node_anchor->charset &&
			 target_cs != UCLYhndl_for_unspec)) {
			StrAllocCat(doc->post_content_type, "; charset=");
			StrAllocCat(doc->post_content_type, target_csname);
		    }
		}
	    } else {
		had_chartrans_warning = YES;
		_user_message(
		    CANNOT_TRANSCODE_FORM,
		    target_csname ? target_csname : "UNKNOWN");
		sleep(AlertSecs);
	    }
	}
    }


    out_csname = target_csname;

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
		char *p;
		int out_cs;
		form_ptr = anchor_ptr->input_field;

		if (form_ptr->type != F_TEXTAREA_TYPE)
		    textarea_lineno = 0;

		switch(form_ptr->type) {
	        case F_RESET_TYPE:
		    break;
	        case F_SUBMIT_TYPE:
	        case F_TEXT_SUBMIT_TYPE:
	        case F_IMAGE_SUBMIT_TYPE:
		    if (!(form_ptr->name && *form_ptr->name != '\0' &&
			  !strcmp(form_ptr->name, link_name))) {
			CTRACE(tfp,
				    "SubmitForm: skipping submit field with ");
			CTRACE(tfp, "name \"%s\" for link_name \"%s\", %s.",
				    form_ptr->name ? form_ptr->name : "???",
				    link_name ? link_name : "???",
				    (form_ptr->name && *form_ptr->name) ?
				    "not current link" : "no field name");
			break;
		    }
		    if (!(form_ptr->type == F_TEXT_SUBMIT_TYPE ||
			(form_ptr->value && *form_ptr->value != '\0' &&
			 !strcmp(form_ptr->value, link_value)))) {
			if (TRACE) {
			    fprintf(tfp,
				    "SubmitForm: skipping submit field with ");
			    fprintf(tfp,
				    "name \"%s\" for link_name \"%s\", %s!",
				    form_ptr->name ? form_ptr->name : "???",
				    link_name ? link_name : "???",
				    "values are different");
			}
			break;
		    }
		    /*  fall through  */
	        case F_RADIO_TYPE:
		case F_CHECKBOX_TYPE:
		case F_TEXTAREA_TYPE:
		case F_PASSWORD_TYPE:
	        case F_TEXT_TYPE:
		case F_OPTION_LIST_TYPE:
		case F_HIDDEN_TYPE:
		    /*
		     *	Be sure to actually look at the option submit value.
		     */
		    if (form_ptr->cp_submit_value != NULL) {
			val_used = form_ptr->cp_submit_value;
		    } else {
			val_used = form_ptr->value;
		    }

		    /*
		     *  Charset-translate value now, because we need
		     *  to know the charset parameter for multipart
		     *  bodyparts. - kw
		     */
		    field_has_8bit = NO;
		    field_has_special = NO;
		    for (p = val_used;
			 p && *p && !(field_has_8bit && field_has_special);
			 p++) {
			if ((*p == HT_NON_BREAK_SPACE) ||
			    (*p == HT_EM_SPACE) ||
			    (*p == LY_SOFT_HYPHEN)) {
			    field_has_special = YES;
			} else if ((*p & 0x80) != 0) {
			    field_has_8bit = YES;
			}
		    }

		    if (field_has_8bit || field_has_special) {
			/*  We should translate back. */
			StrAllocCopy(copied_val_used, val_used);
			success = LYUCTranslateBackFormData(&copied_val_used,
							form_ptr->value_cs,
							target_cs, PlainText);
			CTRACE(tfp, "SubmitForm: field \"%s\" %d %s -> %d %s %s\n",
				    form_ptr->name ? form_ptr->name : "",
				    form_ptr->value_cs,
				    form_ptr->value_cs >= 0 ?
				    LYCharSet_UC[form_ptr->value_cs].MIMEname :
									  "???",
				    target_cs,
				    target_csname ? target_csname : "???",
				    success ? "OK" : "FAILED");
			if (success) {
			    val_used = copied_val_used;
			}
		    } else {  /* We can use the value directly. */
			CTRACE(tfp, "SubmitForm: field \"%s\" %d %s OK\n",
				    form_ptr->name ? form_ptr->name : "",
				    target_cs,
				    target_csname ? target_csname : "???");
			success = YES;
		    }
		    if (!success) {
			if (!had_chartrans_warning) {
			    had_chartrans_warning = YES;
			    _user_message(
				CANNOT_TRANSCODE_FORM,
				target_csname ? target_csname : "UNKNOWN");
			    sleep(AlertSecs);
			}
			out_cs = form_ptr->value_cs;
		    } else {
			out_cs = target_cs;
		    }
		    if (out_cs >= 0)
			out_csname = LYCharSet_UC[out_cs].MIMEname;
		    if (Boundary) {
			if (!success && form_ptr->value_cs < 0) {
			    /*  This is weird. */
			    StrAllocCopy(MultipartContentType,
					 "\r\nContent-Type: text/plain; charset=");
			    StrAllocCat(MultipartContentType, "UNKNOWN-8BIT");
			} else if (!success) {
			    StrAllocCopy(MultipartContentType,
					 "\r\nContent-Type: text/plain; charset=");
			    StrAllocCat(MultipartContentType, out_csname);
			    target_csname = NULL;
			} else {
			    if (!target_csname) {
				target_csname = LYCharSet_UC[target_cs].MIMEname;
			    }
			    StrAllocCopy(MultipartContentType,
					 "\r\nContent-Type: text/plain; charset=");
			    StrAllocCat(MultipartContentType, out_csname);
			}
		    }

		    /*
		     *  Charset-translate name now, because we need
		     *  to know the charset parameter for multipart
		     *  bodyparts. - kw
		     */
		    if (form_ptr->type == F_TEXTAREA_TYPE) {
			textarea_lineno++;
			if (textarea_lineno > 1 &&
			    last_textarea_name && form_ptr->name &&
			    !strcmp(last_textarea_name, form_ptr->name)) {
			    break;
			}
		    }
		    name_used = (form_ptr->name ?
				 form_ptr->name : "");

		    name_has_8bit = NO;
		    name_has_special = NO;
		    for (p = name_used;
			 p && *p && !(name_has_8bit && name_has_special);
			 p++) {
			if ((*p == HT_NON_BREAK_SPACE) ||
			    (*p == HT_EM_SPACE) ||
			    (*p == LY_SOFT_HYPHEN)) {
			    name_has_special = YES;
			} else if ((*p & 0x80) != 0) {
			    name_has_8bit = YES;
			}
		    }

		    if (name_has_8bit || name_has_special) {
			/*  We should translate back. */
			StrAllocCopy(copied_name_used, name_used);
			success = LYUCTranslateBackFormData(&copied_name_used,
							form_ptr->name_cs,
							target_cs, PlainText);
			CTRACE(tfp, "SubmitForm: name \"%s\" %d %s -> %d %s %s\n",
				    form_ptr->name ? form_ptr->name : "",
				    form_ptr->name_cs,
				    form_ptr->name_cs >= 0 ?
				    LYCharSet_UC[form_ptr->name_cs].MIMEname :
									  "???",
				    target_cs,
				    target_csname ? target_csname : "???",
				    success ? "OK" : "FAILED");
			if (success) {
			    name_used = copied_name_used;
			}
			if (Boundary) {
			    if (!success) {
				StrAllocCopy(MultipartContentType, "");
				target_csname = NULL;
			    } else {
				if (!target_csname)
				    target_csname = LYCharSet_UC[target_cs].MIMEname;
			    }
			}
		    } else {  /* We can use the name directly. */
			CTRACE(tfp, "SubmitForm: name \"%s\" %d %s OK\n",
				    form_ptr->name ? form_ptr->name : "",
				    target_cs,
				    target_csname ? target_csname : "???");
			success = YES;
			if (Boundary) {
			    StrAllocCopy(copied_name_used, name_used);
			}
		    }
		    if (!success) {
			if (!had_chartrans_warning) {
			    had_chartrans_warning = YES;
			    _user_message(
				CANNOT_TRANSCODE_FORM,
				target_csname ? target_csname : "UNKNOWN");
			    sleep(AlertSecs);
			}
		    }
		    if (Boundary) {
			/*
			 *  According to RFC 1867, Non-ASCII field names
			 *  "should be encoded according to the prescriptions
			 *  of RFC 1522 [...].  I don't think RFC 1522 actually
			 *  is meant to apply to parameters like this, and it
			 *  is unknown whether any server would make sense of
			 *  it, so for now just use some quoting/escaping and
			 *  otherwise leave 8-bit values as they are.
			 *  Non-ASCII characters in form field names submitted
			 *  as multipart/form-data can only occur if the form
			 *  provider specifically asked for it anyway. - kw
			 */
			HTMake822Word(&copied_name_used);
			name_used = copied_name_used;
		    }

		    break;
		default:
		    CTRACE(tfp, "SubmitForm: What type is %d?\n",
				form_ptr->type);
		}

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
			int cdisp_name_startpos = 0;
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
			    StrAllocCopy(escaped1, name_used);
			} else if (Boundary) {
			    StrAllocCopy(escaped1,
				    "Content-Disposition: form-data; name=");
			    cdisp_name_startpos = strlen(escaped1);
			    StrAllocCat(escaped1, name_used);
			    if (MultipartContentType)
				StrAllocCat(escaped1, MultipartContentType);
			    StrAllocCat(escaped1, "\r\n\r\n");
			} else {
			    escaped1 = HTEscapeSP(name_used, URL_XALPHAS);
			}

			if (PlainText || Boundary) {
			    StrAllocCopy(escaped2,
					 (val_used ?
					  val_used : ""));
			} else {
			    escaped2 = HTEscapeSP(val_used, URL_XALPHAS);
			}

			if (form_ptr->type == F_IMAGE_SUBMIT_TYPE) {
			    /*
			     *  It's a clickable image submit button.
			     *  Fake a 0,0 coordinate pair, which
			     *  typically returns the image's default. - FM
			     */
			    if (Boundary) {
				escaped1[cdisp_name_startpos] = '\0';
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
		    FREE(copied_name_used);
		    FREE(copied_val_used);
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
			    StrAllocCopy(escaped1, name_used);
			} else if (Boundary) {
			    StrAllocCopy(escaped1,
				     "Content-Disposition: form-data; name=");
			    StrAllocCat(escaped1,
					name_used);
			    if (MultipartContentType)
				StrAllocCat(escaped1, MultipartContentType);
			    StrAllocCat(escaped1, "\r\n\r\n");
			} else {
			    escaped1 = HTEscapeSP(name_used, URL_XALPHAS);
			}
			if (PlainText || Boundary) {
			    StrAllocCopy(escaped2,
					 (val_used ?
					  val_used : ""));
			} else {
			    escaped2 = HTEscapeSP(val_used, URL_XALPHAS);
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
		    FREE(copied_name_used);
		    FREE(copied_val_used);
		    break;

		case F_TEXTAREA_TYPE:
		    if (PlainText || Boundary) {
			StrAllocCopy(escaped2,
				     (val_used ?
				      val_used : ""));
		    } else {
			escaped2 = HTEscapeSP(val_used, URL_XALPHAS);
		    }

		    if (!last_textarea_name ||
			strcmp(last_textarea_name, form_ptr->name)) {
			textarea_lineno = 1;
			/*
			 *  Names are different so this is the first
			 *  textarea or a different one from any before
			 *  it.
			 */
			if (Boundary) {
			    StrAllocCopy(previous_blanks, "\r\n");
			} else {
			    FREE(previous_blanks);
			}
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
			    StrAllocCopy(escaped1, name_used);
			} else if (Boundary) {
			    StrAllocCopy(escaped1,
				    "Content-Disposition: form-data; name=");
			    StrAllocCat(escaped1, name_used);
			    if (MultipartContentType)
				StrAllocCat(escaped1, MultipartContentType);
			    StrAllocCat(escaped1, "\r\n\r\n");
			} else {
			    escaped1 = HTEscapeSP(name_used, URL_XALPHAS);
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
		    FREE(copied_val_used);
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
		       StrAllocCopy(escaped1, name_used);
		    } else if (Boundary) {
			StrAllocCopy(escaped1,
				    "Content-Disposition: form-data; name=");
			StrAllocCat(escaped1, name_used);
			if (MultipartContentType)
			    StrAllocCat(escaped1, MultipartContentType);
			StrAllocCat(escaped1, "\r\n\r\n");
		    } else {
			escaped1 = HTEscapeSP(name_used, URL_XALPHAS);
		    }

		    if (PlainText || Boundary) {
			StrAllocCopy(escaped2,
				     (val_used ?
				      val_used : ""));
		    } else {
			escaped2 = HTEscapeSP(val_used, URL_XALPHAS);
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
		    FREE(copied_name_used);
		    FREE(copied_val_used);
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
    FREE(copied_name_used);
    if (Boundary) {
	sprintf(&query[strlen(query)], "\r\n--%s--\r\n", Boundary);
    }
    FREE(previous_blanks);

    if (submit_item->submit_method == URL_MAIL_METHOD) {
	_user_message("Submitting %s", submit_item->submit_action);
	CTRACE(tfp, "\nGridText - mailto_address: %s\n",
			    (submit_item->submit_action+7));
	CTRACE(tfp, "GridText - mailto_subject: %s\n",
			    ((submit_item->submit_title &&
			      *submit_item->submit_title) ?
			      (submit_item->submit_title) :
					(HText_getTitle() ?
				         HText_getTitle() : "")));
	CTRACE(tfp,"GridText - mailto_content: %s\n",query);
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
	CTRACE(tfp,"GridText - post_data: %s\n",doc->post_data);
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
    FREE(HTFormAcceptCharset);
    PerFormInfo_free(HTCurrentForm);

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

PUBLIC BOOL HText_hasUTF8OutputSet ARGS1(
	HText *,	text)
{
    return ((text && text->T.output_utf8) ? TRUE : FALSE);
}

/*
**  Check charset and set the kcode element. - FM
**  Info on the input charset may be passed in in two forms,
**  as a string (if given explicitly) and as a pointer to
**  a LYUCcharset (from chartrans mechanism); either can be NULL.
**  For Japanese the kcode will be reset at a space or explicit
**  line or paragraph break, so what we set here may not last for
**  long.  It's potentially more important not to set HTCJK to
**  NOCJK unless we are sure. - kw
*/
PUBLIC void HText_setKcode ARGS3(
	HText *,	text,
	CONST char *,	charset,
	LYUCcharset *,	p_in)
{
    if (!text)
	return;

    /*
    **  Check whether we have some kind of info. - kw
    */
    if (!charset && !p_in) {
	return;
    }
    /*
    **  If no explicit charset string, use the implied one. - kw
    */
    if (!charset || *charset == '\0') {
	charset = p_in->MIMEname;
    }
    /*
    **  Check whether we have a specified charset. - FM
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
    if (!strcmp(charset, "shift_jis") ||
	!strcmp(charset, "x-shift-jis")) {
	text->kcode = SJIS;
    } else if ((p_in && (p_in->enc == UCT_ENC_CJK)) ||
	       !strcmp(charset, "euc-jp") ||
	       !strncmp(charset, "x-euc-", 6) ||
	       !strcmp(charset, "iso-2022-jp") ||
	       !strcmp(charset, "iso-2022-jp-2") ||
	       !strcmp(charset, "euc-kr") ||
	       !strcmp(charset, "iso-2022-kr") ||
	       !strcmp(charset, "big5") ||
	       !strcmp(charset, "cn-big5") ||
	       !strcmp(charset, "euc-cn") ||
	       !strcmp(charset, "gb2312") ||
	       !strncmp(charset, "cn-gb", 5) ||
	       !strcmp(charset, "iso-2022-cn")) {
	text->kcode = EUC;
    } else {
	/*
	**  If we get to here, it's not CJK, so disable that if
	**  it is enabled.  But only if we are quite sure. - FM & kw
	*/
	text->kcode = NOKANJI;
	if (HTCJK != NOCJK) {
	    if (!p_in || p_in->enc != UCT_ENC_CJK)
		HTCJK = NOCJK;
	}
    }

    return;
}

/*
**  Set a permissible split at the current end of the last line. - FM
*/
PUBLIC void HText_setBreakPoint ARGS1(
	HText *,	text)
{
    if (!text)
	return;

    /*
     *  Can split here. - FM
     */
    text->permissible_split = (int)text->last_line->size;

    return;
}

/*
**  This function determines whether a document which
**  would be sought via the a URL that has a fragment
**  directive appended is otherwise identical to the
**  currently loaded document, and if so, returns
**  FALSE, so that any no_cache directives can be
**  overridden "safely", on the grounds that we are
**  simply acting on the equivalent of a paging
**  command.  Otherwise, it returns TRUE, i.e, that
**  the target document might differ from the current,
**  based on any caching directives or analyses which
**  claimed or suggested this. - FM
*/
PUBLIC BOOL HText_AreDifferent ARGS2(
	HTParentAnchor *,	anchor,
	CONST char *,		full_address)
{
    HTParentAnchor *MTanc;
    char *MTaddress;
    char *MTpound;
    char *TargetPound;

    /*
     *  Do we have a loaded document and both
     *  arguments for this function?
     */
    if (!(HTMainText && anchor && full_address))
	return TRUE;

    /*
     *  Do we have both URLs?
     */
    MTanc = HTMainText->node_anchor;
    if (!(MTanc->address && anchor->address))
	return (TRUE);

    /*
     *  Do we have a fragment associated with the target?
     */
    if ((TargetPound = strchr(full_address, '#')) == NULL)
	return (TRUE);

    /*
     *  Always treat client-side image map menus
     *  as potentially stale, so we'll create a
     *  fresh menu from the LynxMaps HTList.
     */
    if (!strncasecomp(anchor->address, "LYNXIMGMAP:", 11))
	return (TRUE);

    /*
     *  Do the docs differ in the type of request?
     */
    if (MTanc->isHEAD != anchor->isHEAD)
	return (TRUE);

    /*
     *  Are the actual URLs different, after factoring
     *  out a "LYNXIMGMAP:" leader in the MainText URL
     *  and its fragment, if present?
     */
    MTaddress = (strncasecomp(MTanc->address,
			      "LYNXIMGMAP:", 11) ?
				  MTanc->address : (MTanc->address + 11));
    if ((MTpound = strchr(MTaddress, '#')) != NULL)
	*MTpound = '\0';
    if (strcmp(MTaddress, anchor->address)) {
	if (MTpound != NULL) {
	    *MTpound = '#';
	}
	return(TRUE);
    }
    if (MTpound != NULL) {
	*MTpound = '#';
    }

    /*
     *  If the MainText is not an image map menu,
     *  do the docs have different POST contents?
     */
    if (MTaddress == MTanc->address) {
	if (MTanc->post_data) {
	    if (anchor->post_data) {
		if (strcmp(MTanc->post_data, anchor->post_data)) {
		    /*
		     *  Both have contents, and they differ.
		     */
		    return(TRUE);
		}
	    } else {
		/*
		 *  The loaded document has content, but the
		 *  target doesn't, so they're different.
		 */
		return(TRUE);
	    }
	} else if (anchor->post_data) {
	    /*
	     *  The loaded document does not have content, but
	     *  the target does, so they're different.
	     */
	    return(TRUE);
	}
    }

    /*
     *  We'll assume the target is a position in the currently
     *  displayed document, and thus can ignore any header, META,
     *  or other directives not to use a cached rendition. - FM
     */
    return(FALSE);
}
