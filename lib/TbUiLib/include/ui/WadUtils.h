/*
 Copyright (C) 2023 Kristian Duske

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

#include <QStringList>

class QWidget;

namespace tb
{
namespace mdl
{
class Map;
}

namespace ui
{

/**
 * Add the given wad paths to the map. Shows a dialog to choose the path type (absolute,
 * relative to map file, etc.).
 *
 * Returns true if the paths were added and false otherwise.
 */
bool addWadPaths(const QStringList& pathQStrs, mdl::Map& map, QWidget* dialogParent);

} // namespace ui
} // namespace tb
