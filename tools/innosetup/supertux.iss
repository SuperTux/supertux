; Supertux setup script. Process this script with InnoSetup
;
; Assumes the following was already done:
;
; rm -fr /tmp/innobuild
; ./autogen.sh
; LIBS="-L/usr/local/lib -lvorbisenc -lvorbisfile -lvorbis -logg" CFLAGS="-I/usr/local/include" configure
; jam
; jam -sDESTDIR="/tmp/innobuild" install
;

#define MyAppName "SuperTux"
#define MyAppVer "0.3.0"
#define MyAppVerName "SuperTux 0.3.0"
#define MyAppPublisher "SuperTux Development Team"
#define MyAppURL "http://supertux.berlios.de"
#define MyAppExeName "supertux.exe"
#define BuildDir "C:\Documents and Settings\Arvid\Lokala inställningar\Temp\innobuild"
#define DllSourceDir "C:\msys\1.0\build\supertux-0.3"

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppVerName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppVerName}
DefaultGroupName={#MyAppVerName}
ShowLanguageDialog=yes
Compression=none
SolidCompression=true
OutputBaseFilename=supertux-{#MyAppVer}-win32-setup
AllowNoIcons=true
AppID={{4BEF4147-E17A-4848-BDC4-60A0AAC70F2A}
VersionInfoVersion={#MyAppVer}
AppVersion={#MyAppVer}
UninstallDisplayName={#MyAppVerName}
LicenseFile={#BuildDir}\usr\local\share\doc\supertux-0.3.0\COPYING
InfoAfterFile={#BuildDir}\usr\local\share\doc\supertux-0.3.0\README
SetupIconFile={#BuildDir}\usr\local\share\supertux\images\engine\icons\supertux.ico

[Languages]
Name: english; MessagesFile: compiler:Default.isl
Name: basque; MessagesFile: compiler:Languages\Basque.isl
Name: brazilianportuguese; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
Name: catalan; MessagesFile: compiler:Languages\Catalan.isl
Name: czech; MessagesFile: compiler:Languages\Czech.isl
Name: danish; MessagesFile: compiler:Languages\Danish.isl
Name: dutch; MessagesFile: compiler:Languages\Dutch.isl
Name: finnish; MessagesFile: compiler:Languages\Finnish.isl
Name: french; MessagesFile: compiler:Languages\French.isl
Name: german; MessagesFile: compiler:Languages\German.isl
Name: hungarian; MessagesFile: compiler:Languages\Hungarian.isl
Name: italian; MessagesFile: compiler:Languages\Italian.isl
Name: norwegian; MessagesFile: compiler:Languages\Norwegian.isl
Name: polish; MessagesFile: compiler:Languages\Polish.isl
Name: portuguese; MessagesFile: compiler:Languages\Portuguese.isl
Name: russian; MessagesFile: compiler:Languages\Russian.isl
Name: slovak; MessagesFile: compiler:Languages\Slovak.isl
Name: slovenian; MessagesFile: compiler:Languages\Slovenian.isl
Name: spanish; MessagesFile: compiler:Languages\Spanish.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: {#BuildDir}\usr\local\bin\supertux.exe; DestDir: {app}; Flags: ignoreversion
Source: {#BuildDir}\usr\local\share\supertux\images\engine\icons\supertux.ico; DestDir: {app}; Flags: ignoreversion
Source: {#BuildDir}\usr\local\share\doc\supertux-0.3.0\COPYING; DestDir: {app}; Flags: ignoreversion
Source: {#BuildDir}\usr\local\share\doc\supertux-0.3.0\README; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\SDL.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\SDL_image.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\iconv.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\libogg-0.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\libphysfs-1-0-0.dll; DestDir: {app}; Flags: ignoreversion
;Source: {#DllSourceDir}\OpenAL32.dll; DestDir: {app}; Flags: ignoreversion
Source: {#BuildDir}\usr\local\share\supertux\*; DestDir: {app}\data; Flags: ignoreversion recursesubdirs createallsubdirs


[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; IconFilename: {app}\{#MyAppName}.ico
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {userdesktop}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; IconFilename: {app}\{#MyAppName}.ico; Tasks: desktopicon

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: {app}\stdout.txt
Type: filesandordirs; Name: {app}\stderr.txt
