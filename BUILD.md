#All Platforms  
 
###Dependencies  
######pandoc  
- We use [pandoc](http://www.pandoc.org) to generate the documentation. Install a binary distribution from the website and make sure that it is in your PATH, otherwise your builds will fail.

######wxWidgets  
- TrenchBroom utilizes [wxWidgets](http://wxwidgets.org/) for it's GUI. it currently relies on version 3.1.0

>You may want to keep this [wxWidgets 3.1.0 release][Current WxWidget Release Link] link open, until you finish building; all relevent wxwidget related files mentioned below, can be found here.  

___  
#Windows

>Generally, the cmake scripts don't handle paths with spaces very well, so make sure that you check out the TrenchBroom source repository somewhere on a path without any spaces.



#### For Visual Studio (only tested with 2010)####
- Get the binary build of wxWidgets 3.1.0 for your platform [here][Current WxWidget Release Link]

Platform   | Visual Studio Version 
:---------:|:-----------------------:
**vc90**   |   Visual Studio 2008 
**vc100**  |   Visual Studio 2010 
**vc110**  |   Visual Studio 2012
**vc120**  |   Visual Studio 2013
**vc140**  |   Visual Studio 2015
 

  
  - For 32bit builds, you need the following files:  

>download the relevant version for your compiler, _above_. e.g. `wxMSW-<version>_vc140_Dev.7z` for Visual Studio 2015  

>- wxWidgets-<version>_headers.7z  
>- wxMSW-<version>_vc100_Dev.7z  
>- wxMSW-<version>_vc100_ReleaseDLL.7z  
>- wxMSW-<version>_vc100_ReleasePDB.7z  


- Unpack all files into `C:\wxWidgets-<version>` so that "include" and "lib" directories are at the same level after unpacking.  

<b>The directory layout should look like this:</b>  
```sml
wxWidgets-3.1.0\ 
\include
  \msvc 
    \wx
  \wx 
    <wxwidgets header files>
\lib
  \vc100_dll
    <wxwidgets libraries>
```

  - Set a new environment variable `WXWIN=C:\wxWidgets-<version>` (replace the path with the path where you unpacked wxWidgets).
  - If you want to run the binaries without using the installer, add `%WXWIN%\lib\vc100_dll` to your path. The relevant parts of my PATH variable look something like this:
`C:\Program Files (x86)\CMake 2.8\bin;c:\wxWidgets-3.1.0\lib\vc100_dll;`
  - Download and install CMake for Windows (www.cmake.org)
  - Open a command prompt and change into the directory where you unpacked the TrenchBroom sources.
  - Create a new directory, e.g. "`build`", and change into it.
  - Run the following two commands  
    `cmake .. -DCMAKE_BUILD_TYPE=Release`  
    `cmake --build . --config Release --target TrenchBroom`  
     You can replace "Release" with "Debug" if you want to create a debug build. This is also recommended if you want to work on the source in Visual Studio.  
     
__For MinGW 64__  
  - Download and install [MinGW](http://mingw-w64.sourceforge.net/)  
      - Scroll down to Mingw-builds and select the appropriate version for your OS (32 or 64 Bit), then select the SJLJ variant.  
      - Add `<MinGW installation dir>/bin` to your PATH.  
  - Download wxWidgets source [wxWidgets-3.1.0.7z][7z sourcecode]  
      - Unpack the sources somewhere to `C:\wxWidgets-3.1.0`  
  - Build wxWidgets  `  
  - Open a command prompt and cd into `C:\wxWidgets-3.1.0\build\msw`  
      - Run these commands  
      **`mingw32-make -f makefile.gcc SHARED=1 UNICODE=1 OPENGL=1 BUILD=release clean`**  
      **`mingw32-make -f makefile.gcc SHARED=1 UNICODE=1 OPENGL=1 BUILD=release`**  
      **`mingw32-make -f makefile.gcc SHARED=1 UNICODE=1 OPENGL=1 BUILD=debug clean`**  
      **`mingw32-make -f makefile.gcc SHARED=1 UNICODE=1 OPENGL=1 BUILD=debug`**  
  - Set a new environment variable **`WXWIN=C:\wxWidgets-<version>`** (replace the path with the path where you unpacked wxWidgets).  
  - If you want to run the binaries without using the installer, add **`%WXWIN%\lib\gc_dll`** to your PATH. The relevant parts of my PATH variable look something like this:  
    **`C:\Program Files (x86)\CMake 2.8\bin;c:\wxWidgets-3.1.0\lib\gcc_dll;`**  
      
___
Linux
===  
<a name="Linux"></a>  
``` 
!!!NOTE: as of April 20, 2016, Debian/Ubuntu and Arch, repos have not updated to wxwidgets3.1.0 
Binaries and headers are outdated, manual compilation is required.  
```
#####Dependencies  
>Compiling **wxWidgets 3.1.0** requires the following dependencies. You should install them using the package manager of your Linux distribution.  

- **g++** GNU c++ compiler  _(included in arch)_
- **GTK2** and development packages: **`libgtk2.0-dev`** _**(GTK3 will NOT work)**_  
- **FreeImage**: **`libfreeimage-dev`** 
- **OpenGL** and **GLU** development headers _(Mesa OpenGL development packages)_:  
  **`freeglut3`, `freeglut3-dev`, `mesa-common-dev`**  
- **X11 video mode extension library**: **`libxxf86vm-dev`**  
- **wxWidgets** development headers: **`wx3.X-headers` `libwxbase3.X-dev` `libwxgtk-media3.X-dev`**  

>current version required is **v3.1.0** if your distros repositories do not have the current version, ***you need to compile!***

####DEBIAN-BASED DISTRIBUTIONS    

- Install them with this command:  

```bash 
sudo apt-get install libgtk2.0-dev freeglut3 freeglut3-dev libglew-dev mesa-common-dev build-essential libglm-dev libxxf86vm-dev libfreeimage-dev pandoc wx3.0-headers libwxbase3.0-dev libwxgtk-media3.0-dev
```

```
!!!NOTE: these headers are outdated. instructions on installing current version below.
```   

####ARCH-BASED DISTRIBUTIONS  

>NOTE:there is no distinction between a core ***"package"*** and ***"package-dev"*** on Arch, ***"package"*** includes both (and are often missing lib prefix), eg. "**`wxgtk`**" contains "**`wx3.X-headers`**", "**`libwxbase3.X-dev`**" and "**`libwxgtk-media3.X-dev`**" etc

- Install them with this command:  
```bash
sudo pacman -S gtk2 freeglut glew mesa glm freeimage libxxf86vm pandoc wxgtk
```
- check if wxwidgets meets the current version requirement.   
```bash  
pacman -Q wxgtk  
```

```
!!!NOTE:If it doesn't meet the version requirement you'll need to build wxwidgets from source,   
as instructed below.  
```  

>Compiling and linking TrenchBroom requires a working OpenGL installation. This page may help you if you see linker errors about missing GL libraries:
[Install_Mesa_OpenGL_on_Linux_Mint](http://www.wikihow.com/Install-Mesa-%28OpenGL%29-on-Linux-Mint)  

>[Some more detailed](http://andyp123.blogspot.de/2013/03/running-trenchbroom-quake-editor-on.html) (possibly outdated) information about building **TrenchBroom** on Linux:  
  

####wxWidgets
>**Warning:** building/installing ***may*** cause future compile issues with other programs that rely on older versions of **wxWidgets**  

- You have two options here: Either install wxWidgets 3.1.0 using your package manager of choice, or download and build it yourself. For the latter, you may follow these instructions:
  - Get the latest source [wxWidgets-3.1.0.tar.bz2][tar.bz2 sourcecode] and unpack them.
  - Move the unpacked directory someplace where you want to keep it.  
  - Open a terminal and change into the wxwidgets directory.  
  - Create two directories: build-release and build-debug (don't rename those!)  
  from terminal:`mkdir build-release build-debug`  
  - Run the following commands:  

######wxRelease  
 - **change into directory** > **configure** > **compile and install**

```bash
cd build-release
../configure --disable-shared --with-opengl --with-gtk=2
make
sudo make install
```  
######wxDebug  
 - **change into directory** > **configure** > **compile and install**

```bash
cd ../build-debug
../configure --enable-debug --with-opengl --with-gtk=2
make
sudo make install
```  

>This will only build and install binary libraries, if the dev headers above in the dependency section are  
not up to date, you will need to install the current versions headers, to your systems **`/include`** directory.  
Make sure your terminal is open inside your **`<wxWidgets-3.1.0 directory>`** and Run:  
**`sudo cp -rfp ./include/wx /usr/include/wx`**  

####CMake  
- Install CMake using your package manager:  
 **DEBIAN-BASED:** `sudo apt-get install cmake`  
 **ARCH-BASED:** `sudo pacman -S cmake`  
  
####Build TrenchBroom  
- Open a terminal and change into the directory where you unpacked the TrenchBroom sources  
- Create a new directory, e.g. "`build`", and change into it. e.g.`mkdir build` `cd build`  
- Run the following two commands  
  `cmake .. -DCMAKE_BUILD_TYPE=Release`  
  `cmake --build . --target TrenchBroom`  
  You can replace "`Release`" with "`Debug`" if you want to create a debug build. Also change the value of the wxWidgets_PREFIX variable to point to your wxWidgets build-debug directory in that case. 

####Packaging  
- If you want to create packages for Linux (deb or rpm), then you'll need to install these packages: `devscripts`, `debhelper`, `rpm`  
  `sudo apt-get install devscripts debhelper rpm`  

####Linux Notes  
>You can install your preferred wxWidgets configuration using `make install`. If you wish to do this, then you can omit specifying the **`wxWidgets_PREFIX`** variable when generating the build configs with CMake.  

>On some systems, such as Xubuntu, you may have to pass the following extra parameter to CMake when creating the build   scripts: **`-DFREETYPE_INCLUDE_PATH=/usr/include/freetype2/freetype`**  

>  So the first cmake command should be  
  ```bash
  make .. -DCMAKE_BUILD_TYPE=Release -DFREETYPE_INCLUDE_PATH=/usr/include/freetype2/freetype
  ```  

___  
MACOSX  
---  
######Build environment  
**1.** Get **Xcode** from the App Store  

######Dependencies 

**2. CMake & ninja**  
- Install CMake (required) and Ninja (optional). For example, with homebrew:  
  `brew install cmake ninja`  

**3. wxWidgets**   

- Get the latest wxWidgets sources; [wxWidgets-3.1.0.tar.bz2][tar.bz2 sourcecode]
- Move the unpacked directory someplace where you want to keep it.  
- Open a terminal and change into the wxwidgets directory.  
- Apply the patches in TrenchBroom/patches/wxWidgets  
- Create two directories: build-release and build-debug (don't rename those!) 
`mkdir build-release build-debug`
- Run the following commands:  
  
######wxRelease  
 - **change into directory** > **configure** > **compile and install**

  ```bash
cd  ./build-release
../configure --with-osx_cocoa --disable-shared --with-opengl --enable-universal-binary=i386,x86_64 --with-macosx-version-min=10.6 --prefix=$(pwd)/install
make
make install
  ```  

######wxDebug  

 - **change into directory** > **configure** > **compile and install**  

  ```bash
cd ../build-debug
../configure --enable-debug --with-osx_cocoa --with-opengl --enable-universal-binary=i386,x86_64  --with-macosx-version-min=10.6 --prefix=$(pwd)/install
make  
make install   
  ```  

---  
**4.** Build **TrenchBroom**  
>make sure to replace `/your/wxWidgets/directory/build-release/install` with your **actual** directory path.  

- Open a terminal changed to the **`TrenchBroom`** source folder.

######Release
```bash
  mkdir build-ninja  
  cd build-ninja  
  cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DwxWidgets_PREFIX=/your/wxWidgets/directory/build-release/install
  ninja  
```  
######Debug
- same as above except `-DCMAKE_BUILD_TYPE=Debug`
  you may want to create a unique folder for debug build.
  
---
> To generate an Xcode project for developing with:  
from **`TrenchBroom`** source directory  
```bash
mkdir build-xcode  
cd build-xcode  
cmake .. -GXcode  
```  
Open **`TrenchBroom.xcodeproj`** in **Xcode**  

####MACOSX Notes		
>You can install your preferred wxWidgets configuration using make install. If you wish to do this, then you can omit specifying the **`wxWidgets_PREFIX`** variable when generating the build configs with Cmake. 

  [Current WxWidget Release Link]: https://github.com/wxWidgets/wxWidgets/releases/tag/v3.1.0  
  [7z sourcecode]: https://github.com/wxWidgets/wxWidgets/releases/tag/v3.1.0/wxWidgets-3.1.0.7z
  [tar.bz2 sourcecode]: https://github.com/wxWidgets/wxWidgets/releases/tag/v3.1.0/wxWidgets-3.1.0.tar.bz2
