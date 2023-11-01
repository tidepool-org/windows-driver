[Setup]
AppName=Tidepool USB Driver Installation
AppVersion=2.0.0
OutputBaseFilename="Tidepool USB Driver Setup"
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
LicenseFile=tidepool_licence.rtf
SignTool=mycustom sign /n $qTidepool Project$q /t http://timestamp.digicert.com $f
SetupIconFile=Tidepool_T_Icon.ico
PrivilegesRequired=admin
; require at least Windows 10
MinVersion=10.0
; it gets installed to a temporary directory that gets deleted afterwards
DisableDirPage=yes


[Files]
; Place all x64 files here
Source: "TidepoolUSBDriver_x64.exe"; DestDir: "{tmp}\drivers"; Check: Is64BitInstallMode;
Source: "amd64\*"; DestDir: "{tmp}\drivers\amd64"; Check: Is64BitInstallMode;
Source: "x64\*"; DestDir: "{tmp}\drivers\x64"; Check: Is64BitInstallMode;

; Place all x86 files here, first one should be marked 'solidbreak'
; Add  'Parameters: "/q";' for quiet mode
Source: "TidepoolUSBDriver_x86.exe"; DestDir: "{tmp}\drivers"; Check: not Is64BitInstallMode; Flags: solidbreak
; some of the i386 files are needed for 64-bit too
Source: "i386\*"; DestDir: "{tmp}\drivers\i386";
Source: "x86\*"; DestDir: "{tmp}\drivers\x86"; Check: not Is64BitInstallMode;

; Place all common files here, first one should be marked 'solidbreak'
Source: "tidepoolvcp.cat"; DestDir: "{tmp}\drivers"; Flags:solidbreak
Source: "tidepoolvcp.inf"; DestDir: "{tmp}\drivers";
Source: "tidepoolhid.cat"; DestDir: "{tmp}\drivers";
Source: "tidepoolhid.inf"; DestDir: "{tmp}\drivers";
Source: "ftdibus.cat"; DestDir: "{tmp}\drivers";
Source: "ftdibus.inf"; DestDir: "{tmp}\drivers";
Source: "ftdiport.cat"; DestDir: "{tmp}\drivers";
Source: "ftdiport.inf"; DestDir: "{tmp}\drivers";
Source: "phdc_driver.cat"; DestDir: "{tmp}\drivers";
Source: "phdc.inf"; DestDir: "{tmp}\drivers";
Source: "winusbinstallation.cat"; DestDir: "{tmp}\drivers";
Source: "webusb.inf"; DestDir: "{tmp}\drivers";
Source: "dpinst.xml"; DestDir: "{tmp}\drivers";
Source: "tidepool.cer"; DestDir: "{tmp}";


[Messages]
WizardLicense=Agreement


[Run]
Filename: "certutil.exe"; Parameters: "-addstore ""TrustedPublisher"" {tmp}\tidepool.cer"; Flags: waituntilterminated runhidden; \
    StatusMsg: "Adding trusted publisher..."
Filename: "{tmp}\drivers\TidepoolUSBDriver_x86.exe"; Parameters: "/q"; Description: "32-bit Tidepool USB Driver Setup"; Flags: skipifnotsilent; Check: not Is64BitInstallMode
Filename: "{tmp}\drivers\TidepoolUSBDriver_x86.exe"; Description: "32-bit Tidepool USB Driver Setup"; Flags: skipifsilent; Check: not Is64BitInstallMode
Filename: "{tmp}\drivers\TidepoolUSBDriver_x64.exe"; Parameters: "/q"; Description: "64-bit Tidepool USB Driver Setup"; Flags: skipifnotsilent; Check: Is64BitInstallMode
Filename: "{tmp}\drivers\TidepoolUSBDriver_x64.exe"; Description: "64-bit Tidepool USB Driver Setup"; Flags: skipifsilent; Check: Is64BitInstallMode


