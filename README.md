# Master Sword Rebirth
The continuation of Master Sword Continued, a total HL1 conversion mod.

## Building
* Lastest version of [CMake](https://cmake.org/download/) 3.24 or greater.

### Windows
* Visual Studios 2015 or higher, prefered [Visual Studios 2022](https://visualstudio.microsoft.com/vs/community/).
* Latest version of [CMake](https://cmake.org/download/) 3.24 or greater.
* You can either generate project files by running ``createall.bat`` or manually run CMake by ``cmake -S . -B ./build -A Win32``

Currently the project relies on C++20.

### Compiling For Linux
* `sudo dpkg --add-architecture i386`
* `sudo add-apt-repository ppa:ubuntu-toolchain-r/test`
* `sudo apt update`
* `sudo apt install -y libgcc-s1:i386 libstdc++6:i386 libgl1-mesa-dev:i386 pkg-config gcc-multilib g++-multilib`
* `./build-linux.sh`

## Thanks
* https://github.com/twhl-community/halflife-updated - Reference and ParticleMan
* https://github.com/FreeSlave/halflife-featureful - ParticleMan
* https://github.com/tmp64/BugfixedHL-Rebased - VGUI2 and chat schema.
* https://github.com/tmp64/hl1_source_sdk - source_sdk
* https://github.com/twhl-community/halflife-unified-sdk - Organization and CMake reference.

* Solokiller for answering some questions and his contributions to MSC in the past.
* All of our contributors for assets and everything else related to this project.

## Contributing
Have a look at the open Github issues that aren't assigned [here](https://github.com/MSRevive/MasterSwordRebirth/issues), or feel free to join our [Discord](https://discord.gg/nwJB9EhAN6) to ask how to contribute further.
