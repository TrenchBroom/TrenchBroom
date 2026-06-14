#!/bin/sh

# Unlike Flatpak and Snap, AppImage does not make Qt select this plugin automatically.
export QT_QPA_PLATFORMTHEME="${QT_QPA_PLATFORMTHEME:-xdgdesktopportal}"
