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

namespace tb::ui
{

enum class FileDialogDir
{
  Map,
  MaterialCollection,
  CompileTool,
  Engine,
  EntityDefinition,
  GamePath
};

/**
 * Gets the default directory from QSettings to use for the given type of file chooser.
 */
QString fileDialogDefaultDirectory(FileDialogDir type);

void updateFileDialogDefaultDirectoryWithFilename(
  FileDialogDir type, const QString& filename);
void updateFileDialogDefaultDirectoryWithDirectory(
  FileDialogDir type, const QString& newDefaultDirectory);

} // namespace tb::ui
