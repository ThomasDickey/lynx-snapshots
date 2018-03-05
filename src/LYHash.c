/*
 * $LynxId: LYHash.c,v 1.26 2018/03/04 20:01:27 tom Exp $
 *
 * A hash table for the (fake) CSS support in Lynx-rp
 * (c) 1996 Rob Partington
 * rewritten 1997 by Klaus Weide.
 * rewritten 2018 -TD
 */
#include <LYHash.h>
#include <LYUtils.h>
#include <LYStrings.h>

#ifdef USE_COLOR_STYLE

/*
 * This is the same function as the private HASH_FUNCTION() in HTAnchor.c, but
 * with a different value for HASH_SIZE.
 */

#define HASH_SIZE CSHASHSIZE
#define HASH_OF(h, v) ((int)((h) * 3 + UCH(v)) % HASH_SIZE)

static size_t limit;
static char *buffer;

static char *get_buffer(size_t need)
{
    if (++need > limit) {
	char *test = realloc(buffer, (limit = (1 + need) * 2));

	if (test == 0)
	    outofmem(__FILE__, "LYHash");
	buffer = test;
    }
    return buffer;
}

static int hash_code(const char *string)
{
    int hash = 0;

    if (string != 0) {
	const char *p;

	for (p = string; *p; p++)
	    hash = HASH_OF(hash, *p);

	CTRACE_STYLE((tfp, "hash_code(%s) = %d\n", string, hash));
    } else {
	FREE(buffer);
	limit = 0;
    }
    return hash;
}

int hash_code_1(const char *string)
{
    get_buffer(strlen(string));
    strcpy(buffer, string);
    LYLowerCase(buffer);
    return hash_code(buffer);
}

int hash_code_3(const char *p, const char *q, const char *r)
{
    get_buffer(strlen(p) + strlen(q) + strlen(r));
    strcpy(buffer, p);
    strcat(buffer, q);
    strcat(buffer, r);
    LYLowerCase(buffer);
    return hash_code(buffer);
}

#endif /* USE_COLOR_STYLE */
