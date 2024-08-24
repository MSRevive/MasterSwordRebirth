# Master Sword Rebirth
Run createall to create the solution(s).

## Compiling
You may compile with any version of Visual Studio > 2012, primarily you should use VS 2022, but you can even use [VS 2013](https://visualstudio.microsoft.com/vs/older-downloads/) if you don't run Windows 10 and up.

If get the error "Unable to find RegKey for .vcproj or .vcxproj files in solutions," folow these steps:

Add the following registry keys if they don't already exist:
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\10.0\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}

Add string key 'DefaultProjectExtension' and set the data to 'vcproj' and your sln file should generate without issue!

## Troubleshooting
Please see the [Valve Dev Wiki](https://developer.valvesoftware.com/wiki/Source_SDK_2013#Troubleshooting).

## Contributing
Feel free to help out fixing bugs, pull requests or whatever!

## Download
[MS Remake DLs](https://www.msremake.com/resources/master-sword-classic-full-installer.22/)

![MS:C](./msc.png)
