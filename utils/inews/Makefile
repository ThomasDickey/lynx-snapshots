# Makefile for NN version of mini inews.
#
# $RCSfile: Makefile,v $	$Revision: 1.3 $
#
# $Author: news $	$Date: 1989/12/21 17:59:52 $
#
# $State: Exp $	$Locker:  $
#
# $Log: Makefile,v $
# Revision 1.3  1989/12/21 17:59:52  news
# v2_6
#
# Revision 1.3  89/12/21  17:59:52  news
# Added kit processing, cleanups.
#

TARGET	= inews

BINDIR	= /usr/lib/news/
SHELL	= /bin/sh

CC	= cc
# General definitions here which apply to all -- form -DDEF
DEFS	= -DFOR_NN
# Link libraries -- form -lLIB
LIBS	=    # ../hostname.o
# Lint definitions -- same as DEFS, but form -D DEF
LDEFS	= -D lint -D FOR_NN
# Lint libraries -- same as LIBS, but form -l LIB
LLIBS	=
# Set to -g for debugging, -O for optimise, or both if compiler handles it
DEBUG	= -O
CFLAGS	= $(DEBUG) $(DEFS) -I.. -I../conf

SRCS	= inews.c clientlib.c version.c
OBJS	= inews.o clientlib.o version.o
CLUDES	= clientlib.h conf.h nntp.h
KIT	= Makefile README README.NN $(SRCS) $(CLUDES)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

$(OBJS):	$(CLUDES)

all:	$(TARGET)

Manifest:	$(KIT)
	touch Manifest
	rm -f foo
	ls -ls Manifest $(KIT) > foo
	mv foo Manifest
	ls -ls Manifest $(KIT) > foo
	mv foo Manifest

kit:	Manifest
	shar Manifest $(KIT) > inews.shar

clean:
	rm -f $(OBJS) core lint tags

clobber:	clean
	rm -f $(TARGET) Make.Log Manifest $(KIT)

lint:	$(SRCS)
	lint -D lint $(LLIBES) $(LDEFS) $(SRCS) > lint

tags:	$(SRCS)
	ctags $(SRCS)

install:	$(TARGET)
	../inst inews $(TARGET)
#	install -o news -g news -m 511 -s -c $(TARGET) $(BINDIR)
