/* $LynxId: LYCookie.h,v 1.21 2026/10/12 13:11:44 tom Exp $ */
#ifndef LYCOOKIES_H
#define LYCOOKIES_H

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

#include <HTList.h>

#ifdef __cplusplus
extern "C" {
#endif
    typedef enum {
	ACCEPT_ALWAYS = 0
	,REJECT_ALWAYS
	,QUERY_USER
    } behaviour_t;

    typedef enum {
	INVCHECK_QUERY = 0
	,INVCHECK_STRICT
	,INVCHECK_LOOSE
    } invcheck_behaviour_t;

    typedef enum {
	FLAG_ACCEPT_ALWAYS = 0
	,FLAG_REJECT_ALWAYS
	,FLAG_QUERY_USER
	,FLAG_FROM_FILE
	,FLAG_INVCHECK_QUERY
	,FLAG_INVCHECK_STRICT
	,FLAG_INVCHECK_LOOSE
    } cookie_domain_flags;

    struct _domain_entry {
	char *domain;		/* Domain for which these cookies are valid */
	char *ddomain;		/* Domain without leading "." */
	behaviour_t bv;
	invcheck_behaviour_t invcheck_bv;
	HTList *cookie_list;
    };
    typedef struct _domain_entry domain_entry;

    extern void LYSetCookie(const char *SetCookie,
			    const char *SetCookie2,
			    const char *address);
    extern char *LYAddCookieHeader(char *hostname,
				   const char *partialpath,
				   int port,
				   int secure);
    extern void LYStoreCookies(const char *cookie_file);
    extern void LYLoadCookies(const char *cookie_file);
    extern void LYConfigCookies(void);

#ifdef __cplusplus
}
#endif
#endif				/* LYCOOKIES_H */
