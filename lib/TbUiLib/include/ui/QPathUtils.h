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

#include <QString>

#include <filesystem>

namespace tb::ui
{

/**
 * Returns a QString representation of the given path that is usable with Qt's own file
 * handling classes. These paths use forward slashes as separators on all platforms.
 */
QString pathAsQPath(const std::filesystem::path& path);

/**
 * Returns a platform-specific Qt string representation of the given path.
 */
QString pathAsQString(const std::filesystem::path& path);

/**
 * Returns a generic Qt string representation of the given path using the generic path
 * separators as per the std::filesystem::path::generic_string function.
 */
QString pathAsGenericQString(const std::filesystem::path& path);

/**
 * Returns a std::filesystem::path representation of the given QString path.
 */
std::filesystem::path pathFromQString(const QString& path);

} // namespace tb::ui
