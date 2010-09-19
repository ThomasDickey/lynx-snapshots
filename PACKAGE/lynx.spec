# $LynxId: lynx.spec,v 1.4 2010/09/17 09:52:19 tom Exp $
Summary: A text-based Web browser
Name: lynx
Version: 2.8.8
Release: dev.6
License: GPLv2
Group: Applications/Internet
Source: lynx%{version}%{release}.tgz
# URL: http://lynx.isc.org/
Provides: webclient
Provides: text-www-browser
# BuildRequires: openssl-devel, pkgconfig, ncurses-devel >= 5.3-5, 
# BuildRequires: zlib-devel, gettext, rsh, telnet, zip, unzip
# Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
Lynx is a fully-featured World Wide Web (WWW) client for users running
cursor-addressable, character-cell display devices.  It is very fast and easy
to use.  It will display HTML documents containing links to files residing on
the local system, as well as files residing on remote systems running Gopher,
HTTP, FTP, WAIS, and NNTP servers.

%prep
%setup -q -n lynx%{version}%{release}

%build
CPPFLAGS="-DMISC_EXP -DEXP_HTTP_HEADERS" \
%configure \
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
	--with-screen=ncursesw \
	--with-ssl \
	--with-zlib
make

%install
rm -rf $RPM_BUILD_ROOT
chmod -x samples/mailto-form.pl
%makeinstall mandir=$RPM_BUILD_ROOT%{_mandir}/man1 libdir=$RPM_BUILD_ROOT/etc

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc docs README INSTALLATION samples
%doc test lynx.hlp lynx_help
%{_bindir}/lynx
%{_mandir}/*/*
%{_datadir}/locale/*
%config %{_sysconfdir}/lynx.cfg
%config %{_sysconfdir}/lynx.lss

%changelog

* Fri Sep 17 2010 Thomas E. Dickey
- initial version.
