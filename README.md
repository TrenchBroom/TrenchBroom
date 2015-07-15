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

## Changes
### TrenchBroom 1.1.6
- Fix crash when dragging entities on some OpenGL drivers (ericw)
- Fix some crashes when loading mdl and def files (ericw)
- Fix for editor view getting out of sync when exiting vertex mode (ericw)
- Improve floating-point precision for map loading, raise threshold for rounding coords (ericw)
- Workaround "force integer plane points" mode sometimes choosing wrong coordinates (ericw)

### TrenchBroom 1.1.5
- Fix crash during vertex manipulation (ericw)
- Fix vertex drift issue when copy/pasting (ericw)
- Fix for entity key/value corruption when renaming a key (ericw)
- Add checkbox to filter detail brushes (ericw)
- Work around modifier keys getting stuck (ericw)
- Fix for some valid wad files being rejected (ericw)

### TrenchBroom 1.1.4
- Fix crash when updating graphics resources (ericw)
- Fix vertex drift issue leading to grid snapping problems (ericw)

### TrenchBroom 1.1.3
- Fix vertex precision problems when copy/pasting brushes (ericw)

### TrenchBroom 1.1.2
- Enable texture names starting with '{' (ericw)
- Fix hang when adding multiple updating entity targets (ericw, necros)
- Simplify build instructions (ericw)
- Fix vertex drift on map load (ericw)
- Fix crash with colors in byte format (ericw)
- Add option to invert zoom direction when using Alt+MMB (kduske)

### TrenchBroom 1.1.1
- More natural snapping when resizing brushes
- Remove limit on maximum mip texture dimensions
- Don't fall back to software renderer on OS X

### TrenchBroom 1.1.0
- Keyboard customization
- Restrict to X or Y axis when moving objects
- Limit face points to integer coordinates
- New duplication options (drag to duplicate and duplicate in specific direction)
- New information bar at top of 3D view
- Moved search field to information bar
- Improved precision and stability of vertex tool
- Select objects by line number
- Brush resizing feels more natural and snaps more accurately
- Autosave when the applicaton exits
- More brush filtering options
- Better performance for view filters
- Clip tool remembers last clip side
- Clip tool adds new brushes to parent entities of clipped brushes
- Clipped brushes remain selected after deactivating the clip tool
- Double click on brush selects all brushes belonging to the containing entity
- Shift + double click on brush to select all of its faces
- Paint selection
- Show entity angle in 3D view
- Camera tracks for point file navigation and center on selection
- Show compass in 3D view
- Buttons for texture flipping in face inspector
- Hotkeys to activate inspector tabs
- Smooth camera navigation with WASD keys
- Customizable texture browser icon size
- Render skip / clip / hint / trigger and liquid brushes semi-transparently
- Numerous bug fixes

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
