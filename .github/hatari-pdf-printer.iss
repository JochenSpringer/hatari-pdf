; Inno Setup (https://jrsoftware.org/isinfo.php) - Installer fuer Hatari mit PDF-Drucker.
; Vorher build-windows.sh ausfuehren, dann dieses Skript in Inno Setup oeffnen
; und "Compile" waehlen. Ergebnis: Output\HatariPDF-Setup.exe
; Pfad des dist-Ordners relativ zu dieser Datei:
#define DistDir "dist"

[Setup]
AppName=Hatari mit PDF-Drucker
AppVersion=2.6.1-pdf
AppPublisher=Hatari-Team / PDF-Drucker-Erweiterung
DefaultDirName={autopf}\HatariPDF
DefaultGroupName=Hatari mit PDF-Drucker
LicenseFile={#DistDir}\gpl.txt
OutputBaseFilename=HatariPDF-Setup
OutputDir=Output
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequiredOverridesAllowed=dialog

[Languages]
Name: "german"; MessagesFile: "compiler:Languages\German.isl"

[Files]
Source: "{#DistDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion
; GPL: den passenden Quelltext (Overlay + Hatari) mitliefern, falls Du den Installer weitergibst:
; Source: "hatari-pdf-printer-src.zip"; DestDir: "{app}\source"; Flags: ignoreversion

[Icons]
Name: "{group}\Hatari (PDF-Drucker)"; Filename: "{app}\hatari.exe"
Name: "{group}\Handbuch"; Filename: "{app}\doc\manual.html"
Name: "{group}\Deinstallieren"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Hatari (PDF-Drucker)"; Filename: "{app}\hatari.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Symbol auf dem Desktop anlegen"; Flags: unchecked

[Run]
Filename: "{app}\hatari.exe"; Description: "Hatari jetzt starten"; Flags: nowait postinstall skipifsilent
