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
}

namespace TrenchBroom::Assets
{
class Material;
class Texture;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::IO
{
class FileSystem;

static const auto DefaultTexturePath = std::filesystem::path{"textures/__TB_empty.png"};

/**
 * Loads a default texture from the given file system. If the default texture cannot be
 * found or opened, an empty texture is returned.
 *
 * @param fs the file system used to locate the texture file
 * @param name the name of the texture to be returned
 * @return the default texture
 */
Assets::Texture loadDefaultTexture(const FileSystem& fs, Logger& logger);

/**
 * Loads a default material from the given file system. If the default material cannot be
 * found or opened, an empty material is returned.
 *
 * @param fs the file system used to locate the material file
 * @param name the name of the material to be returned
 * @return the default material
 */
Assets::Material loadDefaultMaterial(
  const FileSystem& fs, std::string name, Logger& logger);

QPixmap loadPixmapResource(const std::filesystem::path& imagePath);

QPixmap loadSVGPixmap(const std::filesystem::path& imagePath);

/**
 * Loads an SVG image into a QIcon
 */
QIcon loadSVGIcon(const std::filesystem::path& imagePath);

} // namespace TrenchBroom::IO
