#ifndef LYDOWNLOAD_H
#define LYDOWNLOAD_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCTS_H */

extern void LYDownload (char *line);
extern int LYdownload_options (char **newfile, char *data_file);

#ifdef VMS
extern BOOLEAN LYDidRename;
#endif

#endif /* LYDOWNLOAD_H */
