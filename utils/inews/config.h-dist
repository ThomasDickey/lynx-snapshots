
/**************************** NN CONFIGURATION ***************************
 *
 *      Configuration file for nn.
 *
 *      You must edit this file to reflect your local configuration
 *      and environment.
 *
 *      Before editing this file, read the licence terms in the README
 *      file and the installation guidelines in the INSTALLATION file.
 *
 *      (c) Copyright 1990, Kim F. Storm.  All rights reserved.
 */

#define RELEASE         "6.4"

#include <stdio.h>
#include <ctype.h>


/*********************** NETWORK DEPENDENT DEFINITIONS **********************
 *
 *      Define NETWORK_DATABASE if you share the database through NFS on
 *      a network with different, non-compatible machines, e.g. SUNs and
 *      VAXen, or SUN-3 and SUN-4, or if you are using different compilers
 *      on the same architecture.
 *
 *      In a homogenous network, you can leave it undefined for higher
 *      performance (no data conversion is needed).
 */

/* #define NETWORK_DATABASE     /* */


/********************************** NNTP *********************************
 *
 *      Define NNTP to enable nntp support.  If you are not using NNTP,
 *      just leave the following NNTP_* definitions as they are - they
 *      will be ignored anyway.
 *
 *      With NNTP, the nnmaster still maintains a local database of
 *      all article headers for fast access (and because NNTP does not
 *      support nn - yet), while the articles are fetched from the
 *      nntp server when they are read or saved.
 *
 *      You may still share this database through NFS locally (see the
 *      description of NETWORK_DATABASE above) if you don't want to
 *      have separate nn databases on all your local systems.
 *
 *      Consult the file NNTP for further information on the use of NNTP.
 */

/* #define NNTP                 /* */

/*
 *      Define NNTP_SERVER to the name of a file containing the name of the
 *      nntp server.
 *
 *      It is vital that both the nnmaster and all nn users on a machine
 *      uses the same nntp server, because the nn database is synchronized
 *      with a specific news active file.
 *
 *      If the file name does not start with a slash, it is relative to
 *      LIB_DIRECTORY defined below.
 *      NOTE: If you plan to use the included inews, it MUST be a full pathname
 */

#define NNTP_SERVER     "/usr/lib/nntp_server"

/*
 *      Define NNTP_POST if you want nn to reject attempts to post via
 *      NNTP to a server, that disallows postings.
 *
 *      You should define this, if you use the NNTP based mini-inews for
 *      postings from NNTP clients.  If you use another mechanism, that
 *      does not involve NNTP, you should leave it undefined.
 */

#define NNTP_POST             /* */

/*
 *      NNTP's mini-inews seems to require that messages contain a complete
 *      header with Message-ID, Path, and Date fields which the normal inews
 *      generates itself.  If your mini-inews requires these headers to
 *      be present, define NNTP_MINI_INEWS_HEADER below.
 */

#define NNTP_MINI_INEWS_HEADER  /* uses "broken" mini-inews */

/*
 *      Define NNTP_PATH_HOSTNAME to force a specific hostname into the
 *      Path: header generated when NNTP_MINI_INEWS_HEADER is defined.
 *      This is useful for multi-machine sites with one mail/news gateway.
 *
 *      If the string starts with a '/' it is taken as the name of a file
 *      from which the outgoing hostname should be read (at runtime).
 */

/* #define NNTP_PATH_HOSTNAME   "puthostnamehere"       /* */


/***************** OPERATING SYSTEM DEPENDENT DEFINITIONS *******************
 *
 *      Include the appropriate s- file for your system below.
 *
 *      If a file does not exist for your system, you can use
 *      conf/s-template.h as a starting point for writing you own.
 */

#include "s-sys5.h"

/*
 *      Define DEFAULT_PAGER as the initial value of the 'pager' variable.
 *      nnadmin pipes shell command output though this command.
 */

#define DEFAULT_PAGER           "pg -n -s"      /* system V */
/* #define DEFAULT_PAGER        "more"                  /* bsd */

/*
 *      DEFAULT_PRINTER is the initial value of the 'printer' variable.
 *      nn's :print command pipes text into this command.
 */

#define DEFAULT_PRINTER         "lp -s"         /* System V */
/* #define DEFAULT_PRINTER      "lpr -p -JNEWS" /* bsd */

/*
 *      Define RESIZING to make nn understand dynamic window-resizing.
 *      (It uses the TIOCGWINSZ ioctl found on most 4.3BSD systems)
 */

/* #define RESIZING             /* */


/********************** MACHINE DEPENDENT DEFINITIONS **********************
 *
 *      Include the appropriate m- file for your system below.
 *
 *      If a file does not exist for your system, you can use
 *      conf/m-template.h as a starting point for writing you own.
 */

#include "m-m680x0.h"


/***************************** OWNERSHIP ***************************
 *
 *      Specify owner and group for installed files and programs.
 *
 *      The nnmaster will run suid/sgid to this owner and group.
 *
 *      The only requirements are that the ownership allows the
 *      nnmaster to READ the news related files and directories, and
 *      the ordinary users to read the database and execute the nn*
 *      programs.
 *
 *      Common choices are:  (news, news)  and   (your uid, your gid)
 */

#define OWNER   "news"
#define GROUP   "news"


/**************************** LOCALIZATION ****************************
 *
 *      Specify where programs and files are installed.
 *
 *      BIN_DIRECTORY    - the location of the user programs (mandatory)
 *
 *      LIB_DIRECTORY    - the location of auxiliary programs and files.
 *                         (mandatory UNLESS ALL of the following are defined).
 *
 *      MASTER_DIRECTORY - the location of the master program (on server)
 *                         (= LIB_DIRECTORY if undefined)
 *
 *      CLIENT_DIRECTORY - the location of auxiliary programs (on clients)
 *                         (= LIB_DIRECTORY if undefined)
 *
 *      HELP_DIRECTORY   - the location of help files, online manual, etc.
 *                         (= CLIENT_DIRECTORY/help if undefined)
 *
 *      CACHE_DIRECTORY  - if NNTP is used, nn uses this central directory
 *                         to store working copies of articles on the local
 *                         system.  If not defined, it stores the articles
 *                         in each user's ~/.nn directory.
 *
 *      TMP_DIRECTORY    - temporary file storage.  Overriden by $TMPDIR.
 *                         (= /usr/tmp if undefined).
 *
 *      LOG_FILE         - the location of nn's log file.
 *                         (= LIB_DIRECTORY/Log if undefined).
 */

#define BIN_DIRECTORY   "/usr/local/bin"
#define LIB_DIRECTORY   "/usr/local/lib/nn"


/**************************** DATABASE LOCATION **************************
 *
 *      Specify where the nn database should be installed.
 *
 *      If none of the following symbols are defined, the database will
 *      be contained in the NEWS_DIRECTORY in a separate .nn directory for
 *      master files and in files named .nnx and .nnd in each group's
 *      spool directory.  To use this scheme, the OWNER specified above
 *      must have write permission on the news spool directories.
 *
 *      If you access news via NNTP, you will probably always have to
 *      give the database directory explicitly through DB_DIRECTORY
 *      (and DB_DATA_DIRECTORY), since the normal news spool directories
 *      are probably not available on the local system.
 *      The exception may be if nnmaster runs directly on the nntp server.
 *
 *      To change the default behaviour, you can define the following
 *      symbols:
 *
 *      DB_DIRECTORY       - the directory containing the master files.
 *
 *      DB_DATA_DIRECTORY  - the directory containing the per-group files
 *                           (default is DB_DIRECTORY/DATA if undefined).
 *
 *      DB_LONG_NAMES      - use group's name rather than number when
 *                           building file names in DB_DATA_DIRECTORY.
 *           (The file system must support long file names!!)
 */

#define DB_DIRECTORY    "/usr/spool/nn"


/*************************** NEWS TRANSPORT **************************
 *
 *      Specify the location of your news programs and files
 *      You only need to specify these if you are not
 *      satisfied with the default settings.
 *
 *      NEWS_DIRECTORY          - The news spool directory.
 *                                Default: /usr/spool/news
 *
 *      NEWS_LIB_DIRECTORY      - The news lib directory.
 *                                Default: /usr/lib/news
 *
 *      INEWS_PATH              - The location of the inews program.
 *                                Default: NEWS_LIB_DIR/inews
 *
 *      RMGROUP_PATH            - The location of the rmgroup program.
 *                                Default: NEWS_LIB_DIR/{rm,del}group
 */

#define NEWS_DIRECTORY          "/usr/spool/news"       /* */
#define NEWS_LIB_DIRECTORY      "/usr/lib/news"         /* */

/* #define INEWS_PATH           "/usr/lib/news/inews"   /* */


/*************************** MAIL INTERFACE *************************
 *
 *      Specify a mailer that accepts a letter WITH a header IN THE TEXT.
 *
 *      A program named 'recmail' program is normally delivered with
 *      the Bnews system, or you can use sendmail -t if you have it.
 *
 *      The contrib/ directory contains two programs which you might
 *      be able to use with a little tweaking.
 */

#define REC_MAIL        "/usr/lib/news/recmail" /* non-sendmail */
/* #define REC_MAIL     "/usr/lib/sendmail -t"  /* sendmail */


/*
 *      Define HAVE_ROUTING if your mailer understands domain based
 *      adresses (...@...) and performs the necessary rerouting (e.g.
 *      Sendmail or Smail).
 *
 *      Otherwise, nn will provide a simple routing facility using
 *      routing information specified in the file LIB_DIRECTORY/routes.
 */

#define HAVE_ROUTING                    /* */

/*
 *      If HAVE_ROUTING is NOT defined, nn needs to know the name of
 *      your host.  To obtain the host name it will use either of the
 *      'uname' or 'gethostname' system calls as specified in the s-
 *      file included above.
 *
 *      If neither 'uname' nor 'gethostname' is available, you must
 *      define HOSTNAME to be the name of your host.  Otherwise, leave
 *      it undefined (it will not be used anyway).
 */

/* #define HOSTNAME     "myhost"        /* Not used if HAVE_ROUTING */

/*
 *      Define APPEND_SIGNATURE if you want nn to ask users to append
 *      ~/.signature to mail messages (reply/forward/mail).
 *
 *      If the mailer defined in REC_MAIL automatically includes .signature
 *      you should not define this (it will fool people to include it twice).
 *
 *      I think 'recmail' includes .signature, but 'sendmail -t' doesn't.
 */

/* #define APPEND_SIGNATURE             /* */

/*
 *      BUG_REPORT_ADDRESS is the initial value of the bug-report-address
 *      variable which is used by the :bug command to report bugs in
 *      the nn software.
 */

#define BUG_REPORT_ADDRESS      "nn-bugs@dkuug.dk"


/*************************** DOCUMENTATION ***************************
 *
 *      Specify directories for the user and system manuals
 *
 *      Adapt this to your local standards; the manuals will be named
 *              $(MAN_DIR)/program.$(MAN_SECTION)
 *
 *      USER_MAN        - nn, nntidy, nngrep, etc.
 *      SYS_MAN         - nnadmin
 *      DAEMON_MAN      - nnmaster
 */

#define USER_MAN_DIR    "/usr/man/man1"
#define USER_MAN_SECTION        "1"

#define SYS_MAN_DIR     "/usr/man/man1"
#define SYS_MAN_SECTION         "1m"

#define DAEMON_MAN_DIR  "/usr/man/man8"
#define DAEMON_MAN_SECTION      "8"


/************************** LOCAL POLICY *****************************
 *
 *      Define STATISTICS if you want to keep a record of how much
 *      time the users spend on news reading.
 *
 *      Sessions shorter than the specified number of minutes are not
 *      recorded (don't clutter up the log file).
 *
 *      Usage statistics is entered into the $LOG_FILE with code U
 */

/* #define STATISTICS   5 /* minutes */

/*
 *      Define ACCOUNTING if you want to keep accumulated accounting
 *      based on the statistics in a separate 'acct' file.  In this
 *      case, the accounting figures will be secret, and not be
 *      written to the Log file.  And the users will not be able to
 *      "decrease" their own account.
 *
 *      See account.c for optional cost calculation parameters.
 */

/* #define ACCOUNTING           /* */

/*
 *      Define AUTHORIZE if you want to restrict the use of nn to
 *      certain users or certain periods of the day.  Define both
 *      this and ACCOUNTING if you want to impose a usage quota
 *
 *      See account.c for implementing various access policies.
 */

/* #define AUTHORIZE    /* */

/*
 *      Default folder directory
 */

#define FOLDER_DIRECTORY        "~/News"

/*
 *      Max length of authors name (in "edited" format).
 *      Also size of "Name" field on the article menus.
 *      You may want to increase this if your terminals are wider than
 *      80 columns.
 */

#define NAME_LENGTH             16


/************************ CONFIGURATION COMPLETED ************************/

#include "global.h"

