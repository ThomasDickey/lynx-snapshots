/*
 * $LynxId: dtd_util.c,v 1.26 2008/07/07 00:03:09 tom Exp $
 *
 * Given a SGML_dtd structure, write a corresponding flat file, or "C" source.
 * Given the flat-file, write the "C" source.
 *
 * TODO: read flat-file
 */

#include <HTUtils.h>
#include <HTMLDTD.h>
#include <string.h>

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
#define GETOPT "chlo:ts"

#define NOTE(message) fprintf(output, message "\n");

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
	"  -c           generate C-source"
	"  -h           generate C-header"
	"  -l           load",
	"  -o filename  specify output (default: stdout)",
	"  -s           strict (HTML DTD 0)",
	"  -t           tagsoup (HTML DTD 1)",
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

static char *no_dashes(char *target, const char *source)
{
    int j;

    for (j = 0; (target[j] = source[j]) != '\0'; ++j) {
	if (!isalnum(target[j]))
	    target[j] = '_';
    }
    return target;
}

/* the second "OBJECT" is treated specially */
static int first_object(const SGML_dtd * dtd, int which)
{
    int check;

    for (check = 0; check <= which; ++check) {
	if (!strcmp(dtd->tags[check].name, "OBJECT"))
	    break;
    }
    return (check == which);
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

static const char *DEF_name(const SGML_dtd * dtd, int which)
{
    const char *result = dtd->tags[which].name;

    if (!strcmp(result, "OBJECT") && !first_object(dtd, which))
	result = "OBJECT_PCDATA";
    return result;
}

typedef struct {
    const char *name;
    attr *attrs;
    int count;
    int which;
} AttrInfo;

static int compare_attr(const void *a, const void *b)
{
    const AttrInfo *p = (const AttrInfo *) a;
    const AttrInfo *q = (const AttrInfo *) b;

    return strcmp(p->name, q->name);
}

static AttrInfo *sorted_attrs(const SGML_dtd * dtd, unsigned *countp, int lower)
{
    int j;

    AttrInfo *data = (AttrInfo *) calloc(dtd->number_of_tags, sizeof(AttrInfo));
    unsigned count = 0;

    /* get the attribute-data */
    for (j = 0; j < dtd->number_of_tags; ++j) {
	if (first_attrs(dtd, j)) {
	    if (lower)
		data[count].name = strdup(XXX_attr(dtd, j));
	    else
		data[count].name = NameOfAttrs(dtd, j);
	    data[count].attrs = dtd->tags[j].attributes;
	    data[count].count = dtd->tags[j].number_of_attributes;
	    data[count].which = j;
	    ++count;
	}
    }
    /* sort the data by the name of their associated tag */
    qsort(data, count, sizeof(*data), compare_attr);
    *countp = count;
    return data;
}

static void dump_src_HTTag_Defines(FILE *output, const SGML_dtd * dtd, int which)
{
    HTTag *tag = &(dtd->tags[which]);

#define myFMT "0x%05X"
    fprintf(output,
	    "#define T_%-13s "
	    myFMT "," myFMT "," myFMT "," myFMT "," myFMT "," myFMT
	    "," myFMT "\n",
	    DEF_name(dtd, which),
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
    NOTE("");
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
    fprintf(output, "T_%s", DEF_name(dtd, which));
    fprintf(output, "},\n");
}

static void dump_source(FILE *output, const SGML_dtd * dtd, int dtd_version)
{
    const char *marker = "src_HTMLDTD_H";
    int j;

    unsigned count = 0;
    AttrInfo *data = sorted_attrs(dtd, &count, 1);

    fprintf(output, "/* %cLynxId%c */\n", '$', '$');
    fprintf(output, "#ifndef %s%d\n", marker, dtd_version);
    fprintf(output, "#define %s%d 1\n\n", marker, dtd_version);

    /*
     * If we ifdef this for once, and make the table names distinct, we can
     * #include the strict- and tagsoup-output directly in HTMLDTD.c
     */
    NOTE("#ifndef once_HTMLDTD");
    NOTE("#define once_HTMLDTD 1");
    NOTE("");

    /* construct TagClass-define's */
    for (j = 0; j <= dtd->number_of_tags; ++j) {
	dump_src_HTTag_Defines(output, dtd, j);
    }
    NOTE("#define T__UNREC_	0x00000,0x00000,0x00000,0x00000,0x00000,0x00000,0x00000");

    /* construct attribute-tables */
    NOTE("#ifdef USE_PRETTYSRC");
    NOTE("# define N HTMLA_NORMAL");
    NOTE("# define i HTMLA_ANAME");
    NOTE("# define h HTMLA_HREF");
    NOTE("# define c HTMLA_CLASS");
    NOTE("# define x HTMLA_AUXCLASS");
    NOTE("# define T(t) , t");
    NOTE("#else");
    NOTE("# define T(t)			/*nothing */");
    NOTE("#endif");
    NOTE("/* *INDENT-OFF* */");
    NOTE("");
    for (j = 0; j < (int) count; ++j) {
	dump_src_HTTag_Attrs(output, dtd, data[j].which);
    }
    NOTE("/* *INDENT-ON* */");
    NOTE("");
    NOTE("#undef N");
    NOTE("#undef i");
    NOTE("#undef h");
    NOTE("#undef c");
    NOTE("#undef x");
    NOTE("");
    NOTE("#undef T");

    NOTE("");
    NOTE("/* these definitions are used in the tags-tables */");
    NOTE("#undef P");
    NOTE("#undef P_");
    NOTE("#ifdef USE_COLOR_STYLE");
    NOTE("#define P_(x) x , (sizeof x) -1");
    NOTE("#define NULL_HTTag_ NULL, 0");
    NOTE("#else");
    NOTE("#define P_(x) x");
    NOTE("#define NULL_HTTag_ NULL");
    NOTE("#endif");
    NOTE("");
    NOTE("#ifdef EXP_JUSTIFY_ELTS");
    NOTE("#define P(x) P_(x), 1");
    NOTE("#define P0(x) P_(x), 0");
    NOTE("#define NULL_HTTag NULL_HTTag_,0");
    NOTE("#else");
    NOTE("#define P(x) P_(x)");
    NOTE("#define P0(x) P_(x)");
    NOTE("#define NULL_HTTag NULL_HTTag_");
    NOTE("#endif");
    NOTE("");
    NOTE("#endif /* once_HTMLDTD */");
    NOTE("/* *INDENT-OFF* */");

    /* construct the tags table */
    fprintf(output,
	    "static const HTTag tags_table%d[HTML_ALL_ELEMENTS] = {\n",
	    dtd_version);
    for (j = 0; j <= dtd->number_of_tags; ++j) {
	if (j == dtd->number_of_tags) {
	    NOTE("/* additional (alternative variants), not counted in HTML_ELEMENTS: */");
	    NOTE("/* This one will be used as a temporary substitute within the parser when");
	    NOTE("   it has been signalled to parse OBJECT content as MIXED. - kw */");
	}
	dump_src_HTTag(output, dtd, j);
    }
    fprintf(output, "};\n");

    NOTE("/* *INDENT-ON* */");
    NOTE("");
    fprintf(output, "#endif /* %s%d */\n", marker, dtd_version);

    free(data);
}

static void dump_hdr_attr(FILE *output, AttrInfo * data)
{
    int j;
    char buffer[BUFSIZ];

    for (j = 0; j < data->count; ++j) {
	PrintF(output, 33, "#define HTML_%s_%s",
	       data->name,
	       no_dashes(buffer, data->attrs[j].name));
	fprintf(output, "%2d\n", j);
    }
    PrintF(output, 33, "#define HTML_%s_ATTRIBUTES", data->name);
    fprintf(output, "%2d\n", data->count);
    fprintf(output, "\n");
}

static void dump_header(FILE *output, const SGML_dtd * dtd)
{
    const char *marker = "hdr_HTMLDTD_H";
    int j;

    unsigned count = 0;
    AttrInfo *data = sorted_attrs(dtd, &count, 0);

    fprintf(output, "/* %cLynxId%c */\n", '$', '$');
    fprintf(output, "#ifndef %s\n", marker);
    fprintf(output, "#define %s 1\n\n", marker);

    NOTE("#ifdef __cplusplus");
    NOTE("extern \"C\" {");
    NOTE("#endif");

    NOTE("/*");
    NOTE("");
    NOTE("   Element Numbers");
    NOTE("");
    NOTE("   Must Match all tables by element!");
    NOTE("   These include tables in HTMLDTD.c");
    NOTE("   and code in HTML.c.");
    NOTE("");
    NOTE(" */");

    fprintf(output, "    typedef enum {\n");
    for (j = 0; j < dtd->number_of_tags; ++j) {
	fprintf(output, "\tHTML_%s,\n", dtd->tags[j].name);
    }
    NOTE("\tHTML_ALT_OBJECT");
    NOTE("    } HTMLElement;\n");
    NOTE("/* Notes: HTML.c uses a different extension of the");
    NOTE("          HTML_ELEMENTS space privately, see");
    NOTE("          HTNestedList.h.");
    NOTE("");
    NOTE("   Do NOT replace HTML_ELEMENTS with");
    NOTE("   TABLESIZE(mumble_dtd.tags).");
    NOTE("");
    NOTE("   Keep the following defines in synch with");
    NOTE("   the above enum!");
    NOTE(" */");
    NOTE("");
    NOTE("/* # of elements generally visible to Lynx code */");
    fprintf(output, "#define HTML_ELEMENTS %d\n", dtd->number_of_tags);
    NOTE("");
    NOTE("/* # of elements visible to SGML parser */");
    fprintf(output, "#define HTML_ALL_ELEMENTS %d\n", dtd->number_of_tags + 1);
    NOTE("");
    NOTE("/*");
    NOTE("");
    NOTE("   Attribute numbers");
    NOTE("");
    NOTE("   Identifier is HTML_<element>_<attribute>.");
    NOTE("   These must match the tables in HTML.c!");
    NOTE("");
    NOTE(" */");

    /* output the sorted list */
    for (j = 0; j < (int) count; ++j) {
	dump_hdr_attr(output, data + j);
    }
    free(data);

    NOTE("#ifdef __cplusplus");
    NOTE("}");
    NOTE("#endif");

    fprintf(output, "#endif\t\t\t\t/* %s */\n", marker);
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
    int dtd_version = 0;
    int c_option = FALSE;
    int h_option = FALSE;
    int l_option = FALSE;
    FILE *input = stdin;
    FILE *output = stdout;

    while ((ch = getopt(argc, argv, GETOPT)) != -1) {
	switch (ch) {
	case 'c':
	    c_option = TRUE;
	    break;
	case 'h':
	    h_option = TRUE;
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
	    dtd_version = 1;
	    break;
	case 's':
	    dtd_version = 0;
	    break;
	default:
	    usage();
	}
    }

    HTSwitchDTD(dtd_version);
    if (l_option)
	load_flatfile(input, the_dtd);

    if (c_option)
	dump_source(output, the_dtd, dtd_version);
    if (h_option)
	dump_header(output, the_dtd);
    if (!c_option && !h_option)
	dump_flatfile(output, the_dtd);

    return EXIT_SUCCESS;
}
