/*                   Lynx CGI support                              LYCgi.c
**                   ================
**
** Authors
**          GL      George Lindholm <George.Lindholm@ubc.ca>
**
** History
**      15 Jun 95   Created as way to provide a lynx based service with
**                  dynamic pages without the need for a http daemon. GL
**      27 Jun 95   Added <index> (command line) support. Various cleanup
**                  and bug fixes. GL
**
** Bugs
**      If the called scripts aborts before sending the mime headers then
**      lynx hangs.
**
**      Should do something about SIGPIPE, (but then it should never happen)
**
**      No support for redirection. Or mime-types.
**
**      Should try and parse for a HTTP 1.1 header in case we are "calling" a
**      nph- script.
*/ 

#include "HTUtils.h"
#include "tcp.h"
#include "HTTP.h"
#include "HTParse.h"
#include "HTTCP.h"
#include "HTFormat.h"
#include "HTFile.h"
#include "HTAlert.h"
#include "HTMIME.h"
#include "HTAABrow.h"

#include "LYGlobalDefs.h"
#include "LYUtils.h"
#include "HTML.h"
#include "HTInit.h"
#include "LYGetFile.h"
#include "LYBookmark.h"
#include "GridText.h"
#include <ctype.h>
#include "LYCgi.h"
#include "LYSignal.h"
#include "LYLocal.h"

#include "LYLeaks.h"

#define FREE(x) if (x) {free(x); x = NULL;}

struct _HTStream 
{
  HTStreamClass * isa;
};

PRIVATE char **env = NULL;  /* Environment variables */
PRIVATE int envc_size = 0;  /* Slots in environment array */
PRIVATE envc = 0;	    /* Slots used so far */
#ifdef LYNXCGI_LINKS
PRIVATE char user_agent[64];
PRIVATE char server_software[64];
#endif /* LYNXCGI_LINKS */

extern BOOLEAN reloading;

PRIVATE void add_environment_value PARAMS((char *env_value));


/*
 * Simple routine for expanding the environment array and adding a value to
 * it
 */
PRIVATE void add_environment_value ARGS1(
	char *,	env_value)
{
    if (envc == envc_size) {   /* Need some more slots */
	envc_size += 10;
	if (env)
	    env = (char **)realloc(env,
				   sizeof(env[0]) * (envc_size + 2));
						/* + terminator and base 0 */
	else
	    env = (char **)malloc(sizeof(env[0]) * (envc_size + 2));
						/* + terminator and base 0 */
	if (env == NULL) {
	    outofmem(__FILE__, "LYCgi");
	}
    }

    env[envc++] = env_value;
    env[envc] = NULL;      /* Make sure it is always properly terminated */
}
    
/*
 * Add the value of an existing environment variable to those passed on to the
 * lynxcgi script.
 */
PUBLIC void add_lynxcgi_environment ARGS1(
	CONST char *,	variable_name)
{
    char *env_value;

    env_value = getenv(variable_name);
    if (env_value != NULL) {
	char *add_value = NULL;

	add_value = (char *)malloc(strlen(variable_name) +
				   strlen(env_value) + 2);
	if (add_value == NULL) {
	    outofmem(__FILE__, "LYCgi");
	}
	strcpy(add_value, variable_name);
	strcat(add_value, "=");
	strcat(add_value, env_value);
	add_environment_value(add_value);
    }
}

PUBLIC int LYLoadCGI ARGS4(
	CONST char *, 		arg,
	HTParentAnchor *,	anAnchor,
	HTFormat,		format_out,
	HTStream*,		sink)
{
    int status;
#ifdef LYNXCGI_LINKS
#ifndef VMS
    char *cp;
    struct stat stat_buf;
    char *pgm = NULL;		        /* executable */
    char *pgm_args = NULL;	        /* and its argument(s) */

    if (arg) {
	if (strncmp(arg, "lynxcgi://localhost", 19) == 0) {
	    StrAllocCopy(pgm, arg+19);
	} else {
	    StrAllocCopy(pgm, arg+8);
	}
	if ((cp=strchr(pgm, '?')) != NULL) { /* Need to terminate executable */
	    *cp++ = '\0';
	    pgm_args = cp;
	}
    }

    if (!arg || !*arg) {
	HTAlert(BAD_REQUEST);
	status = -2;

    } else if (stat(pgm, &stat_buf) < 0) {
	HTAlert("Unable to access cgi script");
	if (TRACE) {
	    perror("LYNXCGI: stat() failed");
	}
	status = -4;

    } else if (!(S_ISREG(stat_buf.st_mode) &&
		 stat_buf.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))) {
	/* Not a runnable file, See if we can load it using file: code */
	char *temp = NULL;
	char *new_arg = NULL;

	StrAllocCopy(temp, pgm);
	StrAllocCopy(new_arg, "file://localhost");
	StrAllocCat(new_arg, temp);

	if (TRACE) {
	    fprintf(stderr,
		    "%s is not an executable file, passing the buck.\n", arg);
	}
	status = HTLoadFile(new_arg, anAnchor, format_out, sink);
	FREE(new_arg);

    } else if (no_lynxcgi) {
	_statusline(CGI_DISABLED);
	sleep(MessageSecs);
	status = HT_NOT_LOADED;

    } else if (!reloading && no_bookmark_exec && bookmark_page &&
	       (strstr(HTLoadedDocumentURL(), bookmark_page) ||
		!strcmp(HTLoadedDocumentTitle(),
			MOSAIC_BOOKMARK_TITLE))) {
	_statusline(BOOKMARK_EXEC_DISABLED);
	sleep(MessageSecs);
	status = HT_NOT_LOADED;

    } else if (!reloading && !exec_ok(HTLoadedDocumentURL(), pgm,
			CGI_PATH)) { /* exec_ok gives out msg. */
	status = HT_NOT_LOADED;

    } else {
	HTFormat format_in;
	HTStream *target  = NULL;		/* Unconverted data */
	int fd1[2], fd2[2];
	char buf[1024];
	pid_t pid;
#if defined(NeXT) || defined(AIX4) || defined(sony_news)
	union wait wstatus;
#else
	int wstatus;
#endif /* NeXT || AIX4 || sony_news */

	/* Decode full HTTP response */
	format_in = HTAtom_for("www/mime");
		
	target = HTStreamStack(format_in,
			       format_out,
			       sink, anAnchor);
		
	if (!target || target == NULL) {
	    sprintf(buf, CANNOT_CONVERT_I_TO_O,
		    HTAtom_name(format_in), HTAtom_name(format_out));
	    _statusline(buf);
	    sleep(AlertSecs);
	    status = HT_NOT_LOADED;

	} else if (anAnchor->post_data && pipe(fd1) < 0) {
	    HTAlert(CONNECT_SET_FAILED);
	    if (TRACE) {
		perror("LYNXCGI: pipe() failed");
	    }
	    status = -3;
	    
	} else if (pipe(fd2) < 0) {
	    HTAlert(CONNECT_SET_FAILED);
	    if (TRACE) {
		perror("LYNXCGI: pipe() failed");
	    }
	    close(fd1[0]);
	    close(fd1[1]);
	    status = -3;
	    
	} else {	
	    static BOOL first_time = TRUE;      /* One time setup flag */

	    if (first_time) {	/* Set up static environment variables */
		first_time = FALSE;	/* Only once */
		
		add_environment_value("REMOTE_HOST=localhost");
		add_environment_value("REMOTE_ADDR=127.0.0.1");
		
		sprintf(user_agent, "HTTP_USER_AGENT=%s/%s libwww/%s",
			LYNX_NAME, LYNX_VERSION, HTLibraryVersion);
		add_environment_value(user_agent);
		
		sprintf(server_software, "SERVER_SOFTWARE=%s/%s",
			LYNX_NAME, LYNX_VERSION);
		add_environment_value(server_software);
	    }
	    
	    if ((pid = fork()) > 0) { /* The good, */
		int chars, total_chars;
		
		close(fd2[1]);
		
		if (anAnchor->post_data) {
		    close(fd1[0]);

		    /* We have form data to push across the pipe */
		    if (TRACE) {
			fprintf(stderr, "LYNXCGI: Doing post, content-type '%s'\n",
				anAnchor->post_content_type);
			fprintf(stderr,
				"LYNXCGI: Writing:\n%s----------------------------------\n",
				anAnchor->post_data);			
		    }
		    write(fd1[1], anAnchor->post_data,
			  strlen(anAnchor->post_data));
		}
		
		total_chars = 0;
		while((chars = read(fd2[0], buf, sizeof(buf))) > 0) {
		    char line[40];
		    
		    total_chars += chars;
		    sprintf (line, "Read %d bytes of data.", total_chars);
		    HTProgress(line);
		    if (TRACE) {
			fprintf(stderr, "LYNXCGI: Rx: %s\n", buf);
		    }
		    
		    (*target->isa->put_block)(target, buf, chars);
		}
#if defined(NeXT) || defined(AIX4) || defined(sony_news)
		while (wait(&wstatus) != pid)
		    ; /* do nothing */
#else
		waitpid(pid, &wstatus, 0); /* wait for child */
#endif /* NeXT || AIX4 || sony_news */
		if (anAnchor->post_data) {
		    close(fd1[1]);
		}
		close(fd2[0]);
		status = HT_LOADED;
		
	    } else if (pid == 0) { /* The Bad, */
		char **argv = NULL;
		char post_len[32];
		int argv_cnt = 3; /* name, one arg and terminator */
		char **cur_argv = NULL;

		/* Set up output pipe */
		close(fd2[0]);
		dup2(fd2[1], fileno(stdout)); /* Should check success code */
		dup2(fd2[1], fileno(stderr));
		close(fd2[1]);
		
		
		if (anAnchor->post_data) { /* post script, read stdin */
		    close(fd1[1]);
		    dup2(fd1[0], fileno(stdin));
		    close(fd1[0]);

		    /* Build environment variables */

		    add_environment_value("REQUEST_METHOD=POST");

		    sprintf(post_len, "CONTENT_LENGTH=%d",
			    strlen(anAnchor->post_data));
		    add_environment_value(post_len);
		} else {
		    close(fileno(stdin));
		}

		/* 
		 * Set up argument line, mainly for <index> scripts
		 */
		if (pgm_args != NULL) {
		    for (cp = pgm_args; *cp != '\0'; cp++) {
			if (*cp == '+') {
			    argv_cnt++;
			}
		    }
		}

		argv = (char**)malloc(argv_cnt * sizeof(char*));
		if (argv == NULL) {
		    outofmem(__FILE__, "LYCgi");
		}
		cur_argv = argv + 1;		/* For argv[0] */
		if (pgm_args != NULL) {		
		    char *cr;

		    /* Data for a get/search form */
		    if (is_www_index) {
			add_environment_value("REQUEST_METHOD=SEARCH");
		    } else {
			add_environment_value("REQUEST_METHOD=GET");
		    }
		    
		    cp = NULL;
		    StrAllocCopy(cp, "QUERY_STRING=");
		    StrAllocCat(cp, pgm_args);
		    add_environment_value(cp);

		    /*
		     * Split up arguments into argv array
		     */
		    cp = pgm_args;
		    cr = cp;
		    while(1) {
			if (*cp == '\0') {
			    *(cur_argv++) = HTUnEscape(cr);
			    break;
			    
			} else if (*cp == '+') {
			    *cp++ = '\0';
			    *(cur_argv++) = HTUnEscape(cr);
			    cr = cp;
			}
			cp++;
		    }
		}
		*cur_argv = NULL;	/* Terminate argv */		
		argv[0] = pgm;

		execve(argv[0], argv, env);
		if (TRACE) {
		    perror("LYNXCGI: execve failed");
		}
		
	    } else {	/* and the Ugly */
		HTAlert(CONNECT_FAILED);
		if (TRACE) {
		    perror("LYNXCGI: fork() failed");
		}
		status = HT_NO_DATA;
		close(fd1[0]);
		close(fd1[1]);
		close(fd2[0]);
		close(fd2[1]);
		status = -1;
	    }

	}
	if (target != NULL) {
	    (*target->isa->_free)(target);
	}
    }
    FREE(pgm);
#else  /* VMS */
	HTStream *target;
	char buf[256];

	target = HTStreamStack(WWW_HTML, 
			       format_out,
			       sink, anAnchor);

	sprintf(buf,"<head>\n<title>Good Advice</title>\n</head>\n<body>\n");
	(*target->isa->put_block)(target, buf, strlen(buf));
	
	sprintf(buf,"<h1>Good Advice</h1>\n");
	(*target->isa->put_block)(target, buf, strlen(buf));

	sprintf(buf, "An excellent http server for VMS is available via <a\n");
	(*target->isa->put_block)(target, buf, strlen(buf));

	sprintf(buf,
	 "href=\"http://kcgl1.eng.ohio-state.edu/www/doc/serverinfo.html\"\n");
	(*target->isa->put_block)(target, buf, strlen(buf));

	sprintf(buf, ">this link</a>.\n");
	(*target->isa->put_block)(target, buf, strlen(buf));

	sprintf(buf,
		"<p>It provides <b>state of the art</b> CGI script support.\n");
	(*target->isa->put_block)(target, buf, strlen(buf));

	sprintf(buf,"</body>\n");
	(*target->isa->put_block)(target, buf, strlen(buf));

	(*target->isa->_free)(target);
	status = HT_LOADED;
#endif /* VMS */
#else /* LYNXCGI_LINKS */
    _statusline(CGI_NOT_COMPILED);
    sleep(MessageSecs);
    status = HT_NOT_LOADED;
#endif /* LYNXCGI_LINKS */
    return(status);
}

#ifdef GLOBALDEF_IS_MACRO
#define _LYCGI_C_GLOBALDEF_1_INIT { "lynxcgi", LYLoadCGI, 0 }
GLOBALDEF (HTProtocol,LYLynxCGI,_LYCGI_C_GLOBALDEF_1_INIT);
#else
GLOBALDEF PUBLIC HTProtocol LYLynxCGI = { "lynxcgi", LYLoadCGI, 0 };
#endif /* GLOBALDEF_IS_MACRO */
