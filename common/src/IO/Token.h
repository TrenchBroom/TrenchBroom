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

#include "FileLocation.h"

#include "kdl/string_utils.h"

#include <cassert>
#include <string>

namespace TrenchBroom::IO
{

template <typename Type>
class TokenTemplate
{
private:
  Type m_type = 0;
  const char* m_begin = nullptr;
  const char* m_end = nullptr;
  size_t m_position = 0;
  size_t m_line = 0;
  size_t m_column = 0;

public:
  TokenTemplate() = default;

  TokenTemplate(
    const Type type,
    const char* begin,
    const char* end,
    const size_t position,
    const size_t line,
    const size_t column)
    : m_type{type}
    , m_begin{begin}
    , m_end{end}
    , m_position{position}
    , m_line{line}
    , m_column{column}
  {
    assert(end >= begin);
  }

  Type type() const { return m_type; }

  bool hasType(const Type typeMask) const { return (m_type & typeMask) != 0; }

  const char* begin() const { return m_begin; }

  const char* end() const { return m_end; }

  const std::string data() const { return std::string(m_begin, length()); }

  size_t position() const { return m_position; }

  size_t length() const { return static_cast<size_t>(m_end - m_begin); }

  size_t line() const { return m_line; }

  size_t column() const { return m_column; }

  FileLocation location() const { return FileLocation{m_line, m_column}; }

  template <typename T>
  T toFloat() const
  {
    return static_cast<T>(kdl::str_to_double(std::string(m_begin, m_end)).value_or(0.0));
  }

  template <typename T>
  T toInteger() const
  {
    return static_cast<T>(kdl::str_to_long(std::string(m_begin, m_end)).value_or(0l));
  }
};

} // namespace TrenchBroom::IO
