#ifndef LYCOOKIES_H
#define LYCOOKIES_H

#include <HTUtils.h>
#include <HTList.h>

extern void LYSetCookie PARAMS((
	CONST char *	SetCookie,
	CONST char *	SetCookie2,
	CONST char *	address));
extern char *LYCookie PARAMS((
	CONST char *	hostname,
	CONST char *	partialpath,
	int		port,
	BOOL		secure));
extern void LYStoreCookies PARAMS((
	CONST char *	cookie_file));
extern void LYLoadCookies PARAMS((
	CONST char * 	cookie_file));
extern void cookie_add_acceptlist PARAMS((
	char *		acceptdomains));
extern void cookie_add_rejectlist PARAMS((
	char *		rejectdomains));

typedef enum {ACCEPT_ALWAYS, REJECT_ALWAYS, QUERY_USER, FROM_FILE} behaviour;

struct _domain_entry {
    char *	domain;  /* Domain for which these cookies are valid */
    behaviour	bv;
    HTList *	cookie_list;
};
typedef struct _domain_entry domain_entry;

#endif /* LYCOOKIES_H */
