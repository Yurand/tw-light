;NSIS Setup Script

!define VER_VERSION "0.4"
!define PRODUCT_NAME "TW-Light"
!define PRODUCT_WEB_SITE "http://tw-light.appspot.com"

;--------------------------------
;Configuration

OutFile ..\tw-light-setup-${VER_VERSION}.exe
SetCompressor /SOLID lzma
InstallDir $PROGRAMFILES\TW-Light
InstallDirRegKey HKLM SOFTWARE\TW-Light ""

RequestExecutionLevel admin

;--------------------------------

;Include Modern UI
!include "MUI2.nsh"

;--------------------------------
;Configuration

;Names
Name "TW-Light"
Caption "${PRODUCT_NAME} ${VER_VERSION} Setup"

;Interface Settings
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_ICON "..\tw-light.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall-full.ico"

!define MUI_COMPONENTSPAGE_SMALLDESC

;Pages
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${PRODUCT_NAME}.$\r$\n$\r$\n$_CLICK"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\copying"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_LINK "Visit the ${PRODUCT_NAME} website for the latest news"
!define MUI_FINISHPAGE_LINK_LOCATION "http://tw-light.appspot.com/"

!define MUI_FINISHPAGE_RUN "$INSTDIR\tw-light.exe"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
;--------------------------------

;--------------------------------
;Variables

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

; The stuff to install
Section "TimeWarp Core (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\*.dll"
  File "..\tw-light.exe"
  File "..\INSTALL"

  SetOutPath $INSTDIR\data
  File /r "..\data\*.txt"
  File /r "..\data\*.ini"
  File /r "..\data\*.dat"
  File /r "..\data\*.html"
  File /r "..\data\palette"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\TW-Light "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TW-Light" "DisplayName" "TW-Light (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TW-Light" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"

  WriteUninstaller "uninstall.exe"

  CreateShortCut "$DESKTOP\TW-Light.lnk" "$INSTDIR\tw-light.exe"
  
  CreateDirectory "$SMPROGRAMS\TW-Light"
  WriteIniStr "$INSTDIR\Website.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\TW-Light\Website.lnk" "$INSTDIR\Website.url"
  CreateShortCut "$SMPROGRAMS\TW-Light\tw-light.lnk" "$INSTDIR\tw-light.exe" "${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\TW-Light\uninstall.lnk" "$INSTDIR\uninstall.exe" "Uninstall"

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

