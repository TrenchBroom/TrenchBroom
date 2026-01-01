# TrenchBroom manual documentation

## The Build Process

TrenchBroom's documentation is contained in a single markdown file (index.md). This file is converted into HTML using [pandoc](http://www.pandoc.org) during the build process. The build process also converts our custom macros (see below) into javascript snippets that output some information into the help document.

If you want to preview the generated HTML without doing a full build, you can just build the GenerateHelp target. Change into your build directory and run

    cmake --build . --target GenerateManual

You will find the generated documentation files in a directory called "gen-manual" (`<build dir>/app/gen-manual`). If you add new resources such as images to the manual's files, you have to refresh your cmake cache first by running

    cmake ..

## Custom Macros

We use two macros to output keyboard shortcuts and menu entries (with full paths) into the documentation. This is to avoid hard coding the defaults for these into the documentation, as they might change later on. However, the keyboard shortcuts and menu structure must be available to the web browser when the help file is displayed. Both the shortcuts and the menu structure are therefore stored in the file shortcuts.js, which is automatically generated during the build process.

The macros are used as follows.

- Print a keyboard shortcut, the default of which is stored in the preferences under the given path:

    \#action('Controls/Map view/Duplicate and move objects up; Duplicate and move objects forward‘)
    
- Print a menu entry, the default of which is again stored under the given path in the preferences:

    \#menu('Menu/Edit/Show All‘)

- Print a key. You can find the key numbers in the shortcuts.js file.

    \#key(123)
