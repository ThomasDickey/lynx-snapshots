
#ifndef LYOPTIONS_H
#define LYOPTIONS_H

extern void options NOPARAMS;

/* values for options */
#define L_EDITOR	 2
#define L_DISPLAY	 3
#define L_HOME		 4
#define L_FTPSTYPE	 5
#define L_MAIL_ADDRESS	 6
#define L_SSEARCH	 7
#define L_CHARSET	 8
#define L_RAWMODE	 9
#define L_LANGUAGE	10
#define L_PREF_CHARSET	11
#define L_VIKEYS	12
#define L_EMACSKEYS	13
#define L_KEYPAD	14 
#define L_LINEED	15

#ifdef DIRED_SUPPORT
#define L_DIRED		16
#define L_SHOW_DOTFILES	17
#define L_USER_MODE	18
#define L_USER_AGENT	19
#define L_EXEC		20
#else
#define L_SHOW_DOTFILES	16
#define L_USER_MODE	17
#define L_USER_AGENT	18
#define L_EXEC		19
#endif /* DIRED_SUPPORT */

#endif /* LYOPTIONS_H */
