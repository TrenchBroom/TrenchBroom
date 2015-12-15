# TrenchBroom help documentation

## The Build Process

TrenchBroom's documentation is contained in a single markdown file (index.md). This file is converted into HTML using [pandoc](http://www.pandoc.org) during the build process. The build process also converts our custom macros (see below) into javascript snippets that output some information into the help document.

If you want to preview the generated HTML without doing a rebuild, you can just run pandoc like this:

    pandoc -s --toc --toc-depth=2 --template template.html -o index.html index.md

## Custom Macros

We use two macros to output keyboard shortcuts and menu entries (with full paths) into the documentation. This is to avoid hard coding the defaults for these into the documentation, as they might change later on. However, it's not fully automated, as the keyboard shortcuts and menu structure must be available to the web browser when the help file is displayed. Both the shortcuts and the mnu structure are therefore stored in the file shortcuts.js, which can be generated from the Debug menu in a Debug build of TrenchBroom. So whenever a keyboard shortcut or the menu is changed in code, this file must be updated.

The macros are used like so:
- **\#action('Controls/Map view/Duplicate and move objects up; Duplicate and move objects forward‘)** outputs a keyboard shortcut, the default of which is stored in the preferences under the given path.
- **\#menu('Menu/Edit/Show All‘)** outputs a menu entry, the default of which is again stored under the given path in the preferences.
