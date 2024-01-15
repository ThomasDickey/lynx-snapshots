/*
 * $LynxId: HTTCP.c,v 1.169 2024/01/14 19:19:39 tom Exp $
 *
 *			Generic Communication Code		HTTCP.c
 *			==========================
 *
 *	This code is in common between client and server sides.
 *
 *	16 Jan 92  TBL	Fix strtol() undefined on CMU Mach.
 *	25 Jun 92  JFG	Added DECNET option through TCP socket emulation.
 *	13 Sep 93  MD	Added correct return of vmserrorno for HTInetStatus.
 *			Added decoding of vms error message for MULTINET.
 *	7-DEC-1993 Bjorn S. Nilsson, ALEPH, CERN, VMS UCX ioctl() changes
 *			(done of Mosaic)
 *	19 Feb 94  Danny Mayer	Added Bjorn Fixes to Lynx version
 *	 7 Mar 94  Danny Mayer	Added Fix UCX version for full domain name
 *	20 May 94  Andy Harper	Added support for CMU TCP/IP transport
 *	17 Nov 94  Andy Harper	Added support for SOCKETSHR transport
 *	16 Jul 95  S. Bjorndahl added kluge to deal with LIBCMU bug
 */

#define LYNX_ADDRINFO	struct addrinfo
#define LYNX_HOSTENT	struct hostent

#include <HTUtils.h>
#include <HTParse.h>
#include <HTAlert.h>
#include <HTTCP.h>
#include <LYGlobalDefs.h>	/* added for no_suspend */
#include <LYUtils.h>

#ifdef NSL_FORK
#include <signal.h>
#include <www_wait.h>
#define FREE_NSL_FORK(p) { FREE(p); }
#elif defined(_WINDOWS_NSL)
#define FREE_NSL_FORK(p) if ((p) == gbl_phost) { FREE(p); }
#else
#define FREE_NSL_FORK(p)	/* nothing */
#endif /* NSL_FORK */

#ifdef HAVE_RESOLV_H
#include <resolv.h>
#endif

#ifdef __DJGPP__
#include <netdb.h>
#endif /* __DJGPP__ */

#define OK_HOST(p) ((p) != 0 && ((p)->h_length) != 0)

#ifdef SVR4_BSDSELECT
int BSDselect(int nfds,
	      fd_set * readfds,
	      fd_set * writefds,
	      fd_set * exceptfds,
	      struct timeval *select_timeout);

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

/*
 *  Module-Wide variables
 */
static char *hostname = NULL;	/* The name of this host */

/*
 *  PUBLIC VARIABLES
 */
#ifdef SOCKS
unsigned long socks_bind_remoteAddr;	/* for long Rbind */
#endif /* SOCKS */

/*	Encode INET status (as in sys/errno.h)			  inet_status()
 *	------------------
 *
 *  On entry,
 *	where		gives a description of what caused the error
 *	global errno	gives the error number in the Unix way.
 *
 *  On return,
 *	returns		a negative status in the Unix way.
 */

#ifdef DECL_SYS_ERRLIST
extern char *sys_errlist[];	/* see man perror on cernvax */
extern int sys_nerr;
#endif /* DECL_SYS_ERRLIST */

#ifdef __DJGPP__
static int ResolveYield(void)
{
    return HTCheckForInterrupt()? 0 : 1;
}
#endif

#if defined(VMS) && defined(UCX)
/*
 *  A routine to mimic the ioctl function for UCX.
 *  Bjorn S. Nilsson, 25-Nov-1993. Based on an example in the UCX manual.
 */
#include <HTioctl.h>

int HTioctl(int d,
	    int request,
	    int *argp)
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

    if ((sdc = vaxc$get_sdc(d)) == 0) {
	set_errno(EBADF);
	return -1;
    }
    ioctl_desc.opt = UCX$C_IOCTL;
    ioctl_desc.len = sizeof(struct comm);

    ioctl_desc.addr = &ioctl_comm;
    if (request & IOC_OUT) {
	fun = IO$_SENSEMODE;
	p5 = 0;
	p6 = (char *) &ioctl_desc;
    } else {
	fun = IO$_SETMODE;
	p5 = (char *) &ioctl_desc;
	p6 = 0;
    }
    ioctl_comm.command = request;
    ioctl_comm.addr = (char *) argp;
    status = sys$qiow(0, sdc, fun, iosb, 0, 0, 0, 0, 0, 0, p5, p6);
    if (!(status & 01)) {
	set_errno(status);
	return -1;
    }
    if (!(iosb[0] & 01)) {
	set_errno(iosb[0]);
	return -1;
    }
    return 0;
}
#endif /* VMS && UCX */

#define MY_FORMAT "TCP: Error %d in `SOCKET_ERRNO' after call to %s() failed.\n\t%s\n"
	   /* third arg is transport/platform specific */

/*	Report Internet Error
 *	---------------------
 */
int HTInetStatus(const char *where)
{
    int status;
    int saved_errno = errno;

#ifdef VMS
#ifdef MULTINET
    SOCKET_ERRNO = vmserrno;
#endif /* MULTINET */
#endif /* VMS */

#ifdef VM
    CTRACE((tfp, MY_FORMAT, SOCKET_ERRNO, where,
	    "(Error number not translated)"));	/* What Is the VM equiv? */
#define ER_NO_TRANS_DONE
#endif /* VM */

#ifdef VMS
#ifdef MULTINET
    CTRACE((tfp, MY_FORMAT, SOCKET_ERRNO, where,
	    vms_errno_string()));
#else
    CTRACE((tfp, MY_FORMAT, SOCKET_ERRNO, where,
	    ((SOCKET_ERRNO > 0 && SOCKET_ERRNO <= 65) ?
	     strerror(SOCKET_ERRNO) : "(Error number not translated)")));
#endif /* MULTINET */
#define ER_NO_TRANS_DONE
#endif /* VMS */

#ifdef HAVE_STRERROR
    CTRACE((tfp, MY_FORMAT, SOCKET_ERRNO, where,
	    strerror(SOCKET_ERRNO)));
#define ER_NO_TRANS_DONE
#endif /* HAVE_STRERROR */

#ifndef ER_NO_TRANS_DONE
    CTRACE((tfp, MY_FORMAT, SOCKET_ERRNO, where,
	    (SOCKET_ERRNO < sys_nerr ?
	     sys_errlist[SOCKET_ERRNO] : "Unknown error")));
#endif /* !ER_NO_TRANS_DONE */

#ifdef VMS
#ifndef MULTINET
    CTRACE((tfp,
	    "         Unix error number (SOCKET_ERRNO) = %ld dec\n",
	    SOCKET_ERRNO));
    CTRACE((tfp,
	    "         VMS error (vaxc$errno)    = %lx hex\n",
	    vaxc$errno));
#endif /* MULTINET */
#endif /* VMS */

    set_errno(saved_errno);

#ifdef VMS
    /*
     * uerrno and errno happen to be zero if vmserrno <> 0
     */
#ifdef MULTINET
    status = -vmserrno;
#else
    status = -vaxc$errno;
#endif /* MULTINET */
#else
    status = -SOCKET_ERRNO;
#endif /* VMS */
    return status;
}

/*	Parse a cardinal value				       parse_cardinal()
 *	----------------------
 *
 * On entry,
 *	*pp	    points to first character to be interpreted, terminated by
 *		    non 0:9 character.
 *	*pstatus    points to status already valid
 *	maxvalue    gives the largest allowable value.
 *
 * On exit,
 *	*pp	    points to first unread character
 *	*pstatus    points to status updated iff bad
 */
unsigned int HTCardinal(int *pstatus,
			char **pp,
			unsigned int max_value)
{
    unsigned int n;

    if ((**pp < '0') || (**pp > '9')) {		/* Null string is error */
	*pstatus = -3;		/* No number where one expected */
	return 0;
    }

    n = 0;
    while ((**pp >= '0') && (**pp <= '9'))
	n = n * 10 + (unsigned) (*((*pp)++) - '0');

    if (n > max_value) {
	*pstatus = -4;		/* Cardinal outside range */
	return 0;
    }

    return n;
}

#ifndef DECNET			/* Function only used below for a trace message */
/*	Produce a string for an Internet address
 *	----------------------------------------
 *
 *  On exit,
 *	returns a pointer to a static string which must be copied if
 *		it is to be kept.
 */
const char *HTInetString(LY_SOCKADDR * soc_A)
{
#ifdef INET6
    static char hostbuf[MAXHOSTNAMELEN];
    struct sockaddr *soc_addr = &(soc_A->soc_address);

    getnameinfo(soc_addr,
		SA_LEN(soc_addr),
		hostbuf, (socklen_t) sizeof(hostbuf),
		NULL, 0,
		NI_NUMERICHOST);
    return hostbuf;
#else
    struct sockaddr_in *soc_in = &(soc_A->soc_in);
    static char string[20];

    sprintf(string, "%d.%d.%d.%d",
	    (int) *((unsigned char *) (&soc_in->sin_addr) + 0),
	    (int) *((unsigned char *) (&soc_in->sin_addr) + 1),
	    (int) *((unsigned char *) (&soc_in->sin_addr) + 2),
	    (int) *((unsigned char *) (&soc_in->sin_addr) + 3));
    return string;
#endif /* INET6 */
}
#endif /* !DECNET */

/*	Check whether string is a valid Internet hostname - kw
 *	-------------------------------------------------
 *
 *  Checks whether
 *  - contains only valid chars for domain names (actually, the
 *    restrictions are somewhat relaxed),
 *  - no leading dots or empty segments,
 *  - no segment starts with '-' or '+' [this protects telnet command],
 *  - max. length of dot-separated segment <= 63 (RFC 1034,1035),
 *  - total length <= 254 (if it ends with dot) or 253 (otherwise)
 *     [an interpretation of RFC 1034,1035, although RFC 1123
 *      suggests 255 as limit - kw].
 *
 *  Note: user (before '@') and port (after ':') components from
 *      host part of URL should be already stripped (if appropriate)
 *      from the input string.
 *
 *  On exit,
 *	returns 1 if valid, otherwise 0.
 */
BOOL valid_hostname(char *name)
{
    int i = 1, iseg = 0;
    char *cp = name;

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
	} else if (iseg == 0 && (*cp == '-' || *cp == '+')) {
	    return NO;
	} else if (++iseg > 63) {
	    return NO;
	}
	if (!isalnum(UCH(*cp)) &&
	    *cp != '-' && *cp != '_' &&
	    *cp != '$' && *cp != '+') {
	    return NO;
	}
    }
    return (BOOL) (*cp == '\0' || (*cp == '.' && iseg != 0 && cp[1] == '\0'));
}

/* for transfer of status from child to parent: */
typedef struct _statuses {
    size_t rehostentlen;
    int h_length;
    int child_errno;		/* sometimes useful to pass this on */
    int child_h_errno;
    BOOL h_errno_valid;
} STATUSES;

/*
 *  Function to allow us to be killed with a normal signal (not
 *  SIGKILL), but don't go through normal libc exit() processing, which
 *  would screw up parent's stdio.  -BL
 */
#ifdef NSL_FORK
static void quench(int sig GCC_UNUSED)
{
    _exit(2);
}
#endif

int lynx_nsl_status = HT_OK;

#define DEBUG_HOSTENT		/* disable in case of problems */
#define DEBUG_HOSTENT_CHILD	/* for NSL_FORK, may screw up trace file */

/*
 *  dump_hostent - dumps the contents of a LYNX_HOSTENT to the
 *  trace log or stderr, including all pointer values, strings, and
 *  addresses, in a format inspired by gdb's print format. - kw
 */
static void dump_hostent(const char *msgprefix,
			 const void *data)
{
    if (TRACE) {
	int i;
	char **pcnt;
	const LYNX_HOSTENT *phost = data;

	CTRACE((tfp, "%s: %p ", msgprefix, (const void *) phost));
	if (phost) {
	    CTRACE((tfp, "{ h_name = %p", (void *) phost->h_name));
	    if (phost->h_name) {
		CTRACE((tfp, " \"%s\",", phost->h_name));
	    } else {
		CTRACE((tfp, ","));
	    }
	    CTRACE((tfp, "\n\t h_aliases = %p", (void *) phost->h_aliases));
	    if (phost->h_aliases) {
		CTRACE((tfp, " {"));
		for (pcnt = phost->h_aliases; *pcnt; pcnt++) {
		    CTRACE((tfp, "%s %p \"%s\"",
			    (pcnt == phost->h_aliases ? " " : ", "),
			    (void *) *pcnt, *pcnt));
		}
		CTRACE((tfp, "%s0x0 },\n\t",
			(*phost->h_aliases ? ", " : " ")));
	    } else {
		CTRACE((tfp, ",\n\t"));
	    }
	    CTRACE((tfp, " h_addrtype = %d,", phost->h_addrtype));
	    CTRACE((tfp, " h_length = %d,\n\t", phost->h_length));
	    CTRACE((tfp, " h_addr_list = %p", (void *) phost->h_addr_list));
	    if (phost->h_addr_list) {
		CTRACE((tfp, " {"));
		for (pcnt = phost->h_addr_list; *pcnt; pcnt++) {
		    CTRACE((tfp, "%s %p",
			    (pcnt == phost->h_addr_list ? "" : ","),
			    (void *) *pcnt));
		    for (i = 0; i < phost->h_length; i++) {
			CTRACE((tfp, "%s%d%s", (i == 0 ? " \"" : "."),
				(int) *((unsigned char *) (*pcnt) + i),
				(i + 1 == phost->h_length ? "\"" : "")));
		    }
		}
		if (*phost->h_addr_list) {
		    CTRACE((tfp, ", 0x0 } }"));
		} else {
		    CTRACE((tfp, " 0x0 } }"));
		}
	    } else {
		CTRACE((tfp, "}"));
	    }
	}
	CTRACE((tfp, "\n"));
	fflush(tfp);
    }
}

#ifdef NSL_FORK

/*
 * Even though it is a small amount, we cannot count on reading the whole
 * struct via a pipe in one read -TD
 */
static unsigned read_bytes(int fd, char *buffer, size_t length)
{
    unsigned result = 0;

    while (length != 0) {
	unsigned got = (unsigned) read(fd, buffer, length);

	if ((int) got > 0) {
	    result += got;
	    buffer += got;
	    length -= got;
	} else {
	    break;
	}
    }
    return result;
}

static unsigned read_hostent(int fd, char *buffer, size_t length)
{
    unsigned have = read_bytes(fd, buffer, length);

    if (have) {
	LYNX_HOSTENT *data = (LYNX_HOSTENT *) (void *) buffer;
	char *next_char = (char *) data + sizeof(*data);
	char **next_ptr = (char **) (void *) next_char;
	long offset = 0;
	int n;
	int num_addrs = 0;
	int num_aliases = 0;

	if (data->h_addr_list) {
	    data->h_addr_list = next_ptr;
	    while (next_ptr[num_addrs] != 0) {
		++num_addrs;
	    }
	    next_ptr += (num_addrs + 1);
	    next_char += (size_t) (num_addrs + 1) * sizeof(data->h_addr_list[0]);
	}

	if (data->h_aliases) {
	    data->h_aliases = next_ptr;
	    while (next_ptr[num_aliases] != 0) {
		++num_aliases;
	    }
	    next_char += (size_t) (num_aliases + 1) * sizeof(data->h_aliases[0]);
	}

	if (data->h_name) {
	    offset = next_char - data->h_name;
	    data->h_name = next_char;
	} else if (data->h_addr_list) {
	    offset = next_char - (char *) data->h_addr_list[0];
	} else if (data->h_aliases) {
	    offset = next_char - (char *) data->h_aliases[0];
	}

	if (data->h_addr_list) {
	    for (n = 0; n < num_addrs; ++n) {
		data->h_addr_list[n] += offset;
	    }
	}

	if (data->h_aliases) {
	    for (n = 0; n < num_aliases; ++n) {
		data->h_aliases[n] += offset;
	    }
	}
    }

    return have;
}
#endif /* NSL_FORK */

/*
 *  fill_rehostent - copies as much as possible relevant content from
 *  the LYNX_HOSTENT pointed to by phost to the char buffer given
 *  by rehostent, subject to maximum output length rehostentsize,
 *  following pointers and building self-contained output which can be
 *  cast to a LYNX_HOSTENT. - kw
 *  See also description of LYGetHostByName.
 */
#if defined(NSL_FORK) || defined(_WINDOWS_NSL)

#define REHOSTENT_SIZE 128	/* not bigger than pipe buffer! */

typedef struct {
    LYNX_HOSTENT h;
    char rest[REHOSTENT_SIZE];
} AlignedHOSTENT;

static size_t fill_rehostent(void **rehostent,
			     const LYNX_HOSTENT *phost)
{
    static const char *this_func = "fill_rehostent";

    LYNX_HOSTENT *data = 0;
    int num_addrs = 0;
    int num_aliases = 0;
    char *result = 0;
    char *p_next_char;
    char **p_next_charptr;
    size_t name_len = 0;
    size_t need = sizeof(LYNX_HOSTENT);
    int n;

    if (!phost)
	return 0;

    if (phost->h_name) {
	name_len = strlen(phost->h_name);
	need += name_len + 1;
    }
    if (phost->h_addr_list) {
	while (phost->h_addr_list[num_addrs]) {
	    num_addrs++;
	}
	need += ((size_t) num_addrs + 1) * ((size_t) phost->h_length
					    + sizeof(phost->h_addr_list[0]));
    }
    if (phost->h_aliases) {
	while (phost->h_aliases[num_aliases]) {
	    need += strlen(phost->h_aliases[num_aliases]) + 1;
	    num_aliases++;
	}
	need += ((size_t) num_aliases + 1) * sizeof(phost->h_aliases[0]);
    }

    if ((result = calloc(need, sizeof(char))) == 0)
	  outofmem(__FILE__, this_func);

    *rehostent = result;

    data = (LYNX_HOSTENT *) (void *) result;

    data->h_addrtype = phost->h_addrtype;
    data->h_length = phost->h_length;

    p_next_char = result + sizeof(LYNX_HOSTENT);

    p_next_charptr = (char **) (void *) p_next_char;
    if (phost->h_addr_list)
	p_next_char += (size_t) (num_addrs + 1) * sizeof(phost->h_addr_list[0]);
    if (phost->h_aliases)
	p_next_char += (size_t) (num_aliases + 1) * sizeof(phost->h_aliases[0]);

    if (phost->h_name) {
	data->h_name = p_next_char;
	strcpy(p_next_char, phost->h_name);
	p_next_char += name_len + 1;
    }

    if (phost->h_addr_list) {
	data->h_addr_list = p_next_charptr;
	for (n = 0; n < num_addrs; ++n) {
	    MemCpy(p_next_char, phost->h_addr_list[n], phost->h_length);
	    *p_next_charptr++ = p_next_char;
	    p_next_char += phost->h_length;
	}
	++p_next_charptr;
    }

    if (phost->h_aliases) {
	data->h_aliases = p_next_charptr;
	for (n = 0; n < num_aliases; ++n) {
	    strcpy(p_next_char, phost->h_aliases[n]);
	    *p_next_charptr++ = p_next_char;
	    p_next_char += strlen(phost->h_aliases[n]) + 1;;
	}
    }
    return need;
}
#endif /* NSL_FORK */

/*
 * This chunk of code is used in both win32 and cygwin.
 */
#if defined(_WINDOWS_NSL)
static LYNX_HOSTENT *gbl_phost;	/* Pointer to host - See netdb.h */

#if !(defined(__CYGWIN__) && defined(NSL_FORK))
static int donelookup;

static unsigned long __stdcall _fork_func(void *arg)
{
    const char *host = (const char *) arg;
    static AlignedHOSTENT aligned_full_rehostent;
    char *rehostent = (char *) &aligned_full_rehostent;
    size_t rehostentlen = 0;

#ifdef SH_EX
    unsigned long addr;

    addr = (unsigned long) inet_addr(host);
    if (addr != INADDR_NONE)
	gbl_phost = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET);
    else
	gbl_phost = gethostbyname(host);
#else
    gbl_phost = gethostbyname(host);
#endif

    if (gbl_phost) {
	rehostentlen = fill_rehostent((void **) &rehostent, gbl_phost);
	if (rehostentlen == 0) {
	    gbl_phost = (LYNX_HOSTENT *) NULL;
	} else {
	    gbl_phost = (LYNX_HOSTENT *) rehostent;
	}
    }

    donelookup = TRUE;
    return (unsigned long) 1;	/* nonzero for successful exit */
}
#endif /* __CYGWIN__ */
#endif /* _WINDOWS_NSL */

#ifdef NSL_FORK

#ifndef HAVE_H_ERRNO
#undef  h_errno
#define h_errno my_errno
static int my_errno;

#else /* we do HAVE_H_ERRNO: */
#ifndef h_errno			/* there may be a macro as well as the extern data */
extern int h_errno;
#endif
#endif

static BOOL setup_nsl_fork(void (*really) (const char *,
					   const char *,
					   STATUSES *,
					   void **),
			   unsigned (*readit) (int, char *, size_t),
			   void (*dumpit) (const char *, const void *),
			   const char *host,
			   const char *port,
			   void **rehostent)
{
    static const char *this_func = "setup_nsl_fork";

    STATUSES statuses;

    /*
     * fork-based gethostbyname() with checks for interrupts.
     * - Tom Zerucha (tz@execpc.com) & FM
     */
    int got_rehostent = 0;

#if HAVE_SIGACTION
    sigset_t old_sigset;
    sigset_t new_sigset;
#endif
    /*
     * Pipe, child pid, status buffers, start time, select() control
     * variables.
     */
    int fpid, waitret;
    int pfd[2], selret;
    unsigned readret;

#ifdef HAVE_TYPE_UNIONWAIT
    union wait waitstat;

#else
    int waitstat = 0;
#endif
    time_t start_time = time((time_t *) 0);
    fd_set readfds;
    struct timeval one_second;
    long dns_patience = 30;	/* how many seconds will we wait for DNS? */
    int child_exited = 0;

    memset(&statuses, 0, sizeof(STATUSES));
    statuses.h_errno_valid = NO;

    /*
     * Reap any children that have terminated since last time through.
     * This might include children that we killed, then waited with WNOHANG
     * before they were actually ready to be reaped.  (Should be max of 1
     * in this state, but the loop is safe if waitpid() is implemented
     * correctly:  returns 0 when children exist but none have exited; -1
     * with errno == ECHILD when no children.) -BL
     */
    do {
	waitret = waitpid(-1, 0, WNOHANG);
    } while (waitret > 0 || (waitret == -1 && errno == EINTR));
    waitret = 0;

    IGNORE_RC(pipe(pfd));

#if HAVE_SIGACTION
    /*
     * Attempt to prevent a rare situation where the child could execute
     * the Lynx signal handlers because it gets killed before it even has a
     * chance to reset its handlers, resulting in bogus 'Exiting via
     * interrupt' message and screen corruption or worse.
     * Should that continue to be reported, for systems without
     * sigprocmask(), we need to find a different solutions for those.  -
     * kw 19990430
     */
    sigemptyset(&new_sigset);
    sigaddset(&new_sigset, SIGTERM);
    sigaddset(&new_sigset, SIGINT);
#ifndef NOSIGHUP
    sigaddset(&new_sigset, SIGHUP);
#endif /* NOSIGHUP */
#ifdef SIGTSTP
    sigaddset(&new_sigset, SIGTSTP);
#endif /* SIGTSTP */
#ifdef SIGWINCH
    sigaddset(&new_sigset, SIGWINCH);
#endif /* SIGWINCH */
    sigprocmask(SIG_BLOCK, &new_sigset, &old_sigset);
#endif /* HAVE_SIGACTION */

    if ((fpid = fork()) == 0) {
	/*
	 * Child - for the long call.
	 *
	 * Make sure parent can kill us at will.  -BL
	 */
	(void) signal(SIGTERM, quench);

	/*
	 * Also make sure the child does not run one of the signal handlers
	 * that may have been installed by Lynx if one of those signals
	 * occurs.  For example we don't want the child to remove temp
	 * files on ^C, let the parent deal with that.  - kw
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

#if HAVE_SIGACTION
	/* Restore signal mask to whatever it was before the fork. -kw */
	sigprocmask(SIG_SETMASK, &old_sigset, NULL);
#endif /* HAVE_SIGACTION */

	/*
	 * Child won't use read side.  -BL
	 */
	close(pfd[0]);
#ifdef HAVE_H_ERRNO
	/* to detect cases when it doesn't get set although it should */
	h_errno = -2;
#endif
	set_errno(0);
	really(host, port, &statuses, rehostent);
	/*
	 * Send variables indicating status of lookup to parent.  That
	 * includes rehostentlen, which the parent will use as the size for
	 * the second read (if > 0).
	 */
	if (!statuses.child_errno)
	    statuses.child_errno = errno;
	IGNORE_RC(write(pfd[1], &statuses, sizeof(statuses)));

	if (statuses.rehostentlen) {
	    /*
	     * Return our resulting rehostent through pipe...
	     */
	    IGNORE_RC(write(pfd[1], *rehostent, statuses.rehostentlen));
	    close(pfd[1]);
	    _exit(0);
	} else {
	    /*
	     * ...  or return error as exit code.
	     */
	    _exit(1);
	}
    }
#if HAVE_SIGACTION
    /*
     * (parent) Restore signal mask to whatever it was before the fork.  -
     * kw
     */
    sigprocmask(SIG_SETMASK, &old_sigset, NULL);
#endif /* HAVE_SIGACTION */

    /*
     * (parent) Wait until lookup finishes, or interrupt, or cycled too
     * many times (just in case) -BL
     */

    close(pfd[1]);		/* parent won't use write side -BL */

    if (fpid < 0) {		/* fork failed */
	close(pfd[0]);
	goto failed;
    }

    while (child_exited || (long) (time((time_t *) 0) - start_time) < dns_patience) {

	FD_ZERO(&readfds);
	/*
	 * This allows us to abort immediately, not after 1-second timeout,
	 * when user hits abort key.  Can't do this when using SLANG (or at
	 * least I don't know how), so SLANG users must live with up-to-1s
	 * timeout.  -BL
	 *
	 * Whoops -- we need to make sure stdin is actually selectable!
	 * /dev/null isn't, on some systems, which makes some useful Lynx
	 * invocations fail.  -BL
	 */
	{
	    int kbd_fd = LYConsoleInputFD(TRUE);

	    if (kbd_fd != INVSOC) {
		FD_SET(kbd_fd, &readfds);
	    }
	}

	one_second.tv_sec = 1;
	one_second.tv_usec = 0;
	FD_SET(pfd[0], &readfds);

	/*
	 * Return when data received, interrupted, or failed.  If nothing
	 * is waiting, we sleep for 1 second in select(), to be nice to the
	 * system.  -BL
	 */
#ifdef SOCKS
	if (socks_flag)
	    selret = Rselect(pfd[0] + 1, &readfds, NULL, NULL, &one_second);
	else
#endif /* SOCKS */
	    selret = select(pfd[0] + 1, &readfds, NULL, NULL, &one_second);

	if ((selret > 0) && FD_ISSET(pfd[0], &readfds)) {
	    /*
	     * First get status, including length of address.  -BL, kw
	     */
	    readret = read_bytes(pfd[0], (char *) &statuses, sizeof(statuses));
	    if (readret == sizeof(statuses)) {
		h_errno = statuses.child_h_errno;
		set_errno(statuses.child_errno);
#ifdef HAVE_H_ERRNO
		if (statuses.h_errno_valid) {
		    lynx_nsl_status = HT_H_ERRNO_VALID;
		    /*
		     * If something went wrong in the child process other
		     * than normal lookup errors, and it appears that we
		     * have enough info to know what went wrong, generate
		     * diagnostic output.  ENOMEM observed on linux in
		     * processes constrained with ulimit.  It would be too
		     * unkind to abort the session, access to local files
		     * or through a proxy may still work.  - kw
		     */
		    if (
#ifdef NETDB_INTERNAL		/* linux glibc: defined in netdb.h */
			   (errno && h_errno == NETDB_INTERNAL) ||
#endif
			   (errno == ENOMEM &&
			    statuses.rehostentlen == 0 &&
		    /* should probably be NETDB_INTERNAL if child
		       memory exhausted, but we may find that
		       h_errno remains unchanged. - kw */
			    h_errno == -2)) {
#ifndef MULTINET
			HTInetStatus("CHILD gethostbyname");
#endif
			HTAlert(LYStrerror(statuses.child_errno));
			if (errno == ENOMEM) {
			    /*
			     * Not much point in continuing, right?  Fake a
			     * 'z', should shorten pointless guessing
			     * cycle.  - kw
			     */
			    LYFakeZap(YES);
			}
		    }
		}
#endif /* HAVE_H_ERRNO */
		if (statuses.rehostentlen != 0) {
		    /*
		     * Then get the full reorganized hostent.  -BL, kw
		     */
		    if ((*rehostent = malloc(statuses.rehostentlen)) == 0)
			outofmem(__FILE__, this_func);
		    readret = (*readit) (pfd[0], *rehostent, statuses.rehostentlen);
#ifdef DEBUG_HOSTENT
		    dumpit("Read from pipe", *rehostent);
#endif
		    if (readret == statuses.rehostentlen) {
			got_rehostent = 1;
			lynx_nsl_status = HT_OK;
		    } else if (!statuses.h_errno_valid) {
			lynx_nsl_status = HT_INTERNAL;
		    }
		}
	    } else {
		lynx_nsl_status = HT_ERROR;
	    }
	    /*
	     * Make sure child is cleaned up.  -BL
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
	 * Clean up if child exited before & no data received.  -BL
	 */
	if (child_exited) {
	    waitret = waitpid(fpid, &waitstat, WNOHANG);
	    break;
	}
	/*
	 * If child exited, loop once more looking for data.  -BL
	 */
	if ((waitret = waitpid(fpid, &waitstat, WNOHANG)) > 0) {
	    /*
	     * Data will be arriving right now, so make sure we don't
	     * short-circuit out for too many loops, and skip the interrupt
	     * check.  -BL
	     */
	    child_exited = 1;
	    continue;
	}

	/*
	 * Abort if interrupt key pressed.
	 */
	if (HTCheckForInterrupt()) {
	    CTRACE((tfp, "%s: INTERRUPTED gethostbyname.\n", this_func));
	    kill(fpid, SIGTERM);
	    waitpid(fpid, NULL, WNOHANG);
	    close(pfd[0]);
	    lynx_nsl_status = HT_INTERRUPTED;
	    return FALSE;
	}
    }
    close(pfd[0]);
    if (waitret <= 0) {
	kill(fpid, SIGTERM);
	waitret = waitpid(fpid, &waitstat, WNOHANG);
    }
    if (waitret > 0) {
	if (WIFEXITED(waitstat)) {
	    CTRACE((tfp,
		    "%s: NSL_FORK child %d exited, status 0x%x.\n",
		    this_func, (int) waitret, WEXITSTATUS(waitstat)));
	} else if (WIFSIGNALED(waitstat)) {
	    CTRACE((tfp,
		    "%s: NSL_FORK child %d got signal, status 0x%x!\n",
		    this_func, (int) waitret, WTERMSIG(waitstat)));
#ifdef WCOREDUMP
	    if (WCOREDUMP(waitstat)) {
		CTRACE((tfp,
			"%s: NSL_FORK child %d dumped core!\n",
			this_func, (int) waitret));
	    }
#endif /* WCOREDUMP */
	} else if (WIFSTOPPED(waitstat)) {
	    CTRACE((tfp,
		    "%s: NSL_FORK child %d is stopped, status 0x%x!\n",
		    this_func, (int) waitret, WSTOPSIG(waitstat)));
	}
    }
    if (!got_rehostent) {
	goto failed;
    }
    return TRUE;
  failed:
    return FALSE;
}

/*
 * This is called via the child-side of the fork.
 */
static void really_gethostbyname(const char *host,
				 const char *port GCC_UNUSED,
				 STATUSES * statuses,
				 void **rehostent)
{
    LYNX_HOSTENT *phost;	/* Pointer to host - See netdb.h */
    LYNX_HOSTENT *result = 0;

    (void) port;

    phost = gethostbyname(host);
    statuses->rehostentlen = 0;
    statuses->child_errno = errno;
    statuses->child_h_errno = h_errno;
#ifdef HAVE_H_ERRNO
    statuses->h_errno_valid = YES;
#endif
#ifdef MVS
    CTRACE((tfp, "really_gethostbyname() returned %d\n", phost));
#endif /* MVS */

#ifdef DEBUG_HOSTENT_CHILD
    dump_hostent("CHILD gethostbyname", phost);
#endif
    if (OK_HOST(phost)) {
	statuses->rehostentlen = fill_rehostent(rehostent, phost);
	result = (LYNX_HOSTENT *) (*rehostent);
#ifdef DEBUG_HOSTENT_CHILD
	dump_hostent("CHILD fill_rehostent", result);
#endif
    }
    if (statuses->rehostentlen <= sizeof(LYNX_HOSTENT) || !OK_HOST(result)) {
	statuses->rehostentlen = 0;
	statuses->h_length = 0;
    } else {
	statuses->h_length = result->h_length;
#ifdef HAVE_H_ERRNO
	if (h_errno == -2)	/* success, but h_errno unchanged? */
	    statuses->h_errno_valid = NO;
#endif
    }
}
#endif /* NSL_FORK */

/*	Resolve an internet hostname, like gethostbyname
 *	------------------------------------------------
 *
 *  On entry,
 *	host	points to the given host name, not numeric address,
 *		without colon or port number.
 *
 *  On exit,
 *	returns a pointer to a LYNX_HOSTENT in static storage,
 *	or NULL in case of error or user interruption.
 *
 *  The interface is intended to be exactly the same as for (Unix)
 *  gethostbyname(), except for the following:
 *
 *  If NSL_FORK is not used, the result of gethostbyname is returned
 *  directly.  Otherwise:
 *  All lists, addresses, and strings referred to by pointers in the
 *  returned struct are located, together with the returned struct
 *  itself, in a buffer of size REHOSTENT_SIZE.  If not everything fits,
 *  some info is omitted, but the function is careful to still return
 *  a valid structure, without truncating strings; it tries to return,
 *  in order of decreasing priority, the first address (h_addr_list[0]), the
 *  official name (h_name), the additional addresses, then alias names.
 *
 *  If NULL is returned, the reason is made available in the global
 *  variable lynx_nsl_status, with one of the following values:
 *	HT_INTERRUPTED		Interrupted by user
 *	HT_NOT_ACCEPTABLE	Hostname detected as invalid
 *				(also sets h_errno)
 *	HT_H_ERRNO_VALID	An error occurred, and h_errno holds
 *				an appropriate value
 *	HT_ERROR		Resolver error, reason not known
 *	HT_INTERNAL		Internal error
 */
static LYNX_HOSTENT *LYGetHostByName(char *host)
{
    static const char *this_func = "LYGetHostByName";

#ifdef NSL_FORK
    /* for transfer of result between from child to parent: */
    LYNX_HOSTENT *rehostent = 0;
#endif /* NSL_FORK */

    LYNX_HOSTENT *result_phost = NULL;

#ifdef __DJGPP__
    _resolve_hook = ResolveYield;
#endif

    if (!host) {
	CTRACE((tfp, "%s: Can't parse `NULL'.\n", this_func));
	lynx_nsl_status = HT_INTERNAL;
	return NULL;
    }
    CTRACE((tfp, "%s: parsing `%s'.\n", this_func, host));

    /*  Could disable this if all our callers already check - kw */
    if (HTCheckForInterrupt()) {
	CTRACE((tfp, "%s: INTERRUPTED for '%s'.\n", this_func, host));
	lynx_nsl_status = HT_INTERRUPTED;
	return NULL;
    }

    if (!valid_hostname(host)) {
	lynx_nsl_status = HT_NOT_ACCEPTABLE;
#ifdef NO_RECOVERY
#ifdef _WINDOWS
	WSASetLastError(NO_RECOVERY);
#else
	h_errno = NO_RECOVERY;
#endif
#endif
	return NULL;
    }
#ifdef MVS			/* Outstanding problem with crash in MVS gethostbyname */
    CTRACE((tfp, "%s: Calling gethostbyname(%s)\n", this_func, host));
#endif /* MVS */

    CTRACE_FLUSH(tfp);		/* so child messages will not mess up parent log */

    lynx_nsl_status = HT_INTERNAL;	/* should be set to something else below */

#ifdef NSL_FORK
    if (!setup_nsl_fork(really_gethostbyname,
			read_hostent,
			dump_hostent,
			host, NULL, (void **) &rehostent)) {
	goto failed;
    }
    result_phost = rehostent;
#else /* Not NSL_FORK: */

#ifdef _WINDOWS_NSL
    {
	HANDLE hThread;
	DWORD dwThreadID;

#ifndef __CYGWIN__
	if (!system_is_NT) {	/* for Windows9x */
	    unsigned long t;

	    t = (unsigned long) inet_addr(host);
	    if (t != INADDR_NONE)
		gbl_phost = gethostbyaddr((char *) &t, sizeof(t), AF_INET);
	    else
		gbl_phost = gethostbyname(host);
	} else {		/* for Windows NT */
#endif /* !__CYGWIN__ */
	    gbl_phost = (LYNX_HOSTENT *) NULL;
	    donelookup = FALSE;

#if defined(__CYGWIN__) || defined(USE_WINSOCK2_H)
	    SetLastError(WSAHOST_NOT_FOUND);
#else
	    WSASetLastError(WSAHOST_NOT_FOUND);
#endif

	    hThread = CreateThread(NULL, 4096UL, _fork_func, host, 0UL,
				   &dwThreadID);
	    if (!hThread)
		MessageBox(NULL, "CreateThread",
			   "CreateThread Failed", 0L);

	    while (!donelookup) {
		if (HTCheckForInterrupt()) {
		    /* Note that host is a character array and is not freed */
		    /* to avoid possible subthread problems: */
		    if (!CloseHandle(hThread)) {
			MessageBox((void *) NULL,
				   "CloseHandle", "CloseHandle Failed", 0L);
		    }
		    lynx_nsl_status = HT_INTERRUPTED;
		    return NULL;
		}
	    }
#ifndef __CYGWIN__
	}
#endif /* !__CYGWIN__ */
	if (gbl_phost) {
	    lynx_nsl_status = HT_OK;
	    result_phost = gbl_phost;
	} else {
	    lynx_nsl_status = HT_ERROR;
	    goto failed;
	}
    }

#else /* !NSL_FORK, !_WINDOWS_NSL: */
    {
	LYNX_HOSTENT *phost;

	phost = gethostbyname(host);	/* See netdb.h */
#ifdef MVS
	CTRACE((tfp, "%s: gethostbyname() returned %d\n", this_func, phost));
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
    dump_hostent(this_func, result_phost);
    CTRACE((tfp, "%s: Resolved name to a hostent.\n", this_func));
#endif

    return result_phost;	/* OK */

  failed:
    CTRACE((tfp, "%s: Can't find internet node name `%s'.\n", this_func, host));
    return NULL;
}

BOOLEAN LYCheckHostByName(char *host)
{
    LYNX_HOSTENT *data = LYGetHostByName(host);
    BOOLEAN result = (data != NULL);

    FREE_NSL_FORK(data);
    return result;
}

/*	Parse a network node address and port
 *	-------------------------------------
 *
 *  On entry,
 *	str	points to a string with a node name or number,
 *		with optional trailing colon and port number.
 *	soc_in	points to the binary internet or decnet address field.
 *
 *  On exit,
 *	*soc_in is filled in.  If no port is specified in str, that
 *		field is left unchanged in *soc_in.
 */
#ifndef INET6
static int HTParseInet(SockA *soc_in, const char *str)
{
    static const char *this_func = "HTParseInet";

    char *port;
    int dotcount_ip = 0;	/* for dotted decimal IP addr */
    char *strptr;
    char *host = NULL;

    if (!str) {
	CTRACE((tfp, "%s: Can't parse `NULL'.\n", this_func));
	return -1;
    }
    CTRACE((tfp, "%s: parsing `%s'.\n", this_func, str));
    if (HTCheckForInterrupt()) {
	CTRACE((tfp, "%s: INTERRUPTED for '%s'.\n", this_func, str));
	return -1;
    }
    StrAllocCopy(host, str);	/* Make a copy we can mutilate */
    /*
     * Parse port number if present.
     */
    if ((port = StrChr(host, ':')) != NULL) {
	*port++ = 0;		/* Chop off port */
	strptr = port;
	if (port[0] >= '0' && port[0] <= '9') {
#ifdef UNIX
	    soc_in->sin_port = (PortNumber) htons(strtol(port, &strptr, 10));
#else /* VMS: */
#ifdef DECNET
	    soc_in->sdn_objnum = (unsigned char) (strtol(port, &strptr, 10));
#else
	    soc_in->sin_port = htons((PortNumber) strtol(port, &strptr, 10));
#endif /* Decnet */
#endif /* Unix vs. VMS */
	}
	if (strptr && *strptr != '\0') {
	    FREE(host);
	    HTAlwaysAlert(NULL, gettext("Address has invalid port"));
	    return -1;
	}
    }
#ifdef DECNET
    /*
     * Read Decnet node name.  @@ Should know about DECnet addresses, but it's
     * probably worth waiting until the Phase transition from IV to V.
     */
    soc_in->sdn_nam.n_len = min(DN_MAXNAML, strlen(host));	/* <=6 in phase 4 */
    StrNCpy(soc_in->sdn_nam.n_name, host, soc_in->sdn_nam.n_len + 1);
    CTRACE((tfp,
	    "DECnet: Parsed address as object number %d on host %.6s...\n",
	    soc_in->sdn_objnum, host));
#else /* parse Internet host: */

    if (*host >= '0' && *host <= '9') {		/* Test for numeric node address: */
	strptr = host;
	while (*strptr) {
	    if (*strptr == '.') {
		dotcount_ip++;
	    } else if (!isdigit(UCH(*strptr))) {
		break;
	    }
	    strptr++;
	}
	if (*strptr) {		/* found non-numeric, assume domain name */
	    dotcount_ip = 0;
	}
    }

    /*
     * Parse host number if present.
     */
    if (dotcount_ip == 3)	/* Numeric node address: */
    {
#ifdef GUSI
	soc_in->sin_addr = inet_addr(host);	/* See netinet/in.h */
#else
#ifdef HAVE_INET_ATON
	if (!inet_aton(host, &(soc_in->sin_addr))) {
	    CTRACE((tfp, "inet_aton(%s) returns error\n", host));
	    FREE(host);
	    return -1;
	}
#else
	soc_in->sin_addr.s_addr = inet_addr(host);	/* See arpa/inet.h */
#endif /* HAVE_INET_ATON */
#endif /* GUSI */
	FREE(host);
    } else {			/* Alphanumeric node name: */

#ifdef MVS			/* Outstanding problem with crash in MVS gethostbyname */
	CTRACE((tfp, "%s: Calling LYGetHostByName(%s)\n", this_func, host));
#endif /* MVS */

#ifdef _WINDOWS_NSL
	gbl_phost = LYGetHostByName(host);	/* See above */
	if (!gbl_phost)
	    goto failed;
	MemCpy((void *) &soc_in->sin_addr, gbl_phost->h_addr_list[0], gbl_phost->h_length);
	FREE(gbl_phost);
#else /* !_WINDOWS_NSL */
	{
	    LYNX_HOSTENT *phost;

	    phost = LYGetHostByName(host);	/* See above */

	    if (!phost)
		goto failed;
	    if (phost->h_length != sizeof soc_in->sin_addr) {
		HTAlwaysAlert(host, gettext("Address length looks invalid"));
	    }
	    MemCpy((void *) &soc_in->sin_addr, phost->h_addr_list[0], phost->h_length);
	    FREE_NSL_FORK(phost);
	}
#endif /* _WINDOWS_NSL */

	FREE(host);
    }				/* Alphanumeric node name */

    CTRACE((tfp,
	    "%s: Parsed address as port %d, IP address %d.%d.%d.%d\n",
	    this_func,
	    (int) ntohs(soc_in->sin_port),
	    (int) *((unsigned char *) (&soc_in->sin_addr) + 0),
	    (int) *((unsigned char *) (&soc_in->sin_addr) + 1),
	    (int) *((unsigned char *) (&soc_in->sin_addr) + 2),
	    (int) *((unsigned char *) (&soc_in->sin_addr) + 3)));
#endif /* Internet vs. Decnet */

    return 0;			/* OK */

  failed:
    CTRACE((tfp, "%s: Can't find internet node name `%s'.\n",
	    this_func, host));
    FREE(host);
    switch (lynx_nsl_status) {
    case HT_NOT_ACCEPTABLE:
    case HT_INTERRUPTED:
	return lynx_nsl_status;
    default:
	return -1;
    }
}
#endif /* !INET6 */

#ifdef INET6

static void dump_addrinfo(const char *tag, const void *data)
{
    const LYNX_ADDRINFO *res;
    int count = 0;

    CTRACE((tfp, "dump_addrinfo %s:\n", tag));
    for (res = (const LYNX_ADDRINFO *) data; res; res = res->ai_next) {
	char hostbuf[1024], portbuf[1024];

	++count;
	hostbuf[0] = '\0';
	portbuf[0] = '\0';
	getnameinfo(res->ai_addr, res->ai_addrlen,
		    hostbuf, (socklen_t) sizeof(hostbuf),
		    portbuf, (socklen_t) sizeof(portbuf),
		    NI_NUMERICHOST | NI_NUMERICSERV);

	CTRACE((tfp,
		"\t[%d] family %d, socktype %d, protocol %d addr %s port %s\n",
		count,
		res->ai_family,
		res->ai_socktype,
		res->ai_protocol,
		hostbuf,
		portbuf));
    }
}

#if defined(NSL_FORK)

/*
 * Copy the relevant information (on the child-side).
 */
static size_t fill_addrinfo(void **buffer,
			    const LYNX_ADDRINFO *phost)
{
    static const char *this_func = "fill_addinfo";

    const LYNX_ADDRINFO *q;
    LYNX_ADDRINFO *actual;
    LYNX_ADDRINFO *result;
    int count = 0;
    int limit = 0;
    size_t need = sizeof(LYNX_ADDRINFO);
    char *heap;

    CTRACE((tfp, "filladdr_info %p\n", (const void *) phost));
    for (q = phost; q != 0; q = q->ai_next) {
	++limit;
	need += q->ai_addrlen;
	need += sizeof(LYNX_ADDRINFO);
    }
    CTRACE((tfp, "...fill_addrinfo %d:%lu\n", limit, (unsigned long) need));

    if ((result = (LYNX_ADDRINFO *) calloc(1, need)) == 0)
	outofmem(__FILE__, this_func);

    *buffer = actual = result;
    heap = ((char *) actual) + ((size_t) limit * sizeof(LYNX_ADDRINFO));

    for (count = 0; count < limit; ++count) {

	/*
	 * copying the whole structure seems simpler but because it is not
	 * packed, uninitialized gaps make it hard to analyse with valgrind.
	 */
	/* *INDENT-EQLS* */
	actual->ai_flags    = phost->ai_flags;
	actual->ai_family   = phost->ai_family;
	actual->ai_socktype = phost->ai_socktype;
	actual->ai_protocol = phost->ai_protocol;
	actual->ai_addrlen  = phost->ai_addrlen;
	actual->ai_addr     = (struct sockaddr *) (void *) heap;

	MemCpy(heap, phost->ai_addr, phost->ai_addrlen);
	heap += phost->ai_addrlen;

	phost = phost->ai_next;

	actual->ai_next = ((count + 1 < limit)
			   ? (actual + 1)
			   : 0);
	++actual;
    }
    return (size_t) (heap - (char *) result);
}

/*
 * Read data, repair pointers as done in fill_addrinfo().
 */
static unsigned read_addrinfo(int fd, char *buffer, size_t length)
{
    unsigned result = read_bytes(fd, buffer, length);
    LYNX_ADDRINFO *actual = (LYNX_ADDRINFO *) (void *) buffer;
    LYNX_ADDRINFO *res;
    int count = 0;
    int limit;
    char *heap;

    CTRACE((tfp, "read_addrinfo length %lu\n", (unsigned long) length));
    for (limit = 0; actual[limit].ai_next; ++limit) {
    }
    ++limit;
    heap = (char *) (actual + limit);
    CTRACE((tfp, "...read_addrinfo %d items\n", limit));

    for (res = actual, count = 0; count < limit; ++count) {
	res->ai_addr = (struct sockaddr *) (void *) heap;
	heap += res->ai_addrlen;
	if (count < limit - 1) {
	    res->ai_next = (res + 1);
	    ++res;
	} else {
	    res->ai_next = 0;
	}
    }

#ifdef DEBUG_HOSTENT
    dump_addrinfo("read_addrinfo", buffer);
#endif
    return result;
}

/*
 * This is called via the child-side of the fork.
 */
static void really_getaddrinfo(const char *host,
			       const char *port,
			       STATUSES * statuses,
			       void **result)
{
    LYNX_ADDRINFO hints, *res = 0;
    int error;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(host, port, &hints, &res);
    if (error) {
	CTRACE((tfp, "HTGetAddrInfo: getaddrinfo(%s, %s): %s\n", host, port,
		gai_strerror(error)));
    } else {
	statuses->child_errno = errno;
	statuses->child_h_errno = h_errno;
#ifdef HAVE_H_ERRNO
	statuses->h_errno_valid = YES;
#endif

#ifdef DEBUG_HOSTENT_CHILD
	dump_addrinfo("CHILD getaddrinfo", res);
#endif
	statuses->rehostentlen = fill_addrinfo(result, res);
#ifdef DEBUG_HOSTENT_CHILD
	dump_addrinfo("CHILD fill_addrinfo", (const LYNX_ADDRINFO *) (*result));
#endif
	if (statuses->rehostentlen <= sizeof(LYNX_ADDRINFO) || (*result) == NULL) {
	    statuses->rehostentlen = 0;
	    statuses->h_length = 0;
	} else {
	    statuses->h_length = (int) (((LYNX_ADDRINFO *) (*result))->ai_addrlen);
	}
	freeaddrinfo(res);
    }
}
#endif /* NSL_FORK */

static LYNX_ADDRINFO *HTGetAddrInfo(const char *str,
				    const int defport)
{
#ifdef NSL_FORK
    /* for transfer of result between from child to parent: */
    void *readdrinfo = 0;

#else
    LYNX_ADDRINFO hints;
    int error;
#endif /* NSL_FORK */
    LYNX_ADDRINFO *res;
    char *p;
    char *s = NULL;
    char *host, *port;
    char pbuf[80];

    StrAllocCopy(s, str);

    if (s[0] == '[' && (p = StrChr(s, ']')) != NULL) {
	*p++ = '\0';
	host = s + 1;
    } else {
	p = s;
	host = &s[0];
    }
    port = strrchr(p, ':');
    if (port) {
	*port++ = '\0';
    } else {
	sprintf(pbuf, "%d", defport);
	port = pbuf;
    }

#ifdef NSL_FORK
    if (setup_nsl_fork(really_getaddrinfo,
		       read_addrinfo,
		       dump_addrinfo,
		       host, port, &readdrinfo)) {
	res = readdrinfo;
    } else {
	res = NULL;
    }
#else
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(host, port, &hints, &res);
    if (error || !res) {
	CTRACE((tfp, "HTGetAddrInfo: getaddrinfo(%s, %s): %s\n", host, port,
		gai_strerror(error)));
	res = NULL;
    }
#endif

    free(s);
#ifdef DEBUG_HOSTENT
    dump_addrinfo("HTGetAddrInfo", res);
#endif
    return res;
}

BOOLEAN HTCheckAddrInfo(const char *str, const int defport)
{
    LYNX_ADDRINFO *data = HTGetAddrInfo(str, defport);
    BOOLEAN result = (data != 0);

    FREE_NSL_FORK(data);
    return result;
}
#endif /* INET6 */

#ifdef LY_FIND_LEAKS
/*	Free our name for the host on which we are - FM
 *	-------------------------------------------
 *
 */
static void free_HTTCP_hostname(void)
{
    FREE(hostname);
}
#endif /* LY_FIND_LEAKS */

/*	Derive the name of the host on which we are
 *	-------------------------------------------
 *
 */
static void get_host_details(void)
{
    char name[MAXHOSTNAMELEN + 1];	/* The name of this host */

#ifdef UCX
    char *domain_name;		/* The name of this host domain */
#endif /* UCX */
#ifdef NEED_HOST_ADDRESS	/* no -- needs name server! */
#ifdef INET6
    LYNX_ADDRINFO hints, *res;
    int error;

#else
    LYNX_HOSTENT *phost;	/* Pointer to host -- See netdb.h */
#endif /* INET6 */
#endif /* NEED_HOST_ADDRESS */
    size_t namelength = sizeof(name);

    if (hostname)
	return;			/* Already done */
    gethostname(name, namelength);	/* Without domain */
    StrAllocCopy(hostname, name);
#ifdef LY_FIND_LEAKS
    atexit(free_HTTCP_hostname);
#endif
#ifdef UCX
    /*
     * UCX doesn't give the complete domain name.  Get rest from UCX$BIND_DOM
     * logical.
     */
    if (StrChr(hostname, '.') == NULL) {	/* Not full address */
	domain_name = LYGetEnv("UCX$BIND_DOMAIN");
	if (domain_name == NULL)
	    domain_name = LYGetEnv("TCPIP$BIND_DOMAIN");
	if (domain_name != NULL) {
	    StrAllocCat(hostname, ".");
	    StrAllocCat(hostname, domain_name);
	}
    }
#endif /* UCX */
    CTRACE((tfp, "TCP: Local host name is %s\n", hostname));

#ifndef DECNET			/* Decnet ain't got no damn name server 8#OO */
#ifdef NEED_HOST_ADDRESS	/* no -- needs name server! */
#ifdef INET6
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    error = getaddrinfo(name, NULL, &hints, &res);
    if (error || !res || !res->ai_canonname) {
	CTRACE((tfp, "TCP: %s: `%s'\n", gai_strerror(error), name));
	if (res)
	    freeaddrinfo(res);
	return;			/* Fail! */
    }
    StrAllocCopy(hostname, res->ai_canonname);
    MemCpy(&HTHostAddress, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
#else
    phost = gethostbyname(name);	/* See netdb.h */
    if (!OK_HOST(phost)) {
	CTRACE((tfp,
		"TCP: Can't find my own internet node address for `%s'!!\n",
		name));
	return;			/* Fail! */
    }
    StrAllocCopy(hostname, phost->h_name);
    MemCpy(&HTHostAddress, &phost->h_addr_list[0], phost->h_length);
#endif /* INET6 */
    CTRACE((tfp, "     Name server says that I am `%s' = %s\n",
	    hostname, HTInetString(&HTHostAddress)));
#endif /* NEED_HOST_ADDRESS */

#endif /* !DECNET */
}

const char *HTHostName(void)
{
    get_host_details();
    return hostname;
}

#ifdef _WINDOWS
#define SET_EINTR WSASetLastError(EINTR)
#else
#define SET_EINTR SOCKET_ERRNO = EINTR
#endif

static BOOL HTWasInterrupted(int *status)
{
    BOOL result = FALSE;

    if (HTCheckForInterrupt()) {
	result = TRUE;
	*status = HT_INTERRUPTED;
	SET_EINTR;
    }
    return result;
}

#define TRIES_PER_SECOND 10

/*
 * Set the select-timeout to 0.1 seconds.
 */
static void set_timeout(struct timeval *timeoutp)
{
    timeoutp->tv_sec = 0;
    timeoutp->tv_usec = 100000;
}

#ifndef MULTINET		/* SOCKET_ERRNO != errno ? */
#if !defined(UCX) || !defined(VAXC)	/* errno not modifiable ? */
#define SOCKET_DEBUG_TRACE	/* show errno status after some system calls */
#endif /* UCX && VAXC */
#endif /* MULTINET */
/*
 *  Interruptible connect as implemented for Mosaic by Marc Andreesen
 *  and hacked in for Lynx years ago by Lou Montulli, and further
 *  modified over the years by numerous Lynx lovers. - FM
 */
int HTDoConnect(const char *url,
		const char *protocol,
		int default_port,
		int *s)
{
    char *socks5_host = NULL;
    unsigned socks5_host_len = 0;
    int socks5_port;
    const char *socks5_orig_url;
    char *socks5_new_url = NULL;
    char *socks5_protocol = NULL;
    int status = HT_OK;
    char *line = NULL;
    char *p1 = NULL;
    char *host = NULL;
    char const *emsg;

#ifdef INET6
    LYNX_ADDRINFO *res = 0, *res0 = 0;

#else
    struct sockaddr_in sock_A;
    struct sockaddr_in *soc_in = &sock_A;
#endif

    *s = -1;			/* nothing is open yet */

    /* In case of a present SOCKS5 proxy, marshal */
    if (socks5_proxy == NULL)
	socks5_proxy = LYGetEnv("SOCKS5_PROXY");
    if ((socks5_orig_url = socks5_proxy) != NULL) {
	int xport;

	xport = default_port;
	socks5_orig_url = url;
	StrAllocCopy(socks5_new_url, url);

	/* Get node name and optional port number of wanted URL */
	if ((p1 = HTParse(socks5_new_url, "", PARSE_HOST)) != NULL) {
	    StrAllocCopy(socks5_host, p1);
	    strip_userid(socks5_host, FALSE);
	    FREE(p1);
	}

	if (isEmpty(socks5_host)) {
	    emsg = gettext("SOCKS5: no hostname found.");
	    status = HT_ERROR;
	    goto report_error;
	}

	if (strlen(socks5_host) > 255) {
	    emsg = gettext("SOCKS5: hostname too long.");
	    status = HT_ERROR;
	    goto report_error;
	}
	socks5_host_len = (unsigned) strlen(socks5_host);

	if (HTParsePort(socks5_new_url, &socks5_port) == NULL)
	    socks5_port = xport;
	FREE(socks5_new_url);

	/* And switch over to our SOCKS5 config; in order to embed that into
	 * lynx environment, prepend protocol prefix */
	default_port = 1080;	/* RFC 1928 */
	HTSACat(&socks5_new_url, "socks://");
	HTSACat(&socks5_new_url, socks5_proxy);
	url = socks5_new_url;

	HTSprintf0(&socks5_protocol,
		   gettext("(for %s at %s) SOCKS5"),
		   protocol, socks5_host);
	protocol = socks5_protocol;
    }
#ifndef INET6
    /*
     * Set up defaults.
     */
    memset(soc_in, 0, sizeof(*soc_in));
    soc_in->sin_family = AF_INET;
    soc_in->sin_port = htons((PortNumber) default_port);
#endif /* INET6 */

    /*
     * Get node name and optional port number.
     */
    p1 = HTParse(url, "", PARSE_HOST);
    StrAllocCopy(host, p1);
    strip_userid(host, FALSE);
    FREE(p1);

    HTSprintf0(&line, "%s%s", WWW_FIND_MESSAGE, host);
    _HTProgress(line);
#ifdef INET6
    /* HTParseInet() is useless! */
    res0 = HTGetAddrInfo(host, default_port);
    if (res0 == NULL) {
	HTSprintf0(&line, gettext("Unable to locate remote host %s."), host);
	_HTProgress(line);
	status = HT_NO_DATA;
	goto cleanup;
    }
#else
    status = HTParseInet(soc_in, host);
    if (status) {
	if (status != HT_INTERRUPTED) {
	    if (status == HT_NOT_ACCEPTABLE) {
		/* Not HTProgress, so warning won't be overwritten immediately;
		 * but not HTAlert, because typically there will be other
		 * alerts from the callers.  - kw
		 */
		HTUserMsg2(gettext("Invalid hostname %s"), host);
	    } else {
		HTSprintf0(&line,
			   gettext("Unable to locate remote host %s."), host);
		_HTProgress(line);
	    }
	    status = HT_NO_DATA;
	}
	goto cleanup;
    }
#endif /* INET6 */

    HTSprintf0(&line, gettext("Making %s connection to %s"), protocol, host);
    _HTProgress(line);

    /*
     * Now, let's get a socket set up from the server for the data.
     */
#ifndef INET6
    *s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*s == -1) {
	status = HT_NO_DATA;
	emsg = gettext("socket failed.");
	goto report_error;
    }
#else
    for (res = res0; res; res = res->ai_next) {
	*s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (*s == -1) {
	    char hostbuf[1024], portbuf[1024];

	    getnameinfo(res->ai_addr, res->ai_addrlen,
			hostbuf, (socklen_t) sizeof(hostbuf),
			portbuf, (socklen_t) sizeof(portbuf),
			NI_NUMERICHOST | NI_NUMERICSERV);
	    HTSprintf0(&line,
		       gettext("socket failed: family %d addr %s port %s."),
		       res->ai_family, hostbuf, portbuf);
	    _HTProgress(line);
	    continue;
	}
#endif /* INET6 */

#if !defined(DOSPATH) || defined(__DJGPP__)
#if !defined(NO_IOCTL) || defined(USE_FCNTL)
	/*
	 * Make the socket non-blocking, so the connect can be canceled.  This
	 * means that when we issue the connect we should NOT have to wait for
	 * the accept on the other end.
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
#endif /* !DOSPATH || __DJGPP__ */

	/*
	 * Issue the connect.  Since the server can't do an instantaneous
	 * accept and we are non-blocking, this will almost certainly return a
	 * negative status.
	 */
#ifdef SOCKS
	if (socks_flag) {
#ifdef INET6
	    status = Rconnect(*s, res->ai_addr, res->ai_addrlen);
#else
	    status = Rconnect(*s, SOCKADDR_OF(sock_A), sizeof(sock_A));
#endif /* INET6 */
	} else
#endif /* SOCKS */
#ifdef INET6
	    status = connect(*s, res->ai_addr, res->ai_addrlen);
#else
	    status = connect(*s, SOCKADDR_OF(sock_A), sizeof(sock_A));
#endif /* INET6 */

	/*
	 * According to the Sun man page for connect:
	 *  EINPROGRESS         The socket is non-blocking and the  con-
	 *                      nection cannot be completed immediately.
	 *                      It is possible to select(2) for  comple-
	 *                      tion  by  selecting the socket for writ-
	 *                      ing.
	 * According to the Motorola SVR4 man page for connect:
	 *  EAGAIN              The socket is non-blocking and the  con-
	 *                      nection cannot be completed immediately.
	 *                      It is possible to select for  completion
	 *                      by  selecting  the  socket  for writing.
	 *                      However, this is only  possible  if  the
	 *                      socket  STREAMS  module  is  the topmost
	 *                      module on  the  protocol  stack  with  a
	 *                      write  service  procedure.  This will be
	 *                      the normal case.
	 */
	CTRACE((tfp, "connect(): status: %d, SOCK_ERRNO: %d\n", status, SOCKET_ERRNO));

	if ((status < 0) &&
	    (SOCKET_ERRNO == EINPROGRESS
#ifdef EAGAIN
	     || SOCKET_ERRNO == EAGAIN
#endif
	    )) {
	    struct timeval select_timeout;
	    int ret;
	    int tries = 0;

#ifdef SOCKET_DEBUG_TRACE
	    if (SOCKET_ERRNO != EINPROGRESS) {
		HTInetStatus("this socket's first connect");
	    }
#endif /* SOCKET_DEBUG_TRACE */
	    ret = 0;
	    while (ret <= 0) {
		fd_set writefds;

		/*
		 * Protect against an infinite loop.
		 */
		if ((tries++ / TRIES_PER_SECOND) >= connect_timeout) {
		    HTAlert(gettext("Connection failed (too many retries)."));
#ifdef INET6
#ifndef NSL_FORK
		    if (res0)
			freeaddrinfo(res0);
#endif
#endif /* INET6 */
		    status = HT_NO_DATA;
		    goto cleanup;
		}
		set_timeout(&select_timeout);
		FD_ZERO(&writefds);
		FD_SET((LYNX_FD) *s, &writefds);
#ifdef SOCKS
		if (socks_flag)
		    ret = Rselect(*s + 1, NULL,
				  &writefds, NULL, &select_timeout);
		else
#endif /* SOCKS */
		    ret = select(*s + 1,
				 NULL,
				 &writefds,
				 NULL,
				 &select_timeout);

#ifdef SOCKET_DEBUG_TRACE
		if (tries == 1) {
		    if (SOCKET_ERRNO != EINPROGRESS) {
			HTInetStatus("this socket's first select");
		    }
		}
#endif /* SOCKET_DEBUG_TRACE */
		/*
		 * If we suspend, then it is possible that select will be
		 * interrupted.  Allow for this possibility.  - JED
		 */
		if ((ret == -1) && (SOCKET_ERRNO == EINTR))
		    continue;

#ifdef SOCKET_DEBUG_TRACE
		if (ret < 0) {
		    HTInetStatus("failed select");
		}
#endif /* SOCKET_DEBUG_TRACE */
		/*
		 * Again according to the Sun and Motorola man pages for
		 * connect:
		 *  EALREADY    The socket is non-blocking and a  previ-
		 *              ous  connection attempt has not yet been
		 *              completed.
		 * Thus if the SOCKET_ERRNO is NOT EALREADY we have a real
		 * error, and should break out here and return that error.
		 * Otherwise if it is EALREADY keep on trying to complete the
		 * connection.
		 */
		if ((ret < 0) && (SOCKET_ERRNO != EALREADY)) {
		    status = ret;
		    break;
		} else if (((SOCKET_ERRNO == EALREADY) ||
			    (SOCKET_ERRNO == EINPROGRESS)) &&
			   HTCheckForInterrupt()) {
		    status = HT_INTERRUPTED;
		    break;
		} else if (ret > 0) {
		    /*
		     * Extra check here for connection success, if we try to
		     * connect again, and get EISCONN, it means we have a
		     * successful connection.  But don't check with SOCKS.
		     */
#ifdef SOCKS
		    if (socks_flag) {
			status = 0;
		    } else {
#endif /* SOCKS */
#ifdef INET6
			status = connect(*s, res->ai_addr, res->ai_addrlen);
#else
			status = connect(*s, SOCKADDR_OF(sock_A), sizeof(sock_A));
#endif /* INET6 */
#ifdef UCX
			/*
			 * A UCX feature:  Instead of returning EISCONN UCX
			 * returns EADDRINUSE.  Test for this status also.
			 */
			if ((status < 0) && ((SOCKET_ERRNO == EISCONN) ||
					     (SOCKET_ERRNO == EADDRINUSE)))
#else
			if ((status < 0) && (SOCKET_ERRNO == EISCONN))
#endif /* UCX */
			{
			    status = 0;
			}

			if (status && (SOCKET_ERRNO == EALREADY))	/* new stuff LJM */
			    ret = 0;	/* keep going */
			else {
#ifdef SOCKET_DEBUG_TRACE
			    if (status < 0) {
				HTInetStatus("confirm-ready connect");
			    }
#endif /* SOCKET_DEBUG_TRACE */
			    break;
			}
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
		     * The select says we aren't ready yet.  Try to connect
		     * again to make sure.  If we don't get EALREADY or
		     * EISCONN, something has gone wrong.  Break out and report
		     * it.
		     *
		     * For some reason, SVR4 returns EAGAIN here instead of
		     * EALREADY, even though the man page says it should be
		     * EALREADY.
		     *
		     * For some reason, UCX pre 3 apparently returns errno =
		     * 18242 instead of EALREADY or EISCONN.
		     */
#ifdef INET6
		    status = connect(*s, res->ai_addr, res->ai_addrlen);
#else
		    status = connect(*s, SOCKADDR_OF(sock_A), sizeof(sock_A));
#endif /* INET6 */
		    if ((status < 0) &&
			(SOCKET_ERRNO != EALREADY
#ifdef EAGAIN
			 && SOCKET_ERRNO != EAGAIN
#endif
			) &&
#ifdef UCX
			(SOCKET_ERRNO != 18242) &&
#endif /* UCX */
			(SOCKET_ERRNO != EISCONN)) {
#ifdef SOCKET_DEBUG_TRACE
			HTInetStatus("confirm-not-ready connect");
#endif /* SOCKET_DEBUG_TRACE */
			break;
		    }
		}
		if (HTWasInterrupted(&status)) {
		    CTRACE((tfp, "*** INTERRUPTED in middle of connect.\n"));
		    break;
		}
	    }
	}
#ifdef SOCKET_DEBUG_TRACE
	else if (status < 0) {
	    HTInetStatus("this socket's first and only connect");
	}
#endif /* SOCKET_DEBUG_TRACE */
#ifdef INET6
	if (status < 0) {
	    NETCLOSE(*s);
	    *s = -1;
	    if (status != HT_INTERRUPTED)
		continue;
	}
	break;
    }
#endif /* INET6 */

#ifdef INET6
    if (*s < 0)
#else
    if (status < 0)
#endif /* INET6 */
    {
	/*
	 * The connect attempt failed or was interrupted, so close up the
	 * socket.
	 */
	NETCLOSE(*s);
    }
#if !defined(DOSPATH) || defined(__DJGPP__)
#if !defined(NO_IOCTL) || defined(USE_FCNTL)
    else {
	/*
	 * Make the socket blocking again on good connect.
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
#endif /* !DOSPATH || __DJGPP__ */

#ifdef INET6
#ifdef NSL_FORK
    FREE_NSL_FORK(res0);
#else
    if (res0)
	freeaddrinfo(res0);
#endif
#endif /* INET6 */

    /* Now if this was a SOCKS5 proxy connection, go for the real one */
    if (status >= 0 && socks5_orig_url != NULL) {
	unsigned char pbuf[4 + 1 + 255 + 2];
	unsigned i;

	/* RFC 1928: version identifier/method selection message */
	pbuf[0] = 0x05;		/* VER: protocol version: X'05' */
	pbuf[1] = 0x01;		/* NMETHODS: 1 */
	pbuf[2] = 0x00;		/* METHOD: X'00' NO AUTHENTICATION REQUIRED */
	if (NETWRITE(*s, (char *) pbuf, 3) != 3) {
	    goto report_system_err;
	} else if (HTDoRead(*s, pbuf, 2) != 2) {
	    goto report_system_err;
	} else if (pbuf[0] != 0x05 || pbuf[1] != 0x00) {
	    goto report_unexpected_reply;
	}

	/* RFC 1928: CONNECT request */
	HTSprintf0(&line, gettext("SOCKS5: connecting to %s"), socks5_host);
	_HTProgress(line);
	pbuf[0] = 0x05;		/* VER: protocol version: X'05' */
	pbuf[1] = 0x01;		/* CMD: CONNECT X'01' */
	pbuf[2] = 0x00;		/* RESERVED */
	pbuf[3] = 0x03;		/* ATYP: domain name */
	pbuf[4] = (unsigned char) socks5_host_len;
	memcpy(&pbuf[i = 5], socks5_host, socks5_host_len);
	i += socks5_host_len;
	/* C99 */  {
	    unsigned short x;	/* XXX 16-bit? */

	    x = htons((PortNumber) socks5_port);
	    memcpy(&pbuf[i], (unsigned char *) &x, sizeof x);
	    i += (unsigned) sizeof(x);
	}
	if ((size_t) NETWRITE(*s, (char *) pbuf, i) != i) {
	    goto report_system_err;
	} else if ((unsigned) HTDoRead(*s, pbuf, 4) != 4) {
	    goto report_system_err;
	}
	/* Version 5, reserved must be 0 */
	if (pbuf[0] == 0x05 && pbuf[2] == 0x00) {
	    /* Result */
	    switch (pbuf[1]) {
	    case 0x00:
		emsg = NULL;
		break;
	    case 0x01:
		emsg = gettext("SOCKS server failure");
		break;
	    case 0x02:
		emsg = gettext("connection not allowed by ruleset");
		break;
	    case 0x03:
		emsg = gettext("network unreachable");
		break;
	    case 0x04:
		emsg = gettext("host unreachable");
		break;
	    case 0x05:
		emsg = gettext("connection refused");
		break;
	    case 0x06:
		emsg = gettext("TTL expired");
		break;
	    case 0x07:
		emsg = gettext("command not supported");
		break;
	    case 0x08:
		emsg = gettext("address type not supported");
		break;
	    default:
		emsg = gettext("unknown SOCKS error code");
		break;
	    }
	    if (emsg != NULL) {
		goto report_no_connection;
	    }
	} else {
	    goto report_unexpected_reply;
	}

	/* Address type variable; read the BND.PORT with it.
	 * This is actually false since RFC 1928 says that the BND.ADDR reply
	 * to CONNECT contains the IP address, so only 0x01 and 0x04 are
	 * allowed */
	switch (pbuf[3]) {
	case 0x01:
	    i = 4;
	    break;
	case 0x03:
	    i = 1;
	    break;
	case 0x04:
	    i = 16;
	    break;
	default:
	    goto report_unexpected_reply;
	}
	i += (unsigned) sizeof(unsigned short);

	if ((size_t) HTDoRead(*s, pbuf, i) != i) {
	    goto report_system_err;
	} else if (i == 1 + sizeof(unsigned short)) {
	    i = pbuf[0];
	    if ((size_t) HTDoRead(*s, pbuf, i) != i) {
		goto report_system_err;
	    }
	}
    }
    goto cleanup;

  report_system_err:
    emsg = LYStrerror(errno);
    goto report_no_connection;

  report_unexpected_reply:
    emsg = gettext("unexpected reply\n");
    /* FALLTHRU */

  report_no_connection:
    status = HT_NO_CONNECTION;
    /* FALLTHRU */

  report_error:
    HTAlert(emsg);
    if (*s != -1) {
	NETCLOSE(*s);
    }

  cleanup:
    if (socks5_proxy != NULL) {
	FREE(socks5_new_url);
	FREE(socks5_protocol);
	FREE(socks5_host);
    }
    FREE(host);
    FREE(line);
    return status;
}

/*
 *  This is interruptible so reads can be implemented cleanly.
 */
int HTDoRead(int fildes,
	     void *buf,
	     unsigned nbyte)
{
    int result;
    BOOL ready;

#if !defined(NO_IOCTL)
    int ret;
    fd_set readfds;
    struct timeval select_timeout;
    int tries = 0;

#ifdef USE_READPROGRESS
    int otries = 0;
    time_t otime = time((time_t *) 0);
    time_t start = otime;
#endif
#endif /* !NO_IOCTL */

#if defined(UNIX) && !defined(__BEOS__)
    if (fildes == 0) {
	/*
	 * 0 can be a valid socket fd, but if it's a tty something must have
	 * gone wrong.  - kw
	 */
	if (isatty(fildes)) {
	    CTRACE((tfp, "HTDoRead - refusing to read fd 0 which is a tty!\n"));
	    return -1;
	}
    } else
#endif
    if (fildes <= 0) {
	CTRACE((tfp, "HTDoRead - no file descriptor!\n"));
	return -1;
    }

    if (HTWasInterrupted(&result)) {
	CTRACE((tfp, "HTDoRead - interrupted before starting!\n"));
	return (result);
    }
#if defined(NO_IOCTL)
    ready = TRUE;
#else
    ready = FALSE;
    while (!ready) {
	/*
	 * Protect against an infinite loop.
	 */
	if ((tries++ / TRIES_PER_SECOND) >= reading_timeout) {
	    HTAlert(gettext("Socket read failed (too many tries)."));
	    SET_EINTR;
	    result = HT_INTERRUPTED;
	    break;
	}
#ifdef USE_READPROGRESS
	if (tries - otries > TRIES_PER_SECOND) {
	    time_t t = time((time_t *) 0);

	    otries = tries;
	    if (t - otime >= 5) {
		otime = t;
		HTReadProgress((off_t) (-1), (off_t) 0);	/* Put "stalled" message */
	    }
	}
#endif

	/*
	 * If we suspend, then it is possible that select will be interrupted.
	 * Allow for this possibility.  - JED
	 */
	do {
	    set_timeout(&select_timeout);
	    FD_ZERO(&readfds);
	    FD_SET((LYNX_FD) fildes, &readfds);
#ifdef SOCKS
	    if (socks_flag)
		ret = Rselect(fildes + 1,
			      &readfds, NULL, NULL, &select_timeout);
	    else
#endif /* SOCKS */
		ret = select(fildes + 1,
			     &readfds, NULL, NULL, &select_timeout);
	} while ((ret == -1) && (errno == EINTR));

	if (ret < 0) {
	    result = -1;
	    break;
	} else if (ret > 0) {
	    ready = TRUE;
	} else if (HTWasInterrupted(&result)) {
#ifdef USE_READPROGRESS
	    if (tries > TRIES_PER_SECOND)
		result = 0;
#endif
	    break;
	}
    }
#endif /* !NO_IOCTL */

    if (ready) {
#if defined(UCX) && defined(VAXC)
	/*
	 * VAXC and UCX problem only.
	 */
	errno = vaxc$errno = 0;
	result = SOCKET_READ(fildes, buf, nbyte);
	CTRACE((tfp,
		"Read - result,errno,vaxc$errno: %d %d %d\n", result, errno, vaxc$errno));
	if ((result <= 0) && TRACE)
	    perror("HTTCP.C:HTDoRead:read");	/* RJF */
	/*
	 * An errno value of EPIPE and result < 0 indicates end-of-file on VAXC.
	 */
	if ((result <= 0) && (errno == EPIPE)) {
	    result = 0;
	    set_errno(0);
	}
#else
#ifdef UNIX
	while ((result = (int) SOCKET_READ(fildes, buf, nbyte)) == -1) {
	    if (errno == EINTR)
		continue;
#ifdef ERESTARTSYS
	    if (errno == ERESTARTSYS)
		continue;
#endif /* ERESTARTSYS */
	    HTInetStatus("read");
	    break;
	}
#else /* UNIX */
	result = NETREAD(fildes, (char *) buf, nbyte);
#endif /* !UNIX */
#endif /* UCX && VAXC */
    }
#ifdef USE_READPROGRESS
    CTRACE2(TRACE_TIMING, (tfp, "...HTDoRead returns %d (%" PRI_time_t
			   " seconds)\n",
			   result, CAST_time_t (time((time_t *)0) - start)));
#endif
    return result;
}

#ifdef SVR4_BSDSELECT
/*
 *  This is a fix for the difference between BSD's select() and
 *  SVR4's select().  SVR4's select() can never return a value larger
 *  than the total number of file descriptors being checked.  So, if
 *  you select for read and write on one file descriptor, and both
 *  are true, SVR4 select() will only return 1.  BSD select in the
 *  same situation will return 2.
 *
 *	Additionally, BSD select() on timing out, will zero the masks,
 *	while SVR4 does not.  This is fixed here as well.
 *
 *	Set your tabstops to 4 characters to have this code nicely formatted.
 *
 *	Jerry Whelan, guru@bradley.edu, June 12th, 1993
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

int BSDselect(int nfds,
	      fd_set * readfds,
	      fd_set * writefds,
	      fd_set * exceptfds,
	      struct timeval *select_timeout)
{
    int rval, i;

#ifdef SOCKS
    if (socks_flag)
	rval = Rselect(nfds, readfds, writefds, exceptfds, select_timeout);
    else
#endif /* SOCKS */
	rval = select(nfds, readfds, writefds, exceptfds, select_timeout);

    switch (rval) {
    case -1:
	return (rval);

    case 0:
	if (readfds != NULL)
	    FD_ZERO(readfds);
	if (writefds != NULL)
	    FD_ZERO(writefds);
	if (exceptfds != NULL)
	    FD_ZERO(exceptfds);
	return (rval);

    default:
	for (i = 0, rval = 0; i < nfds; i++) {
	    if ((readfds != NULL) && FD_ISSET(i, readfds))
		rval++;
	    if ((writefds != NULL) && FD_ISSET(i, writefds))
		rval++;
	    if ((exceptfds != NULL) && FD_ISSET(i, exceptfds))
		rval++;

	}
	return (rval);
    }
/* Should never get here */
}
#endif /* SVR4_BSDSELECT */
