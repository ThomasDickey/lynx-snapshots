#ifndef LYOPTIONS_H
#define LYOPTIONS_H

#include <LYStructs.h>

extern BOOLEAN term_options; /* for LYgetstr() */

extern void edit_bookmarks NOPARAMS;

extern int postoptions PARAMS((document *newdoc));
extern int gen_options PARAMS((char **newfile));

#ifndef EXP_FORMS_OPTIONS
extern void LYoptions NOPARAMS;
#endif /* !EXP_FORMS_OPTIONS */

#endif /* LYOPTIONS_H */
