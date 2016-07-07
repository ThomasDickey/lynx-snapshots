/*			Generic Communication Code		HTTCP.c
**			==========================
**
**	This code is in common between client and server sides.
**
**	16 Jan 92  TBL	Fix strtol() undefined on CMU Mach.
**	25 Jun 92  JFG	Added DECNET option through TCP socket emulation.
**	13 Sep 93  MD	Added correct return of vmserrorno for HTInetStatus.
**			Added decoding of vms error message for MULTINET.
**	7-DEC-1993 Bjorn S. Nilsson, ALEPH, CERN, VMS UCX ioctl() changes
**			(done of Mosaic)
**	19 Feb 94  Danny Mayer	Added Bjorn Fixes to Lynx version
**	 7 Mar 94  Danny Mayer	Added Fix UCX version for full domain name
**	20 May 94  Andy Harper	Added support for CMU TCP/IP transport
**	17 Nov 94  Andy Harper	Added support for SOCKETSHR transport
**	16 Jul 95  S. Bjorndahl added kluge to deal with LIBCMU bug
*/

#include <HTUtils.h>
#include <HTParse.h>
#include <HTAlert.h>
#include <HTTCP.h>
#include <LYGlobalDefs.h>	/* added for no_suspend */
#include <LYUtils.h>

#ifdef NSL_FORK
#include <signal.h>
#include <sys/wait.h>
#endif /* NSL_FORK */

#ifdef HAVE_RESOLV_H
#include <resolv.h>
#endif

#define OK_HOST(p) ((p) != 0 && ((p)->h_length) != 0)

#ifdef SVR4_BSDSELECT
PUBLIC int BSDselect PARAMS((
	int		 nfds,
	fd_set *	 readfds,
	fd_set *	 writefds,
	fd_set *	 exceptfds,
	struct timeval * timeout));
#ifdef select
#undef select
#endif /* select */
#define select BSDselect
#ifdef SOCKS
#ifdef Rselect
#undef Rselect
#endif /* Rselect */
#define Rselect BSDselect
#endif /* SOCKS */
#endif /* SVR4_BSDSELECT */

#include <LYLeaks.h>

#ifndef FD_SETSIZE
#if defined(UCX) || defined(SOCKETSHR_TCP) || defined(CMU_TCP)
#define FD_SETSIZE 32
#else
#define FD_SETSIZE 256
#endif /* Limit # sockets to 32 for UCX, BSN - also SOCKETSHR and CMU, AH */
#endif /* FD_SETSIZE */

/*
**  Module-Wide variables
*/
PRIVATE char *hostname = NULL;		/* The name of this host */

/*
**  PUBLIC VARIABLES
*/
#ifdef SOCKS
extern BOOLEAN socks_flag;
PUBLIC unsigned long socks_bind_remoteAddr; /* for long Rbind */
#endif /* SOCKS */

/* PUBLIC SockA HTHostAddress; */	/* The internet address of the host */
					/* Valid after call to HTHostName() */

/*	Encode INET status (as in sys/errno.h)			  inet_status()
**	------------------
**
**  On entry,
**	where		gives a description of what caused the error
**	global errno	gives the error number in the Unix way.
**
**  On return,
**	returns 	a negative status in the Unix way.
*/
#ifndef PCNFS

#ifdef VMS
#include <perror.h>
#ifndef errno
extern int errno;
#endif /* !errno */
#endif /* VMS */

#ifndef VM
#ifndef VMS
#ifndef THINK_C

#ifdef DECL_SYS_ERRLIST
extern char *sys_errlist[];		/* see man perror on cernvax */
extern int sys_nerr;
#endif /* DECL_SYS_ERRLIST */

#endif /* !THINK_C */
#endif /* !VMS */
#endif /* !VM */

#endif	/* !PCNFS */

#ifdef _WINDOWS_NSL
	 char host[512];
	 struct hostent  *phost;	/* Pointer to host - See netdb.h */
	 int donelookup;

unsigned long _fork_func (void *arglist)
{
	 phost = gethostbyname(host);
	 donelookup = TRUE;
	 return (unsigned long)(phost);
}
#endif /* _WINDOWS_NSL */

#if defined(VMS) && defined(UCX)
/*
**  A routine to mimic the ioctl function for UCX.
**  Bjorn S. Nilsson, 25-Nov-1993. Based on an example in the UCX manual.
*/
#include <iodef.h>
#define IOC_OUT (int)0x40000000
extern int vaxc$get_sdc(), sys$qiow();

PUBLIC int HTioctl ARGS3(
	int,		d,
	int,		request,
	int *,		argp)
{
    int sdc, status;
    unsigned short fun, iosb[4];
    char *p5, *p6;
    struct comm {
	int command;
	char *addr;
    } ioctl_comm;
    struct it2 {
	unsigned short len;
	unsigned short opt;
	struct comm *addr;
    } ioctl_desc;

    if ((sdc = vaxc$get_sdc (d)) == 0) {
	errno = EBADF;
	return -1;
    }
    ioctl_desc.opt  = UCX$C_IOCTL;
    ioctl_desc.len  = sizeof(struct comm);
    ioctl_desc.addr = &ioctl_comm;
    if (request & IOC_OUT) {
	fun = IO$_SENSEMODE;
	p5 = 0;
	p6 = (char *)&ioctl_desc;
    } else {
	fun = IO$_SETMODE;
	p5 = (char *)&ioctl_desc;
	p6 = 0;
    }
    ioctl_comm.command = request;
    ioctl_comm.addr = (char *)argp;
    status = sys$qiow (0, sdc, fun, iosb, 0, 0, 0, 0, 0, 0, p5, p6);
    if (!(status & 01)) {
	errno = status;
	return -1;
    }
    if (!(iosb[0] & 01)) {
	errno = iosb[0];
	return -1;
    }
    return 0;
}
#endif /* VMS && UCX */

/*	Report Internet Error
**	---------------------
*/
PUBLIC int HTInetStatus ARGS1(
	char *, 	where)
{
#ifdef VMS
#ifdef MULTINET
    SOCKET_ERRNO = vmserrno;
#endif /* MULTINET */
#endif /* VMS */

    CTRACE(tfp,
	"TCP: Error %d in `SOCKET_ERRNO' after call to %s() failed.\n\t%s\n",
	   SOCKET_ERRNO,  where,
	   /* third arg is transport/platform specific */
#ifdef VM
	   "(Error number not translated)");	/* What Is the VM equiv? */
#define ER_NO_TRANS_DONE
#endif /* VM */

#ifdef VMS
#ifdef MULTINET
	   vms_errno_string());
#else
	   ((SOCKET_ERRNO > 0 && SOCKET_ERRNO <= 65) ?
	    strerror(SOCKET_ERRNO) : "(Error number not translated)"));
#endif /* MULTINET */
#define ER_NO_TRANS_DONE
#endif /* VMS */

#ifdef HAVE_STRERROR
	   strerror(SOCKET_ERRNO));
#define ER_NO_TRANS_DONE
#endif /* HAVE_STRERROR */

#ifndef ER_NO_TRANS_DONE
	   (SOCKET_ERRNO < sys_nerr ?
	    sys_errlist[SOCKET_ERRNO] : "Unknown error" ));
#endif /* !ER_NO_TRANS_DONE */

#ifdef VMS
#ifndef MULTINET
    CTRACE(tfp,
	   "         Unix error number (SOCKET_ERRNO) = %ld dec\n",
	   SOCKET_ERRNO);
    CTRACE(tfp,
	   "         VMS error (vaxc$errno)    = %lx hex\n",
	   vaxc$errno);
#endif /* MULTINET */
#endif /* VMS */

#ifdef VMS
    /*
    **	uerrno and errno happen to be zero if vmserrno <> 0
    */
#ifdef MULTINET
    return -vmserrno;
#else
    return -vaxc$errno;
#endif /* MULTINET */
#else
    return -SOCKET_ERRNO;
#endif /* VMS */
}

/*	Parse a cardinal value				       parse_cardinal()
**	----------------------
**
** On entry,
**	*pp	    points to first character to be interpreted, terminated by
**		    non 0:9 character.
**	*pstatus    points to status already valid
**	maxvalue    gives the largest allowable value.
**
** On exit,
**	*pp	    points to first unread character
**	*pstatus    points to status updated iff bad
*/
PUBLIC unsigned int HTCardinal ARGS3(
	int *,		pstatus,
	char **,	pp,
	unsigned int,	max_value)
{
    unsigned int n;
    if ((**pp<'0') || (**pp>'9')) {	    /* Null string is error */
	*pstatus = -3;	/* No number where one expected */
	return 0;
    }

    n = 0;
    while ((**pp >= '0') && (**pp <= '9'))
	n = n*10 + *((*pp)++) - '0';

    if (n > max_value) {
	*pstatus = -4;	/* Cardinal outside range */
	return 0;
    }

    return n;
}

#ifndef DECNET	/* Function only used below for a trace message */
/*	Produce a string for an Internet address
**	----------------------------------------
**
**  On exit,
**	returns a pointer to a static string which must be copied if
**		it is to be kept.
*/
PUBLIC CONST char * HTInetString ARGS1(
	SockA*, 	soc_in)
{
    static char string[16];
    sprintf(string, "%d.%d.%d.%d",
	    (int)*((unsigned char *)(&soc_in->sin_addr)+0),
	    (int)*((unsigned char *)(&soc_in->sin_addr)+1),
	    (int)*((unsigned char *)(&soc_in->sin_addr)+2),
	    (int)*((unsigned char *)(&soc_in->sin_addr)+3));
    return string;
}
#endif /* !DECNET */

/*	Check whether string is a valid Internet hostname - kw
**	-------------------------------------------------
**
**  Checks whether
**  - contains only valid chars for domain names (actually, the
**    restrictions are somewhat relaxed),
**  - no leading dots or empty segments,
**  - max. length of dot-separated segment <= 63 (RFC 1034,1035),
**  - total length <= 254 (if it ends with dot) or 253 (otherwise)
**     [an interpretation of RFC 1034,1035, although RFC 1123
**      suggests 255 as limit - kw].
**
**  Note: user (before '@') and port (after ':') components from
**      host part of URL should be already stripped (if appropriate)
**      from the input string.
**
**  On exit,
**	returns 1 if valid, otherwise 0.
*/
PUBLIC BOOL valid_hostname ARGS1(
	CONST char *,	name)
{
    int i=1, iseg = 0;
    CONST char *cp = name;
    if (!(name && *name))
	return NO;
    for (; (*cp && i <= 253); cp++, i++) {
	if (*cp == '.') {
	    if (iseg == 0) {
		return NO;
	    } else {
		iseg = 0;
		continue;
	    }
	} else if (++iseg > 63) {
		return NO;
	}
	if (!isalnum((unsigned char)*cp) &&
	    *cp != '-' && *cp != '_' &&
	    *cp != '$' && *cp != '+') {
	    return NO;
	}
    }
    return (*cp == '\0' || (*cp == '.' && iseg != 0 && cp[1] == '\0'));
}

#ifdef NSL_FORK
/*
**  Function to allow us to be killed with a normal signal (not
**  SIGKILL), but don't go through normal libc exit() processing, which
**  would screw up parent's stdio.  -BL
*/
PRIVATE void quench ARGS1(
	int,	sig GCC_UNUSED)
{
    _exit(2);
}
#endif /* NSL_FORK */

PUBLIC int lynx_nsl_status = HT_OK;

#ifndef DJGPP			/* much excluded! */

#define DEBUG_HOSTENT		/* disable in case of problems */
#define DEBUG_HOSTENT_CHILD  /* for NSL_FORK, may screw up trace file */

/*
**  Two auxiliary functions for name lookup and struct hostent.
**
**  dump_hostent - dumps the contents of a struct hostent to the
**  trace log or stderr, including all pointer values, strings, and
**  addresses, in a format inspired by gdb's print format. - kw
*/
PRIVATE void dump_hostent ARGS2(
    CONST char *,		msgprefix,
    CONST struct hostent *,	phost)
{
    if (TRACE) {
	int i;
	char **pcnt;
	CTRACE(tfp,"%s: %p ", msgprefix, phost);
	if (phost) {
	    CTRACE(tfp,"{ h_name = %p", phost->h_name);
	    if (phost->h_name) {
		CTRACE(tfp, " \"%s\",", phost->h_name);
	    } else {
		CTRACE(tfp, ",");
	    }
	    CTRACE(tfp,"\n\t h_aliases = %p", phost->h_aliases);
	    if (phost->h_aliases) {
		CTRACE(tfp, " {");
		for (pcnt = phost->h_aliases; *pcnt; pcnt++) {
		    CTRACE(tfp,"%s %p \"%s\"",
			   (pcnt == phost->h_aliases ? " " : ", "),
			   *pcnt, *pcnt);
		}
		CTRACE(tfp, "%s0x0 },\n\t",
		       (*phost->h_aliases ? ", " : " "));
	    } else {
		CTRACE(tfp, ",\n\t");
	    }
	    CTRACE(tfp," h_addrtype = %d,", phost->h_addrtype);
	    CTRACE(tfp," h_length = %d,\n\t", phost->h_length);
	    CTRACE(tfp," h_addr_list = %p", phost->h_addr_list);
	    if (phost->h_addr_list) {
		CTRACE(tfp, " {");
		for (pcnt = phost->h_addr_list; *pcnt; pcnt++) {
		    CTRACE(tfp,"%s %p",
			   (pcnt == phost->h_addr_list ? "" : ","),
			   *pcnt);
		    for (i = 0; i < phost->h_length; i++) {
			CTRACE(tfp, "%s%d%s", (i==0 ? " \"" : "."),
			       (int)*((unsigned char *)(*pcnt)+i),
			       (i+1 == phost->h_length ? "\"" : ""));
		    }
		}
		if (*phost->h_addr_list) {
		    CTRACE(tfp, ", 0x0 } }");
		} else {
		    CTRACE(tfp, " 0x0 } }");
		}
	    } else {
		CTRACE(tfp, "}");
	    }
	}
	CTRACE(tfp,"\n");
	fflush(tfp);
    }
}

/*
**  fill_rehostent - copies as much as possible relevant content from
**  the struct hostent pointed to by phost to the char buffer given
**  by rehostent, subject to maximum output length rehostentsize,
**  following pointers and building self-contained output which can be
**  cast to a struct hostent. - kw
**  See also description of LYGetHostByName.
*/
PRIVATE size_t fill_rehostent ARGS3(
    char *,			rehostent,
    size_t,			rehostentsize,
    CONST struct hostent *,	phost)
{
    int num_addrs = 0;
    int num_aliases = 0;
    char **pcnt;
    char *p_next_char;
    char **p_next_charptr;
    size_t name_len = 0;
    size_t required_per_addr;
    size_t curlen = sizeof(struct hostent);
    size_t available = rehostentsize - curlen;
    size_t chk_available, mem_this_alias, required_this_alias;
    int i_addr, i_alias;

    if (!phost)
	return 0;
    required_per_addr = phost->h_length + sizeof(char *);
    if (phost->h_addr_list)
	available -= sizeof(phost->h_addr_list[0]);
    if (phost->h_aliases)
	available -= sizeof(phost->h_aliases[0]);
    if (phost->h_name)
	available--;
    if (phost->h_addr_list) {
	if (phost->h_addr_list[0]) {
	    if (available >= required_per_addr) {
		num_addrs++;
		available -= required_per_addr;
	    }
	}
    }
    if (phost->h_name) {
	name_len = strlen(phost->h_name);
	if (available >= name_len) {
	    available -= name_len;
	} else {
	    name_len = 0;
	}
    }
    if (num_addrs) {
	for (pcnt=phost->h_addr_list+1; *pcnt; pcnt++) {
	    if (available >= required_per_addr) {
		num_addrs++;
		available -= required_per_addr;
	    } else {
		break;
	    }
	}
    }
    chk_available = available;
    if (phost->h_aliases) {
	for (pcnt=phost->h_aliases; *pcnt; pcnt++) {
	    required_this_alias = sizeof(phost->h_aliases[0]) +
		strlen(*pcnt) + 1;
	    if (chk_available >= required_this_alias) {
		num_aliases++;
		chk_available -= required_this_alias;
	    }
	}
    }

    ((struct hostent *)rehostent)->h_addrtype = phost->h_addrtype;
    ((struct hostent *)rehostent)->h_length = phost->h_length;
    p_next_charptr = (char **)(rehostent + curlen);
    p_next_char = rehostent + curlen;
    if (phost->h_addr_list)
	p_next_char += (num_addrs+1) * sizeof(phost->h_addr_list[0]);
    if (phost->h_aliases)
	p_next_char += (num_aliases+1) * sizeof(phost->h_aliases[0]);

    if (phost->h_addr_list) {
	((struct hostent *)rehostent)->h_addr_list = p_next_charptr;
	for (pcnt=phost->h_addr_list, i_addr = 0;
	     i_addr < num_addrs;
	     pcnt++, i_addr++) {
	    memcpy(p_next_char, *pcnt, sizeof(phost->h_addr_list[0]));
	    *p_next_charptr++ = p_next_char;
	    p_next_char += sizeof(phost->h_addr_list[0]);
	}
	*p_next_charptr++ = NULL;
    } else {
	((struct hostent *)rehostent)->h_addr_list = NULL;
    }

    if (phost->h_name) {
	((struct hostent *)rehostent)->h_name = p_next_char;
	if (name_len) {
	    strcpy(p_next_char, phost->h_name);
	    p_next_char += name_len + 1;
	} else {
	    *p_next_char++ = '\0';
	}
    } else {
	((struct hostent *)rehostent)->h_name = NULL;
    }

    if (phost->h_aliases) {
	((struct hostent *)rehostent)->h_aliases = p_next_charptr;
	for (pcnt=phost->h_aliases, i_alias = 0;
	     (*pcnt && i_alias < num_addrs);
	     pcnt++, i_alias++) {
	    mem_this_alias = strlen(*pcnt) + 1;
	    required_this_alias = sizeof(phost->h_aliases[0]) +
		mem_this_alias;
	    if (available >= required_this_alias) {
		i_alias++;
		available -= required_this_alias;
		strcpy(p_next_char, *pcnt);
		*p_next_charptr++ = p_next_char;
		p_next_char += mem_this_alias;
	    }
	    p_next_char += sizeof(phost->h_aliases[0]);
	}
	*p_next_charptr++ = NULL;
    } else {
	((struct hostent *)rehostent)->h_aliases = NULL;
    }
    curlen = p_next_char - (char *)rehostent;
    return curlen;
}

#define REHOSTENT_SIZE 128		/* not bigger than pipe buffer! */

#ifndef HAVE_H_ERRNO
#undef  h_errno
#define h_errno my_errno
static int my_errno;
#else /* we do HAVE_H_ERRNO: */
extern int h_errno;
#endif

/*	Resolve an internet hostname, like gethostbyname
**	------------------------------------------------
**
**  On entry,
**	str	points to the given host name, not numeric address,
**		without colon or port number.
**
**  On exit,
**	returns a pointer to a struct hostent in static storage,
**	or NULL in case of error or user interruption.
**
**  The interface is intended to be exactly the same as for (Unix)
**  gethostbyname(), except for the following:
**
**  If NSL_FORK is not used, the result of gethostbyname is returned
**  directly.  Otherwise:
**  All lists, addresses, and strings referred to by pointers in the
**  returned struct are located, together with the returned struct
**  itself, in a buffer of size REHOSTENT_SIZE.  If not everything fits,
**  some info is omitted, but the function is careful to still return
**  a valid structure, without truncating strings; it tries to return,
**  in order of decreasing priority, the first address (h_addr), the
**  official name (h_name), the additional addresses, then alias names.
**
**  If NULL is returned, the reason is made available in the global
**  variable lynx_nsl_status, with one of the following values:
**	HT_INTERRUPTED		Interrupted by user
**	HT_NOT_ACCEPTABLE	Hostname detected as invalid
**				(also sets h_errno)
**	HT_H_ERRNO_VALID	An error occurred, and h_errno holds
**				an appropriate value
**	HT_ERROR		Resolver error, reason not known
**	HT_INTERNAL		Internal error
*/
PUBLIC struct hostent * LYGetHostByName ARGS1(
	CONST char *,	str)
{
#ifndef _WINDOWS_NSL
    CONST char *host = str;
#endif /* _WINDOWS_NSL */
#ifdef NSL_FORK
    /* for transfer of result between from child to parent: */
    static struct {
	struct hostent	h;
	char		rest[REHOSTENT_SIZE];
    } aligned_full_rehostent;
    /*
     * We could define rehosten directly as a
     * static char rehostent[REHOSTENT_SIZE],
     * but the indirect approach via the above struct
     * should automatically take care of alignment requirements.
     * Note that, in addition,
     *  - this must be static, as we shall return a pointer to it
     *    which must remain valid, and
     *  - we have to use the same rehostent in the child process as
     *    in the parent (its address in the parent's address space
     *    must be the same as in the child's, otherwise the internal
     *    pointers built by the child's call to fill_rehostent would
     *    be invalid when seen by the parent). - kw
     */
    char *rehostent = (char *)&aligned_full_rehostent;

    /* for transfer of status from child to parent: */
    struct _statuses {
	size_t rehostentlen;
	int h_length;
	int child_errno;  /* maybe not very useful */
	int child_h_errno;
	BOOL h_errno_valid;
    } statuses;

    size_t rehostentlen = 0;
#endif /* NSL_FORK */

    struct hostent *result_phost = NULL;

    if (!str) {
	CTRACE(tfp, "LYGetHostByName: Can't parse `NULL'.\n");
	lynx_nsl_status = HT_INTERNAL;
	return NULL;
    }
    CTRACE(tfp, "LYGetHostByName: parsing `%s'.\n", str);

	/*  Could disable this if all our callers already check - kw */
    if (HTCheckForInterrupt()) {
	CTRACE (tfp, "LYGetHostByName: INTERRUPTED for '%s'.\n", str);
	lynx_nsl_status = HT_INTERRUPTED;
	return NULL;
    }

    if (!valid_hostname(host)) {
	lynx_nsl_status = HT_NOT_ACCEPTABLE;
#ifdef NO_RECOVERY
	h_errno = NO_RECOVERY;
#endif
	return NULL;
    }

#ifdef _WINDOWS_NSL
    strncpy(host, str, (size_t)512);
#else
    host = str;
#endif /*  _WINDOWS_NSL */

#ifdef MVS	/* Outstanding problem with crash in MVS gethostbyname */
    CTRACE(tfp, "LYGetHostByName: Calling gethostbyname(%s)\n", host);
#endif /* MVS */

    CTRACE_FLUSH(tfp);  /* so child messages will not mess parent log */

    lynx_nsl_status = HT_INTERNAL;	/* should be set to something else below */

#ifdef NSL_FORK
    statuses.h_errno_valid = NO;
	/*
	**  Start block for fork-based gethostbyname() with
	**  checks for interrupts. - Tom Zerucha (tz@execpc.com) & FM
	*/
    {
	int got_rehostent = 0;
	/*
	**	Pipe, child pid, status buffers, start time, select()
	**	control variables.
	*/
	pid_t fpid, waitret;
	int pfd[2], selret, readret, waitstat = 0;
	time_t start_time = time(NULL);
	fd_set readfds;
	struct timeval timeout;
	int dns_patience = 30; /* how many seconds will we wait for DNS? */
	int child_exited = 0;

	    /*
	    **  Reap any children that have terminated since last time
	    **  through.  This might include children that we killed,
	    **  then waited with WNOHANG before they were actually ready
	    **  to be reaped.  (Should be max of 1 in this state, but
	    **  the loop is safe if waitpid() is implemented correctly:
	    **  returns 0 when children exist but none have exited; -1
	    **  with errno == ECHILD when no children.)  -BL
	    */
	do {
	    waitret = waitpid(-1, 0, WNOHANG);
	} while (waitret > 0 || (waitret == -1 && errno == EINTR));
	waitret = 0;

	pipe(pfd);

	if ((fpid = fork()) == 0 ) {
	    struct hostent  *phost;	/* Pointer to host - See netdb.h */
	    /*
	    **  Child - for the long call.
	    **
	    **  Make sure parent can kill us at will.  -BL
	    */
	    (void) signal(SIGTERM, quench);

	    /*
	    **  Also make sure the child does not run one of the
	    **  signal handlers that may have been installed by
	    **  Lynx if one of those signals occurs.  For example
	    **  we don't want the child to remove temp files on
	    **  ^C, let the parent deal with that. - kw
	    */
	    (void) signal(SIGINT, quench);
#ifndef NOSIGHUP
	    (void) signal(SIGHUP, quench);
#endif /* NOSIGHUP */
#ifdef SIGTSTP
	    if (no_suspend)
		(void) signal(SIGTSTP, SIG_IGN);
	    else
		(void) signal(SIGTSTP, SIG_DFL);
#endif /* SIGTSTP */
#ifdef SIGWINCH
	    (void) signal(SIGWINCH, SIG_IGN);
#endif /* SIGWINCH */
#ifndef __linux__
#ifndef DOSPATH
	    signal(SIGBUS, SIG_DFL);
#endif /* DOSPATH */
#endif /* !__linux__ */
	    signal(SIGSEGV, SIG_DFL);
	    signal(SIGILL, SIG_DFL);

	    /*
	    **  Child won't use read side.  -BL
	    */
	    close(pfd[0]);
	    phost = gethostbyname(host);
	    statuses.child_h_errno = h_errno;
	    statuses.h_errno_valid = YES;
#ifdef MVS
	    CTRACE(tfp, "LYGetHostByName: gethostbyname() returned %d\n", phost);
#endif /* MVS */

#ifdef DEBUG_HOSTENT_CHILD
	    dump_hostent("CHILD gethostbyname", phost);
#endif
	    if (OK_HOST(phost)) {
		rehostentlen = fill_rehostent(rehostent, REHOSTENT_SIZE, phost);
#ifdef DEBUG_HOSTENT_CHILD
		dump_hostent("CHILD fill_rehostent", (struct hostent *)rehostent);
#endif
	    }
	    if (rehostentlen <= sizeof(struct hostent) ||
		!OK_HOST((struct hostent *)rehostent)) {
		rehostentlen = 0;
		statuses.h_length = 0;
	    } else {
		statuses.h_length = ((struct hostent *)rehostent)->h_length;
	    }
	    /*
	    **  Send variables indicating status of lookup to parent.
	    **  That includes rehostentlen, which the parent will use
	    **  as the size for the second read (if > 0).
	    */
	    statuses.child_errno = errno;
	    statuses.rehostentlen = rehostentlen;
	    write(pfd[1], &statuses, sizeof(statuses));


	    if (rehostentlen) {
		/*
		**  Return our resulting rehostent through pipe...
		*/
		write(pfd[1], rehostent, rehostentlen);
		_exit(0);
	    } else {
		/*
		**  ... or return error as exit code.
		*/
		_exit(1);
	    }
	}

	/*
	**	(parent) Wait until lookup finishes, or interrupt,
	**	or cycled too many times (just in case) -BL
	*/

	close(pfd[1]);      /* parent won't use write side -BL */

	if (fpid < 0) {     /* fork failed */
		close(pfd[0]);
		goto failed;
	}

	while (child_exited || time(NULL) - start_time < dns_patience) {

	    FD_ZERO(&readfds);
	    /*
	    **  This allows us to abort immediately, not after 1-second
	    **  timeout, when user hits abort key.  Can't do this when
	    **  using SLANG (or at least I don't know how), so SLANG
	    **  users must live with up-to-1s timeout.  -BL
	    **
	    **  Whoops -- we need to make sure stdin is actually
	    **  selectable!  /dev/null isn't, on some systems, which
	    **  makes some useful Lynx invocations fail.  -BL
	    */
	    {
		int kbd_fd = LYConsoleInputFD(TRUE);
		if (kbd_fd != INVSOC) {
		    FD_SET(kbd_fd, &readfds);
		}
	    }

	    timeout.tv_sec = 1;
	    timeout.tv_usec = 0;
	    FD_SET(pfd[0], &readfds);

		/*
		**  Return when data received, interrupted, or failed.
		**  If nothing is waiting, we sleep for 1 second in
		**  select(), to be nice to the system.  -BL
		*/
#ifdef SOCKS
	    if (socks_flag)
		selret = Rselect(pfd[0] + 1, (void *)&readfds, NULL, NULL, &timeout);
	    else
#endif /* SOCKS */
		selret = select(pfd[0] + 1, (void *)&readfds, NULL, NULL, &timeout);

	    if ((selret > 0) && FD_ISSET(pfd[0], &readfds)) {
		/*
		**	First get status, including length of address.  -BL, kw
		*/
		readret = read(pfd[0], &statuses, sizeof(statuses));
		if (readret == sizeof(statuses)) {
		    h_errno = statuses.child_h_errno;
		    errno = statuses.child_errno;
		    if (statuses.h_errno_valid)
			lynx_nsl_status = HT_H_ERRNO_VALID;
		    if (statuses.rehostentlen > sizeof(struct hostent)) {
			/*
			**  Then get the full reorganized hostent.  -BL, kw
			*/
			readret = read(pfd[0], rehostent, statuses.rehostentlen);
#ifdef DEBUG_HOSTENT
			dump_hostent("Read from pipe", (struct hostent *)rehostent);
#endif
			if (readret == (int) statuses.rehostentlen) {
			    got_rehostent = 1;
			    result_phost = (struct hostent *)rehostent;
			    lynx_nsl_status = HT_OK;
			} else if (!statuses.h_errno_valid) {
			    lynx_nsl_status = HT_INTERNAL;
			}
	    	    }
		} else {
		    lynx_nsl_status = HT_ERROR;
		}
		/*
		**  Make sure child is cleaned up.  -BL
		*/
		if (!child_exited)
		    waitret = waitpid(fpid, &waitstat, WNOHANG);
		if (!WIFEXITED(waitstat) && !WIFSIGNALED(waitstat)) {
		    kill(fpid, SIGTERM);
		    waitret = waitpid(fpid, &waitstat, WNOHANG);
		}
		break;
	    }

	    /*
	    **  Clean up if child exited before & no data received.  -BL
	    */
	    if (child_exited) {
		waitret = waitpid(fpid, &waitstat, WNOHANG);
		break;
	    }
	    /*
	    **  If child exited, loop once more looking for data.  -BL
	    */
	    if ((waitret = waitpid(fpid, &waitstat, WNOHANG)) > 0) {
		/*
		**	Data will be arriving right now, so make sure we
		**	don't short-circuit out for too many loops, and
		**	skip the interrupt check.  -BL
		*/
		child_exited = 1;
		continue;
	    }

	    /*
	    **  Abort if interrupt key pressed.
	    */
	    if (HTCheckForInterrupt()) {
		CTRACE(tfp, "LYGetHostByName: INTERRUPTED gethostbyname.\n");
		kill(fpid, SIGTERM);
		waitpid(fpid, NULL, WNOHANG);
		close(pfd[0]);
		lynx_nsl_status = HT_INTERRUPTED;
		return NULL;
	    }
	}
	close(pfd[0]);
	if (waitret <= 0) {
	    kill(fpid, SIGTERM);
	    waitret = waitpid(fpid, &waitstat, WNOHANG);
	}
	if (waitret > 0) {
	    if (WIFEXITED(waitstat)) {
		CTRACE(tfp, "LYGetHostByName: NSL_FORK child %d exited, status 0x%x.\n",
		       (int)waitret, waitstat);
	    } else if (WIFSIGNALED(waitstat)) {
		CTRACE(tfp, "LYGetHostByName: NSL_FORK child %d got signal, status 0x%x!\n",
		       (int)waitret, waitstat);
#ifdef WCOREDUMP
		if (WCOREDUMP(waitstat)) {
		    CTRACE(tfp, "LYGetHostByName: NSL_FORK child %d dumped core!\n",
			   (int)waitret);
		}
#endif /* WCOREDUMP */
	    } else if (WIFSTOPPED(waitstat)) {
		CTRACE(tfp, "LYGetHostByName: NSL_FORK child %d is stopped, status 0x%x!\n",
		       (int)waitret, waitstat);
	    }
	}
	if (!got_rehostent) {
	    goto failed;
	}
    }
#else /* Not NSL_FORK: */

#ifdef _WINDOWS_NSL
    {
#ifdef __BORLANDC__
		HANDLE hThread, dwThreadID;
#else
		unsigned long hThread, dwThreadID;
#endif /* __BORLANDC__ */
		phost = (struct hostent *) NULL;
		hThread = CreateThread((void *)NULL, 4096UL,
#ifdef __BORLANDC__
			 (LPTHREAD_START_ROUTINE)_fork_func,
#else
			 (unsigned long (*)())_fork_func,
#endif /* __BORLANDC__ */
			 (void *)NULL, 0UL, (unsigned long *)&dwThreadID);
		if (!hThread)
			 MessageBox((void *)NULL, "CreateThread",
				"CreateThread Failed", 0L);

		donelookup = FALSE;
		while (!donelookup)
			if (HTCheckForInterrupt())
			 {
			  /* Note that host is a character array and is not freed */
			  /* to avoid possible subthread problems: */
			  if (!CloseHandle(hThread))
				 MessageBox((void *)NULL, "CloseHandle","CloseHandle Failed",
						0L);
			  lynx_nsl_status = HT_INTERRUPTED;
			  return NULL;
			};
		if (phost) {
		    lynx_nsl_status = HT_OK;
		    result_phost = phost;
		} else {
		    lynx_nsl_status = HT_ERROR;
		    goto failed;
		}
    };

#else /* !NSL_FORK, !_WINDOWS_NSL: */
    {
	struct hostent  *phost;
	phost = gethostbyname((char *)host);	/* See netdb.h */
#ifdef MVS
	CTRACE(tfp, "LYGetHostByName: gethostbyname() returned %d\n", phost);
#endif /* MVS */
	if (phost) {
	    lynx_nsl_status = HT_OK;
	    result_phost = phost;
	} else {
	    lynx_nsl_status = HT_H_ERRNO_VALID;
	    goto failed;
	}
    }
#endif /* !NSL_FORK, !_WINDOWS_NSL */
#endif /* !NSL_FORK */

#ifdef DEBUG_HOSTENT
    dump_hostent("End of LYGetHostByName", result_phost);
    CTRACE(tfp, "LYGetHostByName: Resolved name to a hostent.\n");
#endif

    return result_phost;	/* OK */

failed:
    CTRACE(tfp, "LYGetHostByName: Can't find internet node name `%s'.\n",
		host);
    return NULL;
}

#endif /* from here on DJGPP joins us again. */


/*	Parse a network node address and port
**	-------------------------------------
**
**  On entry,
**	str	points to a string with a node name or number,
**		with optional trailing colon and port number.
**	soc_in	points to the binary internet or decnet address field.
**
**  On exit,
**	*soc_in is filled in.  If no port is specified in str, that
**		field is left unchanged in *soc_in.
*/
PUBLIC int HTParseInet ARGS2(
	SockA *,	soc_in,
	CONST char *,	str)
{
    char *port;
    int dotcount_ip = 0;	/* for dotted decimal IP addr */
#ifndef _WINDOWS_NSL
    char *host = NULL;
#endif /* _WINDOWS_NSL */

    if (!str) {
	CTRACE(tfp, "HTParseInet: Can't parse `NULL'.\n");
	return -1;
    }
    CTRACE(tfp, "HTParseInet: parsing `%s'.\n", str);
    if (HTCheckForInterrupt()) {
	CTRACE (tfp, "HTParseInet: INTERRUPTED for '%s'.\n", str);
	return -1;
    }
#ifdef _WINDOWS_NSL
    strncpy(host, str, (size_t)512);
#else
    StrAllocCopy(host, str);	/* Make a copy we can mutilate */
#endif /*  _WINDOWS_NSL */
    /*
    **	Parse port number if present.
    */
    if ((port = strchr(host, ':')) != NULL) {
	*port++ = 0;		/* Chop off port */
	if (port[0] >= '0' && port[0] <= '9') {
#ifdef unix
	    soc_in->sin_port = htons(atol(port));
#else /* VMS: */
#ifdef DECNET
	    soc_in->sdn_objnum = (unsigned char)(strtol(port, (char**)0, 10));
#else
	    soc_in->sin_port = htons((unsigned short)strtol(port,(char**)0,10));
#endif /* Decnet */
#endif /* Unix vs. VMS */
#ifdef SUPPRESS 	/* 1. crashes!?!.  2. Not recommended */
	} else {
	    struct servent * serv = getservbyname(port, (char*)0);
	    if (serv) {
		soc_in->sin_port = serv->s_port;
	    } else {
		CTRACE(tfp, "TCP: Unknown service %s\n", port);
	    }
#endif /* SUPPRESS */
	}
    }

#ifdef DECNET
    /*
    **	Read Decnet node name. @@ Should know about DECnet addresses, but
    **	it's probably worth waiting until the Phase transition from IV to V.
    */
    soc_in->sdn_nam.n_len = min(DN_MAXNAML, strlen(host));  /* <=6 in phase 4 */
    strncpy(soc_in->sdn_nam.n_name, host, soc_in->sdn_nam.n_len + 1);
    CTRACE(tfp, "DECnet: Parsed address as object number %d on host %.6s...\n",
		soc_in->sdn_objnum, host);
#else  /* parse Internet host: */

    if (*host >= '0' && *host <= '9') {   /* Test for numeric node address: */
	char *strptr = host;
	while (*strptr) {
	    if (*strptr == '.') {
		dotcount_ip++;
	    } else if (!isdigit(*strptr)) {
		break;
	    }
	    strptr++;
	}
	if (*strptr) {		/* found non-numeric, assume domain name */
	    dotcount_ip = 0;
	}
    }

    /*
    **	Parse host number if present.
    */
    if (dotcount_ip == 3) {   /* Numeric node address: */

#ifdef DJGPP
	soc_in->sin_addr.s_addr = htonl(aton(host));
#else
#ifdef DGUX_OLD
	soc_in->sin_addr.s_addr = inet_addr(host).s_addr; /* See arpa/inet.h */
#else
#ifdef GUSI
	soc_in->sin_addr = inet_addr(host);		/* See netinet/in.h */
#else
#ifdef HAVE_INET_ATON
	if (!inet_aton(host, &(soc_in->sin_addr))) {
	    CTRACE(tfp, "inet_aton(%s) returns error\n", host);
#ifndef _WINDOWS_NSL
	    FREE(host);
#endif /* _WINDOWS_NSL */
	    return -1;
	}
#else
	soc_in->sin_addr.s_addr = inet_addr(host);	/* See arpa/inet.h */
#endif /* HAVE_INET_ATON */
#endif /* GUSI */
#endif /* DGUX_OLD */
#endif /* DJGPP */
#ifndef _WINDOWS_NSL
	FREE(host);
#endif /* _WINDOWS_NSL */
    } else {		    /* Alphanumeric node name: */

#ifdef MVS	/* Outstanding problem with crash in MVS gethostbyname */
	CTRACE(tfp, "HTParseInet: Calling LYGetHostByName(%s)\n", host);
#endif /* MVS */

#ifdef DJGPP
	if (!valid_hostname(host)) {
	    FREE(host);
	    return HT_NOT_ACCEPTABLE; /* only HTDoConnect checks this. */
	}
	soc_in->sin_addr.s_addr = htonl(resolve(host));
	if (soc_in->sin_addr.s_addr == 0) {
	    goto failed;
	}
#else /* !DJGPP: */
#ifdef _WINDOWS_NSL
	phost = LYGetHostByName(host);	/* See above */
	if (!phost) goto failed;
	memcpy((void *)&soc_in->sin_addr, phost->h_addr, phost->h_length);
#else /* !DJGPP, !_WINDOWS_NSL: */
	{
	    struct hostent  *phost;
	    phost = LYGetHostByName(host);	/* See above */

	    if (!phost) goto failed;
#if defined(VMS) && defined(CMU_TCP)
	    /*
	    **  In LIBCMU, phost->h_length contains not the length of one address
	    **  (four bytes) but the number of bytes in *h_addr, i.e., some multiple
	    **  of four.  Thus we need to hard code the value here, and remember to
	    **  change it if/when IP addresses change in size. :-(	LIBCMU is no
	    **  longer supported, and CMU users are encouraged to obtain and use
	    **  SOCKETSHR/NETLIB instead. - S. Bjorndahl
	    */
	    memcpy((void *)&soc_in->sin_addr, phost->h_addr, 4);
#else
	    if (!phost) goto failed;
	    if (phost->h_length != sizeof soc_in->sin_addr) {
		HTAlwaysAlert(host, gettext("Address length looks invalid"));
	    }
	    memcpy((void *)&soc_in->sin_addr, phost->h_addr, phost->h_length);
#endif /* VMS && CMU_TCP */
	}
#endif /* !DJGPP, !_WINDOWS_NSL */
#endif /* !DJGPP */
#ifndef _WINDOWS_NSL
	FREE(host);
#endif /* _WINDOWS_NSL */

    }	/* Alphanumeric node name */

    CTRACE(tfp, "HTParseInet: Parsed address as port %d, IP address %d.%d.%d.%d\n",
		(int)ntohs(soc_in->sin_port),
		(int)*((unsigned char *)(&soc_in->sin_addr)+0),
		(int)*((unsigned char *)(&soc_in->sin_addr)+1),
		(int)*((unsigned char *)(&soc_in->sin_addr)+2),
		(int)*((unsigned char *)(&soc_in->sin_addr)+3));
#endif	/* Internet vs. Decnet */

    return 0;	/* OK */

failed:
    CTRACE(tfp, "HTParseInet: Can't find internet node name `%s'.\n",
		host);
#ifndef _WINDOWS_NSL
    FREE(host);
#endif /* _WINDOWS_NSL */
    switch (lynx_nsl_status) {
    case HT_NOT_ACCEPTABLE:
    case HT_INTERRUPTED:
	return lynx_nsl_status;
    default:
    return -1;
}
}

#ifdef LY_FIND_LEAKS
/*	Free our name for the host on which we are - FM
**	-------------------------------------------
**
*/
PRIVATE void free_HTTCP_hostname NOARGS
{
    FREE(hostname);
}
#endif /* LY_FIND_LEAKS */

/*	Derive the name of the host on which we are
**	-------------------------------------------
**
*/
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64		/* Arbitrary limit */
#endif /* MAXHOSTNAMELEN */

PRIVATE void get_host_details NOARGS
{
    char name[MAXHOSTNAMELEN+1];	/* The name of this host */
#ifdef UCX
    char *domain_name;			/* The name of this host domain */
#endif /* UCX */
#ifdef NEED_HOST_ADDRESS		/* no -- needs name server! */
    struct hostent * phost;		/* Pointer to host -- See netdb.h */
#endif /* NEED_HOST_ADDRESS */
    int namelength = sizeof(name);

    if (hostname)
	return; 			/* Already done */
    gethostname(name, namelength);	/* Without domain */
    StrAllocCopy(hostname, name);
#ifdef LY_FIND_LEAKS
    atexit(free_HTTCP_hostname);
#endif
#ifdef UCX
    /*
    **	UCX doesn't give the complete domain name.
    **	Get rest from UCX$BIND_DOM logical.
    */
    if (strchr(hostname,'.') == NULL) { 	  /* Not full address */
	domain_name = getenv("UCX$BIND_DOMAIN");
	if (domain_name != NULL) {
	    StrAllocCat(hostname, ".");
	    StrAllocCat(hostname, domain_name);
	}
     }
#endif /* UCX */
    CTRACE(tfp, "TCP: Local host name is %s\n", hostname);

#ifndef DECNET	/* Decnet ain't got no damn name server 8#OO */
#ifdef NEED_HOST_ADDRESS		/* no -- needs name server! */
    phost = gethostbyname(name);	/* See netdb.h */
    if (!OK_HOST(phost)) {
	CTRACE(tfp, "TCP: Can't find my own internet node address for `%s'!!\n",
		    name);
	return;  /* Fail! */
    }
    StrAllocCopy(hostname, phost->h_name);
    memcpy(&HTHostAddress, &phost->h_addr, phost->h_length);
    CTRACE(tfp, "     Name server says that I am `%s' = %s\n",
		hostname, HTInetString(&HTHostAddress));
#endif /* NEED_HOST_ADDRESS */

#endif /* !DECNET */
}

PUBLIC CONST char * HTHostName NOARGS
{
    get_host_details();
    return hostname;
}

/*
**  Interruptable connect as implemented for Mosaic by Marc Andreesen
**  and hacked in for Lynx years ago by Lou Montulli, and further
**  modified over the years by numerous Lynx lovers. - FM
*/
PUBLIC int HTDoConnect ARGS4(
	CONST char *,	url,
	char *, 	protocol,
	int,		default_port,
	int *,		s)
{
    struct sockaddr_in soc_address;
    struct sockaddr_in *soc_in = &soc_address;
    int status;
    char *line = NULL;
    char *p1 = NULL;
    char *at_sign = NULL;
    char *host = NULL;

    /*
    **	Set up defaults.
    */
    memset(soc_in, 0, sizeof(*soc_in));
    soc_in->sin_family = AF_INET;
    soc_in->sin_port = htons(default_port);

    /*
    **	Get node name and optional port number.
    */
    p1 = HTParse(url, "", PARSE_HOST);
    if ((at_sign = strchr(p1, '@')) != NULL) {
	/*
	**  If there's an @ then use the stuff after it as a hostname.
	*/
	StrAllocCopy(host, (at_sign + 1));
    } else {
	StrAllocCopy(host, p1);
    }
    FREE(p1);

    HTSprintf0 (&line, gettext("Looking up %s."), host);
    _HTProgress (line);
    status = HTParseInet(soc_in, host);
    if (status) {
	if (status != HT_INTERRUPTED) {
	    if (status == HT_NOT_ACCEPTABLE) {
		/*  Not HTProgress, so warning won't be overwritten
		 *  immediately; but not HTAlert, because typically
		 *  there will be other alerts from the callers. - kw
		 */
		HTUserMsg2(gettext("Invalid hostname %s"), host);
	    } else {
		HTSprintf0 (&line,
			 gettext("Unable to locate remote host %s."), host);
		_HTProgress(line);
	    }
	    status = HT_NO_DATA;
	}
	FREE(host);
	FREE(line);
	return status;
    }

    HTSprintf0 (&line, gettext("Making %s connection to %s."), protocol, host);
    _HTProgress (line);
    FREE(host);
    FREE(line);

    /*
    **	Now, let's get a socket set up from the server for the data.
    */
    *s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*s == -1) {
	HTAlert(gettext("socket failed."));
	return HT_NO_DATA;
    }

#ifndef DOSPATH
#if !defined(NO_IOCTL) || defined(USE_FCNTL)
    /*
    **	Make the socket non-blocking, so the connect can be canceled.
    **	This means that when we issue the connect we should NOT
    **	have to wait for the accept on the other end.
    */
    {
#ifdef USE_FCNTL
	int ret = fcntl(*s, F_SETFL, O_NONBLOCK);
#else
	int val = 1;
	int ret = IOCTL(*s, FIONBIO, &val);
#endif /* USE_FCNTL */
	if (ret == -1)
	    _HTProgress(gettext("Could not make connection non-blocking."));
    }
#endif /* !NO_IOCTL || USE_FCNTL */
#endif /* !DOSPATH */

    /*
    **	Issue the connect.  Since the server can't do an instantaneous
    **	accept and we are non-blocking, this will almost certainly return
    **	a negative status.
    */
#ifdef SOCKS
    if (socks_flag) {
	status = Rconnect(*s, (struct sockaddr*)&soc_address,
			  sizeof(soc_address));
	/*
	**  For long Rbind.
	*/
	socks_bind_remoteAddr = soc_address.sin_addr.s_addr;
    } else
#endif /* SOCKS */
    status = connect(*s, (struct sockaddr*)&soc_address, sizeof(soc_address));
#ifndef DJGPP
    /*
    **	According to the Sun man page for connect:
    **	   EINPROGRESS	       The socket is non-blocking and the  con-
    **			       nection cannot be completed immediately.
    **			       It is possible to select(2) for	comple-
    **			       tion  by  selecting the socket for writ-
    **			       ing.
    **	According to the Motorola SVR4 man page for connect:
    **	   EAGAIN	       The socket is non-blocking and the  con-
    **			       nection cannot be completed immediately.
    **			       It is possible to select for  completion
    **			       by  selecting  the  socket  for writing.
    **			       However, this is only  possible	if  the
    **			       socket  STREAMS	module	is  the topmost
    **			       module on  the  protocol  stack	with  a
    **			       write  service  procedure.  This will be
    **			       the normal case.
    */
    if ((status < 0) &&
	(SOCKET_ERRNO == EINPROGRESS || SOCKET_ERRNO == EAGAIN)) {
	struct timeval timeout;
	int ret;
	int tries=0;

	ret = 0;
	while (ret <= 0) {
	    fd_set writefds;

	    /*
	    **	Protect against an infinite loop.
	    */
	    if (tries++ >= 180000) {
		HTAlert(gettext("Connection failed for 180,000 tries."));
		return HT_NO_DATA;
	    }

#ifdef _WINDOWS_NSL
	    timeout.tv_sec = 100;
#else
	    timeout.tv_sec = 0;
#endif /* _WINDOWS_NSL */
	    timeout.tv_usec = 100000;
	    FD_ZERO(&writefds);
	    FD_SET(*s, &writefds);
#ifdef SOCKS
	    if (socks_flag)
		ret = Rselect(FD_SETSIZE, NULL,
			      (void *)&writefds, NULL, &timeout);
	    else
#endif /* SOCKS */
	    ret = select(FD_SETSIZE, NULL, (void *)&writefds, NULL, &timeout);

	   /*
	   **  If we suspend, then it is possible that select will be
	   **  interrupted.  Allow for this possibility. - JED
	   */
	   if ((ret == -1) && (errno == EINTR))
	     continue;

	    /*
	    **	Again according to the Sun and Motorola man pages for connect:
	    **	   EALREADY	       The socket is non-blocking and a  previ-
	    **			       ous  connection attempt has not yet been
	    **			       completed.
	    **	Thus if the SOCKET_ERRNO is NOT EALREADY we have a real error,
	    **	and should break out here and return that error.
	    **	Otherwise if it is EALREADY keep on trying to complete the
	    **	connection.
	    */
	    if ((ret < 0) && (SOCKET_ERRNO != EALREADY)) {
		status = ret;
		break;
	    } else if (ret > 0) {
		/*
		**  Extra check here for connection success, if we try to
		**  connect again, and get EISCONN, it means we have a
		**  successful connection.  But don't check with SOCKS.
		*/
#ifdef SOCKS
		if (socks_flag) {
		    status = 0;
		} else {
#endif /* SOCKS */
		status = connect(*s, (struct sockaddr*)&soc_address,
				 sizeof(soc_address));
#ifdef UCX
		/*
		**  A UCX feature: Instead of returning EISCONN
		**		 UCX returns EADDRINUSE.
		**  Test for this status also.
		*/
		if ((status < 0) && ((SOCKET_ERRNO == EISCONN) ||
				     (SOCKET_ERRNO == EADDRINUSE)))
#else
		if ((status < 0) && (SOCKET_ERRNO == EISCONN))
#endif /* UCX */
		{
		    status = 0;
		}

		if (status && (SOCKET_ERRNO == EALREADY)) /* new stuff LJM */
		    ret = 0; /* keep going */
		else
		    break;
#ifdef SOCKS
		}
#endif /* SOCKS */
	    }
#ifdef SOCKS
	    else if (!socks_flag)
#else
	    else
#endif /* SOCKS */
	    {
		/*
		**  The select says we aren't ready yet.  Try to connect
		**  again to make sure.  If we don't get EALREADY or EISCONN,
		**  something has gone wrong.  Break out and report it.
		**
		**  For some reason, SVR4 returns EAGAIN here instead of
		**  EALREADY, even though the man page says it should be
		**  EALREADY.
		**
		**  For some reason, UCX pre 3 apparently returns
		**  errno = 18242 instead the EALREADY or EISCONN.
		*/
		status = connect(*s, (struct sockaddr*)&soc_address,
				 sizeof(soc_address));
		if ((status < 0) &&
		    (SOCKET_ERRNO != EALREADY && SOCKET_ERRNO != EAGAIN) &&
#ifdef UCX
		    (SOCKET_ERRNO != 18242) &&
#endif /* UCX */
		    (SOCKET_ERRNO != EISCONN)) {
		    break;
		}
	    }
	    if (HTCheckForInterrupt()) {
		CTRACE(tfp, "*** INTERRUPTED in middle of connect.\n");
		status = HT_INTERRUPTED;
		SOCKET_ERRNO = EINTR;
		break;
	    }
	}
    }
#endif /* !DJGPP */
    if (status < 0) {
	/*
	**  The connect attempt failed or was interrupted,
	**  so close up the socket.
	*/
	NETCLOSE(*s);
    }
#ifndef DOSPATH
#if !defined(NO_IOCTL) || defined(USE_FCNTL)
    else {
	/*
	**  Make the socket blocking again on good connect.
	*/
#ifdef USE_FCNTL
	int ret = fcntl(*s, F_SETFL, 0);
#else
	int val = 0;
	int ret = IOCTL(*s, FIONBIO, &val);
#endif /* USE_FCNTL */
	if (ret == -1)
	    _HTProgress(gettext("Could not restore socket to blocking."));
    }
#endif /* !NO_IOCTL || USE_FCNTL */
#endif /* !DOSPATH */

    return status;
}

/*
**  This is so interruptible reads can be implemented cleanly.
*/
PUBLIC int HTDoRead ARGS3(
	int,		fildes,
	void *, 	buf,
	unsigned,	nbyte)
{
    int ready, ret;
    fd_set readfds;
    struct timeval timeout;
    int tries=0;
#if defined(UNIX) || defined(UCX)
    int nb;
#endif /* UCX, BSN */

#ifdef UNIX
    if (fildes == 0) {
	/*
	 *  0 can be a valid socket fd, but if it's a tty something must
	 *  have gone wrong. - kw
	 */
	if (isatty(fildes)) {
	    CTRACE(tfp, "HTDoRead - refusing to read fd 0 which is a tty!\n");
	    return -1;
	}
    } else
#endif
    if (fildes <= 0)
	return -1;

    if (HTCheckForInterrupt()) {
	SOCKET_ERRNO = EINTR;
	return (HT_INTERRUPTED);
    }

#if !defined(NO_IOCTL)
    ready = 0;
#else
    ready = 1;
#endif /* bypass for NO_IOCTL */
    while (!ready) {
	/*
	**  Protect against an infinite loop.
	*/
	if (tries++ >= 180000) {
	    HTAlert(gettext("Socket read failed for 180,000 tries."));
	    SOCKET_ERRNO = EINTR;
	    return HT_INTERRUPTED;
	}

	/*
	**  If we suspend, then it is possible that select will be
	**  interrupted.  Allow for this possibility. - JED
	*/
	do {
	    timeout.tv_sec = 0;
	    timeout.tv_usec = 100000;
	    FD_ZERO(&readfds);
	    FD_SET(fildes, &readfds);
#ifdef SOCKS
	    if (socks_flag)
		ret = Rselect(FD_SETSIZE,
			      (void *)&readfds, NULL, NULL, &timeout);
	    else
#endif /* SOCKS */
		ret = select(FD_SETSIZE,
			     (void *)&readfds, NULL, NULL, &timeout);
	} while ((ret == -1) && (errno == EINTR));

	if (ret < 0) {
	    return -1;
	} else if (ret > 0) {
	    ready = 1;
	} else if (HTCheckForInterrupt()) {
	    SOCKET_ERRNO = EINTR;
	    return HT_INTERRUPTED;
	}
    }

#if !defined(UCX) || !defined(VAXC)
#ifdef UNIX
    while ((nb = SOCKET_READ (fildes, buf, nbyte)) == -1) {
	int saved_errno = errno;
	if (errno == EINTR)
	    continue;
#ifdef ERESTARTSYS
	if (errno == ERESTARTSYS)
	    continue;
#endif /* ERESTARTSYS */
	HTInetStatus("read");
	errno = saved_errno;	/* our caller may check it */
	break;
    }
    return nb;
#else  /* UNIX */
    return SOCKET_READ (fildes, buf, nbyte);
#endif /* !UNIX */

#else  /* UCX && VAXC */
    /*
    **	VAXC and UCX problem only.
    */
    errno = vaxc$errno = 0;
    nb = SOCKET_READ (fildes, buf, nbyte);
    CTRACE(tfp,
	   "Read - nb,errno,vaxc$errno: %d %d %d\n", nb,errno,vaxc$errno);
    if ((nb <= 0) && TRACE)
	perror ("HTTCP.C:HTDoRead:read");	   /* RJF */
    /*
    **	An errno value of EPIPE and nb < 0 indicates end-of-file on VAXC.
    */
    if ((nb <= 0) && (errno == EPIPE)) {
	nb = 0;
	errno = 0;
    }
    return nb;
#endif /* UCX, BSN */
}

#ifdef SVR4_BSDSELECT
/*
**  This is a fix for the difference between BSD's select() and
**  SVR4's select().  SVR4's select() can never return a value larger
**  than the total number of file descriptors being checked.  So, if
**  you select for read and write on one file descriptor, and both
**  are true, SVR4 select() will only return 1.  BSD select in the
**  same situation will return 2.
**
**	Additionally, BSD select() on timing out, will zero the masks,
**	while SVR4 does not.  This is fixed here as well.
**
**	Set your tabstops to 4 characters to have this code nicely formatted.
**
**	Jerry Whelan, guru@bradley.edu, June 12th, 1993
*/
#ifdef select
#undef select
#endif /* select */

#ifdef SOCKS
#ifdef Rselect
#undef Rselect
#endif /* Rselect */
#endif /* SOCKS */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

PUBLIC int BSDselect ARGS5(
	int,			nfds,
	fd_set *,		readfds,
	fd_set *,		writefds,
	fd_set *,		exceptfds,
	struct timeval *,	timeout)
{
    int rval,
    i;

#ifdef SOCKS
    if (socks_flag)
	rval = Rselect(nfds, readfds, writefds, exceptfds, timeout);
    else
#endif /* SOCKS */
    rval = select(nfds, readfds, writefds, exceptfds, timeout);

    switch (rval) {
	case -1:
	    return(rval);
	    break;

	case 0:
	    if (readfds != NULL)
		FD_ZERO(readfds);
	    if (writefds != NULL)
		FD_ZERO(writefds);
	    if (exceptfds != NULL)
		FD_ZERO(exceptfds);
	    return(rval);
	    break;

	default:
	    for (i = 0, rval = 0; i < nfds; i++) {
		if ((readfds != NULL) && FD_ISSET(i, readfds))
		    rval++;
		if ((writefds != NULL) && FD_ISSET(i, writefds))
		    rval++;
		if ((exceptfds != NULL) && FD_ISSET(i, exceptfds))
		    rval++;

	    }
	    return(rval);
    }
/* Should never get here */
}
#endif /* SVR4_BSDSELECT */
