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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\MrSID\libjpeg" /I "..\3rd-party\libjpeg" /I "..\include" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /c
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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\MrSID\libjpeg" /I "..\3rd-party\libjpeg" /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /GZ /c
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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\MrSID\libjpeg" /I "..\3rd-party\libjpeg" /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /GZ /c
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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\MrSID\libjpeg" /I "..\3rd-party\libjpeg" /I "..\include" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /YX /FD /c
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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\MrSID\libjpeg" /I "..\3rd-party\libjpeg" /I "..\include" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /D "WIN64" /Wp64 /c
# SUBTRACT CPP /YX
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
# ADD CPP /nologo /MDd /W3 /GX /Od /I "..\..\..\MrSID\libjpeg" /I "..\3rd-party\libjpeg" /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /D "WIN64" /GZ /Wp64 /c
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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\MrSID\libjpeg" /I "..\3rd-party\libjpeg" /I "..\include" /D "NDEBUG" /D "NEED_JPEG_DECODER" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /D "WIN64" /Wp64 /c
# SUBTRACT CPP /YX
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
# ADD CPP /nologo /MTd /W3 /GX /Od /I "..\..\..\MrSID\libjpeg" /I "..\3rd-party\libjpeg" /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "DJVU_STATIC_LIBRARY" /D "NEED_JPEG_DECODER" /D "GCONTAINER_NO_MEMBER_TEMPLATES" /D "NEED_DJVU_MEMORY" /D "NEED_DJVU_PROGRESS" /FR /GZ /Wp64 /c
# SUBTRACT CPP /YX
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
DEP_CPP_ARRAY=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	
# End Source File
# Begin Source File

SOURCE=.\BSByteStream.cpp
DEP_CPP_BSBYT=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\BSByteStream.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GOS.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\ZPCodec.h"\
	
# End Source File
# Begin Source File

SOURCE=.\ByteStream.cpp
DEP_CPP_BYTES=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GOS.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DataPool.cpp
DEP_CPP_DATAP=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DataPool.h"\
	".\debug.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GOS.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\IFFByteStream.h"\
	
NODEP_CPP_DATAP=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\debug.cpp
DEP_CPP_DEBUG=\
	"..\include\djvu.h"\
	".\debug.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	
NODEP_CPP_DEBUG=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVmDir.cpp
DEP_CPP_DJVMD=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\BSByteStream.h"\
	".\ByteStream.h"\
	".\debug.h"\
	".\DjVmDir.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\ZPCodec.h"\
	
NODEP_CPP_DJVMD=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVmDir0.cpp
DEP_CPP_DJVMDI=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\debug.h"\
	".\djvmdir0.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVmDoc.cpp
DEP_CPP_DJVMDO=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DataPool.h"\
	".\debug.h"\
	".\DjVmDir.h"\
	".\DjVmDoc.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GOS.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\IFFByteStream.h"\
	
NODEP_CPP_DJVMDO=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuAnno.cpp
DEP_CPP_DJVUA=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\BSByteStream.h"\
	".\ByteStream.h"\
	".\debug.h"\
	".\DjVuAnno.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GMapAreas.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\IFFByteStream.h"\
	".\ZPCodec.h"\
	
NODEP_CPP_DJVUA=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuDocEditor.cpp
DEP_CPP_DJVUD=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DataPool.h"\
	".\debug.h"\
	".\DjVmDir.h"\
	".\djvmdir0.h"\
	".\DjVmDoc.h"\
	".\DjVuAnno.h"\
	".\DjVuDocEditor.h"\
	".\DjVuDocument.h"\
	".\DjVuFile.h"\
	".\DjVuFileCache.h"\
	".\DjVuGlobal.h"\
	".\DjVuImage.h"\
	".\DjVuInfo.h"\
	".\DjVuNavDir.h"\
	".\DjVuPalette.h"\
	".\DjVuPort.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GMapAreas.h"\
	".\GOS.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\GURL.h"\
	".\IFFByteStream.h"\
	".\IWImage.h"\
	".\JB2Image.h"\
	".\ZPCodec.h"\
	
NODEP_CPP_DJVUD=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuDocument.cpp
DEP_CPP_DJVUDO=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DataPool.h"\
	".\debug.h"\
	".\DjVmDir.h"\
	".\djvmdir0.h"\
	".\DjVmDoc.h"\
	".\DjVuAnno.h"\
	".\DjVuDocument.h"\
	".\DjVuFile.h"\
	".\DjVuFileCache.h"\
	".\DjVuGlobal.h"\
	".\DjVuImage.h"\
	".\DjVuInfo.h"\
	".\DjVuNavDir.h"\
	".\DjVuPalette.h"\
	".\DjVuPort.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GMapAreas.h"\
	".\GOS.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\GURL.h"\
	".\IFFByteStream.h"\
	".\IWImage.h"\
	".\JB2Image.h"\
	".\ZPCodec.h"\
	
NODEP_CPP_DJVUDO=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuDumpHelper.cpp
DEP_CPP_DJVUDU=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DataPool.h"\
	".\DjVmDir.h"\
	".\DjVuDumpHelper.h"\
	".\DjVuGlobal.h"\
	".\DjVuInfo.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\IFFByteStream.h"\
	
NODEP_CPP_DJVUDU=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuErrorList.cpp
DEP_CPP_DJVUE=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DataPool.h"\
	".\DjVmDir.h"\
	".\DjVmDoc.h"\
	".\DjVuErrorList.h"\
	".\DjVuGlobal.h"\
	".\DjVuPort.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GOS.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\GURL.h"\
	
NODEP_CPP_DJVUE=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuFile.cpp
DEP_CPP_DJVUF=\
	"..\3rd-party\libjpeg\jconfig.h"\
	"..\3rd-party\libjpeg\jerror.h"\
	"..\3rd-party\libjpeg\jinclude.h"\
	"..\3rd-party\libjpeg\jmorecfg.h"\
	"..\3rd-party\libjpeg\jpegint.h"\
	"..\3rd-party\libjpeg\jpeglib.h"\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\BSByteStream.h"\
	".\ByteStream.h"\
	".\DataPool.h"\
	".\debug.h"\
	".\DjVuAnno.h"\
	".\DjVuFile.h"\
	".\DjVuGlobal.h"\
	".\DjVuInfo.h"\
	".\DjVuNavDir.h"\
	".\DjVuPalette.h"\
	".\DjVuPort.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GMapAreas.h"\
	".\GOS.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\GURL.h"\
	".\IFFByteStream.h"\
	".\IWImage.h"\
	".\JB2Image.h"\
	".\JPEGDecoder.h"\
	".\MMRDecoder.h"\
	".\ZPCodec.h"\
	
NODEP_CPP_DJVUF=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuFileCache.cpp
DEP_CPP_DJVUFI=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DataPool.h"\
	".\debug.h"\
	".\DjVuFile.h"\
	".\DjVuFileCache.h"\
	".\DjVuGlobal.h"\
	".\DjVuInfo.h"\
	".\DjVuNavDir.h"\
	".\DjVuPalette.h"\
	".\DjVuPort.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\GURL.h"\
	".\IFFByteStream.h"\
	".\IWImage.h"\
	".\JB2Image.h"\
	".\ZPCodec.h"\
	
NODEP_CPP_DJVUFI=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuGlobal.cpp
DEP_CPP_DJVUG=\
	"..\include\djvu.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GOS.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	
NODEP_CPP_DJVUG=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuGlobalMemory.cpp
DEP_CPP_DJVUGL=\
	"..\include\djvu.h"\
	".\DjVuGlobal.h"\
	".\GException.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuImage.cpp
DEP_CPP_DJVUI=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DataPool.h"\
	".\debug.h"\
	".\DjVmDir.h"\
	".\djvmdir0.h"\
	".\DjVmDoc.h"\
	".\DjVuAnno.h"\
	".\DjVuDocument.h"\
	".\DjVuFile.h"\
	".\DjVuFileCache.h"\
	".\DjVuGlobal.h"\
	".\DjVuImage.h"\
	".\DjVuInfo.h"\
	".\DjVuNavDir.h"\
	".\DjVuPalette.h"\
	".\DjVuPort.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GMapAreas.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GScaler.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\GURL.h"\
	".\IFFByteStream.h"\
	".\IWImage.h"\
	".\JB2Image.h"\
	".\ZPCodec.h"\
	
NODEP_CPP_DJVUI=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuInfo.cpp
DEP_CPP_DJVUIN=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\DjVuInfo.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	
NODEP_CPP_DJVUIN=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuMessage.cpp
DEP_CPP_DJVUM=\
	"..\include\djvu.h"\
	".\DjVuGlobal.h"\
	".\DjVuMessage.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\parseoptions.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuNavDir.cpp
DEP_CPP_DJVUN=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\debug.h"\
	".\DjVuGlobal.h"\
	".\DjVuNavDir.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GOS.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\GURL.h"\
	
NODEP_CPP_DJVUN=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuPalette.cpp
DEP_CPP_DJVUP=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\BSByteStream.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\DjVuPalette.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\ZPCodec.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DjVuPort.cpp
DEP_CPP_DJVUPO=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DataPool.h"\
	".\debug.h"\
	".\DjVmDir.h"\
	".\djvmdir0.h"\
	".\DjVmDoc.h"\
	".\DjVuAnno.h"\
	".\DjVuDocument.h"\
	".\DjVuFile.h"\
	".\DjVuFileCache.h"\
	".\DjVuGlobal.h"\
	".\DjVuImage.h"\
	".\DjVuInfo.h"\
	".\DjVuMessage.h"\
	".\DjVuNavDir.h"\
	".\DjVuPalette.h"\
	".\DjVuPort.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GMapAreas.h"\
	".\GOS.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\GURL.h"\
	".\IFFByteStream.h"\
	".\IWImage.h"\
	".\JB2Image.h"\
	".\ZPCodec.h"\
	
NODEP_CPP_DJVUPO=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GBitmap.cpp
DEP_CPP_GBITM=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	
NODEP_CPP_GBITM=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GContainer.cpp
DEP_CPP_GCONT=\
	"..\include\djvu.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GException.cpp
DEP_CPP_GEXCE=\
	"..\include\djvu.h"\
	".\debug.h"\
	".\DjVuGlobal.h"\
	".\DjVuMessage.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GIFFManager.cpp
DEP_CPP_GIFFM=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\debug.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\giffmanager.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\IFFByteStream.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GMapAreas.cpp
DEP_CPP_GMAPA=\
	"..\include\djvu.h"\
	".\debug.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GMapAreas.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GOS.cpp
DEP_CPP_GOS_C=\
	"..\include\djvu.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GOS.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	
NODEP_CPP_GOS_C=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GPixmap.cpp
DEP_CPP_GPIXM=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	
NODEP_CPP_GPIXM=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GRect.cpp
DEP_CPP_GRECT=\
	"..\include\djvu.h"\
	".\DjVuGlobal.h"\
	".\GException.h"\
	".\GRect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GScaler.cpp
DEP_CPP_GSCAL=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GScaler.h"\
	".\GSmartPointer.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GSmartPointer.cpp
DEP_CPP_GSMAR=\
	"..\include\djvu.h"\
	".\DjVuGlobal.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GThreads.h"\
	
NODEP_CPP_GSMAR=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GString.cpp
DEP_CPP_GSTRI=\
	"..\include\djvu.h"\
	".\debug.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GThreads.cpp
DEP_CPP_GTHRE=\
	"..\include\djvu.h"\
	".\DjVuGlobal.h"\
	".\DjVuMessage.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	
NODEP_CPP_GTHRE=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GURL.cpp
DEP_CPP_GURL_=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GOS.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\GURL.h"\
	
NODEP_CPP_GURL_=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\IFFByteStream.cpp
DEP_CPP_IFFBY=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\IFFByteStream.h"\
	
# End Source File
# Begin Source File

SOURCE=.\IWImage.cpp
DEP_CPP_IWIMA=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\IFFByteStream.h"\
	".\IWImage.h"\
	".\IWTransform.h"\
	".\ZPCodec.h"\
	
# End Source File
# Begin Source File

SOURCE=.\IWTransform.cpp
DEP_CPP_IWTRA=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GOS.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\IWTransform.h"\
	".\MMX.h"\
	
# End Source File
# Begin Source File

SOURCE=.\JB2Image.cpp
DEP_CPP_JB2IM=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\JB2Image.h"\
	".\ZPCodec.h"\
	
NODEP_CPP_JB2IM=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\JPEGDecoder.cpp
DEP_CPP_JPEGD=\
	"..\3rd-party\libjpeg\jconfig.h"\
	"..\3rd-party\libjpeg\jerror.h"\
	"..\3rd-party\libjpeg\jinclude.h"\
	"..\3rd-party\libjpeg\jmorecfg.h"\
	"..\3rd-party\libjpeg\jpegint.h"\
	"..\3rd-party\libjpeg\jpeglib.h"\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GPixmap.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\JPEGDecoder.h"\
	
# End Source File
# Begin Source File

SOURCE=.\MMRDecoder.cpp
DEP_CPP_MMRDE=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GBitmap.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GRect.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\JB2Image.h"\
	".\MMRDecoder.h"\
	".\ZPCodec.h"\
	
# End Source File
# Begin Source File

SOURCE=.\MMX.cpp
DEP_CPP_MMX_C=\
	"..\include\djvu.h"\
	".\DjVuGlobal.h"\
	".\MMX.h"\
	
# End Source File
# Begin Source File

SOURCE=.\parseoptions.cpp
DEP_CPP_PARSE=\
	"..\include\djvu.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\GString.h"\
	".\GThreads.h"\
	".\parseoptions.h"\
	
NODEP_CPP_PARSE=\
	".\jri.h"\
	
# End Source File
# Begin Source File

SOURCE=.\ZPCodec.cpp
DEP_CPP_ZPCOD=\
	"..\include\djvu.h"\
	".\Arrays.h"\
	".\ByteStream.h"\
	".\DjVuGlobal.h"\
	".\GContainer.h"\
	".\GException.h"\
	".\GSmartPointer.h"\
	".\ZPCodec.h"\
	
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

SOURCE=.\GURL.h
# End Source File
# Begin Source File

SOURCE=.\IFFByteStream.h
# End Source File
# Begin Source File

SOURCE=.\IWImage.h
# End Source File
# Begin Source File

SOURCE=.\IWTransform.h
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

SOURCE=.\ZPCodec.h
# End Source File
# End Group
# End Target
# End Project
