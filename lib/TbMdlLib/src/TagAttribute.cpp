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

#include "mdl/TagAttribute.h"

#include "kd/struct_io.h"

namespace tb::mdl
{


std::weak_ordering TagAttribute::operator<=>(const TagAttribute& other) const
{
  return name <=> other.name;
}

bool TagAttribute::operator==(const TagAttribute& other) const
{
  return name == other.name;
}

std::ostream& operator<<(std::ostream& str, const TagAttribute& attr)
{
  kdl::struct_stream{str} << "TagAttribute"
                          << "type" << attr.type << "name" << attr.name;
  return str;
}

namespace TagAttributes
{
const TagAttribute Transparency = TagAttribute{1, "transparent"};

} // namespace TagAttributes
} // namespace tb::mdl
