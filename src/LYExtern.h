#ifndef EXTERNALS_H
#define EXTERNALS_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCTS_H */

void run_external PARAMS((char * c));
char *string_short PARAMS((char * str, int cut_pos));

#endif /* EXTERNALS_H */
