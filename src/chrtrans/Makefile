#
# Makefile for the makeuctb and unicode tables.
#
# This may not yet work for the general case.
# Only some dependencies included.
#
#
CFLAGS = $(MCFLAGS) -I.. -I../.. -I../../WWW/Library/Implementation

.SUFFIXES: .tbl
#
# This file contains the font map for the default (hardware) font
#

FONTMAP_INC = iso01_uni.h# default, if not set by recursive call

### #include $(TOPDIR)/Rules.make ???

### fastdep: $(FONTMAP_INC)

### MCFLAGS=-g -DUNIX -DLINUX -DNO_KEYPAD -DNO_TTYTYPE -DEXP_CHARTRANS -I.. -I../../WWW/Library/Implementation -I../..

CHRTR=

TABLES= $(CHRTR)iso02_uni.h \
 $(CHRTR)iso01_uni.h \
 $(CHRTR)def7_uni.h \
 $(CHRTR)iso03_uni.h \
 $(CHRTR)iso04_uni.h \
 $(CHRTR)iso05_uni.h \
 $(CHRTR)iso07_uni.h \
 $(CHRTR)iso09_uni.h \
 $(CHRTR)iso10_uni.h \
 $(CHRTR)koi8r_uni.h \
 $(CHRTR)cp437_uni.h \
 $(CHRTR)cp850_uni.h \
 $(CHRTR)cp852_uni.h \
 $(CHRTR)cp866_uni.h \
 $(CHRTR)cp1250_uni.h \
 $(CHRTR)cp1251_uni.h \
 $(CHRTR)cp1252_uni.h \
 $(CHRTR)utf8_uni.h \
 $(CHRTR)rfc_suni.h \
 $(CHRTR)mnemonic_suni.h \
 $(CHRTR)mnem_suni.h

default: $(FONTMAP_INC)

tables: $(TABLES)

makeuctb: makeuctb.c UCkd.h
	$(CC) $(CFLAGS) -o makeuctb makeuctb.c

.tbl.h:
	./makeuctb $*.tbl > $@

iso01_uni.h: iso01_uni.tbl makeuctb
iso02_uni.h: iso02_uni.tbl makeuctb
def7_uni.h: def7_uni.tbl makeuctb
iso03_uni.h: iso03_uni.tbl makeuctb
iso04_uni.h: iso04_uni.tbl makeuctb
iso05_uni.h: iso05_uni.tbl makeuctb
iso07_uni.h: iso07_uni.tbl makeuctb
iso09_uni.h: iso09_uni.tbl makeuctb
iso10_uni.h: iso10_uni.tbl makeuctb
koi8r_uni.h: koi8r_uni.tbl makeuctb
cp437_uni.h: cp437_uni.tbl makeuctb
cp850_uni.h: cp850_uni.tbl makeuctb
cp852_uni.h: cp852_uni.tbl makeuctb
cp1250_uni.h: cp1250_uni.tbl makeuctb
cp1251_uni.h: cp1251_uni.tbl makeuctb
cp1252_uni.h: cp1252_uni.tbl makeuctb
utf8_uni.h: utf8_uni.tbl makeuctb
mnemonic_suni.h: mnemonic_suni.tbl makeuctb
mnem_suni.h: mnem_suni.tbl makeuctb
rfc_suni.h: rfc_suni.tbl makeuctb

clean:
	rm -f makeuctb *.o *uni.h

distclean: clean
	-rm -rf obsolete
	rm -f core *.bak *.sav *~ *.h_old
