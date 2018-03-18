@echo off
@rem $LynxId: develop.bat,v 1.2 2018/03/18 23:13:30 tom Exp $
@rem ensure that all IDE files are writable

attrib -r *.bat /s
attrib -r *.sln /s
attrib -r *.vcxproj /s