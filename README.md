# TrenchBroom

TrenchBroom is a modern cross-platform level editor for Quake-engine based games.

- Website:   http://kristianduske.com/trenchbroom
- Downloads: http://kristianduske.com/trenchbroom/downloads.php

## Features
* **General**
	- Full support for editing in 3D and in up to three 2D views
	- High performance renderer with support for huge maps
	- Unlimited Undo and Redo
	- Macro-like command repetition
	- Issue browser with automatic quick fixes
	- Point file support
	- Automatic backups
	- Free and cross platform
* **Brush Editing**
	- Robust vertex editing with edge and face splitting and manipulating multiple vertices together
	- Clipping tool with two and three points
	- CSG operations: merge, subtract, intersect
	- UV view for easy texture manipulations
	- Precise texture lock for all brush editing operations
	- Multiple texture collections
* **Entity Editing**
	- Entity browser with drag and drop support
	- Support for FGD and DEF files for entity definitions
	- Mod support
	- Entity link visualization
	- Displays 3D models in the editor
	- Smart entity property editors

## Compiling
- Read [Build.txt](Build.txt) for instructions

# Contributing
- Bug reports and feature suggestions are welcome. Please submit them at https://github.com/kduske/TrenchBroom/issues
- If you wish to contribute code or improve the documentation, please get in touch with me at kristian.duske@gmail.com.
- All help is appreciated!

# Credits
- wxWidgets www.wxwidgets.org
- FreeType www.freetype.org
- FreeImage www.freeimage.org
- GLEW glew.sourceforge.net
- GoogleTest code.google.com/p/googletest/
- GoogleMock code.google.com/p/googlemock/
- CMake www.cmake.org
- Pandoc www.pandoc.org
- Quake icons by Th3 ProphetMan th3-prophetman.deviantart.com
- Hexen 2 icon by thedoctor45 thedoctor45.deviantart.com
- Source Sans Pro font www.google.com/fonts/specimen/Source+Sans+Pro

## Changes
### TrenchBroom 2.0.0 Beta e439e68
- Fix keyboard shortcuts not working after cycling an editing viewport

### TrenchBroom 2.0.0 Beta 0f4b6d2
- Add Wavefront OBJ exporter
- Add support for Alt+RMB drag for scrolling browser views
- Add support for zooming 2D views with Alt+MMB
- Add crash reporter by @ericwa
- Write entity and brush ID comments like Radiant
- Better support for OS themes
- Fix a crash when undoing too many times
- Fix a problem that made certain keyboard shortcuts unusable on Windows
- Fix ESC key not usable in keyboard shortcut editor
- Fix dropdown menus not working on popup windows
- Fix a parse error when copy / pasting brush faces
- Fix reset to defaults not working for certain keyboard shortcuts
- Fix a crash when opening entity or texture browser on Windows with Intel hardware
- Fix a crash in clip tool when dragging clip points
- Fix an OpenGL related crash on Linux
- Preliminary FreeBSD support by @danfe
- Upgrade to wxWidgets 3.1.0
- Support for Visual Studio 15 by @ericwa
- Minor performance improvements
- Minor fixes

### TrenchBroom 2.0.0 Beta 93e34bf
- Complete rewrite with many new features and bug fixes
- New 2D views with multiple view layouts
- Support for multiple games
- Macro-like command repetition
- Issue browser
- CSG operations: merge, subtract, intersect
- UV view for easy texture manipulations
- Proper Linux support

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
