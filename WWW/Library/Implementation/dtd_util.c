/*
 * $LynxId: dtd_util.c,v 1.10 2008/07/05 00:32:17 tom Exp $
 *
 * Given a SGML_dtd structure, write a corresponding flat file, or "C" source.
 * Given the flat-file, write the "C" source.
 */

#include <HTUtils.h>
#include <HTMLDTD.h>

/*
 * Tweaks to build standalone.
 */
#undef exit

BOOLEAN WWW_TraceFlag = FALSE;
FILE *TraceFP(void)
{
    return stderr;
}

/*
 * Begin the actual utility.
 */
#define GETOPT "flo:ts"

static void failed(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

static void usage(void)
{
    static const char *tbl[] =
    {
	"Usage: dtd_util [options]",
	"",
	"Options:",
	"  -o filename",
	"  -l load",
	"  -f flat file",
	"  -t tagsoup",
	"  -s strict"
    };
    unsigned n;

    for (n = 0; n < TABLESIZE(tbl); ++n) {
	fprintf(stderr, "%s\n", tbl[n]);
    }
    exit(EXIT_FAILURE);
}

static const char *SGMLContent2s(SGMLContent contents)
{
    char *value = "?";

    switch (contents) {
    case SGML_EMPTY:
	value = "SGML_EMPTY";
	break;
    case SGML_LITTERAL:
	value = "SGML_LITTERAL";
	break;
    case SGML_CDATA:
	value = "SGML_CDATA";
	break;
    case SGML_SCRIPT:
	value = "SGML_SCRIPT";
	break;
    case SGML_RCDATA:
	value = "SGML_RCDATA";
	break;
    case SGML_MIXED:
	value = "SGML_MIXED";
	break;
    case SGML_ELEMENT:
	value = "SGML_ELEMENT";
	break;
    case SGML_PCDATA:
	value = "SGML_PCDATA";
	break;
    }
    return value;
}

static void PrintF(FILE *, int, const char *,...) GCC_PRINTFLIKE(3, 4);

static void PrintF(FILE *output, int width, const char *fmt,...)
{
    char buffer[BUFSIZ];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);

    fprintf(output, "%-*s", width, buffer);
}

static int first_attrs(const SGML_dtd * dtd, int which)
{
    int check;
    int result = TRUE;

    for (check = 0; check < which; ++check) {
	if (dtd->tags[check].attributes == dtd->tags[which].attributes) {
	    result = FALSE;
	    break;
	}
    }
    return result;
}

static const char *NameOfAttrs(const SGML_dtd * dtd, int which)
{
    int check;
    const char *result = dtd->tags[which].name;

    for (check = 0; check < which; ++check) {
	if (dtd->tags[check].attributes == dtd->tags[which].attributes) {
	    result = dtd->tags[check].name;
	    break;
	}
    }
    /* special cases to match existing headers */
    if (!strcmp(result, "ABBR"))
	result = "GEN";
    else if (!strcmp(result, "BLOCKQUOTE"))
	result = "BQ";
    else if (!strcmp(result, "BASEFONT"))
	result = "FONT";
    else if (!strcmp(result, "CENTER"))
	result = "DIV";
    else if (!strcmp(result, "DIR"))
	result = "UL";
    else if (!strcmp(result, "H1"))
	result = "H";
    else if (!strcmp(result, "TBODY"))
	result = "TR";
    return result;
}

static const char *XXX_attr(const SGML_dtd * dtd, int which)
{
    static char result[80];
    int i;

    strcpy(result, NameOfAttrs(dtd, which));
    for (i = 0; result[i]; ++i)
	result[i] = tolower(result[i]);
    return result;
}

static void dump_src_HTTag_Defines(FILE *output, HTTag * tag)
{
#define myFMT "0x%05X"
    fprintf(output,
	    "#define T_%-10s "
	    myFMT "," myFMT "," myFMT "," myFMT "," myFMT "," myFMT
	    ", " myFMT "\n",
	    tag->name,
	    tag->tagclass,
	    tag->contains,
	    tag->icontains,
	    tag->contained,
	    tag->icontained,
	    tag->canclose,
	    tag->flags);
}

static void dump_src_HTTag_Attrs(FILE *output, const SGML_dtd * dtd, int which)
{
    HTTag *tag = &(dtd->tags[which]);
    char buffer[BUFSIZ];
    char pretty = 'N';
    int n;

    sprintf(buffer, "static attr %s_attr[] = {", XXX_attr(dtd, which));
    fprintf(output,
	    "%-40s/* %s attributes */\n", buffer, tag->name);
    for (n = 0; n < tag->number_of_attributes; ++n) {
	sprintf(buffer, "\"%s\"", tag->attributes[n].name);
#ifdef USE_PRETTYSRC
	switch (tag->attributes[n].type) {
	case HTMLA_NORMAL:
	    pretty = 'N';
	    break;
	case HTMLA_ANAME:
	    pretty = 'i';
	    break;
	case HTMLA_HREF:
	    pretty = 'h';
	    break;
	case HTMLA_CLASS:
	    pretty = 'c';
	    break;
	case HTMLA_AUXCLASS:
	    pretty = 'x';
	    break;
	}
#endif
	fprintf(output, "\t{ %-15s T(%c) },\n", buffer, pretty);
    }
    fprintf(output, "\t{ 0               T(N) }\t/* Terminate list */\n");
    fprintf(output, "};\n");
    fprintf(output, "\n");
}

static void dump_src_HTTag(FILE *output, const SGML_dtd * dtd, int which)
{
    HTTag *tag = &(dtd->tags[which]);
    char *P_macro = "P";

#ifdef EXP_JUSTIFY_ELTS
    if (!tag->can_justify)
	P_macro = "P0";
#endif
    PrintF(output, 19, " { %s(\"%s\"),", P_macro, tag->name);
    PrintF(output, 16, "%s_attr,", XXX_attr(dtd, which));
    PrintF(output, 28, "HTML_%s_ATTRIBUTES,", NameOfAttrs(dtd, which));
    PrintF(output, 14, "%s,", SGMLContent2s(tag->contents));
    fprintf(output, "T_%s", tag->name);
    fprintf(output, "},\n");
}

static void dump_source(FILE *output, const SGML_dtd * dtd)
{
    int j;

    /* construct TagClass-define's */
    fprintf(output, "/*\n vile:cmode\n   %d tags\n */\n", dtd->number_of_tags);
    for (j = 0; j < dtd->number_of_tags; ++j) {
	dump_src_HTTag_Defines(output, &(dtd->tags[j]));
    }
    fprintf(output, "\n");

    /* construct attribute-tables */
    for (j = 0; j < dtd->number_of_tags; ++j) {
	if (first_attrs(dtd, j))
	    dump_src_HTTag_Attrs(output, dtd, j);
    }
    /* construct the tags table */
    fprintf(output, "static const HTTag tags_table[HTML_ALL_ELEMENTS] = {\n");
    for (j = 0; j < dtd->number_of_tags; ++j) {
	dump_src_HTTag(output, dtd, j);
    }
    fprintf(output, "};\n\n");
}

static void dump_flat_attrs(FILE *output, const char *name, attr * attributes, int number_of_attributes)
{
    int n;

    fprintf(output, "\t\t%d %s:\n", number_of_attributes, name);
    for (n = 0; n < number_of_attributes; ++n) {
	fprintf(output, "\t\t\t%d:%s\n", n, attributes[n].name);
    }
}

static void dump_flat_SGMLContent(FILE *output, const char *name, SGMLContent contents)
{
    fprintf(output, "\t\t%s: %s\n", name, SGMLContent2s(contents));
}

#define DUMP(name) \
	if (theClass & Tgc_##name) {\
	    fprintf(output, " " #name); \
	    theClass &= ~(Tgc_##name); \
	}

static void dump_flat_TagClass(FILE *output, const char *name, TagClass theClass)
{
    fprintf(output, "\t\t%s:", name);
    DUMP(FONTlike);
    DUMP(EMlike);
    DUMP(MATHlike);
    DUMP(Alike);
    DUMP(formula);
    DUMP(TRlike);
    DUMP(SELECTlike);
    DUMP(FORMlike);
    DUMP(Plike);
    DUMP(DIVlike);
    DUMP(LIlike);
    DUMP(ULlike);
    DUMP(BRlike);
    DUMP(APPLETlike);
    DUMP(HRlike);
    DUMP(MAPlike);
    DUMP(outer);
    DUMP(BODYlike);
    DUMP(HEADstuff);
    DUMP(same);
    if (theClass)
	fprintf(output, " OOPS:%#x", theClass);
    fprintf(output, "\n");
}

#undef DUMP

#define DUMP(name) \
	if (theFlags & Tgf_##name) {\
	    fprintf(output, " " #name); \
	    theFlags &= ~(Tgf_##name); \
	}

static void dump_flat_TagFlags(FILE *output, const char *name, TagFlags theFlags)
{
    fprintf(output, "\t\t%s:", name);
    DUMP(endO);
    DUMP(startO);
    DUMP(mafse);
    DUMP(strict);
    DUMP(nreie);
    DUMP(frecyc);
    DUMP(nolyspcl);
    if (theFlags)
	fprintf(output, " OOPS:%#x", theFlags);
    fprintf(output, "\n");
}

#undef DUMP

static void dump_flat_HTTag(FILE *output, unsigned n, HTTag * tag)
{
    fprintf(output, "\t%u:%s\n", n, tag->name);
#ifdef EXP_JUSTIFY_ELTS
    fprintf(output, "\t\t%s\n", tag->can_justify ? "justify" : "nojustify");
#endif
    dump_flat_attrs(output, "attributes", tag->attributes, tag->number_of_attributes);
    dump_flat_SGMLContent(output, "contents", tag->contents);
    dump_flat_TagClass(output, "tagclass", tag->tagclass);
    dump_flat_TagClass(output, "contains", tag->contains);
    dump_flat_TagClass(output, "icontains", tag->icontains);
    dump_flat_TagClass(output, "contained", tag->contained);
    dump_flat_TagClass(output, "icontained", tag->icontained);
    dump_flat_TagClass(output, "canclose", tag->canclose);
    dump_flat_TagFlags(output, "flags", tag->flags);
}

static void dump_flatfile(FILE *output, const SGML_dtd * dtd)
{
    unsigned n;

    fprintf(output, "%d tags\n", dtd->number_of_tags);
    for (n = 0; (int) n < dtd->number_of_tags; ++n) {
	dump_flat_HTTag(output, n, &(dtd->tags[n]));
    }
#if 0
    fprintf(output, "%d entities\n", dtd->number_of_entities);
    for (n = 0; n < dtd->number_of_entities; ++n) {
    }
#endif
}

static void load_flatfile(FILE *input, const SGML_dtd * dtd)
{
    (void) input;
    (void) dtd;
}

int main(int argc, char *argv[])
{
    const SGML_dtd *the_dtd = &HTML_dtd;
    int ch;
    int f_option = FALSE;
    int l_option = FALSE;
    FILE *input = stdin;
    FILE *output = stdout;

    HTSwitchDTD(FALSE);
    while ((ch = getopt(argc, argv, GETOPT)) != -1) {
	switch (ch) {
	case 'f':
	    f_option = TRUE;
	    break;
	case 'l':
	    l_option = TRUE;
	    break;
	case 'o':
	    output = fopen(optarg, "w");
	    if (output == 0)
		failed(optarg);
	    break;
	case 't':
	    HTSwitchDTD(TRUE);
	    break;
	case 's':
	    HTSwitchDTD(FALSE);
	    break;
	default:
	    usage();
	}
    }
    if (l_option)
	load_flatfile(input, the_dtd);
    if (f_option)
	dump_flatfile(output, the_dtd);
    else
	dump_source(output, the_dtd);
    return EXIT_SUCCESS;
}
