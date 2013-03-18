;NSIS Installer Script for QLandkarte GT PN

;NSIS References/Documentation 
;http://nsis.sourceforge.net/Docs/Modern%20UI%202/Readme.html
;http://nsis.sourceforge.net/Docs/Modern%20UI/Readme.html
;http://nsis.sourceforge.net/Docs/Chapter4.html
;http://nsis.sourceforge.net/Many_Icons_Many_shortcuts

;Revision Log
; 2012-03-26 Adapted for power network extension and MSVC 2010

;=================== BEGIN SCRIPT ====================
; Include for nice Setup UI
!include MUI2.nsh

;------------------------------------------------------------------------
; Modern UI2 definition                                                  -
;------------------------------------------------------------------------
; Description
Name "QLandkarte GT Power Network Extension"

;Default installation folder
InstallDir "$PROGRAMFILES\QLandkarteGT-PN"

;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\QLandkarteGT-PN" ""

;Request application privileges for Windows Vista
RequestExecutionLevel admin


; The file to write
OutFile "QLandkarteGT-PN.exe"

;------------------------------------------------------------------------
; Modern UI definition                                                    -
;------------------------------------------------------------------------
;!define MUI_COMPONENTSPAGE_SMALLDESC ;No value
!define MUI_INSTFILESPAGE_COLORS "FFFFFF 000000" ;Two colors

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "logo_small.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "logo_big.bmp"

; Page welcome description
!define MUI_WELCOMEPAGE_TITLE "QLandkarte GT Power Network Extension"
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEPAGE_TEXT "This is a GeoTiff viewer for the PC. Next to displaying maps it is able to visualize data acquired by a GPSr on the map. With QLandkarte GT you are able to produce smaller map subsets to be used by mobile devices. The power network extension allows you to create electric distribution network for stand-alone power generation sites, e.g. a micro-hydro installation."

!define MUI_LICENSEPAGE_CHECKBOX

;------------------------------------------------------------------------
; Pages definition order                                                -
;------------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "License.rtf"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
Var StartMenuFolder
!insertmacro MUI_PAGE_STARTMENU "Application" $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
;------------------------------------------------------------------------

;------------------------------------------------------------------------
;Uninstaller                                                            -
;------------------------------------------------------------------------
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Language settings
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"


;------------------------------------------------------------------------
; Component add                                                            -
;------------------------------------------------------------------------
;Components description

Section "MSVC++ 2010 SP1 Runtime" MSVC

  SetOutPath $INSTDIR
  File Files\vcredist_x86.exe
  ExecWait '"$INSTDIR\vcredist_x86.exe"'
  Delete "$INSTDIR\vcredist_x86.exe"
  
SectionEnd
LangString DESC_MSVC ${LANG_ENGLISH} "Microsoft Visual C++ 2010 SP1 Runtime Libraries. Typically already installed on your PC. You only need to install them if it doesn't work without ;-)."
LangString DESC_MSVC ${LANG_GERMAN} "Microsoft Visual C++ 2010 SP1 Laufzeitbibliotheken. Diese sind meist bereits auf dem Rechner installiert. Versuchen Sie die Installation zun�chst einmal ohne dies."

Section "QLandkarte GT PN" QLandkarteGT-PN

  ;Install for all users
  SetShellVarContext all

  ;BEGIN QLandkarte GT PN Files    
  SetOutPath $INSTDIR
    File Files\qlandkartegt-pn.exe
    File Files\map2gcm.exe
    File Files\map2rmap.exe
    File Files\cache2gtiff.exe
    File Files\map2rmp.exe
    File Files\map2jnx.exe
    File Files\*.ico
    File Files\qlandkartegt_*.qm
    File Files\qt_??.qm
  ;END QLandkarte GT PN Files    
   
  ;BEGIN Qt Files
  SetOutPath $INSTDIR
    File Files\QtCore4.dll
    File Files\QtGui4.dll
    File Files\QtNetwork4.dll
    File Files\QtSvg4.dll
    File Files\QtXml4.dll
    File Files\QtOpenGL4.dll
    File Files\QtSql4.dll
    File Files\QtWebKit4.dll
    File Files\phonon4.dll
    File Files\QtScript4.dll

  SetOutPath "$INSTDIR\imageformats\"
    File Files\imageformats\qgif4.dll
    File Files\imageformats\qjpeg4.dll
    File Files\imageformats\qmng4.dll
    File Files\imageformats\qsvg4.dll

  SetOutPath "$INSTDIR\sqldrivers\"
    File Files\sqldrivers\qsqlite4.dll
  ;END Qt Files
    
  ;BEGIN GDAL Files    
  SetOutPath $INSTDIR
    File /r Files\gdal_bin\*.*
  ;END GDAL Files        
    
  ;BEGIN additional Files    
  SetOutPath $INSTDIR
    File Files\3rdparty.txt
    File /nonfatal Files\libexif-12.dll
  ;END additional Files    
    
  ;the last "SetOutPath" will be the default directory
  SetOutPath $INSTDIR    
  

  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;create batch file for a GDAL shell
  fileOpen $0 "$INSTDIR\gdal.bat" w
  fileWrite $0 "cd /D $\"$INSTDIR\gdal\apps$\"$\r$\n" 
  fileWrite $0 "SET PATH=$INSTDIR;$INSTDIR\gdal\python\osgeo;$INSTDIR\proj\apps;$INSTDIR\gdal\apps;$INSTDIR\curl;%PATH%$\r$\n"
  fileWrite $0 "SET GDAL_DATA=$INSTDIR\gdal-data$\r$\n"
  fileWrite $0 "SET GDAL_DRIVER_PATH=$INSTDIR\gdal\plugins$\r$\n"
  fileWrite $0 "SET PYTHONPATH=$INSTDIR\gdal\python;%PYTHONPATH%$\r$\n"
  fileWrite $0 "SET PROJ_LIB=$INSTDIR\proj\SHARE$\r$\n"
  fileClose $0

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
     ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\QLandkarteGT-PN.lnk" "$INSTDIR\qlandkartegt-pn.exe" "" "$INSTDIR\GlobeWin.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\qlandkarte.org.lnk" "http://www.qlandkarte.org/" "" "$INSTDIR\kfm_home.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Help.lnk" "https://sourceforge.net/apps/mediawiki/qlandkartegt/index.php?title=Help_for_QLandkarte_GT" "" "$INSTDIR\khelpcenter.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Download.lnk" "http://sourceforge.net/projects/qlandkartegt/" "" "$INSTDIR\kget.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\GDAL.lnk" %COMSPEC% "/k $\"$INSTDIR\gdal.bat$\""
   !insertmacro MUI_STARTMENU_WRITE_END

  ;Create registry entries
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QLandkarte GT PN" "DisplayName" "QLandkarte GT PN(remove only)"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QLandkarte GT PN" "UninstallString" "$INSTDIR\Uninstall.exe"

SectionEnd
LangString DESC_QLandkarteGT-PN ${LANG_ENGLISH} "View GeoTiff and Garmin Maps. Visualize and analyze GPX files and much more!"
LangString DESC_QLandkarteGT-PN ${LANG_GERMAN}  "Landkarten im GeoTiff und Garmin Format betrachten. GPX Dateien visualisieren und analysieren und vieles mehr!"


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
   !insertmacro MUI_DESCRIPTION_TEXT ${QLandkarteGT-PN} $(DESC_QLandkarteGT-PN)
   !insertmacro MUI_DESCRIPTION_TEXT ${MSVC} $(DESC_MSVC)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;------------------------------------------------------------------------
;Uninstaller Sections                                                    -
;------------------------------------------------------------------------
Section "Uninstall"

  ;Install for all users
  SetShellVarContext all

  Delete "$INSTDIR\Uninstall.exe"

  SetOutPath $TEMP

  RMDir /r $INSTDIR

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\QLandkarteGT-PN.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\qlandkarte.org.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Help.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Download.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\GDAL.lnk"
  
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey /ifempty HKCU "Software\QLandkarteGT-PN"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QLandkarte GT PN"

SectionEnd

Function .onInit
  # set section 'MSVC' as unselected
  SectionSetFlags ${MSVC} 0
FunctionEnd

