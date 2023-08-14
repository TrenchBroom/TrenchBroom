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

#include <kdl/reflection_decl.h>
#include <kdl/result_forward.h>

#include <filesystem>
#include <iosfwd>
#include <string>
#include <vector>

namespace TrenchBroom
{
struct Error;
class Logger;
} // namespace TrenchBroom

namespace TrenchBroom::Assets
{
class TextureCollection;
}

namespace TrenchBroom::Model
{
struct TextureConfig;
}

namespace TrenchBroom::IO
{
class FileSystem;

kdl::result<std::vector<std::filesystem::path>, Error> findTextureCollections(
  const FileSystem& gameFS, const Model::TextureConfig& textureConfig);

struct LoadTextureCollectionError
{
  std::string msg;

  kdl_reflect_decl(LoadTextureCollectionError, msg);
};

kdl::result<Assets::TextureCollection, LoadTextureCollectionError> loadTextureCollection(
  const std::filesystem::path& path,
  const FileSystem& gameFS,
  const Model::TextureConfig& textureConfig,
  Logger& logger);

} // namespace TrenchBroom::IO
