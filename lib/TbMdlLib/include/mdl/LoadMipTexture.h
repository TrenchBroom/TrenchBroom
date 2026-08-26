/*
 Copyright (C) 2010 Kristian Duske

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

#include "base/Result.h"

#include <filesystem>
#include <string>

namespace tb
{
namespace gl
{
class Texture;
} // namespace gl

namespace fs
{
class Reader;
} // namespace fs

namespace mdl
{
class Palette;

std::string readMipTextureName(fs::Reader& reader);

bool isIdMipTexture(const std::filesystem::path& path);
bool isHlMipTexture(const std::filesystem::path& path);
bool isMipTexture(const std::filesystem::path& path);

Result<gl::Texture> loadIdMipTexture(
  fs::Reader& reader, const Palette& palette, bool isMasked);

Result<gl::Texture> loadHlMipTexture(fs::Reader& reader, bool isMasked);

} // namespace mdl
} // namespace tb
