/*                  /Net/dxcern/userd/timbl/hypertext/WWW/Library/Implementation/HTMLGen.html
                                      HTML GENERATOR
                                             
   This module converts structed stream into stream.  That is, given a stream to write to,
   it will give you a structured stream to
   
 */
#ifndef HTMLGEN_H
#define HTMLGEN_H

#include <HTML.h>
#include <HTStream.h>

/* Subclass:
*/
/* extern const HTStructuredClass HTMLGeneration; */

/* Special Creation:
*/
extern HTStructured * HTMLGenerator (HTStream * output);

extern HTStream * HTPlainToHTML (
        HTPresentation *        pres,
        HTParentAnchor *        anchor,
        HTStream *              sink);

#endif /* HTMLGEN_H */
