/*			File Access				HTFile.c
**			===========
**
**	This is unix-specific code in general, with some VMS bits.
**	These are routines for file access used by browsers.
**	Development of this module for Unix DIRED_SUPPORT in Lynx
**	 regrettably has has been conducted in a manner with now
**	 creates a major impediment for hopes of adapting Lynx to
**	 a newer version of the library.
**
**  History:
**	   Feb 91	Written Tim Berners-Lee CERN/CN
**	   Apr 91	vms-vms access included using DECnet syntax
**	26 Jun 92 (JFG) When running over DECnet, suppressed FTP.
**			Fixed access bug for relative names on VMS.
**	   Sep 93 (MD)	Access to VMS files allows sharing.
**	15 Nov 93 (MD)	Moved HTVMSname to HTVMSUTILS.C
**	27 Dec 93 (FM)	FTP now works with VMS hosts.
**			FTP path must be Unix-style and cannot include
**			 the device or top directory.
*/

#ifndef VMS
/* #define LONG_LIST */ /* Define this for long style unix listings (ls -l) */
/* #define NO_PARENT_DIR_REFERENCE */ /* Define this for no parent links */
#endif /* !VMS */

#ifdef DOSPATH
#define HAVE_READDIR 1
#define USE_DIRENT
#include <HTDOS.h>
#endif /* DOSPATH */

#include <HTUtils.h>
#include <tcp.h>
#include <HTFile.h>		/* Implemented here */
#ifdef VMS
#include <stat.h>
#endif /* VMS */

#ifndef VMS
#ifdef LONG_LIST
#include <pwd.h>
#include <grp.h>
#endif /* LONG_LIST */
#endif /* !VMS */

#ifdef USE_ZLIB
#include <GridText.h>
#endif

#define INFINITY 512		/* file name length @@ FIXME */
#define MULTI_SUFFIX ".multi"	/* Extension for scanning formats */

#define HT_EM_SPACE ((char)2)

#define FREE(x) if (x) {free(x); x = NULL;}

#ifdef VMS
#include <HTVMSUtils.h>
#endif /* VMS */

#include <HTParse.h>
#include <HTTCP.h>
#ifndef DECNET
#include <HTFTP.h>
#endif /* !DECNET */
#include <HTAnchor.h>
#include <HTAtom.h>
#include <HTWriter.h>
#include <HTFWriter.h>
#include <HTInit.h>
#include <HTBTree.h>
#include <HTAlert.h>
#include <HTCJK.h>
#include <UCDefs.h>
#include <UCMap.h>
#include <UCAux.h>

#include <LYexit.h>
#include <LYCharSets.h>
#include <LYGlobalDefs.h>
#include <LYStrings.h>
#include <LYUtils.h>
#include <LYLeaks.h>

typedef struct _HTSuffix {
	char *		suffix;
	HTAtom *	rep;
	HTAtom *	encoding;
	char *		desc;
	float		quality;
} HTSuffix;

#ifndef NGROUPS
#ifdef NGROUPS_MAX
#define NGROUPS NGROUPS_MAX
#else
#define NGROUPS 32
#endif /* NGROUPS_MAX */
#endif /* NGROUPS */

#include <HTML.h>		/* For directory object building */

#define PUTC(c) (*target->isa->put_character)(target, c)
#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0, -1, 0)
#define END(e) (*target->isa->end_element)(target, e, 0)
#define MAYBE_END(e) if (HTML_dtd.tags[e].contents != SGML_EMPTY) \
			(*target->isa->end_element)(target, e, 0)
#define FREE_TARGET (*target->isa->_free)(target)
struct _HTStructured {
	CONST HTStructuredClass *	isa;
	/* ... */
};

/*
**  Controlling globals.
*/
PUBLIC int HTDirAccess = HT_DIR_OK;

#ifdef DIRED_SUPPORT
PUBLIC int HTDirReadme = HT_DIR_README_NONE;
#define FILES_FIRST 1
#define MIXED_STYLE 2
extern BOOLEAN lynx_edit_mode;
extern BOOLEAN dir_list_style;
#else
PUBLIC int HTDirReadme = HT_DIR_README_TOP;
#endif /* DIRED_SUPPORT */

extern BOOLEAN LYRawMode;
extern BOOL HTPassEightBitRaw;
extern HTCJKlang HTCJK;

PRIVATE char *HTMountRoot = "/Net/";		/* Where to find mounts */
#ifdef VMS
PRIVATE char *HTCacheRoot = "/WWW$SCRATCH";	/* Where to cache things */
#else
PRIVATE char *HTCacheRoot = "/tmp/W3_Cache_";	/* Where to cache things */
#endif /* VMS */

/*PRIVATE char *HTSaveRoot  = "$(HOME)/WWW/";*/ /* Where to save things */

/*
**  Suffix registration.
*/
PRIVATE HTList * HTSuffixes = 0;
PRIVATE HTSuffix no_suffix = { "*", NULL, NULL, NULL, 1.0 };
PRIVATE HTSuffix unknown_suffix = { "*.*", NULL, NULL, NULL, 1.0};


#ifdef _WINDOWS
int exists(char *filename)
{
 return (access(filename,0)==0);
}
#endif


/*	To free up the suffixes at program exit.
**	----------------------------------------
*/
PRIVATE void free_suffixes NOPARAMS;

#ifdef LONG_LIST
PRIVATE void LYListFmtParse ARGS5(
	char *, 	fmtstr,
	char *, 	file,
	HTStructured *, target,
	char *, 	entry,
	char *, 	tail)
{
	char c;
	char *s;
	char *end;
	char *start;
	char *str = NULL;
	struct stat st;
	char buf[512];
	char fmt[512];
	char type;
	struct passwd *p;
	struct group *g;
	time_t now;
	char *datestr;
	int len;
#define SEC_PER_YEAR	(60 * 60 * 24 * 365)
	static char *pbits[] = { "---", "--x", "-w-", "-wx",
		"r--", "r-x", "rw-", "rwx", 0 };
	static char *psbits[] = { "--S", "--s", "-wS", "-ws",
		"r-S", "r-s", "rwS", "rws", 0 };
#define PBIT(a, n, s)  (s) ? psbits[((a) >> (n)) & 0x7] : \
	pbits[((a) >> (n)) & 0x7]

	if (lstat(file, &st) < 0)
		fmtstr = "%a";	/* can't stat so just do anchor */

	StrAllocCopy(str, fmtstr);
	s = str;
	end = str + strlen(str);
	START(HTML_PRE);
	while (*s) {
		start = s;
		while (*s) {
			if (*s == '%') {
				if (*(s+1) == '%') /* literal % */
					s++;
				else
					break;
			}
			s++;
		}
		/* s is positioned either at a % or at \0 */
		*s = '\0';
		if (s > start) {	/* some literal chars. */
			PUTS(start);
		}
		if (s == end)
			break;
		start = ++s;
		while (isdigit(*s) || *s == '.' || *s == '-')
			s++;
		c = *s; 	/* the format char. or \0 */
		*s = '\0';

		switch (c) {
		case '\0':
			break;

		case 'A':
		case 'a':	/* anchor */
			HTDirEntry(target, tail, entry);
			sprintf(fmt, "%%%ss", start);
			sprintf(buf, fmt, entry);
			PUTS(buf);
			END(HTML_A);
#ifdef S_IFLNK
			if (c != 'A' && (st.st_mode & S_IFMT) == S_IFLNK &&
			    (len = readlink(file, buf, sizeof(buf))) >= 0) {
				PUTS(" -> ");
				buf[len] = '\0';
				PUTS(buf);
			}
#endif
			*buf = '\0';
			break;

		case 'd':	/* date */
			now = time(0);
			datestr = ctime(&st.st_mtime);
			if ((now - st.st_mtime) < SEC_PER_YEAR/2)
				/*
				**  MMM DD HH:MM
				*/
				sprintf(buf, "%.12s", datestr + 4);
			else
				/*
				**  MMM DD  YYYY
				*/
				sprintf(buf, "%.7s %.4s ", datestr + 4,
				  datestr + 20);
			sprintf(fmt, "%%%ss", start);
			sprintf(buf, fmt, buf);
			break;

		case 's':	/* size in bytes */
			sprintf(fmt, "%%%sd", start);
			sprintf(buf, fmt, st.st_size);
			break;

		case 'K':	/* size in Kilobytes but not for directories */
			if ((st.st_mode & S_IFMT) == S_IFDIR) {
				sprintf(fmt, "%%%ss ", start);
				sprintf(buf, fmt, "");
				break;
			}
			/* FALL THROUGH */
		case 'k':	/* size in Kilobytes */
			sprintf(fmt, "%%%sdK", start);
			sprintf(buf, fmt, (st.st_size+1023)/1024);
			break;

		case 'p':	/* unix-style permission bits */
			switch(st.st_mode & S_IFMT) {
			case S_IFIFO: type = 'p'; break;
			case S_IFCHR: type = 'c'; break;
			case S_IFDIR: type = 'd'; break;
			case S_IFREG: type = '-'; break;
#ifdef S_IFBLK
			case S_IFBLK: type = 'b'; break;
#endif
#ifdef S_IFLNK
			case S_IFLNK: type = 'l'; break;
#endif
#ifdef S_IFSOCK
# ifdef S_IFIFO 	/* some older machines (e.g., apollo) have a conflict */
#  if S_IFIFO != S_IFSOCK
			case S_IFSOCK: type = 's'; break;
#  endif
# else
			case S_IFSOCK: type = 's'; break;
# endif
#endif /* S_IFSOCK */
			default: type = '?'; break;
			}
			sprintf(buf, "%c%s%s%s", type,
			  PBIT(st.st_mode, 6, st.st_mode & S_ISUID),
			  PBIT(st.st_mode, 3, st.st_mode & S_ISGID),
			  PBIT(st.st_mode, 0, 0));
			sprintf(fmt, "%%%ss", start);
			sprintf(buf, fmt, buf);
			break;

		case 'o':	/* owner */
			sprintf(fmt, "%%%ss", start);
			p = getpwuid(st.st_uid);
			if (p) {
				sprintf(fmt, "%%%ss", start);
				sprintf(buf, fmt, p->pw_name);
			} else {

				sprintf(fmt, "%%%sd", start);
				sprintf(buf, fmt, st.st_uid);
			}
			break;

		case 'g':	/* group */
			g = getgrgid(st.st_gid);
			if (g) {
				sprintf(fmt, "%%%ss", start);
				sprintf(buf, fmt, g->gr_name);
			} else {
				sprintf(fmt, "%%%sd", start);
				sprintf(buf, fmt, st.st_gid);
			}
			break;

		case 'l':	/* link count */
			sprintf(fmt, "%%%sd", start);
			sprintf(buf, fmt, st.st_nlink);
			break;

		default:
			fprintf(stderr,
			"Unknown format character `%c' in list format\n", c);
			break;
		}
		PUTS(buf);

		s++;
	}
	END(HTML_PRE);
	PUTS("\n");
	FREE(str);
}
#endif /* LONG_LIST */

/*	Define the representation associated with a file suffix.
**	--------------------------------------------------------
**
**	Calling this with suffix set to "*" will set the default
**	representation.
**	Calling this with suffix set to "*.*" will set the default
**	representation for unknown suffix files which contain a ".".
**
**	The encoding parameter can give a trivial (8bit, 7bit, binary)
**	or real (gzip, compress) encoding.
**
**	If filename suffix is already defined with the same encoding
**	its previous definition is overridden.
*/
PUBLIC void HTSetSuffix5 ARGS5(
	CONST char *,	suffix,
	CONST char *,	representation,
	CONST char *,	encoding,
	CONST char *,	desc,
	float,		value)
{
    HTSuffix * suff;
    BOOL trivial_enc = IsUnityEncStr(encoding);

    if (strcmp(suffix, "*") == 0)
	suff = &no_suffix;
    else if (strcmp(suffix, "*.*") == 0)
	suff = &unknown_suffix;
    else {
	HTList *cur = HTSuffixes;

	while (NULL != (suff = (HTSuffix*)HTList_nextObject(cur))) {
	    if (suff->suffix && 0 == strcmp(suff->suffix, suffix) &&
		((trivial_enc && IsUnityEnc(suff->encoding)) ||
		 (!trivial_enc && !IsUnityEnc(suff->encoding) &&
		     strcmp(encoding, HTAtom_name(suff->encoding)) == 0)))
		break;
	}
	if (!suff) { /* Not found -- create a new node */
	    suff = (HTSuffix *) calloc(1, sizeof(HTSuffix));
	    if (suff == NULL)
		outofmem(__FILE__, "HTSetSuffix");

	    /*
	    **	Memory leak fixed.
	    **	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
	    */
	    if (!HTSuffixes)	{
		HTSuffixes = HTList_new();
		atexit(free_suffixes);
	    }

	    HTList_addObject(HTSuffixes, suff);

	    StrAllocCopy(suff->suffix, suffix);
	}
    }

    if (representation)
	suff->rep = HTAtom_for(representation);

    /*
    **	Memory leak fixed.
    **	05-28-94 Lynx 2-3-1 Garrett Arch Blythe
    **	Invariant code removed.
    */
    suff->encoding = HTAtom_for(encoding);

    StrAllocCopy(suff->desc, desc);

    suff->quality = value;
}

/*
**	Purpose:	Free all added suffixes.
**	Arguments:	void
**	Return Value:	void
**	Remarks/Portability/Dependencies/Restrictions:
**		To be used at program exit.
**	Revision History:
**		05-28-94	created Lynx 2-3-1 Garrett Arch Blythe
*/
PRIVATE void free_suffixes NOARGS
{
    HTSuffix * suff = NULL;

    /*
    **	Loop through all suffixes.
    */
    while (!HTList_isEmpty(HTSuffixes)) {
	/*
	**  Free off each item and its members if need be.
	*/
	suff = (HTSuffix *)HTList_removeLastObject(HTSuffixes);
	FREE(suff->suffix);
	FREE(suff->desc);
	FREE(suff);
    }
    /*
    **	Free off the list itself.
    */
    HTList_delete(HTSuffixes);
    HTSuffixes = NULL;
}

/*	Send README file.
**	-----------------
**
**  If a README file exists, then it is inserted into the document here.
*/
#ifdef HAVE_READDIR
PRIVATE void do_readme ARGS2(HTStructured *, target, CONST char *, localname)
{
    FILE * fp;
    char * readme_file_name =
	malloc(strlen(localname)+ 1 + strlen(HT_DIR_README_FILE) + 1);
    if (readme_file_name == NULL)
	outofmem(__FILE__, "do_readme");
    strcpy(readme_file_name, localname);
    strcat(readme_file_name, "/");
    strcat(readme_file_name, HT_DIR_README_FILE);

    fp = fopen(readme_file_name,  "r");

    if (fp) {
	HTStructuredClass targetClass;

	targetClass =  *target->isa;	/* (Can't init agregate in K&R) */
	START(HTML_PRE);
	for (;;){
	    char c = fgetc(fp);
	    if (c == (char)EOF) break;
#ifdef NOTDEFINED
	    switch (c) {
		case '&':
		case '<':
		case '>':
			PUTC('&');
			PUTC('#');
			PUTC((char)(c / 10));
			PUTC((char) (c % 10));
			PUTC(';');
			break;
/*		case '\n':
			PUTC('\r');
Bug removed thanks to joe@athena.mit.edu */
		default:
			PUTC(c);
	    }
#else
	    PUTC(c);
#endif /* NOTDEFINED */
	}
	END(HTML_PRE);
	fclose(fp);
    }
}
#endif /* HAVE_READDIR */

/*	Make the cache file name for a W3 document.
**	-------------------------------------------
**	Make up a suitable name for saving the node in
**
**	E.g.	/tmp/WWW_Cache_news/1234@cernvax.cern.ch
**		/tmp/WWW_Cache_http/crnvmc/FIND/xx.xxx.xx
**
**  On exit:
**	Returns a malloc'ed string which must be freed by the caller.
*/
PUBLIC char * HTCacheFileName ARGS1(
	CONST char *,	name)
{
    char * acc_method = HTParse(name, "", PARSE_ACCESS);
    char * host = HTParse(name, "", PARSE_HOST);
    char * path = HTParse(name, "", PARSE_PATH+PARSE_PUNCTUATION);

    char * result;
    result = (char *)malloc(
	    strlen(HTCacheRoot)+strlen(acc_method)
	    +strlen(host)+strlen(path)+6+1);
    if (result == NULL)
	outofmem(__FILE__, "HTCacheFileName");
    sprintf(result, "%s/WWW/%s/%s%s", HTCacheRoot, acc_method, host, path);
    FREE(path);
    FREE(acc_method);
    FREE(host);
    return result;
}

/*	Open a file for write, creating the path.
**	-----------------------------------------
*/
#ifdef NOT_IMPLEMENTED
PRIVATE int HTCreatePath ARGS1(CONST char *,path)
{
    return -1;
}
#endif /* NOT_IMPLEMENTED */

/*	Convert filenames between local and WWW formats.
**	------------------------------------------------
**	Make up a suitable name for saving the node in
**
**	E.g.	$(HOME)/WWW/news/1234@cernvax.cern.ch
**		$(HOME)/WWW/http/crnvmc/FIND/xx.xxx.xx
**
**  On exit:
**	Returns a malloc'ed string which must be freed by the caller.
*/
PUBLIC char * HTLocalName ARGS1(
	CONST char *,	name)
{
    char * acc_method = HTParse(name, "", PARSE_ACCESS);
    char * host = HTParse(name, "", PARSE_HOST);
    char * path = HTParse(name, "", PARSE_PATH+PARSE_PUNCTUATION);

    HTUnEscape(path);	/* Interpret % signs */

    if (0 == strcmp(acc_method, "file")) { /* local file */
	FREE(acc_method);
	if ((0 == strcasecomp(host, HTHostName())) ||
	    (0 == strcasecomp(host, "localhost")) || !*host) {
	    FREE(host);
	    CTRACE(tfp, "Node `%s' means path `%s'\n", name, path);
#ifdef DOSPATH
	    {
		char *ret_path = NULL;
		StrAllocCopy(ret_path, HTDOS_name(path));
		CTRACE(tfp, "HTDOS_name changed `%s' to `%s'\n",
			    path, ret_path);
		FREE(path);
		return(ret_path);
	    }
#else
#ifdef __EMX__
	    {
		char *ret_path = NULL;
		if (path[0] == '/') /* pesky leading slash */
		    StrAllocCopy(ret_path, path+1);
		else
		    StrAllocCopy(ret_path, path);
		CTRACE(tfp, "EMX hack changed `%s' to `%s'\n",
			    path, ret_path);
		FREE(path);
		return(ret_path);
	    }
#else
	    return(path);
#endif /* __EMX__ */
#endif /* DOSPATH */
	} else {
	    char * result = (char *)malloc(
				strlen("/Net/")+strlen(host)+strlen(path)+1);
	      if (result == NULL)
		  outofmem(__FILE__, "HTLocalName");
	    sprintf(result, "%s%s%s", "/Net/", host, path);
	    FREE(host);
	    FREE(path);
	    CTRACE(tfp, "Node `%s' means file `%s'\n", name, result);
	    return result;
	}
    } else {  /* other access */
	char * result;
#ifdef VMS
	char * home =  getenv("HOME");
	if (!home)
	    home = HTCacheRoot;
	else
	    home = HTVMS_wwwName(home);
#else
	CONST char * home =  (CONST char*)getenv("HOME");
	if (!home)
	    home = "/tmp";
#endif /* VMS */
	result = (char *)malloc(
		strlen(home)+strlen(acc_method)+strlen(host)+strlen(path)+6+1);
	if (result == NULL)
	    outofmem(__FILE__, "HTLocalName");
	sprintf(result, "%s/WWW/%s/%s%s", home, acc_method, host, path);
	FREE(path);
	FREE(acc_method);
	FREE(host);
	return result;
    }
}

/*	Make a WWW name from a full local path name.
**	--------------------------------------------
**
**  Bugs:
**	At present, only the names of two network root nodes are hand-coded
**	in and valid for the NeXT only. This should be configurable in
**	the general case.
*/
PUBLIC char * WWW_nameOfFile ARGS1(
	CONST char *,	name)
{
    char * result;
#ifdef NeXT
    if (0 == strncmp("/private/Net/", name, 13)) {
	result = (char *)malloc(7+strlen(name+13)+1);
	if (result == NULL)
	    outofmem(__FILE__, "WWW_nameOfFile");
	sprintf(result, "file://%s", name+13);
    } else
#endif /* NeXT */
    if (0 == strncmp(HTMountRoot, name, 5)) {
	result = (char *)malloc(7+strlen(name+5)+1);
	if (result == NULL)
	    outofmem(__FILE__, "WWW_nameOfFile");
	sprintf(result, "file://%s", name+5);
    } else {
	result = (char *)malloc(7+strlen(HTHostName())+strlen(name)+1);
	if (result == NULL)
	    outofmem(__FILE__, "WWW_nameOfFile");
	sprintf(result, "file://%s%s", HTHostName(), name);
    }
    CTRACE(tfp, "File `%s'\n\tmeans node `%s'\n", name, result);
    return result;
}

/*	Determine a suitable suffix, given the representation.
**	------------------------------------------------------
**
**  On entry,
**	rep	is the atomized MIME style representation
**	enc	is an encoding, trivial (8bit, binary, etc.) or gzip etc.
**
**  On exit:
**	Returns a pointer to a suitable suffix string if one has been
**	found, else "".
*/
PUBLIC CONST char * HTFileSuffix ARGS2(
	HTAtom*,	rep,
	CONST char *,	enc)
{
    HTSuffix * suff;
#ifdef FNAMES_8_3
    HTSuffix * first_found = NULL;
#endif
    BOOL trivial_enc;
    int n;
    int i;

#define NO_INIT  /* don't init anymore since I do it in Lynx at startup */
#ifndef NO_INIT
    if (!HTSuffixes)
	HTFileInit();
#endif /* !NO_INIT */

    trivial_enc = IsUnityEncStr(enc);
    n = HTList_count(HTSuffixes);
    for (i = 0; i < n; i++) {
	suff = (HTSuffix *)HTList_objectAt(HTSuffixes, i);
	if (suff->rep == rep &&
#if defined(VMS) || defined(FNAMES_8_3)
	    /*	Don't return a suffix whose first char is a dot, and which
		has more dots or asterisks after that, for
		these systems - kw */
	    (!suff->suffix || !suff->suffix[0] || suff->suffix[0] != '.' ||
	     (strchr(suff->suffix + 1, '.') == NULL &&
	      strchr(suff->suffix + 1, '*') == NULL)) &&
#endif
	    ((trivial_enc && IsUnityEnc(suff->encoding)) ||
	     (!trivial_enc && !IsUnityEnc(suff->encoding) &&
	      strcmp(enc, HTAtom_name(suff->encoding)) == 0))) {
#ifdef FNAMES_8_3
	    if (suff->suffix && (strlen(suff->suffix) <= 4)) {
		/*
		 *  If length of suffix (including dot) is 4 or smaller,
		 *  return this one even if we found a longer one
		 *  earlier - kw
		 */
		return suff->suffix;
	    } else if (!first_found) {
		first_found = suff;		/* remember this one */
	    }
#else
	    return suff->suffix;		/* OK -- found */
#endif
	}
    }
#ifdef FNAMES_8_3
    if (first_found)
	return first_found->suffix;
#endif
    return "";		/* Dunno */
}

/*	Determine file format from file name.
**	-------------------------------------
**
**	This version will return the representation and also set
**	a variable for the encoding.
**
**	Encoding may be a unity encoding (binary, 8bit, etc.) or
**	a content-coding like gzip, compress.
**
**	It will handle for example  x.txt, x.txt,Z, x.Z
*/
PUBLIC HTFormat HTFileFormat ARGS3(
	CONST char *,	filename,
	HTAtom **,	pencoding,
	CONST char**,	pdesc)
{
    HTSuffix * suff;
    int n;
    int i;
    int lf;
#ifdef VMS
    char *semicolon = NULL;
#endif /* VMS */

    if (pencoding)
	*pencoding = NULL;
    if (pdesc)
	*pdesc = NULL;
    if (LYforce_HTML_mode) {
	if (pencoding)
	    *pencoding = WWW_ENC_8BIT;
	return WWW_HTML;
    }

#ifdef VMS
    /*
    **	Trim at semicolon if a version number was
    **	included, so it doesn't interfere with the
    **	code for getting the MIME type. - FM
    */
    if ((semicolon = strchr(filename, ';')) != NULL)
	*semicolon = '\0';
#endif /* VMS */

#ifndef NO_INIT
    if (!HTSuffixes)
	HTFileInit();
#endif /* !NO_INIT */
    lf	= strlen(filename);
    n = HTList_count(HTSuffixes);
    for (i = 0; i < n; i++) {
	int ls;
	suff = (HTSuffix *)HTList_objectAt(HTSuffixes, i);
	ls = strlen(suff->suffix);
	if ((ls <= lf) && 0 == strcasecomp(suff->suffix, filename + lf - ls)) {
	    int j;
	    if (pencoding)
		*pencoding = suff->encoding;
	    if (pdesc)
		*pdesc = suff->desc;
	    if (suff->rep) {
#ifdef VMS
		if (semicolon != NULL)
		    *semicolon = ';';
#endif /* VMS */
		return suff->rep;		/* OK -- found */
	    }
	    for (j = 0; j < n; j++) {  /* Got encoding, need representation */
		int ls2;
		suff = (HTSuffix *)HTList_objectAt(HTSuffixes, j);
		ls2 = strlen(suff->suffix);
		if ((ls + ls2 <= lf) && 0 == strncasecomp(
			suff->suffix, filename + lf - ls -ls2, ls2)) {
		    if (suff->rep) {
			if (pdesc && !(*pdesc))
			    *pdesc = suff->desc;
#ifdef VMS
			if (semicolon != NULL)
			    *semicolon = ';';
#endif /* VMS */
			return suff->rep;
		    }
		}
	    }

	}
    }

    /* defaults tree */

    suff = strchr(filename, '.') ?	/* Unknown suffix */
	 ( unknown_suffix.rep ? &unknown_suffix : &no_suffix)
	 : &no_suffix;

    /*
    **	Set default encoding unless found with suffix already.
    */
    if (pencoding && !*pencoding)
	*pencoding = suff->encoding ? suff->encoding
				    : HTAtom_for("binary");
#ifdef VMS
    if (semicolon != NULL)
	*semicolon = ';';
#endif /* VMS */
    return suff->rep ? suff->rep : WWW_BINARY;
}

/*	Revise the file format in relation to the Lynx charset. - FM
**	-------------------------------------------------------
**
**	This checks the format associated with an anchor for
**	an extended MIME Content-Type, and if a charset is
**	indicated, sets Lynx up for proper handling in relation
**	to the currently selected character set. - FM
*/
PUBLIC HTFormat HTCharsetFormat ARGS3(
	HTFormat,		format,
	HTParentAnchor *,	anchor,
	int,			default_LYhndl)
{
    char *cp = NULL, *cp1, *cp2, *cp3 = NULL, *cp4;
    BOOL chartrans_ok = FALSE;
    int chndl = -1;

    FREE(anchor->charset);
    StrAllocCopy(cp, format->name);
    LYLowerCase(cp);
    if (((cp1 = strchr(cp, ';')) != NULL) &&
	(cp2 = strstr(cp1, "charset")) != NULL) {
	CTRACE(tfp, "HTCharsetFormat: Extended MIME Content-Type is %s\n",
		    format->name);
	cp2 += 7;
	while (*cp2 == ' ' || *cp2 == '=')
	    cp2++;
	StrAllocCopy(cp3, cp2); /* copy to mutilate more */
	for (cp4 = cp3; (*cp4 != '\0' && *cp4 != '"' &&
			 *cp4 != ';'  && *cp4 != ':' &&
			 !WHITE(*cp4)); cp4++) {
	    ; /* do nothing */
	}
	*cp4 = '\0';
	cp4 = cp3;
	chndl = UCGetLYhndl_byMIME(cp3);
	if (UCCanTranslateFromTo(chndl, current_char_set)) {
	    chartrans_ok = YES;
	    *cp1 = '\0';
	    format = HTAtom_for(cp);
	    StrAllocCopy(anchor->charset, cp4);
	    HTAnchor_setUCInfoStage(anchor, chndl,
				    UCT_STAGE_MIME,
				    UCT_SETBY_MIME);
	} else if (chndl < 0) {
	    /*
	    **	Got something but we don't recognize it.
	    */
	    chndl = UCLYhndl_for_unrec;
	    if (UCCanTranslateFromTo(chndl, current_char_set)) {
		chartrans_ok = YES;
		HTAnchor_setUCInfoStage(anchor, chndl,
					UCT_STAGE_MIME,
					UCT_SETBY_DEFAULT);
	    }
	}
	if (chartrans_ok) {
	    LYUCcharset *p_in = HTAnchor_getUCInfoStage(anchor,
							UCT_STAGE_MIME);
	    LYUCcharset *p_out = HTAnchor_setUCInfoStage(anchor,
							 current_char_set,
							 UCT_STAGE_HTEXT,
							 UCT_SETBY_DEFAULT);
	    if (!p_out) {
		/*
		**  Try again.
		*/
		p_out = HTAnchor_getUCInfoStage(anchor, UCT_STAGE_HTEXT);
	    }
	    if (!strcmp(p_in->MIMEname, "x-transparent")) {
		HTPassEightBitRaw = TRUE;
		HTAnchor_setUCInfoStage(anchor,
					HTAnchor_getUCLYhndl(anchor,
							     UCT_STAGE_HTEXT),
					UCT_STAGE_MIME,
					UCT_SETBY_DEFAULT);
	    }
	    if (!strcmp(p_out->MIMEname, "x-transparent")) {
		HTPassEightBitRaw = TRUE;
		HTAnchor_setUCInfoStage(anchor,
					HTAnchor_getUCLYhndl(anchor,
							     UCT_STAGE_MIME),
					UCT_STAGE_HTEXT,
					UCT_SETBY_DEFAULT);
	    }
	    if (p_in->enc != UCT_ENC_CJK) {
		HTCJK = NOCJK;
		if (!(p_in->codepoints &
		      UCT_CP_SUBSETOF_LAT1) &&
		    chndl == current_char_set) {
		    HTPassEightBitRaw = TRUE;
		}
	    } else if (p_out->enc == UCT_ENC_CJK) {
		if (LYRawMode) {
		    if ((!strcmp(p_in->MIMEname, "euc-jp") ||
			 !strcmp(p_in->MIMEname, "shift_jis")) &&
			(!strcmp(p_out->MIMEname, "euc-jp") ||
			 !strcmp(p_out->MIMEname, "shift_jis"))) {
			HTCJK = JAPANESE;
		    } else if (!strcmp(p_in->MIMEname, "euc-cn") &&
			       !strcmp(p_out->MIMEname, "euc-cn")) {
			HTCJK = CHINESE;
		    } else if (!strcmp(p_in->MIMEname, "big-5") &&
			       !strcmp(p_out->MIMEname, "big-5")) {
			HTCJK = TAIPEI;
		    } else if (!strcmp(p_in->MIMEname, "euc-kr") &&
			       !strcmp(p_out->MIMEname, "euc-kr")) {
			HTCJK = KOREAN;
		    } else {
			HTCJK = NOCJK;
		    }
		} else {
		    HTCJK = NOCJK;
		}
	    }
	/*
	**  Check for an iso-8859-# we don't know. - FM
	*/
	} else if (!strncmp(cp4, "iso-8859-", 9) &&
		   isdigit((unsigned char)cp4[9]) &&
		   !strncmp(LYchar_set_names[current_char_set],
			    "Other ISO Latin", 15)) {
	    /*
	    **	Hope it's a match, for now. - FM
	    */
	    *cp1 = '\0';
	    format = HTAtom_for(cp);
	    cp1 = &cp4[10];
	    while (*cp1 &&
		   isdigit((unsigned char)(*cp1)))
		cp1++;
	    *cp1 = '\0';
	    StrAllocCopy(anchor->charset, cp4);
	    HTPassEightBitRaw = TRUE;
	    HTAlert(anchor->charset);
	}
	FREE(cp3);
    } else if (cp1 != NULL) {
	/*
	**  No charset parameter is present.
	**  Ignore all other parameters, as
	**  we do when charset is present. - FM
	*/
	*cp1 = '\0';
	format = HTAtom_for(cp);
    }
    FREE(cp);

    /*
    **	Set up defaults, if needed. - FM
    */
    if (!chartrans_ok && !anchor->charset && default_LYhndl >= 0) {
	HTAnchor_setUCInfoStage(anchor, default_LYhndl,
				UCT_STAGE_MIME,
				UCT_SETBY_DEFAULT);
    }
    HTAnchor_copyUCInfoStage(anchor,
			    UCT_STAGE_PARSER,
			    UCT_STAGE_MIME,
			    -1);

    return format;
}

/*	Determine value from file name.
**	-------------------------------
**
*/
PUBLIC float HTFileValue ARGS1(
	CONST char *,	filename)
{
    HTSuffix * suff;
    int n;
    int i;
    int lf = strlen(filename);

#ifndef NO_INIT
    if (!HTSuffixes)
	HTFileInit();
#endif /* !NO_INIT */
    n = HTList_count(HTSuffixes);
    for (i = 0; i < n; i++) {
	int ls;
	suff = (HTSuffix *)HTList_objectAt(HTSuffixes, i);
	ls = strlen(suff->suffix);
	if ((ls <= lf) && 0==strcmp(suff->suffix, filename + lf - ls)) {
	    CTRACE(tfp, "File: Value of %s is %.3f\n",
			filename, suff->quality);
	    return suff->quality;		/* OK -- found */
	}
    }
    return 0.3; 	/* Dunno! */
}

/*	Determine write access to a file.
**	---------------------------------
**
**  On exit:
**	Returns YES if file can be accessed and can be written to.
**
**  Bugs:
**	1.	No code for non-unix systems.
**	2.	Isn't there a quicker way?
*/

#if defined(HAVE_CONFIG_H)

#ifndef HAVE_GETGROUPS
#define NO_GROUPS
#endif

#else

#ifdef VMS
#define NO_GROUPS
#endif /* VMS */
#ifdef NO_UNIX_IO
#define NO_GROUPS
#endif /* NO_UNIX_IO */
#ifdef PCNFS
#define NO_GROUPS
#endif /* PCNFS */
#ifdef NOUSERS
#define NO_GROUPS
#endif /* PCNFS */

#endif	/* HAVE_CONFIG_H */

PUBLIC BOOL HTEditable ARGS1(
	CONST char *,	filename)
{
#ifdef NO_GROUPS
    return NO;		/* Safe answer till we find the correct algorithm */
#else
#ifdef NeXT
    int 	groups[NGROUPS];
#else
    gid_t	groups[NGROUPS];
#endif /* NeXT */
    uid_t	myUid;
    int 	ngroups;			/* The number of groups  */
    struct stat fileStatus;
    int 	i;

    if (stat(filename, &fileStatus))		/* Get details of filename */
	return NO;				/* Can't even access file! */

    ngroups = getgroups(NGROUPS, groups);	/* Groups to which I belong  */
    myUid = geteuid();				/* Get my user identifier */

    if (TRACE) {
	int i2;
	fprintf(tfp,
	    "File mode is 0%o, uid=%d, gid=%d. My uid=%d, %d groups (",
	    (unsigned int) fileStatus.st_mode, fileStatus.st_uid,
	    (int) fileStatus.st_gid,
	    (int) myUid,
	    (int) ngroups);
	for (i2 = 0; i2 < ngroups; i2++)
	    fprintf(tfp, " %d", groups[i2]);
	fprintf(tfp, ")\n");
    }

    if (fileStatus.st_mode & 0002)		/* I can write anyway? */
	return YES;

    if ((fileStatus.st_mode & 0200)		/* I can write my own file? */
     && (fileStatus.st_uid == myUid))
	return YES;

    if (fileStatus.st_mode & 0020)		/* Group I am in can write? */
    {
	for (i = 0; i < ngroups; i++) {
	    if (groups[i] == fileStatus.st_gid)
		return YES;
	}
    }
    CTRACE(tfp, "\tFile is not editable.\n");
    return NO;					/* If no excuse, can't do */
#endif /* NO_GROUPS */
}

/*	Make a save stream.
**	-------------------
**
**	The stream must be used for writing back the file.
**	@@@ no backup done
*/
PUBLIC HTStream * HTFileSaveStream ARGS1(
	HTParentAnchor *,	anchor)
{
    CONST char * addr = HTAnchor_address((HTAnchor*)anchor);
    char *  localname = HTLocalName(addr);

    FILE* fp = fopen(localname, "w");
    if (!fp)
	return NULL;

    return HTFWriter_new(fp);
}

/*	Output one directory entry.
**	---------------------------
*/
PUBLIC void HTDirEntry ARGS3(
	HTStructured *, target,
	CONST char *,	tail,
	CONST char *,	entry)
{
    char * relative = NULL;
    char * escaped = NULL;
    int len;

    if (0 == strcmp(entry,"../")) {
	/*
	**  Undo slash appending for anchor creation.
	*/
	StrAllocCopy(escaped,"..");
    } else {
	escaped = HTEscape(entry, URL_XPALPHAS);
	if (((len = strlen(escaped)) > 2) &&
	    escaped[(len - 3)] == '%' &&
	    escaped[(len - 2)] == '2' &&
	    TOUPPER(escaped[(len - 1)]) == 'F') {
	    escaped[(len - 3)] = '\0';
	}
    }

    if (tail == NULL || *tail == '\0') {
	/*
	**  Handle extra slash at end of path.
	*/
	HTStartAnchor(target, NULL, (escaped[0] != '\0' ? escaped : "/"));
    } else {
	/*
	**  If empty tail, gives absolute ref below.
	*/
	relative = (char*)malloc(strlen(tail) + strlen(escaped)+2);
	if (relative == NULL)
	    outofmem(__FILE__, "HTDirEntry");
	sprintf(relative, "%s%s%s",
			   tail,
			   (*escaped != '\0' ? "/" : ""),
			   escaped);
	HTStartAnchor(target, NULL, relative);
	FREE(relative);
    }
    FREE(escaped);
}

/*	Output parent directory entry.
**	------------------------------
**
**    This gives the TITLE and H1 header, and also a link
**    to the parent directory if appropriate.
**
**  On exit:
**	Returns TRUE if an "Up to <parent>" link was not created
**	for a readable local directory because LONG_LIST is defined
**	and NO_PARENT_DIR_REFERENCE is not defined, such that the
**	calling function use LYListFmtParse() to create a link to
**	the parent directory.  Otherwise, it returns FALSE. - FM
*/
PUBLIC BOOL HTDirTitles ARGS3(
	HTStructured *, target,
	HTAnchor *,	anchor,
	BOOL,		tildeIsTop)
{
    char * logical = HTAnchor_address(anchor);
    char * path = HTParse(logical, "", PARSE_PATH + PARSE_PUNCTUATION);
    char * current;
    char * cp = NULL;
    BOOL need_parent_link = FALSE;
    int i;

#ifdef DOSPATH
    BOOL local_link = FALSE;
    if (logical[18] == ':') local_link = TRUE;
#endif
    /*
    **	Check tildeIsTop for treating home directory as Welcome
    **	(assume the tilde is not followed by a username). - FM
    */
    if (tildeIsTop && !strncmp(path, "/~", 2)) {
	if (path[2] == '\0') {
	    path[1] = '\0';
	} else {
	    for (i = 0; path[(i + 2)]; i++) {
		path[i] = path[(i + 2)];
	    }
	    path[i] = '\0';
	}
    }

    /*
    **	Trim out the ;type= parameter, if present. - FM
    */
    if ((cp = strrchr(path, ';')) != NULL) {
	if (!strncasecomp((cp+1), "type=", 5)) {
	    if (TOUPPER(*(cp+6)) == 'D' ||
		TOUPPER(*(cp+6)) == 'A' ||
		TOUPPER(*(cp+6)) == 'I')
		*cp = '\0';
	}
	cp = NULL;
    }
    current = strrchr(path, '/');	/* last part or "" */

    {
      char * printable = NULL;

#ifdef DIRED_SUPPORT
      if (0 == strncasecomp(path, "/%2F", 4))
	  StrAllocCopy(printable, (path+1));
      else
	  StrAllocCopy(printable, path);
      if (0 == strncasecomp(printable, "/vmsysu%2b", 10) ||
	  0 == strncasecomp(printable, "/anonymou.", 10)) {
	  StrAllocCopy(cp, (printable+1));
	  StrAllocCopy(printable, cp);
	  FREE(cp);
      }
#else
      StrAllocCopy(printable, (current ? current + 1 : ""));
#endif /* DIRED_SUPPORT */

      START(HTML_HEAD);
      PUTS("\n");
      HTUnEscape(printable);
      START(HTML_TITLE);
      PUTS(*printable ? printable : "Welcome");
      PUTS(" directory");
      END(HTML_TITLE);
      PUTS("\n");
      END(HTML_HEAD);
      PUTS("\n");

#ifdef DIRED_SUPPORT
      START(HTML_H2);
      PUTS(*printable ? "Current directory is " : "");
      PUTS(*printable ? printable : "Welcome");
      END(HTML_H2);
      PUTS("\n");
#else
      START(HTML_H1);
      PUTS(*printable ? printable : "Welcome");
      END(HTML_H1);
      PUTS("\n");
#endif /* DIRED_SUPPORT */
      if (((0 == strncasecomp(printable, "vmsysu:", 7)) &&
	   (cp = strchr(printable, '.')) != NULL &&
	   strchr(cp, '/') == NULL) ||
	  (0 == strncasecomp(printable, "anonymou.", 9) &&
	   strchr(printable, '/') == NULL)) {
	  FREE(printable);
	  FREE(logical);
	  FREE(path);
	  return(need_parent_link);
      }
      FREE(printable);
    }

#ifndef NO_PARENT_DIR_REFERENCE
    /*
    **	Make link back to parent directory.
    */
    if (current && current[1]) {   /* was a slash AND something else too */
	char * parent = NULL;
	char * relative = NULL;

	*current++ = '\0';
	parent = strrchr(path, '/');  /* penultimate slash */

	if ((parent &&
	     (!strcmp(parent, "/..") ||
	      !strncasecomp(parent, "/%2F", 4))) ||
	    !strncasecomp(current, "%2F", 3)) {
	    FREE(logical);
	    FREE(path);
	    return(need_parent_link);
	}

	relative = (char*) malloc(strlen(current) + 4);
	if (relative == NULL)
	    outofmem(__FILE__, "HTDirTitles");
	sprintf(relative, "%s/..", current);

#ifdef DOSPATH
	if (local_link)
	    if (strlen(parent) == 3 )
		StrAllocCat(relative, "/.");
#endif

#if !defined (VMS)
#ifdef DOSPATH
	if(!local_link)
#endif
	{
	    /*
	    **	On Unix, if it's not ftp and the directory cannot
	    **	be read, don't put out a link.
	    **
	    **	On VMS, this problem is dealt with internally by
	    **	HTVMSBrowseDir().
	    */
	    DIR  * dp = NULL;

	    if (LYisLocalFile(logical)) {
		/*
		**  We need an absolute file path for the opendir.
		**  We also need to unescape for this test.
		**  Don't worry about %2F now, they presumably have been
		**  dealt with above, and shouldn't appear for local
		**  files anyway...  Assume OS / filesystem will just
		**  ignore superfluous slashes. - KW
		*/
		char * fullparentpath = NULL;

		/*
		**  Path has been shortened above.
		*/
		StrAllocCopy(fullparentpath, *path ? path : "/");

		/*
		**  Guard against weirdness.
		*/
		if (0 == strcmp(current,"..")) {
		    StrAllocCat(fullparentpath,"/../..");
		} else if (0 == strcmp(current,".")) {
		    StrAllocCat(fullparentpath,"/..");
		}

		HTUnEscape(fullparentpath);
		if ((dp = opendir(fullparentpath)) == NULL) {
		    FREE(fullparentpath);
		    FREE(logical);
		    FREE(relative);
		    FREE(path);
		    return(need_parent_link);
		}
		closedir(dp);
		FREE(fullparentpath);
#ifdef LONG_LIST
		need_parent_link = TRUE;
		FREE(logical);
		FREE(path);
		FREE(relative);
		return(need_parent_link);
#endif /* LONG_LIST */
	    }
	}
#endif /* !VMS */
	HTStartAnchor(target, "", relative);
	FREE(relative);

	PUTS("Up to ");
	if (parent) {
	    if ((0 == strcmp(current,".")) ||
		(0 == strcmp(current,".."))) {
		/*
		**  Should not happen, but if it does,
		**  at least avoid giving misleading info. - KW
		*/
		PUTS("..");
	    } else {
		char * printable = NULL;
		StrAllocCopy(printable, parent + 1);
		HTUnEscape(printable);
		PUTS(printable);
		FREE(printable);
	    }
	} else {
	    PUTS("/");
	}
	END(HTML_A);
    }
#endif /* !NO_PARENT_DIR_REFERENCE */

    FREE(logical);
    FREE(path);
    return(need_parent_link);
}

/*	Load a document.
**	----------------
**
**  On entry:
**	addr		must point to the fully qualified hypertext reference.
**			This is the physical address of the file
**
**  On exit:
**	returns 	<0		Error has occurred.
**			HTLOADED	OK
**
*/
PUBLIC int HTLoadFile ARGS4(
	CONST char *,		addr,
	HTParentAnchor *,	anchor,
	HTFormat,		format_out,
	HTStream *,		sink)
{
    char * filename = NULL;
    char * acc_method = NULL;
    HTFormat format;
    char * nodename = NULL;
    char * newname = NULL;	/* Simplified name of file */
    HTAtom * encoding;		/* @@ not used yet */
    HTAtom * myEncoding = NULL; /* enc of this file, may be gzip etc. */
    int status;
#ifdef VMS
    struct stat stat_info;
#endif /* VMS */
#ifdef USE_ZLIB
    gzFile gzfp = 0;
    BOOL use_gzread = NO;
#endif /* USE_ZLIB */

    /*
    **	Reduce the filename to a basic form (hopefully unique!).
    */
    StrAllocCopy(newname, addr);
    filename=HTParse(newname, "", PARSE_PATH|PARSE_PUNCTUATION);
    nodename=HTParse(newname, "", PARSE_HOST);

    /*
    **	If access is ftp, or file is on another host, invoke ftp now.
    */
    acc_method = HTParse(newname, "", PARSE_ACCESS);
    if (strcmp("ftp", acc_method) == 0 ||
       (strcmp("localhost", nodename) != 0 &&
#ifdef VMS
	strcasecomp(nodename, HTHostName()) != 0
#else
	strcmp(nodename, HTHostName()) != 0
#endif /* VMS */
    )) {
	FREE(newname);
	FREE(filename);
	FREE(nodename);
	FREE(acc_method);
	return HTFTPLoad(addr, anchor, format_out, sink);
    } else {
	FREE(newname);
	FREE(acc_method);
    }
#ifdef VMS
    HTUnEscape(filename);
#endif /* VMS */

    /*
    **	Determine the format and encoding mapped to any suffix.
    */
    if (anchor->content_type && anchor->content_encoding) {
	/*
	 *  If content_type and content_encoding are BOTH already set
	 *  in the anchor object, we believe it and don't try to
	 *  derive format and encoding from the filename. - kw
	 */
	format = HTAtom_for(anchor->content_type);
	myEncoding = HTAtom_for(anchor->content_encoding);
    } else {
	format = HTFileFormat(filename, &myEncoding, NULL);

    /*
    **	Check the format for an extended MIME charset value, and
    **	act on it if present.  Otherwise, assume what is indicated
    **	by the last parameter (fallback will effectively be
    **	UCLYhndl_for_unspec, by default ISO-8859-1). - kw
    */
	format = HTCharsetFormat(format, anchor, UCLYhndl_HTFile_for_unspec);
    }

#ifdef VMS
    /*
    **	Check to see if the 'filename' is in fact a directory.	If it is
    **	create a new hypertext object containing a list of files and
    **	subdirectories contained in the directory.  All of these are links
    **	to the directories or files listed.
    */
    if (HTStat(filename, &stat_info) == -1) {
	CTRACE(tfp, "HTLoadFile: Can't stat %s\n", filename);
    } else {
	if (((stat_info.st_mode) & S_IFMT) == S_IFDIR) {
	    if (HTDirAccess == HT_DIR_FORBID) {
		FREE(filename);
		FREE(nodename);
		return HTLoadError(sink, 403,
		"Directory browsing is not allowed.");
	    }

	    if (HTDirAccess == HT_DIR_SELECTIVE) {
		char * enable_file_name =
		    malloc(strlen(filename)+ 1 +
		    strlen(HT_DIR_ENABLE_FILE) + 1);
		if (enable_file_name == NULL)
		    outofmem(__FILE__, "HTLoadFile");
		strcpy(enable_file_name, filename);
		strcat(enable_file_name, "/");
		strcat(enable_file_name, HT_DIR_ENABLE_FILE);
		if (HTStat(enable_file_name, &stat_info) == -1) {
		    FREE(filename);
		    FREE(nodename);
		    return HTLoadError(sink, 403,
		    "Selective access is not enabled for this directory");
		}
	    }

	    FREE(filename);
	    FREE(nodename);
	    return HTVMSBrowseDir(addr, anchor, format_out, sink);
	}
    }

    /*
    **	Assume that the file is in Unix-style syntax if it contains a '/'
    **	after the leading one. @@
    */
    {
	FILE * fp;
	char * vmsname = strchr(filename + 1, '/') ?
		    HTVMS_name(nodename, filename) : filename + 1;
	fp = fopen(vmsname, "r", "shr=put", "shr=upd");

	/*
	**  If the file wasn't VMS syntax, then perhaps it is Ultrix.
	*/
	if (!fp) {
	    char ultrixname[INFINITY];
	    CTRACE(tfp, "HTLoadFile: Can't open as %s\n", vmsname);
	    sprintf(ultrixname, "%s::\"%s\"", nodename, filename);
	    fp = fopen(ultrixname, "r", "shr=put", "shr=upd");
	    if (!fp) {
		CTRACE(tfp, "HTLoadFile: Can't open as %s\n",
			    ultrixname);
	    }
	}
	if (fp) {
	    int len;
	    char *cp = NULL;
	    char *semicolon = NULL;

	    if (HTEditable(vmsname)) {
		HTAtom * put = HTAtom_for("PUT");
		HTList * methods = HTAnchor_methods(anchor);
		if (HTList_indexOf(methods, put) == (-1)) {
		    HTList_addObject(methods, put);
		}
	    }
	    /*
	    **	Trim vmsname at semicolon if a version number was
	    **	included, so it doesn't interfere with the check
	    **	for a compressed file. - FM
	    */
	    if ((semicolon = strchr(vmsname, ';')) != NULL)
		*semicolon = '\0';
	    /*
	    **	Fake a Content-Encoding for compressed files. - FM
	    */
	    if (!IsUnityEnc(myEncoding)) {
		/*
		 *  We already know from the call to HTFileFormat above
		 *  that this is a compressed file, no need to look at
		 *  the filename again. - kw
		 */
#ifdef USE_ZLIB
		if (strcmp(format_out->name, "www/download") != 0 &&
		    (!strcmp(HTAtom_name(myEncoding), "gzip") ||
		     !strcmp(HTAtom_name(myEncoding), "x-gzip"))) {
		    fclose(fp);
		    if (semicolon != NULL)
			*semicolon = ';';
		    gzfp = gzopen(vmsname, "rb");

		    CTRACE(tfp, "HTLoadFile: gzopen of `%s' gives %p\n",
				vmsname, (void*)gzfp);
		    use_gzread = YES;
		} else
#endif	/* USE_ZLIB */
		{
		    StrAllocCopy(anchor->content_type, format->name);
		    StrAllocCopy(anchor->content_encoding, HTAtom_name(myEncoding));
		    format = HTAtom_for("www/compressed");
		}
	    } else if ((len = strlen(vmsname)) > 2) {
		if ((vmsname[len - 1] == 'Z') &&
		    (vmsname[len - 2] == '.' ||
		     vmsname[len - 2] == '-' ||
		     vmsname[len - 2] == '_') &&
		    vmsname[len - 3] != ']' &&
		    vmsname[len - 3] != ':') {
		    StrAllocCopy(cp, vmsname);
		    cp[len - 2] = '\0';
		    format = HTFileFormat(cp, &encoding, NULL);
		    FREE(cp);
		    format = HTCharsetFormat(format, anchor,
					     UCLYhndl_HTFile_for_unspec);
		    StrAllocCopy(anchor->content_type, format->name);
		    StrAllocCopy(anchor->content_encoding, "x-compress");
		    format = HTAtom_for("www/compressed");
		} else if ((len > 3) &&
			   !strcasecomp((char *)&vmsname[len - 2], "gz")) {
		    if (vmsname[len - 3] == '.' ||
			vmsname[len - 3] == '-' ||
			vmsname[len - 3] == '_') {
			StrAllocCopy(cp, vmsname);
			cp[len - 3] = '\0';
			format = HTFileFormat(cp, &encoding, NULL);
			FREE(cp);
			format = HTCharsetFormat(format, anchor,
						 UCLYhndl_HTFile_for_unspec);
			StrAllocCopy(anchor->content_type, format->name);
			StrAllocCopy(anchor->content_encoding, "x-gzip");
#ifdef USE_ZLIB
			if (strcmp(format_out->name, "www/download") != 0) {
			    fclose(fp);
			    if (semicolon != NULL)
				*semicolon = ';';
			    gzfp = gzopen(vmsname, "rb");

			    CTRACE(tfp, "HTLoadFile: gzopen of `%s' gives %p\n",
					vmsname, (void*)gzfp);
			    use_gzread = YES;
			}
#else  /* USE_ZLIB */
			format = HTAtom_for("www/compressed");
#endif	/* USE_ZLIB */
		    }
		}
	    }
	    if (semicolon != NULL)
		*semicolon = ';';
	    FREE(filename);
	    FREE(nodename);
#ifdef USE_ZLIB
	    if (use_gzread) {
		if (gzfp) {
		    char * sugfname = NULL;
		    if (anchor->SugFname) {
			StrAllocCopy(sugfname, anchor->SugFname);
		    } else {
			char * anchor_path = HTParse(anchor->address, "",
						     PARSE_PATH + PARSE_PUNCTUATION);
			char * lastslash;
			HTUnEscape(anchor_path);
			lastslash = strrchr(anchor_path, '/');
			if (lastslash)
			    StrAllocCopy(sugfname, lastslash + 1);
			FREE(anchor_path);
		    }
		    FREE(anchor->content_encoding);
		    if (sugfname && *sugfname)
			HTCheckFnameForCompression(&sugfname, anchor,
						   TRUE);
		    if (sugfname && *sugfname)
			StrAllocCopy(anchor->SugFname, sugfname);
		    FREE(sugfname);
		    status = HTParseGzFile(format, format_out,
					   anchor,
					   gzfp, sink);
		} else {
		    status = HTLoadError(NULL,
					 -(HT_ERROR),
					 "Could not open file for decompression!");
		}
	    } else
#endif /* USE_ZLIB */
	    {
		status = HTParseFile(format, format_out, anchor, fp, sink);
		fclose(fp);
	    }
	    return status;
	}  /* If successful open */
	FREE(filename);
    }

#else /* Unix: */

    FREE(filename);

    /*
    **	For unix, we try to translate the name into the name of a
    **	transparently mounted file.
    **
    **	Not allowed in secure (HTClienntHost) situations. TBL 921019
    */
#ifndef NO_UNIX_IO
    /*	Need protection here for telnet server but not httpd server. */

    if (!HTSecure) {		/* try local file system */
	char * localname = HTLocalName(addr);
	struct stat dir_info;

#ifdef HAVE_READDIR
	/*
	**  Multiformat handling.
	**
	**  If needed, scan directory to find a good file.
	**  Bug:  We don't stat the file to find the length.
	*/
	if ((strlen(localname) > strlen(MULTI_SUFFIX)) &&
	    (0 == strcmp(localname + strlen(localname) - strlen(MULTI_SUFFIX),
			 MULTI_SUFFIX))) {
	    DIR *dp = 0;
	    BOOL forget_multi = NO;

	    STRUCT_DIRENT * dirbuf;
	    float best = NO_VALUE_FOUND;	/* So far best is bad */
	    HTFormat best_rep = NULL;	/* Set when rep found */
	    HTAtom * best_enc = NULL;
	    char * best_name = NULL;	/* Best dir entry so far */

	    char *base = strrchr(localname, '/');
	    int baselen = 0;

	    if (!base || base == localname) {
		forget_multi = YES;
	    } else {
		*base++ = '\0'; 	/* Just got directory name */
		baselen = strlen(base)- strlen(MULTI_SUFFIX);
		base[baselen] = '\0';	/* Chop off suffix */

		dp = opendir(localname);
	    }
	    if (forget_multi || !dp) {
		FREE(localname);
		FREE(nodename);
		return HTLoadError(sink, 500,
			"Multiformat: directory scan failed.");
	    }

	    while ((dirbuf = readdir(dp)) != NULL) {
		/*
		**  While there are directory entries to be read...
		*/
#ifndef DOSPATH
		if (dirbuf->d_ino == 0)
		    continue;	/* if the entry is not being used, skip it */
#endif
		if ((int)strlen(dirbuf->d_name) > baselen &&	 /* Match? */
		    !strncmp(dirbuf->d_name, base, baselen)) {
		    HTAtom * enc;
		    HTFormat rep = HTFileFormat(dirbuf->d_name, &enc, NULL);
		    float filevalue = HTFileValue(dirbuf->d_name);
		    float value = HTStackValue(rep, format_out,
						filevalue,
						0L  /* @@@@@@ */);
		    if (value <= 0.0) {
			char * cp = NULL;
			int len = strlen(dirbuf->d_name);
			enc = NULL;
			if (len > 2 &&
			    dirbuf->d_name[len - 1] == 'Z' &&
			    dirbuf->d_name[len - 2] == '.') {
			    StrAllocCopy(cp, dirbuf->d_name);
			    cp[len - 2] = '\0';
			    format = HTFileFormat(cp, NULL, NULL);
			    FREE(cp);
			    value = HTStackValue(format, format_out,
						 filevalue, 0);
			    if (value <= 0.0) {
				format = HTAtom_for("application/x-compressed");
				value = HTStackValue(format, format_out,
						     filevalue, 0);
			    }
			    if (value <= 0.0) {
				format = HTAtom_for("www/compressed");
				value = HTStackValue(format, format_out,
						     filevalue, 0);
			    }
			} else if ((len > 3) &&
				   !strcasecomp((char *)&dirbuf->d_name[len - 2],
						"gz") &&
				   dirbuf->d_name[len - 3] == '.') {
			    StrAllocCopy(cp, dirbuf->d_name);
			    cp[len - 3] = '\0';
			    format = HTFileFormat(cp, NULL, NULL);
			    FREE(cp);
			    value = HTStackValue(format, format_out,
						 filevalue, 0);
			    if (value <= 0.0) {
				format = HTAtom_for("application/x-gzip");
				value = HTStackValue(format, format_out,
						     filevalue, 0);
			    }
			    if (value <= 0.0) {
				format = HTAtom_for("www/compressed");
				value = HTStackValue(format, format_out,
						     filevalue, 0);
			    }
			}
		    }
		    if (value != NO_VALUE_FOUND) {
			CTRACE(tfp, "HTLoadFile: value of presenting %s is %f\n",
				    HTAtom_name(rep), value);
			if  (value > best) {
			    best_rep = rep;
			    best_enc = enc;
			    best = value;
			    StrAllocCopy(best_name, dirbuf->d_name);
		       }
		    }	/* if best so far */
		 } /* if match */

	    } /* end while directory entries left to read */
	    closedir(dp);

	    if (best_rep) {
		format = best_rep;
		myEncoding = best_enc;
		base[-1] = '/'; 	/* Restore directory name */
		base[0] = '\0';
		StrAllocCat(localname, best_name);
		FREE(best_name);
	    } else {			/* If not found suitable file */
		FREE(localname);
		FREE(nodename);
		return HTLoadError(sink, 403,	/* List formats? */
		   "Could not find suitable representation for transmission.");
	    }
	    /*NOTREACHED*/
	} /* if multi suffix */

	/*
	**  Check to see if the 'localname' is in fact a directory.  If it
	**  is create a new hypertext object containing a list of files and
	**  subdirectories contained in the directory.	All of these are
	**  links to the directories or files listed.
	**  NB This assumes the existence of a type 'STRUCT_DIRENT', which
	**  will hold the directory entry, and a type 'DIR' which is used
	**  to point to the current directory being read.
	*/
#ifdef _WINDOWS
	if (!exists(localname))
#else
	if (stat(localname,&dir_info) == -1)	   /* get file information */
#endif
	{
				       /* if can't read file information */
	    CTRACE(tfp, "HTLoadFile: can't stat %s\n", localname);

	}  else {		/* Stat was OK */

#ifdef _WINDOWS
	    if (stat(localname,&dir_info) == -1) dir_info.st_mode = S_IFDIR;
#endif

	    if (((dir_info.st_mode) & S_IFMT) == S_IFDIR) {
		/*
		**  If localname is a directory.
		*/
		HTStructured *target;		/* HTML object */
		HTStructuredClass targetClass;
		DIR *dp;
		STRUCT_DIRENT * dirbuf;
		char *logical = NULL;
		char *pathname = NULL;
		char *tail = NULL;
		BOOL present[HTML_A_ATTRIBUTES];
		char * tmpfilename = NULL;
		BOOL need_parent_link = FALSE;
		struct stat file_info;

		CTRACE(tfp, "%s is a directory\n", localname);

		/*
		**  Check directory access.
		**  Selective access means only those directories containing
		**  a marker file can be browsed.
		*/
		if (HTDirAccess == HT_DIR_FORBID) {
		    FREE(localname);
		    FREE(nodename);
		    return HTLoadError(sink, 403,
		    "Directory browsing is not allowed.");
		}


		if (HTDirAccess == HT_DIR_SELECTIVE) {
		    char * enable_file_name =
			malloc(strlen(localname)+ 1 +
				      strlen(HT_DIR_ENABLE_FILE) + 1);
		    if (enable_file_name == NULL)
			outofmem(__FILE__, "HTLoadFile");
		    strcpy(enable_file_name, localname);
		    strcat(enable_file_name, "/");
		    strcat(enable_file_name, HT_DIR_ENABLE_FILE);
		    if (stat(enable_file_name, &file_info) != 0) {
			FREE(localname);
			FREE(nodename);
			return HTLoadError(sink, 403,
			"Selective access is not enabled for this directory");
		    }
		}

		dp = opendir(localname);
		if (!dp) {
		    FREE(localname);
		    FREE(nodename);
		    return HTLoadError(sink, 403,
				       "This directory is not readable.");
		}

		/*
		**  Directory access is allowed and possible.
		*/
		logical = HTAnchor_address((HTAnchor*)anchor);
		pathname = HTParse(logical, "",
					PARSE_PATH + PARSE_PUNCTUATION);

		if (!strcmp(pathname,"/")) {
		    /*
		    **	Root path.
		    */
		    StrAllocCopy (tail, "/foo/..");
		} else {
		    char *p = strrchr(pathname, '/');  /* find last slash */

		    if (!p) {
			/*
			**  This probably should not happen,
			**  but be prepared if it does. - KW
			*/
			StrAllocCopy (tail, "/foo/..");
		    } else {
			/*
			**  Take slash off the beginning.
			*/
			StrAllocCopy(tail, (p + 1));
		    }
		}
		FREE(pathname);

		if (UCLYhndl_HTFile_for_unspec >= 0) {
		    HTAnchor_setUCInfoStage(anchor,
					    UCLYhndl_HTFile_for_unspec,
					    UCT_STAGE_PARSER,
					    UCT_SETBY_DEFAULT);
		}

		target = HTML_new(anchor, format_out, sink);
		targetClass = *target->isa;	/* Copy routine entry points */

		{ int i;
			for (i = 0; i < HTML_A_ATTRIBUTES; i++)
				present[i] = (i == HTML_A_HREF);
		}

		/*
		**  The need_parent_link flag will be set if an
		**  "Up to <parent>" link was not created for a
		**  readable parent in HTDirTitles() because
		**  LONG_LIST is defined and NO_PARENT_DIR_REFERENCE
		**  is not defined so that need we to create the
		**  link via an LYListFmtParse() call. - FM
		*/
		need_parent_link = HTDirTitles(target,
					       (HTAnchor *)anchor, FALSE);

#ifdef DIRED_SUPPORT
		if (strncmp(anchor->address, "lynxcgi:", 8)) {
		    HTAnchor_setFormat((HTParentAnchor *) anchor, WWW_DIRED);
		    lynx_edit_mode = TRUE;
		}
#endif /* DIRED_SUPPORT */
		if (HTDirReadme == HT_DIR_README_TOP)
		    do_readme(target, localname);
		{
		    HTBTree * bt = HTBTree_new((HTComparer)strcmp);

		    while ((dirbuf = readdir(dp)) != NULL) {
			/*
			**  While there are directory entries to be read...
			*/
			char * dirname = NULL;

#ifndef DOSPATH
			if (dirbuf->d_ino == 0)
			    /*
			    **	If the entry is not being used, skip it.
			    */
			    continue;
#endif
			/*
			**  Skip self, parent if handled in HTDirTitles()
			**  or if NO_PARENT_DIR_REFERENCE is not defined,
			**  and any dot files if no_dotfiles is set or
			**  show_dotfiles is not set. - FM
			*/
			if (!strcmp(dirbuf->d_name, ".")   /* self   */ ||
			    (!strcmp(dirbuf->d_name, "..") /* parent */ &&
			     need_parent_link == FALSE) ||
			    ((strcmp(dirbuf->d_name, "..")) &&
			     (dirbuf->d_name[0] == '.' &&
			      (no_dotfiles || !show_dotfiles))))
			    continue;

			dirname = (char *)malloc(strlen(dirbuf->d_name) + 4);
			if (dirname == NULL)
			    outofmem(__FILE__, "HTLoadFile");
			StrAllocCopy(tmpfilename, localname);
			if (strcmp(localname, "/"))
			    /*
			    **	If filename is not root directory.
			    */
			    StrAllocCat(tmpfilename, "/");

			StrAllocCat(tmpfilename, dirbuf->d_name);
			stat(tmpfilename, &file_info);
			if (((file_info.st_mode) & S_IFMT) == S_IFDIR)
#ifndef DIRED_SUPPORT
			    sprintf((char *)dirname, "D%s",dirbuf->d_name);
			else
			    sprintf((char *)dirname, "F%s",dirbuf->d_name);
			    /* D & F to have first directories, then files */
#else
			    if (dir_list_style == MIXED_STYLE)
				sprintf((char *)dirname,
					" %s/", dirbuf->d_name);
			    else if (!strcmp(dirbuf->d_name, ".."))
				sprintf((char *)dirname,
					"A%s", dirbuf->d_name);
			    else
				sprintf((char *)dirname,
					"D%s", dirbuf->d_name);
			else if (dir_list_style == MIXED_STYLE)
			    sprintf((char *)dirname, " %s", dirbuf->d_name);
			else if (dir_list_style == FILES_FIRST)
			    sprintf((char *)dirname, "C%s", dirbuf->d_name);
			    /* C & D to have first files, then directories */
			else
			    sprintf((char *)dirname, "F%s", dirbuf->d_name);
#endif /* !DIRED_SUPPORT */
			/*
			**  Sort dirname in the tree bt.
			*/
			HTBTree_add(bt, dirname);
		    }

		    /*
		    **	Run through tree printing out in order.
		    */
		    {
			HTBTElement * next_element = HTBTree_next(bt,NULL);
			    /* pick up the first element of the list */
			char state;
			    /* I for initial (.. file),
			       D for directory file,
			       F for file */

#ifdef DIRED_SUPPORT
			char test;
#endif /* DIRED_SUPPORT */
			state = 'I';

			while (next_element != NULL) {
			    char *entry, *file_extra;

			    StrAllocCopy(tmpfilename,localname);
			    if (strcmp(localname, "/"))
				/*
				**  If filename is not root directory.
				*/
				StrAllocCat(tmpfilename, "/");

			    StrAllocCat(tmpfilename,
					(char *)HTBTree_object(next_element)+1);
			    /*
			    **	Append the current entry's filename
			    **	to the path.
			    */
			    HTSimplify(tmpfilename);
			    /*
			    **	Output the directory entry.
			    */
			    if (strcmp((char *)
				       (HTBTree_object(next_element)), "D..") &&
				strcmp((char *)
				       (HTBTree_object(next_element)), "A.."))
			    {
#ifdef DIRED_SUPPORT
				test = (*(char *)(HTBTree_object(next_element))
					== 'D' ? 'D' : 'F');
				if (state != test) {
#ifndef LONG_LIST
				    if (dir_list_style == FILES_FIRST) {
				       if (state == 'F')
					  END(HTML_DIR);
				    } else if (dir_list_style != MIXED_STYLE)
				       if (state == 'D')
					  END(HTML_DIR);
#endif /* !LONG_LIST */
				    state =
				       (*(char *)(HTBTree_object(next_element))
					== 'D' ? 'D' : 'F');
				    START(HTML_H2);
				    if (dir_list_style != MIXED_STYLE) {
				       START(HTML_EM);
				       PUTS(state == 'D' ?
					  "Subdirectories:" : "Files:");
				       END(HTML_EM);
				    }
				    END(HTML_H2);
#ifndef LONG_LIST
				    START(HTML_DIR);
#endif /* !LONG_LIST */
				}
#else
				if (state != *(char *)(HTBTree_object(
							 next_element))) {
#ifndef LONG_LIST
				    if (state == 'D')
					END(HTML_DIR);
#endif /* !LONG_LIST */
				    state =
				      (*(char *)(HTBTree_object(next_element))
				       == 'D' ? 'D' : 'F');
				    START(HTML_H2);
				    START(HTML_EM);
				    PUTS(state == 'D' ?
				    "Subdirectories:" : "Files:");
				    END(HTML_EM);
				    END(HTML_H2);
#ifndef LONG_LIST
				    START(HTML_DIR);
#endif /* !LONG_LIST */
				}
#endif /* DIRED_SUPPORT */
#ifndef LONG_LIST
				START(HTML_LI);
#endif /* !LONG_LIST */
			    }
			    entry = (char*)HTBTree_object(next_element)+1;
			    file_extra = NULL;

#ifdef LONG_LIST
			    LYListFmtParse(list_format, tmpfilename, target,
				entry, tail);
#else
			    HTDirEntry(target, tail, entry);
			    PUTS(entry);
			    END(HTML_A);
			    if (file_extra) {
				PUTS(file_extra);
				FREE(file_extra);
			    }
			    MAYBE_END(HTML_LI);
#endif /* LONG_LIST */

			    next_element = HTBTree_next(bt, next_element);
				/* pick up the next element of the list;
				 if none, return NULL*/
			}
			if (state == 'I') {
			    START(HTML_P);
			    PUTS("Empty Directory");
			}
#ifndef LONG_LIST
			else
			    END(HTML_DIR);
#endif /* !LONG_LIST */
		    }
			/* end while directory entries left to read */
		    closedir(dp);
		    FREE(logical);
		    FREE(tmpfilename);
		    FREE(tail);
		    HTBTreeAndObject_free(bt);

		    if (HTDirReadme == HT_DIR_README_BOTTOM)
			  do_readme(target, localname);
		    FREE_TARGET;
		    FREE(localname);
		    FREE(nodename);
		    return HT_LOADED;	/* document loaded */
		}

	    } /* end if localname is directory */

	} /* end if file stat worked */

/* End of directory reading section
*/
#endif /* HAVE_READDIR */
	{
	    FILE * fp = fopen(localname, "r");

	    CTRACE (tfp, "HTLoadFile: Opening `%s' gives %p\n",
				 localname, (void*)fp);
	    if (fp) {		/* Good! */
		int len;
		char *cp = NULL;

		if (HTEditable(localname)) {
		    HTAtom * put = HTAtom_for("PUT");
		    HTList * methods = HTAnchor_methods(anchor);
		    if (HTList_indexOf(methods, put) == (-1)) {
			HTList_addObject(methods, put);
		    }
		}
		/*
		**  Fake a Content-Encoding for compressed files. - FM
		*/
		if (!IsUnityEnc(myEncoding)) {
		    /*
		     *	We already know from the call to HTFileFormat above
		     *	that this is a compressed file, no need to look at
		     *	the filename again. - kw
		     */
#ifdef USE_ZLIB
		    if (strcmp(format_out->name, "www/download") != 0 &&
			(!strcmp(HTAtom_name(myEncoding), "gzip") ||
			 !strcmp(HTAtom_name(myEncoding), "x-gzip"))) {
			fclose(fp);
			gzfp = gzopen(localname, "rb");

			CTRACE(tfp, "HTLoadFile: gzopen of `%s' gives %p\n",
				    localname, (void*)gzfp);
			use_gzread = YES;
		    } else
#endif	/* USE_ZLIB */
		    {
			StrAllocCopy(anchor->content_type, format->name);
			StrAllocCopy(anchor->content_encoding, HTAtom_name(myEncoding));
			format = HTAtom_for("www/compressed");
		    }
		} else if ((len = strlen(localname)) > 2) {
		    if (localname[len - 1] == 'Z' &&
			localname[len - 2] == '.') {
			StrAllocCopy(cp, localname);
			cp[len - 2] = '\0';
			format = HTFileFormat(cp, &encoding, NULL);
			FREE(cp);
			format = HTCharsetFormat(format, anchor,
						 UCLYhndl_HTFile_for_unspec);
			StrAllocCopy(anchor->content_type, format->name);
			StrAllocCopy(anchor->content_encoding, "x-compress");
			format = HTAtom_for("www/compressed");
		    } else if ((len > 3) &&
			       !strcasecomp((char *)&localname[len - 2],
					    "gz") &&
			       localname[len - 3] == '.') {
			StrAllocCopy(cp, localname);
			cp[len - 3] = '\0';
			format = HTFileFormat(cp, &encoding, NULL);
			FREE(cp);
			format = HTCharsetFormat(format, anchor,
						 UCLYhndl_HTFile_for_unspec);
			StrAllocCopy(anchor->content_type, format->name);
			StrAllocCopy(anchor->content_encoding, "x-gzip");
#ifdef USE_ZLIB
			if (strcmp(format_out->name, "www/download") != 0) {
			    fclose(fp);
			    gzfp = gzopen(localname, "rb");

			    CTRACE(tfp, "HTLoadFile: gzopen of `%s' gives %p\n",
					localname, (void*)gzfp);
			    use_gzread = YES;
			}
#else  /* USE_ZLIB */
			format = HTAtom_for("www/compressed");
#endif	/* USE_ZLIB */
		    }
		}
		FREE(localname);
		FREE(nodename);
#ifdef USE_ZLIB
		if (use_gzread) {
		    if (gzfp) {
			char * sugfname = NULL;
			if (anchor->SugFname) {
			    StrAllocCopy(sugfname, anchor->SugFname);
			} else {
			    char * anchor_path = HTParse(anchor->address, "",
							 PARSE_PATH + PARSE_PUNCTUATION);
			    char * lastslash;
			    HTUnEscape(anchor_path);
			    lastslash = strrchr(anchor_path, '/');
			    if (lastslash)
				StrAllocCopy(sugfname, lastslash + 1);
			    FREE(anchor_path);
			}
			FREE(anchor->content_encoding);
			if (sugfname && *sugfname)
			    HTCheckFnameForCompression(&sugfname, anchor,
						       TRUE);
			if (sugfname && *sugfname)
			    StrAllocCopy(anchor->SugFname, sugfname);
			FREE(sugfname);
			status = HTParseGzFile(format, format_out,
					       anchor,
					       gzfp, sink);
		    } else {
			status = HTLoadError(NULL,
					     -(HT_ERROR),
				     "Could not open file for decompression!");
		    }
		} else
#endif /* USE_ZLIB */
		{
		    status = HTParseFile(format, format_out, anchor, fp, sink);
		    fclose(fp);
		}
		return status;
	    }  /* If successful open */
	    FREE(localname);
	}  /* scope of fp */
    }  /* local unix file system */
#endif /* !NO_UNIX_IO */
#endif /* VMS */

#ifndef DECNET
    /*
    **	Now, as transparently mounted access has failed, we try FTP.
    */
    {
	/*
	**  Deal with case-sensitivity differences on VMS versus Unix.
	*/
#ifdef VMS
	if (strcasecomp(nodename, HTHostName()) != 0)
#else
	if (strcmp(nodename, HTHostName()) != 0)
#endif /* VMS */
	{
	    FREE(nodename);
	    if (!strncmp(addr, "file://localhost", 16)) {
		return -1;  /* never go to ftp site when URL
			     * is file://localhost
			     */
	    } else {
		return HTFTPLoad(addr, anchor, format_out, sink);
	    }
	}
	FREE(nodename);
    }
#endif /* !DECNET */

    /*
    **	All attempts have failed.
    */
    {
	CTRACE(tfp, "Can't open `%s', errno=%d\n", addr, SOCKET_ERRNO);

	return HTLoadError(sink, 403, "Can't access requested file.");
    }
}

/*
**	Protocol descriptors
*/
#ifdef GLOBALDEF_IS_MACRO
#define _HTFILE_C_1_INIT { "ftp", HTLoadFile, 0 }
GLOBALDEF (HTProtocol,HTFTP,_HTFILE_C_1_INIT);
#define _HTFILE_C_2_INIT { "file", HTLoadFile, HTFileSaveStream }
GLOBALDEF (HTProtocol,HTFile,_HTFILE_C_2_INIT);
#else
GLOBALDEF PUBLIC HTProtocol HTFTP  = { "ftp", HTLoadFile, 0 };
GLOBALDEF PUBLIC HTProtocol HTFile = { "file", HTLoadFile, HTFileSaveStream };
#endif /* GLOBALDEF_IS_MACRO */
