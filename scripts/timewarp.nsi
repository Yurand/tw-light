;NSIS Setup Script

!include ver

!define PRODUCT_NAME "TW-Light"

;--------------------------------
;Configuration

OutFile ..\..\tw-light-setup-${VER_VERSION}.exe
SetCompressor lzma
SetCompressorDictSize 16
InstallDir $PROGRAMFILES\TW-Light
InstallDirRegKey HKLM SOFTWARE\TW-Light ""

;--------------------------------

;Include Modern UI
!include "MUI.nsh"

;--------------------------------
;Configuration

;Names
Name "TW-Light"
Caption "${PRODUCT_NAME} ${VER_VERSION} Setup"

;Interface Settings
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_ICON "..\tw-light.ico"
!define MUI_UNICON "..\tw-light.ico"

!define MUI_COMPONENTSPAGE_SMALLDESC

;Pages
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${PRODUCT_NAME}.\r\n\r\n\r\n$_CLICK"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\copying"
!insertmacro MUI_PAGE_COMPONENTS
Page custom CreateShortCutF
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_LINK "Visit the ${PRODUCT_NAME} website for the latest news"
!define MUI_FINISHPAGE_LINK_LOCATION "http://tw-light.berlios.de/"

!define MUI_FINISHPAGE_RUN "$INSTDIR\tw-light.exe"
!define MUI_FINISHPAGE_SHOWREADME $INSTDIR\data\readme.html

!define MUI_FINISHPAGE_NOREBOOTSUPPORT

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
;--------------------------------


;Reserve Files
  
  ;These files should be inserted before other files in the data block
  ;Keep these lines before any File command
  ;Only for BZIP2 (solid) compression
  
  ReserveFile "timewarp.ini"
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
;Variables

  Var INI_VALUE

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

!define SF_SELECTED 1

; The stuff to install
Section "TimeWarp Core (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\*.dll"
  File "..\tw-light.exe"
  File "..\INSTALL"

  SetOutPath $INSTDIR\gamedata
  File /r "..\gamedata\*.*"
  SetOutPath $INSTDIR\util
  File /r "..\util\*.*"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\TW-Light "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TW-Light" "DisplayName" "TW-Light (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TW-Light" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"

  WriteUninstaller "uninstall.exe"

!insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE "timewarp.ini" "Field 3" "State" 
StrCmp $INI_VALUE "1" "" +2
CreateShortCut "$DESKTOP\TW-Light.lnk" "$INSTDIR\tw-light.exe"
  
!insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE "timewarp.ini" "Field 2" "State"
 StrCmp $INI_VALUE "1" "" +5
  CreateDirectory "$SMPROGRAMS\TW-Light"
  CreateShortCut "$SMPROGRAMS\TW-Light\readme.lnk" "$INSTDIR\readme.html" "Readme.html"
  CreateShortCut "$SMPROGRAMS\TW-Light\tw-light.lnk" "$INSTDIR\tw-light.exe" "${PRODUCT_NAME}"

SectionEnd

Section "Source"
  SetOutPath $INSTDIR
  File "..\makefile"
  File "..\tw-light.rc"
  File "..\tw-light.ico"
  File "..\sources.lst"
  File "..\install-sh"

  SetOutPath $INSTDIR\source
  File /r "..\source\*.*"

  SetOutPath $INSTDIR\tests
  File /r "..\tests\*.*"
  
  SetOutPath $INSTDIR\mingw-libs
  File /r "..\mingw-libs\*.*"

SectionEnd
;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TW-Light"
  DeleteRegKey HKLM SOFTWARE\TW-Light


  RMDir  /r "$INSTDIR"

  ; remove group used
  RMDir /r "$SMPROGRAMS\TW-Light"

  ; remove shortcuts, if any
  Delete "$DESKTOP\TW-Light.lnk"

SectionEnd

;--------------------------------
;Installer Functions

Function .onInit
  ;Extract InstallOptions INI files
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "timewarp.ini"
FunctionEnd

Function CreateShortCutF
  !insertmacro MUI_HEADER_TEXT "${PRODUCT_NAME} Option" ""
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "timewarp.ini"
FunctionEnd

