/*
 Copyright (C) 2026 Kristian Duske

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

#include <ostream>

namespace tb::img
{

/**
 * The domain of alpha values present in a texture's decoded pixel data, for formats
 * where this can only be determined by inspecting the pixels (as opposed to a
 * format/name-level convention such as the legacy `{`-prefixed palette masking). Used by
 * materials to decide between alpha-testing (Binary) and real alpha blending (Graduated)
 * when no explicit override is set.
 */
enum class ImageAlphaDomain
{
  /**
   * Every pixel is fully opaque.
   */
  Opaque,
  /**
   * Every pixel is either fully transparent or fully opaque.
   */
  Binary,
  /**
   * At least one pixel has an alpha value strictly between fully transparent and fully
   * opaque.
   */
  Graduated,
};

std::ostream& operator<<(std::ostream& lhs, const ImageAlphaDomain& rhs);

} // namespace tb::img
