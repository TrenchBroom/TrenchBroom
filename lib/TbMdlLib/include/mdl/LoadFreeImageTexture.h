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
#include "Result.h"
#include "gl/GL.h"

#include <filesystem>

namespace tb
{
namespace gl
{
class Texture;
class TextureBuffer;
} // namespace gl

namespace fs
{
class Reader;
} // namespace fs

namespace mdl
{

Color getAverageColor(const gl::TextureBuffer& buffer, GLenum format);

Result<gl::Texture> loadFreeImageTextureFromMemory(const uint8_t* begin, size_t size);

Result<gl::Texture> loadFreeImageTexture(fs::Reader& reader);

bool isSupportedFreeImageExtension(const std::filesystem::path& extension);

} // namespace mdl
} // namespace tb
