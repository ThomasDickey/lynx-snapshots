/*                                                                     File Writer for libwww
                                      C FILE WRITER

   It is useful to have both FWriter and Writer for environments in which fdopen() doesn't
   exist for example.

 */
#ifndef HTFWRITE_H
#define HTFWRITE_H

#include <HTStream.h>
#include <HTFormat.h>

extern HTStream * HTFWriter_new (FILE * fp);

extern HTStream * HTSaveAndExecute (
        HTPresentation *        pres,
        HTParentAnchor *        anchor, /* Not used */
        HTStream *              sink);

extern HTStream * HTSaveLocally (
        HTPresentation *        pres,
        HTParentAnchor *        anchor, /* Not used */
        HTStream *              sink);

#endif /* HTFWRITE_H */
