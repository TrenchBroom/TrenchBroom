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

#include <vector>

namespace tb::img
{

/**
 * Decodes a BMP image and returns its colors as a flat sequence of RGB triples: the
 * image's own palette if it has one (for indexed images), otherwise its pixels (for
 * direct-color images).
 */
Result<std::vector<unsigned char>> decodeBmpPalette(
  const unsigned char* begin, const unsigned char* end);

} // namespace tb::img
