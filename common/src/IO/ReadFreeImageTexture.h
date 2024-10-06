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

#include "Color.h"
#include "Renderer/GL.h"
#include "Result.h"

#include <string_view>

namespace tb::asset
{
class Texture;
class TextureBuffer;
} // namespace tb::asset

namespace tb::IO
{

class Reader;

Color getAverageColor(const asset::TextureBuffer& buffer, GLenum format);

Result<asset::Texture> readFreeImageTextureFromMemory(const uint8_t* begin, size_t size);

Result<asset::Texture> readFreeImageTexture(Reader& reader);

bool isSupportedFreeImageExtension(std::string_view extension);

} // namespace tb::IO
