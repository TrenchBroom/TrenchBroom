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

#include "ui/ViewUtils.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QString>
#include <QWidget>

#include "PreferenceManager.h"
#include "mdl/EntityDefinitionFileSpec.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "mdl/Map_Assets.h"
#include "ui/ChoosePathTypeDialog.h"
#include "ui/QPathUtils.h"

#include "kd/path_utils.h"
#include "kd/string_compare.h"
#include "kd/string_format.h"

#include <algorithm>
#include <filesystem>

namespace tb::ui
{
namespace
{

bool isEntityDefinitionFile(const std::filesystem::path& path)
{
  static const auto extensions = {".fgd", ".def", ".ent"};

  return std::ranges::any_of(extensions, [&](const auto& extension) {
    return kdl::path_has_extension(kdl::path_to_lower(path), extension);
  });
}

} // namespace

void combineFlags(
  const size_t numFlags, const int newFlagValue, int& setFlags, int& mixedFlags)
{
  for (size_t i = 0; i < numFlags; ++i)
  {
    const auto alreadySet = (newFlagValue & (1 << i)) != 0;
    const auto willBeSet = (setFlags & (1 << i)) != 0;
    if (alreadySet == willBeSet)
    {
      continue;
    }

    setFlags &= ~(1 << i);
    mixedFlags |= (1 << i);
  }
}

bool loadEntityDefinitionFile(mdl::Map& map, QWidget* parent, const QString& pathStr)
{
  const auto gamePath = pref(map.gameInfo().gamePathPreference);
  const auto docPath = map.path();

  const auto absPath = pathFromQString(pathStr);
  if (isEntityDefinitionFile(absPath))
  {
    auto pathDialog = ChoosePathTypeDialog{parent->window(), absPath, docPath, gamePath};
    if (pathDialog.exec() == QDialog::Accepted)
    {
      const auto path =
        convertToPathType(pathDialog.pathType(), absPath, docPath, gamePath);
      const auto spec = mdl::EntityDefinitionFileSpec::makeExternal(path);
      setEntityDefinitionFile(map, spec);
      return true;
    }
  }

  return false;
}

static std::string queryObjectName(
  QWidget* parent, const QString& objectType, const std::string& suggestion)
{
  while (true)
  {
    auto ok = false;
    const auto name = QInputDialog::getText(
                        parent,
                        "Enter a name",
                        QObject::tr("%1 Name").arg(objectType),
                        QLineEdit::Normal,
                        QString::fromStdString(suggestion),
                        &ok)
                        .toStdString();

    if (!ok)
    {
      return "";
    }

    if (kdl::str_is_blank(name))
    {
      if (
        QMessageBox::warning(
          parent,
          "Error",
          QObject::tr("%1 names cannot be blank.").arg(objectType),
          QMessageBox::Ok | QMessageBox::Cancel,
          QMessageBox::Ok)
        != QMessageBox::Ok)
      {
        return "";
      }
    }
    else if (kdl::ci::str_contains(name, "\""))
    {
      if (
        QMessageBox::warning(
          parent,
          "Error",
          QObject::tr("%1 names cannot contain double quotes.").arg(objectType),
          QMessageBox::Ok | QMessageBox::Cancel,
          QMessageBox::Ok)
        != QMessageBox::Ok)
      {
        return "";
      }
    }
    else
    {
      return name;
    }
  }
}

std::string queryGroupName(QWidget* parent, const std::string& suggestion)
{
  return queryObjectName(parent, QObject::tr("Group"), suggestion);
}

std::string queryLayerName(QWidget* parent, const std::string& suggestion)
{
  return queryObjectName(parent, QObject::tr("Layer"), suggestion);
}

} // namespace tb::ui
