OBJS=  LYClean.o LYShowInfo.o LYEdit.o LYStrings.o \
LYMail.o HTAlert.o GridText.o LYGetFile.o \
LYMain.o LYMainLoop.o LYCurses.o LYBookmark.o LYUtils.o \
LYOptions.o LYReadCFG.o LYSearch.o LYHistory.o \
LYForms.o LYPrint.o LYrcFile.o LYDownload.o LYNews.o LYKeymap.o \
HTML.o HTFWriter.o HTInit.o DefaultStyle.o LYLocal.o LYUpload.o \
LYLeaks.o LYexit.o LYJump.o LYList.o LYCgi.o LYTraversal.o \
LYEditmap.o LYCharSets.o LYCharUtils.o LYMap.o

CFLAGS= $(MCFLAGS) -I.. $(SLANGINC)

all: lynx

lynx:   message $(OBJS) $(WWWLIB)
	@echo "Linking and creating Lynx executable"
	$(CC) $(CFLAGS) -o lynx  $(OBJS) $(WWWLIB) $(SLANGLIB) $(LIBS)
	@echo "Copying Lynx executable into this directory"
	cp lynx ..
	@echo "Welcome to Lynx!"

message:
	@echo "Compiling Lynx sources"

dbg:	$(OBJS) $(WWWLIB)
	@echo "Making Lynx code"
	$(CC) -g $(OBJS) $(CFLAGS) $(WWWLIB) $(SLANGLIB) $(LIBS)

lint:
	lint *.c  > ../lint.out

clean:
	rm -f lynx core *.[ob]

LYMain.o: ../userdefs.h
LYMainLoop.o: ../userdefs.h
LYReadCFG.o: ../userdefs.h
HTFWriter.o: ../userdefs.h
LYGetFile.o: ../userdefs.h
LYOptions.o: ../userdefs.h
LYReadCFG.o: ../userdefs.h
LYrcFile.o: ../userdefs.h
LYMail.o: ../userdefs.h
LYUtils.o: ../userdefs.h
HTInit.o: ../userdefs.h
LYTraversal.o: ../userdefs.h
LYMail.o: ../userdefs.h
LYCharSets.o: ../userdefs.h
