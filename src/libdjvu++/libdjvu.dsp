# Microsoft Developer Studio Project File - Name="libdjvu" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libdjvu - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libdjvu.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libdjvu.mak" CFG="libdjvu - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libdjvu - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libdjvu - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libdjvu - Win32 Debug_md" (based on "Win32 (x86) Static Library")
!MESSAGE "libdjvu - Win32 Release_md" (based on "Win32 (x86) Static Library")
!MESSAGE "libdjvu - Win64 Release_md" (based on "Win32 (x86) Static Library")
!MESSAGE "libdjvu - Win64 Debug_md" (based on "Win32 (x86) Static Library")
!MESSAGE "libdjvu - Win64 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libdjvu - Win64 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libdjvu - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../3rd-party/libjpeg" /I "../include" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../Release/lib/LibDjVu.lib" /nodefaultlib

!ELSEIF  "$(CFG)" == "libdjvu - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../3rd-party/libjpeg" /I "../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../Debug/lib/LibDjVu.lib" /nodefaultlib

!ELSEIF  "$(CFG)" == "libdjvu - Win32 Debug_md"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libdjvu___Win32_Debug_md"
# PROP BASE Intermediate_Dir "libdjvu___Win32_Debug_md"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\lib_md_temp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\3rd-party\libjpeg" /I "..\..\..\MrSID\libjpeg" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\3rd-party\libjpeg" /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /nodefaultlib
# ADD LIB32 /nologo /out:"Debug\libdjvu_md.lib" /nodefaultlib

!ELSEIF  "$(CFG)" == "libdjvu - Win32 Release_md"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libdjvu___Win32_Release_md"
# PROP BASE Intermediate_Dir "libdjvu___Win32_Release_md"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release\"
# PROP Intermediate_Dir "Release\lib_md_temp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\3rd-party\libjpeg" /I "..\..\..\MrSID\libjpeg" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\3rd-party\libjpeg" /I "..\include" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /nodefaultlib
# ADD LIB32 /nologo /out:"Release\libdjvu_md.lib" /nodefaultlib

!ELSEIF  "$(CFG)" == "libdjvu - Win64 Release_md"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libdjvu___Win64_Release_md"
# PROP BASE Intermediate_Dir "libdjvu___Win64_Release_md"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libdjvu___Win64_Release_md"
# PROP Intermediate_Dir "libdjvu___Win64_Release_md"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\3rd-party\libjpeg" /I "..\..\..\MrSID\libjpeg" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\3rd-party\libjpeg" /I "..\include" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /D "WIN64" /Wp64 /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Release\libdjvu_md.lib" /nodefaultlib
# ADD LIB32 /nologo /out:"Release\libdjvu_md.lib" /nodefaultlib

!ELSEIF  "$(CFG)" == "libdjvu - Win64 Debug_md"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libdjvu___Win64_Debug_md"
# PROP BASE Intermediate_Dir "libdjvu___Win64_Debug_md"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libdjvu___Win64_Debug_md"
# PROP Intermediate_Dir "libdjvu___Win64_Debug_md"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\3rd-party\libjpeg" /I "..\..\..\MrSID\libjpeg" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Od /I "..\3rd-party\libjpeg" /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /D "WIN64" /GZ /Wp64 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug\libdjvu_md.lib" /nodefaultlib
# ADD LIB32 /nologo /out:"Debug\libdjvu_md.lib" /nodefaultlib

!ELSEIF  "$(CFG)" == "libdjvu - Win64 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libdjvu___Win64_Release"
# PROP BASE Intermediate_Dir "libdjvu___Win64_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libdjvu___Win64_Release"
# PROP Intermediate_Dir "libdjvu___Win64_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\3rd-party\libjpeg" /I "..\..\..\MrSID\libjpeg" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\3rd-party\libjpeg" /I "..\include" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /D "WIN64" /Wp64 /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /nodefaultlib
# ADD LIB32 /nologo /nodefaultlib

!ELSEIF  "$(CFG)" == "libdjvu - Win64 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libdjvu___Win64_Debug"
# PROP BASE Intermediate_Dir "libdjvu___Win64_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libdjvu___Win64_Debug"
# PROP Intermediate_Dir "libdjvu___Win64_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\3rd-party\libjpeg" /I "..\..\..\MrSID\libjpeg" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Od /I "..\3rd-party\libjpeg" /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /GZ /Wp64 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /nodefaultlib
# ADD LIB32 /nologo /nodefaultlib

!ENDIF 

# Begin Target

# Name "libdjvu - Win32 Release"
# Name "libdjvu - Win32 Debug"
# Name "libdjvu - Win32 Debug_md"
# Name "libdjvu - Win32 Release_md"
# Name "libdjvu - Win64 Release_md"
# Name "libdjvu - Win64 Debug_md"
# Name "libdjvu - Win64 Release"
# Name "libdjvu - Win64 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Arrays.cpp
# End Source File
# Begin Source File

SOURCE=.\BSByteStream.cpp
# End Source File
# Begin Source File

SOURCE=.\ByteStream.cpp
# End Source File
# Begin Source File

SOURCE=.\DataPool.cpp
# End Source File
# Begin Source File

SOURCE=.\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVmDir.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVmDir0.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVmDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuAnno.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuDocEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuDocument.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuDumpHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuErrorList.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuFile.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuFileCache.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuGlobal.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuGlobalMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuImage.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuNavDir.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuPalette.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuPort.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuText.cpp
# End Source File
# Begin Source File

SOURCE=.\GBitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\GContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\GException.cpp
# End Source File
# Begin Source File

SOURCE=.\GIFFManager.cpp
# End Source File
# Begin Source File

SOURCE=.\GMapAreas.cpp
# End Source File
# Begin Source File

SOURCE=.\GOS.cpp
# End Source File
# Begin Source File

SOURCE=.\GPixmap.cpp
# End Source File
# Begin Source File

SOURCE=.\GRect.cpp
# End Source File
# Begin Source File

SOURCE=.\GScaler.cpp
# End Source File
# Begin Source File

SOURCE=.\GSmartPointer.cpp
# End Source File
# Begin Source File

SOURCE=.\GString.cpp
# End Source File
# Begin Source File

SOURCE=.\GThreads.cpp
# End Source File
# Begin Source File

SOURCE=.\GUnicode.cpp
# End Source File
# Begin Source File

SOURCE=.\GURL.cpp
# End Source File
# Begin Source File

SOURCE=.\IFFByteStream.cpp
# End Source File
# Begin Source File

SOURCE=.\IW44EncodeCodec.cpp
# End Source File
# Begin Source File

SOURCE=.\IW44Image.cpp
# End Source File
# Begin Source File

SOURCE=.\JB2EncodeCodec.cpp
# End Source File
# Begin Source File

SOURCE=.\JB2Image.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEGDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\MMRDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\MMX.cpp
# End Source File
# Begin Source File

SOURCE=.\parseoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\UnicodeByteStream.cpp
# End Source File
# Begin Source File

SOURCE=.\XMLAnno.cpp
# End Source File
# Begin Source File

SOURCE=.\XMLTags.cpp
# End Source File
# Begin Source File

SOURCE=.\ZPCodec.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Arrays.h
# End Source File
# Begin Source File

SOURCE=.\BSByteStream.h
# End Source File
# Begin Source File

SOURCE=.\ByteStream.h
# End Source File
# Begin Source File

SOURCE=.\DataPool.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\DjVmDir.h
# End Source File
# Begin Source File

SOURCE=.\DjVmDoc.h
# End Source File
# Begin Source File

SOURCE=.\DjVuAnno.h
# End Source File
# Begin Source File

SOURCE=.\DjVuDocEditor.h
# End Source File
# Begin Source File

SOURCE=.\DjVuDocument.h
# End Source File
# Begin Source File

SOURCE=.\DjVuDumpHelper.h
# End Source File
# Begin Source File

SOURCE=.\DjVuErrorList.h
# End Source File
# Begin Source File

SOURCE=.\DjVuFile.h
# End Source File
# Begin Source File

SOURCE=.\DjVuFileCache.h
# End Source File
# Begin Source File

SOURCE=.\DjVuGlobal.h
# End Source File
# Begin Source File

SOURCE=.\DjVuImage.h
# End Source File
# Begin Source File

SOURCE=.\DjVuInfo.h
# End Source File
# Begin Source File

SOURCE=.\DjVuMessage.h
# End Source File
# Begin Source File

SOURCE=.\DjVuNavDir.h
# End Source File
# Begin Source File

SOURCE=.\DjVuPalette.h
# End Source File
# Begin Source File

SOURCE=.\DjVuPort.h
# End Source File
# Begin Source File

SOURCE=.\DjVuText.h
# End Source File
# Begin Source File

SOURCE=.\GBitmap.h
# End Source File
# Begin Source File

SOURCE=.\GContainer.h
# End Source File
# Begin Source File

SOURCE=.\GException.h
# End Source File
# Begin Source File

SOURCE=.\GMapAreas.h
# End Source File
# Begin Source File

SOURCE=.\GOS.h
# End Source File
# Begin Source File

SOURCE=.\GPixmap.h
# End Source File
# Begin Source File

SOURCE=.\GRect.h
# End Source File
# Begin Source File

SOURCE=.\GScaler.h
# End Source File
# Begin Source File

SOURCE=.\GSmartPointer.h
# End Source File
# Begin Source File

SOURCE=.\GString.h
# End Source File
# Begin Source File

SOURCE=.\GThreads.h
# End Source File
# Begin Source File

SOURCE=.\GUnicode.h
# End Source File
# Begin Source File

SOURCE=.\GURL.h
# End Source File
# Begin Source File

SOURCE=.\IFFByteStream.h
# End Source File
# Begin Source File

SOURCE=.\IW44Image.h
# End Source File
# Begin Source File

SOURCE=.\JB2Image.h
# End Source File
# Begin Source File

SOURCE=.\JPEGDecoder.h
# End Source File
# Begin Source File

SOURCE=.\MMRDecoder.h
# End Source File
# Begin Source File

SOURCE=.\MMX.h
# End Source File
# Begin Source File

SOURCE=.\parseoptions.h
# End Source File
# Begin Source File

SOURCE=.\UnicodeByteStream.h
# End Source File
# Begin Source File

SOURCE=.\XMLAnno.h
# End Source File
# Begin Source File

SOURCE=.\XMLTags.h
# End Source File
# Begin Source File

SOURCE=.\ZPCodec.h
# End Source File
# End Group
# End Target
# End Project
