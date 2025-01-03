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

#include <memory>
#include <string>

class QWidget;
class QString;

namespace tb
{
class Logger;
}

namespace tb::ui
{
class MapDocument;

void combineFlags(size_t numFlags, int newFlagValue, int& setFlags, int& mixedFlags);

bool loadEntityDefinitionFile(
  std::weak_ptr<MapDocument> document, QWidget* parent, const QString& path);
size_t loadEntityDefinitionFile(
  std::weak_ptr<MapDocument> document, QWidget* parent, const QStringList& pathStrs);

std::string queryGroupName(QWidget* parent, const std::string& suggestion);
std::string queryLayerName(QWidget* parent, const std::string& suggestion);

} // namespace tb::ui
