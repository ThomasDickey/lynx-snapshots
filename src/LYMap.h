#ifndef LYMAP_H
#define LYMAP_H

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

#include <HTList.h>
#include <HTAnchor.h>

extern BOOL LYMapsOnly;

extern void ImageMapList_free (HTList * list);
extern BOOL LYAddImageMap (char *address, char *title,
				  HTParentAnchor *node_anchor);
extern BOOL LYAddMapElement (char *map, char *address, char *title,
				    HTParentAnchor *node_anchor,
				    BOOL intern_flag);
extern BOOL LYHaveImageMap (char *address);

#endif /* LYMAP_H */
