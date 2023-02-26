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

#include "Macros.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace TrenchBroom
{
class Logger;
}

namespace TrenchBroom::Assets
{
class Texture;
}

namespace TrenchBroom::IO
{
class File;
class FileSystem;
class Path;

using GetTextureName = std::function<std::string(std::string_view, const Path&)>;

/**
 * Always returns the given texture name. The given path is ignored.
 */
std::string getTextureNameFromTexture(std::string_view textureName, const Path& path);

std::string getTextureNameFromPathSuffix(const Path& path, size_t prefixLength);

/**
 * Returns a function that determines a texture name from a path removing a prefix of
 * the path and returning the remaining suffix as a string, with the extension removed.
 *
 * Note that the length of a prefix refers to the number of path components and not to
 * the number of characetrs.
 *
 * For example, given the path /this/that/over/here/texture.png and a prefix length of
 * 3, the function will return here/texture as the texture name.
 *
 * Given a path with fewer than or the same number of components as the prefix length,
 * an empty string is returned.
 */
GetTextureName makeGetTextureNameFromPathSuffix(size_t prefixLength);

/**
 * Returns a function that always returns the given string when called.
 */
GetTextureName makeGetTextureNameFromString(std::string staticName);

bool checkTextureDimensions(size_t width, size_t height);

size_t mipSize(size_t width, size_t height, size_t mipLevel);

class TextureReader
{
private:
  GetTextureName m_getTextureName;

protected:
  const FileSystem& m_fs;
  Logger& m_logger;

protected:
  TextureReader(GetTextureName getTextureName, const FileSystem& fs, Logger& logger);

public:
  virtual ~TextureReader();

  /**
   * Loads a texture from the given file and returns it. If an error occurs while loading
   * the texture, the default texture is returned.
   *
   * @param file the file containing the texture
   * @return an Assets::Texture object
   */
  Assets::Texture readTexture(std::shared_ptr<File> file) const;

protected:
  std::string textureName(const std::string& textureName, const Path& path) const;
  std::string textureName(const Path& path) const;

private:
  /**
   * Loads a texture and returns an Assets::Texture object allocated with new. Should not
   * throw exceptions to report errors loading textures except for unrecoverable errors
   * (out of memory, bugs, etc.).
   *
   * @param file the file containing the texture
   * @return an Assets::Texture object
   */
  virtual Assets::Texture doReadTexture(std::shared_ptr<File> file) const = 0;

  deleteCopyAndMove(TextureReader);
};

} // namespace TrenchBroom::IO
