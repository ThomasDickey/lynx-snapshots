/*		Our Static DTD for HTML
**		-----------------------
*/

/* Implements:
*/

#include "HTUtils.h"
#include "HTMLDTD.h"
#include "LYLeaks.h"

/* 	Entity Names
**	------------
**
**	This table must be matched exactly with ALL the translation tables
*/
static CONST char* entities[] = {
  "AElig",	/* capital AE diphthong (ligature) */ 
  "Aacute",	/* capital A, acute accent */ 
  "Acirc",	/* capital A, circumflex accent */ 
  "Agrave",	/* capital A, grave accent */ 
  "Aring",	/* capital A, ring */ 
  "Atilde",	/* capital A, tilde */ 
  "Auml",	/* capital A, dieresis or umlaut mark */ 
  "Ccedil",	/* capital C, cedilla */ 
  "Dstrok",	/* capital Eth, Icelandic */ 
  "ETH",	/* capital Eth, Icelandic */ 
  "Eacute",	/* capital E, acute accent */ 
  "Ecirc",	/* capital E, circumflex accent */ 
  "Egrave",	/* capital E, grave accent */ 
  "Euml",	/* capital E, dieresis or umlaut mark */ 
  "Iacute",	/* capital I, acute accent */ 
  "Icirc",	/* capital I, circumflex accent */ 
  "Igrave",	/* capital I, grave accent */ 
  "Iuml",	/* capital I, dieresis or umlaut mark */ 
  "Ntilde",	/* capital N, tilde */ 
  "Oacute",	/* capital O, acute accent */ 
  "Ocirc",	/* capital O, circumflex accent */ 
  "Ograve",	/* capital O, grave accent */ 
  "Oslash",	/* capital O, slash */ 
  "Otilde",	/* capital O, tilde */ 
  "Ouml",	/* capital O, dieresis or umlaut mark */ 
  "THORN",	/* capital THORN, Icelandic */ 
  "Uacute",	/* capital U, acute accent */ 
  "Ucirc",	/* capital U, circumflex accent */ 
  "Ugrave",	/* capital U, grave accent */ 
  "Uuml",	/* capital U, dieresis or umlaut mark */ 
  "Yacute",	/* capital Y, acute accent */ 
  "aacute",	/* small a, acute accent */ 
  "acirc",	/* small a, circumflex accent */ 
  "acute",	/* spacing acute */
  "aelig",	/* small ae diphthong (ligature) */ 
  "agrave",	/* small a, grave accent */ 
  "amp",	/* ampersand */ 
  "aring",	/* small a, ring */ 
  "atilde",	/* small a, tilde */ 
  "auml",	/* small a, dieresis or umlaut mark */ 
  "brkbar",	/* broken vertical bar */
  "brvbar",	/* broken vertical bar */
  "ccedil",	/* small c, cedilla */ 
  "cedil",	/* spacing cedilla */
  "cent",	/* cent sign */
  "copy",	/* copyright sign */
  "curren",	/* currency sign */
  "deg",	/* degree sign */
  "die",	/* spacing diaresis */
  "divide",	/* division sign */
  "eacute",	/* small e, acute accent */ 
  "ecirc",	/* small e, circumflex accent */ 
  "egrave",	/* small e, grave accent */ 
  "emdash",	/* dash the width of emsp */
  "emsp",	/* em space - not collapsed */
  "endash",	/* dash the width of ensp */
  "ensp",	/* en space - not collapsed */
  "eth",	/* small eth, Icelandic */ 
  "euml",	/* small e, dieresis or umlaut mark */ 
  "frac12",	/* fraction 1/2 */
  "frac14",	/* fraction 1/4 */
  "frac34",	/* fraction 3/4 */
  "gt",		/* greater than */ 
  "hibar",	/* spacing macron */
  "iacute",	/* small i, acute accent */ 
  "icirc",	/* small i, circumflex accent */ 
  "iexcl",	/* inverted exclamation mark */
  "igrave",	/* small i, grave accent */ 
  "iquest",	/* inverted question mark */
  "iuml",	/* small i, dieresis or umlaut mark */ 
  "laquo",	/* angle quotation mark, left */
  "lt",		/* less than */ 
  "macr",	/* spacing macron */
  "mdash",	/* dash the width of emsp */
  "micro",	/* micro sign */
  "middot",	/* middle dot */
  "nbsp",       /* non breaking space */
  "ndash",	/* dash the width of ensp */
  "not",	/* negation sign */
  "ntilde",	/* small n, tilde */ 
  "oacute",	/* small o, acute accent */ 
  "ocirc",	/* small o, circumflex accent */ 
  "ograve",	/* small o, grave accent */ 
  "ordf",	/* feminine ordinal indicator */
  "ordm",	/* masculine ordinal indicator */
  "oslash",	/* small o, slash */ 
  "otilde",	/* small o, tilde */ 
  "ouml",	/* small o, dieresis or umlaut mark */ 
  "para",	/* paragraph sign */
  "plusmn",	/* plus-or-minus sign */
  "pound",	/* pound sign */
  "quot",	/* quot '"' */
  "raquo",	/* angle quotation mark, right */
  "reg",	/* circled R registered sign */
  "sect",	/* section sign */
  "shy",	/* soft hyphen */
  "sup1",	/* superscript 1 */
  "sup2",	/* superscript 2 */
  "sup3",	/* superscript 3 */
  "szlig",	/* small sharp s, German (sz ligature) */ 
  "thinsp",	/* thin space (not collapsed) */ 
  "thorn",	/* small thorn, Icelandic */ 
  "times",	/* multiplication sign */ 
  "trade",	/* trade mark sign (U+2122) */ 
  "uacute",	/* small u, acute accent */ 
  "ucirc",	/* small u, circumflex accent */ 
  "ugrave",	/* small u, grave accent */ 
  "uml",	/* spacing diaresis */
  "uuml",	/* small u, dieresis or umlaut mark */ 
  "yacute",	/* small y, acute accent */ 
  "yen",	/* yen sign */
  "yuml",	/* small y, dieresis or umlaut mark */ 
};

#define HTML_ENTITIES 112

#ifdef EXP_CHARTRANS
/* 	Extra Entity Names
**	------------------
**
**	This table contains Unicodes in addition to the Names.
**
**      Just an idea how it could be done. -kw
*
*	I think in the future the whole entities[] thing above could migrate
*	to this kind of structure.  The structured streams to which
*	the SGML modules sends its output could then easily have access
*	to both entity names and unicode values for each (special)
*	character.  Probably the whole translation to display characters
*	should be done at that later stage (e.g. in HTML.c).
*	What's missing is a way for the later stage to return info
*	to SGML whether the entity could be displayed or not.
*	(like between SGML_character() and handle_entity() via FoundEntity.)
*	Well, trying to do that now.
*       Why keep two structures for entities?  Backward compatibility..
*/

/* UC_entity_info structure is defined in SGML.h. */
/* This has to be sorted alphabetically, 
   bear this in mind when you add some more entities.. 
   
   Now we have here: - all ISO-8859-2 entites 
                     - lrm, rlm, zwnj and zwj 
*/
static CONST UC_entity_info extra_entities[] = {
  {"Aacute",  0x00c1},  /* A with acute */
  {"Abreve",  0x0102},  /* A with breve */
  {"Acirc",  0x00c2},  /* A with circumflex */
  {"Aogon",  0x0104},  /* A with ogonek */
  {"Auml",  0x00c4},  /* A with diaeresis */
  {"Cacute",  0x0106},  /* C with acute */
  {"Ccaron",  0x010c},  /* C with caron */
  {"Ccedil",  0x00c7},  /* C with cedilla */
  {"Dcaron",  0x010e},  /* D with caron */
  {"Dstrok",  0x0110},  /* D with stroke */
  {"Eacute",  0x00c9},  /* E with acute */
  {"Ecaron",  0x011a},  /* E with caron */
  {"Eogon",  0x0118},  /* E with ogonek */
  {"Euml",  0x00cb},  /* E with diaeresis */
  {"Iacute",  0x00cd},  /* I with acute */
  {"Icirc",  0x00ce},  /* I with circumflex */
  {"Lacute",  0x0139},  /* L with acute */
  {"Lcaron",  0x013d},  /* L with caron */
  {"Lstrok",  0x0141},  /* L with stroke */
  {"Nacute",  0x0143},  /* N with acute */
  {"Ncaron",  0x0147},  /* N with caron */
  {"Oacute",  0x00d3},  /* O with acute */
  {"Ocirc",  0x00d4},  /* O with circumflex */
  {"Odblac",  0x0150},  /* O with double acute */
  {"Ouml",  0x00d6},  /* O with diaeresis */
  {"Racute",  0x0154},  /* R with acute */
  {"Rcaron",  0x0158},  /* R with caron */
  {"Sacute",  0x015a},  /* S with acute */
  {"Scaron",  0x0160},  /* S with caron */
  {"Scedil",  0x015e},  /* S with cedilla */
  {"Tcaron",  0x0164},  /* T with caron */
  {"Tcedil",  0x0162},  /* T with cedilla */
  {"Uacute",  0x00da},  /* U with acute */
  {"Udblac",  0x0170},  /* U with double acute */
  {"Uring",  0x016e},  /* U with ring above */
  {"Uuml",  0x00dc},  /* U with diaeresis */
  {"Yacute",  0x00dd},  /* Y with acute */
  {"Zacute",  0x0179},  /* Z with acute */
  {"Zcaron",  0x017d},  /* Z with caron */
  {"Zdot",  0x017b},  /* Z with dot above */
  {"aacute",  0x00e1},  /* a with acute */
  {"abreve",  0x0103},  /* a with breve */
  {"acirc",  0x00e2},  /* a with circumflex */
  {"acute",  0x00b4},  /* acuteaccent */
  {"amp",  0x0026},  /* ampersand */
  {"aogon",  0x0105},  /* a with ogonek */
  {"apos",  0x0027},  /* apostrophe */
  {"ast",  0x002a},  /* asterisk */
  {"auml",  0x00e4},  /* a with diaeresis */
  {"breve",  0x02d8},  /* breve */
  {"bsol",  0x005c},  /* reversesolidus */
  {"cacute",  0x0107},  /* c with acute */
  {"caron",  0x02c7},  /* caron */
  {"ccaron",  0x010d},  /* c with caron */
  {"ccedil",  0x00e7},  /* c with cedilla */
  {"cedil",  0x00b8},  /* cedilla */
  {"circ",  0x005e},  /* circumflexaccent */
  {"colon",  0x003a},  /* colon */
  {"comma",  0x002c},  /* comma */
  {"commat",  0x0040},  /* commercialat */
  {"curren",  0x00a4},  /* currencysign */
  {"dblac",  0x02dd},  /* doubleacuteaccent */
  {"dcaron",  0x010f},  /* d with caron */
  {"deg",  0x00b0},  /* degreesign */
  {"divide",  0x00f7},  /* divisionsign */
  {"dollar",  0x0024},  /* dollarsign */
  {"dot",  0x02d9},  /* dotabove */
  {"dstrok",  0x0111},  /* d with stroke */
  {"eacute",  0x00e9},  /* e with acute */
  {"ecaron",  0x011b},  /* e with caron */
  {"eogon",  0x0119},  /* e with ogonek */
  {"equals",  0x003d},  /* equalssign */
  {"euml",  0x00eb},  /* e with diaeresis */
  {"excl",  0x0021},  /* exclamationmark */
  {"grave",  0x0060},  /* graveaccent */
  {"gt",  0x003e},  /* greater-thansign */
  {"hyphen",  0x002d},  /* hyphen-minus */
  {"iacute",  0x00ed},  /* i with acute */
  {"icirc",  0x00ee},  /* i with circumflex */
  {"lacute",  0x013a},  /* l with acute */
  {"lcaron",  0x013e},  /* l with caron */
  {"lcub",  0x007b},  /* leftcurlybracket */
  {"lowbar",  0x005f},  /* lowline */
  {"lpar",  0x0028},  /* leftparenthesis */
  {"lrm",	8206},	/* left-to-right mark */ 
  {"lsqb",  0x005b},  /* leftsquarebracket */
  {"lstrok",  0x0142},  /* l with stroke */
  {"lt",  0x003c},  /* less-thansign */
  {"nacute",  0x0144},  /* n with acute */
  {"nbsp",  0x00a0},  /* no-breakspace */
  {"ncaron",  0x0148},  /* n with caron */
  {"num",  0x0023},  /* numbersign */
  {"oacute",  0x00f3},  /* o with acute */
  {"ocirc",  0x00f4},  /* o with circumflex */
  {"odblac",  0x0151},  /* o with double acute */
  {"ogon",  0x02db},  /* ogonek */
  {"ouml",  0x00f6},  /* o with diaeresis */
  {"percnt",  0x0025},  /* percentsign */
  {"period",  0x002e},  /* fullstop */
  {"plus",  0x002b},  /* plussign */
  {"quest",  0x003f},  /* questionmark */
  {"quot",  0x0022},  /* quotationmark */
  {"racute",  0x0155},  /* r with acute */
  {"rcaron",  0x0159},  /* r with caron */
  {"rcub",  0x007d},  /* rightcurlybracket */
  {"rlm",	8207},	/* right-to-left mark */ 
  {"rpar",  0x0029},  /* rightparenthesis */
  {"rsqb",  0x005d},  /* rightsquarebracket */
  {"sacute",  0x015b},  /* s with acute */
  {"scaron",  0x0161},  /* s with caron */
  {"scedil",  0x015f},  /* s with cedilla */
  {"sect",  0x00a7},  /* sectionsign */
  {"semi",  0x003b},  /* semicolon */
  {"shy",  0x00ad},  /* softhyphen */
  {"sol",  0x002f},  /* solidus */
  {"tcaron",  0x0165},  /* t with caron */
  {"tcedil",  0x0163},  /* t with cedilla */
  {"tilde",  0x007e},  /* tilde */
  {"times",  0x00d7},  /* multiplicationsign */
  {"uacute",  0x00fa},  /* u with acute */
  {"udblac",  0x0171},  /* u with double acute */
  {"uml",  0x00a8},  /* diaeresis */
  {"uring",  0x016f},  /* u with ring above */
  {"uuml",  0x00fc},  /* u with diaeresis */
  {"verbar",  0x007c},  /* verticalline */
  {"yacute",  0x00fd},  /* y with acute */
  {"zacute",  0x017a},  /* z with acute */
  {"zcaron",  0x017e},  /* z with caron */
  {"zdot",  0x017c},  /* z with dot above */
  {"zwj",	8205},	/* zero width joiner */ 
  {"zwnj",	8204},	/* zero width non-joiner */ 

};
#endif /* EXP_CHARTRANS */

/*		Attribute Lists
**		---------------
**
**	Lists must be in alphatbetical order by attribute name
**	The tag elements contain the number of attributes
*/
static attr a_attr[] = {			/* Anchor attributes */
	{ "ACCESSKEY" },
	{ "CHARSET" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "COORDS" },
	{ "DIR" },
	{ "HREF" },
	{ "ID" },
	{ "ISMAP" },
	{ "LANG" },
	{ "MD" },
	{ "NAME" },
	{ "NOTAB" },
	{ "ONCLICK" },
	{ "ONMOUSEOUT" },
	{ "ONMOUSEOVER" },
	{ "REL" },
	{ "REV" },
	{ "SHAPE" },
	{ "STYLE" },
	{ "TABINDEX" },
	{ "TARGET" },
	{ "TITLE" },
	{ "TYPE" },
	{ "URN" },
	{ 0 }	/* Terminate list */
};	

static attr address_attr[] = {			/* ADDRESS attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "NOWRAP" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr applet_attr[] = {			/* APPLET attributes */
	{ "ALIGN" },
	{ "ALT" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "CODE" },
	{ "CODEBASE" },
	{ "DIR" },
	{ "DOWNLOAD" },
	{ "HEIGHT" },
	{ "HSPACE" },
	{ "ID" },
	{ "LANG" },
	{ "NAME" },
	{ "STYLE" },
	{ "TITLE" },
	{ "VSPACE" },
	{ "WIDTH" },
	{ 0 }	/* Terminate list */
};

static attr area_attr[] = {			/* AREA attributes */
	{ "ALT" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "COORDS" },
	{ "DIR" },
	{ "HREF" },
	{ "ID" },
	{ "LANG" },
	{ "NOHREF" },
	{ "NOTAB" },
	{ "ONCLICK" },
	{ "ONMOUSEOUT" },
	{ "ONMOUSEOVER" },
	{ "SHAPE" },
	{ "STYLE" },
	{ "TABINDEX" },
	{ "TARGET" },
	{ "TITLE" },
	{ 0 }	/* Terminate list */
};

static attr base_attr[] = {			/* BASE attributes */
	{ "HREF" },
	{ "TARGET" },
	{ 0 }	/* Terminate list */
};	

static attr bgsound_attr[] = {			/* BGSOUND attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "LOOP" },
	{ "SRC" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr body_attr[] = {			/* BODY attributes */
	{ "ALINK" },
	{ "BACKGROUND" },
	{ "BGCOLOR" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "LINK" },
	{ "ONLOAD" },
	{ "ONUNLOAD" },
	{ "STYLE" },
	{ "TEXT" },
	{ "VLINK" },
	{ 0 } /* Terminate list */
};

static attr bodytext_attr[] = {			/* BODYTEXT attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DATA" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "NAME" },
	{ "OBJECT" },
	{ "REF" },
	{ "STYLE" },
	{ "TYPE" },
	{ "VALUE" },
	{ "VALUETYPE" },
	{ 0 } /* Terminate list */
};

static attr bq_attr[] = {			/* BQ (BLOCKQUOTE) attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "NOWRAP" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr caption_attr[] = {			/* CAPTION attributes */
	{ "ACCESSKEY" },
	{ "ALIGN" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr col_attr[] = {		/* COL and COLGROUP attributes */
	{ "ALIGN" },
	{ "CHAR" },
	{ "CHAROFF" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "SPAN" },
	{ "STYLE" },
	{ "VALIGN" },
	{ "WIDTH" },
	{ 0 }	/* Terminate list */
};

static attr credit_attr[] = {			/* CREDIT attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr div_attr[] = {			/* DIV attribures */
	{ "ALIGN" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr embed_attr[] = {			/* EMBED attributes */
	{ "ALIGN" },	/* (including, for now, those from FIG and IMG) */
	{ "ALT" },
	{ "BORDER" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "HEIGHT" },
	{ "ID" },
	{ "IMAGEMAP" },
	{ "ISMAP" },
	{ "LANG" },
	{ "MD" },
	{ "NAME" },
	{ "NOFLOW" },
	{ "PARAMS" },
	{ "SRC" },
	{ "STYLE" },
	{ "UNITS" },
	{ "USEMAP" },
	{ "WIDTH" },
	{ 0 }	/* Terminate list */
};

static attr fig_attr[] = {			/* FIG attributes */
	{ "ALIGN" },
	{ "BORDER" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "HEIGHT" },
	{ "ID" },
	{ "IMAGEMAP" },
	{ "ISOBJECT" },
	{ "LANG" },
	{ "MD" },
	{ "NOFLOW" },
	{ "SRC" },
	{ "STYLE" },
	{ "UNITS" },
	{ "WIDTH" },
	{ 0 }	/* Terminate list */
};

static attr fieldset_attr[] = {			/* FIELDSET attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "STYLE" },
	{ "TITLE" },
	{ 0 }	/* Terminate list */
};

static attr fn_attr[] = {			/* FN attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr font_attr[] = {			/* FONT attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "COLOR" },
	{ "DIR" },
	{ "END" },
	{ "FACE" },
	{ "ID" },
	{ "LANG" },
	{ "SIZE" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr form_attr[] = {			/* FORM attributes */
	{ "ACTION"},
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ENCTYPE" },
	{ "ID" },
	{ "LANG" },
	{ "METHOD" },
	{ "ONSUBMIT" },
	{ "SCRIPT" },
	{ "STYLE" },
	{ "SUBJECT" },
	{ "TARGET" },
	{ "TITLE" },
	{ 0 }	/* Terminate list */
};

static attr frame_attr[] = {			/* FRAME attributes */
	{ "MARGINHEIGHT"},
	{ "MARGINWIDTH" },
	{ "NAME" },
	{ "NORESIZE" },
	{ "SCROLLING" },
	{ "SRC" },
	{ 0 }	/* Terminate list */
};

static attr frameset_attr[] = {			/* FRAMESET attributes */
	{ "COLS"},
	{ "ROWS" },
	{ 0 }	/* Terminate list */
};

static attr gen_attr[] = {			/* Minimum HTML 3.0 */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr glossary_attr[] = {			/* DL (and DLC) attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "COMPACT" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr h_attr[] = {			/* H1 - H6 attributes */
	{ "ALIGN" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DINGBAT" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "MD" },
	{ "NOWRAP" },
	{ "SEQNUM" },
	{ "SKIP" },
	{ "SRC" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr hr_attr[] = {			/* HR attributes */
	{ "ALIGN" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "MD" },
	{ "NOSHADE" },
	{ "SIZE" },
	{ "SRC" },
	{ "STYLE" },
	{ "WIDTH" },
	{ 0 }	/* Terminate list */
};

static attr img_attr[] = {			/* IMG attributes */
	{ "ALIGN" },
	{ "ALT" },
	{ "BORDER" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "HEIGHT" },
	{ "ID" },
	{ "ISMAP" },
	{ "ISOBJECT" },
	{ "LANG" },
	{ "MD" },
	{ "SRC" },
	{ "STYLE" },
	{ "TITLE" },
	{ "UNITS" },
	{ "USEMAP" },
	{ "WIDTH" },
	{ 0 }	/* Terminate list */
};	

static attr input_attr[] = {			/* INPUT attributes */
	{ "ACCEPT" },
	{ "ALIGN" },
	{ "ALT" },
	{ "CHECKED" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "DISABLED" },
	{ "ERROR" },
	{ "HEIGHT" },
	{ "ID" },
	{ "LANG" },
	{ "MAX" },
	{ "MAXLENGTH" },
	{ "MD" },
	{ "MIN" },
	{ "NAME" },
	{ "NOTAB" },
	{ "ONBLUR" },
	{ "ONCHANGE" },
	{ "ONCLICK" },
	{ "ONFOCUS" },
	{ "ONSELECT" },
	{ "SIZE" },
	{ "SRC" },
	{ "STYLE" },
	{ "TABINDEX" },
	{ "TITLE" },
	{ "TYPE" },
	{ "VALUE" },
	{ "WIDTH" },
	{ 0 } /* Terminate list */
};

static attr isindex_attr[] = {			/* ISINDEX attributes */
	{ "ACTION" },	/* Not in spec.  Lynx treats it as HREF. - FM */
	{ "DIR" },
	{ "HREF" },	/* HTML 3.0 attritute for search action. - FM */ 
	{ "LANG" },
	{ "PROMPT" },	/* HTML 3.0 attribute for prompt string. - FM */
	{ 0 }	/* Terminate list */
};	

static attr keygen_attr[] = {			/* KEYGEN attributes */
	{ "CHALLENGE" },
	{ "CLASS" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "NAME" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr label_attr[] = {			/* LABEL attributes */
	{ "ACCESSKEY" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "FOR" },
	{ "ID" },
	{ "LANG" },
	{ "ONCLICK" },
	{ "STYLE" },
	{ "TITLE" },
	{ 0 }	/* Terminate list */
};

static attr link_attr[] = {			/* LINK attributes */
	{ "CLASS" },
	{ "HREF" },
	{ "ID" },
	{ "REL" },
	{ "REV" },
	{ "STYLE" },
	{ "TARGET" },
	{ "TITLE" },
	{ "TYPE" },
	{ 0 }	/* Terminate list */
};

static attr list_attr[] = {			/* LI attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DINGBAT" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "MD" },
	{ "SKIP" },
	{ "SRC" },
	{ "STYLE" },
	{ "TYPE" },
	{ "VALUE" },
	{ 0 }	/* Terminate list */
};

static attr map_attr[] = {			/* MAP attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "NAME" },
	{ "STYLE" },
	{ "TITLE" },
	{ 0 }	/* Terminate list */
};

static attr math_attr[] = {			/* MATH attributes */
	{ "BOX" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr meta_attr[] = {			/* META attributes */
	{ "CONTENT" },
	{ "HTTP-EQUIV" },
	{ "NAME" },
	{ 0 }	/* Terminate list */
};

static attr nextid_attr[] = {			/* NEXTID attribures */
	{ "N" }
};

static attr note_attr[]	= {			/* NOTE attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "MD" },
	{ "ROLE" },
	{ "SRC" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr object_attr[] = {			/* OBJECT attributes */
	{ "ALIGN" },
	{ "BORDER" },
	{ "CLASS" },
	{ "CLASSID" },
	{ "CODEBASE" },
	{ "CODETYPE" },
	{ "DATA" },
	{ "DECLARE" },
	{ "DIR" },
	{ "HEIGHT" },
	{ "HSPACE" },
	{ "ID" },
	{ "ISMAP" },
	{ "LANG" },
	{ "NAME" },
	{ "NOTAB" },
	{ "SHAPES" },
	{ "STANDBY" },
	{ "STYLE" },
	{ "TABINDEX" },
	{ "TITLE" },
	{ "TYPE" },
	{ "USEMAP" },
	{ "VSPACE" },
	{ "WIDTH" },
	{ 0 } /* Terminate list */
};

static attr olist_attr[] = {			/* OL attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "COMPACT" },
	{ "CONTINUE" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "SEQNUM" },
	{ "START" },
	{ "STYLE" },
	{ "TYPE" },
	{ 0 }	/* Terminate list */
};

static attr option_attr[] = {			/* OPTION attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "DISABLED" },
	{ "ERROR" },
	{ "ID" },
	{ "LANG" },
	{ "SELECTED" },
	{ "SHAPE" },
	{ "STYLE" },
	{ "VALUE" },
	{ 0 }	/* Terminate list */
};

static attr overlay_attr[] = {			/* OVERLAY attributes */
	{ "CLASS" },
	{ "HEIGHT" },
	{ "ID" },
	{ "IMAGEMAP" },
	{ "MD" },
	{ "SRC" },
	{ "STYLE" },
	{ "UNITS" },
	{ "WIDTH" },
	{ "X" },
	{ "Y" },
	{ 0 }	/* Terminate list */
};

static attr p_attr[] = {			/* P attributes */
	{ "ALIGN" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "NOWRAP" },
	{ "STYLE" },
	{ 0 }	/* Terminate list */
};

static attr param_attr[] = {			/* PARAM attribures */
	{ "ACCEPT" },
	{ "ACCEPT-CHARSET" },
	{ "ACCEPT-ENCODING" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DATA" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "NAME" },
	{ "OBJECT" },
	{ "REF" },
	{ "STYLE" },
	{ "TYPE" },
	{ "VALUE" },
	{ "VALUEREF" },
	{ "VALUETYPE" },
	{ 0 }	/* Terminate list */
};

static attr script_attr[] = {			/* SCRIPT attribures */
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "EVENT" },
	{ "FOR" },
	{ "ID" },
	{ "LANG" },
	{ "LANGUAGE" },
	{ "NAME" },
	{ "SCRIPTENGINE" },
	{ "SRC" },
	{ "STYLE" },
	{ "TYPE" },
	{ 0 }	/* Terminate list */
};

static attr select_attr[] = {			/* SELECT attributes */
	{ "ALIGN" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "DISABLED" },
	{ "ERROR" },
	{ "HEIGHT" },
	{ "ID" },
	{ "LANG" },
	{ "MD" },
	{ "MULTIPLE" },
	{ "NAME" },
	{ "NOTAB" },
	{ "ONBLUR" },
	{ "ONCHANGE" },
	{ "ONFOCUS" },
	{ "SIZE" },
	{ "STYLE" },
	{ "TABINDEX" },
	{ "TITLE" },
	{ "UNITS" },
	{ "WIDTH" },
	{ 0 }	/* Terminate list */
};

static attr style_attr[] = {			/* STYLE attributes */
	{ "DIR" },
	{ "LANG" },
	{ "NOTATION" },
	{ "TITLE" },
	{ 0 }	/* Terminate list */
};

static attr tab_attr[] = {			/* TAB attributes */
	{ "ALIGN" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "DP" },
	{ "ID" },
	{ "INDENT" },
	{ "LANG" },
	{ "STYLE" },
	{ "TO" },
	{ 0 }	/* Terminate list */
};

static attr table_attr[] = {			/* TABLE attributes */
	{ "ALIGN" },
	{ "BORDER" },
	{ "CELLPADDING" },
	{ "CELLSPACING" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "COLS" },
	{ "COLSPEC" },
	{ "DIR" },
	{ "DP" },
	{ "FRAME" },
	{ "ID" },
	{ "LANG" },
	{ "NOFLOW" },
	{ "NOWRAP" },
	{ "RULES" },
	{ "STYLE" },
	{ "UNITS" },
	{ "WIDTH" },
	{ 0 }	/* Terminate list */
};

static attr td_attr[] = {			/* TD and TH attributes */
	{ "ALIGN" },
	{ "AXES" },
	{ "AXIS" },
	{ "CHAR" },
	{ "CHAROFF" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "COLSPAN" },
	{ "DIR" },
	{ "DP" },
	{ "ID" },
	{ "LANG" },
	{ "NOWRAP" },
	{ "ROWSPAN" },
	{ "STYLE" },
	{ "VALIGN" },
	{ 0 }	/* Terminate list */
};

static attr textarea_attr[] = {			/* TEXTAREA attributes */
	{ "ALIGN" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "COLS" },
	{ "DIR" },
	{ "DISABLED" },
	{ "ERROR" },
	{ "ID" },
	{ "LANG" },
	{ "NAME" },
	{ "NOTAB" },
	{ "ONBLUR" },
	{ "ONCHANGE" },
	{ "ONFOCUS" },
	{ "ONSELECT" },
	{ "ROWS" },
	{ "STYLE" },
	{ "TABINDEX" },
	{ "TITLE" },
	{ 0 }	/* Terminate list */
};

static attr tr_attr[] = {	/* TR, THEAD, TFOOT, and TBODY attributes */
	{ "ALIGN" },
	{ "CHAR" },
	{ "CHAROFF" },
	{ "CLASS" },
	{ "CLEAR" },
	{ "DIR" },
	{ "DP" },
	{ "ID" },
	{ "LANG" },
	{ "NOWRAP" },
	{ "STYLE" },
	{ "VALIGN" },
	{ 0 }	/* Terminate list */
};

static attr ulist_attr[] = {			/* UL attributes */
	{ "CLASS" },
	{ "CLEAR" },
	{ "COMPACT" },
	{ "DINGBAT" },
	{ "DIR" },
	{ "ID" },
	{ "LANG" },
	{ "MD" },
	{ "PLAIN" },
	{ "SRC" },
	{ "STYLE" },
	{ "TYPE" },
	{ "WRAP" },
	{ 0 }	/* Terminate list */
};

/*	Elements
**	--------
**
**	Must match definitions in HTMLDTD.html!
**	Must be in alphabetical order.
**
**    Name, 	Attributes, 		content
*/
static HTTag tags[HTML_ELEMENTS] = {
    { "A"	, a_attr,	HTML_A_ATTRIBUTES,	SGML_MIXED },
    { "ABBREV"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "ACRONYM"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "ADDRESS"	, address_attr,	HTML_ADDRESS_ATTRIBUTES, SGML_MIXED },
    { "APPLET"	, applet_attr,	HTML_APPLET_ATTRIBUTES, SGML_MIXED },
    { "AREA"	, area_attr,	HTML_AREA_ATTRIBUTES,   SGML_EMPTY },
    { "AU"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "AUTHOR"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "B"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "BANNER"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "BASE"	, base_attr,	HTML_BASE_ATTRIBUTES,	SGML_EMPTY },
    { "BASEFONT", font_attr,	HTML_FONT_ATTRIBUTES,	SGML_EMPTY },
    { "BDO"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "BGSOUND"	, bgsound_attr,	HTML_BGSOUND_ATTRIBUTES, SGML_EMPTY },
    { "BIG"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "BLINK"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "BLOCKQUOTE", bq_attr,	HTML_BQ_ATTRIBUTES,	SGML_MIXED },
    { "BODY"	, body_attr,	HTML_BODY_ATTRIBUTES,	SGML_MIXED },
    { "BODYTEXT", bodytext_attr,HTML_BODYTEXT_ATTRIBUTES, SGML_MIXED },
    { "BQ"	, bq_attr,	HTML_BQ_ATTRIBUTES,	SGML_MIXED },
    { "BR"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_EMPTY },
    { "CAPTION"	, caption_attr,	HTML_CAPTION_ATTRIBUTES, SGML_MIXED },
    { "CENTER"	, div_attr,	HTML_DIV_ATTRIBUTES,	SGML_MIXED },
    { "CITE"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "CODE"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "COL"	, col_attr,	HTML_COL_ATTRIBUTES,	SGML_EMPTY },
    { "COLGROUP", col_attr,	HTML_COL_ATTRIBUTES,	SGML_EMPTY },
    { "COMMENT"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "CREDIT"	, credit_attr,	HTML_CREDIT_ATTRIBUTES,	SGML_MIXED },
    { "DD"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_EMPTY },
    { "DEL"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "DFN"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "DIR"	, ulist_attr,	HTML_UL_ATTRIBUTES,	SGML_MIXED },
    { "DIV"	, div_attr,	HTML_DIV_ATTRIBUTES,	SGML_MIXED },
    { "DL"	, glossary_attr, HTML_DL_ATTRIBUTES,	SGML_MIXED },
    { "DLC"	, glossary_attr, HTML_DL_ATTRIBUTES,	SGML_MIXED },
    { "DT"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_EMPTY },
    { "EM"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "EMBED"	, embed_attr,	HTML_EMBED_ATTRIBUTES,	SGML_EMPTY },
    { "FIELDSET", fieldset_attr,HTML_FIELDSET_ATTRIBUTES, SGML_MIXED },
    { "FIG"	, fig_attr,	HTML_FIG_ATTRIBUTES,	SGML_MIXED },
    { "FN"	, fn_attr,	HTML_FN_ATTRIBUTES,	SGML_MIXED },
    { "FONT"	, font_attr,	HTML_FONT_ATTRIBUTES,	SGML_EMPTY },
    { "FORM"	, form_attr,	HTML_FORM_ATTRIBUTES,	SGML_EMPTY },
    { "FRAME"	, frame_attr,	HTML_FRAME_ATTRIBUTES,	SGML_EMPTY },
    { "FRAMESET", frameset_attr,HTML_FRAMESET_ATTRIBUTES, SGML_MIXED },
    { "H1"	, h_attr,	HTML_H_ATTRIBUTES,	SGML_MIXED },
    { "H2"	, h_attr,	HTML_H_ATTRIBUTES,	SGML_MIXED },
    { "H3"	, h_attr,	HTML_H_ATTRIBUTES,	SGML_MIXED },
    { "H4"	, h_attr,	HTML_H_ATTRIBUTES,	SGML_MIXED },
    { "H5"	, h_attr,	HTML_H_ATTRIBUTES,	SGML_MIXED },
    { "H6"	, h_attr,	HTML_H_ATTRIBUTES,	SGML_MIXED },
    { "HEAD"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "HR"	, hr_attr,	HTML_HR_ATTRIBUTES,	SGML_EMPTY },
    { "HTML"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "I"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "IMG"     , img_attr,	HTML_IMG_ATTRIBUTES,	SGML_EMPTY },
    { "INPUT"   , input_attr,	HTML_INPUT_ATTRIBUTES,	SGML_EMPTY },
    { "INS"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "ISINDEX" , isindex_attr,	HTML_ISINDEX_ATTRIBUTES,SGML_EMPTY },
    { "KBD"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "KEYGEN"	, keygen_attr,	HTML_KEYGEN_ATTRIBUTES,	SGML_EMPTY },
    { "LABEL"	, label_attr,	HTML_LABEL_ATTRIBUTES,	SGML_MIXED },
    { "LH"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_EMPTY },
    { "LI"	, list_attr,	HTML_LI_ATTRIBUTES,	SGML_EMPTY },
    { "LINK"	, link_attr,	HTML_LINK_ATTRIBUTES,	SGML_EMPTY },
    { "LISTING"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_LITTERAL },
    { "MAP"	, map_attr,	HTML_MAP_ATTRIBUTES,	SGML_MIXED },
    { "MARQUEE"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "MATH"	, math_attr,	HTML_MATH_ATTRIBUTES,	SGML_LITTERAL },
    { "MENU"	, ulist_attr,	HTML_UL_ATTRIBUTES,	SGML_MIXED },
    { "META"	, meta_attr,	HTML_META_ATTRIBUTES,	SGML_EMPTY },
    { "NEXTID"  , nextid_attr,	1,			SGML_EMPTY },
    { "NOFRAMES", gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "NOTE"	, note_attr,	HTML_NOTE_ATTRIBUTES,	SGML_MIXED },
    { "OBJECT"	, object_attr,	HTML_OBJECT_ATTRIBUTES,	SGML_LITTERAL },
    { "OL"	, olist_attr,	HTML_OL_ATTRIBUTES,	SGML_MIXED },
    { "OPTION"	, option_attr,	HTML_OPTION_ATTRIBUTES,	SGML_EMPTY },
    { "OVERLAY"	, overlay_attr,	HTML_OVERLAY_ATTRIBUTES, SGML_EMPTY },
    { "P"	, p_attr,	HTML_P_ATTRIBUTES,	SGML_EMPTY },
    { "PARAM"	, param_attr,	HTML_PARAM_ATTRIBUTES,	SGML_EMPTY },
    { "PLAINTEXT", gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_LITTERAL },
    { "PRE"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "Q"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "S"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "SAMP"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "SCRIPT"	, script_attr,	HTML_SCRIPT_ATTRIBUTES,	SGML_LITTERAL },
    { "SELECT"	, select_attr,	HTML_SELECT_ATTRIBUTES,	SGML_MIXED },
    { "SMALL"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "SPAN"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "SPOT"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_EMPTY },
    { "STRIKE"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "STRONG"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "STYLE"	, style_attr,	HTML_STYLE_ATTRIBUTES,	SGML_LITTERAL },
    { "SUB"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "SUP"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "TAB"	, tab_attr,	HTML_TAB_ATTRIBUTES,	SGML_EMPTY },
    { "TABLE"	, table_attr,	HTML_TABLE_ATTRIBUTES,	SGML_MIXED },
    { "TBODY"	, tr_attr,	HTML_TR_ATTRIBUTES,	SGML_EMPTY },
    { "TD"	, td_attr,	HTML_TD_ATTRIBUTES,	SGML_EMPTY },
    { "TEXTAREA", textarea_attr,HTML_TEXTAREA_ATTRIBUTES, SGML_LITTERAL },
    { "TEXTFLOW", bodytext_attr,HTML_BODYTEXT_ATTRIBUTES, SGML_MIXED },
    { "TFOOT"	, tr_attr,	HTML_TR_ATTRIBUTES,	SGML_EMPTY },
    { "TH"	, td_attr,	HTML_TD_ATTRIBUTES,	SGML_EMPTY },
    { "THEAD"	, tr_attr,	HTML_TR_ATTRIBUTES,	SGML_EMPTY },
    { "TITLE", 	  gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_RCDATA },
    { "TR"	, tr_attr,	HTML_TR_ATTRIBUTES,	SGML_EMPTY },
    { "TT"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "U"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "UL"	, ulist_attr,	HTML_UL_ATTRIBUTES,	SGML_MIXED },
    { "VAR"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_MIXED },
    { "XMP"	, gen_attr,	HTML_GEN_ATTRIBUTES,	SGML_LITTERAL },
};

PUBLIC CONST SGML_dtd HTML_dtd = {
	tags,
	HTML_ELEMENTS,
	entities,
	sizeof(entities)/sizeof(char*),
#ifdef EXP_CHARTRANS
	extra_entities,
	sizeof(extra_entities)/sizeof(UC_entity_info),
#endif
};


/*
**	Utility Routine:  Useful for people building HTML objects.
*/

/*	Start anchor element
**	--------------------
**
**	It is kinda convenient to have a particulr routine for
**	starting an anchor element, as everything else for HTML is
**	simple anyway.
*/
struct _HTStructured {
    HTStructuredClass * isa;
	/* ... */
};

PUBLIC void HTStartAnchor ARGS3(
	HTStructured *,		obj,
	CONST char *,		name,
	CONST char *,		href)
{
    BOOL		present[HTML_A_ATTRIBUTES];
    CONST char * 	value[HTML_A_ATTRIBUTES];
    int i;

    for (i = 0; i < HTML_A_ATTRIBUTES; i++)
	 present[i] = NO;

    if (name && *name) {
    	present[HTML_A_NAME] = YES;
	value[HTML_A_NAME] = (CONST char *)name;
    }
    if (href) {
        present[HTML_A_HREF] = YES;
        value[HTML_A_HREF] = (CONST char *)href;
    }

    (*obj->isa->start_element)(obj, HTML_A, present, value, 0);
}

PUBLIC void HTStartIsIndex ARGS3(
	HTStructured *,		obj,
	CONST char *,		prompt,
	CONST char *,		href)
{
    BOOL		present[HTML_ISINDEX_ATTRIBUTES];
    CONST char * 	value[HTML_ISINDEX_ATTRIBUTES];
    int i;

    for (i = 0; i < HTML_ISINDEX_ATTRIBUTES; i++)
	present[i] = NO;

    if (prompt && *prompt) {
    	present[HTML_ISINDEX_PROMPT] = YES;
	value[HTML_ISINDEX_PROMPT] = (CONST char *)prompt;
    }
    if (href) {
        present[HTML_ISINDEX_HREF] = YES;
        value[HTML_ISINDEX_HREF] = (CONST char *)href;
    }

    (*obj->isa->start_element)(obj, HTML_ISINDEX , present, value, 0);
}
