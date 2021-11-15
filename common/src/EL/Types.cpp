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

#include "Types.h"

#include "Macros.h"

#include <string>

namespace TrenchBroom {
namespace EL {
std::string typeName(const ValueType type) {
  switch (type) {
    case ValueType::Boolean:
      return "Boolean";
    case ValueType::String:
      return "String";
    case ValueType::Number:
      return "Number";
    case ValueType::Array:
      return "Array";
    case ValueType::Map:
      return "Map";
    case ValueType::Range:
      return "Range";
    case ValueType::Null:
      return "Null";
    case ValueType::Undefined:
      return "Undefined";
      switchDefault();
  }
}

ValueType typeForName(const std::string& type) {
  if (type == "Boolean")
    return ValueType::Boolean;
  if (type == "String")
    return ValueType::String;
  if (type == "Number")
    return ValueType::Number;
  if (type == "Array")
    return ValueType::Array;
  if (type == "Map")
    return ValueType::Map;
  if (type == "Range")
    return ValueType::Range;
  if (type == "Undefined")
    return ValueType::Undefined;
  assert(false);
  return ValueType::Null;
}
} // namespace EL
} // namespace TrenchBroom
