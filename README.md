# Master Sword Rebirth
The continuation of Master Sword Continued, a total HL1 conversion mod.

## Building
* Lastest version of [CMake](https://cmake.org/download/) 3.24 or greater.

### Windows
* Visual Studios 2015 or higher, prefered [Visual Studios 2022](https://visualstudio.microsoft.com/vs/community/).
* Latest version of [CMake](https://cmake.org/download/) 3.24 or greater.
* You can either generate project files by running ``createall.bat`` or manually run CMake by ``cmake -S . -B ./build -A Win32``

Currently the project relies on C++20.

## Compiling For Linux
* `sudo dpkg --add-architecture i386`
* `sudo add-apt-repository ppa:ubuntu-toolchain-r/test`
* `sudo apt update`
* `sudo apt install -y g++-11-multilib libgcc-s1:i386 libstdc++6:i386 libgl1-mesa-dev:i386 gcc-multilib g++-multilib`
* `./build-linux.sh`

## Troubleshooting
If the client immediately closes to desktop on launch with the latest compile chances are the scripts need to be repacked in which ``scripts.pak`` needs to go in the game's base directory AKA ``msrebirth\msr\scripts.pak``. As an alternative if you aren't doing script backend stuff you can rollback to https://github.com/MSRevive/MasterSwordRebirth/commit/ffa2dc04aebd4f470895dbaf91559157a397b655

## Contributing
Have a look at the open Github issues that aren't assigned [here](https://github.com/MSRevive/MasterSwordRebirth/issues), or feel free to join our [Discord](https://discord.gg/nwJB9EhAN6) to ask how to contribute further.
