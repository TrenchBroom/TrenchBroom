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

#include "IOUtils.h"

#include "Ensure.h"
#include "Exceptions.h"
#include "IO/PathQt.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <iostream>
#include <streambuf>
#include <string>

namespace TrenchBroom
{
namespace IO
{
size_t fileSize(std::FILE* file)
{
  ensure(file != nullptr, "file is null");
  const auto pos = std::ftell(file);
  if (pos < 0)
  {
    throw FileSystemException("ftell failed");
  }

  if (std::fseek(file, 0, SEEK_END) != 0)
  {
    throw FileSystemException("fseek failed");
  }

  const auto size = std::ftell(file);
  if (size < 0)
  {
    throw FileSystemException("ftell failed");
  }

  if (std::fseek(file, pos, SEEK_SET) != 0)
  {
    throw FileSystemException("fseek failed");
  }

  return static_cast<size_t>(size);
}

} // namespace IO
} // namespace TrenchBroom
