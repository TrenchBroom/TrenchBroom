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

### Brush Geometry

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

You can click the button labelled "New map..." to create a new map or you can click the button labelled "Browse..." to find a map file on your computer. Double click one of the documents in the list on the right of the window to open it. The light gray text on the left gives you some information about which version of TrenchBroom you are currently running. The version information is useful if you wish to report a problem with the editor (see [here](#reporting_bugs) for more information).

If you choose to create a new map, TrenchBroom needs to know which game the map should be for, and will show a dialog in which you can select a game and, if applicable, a map format. This dialog may also be shown when you open an existing map. TrenchBroom will try to detect this information from the map file, but if that fails, you need to select the game and map format.

![The game selection dialog (Mac OS X)](GameSelectionDialog.png)

The list of supported games is shown on the right side of the dialog. Below the game list, there is a dropdown menu for choosing a map format; this is only shown if the game supports more than one map format. One example for this is Quake, which supports both the standard format and the Valve 220 format for map files. In the screenshot above, none of the games in the list were actually found on the hard disk. This is because the respective game paths have not been configured yet. TrenchBroom allows you to create maps for missing games, but you will not be able to see the entity models in the editor and other resources such as textures might be missing as well. To open the game configuration preferences, you can click the button labelled "Open preferences...". Click [here](#game_configuration) to find out how to configure the supported games.

Once you have selected a game and a map format, TrenchBroom will open the main editing window with a new map. The following section gives an overview of this window and its main elements. If you want to find out how to [work with mods](#mods) and how to [add textures](#texture_management), you can skip ahead to the respective sections.

## Main Window {#main_window}

The main window consists of a menu bar, a toolbar, the editing area, an inspector on the right and an info panel at the bottom. In the screenshot below, there are three editing area: one 3D viewport and two orthographic 2D editing area.

![The main editing window (Linux XFCE)](MainWindow.png)

The sizes of the editing area, the inspector and the info bar can be changed by dragging the dividers with the mouse. This applies to some of the dividers in the inspector as well. If a divider is 2 pixels thick, it can be dragged with the mouse. The following subsections introduce the most important parts of the main window: the editing area, the inspector, and the info bar. The toolbar and the menu will be explained in more detail in later sections.

### The Editing Area

The editing area is divided in two sections: The context sensitive info bar at the top and the viewports below. The info bar contains different controls depending on which tool is currently activated. You can switch between tools such as the rotate tool and the vertex tool using the toolbar buttons, the menu or with the respective keyboard shortcuts. The context sensitive controls allow you to perform certain actions that are relevant to the current tool such as setting the rotation center when in the rotate tool or moving objects by a given delta when in the default move tool. Additonally, there is a button labeled "View" on the right of the info bar. Clicking on this button unfolds a dropdown containing controls to [filter out](#filtering) certain objects in the viewports or to change how the viewport [renders its contents](#rendering_options).

![The info bar with view dropdown (Windows 7)](ViewDropdown.png)

There are two types of viewports: 3D viewports and 2D viewports. TrenchBroom gives you some control over the layout of the viewports: You can have one, two, three, or four viewports. See section [View Layout and Rendering](#view_layout_and_rendering) to find out how to change the layout of the viewports. If you have fewer than four viewports, then one of the viewports can be cycled by hitting #action('Controls/Map view/Cycle map view'). Which of the viewports can be cycled and the order of cycling the viewports is given in the following table:

No. of Viewports    Cycling View         Cycling Order
----------------    ------------         -------------
1                   Single view          3D > XY > XZ > YZ
2                   Right view           XY > XZ > YZ
3                   Bottom right view    XZ > YZ
4                   None

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

## Selecting Objects in the 3D Viewport

To select a single object, simply left click it in the 3D viewport. Left clicking anywhere in the 3D viewport deselects all other selected objects, so if you wish to select multiple objects, hold #key(308) while left clicking the objects to be selected. If you click an already selected object while holding #key(308), it will be deselected. You can also _paint select_ multiple objects. First, select an object, then hold #key(308) while dragging over unselected objects. Take care not to begin your dragging over a selected object, as this will [duplicate the selected objects](#duplicating_objects).

You can only select the frontmost object with the mouse. To select an object that is obstructed by another object, you can use the mouse wheel. First, select the frontmost object, that is, the object that occludes the object that you really wish to select. Then, hold #key(308) and scroll up to push the selection away from the camera or scroll down to pull the selection towards the camera. Note that the selection depends on what object is under the mouse, so you need to make sure that the mouse cursor hovers over the object that you wish to select.

![Selection drilling in the 3D viewport](DrillSelection.gif)

Sometimes, selecting objects manually is too tedious. To select all currently editable objects, you can choose #menu('Menu/Edit/Select All') from the menu. Note that hidden and locked objects are excluded, so this command is particularly useful in conjunction with those features. Another option to select multiple objects at once is to use a _selection brush_. Just create a new brush that encloses or touches all the objects you wish to select. This brush is called a selection brush. Then choose #menu('Menu/Edit/Select Inside') to select every object enclosed inside that brush, or choose #menu('Menu/Edit/Select Touching') to select every object touched by that brush. If you have selected a single brush that belongs to an entity or group, and you wish to select every other object belonging to that entity or group, you can choose #menu('Menu/Edit/Select Siblings'). The same effect can be achieved by left double clicking on a brush that belongs to an entity or group.

![Using a selection brush](SelectTouching.gif)

The menu command #menu('Menu/Edit/Select by Line Number') is useful for diagnostic purposes. If an external program such as a map compiler presents you with an error message and a line number indicating where in the map file that error occured, you can use this menu command to have TrenchBroom select the offending object for you.

Finally, you can deselect everything by left clicking in the void, or by choosing #menu('Menu/Edit/Select None').

## Selecting Objects in a 2D Viewport

In a 2D viewport, you can also left click an object to select it. But unlike in the 3D viewport, this will not necessarily select the frontmost object. Instead, TrenchBroom will analyze the objects under the mouse, specifically the faces under the mouse, and it will find the one with the smallest visible area. Having found such a face, it will select the object to which this face belongs. Since entities don't necessarily have faces, the faces of their bounding boxes will be considered instead. The advantage of this technique is that it allows you to easily select occluded objects in the 2D viewports.

You may also think of left click selection like this: In both the 3D viewport or a 2D viewport, TrenchBroom first compiles a set of candidate objects. These are all objects under the mouse. Then, it must choose an object to be selected from these candidates. In the 3D viewport, the frontmost object always wins (unless you're using the scroll wheel to drill the selection), and in a 2D view, the object with the smallest visible area wins. Other than that, selection behaves exactly the same in both viewports, that is, you can hold #key(308) to select multiple objects and so on.

## Selecting Brush Faces in the 3D Viewport

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

![Entity definition editor](EntityDefinitionEditor.png) Entity definitions are text files containing information about [the meaning of entities and their properties](#entitiy_definitions). Depending on the game and mod you are mapping for, you might want to load different entity definitions into the editor. To load an entity definition file into TrenchBroom, switch to the entity inspector and unfold the entity definition browser at the bottom of the inspector. The entity definition browser is horizontally divided into two areas. The upper area contains a list of _builtin_ entity definition files. These are the entity definition files that came with TrenchBroom for the game you are currently working on. You can select one of these builtin files by clicking on it. TrenchBroom will load the file and update its resources accordingly. Alternatively, you may want to load an external entity definition file of your own. To do this, click the button labeled "Browse" in the lower area of the entity defiinition browser and choose the file you wish to load. Currently, TrenchBroom supports Radiant DEF files and [Valve FGD][FGD File Format] files. To reload the entity definitions from the currently loaded external file, you can click the button labeled "Reload" at the bottom. This may be useful if you're editing an entity definition file for a mod you're working on.

Note that FGD files contain much more information than DEF files and are generally preferrable. While TrenchBroom supports both file types, its support for FGD is better and more comprehensive. Unfortunately, DEF files are still relevant because Radiant style editors require them, so TrenchBroom allows you to use them, too.

The path of an external entity definition file is stored in a worldspawn property called "_tb_def".

### Managing Textures {#texture_management}

![Texture collection editor](TextureCollectionEditor.png) [Texture collections](#textures) can be managed in the texture collection editor, which can be found at the bottom of the face inspector. Unfolding the texture collection editor presents you with a list of the currently loaded texture collections and a few buttons to manage that list. To load a texture collection, click the "+" button below the list, and to remove the selected texture collections, click the "-" button. The order in which the texture collections are loaded determines their respective priorities when TrenchBroom resolves name conflicts, so you can change the loading order by selecting a texture collection and clicking the triangle buttons. The lower a texture collection appears in the list, the higher is its priority. An easy way to think about this is to imagine that textures overwrite each other: If a texture is loaded that has a name conflict with an already loaded texture, then the newly loaded texture overwrites the previously loaded one.

It depends on the game how the texture collection paths are saved in the map file. For Quake and its direct descendants such as Hexen 2, the texture collection paths are stored in a worldspawn property called "wad", as that is what the BSP compilers expect. For other games, they are stored in a worldspawn property called "_tb_textures".

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

To use the brush tool, you first have to activate it by choosing #menu('Menu/Edit/Tools/Brush Tool'). Then, you can place single points onto the grid by left clicking on the faces of other brushes. Additionally, you can left double click on a face to place points on all of its vertices. You can also draw a rectangular shape by left dragging on an existing brush face and thereby place four points at the corners of that rectangle. Finally, if the points you have placed so far form a polygon, you can duplicate and move that polygon along its normal by left dragging it while holding #key(306).

It is not possible to modify or remove points after they have been placed, except discarding all of them by hitting the #action('Controls/Map view/Cancel') key.

### Creating Entities {#creating_entities}

### Duplicating Objects {#duplicating_objects}

### Copy and Paste

## Editing Objects

## Transforming Objects

## Deleting Objects

## Working with Textures

### The UV Editor {#uv_editor}

### The Texture Browser {#texture_browser}

## Shaping brushes

### Clipping Tool

### Vertex Editing

### CSG Operations

## Entity Properties {#entity_properties}

## Keeping an Overview

### Layers {#layers}

### Groups {#groups}

### Hiding and Isolating Objects

### Filtering {#filtering}

### Rendering Options {#rendering_options}

## Undo and Redo

# Preferences

## Game Configuration {#game_configuration}

## View Layout and Rendering {#view_layout_and_rendering}

## Mouse Input {#mouse_input}

## Keyboard Shortcuts

# Advanced Topics

## Command Repetition

## Issue Browser {#issue_browser}

## Solving Problems

## Display Models for Entities

## Customization

# Getting Involved

## Reporting Bugs {#reporting_bugs}

### The Version Information

Open the "About TrenchBroom" dialog from the menu. The light gray text on the left gives you some information about which version of TrenchBroom you are currently running, for example "Version 2.0.0 f335082 D". The first three numbers represent the version (2.0.0), the following seven letter string is the build id (f335082), and the final letter indicates the build type ("D" for Debug and "R" for release). You can also find this information in the Welcome window that the editor shows at startup.

# References and Links {#references_and_links}

- [func_msgboard] - Quake Mapping Forum
- [Tome of Preach] - Quake Map Hacks and QuakeC Hacks

[func_msgboard]: http://celephais.net/board/
[Tome of Preach]: https://tomeofpreach.wordpress.com/
[FGD File Format]: http://developer.valvesoftware.com/wiki/FGD
