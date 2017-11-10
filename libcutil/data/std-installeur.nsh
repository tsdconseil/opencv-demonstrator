; Script d'installation std

;!define DESCRIPTIF      "Traitement d'image Neobie"
;!define FICHIER_SORTIE  "neobie-test-iu-1.4.0-win32-i386.exe"
;!define DOSSIER_INSTALL TSDConseil
;!define FICHIERS	 ../build/release/iu.exe
;!define NOM_EXE	 iu.exe

!include "MUI.nsh"

Name $DESCRIPTIF
OutFile ../build/${FICHIER_SORTIE}

SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
XPStyle on

;Default installation folder
InstallDir "$PROGRAMFILES\${DOSSIER_INSTALL}"
  
;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\${DOSSIER_INSTALL}" ""

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

  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "English" 
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"


;--------------------------------
;Installer Sections

;--------------------------------

Section "NeobieTest" SecCkernide
  SetOutPath "$INSTDIR"
  ; Logiciel de configuration
  !ifdef FICHIERS
  File ${FICHIERS}
  !endif
#  !echo "EXEDIR = $EXEDIR"
#  !echo $EXEDIR
  File ..\build\release\${NOM_EXE}
  SetOutPath "$INSTDIR\data"
  File /r "..\build\release\data\*"
!ifndef MODE_PATCH
  SetOutPath "$INSTDIR\share"
  File /r ".\share\*"
!endif

  SetOutPath "$INSTDIR"
!ifndef MODE_PATCH
  File "c:\ocv301\build\install\x86\mingw\bin\*.dll"
  File "c:\gtkmm340_mingw462\bin\*.dll"
!endif

  CreateDirectory "$SMPROGRAMS\${DOSSIER_INSTALL}"
  CreateShortCut "$SMPROGRAMS\${DOSSIER_INSTALL}\${DESCRIPTIF}.lnk" "$INSTDIR\${NOM_EXE}" "" "$INSTDIR\data\img\opencv.ico"
  CreateShortCut "$DESKTOP\Neobie test traitements.lnk" "$INSTDIR\${NOM_EXE}" "" "$INSTDIR\data\img\opencv.ico"
  CreateShortCut "$SMPROGRAMS\${DOSSIER_INSTALL}\Désinstallation.lnk" "$INSTDIR\Uninstall.exe"

  ;Store installation folder
  WriteRegStr HKCU "Software\${DOSSIER_INSTALL}" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd





;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecCkernide ${LANG_FRENCH} "${DESCRIPTIF}"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCkernide} $(DESC_SecCkernide)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
Section "Uninstall"
  IfFileExists "$DESKTOP\${DESCRIPTIF}.lnk" 0 Suite
  Delete "$DESKTOP\${DESCRIPTIF}.lnk"
  Suite:
  DeleteRegKey /ifempty HKCU "Software\${DOSSIER_INSTALL}"
  RMDir /r /REBOOTOK "$SMPROGRAMS\${DOSSIER_INSTALL}\*"
  RMDir /r /REBOOTOK $INSTDIR
SectionEnd


