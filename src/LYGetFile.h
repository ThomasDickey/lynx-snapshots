
#ifndef LYGETFILE_H
#define LYGETFILE_H

#define NOT_FOUND 0
#define NORMAL 1
#define NULLFILE 3

extern BOOLEAN getfile PARAMS((document *doc));
extern int follow_link_number PARAMS((int c, int cur));
extern void add_trusted PARAMS((char *str, int type));
extern BOOLEAN exec_ok PARAMS((CONST char *source, CONST char *linkpath, int type));

/* values for follow_link_number.c */
#define DO_FORMS_STUFF 1
#define DO_LINK_STUFF  2
#define PRINT_ERROR    3

/* values for add_trusted() and exec_ok() */
#define EXEC_PATH 0
#define ALWAYS_EXEC_PATH  1
#define CGI_PATH  2

#endif /* LYGETFILE_H */
