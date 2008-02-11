/* $LynxId: LYSession.h,v 1.2 2008/01/10 23:50:45 tom Exp $ */
#ifndef LYSESSION_H
#define LYSESSION_H

#include <HTUtils.h>

#ifdef __cplusplus
extern "C" {
#endif
    extern BOOLEAN RestoreSession(void);
    extern BOOLEAN SaveSession(void);

#ifdef __cplusplus
}
#endif
#endif				/* LYSESSION_H */
