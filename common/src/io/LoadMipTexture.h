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

#include "Result.h"

#include <string>

namespace tb
{
namespace fs
{
class Reader;
} // namespace fs

namespace mdl
{
class Palette;
class Texture;
enum class TextureMask;
} // namespace mdl

namespace io
{

std::string readMipTextureName(fs::Reader& reader);

Result<mdl::Texture> loadIdMipTexture(
  fs::Reader& reader, const mdl::Palette& palette, mdl::TextureMask mask);

Result<mdl::Texture> loadHlMipTexture(fs::Reader& reader, mdl::TextureMask mask);

} // namespace io
} // namespace tb
