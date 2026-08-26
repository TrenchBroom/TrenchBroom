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

#include "img/ImageAlphaDomain.h"

#include "kd/reflection_decl.h"

#include <vector>

namespace tb::img
{

struct Image
{
  size_t width = 0;
  size_t height = 0;
  std::vector<unsigned char> pixels; // RGBA8, 4 bytes/pixel, row-major, tightly packed
  ImageAlphaDomain alphaDomain = ImageAlphaDomain::Opaque;

  kdl_reflect_decl(Image, width, height, pixels, alphaDomain);
};

} // namespace tb::img
