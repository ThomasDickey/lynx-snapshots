/*
 * $LynxId: LYHash.c,v 1.39 2018/03/29 00:38:59 tom Exp $
 *
 * A hash table for the (fake) CSS support in Lynx-rp
 * (c) 1996 Rob Partington
 * rewritten 1997 by Klaus Weide.
 * rewritten 2018 -TD
 */
#include <LYHash.h>
#include <LYUtils.h>
#include <LYLeaks.h>
#include <LYStrings.h>
#include <LYGlobalDefs.h>

#ifdef USE_COLOR_STYLE

#undef HASH_TYPE

#define HASH_SIZE CSHASHSIZE
#define HASH_TYPE     int
#define HASH_OF(h, v) ((HASH_TYPE)((h) * 3 + UCH(v)) % HASH_SIZE)

static int count_bump;
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
	++count_bump;
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
    int hash;

    if (dump_output_immediately) {
	hash = 0;
    } else {
	get_buffer(strlen(string));
	strcpy(buffer, string);
	LYLowerCase(buffer);
	hash = cs_hash(buffer);
    }
    return hash;
}

int color_style_3(const char *p, const char *q, const char *r)
{
    int hash;

    if (dump_output_immediately) {
	hash = 0;
    } else {
	get_buffer(strlen(p) + strlen(q) + strlen(r));
	strcpy(buffer, p);
	strcat(buffer, q);
	strcat(buffer, r);
	LYLowerCase(buffer);
	hash = cs_hash(buffer);
    }
    return hash;
}

void report_hashStyles(void)
{
    int i;
    int count_name = 0;
    int count_used = 0;

    for (i = 0; i < CSHASHSIZE; i++) {
	count_name += (hashStyles[i].name != 0);
	count_used += (hashStyles[i].used != 0);
    }
    CTRACE((tfp, "Style hash:\n"));
    CTRACE((tfp, "%5d names allocated\n", count_name));
    CTRACE((tfp, "%5d buckets used\n", count_used));
    CTRACE((tfp, "%5d hash collisions\n", count_bump));
}

void free_hashStyles(void)
{
    int i;

    for (i = 0; i < CSHASHSIZE; i++) {
	FREE(hashStyles[i].name);
	hashStyles[i].used = FALSE;
    }
    FREE(buffer);
    limit = 0;
    count_bump = 0;
}

#endif /* USE_COLOR_STYLE */
