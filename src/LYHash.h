/* $LynxId: LYHash.h,v 1.40 2018/03/10 01:47:33 tom Exp $ */
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
	BOOL used;		/* color/attributes have been assigned */
	int color;		/* color highlighting to be done */
	int mono;		/* mono highlighting to be done */
	int cattr;		/* attributes to go with the color */
    } bucket;

#define CSHASHSIZE 9973		/* Arbitrary prime.  Memory/speed tradeoff */

#define NOSTYLE -1

    extern bucket hashStyles[CSHASHSIZE];
    extern bucket *nostyle_bucket(void);

    extern int color_style_1(const char *string);
    extern int color_style_3(const char *p, const char *q, const char *r);
    extern void free_hashStyles(void);
    extern void report_hashStyles(void);

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
