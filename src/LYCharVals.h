#ifndef LYCHARVALS_H
#define LYCHARVALS_H 1

#include <HTUtils.h>

#ifndef   CH_ESC
#ifdef    EBCDIC
#define CH_DEL     '\x07'
#define CH_ESC     '\x27'
#define CH_ESC_PAR '\x27'
#define CH_HICTL   '\x3f'
#define CH_NBSP    '\x41'
#define CH_SHY     '\xca'
#else  /* EBCDIC */
#define CH_ESC     '\033'
#define CH_DEL     '\177'
#define CH_ESC_PAR '\233'
#define CH_HICTL   '\237'
#define CH_NBSP    '\240'
#define CH_SHY     '\255'
#endif /* EBCDIC */
#endif /* CH_ESC */

#endif /* LYCHARVALS_H */
