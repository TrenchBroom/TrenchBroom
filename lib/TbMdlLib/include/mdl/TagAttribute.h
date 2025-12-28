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

#include <string>

namespace tb::mdl
{

/**
 * This struct represents an attribute of a tag. A tag can have multiple attributes, but
 * the names must be unique.
 */
struct TagAttribute
{
  using AttributeType = unsigned long;

  AttributeType type;
  std::string name;

  std::weak_ordering operator<=>(const TagAttribute& other) const;
  bool operator==(const TagAttribute& other) const;

  friend std::ostream& operator<<(std::ostream& str, const TagAttribute& attr);
};

namespace TagAttributes
{

inline const TagAttribute Transparency = TagAttribute{1, "transparent"};

}

} // namespace tb::mdl
