/*
 Copyright (C) 2010-2017 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ResourceUtils.h"

#include "IO/Path.h"
#include "IO/SystemPaths.h"

#include <wx/log.h>

namespace TrenchBroom {
    namespace IO {
        wxBitmap loadImageResource(const String& name) {
            return loadImageResource(IO::Path(name));
        }

        wxBitmap loadImageResource(const IO::Path& imagePath) {
            wxLogNull logNull; // need this to suppress errors when loading PNG files, see http://trac.wxwidgets.org/ticket/15331
            const IO::Path fullPath = imagePath.isAbsolute() ? imagePath : IO::SystemPaths::resourceDirectory() + IO::Path("images") + imagePath;
            if (!::wxFileExists(fullPath.asString())) {
                return wxNullBitmap;
            } else {
                return wxBitmap(fullPath.asString(), wxBITMAP_TYPE_PNG);
            }
        }

        wxIcon loadIconResource(const IO::Path& imagePath) {
            wxLogNull logNull; // need this to suppress errors when loading PNG files, see http://trac.wxwidgets.org/ticket/15331
            
            wxBitmapType type = wxICON_DEFAULT_TYPE;
            IO::Path fullPath = imagePath.isAbsolute() ? imagePath : IO::SystemPaths::resourceDirectory() + imagePath;
#if defined __APPLE__
            fullPath = fullPath.addExtension("icns");
#elif defined _WIN32
            type = wxBITMAP_TYPE_ICO;
            fullPath = fullPath.addExtension("ico");
#else
            type = wxBITMAP_TYPE_PNG;
            fullPath = fullPath.addExtension("png");
#endif
            if (!::wxFileExists(fullPath.asString())) {
                return wxNullIcon;
            } else {
                return wxIcon(fullPath.asString(), type, 16, 16);
            }
        }

        static QString imagePathToString(const IO::Path& imagePath) {
            const IO::Path fullPath = imagePath.isAbsolute() ? imagePath : IO::SystemPaths::resourceDirectory() + IO::Path("images") + imagePath;
            return QString::fromStdString(fullPath.asString());
        }

        QPixmap loadPixmapResource(const String& name) {
            return loadPixmapResource(IO::Path(name));
        }

        QPixmap loadPixmapResource(const IO::Path& imagePath) {
            const QString imagePathString = imagePathToString(imagePath);
            return QPixmap(imagePathString);
        }

        QIcon loadIconResourceQt(const IO::Path& imagePath) {
            const QString imagePathString = imagePathToString(imagePath);
            return QIcon(imagePathString);
        }
        QIcon loadIconResourceOffOnQt(const IO::Path& offImagePath, const IO::Path& onImagePath) {
            const QString offImagePathString = imagePathToString(offImagePath);
            const QString onImagePathString = imagePathToString(onImagePath);

            QIcon result;
            result.addFile(offImagePathString, QSize(), QIcon::Normal, QIcon::Off);
            result.addFile(onImagePathString, QSize(), QIcon::Normal, QIcon::On);
            return result;
        }
    }
}
