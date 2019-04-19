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

namespace TrenchBroom {
    namespace IO {
        static QString imagePathToString(const IO::Path& imagePath) {
            const IO::Path fullPath = imagePath.isAbsolute() ? imagePath : IO::SystemPaths::findResourceFile(IO::Path("images") + imagePath);
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
            QIcon result;
            if (!imagePath.isEmpty()) {
                const auto onPath = imagePathToString(imagePath.replaceBasename(imagePath.basename() + "_on"));
                const auto offPath = imagePathToString(imagePath.replaceBasename(imagePath.basename() + "_off"));

                if (!onPath.isEmpty() && !offPath.isEmpty()) {
                    result.addFile(onPath, QSize(), QIcon::Normal, QIcon::On);
                    result.addFile(offPath, QSize(), QIcon::Normal, QIcon::Off);
                } else {
                    const auto imagePathString = imagePathToString(imagePath);
                    result.addFile(imagePathString, QSize(), QIcon::Normal);
                }
            }

            return result;
        }
    }
}
