
#ifndef LYCHARSETS_H
#define LYCHARSETS_H

/* LYchar_set_name[current_char_set] points to the currently active set */
extern int current_char_set;
extern CONST char * LYchar_set_names[];


extern char ** LYCharSets[];
extern char * SevenBitApproximations[];    /* made public. -kw */
extern char ** p_entity_values;
#if defined(USE_SLANG) || defined(EXP_CHARTRANS)
extern int LYlowest_eightbit[];
#endif /* USE_SLANG || EXP_CHARTRANS */

#ifdef EXP_CHARTRANS
extern int LYNumCharsets;
extern LYUCcharset LYCharSet_UC[];
#endif   /* EXP_CHARTRANS */

/* Initializer for LYCharSets.c */
extern int LYCharSetsDeclared NOPARAMS;

extern void HTMLSetCharacterHandling PARAMS((int i));
extern void HTMLSetRawModeDefault PARAMS((int i));
extern void HTMLSetUseDefaultRawMode PARAMS((int i, BOOLEAN modeflag));
extern void HTMLSetHaveCJKCharacterSet PARAMS((int i));
extern CONST char * LYEntityNames[];
extern CONST char * HTMLGetEntityName PARAMS((int i));
extern char HTMLGetLatinOneValue PARAMS((int i));
extern void HTMLUseCharacterSet PARAMS((int i));

#endif /* LYCHARSETS_H */
