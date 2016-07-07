/*                                                          Gopher protocol module for libwww
                                      GOPHER ACCESS
                                             
  HISTORY:
  
  8 Jan 92               Adapted from HTTP TBL
                         
 */


#ifndef HTGOPHER_H
#define HTGOPHER_H

#include <HTAccess.h>
#include <HTAnchor.h>

#ifdef GLOBALREF_IS_MACRO
extern GLOBALREF (HTProtocol, HTGopher);
#else
GLOBALREF HTProtocol HTGopher;
#endif /* GLOBALREF_IS_MACRO */

#endif /* HTGOPHER_H */

/*

   end of gopher module */
