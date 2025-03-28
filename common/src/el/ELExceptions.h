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

#include "Exceptions.h"
#include "FileLocation.h"

#include <string>
#include <string_view>

namespace tb::el
{
class ExpressionNode;
class Value;
enum class ValueType;

class Exception : public tb::Exception
{
public:
  using tb::Exception::Exception;
};

class InterpolationError : public tb::Exception
{
public:
  using Exception::Exception;
};

class ConversionError : public Exception
{
public:
  ConversionError(
    const std::optional<FileLocation>& fileLocation,
    const std::string& value,
    ValueType from,
    ValueType to);
};

class DereferenceError : public Exception
{
public:
  DereferenceError(
    const std::optional<FileLocation>& fileLocation,
    const std::string& value,
    ValueType from,
    ValueType to);
};

class EvaluationError : public Exception
{
public:
  EvaluationError();
  EvaluationError(const ExpressionNode& expression, std::string_view reason);
  EvaluationError(const std::optional<FileLocation>& location, std::string_view message);
};

class IndexError : public EvaluationError
{
public:
  IndexError(
    const ExpressionNode& expression,
    const Value& indexableValue,
    const Value& indexValue);
  IndexError(
    const std::optional<FileLocation>& location,
    const Value& indexableValue,
    size_t index);
  IndexError(
    const std::optional<FileLocation>& location,
    const Value& indexableValue,
    const std::string& key);
};

class IndexOutOfBoundsError : public EvaluationError
{
public:
  IndexOutOfBoundsError(
    const ExpressionNode& expression, const Value& indexableValue, size_t index);
  IndexOutOfBoundsError(
    const std::optional<FileLocation>& location,
    const Value& indexableValue,
    size_t index);
  IndexOutOfBoundsError(
    const std::optional<FileLocation>& location,
    const Value& indexableValue,
    const std::string& key);
};

} // namespace tb::el
