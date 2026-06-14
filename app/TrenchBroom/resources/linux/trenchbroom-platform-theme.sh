#!/bin/sh

if [ -z "${QT_QPA_PLATFORMTHEME:-}" ]; then
  case "${XDG_CURRENT_DESKTOP:-}" in
    *GNOME* | *gnome* | *Unity* | *unity* | *XFCE* | *xfce*)
      export QT_QPA_PLATFORMTHEME=gtk3
      ;;
    *)
      export QT_QPA_PLATFORMTHEME=xdgdesktopportal
      ;;
  esac
fi
