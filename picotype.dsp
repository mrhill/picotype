# Microsoft Developer Studio Project File - Name="picotype" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=picotype - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "picotype.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "picotype.mak" CFG="picotype - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "picotype - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "picotype - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "picotype - Win32 Release"

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
# ADD CPP /nologo /G6 /Zp4 /W3 /O2 /I "..\babel\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "UNICODE" /YX /FD /c
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "picotype - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "picotype___Win32_Debug"
# PROP BASE Intermediate_Dir "picotype___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /Zp4 /W3 /Gm /ZI /Od /I "..\babel\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "UNICODE" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "picotype - Win32 Release"
# Name "picotype - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\ptFont.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptFont_mem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptFont_win.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptFontMan.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptGC.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptGC16.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptGC8.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptGCGDI.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptImage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptImage_jpeglib.cpp

!IF  "$(CFG)" == "picotype - Win32 Release"

!ELSEIF  "$(CFG)" == "picotype - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\ptPalMan.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ptSprite.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\pt.h
# End Source File
# Begin Source File

SOURCE=.\src\ptdefs.h
# End Source File
# Begin Source File

SOURCE=.\src\ptFont.h
# End Source File
# Begin Source File

SOURCE=.\src\ptFont_mem.h
# End Source File
# Begin Source File

SOURCE=.\src\ptFont_win.h
# End Source File
# Begin Source File

SOURCE=.\src\ptFontMan.h
# End Source File
# Begin Source File

SOURCE=.\src\ptGC.h
# End Source File
# Begin Source File

SOURCE=.\src\ptGC16.h
# End Source File
# Begin Source File

SOURCE=.\src\ptGC8.h
# End Source File
# Begin Source File

SOURCE=.\src\ptGCGDI.h
# End Source File
# Begin Source File

SOURCE=.\src\ptImage.h
# End Source File
# Begin Source File

SOURCE=.\src\ptImage_jpeglib.h
# End Source File
# Begin Source File

SOURCE=.\src\ptPalMan.h
# End Source File
# Begin Source File

SOURCE=.\src\ptSprite.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Makefile
# End Source File
# Begin Source File

SOURCE=.\ptconfig.h
# End Source File
# End Target
# End Project
