# Building TrenchBroom

Follow these instructions if you want to get started developing on TrenchBroom.

To see how releases of TB are packaged, consult our CI scripts instead.

## All Platforms

First, clone the TrenchBroom repository. If you are using the official repository and not a fork, you can clone the
repository by running

```
git clone https://github.com/kduske/TrenchBroom.git
```

TrenchBroom uses a few git submodules for some of its dependencies. To initialize the submodules, issue the following command:

```
cd TrenchBroom
git submodule update --init --recursive
```

## Windows

- Visual Studio 2017 or 2019 can be used for development. Code needs to compile with VS 2017 (that's what our releases are built with). 
- For Visual Studio 2017:
    - In the Visual Studio Installer, you'll need to install:
      - Workloads 
        - **Desktop development with C++**
      - Individual components
        - **VC++ 2017 version 15.9 v14.16 latest v141 tools**
        - **Windows Universal CRT SDK**
        - **Windows XP support for C++**
  - For Visual Studio 2019:
    - In the Visual Studio Installer, you'll need to install:
        - Workloads
          - **Desktop development with C++**
  - Download and install [Qt](https://www.qt.io/download) for MSVC 2017 32-bit
  - Download and install [CMake](http://www.cmake.org) for Windows
  - Download and install [pandoc](http://www.pandoc.org)
  - Create a subdirectory of the TrenchBroom directory, e.g. "build", and open a command prompt there
  - For Visual Studio 2017, run:

    ```
    cmake .. -T v141 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:\Qt\5.13.0\msvc2017
    ```

  - For Visual Studio 2019, run:

    ```
    cmake .. -G "Visual Studio 16 2019" -A Win32 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:\Qt\5.13.0\msvc2017
    ```

    Finally, open `build\TrenchBroom.sln`.

  - **Note:** specifying CMAKE_BUILD_TYPE is required when running cmake; you can't change between Release and Debug from Visual Studio

## Linux
### Dependencies
TrenchBroom depends on:
- g++ >= 7
- Qt >= 5.9
- FreeImage: libfreeimage-dev
- OpenGL and GLU development headers (Mesa OpenGL development packages)
  freeglut3, freeglut3-dev, mesa-common-dev
- X11 video mode extension library: libxxf86vm-dev
- If you have a debian-based distribution, install them with this command:

  ```
  sudo apt-get install g++-7 qt5-default freeglut3-dev libglew-dev mesa-common-dev build-essential libglm-dev libxxf86vm-dev libfreeimage-dev libfreetype6-dev pandoc cmake p7zip-full ninja-build
  ```

### Build TrenchBroom
- Open a terminal and change into the directory where you unpacked the TrenchBroom sources
- Create a new directory, e.g. "build", and change into it.
- Run the following two commands

  ```
  cmake .. -DCMAKE_BUILD_TYPE=Debug
  cmake --build . --target TrenchBroom
  ```

- You can replace "Debug" with "Release" if you want to create a release build.

## Mac OS X
### Build environment
1. Get Xcode from the App Store

2. Dependencies

      ```
      brew install cmake qt pandoc
      ```

3. Build

      ```
      mkdir build-xcode
      cd build-xcode
      cmake .. -GXcode -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
      open TrenchBroom.xcodeproj
      ```

      Don't enable *Address Sanitizer* in Xcode; it breaks rebuilding of the project (see [#1373](https://github.com/kduske/TrenchBroom/issues/1373)).

### Notes
- The changelog is generated with `git log --oneline --decorate <LAST_REL_TAG>..HEAD`, where <LAST_REL_TAG> is replaced by whatever tag marks the last release. The generated log is then manually cleaned up.
- To create a release, push the appropriate tag, e.g. `git tag -a v2019.1 -m "This tag marks TrenchBroom 2019.1."`, then `git push origin v2019.1`.
