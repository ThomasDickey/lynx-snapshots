/*
 * $LynxId: dtd_util.c,v 1.53 2008/09/20 13:45:30 tom Exp $
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
/* *INDENT-OFF* */
#ifdef USE_PRETTYSRC
# define N HTMLA_NORMAL
# define i HTMLA_ANAME
# define h HTMLA_HREF
# define c HTMLA_CLASS
# define x HTMLA_AUXCLASS
# define T(t) , t
#else
# define T(t)			/*nothing */
#endif

#define ATTR_TYPE(name) { #name, name##_attr_list }

static const attr core_attr_list[] = {
	{ "CLASS"         T(c) },
	{ "ID"            T(i) },
	{ "STYLE"         T(N) },
	{ "TITLE"         T(N) },
	{ 0               T(N) }	/* Terminate list */
};

static const attr i18n_attr_list[] = {
	{ "DIR"           T(N) },
	{ "LANG"          T(N) },
	{ 0               T(N) }	/* Terminate list */
};

static const attr events_attr_list[] = {
	{ "ONCLICK"       T(N) },
	{ "ONDBLCLICK"    T(N) },
	{ "ONKEYDOWN"     T(N) },
	{ "ONKEYPRESS"    T(N) },
	{ "ONKEYUP"       T(N) },
	{ "ONMOUSEDOWN"   T(N) },
	{ "ONMOUSEMOVE"   T(N) },
	{ "ONMOUSEOUT"    T(N) },
	{ "ONMOUSEOVER"   T(N) },
	{ "ONMOUSEUP"     T(N) },
	{ 0               T(N) }	/* Terminate list */
};

static const attr align_attr_list[] = {
	{ "ALIGN"         T(N) },
	{ 0               T(N) }	/* Terminate list */
};

static const attr cellalign_attr_list[] = {
	{ "ALIGN"         T(N) },
	{ "CHAR"          T(N) },
	{ "CHAROFF"       T(N) },
	{ "VALIGN"        T(N) },
	{ 0               T(N) }	/* Terminate list */
};

static const attr bgcolor_attr_list[] = {
	{ "BGCOLOR"       T(N) },
	{ 0               T(N) }	/* Terminate list */
};

#undef T
/* *INDENT-ON* */

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

static const char *DEF_name(const SGML_dtd * dtd, int which)
{
    const char *result = dtd->tags[which].name;

    if (!strcmp(result, "OBJECT") && !first_object(dtd, which))
	result = "OBJECT_PCDATA";
    return result;
}

typedef struct {
    const char *name;
    const attr *attrs;
    int count;
    int which;
} AttrInfo;

static int compare_attr(const void *a, const void *b)
{
    const AttrInfo *p = (const AttrInfo *) a;
    const AttrInfo *q = (const AttrInfo *) b;

    return strcmp(p->name, q->name);
}

static int len_AttrList(AttrList data)
{
    int result = 0;

    for (result = 0; data[result].name != 0; ++result) {
	;
    }
    return result;
}

static void sort_uniq_AttrList(attr *data)
{
    unsigned have = len_AttrList(data);
    unsigned j, k;

    qsort(data, have, sizeof(*data), compare_attr);
    /*
     * Eliminate duplicates
     */
    for (j = k = 0; j < have; ++j) {
	for (k = j; data[k].name; ++k) {
	    if (data[k + 1].name == 0)
		break;
	    if (strcmp(data[j].name, data[k + 1].name)) {
		break;
	    }
	}
	data[j] = data[k];
    }
    memset(data + j, 0, sizeof(data[0]));
}

static attr *copy_AttrList(AttrList data)
{
    unsigned need = len_AttrList(data);
    unsigned n;

    attr *result = (attr *) calloc(need + 1, sizeof(attr));

    for (n = 0; n < need; ++n)
	result[n] = data[n];
    sort_uniq_AttrList(result);
    return result;
}

static attr *merge_AttrLists(const AttrType * data)
{
    const AttrType *at;
    attr *result = 0;
    unsigned need = 1;
    unsigned have = 0;
    unsigned j;

    for (at = data; at->name; ++at) {
	need += len_AttrList(at->list);
    }
    result = (attr *) calloc(need + 1, sizeof(attr));
    for (at = data; at->name; ++at) {
	if (!strcmp(at->name, "events")) {
	    ;			/* lynx does not use events */
	} else {
	    for (j = 0; at->list[j].name; ++j) {
		result[have++] = at->list[j];
	    }
	}
    }
    sort_uniq_AttrList(result);
    return result;
}

static int clean_AttrList(attr * target, AttrList source)
{
    int result = 0;
    int j, k;

    for (j = 0; target[j].name != 0; ++j) {
	for (k = 0; source[k].name != 0; ++k) {
	    if (!strcmp(target[j].name, source[k].name)) {
		k = j--;
		for (;;) {
		    target[k] = target[k + 1];
		    if (target[k++].name == 0)
			break;
		}
		++result;
		break;
	    }
	}
    }
    return result;
}

/*
 * Actually COUNT the number of attributes, to make it possible to edit a
 * attribute-table in src0_HTMLDTD.h and have all of the files updated by
 * just doing a "make sources".
 */
static int AttrCount(HTTag * tag)
{
    return len_AttrList(tag->attributes);
}

static AttrInfo *sorted_attrs(const SGML_dtd * dtd, unsigned *countp)
{
    int j;

    AttrInfo *data = (AttrInfo *) calloc(dtd->number_of_tags, sizeof(AttrInfo));
    unsigned count = 0;

    /* get the attribute-data */
    for (j = 0; j < dtd->number_of_tags; ++j) {
	if (first_attrs(dtd, j)) {
	    data[count].name = NameOfAttrs(dtd, j);
	    data[count].attrs = dtd->tags[j].attributes;
	    data[count].count = AttrCount(&(dtd->tags[j]));
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

static void dump_AttrItem(FILE *output, const attr * data)
{
    char buffer[BUFSIZ];
    char pretty = 'N';

    sprintf(buffer, "\"%s\"", data->name);
#ifdef USE_PRETTYSRC
    switch (data->type) {
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

static void dump_AttrItem0(FILE *output)
{
    fprintf(output, "\t{ 0               T(N) }\t/* Terminate list */\n");
}

static void dump_src_AttrType(FILE *output, const char *name, AttrList data, const char **from)
{
    int n;

    fprintf(output, "static const attr %s_attr_list[] = {\n", name);
    if (data != 0) {
	for (n = 0; data[n].name != 0; ++n) {
	    dump_AttrItem(output, data + n);
	}
    }
    fprintf(output, "\t{ 0               T(N) }	/* Terminate list */\n");
    fprintf(output, "};\n");
    NOTE("");
    fprintf(output, "static const AttrType %s_attr_type[] = {\n", name);
    if (from != 0) {
	while (*from != 0) {
	    fprintf(output, "\t{ ATTR_TYPE(%s) },\n", *from);
	    ++from;
	}
    } else {
	fprintf(output, "\t{ ATTR_TYPE(%s) },\n", name);
    }
    fprintf(output, "\t{ 0, 0 },\n");
    fprintf(output, "};\n");
    NOTE("");
}

static void dump_src_HTTag_Attrs(FILE *output, const SGML_dtd * dtd, int which)
{
    HTTag *tag = &(dtd->tags[which]);
    attr *list = merge_AttrLists(tag->attr_types);
    char buffer[BUFSIZ];
    int n;
    int limit = len_AttrList(list);

    sprintf(buffer, "static const attr %s_attr[] = {", NameOfAttrs(dtd, which));
    fprintf(output,
	    "%-40s/* %s attributes */\n", buffer, tag->name);
    for (n = 0; n < limit; ++n) {
	dump_AttrItem(output, list + n);
    }
    dump_AttrItem0(output);
    fprintf(output, "};\n");
    NOTE("");
    free(list);
}

static void dump_src_HTTag(FILE *output, const SGML_dtd * dtd, int which)
{
    HTTag *tag = &(dtd->tags[which]);
    char *P_macro = "P";

#ifdef EXP_JUSTIFY_ELTS
    if (!tag->can_justify)
	P_macro = "P0";
#endif
    PrintF(output, 19, " { %s(%s),", P_macro, tag->name);
    PrintF(output, 24, "ATTR_DATA(%s), ", NameOfAttrs(dtd, which));
    PrintF(output, 14, "%s,", SGMLContent2s(tag->contents));
    fprintf(output, "T_%s", DEF_name(dtd, which));
    fprintf(output, "},\n");
}

static void dump_source(FILE *output, const SGML_dtd * dtd, int dtd_version)
{
    static AttrType generic_types[] =
    {
	ATTR_TYPE(core),
	ATTR_TYPE(i18n),
	ATTR_TYPE(events),
	ATTR_TYPE(align),
	ATTR_TYPE(cellalign),
	ATTR_TYPE(bgcolor),
	{0, 0}
    };
    AttrType *gt;

    const char *marker = "src_HTMLDTD_H";
    int j;

    unsigned count = 0;
    AttrInfo *data = sorted_attrs(dtd, &count);

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
    NOTE("#define ATTR_TYPE(name) #name, name##_attr_list");
    NOTE("");
    NOTE("/* generic attributes, used in different tags */");
    for (gt = generic_types; gt->name != 0; ++gt) {
	dump_src_AttrType(output, gt->name, gt->list, 0);
    }
    NOTE("");
    NOTE("/* tables defining attributes per-tag in terms of generic attributes (editable) */");
    for (j = 0; j < (int) count; ++j) {
	int which = data[j].which;

	if (first_attrs(dtd, which)) {
	    HTTag *tag = &(dtd->tags[which]);
	    const AttrType *types = tag->attr_types;
	    const char *name = NameOfAttrs(dtd, which);
	    attr *list = 0;
	    const char *from_attr[10];
	    int from_size = 0;

	    while (types->name != 0) {
		from_attr[from_size++] = types->name;
		if (!strcmp(types->name, name)) {
		    list = copy_AttrList(types->list);
		    for (gt = generic_types; gt->name != 0; ++gt) {
			if (clean_AttrList(list, gt->list)) {
			    int k;
			    int found = 0;

			    for (k = 0; k < from_size; ++k) {
				if (!strcmp(from_attr[k], gt->name)) {
				    found = 1;
				    break;
				}
			    }
			    if (!found)
				from_attr[from_size++] = gt->name;
			    break;
			}
		    }
		}
		++types;
	    }
	    from_attr[from_size] = 0;

	    if (list != 0) {
		dump_src_AttrType(output, name, list, from_attr);
		free(list);
	    }
	}
    }
    NOTE("");
    NOTE("/* attribute lists for the runtime (generated by dtd_util) */");
    for (j = 0; j < (int) count; ++j) {
	dump_src_HTTag_Attrs(output, dtd, data[j].which);
    }
    NOTE("/* *INDENT-ON* */");
    NOTE("");
    NOTE("/* justification-flags */");
    NOTE("#undef N");
    NOTE("#undef i");
    NOTE("#undef h");
    NOTE("#undef c");
    NOTE("#undef x");
    NOTE("");
    NOTE("#undef T");
    NOTE("");
    NOTE("/* tag-names */");
    for (j = 0; j <= dtd->number_of_tags; ++j) {
	fprintf(output, "#undef %s\n", DEF_name(dtd, j));
    }
    NOTE("");
    NOTE("/* these definitions are used in the tags-tables */");
    NOTE("#undef P");
    NOTE("#undef P_");
    NOTE("#ifdef USE_COLOR_STYLE");
    NOTE("#define P_(x) #x, (sizeof #x) -1");
    NOTE("#define NULL_HTTag_ NULL, 0");
    NOTE("#else");
    NOTE("#define P_(x) #x");
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
    NOTE("#define ATTR_DATA(name) name##_attr, HTML_##name##_ATTRIBUTES, name##_attr_type");
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
    AttrInfo *data = sorted_attrs(dtd, &count);

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

static void dump_flat_attrs(FILE *output,
			    const char *name,
			    const attr * attributes,
			    int number_of_attributes)
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
    dump_flat_attrs(output, "attributes", tag->attributes, AttrCount(tag));
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
