/*
 Copyright (C) 2024 Kristian Duske

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

#include "FileLocation.h"

#include "kd/reflection_impl.h"

#include <ostream>
#include <sstream>

namespace tb
{

kdl_reflect_impl(FileLocation);

std::string prependLocation(
  const std::optional<FileLocation>& location, std::string_view str)
{
  auto msg = std::stringstream();

  msg << "At ";
  if (location)
  {
    msg << "line " << location->line;
    if (location->column)
    {
      msg << ", column " << *location->column;
    }
  }
  else
  {
    msg << "unknown location";
  }

  msg << ":";
  if (!str.empty())
  {
    msg << " " << str;
  }
  return msg.str();
}

} // namespace tb
