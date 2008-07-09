# Microsoft Developer Studio Project File - Name="darwin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=darwin - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "darwin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "darwin.mak" CFG="darwin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "darwin - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "darwin - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "darwin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /GX /O2 /I ".." /I "..\gtk\include\\" /I "..\gtk\lib\glib-2.0\include" /I "..\gtk\lib\gtk-2.0\include" /I "..\png\libpng\include" /I "..\png\zlib1\include" /D "CRTDLL2" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 zdll.lib libpng.lib gthread-2.0.lib gmodule-2.0.lib Ws2_32.lib iconv.lib intl.lib atk-1.0.lib pango-1.0.lib gdk_pixbuf-2.0.lib gobject-2.0.lib glib-2.0.lib gdk-win32-2.0.lib jpeg.lib gtk-win32-2.0.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"mainCRTStartup" /subsystem:console /incremental:yes /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrtd.lib" /libpath:"..\gtk\lib" /libpath:"..\jpeg-6b\lib" /libpath:"..\png\libpng\lib" /libpath:"..\png\zlib1\lib"
# SUBTRACT LINK32 /verbose /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "darwin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /Gm /GX /ZI /Od /I ".." /I "..\png\libpng\include" /I "..\png\zlib1\include" /I "..\gtk\include\\" /I "..\gtk\lib\glib-2.0\include" /I "..\gtk\lib\gtk-2.0\include" /D "CRTDLL2" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 zdll.lib libpng.lib gthread-2.0.lib gmodule-2.0.lib Ws2_32.lib iconv.lib intl.lib atk-1.0.lib pango-1.0.lib gdk_pixbuf-2.0.lib gobject-2.0.lib glib-2.0.lib gdk-win32-2.0.lib jpeg.lib gtk-win32-2.0.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"..\gtk\lib" /libpath:"..\jpeg-6b\lib" /libpath:"..\png\libpng\lib" /libpath:"..\png\zlib1\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "darwin - Win32 Release"
# Name "darwin - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\interface\AboutDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\AlterBrightnessDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\matching\AreaMatch.cxx
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\BinaryImage.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\CatalogSchemeDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Chain.cxx
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\ColorImage.cxx
# End Source File
# Begin Source File

SOURCE=..\src\ConfigFile.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Contour.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\ContourInfoDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\conversions.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\CreateDatabaseDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Database.cxx
# End Source File
# Begin Source File

SOURCE=..\src\DatabaseSupport.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\DataExportDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\DeleteOutlineDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\edge_detect.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\EnhanceContrastDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\ErrorDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\exif.cpp
# End Source File
# Begin Source File

SOURCE=..\src\FloatContour.cxx
# End Source File
# Begin Source File

SOURCE=..\src\math\gaussj.cxx
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\GrayImage.cxx
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\hysteresis.cxx
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\ImageMod.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\ImageViewDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\IntensityContour.cxx
# End Source File
# Begin Source File

SOURCE=..\src\IntensityContourCyan.cxx
# End Source File
# Begin Source File

SOURCE=..\src\main.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\MainWindow.cxx

!IF  "$(CFG)" == "darwin - Win32 Release"

!ELSEIF  "$(CFG)" == "darwin - Win32 Debug"

# ADD CPP /w /W0

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\mapContour.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\MappedContoursDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\matching\Match.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\MatchingDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\matching\MatchingQueue.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\MatchingQueueDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\matching\MatchResults.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\MatchResultsWindow.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\ModifyDatabaseWindow.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\NoMatchWindow.cxx
# End Source File
# Begin Source File

SOURCE=..\src\OldDatabase.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\OpenFileChooserDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\OptionsDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\Outline.cxx
# End Source File
# Begin Source File

SOURCE=..\png\pngImageSupport.c
# End Source File
# Begin Source File

SOURCE=..\src\interface\ResizeDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\SaveFileSelectionDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\src\snake.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\SplashWindow.cxx
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3.c
# End Source File
# Begin Source File

SOURCE=..\src\SQLiteDatabase.cxx
# End Source File
# Begin Source File

SOURCE=..\src\support.cxx
# End Source File
# Begin Source File

SOURCE=..\src\thumbnail.cxx
# End Source File
# Begin Source File

SOURCE=..\src\interface\TraceWindow.cxx
# End Source File
# Begin Source File

SOURCE=..\src\waveletUtil.cxx
# End Source File
# Begin Source File

SOURCE=..\src\wavelet\wlcError.c
# End Source File
# Begin Source File

SOURCE=..\src\wavelet\wlcMem.c
# End Source File
# Begin Source File

SOURCE=..\src\wavelet\wlcPackets.c
# End Source File
# Begin Source File

SOURCE=..\src\wavelet\wlcPWavelet.c
# End Source File
# Begin Source File

SOURCE=..\src\wavelet\wlcSBFilter.c
# End Source File
# Begin Source File

SOURCE=..\src\wavelet\wlcSWavelet.c
# End Source File
# Begin Source File

SOURCE=..\src\wavelet\wlcUtils.c
# End Source File
# Begin Source File

SOURCE=..\src\wavelet\wlRWavelet.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Src\interface\AboutDialog.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\AlterBrightnessDialog.h
# End Source File
# Begin Source File

SOURCE=..\src\matching\AreaMatch.h
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\BinaryImage.h
# End Source File
# Begin Source File

SOURCE=..\src\interface\CatalogSchemeDialog.h
# End Source File
# Begin Source File

SOURCE=..\src\Chain.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\ColorImage.h
# End Source File
# Begin Source File

SOURCE=..\Config.h
# End Source File
# Begin Source File

SOURCE=..\Src\ConfigFile.h
# End Source File
# Begin Source File

SOURCE=..\Src\constants.h
# End Source File
# Begin Source File

SOURCE=..\Src\Contour.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\ContourInfoDialog.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\conversions.h
# End Source File
# Begin Source File

SOURCE=..\src\interface\CreateDatabaseDialog.h
# End Source File
# Begin Source File

SOURCE=..\Src\Database.h
# End Source File
# Begin Source File

SOURCE=..\Src\DatabaseFin.h
# End Source File
# Begin Source File

SOURCE=..\src\DatabaseSupport.h
# End Source File
# Begin Source File

SOURCE=..\src\interface\DataExportDialog.h
# End Source File
# Begin Source File

SOURCE=..\src\interface\DeleteOutlineDialog.h
# End Source File
# Begin Source File

SOURCE=..\src\DummyDatabase.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\edge_detect.h
# End Source File
# Begin Source File

SOURCE=..\src\interface\EnhanceContrastDialog.h
# End Source File
# Begin Source File

SOURCE=..\Src\Error.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\ErrorDialog.h
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\exif.h
# End Source File
# Begin Source File

SOURCE=..\src\FloatContour.h
# End Source File
# Begin Source File

SOURCE=..\src\math\gaussj.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\GrayImage.h
# End Source File
# Begin Source File

SOURCE=..\src\interface\GtkCompat.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\Histogram.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\hysteresis.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\Image.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\ImageFile.h
# End Source File
# Begin Source File

SOURCE=..\src\image_processing\ImageMod.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\ImageViewDialog.h
# End Source File
# Begin Source File

SOURCE=..\src\IntensityContour.h
# End Source File
# Begin Source File

SOURCE=..\src\IntensityContourCyan.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\MainWindow.h
# End Source File
# Begin Source File

SOURCE=..\src\mapContour.h
# End Source File
# Begin Source File

SOURCE=..\src\interface\MappedContoursDialog.h
# End Source File
# Begin Source File

SOURCE=..\src\matching\Match.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\MatchingDialog.h
# End Source File
# Begin Source File

SOURCE=..\Src\Matching\MatchingQueue.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\MatchingQueueDialog.h
# End Source File
# Begin Source File

SOURCE=..\Src\Matching\MatchResults.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\MatchResultsWindow.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\Matrix.h
# End Source File
# Begin Source File

SOURCE=..\src\interface\ModifyDatabaseWindow.h
# End Source File
# Begin Source File

SOURCE=..\src\interface\NoMatchWindow.h
# End Source File
# Begin Source File

SOURCE=..\src\OldDatabase.h
# End Source File
# Begin Source File

SOURCE=..\src\interface\OpenFileChooserDialog.h
# End Source File
# Begin Source File

SOURCE=..\Src\Options.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\OptionsDialog.h
# End Source File
# Begin Source File

SOURCE=..\src\Outline.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\Pixel.h
# End Source File
# Begin Source File

SOURCE=..\png\pngFile.h
# End Source File
# Begin Source File

SOURCE=..\src\Point.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\ResizeDialog.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\SaveFileSelectionDialog.h
# End Source File
# Begin Source File

SOURCE=..\Src\Snake.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\SplashWindow.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3.h
# End Source File
# Begin Source File

SOURCE=..\src\SQLiteDatabase.h
# End Source File
# Begin Source File

SOURCE=..\Src\Support.h
# End Source File
# Begin Source File

SOURCE=..\src\thumbnail.h
# End Source File
# Begin Source File

SOURCE=..\Src\interface\TraceWindow.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\transform.h
# End Source File
# Begin Source File

SOURCE=..\Src\image_processing\Types.h
# End Source File
# Begin Source File

SOURCE=..\Src\Utility.h
# End Source File
# Begin Source File

SOURCE=..\src\waveletUtil.h
# End Source File
# Begin Source File

SOURCE=..\Src\Wavelet\Wlcore.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Icon.rc
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# End Group
# Begin Source File

SOURCE=..\pixmaps\addpoint.xpm
# End Source File
# Begin Source File

SOURCE=..\pixmaps\addpoint_cursor.xbm
# End Source File
# Begin Source File

SOURCE=.\temptemp.txt
# End Source File
# Begin Source File

SOURCE=".\to-do.txt"
# End Source File
# End Target
# End Project
