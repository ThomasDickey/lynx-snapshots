#ifndef LYEDIT_H
#define LYEDIT_H

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

extern BOOLEAN editor_can_position(void);
extern int edit_current_file(char *newfile, int cur, int lineno);
extern void edit_temporary_file(char *filename, char *position, char *message);

#endif /* LYEDIT_H */
