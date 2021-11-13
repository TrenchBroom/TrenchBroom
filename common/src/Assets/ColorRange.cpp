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

#include "ColorRange.h"

#include <kdl/string_utils.h>

namespace TrenchBroom {
namespace Assets {
ColorRange::Type detectColorRange(const std::vector<std::string>& components);

ColorRange::Type detectColorRange(const std::string& str) {
  return detectColorRange(kdl::str_split(str, " "));
}

ColorRange::Type detectColorRange(const std::vector<std::string>& components) {
  if (components.size() != 3)
    return ColorRange::Unset;

  auto range = ColorRange::Byte;
  auto leq1 = true;
  for (size_t i = 0; i < 3 && range == ColorRange::Byte; ++i) {
    if (components[i].find('.') != std::string::npos) {
      range = ColorRange::Float;
    } else if (components[i] != "0" && components[i] != "1") {
      leq1 = false;
    }
  }

  // All values are either 0 or 1, so we assume float range.
  if (range == ColorRange::Byte && leq1) {
    range = ColorRange::Float;
  }

  return range;
}
} // namespace Assets
} // namespace TrenchBroom
