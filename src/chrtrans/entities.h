/*	Entity Names to Unicode table
 *     -----------------------------
 *
 *     This is a one-way mapping to Unicode so chartrans implementation
 *     now process character entities like &nbsp the similar way it handles
 *     the numeric entities like &#123.
 *     The only call to this structure is via HTMLGetEntityUCValue().
 *

Unlike the numeric entities &#234 which may be for any Unicode character, the
character references should be defined within HTML standards to get a
compatibility between browsers.

Now we have a choice:  use clean HTML4.0 entities list (and reject everithing
others), or use a relaxed list with lots of synonyms and new symbols found at

ftp://ftp.unicode.org/MAPPINGS/VENDORS/MISC/SGML.TXT

We hold both: #define ENTITIES_HTML40_ONLY for strict version,
otherwise relaxed.

 */

#include <UCkd.h>		/* typedef u16 */
typedef struct {
    const char *name;		/* sorted alphabetically (case-sensitive) */
    u16 code;
} UC_entity_info;

static const UC_entity_info unicode_entities[] =
/* *INDENT-OFF* */
#ifdef ENTITIES_HTML40_ONLY
/*********************************************************************

   The full list of character references defined as part of HTML 4.0.
   http://www.w3.org/TR/PR-html40/sgml/entities.html

   Informal history:
   * ISO Latin 1 entities for 160-255 range were introduced in HTML 2.0
   * few important entities were added, including &lt, &gt, &amp.
   * Greek letters and some math symbols were finally added in HTML 4.0

   Totally 252 entries (Nov 1997 HTML 4.0 draft), it is 1:1 mapping.
   Please do not add more unless a new HTML version will be released,
   try the #else table for experiments and fun...

****/
{
  {"AElig",      198}, /* latin capital letter AE = latin capital ligature AE, U+00C6 ISOlat1 */
  {"Aacute",     193}, /* latin capital letter A with acute, U+00C1 ISOlat1 */
  {"Acirc",      194}, /* latin capital letter A with circumflex, U+00C2 ISOlat1 */
  {"Agrave",     192}, /* latin capital letter A with grave = latin capital letter A grave, U+00C0 ISOlat1 */
  {"Alpha",      913}, /* greek capital letter alpha, U+0391 */
  {"Aring",      197}, /* latin capital letter A with ring above = latin capital letter A ring, U+00C5 ISOlat1 */
  {"Atilde",     195}, /* latin capital letter A with tilde, U+00C3 ISOlat1 */
  {"Auml",       196}, /* latin capital letter A with diaeresis, U+00C4 ISOlat1 */
  {"Beta",       914}, /* greek capital letter beta, U+0392 */
  {"Ccedil",     199}, /* latin capital letter C with cedilla, U+00C7 ISOlat1 */
  {"Chi",        935}, /* greek capital letter chi, U+03A7 */
  {"Dagger",    8225}, /* double dagger, U+2021 ISOpub */
  {"Delta",      916}, /* greek capital letter delta, U+0394 ISOgrk3 */
  {"ETH",        208}, /* latin capital letter ETH, U+00D0 ISOlat1 */
  {"Eacute",     201}, /* latin capital letter E with acute, U+00C9 ISOlat1 */
  {"Ecirc",      202}, /* latin capital letter E with circumflex, U+00CA ISOlat1 */
  {"Egrave",     200}, /* latin capital letter E with grave, U+00C8 ISOlat1 */
  {"Epsilon",    917}, /* greek capital letter epsilon, U+0395 */
  {"Eta",        919}, /* greek capital letter eta, U+0397 */
  {"Euml",       203}, /* latin capital letter E with diaeresis, U+00CB ISOlat1 */
  {"Gamma",      915}, /* greek capital letter gamma, U+0393 ISOgrk3 */
  {"Iacute",     205}, /* latin capital letter I with acute, U+00CD ISOlat1 */
  {"Icirc",      206}, /* latin capital letter I with circumflex, U+00CE ISOlat1 */
  {"Igrave",     204}, /* latin capital letter I with grave, U+00CC ISOlat1 */
  {"Iota",       921}, /* greek capital letter iota, U+0399 */
  {"Iuml",       207}, /* latin capital letter I with diaeresis, U+00CF ISOlat1 */
  {"Kappa",      922}, /* greek capital letter kappa, U+039A */
  {"Lambda",     923}, /* greek capital letter lambda, U+039B ISOgrk3 */
  {"Mu",         924}, /* greek capital letter mu, U+039C */
  {"Ntilde",     209}, /* latin capital letter N with tilde, U+00D1 ISOlat1 */
  {"Nu",         925}, /* greek capital letter nu, U+039D */
  {"OElig",      338}, /* latin capital ligature OE, U+0152 ISOlat2 */
  {"Oacute",     211}, /* latin capital letter O with acute, U+00D3 ISOlat1 */
  {"Ocirc",      212}, /* latin capital letter O with circumflex, U+00D4 ISOlat1 */
  {"Ograve",     210}, /* latin capital letter O with grave, U+00D2 ISOlat1 */
  {"Omega",      937}, /* greek capital letter omega, U+03A9 ISOgrk3 */
  {"Omicron",    927}, /* greek capital letter omicron, U+039F */
  {"Oslash",     216}, /* latin capital letter O with stroke = latin capital letter O slash, U+00D8 ISOlat1 */
  {"Otilde",     213}, /* latin capital letter O with tilde, U+00D5 ISOlat1 */
  {"Ouml",       214}, /* latin capital letter O with diaeresis, U+00D6 ISOlat1 */
  {"Phi",        934}, /* greek capital letter phi, U+03A6 ISOgrk3 */
  {"Pi",         928}, /* greek capital letter pi, U+03A0 ISOgrk3 */
  {"Prime",     8243}, /* double prime = seconds = inches, U+2033 ISOtech */
  {"Psi",        936}, /* greek capital letter psi, U+03A8 ISOgrk3 */
  {"Rho",        929}, /* greek capital letter rho, U+03A1 */
  {"Scaron",     352}, /* latin capital letter S with caron, U+0160 ISOlat2 */
/* there is no Sigmaf, and no U+03A2 character either */
  {"Sigma",      931}, /* greek capital letter sigma, U+03A3 ISOgrk3 */
  {"THORN",      222}, /* latin capital letter THORN, U+00DE ISOlat1 */
  {"Tau",        932}, /* greek capital letter tau, U+03A4 */
  {"Theta",      920}, /* greek capital letter theta, U+0398 ISOgrk3 */
  {"Uacute",     218}, /* latin capital letter U with acute, U+00DA ISOlat1 */
  {"Ucirc",      219}, /* latin capital letter U with circumflex, U+00DB ISOlat1 */
  {"Ugrave",     217}, /* latin capital letter U with grave, U+00D9 ISOlat1 */
  {"Upsilon",    933}, /* greek capital letter upsilon, U+03A5 ISOgrk3 */
  {"Uuml",       220}, /* latin capital letter U with diaeresis, U+00DC ISOlat1 */
  {"Xi",         926}, /* greek capital letter xi, U+039E ISOgrk3 */
  {"Yacute",     221}, /* latin capital letter Y with acute, U+00DD ISOlat1 */
  {"Yuml",       376}, /* latin capital letter Y with diaeresis, U+0178 ISOlat2 */
  {"Zeta",       918}, /* greek capital letter zeta, U+0396 */
  {"aacute",     225}, /* latin small letter a with acute, U+00E1 ISOlat1 */
  {"acirc",      226}, /* latin small letter a with circumflex, U+00E2 ISOlat1 */
  {"acute",      180}, /* acute accent = spacing acute, U+00B4 ISOdia */
  {"aelig",      230}, /* latin small letter ae = latin small ligature ae, U+00E6 ISOlat1 */
  {"agrave",     224}, /* latin small letter a with grave = latin small letter a grave, U+00E0 ISOlat1 */
  {"alefsym",   8501}, /* alef symbol = first transfinite cardinal, U+2135 NEW */
/* alef symbol is NOT the same as hebrew letter alef, U+05D0 although the same glyph could be used to depict both characters */
  {"alpha",      945}, /* greek small letter alpha, U+03B1 ISOgrk3 */
  {"amp",         38}, /* ampersand, U+0026 ISOnum */
  {"and",       8743}, /* logical and = wedge, U+2227 ISOtech */
  {"ang",       8736}, /* angle, U+2220 ISOamso */
  {"aring",      229}, /* latin small letter a with ring above = latin small letter a ring, U+00E5 ISOlat1 */
  {"asymp",     8776}, /* almost equal to = asymptotic to, U+2248 ISOamsr */
  {"atilde",     227}, /* latin small letter a with tilde, U+00E3 ISOlat1 */
  {"auml",       228}, /* latin small letter a with diaeresis, U+00E4 ISOlat1 */
  {"bdquo",     8222}, /* double low-9 quotation mark, U+201E NEW */
  {"beta",       946}, /* greek small letter beta, U+03B2 ISOgrk3 */
  {"brvbar",     166}, /* broken bar = broken vertical bar, U+00A6 ISOnum */
  {"bull",      8226}, /* bullet = black small circle, U+2022 ISOpub  */
/* bullet is NOT the same as bullet operator, U+2219 */
  {"cap",       8745}, /* intersection = cap, U+2229 ISOtech */
  {"ccedil",     231}, /* latin small letter c with cedilla, U+00E7 ISOlat1 */
  {"cedil",      184}, /* cedilla = spacing cedilla, U+00B8 ISOdia */
  {"cent",       162}, /* cent sign, U+00A2 ISOnum */
  {"chi",        967}, /* greek small letter chi, U+03C7 ISOgrk3 */
  {"circ",       710}, /* modifier letter circumflex accent, U+02C6 ISOpub */
  {"clubs",     9827}, /* black club suit = shamrock, U+2663 ISOpub */
  {"cong",      8773}, /* approximately equal to, U+2245 ISOtech */
  {"copy",       169}, /* copyright sign, U+00A9 ISOnum */
  {"crarr",     8629}, /* downwards arrow with corner leftwards = carriage return, U+21B5 NEW */
  {"cup",       8746}, /* union = cup, U+222A ISOtech */
  {"curren",     164}, /* currency sign, U+00A4 ISOnum */
  {"dArr",      8659}, /* downwards double arrow, U+21D3 ISOamsa */
  {"dagger",    8224}, /* dagger, U+2020 ISOpub */
  {"darr",      8595}, /* downwards arrow, U+2193 ISOnum */
  {"deg",        176}, /* degree sign, U+00B0 ISOnum */
  {"delta",      948}, /* greek small letter delta, U+03B4 ISOgrk3 */
  {"diams",     9830}, /* black diamond suit, U+2666 ISOpub */
  {"divide",     247}, /* division sign, U+00F7 ISOnum */
  {"eacute",     233}, /* latin small letter e with acute, U+00E9 ISOlat1 */
  {"ecirc",      234}, /* latin small letter e with circumflex, U+00EA ISOlat1 */
  {"egrave",     232}, /* latin small letter e with grave, U+00E8 ISOlat1 */
  {"empty",     8709}, /* empty set = null set = diameter, U+2205 ISOamso */
  {"emsp",      8195}, /* em space, U+2003 ISOpub */
  {"ensp",      8194}, /* en space, U+2002 ISOpub */
  {"epsilon",    949}, /* greek small letter epsilon, U+03B5 ISOgrk3 */
  {"equiv",     8801}, /* identical to, U+2261 ISOtech */
  {"eta",        951}, /* greek small letter eta, U+03B7 ISOgrk3 */
  {"eth",        240}, /* latin small letter eth, U+00F0 ISOlat1 */
  {"euml",       235}, /* latin small letter e with diaeresis, U+00EB ISOlat1 */
  {"euro",      8364}, /* euro sign, U+20AC NEW */
  {"exist",     8707}, /* there exists, U+2203 ISOtech */
  {"fnof",       402}, /* latin small f with hook = function = florin, U+0192 ISOtech */
  {"forall",    8704}, /* for all, U+2200 ISOtech */
  {"frac12",     189}, /* vulgar fraction one half = fraction one half, U+00BD ISOnum */
  {"frac14",     188}, /* vulgar fraction one quarter = fraction one quarter, U+00BC ISOnum */
  {"frac34",     190}, /* vulgar fraction three quarters = fraction three quarters, U+00BE ISOnum */
  {"frasl",     8260}, /* fraction slash, U+2044 NEW */
  {"gamma",      947}, /* greek small letter gamma, U+03B3 ISOgrk3 */
  {"ge",        8805}, /* greater-than or equal to, U+2265 ISOtech */
  {"gt",          62}, /* greater-than sign, U+003E ISOnum */
  {"hArr",      8660}, /* left right double arrow, U+21D4 ISOamsa */
  {"harr",      8596}, /* left right arrow, U+2194 ISOamsa */
  {"hearts",    9829}, /* black heart suit = valentine, U+2665 ISOpub */
  {"hellip",    8230}, /* horizontal ellipsis = three dot leader, U+2026 ISOpub  */
  {"iacute",     237}, /* latin small letter i with acute, U+00ED ISOlat1 */
  {"icirc",      238}, /* latin small letter i with circumflex, U+00EE ISOlat1 */
  {"iexcl",      161}, /* inverted exclamation mark, U+00A1 ISOnum */
  {"igrave",     236}, /* latin small letter i with grave, U+00EC ISOlat1 */
  {"image",     8465}, /* blackletter capital I = imaginary part, U+2111 ISOamso */
  {"infin",     8734}, /* infinity, U+221E ISOtech */
  {"int",       8747}, /* integral, U+222B ISOtech */
  {"iota",       953}, /* greek small letter iota, U+03B9 ISOgrk3 */
  {"iquest",     191}, /* inverted question mark = turned question mark, U+00BF ISOnum */
  {"isin",      8712}, /* element of, U+2208 ISOtech */
  {"iuml",       239}, /* latin small letter i with diaeresis, U+00EF ISOlat1 */
  {"kappa",      954}, /* greek small letter kappa, U+03BA ISOgrk3 */
  {"lArr",      8656}, /* leftwards double arrow, U+21D0 ISOtech */
/* Unicode does not say that lArr is the same as the 'is implied by' arrow
    but also does not have any other character for that function. So ? lArr can
    be used for 'is implied by' as ISOtech suggests */
  {"lambda",     955}, /* greek small letter lambda, U+03BB ISOgrk3 */
  {"lang",      9001}, /* left-pointing angle bracket = bra, U+2329 ISOtech */
/* lang is NOT the same character as U+003C 'less than' or U+2039 'single left-pointing angle quotation mark' */
  {"laquo",      171}, /* left-pointing double angle quotation mark = left pointing guillemet, U+00AB ISOnum */
  {"larr",      8592}, /* leftwards arrow, U+2190 ISOnum */
  {"lceil",     8968}, /* left ceiling = apl upstile, U+2308 ISOamsc  */
  {"ldquo",     8220}, /* left double quotation mark, U+201C ISOnum */
  {"le",        8804}, /* less-than or equal to, U+2264 ISOtech */
  {"lfloor",    8970}, /* left floor = apl downstile, U+230A ISOamsc  */
  {"lowast",    8727}, /* asterisk operator, U+2217 ISOtech */
  {"loz",       9674}, /* lozenge, U+25CA ISOpub */
  {"lrm",       8206}, /* left-to-right mark, U+200E NEW RFC 2070 */
  {"lsaquo",    8249}, /* single left-pointing angle quotation mark, U+2039 ISO proposed */
/* lsaquo is proposed but not yet ISO standardised */
  {"lsquo",     8216}, /* left single quotation mark, U+2018 ISOnum */
  {"lt",          60}, /* less-than sign, U+003C ISOnum */
  {"macr",       175}, /* macron = spacing macron = overline = APL overbar, U+00AF ISOdia */
  {"mdash",     8212}, /* em dash, U+2014 ISOpub */
  {"micro",      181}, /* micro sign, U+00B5 ISOnum */
  {"middot",     183}, /* middle dot = Georgian comma = Greek middle dot, U+00B7 ISOnum */
  {"minus",     8722}, /* minus sign, U+2212 ISOtech */
  {"mu",         956}, /* greek small letter mu, U+03BC ISOgrk3 */
  {"nabla",     8711}, /* nabla = backward difference, U+2207 ISOtech */
  {"nbsp",       160}, /* no-break space = non-breaking space, U+00A0 ISOnum */
  {"ndash",     8211}, /* en dash, U+2013 ISOpub */
  {"ne",        8800}, /* not equal to, U+2260 ISOtech */
  {"ni",        8715}, /* contains as member, U+220B ISOtech */
/* should there be a more memorable name than 'ni'? */
  {"not",        172}, /* not sign = discretionary hyphen, U+00AC ISOnum */
  {"notin",     8713}, /* not an element of, U+2209 ISOtech */
  {"nsub",      8836}, /* not a subset of, U+2284 ISOamsn */
  {"ntilde",     241}, /* latin small letter n with tilde, U+00F1 ISOlat1 */
  {"nu",         957}, /* greek small letter nu, U+03BD ISOgrk3 */
  {"oacute",     243}, /* latin small letter o with acute, U+00F3 ISOlat1 */
  {"ocirc",      244}, /* latin small letter o with circumflex, U+00F4 ISOlat1 */
  {"oelig",      339}, /* latin small ligature oe, U+0153 ISOlat2 */
  {"ograve",     242}, /* latin small letter o with grave, U+00F2 ISOlat1 */
  {"oline",     8254}, /* overline = spacing overscore, U+203E NEW */
  {"omega",      969}, /* greek small letter omega, U+03C9 ISOgrk3 */
  {"omicron",    959}, /* greek small letter omicron, U+03BF NEW */
  {"oplus",     8853}, /* circled plus = direct sum, U+2295 ISOamsb */
  {"or",        8744}, /* logical or = vee, U+2228 ISOtech */
  {"ordf",       170}, /* feminine ordinal indicator, U+00AA ISOnum */
  {"ordm",       186}, /* masculine ordinal indicator, U+00BA ISOnum */
  {"oslash",     248}, /* latin small letter o with stroke, = latin small letter o slash, U+00F8 ISOlat1 */
  {"otilde",     245}, /* latin small letter o with tilde, U+00F5 ISOlat1 */
  {"otimes",    8855}, /* circled times = vector product, U+2297 ISOamsb */
  {"ouml",       246}, /* latin small letter o with diaeresis, U+00F6 ISOlat1 */
  {"para",       182}, /* pilcrow sign = paragraph sign, U+00B6 ISOnum */
  {"part",      8706}, /* partial differential, U+2202 ISOtech  */
  {"permil",    8240}, /* per mille sign, U+2030 ISOtech */
  {"perp",      8869}, /* up tack = orthogonal to = perpendicular, U+22A5 ISOtech */
  {"phi",        966}, /* greek small letter phi, U+03C6 ISOgrk3 */
  {"pi",         960}, /* greek small letter pi, U+03C0 ISOgrk3 */
  {"piv",        982}, /* greek pi symbol, U+03D6 ISOgrk3 */
  {"plusmn",     177}, /* plus-minus sign = plus-or-minus sign, U+00B1 ISOnum */
  {"pound",      163}, /* pound sign, U+00A3 ISOnum */
  {"prime",     8242}, /* prime = minutes = feet, U+2032 ISOtech */
  {"prod",      8719}, /* n-ary product = product sign, U+220F ISOamsb */
/* prod is NOT the same character as U+03A0 'greek capital letter pi' though the same glyph might be used for both */
  {"prop",      8733}, /* proportional to, U+221D ISOtech */
  {"psi",        968}, /* greek small letter psi, U+03C8 ISOgrk3 */
  {"quot",        34}, /* quotation mark = APL quote, U+0022 ISOnum */
  {"rArr",      8658}, /* rightwards double arrow, U+21D2 ISOtech */
/* Unicode does not say this is the 'implies' character but does not have
     another character with this function so ?
     rArr can be used for 'implies' as ISOtech suggests */
  {"radic",     8730}, /* square root = radical sign, U+221A ISOtech */
  {"rang",      9002}, /* right-pointing angle bracket = ket, U+232A ISOtech */
/* rang is NOT the same character as U+003E 'greater than' or U+203A 'single right-pointing angle quotation mark' */
  {"raquo",      187}, /* right-pointing double angle quotation mark = right pointing guillemet, U+00BB ISOnum */
  {"rarr",      8594}, /* rightwards arrow, U+2192 ISOnum */
  {"rceil",     8969}, /* right ceiling, U+2309 ISOamsc  */
  {"rdquo",     8221}, /* right double quotation mark, U+201D ISOnum */
  {"real",      8476}, /* blackletter capital R = real part symbol, U+211C ISOamso */
  {"reg",        174}, /* registered sign = registered trade mark sign, U+00AE ISOnum */
  {"rfloor",    8971}, /* right floor, U+230B ISOamsc  */
  {"rho",        961}, /* greek small letter rho, U+03C1 ISOgrk3 */
  {"rlm",       8207}, /* right-to-left mark, U+200F NEW RFC 2070 */
  {"rsaquo",    8250}, /* single right-pointing angle quotation mark, U+203A ISO proposed */
/* rsaquo is proposed but not yet ISO standardised */
  {"rsquo",     8217}, /* right single quotation mark, U+2019 ISOnum */
  {"sbquo",     8218}, /* single low-9 quotation mark, U+201A NEW */
  {"scaron",     353}, /* latin small letter s with caron, U+0161 ISOlat2 */
  {"sdot",      8901}, /* dot operator, U+22C5 ISOamsb */
/* dot operator is NOT the same character as U+00B7 middle dot */
  {"sect",       167}, /* section sign, U+00A7 ISOnum */
  {"shy",        173}, /* soft hyphen = discretionary hyphen, U+00AD ISOnum */
  {"sigma",      963}, /* greek small letter sigma, U+03C3 ISOgrk3 */
  {"sigmaf",     962}, /* greek small letter final sigma, U+03C2 ISOgrk3 */
  {"sim",       8764}, /* tilde operator = varies with = similar to, U+223C ISOtech */
/* tilde operator is NOT the same character as the tilde, U+007E, although the same glyph might be used to represent both */
  {"spades",    9824}, /* black spade suit, U+2660 ISOpub */
/* black here seems to mean filled as opposed to hollow */
  {"sub",       8834}, /* subset of, U+2282 ISOtech */
  {"sube",      8838}, /* subset of or equal to, U+2286 ISOtech */
  {"sum",       8721}, /* n-ary sumation, U+2211 ISOamsb */
/* sum is NOT the same character as U+03A3 'greek capital letter sigma' though the same glyph might be used for both */
  {"sup",       8835}, /* superset of, U+2283 ISOtech */
/* note that nsup, 'not a superset of, U+2283' is not covered by the Symbol
     font encoding and is not included. Should it be, for symmetry?
     It is in ISOamsn */
  {"sup1",       185}, /* superscript one = superscript digit one, U+00B9 ISOnum */
  {"sup2",       178}, /* superscript two = superscript digit two = squared, U+00B2 ISOnum */
  {"sup3",       179}, /* superscript three = superscript digit three = cubed, U+00B3 ISOnum */
  {"supe",      8839}, /* superset of or equal to, U+2287 ISOtech */
  {"szlig",      223}, /* latin small letter sharp s = ess-zed,  U+00DF ISOlat1 */
  {"tau",        964}, /* greek small letter tau, U+03C4 ISOgrk3 */
  {"there4",    8756}, /* therefore, U+2234 ISOtech */
  {"theta",      952}, /* greek small letter theta, U+03B8 ISOgrk3 */
  {"thetasym",   977}, /* greek small letter theta symbol, U+03D1 NEW */
  {"thinsp",    8201}, /* thin space, U+2009 ISOpub */
  {"thorn",      254}, /* latin small letter thorn with, U+00FE ISOlat1 */
  {"tilde",      732}, /* small tilde, U+02DC ISOdia */
  {"times",      215}, /* multiplication sign, U+00D7 ISOnum */
  {"trade",     8482}, /* trade mark sign, U+2122 ISOnum */
  {"uArr",      8657}, /* upwards double arrow, U+21D1 ISOamsa */
  {"uacute",     250}, /* latin small letter u with acute, U+00FA ISOlat1 */
  {"uarr",      8593}, /* upwards arrow, U+2191 ISOnum */
  {"ucirc",      251}, /* latin small letter u with circumflex, U+00FB ISOlat1 */
  {"ugrave",     249}, /* latin small letter u with grave, U+00F9 ISOlat1 */
  {"uml",        168}, /* diaeresis = spacing diaeresis, U+00A8 ISOdia */
  {"upsih",      978}, /* greek upsilon with hook symbol, U+03D2 NEW */
  {"upsilon",    965}, /* greek small letter upsilon, U+03C5 ISOgrk3 */
  {"uuml",       252}, /* latin small letter u with diaeresis, U+00FC ISOlat1 */
  {"weierp",    8472}, /* script capital P = power set = Weierstrass p, U+2118 ISOamso */
  {"xi",         958}, /* greek small letter xi, U+03BE ISOgrk3 */
  {"yacute",     253}, /* latin small letter y with acute, U+00FD ISOlat1 */
  {"yen",        165}, /* yen sign = yuan sign, U+00A5 ISOnum */
  {"yuml",       255}, /* latin small letter y with diaeresis, U+00FF ISOlat1 */
  {"zeta",       950}, /* greek small letter zeta, U+03B6 ISOgrk3 */
  {"zwj",       8205}, /* zero width joiner, U+200D NEW RFC 2070 */
  {"zwnj",      8204}, /* zero width non-joiner, U+200C NEW RFC 2070 */
};

#else /* not ENTITIES_HTML40_ONLY: */
/***************************************************************************

This table prepared from ftp://ftp.unicode.org/MAPPINGS/VENDORS/MISC/SGML.TXT
original comment follows:


# Author: John Cowan <cowan@ccil.org>
# Date: 25 July 1997
#
# The following table maps SGML character entities from various
# public sets (namely, ISOamsa, ISOamsb, ISOamsc, ISOamsn, ISOamso,
# ISOamsr, ISObox, ISOcyr1, ISOcyr2, ISOdia, ISOgrk1, ISOgrk2,
# ISOgrk3, ISOgrk4, ISOlat1, ISOlat2, ISOnum, ISOpub, ISOtech,
# HTMLspecial, HTMLsymbol) to corresponding Unicode characters.
#
# The table has four tab-separated columns:
#	Column 1: SGML character entity name
#	Column 2: SGML public entity set
#	Column 3: Unicode 2.0 character code
#	Column 4: Unicode 2.0 character name (UPPER CASE)
# Entries which don't have Unicode equivalents have "0x????"
# in Column 3 and a lower case description (from the public entity
# set DTD) in Column 4.  The mapping is not reversible, because many
# distinctions are unified away in Unicode, particularly between
# mathematical symbols.


   We just sort it and move column 2 away (line too long, sorry;
   look at sgml.html in test/ directory for details).

Changes:
   * Add few (obsolete) synonyms for compatibility with Lynx/2.5 and up:
          "brkbar"  for "brvbar" 0x00A6
          "emdash"  for "mdash" 0x2014
          "endash"  for "ndash" 0x2013
          "hibar"  for "macr" 0x00AF
     BTW, lots of synonyms found in this table, we shouldn't worry about...
     Totally around 1000 entries.


Modified by Jacob Poon <jacob.poon@utoronto.ca>

This table is modified improve support of HTML 4.0 character entity references,
including Euro symbol support ("euro" 0x20AC).

Known issues:

The original table includes two different definitions of &loz; reference.
Since HTML 4.0 only uses U+25CA, the U+2727 definition is commented out,
until there is a good reason to put it back in.

"b.delta" mapping fixed (was 0x03B3 = small gamma).

At the end of the table, there are several unnumbered, commented references.
These are not defined in HTML 4.0, and will remain so until they are defined
in future SGML/HTML standards.

The support for obsolete references are for backwards compatibility only.  New
SGML/HTML documents should not depend on these references just because Lynx can
display them.

****/
{
  {"AElig",	0x00C6},  /* LATIN CAPITAL LETTER AE			   */
  {"Aacgr",	0x0386},  /* GREEK CAPITAL LETTER ALPHA WITH TONOS	   */
  {"Aacute",	0x00C1},  /* LATIN CAPITAL LETTER A WITH ACUTE		   */
  {"Abreve",	0x0102},  /* LATIN CAPITAL LETTER A WITH BREVE		   */
  {"Acirc",	0x00C2},  /* LATIN CAPITAL LETTER A WITH CIRCUMFLEX	   */
  {"Acy",	0x0410},  /* CYRILLIC CAPITAL LETTER A			   */
  {"Agr",	0x0391},  /* GREEK CAPITAL LETTER ALPHA 		   */
  {"Agrave",	0x00C0},  /* LATIN CAPITAL LETTER A WITH GRAVE		   */
  {"Alpha",	0x0391},  /* GREEK CAPITAL LETTER ALPHA 		   */
  {"Amacr",	0x0100},  /* LATIN CAPITAL LETTER A WITH MACRON 	   */
  {"Aogon",	0x0104},  /* LATIN CAPITAL LETTER A WITH OGONEK 	   */
  {"Aring",	0x00C5},  /* LATIN CAPITAL LETTER A WITH RING ABOVE	   */
  {"Atilde",	0x00C3},  /* LATIN CAPITAL LETTER A WITH TILDE		   */
  {"Auml",	0x00C4},  /* LATIN CAPITAL LETTER A WITH DIAERESIS	   */
  {"Barwed",	0x2306},  /* PERSPECTIVE				   */
  {"Bcy",	0x0411},  /* CYRILLIC CAPITAL LETTER BE 		   */
  {"Beta",	0x0392},  /* GREEK CAPITAL LETTER BETA			   */
  {"Bgr",	0x0392},  /* GREEK CAPITAL LETTER BETA			   */
  {"CHcy",	0x0427},  /* CYRILLIC CAPITAL LETTER CHE		   */
  {"Cacute",	0x0106},  /* LATIN CAPITAL LETTER C WITH ACUTE		   */
  {"Cap",	0x22D2},  /* DOUBLE INTERSECTION			   */
  {"Ccaron",	0x010C},  /* LATIN CAPITAL LETTER C WITH CARON		   */
  {"Ccedil",	0x00C7},  /* LATIN CAPITAL LETTER C WITH CEDILLA	   */
  {"Ccirc",	0x0108},  /* LATIN CAPITAL LETTER C WITH CIRCUMFLEX	   */
  {"Cdot",	0x010A},  /* LATIN CAPITAL LETTER C WITH DOT ABOVE	   */
  {"Chi",	0x03A7},  /* GREEK CAPITAL LETTER CHI			   */
  {"Cup",	0x22D3},  /* DOUBLE UNION				   */
  {"DJcy",	0x0402},  /* CYRILLIC CAPITAL LETTER DJE		   */
  {"DScy",	0x0405},  /* CYRILLIC CAPITAL LETTER DZE		   */
  {"DZcy",	0x040F},  /* CYRILLIC CAPITAL LETTER DZHE		   */
  {"Dagger",	0x2021},  /* DOUBLE DAGGER				   */
  {"Dcaron",	0x010E},  /* LATIN CAPITAL LETTER D WITH CARON		   */
  {"Dcy",	0x0414},  /* CYRILLIC CAPITAL LETTER DE 		   */
  {"Delta",	0x0394},  /* GREEK CAPITAL LETTER DELTA 		   */
  {"Dgr",	0x0394},  /* GREEK CAPITAL LETTER DELTA 		   */
  {"Dot",	0x00A8},  /* DIAERESIS					   */
  {"DotDot",	0x20DC},  /* COMBINING FOUR DOTS ABOVE			   */
  {"Dstrok",	0x0110},  /* LATIN CAPITAL LETTER D WITH STROKE 	   */
  {"EEacgr",	0x0389},  /* GREEK CAPITAL LETTER ETA WITH TONOS	   */
  {"EEgr",	0x0397},  /* GREEK CAPITAL LETTER ETA			   */
  {"ENG",	0x014A},  /* LATIN CAPITAL LETTER ENG			   */
  {"ETH",	0x00D0},  /* LATIN CAPITAL LETTER ETH			   */
  {"Eacgr",	0x0388},  /* GREEK CAPITAL LETTER EPSILON WITH TONOS	   */
  {"Eacute",	0x00C9},  /* LATIN CAPITAL LETTER E WITH ACUTE		   */
  {"Ecaron",	0x011A},  /* LATIN CAPITAL LETTER E WITH CARON		   */
  {"Ecirc",	0x00CA},  /* LATIN CAPITAL LETTER E WITH CIRCUMFLEX	   */
  {"Ecy",	0x042D},  /* CYRILLIC CAPITAL LETTER E			   */
  {"Edot",	0x0116},  /* LATIN CAPITAL LETTER E WITH DOT ABOVE	   */
  {"Egr",	0x0395},  /* GREEK CAPITAL LETTER EPSILON		   */
  {"Egrave",	0x00C8},  /* LATIN CAPITAL LETTER E WITH GRAVE		   */
  {"Emacr",	0x0112},  /* LATIN CAPITAL LETTER E WITH MACRON 	   */
  {"Eogon",	0x0118},  /* LATIN CAPITAL LETTER E WITH OGONEK 	   */
  {"Epsilon",	0x0395},  /* GREEK CAPITAL LETTER EPSILON		   */
  {"Eta",	0x0397},  /* GREEK CAPITAL LETTER ETA			   */
  {"Euml",	0x00CB},  /* LATIN CAPITAL LETTER E WITH DIAERESIS	   */
  {"Fcy",	0x0424},  /* CYRILLIC CAPITAL LETTER EF 		   */
  {"GJcy",	0x0403},  /* CYRILLIC CAPITAL LETTER GJE		   */
  {"Gamma",	0x0393},  /* GREEK CAPITAL LETTER GAMMA 		   */
  {"Gbreve",	0x011E},  /* LATIN CAPITAL LETTER G WITH BREVE		   */
  {"Gcedil",	0x0122},  /* LATIN CAPITAL LETTER G WITH CEDILLA	   */
  {"Gcirc",	0x011C},  /* LATIN CAPITAL LETTER G WITH CIRCUMFLEX	   */
  {"Gcy",	0x0413},  /* CYRILLIC CAPITAL LETTER GHE		   */
  {"Gdot",	0x0120},  /* LATIN CAPITAL LETTER G WITH DOT ABOVE	   */
  {"Gg",	0x22D9},  /* VERY MUCH GREATER-THAN			   */
  {"Ggr",	0x0393},  /* GREEK CAPITAL LETTER GAMMA 		   */
  {"Gt",	0x226B},  /* MUCH GREATER-THAN				   */
  {"HARDcy",	0x042A},  /* CYRILLIC CAPITAL LETTER HARD SIGN		   */
  {"Hcirc",	0x0124},  /* LATIN CAPITAL LETTER H WITH CIRCUMFLEX	   */
  {"Hstrok",	0x0126},  /* LATIN CAPITAL LETTER H WITH STROKE 	   */
  {"IEcy",	0x0415},  /* CYRILLIC CAPITAL LETTER IE 		   */
  {"IJlig",	0x0132},  /* LATIN CAPITAL LIGATURE IJ			   */
  {"IOcy",	0x0401},  /* CYRILLIC CAPITAL LETTER IO 		   */
  {"Iacgr",	0x038A},  /* GREEK CAPITAL LETTER IOTA WITH TONOS	   */
  {"Iacute",	0x00CD},  /* LATIN CAPITAL LETTER I WITH ACUTE		   */
  {"Icirc",	0x00CE},  /* LATIN CAPITAL LETTER I WITH CIRCUMFLEX	   */
  {"Icy",	0x0418},  /* CYRILLIC CAPITAL LETTER I			   */
  {"Idigr",	0x03AA},  /* GREEK CAPITAL LETTER IOTA WITH DIALYTIKA	   */
  {"Idot",	0x0130},  /* LATIN CAPITAL LETTER I WITH DOT ABOVE	   */
  {"Igr",	0x0399},  /* GREEK CAPITAL LETTER IOTA			   */
  {"Igrave",	0x00CC},  /* LATIN CAPITAL LETTER I WITH GRAVE		   */
  {"Imacr",	0x012A},  /* LATIN CAPITAL LETTER I WITH MACRON 	   */
  {"Iogon",	0x012E},  /* LATIN CAPITAL LETTER I WITH OGONEK 	   */
  {"Iota",	0x0399},  /* GREEK CAPITAL LETTER IOTA			   */
  {"Itilde",	0x0128},  /* LATIN CAPITAL LETTER I WITH TILDE		   */
  {"Iukcy",	0x0406},  /* CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN*/
  {"Iuml",	0x00CF},  /* LATIN CAPITAL LETTER I WITH DIAERESIS	   */
  {"Jcirc",	0x0134},  /* LATIN CAPITAL LETTER J WITH CIRCUMFLEX	   */
  {"Jcy",	0x0419},  /* CYRILLIC CAPITAL LETTER SHORT I		   */
  {"Jsercy",	0x0408},  /* CYRILLIC CAPITAL LETTER JE 		   */
  {"Jukcy",	0x0404},  /* CYRILLIC CAPITAL LETTER UKRAINIAN IE	   */
  {"KHcy",	0x0425},  /* CYRILLIC CAPITAL LETTER HA 		   */
  {"KHgr",	0x03A7},  /* GREEK CAPITAL LETTER CHI			   */
  {"KJcy",	0x040C},  /* CYRILLIC CAPITAL LETTER KJE		   */
  {"Kappa",	0x039A},  /* GREEK CAPITAL LETTER KAPPA 		   */
  {"Kcedil",	0x0136},  /* LATIN CAPITAL LETTER K WITH CEDILLA	   */
  {"Kcy",	0x041A},  /* CYRILLIC CAPITAL LETTER KA 		   */
  {"Kgr",	0x039A},  /* GREEK CAPITAL LETTER KAPPA 		   */
  {"LJcy",	0x0409},  /* CYRILLIC CAPITAL LETTER LJE		   */
  {"Lacute",	0x0139},  /* LATIN CAPITAL LETTER L WITH ACUTE		   */
  {"Lambda",	0x039B},  /* GREEK CAPITAL LETTER LAMDA 		   */
  {"Larr",	0x219E},  /* LEFTWARDS TWO HEADED ARROW 		   */
  {"Lcaron",	0x013D},  /* LATIN CAPITAL LETTER L WITH CARON		   */
  {"Lcedil",	0x013B},  /* LATIN CAPITAL LETTER L WITH CEDILLA	   */
  {"Lcy",	0x041B},  /* CYRILLIC CAPITAL LETTER EL 		   */
  {"Lgr",	0x039B},  /* GREEK CAPITAL LETTER LAMDA 		   */
  {"Ll",	0x22D8},  /* VERY MUCH LESS-THAN			   */
  {"Lmidot",	0x013F},  /* LATIN CAPITAL LETTER L WITH MIDDLE DOT	   */
  {"Lstrok",	0x0141},  /* LATIN CAPITAL LETTER L WITH STROKE 	   */
  {"Lt",	0x226A},  /* MUCH LESS-THAN				   */
  {"Mcy",	0x041C},  /* CYRILLIC CAPITAL LETTER EM 		   */
  {"Mgr",	0x039C},  /* GREEK CAPITAL LETTER MU			   */
  {"Mu",	0x039C},  /* GREEK CAPITAL LETTER MU			   */
  {"NJcy",	0x040A},  /* CYRILLIC CAPITAL LETTER NJE		   */
  {"Nacute",	0x0143},  /* LATIN CAPITAL LETTER N WITH ACUTE		   */
  {"Ncaron",	0x0147},  /* LATIN CAPITAL LETTER N WITH CARON		   */
  {"Ncedil",	0x0145},  /* LATIN CAPITAL LETTER N WITH CEDILLA	   */
  {"Ncy",	0x041D},  /* CYRILLIC CAPITAL LETTER EN 		   */
  {"Ngr",	0x039D},  /* GREEK CAPITAL LETTER NU			   */
  {"Ntilde",	0x00D1},  /* LATIN CAPITAL LETTER N WITH TILDE		   */
  {"Nu",	0x039D},  /* GREEK CAPITAL LETTER NU			   */
  {"OElig",	0x0152},  /* LATIN CAPITAL LIGATURE OE			   */
  {"OHacgr",	0x038F},  /* GREEK CAPITAL LETTER OMEGA WITH TONOS	   */
  {"OHgr",	0x03A9},  /* GREEK CAPITAL LETTER OMEGA 		   */
  {"Oacgr",	0x038C},  /* GREEK CAPITAL LETTER OMICRON WITH TONOS	   */
  {"Oacute",	0x00D3},  /* LATIN CAPITAL LETTER O WITH ACUTE		   */
  {"Ocirc",	0x00D4},  /* LATIN CAPITAL LETTER O WITH CIRCUMFLEX	   */
  {"Ocy",	0x041E},  /* CYRILLIC CAPITAL LETTER O			   */
  {"Odblac",	0x0150},  /* LATIN CAPITAL LETTER O WITH DOUBLE ACUTE	   */
  {"Ogr",	0x039F},  /* GREEK CAPITAL LETTER OMICRON		   */
  {"Ograve",	0x00D2},  /* LATIN CAPITAL LETTER O WITH GRAVE		   */
  {"Omacr",	0x014C},  /* LATIN CAPITAL LETTER O WITH MACRON 	   */
  {"Omega",	0x03A9},  /* GREEK CAPITAL LETTER OMEGA 		   */
  {"Omicron",	0x039F},  /* GREEK CAPITAL LETTER OMICRON		   */
  {"Oslash",	0x00D8},  /* LATIN CAPITAL LETTER O WITH STROKE 	   */
  {"Otilde",	0x00D5},  /* LATIN CAPITAL LETTER O WITH TILDE		   */
  {"Ouml",	0x00D6},  /* LATIN CAPITAL LETTER O WITH DIAERESIS	   */
  {"PHgr",	0x03A6},  /* GREEK CAPITAL LETTER PHI			   */
  {"PSgr",	0x03A8},  /* GREEK CAPITAL LETTER PSI			   */
  {"Pcy",	0x041F},  /* CYRILLIC CAPITAL LETTER PE 		   */
  {"Pgr",	0x03A0},  /* GREEK CAPITAL LETTER PI			   */
  {"Phi",	0x03A6},  /* GREEK CAPITAL LETTER PHI			   */
  {"Pi",	0x03A0},  /* GREEK CAPITAL LETTER PI			   */
  {"Prime",	0x2033},  /* DOUBLE PRIME				   */
  {"Psi",	0x03A8},  /* GREEK CAPITAL LETTER PSI			   */
  {"Racute",	0x0154},  /* LATIN CAPITAL LETTER R WITH ACUTE		   */
  {"Rarr",	0x21A0},  /* RIGHTWARDS TWO HEADED ARROW		   */
  {"Rcaron",	0x0158},  /* LATIN CAPITAL LETTER R WITH CARON		   */
  {"Rcedil",	0x0156},  /* LATIN CAPITAL LETTER R WITH CEDILLA	   */
  {"Rcy",	0x0420},  /* CYRILLIC CAPITAL LETTER ER 		   */
  {"Rgr",	0x03A1},  /* GREEK CAPITAL LETTER RHO			   */
  {"Rho",	0x03A1},  /* GREEK CAPITAL LETTER RHO			   */
  {"SHCHcy",	0x0429},  /* CYRILLIC CAPITAL LETTER SHCHA		   */
  {"SHcy",	0x0428},  /* CYRILLIC CAPITAL LETTER SHA		   */
  {"SOFTcy",	0x042C},  /* CYRILLIC CAPITAL LETTER SOFT SIGN		   */
  {"Sacute",	0x015A},  /* LATIN CAPITAL LETTER S WITH ACUTE		   */
  {"Scaron",	0x0160},  /* LATIN CAPITAL LETTER S WITH CARON		   */
  {"Scedil",	0x015E},  /* LATIN CAPITAL LETTER S WITH CEDILLA	   */
  {"Scirc",	0x015C},  /* LATIN CAPITAL LETTER S WITH CIRCUMFLEX	   */
  {"Scy",	0x0421},  /* CYRILLIC CAPITAL LETTER ES 		   */
  {"Sgr",	0x03A3},  /* GREEK CAPITAL LETTER SIGMA 		   */
  {"Sigma",	0x03A3},  /* GREEK CAPITAL LETTER SIGMA 		   */
  {"Sub",	0x22D0},  /* DOUBLE SUBSET				   */
  {"Sup",	0x22D1},  /* DOUBLE SUPERSET				   */
  {"THORN",	0x00DE},  /* LATIN CAPITAL LETTER THORN 		   */
  {"THgr",	0x0398},  /* GREEK CAPITAL LETTER THETA 		   */
  {"TSHcy",	0x040B},  /* CYRILLIC CAPITAL LETTER TSHE		   */
  {"TScy",	0x0426},  /* CYRILLIC CAPITAL LETTER TSE		   */
  {"Tau",	0x03A4},  /* GREEK CAPITAL LETTER TAU			   */
  {"Tcaron",	0x0164},  /* LATIN CAPITAL LETTER T WITH CARON		   */
  {"Tcedil",	0x0162},  /* LATIN CAPITAL LETTER T WITH CEDILLA	   */
  {"Tcy",	0x0422},  /* CYRILLIC CAPITAL LETTER TE 		   */
  {"Tgr",	0x03A4},  /* GREEK CAPITAL LETTER TAU			   */
  {"Theta",	0x0398},  /* GREEK CAPITAL LETTER THETA 		   */
  {"Tstrok",	0x0166},  /* LATIN CAPITAL LETTER T WITH STROKE 	   */
  {"Uacgr",	0x038E},  /* GREEK CAPITAL LETTER UPSILON WITH TONOS	   */
  {"Uacute",	0x00DA},  /* LATIN CAPITAL LETTER U WITH ACUTE		   */
  {"Ubrcy",	0x040E},  /* CYRILLIC CAPITAL LETTER SHORT U		   */
  {"Ubreve",	0x016C},  /* LATIN CAPITAL LETTER U WITH BREVE		   */
  {"Ucirc",	0x00DB},  /* LATIN CAPITAL LETTER U WITH CIRCUMFLEX	   */
  {"Ucy",	0x0423},  /* CYRILLIC CAPITAL LETTER U			   */
  {"Udblac",	0x0170},  /* LATIN CAPITAL LETTER U WITH DOUBLE ACUTE	   */
  {"Udigr",	0x03AB},  /* GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA   */
  {"Ugr",	0x03A5},  /* GREEK CAPITAL LETTER UPSILON		   */
  {"Ugrave",	0x00D9},  /* LATIN CAPITAL LETTER U WITH GRAVE		   */
  {"Umacr",	0x016A},  /* LATIN CAPITAL LETTER U WITH MACRON 	   */
  {"Uogon",	0x0172},  /* LATIN CAPITAL LETTER U WITH OGONEK 	   */
  {"Upsi",	0x03A5},  /* GREEK CAPITAL LETTER UPSILON		   */
  {"Upsilon",	0x03A5},  /* GREEK CAPITAL LETTER UPSILON		   */
  {"Uring",	0x016E},  /* LATIN CAPITAL LETTER U WITH RING ABOVE	   */
  {"Utilde",	0x0168},  /* LATIN CAPITAL LETTER U WITH TILDE		   */
  {"Uuml",	0x00DC},  /* LATIN CAPITAL LETTER U WITH DIAERESIS	   */
  {"Vcy",	0x0412},  /* CYRILLIC CAPITAL LETTER VE 		   */
  {"Vdash",	0x22A9},  /* FORCES					   */
  {"Verbar",	0x2016},  /* DOUBLE VERTICAL LINE			   */
  {"Vvdash",	0x22AA},  /* TRIPLE VERTICAL BAR RIGHT TURNSTILE	   */
  {"Wcirc",	0x0174},  /* LATIN CAPITAL LETTER W WITH CIRCUMFLEX	   */
  {"Xgr",	0x039E},  /* GREEK CAPITAL LETTER XI			   */
  {"Xi",	0x039E},  /* GREEK CAPITAL LETTER XI			   */
  {"YAcy",	0x042F},  /* CYRILLIC CAPITAL LETTER YA 		   */
  {"YIcy",	0x0407},  /* CYRILLIC CAPITAL LETTER YI 		   */
  {"YUcy",	0x042E},  /* CYRILLIC CAPITAL LETTER YU 		   */
  {"Yacute",	0x00DD},  /* LATIN CAPITAL LETTER Y WITH ACUTE		   */
  {"Ycirc",	0x0176},  /* LATIN CAPITAL LETTER Y WITH CIRCUMFLEX	   */
  {"Ycy",	0x042B},  /* CYRILLIC CAPITAL LETTER YERU		   */
  {"Yuml",	0x0178},  /* LATIN CAPITAL LETTER Y WITH DIAERESIS	   */
  {"ZHcy",	0x0416},  /* CYRILLIC CAPITAL LETTER ZHE		   */
  {"Zacute",	0x0179},  /* LATIN CAPITAL LETTER Z WITH ACUTE		   */
  {"Zcaron",	0x017D},  /* LATIN CAPITAL LETTER Z WITH CARON		   */
  {"Zcy",	0x0417},  /* CYRILLIC CAPITAL LETTER ZE 		   */
  {"Zdot",	0x017B},  /* LATIN CAPITAL LETTER Z WITH DOT ABOVE	   */
  {"Zeta",	0x0396},  /* GREEK CAPITAL LETTER ZETA			   */
  {"Zgr",	0x0396},  /* GREEK CAPITAL LETTER ZETA			   */
  {"aacgr",	0x03AC},  /* GREEK SMALL LETTER ALPHA WITH TONOS	   */
  {"aacute",	0x00E1},  /* LATIN SMALL LETTER A WITH ACUTE		   */
  {"abreve",	0x0103},  /* LATIN SMALL LETTER A WITH BREVE		   */
  {"acirc",	0x00E2},  /* LATIN SMALL LETTER A WITH CIRCUMFLEX	   */
  {"acute",	0x00B4},  /* ACUTE ACCENT				   */
  {"acy",	0x0430},  /* CYRILLIC SMALL LETTER A			   */
  {"aelig",	0x00E6},  /* LATIN SMALL LETTER AE			   */
  {"agr",	0x03B1},  /* GREEK SMALL LETTER ALPHA			   */
  {"agrave",	0x00E0},  /* LATIN SMALL LETTER A WITH GRAVE		   */
  {"alefsym",	0x2135},  /* ALEF SYMBOL				   */
  {"aleph",	0x2135},  /* ALEF SYMBOL				   */
  {"alpha",	0x03B1},  /* GREEK SMALL LETTER ALPHA			   */
  {"amacr",	0x0101},  /* LATIN SMALL LETTER A WITH MACRON		   */
  {"amalg",	0x2210},  /* N-ARY COPRODUCT				   */
  {"amp",	0x0026},  /* AMPERSAND					   */
  {"and",	0x2227},  /* LOGICAL AND				   */
  {"ang",	0x2220},  /* ANGLE					   */
  {"ang90",	0x221F},  /* RIGHT ANGLE				   */
  {"angmsd",	0x2221},  /* MEASURED ANGLE				   */
  {"angsph",	0x2222},  /* SPHERICAL ANGLE				   */
  {"angst",	0x212B},  /* ANGSTROM SIGN				   */
  {"aogon",	0x0105},  /* LATIN SMALL LETTER A WITH OGONEK		   */
  {"ap",	0x2248},  /* ALMOST EQUAL TO				   */
  {"ape",	0x224A},  /* ALMOST EQUAL OR EQUAL TO			   */
  {"apos",	0x02BC},  /* MODIFIER LETTER APOSTROPHE 		   */
  {"aring",	0x00E5},  /* LATIN SMALL LETTER A WITH RING ABOVE	   */
  {"ast",	0x002A},  /* ASTERISK					   */
  {"asymp",	0x2248},  /* ALMOST EQUAL TO				   */
  {"atilde",	0x00E3},  /* LATIN SMALL LETTER A WITH TILDE		   */
  {"auml",	0x00E4},  /* LATIN SMALL LETTER A WITH DIAERESIS	   */
  {"b.Delta",	0x0394},  /* GREEK CAPITAL LETTER DELTA 		   */
  {"b.Gamma",	0x0393},  /* GREEK CAPITAL LETTER GAMMA 		   */
  {"b.Lambda",	0x039B},  /* GREEK CAPITAL LETTER LAMDA 		   */
  {"b.Omega",	0x03A9},  /* GREEK CAPITAL LETTER OMEGA 		   */
  {"b.Phi",	0x03A6},  /* GREEK CAPITAL LETTER PHI			   */
  {"b.Pi",	0x03A0},  /* GREEK CAPITAL LETTER PI			   */
  {"b.Psi",	0x03A8},  /* GREEK CAPITAL LETTER PSI			   */
  {"b.Sigma",	0x03A3},  /* GREEK CAPITAL LETTER SIGMA 		   */
  {"b.Theta",	0x0398},  /* GREEK CAPITAL LETTER THETA 		   */
  {"b.Upsi",	0x03A5},  /* GREEK CAPITAL LETTER UPSILON		   */
  {"b.Xi",	0x039E},  /* GREEK CAPITAL LETTER XI			   */
  {"b.alpha",	0x03B1},  /* GREEK SMALL LETTER ALPHA			   */
  {"b.beta",	0x03B2},  /* GREEK SMALL LETTER BETA			   */
  {"b.chi",	0x03C7},  /* GREEK SMALL LETTER CHI			   */
  {"b.delta",	0x03B4},  /* GREEK SMALL LETTER DELTA			   */
  {"b.epsi",	0x03B5},  /* GREEK SMALL LETTER EPSILON 		   */
  {"b.epsis",	0x03B5},  /* GREEK SMALL LETTER EPSILON 		   */
  {"b.epsiv",	0x03B5},  /* GREEK SMALL LETTER EPSILON 		   */
  {"b.eta",	0x03B7},  /* GREEK SMALL LETTER ETA			   */
  {"b.gamma",	0x03B3},  /* GREEK SMALL LETTER GAMMA			   */
  {"b.gammad",	0x03DC},  /* GREEK LETTER DIGAMMA			   */
  {"b.iota",	0x03B9},  /* GREEK SMALL LETTER IOTA			   */
  {"b.kappa",	0x03BA},  /* GREEK SMALL LETTER KAPPA			   */
  {"b.kappav",	0x03F0},  /* GREEK KAPPA SYMBOL 			   */
  {"b.lambda",	0x03BB},  /* GREEK SMALL LETTER LAMDA			   */
  {"b.mu",	0x03BC},  /* GREEK SMALL LETTER MU			   */
  {"b.nu",	0x03BD},  /* GREEK SMALL LETTER NU			   */
  {"b.omega",	0x03CE},  /* GREEK SMALL LETTER OMEGA WITH TONOS	   */
  {"b.phis",	0x03C6},  /* GREEK SMALL LETTER PHI			   */
  {"b.phiv",	0x03D5},  /* GREEK PHI SYMBOL				   */
  {"b.pi",	0x03C0},  /* GREEK SMALL LETTER PI			   */
  {"b.piv",	0x03D6},  /* GREEK PI SYMBOL				   */
  {"b.psi",	0x03C8},  /* GREEK SMALL LETTER PSI			   */
  {"b.rho",	0x03C1},  /* GREEK SMALL LETTER RHO			   */
  {"b.rhov",	0x03F1},  /* GREEK RHO SYMBOL				   */
  {"b.sigma",	0x03C3},  /* GREEK SMALL LETTER SIGMA			   */
  {"b.sigmav",	0x03C2},  /* GREEK SMALL LETTER FINAL SIGMA		   */
  {"b.tau",	0x03C4},  /* GREEK SMALL LETTER TAU			   */
  {"b.thetas",	0x03B8},  /* GREEK SMALL LETTER THETA			   */
  {"b.thetav",	0x03D1},  /* GREEK THETA SYMBOL 			   */
  {"b.upsi",	0x03C5},  /* GREEK SMALL LETTER UPSILON 		   */
  {"b.xi",	0x03BE},  /* GREEK SMALL LETTER XI			   */
  {"b.zeta",	0x03B6},  /* GREEK SMALL LETTER ZETA			   */
  {"barwed",	0x22BC},  /* NAND					   */
  {"bcong",	0x224C},  /* ALL EQUAL TO				   */
  {"bcy",	0x0431},  /* CYRILLIC SMALL LETTER BE			   */
  {"bdquo",	0x201E},  /* DOUBLE LOW-9 QUOTATION MARK		   */
  {"becaus",	0x2235},  /* BECAUSE					   */
  {"bepsi",	0x220D},  /* SMALL CONTAINS AS MEMBER			   */
  {"bernou",	0x212C},  /* SCRIPT CAPITAL B				   */
  {"beta",	0x03B2},  /* GREEK SMALL LETTER BETA			   */
  {"beth",	0x2136},  /* BET SYMBOL 				   */
  {"bgr",	0x03B2},  /* GREEK SMALL LETTER BETA			   */
  {"blank",	0x2423},  /* OPEN BOX					   */
  {"blk12",	0x2592},  /* MEDIUM SHADE				   */
  {"blk14",	0x2591},  /* LIGHT SHADE				   */
  {"blk34",	0x2593},  /* DARK SHADE 				   */
  {"block",	0x2588},  /* FULL BLOCK 				   */
  {"bottom",	0x22A5},  /* UP TACK					   */
  {"bowtie",	0x22C8},  /* BOWTIE					   */
  {"boxDL",	0x2557},  /* BOX DRAWINGS DOUBLE DOWN AND LEFT		   */
  {"boxDR",	0x2554},  /* BOX DRAWINGS DOUBLE DOWN AND RIGHT 	   */
  {"boxDl",	0x2556},  /* BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE	   */
  {"boxDr",	0x2553},  /* BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE	   */
  {"boxH",	0x2550},  /* BOX DRAWINGS DOUBLE HORIZONTAL		   */
  {"boxHD",	0x2566},  /* BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL	   */
  {"boxHU",	0x2569},  /* BOX DRAWINGS DOUBLE UP AND HORIZONTAL	   */
  {"boxHd",	0x2564},  /* BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE*/
  {"boxHu",	0x2567},  /* BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE  */
  {"boxUL",	0x255D},  /* BOX DRAWINGS DOUBLE UP AND LEFT		   */
  {"boxUR",	0x255A},  /* BOX DRAWINGS DOUBLE UP AND RIGHT		   */
  {"boxUl",	0x255C},  /* BOX DRAWINGS UP DOUBLE AND LEFT SINGLE	   */
  {"boxUr",	0x2559},  /* BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE	   */
  {"boxV",	0x2551},  /* BOX DRAWINGS DOUBLE VERTICAL		   */
  {"boxVH",	0x256C},  /* BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL   */
  {"boxVL",	0x2563},  /* BOX DRAWINGS DOUBLE VERTICAL AND LEFT	   */
  {"boxVR",	0x2560},  /* BOX DRAWINGS DOUBLE VERTICAL AND RIGHT	   */
  {"boxVh",	0x256B},  /* BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SI*/
  {"boxVl",	0x2562},  /* BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE  */
  {"boxVr",	0x255F},  /* BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE */
  {"boxdL",	0x2555},  /* BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE	   */
  {"boxdR",	0x2552},  /* BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE	   */
  {"boxdl",	0x2510},  /* BOX DRAWINGS LIGHT DOWN AND LEFT		   */
  {"boxdr",	0x250C},  /* BOX DRAWINGS LIGHT DOWN AND RIGHT		   */
  {"boxh",	0x2500},  /* BOX DRAWINGS LIGHT HORIZONTAL		   */
  {"boxhD",	0x2565},  /* BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE*/
  {"boxhU",	0x2568},  /* BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE  */
  {"boxhd",	0x252C},  /* BOX DRAWINGS LIGHT DOWN AND HORIZONTAL	   */
  {"boxhu",	0x2534},  /* BOX DRAWINGS LIGHT UP AND HORIZONTAL	   */
  {"boxuL",	0x255B},  /* BOX DRAWINGS UP SINGLE AND LEFT DOUBLE	   */
  {"boxuR",	0x2558},  /* BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE	   */
  {"boxul",	0x2518},  /* BOX DRAWINGS LIGHT UP AND LEFT		   */
  {"boxur",	0x2514},  /* BOX DRAWINGS LIGHT UP AND RIGHT		   */
  {"boxv",	0x2502},  /* BOX DRAWINGS LIGHT VERTICAL		   */
  {"boxvH",	0x256A},  /* BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DO*/
  {"boxvL",	0x2561},  /* BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE  */
  {"boxvR",	0x255E},  /* BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE */
  {"boxvh",	0x253C},  /* BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL    */
  {"boxvl",	0x2524},  /* BOX DRAWINGS LIGHT VERTICAL AND LEFT	   */
  {"boxvr",	0x251C},  /* BOX DRAWINGS LIGHT VERTICAL AND RIGHT	   */
  {"bprime",	0x2035},  /* REVERSED PRIME				   */
  {"breve",	0x02D8},  /* BREVE					   */
  {"brkbar",	0x00A6},  /* obsolete synonym for "brvbar" 0x00A6	   */
  {"brvbar",	0x00A6},  /* BROKEN BAR 				   */
  {"bsim",	0x223D},  /* REVERSED TILDE				   */
  {"bsime",	0x22CD},  /* REVERSED TILDE EQUALS			   */
  {"bsol",	0x005C},  /* REVERSE SOLIDUS				   */
  {"bull",	0x2022},  /* BULLET					   */
  {"bump",	0x224E},  /* GEOMETRICALLY EQUIVALENT TO		   */
  {"bumpe",	0x224F},  /* DIFFERENCE BETWEEN 			   */
  {"cacute",	0x0107},  /* LATIN SMALL LETTER C WITH ACUTE		   */
  {"cap",	0x2229},  /* INTERSECTION				   */
  {"caret",	0x2041},  /* CARET INSERTION POINT			   */
  {"caron",	0x02C7},  /* CARON					   */
  {"ccaron",	0x010D},  /* LATIN SMALL LETTER C WITH CARON		   */
  {"ccedil",	0x00E7},  /* LATIN SMALL LETTER C WITH CEDILLA		   */
  {"ccirc",	0x0109},  /* LATIN SMALL LETTER C WITH CIRCUMFLEX	   */
  {"cdot",	0x010B},  /* LATIN SMALL LETTER C WITH DOT ABOVE	   */
  {"cedil",	0x00B8},  /* CEDILLA					   */
  {"cent",	0x00A2},  /* CENT SIGN					   */
  {"chcy",	0x0447},  /* CYRILLIC SMALL LETTER CHE			   */
  {"check",	0x2713},  /* CHECK MARK 				   */
  {"chi",	0x03C7},  /* GREEK SMALL LETTER CHI			   */
  {"cir",	0x25CB},  /* WHITE CIRCLE				   */
  {"circ",	0x02C6},  /* MODIFIER LETTER CIRCUMFLEX ACCENT		   */
  {"cire",	0x2257},  /* RING EQUAL TO				   */
  {"clubs",	0x2663},  /* BLACK CLUB SUIT				   */
  {"colon",	0x003A},  /* COLON					   */
  {"colone",	0x2254},  /* COLON EQUALS				   */
  {"comma",	0x002C},  /* COMMA					   */
  {"commat",	0x0040},  /* COMMERCIAL AT				   */
  {"comp",	0x2201},  /* COMPLEMENT 				   */
  {"compfn",	0x2218},  /* RING OPERATOR				   */
  {"cong",	0x2245},  /* APPROXIMATELY EQUAL TO			   */
  {"conint",	0x222E},  /* CONTOUR INTEGRAL				   */
  {"coprod",	0x2210},  /* N-ARY COPRODUCT				   */
  {"copy",	0x00A9},  /* COPYRIGHT SIGN				   */
  {"copysr",	0x2117},  /* SOUND RECORDING COPYRIGHT			   */
  {"crarr",	0x21B5},  /* DOWNWARDS ARROW WITH CORNER LEFTWARDS	   */
  {"cross",	0x2717},  /* BALLOT X					   */
  {"cuepr",	0x22DE},  /* EQUAL TO OR PRECEDES			   */
  {"cuesc",	0x22DF},  /* EQUAL TO OR SUCCEEDS			   */
  {"cularr",	0x21B6},  /* ANTICLOCKWISE TOP SEMICIRCLE ARROW 	   */
  {"cup",	0x222A},  /* UNION					   */
  {"cupre",	0x227C},  /* PRECEDES OR EQUAL TO			   */
  {"curarr",	0x21B7},  /* CLOCKWISE TOP SEMICIRCLE ARROW		   */
  {"curren",	0x00A4},  /* CURRENCY SIGN				   */
  {"cuvee",	0x22CE},  /* CURLY LOGICAL OR				   */
  {"cuwed",	0x22CF},  /* CURLY LOGICAL AND				   */
  {"dArr",	0x21D3},  /* DOWNWARDS DOUBLE ARROW			   */
  {"dagger",	0x2020},  /* DAGGER					   */
  {"daleth",	0x2138},  /* DALET SYMBOL				   */
  {"darr",	0x2193},  /* DOWNWARDS ARROW				   */
  {"darr2",	0x21CA},  /* DOWNWARDS PAIRED ARROWS			   */
  {"dash",	0x2010},  /* HYPHEN					   */
  {"dashv",	0x22A3},  /* LEFT TACK					   */
  {"dblac",	0x02DD},  /* DOUBLE ACUTE ACCENT			   */
  {"dcaron",	0x010F},  /* LATIN SMALL LETTER D WITH CARON		   */
  {"dcy",	0x0434},  /* CYRILLIC SMALL LETTER DE			   */
  {"deg",	0x00B0},  /* DEGREE SIGN				   */
  {"delta",	0x03B4},  /* GREEK SMALL LETTER DELTA			   */
  {"dgr",	0x03B4},  /* GREEK SMALL LETTER DELTA			   */
  {"dharl",	0x21C3},  /* DOWNWARDS HARPOON WITH BARB LEFTWARDS	   */
  {"dharr",	0x21C2},  /* DOWNWARDS HARPOON WITH BARB RIGHTWARDS	   */
  {"diam",	0x22C4},  /* DIAMOND OPERATOR				   */
  {"diams",	0x2666},  /* BLACK DIAMOND SUIT 			   */
  {"die",	0x00A8},  /* DIAERESIS					   */
  {"divide",	0x00F7},  /* DIVISION SIGN				   */
  {"divonx",	0x22C7},  /* DIVISION TIMES				   */
  {"djcy",	0x0452},  /* CYRILLIC SMALL LETTER DJE			   */
  {"dlarr",	0x2199},  /* SOUTH WEST ARROW				   */
  {"dlcorn",	0x231E},  /* BOTTOM LEFT CORNER 			   */
  {"dlcrop",	0x230D},  /* BOTTOM LEFT CROP				   */
  {"dollar",	0x0024},  /* DOLLAR SIGN				   */
  {"dot",	0x02D9},  /* DOT ABOVE					   */
  {"drarr",	0x2198},  /* SOUTH EAST ARROW				   */
  {"drcorn",	0x231F},  /* BOTTOM RIGHT CORNER			   */
  {"drcrop",	0x230C},  /* BOTTOM RIGHT CROP				   */
  {"dscy",	0x0455},  /* CYRILLIC SMALL LETTER DZE			   */
  {"dstrok",	0x0111},  /* LATIN SMALL LETTER D WITH STROKE		   */
  {"dtri",	0x25BF},  /* WHITE DOWN-POINTING SMALL TRIANGLE 	   */
  {"dtrif",	0x25BE},  /* BLACK DOWN-POINTING SMALL TRIANGLE 	   */
  {"dzcy",	0x045F},  /* CYRILLIC SMALL LETTER DZHE 		   */
  {"eDot",	0x2251},  /* GEOMETRICALLY EQUAL TO			   */
  {"eacgr",	0x03AD},  /* GREEK SMALL LETTER EPSILON WITH TONOS	   */
  {"eacute",	0x00E9},  /* LATIN SMALL LETTER E WITH ACUTE		   */
  {"ecaron",	0x011B},  /* LATIN SMALL LETTER E WITH CARON		   */
  {"ecir",	0x2256},  /* RING IN EQUAL TO				   */
  {"ecirc",	0x00EA},  /* LATIN SMALL LETTER E WITH CIRCUMFLEX	   */
  {"ecolon",	0x2255},  /* EQUALS COLON				   */
  {"ecy",	0x044D},  /* CYRILLIC SMALL LETTER E			   */
  {"edot",	0x0117},  /* LATIN SMALL LETTER E WITH DOT ABOVE	   */
  {"eeacgr",	0x03AE},  /* GREEK SMALL LETTER ETA WITH TONOS		   */
  {"eegr",	0x03B7},  /* GREEK SMALL LETTER ETA			   */
  {"efDot",	0x2252},  /* APPROXIMATELY EQUAL TO OR THE IMAGE OF	   */
  {"egr",	0x03B5},  /* GREEK SMALL LETTER EPSILON 		   */
  {"egrave",	0x00E8},  /* LATIN SMALL LETTER E WITH GRAVE		   */
  {"egs",	0x22DD},  /* EQUAL TO OR GREATER-THAN			   */
  {"ell",	0x2113},  /* SCRIPT SMALL L				   */
  {"els",	0x22DC},  /* EQUAL TO OR LESS-THAN			   */
  {"emacr",	0x0113},  /* LATIN SMALL LETTER E WITH MACRON		   */
  {"emdash",	0x2014},  /* obsolete synonym for "mdash" 0x2014	   */
  {"empty",	0x2205},  /* EMPTY SET					   */
  {"emsp",	0x2003},  /* EM SPACE					   */
  {"emsp13",	0x2004},  /* THREE-PER-EM SPACE 			   */
  {"emsp14",	0x2005},  /* FOUR-PER-EM SPACE				   */
  {"endash",	0x2013},  /* obsolete synonym for "ndash" 0x2013	   */
  {"eng",	0x014B},  /* LATIN SMALL LETTER ENG			   */
  {"ensp",	0x2002},  /* EN SPACE					   */
  {"eogon",	0x0119},  /* LATIN SMALL LETTER E WITH OGONEK		   */
  {"epsi",	0x03B5},  /* GREEK SMALL LETTER EPSILON 		   */
  {"epsilon",	0x03B5},  /* GREEK SMALL LETTER EPSILON 		   */
  {"epsis",	0x220A},  /* SMALL ELEMENT OF				   */
  {"equals",	0x003D},  /* EQUALS SIGN				   */
  {"equiv",	0x2261},  /* IDENTICAL TO				   */
  {"erDot",	0x2253},  /* IMAGE OF OR APPROXIMATELY EQUAL TO 	   */
  {"esdot",	0x2250},  /* APPROACHES THE LIMIT			   */
  {"eta",	0x03B7},  /* GREEK SMALL LETTER ETA			   */
  {"eth",	0x00F0},  /* LATIN SMALL LETTER ETH			   */
  {"euml",	0x00EB},  /* LATIN SMALL LETTER E WITH DIAERESIS	   */
  {"euro",	0x20AC},  /* EURO SIGN					   */
  {"excl",	0x0021},  /* EXCLAMATION MARK				   */
  {"exist",	0x2203},  /* THERE EXISTS				   */
  {"fcy",	0x0444},  /* CYRILLIC SMALL LETTER EF			   */
  {"female",	0x2640},  /* FEMALE SIGN				   */
  {"ffilig",	0xFB03},  /* LATIN SMALL LIGATURE FFI			   */
  {"fflig",	0xFB00},  /* LATIN SMALL LIGATURE FF			   */
  {"ffllig",	0xFB04},  /* LATIN SMALL LIGATURE FFL			   */
  {"filig",	0xFB01},  /* LATIN SMALL LIGATURE FI			   */
  {"flat",	0x266D},  /* MUSIC FLAT SIGN				   */
  {"fllig",	0xFB02},  /* LATIN SMALL LIGATURE FL			   */
  {"fnof",	0x0192},  /* LATIN SMALL LETTER F WITH HOOK		   */
  {"forall",	0x2200},  /* FOR ALL					   */
  {"fork",	0x22D4},  /* PITCHFORK					   */
  {"frac12",	0x00BD},  /* VULGAR FRACTION ONE HALF			   */
  {"frac13",	0x2153},  /* VULGAR FRACTION ONE THIRD			   */
  {"frac14",	0x00BC},  /* VULGAR FRACTION ONE QUARTER		   */
  {"frac15",	0x2155},  /* VULGAR FRACTION ONE FIFTH			   */
  {"frac16",	0x2159},  /* VULGAR FRACTION ONE SIXTH			   */
  {"frac18",	0x215B},  /* VULGAR FRACTION ONE EIGHTH 		   */
  {"frac23",	0x2154},  /* VULGAR FRACTION TWO THIRDS 		   */
  {"frac25",	0x2156},  /* VULGAR FRACTION TWO FIFTHS 		   */
  {"frac34",	0x00BE},  /* VULGAR FRACTION THREE QUARTERS		   */
  {"frac35",	0x2157},  /* VULGAR FRACTION THREE FIFTHS		   */
  {"frac38",	0x215C},  /* VULGAR FRACTION THREE EIGHTHS		   */
  {"frac45",	0x2158},  /* VULGAR FRACTION FOUR FIFTHS		   */
  {"frac56",	0x215A},  /* VULGAR FRACTION FIVE SIXTHS		   */
  {"frac58",	0x215D},  /* VULGAR FRACTION FIVE EIGHTHS		   */
  {"frac78",	0x215E},  /* VULGAR FRACTION SEVEN EIGHTHS		   */
  {"frasl",	0x2044},  /* FRACTION SLASH				   */
  {"frown",	0x2322},  /* FROWN					   */
  {"gE",	0x2267},  /* GREATER-THAN OVER EQUAL TO 		   */
  {"gacute",	0x01F5},  /* LATIN SMALL LETTER G WITH ACUTE		   */
  {"gamma",	0x03B3},  /* GREEK SMALL LETTER GAMMA			   */
  {"gammad",	0x03DC},  /* GREEK LETTER DIGAMMA			   */
  {"gbreve",	0x011F},  /* LATIN SMALL LETTER G WITH BREVE		   */
  {"gcedil",	0x0123},  /* LATIN SMALL LETTER G WITH CEDILLA		   */
  {"gcirc",	0x011D},  /* LATIN SMALL LETTER G WITH CIRCUMFLEX	   */
  {"gcy",	0x0433},  /* CYRILLIC SMALL LETTER GHE			   */
  {"gdot",	0x0121},  /* LATIN SMALL LETTER G WITH DOT ABOVE	   */
  {"ge",	0x2265},  /* GREATER-THAN OR EQUAL TO			   */
  {"gel",	0x22DB},  /* GREATER-THAN EQUAL TO OR LESS-THAN 	   */
  {"ges",	0x2265},  /* GREATER-THAN OR EQUAL TO			   */
  {"ggr",	0x03B3},  /* GREEK SMALL LETTER GAMMA			   */
  {"gimel",	0x2137},  /* GIMEL SYMBOL				   */
  {"gjcy",	0x0453},  /* CYRILLIC SMALL LETTER GJE			   */
  {"gl",	0x2277},  /* GREATER-THAN OR LESS-THAN			   */
  {"gnE",	0x2269},  /* GREATER-THAN BUT NOT EQUAL TO		   */
  {"gne",	0x2269},  /* GREATER-THAN BUT NOT EQUAL TO		   */
  {"gnsim",	0x22E7},  /* GREATER-THAN BUT NOT EQUIVALENT TO 	   */
  {"grave",	0x0060},  /* GRAVE ACCENT				   */
  {"gsdot",	0x22D7},  /* GREATER-THAN WITH DOT			   */
  {"gsim",	0x2273},  /* GREATER-THAN OR EQUIVALENT TO		   */
  {"gt",	0x003E},  /* GREATER-THAN SIGN				   */
  {"gvnE",	0x2269},  /* GREATER-THAN BUT NOT EQUAL TO		   */
  {"hArr",	0x21D4},  /* LEFT RIGHT DOUBLE ARROW			   */
  {"hairsp",	0x200A},  /* HAIR SPACE 				   */
  {"half",	0x00BD},  /* VULGAR FRACTION ONE HALF			   */
  {"hamilt",	0x210B},  /* SCRIPT CAPITAL H				   */
  {"hardcy",	0x044A},  /* CYRILLIC SMALL LETTER HARD SIGN		   */
  {"harr",	0x2194},  /* LEFT RIGHT ARROW				   */
  {"harrw",	0x21AD},  /* LEFT RIGHT WAVE ARROW			   */
  {"hcirc",	0x0125},  /* LATIN SMALL LETTER H WITH CIRCUMFLEX	   */
  {"hearts",	0x2665},  /* BLACK HEART SUIT				   */
  {"hellip",	0x2026},  /* HORIZONTAL ELLIPSIS			   */
  {"hibar",	0x00AF},  /* obsolete synonym for "macr" 0x00AF 	   */
  {"horbar",	0x2015},  /* HORIZONTAL BAR				   */
  {"hstrok",	0x0127},  /* LATIN SMALL LETTER H WITH STROKE		   */
  {"hybull",	0x2043},  /* HYPHEN BULLET				   */
  {"hyphen",	0x002D},  /* HYPHEN-MINUS				   */
  {"iacgr",	0x03AF},  /* GREEK SMALL LETTER IOTA WITH TONOS 	   */
  {"iacute",	0x00ED},  /* LATIN SMALL LETTER I WITH ACUTE		   */
  {"icirc",	0x00EE},  /* LATIN SMALL LETTER I WITH CIRCUMFLEX	   */
  {"icy",	0x0438},  /* CYRILLIC SMALL LETTER I			   */
  {"idiagr",	0x0390},  /* GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TON*/
  {"idigr",	0x03CA},  /* GREEK SMALL LETTER IOTA WITH DIALYTIKA	   */
  {"iecy",	0x0435},  /* CYRILLIC SMALL LETTER IE			   */
  {"iexcl",	0x00A1},  /* INVERTED EXCLAMATION MARK			   */
  {"iff",	0x21D4},  /* LEFT RIGHT DOUBLE ARROW			   */
  {"igr",	0x03B9},  /* GREEK SMALL LETTER IOTA			   */
  {"igrave",	0x00EC},  /* LATIN SMALL LETTER I WITH GRAVE		   */
  {"ijlig",	0x0133},  /* LATIN SMALL LIGATURE IJ			   */
  {"imacr",	0x012B},  /* LATIN SMALL LETTER I WITH MACRON		   */
  {"image",	0x2111},  /* BLACK-LETTER CAPITAL I			   */
  {"incare",	0x2105},  /* CARE OF					   */
  {"infin",	0x221E},  /* INFINITY					   */
  {"inodot",	0x0131},  /* LATIN SMALL LETTER DOTLESS I		   */
  {"int",	0x222B},  /* INTEGRAL					   */
  {"intcal",	0x22BA},  /* INTERCALATE				   */
  {"iocy",	0x0451},  /* CYRILLIC SMALL LETTER IO			   */
  {"iogon",	0x012F},  /* LATIN SMALL LETTER I WITH OGONEK		   */
  {"iota",	0x03B9},  /* GREEK SMALL LETTER IOTA			   */
  {"iquest",	0x00BF},  /* INVERTED QUESTION MARK			   */
  {"isin",	0x2208},  /* ELEMENT OF 				   */
  {"itilde",	0x0129},  /* LATIN SMALL LETTER I WITH TILDE		   */
  {"iukcy",	0x0456},  /* CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I*/
  {"iuml",	0x00EF},  /* LATIN SMALL LETTER I WITH DIAERESIS	   */
  {"jcirc",	0x0135},  /* LATIN SMALL LETTER J WITH CIRCUMFLEX	   */
  {"jcy",	0x0439},  /* CYRILLIC SMALL LETTER SHORT I		   */
  {"jsercy",	0x0458},  /* CYRILLIC SMALL LETTER JE			   */
  {"jukcy",	0x0454},  /* CYRILLIC SMALL LETTER UKRAINIAN IE 	   */
  {"kappa",	0x03BA},  /* GREEK SMALL LETTER KAPPA			   */
  {"kappav",	0x03F0},  /* GREEK KAPPA SYMBOL 			   */
  {"kcedil",	0x0137},  /* LATIN SMALL LETTER K WITH CEDILLA		   */
  {"kcy",	0x043A},  /* CYRILLIC SMALL LETTER KA			   */
  {"kgr",	0x03BA},  /* GREEK SMALL LETTER KAPPA			   */
  {"kgreen",	0x0138},  /* LATIN SMALL LETTER KRA			   */
  {"khcy",	0x0445},  /* CYRILLIC SMALL LETTER HA			   */
  {"khgr",	0x03C7},  /* GREEK SMALL LETTER CHI			   */
  {"kjcy",	0x045C},  /* CYRILLIC SMALL LETTER KJE			   */
  {"lAarr",	0x21DA},  /* LEFTWARDS TRIPLE ARROW			   */
  {"lArr",	0x21D0},  /* LEFTWARDS DOUBLE ARROW			   */
  {"lE",	0x2266},  /* LESS-THAN OVER EQUAL TO			   */
  {"lacute",	0x013A},  /* LATIN SMALL LETTER L WITH ACUTE		   */
  {"lagran",	0x2112},  /* SCRIPT CAPITAL L				   */
  {"lambda",	0x03BB},  /* GREEK SMALL LETTER LAMDA			   */
  {"lang",	0x2329},  /* LEFT-POINTING ANGLE BRACKET		   */
  {"laquo",	0x00AB},  /* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK	   */
  {"larr",	0x2190},  /* LEFTWARDS ARROW				   */
  {"larr2",	0x21C7},  /* LEFTWARDS PAIRED ARROWS			   */
  {"larrhk",	0x21A9},  /* LEFTWARDS ARROW WITH HOOK			   */
  {"larrlp",	0x21AB},  /* LEFTWARDS ARROW WITH LOOP			   */
  {"larrtl",	0x21A2},  /* LEFTWARDS ARROW WITH TAIL			   */
  {"lcaron",	0x013E},  /* LATIN SMALL LETTER L WITH CARON		   */
  {"lcedil",	0x013C},  /* LATIN SMALL LETTER L WITH CEDILLA		   */
  {"lceil",	0x2308},  /* LEFT CEILING				   */
  {"lcub",	0x007B},  /* LEFT CURLY BRACKET 			   */
  {"lcy",	0x043B},  /* CYRILLIC SMALL LETTER EL			   */
  {"ldot",	0x22D6},  /* LESS-THAN WITH DOT 			   */
  {"ldquo",	0x201C},  /* LEFT DOUBLE QUOTATION MARK 		   */
  {"ldquor",	0x201E},  /* DOUBLE LOW-9 QUOTATION MARK		   */
  {"le",	0x2264},  /* LESS-THAN OR EQUAL TO			   */
  {"leg",	0x22DA},  /* LESS-THAN EQUAL TO OR GREATER-THAN 	   */
  {"les",	0x2264},  /* LESS-THAN OR EQUAL TO			   */
  {"lfloor",	0x230A},  /* LEFT FLOOR 				   */
  {"lg",	0x2276},  /* LESS-THAN OR GREATER-THAN			   */
  {"lgr",	0x03BB},  /* GREEK SMALL LETTER LAMDA			   */
  {"lhard",	0x21BD},  /* LEFTWARDS HARPOON WITH BARB DOWNWARDS	   */
  {"lharu",	0x21BC},  /* LEFTWARDS HARPOON WITH BARB UPWARDS	   */
  {"lhblk",	0x2584},  /* LOWER HALF BLOCK				   */
  {"ljcy",	0x0459},  /* CYRILLIC SMALL LETTER LJE			   */
  {"lmidot",	0x0140},  /* LATIN SMALL LETTER L WITH MIDDLE DOT	   */
  {"lnE",	0x2268},  /* LESS-THAN BUT NOT EQUAL TO 		   */
  {"lne",	0x2268},  /* LESS-THAN BUT NOT EQUAL TO 		   */
  {"lnsim",	0x22E6},  /* LESS-THAN BUT NOT EQUIVALENT TO		   */
  {"lowast",	0x2217},  /* ASTERISK OPERATOR				   */
  {"lowbar",	0x005F},  /* LOW LINE					   */
  {"loz",	0x25CA},  /* LOZENGE					   */
/*{"loz",	0x2727},     WHITE FOUR POINTED STAR			   */
  /* Warning: Duplicated &loz; entry.  HTML 4,0 defines it as U+25CA.	   */
  {"lozf",	0x2726},  /* BLACK FOUR POINTED STAR			   */
  {"lpar",	0x0028},  /* LEFT PARENTHESIS				   */
  {"lrarr2",	0x21C6},  /* LEFTWARDS ARROW OVER RIGHTWARDS ARROW	   */
  {"lrhar2",	0x21CB},  /* LEFTWARDS HARPOON OVER RIGHTWARDS HARPOON	   */
  {"lrm",	0x200E},  /* LEFT-TO-RIGHT MARK 			   */
  {"lsaquo",	0x2039},  /* SINGLE LEFT-POINTING ANGLE QUOTATION MARK	   */
  {"lsh",	0x21B0},  /* UPWARDS ARROW WITH TIP LEFTWARDS		   */
  {"lsim",	0x2272},  /* LESS-THAN OR EQUIVALENT TO 		   */
  {"lsqb",	0x005B},  /* LEFT SQUARE BRACKET			   */
  {"lsquo",	0x2018},  /* LEFT SINGLE QUOTATION MARK 		   */
  {"lsquor",	0x201A},  /* SINGLE LOW-9 QUOTATION MARK		   */
  {"lstrok",	0x0142},  /* LATIN SMALL LETTER L WITH STROKE		   */
  {"lt",	0x003C},  /* LESS-THAN SIGN				   */
  {"lthree",	0x22CB},  /* LEFT SEMIDIRECT PRODUCT			   */
  {"ltimes",	0x22C9},  /* LEFT NORMAL FACTOR SEMIDIRECT PRODUCT	   */
  {"ltri",	0x25C3},  /* WHITE LEFT-POINTING SMALL TRIANGLE 	   */
  {"ltrie",	0x22B4},  /* NORMAL SUBGROUP OF OR EQUAL TO		   */
  {"ltrif",	0x25C2},  /* BLACK LEFT-POINTING SMALL TRIANGLE 	   */
  {"lvnE",	0x2268},  /* LESS-THAN BUT NOT EQUAL TO 		   */
  {"macr",	0x00AF},  /* MACRON					   */
  {"male",	0x2642},  /* MALE SIGN					   */
  {"malt",	0x2720},  /* MALTESE CROSS				   */
  {"map",	0x21A6},  /* RIGHTWARDS ARROW FROM BAR			   */
  {"marker",	0x25AE},  /* BLACK VERTICAL RECTANGLE			   */
  {"mcy",	0x043C},  /* CYRILLIC SMALL LETTER EM			   */
  {"mdash",	0x2014},  /* EM DASH					   */
  {"mgr",	0x03BC},  /* GREEK SMALL LETTER MU			   */
  {"micro",	0x00B5},  /* MICRO SIGN 				   */
  {"mid",	0x2223},  /* DIVIDES					   */
  {"middot",	0x00B7},  /* MIDDLE DOT 				   */
  {"minus",	0x2212},  /* MINUS SIGN 				   */
  {"minusb",	0x229F},  /* SQUARED MINUS				   */
  {"mldr",	0x2026},  /* HORIZONTAL ELLIPSIS			   */
  {"mnplus",	0x2213},  /* MINUS-OR-PLUS SIGN 			   */
  {"models",	0x22A7},  /* MODELS					   */
  {"mu",	0x03BC},  /* GREEK SMALL LETTER MU			   */
  {"mumap",	0x22B8},  /* MULTIMAP					   */
  {"nVDash",	0x22AF},  /* NEGATED DOUBLE VERTICAL BAR DOUBLE RIGHT TURNS*/
  {"nVdash",	0x22AE},  /* DOES NOT FORCE				   */
  {"nabla",	0x2207},  /* NABLA					   */
  {"nacute",	0x0144},  /* LATIN SMALL LETTER N WITH ACUTE		   */
  {"nap",	0x2249},  /* NOT ALMOST EQUAL TO			   */
  {"napos",	0x0149},  /* LATIN SMALL LETTER N PRECEDED BY APOSTROPHE   */
  {"natur",	0x266E},  /* MUSIC NATURAL SIGN 			   */
  {"nbsp",	0x00A0},  /* NO-BREAK SPACE				   */
  {"ncaron",	0x0148},  /* LATIN SMALL LETTER N WITH CARON		   */
  {"ncedil",	0x0146},  /* LATIN SMALL LETTER N WITH CEDILLA		   */
  {"ncong",	0x2247},  /* NEITHER APPROXIMATELY NOR ACTUALLY EQUAL TO   */
  {"ncy",	0x043D},  /* CYRILLIC SMALL LETTER EN			   */
  {"ndash",	0x2013},  /* EN DASH					   */
  {"ne",	0x2260},  /* NOT EQUAL TO				   */
  {"nearr",	0x2197},  /* NORTH EAST ARROW				   */
  {"nequiv",	0x2262},  /* NOT IDENTICAL TO				   */
  {"nexist",	0x2204},  /* THERE DOES NOT EXIST			   */
  {"nge",	0x2271},  /* NEITHER GREATER-THAN NOR EQUAL TO		   */
  {"nges",	0x2271},  /* NEITHER GREATER-THAN NOR EQUAL TO		   */
  {"ngr",	0x03BD},  /* GREEK SMALL LETTER NU			   */
  {"ngt",	0x226F},  /* NOT GREATER-THAN				   */
  {"nhArr",	0x21CE},  /* LEFT RIGHT DOUBLE ARROW WITH STROKE	   */
  {"nharr",	0x21AE},  /* LEFT RIGHT ARROW WITH STROKE		   */
  {"ni",	0x220B},  /* CONTAINS AS MEMBER 			   */
  {"njcy",	0x045A},  /* CYRILLIC SMALL LETTER NJE			   */
  {"nlArr",	0x21CD},  /* LEFTWARDS DOUBLE ARROW WITH STROKE 	   */
  {"nlarr",	0x219A},  /* LEFTWARDS ARROW WITH STROKE		   */
  {"nldr",	0x2025},  /* TWO DOT LEADER				   */
  {"nle",	0x2270},  /* NEITHER LESS-THAN NOR EQUAL TO		   */
  {"nles",	0x2270},  /* NEITHER LESS-THAN NOR EQUAL TO		   */
  {"nlt",	0x226E},  /* NOT LESS-THAN				   */
  {"nltri",	0x22EA},  /* NOT NORMAL SUBGROUP OF			   */
  {"nltrie",	0x22EC},  /* NOT NORMAL SUBGROUP OF OR EQUAL TO 	   */
  {"nmid",	0x2224},  /* DOES NOT DIVIDE				   */
  {"not",	0x00AC},  /* NOT SIGN					   */
  {"notin",	0x2209},  /* NOT AN ELEMENT OF				   */
  {"npar",	0x2226},  /* NOT PARALLEL TO				   */
  {"npr",	0x2280},  /* DOES NOT PRECEDE				   */
  {"npre",	0x22E0},  /* DOES NOT PRECEDE OR EQUAL			   */
  {"nrArr",	0x21CF},  /* RIGHTWARDS DOUBLE ARROW WITH STROKE	   */
  {"nrarr",	0x219B},  /* RIGHTWARDS ARROW WITH STROKE		   */
  {"nrtri",	0x22EB},  /* DOES NOT CONTAIN AS NORMAL SUBGROUP	   */
  {"nrtrie",	0x22ED},  /* DOES NOT CONTAIN AS NORMAL SUBGROUP OR EQUAL  */
  {"nsc",	0x2281},  /* DOES NOT SUCCEED				   */
  {"nsce",	0x22E1},  /* DOES NOT SUCCEED OR EQUAL			   */
  {"nsim",	0x2241},  /* NOT TILDE					   */
  {"nsime",	0x2244},  /* NOT ASYMPTOTICALLY EQUAL TO		   */
  {"nspar",	0x2226},  /* NOT PARALLEL TO				   */
  {"nsub",	0x2284},  /* NOT A SUBSET OF				   */
  {"nsubE",	0x2288},  /* NEITHER A SUBSET OF NOR EQUAL TO		   */
  {"nsube",	0x2288},  /* NEITHER A SUBSET OF NOR EQUAL TO		   */
  {"nsup",	0x2285},  /* NOT A SUPERSET OF				   */
  {"nsupE",	0x2289},  /* NEITHER A SUPERSET OF NOR EQUAL TO 	   */
  {"nsupe",	0x2289},  /* NEITHER A SUPERSET OF NOR EQUAL TO 	   */
  {"ntilde",	0x00F1},  /* LATIN SMALL LETTER N WITH TILDE		   */
  {"nu",	0x03BD},  /* GREEK SMALL LETTER NU			   */
  {"num",	0x0023},  /* NUMBER SIGN				   */
  {"numero",	0x2116},  /* NUMERO SIGN				   */
  {"numsp",	0x2007},  /* FIGURE SPACE				   */
  {"nvDash",	0x22AD},  /* NOT TRUE					   */
  {"nvdash",	0x22AC},  /* DOES NOT PROVE				   */
  {"nwarr",	0x2196},  /* NORTH WEST ARROW				   */
  {"oS",	0x24C8},  /* CIRCLED LATIN CAPITAL LETTER S		   */
  {"oacgr",	0x03CC},  /* GREEK SMALL LETTER OMICRON WITH TONOS	   */
  {"oacute",	0x00F3},  /* LATIN SMALL LETTER O WITH ACUTE		   */
  {"oast",	0x229B},  /* CIRCLED ASTERISK OPERATOR			   */
  {"ocir",	0x229A},  /* CIRCLED RING OPERATOR			   */
  {"ocirc",	0x00F4},  /* LATIN SMALL LETTER O WITH CIRCUMFLEX	   */
  {"ocy",	0x043E},  /* CYRILLIC SMALL LETTER O			   */
  {"odash",	0x229D},  /* CIRCLED DASH				   */
  {"odblac",	0x0151},  /* LATIN SMALL LETTER O WITH DOUBLE ACUTE	   */
  {"odot",	0x2299},  /* CIRCLED DOT OPERATOR			   */
  {"oelig",	0x0153},  /* LATIN SMALL LIGATURE OE			   */
  {"ogon",	0x02DB},  /* OGONEK					   */
  {"ogr",	0x03BF},  /* GREEK SMALL LETTER OMICRON 		   */
  {"ograve",	0x00F2},  /* LATIN SMALL LETTER O WITH GRAVE		   */
  {"ohacgr",	0x03CE},  /* GREEK SMALL LETTER OMEGA WITH TONOS	   */
  {"ohgr",	0x03C9},  /* GREEK SMALL LETTER OMEGA			   */
  {"ohm",	0x2126},  /* OHM SIGN					   */
  {"olarr",	0x21BA},  /* ANTICLOCKWISE OPEN CIRCLE ARROW		   */
  {"oline",	0x203E},  /* OVERLINE					   */
  {"omacr",	0x014D},  /* LATIN SMALL LETTER O WITH MACRON		   */
  {"omega",	0x03C9},  /* GREEK SMALL LETTER OMEGA			   */
  {"omicron",	0x03BF},  /* GREEK SMALL LETTER OMICRON 		   */
  {"ominus",	0x2296},  /* CIRCLED MINUS				   */
  {"oplus",	0x2295},  /* CIRCLED PLUS				   */
  {"or",	0x2228},  /* LOGICAL OR 				   */
  {"orarr",	0x21BB},  /* CLOCKWISE OPEN CIRCLE ARROW		   */
  {"order",	0x2134},  /* SCRIPT SMALL O				   */
  {"ordf",	0x00AA},  /* FEMININE ORDINAL INDICATOR 		   */
  {"ordm",	0x00BA},  /* MASCULINE ORDINAL INDICATOR		   */
  {"oslash",	0x00F8},  /* LATIN SMALL LETTER O WITH STROKE		   */
  {"osol",	0x2298},  /* CIRCLED DIVISION SLASH			   */
  {"otilde",	0x00F5},  /* LATIN SMALL LETTER O WITH TILDE		   */
  {"otimes",	0x2297},  /* CIRCLED TIMES				   */
  {"ouml",	0x00F6},  /* LATIN SMALL LETTER O WITH DIAERESIS	   */
  {"par",	0x2225},  /* PARALLEL TO				   */
  {"para",	0x00B6},  /* PILCROW SIGN				   */
  {"part",	0x2202},  /* PARTIAL DIFFERENTIAL			   */
  {"pcy",	0x043F},  /* CYRILLIC SMALL LETTER PE			   */
  {"percnt",	0x0025},  /* PERCENT SIGN				   */
  {"period",	0x002E},  /* FULL STOP					   */
  {"permil",	0x2030},  /* PER MILLE SIGN				   */
  {"perp",	0x22A5},  /* UP TACK					   */
  {"pgr",	0x03C0},  /* GREEK SMALL LETTER PI			   */
  {"phgr",	0x03C6},  /* GREEK SMALL LETTER PHI			   */
  {"phi",	0x03C6},  /* GREEK SMALL LETTER PHI			   */
  {"phis",	0x03C6},  /* GREEK SMALL LETTER PHI			   */
  {"phiv",	0x03D5},  /* GREEK PHI SYMBOL				   */
  {"phmmat",	0x2133},  /* SCRIPT CAPITAL M				   */
  {"phone",	0x260E},  /* BLACK TELEPHONE				   */
  {"pi",	0x03C0},  /* GREEK SMALL LETTER PI			   */
  {"piv",	0x03D6},  /* GREEK PI SYMBOL				   */
  {"planck",	0x210F},  /* PLANCK CONSTANT OVER TWO PI		   */
  {"plus",	0x002B},  /* PLUS SIGN					   */
  {"plusb",	0x229E},  /* SQUARED PLUS				   */
  {"plusdo",	0x2214},  /* DOT PLUS					   */
  {"plusmn",	0x00B1},  /* PLUS-MINUS SIGN				   */
  {"pound",	0x00A3},  /* POUND SIGN 				   */
  {"pr",	0x227A},  /* PRECEDES					   */
  {"pre",	0x227C},  /* PRECEDES OR EQUAL TO			   */
  {"prime",	0x2032},  /* PRIME					   */
  {"prnsim",	0x22E8},  /* PRECEDES BUT NOT EQUIVALENT TO		   */
  {"prod",	0x220F},  /* N-ARY PRODUCT				   */
  {"prop",	0x221D},  /* PROPORTIONAL TO				   */
  {"prsim",	0x227E},  /* PRECEDES OR EQUIVALENT TO			   */
  {"psgr",	0x03C8},  /* GREEK SMALL LETTER PSI			   */
  {"psi",	0x03C8},  /* GREEK SMALL LETTER PSI			   */
  {"puncsp",	0x2008},  /* PUNCTUATION SPACE				   */
  {"quest",	0x003F},  /* QUESTION MARK				   */
  {"quot",	0x0022},  /* QUOTATION MARK				   */
  {"rAarr",	0x21DB},  /* RIGHTWARDS TRIPLE ARROW			   */
  {"rArr",	0x21D2},  /* RIGHTWARDS DOUBLE ARROW			   */
  {"racute",	0x0155},  /* LATIN SMALL LETTER R WITH ACUTE		   */
  {"radic",	0x221A},  /* SQUARE ROOT				   */
  {"rang",	0x232A},  /* RIGHT-POINTING ANGLE BRACKET		   */
  {"raquo",	0x00BB},  /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK    */
  {"rarr",	0x2192},  /* RIGHTWARDS ARROW				   */
  {"rarr2",	0x21C9},  /* RIGHTWARDS PAIRED ARROWS			   */
  {"rarrhk",	0x21AA},  /* RIGHTWARDS ARROW WITH HOOK 		   */
  {"rarrlp",	0x21AC},  /* RIGHTWARDS ARROW WITH LOOP 		   */
  {"rarrtl",	0x21A3},  /* RIGHTWARDS ARROW WITH TAIL 		   */
  {"rarrw",	0x219D},  /* RIGHTWARDS WAVE ARROW			   */
  {"rcaron",	0x0159},  /* LATIN SMALL LETTER R WITH CARON		   */
  {"rcedil",	0x0157},  /* LATIN SMALL LETTER R WITH CEDILLA		   */
  {"rceil",	0x2309},  /* RIGHT CEILING				   */
  {"rcub",	0x007D},  /* RIGHT CURLY BRACKET			   */
  {"rcy",	0x0440},  /* CYRILLIC SMALL LETTER ER			   */
  {"rdquo",	0x201D},  /* RIGHT DOUBLE QUOTATION MARK		   */
  {"rdquor",	0x201C},  /* LEFT DOUBLE QUOTATION MARK 		   */
  {"real",	0x211C},  /* BLACK-LETTER CAPITAL R			   */
  {"rect",	0x25AD},  /* WHITE RECTANGLE				   */
  {"reg",	0x00AE},  /* REGISTERED SIGN				   */
  {"rfloor",	0x230B},  /* RIGHT FLOOR				   */
  {"rgr",	0x03C1},  /* GREEK SMALL LETTER RHO			   */
  {"rhard",	0x21C1},  /* RIGHTWARDS HARPOON WITH BARB DOWNWARDS	   */
  {"rharu",	0x21C0},  /* RIGHTWARDS HARPOON WITH BARB UPWARDS	   */
  {"rho",	0x03C1},  /* GREEK SMALL LETTER RHO			   */
  {"rhov",	0x03F1},  /* GREEK RHO SYMBOL				   */
  {"ring",	0x02DA},  /* RING ABOVE 				   */
  {"rlarr2",	0x21C4},  /* RIGHTWARDS ARROW OVER LEFTWARDS ARROW	   */
  {"rlhar2",	0x21CC},  /* RIGHTWARDS HARPOON OVER LEFTWARDS HARPOON	   */
  {"rlm",	0x200F},  /* RIGHT-TO-LEFT MARK 			   */
  {"rpar",	0x0029},  /* RIGHT PARENTHESIS				   */
  {"rsaquo",	0x203A},  /* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK    */
  {"rsh",	0x21B1},  /* UPWARDS ARROW WITH TIP RIGHTWARDS		   */
  {"rsqb",	0x005D},  /* RIGHT SQUARE BRACKET			   */
  {"rsquo",	0x2019},  /* RIGHT SINGLE QUOTATION MARK		   */
  {"rsquor",	0x2018},  /* LEFT SINGLE QUOTATION MARK 		   */
  {"rthree",	0x22CC},  /* RIGHT SEMIDIRECT PRODUCT			   */
  {"rtimes",	0x22CA},  /* RIGHT NORMAL FACTOR SEMIDIRECT PRODUCT	   */
  {"rtri",	0x25B9},  /* WHITE RIGHT-POINTING SMALL TRIANGLE	   */
  {"rtrie",	0x22B5},  /* CONTAINS AS NORMAL SUBGROUP OR EQUAL TO	   */
  {"rtrif",	0x25B8},  /* BLACK RIGHT-POINTING SMALL TRIANGLE	   */
  {"rx",	0x211E},  /* PRESCRIPTION TAKE				   */
  {"sacute",	0x015B},  /* LATIN SMALL LETTER S WITH ACUTE		   */
  {"samalg",	0x2210},  /* N-ARY COPRODUCT				   */
  {"sbquo",	0x201A},  /* SINGLE LOW-9 QUOTATION MARK		   */
  {"sbsol",	0x005C},  /* REVERSE SOLIDUS				   */
  {"sc",	0x227B},  /* SUCCEEDS					   */
  {"scaron",	0x0161},  /* LATIN SMALL LETTER S WITH CARON		   */
  {"sccue",	0x227D},  /* SUCCEEDS OR EQUAL TO			   */
  {"sce",	0x227D},  /* SUCCEEDS OR EQUAL TO			   */
  {"scedil",	0x015F},  /* LATIN SMALL LETTER S WITH CEDILLA		   */
  {"scirc",	0x015D},  /* LATIN SMALL LETTER S WITH CIRCUMFLEX	   */
  {"scnsim",	0x22E9},  /* SUCCEEDS BUT NOT EQUIVALENT TO		   */
  {"scsim",	0x227F},  /* SUCCEEDS OR EQUIVALENT TO			   */
  {"scy",	0x0441},  /* CYRILLIC SMALL LETTER ES			   */
  {"sdot",	0x22C5},  /* DOT OPERATOR				   */
  {"sdotb",	0x22A1},  /* SQUARED DOT OPERATOR			   */
  {"sect",	0x00A7},  /* SECTION SIGN				   */
  {"semi",	0x003B},  /* SEMICOLON					   */
  {"setmn",	0x2216},  /* SET MINUS					   */
  {"sext",	0x2736},  /* SIX POINTED BLACK STAR			   */
  {"sfgr",	0x03C2},  /* GREEK SMALL LETTER FINAL SIGMA		   */
  {"sfrown",	0x2322},  /* FROWN					   */
  {"sgr",	0x03C3},  /* GREEK SMALL LETTER SIGMA			   */
  {"sharp",	0x266F},  /* MUSIC SHARP SIGN				   */
  {"shchcy",	0x0449},  /* CYRILLIC SMALL LETTER SHCHA		   */
  {"shcy",	0x0448},  /* CYRILLIC SMALL LETTER SHA			   */
  {"shy",	0x00AD},  /* SOFT HYPHEN				   */
  {"sigma",	0x03C3},  /* GREEK SMALL LETTER SIGMA			   */
  {"sigmaf",	0x03C2},  /* GREEK SMALL LETTER FINAL SIGMA		   */
  {"sigmav",	0x03C2},  /* GREEK SMALL LETTER FINAL SIGMA		   */
  {"sim",	0x223C},  /* TILDE OPERATOR				   */
  {"sime",	0x2243},  /* ASYMPTOTICALLY EQUAL TO			   */
  {"smile",	0x2323},  /* SMILE					   */
  {"softcy",	0x044C},  /* CYRILLIC SMALL LETTER SOFT SIGN		   */
  {"sol",	0x002F},  /* SOLIDUS					   */
  {"spades",	0x2660},  /* BLACK SPADE SUIT				   */
  {"spar",	0x2225},  /* PARALLEL TO				   */
  {"sqcap",	0x2293},  /* SQUARE CAP 				   */
  {"sqcup",	0x2294},  /* SQUARE CUP 				   */
  {"sqsub",	0x228F},  /* SQUARE IMAGE OF				   */
  {"sqsube",	0x2291},  /* SQUARE IMAGE OF OR EQUAL TO		   */
  {"sqsup",	0x2290},  /* SQUARE ORIGINAL OF 			   */
  {"sqsupe",	0x2292},  /* SQUARE ORIGINAL OF OR EQUAL TO		   */
  {"squ",	0x25A1},  /* WHITE SQUARE				   */
  {"square",	0x25A1},  /* WHITE SQUARE				   */
  {"squf",	0x25AA},  /* BLACK SMALL SQUARE 			   */
  {"ssetmn",	0x2216},  /* SET MINUS					   */
  {"ssmile",	0x2323},  /* SMILE					   */
  {"sstarf",	0x22C6},  /* STAR OPERATOR				   */
  {"star",	0x2606},  /* WHITE STAR 				   */
  {"starf",	0x2605},  /* BLACK STAR 				   */
  {"sub",	0x2282},  /* SUBSET OF					   */
  {"subE",	0x2286},  /* SUBSET OF OR EQUAL TO			   */
  {"sube",	0x2286},  /* SUBSET OF OR EQUAL TO			   */
  {"subnE",	0x228A},  /* SUBSET OF WITH NOT EQUAL TO		   */
  {"subne",	0x228A},  /* SUBSET OF WITH NOT EQUAL TO		   */
  {"sum",	0x2211},  /* N-ARY SUMMATION				   */
  {"sung",	0x266A},  /* EIGHTH NOTE				   */
  {"sup",	0x2283},  /* SUPERSET OF				   */
  {"sup1",	0x00B9},  /* SUPERSCRIPT ONE				   */
  {"sup2",	0x00B2},  /* SUPERSCRIPT TWO				   */
  {"sup3",	0x00B3},  /* SUPERSCRIPT THREE				   */
  {"supE",	0x2287},  /* SUPERSET OF OR EQUAL TO			   */
  {"supe",	0x2287},  /* SUPERSET OF OR EQUAL TO			   */
  {"supnE",	0x228B},  /* SUPERSET OF WITH NOT EQUAL TO		   */
  {"supne",	0x228B},  /* SUPERSET OF WITH NOT EQUAL TO		   */
  {"szlig",	0x00DF},  /* LATIN SMALL LETTER SHARP S 		   */
  {"target",	0x2316},  /* POSITION INDICATOR 			   */
  {"tau",	0x03C4},  /* GREEK SMALL LETTER TAU			   */
  {"tcaron",	0x0165},  /* LATIN SMALL LETTER T WITH CARON		   */
  {"tcedil",	0x0163},  /* LATIN SMALL LETTER T WITH CEDILLA		   */
  {"tcy",	0x0442},  /* CYRILLIC SMALL LETTER TE			   */
  {"tdot",	0x20DB},  /* COMBINING THREE DOTS ABOVE 		   */
  {"telrec",	0x2315},  /* TELEPHONE RECORDER 			   */
  {"tgr",	0x03C4},  /* GREEK SMALL LETTER TAU			   */
  {"there4",	0x2234},  /* THEREFORE					   */
  {"theta",	0x03B8},  /* GREEK SMALL LETTER THETA			   */
  {"thetas",	0x03B8},  /* GREEK SMALL LETTER THETA			   */
  {"thetasym",	0x03D1},  /* GREEK THETA SYMBOL 			   */
  {"thetav",	0x03D1},  /* GREEK THETA SYMBOL 			   */
  {"thgr",	0x03B8},  /* GREEK SMALL LETTER THETA			   */
  {"thinsp",	0x2009},  /* THIN SPACE 				   */
  {"thkap",	0x2248},  /* ALMOST EQUAL TO				   */
  {"thksim",	0x223C},  /* TILDE OPERATOR				   */
  {"thorn",	0x00FE},  /* LATIN SMALL LETTER THORN			   */
  {"tilde",	0x02DC},  /* SMALL TILDE				   */
  {"times",	0x00D7},  /* MULTIPLICATION SIGN			   */
  {"timesb",	0x22A0},  /* SQUARED TIMES				   */
  {"top",	0x22A4},  /* DOWN TACK					   */
  {"tprime",	0x2034},  /* TRIPLE PRIME				   */
  {"trade",	0x2122},  /* TRADE MARK SIGN				   */
  {"trie",	0x225C},  /* DELTA EQUAL TO				   */
  {"tscy",	0x0446},  /* CYRILLIC SMALL LETTER TSE			   */
  {"tshcy",	0x045B},  /* CYRILLIC SMALL LETTER TSHE 		   */
  {"tstrok",	0x0167},  /* LATIN SMALL LETTER T WITH STROKE		   */
  {"twixt",	0x226C},  /* BETWEEN					   */
  {"uArr",	0x21D1},  /* UPWARDS DOUBLE ARROW			   */
  {"uacgr",	0x03CD},  /* GREEK SMALL LETTER UPSILON WITH TONOS	   */
  {"uacute",	0x00FA},  /* LATIN SMALL LETTER U WITH ACUTE		   */
  {"uarr",	0x2191},  /* UPWARDS ARROW				   */
  {"uarr2",	0x21C8},  /* UPWARDS PAIRED ARROWS			   */
  {"ubrcy",	0x045E},  /* CYRILLIC SMALL LETTER SHORT U		   */
  {"ubreve",	0x016D},  /* LATIN SMALL LETTER U WITH BREVE		   */
  {"ucirc",	0x00FB},  /* LATIN SMALL LETTER U WITH CIRCUMFLEX	   */
  {"ucy",	0x0443},  /* CYRILLIC SMALL LETTER U			   */
  {"udblac",	0x0171},  /* LATIN SMALL LETTER U WITH DOUBLE ACUTE	   */
  {"udiagr",	0x03B0},  /* GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND */
  {"udigr",	0x03CB},  /* GREEK SMALL LETTER UPSILON WITH DIALYTIKA	   */
  {"ugr",	0x03C5},  /* GREEK SMALL LETTER UPSILON 		   */
  {"ugrave",	0x00F9},  /* LATIN SMALL LETTER U WITH GRAVE		   */
  {"uharl",	0x21BF},  /* UPWARDS HARPOON WITH BARB LEFTWARDS	   */
  {"uharr",	0x21BE},  /* UPWARDS HARPOON WITH BARB RIGHTWARDS	   */
  {"uhblk",	0x2580},  /* UPPER HALF BLOCK				   */
  {"ulcorn",	0x231C},  /* TOP LEFT CORNER				   */
  {"ulcrop",	0x230F},  /* TOP LEFT CROP				   */
  {"umacr",	0x016B},  /* LATIN SMALL LETTER U WITH MACRON		   */
  {"uml",	0x00A8},  /* DIAERESIS					   */
  {"uogon",	0x0173},  /* LATIN SMALL LETTER U WITH OGONEK		   */
  {"uplus",	0x228E},  /* MULTISET UNION				   */
  {"upsi",	0x03C5},  /* GREEK SMALL LETTER UPSILON 		   */
  {"upsih",	0x03D2},  /* GREEK UPSILON WITH HOOK SYMBOL		   */
  {"upsilon",	0x03C5},  /* GREEK SMALL LETTER UPSILON 		   */
  {"urcorn",	0x231D},  /* TOP RIGHT CORNER				   */
  {"urcrop",	0x230E},  /* TOP RIGHT CROP				   */
  {"uring",	0x016F},  /* LATIN SMALL LETTER U WITH RING ABOVE	   */
  {"utilde",	0x0169},  /* LATIN SMALL LETTER U WITH TILDE		   */
  {"utri",	0x25B5},  /* WHITE UP-POINTING SMALL TRIANGLE		   */
  {"utrif",	0x25B4},  /* BLACK UP-POINTING SMALL TRIANGLE		   */
  {"uuml",	0x00FC},  /* LATIN SMALL LETTER U WITH DIAERESIS	   */
  {"vArr",	0x21D5},  /* UP DOWN DOUBLE ARROW			   */
  {"vDash",	0x22A8},  /* TRUE					   */
  {"varr",	0x2195},  /* UP DOWN ARROW				   */
  {"vcy",	0x0432},  /* CYRILLIC SMALL LETTER VE			   */
  {"vdash",	0x22A2},  /* RIGHT TACK 				   */
  {"veebar",	0x22BB},  /* XOR					   */
  {"vellip",	0x22EE},  /* VERTICAL ELLIPSIS				   */
  {"verbar",	0x007C},  /* VERTICAL LINE				   */
  {"vltri",	0x22B2},  /* NORMAL SUBGROUP OF 			   */
  {"vprime",	0x2032},  /* PRIME					   */
  {"vprop",	0x221D},  /* PROPORTIONAL TO				   */
  {"vrtri",	0x22B3},  /* CONTAINS AS NORMAL SUBGROUP		   */
  {"vsubnE",	0x228A},  /* SUBSET OF WITH NOT EQUAL TO		   */
  {"vsubne",	0x228A},  /* SUBSET OF WITH NOT EQUAL TO		   */
  {"vsupnE",	0x228B},  /* SUPERSET OF WITH NOT EQUAL TO		   */
  {"vsupne",	0x228B},  /* SUPERSET OF WITH NOT EQUAL TO		   */
  {"wcirc",	0x0175},  /* LATIN SMALL LETTER W WITH CIRCUMFLEX	   */
  {"wedgeq",	0x2259},  /* ESTIMATES					   */
  {"weierp",	0x2118},  /* SCRIPT CAPITAL P				   */
  {"wreath",	0x2240},  /* WREATH PRODUCT				   */
  {"xcirc",	0x25CB},  /* WHITE CIRCLE				   */
  {"xdtri",	0x25BD},  /* WHITE DOWN-POINTING TRIANGLE		   */
  {"xgr",	0x03BE},  /* GREEK SMALL LETTER XI			   */
  {"xhArr",	0x2194},  /* LEFT RIGHT ARROW				   */
  {"xharr",	0x2194},  /* LEFT RIGHT ARROW				   */
  {"xi",	0x03BE},  /* GREEK SMALL LETTER XI			   */
  {"xlArr",	0x21D0},  /* LEFTWARDS DOUBLE ARROW			   */
  {"xrArr",	0x21D2},  /* RIGHTWARDS DOUBLE ARROW			   */
  {"xutri",	0x25B3},  /* WHITE UP-POINTING TRIANGLE 		   */
  {"yacute",	0x00FD},  /* LATIN SMALL LETTER Y WITH ACUTE		   */
  {"yacy",	0x044F},  /* CYRILLIC SMALL LETTER YA			   */
  {"ycirc",	0x0177},  /* LATIN SMALL LETTER Y WITH CIRCUMFLEX	   */
  {"ycy",	0x044B},  /* CYRILLIC SMALL LETTER YERU 		   */
  {"yen",	0x00A5},  /* YEN SIGN					   */
  {"yicy",	0x0457},  /* CYRILLIC SMALL LETTER YI			   */
  {"yucy",	0x044E},  /* CYRILLIC SMALL LETTER YU			   */
  {"yuml",	0x00FF},  /* LATIN SMALL LETTER Y WITH DIAERESIS	   */
  {"zacute",	0x017A},  /* LATIN SMALL LETTER Z WITH ACUTE		   */
  {"zcaron",	0x017E},  /* LATIN SMALL LETTER Z WITH CARON		   */
  {"zcy",	0x0437},  /* CYRILLIC SMALL LETTER ZE			   */
  {"zdot",	0x017C},  /* LATIN SMALL LETTER Z WITH DOT ABOVE	   */
  {"zeta",	0x03B6},  /* GREEK SMALL LETTER ZETA			   */
  {"zgr",	0x03B6},  /* GREEK SMALL LETTER ZETA			   */
  {"zhcy",	0x0436},  /* CYRILLIC SMALL LETTER ZHE			   */
  {"zwj",	0x200D},  /* ZERO WIDTH JOINER				   */
  {"zwnj",	0x200C},  /* ZERO WIDTH NON-JOINER			   */
/* {"epsiv",	0x????},  variant epsilon			 # ISOgrk3 */
/* {"fjlig",	0x????},  fj ligature				 # ISOpub  */
/* {"gEl",	0x????},  greater-than, double equals, less-than # ISOamsr */
/* {"gap",	0x????},  greater-than, approximately equal to	 # ISOamsr */
/* {"gnap",	0x????},  greater-than, not approximately equal t# ISOamsn */
/* {"jnodot",	0x????},  latin small letter dotless j		 # ISOamso */
/* {"lEg",	0x????},  less-than, double equals, greater-than # ISOamsr */
/* {"lap",	0x????},  less-than, approximately equal to	 # ISOamsr */
/* {"lnap",	0x????},  less-than, not approximately equal to  # ISOamsn */
/* {"lpargt",	0x????},  left parenthesis, greater-than	 # ISOamsc */
/* {"ngE",	0x????},  not greater-than, double equals	 # ISOamsn */
/* {"nlE",	0x????},  not less-than, double equals		 # ISOamsn */
/* {"nsmid",	0x????},  nshortmid				 # ISOamsn */
/* {"prap",	0x????},  precedes, approximately equal to	 # ISOamsr */
/* {"prnE",	0x????},  precedes, not double equal		 # ISOamsn */
/* {"prnap",	0x????},  precedes, not approximately equal to	 # ISOamsn */
/* {"rpargt",	0x????},  right parenthesis, greater-than	 # ISOamsc */
/* {"scap",	0x????},  succeeds, approximately equal to	 # ISOamsr */
/* {"scnE",	0x????},  succeeds, not double equals		 # ISOamsn */
/* {"scnap",	0x????},  succeeds, not approximately equal to	 # ISOamsn */
/* {"smid",	0x????},  shortmid				 # ISOamsr */
};

#endif /* not ENTITIES_HTML40_ONLY */
/* *INDENT-ON* */
