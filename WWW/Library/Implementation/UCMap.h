typedef long UCode_t;

PUBLIC int UCTransUniChar PARAMS((UCode_t unicode, int charset_out));
PUBLIC int UCTransUniCharStr PARAMS((char *outbuf, int buflen, UCode_t unicode,
			     int charset_out, int chk_single_flag));
PUBLIC int UCTransChar PARAMS((char ch_in, int charset_in, int charset_out));
PUBLIC int UCTransCharStr PARAMS((char *outbuf, int buflen, char ch_in,
			int charset_in, int charset_out, int chk_single_flag));
PUBLIC UCode_t UCTransToUni PARAMS((char ch_in, int charset_in));
PUBLIC int UCGetLYhndl_byMIME PARAMS((CONST char *p));
PUBLIC int UCGetRawUniMode_byLYhndl PARAMS((int i));

PUBLIC int UCLYhndl_for_unspec;
PUBLIC int UCLYhndl_for_unrec;
PUBLIC int UCLYhndl_HTFile_for_unspec;
PUBLIC int UCLYhndl_HTFile_for_unrec;

#define UCTRANS_NOTFOUND (-4)

#define HT_CANNOT_TRANSLATE -4	/* could go into HTUtils.h */
