/*
 * [old comments: - kw ]
 * consolemap.h
 *
 * Interface between console.c, selection.c  and UCmap.c
 */
#define LAT1_MAP 0
#define GRAF_MAP 1
#define IBMPC_MAP 2
#define USER_MAP 3

#ifndef MAX_CHARSETS
#define MAX_CHARSETS
#endif

extern int hashtable_contents_valid;
extern unsigned char inverse_translate PARAMS((int glyph));
extern u16 *set_translate PARAMS((int m));
extern int conv_uni_to_pc PARAMS((long ucs));

extern int hashtable_str_contents_valid;   /* ??? probably no use... */

/* Some conventions i try to follow (loosely):
   [a-z]* only internal, names from linux driver code.
   UC_* to be only known internally.
   UC[A-Z]* to be exported to other parts of Lynx. -kw
*/

extern void UC_Charset_Setup PARAMS((char * UC_MIMEcharset, char * UC_LYNXcharset,
		      u8 * unicount, u16 * unitable, int nnuni,
		      struct unimapdesc_str replacedesc, int lowest_eight,
			     int UC_rawuni));

char *UC_GNsetMIMEnames[4] =
           {"iso-8859-1","x-dec-graphics","cp437","x-transparent"};
int UC_GNhandles[4] = {-1, -1, -1, -1};

struct UC_charset {
  char *MIMEname;
  char *LYNXname;
  u8* unicount;
  u16* unitable;
  int num_uni;
  struct unimapdesc_str replacedesc;
  int uc_status;
  int LYhndl;
  int GN;
  int lowest_eight;
  int enc;
};
char * UC_charsetMIMEnames[MAX_CHARSETS];

PUBLIC struct UC_charset UCInfo[MAX_CHARSETS];

PUBLIC int UCNumCharsets;

extern void UCInit NOARGS;

