/* A hash table for the (fake) CSS support in Lynx-rp
** (c) 1996 Rob Partington
*/

#include "LYStructs.h"
#include "LYCurses.h"
#include "AttrList.h"
#include "SGML.h"
#include "HTMLDTD.h"

#include "LYHash.h"

PUBLIC int hash_table[HASHSIZE]; /* 32K should be big enough */

#if UNUSED
PUBLIC int hash_code_rp ARGS1(char*,string)
{
	char* hash_ptr = string;
	int hash_tmp = 0xC00A | ((*hash_ptr) << 4);

	while (*hash_ptr++)
	{
		hash_tmp ^= (((*hash_ptr)<<4) ^ ((*hash_ptr)<<12));
		hash_tmp >>= 1;
	}
	return (hash_tmp % HASHSIZE);
}
#endif

PUBLIC int hash_code ARGS1 (char*, string)
{
	return HASH_FUNCTION(string);
}
