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

#pragma once

#include <filesystem>
#include <memory>
#include <string>

class QIcon;
class QPixmap;

namespace TrenchBroom
{
class Logger;

namespace Assets
{
class Texture;
}

namespace IO
{
class FileSystem;

/**
 * Loads a default texture file from the given file system. If the default texture cannot
 * be found or opened, an empty texture is returned.
 *
 * @param fs the file system used to locate the texture file
 * @param name the name of the texture to be returned
 * @return the default texture
 */
Assets::Texture loadDefaultTexture(
  const FileSystem& fs, const std::string& name, Logger& logger);

QPixmap loadPixmapResource(const std::filesystem::path& imagePath);

/**
 * Loads an SVG image into a QIcon
 */
QIcon loadSVGIcon(const std::filesystem::path& imagePath);
} // namespace IO
} // namespace TrenchBroom
