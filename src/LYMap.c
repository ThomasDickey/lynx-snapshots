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

typedef struct _LYMapElement {
   char * address;
   char * title;
   BOOL   intern_flag;
} LYMapElement;

typedef struct _LYImageMap {
   char * address;
   char * title;
   HTList * elements;
} LYImageMap;

struct _HTStream 
{
  HTStreamClass * isa;
};

PRIVATE HTList * LynxMaps = NULL;

PUBLIC BOOL LYMapsOnly = FALSE;

/* 
 *  Utility for freeing the list of MAPs. - FM
 */
PRIVATE void LYLynxMaps_free NOARGS
{
    LYImageMap *map;
    LYMapElement *element;
    HTList *cur = LynxMaps;
    HTList *current;

    if (!cur)
        return;

    while (NULL != (map = (LYImageMap *)HTList_nextObject(cur))) {
        FREE(map->address);
	FREE(map->title);
	if (map->elements) {
	    current = map->elements;
	    while (NULL !=
	    	   (element = (LYMapElement *)HTList_nextObject(current))) {
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
 *  Utility for creating an LYImageMap list (LynxMaps), if it doesn't
 *  exist already, adding LYImageMap entry structures if needed, and
 *  removing any LYMapElements in a pre-existing LYImageMap entry so that
 *  it will have only those from AREA tags for the current analysis of
 *  MAP element content. - FM
 */
PUBLIC BOOL LYAddImageMap ARGS2(
	char *,		address,
	char *,		title)
{
    LYImageMap *new = NULL;
    LYImageMap *old = NULL;
    HTList *cur = NULL;
    HTList *curele = NULL;
    LYMapElement *ele = NULL;

    if (!(address && *address))
        return FALSE;

    if (!LynxMaps) {
        LynxMaps = HTList_new();
	atexit(LYLynxMaps_free);
    } else {
        cur = LynxMaps;
	while (NULL != (old = (LYImageMap *)HTList_nextObject(cur))) {
	    if (!strcmp(old->address, address)) {
		FREE(old->address);
		FREE(old->title);
		if (old->elements) {
		    curele = old->elements;
		    while (NULL !=
		    	   (ele = (LYMapElement *)HTList_nextObject(curele))) {
		        FREE(ele->address);
			FREE(ele->title);
			FREE(ele);
		    }
		    HTList_delete(old->elements);
		    old->elements = NULL;
		}
		break;
	    }
	}
    }

    new = (old != NULL) ?
		    old : (LYImageMap *)calloc(1, sizeof(LYImageMap));
    if (new == NULL) {
	perror("Out of memory in LYAddImageMap");
	return FALSE;
    }
    StrAllocCopy(new->address, address);
    if (title && *title)
        StrAllocCopy(new->title, title);
    if (new != old)
        HTList_addObject(LynxMaps, new);
    return TRUE;
}

/* 
 * Utility for adding LYMapElements to LYImageMaps
 * in the LynxMaps list. - FM
 */
PUBLIC BOOL LYAddMapElement ARGS4(
	char *,		map,
	char *,		address,
	char *,		title,
				  BOOL,	intern_flag)
{
    LYMapElement *new = NULL;
    LYImageMap *theMap = NULL;
    HTList *cur = NULL;

    if (!(map && *map && address && *address))
        return FALSE;

    if (!LynxMaps)
        LYAddImageMap(map, NULL);

    cur = LynxMaps;
    while (NULL != (theMap = (LYImageMap *)HTList_nextObject(cur))) {
        if (!strcmp(theMap->address, map)) {
	    break;
	}
    }
    if (!theMap)
        return FALSE;
    if (!theMap->elements)
        theMap->elements = HTList_new();
    cur = theMap->elements;
    while (NULL != (new = (LYMapElement *)HTList_nextObject(cur))) {
        if (!strcmp(new->address, address)) {
	    FREE(new->address);
	    FREE(new->title);
	    HTList_removeObject(theMap->elements, new);
	    FREE(new);
	    break;
	}
    }

    new = (LYMapElement *)calloc(1, sizeof(LYMapElement));
    if (new == NULL) {
	perror("Out of memory in LYAddMapElement");
	return FALSE;
    }
    StrAllocCopy(new->address, address);
    if (title && *title)
        StrAllocCopy(new->title, title);
    else
        StrAllocCopy(new->title, address);
    new->intern_flag = intern_flag;
    HTList_appendObject(theMap->elements, new);
    return TRUE;
}

/*
 *  Utility for checking whether an LYImageMap entry
 *  with a given address already exists in the LynxMaps
 *  structure. - FM
 */
#if UNUSED
PUBLIC BOOL LYHaveImageMap ARGS1(
	char *,		address)
{
    LYImageMap *Map;
    HTList *cur = LynxMaps;

    if (!(cur && address && *address != '\0'))
        return FALSE;

    while (NULL != (Map = (LYImageMap *)HTList_nextObject(cur))) {
	if (!strcmp(Map->address, address)) {
	    return TRUE;
	}
    }

    return FALSE;
}
#endif

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
    LYMapElement *new = NULL;
    LYImageMap *theMap = NULL;
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
	WWWDoc.safe = FALSE;
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
    while (NULL != (theMap = (LYImageMap *)HTList_nextObject(cur))) {
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
	WWWDoc.safe = FALSE;
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
	while (NULL != (theMap = (LYImageMap *)HTList_nextObject(cur))) {
	    if (!strcmp(theMap->address, address)) {
		break;
	    }
	}
	if (!(theMap && theMap->elements)) {
	    HTAlert(MAP_NOT_AVAILABLE);
	    return(HT_NOT_LOADED);
	}
    }

    anAnchor->no_cache = TRUE;

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
    while (NULL != (new=(LYMapElement *)HTList_nextObject(cur))) {
        StrAllocCopy(MapAddress, new->address);
	LYEntify(&MapAddress, FALSE);
	(*target->isa->put_block)(target, "<li><a href=\"", 13);
	(*target->isa->put_block)(target, MapAddress, strlen(MapAddress));
	if (new->intern_flag)
	    (*target->isa->put_block)(target, "\" TYPE=\"internal link\"\n>",24);
	else
	    (*target->isa->put_block)(target, "\"\n>", 3);
        StrAllocCopy(MapTitle, new->title);
	LYEntify(&MapTitle, TRUE);
	(*target->isa->put_block)(target, MapTitle, strlen(MapTitle));
	(*target->isa->put_block)(target, "</a>\n", 5);
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
