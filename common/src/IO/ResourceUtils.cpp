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

#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QSvgRenderer>
#include <QThread>

#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Assets/TextureResource.h"
#include "Ensure.h"
#include "Error.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/PathQt.h"
#include "IO/ReadFreeImageTexture.h"
#include "IO/SystemPaths.h"
#include "Logger.h"

#include "kdl/result.h"
#include "kdl/set_temp.h"

#include <map>
#include <string>

namespace TrenchBroom::IO
{

Assets::Texture loadDefaultTexture(const FileSystem& fs, Logger& logger)
{
  // recursion guard
  static auto executing = false;
  if (!executing)
  {
    const auto set_executing = kdl::set_temp{executing};

    return fs.openFile(DefaultTexturePath) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             return readFreeImageTexture(reader);
           })
           | kdl::transform_error([&](auto e) {
               logger.error() << "Could not load default texture: " << e.msg;
               return Assets::Texture{32, 32};
             })
           | kdl::value();
  }
  else
  {
    logger.error() << "Could not load default texture";
  }

  return Assets::Texture{32, 32};
}

Assets::Material loadDefaultMaterial(
  const FileSystem& fs, std::string name, Logger& logger)
{
  auto textureResource = createTextureResource(loadDefaultTexture(fs, logger));
  return Assets::Material{std::move(name), std::move(textureResource)};
}

static QString imagePathToString(const std::filesystem::path& imagePath)
{
  const auto fullPath = imagePath.is_absolute()
                          ? imagePath
                          : SystemPaths::findResourceFile("images" / imagePath);
  return pathAsQString(fullPath);
}

QPixmap loadPixmapResource(const std::filesystem::path& imagePath)
{
  return QPixmap{imagePathToString(imagePath)};
}

namespace
{
QImage createDisabledState(const QImage& image)
{
  // Convert to greyscale, divide the opacity by 3
  auto disabledImage = image.convertToFormat(QImage::Format_ARGB32);
  const auto w = disabledImage.width();
  const auto h = disabledImage.height();
  for (int y = 0; y < h; ++y)
  {
    auto* row = reinterpret_cast<QRgb*>(disabledImage.scanLine(y));
    for (int x = 0; x < w; ++x)
    {
      const auto oldPixel = row[x];
      const auto grey = (qRed(oldPixel) + qGreen(oldPixel) + qBlue(oldPixel)) / 3;
      const auto alpha = qAlpha(oldPixel) / 3;
      row[x] = qRgba(grey, grey, grey, alpha);
    }
  }

  return disabledImage;
}

QImage renderSvgToImage(
  QSvgRenderer& svgSource, const bool invert, const qreal devicePixelRatio)
{
  if (!svgSource.isValid())
  {
    return QImage{};
  }

  auto image = QImage{
    int(svgSource.defaultSize().width() * devicePixelRatio),
    int(svgSource.defaultSize().height() * devicePixelRatio),
    QImage::Format_ARGB32_Premultiplied};
  image.fill(Qt::transparent);

  auto paint = QPainter{&image};
  svgSource.render(&paint);
  image.setDevicePixelRatio(devicePixelRatio);

  if (invert && image.isGrayscale())
  {
    image.invertPixels();
  }

  return image;
}

void renderSvgToIcon(
  QSvgRenderer& svgSource,
  QIcon& icon,
  const QIcon::State state,
  const bool invert,
  const qreal devicePixelRatio)
{
  if (!svgSource.isValid())
  {
    return;
  }

  auto image = renderSvgToImage(svgSource, invert, devicePixelRatio);
  icon.addPixmap(QPixmap::fromImage(image), QIcon::Normal, state);
  icon.addPixmap(QPixmap::fromImage(createDisabledState(image)), QIcon::Disabled, state);
}

} // namespace

QPixmap loadSVGPixmap(const std::filesystem::path& imagePath)
{
  ensure(
    qApp->thread() == QThread::currentThread(),
    "loadSVGIcon can only be used on the main thread");

  static auto cache = std::map<std::filesystem::path, QPixmap>{};
  if (const auto it = cache.find(imagePath); it != cache.end())
  {
    return it->second;
  }

  const auto palette = QPalette{};
  const auto windowColor = palette.color(QPalette::Active, QPalette::Window);
  const auto darkTheme = windowColor.lightness() <= 127;

  // Cache miss, load the image
  if (!imagePath.empty())
  {
    const auto imagePathString = imagePathToString(imagePath);
    auto renderer = QSvgRenderer{imagePathString};
    if (!renderer.isValid())
    {
      qWarning() << "Failed to load SVG " << imagePathString;
    }

    auto pixmap = QPixmap::fromImage(renderSvgToImage(renderer, darkTheme, 1.0));
    cache[imagePath] = pixmap;
    return pixmap;
  }

  cache[imagePath] = QPixmap{};
  return QPixmap{};
}

QIcon loadSVGIcon(const std::filesystem::path& imagePath)
{
  // Simple caching layer.
  // Without it, the .svg files would be read from disk and decoded each time this is
  // called, which is slow. We never evict from the cache which is assumed to be OK
  // because this is just used for icons and there's a relatively small set of them.

  ensure(
    qApp->thread() == QThread::currentThread(),
    "loadSVGIcon can only be used on the main thread");

  static auto cache = std::map<std::filesystem::path, QIcon>{};
  if (const auto it = cache.find(imagePath); it != cache.end())
  {
    return it->second;
  }

  const auto palette = QPalette{};
  const auto windowColor = palette.color(QPalette::Active, QPalette::Window);
  const auto darkTheme = windowColor.lightness() <= 127;

  // Cache miss, load the icon
  auto result = QIcon{};
  if (!imagePath.empty())
  {
    const auto onPath =
      imagePathToString(imagePath.parent_path() / imagePath.stem() += "_on.svg");
    const auto offPath =
      imagePathToString(imagePath.parent_path() / imagePath.stem() += "_off.svg");
    const auto imagePathString = imagePathToString(imagePath);

    if (!onPath.isEmpty() && !offPath.isEmpty())
    {
      auto onRenderer = QSvgRenderer{onPath};
      if (!onRenderer.isValid())
      {
        qWarning() << "Failed to load SVG " << onPath;
      }

      auto offRenderer = QSvgRenderer{offPath};
      if (!offRenderer.isValid())
      {
        qWarning() << "Failed to load SVG " << offPath;
      }

      renderSvgToIcon(onRenderer, result, QIcon::On, darkTheme, 1.0);
      renderSvgToIcon(onRenderer, result, QIcon::On, darkTheme, 2.0);
      renderSvgToIcon(offRenderer, result, QIcon::Off, darkTheme, 1.0);
      renderSvgToIcon(offRenderer, result, QIcon::Off, darkTheme, 2.0);
    }
    else if (!imagePathString.isEmpty())
    {
      auto renderer = QSvgRenderer{imagePathString};
      if (!renderer.isValid())
      {
        qWarning() << "Failed to load SVG " << imagePathString;
      }

      renderSvgToIcon(renderer, result, QIcon::Off, darkTheme, 1.0);
      renderSvgToIcon(renderer, result, QIcon::Off, darkTheme, 2.0);
    }
    else
    {
      qWarning() << "Couldn't find image for path: " << pathAsQString(imagePath);
    }
  }

  cache[imagePath] = result;

  return result;
}

} // namespace TrenchBroom::IO
