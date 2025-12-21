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
#include "render/GL.h"

#include <filesystem>

namespace tb
{
namespace fs
{
class Reader;
} // namespace fs

namespace mdl
{
class Texture;
class TextureBuffer;
} // namespace mdl

namespace io
{

Color getAverageColor(const mdl::TextureBuffer& buffer, GLenum format);

Result<mdl::Texture> loadFreeImageTextureFromMemory(const uint8_t* begin, size_t size);

Result<mdl::Texture> loadFreeImageTexture(fs::Reader& reader);

bool isSupportedFreeImageExtension(const std::filesystem::path& extension);

} // namespace io
} // namespace tb
