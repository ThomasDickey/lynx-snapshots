#ifndef LYREADCFG_H
#define LYREADCFG_H

#ifndef LYSTRUCTS_H
#include "LYStructs.h"
#endif /* LYSTRUCTS_H */

#if defined(USE_COLOR_STYLE) || defined(USE_COLOR_TABLE)

#define DEFAULT_COLOR -1
#define NO_COLOR      -2
#define ERR_COLOR     -3

extern int default_fg;
extern int default_bg;

extern int check_color PARAMS((char * color, int the_default));
#endif

extern void read_cfg PARAMS((char *cfg_filename));
extern BOOLEAN have_read_cfg;

#endif /* LYREADCFG_H */
