
#ifndef LYOPTIONS_H
#define LYOPTIONS_H

extern void options NOPARAMS;

/* values for options */
#define L_EDITOR	 2
#define L_DISPLAY	 3

#define L_HOME		 4
#define C_MULTI		24
#define B_BOOK		34
#define C_DEFAULT	50

#define L_FTPSTYPE	 5
#define L_MAIL_ADDRESS	 6
#define L_SSEARCH	 7
#define L_CHARSET	 8
#define L_RAWMODE	 9
#define L_LANGUAGE	10
#define L_PREF_CHARSET	11

#define L_BOOL_A	12
#define B_VIKEYS	5
#define C_VIKEYS	15
#define B_EMACSKEYS	22
#define C_EMACSKEYS	36
#define B_SHOW_DOTFILES	44
#define C_SHOW_DOTFILES	62

#define L_SELECT_POPUPS 13
#define L_KEYPAD	14 
#define L_LINEED	15

#ifdef DIRED_SUPPORT
#define L_DIRED		16
#define L_USER_MODE	17
#define L_USER_AGENT	18
#define L_EXEC		19
#else
#define L_USER_MODE	16
#define L_USER_AGENT	17
#define L_EXEC		18
#endif /* DIRED_SUPPORT */

#endif /* LYOPTIONS_H */
