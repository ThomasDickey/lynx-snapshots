
#ifndef LYUPLOAD_H
#define LYUPLOAD_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCTS_H */

extern int LYUpload (char *line);
extern int LYUpload_options (char **newfile, char *directory);

#endif /* LYUPLOAD_H */

