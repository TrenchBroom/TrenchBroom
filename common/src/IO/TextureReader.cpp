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

#include "TextureReader.h"

#include "Assets/Texture.h"
#include "Assets/TextureBuffer.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/ResourceUtils.h"
#include "Logger.h"

#include <algorithm>

namespace TrenchBroom::IO
{

std::string getTextureNameFromTexture(const std::string_view textureName, const Path&)
{
  return std::string{textureName};
}

std::string getTextureNameFromPathSuffix(const Path& path, size_t prefixLength)
{
  return prefixLength < path.length()
           ? path.suffix(path.length() - prefixLength).deleteExtension().asString("/")
           : "";
}

GetTextureName makeGetTextureNameFromPathSuffix(const size_t prefixLength)
{
  return [=](const std::string_view, const Path& path) {
    return getTextureNameFromPathSuffix(path, prefixLength);
  };
}

GetTextureName makeGetTextureNameFromString(std::string staticName)
{
  return [staticName = std::move(staticName)](
           const std::string_view, const Path&) { return staticName; };
}

TextureReader::TextureReader(
  GetTextureName getTextureName, const FileSystem& fs, Logger& logger)
  : m_getTextureName{std::move(getTextureName)}
  , m_fs{fs}
  , m_logger{logger}
{
}

TextureReader::~TextureReader() = default;

Assets::Texture TextureReader::readTexture(std::shared_ptr<File> file) const
{
  try
  {
    return doReadTexture(file);
  }
  catch (const AssetException& e)
  {
    m_logger.error() << "Could not read texture '" << file->path() << "': " << e.what();
    return loadDefaultTexture(
      m_fs, textureName(file->path().deleteExtension()), m_logger);
  }
}

std::string TextureReader::textureName(
  const std::string& textureName, const Path& path) const
{
  return m_getTextureName(textureName, path);
}

std::string TextureReader::textureName(const Path& path) const
{
  return m_getTextureName(path.lastComponent().asString(), path);
}

bool TextureReader::checkTextureDimensions(const size_t width, const size_t height)
{
  return width <= 8192 && height <= 8192;
}

size_t TextureReader::mipSize(
  const size_t width, const size_t height, const size_t mipLevel)
{
  const auto size = Assets::sizeAtMipLevel(width, height, mipLevel);
  return size.x() * size.y();
}

} // namespace TrenchBroom::IO
