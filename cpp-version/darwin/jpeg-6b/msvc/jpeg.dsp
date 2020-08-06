# Microsoft Developer Studio Project File - Name="jpeg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=jpeg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jpeg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jpeg.mak" CFG="jpeg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jpeg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "jpeg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jpeg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "jpeg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\src" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\jpeg.lib"

!ENDIF 

# Begin Target

# Name "jpeg - Win32 Release"
# Name "jpeg - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\jcapimin.c
# End Source File
# Begin Source File

SOURCE=..\src\jcapistd.c
# End Source File
# Begin Source File

SOURCE=..\src\jccoefct.c
# End Source File
# Begin Source File

SOURCE=..\src\jccolor.c
# End Source File
# Begin Source File

SOURCE=..\src\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=..\src\jchuff.c
# End Source File
# Begin Source File

SOURCE=..\src\jcinit.c
# End Source File
# Begin Source File

SOURCE=..\src\jcmainct.c
# End Source File
# Begin Source File

SOURCE=..\src\jcmarker.c
# End Source File
# Begin Source File

SOURCE=..\src\jcmaster.c
# End Source File
# Begin Source File

SOURCE=..\src\jcomapi.c
# End Source File
# Begin Source File

SOURCE=..\src\jcparam.c
# End Source File
# Begin Source File

SOURCE=..\src\jcphuff.c
# End Source File
# Begin Source File

SOURCE=..\src\jcprepct.c
# End Source File
# Begin Source File

SOURCE=..\src\jcsample.c
# End Source File
# Begin Source File

SOURCE=..\src\jctrans.c
# End Source File
# Begin Source File

SOURCE=..\src\jdapimin.c
# End Source File
# Begin Source File

SOURCE=..\src\jdapistd.c
# End Source File
# Begin Source File

SOURCE=..\src\jdatadst.c
# End Source File
# Begin Source File

SOURCE=..\src\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=..\src\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=..\src\jdcolor.c
# End Source File
# Begin Source File

SOURCE=..\src\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=..\src\jdhuff.c
# End Source File
# Begin Source File

SOURCE=..\src\jdinput.c
# End Source File
# Begin Source File

SOURCE=..\src\jdmainct.c
# End Source File
# Begin Source File

SOURCE=..\src\jdmarker.c
# End Source File
# Begin Source File

SOURCE=..\src\jdmaster.c
# End Source File
# Begin Source File

SOURCE=..\src\jdmerge.c
# End Source File
# Begin Source File

SOURCE=..\src\jdphuff.c
# End Source File
# Begin Source File

SOURCE=..\src\jdpostct.c
# End Source File
# Begin Source File

SOURCE=..\src\jdsample.c
# End Source File
# Begin Source File

SOURCE=..\src\jdtrans.c
# End Source File
# Begin Source File

SOURCE=..\src\jerror.c
# End Source File
# Begin Source File

SOURCE=..\src\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=..\src\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=..\src\jfdctint.c
# End Source File
# Begin Source File

SOURCE=..\src\jidctflt.c
# End Source File
# Begin Source File

SOURCE=..\src\jidctfst.c
# End Source File
# Begin Source File

SOURCE=..\src\jidctint.c
# End Source File
# Begin Source File

SOURCE=..\src\jidctred.c
# End Source File
# Begin Source File

SOURCE=..\src\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\src\jmemnobs.c
# End Source File
# Begin Source File

SOURCE=..\src\jquant1.c
# End Source File
# Begin Source File

SOURCE=..\src\jquant2.c
# End Source File
# Begin Source File

SOURCE=..\src\jutils.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
