# Signing the driver

## Requirements

# Inno Setup
http://www.jrsoftware.org/isdl.php

# WDK 
Required for `inf2cat` and `signtool`
https://msdn.microsoft.com/en-us/windows/hardware/gg454513.aspx

## Steps

### Generate .cat for Windows XP, Vista, 7, 8 and 8.1:
inf2cat /driver:. /os:7_X64,7_X86,8_X64,8_X86,6_3_X86,6_3_X64,Vista_X86,Vista_X64,XP_X86,XP_X64

### Install certificate:
Double-click to install. You can verify it is installed by running `certmgr`.

### Download the DigiCert cross-certificate:
https://www.digicert.com/code-signing/driver-signing-in-windows-using-signtool.htm#download_cross_certificate

### Sign using signtool:

signtool sign /v /ac "DigiCert High Assurance EV Root CA.crt" /s my /n "Tidepool Project" /t http://timestamp.digicert.com tidepoolvcp.cat

### Verify that drivers are correctly signed:

	signtool verify /kp /v /c tidepoolvcp.cat amd64\ftser2k.sys
	signtool verify /kp /v /c tidepoolvcp.cat i386\ftser2k.sys
	signtool verify /kp /v /c tidepoolvcp.cat x64\silabenm.sys
	signtool verify /kp /v /c tidepoolvcp.cat x86\silabenm.sys
	signtool verify /kp /v /c tidepoolvcp.cat amd64\tiusb.sys
	signtool verify /kp /v /c tidepoolvcp.cat i386\tiusb.sys

## Run InnoSetup:
- Double-click innosetup
- Build -> Compile

## Sign setup.exe:
- cd Output
- rename setup.exe TidepoolUSBDriverSetup.exe
- signtool sign /v /s my /n "Tidepool Project" /t http://timestamp.digicert.com TidepoolUSBDriverSetup.exe

## Notes

- If the drivers fails to install, make sure all devices are unplugged.
- Must have administrator privileges to install.
