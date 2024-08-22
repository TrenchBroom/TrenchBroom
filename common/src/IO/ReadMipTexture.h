/*
 Copyright (C) 2010-2016 Kristian Duske

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

namespace TrenchBroom::Assets
{
class Palette;
class Texture;
enum class TextureMask;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::IO
{
class Reader;

std::string readMipTextureName(Reader& reader);

Result<Assets::Texture> readIdMipTexture(
  Reader& reader, const Assets::Palette& palette, Assets::TextureMask mask);

Result<Assets::Texture> readHlMipTexture(Reader& reader, Assets::TextureMask mask);

} // namespace TrenchBroom::IO
