; -- 64BitTwoArch.iss --
; Demonstrates how to install a program built for two different
; architectures (x86 and x64) using a single installer.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=Tidepool USB Driver
AppVersion=1.4
DefaultDirName={tmp}
DefaultGroupName=Tidepool
;UninstallDisplayIcon={app}\MyProg.exe
Compression=lzma2
SolidCompression=yes
;OutputDir=userdocs:Inno Setup Examples Output
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
; On all other architectures it will install in "32-bit mode".
ArchitecturesInstallIn64BitMode=x64
; Note: We don't set ProcessorsAllowed because we want this
; installation to run on all architectures (including Itanium,
; since it's capable of running 32-bit code too).
LicenseFile=tidepool_licence.rtf

[Files]
; Place all x64 files here
Source: "TidepoolUSBDriver_x64.exe"; DestDir: "{tmp}\drivers"; Check: Is64BitInstallMode;
Source: "amd64\*"; DestDir: "{tmp}\drivers\amd64"; Check: Is64BitInstallMode;
Source: "win7x64\*"; DestDir: "{tmp}\drivers\amd64"; Check: Is64BitInstallMode; OnlyBelowVersion: 6.2;
Source: "x64\*"; DestDir: "{tmp}\drivers\x64"; Check: Is64BitInstallMode;

; Place all x86 files here, first one should be marked 'solidbreak'
Source: "TidepoolUSBDriver_x86.exe"; DestDir: "{tmp}\drivers"; Check: not Is64BitInstallMode; Flags: solidbreak
Source: "i386\*"; DestDir: "{tmp}\drivers\i386"; Check: not Is64BitInstallMode;
Source: "win7x86\*"; DestDir: "{tmp}\drivers\i386"; Check: Is64BitInstallMode; OnlyBelowVersion: 6.2;
Source: "x86\*"; DestDir: "{tmp}\drivers\x86"; Check: not Is64BitInstallMode;

; Place all common files here, first one should be marked 'solidbreak'
Source: "tidepoolvcp.cat"; DestDir: "{tmp}\drivers"; Flags:solidbreak
Source: "tidepoolvcp.inf"; DestDir: "{tmp}\drivers";
Source: "tidepoolhid.cat"; DestDir: "{tmp}\drivers";
Source: "tidepoolhid.inf"; DestDir: "{tmp}\drivers";
Source: "dpinst.xml"; DestDir: "{tmp}\drivers";

[Run]
Filename: {tmp}\drivers\TidepoolUSBDriver_x86.exe; Description: "32-bit Tidepool USB Driver Setup"; Check: not Is64BitInstallMode
Filename: {tmp}\drivers\TidepoolUSBDriver_x64.exe; Description: "64-bit Tidepool USB Driver Setup"; Check: Is64BitInstallMode


