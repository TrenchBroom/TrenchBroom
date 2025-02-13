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

#### Signing and notarization

The app bundle and the archive should be signed and notarized, otherwise macOS' gatekeeper
will refuse to run the app. The signing and notarization process is taken care of by CI,
but a number of prerequisites must be in place for it to work:

1. An Apple developer account.
2. A certificate of type "Developer ID Application".
3. An application specific password for notarization.

Read on to find out how to prepare CI for signing and notarization.

##### Prequisites for signing

Prerequisite: Xcode is logged into an Apple developer account.

To create a certificate for signing, open Xcode and go to account settings. Select the
Apple ID to use for signing and click on "Manage Certificates". Add a new certificate of
type "Developer ID Application". The certificate should be created and stored in your
Mac's keychain.

Now the certificate and it's corresponding private key must be imported into CI. This is
done by the `apple-actions/import-codesign-certs@v3` action (see `ci.yml`), but the action
requires that the certificate and private key are stored in a `.p12` file. To create such
a file, open Keychain Access on your mac and find the following two entries:
- the certificate (`Developer ID Application: <developer account user name> (<group id>)`)
- the private key (`Mac Developer ID Application: <developer account user name>`)

Select the two entries, right click and select "Export 2 items...". In the dialog, set the
file format to `.p12` and choose a location for the exported file. Then choose a secure
password for it -- take note of the password as we will need it again!

The `apple-actions/import-codesign-certs@v3` action accesses the `.p12` file contents and
the password via two github action secrets:
- `ACTIONS_MAC_SIGN_CERTIFICATES_P12` contains the base64 encoded `.p12` file contents. To
  put this data on the pasteboard, type the following in a terminal: 
  `base64 -i <path to .p12 file> | pbcopy`.
- `ACTIONS_MAC_SIGN_CERTIFICATES_P12_PASSWORD`: the password that we used to secure the
  `.p12` file.

Create these two secrets in the github repository settings.

With the certificate and private key in place, we only need to tell `macdeployqt` which
signing identity it should use. For this, we create another github action secret:
- `ACTIONS_MAC_SIGN_IDENTITY`: Set this to the developer account name used when creating
  the certificate. For example, if the certificate is named `Developer ID Application:
  Roger Workman (1234567)`, then the signing identity is "Roger Workman".

Now all prerequisites for signing the app bundle and the archive file should be in place
and we can move on the prerequisites for notarization.

##### Prequisites for notarization

Notarization is a process in which Apple scans the app bundle for malware and other
problems. For this, the app must be submitted to Apple using `notarytool`. This is also
taken care of on CI, but the process also requires some prerequisites, namely three more
github action secrets:
- `ACTIONS_MAC_NOTARIZATION_EMAIL`: This is the primary email address for the Apple
  developer account used for signing.
- `ACTIONS_MAC_NOTARIZATION_TEAM_ID`: This is the team ID associated with the signing
  certificate. For example, if the certificate is named `Developer ID Application: Roger
  Workman (1234567)`, then the team ID is "1234567".
- `ACTIONS_MAC_NOTARIZATION_PASSWORD`: This is an application specific password created
  for the Apple developer account that is used for signing. To create an application
  specific password, [follow these instructions](https://support.apple.com/en-us/102654).

##### Summary

To summarize, singing and notarization is automated on CI, but it requires six github
action secrets to be present:
- `ACTIONS_MAC_SIGN_CERTIFICATES_P12`: a base64 encoded `.p12` file containing the signing
  certificate and associated private key
- `ACTIONS_MAC_SIGN_CERTIFICATES_P12_PASSWORD`: the password used to secure the `.p12`
  file
- `ACTIONS_MAC_SIGN_IDENTITY`: the identity associated with the signing certificate
- `ACTIONS_MAC_NOTARIZATION_EMAIL`: the email address of the Apple developer account for
  notarization
- `ACTIONS_MAC_NOTARIZATION_TEAM_ID`: the team ID associated with the signing certificate
- `ACTIONS_MAC_NOTARIZATION_PASSWORD`: an application specific password

Once all these secrets are in place, CI should produce a signed and notarized archive file.

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
