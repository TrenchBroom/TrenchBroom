# TrenchBroom Filesystem Library

Contains TrenchBroom's filesystem access library. 

## Platform Independent Filesystem Access

`DiskIO.h` contains functions for accessing the filesystem in a platform independent way
(e.g. abstracts away case sensitivity issues). `File.h` contains abstractions for
accessing files that are backed by individual files on the filesystem or by files inside
an image (like a zip file) or a virtual filesystem. `Reader.h` contains facilities to
read from such files.

## Virtual Filesystem

This library also contains support for a virtual filesystem that is made up of a hierarchy
of physical filesystems. The physical filesystems can be backed by the system's filesystem
or by image files such as zip files, pak files, or wad files. Once instantiated,
filesystems can be mounted into a virtual filesystem which allows accessing the files.
The virtual filesystem can also resolve shadowed files correctly.
