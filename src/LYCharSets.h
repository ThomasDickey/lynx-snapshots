
#ifndef LYCHARSETS_H
#define LYCHARSETS_H

/* LYchar_set_name[current_char_set] points to the currently active set */
extern int current_char_set;
extern char *LYchar_set_names[];


extern char ** LYCharSets[];
extern char ** p_entity_values;
#ifdef USE_SLANG
extern int LYlowest_eightbit[];
#endif /* USE_SLANG */
extern void HTMLSetCharacterHandling PARAMS((int i));
extern void HTMLSetRawModeDefault PARAMS((int i));
extern void HTMLSetUseDefaultRawMode PARAMS((int i, BOOLEAN modeflag));
extern void HTMLSetHaveCJKCharacterSet PARAMS((int i));
extern CONST char * LYEntityNames[];
extern CONST char * HTMLGetEntityName PARAMS((int i));
extern char HTMLGetLatinOneValue PARAMS((int i));
extern void HTMLUseCharacterSet PARAMS((int i));

#endif /* LYCHARSETS_H */
