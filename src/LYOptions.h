#ifndef LYOPTIONS_H
#define LYOPTIONS_H

#include <LYStructs.h>

extern BOOLEAN term_options; /* for LYgetstr() */

extern void edit_bookmarks NOPARAMS;

#ifndef NO_OPTION_FORMS
extern int postoptions PARAMS((document *newdoc));
extern int gen_options PARAMS((char **newfile));
#endif /* !NO_OPTION_FORMS */

#ifndef NO_OPTION_MENU
extern void LYoptions NOPARAMS;
#endif /* !NO_OPTION_MENU */

#endif /* LYOPTIONS_H */
