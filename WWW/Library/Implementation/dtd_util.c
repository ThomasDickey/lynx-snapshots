/*
 * $LynxId: dtd_util.c,v 1.89 2022/09/29 23:51:16 tom Exp $
 *
 * Given a SGML_dtd structure, write a corresponding flat file, or "C" source.
 * Given the flat-file, write the "C" source.
 *
 * TODO: use symbols for HTMLA_NORMAL, etc.
 */

#include <HTUtils.h>
#include <HTMLDTD.h>
#include <string.h>
#include <stdarg.h>

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
#define GETOPT "chl:o:tsx"

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

#define MY_LIMIT		1024
#define FMT_WS			"%[ \t\n]"
#define FMT_NUM_ATTR_TYPES	"%d attr_types"
#define FMT_NUM_ATTRS		"%d attributes:"
#define FMT_ONE_ATTR		"%d:%d:%s"
#define NUM_ONE_ATTR		4

static const char alias_codes[] = "!@#$%^&*";

#define DATA(name) { #name, Tgc_##name }
static const struct {
    const char *name;
    TagClass tagclass;
} class_list[] = {
	DATA(APPLETlike),
	DATA(Alike),
	DATA(BODYlike),
	DATA(BRlike),
	DATA(DELlike),
	DATA(DIVlike),
	DATA(EMlike),
	DATA(FONTlike),
	DATA(FORMlike),
	DATA(HEADstuff),
	DATA(HRlike),
	DATA(LIlike),
	DATA(MAPlike),
	DATA(MATHlike),
	DATA(Plike),
	DATA(SELECTlike),
	DATA(TRlike),
	DATA(ULlike),
	DATA(formula),
	DATA(outer),
	DATA(same)
};

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

static const char *input_filename;
static int input_lineno;

static void failed(const char *s)
{
    if (input_lineno) {
	int save = errno;

	fprintf(stderr, "%s:%d ", input_filename, input_lineno);
	errno = save;
    }
    perror(s);
    exit(EXIT_FAILURE);
}

static void warning(const char *, ...) GCC_PRINTFLIKE(1, 2);

static void warning(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "%s:%d: ", input_filename, input_lineno);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
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
	"  -x           cross-check contains/contained data (repeat for more)"
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

static SGMLContent s2SGMLContent(const char *value)
{
    static SGMLContent table[] =
    {
	SGML_EMPTY,
	SGML_LITTERAL,
	SGML_CDATA,
	SGML_SCRIPT,
	SGML_RCDATA,
	SGML_MIXED,
	SGML_ELEMENT,
	SGML_PCDATA
    };
    unsigned n;
    SGMLContent result = SGML_EMPTY;

    for (n = 0; n < TABLESIZE(table); ++n) {
	if (!strcmp(SGMLContent2s(table[n]), value)) {
	    result = table[n];
	    break;
	}
    }
    return result;
}

static void PrintF(FILE *, int, const char *, ...) GCC_PRINTFLIKE(3, 4);

static void PrintF(FILE *output, int width, const char *fmt, ...)
{
    char buffer[BUFSIZ];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);

    fprintf(output, "%-*s", width, buffer);
}

static char *get_line(FILE *input)
{
    char temp[MY_LIMIT];
    char *result = 0;

    if (fgets(temp, (int) sizeof(temp), input) != 0) {
	result = strdup(temp);
	++input_lineno;
    }
    return result;
}

static int read_num_attr_types(FILE *input)
{
    char *next = get_line(input);
    int count;
    int code;
    char trail[MY_LIMIT];

    if (next == 0
	|| (code = sscanf(next, FMT_NUM_ATTR_TYPES FMT_WS, &count, trail)) < 2
	|| !count) {
	warning("expected attr_types");
    }
    return count;
}

static int same_AttrList(AttrList a, AttrList b)
{
    int result = 1;

    if (a && b) {
	while (a->name && b->name) {
	    if (strcmp(a->name, b->name)) {
		result = 0;
		break;
	    }
	    ++a, ++b;
	}
	if (a->name || b->name)
	    result = 0;
    } else {
	result = 0;
    }
    return result;
}

static int first_attrs(const SGML_dtd * dtd, int which)
{
    int check;
    int result = TRUE;

    for (check = 0; check < which; ++check) {
	if (dtd->tags[check].attributes == dtd->tags[which].attributes) {
	    result = FALSE;
	    break;
	} else if (same_AttrList(dtd->tags[check].attributes,
				 dtd->tags[which].attributes)) {
	    result = FALSE;
	    dtd->tags[which].attributes = dtd->tags[check].attributes;
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
    else if (!strcmp(result, "ARTICLE"))
	result = "GEN5";
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

static const char *DEF_name(const SGML_dtd * dtd, int which, unsigned alias)
{
    const char *result = dtd->tags[which].name;

    if (!strcmp(result, "OBJECT") && !first_object(dtd, which)) {
	result = "OBJECT_PCDATA";
    } else if (alias) {
	char buffer[MY_LIMIT];

	sprintf(buffer, "%s_%d", result, alias + 1);
	result = strdup(buffer);
    }
    return result;
}

static const char *EXT_name(HTTag * tag)
{
    const char *result = tag->name;

    if (tag->alias) {
	char buffer[MY_LIMIT];

	sprintf(buffer, "%s%c", result, alias_codes[tag->alias - 1]);
	result = strdup(buffer);
    }
    return result;
}

typedef struct {
    const char *name;
    const attr *attrs;
    int count;
    int which;
} AttrInfo;

static int compare_attr_types(const void *a, const void *b)
{
    const AttrType *p = (const AttrType *) a;
    const AttrType *q = (const AttrType *) b;
    int result = 0;

    /* keep lowercase AttrType lists before uppercase, since latter are derived */
    if (isupper(p->name[0]) ^ isupper(q->name[0])) {
	if (isupper(p->name[0])) {
	    result = 1;
	} else {
	    result = -1;
	}
    } else {
	result = strcmp(p->name, q->name);
    }
    return result;
}

static int len_AttrTypes(const AttrType * data)
{
    int result = 0;

    for (result = 0; data[result].name != 0; ++result) {
	;
    }
    return result;
}

static AttrType *sorted_AttrTypes(const AttrType * source)
{
    AttrType *result = 0;
    unsigned number = len_AttrTypes(source);

    if (number != 0) {
	result = typecallocn(AttrType, number + 1);
	if (result != 0) {
	    MemCpy(result, source, number * sizeof(*result));
	    qsort(result, number, sizeof(*result), compare_attr_types);
	}
    }

    return result;
}

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

static void sort_uniq_AttrList(attr * data)
{
    unsigned have = len_AttrList(data);
    unsigned j, k;

    qsort(data, have, sizeof(*data), compare_attr);
    /*
     * Eliminate duplicates
     */
    for (j = 0; j < have; ++j) {
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
	    DEF_name(dtd, which, tag->alias),
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

#ifdef USE_JUSTIFY_ELTS
    if (!tag->can_justify)
	P_macro = "P0";
#endif
    PrintF(output, 19, " { %s(%s),", P_macro, tag->name);
    PrintF(output, 24, "ATTR_DATA(%s), ", NameOfAttrs(dtd, which));
    PrintF(output, 14, "%s,", SGMLContent2s(tag->contents));
    fprintf(output, "T_%s", DEF_name(dtd, which, tag->alias));
    fprintf(output, ", %d", tag->alias);
    fprintf(output, ", %d", tag->aliases);
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
    TagAlias aliases;

    const char *marker = "src_HTMLDTD_H";
    int j;
    int inalias;

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
		/* FIXME check if each remaining attribute is defined in XXX */

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
    /* these may be needed as parameters to macros */
    for (j = 0; j <= dtd->number_of_tags; ++j) {
	fprintf(output, "#undef %s\n", DEF_name(dtd, j, 0));
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
    NOTE("#ifdef USE_JUSTIFY_ELTS");
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
    aliases = 0;
    inalias = -1;
    for (j = dtd->number_of_tags - 1; j >= 0; --j) {
	dtd->tags[j].aliases = aliases;
	if (aliases != 0) {
	    if ((inalias - aliases) >= j) {
		aliases = 0;
		inalias = -1;
	    }
	} else if ((aliases = dtd->tags[j].alias) != 0) {
	    inalias = j;
	    dtd->tags[j].aliases = aliases;
	}
    }
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
	if (dtd->tags[j].alias) {
	    fprintf(output, "\tHTML_%s_%d,\n", dtd->tags[j].name,
		    dtd->tags[j].alias + 1);
	} else {
	    fprintf(output, "\tHTML_%s,\n", dtd->tags[j].name);
	}
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
			    const attr * attributes,
			    int number_of_attributes)
{
    int n;

    fprintf(output, "\t\t" FMT_NUM_ATTRS "\n", number_of_attributes);
    for (n = 0; n < number_of_attributes; ++n) {
	fprintf(output, "\t\t\t" FMT_ONE_ATTR "\n", n,
#ifdef USE_PRETTYSRC
		attributes[n].type,
#else
		0,		/* need placeholder for source-compat */
#endif
		attributes[n].name
	    );
    }
}

static void dump_flat_attr_types(FILE *output, const AttrType * attr_types)
{
    const AttrType *p = sorted_AttrTypes(attr_types);
    int number = len_AttrTypes(attr_types);

    fprintf(output, "\t\t" FMT_NUM_ATTR_TYPES "\n", number);

    if (p != 0) {
	while (p->name != 0) {
	    fprintf(output, "\t\t\t%s\n", p->name);
	    ++p;
	}
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
    DUMP(DELlike);
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
    fprintf(output, "\t%u:%s\n", n, EXT_name(tag));
#ifdef USE_JUSTIFY_ELTS
    fprintf(output, "\t\t%s\n", tag->can_justify ? "justify" : "nojustify");
#endif
    dump_flat_attrs(output, tag->attributes, AttrCount(tag));
    dump_flat_attr_types(output, tag->attr_types);
    dump_flat_SGMLContent(output, "contents", tag->contents);
    dump_flat_TagClass(output, "tagclass", tag->tagclass);
    dump_flat_TagClass(output, "contains", tag->contains);
    dump_flat_TagClass(output, "icontains", tag->icontains);
    dump_flat_TagClass(output, "contained", tag->contained);
    dump_flat_TagClass(output, "icontained", tag->icontained);
    dump_flat_TagClass(output, "canclose", tag->canclose);
    dump_flat_TagFlags(output, "flags", tag->flags);
}

static int count_attr_types(AttrType * attr_types, HTTag * tag)
{
    int count = 0;
    const AttrType *p;
    AttrType *q;

    if ((p = tag->attr_types) != 0) {
	while (p->name != 0) {
	    if ((q = attr_types) != 0) {
		while (q->name != 0) {
		    if (!strcmp(q->name, p->name)) {
			--count;
			break;
		    }
		    ++q;
		}
		*q = *p;
	    }
	    ++count;
	    ++p;
	}
    }
    return count;
}

static void dump_flatfile(FILE *output, const SGML_dtd * dtd)
{
    AttrType *attr_types = 0;
    int pass;
    unsigned count = 0;
    unsigned n;

    /* merge all of the attr_types data */
    for (pass = 0; pass < 2; ++pass) {
	for (n = 0; (int) n < dtd->number_of_tags; ++n) {
	    count += count_attr_types(attr_types, &(dtd->tags[n]));
	}
	if (pass == 0) {
	    attr_types = typecallocn(AttrType, count + 1);
	    count = 0;
	} else {
	    count = len_AttrTypes(attr_types);
	    qsort(attr_types, count, sizeof(*attr_types), compare_attr_types);
	    fprintf(output, FMT_NUM_ATTR_TYPES "\n", count);
	    for (n = 0; n < count; ++n) {
		fprintf(output, "\t%d:%s\n", n, attr_types[n].name);
		dump_flat_attrs(output, attr_types[n].list,
				len_AttrList(attr_types[n].list));
	    }
	}
    }

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

#define LOAD(name) \
	if (!strcmp(data, #name)) {\
	    *theClass |= Tgc_##name; \
	    continue; \
	}

static int load_flat_TagClass(FILE *input, const char *name, TagClass * theClass)
{
    char prefix[80];
    char *next = get_line(input);
    char *data;
    int result = 0;

    *theClass = 0;
    if (next != 0) {
	sprintf(prefix, "\t\t%s:", name);
	data = strtok(next, "\n ");

	if (data != 0 && !strcmp(data, prefix)) {
	    result = 1;

	    while ((data = strtok(NULL, "\n ")) != 0) {

		LOAD(FONTlike);
		LOAD(EMlike);
		LOAD(MATHlike);
		LOAD(Alike);
		LOAD(formula);
		LOAD(TRlike);
		LOAD(SELECTlike);
		LOAD(FORMlike);
		LOAD(Plike);
		LOAD(DIVlike);
		LOAD(LIlike);
		LOAD(ULlike);
		LOAD(BRlike);
		LOAD(APPLETlike);
		LOAD(HRlike);
		LOAD(MAPlike);
		LOAD(outer);
		LOAD(BODYlike);
		LOAD(HEADstuff);
		LOAD(same);
		LOAD(DELlike);

		warning("Unexpected TagClass '%s'", data);
		result = 0;
		break;
	    }
	} else if (data) {
	    warning("load_flat_TagClass: '%s' vs '%s'", data, prefix);
	}
	free(next);
    } else {
	warning("Did not find contents");
    }
    return result;
}

#undef LOAD

#define LOAD(name) \
	if (!strcmp(data, #name)) {\
	    *flags |= Tgf_##name; \
	    continue; \
	}

static int load_flat_TagFlags(FILE *input, const char *name, TagFlags * flags)
{
    char prefix[80];
    char *next = get_line(input);
    char *data;
    int result = 0;

    *flags = 0;
    if (next != 0) {
	sprintf(prefix, "\t\t%s:", name);
	data = strtok(next, "\n ");

	if (data != 0 && !strcmp(data, prefix)) {
	    result = 1;

	    while ((data = strtok(NULL, "\n ")) != 0) {

		LOAD(endO);
		LOAD(startO);
		LOAD(mafse);
		LOAD(strict);
		LOAD(nreie);
		LOAD(frecyc);
		LOAD(nolyspcl);

		warning("Unexpected TagFlag '%s'", data);
		result = 0;
		break;
	    }
	} else if (data) {
	    warning("load_flat_TagFlags: '%s' vs '%s'", data, prefix);
	}
	free(next);
    }
    return result;
}

#undef LOAD

static int load_flat_AttrList(FILE *input, AttrList * attrs, int *length)
{
    attr *attributes;
    int j, jcmp, code;
    int result = 1;
    char name[MY_LIMIT];
    char *next;
    char trail[MY_LIMIT];

#ifdef USE_PRETTYSRC
    int atype;
#endif

    next = get_line(input);
    if (sscanf(next, FMT_NUM_ATTRS FMT_WS, length, trail) == 2
	&& *length > 0
	&& (attributes = typecallocn(attr, *length + 1)) != 0) {
	*attrs = attributes;
	for (j = 0; j < *length; ++j) {
	    next = get_line(input);
	    code = sscanf(next, FMT_ONE_ATTR FMT_WS, &jcmp, &atype, name, trail);
	    if (code == NUM_ONE_ATTR) {
		if (j != jcmp)
		    warning("resequence %s from %d to %d", name, jcmp, j);
		attributes[j].name = strdup(name);
#ifdef USE_PRETTYSRC
		attributes[j].type = atype;
#endif
	    } else {
		warning("Did not find attributes");
		result = 0;
		break;
	    }
	}
	if (*length > 1 && result)
	    qsort(attributes, *length, sizeof(attributes[0]), compare_attr);
    } else {
	warning("expected attribute count:\n%s", next);
    }
    return result;
}

/* find the given attribute in the list of attr_types */
static int find_attribute(const char *attribute, const AttrType * attr_types)
{
    int j, k;
    int found = -1;

    for (j = 0; attr_types[j].name; ++j) {
	AttrList list = attr_types[j].list;

	for (k = 0; list[k].name; ++k) {
	    if (!strcmp(attribute, list[k].name)) {
		if (found >= 0)
		    warning("attribute %s is in attr_types %s and %s",
			    attribute,
			    attr_types[found].name,
			    attr_types[j].name);
		found = j;
	    }
	}
    }
    return (found >= 0);
}

static int load_flat_HTTag(FILE *input, HTTag * tag, AttrType * allTypes)
{
    int result = 0;
    unsigned ncmp = 0;
    char name[MY_LIMIT];
    char trail[MY_LIMIT];
    char *next;
    int code;
    int j;

    next = get_line(input);
    code = sscanf(next, "%d:%s" FMT_WS, &ncmp, name, trail);
    if (code == 3) {
	result = 1;
	tag->alias = 0;
	tag->aliases = 0;
	for (j = 0; name[j] != '\0'; ++j) {
	    int ch = UCH(name[j]);

	    if (!isalnum(ch)) {
		if (strchr(alias_codes, ch) != NULL) {
		    tag->alias = (ch - '!') + 1;
		    name[j] = '\0';
		} else {
		    result = 0;
		}
		break;
	    }
	}
	tag->name = strdup(name);
#ifdef USE_COLOR_STYLE
	tag->name_len = strlen(tag->name);
#endif
#ifdef USE_JUSTIFY_ELTS
	next = get_line(input);
	if (sscanf(next, "%s" FMT_WS, name, trail) == 2) {
	    tag->can_justify = !strcmp(name, "justify");
	} else {
	    warning("Did not find can_justify");
	    result = 0;
	}
#endif
	if (result) {
	    result = load_flat_AttrList(input, &(tag->attributes), &(tag->number_of_attributes));
	}
	if (result) {
	    AttrType *myTypes;
	    int k, count;

	    if ((count = read_num_attr_types(input)) > 0
		&& (myTypes = typecallocn(AttrType, count + 1)) != 0) {
		tag->attr_types = myTypes;
		for (k = 0; k < count; ++k) {
		    next = get_line(input);
		    if (next != 0
			&& sscanf(next, "%s" FMT_WS, name, trail)) {
			int found = -1;

			for (j = 0; allTypes[j].name != 0; ++j) {
			    if (!strcmp(allTypes[j].name, name)) {
				myTypes[k].name = strdup(name);
				myTypes[k].list = allTypes[j].list;
				found = k;
				break;
			    }
			}
			if (found < 0) {
			    warning("not found in attr_types: %s", name);
			    continue;
			}
		    } else {
			result = 0;
			break;
		    }
		}
		if (result) {
		    if (count > 1)
			qsort(myTypes,
			      count,
			      sizeof(myTypes[0]),
			      compare_attr_types);
		    for (k = 0; k < tag->number_of_attributes; ++k) {
			if (!find_attribute(tag->attributes[k].name, tag->attr_types)) {
			    warning("%s attribute %s not found in attr_types",
				    tag->name, tag->attributes[k].name);
			}
		    }
		}
	    }
	}
	if (result) {

	    next = get_line(input);
	    if (next != 0
		&& sscanf(next, "\t\tcontents: %s" FMT_WS, name, trail)) {
		tag->contents = s2SGMLContent(name);
		free(next);
	    } else {
		warning("Did not find contents");
		result = 0;
	    }
	}
	if (result) {
	    result = load_flat_TagClass(input, "tagclass", &(tag->tagclass));
	}
	if (result) {
	    result = load_flat_TagClass(input, "contains", &(tag->contains));
	}
	if (result) {
	    result = load_flat_TagClass(input, "icontains", &(tag->icontains));
	}
	if (result) {
	    result = load_flat_TagClass(input, "contained", &(tag->contained));
	}
	if (result) {
	    result = load_flat_TagClass(input, "icontained", &(tag->icontained));
	}
	if (result) {
	    result = load_flat_TagClass(input, "canclose", &(tag->canclose));
	}
	if (result) {
	    result = load_flat_TagFlags(input, "flags", &(tag->flags));
	}
    } else {
	warning("load_flat_HTTag error");
    }
    return result;
}

static int load_flat_AttrType(FILE *input, AttrType * types, size_t ncmp)
{
    int result = 0;
    int ntst;
    char name[MY_LIMIT];
    char trail[MY_LIMIT];
    char *next;

    next = get_line(input);
    if (sscanf(next, "%d:%s" FMT_WS, &ntst, name, trail) == 3
	&& (ntst == (int) ncmp)) {
	result = 1;
	types->name = strdup(name);
	if (!load_flat_AttrList(input, &(types->list), &ntst))
	    result = 0;
    } else {
	warning("expected a tag:\n%s", next);
    }
    return result;
}

static int compare_tags(const void *a, const void *b)
{
    const HTTag *p = (const HTTag *) a;
    const HTTag *q = (const HTTag *) b;
    int result = 0;

    if ((result = strcmp(p->name, q->name)) == 0) {
	result = p->alias - q->alias;
    }
    return result;
}

static SGML_dtd *load_flatfile(FILE *input)
{
    AttrType *attr_types = 0;
    SGML_dtd *result = 0;
    char *next;
    size_t n;
    int number_of_attrs = 0;
    int number_of_tags = 0;
    HTTag *tag;
    int code;
    char trail[MY_LIMIT];

    if ((number_of_attrs = read_num_attr_types(input)) <= 0) {
	;
    } else if ((attr_types = typecallocn(AttrType, number_of_attrs + 1)) == 0) {
	failed("calloc attr_types");
    }

    for (n = 0; n < (size_t) number_of_attrs; ++n) {
	if (!load_flat_AttrType(input, attr_types + n, n)) {
	    break;
	}
    }

    next = get_line(input);
    code = sscanf(next, "%d tags" FMT_WS, &number_of_tags, trail);
    if (code == 2) {
	if ((result = typecalloc(SGML_dtd)) != 0
	    && (result->tags = typecallocn(HTTag, number_of_tags + 2)) != 0) {
	    for (n = 0; n < (size_t) number_of_tags; ++n) {
		if (load_flat_HTTag(input, &(result->tags[n]), attr_types)) {
		    result->number_of_tags = (n + 1);
		} else {
		    break;
		}
	    }
	    tag = 0;
	    for (n = 0; n < (size_t) number_of_tags; ++n) {
		if (result->tags[n].name != 0
		    && !strcmp(result->tags[n].name, "OBJECT")) {
		    tag = result->tags + number_of_tags;
		    *tag = result->tags[n];
		    tag->contents = SGML_MIXED;
		    tag->flags = Tgf_strict;
		    break;
		}
	    }
	    if (tag == 0) {
		warning("Did not find OBJECT tag");
		result = 0;
	    } else {
		qsort(result->tags, number_of_tags, sizeof(HTTag), compare_tags);
	    }
	}
    } else {
	warning("expected tag count:\n%s", next);
    }
    return result;
}

static void cross_check(FILE *output, const SGML_dtd * the_dtd, int level)
{
    int ft;

    fprintf(output, "Cross-check HTML DTD:\n");
    fprintf(output, "\n");
    /* make a sorted list of tags */
    /* for each tag in the list, find the classes it might contain */
    for (ft = 0; ft < the_dtd->number_of_tags; ++ft) {
	int xc;
	HTTag *ftag = &(the_dtd->tags[ft]);

	/* for each contained-class, check if it says it can be contained */
	fprintf(output, "tag %s\n", EXT_name(ftag));
	for (xc = 0; class_list[xc].name != 0; ++xc) {
	    int rt;
	    int direct;
	    int passes = 2;

	    /* most of the tags are (should be) symmetric */
	    if (level <= 1) {
		BOOL same = TRUE;

		if ((ftag->contains & class_list[xc].tagclass)
		    != (ftag->icontains & class_list[xc].tagclass)) {
		    same = FALSE;
		} else if ((ftag->contains & class_list[xc].tagclass) == 0) {
		    continue;
		} else if (0) {
		    HTTag *rtag;

		    for (rt = 0; rt < the_dtd->number_of_tags; ++rt) {
			rtag = &(the_dtd->tags[rt]);
			if (ftag == rtag)
			    continue;
			if ((ftag->contains & rtag->tagclass) == 0)
			    continue;
			if ((rtag->contained & ftag->tagclass)
			    != (rtag->icontained & ftag->tagclass)) {
			    same = FALSE;
			    break;
			}
		    }
		}
		if (same)
		    passes = 1;
	    }

	    for (direct = 0; direct < passes; ++direct) {
		TagClass check = (direct ? ftag->contains : ftag->icontains);
		BOOL first = TRUE;

		if ((check &= class_list[xc].tagclass) == 0)
		    continue;

		for (rt = 0; rt < the_dtd->number_of_tags; ++rt) {
		    HTTag *rtag = &(the_dtd->tags[rt]);
		    TagClass check2 = (direct ? rtag->contained : rtag->icontained);

		    if (rt == ft)
			continue;
		    if ((check & rtag->tagclass) == 0)
			continue;
		    if ((check2 & ftag->tagclass) == 0)
			continue;
		    if (first) {
			if (passes == 2) {
			    fprintf(output, "\t%s (%s)\n",
				    class_list[xc].name,
				    direct ? "direct" : "indirect");
			} else {
			    fprintf(output, "\t%s\n",
				    class_list[xc].name);
			}
		    }
		    fprintf(output, "%s%s",
			    first ? "\t\t" : " ",
			    EXT_name(rtag));
		    first = FALSE;
		}
		if (first) {
		    if (level > 1) {
			if (passes == 2) {
			    fprintf(output, "\t%s (%s)\n",
				    class_list[xc].name,
				    direct ? "direct" : "indirect");
			} else {
			    fprintf(output, "\t%s\n",
				    class_list[xc].name);
			}
			fprintf(output, "\t\t(missing)\n");
		    }
		} else {
		    fprintf(output, "\n");
		}
	    }
	}
    }
}

int main(int argc, char *argv[])
{
    const SGML_dtd *the_dtd = &HTML_dtd;
    int ch;
    int dtd_version = 0;
    int c_option = FALSE;
    int h_option = FALSE;
    int l_option = FALSE;
    int x_option = 0;
    FILE *input = stdin;
    FILE *output = stdout;

    input_filename = "<builtin>";
    input_lineno = 0;

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
	    input = fopen((input_filename = optarg), "r");
	    if (input == 0)
		failed(optarg);
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
	case 'x':
	    ++x_option;
	    break;
	default:
	    usage();
	}
    }

    HTSwitchDTD(dtd_version);
    if (l_option)
	the_dtd = load_flatfile(input);

    if (the_dtd != 0) {
	if (c_option)
	    dump_source(output, the_dtd, dtd_version);
	if (h_option)
	    dump_header(output, the_dtd);
	if (x_option)
	    cross_check(output, the_dtd, x_option);
	if (!c_option && !h_option && !x_option)
	    dump_flatfile(output, the_dtd);
    }

    return EXIT_SUCCESS;
}
