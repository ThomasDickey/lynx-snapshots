#include "HTUtils.h"
#include "tcp.h"
#include "HTCJK.h"
#include "HTTP.h"
#include "HTAlert.h"
#include "LYCurses.h"
#include "GridText.h"
#include "LYUtils.h"
#include "LYStructs.h"  /* includes HTForms.h */
#include "LYStrings.h"
#include "LYGlobalDefs.h"
#include "LYKeymap.h"
#include "LYSignal.h"

#include "LYLeaks.h"

#ifndef BOXVERT
#define BOXVERT '*'	/* character for popup window vertical borders */
#endif
#ifndef BOXHORI
#define BOXHORI '*'	/* character for popup window horizontal borders */
#endif

extern HTCJKlang HTCJK;

PRIVATE int form_getstr PARAMS((struct link * form_link));
PRIVATE int popup_options PARAMS((int cur_selection, OptionType *list, 
				  int ly, int lx, int width,
				  int i_length, int disabled));

PUBLIC int change_form_link ARGS6(struct link *, form_link, int, mode, 
				  document *,newdoc, BOOLEAN *,refresh_screen,
				  char *,link_name, char *,link_value)
{
    extern BOOL reloading;
    FormInfo *form = form_link->form;
    int c=DO_NOTHING;

	/*
	 *  If there is no form to perform action on, don't do anything.
	 */
	if (form == NULL) {
	    return(c);
	}

    /* move to the link position */
    move(form_link->ly, form_link->lx);

    switch(form->type) {
	case F_CHECKBOX_TYPE:
	    if (form->disabled == YES)
	        break;
	    if (form->num_value) {
		form_link->hightext = unchecked_box;
		form->num_value = 0;
	    } else {
		form_link->hightext = checked_box;
		form->num_value = 1;
	    }
	    break;

	case F_OPTION_LIST_TYPE:
	    if (!form->select_list) {
	        HTAlert(BAD_HTML_NO_POPUP);
		c = DO_NOTHING;
		break;
	    }

	    if (form->disabled == YES) {
	        int dummy;
		dummy = popup_options(form->num_value, form->select_list,
				form_link->ly, form_link->lx, form->size,
				form->size_l, form->disabled);
	        c = 12;  /* CTRL-R for repaint */
	        break;
	    }
	    form->num_value = popup_options(form->num_value, form->select_list,
				form_link->ly, form_link->lx, form->size,
				form->size_l, form->disabled);

	    {
    	        OptionType * opt_ptr=form->select_list;
		int i;
    	        for (i = 0; i < form->num_value; i++, opt_ptr = opt_ptr->next) 
		    ; /* null body */
		form->value = opt_ptr->name;   /* set the name */
		form->cp_submit_value = opt_ptr->cp_submit_value; /* set the value */
	    }
	    c = 12;  /* CTRL-R for repaint */
	    break;

	case F_RADIO_TYPE:
	    if (form->disabled == YES)
	        break;
		/* radio buttons must have one and
		 * only one down at a time! 
		 */
	    if (form->num_value) {
		_statusline(NEED_CHECKED_RADIO_BUTTON);
		sleep(MessageSecs);

	    } else {	
		int i;
		/* run though list of the links on the screen and
		 * unselect any that are selected. :) 
		 */
		start_bold();
		for (i = 0; i < nlinks; i++)
		   if (links[i].type == WWW_FORM_LINK_TYPE &&
		       links[i].form->type == F_RADIO_TYPE &&
		        links[i].form->number == form->number &&
   		         /* if it has the same name and its on */
                          !strcmp(links[i].form->name, form->name) &&
                            links[i].form->num_value ) 
		     {
			move(links[i].ly, links[i].lx);
			addstr(unchecked_radio);
			links[i].hightext = unchecked_radio;
		     }
		stop_bold();
		/* will unselect other button and select this one */
		HText_activateRadioButton(form);
		/* now highlight this one */
		form_link->hightext = checked_radio;
	    }
	    break;

	case F_TEXT_TYPE:
	case F_TEXTAREA_TYPE:
	case F_PASSWORD_TYPE:
	    c = form_getstr(form_link);
	    if (form->type == F_PASSWORD_TYPE) 
        	form_link->hightext = STARS(strlen(form->value));
	    else
	    	form_link->hightext = form->value;
	    break;

	case F_RESET_TYPE:
	    if (form->disabled == YES)
	        break;
	    HText_ResetForm(form);
            *refresh_screen = TRUE;
	    break;
	
	case F_TEXT_SUBMIT_TYPE:
	    c = form_getstr(form_link);
	    if (form->disabled == YES &&
	        (c == '\r' || c == '\n')) {
	        c = '\t';
		break;
	    }
	    if (c == '\r' || c == '\n') {
	        form_link->hightext = form->value;
		if (!form->submit_action || *form->submit_action == '\0') {
		    _statusline(NO_FORM_ACTION);
		    sleep(MessageSecs);
		    c = DO_NOTHING;
		    break;
		} else if (form->submit_method == URL_MAIL_METHOD && no_mail) {
		    HTAlert(FORM_MAILTO_DISALLOWED);
		    c = DO_NOTHING;
		    break;
		} else {
		    if (form->no_cache &&
		        form->submit_method != URL_MAIL_METHOD) {
		        LYforce_no_cache = TRUE;
			reloading = TRUE;
		    }
	            HText_SubmitForm(form, newdoc, link_name, form->value);
		}
		if (form->submit_method == URL_MAIL_METHOD)
		    *refresh_screen = TRUE;
		else
	            /* returns new document URL */
	            newdoc->link = 0;
		c = DO_NOTHING;
		break;
	    } else {
	    	form_link->hightext = form->value;
	    }
	    break;

	case F_SUBMIT_TYPE:
	case F_IMAGE_SUBMIT_TYPE:
	    if (form->disabled == YES)
	        break;
	    if (form->no_cache &&
	        form->submit_method != URL_MAIL_METHOD) {
		LYforce_no_cache = TRUE;
		reloading = TRUE;
	    }
	    HText_SubmitForm(form, newdoc, link_name, link_value);
	    if (form->submit_method == URL_MAIL_METHOD)
                *refresh_screen = TRUE;
	    else
	        /* returns new document URL */
	        newdoc->link = 0;
	    break;

    }

    return(c);

} 

#ifdef USE_SLANG
#define GetYX(y,x)   y = SLsmg_get_row(), x = SLsmg_get_column()
#else
#ifdef getyx
#define GetYX(y,x)   getyx(stdscr,y,x)
#else
#define GetYX(y,x)   y = stdscr->_cury, x = stdscr->_curx
#endif /* getyx */
#endif /* USE_SLANG */

PRIVATE int form_getstr ARGS1(struct link *, form_link)
{
    FormInfo *form = form_link->form;
    int ch;
    int far_col;
    int max_length = (form->maxlength ? form->maxlength : 1023);
    int startcol, startline;
    BOOL HaveMaxlength = FALSE;

#ifdef VMS
    extern BOOLEAN HadVMSInterrupt;/* Flag from cleanup_sig() AST       */
#endif

    EditFieldData MyEdit;
    BOOLEAN Edited = FALSE;       /* Value might be updated? */

    /* get the initial position of the cursor */
    GetYX(startline, startcol);

    if (startcol + form->size > LYcols-1)
	far_col = LYcols-1;
    else
	far_col = startcol + form->size;

    /* Print panned line */

    LYSetupEdit(&MyEdit, form->value, max_length, far_col-startcol);
    MyEdit.pad='_';
    MyEdit.hidden = (form->type == F_PASSWORD_TYPE);
    LYRefreshEdit(&MyEdit);

    /* And go for it.. */

    for (;;) {
again:
        ch = LYgetch();
#ifdef VMS
        if (HadVMSInterrupt) {
	    HadVMSInterrupt = FALSE;
	    ch = 7;
	}
#endif /* VMS */

	/*
	 * Filter out global navigation keys that should not be passed
	 * to line editor, and LYK_REFRESH.
	 */
	if (EditBinding(ch) == LYE_ENTER)
	    break;
	if (EditBinding(ch) == LYE_AIX && HTCJK == NOCJK)
	    break;
        if (EditBinding(ch) == LYE_TAB)
	  {
	    ch = (int)('\t');
	    break;
	  }
	if (EditBinding(ch) == LYE_ABORT)
	    return(DO_NOTHING);
	if (keymap[ch + 1] == LYK_REFRESH)
	    goto breakfor;
	switch (ch) {
	    case DNARROW:
	    case UPARROW:
	    case PGUP:
	    case PGDOWN:
#ifdef NOTDEFINED
	    case HOME:
	    case END:
	    case FIND_KEY:
	    case SELECT_KEY:
#endif /* NOTDEFINED */
                goto breakfor;

	    /*
	     * Left arrrow in column 0 deserves special treatment here,
	     * else you can get trapped in a form without submit button!
	     */
	    case LTARROW:
	        if (MyEdit.pos==0) {
		    int c='Y';	/* Go back immediately if no changes */
		    if (strcmp(MyEdit.buffer, form->value)) {
		        _statusline(PREV_DOC_QUERY);
			c=LYgetch();
		    }
		    if (TOUPPER(c) == 'Y') {
#ifdef NOTDEFINED
			/*
			 * Why not just keep what we have??
			 * We don't erase the other fields either??
			 */
			StrAllocCopy(form->value, "");
#endif /* NOTDEFINED */
			return(ch);
		    } else {
		        if (form->disabled == YES)
		            _statusline(ARROWS_OR_TAB_TO_MOVE);
			else
			    _statusline(ENTER_TEXT_ARROWS_OR_TAB);
		    }
		}
		/* fall through */

	    default:
		if (form->disabled == YES)
		    goto again;
		/* Make sure the statusline uses editmode help... */
		Edited = TRUE;
		LYLineEdit(&MyEdit,ch,TRUE);
		if (MyEdit.strlen >= max_length) {
		    HaveMaxlength = TRUE;
		} else if (HaveMaxlength &&
			   MyEdit.strlen < max_length) {
		    HaveMaxlength = FALSE;
		    _statusline(ENTER_TEXT_ARROWS_OR_TAB);
		}
		LYRefreshEdit(&MyEdit);
	}
    }
breakfor:
    if (Edited) {
        char  *p;
	StrAllocCopy(form->value, MyEdit.buffer);

	/*
	 * Remove trailing spaces
	 *
	 * Do we really need to do that here? Trailing spaces will only
	 * be there if user keyed them in. Rather rude to throw away
	 * their hard earned spaces. Better deal with trailing spaces
	 * when submitting the form????
	 */
	p = &(form->value[strlen(form->value)]);
	while ((p != form->value) && (p[-1]==' '))
	    p--;
	*p = '\0';
    }
    return(ch);
}


PRIVATE int popup_options ARGS7(
	int,		cur_selection,
	OptionType *,	list,
	int,		ly,
	int,		lx,
	int,		width,
	int,		i_length,
	int,		disabled)
{
    /*
     *  Revamped to handle within-tag VALUE's, if present,
     *  and to position the popup window appropriately,
     *  taking the user_mode setting into account. -- FM
     */
    int c = 0, cmd = 0, i = 0, j = 0;
    int orig_selection = cur_selection;
#ifndef USE_SLANG
    WINDOW * form_window;
#endif /* !USE_SLANG */
    int num_options = 0, top, bottom, length = -1;
    OptionType * opt_ptr = list;
    int window_offset = 0;
    int display_lines;
#ifdef VMS
    extern BOOLEAN HadVMSInterrupt; /* Flag from cleanup_sig() AST */
#endif /* VMS */
    static char prev_target[512]; 		/* Search string buffer */
    static char prev_target_buffer[512];	/* Next search buffer */
    static BOOL first = TRUE;
    char *cp;
    int ch = 0, recall;
    int QueryTotal;
    int QueryNum;
    BOOLEAN FirstRecall = TRUE;
    OptionType * tmp_ptr;
    BOOLEAN ReDraw = FALSE;

    /*
     * Initialize the search string buffer. - FM
     */
    if (first) {
	*prev_target_buffer = '\0';
	first = FALSE;
    }
    *prev_target = '\0';
    QueryTotal = (search_queries ? HTList_count(search_queries) : 0);
    recall = ((QueryTotal >= 1) ? RECALL : NORECALL);
    QueryNum = QueryTotal;

    /*
     *  Set display_lines based on the user_mode global.
     */
    if (user_mode==NOVICE_MODE)
        display_lines = LYlines-4;
    else
        display_lines = LYlines-2;

    /*
     *  Counting the number of options to be displayed.
     *   num_options ranges 0...n
     */
    for (; opt_ptr->next; num_options++, opt_ptr = opt_ptr->next)
         ; /* null body */

    /*
     *  Let's assume for the sake of sanity that ly is the number
     *   corresponding to the line the selection box is on.
     *  Let's also assume that cur_selection is the number of the
     *   item that should be initially selected, as 0 beign the
     *   first item.
     *  So what we have, is the top equal to the current screen line
     *   subtracting the cur_selection + 1 (the one must be for the
     *   top line we will draw in a box).  If the top goes under 0,
     *   consider it 0.
     */
    top = ly - (cur_selection + 1);
    if (top < 0)
	top = 0;

    /*
     *  Check and see if we need to put the i_length parameter up to
     *  the number of real options.
     */
    if (!i_length) {
        i_length = num_options;
    }
    else {
        /*
	 *  Otherwise, it is really one number too high.
	 */
	i_length--;
    }

    /*
     *  The bottom is the value of the top plus the number of options
     *  to view plus 3 (one for the top line, one for the bottom line,
     *  and one to offset the 0 counted in the num_options).
     */
    bottom = top + i_length + 3;

    /*
     *  Hmm...  If the bottom goes beyond the number of lines available,
     */
    if (bottom > display_lines) {
        /*
	 *  Position the window at the top if we have more
	 *  options than will fit in the window.
	 */
	if (i_length+3 > display_lines) {
	    top = 0;
            bottom = top + i_length+3;
	    if (bottom > display_lines)
	        bottom = display_lines + 1;
	} else {
	    /*
	     *  Try to position the window so that the selected option will
	     *    appear where the selection box currently is positioned.
	     *  It could end up too high, at this point, but we'll move it
	     *    down latter, if that has happened.
	     */
	    top = (display_lines + 1) - (i_length + 3);
	    bottom = (display_lines + 1);
	}
    }

    /*
     *  This is really fun, when the length is 4, it means 0-4, or 5.
     */
    length = (bottom - top) - 2;

    /*
     *  Move the window down if it's too high.
     */
    if (bottom < ly + 2) {
        bottom = ly + 2;
	if (bottom > display_lines + 1)
	    bottom = display_lines + 1;
	top = bottom - length - 2;
    }

    /*
     *  Set up the overall window, including the boxing characters ('*'),
     *  if it all fits.  Otherwise, set up the widest window possible. - FM
     */
#ifndef USE_SLANG
    if (!(form_window = newwin(bottom - top, width+4, top, lx - 1)) &&
        !(form_window = newwin(bottom - top, 0, top, 0))) {
	HTAlert(POPUP_FAILED);
        return(orig_selection);
    }
    scrollok(form_window, TRUE);
#else
    SLsmg_fill_region (top, lx - 1, bottom - top, width + 4, ' ');
#endif /* !USE_SLANG */

    /*
     *  Set up the window_offset for options.
     *   cur_selection ranges from 0...n
     *   length ranges from 0...m
     */
    if (cur_selection >= length) {
        window_offset = cur_selection - length + 1;
    }

/*
 * OH!  I LOVE GOTOs! hack hack hack
 *        07-11-94 GAB
 *      MORE hack hack hack
 *        09-05-94 FM
 */
redraw:
    opt_ptr = list;

    /*
     *  Display the boxed options.
     */
    for (i = 0; i <= num_options; i++, opt_ptr = opt_ptr->next) {
        if (i >= window_offset && i - window_offset < length) {
#ifndef USE_SLANG
	    wmove(form_window,(i+1)-window_offset,2);
	    wclrtoeol(form_window);
	    waddstr(form_window,opt_ptr->name);
#else
	    SLsmg_gotorc (top + (i+1)-window_offset, lx - 1 + 2);
	    SLsmg_write_nstring (opt_ptr->name, width);
#endif /* !USE_SLANG */
	}
    }
#ifndef USE_SLANG
#ifdef VMS
    VMSbox(form_window, (bottom - top), (width + 4));
#else
    box(form_window, BOXVERT, BOXHORI);
#endif /* VMS */
    wrefresh(form_window);
#else
    SLsmg_draw_box (top, lx - 1, bottom - top, width + 4);
#endif /* !USE_SLANG */
    opt_ptr = NULL;

    /*
     *  Loop on user input.
     */
    while (cmd != LYK_ACTIVATE) {

        /*
	 *  Unreverse cur selection.
	 */
	if (opt_ptr != NULL) {
#ifndef USE_SLANG
	    wmove(form_window,(i+1)-window_offset,2);
            waddstr(form_window,opt_ptr->name);
#else
	    SLsmg_gotorc (top + (i+1)-window_offset, lx - 1 + 2);
	    SLsmg_write_nstring (opt_ptr->name, width);
#endif /* !USE_SLANG */
	}

        opt_ptr = list;

        for (i = 0; i < cur_selection; i++, opt_ptr = opt_ptr->next) 
	    ; /* null body */

#ifndef USE_SLANG
        wstart_reverse(form_window);
        wmove(form_window,(i+1)-window_offset,2);
        waddstr(form_window,opt_ptr->name);
        wstop_reverse(form_window);
        wrefresh(form_window);
#else
        SLsmg_set_color (2);
        SLsmg_gotorc (top + (i+1)-window_offset, lx - 1 + 2);
        SLsmg_write_nstring (opt_ptr->name, width);
        SLsmg_set_color (0);
        SLsmg_refresh ();
#endif /* !USE_SLANG  */

        c = LYgetch();
	cmd = keymap[c+1];
#ifdef VMS
	  if (HadVMSInterrupt) {
	      HadVMSInterrupt = FALSE;
	      cmd = 7;
	  }
#endif /* VMS */

        switch(cmd) {
            case LYK_PREV_LINK:
	    case LYK_UP_LINK:

		if (cur_selection > 0)
		    cur_selection--;

		/*
		 *  Scroll the window up if neccessary.
		 */
		if ((cur_selection - window_offset) < 0) {
		    window_offset--;
		    goto redraw;
		}
                break;

            case LYK_NEXT_LINK:
	    case LYK_DOWN_LINK:
		if (cur_selection < num_options)
                    cur_selection++;

		/*
		 *  Scroll the window down if neccessary
		 */
		if ((cur_selection - window_offset) >= length) {
		    window_offset++;
		    goto redraw;
		}
                break;

	    case LYK_NEXT_PAGE:
		/*
		 *  Okay, are we on the last page of the list?
		 *  If not then,
		 */
		if (window_offset != (num_options - length + 1)) {
		    /*
		     *  Modify the current selection to not be a
		     *  coordinate in the list, but a coordinate
		     *  on the item selected in the window.
		     */
		    cur_selection -= window_offset;

		    /*
		     *  Page down the proper length for the list.
		     *  If simply to far, back up.
		     */
		    window_offset += length;
		    if (window_offset > (num_options - length)) {
		        window_offset = (num_options - length + 1);
		    }

		    /*
		     *  Readjust the current selection to be a
		     *  list coordinate rather than window.
		     *  Redraw this thing.
		     */
		    cur_selection += window_offset;
		    goto redraw;
		}
		else if (cur_selection < num_options) {
		    /*
		     *  Already on last page of the list so just
		     *  redraw it with the last item selected.
		     */
		    cur_selection = num_options;
		}
		break;

	    case LYK_PREV_PAGE:
		/*
		 *  Are we on the first page of the list?
		 *  If not then,
		 */
		if (window_offset != 0) {
		    /*
		     *  Modify the current selection to not be a
		     *  list coordinate, but a window coordinate.
		     */
		    cur_selection -= window_offset;

		    /*
		     *  Page up the proper length.
		     *  If too far, back up.
		     */
		    window_offset -= length;
		    if (window_offset < 0) {
		        window_offset = 0;
		    }

		    /*
		     *  Readjust the current selection.
		     */
		    cur_selection += window_offset;
		    goto redraw;
		} else if (cur_selection > 0) {
		    /*
		     *  Already on the first page so just
		     *  back up to the first item.
		     */
		    cur_selection = 0;
		}
		break;

	    case LYK_HOME:
	        cur_selection = 0;
		if (window_offset > 0) {
		    window_offset = 0;
		    goto redraw;
		}
		break;

	    case LYK_END:
	        cur_selection = num_options;
		if (window_offset != (num_options - length + 1)) {
		    window_offset = (num_options - length + 1);
		    goto redraw;
		}
		break;

            case LYK_DOWN_TWO:
	        cur_selection += 2;
		if (cur_selection > num_options)
                    cur_selection = num_options;

		/*
		 *  Scroll the window down if neccessary.
		 */
		if ((cur_selection - window_offset) >= length) {
		    window_offset += 2;
		    if (window_offset > (num_options - length + 1))
		        window_offset = (num_options - length + 1);
		    goto redraw;
		}
                break;

	    case LYK_UP_TWO:
	        cur_selection -= 2;
		if (cur_selection < 0)
		    cur_selection = 0;

		/*
		 *  Scroll the window up if neccessary.
		 */
		if ((cur_selection - window_offset) < 0) {
		    window_offset -= 2;
		    if (window_offset < 0)
		        window_offset = 0;
		    goto redraw;
		}
                break;

            case LYK_DOWN_HALF:
	        cur_selection += (length/2);
		if (cur_selection > num_options)
                    cur_selection = num_options;

		/*
		 *  Scroll the window down if neccessary.
		 */
		if ((cur_selection - window_offset) >= length) {
		    window_offset += (length/2);
		    if (window_offset > (num_options - length + 1))
		        window_offset = (num_options - length + 1);
		    goto redraw;
		}
                break;

	    case LYK_UP_HALF:
	        cur_selection -= (length/2);
		if (cur_selection < 0)
		    cur_selection = 0;

		/*
		 *  Scroll the window up if neccessary.
		 */
		if ((cur_selection - window_offset) < 0) {
		    window_offset -= (length/2);
		    if (window_offset < 0)
		        window_offset = 0;
		    goto redraw;
		}
                break;

	    case LYK_REFRESH:
	        clearok(curscr, TRUE);
	        refresh();
		break;

	    case LYK_NEXT:
	        strcpy(prev_target, prev_target_buffer);
	    case LYK_WHEREIS:
	        if (*prev_target == '\0' ) {
		    _statusline(ENTER_WHEREIS_QUERY);
		    if ((ch = LYgetstr(prev_target, VISIBLE,
	    		 	       sizeof(prev_target_buffer),
				       recall)) < 0) {
			/*
			 *  User cancelled the search via ^G. - FM
			 */
			_statusline(CANCELLED);
			sleep(InfoSecs);
			goto restore_popup_statusline;
		    }
		}

check_recall:
		if (*prev_target == '\0' &&
		    !(recall && (ch == UPARROW || ch == DNARROW))) {
		    /*
		     *  No entry.  Simply break.   - FM
		     */
	            _statusline(CANCELLED);
		    sleep(InfoSecs);
		    goto restore_popup_statusline;
		}

		if (recall && ch == UPARROW) {
		    if (FirstRecall) {
		        /*
			 *  Use the current string or
			 *  last query in the list. - FM
			 */
			FirstRecall = FALSE;
			if (*prev_target_buffer) {
			    for (QueryNum = (QueryTotal - 1);
			         QueryNum > 0; QueryNum--) {
				if ((cp=(char *)HTList_objectAt(search_queries,
	    						QueryNum)) != NULL &&
				    !strcmp(prev_target_buffer, cp)) {
				    break;
				}
			    }
			} else {
			    QueryNum = 0;
			}
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
			strcpy(prev_target, cp);
			if (*prev_target_buffer &&
			    !strcmp(prev_target_buffer, prev_target)) {
			    _statusline(EDIT_CURRENT_QUERY);
			} else if ((*prev_target_buffer && QueryTotal == 2) ||
				   (!(*prev_target_buffer) &&
				      QueryTotal == 1)) {
			    _statusline(EDIT_THE_PREV_QUERY);
			} else {
			    _statusline(EDIT_A_PREV_QUERY);
			}
			if ((ch=LYgetstr(prev_target, VISIBLE,
				sizeof(prev_target_buffer), recall)) < 0) {
			    /*
			     *  User cancelled the search via ^G. - FM
			     */
			    _statusline(CANCELLED);
			    sleep(InfoSecs);
			    goto restore_popup_statusline;
			}
			goto check_recall;
		    }
		} else if (recall && ch == DNARROW) {
		    if (FirstRecall) {
		    /*
		     *  Use the current string or
		     *  first query in the list. - FM
		     */
		    FirstRecall = FALSE;
		    if (*prev_target_buffer) {
			for (QueryNum = 0;
			     QueryNum < (QueryTotal - 1); QueryNum++) {
			    if ((cp=(char *)HTList_objectAt(search_queries,
	    					    QueryNum)) != NULL &&
				!strcmp(prev_target_buffer, cp)) {
				    break;
			    }
			}
		    } else {
			QueryNum = QueryTotal - 1;
		    }
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
			strcpy(prev_target, cp);
			if (*prev_target_buffer &&
			    !strcmp(prev_target_buffer, prev_target)) {
			    _statusline(EDIT_CURRENT_QUERY);
			} else if ((*prev_target_buffer &&
				    QueryTotal == 2) ||
				   (!(*prev_target_buffer) &&
				    QueryTotal == 1)) {
			    _statusline(EDIT_THE_PREV_QUERY);
			} else {
			    _statusline(EDIT_A_PREV_QUERY);
			}
			if ((ch = LYgetstr(prev_target, VISIBLE,
					   sizeof(prev_target_buffer),
					   recall)) < 0) {
			    /*
			     * User cancelled the search via ^G. - FM
			     */
			    _statusline(CANCELLED);
			    sleep(InfoSecs);
			    goto restore_popup_statusline;
	    		}
			goto check_recall;
		    }
 		}
		/*
		 *  Replace the search string buffer with the new target. - FM
		 */
		strcpy(prev_target_buffer, prev_target);
		HTAddSearchQuery(prev_target_buffer);

		/*
		 *  Start search at the next option. - FM
		 */
		for (j = 1, tmp_ptr = opt_ptr->next;
		     tmp_ptr != NULL; tmp_ptr = tmp_ptr->next, j++) {
		    if (case_sensitive) {
			if (strstr(tmp_ptr->name, prev_target_buffer) != NULL)
			    break;
		    } else {
			if (LYstrstr(tmp_ptr->name, prev_target_buffer) != NULL)
			    break;
		    }
		}
		if (tmp_ptr != NULL) {
		    /*
		     *  We have a hit, so make that option the current. - FM
		     */
		    cur_selection += j;
		    /*
		     *  Scroll the window down if neccessary.
		     */
		    if ((cur_selection - window_offset) >= length) {
		        window_offset += j;
			if (window_offset > (num_options - length + 1))
			    window_offset = (num_options - length + 1);
			ReDraw = TRUE;
		    }
		    goto restore_popup_statusline;
		}

		/*
		 *  If we started at the beginning, it can't be present. - FM
		 */
		if (cur_selection == 0) {
		    _user_message(STRING_NOT_FOUND, prev_target_buffer);
		    sleep(MessageSecs);
		    goto restore_popup_statusline;
		}

		/*
		 *  Search from the beginning to the current option. - FM
		 */
		for (j = 0, tmp_ptr = list;
		     j < cur_selection; tmp_ptr = tmp_ptr->next, j++) {
		    if (case_sensitive) {
			if (strstr(tmp_ptr->name, prev_target_buffer) != NULL)
			    break;
		    } else {
			if (LYstrstr(tmp_ptr->name, prev_target_buffer) != NULL)
			    break;
		    }
		}
		if (j < cur_selection) {
		    /*
		     *  We have a hit, so make that option the current. - FM
		     */
		    j = (cur_selection - j);
		    cur_selection -= j;
		    /*
		     *  Scroll the window up if neccessary.
		     */
		    if ((cur_selection - window_offset) < 0) {
		        window_offset -= j;
			if (window_offset < 0)
			    window_offset = 0;
			ReDraw = TRUE;
		    }
		    goto restore_popup_statusline;
		}

		/*
		 *  Didn't find it in the preceding options either. - FM
		 */
		_user_message(STRING_NOT_FOUND, prev_target_buffer);
		sleep(MessageSecs);

restore_popup_statusline:
		/*
		 *  Restore the popup statusline and
		 *  reset the search variables. - FM
		 */
		if (disabled)
		    _statusline(FORM_LINK_OPTION_LIST_UNM_MSG);
   		else
		    _statusline(FORM_LINK_OPTION_LIST_MESSAGE);
		*prev_target = '\0';
		QueryTotal = (search_queries ? HTList_count(search_queries)
					     : 0);
		recall = ((QueryTotal >= 1) ? RECALL : NORECALL);
		QueryNum = QueryTotal;
		if (ReDraw == TRUE) {
		    ReDraw = FALSE;
		    goto redraw;
		}
		break;

	    case LYK_QUIT:
	    case LYK_ABORT:
	    case LYK_PREV_DOC:
	    case 7:	/* Control-G */
	    case 3:	/* Control-C */
		cur_selection = orig_selection;
		cmd = LYK_ACTIVATE; /* to exit */
		break;
        }

    }
#ifndef USE_SLANG
    delwin(form_window);
#endif /* !USE_SLANG */
    refresh();

    return(cur_selection);
}
