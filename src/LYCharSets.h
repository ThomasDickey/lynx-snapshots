
#ifndef LYCHARSETS_H
#define LYCHARSETS_H

#ifndef UCMAP_H
#include "UCMap.h"
#endif /* !UCMAP_H */

extern BOOLEAN LYHaveCJKCharacterSet;

/*
 *  LYchar_set_name[current_char_set] points to the currently active set.
 */
extern int current_char_set;
extern CONST char * LYchar_set_names[];

/*
 *  Initializer for LYCharSets.c.
 */
extern int LYCharSetsDeclared NOPARAMS;


extern char ** LYCharSets[];
extern char * SevenBitApproximations[];
extern char ** p_entity_values;
extern int LYlowest_eightbit[];
extern int LYNumCharsets;
extern LYUCcharset LYCharSet_UC[];
extern void HTMLSetCharacterHandling PARAMS((int i));
extern void HTMLSetRawModeDefault PARAMS((int i));
extern void HTMLSetUseDefaultRawMode PARAMS((int i, BOOLEAN modeflag));
extern void HTMLSetHaveCJKCharacterSet PARAMS((int i));
extern CONST char * LYEntityNames[];
extern CONST char * HTMLGetEntityName PARAMS((UCode_t code));
extern UCode_t HTMLGetEntityUCValue PARAMS((CONST char *name));
extern void HTMLUseCharacterSet PARAMS((int i));

#endif /* LYCHARSETS_H */
