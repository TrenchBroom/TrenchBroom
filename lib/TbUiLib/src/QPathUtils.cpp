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

#include "ui/QPathUtils.h"

#include <QFileInfo>

#include "kd/path_utils.h"

namespace tb::ui
{

QString pathAsQPath(const std::filesystem::path& path)
{
  return QFile{path}.fileName();
}

QString pathAsQString(const std::filesystem::path& path)
{
#ifdef _WIN32
  return QString::fromStdWString(path.wstring());
#else
  return QString::fromStdString(path.string());
#endif
}

QString pathAsGenericQString(const std::filesystem::path& path)
{
#ifdef _WIN32
  return QString::fromStdWString(path.generic_wstring());
#else
  return QString::fromStdString(path.generic_string());
#endif
}

std::filesystem::path pathFromQString(const QString& path)
{
#ifdef _WIN32
  return std::filesystem::path{kdl::parse_path(path.toStdWString())};
#else
  return std::filesystem::path{kdl::parse_path(path.toStdString())};
#endif
}

} // namespace tb::ui
