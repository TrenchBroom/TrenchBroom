# Preference Dialog (32x32)
The preference dialog is split into the following four preference panes, which are selectable with four large buttons in a toolbar at the top of the window:
- Game Setup: This is where the user sets the options pertaining to particular games, such as the location of the game on the hard drive etc.
- View Setup: Here, the user can set some options related to how objects are displayed in the main 3D view.
- Mouse Setup: In this preference pane, the user can tune how the camera reacts to mouse input (e.g. sensitivity etc.)
- Keyboard Setup: This is where the user can change the keyboard bindings of the various menu commands.

# Generic Icons (16x16)
These icons are to be used in several places all over the editor and therefore should be as generic as possible.
- Add something to a list of things: Could be as simple as a plus sign.
- Remove something from a list of things.
- Move something up / down / left / right in a list of things.
- Delete something (this is different from removing because it will cause something to be erased permanently).
- Select something from disk (open a browser window / open file dialog).

# Application Icon
This icon must be a vector graphic because I may need to adapt it and scale it from 16x16 up to 1014x1024. Currently, TB has a Quake crate as its icon, which I like. However, this crate is recognizable only to Quake players, and since TB is going to be available for other games as well, it is not a very good icon anymore. Maybe we can abstract from the look of the crate a bit. One element of the current icon I definitely like is that the crate has grid lines projected onto its faces. Since the grid projection is quite unique to TrenchBroom, I think it's a good idea that it shows up in the icon as well.

In addition to the crate, I can imagine the icon featuring some sort of instrument that is being used by architects (e.g., pair of compasses, set square, steel square, etc.).

# Document Icon
The document icon is what the user sees when they associate a file type (such as .map) with TrenchBroom. It should be some combination of a generic document icon (a white sheet of paper) and the TB logo, or an element of it.