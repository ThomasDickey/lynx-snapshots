#ifndef LYUTILS_H
#define LYUTILS_H

#ifndef HTLIST_H
#include <HTList.h>
#endif /* HTLIST_H */

#ifdef VMS
#include <HTVMSUtils.h>
#define HTSYS_name(path)   HTVMS_name("", path)
#define HTSYS_purge(path)  HTVMS_purge(path)
#define HTSYS_remove(path) HTVMS_remove(path)
#endif /* VMS */

#if defined(DOSPATH) || defined(__EMX__)
#include <HTDOS.h>
#define HTSYS_name(path) HTDOS_name(path)
#endif

#ifndef HTSYS_name
#define HTSYS_name(path) path
#endif

#ifndef HTSYS_purge
#define HTSYS_purge(path) /*nothing*/
#endif

#ifndef HTSYS_remove
#define HTSYS_remove(path) remove(path)
#endif

#ifdef DOSPATH
#define LYIsPathSep(ch) ((ch) == '/' || (ch) == '\\')
#else
#define LYIsPathSep(ch) ((ch) == '/')
#endif

#define LYIsHtmlSep(ch) ((ch) == '/')

#define TABLESIZE(v) (sizeof(v)/sizeof(v[0]))

extern BOOLEAN LYAddSchemeForURL PARAMS((char **AllocatedString, char *default_scheme));
extern BOOLEAN LYCachedTemp PARAMS((char *result, char **cached));
extern BOOLEAN LYCanDoHEAD PARAMS((CONST char *address));
extern BOOLEAN LYExpandHostForURL PARAMS((char **AllocatedString, char *prefix_list, char *suffix_list));
extern BOOLEAN LYPathOffHomeOK PARAMS((char *fbuffer, size_t fbuffer_size));
extern BOOLEAN LYisLocalAlias PARAMS((char *filename));
extern BOOLEAN LYisLocalFile PARAMS((char *filename));
extern BOOLEAN LYisLocalHost PARAMS((char *filename));
extern BOOLEAN inlocaldomain NOPARAMS;
extern CONST char *Home_Dir NOPARAMS;
extern FILE *LYAppendToTxtFile PARAMS((char * name));
extern FILE *LYNewBinFile PARAMS((char * name));
extern FILE *LYNewTxtFile PARAMS((char * name));
extern FILE *LYOpenScratch PARAMS((char *result, CONST char *prefix));
extern FILE *LYOpenTemp PARAMS((char *result, CONST char *suffix, CONST char *mode));
extern FILE *LYReopenTemp PARAMS((char *name));
extern char *LYPathLeaf PARAMS((char * pathname));
extern char *LYSysShell NOPARAMS;
extern char *LYgetXDisplay NOPARAMS;
extern char *quote_pathname PARAMS((char *pathname));
extern char *strip_trailing_slash PARAMS((char * my_dirname));
extern char *wwwName PARAMS((CONST char *pathname));
extern int HTCheckForInterrupt NOPARAMS;
extern int LYCheckForProxyURL PARAMS((char *filename));
extern int LYOpenInternalPage PARAMS((FILE **fp0, char **newfile));
extern int LYSystem PARAMS((char *command));
extern int is_url PARAMS((char *filename));
extern int number2arrows PARAMS((int number));
extern time_t LYmktime PARAMS((char *string, BOOL absolute));
extern void BeginInternalPage PARAMS((FILE *fp0, char *Title, char *HelpURL));
extern void EndInternalPage PARAMS((FILE *fp0));
extern void HTAddSugFilename PARAMS((char *fname));
extern void HTSugFilenames_free NOPARAMS;
extern void LYAddHtmlSep PARAMS((char **path));
extern void LYAddHtmlSep0 PARAMS((char *path));
extern void LYAddLocalhostAlias PARAMS((char *alias));
extern void LYAddPathSep PARAMS((char **path));
extern void LYAddPathSep0 PARAMS((char *path));
extern void LYAddPathToHome PARAMS((char *fbuffer, size_t fbuffer_size, char *fname));
extern void LYCheckMail NOPARAMS;
extern void LYCleanupTemp NOPARAMS;
extern void LYCloseTemp PARAMS((char *name));
extern void LYCloseTempFP PARAMS((FILE *fp));
extern void LYConvertToURL PARAMS((char **AllocatedString));
extern void LYDoCSI PARAMS((char *url, CONST char *comment, char **csi));
extern void LYEnsureAbsoluteURL PARAMS((char **href, CONST char *name));
extern void LYFakeZap PARAMS((BOOL set));
extern void LYLocalFileToURL PARAMS((char **target, CONST char *source));
extern void LYLocalhostAliases_free NOPARAMS;
extern void LYRemoveTemp PARAMS((char *name));
extern void LYTrimHtmlSep PARAMS((char *path));
extern void LYTrimPathSep PARAMS((char *path));
extern void LYTrimRelFromAbsPath PARAMS((char *path));
extern void LYsetXDisplay PARAMS((char *new_display));
extern void change_sug_filename PARAMS((char *fname));
extern void checkmail NOPARAMS;
extern void convert_to_spaces PARAMS((char *string, BOOL condense));
extern void free_and_clear PARAMS((char **obj));
extern void highlight PARAMS((int flag, int cur, char *target));
extern void noviceline PARAMS((int more_flag));
extern void parse_restrictions PARAMS((CONST char *s));
extern void remove_backslashes PARAMS((char *buf));
extern void size_change PARAMS((int sig));
extern void statusline PARAMS((CONST char *text));
extern void toggle_novice_line NOPARAMS;

#ifdef VMS
extern void Define_VMSLogical PARAMS((char *LogicalName, char *LogicalValue));
#endif /* VMS */

#if ! HAVE_PUTENV
extern int putenv PARAMS((CONST char *string));
#endif /* HAVE_PUTENV */

#ifdef UNIX
extern void LYRelaxFilePermissions PARAMS((CONST char * name));
#endif

/*
 *  Whether or not the status line must be shown.
 */
extern BOOLEAN mustshow;
#define _statusline(msg)	mustshow = TRUE, statusline(msg)

/*
 *  For is_url().
 *
 *  Universal document id types.
 */
#define HTTP_URL_TYPE		 1
#define FILE_URL_TYPE		 2
#define FTP_URL_TYPE		 3
#define WAIS_URL_TYPE		 4
#define NEWS_URL_TYPE		 5
#define NNTP_URL_TYPE		 6
#define TELNET_URL_TYPE		 7
#define TN3270_URL_TYPE		 8
#define RLOGIN_URL_TYPE		 9
#define GOPHER_URL_TYPE		10
#define HTML_GOPHER_URL_TYPE	11
#define TELNET_GOPHER_URL_TYPE	12
#define INDEX_GOPHER_URL_TYPE	13
#define MAILTO_URL_TYPE		14
#define FINGER_URL_TYPE		15
#define CSO_URL_TYPE		16
#define HTTPS_URL_TYPE		17
#define SNEWS_URL_TYPE		18
#define PROSPERO_URL_TYPE	19
#define AFS_URL_TYPE		20

#define DATA_URL_TYPE		21

#define LYNXEXEC_URL_TYPE	22
#define LYNXPROG_URL_TYPE	23
#define LYNXCGI_URL_TYPE	24

#define NEWSPOST_URL_TYPE	25
#define NEWSREPLY_URL_TYPE	26
#define SNEWSPOST_URL_TYPE	27
#define SNEWSREPLY_URL_TYPE	28

#define LYNXPRINT_URL_TYPE	29
#define LYNXHIST_URL_TYPE	30
#define LYNXDOWNLOAD_URL_TYPE	31
#define LYNXKEYMAP_URL_TYPE	32
#define LYNXIMGMAP_URL_TYPE	33
#define LYNXCOOKIE_URL_TYPE	34
#define LYNXDIRED_URL_TYPE	35
#define LYNXOPTIONS_URL_TYPE	36

#define PROXY_URL_TYPE		37

#define UNKNOWN_URL_TYPE	38

/*
 *  For change_sug_filename().
 */
extern HTList *sug_filenames;

/*
 *  Miscellaneous.
 */
#define ON      1
#define OFF     0
#define STREQ(a,b) (strcmp(a,b) == 0)
#define STRNEQ(a,b,c) (strncmp(a,b,c) == 0)

#define HIDE_CHMOD 0600
#define HIDE_UMASK 0077

#endif /* LYUTILS_H */
