
#ifndef HTFORMS_H
#define HTFORMS_H

#ifndef LYSTRUCTS_H
#include "LYStructs.h"
#endif /* LYSTRUCTS_H */

/* in LYForms.c */
extern int change_form_link PARAMS((struct link *form_link, int mode,
                                    document *newdoc, BOOLEAN *refresh_screen,
				    char *link_name, char *link_value));

/* InputFieldData is used to pass the info between
 * HTML.c and Gridtext.c in HText_beginInput()
 */
typedef struct _InputFieldData {
	char *accept;
	char *align;
	int   checked;
	char *class;
	int   disabled;
	char *error;
	char *height;
	char *id;
	char *lang;
	char *max;
	char *maxlength;
	char *md;
	char *min;
    	char *name;
	char *size;
	char *src;
	char *type;
	char *value;
	char *width;
} InputFieldData;

/* The OptionType structure is for a linked list of option entries
 */
typedef struct _OptionType {
	char *			name;		 /* the name of the entry */
	char *			cp_submit_value; /* the value to submit	  */
	struct _OptionType *	next;		 /* the next entry	  */
} OptionType;

/* the FormInfo structure is used to contain the form field
 * data within each anchor
 * A pointer to this structure is in the TextAnchor struct.
 */
typedef struct _FormInfo {
	char *			name;	   /* the name of the link */
	int			number;	   /* which form is the link within */
	int			type;	   /* string, int, etc. */
	char *			value;	   /* user entered string data */
	char *			orig_value;/* the original value */
	int			size;	   /* width on the screen */
	int			maxlength; /* max width of data */
	int			group;	   /* a group associated with the link
					    *  this is used for select's
					    */
	int			num_value; /* value of the numerical fields */
	int 			hrange;	   /* high numerical range */
	int			lrange;	   /* low numerical range */
	OptionType *		select_list; /* array of option choices */
        char *                  submit_action;  /* form's action */
        int                     submit_method;  /* form's method */
        char *                  submit_enctype; /* form's entype */
	char *			submit_title;	/* form's title */
	BOOL			no_cache;  /* Always resubmit? */
	char *			cp_submit_value; /* option value to submit */
        char *			orig_submit_value; /* original submit value */
	int			size_l;	   /* The length of the option list */
	int			disabled;  /* If YES, can't change values */
} FormInfo;

#define HYPERTEXT_ANCHOR 1
#define INPUT_ANCHOR     2   /* forms mode input fields */

#define F_TEXT_TYPE	   1
#define F_PASSWORD_TYPE    2
#define F_CHECKBOX_TYPE    3
#define F_RADIO_TYPE	   4
#define F_SUBMIT_TYPE	   5
#define F_RESET_TYPE	   6
#define F_OPTION_LIST_TYPE 7
#define F_HIDDEN_TYPE      8
#define F_TEXTAREA_TYPE    9
#define F_RANGE_TYPE      10
#define F_FILE_TYPE       11
#define F_TEXT_SUBMIT_TYPE 12

#define WWW_FORM_LINK_TYPE  1
#define WWW_LINK_TYPE   2
/* #define different lynx modes */
#define NORMAL_LYNX_MODE 1
#define FORMS_LYNX_MODE  2

#define FORM_UP   1
#define FORM_DOWN 2

#define FIRST_ORDER  1
#define MIDDLE_ORDER 2
#define LAST_ORDER   3

#endif /* HTFORMS_H */
