; ============================================
;  DUCK FACE Synth - Inno Setup installer
;  Build the project first with build.bat,
;  then compile this script with Inno Setup.
; ============================================
#define MyAppName "DUCK FACE Synth"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "DuckFace Audio"
#define MyAppExeName "DuckFaceSynth.exe"

[Setup]
AppId={{7D0C4F2A-D0CE-4F2B-9A11-DUCKFACE0001}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\DuckFaceSynth
DefaultGroupName={#MyAppName}
OutputBaseFilename=DuckFaceSynth-Setup-{#MyAppVersion}
OutputDir=Output
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
; MSVC output path; adjust to ..\build\DuckFaceSynth.exe if using MinGW
Source: "..\build\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall skipifsilent
