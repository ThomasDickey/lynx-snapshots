/*			Lynx Client-side Image MAP Support	       LYMap.c
**			==================================
**
**	Author:	FM	Foteos Macrides (macrides@sci.wfbr.edu)
**
*/

#include "HTUtils.h"
#include "tcp.h"
#include "HTAnchor.h"
#include "HTAccess.h"
#include "HTFormat.h"
#include "HTParse.h"
#include "HTAlert.h"
#include "LYUtils.h"
#include "LYMap.h"
#include "GridText.h"
#include "LYSignal.h"
#include "LYGlobalDefs.h"
#include "LYKeymap.h"
#include "LYCharUtils.h"

#ifdef DIRED_SUPPORT
#include "LYUpload.h"
#include "LYLocal.h"
#endif

#include "LYexit.h"
#include "LYLeaks.h"
 
#define FREE(x) if (x) {free(x); x=NULL;}

extern BOOL reloading;    /* For Flushing Cache on Proxy Server */

typedef struct _MapElement {
   char * address;
   char * title;
} MapElement;

typedef struct _ImageMap {
   char * address;
   char * title;
   HTList * elements;
} ImageMap;

struct _HTStream 
{
  HTStreamClass * isa;
};

PUBLIC HTList * LynxMaps = NULL;
PUBLIC BOOL LYMapsOnly = FALSE;

/* 
 * Utility for freeing the list of MAPs. - FM
 */
PUBLIC void LYLynxMaps_free NOARGS
{
    ImageMap *map;
    MapElement *element;
    HTList *cur = LynxMaps;
    HTList *current;

    if (!cur)
        return;

    while (NULL != (map = (ImageMap *)HTList_nextObject(cur))) {
        FREE(map->address);
	FREE(map->title);
	if (map->elements) {
	    current = map->elements;
	    while (NULL !=
	    	   (element = (MapElement *)HTList_nextObject(current))) {
	        FREE(element->address);
		FREE(element->title);
		FREE(element);
	    }
	    HTList_delete(map->elements);
	    map->elements = NULL;
	}
	FREE(map);
    }
    HTList_delete(LynxMaps);
    LynxMaps = NULL;
    return;
}

/* 
 * Utility for creating an ImageMap list (LynxMaps), if it doesn't
 * exist already, and adding ImageMap entry structures. - FM
 */
PUBLIC BOOL LYAddImageMap ARGS2(char *, address, char *, title)
{
    ImageMap *new = NULL;
    ImageMap *old;
    HTList *cur = NULL;

    if (!(address && *address))
        return FALSE;

    if (!LynxMaps) {
        LynxMaps = HTList_new();
	atexit(LYLynxMaps_free);
    } else {
        cur = LynxMaps;
	while (NULL != (old = (ImageMap *)HTList_nextObject(cur))) {
	    if (!strcmp(old->address, address)) {
		return TRUE;
	    }
	}
    }

    new = (ImageMap *)calloc(1, sizeof(ImageMap));
    if (new == NULL) {
	perror("Out of memory in LYAddImageMap");
	return FALSE;
    }
    StrAllocCopy(new->address, address);
    if (title && *title)
        StrAllocCopy(new->title, title);
    HTList_addObject(LynxMaps, new);
    return TRUE;
}

/* 
 * Utility for adding MapElements to ImageMaps
 * in the LynxMaps list. - FM
 */
PUBLIC BOOL LYAddMapElement ARGS3(char *,map, char *,address, char *, title)
{
    MapElement *new = NULL;
    ImageMap *theMap = NULL;
    HTList *cur = NULL;

    if (!(map && *map && address && *address))
        return FALSE;

    if (!LynxMaps)
        LYAddImageMap(map, NULL);

    cur = LynxMaps;
    while (NULL != (theMap = (ImageMap *)HTList_nextObject(cur))) {
        if (!strcmp(theMap->address, map)) {
	    break;
	}
    }
    if (!theMap)
        return FALSE;
    if (!theMap->elements)
        theMap->elements = HTList_new();
    cur = theMap->elements;
    while (NULL != (new = (MapElement *)HTList_nextObject(cur))) {
        if (!strcmp(new->address, address)) {
	    FREE(new->address);
	    FREE(new->title);
	    HTList_removeObject(theMap->elements, new);
	    FREE(new);
	    break;
	}
    }

    new = (MapElement *)calloc(1, sizeof(MapElement));
    if (new == NULL) {
	perror("Out of memory in LYAddMapElement");
	return FALSE;
    }
    StrAllocCopy(new->address, address);
    if (title && *title)
        StrAllocCopy(new->title, title);
    else
        StrAllocCopy(new->title, address);
    HTList_appendObject(theMap->elements, new);
    return TRUE;
}

/* 	LYLoadIMGmap - F.Macrides (macrides@sci.wfeb.edu)
**	------------
**  	Create a text/html stream with a list of links
**	for HyperText References in AREAs of a MAP.
*/

PRIVATE int LYLoadIMGmap ARGS4 (
	CONST char *, 		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
    HTFormat format_in = WWW_HTML;
    HTStream *target = NULL;
    char buf[1024];
    MapElement *new = NULL;
    ImageMap *theMap = NULL;
    char *MapTitle = NULL;
    char *MapAddress = NULL;
    HTList *cur = NULL;
    char *address = NULL;
    char *cp = NULL;
    DocAddress WWWDoc;
    BOOL old_cache_setting = LYforce_no_cache;
    BOOL old_reloading = reloading;
    HTFormat old_format_out = HTOutputFormat;

    if (!strncasecomp(arg, "LYNXIMGMAP:", 11)) {
        address = (char * )(arg + 11);
    }
    if (!(address && strchr(address, ':'))) {
        HTAlert(MISDIRECTED_MAP_REQUEST);
	return(HT_NOT_LOADED);
    }

    if (!LynxMaps) {
	WWWDoc.address = address;
        WWWDoc.post_data = NULL;
        WWWDoc.post_content_type = NULL;
        WWWDoc.bookmark = NULL;
	WWWDoc.isHEAD = FALSE;
        LYforce_no_cache = TRUE;
	reloading = TRUE;
	HTOutputFormat = WWW_PRESENT;
	LYMapsOnly = TRUE;
 	if (!HTLoadAbsolute(&WWWDoc)) {
	    LYforce_no_cache = old_cache_setting;
	    reloading = old_reloading;
	    HTOutputFormat = old_format_out;
	    LYMapsOnly = FALSE;
	    HTAlert(MAP_NOT_ACCESSIBLE);
	    return(HT_NOT_LOADED);
	}
        LYforce_no_cache = old_cache_setting;
	reloading = old_reloading;
	HTOutputFormat = old_format_out;
	LYMapsOnly = FALSE;
    }

    if (!LynxMaps) {
	HTAlert(MAPS_NOT_AVAILABLE);
	return(HT_NOT_LOADED);
    }

    cur = LynxMaps;
    while (NULL != (theMap = (ImageMap *)HTList_nextObject(cur))) {
        if (!strcmp(theMap->address, address)) {
	    break;
	}
    }
    if (!(theMap && theMap->elements)) {
	WWWDoc.address = address;
        WWWDoc.post_data = NULL;
        WWWDoc.post_content_type = NULL;
        WWWDoc.bookmark = NULL;
	WWWDoc.isHEAD = FALSE;
        LYforce_no_cache = TRUE;
	reloading = TRUE;
	HTOutputFormat = WWW_PRESENT;
	LYMapsOnly = TRUE;
 	if (!HTLoadAbsolute(&WWWDoc)) {
	    LYforce_no_cache = old_cache_setting;
	    reloading = old_reloading;
	    HTOutputFormat = old_format_out;
	    LYMapsOnly = FALSE;
	    HTAlert(MAP_NOT_ACCESSIBLE);
	    return(HT_NOT_LOADED);
	}
	LYforce_no_cache = old_cache_setting;
	reloading = old_reloading;
	HTOutputFormat = old_format_out;
	LYMapsOnly = FALSE;
	cur = LynxMaps;
	while (NULL != (theMap = (ImageMap *)HTList_nextObject(cur))) {
	    if (!strcmp(theMap->address, address)) {
		break;
	    }
	}
	if (!(theMap && theMap->elements)) {
	    HTAlert(MAP_NOT_AVAILABLE);
	    return(HT_NOT_LOADED);
	}
    }

    target = HTStreamStack(format_in, 
			   format_out,
			   sink, anAnchor);

    if (!target || target == NULL) {
	sprintf(buf, CANNOT_CONVERT_I_TO_O,
		HTAtom_name(format_in), HTAtom_name(format_out));
	HTAlert(buf);
	return(HT_NOT_LOADED);
    }

    if (theMap->title && *theMap->title) {
        StrAllocCopy(MapTitle, theMap->title);
    } else if (anAnchor->title && *anAnchor->title) {
        StrAllocCopy(MapTitle, anAnchor->title);
    } else if (LYRequestTitle && *LYRequestTitle &&
    	       strcasecomp(LYRequestTitle, "[USEMAP]")) {
        StrAllocCopy(MapTitle, LYRequestTitle);
    } else if ((cp=strrchr(address, '#')) != NULL) {
        StrAllocCopy(MapTitle, (cp+1));
    }
    if (!(MapTitle && *MapTitle)) {
        StrAllocCopy(MapTitle, "[USEMAP]");
    } else {
        LYEntify(&MapTitle, TRUE);
    }
    
    sprintf(buf,"<head>\n<title>%s</title>\n</head>\n<body>\n", MapTitle);
    (*target->isa->put_block)(target, buf, strlen(buf));

    sprintf(buf,"<h1><em>%s</em></h1>\n", MapTitle);
    (*target->isa->put_block)(target, buf, strlen(buf));

    StrAllocCopy(MapAddress, address);
    LYEntify(&MapAddress, FALSE);
    sprintf(buf,"<h2><em>MAP:</em>&nbsp;%s</h2>\n", MapAddress);
    (*target->isa->put_block)(target, buf, strlen(buf));

    sprintf(buf, "<%s compact>\n", (keypad_mode == LINKS_ARE_NUMBERED) ?
    				   "ul" : "ol");
    (*target->isa->put_block)(target, buf, strlen(buf));
    cur = theMap->elements;
    while (NULL != (new=(MapElement *)HTList_nextObject(cur))) {
        StrAllocCopy(MapAddress, new->address);
	LYEntify(&MapAddress, FALSE);
        sprintf(buf, "<li><a href=\"%s\"\n", MapAddress);
	(*target->isa->put_block)(target, buf, strlen(buf));
        StrAllocCopy(MapTitle, new->title);
	LYEntify(&MapTitle, TRUE);
	sprintf(buf, ">%s</a>\n", MapTitle);
	(*target->isa->put_block)(target, buf, strlen(buf));
    }
    sprintf(buf,"</%s>\n</body>\n", (keypad_mode == LINKS_ARE_NUMBERED) ?
    				      "ul" : "ol");
    (*target->isa->put_block)(target, buf, strlen(buf));

    (*target->isa->_free)(target);
    FREE(MapAddress);
    FREE(MapTitle);
    return(HT_LOADED);
}

#ifdef GLOBALDEF_IS_MACRO
#define _LYIMGMAP_C_GLOBALDEF_1_INIT { "LYNXIMGMAP", LYLoadIMGmap, 0}
GLOBALDEF (HTProtocol,LYLynxIMGmap,_LYIMGMAP_C_GLOBALDEF_1_INIT);
#else
GLOBALDEF PUBLIC HTProtocol LYLynxIMGmap = {"LYNXIMGMAP", LYLoadIMGmap, 0};
#endif /* GLOBALDEF_IS_MACRO */
