# TrenchBroom

TrenchBroom is a modern cross-platform level editor for Quake.

- Website:   http://kristianduske.com/trenchbroom
- Downloads: http://kristianduske.com/trenchbroom/downloads.php

## Features
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

## Compiling
- Windows: You need Visual Studio 2010, read [Windows/Build.txt](Windows/Build.txt) for instructions
- Mac OS X: You need XCode 4, read [Mac/Build.txt](Mac/Build.txt) for instructions
- Linux: You need Code::Blocks, read [Linux/Build.txt](Linux/Build.txt) for instructions

# Contributing
- Bug reports and feature suggestions are welcome. Please submit them at https://github.com/kduske/TrenchBroom/issues
- If you wish to contribute code or improve the documentation, please get in touch with me at kristian.duske@gmail.com.
- All help is appreciated!

# Credits
- wxWidgets www.wxwidgets.org
- FreeType library www.freetype.org
- FreeImage library www.freeimage.org
- GoogleTest library code.google.com/p/googletest/
- GoogleMock library code.google.com/p/googlemock/
- Quake icons by Th3 ProphetMan th3-prophetman.deviantart.com
- Hexen 2 icon by thedoctor45 thedoctor45.deviantart.com

## Changes
### TrenchBroom 1.0.9
- Fix the rotation tool handle position

### TrenchBroom 1.0.8
- Fix the rotation tool
- Allow snapping faces to the grid when resizing brushes
- Improved major grid line shading (rebb)
- Fixed umlauts in About dialog

### TrenchBroom 1.0.7
- Emergency bugfix

### TrenchBroom 1.0.6
- New text rendering system (faster and fewer glitches)
- Brush drawing now more accurate (see docs)
- Option to use integer plane point coordinates
- Recompute vertices after every change to brush geometry
- Major lines of the grid are rendered thicker
- Read write protected map files
- Don't crash when trying to save a write protected file
- Internal Worldspawn properties set to read only
- Fix 3D view focus issues on Windows
- Drop invalid brushes in parser instead of crashing
- Always autosave (don't wait until the map is changed)
- Keep more autosaves and save only every 10 minutes
- Don't show rotation decorators for invisible entities
- Documentation updates and fixes

### TrenchBroom 1.0.5
- Fixed a crash when undoing edge and face move operations.
- Improved map loading speed.
- Improved accuracy of vertex computation.
- Added antialiasing for grid lines (rebb).
- Thin out grid lines after a certain distance (rebb).
- Draw thinner grid lines for small grid sizes.
- Added face shading depending on view direction (rebb).
- Fixed a performance problem when editing face attributes.
- Place initial brush in new maps and make the camera look at it.
- Position pasted objects so that they line up with objects under the mouse cursor.
- Added option to use Alt+MMB to move the camera forward and backward.
- Fixed parsing of color values in FGD files.
- Improved Quake.fgd and Quoth2.fgd.
- Updated and fixed some errors in the documentation.

### TrenchBroom 1.0.4
- Improved Quake.fgd and Quoth2.fgd.
- Improvements to clipboard pasting.
- Fixed a crash bug when loading maps with invalid brushes.

### TrenchBroom 1.0.3
- Fix off-by-one bug of mod list in map properties dialog.
- Snap vertex coordinates if close to integer coordinates.
- Add logging on all platforms.
- Properly merge spawnflags when loading fgd or def files.
- Properly set the size of point entities from fgd files.
- Fix crash bug when clipping all selected brushes.
