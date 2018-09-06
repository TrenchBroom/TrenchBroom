# Building TrenchBroom

## All Platforms

- We use [pandoc](http://www.pandoc.org) to generate the documentation. Install a binary distribution from the website and make sure that it is in your `PATH`, otherwise your builds will fail.

## Windows

- Generally, the cmake scripts don't handle paths with spaces very well, so make sure that you check out the TrenchBroom source repository somewhere on a path without any spaces.
- For Visual Studio:
    - VS2017 is required. The community edition works fine.
    - For VS2017, in the Visual Studio Installer, in the "Individual Components" tab, under the "Compilers, Build Tools, and Runtimes" heading, select the following components:
      - VC++ 2017 v141 toolset for desktop (x86,x64)
      - Windows XP support for C++

    - Download the following wxWidgets binary packages:
        - [wxWidgets-3.1.1-headers.7z](https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.1/wxWidgets-3.1.1-headers.7z)
        - [wxMSW-3.1.1_vc141_Dev.7z](https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.1/wxMSW-3.1.1_vc141_Dev.7z)
        - [wxMSW-3.1.1_vc141_ReleaseDLL.7z](https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.1/wxMSW-3.1.1_vc141_ReleaseDLL.7z)
        - [wxMSW-3.1.1_vc141_ReleasePDB.7z](https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.1/wxMSW-3.1.1_vc141_ReleasePDB.7z)

    - Unpack all files into `c:\wxWidgets-<version>` so that `include` and `lib` directories are at the same level after unpacking.
    - The directory layout should look like this:

      ```
      wxWidgets-3.1.1
        \include
            \msvc
            \wx
            \wx
            <wxwidgets header files>
        \lib
            \vc141_dll
            <wxwidgets libraries>
      ```

    - Set a new environment variable `WXWIN=C:\wxWidgets-<version>` (replace the path with the path where you unpacked wxWidgets).
    - If you want to run the binaries without using the installer, add `%WXWIN%\lib\vc141_dll` to your path. The relevant parts of my `PATH` variable look something like this:

      ```
      C:\Program Files (x86)\CMake 2.8\bin;c:\wxWidgets-3.1.1\lib\vc141_dll;
      ```

  - Download and install [CMake](http://www.cmake.org) for Windows
  - Open a command prompt and change into the directory where you unpacked the TrenchBroom sources.
  - Create a new directory, e.g. "build", and change into it.
  - Run the following two commands

    ```
    cmake .. -T v141_xp -DCMAKE_BUILD_TYPE=Release
    cmake --build . --config Release --target TrenchBroom
    ```

    The `-T` option selects the "platform toolset" for the Visual Studio generator, which determines which C++ compiler and runtime the project will use. `v141_xp` is the _Visual Studio 2017_ runtime, with compatibility down to Windows XP. TrenchBroom releases and CI builds use `v141_xp`; earlier versions won't be able to compile TrenchBroom.

    You can replace "Release" with "Debug" if you want to create a debug build. This is also recommended if you want to work on the source in Visual Studio.

## Linux
### Dependencies
Compiling wxWidgets 3 requires the following dependencies. You should install them using the packager of your Linux distribution.
- g++ GNU c++ compiler
- GTK2 and development packages: libgtk2.0-dev (GTK3 will NOT work)
- FreeImage: libfreeimage-dev
- OpenGL and GLU development headers (Mesa OpenGL development packages)
  freeglut3, freeglut3-dev, mesa-common-dev
- X11 video mode extension library: libxxf86vm-dev
- If you have a debian-based distribution, install them with this command:

  ```
  sudo apt-get install libgtk2.0-dev freeglut3 freeglut3-dev libglew-dev mesa-common-dev build-essential libglm-dev libxxf86vm-dev libfreeimage-dev pandoc
  ```

Compiling and linking TrenchBroom requires a working OpenGL installation. [This page](http://www.wikihow.com/Install-Mesa-%28OpenGL%29-on-Linux-Mint) may help you if you see linker errors about missing GL libraries.

- Some more detailed (possibly outdated) information about building TrenchBroom on Linux: http://andyp123.blogspot.de/2013/03/running-trenchbroom-quake-editor-on.html

### wxWidgets
- Currently you must download, patch, and build wxWidgets yourself.
  - Get the latest sources of wxWidgets 3.1 from [wxwidgets.org](http://www.wxwidgets.org) and unpack them.
  - Move the unpacked directory someplace where you want to keep it.
  - Open a terminal and change into the wxwidgets directory.
  - Apply the patches in `TrenchBroom/patches/wxWidgets` as follows:

    ```
    patch -p0 < <path_to_trenchbroom_directory>/patches/wxWidgets/*.patch
    ```

  - Create two directories: `build-release` and `build-debug` (don't rename those!)
  - Change into `wxwidgets/build-release`
  - Run 

    ```
    ../configure --disable-shared --with-opengl --with-gtk=2 --prefix=$(pwd)/install
    ```

  - Run

    ``` 
    make
    make install
    ```

  - Change into `wxwidgets/build-debug`
  - Run 

    ```
    ../configure --enable-debug --with-opengl --with-gtk=2 --prefix=$(pwd)/install
    ```

  - Run 

    ```
    make
    make install
    ```

### CMake
- Install CMake using your package manager: `sudo apt-get install cmake`

### Build TrenchBroom
- Open a terminal and change into the directory where you unpacked the TrenchBroom sources
- Create a new directory, e.g. "build", and change into it.
- Run the following two commands

  1. 
  
  ```
  cmake .. -DCMAKE_BUILD_TYPE=Release -DwxWidgets_PREFIX=/your/wxWidgets/directory/build-release/install
  ```
  
  If you use a system-wide installed wxwidgets, be aware that the CMAKE-script appends `./include/wx-3.1` to the given prefix, so for a wxWidgets installed in `/usr/include/wx-3.1/` use:
  
  ```
  cmake .. -DCMAKE_BUILD_TYPE=Release -DwxWidgets_PREFIX=/usr
  ```
  
  
  2.
  
  ```
  cmake --build . --target TrenchBroom
  ```

- You can replace "Release" with "Debug" if you want to create a debug build. Also change the value of the `wxWidgets_PREFIX` variable to point to your wxWidgets `build-debug` directory in that case.

- Unless you install TrenchBroom system-wide (see Packaging below), you'll need to set the `TB_DEV_MODE` environment variable to `1` when launching TrenchBroom:

  ```
  TB_DEV_MODE=1 ./trenchbroom
  ```

  This is necessary to tell TrenchBroom to look for resources in the current directory, instead of a system-wide location (in `/usr/`).

### Packaging
- If you want to create packages for Linux (deb or rpm), then you'll need to install these packages: devscripts, debhelper, rpm

  ```
  sudo apt-get install devscripts debhelper rpm
  ```

### Notes
- You can install your preferred wxWidgets configuration using make install. If you wish to do this, then you can omit specifying the `wxWidgets_PREFIX` variable when generating the build configs with Cmake.
- On some systems, such as Xubuntu, you may have to pass the following extra paramter to cmake when creating the build scripts: `-DFREETYPE_INCLUDE_PATH=/usr/include/freetype2/freetype`
  So the first cmake command should be

  ```
  cmake .. -DCMAKE_BUILD_TYPE=Release -DFREETYPE_INCLUDE_PATH=/usr/include/freetype2/freetype
  ```

## Mac OS X
### Build environment
1. Get Xcode from the App Store

2. Dependencies
    - Install cmake (required) and ninja (optional). For example, with homebrew:

      ```
      brew install cmake ninja
      ```

3. wxWidgets
    - Get the latest sources of wxWidgets 3.1 from [wxwidgets.org](http://www.wxwidgets.org) and unpack them.
    - Move the unpacked directory someplace where you want to keep it.
    - Open a terminal and change into the wxwidgets directory.
    - Apply the patches in `TrenchBroom/patches/wxWidgets` as follows:

      ```
      patch -p0 < <path_to_trenchbroom_directory>/patches/wxWidgets/*.patch
      ```

    - Create two directories: `build-release` and `build-debug` (don't rename those!)
    - Change into `wxwidgets/build-release`
    - Run

      ```
      ../configure --with-osx_cocoa --disable-shared --disable-mediactrl --with-opengl --with-macosx-version-min=10.9 --with-cxx=17 --prefix=$(pwd)/install
      ```

    - Run

      ```
      make
      make install
      ```

    - Change into `wxwidgets/build-debug`
    - Run 

      ```
      ../configure --enable-debug --with-osx_cocoa --disable-mediactrl --with-opengl --with-macosx-version-min=10.9 --with-cxx=17 --prefix=$(pwd)/install
      ```

    - Run

      ```
      make
      make install
      ```

4. Build
    - For a release build:

      ```
      mkdir build-ninja
      cd build-ninja
      cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DwxWidgets_PREFIX=/your/wxWidgets/directory/build-release/install
      ninja
      ```

    - To generate an Xcode project for developing with:

      ```
      mkdir build-xcode
      cd build-xcode
      cmake .. -GXcode -DCMAKE_BUILD_TYPE=Debug -DwxWidgets_PREFIX=/your/wxWidgets/directory/build-debug/install
      open TrenchBroom.xcodeproj
      ```

      Don't enable *Address Sanitizer* in Xcode; it breaks rebuilding of the project (see [#1373](https://github.com/kduske/TrenchBroom/issues/1373)).

### Notes
- You can install your preferred wxWidgets configuration using `make install`. If you wish to do this, then you can omit specifying the `wxWidgets_PREFIX` variable when generating the build configs with Cmake.
- The changelog is generated with `git log --oneline --decorate <LAST_REL_TAG>..HEAD`, where <LAST_REL_TAG> is replaced by whatever tag marks the last release. The generated log is then manually cleaned up.
- To create a release, push the appropriate tag, e.g. `git tag -a v2.0.0-RC5 -m "This tag marks TrenchBroom 2 release candidate 5."`, then `git push origin v2.0.0-RC5`.
