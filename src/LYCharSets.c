#include "HTUtils.h"
#include "tcp.h"
#include "HTCJK.h"
#include "HTMLDTD.h"

#include "LYGlobalDefs.h"
#include "UCMap.h"
#include "UCDefs.h"
#include "LYCharSets.h"
#include "LYCharUtils.h"
#include "HTFont.h"
#include "GridText.h"
#include "LYCurses.h"
#include "LYStrings.h"

#include "LYexit.h"
#include "LYLeaks.h"

extern BOOL HTPassEightBitRaw;
extern BOOL HTPassEightBitNum;
extern BOOL HTPassHighCtrlRaw;
extern BOOL HTPassHighCtrlNum;
extern HTCJKlang HTCJK;
PUBLIC HTkcode kanji_code = NOKANJI;
PUBLIC BOOLEAN LYHaveCJKCharacterSet = FALSE;
extern void UCInit NOARGS;
extern int UCInitialized;
PUBLIC int LYNumCharsets = 0; /* Will be initialized later by UC_Register. */

/*
 *  INSTRUCTIONS for adding new character sets which do not have
 *		 Unicode tables.
 *
 *  Make up a character set and add it in the same
 *  style as the ISO_LATIN1 set below, giving it a unique name.
 *
 *  Near the end of this file is a place marked "Add your character sets HERE".
 *
 *  Add the name of the set to LYCharSets at the bottom of this file, and
 *  also add it to the LYchar_set_names table below LYCharSets.
 *  Similarly add the appropriate information to LYCharSet_UC and to
 *  LYlowest_eightbit below that.
 *  These tables all MUST have the same order.
 */

/* 	Entity values -- for ISO Latin 1 local representation
**
**	This MUST match exactly the table referred to in the DTD!
*/
PRIVATE char * ISO_Latin1[] = {
	"\306",	/* capital AE diphthong (ligature) (&#198;) - AElig */
	"\301",	/* capital A, acute accent (&#193;) - Aacute */
	"\302",	/* capital A, circumflex accent (&#194;) - Acirc */
	"\300",	/* capital A, grave accent (&#192;) - Agrave */
	"\305",	/* capital A, ring - Aring (&#197;) */
	"\303",	/* capital A, tilde - Atilde (&#195;) */
	"\304",	/* capital A, dieresis or umlaut mark (&#196;) - Auml */
	"\307",	/* capital C, cedilla - Ccedil (&#199;) */
  	"\320",	/* capital Eth or D with stroke (&#208;) - Dstrok */ 
	"\320",	/* capital Eth, Icelandic (&#208;) - ETH */
	"\311",	/* capital E, acute accent (&#201;) - Eacute */
	"\312",	/* capital E, circumflex accent (&#202;) - Ecirc */
	"\310",	/* capital E, grave accent (&#200;) - Egrave */
	"\313",	/* capital E, dieresis or umlaut mark (&#203;) - Euml */
	"\315",	/* capital I, acute accent (&#205;) - Iacute */
	"\316",	/* capital I, circumflex accent (&#206;) - Icirc */
	"\314",	/* capital I, grave accent (&#200;) - Igrave */
	"\317",	/* capital I, dieresis or umlaut mark (&#207;) - Iuml */
	"\321",	/* capital N, tilde (&#209;) - Ntilde */
	"\323",	/* capital O, acute accent (&#211;) - Oacute */
	"\324",	/* capital O, circumflex accent (&#212;) - Ocirc */
	"\322",	/* capital O, grave accent (&#210;) - Ograve */
	"\330",	/* capital O, slash (&#216;) - Oslash */
	"\325",	/* capital O, tilde (&#213;) - Otilde */
	"\326",	/* capital O, dieresis or umlaut mark (&#214;) - Ouml */
	"\336",	/* capital THORN, Icelandic (&#222;) - THORN */
	"\332",	/* capital U, acute accent (&#218;) - Uacute */
	"\333",	/* capital U, circumflex accent (&#219;) - Ucirc */
	"\331",	/* capital U, grave accent (&#217;) - Ugrave */
	"\334",	/* capital U, dieresis or umlaut mark (&#220;) - Uuml */
	"\335",	/* capital Y, acute accent (&#221;) - Yacute */
	"\341",	/* small a, acute accent (&#225;) - aacute */
	"\342",	/* small a, circumflex accent (&#226;) - acirc */
	"\264",	/* spacing acute (&#180;) - acute */
	"\346",	/* small ae diphthong (ligature) (&#230;) - aelig */
	"\340",	/* small a, grave accent (&#224;) - agrave */
	"\046",	/* ampersand (&#38;) - amp */
	"\345",	/* small a, ring (&#229;) - aring */
	"\343",	/* small a, tilde (&#227;) - atilde */
	"\344",	/* small a, dieresis or umlaut mark (&#228;) - auml */
	"\246",	/* broken vertical bar (&#166;) - brkbar */
	"\246",	/* broken vertical bar (&#166;) - brvbar */
	"\347",	/* small c, cedilla (&#231;) - ccedil */
	"\270",	/* spacing cedilla (&#184;) - cedil */
	"\242",	/* cent sign (&#162;) - cent */
	"\251",	/* copyright sign (&#169;) - copy */
	"\244",	/* currency sign (&#164;) - curren */
	"\260",	/* degree sign (&#176;) - deg */
	"\250",	/* spacing diaresis (&#168;) - die */
	"\367",	/* division sign (&#247;) - divide */
	"\351",	/* small e, acute accent (&#233;) - eacute */
	"\352",	/* small e, circumflex accent (&#234;) - ecirc */
	"\350",	/* small e, grave accent (&#232;) - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp, em space - not collapsed NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp, en space - not collapsed NEVER CHANGE THIS - ensp */
	"\360",	/* small eth, Icelandic (&#240;) - eth */
	"\353",	/* small e, dieresis or umlaut mark (&#235;) - euml */
	"\275",	/* fraction 1/2 (&#189;) - frac12 */
	"\274",	/* fraction 1/4 (&#188;) - frac14 */
	"\276",	/* fraction 3/4 (&#190;) - frac34 */
	"\076",	/* greater than (&#62;) - gt */
	"\257",	/* spacing macron (&#175;) - hibar */
	"\355",	/* small i, acute accent (&#237;) - iacute */
	"\356",	/* small i, circumflex accent (&#238;) - icirc */
	"\241",	/* inverted exclamation mark (&#161;) - iexcl */
	"\354",	/* small i, grave accent (&#236;) - igrave */
	"\277",	/* inverted question mark (&#191;) - iquest */
	"\357",	/* small i, dieresis or umlaut mark (&#239;) - iuml */
	"\253",	/* angle quotation mark, left (&#171;) - laquo */
	"\074",	/* less than (&#60;) - lt */
	"\257",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"\265",	/* micro sign (&#181;) - micro */
	"\267",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"\254",	/* negation sign (&#172;) - not */
	"\361",	/* small n, tilde (&#241;) - ntilde */
	"\363",	/* small o, acute accent (&#243;) - oacute */
	"\364",	/* small o, circumflex accent (&#244;) - ocirc */
	"\362",	/* small o, grave accent (&#242;) - ograve */
	"\252",	/* feminine ordinal indicator (&#170;) - ordf */
	"\272",	/* masculine ordinal indicator (&#186;) - ordm */
	"\370",	/* small o, slash (&#248;) - oslash */
	"\365",	/* small o, tilde (&#245;) - otilde */
	"\366",	/* small o, dieresis or umlaut mark (&#246;) - ouml */
	"\266",	/* paragraph sign (&#182;) - para */
	"\261",	/* plus-or-minus sign (&#177;) - plusmn */
	"\243",	/* pound sign (&#163;) - pound */
	"\042",	/* quote '"' (&#34;) - quot */
	"\273",	/* angle quotation mark, right (&#187;) - raquo */
	"\256",	/* circled R registered sign (&#174;) - reg */
	"\247",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"\271",	/* superscript 1 (&#185;) - sup1 */
	"\262",	/* superscript 2 (&#178;) - sup2 */
	"\263",	/* superscript 3 (&#179;) - sup3 */
	"\337",	/* small sharp s, German (sz ligature) (&#223;) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"\376",	/* small thorn, Icelandic (&#254;) - thorn */
	"\327",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"\372",	/* small u, acute accent (&#250;) - uacute */
	"\373",	/* small u, circumflex accent (&#251;) - ucirc */
	"\371",	/* small u, grave accent (&#249;) - ugrave */
	"\250",	/* spacing diaresis (&#168;) - uml */
	"\374",	/* small u, dieresis or umlaut mark (&#252;) - uuml */
	"\375",	/* small y, acute accent (&#253;) - yacute */
	"\245",	/* yen sign (&#165;) - yen */
	"\377",	/* small y, dieresis or umlaut mark (&#255;) - yuml */
};

/* 	Entity values -- for ISO Latin 2 local representation
**
**	This MUST match exactly the table referred to in the DTD!
*/
PRIVATE char * ISO_Latin2[] = {
	"AE",	/* capital AE diphthong (ligature) - AElig */
	"\301",	/* capital A, acute accent - Aacute */
	"\302",	/* capital A, circumflex accent - Acirc */
	"A",	/* capital A, grave accent - Agrave */
	"A",	/* capital A, ring - Aring */
	"A",	/* capital A, tilde - Atilde */
	"\304",	/* capital A, dieresis or umlaut mark - Auml */
	"\307",	/* capital C, cedilla - Ccedil */
	"\320",	/* capital Eth or D with stroke - Dstrok */
	"\320",	/* capital Eth, Icelandic - ETH */
	"\311",	/* capital E, acute accent - Eacute */
	"E",	/* capital E, circumflex accent - Ecirc */
	"E",	/* capital E, grave accent - Egrave */
	"\313",	/* capital E, dieresis or umlaut mark - Euml */
	"\315",	/* capital I, acute accent - Iacute */
	"\316",	/* capital I, circumflex accent - Icirc */
	"I",	/* capital I, grave accent - Igrave */
	"I",	/* capital I, dieresis or umlaut mark - Iuml */
	"N",	/* capital N, tilde - Ntilde */
	"\323",	/* capital O, acute accent - Oacute */
	"\324",	/* capital O, circumflex accent - Ocirc */
	"O",	/* capital O, grave accent - Ograve */
	"O",	/* capital O, slash - Oslash */
	"O",	/* capital O, tilde - Otilde */
	"O",	/* capital O, dieresis or umlaut mark - Ouml */
	"P",	/* capital THORN, Icelandic - THORN */
	"\332",	/* capital U, acute accent - Uacute */
	"U",	/* capital U, circumflex accent - Ucirc */
	"U",	/* capital U, grave accent - Ugrave */
	"\334",	/* capital U, dieresis or umlaut mark - Uuml */
	"\335",	/* capital Y, acute accent - Yacute */
	"\341",	/* small a, acute accent - aacute */
	"\342",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"ae",	/* small ae diphthong (ligature) - aelig */
	"a",	/* small a, grave accent - agrave */
	"\046",	/* ampersand - amp */
	"a",	/* small a, ring - aring */
	"a",	/* small a, tilde - atilde */
	"\344",	/* small a, dieresis or umlaut mark - auml */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"\347",	/* small c, cedilla - ccedil */
	"\270",	/* spacing cedilla (&#184;) - cedil */
	"-c-",	/* cent sign (&#162;) - cent */
	"(c)",	/* copyright sign (&#169;) - copy */
	"\244",	/* currency sign (&#164;) - curren */
	"\260",	/* degree sign (&#176;) - deg */
	"\250",	/* spacing diaresis (&#168;) - die */
	"\367",	/* division sign (&#247;) - divide */
	"\351",	/* small e, acute accent - eacute */
	"e",	/* small e, circumflex accent - ecirc */
	"e",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp, em space - not collapsed NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp, en space - not collapsed NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic - eth */
	"\353",	/* small e, dieresis or umlaut mark - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	"\076",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"\355",	/* small i, acute accent - iacute */
	"\356",	/* small i, circumflex accent - icirc */
	"!",	/* inverted exclamation mark (&#161;) - iexcl */
	"i",	/* small i, grave accent - igrave */
	"?",	/* inverted question mark (&#191;) - iquest */
	"i",	/* small i, dieresis or umlaut mark - iuml */
	"<<",	/* angle quotation mark, left (&#171;) - laquo */
	"\074",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"u",	/* micro sign (&#181;) - micro */
	".",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172;) - not */
	"n",	/* small n, tilde - ntilde */
	"\363",	/* small o, acute accent - oacute */
	"\364",	/* small o, circumflex accent - ocirc */
	"o",	/* small o, grave accent - ograve */
	"-a",	/* feminine ordinal indicator (&#170;) - ordf */
	"-o",	/* masculine ordinal indicator (&#186;) - ordm */
	"o",	/* small o, slash - oslash */
	"o",	/* small o, tilde - otilde */
	"\366",	/* small o, dieresis or umlaut mark - ouml */
	"P:",	/* paragraph sign (&#182;) - para */
	"+-",	/* plus-or-minus sign (&#177;) - plusmn */
	"\243",	/* pound sign (&#163;) - pound */
	"\042",	/* quote '"' - quot */
	">>",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"\247",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"^2",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"\337",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic - thorn */
	"\327",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"\372",	/* small u, acute accent - uacute */
	"u",	/* small u, circumflex accent - ucirc */
	"u",	/* small u, grave accent - ugrave */
	"\250",	/* spacing diaresis (&#168;) - uml */
	"\374",	/* small u, dieresis or umlaut mark - uuml */
	"\375",	/* small y, acute accent - yacute */
	"YEN",	/* yen sign (&#165;) - yen */
	"y",	/* small y, dieresis or umlaut mark - yuml */
};

/*      Entity values -- ISO Latin N > 2
**
**      This MUST match exactly the table referred to in the DTD!
*/
PRIVATE char * ISO_LatinN[] = {
	"AE",	/* capital AE diphthong (ligature) - AElig */
	"A",	/* capital A, acute accent - Aacute */
	"A",	/* capital A, circumflex accent - Acirc */
	"A",	/* capital A, grave accent - Agrave */
	"A",	/* capital A, ring - Aring */
	"A",	/* capital A, tilde - Atilde */
#ifdef LY_UMLAUT
	"Ae",	/* capital A, dieresis or umlaut mark - Auml*/
#else
	"A",	/* capital A, dieresis or umlaut mark - Auml*/
#endif /* LY_UMLAUT */
	"C",	/* capital C, cedilla - Ccedil */
	"Dj",	/* capital D with stroke - Dstrok */
	"DH",	/* capital Eth, Icelandic - ETH */
	"E",	/* capital E, acute accent - Eacute */
	"E",	/* capital E, circumflex accent - Ecirc */
	"E",	/* capital E, grave accent - Egrave */
	"E",	/* capital E, dieresis or umlaut mark - Euml */
	"I",	/* capital I, acute accent - Iacute */
	"I",	/* capital I, circumflex accent - Icirc */
	"I",	/* capital I, grave accent - Igrave */
	"I",	/* capital I, dieresis or umlaut mark - Iuml */
	"N",	/* capital N, tilde - Ntilde */
	"O",	/* capital O, acute accent - Oacute */
	"O",	/* capital O, circumflex accent - Ocirc */
	"O",	/* capital O, grave accent - Ograve */
	"O",	/* capital O, slash - Oslash */
	"O",	/* capital O, tilde - Otilde */
#ifdef LY_UMLAUT
	"Oe",	/* capital O, dieresis or umlaut mark - Ouml */
#else
	"O",	/* capital O, dieresis or umlaut mark - Ouml */
#endif /* LY_UMLAUT */
	"P",	/* capital THORN, Icelandic - THORN */
	"U",	/* capital U, acute accent - Uacute */
	"U",	/* capital U, circumflex accent - Ucirc */
	"U",	/* capital U, grave accent - Ugrave */
#ifdef LY_UMLAUT
	"Ue",	/* capital U, dieresis or umlaut mark - Uuml */
#else
	"U",	/* capital U, dieresis or umlaut mark - Uuml */
#endif /* LY_UMLAUT */
	"Y",	/* capital Y, acute accent - Yacute */
	"a",	/* small a, acute accent - aacute */
	"a",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"ae",	/* small ae diphthong (ligature) - aelig */
	"`a",	/* small a, grave accent - agrave */
	"&",	/* ampersand - amp */
	"a",	/* small a, ring - aring */
	"a",	/* small a, tilde - atilde */
#ifdef LY_UMLAUT
	"ae",	/* small a, dieresis or umlaut mark - auml */
#else
	"a",	/* small a, dieresis or umlaut mark - auml */
#endif /* LY_UMLAUT */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"c",	/* small c, cedilla - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"-c-",	/* cent sign (&#162;) - cent */
	"(c)",	/* copyright sign (&#169;) - copy */
	"CUR",	/* currency sign (&#164;) - curren */
	"DEG",	/* degree sign (&#176;) - deg */
	"\042",	/* spacing diaresis (&#168;) - die */
	"/",	/* division sign (&#247;) - divide */
	"e",	/* small e, acute accent - eacute */
	"e",	/* small e, circumflex accent - ecirc */
	"e",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic eth */
	"e",	/* small e, dieresis or umlaut mark - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	">",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"i",	/* small i, acute accent - iacute */
	"i",	/* small i, circumflex accent - icirc*/
	"!",	/* inverted exclamation mark (&#161;) - iexcl */
	"`i",	/* small i, grave accent - igrave */
	"?",	/* inverted question mark (&#191;) - iquest */
	"i",	/* small i, dieresis or umlaut mark - iuml */
	"<<",	/* angle quotation mark, left (&#171;) - laquo */
	"<",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"u",	/* micro sign (&#181;) - micro */
	".",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172;) - not */
	"n",	/* small n, tilde - ntilde */
	"o",	/* small o, acute accent - oacute */
	"o",	/* small o, circumflex accent - ocirc */
	"o",	/* small o, grave accent - ograve */
	"-a",	/* feminine ordinal indicator (&#170;) - ordf */
	"-o",	/* masculine ordinal indicator (&#186;) - ordm */
	"o",	/* small o, slash - oslash */
	"o",	/* small o, tilde - otilde */
#ifdef LY_UMLAUT
	"oe",	/* small o, dieresis or umlaut mark - ouml */
#else
	"o",	/* small o, dieresis or umlaut mark - ouml */
#endif /* LY_UMLAUT */
	"P:",	/* paragraph sign (&#182;) - para */
	"+-",	/* plus-or-minus sign (&#177;) - plusmn */
	"-L-",	/* pound sign (&#163;) - pound */
	"\"",	/* quote '"' - quot */
	">>",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"S:",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"^2",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"ss",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"u",	/* small u, acute accent - uacute */
	"u",	/* small u, circumflex accent - ucirc */
	"u",	/* small u, grave accent - ugrave */
	"\042",	/* spacing diaresis (&#168;) - uml */
#ifdef LY_UMLAUT
	"ue",	/* small u, dieresis or umlaut mark - uuml */
#else
	"u",	/* small u, dieresis or umlaut mark - uuml */
#endif /* LY_UMLAUT */
	"y",	/* small y, acute accent - yacute */
	"YEN",	/* yen sign (&#165;) - yen */
	"y",	/* small y, dieresis or umlaut mark - yuml */
};

/*      Entity values -- DEC Multinational
**
**      This MUST match exactly the table referred to in the DTD!
*/
PRIVATE char * DEC_Multinational[] = {
	"\306",	/* capital AE diphthong (ligature) - AElig */
	"\301",	/* capital A, acute accent - Aacute */
	"\302",	/* capital A, circumflex accent - Acirc */
	"\300",	/* capital A, grave accent - Agrave */
	"\305",	/* capital A, ring - Aring */
	"\303",	/* capital A, tilde - Atilde */
	"\304",	/* capital A, dieresis or umlaut mark - Auml */
	"\307",	/* capital C, cedilla - Ccedil */
	"Dj",	/* capital D with stroke - Dstrok */
	"DH",	/* capital Eth, Icelandic - ETH */
	"\311",	/* capital E, acute accent - Eacute */
	"\312",	/* capital E, circumflex accent - Ecirc */
	"\310",	/* capital E, grave accent - Egrave */
	"\313",	/* capital E, dieresis or umlaut mark - Euml */
	"\315",	/* capital I, acute accent - Iacute */
	"\316",	/* capital I, circumflex accent - Icirc */
	"\314",	/* capital I, grave accent - Igrave */
	"\317",	/* capital I, dieresis or umlaut mark - Iuml */
	"\321",	/* capital N, tilde - Ntilde */
	"\323",	/* capital O, acute accent - Oacute */
	"\324",	/* capital O, circumflex accent - Ocirc */
	"\322",	/* capital O, grave accent - Ograve */
	"\330",	/* capital O, slash - Oslash */
	"\325",	/* capital O, tilde - Otilde */
	"\326",	/* capital O, dieresis or umlaut mark - Ouml */
	"P",	/* capital THORN, Icelandic - THORN */
	"\332",	/* capital U, acute accent - Uacute */
	"\333",	/* capital U, circumflex accent - Ucirc */
	"\331",	/* capital U, grave accent - Ugrave */
	"\334",	/* capital U, dieresis or umlaut mark - Uuml */
	"\335",	/* capital Y, acute accent - Yacute */
	"\341",	/* small a, acute accent - aacute */
	"\342",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"\346",	/* small ae diphthong (ligature) - aelig */
	"\340",	/* small a, grave accent - agrave */
	"\046",	/* ampersand - amp */
	"\345",	/* small a, ring - aring */
	"\343",	/* small a, tilde - atilde */
	"\344",	/* small a, dieresis or umlaut mark - auml */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"\347",	/* small c, cedilla - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"\242",	/* cent sign (&#162;) - cent */
	"\251",	/* copyright sign (&#169;) - copy */
	"CUR",	/* currency sign (&#164;) - curren */
	"\260",	/* degree sign (&#176;) - deg */
	"\250",	/* spacing diaresis (&#168;) - die */
	"/",	/* division sign (&#247;) - divide */
	"\351",	/* small e, acute accent - eacute */
	"\352",	/* small e, circumflex accent - ecirc */
	"\350",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp, em space - not collapsed NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp, en space - not collapsed NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic - eth */
	"\353",	/* small e, dieresis or umlaut mark - euml */
	"\275",	/* fraction 1/2 (&#189;) - frac12 */
	"\274",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	"\076",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"\355",	/* small i, acute accent - iacute */
	"\356",	/* small i, circumflex accent - icirc */
	"\241",	/* inverted exclamation mark (&#161;) - iexcl */
	"\354",	/* small i, grave accent - igrave */
	"\277",	/* inverted question mark (&#191;) - iquest */
	"\357",	/* small i, dieresis or umlaut mark - iuml */
	"\253",	/* angle quotation mark, left (&#171;) - laquo */
	"\074",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"\265",	/* micro sign (&#181;) - micro */
	"\267",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172); - not */
	"\361",	/* small n, tilde - ntilde */
	"\363",	/* small o, acute accent - oacute */
	"\364",	/* small o, circumflex accent - ocirc */
	"\362",	/* small o, grave accent - ograve */
	"\252",	/* feminine ordinal indicator (&#170;) - ordf */
	"\272",	/* masculine ordinal indicator (&#186;) - ordm */
	"\370",	/* small o, slash - oslash */
	"\365",	/* small o, tilde - otilde */
	"\366",	/* small o, dieresis or umlaut mark - ouml */
	"\266",	/* paragraph sign (&#182;) - para */
	"\261",	/* plus-or-minus sign (&#177;) - plusmn */
	"\243",	/* pound sign (&#163;) - pound */
	"\042",	/* quote '"' - quot */
	"\273",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"\247",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"\271",	/* superscript 1 (&#185;) - sup1 */
	"\262",	/* superscript 2 (&#178;) - sup2 */
	"\263",	/* superscript 3 (&#179;) - sup3 */
	"\337",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"\372",	/* small u, acute accent - uacute */
	"\373",	/* small u, circumflex accent - ucirc */
	"\371",	/* small u, grave accent - ugrave */
	"\250",	/* spacing diaresis (&#168;) - uml */
	"\374",	/* small u, dieresis or umlaut mark - uuml */
	"y'",	/* small y, acute accent - yacute */
	"\245",	/* yen sign (&#165;) - yen */
	"\375",	/* small y, dieresis or umlaut mark - yuml */
};

/*
**      Entity values -- regular PC charset
**	(codepage 437 - hardware default on most US market PCs)
**	(almost the same as Windows font "Terminal")
**
**      updated January 10, 1996  mike@hyperreal.com
*/
PRIVATE char * PC_charset[] = {
	"\222",	/* capital AE diphthong (ligature) - AElig */
	"A",	/* capital A, acute accent - Aacute */
	"A",	/* capital A, circumflex accent - Acirc */
	"A",	/* capital A, grave accent - Agrave */
	"\217",	/* capital A, ring - Aring */
	"A",	/* capital A, tilde - Atilde */
	"\216",	/* capital A, dieresis or umlaut mark - Auml */
	"\200",	/* capital C, cedilla - Ccedil */
	"Dj",	/* capital D with stroke - Dstrok */
	"DH",	/* capital Eth, Icelandic - ETH */
	"\220",	/* capital E, acute accent - Eacute */
	"E",	/* capital E, circumflex accent - Ecirc */
	"E",	/* capital E, grave accent - Egrave */
	"E",	/* capital E, dieresis or umlaut mark - Euml */
	"I",	/* capital I, acute accent - Iacute */
	"I",	/* capital I, circumflex accent - Icirc */
	"I",	/* capital I, grave accent - Igrave */
	"I",	/* capital I, dieresis or umlaut mark - Iuml */
	"\245",	/* capital N, tilde - Ntilde */
	"O",	/* capital O, acute accent - Oacute */
	"O",	/* capital O, circumflex accent - Ocirc */
	"O",	/* capital O, grave accent - Ograve */
	"O",	/* capital O, slash - Oslash */
	"O",	/* capital O, tilde - Otilde */
	"\231",	/* capital O, dieresis or umlaut mark - Ouml */
	"\350",	/* capital THORN, Icelandic - THORN */
	"U",	/* capital U, acute accent - Uacute */
	"U",	/* capital U, circumflex accent - Ucirc */
	"U",	/* capital U, grave accent - Ugrave */
	"\232",	/* capital U, dieresis or umlaut mark - Uuml */
	"Y",	/* capital Y, acute accent - Yacute */
	"\240",	/* small a, acute accent - aacute */
	"\203",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"\221",	/* small ae diphthong (ligature) - aelig */
	"\205",	/* small a, grave accent - agrave */
	"\046",	/* ampersand - amp */
	"\206",	/* small a, ring - aring */
	"a",	/* small a, tilde - atilde */
	"\204",	/* small a, dieresis or umlaut mark - auml */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"\207",	/* small c, cedilla - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"\233",	/* cent sign (&#162;) - cent */
	"(c)",	/* copyright sign (&#169;) - copy */
	"CUR",	/* currency sign (&#164;) - curren */
	"\370",	/* degree sign (&#176;) - deg */
	"\042",	/* spacing diaresis (&#168;) - die */
	"\366",	/* division sign (&#247;) - divide */
	"\202",	/* small e, acute accent - eacute */
	"\210",	/* small e, circumflex accent - ecirc */
	"\212",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic - eth */
	"\211",	/* small e, dieresis or umlaut mark - euml */
	"\253",	/* fraction 1/2 (&#189;) - frac12 */
	"\254",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	"\076",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"\241",	/* small i, acute accent - iacute */
	"\214",	/* small i, circumflex accent - icirc */
	"\255",	/* inverted exclamation mark (&#161;) - iexcl */
	"\215",	/* small i, grave accent - igrave */
	"\250",	/* inverted question mark (&#191;) - iquest */
	"\213",	/* small i, dieresis or umlaut mark - iuml */
	"\256",	/* angle quotation mark, left (&#171;) - laquo */
	"\074",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"\346",	/* micro sign (&#181;) - micro */
	"\372",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"\252",	/* negation sign (&#172;) - not */
	"\244",	/* small n, tilde - ntilde */
	"\242",	/* small o, acute accent - oacute */
	"\223",	/* small o, circumflex accent - ocirc */
	"\225",	/* small o, grave accent - ograve */
	"\246",	/* feminine ordinal indicator (&#170;) - ordf */
	"\247",	/* masculine ordinal indicator (&#186;) - ordm */
	"\355",	/* small o, slash - oslash */  /* Greek letter substitute */
	"o",	/* small o, tilde - otilde */
	"\224",	/* small o, dieresis or umlaut mark - ouml */
	"P:",	/* paragraph sign (&#182;) - para */
	"\361",	/* plus-or-minus sign (&#177;) - plusmn */
	"\234",	/* pound sign (&#163;) - pound */
	"\042",	/* quote '"' - quot */
	"\257",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"S:",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"\375",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"\341",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"\347",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"\243",	/* small u, acute accent - uacute */
	"\226",	/* small u, circumflex accent - ucirc */
	"\227",	/* small u, grave accent - ugrave */
	"\042",	/* spacing diaresis (&#168;) - uml */
	"\201",	/* small u, dieresis or umlaut mark - uuml */
	"y'",	/* small y, acute accent - yacute */
	"\235",	/* yen sign (&#165;) - yen */
	"\230",	/* small y, dieresis or umlaut mark - yuml */
};

/*
**      Entity values - ISO-Latin PC charset
**	(MS-DOS codepage 850)
**	(based on a translation table by Tor Slettnes)
**
**      January 10, 1996  mike@hyperreal.com
*/
PRIVATE char * PC_850_charset[] = {
	"\222",	/* capital AE diphthong (ligature) - AElig */
	"\265",	/* capital A, acute accent - Aacute */
	"\266",	/* capital A, circumflex accent - Acirc */
	"\267",	/* capital A, grave accent - Agrave */
	"\217",	/* capital A, ring - Aring */
	"\307",	/* capital A, tilde - Atilde */
	"\216",	/* capital A, dieresis or umlaut mark - Auml */
	"\200",	/* capital C, cedilla - Ccedil */
	"\321",	/* capital Eth or D with stroke - Dstrok */
	"\321",	/* capital Eth, Icelandic - ETH */
	"\220",	/* capital E, acute accent - Eacute */
	"\322",	/* capital E, circumflex accent - Ecirc */
	"\324",	/* capital E, grave accent - Egrave */
	"\323",	/* capital E, dieresis or umlaut mark - Euml */
	"\326",	/* capital I, acute accent - Iacute */
	"\327",	/* capital I, circumflex accent - Icirc */
	"\336",	/* capital I, grave accent - Igrave */
	"\330",	/* capital I, dieresis or umlaut mark - Iuml */
	"\245",	/* capital N, tilde - Ntilde */
	"\340",	/* capital O, acute accent - Oacute */
	"\342",	/* capital O, circumflex accent - Ocirc */
	"\343",	/* capital O, grave accent - Ograve */
	"\235",	/* capital O, slash - Oslash */
	"\345",	/* capital O, tilde - Otilde */
	"\231",	/* capital O, dieresis or umlaut mark - Ouml */
	"\350",	/* capital THORN, Icelandic - THORN */
	"\351",	/* capital U, acute accent - Uacute */
	"\352",	/* capital U, circumflex accent - Ucirc */
	"\353",	/* capital U, grave accent - Ugrave */
	"\232",	/* capital U, dieresis or umlaut mark - Uuml */
	"\355",	/* capital Y, acute accent - Yacute */
	"\240",	/* small a, acute accent - aacute */
	"\203",	/* small a, circumflex accent - acirc */
	"\357",	/* spacing acute (&#180;) - acute */
	"\221",	/* small ae diphthong (ligature) - aelig */
	"\205",	/* small a, grave accent - agrave */
	"\046",	/* ampersand - amp */
	"\206",	/* small a, ring - aring */
	"\306",	/* small a, tilde - atilde */
	"\204",	/* small a, dieresis or umlaut mark - auml */
	"\335",	/* broken vertical bar (&#166;) - brkbar */
	"\335",	/* broken vertical bar (&#166;) - brvbar */
	"\207",	/* small c, cedilla - ccedil */
	"\367",	/* spacing cedilla (&#184;) - cedil */
	"\275",	/* cent sign (&#162;) - cent */
	"\270",	/* copyright sign (&#169;) - copy */
	"\317",	/* currency sign (&#164;) - curren */
	"\370",	/* degree sign (&#176;) - deg */
	"\371",	/* spacing diaresis (&#168;) - die */
	"\366",	/* division sign (&#247;) - divide */
	"\202",	/* small e, acute accent - eacute */
	"\210",	/* small e, circumflex accent - ecirc */
	"\212",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp NEVER CHANGE THIS - ensp */
	"\320",	/* small eth, Icelandic - eth */
	"\211",	/* small e, dieresis or umlaut mark - euml */
	"\253",	/* fraction 1/2 (&#189;) - frac12 */
	"\254",	/* fraction 1/4 (&#188;) - frac14 */
	"\363",	/* fraction 3/4 (&#190;) - frac34 */
	"\076",	/* greater than - gt */
	"\356",	/* spacing macron (&#175;) - hibar */
	"\241",	/* small i, acute accent - iacute */
	"\214",	/* small i, circumflex accent - icirc */
	"\255",	/* inverted exclamation mark (&#161;) - iexcl */
	"\215",	/* small i, grave accent - igrave */
	"\250",	/* inverted question mark (&#191;) - iquest */
	"\213",	/* small i, dieresis or umlaut mark - iuml */
	"\256",	/* angle quotation mark, left (&#171;) - laquo */
	"\074",	/* less than - lt */
	"\356",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"\346",	/* micro sign (&#181;) - micro */
	"\372",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"\252",	/* negation sign (&#172;) - not */
	"\244",	/* small n, tilde - ntilde */
	"\242",	/* small o, acute accent - oacute */
	"\223",	/* small o, circumflex accent - ocirc */
	"\225",	/* small o, grave accent - ograve */
	"\246",	/* feminine ordinal indicator (&#170;) - ordf */
	"\247",	/* masculine ordinal indicator (&#186;) - ordm */
	"\233",	/* small o, slash - oslash */
	"\344",	/* small o, tilde - otilde */
	"\224",	/* small o, dieresis or umlaut mark - ouml */
	"\364",	/* paragraph sign (&#182;) - para */
	"\361",	/* plus-or-minus sign (&#177;) - plusmn */
	"\234",	/* pound sign (&#163;) - pound */
	"\042",	/* quote '"' - quot */
	"\257",	/* angle quotation mark, right (&#187;) - raquo */
	"\251",	/* circled R registered sign (&#174;) - reg */
	"\365",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"\373",	/* superscript 1 (&#185;) - sup1 */
	"\375",	/* superscript 2 (&#178;) - sup2 */
	"\374",	/* superscript 3 (&#179;) - sup3 */
	"\341",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"\347",	/* small thorn, Icelandic - thorn */
	"\236",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"\243",	/* small u, acute accent - uacute */
	"\226",	/* small u, circumflex accent - ucirc */
	"\227",	/* small u, grave accent - ugrave */
	"\371",	/* spacing diaresis (&#168;) - uml */
	"\201",	/* small u, dieresis or umlaut mark - uuml */
	"\354",	/* small y, acute accent - yacute */
	"\276",	/* yen sign (&#165;) - yen */
	"\230",	/* small y, dieresis or umlaut mark - yuml */
};

/*
**      Entity values -- 8 bit Apple Macintosh.
**	(Assumes regular PostScript-style font)
**
**      November 6 1995. nkg@freenet.vancouver.bc.ca
*/
PRIVATE char * Macintosh[] = {
	"\256",	/* capital AE diphthong (ligature) - AElig */
	"\347",	/* capital A, acute accent - Aacute */
	"\345",	/* capital A, circumflex accent - Acirc */
	"\313",	/* capital A, grave accent - Agrave */
	"\201",	/* capital A, ring - Aring */
	"\314",	/* capital A, tilde - Atilde */
	"\200",	/* capital A, dieresis or umlaut mark - Auml */
	"\202",	/* capital C, cedilla - Ccedil */
	"Dj",	/* capital D with stroke - Dstrok */
	"DH",	/* capital Eth, Icelandic - ETH */
	"\203",	/* capital E, acute accent - Eacute */
	"\346",	/* capital E, circumflex accent - Ecirc */
	"\351",	/* capital E, grave accent - Egrave */
	"\350",	/* capital E, dieresis or umlaut mark - Euml */
	"\352",	/* capital I, acute accent - Iacute */
	"\353",	/* capital I, circumflex accent - Icirc */
	"\355",	/* capital I, grave accent - Igrave */
	"\354",	/* capital I, dieresis or umlaut mark - Iuml */
	"\204",	/* capital N, tilde - Ntilde */
	"\356",	/* capital O, acute accent - Oacute */
	"\357",	/* capital O, circumflex accent - Ocirc */
	"\361",	/* capital O, grave accent - Ograve */
	"\257",	/* capital O, slash - Oslash */
	"\315",	/* capital O, tilde - Otilde */
	"\205",	/* capital O, dieresis or umlaut mark - Ouml */
	"P",	/* capital THORN, Icelandic - THORN */
	"\362",	/* capital U, acute accent - Uacute */
	"\363",	/* capital U, circumflex accent - Ucirc */
	"\364",	/* capital U, grave accent - Ugrave */
	"\206",	/* capital U, dieresis or umlaut mark - Uuml */
	"Y'",	/* capital Y, acute accent - Yacute */
	"\207",	/* small a, acute accent - aacute */
	"\211",	/* small a, circumflex accent - acirc */
	"\253",	/* spacing acute (&#180;) - acute */
	"\276",	/* small ae diphthong (ligature) - aelig */
	"\210",	/* small a, grave accent - agrave */
	"\046",	/* ampersand - amp */
	"\214",	/* small a, ring - aring */
	"\213",	/* small a, tilde - atilde */
	"\212",	/* small a, dieresis or umlaut mark - auml */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"\215",	/* small c, cedilla - ccedil */
	"\374",	/* spacing cedilla (&#184;) - cedil */
	"\242",	/* cent sign (&#162;) - cent */
	"\251",	/* copyright sign (&#169;) - copy */
	"\333",	/* currency sign (&#164;) - curren */
	"\241",	/* degree sign (&#176;) - deg */
	"\254",	/* spacing diaresis (&#168;) - die */
	"\326",	/* division sign (&#247;) - divide */
	"\216",	/* small e, acute accent - eacute */
	"\220",	/* small e, circumflex accent - ecirc */
	"\217",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp, em space - not collapsed NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp, en space - not collapsed NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic - eth */
	"\221",	/* small e, dieresis or umlaut mark - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	"\076",	/* greater than - gt */
	"\370",	/* spacing macron (&#175;) - hibar */
	"\222",	/* small i, acute accent - iacute */
	"\224",	/* small i, circumflex accent - icirc */
	"\301",	/* inverted exclamation mark (&#161;) - iexcl */
	"\223",	/* small i, grave accent - igrave */
	"\300",	/* inverted question mark (&#191;) - iquest */
	"\225",	/* small i, dieresis or umlaut mark - iuml */
	"\307",	/* angle quotation mark, left (&#171;) - laquo */
	"\074",	/* less than - lt */
	"\370",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"\265",	/* micro sign (&#181;) - micro */
	"\245",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"\302",	/* negation sign (&#172;) - not */
	"\226",	/* small n, tilde - ntilde */
	"\227",	/* small o, acute accent - oacute */
	"\231",	/* small o, circumflex accent - ocirc */
	"\230",	/* small o, grave accent - ograve */
	"\273",	/* feminine ordinal indicator (&#170;) - ordf */
	"\274",	/* masculine ordinal indicator (&#186;) - ordm */
	"\277",	/* small o, slash - oslash */
	"\233",	/* small o, tilde - otilde */
	"\232",	/* small o, dieresis or umlaut mark - ouml */
	"\246",	/* paragraph sign (&#182;) - para */
	"\261",	/* plus-or-minus sign (&#177;) - plusmn */
	"\243",	/* pound sign (&#163;) - pound */
	"\042",	/* quote '"' - quot */
	"\310",	/* angle quotation mark, right (&#187;) - raquo */
	"\250",	/* circled R registered sign (&#174;) - reg */
	"\244",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"^2",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"\247",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"\252",	/* circled TM trade mark sign (&#8482;) - trade */
	"\234",	/* small u, acute accent - uacute */
	"\236",	/* small u, circumflex accent - ucirc */
	"\235",	/* small u, grave accent - ugrave*/
	"\254",	/* spacing diaresis (&#168;) - uml */
	"\237",	/* small u, dieresis or umlaut mark - uuml */
	"y'",	/* small y, acute accent - yacute*/
	"\264",	/* yen sign (&#165;) - yen */
	"\330",	/* small y, dieresis or umlaut mark - yuml */
};

/*      Entity values -- NeXT
**
**      This MUST match exactly the table referred to in the DTD!
*/
PRIVATE char * NeXT_Step[] = {
	"\341",	/* capital AE diphthong (ligature) - AElig */
	"\202",	/* capital A, acute accent - Aacute */
	"\203",	/* capital A, circumflex accent - Acirc */
	"\201",	/* capital A, grave accent - Agrave */
	"\206",	/* capital A, ring - Aring */
	"\204",	/* capital A, tilde - Atilde */
	"\205",	/* capital A, dieresis or umlaut mark - Auml */
	"\207",	/* capital C, cedilla - Ccedil */
	"\220",	/* capital Eth or D with stroke - Dstrok */
	"\220",	/* capital Eth, Icelandic - ETH */
	"\211",	/* capital E, acute accent - Eacute */
	"\212",	/* capital E, circumflex accent - Ecirc */
	"\210",	/* capital E, grave accent - Egrave */
	"\213",	/* capital E, dieresis or umlaut mark - Euml */
	"\215",	/* capital I, acute accent - Iacute */
	"\216",	/* capital I, circumflex accent - Icirc */
	"\214",	/* capital I, grave accent - Igrave */
	"\217",	/* capital I, dieresis or umlaut mark - Iuml */
	"\221",	/* capital N, tilde - Ntilde */
	"\223",	/* capital O, acute accent - Oacute */
	"\224",	/* capital O, circumflex accent - Ocirc */
	"\222",	/* capital O, grave accent - Ograve */
	"\351",	/* capital O, slash - Oslash */
	"\225",	/* capital O, tilde - Otilde */
	"\226",	/* capital O, dieresis or umlaut mark - Ouml */
	"\234",	/* capital THORN, Icelandic - THORN */
	"\230",	/* capital U, acute accent - Uacute */
	"\231",	/* capital U, circumflex accent - Ucirc */
	"\227",	/* capital U, grave accent - Ugrave */
	"\232",	/* capital U, dieresis or umlaut mark - Uuml */
	"\233",	/* capital Y, acute accent - Yacute */
	"\326",	/* small a, acute accent - aacute */
	"\327",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"\361",	/* small ae diphthong (ligature) - aelig */
	"\325",	/* small a, grave accent - agrave */
	"\046",	/* ampersand - amp */
	"\332",	/* small a, ring - aring */
	"\330",	/* small a, tilde - atilde */
	"\331",	/* small a, dieresis or umlaut mark - auml */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"\333",	/* small c, cedilla - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"-c-",	/* cent sign (&#162;) - cent */
	"(c)",	/* copyright sign (&#169;) - copy */
	"CUR",	/* currency sign (&#164;) - curren */
	"DEG",	/* degree sign (&#176;) - deg */
	"\042",	/* spacing diaresis (&#168;) - die */
	"/",	/* division sign (&#247;) - divide */
	"\335",	/* small e, acute accent - eacute */
	"\336",	/* small e, circumflex accent - ecirc */
	"\334",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp, em space - not collapsed NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp, en space - not collapsed NEVER CHANGE THIS - ensp */
	"\346",	/* small eth, Icelandic - eth */
	"\337",	/* small e, dieresis or umlaut mark - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	"\076",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"\342",	/* small i, acute accent - iacute */
	"\344",	/* small i, circumflex accent - icirc */
	"!",	/* inverted exclamation mark (&#161;) - iexcl */
	"\340",	/* small i, grave accent - igrave */
	"?",	/* inverted question mark (&#191;) - iquest */
	"\345",	/* small i, dieresis or umlaut mark - iuml */
	"<<",	/* angle quotation mark, left (&#171;) - laquo */
	"\074",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"u",	/* micro sign (&#181;) - micro */
	".",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172;) - not */
	"\347",	/* small n, tilde - ntilde */
	"\355",	/* small o, acute accent - oacute */
	"\356",	/* small o, circumflex accent - ocirc */
	"\354",	/* small o, grave accent - ograve */
	"-a",	/* feminine ordinal indicator (&#170;) - ordf */
	"-o",	/* masculine ordinal indicator (&#186;) - ordm */
	"\371",	/* small o, slash - oslash */
	"\357",	/* small o, tilde - otilde */
	"\360",	/* small o, dieresis or umlaut mark - ouml */
	"P:",	/* paragraph sign (&#182;) - para */
	"+-",	/* plus-or-minus sign (&#177;) - plusmn */
	"-L-",	/* pound sign (&#163;) - pound */
	"\042",	/* quote '"' - quot */
	">>",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"S:",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"^2",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"\373",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"\374",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"\363",	/* small u, acute accent - uacute */
	"\364",	/* small u, circumflex accent - ucirc */
	"\362",	/* small u, grave accent - ugrave*/
	"\042",	/* spacing diaresis (&#168;) - uml */
	"\366",	/* small u, dieresis or umlaut mark - uuml */
	"\367",	/* small y, acute accent - yacute*/
	"YEN",	/* yen sign (&#165;) - yen */
	"\375",	/* small y, dieresis or umlaut mark - yuml */
};

/*      Entity values -- 8 bit Russian KOI8-R
**
**      This MUST match exactly the table referred to in the DTD!
**      Dec 11, 1995, ache@astral.msk.su
*/
PRIVATE char * KOI8_R[] = {
	"AE",	/* capital AE diphthong (ligature) - AElig */
	"A",	/* capital A, acute accent - Aacute */
	"A",	/* capital A, circumflex accent - Acirc */
	"A",	/* capital A, grave accent - Agrave */
	"A",	/* capital A, ring - Aring */
	"A",	/* capital A, tilde - Atilde */
#ifdef LY_UMLAUT
	"Ae",	/* capital A, dieresis or umlaut mark - Auml*/
#else
	"A",	/* capital A, dieresis or umlaut mark - Auml*/
#endif /* LY_UMLAUT */
	"C",	/* capital C, cedilla - Ccedil */
	"Dj",	/* capital D with stroke - Dstrok */
	"DH",	/* capital Eth, Icelandic - ETH */
	"E",	/* capital E, acute accent - Eacute */
	"E",	/* capital E, circumflex accent - Ecirc */
	"E",	/* capital E, grave accent - Egrave */
	"\263",	/* capital E, dieresis or umlaut mark - Euml */
	"I",	/* capital I, acute accent - Iacute */
	"I",	/* capital I, circumflex accent - Icirc */
	"I",	/* capital I, grave accent - Igrave */
	"I",	/* capital I, dieresis or umlaut mark - Iuml */
	"N",	/* capital N, tilde - Ntilde */
	"O",	/* capital O, acute accent - Oacute */
	"O",	/* capital O, circumflex accent - Ocirc */
	"O",	/* capital O, grave accent - Ograve */
	"O",	/* capital O, slash - Oslash */
	"O",	/* capital O, tilde - Otilde */
#ifdef LY_UMLAUT
	"Oe",	/* capital O, dieresis or umlaut mark - Ouml */
#else
	"O",	/* capital O, dieresis or umlaut mark - Ouml */
#endif /* LY_UMLAUT */
	"P",	/* capital THORN, Icelandic - THORN */
	"U",	/* capital U, acute accent - Uacute */
	"U",	/* capital U, circumflex accent - Ucirc */
	"U",	/* capital U, grave accent - Ugrave */
#ifdef LY_UMLAUT
	"Ue",	/* capital U, dieresis or umlaut mark - Uuml */
#else
	"U",	/* capital U, dieresis or umlaut mark - Uuml */
#endif /* LY_UMLAUT */
	"Y",	/* capital Y, acute accent - Yacute */
	"a",	/* small a, acute accent - aacute */
	"a",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"ae",	/* small ae diphthong (ligature) - aelig */
	"`a",	/* small a, grave accent - agrave */
	"&",	/* ampersand - amp */
	"a",	/* small a, ring - aring */
	"a",	/* small a, tilde - atilde */
#ifdef LY_UMLAUT
	"ae",	/* small a, dieresis or umlaut mark - auml */
#else
	"a",	/* small a, dieresis or umlaut mark - auml */
#endif /* LY_UMLAUT */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"c",	/* small c, cedilla - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"-c-",	/* cent sign (&#162;) - cent */
	"\277",	/* copyright sign (&#169;) - copy */
	"$",	/* currency sign (&#164;) - curren */
	"\234",	/* degree sign (&#176;) - deg */
	"\042",	/* spacing diaresis (&#168;) - die */
	"\237",	/* division sign (&#247;) - divide */
	"e",	/* small e, acute accent - eacute */
	"e",	/* small e, circumflex accent - ecirc */
	"e",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic eth */
	"\243",	/* small e, dieresis or umlaut mark - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	">",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"i",	/* small i, acute accent - iacute */
	"i",	/* small i, circumflex accent - icirc*/
	"!",	/* inverted exclamation mark (&#161;) - iexcl */
	"`i",	/* small i, grave accent - igrave */
	"?",	/* inverted question mark (&#191;) - iquest */
	"i",	/* small i, dieresis or umlaut mark - iuml */
	"<<",	/* angle quotation mark, left (&#171;) - laquo */
	"<",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"u",	/* micro sign (&#181;) - micro */
	"\236",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172;) - not */
	"n",	/* small n, tilde - ntilde */
	"o",	/* small o, acute accent - oacute */
	"o",	/* small o, circumflex accent - ocirc */
	"o",	/* small o, grave accent - ograve */
	"-a",	/* feminine ordinal indicator (&#170;) - ordf */
	"-o",	/* masculine ordinal indicator (&#186;) - ordm */
	"o",	/* small o, slash - oslash */
	"o",	/* small o, tilde - otilde */
#ifdef LY_UMLAUT
	"oe",	/* small o, dieresis or umlaut mark - ouml */
#else
	"o",	/* small o, dieresis or umlaut mark - ouml */
#endif /* LY_UMLAUT */
	"P:",	/* paragraph sign (&#182;) - para */
	"+-",	/* plus-or-minus sign (&#177;) - plusmn */
	"-L-",	/* pound sign (&#163;) - pound */
	"\"",	/* quote '"' - quot */
	">>",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"S:",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"\235",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"ss",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"u",	/* small u, acute accent - uacute */
	"u",	/* small u, circumflex accent - ucirc */
	"u",	/* small u, grave accent - ugrave */
	"\042",	/* spacing diaresis (&#168;) - uml */
#ifdef LY_UMLAUT
	"ue",	/* small u, dieresis or umlaut mark - uuml */
#else
	"u",	/* small u, dieresis or umlaut mark - uuml */
#endif /* LY_UMLAUT */
	"y",	/* small y, acute accent - yacute */
	"YEN",	/* yen sign (&#165;) - yen */
	"y",	/* small y, dieresis or umlaut mark - yuml */
};

/*      Entity values -- Chinese (EUC)
**
**      This MUST match exactly the table referred to in the DTD!
*/
PRIVATE char * Chinese[] = {
	"AE",	/* capital AE diphthong (ligature) - AElig */
	"A",	/* capital A, acute accent - Aacute */
	"A",	/* capital A, circumflex accent - Acirc */
	"A",	/* capital A, grave accent - Agrave */
	"A",	/* capital A, ring - Aring */
	"A",	/* capital A, tilde - Atilde */
#ifdef LY_UMLAUT
	"Ae",	/* capital A, dieresis or umlaut mark - Auml*/
#else
	"A",	/* capital A, dieresis or umlaut mark - Auml*/
#endif /* LY_UMLAUT */
	"C",	/* capital C, cedilla - Ccedil */
	"Dj",	/* capital D with stroke - Dstrok */
	"DH",	/* capital Eth, Icelandic - ETH */
	"E",	/* capital E, acute accent - Eacute */
	"E",	/* capital E, circumflex accent - Ecirc */
	"E",	/* capital E, grave accent - Egrave */
	"E",	/* capital E, dieresis or umlaut mark - Euml */
	"I",	/* capital I, acute accent - Iacute */
	"I",	/* capital I, circumflex accent - Icirc */
	"I",	/* capital I, grave accent - Igrave */
	"I",	/* capital I, dieresis or umlaut mark - Iuml */
	"N",	/* capital N, tilde - Ntilde */
	"O",	/* capital O, acute accent - Oacute */
	"O",	/* capital O, circumflex accent - Ocirc */
	"O",	/* capital O, grave accent - Ograve */
	"O",	/* capital O, slash - Oslash */
	"O",	/* capital O, tilde - Otilde */
#ifdef LY_UMLAUT
	"Oe",	/* capital O, dieresis or umlaut mark - Ouml */
#else
	"O",	/* capital O, dieresis or umlaut mark - Ouml */
#endif /* LY_UMLAUT */
	"P",	/* capital THORN, Icelandic - THORN */
	"U",	/* capital U, acute accent - Uacute */
	"U",	/* capital U, circumflex accent - Ucirc */
	"U",	/* capital U, grave accent - Ugrave */
#ifdef LY_UMLAUT
	"Ue",	/* capital U, dieresis or umlaut mark - Uuml */
#else
	"U",	/* capital U, dieresis or umlaut mark - Uuml */
#endif /* LY_UMLAUT */
	"Y",	/* capital Y, acute accent - Yacute */
	"a",	/* small a, acute accent - aacute */
	"a",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"ae",	/* small ae diphthong (ligature) - aelig */
	"`a",	/* small a, grave accent - agrave */
	"&",	/* ampersand - amp */
	"a",	/* small a, ring - aring */
	"a",	/* small a, tilde - atilde */
#ifdef LY_UMLAUT
	"ae",	/* small a, dieresis or umlaut mark - auml */
#else
	"a",	/* small a, dieresis or umlaut mark - auml */
#endif /* LY_UMLAUT */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"c",	/* small c, cedilla - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"-c-",	/* cent sign (&#162;) - cent */
	"(c)",	/* copyright sign (&#169;) - copy */
	"CUR",	/* currency sign (&#164;) - curren */
	"DEG",	/* degree sign (&#176;) - deg */
	"\042",	/* spacing diaresis (&#168;) - die */
	"/",	/* division sign (&#247;) - divide */
	"e",	/* small e, acute accent - eacute */
	"e",	/* small e, circumflex accent - ecirc */
	"e",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic eth */
	"e",	/* small e, dieresis or umlaut mark - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	">",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"i",	/* small i, acute accent - iacute */
	"i",	/* small i, circumflex accent - icirc*/
	"!",	/* inverted exclamation mark (&#161;) - iexcl */
	"`i",	/* small i, grave accent - igrave */
	"?",	/* inverted question mark (&#191;) - iquest */
	"i",	/* small i, dieresis or umlaut mark - iuml */
	"<<",	/* angle quotation mark, left (&#171;) - laquo */
	"<",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"u",	/* micro sign (&#181;) - micro */
	".",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172;) - not */
	"n",	/* small n, tilde - ntilde */
	"o",	/* small o, acute accent - oacute */
	"o",	/* small o, circumflex accent - ocirc */
	"o",	/* small o, grave accent - ograve */
	"-a",	/* feminine ordinal indicator (&#170;) - ordf */
	"-o",	/* masculine ordinal indicator (&#186;) - ordm */
	"o",	/* small o, slash - oslash */
	"o",	/* small o, tilde - otilde */
#ifdef LY_UMLAUT
	"oe",	/* small o, dieresis or umlaut mark - ouml */
#else
	"o",	/* small o, dieresis or umlaut mark - ouml */
#endif /* LY_UMLAUT */
	"P:",	/* paragraph sign (&#182;) - para */
	"+-",	/* plus-or-minus sign (&#177;) - plusmn */
	"-L-",	/* pound sign (&#163;) - pound */
	"\"",	/* quote '"' - quot */
	">>",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"S:",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"^2",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"ss",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"u",	/* small u, acute accent - uacute */
	"u",	/* small u, circumflex accent - ucirc */
	"u",	/* small u, grave accent - ugrave */
	"\042",	/* spacing diaresis (&#168;) - uml */
#ifdef LY_UMLAUT
	"ue",	/* small u, dieresis or umlaut mark - uuml */
#else
	"u",	/* small u, dieresis or umlaut mark - uuml */
#endif /* LY_UMLAUT */
	"y",	/* small y, acute accent - yacute */
	"YEN",	/* yen sign (&#165;) - yen */
	"y",	/* small y, dieresis or umlaut mark - yuml */
};

/*      Entity values -- Japanese (EUC)
**
**      This MUST match exactly the table referred to in the DTD!
*/
PRIVATE char * EUC_JP[] = {
	"AE",	/* capital AE diphthong (ligature) - AElig */
	"A",	/* capital A, acute accent - Aacute */
	"A",	/* capital A, circumflex accent - Acirc */
	"A",	/* capital A, grave accent - Agrave */
	"A",	/* capital A, ring - Aring */
	"A",	/* capital A, tilde - Atilde */
#ifdef LY_UMLAUT
	"Ae",	/* capital A, dieresis or umlaut mark - Auml*/
#else
	"A",	/* capital A, dieresis or umlaut mark - Auml*/
#endif /* LY_UMLAUT */
	"C",	/* capital C, cedilla - Ccedil */
	"Dj",	/* capital D with stroke - Dstrok */
	"DH",	/* capital Eth, Icelandic - ETH */
	"E",	/* capital E, acute accent - Eacute */
	"E",	/* capital E, circumflex accent - Ecirc */
	"E",	/* capital E, grave accent - Egrave */
	"E",	/* capital E, dieresis or umlaut mark - Euml */
	"I",	/* capital I, acute accent - Iacute */
	"I",	/* capital I, circumflex accent - Icirc */
	"I",	/* capital I, grave accent - Igrave */
	"I",	/* capital I, dieresis or umlaut mark - Iuml */
	"N",	/* capital N, tilde - Ntilde */
	"O",	/* capital O, acute accent - Oacute */
	"O",	/* capital O, circumflex accent - Ocirc */
	"O",	/* capital O, grave accent - Ograve */
	"O",	/* capital O, slash - Oslash */
	"O",	/* capital O, tilde - Otilde */
#ifdef LY_UMLAUT
	"Oe",	/* capital O, dieresis or umlaut mark - Ouml */
#else
	"O",	/* capital O, dieresis or umlaut mark - Ouml */
#endif /* LY_UMLAUT */
	"P",	/* capital THORN, Icelandic - THORN */
	"U",	/* capital U, acute accent - Uacute */
	"U",	/* capital U, circumflex accent - Ucirc */
	"U",	/* capital U, grave accent - Ugrave */
#ifdef LY_UMLAUT
	"Ue",	/* capital U, dieresis or umlaut mark - Uuml */
#else
	"U",	/* capital U, dieresis or umlaut mark - Uuml */
#endif /* LY_UMLAUT */
	"Y",	/* capital Y, acute accent - Yacute */
	"a",	/* small a, acute accent - aacute */
	"a",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"ae",	/* small ae diphthong (ligature) - aelig */
	"`a",	/* small a, grave accent - agrave */
	"&",	/* ampersand - amp */
	"a",	/* small a, ring - aring */
	"a",	/* small a, tilde - atilde */
#ifdef LY_UMLAUT
	"ae",	/* small a, dieresis or umlaut mark - auml */
#else
	"a",	/* small a, dieresis or umlaut mark - auml */
#endif /* LY_UMLAUT */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"c",	/* small c, cedilla - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"-c-",	/* cent sign (&#162;) - cent */
	"(c)",	/* copyright sign (&#169;) - copy */
	"CUR",	/* currency sign (&#164;) - curren */
	"DEG",	/* degree sign (&#176;) - deg */
	"\042",	/* spacing diaresis (&#168;) - die */
	"/",	/* division sign (&#247;) - divide */
	"e",	/* small e, acute accent - eacute */
	"e",	/* small e, circumflex accent - ecirc */
	"e",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic eth */
	"e",	/* small e, dieresis or umlaut mark - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	">",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"i",	/* small i, acute accent - iacute */
	"i",	/* small i, circumflex accent - icirc*/
	"!",	/* inverted exclamation mark (&#161;) - iexcl */
	"`i",	/* small i, grave accent - igrave */
	"?",	/* inverted question mark (&#191;) - iquest */
	"i",	/* small i, dieresis or umlaut mark - iuml */
	"<<",	/* angle quotation mark, left (&#171;) - laquo */
	"<",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"u",	/* micro sign (&#181;) - micro */
	".",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172;) - not */
	"n",	/* small n, tilde - ntilde */
	"o",	/* small o, acute accent - oacute */
	"o",	/* small o, circumflex accent - ocirc */
	"o",	/* small o, grave accent - ograve */
	"-a",	/* feminine ordinal indicator (&#170;) - ordf */
	"-o",	/* masculine ordinal indicator (&#186;) - ordm */
	"o",	/* small o, slash - oslash */
	"o",	/* small o, tilde - otilde */
#ifdef LY_UMLAUT
	"oe",	/* small o, dieresis or umlaut mark - ouml */
#else
	"o",	/* small o, dieresis or umlaut mark - ouml */
#endif /* LY_UMLAUT */
	"P:",	/* paragraph sign (&#182;) - para */
	"+-",	/* plus-or-minus sign (&#177;) - plusmn */
	"-L-",	/* pound sign (&#163;) - pound */
	"\"",	/* quote '"' - quot */
	">>",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"S:",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"^2",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"ss",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"u",	/* small u, acute accent - uacute */
	"u",	/* small u, circumflex accent - ucirc */
	"u",	/* small u, grave accent - ugrave */
	"\042",	/* spacing diaresis (&#168;) - uml */
#ifdef LY_UMLAUT
	"ue",	/* small u, dieresis or umlaut mark - uuml */
#else
	"u",	/* small u, dieresis or umlaut mark - uuml */
#endif /* LY_UMLAUT */
	"y",	/* small y, acute accent - yacute */
	"YEN",	/* yen sign (&#165;) - yen */
	"y",	/* small y, dieresis or umlaut mark - yuml */
};

/*      Entity values -- Japanese (SJIS)
**
**      This MUST match exactly the table referred to in the DTD!
*/
PRIVATE char * Shift_JIS[] = {
	"AE",	/* capital AE diphthong (ligature) - AElig */
	"A",	/* capital A, acute accent - Aacute */
	"A",	/* capital A, circumflex accent - Acirc */
	"A",	/* capital A, grave accent - Agrave */
	"A",	/* capital A, ring - Aring */
	"A",	/* capital A, tilde - Atilde */
#ifdef LY_UMLAUT
	"Ae",	/* capital A, dieresis or umlaut mark - Auml*/
#else
	"A",	/* capital A, dieresis or umlaut mark - Auml*/
#endif /* LY_UMLAUT */
	"C",	/* capital C, cedilla - Ccedil */
	"Dj",	/* capital D with stroke - Dstrok */
	"DH",	/* capital Eth, Icelandic - ETH */
	"E",	/* capital E, acute accent - Eacute */
	"E",	/* capital E, circumflex accent - Ecirc */
	"E",	/* capital E, grave accent - Egrave */
	"E",	/* capital E, dieresis or umlaut mark - Euml */
	"I",	/* capital I, acute accent - Iacute */
	"I",	/* capital I, circumflex accent - Icirc */
	"I",	/* capital I, grave accent - Igrave */
	"I",	/* capital I, dieresis or umlaut mark - Iuml */
	"N",	/* capital N, tilde - Ntilde */
	"O",	/* capital O, acute accent - Oacute */
	"O",	/* capital O, circumflex accent - Ocirc */
	"O",	/* capital O, grave accent - Ograve */
	"O",	/* capital O, slash - Oslash */
	"O",	/* capital O, tilde - Otilde */
#ifdef LY_UMLAUT
	"Oe",	/* capital O, dieresis or umlaut mark - Ouml */
#else
	"O",	/* capital O, dieresis or umlaut mark - Ouml */
#endif /* LY_UMLAUT */
	"P",	/* capital THORN, Icelandic - THORN */
	"U",	/* capital U, acute accent - Uacute */
	"U",	/* capital U, circumflex accent - Ucirc */
	"U",	/* capital U, grave accent - Ugrave */
#ifdef LY_UMLAUT
	"Ue",	/* capital U, dieresis or umlaut mark - Uuml */
#else
	"U",	/* capital U, dieresis or umlaut mark - Uuml */
#endif /* LY_UMLAUT */
	"Y",	/* capital Y, acute accent - Yacute */
	"a",	/* small a, acute accent - aacute */
	"a",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"ae",	/* small ae diphthong (ligature) - aelig */
	"`a",	/* small a, grave accent - agrave */
	"&",	/* ampersand - amp */
	"a",	/* small a, ring - aring */
	"a",	/* small a, tilde - atilde */
#ifdef LY_UMLAUT
	"ae",	/* small a, dieresis or umlaut mark - auml */
#else
	"a",	/* small a, dieresis or umlaut mark - auml */
#endif /* LY_UMLAUT */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"c",	/* small c, cedilla - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"-c-",	/* cent sign (&#162;) - cent */
	"(c)",	/* copyright sign (&#169;) - copy */
	"CUR",	/* currency sign (&#164;) - curren */
	"DEG",	/* degree sign (&#176;) - deg */
	"\042",	/* spacing diaresis (&#168;) - die */
	"/",	/* division sign (&#247;) - divide */
	"e",	/* small e, acute accent - eacute */
	"e",	/* small e, circumflex accent - ecirc */
	"e",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic eth */
	"e",	/* small e, dieresis or umlaut mark - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	">",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"i",	/* small i, acute accent - iacute */
	"i",	/* small i, circumflex accent - icirc*/
	"!",	/* inverted exclamation mark (&#161;) - iexcl */
	"`i",	/* small i, grave accent - igrave */
	"?",	/* inverted question mark (&#191;) - iquest */
	"i",	/* small i, dieresis or umlaut mark - iuml */
	"<<",	/* angle quotation mark, left (&#171;) - laquo */
	"<",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"u",	/* micro sign (&#181;) - micro */
	".",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172;) - not */
	"n",	/* small n, tilde - ntilde */
	"o",	/* small o, acute accent - oacute */
	"o",	/* small o, circumflex accent - ocirc */
	"o",	/* small o, grave accent - ograve */
	"-a",	/* feminine ordinal indicator (&#170;) - ordf */
	"-o",	/* masculine ordinal indicator (&#186;) - ordm */
	"o",	/* small o, slash - oslash */
	"o",	/* small o, tilde - otilde */
#ifdef LY_UMLAUT
	"oe",	/* small o, dieresis or umlaut mark - ouml */
#else
	"o",	/* small o, dieresis or umlaut mark - ouml */
#endif /* LY_UMLAUT */
	"P:",	/* paragraph sign (&#182;) - para */
	"+-",	/* plus-or-minus sign (&#177;) - plusmn */
	"-L-",	/* pound sign (&#163;) - pound */
	"\"",	/* quote '"' - quot */
	">>",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"S:",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"^2",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"ss",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"u",	/* small u, acute accent - uacute */
	"u",	/* small u, circumflex accent - ucirc */
	"u",	/* small u, grave accent - ugrave */
	"\042",	/* spacing diaresis (&#168;) - uml */
#ifdef LY_UMLAUT
	"ue",	/* small u, dieresis or umlaut mark - uuml */
#else
	"u",	/* small u, dieresis or umlaut mark - uuml */
#endif /* LY_UMLAUT */
	"y",	/* small y, acute accent - yacute */
	"YEN",	/* yen sign (&#165;) - yen */
	"y",	/* small y, dieresis or umlaut mark - yuml */
};

/*      Entity values -- Korean (EUC)
**
**      This MUST match exactly the table referred to in the DTD!
*/
PRIVATE char * Korean[] = {
	"AE",	/* capital AE diphthong (ligature) - AElig */
	"A",	/* capital A, acute accent - Aacute */
	"A",	/* capital A, circumflex accent - Acirc */
	"A",	/* capital A, grave accent - Agrave */
	"A",	/* capital A, ring - Aring */
	"A",	/* capital A, tilde - Atilde */
#ifdef LY_UMLAUT
	"Ae",	/* capital A, dieresis or umlaut mark - Auml*/
#else
	"A",	/* capital A, dieresis or umlaut mark - Auml*/
#endif /* LY_UMLAUT */
	"C",	/* capital C, cedilla - Ccedil */
	"Dj",	/* capital D with stroke - Dstrok */
	"DH",	/* capital Eth, Icelandic - ETH */
	"E",	/* capital E, acute accent - Eacute */
	"E",	/* capital E, circumflex accent - Ecirc */
	"E",	/* capital E, grave accent - Egrave */
	"E",	/* capital E, dieresis or umlaut mark - Euml */
	"I",	/* capital I, acute accent - Iacute */
	"I",	/* capital I, circumflex accent - Icirc */
	"I",	/* capital I, grave accent - Igrave */
	"I",	/* capital I, dieresis or umlaut mark - Iuml */
	"N",	/* capital N, tilde - Ntilde */
	"O",	/* capital O, acute accent - Oacute */
	"O",	/* capital O, circumflex accent - Ocirc */
	"O",	/* capital O, grave accent - Ograve */
	"O",	/* capital O, slash - Oslash */
	"O",	/* capital O, tilde - Otilde */
#ifdef LY_UMLAUT
	"Oe",	/* capital O, dieresis or umlaut mark - Ouml */
#else
	"O",	/* capital O, dieresis or umlaut mark - Ouml */
#endif /* LY_UMLAUT */
	"P",	/* capital THORN, Icelandic - THORN */
	"U",	/* capital U, acute accent - Uacute */
	"U",	/* capital U, circumflex accent - Ucirc */
	"U",	/* capital U, grave accent - Ugrave */
#ifdef LY_UMLAUT
	"Ue",	/* capital U, dieresis or umlaut mark - Uuml */
#else
	"U",	/* capital U, dieresis or umlaut mark - Uuml */
#endif /* LY_UMLAUT */
	"Y",	/* capital Y, acute accent - Yacute */
	"a",	/* small a, acute accent - aacute */
	"a",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"ae",	/* small ae diphthong (ligature) - aelig */
	"`a",	/* small a, grave accent - agrave */
	"&",	/* ampersand - amp */
	"a",	/* small a, ring - aring */
	"a",	/* small a, tilde - atilde */
#ifdef LY_UMLAUT
	"ae",	/* small a, dieresis or umlaut mark - auml */
#else
	"a",	/* small a, dieresis or umlaut mark - auml */
#endif /* LY_UMLAUT */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"c",	/* small c, cedilla - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"-c-",	/* cent sign (&#162;) - cent */
	"(c)",	/* copyright sign (&#169;) - copy */
	"CUR",	/* currency sign (&#164;) - curren */
	"DEG",	/* degree sign (&#176;) - deg */
	"\042",	/* spacing diaresis (&#168;) - die */
	"/",	/* division sign (&#247;) - divide */
	"e",	/* small e, acute accent - eacute */
	"e",	/* small e, circumflex accent - ecirc */
	"e",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic eth */
	"e",	/* small e, dieresis or umlaut mark - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	">",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"i",	/* small i, acute accent - iacute */
	"i",	/* small i, circumflex accent - icirc*/
	"!",	/* inverted exclamation mark (&#161;) - iexcl */
	"`i",	/* small i, grave accent - igrave */
	"?",	/* inverted question mark (&#191;) - iquest */
	"i",	/* small i, dieresis or umlaut mark - iuml */
	"<<",	/* angle quotation mark, left (&#171;) - laquo */
	"<",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"u",	/* micro sign (&#181;) - micro */
	".",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172;) - not */
	"n",	/* small n, tilde - ntilde */
	"o",	/* small o, acute accent - oacute */
	"o",	/* small o, circumflex accent - ocirc */
	"o",	/* small o, grave accent - ograve */
	"-a",	/* feminine ordinal indicator (&#170;) - ordf */
	"-o",	/* masculine ordinal indicator (&#186;) - ordm */
	"o",	/* small o, slash - oslash */
	"o",	/* small o, tilde - otilde */
#ifdef LY_UMLAUT
	"oe",	/* small o, dieresis or umlaut mark - ouml */
#else
	"o",	/* small o, dieresis or umlaut mark - ouml */
#endif /* LY_UMLAUT */
	"P:",	/* paragraph sign (&#182;) - para */
	"+-",	/* plus-or-minus sign (&#177;) - plusmn */
	"-L-",	/* pound sign (&#163;) - pound */
	"\"",	/* quote '"' - quot */
	">>",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"S:",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"^2",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"ss",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"u",	/* small u, acute accent - uacute */
	"u",	/* small u, circumflex accent - ucirc */
	"u",	/* small u, grave accent - ugrave */
	"\042",	/* spacing diaresis (&#168;) - uml */
#ifdef LY_UMLAUT
	"ue",	/* small u, dieresis or umlaut mark - uuml */
#else
	"u",	/* small u, dieresis or umlaut mark - uuml */
#endif /* LY_UMLAUT */
	"y",	/* small y, acute accent - yacute */
	"YEN",	/* yen sign (&#165;) - yen */
	"y",	/* small y, dieresis or umlaut mark - yuml */
};

/*      Entity values -- Taipei (Big5)
**
**	This MUST match exactly the table referred to in the DTD!
*/
PRIVATE char * Taipei[] = {
	"AE",	/* capital AE diphthong (ligature) - AElig */
	"A",	/* capital A, acute accent - Aacute */
	"A",	/* capital A, circumflex accent - Acirc */
	"A",	/* capital A, grave accent - Agrave */
	"A",	/* capital A, ring - Aring */
	"A",	/* capital A, tilde - Atilde */
#ifdef LY_UMLAUT
	"Ae",	/* capital A, dieresis or umlaut mark - Auml*/
#else
	"A",	/* capital A, dieresis or umlaut mark - Auml*/
#endif /* LY_UMLAUT */
	"C",	/* capital C, cedilla - Ccedil */
	"Dj",	/* capital D with stroke - Dstrok */
	"DH",	/* capital Eth, Icelandic - ETH */
	"E",	/* capital E, acute accent - Eacute */
	"E",	/* capital E, circumflex accent - Ecirc */
	"E",	/* capital E, grave accent - Egrave */
	"E",	/* capital E, dieresis or umlaut mark - Euml */
	"I",	/* capital I, acute accent - Iacute */
	"I",	/* capital I, circumflex accent - Icirc */
	"I",	/* capital I, grave accent - Igrave */
	"I",	/* capital I, dieresis or umlaut mark - Iuml */
	"N",	/* capital N, tilde - Ntilde */
	"O",	/* capital O, acute accent - Oacute */
	"O",	/* capital O, circumflex accent - Ocirc */
	"O",	/* capital O, grave accent - Ograve */
	"O",	/* capital O, slash - Oslash */
	"O",	/* capital O, tilde - Otilde */
#ifdef LY_UMLAUT
	"Oe",	/* capital O, dieresis or umlaut mark - Ouml */
#else
	"O",	/* capital O, dieresis or umlaut mark - Ouml */
#endif /* LY_UMLAUT */
	"P",	/* capital THORN, Icelandic - THORN */
	"U",	/* capital U, acute accent - Uacute */
	"U",	/* capital U, circumflex accent - Ucirc */
	"U",	/* capital U, grave accent - Ugrave */
#ifdef LY_UMLAUT
	"Ue",	/* capital U, dieresis or umlaut mark - Uuml */
#else
	"U",	/* capital U, dieresis or umlaut mark - Uuml */
#endif /* LY_UMLAUT */
	"Y",	/* capital Y, acute accent - Yacute */
	"a",	/* small a, acute accent - aacute */
	"a",	/* small a, circumflex accent - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"ae",	/* small ae diphthong (ligature) - aelig */
	"`a",	/* small a, grave accent - agrave */
	"&",	/* ampersand - amp */
	"a",	/* small a, ring - aring */
	"a",	/* small a, tilde - atilde */
#ifdef LY_UMLAUT
	"ae",	/* small a, dieresis or umlaut mark - auml */
#else
	"a",	/* small a, dieresis or umlaut mark - auml */
#endif /* LY_UMLAUT */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"c",	/* small c, cedilla - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"-c-",	/* cent sign (&#162;) - cent */
	"(c)",	/* copyright sign (&#169;) - copy */
	"CUR",	/* currency sign (&#164;) - curren */
	"DEG",	/* degree sign (&#176;) - deg */
	"\042",	/* spacing diaresis (&#168;) - die */
	"/",	/* division sign (&#247;) - divide */
	"e",	/* small e, acute accent - eacute */
	"e",	/* small e, circumflex accent - ecirc */
	"e",	/* small e, grave accent - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp, em space - not collapsed NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic eth */
	"e",	/* small e, dieresis or umlaut mark - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	">",	/* greater than - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"i",	/* small i, acute accent - iacute */
	"i",	/* small i, circumflex accent - icirc*/
	"!",	/* inverted exclamation mark (&#161;) - iexcl */
	"`i",	/* small i, grave accent - igrave */
	"?",	/* inverted question mark (&#191;) - iquest */
	"i",	/* small i, dieresis or umlaut mark - iuml */
	"<<",	/* angle quotation mark, left (&#171;) - laquo */
	"<",	/* less than - lt */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"u",	/* micro sign (&#181;) - micro */
	".",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172;) - not */
	"n",	/* small n, tilde - ntilde */
	"o",	/* small o, acute accent - oacute */
	"o",	/* small o, circumflex accent - ocirc */
	"o",	/* small o, grave accent - ograve */
	"-a",	/* feminine ordinal indicator (&#170;) - ordf */
	"-o",	/* masculine ordinal indicator (&#186;) - ordm */
	"o",	/* small o, slash - oslash */
	"o",	/* small o, tilde - otilde */
#ifdef LY_UMLAUT
	"oe",	/* small o, dieresis or umlaut mark - ouml */
#else
	"o",	/* small o, dieresis or umlaut mark - ouml */
#endif /* LY_UMLAUT */
	"P:",	/* paragraph sign (&#182;) - para */
	"+-",	/* plus-or-minus sign (&#177;) - plusmn */
	"-L-",	/* pound sign (&#163;) - pound */
	"\"",	/* quote '"' - quot */
	">>",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"S:",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"^2",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"ss",	/* small sharp s, German (sz ligature) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"u",	/* small u, acute accent - uacute */
	"u",	/* small u, circumflex accent - ucirc */
	"u",	/* small u, grave accent - ugrave */
	"\042",	/* spacing diaresis (&#168;) - uml */
#ifdef LY_UMLAUT
	"ue",	/* small u, dieresis or umlaut mark - uuml */
#else
	"u",	/* small u, dieresis or umlaut mark - uuml */
#endif /* LY_UMLAUT */
	"y",	/* small y, acute accent - yacute */
	"YEN",	/* yen sign (&#165;) - yen */
	"y",	/* small y, dieresis or umlaut mark - yuml */
};

/*      Entity values -- 7 bit character approximations
**
**      This MUST match exactly the table referred to in the DTD!
*/
PUBLIC char * SevenBitApproximations[] = {
	"AE",	/* capital AE diphthong (ligature) (&#198;) - AElig */
	"A",	/* capital A, acute accent (&#193;) - Aacute */
	"A",	/* capital A, circumflex accent (&#194;) - Acirc */
	"A",	/* capital A, grave accent (&#192;) - Agrave */
	"A",	/* capital A, ring - Aring (&#197;) */
	"A",	/* capital A, tilde - Atilde (&#195;) */
#ifdef LY_UMLAUT
	"Ae",	/* capital A, dieresis or umlaut mark (&#196;) - Auml*/
#else
	"A",	/* capital A, dieresis or umlaut mark (&#196;) - Auml*/
#endif /* LY_UMLAUT */
	"C",	/* capital C, cedilla (&#199;) - Ccedil */
	"Dj",	/* capital D with stroke (&#208;) - Dstrok */
	"DH",	/* capital Eth, Icelandic (&#208;) - ETH */
	"E",	/* capital E, acute accent (&#201;) - Eacute */
	"E",	/* capital E, circumflex accent (&#202;) - Ecirc */
	"E",	/* capital E, grave accent (&#200;) - Egrave */
	"E",	/* capital E, dieresis or umlaut mark (&#203;) - Euml */
	"I",	/* capital I, acute accent (&#205;) - Iacute */
	"I",	/* capital I, circumflex accent (&#206;) - Icirc */
	"I",	/* capital I, grave accent (&#204;) - Igrave */
	"I",	/* capital I, dieresis or umlaut mark (&#207;) - Iuml */
	"N",	/* capital N, tilde - Ntilde (&#209;) */
	"O",	/* capital O, acute accent (&#211;) - Oacute */
	"O",	/* capital O, circumflex accent (&#212;) - Ocirc */
	"O",	/* capital O, grave accent (&#210;) - Ograve */
	"O",	/* capital O, slash (&#216;) - Oslash */
	"O",	/* capital O, tilde (&#213;) - Otilde */
#ifdef LY_UMLAUT
	"Oe",	/* capital O, dieresis or umlaut mark (&#214;) - Ouml */
#else
	"O",	/* capital O, dieresis or umlaut mark (&#214;) - Ouml */
#endif /* LY_UMLAUT */
	"P",	/* capital THORN, Icelandic (&#222;) - THORN */
	"U",	/* capital U, acute accent (&#218;) - Uacute */
	"U",	/* capital U, circumflex accent (&#219;) - Ucirc */
	"U",	/* capital U, grave accent (&#217;) - Ugrave */
#ifdef LY_UMLAUT
	"Ue",	/* capital U, dieresis or umlaut mark (&#220;) - Uuml */
#else
	"U",	/* capital U, dieresis or umlaut mark (&#220;) - Uuml */
#endif /* LY_UMLAUT */
	"Y",	/* capital Y, acute accent (&#221;) - Yacute */
	"a",	/* small a, acute accent (&#225;) - aacute */
	"a",	/* small a, circumflex accent (&#226;) - acirc */
	"'",	/* spacing acute (&#180;) - acute */
	"ae",	/* small ae diphthong (ligature) (&#230;) - aelig */
	"`a",	/* small a, grave accent (&#232;) - agrave */
	"&",	/* ampersand (&#38;) - amp */
	"a",	/* small a, ring (&#229;) - aring */
	"a",	/* small a, tilde (&#227;) - atilde */
#ifdef LY_UMLAUT
	"ae",	/* small a, dieresis or umlaut mark (&#228;) - auml */
#else
	"a",	/* small a, dieresis or umlaut mark (&#228;) - auml */
#endif /* LY_UMLAUT */
	"|",	/* broken vertical bar (&#166;) - brkbar */
	"|",	/* broken vertical bar (&#166;) - brvbar */
	"c",	/* small c, cedilla (&#231;) - ccedil */
	",",	/* spacing cedilla (&#184;) - cedil */
	"-c-",	/* cent sign (&#162;) - cent */
	"(c)",	/* copyright sign (&#169;) - copy */
	"CUR",	/* currency sign (&#164;) - curren */
	"DEG",	/* degree sign (&#176;) - deg */
	"\042",	/* spacing diaresis (&#168;) - die */
	"/",	/* division sign (&#247;) - divide */
	"e",	/* small e, acute accent (&#233;) - eacute */
	"e",	/* small e, circumflex accent (&#234;) - ecirc */
	"e",	/* small e, grave accent (&#232;) - egrave */
	"-",	/* dash the width of emsp - emdash */
	"\002",	/* emsp NEVER CHANGE THIS - emsp */
	"-",	/* dash the width of ensp - endash */
	"\002",	/* ensp NEVER CHANGE THIS - ensp */
	"dh",	/* small eth, Icelandic eth (&#240;) */
	"e",	/* small e, dieresis or umlaut mark (&#235;) - euml */
	" 1/2",	/* fraction 1/2 (&#189;) - frac12 */
	" 1/4",	/* fraction 1/4 (&#188;) - frac14 */
	" 3/4",	/* fraction 3/4 (&#190;) - frac34 */
	">",	/* greater than (&#62;) - gt */
	"-",	/* spacing macron (&#175;) - hibar */
	"i",	/* small i, acute accent (&#237;) - iacute */
	"i",	/* small i, circumflex accent (&#238;) - icirc*/
	"!",	/* inverted exclamation mark (&#161;) - iexcl */
	"`i",	/* small i, grave accent (&#236;) - igrave */
	"?",	/* inverted question mark (&#191;) - iquest */
	"i",	/* small i, dieresis or umlaut mark (&#239;) - iuml */
	"<<",	/* angle quotation mark, left (&#171;) - laquo */
	"<",	/* less than - lt (&#60;) */
	"-",	/* spacing macron (&#175;) - macr */
	"-",	/* dash the width of emsp - mdash */
	"u",	/* micro sign (&#181;) - micro */
	".",	/* middle dot (&#183;) - middot */
	"\001",	/* nbsp non-breaking space NEVER CHANGE THIS - nbsp */
	"-",	/* dash the width of ensp - ndash */
	"NOT",	/* negation sign (&#172;) - not */
	"n",	/* small n, tilde (&#241;) - ntilde */
	"o",	/* small o, acute accent (&#243;) - oacute */
	"o",	/* small o, circumflex accent (&#244;) - ocirc */
	"o",	/* small o, grave accent (&#242;) - ograve */
	"-a",	/* feminine ordinal indicator (&#170;) - ordf */
	"-o",	/* masculine ordinal indicator (&#186;) - ordm */
	"o",	/* small o, slash (&#248;) - oslash */
	"o",	/* small o, tilde (&#245;) - otilde */
#ifdef LY_UMLAUT
	"oe",	/* small o, dieresis or umlaut mark (&#246;) - ouml */
#else
	"o",	/* small o, dieresis or umlaut mark (&#246;) - ouml */
#endif /* LY_UMLAUT */
	"P:",	/* paragraph sign (&#182;) - para */
	"+-",	/* plus-or-minus sign (&#177;) - plusmn */
	"-L-",	/* pound sign (&#163;) - pound */
	"\"",	/* quote '"' (&#34;) - quot */
	">>",	/* angle quotation mark, right (&#187;) - raquo */
	"(R)",	/* circled R registered sign (&#174;) - reg */
	"S:",	/* section sign (&#167;) - sect */
	"\007",	/* soft hyphen (&#173;) NEVER CHANGE THIS - shy */
	"^1",	/* superscript 1 (&#185;) - sup1 */
	"^2",	/* superscript 2 (&#178;) - sup2 */
	"^3",	/* superscript 3 (&#179;) - sup3 */
	"ss",	/* small sharp s, German (sz ligature) (&#223;) - szlig */
	"\002",	/* thin space - not collapsed NEVER CHANGE THIS - thinsp */
	"p",	/* small thorn, Icelandic (&#254;) - thorn */
	"*",	/* multiplication sign (&#215;) - times */
	"(TM)",	/* circled TM trade mark sign (&#8482;) - trade */
	"u",	/* small u, acute accent (&#250;) - uacute */
	"u",	/* small u, circumflex accent (&#251;) - ucirc */
	"u",	/* small u, grave accent (&#249;) - ugrave */
	"\042",	/* spacing diaresis (&#168;) - uml */
#ifdef LY_UMLAUT
	"ue",	/* small u, dieresis or umlaut mark (&#252;) - uuml */
#else
	"u",	/* small u, dieresis or umlaut mark (&#252;) - uuml */
#endif /* LY_UMLAUT */
	"y",	/* small y, acute accent (&#253;) - yacute */
	"YEN",	/* yen sign (&#165;) - yen */
	"y",	/* small y, dieresis or umlaut mark (&#255;) - yuml */
};

/*
 *  Add your new character sets HERE (but only if you
 *  can't construct Unicode tables for them). - FM
 */

/*
 *  Add the array name to LYCharSets
 */
PUBLIC char ** LYCharSets[MAXCHARSETS]={
	ISO_Latin1,		/* ISO Latin 1		*/
	ISO_Latin2,
	ISO_LatinN,           	/* Other ISO Latin	*/
	SevenBitApproximations,	/* WinLatin1 (cp1252)	*/
	DEC_Multinational,     	/* DEC Multinational	*/
	Macintosh,              /* Macintosh (8 bit)	*/
	NeXT_Step,              /* NeXT character set	*/
	KOI8_R,
	Chinese,               	/* Chinese		*/
	EUC_JP,                 /* Japanese (EUC)	*/
	Shift_JIS,              /* Japanese (SJIS)	*/
	Korean,                 /* Korean		*/
	Taipei,                 /* Taipei (Big5)	*/
	SevenBitApproximations, /* Vietnamese (VISCII)	*/
	SevenBitApproximations,	/* 7 Bit Approximations	*/
	SevenBitApproximations,	/* Transparent		*/
	PC_charset,             /* DosLatinUS (cp437)	*/
	PC_850_charset         	/* DosLatin1 (cp850)	*/
};

/*
 *  Add the name that the user will see below.
 *  The order of LYCharSets and LYchar_set_names MUST be the same
 */
PUBLIC CONST char * LYchar_set_names[MAXCHARSETSP]={
	"ISO Latin 1         ",
	"ISO Latin 2         ",
	"Other ISO Latin     ",
	"WinLatin1 (cp1252)  ",
	"DEC Multinational   ",
	"Macintosh (8 bit)   ",
	"NeXT character set  ",
	"KOI8-R Cyrillic     ",
	"Chinese             ",
	"Japanese (EUC)      ",
	"Japanese (SJIS)     ",
	"Korean              ",
	"Taipei (Big5)       ",
	"Vietnamese (VISCII) ",
	"7 bit approximations",
	"Transparent         ",
	"DosLatinUS (cp437)  ",
	"DosLatin1 (cp850)   ",
	(char *) 0
};

/*
 *  Associate additional pieces of info with each of the charsets listed
 *  above.
 *  Will be automatically modified (and extended) by charset translations
 *  which are loaded using the chartrans mechanism.
 *  Most important piece of info to put here is a MIME charset name.
 *  Used for chartrans.
 *  The order of LYCharSets and LYCharSet_UC MUST be the same.
 *
 *  Note that most of the charsets added by the new mechanism in src/chrtrans
 *  don't show up here at all.  They don't have to.
 */
PUBLIC LYUCcharset LYCharSet_UC[MAXCHARSETS]=
{
  /*
   *  Zero position placeholder and HTMLGetEntityUCValue() reference. - FM
   */
  {-1,"iso-8859-1",	UCT_ENC_8BIT,
			UCT_REP_IS_LAT1,
			UCT_CP_IS_LAT1,		UCT_R_LAT1,UCT_R_LAT1},
  /*
   *  Placeholder for Unicode table.
   */
  {-1,"iso-8859-2",	UCT_ENC_8BIT,0,0,	UCT_R_LAT1,UCT_R_8859S},
  /*
   *  For unknown iso-8859-# charsets. - FM
   */
  {-1,"x-iso-8859-other",UCT_ENC_8BIT,0,0,	UCT_R_LAT1,UCT_R_8859S},

  /*
   *  Placeholders for Unicode tables. - FM
   */
  {-1,"cp1252",		UCT_ENC_8BIT,
			UCT_REP_SUPERSETOF_LAT1,
			0,			UCT_R_8BIT,UCT_R_ASCII},
  {-1,"dec-mcs",	UCT_ENC_8BIT,0,0,	UCT_R_8BIT,UCT_R_ASCII},
  {-1,"macintosh",	UCT_ENC_8BIT,0,0,	UCT_R_8BIT,UCT_R_ASCII},
  {-1,"next",		UCT_ENC_8BIT,0,0,	UCT_R_8BIT,UCT_R_ASCII},
  {-1,"koi8-r",		UCT_ENC_8BIT,0,0,	UCT_R_8BIT,UCT_R_ASCII},

  /*
   *  There is no strict correlation for the next five, since the transfer
   *  charset gets decoded into Display Char Set by the CJK code (separate
   *  from Unicode mechanism).  For now we use the MIME name that describes
   *  what is output to the terminal. - KW
   */
  {-1,"euc-cn",		UCT_ENC_CJK,0,0,	UCT_R_8BIT,UCT_R_ASCII},
  {-1,"euc-jp",		UCT_ENC_CJK,0,0,	UCT_R_8BIT,UCT_R_ASCII},
  {-1,"shift_jis",	UCT_ENC_CJK,0,0,	UCT_R_8BIT,UCT_R_ASCII},
  {-1,"euc-kr",		UCT_ENC_CJK,0,0,	UCT_R_8BIT,UCT_R_ASCII},
  {-1,"big5",		UCT_ENC_CJK,0,0,	UCT_R_8BIT,UCT_R_ASCII},

  /*
   *  Placeholders for Unicode tables. - FM
   */
  {-1,"viscii",		UCT_ENC_8BIT_C0,0,0,	UCT_R_8BIT,UCT_R_ASCII},
  {-1,"us-ascii",	UCT_ENC_7BIT,
			UCT_REP_SUBSETOF_LAT1,
			UCT_CP_SUBSETOF_LAT1,	UCT_R_ASCII,UCT_R_ASCII},

  /*
   *  Placeholder for non-translation mode. - FM
   */
  {-1,"x-transparent",	UCT_ENC_8BIT,0,0,	UCT_R_8BIT,UCT_R_ASCII},

  /*
   *  Placeholders for Unicode tables.
   */
  {-1,"cp437",		UCT_ENC_8BIT,0,0,	UCT_R_8BIT,UCT_R_ASCII},
  {-1,"cp850",		UCT_ENC_8BIT,
			UCT_REP_SUPERSETOF_LAT1,
			0,			UCT_R_8BIT,UCT_R_ASCII}
};

/*
 *  Add the code of the the lowest character with the high bit set
 *  that can be directly displayed.
 *  The order of LYCharSets and LYlowest_eightbit MUST be the same.
 */
PUBLIC int LYlowest_eightbit[MAXCHARSETS]={
	160,	/* ISO Latin 1		*/
	160,	/* ISO Latin 2 */
	160,	/* Other ISO Latin	*/
	130,	/* WinLatin1 (cp1252)	*/
	160,	/* DEC Multinational	*/
	128,	/* Macintosh (8 bit)	*/
	128,	/* NeXT character set	*/
	128,	/* KOI8-R Cyrillic      */
	128,	/* Chinese		*/
	128,	/* Japanese (EUC)	*/
	128,	/* Japanese (SJIS)	*/
	128,	/* Korean		*/
	128,	/* Taipei (Big5)	*/
	128,	/* Vietnamese (VISCII)	*/
	999,	/* 7 bit approximations */
	128,	/* Transparent  (???)	*/
	128,	/* DosLatinUS (cp437)	*/
	128	/* DosLatin1 (cp850)	*/
};

/*
 *  The default character set.
 *  --------------------------
 *  Use lynx.cfg and/or user
 *  'o'ptions (.lynxrc) to
 *  set a different default.
 */
PUBLIC char** p_entity_values = ISO_Latin1;	/* Pointer to translation */
PUBLIC int current_char_set = 0;		/* Index for translation */

/*
 *  Function to set the handling of selected character sets
 *  based on the current LYUseDefaultRawMode value. - FM
 */
PUBLIC void HTMLSetCharacterHandling ARGS1(int,i)
{
    int chndl = -2;
    if (LYCharSet_UC[i].enc != UCT_ENC_CJK) {
	chndl = 0;

	if (UCAssume_MIMEcharset)
	    chndl = UCGetLYhndl_byMIME(UCAssume_MIMEcharset);
	HTCJK = NOCJK;
	kanji_code = NOKANJI;

	HTPassEightBitNum =
	    ((LYCharSet_UC[i].codepoints & UCT_CP_SUPERSETOF_LAT1) ||
		(LYCharSet_UC[i].like8859 & UCT_R_HIGH8BIT));

	if (i == (chndl < 0 ? 0 : chndl)) {
	    LYRawMode = LYUseDefaultRawMode ? TRUE : FALSE;
	} else {
	    LYRawMode = LYUseDefaultRawMode ? FALSE : TRUE;
	}
	if (LYRawMode) {
	    HTPassEightBitRaw = (LYlowest_eightbit[i] <= 160);
	} else {
	    HTPassEightBitRaw = FALSE;
	}
	if (LYRawMode || i == chndl) {
	    HTPassHighCtrlRaw = (LYlowest_eightbit[i] <= 130);
	} else {
	    HTPassHighCtrlRaw = FALSE;
	}

	HTPassHighCtrlNum = FALSE;

    } else if (!strncmp(LYchar_set_names[i], "Chinese", 7)) {
	HTCJK = LYUseDefaultRawMode ? CHINESE : NOCJK;
	LYRawMode = (HTCJK != NOCJK) ? TRUE : FALSE;
	kanji_code = EUC;
	HTPassEightBitRaw = FALSE;
	HTPassEightBitNum = FALSE;
	HTPassHighCtrlRaw = (HTCJK != NOCJK) ? TRUE : FALSE;
	HTPassHighCtrlNum = FALSE;

    } else if (!strncmp(LYchar_set_names[i], "Japanese (EUC)", 14)) {
	HTCJK = LYUseDefaultRawMode ? JAPANESE : NOCJK;
	LYRawMode = (HTCJK != NOCJK) ? TRUE : FALSE;
	kanji_code = EUC;
	HTPassEightBitRaw = FALSE;
	HTPassEightBitNum = FALSE;
	HTPassHighCtrlRaw = (HTCJK != NOCJK) ? TRUE : FALSE;
	HTPassHighCtrlNum = FALSE;

    } else if (!strncmp(LYchar_set_names[i], "Japanese (SJIS)", 15)) {
	HTCJK = LYUseDefaultRawMode ? JAPANESE : NOCJK;
	LYRawMode = (HTCJK != NOCJK) ? TRUE : FALSE;
	kanji_code = SJIS;
	HTPassEightBitRaw = FALSE;
	HTPassEightBitNum = FALSE;
	HTPassHighCtrlRaw = (HTCJK != NOCJK) ? TRUE : FALSE;
	HTPassHighCtrlNum = FALSE;

    } else if (!strncmp(LYchar_set_names[i], "Korean", 6)) {
	HTCJK = LYUseDefaultRawMode ? KOREAN : NOCJK;
	LYRawMode = (HTCJK != NOCJK) ? TRUE : FALSE;
	kanji_code = EUC;
	HTPassEightBitRaw = FALSE;
	HTPassEightBitNum = FALSE;
	HTPassHighCtrlRaw = (HTCJK != NOCJK) ? TRUE : FALSE;
	HTPassHighCtrlNum = FALSE;

    } else if (!strncmp(LYchar_set_names[i], "Taipei (Big5)", 13)) {
	HTCJK = LYUseDefaultRawMode ? TAIPEI : NOCJK;
	LYRawMode = (HTCJK != NOCJK) ? TRUE : FALSE;
	kanji_code = EUC;
	HTPassEightBitRaw = FALSE;
	HTPassEightBitNum = FALSE;
	HTPassHighCtrlRaw = (HTCJK != NOCJK) ? TRUE : FALSE;
	HTPassHighCtrlNum = FALSE;

    } else {
	HTCJK = NOCJK;
	kanji_code = NOKANJI;
	HTPassEightBitRaw = LYUseDefaultRawMode ? FALSE : TRUE;
	LYRawMode = HTPassEightBitRaw;
	HTPassEightBitNum = FALSE;
        HTPassHighCtrlRaw = FALSE;
	HTPassHighCtrlNum = FALSE;
    }

    if (LYRawMode) {
	UCLYhndl_for_unspec = i;
    } else {
	if (chndl == -2) {
	    chndl = 0;

	    if (UCAssume_MIMEcharset)
		chndl = UCGetLYhndl_byMIME(UCAssume_MIMEcharset);
	}
	if (chndl != i && chndl >= 0 &&
	    (LYCharSet_UC[i].enc != UCT_ENC_CJK ||
	     LYCharSet_UC[chndl].enc != UCT_ENC_CJK)) {
	    UCLYhndl_for_unspec = chndl;
	} else {
	    UCLYhndl_for_unspec = 0;
	}
    }

#ifdef USE_SLANG
    if (LYlowest_eightbit[i] > 191) {
	/*
	 *  Higher than this may output cntrl chars to screen. - KW
	 */
	SLsmg_Display_Eight_Bit = 191;
    } else {
	SLsmg_Display_Eight_Bit = LYlowest_eightbit[i];
    }
#endif /* USE_SLANG */

    ena_csi((LYlowest_eightbit[current_char_set] > 155));

    return;
}

/*
 *  Function to set the LYDefaultRawMode value
 *  based on the selected character set. - FM
 */
PUBLIC void HTMLSetRawModeDefault ARGS1(int,i)
{
    if (!strncmp(LYchar_set_names[i], "ISO Latin 1", 11) ||
	!strncmp(LYchar_set_names[i], "Chinese", 7) ||
	!strncmp(LYchar_set_names[i], "Japanese (EUC)", 14) ||
	!strncmp(LYchar_set_names[i], "Japanese (SJIS)", 15) ||
	!strncmp(LYchar_set_names[i], "Korean", 6) ||
	!strncmp(LYchar_set_names[i], "Taipei (Big5)", 13)) {
	LYDefaultRawMode = TRUE;
    } else {
	LYDefaultRawMode = FALSE;
    }
    return;
}

/*
 *  Function to set the LYUseDefaultRawMode value
 *  based on the selected character set and the
 *  current LYRawMode value. - FM
 */
PUBLIC void HTMLSetUseDefaultRawMode ARGS2(
	int,		i,
	BOOLEAN,	modeflag)
{
    if (LYCharSet_UC[i].enc != UCT_ENC_CJK) {
	int chndl = 0;

	if (UCAssume_MIMEcharset)
	    chndl = UCGetLYhndl_byMIME(UCAssume_MIMEcharset);
	if (i == chndl)
	    LYUseDefaultRawMode = modeflag;
	else
	    LYUseDefaultRawMode = (!modeflag);
    } else if (!strncmp(LYchar_set_names[i], "ISO Latin 1", 11) ||
	       !strncmp(LYchar_set_names[i], "Chinese", 7) ||
	       !strncmp(LYchar_set_names[i], "Japanese (EUC)", 14) ||
	       !strncmp(LYchar_set_names[i], "Japanese (SJIS)", 15) ||
	       !strncmp(LYchar_set_names[i], "Korean", 6) ||
	       !strncmp(LYchar_set_names[i], "Taipei (Big5)", 13)) {
	if (modeflag == TRUE) {
	    LYUseDefaultRawMode = TRUE;
	} else {
	    LYUseDefaultRawMode = FALSE;
	}
    } else {
	if (modeflag == FALSE) {
	    LYUseDefaultRawMode = TRUE;
	} else {
	    LYUseDefaultRawMode = FALSE;
	}
    }
    return;
}

/*
 *  Function to set the LYHaveCJKCharacterSet value
 *  based on the selected character set. - FM
 */
PUBLIC void HTMLSetHaveCJKCharacterSet ARGS1(int,i)
{
    if (!strncmp(LYchar_set_names[i], "Chinese", 7) ||
	!strncmp(LYchar_set_names[i], "Japanese (EUC)", 14) ||
	!strncmp(LYchar_set_names[i], "Japanese (SJIS)", 15) ||
	!strncmp(LYchar_set_names[i], "Korean", 6) ||
	!strncmp(LYchar_set_names[i], "Taipei (Big5)", 13)) {
	LYHaveCJKCharacterSet = TRUE;
    } else {
        LYHaveCJKCharacterSet = FALSE;
    }
    return;
}

/*
 *  Entity names -- Ordered by ISO Latin 1 value.
 *  ---------------------------------------------
 *   For conversions of DECIMAL escaped entities.
 *   Must be in order of ascending value.
 */
PUBLIC CONST char * LYEntityNames[] = {
/*	 NAME		   DECIMAL VALUE */
	"nbsp",		/* 160,	non breaking space */
	"iexcl",	/* 161,	inverted exclamation mark */
	"cent",		/* 162,	cent sign */
	"pound",	/* 163,	pound sign */
	"curren",	/* 164,	currency sign */
	"yen",		/* 165,	yen sign */
	"brvbar",	/* 166,	broken vertical bar, (brkbar) */
	"sect",		/* 167,	section sign */
	"uml",		/* 168,	spacing diaresis */
	"copy",		/* 169,	copyright sign */
	"ordf",		/* 170,	feminine ordinal indicator */
	"laquo",	/* 171,	angle quotation mark, left */
	"not",		/* 172,	negation sign */
	"shy",		/* 173,	soft hyphen */
	"reg",		/* 174,	circled R registered sign */
	"hibar",	/* 175,	spacing macron */
	"deg",		/* 176,	degree sign */
	"plusmn",	/* 177,	plus-or-minus sign */
	"sup2",		/* 178,	superscript 2 */
	"sup3",		/* 179,	superscript 3 */
	"acute",	/* 180,	spacing acute (96) */
	"micro",	/* 181,	micro sign */
	"para",		/* 182,	paragraph sign */
	"middot",	/* 183,	middle dot */
	"cedil",	/* 184,	spacing cedilla */
	"sup1",		/* 185,	superscript 1 */
	"ordm",		/* 186,	masculine ordinal indicator */
	"raquo",	/* 187,	angle quotation mark, right */
	"frac14",	/* 188,	fraction 1/4 */
	"frac12",	/* 189,	fraction 1/2 */
	"frac34",	/* 190,	fraction 3/4 */
	"iquest",	/* 191,	inverted question mark */
	"Agrave",	/* 192,	capital A, grave accent */
	"Aacute",	/* 193,	capital A, acute accent */
	"Acirc",	/* 194,	capital A, circumflex accent */
	"Atilde",	/* 195,	capital A, tilde */
	"Auml",		/* 196,	capital A, dieresis or umlaut mark */
	"Aring",	/* 197,	capital A, ring */
	"AElig",	/* 198,	capital AE diphthong (ligature) */
	"Ccedil",	/* 199,	capital C, cedilla */
	"Egrave",	/* 200,	capital E, grave accent */
	"Eacute",	/* 201,	capital E, acute accent */
	"Ecirc",	/* 202,	capital E, circumflex accent */
	"Euml",		/* 203,	capital E, dieresis or umlaut mark */
	"Igrave",	/* 204,	capital I, grave accent */
	"Iacute",	/* 205,	capital I, acute accent */
	"Icirc",	/* 206,	capital I, circumflex accent */
	"Iuml",		/* 207,	capital I, dieresis or umlaut mark */
	"ETH",		/* 208,	capital Eth, Icelandic (or Latin2 Dstrok) */
	"Ntilde",	/* 209,	capital N, tilde */
	"Ograve",	/* 210,	capital O, grave accent */
	"Oacute",	/* 211,	capital O, acute accent */
	"Ocirc",	/* 212,	capital O, circumflex accent */
	"Otilde",	/* 213,	capital O, tilde */
	"Ouml",		/* 214,	capital O, dieresis or umlaut mark */
	"times",	/* 215,	multiplication sign */
	"Oslash",	/* 216,	capital O, slash */
	"Ugrave",	/* 217,	capital U, grave accent */
	"Uacute",	/* 218,	capital U, acute accent */
	"Ucirc",	/* 219,	capital U, circumflex accent */
	"Uuml",		/* 220,	capital U, dieresis or umlaut mark */
	"Yacute",	/* 221,	capital Y, acute accent */
	"THORN",	/* 222,	capital THORN, Icelandic */
	"szlig",	/* 223,	small sharp s, German (sz ligature) */
	"agrave",	/* 224,	small a, grave accent */
	"aacute",	/* 225,	small a, acute accent */
	"acirc",	/* 226,	small a, circumflex accent */
	"atilde",	/* 227,	small a, tilde */
	"auml",		/* 228,	small a, dieresis or umlaut mark */
	"aring",	/* 229,	small a, ring */
	"aelig",	/* 230,	small ae diphthong (ligature) */
	"ccedil",	/* 231,	small c, cedilla */
	"egrave",	/* 232,	small e, grave accent */
	"eacute",	/* 233,	small e, acute accent */
	"ecirc",	/* 234,	small e, circumflex accent */
	"euml",		/* 235,	small e, dieresis or umlaut mark */
	"igrave",	/* 236,	small i, grave accent */
	"iacute",	/* 237,	small i, acute accent */
	"icirc",	/* 238,	small i, circumflex accent */
	"iuml",		/* 239,	small i, dieresis or umlaut mark */
	"eth",		/* 240,	small eth, Icelandic */
	"ntilde",	/* 241,	small n, tilde */
	"ograve",	/* 242,	small o, grave accent */
	"oacute",	/* 243,	small o, acute accent */
	"ocirc",	/* 244,	small o, circumflex accent */
	"otilde",	/* 245,	small o, tilde */
	"ouml",		/* 246,	small o, dieresis or umlaut mark */
	"divide",	/* 247,	division sign */
	"oslash",	/* 248,	small o, slash */
	"ugrave",	/* 249,	small u, grave accent */
	"uacute",	/* 250,	small u, acute accent */
	"ucirc",	/* 251,	small u, circumflex accent */
	"uuml",		/* 252,	small u, dieresis or umlaut mark */
	"yacute",	/* 253,	small y, acute accent */
	"thorn",	/* 254,	small thorn, Icelandic */
	"yuml",		/* 255,	small y, dieresis or umlaut mark */
};

/*
 *  Function to return the entity names of
 *  ISO-8859-1 8-bit characters. - FM
 */
PUBLIC CONST char * HTMLGetEntityName ARGS1(
	UCode_t,	code)
{
#define IntValue code
    int MaxValue = ((sizeof(LYEntityNames)/sizeof(char **)) - 1);

    if (IntValue < 0 || IntValue > MaxValue) {
	return "";
    }

    return LYEntityNames[IntValue];
}

/*
 *  Function to return the UCode_t (long int) value for entity names
 *  in the ISO_Latin1 and UC_entity_info extra_entities arrays.  It
 *  returns 0 if not found. - FM
 */
PUBLIC UCode_t HTMLGetEntityUCValue ARGS1(
	CONST char *,	name)
{
    UCode_t value = 0;
    int i, high, low, diff = 0;
    CONST char ** entities = HTML_dtd.entity_names;
    CONST UC_entity_info * extra_entities = HTML_dtd.extra_entity_info;

    /*
     *  Make sure we have a non-zero length name. - FM
     */
    if (!(name && *name))
	return(value);

    /*
     *  Handle names that have control characters
     *  or strings in ISO_Latin1[]. - FM
     */
    if (!strcmp(name, "nbsp")) {
	return(160);
    }
    if (!strcmp(name, "shy")) {
	return(173);
    }
    if (!strcmp(name, "ensp")) {
	return(8194);
    }
    if (!strcmp(name, "emsp")) {
	return(8195);
    }
    if (!strcmp(name, "thinsp")) {
	return(8201);
    }
    if (!strcmp(name, "ndash") ||
	!strcmp(name, "endash")) {
	return(8211);
    }
    if (!strcmp(name, "mdash") ||
	!strcmp(name, "emdash")) {
	return(8212);
    }
    if (!strcmp(name, "trade")) {
	return(8482);
    }

    /*
     *  Now try ISO_Latin1[]. - FM
     */
    for (low = 0, high = HTML_dtd.number_of_entities;
	 high > low;
	 diff < 0 ? (low = i+1) : (high = i)) {
	/*
	**  Binary search.
	*/
	i = (low + (high-low)/2);
	diff = strcmp(entities[i], name);
	if (diff == 0) {
	    if (strlen(ISO_Latin1[i]) == 1) {
		value = (unsigned char)(ISO_Latin1[i][0]);
	    }
	    break;
	}
    }
    if (value > 0) {
	return(value);
    }

    /*
     *  Not yet found, so try UC_entity_info extra_entities[]. - FM
     */
    for (low = 0, high = HTML_dtd.number_of_extra_entities;
	 high > low;
	 diff < 0 ? (low = i+1) : (high = i)) {
	/*
	**  Binary search.
	*/
	i = (low + (high-low)/2);
	diff = strcmp(extra_entities[i].name, name);
	if (diff == 0) {
	    value = extra_entities[i].code;
	    break;
	}
    }

    return(value);
}

/*
 *  Function to return the values of the ISO_Latin1 Character Set.
 *  It assumes the strings have only one character, and restores
 *  nbsp to 160 and shy to 173, but keeps our substitutions for
 *  characters that are not part of the ISO-8859-1 charset. - FM
 *
 *  Return '\0' to signal that there isn't a one-character
 *  equivalent.  Caller must check! and do whatever additional
 *  processing it wants to do instead.  - KW
 */
PUBLIC char HTMLGetLatinOneValue ARGS1(int,i)
{
    char ch = ISO_Latin1[i][0];

    switch ((unsigned char)ch) {
        case HT_NON_BREAK_SPACE:
	    ch = 160;
	    break;

        case HT_EM_SPACE:
	    ch = 32;
	    break;

        case LY_SOFT_HYPHEN:
	    ch = 173;
	    break;

	default:
	    if (ch && ISO_Latin1[i][1]) {
		/*
		 *  Got a string longer than 1 char.
		 */
		return '\0';
	    }
	    break;
     }

     return ch;
}

/*
 *  Function to select a character set and then set the
 *  character handling and LYHaveCJKCharacterSet flag. - FM
 */
PUBLIC void HTMLUseCharacterSet ARGS1(int,i)
{
    p_entity_values = LYCharSets[i];
    HTMLSetCharacterHandling(i);
    HTMLSetHaveCJKCharacterSet(i);
    return;
}

/*
 *  Initializer, calls initialization function for the
 *  CHARTRANS handling if compiled in. - KW
 */
PUBLIC int LYCharSetsDeclared NOPARAMS
{
    UCInit();

    if (UCAssume_MIMEcharset && *UCAssume_MIMEcharset) {
	UCLYhndl_for_unspec = UCGetLYhndl_byMIME(UCAssume_MIMEcharset);
    } else {
	UCLYhndl_for_unspec = 0;
    }
    if (UCAssume_localMIMEcharset && *UCAssume_localMIMEcharset)
	UCLYhndl_HTFile_for_unspec =
	    UCGetLYhndl_byMIME(UCAssume_localMIMEcharset);
    if (UCAssume_unrecMIMEcharset && *UCAssume_unrecMIMEcharset)
	UCLYhndl_for_unrec =
	    UCGetLYhndl_byMIME(UCAssume_unrecMIMEcharset);

    return UCInitialized;
}
