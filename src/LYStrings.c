#include "HTUtils.h"
#include "tcp.h"
#include "HTCJK.h"
#include "LYCurses.h"
#include "LYUtils.h"
#include "LYStrings.h"
#include "LYGlobalDefs.h"
#include "GridText.h"
#include "LYKeymap.h"
#include "LYSignal.h"
#include "LYClean.h"

#include <ctype.h>

#include "LYLeaks.h"

#if !defined(FANCY_CURSES) && !defined(NO_KEYPAD)
#define NO_KEYPAD
#endif /* !FANCY_CURSES && !NO_KEYPAD */

#define FREE(x) if (x) {free(x); x = NULL;}

extern BOOL HTPassHighCtrlRaw;
extern HTCJKlang HTCJK;

/*
 * LYstrncpy() terminates strings with a null byte.
 * Writes a null byte into the n+1 byte of dst.
 */
PUBLIC char * LYstrncpy ARGS3(char *,dst, char *,src, int,n)
{
    char *val;
    int len=strlen(src);

    if (n < 0)
        n = 0;

    val = strncpy(dst, src, n);
    if (len < n)
        *(dst+len) = '\0';
    else
        *(dst+n) = '\0';
    return val;
}

#ifdef USE_SLANG
#ifdef VMS
#define GetChar() ttgetc()
#else
#define GetChar (int)SLang_getkey
#endif /* VMS */
#else /* Not slang: */
#ifdef VMS
#define GetChar() ttgetc()
#else
#if defined(SNAKE) || defined(NCURSES)
#define GetChar() wgetch(stdscr)
#else /* everything but VMS, SNAKE and NCURSES */
#ifndef USE_GETCHAR
#define USE_GETCHAR
#endif /* !USE_GETCHAR */
#define GetChar() getchar()  /* used to be "getc(stdin)" and "getch()" */
#endif /* SNAKE || NCURSES */
#endif /* VMS */
#endif /* USE_SLANG */

/*
 * LYgetch() translates some escape sequences and may fake noecho
 */
PUBLIC int LYgetch ()
{
    int a, b, c, d = -1;

#if defined(IGNORE_CTRL_C) || defined(USE_GETCHAR)
re_read:
#endif /* IGNORE_CTRL_C || USE_GETCHAR */
#ifndef USE_SLANG
    clearerr(stdin); /* needed here for ultrix and SOCKETSHR, but why? - FM */
#endif /* !USE_SLANG */
#if !defined(USE_SLANG) || defined(VMS)
    c = GetChar();
#else
    if (LYCursesON) {
	c = GetChar();
    } else {
        c = getchar();
	if (c == EOF && errno == EINTR)	/* Ctrl-Z causes EINTR in getchar() */
	    clearerr(stdin);
	if (feof(stdin) || ferror(stdin) || c == EOF) {
#ifdef IGNORE_CTRL_C
	    if (sigint)
	        sigint = FALSE;
#endif /* IGNORE_CTRL_C */
	    return(7); /* use ^G to cancel whatever called us. */
	}
   }
#endif /* !USE_SLANG || VMS */

#ifdef USE_GETCHAR
    if (c == EOF && errno == EINTR)	/* Ctrl-Z causes EINTR in getchar() */
        goto re_read;
#endif /* USE_GETCHAR */

#ifdef USE_SLANG
    if (c == 0xFFFF && LYCursesON) {
#ifdef IGNORE_CTRL_C
	if (sigint) {
	    sigint = FALSE;
	    goto re_read;
	}
#endif /* IGNORE_CTRL_C */
        return(7); /* use ^G to cancel whatever called us. */
    }
#else
    if (feof(stdin) || ferror(stdin) || c == EOF) {
	if (recent_sizechange)
	    return(7); /* use ^G to cancel whatever called us. */
#ifdef IGNORE_CTRL_C
	if (sigint) {
	    sigint = FALSE;
	    /* clearerr(stdin);  don't need here if stays above - FM */
	    goto re_read;
	}
#endif /* IGNORE_CTRL_C */
	cleanup();
        (void) signal(SIGHUP, SIG_DFL);
        (void) signal(SIGTERM, SIG_DFL);
#ifndef VMS
        (void) signal(SIGINT, SIG_DFL);
#endif /* !VMS */
#ifdef SIGTSTP
	if (no_suspend)
	  (void) signal(SIGTSTP,SIG_DFL);
#endif /* SIGTSTP */
	exit(0);
    }
#endif /* USE_SLANG */

    if (c == 27 || c == 155) {      /* handle escape sequence */
        b = GetChar();

        if (b == '[' || b == 'O') {
            a = GetChar();
        } else {
            a = b;
	}

        switch (a) {
        case 'A': c = UPARROW; break;
        case 'x': c = UPARROW; break;  /* keypad up on pc ncsa telnet */
        case 'B': c = DNARROW; break;
        case 'r': c = DNARROW; break; /* keypad down on pc ncsa telnet */
        case 'C': c = RTARROW; break;
        case 'v': c = RTARROW; break; /* keypad right on pc ncsa telnet */
        case 'D': c = LTARROW; break;
        case 't': c = LTARROW; break; /* keypad left on pc ncsa telnet */
        case 'y': c = PGUP; break;  /* keypad on pc ncsa telnet */
        case 's': c = PGDOWN; break;  /* keypad on pc ncsa telnet */
        case 'w': c = HOME; break;  /* keypad on pc ncsa telnet */
        case 'q': c = END; break;  /* keypad on pc ncsa telnet */
        case 'M': c = '\n'; break; /* kepad enter on pc ncsa telnet */

        case 'm':
#ifdef VMS
            if (b != 'O')
#endif /* VMS */
                c = '-';  /* keypad on pc ncsa telnet */
            break;
	case 'k':
	    if (b == 'O')
		c = '+';  /* keypad + on my xterminal :) */
	    break;
        case 'l':
#ifdef VMS
            if (b != 'O')
#endif /* VMS */
                c = '+';  /* keypad on pc ncsa telnet */
            break;
        case 'P':
#ifdef VMS
            if (b != 'O')
#endif /* VMS */
                c = F1;
            break;
        case 'u':
#ifdef VMS
            if (b != 'O')
#endif /* VMS */
                c = F1;  /* macintosh help button */
            break;
        case '1':                           /** VT300  Find  **/
            if ((b == '[' || c == 155) && (d=GetChar()) == '~')
                c = FIND_KEY;
            break;
	case '2':
	    if (b == '[' || c == 155) {
	        if ((d=GetChar())=='~')     /** VT300 Insert **/
	            c = INSERT_KEY;
	        else if ((d == '8' ||
			  d == '9') &&
			 GetChar() == '~')
	         {
		    if (d == '8')            /** VT300  Help **/
	                c = F1;
	            else if (d == '9')       /** VT300   Do  **/
	                c = DO_KEY;
		    d = -1;
		 }
	    }
	    break;
	case '3':			     /** VT300 Delete **/
	    if ((b == '[' || c == 155) && (d=GetChar()) == '~')
	        c = REMOVE_KEY;
	    break;
        case '4':                            /** VT300 Select **/
            if ((b == '[' || c == 155) && (d=GetChar()) == '~')
                c = SELECT_KEY;
            break;
        case '5':                            /** VT300 PrevScreen **/
            if ((b == '[' || c == 155) && (d=GetChar()) == '~')
                c = PGUP;
            break;
        case '6':                            /** VT300 NextScreen **/
            if ((b == '[' || c == 155) && (d=GetChar()) == '~')
                c = PGDOWN;
            break;
	default:
	   if (TRACE) {
		fprintf(stderr,"Unknown key sequence: %d:%d:%d\n",c,b,a);
		sleep(MessageSecs);
	   }
        }
	if (isdigit(a) && (b == '[' || c == 155) && d != -1 && d != '~')
	    d = GetChar();
#if defined(NO_KEYPAD) || defined(VMS)
    }
#else
    } else {

	/* convert keypad() mode keys into Lynx defined keys
	 */

	switch(c) {
	case KEY_DOWN:	           /* The four arrow keys ... */
	   c=DNARROW;
	   break;
	case KEY_UP:	
	   c=UPARROW;
	   break;
	case KEY_LEFT:	
	   c=LTARROW;
	   break;
	case KEY_RIGHT:	           /* ... */
	   c=RTARROW;
	   break;
	case KEY_HOME:	           /* Home key (upward+left arrow) */
	   c=HOME;
	   break;
	case KEY_CLEAR:	           /* Clear screen */
	   c=18; /* CTRL-R */
	   break;
	case KEY_NPAGE:	           /* Next page */
	   c=PGDOWN;
	   break;
	case KEY_PPAGE:	           /* Previous page */
	   c=PGUP;
	   break;
	case KEY_LL:	           /* home down or bottom (lower left) */
	   c=END;
	   break;
                                        /* The keypad is arranged like this:*/
                                        /*    a1    up    a3   */
                                        /*   left   b2  right  */
                                        /*    c1   down   c3   */
	case KEY_A1:	           /* upper left of keypad */
	   c=HOME;
	   break;
	case KEY_A3:	           /* upper right of keypad */
	   c=PGUP;
	   break;
	case KEY_B2:	           /* center of keypad */
	   c=DO_NOTHING;
	   break;
	case KEY_C1:	           /* lower left of keypad */
	   c=END;
	   break;
	case KEY_C3:	           /* lower right of keypad */
	   c=PGDOWN;
	   break;
#ifdef KEY_END
	case KEY_END:	           /* end key           001 */
	   c=END;
	   break;
#endif /* KEY_END */
#ifdef KEY_HELP
	case KEY_HELP:	           /* help key          001 */
	   c=F1;
	   break;
#endif /* KEY_HELP */
#ifdef KEY_BACKSPACE
	case KEY_BACKSPACE:
	   c=127;		   /* backspace key (delete, not Ctrl-H) */
	   break;
#endif /* KEY_BACKSPACE */
	}
    }
#endif /* defined(NO_KEYPAD) || defined(VMS) */

    return(c);
}


/*
 * display the current value of the string and allow the user
 * to edit it.
 */

#ifdef USE_SLANG
#define GetYX(y,x)   y = SLsmg_get_row(), x = SLsmg_get_column()
#else
#ifdef getyx
#define GetYX(y,x)   getyx(stdscr,y,x)
#else
#define GetYX(y,x)   y = stdscr->_cury, x = stdscr->_curx
#endif /* getyx */
#endif /* USE_SLANG */

#define EDREC    EditFieldData

/* shorthand to get rid of all most of the "edit->suchandsos" */
#define Buf      edit->buffer
#define Pos      edit->pos
#define StrLen   edit->strlen
#define MaxLen   edit->maxlen
#define DspWdth  edit->dspwdth
#define DspStart edit->xpan
#define Margin   edit->margin

PUBLIC void LYSetupEdit ARGS4(EDREC *,edit, char *,old, int,maxstr, int,maxdsp)
{
    /* Initialize edit record */

    GetYX(edit->sy, edit->sx);
    edit->pad   = ' ';
    edit->dirty = TRUE;
    edit->panon = FALSE;

    StrLen  = strlen(old);
    MaxLen  = maxstr;
    DspWdth = maxdsp;
    Margin  = 0;
    Pos = strlen(old);
    DspStart = 0;

    if (maxstr > maxdsp) {  /* Need panning? */
        if (DspWdth > 4)    /* Else "{}" take up precious screen space */
	    edit->panon = TRUE;

	/* Figure out margins. If too big we do a lot of unnecessary
	 * scrolling. If too small user doesn't have sufficient look-ahead.
	 * Let's say 25% for each margin, upper bound is 10 columns.
	 */
	Margin = DspWdth/4;
	if (Margin > 10)
	    Margin = 10;
    }
    strcpy(edit->buffer, old);
}

PUBLIC int LYEdit1 ARGS4(EDREC *,edit, int,ch, int,action, BOOL,maxMessage)
{   /* returns 0    character processed
     *         ch   otherwise
     */

    int i;
    int length;

    if (MaxLen <= 0)
        return(0); /* Be defensive */

    length=strlen(&Buf[0]);
    StrLen = length;

    switch (action) {
    case LYE_AIX:
        /*
	 * Hex 97.
	 * Fall through as a character for CJK.
	 * Otherwise, we treat this as LYE_ENTER.
	 */
	 if (HTCJK == NOCJK)
	     return(ch);
    case LYE_CHAR:
        /*
	 * ch is printable or ISO-8859-1 escape character.
	 */
	if (Pos <= (MaxLen) && StrLen < (MaxLen)) {
	    for(i = length; i >= Pos; i--)    /* Make room */
		Buf[i+1] = Buf[i];
	    Buf[length+1]='\0';
	    Buf[Pos] = (unsigned char) ch;
	    Pos++;
	} else if (maxMessage) {
	    _statusline(MAXLEN_REACHED_DEL_OR_MOV);
	}
	break;

    case LYE_BACKW:
        /*
	 * Backword.
	 * Definition of word is very naive: 1 or more a/n characters.
	 */
	while (Pos && !isalnum(Buf[Pos-1]))
	    Pos--;
	while (Pos &&  isalnum(Buf[Pos-1]))
	    Pos--;
	break;

    case LYE_FORWW:
        /*
	 * Word forward.
	 */
	while (isalnum(Buf[Pos]))
	    Pos++;   /* '\0' is not a/n */
	while (!isalnum(Buf[Pos]) && Buf[Pos])
	    Pos++ ;
	break;

    case LYE_ERASE:
        /*
	 * Erase the line to start fresh.
	 */
	 Buf[0] = '\0';
	 /* fall through */

    case LYE_BOL:
        /*
	 * Go to first column.
	 */
	Pos = 0;
	break;

    case LYE_EOL:
        /*
	 * Go to last column.
	 */
	Pos = length;
	break;

    case LYE_DELNW:
        /*
	 * Delete next word.
	 */
	{
	    int pos0 = Pos;
	    LYEdit1 (edit, 0, LYE_FORWW, FALSE);
	    while (Pos > pos0)
	        LYEdit1(edit, 0, LYE_DELP, FALSE);
	}
	break;

    case LYE_DELPW:
        /*
	 * Delete previous word.
	 */
	{
	    int pos0 = Pos;
	    LYEdit1 (edit, 0, LYE_BACKW, FALSE);
	    pos0 -= Pos;
	    while (pos0--)
	        LYEdit1(edit, 0, LYE_DELN, FALSE);
	}
	break;

    case LYE_DELN:
        /*
	 * Delete next character
	 */
	if (Pos >= length)
	    break;
	Pos++;
	/* fall through */

    case LYE_DELP:
        /*
	 * Delete preceding character.
	 */
	if (length == 0 || Pos == 0)
	    break;
	Pos--;
	for (i = Pos; i < length; i++)
	    Buf[i] = Buf[i+1];
	i--;
	Buf[i] = 0;
	break;

    case LYE_DELC:
        /*
	 * Delete current character.
	 */
	if (length == 0)
	    break;
	for (i = Pos; i < length; i++)
	    Buf[i] = Buf[i+1];
	i--;
	Buf[i] = 0;
	break;

    case LYE_FORW:
        /*
	 * Move cursor to the right.
	 */
	if (Pos < length)
	    Pos++;
	break;

    case LYE_BACK:
        /*
	 * Left-arrow move cursor to the left.
	 */
	if (Pos > 0)
	    Pos--;
	break;

    case LYE_UPPER:
	for (i = 0; Buf[i]; i++)
	   Buf[i] = TOUPPER(Buf[i]);
	break;

    case LYE_LOWER:
	for (i = 0; Buf[i]; i++)
	   Buf[i] = TOLOWER(Buf[i]);
	break;

    default:
	return(ch);
    }
    edit->dirty = TRUE;
    StrLen = strlen(&Buf[0]);
    return(0);
}


PUBLIC void LYRefreshEdit ARGS1(EDREC *,edit)
{
    int i;
    int length;
    int nrdisplayed;
    int padsize;
    char *str;
    char buffer[3];

    buffer[0] = buffer[1] = buffer[2] = '\0';
    if (!edit->dirty || (DspWdth == 0))
        return;
    edit->dirty = FALSE;

    length=strlen(&Buf[0]);
    edit->strlen = length;
/*
 *  Now we have:
 *                .--DspWdth---.
 *      +---------+=============+-----------+
 *      |         |M           M|           |   (M=margin)
 *      +---------+=============+-----------+
 *      0         DspStart                   length
 *
 *  Insertion point can be anywhere beween 0 and stringlength.
 *  Figure out new display starting point.
 *
 *   The first "if" below makes Lynx scroll several columns at a time when
 *   extending the string. Looks awful, but that way we can keep up with
 *   data entry at low baudrates.
 */
    if ((DspStart + DspWdth) <= length)
        if (Pos >= (DspStart + DspWdth) - Margin)
	    DspStart=(Pos - DspWdth) + Margin;

    if (Pos < DspStart + Margin) {
        DspStart = Pos - Margin;
	if (DspStart < 0)
	    DspStart = 0;
    }

    str = &Buf[DspStart];

    nrdisplayed = length-DspStart;
    if (nrdisplayed > DspWdth)
        nrdisplayed = DspWdth;

    move(edit->sy, edit->sx);
    if (edit->hidden) {
        for (i = 0; i < nrdisplayed; i++)
	    addch('*');
    } else {
        for (i = 0; i < nrdisplayed; i++)
	    if ((buffer[0] = str[i]) == 1 || buffer[0] == 2 ||
	        ((unsigned char)buffer[0] == 160 &&
		 !(HTPassHighCtrlRaw || HTCJK != NOCJK))) {
	        addch(' ');
	    } else {
		/* For CJK strings, by Masanobu Kimura */
		if (HTCJK != NOCJK && !isascii(buffer[0])) {
		    if (i < (nrdisplayed - 1))
		        buffer[1] = str[++i];
		    addstr(buffer);
		    buffer[1] = '\0';
		} else {
		    addstr(buffer);
		}
	    }
    }

    /*
     * Erase rest of input area.
     */
    padsize= DspWdth-nrdisplayed;
    while (padsize--)
        addch((unsigned char)edit->pad);

    /*
     * Scrolling indicators.
     */
    if (edit->panon) {
        if ((DspStart + nrdisplayed) < length) {
	    move(edit->sy, edit->sx+nrdisplayed-1);
	    addch('}');
	}
	if (DspStart) {
	    move(edit->sy, edit->sx);
	    addch('{');
	}
    }

    move(edit->sy, edit->sx + Pos - DspStart);
    refresh();
}


PUBLIC int LYgetstr ARGS4(char *,inputline, int,hidden,
			  int,bufsize, int,recall)
{
    extern BOOLEAN term_letter;     /* Flag from terminate_letter() AST  */
    extern BOOLEAN term_options;    /* Flag from terminate_options() AST */
    extern BOOLEAN term_message;    /* Flag from terminate_message() AST */
#ifdef VMS
    extern BOOLEAN HadVMSInterrupt; /* Flag from cleanup_sig() AST       */
#endif
    int x, y, MaxStringSize;
    int ch;
    EditFieldData MyEdit;

    GetYX(y,x);                 /* Use screen from cursor position to eol */
    MaxStringSize = (bufsize < sizeof(MyEdit.buffer)) ?
    		    (bufsize - 1) : (sizeof(MyEdit.buffer) - 1);
    LYSetupEdit(&MyEdit, inputline, MaxStringSize, (LYcols-1)-x);
    MyEdit.hidden = hidden ;

    for (;;) {
again:
        LYRefreshEdit(&MyEdit);
        ch = LYgetch();
#ifdef VMS
	if (term_letter || term_options || term_message || HadVMSInterrupt) {
	    HadVMSInterrupt = FALSE;
	    ch = 7;
	}
#else
	if (term_letter || term_options || term_message)
	    ch = 7;
#endif /* VMS */
	if (recall && (ch == UPARROW || ch == DNARROW)) {
	    strcpy(inputline, MyEdit.buffer);
	    return(ch);
	}
	if (keymap[ch + 1] == LYK_REFRESH)
	    goto again;
        switch (EditBinding(ch)) {
	case LYE_TAB:
	    ch = '\t';
	    /* fall through */
	case LYE_AIX:
	    /*
	     * Hex 97.
	     * Treat as a character for CJK.
	     * Otherwise, we treat this as LYE_ENTER.
	     */
	    if (HTCJK != NOCJK && ch != '\t') {
	        LYLineEdit(&MyEdit,ch, FALSE);
		break;
	    }
	case LYE_ENTER:
	    /*
	     * Terminate the string and return.
	     */
	    strcpy(inputline, MyEdit.buffer);
            return(ch);
	    break;

        case LYE_ABORT:
	    /*
	     * Control-C or Control-G aborts.
	     */
	    inputline[0] = '\0';		   
	    return(-1);
            break;

        default:
            LYLineEdit(&MyEdit,ch, FALSE);
        }
    }
}

/*
 * LYstrstr will find the first occurence of the string pointed to by tarptr
 * in the string pointed to by chptr.  
 * It is a case insensitive search.
 */
PUBLIC char * LYstrstr ARGS2(char *,chptr, char *,tarptr)
{
    register char *tmpchptr, *tmptarptr;

    for(; *chptr != '\0'; chptr++) {
	if(TOUPPER(*chptr) == TOUPPER(*tarptr)) {	
	    /* see if they line up */ 
	    for(tmpchptr = chptr+1, tmptarptr = tarptr+1;
	        (TOUPPER(*tmpchptr) == TOUPPER(*tmptarptr) &&
		 *tmptarptr != '\0' && *tmpchptr != '\0');
	        tmpchptr++, tmptarptr++)
		   ; /* null body */ 
	    if(*tmptarptr == '\0') 
	  	return(chptr);
	}
    } /* end for */

    return(NULL);
}	

/*
 * LYno_attr_char_case_strstr will find the first occurence of the string 
 * pointed to by tarptr in the string pointed to by chptr.
 * It ignores the characters: LY_UNDERLINE_START_CHAR and
 * 			      LY_UNDERLINE_END_CHAR
 * 			      LY_BOLD_START_CHAR
 * 			      LY_BOLD_END_CHAR[B
 *			      if present in chptr.
 * It is a case insensitive search.
 */
PUBLIC char * LYno_attr_char_case_strstr ARGS2(char *,chptr, char *,tarptr)
{
    register char *tmpchptr, *tmptarptr;

    if (!chptr)
        return(NULL);

    while (IsSpecialAttrChar(*chptr) && *chptr != '\0')
        chptr++;

    for (; *chptr != '\0'; chptr++) {

        if (TOUPPER(*chptr) == TOUPPER(*tarptr)) {

            /* see if they line up */
	    tmpchptr = chptr+1;
	    tmptarptr = tarptr+1;

	    if (*tmptarptr == '\0')  /* one char target */
		 return(chptr);

	    while (1) {
		 if (!IsSpecialAttrChar(*tmpchptr)) {

                    if (TOUPPER(*tmpchptr) != TOUPPER(*tmptarptr))
			break;

		    tmpchptr++;
		    tmptarptr++;

		 } else {
		    tmpchptr++;
		 }

                 if (*tmptarptr == '\0')
		     return(chptr);

		 if (*tmpchptr == '\0')
		     break;
	    }
        }
    } /* end for */

    return(NULL);
}

/*
 * LYno_attr_char_strstr will find the first occurence of the string
 * pointed to by tarptr in the string pointed to by chptr.
 * It ignores the characters: LY_UNDERLINE_START_CHAR and
 *                            LY_UNDERLINE_END_CHAR
 *                            LY_BOLD_START_CHAR
 *                            LY_BOLD_END_CHAR
 *			      if present in chptr.
 * It is a case sensitive search.
 */
PUBLIC char * LYno_attr_char_strstr ARGS2(char *,chptr, char *,tarptr)
{
    register char *tmpchptr, *tmptarptr;

    if (!chptr)
        return(NULL);

    while (IsSpecialAttrChar(*chptr) && *chptr != '\0')
        chptr++;

    for (; *chptr != '\0'; chptr++) {

        if ((*chptr) == (*tarptr)) {

            /* see if they line up */
            tmpchptr = chptr+1;
            tmptarptr = tarptr+1;

	    if (*tmptarptr == '\0')  /* one char target */
		 return(chptr);

            while (1) {
		 if (!IsSpecialAttrChar(*tmpchptr)) {

                    if ((*tmpchptr) != (*tmptarptr))
                        break;

                    tmpchptr++;
                    tmptarptr++;

                 } else {
                    tmpchptr++;
                 }

                 if (*tmptarptr == '\0')
                     return(chptr);

                 if (*tmpchptr == '\0')
                     break;
            }
        }
    } /* end for */

    return(NULL);
}

/*      Allocate a new copy of a string, and returns it
*/
PUBLIC char * SNACopy ARGS3 (char **,dest, CONST char *,src, int,n)
{
  FREE(*dest);
  if (src) {
    *dest = (char *) calloc (1, n + 1);
    if (*dest == NULL) {
	fprintf(stderr,"Tried to calloc %d bytes\n",n);
	outofmem(__FILE__, "SNACopy");
    }
    strncpy (*dest, src, n);
    *(*dest + n) = '\0'; /* terminate */
  }
  return *dest;
}

/*      String Allocate and Concatenate
*/
PUBLIC char * SNACat ARGS3 (char **,dest, CONST char *,src, int,n)
{
  if (src && *src) {
    if (*dest) {
      int length = strlen (*dest);
      *dest = (char *) realloc (*dest, length + n + 1);
      if (*dest == NULL)
          outofmem(__FILE__, "SNACat");
      strncpy (*dest + length, src, n);
      *(*dest + length + n) = '\0'; /* terminate */
    } else {
      *dest = (char *) calloc (1, strlen(src) + 1);
      if (*dest == NULL)
          outofmem(__FILE__, "SNACat");
      strncpy (*dest, src, n);
      *dest[n] = '\0'; /* terminate */
    }
  }
  return *dest;
}


