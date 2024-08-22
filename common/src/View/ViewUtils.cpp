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

#include "ViewUtils.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QString>
#include <QWidget>

#include "Assets/EntityDefinitionFileSpec.h"
#include "IO/PathQt.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/ChoosePathTypeDialog.h"
#include "View/MapDocument.h"

#include "kdl/memory_utils.h"
#include "kdl/string_compare.h"
#include "kdl/string_format.h"

#include <filesystem>
#include <memory>

namespace TrenchBroom::View
{

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

bool loadEntityDefinitionFile(
  std::weak_ptr<MapDocument> document, QWidget* parent, const QString& path)
{
  return loadEntityDefinitionFile(document, parent, QStringList{path}) == 0;
}

size_t loadEntityDefinitionFile(
  std::weak_ptr<MapDocument> i_document, QWidget* parent, const QStringList& pathStrs)
{
  if (pathStrs.empty())
  {
    return 0;
  }

  auto document = kdl::mem_lock(i_document);
  auto game = document->game();
  const auto& gameFactory = Model::GameFactory::instance();
  const auto gamePath = gameFactory.gamePath(game->config().name);
  const auto docPath = document->path();

  for (int i = 0; i < pathStrs.size(); ++i)
  {
    const auto& pathStr = pathStrs[i];
    const auto absPath = IO::pathFromQString(pathStr);
    if (game->isEntityDefinitionFile(absPath))
    {
      auto pathDialog =
        ChoosePathTypeDialog{parent->window(), absPath, docPath, gamePath};
      if (pathDialog.exec() == QDialog::Accepted)
      {
        const auto path =
          convertToPathType(pathDialog.pathType(), absPath, docPath, gamePath);
        const auto spec = Assets::EntityDefinitionFileSpec::external(path);
        document->setEntityDefinitionFile(spec);
        return static_cast<size_t>(i);
      }
    }
  }

  return static_cast<size_t>(pathStrs.size());
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

} // namespace TrenchBroom::View
