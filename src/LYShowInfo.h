#ifndef LYSHOWINFO_H
#define LYSHOWINFO_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCTS_H */

extern BOOL LYVersionIsRelease(void);
extern const char *LYVersionStatus(void);
extern const char *LYVersionDate(void);
extern int LYShowInfo(DocInfo *doc,
		      int size_of_file,
		      DocInfo *newdoc,
		      char *owner_address);

#endif /* LYSHOWINFO_H */
