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

#include "el/Expression.h"
#include "el/Types.h"
#include "el/Value.h"

#include <fmt/format.h>

#include <string>

namespace tb::el
{

ConversionError::ConversionError(
  const std::optional<FileLocation>& fileLocation,
  const std::string& value,
  const ValueType from,
  const ValueType to)
  : Exception{prependLocation(
      fileLocation,
      fmt::format(
        "Cannot convert value '{}' of type '{}' to type '{}'",
        value,
        typeName(from),
        typeName(to)))}
{
}

DereferenceError::DereferenceError(
  const std::optional<FileLocation>& fileLocation,
  const std::string& value,
  const ValueType from,
  const ValueType to)
  : Exception{prependLocation(
      fileLocation,
      fmt::format(
        "Cannot dereference value '{}' of type '{}' as type '{}'",
        value,
        typeName(from),
        typeName(to)))}
{
}

EvaluationError::EvaluationError() = default;

EvaluationError::EvaluationError(
  const ExpressionNode& expression, const std::string_view reason)
  : EvaluationError{
      expression.location(),
      fmt::format("Cannot evaluate expression '{}': {}", expression.asString(), reason)}
{
}

EvaluationError::EvaluationError(
  const std::optional<FileLocation>& location, std::string_view message)
  : Exception{prependLocation(location, message)}
{
}

IndexError::IndexError(
  const ExpressionNode& expression, const Value& indexableValue, const Value& indexValue)
  : EvaluationError{
      expression,
      fmt::format(
        "'{}' is not a compatible index for '{}'",
        indexValue.describe(),
        indexableValue.describe())}
{
}

IndexError::IndexError(
  const std::optional<FileLocation>& location,
  const Value& indexableValue,
  const size_t index)
  : EvaluationError{
      location,
      fmt::format(
        "{} is not a compatible index for '{}'", index, indexableValue.describe())}
{
}

IndexError::IndexError(
  const std::optional<FileLocation>& location,
  const Value& indexableValue,
  const std::string& key)
  : EvaluationError{
      location,
      fmt::format(
        "'{}' is not a compatible index for '{}'", key, indexableValue.describe())}
{
}

IndexOutOfBoundsError::IndexOutOfBoundsError(
  const ExpressionNode& expression, const Value& indexableValue, const size_t index)
  : EvaluationError{
      expression,
      fmt::format("{} is out of bounds for '{}'", index, indexableValue.describe())}
{
}

IndexOutOfBoundsError::IndexOutOfBoundsError(
  const std::optional<FileLocation>& location,
  const Value& indexableValue,
  const size_t index)
  : EvaluationError{
      location,
      fmt::format("{} is out of bounds for '{}'", index, indexableValue.describe())}
{
}

IndexOutOfBoundsError::IndexOutOfBoundsError(
  const std::optional<FileLocation>& location,
  const Value& indexableValue,
  const std::string& key)
  : EvaluationError{
      location, fmt::format("'{}' not found in '{}'", key, indexableValue.describe())}
{
}

} // namespace tb::el
