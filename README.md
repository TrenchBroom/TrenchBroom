TrenchBroom
===========

TrenchBroom is a modern cross-platform level editor for Quake.

# Features
- True 3D editing, no 2D views required
- High performance renderer with support for huge maps
- Vertex editing with edge and face splitting that will not create invalid brushes
- Manipulation of multiple vertices at once (great for trisoup editing)
- Smart clip tool
- Move, rotate and flip brushes and entities
- Precise texture lock for all operations
- Smart entity property editors
- Graphical entity browser with drag and drop support
- Comprehensive texture application and manipulation tools
- Search and filter functions
- Unlimited undo and redo
- Point file support
- Automatic backup
- Support for .def and .fdg files, mods and multiple wad files
- Full documentation
- Free (as in beer) and open source (GPLv3)
- Cross platform (Windows, Mac OS X and Linux supported)

# Compiling
- Windows: You need Visual Studio 2010, read [Windows/Build.txt](Windows/Build.txt) for instructions
- Mac OS X: You need XCode 4, read [Mac/Build.txt](Mac/Build.txt) for instructions
- Linux: You need Code::Blocks, read [Linux/Build.txt](Linux/Build.txt) for instructions

# Contributing
- Bug reports and feature suggestions are welcome. Please submit them at https://github.com/kduske/TrenchBroom/issues
- If you wish to contribute code or improve the documentation, please get in touch with me at kristian.duske@gmail.com.
- All help is appreciated!

# Changes
## TrenchBroom 1.0.4
- Improved Quake.fgd and Quoth2.fgd.
- Improvements to clipboard pasting.
- Fixed a crash bug when loading maps with invalid brushes.
## TrenchBroom 1.0.3
- Fix off-by-one bug of mod list in map properties dialog.
- Snap vertex coordinates if close to integer coordinates.
- Add logging on all platforms.
- Properly merge spawnflags when loading fgd or def files.
- Properly set the size of point entities from fgd files.
- Fix crash bug when clipping all selected brushes.
