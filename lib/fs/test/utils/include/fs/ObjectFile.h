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

#pragma once

#include "io/File.h"

namespace tb::io
{

/**
 * A file that is backed by a C++ object. These kinds of files are used to insert custom
 * objects into the virtual filesystem. An example would be shader objects which are
 * parsed by the shader file system.
 *
 * @tparam T the type of the object represented by this file
 */
template <typename T>
class ObjectFile : public File
{
private:
  T m_object;

public:
  /**
   * Creates a new file with the given object.
   *
   * @tparam S the type of the given object, must be convertible to T
   * @param object the object
   */
  template <typename S>
  explicit ObjectFile(S&& object)
    : m_object(std::forward<S>(object))
  {
  }

  Reader reader() const override
  {
    const auto addr = reinterpret_cast<const char*>(&m_object);
    return Reader::from(addr, addr + size());
  }

  size_t size() const override { return sizeof(m_object); }

  /**
   * Returns the object that backs this file.
   */
  const T& object() const { return m_object; }
};

} // namespace tb::io
