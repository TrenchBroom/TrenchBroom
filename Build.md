# Building TrenchBroom

Follow these instructions if you want to get started developing on TrenchBroom.

To see how releases of TrenchBroom are packaged, consult our CI scripts instead.

## All Platforms

First, clone the TrenchBroom repository. `--recursive` is needed because we use git submodules:

```bash
git clone --recursive https://github.com/TrenchBroom/TrenchBroom.git
```

If you have an existing git clone, you can update submodules using:

```bash
git submodule update --init --recursive
```

---

## Windows

### Prerequisites

In order to develop, debug and compile TrenchBroom, you need to install tools listed below.

#### Visual Studio 2019

Download [VS 2019](https://visualstudio.microsoft.com/vs/) and install it.

You'll also need to install these dependencies using Visual Studio Installer (included with main installer):

- Workloads
  - **Desktop development with C++**
  
#### Project dependencies

- Download and install [Qt](https://www.qt.io/download) for MSVC 2017 32-bit and/or 64-bit
  - **Important**: You have to create a personal account
  - Minimum required version is `5.12`
- Download and install latest version of [CMake](http://www.cmake.org) for Windows
  - Make sure to add `cmake` as [global or user environment variable](https://support.shotgunsoftware.com/hc/en-us/articles/114094235653-Setting-global-environment-variables-on-Windows)
- Download and install latest version of [pandoc](http://www.pandoc.org)

### Project configuration with cmake

Create a subdirectory in TrenchBroom directory called `build`.

Open a command prompt and change directory to `build`:

```bash
cd <path/to/TrenchBroom>/build
```

Then, execute this command to configure the project:

- For 32-bit:
```bash
cmake .. -G "Visual Studio 16 2019" -T v142 -A Win32 -DCMAKE_PREFIX_PATH="C:\Qt\5.13.0\msvc2017"
```

- For 64-bit:
```bash
cmake .. -G "Visual Studio 16 2019" -T v142 -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\5.13.0\msvc2017_64"
```

> **Note**: Make sure to specify the correct Qt as `CMAKE_PREFIX_PATH` value.

### Build and debug TrenchBroom

To build and debug TrenchBroom, press `F5` key. After a success compilation process, TrenchBroom should automatically be opened in `Debug` mode.

---

## Linux

### Prerequisites

In order to develop, debug and compile TrenchBroom, you need to install tools listed below.

#### Project dependencies

TrenchBroom depends on:

- g++ >= 7
- Qt >= 5.9
- FreeImage: libfreeimage-dev
- OpenGL and GLU development headers (Mesa OpenGL development packages) freeglut3, freeglut3-dev, mesa-common-dev
- X11 video mode extension library: libxxf86vm-dev

If you have a debian-based distribution, open a command prompt and execute this command to install required dependencies:

```bash
sudo apt-get install g++-7 qt5-default libqt5svg5-dev freeglut3-dev libglew-dev mesa-common-dev build-essential libglm-dev libxxf86vm-dev libfreeimage-dev libfreetype6-dev pandoc cmake p7zip-full ninja-build
```

Or, on Fedora:

```bash
sudo dnf install g++ cmake qt5-qtbase-devel qt5-qtsvg-devel ninja-build freetype-devel freeimage-devel pandoc mesa-libGLU-devel
```

### Build TrenchBroom

Create a subdirectory in TrenchBroom directory called `build`.

Open a command prompt and change directory to `build`:

```bash
cd <path/to/TrenchBroom>/build
```

Then, execute this command to configure the project:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target TrenchBroom
```

> **Note**: You can replace `Debug` with `Release` if you want to create a release build.

---

## Mac OS X

### Prerequisites

In order to develop, debug and compile TrenchBroom, you need to install tools listed below.

#### Xcode

Download Xcode from the App Store.

#### Project dependencies

Open a command prompt and execute this command to install required dependencies:

```bash
brew install cmake qt pandoc
```

Finally, build the project:

```bash
mkdir build-xcode
cd build-xcode
cmake .. -GXcode -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
open TrenchBroom.xcodeproj
```

> **Note**: Don't enable `Address Sanitizer` in Xcode; it breaks rebuilding of the project (see [#1373](https://github.com/kduske/TrenchBroom/issues/1373)).

---

## How to release

Open a new command prompt.

Generate a changelog:

```bash
git log --oneline --decorate <LAST_REL_TAG>..HEAD
```

> **Note**: `<LAST_REL_TAG>` is replaced by whatever tag marks the last release. The generated log is then manually cleaned up.

Then, tag your changes:

```bash
git tag -a v2019.1 -m "This tag marks TrenchBroom 2019.1."
```

And finally, push the tagged release:

```bash
git push origin v2019.1
```
