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

#include <QMenu>
#include <QSettings>
#include <QVariant>

#include "IO/PathQt.h"
#include "Notifier.h"
#include "View/QtUtils.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom::View
{
std::vector<std::filesystem::path> loadRecentDocuments(const size_t max)
{
  auto result = std::vector<std::filesystem::path>{};
  result.reserve(max);

  const auto settings = QSettings{};
  for (size_t i = 0; i < max; ++i)
  {
    const auto key = QString::fromStdString("RecentDocuments/" + std::to_string(i));
    const auto value = settings.value(key);
    if (value.isValid())
    {
      result.push_back(IO::pathFromQString(value.toString()));
    }
    else
    {
      break;
    }
  }

  return result;
}

void saveRecentDocuments(const std::vector<std::filesystem::path>& paths)
{
  auto settings = QSettings{};
  settings.remove("RecentDocuments");

  for (size_t i = 0; i < paths.size(); ++i)
  {
    const auto key = QString::fromStdString("RecentDocuments/" + std::to_string(i));
    const auto value = QVariant{IO::pathAsQString(paths[i])};
    settings.setValue(key, value);
  }
}

RecentDocuments::RecentDocuments(const size_t maxSize, QObject* parent)
  : QObject{parent}
  , m_maxSize{maxSize}
{
  assert(m_maxSize > 0);
}

const std::vector<std::filesystem::path>& RecentDocuments::recentDocuments() const
{
  return m_recentDocuments;
}

void RecentDocuments::reload()
{
  const auto previousRecentDocuments = loadFromConfig();
  if (previousRecentDocuments != m_recentDocuments)
  {
    updateMenus();
    emit didChange();
  }
}

void RecentDocuments::addMenu(QMenu& menu)
{
  clearMenu(menu);
  createMenuItems(menu);
  m_menus.push_back(&menu);
}

void RecentDocuments::removeMenu(QMenu& menu)
{
  clearMenu(menu);
  m_menus = kdl::vec_erase(std::move(m_menus), &menu);
}

void RecentDocuments::updatePath(const std::filesystem::path& path)
{
  insertPath(path);
  updateMenus();
  saveToConfig();
  emit didChange();
}

void RecentDocuments::removePath(const std::filesystem::path& path)
{
  const auto oldSize = m_recentDocuments.size();

  const auto canonPath = path.lexically_normal();
  m_recentDocuments = kdl::vec_erase(std::move(m_recentDocuments), canonPath);

  if (oldSize > m_recentDocuments.size())
  {
    updateMenus();
    saveToConfig();
    emit didChange();
  }
}

std::vector<std::filesystem::path> RecentDocuments::loadFromConfig()
{
  return std::exchange(m_recentDocuments, loadRecentDocuments(m_maxSize));
}

void RecentDocuments::saveToConfig()
{
  saveRecentDocuments(m_recentDocuments);
}

void RecentDocuments::insertPath(const std::filesystem::path& path)
{
  const auto canonPath = path.lexically_normal();
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
    clearMenu(*menu);
    createMenuItems(*menu);
  }
}

void RecentDocuments::clearMenu(QMenu& menu)
{
  menu.clear();
}

void RecentDocuments::createMenuItems(QMenu& menu)
{
  for (const auto& path : m_recentDocuments)
  {
    menu.addAction(
      IO::pathAsQString(path.filename()), [this, path]() { loadDocument(path); });
  }
}

} // namespace TrenchBroom::View
