/* $LynxId: LYExtern.h,v 1.15 2026/10/12 13:17:47 tom Exp $ */
#ifndef EXTERNALS_H
#define EXTERNALS_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCTS_H */

#ifdef __cplusplus
extern "C" {
#endif
    extern BOOL run_external(const char *c, int only_overriders);
#ifdef __cplusplus
}
#endif
#endif				/* EXTERNALS_H */
