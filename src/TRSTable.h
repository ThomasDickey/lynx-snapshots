typedef struct _STable_info STable_info;
extern STable_info * Stbl_startTABLE PARAMS((short));
extern int Stbl_finishTABLE PARAMS((STable_info *));
extern void Stbl_free PARAMS((STable_info *));
extern int Stbl_addRowToTable PARAMS((STable_info *, short, int));
extern int Stbl_addCellToTable PARAMS((STable_info *, int, short, BOOL, int, int));
extern int Stbl_finishCellInTable PARAMS((STable_info *, BOOL, int, int));
#define Stbl_lineBreak(stbl,l,pos) Stbl_finishCellInTable(stbl, NO, l, pos)
extern int Stbl_getStartLine PARAMS((STable_info *));
extern int Stbl_getFixupPositions PARAMS((
    STable_info *	me,
    int			lineno,
    int *		oldpos,
    int *		newpos));
extern short Stbl_getAlignment PARAMS((STable_info *));
