/*
**  This file contains code for changing the Linux console mode.
**  Currently some names for font files are hardwired in here.
**  You have to change this code if it needs accommodation for your
**  system (or get the required files...).
**
**  Depending on the Display Character Set switched to, and the previous
**  one as far as it is known, system("setfont ...") and/or output of
**  escape sequences to switch console mode are done.  Curses will be
**  temporarily suspended while that happens.
**
**  NOTE that the setfont calls will also affect all other virtual consoles.
**
**  Any ideas how to do this for other systems?
*/

#include <HTUtils.h>
#include <LYUtils.h>

#include <UCMap.h>
#include <UCDefs.h>
#include <UCAuto.h>
#include <LYGlobalDefs.h>
#include <LYClean.h>
#include <LYLeaks.h>

#ifdef EXP_CHARTRANS_AUTOSWITCH

#ifdef LINUX
typedef enum {
    Is_Unset, Is_Set, Dunno, Dont_Care
} TGen_state_t;
typedef enum {
    G0, G1
} TGNstate_t;
typedef enum {
    GN_Blat1, GN_0decgraf, GN_Ucp437, GN_Kuser, GN_dunno, GN_dontCare
} TTransT_t;

static char *T_font_fn = NULL;
static char *T_umap_fn = NULL;

#define SETFONT "setfont"
#define NOOUTPUT "2>/dev/null >/dev/null"

/*
 *  call_setfont - execute "setfont" command via system()
 *  returns:	0  ok (as far as we know)
 *	       -1  error (assume font and umap are not loaded)
 *		1  error with umap (assume font loaded but umap empty)
 */
PRIVATE int call_setfont ARGS3(
	CONST char *,	font,
	CONST char *,	fnsuffix,
	CONST char *,	umap)
{
    char *T_setfont_cmd = NULL;
    int rv;

    if ((font && T_font_fn && !strcmp(font, T_font_fn))
     && (umap && T_umap_fn && !strcmp(umap, T_umap_fn))) {
	/*
	 *  No need to repeat.
	 */
	return 0;
    }
    if (font)
	StrAllocCopy(T_font_fn, font);
    if (umap)
	StrAllocCopy(T_umap_fn, umap);

    if (!*fnsuffix)
	fnsuffix = "";

    if (umap &&*umap && font && *font) {
	HTSprintf0(&T_setfont_cmd, "%s %s%s -u %s %s",
		SETFONT, font, fnsuffix,	umap,	NOOUTPUT);
    } else if (font && *font) {
	HTSprintf0(&T_setfont_cmd, "%s %s%s %s",
		SETFONT, font, fnsuffix,		NOOUTPUT);
    } else if (umap && *umap) {
	HTSprintf0(&T_setfont_cmd, "%s -u %s %s",
		SETFONT,			umap,	NOOUTPUT);
    }

    if (T_setfont_cmd) {
	CTRACE((tfp, "Executing setfont: '%s'\n", T_setfont_cmd));
	rv = LYSystem(T_setfont_cmd);
	FREE(T_setfont_cmd);
	if (rv) {
	    CTRACE((tfp, "call_setfont: system returned %d (0x%x)!\n",
		   rv, rv));
	    if ((rv == 0x4200 || rv == 0x4100) && umap && *umap)
		/*
		 * It seems setfont returns 65 or 66 to the shell if
		 * the font was loaded ok but something was wrong with
		 * the umap file. - kw
		 */
		return 1;
	    else
		return -1;
	}
    }
    return 0;
}

PRIVATE void write_esc ARGS1(
	CONST char *,	p)
{
    int fd = open("/dev/tty", O_WRONLY);

    if (fd >= 0) {
	write(fd, p, strlen(p));
	close(fd);
    }
}

PRIVATE int nonempty_file ARGS1(
	CONST char *,	p)
{
    struct stat sb;

    return (stat(p, &sb) == 0 &&
	    S_ISREG(sb.st_mode) &&
	    (sb.st_size != 0));
}

/*
 *  This is the thing that actually gets called from display_page().
 */
PUBLIC void UCChangeTerminalCodepage ARGS2(
	int,		newcs,
	LYUCcharset *,	p)
{
    static int lastcs = -1;
    static CONST char * lastname = NULL;
    static TTransT_t lastTransT = GN_dunno;
    static TGen_state_t lastUtf = Dunno;
    static TGen_state_t lastHasUmap = Dunno;

    static char *old_font = NULL;
    static char *old_umap = NULL;

    CONST char * name;
    TTransT_t TransT = GN_dunno;
    TGen_state_t Utf = Dunno;
    TGen_state_t HasUmap = Dunno;

    char *tmpbuf1 = NULL;
    char *tmpbuf2 = NULL;
    int status = 0;

    /*
     *	Restore the original character set.
     */
    if (newcs < 0 || p == 0) {
	if (old_font && *old_font &&
	    old_umap && *old_umap) {
	    int have_font = nonempty_file(old_font);
	    int have_umap = nonempty_file(old_umap);

	    if (have_font) {
		if (have_umap) {
		    HTSprintf0(&tmpbuf1, "%s %s -u %s %s",
			    SETFONT, old_font, old_umap, NOOUTPUT);
		} else {
		    HTSprintf0(&tmpbuf1, "%s %s %s",
			    SETFONT, old_font, NOOUTPUT);
		}
		CTRACE((tfp, "Executing setfont to restore: '%s'\n", tmpbuf1));
		status = LYSystem(tmpbuf1);
		FREE(tmpbuf1);
	    }
	}
	if (newcs < 0 && p == 0) {
	    if (old_font) {
		LYRemoveTemp(old_font);
		FREE(old_font);
	    }
	    if (old_umap) {
		LYRemoveTemp(old_umap);
		FREE(old_umap);
	    }
	    if (status == 0) {
		FREE(T_font_fn);
		FREE(T_umap_fn);
	    }
	}
	return;
    } else if (lastcs < 0 && old_umap == 0 && old_font == 0) {
	FILE * fp1;
	FILE * fp2 = NULL;
	if ((old_font = calloc(1, LY_MAXPATH)))
	    old_umap = calloc(1, LY_MAXPATH);
	if ((fp1 = LYOpenTemp(old_font, ".fnt", "wb")))
	    fp2 = LYOpenTemp(old_umap, ".uni", "wb");
	if (fp1 && fp2) {
	    size_t nlen;
	    char *rp;
	    HTSprintf0(&tmpbuf1, "%s -o %s -ou %s %s",
		       SETFONT, old_font, old_umap, NOOUTPUT);
	    CTRACE((tfp, "Executing setfont to save: '%s'\n", tmpbuf1));
	    LYSystem(tmpbuf1);
	    FREE(tmpbuf1);
	    LYCloseTempFP(fp1);
	    LYCloseTempFP(fp2);
	    if ((nlen = strlen(old_font)) + 1 < LY_MAXPATH &&
		 (rp = realloc(old_font, nlen + 1)))
		old_font = rp;
	    if ((nlen = strlen(old_umap)) + 1 < LY_MAXPATH &&
		 (rp = realloc(old_umap, nlen + 1)))
		old_umap = rp;
	} else {
	    if (fp1)
		LYRemoveTemp(old_font);
	    if (fp2)
		LYRemoveTemp(old_umap);
	    FREE(old_font);
	    FREE(old_umap);
	}
    }

    name = p->MIMEname;

    /*
     *	Font sizes are currently hardwired here.
     */
#define SUFF1 ".f16"
#define SUFF2 "-16.psf"
#define SUFF3 "-8x16"
#define SUFF4 "8x16"
#define SUFF5 ".cp -16"

    /*
     *	Use this for output of escape sequences.
     */
    if ((x_display != NULL) ||
	LYgetXDisplay() != NULL) {
	/*
	 *  We won't do anything in an xterm.  Better that way...
	 */
	return;
    }

    /* NOTE: `!!umap not in kbd!!' comments below means that the *.uni file
       is not found in kbd package.  Reference Debian Package: kbd-data,
       Version: 0.96a-14.  They should be located elsewhere or generated.
       Also some cpNNN fonts used below are not in the kbd-data.  - kw
       */

    if (!strncmp(name, "iso-8859-1", 10) &&
	       (!name[10] || !isdigit((unsigned char)name[10]))
	) {
	if ((lastHasUmap == Is_Set) && !strcmp(lastname, "cp850")) {
	    /*
	     *	cp850 already contains all latin1 characters.
	     */
	    if (lastTransT != GN_Blat1) {
		TransT = GN_Blat1;
	    }
	} else {
	    /*
	     *	"setfont lat1u-16.psf -u lat1u.uni"
	     */
	    status = call_setfont("lat1u", SUFF2, "lat1u.uni");
	    HasUmap = Is_Set;
	    if (lastTransT != GN_Blat1) {
		TransT = GN_Blat1;
	    }
	}
	Utf = Is_Unset;
    } else if (!strcmp(name, "iso-8859-2")) {
#ifdef NOTDEFINED
	/*
	 *  "setfont lat2-16.psf -u lat2.uni"
	 */
	status = call_setfont("lat2", SUFF2, "lat2.uni");
#endif /* NOTDEFINED */
	/*
	 *  "setfont iso02.f16 -u iso02.uni"
	 */
	status = call_setfont("iso02", SUFF1, "iso02.uni");
	TransT = GN_Kuser;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (!strcmp(name, "iso-8859-15")) {
	/*
	 *  "setfont lat0-16.psf"
	 */
	status = call_setfont("lat0", SUFF2, NULL);
	TransT = GN_Blat1;	/* bogus! */
	HasUmap = Dunno; /* distributed lat0 files have bogus map data! */
	Utf = Is_Unset;
    } else if (!strncmp(name, "iso-8859-", 9)) {
	if (strlen(name) <= 10 || !isdigit((unsigned char)name[10]))
	    HTSprintf0(&tmpbuf1, "iso0%s", &name[9]);
	else
	    HTSprintf0(&tmpbuf1, "iso%s", &name[9]);
	HTSprintf0(&tmpbuf2, "%s.uni", tmpbuf1);
	/*
	 *  "setfont iso0N.f16 -u iso0N.uni"
	 */
	status = call_setfont(tmpbuf1, SUFF1, tmpbuf2);
	FREE(tmpbuf1);
	FREE(tmpbuf2);
	TransT = GN_Kuser;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (!strcmp(name, "koi8-r")) {
	/*
	 *  "setfont koi8-8x16"
	 */
	status = call_setfont("koi8", SUFF3, "koi8r.uni"); /* !!umap not in kbd!! */
	TransT = GN_Kuser;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (!strcmp(name, "cp437")) {
	/*
	 *  "setfont default8x16 -u cp437.uni"
	 */
	status = call_setfont("default", SUFF4, "cp437.uni");
	if (lastTransT == GN_Kuser || lastTransT == GN_Ucp437)
	    TransT = GN_dontCare;
	else
	    TransT = GN_Ucp437;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (!strcmp(name, "cp850")) {
	/*
	 *  "setfont cp850-8x16 -u cp850.uni"
	 */
	status = call_setfont("cp850", SUFF3, "cp850.uni"); /* !!umap not in kbd!! */
	TransT = GN_Kuser;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (!strcmp(name, "cp866") ||
	       !strcmp(name, "cp852") ||
#if 0
	       !strcmp(name, "cp861") ||
	       !strcmp(name, "cp850") ||
	       !strcmp(name, "cp437") ||
#endif
	       !strcmp(name, "cp862")) { /* MS-Kermit has these files */
	HTSprintf0(&tmpbuf2, "%s.uni", name);
	/*
	 *  "setfont cpNNN.f16"
	 */
	status = call_setfont(name, SUFF1, tmpbuf2); /* !!umap not in kbd!! */
	FREE(tmpbuf2);
	TransT = GN_Kuser;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (!strcmp(name, "cp737")) {
	/*
	 *  "setfont cp737.cp"
	 */
	status = call_setfont("737", SUFF5, "cp737.uni"); /* !!umap not in kbd!! */
	TransT = GN_Kuser;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (!strcmp(name, "x-transparent")) {
	Utf = Dont_Care;
    } else if (!strcmp(name, "us-ascii")) {
	Utf = Dont_Care;
    } else if (!strncmp(name, "mnem", 4)) {
	Utf = Dont_Care;
    }

    if (status == 1)
	HasUmap = Is_Unset;
    else if (status < 0) {
	if (HasUmap == Is_Set)
	    HasUmap = Dunno;
	name = "unknown-8bit";
    }

    if (TransT != lastTransT) {
	if (TransT == GN_Blat1) {
	    /*
	     *	Switch Linux console to lat1 table.
	     */
	    write_esc("\033(B");
	} else if (TransT == GN_0decgraf) {
	    write_esc("\033(0");
	} else if (TransT == GN_Ucp437) {
	     /*
	      *  Switch Linux console to 437 table?
	      */
	    write_esc("\033(U");
	} else if (TransT == GN_Kuser) {
	     /*
	      *  Switch Linux console to user table.
	      */
	    write_esc("\033(K");
	}
	if (TransT != GN_dunno && TransT != GN_dontCare) {
	    lastTransT = TransT;
	} else {
	    TransT = lastTransT;
	}
    }

    if (HasUmap != Dont_Care && HasUmap != Dunno)
	lastHasUmap = HasUmap;

    if (p->enc == UCT_ENC_UTF8) {
	if (lastUtf != Is_Set) {
	    Utf = Is_Set;
	    /*
	     *	Turn Linux console UTF8 mode ON.
	     */
	    write_esc("\033%G");
	    lastUtf = Utf;
	}
	return;
    } else if (lastUtf == Is_Set && Utf != Dont_Care) {
	Utf = Is_Unset;
	/*
	 *  Turn Linux console UTF8 mode OFF.
	 */
	write_esc("\033%@");
	lastUtf = Utf;
    }

    if (Utf != Dont_Care && Utf != Dunno)
	lastUtf = Utf;

    lastcs = newcs;
    lastname = name;
}

#else /* Not LINUX: */
/*
 *  This is the thing that actually gets called from display_page().
 */
PUBLIC void UCChangeTerminalCodepage ARGS2(
	int,		newcs,
	LYUCcharset *,	p)
{
#ifdef __EMX__
    int res = 0;

    if (p->codepage > 0) {
	res = VioSetCp(0, p->codepage, 0);
	CTRACE((tfp, "UCChangeTerminalCodepage: VioSetCp(%d) returned %d\n", p->codepage, res));
    }
#else
    CTRACE((tfp, "UCChangeTerminalCodepage: Called, but not implemented!"));
#endif
}
#endif /* LINUX */

#else /* EXP_CHARTRANS_AUTOSWITCH not defined: */
/*
 *  This is the thing that actually gets called from display_page().
 */
PUBLIC void UCChangeTerminalCodepage ARGS2(
	int,		newcs GCC_UNUSED,
	LYUCcharset *,	p GCC_UNUSED)
{
    CTRACE((tfp, "UCChangeTerminalCodepage: Called, but not implemented!"));
}
#endif /* EXP_CHARTRANS_AUTOSWITCH */
