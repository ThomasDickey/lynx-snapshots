/*             DOS specific routines

 */

#ifndef HTDOS_H
#define HTDOS_H

/* PUBLIC                                                       HTDOS_wwwName()
**              CONVERTS DOS Name into WWW Name
** ON ENTRY:
**      dosname         DOS file specification (NO NODE)
**
** ON EXIT:
**	returns 	www file specification
**
*/
char * HTDOS_wwwName (char * dosname);

/* PUBLIC                                                       HTDOS_name()
**              CONVERTS WWW name into a DOS name
** ON ENTRY:
**	fn		WWW file name
**
** ON EXIT:
**      returns         dos file specification
**
** Bug:	Returns pointer to static -- non-reentrant
*/
char * HTDOS_name (char * fn);

#endif /* not HTDOS_H */
/*

	End of file HTDOS.h.  */
