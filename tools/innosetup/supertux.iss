; Supertux setup script. Process this script with InnoSetup
;
; Assumes the following was already done:
;
; rm -fr /build/supertux/build/innosetup
; ./autogen.sh
; LIBS="-L/usr/local/lib -lvorbisenc -lvorbisfile -lvorbis -logg" CFLAGS="-I/usr/local/include" configure
; jam
; jam -sDESTDIR="/build/supertux/build/innosetup" install
;

#define MyAppName "SuperTux"
#define MyAppVer "0.3.2-SVN"
#define MyAppVerName "SuperTux 0.3.2-SVN"
#define MyAppPublisher "SuperTux Development Team"
#define MyAppURL "http://supertux.lethargik.org"
#define MyAppExeName "supertux2.exe"
#define BuildDir "C:\msys\1.0\build\supertux\build\innosetup"
#define DllSourceDir "C:\msys\1.0\build\supertux"

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
Compression=lzma
SolidCompression=true
OutputBaseFilename=supertux-{#MyAppVer}-win32-setup
AllowNoIcons=true
AppID={{4BEF4147-E17A-4848-BDC4-60A0AAC70F2A}
VersionInfoVersion=0.3
VersionInfoTextVersion={#MyAppVerName}
AppVersion={#MyAppVer}
UninstallDisplayName={#MyAppVerName}
LicenseFile={#BuildDir}\usr\local\share\doc\supertux-{#MyAppVer}\LICENSE
InfoAfterFile={#BuildDir}\usr\local\share\doc\supertux-{#MyAppVer}\README.md
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
Source: {#BuildDir}\usr\local\bin\supertux2.exe; DestDir: {app}; Flags: ignoreversion
Source: {#BuildDir}\usr\local\share\supertux\images\engine\icons\supertux.ico; DestDir: {app}; Flags: ignoreversion
Source: {#BuildDir}\usr\local\share\doc\supertux-{#MyAppVer}\COPYING; DestDir: {app}; Flags: ignoreversion
Source: {#BuildDir}\usr\local\share\doc\supertux-{#MyAppVer}\README; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\SDL.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\SDL_image.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\iconv.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\libogg-0.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\libphysfs-1-0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\OpenAl32.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\wrap_oal.dll; DestDir: {app}; Flags: ignoreversion
Source: {#DllSourceDir}\libcurl-4.dll; DestDir: {app}; Flags: ignoreversion
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
