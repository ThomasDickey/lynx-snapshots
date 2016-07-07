!	Make WorldWideWeb LIBRARY under VMS
!       =======================================================
!
! History:
!  14 Aug 91 (TBL)	Reconstituted
!  25 Jun 92 (JFG)	Added TCP socket emulation over DECnet
!  07 Sep 93 (MD)	Remade for version 2.09a
!  10 Dec 93 (FM)	Upgrade for version 2.14 with Lynx v2.1
!  13 Dec 93 (FM)	Added conditional compilations for VAXC vs. DECC
!			(MMS can't handle a MODULE list as large as the
!			 WWWLibrary has become, so this just illustrates
!			 how you'd set it up if it could 8-).
!  26 Oct 94 (RLD)	Updated to work with VAX/VMS v5.5-1 and AXP/VMS v6.1
!  31 Oct 94 (RLD)      Updated for Lynx v2.3.4, supporting OpenCMU and
!                       TCPWare
!  18 Nov 94 (FM)	Updated for SOCKETSHR/NETLIB
!  07 Dec 94 (FM)	Updated for DECC/VAX, VAXC/VAX and DECC/AXP
!  03 May 95 (FM)	Include /NoMember for DECC (not the default on AXP,
!			and the code assumes byte alignment).
!  07 Jul 95 (FM)	Added GNUC support.
!  15 Sep 06 (TD)	Cleanup...
!
! Bugs:
!	The dependencies are anything but complete - they were
!	just enough to allow the files to be compiled.
!
! Instructions:
! 	Copy [WWW.LIBRARY.VMS]DESCRIP.MMS into [WWW.LIBRARY.IMPLEMENTATION]
!	Use the correct command line for your TCP/IP implementation,
!	inside the IMPLEMENTATION directory:
!
!	$ MMS/MACRO=(MULTINET=1)		for VAXC - MultiNet
!	$ MMS/MACRO=(WIN_TCP=1)			for VAXC - Wollongong TCP/IP
!	$ MMS/MACRO=(UCX=1)			for VAXC - UCX
!	$ MMS/MACRO=(CMU_TCP=1)			for VAXC - OpenCMU TCP/IP
!	$ MMS/MACRO=(SOCKETSHR_TCP=1)		for VAXC - SOCKETSHR/NETLIB
!	$ MMS/MACRO=(TCPWARE=1)			for VAXC - TCPWare TCP/IP
!	$ MMS/MACRO=(DECNET=1)		for VAXC - socket emulation over DECnet
!
!	$ MMS/MACRO=(MULTINET=1,DEC_C=1)	for DECC - MultiNet
!	$ MMS/MACRO=(WIN_TCP=1,DEC_C=1)		for DECC - Wollongong TCP/IP
!	$ MMS/MACRO=(UCX=1,DEC_C=1)		for DECC - UCX
!	$ MMS/MACRO=(CMU_TCP=1,DEC_C=1)		for DECC - OpenCMU TCP/IP
!	$ MMS/MACRO=(SOCKETSHR_TCP=1,DEC_C=1)	for DECC - SOCKETSHR/NETLIB
!	$ MMS/MACRO=(TCPWARE=1,DEC_C=1)		for DECC - TCPWare TCP/IP
!	$ MMS/MACRO=(DECNET=1,DEC_C=1)	for DECC - socket emulation over DECnet
!
!	$ MMS/MACRO=(MULTINET=1,GNU_C=1)	for GNUC - MultiNet
!	$ MMS/MACRO=(WIN_TCP=1,GNU_C=1)		for GNUC - Wollongong TCP/IP
!	$ MMS/MACRO=(UCX=1,GNU_C=1)		for GNUC - UCX
!	$ MMS/MACRO=(CMU_TCP=1,GNU_C=1)		for GNUC - OpenCMU TCP/IP
!	$ MMS/MACRO=(SOCKETSHR_TCP=1,GNU_C=1)	for GNUC - SOCKETSHR/NETLIB
!	$ MMS/MACRO=(TCPWARE=1,GNU_C=1)		for GNUC - TCPWare TCP/IP
!	$ MMS/MACRO=(DECNET=1,GNU_C=1)	for GNUC - socket emulation over DECnet
!
! To compile with debug mode:
!
!	$ MMS/MACRO=(MULTINET=1, DEBUG=1)	for Multinet
!
!
! If you are on HEP net and want to build using the really latest sources on
! PRIAM:: then define an extra macro U=PRIAM::, e.g.
!
!	$ MMS/MACRO=(MULTINET=1, U=PRIAM::)	for Multinet
!
! This will copy the sources from PRIAM as necessary. You can also try
!
!	$ MMS/MACRO=(U=PRIAM::) descrip.mms
!
! to update this file.


.include Version.make

! debug flags
.ifdef DEBUG
DEBUGFLAGS = /Debug /NoOptimize
.endif

INCLUDES = /Include=([-.Implementation],[---.src],[---])

! defines valid for all compilations
EXTRADEFINES = ACCESS_AUTH, VC="""$(VC)"""

.ifdef WIN_TCP
NETWORK_DEF = WIN_TCP
.else
.ifdef CMU_TCP
NETWORK_DEF = CMU_TCP
.else
.ifdef SOCKETSHR_TCP
NETWORK_DEF = SOCKETSHR_TCP
.else
.ifdef UCX
NETWORK_DEF = UCX
.else
.ifdef TCPWARE
NETWORK_DEF = TCPWARE,UCX
.else
.ifdef DECnet
NETWORK_DEF = DECNET
.else !  Default to MultiNet
NETWORK_DEF = MULTINET
.endif !  DECnet
.endif !  TCPWARE
.endif !  UCX
.endif !  SOCKETSHR_TCP
.endif !  CMU_TCP
.endif !  WIN_TCP

! DECC flags for all compilations
.ifdef DEC_C
MY_CFLAGS = /decc/Prefix=All /NoMember /Warning=(disable=implicitfunc) $(INCLUDES)
.else
MY_CFLAGS = $(INCLUDES)
.endif

CFLAGS = $(DEBUGFLAGS) $(MY_CFLAGS) /Define=($(EXTRADEFINES), $(NETWORK_DEF))

.ifdef GNU_C
CC = gcc
.endif

!HEADERS = HTUtils.h, HTStream.h, www_tcp.h, HText.h -
!        HTParse.h, HTAccess.h, HTTP.h, HTFile.h, -
!	HTBTree.h, HTTCP.h, SGML.h, -
!	HTML.h, HTMLDTD.h, HTChunk.h, HTPlain.h, -
!	HTFwriter.h, HTMLGen.h, -
!	HTAtom.h, HTAnchor.h, HTStyle.h, -
!	HTList.h, HTString.h, HTAlert.h, -
!	HTRules.h, HTFormat.h, HTInit.h, -
!	HTMIME.h, HTTelnet.h, -
!	HTFinger.h, HTAABrow.h, -
!	HTAAProt.h, HTAAUtil.h, -
!	HTAssoc.h, HTUU.h, -
!	HTVMSUtils.h,

MODULES = HTParse, HTAccess, HTTP, HTFile, HTBTree, HTFTP, HTTCP, HTString, -
	SGML, HTMLDTD, HTChunk, HTPlain, HTMLGen, -
	HTAtom, HTAnchor, HTStyle, HTList, HTRules, HTFormat, -
	HTMIME, HTNews, HTGopher, HTTelnet, HTFinger, -
	HTWSRC, HTAAUtil, HTAABrow, HTGroup, -
	HTAAProt, HTAssoc, HTLex, HTUU, HTVMSUtils, -
	HTWAIS, HTVMS_WaisUI, HTVMS_WaisProt

!.ifdef DECNET  ! Strip FTP, Gopher, News, WAIS
!HEADERS = $(COMMON_HEADERS)
!MODULES = $(COMMON_MODULES)
!.else
!HEADERS = $(COMMON_HEADERS), $(EXTRA_HEADERS), $(WAIS_HEADER)
!MODULES = $(COMMON_MODULES), $(EXTRA_MODULES), $(WAIS_MODULE)
!.endif

!___________________________________________________________________
! WWW Library

!library : $(HEADERS)  wwwlib($(MODULES))
library : wwwlib($(MODULES))
 	@ Continue

build_www.com : descrip.mms
	$(MMS) /NoAction /From_Sources /Output = build_www.com /Macro = ($(NETWORK_DEF)=1)

clean :
	- Set Protection = (Owner:RWED) *.*;-1
	- Purge /NoLog /NoConfirm
	- Delete /NoLog /NoConfirm *.obj;,*.olb;

!___________________________________________________________________
! Simple Dependencies


!HTString.obj :	HTString.c HTString.h www_tcp.h Version.make HTUtils.h
!HTAtom.obj :	HTAtom.c HTAtom.h HTUtils.h HTString.h
!HTChunk.obj :	HTChunk.c HTChunk.h HTUtils.h
!HTList.obj :	HTList.c HTList.h HTUtils.h
!HTBTree.obj :	HTBTree.c HTBTree.h HTUtils.h
!HTMLDTD.obj :	HTMLDTD.c HTMLDTD.h SGML.h
!HTPlain.obj :	HTPlain.c HTPlain.h HTStream.h
!HTMLGen.obj :	HTMLGen.c HTMLGen.h HTUtils.h HTMLDTD.h
!HTRules.obj :	HTRules.c HTRules.h HTUtils.h Version.make
!HTMIME.obj :	HTMIME.c HTMIME.h HTUtils.h HTList.h
!HTTelnet.obj :	HTTelnet.c HTTelnet.h HTUtils.h
!HTWAIS.obj :	HTWAIS.c HTWAIS.h HTUtils.h HTList.h
!HTWSRC.obj :	HTWSRC.c HTWSRC.h HTUtils.h HTList.h
!HTAccess.obj :	HTAccess.c HTAccess.h HTUtils.h
!HTAnchor.obj :	HTAnchor.c HTAnchor.h HTUtils.h HTList.h
!HTFile.obj :	HTFile.c HTFile.h HTUtils.h HTVMSUtils.h
!HTFormat.obj :	HTFormat.c HTFormat.h HTUtils.h HTML.h SGML.h HTPlain.h HTMLGen.h HTList.h
!HTFTP.obj :	HTFTP.c HTFTP.h HTUtils.h
!HTGopher.obj :	HTGopher.c HTGopher.h HTUtils.h HTList.h
!HTFinger.obj :	HTFinger.c HTFinger.h HTUtils.h HTList.h
!HTNews.obj :	HTNews.c HTNews.h HTUtils.h HTList.h
!HTParse.obj :	HTParse.c HTParse.h HTUtils.h
!HTStyle.obj :	HTStyle.c HTStyle.h HTUtils.h
!HTTCP.obj :	HTTCP.c HTTCP.h HTUtils.h www_tcp.h
!HTTP.obj :	HTTP.c HTTP.h HTUtils.h
!SGML.obj :	SGML.c SGML.h HTUtils.h
!HTAABrow.obj :	HTAABrow.c HTUtils.h
!HTAAProt.obj :	HTAAProt.c HTUtils.h
!HTAAUtil.obj :	HTAAUtil.c HTUtils.h
!HTGroup.obj :	HTGroup.c HTUtils.h
!HTLex.obj :	HTLex.c HTUtils.h
!HTAssoc.obj :	HTAssoc.c HTAssoc.h HTAAUtil.h HTString.h
!HTUU.obj :	HTUU.c HTUU.h HTUtils.h
!HTVMSUtils.obj :	HTVMSUtils.c HTVMSUtils.h HTUtils.h
