#ifdef EXP_CHARTRANS
#ifdef EXP_CHARTRANS_AUTOSWITCH
#ifdef LINUX

/* This file contains code for changing the Linux console mode.
** Currently some names for font files are hardwired in here.
** You have to change this code if it needs accomodation for your
** system (or get the required files...).
** 
** Depending on the Display Character Set switched to, and the previous
** one as far as it is known, system("setfont ...") and/or output of
** escape sequences to switch console mode are done.  Curses will be
** temporarily suspended while that happens.
**
** NOTE that the setfont calls will also affect all other virtual consoles.
**
** Any ideas how to do this for other systems?
*/
#include "tcp.h"
#include "HTUtils.h"
#include "UCMap.h"
#include "UCDefs.h"
#include "UCAuto.h"
#include "LYGlobalDefs.h"

typedef enum {Is_Unset, Is_Set, Dunno, Dont_Care} TGen_state_t;
typedef enum {G0, G1} TGNstate_t;
typedef enum
{
    GN_Blat1, GN_0decgraf, GN_Ucp437, GN_Kuser, GN_dunno, GN_dontCare
} TTransT_t;

static char T_font_fn[100] = "\0";
static char T_umap_fn[100] = "\0";
static char T_setfont_cmd[200] = "\0";
#define SETFONT "setfont"
#define NOOUTPUT "2>/dev/null"

PRIVATE void call_setfont ARGS3(char *,font, char *,fnsuffix, char *,umap)
{
    if (font && *font && umap && *umap &&
	0==strcmp(font,T_font_fn) && 0==strcmp(umap,T_umap_fn)) {
	return;			/* no need to repeat */
    }
    if (font)
	strcpy(T_font_fn, font);
    if (umap)
	strcpy(T_umap_fn, umap);

    if (!*fnsuffix)
	fnsuffix = "";

    if (umap &&*umap && font && *font) {
	sprintf(T_setfont_cmd, "%s %s%s -u %s %s",
		SETFONT, font, fnsuffix, 	umap, 	NOOUTPUT);
    } else if (font && *font) {
	sprintf(T_setfont_cmd, "%s %s%s %s",
		SETFONT, font, fnsuffix, 		NOOUTPUT);
    } else if (umap && *umap) {
	sprintf(T_setfont_cmd, "%s -u %s %s",
		SETFONT, 			umap, 	NOOUTPUT);
    } else {
	*T_setfont_cmd = '\0';
    }
    
    if (*T_setfont_cmd) {
    
            if (TRACE) {
                        fprintf(stderr, "Executing setfont: '%s'\n", 
                        T_setfont_cmd);
                        }
                                
	system(T_setfont_cmd);
    }
}

/* This is the thing that actually gets called from display_page(). */

PUBLIC void UCChangeTerminalCodepage ARGS2(int, newcs, LYUCcharset *,p)
{
    static int lastcs = -1;
    static char * lastname = NULL;
    static TGNstate_t lastGN = G0;
    static TTransT_t lastTransT = GN_dunno;
    static TGen_state_t lastUtf = Dunno;
    static TGen_state_t lastHasUmap = Dunno;

    char * name = p->MIMEname;
    TGNstate_t GN = G0;
    TTransT_t TransT = GN_dunno;
    TGen_state_t Utf = Dunno;
    TGen_state_t HasUmap = Dunno;

    char tmpbuf1[100], tmpbuf2[20];
    char *cp;

/* Font sizes are currently hardwired here... */
#define SUFF1 ".f16"
#define SUFF2 "-16.psf"
#define SUFF3 "-8x16"
#define SUFF4 "8x16"

/* use this for output of escape sequences... */
/* for some reason stdout won't do... maybe needs flush() somewhere.. */
#define ESCOUT stderr

#ifdef VMS
#define DISPLAY "DECW$DISPLAY"
#else
#define DISPLAY "DISPLAY"
#endif /* VMS */

    if (display || (cp = getenv(DISPLAY)) != NULL) {
      /* We won't do anything in an xterm.  Better that way...  */
	return;
    }

    if (0==strcmp(name,"iso-8859-10")) {
	call_setfont("iso10", SUFF1,	"iso10.uni");
	TransT = GN_Kuser;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (0==strncmp(name,"iso-8859-1",10)) {
	if ((lastHasUmap==Is_Set) && 0==strcmp(lastname,"cp850")) {
	    /* cp850 already contains all latin1 characters */
	    if (lastTransT != GN_Blat1)
		TransT = GN_Blat1;
	} else {
	    call_setfont("lat1u", SUFF2,	"lat1u.uni");
				/* "setfont lat1u-16.psf -u lat1u.uni" */
	    HasUmap = Is_Set;
	    if (lastTransT != GN_Blat1)
		TransT = GN_Blat1;
	}
	Utf = Is_Unset;
    } else if (0==strcmp(name,"iso-8859-2")) {
/*	call_setfont("lat2", SUFF2,	"lat2.uni");  */
				/* "setfont lat2-16.psf -u lat2.uni" */
	call_setfont("iso02", SUFF1,	"iso02.uni");
				/* "setfont iso02.f16 -u iso02.uni" */
	TransT = GN_Kuser;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (0==strncmp(name,"iso-8859-",9)) {
	sprintf(tmpbuf1, "iso0%s", &name[9]);
	sprintf(tmpbuf2, "iso0%s%s", &name[9],".uni");
        call_setfont(tmpbuf1,SUFF1,	tmpbuf2);
				/* "setfont iso0N.f16 -u iso0N.uni" */
	TransT = GN_Kuser;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (0==strcmp(name,"koi8-r")) {
	call_setfont("koi8", SUFF3,	NULL);
	TransT = GN_Kuser;
	HasUmap = Is_Unset;
	Utf = Is_Unset;
    } else if (0==strcmp(name,"cp437")) {
	call_setfont("default", SUFF4 ,	"cp437.uni");
				/* "setfont default8x16 -u cp437.uni" */
	if (TransT == GN_Kuser || TransT == GN_Ucp437)
	    TransT = GN_dontCare;
	else
	    TransT = GN_Ucp437;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (0==strcmp(name,"cp850")) {
	call_setfont("cp850"   ,SUFF3 ,	"cp850.uni");
				/* "setfont cp850-8x16 -u cp850.uni" */
	TransT = GN_Kuser;
	HasUmap = Is_Set;
	Utf = Is_Unset;
    } else if (0==strcmp(name,"x-transparent")) {
	Utf = Dont_Care;
    } else if (0==strcmp(name,"us-ascii")) {
	Utf = Dont_Care;
    } else if (0==strncmp(name,"mnem",4)) {
	Utf = Dont_Care;
    }

    if (TransT != lastTransT) {
	if (TransT == GN_Blat1) {
	    fprintf(ESCOUT,"\033(B"); /* switch Linux console to lat1 table */
	} else if (TransT == GN_0decgraf) {
	    fprintf(ESCOUT,"\033(0");
	} else if (TransT == GN_Ucp437) {
	    fprintf(ESCOUT,"\033(U"); /* switch Linux console to 437 table? */
	} else if (TransT == GN_Kuser) {
	    fprintf(ESCOUT,"\033(K"); /* switch Linux console to user table */
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
	    fprintf(ESCOUT,"\033%%G"); /* turn Linux console UTF8 mode ON */
	    lastUtf = Utf;
	}
	return;
    } else if (lastUtf == Is_Set && Utf != Dont_Care) {
	Utf = Is_Unset;
	fprintf(ESCOUT,"\033%%@"); /* turn Linux console UTF8 mode OFF */
	lastUtf = Utf;
    }

    if (Utf != Dont_Care && Utf != Dunno)
	lastUtf = Utf;

    lastcs = newcs;
    lastname = name;
}
#endif /* LINUX */
#endif /* EXP_CHARTRANS_AUTOSWITCH */
#endif /* EXP_CHARTRANS */
