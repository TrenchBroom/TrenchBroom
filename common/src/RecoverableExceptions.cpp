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

#include "RecoverableExceptions.h"

#include "IO/DiskIO.h"

namespace TrenchBroom {
FileDeletingException::FileDeletingException(std::string&& str, const IO::Path& path)
  : RecoverableException(std::move(str))
  , m_path(path) {}

std::string_view FileDeletingException::query() const {
  constexpr auto str = "Do you want to delete the file?";
  return str;
}

void FileDeletingException::recover() const {
  IO::Disk::deleteFile(m_path);
}
} // namespace TrenchBroom
