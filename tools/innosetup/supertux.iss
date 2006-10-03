; Supertux setup script. Process this script with InnoSetup
;
; This script assumes that you have prepared a directory that contains all
; files that "jam install" installs in the data\ subdir. You should also place
; files like README and COPYING
; into the doc\ dir.
; The main directory should contain this file the supertux.ico, all .dll
; files and the supertux.exe file.
[Setup]
AppName=SuperTux
AppVerName=SuperTux 0.3.0
AppPublisher=SuperTux Development Team
AppPublisherURL=http://supertux.berlios.de
AppSupportURL=http://supertux.berlios.de
AppUpdatesURL=http://supertux.berlios.de
DefaultDirName={pf}\SuperTux-0.3
DefaultGroupName=SuperTux
ShowLanguageDialog=yes
Compression=none
OutputBaseFilename=SuperTux-0.3.0-setup
LicenseFile=C:\msys\1.0\build\supertux-0.3\tools\innosetup\doc\COPYING.txt
UninstallDisplayIcon={app}\supertux.ico

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: supertux.exe; DestDir: {app}; Flags: ignoreversion
Source: supertux.ico; DestDir: {app}; Flags: ignoreversion
Source: *.dll; DestDir: {app}; Flags: ignoreversion
Source: doc\*.*; DestDir: {app}\doc\; Flags: ignoreversion
Source: data\*.*; DestDir: {app}\data\; Flags: ignoreversion recursesubdirs

[Icons]
Name: {group}\SuperTux; Filename: {app}\supertux.exe; WorkingDir: {app}; IconFilename: {app}\supertux.ico
Name: {group}\{cm:UninstallProgram,SuperTux}; Filename: {uninstallexe}
Name: {userdesktop}\SuperTux; Filename: {app}\supertux.exe; WorkingDir: {app}; IconFilename: {app}\supertux.ico; Tasks: desktopicon

[Run]
Filename: {app}\supertux.exe; Description: {cm:LaunchProgram,SuperTux}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: {app}\stdout.txt
Type: filesandordirs; Name: {app}\stderr.txt
