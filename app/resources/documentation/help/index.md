% TrenchBroom Documentation
% Kristian Duske
% 11-13-2015

# Introduction {#introduction}

TrenchBroom is a level editing program for brush-based game engines such as Quake, Quake 2, and Hexen 2. TrenchBroom is easy to use and provides many simple and advanced tools to create complex and interesting levels with ease. This document contains the documentation for TrenchBroom. Reading this document will teach you how to use TrenchBroom and how to use its advanced features.

## Features {#features}

* **General**
	- Full support for editing in 3D and in up to three 2D views
	- High performance renderer with support for huge maps
	- Unlimited Undo and Redo
	- Macro-like command repetition
	- Point file support
	- Automatic backups
	- Free and cross platform
	- Issue browser with automatic quick fixes
* **Brush Editing**
	- Robust vertex editing with edge and face splitting
	- Clipping tool with two and three points
	- CSG operations: merge, subtract, intersect, partition
	- UV view for easy texture manipulations
	- Precise texture lock for all brush editing operations
* **Entity Editing**
	- Entity browser with drag and drop support
	- Entity link visualization
	- Displays 3D models in the editor
	- Smart Entity Property Editors

## About This Document

This document is intended to help you learn to use the TrenchBroom editor. It is not intended to teach you how to map, and it isn't a tutorial either. If you are having technical problems with your maps, or need information on how to create particular effects or setups for the particular game you are mapping for, you should ask other mappers for help (see [References and Links](#references_and_links) to find mapping communities and such). This document will only teach you how to use this editor. 

# Getting Started {#getting_started}

This section starts off with a small introduction to the most important technical terms related to mapping for brush-based engines. Additionally, we introduce some concepts on which TrenchBroom is built. Afterwards, we introduce the welcome window, the game selection dialog, we give an overview of the main window and explain the camera navigation in the 3D and 2D views.

## Preliminaries {#preliminaries}

In this section we introduce some technical terms related to mapping in Quake-engine based games. It is not important that you understand every detail of all of these terms, but in order to understand how TrenchBroom works, you should have a general idea how maps are structured and how TrenchBroom views and manages that structure. In particular, this section introduces some concepts that we added to the map structures (without changing the file format, of course). Knowing and understanding these concepts will help you to get a grip on several important aspects of editing levels in TrenchBroom.

### Map Definitions

In Quake-engine based games, levels are usually called maps. The following simple specification of the structure of a map is written in extended Backus-Naur-Form (EBNF), a simple syntax to define hierarchical structures that is widely used in computer science to define the syntax of computer languages. Don't worry, you don't have to understand EBNF as we will explain the definitions line by line.

    1. Map 			= Entity {Entity}
    2. Entity 		= {Property} {Brush}
    3. Property		= Key Value
    4. Brush 		= {Face}
    5. Face         = Plane Texture Offset Scale Rotation ...

The first line specifies that a **map** contains of an entity followed by zero or more entities. In EBNF, the braces surrounding the word "Entity" indicate that it stands for a possibly empty sequence of entities. Altogether, line one means that a map is just a sequence of one or more entities. An **entity** is a possibly empty sequence of properties followed by zero or more brushes. A **property** is just a key-value pair, and both the key and the value are strings (this information was omitted from the EBNF).

Line four defines a **Brush** as a possibly empty sequence of faces (but usually it will have at least four, otherwise the brush would be invalid). And in line five, we finally define that a **Face** has a plane, a texture, the X and Y offsets and scales, the rotation angle, and possibly other attributes depending on the game.

In this document, we use the term _object_ to refer to entities and brushes.

### TrenchBroom's View of Maps

TrenchBroom organizes the objects of a map a bit differently than how they are organized in the map files. Firstly, TrenchBroom introduces two additional concepts: [Layers](#layers) and [Groups](#groups). Secondly, the worldspawn entity is hidden from you and its properties are associated with the map. To differentiate TrenchBroom's view from the view that other tools and compilers have, we use the term "world" instead of "map" here. In the remainder of this document, we will use the term "map" again.

    1. World 			= {Property} DefaultLayer {Layer}
    2. DefaultLayer 	= Layer
    3. Layer 			= Name {Group} {Entity} {Brush}
    4. Group 			= Name {Group} {Entity} {Brush}

The first line defines the structure of a map as TrenchBroom sees it. To TrenchBroom, a **World** consists of zero or more properties, a default layer, and zero or more additional layers. The second line specifies that the **DefaultLayer** is just a layer. Then, the third line defines what a **Layer** is: A layer has a name (just a string), and it contains zero or more groups, zero or more entities, and zero or more brushes. In TrenchBroom, layers are used to partition a map into several large areas in order to reduce visual clutter by hiding them. In contrast, groups are used to merge a small number of objects into one object so that they can all be edited as one. Like layers, a **Group** is composed of a name, zero or more groups, zero or more entities, and zero or more brushes. In case you didn't notice, groups induce a hierarchy, that is, a group can contain other sub-groups. All other definitions are exactly the same as in the previous section.

To summarize, TrenchBroom sees a map as a hierarchy (a tree). The root of the hierarchy is called world and represents the entire map. The world consists first of layers, then groups, entities, and brushes, whereby groups can contain more groups, entities, and brushes, and entities can again contain brushes. Because groups can contain other groups, the hierarchy can be arbitrarily deep, although in practice groups will rarely contain more than one additional level of sub groups.

### Brush Geometry {#brush_geometry}

In standard map files, the geometry of a brush is defined by a set of faces. Apart from the attributes defining the texture mapping, a face also specifies a plane using three plane points. The plane is oriented by the order in which the three points are written in the face specification in the map file. Here is an example:

    ( 32 32 -3824  ) ( -32 32 -3824  ) ( -32 96 -3824 ) mmetal1_2 0 0 0 1 1

The plane points are given as three groups of three numbers. Each group is made up of three numbers that specify the X,Y and Z coordinates of the respective plane point. The three points, p1, p2 and p3, define two vectors, v1 and v2, as follows:

	  *p2
	 /|\
	  |
	  v2
	  |
	p1*--v1--->*p3

The normal of the plane is in the direction of the cross product of v1 and v2. In the diagram above, the plane normal points towards you. Together with its normal, the plane divides three dimensional space into two half spaces: The upper half space above the plane, and the lower half space below the plane. In this interpretation, the volume of the brush is the intersection of the lower half spaces defined by the face planes. This way of defining brushes has the advantage that all brushes are automatically convex. However, this representation of brushes does not directly contain any other geometric information of the brush, particularly its vertices, edges, and facets, which must be computed from the plane representation. In TrenchBroom, the vertices, edges, and facets are called the **brush geometry**. TrenchBroom uses the same method as the BSP compilers to compute the brush geometry. Having the brush geometry is necessary mainly for two things: Rendering and vertex editing.

### Entity Definitions {#entity_definitions}

To TrenchBroom, entities are just a sequence of properties (key value pairs), most of which are without any special meaning. In particular, the entity properties do not specify which model TrenchBroom should display for the entity in its viewports. Addtionally, some property values have specific types, such as color, angle, or position. But these types cannot be hardcoded into the editor, because depending on the game, mod, or entity, a property called "angle" may have a different meaning. To give TrenchBroom more information on how to interpret a particular entity and its properties, an entity definition is necessary. Entity definitions specify the spawnflags of an entity, the names and types of its properties, the model to display in the editor, and other things. They are usually specified in a separate file that you can [load into the editor](#entity_definition_setup).

### Mods {#mods}

A mod (short for game modification) is a way of extending a Quake-engine based game with custom gameplay and assets. Custom assets include models, sounds, and textures. From the perspective of the game, a mod is a subdirectory in the game directory where the additional assets reside in the form of loose files or archives such as pak files. As far as TrenchBroom is concerned, a mod just provides assets, some of which replace existing assets from the game and some of which are new. For example, a mod might provide a new model for an entity, or it provides an entirely new entity. In order to make these new entities usable in TrenchBroom, two things are required: First, TrenchBroom needs an entity definition for these entities, and TrenchBroom needs to know where it should look for the models to display in the viewports. The first issue can be addressed by pointing TrenchBroom to an alternate [entity definition file](#entity_definitions), and the second issue can be addressed by [adding a mod directory](#mod_setup) for the current map.

Every game has a default mod which is always loaded by TrenchBroom. As an example, the default mod for Quake is _id1_, which is the directory that contains all game content. TrenchBroom supports multiple mods, and in the case of multiple mods, there has to be a policy for resolving name conflicts between resources. For example, a mod might provide an entity model for an entity defined in the default mod in order to replace the default model with a more detailed one. To do this, the mod provides an entity model with the same name as the one it wants to replace. To resolve such name conflicts, TrenchBroom assigns priorities for mods, and if a name conflict occurs between different mods, the mod with the highest priority wins. Note that the default mod always has the loest priority.

### Textures and Texture Collections {#textures}

A texture is an image that is projected onto a face of a brush. Textures are usually not provided individually, but as texture collections. A texture collection can be a directory containing loose image files, or it can be an archive such as a wad file. Some games such as Quake 2 come with textures that are readily loadable by TrenchBroom. Such textures are called _internal_. _External_ textures on the other hand are textures that you provide by loading a texture collection. Since some games such as Quake don't come with their own textures readily available, you have to obtain the textures you wish to use and add them to TrenchBroom manually by [loading a texture collection](#texture_management).

Multiple texture collections may contain a texture with the same name, resulting in a name conflict. Similar to how name conflicts are resolved with mods, TrenchBroom assigns priorities to texture collections and lets the texture collection with the highest priority win the conflict. Internal texture collections have a higher priority than external texture collections. The priority of internal or external texture collections then depends on the order in which they were loaded.

TrenchBroom currently does not support high resolution replacement textures, but this is on the Todo list.

## Startup {#startup}

The first thing you will see when TrenchBroom starts is the welcome window. This window allows you to open one of your most recently edited maps, to create a new map or to browse your computer for an existing map you wish to open in TrenchBroom.

![TrenchBroom's welcome window (Mac OS X)](WelcomeWindow.png)

You can click the button labeled "New map..." to create a new map or you can click the button labeled "Browse..." to find a map file on your computer. Double click one of the documents in the list on the right of the window to open it. The light gray text on the left gives you some information about which version of TrenchBroom you are currently running. The version information is useful if you wish to report a problem with the editor (see [here](#reporting_bugs) for more information).

If you choose to create a new map, TrenchBroom needs to know which game the map should be for, and will show a dialog in which you can select a game and, if applicable, a map format. This dialog may also be shown when you open an existing map. TrenchBroom will try to detect this information from the map file, but if that fails, you need to select the game and map format.

![The game selection dialog (Mac OS X)](GameSelectionDialog.png)

The list of supported games is shown on the right side of the dialog. Below the game list, there is a dropdown menu for choosing a map format; this is only shown if the game supports more than one map format. One example for this is Quake, which supports both the standard format and the Valve 220 format for map files. In the screenshot above, none of the games in the list were actually found on the hard disk. This is because the respective game paths have not been configured yet. TrenchBroom allows you to create maps for missing games, but you will not be able to see the entity models in the editor and other resources such as textures might be missing as well. To open the game configuration preferences, you can click the button labeled "Open preferences...". Click [here](#game_configuration) to find out how to configure the supported games.

Once you have selected a game and a map format, TrenchBroom will open the main editing window with a new map. The following section gives an overview of this window and its main elements. If you want to find out how to [work with mods](#mods) and how to [add textures](#texture_management), you can skip ahead to the respective sections.

## Main Window {#main_window}

The main window consists of a menu bar, a toolbar, the editing area, an inspector on the right and an info panel at the bottom. In the screenshot below, there are three editing area: one 3D viewport and two orthographic 2D editing area.

![The main editing window (Linux XFCE)](MainWindow.png)

The sizes of the editing area, the inspector and the info bar can be changed by dragging the dividers with the mouse. This applies to some of the dividers in the inspector as well. If a divider is 2 pixels thick, it can be dragged with the mouse. The positions of the dividers and the size of the editing window are saved when you close a window. The following subsections introduce the most important parts of the main window: the editing area, the inspector, and the info bar. The toolbar and the menu will be explained in more detail in later sections.

### The Editing Area

The editing area is divided in two sections: The context sensitive info bar at the top and the viewports below. The info bar contains different controls depending on which tool is currently activated. You can switch between tools such as the rotate tool and the vertex tool using the toolbar buttons, the menu or with the respective keyboard shortcuts. The context sensitive controls allow you to perform certain actions that are relevant to the current tool such as setting the rotation center when in the rotate tool or moving objects by a given delta when in the default move tool. Additonally, there is a button labeled "View" on the right of the info bar. Clicking on this button unfolds a dropdown containing controls to [filter out](#filtering_rendering_options) certain objects in the viewports or to change how the viewport [renders its contents](#filtering_rendering_options).

![The info bar with view dropdown (Windows 7)](ViewDropdown.png)

There are two types of viewports: 3D viewports and 2D viewports. TrenchBroom gives you some control over the layout of the viewports: You can have one, two, three, or four viewports. See section [View Layout and Rendering](#view_layout_and_rendering) to find out how to change the layout of the viewports. If you have fewer than four viewports, then one of the viewports can be cycled by hitting #action('Controls/Map view/Cycle map view'). Which of the viewports can be cycled and the order of cycling the viewports is given in the following table:

No. of Viewports    Cycling View         Cycling Order
----------------    ------------         -------------
1                   Single view          3D > XY > XZ > YZ
2                   Right view           XY > XZ > YZ
3                   Bottom right view    XZ > YZ
4                   None

There are three types of 2D viewports: the XY, the XZ, and the YZ viewport. You can also think of these viewports as the top, the front, and the side view, respectively. The following table summarizes the properties of the three 2D viewports. Remember that Quake-based engines use a right handed coordinate system.

Viewport    Right Axis    Up Axis    Normal Axis    Name
--------    ----------    -------    -----------    ----
XY          +X            +Y         +Z             Top
XZ          +X            +Z         -Y             Front
YZ          +Y            +Z         +X             Side

The normal axis is the axis that would be protruding from the screen when looking at the respective 2D viewport. In the case of the XY viewport, the normal axis is the positive Z axis, but in the case of the XZ viewport, the normal axis is the negative Y axis. For the mathematically inclined, the normal axis is the cross product of the right axis and the up axis. Sometimes, we will also refer to the inverted normal axis as the depth axis. So, the depth axis of the XY viewport is the negative Z axis. We also refer to the plane that is spanned by the first two axes as the view plane of a 2D viewport. Accordingly, the view plane of the XZ viewport is the X/Z plane. 

![The compass](Compass3D.png) In the bottom left of each viewport, there is a compass that indicates the orientation of the camera of that viewport. In the 3D viewport, you can see how the compass rotates when you rotate the camera. In the 2D viewport, the compass axes are fixed, but they indicate which of the coordinate system axes are the right and the up axis for that viewport. The colors of the compass hands represent the axes: Red is the X axis, green is the Y axis, and blue is the Z axis (RGB vs. XYZ). Sometimes the axes are rendered with a white outline to indicate an [axis restriction](#axis_restriction).

At most one of the viewports can have focus, that is, only one of them can receive mouse and keyboard events. Focus is indicated by a highlight rectangle at the border of the viewport. If no viewport is focused, you have to click on one of them to give it focus. Once a viewport has focus, the focus follows the mouse pointer, that is, to move focus from one viewport to another, simply move the mouse to the other viewport. The focused viewport can also be maximized by choosing #menu('Menu/View/Maximize Current View') from the menu. Hit the same keyboard shortcut again to restore the previous view layout.

### The Inspector

The inspector is located at the right of the main window and contains various controls, distributed to several pages, to change certain properties of the currently selected objects. You can show or hide the inspector by choosing #menu('Menu/View/Toggle Inspector'). To switch directly to a particular inspector page, choose #menu('Menu/View/Switch to Map Inspector') for the map inspector, #menu('Menu/View/Switch to Entity Inspector') for the entity inspector, and #menu('Menu/View/Switch to Face Inspector') for the face inspector.

![Map, Entity, and Face inspectors (Mac OS X)](Inspector.png)

The **Map Inspector** allows you to edit [layers](#layers) and to set up which game modifications ([mods](#mods)) you are working with. The **Entity Inspector** is the tool of choice to change the [properties](#entity_properties) of entities. It also contains an entity browser that allows you to [create new entities](#creating_entities) by dragging them from the browser to a viewport and it allows you to [set up entity definitions](#entity_definitions). Additionally, you can manage entity definition files in the entity inspector. The face inspector is used to edit the attributes of the currently selected faces. At the top, it has a graphical [UV editor](#uv_editor). Below that, you can edit the face attributes directly by editing their values. To select a texture for the currently selected faces, you can use the [texture browser](#texture_browser). Finally, the face inspector allows you to [manage your texture collections](#texture_management).

### The Info Bar

You can show or hide the info bar by choosing #menu('Menu/View/Toggle Info Panel'). It contains the console where TrenchBroom prints out messages, warnings and errors, and the live [issue browser](#issue_browser) where you can view, filter and resolve issues with your map.

## Camera Navigation {#camera_navigation}

Navigation in TrenchBroom is quite simple and straightforward. You will mostly use the mouse to move and pan the camera and to look around. There are also some keyboard shortcuts to help you position the camera with more precision. Just like in Quake, the camera cannot be rolled - in other words, up is always in the positive Z direction and down is always in the negative Z direction. The behavior of the camera can be controlled in the [preferences](#mouse_input).

### Looking and Moving Around

In the 3D viewport, can look around by holding the right mouse button and dragging the mouse around. There are several ways to move the camera around. First and foremost, you can move the camera forward and backward in the viewing direction by spinning the scroll wheel. If you prefer to have the scroll wheel move the camera in the direction where the mouse cursor is pointing, you can check the "Move camera towards cursor" option in the preferences. To move the camera sideways and up / down, hold the middle mouse button and drag the mouse in any direction. For tablet users, there is an option in the preferences that will enable you to move the camera horizontally by holding #key(307). Additionally, you can use the [fly mode](#fly_mode) keyboard shortcuts even when fly mode is disabled.

### Orbiting

The camera orbit mode allows you to rotate the camera about a selectable point. To get an idea as to what this means, imagine that you define a point in the map by clicking on a brush. The point where you clicked will be the center of your camera orbit. Now image a sphere whose center is the point where you just clicked and whose radius is the distance between the camera and the point. Orbiting will move the camera on the surface of that sphere while adjusting the camera's direction so that you keep looking at the same point. Visually, this is the same as rotating the entire map about the orbit center. Of course, you are not actually rotating anything - only the camera's position and direction are modified. Note that, since up and down are always fixed, you cannot cross the north and south poles of the orbit sphere.

Camera orbit mode is very useful if you are editing a brush because it allows you to view this brush from all sides quickly. Its best to try it and see for yourself how useful it is. To invoke the orbit mode, click and drag with the right mouse button while holding #key(307). The orbit center is the point in the map which you initially clicked. Dragging sideways will orbit the camera horizontally and dragging up and down will orbit the camera vertically. You can change the orbit radius during the orbit with the scroll wheel.

### Fly Mode

Fly mode allows you to move around freely in a map, much like "no clipping" modes in some games do. When fly mode is active, you cannot perform any editing and your mouse cursor is hidden until fly mode is deactivated again. Hit #action('Controls/Map view/Toggle fly mode') to toggle fly mode. The following table lists the keyboard shortcuts you can use to move the camera around when in fly mode.

Direction    Key
---------    ---
Forward      #action('Controls/Camera/Move forward')
Backward     #action('Controls/Camera/Move backward')
Left         #action('Controls/Camera/Move left')
Right        #action('Controls/Camera/Move right')

### Automatic Navigation

You can center the camera on the current selection by choosing #menu('Menu/View/Camera/Center on Selection') from the menu. This will position the camera so that all selected objects become visible. The camera will not be rotated at all; only its position will be changed. Note that this action will also adjust the 2D viewports so that the selection becomes visible there as well.

Finally, you can move the camera to a particular position. To do this, choose #menu('Menu/View/Camera/Move Camera to...') and enter the position into the dialog that pops up. This does not affect the 2D views.

## Navigating the 2D Viewports

![Linked YZ and XZ viewports](Linked2DViewports.gif)

Navigating the 2D viewports is naturally a lot simpler than navigating the 3D viewport. You can pan the viewport by holding and dragging either the middle or the right mouse button, and you can use the scroll wheel to adjust the zoom. Note that if you have set your viewport layout to show more than one 2D viewport, then the 2D viewportss are linked. Specifically, the zoom factor is linked so that you always have a consistent zoom level across all 2D viewports, and the viewports are panned simultaneously along the same axes. This means that if you pan the XY viewport along the X axis, the XZ viewport gets panned along the X axis, too, so that you don't have to pan both viewports manually. Also note that zooming always keeps the coordinates under the mouse pointer invariant, that is, you can focus on a particular object or area by hovering the mouse over it and zooming in or out.

# Selection {#selection}

There are two different kinds of things that can be selected in TrenchBroom: objects and brush faces. Most of the time, you will select objects such as entities and brushes. Selecting individual brush faces is only useful for changing their attributes, e.g. the textures. You can only select objects or brush faces, but not both at the same time. However, a selection of brushes (and nothing else) is treated as if all of their brush faces were selected individually.

![Object selection in 3D and 2D viewports](ObjectSelection.gif)

In the 3D viewport, selected objects are rendered with red edges and slightly tinted faces to distinguish them visually. The bounding box of the selected objects is rendered in red, with spikes extending from every corner when the mouse hovers over one of the selected objects. These spikes are useful to precisely position objects in relation to other objects. Additionally, the dimensions of the bounding box are displayed in the 3D viewport. In a 2D viewport, selected objects are just rendered with red edges. No spikes or bounding boxes are displayed there since the 2D viewports have a continuous grid anyway.

## Selecting Objects

To select a single object, simply left click it in a viewport. Left clicking anywhere in a viewport deselects all other selected objects, so if you wish to select multiple objects, hold #key(308) while left clicking the objects to be selected. If you click an already selected object while holding #key(308), it will be deselected. You can also _paint select_ multiple objects. First, select an object, then hold #key(308) while dragging over unselected objects. Take care not to begin your dragging over a selected object, as this will [duplicate the selected objects](#duplicating_objects).

In the 3D viewport, you can only select the frontmost object with the mouse. To select an object that is obstructed by another object, you can use the mouse wheel. First, select the frontmost object, that is, the object that occludes the object that you really wish to select. Then, hold #key(308) and scroll up to push the selection away from the camera or scroll down to pull the selection towards the camera. Note that the selection depends on what object is under the mouse, so you need to make sure that the mouse cursor hovers over the object that you wish to select.

![Selection drilling in the 3D viewport](DrillSelection.gif)

In a 2D viewport, you can also left click an object to select it. But unlike in the 3D viewport, this will not necessarily select the frontmost object. Instead, TrenchBroom will analyze the objects under the mouse, specifically the faces under the mouse, and it will find the one with the smallest visible area. Having found such a face, it will select the object to which this face belongs. Since entities don't necessarily have faces, the faces of their bounding boxes will be considered instead. The advantage of this technique is that it allows you to easily select occluded objects in the 2D viewports.

You may also think of left click selection like this: In both the 3D viewport or a 2D viewport, TrenchBroom first compiles a set of candidate objects. These are all objects under the mouse. Then, it must choose an object to be selected from these candidates. In the 3D viewport, the frontmost object always wins (unless you're using the scroll wheel to drill the selection), and in a 2D view, the object with the smallest visible area wins. Other than that, selection behaves exactly the same in both viewports, that is, you can hold #key(308) to select multiple objects and so on.

Sometimes, selecting objects manually is too tedious. To select all currently editable objects, you can choose #menu('Menu/Edit/Select All') from the menu. Note that hidden and locked objects are excluded, so this command is particularly useful in conjunction with those features. Another option to select multiple objects at once is to use _selection brushes_. Just create one or more new brushes that enclose or touch all the objects you wish to select. These brushes are called a selection brushes. Select all of these newly created selection brushes, and choose #menu('Menu/Edit/Select Touching') to select every object touched by the selection brushes, or choose #menu('Menu/Edit/Select Inside') to select every object enclosed inside them. 

![Using a selection brush](SelectTouching.gif)

A similar operation can be found under #menu('Menu/Edit/Select Tall'), but this particular operation is only available when a 2D view has focus. It projects the selection brushes onto the view plane of the currently focused 2D viewport, which results in a 2D polygon, and then selects every object that is contained entirely within that polygon. You can think of this as a cheap lasso selection tool that works like #menu('Menu/Edit/Select Inside'), but without having to adjust the distance and thickness of the selection brushes.

If you have selected a single brush that belongs to an entity or group, and you wish to select every other object belonging to that entity or group, you can choose #menu('Menu/Edit/Select Siblings'). The same effect can be achieved by left double clicking on a brush that belongs to an entity or group. The menu command #menu('Menu/Edit/Select by Line Number') is useful for diagnostic purposes. If an external program such as a map compiler presents you with an error message and a line number indicating where in the map file that error occured, you can use this menu command to have TrenchBroom select the offending object for you.

Finally, you can deselect everything by left clicking in the void, or by choosing #menu('Menu/Edit/Select None').

## Selecting Brush Faces

![Selected brush face](BrushFaceSelection.png)

To select a brush face, you need to hold #key(306) and left click it in the 3D viewport. You can select multiple brush faces by additionally holding #key(308). To select all faces of a brush, you can left double click that brush while holding #key(306). If you additionally hold #key(308), the faces are added to the current selection. To paint select brush faces, first select one brush face, then left drag while holding #key(308) and #key(306). To deselect all brush faces, simply click in the void or choose #menu('Menu/Edit/Select None').

# Editing

In this section, we will cover all topics related to the actual editing of a map. We begin by explaining how to set up the map itself, that is, how to set up mods, entity definitions, and texture collection. Afterwards, we show how to create new objects such as entities or brushes, how to edit and transform them, and how to delete them. After that, we explain how you can work with textures in TrenchBroom. The following section introduces the various tools at your disposal to shape brushes while the section after that focuses on entities and how to edit their properties. The goal of the final section is to help you keep an overview in your map by using layers, groups, and by various other means.

## Map Setup

The first step when creating a new map is the setup of mods, entity definitions, and texture collections.

### Setting Up Mods {#mod_setup}

![Mod editor](ModEditor.png) We explained [previously](#mods) that a mod is just a sub directory in the game directory itself. Every game has a default mod that is always active and that cannot be deactivated - in Quake, this is _id1_, the directory that contains all game resources. TrenchBroom supports an unlimited number of additional mods. Mods can be added, removed, and reordered by using the mod editor, which you can find at the bottom of the map inspector.

When the editor starts, the mod editor is folded, but you can unfold it by clicking on its title bar. You are then presented with a two column view of the available mods (left column) and the enabled mods (right column). The list of available mods contains every sub directory in the game directory in alphabetic order. This list can be filtered using the search field at the bottom. To enable some mods, select them in the list of available mods and click on the plus icon below the list of enabled mods. To disable mods, select them in the list of enabled mods and click on the minus icon.

The order of the enabled mods is important because it defines the [priorities for loading resources](#mods). You can change the priority of enabled mods by reordering them in the list. For this, you can select an enabled mod and use the small triangles below the list of enabled mods to push it up or down.

Mods are stored in a worldspawn property called "_tb_mod".

### Loading Entity Definitions {#entity_definition_setup}

![Entity definition editor](EntityDefinitionEditor.png) Entity definitions are text files containing information about [the meaning of entities and their properties](#entitiy_definitions). Depending on the game and mod you are mapping for, you might want to load different entity definitions into the editor. To load an entity definition file into TrenchBroom, switch to the entity inspector and unfold the entity definition browser at the bottom of the inspector. The entity definition browser is horizontally divided into two areas. The upper area contains a list of _builtin_ entity definition files. These are the entity definition files that came with TrenchBroom for the game you are currently working on. You can select one of these builtin files by clicking on it. TrenchBroom will load the file and update its resources accordingly. Alternatively, you may want to load an external entity definition file of your own. To do this, click the button labeled "Browse" in the lower area of the entity defiinition browser and choose the file you wish to load. Alternatively, you can just drag and drop an entity definition file onto TrenchBroom's Main Window from a file manager (such as Windows Explorer). Currently, TrenchBroom supports Radiant DEF files and [Valve FGD][FGD File Format] files. To reload the entity definitions from the currently loaded external file, you can click the button labeled "Reload" at the bottom. This may be useful if you're editing an entity definition file for a mod you're working on.

Note that FGD files contain much more information than DEF files and are generally preferrable. While TrenchBroom supports both file types, its support for FGD is better and more comprehensive. Unfortunately, DEF files are still relevant because Radiant style editors require them, so TrenchBroom allows you to use them, too.

The path of an external entity definition file is stored in a worldspawn property called "_tb_def".

### Managing Textures {#texture_management}

![Texture collection editor](TextureCollectionEditor.png) [Texture collections](#textures) can be managed in the texture collection editor, which can be found at the bottom of the face inspector. Unfolding the texture collection editor presents you with a list of the currently loaded texture collections and a few buttons to manage that list. To load a texture collection, click the "+" button below the list, and to remove the selected texture collections, click the "-" button. Alternatively, you can load a texture collection by dragging and dropping it onto TrenchBroom's Main Window from a file manager (such as Windows Explorer). The order in which the texture collections are loaded determines their respective priorities when TrenchBroom resolves name conflicts, so you can change the loading order by selecting a texture collection and clicking the triangle buttons. The lower a texture collection appears in the list, the higher is its priority. An easy way to think about this is to imagine that textures overwrite each other: If a texture is loaded that has a name conflict with an already loaded texture, then the newly loaded texture overwrites the previously loaded one.

It depends on the game how the texture collection paths are saved in the map file. For Quake and its direct descendants such as Hexen 2, the texture collection paths are stored in a worldspawn property called "wad", as that is what the BSP compilers expect. For other games, they are stored in a worldspawn property called "_tb_textures".

## Interacting With the Editor

Before we delve into specific editing operations such as creating new objects, you should learn some basics about how to interact with the editor itself. In particular, it is import to understand the concept of tools in TrenchBroom and how mouse input is mapped to 3D coordinates.

### Working with Tools

All editing functionality in TrenchBroom is provided by tools. There are two types of tools in TrenchBroom: Permanently active tools and modal tools. Modal tools are tools which have to be activated or deactivated manually by the user. Permanently active tools are tools which are always available, unless they are deactivated by a modal tool. The following table lists all tools with a short description:

Tool                  Viewports    Type          Purpose
----                  ---------    ----          -----------
Camera Tool           2D, 3D       Permanent     Adjusting the 3D camera and the 2D viewports
Selection Tool        2D, 3D       Permanent     Selecting objects and brush faces
Simple Brush Tool     2D, 3D       Permanent*    Creating simple cuboid brushes
Complex Brush Tool    3D           Modal         Creating arbitrarily shaped brushes
Entity Drag Tool      2D, 3D       Permanent     Creating entities by drag and drop
Resize Tool           2D, 3D       Permanent*    Resizing brushes by dragging faces
Move Tool             2D, 3D       Permanent*    Moving objects around
Rotate Tool           2D, 3D       Modal         Rotating objects
Clip Tool             2D, 3D       Modal         Clipping brushes
Vertex Tool           2D, 3D       Modal         Editing brush vertices, edges, and faces

Tools of the type Permanent* are deactivated whenever a modal tool is active. For example you cannot create cuboid brushes when the vertex tool is active. Additionally, at most one modal tool can be active at a time. You can activate and deactivate modal tools using the menu and keyboard shortcuts listed in the following table:

Tool                  Menu
----                  -----------
Complex Brush Tool    #menu('Menu/Edit/Tools/Brush Tool')
Rotate Tool           #menu('Menu/Edit/Tools/Rotate Tool')
Clip Tool             #menu('Menu/Edit/Tools/Clip Tool')
Vertex Tool           #menu('Menu/Edit/Tools/Vertex Tool')

![Tool buttons](ToolbarTools.png) Additionally, tools can be toggled by using the buttons on the left of the toolbar. In the image, the first button is active, however, this particular button does not represent any of the modal tools listed in the table above. Rather, it indicates that no modal tool is currently active, and therefore all permanent tools are available. The buttons icon indicates that objects can be moved, which is only possible if no modal tool is active. The second button represents the convex brush tool, the third button toggles the clip tool, the fourth button is used to toggle the vertex tool and the fifth button toggles the rotate tool.

You can learn more about these tools in later sections. But before you can learn about the tools in detail, you should undertand how TrenchBroom processes mouse input, which is what the following two sections will explain.

### Cancelling Operations and Tools {#cancelling}

To cancel a mouse drag by hitting #action('Controls/Map view/Cancel'). The operation will be undone immediately. The same keyboard shortcut can be used to cancel all kinds of things in the editor. The following table lists the effects of cancelling depending on the current state of the editor.

State                 Effect
-----                 ------
Complex Brush Tool    Discard all placed points; deactivate tool
Clip Tool             Discard most recently placed clip point; deactivate tool
Vertex Tool           Discard current vertex selection; deactivate tool
Selection Tool        Discard current selection

For those tools where a second effect is listed (separated by a semicolon), the second effect only takes place if the first effect couldn't be realized. For example, if the clip tool is active but no clip points have been placed, then hitting #action('Controls/Map view/Cancel') will deactivate the clip tool. Hitting #action('Controls/Map view/Cancel') yet again will deselect all selected objects or brush faces.

In addition, you can hit #action('Controls/Map view/Deactivate current tool') to directly deactivate the current tool regardless of what state the tool is in.

### Mouse Input in 3D

It is very important that you understand how mouse input is mapped to 3D coordinates when editing objects in TrenchBroom's 3D viewport. Since the mouse is a 2D input device, you cannot directly control all three dimensions when you edit objects with the mouse. For example, if you want to move a brush around, you can only move it in two directions by dragging it. Because of this, TrenchBroom maps mouse input to the horizontal XY plane. This means that you can only move things around horizontally by default. To move an object vertically, you need to hold #key(307) during editing. This applies to moving objects and vertices, for the most part.

But this is not always true, since some editing operations are spacially restricted. For example, when resizing a brush, you drag one of its faces along its normal, so the editing operation is restricted to that normal vector. In fact, the mouse pointer's position must be mapped to a one-dimensional value that represents the distance by which the brush face has been dragged. Whenever mouse input has to be mapped to one or two dimensions, TrenchBroom does this mapping automatically and no additional thought is required. But if mouse input must be mapped to three dimensions, TrenchBroom does so by employing the editing plane metaphor explained before.

### Mouse Input in 2D

Mapping mouse input to 3D coordinates is much simpler in the 2D viewports, because the first and second dimension is given by the fixed viewport axes, and the third dimension (the depth) is usually taken from the context of the editing operation. For example, if you move an object by left dragging it in the XY viewport, then the mouse input is mapped to the X and Y axes, and the object's Z coordinates remain unchanged. When creating new objects, the depth is usually computed from the bounds of the most recently selected objects. So if you create a new brush by left dragging in the XY view, its distance and its height is determined by the most recently selected objects, while its X/Y extents are determined by the mouse drag.

### Axis Restriction {#axis_restriction}

To avoid imprecise movements when moving objects in two dimensions, you can limit movement to a single axis. The following table lists the respective shortcuts and their effects:

Shortcut                                             Effect
--------                                             ------
#action('Controls/Map view/Set movement axis X')     Restrict movement to X axis
#action('Controls/Map view/Set movement axis Y')     Restrict movement to Y axis
#action('Controls/Map view/Set movement axis Z')     Restrict movement to Z axis
#action('Controls/Map view/Toggle movement axis')    Cycle through movement axis: X, Y, Z, none

![Axis restriction](AxisRestriction.png) Note that these restrictions apply to all viewports. So it might happen that you restrict movement to the Z axis in the XZ view and then try to move an object in the 3D viewport, only to find that your mouse dragging has no effect on the object because by default, movements are restricted to the XY plane in the 3D viewport. The compass at the bottom left of each view indicates the current axis restriction by drawing a white outline around the restricted axis. In the image to the left, movement is restricted to the X axis, so the red axis is drawn with a white outline.

### The Grid

TrenchBroom provides you with a static grid to align your objects to each other. The grid size can be 1, 2, 4, 8, 16, etc. up to 256. If grid snapping is enabled, then most editing operations will be snapped to the grid. For example, you can only move objects by the current grid size if grid snapping is enabled. In the 3D viewport, the grid is projected onto the brush faces. Therefore the grid may appear distorted if a brush face is not axis aligned. In the 2D viewports, the grid is just drawn in the background. You can change the brightness of the grid lines in the preferences.

The grid size can be set via the menu, or by scrolling the mouse wheel while holding both #key(307) and #key(308).

## Creating Objects

TrenchBroom gives you various options on how to create new objects. In the following sections, we will introduce these options one by one.

### Creating Simple Brushes

The easiest way to create a new brush is to just draw it with the mouse. To draw a brush, make sure nothing is currently selected and left drag in the 3D viewport or any of the 2D viewports. If you draw a brush in the 3D viewport, its shape is controlled by the point where you initially started your drag, the point currently under the mouse, and the current grid size. Essentially, you will draw your brush on the XY plane, and its height will be set to the current grid size. But it is possible to change the height while drawing a brush by holding #key(307). Whenever #key(307) is held, you can change the brush height, otherwise you control the X/Y extents of the brush.

![Creating a cuboid in the 3D viewport](CreateSimpleBrush.gif)

If, on the other hand, you draw a brush in a 2D viewport, you only control its extents on whatever axes the 2D view is set to display. So if you are drawing a brush in the XZ view, you control the X/Z extents with the mouse, and there's no way to change the Y extents directly, which is always fixed to the Y extents of the most recently selected objects. This of course applies to all of the different 2D viewports in the same way.

In either case, the texture assigned to the newly created brush is the _current texture_. The current nexture is set by chooseing a texture in the [texture browser](#texture_browser) or by selecting a face that already has a texture. This concept applies to other ways of creating new brushes, too.

This way of creating brushes only allows you to create cuboids. In the next section, you will learn how to create more complex brush shapes with the complex brush tool.

### Creating Complex Brushes

![Drawing a rectangle and duplicating it](CreateBrushByDuplicatingPolygon.gif) If you want to create a brush that is not a simple, axis-aligned cuboid, you can use the brush tool. The brush tool allows you to define a set of points and create the convex hull of these points. A convex hull is the smallest convex volume that contains all the points. The points become the vertices of the new brush, unless they are placed within the brush, in which case they are discarded. Accordingly, the brush tool gives you several ways to place points, but there are two limitations: First, you can only place points in the 3D viewport, and second, you can only place points by using other brushes as reference.

To use the brush tool, you first have to activate it by choosing #menu('Menu/Edit/Tools/Brush Tool'). Then, you can place single points onto the grid by left clicking on the faces of other brushes. Additionally, you can left double click on a face to place points on all of its vertices. You can also draw a rectangular shape by left dragging on an existing brush face and thereby place four points at the corners of that rectangle. Finally, if the points you have placed so far form a polygon, you can duplicate and move that polygon along its normal by left dragging it while holding #key(306). Once you have placed all points, hit #action('Controls/Map view/Create brush') to actually create the brush.

It is not possible to modify or remove points after they have been placed, except discarding all of them by hitting the #action('Controls/Map view/Cancel') key.

### Creating Entities {#creating_entities}

There are two types of entities: point entities and brush entities, and it depends on the type how an entity is created. In the following sections, we present two ways of creating point entities and one way to create brush entities.

#### Point Entities

There are two ways of creating new point entities. Firstly, you can drop new entities in the 3D and 2D viewports by using the context menu. To open the context menu, right click into the viewport. To create a point entity such as a pickup weapon or a monster, open the "Create Point Entity" sub menu and select the correct entity definition from the sub menus.

![Dropping an entity with the context menu (Mac OS X)](CreateEntityContextMenu.png)

The location of the newly created entity depends on whether you clicked on the 3D viewport or a 2D viewport. If you clicked on the 3D viewport, then the entity will be placed on the brush under the mouse. If there was no brush under the mouse, then the entity will be placed at a default distance. Note that the bounding box of the entity will be snapped to the grid. If you clicked on the 2D view, then the position of the entity depends on what was under the mouse when you clicked, too. If a selected brush was under the mouse, then the new entity will be placed on that brush. If no selected brush was under the mouse, then the entity will be placed at the far end of the bounding box of the most recently selected objects. Again, the bounding box of the newly created entity will be snapped to the grid.

![Entity browser](EntityBrowser.png) Secondly, you can create new point entities by dragging them from the entity browser. The entity browser can be found in the entity inspector page. At the bottom of the entity browser, you can find a number of controls to change the sort order and to filter the entities displayed in the browser.

The leftmost dropdown list allows you to change the sort order. Entities can be sorted by name or by their usage count, with the most used entities at the top. The "Group" button toggles grouping the entities by their category, which is derived from the first part of the entity name. For example, all entities starting with "key_" are put into a category called "key". The button labeled "Used" toggles all unused entities, when it is pressed, only those entities which are used in the map are shown in the browser. To filter entities by name, enter some text in the search box on the right. Only entities containing the search text will be shown in the browser.

To create a new entity, simply drag it out of the browser and onto the 3D or a 2D viewport. If you drag it onto the 3D viewport, the entity will be positioned on the brush under the mouse, with its bounding box snapped to the grid. If you drag the entity onto a 2D viewport, its position is determined by the far end of the most recently selected object.

#### Brush Entities

![Moving brushes to brush entities](MoveBrushesToEntity.png) Creating brush entities is also done using the context menu. Select a couple of brushes and right click on them, then select the desired brush entity from the menu. To move brushes from one brush entity to another, select the brushes you wish to move and right click on a brush belonging to the brush entity to which you want to move the brushes, and select "Move brushes to Entity", where "Entity" is the name of the target brush entity, for example "func_button" in the picture on the left. If the brush entity containing the brushes to be moved becomes empty, it will be automatically deleted. To move brushes from a brush entity back into the world, select the brushes and right click on a world brush or on the void and select "Move brushes to worldspawn".

Often, it is much quicker to create new objects by duplicating existing ones. Objects can be duplicated using dedicated functions in TrenchBroom, or just by copying and pasting them.

### Duplicating Objects {#duplicating_objects}

The currently selected objects can be duplicated by choosing #menu('Menu/Edit/Duplicate'). This will duplicate the objects in place, that is, the duplicates retain the exact position of the original objects. To give visual feedback, the duplicated objects are flashed in white really quickly. In the following short clip, you can see that the selected brush gets duplicated. After that, the duplicated brush is moved upwards.

![Duplicating a brush in place](DuplicateInPlace.gif)

Very often, you will want to duplicate objects and move them to a different position immediately afterwards, because having duplicates retain the same position as their originals is very seldomly useful. That's why you can also duplicate and move objects at once without having to perform two separate actions. To duplicate and move objects, you can use the following keyboard shortcuts:

Direction     Shortcut (2D)                                                                                        Shortcut (3D)
---------     -------------                                                                                        -------------
Left          #action('Controls/Map view/Duplicate and move objects left')                                         #action('Controls/Map view/Duplicate and move objects left')
Right         #action('Controls/Map view/Duplicate and move objects right')                                        #action('Controls/Map view/Duplicate and move objects right')
Up            #action('Controls/Map view/Duplicate and move objects up; Duplicate and move objects forward')       #action('Controls/Map view/Duplicate and move objects forward; Duplicate and move objects up')
Down          #action('Controls/Map view/Duplicate and move objects down; Duplicate and move objects backward')    #action('Controls/Map view/Duplicate and move objects backward; Duplicate and move objects down')
Forward       #action('Controls/Map view/Duplicate and move objects forward; Duplicate and move objects up')       #action('Controls/Map view/Duplicate and move objects up; Duplicate and move objects forward')
Backward      #action('Controls/Map view/Duplicate and move objects backward; Duplicate and move objects down')    #action('Controls/Map view/Duplicate and move objects down; Duplicate and move objects backward')

Essentially, these are the same keyboard shortcuts that you use to [move objects around](#moving_objects) in the 3D and 2D viewports, but while holding #key(308). In the same vein, you can hold #key(308) while left dragging a selected object to duplicate and move all selected objects.

![Duplicating and moving a brush](DuplicateAndMove.gif)

Note that in the image above, the selected brush flashes while it is moved to the right. This indicates that in this case, the duplication and the translation happened at the same time instead of one after the other as in the previous example.

### Copy and Paste

You can copy objects by selecting them and choosing #menu('Menu/Edit/Copy'). TrenchBroom will create text representations of the selected objects as if they were saved to a map, and put that text representation on the clipboard. This allows you to paste them into map files, and also to directly copy objects from map files and paste them into TrenchBroom. Note that you can also copy brush faces, which will also put a text representation of that brush face on the clipboard. Having copied a brush face, you can paste the attributes of that face (texture, offset, scale, etc.) into other selected brush faces.

There are two menu commands to paste objects from the clipboard into the map. The simpler of the two is #menu('Menu/Edit/Paste at Original Position'), which will simply paste the objects from the clipboard without changing their position. The other command, available at #menu('Menu/Edit/Paste'), does not paste the objects from the clipboard at their original positions, but will try to position them using the current mouse position. If pasted into the 3D viewport, the pasted objects will be placed on top of the brush under the mouse. If no brush is under the mouse, the objects will be placed at a default distance. The bounding box of the pasted objects is snapped to the grid, and TrenchBroom will attempt to keep the center of the bounding box of the pasted objects near the mouse cursor. The following clip illustrates these concepts. The light fixture is copied, then pasted several times.

![Pasting objects in the 3D viewport](PastePositioning3D.gif)

Positioning of objects pasted into a 2D viewport attempts to achieve a similar effect by positiong the pasted objects such that they line up with the far end of the bounds of the most recently selected objects while keeping them under the mouse, with their center snapped to the grid.

## Editing Objects

The following section is divided into several sub sections: First, we introduce editing operations that can be applied to all objects, such as moving, rotating, or deleting them. Then we proceed with the tools to shape brushes, such as the clip tool, the vertex tool, and the CSG operations. Afterwards we explain how you work with textures in TrenchBroom, and then we move on to editing entities and their properties. The final subsection deals with TrenchBroom's undo and redo capabilities.

### Moving Objects {#moving_objects}

You can move objects around by using either the mouse or keyboard shortcuts. Left click and drag on a selected object to move it (and all other selected objects) around. In the 3D viewport, the objects are moved on the XY plane by default. Hold #key(307) to move the objects vertically along the Z axis. In a 2D viewport, the objects are moved on the viewport's view plane. There is no way to change an object's distance from the camera using the mouse in a 2D viewport. If grid snapping is enabled, the distances by which you move them are snapped to the grid component-wise, that is, if the grid is set to 16 units, you can move objects by 16 units in either direction.

![Moving a brush in the 3D viewport with trace line](MoveTrace.png)

Note that TrenchBroom draws a light blue trace line for you when you move objects with the mouse. The trace line helps to move objects in straight lines and as a visual feedback for your move. Remember that you can [cancel moving objects](#cancelling) by hitting #action('Controls/Map view/Cancel'), and that you can [restrict the movement axis](#axis_restriction) when moving objects with the mouse.

You can also use the keyboard to move objects. Every time you hit one of the shortcuts in the following table, the object will move in the appropriate direction by the current grid size. Also remember that you can [duplicate objects and move them](#duplicating_objects) in the given direction in one operation by holding #key(308) and hitting one of the keyboard shortcuts listed below.

Direction     Shortcut (2D)                                                            Shortcut (3D)
---------     -------------                                                            -------------
Left          #action('Controls/Map view/Move objects left')                           #action('Controls/Map view/Move objects left')
Right         #action('Controls/Map view/Move objects right')                          #action('Controls/Map view/Move objects right')
Up            #action('Controls/Map view/Move objects up; Move objects forward')       #action('Controls/Map view/Move objects forward; Move objects up')
Down          #action('Controls/Map view/Move objects down; Move objects backward')    #action('Controls/Map view/Move objects backward; Move objects down')
Forward       #action('Controls/Map view/Move objects forward; Move objects up')       #action('Controls/Map view/Move objects up; Move objects forward')
Backward      #action('Controls/Map view/Move objects backward; Move objects down')    #action('Controls/Map view/Move objects down; Move objects backward')

Note that the meaning of the keyboard shortcuts depends on the viewport in which you use them. While #action('Controls/Map view/Move objects up; Move objects forward') moves the selected objects in the direction of the up axis if used in a 2D viewport, it moves the objects (roughly) in the direction of the camera viewing direction (i.e. forward) on the editing plane if used in the 3D viewport. Likewise, #action('Controls/Map view/Move objects forward; Move objects up') moves the selected objects in the direction of the normal axis (i.e. forward) if used in a 2D viewport and in the direction of the Z axis if used in the 3D viewport.

![Additional controls for move tool](MoveObjectsToolPage.png)

The move tool adds a few controls to the info bar above the viewports: A text box and a button labeled "Apply". You can enter a vector into the text box (three space separated numbers), and if you click the button, the currently selected objects will be moved by that vector.

### Rotating Objects {#rotating_objects}

The easiest way to rotate objects in TrenchBroom is to use the following keyboard shortcuts:

Shortcut                                                        Type     Rotation (3D)                         Rotation (2D)
--------                                                        ----     -------------                         -------------
#action('Controls/Map view/Roll objects clockwise')             Roll     Clockwise about view axis             Clockwise about normal axis
#action('Controls/Map view/Roll objects counter-clockwise')     Roll     Counter-clockwise about view axis     Counter-clockwise about normal axis
#action('Controls/Map view/Pitch objects clockwise')            Pitch    Clockwise about right axis            Clockwise about right axis
#action('Controls/Map view/Pitch objects counter-clockwise')    Pitch    Counter-Clockwise about right axis    Counter-Clockwise about right axis
#action('Controls/Map view/Yaw objects clockwise')              Yaw      Clockwise about Z axis                Clockwise about up axis
#action('Controls/Map view/Yaw objects counter-clockwise')      Yaw      Counter-clockwise about Z axis        Counter-clockwise about up axis

If the rotate tool is active, these keyboard shortcuts rotate the selected objects using the center of rotation and the angle set using the tool's rotation handle and the input controls above the viewports. If the rotate tool is not active, the center of rotation is the center of the bounding box of the currently selected objects (snapped to the grid), and the rotation angle is fixed to 90°.

![Rotation handle](RotateHandle.png) The rotate tool gives you more control over rotation than the keyboard shortcuts do. Hit #menu('Menu/Edit/Tools/Rotate Tool') to activate the rotate tool and a rotation handle will appear in the viewports. The rotation handle allows you to set the center of rotation and to perform the actual rotation of the selected objects about the X, Y, or Z axis. In the 3D viewport, you can rotate the objects about any of those axes by left dragging the appropriate part of the rotate handle, but in a 2D viewport, you can only rotate the objects about the normal axis of that viewport.

In the 3D viewport, the rotation handle will appear as in the image on the left. It has three axes, color coded with the X axis in red, the Y axis in green, and the Z axis in blue as usual. In addition to the axes, it has three quarter circles, again color coded, and four small spherical handles. The center handle (the yellow sphere) changes the center of rotation if you drag it with the left mouse button. Moving the center of rotation works exactly as [moving objects with the move tool](#moving_objects) does. If you hover the mouse over the center handle, you will notice that the coordinates of the center of rotation are displayed above the center handle and that the handle is highlighted by a red outline. To perform a rotation, you have to use one of the three other spherical handles. Clicking and dragging the blue spherical handle with the left mouse button rotates the objects about the Z axis, and likewise for the red and green handles (see the clip below).

In the 2D viewport, the rotation handle will just appear as a circle with two circular handles. The center handle allows you to move the center of rotation on the view plane of that viewport, and the handle on the outer circle allows you to perform the rotation. In the 2D viewports, the handle is also color coded, the colors of the outer circle reflecting the axis of rotation in a similar fashion to the 3D rotate handle.

![Rotate tool controls](RotateToolControls.png)

Like the move tool, the rotate tool places some controls above the viewport. On the very left, there is a text field that displays the coordinates of the center of rotation. This text field automatically updates if you move the rotate handle around in the 2D or 3D viewports. If you want to set the center of rotation manually, you can enter three coordinates here and hit #key(13). Alternatively, you can click the button labeled "Reset" to set the center of rotation to the center of the bounding box of the currently selected objects, snapped to the grid. The rest of the controls allow you to perform a rotation by entering an angle in the text box, selecting the rotation axis from the dropdown list, and clicking the "Apply" button.

![Rotating objects about the Z axis in the 3D viewport](RotateTool.gif)

If you look closely at the clip above, you will notice that the entity in the picture, a green armor, rotates nicely with the brush it is placed on. Firstly, its position does not seem to change in relation to the brush, and secondly, its angle of rotation is also changed according to the rotation being performed by the user. Whether and how TrenchBroom can adapt the angle of rotation of an entity depends on the following rules. 

- First, TrenchBroom looks at the entity's classname and its properties to determine its rotation type.
	- If the entity does not have a classname, then its rotation remains unchanged.
	- If the classname starts with "light", then TrenchBroom checks its properties.
		- If it has a property named "mangle", then the value of the property consists of three separate angles.
		- If it does not have a target property, and
			- if it has a property called "angles", then the value of the property consists of three separate angles.
			- otherwise TrenchBroom assumes it has a property called "angle", which is a single value that indicates the rotation angle about the Z axis.
	- If the classname does not start with "light", and
		- if the entity is not a point entity, and
			- if it has a property called "angles", then the value of the property consists of three separate angles.
			- otherwise TrenchBroom assumes it has a property called "angle", which is a single value that indicates the rotation angle about the Z axis.
		- if the entity is a point entity, and if the origin of the entity's bounding box is at the center, and
			- if it has a property called "angles", then the value of the property consists of three separate angles.
			- otherwise TrenchBroom assumes it has a property called "angle", which is a single value that indicates the rotation angle about the Z axis.

Finally, if TrenchBroom has found a property that contains the rotation angle of the entity, it adapts the value of that property according to the rotation being performed by the user. These rules are quite complicated because sadly, the entity definitions do not contain information about how rotations should be applied to entities. But in practice, they should just perform as expected when you work with the rotate tool in the editor.

### Flipping Objects {#flipping_objects}

Flipping has the effect of mirroring the selected objects, the mirror being a plane which is defined by the center of the bounding box of the selected objects, snapped to the grid, and by a normal vector. The normal vector of the plane depends on the actual flipping command and the viewing direction of the camera in the 3D viewport or the view plane of the focused 2D viewport. The following table explains how the normal vector is derived from this information.

Shortcut                                                  Direction     Normal (2D)   Normal (3D)
--------                                                  ---------     -----------   -----------
#action('Controls/Map view/Flip objects horizontally')    Horizontal    Right axis    Axis-aligned right axis
#action('Controls/Map view/Flip objects vertically')      Vertical      Up axis       Z axis

In the case of the 3D viewport, the normal of the mirror plane is the coordinate system axis that is closest to the right axis of the camera. This means that if the camera is pointing in the general direction of the Y axis, and therefore its right axis points in the general direction of the X axis, the normal of the mirror plane will be the X axis. Sometimes, you will not be able to determine which of the coordinate system axes is closest to the right axis of the camera because the right axis is close to two coordinate system axes. To avoid such confusion, it is best to perform flipping in the 2D viewports.

### Deleting Objects

Deleting objects is as simple as selecting them and choosing #menu('Menu/Edit/Delete'). Note that if you delete all remaining brushes of a brush entity, that entity gets deleted automatically. Likewise, if you delete all remaining objects of a group, that group also gets deleted.

## Shaping Brushes

TrenchBroom offser several tools to change the shapes of brushes. The most powerful of these tools, and also the one that requires the most care, is the vertex tool. Before we discuss this tool, we will introduce the clip tool with which you can chop parts off of brushes. But first, we introduce the resize tool which, as the name suggests, allows you to quickly change the size of brushes. Finally, we explain how you can shape brushes using TrenchBroom's CSG operatons.

### Resizing

Brushes can be resized using the resize tool by moving their faces along their respective normal vectors with the mouse. To resize a selected brush, hold #key(306) and move your mouse pointer onto or near the face you wish to move. You will notice that one of the brush's faces is highlighted with a yellow outline. Drag with your left mouse button while still holding #key(306) to move the highlighted face along its normal. Note that you can also move brush faces which are behind the brush as long as these faces have an edge that is visible from the camera.

![Resizing a brush in the 3D viewport](ResizeTool3D.gif)

Note that you cannot change the number of faces of a brush by with the resize tool. This means that you cannot push a face back into a brush indefinitely. TrenchBroom will refuse to move it further as soon as that movement would make other faces disappear. The same applies to pulling a face out of a brush, which can make that face disappear. This is disallowed as well.

If you hold #key(308) when you start dragging, the brush will not be resized. Instead, a new brush will be created. Both of the original and the new brush together will have the same shape that the original would have had if you had just resized it without holding #key(308), and the two brushes are split where the face you were dragging originally sat.

![Splitting a brush in the 3D viewport](ResizeTool3DSplitMode.gif)

You can also resize several brushes at the same time by moving their faces using the resize tool, but only if these faces line up perfectly. As the following animation illustrates, it's not enough that the faces are parallel - they have to be identical.

![Resizing multiple brushes](ResizeTool3DMultipleBrushes.gif)

The resize tool also works in the 2D viewports, of course, but the ability to move faces which are behind the selected brush is absent there. In both cases, TrenchBroom uses two methods to determine how to snap the distance by which you drag the face:

- The distance is snapped to the current grid size, i.e., if you drag a face by 17.5 units along its normal, it will be moved by 16.0 units if the current grid size is 16. This is useful if you are resizing brushes which are part of a curve because their faces will line up after the drag.
- The vertices of the dragged faces are snapped to the grid planes, i.e., whenever at least one vertex component (X,Y, or Z) is a multiple of the current grid size, the face is snapped to that vertex. This makes it easy to align a face to other adjacent faces.

Both snap modes are used simultaneously. There may be situations when you have to move the camera closer to a face in order to have sufficient precision when dragging the face.

### Clipping

Clipping is the most basic operation for Quake maps due to how brushes are [constructed from planes](#brush_geometry). In essence, all that clipping does is adding a new plane to a brush and, depending on the brush's shape, removing other planes from it if they become superfluous. In TrenchBroom, clipping is done using the clip tool, which you can activate by choosing #menu('Menu/Edit/Tools/Clip Tool'). The clip tool lets you define a clip plane in various ways, and the lets you apply that plane to the selected brushes.

There are three different outcomes of applying a clip plane: Drop all parts of the selected brushes that are in front of the (oriented) clip plane, drop all parts of the selected brushes that are behind the clip plane, or slice the selected brushes into two pieces each. The following image illustrates these three modes:

![The three clip modes](ClipModes.png)

In all three images, there is a clip plane defined by two points. This clip plane slices the single brush in the image into two parts whereby the left part is below the plane and the right part is above it. In the first image, the clip mode is set to retain the part of the brush that is below the clip plane and to discard the part that is above the clip plane. The resulting brush will be shaped like the red part of the brush in the image. In the second image, the clip mode is set to retain both parts of the brush, and the result of this clipping operation will be two brushes. In the third image, the clip mode is set to retain the part of the brush that is above the clip plane and to discard the other part. This is the opposite of the first case. In the clip tool, you can cycle through these three modes by hitting #action('Controls/Map view/Toggle clip side'). There are two ways of defining a clip plane: The more common way is to place at least two and and most three points (in this context, these points are called clip points) in either the 3D or a 2D viewport. The other way is to define the clip plane by using an existing brush face.

#### Clip Points

To place clip points, you simply left click into a viewport when the clip tool is active. In the 3D viewport, you can only place clip points on already existing brushes, whereas in the 2D viewports, you can place them anywhere. Clip points are snapped to the grid, however, in the 3D viewport, there is a caveat which we will explain below. When the clip tool is active, it gives you some feedback in the form of an orange sphere that appears close to your mouse pointer. This sphere indicates where a clip point would be placed after being snapped to the grid. This feedback sphere is only shown if a clip point can actually be placed at or close to the point under the mouse.

Once two clip points have been placed, TrenchBroom will attempt to guess a clip plane even though it is underspecified: You cannot define a plane with only two points. If you are happy with the clip plane that TrenchBroom has determined, then you can apply the clipping operation by hitting #action('Controls/Map view/Perform clip'). Otherwise, you can place the third point to fully define the clip plane, or you can change the clip points you have already placed. To change a clip point, you can just left click and drag it with the mouse. To remove the most recently place clip point, you can hit #action('Controls/Map view/').

#### Clip Point Snapping

In the 3D viewport, clip points can only be placed on the faces of already existing brushes. Such a clip point is snapped to the grid that has been projected onto the brush face on which you placed that point. So it appears to be snapped to the projected grid, but it is also kept glued onto the brush face. If the point were snapped in all dimensions, then it would either sink into or move away from the brush face it was placed on. TrenchBroom avoids this by glueing clip points to the brush faces on which they were placed by the user. This means that if you attempt to move an already placed clip point around using the 3D viewport, that point will be moved to the closest snapped point on the brush face under the mouse.

In the 2D viewport, clip points are just snapped to the visible grid, so they are not restricted to being glued to brush faces. You can place clip points in any viewport you wish, and you can move clip points that have been placed in one viewport using any other viewport, but the grid snapping will be that of the viewport that you are using to move the clip point. That means if you use a 2D viewport to move a clip point that was placed in the 3D viewport, then that point can be dragged off of the brush face on which it was placed and into the void. Conversely, if you use the 3D viewport to move a clip point that was placed in a 2D viewport, that clip point will snap onto  the brush face under the mouse, or it will not move at all if there is no brush face under the mouse.

#### Matching Clip Plane

![Matching a clip plane](MatchingClipPlane.gif) The clip plane can also be defined by matching it to an existing brush face. To match a clip plane to an existing brush face, you have to double click that face in the 3D viewport. As a result, the brush face gets an orange outline, and a clip plane is defined to match the face's plane exactly. This can be quite useful when shaping geometry to other geometry. Note that the plane points of the clip plane are the plane points of the brush face to which the clip plane was matched, so there should be no trouble with microleaks when using this particular function.

### Vertex Editing

Unlike other editors, modifying the three geometric aspects of a brush, it's vertices, edges or faces, is all integrated into the vertex tool. To activate the vertex tool, choose #menu('Menu/Edit/Tools/Vertex Tool'). When the vertex tool is active, yellow handles appear at various points on the selected brushes to allow manipulation.

There are three types of handles: vertex handles, edge handles, and face handles. The vertex handles are positioned at the vertices of the brush, edge handles are positioned at the centers of the edges, and face handles are positioned at the centers of the faces. All handles are rendered as yellow spheres. Moving the mouse pointer over a handle highlights that handle with a red circular outline, and the position of that handle is displayed above it. When hovering over an edge handle, the edge itself is rendered in yellow, and when hovering over a face handle, the face itself is also rendered in yellow. See the following three images for examples.

![Selection highlighting for vertices, edges, and faces](VertexToolHighlighting.png)

Selecting handles is treated in the same way as selecting objects. To select a handle, click on a handle to select it. Multiple handles can be selected by holding #key(308). Once you select a handle of a particular type, all other handles except those of the same type are hidden. For example, if you select a vertex handle, only vertex handles are shown and the edge and face handles are hidden. As a consequence, you cannot mix handles of different types in a handle selection. The vertex tool also allows you to select multiple handles using a selection lasso. Left drag with the mouse button to create a rectangular selection lasso. Release the left mouse button, and all handles inside the lasso are selected. Note that lasso selection defaults to selecting vertex handles if no handle has been selected yet. Selected handles are rendered in red, and in the case of edge or face handles, the selected edges and faces are rendered in yellow.

![Selected handles turn red](VertexToolSelections.png)

When you have selected some handles, you can move them around by dragging them with the left mouse button. Moving handles (and their respective vertices, edges, or faces) works in a similar fashion to moving objects, so in the 3D viewport, you can move them on the XY plane, or you can hold #key(307) to move them vertically. In a 2D viewport, you can move the handles on the view plane of that viewport. If you begin your drag on an unselected handle, that handle is automatically selected, so if you just want to move a single handle around, you do not need to select it first. Once you press the left mouse button on a handle to begin a drag, yellow guide lines show up that help you to position the handle in relation to other objects. When moving handles around, the move distances are snapped to the current grid size component wise, just like when you move objects. However, in the vertex tool, you can hold #key(306) to change this behavior. If you hold #key(306), then the position of the handle that is being dragged will be snapped to the grid instead of the move distance. This makes it easer to force vertices to be on the grid.

Manipulating edges and faces with the vertex tool is just a shorthand to selecting all vertices incident to the edge or face at once. Thus, moving an edge with its handle is equivalent to selecting both vertices of that edge and move them at once, and the same applies to faces. Because of this, we will only discuss vertex editing in the remainder of this section, as all other operations are just special cases of vertex editing.

![Chopping faces](VertexToolFaceChopping.gif) TrenchBroom ensures that you do not create invalid brushes with the vertex tool. For example, it is impossible to make a brush concave by pushing a vertex into the brush. To achieve this, TrenchBroom will chop up the faces incident to that vertex into triangles depending on the direction in which that vertex is moved. In the animation on the left, you can see that the top face of the cube has one triangle chopped off in the first move where the vertex is moved downward, while in the second move, the front face is chopped into a triangle fan when the vertex is moved outward. Sometimes, TrenchBroom will even delete a vertex if a move would push it inside the brush, which would make it concave. In such a case, the vertex move is concluded.

The vertex tool also allows you to fuse adjacent vertices. If a vertex ends up on an adjacent vertex during a vertex move, the two vertices will be fused. This does not conclude the move however, you can keep moving the fused vertex, and it remains selected. Note that fusing is not allowed when you are moving edges or faces, even though fusing does happen when you move multiple vertices at once. If you want to restrict a vertex move operation to end up only on another vertex, you can double click the vertex to select it, and then drag it onto the target vertex. The vertex which you selected will not be moved until your drag lands it on another vertex.

![Splitting](VertexToolSplitting.gif) Besides moving and fusing vertices, you can also add new vertices to a brush with the vertex tool. By double clicking on an edge or face handle, you can split the edge or face that the handle represented. After the double click, all other handles are hidden, and you have to move the single remaining handle away from the brush to actually create the new vertex. As a shortcut, you can also start your mouse drag with the second press of the left mouse button when you double click on the edge or face handle that you wish to use for splitting.

Vertex editing is not limited to working with single brushes. Selecting more than one brush and activating the vertex tool will cause vertex handles to appear for all brushes in the selection. This is more useful when working on organic brushwork such as terrain. You can build a large group of brushes and modify them all at once without having to change the selection. Trenchbroom will recognize when vertices of multiple brushes share the same position. In this case, when trying to move a vertex, Trenchbroom will move all those vertices together making editing terrain much quicker and easier. In the following animation, the vertices under the cursor were moved with a single drag operation because they share the same position. This behaviour also applies to edge and face editing if the handles of those components share the same position in space.

![Vertex clumping](VertexToolVertexClumping.gif)

The vertex tool also provides some keyboard shortcuts to move vertices. These are listed in the following table.

Direction     Shortcut (2D)                                                            Shortcut (3D)
---------     -------------                                                            -------------
Left          #action('Controls/Map view/Move vertices left')                           #action('Controls/Map view/Move vertices left')
Right         #action('Controls/Map view/Move vertices right')                          #action('Controls/Map view/Move vertices right')
Up            #action('Controls/Map view/Move vertices up; Move vertices forward')       #action('Controls/Map view/Move vertices forward; Move vertices up')
Down          #action('Controls/Map view/Move vertices down; Move vertices backward')    #action('Controls/Map view/Move vertices backward; Move vertices down')
Forward       #action('Controls/Map view/Move vertices forward; Move vertices up')       #action('Controls/Map view/Move vertices up; Move vertices forward')
Backward      #action('Controls/Map view/Move vertices backward; Move vertices down')    #action('Controls/Map view/Move vertices down; Move vertices backward')

This concludes the functionality of the vertex tool. While it is very powerful, it should also be used with care, as vertex editing can sometimes create invalid brushes and microleaks in the map. To help you avoid such problems, the following section contains a few best practices you should keep in mind when you use the vertex tool.

#### Best Practices

- Don't use it too much on sealing brushes, better to use it on detail.
- Don't do too much in one go, compile and test often.
- When doing terrain, create a quad- or trisoup and only move vertices orthogonal to the grid's plane (along the plane's normal). Don't move vertices sideways.
- Be careful with detail brushes, they might open vis portals.
- Detail might also result in PVS leaves too much information, better to turn some detail into actual brushes to force vis to break a room into several PVS leaves. Or use hint brushes to force vis to add more leaves.

### CSG Operations

CSG stands for Constructive Solid Geometry. CSG is a technique used in professional modeling tools to create complex shape by combining simple shapes using set operators such as union (sometimes called addition), subtraction, and intersection. However, CSG union and subtraction cannot be directly applied to brushes since they may create concave shapes which cannot be represented directly using brushes (remember that brushes are always convex). But some of these operators can be emulated with brushes. TrenchBroom supports the operations _convex merge_ (in place of union), _subtraction_ (emulated by creating new brushes to represent the resulting concave shape), and _intersection_ (supported directly).

#### CSG Convex Merge

Convex merge takes a set of brushes as its input, computes the _convex hull_ of all vertices of those brushes, and creates a new brush with the shape of the convex hull. The convex hull of a set of points is the smallest convex volume that contains all of the points. In the following animation, two brushes are being merged into one. The operation takes the vertices of both brushes and computes its convex hull. Some of the original brushes' vertices end up as vertices of the convex hull, and some of them are discarded, such as the vertices at the bottom right corner of the top left brush in the 2D viewport. The discarded vertices are those which end up inside the convex hull.

![CSG convex merge](CSGConvexMerge.gif)

As you can see, the newly created brush covers some areas which were not covered by the original brushes. This follows the restriction that the resulting brush must be convex. Whether the resulting brush covers such previously void areas depends on how the input brushes are aligned with each other. To perform a convex merge, select the brushes to be merged and choose #menu('Menu/Edit/CSG/Convex Merge').

#### CSG Subtraction

CSG subtraction takes one brush (the minuend) and subtracts it from a set of brushes (the subtrahends) by subtracting the minuend individually from each subtrahend, so we can reduce the case of multiple subtrahends to the case of one subtrahend for the following explanations. In addition, we can limit ourself to cases where the minuend does not protrude out of the subtrahend because we can chop off such parts of the minuend without changing the result of the subtraction. In general, the result of a CSG subtraction of two shapes is a concave shape. Since a concave shape cannot be represented directly using a single brush, it has to be represented using multiple brushes (the result set). Unfortunately, there is an infinite number of result sets, all of which represent the concave shape that was the result of the subtraction. However, some result sets are better than others because the have a few desirable characteristics:

- The brushes in the result set represent the concave result shape perfectly.
- The brushes are pairwise disjoint (i.e., none of them overlap).
- The result set contains as few brushes as possible.
- The brushes are somehow symmetrical if possible.
- The brushes in the result set only reuse the vertices of the subtrahend and the (chopped) minuend and as few additional vertices as possible.

Computing a result set that has all of these characteristics is very hard. That's why TrenchBroom uses a few heuristics when it computes the result set. These heuristics lead to result sets that always fulfil the first two critera, but cannot always be optimal in all of the remaining critera. Particularly the last criterion may be violated in some cases where oddly shaped brushes are subtracted from each other. Let's consider an example of a "good" result set. In the following animation, we create an arch by subtracting the smaller brush from the larger one:

![CSG subtracting to create an arch](CSGSubtractArch.gif)

The result set contains seven brushes, and it is optimal according to the criteria listed above: It represents the convex result shape perfectly, all brushes are pairwise disjoint, there is no smaller result set possible, all brushes in the result set disjoint, the brushes are mirror symmetrical, and they only use vertices of the chopped minuend. Remember that chopping the minuend removes all parts of it that protrude out of the subtrahend.

Next, we take a look at a CSG subtraction with a suboptimal result set. In the following animation, there are two cuboids. The outer cuboid (the subtrahend) is axis aligned and the inner cuboid (the minuend) has been rotated about two axes. Several of its corners protrude out of the subtrahend. It may be a bit hard to see that the inner brush is actually a cuboid from the images.

![CSG subtracting two cuboids](CSGSubtractCuboids.gif)

The result set of this subtraction is not optimal according to the criteria listed before. On one hand, the result set does represent the convex result shape perfectly, and all brushes in the result set are pairwise disjoint. On the other hand, TrenchBroom had to introduce new vertices in order to create the brushes in the result set. One of these additional vertices can be seen on the front face of the outer cuboid. Three triangles meet at this vertex. Furthermore, the result set may not be minimal - there might be a smaller result set, but TrenchBroom could not find it. And finally, the brushes in the result set are not symmetrical - but with the given two brushes as input, it could not be symmetric anyway.

To perform a CSG subtraction, first select the subtrahends (the brushes you wish to subtract from) and then add the minuend to the selection and choose #menu('Menu/Edit/CSG/Subtract'). Assuming you have selected n brushes, TrenchBroom applies the subtraction to the selected brushes by subtracting the nth selected brush (the most recently selected one) from the n-1 previously selected brushes.

In summary, TrenchBroom provides you with a CSG subtract algorithm that can produce "good" results according to the aforementioned criteria. These criteria attempt to capture what a designer might consider a good CSG subtraction solution, so TrenchBroom's CSG subtraction should be more useful than the simplistic implementations found in other tools.

#### CSG Intersection

CSG intersection takes a set of brushes and computes their intersection, that is, it computes the largest brush that is contained in each of the input brushes. Another way to think of this is that intersection takes that part of the input brushes where they all overlap, and creates a new brush that represents that part. If there is no such part, then the input brushes are disjoint, and their intersection is empty. The input brushes are then removed from the map.

![CSG intersection of two cuboids](CSGIntersect.gif)

You can perform a CSG intersection by selecting the brushes you wish to intersect, and then choosing #menu('Menu/Edit/CSG/Intersect').

#### Textures and CSG Operations {#textures_and_csg_operations}

In each of the CSG operations, new brushes are created, and TrenchBroom has to assign textures to their faces. To determine which texture to assign to a new brush face, TrenchBroom will attempt to find a face in the input brushes that has the same plane as the newly created face. If such a face was found, TrenchBroom assigns the texture and attributes of that brush face to the newly created brush face. Otherwise, it will assign the [current texture](#working_with_textures).

![CSG texture handling](CSGTexturing.gif)

In the example above, a brush is subtracted from another brush to form an archway. The subtrahend has a blue brick texture and the minuend has a dark metal texture. After the subtraction, those faces of the result that line up with the faces of the subtrahend have also been assigned the blue brick texture, while those faces that line up with the faces of the minuend brush have been assigned the dark metal texture. Some of the faces of the result brushes are invisible because they are shared by other brushes - these faces have been assigned the current texture because no face of the subtrahend or the minuend lines up with them.

## Working with Textures {#working_with_textures}

There are two aspects to working with textures in a level editor. [Texture management](#texture_management) and texture application. This section deals with the latter, so you will learn different ways to apply textures to brush faces and to manipulate their alignment to the faces. But before we dive into that, we have to cover three general topics: First, we present the texture browser, then we explain how TrenchBroom assigns textures to newly created brush faces, and finally we discuss different texture projection modes in TrenchBroom.

### The Texture Browser {#texture_browser}

![The texture browser](TextureBrowser.png) The texture browser is part of the face inspector and is used for two purposes: Changing the texture for the currently selected faces and selecting the _current texture_. In the texture browser, textures are displayed with a maximum width of 64 pixels - wider textures are proportionally scaled down. The name of every texture is displayed below the image. Textures that are currently in used have a yellow border, while the current texture has a red border. If you hover over a texture image with the mouse, you will see a tooltip with the name and the dimensions of the texture.

Below the texture browser, there are the same controls as in the [entity browser](#entity_browser): A dropdown for changing the sort order (name or usage count), a button to group by [texture collection](#texture_management), a button to filter by usage, and a text field to filter by name. If the size of the images is too small or too large on your monitor, you can change it in the [preferences](#view_layout_and_rendering).

### Texture Projection Modes {#texture_projection_modes}

In the original Quake engine, textures are projected onto brush faces along the axes of the coordinate system. In practice, the engine (the compiler, to be precise), uses the normal of a brush face to determine the projection axis - the chose axis is the one that has the smallest angle with the face's normal. Then, the texture is projected onto the brush face along that axis. This leads to some distortion (shearing) that is particularly apparent for slanted brush faces where the face's normal is linearly dependent on all three coordinate system axes. However, this type of projection, which we call _paraxial texture projection_ in TrenchBroom, also has an advantage: If the face's normal is linearly dependent on only two or less coordinate system axes (that is, it lies in the plane defined by two of the axes, e.g., the XY plane), then the paraxial projection ensures that the texture still fits the brush faces without having to change the scaling factors.

The main disadvantage of paraxial texture projection is that it is impossible to do perfect texture locking. _Texture locking_ means that the texture remains perfectly in place on the brush faces during all transformations of the face. For example, if the brush moves by 16 units along the X axis, then the textures on all faces of the brush do not move relatively to the brush. With paraxial texture projection, textures may become distorted due to the face normals changing by the transformation, but it is impossible to compensate for that shearing.

This is (probably) one of the reasons why the Valve 220 map format was introduced for Half Life. This map format extends the brush faces with additional information about the texture axes for each brush faces. In principle, this makes it possible to have arbitrary linear transformations for the texture coordinates due to their projection, but in practice, most editors keep the texture axes perpendicular to the face normals. In that case, the texture is projected onto the face along the normal of the face (and not a coordinate system axis). In TrenchBroom, this mode of texture projection is called _parallel texture projection_, and it is only available in maps that have the Valve 220 map format.

### How TrenchBroom Assigns Textures to New Brushes

In TrenchBroom, there is the notion of a current texture, which we have already mentioned previous sections. Initially, the current texture is unset, and it is changed by two actions: selecting a brush face and selecting the current texture by clicking on a texture in the texture browser. When TrenchBroom creates a new brush or a new brush face, it may consult the current texture to determine which texture to apply to the newly created brush faces. This is not always the case: Sometimes, TrenchBroom can determine textures for newly created brush faces from the context of the operations. We have discussed this earlier for [CSG operations](#textures_and_csg_operations). In other cases, such as when you create a new brush with the mouse, TrenchBroom will always apply the current texture.

### Assigning Textures Manually

There are several ways in which TrenchBroom lets you change the texture of a brush face. Firstly, you can change the texture of the currently selected faces by clicking on a texture in the texture browser. This also works if you have selected brushes (and nothing else) - in this case, the new texture is applied to all faces of the currently selected brushes. Secondly, you can copy the texture from a selected face to another face as follows: First, select the brush face that has the texture that you wish to copy, then click on the brush face that you wish to copy the texture to with the left mouse button while holding #key(307). If you wish to copy the texture to all faces of a brush, you can double click the left mouse button while holding #key(307). Note that this only copies the texture and not the face attributes such as the offset or scale. If you wish to copy the attributes also, you need to hold #key(308) in addition to holding #key(307). Finally, you can use copy and paste to copy the texture and attributes of a selected face onto other faces: First, select the face that you wish to copy from and choose #menu('Menu/Edit/Copy'), then selected the faces that you wish to copy to, and choose #menu('Edit/Copy/Paste').

### Replacing Textures

If you want to replace a particular texture with another one, you can choose #menu('Menu/Edit/Replace Texture...'). This opens a window where you can select the texture to be replaced and the replacement texture using two texture browser. This window is depicted in the following screenshot.

![Texture Replace Window (Mac OS X)](ReplaceTexture.png)

Select the texture you wish to replace in the left texture browser. This browser by default only shows you the textures which are currently in use in the map. In the screenshot, the texture "b_pv_v1a1" has been selected for replacement and therefore has a red border. Then select the replacement texture in the right texture browser ("b_sr_20c" in the screenshot). Finally, hit the "Replace" button. The replacement is applied to all brush faces in the map if nothing is currently selected. Otherwise, it is applied to the selected brush faces only. If the replacement succeeded, the faces which have been replaced are subsequently selected. Otherwise, the selection remains unchanged.

Note that the window is not modal - you can switch back to the main window to change your selection if you wish.

### Setting Face Attributes

Face attributes control how textures are mapped onto brush faces. At the very least, every face has the attributes offset, scale, and angle. The offset allows you to shift a texture on a face, the scale factors stretch the texture, and by changing the angle you can rotate the texture. Additionally, some engines have further attributes. Quake 2 adds surface flags and a surface value, and additional content flags. All of these values can be changed in different ways: There is a face attribute editor that allows you to enter the values directly, you can use keyboard shortcuts in the 3D viewport, or you can use the UV editor.

#### The Face Attribute Editor

The face attribute editor is located in the face inspector, right between the UV editor and the texture browser. It contains several controls to edit the face attributes of one or several selected brush faces.

![Face Attribute Editor (Mac OS X)](FaceAttribsEditor.png)

Two types of controls are visible in the screenshot above. Numerical input controls consist of a text field and small buttons to increase or decrease the value in the field. The text field will show the value of the respective face attribute, such as "1" for the X Scale in the screenshot. If more than one brush face is selected, the text field will also show the value if all faces have the same value for the respective attribute, or it will show the placeholder word "multi" otherwise. In the screenshot above, the X Offsets of the selected brush faces differ, hence the text field shows "multi". All other values are identical for all selected brush faces, so all other attribute editors show concrete values instead of the placeholder. By entering a number into the text field, the attribute value of all selected brush faces can be set to that value. Consequently, if you were to enter the value "32" into the X Offset editor in our example above, all selected brush faces would have this value as their X Offset afterwards.

The spin button however works differently. By clicking the up- or down arrow button, you can increase or decrease the value of the respective face attribute by a certain delta value, which depends on the grid settings and the currently pressed modifier keys. The following table explains which delta value is chosen in each case.

Attribute    Default      #key(306) pressed    #key(308) pressed
---------    -------      -----------------    -----------------
Offset       Grid size    2 * grid size        1.0 
Scale        0.1          0.25                 0.01
Angle        15°          90°                  1°

Note that these deltas are applied to the respective attributes of every selected brush face. So if you have selected two brush faces, one with an X Offset of 0 and one with an X Offset of 8, and your current grid size is 16, clicking on the up arrow button next to the X Offset attribute editor will change the X Offsets to 16 and 24, respectively. Only entering a value in the text field will set the two X Offsets to the same value.

In addition to using the buttons to change the values, you can use the scroll wheel or the arrow keys when the text field has focus. Scrolling and the arrow keys follow the same rules to determine the delta values as described in the table above.

For attributes that represent flag values, such as the surface and content flags for Quake 2, there is a different type of control available in the face attribute editor. This control shows a textual representation of the flag values in a text field, and you can change the flags using a dropdown window that is shown if you click on the button labeled "..." next to the text field. The dropdown window contains one checkbox for each flag, and you can check or uncheck them individually.

#### Keyboard Shortcuts

In the 3D viewport, you can change the offset and angle of the currently selected brush faces using the following keyboard shortcuts:

Attribute    Keys                                    Default      #key(306) pressed    #key(308) pressed
---------    ----                                    -------      -----------------    -----------------
Offset       #key(314)#key(316)#key(315)#key(317)    Grid size    2 * grid size        1.0 
Angle        #key(366)#key(367)                      15°          90°                  1°

Note that TrenchBroom attempts to match the shortcuts to the directions in which the textures will move visually. This means that pressing #key(315) will move a texture roughly in that directiony visually instead of always applying to the same face attribute. So depending on the camera angle and the orientation of the face itself, pressing #key(315) may affect the X or Y offset by incrementing or decrementing it. The angle is treated similarly: Pressing #key(366) will rotate the texture counter clockwise visually, and pressing #key(367) will rotate it clockwise.

#### The UV Editor {#uv_editor}

The UV editor is located at the top of the face inspector. Using the UV editor, you can adjust the offset, the scale and the angle of the texture of the currently selected brush face. Note that the UV editor is only usable if one brush face is selected. If multiple brush faces are selected, the UV editor is empty.

![UV Editor](UVEditor.png)

The texture of the current face is shown in the background of the UV editor. The texture is tiled, and the tiling edges are displayed in gray. These tiling edges are called the _texture grid_. The shape of the currently selected brush face is displayed in white. The yellow filled circle marks the origin of the texture, and the two red lines that meet at the origin mark the origin axes of the texture - these are used for scaling. The larger yellow circle is a handle used for rotating the texture. The texture axes are displayed in red and green at the center of the brush face.

To change the offset of the texture in relation to the brush face, you can just click and drag the texture anywhere with the left mouse button. Note that the texture will snap to the vertices of the brush face to make alignment easier.

You can scale the texture by clicking and dragging the gray texture grid lines, which will also snap to the vertices of the brush face. Scaling is relative to the origin of the texture, as marked by the yellow circle. To change the scaling origin, you can left click and drag the yellow circle, or you can left click and drag the red lines meeting at the origin. The lines allow you to set the X and Y coordinates of the origin separately. The origin snaps to the vertices of the face and to its center.

The large yellow circle allows you to rotate the texture about the origin. Left click and drag the circle with the mouse to adjust the rotation angle. The angle will snap to the edges of the brush face to make it easier to adjust it to the shape of the face.

If the map you are currently editing uses a [parallel texture projection](#texture_projection_modes), you can also shear the texture by left clicking and dragging the gray texture grid lines while holding #key(307).

At the bottom of the UV editor, you can find a number of controls. On the left, there are five buttons. The first button resets the face attributes to their defaults. The second and third buttons flip the texture horizontally and vertically, respectively, and the fourth and firth buttons rotate the texture by 90° counter clockwise and clockwise. The two controls on the right allow you to subdivide the texture grid. This can be useful to align the texture to smaller brush faces.

## Entity Properties {#entity_properties}

An entity is, essentially, a collection of properties, and a property is a key value pair where both the key and the value is a string. Some values have a special format, such as colors, points, or angles. But in general, if you are editing an entity, you will be working with strings. In TrenchBroom, you can add, remove, and edit entity properties using the entity property editor, which is located at the top of the entity inspector.

![Entity Property Editor](EntityPropertyEditor.png)

The entity property editor is split into two separate areas. At the top, there is a tabular representation of the properties of the currently selected entities and, if applicable, the defaults for those properties which are not present in the selected entities. The default properties are shown in _italics_ below the actual properties of the selected entities. To hide the default properties, you can uncheck the checkbox at the bottom of the table.

### Editing Properties

To select an entity property, just click in the row that represents that property in the table. The row will be highlighted, and at the same time, the field which you clicked on will get a black outline. The black outline is called a _cursor_ and it indicates that you can change the outlined field by entering text. In the screenshot above, the "origin" property has been selected, and its key has the cursor, indicating that it is ready to be changed.

If you are changing a lot of properties, you may wish to navigate quickly through the table. You can use the cursor keys to move the cursor around in the table. Alternatively, you can hit #key(9) to move field by field. If the cursor is on a key of some property, hitting tab will move the cursor the value field of that property, and hitting tab again will move it to the key field of the next property, and so on until you reaach the end of the table. You can also move in the opposite direction by hittin #key(306)#key(9). #key(13) moves vertically through the list, meaning that if the cursor is on a property key and you hit enter, the cursor will move to the key of the next property in the list. Use this navigation method to mass renamed property keys, for example.

To change the key or the value of a property, set the focus to the appropriate field in the table. If you enter some text now, that text will replace the key of the property. An alternative way to change a field is to click on it while its property is already selected. This will show an actual text field in which you can enter the text.

There are several ways to add a property to an entity. First, you can click on the button with the "+" label at the bottom of the table. This will insert a new property with a default name and no value into the table. Second, you can hit #key(308)#key(13) to add a new property. In both cases, the new property will be selected so that you can start editing its key and value right away as described above. Finally, you can add a property by changing the value of a default property. This will promote the default property to an actual property of the entity.

To remove entity properties, you should select the rows in the table that represent them and hit the button labeled "-" at the bottom of the table.

### Multiple Entity Selections

![Multiple Entity Selections](EntityPropertyEditorMultiSelection.png) If multiple entities are selected, the table will show the union of all their properties and not just those properties which all of the selected entities have in common. Properties which are not present in all of the selected entities have their names grayed out, and properties which have different values in those entities that actually have those properties are displayed with an empty value. In the screenshot, there are three light entities selected. Consequentially, the "classname" property is present in all of them and has the same value everywhere. Likewise, the "origin" property is present in all of these entities, but it has different values in each of them, so it is shown without a value. The "light", "wait", "angle", and "mangle" properties on the other hand are only present in some of the selected entities, but they do have the same values in each of the entities that have them.

If you change an entity property when multiple entities are selected, the change gets applied to all of the selected entities, even if that requires adding that property. So if you were to change the value of the "light" property in the example above to 200, each of the selected entities will subsequently have a "light" property with the value 200, even if only a subset of the selected entities had that property before.

### Smart Entity Property Editors

TrenchBroom provides special editors for the following entity properties: spawnflags, colors, and choices. These special editors are callled _smart property editors_ and are displayed below the entity property table if you select an entity property for which such an editor exists. 

Type             Editor                                                  Description
----             ------                                                  -----------
Spawnflags       ![Smart Spawnflags Editor](SmartSpawnflagsEditor.png)    A table of checkboxes which allow you to toggle the individual spawnflag values.
Color            ![Smart Spawnflags Editor](SmartColorEditor.png)         A color chooser control that allows you to convert between byte and float color values, and provides a list of all colors found in the map.
Choice           ![Smart Spawnflags Editor](SmartChoiceEditor.png)        A dropdown list of values. You can also enter any text into the text box.

### Linking Entities

Entities can be linked using special link properties. Each link has a source and a target entity. The target entity has a property called "targetname", and the value of that property is some arbitrary string. The souce entity has a "target" or a "killtarget" property, and the value of that property is the value of the target entity's "targetname" property. To create an entity link, you have to manually set these properties to the proper values. Currently, the names of the link properties are hardcoded into TrenchBroom, but in the future they will be read from the FGD file if appropriate. The following section explains how entity links are visualized in the editor.

### Entity Link Visualization

Entity Links are rendered as lines in the 3D and 2D viewports. TrenchBroom provides you with for modes for entity link visualization. You can switch between these modes in the dropdown menu that is displayed when you click on the "View" button at the right of the info bar. The following table explains the four different modes.

Mode                   Description
----                   -----------
All                    Always show all entity links.
Transitive selected    Show all entity links connected to the selected entities, and any link that is reachable from the selected entities, too.
Direct selected        Show all entity links connected to the selected entities.
No                     Don't show any entity links.

An entity link that is connected to a currently selected entity is rendered as a red line that connects the selected entity with the source or target of that link. Other entity links are colored green.

![Entity Link Visualization](EntityLinkVisualization.png)

In the screenshot above, the link between the two info_null entities is rendered in green because neither of the entities is selected.

## Undo and Redo

Almost everything that you do in TrenchBroom can be undone by choosing #menu('Menu/Edit/Undo'). This applies to every action that somehow modifies the map file (such as moving objects), but it also applies to some actions that do not change the map file, such as selection, hiding, and locking. There is no limit to how many actions you can undo, and once an action is undone, you can redo it by choosing #menu('Menu/Edit/Redo').

### Undo Collation and Transactions

Note that TrenchBroom groups certain sequences of actions into transactions which can be undone and redone as one. For example, if you select a few objects and then hide them, the objects are automatically deselected. Both the action of deselecting the objects to be hidden and hiding them are grouped together into a transaction, so when you undo, the objects will be unhidden and reselected at the same time.

On top of that, TrenchBroom will merge sequences of the same action if they happen within one mouse drag or within a certain time. So if you move a brush around, all steps of the move will be merged into one action, or if you move a brush around by pressing the appropriate keyboard shortcuts within a certain time, all these actions will also be merged into one. In practice, this saves memory and it allows you to undo such sequences in one fell swoop.

# Keeping an Overview

If you are working on large maps, it can become cumbersome to manage the objects in the map and to keep an overview over them. Some areas may be crowded with a lot of brushes and entities so that it becomes difficult to edit a particular object that is occluded by other things. TrenchBroom provides you with a number of tools to easily keep an overview over your map and to remove clutter in crowded areas.

## Filtering {#filtering_rendering_options}

To filter out certain types of objects, you can open the view dropdown window by clicking on the "View" button at the right of the info bar above the editing area.

![The info bar with view dropdown (Windows 7)](ViewDropdown.png)

In the left of the view dropdown, there is a list of checkboxes that allows you to hide all entities that share the same entity definition (i.e., the same classname). Uncheck an entity definition (or a group thereof) to hide the respective entities. For quickly hiding and showing all entities, you can click on the two buttons below the list. The right half of the view dropdown contains some options for the renderer, most of which are self explanatory.

## Hiding and Isolation

If you are working on a crowded area, it can be useful to hide certain objects, or to hide everything but the objects of interest. To hide the selected objects, choose #menu('View/Hide'), and to isolate the selected objects, choose #menu('View/Isolate'). To show all hidden objects, choose #menu('View/Show All'). All of these actions can be undone.

## Locking

Locking prevents objects from being selected or edited in anyway. Locked objects are rendered with blue edges and their faces are tinted in blue, as shown in the following screenshot.

![Locked Objects](Locking.png)

Objects can be locked either if you are editing an open group or if you set a layer to locked (see below). You cannot lock objects individually.

## Groups {#groups}

Groups allow you to treat several objects as one and to give them a name. A group can contain the following types of objects: entities, brushes, and more groups. The fact that a group can contain groups induces a hierarchy - but in practice, you will rarely create such nested groups. In the viewports, groups have their bounding box rendered in blue, and their name is displayed above them.

To create a group, select some objects and choose #menu('Menu/Edit/Group'). The editor will ask you for a name. Group names need not be unique, so you can have several groups with the same name. To select a group, you can click on any of the objects contained in it. This will not select the individual object, but the entire group, which is why you can only editor all objects within a group as one. If you want to edit individual objects in a group, you have to open the group by double clicking on it with the left mouse button. This will lock every other object in the map (locked objects are not editable and rendered in blue). Once the group is opened, you can edit the individual objects in it, or you can create new objects within the group in the usual ways. Once you are done editing the group, you can close it again by left double clicking anywhere outside of the group. Finally, you can remove a group by selecting it and choosing #menu('Menu/Edit/Ungroup'). Note that removing a group does not remove the objects in the group from the map, the objects are merely ungrouped.

## Layers {#layers}

Layers decompose your map into several parts. For example, you might create a layer for separate rooms or areas. Layers can contain groups, entities, or brushes, and each of these objects can belong to one layer only. Each layer has a name and can be set to hidden or locked. Every map contains a "Default Layer" that cannot be removed. This layer receives all objects that haven't been assigned to another layer.

![Layer Editor](LayerEditor.png)

Layers are managed in the layer editor, which is part of the map inspector. The layer editor displays a list of all layers in your map. You can hide or show a layer by clicking on the eye icon below the layer name, and you can lock or unlock a layer by clicking on the lock icon. To create a new layer, click the plus button at the bottom of the layer list, and to remove one ore more layers, select them and click on the minus button. They eye button next to the minus button shows all layers.

When you create new objects, TrenchBroom puts them into the current layer (unless you are working in a group). The current layer is indicated in the layer list by having its name in bold, and you can set the current layer by double clicking on a layer in the layer list. Note that you cannot hide or lock the current layer, and if you make a layer the current layer, that layer is shown and unlocked automatically. The fact that the current layer can neither be hidden nor locked also implies that you cannot delete the only visible or the only unlocked layer, because then TrenchBroom would have to choose a hidden or a locked layer to be the new current layer.

# Preferences

The preferences dialog allows you to set the game configurations, to change the view layout and control the rendering, and to customize the mouse and the keyboard shortcuts. You can open the preferences dialog by choosing #menu('Menu/File/Preferences...'). The dialog is split into four panes, each of which we will review in the following sections.

## Game Configuration {#game_configuration}

## View Layout and Rendering {#view_layout_and_rendering}

## Mouse Input {#mouse_input}

## Keyboard Shortcuts

# Advanced Topics

## Command Repetition

## Issue Browser {#issue_browser}

## Solving Problems

### Automatic Backups

## Display Models for Entities

## Customization

# Getting Involved

## Suggest a Feature

If you have an idea for a nice feature that you're missing in TrenchBroom, then you can submit a request at the [TrenchBroom issue tracker]. Try to describe your feature, but don't go into too much detail. If it gets picked up, we will hash out the details together.

## Reporting Bugs {#reporting_bugs}

You can submit bug reports at the [TrenchBroom issue tracker]. Be sure to include the version information (see below) and try to describe the steps to reproduce.

### The Version Information

Open the "About TrenchBroom" dialog from the menu. The light gray text on the left gives you some information about which version of TrenchBroom you are currently running, for example "Version 2.0.0 f335082 D". The first three numbers represent the version (2.0.0), the following seven letter string is the build id (f335082), and the final letter indicates the build type ("D" for Debug and "R" for release). You can also find this information in the Welcome window that the editor shows at startup.

### Contact

- IRC: \#trenchbroom on quakenet.org

# References and Links {#references_and_links}

- [TrenchBroom website] - TrenchBroom's website
- [TrenchBroom on github] - TrenchBroom's github page
- [func_msgboard] - Quake Mapping Forum
- [Tome of Preach] - Quake Map Hacks and QuakeC Hacks

[TrenchBroom website]: http://kristianduske.com/trenchbroom/
[TrenchBroom on github]: http://github.com/kduske/TrenchBroom/
[TrenchBroom issue tracker]: http://github.com/kduske/TrenchBroom/issues/
[func_msgboard]: http://celephais.net/board/
[Tome of Preach]: https://tomeofpreach.wordpress.com/
[FGD File Format]: http://developer.valvesoftware.com/wiki/FGD
