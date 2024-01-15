/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 2
#define YYMINOR 0
#define YYPATCH 20240109

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#undef YYBTYACC
#define YYBTYACC 0
#define YYDEBUGSTR YYPREFIX "debug"
#define YYPREFIX "yy"

#define YYPURE 0

#line 2 "./parsdate.y"

#include <LYLeaks.h>

/*
 *  $LynxId: parsdate.y,v 1.32 2024/01/15 11:09:08 tom Exp $
 *
 *  This module is adapted and extended from tin, to use for LYmktime().
 *
 *  Project   : tin - a Usenet reader
 *  Module    : parsedate.y
 *  Author    : S. Bellovin, R. $alz, J. Berets, P. Eggert
 *  Created   : 1990-08-01
 *  Updated   : 2019-08-28 (by Thomas Dickey, for Lynx)
 *  Notes     : This grammar has 8 shift/reduce conflicts.
 *
 *              Originally written by Steven M. Bellovin <smb@research.att.com>
 *              while at the University of North Carolina at Chapel Hill.
 *              Later tweaked by a couple of people on Usenet.  Completely
 *              overhauled by Rich $alz <rsalz@osf.org> and Jim Berets
 *              <jberets@bbn.com> in August, 1990.
 *
 *              Further revised (removed obsolete constructs and cleaned up
 *              timezone names) in August, 1991, by Rich.
 *              Paul Eggert <eggert@twinsun.com> helped in September 1992.
 *              Roland Rosenfeld added MET DST code in April 1994.
 *
 *  Revision  : 1.13
 *  Copyright : This code is in the public domain and has no copyright.
 */

/* SUPPRESS 530 */ /* Empty body for statement */
/* SUPPRESS 593 on yyerrlab */ /* Label was not used */
/* SUPPRESS 593 on yynewstate */ /* Label was not used */
/* SUPPRESS 595 on yypvt */ /* Automatic variable may be used before set */

#undef alloca			/* conflicting def may be set by yacc */
#include <parsdate.h>

/*
**  Get the number of elements in a fixed-size array, or a pointer just
**  past the end of it.
*/
#define ENDOF(array)	(&array[ARRAY_SIZE(array)])

#ifdef EBCDIC
#define TO_ASCII(c)	TOASCII(c)
#define TO_LOCAL(c)	FROMASCII(c)
#else
#define TO_ASCII(c)	(c)
#define TO_LOCAL(c)	(c)
#endif

#define IS7BIT(x)		((unsigned) TO_ASCII(x) < 128)
#define CTYPE(isXXXXX, c)	(IS7BIT(c) && isXXXXX(((unsigned char)c)))

typedef char *PD_STRING;

extern int date_parse(void);

#define yyparse		date_parse
#define yylex		date_lex
#define yyerror		date_error

#define BAD_TIME	((time_t)-1)
#define isBadTime(n)	((n) != 0 && (((n) == BAD_TIME) || !((n) > 0)))

    /* See the LeapYears table in Convert. */
#define EPOCH		1970
#define END_OF_TIME	2200

    /* Constants for general time calculations. */
#define DST_OFFSET	1
#define SECSPERDAY	(24L * 60L * 60L)
    /* Readability for TABLE stuff. */
#define HOUR(x)		(x * 60)

#define LPAREN		'('
#define RPAREN		')'

/*
**  Daylight-savings mode:  on, off, or not yet known.
*/
typedef enum _DSTMODE {
    DSTon, DSToff, DSTmaybe
} DSTMODE;

/*
**  Meridian:  am, pm, or 24-hour style.
*/
typedef enum _MERIDIAN {
    MERam, MERpm, MER24
} MERIDIAN;

/*
**  Global variables.  We could get rid of most of them by using a yacc
**  union, but this is more efficient.  (This routine predates the
**  yacc %union construct.)
*/
static char *yyInput;
static DSTMODE yyDSTmode;
static int yyHaveDate;
static int yyHaveRel;
static int yyHaveTime;
static time_t yyTimezone;
static time_t yyDay;
static time_t yyHour;
static time_t yyMinutes;
static time_t yyMonth;
static time_t yySeconds;
static time_t yyYear;
static MERIDIAN yyMeridian;
static time_t yyRelMonth;
static time_t yyRelSeconds;

static time_t ToSeconds(time_t, time_t, time_t, MERIDIAN);
static time_t Convert(time_t, time_t, time_t, time_t, time_t, time_t,
		      MERIDIAN, DSTMODE);
static time_t DSTcorrect(time_t, time_t);
static time_t RelativeMonth(time_t, time_t);
static int LookupWord(char *, int);
static int date_lex(void);
static int GetTimeInfo(TIMEINFO * Now);

/*
 * The 'date_error()' function is declared here to work around a defect in
 * bison 1.22, which redefines 'const' further down in this file, making it
 * impossible to put a prototype here, and the function later.  We're using
 * 'const' on the parameter to quiet gcc's -Wwrite-strings warning.
 */
/*ARGSUSED*/
static void date_error(const char GCC_UNUSED *s)
{
    /*NOTREACHED */
}

#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#line 139 "./parsdate.y"
typedef union YYSTYPE {
    time_t		Number;
    enum _MERIDIAN	Meridian;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
#line 172 "y.tab.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

#if !(defined(yylex) || defined(YYSTATE))
int YYLEX_DECL();
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

#define tDAY 257
#define tDAYZONE 258
#define tMERIDIAN 259
#define tMONTH 260
#define tMONTH_UNIT 261
#define tSEC_UNIT 262
#define tSNUMBER 263
#define tUNUMBER 264
#define tZONE 265
#define tDST 266
#define YYERRCODE 256
typedef int YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    4,    4,    4,    4,    4,    4,    5,    5,
    5,    5,    5,    2,    2,    2,    2,    2,    1,    6,
    6,    6,    6,    6,    6,    6,    6,    6,    7,    8,
    8,    8,    8,    3,    3,
};
static const YYINT yylen[] = {                            2,
    0,    2,    1,    2,    1,    1,    2,    1,    2,    4,
    4,    6,    6,    1,    1,    2,    2,    1,    1,    3,
    5,    2,    4,    2,    3,    5,    6,    3,    9,    2,
    2,    2,    2,    0,    1,
};
static const YYINT yydefred[] = {                         1,
    0,    0,    0,    0,    0,    2,    0,    5,    0,    8,
    0,    0,    0,   32,   30,   35,    0,   33,   31,    0,
    0,    0,    9,    0,   19,    0,   18,    4,    7,    0,
    0,    0,   25,   28,    0,    0,   16,   17,    0,    0,
    0,   23,    0,   11,   10,    0,    0,   26,    0,    0,
   21,    0,   27,   13,   12,    0,    0,   29,
};
#if defined(YYDESTRUCT_CALL) || defined(YYSTYPE_TOSTRING)
static const YYINT yystos[] = {                           0,
  268,  257,  260,  263,  264,  272,  273,  274,  275,  276,
  260,   44,  264,  261,  262,  259,  260,  261,  262,  263,
   58,   47,  271,  258,  263,  265,  269,  270,  270,  264,
  264,   44,  264,  263,  264,  264,  266,  269,  264,  260,
   45,  264,   58,  269,  271,   47,   58,  264,  260,  264,
  264,  264,  263,  269,  271,   58,  264,  264,
};
#endif /* YYDESTRUCT_CALL || YYSTYPE_TOSTRING */
static const YYINT yydgoto[] = {                          1,
   27,   28,   23,    6,    7,    8,    9,   10,
};
static const YYINT yysindex[] = {                         0,
 -240,  -41, -256, -227,  -45,    0, -251,    0, -251,    0,
 -254, -249,  -22,    0,    0,    0, -237,    0,    0, -235,
 -228, -226,    0, -236,    0, -224,    0,    0,    0, -223,
  -39, -222,    0,    0,  -58,   -7,    0,    0,  -15, -220,
 -215,    0, -218,    0,    0, -217, -216,    0, -214, -234,
    0,   -8,    0,    0,    0, -213, -212,    0,
};
static const YYINT yyrindex[] = {                         0,
    0,    0,    0,    0,    5,    0,   26,    0,   31,    0,
    0,    0,   11,    0,    0,    0,   37,    0,    0,    0,
    0,    0,    0,   16,    0,   32,    0,    0,    0,    0,
    0,    0,    0,    0,    1,   21,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,
    0,    0,    0,    0,    0,    0,    0,    0,
};
#if YYBTYACC
static const YYINT yycindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};
#endif
static const YYINT yygindex[] = {                         0,
  -17,   44,  -31,    0,    0,    0,    0,    0,
};
#define YYTABLESIZE 300
static const YYINT yytable[] = {                         43,
   34,   22,   12,   45,   34,   41,   24,   13,   38,   30,
   22,   25,   21,   26,   31,   15,    2,   44,   55,    3,
   20,   32,    4,    5,   16,    3,   33,   34,   25,   37,
    6,   14,   54,   14,   15,   35,   24,   36,   25,   46,
   39,   42,   47,   48,   49,   50,   51,   52,   53,   56,
   57,   58,   29,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   16,    0,    0,    0,   25,    0,    0,    0,    0,    0,
    0,    0,    0,   16,   17,   18,   19,   20,   11,    0,
   40,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   34,   34,    0,
   34,   34,   34,    0,   34,   34,    0,   22,   34,   34,
   22,    0,   15,   22,   22,   15,    0,   20,   15,   15,
   20,    0,    3,   20,   20,    3,    0,    6,   14,    3,
    6,   14,    0,   24,    6,   14,   24,    0,    0,   24,
};
static const YYINT yycheck[] = {                         58,
    0,   47,   44,   35,    0,   45,  258,  264,   26,  264,
    0,  263,   58,  265,  264,    0,  257,   35,   50,  260,
    0,   44,  263,  264,  259,    0,  264,  263,  263,  266,
    0,    0,   50,  261,  262,  264,    0,  264,  263,   47,
  264,  264,   58,  264,  260,  264,  264,  264,  263,   58,
  264,  264,    9,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  259,   -1,   -1,   -1,  263,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  259,  260,  261,  262,  263,  260,   -1,
  260,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  257,  258,   -1,
  260,  257,  258,   -1,  264,  265,   -1,  257,  264,  265,
  260,   -1,  257,  263,  264,  260,   -1,  257,  263,  264,
  260,   -1,  257,  263,  264,  260,   -1,  257,  257,  264,
  260,  260,   -1,  257,  264,  264,  260,   -1,   -1,  263,
};
#if YYBTYACC
static const YYINT yyctable[] = {                        -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
};
#endif
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 266
#define YYUNDFTOKEN 277
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"$end",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,"','","'-'",0,"'/'",0,0,0,0,0,0,0,0,0,0,"':'",0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"error","tDAY","tDAYZONE",
"tMERIDIAN","tMONTH","tMONTH_UNIT","tSEC_UNIT","tSNUMBER","tUNUMBER","tZONE",
"tDST","$accept","spec","numzone","zone","o_merid","item","time","date","both",
"rel","illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : spec",
"spec :",
"spec : spec item",
"item : time",
"item : time zone",
"item : date",
"item : both",
"item : both zone",
"item : rel",
"time : tUNUMBER o_merid",
"time : tUNUMBER ':' tUNUMBER o_merid",
"time : tUNUMBER ':' tUNUMBER numzone",
"time : tUNUMBER ':' tUNUMBER ':' tUNUMBER o_merid",
"time : tUNUMBER ':' tUNUMBER ':' tUNUMBER numzone",
"zone : tZONE",
"zone : tDAYZONE",
"zone : tDAYZONE tDST",
"zone : tZONE numzone",
"zone : numzone",
"numzone : tSNUMBER",
"date : tUNUMBER '/' tUNUMBER",
"date : tUNUMBER '/' tUNUMBER '/' tUNUMBER",
"date : tMONTH tUNUMBER",
"date : tMONTH tUNUMBER ',' tUNUMBER",
"date : tUNUMBER tMONTH",
"date : tUNUMBER tMONTH tUNUMBER",
"date : tDAY ',' tUNUMBER tMONTH tUNUMBER",
"date : tDAY ',' tUNUMBER '-' tMONTH tSNUMBER",
"date : tUNUMBER tSNUMBER tSNUMBER",
"both : tDAY tMONTH tUNUMBER tUNUMBER ':' tUNUMBER ':' tUNUMBER tUNUMBER",
"rel : tSNUMBER tSEC_UNIT",
"rel : tUNUMBER tSEC_UNIT",
"rel : tSNUMBER tMONTH_UNIT",
"rel : tUNUMBER tMONTH_UNIT",
"o_merid :",
"o_merid : tMERIDIAN",

};
#endif

#if YYDEBUG
int      yydebug;
#endif

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;
int      yynerrs;

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
YYLTYPE  yyloc; /* position returned by actions */
YYLTYPE  yylloc; /* position from the lexer */
#endif

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
#ifndef YYLLOC_DEFAULT
#define YYLLOC_DEFAULT(loc, rhs, n) \
do \
{ \
    if (n == 0) \
    { \
        (loc).first_line   = YYRHSLOC(rhs, 0).last_line; \
        (loc).first_column = YYRHSLOC(rhs, 0).last_column; \
        (loc).last_line    = YYRHSLOC(rhs, 0).last_line; \
        (loc).last_column  = YYRHSLOC(rhs, 0).last_column; \
    } \
    else \
    { \
        (loc).first_line   = YYRHSLOC(rhs, 1).first_line; \
        (loc).first_column = YYRHSLOC(rhs, 1).first_column; \
        (loc).last_line    = YYRHSLOC(rhs, n).last_line; \
        (loc).last_column  = YYRHSLOC(rhs, n).last_column; \
    } \
} while (0)
#endif /* YYLLOC_DEFAULT */
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
#if YYBTYACC

#ifndef YYLVQUEUEGROWTH
#define YYLVQUEUEGROWTH 32
#endif
#endif /* YYBTYACC */

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#ifndef YYINITSTACKSIZE
#define YYINITSTACKSIZE 200
#endif

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    YYLTYPE  *p_base;
    YYLTYPE  *p_mark;
#endif
} YYSTACKDATA;
#if YYBTYACC

struct YYParseState_s
{
    struct YYParseState_s *save;    /* Previously saved parser state */
    YYSTACKDATA            yystack; /* saved parser stack */
    int                    state;   /* saved parser state */
    int                    errflag; /* saved error recovery status */
    int                    lexeme;  /* saved index of the conflict lexeme in the lexical queue */
    YYINT                  ctry;    /* saved index in yyctable[] for this conflict */
};
typedef struct YYParseState_s YYParseState;
#endif /* YYBTYACC */
/* variables for the parser stack */
static YYSTACKDATA yystack;
#if YYBTYACC

/* Current parser state */
static YYParseState *yyps = 0;

/* yypath != NULL: do the full parse, starting at *yypath parser state. */
static YYParseState *yypath = 0;

/* Base of the lexical value queue */
static YYSTYPE *yylvals = 0;

/* Current position at lexical value queue */
static YYSTYPE *yylvp = 0;

/* End position of lexical value queue */
static YYSTYPE *yylve = 0;

/* The last allocated position at the lexical value queue */
static YYSTYPE *yylvlim = 0;

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
/* Base of the lexical position queue */
static YYLTYPE *yylpsns = 0;

/* Current position at lexical position queue */
static YYLTYPE *yylpp = 0;

/* End position of lexical position queue */
static YYLTYPE *yylpe = 0;

/* The last allocated position at the lexical position queue */
static YYLTYPE *yylplim = 0;
#endif

/* Current position at lexical token queue */
static YYINT  *yylexp = 0;

static YYINT  *yylexemes = 0;
#endif /* YYBTYACC */
#line 361 "./parsdate.y"


/*
**  An entry in the lexical lookup table.
*/
/* *INDENT-OFF* */
typedef struct _TABLE {
    const char *name;
    int		type;
    time_t	value;
} TABLE;

/* Month and day table. */
static const TABLE MonthDayTable[] = {
    { "january",	tMONTH,  1 },
    { "february",	tMONTH,  2 },
    { "march",		tMONTH,  3 },
    { "april",		tMONTH,  4 },
    { "may",		tMONTH,  5 },
    { "june",		tMONTH,  6 },
    { "july",		tMONTH,  7 },
    { "august",		tMONTH,  8 },
    { "september",	tMONTH,  9 },
    { "october",	tMONTH, 10 },
    { "november",	tMONTH, 11 },
    { "december",	tMONTH, 12 },
	/* The value of the day isn't used... */
    { "sunday",		tDAY, 0 },
    { "monday",		tDAY, 0 },
    { "tuesday",	tDAY, 0 },
    { "wednesday",	tDAY, 0 },
    { "thursday",	tDAY, 0 },
    { "friday",		tDAY, 0 },
    { "saturday",	tDAY, 0 },
};

/* Time units table. */
static const TABLE	UnitsTable[] = {
    { "year",		tMONTH_UNIT,	12 },
    { "month",		tMONTH_UNIT,	1 },
    { "week",		tSEC_UNIT,	7 * 24 * 60 * 60 },
    { "day",		tSEC_UNIT,	1 * 24 * 60 * 60 },
    { "hour",		tSEC_UNIT,	60 * 60 },
    { "minute",		tSEC_UNIT,	60 },
    { "min",		tSEC_UNIT,	60 },
    { "second",		tSEC_UNIT,	1 },
    { "sec",		tSEC_UNIT,	1 },
};

/* Timezone table. */
static const TABLE	TimezoneTable[] = {
    { "gmt",	tZONE,     HOUR( 0) },	/* Greenwich Mean */
    { "ut",	tZONE,     HOUR( 0) },	/* Universal */
    { "utc",	tZONE,     HOUR( 0) },	/* Universal Coordinated */
    { "cut",	tZONE,     HOUR( 0) },	/* Coordinated Universal */
    { "z",	tZONE,     HOUR( 0) },	/* Greenwich Mean */
    { "wet",	tZONE,     HOUR( 0) },	/* Western European */
    { "bst",	tDAYZONE,  HOUR( 0) },	/* British Summer */
    { "nst",	tZONE,     HOUR(3)+30 }, /* Newfoundland Standard */
    { "ndt",	tDAYZONE,  HOUR(3)+30 }, /* Newfoundland Daylight */
    { "ast",	tZONE,     HOUR( 4) },	/* Atlantic Standard */
    { "adt",	tDAYZONE,  HOUR( 4) },	/* Atlantic Daylight */
    { "est",	tZONE,     HOUR( 5) },	/* Eastern Standard */
    { "edt",	tDAYZONE,  HOUR( 5) },	/* Eastern Daylight */
    { "cst",	tZONE,     HOUR( 6) },	/* Central Standard */
    { "cdt",	tDAYZONE,  HOUR( 6) },	/* Central Daylight */
    { "mst",	tZONE,     HOUR( 7) },	/* Mountain Standard */
    { "mdt",	tDAYZONE,  HOUR( 7) },	/* Mountain Daylight */
    { "pst",	tZONE,     HOUR( 8) },	/* Pacific Standard */
    { "pdt",	tDAYZONE,  HOUR( 8) },	/* Pacific Daylight */
    { "yst",	tZONE,     HOUR( 9) },	/* Yukon Standard */
    { "ydt",	tDAYZONE,  HOUR( 9) },	/* Yukon Daylight */
    { "akst",	tZONE,     HOUR( 9) },	/* Alaska Standard */
    { "akdt",	tDAYZONE,  HOUR( 9) },	/* Alaska Daylight */
    { "hst",	tZONE,     HOUR(10) },	/* Hawaii Standard */
    { "hast",	tZONE,     HOUR(10) },	/* Hawaii-Aleutian Standard */
    { "hadt",	tDAYZONE,  HOUR(10) },	/* Hawaii-Aleutian Daylight */
    { "ces",	tDAYZONE,  -HOUR(1) },	/* Central European Summer */
    { "cest",	tDAYZONE,  -HOUR(1) },	/* Central European Summer */
    { "mez",	tZONE,     -HOUR(1) },	/* Middle European */
    { "mezt",	tDAYZONE,  -HOUR(1) },	/* Middle European Summer */
    { "cet",	tZONE,     -HOUR(1) },	/* Central European */
    { "met",	tZONE,     -HOUR(1) },	/* Middle European */
/* Additional aliases for MET / MET DST *************************************/
    { "mez",    tZONE,     -HOUR(1) },  /* Middle European */
    { "mewt",   tZONE,     -HOUR(1) },  /* Middle European Winter */
    { "mest",   tDAYZONE,  -HOUR(1) },  /* Middle European Summer */
    { "mes",    tDAYZONE,  -HOUR(1) },  /* Middle European Summer */
    { "mesz",   tDAYZONE,  -HOUR(1) },  /* Middle European Summer */
    { "msz",    tDAYZONE,  -HOUR(1) },  /* Middle European Summer */
    { "metdst", tDAYZONE,  -HOUR(1) },  /* Middle European Summer */
/****************************************************************************/
    { "eet",	tZONE,     -HOUR(2) },	/* Eastern Europe */
    { "msk",	tZONE,     -HOUR(3) },	/* Moscow Winter */
    { "msd",	tDAYZONE,  -HOUR(3) },	/* Moscow Summer */
    { "wast",	tZONE,     -HOUR(8) },	/* West Australian Standard */
    { "wadt",	tDAYZONE,  -HOUR(8) },	/* West Australian Daylight */
    { "hkt",	tZONE,     -HOUR(8) },	/* Hong Kong */
    { "cct",	tZONE,     -HOUR(8) },	/* China Coast */
    { "jst",	tZONE,     -HOUR(9) },	/* Japan Standard */
    { "kst",	tZONE,     -HOUR(9) },	/* Korean Standard */
    { "kdt",	tZONE,     -HOUR(9) },	/* Korean Daylight */
    { "cast",	tZONE,     -(HOUR(9)+30) }, /* Central Australian Standard */
    { "cadt",	tDAYZONE,  -(HOUR(9)+30) }, /* Central Australian Daylight */
    { "east",	tZONE,     -HOUR(10) },	/* Eastern Australian Standard */
    { "eadt",	tDAYZONE,  -HOUR(10) },	/* Eastern Australian Daylight */
    { "nzst",	tZONE,     -HOUR(12) },	/* New Zealand Standard */
    { "nzdt",	tDAYZONE,  -HOUR(12) },	/* New Zealand Daylight */

    /* For completeness we include the following entries. */
#if	0

    /* Duplicate names.  Either they conflict with a zone listed above
     * (which is either more likely to be seen or just been in circulation
     * longer), or they conflict with another zone in this section and
     * we could not reasonably choose one over the other. */
    { "fst",	tZONE,     HOUR( 2) },	/* Fernando De Noronha Standard */
    { "fdt",	tDAYZONE,  HOUR( 2) },	/* Fernando De Noronha Daylight */
    { "bst",	tZONE,     HOUR( 3) },	/* Brazil Standard */
    { "est",	tZONE,     HOUR( 3) },	/* Eastern Standard (Brazil) */
    { "edt",	tDAYZONE,  HOUR( 3) },	/* Eastern Daylight (Brazil) */
    { "wst",	tZONE,     HOUR( 4) },	/* Western Standard (Brazil) */
    { "wdt",	tDAYZONE,  HOUR( 4) },	/* Western Daylight (Brazil) */
    { "cst",	tZONE,     HOUR( 5) },	/* Chile Standard */
    { "cdt",	tDAYZONE,  HOUR( 5) },	/* Chile Daylight */
    { "ast",	tZONE,     HOUR( 5) },	/* Acre Standard */
    { "adt",	tDAYZONE,  HOUR( 5) },	/* Acre Daylight */
    { "cst",	tZONE,     HOUR( 5) },	/* Cuba Standard */
    { "cdt",	tDAYZONE,  HOUR( 5) },	/* Cuba Daylight */
    { "est",	tZONE,     HOUR( 6) },	/* Easter Island Standard */
    { "edt",	tDAYZONE,  HOUR( 6) },	/* Easter Island Daylight */
    { "sst",	tZONE,     HOUR(11) },	/* Samoa Standard */
    { "ist",	tZONE,     -HOUR(2) },	/* Israel Standard */
    { "idt",	tDAYZONE,  -HOUR(2) },	/* Israel Daylight */
    { "idt",	tDAYZONE,  -(HOUR(3)+30) }, /* Iran Daylight */
    { "ist",	tZONE,     -(HOUR(3)+30) }, /* Iran Standard */
    { "cst",	 tZONE,     -HOUR(8) },	/* China Standard */
    { "cdt",	 tDAYZONE,  -HOUR(8) },	/* China Daylight */
    { "sst",	 tZONE,     -HOUR(8) },	/* Singapore Standard */

    /* Dubious (e.g., not in Olson's TIMEZONE package) or obsolete. */
    { "gst",	tZONE,     HOUR( 3) },	/* Greenland Standard */
    { "wat",	tZONE,     -HOUR(1) },	/* West Africa */
    { "at",	tZONE,     HOUR( 2) },	/* Azores */
    { "gst",	tZONE,     -HOUR(10) },	/* Guam Standard */
    { "nft",	tZONE,     HOUR(3)+30 }, /* Newfoundland */
    { "idlw",	tZONE,     HOUR(12) },	/* International Date Line West */
    { "mewt",	tZONE,     -HOUR(1) },	/* Middle European Winter */
    { "mest",	tDAYZONE,  -HOUR(1) },	/* Middle European Summer */
    { "swt",	tZONE,     -HOUR(1) },	/* Swedish Winter */
    { "sst",	tDAYZONE,  -HOUR(1) },	/* Swedish Summer */
    { "fwt",	tZONE,     -HOUR(1) },	/* French Winter */
    { "fst",	tDAYZONE,  -HOUR(1) },	/* French Summer */
    { "bt",	tZONE,     -HOUR(3) },	/* Baghdad */
    { "it",	tZONE,     -(HOUR(3)+30) }, /* Iran */
    { "zp4",	tZONE,     -HOUR(4) },	/* USSR Zone 3 */
    { "zp5",	tZONE,     -HOUR(5) },	/* USSR Zone 4 */
    { "ist",	tZONE,     -(HOUR(5)+30) }, /* Indian Standard */
    { "zp6",	tZONE,     -HOUR(6) },	/* USSR Zone 5 */
    { "nst",	tZONE,     -HOUR(7) },	/* North Sumatra */
    { "sst",	tZONE,     -HOUR(7) },	/* South Sumatra */
    { "jt",	tZONE,     -(HOUR(7)+30) }, /* Java (3pm in Cronusland!) */
    { "nzt",	tZONE,     -HOUR(12) },	/* New Zealand */
    { "idle",	tZONE,     -HOUR(12) },	/* International Date Line East */
    { "cat",	tZONE,     HOUR(10) },	/* -- expired 1967 */
    { "nt",	tZONE,     HOUR(11) },	/* -- expired 1967 */
    { "ahst",	tZONE,     HOUR(10) },	/* -- expired 1983 */
    { "hdt",	tDAYZONE,  HOUR(10) },	/* -- expired 1986 */
#endif	/* 0 */
};
/* *INDENT-ON* */

static time_t ToSeconds(time_t Hours, time_t Minutes, time_t Seconds, MERIDIAN Meridian)
{
    if (isBadTime(Minutes) || Minutes > 59 || isBadTime(Seconds) || Seconds > 61)
	return BAD_TIME;
    if (Meridian == MER24) {
	if (isBadTime(Hours) || Hours > 23)
	    return BAD_TIME;
    } else {
	if (Hours < 1 || Hours > 12)
	    return BAD_TIME;
	if (Hours == 12)
	    Hours = 0;
	if (Meridian == MERpm)
	    Hours += 12;
    }
    return (Hours * 60L + Minutes) * 60L + Seconds;
}

static time_t Convert(time_t Month, time_t Day, time_t Year, time_t Hours,
		      time_t Minutes, time_t Seconds, MERIDIAN Meridian,
		      DSTMODE dst)
{
    static const int DaysNormal[13] =
    {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    static const int DaysLeap[13] =
    {
	0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    /* *INDENT-OFF* */
    static const int LeapYears[] =
    {
	1972, 1976, 1980, 1984, 1988, 1992, 1996,
	2000, 2004, 2008, 2012, 2016, 2020, 2024, 2028, 2032, 2036,
	2040, 2044, 2048, 2052, 2056, 2060, 2064, 2068, 2072, 2076,
	2080, 2084, 2088, 2092, 2096, /***/ 2104, 2108, 2112, 2116,
	2120, 2124, 2128, 2132, 2136, 2140, 2144, 2148, 2152, 2156,
	2160, 2164, 2168, 2172, 2176, 2180, 2184, 2188, 2192, 2196,
    };
    /* *INDENT-ON* */

    const int *yp;
    const int *mp;
    int i;
    time_t Julian;
    time_t tod;

    if (isBadTime(Year))
	Year = -Year;
    if (Year < 70)
	Year += 2000;
    if (Year < 100)
	Year += 1900;
    if (Year < EPOCH)
	Year += 100;
    for (mp = DaysNormal, yp = LeapYears; yp < ENDOF(LeapYears); yp++)
	if (Year == *yp) {
	    mp = DaysLeap;
	    break;
	}
    if (Year < EPOCH || Year > END_OF_TIME
	|| Month < 1 || Month > 12
    /* NOSTRICT */
    /* conversion from long may lose accuracy */
	|| Day < 1 || Day > mp[(int) Month]) {
	return BAD_TIME;
    }

    Julian = Day - 1 + (Year - EPOCH) * 365;
    for (yp = LeapYears; yp < ENDOF(LeapYears); yp++, Julian++) {
	if (Year <= *yp)
	    break;
    }
    for (i = 1; i < Month; i++)
	Julian += *++mp;
    Julian *= SECSPERDAY;
    Julian += yyTimezone * 60L;
    tod = ToSeconds(Hours, Minutes, Seconds, Meridian);
    if (isBadTime(tod)) {
	return BAD_TIME;
    }
    Julian += tod;
    tod = Julian;
    if (dst == DSTon) {
	Julian -= DST_OFFSET * 60 * 60;
    } else if (dst == DSTmaybe) {
	struct tm *tm = localtime(&tod);

	if (tm != NULL) {
	    if (tm->tm_isdst)
		Julian -= DST_OFFSET * 60 * 60;
	} else {
	    Julian = BAD_TIME;
	}
    }
    return Julian;
}

static time_t DSTcorrect(time_t Start, time_t Future)
{
    struct tm *tm;
    time_t StartDay;
    time_t FutureDay;

    if ((tm = localtime(&Start)) == NULL)
	return BAD_TIME;
    StartDay = (tm->tm_hour + 1) % 24;

    if ((tm = localtime(&Future)) == NULL)
	return BAD_TIME;
    FutureDay = (tm->tm_hour + 1) % 24;

    return (Future - Start) + (StartDay - FutureDay) * DST_OFFSET * 60 * 60;
}

static time_t RelativeMonth(time_t Start, time_t RelMonth)
{
    struct tm *tm;
    time_t Month;
    time_t Year;

    if ((tm = localtime(&Start)) == NULL)
	return BAD_TIME;

    Month = 12 * tm->tm_year + tm->tm_mon + RelMonth;
    Year = Month / 12 + 1900;
    Month = Month % 12 + 1;
    return DSTcorrect(Start,
		      Convert(Month, (time_t) tm->tm_mday, Year,
			      (time_t) tm->tm_hour, (time_t) tm->tm_min,
			      (time_t) tm->tm_sec,
			      MER24, DSTmaybe));
}

static int LookupWord(char *buff,
		      int length)
{
    char *p;
    const char *q;
    const TABLE *tp;
    int c;

    p = buff;
    c = p[0];

    /* See if we have an abbreviation for a month. */
    if (length == 3 || (length == 4 && p[3] == '.')) {
	for (tp = MonthDayTable; tp < ENDOF(MonthDayTable); tp++) {
	    q = tp->name;
	    if (c == q[0] && p[1] == q[1] && p[2] == q[2]) {
		yylval.Number = tp->value;
		return tp->type;
	    }
	}
    } else {
	for (tp = MonthDayTable; tp < ENDOF(MonthDayTable); tp++) {
	    if (c == tp->name[0] && strcmp(p, tp->name) == 0) {
		yylval.Number = tp->value;
		return tp->type;
	    }
	}
    }

    /* Try for a timezone. */
    for (tp = TimezoneTable; tp < ENDOF(TimezoneTable); tp++) {
	if (c == tp->name[0] && p[1] == tp->name[1]
	    && strcmp(p, tp->name) == 0) {
	    yylval.Number = tp->value;
	    return tp->type;
	}
    }

    if (strcmp(buff, "dst") == 0)
	return tDST;

    /* Try the units table. */
    for (tp = UnitsTable; tp < ENDOF(UnitsTable); tp++) {
	if (c == tp->name[0] && strcmp(p, tp->name) == 0) {
	    yylval.Number = tp->value;
	    return tp->type;
	}
    }

    /* Strip off any plural and try the units table again. */
    if (--length > 0 && p[length] == 's') {
	p[length] = '\0';
	for (tp = UnitsTable; tp < ENDOF(UnitsTable); tp++) {
	    if (c == tp->name[0] && strcmp(p, tp->name) == 0) {
		p[length] = 's';
		yylval.Number = tp->value;
		return tp->type;
	    }
	}
	p[length] = 's';
    }
    length++;

    /* Drop out any periods. */
    for (p = buff, q = (PD_STRING) buff; *q; q++) {
	if (*q != '.')
	    *p++ = *q;
    }
    *p = '\0';

    /* Try the meridians. */
    if (buff[1] == 'm' && buff[2] == '\0') {
	if (buff[0] == 'a') {
	    yylval.Meridian = MERam;
	    return tMERIDIAN;
	}
	if (buff[0] == 'p') {
	    yylval.Meridian = MERpm;
	    return tMERIDIAN;
	}
    }

    /* If we saw any periods, try the timezones again. */
    if (p - buff != length) {
	c = buff[0];
	for (p = buff, tp = TimezoneTable; tp < ENDOF(TimezoneTable); tp++) {
	    if (c == tp->name[0] && p[1] == tp->name[1]
		&& strcmp(p, tp->name) == 0) {
		yylval.Number = tp->value;
		return tp->type;
	    }
	}
    }

    /* Unknown word -- assume GMT timezone. */
    yylval.Number = 0;
    return tZONE;
}

/*
 * This returns characters as-is (the ones that are not part of some token),
 * and codes greater than 256 (the token values).
 *
 * yacc generates tables that may use the character value.  In particular,
 * byacc's yycheck[] table contains integer values for the expected codes from
 * this function, which (unless byacc is run locally) are ASCII codes.
 *
 * The TO_LOCAL() function assumes its input is in ASCII, and the output is
 * whatever native encoding is used on the machine, e.g., EBCDIC.
 *
 * The TO_ASCII() function is the inverse of TO_LOCAL().
 */
static int date_lex(void)
{
    int c;
    char *p;
    char buff[20];
    int sign;
    int i;
    int nesting;

    /* Get first character after the whitespace. */
    for (;;) {
	while (CTYPE(isspace, *yyInput))
	    yyInput++;
	c = *yyInput;

	/* Ignore RFC 822 comments, typically time zone names. */
	if (c != LPAREN)
	    break;
	for (nesting = 1;
	     (c = *++yyInput) != RPAREN || --nesting;
	    ) {
	    if (c == LPAREN) {
		nesting++;
	    } else if (!IS7BIT(c) || c == '\0' || c == '\r'
		       || (c == '\\'
			   && ((c = *++yyInput) == '\0'
			       || !IS7BIT(c)))) {
		/* Lexical error: bad comment. */
		return '?';
	    }
	}
	yyInput++;
    }

    /* A number? */
    if (CTYPE(isdigit, c) || c == '-' || c == '+') {
	if (c == '-' || c == '+') {
	    sign = c == '-' ? -1 : 1;
	    yyInput++;
	    if (!CTYPE(isdigit, *yyInput)) {
		/* Return the isolated plus or minus sign. */
		--yyInput;
		return *yyInput++;
	    }
	} else {
	    sign = 0;
	}
	for (p = buff;
	     (c = *yyInput++) != '\0' && CTYPE(isdigit, c);
	    ) {
	    if (p < &buff[sizeof buff - 1])
		*p++ = (char) c;
	}
	*p = '\0';
	i = atoi(buff);

	yyInput--;
	yylval.Number = sign < 0 ? -i : i;
	return sign ? tSNUMBER : tUNUMBER;
    }

    /* A word? */
    if (CTYPE(isalpha, c)) {
	for (p = buff;
	     (c = *yyInput++) == '.' || CTYPE(isalpha, c);
	    ) {
	    if (p < &buff[sizeof buff - 1])
		*p++ = (char) (CTYPE(isupper, c) ? tolower(c) : c);
	}
	*p = '\0';
	yyInput--;
	return LookupWord(buff, (int) (p - buff));
    }

    return *yyInput++;
}

static int GetTimeInfo(TIMEINFO * Now)
{
    static time_t LastTime;
    static long LastTzone;
    struct tm *tm;

#if	defined(HAVE_GETTIMEOFDAY)
    struct timeval tv;
#endif /* defined(HAVE_GETTIMEOFDAY) */
#if	defined(DONT_HAVE_TM_GMTOFF)
    struct tm local;
    struct tm gmt;
#endif /* !defined(DONT_HAVE_TM_GMTOFF) */

    /* Get the basic time. */
    memset(Now, 0, sizeof(TIMEINFO));
#if defined(HAVE_GETTIMEOFDAY)
    if (gettimeofday(&tv, (struct timezone *) NULL) == -1)
	return -1;
    Now->time = tv.tv_sec;
    Now->usec = tv.tv_usec;
#else
    /* Can't check for -1 since that might be a time, I guess. */
    (void) time(&Now->time);
    Now->usec = 0;
#endif /* defined(HAVE_GETTIMEOFDAY) */

    /* Now get the timezone if it's been an hour since the last time. */
    if (Now->time - LastTime > 60 * 60) {
	LastTime = Now->time;
	if ((tm = localtime(&Now->time)) == NULL)
	    return -1;
#if	defined(DONT_HAVE_TM_GMTOFF)
	/* To get the timezone, compare localtime with GMT. */
	local = *tm;
	if ((tm = gmtime(&Now->time)) == NULL)
	    return -1;
	gmt = *tm;

	/* Assume we are never more than 24 hours away. */
	LastTzone = gmt.tm_yday - local.tm_yday;
	if (LastTzone > 1)
	    LastTzone = -24;
	else if (LastTzone < -1)
	    LastTzone = 24;
	else
	    LastTzone *= 24;

	/* Scale in the hours and minutes; ignore seconds. */
	LastTzone += gmt.tm_hour - local.tm_hour;
	LastTzone *= 60;
	LastTzone += gmt.tm_min - local.tm_min;
#else
	LastTzone = (0 - tm->tm_gmtoff) / 60;
#endif /* defined(DONT_HAVE_TM_GMTOFF) */
    }
    Now->tzone = LastTzone;
    return 0;
}

#if defined(YYBYACC) && defined(YYPURE) && defined(LY_FIND_LEAKS)
#undef YYPURE
#define YYPURE 1
static void yyfreestack(YYSTACKDATA *);
static void parsedate_leaks(void)
{
    yyfreestack(&yystack);
}
#endif

time_t parsedate(char *p,
		 TIMEINFO * now)
{
    struct tm *tm;
    TIMEINFO ti;
    time_t Start;

#if defined(YYBYACC) && defined(YYPURE) && defined(LY_FIND_LEAKS)
    static int initialized;

    if (!initialized) {
	initialized = 1;
	atexit(parsedate_leaks);
    }
#endif

    yyInput = p;
    if (now == NULL) {
	now = &ti;
	(void) GetTimeInfo(&ti);
    }

    if ((tm = localtime(&now->time)) == NULL)
	return BAD_TIME;

    /* *INDENT-EQLS* */
    yyYear       = tm->tm_year + 1900;
    yyMonth      = tm->tm_mon + 1;
    yyDay        = tm->tm_mday;
    yyTimezone   = now->tzone;
    if (tm->tm_isdst)		/* Correct timezone offset for DST */
	yyTimezone   += DST_OFFSET * 60;
    yyDSTmode    = DSTmaybe;
    yyHour       = 0;
    yyMinutes    = 0;
    yySeconds    = 0;
    yyMeridian   = MER24;
    yyRelSeconds = 0;
    yyRelMonth   = 0;
    yyHaveDate   = 0;
    yyHaveRel    = 0;
    yyHaveTime   = 0;

    if (date_parse() || yyHaveTime > 1 || yyHaveDate > 1)
	return BAD_TIME;

    if (yyHaveDate || yyHaveTime) {
	Start = Convert(yyMonth, yyDay, yyYear, yyHour, yyMinutes, yySeconds,
			yyMeridian, yyDSTmode);
	if (isBadTime(Start))
	    return BAD_TIME;
    } else {
	Start = now->time;
	if (!yyHaveRel)
	    Start -= (tm->tm_hour * 60L + tm->tm_min) * 60L + tm->tm_sec;
    }

    Start += yyRelSeconds;
    if (yyRelMonth)
	Start += RelativeMonth(Start, yyRelMonth);

    /* Have to do *something* with a legitimate -1 so it's distinguishable
     * from the error return value.  (Alternately could set errno on error.) */
    return (Start == BAD_TIME) ? 0 : Start;
}
#line 1203 "y.tab.c"

/* For use in generated program */
#define yydepth (int)(yystack.s_mark - yystack.s_base)
#if YYBTYACC
#define yytrial (yyps->save)
#endif /* YYBTYACC */

#if YYDEBUG
#include <stdio.h>	/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    YYLTYPE *newps;
#endif

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    newps = (YYLTYPE *)realloc(data->p_base, newsize * sizeof(*newps));
    if (newps == 0)
        return YYENOMEM;

    data->p_base = newps;
    data->p_mark = newps + i;
#endif

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;

#if YYDEBUG
    if (yydebug)
        fprintf(stderr, "%sdebug: stack size increased to %d\n", YYPREFIX, newsize);
#endif
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    free(data->p_base);
#endif
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif /* YYPURE || defined(YY_NO_LEAKS) */
#if YYBTYACC

static YYParseState *
yyNewState(unsigned size)
{
    YYParseState *p = (YYParseState *) malloc(sizeof(YYParseState));
    if (p == NULL) return NULL;

    p->yystack.stacksize = size;
    if (size == 0)
    {
        p->yystack.s_base = NULL;
        p->yystack.l_base = NULL;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        p->yystack.p_base = NULL;
#endif
        return p;
    }
    p->yystack.s_base    = (YYINT *) malloc(size * sizeof(YYINT));
    if (p->yystack.s_base == NULL) return NULL;
    p->yystack.l_base    = (YYSTYPE *) malloc(size * sizeof(YYSTYPE));
    if (p->yystack.l_base == NULL) return NULL;
    memset(p->yystack.l_base, 0, size * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    p->yystack.p_base    = (YYLTYPE *) malloc(size * sizeof(YYLTYPE));
    if (p->yystack.p_base == NULL) return NULL;
    memset(p->yystack.p_base, 0, size * sizeof(YYLTYPE));
#endif

    return p;
}

static void
yyFreeState(YYParseState *p)
{
    yyfreestack(&p->yystack);
    free(p);
}
#endif /* YYBTYACC */

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab
#if YYBTYACC
#define YYVALID        do { if (yyps->save)            goto yyvalid; } while(0)
#define YYVALID_NESTED do { if (yyps->save && \
                                yyps->save->save == 0) goto yyvalid; } while(0)
#endif /* YYBTYACC */

int
YYPARSE_DECL()
{
    int yym, yyn, yystate, yyresult;
#if YYBTYACC
    int yynewerrflag;
    YYParseState *yyerrctx = NULL;
#endif /* YYBTYACC */
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    YYLTYPE  yyerror_loc_range[3]; /* position of error start/end (0 unused) */
#endif
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
    if (yydebug)
        fprintf(stderr, "%sdebug[<# of symbols on state stack>]\n", YYPREFIX);
#endif
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    memset(yyerror_loc_range, 0, sizeof(yyerror_loc_range));
#endif

#if YYBTYACC
    yyps = yyNewState(0); if (yyps == 0) goto yyenomem;
    yyps->save = 0;
#endif /* YYBTYACC */
    yym = 0;
    /* yyn is set below */
    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark = yystack.p_base;
#endif
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
#if YYBTYACC
        do {
        if (yylvp < yylve)
        {
            /* we're currently re-reading tokens */
            yylval = *yylvp++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yylloc = *yylpp++;
#endif
            yychar = *yylexp++;
            break;
        }
        if (yyps->save)
        {
            /* in trial mode; save scanner results for future parse attempts */
            if (yylvp == yylvlim)
            {   /* Enlarge lexical value queue */
                size_t p = (size_t) (yylvp - yylvals);
                size_t s = (size_t) (yylvlim - yylvals);

                s += YYLVQUEUEGROWTH;
                if ((yylexemes = (YYINT *)realloc(yylexemes, s * sizeof(YYINT))) == NULL) goto yyenomem;
                if ((yylvals   = (YYSTYPE *)realloc(yylvals, s * sizeof(YYSTYPE))) == NULL) goto yyenomem;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                if ((yylpsns   = (YYLTYPE *)realloc(yylpsns, s * sizeof(YYLTYPE))) == NULL) goto yyenomem;
#endif
                yylvp   = yylve = yylvals + p;
                yylvlim = yylvals + s;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylpp   = yylpe = yylpsns + p;
                yylplim = yylpsns + s;
#endif
                yylexp  = yylexemes + p;
            }
            *yylexp = (YYINT) YYLEX;
            *yylvp++ = yylval;
            yylve++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            *yylpp++ = yylloc;
            yylpe++;
#endif
            yychar = *yylexp++;
            break;
        }
        /* normal operation, no conflict encountered */
#endif /* YYBTYACC */
        yychar = YYLEX;
#if YYBTYACC
        } while (0);
#endif /* YYBTYACC */
        if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            fprintf(stderr, "%s[%d]: state %d, reading token %d (%s)",
                            YYDEBUGSTR, yydepth, yystate, yychar, yys);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
            if (!yytrial)
#endif /* YYBTYACC */
                fprintf(stderr, " <%s>", YYSTYPE_TOSTRING(yychar, yylval));
#endif
            fputc('\n', stderr);
        }
#endif
    }
#if YYBTYACC

    /* Do we have a conflict? */
    if (((yyn = yycindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
        yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
        YYINT ctry;

        if (yypath)
        {
            YYParseState *save;
#if YYDEBUG
            if (yydebug)
                fprintf(stderr, "%s[%d]: CONFLICT in state %d: following successful trial parse\n",
                                YYDEBUGSTR, yydepth, yystate);
#endif
            /* Switch to the next conflict context */
            save = yypath;
            yypath = save->save;
            save->save = NULL;
            ctry = save->ctry;
            if (save->state != yystate) YYABORT;
            yyFreeState(save);

        }
        else
        {

            /* Unresolved conflict - start/continue trial parse */
            YYParseState *save;
#if YYDEBUG
            if (yydebug)
            {
                fprintf(stderr, "%s[%d]: CONFLICT in state %d. ", YYDEBUGSTR, yydepth, yystate);
                if (yyps->save)
                    fputs("ALREADY in conflict, continuing trial parse.\n", stderr);
                else
                    fputs("Starting trial parse.\n", stderr);
            }
#endif
            save                  = yyNewState((unsigned)(yystack.s_mark - yystack.s_base + 1));
            if (save == NULL) goto yyenomem;
            save->save            = yyps->save;
            save->state           = yystate;
            save->errflag         = yyerrflag;
            save->yystack.s_mark  = save->yystack.s_base + (yystack.s_mark - yystack.s_base);
            memcpy (save->yystack.s_base, yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
            save->yystack.l_mark  = save->yystack.l_base + (yystack.l_mark - yystack.l_base);
            memcpy (save->yystack.l_base, yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            save->yystack.p_mark  = save->yystack.p_base + (yystack.p_mark - yystack.p_base);
            memcpy (save->yystack.p_base, yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
            ctry                  = yytable[yyn];
            if (yyctable[ctry] == -1)
            {
#if YYDEBUG
                if (yydebug && yychar >= YYEOF)
                    fprintf(stderr, "%s[%d]: backtracking 1 token\n", YYDEBUGSTR, yydepth);
#endif
                ctry++;
            }
            save->ctry = ctry;
            if (yyps->save == NULL)
            {
                /* If this is a first conflict in the stack, start saving lexemes */
                if (!yylexemes)
                {
                    yylexemes = (YYINT *) malloc((YYLVQUEUEGROWTH) * sizeof(YYINT));
                    if (yylexemes == NULL) goto yyenomem;
                    yylvals   = (YYSTYPE *) malloc((YYLVQUEUEGROWTH) * sizeof(YYSTYPE));
                    if (yylvals == NULL) goto yyenomem;
                    yylvlim   = yylvals + YYLVQUEUEGROWTH;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    yylpsns   = (YYLTYPE *) malloc((YYLVQUEUEGROWTH) * sizeof(YYLTYPE));
                    if (yylpsns == NULL) goto yyenomem;
                    yylplim   = yylpsns + YYLVQUEUEGROWTH;
#endif
                }
                if (yylvp == yylve)
                {
                    yylvp  = yylve = yylvals;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    yylpp  = yylpe = yylpsns;
#endif
                    yylexp = yylexemes;
                    if (yychar >= YYEOF)
                    {
                        *yylve++ = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                        *yylpe++ = yylloc;
#endif
                        *yylexp  = (YYINT) yychar;
                        yychar   = YYEMPTY;
                    }
                }
            }
            if (yychar >= YYEOF)
            {
                yylvp--;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylpp--;
#endif
                yylexp--;
                yychar = YYEMPTY;
            }
            save->lexeme = (int) (yylvp - yylvals);
            yyps->save   = save;
        }
        if (yytable[yyn] == ctry)
        {
#if YYDEBUG
            if (yydebug)
                fprintf(stderr, "%s[%d]: state %d, shifting to state %d\n",
                                YYDEBUGSTR, yydepth, yystate, yyctable[ctry]);
#endif
            if (yychar < 0)
            {
                yylvp++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylpp++;
#endif
                yylexp++;
            }
            if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
                goto yyoverflow;
            yystate = yyctable[ctry];
            *++yystack.s_mark = (YYINT) yystate;
            *++yystack.l_mark = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            *++yystack.p_mark = yylloc;
#endif
            yychar  = YYEMPTY;
            if (yyerrflag > 0) --yyerrflag;
            goto yyloop;
        }
        else
        {
            yyn = yyctable[ctry];
            goto yyreduce;
        }
    } /* End of code dealing with conflicts */
#endif /* YYBTYACC */
    if (((yyn = yysindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
#if YYDEBUG
        if (yydebug)
            fprintf(stderr, "%s[%d]: state %d, shifting to state %d\n",
                            YYDEBUGSTR, yydepth, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        *++yystack.p_mark = yylloc;
#endif
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if (((yyn = yyrindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag != 0) goto yyinrecovery;
#if YYBTYACC

    yynewerrflag = 1;
    goto yyerrhandler;
    goto yyerrlab; /* redundant goto avoids 'unused label' warning */

yyerrlab:
    /* explicit YYERROR from an action -- pop the rhs of the rule reduced
     * before looking for error recovery */
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark -= yym;
#endif

    yynewerrflag = 0;
yyerrhandler:
    while (yyps->save)
    {
        int ctry;
        YYParseState *save = yyps->save;
#if YYDEBUG
        if (yydebug)
            fprintf(stderr, "%s[%d]: ERROR in state %d, CONFLICT BACKTRACKING to state %d, %d tokens\n",
                            YYDEBUGSTR, yydepth, yystate, yyps->save->state,
                    (int)(yylvp - yylvals - yyps->save->lexeme));
#endif
        /* Memorize most forward-looking error state in case it's really an error. */
        if (yyerrctx == NULL || yyerrctx->lexeme < yylvp - yylvals)
        {
            /* Free old saved error context state */
            if (yyerrctx) yyFreeState(yyerrctx);
            /* Create and fill out new saved error context state */
            yyerrctx                 = yyNewState((unsigned)(yystack.s_mark - yystack.s_base + 1));
            if (yyerrctx == NULL) goto yyenomem;
            yyerrctx->save           = yyps->save;
            yyerrctx->state          = yystate;
            yyerrctx->errflag        = yyerrflag;
            yyerrctx->yystack.s_mark = yyerrctx->yystack.s_base + (yystack.s_mark - yystack.s_base);
            memcpy (yyerrctx->yystack.s_base, yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
            yyerrctx->yystack.l_mark = yyerrctx->yystack.l_base + (yystack.l_mark - yystack.l_base);
            memcpy (yyerrctx->yystack.l_base, yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yyerrctx->yystack.p_mark = yyerrctx->yystack.p_base + (yystack.p_mark - yystack.p_base);
            memcpy (yyerrctx->yystack.p_base, yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
            yyerrctx->lexeme         = (int) (yylvp - yylvals);
        }
        yylvp          = yylvals   + save->lexeme;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        yylpp          = yylpsns   + save->lexeme;
#endif
        yylexp         = yylexemes + save->lexeme;
        yychar         = YYEMPTY;
        yystack.s_mark = yystack.s_base + (save->yystack.s_mark - save->yystack.s_base);
        memcpy (yystack.s_base, save->yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
        yystack.l_mark = yystack.l_base + (save->yystack.l_mark - save->yystack.l_base);
        memcpy (yystack.l_base, save->yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        yystack.p_mark = yystack.p_base + (save->yystack.p_mark - save->yystack.p_base);
        memcpy (yystack.p_base, save->yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
        ctry           = ++save->ctry;
        yystate        = save->state;
        /* We tried shift, try reduce now */
        if ((yyn = yyctable[ctry]) >= 0) goto yyreduce;
        yyps->save     = save->save;
        save->save     = NULL;
        yyFreeState(save);

        /* Nothing left on the stack -- error */
        if (!yyps->save)
        {
#if YYDEBUG
            if (yydebug)
                fprintf(stderr, "%sdebug[%d,trial]: trial parse FAILED, entering ERROR mode\n",
                                YYPREFIX, yydepth);
#endif
            /* Restore state as it was in the most forward-advanced error */
            yylvp          = yylvals   + yyerrctx->lexeme;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yylpp          = yylpsns   + yyerrctx->lexeme;
#endif
            yylexp         = yylexemes + yyerrctx->lexeme;
            yychar         = yylexp[-1];
            yylval         = yylvp[-1];
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yylloc         = yylpp[-1];
#endif
            yystack.s_mark = yystack.s_base + (yyerrctx->yystack.s_mark - yyerrctx->yystack.s_base);
            memcpy (yystack.s_base, yyerrctx->yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
            yystack.l_mark = yystack.l_base + (yyerrctx->yystack.l_mark - yyerrctx->yystack.l_base);
            memcpy (yystack.l_base, yyerrctx->yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yystack.p_mark = yystack.p_base + (yyerrctx->yystack.p_mark - yyerrctx->yystack.p_base);
            memcpy (yystack.p_base, yyerrctx->yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
            yystate        = yyerrctx->state;
            yyFreeState(yyerrctx);
            yyerrctx       = NULL;
        }
        yynewerrflag = 1;
    }
    if (yynewerrflag == 0) goto yyinrecovery;
#endif /* YYBTYACC */

    YYERROR_CALL("syntax error");
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yyerror_loc_range[1] = yylloc; /* lookahead position is error start position */
#endif

#if !YYBTYACC
    goto yyerrlab; /* redundant goto avoids 'unused label' warning */
yyerrlab:
#endif
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if (((yyn = yysindex[*yystack.s_mark]) != 0) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    fprintf(stderr, "%s[%d]: state %d, error recovery shifting to state %d\n",
                                    YYDEBUGSTR, yydepth, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                /* lookahead position is error end position */
                yyerror_loc_range[2] = yylloc;
                YYLLOC_DEFAULT(yyloc, yyerror_loc_range, 2); /* position of error span */
                *++yystack.p_mark = yyloc;
#endif
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    fprintf(stderr, "%s[%d]: error recovery discarding state %d\n",
                                    YYDEBUGSTR, yydepth, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                /* the current TOS position is the error start position */
                yyerror_loc_range[1] = *yystack.p_mark;
#endif
#if defined(YYDESTRUCT_CALL)
#if YYBTYACC
                if (!yytrial)
#endif /* YYBTYACC */
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    YYDESTRUCT_CALL("error: discarding state",
                                    yystos[*yystack.s_mark], yystack.l_mark, yystack.p_mark);
#else
                    YYDESTRUCT_CALL("error: discarding state",
                                    yystos[*yystack.s_mark], yystack.l_mark);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
#endif /* defined(YYDESTRUCT_CALL) */
                --yystack.s_mark;
                --yystack.l_mark;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                --yystack.p_mark;
#endif
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            fprintf(stderr, "%s[%d]: state %d, error recovery discarding token %d (%s)\n",
                            YYDEBUGSTR, yydepth, yystate, yychar, yys);
        }
#endif
#if defined(YYDESTRUCT_CALL)
#if YYBTYACC
        if (!yytrial)
#endif /* YYBTYACC */
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            YYDESTRUCT_CALL("error: discarding token", yychar, &yylval, &yylloc);
#else
            YYDESTRUCT_CALL("error: discarding token", yychar, &yylval);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
#endif /* defined(YYDESTRUCT_CALL) */
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
    yym = yylen[yyn];
#if YYDEBUG
    if (yydebug)
    {
        fprintf(stderr, "%s[%d]: state %d, reducing by rule %d (%s)",
                        YYDEBUGSTR, yydepth, yystate, yyn, yyrule[yyn]);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
        if (!yytrial)
#endif /* YYBTYACC */
            if (yym > 0)
            {
                int i;
                fputc('<', stderr);
                for (i = yym; i > 0; i--)
                {
                    if (i != yym) fputs(", ", stderr);
                    fputs(YYSTYPE_TOSTRING(yystos[yystack.s_mark[1-i]],
                                           yystack.l_mark[1-i]), stderr);
                }
                fputc('>', stderr);
            }
#endif
        fputc('\n', stderr);
    }
#endif
    if (yym > 0)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)

    /* Perform position reduction */
    memset(&yyloc, 0, sizeof(yyloc));
#if YYBTYACC
    if (!yytrial)
#endif /* YYBTYACC */
    {
        YYLLOC_DEFAULT(yyloc, &yystack.p_mark[-yym], yym);
        /* just in case YYERROR is invoked within the action, save
           the start of the rhs as the error start position */
        yyerror_loc_range[1] = yystack.p_mark[1-yym];
    }
#endif

    switch (yyn)
    {
case 3:
#line 157 "./parsdate.y"
	{
	    yyHaveTime++;
#if	defined(lint)
	    /* I am compulsive about lint natterings... */
	    if (yyHaveTime == -1) {
		YYERROR;
	    }
#endif	/* defined(lint) */
	}
#line 1884 "y.tab.c"
break;
case 4:
#line 166 "./parsdate.y"
	{
	    yyHaveTime++;
	    yyTimezone = yystack.l_mark[0].Number;
	}
#line 1892 "y.tab.c"
break;
case 5:
#line 170 "./parsdate.y"
	{
	    yyHaveDate++;
	}
#line 1899 "y.tab.c"
break;
case 6:
#line 173 "./parsdate.y"
	{
	    yyHaveDate++;
	    yyHaveTime++;
	}
#line 1907 "y.tab.c"
break;
case 7:
#line 177 "./parsdate.y"
	{
	    yyHaveDate++;
	    yyHaveTime++;
	    yyTimezone = yystack.l_mark[0].Number;
	}
#line 1916 "y.tab.c"
break;
case 8:
#line 182 "./parsdate.y"
	{
	    yyHaveRel = 1;
	}
#line 1923 "y.tab.c"
break;
case 9:
#line 187 "./parsdate.y"
	{
	    if (yystack.l_mark[-1].Number < 100) {
		yyHour = yystack.l_mark[-1].Number;
		yyMinutes = 0;
	    }
	    else {
		yyHour = yystack.l_mark[-1].Number / 100;
		yyMinutes = yystack.l_mark[-1].Number % 100;
	    }
	    yySeconds = 0;
	    yyMeridian = yystack.l_mark[0].Meridian;
	}
#line 1939 "y.tab.c"
break;
case 10:
#line 199 "./parsdate.y"
	{
	    yyHour = yystack.l_mark[-3].Number;
	    yyMinutes = yystack.l_mark[-1].Number;
	    yySeconds = 0;
	    yyMeridian = yystack.l_mark[0].Meridian;
	}
#line 1949 "y.tab.c"
break;
case 11:
#line 205 "./parsdate.y"
	{
	    yyHour = yystack.l_mark[-3].Number;
	    yyMinutes = yystack.l_mark[-1].Number;
	    yyTimezone = yystack.l_mark[0].Number;
	    yyMeridian = MER24;
	    yyDSTmode = DSToff;
	}
#line 1960 "y.tab.c"
break;
case 12:
#line 212 "./parsdate.y"
	{
	    yyHour = yystack.l_mark[-5].Number;
	    yyMinutes = yystack.l_mark[-3].Number;
	    yySeconds = yystack.l_mark[-1].Number;
	    yyMeridian = yystack.l_mark[0].Meridian;
	}
#line 1970 "y.tab.c"
break;
case 13:
#line 218 "./parsdate.y"
	{
	    yyHour = yystack.l_mark[-5].Number;
	    yyMinutes = yystack.l_mark[-3].Number;
	    yySeconds = yystack.l_mark[-1].Number;
	    yyTimezone = yystack.l_mark[0].Number;
	    yyMeridian = MER24;
	    yyDSTmode = DSToff;
	}
#line 1982 "y.tab.c"
break;
case 14:
#line 228 "./parsdate.y"
	{
	    yyval.Number = yystack.l_mark[0].Number;
	    yyDSTmode = DSToff;
	}
#line 1990 "y.tab.c"
break;
case 15:
#line 232 "./parsdate.y"
	{
	    yyval.Number = yystack.l_mark[0].Number;
	    yyDSTmode = DSTon;
	}
#line 1998 "y.tab.c"
break;
case 16:
#line 236 "./parsdate.y"
	{
	    yyTimezone = yystack.l_mark[-1].Number;
	    yyDSTmode = DSTon;
	}
#line 2006 "y.tab.c"
break;
case 17:
#line 240 "./parsdate.y"
	{
	    /* Only allow "GMT+300" and "GMT-0800" */
	    if (yystack.l_mark[-1].Number != 0) {
		YYABORT;
	    }
	    yyval.Number = yystack.l_mark[0].Number;
	    yyDSTmode = DSToff;
	}
#line 2018 "y.tab.c"
break;
case 18:
#line 248 "./parsdate.y"
	{
	    yyval.Number = yystack.l_mark[0].Number;
	    yyDSTmode = DSToff;
	}
#line 2026 "y.tab.c"
break;
case 19:
#line 254 "./parsdate.y"
	{
	    int	i;

	    /* Unix and GMT and numeric timezones -- a little confusing. */
	    if ((long)yystack.l_mark[0].Number < 0) {
		/* Don't work with negative modulus. */
		yystack.l_mark[0].Number = -(int)yystack.l_mark[0].Number;
		if (yystack.l_mark[0].Number > 9999 || (i = (int) (yystack.l_mark[0].Number % 100)) >= 60) {
			YYABORT;
		}
		yyval.Number = (yystack.l_mark[0].Number / 100) * 60 + i;
	    }
	    else {
		if (yystack.l_mark[0].Number > 9999 || (i = (int) (yystack.l_mark[0].Number % 100)) >= 60) {
			YYABORT;
		}
		yyval.Number = -((yystack.l_mark[0].Number / 100) * 60 + i);
	    }
	}
#line 2049 "y.tab.c"
break;
case 20:
#line 275 "./parsdate.y"
	{
	    yyMonth = yystack.l_mark[-2].Number;
	    yyDay = yystack.l_mark[0].Number;
	}
#line 2057 "y.tab.c"
break;
case 21:
#line 279 "./parsdate.y"
	{
	    if (yystack.l_mark[-4].Number > 100) {
		yyYear = yystack.l_mark[-4].Number;
		yyMonth = yystack.l_mark[-2].Number;
		yyDay = yystack.l_mark[0].Number;
	    }
	    else {
		yyMonth = yystack.l_mark[-4].Number;
		yyDay = yystack.l_mark[-2].Number;
		yyYear = yystack.l_mark[0].Number;
	    }
	}
#line 2073 "y.tab.c"
break;
case 22:
#line 291 "./parsdate.y"
	{
	    yyMonth = yystack.l_mark[-1].Number;
	    yyDay = yystack.l_mark[0].Number;
	}
#line 2081 "y.tab.c"
break;
case 23:
#line 295 "./parsdate.y"
	{
	    yyMonth = yystack.l_mark[-3].Number;
	    yyDay = yystack.l_mark[-2].Number;
	    yyYear = yystack.l_mark[0].Number;
	}
#line 2090 "y.tab.c"
break;
case 24:
#line 300 "./parsdate.y"
	{
	    yyDay = yystack.l_mark[-1].Number;
	    yyMonth = yystack.l_mark[0].Number;
	}
#line 2098 "y.tab.c"
break;
case 25:
#line 304 "./parsdate.y"
	{
	    yyDay = yystack.l_mark[-2].Number;
	    yyMonth = yystack.l_mark[-1].Number;
	    yyYear = yystack.l_mark[0].Number;
	}
#line 2107 "y.tab.c"
break;
case 26:
#line 309 "./parsdate.y"
	{
	    yyDay = yystack.l_mark[-2].Number;
	    yyMonth = yystack.l_mark[-1].Number;
	    yyYear = yystack.l_mark[0].Number;
	}
#line 2116 "y.tab.c"
break;
case 27:
#line 314 "./parsdate.y"
	{
	    yyDay = yystack.l_mark[-3].Number;
	    yyMonth = yystack.l_mark[-1].Number;
	    yyYear = -yystack.l_mark[0].Number;
	}
#line 2125 "y.tab.c"
break;
case 28:
#line 319 "./parsdate.y"
	{
	    yyDay = yystack.l_mark[-2].Number;
	    yyMonth = -yystack.l_mark[-1].Number;
	    yyYear = -yystack.l_mark[0].Number;
	    yyDSTmode = DSToff;	/* assume midnight if no time given */
	    yyTimezone = 0;	/* Lynx assumes GMT for this format */
	}
#line 2136 "y.tab.c"
break;
case 29:
#line 328 "./parsdate.y"
	{
	    yyMonth = yystack.l_mark[-7].Number;
	    yyDay = yystack.l_mark[-6].Number;
	    yyYear = yystack.l_mark[0].Number;
	    yyHour = yystack.l_mark[-5].Number;
	    yyMinutes = yystack.l_mark[-3].Number;
	    yySeconds = yystack.l_mark[-1].Number;
	}
#line 2148 "y.tab.c"
break;
case 30:
#line 338 "./parsdate.y"
	{
	    yyRelSeconds += yystack.l_mark[-1].Number * yystack.l_mark[0].Number;
	}
#line 2155 "y.tab.c"
break;
case 31:
#line 341 "./parsdate.y"
	{
	    yyRelSeconds += yystack.l_mark[-1].Number * yystack.l_mark[0].Number;
	}
#line 2162 "y.tab.c"
break;
case 32:
#line 344 "./parsdate.y"
	{
	    yyRelMonth += yystack.l_mark[-1].Number * yystack.l_mark[0].Number;
	}
#line 2169 "y.tab.c"
break;
case 33:
#line 347 "./parsdate.y"
	{
	    yyRelMonth += yystack.l_mark[-1].Number * yystack.l_mark[0].Number;
	}
#line 2176 "y.tab.c"
break;
case 34:
#line 352 "./parsdate.y"
	{
	    yyval.Meridian = MER24;
	}
#line 2183 "y.tab.c"
break;
case 35:
#line 355 "./parsdate.y"
	{
	    yyval.Meridian = yystack.l_mark[0].Meridian;
	}
#line 2190 "y.tab.c"
break;
#line 2192 "y.tab.c"
    default:
        break;
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark -= yym;
#endif
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
        {
            fprintf(stderr, "%s[%d]: after reduction, ", YYDEBUGSTR, yydepth);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
            if (!yytrial)
#endif /* YYBTYACC */
                fprintf(stderr, "result is <%s>, ", YYSTYPE_TOSTRING(yystos[YYFINAL], yyval));
#endif
            fprintf(stderr, "shifting from state 0 to final state %d\n", YYFINAL);
        }
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        *++yystack.p_mark = yyloc;
#endif
        if (yychar < 0)
        {
#if YYBTYACC
            do {
            if (yylvp < yylve)
            {
                /* we're currently re-reading tokens */
                yylval = *yylvp++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylloc = *yylpp++;
#endif
                yychar = *yylexp++;
                break;
            }
            if (yyps->save)
            {
                /* in trial mode; save scanner results for future parse attempts */
                if (yylvp == yylvlim)
                {   /* Enlarge lexical value queue */
                    size_t p = (size_t) (yylvp - yylvals);
                    size_t s = (size_t) (yylvlim - yylvals);

                    s += YYLVQUEUEGROWTH;
                    if ((yylexemes = (YYINT *)realloc(yylexemes, s * sizeof(YYINT))) == NULL)
                        goto yyenomem;
                    if ((yylvals   = (YYSTYPE *)realloc(yylvals, s * sizeof(YYSTYPE))) == NULL)
                        goto yyenomem;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    if ((yylpsns   = (YYLTYPE *)realloc(yylpsns, s * sizeof(YYLTYPE))) == NULL)
                        goto yyenomem;
#endif
                    yylvp   = yylve = yylvals + p;
                    yylvlim = yylvals + s;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    yylpp   = yylpe = yylpsns + p;
                    yylplim = yylpsns + s;
#endif
                    yylexp  = yylexemes + p;
                }
                *yylexp = (YYINT) YYLEX;
                *yylvp++ = yylval;
                yylve++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                *yylpp++ = yylloc;
                yylpe++;
#endif
                yychar = *yylexp++;
                break;
            }
            /* normal operation, no conflict encountered */
#endif /* YYBTYACC */
            yychar = YYLEX;
#if YYBTYACC
            } while (0);
#endif /* YYBTYACC */
            if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
                fprintf(stderr, "%s[%d]: state %d, reading token %d (%s)\n",
                                YYDEBUGSTR, yydepth, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if (((yyn = yygindex[yym]) != 0) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
    {
        fprintf(stderr, "%s[%d]: after reduction, ", YYDEBUGSTR, yydepth);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
        if (!yytrial)
#endif /* YYBTYACC */
            fprintf(stderr, "result is <%s>, ", YYSTYPE_TOSTRING(yystos[yystate], yyval));
#endif
        fprintf(stderr, "shifting from state %d to state %d\n", *yystack.s_mark, yystate);
    }
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    *++yystack.p_mark = yyloc;
#endif
    goto yyloop;
#if YYBTYACC

    /* Reduction declares that this path is valid. Set yypath and do a full parse */
yyvalid:
    if (yypath) YYABORT;
    while (yyps->save)
    {
        YYParseState *save = yyps->save;
        yyps->save = save->save;
        save->save = yypath;
        yypath = save;
    }
#if YYDEBUG
    if (yydebug)
        fprintf(stderr, "%s[%d]: state %d, CONFLICT trial successful, backtracking to state %d, %d tokens\n",
                        YYDEBUGSTR, yydepth, yystate, yypath->state, (int)(yylvp - yylvals - yypath->lexeme));
#endif
    if (yyerrctx)
    {
        yyFreeState(yyerrctx);
        yyerrctx = NULL;
    }
    yylvp          = yylvals + yypath->lexeme;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yylpp          = yylpsns + yypath->lexeme;
#endif
    yylexp         = yylexemes + yypath->lexeme;
    yychar         = YYEMPTY;
    yystack.s_mark = yystack.s_base + (yypath->yystack.s_mark - yypath->yystack.s_base);
    memcpy (yystack.s_base, yypath->yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
    yystack.l_mark = yystack.l_base + (yypath->yystack.l_mark - yypath->yystack.l_base);
    memcpy (yystack.l_base, yypath->yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark = yystack.p_base + (yypath->yystack.p_mark - yypath->yystack.p_base);
    memcpy (yystack.p_base, yypath->yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
    yystate        = yypath->state;
    goto yyloop;
#endif /* YYBTYACC */

yyoverflow:
    YYERROR_CALL("yacc stack overflow");
#if YYBTYACC
    goto yyabort_nomem;
yyenomem:
    YYERROR_CALL("memory exhausted");
yyabort_nomem:
#endif /* YYBTYACC */
    yyresult = 2;
    goto yyreturn;

yyabort:
    yyresult = 1;
    goto yyreturn;

yyaccept:
#if YYBTYACC
    if (yyps->save) goto yyvalid;
#endif /* YYBTYACC */
    yyresult = 0;

yyreturn:
#if defined(YYDESTRUCT_CALL)
    if (yychar != YYEOF && yychar != YYEMPTY)
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        YYDESTRUCT_CALL("cleanup: discarding token", yychar, &yylval, &yylloc);
#else
        YYDESTRUCT_CALL("cleanup: discarding token", yychar, &yylval);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */

    {
        YYSTYPE *pv;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        YYLTYPE *pp;

        for (pv = yystack.l_base, pp = yystack.p_base; pv <= yystack.l_mark; ++pv, ++pp)
             YYDESTRUCT_CALL("cleanup: discarding state",
                             yystos[*(yystack.s_base + (pv - yystack.l_base))], pv, pp);
#else
        for (pv = yystack.l_base; pv <= yystack.l_mark; ++pv)
             YYDESTRUCT_CALL("cleanup: discarding state",
                             yystos[*(yystack.s_base + (pv - yystack.l_base))], pv);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
    }
#endif /* defined(YYDESTRUCT_CALL) */

#if YYBTYACC
    if (yyerrctx)
    {
        yyFreeState(yyerrctx);
        yyerrctx = NULL;
    }
    while (yyps)
    {
        YYParseState *save = yyps;
        yyps = save->save;
        save->save = NULL;
        yyFreeState(save);
    }
    while (yypath)
    {
        YYParseState *save = yypath;
        yypath = save->save;
        save->save = NULL;
        yyFreeState(save);
    }
#endif /* YYBTYACC */
    yyfreestack(&yystack);
    return (yyresult);
}
