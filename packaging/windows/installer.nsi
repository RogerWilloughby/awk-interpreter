; AWK Interpreter NSIS Installer Script
; This is a custom NSIS script for more control over the installer

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "EnvVarUpdate.nsh"

; General settings
Name "AWK Interpreter"
OutFile "awk-${VERSION}-setup.exe"
InstallDir "$PROGRAMFILES64\AWK"
InstallDirRegKey HKLM "Software\AWK" "InstallDir"
RequestExecutionLevel admin

; Version info
VIProductVersion "${VERSION}.0"
VIAddVersionKey "ProductName" "AWK Interpreter"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "FileDescription" "AWK Interpreter Setup"
VIAddVersionKey "LegalCopyright" "MIT License"

; Interface settings
!define MUI_ABORTWARNING
!define MUI_ICON "awk.ico"
!define MUI_UNICON "awk.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "welcome.bmp"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"

; Installer sections
Section "AWK Interpreter (required)" SecMain
    SectionIn RO

    SetOutPath "$INSTDIR\bin"
    File "..\..\build\bin\awk.exe"

    SetOutPath "$INSTDIR\lib"
    File "..\..\build\lib\awk.lib"

    ; Write registry keys
    WriteRegStr HKLM "Software\AWK" "InstallDir" "$INSTDIR"
    WriteRegStr HKLM "Software\AWK" "Version" "${VERSION}"

    ; Uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Add/Remove Programs entry
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AWK" \
        "DisplayName" "AWK Interpreter"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AWK" \
        "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AWK" \
        "DisplayIcon" "$INSTDIR\bin\awk.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AWK" \
        "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AWK" \
        "Publisher" "AWK Interpreter Project"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AWK" \
        "URLInfoAbout" "https://github.com/yourusername/awk"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AWK" \
        "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AWK" \
        "NoRepair" 1

    ; Get installed size
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AWK" \
        "EstimatedSize" "$0"
SectionEnd

Section "Development Files" SecDev
    SetOutPath "$INSTDIR\include"
    File /r "..\..\include\*"

    SetOutPath "$INSTDIR\lib\cmake\awk"
    File "..\..\build\awkConfig.cmake"
    File "..\..\build\awkConfigVersion.cmake"
    File "..\..\build\awkTargets.cmake"
SectionEnd

Section "Documentation" SecDoc
    SetOutPath "$INSTDIR\doc"
    File "..\..\README.md"
    File "..\..\LICENSE"
    File "..\..\CHANGELOG.md"

    SetOutPath "$INSTDIR\doc\docs"
    File "..\..\docs\*.md"
SectionEnd

Section "Add to PATH" SecPath
    ; Add to system PATH
    ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR\bin"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "The AWK interpreter executable and library."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDev} "Header files and CMake config for development."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDoc} "User manual and documentation."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPath} "Add AWK to system PATH for command-line access."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Uninstaller
Section "Uninstall"
    ; Remove from PATH
    ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\bin"

    ; Remove files
    RMDir /r "$INSTDIR\bin"
    RMDir /r "$INSTDIR\lib"
    RMDir /r "$INSTDIR\include"
    RMDir /r "$INSTDIR\doc"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir "$INSTDIR"

    ; Remove registry keys
    DeleteRegKey HKLM "Software\AWK"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AWK"
SectionEnd
