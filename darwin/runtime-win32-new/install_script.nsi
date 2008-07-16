;darwin nsis installer

!include "MUI2.nsh"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp" ; optional ;
!define MUI_ABORTWARNING


Name "DARWIN"

;default install directory
InstallDir "$PROGRAMFILES\darwin"

;license to display (.txt or .rtf), .txt must use windows EOL (\r\n)
;LicenseData license.rtf
LicenseForceSelection checkbox

;To check for a registry values of previous install location use the following.
;Will override definstdir if string exists and is valid
InstallDirRegKey HKLM Software\darwin InstallLocation



;--------------------------------
;Pages
  Page custom brandimage "" ": Brand Image"
  !insertmacro MUI_PAGE_LICENSE "license.rtf"
  ;!insertmacro MUI_PAGE_COMPONENTS ;We only have one componenet/section currently
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  
  UninstPage custom un.brandimage "" ": Brand Image"
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  

;Add Branding
AddBrandingImage left 200 ;Must be ouside section

Function brandimage
  SetOutPath "$TEMP"
  SetFileAttributes darwin_splash.bmp temporary
  File darwin_splash.bmp
  SetBrandingImage "$TEMP\darwin_splash.bmp" /resizetofit
FunctionEnd

Function un.brandimage
  SetOutPath "$TEMP"
  SetFileAttributes darwin_splash.bmp temporary
  File darwin_splash.bmp
  SetBrandingImage "$TEMP\darwin_splash.bmp" /resizetofit
FunctionEnd
;Remove "Nullsoft Install System vX.XX"
;BrandingText " "

;name of install file to generate (e.g. setup.exe)
outfile setup.exe



;Need at least one of these
Section "Installer Section"
	SetOutPath $INSTDIR
	
	;Extract a zip file with all the current setup
	;File darwin.zip
	
	
	;better idea, add the directory recursively
	File /r /x *.nsi /x darwin_splash.bmp /x setup.exe *
	
	;Add some Start Menu shortcuts ($SMPROGRAMS)
	CreateDirectory "$SMPROGRAMS\darwin"
	;link.lnk target.file [parameters [icon.file [icon_index_number [start_options [keyboard_shortcut [description]]]]]]
	CreateShortCut "$SMPROGRAMS\darwin\darwin.lnk" "$INSTDIR\runDarwin.bat" "" "$INSTDIR\system\bin\darwin.exe" 0
	CreateShortCut "$SMPROGRAMS\darwin\backups.lnk" "$INSTDIR\backups\"
	CreateShortCut "$SMPROGRAMS\darwin\survey areas.lnk" "$INSTDIR\surveyAreas\"
	CreateShortCut "$SMPROGRAMS\darwin\uninstall.lnk" "$INSTDIR\uninstall.exe"
	
	;Desktop ShortCut
	CreateShortCut "$DESKTOP\darwin.lnk" "$INSTDIR\runDarwin.bat" "" "$INSTDIR\system\bin\darwin.exe" 0

	;Environmental Variable
	;http://nsis.sourceforge.net/Setting_Environment_Variables_Examples
	
	
	
	;Write the uninstaller and add to Add/Remove Programs before we finish
	WriteUninstaller $INSTDIR\uninstall.exe
	WriteRegStr HKLM "Software\darwin" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Darwin" \
                 "DisplayName" "DARWIN -- Fin Recognition Software"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Darwin" \
                 "UninstallString" "$INSTDIR\uninstall.exe"
SectionEnd


;Uninstall section--must start with un.
Section "un.Uninstaller Section"
	;Should really prompt to preserve data!!!!!

	MessageBox MB_YESNO "Would you like to keep all data (image files and catelog)?" IDYES done
		RMDir /r /REBOOTOK $INSTDIR ;remove absolutely everything
	done:

	;remove absolutely everything
	;RMDir /r /REBOOTOK $INSTDIR
	
	Delete *
	RMDIR /r "$INSTDIR\licenses-etc"
	RMDIR /r "$INSTDIR\docs"
	RMDIR /r "$INSTDIR\system"
	
	;Remove shortcuts
	RMDir /r /REBOOTOK "$SMPROGRAMS\darwin"
	Delete /REBOOTOK "$DESKTOP\darwin.lnk"
	
	;Always remember to remove key from Add/Remove Programs
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Darwin"
	DeleteRegKey HKLM "Software\darwin"
SectionEnd

