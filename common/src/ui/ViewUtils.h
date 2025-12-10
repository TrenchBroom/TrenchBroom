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

#include <QStringList>

#include <string>

class QWidget;
class QString;

namespace tb
{
class Logger;

namespace mdl
{
class Map;
}

namespace ui
{

void combineFlags(size_t numFlags, int newFlagValue, int& setFlags, int& mixedFlags);

bool loadEntityDefinitionFile(mdl::Map& map, QWidget* parent, const QString& path);

std::string queryGroupName(QWidget* parent, const std::string& suggestion);
std::string queryLayerName(QWidget* parent, const std::string& suggestion);

} // namespace ui
} // namespace tb
