#ifndef HTSAVETOFILE_H
#define HTSAVETOFILE_H

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

#include <HTStream.h>
#include <HTFormat.h>

extern HTStream * HTSaveToFile (
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink);

extern HTStream * HTDumpToStdout (
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink);

extern HTStream * HTCompressed (
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink);

#endif /* HTSAVETOFILE_H */
