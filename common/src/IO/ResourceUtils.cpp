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

#include "Ensure.h"
#include "IO/Path.h"
#include "IO/PathQt.h"
#include "IO/SystemPaths.h"

#include <map>
#include <string>

#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QThread>


namespace TrenchBroom {
    namespace IO {
        static QString imagePathToString(const IO::Path& imagePath) {
            const IO::Path fullPath = imagePath.isAbsolute() ? imagePath : IO::SystemPaths::findResourceFile(IO::Path("images") + imagePath);
            return IO::pathAsQString(fullPath);
        }

        QPixmap loadPixmapResource(const std::string& name) {
            return loadPixmapResource(IO::Path(name));
        }

        QPixmap loadPixmapResource(const IO::Path& imagePath) {
            const QString imagePathString = imagePathToString(imagePath);
            return QPixmap(imagePathString);
        }

        QIcon loadIconResourceQt(const IO::Path& imagePath) {
            // Simple caching layer.
            // Without it, the .png files would be read from disk and decoded each time this is called, which is slow.
            // We never evict from the cache which is assumed to be OK because this is just used for icons
            // and there's a relatively small set of them.

            ensure(qApp->thread() == QThread::currentThread(), "loadIconResourceQt can only be used on the main thread");

            static std::map<IO::Path, QIcon> cache;
            {
                auto it = cache.find(imagePath);
                if (it != cache.end()) {
                    return it->second;
                }
            }

            // Cache miss, load the icon
            QIcon result;
            if (!imagePath.isEmpty()) {
                const auto onPath = imagePathToString(imagePath.replaceBasename(imagePath.basename() + "_on"));
                const auto offPath = imagePathToString(imagePath.replaceBasename(imagePath.basename() + "_off"));

                if (!onPath.isEmpty() && !offPath.isEmpty()) {
                    result.addFile(onPath, QSize(), QIcon::Normal, QIcon::On);
                    result.addFile(offPath, QSize(), QIcon::Normal, QIcon::Off);
                } else {
                    const auto imagePathString = imagePathToString(imagePath);

                    if (imagePathString.isEmpty()) {
                        qWarning() << "Couldn't find image for path: " << IO::pathAsQString(imagePath);
                    }

                    result.addFile(imagePathString, QSize(), QIcon::Normal);
                }
            }

            cache[imagePath] = result;

            return result;
        }
    }
}
