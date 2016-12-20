# Creating binary packages for linux

## Requirements

### Docker

The scripts uses docker to build and package the software in an isolated environment with pre-installed dependencies and tools.
This helps with "builds on my machine" == "builds anywhere".
To install docker see https://docs.docker.com/linux/step_one/ or use the following commands

```
# install docker package
sudo apt-get install docker.io

# add your user to docker group so you don't need root privileges to run docker
sudo usermod -aG docker yourname

# Logoff/on your usersession to make the new group work

# Test if it works with docker ps
>$ docker ps
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
```

## Building

The packaging directory has a make file with some make targets. The targets exist for 64bit and 32bit compilation.

* `make image-64` - Builds the dockerimage. It downloads the base image from dockerhub and applies the changes specified in the Dockerfile (base.Dockerfile). Running `docker images` after this should show some new images including `trenchbroom/build64`
* `make build-64` - Clones the upstream git repository, checks out the specified branch and then builds the software using cmake.
* `make package-64` - Builds deb, rpm and tar.gz distribution packages using fpm and writes to `pkg-out`
* `make bash-64` - Starts the docker container in interactive mode and launches bash. For testing purposes

**TL;DR** To create 64bit packages for the first time use

```
make image-64
make build-64
make package-64
```

## Details

### Dockerfile

The dockerfile includes apt-get instructions for the package dependencies. 
Add new packages and rebuild the image if dependencies change. 
The docker file is missing the `FROM` directive that specifies the base images. 
It is passed from the commandline on build invocation (see Makefile) the reason being that we need 2 containers i386/amd64 that use the same instructions but a different base image. 
Currently they are `ubuntu:14.04` for 64bit and `ioft/i386-ubuntu:14.04` for 32bit

### Docker Volumes

When the docker container is launched 3 directories from the host get mounted into the container. 
The build directory eg. `build64-out` which is the working directory. 
The pkg directory `pkg-out` where resulting packages get moved and the script directory where the build instructions are.

### Script directory

```
.
├── after-install.sh        # package script, this gets executed when the package is installed
├── after-remove.sh         # package script, this gets executed when the package is uninstalled
├── build.sh                # docker run script, this get executed for the the build target
├── copyright               # copyright notice, gets distributed
├── debian
│   └── control             # dpkg-shlibs is used to determine package dependencies. It will complain if this file is not present
├── desktop
│   ├── icon256x256
│   │   └── trenchbroom.png # high-res icon
│   ├── icon48x48
│   │   └── trenchbroom.png # low-res icon
│   └── trenchbroom.desktop # desktop file
├── Makefile                # Makefile with build instructions
└── package.sh              # docker run script, this get executed for the the package target
```

The Makefile includes all the important information. When building packages for a new release, version variables should be edited.

## Notes

### FHS

Trenchbroom comes as a single directory distribution and therefore doesn't fit under /usr/local/.
The most FHS compliant location imho is under /opt.
The post-install script creates a symlink in /usr/local/bin

### FPM

Packacke building ist done with FPM (https://github.com/jordansissel/fpm). It's a neat little tool. While it might not please official repository maintainers it does the job in a streamlined manner.

### Better package building

OpenSuse seems to have a service to build packages for a variety of distros in a professional manner.
This might be the enterprise way to do it.

http://openbuildservice.org/

## Troubleshooting / Testing

### Ubuntu 14.04 LTS / Debian 8 (jessie)

Uses deb package 

* **OK**: Installing and Downloading correct dependencies
* **OK**: Trenchbroom launches

Tested on amd64 and i386

### Fedora 23

Uses rpm package

* **NOTOK**: dnf cannot find packages for dependencies. Probably missing wx3 libraries with gtk2. Might work on Fedora 22 or with manual compilation of wxWidgets.
* **NOTOK**: Trenchbroom does not install nor launch nor build. Missing `libwx_gtk2u_adv-3.0.so.0`

Tested on amd64

### Mageia 5

Uses rpm package

* **OK**: Installing and Downloading some dependencies
* **NOTOK**: Installer downloads libfreeimage3 for wrong architecture. Manually installing lib64freeimage3 resolves the problem (`sudo urpmi lib64freeimage3`)
* **OK**: Trenchbroom launches

Tested on amd64
