dnl $LynxId: aclocal.m4,v 1.304 2021/07/10 16:22:27 tom Exp $
dnl Macros for auto-configure script.
dnl by Thomas E. Dickey <dickey@invisible-island.net>
dnl and Jim Spath <jspath@mail.bcpl.lib.md.us>
dnl and Philippe De Muyter <phdm@macqel.be>
dnl
dnl Created: 1997/01/28
dnl
dnl The autoconf used in Lynx development is GNU autoconf 2.13 or 2.52, patched
dnl by Thomas Dickey.  See your local GNU archives, and this URL:
dnl https://invisible-island.net/autoconf/autoconf.html
dnl
dnl ---------------------------------------------------------------------------
dnl
dnl Copyright 1997-2020,2021 by Thomas E. Dickey
dnl
dnl Permission to use, copy, modify, and distribute this software and its
dnl documentation for any purpose and without fee is hereby granted,
dnl provided that the above copyright notice appear in all copies and that
dnl both that copyright notice and this permission notice appear in
dnl supporting documentation, and that the name of the above listed
dnl copyright holder(s) not be used in advertising or publicity pertaining
dnl to distribution of the software without specific, written prior
dnl permission.
dnl
dnl THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD
dnl TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
dnl AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE
dnl LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
dnl WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
dnl ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
dnl OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
dnl
dnl ---------------------------------------------------------------------------
dnl A few macros (the AM_xxx ones) are originally from gettext 0.10.35 (but
dnl are modified), and are part of this file due to mechanical limitations of
dnl autoconf.
dnl ---------------------------------------------------------------------------
dnl See
dnl     https://invisible-island.net/autoconf/autoconf.html
dnl     https://invisible-island.net/autoconf/my-autoconf.html
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl AM_GNU_GETTEXT version: 15 updated: 2021/01/02 09:31:20
dnl --------------
dnl Usage: Just like AM_WITH_NLS, which see.
AC_DEFUN([AM_GNU_GETTEXT],
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_CANONICAL_HOST])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl
   AC_REQUIRE([jm_GLIBC21])dnl
   AC_REQUIRE([CF_PROG_CC])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h stddef.h \
stdlib.h string.h unistd.h sys/param.h])
   AC_CHECK_FUNCS([feof_unlocked fgets_unlocked getcwd getegid geteuid \
getgid getuid mempcpy munmap putenv setenv setlocale stpcpy strchr strcasecmp \
strdup strtoul tsearch __argz_count __argz_stringify __argz_next])

   AM_ICONV
   AM_LANGINFO_CODESET
   AM_LC_MESSAGES
   AM_WITH_NLS([$1],[$2],[$3],[$4])

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
       for presentlang in $ALL_LINGUAS; do
         useit=no
         for desiredlang in ${LINGUAS-$ALL_LINGUAS}; do
           # Use the presentlang catalog if desiredlang is
           #   a. equal to presentlang, or
           #   b. a variant of presentlang (because in this case,
           #      presentlang can be used as a fallback for messages
           #      which are not translated in the desiredlang catalog).
           case "$desiredlang" in
             ("$presentlang"*) useit=yes;;
           esac
         done
         if test "$useit" = yes; then
           NEW_LINGUAS="$NEW_LINGUAS $presentlang"
         fi
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

   dnl Enable libtool support if the surrounding package wishes it.
   INTL_LIBTOOL_SUFFIX_PREFIX=ifelse([$1], use-libtool, [l], [])
   AC_SUBST(INTL_LIBTOOL_SUFFIX_PREFIX)
])dnl
dnl ---------------------------------------------------------------------------
dnl AM_ICONV version: 12 updated: 2007/07/30 19:12:03
dnl --------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl iconv.m4
dnl ====================
dnl serial AM2
dnl
dnl From Bruno Haible.
dnl
dnl ====================
dnl Modified to use CF_FIND_LINKAGE and CF_ADD_SEARCHPATH, to broaden the
dnl range of locations searched.  Retain the same cache-variable naming to
dnl allow reuse with the other gettext macros -Thomas E Dickey
AC_DEFUN([AM_ICONV],
[
  dnl Some systems have iconv in libc, some have it in libiconv (OSF/1 and
  dnl those with the standalone portable GNU libiconv installed).

  AC_ARG_WITH([libiconv-prefix],
[  --with-libiconv-prefix=DIR
                          search for libiconv in DIR/include and DIR/lib], [
    CF_ADD_OPTIONAL_PATH($withval, libiconv)
   ])

  AC_CACHE_CHECK(for iconv, am_cv_func_iconv, [
    CF_FIND_LINKAGE(CF__ICONV_HEAD,
      CF__ICONV_BODY,
      iconv,
      am_cv_func_iconv=yes,
      am_cv_func_iconv=["no, consider installing GNU libiconv"])])

  if test "$am_cv_func_iconv" = yes; then
    AC_DEFINE(HAVE_ICONV, 1, [Define if you have the iconv() function.])

    AC_CACHE_CHECK([if the declaration of iconv() needs const.],
		   am_cv_proto_iconv_const,[
      AC_TRY_COMPILE(CF__ICONV_HEAD [
extern
#ifdef __cplusplus
"C"
#endif
#if defined(__STDC__) || defined(__cplusplus)
size_t iconv (iconv_t cd, char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);
#else
size_t iconv();
#endif
],[], am_cv_proto_iconv_const=no,
      am_cv_proto_iconv_const=yes)])

    if test "$am_cv_proto_iconv_const" = yes ; then
      am_cv_proto_iconv_arg1="const"
    else
      am_cv_proto_iconv_arg1=""
    fi

    AC_DEFINE_UNQUOTED(ICONV_CONST, $am_cv_proto_iconv_arg1,
      [Define as const if the declaration of iconv() needs const.])
  fi

  LIBICONV=
  if test "$cf_cv_find_linkage_iconv" = yes; then
    CF_ADD_INCDIR($cf_cv_header_path_iconv)
    if test -n "$cf_cv_library_file_iconv" ; then
      LIBICONV="-liconv"
      CF_ADD_LIBDIR($cf_cv_library_path_iconv)
    fi
  fi

  AC_SUBST(LIBICONV)
])dnl
dnl ---------------------------------------------------------------------------
dnl AM_LANGINFO_CODESET version: 6 updated: 2021/01/01 16:53:59
dnl -------------------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl codeset.m4
dnl ====================
dnl serial AM1
dnl
dnl From Bruno Haible.
AC_DEFUN([AM_LANGINFO_CODESET],
[
AC_CACHE_CHECK([for nl_langinfo and CODESET], am_cv_langinfo_codeset,
	[AC_TRY_LINK([#include <langinfo.h>],
	[char* cs = nl_langinfo(CODESET); (void)cs],
	am_cv_langinfo_codeset=yes,
	am_cv_langinfo_codeset=no)
	])
	if test "$am_cv_langinfo_codeset" = yes; then
		AC_DEFINE(HAVE_LANGINFO_CODESET, 1,
		[Define if you have <langinfo.h> and nl_langinfo(CODESET).])
	fi
])dnl
dnl ---------------------------------------------------------------------------
dnl AM_LC_MESSAGES version: 6 updated: 2021/01/02 09:31:20
dnl --------------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl lcmessage.m4
dnl ====================
dnl Check whether LC_MESSAGES is available in <locale.h>.
dnl Ulrich Drepper <drepper@cygnus.com>, 1995.
dnl
dnl This file can be copied and used freely without restrictions.  It can
dnl be used in projects which are not available under the GNU General Public
dnl License or the GNU Library General Public License but which still want
dnl to provide support for the GNU gettext functionality.
dnl Please note that the actual code of the GNU gettext library is covered
dnl by the GNU Library General Public License, and the rest of the GNU
dnl gettext package package is covered by the GNU General Public License.
dnl They are *not* in the public domain.
dnl
dnl serial 2
dnl
AC_DEFUN([AM_LC_MESSAGES],
[if test "$ac_cv_header_locale_h" = yes; then
	AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
		[AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
		am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
	if test "$am_cv_val_LC_MESSAGES" = yes; then
		AC_DEFINE(HAVE_LC_MESSAGES, 1,
		[Define if your <locale.h> file defines LC_MESSAGES.])
	fi
fi])dnl
dnl ---------------------------------------------------------------------------
dnl AM_PATH_PROG_WITH_TEST version: 10 updated: 2021/01/02 09:31:20
dnl ----------------------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl progtest.m4
dnl ====================
dnl Search path for a program which passes the given test.
dnl Ulrich Drepper <drepper@cygnus.com>, 1996.
dnl
dnl This file can be copied and used freely without restrictions.  It can
dnl be used in projects which are not available under the GNU General Public
dnl License or the GNU Library General Public License but which still want
dnl to provide support for the GNU gettext functionality.
dnl Please note that the actual code of the GNU gettext library is covered
dnl by the GNU Library General Public License, and the rest of the GNU
dnl gettext package package is covered by the GNU General Public License.
dnl They are *not* in the public domain.
dnl
dnl serial 2
dnl
dnl AM_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN([AM_PATH_PROG_WITH_TEST],
[# Extract the first word of "$2", so it can be a program name with args.
AC_REQUIRE([CF_PATHSEP])
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  ([[\\/]*|?:[\\/]]*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  (*)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR}"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f "$ac_dir/$ac_word$ac_exeext" ; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word$ac_exeext"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test ifelse([$4], , [-n "[$]$1"], ["[$]$1" != "$4"]); then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl AM_WITH_NLS version: 31 updated: 2021/01/04 17:48:08
dnl -----------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl gettext.m4
dnl ====================
dnl Macro to add for using GNU gettext.
dnl Ulrich Drepper <drepper@cygnus.com>, 1995.
dnl ====================
dnl Modified to use CF_FIND_LINKAGE and CF_ADD_SEARCHPATH, to broaden the
dnl range of locations searched.  Retain the same cache-variable naming to
dnl allow reuse with the other gettext macros -Thomas E Dickey
dnl ====================
dnl
dnl This file can be copied and used freely without restrictions.  It can
dnl be used in projects which are not available under the GNU General Public
dnl License or the GNU Library General Public License but which still want
dnl to provide support for the GNU gettext functionality.
dnl Please note that the actual code of the GNU gettext library is covered
dnl by the GNU Library General Public License, and the rest of the GNU
dnl gettext package package is covered by the GNU General Public License.
dnl They are *not* in the public domain.
dnl
dnl serial 10
dnl
dnl Usage: AM_WITH_NLS([TOOLSYMBOL], [NEEDSYMBOL], [LIBDIR], [ENABLED]).
dnl If TOOLSYMBOL is specified and is 'use-libtool', then a libtool library
dnl    $(top_builddir)/intl/libintl.la will be created (shared and/or static,
dnl    depending on --{enable,disable}-{shared,static} and on the presence of
dnl    AM-DISABLE-SHARED). Otherwise, a static library
dnl    $(top_builddir)/intl/libintl.a will be created.
dnl If NEEDSYMBOL is specified and is 'need-ngettext', then GNU gettext
dnl    implementations (in libc or libintl) without the ngettext() function
dnl    will be ignored.
dnl LIBDIR is used to find the intl libraries.  If empty,
dnl    the value `$(top_builddir)/intl/' is used.
dnl ENABLED is used to control the default for the related --enable-nls, since
dnl    not all application developers want this feature by default, e.g., lynx.
dnl
dnl The result of the configuration is one of three cases:
dnl 1) GNU gettext, as included in the intl subdirectory, will be compiled
dnl    and used.
dnl    Catalog format: GNU --> install in $(datadir)
dnl    Catalog extension: .mo after installation, .gmo in source tree
dnl 2) GNU gettext has been found in the system's C library.
dnl    Catalog format: GNU --> install in $(datadir)
dnl    Catalog extension: .mo after installation, .gmo in source tree
dnl 3) No internationalization, always use English msgid.
dnl    Catalog format: none
dnl    Catalog extension: none
dnl The use of .gmo is historical (it was needed to avoid overwriting the
dnl GNU format catalogs when building on a platform with an X/Open gettext),
dnl but we keep it in order not to force irrelevant filename changes on the
dnl maintainers.
dnl
AC_DEFUN([AM_WITH_NLS],
[AC_MSG_CHECKING([whether NLS is requested])
  dnl Default is enabled NLS
  ifelse([$4],,[
  AC_ARG_ENABLE(nls,
    [  --disable-nls           do not use Native Language Support],
    USE_NLS=$enableval, USE_NLS=yes)],[
  AC_ARG_ENABLE(nls,
    [  --enable-nls            use Native Language Support],
    USE_NLS=$enableval, USE_NLS=no)])
  AC_MSG_RESULT($USE_NLS)
  AC_SUBST(USE_NLS)

  BUILD_INCLUDED_LIBINTL=no
  USE_INCLUDED_LIBINTL=no
  INTLLIBS=

  dnl If we use NLS figure out what method
  if test "$USE_NLS" = "yes"; then
    dnl We need to process the po/ directory.
    POSUB=po
    AC_DEFINE(ENABLE_NLS, 1,
      [Define to 1 if translation of program messages to the user's native language
 is requested.])
    AC_MSG_CHECKING([whether included gettext is requested])
    AC_ARG_WITH(included-gettext,
      [  --with-included-gettext use the GNU gettext library included here],
      nls_cv_force_use_gnu_gettext=$withval,
      nls_cv_force_use_gnu_gettext=no)
    AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)

    nls_cv_use_gnu_gettext="$nls_cv_force_use_gnu_gettext"
    if test "$nls_cv_force_use_gnu_gettext" != "yes"; then
      dnl User does not insist on using GNU NLS library.  Figure out what
      dnl to use.  If GNU gettext is available we use this.  Else we may have
      dnl to fall back to GNU NLS library.
      CATOBJEXT=NONE

      dnl Save these (possibly-set) variables for reference.  If the user
      dnl overrode these to provide full pathnames, then warn if not actually
      dnl GNU gettext, but do not override their values.  Also, if they were
      dnl overridden, suppress the part of the library test which prevents it
      dnl from finding anything other than GNU gettext.  Doing this also
      dnl suppresses a bogus search for the intl library.
      cf_save_msgfmt_path="$MSGFMT"
      cf_save_xgettext_path="$XGETTEXT"

      dnl Search for GNU msgfmt in the PATH.
      AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
          ["$ac_dir/$ac_word" --statistics /dev/null >/dev/null 2>&1], :)
      AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
      AC_SUBST(MSGFMT)

      dnl Search for GNU xgettext in the PATH.
      AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
          ["$ac_dir/$ac_word" --omit-header /dev/null >/dev/null 2>&1], :)

      cf_save_OPTS_1="$CPPFLAGS"
      if test "x$cf_save_msgfmt_path" = "x$MSGFMT" && \
         test "x$cf_save_xgettext_path" = "x$XGETTEXT" ; then
          CF_ADD_CFLAGS(-DIGNORE_MSGFMT_HACK)
      fi

      cf_save_LIBS_1="$LIBS"
      CF_ADD_LIBS($LIBICONV)

      CF_FIND_LINKAGE(CF__INTL_HEAD,
        CF__INTL_BODY($2),
        intl,
        cf_cv_func_gettext=yes,
        cf_cv_func_gettext=no)

      AC_MSG_CHECKING([for libintl.h and gettext()])
      AC_MSG_RESULT($cf_cv_func_gettext)

      LIBS="$cf_save_LIBS_1"
      CPPFLAGS="$cf_save_OPTS_1"

      if test "$cf_cv_func_gettext" = yes ; then
        AC_DEFINE(HAVE_LIBINTL_H,1,[Define to 1 if we have libintl.h])

        dnl If an already present or preinstalled GNU gettext() is found,
        dnl use it.  But if this macro is used in GNU gettext, and GNU
        dnl gettext is already preinstalled in libintl, we update this
        dnl libintl.  (Cf. the install rule in intl/Makefile.in.)
        if test "$PACKAGE" != gettext; then
          AC_DEFINE(HAVE_GETTEXT, 1,
              [Define if the GNU gettext() function is already present or preinstalled.])

          CF_ADD_INCDIR($cf_cv_header_path_intl)

          if test -n "$cf_cv_library_file_intl" ; then
            dnl If iconv() is in a separate libiconv library, then anyone
            dnl linking with libintl{.a,.so} also needs to link with
            dnl libiconv.
            INTLLIBS="$cf_cv_library_file_intl $LIBICONV"
            CF_ADD_LIBDIR($cf_cv_library_path_intl,INTLLIBS)
          fi

          gt_save_LIBS="$LIBS"
          LIBS="$LIBS $INTLLIBS"
          AC_CHECK_FUNCS(dcgettext)
          LIBS="$gt_save_LIBS"

          CATOBJEXT=.gmo
        fi
      elif test -z "$MSGFMT" || test -z "$XGETTEXT" ; then
        AC_MSG_WARN(disabling NLS feature)
        sed -e /ENABLE_NLS/d confdefs.h >confdefs.tmp
        mv confdefs.tmp confdefs.h
        ALL_LINGUAS=
        CATOBJEXT=.ignored
        MSGFMT=":"
        GMSGFMT=":"
        XGETTEXT=":"
        POSUB=
        BUILD_INCLUDED_LIBINTL=no
        USE_INCLUDED_LIBINTL=no
        USE_NLS=no
        nls_cv_use_gnu_gettext=no
      fi

      if test "$CATOBJEXT" = "NONE"; then
        dnl GNU gettext is not found in the C library.
        dnl Fall back on GNU gettext library.
        nls_cv_use_gnu_gettext=maybe
      fi
    fi

    if test "$nls_cv_use_gnu_gettext" != "no"; then
      CATOBJEXT=.gmo
      if test -f "$srcdir/intl/libintl.h" ; then
        dnl Mark actions used to generate GNU NLS library.
        INTLOBJS="\$(GETTOBJS)"
        BUILD_INCLUDED_LIBINTL=yes
        USE_INCLUDED_LIBINTL=yes
        INTLLIBS="ifelse([$3],[],\$(top_builddir)/intl,[$3])/libintl.ifelse([$1], use-libtool, [l], [])a $LIBICONV"
        LIBS=`echo " $LIBS " | sed -e 's/ -lintl / /' -e 's/^ //' -e 's/ $//'`
      elif test "$nls_cv_use_gnu_gettext" = "yes"; then
        nls_cv_use_gnu_gettext=no
        AC_MSG_WARN(no NLS library is packaged with this application)
      fi
    fi

    dnl Test whether we really found GNU msgfmt.
    if test "$GMSGFMT" != ":"; then
      if $GMSGFMT --statistics /dev/null >/dev/null 2>&1; then
        : ;
      else
        AC_MSG_WARN([found msgfmt program is not GNU msgfmt])
      fi
    fi

    dnl Test whether we really found GNU xgettext.
    if test "$XGETTEXT" != ":"; then
      if $XGETTEXT --omit-header /dev/null >/dev/null 2>&1; then
        : ;
      else
        AC_MSG_WARN([found xgettext program is not GNU xgettext])
      fi
    fi
  fi

  if test "$XGETTEXT" != ":"; then
    AC_OUTPUT_COMMANDS(
     [for ac_file in $CONFIG_FILES; do

        # Support "outfile[:infile[:infile...]]"
        case "$ac_file" in
          (*:*) ac_file=`echo "$ac_file"|sed 's%:.*%%'` ;;
        esac

        # PO directories have a Makefile.in generated from Makefile.inn.
        case "$ac_file" in
        (*/[Mm]akefile.in)
          # Adjust a relative srcdir.
          ac_dir="`echo "$ac_file"|sed 's%/[^/][^/]*$%%'`"
          ac_dir_suffix="/`echo "$ac_dir"|sed 's%^\./%%'`"
          ac_dots="`echo "$ac_dir_suffix"|sed 's%/[^/]*%../%g'`"
          ac_base="`basename $ac_file .in`"
          # In autoconf-2.13 it is called $ac_given_srcdir.
          # In autoconf-2.50 it is called $srcdir.
          test -n "$ac_given_srcdir" || ac_given_srcdir="$srcdir"

          case "$ac_given_srcdir" in
            (.)  top_srcdir=`echo $ac_dots|sed 's%/$%%'` ;;
            (/*) top_srcdir="$ac_given_srcdir" ;;
            (*)  top_srcdir="$ac_dots$ac_given_srcdir" ;;
          esac

          if test -f "$ac_given_srcdir/$ac_dir/POTFILES.in"; then
            rm -f "$ac_dir/POTFILES"
            test -n "$as_me" && echo "$as_me: creating $ac_dir/POTFILES" || echo "creating $ac_dir/POTFILES"
            sed -e "/^#/d" -e "/^[ 	]*\$/d" -e "s,.*,     $top_srcdir/& \\\\," -e "\$s/\(.*\) \\\\/\1/" < "$ac_given_srcdir/$ac_dir/POTFILES.in" > "$ac_dir/POTFILES"
            test -n "$as_me" && echo "$as_me: creating $ac_dir/$ac_base" || echo "creating $ac_dir/$ac_base"
            sed -e "/POTFILES =/r $ac_dir/POTFILES" "$ac_dir/$ac_base.in" > "$ac_dir/$ac_base"
          fi
          ;;
        esac
      done])

    dnl If this is used in GNU gettext we have to set BUILD_INCLUDED_LIBINTL
    dnl to 'yes' because some of the testsuite requires it.
    if test "$PACKAGE" = gettext; then
      BUILD_INCLUDED_LIBINTL=yes
    fi

    dnl intl/plural.c is generated from intl/plural.y. It requires bison,
    dnl because plural.y uses bison specific features. It requires at least
    dnl bison-1.26 because earlier versions generate a plural.c that doesn't
    dnl compile.
    dnl bison is only needed for the maintainer (who touches plural.y). But in
    dnl order to avoid separate Makefiles or --enable-maintainer-mode, we put
    dnl the rule in general Makefile. Now, some people carelessly touch the
    dnl files or have a broken "make" program, hence the plural.c rule will
    dnl sometimes fire. To avoid an error, defines BISON to ":" if it is not
    dnl present or too old.
    if test "$nls_cv_use_gnu_gettext" = "yes"; then
      AC_CHECK_PROGS([INTLBISON], [bison])
      if test -z "$INTLBISON"; then
        ac_verc_fail=yes
      else
        dnl Found it, now check the version.
        AC_MSG_CHECKING([version of bison])
changequote(<<,>>)dnl
        ac_prog_version=`$INTLBISON --version 2>&1 | sed -n 's/^.*GNU Bison.* \([0-9]*\.[0-9.]*\).*$/\1/p'`
        case "$ac_prog_version" in
          ('') ac_prog_version="v. ?.??, bad"; ac_verc_fail=yes;;
          (1.2[6-9]*|1.[3-9][0-9]*|[2-9].*)
changequote([,])dnl
             ac_prog_version="$ac_prog_version, ok"; ac_verc_fail=no;;
          (*) ac_prog_version="$ac_prog_version, bad"; ac_verc_fail=yes;;
        esac
      AC_MSG_RESULT([$ac_prog_version])
      fi
      if test "$ac_verc_fail" = yes; then
        INTLBISON=:
      fi
    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
      POFILES="$POFILES $lang.po"
    done
  fi

  dnl Make all variables we use known to autoconf.
  AC_SUBST(BUILD_INCLUDED_LIBINTL)
  AC_SUBST(USE_INCLUDED_LIBINTL)
  AC_SUBST(CATALOGS)
  AC_SUBST(CATOBJEXT)
  AC_SUBST(GMOFILES)
  AC_SUBST(INTLLIBS)
  AC_SUBST(INTLOBJS)
  AC_SUBST(POFILES)
  AC_SUBST(POSUB)

  dnl For backward compatibility. Some configure.ins may be using this.
  nls_cv_header_intl=
  nls_cv_header_libgt=

  dnl For backward compatibility. Some Makefiles may be using this.
  DATADIRNAME=share
  AC_SUBST(DATADIRNAME)

  dnl For backward compatibility. Some Makefiles may be using this.
  INSTOBJEXT=.mo
  AC_SUBST(INSTOBJEXT)

  dnl For backward compatibility. Some Makefiles may be using this.
  GENCAT=gencat
  AC_SUBST(GENCAT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ACVERSION_CHECK version: 5 updated: 2014/06/04 19:11:49
dnl ------------------
dnl Conditionally generate script according to whether we're using a given autoconf.
dnl
dnl $1 = version to compare against
dnl $2 = code to use if AC_ACVERSION is at least as high as $1.
dnl $3 = code to use if AC_ACVERSION is older than $1.
define([CF_ACVERSION_CHECK],
[
ifdef([AC_ACVERSION], ,[ifdef([AC_AUTOCONF_VERSION],[m4_copy([AC_AUTOCONF_VERSION],[AC_ACVERSION])],[m4_copy([m4_PACKAGE_VERSION],[AC_ACVERSION])])])dnl
ifdef([m4_version_compare],
[m4_if(m4_version_compare(m4_defn([AC_ACVERSION]), [$1]), -1, [$3], [$2])],
[CF_ACVERSION_COMPARE(
AC_PREREQ_CANON(AC_PREREQ_SPLIT([$1])),
AC_PREREQ_CANON(AC_PREREQ_SPLIT(AC_ACVERSION)), AC_ACVERSION, [$2], [$3])])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ACVERSION_COMPARE version: 3 updated: 2012/10/03 18:39:53
dnl --------------------
dnl CF_ACVERSION_COMPARE(MAJOR1, MINOR1, TERNARY1,
dnl                      MAJOR2, MINOR2, TERNARY2,
dnl                      PRINTABLE2, not FOUND, FOUND)
define([CF_ACVERSION_COMPARE],
[ifelse(builtin([eval], [$2 < $5]), 1,
[ifelse([$8], , ,[$8])],
[ifelse([$9], , ,[$9])])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_CFLAGS version: 15 updated: 2020/12/31 10:54:15
dnl -------------
dnl Copy non-preprocessor flags to $CFLAGS, preprocessor flags to $CPPFLAGS
dnl $1 = flags to add
dnl $2 = if given makes this macro verbose.
dnl
dnl Put any preprocessor definitions that use quoted strings in $EXTRA_CPPFLAGS,
dnl to simplify use of $CPPFLAGS in compiler checks, etc., that are easily
dnl confused by the quotes (which require backslashes to keep them usable).
AC_DEFUN([CF_ADD_CFLAGS],
[
cf_fix_cppflags=no
cf_new_cflags=
cf_new_cppflags=
cf_new_extra_cppflags=

for cf_add_cflags in $1
do
case "$cf_fix_cppflags" in
(no)
	case "$cf_add_cflags" in
	(-undef|-nostdinc*|-I*|-D*|-U*|-E|-P|-C)
		case "$cf_add_cflags" in
		(-D*)
			cf_tst_cflags=`echo "${cf_add_cflags}" |sed -e 's/^-D[[^=]]*='\''\"[[^"]]*//'`

			test "x${cf_add_cflags}" != "x${cf_tst_cflags}" \
				&& test -z "${cf_tst_cflags}" \
				&& cf_fix_cppflags=yes

			if test "$cf_fix_cppflags" = yes ; then
				CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)
				continue
			elif test "${cf_tst_cflags}" = "\"'" ; then
				CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)
				continue
			fi
			;;
		esac
		case "$CPPFLAGS" in
		(*$cf_add_cflags)
			;;
		(*)
			case "$cf_add_cflags" in
			(-D*)
				cf_tst_cppflags=`echo "x$cf_add_cflags" | sed -e 's/^...//' -e 's/=.*//'`
				CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,$cf_tst_cppflags)
				;;
			esac
			CF_APPEND_TEXT(cf_new_cppflags,$cf_add_cflags)
			;;
		esac
		;;
	(*)
		CF_APPEND_TEXT(cf_new_cflags,$cf_add_cflags)
		;;
	esac
	;;
(yes)
	CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)

	cf_tst_cflags=`echo "${cf_add_cflags}" |sed -e 's/^[[^"]]*"'\''//'`

	test "x${cf_add_cflags}" != "x${cf_tst_cflags}" \
		&& test -z "${cf_tst_cflags}" \
		&& cf_fix_cppflags=no
	;;
esac
done

if test -n "$cf_new_cflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$CFLAGS $cf_new_cflags)])
	CF_APPEND_TEXT(CFLAGS,$cf_new_cflags)
fi

if test -n "$cf_new_cppflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$CPPFLAGS $cf_new_cppflags)])
	CF_APPEND_TEXT(CPPFLAGS,$cf_new_cppflags)
fi

if test -n "$cf_new_extra_cppflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$EXTRA_CPPFLAGS $cf_new_extra_cppflags)])
	CF_APPEND_TEXT(EXTRA_CPPFLAGS,$cf_new_extra_cppflags)
fi

AC_SUBST(EXTRA_CPPFLAGS)

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_INCDIR version: 16 updated: 2020/12/31 20:19:42
dnl -------------
dnl Add an include-directory to $CPPFLAGS.  Don't add /usr/include, since it's
dnl redundant.  We don't normally need to add -I/usr/local/include for gcc,
dnl but old versions (and some misinstalled ones) need that.  To make things
dnl worse, gcc 3.x may give error messages if -I/usr/local/include is added to
dnl the include-path).
AC_DEFUN([CF_ADD_INCDIR],
[
if test -n "$1" ; then
  for cf_add_incdir in $1
  do
	while test "$cf_add_incdir" != /usr/include
	do
	  if test -d "$cf_add_incdir"
	  then
		cf_have_incdir=no
		if test -n "$CFLAGS$CPPFLAGS" ; then
		  # a loop is needed to ensure we can add subdirs of existing dirs
		  for cf_test_incdir in $CFLAGS $CPPFLAGS ; do
			if test ".$cf_test_incdir" = ".-I$cf_add_incdir" ; then
			  cf_have_incdir=yes; break
			fi
		  done
		fi

		if test "$cf_have_incdir" = no ; then
		  if test "$cf_add_incdir" = /usr/local/include ; then
			if test "$GCC" = yes
			then
			  cf_save_CPPFLAGS=$CPPFLAGS
			  CF_APPEND_TEXT(CPPFLAGS,-I$cf_add_incdir)
			  AC_TRY_COMPILE([#include <stdio.h>],
				  [printf("Hello")],
				  [],
				  [cf_have_incdir=yes])
			  CPPFLAGS=$cf_save_CPPFLAGS
			fi
		  fi
		fi

		if test "$cf_have_incdir" = no ; then
		  CF_VERBOSE(adding $cf_add_incdir to include-path)
		  ifelse([$2],,CPPFLAGS,[$2])="$ifelse([$2],,CPPFLAGS,[$2]) -I$cf_add_incdir"

		  cf_top_incdir=`echo "$cf_add_incdir" | sed -e 's%/include/.*$%/include%'`
		  test "$cf_top_incdir" = "$cf_add_incdir" && break
		  cf_add_incdir="$cf_top_incdir"
		else
		  break
		fi
	  else
		break
	  fi
	done
  done
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIB version: 2 updated: 2010/06/02 05:03:05
dnl ----------
dnl Add a library, used to enforce consistency.
dnl
dnl $1 = library to add, without the "-l"
dnl $2 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIB],[CF_ADD_LIBS(-l$1,ifelse($2,,LIBS,[$2]))])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIBDIR version: 11 updated: 2020/12/31 20:19:42
dnl -------------
dnl	Adds to the library-path
dnl
dnl	Some machines have trouble with multiple -L options.
dnl
dnl $1 is the (list of) directory(s) to add
dnl $2 is the optional name of the variable to update (default LDFLAGS)
dnl
AC_DEFUN([CF_ADD_LIBDIR],
[
if test -n "$1" ; then
	for cf_add_libdir in $1
	do
		if test "$cf_add_libdir" = /usr/lib ; then
			:
		elif test -d "$cf_add_libdir"
		then
			cf_have_libdir=no
			if test -n "$LDFLAGS$LIBS" ; then
				# a loop is needed to ensure we can add subdirs of existing dirs
				for cf_test_libdir in $LDFLAGS $LIBS ; do
					if test ".$cf_test_libdir" = ".-L$cf_add_libdir" ; then
						cf_have_libdir=yes; break
					fi
				done
			fi
			if test "$cf_have_libdir" = no ; then
				CF_VERBOSE(adding $cf_add_libdir to library-path)
				ifelse([$2],,LDFLAGS,[$2])="-L$cf_add_libdir $ifelse([$2],,LDFLAGS,[$2])"
			fi
		fi
	done
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIBS version: 3 updated: 2019/11/02 16:47:33
dnl -----------
dnl Add one or more libraries, used to enforce consistency.  Libraries are
dnl prepended to an existing list, since their dependencies are assumed to
dnl already exist in the list.
dnl
dnl $1 = libraries to add, with the "-l", etc.
dnl $2 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIBS],[
cf_add_libs="[$]ifelse($2,,LIBS,[$2])"
# reverse order
cf_add_0lib=
for cf_add_1lib in $1; do cf_add_0lib="$cf_add_1lib $cf_add_0lib"; done
# filter duplicates
for cf_add_1lib in $cf_add_0lib; do
	for cf_add_2lib in $cf_add_libs; do
		if test "x$cf_add_1lib" = "x$cf_add_2lib"; then
			cf_add_1lib=
			break
		fi
	done
	test -n "$cf_add_1lib" && cf_add_libs="$cf_add_1lib $cf_add_libs"
done
ifelse($2,,LIBS,[$2])="$cf_add_libs"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIB_AFTER version: 3 updated: 2013/07/09 21:27:22
dnl ----------------
dnl Add a given library after another, e.g., following the one it satisfies a
dnl dependency for.
dnl
dnl $1 = the first library
dnl $2 = its dependency
AC_DEFUN([CF_ADD_LIB_AFTER],[
CF_VERBOSE(...before $LIBS)
LIBS=`echo "$LIBS" | sed -e "s/[[ 	]][[ 	]]*/ /g" -e "s%$1 %$1 $2 %" -e 's%  % %g'`
CF_VERBOSE(...after  $LIBS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_OPTIONAL_PATH version: 3 updated: 2015/05/10 19:52:14
dnl --------------------
dnl Add an optional search-path to the compile/link variables.
dnl See CF_WITH_PATH
dnl
dnl $1 = shell variable containing the result of --with-XXX=[DIR]
dnl $2 = module to look for.
AC_DEFUN([CF_ADD_OPTIONAL_PATH],[
case "$1" in
(no)
	;;
(yes)
	;;
(*)
	CF_ADD_SEARCHPATH([$1], [AC_MSG_ERROR(cannot find $2 under $1)])
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_SEARCHPATH version: 6 updated: 2020/12/31 20:19:42
dnl -----------------
dnl Set $CPPFLAGS and $LDFLAGS with the directories given via the parameter.
dnl They can be either the common root of include- and lib-directories, or the
dnl lib-directory (to allow for things like lib64 directories).
dnl See also CF_FIND_LINKAGE.
dnl
dnl $1 is the list of colon-separated directory names to search.
dnl $2 is the action to take if a parameter does not yield a directory.
AC_DEFUN([CF_ADD_SEARCHPATH],
[
AC_REQUIRE([CF_PATHSEP])
for cf_searchpath in `echo "$1" | tr $PATH_SEPARATOR ' '`; do
	if test -d "$cf_searchpath/include" ; then
		CF_ADD_INCDIR($cf_searchpath/include)
	elif test -d "$cf_searchpath/../include" ; then
		CF_ADD_INCDIR($cf_searchpath/../include)
	ifelse([$2],,,[else
$2])
	fi
	if test -d "$cf_searchpath/lib" ; then
		CF_ADD_LIBDIR($cf_searchpath/lib)
	elif test -d "$cf_searchpath" ; then
		CF_ADD_LIBDIR($cf_searchpath)
	ifelse([$2],,,[else
$2])
	fi
done
])
dnl ---------------------------------------------------------------------------
dnl CF_ADD_SUBDIR_PATH version: 5 updated: 2020/12/31 20:19:42
dnl ------------------
dnl Append to a search-list for a nonstandard header/lib-file
dnl	$1 = the variable to return as result
dnl	$2 = the package name
dnl	$3 = the subdirectory, e.g., bin, include or lib
dnl $4 = the directory under which we will test for subdirectories
dnl $5 = a directory that we do not want $4 to match
AC_DEFUN([CF_ADD_SUBDIR_PATH],
[
test "x$4" != "x$5" && \
test -d "$4" && \
ifelse([$5],NONE,,[{ test -z "$5" || test "x$5" = xNONE || test "x$4" != "x$5"; } &&]) {
	test -n "$verbose" && echo "	... testing for $3-directories under $4"
	test -d "$4/$3" &&          $1="[$]$1 $4/$3"
	test -d "$4/$3/$2" &&       $1="[$]$1 $4/$3/$2"
	test -d "$4/$3/$2/$3" &&    $1="[$]$1 $4/$3/$2/$3"
	test -d "$4/$2/$3" &&       $1="[$]$1 $4/$2/$3"
	test -d "$4/$2/$3/$2" &&    $1="[$]$1 $4/$2/$3/$2"
}
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ALT_CHAR_SET version: 10 updated: 2021/06/08 18:08:14
dnl ---------------
dnl Check for existence of alternate-character-set support in curses, so we
dnl can decide to use it for box characters.
dnl
AC_DEFUN([CF_ALT_CHAR_SET],
[
AC_MSG_CHECKING([if curses supports alternate-character set])
AC_CACHE_VAL(cf_cv_alt_char_set,[
for mapname in acs_map _acs_map
do
	AC_TRY_LINK([
#include <${cf_cv_ncurses_header:-curses.h}>
	],[chtype x = ${mapname}['l']; ${mapname}['m'] = 0; (void)x],
	[cf_cv_alt_char_set=$mapname
	 break],
	[cf_cv_alt_char_set=no])
done
	])
AC_MSG_RESULT($cf_cv_alt_char_set)
test "$cf_cv_alt_char_set" != no && AC_DEFINE_UNQUOTED(ALT_CHAR_SET,$cf_cv_alt_char_set,[Define to 1 if if curses supports alternate-character set])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ANSI_CC_CHECK version: 13 updated: 2012/10/06 11:17:15
dnl ----------------
dnl This was originally adapted from the macros 'fp_PROG_CC_STDC' and
dnl 'fp_C_PROTOTYPES' in the sharutils 4.2 distribution.
AC_DEFUN([CF_ANSI_CC_CHECK],
[
CF_CC_ENV_FLAGS

AC_CACHE_CHECK(for ${CC:-cc} option to accept ANSI C, cf_cv_ansi_cc,[
cf_cv_ansi_cc=no
cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc
# UnixWare 1.2		(cannot use -Xc, since ANSI/POSIX clashes)
for cf_arg in "-DCC_HAS_PROTOS" \
	"" \
	-qlanglvl=ansi \
	-std1 \
	-Ae \
	"-Aa -D_HPUX_SOURCE" \
	-Xc
do
	CF_ADD_CFLAGS($cf_arg)
	AC_TRY_COMPILE(
[
#ifndef CC_HAS_PROTOS
#if !defined(__STDC__) || (__STDC__ != 1)
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
CPPFLAGS="$cf_save_CPPFLAGS"
])

if test "$cf_cv_ansi_cc" != "no"; then
if test ".$cf_cv_ansi_cc" != ".-DCC_HAS_PROTOS"; then
	CF_ADD_CFLAGS($cf_cv_ansi_cc)
else
	AC_DEFINE(CC_HAS_PROTOS,1,[Define to 1 if C compiler supports prototypes])
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ANSI_CC_REQD version: 4 updated: 2008/03/23 14:48:54
dnl ---------------
dnl For programs that must use an ANSI compiler, obtain compiler options that
dnl will make it recognize prototypes.  We'll do preprocessor checks in other
dnl macros, since tools such as unproto can fake prototypes, but only part of
dnl the preprocessor.
AC_DEFUN([CF_ANSI_CC_REQD],
[AC_REQUIRE([CF_ANSI_CC_CHECK])
if test "$cf_cv_ansi_cc" = "no"; then
	AC_MSG_ERROR(
[Your compiler does not appear to recognize prototypes.
You have the following choices:
	a. adjust your compiler options
	b. get an up-to-date compiler
	c. use a wrapper such as unproto])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_APPEND_TEXT version: 1 updated: 2017/02/25 18:58:55
dnl --------------
dnl use this macro for appending text without introducing an extra blank at
dnl the beginning
define([CF_APPEND_TEXT],
[
	test -n "[$]$1" && $1="[$]$1 "
	$1="[$]{$1}$2"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_DISABLE version: 3 updated: 1999/03/30 17:24:31
dnl --------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_ENABLE version: 3 updated: 1999/03/30 17:24:31
dnl -------------
dnl Allow user to enable a normally-off option.
AC_DEFUN([CF_ARG_ENABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],no)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_OPTION version: 5 updated: 2015/05/10 19:52:14
dnl -------------
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
[AC_ARG_ENABLE([$1],[$2],[test "$enableval" != ifelse([$5],no,yes,no) && enableval=ifelse([$5],no,no,yes)
	if test "$enableval" != "$5" ; then
ifelse([$3],,[    :]dnl
,[    $3]) ifelse([$4],,,[
	else
		$4])
	fi],[enableval=$5 ifelse([$4],,,[
	$4
])dnl
])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_AR_FLAGS version: 9 updated: 2021/01/01 13:31:04
dnl -----------
dnl Check for suitable "ar" (archiver) options for updating an archive.
dnl
dnl In particular, handle some obsolete cases where the "-" might be omitted,
dnl as well as a workaround for breakage of make's archive rules by the GNU
dnl binutils "ar" program.
AC_DEFUN([CF_AR_FLAGS],[
AC_REQUIRE([CF_PROG_AR])

AC_CACHE_CHECK(for options to update archives, cf_cv_ar_flags,[
	case "$cf_cv_system_name" in
	(*-msvc*)
		cf_cv_ar_flags=''
		cat >mk_static_lib.sh <<-EOF
		#!$SHELL
		MSVC_BIN="[$]AR"
		out="\[$]1"
		shift
		exec \[$]MSVC_BIN -out:"\[$]out" \[$]@
		EOF
		chmod +x mk_static_lib.sh
		AR=`pwd`/mk_static_lib.sh
		;;
	(*)
		cf_cv_ar_flags=unknown
		for cf_ar_flags in -curvU -curv curv -crv crv -cqv cqv -rv rv
		do

			# check if $ARFLAGS already contains this choice
			if test "x$ARFLAGS" != "x" ; then
				cf_check_ar_flags=`echo "x$ARFLAGS" | sed -e "s/$cf_ar_flags\$//" -e "s/$cf_ar_flags / /"`
				if test "x$ARFLAGS" != "$cf_check_ar_flags" ; then
					cf_cv_ar_flags=
					break
				fi
			fi

			rm -f "conftest.$ac_cv_objext"
			rm -f conftest.a

			cat >"conftest.$ac_ext" <<EOF
#line __oline__ "configure"
int	testdata[[3]] = { 123, 456, 789 };
EOF
			if AC_TRY_EVAL(ac_compile) ; then
				echo "$AR $ARFLAGS $cf_ar_flags conftest.a conftest.$ac_cv_objext" >&AC_FD_CC
				$AR $ARFLAGS "$cf_ar_flags" conftest.a "conftest.$ac_cv_objext" 2>&AC_FD_CC 1>/dev/null
				if test -f conftest.a ; then
					cf_cv_ar_flags="$cf_ar_flags"
					break
				fi
			else
				CF_VERBOSE(cannot compile test-program)
				break
			fi
		done
		rm -f conftest.a "conftest.$ac_ext" "conftest.$ac_cv_objext"
		;;
	esac
])

if test -n "$ARFLAGS" ; then
	if test -n "$cf_cv_ar_flags" ; then
		ARFLAGS="$ARFLAGS $cf_cv_ar_flags"
	fi
else
	ARFLAGS=$cf_cv_ar_flags
fi

AC_SUBST(ARFLAGS)
])
dnl ---------------------------------------------------------------------------
dnl CF_BOOL_DEFS version: 5 updated: 2012/11/08 20:57:52
dnl ------------
dnl Check if curses.h defines TRUE/FALSE (it does under SVr4).
AC_DEFUN([CF_BOOL_DEFS],
[
AC_MSG_CHECKING(if TRUE/FALSE are defined)
AC_CACHE_VAL(cf_cv_bool_defs,[
AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header:-curses.h}>
#include <stdio.h>],[int x = TRUE, y = FALSE],
	[cf_cv_bool_defs=yes],
	[cf_cv_bool_defs=no])])
AC_MSG_RESULT($cf_cv_bool_defs)
if test "$cf_cv_bool_defs" = no ; then
	AC_DEFINE(TRUE,(1),[Define to TRUE if curses.h does not define])
	AC_DEFINE(FALSE,(0),[Define to FALSE if curses.h does not define])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_BUILD_CC version: 9 updated: 2021/01/02 09:31:20
dnl -----------
dnl If we're cross-compiling, allow the user to override the tools and their
dnl options.  The configure script is oriented toward identifying the host
dnl compiler, etc., but we need a build compiler to generate parts of the
dnl source.
dnl
dnl $1 = default for $CPPFLAGS
dnl $2 = default for $LIBS
AC_DEFUN([CF_BUILD_CC],[
CF_ACVERSION_CHECK(2.52,,
	[AC_REQUIRE([CF_PROG_EXT])])
if test "$cross_compiling" = yes ; then

	# defaults that we might want to override
	: ${BUILD_CFLAGS:=''}
	: ${BUILD_CPPFLAGS:='ifelse([$1],,,[$1])'}
	: ${BUILD_LDFLAGS:=''}
	: ${BUILD_LIBS:='ifelse([$2],,,[$2])'}
	: ${BUILD_EXEEXT:='$x'}
	: ${BUILD_OBJEXT:='o'}

	AC_ARG_WITH(build-cc,
		[  --with-build-cc=XXX     the build C compiler ($BUILD_CC)],
		[BUILD_CC="$withval"],
		[AC_CHECK_PROGS(BUILD_CC, [gcc clang c99 c89 cc cl],none)])
	AC_MSG_CHECKING(for native build C compiler)
	AC_MSG_RESULT($BUILD_CC)

	AC_MSG_CHECKING(for native build C preprocessor)
	AC_ARG_WITH(build-cpp,
		[  --with-build-cpp=XXX    the build C preprocessor ($BUILD_CPP)],
		[BUILD_CPP="$withval"],
		[BUILD_CPP='${BUILD_CC} -E'])
	AC_MSG_RESULT($BUILD_CPP)

	AC_MSG_CHECKING(for native build C flags)
	AC_ARG_WITH(build-cflags,
		[  --with-build-cflags=XXX the build C compiler-flags ($BUILD_CFLAGS)],
		[BUILD_CFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_CFLAGS)

	AC_MSG_CHECKING(for native build C preprocessor-flags)
	AC_ARG_WITH(build-cppflags,
		[  --with-build-cppflags=XXX the build C preprocessor-flags ($BUILD_CPPFLAGS)],
		[BUILD_CPPFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_CPPFLAGS)

	AC_MSG_CHECKING(for native build linker-flags)
	AC_ARG_WITH(build-ldflags,
		[  --with-build-ldflags=XXX the build linker-flags ($BUILD_LDFLAGS)],
		[BUILD_LDFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_LDFLAGS)

	AC_MSG_CHECKING(for native build linker-libraries)
	AC_ARG_WITH(build-libs,
		[  --with-build-libs=XXX   the build libraries (${BUILD_LIBS})],
		[BUILD_LIBS="$withval"])
	AC_MSG_RESULT($BUILD_LIBS)

	# this assumes we're on Unix.
	BUILD_EXEEXT=
	BUILD_OBJEXT=o

	: ${BUILD_CC:='${CC}'}

	if { test "$BUILD_CC" = "$CC" || test "$BUILD_CC" = '${CC}'; } ; then
		AC_MSG_ERROR([Cross-build requires two compilers.
Use --with-build-cc to specify the native compiler.])
	fi

else
	: ${BUILD_CC:='${CC}'}
	: ${BUILD_CPP:='${CPP}'}
	: ${BUILD_CFLAGS:='${CFLAGS}'}
	: ${BUILD_CPPFLAGS:='${CPPFLAGS}'}
	: ${BUILD_LDFLAGS:='${LDFLAGS}'}
	: ${BUILD_LIBS:='${LIBS}'}
	: ${BUILD_EXEEXT:='$x'}
	: ${BUILD_OBJEXT:='o'}
fi

AC_SUBST(BUILD_CC)
AC_SUBST(BUILD_CPP)
AC_SUBST(BUILD_CFLAGS)
AC_SUBST(BUILD_CPPFLAGS)
AC_SUBST(BUILD_LDFLAGS)
AC_SUBST(BUILD_LIBS)
AC_SUBST(BUILD_EXEEXT)
AC_SUBST(BUILD_OBJEXT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_BUNDLED_INTL version: 20 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Top-level macro for configuring an application with a bundled copy of
dnl the intl and po directories for gettext.
dnl
dnl $1 specifies either Makefile or makefile, defaulting to the former.
dnl $2 if nonempty sets the option to --enable-nls rather than to --disable-nls
dnl
dnl Sets variables which can be used to substitute in makefiles:
dnl	GT_YES       - "#" comment unless building intl library, otherwise empty
dnl	GT_NO        - "#" comment if building intl library, otherwise empty
dnl	INTLDIR_MAKE - to make ./intl directory
dnl	MSG_DIR_MAKE - to make ./po directory
dnl	SUB_MAKEFILE - list of makefiles in ./intl, ./po directories
dnl
dnl Defines:
dnl	HAVE_LIBGETTEXT_H if we're using ./intl
dnl	NLS_TEXTDOMAIN
dnl
dnl Environment:
dnl	ALL_LINGUAS if set, lists the root names of the ".po" files.
dnl	CONFIG_H assumed to be "config.h"
dnl	PACKAGE must be set, used as default for textdomain
dnl	VERSION may be set, otherwise extract from "VERSION" file.
dnl
AC_DEFUN([CF_BUNDLED_INTL],[
cf_makefile=ifelse($1,,Makefile,$1)

dnl Set of available languages (based on source distribution).  Note that
dnl setting $LINGUAS overrides $ALL_LINGUAS.  Some environments set $LINGUAS
dnl rather than $LC_ALL
test -z "$ALL_LINGUAS" && ALL_LINGUAS=`test -d "$srcdir/po" && cd "$srcdir/po" && echo *.po|sed -e 's/\.po//g' -e 's/*//'`

# Allow override of "config.h" definition:
: ${CONFIG_H:=config.h}
AC_SUBST(CONFIG_H)

if test -z "$PACKAGE" ; then
	AC_MSG_ERROR([[CF_BUNDLED_INTL] used without setting [PACKAGE] variable])
fi

if test -z "$VERSION" ; then
if test -f "$srcdir/VERSION" ; then
	VERSION=`sed -e '2,$d' "$srcdir/VERSION" |cut -f1`
else
	VERSION=unknown
fi
fi
AC_SUBST(VERSION)

AM_GNU_GETTEXT(,,,[$2])

if test "$USE_NLS" = yes ; then
	AC_ARG_WITH(textdomain,
		[  --with-textdomain=PKG   NLS text-domain (default is package name)],
		[NLS_TEXTDOMAIN=$withval],
		[NLS_TEXTDOMAIN=$PACKAGE])
	AC_DEFINE_UNQUOTED(NLS_TEXTDOMAIN,"$NLS_TEXTDOMAIN",[Define to the nls textdomain value])
	AC_SUBST(NLS_TEXTDOMAIN)
fi

INTLDIR_MAKE=
MSG_DIR_MAKE=
SUB_MAKEFILE=

dnl this updates SUB_MAKEFILE and MSG_DIR_MAKE:
CF_OUR_MESSAGES($1)

if test "$USE_INCLUDED_LIBINTL" = yes ; then
	if test "$nls_cv_force_use_gnu_gettext" = yes ; then
		:
	elif test "$nls_cv_use_gnu_gettext" = yes ; then
		:
	else
		INTLDIR_MAKE="#"
	fi
	if test -z "$INTLDIR_MAKE"; then
		AC_DEFINE(HAVE_LIBGETTEXT_H,1,[Define to 1 if we have libgettext.h])
		for cf_makefile in \
			$srcdir/intl/Makefile.in \
			$srcdir/intl/makefile.in
		do
			if test -f "$cf_makefile" ; then
				SUB_MAKEFILE="$SUB_MAKEFILE `echo \"${cf_makefile}\"|sed -e 's,^'$srcdir/',,' -e 's/\.in$//'`:${cf_makefile}"
				break
			fi
		done
	fi
else
	INTLDIR_MAKE="#"
	if test "$USE_NLS" = yes ; then
		AC_CHECK_HEADERS(libintl.h)
	fi
fi

if test -z "$INTLDIR_MAKE" ; then
	CF_APPEND_TEXT(CPPFLAGS,-I../intl)
fi

dnl FIXME:  we use this in lynx (the alternative is a spurious dependency upon
dnl GNU make)
if test "$BUILD_INCLUDED_LIBINTL" = yes ; then
	GT_YES="#"
	GT_NO=
else
	GT_YES=
	GT_NO="#"
fi

AC_SUBST(INTLDIR_MAKE)
AC_SUBST(MSG_DIR_MAKE)
AC_SUBST(GT_YES)
AC_SUBST(GT_NO)

dnl FIXME:  the underlying AM_GNU_GETTEXT macro either needs some fixes or a
dnl little documentation.  It doesn't define anything so that we can ifdef our
dnl own code, except ENABLE_NLS, which is too vague to be of any use.

if test "$USE_INCLUDED_LIBINTL" = yes ; then
	if test "$nls_cv_force_use_gnu_gettext" = yes ; then
		AC_DEFINE(HAVE_GETTEXT,1,[Define to 1 if we have gettext function])
	elif test "$nls_cv_use_gnu_gettext" = yes ; then
		AC_DEFINE(HAVE_GETTEXT,1,[Define to 1 if we have gettext function])
	fi
	if test -n "$nls_cv_header_intl" ; then
		AC_DEFINE(HAVE_LIBINTL_H,1,[Define to 1 if we have header-file for libintl])
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_C11_NORETURN version: 3 updated: 2021/03/28 11:36:23
dnl ---------------
AC_DEFUN([CF_C11_NORETURN],
[
AC_MSG_CHECKING(if you want to use C11 _Noreturn feature)
CF_ARG_ENABLE(stdnoreturn,
	[  --enable-stdnoreturn    enable C11 _Noreturn feature for diagnostics],
	[enable_stdnoreturn=yes],
	[enable_stdnoreturn=no])
AC_MSG_RESULT($enable_stdnoreturn)

if test $enable_stdnoreturn = yes; then
AC_CACHE_CHECK([for C11 _Noreturn feature], cf_cv_c11_noreturn,
	[AC_TRY_COMPILE([
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
static _Noreturn void giveup(void) { exit(0); }
	],
	[if (feof(stdin)) giveup()],
	cf_cv_c11_noreturn=yes,
	cf_cv_c11_noreturn=no)
	])
else
	cf_cv_c11_noreturn=no,
fi

if test "$cf_cv_c11_noreturn" = yes; then
	AC_DEFINE(HAVE_STDNORETURN_H, 1,[Define if <stdnoreturn.h> header is available and working])
	AC_DEFINE_UNQUOTED(STDC_NORETURN,_Noreturn,[Define if C11 _Noreturn keyword is supported])
	HAVE_STDNORETURN_H=1
else
	HAVE_STDNORETURN_H=0
fi

AC_SUBST(HAVE_STDNORETURN_H)
AC_SUBST(STDC_NORETURN)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CC_ENV_FLAGS version: 10 updated: 2020/12/31 18:40:20
dnl ---------------
dnl Check for user's environment-breakage by stuffing CFLAGS/CPPFLAGS content
dnl into CC.  This will not help with broken scripts that wrap the compiler
dnl with options, but eliminates a more common category of user confusion.
dnl
dnl In particular, it addresses the problem of being able to run the C
dnl preprocessor in a consistent manner.
dnl
dnl Caveat: this also disallows blanks in the pathname for the compiler, but
dnl the nuisance of having inconsistent settings for compiler and preprocessor
dnl outweighs that limitation.
AC_DEFUN([CF_CC_ENV_FLAGS],
[
# This should have been defined by AC_PROG_CC
: "${CC:=cc}"

AC_MSG_CHECKING(\$CFLAGS variable)
case "x$CFLAGS" in
(*-[[IUD]]*)
	AC_MSG_RESULT(broken)
	AC_MSG_WARN(your environment uses the CFLAGS variable to hold CPPFLAGS options)
	cf_flags="$CFLAGS"
	CFLAGS=
	for cf_arg in $cf_flags
	do
		CF_ADD_CFLAGS($cf_arg)
	done
	;;
(*)
	AC_MSG_RESULT(ok)
	;;
esac

AC_MSG_CHECKING(\$CC variable)
case "$CC" in
(*[[\ \	]]-*)
	AC_MSG_RESULT(broken)
	AC_MSG_WARN(your environment uses the CC variable to hold CFLAGS/CPPFLAGS options)
	# humor him...
	cf_prog=`echo "$CC" | sed -e 's/	/ /g' -e 's/[[ ]]* / /g' -e 's/[[ ]]*[[ ]]-[[^ ]].*//'`
	cf_flags=`echo "$CC" | ${AWK:-awk} -v prog="$cf_prog" '{ printf("%s", [substr]([$]0,1+length(prog))); }'`
	CC="$cf_prog"
	for cf_arg in $cf_flags
	do
		case "x$cf_arg" in
		(x-[[IUDfgOW]]*)
			CF_ADD_CFLAGS($cf_arg)
			;;
		(*)
			CC="$CC $cf_arg"
			;;
		esac
	done
	CF_VERBOSE(resulting CC: '$CC')
	CF_VERBOSE(resulting CFLAGS: '$CFLAGS')
	CF_VERBOSE(resulting CPPFLAGS: '$CPPFLAGS')
	;;
(*)
	AC_MSG_RESULT(ok)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CACHE version: 13 updated: 2020/12/31 10:54:15
dnl --------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname).  Normally we'll use AC_CANONICAL_HOST, but allow
dnl an extra parameter that we may override, e.g., for AC_CANONICAL_SYSTEM
dnl which is useful in cross-compiles.
dnl
dnl Note: we would use $ac_config_sub, but that is one of the places where
dnl autoconf 2.5x broke compatibility with autoconf 2.13
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f "$srcdir/config.guess" || test -f "$ac_aux_dir/config.guess" ; then
	ifelse([$1],,[AC_CANONICAL_HOST],[$1])
	system_name="$host_os"
else
	system_name="`(uname -s -r) 2>/dev/null`"
	if test -z "$system_name" ; then
		system_name="`(hostname) 2>/dev/null`"
	fi
fi
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name",[Define to the system name.])
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT(Configuring for $cf_cv_system_name)

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_MSG_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CFLAGS version: 4 updated: 2021/01/02 19:22:58
dnl ---------------
dnl Conditionally add to $CFLAGS and $CPPFLAGS values which are derived from
dnl a build-configuration such as imake.  These have the pitfall that they
dnl often contain compiler-specific options which we cannot use, mixed with
dnl preprocessor options that we usually can.
AC_DEFUN([CF_CHECK_CFLAGS],
[
CF_VERBOSE(checking additions to CFLAGS)
cf_check_cflags="$CFLAGS"
cf_check_cppflags="$CPPFLAGS"
CF_ADD_CFLAGS($1,yes)
if test "x$cf_check_cflags" != "x$CFLAGS" ; then
AC_TRY_LINK([#include <stdio.h>],[printf("Hello world");],,
	[CF_VERBOSE(test-compile failed.  Undoing change to \$CFLAGS)
	 if test "x$cf_check_cppflags" != "x$CPPFLAGS" ; then
		 CF_VERBOSE(but keeping change to \$CPPFLAGS)
	 fi
	 CFLAGS="$cf_check_cflags"])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_ERRNO version: 13 updated: 2020/03/10 18:53:47
dnl --------------
dnl Check for data that is usually declared in <stdio.h> or <errno.h>, e.g.,
dnl the 'errno' variable.  Define a DECL_xxx symbol if we must declare it
dnl ourselves.
dnl
dnl $1 = the name to check
dnl $2 = the assumed type
AC_DEFUN([CF_CHECK_ERRNO],
[
AC_CACHE_CHECK(if external $1 is declared, cf_cv_dcl_$1,[
	AC_TRY_COMPILE([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <errno.h> ],
	ifelse([$2],,int,[$2]) x = (ifelse([$2],,int,[$2])) $1; (void)x,
	[cf_cv_dcl_$1=yes],
	[cf_cv_dcl_$1=no])
])

if test "$cf_cv_dcl_$1" = no ; then
	CF_UPPER(cf_result,decl_$1)
	AC_DEFINE_UNQUOTED($cf_result)
fi

# It's possible (for near-UNIX clones) that the data doesn't exist
CF_CHECK_EXTERN_DATA($1,ifelse([$2],,int,[$2]))
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_EXTERN_DATA version: 4 updated: 2015/04/18 08:56:57
dnl --------------------
dnl Check for existence of external data in the current set of libraries.  If
dnl we can modify it, it's real enough.
dnl $1 = the name to check
dnl $2 = its type
AC_DEFUN([CF_CHECK_EXTERN_DATA],
[
AC_CACHE_CHECK(if external $1 exists, cf_cv_have_$1,[
	AC_TRY_LINK([
#undef $1
extern $2 $1;
],
	[$1 = 2],
	[cf_cv_have_$1=yes],
	[cf_cv_have_$1=no])
])

if test "$cf_cv_have_$1" = yes ; then
	CF_UPPER(cf_result,have_$1)
	AC_DEFINE_UNQUOTED($cf_result)
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_FUNCDECL version: 7 updated: 2009/10/15 19:39:18
dnl -----------------
dnl Check if a function is declared by including a set of include files.
dnl Invoke the corresponding actions according to whether it is found or not.
dnl
dnl Gcc (unlike other compilers) will only warn about the miscast assignment
dnl in the first test, but most compilers will oblige with an error in the
dnl second test.
dnl
dnl CF_CHECK_FUNCDECL(INCLUDES, FUNCTION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN([CF_CHECK_FUNCDECL],
[
AC_MSG_CHECKING([for $2 declaration])
AC_CACHE_VAL(ac_cv_func_decl_$2,
[AC_TRY_COMPILE([$1],
[#ifndef $2
extern	int	$2();
#endif],[
AC_TRY_COMPILE([$1],
[#ifndef $2
int	(*p)() = $2;
#endif],[
eval "ac_cv_func_decl_$2=yes"],[
eval "ac_cv_func_decl_$2=no"])],[
eval "ac_cv_func_decl_$2=yes"])])
if eval "test \"`echo '$ac_cv_func_'decl_$2`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$3], , :, [$3])
else
  AC_MSG_RESULT(no)
ifelse([$4], , , [$4
])dnl
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_FUNCDECLS version: 4 updated: 1999/03/30 12:24:31
dnl ------------------
dnl Check if functions are declared by including a set of include files.
dnl and define DECL_XXX if not.
dnl
dnl CF_CHECK_FUNCDECLS(INCLUDES, FUNCTION... [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN([CF_CHECK_FUNCDECLS],
[for ac_func in $2
do
CF_CHECK_FUNCDECL([$1], $ac_func,
[$3],
[
  CF_UPPER(ac_tr_func,DECL_$ac_func)
  AC_DEFINE_UNQUOTED($ac_tr_func) $4])dnl
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_IPV6 version: 3 updated: 2004/01/22 17:38:22
dnl -------------
dnl Check for IPV6 configuration.
AC_DEFUN([CF_CHECK_IPV6],[
CF_FIND_IPV6_TYPE
CF_FIND_IPV6_LIBS

CF_FUNC_GETADDRINFO

if test "$cf_cv_getaddrinfo" != "yes"; then
	if test "$cf_cv_ipv6type" != "linux"; then
		AC_MSG_WARN(
[You must get working getaddrinfo() function,
or you can specify "--disable-ipv6"])
	else
		AC_MSG_WARN(
[The getaddrinfo() implementation on your system seems be buggy.
You should upgrade your system library to the newest version
of GNU C library (aka glibc).])
	fi
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_SIZEOF version: 4 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Improve on AC_CHECK_SIZEOF for cases when the build-environment is
dnl deficient, e.g., if someone tries to build in busybox.  Use the second
dnl parameter as the fallback value.
dnl
dnl By the way, 2.13/2.52 differ in AC_CHECK_SIZEOF regarding the types they
dnl can detect; the former includes only stdio.h for types while the latter
dnl includes several header files.
AC_DEFUN([CF_CHECK_SIZEOF],[
AC_CHECK_SIZEOF([$1],[$2])
if test "${ac_cv_type_$1+set}" = set; then
	cf_cv_sizeof="$ac_cv_sizeof_$1"
	if test "${ac_cv_sizeof_$1+set}" != set; then
		AC_MSG_WARN(using $2 for sizeof $1)
		ac_cv_sizeof_$1=$2
	elif test "x${ac_cv_sizeof_$1}" = x0; then
		AC_MSG_WARN([sizeof $1 not found, using $2])
		ac_cv_sizeof_$1=$2
	fi
	if test "x$ac_cv_sizeof_$1" != "x$cf_cv_sizeof"
	then
		CF_UPPER(cf_cv_type,sizeof_$1)
		sed -e "s/\\([[ 	]]${cf_cv_type}[[ 	]]\\).*/\\1$ac_cv_sizeof_$1/" confdefs.h >conftest.val
		mv conftest.val confdefs.h
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_SSL_X509 version: 5 updated: 2014/11/30 18:19:56
dnl -----------------
dnl Check for X509 support in the SSL library.
define([CF_CHECK_SSL_X509],[
AC_MSG_CHECKING(for X509 support)
AC_TRY_LINK(CF__SSL_HEAD [
#if defined(USE_GNUTLS_INCL)
#include <gnutls/x509.h>
#else
#include <openssl/x509.h>
#endif
],
	[X509_verify_cert_error_string(X509_STORE_CTX_get_error(NULL))],
	[cf_x509_support=yes],
	[cf_x509_support=no])
AC_MSG_RESULT($cf_x509_support)

if test "$cf_x509_support" = yes ; then
	AC_DEFINE(USE_X509_SUPPORT)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_TYPE version: 4 updated: 2021/01/02 09:31:20
dnl -------------
dnl Add a 3rd parameter to AC_CHECK_TYPE, working around autoconf 2.5x's
dnl deliberate incompatibility.
dnl	$1 = name of type to check for
dnl	$2 = default type
dnl	$3 = additional #include's and related preprocessor lines.
ifdef([m4_HAS_AC_CT_FOURARGS], [m4_undefine([m4_HAS_AC_CT_FOURARGS])])dnl
ifelse(m4_PACKAGE_VERSION, [fnord_acsalt], [],
[ifdef([m4_version_compare],[m4_define([m4_HAS_AC_CT_FOURARGS])])])dnl
AC_DEFUN([CF_CHECK_TYPE],
[
ifdef([m4_HAS_AC_CT_FOURARGS],[
	AC_CHECK_TYPE([$1],ac_cv_type_$1=yes,ac_cv_type_$1=no,[$3])
	],[
	AC_MSG_CHECKING(for $1)
	AC_TRY_COMPILE([
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
$3
],[
	static $1 dummy; if (sizeof(dummy)) return 0; else return 1;],
	ac_cv_type_$1=yes,
	ac_cv_type_$1=no)
	AC_MSG_RESULT($ac_cv_type_$1)
])dnl
if test "$ac_cv_type_$1" = no; then
	AC_DEFINE($1, $2, Define to $2 if $1 is not declared)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CLANG_COMPILER version: 8 updated: 2021/01/01 13:31:04
dnl -----------------
dnl Check if the given compiler is really clang.  clang's C driver defines
dnl __GNUC__ (fooling the configure script into setting $GCC to yes) but does
dnl not ignore some gcc options.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = CLANG_COMPILER (default)
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_CLANG_COMPILER],[
ifelse([$2],,CLANG_COMPILER,[$2])=no

if test "$ifelse([$1],,[$1],GCC)" = yes ; then
	AC_MSG_CHECKING(if this is really Clang ifelse([$1],GXX,C++,C) compiler)
	cf_save_CFLAGS="$ifelse([$3],,CFLAGS,[$3])"
	AC_TRY_COMPILE([],[
#ifdef __clang__
#else
make an error
#endif
],[ifelse([$2],,CLANG_COMPILER,[$2])=yes
],[])
	ifelse([$3],,CFLAGS,[$3])="$cf_save_CFLAGS"
	AC_MSG_RESULT($ifelse([$2],,CLANG_COMPILER,[$2]))
fi

CLANG_VERSION=none

if test "x$ifelse([$2],,CLANG_COMPILER,[$2])" = "xyes" ; then
	case "$CC" in
	(c[[1-9]][[0-9]]|*/c[[1-9]][[0-9]])
		AC_MSG_WARN(replacing broken compiler alias $CC)
		CFLAGS="$CFLAGS -std=`echo "$CC" | sed -e 's%.*/%%'`"
		CC=clang
		;;
	esac

	AC_MSG_CHECKING(version of $CC)
	CLANG_VERSION="`$CC --version 2>/dev/null | sed -e '2,$d' -e 's/^.*(CLANG[[^)]]*) //' -e 's/^.*(Debian[[^)]]*) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$CLANG_VERSION" && CLANG_VERSION=unknown
	AC_MSG_RESULT($CLANG_VERSION)

	for cf_clang_opt in \
		-Qunused-arguments \
		-Wno-error=implicit-function-declaration
	do
		AC_MSG_CHECKING(if option $cf_clang_opt works)
		cf_save_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $cf_clang_opt"
		AC_TRY_LINK([
			#include <stdio.h>],[
			printf("hello!\\n");],[
			cf_clang_optok=yes],[
			cf_clang_optok=no])
		AC_MSG_RESULT($cf_clang_optok)
		CFLAGS="$cf_save_CFLAGS"
		if test "$cf_clang_optok" = yes; then
			CF_VERBOSE(adding option $cf_clang_opt)
			CF_APPEND_TEXT(CFLAGS,$cf_clang_opt)
		fi
	done
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_COLOR_CURSES version: 9 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Check if curses supports color.  (Note that while SVr3 curses supports
dnl color, it does this differently from SVr4 curses; more work would be needed
dnl to accommodate SVr3).
dnl
AC_DEFUN([CF_COLOR_CURSES],
[
AC_MSG_CHECKING(if curses supports color attributes)
AC_CACHE_VAL(cf_cv_color_curses,[
	AC_TRY_LINK([
#include <${cf_cv_ncurses_header:-curses.h}>
],
	[chtype x = COLOR_BLUE;
	 has_colors();
	 start_color();
#ifndef NCURSES_BROKEN
	 wbkgd(curscr, getbkgd(stdscr)); /* X/Open XPG4 aka SVr4 Curses */
#endif
	],
	[cf_cv_color_curses=yes],
	[cf_cv_color_curses=no])
	])
AC_MSG_RESULT($cf_cv_color_curses)
if test "$cf_cv_color_curses" = yes ; then
	AC_DEFINE(COLOR_CURSES,1,[Define to 1 if if curses supports color attributes])
	test ".$cf_cv_ncurses_broken" != .yes && AC_DEFINE(HAVE_GETBKGD,1,[Define to 1 if curses has getbkgd function])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CONST_X_STRING version: 7 updated: 2021/06/07 17:39:17
dnl -----------------
dnl The X11R4-X11R6 Xt specification uses an ambiguous String type for most
dnl character-strings.
dnl
dnl It is ambiguous because the specification accommodated the pre-ANSI
dnl compilers bundled by more than one vendor in lieu of providing a standard C
dnl compiler other than by costly add-ons.  Because of this, the specification
dnl did not take into account the use of const for telling the compiler that
dnl string literals would be in readonly memory.
dnl
dnl As a workaround, one could (starting with X11R5) define XTSTRINGDEFINES, to
dnl let the compiler decide how to represent Xt's strings which were #define'd.
dnl That does not solve the problem of using the block of Xt's strings which
dnl are compiled into the library (and is less efficient than one might want).
dnl
dnl Xt specification 7 introduces the _CONST_X_STRING symbol which is used both
dnl when compiling the library and compiling using the library, to tell the
dnl compiler that String is const.
AC_DEFUN([CF_CONST_X_STRING],
[
AC_REQUIRE([AC_PATH_XTRA])

CF_SAVE_XTRA_FLAGS([CF_CONST_X_STRING])

AC_TRY_COMPILE(
[
#include <stdlib.h>
#include <X11/Intrinsic.h>
],
[String foo = malloc(1); free((void*)foo)],[

AC_CACHE_CHECK(for X11/Xt const-feature,cf_cv_const_x_string,[
	AC_TRY_COMPILE(
		[
#define _CONST_X_STRING	/* X11R7.8 (perhaps) */
#undef  XTSTRINGDEFINES	/* X11R5 and later */
#include <stdlib.h>
#include <X11/Intrinsic.h>
		],[String foo = malloc(1); *foo = 0],[
			cf_cv_const_x_string=no
		],[
			cf_cv_const_x_string=yes
		])
])

CF_RESTORE_XTRA_FLAGS([CF_CONST_X_STRING])

case "$cf_cv_const_x_string" in
(no)
	CF_APPEND_TEXT(CPPFLAGS,-DXTSTRINGDEFINES)
	;;
(*)
	CF_APPEND_TEXT(CPPFLAGS,-D_CONST_X_STRING)
	;;
esac

])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_CHTYPE version: 11 updated: 2021/01/02 09:31:20
dnl ----------------
dnl Test if curses defines 'chtype' (usually a 'long' type for SysV curses).
AC_DEFUN([CF_CURSES_CHTYPE],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
AC_CACHE_CHECK(for chtype typedef,cf_cv_chtype_decl,[
	AC_TRY_COMPILE([#include <${cf_cv_ncurses_header:-curses.h}>],
		[chtype foo; (void)foo],
		[cf_cv_chtype_decl=yes],
		[cf_cv_chtype_decl=no])])
if test "$cf_cv_chtype_decl" = yes ; then
	AC_DEFINE(HAVE_TYPE_CHTYPE,1,[Define to 1 if chtype is declared])
	AC_CACHE_CHECK(if chtype is scalar or struct,cf_cv_chtype_type,[
		AC_TRY_COMPILE([#include <${cf_cv_ncurses_header:-curses.h}>],
			[static chtype foo; long x = foo; (void)x],
			[cf_cv_chtype_type=scalar],
			[cf_cv_chtype_type=struct])])
	if test "$cf_cv_chtype_type" = scalar ; then
		AC_DEFINE(TYPE_CHTYPE_IS_SCALAR,1,[Define to 1 if chtype is a scaler/integer])
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_CONFIG version: 2 updated: 2006/10/29 11:06:27
dnl ----------------
dnl Tie together the configure-script macros for curses.  It may be ncurses,
dnl but unless asked, we do not make a special search for ncurses.  However,
dnl still check for the ncurses version number, for use in other macros.
AC_DEFUN([CF_CURSES_CONFIG],
[
CF_CURSES_CPPFLAGS
CF_NCURSES_VERSION
CF_CURSES_LIBS
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_CPPFLAGS version: 14 updated: 2021/01/02 09:31:20
dnl ------------------
dnl Look for the curses headers.
AC_DEFUN([CF_CURSES_CPPFLAGS],[

AC_CACHE_CHECK(for extra include directories,cf_cv_curses_incdir,[
cf_cv_curses_incdir=no
case "$host_os" in
(hpux10.*)
	if test "x$cf_cv_screen" = "xcurses_colr"
	then
		test -d /usr/include/curses_colr && \
		cf_cv_curses_incdir="-I/usr/include/curses_colr"
	fi
	;;
(sunos3*|sunos4*)
	if test "x$cf_cv_screen" = "xcurses_5lib"
	then
		test -d /usr/5lib && \
		test -d /usr/5include && \
		cf_cv_curses_incdir="-I/usr/5include"
	fi
	;;
esac
])
if test "$cf_cv_curses_incdir" != no
then
	CF_APPEND_TEXT(CPPFLAGS,$cf_cv_curses_incdir)
fi

CF_CURSES_HEADER
CF_TERM_HEADER
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_FUNCS version: 20 updated: 2020/12/31 20:19:42
dnl ---------------
dnl Curses-functions are a little complicated, since a lot of them are macros.
AC_DEFUN([CF_CURSES_FUNCS],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
AC_REQUIRE([CF_XOPEN_CURSES])
AC_REQUIRE([CF_CURSES_TERM_H])
AC_REQUIRE([CF_CURSES_UNCTRL_H])
for cf_func in $1
do
	CF_UPPER(cf_tr_func,$cf_func)
	AC_MSG_CHECKING(for ${cf_func})
	CF_MSG_LOG(${cf_func})
	AC_CACHE_VAL(cf_cv_func_$cf_func,[
		eval cf_result='$ac_cv_func_'$cf_func
		if test ".$cf_result" != ".no"; then
			AC_TRY_LINK(CF__CURSES_HEAD,
			[
#ifndef ${cf_func}
long foo = (long)(&${cf_func});
fprintf(stderr, "testing linkage of $cf_func:%p\\n", (void *)foo);
if (foo + 1234L > 5678L)
	${cf_cv_main_return:-return}(foo != 0);
#endif
			],
			[cf_result=yes],
			[cf_result=no])
		fi
		eval 'cf_cv_func_'$cf_func'="$cf_result"'
	])
	# use the computed/retrieved cache-value:
	eval 'cf_result=$cf_cv_func_'$cf_func
	AC_MSG_RESULT($cf_result)
	if test "$cf_result" != no; then
		AC_DEFINE_UNQUOTED(HAVE_${cf_tr_func})
	fi
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_HEADER version: 5 updated: 2015/04/23 20:35:30
dnl ----------------
dnl Find a "curses" header file, e.g,. "curses.h", or one of the more common
dnl variations of ncurses' installs.
dnl
dnl $1 = ncurses when looking for ncurses, or is empty
AC_DEFUN([CF_CURSES_HEADER],[
AC_CACHE_CHECK(if we have identified curses headers,cf_cv_ncurses_header,[
cf_cv_ncurses_header=none
for cf_header in \
	ncurses.h ifelse($1,,,[$1/ncurses.h]) \
	curses.h ifelse($1,,,[$1/curses.h]) ifelse($1,,[ncurses/ncurses.h ncurses/curses.h])
do
AC_TRY_COMPILE([#include <${cf_header}>],
	[initscr(); tgoto("?", 0,0)],
	[cf_cv_ncurses_header=$cf_header; break],[])
done
])

if test "$cf_cv_ncurses_header" = none ; then
	AC_MSG_ERROR(No curses header-files found)
fi

# cheat, to get the right #define's for HAVE_NCURSES_H, etc.
AC_CHECK_HEADERS($cf_cv_ncurses_header)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_LIBS version: 44 updated: 2021/01/02 09:31:20
dnl --------------
dnl Look for the curses libraries.  Older curses implementations may require
dnl termcap/termlib to be linked as well.  Call CF_CURSES_CPPFLAGS first.
AC_DEFUN([CF_CURSES_LIBS],[

AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
AC_MSG_CHECKING(if we have identified curses libraries)
AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
	[initscr(); tgoto("?", 0,0)],
	cf_result=yes,
	cf_result=no)
AC_MSG_RESULT($cf_result)

if test "$cf_result" = no ; then
case "$host_os" in
(freebsd*)
	AC_CHECK_LIB(mytinfo,tgoto,[CF_ADD_LIBS(-lmytinfo)])
	;;
(hpux10.*)
	# Looking at HPUX 10.20, the Hcurses library is the oldest (1997), cur_colr
	# next (1998), and xcurses "newer" (2000).  There is no header file for
	# Hcurses; the subdirectory curses_colr has the headers (curses.h and
	# term.h) for cur_colr
	if test "x$cf_cv_screen" = "xcurses_colr"
	then
		AC_CHECK_LIB(cur_colr,initscr,[
			CF_ADD_LIBS(-lcur_colr)
			ac_cv_func_initscr=yes
			],[
		AC_CHECK_LIB(Hcurses,initscr,[
			# HP's header uses __HP_CURSES, but user claims _HP_CURSES.
			CF_ADD_LIBS(-lHcurses)
			CF_APPEND_TEXT(CPPFLAGS,-D__HP_CURSES -D_HP_CURSES)
			ac_cv_func_initscr=yes
			])])
	fi
	;;
(linux*)
	case `arch 2>/dev/null` in
	(x86_64)
		if test -d /lib64
		then
			CF_ADD_LIBDIR(/lib64)
		else
			CF_ADD_LIBDIR(/lib)
		fi
		;;
	(*)
		CF_ADD_LIBDIR(/lib)
		;;
	esac
	;;
(sunos3*|sunos4*)
	if test "x$cf_cv_screen" = "xcurses_5lib"
	then
		if test -d /usr/5lib ; then
			CF_ADD_LIBDIR(/usr/5lib)
			CF_ADD_LIBS(-lcurses -ltermcap)
		fi
	fi
	ac_cv_func_initscr=yes
	;;
esac

if test ".$ac_cv_func_initscr" != .yes ; then
	cf_save_LIBS="$LIBS"

	if test ".${cf_cv_ncurses_version:-no}" != .no
	then
		cf_check_list="ncurses curses cursesX"
	else
		cf_check_list="cursesX curses ncurses"
	fi

	# Check for library containing tgoto.  Do this before curses library
	# because it may be needed to link the test-case for initscr.
	if test "x$cf_term_lib" = x
	then
		AC_CHECK_FUNC(tgoto,[cf_term_lib=predefined],[
			for cf_term_lib in $cf_check_list otermcap termcap tinfo termlib unknown
			do
				AC_CHECK_LIB($cf_term_lib,tgoto,[
					: "${cf_nculib_root:=$cf_term_lib}"
					break
				])
			done
		])
	fi

	# Check for library containing initscr
	test "$cf_term_lib" != predefined && test "$cf_term_lib" != unknown && LIBS="-l$cf_term_lib $cf_save_LIBS"
	if test "x$cf_curs_lib" = x
	then
		for cf_curs_lib in $cf_check_list xcurses jcurses pdcurses unknown
		do
			LIBS="-l$cf_curs_lib $cf_save_LIBS"
			if test "$cf_term_lib" = unknown || test "$cf_term_lib" = "$cf_curs_lib" ; then
				AC_MSG_CHECKING(if we can link with $cf_curs_lib library)
				AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
					[initscr()],
					[cf_result=yes],
					[cf_result=no])
				AC_MSG_RESULT($cf_result)
				test "$cf_result" = yes && break
			elif test "$cf_curs_lib" = "$cf_term_lib" ; then
				cf_result=no
			elif test "$cf_term_lib" != predefined ; then
				AC_MSG_CHECKING(if we need both $cf_curs_lib and $cf_term_lib libraries)
				AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
					[initscr(); tgoto((char *)0, 0, 0);],
					[cf_result=no],
					[
					LIBS="-l$cf_curs_lib -l$cf_term_lib $cf_save_LIBS"
					AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
						[initscr()],
						[cf_result=yes],
						[cf_result=error])
					])
				AC_MSG_RESULT($cf_result)
				test "$cf_result" != error && break
			fi
		done
	fi
	test "$cf_curs_lib" = unknown && AC_MSG_ERROR(no curses library found)
fi
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_TERM_H version: 15 updated: 2021/01/02 09:31:20
dnl ----------------
dnl SVr4 curses should have term.h as well (where it puts the definitions of
dnl the low-level interface).  This may not be true in old/broken implementations,
dnl as well as in misconfigured systems (e.g., gcc configured for Solaris 2.4
dnl running with Solaris 2.5.1).
AC_DEFUN([CF_CURSES_TERM_H],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl

AC_CACHE_CHECK(for term.h, cf_cv_term_header,[

# If we found <ncurses/curses.h>, look for <ncurses/term.h>, but always look
# for <term.h> if we do not find the variant.

cf_header_list="term.h ncurses/term.h ncursesw/term.h"

case "${cf_cv_ncurses_header:-curses.h}" in
(*/*)
	cf_header_item=`echo "${cf_cv_ncurses_header:-curses.h}" | sed -e 's%\..*%%' -e 's%/.*%/%'`term.h
	cf_header_list="$cf_header_item $cf_header_list"
	;;
esac

for cf_header in $cf_header_list
do
	AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header:-curses.h}>
#include <${cf_header}>],
	[WINDOW *x; (void)x],
	[cf_cv_term_header=$cf_header
	 break],
	[cf_cv_term_header=no])
done

case "$cf_cv_term_header" in
(no)
	# If curses is ncurses, some packagers still mess it up by trying to make
	# us use GNU termcap.  This handles the most common case.
	for cf_header in ncurses/term.h ncursesw/term.h
	do
		AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header:-curses.h}>
#ifdef NCURSES_VERSION
#include <${cf_header}>
#else
make an error
#endif],
			[WINDOW *x; (void)x],
			[cf_cv_term_header=$cf_header
			 break],
			[cf_cv_term_header=no])
	done
	;;
esac
])

case "$cf_cv_term_header" in
(term.h)
	AC_DEFINE(HAVE_TERM_H,1,[Define to 1 if we have term.h])
	;;
(ncurses/term.h)
	AC_DEFINE(HAVE_NCURSES_TERM_H,1,[Define to 1 if we have ncurses/term.h])
	;;
(ncursesw/term.h)
	AC_DEFINE(HAVE_NCURSESW_TERM_H,1,[Define to 1 if we have ncursesw/term.h])
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_UNCTRL_H version: 8 updated: 2021/01/02 09:31:20
dnl ------------------
dnl Any X/Open curses implementation must have unctrl.h, but ncurses packages
dnl may put it in a subdirectory (along with ncurses' other headers, of
dnl course).  Packages which put the headers in inconsistent locations are
dnl broken).
AC_DEFUN([CF_CURSES_UNCTRL_H],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl

AC_CACHE_CHECK(for unctrl.h, cf_cv_unctrl_header,[

# If we found <ncurses/curses.h>, look for <ncurses/unctrl.h>, but always look
# for <unctrl.h> if we do not find the variant.

cf_header_list="unctrl.h ncurses/unctrl.h ncursesw/unctrl.h"

case "${cf_cv_ncurses_header:-curses.h}" in
(*/*)
	cf_header_item=`echo "${cf_cv_ncurses_header:-curses.h}" | sed -e 's%\..*%%' -e 's%/.*%/%'`unctrl.h
	cf_header_list="$cf_header_item $cf_header_list"
	;;
esac

for cf_header in $cf_header_list
do
	AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header:-curses.h}>
#include <${cf_header}>],
	[WINDOW *x; (void)x],
	[cf_cv_unctrl_header=$cf_header
	 break],
	[cf_cv_unctrl_header=no])
done
])

case "$cf_cv_unctrl_header" in
(no)
	AC_MSG_WARN(unctrl.h header not found)
	;;
esac

case "$cf_cv_unctrl_header" in
(unctrl.h)
	AC_DEFINE(HAVE_UNCTRL_H,1,[Define to 1 if we have unctrl.h])
	;;
(ncurses/unctrl.h)
	AC_DEFINE(HAVE_NCURSES_UNCTRL_H,1,[Define to 1 if we have ncurses/unctrl.h])
	;;
(ncursesw/unctrl.h)
	AC_DEFINE(HAVE_NCURSESW_UNCTRL_H,1,[Define to 1 if we have ncursesw/unctrl.h])
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURS_PERFORMANCE version: 6 updated: 2021/01/02 09:31:20
dnl -------------------
dnl Solaris 2.x curses provides a "performance" tradeoff according to whether
dnl CURS_PERFORMANCE is defined.  If defined, the implementation defines macros
dnl that access the WINDOW structure.  Otherwise, function calls are used.
AC_DEFUN([CF_CURS_PERFORMANCE],
[
AC_MSG_CHECKING([for curses performance tradeoff])
AC_CACHE_VAL(cf_cv_curs_performance,[
    cf_cv_curs_performance=no
    AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header:-curses.h}>],[
#if defined(wbkgdset) && defined(clearok) && defined(getbkgd)
	int x = ERR;
#else
	int x = ;	/* force an error */
#endif
	],[
	AC_TRY_COMPILE([
#define CURS_PERFORMANCE
#include <${cf_cv_ncurses_header:-curses.h}>],[
#if defined(wbkgdset) && defined(clearok) && defined(getbkgd)
	int x = ;	/* force an error */
#else
	int x = ERR;
#endif
	],[cf_cv_curs_performance=yes])])])
AC_MSG_RESULT($cf_cv_curs_performance)
test "$cf_cv_curs_performance" = yes && AC_DEFINE(CURS_PERFORMANCE,1,[Define to 1 ifr curses performance tradeoff available])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURS_TOUCHLINE version: 5 updated: 2015/04/15 19:08:48
dnl -----------------
dnl Check for the flavor of the touchline function, to distinguish between BSD
dnl and SYSV.  This is needed on NetBSD 1.5 which has a partial implementation
dnl of SVR4 curses.
AC_DEFUN([CF_CURS_TOUCHLINE],[
AC_CACHE_CHECK(for curses touchline function,cf_cv_curs_touchline,[
	AC_TRY_LINK([
#include <${cf_cv_ncurses_header:-curses.h}>],
	[touchline(stdscr, 1,2,3);],
	[cf_cv_curs_touchline=bsd],
	[AC_TRY_LINK([
#include <${cf_cv_ncurses_header:-curses.h}>],
		[touchline(stdscr, 1,2);],
		[cf_cv_curs_touchline=sysv],
		[cf_cv_curs_touchline=bsd])])])
case "$cf_cv_curs_touchline" in
(bsd)
	AC_DEFINE(HAVE_BSD_TOUCHLINE,1,[Define to 1 if curses has bsd-style touchline])
	;;
(sysv)
	AC_DEFINE(HAVE_SYSV_TOUCHLINE,1,[Define to 1 if curses has sysv-style touchline])
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DEFINE_PROG version: 5 updated: 2021/01/02 09:31:20
dnl --------------
dnl Check for a program in the given list $3, defining the corresponding
dnl program variable $2.
dnl
AC_DEFUN([CF_DEFINE_PROG],[
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL(cf_cv_$2,[
	cf_cv_$2=unknown
	for cv_path in $3
	do
		if test -f "$cv_path" ; then
			cf_cv_$2="$cv_path"
			break
		fi
	done
	])
AC_MSG_RESULT($cf_cv_$2)
AC_DEFINE_UNQUOTED($2,"$cf_cv_$2",[Define to program from list])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DIRNAME version: 5 updated: 2020/12/31 20:19:42
dnl ----------
dnl "dirname" is not portable, so we fake it with a shell script.
AC_DEFUN([CF_DIRNAME],[$1=`echo "$2" | sed -e 's%/[[^/]]*$%%'`])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_ECHO version: 13 updated: 2015/04/18 08:56:57
dnl ---------------
dnl You can always use "make -n" to see the actual options, but it's hard to
dnl pick out/analyze warning messages when the compile-line is long.
dnl
dnl Sets:
dnl	ECHO_LT - symbol to control if libtool is verbose
dnl	ECHO_LD - symbol to prefix "cc -o" lines
dnl	RULE_CC - symbol to put before implicit "cc -c" lines (e.g., .c.o)
dnl	SHOW_CC - symbol to put before explicit "cc -c" lines
dnl	ECHO_CC - symbol to put before any "cc" line
dnl
AC_DEFUN([CF_DISABLE_ECHO],[
AC_MSG_CHECKING(if you want to see long compiling messages)
CF_ARG_DISABLE(echo,
	[  --disable-echo          do not display "compiling" commands],
	[
	ECHO_LT='--silent'
	ECHO_LD='@echo linking [$]@;'
	RULE_CC='@echo compiling [$]<'
	SHOW_CC='@echo compiling [$]@'
	ECHO_CC='@'
],[
	ECHO_LT=''
	ECHO_LD=''
	RULE_CC=''
	SHOW_CC=''
	ECHO_CC=''
])
AC_MSG_RESULT($enableval)
AC_SUBST(ECHO_LT)
AC_SUBST(ECHO_LD)
AC_SUBST(RULE_CC)
AC_SUBST(SHOW_CC)
AC_SUBST(ECHO_CC)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_RPATH_HACK version: 3 updated: 2021/01/05 20:14:44
dnl ---------------------
dnl The rpath-hack makes it simpler to build programs, particularly with the
dnl *BSD ports which may have essential libraries in unusual places.  But it
dnl can interfere with building an executable for the base system.  Use this
dnl option in that case.
AC_DEFUN([CF_DISABLE_RPATH_HACK],
[
AC_MSG_CHECKING(if rpath-hack should be disabled)
CF_ARG_DISABLE(rpath-hack,
	[  --disable-rpath-hack    don't add rpath options for additional libraries],
	[enable_rpath_hack=no],
	[enable_rpath_hack=yes])
dnl TODO - drop cf_disable_rpath_hack
if test "x$enable_rpath_hack" = xno; then cf_disable_rpath_hack=yes; else cf_disable_rpath_hack=no; fi
AC_MSG_RESULT($cf_disable_rpath_hack)

if test "$enable_rpath_hack" = yes ; then
	CF_RPATH_HACK
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_ENABLE_WARNINGS version: 9 updated: 2021/01/05 19:40:50
dnl ------------------
dnl Configure-option to enable gcc warnings
dnl
dnl $1 = extra options to add, if supported
dnl $2 = option for checking attributes.  By default, this is done when
dnl      warnings are enabled.  For other values:
dnl      yes: always do this, e.g., to use in generated library-headers
dnl      no: never do this
AC_DEFUN([CF_ENABLE_WARNINGS],[
if test "$GCC" = yes || test "$GXX" = yes
then
CF_FIX_WARNINGS(CFLAGS)
CF_FIX_WARNINGS(CPPFLAGS)
CF_FIX_WARNINGS(LDFLAGS)
AC_MSG_CHECKING(if you want to turn on gcc warnings)
CF_ARG_ENABLE(warnings,
	[  --enable-warnings       test: turn on gcc compiler warnings],
	[enable_warnings=yes],
	[enable_warnings=no])
AC_MSG_RESULT($enable_warnings)
if test "$enable_warnings" = "yes"
then
	ifelse($2,,[CF_GCC_ATTRIBUTES])
	CF_GCC_WARNINGS($1)
fi
ifelse($2,yes,[CF_GCC_ATTRIBUTES])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ERRNO version: 5 updated: 1997/11/30 12:44:39
dnl --------
dnl Check if 'errno' is declared in <errno.h>
AC_DEFUN([CF_ERRNO],
[
CF_CHECK_ERRNO(errno)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FANCY_CURSES version: 7 updated: 2021/01/02 09:31:20
dnl ---------------
AC_DEFUN([CF_FANCY_CURSES],
[
AC_MSG_CHECKING(if curses supports fancy attributes)
AC_CACHE_VAL(cf_cv_fancy_curses,[
	AC_TRY_LINK([
#include <${cf_cv_ncurses_header:-curses.h}>
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
test "$cf_cv_fancy_curses" = yes && AC_DEFINE(FANCY_CURSES,1,[Define to 1 if curses supports fancy attributes])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_IPV6_LIBS version: 9 updated: 2021/01/02 09:31:20
dnl -----------------
dnl Based on the IPV6 stack type, look for the corresponding library.
AC_DEFUN([CF_FIND_IPV6_LIBS],[
AC_REQUIRE([CF_FIND_IPV6_TYPE])

cf_ipv6lib=none
cf_ipv6dir=none

AC_MSG_CHECKING(for IPv6 library if required)
case "$cf_cv_ipv6type" in
(solaris)
	;;
(inria)
	;;
(kame)
	dnl http://www.kame.net/
	cf_ipv6lib=inet6
	cf_ipv6dir=v6
	;;
(linux-glibc)
	;;
(linux-libinet6)
	dnl http://www.v6.linux.or.jp/
	cf_ipv6lib=inet6
	cf_ipv6dir=inet6
	;;
(toshiba)
	cf_ipv6lib=inet6
	cf_ipv6dir=v6
	;;
(v6d)
	cf_ipv6lib=v6
	cf_ipv6dir=v6
	;;
(zeta)
	cf_ipv6lib=inet6
	cf_ipv6dir=v6
	;;
esac
AC_MSG_RESULT($cf_ipv6lib)

if test "$cf_ipv6lib" != "none"; then

	AC_TRY_LINK([
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip6.h>],
	[getaddrinfo(0, 0, 0, 0)],,[
	CF_HEADER_PATH(cf_search,$cf_ipv6dir)
	for cf_incdir in $cf_search
	do
		cf_header=$cf_incdir/netinet/ip6.h
		if test -f "$cf_header"
		then
			CF_ADD_INCDIR($cf_incdir)
			test -n "$verbose" && echo "	... found $cf_header" 1>&AC_FD_MSG
			break
		fi
		test -n "$verbose" && echo "	... tested $cf_header" 1>&AC_FD_MSG
	done
	])

	CF_FIND_LIBRARY([$cf_ipv6lib],[$cf_ipv6dir],[
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip6.h>],
	[getaddrinfo(0, 0, 0, 0)],
	[getaddrinfo],
	noexit)
	if test "$cf_found_library" = no ; then
		AC_MSG_ERROR(
[No $cf_ipv6lib library found, cannot continue.  You must fetch lib$cf_ipv6lib.a
from an appropriate IPv6 kit and compile beforehand.])
	fi
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_IPV6_TYPE version: 7 updated: 2021/01/02 09:31:20
dnl -----------------
AC_DEFUN([CF_FIND_IPV6_TYPE],[
AC_CACHE_CHECK(ipv6 stack type, cf_cv_ipv6type, [
cf_cv_ipv6type=unknown
for i in solaris inria kame linux-glibc linux-libinet6 toshiba v6d zeta
do
	case "$i" in
	(solaris)
		if test "SunOS" = "`uname -s`"
		then
		  if test -f /usr/include/netinet/ip6.h
		  then
			cf_cv_ipv6type=$i
		  fi
		fi
		;;
	(inria)
		dnl http://www.kame.net/
		AC_EGREP_CPP(yes, [
#include <netinet/in.h>
#ifdef IPV6_INRIA_VERSION
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	(kame)
		dnl http://www.kame.net/
		AC_EGREP_CPP(yes, [
#include <netinet/in.h>
#ifdef __KAME__
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	(linux-glibc)
		dnl http://www.v6.linux.or.jp/
		AC_EGREP_CPP(yes, [
#include <features.h>
#if defined(__GLIBC__) && __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	(linux-libinet6)
		dnl http://www.v6.linux.or.jp/
		if test -d /usr/inet6
		then
			cf_cv_ipv6type=$i
		elif test -f /usr/include/netinet/ip6.h
		then
			cf_cv_ipv6type=$i
		fi
		;;
	(toshiba)
		AC_EGREP_CPP(yes, [
#include <sys/param.h>
#ifdef _TOSHIBA_INET6
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	(v6d)
		AC_EGREP_CPP(yes, [
#include </usr/local/v6/include/sys/v6config.h>
#ifdef __V6D__
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	(zeta)
		AC_EGREP_CPP(yes, [
#include <sys/param.h>
#ifdef _ZETA_MINAMI_INET6
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	esac
	if test "$cf_cv_ipv6type" != "unknown"; then
		break
	fi
done
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_LIBRARY version: 11 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Look for a non-standard library, given parameters for AC_TRY_LINK.  We
dnl prefer a standard location, and use -L options only if we do not find the
dnl library in the standard library location(s).
dnl	$1 = library name
dnl	$2 = library class, usually the same as library name
dnl	$3 = includes
dnl	$4 = code fragment to compile/link
dnl	$5 = corresponding function-name
dnl	$6 = flag, nonnull if failure should not cause an error-exit
dnl
dnl Sets the variable "$cf_libdir" as a side-effect, so we can see if we had
dnl to use a -L option.
AC_DEFUN([CF_FIND_LIBRARY],
[
	eval 'cf_cv_have_lib_'"$1"'=no'
	cf_libdir=""
	AC_CHECK_FUNC($5,
		eval 'cf_cv_have_lib_'"$1"'=yes',[
		cf_save_LIBS="$LIBS"
		AC_MSG_CHECKING(for $5 in -l$1)
		LIBS="-l$1 $LIBS"
		AC_TRY_LINK([$3],[$4],
			[AC_MSG_RESULT(yes)
			 eval 'cf_cv_have_lib_'"$1"'=yes'
			],
			[AC_MSG_RESULT(no)
			CF_LIBRARY_PATH(cf_search,$2)
			for cf_libdir in $cf_search
			do
				AC_MSG_CHECKING(for -l$1 in $cf_libdir)
				LIBS="-L$cf_libdir -l$1 $cf_save_LIBS"
				AC_TRY_LINK([$3],[$4],
					[AC_MSG_RESULT(yes)
			 		 eval 'cf_cv_have_lib_'"$1"'=yes'
					 break],
					[AC_MSG_RESULT(no)
					 LIBS="$cf_save_LIBS"])
			done
			])
		])
eval 'cf_found_library="[$]cf_cv_have_lib_'"$1"\"
ifelse($6,,[
if test "$cf_found_library" = no ; then
	AC_MSG_ERROR(Cannot link $1 library)
fi
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_LINKAGE version: 22 updated: 2020/12/31 20:19:42
dnl ---------------
dnl Find a library (specifically the linkage used in the code fragment),
dnl searching for it if it is not already in the library path.
dnl See also CF_ADD_SEARCHPATH.
dnl
dnl Parameters (4-on are optional):
dnl     $1 = headers for library entrypoint
dnl     $2 = code fragment for library entrypoint
dnl     $3 = the library name without the "-l" option or ".so" suffix.
dnl     $4 = action to perform if successful (default: update CPPFLAGS, etc)
dnl     $5 = action to perform if not successful
dnl     $6 = module name, if not the same as the library name
dnl     $7 = extra libraries
dnl
dnl Sets these variables:
dnl     $cf_cv_find_linkage_$3 - yes/no according to whether linkage is found
dnl     $cf_cv_header_path_$3 - include-directory if needed
dnl     $cf_cv_library_path_$3 - library-directory if needed
dnl     $cf_cv_library_file_$3 - library-file if needed, e.g., -l$3
AC_DEFUN([CF_FIND_LINKAGE],[

# If the linkage is not already in the $CPPFLAGS/$LDFLAGS configuration, these
# will be set on completion of the AC_TRY_LINK below.
cf_cv_header_path_$3=
cf_cv_library_path_$3=

CF_MSG_LOG([Starting [FIND_LINKAGE]($3,$6)])

cf_save_LIBS="$LIBS"

AC_TRY_LINK([$1],[$2],[
	cf_cv_find_linkage_$3=yes
	cf_cv_header_path_$3=/usr/include
	cf_cv_library_path_$3=/usr/lib
],[

LIBS="-l$3 $7 $cf_save_LIBS"

AC_TRY_LINK([$1],[$2],[
	cf_cv_find_linkage_$3=yes
	cf_cv_header_path_$3=/usr/include
	cf_cv_library_path_$3=/usr/lib
	cf_cv_library_file_$3="-l$3"
],[
	cf_cv_find_linkage_$3=no
	LIBS="$cf_save_LIBS"

	CF_VERBOSE(find linkage for $3 library)
	CF_MSG_LOG([Searching for headers in [FIND_LINKAGE]($3,$6)])

	cf_save_CPPFLAGS="$CPPFLAGS"
	cf_test_CPPFLAGS="$CPPFLAGS"

	CF_HEADER_PATH(cf_search,ifelse([$6],,[$3],[$6]))
	for cf_cv_header_path_$3 in $cf_search
	do
		if test -d "$cf_cv_header_path_$3" ; then
			CF_VERBOSE(... testing $cf_cv_header_path_$3)
			CPPFLAGS="$cf_save_CPPFLAGS"
			CF_APPEND_TEXT(CPPFLAGS,-I$cf_cv_header_path_$3)
			AC_TRY_COMPILE([$1],[$2],[
				CF_VERBOSE(... found $3 headers in $cf_cv_header_path_$3)
				cf_cv_find_linkage_$3=maybe
				cf_test_CPPFLAGS="$CPPFLAGS"
				break],[
				CPPFLAGS="$cf_save_CPPFLAGS"
				])
		fi
	done

	if test "$cf_cv_find_linkage_$3" = maybe ; then

		CF_MSG_LOG([Searching for $3 library in [FIND_LINKAGE]($3,$6)])

		cf_save_LIBS="$LIBS"
		cf_save_LDFLAGS="$LDFLAGS"

		ifelse([$6],,,[
		CPPFLAGS="$cf_test_CPPFLAGS"
		LIBS="-l$3 $7 $cf_save_LIBS"
		AC_TRY_LINK([$1],[$2],[
			CF_VERBOSE(... found $3 library in system)
			cf_cv_find_linkage_$3=yes])
			CPPFLAGS="$cf_save_CPPFLAGS"
			LIBS="$cf_save_LIBS"
			])

		if test "$cf_cv_find_linkage_$3" != yes ; then
			CF_LIBRARY_PATH(cf_search,$3)
			for cf_cv_library_path_$3 in $cf_search
			do
				if test -d "$cf_cv_library_path_$3" ; then
					CF_VERBOSE(... testing $cf_cv_library_path_$3)
					CPPFLAGS="$cf_test_CPPFLAGS"
					LIBS="-l$3 $7 $cf_save_LIBS"
					LDFLAGS="$cf_save_LDFLAGS -L$cf_cv_library_path_$3"
					AC_TRY_LINK([$1],[$2],[
					CF_VERBOSE(... found $3 library in $cf_cv_library_path_$3)
					cf_cv_find_linkage_$3=yes
					cf_cv_library_file_$3="-l$3"
					break],[
					CPPFLAGS="$cf_save_CPPFLAGS"
					LIBS="$cf_save_LIBS"
					LDFLAGS="$cf_save_LDFLAGS"
					])
				fi
			done
			CPPFLAGS="$cf_save_CPPFLAGS"
			LDFLAGS="$cf_save_LDFLAGS"
		fi

	else
		cf_cv_find_linkage_$3=no
	fi
	],$7)
])

LIBS="$cf_save_LIBS"

if test "$cf_cv_find_linkage_$3" = yes ; then
ifelse([$4],,[
	CF_ADD_INCDIR($cf_cv_header_path_$3)
	CF_ADD_LIBDIR($cf_cv_library_path_$3)
	CF_ADD_LIB($3)
],[$4])
else
ifelse([$5],,AC_MSG_WARN(Cannot find $3 library),[$5])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIONBIO version: 4 updated: 2021/06/08 18:08:14
dnl ----------
dnl Check for availability of fcntl versus ioctl(,FIONBIO,).  Lynx uses this
dnl for Sequent (ptx), and it is needed for OS/2 EMX.
AC_DEFUN([CF_FIONBIO],
[
AC_CACHE_CHECK(if we should use fcntl or ioctl,cf_cv_fionbio,[
AC_TRY_LINK([
#include <sys/types.h>
#include <sys/ioctl.h>
],[
        int ret = ioctl(0, FIONBIO, (char *)0); (void) ret
	],[cf_cv_fionbio=ioctl],[
AC_TRY_LINK([
#include <sys/types.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#else
#if HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#endif],[
        int ret = fcntl(0, F_SETFL, O_NONBLOCK);
	],
	[cf_cv_fionbio=fcntl],
	[cf_cv_fionbio=unknown])])
])
test "$cf_cv_fionbio" = "fcntl" && AC_DEFINE(USE_FCNTL,1,[Define to 1 if we should use fcntl])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIX_WARNINGS version: 3 updated: 2020/12/31 18:40:20
dnl ---------------
dnl Warning flags do not belong in CFLAGS, CPPFLAGS, etc.  Any of gcc's
dnl "-Werror" flags can interfere with configure-checks.  Those go into
dnl EXTRA_CFLAGS.
dnl
dnl $1 = variable name to repair
define([CF_FIX_WARNINGS],[
if test "$GCC" = yes || test "$GXX" = yes
then
	case [$]$1 in
	(*-Werror=*)
		CF_VERBOSE(repairing $1: [$]$1)
		cf_temp_flags=
		for cf_temp_scan in [$]$1
		do
			case "x$cf_temp_scan" in
			(x-Werror=*)
				CF_APPEND_TEXT(EXTRA_CFLAGS,$cf_temp_scan)
				;;
			(*)
				CF_APPEND_TEXT(cf_temp_flags,$cf_temp_scan)
				;;
			esac
		done
		$1="$cf_temp_flags"
		CF_VERBOSE(... fixed [$]$1)
		CF_VERBOSE(... extra $EXTRA_CFLAGS)
		;;
	esac
fi
AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_CURSES_VERSION version: 8 updated: 2021/01/02 09:31:20
dnl ----------------------
dnl Solaris has a data item 'curses_version', which confuses AC_CHECK_FUNCS.
dnl It's a character string "SVR4", not documented.
AC_DEFUN([CF_FUNC_CURSES_VERSION],
[
AC_CACHE_CHECK(for function curses_version, cf_cv_func_curses_version,[
AC_TRY_RUN([
#include <${cf_cv_ncurses_header:-curses.h}>
int main(void)
{
	char temp[1024];
	sprintf(temp, "%s\\n", curses_version());
	${cf_cv_main_return:-return}(0);
}]
,[cf_cv_func_curses_version=yes]
,[cf_cv_func_curses_version=no]
,[cf_cv_func_curses_version=unknown])
rm -f core])
test "$cf_cv_func_curses_version" = yes && AC_DEFINE(HAVE_CURSES_VERSION,1,[Define to 1 if we have curses_version function])
])
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_GETADDRINFO version: 9 updated: 2017/05/10 18:31:29
dnl -------------------
dnl Look for a working version of getaddrinfo(), for IPV6 support.
AC_DEFUN([CF_FUNC_GETADDRINFO],[
AC_CACHE_CHECK(working getaddrinfo, cf_cv_getaddrinfo,[
AC_TRY_RUN([
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define expect(a,b) if (strcmp(a,b) != 0) goto bad

int main(void)
{
   int passive, gaierr, inet4 = 0, inet6 = 0;
   struct addrinfo hints, *ai, *aitop;
   char straddr[INET6_ADDRSTRLEN], strport[16];

   for (passive = 0; passive <= 1; passive++) {
     memset(&hints, 0, sizeof(hints));
     hints.ai_family = AF_UNSPEC;
     hints.ai_flags = passive ? AI_PASSIVE : 0;
     hints.ai_socktype = SOCK_STREAM;
     if ((gaierr = getaddrinfo(NULL, "54321", &hints, &aitop)) != 0) {
       (void)gai_strerror(gaierr);
       goto bad;
     }
     for (ai = aitop; ai; ai = ai->ai_next) {
       if (ai->ai_addr == NULL ||
           ai->ai_addrlen == 0 ||
           getnameinfo(ai->ai_addr, ai->ai_addrlen,
                       straddr, sizeof(straddr), strport, sizeof(strport),
                       NI_NUMERICHOST|NI_NUMERICSERV) != 0) {
         goto bad;
       }
       switch (ai->ai_family) {
       case AF_INET:
         expect(strport, "54321");
         if (passive) {
           expect(straddr, "0.0.0.0");
         } else {
           expect(straddr, "127.0.0.1");
         }
         inet4++;
         break;
       case AF_INET6:
         expect(strport, "54321");
         if (passive) {
           expect(straddr, "::");
         } else {
           expect(straddr, "::1");
         }
         inet6++;
         break;
       case AF_UNSPEC:
         goto bad;
         break;
       default:
         /* another family support? */
         break;
       }
     }
   }

   if (!(inet4 == 0 || inet4 == 2))
     goto bad;
   if (!(inet6 == 0 || inet6 == 2))
     goto bad;

   if (aitop)
     freeaddrinfo(aitop);
   ${cf_cv_main_return:-return}(0);

  bad:
   if (aitop)
     freeaddrinfo(aitop);
   ${cf_cv_main_return:-return}(1);
}
],
[cf_cv_getaddrinfo=yes],
[cf_cv_getaddrinfo=no],
[cf_cv_getaddrinfo=unknown])
])
if test "$cf_cv_getaddrinfo" = yes ; then
	AC_DEFINE(HAVE_GAI_STRERROR,1,[Define to 1 if we have gai_strerror function])
	AC_DEFINE(HAVE_GETADDRINFO,1,[Define to 1 if we have getaddrinfo function])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_LSTAT version: 5 updated: 2021/06/08 18:08:14
dnl -------------
dnl A conventional existence-check for 'lstat' won't work with the Linux
dnl version of gcc 2.7.0, since the symbol is defined only within <sys/stat.h>
dnl as an inline function.
dnl
dnl So much for portability.
AC_DEFUN([CF_FUNC_LSTAT],
[
AC_MSG_CHECKING(for lstat)
AC_CACHE_VAL(ac_cv_func_lstat,[
AC_TRY_LINK([
#include <sys/types.h>
#include <sys/stat.h>],
	[struct stat sb; lstat(".", &sb); (void) sb],
	[ac_cv_func_lstat=yes],
	[ac_cv_func_lstat=no])
	])
AC_MSG_RESULT($ac_cv_func_lstat )
if test "$ac_cv_func_lstat" = yes; then
	AC_DEFINE(HAVE_LSTAT,1,[Define to 1 if we have lstat])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_SIGACTION version: 3 updated: 2012/11/08 20:57:52
dnl -----------------
dnl Check if we have the sigaction function and related structures.
AC_DEFUN([CF_FUNC_SIGACTION],[
AC_CACHE_CHECK(for sigaction and structs,cf_cv_func_sigaction,[
AC_TRY_LINK([
#include <sys/types.h>
#include <signal.h>],
	[struct sigaction act;
	act.sa_handler = SIG_DFL;
#ifdef SA_RESTART
	act.sa_flags |= SA_RESTART;
#endif /* SA_RESTART */
	sigaction(1, &act, 0);
	],
	[cf_cv_func_sigaction=yes],
	[cf_cv_func_sigaction=no])
])
test "$cf_cv_func_sigaction" = yes && AC_DEFINE(HAVE_SIGACTION,1,[Define to 1 if we have sigaction])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_VASPRINTF version: 2 updated: 2019/12/31 10:27:03
dnl -----------------
dnl Check if vasprintf is available, and if it is (or can be) declared.
AC_DEFUN([CF_FUNC_VASPRINTF],[
AC_REQUIRE([CF_GNU_SOURCE])
AC_CHECK_FUNC(vasprintf,[
	AC_DEFINE(HAVE_VASPRINTF,1,[Define to 1 if we have vasprintf])
	AC_MSG_CHECKING(if vasprintf requires workaround)
	AC_TRY_COMPILE([
		#include <stdio.h>
		],[
		void *p = (void *)vasprintf; return (p != 0)
	],[
		AC_MSG_RESULT(no)
	],[
		AC_TRY_COMPILE([
			#ifndef _GNU_SOURCE
			#define _GNU_SOURCE 1
			#include <stdio.h>
			#endif
			],[
			void *p = (void *)vasprintf; return (p != 0)
		],[
			AC_MSG_RESULT(yes)
			CF_APPEND_TEXT(CPPFLAGS,-D_GNU_SOURCE)
		],[
			AC_MSG_RESULT(unknown)
		])
	])
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_WAIT version: 4 updated: 2021/01/02 09:31:20
dnl ------------
dnl Test for the presence of <sys/wait.h>, 'union wait', arg-type of 'wait()'
dnl and/or 'waitpid()'.
dnl
dnl Note that we cannot simply grep for 'union wait' in the wait.h file,
dnl because some Posix systems turn this on only when a BSD variable is
dnl defined.
dnl
dnl I don't use AC_HEADER_SYS_WAIT, because it defines HAVE_SYS_WAIT_H, which
dnl would conflict with an attempt to test that header directly.
dnl
AC_DEFUN([CF_FUNC_WAIT],
[
AC_REQUIRE([CF_UNION_WAIT])
if test "$cf_cv_type_unionwait" = yes; then

	AC_MSG_CHECKING(if union wait can be used as wait-arg)
	AC_CACHE_VAL(cf_cv_arg_union_wait,[
		AC_TRY_COMPILE($cf_wait_headers,
 			[union wait x; wait(&x)],
			[cf_cv_arg_union_wait=yes],
			[cf_cv_arg_union_wait=no])
		])
	AC_MSG_RESULT($cf_cv_arg_union_wait)
	test "$cf_cv_arg_union_wait" = yes && AC_DEFINE(WAIT_USES_UNION,1,[Define to 1 if wait() uses a union parameter])

	AC_MSG_CHECKING(if union wait can be used as waitpid-arg)
	AC_CACHE_VAL(cf_cv_arg_union_waitpid,[
		AC_TRY_COMPILE($cf_wait_headers,
 			[union wait x; waitpid(0, &x, 0)],
			[cf_cv_arg_union_waitpid=yes],
			[cf_cv_arg_union_waitpid=no])
		])
	AC_MSG_RESULT($cf_cv_arg_union_waitpid)
	test "$cf_cv_arg_union_waitpid" = yes && AC_DEFINE(WAITPID_USES_UNION,1,[Define to 1 if waitpid() uses a union parameter])

fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_ATTRIBUTES version: 24 updated: 2021/03/20 12:00:25
dnl -----------------
dnl Test for availability of useful gcc __attribute__ directives to quiet
dnl compiler warnings.  Though useful, not all are supported -- and contrary
dnl to documentation, unrecognized directives cause older compilers to barf.
AC_DEFUN([CF_GCC_ATTRIBUTES],
[AC_REQUIRE([AC_PROG_FGREP])dnl
AC_REQUIRE([CF_C11_NORETURN])dnl

if test "$GCC" = yes || test "$GXX" = yes
then
cat > conftest.i <<EOF
#ifndef GCC_PRINTF
#define GCC_PRINTF 0
#endif
#ifndef GCC_SCANF
#define GCC_SCANF 0
#endif
#ifndef GCC_NORETURN
#define GCC_NORETURN /* nothing */
#endif
#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif
EOF
if test "$GCC" = yes
then
	AC_CHECKING([for $CC __attribute__ directives])
cat > "conftest.$ac_ext" <<EOF
#line __oline__ "${as_me:-configure}"
#include "confdefs.h"
#include "conftest.h"
#include "conftest.i"
#if	GCC_PRINTF
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
#define GCC_PRINTFLIKE(fmt,var) /*nothing*/
#endif
#if	GCC_SCANF
#define GCC_SCANFLIKE(fmt,var)  __attribute__((format(scanf,fmt,var)))
#else
#define GCC_SCANFLIKE(fmt,var)  /*nothing*/
#endif
extern void wow(char *,...) GCC_SCANFLIKE(1,2);
extern GCC_NORETURN void oops(char *,...) GCC_PRINTFLIKE(1,2);
extern GCC_NORETURN void foo(void);
int main(int argc GCC_UNUSED, char *argv[[]] GCC_UNUSED) { (void)argc; (void)argv; return 0; }
EOF
	cf_printf_attribute=no
	cf_scanf_attribute=no
	for cf_attribute in scanf printf unused noreturn
	do
		CF_UPPER(cf_ATTRIBUTE,$cf_attribute)
		cf_directive="__attribute__(($cf_attribute))"
		echo "checking for $CC $cf_directive" 1>&AC_FD_CC

		case "$cf_attribute" in
		(printf)
			cf_printf_attribute=yes
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
			;;
		(scanf)
			cf_scanf_attribute=yes
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
			;;
		(*)
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE $cf_directive
EOF
			;;
		esac

		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... $cf_attribute)
			cat conftest.h >>confdefs.h
			case "$cf_attribute" in
			(noreturn)
				AC_DEFINE_UNQUOTED(GCC_NORETURN,$cf_directive,[Define to noreturn-attribute for gcc])
				;;
			(printf)
				cf_value='/* nothing */'
				if test "$cf_printf_attribute" != no ; then
					cf_value='__attribute__((format(printf,fmt,var)))'
					AC_DEFINE(GCC_PRINTF,1,[Define to 1 if the compiler supports gcc-like printf attribute.])
				fi
				AC_DEFINE_UNQUOTED(GCC_PRINTFLIKE(fmt,var),$cf_value,[Define to printf-attribute for gcc])
				;;
			(scanf)
				cf_value='/* nothing */'
				if test "$cf_scanf_attribute" != no ; then
					cf_value='__attribute__((format(scanf,fmt,var)))'
					AC_DEFINE(GCC_SCANF,1,[Define to 1 if the compiler supports gcc-like scanf attribute.])
				fi
				AC_DEFINE_UNQUOTED(GCC_SCANFLIKE(fmt,var),$cf_value,[Define to sscanf-attribute for gcc])
				;;
			(unused)
				AC_DEFINE_UNQUOTED(GCC_UNUSED,$cf_directive,[Define to unused-attribute for gcc])
				;;
			esac
		fi
	done
else
	${FGREP-fgrep} define conftest.i >>confdefs.h
fi
rm -rf ./conftest*
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_VERSION version: 8 updated: 2019/09/07 13:38:36
dnl --------------
dnl Find version of gcc, and (because icc/clang pretend to be gcc without being
dnl compatible), attempt to determine if icc/clang is actually used.
AC_DEFUN([CF_GCC_VERSION],[
AC_REQUIRE([AC_PROG_CC])
GCC_VERSION=none
if test "$GCC" = yes ; then
	AC_MSG_CHECKING(version of $CC)
	GCC_VERSION="`${CC} --version 2>/dev/null | sed -e '2,$d' -e 's/^.*(GCC[[^)]]*) //' -e 's/^.*(Debian[[^)]]*) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GCC_VERSION" && GCC_VERSION=unknown
	AC_MSG_RESULT($GCC_VERSION)
fi
CF_INTEL_COMPILER(GCC,INTEL_COMPILER,CFLAGS)
CF_CLANG_COMPILER(GCC,CLANG_COMPILER,CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_WARNINGS version: 41 updated: 2021/01/01 16:53:59
dnl ---------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Winline (usually not worthwhile)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally).  This
dnl		is enabled for ncurses using "--enable-const".
dnl	-pedantic
dnl
dnl Parameter:
dnl	$1 is an optional list of gcc warning flags that a particular
dnl		application might want to use, e.g., "no-unused" for
dnl		-Wno-unused
dnl Special:
dnl	If $with_ext_const is "yes", add a check for -Wwrite-strings
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[
AC_REQUIRE([CF_GCC_VERSION])
if test "x$have_x" = xyes; then CF_CONST_X_STRING fi
cat > "conftest.$ac_ext" <<EOF
#line __oline__ "${as_me:-configure}"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF
if test "$INTEL_COMPILER" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #1683: explicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1684: conversion from pointer to same-sized integral type (potential portability problem)
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #279: controlling expression is constant

	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="$EXTRA_CFLAGS -Wall"
	for cf_opt in \
		wd1419 \
		wd1683 \
		wd1684 \
		wd193 \
		wd593 \
		wd279 \
		wd810 \
		wd869 \
		wd981
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
elif test "$GCC" = yes && test "$GCC_VERSION" != "unknown"
then
	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	cf_gcc_warnings="Wignored-qualifiers Wlogical-op Wvarargs"
	test "x$CLANG_COMPILER" = xyes && cf_gcc_warnings=
	for cf_opt in W Wall \
		Wbad-function-cast \
		Wcast-align \
		Wcast-qual \
		Wdeclaration-after-statement \
		Wextra \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes \
		Wundef Wno-inline $cf_gcc_warnings $cf_warn_CONST $1
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			case "$cf_opt" in
			(Winline)
				case "$GCC_VERSION" in
				([[34]].*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			(Wpointer-arith)
				case "$GCC_VERSION" in
				([[12]].*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			esac
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
fi
rm -rf ./conftest*

AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GNUTLS version: 27 updated: 2021/01/02 09:31:20
dnl ---------
dnl Check for gnutls library (TLS "is" SSL)
dnl $1 = the [optional] directory in which the library may be found
dnl $2 = the [optional] stub file to provide OpenSSL compatibility
AC_DEFUN([CF_GNUTLS],[
	AC_REQUIRE([CF_PKG_CONFIG])
	cf_cv_have_gnutls=no
	cf_cv_pkg_config_ssl=no

	CF_ADD_OPTIONAL_PATH($1, [ssl library])

	cf_pkg_gnutls=none
	if test "x$PKG_CONFIG" != xnone; then
		cf_pkg_gnutls=ifelse([$2],,gnutls-openssl,gnutls)
		case "$1" in
		(no)
			;;
		(yes) # if no explicit directory given, try pkg-config
			CF_VERBOSE(checking pkg-config for $cf_pkg_gnutls)
			if "$PKG_CONFIG" --exists $cf_pkg_gnutls ; then
				CF_VERBOSE(... found $cf_pkg_gnutls in pkg-config)
				cf_cv_have_gnutls=yes
				cf_cv_pkg_config_ssl=yes

				cf_cflags_ssl=`$PKG_CONFIG --cflags $cf_pkg_gnutls`
				cf_libs_ssl=`$PKG_CONFIG --libs $cf_pkg_gnutls`

				if test -n "$cf_cflags_ssl" ; then
					case "$cf_cflags_ssl" in
					(*-I*)
						cf_cv_header_path_gnutls=`echo "$cf_cflags_ssl" | sed -e 's/^.*-I//' -e 's/ .*//'`
						;;
					(*)
						cf_cv_header_path_gnutls=/usr/include
						;;
					esac
					if test -d "$cf_cv_header_path_gnutls/gnutls" ; then
						cf_cv_header_path_gnutls=$cf_cv_header_path_gnutls/gnutls
					fi
					CF_ADD_CFLAGS($cf_cflags_ssl)
				fi

				if test -n "$cf_libs_ssl" ; then
					case "x$cf_libs_ssl" in
					(*-L*)
						cf_cv_library_path_gnutls=`echo "$cf_libs_ssl" | sed -e 's/^.*-L//' -e 's/ .*//'`
						;;
					(*)
						cf_cv_library_path_gnutls=/usr/lib
						;;
					esac
					CF_VERBOSE(adding $cf_libs_ssl to LIBS)
					CF_ADD_LIBS($cf_libs_ssl)
				fi
			else
				CF_VERBOSE(... did not find $cf_pkg_gnutls in pkg-config)
				cf_pkg_gnutls=none
			fi
			;;
		esac
	fi

	ifelse([$2],,
		[AC_DEFINE(USE_GNUTLS_INCL,1,[Define to 1 if we should include gnutls headers])],
		[AC_DEFINE(USE_GNUTLS_FUNCS,1,[Define to 1 if we should use gnutls functions])])

	if test "$cf_cv_have_gnutls" = no ; then
		cf_gnutls_CPPFLAGS=$CPPFLAGS

		CF_FIND_LINKAGE(CF__SSL_HEAD,
			CF__SSL_BODY,
			gnutls,
			cf_cv_have_gnutls=yes,
			cf_cv_have_gnutls=no,
			,
			ifelse([$2],,[-lgnutls-openssl]))

		CPPFLAGS=$cf_gnutls_CPPFLAGS
	fi

	if test "$cf_cv_have_gnutls" = yes ; then
		if test -n "$cf_cv_header_path_gnutls" ; then
			AC_DEFINE(USE_SSL)
			case "$cf_cv_header_path_gnutls" in
			(/usr/include/gnutls)
				;;
			(*)
				CF_ADD_INCDIR($cf_cv_header_path_gnutls)
				;;
			esac
		fi
		if test -n "$cf_cv_library_path_gnutls" ; then
			CF_ADD_LIBDIR($cf_cv_library_path_gnutls)
		fi
		CF_ADD_LIBS(-lgnutls)
		AC_CHECK_FUNCS(gnutls_protocol_set_priority)
		AC_CHECK_FUNC(gnutls_rnd,
				[AC_DEFINE(HAVE_GNUTLS_RND,1,[Define to 1 if we have gnutls_rnd])],
				[CF_ADD_LIBS(-lgcrypt)])

		ifelse([$2],,
			[if test "$cf_pkg_gnutls" = none ; then
				AC_CHECK_LIB(gnutls-openssl,SSL_connect,
					[CF_ADD_LIBS(-lgnutls-openssl)],
					[AC_CHECK_LIB(gnutls-extra,SSL_connect,
					[CF_ADD_LIBS(-lgnutls-extra)],
					[AC_MSG_ERROR(cannot find gnutls openssl functions)])])
			fi],[EXTRA_OBJS="$EXTRA_OBJS $2"])

		CF_CHECK_SSL_X509
	fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GNU_SOURCE version: 10 updated: 2018/12/10 20:09:41
dnl -------------
dnl Check if we must define _GNU_SOURCE to get a reasonable value for
dnl _XOPEN_SOURCE, upon which many POSIX definitions depend.  This is a defect
dnl (or misfeature) of glibc2, which breaks portability of many applications,
dnl since it is interwoven with GNU extensions.
dnl
dnl Well, yes we could work around it...
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
AC_DEFUN([CF_GNU_SOURCE],
[
cf_gnu_xopen_source=ifelse($1,,500,$1)

AC_CACHE_CHECK(if this is the GNU C library,cf_cv_gnu_library,[
AC_TRY_COMPILE([#include <sys/types.h>],[
	#if __GLIBC__ > 0 && __GLIBC_MINOR__ >= 0
		return 0;
	#elif __NEWLIB__ > 0 && __NEWLIB_MINOR__ >= 0
		return 0;
	#else
	#	error not GNU C library
	#endif],
	[cf_cv_gnu_library=yes],
	[cf_cv_gnu_library=no])
])

if test x$cf_cv_gnu_library = xyes; then

	# With glibc 2.19 (13 years after this check was begun), _DEFAULT_SOURCE
	# was changed to help a little.  newlib incorporated the change about 4
	# years later.
	AC_CACHE_CHECK(if _DEFAULT_SOURCE can be used as a basis,cf_cv_gnu_library_219,[
		cf_save="$CPPFLAGS"
		CF_APPEND_TEXT(CPPFLAGS,-D_DEFAULT_SOURCE)
		AC_TRY_COMPILE([#include <sys/types.h>],[
			#if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 19) || (__GLIBC__ > 2)
				return 0;
			#elif (__NEWLIB__ == 2 && __NEWLIB_MINOR__ >= 4) || (__GLIBC__ > 3)
				return 0;
			#else
			#	error GNU C library __GLIBC__.__GLIBC_MINOR__ is too old
			#endif],
			[cf_cv_gnu_library_219=yes],
			[cf_cv_gnu_library_219=no])
		CPPFLAGS="$cf_save"
	])

	if test "x$cf_cv_gnu_library_219" = xyes; then
		cf_save="$CPPFLAGS"
		AC_CACHE_CHECK(if _XOPEN_SOURCE=$cf_gnu_xopen_source works with _DEFAULT_SOURCE,cf_cv_gnu_dftsrc_219,[
			CF_ADD_CFLAGS(-D_DEFAULT_SOURCE -D_XOPEN_SOURCE=$cf_gnu_xopen_source)
			AC_TRY_COMPILE([
				#include <limits.h>
				#include <sys/types.h>
				],[
				#if (_XOPEN_SOURCE >= $cf_gnu_xopen_source) && (MB_LEN_MAX > 1)
					return 0;
				#else
				#	error GNU C library is too old
				#endif],
				[cf_cv_gnu_dftsrc_219=yes],
				[cf_cv_gnu_dftsrc_219=no])
			])
		test "x$cf_cv_gnu_dftsrc_219" = "xyes" || CPPFLAGS="$cf_save"
	else
		cf_cv_gnu_dftsrc_219=maybe
	fi

	if test "x$cf_cv_gnu_dftsrc_219" != xyes; then

		AC_CACHE_CHECK(if we must define _GNU_SOURCE,cf_cv_gnu_source,[
		AC_TRY_COMPILE([#include <sys/types.h>],[
			#ifndef _XOPEN_SOURCE
			#error	expected _XOPEN_SOURCE to be defined
			#endif],
			[cf_cv_gnu_source=no],
			[cf_save="$CPPFLAGS"
			 CF_ADD_CFLAGS(-D_GNU_SOURCE)
			 AC_TRY_COMPILE([#include <sys/types.h>],[
				#ifdef _XOPEN_SOURCE
				#error	expected _XOPEN_SOURCE to be undefined
				#endif],
				[cf_cv_gnu_source=no],
				[cf_cv_gnu_source=yes])
			CPPFLAGS="$cf_save"
			])
		])

		if test "$cf_cv_gnu_source" = yes
		then
		AC_CACHE_CHECK(if we should also define _DEFAULT_SOURCE,cf_cv_default_source,[
			CF_APPEND_TEXT(CPPFLAGS,-D_GNU_SOURCE)
			AC_TRY_COMPILE([#include <sys/types.h>],[
				#ifdef _DEFAULT_SOURCE
				#error	expected _DEFAULT_SOURCE to be undefined
				#endif],
				[cf_cv_default_source=no],
				[cf_cv_default_source=yes])
			])
			if test "$cf_cv_default_source" = yes
			then
				CF_APPEND_TEXT(CPPFLAGS,-D_DEFAULT_SOURCE)
			fi
		fi
	fi

fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HEADER_PATH version: 15 updated: 2021/01/01 13:31:04
dnl --------------
dnl Construct a search-list of directories for a nonstandard header-file
dnl
dnl Parameters
dnl	$1 = the variable to return as result
dnl	$2 = the package name
AC_DEFUN([CF_HEADER_PATH],
[
$1=

# collect the current set of include-directories from compiler flags
cf_header_path_list=""
if test -n "${CFLAGS}${CPPFLAGS}" ; then
	for cf_header_path in $CPPFLAGS $CFLAGS
	do
		case "$cf_header_path" in
		(-I*)
			cf_header_path=`echo ".$cf_header_path" |sed -e 's/^...//' -e 's,/include$,,'`
			CF_ADD_SUBDIR_PATH($1,$2,include,$cf_header_path,NONE)
			cf_header_path_list="$cf_header_path_list [$]$1"
			;;
		esac
	done
fi

# add the variations for the package we are looking for
CF_SUBDIR_PATH($1,$2,include)

test "$includedir" != NONE && \
test "$includedir" != "/usr/include" && \
test -d "$includedir" && {
	test -d "$includedir" &&    $1="[$]$1 $includedir"
	test -d "$includedir/$2" && $1="[$]$1 $includedir/$2"
}

test "$oldincludedir" != NONE && \
test "$oldincludedir" != "/usr/include" && \
test -d "$oldincludedir" && {
	test -d "$oldincludedir"    && $1="[$]$1 $oldincludedir"
	test -d "$oldincludedir/$2" && $1="[$]$1 $oldincludedir/$2"
}

$1="[$]$1 $cf_header_path_list"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HELP_MESSAGE version: 4 updated: 2019/12/31 08:53:54
dnl ---------------
dnl Insert text into the help-message, for readability, from AC_ARG_WITH.
AC_DEFUN([CF_HELP_MESSAGE],
[CF_ACVERSION_CHECK(2.53,[],[
AC_DIVERT_HELP($1)])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INET_ADDR version: 7 updated: 2013/10/08 17:47:05
dnl ------------
dnl For Lynx, check if the libraries we have found give us inet_aton, or
dnl inet_addr.  If not, try to find the latter function with -lbind or
dnl -lresolv, and put that on the end of the libraries, i.e., after the network
dnl libraries.
dnl
dnl FIXME: the inner cases will probably need work on the header files.
AC_DEFUN([CF_INET_ADDR],[
AC_CACHE_CHECK(for inet_aton function,cf_cv_have_inet_aton,[
AC_TRY_LINK(CF__INET_HEAD,[inet_aton(0, (struct in_addr *)0)],
    [cf_cv_have_inet_aton=yes],
    [cf_cv_have_inet_aton=no])])
if test "$cf_cv_have_inet_aton" = yes ; then
    AC_DEFINE(HAVE_INET_ATON,1,[Define to 1 if we have inet_aton])
else
    AC_CACHE_CHECK(for inet_addr function,cf_cv_have_inet_addr,[
    AC_TRY_LINK(CF__INET_HEAD,[inet_addr(0)],
	[cf_cv_have_inet_addr=yes],
	[cf_cv_have_inet_addr=no])])
    if test "$cf_cv_have_inet_addr" = no ; then
	AC_CACHE_CHECK(for library with inet_addr,cf_cv_lib_inet_addr,[
	    cf_save_LIBS="$LIBS"
	    for cf_inetlib in -lbind -lresolv
	    do
		LIBS="$cf_save_LIBS $cf_inetlib"
		AC_TRY_LINK([#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
		],[inet_addr(0)],
		    [cf_cv_lib_inet_addr=$cf_inetlib],
		    [cf_cv_lib_inet_addr=no])
		LIBS="$cf_save_LIBS"
		test "$cf_cv_lib_inet_addr" != no && break
	    done
	])
	if test "$cf_cv_lib_inet_addr" != no ; then
	    CF_ADD_LIBS($cf_cv_lib_inet_addr)
	else
	    AC_MSG_WARN(Unable to find library for inet_addr function)
	fi
    fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 8 updated: 2021/01/01 16:53:59
dnl -----------------
dnl Check if the given compiler is really the Intel compiler for Linux.  It
dnl tries to imitate gcc, but does not return an error when it finds a mismatch
dnl between prototypes, e.g., as exercised by CF_MISSING_CHECK.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = INTEL_COMPILER (default) or INTEL_CPLUSPLUS
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_INTEL_COMPILER],[
AC_REQUIRE([AC_CANONICAL_HOST])
ifelse([$2],,INTEL_COMPILER,[$2])=no

if test "$ifelse([$1],,[$1],GCC)" = yes ; then
	case "$host_os" in
	(linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel ifelse([$1],GXX,C++,C) compiler)
		cf_save_CFLAGS="$ifelse([$3],,CFLAGS,[$3])"
		ifelse([$3],,CFLAGS,[$3])="$ifelse([$3],,CFLAGS,[$3]) -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
make an error
#endif
],[ifelse([$2],,INTEL_COMPILER,[$2])=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147"
],[])
		ifelse([$3],,CFLAGS,[$3])="$cf_save_CFLAGS"
		AC_MSG_RESULT($ifelse([$2],,INTEL_COMPILER,[$2]))
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LARGEFILE version: 12 updated: 2020/03/19 20:23:48
dnl ------------
dnl Add checks for large file support.
AC_DEFUN([CF_LARGEFILE],[
ifdef([AC_FUNC_FSEEKO],[
	AC_SYS_LARGEFILE
	if test "$enable_largefile" != no ; then
	AC_FUNC_FSEEKO

	# Normally we would collect these definitions in the config.h,
	# but (like _XOPEN_SOURCE), some environments rely on having these
	# defined before any of the system headers are included.  Another
	# case comes up with C++, e.g., on AIX the compiler compiles the
	# header files by themselves before looking at the body files it is
	# told to compile.  For ncurses, those header files do not include
	# the config.h
	if test "$ac_cv_sys_large_files" != no
	then
		CF_APPEND_TEXT(CPPFLAGS,-D_LARGE_FILES)
	fi
	if test "$ac_cv_sys_largefile_source" != no
	then
		CF_APPEND_TEXT(CPPFLAGS,-D_LARGEFILE_SOURCE)
	fi
	if test "$ac_cv_sys_file_offset_bits" != no
	then
		CF_APPEND_TEXT(CPPFLAGS,-D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits)
	fi

	AC_CACHE_CHECK(whether to use struct dirent64, cf_cv_struct_dirent64,[
		AC_TRY_COMPILE([
#pragma GCC diagnostic error "-Wincompatible-pointer-types"
#include <sys/types.h>
#include <dirent.h>
		],[
		/* if transitional largefile support is setup, this is true */
		extern struct dirent64 * readdir(DIR *);
		struct dirent64 *x = readdir((DIR *)0);
		struct dirent *y = readdir((DIR *)0);
		int z = x - y;
		(void)z;
		],
		[cf_cv_struct_dirent64=yes],
		[cf_cv_struct_dirent64=no])
	])
	test "$cf_cv_struct_dirent64" = yes && AC_DEFINE(HAVE_STRUCT_DIRENT64,1,[Define to 1 if we have struct dirent64])
	fi
])
])
dnl ---------------------------------------------------------------------------
dnl CF_LASTLOG version: 7 updated: 2021/01/02 09:31:20
dnl ----------
dnl Check for header defining _PATH_LASTLOG, or failing that, see if the lastlog
dnl file exists.
AC_DEFUN([CF_LASTLOG],
[
AC_CHECK_HEADERS(lastlog.h paths.h)
AC_CACHE_CHECK(for lastlog path,cf_cv_path_lastlog,[
AC_TRY_COMPILE([
#include <sys/types.h>
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#else
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#endif],[char *path = _PATH_LASTLOG; (void)path],
	[cf_cv_path_lastlog="_PATH_LASTLOG"],
	[if test -f /usr/adm/lastlog ; then
	 	cf_cv_path_lastlog=/usr/adm/lastlog
	else
		cf_cv_path_lastlog=no
	fi])
])
test "$cf_cv_path_lastlog" != no && AC_DEFINE(USE_LASTLOG,1,[Define to 1 if we can define lastlog pathname])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LD_RPATH_OPT version: 9 updated: 2021/01/01 13:31:04
dnl ---------------
dnl For the given system and compiler, find the compiler flags to pass to the
dnl loader to use the "rpath" feature.
AC_DEFUN([CF_LD_RPATH_OPT],
[
AC_REQUIRE([CF_CHECK_CACHE])

LD_RPATH_OPT=
if test "x$cf_cv_enable_rpath" != xno
then
	AC_MSG_CHECKING(for an rpath option)
	case "$cf_cv_system_name" in
	(irix*)
		if test "$GCC" = yes; then
			LD_RPATH_OPT="-Wl,-rpath,"
		else
			LD_RPATH_OPT="-rpath "
		fi
		;;
	(linux*|gnu*|k*bsd*-gnu|freebsd*)
		LD_RPATH_OPT="-Wl,-rpath,"
		;;
	(openbsd[[2-9]].*|mirbsd*)
		LD_RPATH_OPT="-Wl,-rpath,"
		;;
	(dragonfly*)
		LD_RPATH_OPT="-rpath "
		;;
	(netbsd*)
		LD_RPATH_OPT="-Wl,-rpath,"
		;;
	(osf*|mls+*)
		LD_RPATH_OPT="-rpath "
		;;
	(solaris2*)
		LD_RPATH_OPT="-R"
		;;
	(*)
		;;
	esac
	AC_MSG_RESULT($LD_RPATH_OPT)

	case "x$LD_RPATH_OPT" in
	(x-R*)
		AC_MSG_CHECKING(if we need a space after rpath option)
		cf_save_LIBS="$LIBS"
		CF_ADD_LIBS(${LD_RPATH_OPT}$libdir)
		AC_TRY_LINK(, , cf_rpath_space=no, cf_rpath_space=yes)
		LIBS="$cf_save_LIBS"
		AC_MSG_RESULT($cf_rpath_space)
		test "$cf_rpath_space" = yes && LD_RPATH_OPT="$LD_RPATH_OPT "
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LIBRARY_PATH version: 11 updated: 2021/01/01 13:31:04
dnl ---------------
dnl Construct a search-list of directories for a nonstandard library-file
dnl
dnl Parameters
dnl	$1 = the variable to return as result
dnl	$2 = the package name
AC_DEFUN([CF_LIBRARY_PATH],
[
$1=
cf_library_path_list=""
if test -n "${LDFLAGS}${LIBS}" ; then
	for cf_library_path in $LDFLAGS $LIBS
	do
		case "$cf_library_path" in
		(-L*)
			cf_library_path=`echo ".$cf_library_path" |sed -e 's/^...//' -e 's,/lib$,,'`
			CF_ADD_SUBDIR_PATH($1,$2,lib,$cf_library_path,NONE)
			cf_library_path_list="$cf_library_path_list [$]$1"
			;;
		esac
	done
fi

CF_SUBDIR_PATH($1,$2,lib)

$1="$cf_library_path_list [$]$1"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LOCALE version: 6 updated: 2021/01/02 09:31:20
dnl ---------
dnl Check if we have setlocale() and its header, <locale.h>
dnl The optional parameter $1 tells what to do if we do have locale support.
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
test "$cf_cv_locale" = yes && { ifelse($1,,AC_DEFINE(LOCALE,1,[Define to 1 if we have locale support]),[$1]) }
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKEFLAGS version: 20 updated: 2021/01/03 19:29:49
dnl ------------
dnl Some 'make' programs support ${MAKEFLAGS}, some ${MFLAGS}, to pass 'make'
dnl options to lower-levels.  It's very useful for "make -n" -- if we have it.
dnl (GNU 'make' does both, something POSIX 'make', which happens to make the
dnl ${MAKEFLAGS} variable incompatible because it adds the assignments :-)
AC_DEFUN([CF_MAKEFLAGS],
[AC_REQUIRE([AC_PROG_FGREP])dnl

AC_CACHE_CHECK(for makeflags variable, cf_cv_makeflags,[
	cf_cv_makeflags=''
	for cf_option in '-${MAKEFLAGS}' '${MFLAGS}'
	do
		cat >cf_makeflags.tmp <<CF_EOF
SHELL = $SHELL
all :
	@ echo '.$cf_option'
CF_EOF
		cf_result=`${MAKE:-make} -k -f cf_makeflags.tmp 2>/dev/null | ${FGREP-fgrep} -v "ing directory" | sed -e 's,[[ 	]]*$,,'`
		case "$cf_result" in
		(.*k|.*kw)
			cf_result="`${MAKE:-make} -k -f cf_makeflags.tmp CC=cc 2>/dev/null`"
			case "$cf_result" in
			(.*CC=*)	cf_cv_makeflags=
				;;
			(*)	cf_cv_makeflags=$cf_option
				;;
			esac
			break
			;;
		(.-)
			;;
		(*)
			CF_MSG_LOG(given option \"$cf_option\", no match \"$cf_result\")
			;;
		esac
	done
	rm -f cf_makeflags.tmp
])

AC_SUBST(cf_cv_makeflags)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKE_PHONY version: 3 updated: 2021/01/08 16:08:21
dnl -------------
dnl Check if the make-program handles a ".PHONY" target, e.g,. a target which
dnl acts as a placeholder.
dnl
dnl The ".PHONY" feature was proposed in 2011 here
dnl     https://www.austingroupbugs.net/view.php?id=523
dnl and is scheduled for release in P1003.1 Issue 8 (late 2022).
dnl
dnl This is not supported by SVr4 make (or SunOS 4, 4.3SD, etc), but works with
dnl a few others (i.e., GNU make and the non-POSIX "BSD" make):
dnl
dnl + This is a GNU make feature (since April 1988, but in turn from binutils,
dnl   date unspecified).
dnl
dnl + It was adopted in NetBSD make in June 1995.
dnl
dnl + The other BSD make programs are derived from the NetBSD make (and for
dnl   that reason are not actually different "implementations").
dnl
dnl + Some features of NetBSD make were actually adapted from pmake, which
dnl   began as a modified GNU make starting in 1993.
dnl
dnl + Version 3.8 of the dmake program in January 1992 also implemented this
dnl   GNU make extension, but is less well known than the BSD make.
AC_DEFUN([CF_MAKE_PHONY],[
AC_CACHE_CHECK(for \".PHONY\" make-support, cf_cv_make_PHONY,[
	rm -rf conftest*
	(
		mkdir conftest || exit 1
		cd conftest
		cat >makefile <<'CF_EOF'
.PHONY: always
DATA=0
always:	always.out
	@echo "** making [$]@ [$](DATA)"
once: once.out
	@echo "** making [$]@ [$](DATA)"
always.out:
	@echo "** making [$]@ [$](DATA)"
	echo [$](DATA) > [$]@
once.out:
	@echo "** making [$]@ [$](DATA)"
	echo [$](DATA) > [$]@
CF_EOF
		for cf_data in 1 2 3
		do
			${MAKE:-make} always DATA=$cf_data
			${MAKE:-make} once   DATA=$cf_data
			${MAKE:-make} -t always once
			if test -f always ; then
				echo "no (case 1)" > ../conftest.tmp
			elif test ! -f always.out ; then
				echo "no (case 2)" > ../conftest.tmp
			elif test ! -f once.out ; then
				echo "no (case 3)" > ../conftest.tmp
			elif ! cmp -s always.out once.out ; then
				echo "no (case 4)" > ../conftest.tmp
				diff always.out once.out
			else
				cf_check="`cat always.out`"
				if test "x$cf_check" != "x$cf_data" ; then
					echo "no (case 5)" > ../conftest.tmp
				else
					echo yes > ../conftest.tmp
					rm -f ./*.out
					continue
				fi
			fi
			break
		done
	) >&AC_FD_CC 2>&1
	cf_cv_make_PHONY="`cat conftest.tmp`"
	rm -rf conftest*
])
MAKE_NO_PHONY="#"
MAKE_PHONY="#"
test "x$cf_cv_make_PHONY" = xyes && MAKE_PHONY=
test "x$cf_cv_make_PHONY" != xyes && MAKE_NO_PHONY=
AC_SUBST(MAKE_NO_PHONY)
AC_SUBST(MAKE_PHONY)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKE_TAGS version: 6 updated: 2010/10/23 15:52:32
dnl ------------
dnl Generate tags/TAGS targets for makefiles.  Do not generate TAGS if we have
dnl a monocase filesystem.
AC_DEFUN([CF_MAKE_TAGS],[
AC_REQUIRE([CF_MIXEDCASE_FILENAMES])

AC_CHECK_PROGS(CTAGS, exctags ctags)
AC_CHECK_PROGS(ETAGS, exetags etags)

AC_CHECK_PROG(MAKE_LOWER_TAGS, ${CTAGS:-ctags}, yes, no)

if test "$cf_cv_mixedcase" = yes ; then
	AC_CHECK_PROG(MAKE_UPPER_TAGS, ${ETAGS:-etags}, yes, no)
else
	MAKE_UPPER_TAGS=no
fi

if test "$MAKE_UPPER_TAGS" = yes ; then
	MAKE_UPPER_TAGS=
else
	MAKE_UPPER_TAGS="#"
fi

if test "$MAKE_LOWER_TAGS" = yes ; then
	MAKE_LOWER_TAGS=
else
	MAKE_LOWER_TAGS="#"
fi

AC_SUBST(CTAGS)
AC_SUBST(ETAGS)

AC_SUBST(MAKE_UPPER_TAGS)
AC_SUBST(MAKE_LOWER_TAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MATH_LIB version: 10 updated: 2020/12/31 18:40:20
dnl -----------
dnl Checks for libraries.  At least one UNIX system, Apple Macintosh
dnl Rhapsody 5.5, does not have -lm.  We cannot use the simpler
dnl AC_CHECK_LIB(m,sin), because that fails for C++.
AC_DEFUN([CF_MATH_LIB],
[
AC_CACHE_CHECK(if -lm needed for math functions,
	cf_cv_need_libm,[
	AC_TRY_LINK([
	#include <stdio.h>
	#include <stdlib.h>
	#include <math.h>
	],
	[double x = rand(); printf("result = %g\\n", ]ifelse([$2],,sin(x),$2)[)],
	[cf_cv_need_libm=no],
	[cf_cv_need_libm=yes])])
if test "$cf_cv_need_libm" = yes
then
ifelse($1,,[
	CF_ADD_LIB(m)
],[$1=-lm])
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_MERGE_EXTRA_CFLAGS version: 2 updated: 2021/01/02 09:31:20
dnl ---------------------
dnl CF_FIX_WARNINGS moves problematic flags into EXTRA_CFLAGS, but some scripts
dnl may depend on being able to override that variable at build-time.  Move it
dnl all back.
define([CF_MERGE_EXTRA_CFLAGS],[
if test "$GCC" = yes || test "$GXX" = yes
then
	CF_APPEND_TEXT(CFLAGS,$EXTRA_CFLAGS)
	EXTRA_CFLAGS=
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_MIXEDCASE_FILENAMES version: 9 updated: 2021/01/01 16:53:59
dnl ----------------------
dnl Check if the file-system supports mixed-case filenames.  If we're able to
dnl create a lowercase name and see it as uppercase, it doesn't support that.
AC_DEFUN([CF_MIXEDCASE_FILENAMES],
[
AC_CACHE_CHECK(if filesystem supports mixed-case filenames,cf_cv_mixedcase,[
if test "$cross_compiling" = yes ; then
	case "$target_alias" in
	(*-os2-emx*|*-msdosdjgpp*|*-cygwin*|*-msys*|*-mingw*|*-uwin*|darwin*)
		cf_cv_mixedcase=no
		;;
	(*)
		cf_cv_mixedcase=yes
		;;
	esac
else
	rm -f conftest CONFTEST
	echo test >conftest
	if test -f CONFTEST ; then
		cf_cv_mixedcase=no
	else
		cf_cv_mixedcase=yes
	fi
	rm -f conftest CONFTEST
fi
])
test "$cf_cv_mixedcase" = yes && AC_DEFINE(MIXEDCASE_FILENAMES,1,[Define to 1 if filesystem supports mixed-case filenames.])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MSG_LOG version: 5 updated: 2010/10/23 15:52:32
dnl ----------
dnl Write a debug message to config.log, along with the line number in the
dnl configure script.
AC_DEFUN([CF_MSG_LOG],[
echo "${as_me:-configure}:__oline__: testing $* ..." 1>&AC_FD_CC
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_BROKEN version: 8 updated: 2012/11/08 20:57:52
dnl -----------------
dnl Check for pre-1.9.9g ncurses (among other problems, the most obvious is
dnl that color combinations don't work).
AC_DEFUN([CF_NCURSES_BROKEN],
[
AC_REQUIRE([CF_NCURSES_VERSION])
if test "$cf_cv_ncurses_version" != no ; then
AC_MSG_CHECKING(for obsolete/broken version of ncurses)
AC_CACHE_VAL(cf_cv_ncurses_broken,[
AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header:-curses.h}>],[
#if defined(NCURSES_VERSION) && defined(wgetbkgd)
	make an error
#else
	int x = 1
#endif
],
	[cf_cv_ncurses_broken=no],
	[cf_cv_ncurses_broken=yes])
])
AC_MSG_RESULT($cf_cv_ncurses_broken)
if test "$cf_cv_ncurses_broken" = yes ; then
	AC_MSG_WARN(hmm... you should get an up-to-date version of ncurses)
	AC_DEFINE(NCURSES_BROKEN,1,[Define to 1 if you have an obsolete version of ncurses])
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_CC_CHECK version: 5 updated: 2020/12/31 20:19:42
dnl -------------------
dnl Check if we can compile with ncurses' header file
dnl $1 is the cache variable to set
dnl $2 is the header-file to include
dnl $3 is the root name (ncurses or ncursesw)
AC_DEFUN([CF_NCURSES_CC_CHECK],[
	AC_TRY_COMPILE([
]ifelse($3,ncursesw,[
#define _XOPEN_SOURCE_EXTENDED
#undef  HAVE_LIBUTF8_H	/* in case we used CF_UTF8_LIB */
#define HAVE_LIBUTF8_H	/* to force ncurses' header file to use cchar_t */
])[
#include <$2>],[
#ifdef NCURSES_VERSION
]ifelse($3,ncursesw,[
#ifndef WACS_BSSB
	make an error
#endif
])[
printf("%s\\n", NCURSES_VERSION);
#else
#ifdef __NCURSES_H
printf("old\\n");
#else
	make an error
#endif
#endif
	]
	,[$1=$2]
	,[$1=no])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_CONFIG version: 27 updated: 2021/05/19 19:35:25
dnl -----------------
dnl Tie together the configure-script macros for ncurses, preferring these in
dnl order:
dnl a) ".pc" files for pkg-config, using $NCURSES_CONFIG_PKG
dnl b) the "-config" script from ncurses, using $NCURSES_CONFIG
dnl c) just plain libraries
dnl
dnl $1 is the root library name (default: "ncurses")
AC_DEFUN([CF_NCURSES_CONFIG],[
AC_REQUIRE([CF_PKG_CONFIG])
cf_ncuconfig_root=ifelse($1,,ncurses,$1)
cf_have_ncuconfig=no

if test "x${PKG_CONFIG:=none}" != xnone; then
	AC_MSG_CHECKING(pkg-config for $cf_ncuconfig_root)
	if "$PKG_CONFIG" --exists $cf_ncuconfig_root ; then
		AC_MSG_RESULT(yes)

		AC_MSG_CHECKING(if the $cf_ncuconfig_root package files work)
		cf_have_ncuconfig=unknown

		cf_save_CFLAGS="$CFLAGS"
		cf_save_CPPFLAGS="$CPPFLAGS"
		cf_save_LIBS="$LIBS"

		cf_pkg_cflags="`$PKG_CONFIG --cflags $cf_ncuconfig_root`"
		cf_pkg_libs="`$PKG_CONFIG --libs $cf_ncuconfig_root`"

		# while -W for passing linker flags is prevalent, it is not "standard".
		# At least one wrapper for c89/c99 (in Apple's xcode) has its own
		# incompatible _and_ non-standard -W option which gives an error.  Work
		# around that pitfall.
		case "x${CC}@@${cf_pkg_libs}@${cf_pkg_cflags}" in
		(x*c[[89]]9@@*-W*)
			CF_ADD_CFLAGS($cf_pkg_cflags)
			CF_ADD_LIBS($cf_pkg_libs)

			AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
				[initscr(); mousemask(0,0); tigetstr((char *)0);],
				[AC_TRY_RUN([#include <${cf_cv_ncurses_header:-curses.h}>
					int main(void)
					{ const char *xx = curses_version(); return (xx == 0); }],
					[cf_test_ncuconfig=yes],
					[cf_test_ncuconfig=no],
					[cf_test_ncuconfig=maybe])],
				[cf_test_ncuconfig=no])

			CFLAGS="$cf_save_CFLAGS"
			CPPFLAGS="$cf_save_CPPFLAGS"
			LIBS="$cf_save_LIBS"

			if test "x$cf_test_ncuconfig" != xyes; then
				cf_temp=`echo "x$cf_pkg_cflags" | sed -e s/^x// -e 's/-W[[^ 	]]*//g'`
				cf_pkg_cflags="$cf_temp"
				cf_temp=`echo "x$cf_pkg_libs" | sed -e s/^x// -e 's/-W[[^ 	]]*//g'`
				cf_pkg_libs="$cf_temp"
			fi
			;;
		esac

		CF_ADD_CFLAGS($cf_pkg_cflags)
		CF_ADD_LIBS($cf_pkg_libs)

		AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
			[initscr(); mousemask(0,0); tigetstr((char *)0);],
			[AC_TRY_RUN([#include <${cf_cv_ncurses_header:-curses.h}>
				int main(void)
				{ const char *xx = curses_version(); return (xx == 0); }],
				[cf_have_ncuconfig=yes],
				[cf_have_ncuconfig=no],
				[cf_have_ncuconfig=maybe])],
			[cf_have_ncuconfig=no])
		AC_MSG_RESULT($cf_have_ncuconfig)
		test "$cf_have_ncuconfig" = maybe && cf_have_ncuconfig=yes
		if test "$cf_have_ncuconfig" != "yes"
		then
			CPPFLAGS="$cf_save_CPPFLAGS"
			LIBS="$cf_save_LIBS"
			NCURSES_CONFIG_PKG=none
		else
			AC_DEFINE(NCURSES,1,[Define to 1 if we are using ncurses headers/libraries])
			NCURSES_CONFIG_PKG=$cf_ncuconfig_root
			CF_TERM_HEADER
		fi

	else
		AC_MSG_RESULT(no)
		NCURSES_CONFIG_PKG=none
	fi
else
	NCURSES_CONFIG_PKG=none
fi

if test "x$cf_have_ncuconfig" = "xno"; then
	cf_ncurses_config="${cf_ncuconfig_root}${NCURSES_CONFIG_SUFFIX}-config"; echo "Looking for ${cf_ncurses_config}"

	CF_ACVERSION_CHECK(2.52,
		[AC_CHECK_TOOLS(NCURSES_CONFIG, ${cf_ncurses_config} ${cf_ncuconfig_root}6-config ${cf_ncuconfig_root}6-config ${cf_ncuconfig_root}5-config, none)],
		[AC_PATH_PROGS(NCURSES_CONFIG,  ${cf_ncurses_config} ${cf_ncuconfig_root}6-config ${cf_ncuconfig_root}6-config ${cf_ncuconfig_root}5-config, none)])

	if test "$NCURSES_CONFIG" != none ; then

		CF_ADD_CFLAGS(`$NCURSES_CONFIG --cflags`)
		CF_ADD_LIBS(`$NCURSES_CONFIG --libs`)

		# even with config script, some packages use no-override for curses.h
		CF_CURSES_HEADER(ifelse($1,,ncurses,$1))

		dnl like CF_NCURSES_CPPFLAGS
		AC_DEFINE(NCURSES,1,[Define to 1 if we are using ncurses headers/libraries])

		dnl like CF_NCURSES_LIBS
		CF_UPPER(cf_nculib_ROOT,HAVE_LIB$cf_ncuconfig_root)
		AC_DEFINE_UNQUOTED($cf_nculib_ROOT)

		dnl like CF_NCURSES_VERSION
		cf_cv_ncurses_version="`$NCURSES_CONFIG --version`"

	else

		CF_NCURSES_CPPFLAGS(ifelse($1,,ncurses,$1))
		CF_NCURSES_LIBS(ifelse($1,,ncurses,$1))

	fi
else
	NCURSES_CONFIG=none
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_CPPFLAGS version: 22 updated: 2021/01/02 09:31:20
dnl -------------------
dnl Look for the SVr4 curses clone 'ncurses' in the standard places, adjusting
dnl the CPPFLAGS variable so we can include its header.
dnl
dnl The header files may be installed as either curses.h, or ncurses.h (would
dnl be obsolete, except that some packagers prefer this name to distinguish it
dnl from a "native" curses implementation).  If not installed for overwrite,
dnl the curses.h file would be in an ncurses subdirectory (e.g.,
dnl /usr/include/ncurses), but someone may have installed overwriting the
dnl vendor's curses.  Only very old versions (pre-1.9.2d, the first autoconf'd
dnl version) of ncurses don't define either __NCURSES_H or NCURSES_VERSION in
dnl the header.
dnl
dnl If the installer has set $CFLAGS or $CPPFLAGS so that the ncurses header
dnl is already in the include-path, don't even bother with this, since we cannot
dnl easily determine which file it is.  In this case, it has to be <curses.h>.
dnl
dnl The optional parameter gives the root name of the library, in case it is
dnl not installed as the default curses library.  That is how the
dnl wide-character version of ncurses is installed.
AC_DEFUN([CF_NCURSES_CPPFLAGS],
[AC_REQUIRE([CF_WITH_CURSES_DIR])

AC_PROVIDE([CF_CURSES_CPPFLAGS])dnl
cf_ncuhdr_root=ifelse($1,,ncurses,$1)

test -n "$cf_cv_curses_dir" && \
test "$cf_cv_curses_dir" != "no" && { \
  CF_ADD_INCDIR($cf_cv_curses_dir/include/$cf_ncuhdr_root)
}

AC_CACHE_CHECK(for $cf_ncuhdr_root header in include-path, cf_cv_ncurses_h,[
	cf_header_list="$cf_ncuhdr_root/curses.h $cf_ncuhdr_root/ncurses.h"
	{ test "$cf_ncuhdr_root" = ncurses || test "$cf_ncuhdr_root" = ncursesw; } && cf_header_list="$cf_header_list curses.h ncurses.h"
	for cf_header in $cf_header_list
	do
		CF_NCURSES_CC_CHECK(cf_cv_ncurses_h,$cf_header,$1)
		test "$cf_cv_ncurses_h" != no && break
	done
])

CF_NCURSES_HEADER
CF_TERM_HEADER

# some applications need this, but should check for NCURSES_VERSION
AC_DEFINE(NCURSES,1,[Define to 1 if we are using ncurses headers/libraries])

CF_NCURSES_VERSION
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_HEADER version: 7 updated: 2021/01/04 19:33:05
dnl -----------------
dnl Find a "curses" header file, e.g,. "curses.h", or one of the more common
dnl variations of ncurses' installs.
dnl
dnl See also CF_CURSES_HEADER, which sets the same cache variable.
AC_DEFUN([CF_NCURSES_HEADER],[

if test "$cf_cv_ncurses_h" != no ; then
	cf_cv_ncurses_header=$cf_cv_ncurses_h
else

AC_CACHE_CHECK(for $cf_ncuhdr_root include-path, cf_cv_ncurses_h2,[
	test -n "$verbose" && echo
	CF_HEADER_PATH(cf_search,$cf_ncuhdr_root)
	test -n "$verbose" && echo "search path $cf_search"
	cf_save2_CPPFLAGS="$CPPFLAGS"
	for cf_incdir in $cf_search
	do
		CF_ADD_INCDIR($cf_incdir)
		for cf_header in \
			ncurses.h \
			curses.h
		do
			CF_NCURSES_CC_CHECK(cf_cv_ncurses_h2,$cf_header,$1)
			if test "$cf_cv_ncurses_h2" != no ; then
				cf_cv_ncurses_h2=$cf_incdir/$cf_header
				test -n "$verbose" && echo $ECHO_N "	... found $ECHO_C" 1>&AC_FD_MSG
				break
			fi
			test -n "$verbose" && echo "	... tested $cf_incdir/$cf_header" 1>&AC_FD_MSG
		done
		CPPFLAGS="$cf_save2_CPPFLAGS"
		test "$cf_cv_ncurses_h2" != no && break
	done
	test "$cf_cv_ncurses_h2" = no && AC_MSG_ERROR(not found)
	])

	CF_DIRNAME(cf_1st_incdir,$cf_cv_ncurses_h2)
	cf_cv_ncurses_header="`basename "$cf_cv_ncurses_h2"`"
	if test "`basename "$cf_1st_incdir"`" = "$cf_ncuhdr_root" ; then
		cf_cv_ncurses_header="$cf_ncuhdr_root/$cf_cv_ncurses_header"
	fi
	CF_ADD_INCDIR($cf_1st_incdir)

fi

# Set definitions to allow ifdef'ing for ncurses.h

case "$cf_cv_ncurses_header" in
(*ncurses.h)
	AC_DEFINE(HAVE_NCURSES_H,1,[Define to 1 if we have ncurses.h])
	;;
esac

case "$cf_cv_ncurses_header" in
(ncurses/curses.h|ncurses/ncurses.h)
	AC_DEFINE(HAVE_NCURSES_NCURSES_H,1,[Define to 1 if we have ncurses/ncurses.h])
	;;
(ncursesw/curses.h|ncursesw/ncurses.h)
	AC_DEFINE(HAVE_NCURSESW_NCURSES_H,1,[Define to 1 if we have ncursesw/ncurses.h])
	;;
esac

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_LIBS version: 20 updated: 2021/01/03 08:05:37
dnl ---------------
dnl Look for the ncurses library.  This is a little complicated on Linux,
dnl because it may be linked with the gpm (general purpose mouse) library.
dnl Some distributions have gpm linked with (bsd) curses, which makes it
dnl unusable with ncurses.  However, we don't want to link with gpm unless
dnl ncurses has a dependency, since gpm is normally set up as a shared library,
dnl and the linker will record a dependency.
dnl
dnl The optional parameter gives the root name of the library, in case it is
dnl not installed as the default curses library.  That is how the
dnl wide-character version of ncurses is installed.
AC_DEFUN([CF_NCURSES_LIBS],
[AC_REQUIRE([CF_NCURSES_CPPFLAGS])

cf_nculib_root=ifelse($1,,ncurses,$1)
	# This works, except for the special case where we find gpm, but
	# ncurses is in a nonstandard location via $LIBS, and we really want
	# to link gpm.
cf_ncurses_LIBS=""
cf_ncurses_SAVE="$LIBS"
AC_CHECK_LIB(gpm,Gpm_Open,
	[AC_CHECK_LIB(gpm,initscr,
		[LIBS="$cf_ncurses_SAVE"],
		[cf_ncurses_LIBS="-lgpm"])])

case "$host_os" in
(freebsd*)
	# This is only necessary if you are linking against an obsolete
	# version of ncurses (but it should do no harm, since it's static).
	if test "$cf_nculib_root" = ncurses ; then
		AC_CHECK_LIB(mytinfo,tgoto,[cf_ncurses_LIBS="-lmytinfo $cf_ncurses_LIBS"])
	fi
	;;
esac

CF_ADD_LIBS($cf_ncurses_LIBS)

if test -n "$cf_cv_curses_dir" && test "$cf_cv_curses_dir" != "no"
then
	CF_ADD_LIBS(-l$cf_nculib_root)
else
	CF_FIND_LIBRARY($cf_nculib_root,$cf_nculib_root,
		[#include <${cf_cv_ncurses_header:-curses.h}>],
		[initscr()],
		initscr)
fi

if test -n "$cf_ncurses_LIBS" ; then
	AC_MSG_CHECKING(if we can link $cf_nculib_root without $cf_ncurses_LIBS)
	cf_ncurses_SAVE="$LIBS"
	for p in $cf_ncurses_LIBS ; do
		q=`echo "$LIBS" | sed -e "s%$p %%" -e "s%$p$%%"`
		if test "$q" != "$LIBS" ; then
			LIBS="$q"
		fi
	done
	AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
		[initscr(); mousemask(0,0); tigetstr((char *)0);],
		[AC_MSG_RESULT(yes)],
		[AC_MSG_RESULT(no)
		 LIBS="$cf_ncurses_SAVE"])
fi

CF_UPPER(cf_nculib_ROOT,HAVE_LIB$cf_nculib_root)
AC_DEFINE_UNQUOTED($cf_nculib_ROOT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_VERSION version: 16 updated: 2020/12/31 20:19:42
dnl ------------------
dnl Check for the version of ncurses, to aid in reporting bugs, etc.
dnl Call CF_CURSES_CPPFLAGS first, or CF_NCURSES_CPPFLAGS.  We don't use
dnl AC_REQUIRE since that does not work with the shell's if/then/else/fi.
AC_DEFUN([CF_NCURSES_VERSION],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
AC_CACHE_CHECK(for ncurses version, cf_cv_ncurses_version,[
	cf_cv_ncurses_version=no
	cf_tempfile=out$$
	rm -f "$cf_tempfile"
	AC_TRY_RUN([
#include <${cf_cv_ncurses_header:-curses.h}>
#include <stdio.h>
int main(void)
{
	FILE *fp = fopen("$cf_tempfile", "w");
#ifdef NCURSES_VERSION
# ifdef NCURSES_VERSION_PATCH
	fprintf(fp, "%s.%d\\n", NCURSES_VERSION, NCURSES_VERSION_PATCH);
# else
	fprintf(fp, "%s\\n", NCURSES_VERSION);
# endif
#else
# ifdef __NCURSES_H
	fprintf(fp, "old\\n");
# else
	make an error
# endif
#endif
	${cf_cv_main_return:-return}(0);
}],[
	cf_cv_ncurses_version=`cat $cf_tempfile`],,[

	# This will not work if the preprocessor splits the line after the
	# Autoconf token.  The 'unproto' program does that.
	cat > "conftest.$ac_ext" <<EOF
#include <${cf_cv_ncurses_header:-curses.h}>
#undef Autoconf
#ifdef NCURSES_VERSION
Autoconf NCURSES_VERSION
#else
#ifdef __NCURSES_H
Autoconf "old"
#endif
;
#endif
EOF
	cf_try="$ac_cpp conftest.$ac_ext 2>&AC_FD_CC | grep '^Autoconf ' >conftest.out"
	AC_TRY_EVAL(cf_try)
	if test -f conftest.out ; then
		cf_out=`sed -e 's%^Autoconf %%' -e 's%^[[^"]]*"%%' -e 's%".*%%' conftest.out`
		test -n "$cf_out" && cf_cv_ncurses_version="$cf_out"
		rm -f conftest.out
	fi
])
	rm -f "$cf_tempfile"
])
test "$cf_cv_ncurses_version" = no || AC_DEFINE(NCURSES,1,[Define to 1 if we are using ncurses headers/libraries])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NETLIBS version: 12 updated: 2021/01/02 09:31:20
dnl ----------
dnl After checking for functions in the default $LIBS, make a further check
dnl for the functions that are netlib-related (these aren't always in the
dnl libc, etc., and have to be handled specially because there are conflicting
dnl and broken implementations.
dnl Common library requirements (in order):
dnl	-lresolv -lsocket -lnsl
dnl	-lnsl -lsocket
dnl	-lsocket
dnl	-lbsd
dnl	-lnetwork
AC_DEFUN([CF_NETLIBS],[
cf_test_netlibs=no

AC_MSG_CHECKING(for network libraries)

AC_CACHE_VAL(cf_cv_netlibs,[
AC_MSG_RESULT(working...)

cf_cv_netlibs=""
cf_test_netlibs=yes

case "$host_os" in
(mingw*)
	AC_CHECK_HEADERS( windows.h winsock.h winsock2.h )

	if test "$ac_cv_header_winsock2_h" = "yes" ; then
		cf_winsock_lib="-lws2_32"
	elif test "$ac_cv_header_winsock_h" = "yes" ; then
		cf_winsock_lib="-lwsock32"
	fi

	cf_save_LIBS="$LIBS"
	CF_ADD_LIBS($cf_winsock_lib)

	AC_TRY_LINK([
#ifdef HAVE_WINDOWS_H
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#endif
],[
	char buffer[1024];
	gethostname(buffer, sizeof(buffer));],
	[cf_cv_netlibs="$cf_winsock_lib $cf_cv_netlibs"],
	[AC_MSG_ERROR(Cannot link against winsock library)])

	LIBS="$cf_save_LIBS"
	;;
(*)
	AC_CHECK_FUNCS(gethostname,,[
		CF_RECHECK_FUNC(gethostname,nsl,cf_cv_netlibs,[
			CF_RECHECK_FUNC(gethostname,socket,cf_cv_netlibs)])])

	AC_CHECK_LIB(inet, main, cf_cv_netlibs="-linet $cf_cv_netlibs")

	if test "$ac_cv_func_lsocket" != no ; then
	AC_CHECK_FUNCS(socket,,[
		CF_RECHECK_FUNC(socket,socket,cf_cv_netlibs,[
			CF_RECHECK_FUNC(socket,bsd,cf_cv_netlibs)])])
	fi

	AC_CHECK_FUNCS(gethostbyname,,[
		CF_RECHECK_FUNC(gethostbyname,nsl,cf_cv_netlibs)])

	AC_CHECK_FUNCS(inet_ntoa,,[
		CF_RECHECK_FUNC(inet_ntoa,nsl,cf_cv_netlibs)])

	AC_CHECK_FUNCS(gethostbyname,,[
		CF_RECHECK_FUNC(gethostbyname,network,cf_cv_netlibs)])

	AC_CHECK_FUNCS(strcasecmp,,[
		CF_RECHECK_FUNC(strcasecmp,resolv,cf_cv_netlibs)])
	;;
esac
])

case "$cf_cv_netlibs" in
(*ws2_32*)
	AC_DEFINE(USE_WINSOCK2_H,1,[Define to 1 if we should include winsock2.h])
	;;
esac

CF_ADD_LIBS($cf_cv_netlibs)
test "$cf_test_netlibs" = no && echo "$cf_cv_netlibs" >&AC_FD_MSG
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NGROUPS version: 4 updated: 2012/11/08 20:57:52
dnl ----------
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
])
if test "$cf_cv_ngroups" = no ; then
	AC_DEFINE(NGROUPS,16,[Define to maximum number of groups])
elif test "$cf_cv_ngroups" = NGROUPS_MAX ; then
	AC_DEFINE(NGROUPS,NGROUPS_MAX)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NO_LEAKS_OPTION version: 9 updated: 2021/06/13 19:45:41
dnl ------------------
dnl see CF_WITH_NO_LEAKS
dnl
dnl $1 = option/name
dnl $2 = help-text
dnl $3 = symbol to define if the option is set
dnl $4 = additional actions to take if the option is set
AC_DEFUN([CF_NO_LEAKS_OPTION],[
AC_MSG_CHECKING(if you want to use $1 for testing)
AC_ARG_WITH($1,
	[$2],
	[case "x$withval" in
	(x|xno) ;;
	(*)
		: "${with_cflags:=-g}"
		: "${enable_leaks:=no}"
		with_$1=yes
		AC_DEFINE_UNQUOTED($3,1,"Define to 1 if you want to use $1 for testing.")ifelse([$4],,[
	 $4
])
		;;
	esac],
	[with_$1=])
AC_MSG_RESULT(${with_$1:-no})

case ".$with_cflags" in
(.*-g*)
	case .$CFLAGS in
	(.*-g*)
		;;
	(*)
		CF_ADD_CFLAGS([-g])
		;;
	esac
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NSS_COMPAT version: 7 updated: 2021/01/02 09:31:20
dnl -------------
dnl Check for NSS compatible SSL libraries
dnl $1 = the [optional] directory in which the library may be found
AC_DEFUN([CF_NSS_COMPAT],[
check=`pkg-config --version 2>/dev/null`
if test -n "$check" ; then
	cf_ssl_library=`pkg-config --libs nss`
	cf_ssl_cflags=`pkg-config --cflags nss`
else
	# Without pkg-config, we'll kludge in some defaults
	cf_ssl_library="-lssl3 -lsmime3 -lnss3 -lplds4 -lplc4 -lnspr4 -lpthread -ldl"
	cf_ssl_cflags="-I/usr/include/nss3 -I/usr/include/nspr4"
fi
cf_ssl_library="-lnss_compat_ossl $cf_ssl_library"

case "$1" in
(no)
	cf_ssl_root=
	;;
(yes)
	AC_CHECK_LIB(nss_compat_ossl, SSL_get_version,[],[
		cf_ssl_root=/usr/local/nss_compat_ossl
		if test -d "$cf_ssl_root" ; then
			CF_VERBOSE(assume it is in $cf_ssl_root)
			cf_ssl_library="-L$cf_ssl_root/lib $cf_ssl_library"
		else
			AC_MSG_ERROR(cannot find NSS compliant libraries)
		fi
	],
	[-lnss_compat_ossl])
	;;
(*)
	if test -d "$1" ; then
		if test -d "$1/include" ; then
			cf_ssl_root=$1
		elif test -d "$1/../include" ; then
			cf_ssl_root=$1/..
		else
			AC_MSG_ERROR(cannot find NSS compliant library under $1)
		fi
		cf_ssl_library="-L$cf_ssl_root/lib $cf_ssl_library"
	else
		AC_MSG_WARN(expected a directory: $1)
	fi
	;;
esac
CF_ADD_LIBS($cf_ssl_library)

cf_ssl_subincs=yes
if test -n "$cf_ssl_root" ; then
	if test -d "$cf_ssl_root/include" ; then
		cf_ssl_cflags="-I$cf_ssl_root/include  $cf_ssl_cflags"
		test -d "$cf_ssl_root/include/nss_compat_ossl" || cf_ssl_subincs=no
	fi
fi
CF_ADD_CFLAGS($cf_ssl_cflags)

if test "$cf_ssl_subincs" = yes ; then
AC_MSG_CHECKING(for NSS compliant include directory)
AC_TRY_COMPILE([
#include <stdio.h>
#include <nss_compat_ossl/nss_compat_ossl.h>],
	[SSL_shutdown((SSL *)0)],
	[cf_ssl_incl=yes],
	[cf_ssl_incl=no])
AC_MSG_RESULT($cf_ssl_incl)
test "$cf_ssl_incl" = yes && AC_DEFINE(USE_NSS_COMPAT_INCL,1,[Define to 1 if we should use nss compatibility header])
fi

AC_MSG_CHECKING(if we can link to NSS compliant library)
AC_TRY_LINK([
#include <stdio.h>
#ifdef USE_NSS_COMPAT_INCL
#include <nss_compat_ossl/nss_compat_ossl.h>
#else
#include <ssl.h>
#endif
],
	[SSL_shutdown((SSL *)0)],
	[cf_ssl_library=yes],
	[cf_ssl_library=no])
AC_MSG_RESULT($cf_ssl_library)
if test "$cf_ssl_library" = yes ; then
	AC_DEFINE(USE_SSL,1,[Define to 1 if we should use SSL])
	AC_DEFINE(USE_X509_SUPPORT,1,[Define to 1 if the SSL library provides X509 support])
else
	AC_MSG_ERROR(Cannot link with NSS compliant libraries)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_OUR_MESSAGES version: 8 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Check if we use the messages included with this program
dnl
dnl $1 specifies either Makefile or makefile, defaulting to the former.
dnl
dnl Sets variables which can be used to substitute in makefiles:
dnl	MSG_DIR_MAKE - to make ./po directory
dnl	SUB_MAKEFILE - makefile in ./po directory (see CF_BUNDLED_INTL)
dnl
AC_DEFUN([CF_OUR_MESSAGES],
[
cf_makefile=ifelse($1,,Makefile,$1)

use_our_messages=no
if test "$USE_NLS" = yes ; then
if test -d "$srcdir/po" ; then
AC_MSG_CHECKING(if we should use included message-library)
	AC_ARG_ENABLE(included-msgs,
	[  --disable-included-msgs use included messages, for i18n support],
	[use_our_messages=$enableval],
	[use_our_messages=yes])
fi
AC_MSG_RESULT($use_our_messages)
fi

MSG_DIR_MAKE="#"
if test "$use_our_messages" = yes
then
	SUB_MAKEFILE="$SUB_MAKEFILE po/$cf_makefile.in:$srcdir/po/$cf_makefile.inn"
	MSG_DIR_MAKE=
fi

AC_SUBST(MSG_DIR_MAKE)
AC_SUBST(SUB_MAKEFILE)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATHSEP version: 8 updated: 2021/01/01 13:31:04
dnl ----------
dnl Provide a value for the $PATH and similar separator (or amend the value
dnl as provided in autoconf 2.5x).
AC_DEFUN([CF_PATHSEP],
[
	AC_MSG_CHECKING(for PATH separator)
	case "$cf_cv_system_name" in
	(os2*)	PATH_SEPARATOR=';'  ;;
	(*)	${PATH_SEPARATOR:=':'}  ;;
	esac
ifelse([$1],,,[$1=$PATH_SEPARATOR])
	AC_SUBST(PATH_SEPARATOR)
	AC_MSG_RESULT($PATH_SEPARATOR)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_PROG version: 12 updated: 2021/01/02 09:31:20
dnl ------------
dnl Check for a given program, defining corresponding symbol.
dnl	$1 = environment variable, which is suffixed by "_PATH" in the #define.
dnl	$2 = program name to find.
dnl	$3 = optional list of additional program names to test.
dnl $4 = $PATH
dnl
dnl If there is more than one token in the result, #define the remaining tokens
dnl to $1_ARGS.  We need this for 'install' in particular.
dnl
dnl FIXME: we should allow this to be overridden by environment variables
dnl
AC_DEFUN([CF_PATH_PROG],[
AC_REQUIRE([CF_PATHSEP])
test -z "[$]$1" && $1="$2"
AC_PATH_PROGS($1,[$]$1 $2 ifelse($3,,,$3),[$]$1, ifelse($4,,,$4))

cf_path_prog=""
cf_path_args=""
IFS="${IFS:- 	}"; cf_save_ifs="$IFS"; IFS="${IFS}$PATH_SEPARATOR"
for cf_temp in $ac_cv_path_$1
do
	if test -z "$cf_path_prog" ; then
		if test "$with_full_paths" = yes ; then
			CF_PATH_SYNTAX(cf_temp,break)
			cf_path_prog="$cf_temp"
		else
			cf_path_prog="`basename "$cf_temp"`"
		fi
	elif test -z "$cf_path_args" ; then
		cf_path_args="$cf_temp"
	else
		cf_path_args="$cf_path_args $cf_temp"
	fi
done
IFS="$cf_save_ifs"

if test -n "$cf_path_prog" ; then
	CF_MSG_LOG(defining path for ${cf_path_prog})
	AC_DEFINE_UNQUOTED($1_PATH,"$cf_path_prog",Define to pathname $1)
	test -n "$cf_path_args" && AC_DEFINE_UNQUOTED($1_ARGS,"$cf_path_args",Define to provide args for $1)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_SYNTAX version: 18 updated: 2020/12/31 18:40:20
dnl --------------
dnl Check the argument to see that it looks like a pathname.  Rewrite it if it
dnl begins with one of the prefix/exec_prefix variables, and then again if the
dnl result begins with 'NONE'.  This is necessary to work around autoconf's
dnl delayed evaluation of those symbols.
AC_DEFUN([CF_PATH_SYNTAX],[
if test "x$prefix" != xNONE; then
	cf_path_syntax="$prefix"
else
	cf_path_syntax="$ac_default_prefix"
fi

case ".[$]$1" in
(.\[$]\(*\)*|.\'*\'*)
	;;
(..|./*|.\\*)
	;;
(.[[a-zA-Z]]:[[\\/]]*) # OS/2 EMX
	;;
(.\[$]\{*prefix\}*|.\[$]\{*dir\}*)
	eval $1="[$]$1"
	case ".[$]$1" in
	(.NONE/*)
		$1=`echo "[$]$1" | sed -e s%NONE%$cf_path_syntax%`
		;;
	esac
	;;
(.no|.NONE/*)
	$1=`echo "[$]$1" | sed -e s%NONE%$cf_path_syntax%`
	;;
(*)
	ifelse([$2],,[AC_MSG_ERROR([expected a pathname, not \"[$]$1\"])],$2)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PDCURSES_W32 version: 2 updated: 2019/12/31 10:27:03
dnl ---------------
dnl Configure for PDCurses' Win32 library, checking for definitions as well
dnl which are needed to use its header file correctly.
AC_DEFUN([CF_PDCURSES_W32],[

AC_CHECK_LIB(pdcurses,initscr,[
	CF_ADD_LIBS(-lpdcurses)
	cf_cv_term_header=no
	cf_cv_unctrl_header=no
	AC_CHECK_FUNC(winwstr,[AC_DEFINE(PDC_WIDE,1,[Define to 1 if PDCurses has winwstr])])
	AC_CHECK_FUNC(pdcurses_dll_iname,[AC_DEFINE(PDC_DLL_BUILD,1,[Define to 1 if PDCurses has pdcurses_dll_iname])])
])

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PDCURSES_X11 version: 15 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Configure for PDCurses' X11 library
AC_DEFUN([CF_PDCURSES_X11],[
AC_REQUIRE([CF_X_ATHENA])

CF_ACVERSION_CHECK(2.52,
	[AC_CHECK_TOOLS(XCURSES_CONFIG, xcurses-config, none)],
	[AC_PATH_PROGS(XCURSES_CONFIG, xcurses-config, none)])

if test "$XCURSES_CONFIG" != none ; then

CF_ADD_CFLAGS(`$XCURSES_CONFIG --cflags`)
CF_ADD_LIBS(`$XCURSES_CONFIG --libs`)

cf_cv_lib_XCurses=yes

else

LDFLAGS="$LDFLAGS $X_LIBS"
CF_CHECK_CFLAGS($X_CFLAGS)
AC_CHECK_LIB(X11,XOpenDisplay,
	[CF_ADD_LIBS(-lX11)],,
	[$X_PRE_LIBS $LIBS $X_EXTRA_LIBS])
AC_CACHE_CHECK(for XCurses library,cf_cv_lib_XCurses,[
CF_ADD_LIBS(-lXCurses)
AC_TRY_LINK([
#include <xcurses.h>
char *XCursesProgramName = "test";
],[XCursesExit();],
[cf_cv_lib_XCurses=yes],
[cf_cv_lib_XCurses=no])
])

fi

if test "$cf_cv_lib_XCurses" = yes ; then
	AC_DEFINE(UNIX,1,[Define to 1 if using PDCurses on Unix])
	AC_DEFINE(XCURSES,1,[Define to 1 if using PDCurses on Unix])
	AC_CHECK_HEADER(xcurses.h, AC_DEFINE(HAVE_XCURSES,1,[Define to 1 if using PDCurses on Unix]))
else
	AC_MSG_ERROR(Cannot link with XCurses)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PKG_CONFIG version: 11 updated: 2021/01/01 13:31:04
dnl -------------
dnl Check for the package-config program, unless disabled by command-line.
AC_DEFUN([CF_PKG_CONFIG],
[
AC_MSG_CHECKING(if you want to use pkg-config)
AC_ARG_WITH(pkg-config,
	[  --with-pkg-config{=path} enable/disable use of pkg-config],
	[cf_pkg_config=$withval],
	[cf_pkg_config=yes])
AC_MSG_RESULT($cf_pkg_config)

case "$cf_pkg_config" in
(no)
	PKG_CONFIG=none
	;;
(yes)
	CF_ACVERSION_CHECK(2.52,
		[AC_PATH_TOOL(PKG_CONFIG, pkg-config, none)],
		[AC_PATH_PROG(PKG_CONFIG, pkg-config, none)])
	;;
(*)
	PKG_CONFIG=$withval
	;;
esac

test -z "$PKG_CONFIG" && PKG_CONFIG=none
if test "$PKG_CONFIG" != none ; then
	CF_PATH_SYNTAX(PKG_CONFIG)
elif test "x$cf_pkg_config" != xno ; then
	AC_MSG_WARN(pkg-config is not installed)
fi

AC_SUBST(PKG_CONFIG)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_C_SOURCE version: 11 updated: 2018/12/31 20:46:17
dnl -----------------
dnl Define _POSIX_C_SOURCE to the given level, and _POSIX_SOURCE if needed.
dnl
dnl	POSIX.1-1990				_POSIX_SOURCE
dnl	POSIX.1-1990 and			_POSIX_SOURCE and
dnl		POSIX.2-1992 C-Language			_POSIX_C_SOURCE=2
dnl		Bindings Option
dnl	POSIX.1b-1993				_POSIX_C_SOURCE=199309L
dnl	POSIX.1c-1996				_POSIX_C_SOURCE=199506L
dnl	X/Open 2000				_POSIX_C_SOURCE=200112L
dnl
dnl Parameters:
dnl	$1 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_POSIX_C_SOURCE],
[AC_REQUIRE([CF_POSIX_VISIBLE])dnl

if test "$cf_cv_posix_visible" = no; then

cf_POSIX_C_SOURCE=ifelse([$1],,199506L,[$1])

cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"

CF_REMOVE_DEFINE(cf_trim_CFLAGS,$cf_save_CFLAGS,_POSIX_C_SOURCE)
CF_REMOVE_DEFINE(cf_trim_CPPFLAGS,$cf_save_CPPFLAGS,_POSIX_C_SOURCE)

AC_CACHE_CHECK(if we should define _POSIX_C_SOURCE,cf_cv_posix_c_source,[
	CF_MSG_LOG(if the symbol is already defined go no further)
	AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
make an error
#endif],
	[cf_cv_posix_c_source=no],
	[cf_want_posix_source=no
	 case .$cf_POSIX_C_SOURCE in
	 (.[[12]]??*)
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		;;
	 (.2)
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		cf_want_posix_source=yes
		;;
	 (.*)
		cf_want_posix_source=yes
		;;
	 esac
	 if test "$cf_want_posix_source" = yes ; then
		AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _POSIX_SOURCE
make an error
#endif],[],
		cf_cv_posix_c_source="$cf_cv_posix_c_source -D_POSIX_SOURCE")
	 fi
	 CF_MSG_LOG(ifdef from value $cf_POSIX_C_SOURCE)
	 CFLAGS="$cf_trim_CFLAGS"
	 CPPFLAGS="$cf_trim_CPPFLAGS"
	 CF_APPEND_TEXT(CPPFLAGS,$cf_cv_posix_c_source)
	 CF_MSG_LOG(if the second compile does not leave our definition intact error)
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
make an error
#endif],,
	 [cf_cv_posix_c_source=no])
	 CFLAGS="$cf_save_CFLAGS"
	 CPPFLAGS="$cf_save_CPPFLAGS"
	])
])

if test "$cf_cv_posix_c_source" != no ; then
	CFLAGS="$cf_trim_CFLAGS"
	CPPFLAGS="$cf_trim_CPPFLAGS"
	CF_ADD_CFLAGS($cf_cv_posix_c_source)
fi

fi # cf_cv_posix_visible

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_VISIBLE version: 1 updated: 2018/12/31 20:46:17
dnl ----------------
dnl POSIX documents test-macros which an application may set before any system
dnl headers are included to make features available.
dnl
dnl Some BSD platforms (originally FreeBSD, but copied by a few others)
dnl diverged from POSIX in 2002 by setting symbols which make all of the most
dnl recent features visible in the system header files unless the application
dnl overrides the corresponding test-macros.  Doing that introduces portability
dnl problems.
dnl
dnl This macro makes a special check for the symbols used for this, to avoid a
dnl conflicting definition.
AC_DEFUN([CF_POSIX_VISIBLE],
[
AC_CACHE_CHECK(if the POSIX test-macros are already defined,cf_cv_posix_visible,[
AC_TRY_COMPILE([#include <stdio.h>],[
#if defined(__POSIX_VISIBLE) && ((__POSIX_VISIBLE - 0L) > 0) \
	&& defined(__XSI_VISIBLE) && ((__XSI_VISIBLE - 0L) > 0) \
	&& defined(__BSD_VISIBLE) && ((__BSD_VISIBLE - 0L) > 0) \
	&& defined(__ISO_C_VISIBLE) && ((__ISO_C_VISIBLE - 0L) > 0)
#error conflicting symbols found
#endif
],[cf_cv_posix_visible=no],[cf_cv_posix_visible=yes])
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_AR version: 1 updated: 2009/01/01 20:15:22
dnl ----------
dnl Check for archiver "ar".
AC_DEFUN([CF_PROG_AR],[
AC_CHECK_TOOL(AR, ar, ar)
])
dnl ---------------------------------------------------------------------------
dnl CF_PROG_CC version: 5 updated: 2019/12/31 08:53:54
dnl ----------
dnl standard check for CC, plus followup sanity checks
dnl $1 = optional parameter to pass to AC_PROG_CC to specify compiler name
AC_DEFUN([CF_PROG_CC],[
CF_ACVERSION_CHECK(2.53,
	[AC_MSG_WARN(this will incorrectly handle gnatgcc choice)
	 AC_REQUIRE([AC_PROG_CC])],
	[])
ifelse($1,,[AC_PROG_CC],[AC_PROG_CC($1)])
CF_GCC_VERSION
CF_ACVERSION_CHECK(2.52,
	[AC_PROG_CC_STDC],
	[CF_ANSI_CC_REQD])
CF_CC_ENV_FLAGS
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_CPP_COMMENTS version: 1 updated: 2017/07/02 14:31:07
dnl --------------------
dnl Ask for preprocessor "-C" option if we can get it.
AC_DEFUN([CF_PROG_CPP_COMMENTS],[
AC_REQUIRE([AC_PROG_CPP])
AC_MSG_CHECKING(if preprocessor -C option works)
AC_CACHE_VAL(cf_cv_prog_cpp_comments,[
cf_cv_prog_cpp_comments=no
cat >conftest.c <<CF_EOF
int main(void)
{
	/* COMMENT */
	return 0;
}
CF_EOF
if ( $CPP -C conftest.c >conftest.i 2>/dev/null )
then
	if ( grep COMMENT conftest.i >/dev/null 2>/dev/null )
	then
		cf_cv_prog_cpp_comments=yes
	fi
fi
rm -f conftest.[[ci]]
])
AC_MSG_RESULT($cf_cv_prog_cpp_comments)
if test x$cf_cv_prog_cpp_comments = xyes
then
	CPP="$CPP -C"
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_EXT version: 15 updated: 2021/01/02 09:31:20
dnl -----------
dnl Compute $PROG_EXT, used for non-Unix ports, such as OS/2 EMX.
AC_DEFUN([CF_PROG_EXT],
[
AC_REQUIRE([CF_CHECK_CACHE])
case "$cf_cv_system_name" in
(os2*)
	CFLAGS="$CFLAGS -Zmt"
	CF_APPEND_TEXT(CPPFLAGS,-D__ST_MT_ERRNO__)
	CXXFLAGS="$CXXFLAGS -Zmt"
	# autoconf's macro sets -Zexe and suffix both, which conflict:w
	LDFLAGS="$LDFLAGS -Zmt -Zcrtdll"
	ac_cv_exeext=.exe
	;;
esac

AC_EXEEXT
AC_OBJEXT

PROG_EXT="$EXEEXT"
AC_SUBST(PROG_EXT)
test -n "$PROG_EXT" && AC_DEFINE_UNQUOTED(PROG_EXT,"$PROG_EXT",[Define to the program extension (normally blank)])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_LINT version: 4 updated: 2019/11/20 18:55:37
dnl ------------
AC_DEFUN([CF_PROG_LINT],
[
AC_CHECK_PROGS(LINT, lint cppcheck splint)
case "x$LINT" in
(xcppcheck|x*/cppcheck)
	test -z "$LINT_OPTS" && LINT_OPTS="--enable=all"
	;;
esac
AC_SUBST(LINT_OPTS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_RANLIB version: 1 updated: 2009/01/01 20:15:22
dnl --------------
dnl Check for ranlib.
dnl
dnl TODO: make this conditionally depend on autoconf version, since the later
dnl versions of autoconf use check-tool.
AC_DEFUN([CF_PROG_RANLIB],[
AC_CHECK_TOOL(RANLIB, ranlib, ':')
])
dnl ---------------------------------------------------------------------------
dnl CF_RECHECK_FUNC version: 3 updated: 2000/10/18 19:29:13
dnl ---------------
dnl Re-check on a function to see if we can pick it up by adding a library.
dnl	$1 = function to check
dnl	$2 = library to check in
dnl	$3 = environment to update (e.g., $LIBS)
dnl	$4 = what to do if this fails
dnl
dnl This uses 'unset' if the shell happens to support it, but leaves the
dnl configuration variable set to 'unknown' if not.  This is a little better
dnl than the normal autoconf test, which gives misleading results if a test
dnl for the function is made (e.g., with AC_CHECK_FUNC) after this macro is
dnl used (autoconf does not distinguish between a null token and one that is
dnl set to 'no').
AC_DEFUN([CF_RECHECK_FUNC],[
AC_CHECK_LIB($2,$1,[
	CF_UPPER(cf_tr_func,$1)
	AC_DEFINE_UNQUOTED(HAVE_$cf_tr_func)
	ac_cv_func_$1=yes
	if test "$cf_used_lib_$2" != yes ; then cf_used_lib_$2=yes; $3="-l$2 [$]$3"; fi],[
	ac_cv_func_$1=unknown
	unset ac_cv_func_$1 2>/dev/null
	$4],
	[[$]$3])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_BROKEN version: 4 updated: 2012/11/08 20:57:52
dnl ----------------
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
test "$cf_cv_baddef_remove" != no && AC_DEFINE(NEED_REMOVE,1,[Define to 1 if we should use remove rather than unlink])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_DEFINE version: 3 updated: 2010/01/09 11:05:50
dnl ----------------
dnl Remove all -U and -D options that refer to the given symbol from a list
dnl of C compiler options.  This works around the problem that not all
dnl compilers process -U and -D options from left-to-right, so a -U option
dnl cannot be used to cancel the effect of a preceding -D option.
dnl
dnl $1 = target (which could be the same as the source variable)
dnl $2 = source (including '$')
dnl $3 = symbol to remove
define([CF_REMOVE_DEFINE],
[
$1=`echo "$2" | \
	sed	-e 's/-[[UD]]'"$3"'\(=[[^ 	]]*\)\?[[ 	]]/ /g' \
		-e 's/-[[UD]]'"$3"'\(=[[^ 	]]*\)\?[$]//g'`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RESTORE_XTRA_FLAGS version: 1 updated: 2020/01/11 16:47:45
dnl ---------------------
dnl Restore flags saved in CF_SAVE_XTRA_FLAGS
dnl $1 = name of current macro
define([CF_RESTORE_XTRA_FLAGS],
[
LIBS="$cf_save_LIBS_$1"
CFLAGS="$cf_save_CFLAGS_$1"
CPPFLAGS="$cf_save_CPPFLAGS_$1"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RPATH_HACK version: 13 updated: 2021/01/03 18:30:50
dnl -------------
AC_DEFUN([CF_RPATH_HACK],
[AC_REQUIRE([AC_PROG_FGREP])dnl
AC_REQUIRE([CF_LD_RPATH_OPT])dnl

AC_MSG_CHECKING(for updated LDFLAGS)
if test -n "$LD_RPATH_OPT" ; then
	AC_MSG_RESULT(maybe)

	AC_CHECK_PROGS(cf_ldd_prog,ldd,no)
	cf_rpath_list="/usr/lib /lib"
	if test "$cf_ldd_prog" != no
	then
		cf_rpath_oops=

AC_TRY_LINK([#include <stdio.h>],
		[printf("Hello");],
		[cf_rpath_oops=`"$cf_ldd_prog" "conftest$ac_exeext" | ${FGREP-fgrep} ' not found' | sed -e 's% =>.*$%%' |sort | uniq`
		 cf_rpath_list=`"$cf_ldd_prog" "conftest$ac_exeext" | ${FGREP-fgrep} / | sed -e 's%^.*[[ 	]]/%/%' -e 's%/[[^/]][[^/]]*$%%' |sort | uniq`])

		# If we passed the link-test, but get a "not found" on a given library,
		# this could be due to inept reconfiguration of gcc to make it only
		# partly honor /usr/local/lib (or whatever).  Sometimes this behavior
		# is intentional, e.g., installing gcc in /usr/bin and suppressing the
		# /usr/local libraries.
		if test -n "$cf_rpath_oops"
		then
			for cf_rpath_src in $cf_rpath_oops
			do
				for cf_rpath_dir in \
					/usr/local \
					/usr/pkg \
					/opt/sfw
				do
					if test -f "$cf_rpath_dir/lib/$cf_rpath_src"
					then
						CF_VERBOSE(...adding -L$cf_rpath_dir/lib to LDFLAGS for $cf_rpath_src)
						LDFLAGS="$LDFLAGS -L$cf_rpath_dir/lib"
						break
					fi
				done
			done
		fi
	fi

	CF_VERBOSE(...checking EXTRA_LDFLAGS $EXTRA_LDFLAGS)

	CF_RPATH_HACK_2(LDFLAGS)
	CF_RPATH_HACK_2(LIBS)

	CF_VERBOSE(...checked EXTRA_LDFLAGS $EXTRA_LDFLAGS)
else
	AC_MSG_RESULT(no)
fi
AC_SUBST(EXTRA_LDFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RPATH_HACK_2 version: 8 updated: 2021/01/01 13:31:04
dnl ---------------
dnl Do one set of substitutions for CF_RPATH_HACK, adding an rpath option to
dnl EXTRA_LDFLAGS for each -L option found.
dnl
dnl $cf_rpath_list contains a list of directories to ignore.
dnl
dnl $1 = variable name to update.  The LDFLAGS variable should be the only one,
dnl      but LIBS often has misplaced -L options.
AC_DEFUN([CF_RPATH_HACK_2],
[
CF_VERBOSE(...checking $1 [$]$1)

cf_rpath_dst=
for cf_rpath_src in [$]$1
do
	case "$cf_rpath_src" in
	(-L*)

		# check if this refers to a directory which we will ignore
		cf_rpath_skip=no
		if test -n "$cf_rpath_list"
		then
			for cf_rpath_item in $cf_rpath_list
			do
				if test "x$cf_rpath_src" = "x-L$cf_rpath_item"
				then
					cf_rpath_skip=yes
					break
				fi
			done
		fi

		if test "$cf_rpath_skip" = no
		then
			# transform the option
			if test "$LD_RPATH_OPT" = "-R " ; then
				cf_rpath_tmp=`echo "$cf_rpath_src" |sed -e "s%-L%-R %"`
			else
				cf_rpath_tmp=`echo "$cf_rpath_src" |sed -e "s%-L%$LD_RPATH_OPT%"`
			fi

			# if we have not already added this, add it now
			cf_rpath_tst=`echo "$EXTRA_LDFLAGS" | sed -e "s%$cf_rpath_tmp %%"`
			if test "x$cf_rpath_tst" = "x$EXTRA_LDFLAGS"
			then
				CF_VERBOSE(...Filter $cf_rpath_src ->$cf_rpath_tmp)
				EXTRA_LDFLAGS="$cf_rpath_tmp $EXTRA_LDFLAGS"
			fi
		fi
		;;
	esac
	cf_rpath_dst="$cf_rpath_dst $cf_rpath_src"
done
$1=$cf_rpath_dst

CF_VERBOSE(...checked $1 [$]$1)
AC_SUBST(EXTRA_LDFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SAVE_XTRA_FLAGS version: 1 updated: 2020/01/11 16:46:44
dnl ------------------
dnl Use this macro to save CFLAGS/CPPFLAGS/LIBS before checks against X headers
dnl and libraries which do not update those variables.
dnl
dnl $1 = name of current macro
define([CF_SAVE_XTRA_FLAGS],
[
cf_save_LIBS_$1="$LIBS"
cf_save_CFLAGS_$1="$CFLAGS"
cf_save_CPPFLAGS_$1="$CPPFLAGS"
LIBS="$LIBS ${X_PRE_LIBS} ${X_LIBS} ${X_EXTRA_LIBS}"
for cf_X_CFLAGS in $X_CFLAGS
do
	case "x$cf_X_CFLAGS" in
	x-[[IUD]]*)
		CPPFLAGS="$CPPFLAGS $cf_X_CFLAGS"
		;;
	*)
		CFLAGS="$CFLAGS $cf_X_CFLAGS"
		;;
	esac
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SET_ERRNO version: 6 updated: 2017/05/10 18:31:29
dnl ------------
dnl Check if 'errno' is declared in a fashion that lets us set it.
AC_DEFUN([CF_SET_ERRNO],
[
AC_CACHE_CHECK(if we can set errno,cf_cv_set_errno,[
AC_TRY_RUN([
#include <errno.h>
int main(void)
{
	errno = 255;
	${cf_cv_main_return:-return}(errno != 255);
}],
	[cf_cv_set_errno=yes],
	[cf_cv_set_errno=no],
	[AC_TRY_LINK(
		[#include <errno.h>],
		[errno = 255],
		[cf_cv_set_errno=maybe],
		[cf_cv_set_errno=no])])
])
test "$cf_cv_set_errno" != no && AC_DEFINE(CAN_SET_ERRNO,1,[Define to 1 if we can set errno])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIGWINCH version: 6 updated: 2021/01/01 13:31:04
dnl -----------
dnl Use this macro after CF_XOPEN_SOURCE, but do not require it (not all
dnl programs need this test).
dnl
dnl This is really a Mac OS X 10.4.3 workaround.  Defining _POSIX_C_SOURCE
dnl forces SIGWINCH to be undefined (breaks xterm, ncurses).  Oddly, the struct
dnl winsize declaration is left alone - we may revisit this if Apple choose to
dnl break that part of the interface as well.
AC_DEFUN([CF_SIGWINCH],
[
AC_CACHE_CHECK(if SIGWINCH is defined,cf_cv_define_sigwinch,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/signal.h>
],[int x = SIGWINCH; (void)x],
	[cf_cv_define_sigwinch=yes],
	[AC_TRY_COMPILE([
#undef _XOPEN_SOURCE
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/signal.h>
],[int x = SIGWINCH; (void)x],
	[cf_cv_define_sigwinch=maybe],
	[cf_cv_define_sigwinch=no])
])
])

if test "$cf_cv_define_sigwinch" = maybe ; then
AC_CACHE_CHECK(for actual SIGWINCH definition,cf_cv_fixup_sigwinch,[
cf_cv_fixup_sigwinch=unknown
cf_sigwinch=32
while test "$cf_sigwinch" != 1
do
	AC_TRY_COMPILE([
#undef _XOPEN_SOURCE
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/signal.h>
],[
#if SIGWINCH != $cf_sigwinch
make an error
#endif
int x = SIGWINCH; (void)x],
	[cf_cv_fixup_sigwinch=$cf_sigwinch
	 break])

cf_sigwinch="`expr "$cf_sigwinch" - 1`"
done
])

	if test "$cf_cv_fixup_sigwinch" != unknown ; then
		CPPFLAGS="$CPPFLAGS -DSIGWINCH=$cf_cv_fixup_sigwinch"
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIZECHANGE version: 17 updated: 2021/01/01 13:31:04
dnl -------------
dnl Check for definitions & structures needed for window size-changing
dnl
dnl https://stackoverflow.com/questions/18878141/difference-between-structures-ttysize-and-winsize/50769952#50769952
AC_DEFUN([CF_SIZECHANGE],
[
AC_REQUIRE([CF_STRUCT_TERMIOS])
AC_CACHE_CHECK(declaration of size-change, cf_cv_sizechange,[
	cf_cv_sizechange=unknown
	cf_save_CPPFLAGS="$CPPFLAGS"

for cf_opts in "" "NEED_PTEM_H"
do

	CPPFLAGS="$cf_save_CPPFLAGS"
	if test -n "$cf_opts"
	then
		CF_APPEND_TEXT(CPPFLAGS,-D$cf_opts)
	fi
	AC_TRY_COMPILE([#include <sys/types.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#else
#ifdef HAVE_TERMIO_H
#include <termio.h>
#endif
#endif

#ifdef NEED_PTEM_H
/* This is a workaround for SCO:  they neglected to define struct winsize in
 * termios.h -- it's only in termio.h and ptem.h
 */
#include <sys/stream.h>
#include <sys/ptem.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
],[
#ifdef TIOCGSIZE
	struct ttysize win;	/* SunOS 3.0... */
	int y = win.ts_lines = 2;
	int x = win.ts_cols = 1;
	(void)y;
	(void)x;
#else
#ifdef TIOCGWINSZ
	struct winsize win;	/* everything else */
	int y = win.ws_row = 2;
	int x = win.ws_col = 1;
	(void)y;
	(void)x;
#else
	no TIOCGSIZE or TIOCGWINSZ
#endif /* TIOCGWINSZ */
#endif /* TIOCGSIZE */
	],
	[cf_cv_sizechange=yes],
	[cf_cv_sizechange=no])

	CPPFLAGS="$cf_save_CPPFLAGS"
	if test "$cf_cv_sizechange" = yes ; then
		echo "size-change succeeded ($cf_opts)" >&AC_FD_CC
		test -n "$cf_opts" && cf_cv_sizechange="$cf_opts"
		break
	fi
done
])
if test "$cf_cv_sizechange" != no ; then
	AC_DEFINE(HAVE_SIZECHANGE,1,[Define to 1 if sizechange declarations are provided])
	case "$cf_cv_sizechange" in
	(NEED*)
		AC_DEFINE_UNQUOTED($cf_cv_sizechange )
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SLANG_CPPFLAGS version: 13 updated: 2021/01/04 17:48:08
dnl -----------------
dnl Look for the slang header files in the standard places, adjusting the
dnl CPPFLAGS variable.
dnl
dnl $1 = parameter to search for "slang2" class, e.g., for pkgsrc.
AC_DEFUN([CF_SLANG_CPPFLAGS],
[AC_REQUIRE([AC_PROG_EGREP])dnl

AC_CACHE_CHECK(for $1 header file,cf_cv_$1_header,[
	cf_cv_$1_header=no
	AC_TRY_COMPILE([#include <slang.h>],
	[printf("%s\\n", SLANG_VERSION)],
	[cf_cv_$1_header=predefined],[
	CF_HEADER_PATH(cf_search,$1)
	for cf_incdir in $cf_search
	do
		for cf_header in \
			slang.h
		do
			echo trying "$cf_incdir/$cf_header" 1>&AC_FD_CC
			if ${EGREP-egrep} "SLANG_VERSION" "$cf_incdir/$cf_header" 1>&AC_FD_CC 2>&1; then
				cf_cv_$1_header=$cf_incdir/$cf_header
				break
			fi
		done
		test "$cf_cv_$1_header" != no && break
	done
	])])

if test "x$cf_cv_$1_header" != xno
then
	AC_DEFINE(USE_SLANG,1,[Define to 1 if we should use s-lang library])

	CF_DIRNAME(cf_incdir,$cf_cv_$1_header)

	case "$cf_cv_$1_header" in
	(predefined)
		;;
	(*)
		CF_ADD_INCDIR($cf_incdir)
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SLANG_LIBS version: 10 updated: 2021/01/02 09:31:20
dnl -------------
dnl Look for the slang library.
dnl
dnl $1 = the actual library name, usually "slang"
AC_DEFUN([CF_SLANG_LIBS],
[
cf_slang_LIBS1="$LIBS"
CF_TERMCAP_LIBS
cf_slang_LIBS2="$LIBS"
AC_CHECK_FUNC(acos,,[CF_RECHECK_FUNC(acos,m,LIBS)])
case "$host_os" in
(os2*)
	CF_FIND_LIBRARY(video,video,
		[#include <sys/video.h>],
		[v_init()],
		v_init)
	;;
esac
CF_FIND_LIBRARY($1,$1,
	[#include <slang.h>],
	[SLtt_get_screen_size()],
	SLtt_get_screen_size)
cf_slang_LIBS3="$LIBS"
AC_MSG_CHECKING(if we can link $1 without termcap)
if test -n "`echo "$cf_slang_LIBS1" | sed -e 's/ //g'`" ; then
	cf_exclude=`echo ".$cf_slang_LIBS2" | sed -e "s%$cf_slang_LIBS1%%" -e 's%^.%%'`
else
	cf_exclude="$cf_slang_LIBS2"
fi
LIBS=`echo ".$cf_slang_LIBS3" | sed -e "s%$cf_exclude%%" -e 's%^.%%'`
AC_TRY_LINK([#include <slang.h>],
	[SLtt_get_screen_size()],
	[cf_result=yes],
	[cf_result=no])
AC_MSG_RESULT($cf_result)
test "$cf_result" = no && LIBS="$cf_slang_LIBS3"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SLANG_UNIX_DEFS version: 7 updated: 2021/01/02 09:31:20
dnl ------------------
dnl Slang's header files rely on some predefined symbols to declare variables
dnl that we might find useful.  This check is needed, because those symbols
dnl are generally not available.
AC_DEFUN([CF_SLANG_UNIX_DEFS],
[

CF_SLANG_CPPFLAGS(slang)
if test "x$cf_cv_slang_header" != xno
then
	CF_SLANG_LIBS(slang)
else
	CF_SLANG_CPPFLAGS(slang2)
	if test "x$cf_cv_slang2_header" != xno
	then
		CF_SLANG_LIBS(slang2)
	else
		AC_MSG_ERROR(cannot find slang headers)
	fi
fi

# There's an unofficial set of patches for slang that gives it some limited
# UTF8 capability.  Unfortunately it won't compile unless one defines UTF8.
AC_CACHE_CHECK(if we must define UTF8,cf_cv_slang_utf8,[
	AC_TRY_COMPILE([#include <slang.h>],
	[SLtt_get_screen_size()],
	[cf_cv_slang_utf8=no],
	[
	AC_TRY_COMPILE([
#define UTF8
#include <slang.h>],
	[SLtt_get_screen_size()],
	[cf_cv_slang_utf8=yes],
	[cf_cv_slang_utf8=unknown])])
])

if test "$cf_cv_slang_utf8" = yes ; then
	AC_DEFINE(UTF8,1,[Define to 1 if we must define UTF-8 for s-lang headers])
fi

AC_CACHE_CHECK(if we must tell slang this is UNIX,cf_cv_slang_unix,[
AC_TRY_LINK([#include <slang.h>],
	[
#ifdef REAL_UNIX_SYSTEM
make an error
#else
extern int SLang_TT_Baud_Rate;
SLang_TT_Baud_Rate = 1
#endif
],
	[cf_cv_slang_unix=yes],
	[cf_cv_slang_unix=no])
])
test "$cf_cv_slang_unix" = yes && AC_DEFINE(REAL_UNIX_SYSTEM,1,[Define to 1 if we must define REAL_UNIX_SYSTEM to compile with s-lang headers])

CF_CHECK_TYPE(SLsmg_Color_Type,int,[#include <slang.h>])
CF_CHECK_TYPE(SLtt_Char_Type,unsigned long,[#include <slang.h>])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SOCKS version: 9 updated: 2012/11/08 20:57:52
dnl --------
dnl Check for socks library
dnl $1 = the [optional] directory in which the library may be found
AC_DEFUN([CF_SOCKS],[
  CF_ADD_OPTIONAL_PATH($1, [socks library])
  CF_FIND_LINKAGE([
#include <stdio.h>
],[
      Raccept((char *)0)
],
      socks)

  if test "x$cf_cv_find_linkage_socks" = "xyes" ; then
    AC_DEFINE(SOCKS,1,[Define to 1 if we are using socks library])

    AC_DEFINE(accept,Raccept,[Define to override function name if using socks library])
    AC_DEFINE(bind,Rbind,[Define to override function name if using socks library])
    AC_DEFINE(connect,Rconnect,[Define to override function name if using socks library])
    AC_DEFINE(getpeername,Rgetpeername,[Define to override function name if using socks library])
    AC_DEFINE(getsockname,Rgetsockname,[Define to override function name if using socks library])
    AC_DEFINE(listen,Rlisten,[Define to override function name if using socks library])
    AC_DEFINE(recvfrom,Rrecvfrom,[Define to override function name if using socks library])
    AC_DEFINE(select,Rselect,[Define to override function name if using socks library])
  else
    AC_MSG_ERROR(cannot link with socks library)
  fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SOCKS5 version: 12 updated: 2012/11/08 20:57:52
dnl ---------
dnl Check for socks5 configuration
dnl $1 = the [optional] directory in which the library may be found
AC_DEFUN([CF_SOCKS5],[
  CF_ADD_OPTIONAL_PATH($1, [socks5 library])

CF_ADD_LIBS(-lsocks5)

AC_DEFINE(USE_SOCKS5,1,[Define to 1 if we are using socks5 library])
AC_DEFINE(SOCKS,1,[Define to 1 if we are using socks library])

AC_MSG_CHECKING(if the socks library uses socks4 prefix)
cf_use_socks4=error
AC_TRY_LINK([
#include <socks.h>],[
	Rinit((char *)0)],
	[AC_DEFINE(USE_SOCKS4_PREFIX,1,[Define to 1 if socks library uses socks4 prefix])
	 cf_use_socks4=yes],
	[AC_TRY_LINK([#include <socks.h>],
		[SOCKSinit((char *)0)],
		[cf_use_socks4=no],
		[AC_MSG_ERROR(Cannot link with socks5 library)])])
AC_MSG_RESULT($cf_use_socks4)

if test "$cf_use_socks4" = "yes" ; then
	AC_DEFINE(accept,Raccept)
	AC_DEFINE(bind,Rbind)
	AC_DEFINE(connect,Rconnect)
	AC_DEFINE(getpeername,Rgetpeername)
	AC_DEFINE(getsockname,Rgetsockname)
	AC_DEFINE(listen,Rlisten)
	AC_DEFINE(recvfrom,Rrecvfrom)
	AC_DEFINE(select,Rselect)
else
	AC_DEFINE(accept,SOCKSaccept)
	AC_DEFINE(getpeername,SOCKSgetpeername)
	AC_DEFINE(getsockname,SOCKSgetsockname)
	AC_DEFINE(recvfrom,SOCKSrecvfrom)
fi

AC_MSG_CHECKING(if socks5p.h is available)
AC_TRY_COMPILE([
#define INCLUDE_PROTOTYPES
#include <socks.h>],[
	init((char *)0)],
	[cf_use_socks5p_h=yes],
	[cf_use_socks5p_h=no])
AC_MSG_RESULT($cf_use_socks5p_h)

test "$cf_use_socks5p_h" = yes && AC_DEFINE(INCLUDE_PROTOTYPES,1,[Define to 1 if needed to declare prototypes in socks headers])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SRAND version: 16 updated: 2021/06/08 18:08:14
dnl --------
dnl Check for functions similar to srand() and rand().  lrand48() and random()
dnl return a 31-bit value, while rand() returns a value less than RAND_MAX
dnl which usually is only 16-bits.
dnl
dnl On MirOS, use arc4random_push() and arc4random().
dnl Some systems support an asymmetric variation of this interface.
dnl
dnl $1 = optional prefix for resulting shell variables.  The default "my_"
dnl      gives $my_srand and $my_rand to the caller, as well as MY_RAND_MAX.
dnl      These are all AC_SUBST'd and AC_DEFINE'd.
AC_DEFUN([CF_SRAND],[
AC_CACHE_CHECK(for random-integer functions, cf_cv_srand_func,[
cf_cv_srand_func=unknown
for cf_func in arc4random_push/arc4random arc4random_stir/arc4random srandom/random srand48/lrand48 srand/rand
do
	CF_SRAND_PARSE($cf_func,cf_srand_func,cf_rand_func)

AC_TRY_LINK([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
],[long seed = 1; $cf_srand_func(seed); seed = $cf_rand_func(); (void)seed],
[cf_cv_srand_func=$cf_func
 break])
done
])
if test "$cf_cv_srand_func" != unknown ; then
	AC_CACHE_CHECK(for range of random-integers, cf_cv_rand_max,[
		case "$cf_cv_srand_func" in
		(srand/rand)
			cf_cv_rand_max=RAND_MAX
			cf_rand_max=16
			;;
		(*/arc4random)
			cf_cv_rand_max=0xFFFFFFFFUL
			cf_rand_max=32
			;;
		(*)
			cf_cv_rand_max=INT_MAX
			cf_rand_max=31
			;;
		esac
		AC_TRY_COMPILE([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
		],[long x = $cf_cv_rand_max; (void)x],,
		[cf_cv_rand_max="(1UL<<$cf_rand_max)-1"])
	])

	case "$cf_cv_srand_func" in
	(*/arc4random)
		AC_MSG_CHECKING(if <bsd/stdlib.h> should be included)
		AC_TRY_COMPILE([#include <bsd/stdlib.h>],
					   [void *arc4random(int);
						void *x = arc4random(1); (void)x],
					   [cf_bsd_stdlib_h=no],
					   [AC_TRY_COMPILE([#include <bsd/stdlib.h>],
									   [unsigned x = arc4random(); (void)x],
									   [cf_bsd_stdlib_h=yes],
									   [cf_bsd_stdlib_h=no])])
	    AC_MSG_RESULT($cf_bsd_stdlib_h)
		if test "$cf_bsd_stdlib_h" = yes
		then
			AC_DEFINE(HAVE_BSD_STDLIB_H,1,[Define to 1 if bsd/stdlib.h header should be used])
		else
			AC_MSG_CHECKING(if <bsd/random.h> should be included)
			AC_TRY_COMPILE([#include <bsd/random.h>],
						   [void *arc4random(int);
							void *x = arc4random(1); (void)x],
						   [cf_bsd_random_h=no],
						   [AC_TRY_COMPILE([#include <bsd/random.h>],
										   [unsigned x = arc4random(); (void)x],
										   [cf_bsd_random_h=yes],
										   [cf_bsd_random_h=no])])
			AC_MSG_RESULT($cf_bsd_random_h)
			if test "$cf_bsd_random_h" = yes
			then
				AC_DEFINE(HAVE_BSD_RANDOM_H,1,[Define to 1 if bsd/random.h header should be used])
			else
				AC_MSG_WARN(no header file found for arc4random)
			fi
		fi
		;;
	esac

	CF_SRAND_PARSE($cf_func,cf_srand_func,cf_rand_func)

	CF_UPPER(cf_rand_max,ifelse($1,,my_,$1)rand_max)
	AC_DEFINE_UNQUOTED(ifelse($1,,my_,$1)srand,$cf_srand_func,[Define to the name for the srand function])
	AC_DEFINE_UNQUOTED(ifelse($1,,my_,$1)rand, $cf_rand_func,[Define to the name for the rand function])
	AC_DEFINE_UNQUOTED([$]cf_rand_max, $cf_cv_rand_max,[Define to the name for the RAND_MAX constant])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SRAND_PARSE version: 2 updated: 2015/04/15 19:08:48
dnl --------------
dnl Parse the loop variable for CF_SRAND, with a workaround for asymmetric
dnl variations.
define([CF_SRAND_PARSE],[
	$2=`echo $1 | sed -e 's%/.*%%'`
	$3=`echo $1 | sed -e 's%.*/%%'`

	case [$]$2 in
	(arc4random_stir)
		$2='(void)'
		;;
	esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SSL version: 26 updated: 2021/01/02 09:31:20
dnl ------
dnl Check for ssl library
dnl $1 = [optional] directory in which the library may be found, set by AC_ARG_WITH
AC_DEFUN([CF_SSL],[
	AC_REQUIRE([CF_PKG_CONFIG])
	cf_cv_have_ssl=no
	cf_cv_pkg_config_ssl=no

	# command-line option, etc., override default behavior
	CF_ADD_OPTIONAL_PATH($1, [ssl library])

	if test "x$PKG_CONFIG" != xnone; then
		case "$1" in
		(no)
			;;
		(yes) # if no explicit directory given, try pkg-config
			cf_cv_pkg_ssl=
			for cf_try_package in openssl libssl
			do
				AC_MSG_CHECKING(pkg-config for $cf_try_package)
				if "$PKG_CONFIG" --exists $cf_try_package ; then
					cf_cv_pkg_ssl=$cf_try_package
					AC_MSG_RESULT(yes)
					break
				else
					AC_MSG_RESULT(no)
				fi
			done
			if test -n "$cf_cv_pkg_ssl" ; then
				cf_cv_have_ssl=yes
				cf_cv_pkg_config_ssl=yes

				cf_cflags_ssl=`$PKG_CONFIG --cflags $cf_cv_pkg_ssl`
				cf_libs_ssl=`$PKG_CONFIG --libs $cf_cv_pkg_ssl`

				if test -n "$cf_cflags_ssl" ; then
					case "$cf_cflags_ssl" in
					(*-I*)
						cf_cv_header_path_ssl=`echo "$cf_cflags_ssl" | sed -e 's/^.*-I//' -e 's/ .*//'`
						;;
					(*)
						cf_cv_header_path_ssl=/usr/include
						;;
					esac
					CF_ADD_CFLAGS($cf_cflags_ssl)

					# workaround for broken openssl package using kerberos
					case "$cf_cflags_ssl" in
					(*kerberos*)
						cf_cv_have_ssl=maybe
						;;
					esac
				else
					cf_cv_header_path_ssl=/usr/include
				fi

				if test -d "$cf_cv_header_path_ssl/openssl" ; then
					cf_cv_header_path_ssl=$cf_cv_header_path_ssl/openssl
				fi

				if test -n "$cf_libs_ssl" ; then
					case "x$cf_libs_ssl" in
					(*-L*)
						cf_cv_library_path_ssl=`echo "$cf_libs_ssl" | sed -e 's/^.*-L//' -e 's/ .*//'`
						;;
					(*)
						cf_cv_library_path_ssl=/usr/lib
						;;
					esac
					CF_VERBOSE(adding $cf_libs_ssl to LIBS)
					CF_ADD_LIBS($cf_libs_ssl)
				fi
			fi
			;;
		esac
	fi

	if test "$cf_cv_have_ssl" != yes; then
		case "$host_os" in
		(mingw*)
			cf_extra_ssl_libs="-lcrypto -lgdi32"
			;;
		(*)
			# openssl 0.9.6 and up use dynamic loading for engines.
			cf_extra_ssl_libs="-lcrypto"
			case "x$LIBS" in
			(*-ldl)
				;;
			(*)
				AC_CHECK_LIB(dl,dlsym,
					[cf_extra_ssl_libs="$cf_extra_ssl_libs -ldl"])
				;;
			esac
			;;
		esac

		CF_FIND_LINKAGE(CF__SSL_HEAD,
			CF__SSL_BODY,
			ssl,
			cf_cv_have_ssl=yes,
			cf_cv_have_ssl=no,
			openssl,
			$cf_extra_ssl_libs)

		if test "$cf_cv_pkg_config_ssl" != yes ; then
			if test "$cf_cv_have_ssl" = yes ; then
				if test -n "$cf_cv_library_path_ssl" ; then
					CF_ADD_LIBDIR($cf_cv_library_path_ssl)
				fi
				CF_ADD_LIBS(-lssl $cf_extra_ssl_libs)
				if test -n "$cf_cv_header_path_ssl" ; then
					case "$cf_cv_header_path_ssl" in
					(/usr/include/openssl)
						;;
					(*)
						CF_ADD_INCDIR($cf_cv_header_path_ssl)
						;;
					esac
				fi
			fi
		fi
	fi

	if test "$cf_cv_have_ssl" = yes ; then
		AC_DEFINE(USE_SSL,1,[Define to 1 if we are using SSL])
		if test -n "$cf_cv_header_path_ssl" ; then
			CF_VERBOSE(checking ssl header-path $cf_cv_header_path_ssl)
			case "$cf_cv_header_path_ssl" in
			(*/openssl)
				AC_DEFINE(USE_OPENSSL_INCL,1,[Define to 1 if we are using OpenSSL headers])
				;;
			esac
		fi
		CF_CHECK_SSL_X509
	fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRIP_G_OPT version: 4 updated: 2021/01/02 09:31:20
dnl --------------
dnl	Remove "-g" option from the compiler options
AC_DEFUN([CF_STRIP_G_OPT],
[$1=`echo "${$1}" | CF__SED_TRIMBLANKS(-e 's%-g %%' -e 's%-g$%%')`])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRIP_O_OPT version: 4 updated: 2021/01/02 09:31:20
dnl --------------
dnl	Remove "-O" option from the compiler options
AC_DEFUN([CF_STRIP_O_OPT],[
$1=`echo "${$1}" | CF__SED_TRIMBLANKS(-e 's%-O[[1-9]]\? %%' -e 's%-O[[1-9]]\?$%%')`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRUCT_TERMIOS version: 11 updated: 2020/03/19 20:46:13
dnl -----------------
dnl Some machines require _POSIX_SOURCE to completely define struct termios.
AC_DEFUN([CF_STRUCT_TERMIOS],[
AC_REQUIRE([CF_XOPEN_SOURCE])

AC_CHECK_HEADERS( \
termio.h \
termios.h \
unistd.h \
sys/ioctl.h \
sys/termio.h \
)

if test "$ac_cv_header_termios_h" = yes ; then
	case "$CFLAGS $CPPFLAGS" in
	(*-D_POSIX_SOURCE*)
		termios_bad=dunno ;;
	(*)	termios_bad=maybe ;;
	esac
	if test "$termios_bad" = maybe ; then
	AC_MSG_CHECKING(whether termios.h needs _POSIX_SOURCE)
	AC_TRY_COMPILE([#include <termios.h>],
		[struct termios foo; int x = foo.c_iflag = 1; (void)x],
		termios_bad=no, [
		AC_TRY_COMPILE([
#define _POSIX_SOURCE
#include <termios.h>],
			[struct termios foo; int x = foo.c_iflag = 2; (void)x],
			termios_bad=unknown,
			termios_bad=yes AC_DEFINE(_POSIX_SOURCE,1,[Define to 1 if we must define _POSIX_SOURCE]))
			])
	AC_MSG_RESULT($termios_bad)
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SUBDIR_PATH version: 7 updated: 2014/12/04 04:33:06
dnl --------------
dnl Construct a search-list for a nonstandard header/lib-file
dnl	$1 = the variable to return as result
dnl	$2 = the package name
dnl	$3 = the subdirectory, e.g., bin, include or lib
AC_DEFUN([CF_SUBDIR_PATH],
[
$1=

CF_ADD_SUBDIR_PATH($1,$2,$3,$prefix,NONE)

for cf_subdir_prefix in \
	/usr \
	/usr/local \
	/usr/pkg \
	/opt \
	/opt/local \
	[$]HOME
do
	CF_ADD_SUBDIR_PATH($1,$2,$3,$cf_subdir_prefix,$prefix)
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYSTEM_MAIL_FLAGS version: 6 updated: 2021/01/02 09:31:20
dnl --------------------
AC_DEFUN([CF_SYSTEM_MAIL_FLAGS],
[
AC_MSG_CHECKING([system mail flags])
AC_CACHE_VAL(cf_cv_system_mail_flags,[
	case "$cf_cv_SYSTEM_MAIL" in
	(*/mmdf/*)
		[cf_cv_system_mail_flags="-mlruxto,cc\\\\*"]
        	;;
	(*)
        	[cf_cv_system_mail_flags="-t -oi"]
	esac
	])
AC_MSG_RESULT($cf_cv_system_mail_flags)
AC_DEFINE_UNQUOTED(SYSTEM_MAIL_FLAGS, "$cf_cv_system_mail_flags",[Define to value used for system mail-flags])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYSV_UTMP version: 7 updated: 2021/01/02 09:31:20
dnl ------------
dnl Check if this is a SYSV flavor of UTMP
AC_DEFUN([CF_SYSV_UTMP],
[
AC_CACHE_CHECK(if $cf_cv_have_utmp is SYSV flavor,cf_cv_sysv_utmp,[
test "$cf_cv_have_utmp" = "utmp" && cf_prefix="ut" || cf_prefix="utx"
AC_TRY_LINK([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],[
struct $cf_cv_have_utmp x;
	set${cf_prefix}ent ();
	get${cf_prefix}id(&x);
	put${cf_prefix}line(&x);
	end${cf_prefix}ent();],
	[cf_cv_sysv_utmp=yes],
	[cf_cv_sysv_utmp=no])
])
test "$cf_cv_sysv_utmp" = yes && AC_DEFINE(USE_SYSV_UTMP,1,[Define to 1 if utmp is SYSV flavor])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYS_ERRLIST version: 6 updated: 2001/12/30 13:03:23
dnl --------------
dnl Check for declaration of sys_nerr and sys_errlist in one of stdio.h and
dnl errno.h.  Declaration of sys_errlist on BSD4.4 interferes with our
dnl declaration.  Reported by Keith Bostic.
AC_DEFUN([CF_SYS_ERRLIST],
[
    CF_CHECK_ERRNO(sys_nerr)
    CF_CHECK_ERRNO(sys_errlist)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TAR_OPTIONS version: 3 updated: 2015/04/15 19:08:48
dnl --------------
dnl This is just a list of the most common tar options, allowing for variants
dnl that can operate with the "-" standard input/output option.
AC_DEFUN([CF_TAR_OPTIONS],
[
case ifelse($1,,tar,$1) in
(*pax|*pax$EXEEXT)
	TAR_UP_OPTIONS="-w"
	TAR_DOWN_OPTIONS="-r"
	TAR_PIPE_OPTIONS=""
	TAR_FILE_OPTIONS="-f"
	;;
(*star|*star$EXEEXT)
	TAR_UP_OPTIONS="-c -f"
	TAR_DOWN_OPTIONS="-x -U -f"
	TAR_PIPE_OPTIONS="-"
	TAR_FILE_OPTIONS=""
	;;
(*tar|*tar$EXEEXT)
	# FIXME: some versions of tar require, some don't allow the "-"
	TAR_UP_OPTIONS="-cf"
	TAR_DOWN_OPTIONS="-xf"
	TAR_PIPE_OPTIONS="-"
	TAR_FILE_OPTIONS=""
	;;
esac

AC_SUBST(TAR_UP_OPTIONS)
AC_SUBST(TAR_DOWN_OPTIONS)
AC_SUBST(TAR_FILE_OPTIONS)
AC_SUBST(TAR_PIPE_OPTIONS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TERMCAP_LIBS version: 15 updated: 2015/04/15 19:08:48
dnl ---------------
dnl Look for termcap libraries, or the equivalent in terminfo.
dnl
dnl The optional parameter may be "ncurses", "ncursesw".
AC_DEFUN([CF_TERMCAP_LIBS],
[
AC_CACHE_VAL(cf_cv_termlib,[
cf_cv_termlib=none
AC_TRY_LINK([],[char *x=(char*)tgoto("",0,0)],
[AC_TRY_LINK([],[int x=tigetstr("")],
	[cf_cv_termlib=terminfo],
	[cf_cv_termlib=termcap])
	CF_VERBOSE(using functions in predefined $cf_cv_termlib LIBS)
],[
ifelse([$1],,,[
case "$1" in
(ncurses*)
	CF_NCURSES_CONFIG($1)
	cf_cv_termlib=terminfo
	;;
esac
])
if test "$cf_cv_termlib" = none; then
	# FreeBSD's linker gives bogus results for AC_CHECK_LIB, saying that
	# tgetstr lives in -lcurses when it is only an unsatisfied extern.
	cf_save_LIBS="$LIBS"
	for cf_lib in tinfo curses ncurses termlib termcap
	do
		LIBS="-l$cf_lib $cf_save_LIBS"
		for cf_func in tigetstr tgetstr
		do
			AC_MSG_CHECKING(for $cf_func in -l$cf_lib)
			AC_TRY_LINK([],[int x=$cf_func("")],[cf_result=yes],[cf_result=no])
			AC_MSG_RESULT($cf_result)
			if test "$cf_result" = yes ; then
				if test "$cf_func" = tigetstr ; then
					cf_cv_termlib=terminfo
				else
					cf_cv_termlib=termcap
				fi
				break
			fi
		done
		test "$cf_result" = yes && break
	done
	test "$cf_result" = no && LIBS="$cf_save_LIBS"
fi
if test "$cf_cv_termlib" = none; then
	# allow curses library for broken AIX system.
	AC_CHECK_LIB(curses, initscr, [CF_ADD_LIBS(-lcurses)])
	AC_CHECK_LIB(termcap, tgoto, [CF_ADD_LIBS(-ltermcap) cf_cv_termlib=termcap])
fi
])
if test "$cf_cv_termlib" = none; then
	AC_MSG_WARN([Cannot find -ltermlib, -lcurses, or -ltermcap])
fi
])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TERMIO_AND_CURSES version: 6 updated: 2021/01/02 09:31:20
dnl --------------------
dnl Check if including termio.h with <curses.h> dies like on sysv68
dnl FIXME: this is too Lynx-specific
AC_DEFUN([CF_TERMIO_AND_CURSES],
[
AC_CACHE_CHECK(if we can include termio.h with curses,cf_cv_termio_and_curses,[
    cf_save_CFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -DHAVE_CONFIG_H -I. -I${srcdir:-.} -I${srcdir:-.}/src -I${srcdir:-.}/WWW/Library/Implementation"
    touch lynx_cfg.h
    AC_TRY_COMPILE([
#include <$1>
#include <termio.h>],
    [putchar(0x0a)],
    [cf_cv_termio_and_curses=yes],
    [cf_cv_termio_and_curses=no])
    CPPFLAGS="$cf_save_CFLAGS"
    rm -f lynx_cfg.h
])

test "$cf_cv_termio_and_curses" = yes && AC_DEFINE(TERMIO_AND_CURSES,1,[Define to 1 if we can include termio.h with curses.h])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TERMIO_AND_TERMIOS version: 4 updated: 2021/01/02 09:31:20
dnl ---------------------
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
test "$cf_cv_termio_and_termios" = no && AC_DEFINE(TERMIO_AND_TERMIOS,1,[Define to 1 if we can include termio.h and termios.h])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TERM_HEADER version: 6 updated: 2021/01/02 09:31:20
dnl --------------
dnl Look for term.h, which is part of X/Open curses.  It defines the interface
dnl to terminfo database.  Usually it is in the same include-path as curses.h,
dnl but some packagers change this, breaking various applications.
AC_DEFUN([CF_TERM_HEADER],[
AC_CACHE_CHECK(for terminfo header, cf_cv_term_header,[
case "${cf_cv_ncurses_header}" in
(*/ncurses.h|*/ncursesw.h)
	cf_term_header=`echo "$cf_cv_ncurses_header" | sed -e 's%ncurses[[^.]]*\.h$%term.h%'`
	;;
(*)
	cf_term_header=term.h
	;;
esac

for cf_test in $cf_term_header "ncurses/term.h" "ncursesw/term.h"
do
AC_TRY_COMPILE([#include <stdio.h>
#include <${cf_cv_ncurses_header:-curses.h}>
#include <$cf_test>
],[int x = auto_left_margin; (void)x],[
	cf_cv_term_header="$cf_test"],[
	cf_cv_term_header=unknown
	])
	test "$cf_cv_term_header" != unknown && break
done
])

# Set definitions to allow ifdef'ing to accommodate subdirectories

case "$cf_cv_term_header" in
(*term.h)
	AC_DEFINE(HAVE_TERM_H,1,[Define to 1 if we have term.h])
	;;
esac

case "$cf_cv_term_header" in
(ncurses/term.h)
	AC_DEFINE(HAVE_NCURSES_TERM_H,1,[Define to 1 if we have ncurses/term.h])
	;;
(ncursesw/term.h)
	AC_DEFINE(HAVE_NCURSESW_TERM_H,1,[Define to 1 if we have ncursesw/term.h])
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TM_GMTOFF version: 5 updated: 2021/06/08 18:08:14
dnl ------------
dnl Check if the tm-struct defines the '.tm_gmtoff' member (useful in decoding
dnl dates).
AC_DEFUN([CF_TM_GMTOFF],
[
AC_MSG_CHECKING([for tm.tm_gmtoff])
AC_CACHE_VAL(cf_cv_tm_gmtoff,[
	AC_TRY_COMPILE([
#ifdef TIME_WITH_SYS_TIME
#	include <sys/time.h>
#	include <time.h>
#else
#	ifdef HAVE_SYS_TIME_H
#		include <sys/time.h>
#	else
#		include <time.h>
#	endif
#endif
],[
	static struct tm foo;
	long bar = foo.tm_gmtoff; (void) bar],
	[cf_cv_tm_gmtoff=yes],
	[cf_cv_tm_gmtoff=no])])
AC_MSG_RESULT($cf_cv_tm_gmtoff)
test "$cf_cv_tm_gmtoff" = no && AC_DEFINE(DONT_HAVE_TM_GMTOFF,1,[Define to 1 if the tm-struct defines .tm_gmtoff member])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TRIM_X_LIBS version: 3 updated: 2015/04/12 15:39:00
dnl --------------
dnl Trim extra base X libraries added as a workaround for inconsistent library
dnl dependencies returned by "new" pkg-config files.
AC_DEFUN([CF_TRIM_X_LIBS],[
	for cf_trim_lib in Xmu Xt X11
	do
		case "$LIBS" in
		(*-l$cf_trim_lib\ *-l$cf_trim_lib*)
			LIBS=`echo "$LIBS " | sed -e 's/  / /g' -e 's%-l'"$cf_trim_lib"' %%' -e 's/ $//'`
			CF_VERBOSE(..trimmed $LIBS)
			;;
		esac
	done
])
dnl ---------------------------------------------------------------------------
dnl CF_TRY_PKG_CONFIG version: 6 updated: 2020/12/31 10:54:15
dnl -----------------
dnl This is a simple wrapper to use for pkg-config, for libraries which may be
dnl available in that form.
dnl
dnl $1 = package name, which may be a shell variable
dnl $2 = extra logic to use, if any, after updating CFLAGS and LIBS
dnl $3 = logic to use if pkg-config does not have the package
AC_DEFUN([CF_TRY_PKG_CONFIG],[
AC_REQUIRE([CF_PKG_CONFIG])

if test "$PKG_CONFIG" != none && "$PKG_CONFIG" --exists "$1"; then
	CF_VERBOSE(found package $1)
	cf_pkgconfig_incs="`$PKG_CONFIG --cflags "$1" 2>/dev/null`"
	cf_pkgconfig_libs="`$PKG_CONFIG --libs   "$1" 2>/dev/null`"
	CF_VERBOSE(package $1 CFLAGS: $cf_pkgconfig_incs)
	CF_VERBOSE(package $1 LIBS: $cf_pkgconfig_libs)
	CF_ADD_CFLAGS($cf_pkgconfig_incs)
	CF_ADD_LIBS($cf_pkgconfig_libs)
	ifelse([$2],,:,[$2])
else
	cf_pkgconfig_incs=
	cf_pkgconfig_libs=
	ifelse([$3],,:,[$3])
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_TRY_RPATH version: 2 updated: 2021/01/02 09:31:20
dnl ------------
dnl Work around broken-by-design packaging systems which solve at most the
dnl problem of building packages, without providing for library reuse...
AC_DEFUN([CF_TRY_RPATH],[
AC_REQUIRE([CF_LD_RPATH_OPT])
if test -z "$LD_RPATH_OPT"
then
	CF_VERBOSE(will not attempt to use rpath)
elif test "x${enable_rpath_hack:-yes}" = "xno"
then
	CF_VERBOSE(rpath is disabled)
elif test -z "${LD_RUN_PATH}${LD_LIBRARY_PATH}"
then
	case "$LIBS" in
	(*-L/*)
		CF__CHECK_RUN(cf_check_run)
		if test "x$cf_check_run" = xno
		then
			CF_VERBOSE(linkage is broken)
			cf_result=
			for cf_item in $LIBS
			do
				case " $cf_item" in
				(\ -L/*)
					cf_rpath_arg=`echo " $cf_item" | sed -e "s% -L%$LD_RPATH_OPT%"`
					cf_rpath_tmp=`echo "$cf_result " | sed -e "s% $cf_rpath_arg % %"`
					if test "x$cf_result " = "x$cf_rpath_tmp"
					then
						cf_result="$cf_result $cf_rpath_arg"
					fi
					cf_rpath_tmp=`echo "$cf_result " | sed -e "s% $cf_item % %g"`
					if test "x$cf_result " != "x$cf_rpath_tmp"
					then
						continue
					fi
					;;
				esac
				cf_result="$cf_result $cf_item"
			done
			cf_result=`echo "$cf_result" | sed -e 's/^ //'`
			if test "x$cf_result" != "x$LIBS"
			then
				cf_save_LIBS="$LIBS"
				LIBS="$cf_result"
				CF__CHECK_RUN(cf_check_run)
				if test "x$cf_check_run" = "xyes"
				then
					CF_VERBOSE(use rpath for $cf_save_LIBS)
					LIBS="$cf_result"
					CF_VERBOSE(result is now $LIBS)
				else
					LIBS="$cf_save_LIBS"
				fi
			fi
		fi
		;;
	(*)
		CF_VERBOSE(will not attempt to use rpath)
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TRY_XOPEN_SOURCE version: 2 updated: 2018/06/20 20:23:13
dnl -------------------
dnl If _XOPEN_SOURCE is not defined in the compile environment, check if we
dnl can define it successfully.
AC_DEFUN([CF_TRY_XOPEN_SOURCE],[
AC_CACHE_CHECK(if we should define _XOPEN_SOURCE,cf_cv_xopen_source,[
	AC_TRY_COMPILE([
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_xopen_source=no],
	[cf_save="$CPPFLAGS"
	 CF_APPEND_TEXT(CPPFLAGS,-D_XOPEN_SOURCE=$cf_XOPEN_SOURCE)
	 AC_TRY_COMPILE([
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
],[
#ifdef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_xopen_source=no],
	[cf_cv_xopen_source=$cf_XOPEN_SOURCE])
	CPPFLAGS="$cf_save"
	])
])

if test "$cf_cv_xopen_source" != no ; then
	CF_REMOVE_DEFINE(CFLAGS,$CFLAGS,_XOPEN_SOURCE)
	CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,_XOPEN_SOURCE)
	cf_temp_xopen_source="-D_XOPEN_SOURCE=$cf_cv_xopen_source"
	CF_ADD_CFLAGS($cf_temp_xopen_source)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_TTYTYPE version: 7 updated: 2021/01/02 09:31:20
dnl ----------
AC_DEFUN([CF_TTYTYPE],
[
AC_MSG_CHECKING(if ttytype is declared in curses library)
AC_CACHE_VAL(cf_cv_have_ttytype,[
	AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
	[char *x = &ttytype[1]; *x = 1],
	[cf_cv_have_ttytype=yes],
	[cf_cv_have_ttytype=no])
	])
AC_MSG_RESULT($cf_cv_have_ttytype)
test "$cf_cv_have_ttytype" = yes && AC_DEFINE(HAVE_TTYTYPE,1,[Define to 1 if ttytype is declared in curses library])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TYPE_LONG_LONG version: 5 updated: 2021/01/04 17:48:08
dnl -----------------
dnl Check for long long type.
AC_DEFUN([CF_TYPE_LONG_LONG],[
AC_REQUIRE([AC_PROG_FGREP])dnl

AC_CACHE_CHECK(for long long type,cf_cv_type_long_long,[
	cat >conftest.$ac_ext <<_CFEOF
#include "confdefs.h"
int
main (void)
{
	long long foo = 123456789123456789LL;
	return (foo == 0);
}
_CFEOF
	(eval "[$]ac_compile") 2>conftest.err
	ac_status=$?
	if test "$ac_status" = 0 && test -s conftest.err
	then
		${FGREP-fgrep} warning conftest.err >/dev/null 2>&1 && ac_status=1
	fi
	if test "$ac_status" != 0
	then
		cf_cv_type_long_long=no
	else
		cf_cv_type_long_long=yes
	fi
	test -s conftest.err && cat conftest.err >&5
	rm -f conftest*
])

if test "$cf_cv_type_long_long" = yes ; then
	AC_DEFINE(HAVE_LONG_LONG,1,[Define to 1 if compiler supports long long type])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UNION_WAIT version: 9 updated: 2021/06/08 18:08:14
dnl -------------
dnl Check to see if the BSD-style union wait is declared.  Some platforms may
dnl use this, though it is deprecated in favor of the 'int' type in Posix.
dnl Some vendors provide a bogus implementation that declares union wait, but
dnl uses the 'int' type instead; we try to spot these by checking for the
dnl associated macros.
dnl
dnl Ahem.  Some implementers cast the status value to an int*, as an attempt to
dnl use the macros for either union wait or int.  So we do a check compile to
dnl see if the macros are defined and apply to an int.
dnl
dnl Sets: $cf_cv_type_unionwait
dnl Defines: HAVE_TYPE_UNIONWAIT
AC_DEFUN([CF_UNION_WAIT],
[
AC_REQUIRE([CF_WAIT_HEADERS])
AC_MSG_CHECKING([for union wait])
AC_CACHE_VAL(cf_cv_type_unionwait,[
	AC_TRY_LINK($cf_wait_headers,
	[static int x;
	 int y = WEXITSTATUS(x);
	 int z = WTERMSIG(x);
	 wait(&x);
	 (void)x;
	 (void)y;
	 (void)z;
	],
	[cf_cv_type_unionwait=no
	 echo compiles ok w/o union wait 1>&AC_FD_CC
	],[
	AC_TRY_LINK($cf_wait_headers,
	[union wait x;
#ifdef WEXITSTATUS
	 int y = WEXITSTATUS(x);
#endif
#ifdef WTERMSIG
	 int z = WTERMSIG(x);
#endif
	 wait(&x);
	 (void)x;
#ifdef WEXITSTATUS
	 (void)y;
#endif
#ifdef WTERMSIG
	 (void)z;
#endif
	],
	[cf_cv_type_unionwait=yes
	 echo compiles ok with union wait and possibly macros too 1>&AC_FD_CC
	],
	[cf_cv_type_unionwait=no])])])
AC_MSG_RESULT($cf_cv_type_unionwait)
test "$cf_cv_type_unionwait" = yes && AC_DEFINE(HAVE_TYPE_UNIONWAIT,1,[Define to 1 if type unionwait is declared])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UPPER version: 5 updated: 2001/01/29 23:40:59
dnl --------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
$1=`echo "$2" | sed y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTF8_LIB version: 9 updated: 2021/05/19 19:35:25
dnl -----------
dnl Check for multibyte support, and if not found, utf8 compatibility library
AC_DEFUN([CF_UTF8_LIB],
[
AC_HAVE_HEADERS(wchar.h)
AC_CACHE_CHECK(for multibyte character support,cf_cv_utf8_lib,[
	cf_save_LIBS="$LIBS"
	AC_TRY_LINK([
#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
],[putwc(0,0);],
	[cf_cv_utf8_lib=yes],
	[CF_FIND_LINKAGE([
#include <libutf8.h>],[putwc(0,0);],utf8,
		[cf_cv_utf8_lib=add-on],
		[cf_cv_utf8_lib=no])
])])

# HAVE_LIBUTF8_H is used by ncurses if curses.h is shared between
# ncurses/ncursesw:
if test "$cf_cv_utf8_lib" = "add-on" ; then
	AC_DEFINE(HAVE_LIBUTF8_H,1,[Define to 1 if we should include libutf8.h])
	CF_ADD_INCDIR($cf_cv_header_path_utf8)
	CF_ADD_LIBDIR($cf_cv_library_path_utf8)
	CF_ADD_LIBS($cf_cv_library_file_utf8)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP version: 12 updated: 2021/01/02 09:31:20
dnl -------
dnl Check for UTMP/UTMPX headers
AC_DEFUN([CF_UTMP],
[
AC_REQUIRE([CF_LASTLOG])

AC_CACHE_CHECK(for utmp implementation,cf_cv_have_utmp,[
	cf_cv_have_utmp=no
for cf_header in utmpx utmp ; do
cf_utmp_includes="
#include <sys/types.h>
#include <${cf_header}.h>
#define getutent getutxent
#ifdef USE_LASTLOG
#include <lastlog.h>	/* may conflict with utmpx.h on Linux */
#endif
"
	AC_TRY_COMPILE([$cf_utmp_includes],
	[struct $cf_header x;
	 char *name = x.ut_name; /* utmp.h and compatible definitions */
	 (void)x;
	 (void)name;
	],
	[cf_cv_have_utmp=$cf_header
	 break],
	[
	AC_TRY_COMPILE([$cf_utmp_includes],
	[struct $cf_header x;
	 char *name = x.ut_user; /* utmpx.h must declare this */
	 (void)x;
	 (void)name;
	],
	[cf_cv_have_utmp=$cf_header
	 break
	])])
done
])

if test "$cf_cv_have_utmp" != no ; then
	AC_DEFINE(HAVE_UTMP,1,[Define to 1 if the utmp interface is available])
	test "$cf_cv_have_utmp" = utmpx && AC_DEFINE(UTMPX_FOR_UTMP,1,[Define if we have utmpx interface])
	CF_UTMP_UT_HOST
	CF_UTMP_UT_SYSLEN
	CF_UTMP_UT_NAME
	CF_UTMP_UT_XSTATUS
	CF_UTMP_UT_XTIME
	CF_UTMP_UT_SESSION
	CF_SYSV_UTMP
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_HOST version: 10 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Check if UTMP/UTMPX struct defines ut_host member
AC_DEFUN([CF_UTMP_UT_HOST],
[
if test "$cf_cv_have_utmp" != no ; then
AC_MSG_CHECKING(if ${cf_cv_have_utmp}.ut_host is declared)
AC_CACHE_VAL(cf_cv_have_utmp_ut_host,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[struct $cf_cv_have_utmp x;
	 char *y = &x.ut_host[0];
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_host=yes],
	[cf_cv_have_utmp_ut_host=no])
	])
AC_MSG_RESULT($cf_cv_have_utmp_ut_host)
test "$cf_cv_have_utmp_ut_host" != no && AC_DEFINE(HAVE_UTMP_UT_HOST,1,[Define to 1 if UTMP/UTMPX struct defines ut_host member])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_NAME version: 8 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Check if UTMP/UTMPX struct defines ut_name member
AC_DEFUN([CF_UTMP_UT_NAME],
[
if test "$cf_cv_have_utmp" != no ; then
AC_CACHE_CHECK(if ${cf_cv_have_utmp}.ut_name is declared,cf_cv_have_utmp_ut_name,[
	cf_cv_have_utmp_ut_name=no
cf_utmp_includes="
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>
#define getutent getutxent
#ifdef USE_LASTLOG
#include <lastlog.h>		/* may conflict with utmpx.h on Linux */
#endif
"
for cf_header in ut_name ut_user ; do
	AC_TRY_COMPILE([$cf_utmp_includes],
	[struct $cf_cv_have_utmp x;
	 char *name = x.$cf_header;
	 (void)x;
	 (void)name;
	],
	[cf_cv_have_utmp_ut_name=$cf_header
	 break])
done
])

case "$cf_cv_have_utmp_ut_name" in
(no)
	AC_MSG_ERROR(Cannot find declaration for ut.ut_name)
	;;
(ut_user)
	AC_DEFINE(ut_name,ut_user,[Define to rename UTMP/UTMPX struct ut_name member])
	;;
esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_SESSION version: 9 updated: 2021/06/07 17:39:17
dnl ------------------
dnl Check if UTMP/UTMPX struct defines ut_session member
AC_DEFUN([CF_UTMP_UT_SESSION],
[
if test "$cf_cv_have_utmp" != no ; then
AC_CACHE_CHECK(if ${cf_cv_have_utmp}.ut_session is declared, cf_cv_have_utmp_ut_session,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[static struct $cf_cv_have_utmp x;
	 long y = x.ut_session;
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_session=yes],
	[cf_cv_have_utmp_ut_session=no])
])
if test "$cf_cv_have_utmp_ut_session" != no ; then
	AC_DEFINE(HAVE_UTMP_UT_SESSION,1,[Define to 1 if UTMP/UTMPX struct defines ut_session member])
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_SYSLEN version: 4 updated: 2021/01/02 09:31:20
dnl -----------------
dnl Check if UTMP/UTMPX struct defines ut_syslen member
AC_DEFUN([CF_UTMP_UT_SYSLEN],
[
if test "$cf_cv_have_utmp" != no ; then
AC_MSG_CHECKING(if ${cf_cv_have_utmp}.ut_syslen is declared)
AC_CACHE_VAL(cf_cv_have_utmp_ut_syslen,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[struct $cf_cv_have_utmp x;
	 int y = x.ut_syslen;
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_syslen=yes],
	[cf_cv_have_utmp_ut_syslen=no])
	])
AC_MSG_RESULT($cf_cv_have_utmp_ut_syslen)
test "$cf_cv_have_utmp_ut_syslen" != no && AC_DEFINE(HAVE_UTMP_UT_SYSLEN,1,[Define to 1 if UTMP/UTMPX struct defines ut_syslen member])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_XSTATUS version: 6 updated: 2021/01/02 09:31:20
dnl ------------------
dnl Check for known variants on the UTMP/UTMPX struct's exit-status as reported
dnl by various people:
dnl
dnl	ut_exit.__e_exit (HPUX 11 - David Ellement, also in glibc2)
dnl	ut_exit.e_exit (SVR4)
dnl	ut_exit.ut_e_exit (os390 - Greg Smith)
dnl	ut_exit.ut_exit (Tru64 4.0f - Jeremie Petit, 4.0e - Tomas Vanhala)
dnl
dnl Note: utmp_xstatus is not a conventional compatibility definition in the
dnl system header files.
AC_DEFUN([CF_UTMP_UT_XSTATUS],
[
if test "$cf_cv_have_utmp" != no ; then
AC_CACHE_CHECK(for exit-status in $cf_cv_have_utmp,cf_cv_have_utmp_ut_xstatus,[
for cf_result in \
	ut_exit.__e_exit \
	ut_exit.e_exit \
	ut_exit.ut_e_exit \
	ut_exit.ut_exit
do
AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[struct $cf_cv_have_utmp x;
	 long y = x.$cf_result = 0;
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_xstatus=$cf_result
	 break],
	[cf_cv_have_utmp_ut_xstatus=no])
done
])
if test "$cf_cv_have_utmp_ut_xstatus" != no ; then
	AC_DEFINE(HAVE_UTMP_UT_XSTATUS,1,[Define to 1 if UTMP/UTMPX has exit-status member])
	AC_DEFINE_UNQUOTED(ut_xstatus,$cf_cv_have_utmp_ut_xstatus,[Define if needed to rename member ut_xstatus of UTMP/UTMPX])
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTMP_UT_XTIME version: 11 updated: 2021/01/02 09:31:20
dnl ----------------
dnl Check if UTMP/UTMPX struct defines ut_xtime member
AC_DEFUN([CF_UTMP_UT_XTIME],
[
if test "$cf_cv_have_utmp" != no ; then
AC_CACHE_CHECK(if ${cf_cv_have_utmp}.ut_xtime is declared, cf_cv_have_utmp_ut_xtime,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[struct $cf_cv_have_utmp x;
	 long y = x.ut_xtime = 0;
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_xtime=yes],
	[AC_TRY_COMPILE([
#include <sys/types.h>
#include <${cf_cv_have_utmp}.h>],
	[struct $cf_cv_have_utmp x;
	 long y = x.ut_tv.tv_sec;
	 (void)x;
	 (void)y],
	[cf_cv_have_utmp_ut_xtime=define],
	[cf_cv_have_utmp_ut_xtime=no])
	])
])
if test "$cf_cv_have_utmp_ut_xtime" != no ; then
	AC_DEFINE(HAVE_UTMP_UT_XTIME,1,[Define to 1 if UTMP/UTMPX struct defines ut_xtime member])
	if test "$cf_cv_have_utmp_ut_xtime" = define ; then
		AC_DEFINE(ut_xtime,ut_tv.tv_sec,[Define if needed to alternate name for utmpx.ut_xtime member])
	fi
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_VERBOSE version: 3 updated: 2007/07/29 09:55:12
dnl ----------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
CF_MSG_LOG([$1])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WAIT_HEADERS version: 3 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Build up an expression $cf_wait_headers with the header files needed to
dnl compile against the prototypes for 'wait()', 'waitpid()', etc.  Assume it's
dnl Posix, which uses <sys/types.h> and <sys/wait.h>, but allow SVr4 variation
dnl with <wait.h>.
AC_DEFUN([CF_WAIT_HEADERS],
[
AC_HAVE_HEADERS(sys/wait.h)
cf_wait_headers="#include <sys/types.h>
"
if test "$ac_cv_header_sys_wait_h" = yes; then
cf_wait_headers="$cf_wait_headers
#include <sys/wait.h>
"
else
AC_HAVE_HEADERS(wait.h)
AC_HAVE_HEADERS(waitstatus.h)
if test "$ac_cv_header_wait_h" = yes; then
cf_wait_headers="$cf_wait_headers
#include <wait.h>
"
fi
if test "$ac_cv_header_waitstatus_h" = yes; then
cf_wait_headers="$cf_wait_headers
#include <waitstatus.h>
"
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WIDEC_CURSES version: 6 updated: 2020/03/19 20:23:48
dnl ---------------
dnl Check for curses implementations that can handle wide-characters
AC_DEFUN([CF_WIDEC_CURSES],
[
AC_CACHE_CHECK(if curses supports wide characters,cf_cv_widec_curses,[
AC_TRY_LINK([
#include <stdlib.h>
#include <${cf_cv_ncurses_header:-curses.h}>],[
    wchar_t temp[2];
    wchar_t wch = 'A';
    temp[0] = wch;
    waddnwstr(stdscr, temp, 1);
],
[cf_cv_widec_curses=yes],
[cf_cv_widec_curses=no])
])

if test "$cf_cv_widec_curses" = yes ; then
	AC_DEFINE(WIDEC_CURSES,1,[Define to 1 if curses supports wide characters])

	# This is needed on Tru64 5.0 to declare mbstate_t
	AC_CACHE_CHECK(if we must include wchar.h to declare mbstate_t,cf_cv_widec_mbstate,[
	AC_TRY_COMPILE([
#include <stdlib.h>
#include <${cf_cv_ncurses_header:-curses.h}>],
[mbstate_t state; (void)state],
[cf_cv_widec_mbstate=no],
[AC_TRY_COMPILE([
#include <stdlib.h>
#include <wchar.h>
#include <${cf_cv_ncurses_header:-curses.h}>],
[mbstate_t state; (void)state],
[cf_cv_widec_mbstate=yes],
[cf_cv_widec_mbstate=unknown])])])

if test "$cf_cv_widec_mbstate" = yes ; then
	AC_DEFINE(NEED_WCHAR_H,1,[Define to 1 if we must include wchar.h])
fi

if test "$cf_cv_widec_mbstate" != unknown ; then
	AC_DEFINE(HAVE_MBSTATE_T,1,[Define to 1 if we have mbstate_t type])
fi

fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_BZLIB version: 4 updated: 2007/07/29 13:19:54
dnl -------------
dnl Check for libbz2 aka "bzlib"
dnl
dnl $1 = optional path for headers/library
AC_DEFUN([CF_WITH_BZLIB],[
  CF_ADD_OPTIONAL_PATH($1)

  CF_FIND_LINKAGE([
#include <stdio.h>
#include <bzlib.h>
],[
    BZ2_bzopen("name","mode")
],bz2,,,bzlib)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_CURSES_DIR version: 4 updated: 2021/01/02 19:22:58
dnl ------------------
dnl Wrapper for AC_ARG_WITH to specify directory under which to look for curses
dnl libraries.
AC_DEFUN([CF_WITH_CURSES_DIR],[

AC_MSG_CHECKING(for specific curses-directory)
AC_ARG_WITH(curses-dir,
	[  --with-curses-dir=DIR   directory in which (n)curses is installed],
	[cf_cv_curses_dir=$withval],
	[cf_cv_curses_dir=no])
AC_MSG_RESULT($cf_cv_curses_dir)

if test -n "$cf_cv_curses_dir" && test "$cf_cv_curses_dir" != "no"
then
	CF_PATH_SYNTAX(withval)
	if test -d "$cf_cv_curses_dir"
	then
		CF_ADD_INCDIR($cf_cv_curses_dir/include)
		CF_ADD_LIBDIR($cf_cv_curses_dir/lib)
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DBMALLOC version: 7 updated: 2010/06/21 17:26:47
dnl ----------------
dnl Configure-option for dbmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DBMALLOC],[
CF_NO_LEAKS_OPTION(dbmalloc,
	[  --with-dbmalloc         test: use Conor Cahill's dbmalloc library],
	[USE_DBMALLOC])

if test "$with_dbmalloc" = yes ; then
	AC_CHECK_HEADER(dbmalloc.h,
		[AC_CHECK_LIB(dbmalloc,[debug_malloc]ifelse([$1],,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DMALLOC version: 7 updated: 2010/06/21 17:26:47
dnl ---------------
dnl Configure-option for dmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DMALLOC],[
CF_NO_LEAKS_OPTION(dmalloc,
	[  --with-dmalloc          test: use Gray Watson's dmalloc library],
	[USE_DMALLOC])

if test "$with_dmalloc" = yes ; then
	AC_CHECK_HEADER(dmalloc.h,
		[AC_CHECK_LIB(dmalloc,[dmalloc_debug]ifelse([$1],,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_IDNA version: 11 updated: 2021/07/05 09:09:42
dnl ------------
dnl Check for libidn2, use it if found.  Otherwise, check for libidn, use that.
dnl
dnl $1 = optional path for headers/library
AC_DEFUN([CF_WITH_IDNA],[
CF_ADD_OPTIONAL_PATH($1)

CF_FIND_LINKAGE([
#include <stdio.h>
#include <idn2.h>
],[
	char *output = 0;
	int code = idn2_to_ascii_8z("name", &output, IDN2_USE_STD3_ASCII_RULES);
	(void) code;
],idn2,,[CF_VERBOSE([unsuccessful, will try idn (older)])],,[$LIBICONV])

if test "x$cf_cv_find_linkage_idn2" = xyes ; then
	CF_VERBOSE(found idn2 library)
	AC_DEFINE(USE_IDN2,1,[Define to 1 if we should use IDN2 library])
else
	CF_FIND_LINKAGE([
#include <stdio.h>
#include <idna.h>
],[
	char *output = 0;
	int code = idna_to_ascii_8z("name", &output, IDNA_USE_STD3_ASCII_RULES);
	(void) code;
],idn,,,,[$LIBICONV])

if test "x$cf_cv_find_linkage_idn" = xyes ; then
	CF_VERBOSE(found idn library)
	AC_DEFINE(USE_IDNA,1,[Define to 1 if we should use IDNA library])
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_PATH version: 11 updated: 2012/09/29 15:04:19
dnl ------------
dnl Wrapper for AC_ARG_WITH to ensure that user supplies a pathname, not just
dnl defaulting to yes/no.
dnl
dnl $1 = option name
dnl $2 = help-text
dnl $3 = environment variable to set
dnl $4 = default value, shown in the help-message, must be a constant
dnl $5 = default value, if it's an expression & cannot be in the help-message
dnl
AC_DEFUN([CF_WITH_PATH],
[AC_ARG_WITH($1,[$2 ](default: ifelse([$4],,empty,[$4])),,
ifelse([$4],,[withval="${$3}"],[withval="${$3:-ifelse([$5],,[$4],[$5])}"]))dnl
if ifelse([$5],,true,[test -n "$5"]) ; then
CF_PATH_SYNTAX(withval)
fi
eval $3="$withval"
AC_SUBST($3)dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_SCREEN_PDCURSES version: 1 updated: 2020/08/28 16:56:27
dnl -----------------------
dnl Call this macro before CF_ENABLE_WARNINGS for configure scripts which use
dnl the "--with-screen=pdcurses" selection.  Doing that allows the configure
dnl script to search for the X11/Xt header files to declare (or not) the
dnl symbol needed to enable "const" in those header files.  If that configure
dnl option is not used, then those checks are unnecessary.
AC_DEFUN([CF_WITH_SCREEN_PDCURSES],[
AC_PROVIDE([AC_PATH_XTRA])
AC_PROVIDE([AC_PATH_X])
if test -n "$with_screen" && test "x$with_screen" = "xpdcurses"
then
	AC_PATH_X
	AC_PATH_XTRA
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_ZLIB version: 4 updated: 2011/05/28 12:10:58
dnl ------------
dnl check for libz aka "zlib"
AC_DEFUN([CF_WITH_ZLIB],[
  CF_ADD_OPTIONAL_PATH($1)

  CF_FIND_LINKAGE([
#include <zlib.h>
],[
	gzopen("name","mode")
],z,,,zlib)

AC_CHECK_FUNCS( \
	zError \
)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_XOPEN_CURSES version: 17 updated: 2021/07/10 12:22:27
dnl ---------------
dnl Test if we should define X/Open source for curses, needed on Digital Unix
dnl 4.x, to see the extended functions, but breaks on IRIX 6.x.
dnl
dnl The getbegyx() check is needed for HPUX, which omits legacy macros such
dnl as getbegy().  The latter is better design, but the former is standard.
AC_DEFUN([CF_XOPEN_CURSES],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
AC_CACHE_CHECK(definition to turn on extended curses functions,cf_cv_need_xopen_extension,[
cf_cv_need_xopen_extension=unknown
AC_TRY_LINK([
#include <stdlib.h>
#include <${cf_cv_ncurses_header:-curses.h}>],[
#if defined(NCURSES_VERSION_PATCH)
#if (NCURSES_VERSION_PATCH < 20100501) && (NCURSES_VERSION_PATCH >= 20100403)
	make an error
#endif
#endif
#ifdef NCURSES_WIDECHAR
make an error	/* prefer to fall-through on the second checks */
#endif
	cchar_t check;
	int check2 = curs_set((int)sizeof(check));
	long x = winnstr(stdscr, "", 0);
	int x1, y1;
	(void)check2;
	getbegyx(stdscr, y1, x1);
	(void)x;
	(void)y1;
	(void)x1;
	],
	[cf_cv_need_xopen_extension=none],
	[
	for cf_try_xopen_extension in _XOPEN_SOURCE_EXTENDED NCURSES_WIDECHAR
	do
		AC_TRY_LINK([
#define $cf_try_xopen_extension 1
#include <stdlib.h>
#include <${cf_cv_ncurses_header:-curses.h}>],[
		cchar_t check;
		int check2 = curs_set((int)sizeof(check));
		long x = winnstr(stdscr, "", 0);
		int x1, y1;
		getbegyx(stdscr, y1, x1);
		(void)check2;
		(void)x;
		(void)y1;
		(void)x1;
		],
		[cf_cv_need_xopen_extension=$cf_try_xopen_extension; break])
	done
	])
])

case "$cf_cv_need_xopen_extension" in
(*_*)
	CF_APPEND_TEXT(CPPFLAGS,-D$cf_cv_need_xopen_extension)
	;;
esac

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_XOPEN_SOURCE version: 58 updated: 2021/05/01 17:49:36
dnl ---------------
dnl Try to get _XOPEN_SOURCE defined properly that we can use POSIX functions,
dnl or adapt to the vendor's definitions to get equivalent functionality,
dnl without losing the common non-POSIX features.
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
dnl	$2 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_XOPEN_SOURCE],[
AC_REQUIRE([AC_CANONICAL_HOST])
AC_REQUIRE([CF_POSIX_VISIBLE])

if test "$cf_cv_posix_visible" = no; then

cf_XOPEN_SOURCE=ifelse([$1],,500,[$1])
cf_POSIX_C_SOURCE=ifelse([$2],,199506L,[$2])
cf_xopen_source=

case "$host_os" in
(aix[[4-7]]*)
	cf_xopen_source="-D_ALL_SOURCE"
	;;
(msys)
	cf_XOPEN_SOURCE=600
	;;
(darwin[[0-8]].*)
	cf_xopen_source="-D_APPLE_C_SOURCE"
	;;
(darwin*)
	cf_xopen_source="-D_DARWIN_C_SOURCE"
	cf_XOPEN_SOURCE=
	;;
(freebsd*|dragonfly*|midnightbsd*)
	# 5.x headers associate
	#	_XOPEN_SOURCE=600 with _POSIX_C_SOURCE=200112L
	#	_XOPEN_SOURCE=500 with _POSIX_C_SOURCE=199506L
	cf_POSIX_C_SOURCE=200112L
	cf_XOPEN_SOURCE=600
	cf_xopen_source="-D_BSD_TYPES -D__BSD_VISIBLE -D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE -D_XOPEN_SOURCE=$cf_XOPEN_SOURCE"
	;;
(hpux11*)
	cf_xopen_source="-D_HPUX_SOURCE -D_XOPEN_SOURCE=500"
	;;
(hpux*)
	cf_xopen_source="-D_HPUX_SOURCE"
	;;
(irix[[56]].*)
	cf_xopen_source="-D_SGI_SOURCE"
	cf_XOPEN_SOURCE=
	;;
(linux*|uclinux*|gnu*|mint*|k*bsd*-gnu|cygwin)
	CF_GNU_SOURCE($cf_XOPEN_SOURCE)
	;;
(minix*)
	cf_xopen_source="-D_NETBSD_SOURCE" # POSIX.1-2001 features are ifdef'd with this...
	;;
(mirbsd*)
	# setting _XOPEN_SOURCE or _POSIX_SOURCE breaks <sys/select.h> and other headers which use u_int / u_short types
	cf_XOPEN_SOURCE=
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	;;
(netbsd*)
	cf_xopen_source="-D_NETBSD_SOURCE" # setting _XOPEN_SOURCE breaks IPv6 for lynx on NetBSD 1.6, breaks xterm, is not needed for ncursesw
	;;
(openbsd[[6-9]]*)
	# OpenBSD 6.x has broken locale support, both compile-time and runtime.
	# see https://www.mail-archive.com/bugs@openbsd.org/msg13200.html
	# Abusing the conformance level is a workaround.
	AC_MSG_WARN(this system does not provide usable locale support)
	cf_xopen_source="-D_BSD_SOURCE"
	cf_XOPEN_SOURCE=700
	;;
(openbsd[[4-5]]*)
	# setting _XOPEN_SOURCE lower than 500 breaks g++ compile with wchar.h, needed for ncursesw
	cf_xopen_source="-D_BSD_SOURCE"
	cf_XOPEN_SOURCE=600
	;;
(openbsd*)
	# setting _XOPEN_SOURCE breaks xterm on OpenBSD 2.8, is not needed for ncursesw
	;;
(osf[[45]]*)
	cf_xopen_source="-D_OSF_SOURCE"
	;;
(nto-qnx*)
	cf_xopen_source="-D_QNX_SOURCE"
	;;
(sco*)
	# setting _XOPEN_SOURCE breaks Lynx on SCO Unix / OpenServer
	;;
(solaris2.*)
	cf_xopen_source="-D__EXTENSIONS__"
	cf_cv_xopen_source=broken
	;;
(sysv4.2uw2.*) # Novell/SCO UnixWare 2.x (tested on 2.1.2)
	cf_XOPEN_SOURCE=
	cf_POSIX_C_SOURCE=
	;;
(*)
	CF_TRY_XOPEN_SOURCE
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	;;
esac

if test -n "$cf_xopen_source" ; then
	CF_ADD_CFLAGS($cf_xopen_source,true)
fi

dnl In anything but the default case, we may have system-specific setting
dnl which is still not guaranteed to provide all of the entrypoints that
dnl _XOPEN_SOURCE would yield.
if test -n "$cf_XOPEN_SOURCE" && test -z "$cf_cv_xopen_source" ; then
	AC_MSG_CHECKING(if _XOPEN_SOURCE really is set)
	AC_TRY_COMPILE([#include <stdlib.h>],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_XOPEN_SOURCE_set=yes],
	[cf_XOPEN_SOURCE_set=no])
	AC_MSG_RESULT($cf_XOPEN_SOURCE_set)
	if test "$cf_XOPEN_SOURCE_set" = yes
	then
		AC_TRY_COMPILE([#include <stdlib.h>],[
#if (_XOPEN_SOURCE - 0) < $cf_XOPEN_SOURCE
make an error
#endif],
		[cf_XOPEN_SOURCE_set_ok=yes],
		[cf_XOPEN_SOURCE_set_ok=no])
		if test "$cf_XOPEN_SOURCE_set_ok" = no
		then
			AC_MSG_WARN(_XOPEN_SOURCE is lower than requested)
		fi
	else
		CF_TRY_XOPEN_SOURCE
	fi
fi
fi # cf_cv_posix_visible
])
dnl ---------------------------------------------------------------------------
dnl CF_X_ATHENA version: 24 updated: 2020/03/10 18:53:47
dnl -----------
dnl Check for Xaw (Athena) libraries
dnl
dnl Sets $cf_x_athena according to the flavor of Xaw which is used.
AC_DEFUN([CF_X_ATHENA],
[
cf_x_athena=${cf_x_athena:-Xaw}

AC_MSG_CHECKING(if you want to link with Xaw 3d library)
withval=
AC_ARG_WITH(Xaw3d,
	[  --with-Xaw3d            link with Xaw 3d library])
if test "$withval" = yes ; then
	cf_x_athena=Xaw3d
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if you want to link with Xaw 3d xft library)
withval=
AC_ARG_WITH(Xaw3dxft,
	[  --with-Xaw3dxft         link with Xaw 3d xft library])
if test "$withval" = yes ; then
	cf_x_athena=Xaw3dxft
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if you want to link with neXT Athena library)
withval=
AC_ARG_WITH(neXtaw,
	[  --with-neXtaw           link with neXT Athena library])
if test "$withval" = yes ; then
	cf_x_athena=neXtaw
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if you want to link with Athena-Plus library)
withval=
AC_ARG_WITH(XawPlus,
	[  --with-XawPlus          link with Athena-Plus library])
if test "$withval" = yes ; then
	cf_x_athena=XawPlus
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

cf_x_athena_lib=""

if test "$PKG_CONFIG" != none ; then
	cf_athena_list=
	test "$cf_x_athena" = Xaw && cf_athena_list="xaw8 xaw7 xaw6"
	for cf_athena_pkg in \
		$cf_athena_list \
		${cf_x_athena} \
		${cf_x_athena}-devel \
		lib${cf_x_athena} \
		lib${cf_x_athena}-devel
	do
		CF_TRY_PKG_CONFIG($cf_athena_pkg,[
			cf_x_athena_lib="$cf_pkgconfig_libs"
			CF_UPPER(cf_x_athena_LIBS,HAVE_LIB_$cf_x_athena)
			AC_DEFINE_UNQUOTED($cf_x_athena_LIBS)

			CF_TRIM_X_LIBS

AC_CACHE_CHECK(for usable $cf_x_athena/Xmu package,cf_cv_xaw_compat,[
AC_TRY_LINK([
#include <X11/Xmu/CharSet.h>
],[
int check = XmuCompareISOLatin1("big", "small");
(void)check;
],[cf_cv_xaw_compat=yes],[cf_cv_xaw_compat=no])])

			if test "$cf_cv_xaw_compat" = no
			then
				# workaround for broken ".pc" files...
				case "$cf_x_athena_lib" in
				(*-lXmu*)
					;;
				(*)
					CF_VERBOSE(work around broken package)
					cf_save_xmu="$LIBS"
					cf_first_lib=`echo "$cf_save_xmu" | sed -e 's/^[ ][ ]*//' -e 's/ .*//'`
					CF_TRY_PKG_CONFIG(xmu,[
							LIBS="$cf_save_xmu"
							CF_ADD_LIB_AFTER($cf_first_lib,$cf_pkgconfig_libs)
						],[
							CF_ADD_LIB_AFTER($cf_first_lib,-lXmu)
						])
					CF_TRIM_X_LIBS
					;;
				esac
			fi

			break])
	done
fi

if test -z "$cf_x_athena_lib" ; then
	CF_X_EXT
	CF_X_TOOLKIT
	CF_X_ATHENA_CPPFLAGS($cf_x_athena)
	CF_X_ATHENA_LIBS($cf_x_athena)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_X_ATHENA_CPPFLAGS version: 9 updated: 2020/12/31 10:54:15
dnl --------------------
dnl Normally invoked by CF_X_ATHENA, with $1 set to the appropriate flavor of
dnl the Athena widgets, e.g., Xaw, Xaw3d, neXtaw.
AC_DEFUN([CF_X_ATHENA_CPPFLAGS],
[
AC_REQUIRE([AC_PATH_XTRA])
cf_x_athena_root=ifelse([$1],,Xaw,[$1])
cf_x_athena_inc=""

for cf_path in default \
	/usr/contrib/X11R6 \
	/usr/contrib/X11R5 \
	/usr/lib/X11R5 \
	/usr/local
do
	if test -z "$cf_x_athena_inc" ; then
		CF_SAVE_XTRA_FLAGS([CF_X_ATHENA_CPPFLAGS])
		cf_test=X11/$cf_x_athena_root/SimpleMenu.h
		if test "$cf_path" != default ; then
			CF_APPEND_TEXT(CPPFLAGS,-I$cf_path/include)
			AC_MSG_CHECKING(for $cf_test in $cf_path)
		else
			AC_MSG_CHECKING(for $cf_test)
		fi
		AC_TRY_COMPILE([
#include <X11/Intrinsic.h>
#include <$cf_test>],[],
			[cf_result=yes],
			[cf_result=no])
		AC_MSG_RESULT($cf_result)
		CF_RESTORE_XTRA_FLAGS([CF_X_ATHENA_CPPFLAGS])
		if test "$cf_result" = yes ; then
			test "$cf_path"  = default && cf_x_athena_inc=default
			test "$cf_path" != default && cf_x_athena_inc="$cf_path/include"
			break
		fi
	fi
done

if test -z "$cf_x_athena_inc" ; then
	AC_MSG_WARN([Unable to find Athena header files])
elif test "$cf_x_athena_inc" != default ; then
	CF_APPEND_TEXT(CPPFLAGS,-I$cf_x_athena_inc)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_X_ATHENA_LIBS version: 13 updated: 2020/01/11 18:16:10
dnl ----------------
dnl Normally invoked by CF_X_ATHENA, with $1 set to the appropriate flavor of
dnl the Athena widgets, e.g., Xaw, Xaw3d, neXtaw.
AC_DEFUN([CF_X_ATHENA_LIBS],
[AC_REQUIRE([CF_X_TOOLKIT])
cf_x_athena_root=ifelse([$1],,Xaw,[$1])
cf_x_athena_lib=""

for cf_path in default \
	/usr/contrib/X11R6 \
	/usr/contrib/X11R5 \
	/usr/lib/X11R5 \
	/usr/local
do
	for cf_lib in \
		${cf_x_athena_root} \
		${cf_x_athena_root}7 \
		${cf_x_athena_root}6
	do
	for cf_libs in \
		"-l$cf_lib -lXmu" \
		"-l$cf_lib -lXpm -lXmu" \
		"-l${cf_lib}_s -lXmu_s"
	do
		test -n "$cf_x_athena_lib" && break

		CF_SAVE_XTRA_FLAGS([CF_X_ATHENA_LIBS])
		cf_test=XawSimpleMenuAddGlobalActions
		test "$cf_path" != default && cf_libs="-L$cf_path/lib $cf_libs"
		CF_ADD_LIBS($cf_libs)
		AC_MSG_CHECKING(for $cf_test in $cf_libs)
		AC_TRY_LINK([
#include <X11/Intrinsic.h>
#include <X11/$cf_x_athena_root/SimpleMenu.h>
],[
$cf_test((XtAppContext) 0)],
			[cf_result=yes],
			[cf_result=no])
		AC_MSG_RESULT($cf_result)
		CF_RESTORE_XTRA_FLAGS([CF_X_ATHENA_LIBS])

		if test "$cf_result" = yes ; then
			cf_x_athena_lib="$cf_libs"
			break
		fi
	done # cf_libs
		test -n "$cf_x_athena_lib" && break
	done # cf_lib
done

if test -z "$cf_x_athena_lib" ; then
	AC_MSG_ERROR(
[Unable to successfully link Athena library (-l$cf_x_athena_root) with test program])
fi

CF_ADD_LIBS($cf_x_athena_lib)
CF_UPPER(cf_x_athena_LIBS,HAVE_LIB_$cf_x_athena)
AC_DEFINE_UNQUOTED($cf_x_athena_LIBS)
])
dnl ---------------------------------------------------------------------------
dnl CF_X_EXT version: 3 updated: 2010/06/02 05:03:05
dnl --------
AC_DEFUN([CF_X_EXT],[
CF_TRY_PKG_CONFIG(Xext,,[
	AC_CHECK_LIB(Xext,XextCreateExtension,
		[CF_ADD_LIB(Xext)])])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_X_TOOLKIT version: 26 updated: 2021/01/02 09:31:20
dnl ------------
dnl Check for X Toolkit libraries
AC_DEFUN([CF_X_TOOLKIT],
[
AC_REQUIRE([AC_PATH_XTRA])
AC_REQUIRE([CF_CHECK_CACHE])

# OSX is schizoid about who owns /usr/X11 (old) versus /opt/X11 (new) (and
# in some cases has installed dummy files in the former, other cases replaced
# it with a link to the new location).  This complicates the configure script.
# Check for that pitfall, and recover using pkg-config
#
# If none of these are set, the configuration is almost certainly broken.
if test -z "${X_CFLAGS}${X_PRE_LIBS}${X_LIBS}${X_EXTRA_LIBS}"
then
	CF_TRY_PKG_CONFIG(x11,,[AC_MSG_WARN(unable to find X11 library)])
	CF_TRY_PKG_CONFIG(ice,,[AC_MSG_WARN(unable to find ICE library)])
	CF_TRY_PKG_CONFIG(sm,,[AC_MSG_WARN(unable to find SM library)])
	CF_TRY_PKG_CONFIG(xt,,[AC_MSG_WARN(unable to find Xt library)])
else
	LIBS="$X_PRE_LIBS $LIBS $X_EXTRA_LIBS"
fi

cf_have_X_LIBS=no

CF_TRY_PKG_CONFIG(xt,[

	case "x$LIBS" in
	(*-lX11*)
		;;
	(*)
# we have an "xt" package, but it may omit Xt's dependency on X11
AC_CACHE_CHECK(for usable X dependency,cf_cv_xt_x11_compat,[
AC_TRY_LINK([
#include <X11/Xlib.h>
],[
	int rc1 = XDrawLine((Display*) 0, (Drawable) 0, (GC) 0, 0, 0, 0, 0);
	int rc2 = XClearWindow((Display*) 0, (Window) 0);
	int rc3 = XMoveWindow((Display*) 0, (Window) 0, 0, 0);
	int rc4 = XMoveResizeWindow((Display*)0, (Window)0, 0, 0, 0, 0);
],[cf_cv_xt_x11_compat=yes],[cf_cv_xt_x11_compat=no])])
		if test "$cf_cv_xt_x11_compat" = no
		then
			CF_VERBOSE(work around broken X11 dependency)
			# 2010/11/19 - good enough until a working Xt on Xcb is delivered.
			CF_TRY_PKG_CONFIG(x11,,[CF_ADD_LIB_AFTER(-lXt,-lX11)])
		fi
		;;
	esac

AC_CACHE_CHECK(for usable X Toolkit package,cf_cv_xt_ice_compat,[
AC_TRY_LINK([
#include <X11/Shell.h>
],[int num = IceConnectionNumber(0); (void) num
],[cf_cv_xt_ice_compat=yes],[cf_cv_xt_ice_compat=no])])

	if test "$cf_cv_xt_ice_compat" = no
	then
		# workaround for broken ".pc" files used for X Toolkit.
		case "x$X_PRE_LIBS" in
		(*-lICE*)
			case "x$LIBS" in
			(*-lICE*)
				;;
			(*)
				CF_VERBOSE(work around broken ICE dependency)
				CF_TRY_PKG_CONFIG(ice,
					[CF_TRY_PKG_CONFIG(sm)],
					[CF_ADD_LIB_AFTER(-lXt,$X_PRE_LIBS)])
				;;
			esac
			;;
		esac
	fi

	cf_have_X_LIBS=yes
],[

	LDFLAGS="$X_LIBS $LDFLAGS"
	CF_CHECK_CFLAGS($X_CFLAGS)

	AC_CHECK_FUNC(XOpenDisplay,,[
	AC_CHECK_LIB(X11,XOpenDisplay,
		[CF_ADD_LIB(X11)])])

	AC_CHECK_FUNC(XtAppInitialize,,[
	AC_CHECK_LIB(Xt, XtAppInitialize,
		[AC_DEFINE(HAVE_LIBXT,1,[Define to 1 if we can compile with the Xt library])
		 cf_have_X_LIBS=Xt
		 LIBS="-lXt $LIBS"])])
])

if test "$cf_have_X_LIBS" = no ; then
	AC_MSG_WARN(
[Unable to successfully link X Toolkit library (-lXt) with
test program.  You will have to check and add the proper libraries by hand
to makefile.])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF__CHECK_RUN version: 1 updated: 2018/02/21 21:26:03
dnl -------------
dnl Check if a simple program can be made to run with the existing libraries.
define([CF__CHECK_RUN],[
AC_TRY_RUN([#include <stdio.h>
int main(void) {
	fflush(stderr);
	${cf_cv_main_return:-return}(0);
}],
	$1=yes,
	$1=no,
	$1=unknown)
])
dnl ---------------------------------------------------------------------------
dnl CF__CURSES_HEAD version: 2 updated: 2010/10/23 15:54:49
dnl ---------------
dnl Define a reusable chunk which includes <curses.h> and <term.h> when they
dnl are both available.
define([CF__CURSES_HEAD],[
#ifdef HAVE_XCURSES
#include <xcurses.h>
char * XCursesProgramName = "test";
#else
#include <${cf_cv_ncurses_header:-curses.h}>
#if defined(NCURSES_VERSION) && defined(HAVE_NCURSESW_TERM_H)
#include <ncursesw/term.h>
#elif defined(NCURSES_VERSION) && defined(HAVE_NCURSES_TERM_H)
#include <ncurses/term.h>
#elif defined(HAVE_TERM_H)
#include <term.h>
#endif
#endif
])
dnl ---------------------------------------------------------------------------
dnl CF__ICONV_BODY version: 2 updated: 2007/07/26 17:35:47
dnl --------------
dnl Test-code needed for iconv compile-checks
define([CF__ICONV_BODY],[
	iconv_t cd = iconv_open("","");
	iconv(cd,NULL,NULL,NULL,NULL);
	iconv_close(cd);]
)dnl
dnl ---------------------------------------------------------------------------
dnl CF__ICONV_HEAD version: 1 updated: 2007/07/26 15:57:03
dnl --------------
dnl Header-files needed for iconv compile-checks
define([CF__ICONV_HEAD],[
#include <stdlib.h>
#include <iconv.h>]
)dnl
dnl ---------------------------------------------------------------------------
dnl CF__INET_HEAD version: 1 updated: 2013/10/08 17:47:05
dnl -------------
dnl Header-files needed for inet.h compile-checks
define([CF__INET_HEAD],[
#if defined(__MINGW32__)

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif
])dnl
dnl ---------------------------------------------------------------------------
dnl CF__INTL_BODY version: 4 updated: 2021/05/19 19:35:25
dnl -------------
dnl Test-code needed for libintl compile-checks
dnl $1 = parameter 2 from AM_WITH_NLS
define([CF__INTL_BODY],[
	bindtextdomain ("", "");
	return (gettext ("") != 0)
			ifelse([$1], need-ngettext, [ + (ngettext ("", "", 0) != 0)], [])
#ifndef IGNORE_MSGFMT_HACK
			[ + _nl_msg_cat_cntr]
#endif
])
dnl ---------------------------------------------------------------------------
dnl CF__INTL_HEAD version: 1 updated: 2007/07/26 17:35:47
dnl -------------
dnl Header-files needed for libintl compile-checks
define([CF__INTL_HEAD],[
#include <libintl.h>
extern int _nl_msg_cat_cntr;
])dnl
dnl ---------------------------------------------------------------------------
dnl CF__SED_TRIMBLANKS version: 1 updated: 2021/01/02 09:31:20
dnl ------------------
dnl Trim something using sed, then trim extra whitespace
dnl $1 = extra parameters, e.g., in CF_STRIP_G_OPT
define([CF__SED_TRIMBLANKS],[sed ifelse($1,,,[$1] )-e 's%[[	]]% %g' -e 's% [[ ]]*% %g' -e 's%^ %%' -e 's% [$]%%'])dnl
dnl ---------------------------------------------------------------------------
dnl CF__SSL_BODY version: 2 updated: 2008/04/13 10:56:06
dnl ------------
dnl Body for test-compile of SSL code.
define([CF__SSL_BODY],[
#ifdef USE_GNUTLS_FUNCS
    gnutls_global_init();
#else
    SSL_shutdown((SSL *)0)
#endif
])dnl
dnl ---------------------------------------------------------------------------
dnl CF__SSL_HEAD version: 2 updated: 2008/04/13 10:56:06
dnl ------------
dnl Headers for test-compile of SSL code.
define([CF__SSL_HEAD],[
#include <stdio.h>
#if defined(USE_OPENSSL_INCL)
#include <openssl/ssl.h>
#elif defined(USE_GNUTLS_FUNCS)
#include <gnutls/gnutls.h>
#elif defined(USE_GNUTLS_INCL)
#include <gnutls/openssl.h>
#else
#include <ssl.h>
#endif
])dnl
dnl ---------------------------------------------------------------------------
dnl jm_GLIBC21 version: 4 updated: 2015/05/10 19:52:14
dnl ----------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl glibc21.m4
dnl ====================
dnl serial 2
dnl
dnl Test for the GNU C Library, version 2.1 or newer.
dnl From Bruno Haible.
AC_DEFUN([jm_GLIBC21],
[
AC_CACHE_CHECK(whether we are using the GNU C Library 2.1 or newer,
	ac_cv_gnu_library_2_1,
	[AC_EGREP_CPP([Lucky GNU user],
	[
#include <features.h>
#ifdef __GNU_LIBRARY__
 #if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1) || (__GLIBC__ > 2)
  Lucky GNU user
 #endif
#endif
	],
	ac_cv_gnu_library_2_1=yes,
	ac_cv_gnu_library_2_1=no)])
	AC_SUBST(GLIBC21)
	GLIBC21="$ac_cv_gnu_library_2_1"
])
