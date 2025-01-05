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

## Dependencies

### VCPKG

TrenchBroom uses [vcpkg](https://vcpkg.io/) to manage build dependencies except for Qt. vcpkg is integrated into TrenchBroom's build system and will download and build all dependencies once  during cmake's configure phase. This is an automatic process, but it can take a little while when it happens for the first time.

### Qt

TrenchBroom depends on Qt version 6.7. It might work with later versions, but earlier versions will definitely not work. The easiest way to install a specific version of Qt for your platform is the official installer, which requires you to create an account. Follow [these instructions](https://doc.qt.io/qt-6/qt-online-installation.html) to download and and run the Qt installer. Then, install the latest version of Qt 6.7 for your platform.

Note the path where Qt was installed. For example, on Windows the default installation path would look like `C:\Qt\6.7.3\`. We will refer to this path later on as `<QT_INSTALL_DIR>`.

---

## Docker

An external unofficial project called [Dockerized TrenchBroom](https://github.com/jonathanlinat/dockerized-trenchbroom) is available for developers. It facilitates the compilation of TrenchBroom's source code and the creation of binaries using [Docker](https://www.docker.com/).

---

## Windows

### Prerequisites

In order to develop, debug and compile TrenchBroom, you need to install tools listed below.

#### Visual Studio 2022

Download [VS 2022](https://visualstudio.microsoft.com/vs/) and install it.

You'll also need to install these dependencies using Visual Studio Installer (included with main installer):

- Workloads
  - **Desktop development with C++**
  
#### Project dependencies

- Download and install Qt (see above).
- Download and install latest version of [CMake](http://www.cmake.org) for Windows
  - Make sure to add `cmake` as [global or user environment variable](https://knowledge.autodesk.com/support/shotgrid/learn-explore/caas/CloudHelp/cloudhelp/ENU/SG-RV/files/rv-knowledge-base/SG-RV-rv-knowledge-base-rv-setting-global-variables-windows-html-html.html)
- Download and install latest version of [pandoc](http://www.pandoc.org)

### Project configuration with cmake

Create a subdirectory in TrenchBroom directory called `build`.

Open a command prompt and change directory to `build`:

```bash
cd <path/to/TrenchBroom>/build
```

Then, execute this command to configure the project:

```bash
cmake .. -G"Visual Studio 17 2022" -T v143 -A x64 -DCMAKE_PREFIX_PATH="<QT_INSTALL_DIR>\msvc2022_64"
```

> **Note**: Make sure to specify the correct Qt as `CMAKE_PREFIX_PATH` value.

### Build and debug TrenchBroom

To build and debug TrenchBroom, press `F5` key. After a success compilation process, TrenchBroom should automatically be opened in `Debug` mode.

---

## Linux

### Prerequisites

In order to develop, debug and compile TrenchBroom, you need to install tools listed below.

#### Project dependencies

First, download and install Qt (see above).

If you have a debian-based distribution, open a command prompt and execute this command to install required dependencies:

```bash
sudo apt-get install g++ libxi-dev libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev libglew-dev libxrandr-dev build-essential libglm-dev libxxf86vm-dev libfreetype6-dev libfreeimage-dev libtinyxml2-dev pandoc cmake p7zip-full ninja-build curl
```

### Build TrenchBroom

Create a subdirectory in TrenchBroom directory called `build`.

Open a command prompt and change directory to `build`:

```bash
cd <path/to/TrenchBroom>/build
```

Then, execute this command to configure the project:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="cmake/packages;<QT_INSTALL_DIR>/gcc_64"
cmake --build . --target TrenchBroom
```

> **Note**: You can replace `Debug` with `Release` if you want to create a release build.

---

## macOS

### Prerequisites

In order to develop, debug and compile TrenchBroom, you need to install tools listed below.

#### Xcode

Download Xcode from the App Store.

#### Homebrew

Homebrew is a package manager for macOS. We will use it to install a couple of dependencies. Download and install it from [here](https://brew.sh/).

#### Project dependencies

Open a command prompt and execute the following commands to install required dependencies:

```bash
sudo xcode-select --install
brew install cmake ninja pandoc python pkg-config
```

Finally, build the project:

```bash
mkdir build
cd build
cmake .. -GNinja -DCMAKE_PREFIX_PATH="<QT_INSTALL_DIR>/macos"
cmake --build . --target TrenchBroom
```

---

## How to release

Open a new command prompt and change into the TrenchBroom git repository.

Create a new tag:

```bash
git tag -a v2019.1 -m "This tag marks TrenchBroom 2019.1."
```

Then push the tagged release:

```bash
git push origin v2019.1
```

Finally, create the release on github. Generate the changelog from the release editor.
