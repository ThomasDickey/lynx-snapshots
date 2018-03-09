/*
 * $LynxId: HTAtom.c,v 1.22 2018/03/06 09:46:58 tom Exp $
 *
 *			Atoms: Names to numbers			HTAtom.c
 *			=======================
 *
 *	Atoms are names which are given representative pointer values
 *	so that they can be stored more efficiently, and comparisons
 *	for equality done more efficiently.
 *
 *	Atoms are kept in a hash table consisting of an array of linked lists.
 *
 * Authors:
 *	TBL	Tim Berners-Lee, WorldWideWeb project, CERN
 *	(c) Copyright CERN 1991 - See Copyright.html
 *
 */

#include <HTUtils.h>
#include <HTAtom.h>
#include <HTList.h>
#include <LYexit.h>
#include <LYLeaks.h>

#define HASH_SIZE 101		/* Arbitrary prime.  Memory/speed tradeoff */

static HTAtom *hash_table[HASH_SIZE];
static BOOL initialised = NO;

/*
 *	To free off all atoms.
 */
#ifdef LY_FIND_LEAKS
static void free_atoms(void);
#endif

#define HASH_FUNCTION(cp_hash) ((strlen(cp_hash) * UCH(*cp_hash)) % HASH_SIZE)

HTAtom *HTAtom_for(const char *string)
{
    size_t hash;
    HTAtom *a;

    if (!initialised) {
	memset(hash_table, 0, sizeof(hash_table));
	initialised = YES;
#ifdef LY_FIND_LEAKS
	atexit(free_atoms);
#endif
    }

    hash = HASH_FUNCTION(string);

    for (a = hash_table[hash]; a; a = a->next) {
	if (0 == strcasecomp(a->name, string)) {
	    return a;
	}
    }

    a = (HTAtom *) malloc(sizeof(*a));
    if (a == NULL)
	outofmem(__FILE__, "HTAtom_for");

    a->name = (char *) malloc(strlen(string) + 1);
    if (a->name == NULL)
	outofmem(__FILE__, "HTAtom_for");

    strcpy(a->name, string);
    a->next = hash_table[hash];
    hash_table[hash] = a;
    return a;
}

#ifdef LY_FIND_LEAKS
/*
 *	Purpose:	Free off all atoms.
 *	Arguments:	void
 *	Return Value:	void
 *	Remarks/Portability/Dependencies/Restrictions:
 *		To be used at program exit.
 *	Revision History:
 *		05-29-94	created Lynx 2-3-1 Garrett Arch Blythe
 */
static void free_atoms(void)
{
    auto int i_counter;
    HTAtom *HTAp_freeme;

    /*
     * Loop through all lists of atoms.
     */
    for (i_counter = 0; i_counter < HASH_SIZE; i_counter++) {
	/*
	 * Loop through the list.
	 */
	while (hash_table[i_counter] != NULL) {
	    /*
	     * Free off atoms and any members.
	     */
	    HTAp_freeme = hash_table[i_counter];
	    hash_table[i_counter] = HTAp_freeme->next;
	    FREE(HTAp_freeme->name);
	    FREE(HTAp_freeme);
	}
    }
}
#endif /* LY_FIND_LEAKS */
