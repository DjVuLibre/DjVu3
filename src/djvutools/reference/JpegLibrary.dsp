# Microsoft Developer Studio Project File - Name="JpegLibrary" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=JpegLibrary - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JpegLibrary.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JpegLibrary.mak" CFG="JpegLibrary - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JpegLibrary - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "JpegLibrary - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "JpegLibrary - Win32 Release"

# PROP BASE Use_MFC
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f JpegLibrary.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "JpegLibrary.exe"
# PROP BASE Bsc_Name "JpegLibrary.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "cd .. && msdev djvutools.dsw /MAKE "libjpeg - Win32 Release""
# PROP Rebuild_Opt "/REBUILD"
# PROP Target_File "../../../Release/lib/libjpeg.lib"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "JpegLibrary - Win32 Debug"

# PROP BASE Use_MFC
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "JpegLibrary___Win32_Debug"
# PROP BASE Intermediate_Dir "JpegLibrary___Win32_Debug"
# PROP BASE Cmd_Line "NMAKE /f JpegLibrary.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "JpegLibrary.exe"
# PROP BASE Bsc_Name "JpegLibrary.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "JpegLibrary___Win32_Debug"
# PROP Intermediate_Dir "JpegLibrary___Win32_Debug"
# PROP Cmd_Line "cd .. && msdev djvutools.dsw /MAKE "libjpeg - Win32 Debug""
# PROP Rebuild_Opt "/REBUILD"
# PROP Target_File "../../../Debug/lib/libjpeg.lib"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "JpegLibrary - Win32 Release"
# Name "JpegLibrary - Win32 Debug"

!IF  "$(CFG)" == "JpegLibrary - Win32 Release"

!ELSEIF  "$(CFG)" == "JpegLibrary - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
