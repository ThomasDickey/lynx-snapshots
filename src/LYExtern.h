#ifndef EXTERNALS_H
#define EXTERNALS_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCTS_H */

/*returns TRUE if something matching was executed*/
BOOL run_external (char * c, BOOL only_overriders);

#ifdef WIN_EX
extern char * quote_pathname (char * pathname);
#endif

#endif /* EXTERNALS_H */
