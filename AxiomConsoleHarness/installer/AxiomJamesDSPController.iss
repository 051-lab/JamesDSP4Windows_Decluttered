#define MyAppName "Axiom JamesDSP Controller"
#ifndef MyAppVersion
  #define MyAppVersion "0.0.0-dev"
#endif
#define MyAppPublisher "Axiom DSP"
#define MyAppExeName "AxiomJamesDSPController.exe"
#define PackageDir "..\dist\AxiomJamesDSPController-win-x64"
#define AppIconFile "..\Assets\axiom-controller.ico"

[Setup]
AppId={{8D92EB73-FEE5-4E5B-B0AD-12439B16090A}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\Axiom JamesDSP Controller
DefaultGroupName=Axiom
DisableProgramGroupPage=yes
OutputDir=..\dist\installer
OutputBaseFilename=AxiomJamesDSPController-{#MyAppVersion}-win-x64-setup
Compression=lzma2/max
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
CloseApplications=yes
RestartApplications=no
SetupLogging=yes
UninstallDisplayIcon={app}\{#MyAppExeName}
VersionInfoVersion={#MyAppVersion}.0
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppName} Installer
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}
WizardStyle=modern
SetupIconFile={#AppIconFile}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked
Name: "autostart"; Description: "Start Axiom JamesDSP Controller when I sign in"; GroupDescription: "Startup:"; Flags: unchecked

[Files]
Source: "{#PackageDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Axiom JamesDSP Controller"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\Axiom JamesDSP Controller"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{commonstartup}\Axiom JamesDSP Controller"; Filename: "{app}\{#MyAppExeName}"; Tasks: autostart

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch Axiom JamesDSP Controller"; Flags: nowait postinstall skipifsilent

[Code]
function InitializeUninstall(): Boolean;
begin
  Result := True;
  MsgBox(
    'Axiom application files will be removed. Profiles, settings, diagnostics, and runtime data under Local AppData will be preserved.',
    mbInformation,
    MB_OK);
end;
