$ v = 'f$verify(0)'
$!			BUILD-CHRTRANS.COM
$!
$!   Command file to build MAKEUCTB.EXE on VMS systems
$!   and then use it to create the chrtrans header files.
$!
$!   28-Jun-1997	F.Macrides		macrides@sci.wfeb.edu
$!	Initial version, for Lynx v2.7.1+fotemods
$!
$ ON CONTROL_Y THEN GOTO CLEANUP
$ ON ERROR THEN GOTO CLEANUP
$ CHRproc = f$environment("PROCEDURE")
$ CHRwhere = f$parse(CHRproc,,,"DEVICE") + f$parse(CHRproc,,,"DIRECTORY")
$!
$ if p1 .nes. ""
$   then
$      CHRcc_opts = "/DEBUG/NOOPT"
$      CHRlink_opts = "/DEBUG"
$   else
$      CHRcc_opts = ""
$      CHRlink_opts = ""
$ endif
$!
$ Compile_makeuctb:
$!================
$ v1 = f$verify(1)
$!
$!	Compile the Lynx [.SRC.CHRTRANS]makeuctb module.
$!
$  v1 = 'f$verify(0)'
$ IF f$trnlnm("VAXCMSG") .eqs. "DECC$MSG" .or. -
     f$trnlnm("DECC$CC_DEFAULT") .eqs. "/DECC" .or. -
     f$trnlnm("DECC$CC_DEFAULT") .eqs. "/VAXC"
$ THEN
$  CHRcompiler := "DECC"
$  v1 = f$verify(1)
$! DECC:
$  cc := cc/decc/prefix=all /nomember 'CHRcc_opts'-
	   /INCLUDE=([-],[--],[--.WWW.Library.Implementation]) 
$  v1 = 'f$verify(0)'
$ ELSE
$  IF f$search("gnu_cc:[000000]gcclib.olb") .nes. ""
$  THEN
$   CHRcompiler := "GNUC"
$   v1 = f$verify(1)
$! GNUC:
$   cc := gcc 'cc_opts'/INCLUDE=([-],[--],[--.WWW.Library.Implementation]) 
$   v1 = 'f$verify(0)'
$  ELSE
$   CHRcompiler := "VAXC"
$   v1 = f$verify(1)
$! VAXC:
$   cc := cc 'cc_opts'/INCLUDE=([-],[--],[--.WWW.Library.Implementation]) 
$   v1 = 'f$verify(0)'
$  ENDIF
$ ENDIF
$!
$ v1 = f$verify(1)
$ cc makeuctb
$ v1 = 'f$verify(0)'
$!
$ Link_makeuctb:
$!=============
$ v1 = f$verify(1)
$!
$!	Link the Lynx [.SRC.CHRTRANS]makeuctb module.
$!
$ link/exe=makeuctb.exe'link_opts' makeuctb, -
sys$disk:[-]'CHRcompiler'.opt/opt
$ v1 = 'f$verify(0)'
$!
$ Create_headers:
$!==============
$ v1 = f$verify(1)
$!
$!	Create the Lynx [.SRC.CHRTRANS] header files.
$!
$ makeuctb := $'CHRwhere'makeuctb
$ define/user sys$output 'CHRwhere'iso01_uni.h
$ makeuctb iso01_uni.tbl
$ define/user sys$output 'CHRwhere'iso02_uni.h
$ makeuctb iso02_uni.tbl
$ define/user sys$output 'CHRwhere'def7_uni.h
$ makeuctb def7_uni.tbl
$ define/user sys$output 'CHRwhere'iso03_uni.h
$ makeuctb iso03_uni.tbl
$ define/user sys$output 'CHRwhere'iso04_uni.h
$ makeuctb iso04_uni.tbl
$ define/user sys$output 'CHRwhere'iso05_uni.h
$ makeuctb iso05_uni.tbl
$ define/user sys$output 'CHRwhere'iso07_uni.h
$ makeuctb iso07_uni.tbl
$ define/user sys$output 'CHRwhere'iso09_uni.h
$ makeuctb iso09_uni.tbl
$ define/user sys$output 'CHRwhere'iso10_uni.h
$ makeuctb iso10_uni.tbl
$ define/user sys$output 'CHRwhere'koi8r_uni.h
$ makeuctb koi8r_uni.tbl
$ define/user sys$output 'CHRwhere'cp437_uni.h
$ makeuctb cp437_uni.tbl
$ define/user sys$output 'CHRwhere'cp850_uni.h
$ makeuctb cp850_uni.tbl
$ define/user sys$output 'CHRwhere'cp852_uni.h
$ makeuctb cp852_uni.tbl
$ define/user sys$output 'CHRwhere'cp1250_uni.h
$ makeuctb cp1250_uni.tbl
$ define/user sys$output 'CHRwhere'cp1251_uni.h
$ makeuctb cp1251_uni.tbl
$ define/user sys$output 'CHRwhere'cp1252_uni.h
$ makeuctb cp1252_uni.tbl
$ define/user sys$output 'CHRwhere'utf8_uni.h
$ makeuctb utf8_uni.tbl
$ define/user sys$output 'CHRwhere'mnemonic_suni.h
$ makeuctb mnemonic_suni.tbl
$ define/user sys$output 'CHRwhere'mnem_suni.h
$ makeuctb mnem_suni.tbl
$ define/user sys$output 'CHRwhere'rfc_suni.h
$ makeuctb rfc_suni.tbl
$ v1 = 'f$verify(0)'
$ exit
$!
$ CLEANUP:
$    v1 = 'f$verify(0)'
$    write sys$output "Default directory:"
$    show default
$    v1 = f$verify(v)
$ exit
