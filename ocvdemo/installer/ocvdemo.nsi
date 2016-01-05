; Script d'installation OCV demo

;--------------------------------
;Include Modern UI
  !include "MUI.nsh"
  !define GET_JAVA_URL "http://www.java.com"

;-------------------------
;Variables
;-------------------------
  Var JAVA_VER

  !define TEMP $R0
  !define TEMP2 $R1
  !define TEMP3 $R4
  !define VAL1 $R2
  !define VAL2 $R3
  
  # Splash is the image that pops up on installer startup
  !define SPLASH_IMAGE "splash.bmp"

;--------------------------------
;General

  ;Name and file
  Name "Demonstration OpenCV"
  OutFile "ocvdemo-1.02-win32-i386.exe"

SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
XPStyle on

  ;Default installation folder
  InstallDir "$PROGRAMFILES\TSDConseil"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\TSDConseil" ""

;--------------------------------
;Interface Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp" ; optional
  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  ;!insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Contrib\Modern UI\License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English" 
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"


;--------------------------------
;Installer Sections

;--------------------------------

Section "OCVDemo" SecCkernide
  SetOutPath "$INSTDIR"
  ; Logiciel de configuration
  File "..\build\release\ocvdemo.exe"
  File "..\build\release\test-webcam.exe"
  ;File "C:\Tele\gtkmm-win32-runtime-2.14.1-2.exe"
  SetOutPath "$INSTDIR\data"
  File /r "..\build\release\data\*"
  SetOutPath "$INSTDIR\share"
  File /r ".\share\*"

  SetOutPath "$INSTDIR"
;  File "c:\downloads\ocv\opencv\build\x86\mingw\bin\*.dll"
  File "c:\downloads\ocv3\release\install\x86\mingw\bin\*.dll"
  File "c:\gtkmm340_mingw462\bin\*.dll"

;  ReadRegStr $JAVA_VER HKLM "SOFTWARE\gtkmm\2.4" "Version"
;  DetailPrint "gtkmm version: $JAVA_VER"
;  StrCmp "2.14.1" "$JAVA_VER" SkipGtkmm GtkmmNotPresent
  
;  GtkmmNotPresent:
;  DetailPrint "Installing GTK runtime..."
;  ExecWait "$INSTDIR\gtkmm-win32-runtime-2.14.1-2.exe" $0
;  DetailPrint "GTK installation completed."

;  SkipGtkmm:
  CreateDirectory "$SMPROGRAMS\TSDConseil"
  CreateShortCut "$SMPROGRAMS\TSDConseil\OpenCV demo.lnk" "$INSTDIR\ocvdemo.exe" "" "$INSTDIR\data\img\opencv.ico"
  CreateShortCut "$SMPROGRAMS\TSDConseil\Test webcam.lnk" "$INSTDIR\test-webcam.exe" "" "$INSTDIR\data\img\opencv.ico"
  CreateShortCut "$DESKTOP\OpenCV Demo.lnk" "$INSTDIR\ocvdemo.exe" "" "$INSTDIR\data\img\opencv.ico"
  CreateShortCut "$SMPROGRAMS\TSDConseil\Uninstall OpenCV demo.lnk" "$INSTDIR\Uninstall.exe"

  ;Store installation folder
  WriteRegStr HKCU "Software\TSDConseil" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd





;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecCkernide ${LANG_FRENCH} "Installe logiciel de démonstration OpenCV"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCkernide} $(DESC_SecCkernide)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section
; TODO

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...


  IfFileExists "$DESKTOP\OpenCV Demo.lnk" 0 Suite
  Delete "$DESKTOP\OpenCV Demo.lnk"
  Suite:
  DeleteRegKey /ifempty HKCU "Software\TSDConseil"
  RMDir /r /REBOOTOK "$SMPROGRAMS\TSDConseil\*"
  RMDir /r /REBOOTOK $INSTDIR
  
SectionEnd


