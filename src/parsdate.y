%{

#include <LYLeaks.h>

/*
 *  $LynxId: parsdate.y,v 1.21 2018/04/01 22:21:45 tom Exp $
 *
 *  This module is adapted and extended from tin, to use for LYmktime().
 *
 *  Project   : tin - a Usenet reader
 *  Module    : parsedate.y
 *  Author    : S. Bellovin, R. $alz, J. Berets, P. Eggert
 *  Created   : 1990-08-01
 *  Updated   : 2008-06-30 (by Thomas Dickey, for Lynx)
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

    /* See the LeapYears table in Convert. */
#define EPOCH		1970
#define END_OF_TIME	2038

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

%}

%union {
    time_t		Number;
    enum _MERIDIAN	Meridian;
}

%token	tDAY tDAYZONE tMERIDIAN tMONTH tMONTH_UNIT tSEC_UNIT tSNUMBER
%token	tUNUMBER tZONE tDST

%type	<Number>	tDAYZONE tMONTH tMONTH_UNIT tSEC_UNIT
%type	<Number>	tSNUMBER tUNUMBER tZONE numzone zone
%type	<Meridian>	tMERIDIAN o_merid

%%

spec	: /* NULL */
	| spec item
	;

item	: time {
	    yyHaveTime++;
#if	defined(lint)
	    /* I am compulsive about lint natterings... */
	    if (yyHaveTime == -1) {
		YYERROR;
	    }
#endif	/* defined(lint) */
	}
	| time zone {
	    yyHaveTime++;
	    yyTimezone = $2;
	}
	| date {
	    yyHaveDate++;
	}
	| both {
	    yyHaveDate++;
	    yyHaveTime++;
	}
	| both zone {
	    yyHaveDate++;
	    yyHaveTime++;
	    yyTimezone = $2;
	}
	| rel {
	    yyHaveRel = 1;
	}
	;

time	: tUNUMBER o_merid {
	    if ($1 < 100) {
		yyHour = $1;
		yyMinutes = 0;
	    }
	    else {
		yyHour = $1 / 100;
		yyMinutes = $1 % 100;
	    }
	    yySeconds = 0;
	    yyMeridian = $2;
	}
	| tUNUMBER ':' tUNUMBER o_merid {
	    yyHour = $1;
	    yyMinutes = $3;
	    yySeconds = 0;
	    yyMeridian = $4;
	}
	| tUNUMBER ':' tUNUMBER numzone {
	    yyHour = $1;
	    yyMinutes = $3;
	    yyTimezone = $4;
	    yyMeridian = MER24;
	    yyDSTmode = DSToff;
	}
	| tUNUMBER ':' tUNUMBER ':' tUNUMBER o_merid {
	    yyHour = $1;
	    yyMinutes = $3;
	    yySeconds = $5;
	    yyMeridian = $6;
	}
	| tUNUMBER ':' tUNUMBER ':' tUNUMBER numzone {
	    yyHour = $1;
	    yyMinutes = $3;
	    yySeconds = $5;
	    yyTimezone = $6;
	    yyMeridian = MER24;
	    yyDSTmode = DSToff;
	}
	;

zone	: tZONE {
	    $$ = $1;
	    yyDSTmode = DSToff;
	}
	| tDAYZONE {
	    $$ = $1;
	    yyDSTmode = DSTon;
	}
	| tDAYZONE tDST {
	    yyTimezone = $1;
	    yyDSTmode = DSTon;
	}
	| tZONE numzone {
	    /* Only allow "GMT+300" and "GMT-0800" */
	    if ($1 != 0) {
		YYABORT;
	    }
	    $$ = $2;
	    yyDSTmode = DSToff;
	}
	| numzone {
	    $$ = $1;
	    yyDSTmode = DSToff;
	}
	;

numzone	: tSNUMBER {
	    int	i;

	    /* Unix and GMT and numeric timezones -- a little confusing. */
	    if ((int)$1 < 0) {
		/* Don't work with negative modulus. */
		$1 = -(int)$1;
		if ($1 > 9999 || (i = (int) ($1 % 100)) >= 60) {
			YYABORT;
		}
		$$ = ($1 / 100) * 60 + i;
	    }
	    else {
		if ($1 > 9999 || (i = (int) ($1 % 100)) >= 60) {
			YYABORT;
		}
		$$ = -(($1 / 100) * 60 + i);
	    }
	}
	;

date	: tUNUMBER '/' tUNUMBER {
	    yyMonth = $1;
	    yyDay = $3;
	}
	| tUNUMBER '/' tUNUMBER '/' tUNUMBER {
	    if ($1 > 100) {
		yyYear = $1;
		yyMonth = $3;
		yyDay = $5;
	    }
	    else {
		yyMonth = $1;
		yyDay = $3;
		yyYear = $5;
	    }
	}
	| tMONTH tUNUMBER {
	    yyMonth = $1;
	    yyDay = $2;
	}
	| tMONTH tUNUMBER ',' tUNUMBER {
	    yyMonth = $1;
	    yyDay = $2;
	    yyYear = $4;
	}
	| tUNUMBER tMONTH {
	    yyDay = $1;
	    yyMonth = $2;
	}
	| tUNUMBER tMONTH tUNUMBER {
	    yyDay = $1;
	    yyMonth = $2;
	    yyYear = $3;
	}
	| tDAY ',' tUNUMBER tMONTH tUNUMBER {
	    yyDay = $3;
	    yyMonth = $4;
	    yyYear = $5;
	}
	| tDAY ',' tUNUMBER '-' tMONTH tSNUMBER {
	    yyDay = $3;
	    yyMonth = $5;
	    yyYear = -$6;
	}
	| tUNUMBER tSNUMBER tSNUMBER {
	    yyDay = $1;
	    yyMonth = -$2;
	    yyYear = -$3;
	    yyDSTmode = DSToff;	/* assume midnight if no time given */
	    yyTimezone = 0;	/* Lynx assumes GMT for this format */
	}
	;

both	: tDAY tMONTH tUNUMBER tUNUMBER ':' tUNUMBER ':' tUNUMBER tUNUMBER {
	    yyMonth = $2;
	    yyDay = $3;
	    yyYear = $9;
	    yyHour = $4;
	    yyMinutes = $6;
	    yySeconds = $8;
	}
	;

rel	: tSNUMBER tSEC_UNIT {
	    yyRelSeconds += $1 * $2;
	}
	| tUNUMBER tSEC_UNIT {
	    yyRelSeconds += $1 * $2;
	}
	| tSNUMBER tMONTH_UNIT {
	    yyRelMonth += $1 * $2;
	}
	| tUNUMBER tMONTH_UNIT {
	    yyRelMonth += $1 * $2;
	}
	;

o_merid	: /* NULL */ {
	    $$ = MER24;
	}
	| tMERIDIAN {
	    $$ = $1;
	}
	;

%%


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
    if ((long) Minutes < 0 || Minutes > 59 || (long) Seconds < 0 || Seconds > 61)
	return -1;
    if (Meridian == MER24) {
	if ((long) Hours < 0 || Hours > 23)
	    return -1;
    } else {
	if (Hours < 1 || Hours > 12)
	    return -1;
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
    static const int LeapYears[] =
    {
	1972, 1976, 1980, 1984, 1988, 1992, 1996,
	2000, 2004, 2008, 2012, 2016, 2020, 2024, 2028, 2032, 2036
    };
    const int *yp;
    const int *mp;
    int i;
    time_t Julian;
    time_t tod;

    if ((long) Year < 0)
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
	return -1;
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
    if ((long) (tod = ToSeconds(Hours, Minutes, Seconds, Meridian)) < 0) {
	return -1;
    }
    Julian += tod;
    tod = Julian;
    if (dst == DSTon || (dst == DSTmaybe && localtime(&tod)->tm_isdst))
	Julian -= DST_OFFSET * 60 * 60;
    return Julian;
}

static time_t DSTcorrect(time_t Start, time_t Future)
{
    time_t StartDay;
    time_t FutureDay;

    StartDay = (localtime(&Start)->tm_hour + 1) % 24;
    FutureDay = (localtime(&Future)->tm_hour + 1) % 24;
    return (Future - Start) + (StartDay - FutureDay) * DST_OFFSET * 60 * 60;
}

static time_t RelativeMonth(time_t Start, time_t RelMonth)
{
    struct tm *tm;
    time_t Month;
    time_t Year;

    tm = localtime(&Start);
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

    tm = localtime(&now->time);
    yyYear = tm->tm_year + 1900;
    yyMonth = tm->tm_mon + 1;
    yyDay = tm->tm_mday;
    yyTimezone = now->tzone;
    if (tm->tm_isdst)		/* Correct timezone offset for DST */
	yyTimezone += DST_OFFSET * 60;
    yyDSTmode = DSTmaybe;
    yyHour = 0;
    yyMinutes = 0;
    yySeconds = 0;
    yyMeridian = MER24;
    yyRelSeconds = 0;
    yyRelMonth = 0;
    yyHaveDate = 0;
    yyHaveRel = 0;
    yyHaveTime = 0;

    if (date_parse() || yyHaveTime > 1 || yyHaveDate > 1)
	return -1;

    if (yyHaveDate || yyHaveTime) {
	Start = Convert(yyMonth, yyDay, yyYear, yyHour, yyMinutes, yySeconds,
			yyMeridian, yyDSTmode);
	if ((long) Start < 0)
	    return -1;
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
    return (Start == (time_t) -1) ? 0 : Start;
}
