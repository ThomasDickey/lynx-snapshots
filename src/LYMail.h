/*
 * $LynxId: LYMail.h,v 1.15 2011/05/27 09:54:07 tom Exp $
 */
#ifndef LYMAIL_H
#define LYMAIL_H

#ifndef LYSTRUCTS_H
#include <LYStructs.h>
#endif /* LYSTRUCTS_H */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SH_EX
#undef USE_BLAT_MAILER
#define USE_BLAT_MAILER 1
#endif

#ifndef USE_ALTBLAT_MAILER
#define USE_ALTBLAT_MAILER 0
#endif

#ifndef USE_BLAT_MAILER
#define USE_BLAT_MAILER 0
#endif

#ifdef USE_BLAT_MAILER

#undef SYSTEM_MAIL
#undef SYSTEM_MAIL_FLAGS

#ifdef USE_ALT_BLAT_MAILER
#define SYSTEM_MAIL		ALTBLAT_MAIL
#define SYSTEM_MAIL_FLAGS	""
#else
#define SYSTEM_MAIL		BLAT_MAIL
#define SYSTEM_MAIL_FLAGS	""
#define USE_ALT_BLAT_MAILER     0
#endif

#endif				/* USE_BLAT_MAILER */

#ifdef VMS
#define USE_VMS_MAILER 1
#else
#define USE_VMS_MAILER 0
#endif

#ifndef SYSTEM_MAIL
#define SYSTEM_MAIL "sendmail"
#endif

#ifndef SYSTEM_MAIL_FLAGS
#define SYSTEM_MAIL_FLAGS ""
#endif

/*
 * Ifdef's in case we have a working popen/pclose, useful for piping to the
 * mail program.
 */
#ifndef CAN_PIPE_TO_MAILER
#if !defined(HAVE_POPEN) || USE_VMS_MAILER || defined(DOSPATH) || defined(__CYGWIN__)
#define CAN_PIPE_TO_MAILER 0
#else
#define CAN_PIPE_TO_MAILER 1
#endif
#endif

    extern BOOLEAN term_letter;

    extern BOOLEAN LYSystemMail(void);
    extern BOOLEAN LYMailPMDF(void);
    extern FILE *LYPipeToMailer(void);
    extern int LYSendMailFile(char *the_address,
			      char *the_filename,
			      char *the_subject,
			      char *the_ccaddr,
			      char *message);
    extern void mailform(const char *mailto_address,
			 const char *mailto_subject,
			 const char *mailto_content,
			 const char *mailto_type);
    extern void mailmsg(int cur,
			char *owner_address,
			char *filename,
			char *linkname);
    extern void reply_by_mail(char *mail_address,
			      char *filename,
			      const char *title,
			      const char *refid);

#ifdef __cplusplus
}
#endif
#endif				/* LYMAIL_H */
