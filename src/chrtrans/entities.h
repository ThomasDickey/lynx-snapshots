/*
 * $LynxId: entities.h,v 1.8 2025/06/19 20:56:13 Eric.Lindblad Exp $
 *
 *     Entity Names to Unicode table
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

Now we have a choice:  use clean HTML4.0 entities list (and reject everything
others), or use a relaxed list with lots of synonyms and new symbols found at

ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MISC/SGML.TXT

We hold both: #define ENTITIES_HTML40_ONLY for strict version,
otherwise relaxed.

 */
#define ENTITIES_HTML5

#include <UCkd.h>		/* typedef u16 */
typedef struct {
    const char *name;		/* sorted alphabetically (case-sensitive) */
    unsigned code;
} UC_entity_info;

static const UC_entity_info unicode_entities[] =
/* *INDENT-OFF* */
{
#ifdef ENTITIES_HTML5
  { "AElig",              198}, /* LATIN CAPITAL LETTER AE */
  { "AMP",                 38}, /* AMPERSAND */
  { "Aacgr",              902}, /* GREEK CAPITAL LETTER ALPHA WITH TONOS */
  { "Aacute",             193}, /* LATIN CAPITAL LETTER A WITH ACUTE */
  { "Abreve",             258}, /* LATIN CAPITAL LETTER A WITH BREVE */
  { "Acirc",              194}, /* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
  { "Acy",               1040}, /* CYRILLIC CAPITAL LETTER A */
  { "Afr",             120068}, /* MATHEMATICAL FRAKTUR CAPITAL A */
  { "Agr",                913}, /* GREEK CAPITAL LETTER ALPHA */
  { "Agrave",             192}, /* LATIN CAPITAL LETTER A WITH GRAVE */
  { "Alpha",              913}, /* GREEK CAPITAL LETTER ALPHA */
  { "Amacr",              256}, /* LATIN CAPITAL LETTER A WITH MACRON */
  { "And",              10835}, /* DOUBLE LOGICAL AND */
  { "Aogon",              260}, /* LATIN CAPITAL LETTER A WITH OGONEK */
  { "Aopf",            120120}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL A */
  { "ApplyFunction",     8289}, /* FUNCTION APPLICATION */
  { "Aring",              197}, /* LATIN CAPITAL LETTER A WITH RING ABOVE */
  { "Ascr",            119964}, /* MATHEMATICAL SCRIPT CAPITAL A */
  { "Assign",            8788}, /* COLON EQUALS */
  { "Atilde",             195}, /* LATIN CAPITAL LETTER A WITH TILDE */
  { "Auml",               196}, /* LATIN CAPITAL LETTER A WITH DIAERESIS */
  { "Backslash",         8726}, /* SET MINUS */
  { "Barv",             10983}, /* SHORT DOWN TACK WITH OVERBAR */
  { "Barwed",            8966}, /* PERSPECTIVE */
  { "Bcy",               1041}, /* CYRILLIC CAPITAL LETTER BE */
  { "Because",           8757}, /* BECAUSE */
  { "Bernoullis",        8492}, /* SCRIPT CAPITAL B */
  { "Beta",               914}, /* GREEK CAPITAL LETTER BETA */
  { "Bfr",             120069}, /* MATHEMATICAL FRAKTUR CAPITAL B */
  { "Bgr",                914}, /* GREEK CAPITAL LETTER BETA */
  { "Bopf",            120121}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL B */
  { "Breve",              728}, /* BREVE */
  { "Bscr",              8492}, /* SCRIPT CAPITAL B */
  { "Bumpeq",            8782}, /* GEOMETRICALLY EQUIVALENT TO */
  { "CHcy",              1063}, /* CYRILLIC CAPITAL LETTER CHE */
  { "COPY",               169}, /* COPYRIGHT SIGN */
  { "Cacute",             262}, /* LATIN CAPITAL LETTER C WITH ACUTE */
  { "Cap",               8914}, /* DOUBLE INTERSECTION */
  { "CapitalDifferentialD",  8517}, /* DOUBLE-STRUCK ITALIC CAPITAL D */
  { "Cayleys",           8493}, /* BLACK-LETTER CAPITAL C */
  { "Ccaron",             268}, /* LATIN CAPITAL LETTER C WITH CARON */
  { "Ccedil",             199}, /* LATIN CAPITAL LETTER C WITH CEDILLA */
  { "Ccirc",              264}, /* LATIN CAPITAL LETTER C WITH CIRCUMFLEX */
  { "Cconint",           8752}, /* VOLUME INTEGRAL */
  { "Cdot",               266}, /* LATIN CAPITAL LETTER C WITH DOT ABOVE */
  { "Cedilla",            184}, /* CEDILLA */
  { "CenterDot",          183}, /* MIDDLE DOT */
  { "Cfr",               8493}, /* BLACK-LETTER CAPITAL C */
  { "Chi",                935}, /* GREEK CAPITAL LETTER CHI */
  { "CircleDot",         8857}, /* CIRCLED DOT OPERATOR */
  { "CircleMinus",       8854}, /* CIRCLED MINUS */
  { "CirclePlus",        8853}, /* CIRCLED PLUS */
  { "CircleTimes",       8855}, /* CIRCLED TIMES */
  { "ClockwiseContourIntegral",  8754}, /* CLOCKWISE CONTOUR INTEGRAL */
  { "CloseCurlyDoubleQuote",  8221}, /* RIGHT DOUBLE QUOTATION MARK */
  { "CloseCurlyQuote",   8217}, /* RIGHT SINGLE QUOTATION MARK */
  { "Colon",             8759}, /* PROPORTION */
  { "Colone",           10868}, /* DOUBLE COLON EQUAL */
  { "Congruent",         8801}, /* IDENTICAL TO */
  { "Conint",            8751}, /* SURFACE INTEGRAL */
  { "ContourIntegral",   8750}, /* CONTOUR INTEGRAL */
  { "Copf",              8450}, /* DOUBLE-STRUCK CAPITAL C */
  { "Coproduct",         8720}, /* N-ARY COPRODUCT */
  { "CounterClockwiseContourIntegral",  8755}, /* ANTICLOCKWISE CONTOUR INTEGRAL */
  { "Cross",            10799}, /* VECTOR OR CROSS PRODUCT */
  { "Cscr",            119966}, /* MATHEMATICAL SCRIPT CAPITAL C */
  { "Cup",               8915}, /* DOUBLE UNION */
  { "CupCap",            8781}, /* EQUIVALENT TO */
  { "DD",                8517}, /* DOUBLE-STRUCK ITALIC CAPITAL D */
  { "DDotrahd",         10513}, /* RIGHTWARDS ARROW WITH DOTTED STEM */
  { "DJcy",              1026}, /* CYRILLIC CAPITAL LETTER DJE */
  { "DScy",              1029}, /* CYRILLIC CAPITAL LETTER DZE */
  { "DZcy",              1039}, /* CYRILLIC CAPITAL LETTER DZHE */
  { "Dagger",            8225}, /* DOUBLE DAGGER */
  { "Darr",              8609}, /* DOWNWARDS TWO HEADED ARROW */
  { "Dashv",            10980}, /* VERTICAL BAR DOUBLE LEFT TURNSTILE */
  { "Dcaron",             270}, /* LATIN CAPITAL LETTER D WITH CARON */
  { "Dcy",               1044}, /* CYRILLIC CAPITAL LETTER DE */
  { "Del",               8711}, /* NABLA */
  { "Delta",              916}, /* GREEK CAPITAL LETTER DELTA */
  { "Dfr",             120071}, /* MATHEMATICAL FRAKTUR CAPITAL D */
  { "Dgr",                916}, /* GREEK CAPITAL LETTER DELTA */
  { "DiacriticalAcute",   180}, /* ACUTE ACCENT */
  { "DiacriticalDot",     729}, /* DOT ABOVE */
  { "DiacriticalDoubleAcute",   733}, /* DOUBLE ACUTE ACCENT */
  { "DiacriticalGrave",    96}, /* GRAVE ACCENT */
  { "DiacriticalTilde",   732}, /* SMALL TILDE */
  { "Diamond",           8900}, /* DIAMOND OPERATOR */
  { "DifferentialD",     8518}, /* DOUBLE-STRUCK ITALIC SMALL D */
  { "Dopf",            120123}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL D */
  { "Dot",                168}, /* DIAERESIS */
  { "DotDot",            8412}, /* COMBINING FOUR DOTS ABOVE */
  { "DotEqual",          8784}, /* APPROACHES THE LIMIT */
  { "DoubleContourIntegral",  8751}, /* SURFACE INTEGRAL */
  { "DoubleDot",          168}, /* DIAERESIS */
  { "DoubleDownArrow",   8659}, /* DOWNWARDS DOUBLE ARROW */
  { "DoubleLeftArrow",   8656}, /* LEFTWARDS DOUBLE ARROW */
  { "DoubleLeftRightArrow",  8660}, /* LEFT RIGHT DOUBLE ARROW */
  { "DoubleLeftTee",    10980}, /* VERTICAL BAR DOUBLE LEFT TURNSTILE */
  { "DoubleLongLeftArrow", 10232}, /* LONG LEFTWARDS DOUBLE ARROW */
  { "DoubleLongLeftRightArrow", 10234}, /* LONG LEFT RIGHT DOUBLE ARROW */
  { "DoubleLongRightArrow", 10233}, /* LONG RIGHTWARDS DOUBLE ARROW */
  { "DoubleRightArrow",  8658}, /* RIGHTWARDS DOUBLE ARROW */
  { "DoubleRightTee",    8872}, /* TRUE */
  { "DoubleUpArrow",     8657}, /* UPWARDS DOUBLE ARROW */
  { "DoubleUpDownArrow",  8661}, /* UP DOWN DOUBLE ARROW */
  { "DoubleVerticalBar",  8741}, /* PARALLEL TO */
  { "DownArrow",         8595}, /* DOWNWARDS ARROW */
  { "DownArrowBar",     10515}, /* DOWNWARDS ARROW TO BAR */
  { "DownArrowUpArrow",  8693}, /* DOWNWARDS ARROW LEFTWARDS OF UPWARDS ARROW */
  { "DownBreve",          785}, /* COMBINING INVERTED BREVE */
  { "DownLeftRightVector", 10576}, /* LEFT BARB DOWN RIGHT BARB DOWN HARPOON */
  { "DownLeftTeeVector", 10590}, /* LEFTWARDS HARPOON WITH BARB DOWN FROM BAR */
  { "DownLeftVector",    8637}, /* LEFTWARDS HARPOON WITH BARB DOWNWARDS */
  { "DownLeftVectorBar", 10582}, /* LEFTWARDS HARPOON WITH BARB DOWN TO BAR */
  { "DownRightTeeVector", 10591}, /* RIGHTWARDS HARPOON WITH BARB DOWN FROM BAR */
  { "DownRightVector",   8641}, /* RIGHTWARDS HARPOON WITH BARB DOWNWARDS */
  { "DownRightVectorBar", 10583}, /* RIGHTWARDS HARPOON WITH BARB DOWN TO BAR */
  { "DownTee",           8868}, /* DOWN TACK */
  { "DownTeeArrow",      8615}, /* DOWNWARDS ARROW FROM BAR */
  { "Downarrow",         8659}, /* DOWNWARDS DOUBLE ARROW */
  { "Dscr",            119967}, /* MATHEMATICAL SCRIPT CAPITAL D */
  { "Dstrok",             272}, /* LATIN CAPITAL LETTER D WITH STROKE */
  { "EEacgr",             905}, /* GREEK CAPITAL LETTER ETA WITH TONOS */
  { "EEgr",               919}, /* GREEK CAPITAL LETTER ETA */
  { "ENG",                330}, /* LATIN CAPITAL LETTER ENG */
  { "ETH",                208}, /* LATIN CAPITAL LETTER ETH */
  { "Eacgr",              904}, /* GREEK CAPITAL LETTER EPSILON WITH TONOS */
  { "Eacute",             201}, /* LATIN CAPITAL LETTER E WITH ACUTE */
  { "Ecaron",             282}, /* LATIN CAPITAL LETTER E WITH CARON */
  { "Ecirc",              202}, /* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
  { "Ecy",               1069}, /* CYRILLIC CAPITAL LETTER E */
  { "Edot",               278}, /* LATIN CAPITAL LETTER E WITH DOT ABOVE */
  { "Efr",             120072}, /* MATHEMATICAL FRAKTUR CAPITAL E */
  { "Egr",                917}, /* GREEK CAPITAL LETTER EPSILON */
  { "Egrave",             200}, /* LATIN CAPITAL LETTER E WITH GRAVE */
  { "Element",           8712}, /* ELEMENT OF */
  { "Emacr",              274}, /* LATIN CAPITAL LETTER E WITH MACRON */
  { "EmptySmallSquare",  9723}, /* WHITE MEDIUM SQUARE */
  { "EmptyVerySmallSquare",  9643}, /* WHITE SMALL SQUARE */
  { "Eogon",              280}, /* LATIN CAPITAL LETTER E WITH OGONEK */
  { "Eopf",            120124}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL E */
  { "Epsilon",            917}, /* GREEK CAPITAL LETTER EPSILON */
  { "Equal",            10869}, /* TWO CONSECUTIVE EQUALS SIGNS */
  { "EqualTilde",        8770}, /* MINUS TILDE */
  { "Equilibrium",       8652}, /* RIGHTWARDS HARPOON OVER LEFTWARDS HARPOON */
  { "Escr",              8496}, /* SCRIPT CAPITAL E */
  { "Esim",             10867}, /* EQUALS SIGN ABOVE TILDE OPERATOR */
  { "Eta",                919}, /* GREEK CAPITAL LETTER ETA */
  { "Euml",               203}, /* LATIN CAPITAL LETTER E WITH DIAERESIS */
  { "Exists",            8707}, /* THERE EXISTS */
  { "ExponentialE",      8519}, /* DOUBLE-STRUCK ITALIC SMALL E */
  { "Fcy",               1060}, /* CYRILLIC CAPITAL LETTER EF */
  { "Ffr",             120073}, /* MATHEMATICAL FRAKTUR CAPITAL F */
  { "FilledSmallSquare",  9724}, /* BLACK MEDIUM SQUARE */
  { "FilledVerySmallSquare",  9642}, /* BLACK SMALL SQUARE */
  { "Fopf",            120125}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL F */
  { "ForAll",            8704}, /* FOR ALL */
  { "Fouriertrf",        8497}, /* SCRIPT CAPITAL F */
  { "Fscr",              8497}, /* SCRIPT CAPITAL F */
  { "GJcy",              1027}, /* CYRILLIC CAPITAL LETTER GJE */
  { "GT",                  62}, /* GREATER-THAN SIGN */
  { "Gamma",              915}, /* GREEK CAPITAL LETTER GAMMA */
  { "Gammad",             988}, /* GREEK LETTER DIGAMMA */
  { "Gbreve",             286}, /* LATIN CAPITAL LETTER G WITH BREVE */
  { "Gcedil",             290}, /* LATIN CAPITAL LETTER G WITH CEDILLA */
  { "Gcirc",              284}, /* LATIN CAPITAL LETTER G WITH CIRCUMFLEX */
  { "Gcy",               1043}, /* CYRILLIC CAPITAL LETTER GHE */
  { "Gdot",               288}, /* LATIN CAPITAL LETTER G WITH DOT ABOVE */
  { "Gfr",             120074}, /* MATHEMATICAL FRAKTUR CAPITAL G */
  { "Gg",                8921}, /* VERY MUCH GREATER-THAN */
  { "Ggr",                915}, /* GREEK CAPITAL LETTER GAMMA */
  { "Gopf",            120126}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL G */
  { "GreaterEqual",      8805}, /* GREATER-THAN OR EQUAL TO */
  { "GreaterEqualLess",  8923}, /* GREATER-THAN EQUAL TO OR LESS-THAN */
  { "GreaterFullEqual",  8807}, /* GREATER-THAN OVER EQUAL TO */
  { "GreaterGreater",   10914}, /* DOUBLE NESTED GREATER-THAN */
  { "GreaterLess",       8823}, /* GREATER-THAN OR LESS-THAN */
  { "GreaterSlantEqual", 10878}, /* GREATER-THAN OR SLANTED EQUAL TO */
  { "GreaterTilde",      8819}, /* GREATER-THAN OR EQUIVALENT TO */
  { "Gscr",            119970}, /* MATHEMATICAL SCRIPT CAPITAL G */
  { "Gt",                8811}, /* MUCH GREATER-THAN */
  { "HARDcy",            1066}, /* CYRILLIC CAPITAL LETTER HARD SIGN */
  { "Hacek",              711}, /* CARON */
  { "Hat",                 94}, /* CIRCUMFLEX ACCENT */
  { "Hcirc",              292}, /* LATIN CAPITAL LETTER H WITH CIRCUMFLEX */
  { "Hfr",               8460}, /* BLACK-LETTER CAPITAL H */
  { "HilbertSpace",      8459}, /* SCRIPT CAPITAL H */
  { "Hopf",              8461}, /* DOUBLE-STRUCK CAPITAL H */
  { "HorizontalLine",    9472}, /* BOX DRAWINGS LIGHT HORIZONTAL */
  { "Hscr",              8459}, /* SCRIPT CAPITAL H */
  { "Hstrok",             294}, /* LATIN CAPITAL LETTER H WITH STROKE */
  { "HumpDownHump",      8782}, /* GEOMETRICALLY EQUIVALENT TO */
  { "HumpEqual",         8783}, /* DIFFERENCE BETWEEN */
  { "IEcy",              1045}, /* CYRILLIC CAPITAL LETTER IE */
  { "IJlig",              306}, /* LATIN CAPITAL LIGATURE IJ */
  { "IOcy",              1025}, /* CYRILLIC CAPITAL LETTER IO */
  { "Iacgr",              906}, /* GREEK CAPITAL LETTER IOTA WITH TONOS */
  { "Iacute",             205}, /* LATIN CAPITAL LETTER I WITH ACUTE */
  { "Icirc",              206}, /* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
  { "Icy",               1048}, /* CYRILLIC CAPITAL LETTER I */
  { "Idigr",              938}, /* GREEK CAPITAL LETTER IOTA WITH DIALYTIKA */
  { "Idot",               304}, /* LATIN CAPITAL LETTER I WITH DOT ABOVE */
  { "Ifr",               8465}, /* BLACK-LETTER CAPITAL I */
  { "Igr",                921}, /* GREEK CAPITAL LETTER IOTA */
  { "Igrave",             204}, /* LATIN CAPITAL LETTER I WITH GRAVE */
  { "Im",                8465}, /* BLACK-LETTER CAPITAL I */
  { "Imacr",              298}, /* LATIN CAPITAL LETTER I WITH MACRON */
  { "ImaginaryI",        8520}, /* DOUBLE-STRUCK ITALIC SMALL I */
  { "Implies",           8658}, /* RIGHTWARDS DOUBLE ARROW */
  { "Int",               8748}, /* DOUBLE INTEGRAL */
  { "Integral",          8747}, /* INTEGRAL */
  { "Intersection",      8898}, /* N-ARY INTERSECTION */
  { "InvisibleComma",    8291}, /* INVISIBLE SEPARATOR */
  { "InvisibleTimes",    8290}, /* INVISIBLE TIMES */
  { "Iogon",              302}, /* LATIN CAPITAL LETTER I WITH OGONEK */
  { "Iopf",            120128}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL I */
  { "Iota",               921}, /* GREEK CAPITAL LETTER IOTA */
  { "Iscr",              8464}, /* SCRIPT CAPITAL I */
  { "Itilde",             296}, /* LATIN CAPITAL LETTER I WITH TILDE */
  { "Iukcy",             1030}, /* CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I */
  { "Iuml",               207}, /* LATIN CAPITAL LETTER I WITH DIAERESIS */
  { "Jcirc",              308}, /* LATIN CAPITAL LETTER J WITH CIRCUMFLEX */
  { "Jcy",               1049}, /* CYRILLIC CAPITAL LETTER SHORT I */
  { "Jfr",             120077}, /* MATHEMATICAL FRAKTUR CAPITAL J */
  { "Jopf",            120129}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL J */
  { "Jscr",            119973}, /* MATHEMATICAL SCRIPT CAPITAL J */
  { "Jsercy",            1032}, /* CYRILLIC CAPITAL LETTER JE */
  { "Jukcy",             1028}, /* CYRILLIC CAPITAL LETTER UKRAINIAN IE */
  { "KHcy",              1061}, /* CYRILLIC CAPITAL LETTER HA */
  { "KHgr",               935}, /* GREEK CAPITAL LETTER CHI */
  { "KJcy",              1036}, /* CYRILLIC CAPITAL LETTER KJE */
  { "Kappa",              922}, /* GREEK CAPITAL LETTER KAPPA */
  { "Kcedil",             310}, /* LATIN CAPITAL LETTER K WITH CEDILLA */
  { "Kcy",               1050}, /* CYRILLIC CAPITAL LETTER KA */
  { "Kfr",             120078}, /* MATHEMATICAL FRAKTUR CAPITAL K */
  { "Kgr",                922}, /* GREEK CAPITAL LETTER KAPPA */
  { "Kopf",            120130}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL K */
  { "Kscr",            119974}, /* MATHEMATICAL SCRIPT CAPITAL K */
  { "LJcy",              1033}, /* CYRILLIC CAPITAL LETTER LJE */
  { "LT",                  60}, /* LESS-THAN SIGN */
  { "Lacute",             313}, /* LATIN CAPITAL LETTER L WITH ACUTE */
  { "Lambda",             923}, /* GREEK CAPITAL LETTER LAMDA */
  { "Lang",             10218}, /* MATHEMATICAL LEFT DOUBLE ANGLE BRACKET */
  { "Laplacetrf",        8466}, /* SCRIPT CAPITAL L */
  { "Larr",              8606}, /* LEFTWARDS TWO HEADED ARROW */
  { "Lcaron",             317}, /* LATIN CAPITAL LETTER L WITH CARON */
  { "Lcedil",             315}, /* LATIN CAPITAL LETTER L WITH CEDILLA */
  { "Lcy",               1051}, /* CYRILLIC CAPITAL LETTER EL */
  { "LeftAngleBracket", 10216}, /* MATHEMATICAL LEFT ANGLE BRACKET */
  { "LeftArrow",         8592}, /* LEFTWARDS ARROW */
  { "LeftArrowBar",      8676}, /* LEFTWARDS ARROW TO BAR */
  { "LeftArrowRightArrow",  8646}, /* LEFTWARDS ARROW OVER RIGHTWARDS ARROW */
  { "LeftCeiling",       8968}, /* LEFT CEILING */
  { "LeftDoubleBracket", 10214}, /* MATHEMATICAL LEFT WHITE SQUARE BRACKET */
  { "LeftDownTeeVector", 10593}, /* DOWNWARDS HARPOON WITH BARB LEFT FROM BAR */
  { "LeftDownVector",    8643}, /* DOWNWARDS HARPOON WITH BARB LEFTWARDS */
  { "LeftDownVectorBar", 10585}, /* DOWNWARDS HARPOON WITH BARB LEFT TO BAR */
  { "LeftFloor",         8970}, /* LEFT FLOOR */
  { "LeftRightArrow",    8596}, /* LEFT RIGHT ARROW */
  { "LeftRightVector",  10574}, /* LEFT BARB UP RIGHT BARB UP HARPOON */
  { "LeftTee",           8867}, /* LEFT TACK */
  { "LeftTeeArrow",      8612}, /* LEFTWARDS ARROW FROM BAR */
  { "LeftTeeVector",    10586}, /* LEFTWARDS HARPOON WITH BARB UP FROM BAR */
  { "LeftTriangle",      8882}, /* NORMAL SUBGROUP OF */
  { "LeftTriangleBar",  10703}, /* LEFT TRIANGLE BESIDE VERTICAL BAR */
  { "LeftTriangleEqual",  8884}, /* NORMAL SUBGROUP OF OR EQUAL TO */
  { "LeftUpDownVector", 10577}, /* UP BARB LEFT DOWN BARB LEFT HARPOON */
  { "LeftUpTeeVector",  10592}, /* UPWARDS HARPOON WITH BARB LEFT FROM BAR */
  { "LeftUpVector",      8639}, /* UPWARDS HARPOON WITH BARB LEFTWARDS */
  { "LeftUpVectorBar",  10584}, /* UPWARDS HARPOON WITH BARB LEFT TO BAR */
  { "LeftVector",        8636}, /* LEFTWARDS HARPOON WITH BARB UPWARDS */
  { "LeftVectorBar",    10578}, /* LEFTWARDS HARPOON WITH BARB UP TO BAR */
  { "Leftarrow",         8656}, /* LEFTWARDS DOUBLE ARROW */
  { "Leftrightarrow",    8660}, /* LEFT RIGHT DOUBLE ARROW */
  { "LessEqualGreater",  8922}, /* LESS-THAN EQUAL TO OR GREATER-THAN */
  { "LessFullEqual",     8806}, /* LESS-THAN OVER EQUAL TO */
  { "LessGreater",       8822}, /* LESS-THAN OR GREATER-THAN */
  { "LessLess",         10913}, /* DOUBLE NESTED LESS-THAN */
  { "LessSlantEqual",   10877}, /* LESS-THAN OR SLANTED EQUAL TO */
  { "LessTilde",         8818}, /* LESS-THAN OR EQUIVALENT TO */
  { "Lfr",             120079}, /* MATHEMATICAL FRAKTUR CAPITAL L */
  { "Lgr",                923}, /* GREEK CAPITAL LETTER LAMDA */
  { "Ll",                8920}, /* VERY MUCH LESS-THAN */
  { "Lleftarrow",        8666}, /* LEFTWARDS TRIPLE ARROW */
  { "Lmidot",             319}, /* LATIN CAPITAL LETTER L WITH MIDDLE DOT */
  { "LongLeftArrow",    10229}, /* LONG LEFTWARDS ARROW */
  { "LongLeftRightArrow", 10231}, /* LONG LEFT RIGHT ARROW */
  { "LongRightArrow",   10230}, /* LONG RIGHTWARDS ARROW */
  { "Longleftarrow",    10232}, /* LONG LEFTWARDS DOUBLE ARROW */
  { "Longleftrightarrow", 10234}, /* LONG LEFT RIGHT DOUBLE ARROW */
  { "Longrightarrow",   10233}, /* LONG RIGHTWARDS DOUBLE ARROW */
  { "Lopf",            120131}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL L */
  { "LowerLeftArrow",    8601}, /* SOUTH WEST ARROW */
  { "LowerRightArrow",   8600}, /* SOUTH EAST ARROW */
  { "Lscr",              8466}, /* SCRIPT CAPITAL L */
  { "Lsh",               8624}, /* UPWARDS ARROW WITH TIP LEFTWARDS */
  { "Lstrok",             321}, /* LATIN CAPITAL LETTER L WITH STROKE */
  { "Lt",                8810}, /* MUCH LESS-THAN */
  { "Map",              10501}, /* RIGHTWARDS TWO-HEADED ARROW FROM BAR */
  { "Mcy",               1052}, /* CYRILLIC CAPITAL LETTER EM */
  { "MediumSpace",       8287}, /* MEDIUM MATHEMATICAL SPACE */
  { "Mellintrf",         8499}, /* SCRIPT CAPITAL M */
  { "Mfr",             120080}, /* MATHEMATICAL FRAKTUR CAPITAL M */
  { "Mgr",                924}, /* GREEK CAPITAL LETTER MU */
  { "MinusPlus",         8723}, /* MINUS-OR-PLUS SIGN */
  { "Mopf",            120132}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL M */
  { "Mscr",              8499}, /* SCRIPT CAPITAL M */
  { "Mu",                 924}, /* GREEK CAPITAL LETTER MU */
  { "NJcy",              1034}, /* CYRILLIC CAPITAL LETTER NJE */
  { "Nacute",             323}, /* LATIN CAPITAL LETTER N WITH ACUTE */
  { "Ncaron",             327}, /* LATIN CAPITAL LETTER N WITH CARON */
  { "Ncedil",             325}, /* LATIN CAPITAL LETTER N WITH CEDILLA */
  { "Ncy",               1053}, /* CYRILLIC CAPITAL LETTER EN */
  { "NegativeMediumSpace",  8203}, /* ZERO WIDTH SPACE */
  { "NegativeThickSpace",  8203}, /* ZERO WIDTH SPACE */
  { "NegativeThinSpace",  8203}, /* ZERO WIDTH SPACE */
  { "NegativeVeryThinSpace",  8203}, /* ZERO WIDTH SPACE */
  { "NestedGreaterGreater",  8811}, /* MUCH GREATER-THAN */
  { "NestedLessLess",    8810}, /* MUCH LESS-THAN */
  { "NewLine",             10}, /* LINE FEED (LF) */
  { "Nfr",             120081}, /* MATHEMATICAL FRAKTUR CAPITAL N */
  { "Ngr",                925}, /* GREEK CAPITAL LETTER NU */
  { "NoBreak",           8288}, /* WORD JOINER */
  { "NonBreakingSpace",   160}, /* NO-BREAK SPACE */
  { "Nopf",              8469}, /* DOUBLE-STRUCK CAPITAL N */
  { "Not",              10988}, /* DOUBLE STROKE NOT SIGN */
  { "NotCongruent",      8802}, /* NOT IDENTICAL TO */
  { "NotCupCap",         8813}, /* NOT EQUIVALENT TO */
  { "NotDoubleVerticalBar",  8742}, /* NOT PARALLEL TO */
  { "NotElement",        8713}, /* NOT AN ELEMENT OF */
  { "NotEqual",          8800}, /* NOT EQUAL TO */
  { "NotExists",         8708}, /* THERE DOES NOT EXIST */
  { "NotGreater",        8815}, /* NOT GREATER-THAN */
  { "NotGreaterEqual",   8817}, /* NEITHER GREATER-THAN NOR EQUAL TO */
  { "NotGreaterLess",    8825}, /* NEITHER GREATER-THAN NOR LESS-THAN */
  { "NotGreaterTilde",   8821}, /* NEITHER GREATER-THAN NOR EQUIVALENT TO */
  { "NotLeftTriangle",   8938}, /* NOT NORMAL SUBGROUP OF */
  { "NotLeftTriangleEqual",  8940}, /* NOT NORMAL SUBGROUP OF OR EQUAL TO */
  { "NotLess",           8814}, /* NOT LESS-THAN */
  { "NotLessEqual",      8816}, /* NEITHER LESS-THAN NOR EQUAL TO */
  { "NotLessGreater",    8824}, /* NEITHER LESS-THAN NOR GREATER-THAN */
  { "NotLessTilde",      8820}, /* NEITHER LESS-THAN NOR EQUIVALENT TO */
  { "NotPrecedes",       8832}, /* DOES NOT PRECEDE */
  { "NotPrecedesSlantEqual",  8928}, /* DOES NOT PRECEDE OR EQUAL */
  { "NotReverseElement",  8716}, /* DOES NOT CONTAIN AS MEMBER */
  { "NotRightTriangle",  8939}, /* DOES NOT CONTAIN AS NORMAL SUBGROUP */
  { "NotRightTriangleEqual",  8941}, /* DOES NOT CONTAIN AS NORMAL SUBGROUP OR EQUAL */
  { "NotSquareSubsetEqual",  8930}, /* NOT SQUARE IMAGE OF OR EQUAL TO */
  { "NotSquareSupersetEqual",  8931}, /* NOT SQUARE ORIGINAL OF OR EQUAL TO */
  { "NotSubsetEqual",    8840}, /* NEITHER A SUBSET OF NOR EQUAL TO */
  { "NotSucceeds",       8833}, /* DOES NOT SUCCEED */
  { "NotSucceedsSlantEqual",  8929}, /* DOES NOT SUCCEED OR EQUAL */
  { "NotSupersetEqual",  8841}, /* NEITHER A SUPERSET OF NOR EQUAL TO */
  { "NotTilde",          8769}, /* NOT TILDE */
  { "NotTildeEqual",     8772}, /* NOT ASYMPTOTICALLY EQUAL TO */
  { "NotTildeFullEqual",  8775}, /* NEITHER APPROXIMATELY NOR ACTUALLY EQUAL TO */
  { "NotTildeTilde",     8777}, /* NOT ALMOST EQUAL TO */
  { "NotVerticalBar",    8740}, /* DOES NOT DIVIDE */
  { "Nscr",            119977}, /* MATHEMATICAL SCRIPT CAPITAL N */
  { "Ntilde",             209}, /* LATIN CAPITAL LETTER N WITH TILDE */
  { "Nu",                 925}, /* GREEK CAPITAL LETTER NU */
  { "OElig",              338}, /* LATIN CAPITAL LIGATURE OE */
  { "OHacgr",             911}, /* GREEK CAPITAL LETTER OMEGA WITH TONOS */
  { "OHgr",               937}, /* GREEK CAPITAL LETTER OMEGA */
  { "Oacgr",              908}, /* GREEK CAPITAL LETTER OMICRON WITH TONOS */
  { "Oacute",             211}, /* LATIN CAPITAL LETTER O WITH ACUTE */
  { "Ocirc",              212}, /* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
  { "Ocy",               1054}, /* CYRILLIC CAPITAL LETTER O */
  { "Odblac",             336}, /* LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */
  { "Ofr",             120082}, /* MATHEMATICAL FRAKTUR CAPITAL O */
  { "Ogr",                927}, /* GREEK CAPITAL LETTER OMICRON */
  { "Ograve",             210}, /* LATIN CAPITAL LETTER O WITH GRAVE */
  { "Omacr",              332}, /* LATIN CAPITAL LETTER O WITH MACRON */
  { "Omega",              937}, /* GREEK CAPITAL LETTER OMEGA */
  { "Omicron",            927}, /* GREEK CAPITAL LETTER OMICRON */
  { "Oopf",            120134}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL O */
  { "OpenCurlyDoubleQuote",  8220}, /* LEFT DOUBLE QUOTATION MARK */
  { "OpenCurlyQuote",    8216}, /* LEFT SINGLE QUOTATION MARK */
  { "Or",               10836}, /* DOUBLE LOGICAL OR */
  { "Oscr",            119978}, /* MATHEMATICAL SCRIPT CAPITAL O */
  { "Oslash",             216}, /* LATIN CAPITAL LETTER O WITH STROKE */
  { "Otilde",             213}, /* LATIN CAPITAL LETTER O WITH TILDE */
  { "Otimes",           10807}, /* MULTIPLICATION SIGN IN DOUBLE CIRCLE */
  { "Ouml",               214}, /* LATIN CAPITAL LETTER O WITH DIAERESIS */
  { "OverBar",           8254}, /* OVERLINE */
  { "OverBrace",         9182}, /* TOP CURLY BRACKET */
  { "OverBracket",       9140}, /* TOP SQUARE BRACKET */
  { "OverParenthesis",   9180}, /* TOP PARENTHESIS */
  { "PHgr",               934}, /* GREEK CAPITAL LETTER PHI */
  { "PSgr",               936}, /* GREEK CAPITAL LETTER PSI */
  { "PartialD",          8706}, /* PARTIAL DIFFERENTIAL */
  { "Pcy",               1055}, /* CYRILLIC CAPITAL LETTER PE */
  { "Pfr",             120083}, /* MATHEMATICAL FRAKTUR CAPITAL P */
  { "Pgr",                928}, /* GREEK CAPITAL LETTER PI */
  { "Phi",                934}, /* GREEK CAPITAL LETTER PHI */
  { "Pi",                 928}, /* GREEK CAPITAL LETTER PI */
  { "PlusMinus",          177}, /* PLUS-MINUS SIGN */
  { "Poincareplane",     8460}, /* BLACK-LETTER CAPITAL H */
  { "Popf",              8473}, /* DOUBLE-STRUCK CAPITAL P */
  { "Pr",               10939}, /* DOUBLE PRECEDES */
  { "Precedes",          8826}, /* PRECEDES */
  { "PrecedesEqual",    10927}, /* PRECEDES ABOVE SINGLE-LINE EQUALS SIGN */
  { "PrecedesSlantEqual",  8828}, /* PRECEDES OR EQUAL TO */
  { "PrecedesTilde",     8830}, /* PRECEDES OR EQUIVALENT TO */
  { "Prime",             8243}, /* DOUBLE PRIME */
  { "Product",           8719}, /* N-ARY PRODUCT */
  { "Proportion",        8759}, /* PROPORTION */
  { "Proportional",      8733}, /* PROPORTIONAL TO */
  { "Pscr",            119979}, /* MATHEMATICAL SCRIPT CAPITAL P */
  { "Psi",                936}, /* GREEK CAPITAL LETTER PSI */
  { "QUOT",                34}, /* QUOTATION MARK */
  { "Qfr",             120084}, /* MATHEMATICAL FRAKTUR CAPITAL Q */
  { "Qopf",              8474}, /* DOUBLE-STRUCK CAPITAL Q */
  { "Qscr",            119980}, /* MATHEMATICAL SCRIPT CAPITAL Q */
  { "REG",                174}, /* REGISTERED SIGN */
  { "Racute",             340}, /* LATIN CAPITAL LETTER R WITH ACUTE */
  { "Rang",             10219}, /* MATHEMATICAL RIGHT DOUBLE ANGLE BRACKET */
  { "Rarr",              8608}, /* RIGHTWARDS TWO HEADED ARROW */
  { "Rarrtl",           10518}, /* RIGHTWARDS TWO-HEADED ARROW WITH TAIL */
  { "Rcaron",             344}, /* LATIN CAPITAL LETTER R WITH CARON */
  { "Rcedil",             342}, /* LATIN CAPITAL LETTER R WITH CEDILLA */
  { "Rcy",               1056}, /* CYRILLIC CAPITAL LETTER ER */
  { "Re",                8476}, /* BLACK-LETTER CAPITAL R */
  { "ReverseElement",    8715}, /* CONTAINS AS MEMBER */
  { "ReverseUpEquilibrium", 10607}, /* DOWNWARDS HARPOON WITH BARB LEFT BESIDE UPWARDS HARPOON WITH BARB RIGHT */
  { "Rfr",               8476}, /* BLACK-LETTER CAPITAL R */
  { "Rgr",                929}, /* GREEK CAPITAL LETTER RHO */
  { "Rho",                929}, /* GREEK CAPITAL LETTER RHO */
  { "RightAngleBracket", 10217}, /* MATHEMATICAL RIGHT ANGLE BRACKET */
  { "RightArrow",        8594}, /* RIGHTWARDS ARROW */
  { "RightArrowBar",     8677}, /* RIGHTWARDS ARROW TO BAR */
  { "RightArrowLeftArrow",  8644}, /* RIGHTWARDS ARROW OVER LEFTWARDS ARROW */
  { "RightCeiling",      8969}, /* RIGHT CEILING */
  { "RightDoubleBracket", 10215}, /* MATHEMATICAL RIGHT WHITE SQUARE BRACKET */
  { "RightDownTeeVector", 10589}, /* DOWNWARDS HARPOON WITH BARB RIGHT FROM BAR */
  { "RightDownVector",   8642}, /* DOWNWARDS HARPOON WITH BARB RIGHTWARDS */
  { "RightDownVectorBar", 10581}, /* DOWNWARDS HARPOON WITH BARB RIGHT TO BAR */
  { "RightFloor",        8971}, /* RIGHT FLOOR */
  { "RightTee",          8866}, /* RIGHT TACK */
  { "RightTeeArrow",     8614}, /* RIGHTWARDS ARROW FROM BAR */
  { "RightTeeVector",   10587}, /* RIGHTWARDS HARPOON WITH BARB UP FROM BAR */
  { "RightTriangle",     8883}, /* CONTAINS AS NORMAL SUBGROUP */
  { "RightTriangleBar", 10704}, /* VERTICAL BAR BESIDE RIGHT TRIANGLE */
  { "RightTriangleEqual",  8885}, /* CONTAINS AS NORMAL SUBGROUP OR EQUAL TO */
  { "RightUpDownVector", 10575}, /* UP BARB RIGHT DOWN BARB RIGHT HARPOON */
  { "RightUpTeeVector", 10588}, /* UPWARDS HARPOON WITH BARB RIGHT FROM BAR */
  { "RightUpVector",     8638}, /* UPWARDS HARPOON WITH BARB RIGHTWARDS */
  { "RightUpVectorBar", 10580}, /* UPWARDS HARPOON WITH BARB RIGHT TO BAR */
  { "RightVector",       8640}, /* RIGHTWARDS HARPOON WITH BARB UPWARDS */
  { "RightVectorBar",   10579}, /* RIGHTWARDS HARPOON WITH BARB UP TO BAR */
  { "Rightarrow",        8658}, /* RIGHTWARDS DOUBLE ARROW */
  { "Ropf",              8477}, /* DOUBLE-STRUCK CAPITAL R */
  { "RoundImplies",     10608}, /* RIGHT DOUBLE ARROW WITH ROUNDED HEAD */
  { "Rrightarrow",       8667}, /* RIGHTWARDS TRIPLE ARROW */
  { "Rscr",              8475}, /* SCRIPT CAPITAL R */
  { "Rsh",               8625}, /* UPWARDS ARROW WITH TIP RIGHTWARDS */
  { "RuleDelayed",      10740}, /* RULE-DELAYED */
  { "SHCHcy",            1065}, /* CYRILLIC CAPITAL LETTER SHCHA */
  { "SHcy",              1064}, /* CYRILLIC CAPITAL LETTER SHA */
  { "SOFTcy",            1068}, /* CYRILLIC CAPITAL LETTER SOFT SIGN */
  { "Sacute",             346}, /* LATIN CAPITAL LETTER S WITH ACUTE */
  { "Sc",               10940}, /* DOUBLE SUCCEEDS */
  { "Scaron",             352}, /* LATIN CAPITAL LETTER S WITH CARON */
  { "Scedil",             350}, /* LATIN CAPITAL LETTER S WITH CEDILLA */
  { "Scirc",              348}, /* LATIN CAPITAL LETTER S WITH CIRCUMFLEX */
  { "Scy",               1057}, /* CYRILLIC CAPITAL LETTER ES */
  { "Sfr",             120086}, /* MATHEMATICAL FRAKTUR CAPITAL S */
  { "Sgr",                931}, /* GREEK CAPITAL LETTER SIGMA */
  { "ShortDownArrow",    8595}, /* DOWNWARDS ARROW */
  { "ShortLeftArrow",    8592}, /* LEFTWARDS ARROW */
  { "ShortRightArrow",   8594}, /* RIGHTWARDS ARROW */
  { "ShortUpArrow",      8593}, /* UPWARDS ARROW */
  { "Sigma",              931}, /* GREEK CAPITAL LETTER SIGMA */
  { "SmallCircle",       8728}, /* RING OPERATOR */
  { "Sopf",            120138}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL S */
  { "Sqrt",              8730}, /* SQUARE ROOT */
  { "Square",            9633}, /* WHITE SQUARE */
  { "SquareIntersection",  8851}, /* SQUARE CAP */
  { "SquareSubset",      8847}, /* SQUARE IMAGE OF */
  { "SquareSubsetEqual",  8849}, /* SQUARE IMAGE OF OR EQUAL TO */
  { "SquareSuperset",    8848}, /* SQUARE ORIGINAL OF */
  { "SquareSupersetEqual",  8850}, /* SQUARE ORIGINAL OF OR EQUAL TO */
  { "SquareUnion",       8852}, /* SQUARE CUP */
  { "Sscr",            119982}, /* MATHEMATICAL SCRIPT CAPITAL S */
  { "Star",              8902}, /* STAR OPERATOR */
  { "Sub",               8912}, /* DOUBLE SUBSET */
  { "Subset",            8912}, /* DOUBLE SUBSET */
  { "SubsetEqual",       8838}, /* SUBSET OF OR EQUAL TO */
  { "Succeeds",          8827}, /* SUCCEEDS */
  { "SucceedsEqual",    10928}, /* SUCCEEDS ABOVE SINGLE-LINE EQUALS SIGN */
  { "SucceedsSlantEqual",  8829}, /* SUCCEEDS OR EQUAL TO */
  { "SucceedsTilde",     8831}, /* SUCCEEDS OR EQUIVALENT TO */
  { "SuchThat",          8715}, /* CONTAINS AS MEMBER */
  { "Sum",               8721}, /* N-ARY SUMMATION */
  { "Sup",               8913}, /* DOUBLE SUPERSET */
  { "Superset",          8835}, /* SUPERSET OF */
  { "SupersetEqual",     8839}, /* SUPERSET OF OR EQUAL TO */
  { "Supset",            8913}, /* DOUBLE SUPERSET */
  { "THORN",              222}, /* LATIN CAPITAL LETTER THORN */
  { "THgr",               920}, /* GREEK CAPITAL LETTER THETA */
  { "TRADE",             8482}, /* TRADE MARK SIGN */
  { "TSHcy",             1035}, /* CYRILLIC CAPITAL LETTER TSHE */
  { "TScy",              1062}, /* CYRILLIC CAPITAL LETTER TSE */
  { "Tab",                  9}, /* CHARACTER TABULATION */
  { "Tau",                932}, /* GREEK CAPITAL LETTER TAU */
  { "Tcaron",             356}, /* LATIN CAPITAL LETTER T WITH CARON */
  { "Tcedil",             354}, /* LATIN CAPITAL LETTER T WITH CEDILLA */
  { "Tcy",               1058}, /* CYRILLIC CAPITAL LETTER TE */
  { "Tfr",             120087}, /* MATHEMATICAL FRAKTUR CAPITAL T */
  { "Tgr",                932}, /* GREEK CAPITAL LETTER TAU */
  { "Therefore",         8756}, /* THEREFORE */
  { "Theta",              920}, /* GREEK CAPITAL LETTER THETA */
  { "ThinSpace",         8201}, /* THIN SPACE */
  { "Tilde",             8764}, /* TILDE OPERATOR */
  { "TildeEqual",        8771}, /* ASYMPTOTICALLY EQUAL TO */
  { "TildeFullEqual",    8773}, /* APPROXIMATELY EQUAL TO */
  { "TildeTilde",        8776}, /* ALMOST EQUAL TO */
  { "Topf",            120139}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL T */
  { "TripleDot",         8411}, /* COMBINING THREE DOTS ABOVE */
  { "Tscr",            119983}, /* MATHEMATICAL SCRIPT CAPITAL T */
  { "Tstrok",             358}, /* LATIN CAPITAL LETTER T WITH STROKE */
  { "Uacgr",              910}, /* GREEK CAPITAL LETTER UPSILON WITH TONOS */
  { "Uacute",             218}, /* LATIN CAPITAL LETTER U WITH ACUTE */
  { "Uarr",              8607}, /* UPWARDS TWO HEADED ARROW */
  { "Uarrocir",         10569}, /* UPWARDS TWO-HEADED ARROW FROM SMALL CIRCLE */
  { "Ubrcy",             1038}, /* CYRILLIC CAPITAL LETTER SHORT U */
  { "Ubreve",             364}, /* LATIN CAPITAL LETTER U WITH BREVE */
  { "Ucirc",              219}, /* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
  { "Ucy",               1059}, /* CYRILLIC CAPITAL LETTER U */
  { "Udblac",             368}, /* LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */
  { "Udigr",              939}, /* GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA */
  { "Ufr",             120088}, /* MATHEMATICAL FRAKTUR CAPITAL U */
  { "Ugr",                933}, /* GREEK CAPITAL LETTER UPSILON */
  { "Ugrave",             217}, /* LATIN CAPITAL LETTER U WITH GRAVE */
  { "Umacr",              362}, /* LATIN CAPITAL LETTER U WITH MACRON */
  { "UnderBar",            95}, /* LOW LINE */
  { "UnderBrace",        9183}, /* BOTTOM CURLY BRACKET */
  { "UnderBracket",      9141}, /* BOTTOM SQUARE BRACKET */
  { "UnderParenthesis",  9181}, /* BOTTOM PARENTHESIS */
  { "Union",             8899}, /* N-ARY UNION */
  { "UnionPlus",         8846}, /* MULTISET UNION */
  { "Uogon",              370}, /* LATIN CAPITAL LETTER U WITH OGONEK */
  { "Uopf",            120140}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL U */
  { "UpArrow",           8593}, /* UPWARDS ARROW */
  { "UpArrowBar",       10514}, /* UPWARDS ARROW TO BAR */
  { "UpArrowDownArrow",  8645}, /* UPWARDS ARROW LEFTWARDS OF DOWNWARDS ARROW */
  { "UpDownArrow",       8597}, /* UP DOWN ARROW */
  { "UpEquilibrium",    10606}, /* UPWARDS HARPOON WITH BARB LEFT BESIDE DOWNWARDS HARPOON WITH BARB RIGHT */
  { "UpTee",             8869}, /* UP TACK */
  { "UpTeeArrow",        8613}, /* UPWARDS ARROW FROM BAR */
  { "Uparrow",           8657}, /* UPWARDS DOUBLE ARROW */
  { "Updownarrow",       8661}, /* UP DOWN DOUBLE ARROW */
  { "UpperLeftArrow",    8598}, /* NORTH WEST ARROW */
  { "UpperRightArrow",   8599}, /* NORTH EAST ARROW */
  { "Upsi",               978}, /* GREEK UPSILON WITH HOOK SYMBOL */
  { "Upsilon",            933}, /* GREEK CAPITAL LETTER UPSILON */
  { "Uring",              366}, /* LATIN CAPITAL LETTER U WITH RING ABOVE */
  { "Uscr",            119984}, /* MATHEMATICAL SCRIPT CAPITAL U */
  { "Utilde",             360}, /* LATIN CAPITAL LETTER U WITH TILDE */
  { "Uuml",               220}, /* LATIN CAPITAL LETTER U WITH DIAERESIS */
  { "VDash",             8875}, /* DOUBLE VERTICAL BAR DOUBLE RIGHT TURNSTILE */
  { "Vbar",             10987}, /* DOUBLE UP TACK */
  { "Vcy",               1042}, /* CYRILLIC CAPITAL LETTER VE */
  { "Vdash",             8873}, /* FORCES */
  { "Vdashl",           10982}, /* LONG DASH FROM LEFT MEMBER OF DOUBLE VERTICAL */
  { "Vee",               8897}, /* N-ARY LOGICAL OR */
  { "Verbar",            8214}, /* DOUBLE VERTICAL LINE */
  { "Vert",              8214}, /* DOUBLE VERTICAL LINE */
  { "VerticalBar",       8739}, /* DIVIDES */
  { "VerticalLine",       124}, /* VERTICAL LINE */
  { "VerticalSeparator", 10072}, /* LIGHT VERTICAL BAR */
  { "VerticalTilde",     8768}, /* WREATH PRODUCT */
  { "VeryThinSpace",     8202}, /* HAIR SPACE */
  { "Vfr",             120089}, /* MATHEMATICAL FRAKTUR CAPITAL V */
  { "Vopf",            120141}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL V */
  { "Vscr",            119985}, /* MATHEMATICAL SCRIPT CAPITAL V */
  { "Vvdash",            8874}, /* TRIPLE VERTICAL BAR RIGHT TURNSTILE */
  { "Wcirc",              372}, /* LATIN CAPITAL LETTER W WITH CIRCUMFLEX */
  { "Wedge",             8896}, /* N-ARY LOGICAL AND */
  { "Wfr",             120090}, /* MATHEMATICAL FRAKTUR CAPITAL W */
  { "Wopf",            120142}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL W */
  { "Wscr",            119986}, /* MATHEMATICAL SCRIPT CAPITAL W */
  { "Xfr",             120091}, /* MATHEMATICAL FRAKTUR CAPITAL X */
  { "Xgr",                926}, /* GREEK CAPITAL LETTER XI */
  { "Xi",                 926}, /* GREEK CAPITAL LETTER XI */
  { "Xopf",            120143}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL X */
  { "Xscr",            119987}, /* MATHEMATICAL SCRIPT CAPITAL X */
  { "YAcy",              1071}, /* CYRILLIC CAPITAL LETTER YA */
  { "YIcy",              1031}, /* CYRILLIC CAPITAL LETTER YI */
  { "YUcy",              1070}, /* CYRILLIC CAPITAL LETTER YU */
  { "Yacute",             221}, /* LATIN CAPITAL LETTER Y WITH ACUTE */
  { "Ycirc",              374}, /* LATIN CAPITAL LETTER Y WITH CIRCUMFLEX */
  { "Ycy",               1067}, /* CYRILLIC CAPITAL LETTER YERU */
  { "Yfr",             120092}, /* MATHEMATICAL FRAKTUR CAPITAL Y */
  { "Yopf",            120144}, /* MATHEMATICAL DOUBLE-STRUCK CAPITAL Y */
  { "Yscr",            119988}, /* MATHEMATICAL SCRIPT CAPITAL Y */
  { "Yuml",               376}, /* LATIN CAPITAL LETTER Y WITH DIAERESIS */
  { "ZHcy",              1046}, /* CYRILLIC CAPITAL LETTER ZHE */
  { "Zacute",             377}, /* LATIN CAPITAL LETTER Z WITH ACUTE */
  { "Zcaron",             381}, /* LATIN CAPITAL LETTER Z WITH CARON */
  { "Zcy",               1047}, /* CYRILLIC CAPITAL LETTER ZE */
  { "Zdot",               379}, /* LATIN CAPITAL LETTER Z WITH DOT ABOVE */
  { "ZeroWidthSpace",    8203}, /* ZERO WIDTH SPACE */
  { "Zeta",               918}, /* GREEK CAPITAL LETTER ZETA */
  { "Zfr",               8488}, /* BLACK-LETTER CAPITAL Z */
  { "Zgr",                918}, /* GREEK CAPITAL LETTER ZETA */
  { "Zopf",              8484}, /* DOUBLE-STRUCK CAPITAL Z */
  { "Zscr",            119989}, /* MATHEMATICAL SCRIPT CAPITAL Z */
  { "aacgr",              940}, /* GREEK SMALL LETTER ALPHA WITH TONOS */
  { "aacute",             225}, /* LATIN SMALL LETTER A WITH ACUTE */
  { "abreve",             259}, /* LATIN SMALL LETTER A WITH BREVE */
  { "ac",                8766}, /* INVERTED LAZY S */
  { "acd",               8767}, /* SINE WAVE */
  { "acirc",              226}, /* LATIN SMALL LETTER A WITH CIRCUMFLEX */
  { "acute",              180}, /* ACUTE ACCENT */
  { "acy",               1072}, /* CYRILLIC SMALL LETTER A */
  { "aelig",              230}, /* LATIN SMALL LETTER AE */
  { "af",                8289}, /* FUNCTION APPLICATION */
  { "afr",             120094}, /* MATHEMATICAL FRAKTUR SMALL A */
  { "agr",                945}, /* GREEK SMALL LETTER ALPHA */
  { "agrave",             224}, /* LATIN SMALL LETTER A WITH GRAVE */
  { "alefsym",           8501}, /* ALEF SYMBOL */
  { "aleph",             8501}, /* ALEF SYMBOL */
  { "alpha",              945}, /* GREEK SMALL LETTER ALPHA */
  { "amacr",              257}, /* LATIN SMALL LETTER A WITH MACRON */
  { "amalg",            10815}, /* AMALGAMATION OR COPRODUCT */
  { "amp",                 38}, /* AMPERSAND */
  { "and",               8743}, /* LOGICAL AND */
  { "andand",           10837}, /* TWO INTERSECTING LOGICAL AND */
  { "andd",             10844}, /* LOGICAL AND WITH HORIZONTAL DASH */
  { "andslope",         10840}, /* SLOPING LARGE AND */
  { "andv",             10842}, /* LOGICAL AND WITH MIDDLE STEM */
  { "ang",               8736}, /* ANGLE */
  { "ange",             10660}, /* ANGLE WITH UNDERBAR */
  { "angle",             8736}, /* ANGLE */
  { "angmsd",            8737}, /* MEASURED ANGLE */
  { "angmsdaa",         10664}, /* MEASURED ANGLE WITH OPEN ARM ENDING IN ARROW POINTING UP AND RIGHT */
  { "angmsdab",         10665}, /* MEASURED ANGLE WITH OPEN ARM ENDING IN ARROW POINTING UP AND LEFT */
  { "angmsdac",         10666}, /* MEASURED ANGLE WITH OPEN ARM ENDING IN ARROW POINTING DOWN AND RIGHT */
  { "angmsdad",         10667}, /* MEASURED ANGLE WITH OPEN ARM ENDING IN ARROW POINTING DOWN AND LEFT */
  { "angmsdae",         10668}, /* MEASURED ANGLE WITH OPEN ARM ENDING IN ARROW POINTING RIGHT AND UP */
  { "angmsdaf",         10669}, /* MEASURED ANGLE WITH OPEN ARM ENDING IN ARROW POINTING LEFT AND UP */
  { "angmsdag",         10670}, /* MEASURED ANGLE WITH OPEN ARM ENDING IN ARROW POINTING RIGHT AND DOWN */
  { "angmsdah",         10671}, /* MEASURED ANGLE WITH OPEN ARM ENDING IN ARROW POINTING LEFT AND DOWN */
  { "angrt",             8735}, /* RIGHT ANGLE */
  { "angrtvb",           8894}, /* RIGHT ANGLE WITH ARC */
  { "angrtvbd",         10653}, /* MEASURED RIGHT ANGLE WITH DOT */
  { "angsph",            8738}, /* SPHERICAL ANGLE */
  { "angst",              197}, /* LATIN CAPITAL LETTER A WITH RING ABOVE */
  { "angzarr",           9084}, /* RIGHT ANGLE WITH DOWNWARDS ZIGZAG ARROW */
  { "aogon",              261}, /* LATIN SMALL LETTER A WITH OGONEK */
  { "aopf",            120146}, /* MATHEMATICAL DOUBLE-STRUCK SMALL A */
  { "ap",                8776}, /* ALMOST EQUAL TO */
  { "apE",              10864}, /* APPROXIMATELY EQUAL OR EQUAL TO */
  { "apacir",           10863}, /* ALMOST EQUAL TO WITH CIRCUMFLEX ACCENT */
  { "ape",               8778}, /* ALMOST EQUAL OR EQUAL TO */
  { "apid",              8779}, /* TRIPLE TILDE */
  { "apos",                39}, /* APOSTROPHE */
  { "approx",            8776}, /* ALMOST EQUAL TO */
  { "approxeq",          8778}, /* ALMOST EQUAL OR EQUAL TO */
  { "aring",              229}, /* LATIN SMALL LETTER A WITH RING ABOVE */
  { "ascr",            119990}, /* MATHEMATICAL SCRIPT SMALL A */
  { "ast",                 42}, /* ASTERISK */
  { "asymp",             8776}, /* ALMOST EQUAL TO */
  { "asympeq",           8781}, /* EQUIVALENT TO */
  { "atilde",             227}, /* LATIN SMALL LETTER A WITH TILDE */
  { "auml",               228}, /* LATIN SMALL LETTER A WITH DIAERESIS */
  { "awconint",          8755}, /* ANTICLOCKWISE CONTOUR INTEGRAL */
  { "awint",            10769}, /* ANTICLOCKWISE INTEGRATION */
  { "b.Delta",         120491}, /* MATHEMATICAL BOLD CAPITAL DELTA */
  { "b.Gamma",         120490}, /* MATHEMATICAL BOLD CAPITAL GAMMA */
  { "b.Gammad",        120778}, /* MATHEMATICAL BOLD CAPITAL DIGAMMA */
  { "b.Lambda",        120498}, /* MATHEMATICAL BOLD CAPITAL LAMDA */
  { "b.Omega",         120512}, /* MATHEMATICAL BOLD CAPITAL OMEGA */
  { "b.Phi",           120509}, /* MATHEMATICAL BOLD CAPITAL PHI */
  { "b.Pi",            120503}, /* MATHEMATICAL BOLD CAPITAL PI */
  { "b.Psi",           120511}, /* MATHEMATICAL BOLD CAPITAL PSI */
  { "b.Sigma",         120506}, /* MATHEMATICAL BOLD CAPITAL SIGMA */
  { "b.Theta",         120495}, /* MATHEMATICAL BOLD CAPITAL THETA */
  { "b.Upsi",          120508}, /* MATHEMATICAL BOLD CAPITAL UPSILON */
  { "b.Xi",            120501}, /* MATHEMATICAL BOLD CAPITAL XI */
  { "b.alpha",         120514}, /* MATHEMATICAL BOLD SMALL ALPHA */
  { "b.beta",          120515}, /* MATHEMATICAL BOLD SMALL BETA */
  { "b.chi",           120536}, /* MATHEMATICAL BOLD SMALL CHI */
  { "b.delta",         120517}, /* MATHEMATICAL BOLD SMALL DELTA */
  { "b.epsi",          120518}, /* MATHEMATICAL BOLD SMALL EPSILON */
  { "b.epsiv",         120540}, /* MATHEMATICAL BOLD EPSILON SYMBOL */
  { "b.eta",           120520}, /* MATHEMATICAL BOLD SMALL ETA */
  { "b.gamma",         120516}, /* MATHEMATICAL BOLD SMALL GAMMA */
  { "b.gammad",        120779}, /* MATHEMATICAL BOLD SMALL DIGAMMA */
  { "b.iota",          120522}, /* MATHEMATICAL BOLD SMALL IOTA */
  { "b.kappa",         120523}, /* MATHEMATICAL BOLD SMALL KAPPA */
  { "b.kappav",        120542}, /* MATHEMATICAL BOLD KAPPA SYMBOL */
  { "b.lambda",        120524}, /* MATHEMATICAL BOLD SMALL LAMDA */
  { "b.mu",            120525}, /* MATHEMATICAL BOLD SMALL MU */
  { "b.nu",            120526}, /* MATHEMATICAL BOLD SMALL NU */
  { "b.omega",         120538}, /* MATHEMATICAL BOLD SMALL OMEGA */
  { "b.phi",           120535}, /* MATHEMATICAL BOLD SMALL PHI */
  { "b.phiv",          120543}, /* MATHEMATICAL BOLD PHI SYMBOL */
  { "b.pi",            120529}, /* MATHEMATICAL BOLD SMALL PI */
  { "b.piv",           120545}, /* MATHEMATICAL BOLD PI SYMBOL */
  { "b.psi",           120537}, /* MATHEMATICAL BOLD SMALL PSI */
  { "b.rho",           120530}, /* MATHEMATICAL BOLD SMALL RHO */
  { "b.rhov",          120544}, /* MATHEMATICAL BOLD RHO SYMBOL */
  { "b.sigma",         120532}, /* MATHEMATICAL BOLD SMALL SIGMA */
  { "b.sigmav",        120531}, /* MATHEMATICAL BOLD SMALL FINAL SIGMA */
  { "b.tau",           120533}, /* MATHEMATICAL BOLD SMALL TAU */
  { "b.thetas",        120521}, /* MATHEMATICAL BOLD SMALL THETA */
  { "b.thetav",        120541}, /* MATHEMATICAL BOLD THETA SYMBOL */
  { "b.upsi",          120534}, /* MATHEMATICAL BOLD SMALL UPSILON */
  { "b.xi",            120527}, /* MATHEMATICAL BOLD SMALL XI */
  { "b.zeta",          120519}, /* MATHEMATICAL BOLD SMALL ZETA */
  { "bNot",             10989}, /* REVERSED DOUBLE STROKE NOT SIGN */
  { "backcong",          8780}, /* ALL EQUAL TO */
  { "backepsilon",       1014}, /* GREEK REVERSED LUNATE EPSILON SYMBOL */
  { "backprime",         8245}, /* REVERSED PRIME */
  { "backsim",           8765}, /* REVERSED TILDE */
  { "backsimeq",         8909}, /* REVERSED TILDE EQUALS */
  { "barvee",            8893}, /* NOR */
  { "barwed",            8965}, /* PROJECTIVE */
  { "barwedge",          8965}, /* PROJECTIVE */
  { "bbrk",              9141}, /* BOTTOM SQUARE BRACKET */
  { "bbrktbrk",          9142}, /* BOTTOM SQUARE BRACKET OVER TOP SQUARE BRACKET */
  { "bcong",             8780}, /* ALL EQUAL TO */
  { "bcy",               1073}, /* CYRILLIC SMALL LETTER BE */
  { "bdquo",             8222}, /* DOUBLE LOW-9 QUOTATION MARK */
  { "becaus",            8757}, /* BECAUSE */
  { "because",           8757}, /* BECAUSE */
  { "bemptyv",          10672}, /* REVERSED EMPTY SET */
  { "bepsi",             1014}, /* GREEK REVERSED LUNATE EPSILON SYMBOL */
  { "bernou",            8492}, /* SCRIPT CAPITAL B */
  { "beta",               946}, /* GREEK SMALL LETTER BETA */
  { "beth",              8502}, /* BET SYMBOL */
  { "between",           8812}, /* BETWEEN */
  { "bfr",             120095}, /* MATHEMATICAL FRAKTUR SMALL B */
  { "bgr",                946}, /* GREEK SMALL LETTER BETA */
  { "bigcap",            8898}, /* N-ARY INTERSECTION */
  { "bigcirc",           9711}, /* LARGE CIRCLE */
  { "bigcup",            8899}, /* N-ARY UNION */
  { "bigodot",          10752}, /* N-ARY CIRCLED DOT OPERATOR */
  { "bigoplus",         10753}, /* N-ARY CIRCLED PLUS OPERATOR */
  { "bigotimes",        10754}, /* N-ARY CIRCLED TIMES OPERATOR */
  { "bigsqcup",         10758}, /* N-ARY SQUARE UNION OPERATOR */
  { "bigstar",           9733}, /* BLACK STAR */
  { "bigtriangledown",   9661}, /* WHITE DOWN-POINTING TRIANGLE */
  { "bigtriangleup",     9651}, /* WHITE UP-POINTING TRIANGLE */
  { "biguplus",         10756}, /* N-ARY UNION OPERATOR WITH PLUS */
  { "bigvee",            8897}, /* N-ARY LOGICAL OR */
  { "bigwedge",          8896}, /* N-ARY LOGICAL AND */
  { "bkarow",           10509}, /* RIGHTWARDS DOUBLE DASH ARROW */
  { "blacklozenge",     10731}, /* BLACK LOZENGE */
  { "blacksquare",       9642}, /* BLACK SMALL SQUARE */
  { "blacktriangle",     9652}, /* BLACK UP-POINTING SMALL TRIANGLE */
  { "blacktriangledown",  9662}, /* BLACK DOWN-POINTING SMALL TRIANGLE */
  { "blacktriangleleft",  9666}, /* BLACK LEFT-POINTING SMALL TRIANGLE */
  { "blacktriangleright",  9656}, /* BLACK RIGHT-POINTING SMALL TRIANGLE */
  { "blank",             9251}, /* OPEN BOX */
  { "blk12",             9618}, /* MEDIUM SHADE */
  { "blk14",             9617}, /* LIGHT SHADE */
  { "blk34",             9619}, /* DARK SHADE */
  { "block",             9608}, /* FULL BLOCK */
  { "bnot",              8976}, /* REVERSED NOT SIGN */
  { "bopf",            120147}, /* MATHEMATICAL DOUBLE-STRUCK SMALL B */
  { "bot",               8869}, /* UP TACK */
  { "bottom",            8869}, /* UP TACK */
  { "bowtie",            8904}, /* BOWTIE */
  { "boxDL",             9559}, /* BOX DRAWINGS DOUBLE DOWN AND LEFT */
  { "boxDR",             9556}, /* BOX DRAWINGS DOUBLE DOWN AND RIGHT */
  { "boxDl",             9558}, /* BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE */
  { "boxDr",             9555}, /* BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE */
  { "boxH",              9552}, /* BOX DRAWINGS DOUBLE HORIZONTAL */
  { "boxHD",             9574}, /* BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL */
  { "boxHU",             9577}, /* BOX DRAWINGS DOUBLE UP AND HORIZONTAL */
  { "boxHd",             9572}, /* BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE */
  { "boxHu",             9575}, /* BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE */
  { "boxUL",             9565}, /* BOX DRAWINGS DOUBLE UP AND LEFT */
  { "boxUR",             9562}, /* BOX DRAWINGS DOUBLE UP AND RIGHT */
  { "boxUl",             9564}, /* BOX DRAWINGS UP DOUBLE AND LEFT SINGLE */
  { "boxUr",             9561}, /* BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE */
  { "boxV",              9553}, /* BOX DRAWINGS DOUBLE VERTICAL */
  { "boxVH",             9580}, /* BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL */
  { "boxVL",             9571}, /* BOX DRAWINGS DOUBLE VERTICAL AND LEFT */
  { "boxVR",             9568}, /* BOX DRAWINGS DOUBLE VERTICAL AND RIGHT */
  { "boxVh",             9579}, /* BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SINGLE */
  { "boxVl",             9570}, /* BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE */
  { "boxVr",             9567}, /* BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE */
  { "boxbox",           10697}, /* TWO JOINED SQUARES */
  { "boxdL",             9557}, /* BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE */
  { "boxdR",             9554}, /* BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE */
  { "boxdl",             9488}, /* BOX DRAWINGS LIGHT DOWN AND LEFT */
  { "boxdr",             9484}, /* BOX DRAWINGS LIGHT DOWN AND RIGHT */
  { "boxh",              9472}, /* BOX DRAWINGS LIGHT HORIZONTAL */
  { "boxhD",             9573}, /* BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE */
  { "boxhU",             9576}, /* BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE */
  { "boxhd",             9516}, /* BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
  { "boxhu",             9524}, /* BOX DRAWINGS LIGHT UP AND HORIZONTAL */
  { "boxminus",          8863}, /* SQUARED MINUS */
  { "boxplus",           8862}, /* SQUARED PLUS */
  { "boxtimes",          8864}, /* SQUARED TIMES */
  { "boxuL",             9563}, /* BOX DRAWINGS UP SINGLE AND LEFT DOUBLE */
  { "boxuR",             9560}, /* BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE */
  { "boxul",             9496}, /* BOX DRAWINGS LIGHT UP AND LEFT */
  { "boxur",             9492}, /* BOX DRAWINGS LIGHT UP AND RIGHT */
  { "boxv",              9474}, /* BOX DRAWINGS LIGHT VERTICAL */
  { "boxvH",             9578}, /* BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE */
  { "boxvL",             9569}, /* BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE */
  { "boxvR",             9566}, /* BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE */
  { "boxvh",             9532}, /* BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
  { "boxvl",             9508}, /* BOX DRAWINGS LIGHT VERTICAL AND LEFT */
  { "boxvr",             9500}, /* BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
  { "bprime",            8245}, /* REVERSED PRIME */
  { "breve",              728}, /* BREVE */
  { "brvbar",             166}, /* BROKEN BAR */
  { "bscr",            119991}, /* MATHEMATICAL SCRIPT SMALL B */
  { "bsemi",             8271}, /* REVERSED SEMICOLON */
  { "bsim",              8765}, /* REVERSED TILDE */
  { "bsime",             8909}, /* REVERSED TILDE EQUALS */
  { "bsol",                92}, /* REVERSE SOLIDUS */
  { "bsolb",            10693}, /* SQUARED FALLING DIAGONAL SLASH */
  { "bsolhsub",         10184}, /* REVERSE SOLIDUS PRECEDING SUBSET */
  { "bull",              8226}, /* BULLET */
  { "bullet",            8226}, /* BULLET */
  { "bump",              8782}, /* GEOMETRICALLY EQUIVALENT TO */
  { "bumpE",            10926}, /* EQUALS SIGN WITH BUMPY ABOVE */
  { "bumpe",             8783}, /* DIFFERENCE BETWEEN */
  { "bumpeq",            8783}, /* DIFFERENCE BETWEEN */
  { "cacute",             263}, /* LATIN SMALL LETTER C WITH ACUTE */
  { "cap",               8745}, /* INTERSECTION */
  { "capand",           10820}, /* INTERSECTION WITH LOGICAL AND */
  { "capbrcup",         10825}, /* INTERSECTION ABOVE BAR ABOVE UNION */
  { "capcap",           10827}, /* INTERSECTION BESIDE AND JOINED WITH INTERSECTION */
  { "capcup",           10823}, /* INTERSECTION ABOVE UNION */
  { "capdot",           10816}, /* INTERSECTION WITH DOT */
  { "caret",             8257}, /* CARET INSERTION POINT */
  { "caron",              711}, /* CARON */
  { "ccaps",            10829}, /* CLOSED INTERSECTION WITH SERIFS */
  { "ccaron",             269}, /* LATIN SMALL LETTER C WITH CARON */
  { "ccedil",             231}, /* LATIN SMALL LETTER C WITH CEDILLA */
  { "ccirc",              265}, /* LATIN SMALL LETTER C WITH CIRCUMFLEX */
  { "ccups",            10828}, /* CLOSED UNION WITH SERIFS */
  { "ccupssm",          10832}, /* CLOSED UNION WITH SERIFS AND SMASH PRODUCT */
  { "cdot",               267}, /* LATIN SMALL LETTER C WITH DOT ABOVE */
  { "cedil",              184}, /* CEDILLA */
  { "cemptyv",          10674}, /* EMPTY SET WITH SMALL CIRCLE ABOVE */
  { "cent",               162}, /* CENT SIGN */
  { "centerdot",          183}, /* MIDDLE DOT */
  { "cfr",             120096}, /* MATHEMATICAL FRAKTUR SMALL C */
  { "chcy",              1095}, /* CYRILLIC SMALL LETTER CHE */
  { "check",            10003}, /* CHECK MARK */
  { "checkmark",        10003}, /* CHECK MARK */
  { "chi",                967}, /* GREEK SMALL LETTER CHI */
  { "cir",               9675}, /* WHITE CIRCLE */
  { "cirE",             10691}, /* CIRCLE WITH TWO HORIZONTAL STROKES TO THE RIGHT */
  { "circ",               710}, /* MODIFIER LETTER CIRCUMFLEX ACCENT */
  { "circeq",            8791}, /* RING EQUAL TO */
  { "circlearrowleft",   8634}, /* ANTICLOCKWISE OPEN CIRCLE ARROW */
  { "circlearrowright",  8635}, /* CLOCKWISE OPEN CIRCLE ARROW */
  { "circledR",           174}, /* REGISTERED SIGN */
  { "circledS",          9416}, /* CIRCLED LATIN CAPITAL LETTER S */
  { "circledast",        8859}, /* CIRCLED ASTERISK OPERATOR */
  { "circledcirc",       8858}, /* CIRCLED RING OPERATOR */
  { "circleddash",       8861}, /* CIRCLED DASH */
  { "cire",              8791}, /* RING EQUAL TO */
  { "cirfnint",         10768}, /* CIRCULATION FUNCTION */
  { "cirmid",           10991}, /* VERTICAL LINE WITH CIRCLE ABOVE */
  { "cirscir",          10690}, /* CIRCLE WITH SMALL CIRCLE TO THE RIGHT */
  { "clubs",             9827}, /* BLACK CLUB SUIT */
  { "clubsuit",          9827}, /* BLACK CLUB SUIT */
  { "colon",               58}, /* COLON */
  { "colone",            8788}, /* COLON EQUALS */
  { "coloneq",           8788}, /* COLON EQUALS */
  { "comma",               44}, /* COMMA */
  { "commat",              64}, /* COMMERCIAL AT */
  { "comp",              8705}, /* COMPLEMENT */
  { "compfn",            8728}, /* RING OPERATOR */
  { "complement",        8705}, /* COMPLEMENT */
  { "complexes",         8450}, /* DOUBLE-STRUCK CAPITAL C */
  { "cong",              8773}, /* APPROXIMATELY EQUAL TO */
  { "congdot",          10861}, /* CONGRUENT WITH DOT ABOVE */
  { "conint",            8750}, /* CONTOUR INTEGRAL */
  { "copf",            120148}, /* MATHEMATICAL DOUBLE-STRUCK SMALL C */
  { "coprod",            8720}, /* N-ARY COPRODUCT */
  { "copy",               169}, /* COPYRIGHT SIGN */
  { "copysr",            8471}, /* SOUND RECORDING COPYRIGHT */
  { "crarr",             8629}, /* DOWNWARDS ARROW WITH CORNER LEFTWARDS */
  { "cross",            10007}, /* BALLOT X */
  { "cscr",            119992}, /* MATHEMATICAL SCRIPT SMALL C */
  { "csub",             10959}, /* CLOSED SUBSET */
  { "csube",            10961}, /* CLOSED SUBSET OR EQUAL TO */
  { "csup",             10960}, /* CLOSED SUPERSET */
  { "csupe",            10962}, /* CLOSED SUPERSET OR EQUAL TO */
  { "ctdot",             8943}, /* MIDLINE HORIZONTAL ELLIPSIS */
  { "cudarrl",          10552}, /* RIGHT-SIDE ARC CLOCKWISE ARROW */
  { "cudarrr",          10549}, /* ARROW POINTING RIGHTWARDS THEN CURVING DOWNWARDS */
  { "cuepr",             8926}, /* EQUAL TO OR PRECEDES */
  { "cuesc",             8927}, /* EQUAL TO OR SUCCEEDS */
  { "cularr",            8630}, /* ANTICLOCKWISE TOP SEMICIRCLE ARROW */
  { "cularrp",          10557}, /* TOP ARC ANTICLOCKWISE ARROW WITH PLUS */
  { "cup",               8746}, /* UNION */
  { "cupbrcap",         10824}, /* UNION ABOVE BAR ABOVE INTERSECTION */
  { "cupcap",           10822}, /* UNION ABOVE INTERSECTION */
  { "cupcup",           10826}, /* UNION BESIDE AND JOINED WITH UNION */
  { "cupdot",            8845}, /* MULTISET MULTIPLICATION */
  { "cupor",            10821}, /* UNION WITH LOGICAL OR */
  { "curarr",            8631}, /* CLOCKWISE TOP SEMICIRCLE ARROW */
  { "curarrm",          10556}, /* TOP ARC CLOCKWISE ARROW WITH MINUS */
  { "curlyeqprec",       8926}, /* EQUAL TO OR PRECEDES */
  { "curlyeqsucc",       8927}, /* EQUAL TO OR SUCCEEDS */
  { "curlyvee",          8910}, /* CURLY LOGICAL OR */
  { "curlywedge",        8911}, /* CURLY LOGICAL AND */
  { "curren",             164}, /* CURRENCY SIGN */
  { "curvearrowleft",    8630}, /* ANTICLOCKWISE TOP SEMICIRCLE ARROW */
  { "curvearrowright",   8631}, /* CLOCKWISE TOP SEMICIRCLE ARROW */
  { "cuvee",             8910}, /* CURLY LOGICAL OR */
  { "cuwed",             8911}, /* CURLY LOGICAL AND */
  { "cwconint",          8754}, /* CLOCKWISE CONTOUR INTEGRAL */
  { "cwint",             8753}, /* CLOCKWISE INTEGRAL */
  { "cylcty",            9005}, /* CYLINDRICITY */
  { "dArr",              8659}, /* DOWNWARDS DOUBLE ARROW */
  { "dHar",             10597}, /* DOWNWARDS HARPOON WITH BARB LEFT BESIDE DOWNWARDS HARPOON WITH BARB RIGHT */
  { "dagger",            8224}, /* DAGGER */
  { "daleth",            8504}, /* DALET SYMBOL */
  { "darr",              8595}, /* DOWNWARDS ARROW */
  { "dash",              8208}, /* HYPHEN */
  { "dashv",             8867}, /* LEFT TACK */
  { "dbkarow",          10511}, /* RIGHTWARDS TRIPLE DASH ARROW */
  { "dblac",              733}, /* DOUBLE ACUTE ACCENT */
  { "dcaron",             271}, /* LATIN SMALL LETTER D WITH CARON */
  { "dcy",               1076}, /* CYRILLIC SMALL LETTER DE */
  { "dd",                8518}, /* DOUBLE-STRUCK ITALIC SMALL D */
  { "ddagger",           8225}, /* DOUBLE DAGGER */
  { "ddarr",             8650}, /* DOWNWARDS PAIRED ARROWS */
  { "ddotseq",          10871}, /* EQUALS SIGN WITH TWO DOTS ABOVE AND TWO DOTS BELOW */
  { "deg",                176}, /* DEGREE SIGN */
  { "delta",              948}, /* GREEK SMALL LETTER DELTA */
  { "demptyv",          10673}, /* EMPTY SET WITH OVERBAR */
  { "dfisht",           10623}, /* DOWN FISH TAIL */
  { "dfr",             120097}, /* MATHEMATICAL FRAKTUR SMALL D */
  { "dgr",                948}, /* GREEK SMALL LETTER DELTA */
  { "dharl",             8643}, /* DOWNWARDS HARPOON WITH BARB LEFTWARDS */
  { "dharr",             8642}, /* DOWNWARDS HARPOON WITH BARB RIGHTWARDS */
  { "diam",              8900}, /* DIAMOND OPERATOR */
  { "diamond",           8900}, /* DIAMOND OPERATOR */
  { "diamondsuit",       9830}, /* BLACK DIAMOND SUIT */
  { "diams",             9830}, /* BLACK DIAMOND SUIT */
  { "die",                168}, /* DIAERESIS */
  { "digamma",            989}, /* GREEK SMALL LETTER DIGAMMA */
  { "disin",             8946}, /* ELEMENT OF WITH LONG HORIZONTAL STROKE */
  { "div",                247}, /* DIVISION SIGN */
  { "divide",             247}, /* DIVISION SIGN */
  { "divideontimes",     8903}, /* DIVISION TIMES */
  { "divonx",            8903}, /* DIVISION TIMES */
  { "djcy",              1106}, /* CYRILLIC SMALL LETTER DJE */
  { "dlcorn",            8990}, /* BOTTOM LEFT CORNER */
  { "dlcrop",            8973}, /* BOTTOM LEFT CROP */
  { "dollar",              36}, /* DOLLAR SIGN */
  { "dopf",            120149}, /* MATHEMATICAL DOUBLE-STRUCK SMALL D */
  { "dot",                729}, /* DOT ABOVE */
  { "doteq",             8784}, /* APPROACHES THE LIMIT */
  { "doteqdot",          8785}, /* GEOMETRICALLY EQUAL TO */
  { "dotminus",          8760}, /* DOT MINUS */
  { "dotplus",           8724}, /* DOT PLUS */
  { "dotsquare",         8865}, /* SQUARED DOT OPERATOR */
  { "doublebarwedge",    8966}, /* PERSPECTIVE */
  { "downarrow",         8595}, /* DOWNWARDS ARROW */
  { "downdownarrows",    8650}, /* DOWNWARDS PAIRED ARROWS */
  { "downharpoonleft",   8643}, /* DOWNWARDS HARPOON WITH BARB LEFTWARDS */
  { "downharpoonright",  8642}, /* DOWNWARDS HARPOON WITH BARB RIGHTWARDS */
  { "drbkarow",         10512}, /* RIGHTWARDS TWO-HEADED TRIPLE DASH ARROW */
  { "drcorn",            8991}, /* BOTTOM RIGHT CORNER */
  { "drcrop",            8972}, /* BOTTOM RIGHT CROP */
  { "dscr",            119993}, /* MATHEMATICAL SCRIPT SMALL D */
  { "dscy",              1109}, /* CYRILLIC SMALL LETTER DZE */
  { "dsol",             10742}, /* SOLIDUS WITH OVERBAR */
  { "dstrok",             273}, /* LATIN SMALL LETTER D WITH STROKE */
  { "dtdot",             8945}, /* DOWN RIGHT DIAGONAL ELLIPSIS */
  { "dtri",              9663}, /* WHITE DOWN-POINTING SMALL TRIANGLE */
  { "dtrif",             9662}, /* BLACK DOWN-POINTING SMALL TRIANGLE */
  { "dwangle",          10662}, /* OBLIQUE ANGLE OPENING UP */
  { "dzcy",              1119}, /* CYRILLIC SMALL LETTER DZHE */
  { "dzigrarr",         10239}, /* LONG RIGHTWARDS SQUIGGLE ARROW */
  { "eDot",              8785}, /* GEOMETRICALLY EQUAL TO */
  { "eacgr",              941}, /* GREEK SMALL LETTER EPSILON WITH TONOS */
  { "eacute",             233}, /* LATIN SMALL LETTER E WITH ACUTE */
  { "easter",           10862}, /* EQUALS WITH ASTERISK */
  { "ecaron",             283}, /* LATIN SMALL LETTER E WITH CARON */
  { "ecir",              8790}, /* RING IN EQUAL TO */
  { "ecirc",              234}, /* LATIN SMALL LETTER E WITH CIRCUMFLEX */
  { "ecolon",            8789}, /* EQUALS COLON */
  { "ecy",               1101}, /* CYRILLIC SMALL LETTER E */
  { "edot",               279}, /* LATIN SMALL LETTER E WITH DOT ABOVE */
  { "ee",                8519}, /* DOUBLE-STRUCK ITALIC SMALL E */
  { "eeacgr",             942}, /* GREEK SMALL LETTER ETA WITH TONOS */
  { "eegr",               951}, /* GREEK SMALL LETTER ETA */
  { "efDot",             8786}, /* APPROXIMATELY EQUAL TO OR THE IMAGE OF */
  { "efr",             120098}, /* MATHEMATICAL FRAKTUR SMALL E */
  { "eg",               10906}, /* DOUBLE-LINE EQUAL TO OR GREATER-THAN */
  { "egr",                949}, /* GREEK SMALL LETTER EPSILON */
  { "egrave",             232}, /* LATIN SMALL LETTER E WITH GRAVE */
  { "egs",              10902}, /* SLANTED EQUAL TO OR GREATER-THAN */
  { "egsdot",           10904}, /* SLANTED EQUAL TO OR GREATER-THAN WITH DOT INSIDE */
  { "el",               10905}, /* DOUBLE-LINE EQUAL TO OR LESS-THAN */
  { "elinters",          9191}, /* ELECTRICAL INTERSECTION */
  { "ell",               8467}, /* SCRIPT SMALL L */
  { "els",              10901}, /* SLANTED EQUAL TO OR LESS-THAN */
  { "elsdot",           10903}, /* SLANTED EQUAL TO OR LESS-THAN WITH DOT INSIDE */
  { "emacr",              275}, /* LATIN SMALL LETTER E WITH MACRON */
  { "empty",             8709}, /* EMPTY SET */
  { "emptyset",          8709}, /* EMPTY SET */
  { "emptyv",            8709}, /* EMPTY SET */
  { "emsp",              8195}, /* EM SPACE */
  { "emsp13",            8196}, /* THREE-PER-EM SPACE */
  { "emsp14",            8197}, /* FOUR-PER-EM SPACE */
  { "eng",                331}, /* LATIN SMALL LETTER ENG */
  { "ensp",              8194}, /* EN SPACE */
  { "eogon",              281}, /* LATIN SMALL LETTER E WITH OGONEK */
  { "eopf",            120150}, /* MATHEMATICAL DOUBLE-STRUCK SMALL E */
  { "epar",              8917}, /* EQUAL AND PARALLEL TO */
  { "eparsl",           10723}, /* EQUALS SIGN AND SLANTED PARALLEL */
  { "eplus",            10865}, /* EQUALS SIGN ABOVE PLUS SIGN */
  { "epsi",               949}, /* GREEK SMALL LETTER EPSILON */
  { "epsilon",            949}, /* GREEK SMALL LETTER EPSILON */
  { "epsiv",             1013}, /* GREEK LUNATE EPSILON SYMBOL */
  { "eqcirc",            8790}, /* RING IN EQUAL TO */
  { "eqcolon",           8789}, /* EQUALS COLON */
  { "eqsim",             8770}, /* MINUS TILDE */
  { "eqslantgtr",       10902}, /* SLANTED EQUAL TO OR GREATER-THAN */
  { "eqslantless",      10901}, /* SLANTED EQUAL TO OR LESS-THAN */
  { "equals",              61}, /* EQUALS SIGN */
  { "equest",            8799}, /* QUESTIONED EQUAL TO */
  { "equiv",             8801}, /* IDENTICAL TO */
  { "equivDD",          10872}, /* EQUIVALENT WITH FOUR DOTS ABOVE */
  { "eqvparsl",         10725}, /* IDENTICAL TO AND SLANTED PARALLEL */
  { "erDot",             8787}, /* IMAGE OF OR APPROXIMATELY EQUAL TO */
  { "erarr",            10609}, /* EQUALS SIGN ABOVE RIGHTWARDS ARROW */
  { "escr",              8495}, /* SCRIPT SMALL E */
  { "esdot",             8784}, /* APPROACHES THE LIMIT */
  { "esim",              8770}, /* MINUS TILDE */
  { "eta",                951}, /* GREEK SMALL LETTER ETA */
  { "eth",                240}, /* LATIN SMALL LETTER ETH */
  { "euml",               235}, /* LATIN SMALL LETTER E WITH DIAERESIS */
  { "euro",              8364}, /* EURO SIGN */
  { "excl",                33}, /* EXCLAMATION MARK */
  { "exist",             8707}, /* THERE EXISTS */
  { "expectation",       8496}, /* SCRIPT CAPITAL E */
  { "exponentiale",      8519}, /* DOUBLE-STRUCK ITALIC SMALL E */
  { "fallingdotseq",     8786}, /* APPROXIMATELY EQUAL TO OR THE IMAGE OF */
  { "fcy",               1092}, /* CYRILLIC SMALL LETTER EF */
  { "female",            9792}, /* FEMALE SIGN */
  { "ffilig",           64259}, /* LATIN SMALL LIGATURE FFI */
  { "fflig",            64256}, /* LATIN SMALL LIGATURE FF */
  { "ffllig",           64260}, /* LATIN SMALL LIGATURE FFL */
  { "ffr",             120099}, /* MATHEMATICAL FRAKTUR SMALL F */
  { "filig",            64257}, /* LATIN SMALL LIGATURE FI */
  { "flat",              9837}, /* MUSIC FLAT SIGN */
  { "fllig",            64258}, /* LATIN SMALL LIGATURE FL */
  { "fltns",             9649}, /* WHITE PARALLELOGRAM */
  { "fnof",               402}, /* LATIN SMALL LETTER F WITH HOOK */
  { "fopf",            120151}, /* MATHEMATICAL DOUBLE-STRUCK SMALL F */
  { "forall",            8704}, /* FOR ALL */
  { "fork",              8916}, /* PITCHFORK */
  { "forkv",            10969}, /* ELEMENT OF OPENING DOWNWARDS */
  { "fpartint",         10765}, /* FINITE PART INTEGRAL */
  { "frac12",             189}, /* VULGAR FRACTION ONE HALF */
  { "frac13",            8531}, /* VULGAR FRACTION ONE THIRD */
  { "frac14",             188}, /* VULGAR FRACTION ONE QUARTER */
  { "frac15",            8533}, /* VULGAR FRACTION ONE FIFTH */
  { "frac16",            8537}, /* VULGAR FRACTION ONE SIXTH */
  { "frac18",            8539}, /* VULGAR FRACTION ONE EIGHTH */
  { "frac23",            8532}, /* VULGAR FRACTION TWO THIRDS */
  { "frac25",            8534}, /* VULGAR FRACTION TWO FIFTHS */
  { "frac34",             190}, /* VULGAR FRACTION THREE QUARTERS */
  { "frac35",            8535}, /* VULGAR FRACTION THREE FIFTHS */
  { "frac38",            8540}, /* VULGAR FRACTION THREE EIGHTHS */
  { "frac45",            8536}, /* VULGAR FRACTION FOUR FIFTHS */
  { "frac56",            8538}, /* VULGAR FRACTION FIVE SIXTHS */
  { "frac58",            8541}, /* VULGAR FRACTION FIVE EIGHTHS */
  { "frac78",            8542}, /* VULGAR FRACTION SEVEN EIGHTHS */
  { "frasl",             8260}, /* FRACTION SLASH */
  { "frown",             8994}, /* FROWN */
  { "fscr",            119995}, /* MATHEMATICAL SCRIPT SMALL F */
  { "gE",                8807}, /* GREATER-THAN OVER EQUAL TO */
  { "gacute",             501}, /* LATIN SMALL LETTER G WITH ACUTE */
  { "gamma",              947}, /* GREEK SMALL LETTER GAMMA */
  { "gammad",             989}, /* GREEK SMALL LETTER DIGAMMA */
  { "gap",              10886}, /* GREATER-THAN OR APPROXIMATE */
  { "gbreve",             287}, /* LATIN SMALL LETTER G WITH BREVE */
  { "gcirc",              285}, /* LATIN SMALL LETTER G WITH CIRCUMFLEX */
  { "gcy",               1075}, /* CYRILLIC SMALL LETTER GHE */
  { "gdot",               289}, /* LATIN SMALL LETTER G WITH DOT ABOVE */
  { "ge",                8805}, /* GREATER-THAN OR EQUAL TO */
  { "gel",               8923}, /* GREATER-THAN EQUAL TO OR LESS-THAN */
  { "geq",               8805}, /* GREATER-THAN OR EQUAL TO */
  { "geqq",              8807}, /* GREATER-THAN OVER EQUAL TO */
  { "geqslant",         10878}, /* GREATER-THAN OR SLANTED EQUAL TO */
  { "ges",              10878}, /* GREATER-THAN OR SLANTED EQUAL TO */
  { "gescc",            10921}, /* GREATER-THAN CLOSED BY CURVE ABOVE SLANTED EQUAL */
  { "gesdot",           10880}, /* GREATER-THAN OR SLANTED EQUAL TO WITH DOT INSIDE */
  { "gesdoto",          10882}, /* GREATER-THAN OR SLANTED EQUAL TO WITH DOT ABOVE */
  { "gesdotol",         10884}, /* GREATER-THAN OR SLANTED EQUAL TO WITH DOT ABOVE LEFT */
  { "gesles",           10900}, /* GREATER-THAN ABOVE SLANTED EQUAL ABOVE LESS-THAN ABOVE SLANTED EQUAL */
  { "gfr",             120100}, /* MATHEMATICAL FRAKTUR SMALL G */
  { "gg",                8811}, /* MUCH GREATER-THAN */
  { "ggg",               8921}, /* VERY MUCH GREATER-THAN */
  { "ggr",                947}, /* GREEK SMALL LETTER GAMMA */
  { "gimel",             8503}, /* GIMEL SYMBOL */
  { "gjcy",              1107}, /* CYRILLIC SMALL LETTER GJE */
  { "gl",                8823}, /* GREATER-THAN OR LESS-THAN */
  { "glE",              10898}, /* GREATER-THAN ABOVE LESS-THAN ABOVE DOUBLE-LINE EQUAL */
  { "gla",              10917}, /* GREATER-THAN BESIDE LESS-THAN */
  { "glj",              10916}, /* GREATER-THAN OVERLAPPING LESS-THAN */
  { "gnE",               8809}, /* GREATER-THAN BUT NOT EQUAL TO */
  { "gnap",             10890}, /* GREATER-THAN AND NOT APPROXIMATE */
  { "gnapprox",         10890}, /* GREATER-THAN AND NOT APPROXIMATE */
  { "gneq",             10888}, /* GREATER-THAN AND SINGLE-LINE NOT EQUAL TO */
  { "gneqq",             8809}, /* GREATER-THAN BUT NOT EQUAL TO */
  { "gnsim",             8935}, /* GREATER-THAN BUT NOT EQUIVALENT TO */
  { "gopf",            120152}, /* MATHEMATICAL DOUBLE-STRUCK SMALL G */
  { "grave",               96}, /* GRAVE ACCENT */
  { "gscr",              8458}, /* SCRIPT SMALL G */
  { "gsim",              8819}, /* GREATER-THAN OR EQUIVALENT TO */
  { "gsime",            10894}, /* GREATER-THAN ABOVE SIMILAR OR EQUAL */
  { "gsiml",            10896}, /* GREATER-THAN ABOVE SIMILAR ABOVE LESS-THAN */
  { "gt",                  62}, /* GREATER-THAN SIGN */
  { "gtcc",             10919}, /* GREATER-THAN CLOSED BY CURVE */
  { "gtcir",            10874}, /* GREATER-THAN WITH CIRCLE INSIDE */
  { "gtdot",             8919}, /* GREATER-THAN WITH DOT */
  { "gtlPar",           10645}, /* DOUBLE LEFT ARC GREATER-THAN BRACKET */
  { "gtquest",          10876}, /* GREATER-THAN WITH QUESTION MARK ABOVE */
  { "gtrapprox",        10886}, /* GREATER-THAN OR APPROXIMATE */
  { "gtrarr",           10616}, /* GREATER-THAN ABOVE RIGHTWARDS ARROW */
  { "gtrdot",            8919}, /* GREATER-THAN WITH DOT */
  { "gtreqless",         8923}, /* GREATER-THAN EQUAL TO OR LESS-THAN */
  { "gtreqqless",       10892}, /* GREATER-THAN ABOVE DOUBLE-LINE EQUAL ABOVE LESS-THAN */
  { "gtrless",           8823}, /* GREATER-THAN OR LESS-THAN */
  { "gtrsim",            8819}, /* GREATER-THAN OR EQUIVALENT TO */
  { "hArr",              8660}, /* LEFT RIGHT DOUBLE ARROW */
  { "hairsp",            8202}, /* HAIR SPACE */
  { "half",               189}, /* VULGAR FRACTION ONE HALF */
  { "hamilt",            8459}, /* SCRIPT CAPITAL H */
  { "hardcy",            1098}, /* CYRILLIC SMALL LETTER HARD SIGN */
  { "harr",              8596}, /* LEFT RIGHT ARROW */
  { "harrcir",          10568}, /* LEFT RIGHT ARROW THROUGH SMALL CIRCLE */
  { "harrw",             8621}, /* LEFT RIGHT WAVE ARROW */
  { "hbar",              8463}, /* PLANCK CONSTANT OVER TWO PI */
  { "hcirc",              293}, /* LATIN SMALL LETTER H WITH CIRCUMFLEX */
  { "hearts",            9829}, /* BLACK HEART SUIT */
  { "heartsuit",         9829}, /* BLACK HEART SUIT */
  { "hellip",            8230}, /* HORIZONTAL ELLIPSIS */
  { "hercon",            8889}, /* HERMITIAN CONJUGATE MATRIX */
  { "hfr",             120101}, /* MATHEMATICAL FRAKTUR SMALL H */
  { "hksearow",         10533}, /* SOUTH EAST ARROW WITH HOOK */
  { "hkswarow",         10534}, /* SOUTH WEST ARROW WITH HOOK */
  { "hoarr",             8703}, /* LEFT RIGHT OPEN-HEADED ARROW */
  { "homtht",            8763}, /* HOMOTHETIC */
  { "hookleftarrow",     8617}, /* LEFTWARDS ARROW WITH HOOK */
  { "hookrightarrow",    8618}, /* RIGHTWARDS ARROW WITH HOOK */
  { "hopf",            120153}, /* MATHEMATICAL DOUBLE-STRUCK SMALL H */
  { "horbar",            8213}, /* HORIZONTAL BAR */
  { "hscr",            119997}, /* MATHEMATICAL SCRIPT SMALL H */
  { "hslash",            8463}, /* PLANCK CONSTANT OVER TWO PI */
  { "hstrok",             295}, /* LATIN SMALL LETTER H WITH STROKE */
  { "hybull",            8259}, /* HYPHEN BULLET */
  { "hyphen",            8208}, /* HYPHEN */
  { "iacgr",              943}, /* GREEK SMALL LETTER IOTA WITH TONOS */
  { "iacute",             237}, /* LATIN SMALL LETTER I WITH ACUTE */
  { "ic",                8291}, /* INVISIBLE SEPARATOR */
  { "icirc",              238}, /* LATIN SMALL LETTER I WITH CIRCUMFLEX */
  { "icy",               1080}, /* CYRILLIC SMALL LETTER I */
  { "idiagr",             912}, /* GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS */
  { "idigr",              970}, /* GREEK SMALL LETTER IOTA WITH DIALYTIKA */
  { "iecy",              1077}, /* CYRILLIC SMALL LETTER IE */
  { "iexcl",              161}, /* INVERTED EXCLAMATION MARK */
  { "iff",               8660}, /* LEFT RIGHT DOUBLE ARROW */
  { "ifr",             120102}, /* MATHEMATICAL FRAKTUR SMALL I */
  { "igr",                953}, /* GREEK SMALL LETTER IOTA */
  { "igrave",             236}, /* LATIN SMALL LETTER I WITH GRAVE */
  { "ii",                8520}, /* DOUBLE-STRUCK ITALIC SMALL I */
  { "iiiint",           10764}, /* QUADRUPLE INTEGRAL OPERATOR */
  { "iiint",             8749}, /* TRIPLE INTEGRAL */
  { "iinfin",           10716}, /* INCOMPLETE INFINITY */
  { "iiota",             8489}, /* TURNED GREEK SMALL LETTER IOTA */
  { "ijlig",              307}, /* LATIN SMALL LIGATURE IJ */
  { "imacr",              299}, /* LATIN SMALL LETTER I WITH MACRON */
  { "image",             8465}, /* BLACK-LETTER CAPITAL I */
  { "imagline",          8464}, /* SCRIPT CAPITAL I */
  { "imagpart",          8465}, /* BLACK-LETTER CAPITAL I */
  { "imath",              305}, /* LATIN SMALL LETTER DOTLESS I */
  { "imof",              8887}, /* IMAGE OF */
  { "imped",              437}, /* LATIN CAPITAL LETTER Z WITH STROKE */
  { "in",                8712}, /* ELEMENT OF */
  { "incare",            8453}, /* CARE OF */
  { "infin",             8734}, /* INFINITY */
  { "infintie",         10717}, /* TIE OVER INFINITY */
  { "inodot",             305}, /* LATIN SMALL LETTER DOTLESS I */
  { "int",               8747}, /* INTEGRAL */
  { "intcal",            8890}, /* INTERCALATE */
  { "integers",          8484}, /* DOUBLE-STRUCK CAPITAL Z */
  { "intercal",          8890}, /* INTERCALATE */
  { "intlarhk",         10775}, /* INTEGRAL WITH LEFTWARDS ARROW WITH HOOK */
  { "intprod",          10812}, /* INTERIOR PRODUCT */
  { "iocy",              1105}, /* CYRILLIC SMALL LETTER IO */
  { "iogon",              303}, /* LATIN SMALL LETTER I WITH OGONEK */
  { "iopf",            120154}, /* MATHEMATICAL DOUBLE-STRUCK SMALL I */
  { "iota",               953}, /* GREEK SMALL LETTER IOTA */
  { "iprod",            10812}, /* INTERIOR PRODUCT */
  { "iquest",             191}, /* INVERTED QUESTION MARK */
  { "iscr",            119998}, /* MATHEMATICAL SCRIPT SMALL I */
  { "isin",              8712}, /* ELEMENT OF */
  { "isinE",             8953}, /* ELEMENT OF WITH TWO HORIZONTAL STROKES */
  { "isindot",           8949}, /* ELEMENT OF WITH DOT ABOVE */
  { "isins",             8948}, /* SMALL ELEMENT OF WITH VERTICAL BAR AT END OF HORIZONTAL STROKE */
  { "isinsv",            8947}, /* ELEMENT OF WITH VERTICAL BAR AT END OF HORIZONTAL STROKE */
  { "isinv",             8712}, /* ELEMENT OF */
  { "it",                8290}, /* INVISIBLE TIMES */
  { "itilde",             297}, /* LATIN SMALL LETTER I WITH TILDE */
  { "iukcy",             1110}, /* CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I */
  { "iuml",               239}, /* LATIN SMALL LETTER I WITH DIAERESIS */
  { "jcirc",              309}, /* LATIN SMALL LETTER J WITH CIRCUMFLEX */
  { "jcy",               1081}, /* CYRILLIC SMALL LETTER SHORT I */
  { "jfr",             120103}, /* MATHEMATICAL FRAKTUR SMALL J */
  { "jmath",              567}, /* LATIN SMALL LETTER DOTLESS J */
  { "jopf",            120155}, /* MATHEMATICAL DOUBLE-STRUCK SMALL J */
  { "jscr",            119999}, /* MATHEMATICAL SCRIPT SMALL J */
  { "jsercy",            1112}, /* CYRILLIC SMALL LETTER JE */
  { "jukcy",             1108}, /* CYRILLIC SMALL LETTER UKRAINIAN IE */
  { "kappa",              954}, /* GREEK SMALL LETTER KAPPA */
  { "kappav",            1008}, /* GREEK KAPPA SYMBOL */
  { "kcedil",             311}, /* LATIN SMALL LETTER K WITH CEDILLA */
  { "kcy",               1082}, /* CYRILLIC SMALL LETTER KA */
  { "kfr",             120104}, /* MATHEMATICAL FRAKTUR SMALL K */
  { "kgr",                954}, /* GREEK SMALL LETTER KAPPA */
  { "kgreen",             312}, /* LATIN SMALL LETTER KRA */
  { "khcy",              1093}, /* CYRILLIC SMALL LETTER HA */
  { "khgr",               967}, /* GREEK SMALL LETTER CHI */
  { "kjcy",              1116}, /* CYRILLIC SMALL LETTER KJE */
  { "kopf",            120156}, /* MATHEMATICAL DOUBLE-STRUCK SMALL K */
  { "kscr",            120000}, /* MATHEMATICAL SCRIPT SMALL K */
  { "lAarr",             8666}, /* LEFTWARDS TRIPLE ARROW */
  { "lArr",              8656}, /* LEFTWARDS DOUBLE ARROW */
  { "lAtail",           10523}, /* LEFTWARDS DOUBLE ARROW-TAIL */
  { "lBarr",            10510}, /* LEFTWARDS TRIPLE DASH ARROW */
  { "lE",                8806}, /* LESS-THAN OVER EQUAL TO */
  { "lHar",             10594}, /* LEFTWARDS HARPOON WITH BARB UP ABOVE LEFTWARDS HARPOON WITH BARB DOWN */
  { "lacute",             314}, /* LATIN SMALL LETTER L WITH ACUTE */
  { "laemptyv",         10676}, /* EMPTY SET WITH LEFT ARROW ABOVE */
  { "lagran",            8466}, /* SCRIPT CAPITAL L */
  { "lambda",             955}, /* GREEK SMALL LETTER LAMDA */
  { "lang",             10216}, /* MATHEMATICAL LEFT ANGLE BRACKET */
  { "langd",            10641}, /* LEFT ANGLE BRACKET WITH DOT */
  { "langle",           10216}, /* MATHEMATICAL LEFT ANGLE BRACKET */
  { "lap",              10885}, /* LESS-THAN OR APPROXIMATE */
  { "laquo",              171}, /* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
  { "larr",              8592}, /* LEFTWARDS ARROW */
  { "larrb",             8676}, /* LEFTWARDS ARROW TO BAR */
  { "larrbfs",          10527}, /* LEFTWARDS ARROW FROM BAR TO BLACK DIAMOND */
  { "larrfs",           10525}, /* LEFTWARDS ARROW TO BLACK DIAMOND */
  { "larrhk",            8617}, /* LEFTWARDS ARROW WITH HOOK */
  { "larrlp",            8619}, /* LEFTWARDS ARROW WITH LOOP */
  { "larrpl",           10553}, /* LEFT-SIDE ARC ANTICLOCKWISE ARROW */
  { "larrsim",          10611}, /* LEFTWARDS ARROW ABOVE TILDE OPERATOR */
  { "larrtl",            8610}, /* LEFTWARDS ARROW WITH TAIL */
  { "lat",              10923}, /* LARGER THAN */
  { "latail",           10521}, /* LEFTWARDS ARROW-TAIL */
  { "late",             10925}, /* LARGER THAN OR EQUAL TO */
  { "lbarr",            10508}, /* LEFTWARDS DOUBLE DASH ARROW */
  { "lbbrk",            10098}, /* LIGHT LEFT TORTOISE SHELL BRACKET ORNAMENT */
  { "lbrace",             123}, /* LEFT CURLY BRACKET */
  { "lbrack",              91}, /* LEFT SQUARE BRACKET */
  { "lbrke",            10635}, /* LEFT SQUARE BRACKET WITH UNDERBAR */
  { "lbrksld",          10639}, /* LEFT SQUARE BRACKET WITH TICK IN BOTTOM CORNER */
  { "lbrkslu",          10637}, /* LEFT SQUARE BRACKET WITH TICK IN TOP CORNER */
  { "lcaron",             318}, /* LATIN SMALL LETTER L WITH CARON */
  { "lcedil",             316}, /* LATIN SMALL LETTER L WITH CEDILLA */
  { "lceil",             8968}, /* LEFT CEILING */
  { "lcub",               123}, /* LEFT CURLY BRACKET */
  { "lcy",               1083}, /* CYRILLIC SMALL LETTER EL */
  { "ldca",             10550}, /* ARROW POINTING DOWNWARDS THEN CURVING LEFTWARDS */
  { "ldquo",             8220}, /* LEFT DOUBLE QUOTATION MARK */
  { "ldquor",            8222}, /* DOUBLE LOW-9 QUOTATION MARK */
  { "ldrdhar",          10599}, /* LEFTWARDS HARPOON WITH BARB DOWN ABOVE RIGHTWARDS HARPOON WITH BARB DOWN */
  { "ldrushar",         10571}, /* LEFT BARB DOWN RIGHT BARB UP HARPOON */
  { "ldsh",              8626}, /* DOWNWARDS ARROW WITH TIP LEFTWARDS */
  { "le",                8804}, /* LESS-THAN OR EQUAL TO */
  { "leftarrow",         8592}, /* LEFTWARDS ARROW */
  { "leftarrowtail",     8610}, /* LEFTWARDS ARROW WITH TAIL */
  { "leftharpoondown",   8637}, /* LEFTWARDS HARPOON WITH BARB DOWNWARDS */
  { "leftharpoonup",     8636}, /* LEFTWARDS HARPOON WITH BARB UPWARDS */
  { "leftleftarrows",    8647}, /* LEFTWARDS PAIRED ARROWS */
  { "leftrightarrow",    8596}, /* LEFT RIGHT ARROW */
  { "leftrightarrows",   8646}, /* LEFTWARDS ARROW OVER RIGHTWARDS ARROW */
  { "leftrightharpoons",  8651}, /* LEFTWARDS HARPOON OVER RIGHTWARDS HARPOON */
  { "leftrightsquigarrow",  8621}, /* LEFT RIGHT WAVE ARROW */
  { "leftthreetimes",    8907}, /* LEFT SEMIDIRECT PRODUCT */
  { "leg",               8922}, /* LESS-THAN EQUAL TO OR GREATER-THAN */
  { "leq",               8804}, /* LESS-THAN OR EQUAL TO */
  { "leqq",              8806}, /* LESS-THAN OVER EQUAL TO */
  { "leqslant",         10877}, /* LESS-THAN OR SLANTED EQUAL TO */
  { "les",              10877}, /* LESS-THAN OR SLANTED EQUAL TO */
  { "lescc",            10920}, /* LESS-THAN CLOSED BY CURVE ABOVE SLANTED EQUAL */
  { "lesdot",           10879}, /* LESS-THAN OR SLANTED EQUAL TO WITH DOT INSIDE */
  { "lesdoto",          10881}, /* LESS-THAN OR SLANTED EQUAL TO WITH DOT ABOVE */
  { "lesdotor",         10883}, /* LESS-THAN OR SLANTED EQUAL TO WITH DOT ABOVE RIGHT */
  { "lesges",           10899}, /* LESS-THAN ABOVE SLANTED EQUAL ABOVE GREATER-THAN ABOVE SLANTED EQUAL */
  { "lessapprox",       10885}, /* LESS-THAN OR APPROXIMATE */
  { "lessdot",           8918}, /* LESS-THAN WITH DOT */
  { "lesseqgtr",         8922}, /* LESS-THAN EQUAL TO OR GREATER-THAN */
  { "lesseqqgtr",       10891}, /* LESS-THAN ABOVE DOUBLE-LINE EQUAL ABOVE GREATER-THAN */
  { "lessgtr",           8822}, /* LESS-THAN OR GREATER-THAN */
  { "lesssim",           8818}, /* LESS-THAN OR EQUIVALENT TO */
  { "lfisht",           10620}, /* LEFT FISH TAIL */
  { "lfloor",            8970}, /* LEFT FLOOR */
  { "lfr",             120105}, /* MATHEMATICAL FRAKTUR SMALL L */
  { "lg",                8822}, /* LESS-THAN OR GREATER-THAN */
  { "lgE",              10897}, /* LESS-THAN ABOVE GREATER-THAN ABOVE DOUBLE-LINE EQUAL */
  { "lgr",                955}, /* GREEK SMALL LETTER LAMDA */
  { "lhard",             8637}, /* LEFTWARDS HARPOON WITH BARB DOWNWARDS */
  { "lharu",             8636}, /* LEFTWARDS HARPOON WITH BARB UPWARDS */
  { "lharul",           10602}, /* LEFTWARDS HARPOON WITH BARB UP ABOVE LONG DASH */
  { "lhblk",             9604}, /* LOWER HALF BLOCK */
  { "ljcy",              1113}, /* CYRILLIC SMALL LETTER LJE */
  { "ll",                8810}, /* MUCH LESS-THAN */
  { "llarr",             8647}, /* LEFTWARDS PAIRED ARROWS */
  { "llcorner",          8990}, /* BOTTOM LEFT CORNER */
  { "llhard",           10603}, /* LEFTWARDS HARPOON WITH BARB DOWN BELOW LONG DASH */
  { "lltri",             9722}, /* LOWER LEFT TRIANGLE */
  { "lmidot",             320}, /* LATIN SMALL LETTER L WITH MIDDLE DOT */
  { "lmoustache",        9136}, /* UPPER LEFT OR LOWER RIGHT CURLY BRACKET SECTION */
  { "lnE",               8808}, /* LESS-THAN BUT NOT EQUAL TO */
  { "lnap",             10889}, /* LESS-THAN AND NOT APPROXIMATE */
  { "lnapprox",         10889}, /* LESS-THAN AND NOT APPROXIMATE */
  { "lne",              10887}, /* LESS-THAN AND SINGLE-LINE NOT EQUAL TO */
  { "lneq",             10887}, /* LESS-THAN AND SINGLE-LINE NOT EQUAL TO */
  { "lneqq",             8808}, /* LESS-THAN BUT NOT EQUAL TO */
  { "lnsim",             8934}, /* LESS-THAN BUT NOT EQUIVALENT TO */
  { "loang",            10220}, /* MATHEMATICAL LEFT WHITE TORTOISE SHELL BRACKET */
  { "loarr",             8701}, /* LEFTWARDS OPEN-HEADED ARROW */
  { "lobrk",            10214}, /* MATHEMATICAL LEFT WHITE SQUARE BRACKET */
  { "longleftarrow",    10229}, /* LONG LEFTWARDS ARROW */
  { "longleftrightarrow", 10231}, /* LONG LEFT RIGHT ARROW */
  { "longmapsto",       10236}, /* LONG RIGHTWARDS ARROW FROM BAR */
  { "longrightarrow",   10230}, /* LONG RIGHTWARDS ARROW */
  { "looparrowleft",     8619}, /* LEFTWARDS ARROW WITH LOOP */
  { "looparrowright",    8620}, /* RIGHTWARDS ARROW WITH LOOP */
  { "lopar",            10629}, /* LEFT WHITE PARENTHESIS */
  { "lopf",            120157}, /* MATHEMATICAL DOUBLE-STRUCK SMALL L */
  { "loplus",           10797}, /* PLUS SIGN IN LEFT HALF CIRCLE */
  { "lotimes",          10804}, /* MULTIPLICATION SIGN IN LEFT HALF CIRCLE */
  { "lowast",            8727}, /* ASTERISK OPERATOR */
  { "lowbar",              95}, /* LOW LINE */
  { "loz",               9674}, /* LOZENGE */
  { "lozenge",           9674}, /* LOZENGE */
  { "lozf",             10731}, /* BLACK LOZENGE */
  { "lpar",                40}, /* LEFT PARENTHESIS */
  { "lparlt",           10643}, /* LEFT ARC LESS-THAN BRACKET */
  { "lrarr",             8646}, /* LEFTWARDS ARROW OVER RIGHTWARDS ARROW */
  { "lrcorner",          8991}, /* BOTTOM RIGHT CORNER */
  { "lrhard",           10605}, /* RIGHTWARDS HARPOON WITH BARB DOWN BELOW LONG DASH */
  { "lrm",               8206}, /* LEFT-TO-RIGHT MARK */
  { "lrtri",             8895}, /* RIGHT TRIANGLE */
  { "lsaquo",            8249}, /* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
  { "lscr",            120001}, /* MATHEMATICAL SCRIPT SMALL L */
  { "lsh",               8624}, /* UPWARDS ARROW WITH TIP LEFTWARDS */
  { "lsim",              8818}, /* LESS-THAN OR EQUIVALENT TO */
  { "lsime",            10893}, /* LESS-THAN ABOVE SIMILAR OR EQUAL */
  { "lsimg",            10895}, /* LESS-THAN ABOVE SIMILAR ABOVE GREATER-THAN */
  { "lsqb",                91}, /* LEFT SQUARE BRACKET */
  { "lsquo",             8216}, /* LEFT SINGLE QUOTATION MARK */
  { "lsquor",            8218}, /* SINGLE LOW-9 QUOTATION MARK */
  { "lstrok",             322}, /* LATIN SMALL LETTER L WITH STROKE */
  { "lt",                  60}, /* LESS-THAN SIGN */
  { "ltcc",             10918}, /* LESS-THAN CLOSED BY CURVE */
  { "ltcir",            10873}, /* LESS-THAN WITH CIRCLE INSIDE */
  { "ltdot",             8918}, /* LESS-THAN WITH DOT */
  { "lthree",            8907}, /* LEFT SEMIDIRECT PRODUCT */
  { "ltimes",            8905}, /* LEFT NORMAL FACTOR SEMIDIRECT PRODUCT */
  { "ltlarr",           10614}, /* LESS-THAN ABOVE LEFTWARDS ARROW */
  { "ltquest",          10875}, /* LESS-THAN WITH QUESTION MARK ABOVE */
  { "ltrPar",           10646}, /* DOUBLE RIGHT ARC LESS-THAN BRACKET */
  { "ltri",              9667}, /* WHITE LEFT-POINTING SMALL TRIANGLE */
  { "ltrie",             8884}, /* NORMAL SUBGROUP OF OR EQUAL TO */
  { "ltrif",             9666}, /* BLACK LEFT-POINTING SMALL TRIANGLE */
  { "lurdshar",         10570}, /* LEFT BARB UP RIGHT BARB DOWN HARPOON */
  { "luruhar",          10598}, /* LEFTWARDS HARPOON WITH BARB UP ABOVE RIGHTWARDS HARPOON WITH BARB UP */
  { "mDDot",             8762}, /* GEOMETRIC PROPORTION */
  { "macr",               175}, /* MACRON */
  { "male",              9794}, /* MALE SIGN */
  { "malt",             10016}, /* MALTESE CROSS */
  { "maltese",          10016}, /* MALTESE CROSS */
  { "map",               8614}, /* RIGHTWARDS ARROW FROM BAR */
  { "mapsto",            8614}, /* RIGHTWARDS ARROW FROM BAR */
  { "mapstodown",        8615}, /* DOWNWARDS ARROW FROM BAR */
  { "mapstoleft",        8612}, /* LEFTWARDS ARROW FROM BAR */
  { "mapstoup",          8613}, /* UPWARDS ARROW FROM BAR */
  { "marker",            9646}, /* BLACK VERTICAL RECTANGLE */
  { "mcomma",           10793}, /* MINUS SIGN WITH COMMA ABOVE */
  { "mcy",               1084}, /* CYRILLIC SMALL LETTER EM */
  { "mdash",             8212}, /* EM DASH */
  { "measuredangle",     8737}, /* MEASURED ANGLE */
  { "mfr",             120106}, /* MATHEMATICAL FRAKTUR SMALL M */
  { "mgr",                956}, /* GREEK SMALL LETTER MU */
  { "mho",               8487}, /* INVERTED OHM SIGN */
  { "micro",              181}, /* MICRO SIGN */
  { "mid",               8739}, /* DIVIDES */
  { "midast",              42}, /* ASTERISK */
  { "midcir",           10992}, /* VERTICAL LINE WITH CIRCLE BELOW */
  { "middot",             183}, /* MIDDLE DOT */
  { "minus",             8722}, /* MINUS SIGN */
  { "minusb",            8863}, /* SQUARED MINUS */
  { "minusd",            8760}, /* DOT MINUS */
  { "minusdu",          10794}, /* MINUS SIGN WITH DOT BELOW */
  { "mlcp",             10971}, /* TRANSVERSAL INTERSECTION */
  { "mldr",              8230}, /* HORIZONTAL ELLIPSIS */
  { "mnplus",            8723}, /* MINUS-OR-PLUS SIGN */
  { "models",            8871}, /* MODELS */
  { "mopf",            120158}, /* MATHEMATICAL DOUBLE-STRUCK SMALL M */
  { "mp",                8723}, /* MINUS-OR-PLUS SIGN */
  { "mscr",            120002}, /* MATHEMATICAL SCRIPT SMALL M */
  { "mstpos",            8766}, /* INVERTED LAZY S */
  { "mu",                 956}, /* GREEK SMALL LETTER MU */
  { "multimap",          8888}, /* MULTIMAP */
  { "mumap",             8888}, /* MULTIMAP */
  { "nLeftarrow",        8653}, /* LEFTWARDS DOUBLE ARROW WITH STROKE */
  { "nLeftrightarrow",   8654}, /* LEFT RIGHT DOUBLE ARROW WITH STROKE */
  { "nRightarrow",       8655}, /* RIGHTWARDS DOUBLE ARROW WITH STROKE */
  { "nVDash",            8879}, /* NEGATED DOUBLE VERTICAL BAR DOUBLE RIGHT TURNSTILE */
  { "nVdash",            8878}, /* DOES NOT FORCE */
  { "nabla",             8711}, /* NABLA */
  { "nacute",             324}, /* LATIN SMALL LETTER N WITH ACUTE */
  { "nap",               8777}, /* NOT ALMOST EQUAL TO */
  { "napos",              329}, /* LATIN SMALL LETTER N PRECEDED BY APOSTROPHE */
  { "napprox",           8777}, /* NOT ALMOST EQUAL TO */
  { "natur",             9838}, /* MUSIC NATURAL SIGN */
  { "natural",           9838}, /* MUSIC NATURAL SIGN */
  { "naturals",          8469}, /* DOUBLE-STRUCK CAPITAL N */
  { "nbsp",               160}, /* NO-BREAK SPACE */
  { "ncap",             10819}, /* INTERSECTION WITH OVERBAR */
  { "ncaron",             328}, /* LATIN SMALL LETTER N WITH CARON */
  { "ncedil",             326}, /* LATIN SMALL LETTER N WITH CEDILLA */
  { "ncup",             10818}, /* UNION WITH OVERBAR */
  { "ncy",               1085}, /* CYRILLIC SMALL LETTER EN */
  { "ndash",             8211}, /* EN DASH */
  { "ne",                8800}, /* NOT EQUAL TO */
  { "neArr",             8663}, /* NORTH EAST DOUBLE ARROW */
  { "nearhk",           10532}, /* NORTH EAST ARROW WITH HOOK */
  { "nearr",             8599}, /* NORTH EAST ARROW */
  { "nearrow",           8599}, /* NORTH EAST ARROW */
  { "nequiv",            8802}, /* NOT IDENTICAL TO */
  { "nesear",           10536}, /* NORTH EAST ARROW AND SOUTH EAST ARROW */
  { "nexist",            8708}, /* THERE DOES NOT EXIST */
  { "nexists",           8708}, /* THERE DOES NOT EXIST */
  { "nfr",             120107}, /* MATHEMATICAL FRAKTUR SMALL N */
  { "nge",               8817}, /* NEITHER GREATER-THAN NOR EQUAL TO */
  { "ngeq",              8817}, /* NEITHER GREATER-THAN NOR EQUAL TO */
  { "ngr",                957}, /* GREEK SMALL LETTER NU */
  { "ngsim",             8821}, /* NEITHER GREATER-THAN NOR EQUIVALENT TO */
  { "ngt",               8815}, /* NOT GREATER-THAN */
  { "ngtr",              8815}, /* NOT GREATER-THAN */
  { "nhArr",             8654}, /* LEFT RIGHT DOUBLE ARROW WITH STROKE */
  { "nharr",             8622}, /* LEFT RIGHT ARROW WITH STROKE */
  { "nhpar",            10994}, /* PARALLEL WITH HORIZONTAL STROKE */
  { "ni",                8715}, /* CONTAINS AS MEMBER */
  { "nis",               8956}, /* SMALL CONTAINS WITH VERTICAL BAR AT END OF HORIZONTAL STROKE */
  { "nisd",              8954}, /* CONTAINS WITH LONG HORIZONTAL STROKE */
  { "niv",               8715}, /* CONTAINS AS MEMBER */
  { "njcy",              1114}, /* CYRILLIC SMALL LETTER NJE */
  { "nlArr",             8653}, /* LEFTWARDS DOUBLE ARROW WITH STROKE */
  { "nlarr",             8602}, /* LEFTWARDS ARROW WITH STROKE */
  { "nldr",              8229}, /* TWO DOT LEADER */
  { "nle",               8816}, /* NEITHER LESS-THAN NOR EQUAL TO */
  { "nleftarrow",        8602}, /* LEFTWARDS ARROW WITH STROKE */
  { "nleftrightarrow",   8622}, /* LEFT RIGHT ARROW WITH STROKE */
  { "nleq",              8816}, /* NEITHER LESS-THAN NOR EQUAL TO */
  { "nless",             8814}, /* NOT LESS-THAN */
  { "nlsim",             8820}, /* NEITHER LESS-THAN NOR EQUIVALENT TO */
  { "nlt",               8814}, /* NOT LESS-THAN */
  { "nltri",             8938}, /* NOT NORMAL SUBGROUP OF */
  { "nltrie",            8940}, /* NOT NORMAL SUBGROUP OF OR EQUAL TO */
  { "nmid",              8740}, /* DOES NOT DIVIDE */
  { "nopf",            120159}, /* MATHEMATICAL DOUBLE-STRUCK SMALL N */
  { "not",                172}, /* NOT SIGN */
  { "notin",             8713}, /* NOT AN ELEMENT OF */
  { "notinva",           8713}, /* NOT AN ELEMENT OF */
  { "notinvb",           8951}, /* SMALL ELEMENT OF WITH OVERBAR */
  { "notinvc",           8950}, /* ELEMENT OF WITH OVERBAR */
  { "notni",             8716}, /* DOES NOT CONTAIN AS MEMBER */
  { "notniva",           8716}, /* DOES NOT CONTAIN AS MEMBER */
  { "notnivb",           8958}, /* SMALL CONTAINS WITH OVERBAR */
  { "notnivc",           8957}, /* CONTAINS WITH OVERBAR */
  { "npar",              8742}, /* NOT PARALLEL TO */
  { "nparallel",         8742}, /* NOT PARALLEL TO */
  { "npolint",          10772}, /* LINE INTEGRATION NOT INCLUDING THE POLE */
  { "npr",               8832}, /* DOES NOT PRECEDE */
  { "nprcue",            8928}, /* DOES NOT PRECEDE OR EQUAL */
  { "nprec",             8832}, /* DOES NOT PRECEDE */
  { "nrArr",             8655}, /* RIGHTWARDS DOUBLE ARROW WITH STROKE */
  { "nrarr",             8603}, /* RIGHTWARDS ARROW WITH STROKE */
  { "nrightarrow",       8603}, /* RIGHTWARDS ARROW WITH STROKE */
  { "nrtri",             8939}, /* DOES NOT CONTAIN AS NORMAL SUBGROUP */
  { "nsc",               8833}, /* DOES NOT SUCCEED */
  { "nsccue",            8929}, /* DOES NOT SUCCEED OR EQUAL */
  { "nscr",            120003}, /* MATHEMATICAL SCRIPT SMALL N */
  { "nshortmid",         8740}, /* DOES NOT DIVIDE */
  { "nshortparallel",    8742}, /* NOT PARALLEL TO */
  { "nsim",              8769}, /* NOT TILDE */
  { "nsime",             8772}, /* NOT ASYMPTOTICALLY EQUAL TO */
  { "nsimeq",            8772}, /* NOT ASYMPTOTICALLY EQUAL TO */
  { "nsmid",             8740}, /* DOES NOT DIVIDE */
  { "nspar",             8742}, /* NOT PARALLEL TO */
  { "nsqsube",           8930}, /* NOT SQUARE IMAGE OF OR EQUAL TO */
  { "nsqsupe",           8931}, /* NOT SQUARE ORIGINAL OF OR EQUAL TO */
  { "nsub",              8836}, /* NOT A SUBSET OF */
  { "nsube",             8840}, /* NEITHER A SUBSET OF NOR EQUAL TO */
  { "nsubseteq",         8840}, /* NEITHER A SUBSET OF NOR EQUAL TO */
  { "nsucc",             8833}, /* DOES NOT SUCCEED */
  { "nsup",              8837}, /* NOT A SUPERSET OF */
  { "nsupe",             8841}, /* NEITHER A SUPERSET OF NOR EQUAL TO */
  { "nsupseteq",         8841}, /* NEITHER A SUPERSET OF NOR EQUAL TO */
  { "ntgl",              8825}, /* NEITHER GREATER-THAN NOR LESS-THAN */
  { "ntilde",             241}, /* LATIN SMALL LETTER N WITH TILDE */
  { "ntlg",              8824}, /* NEITHER LESS-THAN NOR GREATER-THAN */
  { "ntriangleleft",     8938}, /* NOT NORMAL SUBGROUP OF */
  { "ntrianglelefteq",   8940}, /* NOT NORMAL SUBGROUP OF OR EQUAL TO */
  { "ntriangleright",    8939}, /* DOES NOT CONTAIN AS NORMAL SUBGROUP */
  { "nu",                 957}, /* GREEK SMALL LETTER NU */
  { "num",                 35}, /* NUMBER SIGN */
  { "numero",            8470}, /* NUMERO SIGN */
  { "numsp",             8199}, /* FIGURE SPACE */
  { "nvDash",            8877}, /* NOT TRUE */
  { "nvHarr",           10500}, /* LEFT RIGHT DOUBLE ARROW WITH VERTICAL STROKE */
  { "nvdash",            8876}, /* DOES NOT PROVE */
  { "nvinfin",          10718}, /* INFINITY NEGATED WITH VERTICAL BAR */
  { "nvlArr",           10498}, /* LEFTWARDS DOUBLE ARROW WITH VERTICAL STROKE */
  { "nvrArr",           10499}, /* RIGHTWARDS DOUBLE ARROW WITH VERTICAL STROKE */
  { "nwArr",             8662}, /* NORTH WEST DOUBLE ARROW */
  { "nwarhk",           10531}, /* NORTH WEST ARROW WITH HOOK */
  { "nwarr",             8598}, /* NORTH WEST ARROW */
  { "nwarrow",           8598}, /* NORTH WEST ARROW */
  { "nwnear",           10535}, /* NORTH WEST ARROW AND NORTH EAST ARROW */
  { "oS",                9416}, /* CIRCLED LATIN CAPITAL LETTER S */
  { "oacgr",              972}, /* GREEK SMALL LETTER OMICRON WITH TONOS */
  { "oacute",             243}, /* LATIN SMALL LETTER O WITH ACUTE */
  { "oast",              8859}, /* CIRCLED ASTERISK OPERATOR */
  { "ocir",              8858}, /* CIRCLED RING OPERATOR */
  { "ocirc",              244}, /* LATIN SMALL LETTER O WITH CIRCUMFLEX */
  { "ocy",               1086}, /* CYRILLIC SMALL LETTER O */
  { "odash",             8861}, /* CIRCLED DASH */
  { "odblac",             337}, /* LATIN SMALL LETTER O WITH DOUBLE ACUTE */
  { "odiv",             10808}, /* CIRCLED DIVISION SIGN */
  { "odot",              8857}, /* CIRCLED DOT OPERATOR */
  { "odsold",           10684}, /* CIRCLED ANTICLOCKWISE-ROTATED DIVISION SIGN */
  { "oelig",              339}, /* LATIN SMALL LIGATURE OE */
  { "ofcir",            10687}, /* CIRCLED BULLET */
  { "ofr",             120108}, /* MATHEMATICAL FRAKTUR SMALL O */
  { "ogon",               731}, /* OGONEK */
  { "ogr",                959}, /* GREEK SMALL LETTER OMICRON */
  { "ograve",             242}, /* LATIN SMALL LETTER O WITH GRAVE */
  { "ogt",              10689}, /* CIRCLED GREATER-THAN */
  { "ohacgr",             974}, /* GREEK SMALL LETTER OMEGA WITH TONOS */
  { "ohbar",            10677}, /* CIRCLE WITH HORIZONTAL BAR */
  { "ohgr",               969}, /* GREEK SMALL LETTER OMEGA */
  { "ohm",                937}, /* GREEK CAPITAL LETTER OMEGA */
  { "oint",              8750}, /* CONTOUR INTEGRAL */
  { "olarr",             8634}, /* ANTICLOCKWISE OPEN CIRCLE ARROW */
  { "olcir",            10686}, /* CIRCLED WHITE BULLET */
  { "olcross",          10683}, /* CIRCLE WITH SUPERIMPOSED X */
  { "oline",             8254}, /* OVERLINE */
  { "olt",              10688}, /* CIRCLED LESS-THAN */
  { "omacr",              333}, /* LATIN SMALL LETTER O WITH MACRON */
  { "omega",              969}, /* GREEK SMALL LETTER OMEGA */
  { "omicron",            959}, /* GREEK SMALL LETTER OMICRON */
  { "omid",             10678}, /* CIRCLED VERTICAL BAR */
  { "ominus",            8854}, /* CIRCLED MINUS */
  { "oopf",            120160}, /* MATHEMATICAL DOUBLE-STRUCK SMALL O */
  { "opar",             10679}, /* CIRCLED PARALLEL */
  { "operp",            10681}, /* CIRCLED PERPENDICULAR */
  { "oplus",             8853}, /* CIRCLED PLUS */
  { "or",                8744}, /* LOGICAL OR */
  { "orarr",             8635}, /* CLOCKWISE OPEN CIRCLE ARROW */
  { "ord",              10845}, /* LOGICAL OR WITH HORIZONTAL DASH */
  { "order",             8500}, /* SCRIPT SMALL O */
  { "orderof",           8500}, /* SCRIPT SMALL O */
  { "ordf",               170}, /* FEMININE ORDINAL INDICATOR */
  { "ordm",               186}, /* MASCULINE ORDINAL INDICATOR */
  { "origof",            8886}, /* ORIGINAL OF */
  { "oror",             10838}, /* TWO INTERSECTING LOGICAL OR */
  { "orslope",          10839}, /* SLOPING LARGE OR */
  { "orv",              10843}, /* LOGICAL OR WITH MIDDLE STEM */
  { "oscr",              8500}, /* SCRIPT SMALL O */
  { "oslash",             248}, /* LATIN SMALL LETTER O WITH STROKE */
  { "osol",              8856}, /* CIRCLED DIVISION SLASH */
  { "otilde",             245}, /* LATIN SMALL LETTER O WITH TILDE */
  { "otimes",            8855}, /* CIRCLED TIMES */
  { "otimesas",         10806}, /* CIRCLED MULTIPLICATION SIGN WITH CIRCUMFLEX ACCENT */
  { "ouml",               246}, /* LATIN SMALL LETTER O WITH DIAERESIS */
  { "ovbar",             9021}, /* APL FUNCTIONAL SYMBOL CIRCLE STILE */
  { "par",               8741}, /* PARALLEL TO */
  { "para",               182}, /* PILCROW SIGN */
  { "parallel",          8741}, /* PARALLEL TO */
  { "parsim",           10995}, /* PARALLEL WITH TILDE OPERATOR */
  { "parsl",            11005}, /* DOUBLE SOLIDUS OPERATOR */
  { "part",              8706}, /* PARTIAL DIFFERENTIAL */
  { "pcy",               1087}, /* CYRILLIC SMALL LETTER PE */
  { "percnt",              37}, /* PERCENT SIGN */
  { "period",              46}, /* FULL STOP */
  { "permil",            8240}, /* PER MILLE SIGN */
  { "perp",              8869}, /* UP TACK */
  { "pertenk",           8241}, /* PER TEN THOUSAND SIGN */
  { "pfr",             120109}, /* MATHEMATICAL FRAKTUR SMALL P */
  { "pgr",                960}, /* GREEK SMALL LETTER PI */
  { "phgr",               966}, /* GREEK SMALL LETTER PHI */
  { "phi",                966}, /* GREEK SMALL LETTER PHI */
  { "phiv",               981}, /* GREEK PHI SYMBOL */
  { "phmmat",            8499}, /* SCRIPT CAPITAL M */
  { "phone",             9742}, /* BLACK TELEPHONE */
  { "pi",                 960}, /* GREEK SMALL LETTER PI */
  { "pitchfork",         8916}, /* PITCHFORK */
  { "piv",                982}, /* GREEK PI SYMBOL */
  { "planck",            8463}, /* PLANCK CONSTANT OVER TWO PI */
  { "planckh",           8462}, /* PLANCK CONSTANT */
  { "plankv",            8463}, /* PLANCK CONSTANT OVER TWO PI */
  { "plus",                43}, /* PLUS SIGN */
  { "plusacir",         10787}, /* PLUS SIGN WITH CIRCUMFLEX ACCENT ABOVE */
  { "plusb",             8862}, /* SQUARED PLUS */
  { "pluscir",          10786}, /* PLUS SIGN WITH SMALL CIRCLE ABOVE */
  { "plusdo",            8724}, /* DOT PLUS */
  { "plusdu",           10789}, /* PLUS SIGN WITH DOT BELOW */
  { "pluse",            10866}, /* PLUS SIGN ABOVE EQUALS SIGN */
  { "plusmn",             177}, /* PLUS-MINUS SIGN */
  { "plussim",          10790}, /* PLUS SIGN WITH TILDE BELOW */
  { "plustwo",          10791}, /* PLUS SIGN WITH SUBSCRIPT TWO */
  { "pm",                 177}, /* PLUS-MINUS SIGN */
  { "pointint",         10773}, /* INTEGRAL AROUND A POINT OPERATOR */
  { "popf",            120161}, /* MATHEMATICAL DOUBLE-STRUCK SMALL P */
  { "pound",              163}, /* POUND SIGN */
  { "pr",                8826}, /* PRECEDES */
  { "prE",              10931}, /* PRECEDES ABOVE EQUALS SIGN */
  { "prap",             10935}, /* PRECEDES ABOVE ALMOST EQUAL TO */
  { "prcue",             8828}, /* PRECEDES OR EQUAL TO */
  { "pre",              10927}, /* PRECEDES ABOVE SINGLE-LINE EQUALS SIGN */
  { "prec",              8826}, /* PRECEDES */
  { "precapprox",       10935}, /* PRECEDES ABOVE ALMOST EQUAL TO */
  { "preccurlyeq",       8828}, /* PRECEDES OR EQUAL TO */
  { "preceq",           10927}, /* PRECEDES ABOVE SINGLE-LINE EQUALS SIGN */
  { "precnapprox",      10937}, /* PRECEDES ABOVE NOT ALMOST EQUAL TO */
  { "precneqq",         10933}, /* PRECEDES ABOVE NOT EQUAL TO */
  { "precnsim",          8936}, /* PRECEDES BUT NOT EQUIVALENT TO */
  { "precsim",           8830}, /* PRECEDES OR EQUIVALENT TO */
  { "prime",             8242}, /* PRIME */
  { "primes",            8473}, /* DOUBLE-STRUCK CAPITAL P */
  { "prnE",             10933}, /* PRECEDES ABOVE NOT EQUAL TO */
  { "prnap",            10937}, /* PRECEDES ABOVE NOT ALMOST EQUAL TO */
  { "prnsim",            8936}, /* PRECEDES BUT NOT EQUIVALENT TO */
  { "prod",              8719}, /* N-ARY PRODUCT */
  { "profalar",          9006}, /* ALL AROUND-PROFILE */
  { "profline",          8978}, /* ARC */
  { "profsurf",          8979}, /* SEGMENT */
  { "prop",              8733}, /* PROPORTIONAL TO */
  { "propto",            8733}, /* PROPORTIONAL TO */
  { "prsim",             8830}, /* PRECEDES OR EQUIVALENT TO */
  { "prurel",            8880}, /* PRECEDES UNDER RELATION */
  { "pscr",            120005}, /* MATHEMATICAL SCRIPT SMALL P */
  { "psgr",               968}, /* GREEK SMALL LETTER PSI */
  { "psi",                968}, /* GREEK SMALL LETTER PSI */
  { "puncsp",            8200}, /* PUNCTUATION SPACE */
  { "qfr",             120110}, /* MATHEMATICAL FRAKTUR SMALL Q */
  { "qint",             10764}, /* QUADRUPLE INTEGRAL OPERATOR */
  { "qopf",            120162}, /* MATHEMATICAL DOUBLE-STRUCK SMALL Q */
  { "qprime",            8279}, /* QUADRUPLE PRIME */
  { "qscr",            120006}, /* MATHEMATICAL SCRIPT SMALL Q */
  { "quaternions",       8461}, /* DOUBLE-STRUCK CAPITAL H */
  { "quatint",          10774}, /* QUATERNION INTEGRAL OPERATOR */
  { "quest",               63}, /* QUESTION MARK */
  { "questeq",           8799}, /* QUESTIONED EQUAL TO */
  { "quot",                34}, /* QUOTATION MARK */
  { "rAarr",             8667}, /* RIGHTWARDS TRIPLE ARROW */
  { "rArr",              8658}, /* RIGHTWARDS DOUBLE ARROW */
  { "rAtail",           10524}, /* RIGHTWARDS DOUBLE ARROW-TAIL */
  { "rBarr",            10511}, /* RIGHTWARDS TRIPLE DASH ARROW */
  { "rHar",             10596}, /* RIGHTWARDS HARPOON WITH BARB UP ABOVE RIGHTWARDS HARPOON WITH BARB DOWN */
  { "racute",             341}, /* LATIN SMALL LETTER R WITH ACUTE */
  { "radic",             8730}, /* SQUARE ROOT */
  { "raemptyv",         10675}, /* EMPTY SET WITH RIGHT ARROW ABOVE */
  { "rang",             10217}, /* MATHEMATICAL RIGHT ANGLE BRACKET */
  { "rangd",            10642}, /* RIGHT ANGLE BRACKET WITH DOT */
  { "range",            10661}, /* REVERSED ANGLE WITH UNDERBAR */
  { "rangle",           10217}, /* MATHEMATICAL RIGHT ANGLE BRACKET */
  { "raquo",              187}, /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
  { "rarr",              8594}, /* RIGHTWARDS ARROW */
  { "rarrap",           10613}, /* RIGHTWARDS ARROW ABOVE ALMOST EQUAL TO */
  { "rarrb",             8677}, /* RIGHTWARDS ARROW TO BAR */
  { "rarrbfs",          10528}, /* RIGHTWARDS ARROW FROM BAR TO BLACK DIAMOND */
  { "rarrc",            10547}, /* WAVE ARROW POINTING DIRECTLY RIGHT */
  { "rarrfs",           10526}, /* RIGHTWARDS ARROW TO BLACK DIAMOND */
  { "rarrhk",            8618}, /* RIGHTWARDS ARROW WITH HOOK */
  { "rarrlp",            8620}, /* RIGHTWARDS ARROW WITH LOOP */
  { "rarrpl",           10565}, /* RIGHTWARDS ARROW WITH PLUS BELOW */
  { "rarrsim",          10612}, /* RIGHTWARDS ARROW ABOVE TILDE OPERATOR */
  { "rarrtl",            8611}, /* RIGHTWARDS ARROW WITH TAIL */
  { "rarrw",             8605}, /* RIGHTWARDS WAVE ARROW */
  { "ratail",           10522}, /* RIGHTWARDS ARROW-TAIL */
  { "ratio",             8758}, /* RATIO */
  { "rationals",         8474}, /* DOUBLE-STRUCK CAPITAL Q */
  { "rbarr",            10509}, /* RIGHTWARDS DOUBLE DASH ARROW */
  { "rbbrk",            10099}, /* LIGHT RIGHT TORTOISE SHELL BRACKET ORNAMENT */
  { "rbrace",             125}, /* RIGHT CURLY BRACKET */
  { "rbrack",              93}, /* RIGHT SQUARE BRACKET */
  { "rbrke",            10636}, /* RIGHT SQUARE BRACKET WITH UNDERBAR */
  { "rbrksld",          10638}, /* RIGHT SQUARE BRACKET WITH TICK IN BOTTOM CORNER */
  { "rbrkslu",          10640}, /* RIGHT SQUARE BRACKET WITH TICK IN TOP CORNER */
  { "rcaron",             345}, /* LATIN SMALL LETTER R WITH CARON */
  { "rcedil",             343}, /* LATIN SMALL LETTER R WITH CEDILLA */
  { "rceil",             8969}, /* RIGHT CEILING */
  { "rcub",               125}, /* RIGHT CURLY BRACKET */
  { "rcy",               1088}, /* CYRILLIC SMALL LETTER ER */
  { "rdca",             10551}, /* ARROW POINTING DOWNWARDS THEN CURVING RIGHTWARDS */
  { "rdldhar",          10601}, /* RIGHTWARDS HARPOON WITH BARB DOWN ABOVE LEFTWARDS HARPOON WITH BARB DOWN */
  { "rdquo",             8221}, /* RIGHT DOUBLE QUOTATION MARK */
  { "rdquor",            8221}, /* RIGHT DOUBLE QUOTATION MARK */
  { "rdsh",              8627}, /* DOWNWARDS ARROW WITH TIP RIGHTWARDS */
  { "real",              8476}, /* BLACK-LETTER CAPITAL R */
  { "realine",           8475}, /* SCRIPT CAPITAL R */
  { "realpart",          8476}, /* BLACK-LETTER CAPITAL R */
  { "reals",             8477}, /* DOUBLE-STRUCK CAPITAL R */
  { "rect",              9645}, /* WHITE RECTANGLE */
  { "reg",                174}, /* REGISTERED SIGN */
  { "rfisht",           10621}, /* RIGHT FISH TAIL */
  { "rfloor",            8971}, /* RIGHT FLOOR */
  { "rfr",             120111}, /* MATHEMATICAL FRAKTUR SMALL R */
  { "rgr",                961}, /* GREEK SMALL LETTER RHO */
  { "rhard",             8641}, /* RIGHTWARDS HARPOON WITH BARB DOWNWARDS */
  { "rharu",             8640}, /* RIGHTWARDS HARPOON WITH BARB UPWARDS */
  { "rharul",           10604}, /* RIGHTWARDS HARPOON WITH BARB UP ABOVE LONG DASH */
  { "rho",                961}, /* GREEK SMALL LETTER RHO */
  { "rhov",              1009}, /* GREEK RHO SYMBOL */
  { "rightarrow",        8594}, /* RIGHTWARDS ARROW */
  { "rightarrowtail",    8611}, /* RIGHTWARDS ARROW WITH TAIL */
  { "rightharpoondown",  8641}, /* RIGHTWARDS HARPOON WITH BARB DOWNWARDS */
  { "rightharpoonup",    8640}, /* RIGHTWARDS HARPOON WITH BARB UPWARDS */
  { "rightleftarrows",   8644}, /* RIGHTWARDS ARROW OVER LEFTWARDS ARROW */
  { "rightrightarrows",  8649}, /* RIGHTWARDS PAIRED ARROWS */
  { "rightsquigarrow",   8605}, /* RIGHTWARDS WAVE ARROW */
  { "rightthreetimes",   8908}, /* RIGHT SEMIDIRECT PRODUCT */
  { "ring",               730}, /* RING ABOVE */
  { "risingdotseq",      8787}, /* IMAGE OF OR APPROXIMATELY EQUAL TO */
  { "rlarr",             8644}, /* RIGHTWARDS ARROW OVER LEFTWARDS ARROW */
  { "rlm",               8207}, /* RIGHT-TO-LEFT MARK */
  { "rmoustache",        9137}, /* UPPER RIGHT OR LOWER LEFT CURLY BRACKET SECTION */
  { "rnmid",            10990}, /* DOES NOT DIVIDE WITH REVERSED NEGATION SLASH */
  { "roang",            10221}, /* MATHEMATICAL RIGHT WHITE TORTOISE SHELL BRACKET */
  { "roarr",             8702}, /* RIGHTWARDS OPEN-HEADED ARROW */
  { "ropar",            10630}, /* RIGHT WHITE PARENTHESIS */
  { "ropf",            120163}, /* MATHEMATICAL DOUBLE-STRUCK SMALL R */
  { "roplus",           10798}, /* PLUS SIGN IN RIGHT HALF CIRCLE */
  { "rotimes",          10805}, /* MULTIPLICATION SIGN IN RIGHT HALF CIRCLE */
  { "rpar",                41}, /* RIGHT PARENTHESIS */
  { "rpargt",           10644}, /* RIGHT ARC GREATER-THAN BRACKET */
  { "rppolint",         10770}, /* LINE INTEGRATION WITH RECTANGULAR PATH AROUND POLE */
  { "rrarr",             8649}, /* RIGHTWARDS PAIRED ARROWS */
  { "rsaquo",            8250}, /* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
  { "rscr",            120007}, /* MATHEMATICAL SCRIPT SMALL R */
  { "rsh",               8625}, /* UPWARDS ARROW WITH TIP RIGHTWARDS */
  { "rsqb",                93}, /* RIGHT SQUARE BRACKET */
  { "rsquo",             8217}, /* RIGHT SINGLE QUOTATION MARK */
  { "rsquor",            8217}, /* RIGHT SINGLE QUOTATION MARK */
  { "rthree",            8908}, /* RIGHT SEMIDIRECT PRODUCT */
  { "rtimes",            8906}, /* RIGHT NORMAL FACTOR SEMIDIRECT PRODUCT */
  { "rtri",              9657}, /* WHITE RIGHT-POINTING SMALL TRIANGLE */
  { "rtrif",             9656}, /* BLACK RIGHT-POINTING SMALL TRIANGLE */
  { "rtriltri",         10702}, /* RIGHT TRIANGLE ABOVE LEFT TRIANGLE */
  { "ruluhar",          10600}, /* RIGHTWARDS HARPOON WITH BARB UP ABOVE LEFTWARDS HARPOON WITH BARB UP */
  { "rx",                8478}, /* PRESCRIPTION TAKE */
  { "sacute",             347}, /* LATIN SMALL LETTER S WITH ACUTE */
  { "sbquo",             8218}, /* SINGLE LOW-9 QUOTATION MARK */
  { "sc",                8827}, /* SUCCEEDS */
  { "scE",              10932}, /* SUCCEEDS ABOVE EQUALS SIGN */
  { "scap",             10936}, /* SUCCEEDS ABOVE ALMOST EQUAL TO */
  { "scaron",             353}, /* LATIN SMALL LETTER S WITH CARON */
  { "sccue",             8829}, /* SUCCEEDS OR EQUAL TO */
  { "sce",              10928}, /* SUCCEEDS ABOVE SINGLE-LINE EQUALS SIGN */
  { "scedil",             351}, /* LATIN SMALL LETTER S WITH CEDILLA */
  { "scirc",              349}, /* LATIN SMALL LETTER S WITH CIRCUMFLEX */
  { "scnE",             10934}, /* SUCCEEDS ABOVE NOT EQUAL TO */
  { "scnap",            10938}, /* SUCCEEDS ABOVE NOT ALMOST EQUAL TO */
  { "scnsim",            8937}, /* SUCCEEDS BUT NOT EQUIVALENT TO */
  { "scpolint",         10771}, /* LINE INTEGRATION WITH SEMICIRCULAR PATH AROUND POLE */
  { "scsim",             8831}, /* SUCCEEDS OR EQUIVALENT TO */
  { "scy",               1089}, /* CYRILLIC SMALL LETTER ES */
  { "sdot",              8901}, /* DOT OPERATOR */
  { "sdotb",             8865}, /* SQUARED DOT OPERATOR */
  { "sdote",            10854}, /* EQUALS SIGN WITH DOT BELOW */
  { "seArr",             8664}, /* SOUTH EAST DOUBLE ARROW */
  { "searhk",           10533}, /* SOUTH EAST ARROW WITH HOOK */
  { "searr",             8600}, /* SOUTH EAST ARROW */
  { "searrow",           8600}, /* SOUTH EAST ARROW */
  { "sect",               167}, /* SECTION SIGN */
  { "semi",                59}, /* SEMICOLON */
  { "seswar",           10537}, /* SOUTH EAST ARROW AND SOUTH WEST ARROW */
  { "setminus",          8726}, /* SET MINUS */
  { "setmn",             8726}, /* SET MINUS */
  { "sext",             10038}, /* SIX POINTED BLACK STAR */
  { "sfgr",               962}, /* GREEK SMALL LETTER FINAL SIGMA */
  { "sfr",             120112}, /* MATHEMATICAL FRAKTUR SMALL S */
  { "sfrown",            8994}, /* FROWN */
  { "sgr",                963}, /* GREEK SMALL LETTER SIGMA */
  { "sharp",             9839}, /* MUSIC SHARP SIGN */
  { "shchcy",            1097}, /* CYRILLIC SMALL LETTER SHCHA */
  { "shcy",              1096}, /* CYRILLIC SMALL LETTER SHA */
  { "shortmid",          8739}, /* DIVIDES */
  { "shortparallel",     8741}, /* PARALLEL TO */
  { "shy",                173}, /* SOFT HYPHEN */
  { "sigma",              963}, /* GREEK SMALL LETTER SIGMA */
  { "sigmaf",             962}, /* GREEK SMALL LETTER FINAL SIGMA */
  { "sigmav",             962}, /* GREEK SMALL LETTER FINAL SIGMA */
  { "sim",               8764}, /* TILDE OPERATOR */
  { "simdot",           10858}, /* TILDE OPERATOR WITH DOT ABOVE */
  { "sime",              8771}, /* ASYMPTOTICALLY EQUAL TO */
  { "simeq",             8771}, /* ASYMPTOTICALLY EQUAL TO */
  { "simg",             10910}, /* SIMILAR OR GREATER-THAN */
  { "simgE",            10912}, /* SIMILAR ABOVE GREATER-THAN ABOVE EQUALS SIGN */
  { "siml",             10909}, /* SIMILAR OR LESS-THAN */
  { "simlE",            10911}, /* SIMILAR ABOVE LESS-THAN ABOVE EQUALS SIGN */
  { "simne",             8774}, /* APPROXIMATELY BUT NOT ACTUALLY EQUAL TO */
  { "simplus",          10788}, /* PLUS SIGN WITH TILDE ABOVE */
  { "simrarr",          10610}, /* TILDE OPERATOR ABOVE RIGHTWARDS ARROW */
  { "slarr",             8592}, /* LEFTWARDS ARROW */
  { "smallsetminus",     8726}, /* SET MINUS */
  { "smashp",           10803}, /* SMASH PRODUCT */
  { "smeparsl",         10724}, /* EQUALS SIGN AND SLANTED PARALLEL WITH TILDE ABOVE */
  { "smid",              8739}, /* DIVIDES */
  { "smile",             8995}, /* SMILE */
  { "smt",              10922}, /* SMALLER THAN */
  { "smte",             10924}, /* SMALLER THAN OR EQUAL TO */
  { "softcy",            1100}, /* CYRILLIC SMALL LETTER SOFT SIGN */
  { "sol",                 47}, /* SOLIDUS */
  { "solb",             10692}, /* SQUARED RISING DIAGONAL SLASH */
  { "solbar",            9023}, /* APL FUNCTIONAL SYMBOL SLASH BAR */
  { "sopf",            120164}, /* MATHEMATICAL DOUBLE-STRUCK SMALL S */
  { "spades",            9824}, /* BLACK SPADE SUIT */
  { "spadesuit",         9824}, /* BLACK SPADE SUIT */
  { "spar",              8741}, /* PARALLEL TO */
  { "sqcap",             8851}, /* SQUARE CAP */
  { "sqcup",             8852}, /* SQUARE CUP */
  { "sqsub",             8847}, /* SQUARE IMAGE OF */
  { "sqsube",            8849}, /* SQUARE IMAGE OF OR EQUAL TO */
  { "sqsubset",          8847}, /* SQUARE IMAGE OF */
  { "sqsubseteq",        8849}, /* SQUARE IMAGE OF OR EQUAL TO */
  { "sqsup",             8848}, /* SQUARE ORIGINAL OF */
  { "sqsupe",            8850}, /* SQUARE ORIGINAL OF OR EQUAL TO */
  { "sqsupset",          8848}, /* SQUARE ORIGINAL OF */
  { "sqsupseteq",        8850}, /* SQUARE ORIGINAL OF OR EQUAL TO */
  { "squ",               9633}, /* WHITE SQUARE */
  { "square",            9633}, /* WHITE SQUARE */
  { "squarf",            9642}, /* BLACK SMALL SQUARE */
  { "squf",              9642}, /* BLACK SMALL SQUARE */
  { "srarr",             8594}, /* RIGHTWARDS ARROW */
  { "sscr",            120008}, /* MATHEMATICAL SCRIPT SMALL S */
  { "ssetmn",            8726}, /* SET MINUS */
  { "ssmile",            8995}, /* SMILE */
  { "sstarf",            8902}, /* STAR OPERATOR */
  { "star",              9734}, /* WHITE STAR */
  { "starf",             9733}, /* BLACK STAR */
  { "straightepsilon",   1013}, /* GREEK LUNATE EPSILON SYMBOL */
  { "straightphi",        981}, /* GREEK PHI SYMBOL */
  { "strns",              175}, /* MACRON */
  { "sub",               8834}, /* SUBSET OF */
  { "subE",             10949}, /* SUBSET OF ABOVE EQUALS SIGN */
  { "subdot",           10941}, /* SUBSET WITH DOT */
  { "sube",              8838}, /* SUBSET OF OR EQUAL TO */
  { "subedot",          10947}, /* SUBSET OF OR EQUAL TO WITH DOT ABOVE */
  { "submult",          10945}, /* SUBSET WITH MULTIPLICATION SIGN BELOW */
  { "subnE",            10955}, /* SUBSET OF ABOVE NOT EQUAL TO */
  { "subne",             8842}, /* SUBSET OF WITH NOT EQUAL TO */
  { "subplus",          10943}, /* SUBSET WITH PLUS SIGN BELOW */
  { "subrarr",          10617}, /* SUBSET ABOVE RIGHTWARDS ARROW */
  { "subset",            8834}, /* SUBSET OF */
  { "subseteq",          8838}, /* SUBSET OF OR EQUAL TO */
  { "subseteqq",        10949}, /* SUBSET OF ABOVE EQUALS SIGN */
  { "subsetneq",         8842}, /* SUBSET OF WITH NOT EQUAL TO */
  { "subsetneqq",       10955}, /* SUBSET OF ABOVE NOT EQUAL TO */
  { "subsim",           10951}, /* SUBSET OF ABOVE TILDE OPERATOR */
  { "subsub",           10965}, /* SUBSET ABOVE SUBSET */
  { "subsup",           10963}, /* SUBSET ABOVE SUPERSET */
  { "succ",              8827}, /* SUCCEEDS */
  { "succapprox",       10936}, /* SUCCEEDS ABOVE ALMOST EQUAL TO */
  { "succcurlyeq",       8829}, /* SUCCEEDS OR EQUAL TO */
  { "succeq",           10928}, /* SUCCEEDS ABOVE SINGLE-LINE EQUALS SIGN */
  { "succnapprox",      10938}, /* SUCCEEDS ABOVE NOT ALMOST EQUAL TO */
  { "succneqq",         10934}, /* SUCCEEDS ABOVE NOT EQUAL TO */
  { "succnsim",          8937}, /* SUCCEEDS BUT NOT EQUIVALENT TO */
  { "succsim",           8831}, /* SUCCEEDS OR EQUIVALENT TO */
  { "sum",               8721}, /* N-ARY SUMMATION */
  { "sung",              9834}, /* EIGHTH NOTE */
  { "sup",               8835}, /* SUPERSET OF */
  { "sup1",               185}, /* SUPERSCRIPT ONE */
  { "sup2",               178}, /* SUPERSCRIPT TWO */
  { "sup3",               179}, /* SUPERSCRIPT THREE */
  { "supE",             10950}, /* SUPERSET OF ABOVE EQUALS SIGN */
  { "supdot",           10942}, /* SUPERSET WITH DOT */
  { "supdsub",          10968}, /* SUPERSET BESIDE AND JOINED BY DASH WITH SUBSET */
  { "supe",              8839}, /* SUPERSET OF OR EQUAL TO */
  { "supedot",          10948}, /* SUPERSET OF OR EQUAL TO WITH DOT ABOVE */
  { "suphsol",          10185}, /* SUPERSET PRECEDING SOLIDUS */
  { "suphsub",          10967}, /* SUPERSET BESIDE SUBSET */
  { "suplarr",          10619}, /* SUPERSET ABOVE LEFTWARDS ARROW */
  { "supmult",          10946}, /* SUPERSET WITH MULTIPLICATION SIGN BELOW */
  { "supnE",            10956}, /* SUPERSET OF ABOVE NOT EQUAL TO */
  { "supne",             8843}, /* SUPERSET OF WITH NOT EQUAL TO */
  { "supplus",          10944}, /* SUPERSET WITH PLUS SIGN BELOW */
  { "supset",            8835}, /* SUPERSET OF */
  { "supseteq",          8839}, /* SUPERSET OF OR EQUAL TO */
  { "supseteqq",        10950}, /* SUPERSET OF ABOVE EQUALS SIGN */
  { "supsetneq",         8843}, /* SUPERSET OF WITH NOT EQUAL TO */
  { "supsetneqq",       10956}, /* SUPERSET OF ABOVE NOT EQUAL TO */
  { "supsim",           10952}, /* SUPERSET OF ABOVE TILDE OPERATOR */
  { "supsub",           10964}, /* SUPERSET ABOVE SUBSET */
  { "supsup",           10966}, /* SUPERSET ABOVE SUPERSET */
  { "swArr",             8665}, /* SOUTH WEST DOUBLE ARROW */
  { "swarhk",           10534}, /* SOUTH WEST ARROW WITH HOOK */
  { "swarr",             8601}, /* SOUTH WEST ARROW */
  { "swarrow",           8601}, /* SOUTH WEST ARROW */
  { "swnwar",           10538}, /* SOUTH WEST ARROW AND NORTH WEST ARROW */
  { "szlig",              223}, /* LATIN SMALL LETTER SHARP S */
  { "target",            8982}, /* POSITION INDICATOR */
  { "tau",                964}, /* GREEK SMALL LETTER TAU */
  { "tbrk",              9140}, /* TOP SQUARE BRACKET */
  { "tcaron",             357}, /* LATIN SMALL LETTER T WITH CARON */
  { "tcedil",             355}, /* LATIN SMALL LETTER T WITH CEDILLA */
  { "tcy",               1090}, /* CYRILLIC SMALL LETTER TE */
  { "tdot",              8411}, /* COMBINING THREE DOTS ABOVE */
  { "telrec",            8981}, /* TELEPHONE RECORDER */
  { "tfr",             120113}, /* MATHEMATICAL FRAKTUR SMALL T */
  { "tgr",                964}, /* GREEK SMALL LETTER TAU */
  { "there4",            8756}, /* THEREFORE */
  { "therefore",         8756}, /* THEREFORE */
  { "theta",              952}, /* GREEK SMALL LETTER THETA */
  { "thetasym",           977}, /* GREEK THETA SYMBOL */
  { "thetav",             977}, /* GREEK THETA SYMBOL */
  { "thgr",               952}, /* GREEK SMALL LETTER THETA */
  { "thickapprox",       8776}, /* ALMOST EQUAL TO */
  { "thicksim",          8764}, /* TILDE OPERATOR */
  { "thinsp",            8201}, /* THIN SPACE */
  { "thkap",             8776}, /* ALMOST EQUAL TO */
  { "thksim",            8764}, /* TILDE OPERATOR */
  { "thorn",              254}, /* LATIN SMALL LETTER THORN */
  { "tilde",              732}, /* SMALL TILDE */
  { "times",              215}, /* MULTIPLICATION SIGN */
  { "timesb",            8864}, /* SQUARED TIMES */
  { "timesbar",         10801}, /* MULTIPLICATION SIGN WITH UNDERBAR */
  { "timesd",           10800}, /* MULTIPLICATION SIGN WITH DOT ABOVE */
  { "tint",              8749}, /* TRIPLE INTEGRAL */
  { "toea",             10536}, /* NORTH EAST ARROW AND SOUTH EAST ARROW */
  { "top",               8868}, /* DOWN TACK */
  { "topbot",            9014}, /* APL FUNCTIONAL SYMBOL I-BEAM */
  { "topcir",           10993}, /* DOWN TACK WITH CIRCLE BELOW */
  { "topf",            120165}, /* MATHEMATICAL DOUBLE-STRUCK SMALL T */
  { "topfork",          10970}, /* PITCHFORK WITH TEE TOP */
  { "tosa",             10537}, /* SOUTH EAST ARROW AND SOUTH WEST ARROW */
  { "tprime",            8244}, /* TRIPLE PRIME */
  { "trade",             8482}, /* TRADE MARK SIGN */
  { "triangle",          9653}, /* WHITE UP-POINTING SMALL TRIANGLE */
  { "triangledown",      9663}, /* WHITE DOWN-POINTING SMALL TRIANGLE */
  { "triangleleft",      9667}, /* WHITE LEFT-POINTING SMALL TRIANGLE */
  { "trianglelefteq",    8884}, /* NORMAL SUBGROUP OF OR EQUAL TO */
  { "triangleq",         8796}, /* DELTA EQUAL TO */
  { "triangleright",     9657}, /* WHITE RIGHT-POINTING SMALL TRIANGLE */
  { "tridot",            9708}, /* WHITE UP-POINTING TRIANGLE WITH DOT */
  { "trie",              8796}, /* DELTA EQUAL TO */
  { "triminus",         10810}, /* MINUS SIGN IN TRIANGLE */
  { "triplus",          10809}, /* PLUS SIGN IN TRIANGLE */
  { "trisb",            10701}, /* TRIANGLE WITH SERIFS AT BOTTOM */
  { "tritime",          10811}, /* MULTIPLICATION SIGN IN TRIANGLE */
  { "trpezium",          9186}, /* WHITE TRAPEZIUM */
  { "tscr",            120009}, /* MATHEMATICAL SCRIPT SMALL T */
  { "tscy",              1094}, /* CYRILLIC SMALL LETTER TSE */
  { "tshcy",             1115}, /* CYRILLIC SMALL LETTER TSHE */
  { "tstrok",             359}, /* LATIN SMALL LETTER T WITH STROKE */
  { "twixt",             8812}, /* BETWEEN */
  { "twoheadleftarrow",  8606}, /* LEFTWARDS TWO HEADED ARROW */
  { "twoheadrightarrow",  8608}, /* RIGHTWARDS TWO HEADED ARROW */
  { "uArr",              8657}, /* UPWARDS DOUBLE ARROW */
  { "uHar",             10595}, /* UPWARDS HARPOON WITH BARB LEFT BESIDE UPWARDS HARPOON WITH BARB RIGHT */
  { "uacgr",              973}, /* GREEK SMALL LETTER UPSILON WITH TONOS */
  { "uacute",             250}, /* LATIN SMALL LETTER U WITH ACUTE */
  { "uarr",              8593}, /* UPWARDS ARROW */
  { "ubrcy",             1118}, /* CYRILLIC SMALL LETTER SHORT U */
  { "ubreve",             365}, /* LATIN SMALL LETTER U WITH BREVE */
  { "ucirc",              251}, /* LATIN SMALL LETTER U WITH CIRCUMFLEX */
  { "ucy",               1091}, /* CYRILLIC SMALL LETTER U */
  { "udblac",             369}, /* LATIN SMALL LETTER U WITH DOUBLE ACUTE */
  { "udiagr",             944}, /* GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS */
  { "udigr",              971}, /* GREEK SMALL LETTER UPSILON WITH DIALYTIKA */
  { "ufisht",           10622}, /* UP FISH TAIL */
  { "ufr",             120114}, /* MATHEMATICAL FRAKTUR SMALL U */
  { "ugr",                965}, /* GREEK SMALL LETTER UPSILON */
  { "ugrave",             249}, /* LATIN SMALL LETTER U WITH GRAVE */
  { "uharl",             8639}, /* UPWARDS HARPOON WITH BARB LEFTWARDS */
  { "uharr",             8638}, /* UPWARDS HARPOON WITH BARB RIGHTWARDS */
  { "uhblk",             9600}, /* UPPER HALF BLOCK */
  { "ulcorn",            8988}, /* TOP LEFT CORNER */
  { "ulcorner",          8988}, /* TOP LEFT CORNER */
  { "ulcrop",            8975}, /* TOP LEFT CROP */
  { "ultri",             9720}, /* UPPER LEFT TRIANGLE */
  { "umacr",              363}, /* LATIN SMALL LETTER U WITH MACRON */
  { "uml",                168}, /* DIAERESIS */
  { "uogon",              371}, /* LATIN SMALL LETTER U WITH OGONEK */
  { "uopf",            120166}, /* MATHEMATICAL DOUBLE-STRUCK SMALL U */
  { "uparrow",           8593}, /* UPWARDS ARROW */
  { "updownarrow",       8597}, /* UP DOWN ARROW */
  { "upharpoonleft",     8639}, /* UPWARDS HARPOON WITH BARB LEFTWARDS */
  { "upharpoonright",    8638}, /* UPWARDS HARPOON WITH BARB RIGHTWARDS */
  { "uplus",             8846}, /* MULTISET UNION */
  { "upsi",               965}, /* GREEK SMALL LETTER UPSILON */
  { "upsih",              978}, /* GREEK UPSILON WITH HOOK SYMBOL */
  { "upsilon",            965}, /* GREEK SMALL LETTER UPSILON */
  { "upuparrows",        8648}, /* UPWARDS PAIRED ARROWS */
  { "urcorn",            8989}, /* TOP RIGHT CORNER */
  { "urcorner",          8989}, /* TOP RIGHT CORNER */
  { "urcrop",            8974}, /* TOP RIGHT CROP */
  { "uring",              367}, /* LATIN SMALL LETTER U WITH RING ABOVE */
  { "urtri",             9721}, /* UPPER RIGHT TRIANGLE */
  { "uscr",            120010}, /* MATHEMATICAL SCRIPT SMALL U */
  { "utdot",             8944}, /* UP RIGHT DIAGONAL ELLIPSIS */
  { "utilde",             361}, /* LATIN SMALL LETTER U WITH TILDE */
  { "utri",              9653}, /* WHITE UP-POINTING SMALL TRIANGLE */
  { "utrif",             9652}, /* BLACK UP-POINTING SMALL TRIANGLE */
  { "uuarr",             8648}, /* UPWARDS PAIRED ARROWS */
  { "uuml",               252}, /* LATIN SMALL LETTER U WITH DIAERESIS */
  { "uwangle",          10663}, /* OBLIQUE ANGLE OPENING DOWN */
  { "vArr",              8661}, /* UP DOWN DOUBLE ARROW */
  { "vBar",             10984}, /* SHORT UP TACK WITH UNDERBAR */
  { "vBarv",            10985}, /* SHORT UP TACK ABOVE SHORT DOWN TACK */
  { "vDash",             8872}, /* TRUE */
  { "vangrt",           10652}, /* RIGHT ANGLE VARIANT WITH SQUARE */
  { "varepsilon",        1013}, /* GREEK LUNATE EPSILON SYMBOL */
  { "varkappa",          1008}, /* GREEK KAPPA SYMBOL */
  { "varnothing",        8709}, /* EMPTY SET */
  { "varphi",             981}, /* GREEK PHI SYMBOL */
  { "varpi",              982}, /* GREEK PI SYMBOL */
  { "varpropto",         8733}, /* PROPORTIONAL TO */
  { "varr",              8597}, /* UP DOWN ARROW */
  { "varrho",            1009}, /* GREEK RHO SYMBOL */
  { "varsigma",           962}, /* GREEK SMALL LETTER FINAL SIGMA */
  { "vartheta",           977}, /* GREEK THETA SYMBOL */
  { "vartriangleleft",   8882}, /* NORMAL SUBGROUP OF */
  { "vartriangleright",  8883}, /* CONTAINS AS NORMAL SUBGROUP */
  { "vcy",               1074}, /* CYRILLIC SMALL LETTER VE */
  { "vdash",             8866}, /* RIGHT TACK */
  { "vee",               8744}, /* LOGICAL OR */
  { "veebar",            8891}, /* XOR */
  { "veeeq",             8794}, /* EQUIANGULAR TO */
  { "vellip",            8942}, /* VERTICAL ELLIPSIS */
  { "verbar",             124}, /* VERTICAL LINE */
  { "vert",               124}, /* VERTICAL LINE */
  { "vfr",             120115}, /* MATHEMATICAL FRAKTUR SMALL V */
  { "vltri",             8882}, /* NORMAL SUBGROUP OF */
  { "vopf",            120167}, /* MATHEMATICAL DOUBLE-STRUCK SMALL V */
  { "vprop",             8733}, /* PROPORTIONAL TO */
  { "vrtri",             8883}, /* CONTAINS AS NORMAL SUBGROUP */
  { "vscr",            120011}, /* MATHEMATICAL SCRIPT SMALL V */
  { "vzigzag",          10650}, /* VERTICAL ZIGZAG LINE */
  { "wcirc",              373}, /* LATIN SMALL LETTER W WITH CIRCUMFLEX */
  { "wedbar",           10847}, /* LOGICAL AND WITH UNDERBAR */
  { "wedge",             8743}, /* LOGICAL AND */
  { "wedgeq",            8793}, /* ESTIMATES */
  { "weierp",            8472}, /* SCRIPT CAPITAL P */
  { "wfr",             120116}, /* MATHEMATICAL FRAKTUR SMALL W */
  { "wopf",            120168}, /* MATHEMATICAL DOUBLE-STRUCK SMALL W */
  { "wp",                8472}, /* SCRIPT CAPITAL P */
  { "wr",                8768}, /* WREATH PRODUCT */
  { "wreath",            8768}, /* WREATH PRODUCT */
  { "wscr",            120012}, /* MATHEMATICAL SCRIPT SMALL W */
  { "xcap",              8898}, /* N-ARY INTERSECTION */
  { "xcirc",             9711}, /* LARGE CIRCLE */
  { "xcup",              8899}, /* N-ARY UNION */
  { "xdtri",             9661}, /* WHITE DOWN-POINTING TRIANGLE */
  { "xfr",             120117}, /* MATHEMATICAL FRAKTUR SMALL X */
  { "xgr",                958}, /* GREEK SMALL LETTER XI */
  { "xhArr",            10234}, /* LONG LEFT RIGHT DOUBLE ARROW */
  { "xharr",            10231}, /* LONG LEFT RIGHT ARROW */
  { "xi",                 958}, /* GREEK SMALL LETTER XI */
  { "xlArr",            10232}, /* LONG LEFTWARDS DOUBLE ARROW */
  { "xlarr",            10229}, /* LONG LEFTWARDS ARROW */
  { "xmap",             10236}, /* LONG RIGHTWARDS ARROW FROM BAR */
  { "xnis",              8955}, /* CONTAINS WITH VERTICAL BAR AT END OF HORIZONTAL STROKE */
  { "xodot",            10752}, /* N-ARY CIRCLED DOT OPERATOR */
  { "xopf",            120169}, /* MATHEMATICAL DOUBLE-STRUCK SMALL X */
  { "xoplus",           10753}, /* N-ARY CIRCLED PLUS OPERATOR */
  { "xotime",           10754}, /* N-ARY CIRCLED TIMES OPERATOR */
  { "xrArr",            10233}, /* LONG RIGHTWARDS DOUBLE ARROW */
  { "xrarr",            10230}, /* LONG RIGHTWARDS ARROW */
  { "xscr",            120013}, /* MATHEMATICAL SCRIPT SMALL X */
  { "xsqcup",           10758}, /* N-ARY SQUARE UNION OPERATOR */
  { "xuplus",           10756}, /* N-ARY UNION OPERATOR WITH PLUS */
  { "xutri",             9651}, /* WHITE UP-POINTING TRIANGLE */
  { "xvee",              8897}, /* N-ARY LOGICAL OR */
  { "xwedge",            8896}, /* N-ARY LOGICAL AND */
  { "yacute",             253}, /* LATIN SMALL LETTER Y WITH ACUTE */
  { "yacy",              1103}, /* CYRILLIC SMALL LETTER YA */
  { "ycirc",              375}, /* LATIN SMALL LETTER Y WITH CIRCUMFLEX */
  { "ycy",               1099}, /* CYRILLIC SMALL LETTER YERU */
  { "yen",                165}, /* YEN SIGN */
  { "yfr",             120118}, /* MATHEMATICAL FRAKTUR SMALL Y */
  { "yicy",              1111}, /* CYRILLIC SMALL LETTER YI */
  { "yopf",            120170}, /* MATHEMATICAL DOUBLE-STRUCK SMALL Y */
  { "yscr",            120014}, /* MATHEMATICAL SCRIPT SMALL Y */
  { "yucy",              1102}, /* CYRILLIC SMALL LETTER YU */
  { "yuml",               255}, /* LATIN SMALL LETTER Y WITH DIAERESIS */
  { "zacute",             378}, /* LATIN SMALL LETTER Z WITH ACUTE */
  { "zcaron",             382}, /* LATIN SMALL LETTER Z WITH CARON */
  { "zcy",               1079}, /* CYRILLIC SMALL LETTER ZE */
  { "zdot",               380}, /* LATIN SMALL LETTER Z WITH DOT ABOVE */
  { "zeetrf",            8488}, /* BLACK-LETTER CAPITAL Z */
  { "zeta",               950}, /* GREEK SMALL LETTER ZETA */
  { "zfr",             120119}, /* MATHEMATICAL FRAKTUR SMALL Z */
  { "zgr",                950}, /* GREEK SMALL LETTER ZETA */
  { "zhcy",              1078}, /* CYRILLIC SMALL LETTER ZHE */
  { "zigrarr",           8669}, /* RIGHTWARDS SQUIGGLE ARROW */
  { "zopf",            120171}, /* MATHEMATICAL DOUBLE-STRUCK SMALL Z */
  { "zscr",            120015}, /* MATHEMATICAL SCRIPT SMALL Z */
  { "zwj",               8205}, /* ZERO WIDTH JOINER */
  { "zwnj",              8204}, /* ZERO WIDTH NON-JOINER */
#elif defined(ENTITIES_HTML40_ONLY)
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
  {"sum",       8721}, /* n-ary summation, U+2211 ISOamsb */
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
#endif /* not ENTITIES_HTML40_ONLY */
};

/* *INDENT-ON* */
