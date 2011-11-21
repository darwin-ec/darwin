;darwin nsis installer

!include "MUI2.nsh"
!include "fileassoc.nsh"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp" ; optional ;
!define MUI_ABORTWARNING


Name "DARWIN"

;default install directory
;InstallDir "$PROGRAMFILES\darwin-2.0\" <-- old version did not force darwin-2.0 on end of browse choice
;leaving backslash off of end makes sure "darwin-2.0" forder is created inside user's chosen
;folder after use of browser
;InstallDir $PROGRAMFILES\darwin-2.0 <-- no longer, since problems here with Windows 7 & Vista
InstallDir C:\darwinPhotoId\darwin-2.0

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
outfile darwin-2.22-setup.exe



;Need at least one of these
Section "Installer Section"
	SetOutPath $INSTDIR
	
	;Extract a zip file with all the current setup
	;File darwin.zip
	
	
	;better idea, add the directory recursively
	File /r /x *.nsi /x Thumbs.db /x darwin_splash.bmp /x darwin-*-setup.exe /x darwin-*-upgrade.exe /x .svn /x stuff *
	
	;Add some Start Menu shortcuts ($SMPROGRAMS)
	CreateDirectory "$SMPROGRAMS\darwin-2.0"
	;link.lnk target.file [parameters [icon.file [icon_index_number [start_options [keyboard_shortcut [description]]]]]]
	SetOutPath $INSTDIR\system\bin
	CreateShortCut "$SMPROGRAMS\darwin-2.0\Run Darwin.lnk" "$INSTDIR\system\bin\darwin.exe" "" "$INSTDIR\system\bin\darwin.exe" 0
	SetOutPath $INSTDIR
	CreateShortCut "$SMPROGRAMS\darwin-2.0\backups.lnk" "$INSTDIR\backups\"
	CreateShortCut "$SMPROGRAMS\darwin-2.0\survey areas.lnk" "$INSTDIR\surveyAreas\"
	CreateShortCut "$SMPROGRAMS\darwin-2.0\help.lnk" "$INSTDIR\docs\usersguide.htm"
	CreateShortCut "$SMPROGRAMS\darwin-2.0\uninstall.lnk" "$INSTDIR\uninstall.exe"
	
	;Desktop ShortCut
	SetOutPath $INSTDIR\system\bin
	CreateShortCut "$DESKTOP\darwin-2.0.lnk" "$INSTDIR\system\bin\darwin.exe" "" "$INSTDIR\system\bin\darwin.exe" 0
	SetOutPath $INSTDIR

	;Environmental Variable
	;http://nsis.sourceforge.net/Setting_Environment_Variables_Examples
	
	;Associate .finz with darwin.exe
	!insertmacro APP_ASSOCIATE "finz" "darwin.FinFile" "Darwin Finz" "$INSTDIR\system\bin\darwin.exe,0" \
	     "Open with darwin" "$INSTDIR\system\bin\darwin.exe $\"%1$\""	
	
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

	;Prompt to preserve data!!!!!

	MessageBox MB_YESNO "Would you like to keep all data (image files and catelog)?" IDYES next

		;remove data folders
		RMDIR /r "$INSTDIR\surveyAreas"
		RMDIR /r "$INSTDIR\backups"

	next:

	;remove non data files and folders
	Delete "$INSTDIR\darwin-2.0-readme.txt"
	Delete "$INSTDIR\fileassoc.nsh"
	Delete "$INSTDIR\license.rtf"
	Delete /REBOOTOK "$INSTDIR\uninstall.exe"
	RMDIR /r "$INSTDIR\licenses-etc"
	RMDIR /r "$INSTDIR\docs"
	RMDIR /r "$INSTDIR\system"
	
	;NOTE: we NEVER remove the install folder, just in case the user forces it
	;to some pathological place like C:\Windows

	;Remove shortcuts
	RMDir /r /REBOOTOK "$SMPROGRAMS\darwin-2.0"
	Delete /REBOOTOK "$DESKTOP\darwin-2.0.lnk"
	
	;Remove file association
	!insertmacro APP_UNASSOCIATE "finz" "darwin.FinFile"
	
	;Always remember to remove key from Add/Remove Programs
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Darwin"
	DeleteRegKey HKLM "Software\darwin"

SectionEnd


