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

#include "FileDialogDefaultDir.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include "Macros.h"

namespace tb::ui
{
namespace
{

QString fileDialogDirToString(const FileDialogDir dir)
{
  switch (dir)
  {
  case FileDialogDir::Map:
    return "Map";
  case FileDialogDir::MaterialCollection:
    return "TextureCollection";
  case FileDialogDir::CompileTool:
    return "CompileTool";
  case FileDialogDir::Engine:
    return "Engine";
  case FileDialogDir::EntityDefinition:
    return "EntityDefinition";
  case FileDialogDir::GamePath:
    return "GamePath";
    switchDefault();
  }
}

QString fileDialogDefaultDirectorySettingsPath(const FileDialogDir dir)
{
  return QString::fromLatin1("FileDialog/%1/DefaultDirectory")
    .arg(fileDialogDirToString(dir));
}

} // namespace

QString fileDialogDefaultDirectory(const FileDialogDir dir)
{
  const auto key = fileDialogDefaultDirectorySettingsPath(dir);

  const auto settings = QSettings{};
  const auto defaultDir = settings.value(key).toString();
  return defaultDir;
}

void updateFileDialogDefaultDirectoryWithFilename(
  FileDialogDir type, const QString& filename)
{
  const auto dirQDir = QFileInfo(filename).absoluteDir();
  const auto dirString = dirQDir.absolutePath();
  updateFileDialogDefaultDirectoryWithDirectory(type, dirString);
}

void updateFileDialogDefaultDirectoryWithDirectory(
  FileDialogDir type, const QString& newDefaultDirectory)
{
  const auto key = fileDialogDefaultDirectorySettingsPath(type);

  auto settings = QSettings{};
  settings.setValue(key, newDefaultDirectory);
}

} // namespace tb::ui
