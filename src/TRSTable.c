/*		Simple table object
**		===================
** Authors
**	KW	Klaus Weide <kweide@enteract.com>
** History:
**   2 Jul 1999	KW	Created.
*/

#include <HTUtils.h>
#include <HTStyle.h>		/* for HT_LEFT, HT_CENTER, HT_RIGHT */
#include <LYCurses.h>
#include <TRSTable.h>

#include <LYLeaks.h>

#ifdef SAVE_TIME_NOT_SPACE
#define CELLS_GROWBY 16
#define ROWS_GROWBY 16
#else
#define CELLS_GROWBY 2
#define ROWS_GROWBY 2
#endif

#define MAX_STBL_POS (LYcols-1)

/* must be different from HT_ALIGN_NONE and HT_LEFT, HT_CENTER etc.: */
#define RESERVEDCELL (-2)  /* cell's alignment field is overloaded, this
			      value means cell was reserved by ROWSPAN */
#define EOCOLG (-2)	/* sumcols' Line field isn't used for line info, this
			      special value means end of COLGROUP */
typedef enum {
    CS_invalid = -1,
    CS_new     =  0,
    CS__0,			/* new, at BOL */
    CS__0eb,			/* starts at BOL, empty, break */
    CS__eb,			/* empty, break */
    CS__0cb,			/* starts at BOL, content, break */
    CS__cb,			/* content, break */
    CS__0f,			/* starts at BOL, finished */
    CS__ef,			/* empty, finished */
    CS__0cf,			/* starts at BOL, content, finished */
    CS__cf,			/* content, finished */
    CS__ebc,			/* empty, break, more content */
    CS__cbc			/* content, break, more content */
} cellstate_t;

typedef struct _STable_states {
    cellstate_t	prev_state;
    cellstate_t	state;
    int		lineno;		/* last line no. looked at */
    int		icell_core;	/* first/best 'core' cell in row so far */
    int		x_td;		/* x pos of currently open cell or -1 */
    int		pending_len;	/* if state is CS__0?[ec]b (??) */
} STable_states;


typedef struct _STable_cellinfo {
	int	Line;		/* lineno in doc (zero-based) */
	int	pos;		/* column where cell starts */
	int	len;		/* number of character positions */
	int	colspan;	/* number of columns to span */
	short	alignment;	/* one of HT_LEFT, HT_CENTER, HT_RIGHT,
				   or RESERVEDCELL */
} STable_cellinfo;

typedef struct _STable_rowinfo {
	int	Line;		/* lineno in doc (zero-based) */
	int	ncells;		/* number of table cells */
/*	int	pending_skip;*/	/* skip this many after finishing open cell */
	BOOL	fixed_line;	/* if we have a 'core' line of cells */
	int	allocated;	/* number of table cells allocated */
	STable_cellinfo * cells;
	short	alignment;	/* global align attribute for this row */
} STable_rowinfo;

struct _STable_info {
	int	startline;	/* lineno where table starts (zero-based) */
	int	nrows;		/* number of rows */
	int	ncols;		/* number of rows */
	int	maxlen;		/* sum of max. cell lengths of any row */
	int	maxpos;		/* max. of max. cell pos's of any row */
	int	allocated_rows; /* number of rows allocated */
	int	allocated_sumcols;	/* number of sumcols allocated */
	int	ncolinfo;		/* number of COL info collected */
	STable_cellinfo * sumcols; /* for summary (max len/pos) col info */
	STable_rowinfo * rows;
	STable_rowinfo	rowspans2eog;
	short	alignment;	/* global align attribute for this table */
	short	rowgroup_align;	/* align default for current group of rows */
	short	pending_colgroup_align;
	int	pending_colgroup_next;
	STable_states s;
};

/*
**  Functions and structures in this source file keep track of positions.
**  They don't know about the character data in those lines, or about
**  the HText and HTLine structures.  GridText.c doesn't know about our
**  structures.  It should stay that way.
**
**  The basic idea: we let the code in HTML.c/GridText.c produce and format
**  output "as usual", i.e. as without Simple Table support.  We keep track
**  of the positions in the generated output where cells and rows start (or
**  end).  If all goes well, that preliminary output (stored in HText/HTLine
**  structures) can be fixed up when the TABLE end tag is processed, by just
**  inserting spaces in the right places (and possibly changing alignment).
**  If all goes not well, we already have a safe fallback.
**
**  Note that positions passed to and from these functions should be
**  in terms of screen positions, not just byte counts in a HTLine.data
**  (cf. line->data vs. HText_TrueLineSize).
**
**  Memory is allocated dynamically, so we can have tables of arbitrary
**  length.  On allocation error we just return and error indication
**  instead of outofmem(), so caller can give up table tracking and maybe
**  recover memory.
**
**  Implemented:
**  - ALIGN={left,right,center,justify} applied to individual table cells
**    ("justify" is treated as "left")
**  - Inheritance of horizontal alignment according to HTML 4.0
**  - COLSPAN >1 (may work incorrectly for some tables?)
**  - ROWSPAN >1 (reserving cells in following rows)
**  - Line breaks at start of first cell or at end of last cell are treated
**    as if they were not part of the cell and row.  This allows us to
**    cooperate with one way in which tables have been made friendly to
**    browsers without any table support.
**  Missing, but can be added:
**  - Support for COLGROUP/COL
**  - Tables wider than display.  The limitation is not here but in GridText.c
**    etc.  If horizontal scrolling were implemented there, the mechanisms
**    here coudl deal with wide tables (just change MAX_STBL_POS code).
**  Missing, unlikely to add:
**  - Support for non-LTR directionality.  A general problem, support is
**    lacking throughout the lynx code.
**  - Support for most other table-related attributes.  Most of them are
**    for decorative purposes.
**  Impossible or very unlikely (because it doesn't fit the model):
**  - Any cell contents of more than one line, line breaks within cells.
**    Anything that requires handling cell contents as paragraphs (block
**    elements), like reflowing.  Vertical alignment.
*/
PRIVATE int Stbl_finishCellInRow PARAMS((
    STable_rowinfo *	me,
    STable_states *	s,
    BOOL		certain,
    int			lineno,
    int			pos));
PRIVATE int Stbl_finishRowInTable PARAMS((
    STable_info *	me));


PUBLIC struct _STable_info * Stbl_startTABLE ARGS1(
    short,		alignment)
{
    STable_info *me = typecalloc(STable_info);
    if (me) {
	me->alignment = alignment;
	me->rowgroup_align = HT_ALIGN_NONE;
	me->pending_colgroup_align = HT_ALIGN_NONE;
	me->s.x_td = -1;
	me->s.icell_core = -1;
    }
    return me;
}

PRIVATE void free_rowinfo ARGS1(
    STable_rowinfo *,	me)
{
    if (me && me->allocated) {
	FREE(me->cells);
    }
}

PUBLIC void Stbl_free ARGS1(
    STable_info *,	me)
{
    if (me && me->allocated_rows && me->rows) {
	int i;
	for (i = 0; i < me->allocated_rows; i++)
	    free_rowinfo(me->rows + i);
	FREE(me->rows);
    }
    free_rowinfo(&me->rowspans2eog);
    if (me)
	FREE(me->sumcols);
    FREE(me);
}

/*
 * Returns -1 on error, otherwise index of just-added table cell.
 */
PRIVATE int Stbl_addCellToRow ARGS9(
    STable_rowinfo *,	me,
    STable_cellinfo *,	colinfo,
    int,		ncolinfo,
    STable_states *,	s,
    int,		colspan,
    short,		alignment,
    BOOL,		isheader,
    int,		lineno,
    int *,		ppos)
{
    STable_cellinfo *cells;
    int i;
    int last_colspan = me->ncells ?
	me->cells[me->ncells - 1].colspan : 1;
    cellstate_t newstate;

    if (me->ncells == 0)
	s->prev_state = CS_invalid;
    else if (s->prev_state == CS_invalid ||
	     (/*s->state != CS_new && */ s->state != CS__0 &&
	      s->state != CS__ef && s->state != CS__0f))
	s->prev_state = s->state;

    if (me->ncells == 0 || *ppos == 0)
	newstate = CS__0;
    else
	newstate = CS_new;

    if (me->ncells > 0 && s->pending_len > 0) {
	if (s->prev_state != CS__cbc)
	    me->cells[me->ncells - 1].len = s->pending_len;
	s->pending_len = 0;
    }
    s->x_td = *ppos;

    if (lineno != s->lineno) {
	if (!me->fixed_line) {
	    if (me->ncells == 0 || *ppos == 0) {
		switch (s->prev_state) {
		case CS_invalid:
		case CS__0:
		case CS__0eb:
		case CS__0cb:
		case CS__0f:
		case CS__0cf:
		    if (me->ncells > 0)
			for (i = me->ncells + last_colspan - 2;
			     i >= me->ncells - 1; i--) {
			    me->cells[i].pos = *ppos;
			    me->cells[i].Line = lineno;
			}
		    me->Line = lineno;
		    /* s->lineno = lineno; */
		    break;
		case CS_new:
		case CS__eb:
		case CS__ef:
		case CS__cf:
		default:
		    break;
		case CS__cb:
		    *ppos = me->cells[me->ncells - 1].pos +
			me->cells[me->ncells - 1].len;
		}
	    } else {
		switch (s->prev_state) {
		case CS__0:
		case CS__0eb:
		case CS__0f:
		    break;
		case CS__cb:
		    return -1;
		case CS__cf:
/*		    HTAlert("foo woo!!"); */
		    return -1;
		case CS__0cb:
		case CS__0cf:
		    if (*ppos > me->cells[0].pos)
			me->Line = lineno;
		    me->fixed_line = YES;
		    break;
		case CS_new:
		case CS__eb:
		case CS__ef:
		default:
		    me->fixed_line = YES;
		    break;
		case CS__cbc:
		    return -1;
		}
	    }
	}
	if (me->fixed_line && lineno != me->Line) {
	    switch (s->prev_state) {
	    case CS__cb:
	    case CS__cf:
		if (*ppos > 0)
		    return -1;
		else
		    *ppos = me->cells[me->ncells - 1].pos /* == 0 */ +
			me->cells[me->ncells - 1].len;
		break;
	    case CS__0cf:
	    case CS__0cb:
		if (*ppos == 0 || *ppos <= me->cells[0].pos)
		    *ppos = me->cells[me->ncells - 1].pos /* == 0 */ +
			me->cells[me->ncells - 1].len;
		break;
	    case CS__0:
	    case CS__0f:
	    case CS__0eb:
/*		me->Line = lineno; */
		break;
	    case CS_new:
	    case CS__eb:
	    case CS__ef:
	    default:
		*ppos = me->cells[me->ncells - 1].pos;	break;
	    case CS__cbc:
/*		*ppos = me->cells[me->ncells - 1].pos +
		    me->cells[me->ncells - 1].len;
		if (*ppos > 0)
		    return -1; */
		break;
	    case CS_invalid:
		break;
	    }
	}
	s->lineno = lineno;
    } else {			/* lineno == s->lineno: */
	switch (s->prev_state) {
	case CS_invalid:
	case CS__0:
	case CS__0eb:		/* cannot happen */
	case CS__0cb:		/* cannot happen */
	case CS__0f:
	case CS__0cf:		/* ##302?? set icell_core? or only in finish? */
	    break;
	case CS__eb:		/* cannot happen */
	case CS__cb:		/* cannot happen */
	case CS__ef:
	    break;
	case CS__ebc:		/* should have done smth in finish */
	case CS__cbc:		/* should have done smth in finish */
	    break;
	case CS_new:
	case CS__cf:
	    if (me->fixed_line && me->Line != lineno) {
		return -1;
	    } else {
		me->fixed_line = YES;
		me->Line = lineno;
	    }
	}
    }

#if 0				/* MEGA_COMMENTOUT */
    if (lineno != me->Line) {
	if (!me->fixed_line) {
	    if (me->ncells == 0 ||
		(*ppos == 0 && me->cells[me->ncells - 1].pos == 0)) {
		if (me->ncells > 0)
		    for (i = me->ncells + last_colspan - 2;
			 i >= 0; i--) {
			me->cells[i].pos = *ppos;
			me->cells[i].Line = lineno;
		    }
		me->Line = lineno;
		s->state = CS__0;
	    }
	    if (*ppos > 0 && me->ncells > 0 &&
		(me->cells[me->ncells - 1].pos > 0 ||
		 me->cells[me->ncells - 1].len > 0)) {
		me->fixed_line = YES;

	    }
	}
	if (me->fixed_line && lineno != me->Line) {
	    if (me->cells[me->ncells - 1].pos > 0 &&
		me->cells[me->ncells - 1].len > 0) {
		return -1;
	    } else if (me->cells[me->ncells - 1].pos == 0 &&
		       me->cells[me->ncells - 1].len > 0) {
		if (*ppos > 0 && *ppos > me->cells[0].pos)
		    me->Line = lineno;
		else
		    *ppos = me->cells[me->ncells - 1].pos; /* == 0 */
	    } else /* if (me->cells[me->ncells - 1].pos == 0 &&
		       me->cells[me->ncells - 1].len <= 0) {
		me->Line = lineno;
	    } else */ {
		*ppos = me->cells[me->ncells - 1].pos;
	    }
	}
#if 0
	for (i = 0; i < me->ncells; i++) {
	    if (me->cells[i].Line == lineno) {
		break;
	    } else if (me->cells[i].len <= 0) {
		me->cells[i].Line = lineno;
		/* @@@ reset its pos too ?? */
	    } else {
		break;
	    }
	}
	if (i < me->ncells && me->cells[i].Line != lineno)
	    return -1;
	me->Line = lineno;
#endif
    }
#endif /* MEGA_COMMENTOUT */
    s->state = newstate;

    if (me->ncells > 0 && me->cells[me->ncells - 1].colspan > 1) {
	me->ncells += me->cells[me->ncells-1].colspan - 1;
    }
    while (me->ncells < me->allocated &&
	   me->cells[me->ncells].alignment == RESERVEDCELL) {
	me->ncells++;
    }
    {
	int growby = 0;
	while (me->ncells + colspan + 1 > me->allocated + growby)
	    growby += CELLS_GROWBY;
	if (growby) {
	    if (me->allocated == 0 && !me->cells) {
		cells = typecallocn(STable_cellinfo, growby);
	    } else {
		cells = realloc(me->cells,
				  (me->allocated + growby)
				  * sizeof(STable_cellinfo));
		for (i = 0; cells && i < growby; i++) {
		    cells[me->allocated + i].alignment = HT_ALIGN_NONE;
		}
	    }
	    if (cells) {
		me->allocated += growby;
		me->cells = cells;
	    } else {
		return -1;
	    }
	}
    }

    me->cells[me->ncells].Line = lineno;
    me->cells[me->ncells].pos = *ppos;
    me->cells[me->ncells].len = -1;
    me->cells[me->ncells].colspan = colspan;

    if (alignment != HT_ALIGN_NONE)
	    me->cells[me->ncells].alignment = alignment;
    else {
	if (ncolinfo >= me->ncells + 1)
	    me->cells[me->ncells].alignment = colinfo[me->ncells].alignment;
	else
	    me->cells[me->ncells].alignment = me->alignment;
	if (me->cells[me->ncells].alignment==HT_ALIGN_NONE)
	    me->cells[me->ncells].alignment = me->alignment;
	if (me->cells[me->ncells].alignment==HT_ALIGN_NONE)
	    me->cells[me->ncells].alignment = isheader ? HT_CENTER : HT_LEFT;
    }
    for (i = me->ncells + 1; i < me->ncells + colspan; i++) {
	me->cells[i].Line = lineno;
	me->cells[i].pos = *ppos;
	me->cells[i].len = -1;
	me->cells[i].colspan = 0;
	me->cells[i].alignment = HT_LEFT;
    }
    me->cells[me->ncells + colspan].pos = -1; /* not yet used */
    me->ncells++;
    return (me->ncells - 1);
}

/* returns -1 on error, 0 otherwise */
/* assumes cells have already been allocated (but may need more) */
PRIVATE int Stbl_reserveCellsInRow ARGS3(
    STable_rowinfo *,	me,
    int,		icell,
    int,		colspan)
{
    STable_cellinfo *cells;
    int i;
    int growby = icell + colspan - me->allocated;
    if (growby > 0) {
	cells = realloc(me->cells,
			(me->allocated + growby)
			* sizeof(STable_cellinfo));
	if (cells) {
	    for (i = 0; i < growby; i++) {
		cells[me->allocated + i].alignment = HT_ALIGN_NONE;
	    }
	    me->allocated += growby;
	    me->cells = cells;
	} else {
	    return -1;
	}
    }
    for (i = icell; i < icell + colspan; i++) {
	me->cells[i].Line = -1;
	me->cells[i].pos = -1;
	me->cells[i].len = -1;
	me->cells[i].colspan = 0;
	me->cells[i].alignment = RESERVEDCELL;
    }
    me->cells[icell].colspan = colspan;
    return 0;
}

PRIVATE int Stbl_finishCellInRow ARGS5(
    STable_rowinfo *,	me,
    STable_states *,	s,
    BOOL,		certain,
    int,		lineno,
    int,		pos)
{
    STable_cellinfo *lastcell;
    cellstate_t newstate = CS_invalid;
    BOOL broken = NO, empty;

    if (me->ncells <= 0)
	return -1;
    lastcell = me->cells + (me->ncells - 1);
    broken = (lineno != lastcell->Line || lineno != s->lineno);
    empty = broken ? (pos == 0) : (pos <= s->x_td);
    if (broken) {
	if (!certain) {
	    switch (s->state) {
	    case CS_invalid:
		newstate = empty ? CS_invalid : CS__cbc;
		break;
	    case CS__0:
		newstate = empty ? CS__0eb : CS__0cb;
		break;
	    case CS__0eb:
		newstate = empty ? CS__0eb : CS__ebc;
		s->state = newstate;
		if (me->fixed_line) {
		    if (empty)
			return lastcell->len <= 0 ? 0 : lastcell->len;
		    else
			return lastcell->len <= 0 ? 0 : -1;
		} else {
		    if (empty)
			return lastcell->len <= 0 ? 0 : lastcell->len;
		    else
			return lastcell->len <= 0 ? 0 : 0;
		}
	    case CS__0cb:
		if (!me->fixed_line) {
		    if (empty) { /* ##462_return_0 */
/*			if (s->icell_core == -1)
			    s->icell_core = lastcell->Line; */ /* we don't know yet */
			/* lastcell->Line = lineno; */
		    } else {	/* !empty */
			if (s->icell_core == -1)
			    me->Line = -1;
		    }
		}
		if (s->pending_len && empty) { /* ##470_why_that?? */
		    if ((me->fixed_line && me->Line == lastcell->Line) ||
			s->icell_core == me->ncells - 1)
			lastcell->len = s->pending_len;
		    s->pending_len = 0;
		} /* @@@ for empty do smth. about ->Line / ->icell_core !! */
		newstate = empty ? CS__0cb : CS__cbc; /* ##474_needs_len!=-1? */
		break;
	    case CS__0f:
	    case CS__0cf:
		break;
	    case CS_new:
		newstate = empty ? CS__eb : CS__cb;
		break;
	    case CS__eb:	/* ##484_set_pending_ret_0_if_empty? */
		newstate = empty ? CS__eb : CS__ebc;
		s->state = newstate;
		if (me->fixed_line) {
		    if (empty)
			return lastcell->len <= 0 ? 0 : lastcell->len;
		    else
			return lastcell->len <= 0 ? 0 : -1;
		} else {
		    if (empty)
			return lastcell->len <= 0 ? 0 : lastcell->len;
		    else
			return lastcell->len <= 0 ? 0 : -1;
		}
	    case CS__cb:
		if (s->pending_len && empty) { /* ##496: */
		    lastcell->len = s->pending_len;
		    s->pending_len = 0;
		} /* @@@ for empty do smth. about ->Line / ->icell_core !! */
		if (empty) {
		    if (!me->fixed_line) {
			me->fixed_line = YES;
			me->Line = lastcell->Line; /* should've happened in break */
		    } else {
			if (me->Line != lastcell->Line)
			    return -1;
		    }
		} else {
		    if (!me->fixed_line) {
			me->fixed_line = YES;
			me->Line = lastcell->Line; /* should've happened in break */
		    }
		    s->state = CS__cbc;
		    return -1;
		}
		newstate = empty ? CS__cb : CS__cbc;
		break;
	    case CS__ef:
		return 0;
	    case CS__cf:
		return lastcell->len; /* ##523_change_state? */
	    case CS__cbc:
		if (!me->fixed_line) {
		    if (empty) {
			if (s->icell_core == -1) /* ##528??: */
			    me->Line = lineno;
			/* lastcell->Line = lineno; */
		    } else {	/* !empty */
			if (s->icell_core == -1)
			    me->Line = -1;
		    }
		}
		s->pending_len = 0;
		newstate = empty ? CS_invalid : CS__cbc;
		break;
	    default:
		break;
	    }
	} else {		/* broken, certain: */
	    s->x_td = -1;
	    switch (s->state) {
	    case CS_invalid:
		/* ##540_return_-1_for_invalid_if_len!: */
		if (!empty && lastcell->len > 0) {
		    newstate = CS__0cf;
		    s->state = newstate;
		    return -1;
		}
				/* ##541_set_len_0_Line_-1_sometimes: */
		lastcell->len = 0;
		lastcell->Line = -1;
		 /* fall thru ##546 really fall thru??: */
		newstate = empty ? CS_invalid : CS__cbc;	break;
	    case CS__0:
		newstate = empty ? CS__0f  : CS__0cf;	break;
	    case CS__0eb:
		newstate = empty ? CS__0f  : CS__0cf;		/* ebc?? */
		s->state = newstate;
		if (me->fixed_line) {
		    if (empty)
			return lastcell->len <= 0 ? 0 : lastcell->len;
		    else
			return lastcell->len <= 0 ? 0 : -1;
		} else {
		    if (empty)
			return lastcell->len <= 0 ? 0 : lastcell->len;
		    else
			return lastcell->len <= 0 ? 0 : 0;
		}
	    case CS__0cb:
		if (s->pending_len) {
		    if (empty)
			lastcell->len = s->pending_len;
		    else
			lastcell->len = 0;
		    s->pending_len = 0;
		}
		if (!me->fixed_line) {
		    if (empty) {
			if (s->icell_core == -1)
			    s->icell_core = me->ncells - 1;
			/* lastcell->Line = lineno; */
		    } else {	/* !empty */
			if (s->icell_core == -1)
			    me->Line = -1;
		    }
		}
		if (s->pending_len && empty) {
		    lastcell->len = s->pending_len;
		    s->pending_len = 0;
		} /* @@@ for empty do smth. about ->Line / ->icell_core !! */
		newstate = empty ? CS__0cf : CS__cbc;	break;
	    case CS__0f:
		newstate = CS__0f;
		/* FALLTHRU */
	    case CS__0cf:
		break;
	    case CS_new:
		newstate = empty ? CS__ef  : CS__cf;	break;
	    case CS__eb:
		newstate = empty ? CS__ef  : CS__ef; /* ##579??? !!!!! */
		s->state = newstate;
		if (me->fixed_line) {
		    if (empty)
			return lastcell->len <= 0 ? 0 : lastcell->len;
		    else
			return lastcell->len <= 0 ? 0 : -1;
		} else {
		    if (empty)
			return lastcell->len <= 0 ? 0 : lastcell->len;
		    else
			return lastcell->len <= 0 ? 0 : -1;
		}
	    case CS__cb:
		if (s->pending_len && empty) {
		    lastcell->len = s->pending_len;
		    s->pending_len = 0;
		}
		if (empty) {
		    if (!me->fixed_line) {
			me->fixed_line = YES;
			me->Line = lastcell->Line; /* should've happened in break */
		    } else {
			if (me->Line != lastcell->Line)
			    return -1;
		    }
		} else {
		    return -1;
		}
		newstate = empty ? CS__cf  : CS__cbc;	break;
	    case CS__ef:		/* ignored error */
	    case CS__cf:		/* ignored error */
		break;
	    case CS__ebc:	/* ##540_handle_ebc: */
		lastcell->len = 0;
		if (!me->fixed_line) {
		    if (!empty) {
			if (s->icell_core == -1)
			    lastcell->Line = -1;
		    }
		}
		s->pending_len = 0;
		newstate = empty ? CS_invalid : CS__cbc;	break;
	    case CS__cbc:	/* ##586 */
		lastcell->len = 0; /* ##613 */
		if (me->fixed_line && me->Line == lastcell->Line)
		    return -1;
		if (!me->fixed_line) {
		    if (empty) {
			if (s->icell_core == -1)
			    me->Line = lineno;
			/* lastcell->Line = lineno; */
#if 0	/* ?? */
		    } else {	/* !empty */
			if (s->icell_core == -1)
			    me->Line = -1;
#endif
		    }
		}
		s->pending_len = 0; /* ##629 v */
		newstate = empty ? CS_invalid : CS__cbc;	break;
	    }
	}
    } else {			/* (!broken) */
	if (!certain) {
	    switch (s->state) {
	    case CS_invalid:
	    case CS__0:
		s->pending_len = empty ? 0 : pos - lastcell->pos;
		newstate = empty ? CS__0eb : CS__0cb;
		s->state = newstate;
		return 0; /* or 0 for xlen to s->pending_len?? */
	    case CS__0eb:	/* cannot happen */
		newstate = CS__eb;
		break;
	    case CS__0cb:	/* cannot happen */
		newstate = CS__cb;
		break;
	    case CS__0f:
	    case CS__0cf:
		break;
	    case CS_new:
		if (!empty && s->prev_state == CS__cbc)	/* ##609: */
		    return -1;
		if (!empty) {
		    if (!me->fixed_line) {
			me->fixed_line = YES;
			me->Line = lineno;
		    } else {
			if (me->Line != lineno)
			    return -1;
		    }
		}
		newstate = empty ? CS__eb : CS__cb;
		s->state = newstate;
		if (!me->fixed_line) {
		    s->pending_len = empty ? 0 : pos - lastcell->pos;
		    return 0;
		} else {
		    s->pending_len = 0;
		    lastcell->len = empty ? 0 : pos - lastcell->pos;
		    return lastcell->len;
		}
	    case CS__eb:	/* cannot happen */
		newstate = empty ? CS__eb : CS__ebc;	break;
	    case CS__cb:	/* cannot happen */
		newstate = empty ? CS__cb : CS__cbc;	break;
	    case CS__ef:
		return 0;
	    case CS__cf:
		return lastcell->len;
	    case CS__cbc:	/* ??? */
		break;
	    default:
		break;
	    }
	} else {		/* !broken, certain: */
	    s->x_td = -1;
	    switch (s->state) {
	    case CS_invalid:	/* ##691_no_lastcell_len_for_invalid: */
		if (!(me->fixed_line && me->Line == lastcell->Line))
		    lastcell->len = 0;
		/* FALLTHRU */
	    case CS__0:
		newstate = empty ? CS__0f  : CS__0cf;	break; /* ##630 */
	    case CS__0eb:
		newstate = empty ? CS__0f : CS__0f;	break; /* ??? */
	    case CS__0cb:
		newstate = empty ? CS__0cf : CS__cbc;	break; /* ??? */
	    case CS__0f:
		newstate = CS__0f;			break; /* ??? */
	    case CS__0cf:
		break;		/* ??? */
	    case CS_new:
		if (!empty && s->prev_state == CS__cbc)
		    return -1;
		if (!empty) { /* ##642_set_fixed!: */
		    if (!me->fixed_line) {
			me->fixed_line = YES;
			me->Line = lineno;
		    } else {
			if (me->Line != lineno)
			    return -1;
		    }
		}
		if (lastcell->len < 0)
		    lastcell->len = empty ? 0 : pos - lastcell->pos;
		newstate = empty ? CS__ef  : CS__cf;
		s->state = newstate;
		return (me->fixed_line && lineno != me->Line) ? -1 : lastcell->len;
	    case CS__eb:
		newstate = empty ? CS__ef  : CS__cf;	break; /* ??? */
	    case CS__cb:
		newstate = empty ? CS__cf  : CS__cf;	break; /* ??? */
	    case CS__ef:		/* ignored error */
	    case CS__cf:		/* ignored error */
	    default:
		break;
	    }
	    lastcell->len = pos - lastcell->pos;
	} /* if (!certain) ... else */
    } /* if (broken) ... else */

#if 0				/* MEGA_COMMENTOUT */
    if (lineno != me->cells[0].Line) {
#if 0
	int i;
	for (i = ncells - 1; i >= 0; i--) {
	    if (!(me->cells[i].len == 0 || me->cells[i].colspan == 0))
		break;
	}
#endif
	if (lineno >= lastcell->Line) {
	    if (me->fixed_line) {
		if (pos == 0) {
		    if (lastcell->len <= 0) {
			return 0;
		    } else {
			return lastcell->len;
		    }
		} else {	/* pos != 0 */
		    if (lastcell->len <= 0 && lineno > lastcell->Line && lastcell->Line <= me->Line) {
			return 0;
		    } else {
			return -1;
		    }
		}
	    } else {	/* not me->fixed_line */
		if (pos == 0) {
		    if (lastcell->len <= 0) {
			return 0;
		    } else {
			return lastcell->len;
		    }
		} else {	/* pos != 0 */
		    if (lastcell->len <= 0) {
			return 0;
		    } else {
			if (me->ncells == 1 || lastcell->pos == 0) {
			    return 0;
			} else
			    return -1;
		    }
		}
	    }
	}
    }
#endif /* MEGA_COMMENTOUT */
    s->state = newstate;
/*    lastcell->len = pos - lastcell->pos; */
    return (lastcell->len);
}

/*
 *  Reserve cells, each of given colspan, in (rowspan-1) rows after
 *  the current row of rowspan>1.  If rowspan==0, use special 'row'
 *  rowspans2eog to keep track of rowspans that are to remain in effect
 *  until the end of the row group (until next THEAD/TFOOT/TBODY) or table.
 */
PRIVATE int Stbl_reserveCellsInTable ARGS4(
    STable_info *,	me,
    int,		icell,
    int,		colspan,
    int,		rowspan)
{
    STable_rowinfo *rows, *row;
    int growby;
    int i;
    if (me->nrows <= 0)
	return -1;		/* must already have at least one row */

    if (rowspan == 0) {
	if (!me->rowspans2eog.cells) {
	    me->rowspans2eog.cells = typecallocn(STable_cellinfo, icell + colspan);
	    if (!me->rowspans2eog.cells)
		return 0;	/* fail silently */
	    else
		me->rowspans2eog.allocated = icell + colspan;
	}
	Stbl_reserveCellsInRow(&me->rowspans2eog, icell, colspan);
    }

    growby = me->nrows + rowspan - 1 - me->allocated_rows;
    if (growby > 0) {
	rows = realloc(me->rows,
		       (me->allocated_rows + growby)
		       * sizeof(STable_rowinfo));
	if (!rows)
	    return 0; /* ignore silently, no free memory, may be recoverable */
	for (i = 0; i < growby; i++) {
	    row = rows + me->allocated_rows + i;
	    row->allocated = 0;
	    if (!me->rowspans2eog.allocated) {
		row->cells = NULL;
	    } else {
		row->cells = typecallocn(STable_cellinfo,
					 me->rowspans2eog.allocated);
		if (row->cells) {
		    row->allocated = me->rowspans2eog.allocated;
		    memcpy(row->cells, me->rowspans2eog.cells,
			   row->allocated * sizeof(STable_cellinfo));
		}
	    }
	    row->ncells = 0;
	    row->fixed_line = NO;
	    row->alignment = HT_ALIGN_NONE;
	}
	me->allocated_rows += growby;
	me->rows = rows;
    }
    for (i = me->nrows;
	 i < (rowspan == 0 ? me->allocated_rows : me->nrows + rowspan - 1);
	 i++) {
	if (!me->rows[i].allocated) {
	    me->rows[i].cells = typecallocn(STable_cellinfo, icell + colspan);
	    if (!me->rows[i].cells)
		return 0;	/* fail silently */
	    else
		me->rows[i].allocated = icell + colspan;
	}
	Stbl_reserveCellsInRow(me->rows + i, icell, colspan);
    }
    return 0;
}

/* Remove reserved cells in trailing rows that were added for rowspan,
 * to be used when a THEAD/TFOOT/TBODY ends. */
PRIVATE void Stbl_cancelRowSpans ARGS1(
    STable_info *,	me)
{
    int i;
    for (i = me->nrows; i < me->allocated_rows; i++) {
	if (!me->rows[i].ncells) { /* should always be the case */
	    FREE(me->rows[i].cells);
	    me->rows[i].allocated = 0;
	}
    }
    free_rowinfo(&me->rowspans2eog);
    me->rowspans2eog.allocated = 0;
}

/*
 * Returns -1 on error, otherwise index of just-added table row.
 */
PUBLIC int Stbl_addRowToTable ARGS3(
    STable_info *,	me,
    short,		alignment,
    int,		lineno)
{
    STable_rowinfo *rows, *row;
    STable_states * s = &me->s;
    if (me->nrows > 0 && me->rows[me->nrows-1].ncells > 0) {
	if (s->pending_len > 0)
	    me->rows[me->nrows-1].cells[me->rows[me->nrows-1].ncells - 1].len = s->pending_len;
	s->pending_len = 0;
/*	if (me->rows[me->nrows-1].cells[me->rows[me->nrows-1].ncells - 1].len >= 0 &&
	    me->rows[me->nrows-1].cells[me->rows[me->nrows-1].ncells - 1].Line == lineno)
	    Stbl_finishCellInTable(me, YES,
				   lineno,
		me->rows[me->nrows-1].cells[me->rows[me->nrows-1].ncells - 1].pos +
		me->rows[me->nrows-1].cells[me->rows[me->nrows-1].ncells - 1].len); */
    }
#if 0
    s->prev_state = s->state = CS_invalid;
    s->lineno = -1;
    s->icell_core = -1;
#endif
    Stbl_finishRowInTable(me);
    if (me->nrows > 0 && me->rows[me->nrows-1].Line == lineno)
	me->rows[me->nrows-1].Line = -1;
    s->pending_len = 0;
    s->x_td = -1;

    {
	int i;
	int growby = 0;
	while (me->nrows + 2 > me->allocated_rows + growby)
	    growby += ROWS_GROWBY;
	if (growby) {
	    if (me->allocated_rows == 0 && !me->rows) {
		rows = typecallocn(STable_rowinfo, growby);
	    } else {
		rows = realloc(me->rows,
				  (me->allocated_rows + growby)
				  * sizeof(STable_rowinfo));
		for (i = 0; rows && i < growby; i++) {
		    row = rows + me->allocated_rows + i;
		    if (!me->rowspans2eog.allocated) {
			row->allocated = 0;
			row->cells = NULL;
		    } else {
			row->cells = typecallocn(STable_cellinfo,
						 me->rowspans2eog.allocated);
			if (row->cells) {
			    row->allocated = me->rowspans2eog.allocated;
			    memcpy(row->cells, me->rowspans2eog.cells,
				   row->allocated * sizeof(STable_cellinfo));
			} else {
			    FREE(rows);
			    break;
			}
		    }
		    row->ncells = 0;
		    row->fixed_line = NO;
		    row->alignment = HT_ALIGN_NONE;
		}
	    }
	    if (rows) {
		me->allocated_rows += growby;
		me->rows = rows;
	    } else {
		return -1;
	    }
	}
    }

    me->rows[me->nrows].Line = lineno;
    if (me->nrows == 0)
	me->startline = lineno;
    if (alignment != HT_ALIGN_NONE)
	me->rows[me->nrows].alignment = alignment;
    else
	me->rows[me->nrows].alignment =
	    (me->rowgroup_align==HT_ALIGN_NONE) ?
				  me->alignment : me->rowgroup_align;
    me->nrows++;
    if (me->pending_colgroup_next > me->ncolinfo) {
	me->ncolinfo = me->pending_colgroup_next;
	me->pending_colgroup_next = 0;
    }
    me->rows[me->nrows].Line = -1; /* not yet used */
    return (me->nrows - 1);
}

/*
 * Returns -1 on error, otherwise current number of rows.
 */
PRIVATE int Stbl_finishRowInTable ARGS1(
    STable_info *,	me)
{
    STable_rowinfo *lastrow;
    STable_states * s = &me->s;
    int ncells;
    if (!me->rows || !me->nrows)
	return -1;		/* no row started! */
    lastrow = me->rows + (me->nrows - 1);
    ncells = lastrow->ncells;
    if (lastrow->ncells > 0) {
	if (s->pending_len > 0)
	    lastrow->cells[lastrow->ncells - 1].len = s->pending_len;
	s->pending_len = 0;
    }
    s->prev_state = s->state = CS_invalid;
    s->lineno = -1;

#if 0
    if (lastrow->Line == -1 && s->icell_core >= 0)
#endif
    if (s->icell_core >= 0 && !lastrow->fixed_line &&
	lastrow->cells[s->icell_core].Line >= 0)
	lastrow->Line = lastrow->cells[s->icell_core].Line;
    s->icell_core = -1;
    return (me->nrows);
}

PRIVATE void update_sumcols0 ARGS7(
    STable_cellinfo *,	sumcols,
    STable_rowinfo *,	lastrow,
    int,		pos,
    int,		len,
    int,		icell,
    int,		ispan,
    int,		allocated_sumcols)
{
    int i;
    if (len > 0) {
	int sumpos = pos;
	int prevsumpos = sumcols[icell + ispan].pos;
	int advance;
	if (ispan > 0) {
	    if (lastrow->cells[icell].pos + len > sumpos)
		sumpos = lastrow->cells[icell].pos + len;
	    if (sumcols[icell+ispan-1].pos + sumcols[icell+ispan-1].len > sumpos)
		sumpos = sumcols[icell+ispan-1].pos + sumcols[icell+ispan-1].len;
	}
	advance = sumpos - prevsumpos;
	if (advance > 0) {
	    for (i = icell + ispan; i < allocated_sumcols; i++) {
		if (ispan > 0 && sumcols[i].colspan < -1) {
		    if (i + sumcols[i].colspan < icell + ispan) {
			advance = sumpos - sumcols[i].pos;
			if (i > 0)
			    advance = HTMAX(advance,
					    sumcols[i-1].pos + sumcols[i-1].len
					    - (sumcols[i].pos));
			if (advance <= 0)
			    break;
		    }
		}
		if (sumcols[i].pos >= 0)
		    sumcols[i].pos += advance;
		else {
		    sumcols[i].pos = sumpos;
		    break;
		}
	    }
	}
    }
}

PRIVATE int get_remaining_colspan ARGS5(
    STable_rowinfo *,	me,
    STable_cellinfo *,	colinfo,
    int,		ncolinfo,
    int,		colspan,
    int,		ncols_sofar)
{
    int i;
    int last_colspan = me->ncells ?
	me->cells[me->ncells - 1].colspan : 1;

    if (ncolinfo == 0 || me->ncells + last_colspan > ncolinfo) {
	colspan = HTMAX(TRST_MAXCOLSPAN,
			ncols_sofar - (me->ncells + last_colspan - 1));
    } else {
	for (i = me->ncells + last_colspan - 1; i < ncolinfo - 1; i++)
	    if (colinfo[i].Line == EOCOLG)
		break;
	colspan = i - (me->ncells + last_colspan - 2);
    }
    return colspan;
}

/*
 * Returns -1 on error, otherwise 0.
 */
PUBLIC int Stbl_addCellToTable ARGS7(
    STable_info *,	me,
    int,		colspan,
    int,		rowspan,
    short,		alignment,
    BOOL,		isheader,
    int,		lineno,
    int,		pos)
{
    STable_states * s = &me->s;
    STable_rowinfo *lastrow;
    STable_cellinfo *sumcols, *sumcol;
    int i, icell, ncells, sumpos;
#if 0
    int prevsumpos, advance;
#endif

    if (!me->rows || !me->nrows)
	return -1;		/* no row started! */
				/* ##850_fail_if_fail?? */
    Stbl_finishCellInTable(me, YES,
			   lineno, pos);
    lastrow = me->rows + (me->nrows - 1);
    if (colspan == 0) {
	colspan = get_remaining_colspan(lastrow, me->sumcols, me->ncolinfo,
					colspan, me->ncols);
    }
    ncells = lastrow->ncells;	/* remember what it was before adding cell. */
    icell = Stbl_addCellToRow(lastrow, me->sumcols, me->ncolinfo, s,
			      colspan, alignment, isheader,
			      lineno, &pos);
    if (icell < 0)
	return icell;
    if (me->nrows == 1 && me->startline < lastrow->Line)
	me->startline = lastrow->Line;

    if (rowspan != 1) {
	Stbl_reserveCellsInTable(me, icell, colspan, rowspan);
	/* me->rows may now have been realloc'd, make lastrow valid pointer */
	lastrow = me->rows + (me->nrows - 1);
    }

    {
	int growby = 0;
	while (icell + colspan + 1 > me->allocated_sumcols + growby)
	    growby += CELLS_GROWBY;
	if (growby) {
	    if (me->allocated_sumcols == 0 && !me->sumcols) {
		sumcols = typecallocn(STable_cellinfo, growby);
	    } else {
		sumcols = realloc(me->sumcols,
				  (me->allocated_sumcols + growby)
				  * sizeof(STable_cellinfo));
		for (i = 0; sumcols && i < growby; i++) {
		    sumcol = sumcols + me->allocated_sumcols + i;
		    sumcol->pos = sumcols[me->allocated_sumcols-1].pos;
		    sumcol->len = 0;
		    sumcol->colspan = 0;
		    sumcol->Line = 0;
		    sumcol->alignment = HT_ALIGN_NONE;
		}
	    }
	    if (sumcols) {
		me->allocated_sumcols += growby;
		me->sumcols = sumcols;
	    } else {
		return -1;
	    }
	}
    }
#if 0
    if (icell + colspan > me->ncols) {
	me->sumcols[icell + colspan].pos = -1; /* not yet used @@@ ??? */
    }
#endif
    if (icell + 1 > me->ncols) {
	me->ncols = icell + 1;
    }
    if (colspan > 1 && colspan + me->sumcols[icell + colspan].colspan > 0)
	me->sumcols[icell + colspan].colspan = -colspan;
    sumpos = pos;
    if (ncells > 0)
	sumpos += me->sumcols[ncells-1].pos - lastrow->cells[ncells-1].pos;
    update_sumcols0(me->sumcols, lastrow, sumpos,
		    sumpos - (ncells > 0 ? me->sumcols[icell].pos : me->sumcols[icell].pos),
		    icell, 0, me->allocated_sumcols);
#if 0
    prevsumpos = me->sumcols[icell].pos;
    advance = sumpos - prevsumpos;
    if (advance > 0) {
	for (i = icell; i < me->allocated_sumcols; i++) {
	    if (me->sumcols[i].pos >= 0)
		me->sumcols[i].pos += advance;
	    else {
		me->sumcols[i].pos = sumpos;
		break;
	    }
	}
    }
#endif


#if 0
	int prevopos = (ncells > 0 ? lastrow->cells[ncells-1].pos : 0);
	int prevnpos = (ncells > 0 ? me->sumcols[ncells-1].pos : 0);
#endif
#if 0
    if (pos > me->maxpos) {
	me->maxpos = pos;
	if (me->maxpos > /* @@@ max. line length we can accept */ MAX_STBL_POS)
	    return -1;
    }
#endif
    me->maxpos = me->sumcols[me->allocated_sumcols-1].pos;
    if (me->maxpos > /* @@@ max. line length we can accept */ MAX_STBL_POS)
	return -1;
    return 0;
}

/*
 * Returns -1 on error, otherwise 0.
 */
PUBLIC int Stbl_finishCellInTable ARGS4(
    STable_info *,	me,
    BOOL,		certain,
    int,		lineno,
    int,		pos)
{
    STable_states * s = &me->s;
    STable_rowinfo *lastrow;
    int len, xlen, icell;
    int i;
    if (me->nrows == 0)
	return -1;
    lastrow = me->rows + (me->nrows - 1);
    icell = lastrow->ncells - 1;
    if (icell < 0)
	return icell;
    if (s->x_td == -1)
	return certain ? -1 : 0;
    len = Stbl_finishCellInRow(lastrow, s, certain, lineno, pos);
    if (len == -1)
	return len;
    xlen = (len > 0) ? len : s->pending_len; /* ##890 use xlen if fixed_line?: */
    if (lastrow->fixed_line && lastrow->Line == lineno)
	len = xlen;
    if (lastrow->cells[icell].colspan > 1) {
	/*
	 * @@@ This is all a too-complicated mess; do we need
	 * sumcols len at all, or is pos enough??
	 * Answer: sumcols len is at least used for center/right
	 * alignment, and should probably continue to be used there;
	 * all other uses are probably not necessary.
	 */
	int spanlen = 0, spanlend = 0;
	for (i = icell; i < icell + lastrow->cells[icell].colspan; i++) {
	    if (me->sumcols[i].len > 0) {
		spanlen += me->sumcols[i].len;
		if (i > icell)
		    spanlen++;
	    }
	    spanlend = HTMAX(spanlend,
			     me->sumcols[i+1].pos - me->sumcols[icell].pos);
	}
	if (spanlend)
	    spanlend--;
	if (spanlend > spanlen)
	    spanlen = spanlend;
	/* @@@ could overcount? */
	if (len > spanlen)
	    me->maxlen += (len - spanlen);
#if 0	/* this is all quite bogus! */
	if (me->sumcols[icell].colspan > 1)
	    me->sumcols[icell+me->sumcols[icell].colspan].pos =
		HTMAX(me->sumcols[icell].pos + len,
		      me->sumcols[icell+me->sumcols[icell].colspan].pos);
	if (lastrow->cells[icell].colspan > me->sumcols[icell].colspan) {
	    me->sumcols[icell].colspan = lastrow->cells[icell].colspan;
	    if (me->sumcols[icell].colspan > 1)
		me->sumcols[icell+me->sumcols[icell].colspan].pos =
		    HTMAX(me->sumcols[icell].pos + len,
			  me->sumcols[icell+me->sumcols[icell].colspan].pos);
	}
#endif
    } else if (len > me->sumcols[icell].len) {
	if (me->sumcols[icell + 1].colspan >= -1)
	    me->maxlen += (len - me->sumcols[icell].len);
	me->sumcols[icell].len = len;
    }

    if (len > 0) {
	update_sumcols0(me->sumcols, lastrow, pos, len,
			icell, lastrow->cells[icell].colspan,
			me->allocated_sumcols);
	me->maxpos = me->sumcols[me->allocated_sumcols-1].pos;
    }
#if 0
    if (len > 0) {
	int sumpos = pos;
	int ispan = lastrow->cells[icell].colspan;
	int prevsumpos = me->sumcols[icell + ispan].pos;
	int advance;
	if (lastrow->cells[icell].pos + len > sumpos)
	    sumpos = lastrow->cells[icell].pos + len;
	if (me->sumcols[icell+ispan-1].pos + me->sumcols[icell+ispan-1].len > sumpos)
	    sumpos = me->sumcols[icell+ispan-1].pos + me->sumcols[icell+ispan-1].len;
	advance = sumpos - prevsumpos;
	if (advance > 0) {
	    for (i = icell + ispan; i < me->allocated_sumcols; i++) {
		if (me->sumcols[i].colspan < -1) {
		    if (i + me->sumcols[i].colspan < icell + ispan) {
			advance = sumpos - me->sumcols[i].pos;
			if (i > 0)
			    advance = HTMAX(advance,
					    me->sumcols[i-1].pos + me->sumcols[i-1].len
					    - (me->sumcols[i].pos));
			if (advance <= 0)
			    break;
		    }
		}
		if (me->sumcols[i].pos >= 0)
		    me->sumcols[i].pos += advance;
		else {
		    me->sumcols[i].pos = sumpos;
		    break;
		}
	    }
	}
	me->maxpos = me->sumcols[me->allocated_sumcols-1].pos;
    }
#endif

    if (me->maxlen + (xlen - len) > MAX_STBL_POS)
	return -1;
    if (me->maxpos > /* @@@ max. line length we can accept */ MAX_STBL_POS)
	return -1;

    if (lineno != lastrow->Line) {
	/* @@@ Do something here?  Or is it taken care of in
	   Stbl_finishCellInRow ? */
    }

    return 0;
}

/*
 * Returns -1 on error, otherwise 0.
 */
PUBLIC int Stbl_addColInfo ARGS4(
    STable_info *,	me,
    int,		colspan,
    short,		alignment,
    BOOL,		isgroup)
{
    STable_cellinfo *sumcols, *sumcol;
    int i, icolinfo;

    if (isgroup) {
	if (me->pending_colgroup_next > me->ncolinfo)
	    me->ncolinfo = me->pending_colgroup_next;
	me->pending_colgroup_next = me->ncolinfo + colspan;
	if (me->ncolinfo > 0)
	    me->sumcols[me->ncolinfo -  1].Line = EOCOLG;
	me->pending_colgroup_align = alignment;
    } else {
	for (i = me->pending_colgroup_next - 1;
	     i >= me->ncolinfo + colspan; i--)
	    me->sumcols[i].alignment = HT_ALIGN_NONE;
	me->pending_colgroup_next = me->ncolinfo + colspan;
    }
    icolinfo = me->ncolinfo;
    if (!isgroup)
	me->ncolinfo += colspan;

    {
	int growby = 0;
	while (icolinfo + colspan + 1 > me->allocated_sumcols + growby)
	    growby += CELLS_GROWBY;
	if (growby) {
	    if (me->allocated_sumcols == 0) {
		sumcols = typecallocn(STable_cellinfo, growby);
	    } else {
		sumcols = realloc(me->sumcols,
				  (me->allocated_sumcols + growby)
				  * sizeof(STable_cellinfo));
		for (i = 0; sumcols && i < growby; i++) {
		    sumcol = sumcols + me->allocated_sumcols + i;
		    sumcol->pos = sumcols[me->allocated_sumcols-1].pos;
		    sumcol->len = 0;
		    sumcol->colspan = 0;
		    sumcol->Line = 0;
		}
	    }
	    if (sumcols) {
		me->allocated_sumcols += growby;
		me->sumcols = sumcols;
	    } else {
		return -1;
	    }
	}
    }

    if (alignment==HT_ALIGN_NONE)
	alignment = me->pending_colgroup_align;
    for (i = icolinfo; i < icolinfo + colspan; i++) {
	me->sumcols[i].alignment = alignment;
    }
    return 0;
}

/*
 * Returns -1 on error, otherwise 0.
 */
PUBLIC int Stbl_finishColGroup ARGS1(
    STable_info *,	me)
{
    if (me->pending_colgroup_next >= me->ncolinfo) {
	me->ncolinfo = me->pending_colgroup_next;
	if (me->ncolinfo > 0)
	    me->sumcols[me->ncolinfo -  1].Line = EOCOLG;
    }
    me->pending_colgroup_next = 0;
    me->pending_colgroup_align = HT_ALIGN_NONE;
    return 0;
}

PUBLIC int Stbl_addRowGroup ARGS2(
    STable_info *,	me,
    short,		alignment)
{
    Stbl_cancelRowSpans(me);
    me->rowgroup_align = alignment;
    return 0;			/* that's all! */
}

PUBLIC int Stbl_finishTABLE ARGS1(
    STable_info *,	me)
{
    STable_states * s = &me->s;
    int i;
    int curpos = 0;

    if (!me || me->nrows <= 0 || me->ncols <= 0) {
	return -1;
    }
    if (me->nrows > 0 && me->rows[me->nrows-1].ncells > 0) {
	if (s->pending_len > 0)
	    me->rows[me->nrows-1].cells[me->rows[me->nrows-1].ncells - 1].len = s->pending_len;
	s->pending_len = 0;
/*	if (me->rows[me->nrows-1].cells[me->rows[me->nrows-1].ncells - 1].len >= 0)
	    Stbl_finishCellInTable(me, YES,
		me->rows[me->nrows-1].cells[me->rows[me->nrows-1].ncells - 1].Line,
		me->rows[me->nrows-1].cells[me->rows[me->nrows-1].ncells - 1].pos +
		me->rows[me->nrows-1].cells[me->rows[me->nrows-1].ncells - 1].len); */
    }
    Stbl_finishRowInTable(me);
    for (i = 0; i < me->ncols; i++) {
	if (me->sumcols[i].pos < curpos) {
	    me->sumcols[i].pos = curpos;
	} else {
	    curpos = me->sumcols[i].pos;
	}
	if (me->sumcols[i].len > 0) {
	    curpos += me->sumcols[i].len;
	}
    }
#if 0				/* ??? */
    for (j = 0; j < me->nrows; j++) {
	STable_rowinfo *row = me->rows + i;
	for (i = 0; i < row->ncells; i++) {
	}
    }
#endif
    return me->ncols;
}

PUBLIC short Stbl_getAlignment ARGS1(
    STable_info *,	me)
{
    return me ? me->alignment : HT_ALIGN_NONE;
}

PRIVATE int get_fixup_positions ARGS4(
    STable_rowinfo *,	me,
    int *,		oldpos,
    int *,		newpos,
    STable_cellinfo *,	sumcols)
{
    int i = 0, ip = 0;
    int next_i, newlen;
    int ninserts;

    if (!me)
	return -1;
    while (i < me->ncells) {
	next_i = i + HTMAX(1, me->cells[i].colspan);
	if (me->cells[i].Line != me->Line) {
	    if (me->cells[i].Line > me->Line)
		break;
	    i = next_i;
	    continue;
	}
	oldpos[ip] = me->cells[i].pos;
	newpos[ip] = sumcols[i].pos;
	if ((me->cells[i].alignment == HT_CENTER ||
	     me->cells[i].alignment == HT_RIGHT) &&
	    me->cells[i].len > 0) {
	    newlen = sumcols[next_i].pos - newpos[ip] - 1;
	    newlen = HTMAX(newlen, sumcols[i].len);
	    if (me->cells[i].len < newlen) {
		if (me->cells[i].alignment == HT_RIGHT) {
		    newpos[ip] += newlen - me->cells[i].len;
		} else {
		    newpos[ip] += (newlen - me->cells[i].len) / 2;
		}
	    }
	}
	ip++;
	i = next_i;
    }
    ninserts = ip;
    return ninserts;
}

/*
 *  Returns -1 if we have no row for this lineno, or for other error,
 *           0 or greater (number of oldpos/newpos pairs) if we have
 *             a table row.
 */
PUBLIC int Stbl_getFixupPositions ARGS4(
    STable_info *,	me,
    int,		lineno,
    int *,		oldpos,
    int *,		newpos)
{
    STable_rowinfo * row;
    int j;
    int ninserts = -1;
    if (!me || !me->nrows)
	return -1;
    for (j = 0; j < me->nrows; j++) {
	row = me->rows + j;
	if (row->Line == lineno) {
	    ninserts = get_fixup_positions(row, oldpos, newpos,
					   me->sumcols);
	    break;
	}
    }
    return ninserts;
}

PUBLIC int Stbl_getStartLine ARGS1(
    STable_info *,	me)
{
    if (!me)
	return -1;
    else
	return me->startline;
}
