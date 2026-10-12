/* $LynxId: LYMap.h,v 1.13 2026/10/12 13:10:50 tom Exp $ */
#ifndef LYMAP_H
#define LYMAP_H

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

#include <HTList.h>
#include <HTAnchor.h>

#ifdef __cplusplus
extern "C" {
#endif
    extern BOOL LYMapsOnly;

    extern void ImageMapList_free(HTList *list);
    extern void LYPrintImgMaps(FILE *fp);
    extern BOOL LYAddImageMap(const char *address,
			      const char *title,
			      HTParentAnchor *node_anchor);
    extern BOOL LYAddMapElement(const char *map,
				const char *address,
				const char *title,
				HTParentAnchor *node_anchor,
				int intern_flag);
    extern BOOL LYHaveImageMap(const char *address);

#ifdef __cplusplus
}
#endif
#endif				/* LYMAP_H */
