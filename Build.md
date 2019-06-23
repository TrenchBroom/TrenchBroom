# Building TrenchBroom

## All Platforms

- We use [pandoc](http://www.pandoc.org) to generate the documentation. Install a binary distribution from the website and make sure that it is in your `PATH`, otherwise your builds will fail.

## Windows

- Generally, the cmake scripts don't handle paths with spaces very well, so make sure that you check out the TrenchBroom source repository somewhere on a path without any spaces.
- For Visual Studio:
    - VS2017 is required. The community edition works fine.
    - In the Visual Studio Installer, you'll need to install:
      - Workloads 
        - **Desktop development with C++**
      - Individual components
        - **VC++ 2017 version 15.9 v14.16 latest v141 tools**
        - **Windows Universal CRT SDK**
        - **Windows XP support for C++**
  - Download and install [Qt](https://www.qt.io/download) for MSVC 2017 32-bit
  - Download and install [CMake](http://www.cmake.org) for Windows
  - Open a command prompt and change into the directory where you unpacked the TrenchBroom sources.
  - Create a new directory, e.g. "build", and change into it.
  - To generate a VS solution (if you want to work on TrenchBroom), run the following command:

    ```
    cmake .. -T v141 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:\Qt\5.13.0\msvc2017
    ```

    Finally, open `build\TrenchBroom.sln`.

    The `-T` option selects the "platform toolset" for the Visual Studio generator, which determines which C++ compiler and runtime the project will use. `v141` is the _Visual Studio 2017_ runtime. 
    TrenchBroom releases and CI builds use `v141`; earlier versions won't be able to compile TrenchBroom.

  - For a release build, instead run:

    ```
    cmake .. -T v141 -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\5.13.0\msvc2017
    cmake --build . --config Release --target TrenchBroom
    ```

  - **Note:** due to current limitations of the TrenchBroom build system, you must specify CMAKE_BUILD_TYPE when invoking cmake, and can't change between Release and Debug from Visual Studio

## Linux
### Dependencies
Compiling wxWidgets 3 requires the following dependencies. You should install them using the packager of your Linux distribution.
- g++ GNU c++ compiler
- Qt
- FreeImage: libfreeimage-dev
- OpenGL and GLU development headers (Mesa OpenGL development packages)
  freeglut3, freeglut3-dev, mesa-common-dev
- X11 video mode extension library: libxxf86vm-dev
- If you have a debian-based distribution, install them with this command:

  ```
  sudo apt-get install libgtk2.0-dev freeglut3 freeglut3-dev libglew-dev mesa-common-dev build-essential libglm-dev libxxf86vm-dev libfreeimage-dev pandoc
  ```

Compiling and linking TrenchBroom requires a working OpenGL installation. [This page](http://www.wikihow.com/Install-Mesa-%28OpenGL%29-on-Linux-Mint) may help you if you see linker errors about missing GL libraries.

### Build TrenchBroom
- Open a terminal and change into the directory where you unpacked the TrenchBroom sources
- Create a new directory, e.g. "build", and change into it.
- Run the following two commands

  ```
  cmake .. -DCMAKE_BUILD_TYPE=Release
  cmake --build . --target TrenchBroom
  ```

- You can replace "Release" with "Debug" if you want to create a debug build.

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

## Mac OS X
### Build environment
1. Get Xcode from the App Store

2. Dependencies
    - Install cmake (required) and ninja (optional). For example, with homebrew:

      ```
      brew install cmake ninja
      ```

3. Install Qt

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
- To create a release, push the appropriate tag, e.g. `git tag -a v2019.1 -m "This tag marks TrenchBroom 2019.1."`, then `git push origin v2019.1`.
