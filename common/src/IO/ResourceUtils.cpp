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
#include "Logger.h"
#include "Assets/Texture.h"
#include "IO/FileSystem.h"
#include "IO/FreeImageTextureReader.h"
#include "IO/Path.h"
#include "IO/PathQt.h"
#include "IO/SystemPaths.h"

#include <kdl/set_temp.h>

#include <map>
#include <string>

#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QThread>
#include <QSvgRenderer>

namespace TrenchBroom {
    namespace IO {
        Assets::Texture loadDefaultTexture(const FileSystem& fs, Logger& logger, const std::string& name) {
            // recursion guard
            static bool executing = false;
            if (!executing) {
                const kdl::set_temp set_executing(executing);
                
                try {
                    const auto file = fs.openFile(Path("textures/__TB_empty.png"));
                    FreeImageTextureReader imageReader(IO::TextureReader::StaticNameStrategy(name), fs, logger);
                    return imageReader.readTexture(file);
                } catch (const Exception& e) {
                    logger.error() << "Could not load default texture: " << e.what();
                    // fall through to return an empty texture
                }
            } else {
                logger.error() << "Could not load default texture";
            }
            return Assets::Texture(name, 32, 32);
        }

        static QString imagePathToString(const Path& imagePath) {
            const Path fullPath = imagePath.isAbsolute() ? imagePath : SystemPaths::findResourceFile(Path("images") + imagePath);
            return pathAsQString(fullPath);
        }

        QPixmap loadPixmapResource(const std::string& name) {
            return loadPixmapResource(Path(name));
        }

        QPixmap loadPixmapResource(const Path& imagePath) {
            const QString imagePathString = imagePathToString(imagePath);
            return QPixmap(imagePathString);
        }

        static QImage createDisabledState(const QImage& image) {
            // Convert to greyscale, divide the opacity by 3
            auto disabledImage = image.convertToFormat(QImage::Format_ARGB32);
            const int w = disabledImage.width();
            const int h = disabledImage.height();
            for (int y = 0; y < h; ++y) {
                QRgb* row = reinterpret_cast<QRgb*>(disabledImage.scanLine(y));
                for (int x = 0; x < w; ++x) {
                    const QRgb oldPixel = row[x];
                    const int grey = (qRed(oldPixel) + qGreen(oldPixel) + qBlue(oldPixel)) / 3;
                    const int alpha = qAlpha(oldPixel) / 3;
                    row[x] = qRgba(grey, grey, grey, alpha);
                }
            }

            return disabledImage;
        }
        
        static void renderSvgToIcon(QSvgRenderer& svgSource, QIcon& icon, const QIcon::State state, const bool invert, const qreal devicePixelRatio) {
            if (!svgSource.isValid()) {
                return;
            }

            QImage image(static_cast<int>(svgSource.defaultSize().width() * devicePixelRatio),
                static_cast<int>(svgSource.defaultSize().height() * devicePixelRatio),
                QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);            
            {
                QPainter paint(&image);
                svgSource.render(&paint);
            }
            image.setDevicePixelRatio(devicePixelRatio);
            
            if (invert && image.isGrayscale()) {
                image.invertPixels();
            }
            
            icon.addPixmap(QPixmap::fromImage(image), QIcon::Normal, state);
            icon.addPixmap(QPixmap::fromImage(createDisabledState(image)), QIcon::Disabled, state);
        }

        QIcon loadSVGIcon(const Path& imagePath) {
            // Simple caching layer.
            // Without it, the .svg files would be read from disk and decoded each time this is called, which is slow.
            // We never evict from the cache which is assumed to be OK because this is just used for icons
            // and there's a relatively small set of them.

            ensure(qApp->thread() == QThread::currentThread(), "loadIconResourceQt can only be used on the main thread");

            static std::map<Path, QIcon> cache;
            {
                auto it = cache.find(imagePath);
                if (it != cache.end()) {
                    return it->second;
                }
            }

            const auto palette = QPalette();
            const auto windowColor = palette.color(QPalette::Active, QPalette::Window);
            const auto darkTheme = windowColor.lightness() <= 127;
            
            // Cache miss, load the icon
            QIcon result;
            if (!imagePath.isEmpty()) {
                const auto onPath = imagePathToString(imagePath.replaceBasename(imagePath.basename() + "_on"));
                const auto offPath = imagePathToString(imagePath.replaceBasename(imagePath.basename() + "_off"));
                const auto imagePathString = imagePathToString(imagePath);

                if (!onPath.isEmpty() && !offPath.isEmpty()) {
                    QSvgRenderer onRenderer(onPath);
                    if (!onRenderer.isValid()) {
                        qWarning() << "Failed to load SVG " << onPath;
                    }

                    QSvgRenderer offRenderer(offPath);
                    if (!offRenderer.isValid()) {
                        qWarning() << "Failed to load SVG " << offPath;
                    }

                    renderSvgToIcon(onRenderer,  result, QIcon::On,  darkTheme, 1.0);
                    renderSvgToIcon(onRenderer,  result, QIcon::On,  darkTheme, 2.0);
                    renderSvgToIcon(offRenderer, result, QIcon::Off, darkTheme, 1.0);
                    renderSvgToIcon(offRenderer, result, QIcon::Off, darkTheme, 2.0);                    
                } else if (!imagePathString.isEmpty()) {
                    QSvgRenderer renderer(imagePathString);
                    if (!renderer.isValid()) {
                        qWarning() << "Failed to load SVG " << imagePathString;
                    }

                    renderSvgToIcon(renderer,    result, QIcon::Off, darkTheme, 1.0);
                    renderSvgToIcon(renderer,    result, QIcon::Off, darkTheme, 2.0);
                } else {
                    qWarning() << "Couldn't find image for path: " << pathAsQString(imagePath);
                }
            }

            cache[imagePath] = result;

            return result;
        }
    }
}
