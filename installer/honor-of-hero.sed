[Version]
Class=IEXPRESS
SEDVersion=3

[Options]
PackagePurpose=InstallApp
ShowInstallProgramWindow=0
HideExtractAnimation=1
UseLongFileName=1
InsideCompressed=0
CAB_FixedSize=0
CAB_ResvCodeSigning=0
RebootMode=N
InstallPrompt=%InstallPrompt%
DisplayLicense=%DisplayLicense%
FinishMessage=%FinishMessage%
TargetName=%TargetName%
FriendlyName=%FriendlyName%
AppLaunched=%AppLaunched%
PostInstallCmd=<None>
AdminQuietInstCmd=%AppLaunched%
UserQuietInstCmd=%AppLaunched%
SourceFiles=SourceFiles

[Strings]
InstallPrompt=
DisplayLicense=
FinishMessage=Honor of Hero has been installed.
TargetName=D:\develop\Qtproject\honor-of-hero\dist\HonorOfHero-Installer.exe
FriendlyName=Honor of Hero Installer
AppLaunched=install.cmd
FILE0=install.cmd
FILE1=HonorOfHero.zip

[SourceFiles]
SourceFiles0=D:\develop\Qtproject\honor-of-hero\installer\

[SourceFiles0]
%FILE0%=
%FILE1%=
