/*
 Copyright (C) 2025 Kristian Duske

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

#include "update/FileUtils.h"

#include <QDir>
#include <QFileInfo>

namespace upd
{

bool remove(const QString& path)
{
  const auto info = QFileInfo{path};
  if (info.isFile() || info.isSymLink())
  {
    if (!QFile::remove(path))
    {
      return false;
    }
  }
  else if (info.isDir())
  {
    if (!QDir{path}.removeRecursively())
    {
      return false;
    }
  }

  return true;
}

bool cleanDirectory(const QString& path)
{
  return remove(path) && QDir{path}.mkpath(".");
}

} // namespace upd
