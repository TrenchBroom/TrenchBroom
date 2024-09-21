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

#include "ELExceptions.h"

#include "EL/Types.h"
#include "EL/Value.h"

#include <fmt/format.h>

#include <string>

namespace TrenchBroom::EL
{

ConversionError::ConversionError(
  const std::string& value, const ValueType from, const ValueType to)
  : Exception{fmt::format(
    "Cannot convert value '{}' of type '{}' to type '{}'",
    value,
    typeName(from),
    typeName(to))}
{
}

DereferenceError::DereferenceError(
  const std::string& value, const ValueType from, const ValueType to)
  : Exception{fmt::format(
    "Cannot dereference value '{}' of type '{}' as type '{}'",
    value,
    typeName(from),
    typeName(to))}
{
}

IndexError::IndexError(const Value& indexableValue, const Value& indexValue)
  : EvaluationError{fmt::format(
    "Cannot index value '{}' of type '{}' using index '{}' of type '{}'",
    indexableValue.describe(),
    indexableValue.typeName(),
    indexValue.describe(),
    typeName(indexValue.type()))}
{
}

IndexError::IndexError(const Value& indexableValue, const size_t /* index */)
  : EvaluationError{fmt::format(
    "Cannot index value '{}' of type '{}' using integral index",
    indexableValue.describe(),
    indexableValue.typeName())}
{
}

IndexError::IndexError(const Value& indexableValue, const std::string& /* key */)
  : EvaluationError{fmt::format(
    "Cannot index value '{}' of type '{}' using string index",
    indexableValue.describe(),
    indexableValue.typeName())}
{
}

IndexOutOfBoundsError::IndexOutOfBoundsError(
  const Value& indexableValue, const Value& indexValue, const size_t outOfBoundsIndex)
  : EvaluationError{fmt::format(
    "Cannot index value '{}' of type '{}' using index '{}' of type '{}': Index value {} "
    "is out of bounds",
    indexableValue.describe(),
    indexableValue.typeName(),
    indexValue.describe(),
    typeName(indexValue.type()),
    std::to_string(outOfBoundsIndex))}
{
}

IndexOutOfBoundsError::IndexOutOfBoundsError(
  const Value& indexableValue,
  const Value& indexValue,
  const std::string& outOfBoundsIndex)
  : EvaluationError{fmt::format(
    "Cannot index value '{}' of type '{}' using index '{}' of type '{}': Key '{}' not "
    "found",
    indexableValue.describe(),
    indexableValue.typeName(),
    indexValue.describe(),
    typeName(indexValue.type()),
    outOfBoundsIndex)}
{
}

IndexOutOfBoundsError::IndexOutOfBoundsError(
  const Value& indexableValue, const size_t index)
  : EvaluationError{fmt::format(
    "Cannot index value '{}' of type '{}' using integral index: Index value {} is out of "
    "bounds",
    indexableValue.describe(),
    indexableValue.typeName(),
    std::to_string(index))}
{
}

IndexOutOfBoundsError::IndexOutOfBoundsError(
  const Value& indexableValue, const std::string& key)
  : EvaluationError{fmt::format(
    "Cannot index value '{}' of type '{}' using string index: Key '{}' not found",
    indexableValue.describe(),
    indexableValue.typeName(),
    key)}
{
}

} // namespace TrenchBroom::EL
