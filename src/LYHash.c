/*
 * $LynxId: LYHash.c,v 1.33 2018/03/07 10:20:10 tom Exp $
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

#define HASH_SIZE CSHASHSIZE
#define HASH_TYPE     int
#define HASH_OF(h, v) ((HASH_TYPE)((h) * 3 + UCH(v)) % HASH_SIZE)

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

/*
 * This is the same algorithm as the private anchor_hash() in HTAnchor.c, but
 * with a different value for HASH_SIZE.
 */
static HASH_TYPE cs_hash(const char *string)
{
    HASH_TYPE hash = 0;
    HASH_TYPE best, n;
    bucket *data;
    const char *p;
    int bumped = 0;

    for (p = string; *p; p++)
	hash = HASH_OF(hash, *p);

    /*
     * The computed hash-code is only a starting point.  Check for collision.
     */
    best = hash;
    for (n = 0; n < HASH_SIZE; n++) {
	int nn = (n + hash) % HASH_SIZE;

	data = &hashStyles[nn];
	if (data->name == 0 || !strcmp(string, data->name)) {
	    best = nn;
	    hash = nn;
	    break;
	}
	++bumped;
    }
    data = &hashStyles[best];
    if (data->name != 0) {
	if (strcmp(string, data->name)) {
	    CTRACE_STYLE((tfp, "cs_hash(%s) overwriting %d\n", string, data->name));
	    FREE(data->name);
	    StrAllocCopy(data->name, string);
	}
    } else {
	StrAllocCopy(data->name, string);
    }

    CTRACE_STYLE((tfp, "cs_hash(%s) = %d\n", string, hash));
    return hash;
}

int color_style_1(const char *string)
{
    get_buffer(strlen(string));
    strcpy(buffer, string);
    LYLowerCase(buffer);
    return cs_hash(buffer);
}

int color_style_3(const char *p, const char *q, const char *r)
{
    get_buffer(strlen(p) + strlen(q) + strlen(r));
    strcpy(buffer, p);
    strcat(buffer, q);
    strcat(buffer, r);
    LYLowerCase(buffer);
    return cs_hash(buffer);
}

#endif /* USE_COLOR_STYLE */
