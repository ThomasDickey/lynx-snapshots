dnl Macros for auto-configure script.
dnl written feb/1997
dnl by T.E.Dickey <dickey@clark.net>
dnl and Jim Spath <jspath@mail.bcpl.lib.md.us>
dnl
dnl ---------------------------------------------------------------------------
dnl Add an include-directory to $CPPFLAGS.  Don't add /usr/include, since it's
dnl redundant.  Also, don't add /usr/local/include if we're using gcc.
AC_DEFUN([CF_ADD_INCDIR],
[
for cf_add_incdir in $1
do
	while true
	do
		case $cf_add_incdir in
		/usr/include) # (vi
			;;
		/usr/local/include) # (vi
			test -z "$GCC" && CPPFLAGS="$CPPFLAGS -I$cf_add_incdir"
			;;
		*) # (vi
			CPPFLAGS="$CPPFLAGS -I$cf_add_incdir"
			;;
		esac
		cf_top_incdir=`echo $cf_add_incdir | sed -e 's:/include/.*$:/include:'`
		test "$cf_top_incdir" = "$cf_add_incdir" && break
		cf_add_incdir="$cf_top_incdir"
	done
done
])dnl
dnl ---------------------------------------------------------------------------
dnl This is adapted from the macros 'fp_PROG_CC_STDC' and 'fp_C_PROTOTYPES'
dnl in the sharutils 4.2 distribution.
AC_DEFUN([CF_ANSI_CC],
[
AC_MSG_CHECKING(for ${CC-cc} option to accept ANSI C)
AC_CACHE_VAL(cf_cv_ansi_cc,[
cf_cv_ansi_cc=no
cf_save_CFLAGS="$CFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc
# UnixWare 1.2		(cannot use -Xc, since ANSI/POSIX clashes)
for cf_arg in "-DCC_HAS_PROTOS" "" -qlanglvl=ansi -std1 "-Aa -D_HPUX_SOURCE" -Xc
do
	CFLAGS="$cf_save_CFLAGS $cf_arg"
	AC_TRY_COMPILE(
[
#ifndef CC_HAS_PROTOS
#if !defined(__STDC__) || __STDC__ != 1
choke me
#endif
#endif
],[
	int test (int i, double x);
	struct s1 {int (*f) (int a);};
	struct s2 {int (*f) (double a);};],
	[cf_cv_ansi_cc="$cf_arg"; break])
done
CFLAGS="$cf_save_CFLAGS"
])
AC_MSG_RESULT($cf_cv_ansi_cc)
if test "$cf_cv_ansi_cc" != "no"; then
if test ".$cf_cv_ansi_cc" != ".-DCC_HAS_PROTOS"; then
	CFLAGS="$CFLAGS $cf_cv_ansi_cc"
fi
fi
])
dnl ---------------------------------------------------------------------------
dnl Check for existence of alternate-character-set support in curses, so we
dnl can decide to use it for box characters.
dnl 
AC_DEFUN([CF_ALT_CHAR_SET],
[
AC_MSG_CHECKING([if curses supports alternate-character set])
AC_CACHE_VAL(cf_cv_alt_char_set,[
	AC_TRY_LINK([
#include <$cf_cv_ncurses_header>
	],[chtype x = acs_map['l']; acs_map['m'] = 0],
	[cf_cv_alt_char_set=yes],  
	[cf_cv_alt_char_set=no])])
AC_MSG_RESULT($cf_cv_alt_char_set)
test $cf_cv_alt_char_set = yes && AC_DEFINE(ALT_CHAR_SET)
])dnl
dnl ---------------------------------------------------------------------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2 (default: on)],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl Allow user to enable a normally-off option.
AC_DEFUN([CF_ARG_ENABLE],
[CF_ARG_OPTION($1,[$2 (default: off)],[$3],[$4],no)])dnl
dnl ---------------------------------------------------------------------------
dnl Restricted form of AC_ARG_ENABLE that ensures user doesn't give bogus
dnl values.
dnl
dnl Parameters:
dnl $1 = option name
dnl $2 = help-string 
dnl $3 = action to perform if option is not default
dnl $4 = action if perform if option is default
dnl $5 = default option value (either 'yes' or 'no')
AC_DEFUN([CF_ARG_OPTION],
[AC_ARG_ENABLE($1,[$2],[test "$enableval" != ifelse($5,no,yes,no) && enableval=ifelse($5,no,no,yes)
  if test "$enableval" != "$5" ; then
ifelse($3,,[    :]dnl
,[    $3]) ifelse($4,,,[
  else
    $4])
  fi],[enableval=$5 ifelse($4,,,[
  $4
])dnl
  ])])dnl
dnl ---------------------------------------------------------------------------
dnl Check if curses.h defines TRUE/FALSE (it does under SVr4).
AC_DEFUN([CF_BOOL_DEFS],
[
AC_MSG_CHECKING(if TRUE/FALSE are defined)
AC_CACHE_VAL(cf_cv_bool_defs,[
AC_TRY_COMPILE([
#include <$cf_cv_ncurses_header>
#include <stdio.h>],[int x = TRUE, y = FALSE],
	[cf_cv_bool_defs=yes],
	[cf_cv_bool_defs=no])])
AC_MSG_RESULT($cf_cv_bool_defs)
if test "$cf_cv_bool_defs" = no ; then
	AC_DEFINE(TRUE,(1))
	AC_DEFINE(FALSE,(0))
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check if curses supports color.  (Note that while SVr3 curses supports
dnl color, it does this differently from SVr4 curses; more work would be needed
dnl to accommodate SVr3).
dnl 
AC_DEFUN([CF_COLOR_CURSES],
[
AC_MSG_CHECKING(if curses supports color attributes)
AC_CACHE_VAL(cf_cv_color_curses,[
	AC_TRY_LINK([
#include <$cf_cv_ncurses_header>
],
	[has_colors();
	 start_color();
#ifndef NCURSES_BROKEN
	 wbkgd(curscr, getbkgd(stdscr)); /* X/Open XPG4 aka SVr4 Curses */
#endif
	],
	[cf_cv_color_curses=yes],
	[cf_cv_color_curses=no])
	])
AC_MSG_RESULT($cf_cv_color_curses)
test $cf_cv_color_curses = yes && AC_DEFINE(COLOR_CURSES)
])
dnl ---------------------------------------------------------------------------
dnl Look for the curses libraries.  Older curses implementations may require
dnl termcap/termlib to be linked as well.
AC_DEFUN([CF_CURSES_LIBS],[
AC_CHECK_FUNC(initscr,,[
case $host_os in #(vi
freebsd*)
	AC_CHECK_LIB(mytinfo,tgoto,[LIBS="-lmytinfo $LIBS"])
	;;
esac
if test -d /usr/5lib ; then
	# SunOS 3.x or 4.x
	CPPFLAGS="$CPPFLAGS -I/usr/5include"
	LIBS="$LIBS -L/usr/5lib"
# elif test -d /usr/ccs/lib ; then
# 	# Solaris 5.x
# 	LIBS="$LIBS -L/usr/ccs/lib -R/usr/ccs/lib"
fi

cf_save_LIBS="$LIBS"
AC_CHECK_FUNC(tgoto,[
	AC_CHECK_LIB(curses,initscr,,[
		AC_ERROR(cannot link curses)])
],[
AC_CHECK_LIB(termcap, tgoto,[
	LIBS="-ltermcap $cf_save_LIBS"
	AC_CHECK_LIB(curses,initscr,,[
		AC_CHECK_LIB(cursesX,initscr,,[
			AC_CHECK_LIB(jcurses,initscr,,[
				AC_ERROR(cannot link curses)])])])
	],[
	AC_CHECK_LIB(curses,initscr,,[
		AC_ERROR(cannot link curses)])])
])
])])
dnl ---------------------------------------------------------------------------
dnl Solaris 2.x curses provides a "performance" tradeoff according to whether
dnl CURS_PERFORMANCE is defined.  If defined, the implementation defines macros
dnl that access the WINDOW structure.  Otherwise, function calls are used.
AC_DEFUN([CF_CURS_PERFORMANCE],
[
AC_MSG_CHECKING([for curses performance tradeoff])
AC_CACHE_VAL(cf_cv_curs_performance,[
    cf_cv_curs_performance=no
    AC_TRY_COMPILE([
#include <$cf_cv_ncurses_header>],[
#if defined(wbkgdset) && defined(clearok) && defined(getbkgd)
	int x = ERR;
#else
	int x = ;	/* force an error */
#endif
	],[
	AC_TRY_COMPILE([
#define CURS_PERFORMANCE
#include <$cf_cv_ncurses_header>],[
#if defined(wbkgdset) && defined(clearok) && defined(getbkgd)
	int x = ;	/* force an error */
#else
	int x = ERR;
#endif
	],[cf_cv_curs_performance=yes])])])
AC_MSG_RESULT($cf_cv_curs_performance)
test $cf_cv_curs_performance = yes && AC_DEFINE(CURS_PERFORMANCE)
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for a program in the given list $3, defining the corresponding
dnl program variable $2.
dnl
AC_DEFUN([CF_DEFINE_PROG],[
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL(cf_cv_$2,[
	cf_cv_$2=unknown
	for cv_path in $3
	do
		if test -f $cv_path ; then
			cf_cv_$2=$cv_path
			break
		fi
	done
	])
AC_MSG_RESULT($cf_cv_$2)
AC_DEFINE_UNQUOTED($2,"$cf_cv_$2")
])
dnl ---------------------------------------------------------------------------
dnl
AC_DEFUN([CF_ERRNO],
[
AC_MSG_CHECKING([for errno external decl])
AC_CACHE_VAL(cf_cv_extern_errno,[
    AC_TRY_COMPILE([
#include <errno.h>],
        [int x = errno],
        [cf_cv_extern_errno=yes],
        [cf_cv_extern_errno=no])])
AC_MSG_RESULT($cf_cv_extern_errno)
test $cf_cv_extern_errno = no && AC_DEFINE(DECL_ERRNO)
])dnl
dnl ---------------------------------------------------------------------------
AC_DEFUN([CF_FANCY_CURSES],
[
AC_MSG_CHECKING(if curses supports fancy attributes)
AC_CACHE_VAL(cf_cv_fancy_curses,[
	AC_TRY_LINK([
#include <$cf_cv_ncurses_header>
],
	[attrset(A_UNDERLINE|A_BOLD|A_REVERSE);
	 wattrset(stdscr, A_BLINK|A_DIM);
	 attroff(A_BOLD);
	 keypad(stdscr,TRUE);
	],
	[cf_cv_fancy_curses=yes],
	[cf_cv_fancy_curses=no])
	])
AC_MSG_RESULT($cf_cv_fancy_curses)
test $cf_cv_fancy_curses = yes && AC_DEFINE(FANCY_CURSES)
])
dnl ---------------------------------------------------------------------------
dnl Look for a non-standard library, given parameters for AC_TRY_LINK.  We
dnl prefer a standard location, and use -L options only if we do not find the
dnl library in the standard library location(s).
dnl	$1 = library name
dnl	$2 = includes
dnl	$3 = code fragment to compile/link
dnl	$4 = corresponding function-name
AC_DEFUN([CF_FIND_LIBRARY],
[
	cf_cv_have_lib_$1=no
	AC_CHECK_FUNC($4,cf_cv_have_lib_$1=no,[
		cf_save_LIBS="$LIBS"
		AC_MSG_CHECKING(for $4 in -l$1)
		LIBS="-l$1 $LIBS"
		AC_TRY_LINK([$2],[$3],
			[AC_MSG_RESULT(yes)
			 cf_cv_have_lib_$1=yes
			],
			[AC_MSG_RESULT(no)
			CF_LIBRARY_PATH(cf_search,$1)
			for cf_libdir in $cf_search
			do
				AC_MSG_CHECKING(for -l$1 in $cf_libdir)
				LIBS="-L$cf_libdir -l$1 $cf_save_LIBS"
				AC_TRY_LINK([$2],[$3],
					[AC_MSG_RESULT(yes)
			 		 cf_cv_have_lib_$1=yes
					 break],
					[AC_MSG_RESULT(no)
					 LIBS="$cf_save_LIBS"])
			done
			])
		])
if test $cf_cv_have_lib_$1 = no ; then
	AC_ERROR(Cannot link $1 library)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-pedantic
dnl
dnl FIXME: the following are useful, but Lynx's not there yet
dnl	-Wcast-qual
dnl	-Wwrite-strings
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[EXTRA_CFLAGS=""
if test -n "$GCC"
then
	changequote(,)dnl
	cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[]) { return argv[argc-1] == 0; }
EOF
	changequote([,])dnl
	AC_CHECKING([for gcc warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="-W -Wall"
	for cf_opt in \
		Wbad-function-cast \
		Wcast-align \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	rm -f conftest*
	CFLAGS="$cf_save_CFLAGS"
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Construct a search-list for a nonstandard header-file
AC_DEFUN([CF_HEADER_PATH],
[$1=""
if test -d "$includedir"  ; then
test "$includedir" != NONE       && $1="[$]$1 $includedir $includedir/$2"
fi
if test -d "$oldincludedir"  ; then
test "$oldincludedir" != NONE    && $1="[$]$1 $oldincludedir $oldincludedir/$2"
fi
if test -d "$prefix"; then
test "$prefix" != NONE           && $1="[$]$1 $prefix/include $prefix/include/$2"
fi
test "$prefix" != /usr/local     && $1="[$]$1 /usr/local/include /usr/local/include/$2"
test "$prefix" != /usr           && $1="[$]$1 /usr/include /usr/include/$2"
])dnl
dnl ---------------------------------------------------------------------------
dnl Construct a search-list for a nonstandard library-file
AC_DEFUN([CF_LIBRARY_PATH],
[$1=""
if test -d "$libdir"  ; then
test "$libdir" != NONE           && $1="[$]$1 $libdir $libdir/$2"
fi
if test -d "$exec_prefix"; then
test "$exec_prefix" != NONE      && $1="[$]$1 $exec_prefix/lib $exec_prefix/lib/$2"
fi
if test -d "$prefix"; then
test "$prefix" != NONE           && \
test "$prefix" != "$exec_prefix" && $1="[$]$1 $prefix/lib $prefix/lib/$2"
fi
test "$prefix" != /usr/local     && $1="[$]$1 /usr/local/lib /usr/local/lib/$2"
test "$prefix" != /usr           && $1="[$]$1 /usr/lib /usr/lib/$2"
])dnl
dnl ---------------------------------------------------------------------------
dnl Check if we've got setlocale() and its header, <locale.h>
AC_DEFUN([CF_LOCALE],
[
AC_MSG_CHECKING(for setlocale())
AC_CACHE_VAL(cf_cv_locale,[
AC_TRY_LINK([#include <locale.h>],
	[setlocale(LC_ALL, "")],
	[cf_cv_locale=yes],
	[cf_cv_locale=no])
	])
AC_MSG_RESULT($cf_cv_locale)
test $cf_cv_locale = yes && AC_DEFINE(LOCALE)
])
dnl ---------------------------------------------------------------------------
dnl Check for the use of 'include' in 'make' (BSDI is a special case)
dnl The symbol $ac_make is set in AC_MAKE_SET, as a side-effect.
AC_DEFUN([CF_MAKE_INCLUDE],
[
AC_MSG_CHECKING(for style of include in makefiles)
make_include_left=""
make_include_right=""
make_include_quote="unknown"
for cf_include in "include" ".include" "!include"
do
	for cf_quote in '' '"'
	do
		cat >WWW/Library/unix/makefile <<CF_EOF
SHELL=/bin/sh
${cf_include} ${cf_quote}../../Library/Implementation/Version.make${cf_quote}
all :
	@echo 'cf_make_include=OK'
CF_EOF
	cf_make_include=""
	eval `cd WWW/Library/unix && ${MAKE-make} 2>&5 | grep cf_make_include=OK`
	if test -n "$cf_make_include"; then
		make_include_left="$cf_include"
		make_include_quote="$cf_quote"
		break
	else
		echo Tried 1>&5
		cat WWW/Library/unix/makefile 1>&5
	fi
	done
	test -n "$cf_make_include" && break
done

if test -z "$make_include_left" ; then
	AC_ERROR(Your $ac_make program does not support includes)
fi
if test ".$make_include_quote" != .unknown ; then
	make_include_left="$make_include_left $make_include_quote"
	make_include_right="$make_include_quote"
fi
AC_MSG_RESULT(${make_include_left}file${make_include_right})
AC_SUBST(make_include_left)
AC_SUBST(make_include_right)
])
dnl ---------------------------------------------------------------------------
dnl Check for pre-1.9.9g ncurses (among other problems, the most obvious is
dnl that color combinations don't work).
AC_DEFUN([CF_NCURSES_BROKEN],
[
if test "$cf_cv_ncurses_version" = yes ; then
AC_CACHE_VAL(cf_cv_ncurses_broken,[
AC_TRY_COMPILE([
#include <$cf_cv_ncurses_header>],[
#if defined(NCURSES_VERSION) && defined(wgetbkgd)
	make an error
#else
	int x = 1
#endif
],
	[cf_cv_ncurses_broken=no],
	[cf_cv_ncurses_broken=yes])
])
if test "$cf_cv_ncurses_broken" = yes ; then
	AC_MSG_WARN(hmm... you should get an up-to-date version of ncurses)
	AC_DEFINE(NCURSES_BROKEN)
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Look for the SVr4 curses clone 'ncurses' in the standard places, adjusting
dnl the CPPFLAGS variable.
dnl
dnl The header files may be installed as either curses.h, or ncurses.h
dnl (obsolete).  If not installed for overwrite, the curses.h file would be
dnl in an ncurses subdirectory (e.g., /usr/include/ncurses), but someone may
dnl have installed overwriting the vendor's curses.  Only very old versions
dnl (pre-1.9.2d, the first autoconf'd version) of ncurses don't define
dnl either __NCURSES_H or NCURSES_VERSION in the header.
dnl
dnl If the installer has set $CFLAGS or $CPPFLAGS so that the ncurses header
dnl is already in the include-path, don't even bother with this, since we cannot
dnl easily determine which file it is.  In this case, it has to be <curses.h>.
dnl
AC_DEFUN([CF_NCURSES_CPPFLAGS],
[
AC_MSG_CHECKING(for ncurses header file)
AC_CACHE_VAL(cf_cv_ncurses_header,[
	AC_TRY_COMPILE([#include <curses.h>],
	[printf("%s\n", NCURSES_VERSION)],
	[cf_cv_ncurses_header=predefined],[
	CF_HEADER_PATH(cf_search,ncurses)
	test -n "$verbose" && echo
	for cf_incdir in $cf_search
	do
		for cf_header in \
			curses.h \
			ncurses.h
		do
			if egrep "NCURSES" $cf_incdir/$cf_header 1>&5 2>&1; then
				cf_cv_ncurses_header=$cf_incdir/$cf_header 
				test -n "$verbose" && echo $ac_n "	... found $ac_c" 1>&6
				break
			fi
			test -n "$verbose" && echo "	... tested $cf_incdir/$cf_header" 1>&6
		done
		test -n "$cf_cv_ncurses_header" && break
	done
	test -z "$cf_cv_ncurses_header" && AC_ERROR(not found)
	])])
AC_MSG_RESULT($cf_cv_ncurses_header)
AC_DEFINE(NCURSES)

changequote(,)dnl
cf_incdir=`echo $cf_cv_ncurses_header | sed -e 's:/[^/]*$::'`
changequote([,])dnl

case $cf_cv_ncurses_header in # (vi
*/ncurses.h)
	AC_DEFINE(NCURSESHEADER)
	;;
esac

case $cf_cv_ncurses_header in # (vi
predefined) # (vi
	cf_cv_ncurses_header=curses.h
	;;
*)
	CF_ADD_INCDIR($cf_incdir)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl Look for the ncurses library.  This is a little complicated on Linux,
dnl because it may be linked with the gpm (general purpose mouse) library.
dnl Some distributions have gpm linked with (bsd) curses, which makes it
dnl unusable with ncurses.  However, we don't want to link with gpm unless
dnl ncurses has a dependency, since gpm is normally set up as a shared library,
dnl and the linker will record a dependency.
AC_DEFUN([CF_NCURSES_LIBS],
[AC_REQUIRE([CF_NCURSES_CPPFLAGS])

cf_ncurses_LIBS=""
AC_CHECK_LIB(gpm,Gpm_Open,[AC_CHECK_LIB(gpm,initscr,,[cf_ncurses_LIBS="-lgpm"])])

case $host_os in #(vi
freebsd*)
	# This is only necessary if you are linking against an obsolete
	# version of ncurses (but it should do no harm, since it's static).
	AC_CHECK_LIB(mytinfo,tgoto,[cf_ncurses_LIBS="-lmytinfo $cf_ncurses_LIBS"])
	;;
esac

LIBS="$cf_ncurses_LIBS $LIBS"
CF_FIND_LIBRARY(ncurses,
	[#include <$cf_cv_ncurses_header>],
	[initscr()],
	initscr)

if test -n "$cf_ncurses_LIBS" ; then
	AC_MSG_CHECKING(if we can link ncurses without $cf_ncurses_LIBS)
	cf_ncurses_SAVE="$LIBS"
	for p in $cf_ncurses_LIBS ; do
		q=`echo $LIBS | sed -e 's/'$p' //' -e 's/'$p'$//'`
		if test "$q" != "$LIBS" ; then
			LIBS="$q"
		fi
	done
	AC_TRY_LINK([#include <$cf_cv_ncurses_header>],
		[initscr()],
		[AC_MSG_RESULT(yes)],
		[AC_MSG_RESULT(no)
		 LIBS="$cf_ncurses_SAVE"])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for the version of ncurses, to aid in reporting bugs, etc.
AC_DEFUN([CF_NCURSES_VERSION],
[AC_MSG_CHECKING(for ncurses version)
AC_CACHE_VAL(cf_cv_ncurses_version,[
	cf_cv_ncurses_version=no
	cat > conftest.$ac_ext <<EOF
#include <$cf_cv_ncurses_header>
#ifdef NCURSES_VERSION
Autoconf NCURSES_VERSION
#else
#ifdef __NCURSES_H
Autoconf "old"
#endif
#endif
EOF
	cf_try="$ac_cpp conftest.$ac_ext 2>&5 | grep '^Autoconf ' >conftest.out"
	AC_TRY_EVAL(cf_try)
	if test -f conftest.out ; then
changequote(,)dnl
		cf_out=`cat conftest.out | sed -e 's@^[^\"]*\"@@' -e 's@\".*@@'`
changequote([,])dnl
		test -n "$cf_out" && cf_cv_ncurses_version="$cf_out"
	fi
])
AC_MSG_RESULT($cf_cv_ncurses_version)
])
dnl ---------------------------------------------------------------------------
dnl After checking for functions in the default $LIBS, make a further check
dnl for the functions that are netlib-related (these aren't always in the
dnl libc, etc., and have to be handled specially because there are conflicting
dnl and broken implementations.
dnl Common library requirements (in order):
dnl	-lresolv -lsocket -lnsl
dnl	-lnsl -lsocket
dnl	-lsocket
dnl	-lbsd
AC_DEFUN([CF_NETLIBS],[
NETLIBS=""
cf_have_lsocket=no
#
AC_CHECK_FUNC(gethostname,[AC_DEFINE(HAVE_GETHOSTNAME)],[
	AC_CHECK_LIB(nsl,gethostname,
		[AC_DEFINE(HAVE_GETHOSTNAME)
		NETLIBS="-lnsl $NETLIBS"],
		AC_CHECK_LIB(socket,gethostname,
		[AC_DEFINE(HAVE_GETHOSTNAME)
		NETLIBS="-lsocket $NETLIBS"
		cf_have_lsocket=yes]),
		[$NETLIBS])])
#
# FIXME:  sequent needs this library (i.e., -lsocket -linet -lnsl), but
# I don't know the entrypoints - 97/7/22 TD
AC_HAVE_LIBRARY(inet,NETLIBS="-linet $NETLIBS")
#
if test $cf_have_lsocket = no ; then
AC_CHECK_FUNC(socket,[AC_DEFINE(HAVE_SOCKET)],[
	AC_CHECK_LIB(socket,socket,
		[AC_DEFINE(HAVE_SOCKET)
		NETLIBS="-lsocket $NETLIBS"],
		AC_CHECK_LIB(bsd,socket,
			[AC_DEFINE(HAVE_SOCKET)
			NETLIBS="-lbsd $NETLIBS"]),
		[$NETLIBS])])
fi
#
AC_CHECK_FUNC(gethostbyname,[AC_DEFINE(HAVE_GETHOSTBYNAME)],[
	AC_CHECK_LIB(nsl,gethostbyname,
		[AC_DEFINE(HAVE_GETHOSTBYNAME)
		NETLIBS="-lnsl $NETLIBS"],,
		[$NETLIBS])])
#
AC_CHECK_FUNC(strcasecmp,[AC_DEFINE(HAVE_STRCASECMP)],[
	AC_CHECK_LIB(resolv,strcasecmp,
		[AC_DEFINE(HAVE_STRCASECMP)
		NETLIBS="-lresolv $NETLIBS"],,
		[$NETLIBS])])
LIBS="$LIBS $NETLIBS"
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for the symbol NGROUPS
AC_DEFUN([CF_NGROUPS],
[
AC_MSG_CHECKING(if NGROUPS is defined)
AC_CACHE_VAL(cf_cv_ngroups,[
AC_TRY_COMPILE([
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
],[int x = NGROUPS],
	[cf_cv_ngroups=yes],
	[AC_TRY_COMPILE([
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
],[int x = NGROUPS_MAX],
		[cf_cv_ngroups=NGROUPS_MAX],
		[cf_cv_ngroups=no])
	])
AC_MSG_RESULT($cf_cv_ngroups)
if test "$cf_cv_ngroups" = no ; then
	AC_DEFINE(NGROUPS,16)
elif test "$cf_cv_ngroups" = NGROUPS_MAX ; then
	AC_DEFINE(NGROUPS,NGROUPS_MAX)
fi
])
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for a given program, defining corresponding symbol.
dnl	$1 = environment variable, which is suffixed by "_PATH" in the #define.
dnl	$2 = program name to find.
dnl
dnl If there is more than one token in the result, #define the remaining tokens
dnl to $1_ARGS.  We need this for 'install' in particular.
dnl
dnl FIXME: we should allow this to be overridden by environment variables
dnl
AC_DEFUN([CF_PATH_PROG],[
test -z "[$]$1" && $1=$2
if test "$with_full_paths" = yes ; then
	AC_PATH_PROG($1,$2,[$]$1)
else
	AC_MSG_CHECKING(for $2)
	AC_MSG_RESULT([$]$1)
fi

cf_path_prog=""
cf_path_args=""
IFS="${IFS= 	}"; cf_save_ifs="$IFS"; IFS="${IFS}:"
for cf_temp in $ac_cv_path_$1
do
	if test -z "$cf_path_prog" ; then
		cf_path_prog="$cf_temp"
	elif test -z "$cf_path_args" ; then
		cf_path_args="$cf_temp"
	else
		cf_path_args="$cf_path_args $cf_temp"
	fi
done
IFS="$cf_save_ifs"

AC_DEFINE_UNQUOTED($1_PATH,"$cf_path_prog")
test -n "$cf_path_args" && AC_DEFINE_UNQUOTED($1_ARGS,"$cf_path_args")
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for broken definition of 'remove()'.  This is (in particular) broken
dnl on the older version of SCO (I'd cite version if I knew where to look) by
dnl having <stdio.h> #define remove to __unlink, which appears in no library.
dnl
dnl Fortuitously, we can combine this with a more general test: do we have
dnl 'unlink()' but no 'remove()'.  Note, however, that we cannot simply #define
dnl remove to unlink, but have to make a fallback function.
dnl
AC_DEFUN([CF_REMOVE_BROKEN],
[
AC_MSG_CHECKING(for broken/missing definition of remove)
AC_CACHE_VAL(cf_cv_baddef_remove,[
AC_TRY_LINK(
	[#include <stdio.h>],
	[remove("dummy")],
	[cf_cv_baddef_remove=no],
	[AC_TRY_LINK(
		[#include <stdio.h>
		int __unlink(name) { return unlink(name); } ],
		[remove("dummy")],
		[cf_cv_baddef_remove=yes],
		[cf_cv_baddef_remove=unknown])
	])
])
AC_MSG_RESULT($cf_cv_baddef_remove)
test "$cf_cv_baddef_remove" = yes && AC_DEFINE(NEED_REMOVE)
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for definitions & structures needed for window size-changing
dnl FIXME: check that this works with "snake" (HP-UX 10.x)
AC_DEFUN([CF_SIZECHANGE],
[
AC_MSG_CHECKING([declaration of size-change])
AC_CACHE_VAL(cf_cv_sizechange,[
    cf_cv_sizechange=unknown
    cf_save_CFLAGS="$CFLAGS"

for cf_opts in "" "NEED_PTEM_H"
do

    CFLAGS="$cf_save_CFLAGS"
    test -n "$cf_opts" && CFLAGS="$CFLAGS -D$cf_opts"
    AC_TRY_COMPILE([#include <sys/types.h>
#if HAVE_TERMIOS_H
#include <termios.h>
#endif
#if NEED_PTEM_H
/* This is a workaround for SCO:  they neglected to define struct winsize in
 * termios.h -- it's only in termio.h and ptem.h
 */
#include        <sys/stream.h>
#include        <sys/ptem.h>
#endif
#if !defined(sun) || !defined(HAVE_TERMIOS_H)
#include <sys/ioctl.h>
#endif
],[
#ifdef TIOCGSIZE
	struct ttysize win;	/* FIXME: what system is this? */
	int y = win.ts_lines;
	int x = win.ts_cols;
#else
#ifdef TIOCGWINSZ
	struct winsize win;
	int y = win.ws_row;
	int x = win.ws_col;
#else
	no TIOCGSIZE or TIOCGWINSZ
#endif /* TIOCGWINSZ */
#endif /* TIOCGSIZE */
	],
	[cf_cv_sizechange=yes],
	[cf_cv_sizechange=no])

	CFLAGS="$cf_save_CFLAGS"
	if test "$cf_cv_sizechange" = yes ; then
		test -n "$cf_opts" && AC_DEFINE($cf_opts)
		break
	fi
done
	])
AC_MSG_RESULT($cf_cv_sizechange)
test $cf_cv_sizechange != no && AC_DEFINE(HAVE_SIZECHANGE)
])dnl
dnl ---------------------------------------------------------------------------
dnl Look for the slang header files in the standard places, adjusting the
dnl CPPFLAGS variable.
dnl
AC_DEFUN([CF_SLANG_CPPFLAGS],
[
AC_MSG_CHECKING(for slang header file)
AC_CACHE_VAL(cf_cv_slang_header,[
	AC_TRY_COMPILE([#include <slang.h>],
	[printf("%s\n", SLANG_VERSION)],
	[cf_cv_slang_header=predefined],[
	CF_HEADER_PATH(cf_search,slang)
	for cf_incdir in $cf_search
	do
		for cf_header in \
			slang.h
		do
			if egrep "SLANG_VERSION" $cf_incdir/$cf_header 1>&5 2>&1; then
				cf_cv_slang_header=$cf_incdir/$cf_header 
				break
			fi
		done
		test -n "$cf_cv_slang_header" && break
	done
	test -z "$cf_cv_slang_header" && AC_ERROR(not found)
	])])
AC_MSG_RESULT($cf_cv_slang_header)
AC_DEFINE(USE_SLANG)

changequote(,)dnl
cf_incdir=`echo $cf_cv_slang_header | sed -e 's:/[^/]*$::'`
changequote([,])dnl

case $cf_cv_slang_header in # (vi
predefined) # (vi
	;;
*)
	CF_ADD_INCDIR($cf_incdir)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl Look for the slang library.
AC_DEFUN([CF_SLANG_LIBS],
[
dnl AC_CHECK_FUNC(acos,,[AC_CHECK_LIB(m,acos,[LIBS="-lm $LIBS"])])
CF_FIND_LIBRARY(slang,
	[#include <slang.h>],
	[SLtt_get_screen_size()],
	SLtt_get_screen_size)
])dnl
dnl ---------------------------------------------------------------------------
dnl	Remove "-g" option from the compiler options
AC_DEFUN([CF_STRIP_G_OPT],
[$1=`echo ${$1} | sed -e 's/-g //' -e 's/-g$//'`])dnl
dnl ---------------------------------------------------------------------------
dnl	Remove "-O" option from the compiler options
AC_DEFUN([CF_STRIP_O_OPT],[
changequote(,)dnl
$1=`echo ${$1} | sed -e 's/-O[1-9]\? //' -e 's/-O[1-9]\?$//'`
changequote([,])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for declaration of sys_errlist in one of stdio.h and errno.h.
dnl Declaration of sys_errlist on BSD4.4 interferes with our declaration.
dnl Reported by Keith Bostic.
AC_DEFUN([CF_SYS_ERRLIST],
[
AC_MSG_CHECKING([declaration of sys_errlist])
AC_CACHE_VAL(cf_cv_dcl_sys_errlist,[
    AC_TRY_COMPILE([
#include <stdio.h>
#include <sys/types.h>
#include <errno.h> ],
    [char *c = (char *) *sys_errlist],
    [cf_cv_dcl_sys_errlist=yes],
    [cf_cv_dcl_sys_errlist=no])])
AC_MSG_RESULT($cf_cv_dcl_sys_errlist)
test $cf_cv_dcl_sys_errlist = no && AC_DEFINE(DECL_SYS_ERRLIST)
])dnl
dnl ---------------------------------------------------------------------------
AC_DEFUN([CF_SYSTEM_MAIL_FLAGS], 
[
AC_MSG_CHECKING([system mail flags])
AC_CACHE_VAL(cf_cv_system_mail_flags,[
	case $cf_cv_SYSTEM_MAIL in
	*/mmdf/*)
		[cf_cv_system_mail_flags="-mlruxto,cc\\\\*"]
        	;; 
	*)
        	[cf_cv_system_mail_flags="-t -oi"]
	esac
	])
AC_MSG_RESULT($cf_cv_system_mail_flags)
AC_DEFINE_UNQUOTED(SYSTEM_MAIL_FLAGS, "$cf_cv_system_mail_flags")
])dnl
dnl ---------------------------------------------------------------------------
dnl Look for termcap libraries, needed by some versions of slang.
AC_DEFUN([CF_TERMCAP_LIBS],
[
AC_CACHE_VAL(cf_cv_lib_termcap,[
cf_cv_lib_termcap=none
# HP-UX 9.x terminfo has setupterm, but no tigetstr.
if test "$termlib" = none; then
	AC_CHECK_LIB(termlib, tigetstr, [LIBS="$LIBS -ltermlib" cf_cv_lib_termcap=terminfo])
fi
if test "$cf_cv_lib_termcap" = none; then
	AC_CHECK_LIB(termlib, tgoto, [LIBS="$LIBS -ltermlib" cf_cv_lib_termcap=termcap])
fi
if test "$cf_cv_lib_termcap" = none; then
	# allow curses library for broken AIX system.
	AC_CHECK_LIB(curses, initscr, [LIBS="$LIBS -lcurses" cf_cv_lib_termcap=termcap])
	AC_CHECK_LIB(termcap, tgoto, [LIBS="$LIBS -ltermcap" cf_cv_lib_termcap=termcap])
fi
if test "$cf_cv_lib_termcap" = none; then
	AC_CHECK_LIB(termcap, tgoto, [LIBS="$LIBS -ltermcap" cf_cv_lib_termcap=termcap])
fi
if test "$cf_cv_lib_termcap" = none; then
	AC_CHECK_LIB(ncurses, tgoto, [LIBS="$LIBS -lncurses" cf_cv_lib_termcap=ncurses])
fi
])
if test "$cf_cv_lib_termcap" = none; then
	AC_ERROR([Can't find -ltermlib, -lcurses, or -ltermcap])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check if including both termio.h and termios.h die like on DG.UX
AC_DEFUN([CF_TERMIO_AND_TERMIOS],
[
AC_MSG_CHECKING([termio.h and termios.h])
AC_CACHE_VAL(cf_cv_termio_and_termios,[
    AC_TRY_COMPILE([
#if HAVE_TERMIO_H
#include <termio.h>
#endif
#if HAVE_TERMIOS_H
#include <termios.h>
#endif ],
    [putchar (0x0a)],
    [cf_cv_termio_and_termios=yes],
    [cf_cv_termio_and_termios=no])])
AC_MSG_RESULT($cf_cv_termio_and_termios)
test $cf_cv_termio_and_termios = no && AC_DEFINE(TERMIO_AND_TERMIOS)
])dnl
dnl ---------------------------------------------------------------------------
AC_DEFUN([CF_TTYTYPE],
[
AC_MSG_CHECKING(if ttytype is declared in curses library)
AC_CACHE_VAL(cf_cv_have_ttytype,[
	AC_TRY_LINK([#include <$cf_cv_ncurses_header>],
	[char *x = &ttytype[1]; *x = 1],
	[cf_cv_have_ttytype=yes],
	[cf_cv_have_ttytype=no])
	])
AC_MSG_RESULT($cf_cv_have_ttytype)
test $cf_cv_have_ttytype = yes && AC_DEFINE(HAVE_TTYTYPE)
])
dnl ---------------------------------------------------------------------------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
changequote(,)dnl
$1=`echo $2 | tr '[a-z]' '[A-Z]'`
changequote([,])dnl
])dnl
dnl ---------------------------------------------------------------------------
AC_DEFUN([CF_UTMP],
[
AC_MSG_CHECKING(if struct utmp is declared)
AC_CACHE_VAL(cf_cv_have_utmp,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <utmp.h>],
	[struct utmp x; char *y = &x.ut_host[0]],
	[cf_cv_have_utmp=yes],
	[AC_TRY_COMPILE([#include <utmpx.h>],
		[struct utmpx x; char *y = &x.ut_host[0]],
		[cf_cv_have_utmp=utmpx],
		[cf_cv_have_utmp=no])
		])
	])
AC_MSG_RESULT($cf_cv_have_utmp)
test $cf_cv_have_utmp != no && AC_DEFINE(HAVE_UTMP)
test $cf_cv_have_utmp = utmpx && AC_DEFINE(UTMPX_FOR_UTMP)
])
