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

#include <ostream>
#include <sstream>

namespace tb
{

bool operator==(const FileLocation& lhs, const FileLocation& rhs)
{
  return lhs.line == rhs.line && lhs.column == rhs.column;
}

bool operator!=(const FileLocation& lhs, const FileLocation& rhs)
{
  return !(lhs == rhs);
}

bool operator<(const FileLocation& lhs, const FileLocation& rhs)
{
  return lhs.line < rhs.line || (lhs.line == rhs.line && lhs.column < rhs.column);
}

bool operator<=(const FileLocation& lhs, const FileLocation& rhs)
{
  return lhs < rhs || lhs == rhs;
}

bool operator>(const FileLocation& lhs, const FileLocation& rhs)
{
  return !(lhs <= rhs);
}

bool operator>=(const FileLocation& lhs, const FileLocation& rhs)
{
  return !(lhs < rhs);
}

std::ostream& operator<<(std::ostream& lhs, const FileLocation& rhs)
{
  lhs << "line " << rhs.line;
  if (rhs.column)
  {
    lhs << ", column " << *rhs.column;
  }
  return lhs;
}

std::string prependLocation(
  const std::optional<FileLocation>& location, std::string_view str)
{
  auto msg = std::stringstream();

  msg << "At ";
  if (location)
  {
    msg << *location;
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
