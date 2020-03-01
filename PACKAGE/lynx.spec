# $LynxId: lynx.spec,v 1.58 2020/03/01 13:47:14 tom Exp $
Summary: A text-based Web browser
Name: lynx-dev
Version: 2.9.0
Release: dev.6
License: GPLv2
Group: Applications/Internet
Source: lynx%{version}%{release}.tgz
URL: https://lynx.invisible-island.net
Provides: webclient >= 0.0
Provides: text-www-browser >= 0.0

# Fedora:
BuildRequires: openssl-devel, pkgconfig, ncurses-devel >= 5.3-5,
BuildRequires: zlib-devel, gettext
BuildRequires: libidn-devel
# BuildRequires: bzip2-devel

# SuSE:
# BuildRequires: libbz2-devel

Requires: gzip, bzip2, tar, zip, unzip

%description
Lynx is a fully-featured World Wide Web (WWW) client for users running
cursor-addressable, character-cell display devices.  It is very fast and easy
to use.  It will display HTML documents containing links to files residing on
the local system, as well as files residing on remote systems running Gopher,
HTTP, FTP, WAIS, and NNTP servers.

%define lynx_doc %{_defaultdocdir}/%{name}
%define lynx_etc %{_sysconfdir}/%{name}

%prep

%define debug_package %{nil}
%setup -q -n lynx%{version}%{release}

%build
%configure \
	--target %{_target_platform} \
	--prefix=%{_prefix} \
	--bindir=%{_bindir} \
	--program-suffix=-dev \
	--datadir=%{lynx_doc} \
	--libdir=%{lynx_etc} \
	--mandir=%{_mandir} \
	--sysconfdir=%{lynx_etc} \
	--with-cfg-path=%{lynx_etc}:%{lynx_doc}/samples \
	--with-textdomain=%{name} \
	--disable-font-switch \
	--disable-internal-links \
	--enable-8bit-toupper \
	--enable-addrlist-page \
	--enable-alt-bindings \
	--enable-ascii-ctypes \
	--enable-cgi-links \
	--enable-change-exec \
	--enable-charset-choice \
	--enable-cjk \
	--enable-default-colors \
	--enable-exec-links \
	--enable-exec-scripts \
	--enable-externs \
	--enable-file-upload \
	--enable-font-switch \
	--enable-forms-options \
	--enable-gzip-help \
	--enable-htmlized-cfg \
	--enable-internal-links \
	--enable-ipv6 \
	--enable-japanese-utf8 \
	--enable-justify-elts \
	--enable-kbd-layout \
	--enable-local-docs \
	--enable-locale-charset \
	--enable-nested-tables \
	--enable-nls \
	--enable-nsl-fork \
	--enable-partial \
	--enable-persistent-cookies \
	--enable-prettysrc \
	--enable-progressbar \
	--enable-read-eta \
	--enable-scrollbar \
	--enable-session-cache \
	--enable-sessions \
	--enable-source-cache \
	--enable-syslog \
	--enable-warnings \
	--with-bzlib \
	--with-screen=ncursesw6dev \
	--with-ssl \
	--with-zlib
make \
	docdir=%{lynx_doc}

%install
rm -rf $RPM_BUILD_ROOT
chmod -x samples/mailto-form.pl

make install-full \
	DESTDIR=$RPM_BUILD_ROOT \
	docdir=%{lynx_doc}

cat >>$RPM_BUILD_ROOT%{lynx_etc}/lynx.cfg <<EOF
DEFAULT_INDEX_FILE:http://www.google.com/
LOCALE_CHARSET:TRUE
EOF

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_mandir}/*/*
%{_datadir}/locale/*
%{lynx_doc}/*
%config %{lynx_etc}/*.cfg
%config %{lynx_etc}/*.lss

%changelog

* Thu May 17 2018 Thomas E. Dickey
- use "ncursesw6dev", reflecting renaming of ncurses test-packages to avoid
  conflict with new packages in Fedora.

* Mon Mar 12 2018 Thomas E. Dickey
- rename to "lynx-dev", add a few dependencies where package names are same.

* Fri Sep 17 2010 Thomas E. Dickey
- initial version.
