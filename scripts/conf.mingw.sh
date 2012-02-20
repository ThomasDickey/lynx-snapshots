#!/bin/bash.exe
LDFLAGS="-mno-cygwin -static -L/mingw/lib" \
LIBS="-lwsock32 -lgdi32 -lmsvcrt -liconv" \
CFLAGS="-mno-cygwin -I/mingw/include -I/mingw/include/openssl -W -Wall -O3 -D_WINDOWS -DSH_EX -DWIN_EX -DNOUSERS -DNOSIGHUP -DDOSPATH -DUSE_ALT_BLAT_MAILER -DBOXHORI=0 -DBOXVERT=0" \
CPPFLAGS="-mno-cygwin -I/mingw/include -I/mingw/include/openssl" \
./configure --prefix=/d/cygwin/mingw/lynx2.8.7dev.13/lynx-conf \
--host=mingw32 \
--disable-dired-override \
--disable-full-paths \
--enable-addrlist-page \
--enable-change-exec \
--enable-charset-choice \
--enable-default-colors \
--enable-exec-links \
--enable-externs \
--enable-file-upload \
--enable-gzip-help \
--enable-nested-tables \
--enable-nls \
--enable-vertrace \
--includedir=/mingw/include \
--sysconfdir=/d/cygwin/mingw/lynx2.8.7dev.13/lynx-conf \
--datadir=/d/cygwin/mingw/lynx2.8.7dev.13/lynx-conf \
--with-bzlib \
--with-cfg-file=d:/cygwin/mingw/lynx2.8.7dev.13/lynx-conf/lynx.cfg \
--with-lss-file=d:/cygwin/mingw/lynx2.8.7dev.13/lynx-conf/lynx.lss \
--with-mime-libdir=c:/ \
--with-nls-datadir=d:/cygwin/mingw/share \
--with-pkg-config=no \
--with-screen=curses \
--with-ssl=/mingw/lib \
--with-zlib
