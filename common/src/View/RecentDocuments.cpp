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

#include "RecentDocuments.h"

#include "IO/Path.h"
#include "IO/PathQt.h"
#include "Notifier.h"
#include "View/QtUtils.h"

#include <kdl/vector_utils.h>

#include <QMenu>
#include <QSettings>
#include <QVariant>

namespace TrenchBroom
{
namespace View
{
RecentDocuments::RecentDocuments(const size_t maxSize, QObject* parent)
  : QObject(parent)
  , m_maxSize(maxSize)
{
  assert(m_maxSize > 0);
  loadFromConfig();
}

const std::vector<IO::Path>& RecentDocuments::recentDocuments() const
{
  return m_recentDocuments;
}

void RecentDocuments::addMenu(QMenu* menu)
{
  ensure(menu != nullptr, "menu is null");
  clearMenu(menu);
  createMenuItems(menu);
  m_menus.push_back(menu);
}

void RecentDocuments::removeMenu(QMenu* menu)
{
  ensure(menu != nullptr, "menu is null");
  clearMenu(menu);
  m_menus = kdl::vec_erase(std::move(m_menus), menu);
}

void RecentDocuments::updatePath(const IO::Path& path)
{
  insertPath(path);
  updateMenus();
  saveToConfig();
  emit didChange();
}

void RecentDocuments::removePath(const IO::Path& path)
{
  const size_t oldSize = m_recentDocuments.size();

  const IO::Path canonPath = path.makeCanonical();
  m_recentDocuments = kdl::vec_erase(std::move(m_recentDocuments), canonPath);

  if (oldSize > m_recentDocuments.size())
  {
    updateMenus();
    saveToConfig();
    emit didChange();
  }
}

void RecentDocuments::loadFromConfig()
{
  m_recentDocuments.clear();
  const QSettings settings;
  for (size_t i = 0; i < m_maxSize; ++i)
  {
    const auto key =
      QString::fromStdString(std::string("RecentDocuments/") + std::to_string(i));
    const QVariant value = settings.value(key);
    if (value.isValid())
    {
      m_recentDocuments.push_back(IO::pathFromQString(value.toString()));
    }
    else
    {
      break;
    }
  }
}

void RecentDocuments::saveToConfig()
{
  QSettings settings;
  settings.remove("RecentDocuments");
  for (size_t i = 0; i < m_recentDocuments.size(); ++i)
  {
    const QString key =
      QString::fromStdString(std::string("RecentDocuments/") + std::to_string(i));
    const QVariant value = QVariant(IO::pathAsQString(m_recentDocuments[i]));
    settings.setValue(key, value);
  }
}

void RecentDocuments::insertPath(const IO::Path& path)
{
  const auto canonPath = path.makeCanonical();
  auto it =
    std::find(std::begin(m_recentDocuments), std::end(m_recentDocuments), canonPath);
  if (it != std::end(m_recentDocuments))
  {
    m_recentDocuments.erase(it);
  }
  m_recentDocuments.insert(std::begin(m_recentDocuments), canonPath);
  if (m_recentDocuments.size() > m_maxSize)
  {
    m_recentDocuments.pop_back();
  }
}

void RecentDocuments::updateMenus()
{
  for (auto* menu : m_menus)
  {
    clearMenu(menu);
    createMenuItems(menu);
  }
}

void RecentDocuments::clearMenu(QMenu* menu)
{
  menu->clear();
}

void RecentDocuments::createMenuItems(QMenu* menu)
{
  for (const auto& path : m_recentDocuments)
  {
    menu->addAction(
      IO::pathAsQString(path.filename()), [this, path]() { loadDocument(path); });
  }
}
} // namespace View
} // namespace TrenchBroom
