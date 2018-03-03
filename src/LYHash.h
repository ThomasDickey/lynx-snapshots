/* $LynxId: LYHash.h,v 1.31 2018/03/03 15:20:39 tom Exp $ */
#ifndef _LYHASH_H_
#define _LYHASH_H_ 1

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
    typedef struct {
	char *name;		/* name of this item */
	int code;		/* code of this item */
	int color;		/* color highlighting to be done */
	int mono;		/* mono highlighting to be done */
	int cattr;		/* attributes to go with the color */
    } bucket;

#ifndef CSHASHSIZE
#define CSHASHSIZE 9973		/* Arbitrary prime.  Memory/speed tradeoff */
#endif

#define NOSTYLE -1

    /* hashStyles[] is used in CTRACE when NOSTYLE is passed as 'style' to
     * curses_w_style
     */
    extern bucket hashStyles[CSHASHSIZE];
    extern int hash_code(const char *string);
    extern bucket *nostyle_bucket(void);

    extern int hash_code_caseless(const char *string);
    extern int hash_code_aggregate(const char *p, const char *q, const char *r);
    extern int hash_code_aggregate_lower_str(const char *c, int hash_was);

    extern int s_a;
    extern int s_aedit;
    extern int s_aedit_arr;
    extern int s_aedit_pad;
    extern int s_aedit_sel;
    extern int s_alert;
    extern int s_alink;
    extern int s_curedit;
    extern int s_forw_backw;
    extern int s_hot_paste;
    extern int s_menu_active;
    extern int s_menu_bg;
    extern int s_menu_entry;
    extern int s_menu_frame;
    extern int s_menu_number;
    extern int s_menu_sb;
    extern int s_normal;
    extern int s_prompt_edit;
    extern int s_prompt_edit_arr;
    extern int s_prompt_edit_pad;
    extern int s_prompt_sel;
    extern int s_status;
    extern int s_title;
    extern int s_whereis;

#ifdef USE_SCROLLBAR
    extern int s_sb_aa;
    extern int s_sb_bar;
    extern int s_sb_bg;
    extern int s_sb_naa;
#endif

#ifdef __cplusplus
}
#endif
#endif				/* _LYHASH_H_ */
